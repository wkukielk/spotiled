idf_component_register(
    SRCS "nvs.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash
             util
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
