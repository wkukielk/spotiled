include("${CMAKE_CURRENT_LIST_DIR}/gccoptions.cmake")

target_compile_options(${COMPONENT_LIB} PRIVATE ${COMPONENT_GCC_OPTIONS})
