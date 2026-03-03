idf_component_register(
     SRCS "HTTPHandler.cpp"
          "HTTPClient_WRAP.cpp"
          "HTTPRetryingHandler.cpp"
     INCLUDE_DIRS "include"
     REQUIRES esp_http_client
              util
     PRIV_REQUIRES i_nterfaces
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")