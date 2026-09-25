cmake_minimum_required(VERSION 3.28)

# 1. Unit & Core Integration Tests
add_executable(test_lexer unit/test_lexer.cpp)
target_link_libraries(test_lexer PRIVATE Execore::core)
add_test(NAME LexerTest COMMAND test_lexer)

add_executable(test_parser unit/test_parser.cpp)
target_link_libraries(test_parser PRIVATE Execore::core)
add_test(NAME ParserTest COMMAND test_parser)

add_executable(test_value unit/test_value.cpp)
target_link_libraries(test_value PRIVATE Execore::core)
add_test(NAME ValueTest COMMAND test_value)

add_executable(test_interpreter unit/test_interpreter.cpp)
target_link_libraries(test_interpreter PRIVATE Execore::core)
add_test(NAME InterpreterTest COMMAND test_interpreter)

add_executable(test_arena unit/test_arena.cpp)
target_link_libraries(test_arena PRIVATE Execore::core)
add_test(NAME ArenaTest COMMAND test_arena)

add_executable(test_pmr_and_monadic unit/test_pmr_and_monadic.cpp)
target_link_libraries(test_pmr_and_monadic PRIVATE Execore::core)
add_test(NAME PmrAndMonadicTest COMMAND test_pmr_and_monadic)

# 2. BDD Scenario Testing (GIVEN, WHEN, THEN)
add_executable(test_bdd_scenarios unit/test_bdd_scenarios.cpp)
target_link_libraries(test_bdd_scenarios PRIVATE Execore::core)
add_test(NAME BddScenariosTest COMMAND test_bdd_scenarios)

# 3. Property-Based Testing
add_executable(test_properties property/test_properties.cpp)
target_link_libraries(test_properties PRIVATE Execore::core)
add_test(NAME PropertyBasedTest COMMAND test_properties)

# 4. Standalone Continuous Fuzz Regression Harness
add_executable(test_fuzz_regression fuzz/fuzz_parser.cpp fuzz/fuzz_standalone_runner.cpp)
target_link_libraries(test_fuzz_regression PRIVATE Execore::core)
add_test(NAME FuzzRegressionTest COMMAND test_fuzz_regression)

# 5. Continuous LibFuzzer Targets (Clang only)
if(EXECORE_BUILD_FUZZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_executable(fuzz_lexer fuzz/fuzz_lexer.cpp)
    target_link_libraries(fuzz_lexer PRIVATE Execore::core)
    target_compile_options(fuzz_lexer PRIVATE -fsanitize=fuzzer,address)
    target_link_options(fuzz_lexer PRIVATE -fsanitize=fuzzer,address)

    add_executable(fuzz_parser fuzz/fuzz_parser.cpp)
    target_link_libraries(fuzz_parser PRIVATE Execore::core)
    target_compile_options(fuzz_parser PRIVATE -fsanitize=fuzzer,address)
    target_link_options(fuzz_parser PRIVATE -fsanitize=fuzzer,address)
endif()
