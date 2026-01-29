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
    "-Wno-error=missing-field-initializers"
)

set(HOST_TEST_GCC_OPTIONS
    ${HOST_TEST_GCC_OPTIONS}
    "-Wno-write-strings"
)
