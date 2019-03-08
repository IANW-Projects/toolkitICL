# toolkitICL

[![License](https://licensebuttons.net/l/by-nc-nd/3.0/88x31.png)](https://creativecommons.org/licenses/by-nc-nd/4.0/legalcode)
[![Build Status Appveyor](https://ci.appveyor.com/api/projects/status/js729s93vvdwgnjx?svg=true)](https://ci.appveyor.com/project/ranocha/toolkitICL)
[![Build Status Travis](https://travis-ci.com/IANW-Projects/toolkitICL.svg?branch=master)](https://travis-ci.com/IANW-Projects/toolkitICL)

`toolkitICL` is an open source tool for automated OpenCL kernel execution. It can be used as an easy
to use cross platform tool to execute a set of kernels for example on compute clusters, run automated
OpenCL benchmarks or test and validate kernels. HDF5 files are used for the entire configuration and
data handling. The list of kernels to be executed, all variables and the workgroup size is defined in
the input HDF5 file. After execution, the output data, data copy- and runtime is written to the
output HDF5 file. The GPU power consumption and temperature for supported Nvidia GPUs can also be logged automatically to the output HDF file.


## Setup
To build toolkitCL the following needs to be installed:
- OpenCL (headers and drivers)
- HDF5
- CMake
- CUDA Toolkit (only for NVidia GPU power/temperature logging)

This project uses the common CMake build system. Thus, the following commands can be used on Linux.
```bash
mkdir build && cd build
cmake .. # if you want to build only the main executable toolkitICL
make
```
If you want to build the tests, you can run
```bash
mkdir build && cd build
cmake -DTESTS=ON .. # if you want to build also the tests
make
make test
```
If you want to change the default compiler, the usual CMake workflow is supported, i.e. you can use something like
```bash
mkdir build && cd build
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
make
```

Recent versions of Visual Studio should automatically detect the cmake configuration files and build
toolkitICL automatically. Otherwise `cmake-gui` can be used on Windows systems to create Visul Studio
project files.

In case some of the OpenCL headers are missing, they can be obtained directly from Khronos:
[https://github.com/KhronosGroup](https://github.com/KhronosGroup). To automate this process,
you can add the command line switch `-DGET_CL2HPP=ON` to CMake.

On some compute cluster systems, it might be necessary to explicitly define the library path in the
cmake configuration files. If no ICD loader is available, the path to the OpenCL vendor library has
to be defined manually.

If the CUDA toolkit is detected, CMake will enable CUDA support for power and temperature logging automatically.
It can also be controlled manually (in the source code) using the `USENVML` define.


## Usage

Examples on how to create the input HDF5 files are provided in the directory
[`notebooks`](https://github.com/IANW-Projects/toolkitICL/tree/master/notebooks).

ToolkitICL can be controlled by the following command line options:
- `-d device_id`: Use the device specified by `device_id`.
- `-b`: Activate benchmark mode (minimal console logs, additional delay before & after runs).
- `-c config.h5`:  Specify the URL `config.h5` of the HDF5 configuration file.
- `-np sample_rate`: Log Nvidia GPU power consumption with sample_rate (ms).
- `-nt sample_rate`: Log Nvidia GPU temperature with sample_rate (ms).

Additional domumentation will be provided in the directory [`doc`](https://github.com/IANW-Projects/toolkitICL/tree/master/doc).

A useful tool to view and edit HDF5 files is [HDFView](https://www.hdfgroup.org/downloads/hdfview/).

## License

This project is licensed under the terms of the Creative Commons
[CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/legalcode) license.


## Disclaimer

Product and company names may be trademarks or registered trademarks of their respective holders.
Use of them does not imply any affiliation with or endorsement by them or their affiliates.
Everything is provided as is and without warranty. Use at your own risk!
