idf_component_register(
    SRCS "main.cpp"
         "app.cpp" 
    PRIV_INCLUDE_DIRS "include"
    REQUIRES global
             runtime
             config
    PRIV_REQUIRES time_aux
                  nvs
                  wifi_wrap
                  mdns_wrap
                  ota_client
             
)

if(DEV)
     nvs_create_partition_image(cfg "${PROJECT_DIR}/nvs/devconfig.csv" FLASH_IN_PROJECT)
else()
     nvs_create_partition_image(cfg "${PROJECT_DIR}/nvs/config.csv" FLASH_IN_PROJECT)
endif()

include("${PROJECT_DIR}/cmake/set_gccoptions.cmake")
