idf_component_register(
    SRCS "HandlerAdapter.cpp"
    INCLUDE_DIRS "include"
    REQUIRES time_aux
             straux
             nvs
             i_wifi
             global
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)
