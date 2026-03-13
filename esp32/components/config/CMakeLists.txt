if(CONFIG_DEV)
    set(cert "nginx.crt")
elseif(CONFIG_RELEASE)
    set(cert "letsencrypt-org.pem")     
else()
    set(cert "cert_stub.crt")     
endif()


idf_component_register(
    SRCS "Configurer.cpp"
         "LogConfig.cpp"
         "mDNSConfig.cpp"
         "data.cpp"
         "BackendConfigurer.cpp"
         "WiFiManagerFactory.cpp"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv"
    REQUIRES nvs
             wifi_manager
             wrappers
             runtime
    EMBED_TXTFILES ${cert}
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
