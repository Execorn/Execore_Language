#include "execore/frontend/parser.hpp"

namespace execore {

Parser::Parser(Lexer& lexer, DiagnosticEngine& diag)
    : lexer_(lexer), diag_(diag) {
    advance();
}

const Token& Parser::current() const noexcept {
    return current_token_;
}

const Token& Parser::previous() const noexcept {
    return previous_token_;
}

bool Parser::check(TokenKind kind) const noexcept {
    return current_token_.kind() == kind;
}

bool Parser::match(TokenKind kind) noexcept {
    if (check(kind)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::advance() noexcept {
    previous_token_ = std::move(current_token_);
    current_token_ = lexer_.next_token();
    return previous_token_;
}

const Token& Parser::consume(TokenKind kind, const std::string& error_message) {
    if (check(kind)) {
        return advance();
    }
    diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                 error_message + " (got " + std::string(to_string(current().kind())) + ")",
                 current().span());
    return current();
}

void Parser::consume_newlines() {
    while (match(TokenKind::Newline)) {}
}

void Parser::synchronize() {
    advance();
    while (!check(TokenKind::Eof)) {
        if (previous().kind() == TokenKind::Newline) return;
        switch (current().kind()) {
            case TokenKind::KwDef:
            case TokenKind::KwIf:
            case TokenKind::KwWhile:
            case TokenKind::KwDo:
            case TokenKind::KwFor:
            case TokenKind::KwReturn:
            case TokenKind::KwPrint:
            case TokenKind::KwInput:
            case TokenKind::KwImport:
            case TokenKind::KwChar:
            case TokenKind::KwInt:
            case TokenKind::KwFloat:
            case TokenKind::KwStr:
            case TokenKind::KwList:
                return;
            default:
                advance();
                break;
        }
    }
}

TypeKind Parser::token_to_type(TokenKind kind) const noexcept {
    switch (kind) {
        case TokenKind::KwChar:  return TypeKind::Char;
        case TokenKind::KwInt:   return TypeKind::Int;
        case TokenKind::KwFloat: return TypeKind::Float;
        case TokenKind::KwStr:   return TypeKind::Str;
        case TokenKind::KwList:  return TypeKind::List;
        default:                 return TypeKind::None;
    }
}

std::unique_ptr<Program> Parser::parse_program() {
    auto program = std::make_unique<Program>();
    consume_newlines();

    while (!check(TokenKind::Eof)) {
        if (auto stmt = parse_statement()) {
            program->add_statement(std::move(stmt));
        } else {
            synchronize();
        }
        consume_newlines();
    }

    return program;
}

Result<std::unique_ptr<Program>, ParseError> Parser::parse_program_monadic() {
    auto program = parse_program();
    if (diag_.has_errors()) {
        return std::unexpected(ParseError{"Syntax error detected during compilation", current().span().start});
    }
    return program;
}

std::unique_ptr<Stmt> Parser::parse_statement() {
    consume_newlines();
    if (check(TokenKind::Eof)) return nullptr;

    SourceLocation start_loc = current().span().start;

    if (current().is_type_keyword()) {
        return parse_var_decl_statement();
    }

    if (match(TokenKind::KwDef)) {
        return parse_function_decl_statement();
    }

    if (match(TokenKind::KwIf)) {
        return parse_if_statement();
    }
    if (match(TokenKind::KwWhile)) {
        return parse_while_statement();
    }
    if (match(TokenKind::KwDo)) {
        return parse_do_while_statement();
    }
    if (match(TokenKind::KwFor)) {
        return parse_for_statement();
    }
    if (match(TokenKind::KwReturn)) {
        return parse_return_statement();
    }
    if (match(TokenKind::KwBreak)) {
        auto span = SourceSpan{start_loc, previous().span().end, lexer_.filename()};
        match(TokenKind::Newline);
        return std::make_unique<BreakStmt>(span);
    }
    if (match(TokenKind::KwContinue)) {
        auto span = SourceSpan{start_loc, previous().span().end, lexer_.filename()};
        match(TokenKind::Newline);
        return std::make_unique<ContinueStmt>(span);
    }
    if (match(TokenKind::KwPass)) {
        auto span = SourceSpan{start_loc, previous().span().end, lexer_.filename()};
        match(TokenKind::Newline);
        return std::make_unique<PassStmt>(span);
    }

    if (match(TokenKind::KwPrint)) {
        return parse_print_statement();
    }
    if (match(TokenKind::KwInput)) {
        return parse_input_statement();
    }
    if (match(TokenKind::KwImport)) {
        return parse_import_statement();
    }

    auto expr = parse_expression();
    if (!expr) return nullptr;

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    match(TokenKind::Newline);
    return std::make_unique<ExprStmt>(std::move(expr), span);
}

std::unique_ptr<VarDeclStmt> Parser::parse_var_decl_statement() {
    SourceLocation start_loc = current().span().start;
    TypeKind type = token_to_type(advance().kind());

    std::vector<VarInit> variables;

    do {
        if (!check(TokenKind::Identifier)) {
            diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                         "Expected variable name in declaration", current().span());
            break;
        }
        Token name_tok = advance();
        std::unique_ptr<Expr> init_val = nullptr;

        if (match(TokenKind::Assign)) {
            init_val = parse_assignment();
        }

        variables.push_back(VarInit{name_tok.lexeme(), std::move(init_val), name_tok.span()});
    } while (match(TokenKind::Comma));

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    match(TokenKind::Newline);
    return std::make_unique<VarDeclStmt>(type, std::move(variables), span);
}

std::unique_ptr<FunctionDeclStmt> Parser::parse_function_decl_statement() {
    SourceLocation start_loc = previous().span().start;

    if (!check(TokenKind::Identifier)) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                     "Expected function name after 'def'", current().span());
        return nullptr;
    }
    std::string name = advance().lexeme();

    consume(TokenKind::LeftParen, "Expected '(' after function name");

    std::vector<std::string> params;
    if (!check(TokenKind::RightParen)) {
        do {
            if (!check(TokenKind::Identifier)) {
                diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                             "Expected parameter name", current().span());
                break;
            }
            params.push_back(advance().lexeme());
        } while (match(TokenKind::Comma));
    }
    consume(TokenKind::RightParen, "Expected ')' after parameters");

    auto body = parse_block();
    SourceSpan span{start_loc, body ? body->span().end : previous().span().end, lexer_.filename()};
    return std::make_unique<FunctionDeclStmt>(std::move(name), std::move(params), std::move(body), span);
}

std::unique_ptr<BlockStmt> Parser::parse_block() {
    SourceLocation start_loc = current().span().start;
    consume_newlines();

    if (!match(TokenKind::Indent)) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                     "Expected indented block", current().span());
        return nullptr;
    }

    auto block = std::make_unique<BlockStmt>();
    consume_newlines();

    while (!check(TokenKind::Dedent) && !check(TokenKind::Eof)) {
        if (auto stmt = parse_statement()) {
            block->add_statement(std::move(stmt));
        } else {
            synchronize();
        }
        consume_newlines();
    }

    consume(TokenKind::Dedent, "Expected unindent (dedent) at end of block");
    block->set_span(SourceSpan{start_loc, previous().span().end, lexer_.filename()});
    return block;
}

std::unique_ptr<IfStmt> Parser::parse_if_statement() {
    SourceLocation start_loc = previous().span().start;

    auto condition = parse_expression();
    auto then_branch = parse_block();
    std::unique_ptr<BlockStmt> else_branch = nullptr;

    consume_newlines();
    if (match(TokenKind::KwElse)) {
        else_branch = parse_block();
    }

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch), std::move(else_branch), span);
}

std::unique_ptr<WhileStmt> Parser::parse_while_statement() {
    SourceLocation start_loc = previous().span().start;

    auto condition = parse_expression();
    auto body = parse_block();

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body), span);
}

std::unique_ptr<DoWhileStmt> Parser::parse_do_while_statement() {
    SourceLocation start_loc = previous().span().start;

    auto body = parse_block();
    consume(TokenKind::KwWhile, "Expected 'while' after 'do' block");
    auto condition = parse_expression();
    match(TokenKind::Newline);

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    return std::make_unique<DoWhileStmt>(std::move(body), std::move(condition), span);
}

std::unique_ptr<ForStmt> Parser::parse_for_statement() {
    SourceLocation start_loc = previous().span().start;

    if (!check(TokenKind::Identifier)) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                     "Expected loop variable name after 'for'", current().span());
        return nullptr;
    }
    std::string var_name = advance().lexeme();

    consume(TokenKind::KwIn, "Expected 'in' in for loop");
    auto sequence = parse_expression();
    auto body = parse_block();

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    return std::make_unique<ForStmt>(std::move(var_name), std::move(sequence), std::move(body), span);
}

std::unique_ptr<ReturnStmt> Parser::parse_return_statement() {
    SourceLocation start_loc = previous().span().start;
    std::unique_ptr<Expr> val = nullptr;

    if (!check(TokenKind::Newline) && !check(TokenKind::Eof)) {
        val = parse_expression();
    }

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    match(TokenKind::Newline);
    return std::make_unique<ReturnStmt>(std::move(val), span);
}

std::unique_ptr<PrintStmt> Parser::parse_print_statement() {
    SourceLocation start_loc = previous().span().start;
    bool raw = false;

    if (check(TokenKind::Minus)) {
        advance();
        if (check(TokenKind::Identifier) && current().lexeme() == "raw") {
            advance();
            raw = true;
        }
    }

    std::vector<std::unique_ptr<Expr>> expressions;
    if (!check(TokenKind::Newline) && !check(TokenKind::Eof)) {
        do {
            if (auto expr = parse_assignment()) {
                expressions.push_back(std::move(expr));
            } else break;
        } while (match(TokenKind::Comma));
    }

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    match(TokenKind::Newline);
    return std::make_unique<PrintStmt>(raw, std::move(expressions), span);
}

std::unique_ptr<InputStmt> Parser::parse_input_statement() {
    SourceLocation start_loc = previous().span().start;
    std::vector<InputItem> items;

    do {
        std::string prompt;
        if (check(TokenKind::StringLiteral)) {
            prompt = advance().lexeme();
        }

        if (!check(TokenKind::Identifier)) {
            diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                         "Expected variable name in input statement", current().span());
            break;
        }
        Token var_tok = advance();
        items.push_back(InputItem{std::move(prompt), var_tok.lexeme(), var_tok.span()});
    } while (match(TokenKind::Comma));

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    match(TokenKind::Newline);
    return std::make_unique<InputStmt>(std::move(items), span);
}

std::unique_ptr<ImportStmt> Parser::parse_import_statement() {
    SourceLocation start_loc = previous().span().start;
    std::vector<std::unique_ptr<Expr>> modules;

    do {
        if (auto expr = parse_assignment()) {
            modules.push_back(std::move(expr));
        } else break;
    } while (match(TokenKind::Comma));

    SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
    match(TokenKind::Newline);
    return std::make_unique<ImportStmt>(std::move(modules), span);
}

std::unique_ptr<Expr> Parser::parse_expression() {
    SourceLocation start_loc = current().span().start;
    auto expr = parse_assignment();
    if (!expr) return nullptr;

    if (match(TokenKind::Comma)) {
        std::vector<std::unique_ptr<Expr>> exprs;
        exprs.push_back(std::move(expr));
        do {
            if (auto next_expr = parse_assignment()) {
                exprs.push_back(std::move(next_expr));
            } else break;
        } while (match(TokenKind::Comma));

        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        return std::make_unique<CommaExpr>(std::move(exprs), span);
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parse_assignment() {
    SourceLocation start_loc = current().span().start;
    auto left = parse_logical_or();
    if (!left) return nullptr;

    if (current().is_assignment_op()) {
        Token op_tok = advance();
        AssignOp op = AssignOp::Assign;
        switch (op_tok.kind()) {
            case TokenKind::Assign:        op = AssignOp::Assign; break;
            case TokenKind::PlusAssign:    op = AssignOp::AddAssign; break;
            case TokenKind::MinusAssign:   op = AssignOp::SubAssign; break;
            case TokenKind::StarAssign:    op = AssignOp::MulAssign; break;
            case TokenKind::SlashAssign:   op = AssignOp::DivAssign; break;
            case TokenKind::PercentAssign: op = AssignOp::ModAssign; break;
            default: break;
        }

        // Recurse into parse_assignment to maintain right-associativity (e.g. a = b = c).
        auto value = parse_assignment();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        return std::make_unique<AssignExpr>(op, std::move(left), std::move(value), span);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parse_logical_or() {
    SourceLocation start_loc = current().span().start;
    auto left = parse_logical_and();
    if (!left) return nullptr;

    while (match(TokenKind::KwOr)) {
        auto right = parse_logical_and();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        left = std::make_unique<BinaryExpr>(BinaryOp::Or, std::move(left), std::move(right), span);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parse_logical_and() {
    SourceLocation start_loc = current().span().start;
    auto left = parse_equality();
    if (!left) return nullptr;

    while (match(TokenKind::KwAnd)) {
        auto right = parse_equality();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        left = std::make_unique<BinaryExpr>(BinaryOp::And, std::move(left), std::move(right), span);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parse_equality() {
    SourceLocation start_loc = current().span().start;
    auto left = parse_relational();
    if (!left) return nullptr;

    while (check(TokenKind::EqualEqual) || check(TokenKind::NotEqual) || check(TokenKind::KwIn)) {
        Token op_tok = advance();
        BinaryOp op = (op_tok.kind() == TokenKind::EqualEqual) ? BinaryOp::Equal
                     : (op_tok.kind() == TokenKind::KwIn)       ? BinaryOp::In
                                                               : BinaryOp::NotEqual;
        auto right = parse_relational();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right), span);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parse_relational() {
    SourceLocation start_loc = current().span().start;
    auto left = parse_additive();
    if (!left) return nullptr;

    while (check(TokenKind::Less) || check(TokenKind::LessEqual) ||
           check(TokenKind::Greater) || check(TokenKind::GreaterEqual)) {
        Token op_tok = advance();
        BinaryOp op = BinaryOp::Less;
        switch (op_tok.kind()) {
            case TokenKind::Less:         op = BinaryOp::Less; break;
            case TokenKind::LessEqual:    op = BinaryOp::LessEqual; break;
            case TokenKind::Greater:      op = BinaryOp::Greater; break;
            case TokenKind::GreaterEqual: op = BinaryOp::GreaterEqual; break;
            default: break;
        }
        auto right = parse_additive();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right), span);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parse_additive() {
    SourceLocation start_loc = current().span().start;
    auto left = parse_multiplicative();
    if (!left) return nullptr;

    while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
        Token op_tok = advance();
        BinaryOp op = (op_tok.kind() == TokenKind::Plus) ? BinaryOp::Add : BinaryOp::Sub;
        auto right = parse_multiplicative();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right), span);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parse_multiplicative() {
    SourceLocation start_loc = current().span().start;
    auto left = parse_unary();
    if (!left) return nullptr;

    while (check(TokenKind::Star) || check(TokenKind::Slash) || check(TokenKind::Percent)) {
        Token op_tok = advance();
        BinaryOp op = (op_tok.kind() == TokenKind::Star) ? BinaryOp::Mul
                     : (op_tok.kind() == TokenKind::Slash) ? BinaryOp::Div
                                                           : BinaryOp::Mod;
        auto right = parse_unary();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right), span);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parse_unary() {
    SourceLocation start_loc = current().span().start;

    if (match(TokenKind::Bang, TokenKind::KwNot)) {
        auto operand = parse_unary();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        return std::make_unique<UnaryExpr>(UnaryOp::Not, std::move(operand), span);
    }
    if (match(TokenKind::Minus)) {
        auto operand = parse_unary();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        return std::make_unique<UnaryExpr>(UnaryOp::Minus, std::move(operand), span);
    }
    if (match(TokenKind::Plus)) {
        auto operand = parse_unary();
        SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
        return std::make_unique<UnaryExpr>(UnaryOp::Plus, std::move(operand), span);
    }

    return parse_postfix();
}

std::unique_ptr<Expr> Parser::parse_postfix() {
    SourceLocation start_loc = current().span().start;
    auto expr = parse_primary();
    if (!expr) return nullptr;

    while (true) {
        if (check(TokenKind::LeftParen)) {
            advance();
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenKind::RightParen)) {
                do {
                    if (auto arg = parse_assignment()) {
                        args.push_back(std::move(arg));
                    } else break;
                } while (match(TokenKind::Comma));
            }
            consume(TokenKind::RightParen, "Expected ')' after argument list");
            SourceSpan span{start_loc, previous().span().end, lexer_.filename()};

            if (auto* id = dynamic_cast<IdentifierExpr*>(expr.get())) {
                expr = std::make_unique<CallExpr>(id->name(), std::move(args), span);
            } else {
                diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                             "Expression cannot be called as a function", span);
            }
        } else if (match(TokenKind::LeftBracket)) {
            std::unique_ptr<Expr> start_idx = nullptr;
            std::unique_ptr<Expr> end_idx = nullptr;
            bool is_slice = false;

            if (match(TokenKind::Colon)) {
                is_slice = true;
                if (!check(TokenKind::RightBracket)) {
                    end_idx = parse_logical_or();
                }
            } else {
                start_idx = parse_logical_or();
                if (match(TokenKind::Colon)) {
                    is_slice = true;
                    if (!check(TokenKind::RightBracket)) {
                        end_idx = parse_logical_or();
                    }
                }
            }
            consume(TokenKind::RightBracket, "Expected ']' at end of index/slice");
            SourceSpan span{start_loc, previous().span().end, lexer_.filename()};

            if (is_slice) {
                expr = std::make_unique<SliceExpr>(std::move(expr), std::move(start_idx), std::move(end_idx), span);
            } else {
                expr = std::make_unique<IndexExpr>(std::move(expr), std::move(start_idx), span);
            }
        } else if (match(TokenKind::Dot)) {
            if (!check(TokenKind::Identifier)) {
                diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                             "Expected method name after '.'", current().span());
                break;
            }
            std::string method_name = advance().lexeme();
            consume(TokenKind::LeftParen, "Expected '(' after method name");
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenKind::RightParen)) {
                do {
                    if (auto arg = parse_assignment()) {
                        args.push_back(std::move(arg));
                    } else break;
                } while (match(TokenKind::Comma));
            }
            consume(TokenKind::RightParen, "Expected ')' after method arguments");
            SourceSpan span{start_loc, previous().span().end, lexer_.filename()};
            expr = std::make_unique<MethodCallExpr>(std::move(expr), std::move(method_name), std::move(args), span);
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parse_primary() {
    if (match(TokenKind::IntLiteral)) {
        return std::make_unique<LiteralExpr>(TypeKind::Int, previous().lexeme(), previous().span());
    }
    if (match(TokenKind::FloatLiteral)) {
        return std::make_unique<LiteralExpr>(TypeKind::Float, previous().lexeme(), previous().span());
    }
    if (match(TokenKind::CharLiteral)) {
        return std::make_unique<LiteralExpr>(TypeKind::Char, previous().lexeme(), previous().span());
    }
    if (match(TokenKind::StringLiteral)) {
        return std::make_unique<LiteralExpr>(TypeKind::Str, previous().lexeme(), previous().span());
    }
    if (match(TokenKind::Identifier)) {
        return std::make_unique<IdentifierExpr>(previous().lexeme(), previous().span());
    }
    if (match(TokenKind::LeftParen)) {
        auto expr = parse_expression();
        consume(TokenKind::RightParen, "Expected ')' after grouping expression");
        return expr;
    }

    diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                 "Expected expression, got " + std::string(to_string(current().kind())),
                 current().span());
    return nullptr;
}

} // namespace execore
