
#TODO: Do we need this here? It is already in the main CMakeLists.txt file.
if (MSVC)
  MESSAGE(STATUS "MSVC++ detected")
  set (CMAKE_CXX_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB" )
  set (CMAKE_C_FLAGS " /std=c++11 /O2 /DH5_BUILT_AS_DYNAMIC_LIB" )
endif(MSVC)

# include header directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OpenCL_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} ../include)

# header files of the project
set(HEADER ../include/opencl_include.hpp ../include/ocl_dev_mgr.hpp ../include/timer.hpp ../include/util.hpp)

set(SOURCES main.cpp ocl_dev_mgr.cpp ${HEADER})

# add hdf5_io as object library in order to reuse it for the tests
add_library(hdf5_io OBJECT ../include/hdf5_io.hpp hdf5_io.cpp)

# Specifiy library paths for linker
link_directories (${OpenCL_LIBRARIES} ${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES})

#set output directory for executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#gernerate executable
add_executable(toolkitICL ${SOURCES} $<TARGET_OBJECTS:hdf5_io>)

#Link libraries
TARGET_LINK_LIBRARIES(toolkitICL ${OpenCL_LIBRARIES} ${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES})
