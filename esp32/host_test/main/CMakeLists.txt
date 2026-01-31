file(GLOB ht_dirs [RELATIVE <path>] LIST_DIRECTORIES true ../../components/*/*_ht)
message(STATUS "HOST_TEST_DIRS=${ht_dirs}")

set(component_names "")
foreach(path IN LISTS ht_dirs)
    cmake_path(GET path STEM component_name)
    list(APPEND component_names ${component_name})
endforeach()
message(STATUS "COMPONENT_NAMES=${component_names}")

idf_component_register(
    SRCS "test.cpp"
    INCLUDE_DIRS "."
    REQUIRES cmock json ${component_names}
    WHOLE_ARCHIVE
)

target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)