cmake_minimum_required(VERSION 3.16)

set(COMPONENTS main)
set(EXTRA_COMPONENT_DIRS "../../")
set(CMAKE_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../cmake")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(spotiled_ota_test)
