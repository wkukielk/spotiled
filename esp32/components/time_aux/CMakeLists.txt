idf_component_register(
    SRCS "time_aux.cpp"
         "time_aux_core.c"
    INCLUDE_DIRS "."
)
include("${CMAKE_CONFIG_DIR}/set_gccoptions.cmake")
