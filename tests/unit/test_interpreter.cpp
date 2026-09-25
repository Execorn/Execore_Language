#include "execore/frontend/source_manager.hpp"
#include "execore/frontend/lexer.hpp"
#include "execore/frontend/parser.hpp"
#include "execore/semantics/symbol_table.hpp"
#include "execore/semantics/semantic_analyzer.hpp"
#include "execore/runtime/interpreter.hpp"
#include <iostream>
#include <sstream>
#include <cassert>

int run_script(const std::string& code, std::string& output) {
    execore::SourceManager sm;
    auto src_opt = sm.add_source("test_inline.exe", code);

    std::ostringstream err_stream;
    std::ostringstream out_stream;
    std::istringstream in_stream;

    execore::DiagnosticEngine diag(err_stream, false);
    execore::Lexer lexer(src_opt, "test_inline.exe", diag);
    execore::Parser parser(lexer, diag);

    auto program = parser.parse_program();
    if (diag.has_errors()) {
        std::cerr << "Parser error:\n" << err_stream.str();
        return -1;
    }

    execore::SymbolTable symbols;
    execore::SemanticAnalyzer analyzer(symbols, diag);
    if (!analyzer.analyze(*program)) {
        std::cerr << "Semantic error:\n" << err_stream.str();
        return -2;
    }

    execore::Interpreter interpreter(diag, &sm, in_stream, out_stream);
    int exit_code = interpreter.execute(*program);
    output = out_stream.str();
    return exit_code;
}

void test_factorial_script() {
    std::string code =
        "def factorial(n)\n"
        "    if n <= 1\n"
        "        return 1\n"
        "    return n * factorial(n - 1)\n"
        "\n"
        "int res = factorial(5)\n"
        "print res\n"
        "return res\n";

    std::string out;
    int code_res = run_script(code, out);
    assert(code_res == 120);
    assert(out == "120\n");
    std::cout << "[PASS] test_factorial_script\n";
}

void test_loops_and_conditionals() {
    std::string code =
        "int sum = 0\n"
        "int i = 1\n"
        "while i <= 10\n"
        "    sum += i\n"
        "    i += 1\n"
        "print sum\n";

    std::string out;
    int code_res = run_script(code, out);
    assert(code_res == 0);
    assert(out == "55\n");
    std::cout << "[PASS] test_loops_and_conditionals\n";
}

void test_for_and_methods() {
    std::string code =
        "str greeting = \"Execore\"\n"
        "int len = greeting.len()\n"
        "str sub = greeting[0:3]\n"
        "print sub, len\n";

    std::string out;
    int code_res = run_script(code, out);
    assert(code_res == 0);
    assert(out == "Exe 7\n");
    std::cout << "[PASS] test_for_and_methods\n";
}

int main() {
    test_factorial_script();
    test_loops_and_conditionals();
    test_for_and_methods();
    std::cout << "All Interpreter integration tests passed successfully!\n";
    return 0;
}
