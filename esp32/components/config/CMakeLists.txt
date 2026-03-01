if(CONFIG_DEV)
    set(cert "nginx.crt")
else()
    set(cert "letsencrypt-org.pem")     
endif()


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
    EMBED_TXTFILES ${cert}
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
