/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "opencl_include.hpp"
#include "util.hpp"
#include "hdf5_io.hpp"


using namespace std;


int main(void)
{
  constexpr int LENGTH = 32;

  string filename{"parsing_test.h5"};

  if (fileExists(filename)) {
    remove(filename.c_str());
  }

  // kernel
  string kernel_url("add_one_kernel.cl");
  ofstream kernel_file;
  kernel_file.open(kernel_url);
  kernel_file << "\n\
#ifdef cl_khr_fp64\n\
  #pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\
#else\n\
  #error \"IEEE-754 double precision not supported by OpenCL implementation.\"\n\
#endif\n\
\n\
kernel void add_one(global REAL* values)\n\
{\n\
  const int gid = get_global_id(0);\n\
  values[gid] += 1;\n\
}\n\
" << endl;
  kernel_file.close();

  h5_create_dir(filename, "/settings");
  h5_write_string(filename, "/settings/kernel_settings", "-DREAL=ulong");
  h5_write_string(filename, "kernel_url", kernel_url.c_str());
  vector<string> kernels(1, string("add_one"));
  h5_write_strings(filename, "kernels", kernels);
  cl_ulong kernel_repetitions = 10000;
  h5_write_single<cl_ulong>(filename, "kernel_repetitions", kernel_repetitions);

  // ranges
  cl_int tmp_range[3];
  tmp_range[0] = LENGTH; tmp_range[1] = 1; tmp_range[2] = 1;
  h5_write_buffer<cl_int>(filename, "settings/global_range", tmp_range, 3);

  tmp_range[0] = 0; tmp_range[1] = 0; tmp_range[2] = 0;
  h5_write_buffer<cl_int>(filename, "settings/local_range", tmp_range, 3);
  h5_write_buffer<cl_int>(filename, "settings/range_start", tmp_range, 3);

  // data
  vector<cl_ulong> values(LENGTH);
  for (cl_ulong i = 0; i < LENGTH; ++i) {
    values.at(i) = i;
  }

  h5_create_dir(filename, "/data");
  h5_write_buffer<cl_ulong>(filename, "data/values", &values[0], LENGTH);


  // Parsing Filename Test
  cout << "Parsing Filename Test" << endl;
  ostringstream command;
#if defined(_WIN32)
  char sep = '\\';
#else
  char sep = '/';
#endif

  command << "toolkitICL -c ." << sep << filename;
  int retval = system(command.str().c_str());
  if (retval) {
    cerr << "Error: " << retval << endl;
    return 1;
  }
  string out_filename = filename;
  out_filename.insert(0, "out_");
  if (!fileExists(out_filename)) {
    cerr << "Error: No output generated. Expected " << out_filename << endl;
    return 1;
  }


  // Unknown Command Line Arguments Test
  cout << "Unknown Command Line Arguments Test" << endl;
  ostringstream().swap(command);
  command << "toolkitICL -what_is_the_answer_to_life_the_universe_and_everything 42 -c " << filename;
  retval = system(command.str().c_str());
  if (!retval) {
    cerr << "Error: Unknown command line argument specified; toolkitICL should throw an error." << endl;
    return 1;
  }


  // Wrong Format of Command Line Arguments Test
  cout << "Wrong Format of Command Line Arguments Test" << endl;
  ostringstream().swap(command);
  command << "toolkitICL -d asd213 -c " << filename;
  retval = system(command.str().c_str());
  if (!retval) {
    cerr << "Error: Wrong command line argument specified; toolkitICL should throw an error." << endl;
    return 1;
  }


  //TODO: possible cleanup?
  // if (fileExists(kernel_url)) {
  //   remove(kernel_url.c_str());
  // }
  // if (fileExists(filename)) {
  //   remove(filename.c_str());
  // }
  // if (fileExists(out_filename)) {
  //   remove(out_filename.c_str());
  // }

  return 0;
}
