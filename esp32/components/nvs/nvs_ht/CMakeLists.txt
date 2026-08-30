idf_component_register(
    SRCS "test_NVS.cpp"
    REQUIRES nvs
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)