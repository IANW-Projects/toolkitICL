
# include header directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OpenCL_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} ../include)

set(HEADER ../include/opencl_include.hpp ../include/ocl_dev_mgr.hpp ../include/timer.hpp ../include/util.hpp)

IF(USEIRAPL)
  list(APPEND HEADER "../include/rapl.hpp")
ENDIF(USEIRAPL)

IF(USEIPG)
  list(APPEND HEADER "../include/rapl.hpp")
ENDIF(USEIPG)

IF(USEAMDP)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OpenCL_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} "$ENV{AMDPROFILERPATH}/include" "../include")
ENDIF(USEAMDP)

IF(USEIRAPL)
  set(SOURCES main.cpp ocl_dev_mgr.cpp rapl.cpp ${HEADER})
ELSE(USEIRAPL)
  IF(USEIPG)
    set(SOURCES main.cpp ocl_dev_mgr.cpp rapl.cpp ${HEADER})
  ELSE(USEIPG)
    set(SOURCES main.cpp ocl_dev_mgr.cpp ${HEADER})
  ENDIF(USEIPG)
ENDIF(USEIRAPL)

# add hdf5_io as object library in order to reuse it for the tests
add_library(hdf5_io OBJECT ../include/hdf5_io.hpp hdf5_io.cpp)

set(LDPATH "${OpenCL_LIBRARIES} ${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES}")

# specifiy library paths for linker
IF(USENVML)
  IF(MSVC)
    list(APPEND LDPATH "${CUDA_TOOLKIT_ROOT_DIR}/lib/x64")
  ELSE(MSVC)
    list(APPEND LDPATH "${CUDA_TOOLKIT_ROOT_DIR}/lib/x86_64-linux-gnu/stubs/")
  ENDIF(MSVC)
ENDIF(USENVML)

IF(USEAMDP)
  IF(MSVC)
    list(APPEND LDPATH "$ENV{AMDPROFILERPATH}")
  ELSE(MSVC)
    list(APPEND LDPATH "$ENV{AMDPROFILERPATH}/bin")
  ENDIF(MSVC)
ENDIF(USEAMDP)

link_directories(${LDPATH})


# set output directory for executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# generate executable
add_executable(toolkitICL ${SOURCES} $<TARGET_OBJECTS:hdf5_io>)

set(LIBRARIES "${OpenCL_LIBRARIES};${HDF5_HL_LIBRARIES};${HDF5_LIBRARIES}")

#set(MSVC_LINK_FLAGS "/DELAYLOAD:")

# link libraries
IF(USENVML)
  if(MSVC)
    list(APPEND LIBRARIES "nvml.lib")
    set(MSVC_LINK_FLAGS "${MSVC_LINK_FLAGS}/DELAYLOAD:nvml.dll ")
  ELSE()
    list(APPEND LIBRARIES "libnvidia-ml.so")
  ENDIF()
ENDIF(USENVML)

IF(USEAMDP)
  if(MSVC)
    list(APPEND LIBRARIES "AMDPowerProfileAPI.lib")
    set(MSVC_LINK_FLAGS "${MSVC_LINK_FLAGS}/DELAYLOAD:AMDPowerProfileAPI.dll ")
  ELSE()
    list(APPEND LIBRARIES "libAMDPowerProfileAPI.so")
  ENDIF()
ENDIF(USEAMDP)

MESSAGE(STATUS "Used libs: ${LIBRARIES}")

if(MSVC)
MESSAGE(STATUS "Used linker flags: ${MSVC_LINK_FLAGS}")
set_target_properties(toolkitICL PROPERTIES LINK_FLAGS "${MSVC_LINK_FLAGS}")
ENDIF()


TARGET_LINK_LIBRARIES(toolkitICL ${LIBRARIES})


