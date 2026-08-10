cmake_minimum_required(VERSION 3.16)

set(CMAKE_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/../cmake")

set(COMPONENTS main)
set(EXTRA_COMPONENT_DIRS "../components" "components")

file(GLOB ht_dirs [RELATIVE <path>] LIST_DIRECTORIES true ../components/*/*_ht)
message(STATUS "HOST_TEST_DIRS=${ht_dirs}")
list(APPEND EXTRA_COMPONENT_DIRS ${ht_dirs})
message(STATUS "EXTRA_COMPONENT_DIRS=${EXTRA_COMPONENT_DIRS}")
message(STATUS "TARGET=${IDF_TARGET}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(host_test)

idf_build_set_property(COMPILE_OPTIONS "-g" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-O0" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-uninitialized-const-pointer" APPEND)

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)
