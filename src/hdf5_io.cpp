/* TODO: Procide a license note */

//#include "../include/util.hpp"
#include "../include/hdf5_io.hpp"
#include "../include/main.hpp"
#include <sys/stat.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <sstream>
#include <vector>
#include <math.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "hdf5.h"
#include "hdf5_hl.h"

#define chunk_factor 64

using namespace std;

inline bool FileExists(const char* filename) //Function to check whether a file already exists
{
  struct stat fileInfo;
  return stat(filename, &fileInfo) == 0;
}



bool h5_check_object(const char* filename, const char* varname)
{
	hid_t h5_file_id;

  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

    if (H5LTpath_valid(h5_file_id, varname, true) > 0) {
      H5Fclose(h5_file_id);
      return true;
    }
    else {
      H5Fclose(h5_file_id);
      return false;
    }
  }

  std::cerr << "File not found " << std::endl;
  return false;
}

uint8_t h5_read_buffer_float(const char* filename, const char* varname, void* data)
{
  hid_t   h5_file_id, dataset_id,dataspace_id,memspace_id;
  hsize_t hdf_dims[4], count[4], offset[4];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    H5LTread_dataset(h5_file_id, varname, H5T_NATIVE_FLOAT,data);
    //TODO: check if varname was found - no idea what error code to use if not
    H5Fclose(h5_file_id);
    return 1;
  }
  else {
    std::cerr << "File not found " << std::endl;
    // File not found - no idea what error code to use
    return 0;
  }
}

uint8_t h5_read_buffer_double(const char* filename, const char* varname, void* data)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[4], count[4], offset[4];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    H5LTread_dataset(h5_file_id, varname, H5T_NATIVE_DOUBLE, data);
    //TODO: check if varname was found - no idea what error code to use if not
    H5Fclose(h5_file_id);
    return 1;
  }
  else {
    std::cerr << "File not found " << std::endl;
    //TODO: File not found - no idea what error code to use
    return 0;
  }
}

uint8_t h5_read_buffer_uint(const char* filename, const char* varname, uint32_t* data)
{
  hid_t   h5_file_id, dataset_id,dataspace_id,memspace_id;
  hsize_t hdf_dims[4], count[4], offset[4];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDONLY, H5P_DEFAULT);
    H5LTread_dataset(h5_file_id, varname, H5T_NATIVE_UINT, data);
    //TODO: check if varname was found - no idea what error code to use if not
    H5Fclose(h5_file_id);
    return 1;
  }
  else {
    std::cerr << "File not found " << std::endl;
    //TODO: File not found - no idea what error code to use
    return 0;
  }
}

uint8_t h5_read_buffer_int(const char* filename, const char* varname, int32_t* data)
{
  hid_t   h5_file_id, dataset_id,dataspace_id,memspace_id;
  hsize_t hdf_dims[4], count[4], offset[4];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDONLY, H5P_DEFAULT);
    H5LTread_dataset(h5_file_id, varname, H5T_NATIVE_INT, data);
    //TODO: check if varname was found - no idea what error code to use if not
    H5Fclose(h5_file_id);
    return 1;
  }
  else {
    std::cerr << "File not found " << std::endl;
    //TODO: File not found - no idea what error code to use
    return 0;
  }
}

uint8_t h5_read_buffer_char(const char* filename, const char* varname, cl_char* data)
{
  hid_t   h5_file_id, dataset_id,dataspace_id,memspace_id;
  hsize_t hdf_dims[4], count[4], offset[4];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression


  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDONLY, H5P_DEFAULT);
    H5LTread_dataset(h5_file_id, varname, H5T_NATIVE_CHAR, data);
    //TODO: check if varname was found - no idea what error code to use if not
    H5Fclose(h5_file_id);
    return 1;
  }
  else {
    std::cerr << "File not found " << std::endl;
    //TODO: File not found - no idea what error code to use
    return 0;
  }
}

uint8_t h5_read_buffer_uchar(const char* filename, const char* varname, unsigned char* data)
{
  hid_t   h5_file_id, dataset_id,dataspace_id,memspace_id;
  hsize_t hdf_dims[4],count[4],offset[4];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  if (FileExists(filename)){
    h5_file_id = H5Fopen(filename,H5F_ACC_RDONLY, H5P_DEFAULT);
    H5LTread_dataset(h5_file_id, varname, H5T_NATIVE_UCHAR, data);
    //TODO: check if varname was found - no idea what error code to use if not
    H5Fclose(h5_file_id);
    return 1;
  }
  else {
    std::cerr << "File not found " << std::endl;
    //TODO: File not found - no idea what error code to use
    return 0;
  }
}

uint8_t h5_read_string(const char* filename, const char* varname, char *buffer)
{
  hid_t h5_file_id;
  float param_value;

  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    hid_t dataset= H5Dopen(h5_file_id, varname, H5P_DEFAULT);

    H5LTread_dataset_string(h5_file_id, varname, buffer);
    //check if varname was found - no idea what error code to use if not

    H5Fclose(h5_file_id);

    return 1;
  }
  else {
    std::cerr << "File not found " << std::endl;
    //TODO: File not found - no idea what error code to use
    return 0;
  }
}


uint8_t h5_write_string(const char * filename, const char* varname, const char *buffer)
{
  hid_t h5_file_id;
  float param_value;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  H5LTmake_dataset_string(h5_file_id, varname, buffer);
  //check if varname was found - no idea what error code to use if not

  H5Fclose(h5_file_id);

  return 1;
}

uint8_t h5_read_strings(const char* filename, const char* varname, std::vector<std::string>& lines)
{
  hid_t h5_file_id, memspace;
  float param_value;
  hsize_t count[2];       /* size of subset in the file */
  hsize_t offset[2];      /* subset offset in the file */
  hsize_t stride[2];
  hsize_t block[2];
  hsize_t out_size[1];
  hsize_t out_off[1];

  if (FileExists(filename)) {

    h5_file_id = H5Fopen(filename,H5F_ACC_RDONLY, H5P_DEFAULT);
    hid_t dataset = H5Dopen(h5_file_id,varname, H5P_DEFAULT);
    hid_t dataspace = H5Dget_space(dataset);
    unsigned int ndims = H5Sget_simple_extent_ndims(dataspace);
    hsize_t* dims = new hsize_t[ndims];
    H5Sget_simple_extent_dims(dataspace,dims,NULL);
    hid_t datatype = H5Dget_type(dataset);
    hid_t native_type = H5Tget_native_type(datatype, H5T_DIR_ASCEND);
    //std::cout<<"Entires:"<<dims[0]<<std::endl;
    count[0]=1;
    count[1]=4;
    offset[1]=0;
    stride[0]=1;
    stride[1]=1;
    block[0]=1;
    block[1]=1;
    out_size[0]=4;
    out_off[0]=0;
    //memspace = H5Screate_simple(1,out_size,NULL);
    //H5Sselect_hyperslab(memspace, H5S_SELECT_SET, out_off, NULL, out_size, NULL);
	  const unsigned int max_buffer_size = 900000;
    char buffer[max_buffer_size]; //TODO: possible buffer overflow?

    H5LTread_dataset_string(h5_file_id,varname,buffer);
    // H5Dread(dataset, H5T_C_S1, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

    unsigned int line_length = H5Tget_size(datatype);
    unsigned int lines_found = 0;

    unsigned int str_start = 0;
    for (unsigned int i = 0; i < dims[0]; i++) {
      char subbuff[65535]; //TODO: possible buffer overflow?
      memcpy(subbuff, &buffer[str_start], line_length);
      // printf("%s\n", subbuff);
      lines.push_back(subbuff);
      // std::cout << varname <<":"<<H5Tget_size(datatype)<<":"<<i<< "       "<<str_start <<"   "<< i - str_start << std::endl;
      str_start += line_length;
      lines_found++;
    }


    // for (uint i=0;i<1000;i++) {
    //      offset[0]=i;

    //H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,NULL, count, NULL);

    //H5Dread (dataset, H5T_C_S1, H5S_ALL, dataspace, H5P_DEFAULT, buffer);
    //std::string test = "test1234567u89";
    //test.resize(4000);
    //H5Dread (dataset, H5T_C_S1, H5S_ALL, H5S_ALL, H5P_DEFAULT,(void*) test.c_str());
    //std::cout<<strlen(buffer)<<std::endl;
    //std::cout<<(char)buffer[6];
    //std::cout<<kernels.size()<<std::endl;
    //   }
    H5Sclose(dataspace);

    //H5LTread_dataset_string(h5_file_id,varname,buffer);
    //check if varname was found - no idea what error code to use if not

    H5Fclose(h5_file_id);

    return 1;
  }
  else {
    //TODO: File not found - no idea what error code to use
    return 0;
  }
}


float h5_read_single_float(const char * filename, const char* varname)
{
hid_t h5_file_id;
float param_value;


    // for (uint i=0;i<1000;i++) {
    //      offset[0]=i;
    //
    //H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,NULL, count, NULL);
    //
    //H5Dread (dataset, H5T_C_S1, H5S_ALL, dataspace, H5P_DEFAULT, buffer);
    //std::string test = "test1234567u89";
    //test.resize(4000);
    //H5Dread (dataset, H5T_C_S1, H5S_ALL, H5S_ALL, H5P_DEFAULT,(void*) test.c_str());
    //std::cout<<strlen(buffer)<<std::endl;
    //std::cout<<(char)buffer[6];
    //std::cout<<kernels.size()<<std::endl;
    //   }
    H5Sclose(dataspace);

    //H5LTread_dataset_string(h5_file_id,varname,buffer);
    //check if varname was found - no idea what error code to use if not

    H5Fclose(h5_file_id);

    return 1;
  }
  else {
    //File not found - no idea what error code to use
   return 0;
  }
}


float h5_read_single_float(const char* filename, const char* varname)
{
  hid_t h5_file_id;
  float param_value;

  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDONLY, H5P_DEFAULT);
    H5LTread_dataset_float(h5_file_id, varname, &param_value);
    //check if varname was found - no idea what error code to use if not
    H5Fclose(h5_file_id);
    return param_value;
  }
  else {
    //File not found - no idea what error code to use
    return -0.0;
  }
}

uint8_t h5_write_single_double(const char* filename, const char* varname, double data)
{
  hid_t h5_file_id;
  hsize_t hdf_dims[2];

  hdf_dims[0] = 1;
  hdf_dims[1] = 0;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDWR , H5P_DEFAULT);
  }

  H5LTmake_dataset_double(h5_file_id,varname,1,hdf_dims,&data);

  H5Fclose(h5_file_id);

  return 1;
}

uint8_t h5_write_single_long(const char* filename, const char* varname, long data)
{
  hid_t h5_file_id;
  hsize_t  hdf_dims[2];

  hdf_dims[0] = 1;
  hdf_dims[1] = 0;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDWR , H5P_DEFAULT);
  }

  H5LTmake_dataset_long(h5_file_id,varname,1,hdf_dims,&data);

  H5Fclose(h5_file_id);

  return 1;
}

uint8_t h5_write_single_float(const char* filename, const char* varname, float data)
{
  hid_t h5_file_id;
  hsize_t  hdf_dims[2];

  hdf_dims[0] = 1;
  hdf_dims[1] = 0;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDWR , H5P_DEFAULT);
  }

  H5LTmake_dataset_float(h5_file_id,varname,1,hdf_dims,&data);

  H5Fclose(h5_file_id);

  return 1;
}


uint8_t h5_write_buffer_float4(const char* filename, const char* varname, cl_float4* data, cl_ulong size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  hdf_dims[0] = size;
  hdf_dims[1] = 4;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR , H5P_DEFAULT);
  }

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = 4;

  plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(plist_id, 2, cdims);
  H5Pset_deflate(plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id, varname , H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);

  memspace_id = H5Screate_simple(2, hdf_dims, NULL);

  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, memspace_id, dataspace_id, H5P_DEFAULT, data);

  H5Sclose(memspace_id);

  H5Sclose(dataspace_id);
  H5Dclose(dataset_id);
  H5Pclose(plist_id);

  //The same can be done using H5 High Level API, but without compression
  //H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, H5T_NATIVE_FLOAT, data);

  H5Fclose(h5_file_id);

  return 1;
}


uint8_t h5_write_buffer_double4(const char* filename, const char* varname, cl_double4* data, cl_ulong size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  hdf_dims[0] = size;
  hdf_dims[1] = 4;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDWR , H5P_DEFAULT);
  }

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = 4;

  plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(plist_id, 2, cdims);
  H5Pset_deflate(plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id,varname , H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);

  memspace_id = H5Screate_simple(2, hdf_dims, NULL);

  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, memspace_id,dataspace_id, H5P_DEFAULT, data);

  H5Sclose(memspace_id);

  H5Sclose(dataspace_id);
  H5Dclose (dataset_id);
  H5Pclose (plist_id);

  //The same can be done using H5 High Level API, but without compression
  //H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, H5T_NATIVE_FLOAT, data);

  H5Fclose(h5_file_id);

  return 1;
}


uint8_t h5_write_buffer_uint4(const char* filename, const char* varname, cl_uint4* data, cl_ulong size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  hdf_dims[0] = size;
  hdf_dims[1] = 4;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = 4;

  plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(plist_id, 2, cdims);
  H5Pset_deflate (plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id, varname, H5T_NATIVE_UINT, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);

  memspace_id = H5Screate_simple(2, hdf_dims, NULL);

  H5Dwrite(dataset_id, H5T_NATIVE_UINT, memspace_id,dataspace_id, H5P_DEFAULT, data);

  H5Sclose(memspace_id);

  H5Sclose(dataspace_id);
  H5Dclose(dataset_id);
  H5Pclose(plist_id);

  //The same can be done using H5 High Level API, but without compression
  //H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, H5T_NATIVE_FLOAT, data);

  H5Fclose(h5_file_id);

  return 1;
}


uint8_t h5_write_buffer_float(const char* filename, const char* varname, float* data, cl_ulong size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  hdf_dims[0] = size;
  hdf_dims[1] = 1;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = 1;

  plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(plist_id, 2, cdims);
  H5Pset_deflate (plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id, varname , H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
  memspace_id = H5Screate_simple(2, hdf_dims, NULL);

  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, memspace_id,dataspace_id, H5P_DEFAULT, data);

  H5Sclose(memspace_id);

  H5Sclose(dataspace_id);
  H5Dclose(dataset_id);
  H5Pclose(plist_id);

  //The same can be done using H5 High Level API, but without compression
  //H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, H5T_NATIVE_FLOAT, data);

  H5Fclose(h5_file_id);

  return 1;
}

uint8_t h5_write_buffer_double(const char* filename, const char* varname, double* data, cl_ulong size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  hdf_dims[0] = size;
  hdf_dims[1] = 1;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDWR , H5P_DEFAULT);
  }

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = 1;

  plist_id  = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk (plist_id, 2, cdims);
  H5Pset_deflate (plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id, varname , H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
  memspace_id = H5Screate_simple(2, hdf_dims, NULL);

  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, memspace_id, dataspace_id, H5P_DEFAULT, data);

  H5Sclose(memspace_id);
  H5Sclose(dataspace_id);
  H5Dclose(dataset_id);
  H5Pclose(plist_id);

  //The same can be done using H5 High Level API, but without compression
  //H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, H5T_NATIVE_FLOAT, data);

  H5Fclose(h5_file_id);

  return 1;
}

uint8_t h5_write_buffer_uint(const char* filename, const char* varname, cl_uint* data, cl_ulong size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  hdf_dims[0] = size;
  hdf_dims[1] = 1;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = 1;

  plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk (plist_id, 2, cdims);
  H5Pset_deflate (plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id,varname , H5T_NATIVE_UINT, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
  memspace_id = H5Screate_simple(2, hdf_dims, NULL);

  H5Dwrite(dataset_id, H5T_NATIVE_UINT, memspace_id,dataspace_id, H5P_DEFAULT, data);

  H5Sclose(memspace_id);
  H5Sclose(dataspace_id);
  H5Dclose(dataset_id);
  H5Pclose(plist_id);

  //The same can be done using H5 High Level API, but without compression
  //H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, H5T_NATIVE_FLOAT, data);

  H5Fclose(h5_file_id);

  return 1;
}

uint8_t h5_write_buffer_int(const char* filename, const char* varname, cl_int* data, cl_ulong size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  hdf_dims[0] = size;
  hdf_dims[1] = 1;

  if (!FileExists(filename)) {
      h5_file_id = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = 1;

  plist_id  = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(plist_id, 2, cdims);
  H5Pset_deflate (plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id,varname , H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
  memspace_id = H5Screate_simple(2, hdf_dims, NULL);

  H5Dwrite(dataset_id, H5T_NATIVE_INT, memspace_id, dataspace_id, H5P_DEFAULT, data);

  H5Sclose (memspace_id);
  H5Sclose(dataspace_id);
  H5Dclose (dataset_id);
  H5Pclose (plist_id);

  //The same can be done using H5 High Level API, but without compression
  //H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, H5T_NATIVE_FLOAT, data);

  H5Fclose(h5_file_id);

  return 1;
}

uint8_t h5_write_buffer_char(const char* filename, const char* varname, cl_char* data, cl_ulong size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression
  hdf_dims[0] = size;
  hdf_dims[1] = 1;
  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDWR , H5P_DEFAULT);
  }

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = 1;

  plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk (plist_id, 2, cdims);
  H5Pset_deflate(plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id,varname , H5T_NATIVE_CHAR, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
  memspace_id = H5Screate_simple(2, hdf_dims, NULL);

  H5Dwrite(dataset_id, H5T_NATIVE_CHAR, memspace_id,dataspace_id, H5P_DEFAULT, data);

  H5Sclose (memspace_id);
  H5Sclose(dataspace_id);
  H5Dclose (dataset_id);
  H5Pclose (plist_id);

  //The same can be done using H5 High Level API, but without compression
  //H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, H5T_NATIVE_FLOAT, data);

  H5Fclose(h5_file_id);

  return 1;
}

uint8_t h5_write_buffer_uchar(const char* filename, const char* varname, cl_uchar* data, cl_ulong size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  hdf_dims[0] = size;
  hdf_dims[1] = 1;

  if (!FileExists(filename)) {
    h5_file_id = H5Fcreate (filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename,H5F_ACC_RDWR , H5P_DEFAULT);
  }

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = 1;

  plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk (plist_id, 2, cdims);
  H5Pset_deflate (plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id,varname , H5T_NATIVE_UCHAR, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);
  memspace_id = H5Screate_simple(2, hdf_dims, NULL);

  H5Dwrite(dataset_id, H5T_NATIVE_UCHAR, memspace_id,dataspace_id, H5P_DEFAULT, data);

  H5Sclose(memspace_id);
  H5Sclose(dataspace_id);
  H5Dclose(dataset_id);
  H5Pclose(plist_id);

  //The same can be done using H5 High Level API, but without compression
  //H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, H5T_NATIVE_FLOAT, data);

  H5Fclose(h5_file_id);

  return 1;
}


uint8_t h5_get_content(const char* filename, const char* hdf_dir,
                       std::vector<std::string> &data_list, std::vector<HD5_Type> &datatype_list, std::vector<size_t> &data_size)
{
  #define MAX_NAME 1024
  hid_t   h5_file_id, grp;
  ssize_t len;

  herr_t err;
  int otype;

  char group_name[MAX_NAME];
  char memb_name[MAX_NAME];

  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  }
  else {
    return 0;
  }

  grp = H5Gopen(h5_file_id, hdf_dir, H5P_DEFAULT);

  hsize_t nobj;
  hid_t dsid;
  err = H5Gget_num_objs(grp, &nobj);

  for (int i = 0; i < nobj; i++) {

    len = H5Gget_objname_by_idx(grp, (hsize_t)i, memb_name, (size_t)MAX_NAME );
    otype = H5Gget_objtype_by_idx(grp, (size_t)i );

    if (otype == H5G_DATASET) {
      sprintf(group_name, "%s%s", hdf_dir, memb_name);
      data_list.push_back(group_name);

      hid_t dataset = H5Dopen(grp, memb_name, H5P_DEFAULT);
      hid_t dataspace = H5Dget_space(dataset);
      unsigned int ndims = H5Sget_simple_extent_ndims(dataspace);
      hsize_t* dims = new hsize_t[ndims];
      H5Sget_simple_extent_dims(dataspace, dims, NULL);
      H5Sclose(dataspace);

      data_size.push_back((size_t)dims[0]* dims[1]);

      delete[] dims; dims = nullptr;

      hid_t datatype = H5Dget_type(dataset);

      hid_t native_type = H5Tget_native_type(datatype, H5T_DIR_ASCEND);

      if (H5Tequal(native_type,H5T_NATIVE_FLOAT)>0)
      {
        datatype_list.push_back(H5_float);
      }
      if (H5Tequal(native_type,H5T_NATIVE_DOUBLE)>0)
      {
        datatype_list.push_back(H5_double);
      }
      if (H5Tequal(native_type,H5T_NATIVE_CHAR)>0)
      {
        datatype_list.push_back(H5_char);
      }
      if (H5Tequal(native_type,H5T_NATIVE_UCHAR)>0)
      {
        datatype_list.push_back(H5_uchar);
      }
      if (H5Tequal(native_type,H5T_NATIVE_INT)>0)
      {
        datatype_list.push_back(H5_int);
      }
      if (H5Tequal(native_type,H5T_NATIVE_UINT)>0)
      {
        datatype_list.push_back(H5_uint);
      }
      if (H5Tequal(native_type,H5T_NATIVE_LONG)>0)
      {
        datatype_list.push_back(H5_long);
      }
      if (H5Tequal(native_type,H5T_NATIVE_ULONG)>0)
      {
        datatype_list.push_back(H5_ulong);
      }

      H5Dclose(dataset);
    }
  }

  H5Fclose(h5_file_id);

  return 1;
}


uint8_t h5_create_dir(const char* filename, const char* hdf_dir)
{
  hid_t h5_file_id, grp;

  if (FileExists(filename)) {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }
  else {
    return 0;
  }

  grp = H5Gcreate1(h5_file_id, hdf_dir, 0);
  H5Gclose(grp);
  H5Fclose(h5_file_id);

  return 1;
}
