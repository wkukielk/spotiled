include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(dom_color_tests
    color_tests.cpp
    hue_histogram_tests.cpp
    wraparound_it_tests.cpp
    dominant_color_tests.cpp
)

target_link_libraries(dom_color_tests PRIVATE dom_color gtest_main)

include(GoogleTest)
gtest_discover_tests(dom_color_tests)
