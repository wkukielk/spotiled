idf_component_register(
    SRCS "test_util.cpp" "test_str.cpp"
         "test_CharVector.cpp"
         "test_priority_queue.cpp"
         "test_url.cpp"
    REQUIRES util htutils
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)