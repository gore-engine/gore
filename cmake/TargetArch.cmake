set(archdetect_c_code "
#if defined(_M_X64) || defined(__x86_64__)
    #error cmake_ARCH x64
#elif defined(_M_ARM64) || defined(__aarch64__)
    #error cmake_ARCH arm64
#endif

#error cmake_ARCH unknown
")

function(target_architecture output_var)
    file(WRITE "${CMAKE_BINARY_DIR}/arch.c" "${archdetect_c_code}")

    enable_language(C)

    try_run(
            run_result_unused
            compile_result_unused
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/arch.c"
            COMPILE_OUTPUT_VARIABLE ARCH
    )

    string(REGEX MATCH "cmake_ARCH ([a-zA-Z0-9_]+)" ARCH "${ARCH}")

    string(REPLACE "cmake_ARCH " "" ARCH "${ARCH}")

    if (NOT ARCH)
        set(ARCH unknown)
    endif()

    set(${output_var} "${ARCH}" PARENT_SCOPE)
endfunction()