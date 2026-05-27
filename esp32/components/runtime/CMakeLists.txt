idf_component_register(
    SRCS "master/runtime.cpp"
         "server/server.cpp"
         "runtime_data.cpp"
    INCLUDE_DIRS "include"
    REQUIRES wifi_manager
             util
             ota_client
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
