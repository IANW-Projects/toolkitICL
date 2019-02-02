/* TODO: Provide a license note */

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

#define ERROR_INFO "Error in line " STRINGIZE(__LINE__) " of " __FILE__ ":\n "


#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>


#endif // MAIN_H
