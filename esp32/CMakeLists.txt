cmake_minimum_required(VERSION 3.5)

set(COMPONENTS main)
set(CMAKE_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(EXCLUDE_COMPONENTS wifi_provisioning)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(spotifyled)
