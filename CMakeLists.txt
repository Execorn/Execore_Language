cmake_minimum_required(VERSION 3.25...3.28)

# Fatal guard: strictly forbid in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are strictly forbidden. Please use 'cmake --preset dev-clang' or specify a separate binary directory (e.g. 'cmake -B build').")
endif()

project(Execore
    VERSION 2.0.0
    DESCRIPTION "Frontier Mission-Critical Language Frontend and Runtime Engine (C++2026 SOTA)"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fast compilation caching (sccache / ccache)
find_program(SCCACHE_PROGRAM sccache)
find_program(CCACHE_PROGRAM ccache)
if(SCCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    message(STATUS "Execore: Enabled sccache compiler launcher (${SCCACHE_PROGRAM})")
elseif(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Execore: Enabled ccache compiler launcher (${CCACHE_PROGRAM})")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Sanitizers)
execore_setup_sanitizers()
include(Linker)
execore_configure_global_linker()
include(Hardening)
include(PGO)

option(EXECORE_BUILD_TESTS "Build Execore test suite" ON)
option(EXECORE_BUILD_BENCHMARKS "Build Execore performance benchmarks" OFF)
option(EXECORE_BUILD_FUZZERS "Build continuous fuzz targets" OFF)

# Core library sources (Pitchfork layout: src/)
set(EXECORE_LIB_SOURCES
    src/common/source_location.cpp
    src/diagnostics/diagnostic_engine.cpp
    src/frontend/source_manager.cpp
    src/frontend/token.cpp
    src/frontend/token_soa.cpp
    src/frontend/lexer.cpp
    src/frontend/parser.cpp
    src/ast/ast_node.cpp
    src/ast/expressions.cpp
    src/ast/statements.cpp
    src/semantics/symbol_table.cpp
    src/semantics/semantic_analyzer.cpp
    src/runtime/value.cpp
    src/runtime/object.cpp
    src/runtime/environment.cpp
    src/runtime/builtins.cpp
    src/runtime/interpreter.cpp
    src/utils/dot_exporter.cpp
    src/utils/string_utils.cpp
)

add_library(libexecore STATIC ${EXECORE_LIB_SOURCES})
add_library(Execore::core ALIAS libexecore)
add_library(Execore::execore ALIAS libexecore)
add_library(execore::libexecore ALIAS libexecore)

target_include_directories(libexecore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(libexecore PUBLIC cxx_std_23)

execore_set_compiler_warnings(libexecore)
execore_enable_sanitizers(libexecore)
execore_configure_linker(libexecore)
execore_enable_hardening(libexecore)
execore_configure_pgo(libexecore)

# CLI executable (tools/execore)
add_executable(execore tools/execore/main.cpp)
add_executable(Execore::cli ALIAS execore)

target_link_libraries(execore PRIVATE Execore::core)
target_compile_features(execore PRIVATE cxx_std_23)

execore_set_compiler_warnings(execore)
execore_enable_sanitizers(execore)
execore_configure_linker(execore)
execore_enable_hardening(execore)
execore_configure_pgo(execore)

# Testing
if(EXECORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Benchmarks
if(EXECORE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
