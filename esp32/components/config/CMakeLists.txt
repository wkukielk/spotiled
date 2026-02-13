idf_component_register(
    SRCS "LogConfig.cpp"
         "mDNSConfig.cpp"
         "data.cpp"
         "ConfigReader.cpp"
         "WiFiManagerFactory.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv"
    REQUIRES nvs
             wifi_manager
             wifi_wrap
             mdns_wrap
)
