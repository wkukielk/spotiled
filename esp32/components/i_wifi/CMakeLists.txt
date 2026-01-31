idf_component_register(
     SRCS "I_WiFi.cpp"
     INCLUDE_DIRS "."
     REQUIRES esp_event
     PRIV_REQUIRES util
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")

idf_component_get_property(complib i_wifi COMPONENT_LIB)
idf_build_get_property(idfpath IDF_PATH)
target_include_directories(${complib} PUBLIC ${idfpath}/components/esp_wifi/include)
target_include_directories(${complib} PUBLIC ${idfpath}/components/wifi_provisioning/include)