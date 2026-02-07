idf_component_register(
     SRCS "CredentialsWiFiManager.cpp" "Provisioning.cpp"
          "ProvEventHandler.cpp"
          "WiFiEventHandler.cpp"
          "WiFiManager.cpp"
          "wifi_aux.cpp"
     INCLUDE_DIRS "include"
     REQUIRES  i_wifi
               esp_netif
               util
               time_aux
     PRIV_REQUIRES  esp_event
                    straux
                    global
                    wifi_wrap
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
target_compile_options(${COMPONENT_LIB} PRIVATE "SHELL:-Wno-unused-parameter")
