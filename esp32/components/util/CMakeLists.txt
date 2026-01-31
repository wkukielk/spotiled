idf_component_register(
    SRCS "util.cpp"
         "queue.cpp"
         "ToggleEventHandler.cpp"
    REQUIRES esp_event straux
    INCLUDE_DIRS "."
)

include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")

