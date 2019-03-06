/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string>
#include <thread>
#include <time.h>
#include <vector>

#include "opencl_include.hpp"
#include "util.hpp"
#include "hdf5_io.hpp"
#include "ocl_dev_mgr.hpp"
#include "timer.hpp"


#if defined(_WIN32)
#include <windows.h>
#include <sys/timeb.h>

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
  static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

  SYSTEMTIME  system_time;
  FILETIME    file_time;
  uint64_t    time;

  GetSystemTime(&system_time);
  SystemTimeToFileTime(&system_time, &file_time);
  time = ((uint64_t)file_time.dwLowDateTime);
  time += ((uint64_t)file_time.dwHighDateTime) << 32;

  tp->tv_sec = (long)((time - EPOCH) / 10000000L);
  tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
  return 0;
}

#else
#include <sys/time.h>
#endif

using namespace std;

#if defined(USENVML)
#include <nvml.h>
bool nv_log_pwr = false;
bool nv_log_tmp = false;
cl_uint nv_p_rate=0;
cl_uint nv_t_rate=0;

std::vector<cl_ushort> nv_tmp;
std::vector<timeval> nv_tmp_time;

std::vector<cl_uint> nv_pwr;
std::vector<timeval> nv_pwr_time;

void nv_log_pwr_func()
{
  if (nv_p_rate>0) {
    unsigned int temp;
    nvmlReturn_t result;
    timeval rawtime;

    nv_pwr.clear();
    nv_pwr_time.clear();

    result = nvmlInit();
    if (NVML_SUCCESS == result)
    {
      nvmlDevice_t device;
      nvmlDeviceGetHandleByIndex(0, &device);

      while (nv_log_pwr== true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(nv_p_rate));

        nvmlDeviceGetPowerUsage(device, &temp);
        //time(&rawtime);
        gettimeofday(&rawtime, NULL);
        nv_pwr_time.push_back(rawtime);
        nv_pwr.push_back(temp);
      }

      nvmlShutdown();
    }
  }
}

void nv_log_tmp_func()
{
  if (nv_t_rate>0) {
    unsigned int temp;
    nvmlReturn_t result;
    timeval rawtime;

    nv_tmp.clear();
    nv_tmp_time.clear();

    result = nvmlInit();
    if (NVML_SUCCESS == result)
    {
      nvmlDevice_t device;
      nvmlDeviceGetHandleByIndex(0, &device);

      while (nv_log_tmp == true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(nv_t_rate));
        result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
        gettimeofday(&rawtime, NULL);
        nv_tmp_time.push_back(rawtime);
        nv_tmp.push_back(temp);
      }

      nvmlShutdown();
    }
  }
}

#endif


#if defined(_WIN32)
typedef LONG NTSTATUS, *PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOEXW);

RTL_OSVERSIONINFOEXW GetRealOSVersion() {
  HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
  if (hMod) {
    RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
    if (fxPtr != nullptr) {
      RTL_OSVERSIONINFOEXW  rovi = { 0 };
      rovi.dwOSVersionInfoSize = sizeof(rovi);
      if (STATUS_SUCCESS == fxPtr(&rovi)) {
        return rovi;
      }

    }
  }

  RTL_OSVERSIONINFOEXW rovi = { 0 };
  return rovi;
}
#else
#include <sys/utsname.h>
#endif

std::string getOS()
{
  std::stringstream version;
#if defined(_WIN32)

  version<<"Windows " << GetRealOSVersion().dwMajorVersion<<"."<< GetRealOSVersion().dwMinorVersion;

  if (GetRealOSVersion().wProductType == VER_NT_WORKSTATION) {
    version << " Workstation";
  }
  else {
    version << " Server";
  }

#elif defined(__APPLE__)

  char line[256];
  string product_name, product_version;
  FILE* sw_vers = popen("sw_vers", "r");
  while (fgets(&line[0], sizeof(line), sw_vers) != nullptr) {
    if (strncmp(line, "ProductName:", 12) == 0) {
      product_name = string(&line[13]);
      product_name.pop_back(); // erase the newline
    }
    else if (strncmp(line, "ProductVersion:", 15) == 0) {
      product_version = string(&line[16]);
      product_version.pop_back(); // erase the newline
    }
  }
  pclose(sw_vers);
  version << product_name << " " << product_version;

#else // linux

  struct utsname unameData;
  uname(&unameData);
  string line;

  version << unameData.sysname << " ";

  ifstream rel_file("/etc/os-release");
  if (rel_file.is_open()) {
    while (rel_file.good()) {
      getline(rel_file, line);
      if (line.size() >= 1 && line.substr(0,11) == "PRETTY_NAME") {
        version << line.substr(13, line.length()-14);
        break;
      }
    }

    rel_file.close();
  }
  else {
    version << "Unknown Distribution";
  }

  version << "/" << unameData.release << "/" << unameData.version;

#endif

  return version.str();
}


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
#if defined(USENVML)
       << "  -np sample_rate: " << "Log Nvidia GPU power consumption with sample_rate (ms)" << endl
       << "  -nt sample_rate: " << "Log Nvidia GPU temperature with sample_rate (ms)" << endl
#endif
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

#if defined(USENVML)
  if (cmdOptionExists(argv, argv + argc, "-np")) {
    char const* tmp = getCmdOption(argv, argv + argc, "-np");
    nv_p_rate = atoi(tmp);
     nv_log_pwr = true;
  }

  if (cmdOptionExists(argv, argv + argc, "-nt")) {
    char const* tmp = getCmdOption(argv, argv + argc, "-nt");
    nv_t_rate = atoi(tmp);
     nv_log_tmp = true;
  }
#endif
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

  cl_ulong kernel_repetitions = 1;
  if (h5_check_object(filename, "Kernel_Repetitions")) {
    kernel_repetitions = h5_read_single<cl_ulong>(filename, "Kernel_Repetitions");
  }
  if (kernel_repetitions <= 0) {
    cout << "Warning: Setting `kernel_repetitions = " << kernel_repetitions << "` implies that no kernels are executed." << endl;
  }

  dev_mgr.add_program_url(0, "ocl_Kernel", kernel_url);

  string settings;
  h5_read_string(filename, "Kernel_Settings", settings);


  uint64_t num_kernels_found = 0;
  num_kernels_found = dev_mgr.compile_kernel(0, "ocl_Kernel", settings);
  if (num_kernels_found == 0) {
    cerr << ERROR_INFO << "No valid kernels found" << endl;
    return -1;
  }

  std::vector<std::string> found_kernels;
  dev_mgr.get_kernel_names(0, "ocl_Kernel", found_kernels);
  cout << "Found Kernels: " << found_kernels.size() << endl;
  if (found_kernels.size() == 0) {
    cerr << ERROR_INFO << "No valid kernels found." << endl;
    return -1;
  }

  cout << "Number of Kernels to execute: " << kernel_list.size() * kernel_repetitions << endl;

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
  h5_write_string(out_name.c_str(), "Host_OS", getOS().c_str());

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
  h5_write_buffer<cl_int>(out_name, "Global_Range", tmp_range, 3,
                          "Argument `global_work_size` of the OpenCL function `clEnqueueNDRangeKernel`.");

  h5_read_buffer<cl_int>(filename, "Range_Start", tmp_range);
  range_start = cl::NDRange(tmp_range[0], tmp_range[1], tmp_range[2]);
  h5_write_buffer<cl_int>(out_name, "Range_Start", tmp_range, 3,
                          "Argument `global_work_offset` of the OpenCL function `clEnqueueNDRangeKernel`.");

  h5_read_buffer<cl_int>(filename, "Local_Range", tmp_range);
  h5_write_buffer<cl_int>(out_name, "Local_Range", tmp_range, 3,
                          "Argument `local_work_size` of the OpenCL function `clEnqueueNDRangeKernel`.");
  if ((tmp_range[0]==0) && (tmp_range[1]==0) && (tmp_range[2]==0)) {
    local_range = cl::NullRange;
  }
  else {
    local_range = cl::NDRange(tmp_range[0], tmp_range[1], tmp_range[2]);
  }

#if defined(USENVML)
  cout << "Using NVML interface..." << endl << endl;
  std::thread nv_log_pwr_thread(nv_log_pwr_func);
  std::thread nv_log_tmp_thread(nv_log_tmp_func);
#endif

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

  for (cl_ulong repetition = 0; repetition < kernel_repetitions; ++repetition) {
    for (string const& kernel_name : kernel_list) {
      exec_time = exec_time + dev_mgr.execute_kernelNA(*(dev_mgr.getKernelbyName(0, "ocl_Kernel", kernel_name)),
                                                       dev_mgr.get_queue(0, 0), range_start, global_range, local_range);
      kernels_run++;
    }
  }

  total_exec_time = timer.getTimeMicroseconds() - total_exec_time;
  h5_write_single<double>(out_name, "Total_ExecTime", (double)total_exec_time / 1000.0,
                          "Time in milliseconds of the total execution (data transfer, kernel, and host code).");


  cout << "Kernels executed: " << kernels_run << endl;
  cout << "Kernel runtime: " << exec_time/1000 << " ms" << endl;

  if (benchmark_mode == true) {
    cout << endl << "Sleeping for 4s" << endl;
    std::chrono::milliseconds timespan(4000);

    std::this_thread::sleep_for(timespan);
  }

 cout << "Saving results... " << endl;

#if defined(USENVML)
  nv_log_pwr = false;
  nv_log_tmp = false;
  nv_log_pwr_thread.join();
  nv_log_tmp_thread.join();

  std::vector<std::string> time_strings;

  h5_create_dir(out_name, "/NV_HK");

  if (nv_p_rate>0) {

    h5_write_buffer<cl_uint>(out_name, "/NV_HK/NV_Power", nv_pwr.data(), nv_pwr.size());

    for(size_t i = 0; i < nv_pwr_time.size(); i++) {
      char time_buffer[100];
      time_t temp = nv_pwr_time.at(i).tv_sec;
      timeinfo = localtime(&temp);
      strftime(time_buffer, sizeof(time_buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
      sprintf(time_buffer, "%s:%03ld", time_buffer, nv_pwr_time.at(i).tv_usec / 1000);
      time_strings.push_back(time_buffer);
    }

    h5_write_strings(out_name, "/NV_HK/NV_Power_Time", time_strings);
    time_strings.clear();
  }

  if (nv_t_rate>0) {

    h5_write_buffer<cl_ushort>(out_name, "/NV_HK/NV_Temperature", nv_tmp.data(), nv_tmp.size());

    for(size_t i = 0; i < nv_tmp_time.size(); i++) {
      char time_buffer[100];
      time_t temp = nv_tmp_time.at(i).tv_sec;
      timeinfo = localtime(&temp);
      strftime(time_buffer, sizeof(time_buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
      sprintf(time_buffer, "%s:%03ld", time_buffer, nv_tmp_time.at(i).tv_usec / 1000);
      time_strings.push_back(time_buffer);
    }

    h5_write_strings(out_name, "/NV_HK/NV_Temperature_Time", time_strings);
    time_strings.clear();
  }
#endif


  char time_buffer[80];
  strftime(time_buffer, sizeof(time_buffer), "%d-%m-%Y %H:%M:%S", timeinfo);

  h5_write_string(out_name, "Kernel_ExecStart", time_buffer);
  h5_write_string(out_name, "OpenCL_Device", dev_mgr.get_avail_dev_info(deviceIndex).name.c_str());
  h5_write_string(out_name, "OpenCL_Version", dev_mgr.get_avail_dev_info(deviceIndex).ocl_version.c_str());
  h5_write_single<double>(out_name, "Kernel_ExecTime", (double)exec_time/1000.0,
                          "Time in milliseconds of the kernel execution (no host code).");
  h5_write_single<double>(out_name, "Data_LoadTime", (double)push_time/1000.0,
                          "Time in milliseconds of the data transfer: hdf5 input file -> host -> device.");

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
  h5_write_single<double>(out_name, "Data_StoreTime", (double)pull_time / 1000.0,
                          "Time in milliseconds of the data transfer: device -> host -> hdf5 output file.");

  return 0;
}
