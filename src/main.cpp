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
#include <codecvt>
#include <string>

#include "opencl_include.hpp"
#include "util.hpp"
#include "hdf5_io.hpp"
#include "ocl_dev_mgr.hpp"
#include "timer.hpp"

#if defined(_WIN32)
#pragma once
#include <windows.h>
#include <sys/timeb.h>
#include "shlobj.h"

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
 static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

 SYSTEMTIME system_time;
 FILETIME  file_time;
 uint64_t  time;

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


inline double timeval2storage(const timeval& timepoint) {
 // convert microseconds to seconds using a resolution of milliseconds
 return timepoint.tv_sec + 1.e-3 * (timepoint.tv_usec / 1000);
}


using namespace std;

#if defined(USEAMDP)
// Disable MS compiler warnings:
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

// Include controller API header file.
#include "AMDTPowerProfileApi.h"

bool amd_log_power = false;
cl_uint amd_power_rate;
bool amd_log_temp = false;
cl_uint amd_temp_rate;


std::vector<std::string> AMDP_names;
std::vector<std::string> AMDT_names;

bool initAMDPP(uint32_t sample_rate) {
 AMDTResult hResult = AMDT_STATUS_OK;

 // Initialize online mode
 hResult = AMDTPwrProfileInitialize(AMDT_PWR_MODE_TIMELINE_ONLINE);
 // check AMDT_STATUS_OK == hResult
 AMDTUInt32 nbrCounters = 0;
 AMDTPwrCounterDesc* pCounters = nullptr;

 hResult = AMDTPwrGetSupportedCounters(&nbrCounters, &pCounters);
 // check AMDT_STATUS_OK == hResult

 // cout << endl << nbrCounters << endl;
 for (AMDTUInt32 idx = 0; idx < nbrCounters; idx++)
 {
  //get only power - for now
  if ((pCounters[idx].m_category == AMDT_PWR_CATEGORY_CORRELATED_POWER) && (amd_power_rate > 0))
  {
   hResult = AMDTPwrEnableCounter(pCounters[idx].m_counterID);
  }
  if ((pCounters[idx].m_category == AMDT_PWR_CATEGORY_TEMPERATURE) && (amd_temp_rate > 0)) {
   hResult = AMDTPwrEnableCounter(pCounters[idx].m_counterID);
  }

 }
 AMDTPwrSetTimerSamplingPeriod(100);

 //dry profiling run, to see which counter are available for real sampling

 AMDTPwrStartProfiling();
 std::this_thread::sleep_for(std::chrono::milliseconds(500));

 AMDTPwrSample* pSampleData = nullptr;
 AMDTUInt32 nbrSamples = 0;

 hResult = AMDTPwrReadAllEnabledCounters(&nbrSamples, &pSampleData);

 std::vector<AMDTUInt32> usable_power_counters;
 std::vector<AMDTUInt32> usable_temp_counters;

 if ((nullptr != pSampleData) && (nbrSamples > 0))
 {
   for (size_t j = 0; j < nbrSamples; j++)
   {

  for (size_t i = 0; i < pSampleData[0].m_numOfCounter; i++) //hardcoded to use the first sample returned
  {
   AMDTPwrCounterDesc counterDesc;
   AMDTPwrGetCounterDesc(pSampleData[0].m_counterValues->m_counterID, &counterDesc);

   if ((counterDesc.m_category == AMDT_PWR_CATEGORY_CORRELATED_POWER) )
   {

     if (std::find(usable_power_counters.begin(), usable_power_counters.end(), pSampleData[0].m_counterValues->m_counterID) != usable_power_counters.end() == false) {
       AMDP_names.push_back(counterDesc.m_name);
       usable_power_counters.push_back(pSampleData[0].m_counterValues->m_counterID);
      // cout << counterDesc.m_name << " P " << pSampleData[0].m_counterValues->m_counterID << pSampleData[0].m_counterValues->m_counterID << endl;
     }
   }
     if ((counterDesc.m_category == AMDT_PWR_CATEGORY_TEMPERATURE) )
     {

       if (std::find(usable_temp_counters.begin(), usable_temp_counters.end(), pSampleData[0].m_counterValues->m_counterID) != usable_temp_counters.end() == false) {
         AMDT_names.push_back(counterDesc.m_name);
         usable_temp_counters.push_back(pSampleData[0].m_counterValues->m_counterID);
        // cout << counterDesc.m_name << " T " << pSampleData[0].m_counterValues->m_counterID << endl;
       }
   }
   pSampleData[i].m_counterValues++;
  }

   }


 }
 //restart AMD profiling
 AMDTPwrStopProfiling();
 AMDTPwrProfileClose();
 hResult = AMDTPwrProfileInitialize(AMDT_PWR_MODE_TIMELINE_ONLINE);
 AMDTPwrSetTimerSamplingPeriod(sample_rate);
 //reenable usable counters
 for (AMDTUInt32 idx = 0; idx < usable_power_counters.size(); idx++)
 {
  AMDTPwrEnableCounter(usable_power_counters.at(idx));
 }
 for (AMDTUInt32 idx = 0; idx < usable_temp_counters.size(); idx++)
 {
   AMDTPwrEnableCounter(usable_temp_counters.at(idx));
 }
 AMDTPwrStartProfiling();
 return true;
}

std::vector<double> amd_temp_time;
std::vector<double> amd_power_time;
std::vector<cl_float> amd_power[10]; //Socket 0
std::vector<cl_float> amd_temp[10]; //Socket 0

void amd_log_power_func()
{

 while ((amd_log_power == true)|| (amd_log_temp == true))
 {
  timeval rawtime;

  std::this_thread::sleep_for(std::chrono::milliseconds(amd_power_rate));

  AMDTResult hResult = AMDT_STATUS_OK;
  AMDTPwrSample* pSampleData = nullptr;
  AMDTUInt32 nbrSamples = 0;

  gettimeofday(&rawtime, NULL);
  hResult = AMDTPwrReadAllEnabledCounters(&nbrSamples, &pSampleData);

  if ((nullptr != pSampleData) && (nbrSamples > 0))
  {
   for (size_t i = 0; i < pSampleData[0].m_numOfCounter; i++) //hardcoded to use the first sample returned
   {

    AMDTPwrCounterDesc counterDesc;
    AMDTPwrGetCounterDesc(pSampleData[0].m_counterValues->m_counterID, &counterDesc);
    if ((counterDesc.m_category == AMDT_PWR_CATEGORY_CORRELATED_POWER))
    {
      amd_power[i].push_back(pSampleData[0].m_counterValues->m_data);
    }
    if ((counterDesc.m_category == AMDT_PWR_CATEGORY_TEMPERATURE))
    {
      amd_temp[i].push_back(pSampleData[0].m_counterValues->m_data);
    }
    pSampleData[i].m_counterValues++;
   }
   amd_power_time.push_back(timeval2storage(rawtime));
  }
 }


}

void amd_log_temp_func()
{

  while (amd_log_temp == true) {
    timeval rawtime;

    std::this_thread::sleep_for(std::chrono::milliseconds(amd_temp_rate));

    AMDTResult hResult = AMDT_STATUS_OK;
    AMDTPwrSample* pSampleData = nullptr;
    AMDTUInt32 nbrSamples = 0;

    gettimeofday(&rawtime, NULL);
    hResult = AMDTPwrReadAllEnabledCounters(&nbrSamples, &pSampleData);

    if ((nullptr != pSampleData) && (nbrSamples > 0)) {
      for (size_t i = 0; i < pSampleData[0].m_numOfCounter; i++) //hardcoded to use the first sample returned
      {

        AMDTPwrCounterDesc counterDesc;
        AMDTPwrGetCounterDesc(pSampleData[0].m_counterValues->m_counterID, &counterDesc);

        if ((counterDesc.m_category == AMDT_PWR_CATEGORY_TEMPERATURE))
        {
          amd_temp[i].push_back(pSampleData[0].m_counterValues->m_data);
        }
        pSampleData[i].m_counterValues++;
      }
      amd_temp_time.push_back(timeval2storage(rawtime));
    }
  }


}

#endif // USEAMDP

#if defined(USEIRAPL)
#include "rapl.hpp"
bool intel_log_power = false;
bool intel_log_temp = false;
cl_uint intel_power_rate = 0;
cl_uint intel_temp_rate = 0;
std::vector<double> intel_power_time;
std::vector<double> intel_temp_time;
std::vector<uint64_t> intel_power0[5]; //Socket 0
std::vector<uint64_t> intel_power1[5]; //Socket 1
std::vector<std::string> MSR_names { "package", "cores", "DRAM", "GT" };
std::vector<cl_ushort> intel_temp0;
std::vector<cl_ushort> intel_temp1;

Rapl *rapl;

void intel_log_power_func() {
 timeval rawtime;

 if (intel_power_rate > 0)
 {
  uint64_t pkg;
  uint64_t pp0;
  uint64_t pp1;
  uint64_t dram;

  intel_power0[0].clear();
  intel_power0[1].clear();
  intel_power0[2].clear();
  intel_power0[3].clear();
  intel_power1[0].clear();
  intel_power1[1].clear();
  intel_power1[2].clear();
  intel_power1[3].clear();
  intel_power_time.clear();

  while (intel_log_power == true) {
   rapl->sample();
   std::this_thread::sleep_for(std::chrono::milliseconds(intel_power_rate / 2));
   gettimeofday(&rawtime, NULL);
   std::this_thread::sleep_for(std::chrono::milliseconds(intel_power_rate / 2));
   intel_power_time.push_back(timeval2storage(rawtime));

   rapl->get_socket0_data(pkg, pp0, pp1, dram);
   intel_power0[0].push_back(pkg);
   intel_power0[1].push_back(pp0);
   intel_power0[2].push_back(dram);
   intel_power0[3].push_back(pp1);

   if (rapl->detect_socket1() == true) {
    rapl->get_socket1_data(pkg, pp0, pp1, dram);
    intel_power1[0].push_back(pkg);
    intel_power1[1].push_back(pp0);
    intel_power1[2].push_back(dram);
    intel_power1[3].push_back(pp1);
   }
  }
 }
}

void intel_log_temp_func()
{
 uint32_t temp0 = 0, temp1 = 0;
 timeval rawtime;

 if (intel_temp_rate > 0) {
  intel_temp0.clear();
  intel_temp1.clear();
  intel_temp_time.clear();

  while (intel_log_temp == true) {
   std::this_thread::sleep_for(std::chrono::milliseconds(intel_temp_rate));

   gettimeofday(&rawtime, NULL);
   temp0 = rapl->get_temp0();
   if (rapl->detect_socket1() == true)
   {
    temp1 = rapl->get_temp1();
		intel_temp1.push_back(temp1);
   }

   intel_temp_time.push_back(timeval2storage(rawtime));
   intel_temp0.push_back(temp0);
  }
 }
}

#endif // USEIRAPL


#if defined(USEIPG)
#include "rapl.hpp"

Rapl *rapl;

#if defined(_WIN32)
std::string utf16ToUtf8(const std::wstring& utf16Str)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
  return conv.to_bytes(utf16Str);
}
#else
#define MAX_PATH 260 /* defined by windows and used below */
std::string utf16ToUtf8(const std::string& utf16Str)
{
  return utf16Str;
}
#endif

std::vector<double> intel_power_time;
std::vector<double> intel_temp_time;
std::vector<std::string> MSR_names;
std::vector<int> MSR;
bool intel_log_power = false;
bool intel_log_temp = false;
cl_uint intel_power_rate = 0;
cl_uint intel_temp_rate = 0;

std::vector<float> intel_power[5];
std::vector<cl_ushort> intel_temp;

void intel_log_temp_func()
{
 // int temp;
 timeval rawtime;

 if (intel_temp_rate > 0) {
  intel_temp.clear();
  intel_temp_time.clear();

  while (intel_log_temp == true) {
   std::this_thread::sleep_for(std::chrono::milliseconds(intel_temp_rate));
   intel_temp.push_back(rapl->get_temp0());
   gettimeofday(&rawtime, NULL);
   intel_temp_time.push_back(timeval2storage(rawtime));

  }
 }
}

void intel_log_power_func()
{
 double data[3];
 int nData;
 timeval rawtime;

 if (intel_power_rate > 0)
 {
	for (unsigned int i = 0; i < MSR.size(); i++) {
	  intel_power[i].clear();
	}
  intel_power_time.clear();

  while (intel_log_power == true) {
   std::this_thread::sleep_for(std::chrono::milliseconds(intel_power_rate));
   rapl->sample();
   gettimeofday(&rawtime, NULL);
   intel_power_time.push_back(timeval2storage(rawtime));

   for (unsigned int i = 0; i < MSR.size(); i++) {
    rapl->GetPowerData(0, MSR.at(i), data, &nData);
    intel_power[i].push_back((float)data[0]);
   }
  }
 }
}


#endif // USEIPG




#if defined(USENVML)
#include <nvml.h>
bool nvidia_log_power = false;
bool nvidia_log_temp = false;
cl_uint nvidia_power_rate = 0;
cl_uint nvidia_temp_rate = 0;
nvmlDevice_t device;
std::vector<cl_ushort> nvidia_temp;
std::vector<double> nvidia_temp_time;

std::vector<float> nvidia_power;
std::vector<double> nvidia_power_time;

void nvidia_log_power_func()
{
  if (nvidia_power_rate > 0) {
    unsigned int temp;
    timeval rawtime;

    nvidia_power.clear();
    nvidia_power_time.clear();

    while (nvidia_log_power == true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(nvidia_power_rate));

      nvmlDeviceGetPowerUsage(device, &temp);
      gettimeofday(&rawtime, NULL);
      nvidia_power_time.push_back(timeval2storage(rawtime));
      // convert milliwatt to watt
      nvidia_power.push_back(1.e-3f * (float)(temp));
    }

    nvmlShutdown();
  }
}

void nvidia_log_temp_func() {
  if (nvidia_temp_rate > 0)
  {
    unsigned int temp;
    timeval rawtime;

    nvidia_temp.clear();
    nvidia_temp_time.clear();

    while (nvidia_log_temp == true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(nvidia_temp_rate));
      nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
      gettimeofday(&rawtime, NULL);
      nvidia_temp_time.push_back(timeval2storage(rawtime));
      nvidia_temp.push_back(temp);
    }

    nvmlShutdown();

  }
}

#endif // USENVML


#if defined(_WIN32)
typedef LONG NTSTATUS, *PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOEXW);

RTL_OSVERSIONINFOEXW GetRealOSVersion() {
 HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
 if (hMod) {
  RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
  if (fxPtr != nullptr) {
   RTL_OSVERSIONINFOEXW rovi = { 0 };
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

 version << "Windows " << GetRealOSVersion().dwMajorVersion << "." << GetRealOSVersion().dwMinorVersion;

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
   if (line.size() >= 1 && line.substr(0, 11) == "PRETTY_NAME") {
    version << line.substr(13, line.length() - 14);
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



void print_help()
{
 cout
  << "Usage: toolkitICL [options] -c config.h5" << endl
  << "Options:" << endl
  << " -d device_id: \n"
    "  Use the device specified by `device_id`." << endl
  << " -b: \n"
    "  Activate the benchmark mode (additional delay before & after runs)." << endl
  << " -c config.h5: \n"
    "  Specify the URL `config.h5` of the HDF5 configuration file." << endl
#if defined(USENVML)
  << " -nvidia_power sample_rate: \n"
    "  Log Nvidia GPU power consumption with `sample_rate` (ms)" << endl
  << " -nvidia_temp sample_rate: \n"
    "  Log Nvidia GPU temperature with `sample_rate` (ms)" << endl
#endif
#if defined(USEIPG) || defined(USEIRAPL)
  << " -intel_power sample_rate: \n"
    "  Log Intel system power consumption with `sample_rate` (ms)" << endl
  << " -intel_temp sample_rate: \n"
    "  Log Intel package temperature with `sample_rate` (ms)" << endl
#endif
#if defined(USEAMDP)
   << " -amd_cpu_power sample_rate: \n"
   "  Log AMD CPU power consumption with `sample_rate` (ms)" << endl
   << " -amd_cpu_temp sample_rate: \n"
   "  Log AMD CPU temperaturen with `sample_rate` (ms)" << endl
#endif
  << endl;
}


int main(int argc, char *argv[]) {

 Timer timer; //used to track performance

 ocl_dev_mgr& dev_mgr = ocl_dev_mgr::getInstance();
 cl_uint devices_availble = dev_mgr.get_avail_dev_num();

 cout << "Available OpenCL devices: " << devices_availble << endl;

 // default options
 cl_uint deviceIndex = 0;
 bool benchmark_mode = false;
 char const* filename = nullptr;

 // parse command line arguments starting at index 1 (because toolkitICL is the 0th argument)
 for (int option_idx = 1; option_idx < argc; ++option_idx)
 {
  if (argv[option_idx] == string("-h")) {
   print_help();
   return 0;
  }
  else if (argv[option_idx] == string("-b")) {
   benchmark_mode = true;
   cout << "Benchmark mode" << endl << endl;
  }
  else if (argv[option_idx] == string("-d")) {
   ++option_idx;
   try {
    deviceIndex = stoi(argv[option_idx]);
   }
   catch (const std::exception& e) {
    cerr << "Error: Could not convert '" << argv[option_idx] << "' to an integer." << endl;
    throw(e);
   }
  }
  else if (argv[option_idx] == string("-c")) {
   ++option_idx;
   filename = argv[option_idx];
  }
#if defined(USENVML)
  else if (argv[option_idx] == string("-nvidia_power")) {
   ++option_idx;
   try {
    nvidia_power_rate = stoi(argv[option_idx]);
   }
   catch (const std::exception& e) {
    cerr << "Error: Could not convert '" << argv[option_idx] << "' to an integer." << endl;
    throw(e);
   }
   nvidia_log_power = true;
  }
  else if (argv[option_idx] == string("-nvidia_temp") || argv[option_idx] == string("-nvidia_temperature")) {
   ++option_idx;
   try {
    nvidia_temp_rate = stoi(argv[option_idx]);
   }
   catch (const std::exception& e) {
    cerr << "Error: Could not convert '" << argv[option_idx] << "' to an integer." << endl;
    throw(e);
   }
   nvidia_log_temp = true;
  }
#endif // defined(USENVML)
#if defined(USEIPG) || defined(USEIRAPL)
  else if (argv[option_idx] == string("-intel_power")) {
   ++option_idx;
   try {
    intel_power_rate = stoi(argv[option_idx]);
   }
   catch (const std::exception& e) {
    cerr << "Error: Could not convert '" << argv[option_idx] << "' to an integer." << endl;
    throw(e);
   }
   intel_log_power = true;
  }
  else if (argv[option_idx] == string("-intel_temp") || argv[option_idx] == string("-intel_temperature")) {
   ++option_idx;
   try {
    intel_temp_rate = stoi(argv[option_idx]);
   }
   catch (const std::exception& e) {
    cerr << "Error: Could not convert '" << argv[option_idx] << "' to an integer." << endl;
    throw(e);
   }
   intel_log_temp = true;
  }
#endif // defined(USEIPG) || defined(USEIRAPL)
#if defined(USEAMDP)
  else if (argv[option_idx] == string("-amd_cpu_power")) {
   ++option_idx;
   try {
    amd_power_rate = stoi(argv[option_idx]);
   }
   catch (const std::exception& e) {
    cerr << "Error: Could not convert '" << argv[option_idx] << "' to an integer." << endl;
    throw(e);
   }
   amd_log_power = true;
  }
  else if (argv[option_idx] == string("-amd_cpu_temp") || argv[option_idx] == string("-amd_cpu_temperature")) {
   ++option_idx;
   try {
    amd_temp_rate = stoi(argv[option_idx]);
   }
   catch (const std::exception& e) {
    cerr << "Error: Could not convert '" << argv[option_idx] << "' to an integer." << endl;
    throw(e);
   }
   amd_log_temp = true;
  }
#endif // defined(USEAMDP)
  else {
   cerr << "Error: Unknown command line option '" << argv[option_idx] << "'." << endl;
   print_help();
   return -1;
  }
 }

 // check necessary/incompatible command line arguments
 if (filename == nullptr) {
  cerr << "Error: A configuration file must be given as command line argument." << endl;
  print_help();
  return -1;
 }

#if defined(USEAMDP)
 if (amd_log_temp && amd_log_power) {
  cerr << endl << "Error: Concurrent logging on AMD systems is not suported, yet!" << endl;
  return -1;
 }
#endif // defined(USEAMDP)


 cout << dev_mgr.get_avail_dev_info(deviceIndex).name.c_str() << endl;
 if (benchmark_mode == false) {
   cout << "OpenCL version: " << dev_mgr.get_avail_dev_info(deviceIndex).ocl_version.c_str() << endl;
   cout << "Memory limit: " << dev_mgr.get_avail_dev_info(deviceIndex).max_mem << endl;
   cout << "WG limit: " << dev_mgr.get_avail_dev_info(deviceIndex).wg_size << endl << endl;
 }
 dev_mgr.init_device(deviceIndex);

 string kernel_url;
 if (h5_check_object(filename, "kernel_url") == true) {
   h5_read_string(filename, "kernel_url", kernel_url);
   if (benchmark_mode == false) {
     cout << "Reading kernel from file: " << kernel_url << "... " << endl;
   }
 }
 else if (h5_check_object(filename, "kernel_source") == true) {
   if (benchmark_mode == false) {
     cout << "Reading kernel from HDF5 file... " << endl;
   }
  std::vector<std::string> kernel_source;
  h5_read_strings(filename, "kernel_source", kernel_source);
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
 h5_read_strings(filename, "kernels", kernel_list);

 cl_ulong kernel_repetitions = 1;
 if (h5_check_object(filename, "settings/kernel_repetitions")) {
  kernel_repetitions = h5_read_single<cl_ulong>(filename, "settings/kernel_repetitions");
 }
 if (kernel_repetitions <= 0) {
  cout << "Warning: Setting `kernel_repetitions = " << kernel_repetitions << "` implies that no kernels are executed." << endl;
 }

 dev_mgr.add_program_url(0, "ocl_Kernel", kernel_url);

 string settings;
 h5_read_string(filename, "settings/kernel_settings", settings);


 uint64_t num_kernels_found = 0;
 num_kernels_found = dev_mgr.compile_kernel(0, "ocl_Kernel", settings);
 if (num_kernels_found == 0) {
  cerr << ERROR_INFO << "No valid kernels found" << endl;
  return -1;
 }

 std::vector<std::string> found_kernels;
 dev_mgr.get_kernel_names(0, "ocl_Kernel", found_kernels);
 if (benchmark_mode == false) {
   cout << "Found Kernels: " << found_kernels.size() << endl;
 }
 if (found_kernels.size() == 0) {
  cerr << ERROR_INFO << "No valid kernels found." << endl;
  return -1;
 }

 cout << "Number of Kernels to execute: " << kernel_list.size() * kernel_repetitions << endl;

 //TODO: Clean up; debug mode?
 // for (uint32_t kernel_idx = 0; kernel_idx < kernel_list.size(); kernel_idx++) {
 //  cout <<"Found : "<< kernel_list.at(kernel_idx) << endl;
 // }

 cout << "Ingesting HDF5 config file..." << endl;

 std::vector<std::string> data_names;
 std::vector<HD5_Type> data_types;
 std::vector<size_t> data_sizes;
 h5_get_content(filename, "/data/", data_names, data_types, data_sizes);

 cout << "Creating output HDF5 file..." << endl;
 string out_name = filename;
 out_name = "out_" + out_name.substr(out_name.find_last_of("/\\") + 1);

 if (fileExists(out_name)) {
  remove(out_name.c_str());
  cout << "Old HDF5 data file found and deleted!" << endl;
 }

 h5_create_dir(out_name, "/settings");
 h5_write_string(out_name, "/settings/kernel_settings", settings);
 h5_write_single<cl_ulong>(out_name, "/settings/kernel_repetitions", kernel_repetitions);

 std::vector<cl::Buffer> data_in;
 bool blocking = CL_TRUE;

 //TODO: Implement functionality! Allow other integer types instead of cl_int?
 vector<cl_int> data_rw_flags(data_names.size(), 0);

 uint64_t push_time, pull_time;
 push_time = timer.getTimeMicroseconds();

 for (cl_uint i = 0; i < data_names.size(); i++) {
  try {
   uint8_t *tmp_data = nullptr;
   size_t var_size = 0;

   switch (data_types.at(i)) {
   case H5_float:
    var_size = data_sizes.at(i) * sizeof(float);
    tmp_data = new uint8_t[var_size];
    h5_read_buffer<float>(filename, data_names.at(i).c_str(), (float*)tmp_data);
    break;
   case H5_double:
    var_size = data_sizes.at(i) * sizeof(double);
    tmp_data = new uint8_t[var_size];
    h5_read_buffer<double>(filename, data_names.at(i).c_str(), (double*)tmp_data);
    break;
   case H5_char:
    var_size = data_sizes.at(i) * sizeof(cl_char);
    tmp_data = new uint8_t[var_size];
    h5_read_buffer<cl_char>(filename, data_names.at(i).c_str(), (cl_char*)tmp_data);
    break;
   case H5_uchar:
    var_size = data_sizes.at(i) * sizeof(cl_uchar);
    tmp_data = new uint8_t[var_size];
    h5_read_buffer<cl_uchar>(filename, data_names.at(i).c_str(), (cl_uchar*)tmp_data);
    break;
   case H5_short:
    var_size = data_sizes.at(i) * sizeof(cl_short);
    tmp_data = new uint8_t[var_size];
    h5_read_buffer<cl_short>(filename, data_names.at(i).c_str(), (cl_short*)tmp_data);
    break;
   case H5_ushort:
    var_size = data_sizes.at(i) * sizeof(cl_ushort);
    tmp_data = new uint8_t[var_size];
    h5_read_buffer<cl_ushort>(filename, data_names.at(i).c_str(), (cl_ushort*)tmp_data);
    break;
   case H5_int:
    var_size = data_sizes.at(i) * sizeof(cl_int);
    tmp_data = new uint8_t[var_size];
    h5_read_buffer<cl_int>(filename, data_names.at(i).c_str(), (cl_int*)tmp_data);
    break;
   case H5_uint:
    var_size = data_sizes.at(i) * sizeof(cl_uint);
    tmp_data = new uint8_t[var_size];
    h5_read_buffer<cl_uint>(filename, data_names.at(i).c_str(), (cl_uint*)tmp_data);
    break;
   case H5_long:
    var_size = data_sizes.at(i) * sizeof(cl_long);
    tmp_data = new uint8_t[var_size];
    h5_read_buffer<cl_long>(filename, data_names.at(i).c_str(), (cl_long*)tmp_data);
    break;
   case H5_ulong:
    var_size = data_sizes.at(i) * sizeof(cl_ulong);
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

 if (benchmark_mode == false) {
   cout << "Setting range..." << endl;
 }

 cl::NDRange range_start;
 cl::NDRange global_range;
 cl::NDRange local_range;

 //TODO: Allow other integer types instead of cl_int?
 cl_int tmp_range[3];
 h5_read_buffer<cl_int>(filename, "/settings/global_range", tmp_range);
 global_range = cl::NDRange(tmp_range[0], tmp_range[1], tmp_range[2]);
 h5_write_buffer<cl_int>(out_name, "/settings/global_range", tmp_range, 3);

 h5_read_buffer<cl_int>(filename, "/settings/range_start", tmp_range);
 range_start = cl::NDRange(tmp_range[0], tmp_range[1], tmp_range[2]);
 h5_write_buffer<cl_int>(out_name, "/settings/range_start", tmp_range, 3);

 h5_read_buffer<cl_int>(filename, "/settings/local_range", tmp_range);
 h5_write_buffer<cl_int>(out_name, "/settings/local_range", tmp_range, 3);
 if ((tmp_range[0] == 0) && (tmp_range[1] == 0) && (tmp_range[2] == 0)) {
  local_range = cl::NullRange;
 }
 else {
  local_range = cl::NDRange(tmp_range[0], tmp_range[1], tmp_range[2]);
 }

#if defined(USEAMDP)
 if (amd_log_power||amd_log_temp)
 {
  cout << "Using AMD Power Profiling interface..." << endl << endl;
  h5_create_dir(out_name, "/housekeeping");
  h5_create_dir(out_name, "/housekeeping/amd");
  initAMDPP(amd_power_rate);
 }
 std::thread amd_log_power_thread(amd_log_power_func);
 // std::thread amd_log_temp_thread(amd_log_temp_func);
#endif

#if defined(USENVML)
#if defined(_WIN32)
 if (nvidia_log_power || nvidia_log_temp)
 {
   //Get Program Files path from system
   TCHAR pf[MAX_PATH];
   SHGetSpecialFolderPath(0,pf,CSIDL_PROGRAM_FILES,FALSE);
   std::string nvsmi_path;
   nvsmi_path.append(pf);
   nvsmi_path.append("/NVIDIA Corporation/NVSMI/nvml.dll");
   if (fileExists(nvsmi_path)) {
     LoadLibraryEx(nvsmi_path.c_str(), NULL, 0);
   }else {
     if (fileExists("nvml.dll")) {
       LoadLibraryEx("nvml.dll", NULL, 0);
     }
     else {
       //No NVML found abort
       cout << "NVML library not found..." << endl;
       nvidia_log_temp = false;
       nvidia_log_power = false;
       nvidia_power_rate = 0;
       nvidia_temp_rate = 0;
     }
      }
 }
#endif
 if (nvidia_log_power || nvidia_log_temp)
 {
   nvmlReturn_t result;
   result = nvmlInit();
   if (NVML_SUCCESS == result)
   {
    cout << "Using NVML interface..." << endl;
    h5_create_dir(out_name, "/housekeeping");
    h5_create_dir(out_name, "/housekeeping/nvidia");
   }
   else {
     cout << "NVML failure..." << endl;
     nvidia_log_temp = false;
     nvidia_log_power = false;
   }
 }
 if (nvidia_log_power || nvidia_log_temp)
 {
   nvmlPciInfo_t nv_pciinfo;
   cl_uint nvml_devnum;
   cl_int nvml_devid = -1;

   nvmlDeviceGetCount(&nvml_devnum);

   for (cl_uint i = 0; i < nvml_devnum; i++)
   {
     nvmlDeviceGetHandleByIndex(i, &device);
     nvmlDeviceGetPciInfo(device, &nv_pciinfo);

     std::ostringstream tmp_devid;
     tmp_devid << nv_pciinfo.domain << ":" << nv_pciinfo.bus << ":" << nv_pciinfo.device;
     //cout<< tmp_devid.str() <<endl;
     std::size_t found = dev_mgr.getDevicePCIeID(deviceIndex).find(tmp_devid.str());
     if (found != std::string::npos) {
       nvml_devid = i;
       if (benchmark_mode == false) {
         cout << "NVidia OpenCL device " << tmp_devid.str() << " found in NVML device list." << endl;
       }
       break;
     }
   }
   if (nvml_devid < 0)
   {
     cout << "NVidia OpenCL device " << dev_mgr.getDevicePCIeID(deviceIndex) << " not found in NVML device list! Aborting!" << endl;
     nvmlShutdown();
     exit(EXIT_FAILURE);
   }

   nvmlDeviceGetHandleByIndex(nvml_devid, &device);
 }
 std::thread nvidia_log_power_thread(nvidia_log_power_func);
 std::thread nvidia_log_temp_thread(nvidia_log_temp_func);
#endif

#if defined(USEIPG)
  if (intel_log_power || intel_log_temp) {
    cout << "Using Intel Power Gadget interface..." << endl;
    h5_create_dir(out_name, "/housekeeping");
    h5_create_dir(out_name, "/housekeeping/intel");
    rapl = new Rapl();
  }

  if (intel_log_power)
  {
    h5_write_single<float>(out_name, "/housekeeping/intel/TDP" , (float)rapl->get_TDP(),
                           "Thermal Design Power in watt");

    int numMsrs = rapl->get_NumMSR();

    //This is necesarry for initalization
    rapl->sample();
    rapl->sample();
    rapl->sample();

    for (int j = 0; j < numMsrs; j++)
    {
      int funcID;
      double data[3];
      int nData;
#if defined(_WIN32)
      wchar_t szName[MAX_PATH];
#else
      char szName[MAX_PATH];
#endif

      rapl->GetMsrFunc(j, &funcID);
      rapl->GetMsrName(j, szName);

      if ((funcID == 1)) {
        MSR.push_back(j);
        if (utf16ToUtf8(szName) == "Processor") {
          MSR_names.push_back("package");
        }
        else {
          if (utf16ToUtf8(szName) == "IA") {
            MSR_names.push_back("cores");
          }
          else {
            MSR_names.push_back(utf16ToUtf8(szName));
          }
        }
      }

      //Get Package Power Limit
      if ((funcID == 3) ) {
        double data[3];
        int nData;
        rapl->GetPowerData(0, j, data, &nData);
        std::string varname = "/housekeeping/intel/" + utf16ToUtf8(szName) + "_power_limit";
        h5_write_single<double>(out_name, varname.c_str() , data[0]);
      }

    }
  }
  std::thread intel_log_power_thread(intel_log_power_func);
  std::thread intel_log_temp_thread(intel_log_temp_func);
#endif

#if defined(USEIRAPL)
 if (intel_log_power || intel_log_temp)
 {
  cout << "Using Intel MSR interface..." << endl;
  h5_create_dir(out_name, "/housekeeping");
  h5_create_dir(out_name, "/housekeeping/intel");
  rapl = new Rapl();
 }

 if (intel_log_power)
 {
  h5_write_single<float>(out_name, "/housekeeping/intel/TDP", (float)rapl->get_TDP(),
              "Thermal Design Power in watt");
 }

 std::thread intel_log_power_thread(intel_log_power_func);
 std::thread intel_log_temp_thread(intel_log_temp_func);
#endif


 if (benchmark_mode == true) {
  cout << "Sleeping for 4s" << endl << endl;
  std::chrono::milliseconds timespan(4000);
  std::this_thread::sleep_for(timespan);
 }

 cout << "Launching kernel..." << endl;


 //get execution timestamp
 timeval start_timeinfo;
 gettimeofday(&start_timeinfo, NULL);

 uint64_t exec_time = 0;
 uint32_t kernels_run = 0;

 uint64_t total_exec_time = timer.getTimeMicroseconds();

 for (cl_ulong repetition = 0; repetition < kernel_repetitions; ++repetition) {
  for (string const& kernel_name : kernel_list) {
   exec_time = exec_time + dev_mgr.execute_kernelNA(*(dev_mgr.getKernelbyName(0, "ocl_Kernel", kernel_name)),
   dev_mgr.get_queue(0, 0), range_start, global_range, local_range);
   kernels_run++;
  }
 }

 total_exec_time = timer.getTimeMicroseconds() - total_exec_time;
 h5_create_dir(out_name, "housekeeping");
 h5_write_single<double>(out_name, "/housekeeping/total_execution_time", 1.e-6 * total_exec_time,
             "Time in seconds of the total execution (data transfer, kernel, and host code).");

 cout << "Kernels executed: " << kernels_run << endl;
 cout << "Kernel runtime: " << exec_time / 1000 << " ms" << endl; // TODO: ms or s, int or double?

 if (benchmark_mode == true) {
  cout << endl << "Sleeping for 4s" << endl;
  std::chrono::milliseconds timespan(4000);

  std::this_thread::sleep_for(timespan);
 }

 cout << "Saving results... " << endl;


#if defined(USEAMDP)
 amd_log_power = false;
 amd_log_temp = false;
 amd_log_power_thread.join();
 //amd_log_temp_thread.join();

 if ((amd_power_rate > 0) || (amd_temp_rate > 0)) {

  AMDTPwrStopProfiling();
 }

 if (amd_power_rate > 0)
 {
  h5_write_buffer<double>(out_name, "/housekeeping/amd/power_time", amd_power_time.data(), amd_power_time.size(),
  "POSIX UTC time in seconds since 1970-01-01T00:00.000 (resolution of milliseconds)");

  for (size_t i = 0; i < AMDP_names.size(); i++)
  {
   std::string varname = "/housekeeping/amd/" + AMDP_names.at(i);
   //cout<<"Power: "<< AMDP_names.at(i) <<endl;
   h5_write_buffer<cl_float>(out_name, varname.c_str(), amd_power[i].data(), amd_power[i].size(),
   "Power in watt");
  }

 }
 if (amd_temp_rate > 0)
 {
   //amd_power_time has to be changed to amd_temp_time as soon as simultaneous logging is fixed!
   h5_write_buffer<double>(out_name, "/housekeeping/amd/temperature_time", amd_power_time.data(), amd_power_time.size(),
     "POSIX UTC time in seconds since 1970-01-01T00:00.000 (resolution of milliseconds)");
   for (size_t i = 0; i < AMDT_names.size(); i++)
   {
     std::string varname = "/housekeeping/amd/" + AMDT_names.at(i);
    //  cout << "Temp: "<< AMDT_names.at(i) << endl;
     h5_write_buffer<cl_float>(out_name, varname.c_str(), amd_temp[i].data(), amd_temp[i].size(),
       "Temperatures in degree C");
   }
 }
#endif

#if defined(USEIRAPL)
 intel_log_power = false;
 intel_log_power_thread.join();

 intel_log_temp = false;
 intel_log_temp_thread.join();

 if (intel_power_rate > 0)
 {
  // size()-1 because differences are computed later
  h5_write_buffer<double>(out_name, "/housekeeping/intel/power_time", intel_power_time.data(), intel_power_time.size()-1,
              "POSIX UTC time in seconds since 1970-01-01T00:00.000 (resolution of milliseconds)");

  std::vector<float> tmp_vector;

  size_t max_entries = MSR_names.size();
  if (rapl->detect_igp() == false) {
   // no GT data
   max_entries--;
  }

  for (size_t i = 0; i < max_entries; i++)
  {
   tmp_vector.clear();

   for (size_t j = 0; j < intel_power0[i].size()-1; j++)
   {
    tmp_vector.push_back((rapl->get_e_unit()*(double)(intel_power0[i].at(j+1)-intel_power0[i].at(j))) / ((double)intel_power_rate*0.001));
   }
   std::string varname = "/housekeeping/intel/" + MSR_names.at(i) + "0";
   h5_write_buffer<float>(out_name, varname.c_str(), tmp_vector.data(), tmp_vector.size(),
               "Power in watt");
  }

  if (rapl->detect_socket1() == true)
  {
   for (size_t i = 0; i < max_entries; i++)
   {
    tmp_vector.clear();

    for (size_t j = 0; j < intel_power1[i].size()-1; j++)
    {
     tmp_vector.push_back((rapl->get_e_unit()*(double)(intel_power1[i].at(j+1)-intel_power1[i].at(j)))/((double)intel_power_rate*0.001));
    }
    std::string varname = "/housekeeping/intel/" + MSR_names.at(i) + "1";
    h5_write_buffer<float>(out_name, varname.c_str(), tmp_vector.data(), tmp_vector.size(),
                "Power in watt");
   }
  }
 }

 if (intel_temp_rate > 0)
 {
  h5_write_buffer<double>(out_name, "/housekeeping/intel/temperature_time", intel_temp_time.data(), intel_temp_time.size(),
              "POSIX UTC time in seconds since 1970-01-01T00:00.000 (resolution of milliseconds)");

  h5_write_buffer<cl_ushort>(out_name, "/housekeeping/intel/package0_temperature", intel_temp0.data(), intel_temp0.size(),
                "Temperature in degree Celsius");

  if (rapl->detect_socket1() == true)
  {
   h5_write_buffer<cl_ushort>(out_name, "/housekeeping/intel/package1_temperature", intel_temp1.data(), intel_temp1.size(),
                 "Temperature in degree Celsius");
  }
 }

#endif

#if defined(USEIPG)
 intel_log_power = false;
 intel_log_power_thread.join();

 intel_log_temp = false;
 intel_log_temp_thread.join();

 if (intel_power_rate > 0)
 {
  h5_write_buffer<double>(out_name, "/housekeeping/intel/power_time", intel_power_time.data(), intel_power_time.size(),
              "POSIX UTC time in seconds since 1970-01-01T00:00.000 (resolution of milliseconds)");

  for (size_t i = 0; i < MSR_names.size(); i++)
  {
   std::string varname = "/housekeeping/intel/" + MSR_names.at(i) + "0";
   h5_write_buffer<float>(out_name, varname.c_str(), intel_power[i].data(), intel_power[i].size(),
               "Power in watt");
  }
 }

 if (intel_temp_rate > 0)
 {
  h5_write_buffer<double>(out_name, "/housekeeping/intel/temperature_time", intel_temp_time.data(), intel_temp_time.size(),
              "POSIX UTC time in seconds since 1970-01-01T00:00.000 (resolution of milliseconds)");

  h5_write_buffer<cl_ushort>(out_name, "/housekeeping/intel/package_temperature", intel_temp.data(), intel_temp.size(),
                "Temperature in degree Celsius");
 }
#endif

#if defined(USENVML)
 nvidia_log_power = false;
 nvidia_log_temp = false;
 nvidia_log_power_thread.join();
 nvidia_log_temp_thread.join();

 if ((nvidia_power_rate > 0)||(nvidia_temp_rate > 0))
 {
   nvmlShutdown();
 }

 if (nvidia_power_rate > 0) {

  h5_write_buffer<float>(out_name, "/housekeeping/nvidia/power", nvidia_power.data(), nvidia_power.size(),
              "Power in watt");

  h5_write_buffer<double>(out_name, "/housekeeping/nvidia/power_time", nvidia_power_time.data(), nvidia_power_time.size(),
              "POSIX UTC time in seconds since 1970-01-01T00:00.000 (resolution of milliseconds)");
 }

 if (nvidia_temp_rate > 0) {

  h5_write_buffer<cl_ushort>(out_name, "/housekeeping/nvidia/temperature", nvidia_temp.data(), nvidia_temp.size(),
                "Temperature in degree Celsius");

  h5_write_buffer<double>(out_name, "/housekeeping/nvidia/temperature_time", nvidia_temp_time.data(), nvidia_temp_time.size(),
              "POSIX UTC time in seconds since 1970-01-01T00:00.000 (resolution of milliseconds)");
 }
#endif


 char time_buffer[100];
 time_t tmp_time = start_timeinfo.tv_sec;
 strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%S", localtime(&tmp_time));
 sprintf(time_buffer, "%s.%03ld", time_buffer, start_timeinfo.tv_usec / 1000);
 h5_create_dir(out_name, "housekeeping");
 h5_write_string(out_name, "housekeeping/kernel_execution_start", time_buffer);
 h5_write_single<double>(out_name, "housekeeping/kernel_execution_time", 1.e-6 * exec_time,
             "Time in seconds of the kernel execution (no host code).");
 h5_write_single<double>(out_name, "housekeeping/data_load_time", 1.e-6 * push_time,
             "Time in seconds of the data transfer: hdf5 input file -> host -> device.");

 h5_create_dir(out_name, "architecture");
 h5_write_string(out_name, "architecture/host_os", getOS().c_str());
 h5_write_string(out_name, "architecture/opencl_device", dev_mgr.get_avail_dev_info(deviceIndex).name.c_str());
 h5_write_string(out_name, "architecture/opencl_platform", dev_mgr.get_avail_dev_info(deviceIndex).platform_name.c_str());
 h5_write_string(out_name, "architecture/opencl_version", dev_mgr.get_avail_dev_info(deviceIndex).ocl_version.c_str());

 h5_create_dir(out_name, "/data");

 pull_time = timer.getTimeMicroseconds();

 uint32_t buffer_counter = 0;

 for (cl_uint i = 0; i < data_names.size(); i++) {
  try {
   uint8_t *tmp_data = nullptr;
   size_t var_size = 0;

   switch (data_types.at(i)) {
    case H5_float: var_size = data_sizes.at(i) * sizeof(cl_float); break;
    case H5_double: var_size = data_sizes.at(i) * sizeof(cl_double); break;
    case H5_char:  var_size = data_sizes.at(i) * sizeof(cl_char);  break;
    case H5_uchar: var_size = data_sizes.at(i) * sizeof(cl_uchar); break;
    case H5_short: var_size = data_sizes.at(i) * sizeof(cl_short); break;
    case H5_ushort: var_size = data_sizes.at(i) * sizeof(cl_ushort); break;
    case H5_int:  var_size = data_sizes.at(i) * sizeof(cl_int);  break;
    case H5_uint:  var_size = data_sizes.at(i) * sizeof(cl_uint);  break;
    case H5_long:  var_size = data_sizes.at(i) * sizeof(cl_long);  break;
    case H5_ulong: var_size = data_sizes.at(i) * sizeof(cl_ulong); break;
    default: cerr << ERROR_INFO << "Data type '" << data_types.at(i) << "' unknown." << endl;
   }

   tmp_data = new uint8_t[var_size];

   switch (data_rw_flags.at(buffer_counter)) {
    case 0: dev_mgr.get_queue(0, 0).enqueueReadBuffer(data_in.at(buffer_counter), blocking, 0, var_size, tmp_data); break;
    case 1: break;
    case 2: dev_mgr.get_queue(0, 0).enqueueReadBuffer(data_in.at(buffer_counter), blocking, 0, var_size, tmp_data); break;
   }

   dev_mgr.get_queue(0, 0).finish(); //Buffer Copy is asynchronous

   switch (data_types.at(i)) {
    case H5_float: h5_write_buffer<float>(  out_name, data_names.at(i).c_str(), (float*)tmp_data,   data_sizes.at(buffer_counter)); break;
    case H5_double: h5_write_buffer<double>(  out_name, data_names.at(i).c_str(), (double*)tmp_data,  data_sizes.at(buffer_counter)); break;
    case H5_char:  h5_write_buffer<cl_char>( out_name, data_names.at(i).c_str(), (cl_char*)tmp_data,  data_sizes.at(buffer_counter)); break;
    case H5_uchar: h5_write_buffer<cl_uchar>( out_name, data_names.at(i).c_str(), (cl_uchar*)tmp_data, data_sizes.at(buffer_counter)); break;
    case H5_short: h5_write_buffer<cl_short>( out_name, data_names.at(i).c_str(), (cl_short*)tmp_data, data_sizes.at(buffer_counter)); break;
    case H5_ushort: h5_write_buffer<cl_ushort>(out_name, data_names.at(i).c_str(), (cl_ushort*)tmp_data, data_sizes.at(buffer_counter)); break;
    case H5_int:  h5_write_buffer<cl_int>(  out_name, data_names.at(i).c_str(), (cl_int*)tmp_data,  data_sizes.at(buffer_counter)); break;
    case H5_uint:  h5_write_buffer<cl_uint>( out_name, data_names.at(i).c_str(), (cl_uint*)tmp_data,  data_sizes.at(buffer_counter)); break;
    case H5_long:  h5_write_buffer<cl_long>( out_name, data_names.at(i).c_str(), (cl_long*)tmp_data,  data_sizes.at(buffer_counter)); break;
    case H5_ulong: h5_write_buffer<cl_ulong>( out_name, data_names.at(i).c_str(), (cl_ulong*)tmp_data, data_sizes.at(buffer_counter)); break;
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
 h5_write_single<double>(out_name, "housekeeping/data_store_time", 1.e-6 * pull_time,
             "Time in seconds of the data transfer: device -> host -> hdf5 output file.");

 return 0;
}