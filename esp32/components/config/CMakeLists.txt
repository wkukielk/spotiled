idf_component_register(
    SRCS "ConfigReader.cpp"
    INCLUDE_DIRS "include"
    REQUIRES nvs
             wifi_manager
             wifi_wrap
)
