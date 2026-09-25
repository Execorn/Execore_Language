#pragma once

#include "execore/frontend/lexer.hpp"
#include "execore/ast/statements.hpp"
#include "execore/ast/expressions.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"
#include "execore/common/result.hpp"
#include "execore/common/concepts.hpp"
#include <memory>
#include <vector>

namespace execore {

struct ParseError {
    std::string message;
    SourceLocation location;
};

inline std::string_view to_string(const ParseError& err) noexcept {
    return err.message;
}
static_assert(DomainErrorConcept<ParseError>, "ParseError must satisfy DomainErrorConcept");

class Parser {
public:
    Parser(Lexer& lexer, DiagnosticEngine& diag);

    std::unique_ptr<Program> parse_program();
    Result<std::unique_ptr<Program>, ParseError> parse_program_monadic();

private:
    // Core token manipulation
    const Token& current() const noexcept;
    const Token& previous() const noexcept;
    bool check(TokenKind kind) const noexcept;
    bool match(TokenKind kind) noexcept;
    template <typename... Rest>
    bool match(TokenKind k1, Rest... rest) noexcept {
        if (check(k1)) { advance(); return true; }
        return match(rest...);
    }
    const Token& advance() noexcept;
    const Token& consume(TokenKind kind, const std::string& error_message);
    void synchronize();

    // Statements
    std::unique_ptr<Stmt> parse_statement();
    std::unique_ptr<VarDeclStmt> parse_var_decl_statement();
    std::unique_ptr<FunctionDeclStmt> parse_function_decl_statement();
    std::unique_ptr<IfStmt> parse_if_statement();
    std::unique_ptr<WhileStmt> parse_while_statement();
    std::unique_ptr<DoWhileStmt> parse_do_while_statement();
    std::unique_ptr<ForStmt> parse_for_statement();
    std::unique_ptr<ReturnStmt> parse_return_statement();
    std::unique_ptr<PrintStmt> parse_print_statement();
    std::unique_ptr<InputStmt> parse_input_statement();
    std::unique_ptr<ImportStmt> parse_import_statement();
    std::unique_ptr<BlockStmt> parse_block();

    // Expressions
    std::unique_ptr<Expr> parse_expression();
    std::unique_ptr<Expr> parse_assignment();
    std::unique_ptr<Expr> parse_logical_or();
    std::unique_ptr<Expr> parse_logical_and();
    std::unique_ptr<Expr> parse_equality();
    std::unique_ptr<Expr> parse_relational();
    std::unique_ptr<Expr> parse_additive();
    std::unique_ptr<Expr> parse_multiplicative();
    std::unique_ptr<Expr> parse_unary();
    std::unique_ptr<Expr> parse_postfix();
    std::unique_ptr<Expr> parse_primary();

    TypeKind token_to_type(TokenKind kind) const noexcept;
    void consume_newlines();

    Lexer& lexer_;
    DiagnosticEngine& diag_;
    Token current_token_;
    Token previous_token_;
};

} // namespace execore
