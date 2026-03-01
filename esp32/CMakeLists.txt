cmake_minimum_required(VERSION 3.5)

set(COMPONENTS main)
set(CMAKE_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(spotifyled)
