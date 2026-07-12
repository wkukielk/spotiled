add_library(histograms)
set(srcs "histogram.cpp")
add_prefix(srcs "${CMAKE_CURRENT_LIST_DIR}/" ${srcs})
target_sources(histograms PRIVATE ${srcs})

target_include_directories(histograms PRIVATE ../histogram_common)
target_link_libraries(histograms PRIVATE histogram_common)