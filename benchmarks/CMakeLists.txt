cmake_minimum_required(VERSION 3.28)

include(FetchContent)

find_package(benchmark CONFIG QUIET)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-error=c2y-extensions -Wno-c2y-extensions -Wno-error=pedantic)
endif()

if(NOT TARGET benchmark::benchmark)
    message(STATUS "Google Benchmark not found in system; fetching v1.9.1 via FetchContent...")
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_DOXYGEN OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        google_benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.1
    )
    FetchContent_MakeAvailable(google_benchmark)
endif()

# Microbenchmarks
add_executable(bench_lexer micro/bench_lexer.cpp)
target_link_libraries(bench_lexer PRIVATE Execore::core benchmark::benchmark benchmark::benchmark_main)
target_compile_features(bench_lexer PRIVATE cxx_std_23)

add_executable(bench_parser micro/bench_parser.cpp)
target_link_libraries(bench_parser PRIVATE Execore::core benchmark::benchmark benchmark::benchmark_main)
target_compile_features(bench_parser PRIVATE cxx_std_23)

add_executable(bench_interpreter micro/bench_interpreter.cpp)
target_link_libraries(bench_interpreter PRIVATE Execore::core benchmark::benchmark benchmark::benchmark_main)
target_compile_features(bench_interpreter PRIVATE cxx_std_23)
