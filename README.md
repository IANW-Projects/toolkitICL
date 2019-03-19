# toolkitICL

[![License](https://licensebuttons.net/l/by-nc-nd/3.0/88x31.png)](https://creativecommons.org/licenses/by-nc-nd/4.0/legalcode)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.2597653.svg)](https://doi.org/10.5281/zenodo.2597653)
[![Build Status Appveyor](https://ci.appveyor.com/api/projects/status/js729s93vvdwgnjx?svg=true)](https://ci.appveyor.com/project/ranocha/toolkitICL)
[![Build Status Travis](https://travis-ci.com/IANW-Projects/toolkitICL.svg?branch=master)](https://travis-ci.com/IANW-Projects/toolkitICL)

`toolkitICL` is an open source tool for automated OpenCL kernel execution. It can be used as an easy
to use cross platform tool to execute a set of kernels for example on compute clusters, run automated
OpenCL benchmarks or test and validate kernels. HDF5 files are used for the entire configuration and
data handling. The list of kernels to be executed, all variables and the workgroup size is defined in
the input HDF5 file. After execution, the output data, data copy- and runtime is written to the
output HDF5 file. The power consumption and temperature for supported Nvidia GPUs and Intel CPUs/GPUS can also be logged.
Which housekeeping data is logged during execution can be selected by individual command line options.


## Setup
To build toolkitCL the following needs to be installed:
- OpenCL (headers and drivers)
- HDF5
- CMake

For optional power and temperature logging, the following is needed:
- CUDA Toolkit (only for NVidia GPU power/temperature logging)
- Intel Power Gadget (only for Intel CPU/GPU power/temperature logging on Windows systems)
- AMD µProf (only for AMD CPU/GPU power/temperature logging on Windows and Linux systems)

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

On Linux based systems, Intel CPU power consumption can be read directly from the model-specific registers (MSR).
To enable MSR support the `msr-tools` package is needed on Debian based systems.
It might be necesarry to set the permissions `sudo chmod o+rw /dev/cpu/0/msr`.
Depending on the architecture, the power consumption is determined by the system based on a heuristic algorithm and not measured directly.

If the Intel Power Gadget API is detected (Windows only), CMake will enable power and temperature logging for supported Intel CPUs and GPUs automatically.
During execution, power and temperture loggin is only available, if the `EnergyLib64.dll` is found.
Depending on the architecture, the power consumption is determined by the system based on a heuristic algorithm and not measured directly.
The necessary drivers and additional information are available at [Intel]( https://software.intel.com/en-us/articles/intel-power-gadget-20).

The default path will be used to search for the necessary libraries (HDF5, Intel Power Gadget, AMD µProf, CUDA). It is also possible to place the libraries together with the executable in the same folder.

## Usage

Examples on how to create the input HDF5 files are provided in the directory
[`notebooks`](https://github.com/IANW-Projects/toolkitICL/tree/master/notebooks).

ToolkitICL can be controlled by the following command line options:
- `-d device_id`: Use the device specified by `device_id`.
- `-b`: Activate benchmark mode (minimal console logs, additional delay before & after runs).
- `-c config.h5`:  Specify the URL `config.h5` of the HDF5 configuration file.
- `-nvidia_power sample_rate`: Log Nvidia GPU power consumption with `sample_rate` (ms).
- `-nvidia_temp sample_rate`: Log Nvidia GPU temperature with `sample_rate` (ms).
- `-intel_power sample_rate`: Log Intel system power consumption with `sample_rate` (ms).
- `-intel_temp sample_rate`: Log Intel CPU temperature with `sample_rate` (ms).
- `-amd_cpu_power sample_rate`: Log AMD CPU power consumption with `sample_rate` (ms).
- `-amd_cpu_temp sample_rate`: Log AMD CPU temperature with `sample_rate` (ms).

Additional domumentation will be provided in the directory [`doc`](https://github.com/IANW-Projects/toolkitICL/tree/master/doc).

A useful tool to view and edit HDF5 files is [HDFView](https://www.hdfgroup.org/downloads/hdfview/).


## Reference

This software can be cited as:
```
@misc{heinisch2019toolkitICL,
  title={{toolkitICL}. {A}n open source tool for automated {OpenCL} kernel execution.},
  author={Heinisch, Philip and Ostaszewski, Katharina and Ranocha, Hendrik},
  month={03},
  year={2019},
  howpublished={\url{https://github.com/IANW-Projects/toolkitICL}},
  doi={10.5281/zenodo.2597653}
}
```


## License

This project is licensed under the terms of the Creative Commons
[CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/legalcode) license.


## Disclaimer

Product and company names may be trademarks or registered trademarks of their respective holders.
Use of them does not imply any affiliation with or endorsement by them or their affiliates.
Everything is provided as is and without warranty. Use at your own risk!
