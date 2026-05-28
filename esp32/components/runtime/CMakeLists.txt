idf_component_register(
    SRCS "server/states.cpp"
         "runtime_data.cpp"
         "runtime_helpers.cpp"
    INCLUDE_DIRS "include"
    REQUIRES wifi_manager
             util
             ota_client
             ws_server
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
