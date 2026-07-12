cmake_minimum_required(VERSION 3.16)
project(dom_color CXX)

add_library(dom_color STATIC
    straux/straux.cpp

    colors/hsv.cpp
    colors/rgb.cpp
    colors/v_cluster.cpp
    colors/hsv_cluster.cpp

    histograms/histogram_common/histogram_aux.cpp
    histograms/histogram/histogram.cpp
    histograms/analyzers/alpha_coefficient.cpp
    histograms/analyzers/basic.cpp
    histograms/analyzers/non_linear.cpp
    histograms/analyzers/wrap_around.cpp
    histograms/sumators/basic.cpp
    histograms/sumators/non_linear.cpp
    histograms/sumators/wrap_around.cpp

    histograms2D/histogram2D.cpp
    histograms2D/analyzers2D/basic.cpp
    histograms2D/analyzers2D/non_linear.cpp
    histograms2D/sumators2D/basic.cpp
    histograms2D/sumators2D/non_linear.cpp

    hsv_histograms/hue_histogram.cpp
    hsv_histograms/sv_histogram.cpp
)

target_compile_features(dom_color PUBLIC cxx_std_20)

target_include_directories(dom_color PUBLIC
    straux
    colors
    histograms2D
    histograms
    histograms/histogram
    histograms/histogram_common
    hsv_histograms
)

include(gccoptions.cmake)
