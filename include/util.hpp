/* TODO: Provide a license note */

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <vector>
#include <algorithm>

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>


#if defined(_WIN32)
#include <windows.h>
#include <io.h>
typedef cl_ulong uint64_t;
typedef unsigned int uint;
#else
#include <stdint.h>
#include <unistd.h>
#endif


//TODO: unify all FileExists methods and provide them in one place instead of 3...
inline bool FileExists(const std::string &Filename)
{
  return access(Filename.c_str(), 0) == 0;
}

char* getCmdOption(char** begin, char** end, const std::string & option)
{
  char** itr = std::find(begin, end, option);
  if (itr != end && ++itr != end)  {
    return *itr;
  }
  return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
  return std::find(begin, end, option) != end;
}


#endif // UTIL_H
