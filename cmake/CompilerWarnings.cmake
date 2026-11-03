# Compiler warning configuration for Execore
option(EXECORE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

function(execore_set_compiler_warnings target_name)
    set(MSVC_WARNINGS
        /W4
        /permissive-
        /w14242
        /w14254
        /w14263
        /w14265
        /w14287
        /we4289
        /w14296
        /w14311
        /w14545
        /w14546
        /w14547
        /w14549
        /w14555
        /w14619
        /w14640
        /w14826
        /w14905
        /w14906
        /w14928
    )

    set(CLANG_GCC_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wformat=2
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND CLANG_GCC_WARNINGS -Wno-interference-size)
    endif()

    if(EXECORE_WARNINGS_AS_ERRORS)
        list(APPEND MSVC_WARNINGS /WX)
        list(APPEND CLANG_GCC_WARNINGS -Werror)
    endif()

    if(MSVC)
        target_compile_options(${target_name} PRIVATE ${MSVC_WARNINGS})
    else()
        target_compile_options(${target_name} PRIVATE ${CLANG_GCC_WARNINGS})
    endif()
endfunction()
