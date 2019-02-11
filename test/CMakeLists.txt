
# include header directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${HDF5_INCLUDE_DIRS} ../include)

# specifiy library paths for linker
link_directories (${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES})

# set output directory for executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)


# copy tests
set(COPY_TESTS copy_float copy_double copy_char copy_uchar copy_short copy_ushort copy_int copy_uint copy_long copy_ulong)

foreach(TEST ${COPY_TESTS})
  add_executable(${TEST} ${TEST}.cpp copy_test.h ../include/opencl_include.hpp ../include/util.hpp ../include/hdf5_io.hpp ../src/hdf5_io.cpp)
endforeach()


# timer test
set(TIMER_TEST timer)
foreach(TEST ${TIMER_TEST})
  add_executable(${TEST} ${TEST}.cpp ../include/timer.hpp)
endforeach()


# kernel repetition test
set(KERNEL_REPETITION_TEST repetition_test)
foreach(TEST ${KERNEL_REPETITION_TEST})
  add_executable(${TEST} ${TEST}.cpp ../include/opencl_include.hpp ../include/util.hpp ../include/hdf5_io.hpp ../src/hdf5_io.cpp)
endforeach()


# output test
set(OUTPUT_TEST output_test)
foreach(TEST ${OUTPUT_TEST})
  add_executable(${TEST} ${TEST}.cpp ../include/opencl_include.hpp ../include/util.hpp ../include/hdf5_io.hpp ../src/hdf5_io.cpp)
endforeach()


# all tests
set(TESTS ${COPY_TESTS} ${TIMER_TEST} ${KERNEL_REPETITION_TEST} ${OUTPUT_TEST})

foreach(TEST ${TESTS})
  target_link_libraries(${TEST} ${OpenCL_LIBRARIES} ${HDF5_HL_LIBRARIES} ${HDF5_LIBRARIES})
  add_test(${TEST} ${TEST})
endforeach()


# julia tests
find_program(JULIA julia)
IF(EXISTS ${JULIA})
  MESSAGE(STATUS "Julia found at" ${JULIA} "\n   Corresponding tests were added.")
  configure_file(copy_test.jl copy_test.jl COPYONLY)
  add_test(${JULIA} julia copy_test.jl)
ELSE()
  MESSAGE(STATUS "Julia not found; no corresponding tests were added.")
ENDIF()
