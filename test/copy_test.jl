# This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license.

using HDF5

type_to_name(::Type{Int8}) = "char"
type_to_name(::Type{UInt8}) = "uchar"
type_to_name(::Type{Int16}) = "short"
type_to_name(::Type{UInt16}) = "ushort"
type_to_name(::Type{Int32}) = "int"
type_to_name(::Type{UInt32}) = "uint"
type_to_name(::Type{Int64}) = "long"
type_to_name(::Type{UInt64}) = "ulong"
type_to_name(::Type{Float32}) = "float"
type_to_name(::Type{Float64}) = "double"

function test(T::DataType, length=32)
  copy_kernel =
"""
#ifdef cl_khr_fp64
  #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
  #error \"IEEE-754 double precision not supported by OpenCL implementation.\"
#endif

kernel void copy(global COPYTYPE const* in, global COPYTYPE* out)
{
  const int gid = get_global_id(0);
  out[gid] = in[gid];
}
"""
  kernel_url = "copy_kernel_jl.cl"
  open(kernel_url, "w") do io
    write(io, copy_kernel)
  end

  in = Vector{T}(0:(length-1))
  out = fill(zero(T), length)
  single_value = T(21)

  filename = "copy_" * type_to_name(T) * "_test_jl.h5"

  h5open(filename, "w") do io
    write(io, "Kernel_Settings", "-DCOPYTYPE="*type_to_name(T))
    write(io, "Kernel_URL", kernel_url)
    write(io, "Kernels", ["copy"])

    # ranges
    tmp_range = Int32[length, 1, 1]
    write(io, "Global_Range", tmp_range)

    tmp_range .= 0
    write(io, "Local_Range", tmp_range)
    write(io, "Range_Start", tmp_range)

    # data
    write(io, "Data/in", in)
    write(io, "Data/out", out)

    # single vales
    write(io, "Single_Value", single_value)
  end

  # call toolkitICL
  try
    run(`toolkitICL -c $filename`)
  catch e
    println(e)
    return false
  end

  # check result
  out_filename = "out_" * filename
  in_test = h5read(out_filename, "Data/in")
  vec(in_test) == in || return false

  out_test = h5read(out_filename, "Data/out")
  vec(out_test) == in || return false

  single_value_test = h5read(filename, "Single_Value")
  single_value_test == single_value || return false

  return true
end


test_types = (Int8, UInt8, Int16, UInt16, Int32, UInt32, Int64, UInt64, Float32, Float64)

for T in test_types
  println("Type: ", T)
  test(T) || exit(1)
  println()
end
