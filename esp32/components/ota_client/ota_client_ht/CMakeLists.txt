idf_component_register(
    SRCS "test_OTAClient.cpp"
    INCLUDE_DIRS "include"
    REQUIRES ota_client
             i_ota_ht
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)