cmake_minimum_required(VERSION 3.15...4.0)

if(SKBUILD)
  project(
    ${SKBUILD_PROJECT_NAME}
    VERSION ${SKBUILD_PROJECT_VERSION}
    LANGUAGES CXX)
else()
  project(dom_color_example LANGUAGES CXX)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../dom_color dom_color_lib)

if(SKBUILD)
  find_package(pybind11 CONFIG REQUIRED)

  python_add_library(_core MODULE src/main.cpp WITH_SOABI)
  target_link_libraries(_core PRIVATE pybind11::headers dom_color)

  install(TARGETS _core DESTINATION dom_color)
else()
  # Plain CMake build (not pip install): standalone playground executable.
  add_executable(example src/example.cpp)
  target_link_libraries(example PRIVATE dom_color)
endif()
