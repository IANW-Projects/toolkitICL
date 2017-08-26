#ifndef MAIN_H
#define MAIN_H


//some headers seem to def strict ansi, which breaks the OpenCL typedefs
#if defined( __STRICT_ANSI__ )
#undef  __STRICT_ANSI__
#endif

//disable strange warnings for newer versions of GCC
#pragma GCC diagnostic ignored "-Wignored-attributes"

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>

#define USE_DOUBLE 0

#if USE_DOUBLE
#define DREAL cl_double
#define DREAL4 cl_double4
#define CL_REAL double
#undef USE_OPENGL
#else
#define DREAL cl_float
#define DREAL4 cl_float4
#define CL_REAL float
#undef USE_OPENGL 
#endif    


#endif // MAIN_H
