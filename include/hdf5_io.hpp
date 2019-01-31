/* TODO: Provide a license note */

#ifndef HDF5_IO_H
#define HDF5_IO_H

#include "hdf5.h"
#include "hdf5_hl.h"
#include "../include/main.hpp"
//TODO: remove dependency on main.hpp since no OpenCL parts are really needed?

enum HD5_Type { H5_float, H5_double, H5_char, H5_uchar, H5_int, H5_uint, H5_long, H5_ulong };


// utility functions for hdf5 dfiles
bool h5_check_object(const char* filename, const char* varname);

uint8_t h5_get_content(const char* filename, const char* hdf_dir,
                       std::vector<std::string> &data_list, std::vector<HD5_Type> &datatype_list, std::vector<size_t> &data_size);

uint8_t h5_create_dir(const char* filename, const char* hdf_dir);


// convert a C type TYPE to the HDF5 identifier of that type
template<typename TYPE>
auto type_to_h5_type(void);


// read a buffer from an HDF5 File
template<typename TYPE>
bool h5_read_buffer(const char* filename, const char* varname, TYPE* data);

bool h5_read_buffer_float(const char* filename, const char* varname, float* data);
bool h5_read_buffer_double(const char* filename, const char* varname, double* data);
bool h5_read_buffer_int(const char* filename, const char* varname, int32_t* data);
bool h5_read_buffer_uint(const char* filename, const char* varname, uint32_t* data);
bool h5_read_buffer_uchar(const char* filename, const char* varname, unsigned char* data);
bool h5_read_buffer_char(const char* filename, const char* varname, cl_char* data);

// write a buffer to an HDF5 File
uint8_t h5_write_buffer_float(const char* filename, const char* varname, float* data, cl_ulong size);
uint8_t h5_write_buffer_double(const char* filename, const char* varname, double* data, cl_ulong size);
uint8_t h5_write_buffer_uint(const char* filename, const char* varname, cl_uint* data, cl_ulong size);
uint8_t h5_write_buffer_int(const char* filename, const char* varname, cl_int* data, cl_ulong size);
uint8_t h5_write_buffer_uchar(const char* filename, const char* varname, cl_uchar* data, cl_ulong size);
uint8_t h5_write_buffer_char(const char* filename, const char* varname, cl_char* data, cl_ulong size);

uint8_t h5_write_buffer_float4(const char* filename, const char* varname, cl_float4* data, cl_ulong size);
uint8_t h5_write_buffer_double4(const char* filename, const char* varname, cl_double4* data, cl_ulong size);
uint8_t h5_write_buffer_uint4(const char* filename, const char* varname, cl_uint4* data, cl_ulong size);


// read a single item from an HDF5 File
float h5_read_single_float(const char* filename, const char* varname);

// write a single item to an HDF5 File
uint8_t h5_write_single_float(const char* filename, const char* varname, float data);
uint8_t h5_write_single_double(const char* filename, const char* varname, double data);
uint8_t h5_write_single_long(const char* filename, const char* varname, long data);


// reading and writing single strings
uint8_t h5_read_string(const char* filename, const char* varname, char* buffer);
uint8_t h5_write_string(const char* filename, const char* varname, const char* buffer);


// reading and writing arrays of strings using the format of the (deprecated)
// matlab function hdfwrite for cell arrays of strings (aka char arrays)
uint8_t h5_read_strings(const char* filename, const char* varname, std::vector<std::string>& lines);
uint8_t h5_write_strings(const char* filename, const char* varname, std::vector<std::string> const& lines);


#endif // HDF5_IO_H
