idf_component_register(
    SRCS "WSServer.cpp"
    INCLUDE_DIRS "include"
    REQUIRES util
             i_nterfaces
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
