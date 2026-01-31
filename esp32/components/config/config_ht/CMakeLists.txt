idf_component_register(
    SRCS "test_config.cpp"
    REQUIRES config nvs nvs_ht
    WHOLE_ARCHIVE
)
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)
