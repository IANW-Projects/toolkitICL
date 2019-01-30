
if (MSVC)
MESSAGE(STATUS "MSVC++ detected")
set (CMAKE_CXX_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB" )
set (CMAKE_C_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB" )
endif(MSVC)

# include header directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${HDF5_INCLUDE_DIRS} ../include)

set(HEADER ../include/hdf5_io.hpp)

set(SOURCES copy_float.cpp ../src/hdf5_io.cpp ${HEADER})


# specifiy library paths for linker
link_directories (${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES})

# set output directory for executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)




add_executable(copy_float ${SOURCES})
# TODO: ${OpenCL_LIBRARIES} are only needed since they are included via main.hpp in hdf5_io.hpp
TARGET_LINK_LIBRARIES(copy_float ${OpenCL_LIBRARIES} ${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES})
add_test(copy_float copy_float)
