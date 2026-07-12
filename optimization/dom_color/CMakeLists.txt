cmake_minimum_required(VERSION 3.15...4.0)

project(
  ${SKBUILD_PROJECT_NAME}
  VERSION ${SKBUILD_PROJECT_VERSION}
  LANGUAGES CXX)

find_package(pybind11 CONFIG REQUIRED)

python_add_library(_core MODULE src/main.cpp WITH_SOABI)
target_link_libraries(_core PRIVATE pybind11::headers)

install(TARGETS _core DESTINATION dom_color)
