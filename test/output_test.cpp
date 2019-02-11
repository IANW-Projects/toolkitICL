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

  h5_write_string(filename.c_str(), "Kernel_Settings", "-DREAL=ulong");
  h5_write_string(filename.c_str(), "Kernel_URL", kernel_url.c_str());
  vector<string> kernels(1, string("add_one"));
  h5_write_strings(filename.c_str(), "Kernels", kernels);
  cl_ulong kernel_repetitions = 10000;
  h5_write_single<cl_ulong>(filename.c_str(), "Kernel_Repetitions", kernel_repetitions);

  // ranges
  cl_int tmp_range[3];
  tmp_range[0] = LENGTH; tmp_range[1] = 1; tmp_range[2] = 1;
  h5_write_buffer<cl_int>(filename.c_str(), "Global_Range", tmp_range, 3);

  tmp_range[0] = 0; tmp_range[1] = 0; tmp_range[2] = 0;
  h5_write_buffer<cl_int>(filename.c_str(), "Local_Range", tmp_range, 3);
  h5_write_buffer<cl_int>(filename.c_str(), "Range_Start", tmp_range, 3);

  // data
  vector<cl_ulong> values(LENGTH);
  for (cl_ulong i = 0; i < LENGTH; ++i) {
    values.at(i) = i;
  }

  h5_create_dir(filename.c_str(), "/Data");
  h5_write_buffer<cl_ulong>(filename.c_str(), "Data/values", &values[0], LENGTH);


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

  double Data_LoadTime = h5_read_single<double>(out_filename.c_str(), "Data_LoadTime");
  cout << "Data_LoadTime   = " << Data_LoadTime << endl;

  double Data_StoreTime = h5_read_single<double>(out_filename.c_str(), "Data_StoreTime");
  cout << "Data_StoreTime  = " << Data_StoreTime << endl;

  double Kernel_ExecTime = h5_read_single<double>(out_filename.c_str(), "Kernel_ExecTime");
  cout << "Kernel_ExecTime = " << Kernel_ExecTime << endl;

  double Total_ExecTime = h5_read_single<double>(out_filename.c_str(), "Total_ExecTime");
  cout << "Total_ExecTime  = " << Total_ExecTime << endl;

  string Kernel_ExecStart;
  h5_read_string(out_filename.c_str(), "Kernel_ExecStart", Kernel_ExecStart);
  cout << "Kernel_ExecStart = " << Kernel_ExecStart << endl;

  string Kernel_Settings;
  h5_read_string(out_filename.c_str(), "Kernel_Settings", Kernel_Settings);
  cout << "Kernel_Settings  = " << Kernel_Settings << endl;

  cl_int Global_Range[3];
  h5_read_buffer<cl_int>(out_filename.c_str(), "Global_Range", &Global_Range[0]);
  cout << "Global_Range = (" << Global_Range[0]
                     << ", " << Global_Range[1]
                     << ", " << Global_Range[2] << ")" << endl;

  cl_int Local_Range[3];
  h5_read_buffer<cl_int>(out_filename.c_str(), "Local_Range", &Local_Range[0]);
  cout << "Local_Range  = (" << Local_Range[0]
                     << ", " << Local_Range[1]
                     << ", " << Local_Range[2] << ")" << endl;

  cl_int Range_Start[3];
  h5_read_buffer<cl_int>(out_filename.c_str(), "Range_Start", &Range_Start[0]);
  cout << "Range_Start  = (" << Range_Start[0]
                     << ", " << Range_Start[1]
                     << ", " << Range_Start[2] << ")" << endl;

  string Host_OS;
  h5_read_string(out_filename.c_str(), "Host_OS", Host_OS);
  cout << "Host_OS = " << Host_OS << endl;

  string OpenCL_Device;
  h5_read_string(out_filename.c_str(), "OpenCL_Device", OpenCL_Device);
  cout << "OpenCL_Device  = " << OpenCL_Device << endl;

  string OpenCL_Version;
  h5_read_string(out_filename.c_str(), "OpenCL_Version", OpenCL_Version);
  cout << "OpenCL_Version = " << OpenCL_Version << endl;

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
