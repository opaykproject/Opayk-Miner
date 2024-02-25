include(cmake/cable/bootstrap.cmake)

include(CableBuildInfo)
include(CableBuildType)
include(CableToolchains)

cable_configure_toolchain(DEFAULT cxx11)
set(CMAKE_CXX_STANDARD 11)

project(progminer)
set(PROJECT_VERSION 1.1.2)

cable_set_build_type(DEFAULT Release CONFIGURATION_TYPES Release RelWithDebInfo)

option(ETHASHCL "Build with OpenCL mining" ON)
option(ETHASHCUDA "Build with CUDA mining" ON)
if (ETHASHCL)
  add_definitions(-DETH_ETHASHCL)
endif()
if (ETHASHCUDA)
  add_definitions(-DETH_ETHASHCUDA)
endif()

find_package(Boost CONFIG REQUIRED COMPONENTS system filesystem thread)

include_directories(BEFORE .)

set(ETHASH_BUILD_TESTS NO)
add_subdirectory("ethash")

include(EthCompilerSettings)
if(UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif()

cable_add_buildinfo_library(PROJECT_NAME ${PROJECT_NAME})

add_subdirectory(libprogpow)
add_subdirectory(libdevcore)
add_subdirectory(libethcore)
add_subdirectory(libhwmon)

if (ETHASHCL)
	add_subdirectory(libethash-cl)
endif ()
if (ETHASHCUDA)
	add_subdirectory(libethash-cuda)
endif ()

if(WIN32)
	set(CPACK_GENERATOR ZIP)
else()
	set(CPACK_GENERATOR TGZ)
endif()
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CHECKSUM SHA256)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
include(CPack)

add_library(progminer STATIC progminer.cpp)
target_link_libraries(progminer PRIVATE ethcore devcore Boost::system)
