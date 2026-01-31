idf_component_register(
    SRCS "main.cpp"
         "app.cpp" 
    PRIV_INCLUDE_DIRS "include"
    REQUIRES hello
             global
             runtime
             config
    PRIV_REQUIRES time_aux     
                  nvs
                  wifi_wrap
             
)

include("${PROJECT_DIR}/cmake/set_gccoptions.cmake")
