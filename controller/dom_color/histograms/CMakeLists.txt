idf_component_register(
    INCLUDE_DIRS "histogram"
                 "histogram_common"
                 "."
    REQUIRES straux
)
add_subdirectory(analyzers)
target_link_libraries(${COMPONENT_LIB} INTERFACE histogram_analyzers)
add_subdirectory(histogram)
target_link_libraries(${COMPONENT_LIB} INTERFACE histograms)
add_subdirectory(sumators)
target_link_libraries(${COMPONENT_LIB} INTERFACE histogram_sumators)
add_subdirectory(histogram_common)
target_link_libraries(${COMPONENT_LIB} INTERFACE histogram_common)