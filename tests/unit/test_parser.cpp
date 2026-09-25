#include "execore/frontend/lexer.hpp"
#include "execore/frontend/parser.hpp"
#include <iostream>
#include <cassert>
#include <sstream>

void test_parse_simple_program() {
    std::ostringstream err;
    execore::DiagnosticEngine diag(err, false);
    std::string source =
        "int a = 10, b = 20\n"
        "int c = a + b * 2\n";

    execore::Lexer lexer(source, "test.exe", diag);
    execore::Parser parser(lexer, diag);

    auto program = parser.parse_program();
    assert(program != nullptr);
    assert(!diag.has_errors());
    assert(program->statements().size() == 2);

    std::cout << "[PASS] test_parse_simple_program\n";
}

void test_parse_function_and_if() {
    std::ostringstream err;
    execore::DiagnosticEngine diag(err, false);
    std::string source =
        "def factorial(n)\n"
        "    if n <= 1\n"
        "        return 1\n"
        "    return n * factorial(n - 1)\n";

    execore::Lexer lexer(source, "fact.exe", diag);
    execore::Parser parser(lexer, diag);

    auto program = parser.parse_program();
    assert(program != nullptr);
    assert(!diag.has_errors());
    assert(program->statements().size() == 1);

    auto* func = dynamic_cast<execore::FunctionDeclStmt*>(program->statements()[0].get());
    assert(func != nullptr);
    assert(func->name() == "factorial");
    assert(func->params().size() == 1);
    assert(func->params()[0] == "n");
    assert(func->body() != nullptr);
    assert(func->body()->statements().size() == 2);

    std::cout << "[PASS] test_parse_function_and_if\n";
}

void test_parse_slicing_and_methods() {
    std::ostringstream err;
    execore::DiagnosticEngine diag(err, false);
    std::string source =
        "str s = \"hello\"\n"
        "str sub = s[1:4]\n"
        "int len = s.len()\n";

    execore::Lexer lexer(source, "slice.exe", diag);
    execore::Parser parser(lexer, diag);

    auto program = parser.parse_program();
    assert(program != nullptr);
    assert(!diag.has_errors());
    assert(program->statements().size() == 3);

    std::cout << "[PASS] test_parse_slicing_and_methods\n";
}

void test_parse_monadic_pipeline() {
    std::ostringstream err;
    execore::DiagnosticEngine diag(err, false);
    std::string valid_source =
        "int x = 42\n"
        "int y = x + 10\n";

    execore::Lexer lexer(valid_source, "monadic.exe", diag);
    execore::Parser parser(lexer, diag);

    // Test C++23 monadic pipeline chaining (.transform and .and_then)
    auto result = parser.parse_program_monadic()
        .transform([](const std::unique_ptr<execore::Program>& prog) -> size_t {
            return prog->statements().size();
        });

    assert(result.has_value());
    assert(result.value() == 2);

    // Test error branch with invalid syntax
    std::ostringstream err_bad;
    execore::DiagnosticEngine diag_bad(err_bad, false);
    std::string invalid_source = "int x = \n"; // Missing expression
    execore::Lexer lexer_bad(invalid_source, "bad.exe", diag_bad);
    execore::Parser parser_bad(lexer_bad, diag_bad);

    auto result_bad = parser_bad.parse_program_monadic();
    assert(!result_bad.has_value());
    assert(result_bad.error().message.find("Syntax error") != std::string::npos);

    std::cout << "[PASS] test_parse_monadic_pipeline\n";
}

int main() {
    test_parse_simple_program();
    test_parse_function_and_if();
    test_parse_slicing_and_methods();
    test_parse_monadic_pipeline();
    std::cout << "All Parser tests passed successfully!\n";
    return 0;
}
