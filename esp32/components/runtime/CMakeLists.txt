idf_component_register(
    SRCS "states.cpp" "runtime.cpp"
    INCLUDE_DIRS "include"
    REQUIRES wifi_manager
             util
             ota_client
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
