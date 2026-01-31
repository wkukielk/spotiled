idf_component_register(
    SRCS "test_str.cpp"
    REQUIRES straux
    WHOLE_ARCHIVE
)
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)