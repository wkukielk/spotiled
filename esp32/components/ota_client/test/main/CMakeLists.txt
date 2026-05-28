idf_component_register(
    SRCS "test_ota_client.cpp"
         "OTAStub.cpp"
    INCLUDE_DIRS "."
    REQUIRES unity
             ota_client
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
