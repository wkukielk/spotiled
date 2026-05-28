idf_component_register(
    SRCS "test_WSServer.cpp"
         "MockHTTPD.cpp"
    INCLUDE_DIRS "include"
    REQUIRES ws_server
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)
