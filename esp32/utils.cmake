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


macro(esp_component_register)
    idf_component_register(${ARGN})
    if(TARGET ${COMPONENT_LIB})
        target_compile_options(${COMPONENT_LIB} PRIVATE ${COMPONENT_GCC_OPTIONS})
    endif()
endmacro()
