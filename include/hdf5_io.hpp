/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#ifndef HDF5_IO_H
#define HDF5_IO_H

#include "hdf5.h"
#include "hdf5_hl.h"

#include "opencl_include.hpp"


enum HD5_Type { H5_float, H5_double, H5_char, H5_uchar, H5_short, H5_ushort, H5_int, H5_uint, H5_long, H5_ulong };


bool h5_check_object(char const* filename, char const* varname);

bool h5_get_content(char const* filename, char const* hdf_dir,
  std::vector<std::string>& data_names, std::vector<HD5_Type>& data_types, std::vector<size_t>& data_sizes);

bool h5_create_dir(char const* filename, char const* hdf_dir);
inline bool h5_create_dir(std::string const& filename, char const* hdf_dir)
{
  return h5_create_dir(filename.c_str(), hdf_dir);
}


// convert a C type TYPE to the HDF5 identifier of that type
template<typename TYPE>
hid_t type_to_h5_type(void);

// get the size of an OpenCL vector type
template<typename TYPE>
constexpr size_t get_vector_size();


// read a buffer from an HDF5 file
template<typename TYPE>
bool h5_read_buffer(char const* filename, char const* varname, TYPE* data);

template<typename TYPE>
inline bool h5_read_buffer(std::string const& filename, char const* varname, TYPE* data)
{
  return h5_read_buffer<TYPE>(filename.c_str(), varname, data);
}

// write a buffer to an HDF5 file using compression
template<typename TYPE>
bool h5_write_buffer(char const* filename, char const* varname, TYPE const* data, size_t size, std::string const& description="");

template<typename TYPE>
inline bool h5_write_buffer(std::string const& filename, char const* varname, TYPE const* data, size_t size, std::string const& description="")
{
  return h5_write_buffer<TYPE>(filename.c_str(), varname, data, size, description);
}


// read a single item from an HDF5 file
template<typename TYPE>
TYPE h5_read_single(char const* filename, char const* varname)
{
  TYPE data;
  h5_read_buffer<TYPE>(filename, varname, &data);
  return data;
}

template<typename TYPE>
inline TYPE h5_read_single(std::string const& filename, char const* varname)
{
  return h5_read_single<TYPE>(filename.c_str(), varname);
}

// write a single item to an HDF5 file
template<typename TYPE>
bool h5_write_single(char const* filename, char const* varname, TYPE data, std::string const& description="");

template<typename TYPE>
inline bool h5_write_single(std::string const& filename, char const* varname, TYPE data, std::string const& description="")
{
  return h5_write_single<TYPE>(filename.c_str(), varname, data, description);
}


// reading and writing single strings
bool h5_read_string(char const* filename, char const* varname, std::string& output);
bool h5_write_string(char const* filename, char const* varname, std::string const& output);

inline bool h5_read_string(std::string const& filename, char const* varname, std::string& buffer)
{
  return h5_read_string(filename.c_str(), varname, buffer);
}
inline bool h5_write_string(std::string const& filename, char const* varname, std::string const& buffer)
{
  return h5_write_string(filename.c_str(), varname, buffer);
}

// reading and writing arrays of strings using the format of the (deprecated)
// matlab function hdfwrite for cell arrays of strings (aka char arrays)
bool h5_read_strings(char const* filename, char const* varname, std::vector<std::string>& lines);
bool h5_write_strings(char const* filename, char const* varname, std::vector<std::string> const& lines);

inline bool h5_read_strings(std::string const& filename, char const* varname, std::vector<std::string>& lines)
{
  return h5_read_strings(filename.c_str(), varname, lines);
}
inline bool h5_write_strings(std::string const& filename, char const* varname, std::vector<std::string> const& lines)
{
  return h5_write_strings(filename.c_str(), varname, lines);
}


#endif // HDF5_IO_H
