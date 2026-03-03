idf_component_register(
    SRCS "app_main.c"
    PRIV_REQUIRES test_utils
)

nvs_create_partition_image(cfg "${PROJECT_DIR}/../nvs/devconfig.csv" FLASH_IN_PROJECT)
nvs_create_partition_image(encnvs "${PROJECT_DIR}/../nvs/enc.csv" FLASH_IN_PROJECT)
