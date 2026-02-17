idf_component_register(
    SRCS "runtime.cpp"
    INCLUDE_DIRS "include"
    REQUIRES wifi_manager
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
