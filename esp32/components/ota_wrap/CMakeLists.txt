idf_component_register(
    SRCS "OTA.cpp"
    INCLUDE_DIRS "include"
    REQUIRES i_nterfaces
             url
    PRIV_REQUIRES esp_https_ota
                  app_update
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")