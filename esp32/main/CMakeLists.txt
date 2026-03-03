idf_component_register(
    SRCS "main.cpp"
         "app.cpp" 
    PRIV_INCLUDE_DIRS "include"
    REQUIRES runtime
             config
    PRIV_REQUIRES nvs
)

if(CONFIG_DEV)
     nvs_create_partition_image(cfg "${PROJECT_DIR}/nvs/devconfig.csv" FLASH_IN_PROJECT)
else()
     nvs_create_partition_image(cfg "${PROJECT_DIR}/nvs/config.csv" FLASH_IN_PROJECT)
endif()

nvs_create_partition_image(encnvs "${PROJECT_DIR}/nvs/enc.csv" FLASH_IN_PROJECT)

include("${PROJECT_DIR}/cmake/set_gccoptions.cmake")
