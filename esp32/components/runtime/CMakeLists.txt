idf_component_register(
    SRCS "runtime.cpp"
    INCLUDE_DIRS "include"
    REQUIRES wifi_manager
             util
             ota_client
             global
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
