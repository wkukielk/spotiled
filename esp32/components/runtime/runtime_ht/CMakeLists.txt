idf_component_register(
    SRCS "test_runtime.cpp"
    INCLUDE_DIRS "include"
    REQUIRES runtime i_ota_ht wifi_manager_ht htutils
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)