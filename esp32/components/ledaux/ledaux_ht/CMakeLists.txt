idf_component_register(
    SRCS "test_animator.cpp"
         "test_channel.cpp"
         "test_LEDC.cpp"
    INCLUDE_DIRS "include"
    REQUIRES ledaux
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)
