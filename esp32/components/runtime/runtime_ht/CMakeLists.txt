idf_component_register(
    SRCS "test_runtime.cpp"
         "test_server.cpp"
    INCLUDE_DIRS "include"
    REQUIRES runtime
             i_nterfaces_ht
             wifi_manager_ht
             htutils
             ledaux
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)