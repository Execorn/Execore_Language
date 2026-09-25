# Binary Hardening Flags for Execore Production Release
function(execore_enable_hardening target_name)
    option(EXECORE_ENABLE_HARDENING "Enable security hardening compile and link flags" OFF)

    if(EXECORE_ENABLE_HARDENING)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            # Buffer overflow checks & stack smashing defense
            target_compile_definitions(${target_name} PRIVATE _FORTIFY_SOURCE=3)
            target_compile_options(${target_name} PRIVATE
                -fstack-protector-strong
                -fstack-clash-protection
            )

            # Control-flow enforcement (Intel CET / AMD shadow stack) if x86_64
            if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
                target_compile_options(${target_name} PRIVATE -fcf-protection=full)
            endif()

            # Read-only relocations & immediate binding (Full RELRO)
            if(NOT APPLE AND NOT WIN32)
                target_link_options(${target_name} PRIVATE
                    -Wl,-z,relro
                    -Wl,-z,now
                    -Wl,-z,noexecstack
                )
            endif()

            # Position-Independent Executable
            set_target_properties(${target_name} PROPERTIES
                POSITION_INDEPENDENT_CODE ON
            )
            message(STATUS "Execore [${target_name}]: Security hardening flags enabled (_FORTIFY_SOURCE=3, stack-protector-strong, full RELRO)")
        endif()
    endif()
endfunction()
