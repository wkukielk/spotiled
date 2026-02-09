idf_component_register(
    SRCS
        "test_HTTPHandler.cpp"
        "HTTPHandlerTestAux.cpp"
        "test_HTTPRepeatingHandlerTest.cpp"
    INCLUDE_DIRS "include"
    REQUIRES http_client htutils
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)
