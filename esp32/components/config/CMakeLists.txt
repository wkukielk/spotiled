idf_component_register(
    SRCS "data.cpp" "ConfigReader.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv"
    REQUIRES nvs
             wifi_manager
             wifi_wrap
             mdns_wrap
)
