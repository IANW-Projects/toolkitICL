# Data Types

Here is a list of data types used in `toolkitICL`.


## OpenCL Kernels

For OpenCL kernels run by `toolkitICL`, the following data types are supported.

|  OpenCL  |    C/C++    |         HDF5        |   Julia   |     Numpy    |  Matlab  |
|:--------:|:-----------:|:-------------------:|:---------:|:------------:|:--------:|
|  `float` |   `float`   |  `H5T_NATIVE_FLOAT` | `Float32` | `np.float32` | `single` |
| `double` |   `double`  | `H5T_NATIVE_DOUBLE` | `Float64` | `np.float64` | `double` |
|  `char`  |  `cl_char`  |  `H5T_NATIVE_INT8`  |   `Int8`  |   `np.int8`  |  `int8`  |
|  `uchar` |  `cl_uchar` |  `H5T_NATIVE_UINT8` |  `UInt8`  |  `np.uint8`  |  `uint8` |
|  `short` |  `cl_short` |  `H5T_NATIVE_INT16` |  `Int16`  |  `np.int16`  |  `int16` |
| `ushort` | `cl_ushort` | `H5T_NATIVE_UINT16` |  `UInt16` |  `np.uint16` | `uint16` |
|   `int`  |   `cl_int`  |  `H5T_NATIVE_INT32` |  `Int32`  |  `np.int32`  |  `int32` |
|  `uint`  |  `cl_uint`  | `H5T_NATIVE_UINT32` |  `UInt32` |  `np.int32`  | `uint32` |
|  `long`  |  `cl_long`  |  `H5T_NATIVE_INT64` |  `Int64`  |  `np.int64`  |  `int64` |
|  `ulong` |  `cl_ulong` | `H5T_NATIVE_UINT64` |  `UInt64` |  `np.uint64` | `uint64` |

The input data should be given in the directory `/Data` of the input HDF5 file
in the same order as used in the OpenCL kernels.


## Date and Time

There are basically two formats for date/time output:
[ISO 8601](https://en.wikipedia.org/wiki/ISO_8601) strings for single pieces of
information such as `/housekeeping/kernel_execution_start`
and `double` values representing seconds.

Durations such as `housekeeping/kernel_execution_time` are given directly in
seconds. Time stamps used for power/temperature logging are given as POSIX UTC
time in seconds since 1970-01-01T00:00.000 with a resolution of microseconds.

Here are some examples using high level languages to read timestamps.
- Julia

  You need the package [HDF5.jl](https://github.com/JuliaIO/HDF5.jl) and the
  standard library package [Dates.jl](https://docs.julialang.org/en/v1/stdlib/Dates/index.html).
  ```julia
  julia> using Dates, HDF5

  julia> rawtime = h5open("out_repetition_test.h5", "r") do io
            read(io, "/housekeeping/nvidia/power_time")
        end
  10-element Array{Float64,1}:
  1.552232558449e9
  1.552232559451e9
  1.552232560453e9
  1.552232561455e9
  1.552232562457e9
  1.55223256346e9
  1.552232564462e9
  1.552232565464e9
  1.552232566466e9
  1.552232567468e9

  julia> time = unix2datetime.(rawtime)
  10-element Array{DateTime,1}:
  2019-03-10T15:42:38.449
  2019-03-10T15:42:39.451
  2019-03-10T15:42:40.453
  2019-03-10T15:42:41.455
  2019-03-10T15:42:42.457
  2019-03-10T15:42:43.46
  2019-03-10T15:42:44.462
  2019-03-10T15:42:45.464
  2019-03-10T15:42:46.466
  2019-03-10T15:42:47.468
  ```

- Python

  You need `h5py` and `numpy`.
  Documentation for datetimes: https://docs.scipy.org/doc/numpy/reference/arrays.datetime.html.
  ```python
  import numpy as np
  import h5py

  with h5py.File("out_repetition_test.h5", "r") as io:
    rawtime = io["housekeeping/nvidia/power_time"][:]

  time = (1.e3 * rawtime).astype("<M8[ms]")
  ```
  ```python
  array(['2019-03-10T15:42:38.449', '2019-03-10T15:42:39.451',
        '2019-03-10T15:42:40.453', '2019-03-10T15:42:41.455',
        '2019-03-10T15:42:42.457', '2019-03-10T15:42:43.460',
        '2019-03-10T15:42:44.462', '2019-03-10T15:42:45.464',
        '2019-03-10T15:42:46.466', '2019-03-10T15:42:47.468'],
        dtype='datetime64[ms]')
  ```

- Matlab

  Documentation: https://de.mathworks.com/help/matlab/ref/datetime.html and https://de.mathworks.com/help/matlab/ref/datetime.posixtime.html.
  ```matlab
  >> rawtime = h5read('out_repetition_test.h5', '/housekeeping/nvidia/power_time');
  >> time = datetime(rawtime, 'ConvertFrom', 'posixtime')

  time =

    10×1 datetime array

      10-Mar-2019 15:42:38
      10-Mar-2019 15:42:39
      10-Mar-2019 15:42:40
      10-Mar-2019 15:42:41
      10-Mar-2019 15:42:42
      10-Mar-2019 15:42:43
      10-Mar-2019 15:42:44
      10-Mar-2019 15:42:45
      10-Mar-2019 15:42:46
      10-Mar-2019 15:42:47
  ```
