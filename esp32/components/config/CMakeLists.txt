idf_component_register(
    SRCS "mDNSConfig.cpp" "data.cpp" "ConfigReader.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv"
    REQUIRES nvs
             wifi_manager
             wifi_wrap
             mdns_wrap
)
