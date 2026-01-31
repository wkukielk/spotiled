idf_component_register(
    SRCS "test_NVS.cpp"
    INCLUDE_DIRS "include"
    REQUIRES nvs
    WHOLE_ARCHIVE
)
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)