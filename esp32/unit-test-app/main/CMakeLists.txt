idf_component_register(
    SRCS "app_main.cpp"
    INCLUDE_DIRS ""
    PRIV_REQUIRES time_aux
                  nvs
                  wifi_wrap
                  mdns_wrap
                  global
                  config
                  test_utils
)

nvs_create_partition_image(cfg "${PROJECT_DIR}/../nvs/devconfig.csv" FLASH_IN_PROJECT)
nvs_create_partition_image(encnvs "${PROJECT_DIR}/../nvs/enc.csv" FLASH_IN_PROJECT)
