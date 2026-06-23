idf_component_register(
     SRCS "hue_histogram.cpp"
          "sv_histogram.cpp"
     INCLUDE_DIRS "."
     REQUIRES histograms
              histograms2D
              colors
)
include(../../gccoptions.cmake)