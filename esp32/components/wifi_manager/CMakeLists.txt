idf_component_register(
     SRCS "CredentialsWiFiManager.cpp"
          "Provisioning.cpp"
          "ProvEventHandler.cpp"
          "WiFiEventHandler.cpp"
          "wifi_aux.cpp"
     INCLUDE_DIRS "include"
     REQUIRES  i_nterfaces
               esp_netif
               util
     PRIV_REQUIRES  esp_event
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
target_compile_options(${COMPONENT_LIB} PRIVATE "SHELL:-Wno-unused-parameter")
