idf_component_register(
    SRCS "mDNS.cpp"
         "OTA.cpp"
         "WiFi.cpp"
         "WiFiProv.cpp"
         "HTTPD.cpp"
         "UDPSocket.cpp"
         "NVSSecurity.cpp"
         "GPIO.cpp"
         "Timer.cpp"
    INCLUDE_DIRS "include"
    REQUIRES i_nterfaces
             util
             esp_wifi
             esp_timer
    	     espressif__network_provisioning
    PRIV_REQUIRES esp_https_ota
                  app_update
                  esp_http_server
                  esp_driver_ledc
                  lwip
)

target_compile_options(${COMPONENT_LIB} PRIVATE "SHELL:-Wno-unused-parameter")
