set(requires "esp_event straux")

if(IDF_TARGET STREQUAL "linux")
    list(APPEND requires "mocks")
endif()

idf_component_register(
    SRCS "util.cpp"
         "queue.cpp"
         "ToggleEventHandler.cpp"
    REQUIRES ${requires}
    INCLUDE_DIRS "."
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")

