idf_component_register(
    SRCS "mDNS.cpp"
         "OTA.cpp"
         "WiFi.cpp"
         "WiFiProv.cpp"
    INCLUDE_DIRS "include"
    REQUIRES i_nterfaces
             util
             wifi_provisioning
             esp_wifi
    PRIV_REQUIRES esp_https_ota
                  app_update
)

target_compile_options(${COMPONENT_LIB} PRIVATE "SHELL:-Wno-unused-parameter")
