/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#ifndef MAIN_H
#define MAIN_H


// some headers seem to def strict ansi, which breaks the OpenCL typedefs
#if defined( __STRICT_ANSI__ )
#undef  __STRICT_ANSI__
#endif

// disable strange warnings for newer versions of GCC for OpenCL typedefs
#pragma GCC diagnostic ignored "-Wignored-attributes"

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>


#endif // MAIN_H
