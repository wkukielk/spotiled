idf_component_register(
    SRCS "master/states.cpp"
         "master/runtime.cpp"
         "runtime_data.cpp"
    INCLUDE_DIRS "include"
    REQUIRES wifi_manager
             util
             ota_client
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
