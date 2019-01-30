# Select compiler, supports gcc, clang

cmake_minimum_required (VERSION 3.5)
PROJECT(toolkitICL)


if (COMPILER STREQUAL "GCC")
  find_program(CMAKE_C_COMPILER NAMES gcc)
  find_program(CMAKE_CXX_COMPILER NAMES g++)
elseif (COMPILER STREQUAL "CLANG")
  set (CMAKE_C_COMPILER   "clang")
  set (CMAKE_CXX_COMPILER "clang++")
  find_program(CMAKE_AR NAMES llvm-ar)
  find_program(CMAKE_LINKER NAMES llvm-ld)
endif ()

# Because Makefiles are shit to read
SET(CMAKE_VERBOSE_MAKEFILE "false")
# Includes modules to find OCL
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")

if (MSVC)
MESSAGE(STATUS "MSVC++ detected")
set (CMAKE_CXX_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB" )
set (CMAKE_C_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB" )
elseif (COMPILER STREQUAL "GCC")
  set (CMAKE_CXX_FLAGS " -std=c++11 -O2 -funroll-loops -fstrict-aliasing -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2 -fPIC -Wall -mfpmath=sse -Wcast-align -Wl,-E")
elseif (COMPILER STREQUAL "CLANG")
  set (CMAKE_CXX_FLAGS " -std=c++11 -O2 -funroll-loops -fstrict-aliasing -msse2 -msse3 -mssse3 -msse4.1 -msse4.2  -mavx -mavx2 -fPIC -Wall")
endif ()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)


# Check for HDF5
find_package(HDF5 REQUIRED COMPONENTS CXX HL)
IF(HDF5_FOUND)
    MESSAGE(STATUS ${HDF5_INCLUDE_DIRS})
ELSE(HDF5_FOUND)
    MESSAGE(STATUS "HDF5 not found!")
ENDIF(HDF5_FOUND)

# Check for OpenCL
find_package(OpenCL REQUIRED)
IF(OpenCL_FOUND)
  MESSAGE(STATUS "Looking for OpenCL - found at ${OpenCL_INCLUDE_DIRS} and ${OpenCL_LIBRARY}")
ELSE(OpenCL_FOUND)
  MESSAGE(STATUS "Looking for OpenCL - not found")
ENDIF(OpenCL_FOUND)

ADD_SUBDIRECTORY(src)
