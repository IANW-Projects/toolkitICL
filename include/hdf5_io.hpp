/* TODO: Provide a license note */

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

bool h5_read_buffer_float(char const* filename, char const* varname, float* data);
bool h5_read_buffer_double(char const* filename, char const* varname, double* data);
bool h5_read_buffer_int(char const* filename, char const* varname, cl_int* data);
bool h5_read_buffer_uint(char const* filename, char const* varname, cl_uint* data);
bool h5_read_buffer_char(char const* filename, char const* varname, cl_char* data);
bool h5_read_buffer_uchar(char const* filename, char const* varname, cl_uchar* data);

// write a buffer to an HDF5 file using compression
template<typename TYPE>
bool h5_write_buffer(char const* filename, char const* varname, TYPE const* data, size_t size);

template<typename TYPE>
inline bool h5_write_buffer(std::string const& filename, char const* varname, TYPE const* data, size_t size)
{
  return h5_write_buffer<TYPE>(filename.c_str(), varname, data, size);
}

bool h5_write_buffer_float(char const* filename, char const* varname, float const* data, size_t size);
bool h5_write_buffer_double(char const* filename, char const* varname, double const* data, size_t size);
bool h5_write_buffer_int(char const* filename, char const* varname, cl_int const* data, size_t size);
bool h5_write_buffer_uint(char const* filename, char const* varname, cl_uint const* data, size_t size);
bool h5_write_buffer_char(char const* filename, char const* varname, cl_char const* data, size_t size);
bool h5_write_buffer_uchar(char const* filename, char const* varname, cl_uchar const* data, size_t size);

bool h5_write_buffer_float4(char const* filename, char const* varname, cl_float4 const* data, size_t size);
bool h5_write_buffer_double4(char const* filename, char const* varname, cl_double4 const* data, size_t size);
bool h5_write_buffer_uint4(char const* filename, char const* varname, cl_uint4 const* data, size_t size);


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

float h5_read_single_float(char const* filename, char const* varname);

// write a single item to an HDF5 file
template<typename TYPE>
bool h5_write_single(char const* filename, char const* varname, TYPE data)
{
  return h5_write_buffer<TYPE>(filename, varname, &data, 1);
}

template<typename TYPE>
inline bool h5_write_single(std::string const& filename, char const* varname, TYPE data)
{
  return h5_write_single<TYPE>(filename.c_str(), varname, data);
}

bool h5_write_single_float(char const* filename, char const* varname, float data);
bool h5_write_single_double(char const* filename, char const* varname, double data);
bool h5_write_single_long(char const* filename, char const* varname, cl_long data);


// reading and writing single strings
bool h5_read_string(char const* filename, char const* varname, char* buffer);
bool h5_write_string(char const* filename, char const* varname, char const* buffer);

inline bool h5_read_string(std::string const& filename, char const* varname, char* buffer)
{
  return h5_read_string(filename.c_str(), varname, buffer);
}
inline bool h5_write_string(std::string const& filename, char const* varname, char const* buffer)
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
