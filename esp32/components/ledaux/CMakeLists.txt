idf_component_register(
    SRCS "ledaux.cpp"
    INCLUDE_DIRS "include"
    REQUIRES colors
             esp_driver_ledc
             esp_driver_gpio
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")