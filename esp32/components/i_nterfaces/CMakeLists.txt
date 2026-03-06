idf_component_register(
     SRCS "I_WiFi.cpp"
          "global.cpp"
     INCLUDE_DIRS "include"
     REQUIRES esp_event
              esp_netif
              esp_http_client
              nvs
     PRIV_REQUIRES util
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")

idf_component_get_property(complib i_nterfaces COMPONENT_LIB)
idf_build_get_property(idfpath IDF_PATH)

# i_wifi
target_include_directories(${complib} PUBLIC ${idfpath}/components/esp_wifi/include)
target_include_directories(${complib} PUBLIC ${idfpath}/components/wifi_provisioning/include)

# i_ota
target_include_directories(${complib} PUBLIC ${idfpath}/components/bootloader_support/include)
target_include_directories(${complib} PUBLIC ${idfpath}/components/esp_bootloader_format/include)
target_include_directories(${complib} PUBLIC ${idfpath}/components/esp_https_ota/include)
target_include_directories(${complib} PUBLIC ${idfpath}/components/esp_app_format/include)
target_include_directories(${complib} PUBLIC ${idfpath}/components/esp_partition/include)

# i_led
target_include_directories(${complib} PUBLIC ${idfpath}/components/esp_driver_ledc/include/driver)
target_include_directories(${complib} PUBLIC ${idfpath}/components/esp_driver_gpio/include)

target_compile_options(${COMPONENT_LIB} PRIVATE "SHELL:-Wno-unused-parameter")
