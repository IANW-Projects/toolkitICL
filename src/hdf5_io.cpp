/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */


#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "hdf5.h"
#include "hdf5_hl.h"

#include "opencl_include.hpp"

#include "util.hpp"
#include "hdf5_io.hpp"

#define chunk_factor 64

using namespace std;


// TODO: Check error codes for all H5... routines?
// TODO: Add HDF append functions


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
template<> hid_t type_to_h5_type<cl_float4>() { return H5T_NATIVE_FLOAT; }
template<> constexpr size_t get_vector_size<cl_float4>() { return 4; };

template<> hid_t type_to_h5_type<cl_double4>() { return H5T_NATIVE_DOUBLE; }
template<> constexpr size_t get_vector_size<cl_double4>() { return 4; };

template<> hid_t type_to_h5_type<cl_char4>() { return H5T_NATIVE_INT8; }
template<> constexpr size_t get_vector_size<cl_char4>() { return 4; };

template<> hid_t type_to_h5_type<cl_uchar4>() { return H5T_NATIVE_UINT8; }
template<> constexpr size_t get_vector_size<cl_uchar4>() { return 4; };

template<> hid_t type_to_h5_type<cl_short4>() { return H5T_NATIVE_INT16; }
template<> constexpr size_t get_vector_size<cl_short4>() { return 4; };

template<> hid_t type_to_h5_type<cl_ushort4>() { return H5T_NATIVE_UINT16; }
template<> constexpr size_t get_vector_size<cl_ushort4>() { return 4; };

template<> hid_t type_to_h5_type<cl_int4>() { return H5T_NATIVE_INT32; }
template<> constexpr size_t get_vector_size<cl_int4>() { return 4; };

template<> hid_t type_to_h5_type<cl_uint4>() { return H5T_NATIVE_UINT32; }
template<> constexpr size_t get_vector_size<cl_uint4>() { return 4; };

template<> hid_t type_to_h5_type<cl_long4>() { return H5T_NATIVE_INT64; }
template<> constexpr size_t get_vector_size<cl_long4>() { return 4; };

template<> hid_t type_to_h5_type<cl_ulong4>() { return H5T_NATIVE_UINT64; }
template<> constexpr size_t get_vector_size<cl_ulong4>() { return 4; };

// fallback;
template<typename TYPE>
constexpr size_t get_vector_size() { return 1; };


bool h5_check_object(char const* filename, char const* varname)
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


bool h5_get_content(char const* filename, char const* hdf_dir,
                    std::vector<std::string>& data_names, std::vector<HD5_Type>& data_types, std::vector<size_t>& data_sizes)
{
  if (!fileExists(filename)) {
    std::cerr << ERROR_INFO << "File '" << filename << "' not found." << std::endl;
    return false;
  }

  hid_t h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t grp = H5Gopen(h5_file_id, hdf_dir, H5P_DEFAULT);

  hsize_t nobj;
  H5Gget_num_objs(grp, &nobj);

  for (hsize_t obj_idx = 0; obj_idx < nobj; obj_idx++) {

    int object_type = H5Gget_objtype_by_idx(grp, obj_idx);
    if (object_type != H5G_DATASET) {
      continue;
    }

    ssize_t len = H5Gget_objname_by_idx(grp, obj_idx, NULL, 0);
    vector<char> object_name(len+1, '\0');
    H5Gget_objname_by_idx(grp, obj_idx, &(object_name[0]), len+1);

    data_names.push_back(string(hdf_dir) + string(object_name.begin(), object_name.end()));

    hid_t dataset = H5Dopen(grp, &(object_name[0]), H5P_DEFAULT);
    hid_t dataspace = H5Dget_space(dataset);
    int ndims = H5Sget_simple_extent_ndims(dataspace);
    vector<hsize_t> dims(ndims);
    H5Sget_simple_extent_dims(dataspace, &(dims[0]), NULL);
    H5Sclose(dataspace);

    data_sizes.push_back(accumulate(begin(dims), end(dims), 1, std::multiplies<hsize_t>()));

    hid_t datatype = H5Dget_type(dataset);
    if (H5Tequal(datatype, type_to_h5_type<float>()) > 0) {
      data_types.push_back(H5_float);
    }
    else if (H5Tequal(datatype, type_to_h5_type<double>()) > 0) {
      data_types.push_back(H5_double);
    }
    else if (H5Tequal(datatype, type_to_h5_type<cl_char>()) > 0) {
      data_types.push_back(H5_char);
    }
    else if (H5Tequal(datatype, type_to_h5_type<cl_uchar>()) > 0) {
      data_types.push_back(H5_uchar);
    }
    else if (H5Tequal(datatype, type_to_h5_type<cl_short>()) > 0) {
      data_types.push_back(H5_short);
    }
    else if (H5Tequal(datatype, type_to_h5_type<cl_ushort>()) > 0) {
      data_types.push_back(H5_ushort);
    }
    else if (H5Tequal(datatype, type_to_h5_type<cl_int>()) > 0) {
      data_types.push_back(H5_int);
    }
    else if (H5Tequal(datatype, type_to_h5_type<cl_uint>()) > 0) {
      data_types.push_back(H5_uint);
    }
    else if (H5Tequal(datatype, type_to_h5_type<cl_long>()) > 0) {
      data_types.push_back(H5_long);
    }
    else if (H5Tequal(datatype, type_to_h5_type<cl_ulong>()) > 0) {
      data_types.push_back(H5_ulong);
    }
    else {
      std::cerr << ERROR_INFO << "Data type '" << datatype << "' unknown." << std::endl;
      //TODO: Exception?
    }

    H5Tclose(datatype);
    H5Dclose(dataset);
  }

  H5Gclose(grp);
  H5Fclose(h5_file_id);

  return true;
}


bool h5_create_dir(char const* filename, char const* hdf_dir)
{
  hid_t h5_file_id, grp;

  if (fileExists(filename)) {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }
  else {
    return false;
  }

  grp = H5Gcreate1(h5_file_id, hdf_dir, 0);
  H5Gclose(grp);
  H5Fclose(h5_file_id);

  return true;
}


// read a buffer from an HDF5 file
template<typename TYPE>
bool h5_read_buffer(char const* filename, char const* varname, TYPE* data)
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

  herr_t err = H5LTread_dataset(h5_file_id, varname, type_to_h5_type<TYPE>(), data);
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
template bool h5_read_buffer(char const* filename, char const* varname, float* data);
template bool h5_read_buffer(char const* filename, char const* varname, double* data);
template bool h5_read_buffer(char const* filename, char const* varname, cl_char* data);
template bool h5_read_buffer(char const* filename, char const* varname, cl_uchar* data);
template bool h5_read_buffer(char const* filename, char const* varname, cl_short* data);
template bool h5_read_buffer(char const* filename, char const* varname, cl_ushort* data);
template bool h5_read_buffer(char const* filename, char const* varname, cl_int* data);
template bool h5_read_buffer(char const* filename, char const* varname, cl_uint* data);
template bool h5_read_buffer(char const* filename, char const* varname, cl_long* data);
template bool h5_read_buffer(char const* filename, char const* varname, cl_ulong* data);


// write a buffer to an HDF5 file using compression
template<typename TYPE>
bool h5_write_buffer(char const* filename, char const* varname, TYPE const* data, size_t size, std::string const& description)
{
  hid_t   h5_file_id, dataset_id, dataspace_id;
  hsize_t hdf_dims[2];
  hid_t   plist_id;
  hsize_t chunk_dims[2]; //chunk size used for compression
  int ndims;

  if (!fileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  hdf_dims[0] = size;
  hdf_dims[1] = get_vector_size<TYPE>();

  if (hdf_dims[1] == 1) {
    ndims = 1;
  }
  else {
    ndims = 2;
  }

  chunk_dims[0] = (hsize_t)(hdf_dims[0]/chunk_factor) + 1;
  chunk_dims[1] = hdf_dims[1];

  plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(plist_id, ndims, chunk_dims);
  H5Pset_deflate(plist_id, 9);

  dataspace_id = H5Screate_simple(ndims, hdf_dims, NULL);
  dataset_id = H5Dcreate2(h5_file_id, varname , type_to_h5_type<TYPE>(), dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);

  H5Dwrite(dataset_id, type_to_h5_type<TYPE>(), dataspace_id, dataspace_id, H5P_DEFAULT, data);
  // The same can be done using H5 High Level API, but without compression
  // H5LTmake_dataset(h5_file_id, varname, ndims, hdf_dims, type_to_h5_type<TYPE>(), data);

  if (!description.empty()) {
    H5LTset_attribute_string(h5_file_id, varname, "description", description.c_str());
  }

  H5Pclose(plist_id);
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);

  H5Fclose(h5_file_id);

  return true;
}

// template instantiations
template bool h5_write_buffer(char const* filename, char const* varname, float const* data, size_t size, std::string const& description);
template bool h5_write_buffer(char const* filename, char const* varname, double const* data, size_t size, std::string const& description);
template bool h5_write_buffer(char const* filename, char const* varname, cl_char const* data, size_t size, std::string const& description);
template bool h5_write_buffer(char const* filename, char const* varname, cl_uchar const* data, size_t size, std::string const& description);
template bool h5_write_buffer(char const* filename, char const* varname, cl_short const* data, size_t size, std::string const& description);
template bool h5_write_buffer(char const* filename, char const* varname, cl_ushort const* data, size_t size, std::string const& description);
template bool h5_write_buffer(char const* filename, char const* varname, cl_int const* data, size_t size, std::string const& description);
template bool h5_write_buffer(char const* filename, char const* varname, cl_uint const* data, size_t size, std::string const& description);
template bool h5_write_buffer(char const* filename, char const* varname, cl_long const* data, size_t size, std::string const& description);
template bool h5_write_buffer(char const* filename, char const* varname, cl_ulong const* data, size_t size, std::string const& description);




// read a single item from an HDF5 file
// template<typename TYPE>
// TYPE h5_read_single(char const* filename, char const* varname);
// is defined in header


// write a single item to an HDF5 file
template<typename TYPE>
bool h5_write_single(char const* filename, char const* varname, TYPE data, std::string const& description)
{
  hid_t h5_file_id;

  if (!fileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  H5LTmake_dataset(h5_file_id, varname, 0, NULL, type_to_h5_type<TYPE>(), &data);

  if (!description.empty()) {
    H5LTset_attribute_string(h5_file_id, varname, "description", description.c_str());
  }

  H5Fclose(h5_file_id);

  return true;
}

// template instantiations
template bool h5_write_single(char const* filename, char const* varname, float data, std::string const& description);
template bool h5_write_single(char const* filename, char const* varname, double data, std::string const& description);
template bool h5_write_single(char const* filename, char const* varname, cl_char data, std::string const& description);
template bool h5_write_single(char const* filename, char const* varname, cl_uchar data, std::string const& description);
template bool h5_write_single(char const* filename, char const* varname, cl_short data, std::string const& description);
template bool h5_write_single(char const* filename, char const* varname, cl_ushort data, std::string const& description);
template bool h5_write_single(char const* filename, char const* varname, cl_int data, std::string const& description);
template bool h5_write_single(char const* filename, char const* varname, cl_uint data, std::string const& description);
template bool h5_write_single(char const* filename, char const* varname, cl_long data, std::string const& description);
template bool h5_write_single(char const* filename, char const* varname, cl_ulong data, std::string const& description);


// reading and writing single strings
bool h5_read_string(char const* filename, char const* varname, std::string& output)
{
  if (!fileExists(filename)) {
    std::cerr << ERROR_INFO << "File '" << filename << "' not found." << std::endl;
    //TODO: File not found - no idea what error code to use
    return false;
  }

  hid_t h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t dataset = H5Dopen(h5_file_id, varname, H5P_DEFAULT);

  hid_t datatype = H5Dget_type(dataset);
  bool variable_length = H5Tis_variable_str(datatype);

  hid_t dataspace = H5Dget_space(dataset);

  if (variable_length) {
    int ndims = H5Sget_simple_extent_ndims(dataspace);
    vector<hsize_t> dims(ndims);
    H5Sget_simple_extent_dims(dataspace, &(dims[0]), NULL);
    hsize_t size = accumulate(begin(dims), end(dims), 1, std::multiplies<hsize_t>());

    std::vector<char*> buffer(size * sizeof(char*));
    H5Dread(dataset, datatype, dataspace, dataspace, H5P_DEFAULT, &(buffer[0]));
    output = std::string(buffer.at(0));

    H5Dvlen_reclaim(datatype, dataspace, H5P_DEFAULT, &(buffer[0]));
  }
  else {
    size_t datatype_size = H5Tget_size(datatype);
    hssize_t npoints = H5Sget_simple_extent_npoints(dataspace);

    std::vector<char> buffer(datatype_size * npoints, '\0');
    H5Dread(dataset, datatype, dataspace, dataspace, H5P_DEFAULT, &(buffer[0]));
    output = std::string(begin(buffer), end(buffer));
  }

  H5Sclose(dataspace);
  H5Tclose(datatype);
  H5Dclose(dataset);
  H5Fclose(h5_file_id);

  return true;
}

bool h5_write_string(char const* filename, char const* varname, std::string const& buffer)
{
  hid_t h5_file_id;

  if (!fileExists(filename)) {
    h5_file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  }
  else {
    h5_file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
  }

  H5LTmake_dataset_string(h5_file_id, varname, buffer.c_str());

  H5Fclose(h5_file_id);

  return true;
}


// reading and writing arrays of strings
bool h5_read_strings(char const* filename, char const* varname, std::vector<std::string>& lines)
{
  if (!fileExists(filename)) {
    std::cerr << ERROR_INFO << "File '" << filename << "' not found." << std::endl;
    //TODO: File not found - Exception? Error code?
    return false;
  }

  hid_t h5_file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t dataset = H5Dopen(h5_file_id, varname, H5P_DEFAULT);

  hid_t datatype = H5Dget_type(dataset);
  bool variable_length = H5Tis_variable_str(datatype);

  hid_t dataspace = H5Dget_space(dataset);

  if (variable_length) {
    int ndims = H5Sget_simple_extent_ndims(dataspace);
    vector<hsize_t> dims(ndims);
    H5Sget_simple_extent_dims(dataspace, &(dims[0]), NULL);
    hsize_t size = accumulate(begin(dims), end(dims), 1, std::multiplies<hsize_t>());

    std::vector<char*> buffer(size * sizeof(char*));
    H5Dread(dataset, datatype, dataspace, dataspace, H5P_DEFAULT, &(buffer[0]));

    for (char const* line : buffer) {
      if (line == nullptr) {
        continue;
      }
      lines.push_back(string(line));
    }

    H5Dvlen_reclaim(datatype, dataspace, H5P_DEFAULT, &(buffer[0]));
  }
  else {
    size_t line_length = H5Tget_size(datatype);
    hssize_t num_lines = H5Sget_simple_extent_npoints(dataspace);

    std::vector<char> buffer(line_length * num_lines, '\0');
    H5LTread_dataset_string(h5_file_id, varname, &(buffer[0]));

    size_t str_start = 0;
    for (hssize_t lines_found = 0; lines_found < num_lines; ++lines_found) {
      lines.push_back(&(buffer[str_start]));
      str_start += line_length;
    }
  }

  H5Sclose(dataspace);
  H5Tclose(datatype);
  H5Dclose(dataset);
  H5Fclose(h5_file_id);

  return true;
}

bool h5_write_strings(char const* filename, char const* varname, std::vector<std::string> const& lines)
{
  // create single C string using the format of the (deprecated) matlab function
  // `hdf5write` for cell arrays of char arrays (aka strings)
  size_t line_length = std::max_element(
    lines.cbegin(), lines.cend(), [] (std::string s1, std::string s2) { return s1.size() < s2.size(); } )->size() + 1;

  std::vector<char> buffer(line_length * lines.size(), '\0');
  size_t str_start = 0;
  for (const string& line : lines) {
    strcpy(&(buffer[str_start]), line.c_str());
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
  hsize_t chunk_dims[1] = {(hsize_t)(hdf_dims[0]/chunk_factor) + 1};

  hid_t plist_id = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(plist_id, 1, chunk_dims);
  H5Pset_deflate(plist_id, 9);

  hid_t dataspace_id = H5Screate_simple(1, hdf_dims, NULL);
  hid_t datatype_id = H5Tcreate(H5T_STRING, line_length);
  hid_t dataset_id = H5Dcreate2(h5_file_id, varname, datatype_id, dataspace_id, H5P_DEFAULT, plist_id, H5P_DEFAULT);

  H5Dwrite(dataset_id, datatype_id, dataspace_id, dataspace_id, H5P_DEFAULT, &(buffer[0]));

  H5Pclose(plist_id);
  H5Dclose(dataset_id);
  H5Tclose(datatype_id);
  H5Sclose(dataspace_id);

  H5Fclose(h5_file_id);

  return true;
}
