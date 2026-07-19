add_library(histogram_analyzers)
set(srcs "alpha_coefficient.cpp"
         "basic.cpp"
         "non_linear.cpp"
         "wrap_around.cpp"
)
add_prefix(srcs "${CMAKE_CURRENT_LIST_DIR}/" ${srcs})
target_sources(histogram_analyzers PRIVATE ${srcs})

target_include_directories(histogram_analyzers PRIVATE ../histogram_common)
target_include_directories(histogram_analyzers PRIVATE ..)

target_link_libraries(histogram_analyzers PRIVATE histogram_common)
target_link_libraries(histogram_analyzers PRIVATE histogram_sumators)

target_include_directories(histogram_analyzers PRIVATE ../../straux)
target_link_libraries(histogram_analyzers PRIVATE ${COMPONENT_LIB})
