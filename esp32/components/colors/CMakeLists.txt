idf_component_register(
    SRCS "hsv.cpp"
         "rgb.cpp"
    INCLUDE_DIRS "."
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
