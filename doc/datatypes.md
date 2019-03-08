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
