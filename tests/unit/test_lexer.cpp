#include "execore/frontend/lexer.hpp"
#include <iostream>
#include <cassert>
#include <sstream>

void test_basic_tokens() {
    std::ostringstream err;
    execore::DiagnosticEngine diag(err, false);
    std::string source = "int x = 42\nfloat y = 3.14\n";

    execore::Lexer lexer(source, "test.exe", diag);

    auto t1 = lexer.next_token();
    assert(t1.is(execore::TokenKind::KwInt));

    auto t2 = lexer.next_token();
    assert(t2.is(execore::TokenKind::Identifier));
    assert(t2.lexeme() == "x");

    auto t3 = lexer.next_token();
    assert(t3.is(execore::TokenKind::Assign));

    auto t4 = lexer.next_token();
    assert(t4.is(execore::TokenKind::IntLiteral));
    assert(t4.lexeme() == "42");

    auto t5 = lexer.next_token();
    assert(t5.is(execore::TokenKind::Newline));

    auto t6 = lexer.next_token();
    assert(t6.is(execore::TokenKind::KwFloat));

    auto t7 = lexer.next_token();
    assert(t7.is(execore::TokenKind::Identifier));
    assert(t7.lexeme() == "y");

    auto t8 = lexer.next_token();
    assert(t8.is(execore::TokenKind::Assign));

    auto t9 = lexer.next_token();
    assert(t9.is(execore::TokenKind::FloatLiteral));
    assert(t9.lexeme() == "3.14");

    std::cout << "[PASS] test_basic_tokens\n";
}

void test_indentation_tracking() {
    std::ostringstream err;
    execore::DiagnosticEngine diag(err, false);
    std::string source =
        "if x > 0\n"
        "    int a = 1\n"
        "    if a == 1\n"
        "        int b = 2\n"
        "    int c = 3\n"
        "int d = 4\n";

    execore::Lexer lexer(source, "indent.exe", diag, 4);

    assert(lexer.next_token().is(execore::TokenKind::KwIf));
    assert(lexer.next_token().is(execore::TokenKind::Identifier));
    assert(lexer.next_token().is(execore::TokenKind::Greater));
    assert(lexer.next_token().is(execore::TokenKind::IntLiteral));
    assert(lexer.next_token().is(execore::TokenKind::Newline));

    // First indent
    assert(lexer.next_token().is(execore::TokenKind::Indent));
    assert(lexer.next_token().is(execore::TokenKind::KwInt));
    assert(lexer.next_token().is(execore::TokenKind::Identifier));
    assert(lexer.next_token().is(execore::TokenKind::Assign));
    assert(lexer.next_token().is(execore::TokenKind::IntLiteral));
    assert(lexer.next_token().is(execore::TokenKind::Newline));

    assert(lexer.next_token().is(execore::TokenKind::KwIf));
    assert(lexer.next_token().is(execore::TokenKind::Identifier));
    assert(lexer.next_token().is(execore::TokenKind::EqualEqual));
    assert(lexer.next_token().is(execore::TokenKind::IntLiteral));
    assert(lexer.next_token().is(execore::TokenKind::Newline));

    // Second indent
    assert(lexer.next_token().is(execore::TokenKind::Indent));
    assert(lexer.next_token().is(execore::TokenKind::KwInt));
    assert(lexer.next_token().is(execore::TokenKind::Identifier));
    assert(lexer.next_token().is(execore::TokenKind::Assign));
    assert(lexer.next_token().is(execore::TokenKind::IntLiteral));
    assert(lexer.next_token().is(execore::TokenKind::Newline));

    // Dedent back to first level
    assert(lexer.next_token().is(execore::TokenKind::Dedent));
    assert(lexer.next_token().is(execore::TokenKind::KwInt));
    assert(lexer.next_token().is(execore::TokenKind::Identifier));
    assert(lexer.next_token().is(execore::TokenKind::Assign));
    assert(lexer.next_token().is(execore::TokenKind::IntLiteral));
    assert(lexer.next_token().is(execore::TokenKind::Newline));

    // Dedent back to level 0
    assert(lexer.next_token().is(execore::TokenKind::Dedent));
    assert(lexer.next_token().is(execore::TokenKind::KwInt));
    assert(lexer.next_token().is(execore::TokenKind::Identifier));
    assert(lexer.next_token().is(execore::TokenKind::Assign));
    assert(lexer.next_token().is(execore::TokenKind::IntLiteral));
    assert(lexer.next_token().is(execore::TokenKind::Newline));

    assert(lexer.next_token().is(execore::TokenKind::Eof));
    assert(!diag.has_errors());

    std::cout << "[PASS] test_indentation_tracking\n";
}

void test_string_escapes() {
    std::ostringstream err;
    execore::DiagnosticEngine diag(err, false);
    std::string source = "\"hello\\nworld\\t!\\\"\"\n";

    execore::Lexer lexer(source, "str.exe", diag);
    auto tok = lexer.next_token();
    assert(tok.is(execore::TokenKind::StringLiteral));
    assert(tok.lexeme() == "hello\nworld\t!\"");

    std::cout << "[PASS] test_string_escapes\n";
}

int main() {
    test_basic_tokens();
    test_indentation_tracking();
    test_string_escapes();
    std::cout << "All Lexer tests passed successfully!\n";
    return 0;
}
