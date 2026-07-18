idf_component_register(
    SRCS "runtime_data.cpp"
         "master/runtime_helpers.cpp"
         "server/command.cpp"
    INCLUDE_DIRS "include"
    REQUIRES wifi_manager
             util
             ota_client
             ws_server
             udp_server
             colors
             ledaux
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
