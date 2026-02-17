idf_component_register(
    SRCS "Configurer.cpp" "LogConfig.cpp"
         "mDNSConfig.cpp"
         "data.cpp"
        
         "WiFiManagerFactory.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv"
    REQUIRES nvs
             wifi_manager
             wifi_wrap
             mdns_wrap
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
