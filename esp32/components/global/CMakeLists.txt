idf_component_register(
    SRCS "global.cpp"
    INCLUDE_DIRS "include"
    REQUIRES i_nterfaces
             time_aux
             nvs
    PRIV_REQUIRES esp_netif
                  esp_event
                  util
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
