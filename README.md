# toolkitICL

[![License](https://licensebuttons.net/l/by-nc-nd/3.0/88x31.png)](https://creativecommons.org/licenses/by-nc-nd/4.0/legalcode)
[![Build Status Appveyor](https://ci.appveyor.com/api/projects/status/js729s93vvdwgnjx?svg=true)](https://ci.appveyor.com/project/ranocha/toolkitICL)
[![Build Status Travis](https://travis-ci.com/IANW-Projects/toolkitICL.svg?branch=master)](https://travis-ci.com/IANW-Projects/toolkitICL)

`toolkitICL` is an open source tool for automated OpenCL kernel execution. It can be used as an easy
to use cross platform tool to execute a set of kernels for example on compute clusters, run automated
OpenCL benchmarks or test and validate kernels. HDF5 files are used for the entire configuration and
data handling. The list of kernels to be executed, all variables and the workgroup size is defined in
the input HDF5 file. After execution, the output data, data copy- and runtime is written to the
output HDF5 file.

## Usage
To use toolkitCL the following needs to be installed:
- OpenCL (headers and drivers)
- HDF5
- CMake

In case some of the OpenCL headers are missing, they can be obtained directly from Khronos:
[https://github.com/KhronosGroup](https://github.com/KhronosGroup).
On some compute cluster systems, it might be necessary to explicitly define the library path in the
cmake configuration files. If no icd loader is available, the path to the OpenCL vendor library hast
to be defined manually.

To compile the code using Linux, create a build directory and run `cmake ..` followed by `make`.
Recent versions of Visual Studio should automatically detect the cmake configuration files and build
toolkitICL automatically. Otherwise `cmake-gui` can be used on Windows systems to create Visul Studio
project files.

Examples on how to create the input HDF5 files are provided in the directory
[`notebooks`](https://github.com/IANW-Projects/toolkitICL/tree/master/notebooks).

ToolkitICL can be controlled by the following command line options:
- `-d device_id`: Use the device specified by `device_id`.
- `-b`: Activate benchmark mode (minimal console logs, additional delay before & after runs).
- `-c config.h5`:  Specify the URL `config.h5` of the HDF5 configuration file.

A useful tool to view and edit HDF5 files is [HDFView](https://www.hdfgroup.org/downloads/hdfview/).

## License

This project is licensed under the terms of the Creative Commons
[CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/legalcode) license.


## Disclaimer

Product and company names may be trademarks or registered trademarks of their respective holders.
Use of them does not imply any affiliation with or endorsement by them or their affiliates.
Everything is provided as is and without warranty. Use at your own risk!
