idf_component_register(
    SRCS "OTAClient.cpp"
    INCLUDE_DIRS "include"
    REQUIRES i_nterfaces
             util
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
