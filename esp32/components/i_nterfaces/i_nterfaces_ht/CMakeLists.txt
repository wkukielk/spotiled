idf_component_register(
    SRCS "MockDftEventLoop.cpp"
    INCLUDE_DIRS "include"
    REQUIRES i_nterfaces
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PUBLIC gmock_main)
