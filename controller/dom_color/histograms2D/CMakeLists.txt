idf_component_register(SRCS "histogram2D.cpp"
                        "analyzers2D/basic.cpp" "analyzers2D/non_linear.cpp"
                        "sumators2D/basic.cpp" "sumators2D/non_linear.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES straux)
