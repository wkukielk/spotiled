idf_component_register(
    SRCS "test_Provisioning.cpp" "test_WiFiEventHandler.cpp"
         "test_ProvEventHandler.cpp"
        
         "test_WiFiManager.cpp"
    REQUIRES wifi_manager
             htutils
    WHOLE_ARCHIVE
)
include("${CMAKE_CONFIG_DIR}/set_ht_gccoptions.cmake")
target_compile_options(${COMPONENT_LIB} PRIVATE "SHELL:-Wno-missing-field-initializers")
target_link_libraries(${COMPONENT_LIB} PRIVATE gmock_main)
