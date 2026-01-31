idf_component_register(
    SRCS "test_config.cpp"
    REQUIRES config nvs nvs_ht
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)
