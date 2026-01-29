idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES "hello"
)

include("${PROJECT_DIR}/cmake/set_gccoptions.cmake")
