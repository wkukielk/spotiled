# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/components/driver/test_apps/components")
list(APPEND EXTRA_COMPONENT_DIRS "../components")

set(CMAKE_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/../cmake")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(unit-test-app)

idf_build_set_property(COMPILE_OPTIONS "-Wno-error=format" APPEND)
