add_library(histogram_common)
set(srcs "histogram_aux.cpp")
add_prefix(srcs "${CMAKE_CURRENT_LIST_DIR}/" ${srcs})
target_sources(histogram_common PRIVATE ${srcs})
