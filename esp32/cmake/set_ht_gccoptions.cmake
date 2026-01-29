include("${CMAKE_CURRENT_LIST_DIR}/gccoptions.cmake")

target_compile_options(${COMPONENT_LIB} PRIVATE ${HOST_TEST_GCC_OPTIONS})
