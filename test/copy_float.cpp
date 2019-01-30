/* TODO: Provide a license note */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#else
#include <stdint.h>
#include <unistd.h>
#endif

#include "../include/hdf5_io.hpp"

using namespace std;


inline bool FileExists(const std::string &filename)
{
  return access(filename.c_str(), 0) == 0;
}


int main(void)
{
  constexpr int LENGTH = 32;

  string filename{"copy_float_test.h5"};

  if (FileExists(filename)) {
    remove(filename.c_str());
  }

  // kernel
  string kernel_url("copy_kernel.cl");
  ofstream kernel_file;
  kernel_file.open(kernel_url);
  kernel_file << " kernel void copy(global COPYTYPE const* in, global COPYTYPE* out) \
    { const int gid = get_global_id(0); out[gid] = in[gid]; } " << endl;
  kernel_file.close();

  h5_write_string(filename.c_str(), "Kernel_Settings", "-DCOPYTYPE=float");
  h5_write_string(filename.c_str(), "Kernel_URL", kernel_url.c_str());
  vector<string> kernels(1, string("copy"));
  h5_write_strings(filename.c_str(), "Kernels", kernels);

  // ranges
  int tmp_range[3];
  tmp_range[0] = LENGTH; tmp_range[1] = 1; tmp_range[2] = 1;
  h5_write_buffer_int(filename.c_str(), "Global_Range", tmp_range, 3);

  tmp_range[0] = 0; tmp_range[1] = 0; tmp_range[2] = 0;
  h5_write_buffer_int(filename.c_str(), "Local_Range", tmp_range, 3);
  h5_write_buffer_int(filename.c_str(), "Range_Start", tmp_range, 3);

  // data
  vector<float> in(LENGTH);
  vector<float> out(LENGTH, 0);
  for (int i = 0; i < LENGTH; ++i) {
    in.at(i) = i;
  }

  h5_create_dir(filename.c_str(), "/Data");
  h5_write_buffer_float(filename.c_str(), "Data/in", &in[0], LENGTH);
  h5_write_buffer_float(filename.c_str(), "Data/out", &out[0], LENGTH);


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
  vector<float> in_test(LENGTH);
  vector<float> out_test(LENGTH);

  if (!FileExists(out_filename)) {
    cerr << "Error: File " << out_filename << " not found." << endl;
    return 1;
  }

  h5_read_buffer_float(out_filename.c_str(), "Data/in", &in_test[0]);
  if (in_test != in) {
    cerr << "Error: Result 'in' is not as expected." << endl;
    // for (int i = 0; i < LENGTH; ++i) {
    //   cout << in[i] << "\t" << in_test[i] << endl;
    // }
    return 1;
  }

  h5_read_buffer_float(out_filename.c_str(), "Data/out", &out_test[0]);
  if (out_test != in) { // copy_kernel copies in to out
    cerr << "Error: Result 'out' is not as expected." << endl;
    // for (int i = 0; i < LENGTH; ++i) {
    //   cout << in[i] << "\t" << out_test[i] << endl;
    // }
    return 1;
  }

  //TODO: possible cleanup?
  // if (FileExists(kernel_url)) {
  //   remove(kernel_url.c_str());
  // }
  // if (FileExists(filename)) {
  //   remove(filename.c_str());
  // }
  // if (FileExists(out_filename)) {
  //   remove(out_filename.c_str());
  // }

  return 0;
}
