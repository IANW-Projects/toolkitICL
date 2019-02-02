/* TODO: Provide a license note */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "hdf5.h"
#include "hdf5_hl.h"

#include "opencl_include.hpp"

#include "util.hpp"
#include "hdf5_io.hpp"

#define chunk_factor 64

using namespace std;


// convert a C type TYPE to the HDF5 identifier of that type
template<>
hid_t type_to_h5_type<float>() { return H5T_NATIVE_FLOAT; }

template<>
hid_t type_to_h5_type<double>() { return H5T_NATIVE_DOUBLE; }

template<>
hid_t type_to_h5_type<cl_char>() { return H5T_NATIVE_INT8; }

template<>
hid_t type_to_h5_type<cl_uchar>() { return H5T_NATIVE_UINT8; }

template<>
hid_t type_to_h5_type<cl_short>() { return H5T_NATIVE_INT16; }

template<>
hid_t type_to_h5_type<cl_ushort>() { return H5T_NATIVE_UINT16; }

template<>
hid_t type_to_h5_type<cl_int>() { return H5T_NATIVE_INT32; }

template<>
hid_t type_to_h5_type<cl_uint>() { return H5T_NATIVE_UINT32; }

template<>
hid_t type_to_h5_type<cl_long>() { return H5T_NATIVE_INT64; }

template<>
hid_t type_to_h5_type<cl_ulong>() { return H5T_NATIVE_UINT64; }

// OpenCL vector types
template<>
hid_t type_to_h5_type<cl_float4>() { return H5T_NATIVE_FLOAT; }
template<>
constexpr size_t get_vector_size<cl_float4>() { return 4; };

template<>
hid_t type_to_h5_type<cl_double4>() { return H5T_NATIVE_DOUBLE; }
template<>
constexpr size_t get_vector_size<cl_double4>() { return 4; };

template<>
hid_t type_to_h5_type<cl_uint4>() { return H5T_NATIVE_UINT; }
template<>
constexpr size_t get_vector_size<cl_uint4>() { return 4; };

// fallback;
template<typename TYPE>
constexpr size_t get_vector_size() { return 1; };


bool h5_check_object(const char* filename, const char* varname)
{
	hid_t h5_file_id;

  if (fileExists(filename)) {
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

  std::cerr << ERROR_INFO << "File '" << filename << "' not found." << std::endl;
  return false;
}


uint8_t h5_get_content(const char* filename, const char* hdf_dir,
                       std::vector<std::string> &data_list, std::vector<HD5_Type> &datatype_list, std::vector<size_t> &data_size)
{
  #define MAX_NAME 1024
  hid_t   h5_file_id, grp;
  ssize_t len;

  herr_t err;
  int otype;

  char group_name[MAX_NAME]; //TODO: possible buffer overflow?
  char memb_name[MAX_NAME];

  if (fileExists(filename)) {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  }
  else {
    std::cerr << ERROR_INFO << "File '" << filename << "' not found." << std::endl;
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

      if (H5Tequal(datatype, type_to_h5_type<float>()) > 0) {
        datatype_list.push_back(H5_float);
      }
      else if (H5Tequal(datatype, type_to_h5_type<double>()) > 0) {
        datatype_list.push_back(H5_double);
      }
      else if (H5Tequal(datatype, type_to_h5_type<cl_char>()) > 0) {
        datatype_list.push_back(H5_char);
      }
      else if (H5Tequal(datatype, type_to_h5_type<cl_uchar>()) > 0) {
        datatype_list.push_back(H5_uchar);
      }
      else if (H5Tequal(datatype, type_to_h5_type<cl_short>()) > 0) {
        datatype_list.push_back(H5_short);
      }
      else if (H5Tequal(datatype, type_to_h5_type<cl_ushort>()) > 0) {
        datatype_list.push_back(H5_ushort);
      }
      else if (H5Tequal(datatype, type_to_h5_type<cl_int>()) > 0) {
        datatype_list.push_back(H5_int);
      }
      else if (H5Tequal(datatype, type_to_h5_type<cl_uint>()) > 0) {
        datatype_list.push_back(H5_uint);
      }
      else if (H5Tequal(datatype, type_to_h5_type<cl_long>()) > 0) {
        datatype_list.push_back(H5_long);
      }
      else if (H5Tequal(datatype, type_to_h5_type<cl_ulong>()) > 0) {
        datatype_list.push_back(H5_ulong);
      }
      else {
        std::cerr << ERROR_INFO << "Data type '" << datatype << "' unknown." << std::endl;
      }

      H5Tclose(datatype);
      H5Dclose(dataset);
    }
  }

  H5Gclose(grp);
  H5Fclose(h5_file_id);

  return 1;
}


uint8_t h5_create_dir(const char* filename, const char* hdf_dir)
{
  hid_t h5_file_id, grp;

  if (fileExists(filename)) {
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


// read a buffer from an HDF5 file
template<typename TYPE>
bool h5_read_buffer(const char* filename, const char* varname, TYPE* data)
{
  if (!fileExists(filename)) {
    std::cerr << ERROR_INFO << "File '" << filename << "' not found." << std::endl;
    //TODO: Exception? Only error code?
    return false;
  }

  hid_t h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (H5LTpath_valid(h5_file_id, varname, true) <= 0) {
    std::cerr << ERROR_INFO << "Variable '" << varname << "' not found in file '" << filename << "'." << std::endl;
    //TODO: Exception? Only error code?
    H5Fclose(h5_file_id);
    return false;
  }

  int err = H5LTread_dataset(h5_file_id, varname, type_to_h5_type<TYPE>(), data);
  if (err < 0) {
    std::cerr << ERROR_INFO << "Reading variable '" << varname << "' in file '" << filename << "' not possible." << std::endl;
    //TODO: Exception? Only error code?
    H5Fclose(h5_file_id);
    return false;
  }

  H5Fclose(h5_file_id);
  return true;
}

// template instantiations
template bool h5_read_buffer(const char* filename, const char* varname, float* data);
template bool h5_read_buffer(const char* filename, const char* varname, double* data);
template bool h5_read_buffer(const char* filename, const char* varname, cl_char* data);
template bool h5_read_buffer(const char* filename, const char* varname, cl_uchar* data);
template bool h5_read_buffer(const char* filename, const char* varname, cl_short* data);
template bool h5_read_buffer(const char* filename, const char* varname, cl_ushort* data);
template bool h5_read_buffer(const char* filename, const char* varname, cl_int* data);
template bool h5_read_buffer(const char* filename, const char* varname, cl_uint* data);
template bool h5_read_buffer(const char* filename, const char* varname, cl_long* data);
template bool h5_read_buffer(const char* filename, const char* varname, cl_ulong* data);

// other forms
bool h5_read_buffer_float(const char* filename, const char* varname, float* data)
{
  return h5_read_buffer<float>(filename, varname, data);
}

bool h5_read_buffer_double(const char* filename, const char* varname, double* data)
{
  return h5_read_buffer<double>(filename, varname, data);
}

bool h5_read_buffer_int(const char* filename, const char* varname, cl_int* data)
{
  return h5_read_buffer<cl_int>(filename, varname, data);
}

bool h5_read_buffer_uint(const char* filename, const char* varname, cl_uint* data)
{
  return h5_read_buffer<cl_uint>(filename, varname, data);
}

bool h5_read_buffer_char(const char* filename, const char* varname, cl_char* data)
{
  return h5_read_buffer<cl_char>(filename, varname, data);
}

bool h5_read_buffer_uchar(const char* filename, const char* varname, cl_uchar* data)
{
  return h5_read_buffer<cl_uchar>(filename, varname, data);
}


// write a buffer to an HDF5 file using compression
template<typename TYPE>
bool h5_write_buffer(const char* filename, const char* varname, TYPE const* data, size_t size)
{
  hid_t   h5_file_id, dataset_id, dataspace_id, memspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t cdims[2]; //chunk size used for compression

  if (!fileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  hdf_dims[0] = size;
  hdf_dims[1] = get_vector_size<TYPE>();

  cdims[0] = (int)(hdf_dims[0]/chunk_factor)+1;
  cdims[1] = hdf_dims[1];

  plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(plist_id, 2, cdims);
  H5Pset_deflate(plist_id, 9);

  dataspace_id = H5Screate_simple(2, hdf_dims, NULL);
  memspace_id = H5Screate_simple(2, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id, varname , type_to_h5_type<TYPE>(), dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);

  H5Dwrite(dataset_id, type_to_h5_type<TYPE>(), memspace_id, dataspace_id, H5P_DEFAULT, data);
  // The same can be done using H5 High Level API, but without compression
  // H5LTmake_dataset(h5_file_id, varname, 2, hdf_dims, type_to_h5_type<TYPE>(), data);

  // TODO: Check error codes?

  H5Pclose(plist_id);
  H5Sclose(dataspace_id);
  H5Sclose(memspace_id);
  H5Dclose(dataset_id);

  H5Fclose(h5_file_id);

  return 1;
}

// template instantiations
template bool h5_write_buffer(const char* filename, const char* varname, float const* data, size_t size);
template bool h5_write_buffer(const char* filename, const char* varname, double const* data, size_t size);
template bool h5_write_buffer(const char* filename, const char* varname, cl_char const* data, size_t size);
template bool h5_write_buffer(const char* filename, const char* varname, cl_uchar const* data, size_t size);
template bool h5_write_buffer(const char* filename, const char* varname, cl_short const* data, size_t size);
template bool h5_write_buffer(const char* filename, const char* varname, cl_ushort const* data, size_t size);
template bool h5_write_buffer(const char* filename, const char* varname, cl_int const* data, size_t size);
template bool h5_write_buffer(const char* filename, const char* varname, cl_uint const* data, size_t size);
template bool h5_write_buffer(const char* filename, const char* varname, cl_long const* data, size_t size);
template bool h5_write_buffer(const char* filename, const char* varname, cl_ulong const* data, size_t size);

// other forms
bool h5_write_buffer_float(const char* filename, const char* varname, float const* data, size_t size)
{
  return h5_write_buffer<float>(filename, varname, data, size);
}

bool h5_write_buffer_double(const char* filename, const char* varname, double const* data, size_t size)
{
  return h5_write_buffer<double>(filename, varname, data, size);
}

bool h5_write_buffer_int(const char* filename, const char* varname, cl_int const* data, size_t size)
{
  return h5_write_buffer<cl_int>(filename, varname, data, size);
}

bool h5_write_buffer_uint(const char* filename, const char* varname, cl_uint const* data, size_t size)
{
  return h5_write_buffer<cl_uint>(filename, varname, data, size);
}

bool h5_write_buffer_char(const char* filename, const char* varname, cl_char const* data, size_t size)
{
  return h5_write_buffer<cl_char>(filename, varname, data, size);
}

bool h5_write_buffer_uchar(const char* filename, const char* varname, cl_uchar const* data, size_t size)
{
  return h5_write_buffer<cl_uchar>(filename, varname, data, size);
}

bool h5_write_buffer_float4(const char* filename, const char* varname, cl_float4 const* data, size_t size)
{
  return h5_write_buffer<cl_float4>(filename, varname, data, size);
}

bool h5_write_buffer_double4(const char* filename, const char* varname, cl_double4 const* data, size_t size)
{
  return h5_write_buffer<cl_double4>(filename, varname, data, size);
}

bool h5_write_buffer_uint4(const char* filename, const char* varname, cl_uint4 const* data, size_t size)
{
  return h5_write_buffer<cl_uint4>(filename, varname, data, size);
}


// read a single item from an HDF5 file
// template<typename TYPE>
// TYPE h5_read_single(const char* filename, const char* varname);
// is defined in header

// other forms
float h5_read_single_float(const char* filename, const char* varname)
{
  return h5_read_single<float>(filename, varname);
}

float h5_read_single_double(const char* filename, const char* varname)
{
  return h5_read_single<double>(filename, varname);
}


// write a single item to an HDF5 file
// template<typename TYPE>
// bool h5_write_single(const char* filename, const char* varname, TYPE data);
// is defined in header

// other forms
bool h5_write_single_float(const char* filename, const char* varname, float data)
{
  return h5_write_single<float>(filename, varname, data);
}

bool h5_write_single_double(const char* filename, const char* varname, double data)
{
  return h5_write_single<double>(filename, varname, data);
}

bool h5_write_single_long(const char* filename, const char* varname, cl_long data)
{
  return h5_write_single<cl_long>(filename, varname, data);
}


// reading and writing single strings
bool h5_read_string(const char* filename, const char* varname, char* buffer)
{
  if (!fileExists(filename)) {
    std::cerr << ERROR_INFO << "File '" << filename << "' not found." << std::endl;
    //TODO: File not found - no idea what error code to use
    return false;
  }

  hid_t h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

  H5LTread_dataset_string(h5_file_id, varname, buffer);
  //TODO: check if varname was found - no idea what error code to use if not

  H5Fclose(h5_file_id);

  return true;
}

bool h5_write_string(const char* filename, const char* varname, const char* buffer)
{
  hid_t h5_file_id;

  if (!fileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  H5LTmake_dataset_string(h5_file_id, varname, buffer);
  //TODO: check if successful

  H5Fclose(h5_file_id);

  return true;
}


// reading and writing arrays of strings using the format of the (deprecated)
// matlab function hdfwrite for cell arrays of strings (aka char arrays)
bool h5_read_strings(const char* filename, const char* varname, std::vector<std::string>& lines)
{
  if (!fileExists(filename)) {
    std::cerr << ERROR_INFO << "File '" << filename << "' not found." << std::endl;
    //TODO: File not found - Exception? Error code?
    return false;
  }

  //TODO: increase; heap!
  constexpr size_t buffer_size(900000);
  constexpr size_t subbuffer_size(65535);

  hid_t h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

  // get information about the data
  hid_t dataset = H5Dopen(h5_file_id, varname, H5P_DEFAULT);
  hid_t dataspace = H5Dget_space(dataset);
  unsigned int ndims = H5Sget_simple_extent_ndims(dataspace);
  if (ndims != 1) {
    std::cerr << ERROR_INFO << "Error: Dataset '" << varname << "' in '" << filename << "' has a wrong format." << std::endl;
    H5Sclose(dataspace);
    H5Dclose(dataset);
    H5Fclose(h5_file_id);
    //TODO: Exception?
    return false;
  }
  hsize_t* dims = new hsize_t[ndims];
  H5Sget_simple_extent_dims(dataspace, dims, NULL);
  hid_t datatype = H5Dget_type(dataset);
  unsigned int line_length = H5Tget_size(datatype);

  H5Tclose(datatype);
  H5Sclose(dataspace);
  H5Dclose(dataset);

  // load the data
  char buffer[buffer_size]; //TODO: possible buffer overflow?

  H5LTread_dataset_string(h5_file_id, varname, buffer);
  //TODO: check if varname was found - no idea what error code to use if not

  unsigned int lines_found = 0;
  unsigned int str_start = 0;
  for (unsigned int i = 0; i < dims[0]; i++) {
    char subbuff[subbuffer_size];
    memcpy(subbuff, &buffer[str_start], line_length);
    lines.push_back(subbuff);
    str_start += line_length;
    lines_found++;
  }

  delete[] dims; dims = nullptr;
  H5Fclose(h5_file_id);

  return true;
}

bool h5_write_strings(const char* filename, const char* varname, std::vector<std::string> const& lines)
{
  constexpr size_t buffer_size(900000);

  // create single C string using the format of the (deprecated) matlab function
  // `hdf5write` for cell arrays of char arrays (aka strings)
  size_t line_length = std::max_element(
    lines.cbegin(), lines.cend(), [] (std::string s1, std::string s2) { return s1.size() < s2.size(); } )->size() + 1;

  char buffer[buffer_size] = {'\0'};
  if (line_length >= buffer_size) {
    std::cerr << ERROR_INFO << "Error: The strings are too long (length = " << line_length << ", buffer size = " << buffer_size << "." << std::endl;
    return false;
  }

  size_t str_start = 0;
  for (const string& line : lines) {
    strcpy(&buffer[str_start], line.c_str());
    str_start += line_length;
  }

  // save buffer and additional information
  hid_t h5_file_id;
  if (!fileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR , H5P_DEFAULT);
  }

  hsize_t hdf_dims[1] = {lines.size()};
  hid_t dataspace = H5Screate_simple(1, hdf_dims, NULL);
  hid_t datatype = H5Tcreate(H5T_STRING, line_length);
  hid_t dataset = H5Dcreate2(h5_file_id, varname, datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  H5Dwrite(dataset, datatype, dataspace, dataspace, H5P_DEFAULT, buffer);

  H5Dclose(dataset);
  H5Tclose(datatype);
  H5Sclose(dataspace);

  H5Fclose(h5_file_id);

  return 1;
}
