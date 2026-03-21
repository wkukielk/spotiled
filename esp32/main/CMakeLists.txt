idf_component_register(
    SRCS "main.cpp"
         "app.cpp" 
    PRIV_INCLUDE_DIRS "include"
    REQUIRES runtime
             config
             ledaux
    PRIV_REQUIRES nvs
)

if(CONFIG_DEV)
     nvs_create_partition_image(cfg "${PROJECT_DIR}/firmware/nvs/devconfig.csv" FLASH_IN_PROJECT)
elseif(CONFIG_RELEASE)
     nvs_create_partition_image(cfg "${PROJECT_DIR}/firmware/nvs/config.csv" FLASH_IN_PROJECT)
endif()

if(NOT CONFIG_CI)
     nvs_create_partition_image(encnvs "${PROJECT_DIR}/firmware/nvs/enc.csv" FLASH_IN_PROJECT)
endif()

include("${PROJECT_DIR}/cmake/set_gccoptions.cmake")
