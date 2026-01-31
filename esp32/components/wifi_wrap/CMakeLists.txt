idf_component_register(
     SRCS
          "WiFi.cpp"
          "WiFiProv.cpp"
     INCLUDE_DIRS "include"
     REQUIRES  
          i_wifi
          util
          wifi_provisioning
          esp_wifi
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
target_compile_options(${COMPONENT_LIB} PRIVATE "SHELL:-Wno-unused-parameter")
