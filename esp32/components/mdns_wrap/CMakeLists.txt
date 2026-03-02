idf_component_register(SRCS "mDNS.cpp"
                       REQUIRES i_nterfaces util
                       INCLUDE_DIRS "include")

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")