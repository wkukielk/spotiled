add_library(histogram_sumators)
set(srcs "basic.cpp"
         "non_linear.cpp"
         "wrap_around.cpp"
)
add_prefix(srcs "${CMAKE_CURRENT_LIST_DIR}/" ${srcs})
target_sources(histogram_sumators PRIVATE ${srcs})

target_include_directories(histogram_sumators PRIVATE ../histogram_common)
target_link_libraries(histogram_sumators PRIVATE histogram_common)

target_include_directories(histogram_sumators PRIVATE ../../straux)
target_link_libraries(histogram_sumators PRIVATE ${COMPONENT_LIB})