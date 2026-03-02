idf_component_register(
    INCLUDE_DIRS "include"
    REQUIRES i_nterfaces
    WHOLE_ARCHIVE
)
target_link_libraries(${COMPONENT_LIB} INTERFACE gmock_main)