
# Include header directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}	${OPENGL_INCLUDE_DIRS} ${OpenCL_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} ../include)

#Header files of the project

set(HEADER ../include/ocl_util.hpp ../include/util.hpp ../include/ocl_dev_mgr.hpp)

set(SOURCES main.cpp hdf5_io.cpp ocl_dev_mgr.cpp ${HEADER})


# Specifiy library paths for linker
link_directories (${OpenCL_LIBRARIES} ${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES}) 

#set output directory for executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#gernerate executable
add_executable(toolkitICL ${SOURCES})


#Link libraries
TARGET_LINK_LIBRARIES(toolkitICL ${OpenCL_LIBRARIES} ${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES})

IF(OPENGL)
MESSAGE("Adding defines...")
ENDIF()
#target_compile_definitions(test1CL PUBLIC USE_OPENGL)

configure_file(filter.cl ${CMAKE_BINARY_DIR}/bin/filter.cl COPYONLY)
configure_file(../misc/make.sh ${CMAKE_BINARY_DIR}/bin/make.sh COPYONLY)
