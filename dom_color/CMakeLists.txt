cmake_minimum_required(VERSION 3.16)
project(dom_color CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(dom_color STATIC
    src/dominant_color.cpp

    src/colors/hsv.cpp
    src/colors/rgb.cpp
    src/colors/v_cluster.cpp
    src/colors/hsv_cluster.cpp

    src/histograms/histogram_common/histogram_aux.cpp
    src/histograms/histogram/histogram.cpp
    src/histograms/analyzers/alpha_coefficient.cpp
    src/histograms/analyzers/search_algorithm.cpp
    src/histograms/sumators/common.cpp
    src/histograms/sumators/basic.cpp
    src/histograms/sumators/non_linear.cpp
    src/histograms/sumators/wrap_around.cpp

    src/histograms2D/histogram2D.cpp
    src/histograms2D/analyzers2D/basic.cpp
    src/histograms2D/analyzers2D/non_linear.cpp
    src/histograms2D/sumators2D/basic.cpp
    src/histograms2D/sumators2D/non_linear.cpp

    src/hsv_histograms/hue_histogram.cpp
    src/hsv_histograms/sv_histogram.cpp
)

target_compile_features(dom_color PUBLIC cxx_std_20)

target_include_directories(dom_color PUBLIC
    src/
    src/straux
    src/colors
    src/histograms2D
    src/histograms
    src/histograms/histogram
    src/histograms/histogram_common
    src/hsv_histograms
)

include(gccoptions.cmake)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(DOM_COLOR_BUILD_TESTS "Build dom_color unit tests" ON)
    if(DOM_COLOR_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
