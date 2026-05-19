set(requires "esp_event http_parser")

if(IDF_TARGET STREQUAL "linux")
    list(APPEND requires "mocks")
endif()

idf_component_register(
    SRCS "util.cpp"        
         "ToggleEventHandler.cpp"
         "std/straux.cpp"
         "std/CharVector.cpp"
         "URL.cpp"
         "time/aux.cpp"
         "time/core.c"
    INCLUDE_DIRS "include"
                 "std/include"
                 "time/include"
    REQUIRES ${requires}
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")

