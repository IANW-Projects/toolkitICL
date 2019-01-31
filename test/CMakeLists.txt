
if (MSVC)
MESSAGE(STATUS "MSVC++ detected")
set (CMAKE_CXX_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB" )
set (CMAKE_C_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB" )
endif(MSVC)

# include header directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${HDF5_INCLUDE_DIRS} ../include)

# specifiy library paths for linker
link_directories (${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES})

# set output directory for executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)


# copy tests
# TODO: short, long
set(COPY_TESTS copy_float copy_double copy_char copy_uchar copy_int copy_uint)

foreach(TEST ${COPY_TESTS})
  add_executable(${TEST} ${TEST}.cpp copy_test.h ../include/hdf5_io.hpp ../src/hdf5_io.cpp)
endforeach()


# all tests
set(TESTS ${COPY_TESTS})

foreach(TEST ${TESTS})
  target_link_libraries(${TEST} ${OpenCL_LIBRARIES} ${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES})
  add_test(${TEST} ${TEST})
endforeach()
