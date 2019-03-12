/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include <fstream>
#include <iostream>
#include <string>

#include "opencl_include.hpp"
#include "util.hpp"
#include "hdf5_io.hpp"


using namespace std;


int main(void)
{
  constexpr int LENGTH = 32;

  string filename{"output_test.h5"};

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


  // call toolkitICL
  string command("toolkitICL -c ");
  command.append(filename);
  int retval = system(command.c_str());
  if (retval) {
    cerr << "Error: " << retval << endl;
    return 1;
  }


  // check whether all necessary output is present
  string out_filename("out_");
  out_filename.append(filename);
  vector<cl_ulong> values_test(LENGTH);

  if (!fileExists(out_filename)) {
    cerr << "Error: File " << out_filename << " not found." << endl;
    return 1;
  }

  double Data_LoadTime = h5_read_single<double>(out_filename, "housekeeping/data_load_time");
  cout << "data_load_time         = " << Data_LoadTime << endl;

  double Data_StoreTime = h5_read_single<double>(out_filename, "housekeeping/data_store_time");
  cout << "data_store_time        = " << Data_StoreTime << endl;

  double Kernel_ExecTime = h5_read_single<double>(out_filename, "/housekeeping/kernel_execution_time");
  cout << "kernel_execution_time  = " << Kernel_ExecTime << endl;

  double Total_ExecTime = h5_read_single<double>(out_filename, "/housekeeping/total_execution_time");
  cout << "total_execution_time   = " << Total_ExecTime << endl;

  string Kernel_ExecStart;
  h5_read_string(out_filename, "/housekeeping/kernel_execution_start", Kernel_ExecStart);
  cout << "kernel_execution_start = " << Kernel_ExecStart << endl;

  string Kernel_Settings;
  h5_read_string(out_filename, "settings/kernel_settings", Kernel_Settings);
  cout << "kernel_settings        = " << Kernel_Settings << endl;

  cl_int global_range[3];
  h5_read_buffer<cl_int>(out_filename, "settings/global_range", &global_range[0]);
  cout << "global_range = (" << global_range[0]
                     << ", " << global_range[1]
                     << ", " << global_range[2] << ")" << endl;

  cl_int local_range[3];
  h5_read_buffer<cl_int>(out_filename, "/settings/local_range", &local_range[0]);
  cout << "local_range  = (" << local_range[0]
                     << ", " << local_range[1]
                     << ", " << local_range[2] << ")" << endl;

  cl_int range_start[3];
  h5_read_buffer<cl_int>(out_filename, "/settings/range_start", &range_start[0]);
  cout << "range_start  = (" << range_start[0]
                     << ", " << range_start[1]
                     << ", " << range_start[2] << ")" << endl;

  string host_os;
  h5_read_string(out_filename, "architecture/host_os", host_os);
  cout << "host_os         = " << host_os << endl;

  string opencl_device;
  h5_read_string(out_filename, "/architecture/opencl_device", opencl_device);
  cout << "opencl_device   = " << opencl_device << endl;

  string opencl_platform;
  h5_read_string(out_filename, "/architecture/opencl_platform", opencl_platform);
  cout << "opencl_platform = " << opencl_platform << endl;

  string opencl_version;
  h5_read_string(out_filename, "/architecture/opencl_version", opencl_version);
  cout << "opencl_version  = " << opencl_version << endl;

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
