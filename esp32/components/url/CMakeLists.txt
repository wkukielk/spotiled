idf_component_register(SRCS "URL.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES http_parser util
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")