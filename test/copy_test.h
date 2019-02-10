/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#ifndef COPY_TEST_H
#define COPY_TEST_H

#include <fstream>
#include <iostream>
#include <string>

#include "opencl_include.hpp"
#include "util.hpp"
#include "hdf5_io.hpp"


using namespace std;


int runtest(void)
{
  constexpr int LENGTH = 32;

  string filename{"copy_" STRINGIZE(COPYTYPE_CL) "_test.h5"};

  if (fileExists(filename)) {
    remove(filename.c_str());
  }

  // kernel
  string kernel_url("copy_kernel.cl");
  ofstream kernel_file;
  kernel_file.open(kernel_url);
  kernel_file << "\n\
#ifdef cl_khr_fp64\n\
  #pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\
#else\n\
  #error \"IEEE-754 double precision not supported by OpenCL implementation.\"\n\
#endif\n\
\n\
kernel void copy(global COPYTYPE const* in, global COPYTYPE* out)\n\
{\n\
  const int gid = get_global_id(0);\n\
  out[gid] = in[gid];\n\
}\n\
" << endl;
  kernel_file.close();

  h5_write_string(filename.c_str(), "Kernel_Settings", "-DCOPYTYPE=" STRINGIZE(COPYTYPE_CL));
  h5_write_string(filename.c_str(), "Kernel_URL", kernel_url.c_str());
  vector<string> kernels(1, string("copy"));
  h5_write_strings(filename.c_str(), "Kernels", kernels);

  // ranges
  cl_int tmp_range[3];
  tmp_range[0] = LENGTH; tmp_range[1] = 1; tmp_range[2] = 1;
  h5_write_buffer<cl_int>(filename.c_str(), "Global_Range", tmp_range, 3);

  tmp_range[0] = 0; tmp_range[1] = 0; tmp_range[2] = 0;
  h5_write_buffer<cl_int>(filename.c_str(), "Local_Range", tmp_range, 3);
  h5_write_buffer<cl_int>(filename.c_str(), "Range_Start", tmp_range, 3);

  // data
  vector<COPYTYPE> in(LENGTH);
  vector<COPYTYPE> out(LENGTH, 0);
  for (int i = 0; i < LENGTH; ++i) {
    in.at(i) = i;
  }

  h5_create_dir(filename.c_str(), "/Data");
  h5_write_buffer<COPYTYPE>(filename.c_str(), "Data/in", &in[0], LENGTH);
  h5_write_buffer<COPYTYPE>(filename.c_str(), "Data/out", &out[0], LENGTH);

  // single values
  COPYTYPE single_value(21);
  h5_write_single<COPYTYPE>(filename.c_str(), "Single_Value", single_value);

  if (single_value != h5_read_single<COPYTYPE>(filename.c_str(), "Single_Value")) {
    cerr << "Error: Result 'Single_Value' is not as expected." << endl;
    return 1;
  }


  // call toolkitICL
  string command("toolkitICL -c ");
  command.append(filename);
  int retval = system(command.c_str());
  if (retval) {
    cerr << "Error: " << retval << endl;
    return 1;
  }


  // check result
  string out_filename("out_");
  out_filename.append(filename);
  vector<COPYTYPE> in_test(LENGTH);
  vector<COPYTYPE> out_test(LENGTH);

  if (!fileExists(out_filename)) {
    cerr << "Error: File " << out_filename << " not found." << endl;
    return 1;
  }

  h5_read_buffer<COPYTYPE>(out_filename.c_str(), "Data/in", &in_test[0]);
  if (in_test != in) {
    cerr << "Error: Result 'in' is not as expected." << endl;
    // for (int i = 0; i < LENGTH; ++i) {
    //   cout << in[i] << "\t" << in_test[i] << endl;
    // }
    return 1;
  }

  h5_read_buffer<COPYTYPE>(out_filename.c_str(), "Data/out", &out_test[0]);
  if (out_test != in) { // copy_kernel copies in to out
    cerr << "Error: Result 'out' is not as expected." << endl;
    // for (int i = 0; i < LENGTH; ++i) {
    //   cout << in[i] << "\t" << out_test[i] << endl;
    // }
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


#endif // COPY_TEST_H
