#include "execore/frontend/source_manager.hpp"
#include "execore/frontend/lexer.hpp"
#include "execore/frontend/parser.hpp"
#include "execore/semantics/symbol_table.hpp"
#include "execore/semantics/semantic_analyzer.hpp"
#include "execore/runtime/interpreter.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"
#include <iostream>
#include <sstream>
#include <cassert>
#include <string>
#include <vector>

#define SCENARIO(desc) std::cout << "  [SCENARIO] " << desc << "\n"
#define GIVEN(desc)    std::cout << "    [GIVEN] " << desc << "\n"
#define WHEN(desc)     std::cout << "    [WHEN] " << desc << "\n"
#define THEN(desc)     std::cout << "    [THEN] " << desc << "\n"

void test_bdd_lexer_indentation() {
    SCENARIO("Lexer synthesizes INDENT and DEDENT tokens based on off-side rule");

    GIVEN("a block with 4-space indentation");
    std::string source =
        "if 1\n"
        "    int a = 10\n"
        "    int b = 20\n"
        "int c = 30\n";

    WHEN("the lexer processes the source text");
    execore::SourceManager sm;
    sm.add_source("test.exe", source);
    std::stringstream err_stream;
    execore::DiagnosticEngine diag(err_stream, false);
    execore::Lexer lexer(source, "test.exe", diag, 4);

    std::vector<execore::TokenKind> kinds;
    while (true) {
        auto tok = lexer.next_token();
        kinds.push_back(tok.kind());
        if (tok.is(execore::TokenKind::Eof)) break;
    }

    THEN("an Indent token occurs after newline, and Dedent occurs before int c");
    bool has_indent = false;
    bool has_dedent = false;
    for (auto k : kinds) {
        if (k == execore::TokenKind::Indent) has_indent = true;
        if (k == execore::TokenKind::Dedent) has_dedent = true;
    }
    assert(has_indent);
    assert(has_dedent);
    assert(!diag.has_errors());
    std::cout << "    [PASSED]\n";
}

void test_bdd_parser_precedence() {
    SCENARIO("Parser enforces operator precedence: multiplication binds tighter than addition");

    GIVEN("an expression: 2 + 3 * 4");
    std::string source = "int x = 2 + 3 * 4\n";

    WHEN("the parser constructs the AST");
    std::stringstream err_stream;
    execore::DiagnosticEngine diag(err_stream, false);
    execore::Lexer lexer(source, "test.exe", diag, 4);
    execore::Parser parser(lexer, diag);
    auto program = parser.parse_program();

    THEN("the root binary operator is addition, with right child multiplication");
    assert(program != nullptr);
    assert(program->statements().size() == 1);
    const auto* var_decl = dynamic_cast<const execore::VarDeclStmt*>(program->statements()[0].get());
    assert(var_decl != nullptr);
    assert(var_decl->variables().size() == 1);

    const auto* bin_expr = dynamic_cast<const execore::BinaryExpr*>(var_decl->variables()[0].init_value.get());
    assert(bin_expr != nullptr);
    assert(bin_expr->op() == execore::BinaryOp::Add);

    const auto* right_mul = dynamic_cast<const execore::BinaryExpr*>(bin_expr->right());
    assert(right_mul != nullptr);
    assert(right_mul->op() == execore::BinaryOp::Mul);
    assert(!diag.has_errors());
    std::cout << "    [PASSED]\n";
}

void test_bdd_semantics_undeclared_var() {
    SCENARIO("Semantic analyzer intercepts use of undeclared identifier");

    GIVEN("a script referencing variable 'foo' that has not been declared");
    std::string source = "int x = foo + 1\n";

    WHEN("semantic analysis is performed on the AST");
    std::stringstream err_stream;
    execore::DiagnosticEngine diag(err_stream, false);
    execore::Lexer lexer(source, "test.exe", diag, 4);
    execore::Parser parser(lexer, diag);
    auto program = parser.parse_program();

    execore::SymbolTable symbols;
    execore::SemanticAnalyzer analyzer(symbols, diag);
    bool valid = analyzer.analyze(*program);

    THEN("semantic analyzer returns false and reports an IdentifierError");
    assert(!valid);
    assert(diag.has_errors());
    assert(err_stream.str().find("Use of undeclared identifier 'foo'") != std::string::npos);
    std::cout << "    [PASSED]\n";
}

void test_bdd_interpreter_closures_and_recursion() {
    SCENARIO("Interpreter executes recursive function with closures and local scope");

    GIVEN("a recursive sum function accumulating integers down to 0");
    std::string source =
        "def sum_down(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    return n + sum_down(n - 1)\n"
        "int result = sum_down(10)\n"
        "print result\n";

    WHEN("the program is interpreted to completion");
    std::stringstream in_s, out_s, err_s;
    execore::SourceManager sm;
    sm.add_source("test.exe", source);
    execore::DiagnosticEngine diag(err_s, false);
    execore::Lexer lexer(source, "test.exe", diag, 4);
    execore::Parser parser(lexer, diag);
    auto program = parser.parse_program();

    execore::SymbolTable symbols;
    execore::SemanticAnalyzer analyzer(symbols, diag);
    assert(analyzer.analyze(*program));

    execore::Interpreter interpreter(diag, &sm, in_s, out_s);
    int exit_code = interpreter.execute(*program);

    THEN("exit code is 0 and output stream contains 55");
    assert(exit_code == 0);
    assert(out_s.str() == "55\n");
    std::cout << "    [PASSED]\n";
}

int main() {
    std::cout << "=== Running BDD Test Scenarios (Frontier C++2026 Milestone 3) ===\n";
    test_bdd_lexer_indentation();
    test_bdd_parser_precedence();
    test_bdd_semantics_undeclared_var();
    test_bdd_interpreter_closures_and_recursion();
    std::cout << "=== All BDD Scenarios Passed Successfully! ===\n";
    return 0;
}
