idf_component_register(
    SRCS "global.cpp"
    INCLUDE_DIRS "include"
    REQUIRES i_nterfaces
             nvs
    PRIV_REQUIRES esp_netif
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
