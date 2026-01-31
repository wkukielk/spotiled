idf_component_register(
    SRCS "init.cpp"
         "global.cpp"
    INCLUDE_DIRS "include"
    REQUIRES i_wifi
             time_aux
             nvs
    PRIV_REQUIRES esp_netif
                  esp_event
                  util
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
