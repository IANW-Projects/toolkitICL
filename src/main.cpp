/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <thread>
#include <vector>

#include "opencl_include.hpp"
#include "util.hpp"
#include "hdf5_io.hpp"
#include "ocl_dev_mgr.hpp"
#include "timer.hpp"


using namespace std;


// command line arguments
char const* getCmdOption(char** begin, char** end, std::string const& option)
{
  char** itr = find(begin, end, option);
  if (itr != end && ++itr != end)  {
    return *itr;
  }
  return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
  return find(begin, end, option) != end;
}

void print_help()
{
  cout << "Usage: toolkitICL [options] -c config.h5" << endl
       << "Options:" << endl
       << "  -d device_id: " << "Use the device specified by `device_id`." << endl
       << "  -b          : " << "Activate the benchmark mode (additional delay before & after runs)." << endl
       << "  -c config.h5: " << "Specify the URL `config.h5` of the HDF5 configuration file." << endl
       << endl;
}


int main(int argc, char *argv[]) {

  Timer timer; //used to track performance

  cl_uint deviceIndex = 0; // set default OpenCL Device

  // parse command line arguments
  bool benchmark_mode = false;
  if (cmdOptionExists(argv, argv + argc, "-b"))  {
    benchmark_mode = true;
    cout << "Benchmark mode" << endl << endl;
  }

  if (cmdOptionExists(argv, argv + argc, "-d")) {
    char const* dev_id = getCmdOption(argv, argv + argc, "-d");
    deviceIndex = atoi(dev_id);
  }

  if (cmdOptionExists(argv, argv + argc, "-h") || !cmdOptionExists(argv, argv + argc, "-c")) {
    print_help();
    return 0;
  }
  char const* filename = getCmdOption(argv, argv + argc, "-c");


  ocl_dev_mgr& dev_mgr = ocl_dev_mgr::getInstance();
  cl_uint devices_availble=dev_mgr.get_avail_dev_num();

  cout << "Available devices: " << devices_availble << endl
       << dev_mgr.get_avail_dev_info(deviceIndex).name.c_str() << endl;
  cout << "OpenCL version: " << dev_mgr.get_avail_dev_info(deviceIndex).ocl_version.c_str() << endl;
  cout << "Memory limit: "<< dev_mgr.get_avail_dev_info(deviceIndex).max_mem << endl;
  cout << "WG limit: "<< dev_mgr.get_avail_dev_info(deviceIndex).wg_size << endl << endl;
  dev_mgr.init_device(deviceIndex);

  string kernel_url;
  if (h5_check_object(filename, "Kernel_URL") == true) {
	  h5_read_string(filename, "Kernel_URL", kernel_url);
	  cout << "Reading kernel from file: " << kernel_url << "... " << endl;
  }
  else if (h5_check_object(filename, "Kernel_Source") == true) {
    cout << "Reading kernel from HDF5 file... " << endl;
    std::vector<std::string> kernel_source;
    h5_read_strings(filename, "Kernel_Source", kernel_source);
    ofstream tmp_clfile;
    tmp_clfile.open("tmp_kernel.cl");
    for (string const& kernel : kernel_source) {
      tmp_clfile << kernel << endl;
    }

    tmp_clfile.close();
    kernel_url = string("tmp_kernel.cl");
  }
  else {
    cerr << "No kernel information found! " << endl;
    return -1;
  }

  std::vector<std::string> kernel_list;
  h5_read_strings(filename, "Kernels", kernel_list);

  dev_mgr.add_program_url(0, "ocl_Kernel", kernel_url);

	string settings;
  h5_read_string(filename, "Kernel_Settings", settings);


  uint64_t num_kernels_found = 0;
  num_kernels_found = dev_mgr.compile_kernel(0, "ocl_Kernel", settings);
  if (num_kernels_found == 0) {
    cerr << "Error: No valid kernels found" << endl;
    return -1;
  }

  std::vector<std::string> found_kernels;
  dev_mgr.get_kernel_names(0, "ocl_Kernel", found_kernels);
  cout << "Found Kernels: " << found_kernels.size() << endl;
  if (found_kernels.size() == 0) {
    cerr << "Error: No valid kernels found." << endl;
    return -1;
  }

  cout << "Number of Kernels to execute: " << kernel_list.size() << endl;

  //TODO: Clean up; debug mode?
  // for (uint32_t kernel_idx = 0; kernel_idx < kernel_list.size(); kernel_idx++) {
  //   cout <<"Found : "<< kernel_list.at(kernel_idx) << endl;
  // }

  cout << "Ingesting HDF5 config file..." << endl;

  std::vector<std::string> data_names;
  std::vector<HD5_Type> data_types;
  std::vector<size_t> data_sizes;
  h5_get_content(filename, "/Data/", data_names, data_types, data_sizes);

  cout << "Creating output HDF5 file..." << endl;
  string out_name = "out_" + string(filename);

  if (fileExists(out_name)) {
    remove(out_name.c_str());
    cout << "Old HDF5 data file found and deleted!" << endl;
  }

  h5_write_string(out_name.c_str(), "Kernel_Settings", settings);

  std::vector<cl::Buffer> data_in;
  bool blocking = CL_TRUE;

  //TODO: Implement functionality! Allow other integer types instead of cl_int?
  vector<cl_int> data_rw_flags(data_names.size(), 0);

  uint64_t push_time, pull_time;
  push_time = timer.getTimeMicroseconds();

  for(cl_uint i = 0; i < data_names.size(); i++) {
    try {
      uint8_t *tmp_data = nullptr;
      size_t var_size = 0;

      switch (data_types.at(i)) {
        case H5_float:
          var_size = data_sizes.at(i)*sizeof(float);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<float>(filename, data_names.at(i).c_str(), (float*)tmp_data);
          break;
        case H5_double:
          var_size = data_sizes.at(i)*sizeof(double);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<double>(filename, data_names.at(i).c_str(), (double*)tmp_data);
          break;
        case H5_char:
          var_size=data_sizes.at(i)*sizeof(cl_char);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<cl_char>(filename, data_names.at(i).c_str(), (cl_char*)tmp_data);
          break;
        case H5_uchar:
          var_size = data_sizes.at(i)*sizeof(cl_uchar);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<cl_uchar>(filename, data_names.at(i).c_str(), (cl_uchar*)tmp_data);
          break;
        case H5_short:
          var_size=data_sizes.at(i)*sizeof(cl_short);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<cl_short>(filename, data_names.at(i).c_str(), (cl_short*)tmp_data);
          break;
        case H5_ushort:
          var_size=data_sizes.at(i)*sizeof(cl_ushort);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<cl_ushort>(filename, data_names.at(i).c_str(), (cl_ushort*)tmp_data);
          break;
        case H5_int:
          var_size=data_sizes.at(i)*sizeof(cl_int);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<cl_int>(filename, data_names.at(i).c_str(), (cl_int*)tmp_data);
          break;
        case H5_uint:
          var_size=data_sizes.at(i)*sizeof(cl_uint);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<cl_uint>(filename, data_names.at(i).c_str(), (cl_uint*)tmp_data);
          break;
        case H5_long:
          var_size=data_sizes.at(i)*sizeof(cl_long);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<cl_long>(filename, data_names.at(i).c_str(), (cl_long*)tmp_data);
          break;
        case H5_ulong:
          var_size=data_sizes.at(i)*sizeof(cl_ulong);
          tmp_data = new uint8_t[var_size];
          h5_read_buffer<cl_ulong>(filename, data_names.at(i).c_str(), (cl_ulong*)tmp_data);
          break;
        default:
          cerr << ERROR_INFO << "Data type '" << data_types.at(i) << "' unknown." << endl;
          break;
      }

      switch (data_rw_flags.at(i)) {
        case 0:
          data_in.push_back(cl::Buffer(dev_mgr.get_context(0), CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, var_size));
          dev_mgr.get_queue(0, 0).enqueueWriteBuffer(data_in.back(), blocking, 0, var_size, tmp_data);
          break;
        case 1:
          data_in.push_back(cl::Buffer(dev_mgr.get_context(0), CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, var_size));
          dev_mgr.get_queue(0, 0).enqueueWriteBuffer(data_in.back(), blocking, 0, var_size, tmp_data);
          break;
        case 2:
          data_in.push_back(cl::Buffer(dev_mgr.get_context(0), CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, var_size));
          break;
      }

      for (uint32_t kernel_idx = 0; kernel_idx < found_kernels.size(); kernel_idx++) {
        dev_mgr.getKernelbyName(0, "ocl_Kernel", found_kernels.at(kernel_idx))->setArg(i, data_in.back());
      }

      if (tmp_data != nullptr) {
        delete[] tmp_data; tmp_data = nullptr;
      }
    }
    catch (cl::Error err) {
      std::cerr << ERROR_INFO << "Exception: " << err.what() << std::endl;
    }
  }

  dev_mgr.get_queue(0, 0).finish(); // Buffer Copy is asynchronous

  push_time = timer.getTimeMicroseconds() - push_time;

  cout << "Setting range..." << endl;

  cl::NDRange range_start;
  cl::NDRange global_range;
  cl::NDRange local_range;

  //TODO: Allow other integer types instead of cl_int?
  cl_int tmp_range[3];
  h5_read_buffer<cl_int>(filename, "Global_Range", tmp_range);
  global_range = cl::NDRange(tmp_range[0], tmp_range[1], tmp_range[2]);
  h5_write_buffer<cl_int>(out_name, "Global_Range", tmp_range, 3);

  h5_read_buffer<cl_int>(filename, "Range_Start", tmp_range);
  range_start = cl::NDRange(tmp_range[0], tmp_range[1], tmp_range[2]);
  h5_write_buffer<cl_int>(out_name, "Range_Start", tmp_range, 3);

  h5_read_buffer<cl_int>(filename, "Local_Range", tmp_range);
  h5_write_buffer<cl_int>(out_name, "Local_Range", tmp_range, 3);
  if ((tmp_range[0]==0) && (tmp_range[1]==0) && (tmp_range[2]==0)) {
    local_range = cl::NullRange;
  }
  else {
    local_range = cl::NDRange(tmp_range[0], tmp_range[1], tmp_range[2]);
  }


  if (benchmark_mode == true) {
    cout << "Sleeping for 4s" << endl << endl;
    std::chrono::milliseconds timespan(4000);
    std::this_thread::sleep_for(timespan);
  }

  cout << "Launching kernel..." << endl;

  time_t rawtime;
  struct tm * timeinfo;

  //get execution timestamp
  time(&rawtime);
  timeinfo = localtime(&rawtime);


  uint64_t exec_time = 0;
  uint32_t kernels_run=0;

  uint64_t total_exec_time = timer.getTimeMicroseconds();

  for (string const& kernel_name : kernel_list) {
    exec_time = exec_time + dev_mgr.execute_kernelNA(*(dev_mgr.getKernelbyName(0, "ocl_Kernel", kernel_name)),
                                                     dev_mgr.get_queue(0, 0), range_start, global_range, local_range);
    kernels_run++;
  }

  total_exec_time = timer.getTimeMicroseconds() - total_exec_time;
  h5_write_single<double>(out_name, "Total_ExecTime", (double)total_exec_time / 1000.0);


  cout << "Kernels executed: " << kernels_run << endl;
  cout << "Kernel runtime: " << exec_time/1000 << " ms" << endl;

  if (benchmark_mode == true) {
    cout << endl << "Sleeping for 4s" << endl;
    std::chrono::milliseconds timespan(4000);

    std::this_thread::sleep_for(timespan);
  }

  cout << "Saving results... " << endl;

  char time_buffer[80];
  strftime(time_buffer, sizeof(time_buffer), "%d-%m-%Y %H:%M:%S", timeinfo);

  h5_write_string(out_name, "Kernel_ExecStart", time_buffer);
  h5_write_string(out_name, "OpenCL_Device", dev_mgr.get_avail_dev_info(deviceIndex).name.c_str());
  h5_write_string(out_name, "OpenCL_Version", dev_mgr.get_avail_dev_info(deviceIndex).ocl_version.c_str());
  h5_write_single<double>(out_name,"Kernel_ExecTime", (double)exec_time/1000.0);
  h5_write_single<double>(out_name, "Data_LoadTime", (double)push_time/1000.0);

  h5_create_dir(out_name, "/Data");

  pull_time = timer.getTimeMicroseconds();

  uint32_t buffer_counter = 0;

  for(cl_uint i = 0; i < data_names.size(); i++) {
    try {
      uint8_t *tmp_data = nullptr;
      size_t var_size = 0;

      switch (data_types.at(i)) {
        case H5_float:  var_size=data_sizes.at(i)*sizeof(cl_float);  break;
        case H5_double: var_size=data_sizes.at(i)*sizeof(cl_double); break;
        case H5_char:   var_size=data_sizes.at(i)*sizeof(cl_char);   break;
        case H5_uchar:  var_size=data_sizes.at(i)*sizeof(cl_uchar);  break;
        case H5_short:  var_size=data_sizes.at(i)*sizeof(cl_short);  break;
        case H5_ushort: var_size=data_sizes.at(i)*sizeof(cl_ushort); break;
        case H5_int:    var_size=data_sizes.at(i)*sizeof(cl_int);    break;
        case H5_uint:   var_size=data_sizes.at(i)*sizeof(cl_uint);   break;
        case H5_long:   var_size=data_sizes.at(i)*sizeof(cl_long);   break;
        case H5_ulong:  var_size=data_sizes.at(i)*sizeof(cl_ulong);  break;
        default: cerr << ERROR_INFO << "Data type '" << data_types.at(i) << "' unknown." << endl;
      }

      tmp_data = new uint8_t[var_size];

      switch (data_rw_flags.at(buffer_counter)) {
        case 0: dev_mgr.get_queue(0, 0).enqueueReadBuffer(data_in.at(buffer_counter), blocking, 0, var_size, tmp_data); break;
        case 1: break;
        case 2: dev_mgr.get_queue(0, 0).enqueueReadBuffer(data_in.at(buffer_counter), blocking, 0, var_size, tmp_data); break;
      }

      dev_mgr.get_queue(0, 0).finish(); //Buffer Copy is asynchronous

      switch (data_types.at(i)){
        case H5_float:  h5_write_buffer<float>(    out_name, data_names.at(i).c_str(), (float*)tmp_data,     data_sizes.at(buffer_counter)); break;
        case H5_double: h5_write_buffer<double>(   out_name, data_names.at(i).c_str(), (double*)tmp_data,    data_sizes.at(buffer_counter)); break;
        case H5_char:   h5_write_buffer<cl_char>(  out_name, data_names.at(i).c_str(), (cl_char*)tmp_data,   data_sizes.at(buffer_counter)); break;
        case H5_uchar:  h5_write_buffer<cl_uchar>( out_name, data_names.at(i).c_str(), (cl_uchar*)tmp_data,  data_sizes.at(buffer_counter)); break;
        case H5_short:  h5_write_buffer<cl_short>( out_name, data_names.at(i).c_str(), (cl_short*)tmp_data,  data_sizes.at(buffer_counter)); break;
        case H5_ushort: h5_write_buffer<cl_ushort>(out_name, data_names.at(i).c_str(), (cl_ushort*)tmp_data, data_sizes.at(buffer_counter)); break;
        case H5_int:    h5_write_buffer<cl_int>(   out_name, data_names.at(i).c_str(), (cl_int*)tmp_data,    data_sizes.at(buffer_counter)); break;
        case H5_uint:   h5_write_buffer<cl_uint>(  out_name, data_names.at(i).c_str(), (cl_uint*)tmp_data,   data_sizes.at(buffer_counter)); break;
        case H5_long:   h5_write_buffer<cl_long>(  out_name, data_names.at(i).c_str(), (cl_long*)tmp_data,   data_sizes.at(buffer_counter)); break;
        case H5_ulong:  h5_write_buffer<cl_ulong>( out_name, data_names.at(i).c_str(), (cl_ulong*)tmp_data,  data_sizes.at(buffer_counter)); break;
        default: cerr << ERROR_INFO << "Data type '" << data_types.at(i) << "' unknown." << endl;
      }
      if (tmp_data != nullptr) {
        delete[] tmp_data; tmp_data = nullptr;
      }
      buffer_counter++;
    }
    catch (cl::Error err) {
      std::cerr << ERROR_INFO << "Exception: " << err.what() << std::endl;
    }
  }

  pull_time = timer.getTimeMicroseconds() - pull_time;
  h5_write_single<double>(out_name, "Data_StoreTime", (double)pull_time / 1000.0);

  return 0;
}
