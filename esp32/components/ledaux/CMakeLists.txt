idf_component_register(
    SRCS "ledaux.cpp"
    INCLUDE_DIRS "include"
    REQUIRES colors
             i_nterfaces
             util
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")