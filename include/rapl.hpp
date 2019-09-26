#ifndef RAPL_H_
#define RAPL_H_

#if defined(_WIN32)
#pragma once
#include <Windows.h>
#include <vector>
#include <tchar.h>
#include <codecvt>
class Rapl {
private:
    HMODULE g_hModule = NULL;
    std::wstring g_lastError;
    bool GetIPGLocation(std::string& strLocation);
    std::wstring utf8ToUtf16(const std::string& utf8Str);
    bool pp1_supported = false;
    bool socket1_detected = false;

    typedef bool(*IInitialize) ();
    typedef bool(*IGetTDP) (int iNode, double *TDP);
    typedef bool(*IGetNumNodes) (int *nNodes);
    typedef bool(*IGetNumMsrs) (int *nMsr);
    typedef bool(*IReadSample) ();
    typedef bool(*IIsGTAvailable) ();
    typedef bool(*IGetMsrName) (int iMsr, wchar_t *szName);
    typedef bool(*IGetMsrFunc) (int iMsr, int *funcID);
    typedef bool(*IGetPowerData) (int iNode, int iMSR, double *result, int *nResult);
    typedef bool(*IGetTemperature) (int iNode, int *degreeC);

    IInitialize pInitialize;
    IGetTDP pGetTDP;
    IGetNumNodes pGetNumNodes;
    IGetNumMsrs pGetNumMsrs;
    IReadSample pReadSample;
    IIsGTAvailable pIsGTAvailable;
    IGetMsrName pGetMsrName;
    IGetTemperature pGetTemperature;
    IGetPowerData pGetPowerData;
    IGetMsrFunc pGetMsrFunc;

public:
    Rapl();
    bool detect_igp();
    bool detect_socket1();
    uint32_t get_TDP();
    int32_t get_NumMSR();
    bool GetMsrName(int iMsr, wchar_t *pszName);
    bool GetMsrFunc(int iMsr, int *funcID);
    void sample();
    int32_t get_temp0();
    int32_t get_temp1();
    bool GetPowerData(int iNode, int iMSR, double *results, int *nResult);
};

#elif defined(USEIPG) /* Mac OS */

#include <unistd.h>

class Rapl {
private:
  bool pp1_supported = false;
  bool socket1_detected = false;

public:
  Rapl();
  bool detect_igp();
  bool detect_socket1();
  uint32_t get_TDP();
  int32_t get_NumMSR();
  bool GetMsrName(int iMsr, char *pszName);
  bool GetMsrFunc(int iMsr, int *funcID);
  void sample();
  int32_t get_temp0();
  int32_t get_temp1();
  bool GetPowerData(int iNode, int iMSR, double *results, int *nResult);
};

#else /* probably Linux */

#include <unistd.h>
#include <cstdint>

class Rapl {

private:
  int fd0;
  int fd1;
  bool pp1_supported = false;
  bool socket1_detected = false;

  int temp_target;
  double power_units, energy_units, time_units;
  double thermal_spec_power, minimum_power, maximum_power, time_window;

  uint64_t pkg_0;
  uint64_t pp0_0;
  uint64_t pp1_0;
  uint64_t dram_0;

  uint64_t pkg_1;
  uint64_t pp0_1;
  uint64_t pp1_1;
  uint64_t dram_1;

  void open_msr(unsigned int node, int & fd);
  uint64_t read_msr(int msr_offset, int & fd);


public:
  Rapl();
  void sample();

  bool get_socket0_data(uint64_t &Epkg, uint64_t &Epp0, uint64_t &Epp1, uint64_t &Edram);
  bool get_socket1_data(uint64_t &Epkg, uint64_t &Epp0, uint64_t &Epp1, uint64_t &Edram);
  double get_e_unit();
  uint32_t get_TDP();
  int32_t get_temp0();
  int32_t get_temp1();
  bool detect_igp();
  bool detect_socket1();
};
#endif

#endif /* RAPL_H_ */
