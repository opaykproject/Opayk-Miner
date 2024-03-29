# A custom command and target to turn the OpenCL kernel into a byte array header
# The normal build depends on it properly and if the kernel file is changed, then
# a rebuild of libethash-cl should be triggered

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel.h
	COMMAND ${CMAKE_COMMAND} ARGS
	-DTXT2STR_SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel.cl"
	-DTXT2STR_VARIABLE_NAME=CLMiner_kernel
	-DTXT2STR_HEADER_FILE="${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel.h"
	-P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/txt2str.cmake"
	COMMENT "Generating OpenCL Kernel"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel.cl
)
add_custom_target(cl_kernel DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel.h ${CMAKE_CURRENT_SOURCE_DIR}/CLMiner_kernel.cl)

set(SOURCES
	CLMiner.h CLMiner.cpp
	${CMAKE_CURRENT_BINARY_DIR}/CLMiner_kernel.h
)

if(APPLE)
	# On macOS use system OpenCL library.
	find_package(OpenCL REQUIRED)
else()
  find_package(OpenCL MODULE REQUIRED)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(..)

add_library(ethash-cl STATIC ${SOURCES})
target_link_libraries(ethash-cl PUBLIC ethcore ethash::ethash progpow)
target_link_libraries(ethash-cl PRIVATE OpenCL::OpenCL)
target_link_libraries(ethash-cl PRIVATE Boost::filesystem Boost::thread)
