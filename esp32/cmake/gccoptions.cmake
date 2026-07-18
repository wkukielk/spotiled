set(COMPONENT_GCC_OPTIONS
    "-Werror=unused-function"
    "-Werror=unused-variable"
    "-Werror=unused-but-set-variable"
    "-Werror=deprecated-declarations"
    "-Werror=extra"
    "-Wunused-parameter"
    "-Wsign-compare"
    "-Wenum-conversion"
    "-Wno-missing-field-initializers"
    "-Wno-unqualified-std-cast-call"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND COMPONENT_GCC_OPTIONS "-Wsometimes-uninitialized")
endif()

set(HOST_TEST_GCC_OPTIONS
    ${COMPONENT_GCC_OPTIONS}
    "-Wno-write-strings"
)
