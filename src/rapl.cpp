#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <fcntl.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include "rapl.hpp"

#define MSR_RAPL_POWER_UNIT            0x606

/*
 * Platform specific RAPL Domains.
 * see Intel architecture datahseets for more information.
 */

/* Temperature, Cores */
#define IA32_THERM_STATUS              0x19c
#define IA32_THERM_STATUS_UNIT_MASK    30:27
#define IA32_THERM_STATUS_MASK         22:16

/* Temperature, Package */
#define IA32_PACKAGE_THERM_STATUS      0x1b1
#define IA32_PACKAGE_THERM_STATUS_MASK 22:16
#define MSR_TEMPERATURE_TARGET         0x1a2
#define MSR_TEMPERATURE_TARGET_MASK    23:16

/* Package RAPL  */
#define MSR_PKG_RAPL_POWER_LIMIT       0x610
#define MSR_PKG_ENERGY_STATUS          0x611
#define MSR_PKG_PERF_STATUS            0x13
#define MSR_PKG_POWER_INFO             0x614

/* PP0 RAPL */
#define MSR_PP0_POWER_LIMIT            0x638
#define MSR_PP0_ENERGY_STATUS          0x639
#define MSR_PP0_POLICY                 0x63A
#define MSR_PP0_PERF_STATUS            0x63B

/* PP1 RAPL */
#define MSR_PP1_POWER_LIMIT            0x640
#define MSR_PP1_ENERGY_STATUS          0x641
#define MSR_PP1_POLICY                 0x642

/* DRAM RAPL  */
#define MSR_DRAM_POWER_LIMIT           0x618
#define MSR_DRAM_ENERGY_STATUS         0x619
#define MSR_DRAM_PERF_STATUS           0x61B
#define MSR_DRAM_POWER_INFO            0x61C

/* RAPL UNIT BITMASK */
#define POWER_UNIT_OFFSET              0
#define POWER_UNIT_MASK                0x0F

#define ENERGY_UNIT_OFFSET             0x08
#define ENERGY_UNIT_MASK               0x1F00

#define TIME_UNIT_OFFSET               0x10
#define TIME_UNIT_MASK                 0xF000

#define SIGNATURE_MASK                 0xFFFF0
#define IVYBRIDGE_E                    0x306F0
#define SANDYBRIDGE_E                  0x206D0

#if defined(_WIN32)


#pragma once
#include <Windows.h>
#include <vector>
#include <tchar.h>
#include <codecvt>



std::wstring Rapl::utf8ToUtf16(const std::string& utf8Str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.from_bytes(utf8Str);
}

bool Rapl::GetIPGLocation(std::string& strLocation)
{
	char * val = getenv("IPG_Dir");
	std::string rapl_dir = std::string(val);
	if (rapl_dir.length() > 2) {
#if _M_X64
		strLocation = std::string(val) + "\\EnergyLib64.dll";
#else
		strLocation = std::string(val) + "\\EnergyLib32.dll";
#endif
		return true;
	}
	else
	{
		return false;
	}
}

Rapl::Rapl(){

    std::string strLocation;
    if (GetIPGLocation(strLocation) == false)
    {
        g_lastError = L"Intel Power Gadget 2.7 or higher not found.";
        return;
    }

    g_hModule = LoadLibrary(_T(strLocation.c_str()));
    if (g_hModule == NULL)
    {
        g_lastError = L"LoadLibrary failed on " + utf8ToUtf16(strLocation);
        return;
    }

    IGetNumNodes pGetNumNodes = (IGetNumNodes)GetProcAddress(g_hModule, "GetNumNodes");
    IIsGTAvailable pIsGTAvailable = (IIsGTAvailable)GetProcAddress(g_hModule, "IsGTAvailable");
    IInitialize pInitialize = (IInitialize)GetProcAddress(g_hModule, "IntelEnergyLibInitialize");

    if (pInitialize() == false) {
        std::cout << "Library error!" << std::endl;
        return;
    }

    pp1_supported = pIsGTAvailable();
    int numCPUnodes = 99;
    pGetNumNodes(&numCPUnodes);
    if (numCPUnodes > 1)
    {
		socket1_detected = true;
    }
}

bool Rapl::detect_igp() {
    return pp1_supported;
}

bool Rapl::detect_socket1() {
    return socket1_detected;
}

uint32_t Rapl::get_TDP() {
    IGetTDP pGetTDP = (IGetTDP)GetProcAddress(g_hModule, "GetTDP");
    double CPU_TDP=0;
    pGetTDP(0,&CPU_TDP); //Hardcoded to Socket 0
    return (uint32_t)round(CPU_TDP);
}

int32_t Rapl::get_NumMSR() {

    int32_t nMsrs = 0;
    IGetNumMsrs pGetNumMsrs = (IGetNumMsrs)GetProcAddress(g_hModule, "GetNumMsrs");
    pGetNumMsrs(&nMsrs);
    return nMsrs;
}


void Rapl::sample() {
    IReadSample pReadSample = (IReadSample)GetProcAddress(g_hModule, "ReadSample");
    pReadSample();
}


int32_t Rapl::get_temp0() {
    int32_t degC;
    IGetTemperature pGetTemperature = (IGetTemperature)GetProcAddress(g_hModule, "GetTemperature");
    pGetTemperature(0, &degC);
    return degC;
}

int32_t Rapl::get_temp1() {
    int32_t degC;
    IGetTemperature pGetTemperature = (IGetTemperature)GetProcAddress(g_hModule, "GetTemperature");
    pGetTemperature(1, &degC);
    return degC;
}

bool Rapl::GetPowerData(int iNode, int iMSR, double *results, int *nResult)
{
    IGetPowerData pGetPowerData = (IGetPowerData)GetProcAddress(g_hModule, "GetPowerData");
    return pGetPowerData(iNode, iMSR, results, nResult);
}

bool Rapl::GetMsrName(int iMsr, wchar_t *pszName)
{
    IGetMsrName pGetMsrName = (IGetMsrName)GetProcAddress(g_hModule, "GetMsrName");
    return pGetMsrName(iMsr, pszName);
}

bool Rapl::GetMsrFunc(int iMsr, int *funcID)
{

    IGetMsrFunc pGetMsrFunc = (IGetMsrFunc)GetProcAddress(g_hModule, "GetMsrFunc");
    return pGetMsrFunc(iMsr, funcID);
}

#else

#include <unistd.h>
#include <sys/time.h>

Rapl::Rapl() {
  uint32_t core_id0 = 0;
  uint32_t core_id1 = 0;

  pp1_supported = detect_igp();
  open_msr(core_id0, fd0);

  std::ifstream fileInput;
  std::string line;

  // open /proc/cpuinfo to parse cpu configuration
  fileInput.open("/proc/cpuinfo");
  if(fileInput.is_open()) {
    bool first_found = false;
    uint32_t pid,pid_0 = 0;
    uint32_t curLine = 0;
    while (getline(fileInput, line)) {
      curLine++;
      size_t npos = line.find("physical id", 0);
      if (npos != std::string::npos) {
        npos = line.find(":");
        std::string token = line.substr(npos + 1, line.length() - npos - 1);

        sscanf (token.c_str(), "%d", &pid);
        if (first_found == false) {
          first_found = true;
          pid_0 = pid;
        } else {
          if (!(pid == pid_0))  // this core has a different socket
          {
            socket1_detected = true;
            open_msr(core_id1, fd1);
            break;
          }
        }

        core_id1++;
      }

    }
    fileInput.close();
  }
  else {
    std::cerr << "Unable to open /proc/cpuinfo.";
  }

  /* Read MSR_RAPL_POWER_UNIT Register */
  uint64_t raw_value = read_msr(MSR_RAPL_POWER_UNIT, fd0);
  power_units = pow(0.5, (double)(raw_value & 0xf));
  energy_units = pow(0.5, (double)((raw_value >> 8) & 0x1f));
  time_units = pow(0.5, (double)((raw_value >> 16) & 0xf));

  /* Read MSR_PKG_POWER_INFO Register */
  raw_value = read_msr(MSR_PKG_POWER_INFO, fd0);
  thermal_spec_power = power_units * ((double)(raw_value & 0x7fff));
  minimum_power = power_units * ((double)((raw_value >> 16) & 0x7fff));
  maximum_power = power_units * ((double)((raw_value >> 32) & 0x7fff));
  time_window = time_units * ((double)((raw_value >> 48) & 0x7fff));

  /* Read MSR_TEMPERATURE_TARGET Register */
  raw_value = read_msr(MSR_TEMPERATURE_TARGET, fd0);
  temp_target = (raw_value >> 16) & (uint64_t)(255); // get 8 bits starting at bit 16
}



bool Rapl::detect_igp() {
  uint64_t data;

  if (pread(fd0, &data, sizeof(data), MSR_PP1_ENERGY_STATUS) != sizeof(data)) {
    return false;
  }
  return true;
}

bool Rapl::detect_socket1() {
  return socket1_detected;
}


void Rapl::open_msr(unsigned int node, int &fd) {
  std::stringstream filename_stream;
  filename_stream << "/dev/cpu/" << node << "/msr";
  fd = open(filename_stream.str().c_str(), O_RDONLY);
  if (fd < 0) {
    if (errno == ENXIO) {
      fprintf(stderr, "rdmsr: No CPU %d\n", node);
      exit(2);
    } else if (errno == EIO) {
      fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", node);
      exit(3);
    } else {
      perror("rdmsr:open");
      fprintf(stderr, "Trying to open %s\n",
      filename_stream.str().c_str());
      exit(127);
    }
  }
}

uint64_t Rapl::read_msr(int msr_offset, int &fd) {
  uint64_t data;
  if (pread(fd, &data, sizeof(data), msr_offset) != sizeof(data)) {
    perror("read_msr():pread");
    exit(127);
  }
  return data;
}

void Rapl::sample() {
  uint32_t max_int = ~((uint32_t) 0);

  pkg_0 = read_msr(MSR_PKG_ENERGY_STATUS,fd0) & max_int;
  pp0_0 = read_msr(MSR_PP0_ENERGY_STATUS,fd0) & max_int;
  dram_0 = read_msr(MSR_DRAM_ENERGY_STATUS,fd0) & max_int;

  if (pp1_supported) {
    pp1_0 = read_msr(MSR_PP1_ENERGY_STATUS,fd0) & max_int;
  } else {
    pp1_0 = 0;
  }

  if (socket1_detected == true) {
    pkg_1 = read_msr(MSR_PKG_ENERGY_STATUS,fd1) & max_int;
    pp0_1 = read_msr(MSR_PP0_ENERGY_STATUS,fd1) & max_int;
    dram_1 = read_msr(MSR_DRAM_ENERGY_STATUS,fd1) & max_int;

    if (pp1_supported) {
      pp1_1 = read_msr(MSR_PP1_ENERGY_STATUS,fd1) & max_int;
    } else {
      pp1_1 = 0;
    }
  }

}

bool Rapl::get_socket0_data(uint64_t &Epkg, uint64_t &Epp0, uint64_t &Epp1, uint64_t &Edram) {
  Epkg = pkg_0;
  Epp0 = pp0_0;
  Epp1 = pp1_0;
  Edram = dram_0;
  return true;
}

bool Rapl::get_socket1_data(uint64_t &Epkg , uint64_t &Epp0 , uint64_t &Epp1 , uint64_t &Edram) {
  Epkg = pkg_1;
  Epp0 = pp0_1;
  Epp1 = pp1_1;
  Edram = dram_1;
  return true;
}

double Rapl::get_e_unit() {
  return energy_units;
}

uint32_t Rapl::get_TDP() {
  return (uint32_t)round(thermal_spec_power);
}


int32_t Rapl::get_temp0() {
  uint64_t raw_value = read_msr(IA32_PACKAGE_THERM_STATUS, fd0);
  uint32_t temp = (raw_value >> 16) & (uint64_t)(127); // get 7 bits starting at bit 16
  return temp_target - temp; // temp is the number of degrees Celsius below the thermal throttling temperature
}

int32_t Rapl::get_temp1() {
  uint64_t raw_value = read_msr(IA32_PACKAGE_THERM_STATUS, fd1);
  uint32_t temp = (raw_value >> 16) & (uint64_t)(127); // get 7 bits starting at bit 16
  return temp_target - temp; // temp is the number of degrees Celsius below the thermal throttling temperature
}

#endif
