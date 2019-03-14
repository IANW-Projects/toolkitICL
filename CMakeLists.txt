# Select compiler, supports gcc, clang
cmake_minimum_required (VERSION 3.5)

PROJECT(toolkitICL)

option(BUILDREL "Official release build" OFF)

if(DEFINED ENV{AMDPROFILERPATH})
    message(STATUS "Found AMD profiling driver")
    option(USEAMDP "Use AMD Profiling" ON)
    add_definitions(-DUSEAMDP)
ELSE()
    option(USEAMDP "Use AMD Profiling" OFF)
endif()

find_package(CUDA)

IF(CUDA_FOUND)
  option(USENVML "Use NVML" ON)
ELSE()
  option(USENVML "Use NVML" OFF)
ENDIF()

IF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
option(USEIRAPL "Use RAPL" OFF)
ELSE()
option(USEIRAPL "Use RAPL" ON)
ENDIF()

if(DEFINED ENV{IPG_Dir})
    message(STATUS "Found Intel Power Gadget Driver")
	option(USEIPG "Use Intel Power Gadget" ON)
	option(USEIRAPL "Use RAPL" OFF)
ELSE()
	option(USEIPG "Use Intel Power Gadget" OFF)
endif()

IF(USEIPG)
  MESSAGE(STATUS "Using Intel Power Gadget")
  add_definitions(-DUSEIPG)
ENDIF()


IF(USEIRAPL)
  MESSAGE(STATUS "Using Intel RAPL")
  add_definitions(-DUSEIRAPL)
ENDIF()

IF(USENVML)
  MESSAGE(STATUS "Using NVML")
  add_definitions(-DUSENVML)
ENDIF()

set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)

SET(CMAKE_VERBOSE_MAKEFILE "false")
# Includes modules to find OCL
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")

# compiler flags
IF(BUILDREL)
MESSAGE(STATUS "Building release version")
IF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  MESSAGE(STATUS "Microsoft Visual Studio detected. Corresponding compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB")
  SET(CMAKE_C_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  MESSAGE(STATUS "GNU Compiler Collection detected. Corresponding compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " -std=c++11 -O2 -funroll-loops -fstrict-aliasing -msse2 -fPIC -Wall -mfpmath=sse -Wcast-align -Wl,-E")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  MESSAGE(STATUS "LLVM Clang detected.  Corresponding compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " -std=c++11 -O2 -funroll-loops -fstrict-aliasing -msse2 -fPIC -Wall")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  MESSAGE(STATUS "Apple Clang detected.  Corresponding compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " -std=c++11 -O2 -funroll-loops -fstrict-aliasing -msse2 -fPIC -Wall")
ELSE()
  MESSAGE(STATUS "Compiler ${CMAKE_CXX_COMPILER_ID} unknown. Simple compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " -std=c++11 -O2 -fstrict-aliasing -fPIC -Wall")
  ENDIF()
ELSE(BUILDREL)
IF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  MESSAGE(STATUS "Microsoft Visual Studio detected. Corresponding compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB")
  SET(CMAKE_C_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  MESSAGE(STATUS "GNU Compiler Collection detected. Corresponding compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " -std=c++11 -O2 -funroll-loops -fstrict-aliasing -march=native -fPIC -Wall -mfpmath=sse -Wcast-align -Wl,-E")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  MESSAGE(STATUS "LLVM Clang detected.  Corresponding compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " -std=c++11 -O2 -funroll-loops -fstrict-aliasing -march=native -fPIC -Wall")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  MESSAGE(STATUS "Apple Clang detected.  Corresponding compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " -std=c++11 -O2 -funroll-loops -fstrict-aliasing -march=native -fPIC -Wall")
ELSE()
  MESSAGE(STATUS "Compiler ${CMAKE_CXX_COMPILER_ID} unknown. Simple compiler flags have been set.")
  SET(CMAKE_CXX_FLAGS " -std=c++11 -O2 -fstrict-aliasing -march=native -fPIC -Wall")
ENDIF()
ENDIF(BUILDREL)

# Get cl2.hpp header file
OPTION(GET_CL2HPP "Download cl2.hpp from Khronos" OFF)
SET(CL2_URL https://github.com/KhronosGroup/OpenCL-CLHPP/releases/download/v2.0.10/cl2.hpp)

IF(GET_CL2HPP)
  # download to default directory
  FILE(DOWNLOAD ${CL2_URL} ${CMAKE_CURRENT_SOURCE_DIR}/include/CL/cl2.hpp)
ENDIF()

IF(APPLE)
  # Apple does not distribute a cl2.hpp include file and uses another default path for OpenCL
  FILE(DOWNLOAD ${CL2_URL} ${CMAKE_CURRENT_SOURCE_DIR}/include/OpenCL/cl2.hpp)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework OpenCL")
ENDIF()


# NVML: NVIDIA Management Library, used to query GPU power and temperature
IF(USENVML)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CUDA_TOOLKIT_INCLUDE})
ELSE()
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
ENDIF(USENVML)

# Check for HDF5
find_package(HDF5 REQUIRED COMPONENTS CXX HL)
IF(HDF5_FOUND)
  MESSAGE(STATUS "Looking for HDF5 - found at\n   ${HDF5_INCLUDE_DIRS}\n   ${HDF5_LIBRARIES}\n   ${HDF5_HL_LIBRARIES}")
ELSE(HDF5_FOUND)
  MESSAGE(STATUS "Looking for HDF5 - not found!")
ENDIF(HDF5_FOUND)

# Check for OpenCL
find_package(OpenCL REQUIRED)
IF(OpenCL_FOUND)
  MESSAGE(STATUS "Looking for OpenCL - found at\n   ${OpenCL_INCLUDE_DIRS}\n   ${OpenCL_LIBRARY}")
ELSE(OpenCL_FOUND)
  MESSAGE(STATUS "Looking for OpenCL - not found")
ENDIF(OpenCL_FOUND)

# toolkitICL
ADD_SUBDIRECTORY(src)

# tests
OPTION(TESTS "Build unit tests" OFF)
IF(TESTS)
  enable_testing()
  ADD_SUBDIRECTORY(test)
ENDIF()
