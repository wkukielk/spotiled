idf_component_register(SRCS "I_OTA.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_partition
                             esp_app_format
                             esp_http_client
)

idf_component_get_property(complib i_ota COMPONENT_LIB)
idf_build_get_property(idfpath IDF_PATH)
target_include_directories(${complib} PUBLIC ${idfpath}/components/bootloader_support/include)
target_include_directories(${complib} PUBLIC ${idfpath}/components/esp_https_ota/include)
