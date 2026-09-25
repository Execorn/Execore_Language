function(execore_configure_global_linker)
    option(EXECORE_USE_MOLD "Use mold linker if available" ON)
    option(EXECORE_USE_LLD "Use lld linker if available" ON)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        find_program(MOLD_PATH mold)
        find_program(LLD_PATH ld.lld)

        if(EXECORE_USE_MOLD AND MOLD_PATH)
            add_link_options(-fuse-ld=mold)
            message(STATUS "Execore: globally using mold linker (${MOLD_PATH})")
        elseif(EXECORE_USE_LLD AND LLD_PATH)
            add_link_options(-fuse-ld=lld)
            message(STATUS "Execore: globally using lld linker (${LLD_PATH})")
        endif()
    endif()
endfunction()

function(execore_configure_linker target_name)
    # Kept for per-target overrides if needed
endfunction()
