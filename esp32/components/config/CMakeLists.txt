idf_component_register(
    SRCS "Configurer.cpp" "LogConfig.cpp"
         "mDNSConfig.cpp"
         "data.cpp"
         "BackendConfigurer.cpp"
         "WiFiManagerFactory.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv"
    REQUIRES nvs
             wifi_manager
             wifi_wrap
             mdns_wrap
             ota_wrap
             runtime
    EMBED_TXTFILES "letsencrypt-org.pem"
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
