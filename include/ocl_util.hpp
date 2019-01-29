/* TODO: Procide a license note */

#ifndef OCL_UTIL_H
#define OCL_UTIL_H

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <vector>

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>

DREAL4 operator *(const DREAL4& lhs, const DREAL4& rhs) {
  DREAL4 result;

  result.x = lhs.x * rhs.x;
  result.y = lhs.y * rhs.y;
  result.z = lhs.z * rhs.z;
  result.w = lhs.w * rhs.w;

  return result;
}


DREAL4 operator +(const DREAL4& lhs,const DREAL4& rhs) {
  DREAL4 result;

  result.x = lhs.x + rhs.x;
  result.y = lhs.y + rhs.y;
  result.z = lhs.z + rhs.z;
  result.w = lhs.w + rhs.w;

  return result;
}


//Thread function to compile OpenCL Kernel asynchronously
void compile_kernel(cl::Program& cl_prog, char* options)
{
  std::stringstream default_options;
  default_options.setf(std::ios::fixed);
  default_options << " -cl-fast-relaxed-math";
  default_options << " -cl-single-precision-constant";
  default_options << " -DREAL4=" << STRINGIZE(CL_REAL) << "4"; //define REAL as alias for float/double as defined in main.hpp
  default_options << " -DREAL="  << STRINGIZE(CL_REAL);
  default_options << options;


  try {
    cl_prog.build(default_options.str().c_str());
  }
  catch (cl::BuildError error) {
    std::string log = error.getBuildLog()[0].second;
    std::cerr << std::endl << "Build error:" << std::endl << log << std::endl;
  }
  catch (cl::Error err) {
    std::cerr << "Exception:" << std::endl << "ERROR: " << err.what() << std::endl;
  }
}

int nextMul(int numToRound, int multiple)
{
  if(multiple == 0) {
    return numToRound;
  }

  int remainder = numToRound % multiple;
  if (remainder == 0) {
    return numToRound;
  }

  return numToRound + multiple - remainder;
}

#endif // OCL_UTIL_H
