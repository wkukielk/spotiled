idf_component_register(
    SRCS "test_priority_queue.cpp"
    REQUIRES util
    WHOLE_ARCHIVE
)
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)