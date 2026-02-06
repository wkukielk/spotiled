cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "../components")
set(CMAKE_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/../cmake")

set(TEST_COMPONENTS "nvs")

project(unit_test_test)
