#include "execore/frontend/lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace execore {

namespace {
    const std::unordered_map<std::string_view, TokenKind> KEYWORDS = {
        {"and",      TokenKind::KwAnd},
        {"break",    TokenKind::KwBreak},
        {"char",     TokenKind::KwChar},
        {"continue", TokenKind::KwContinue},
        {"def",      TokenKind::KwDef},
        {"do",       TokenKind::KwDo},
        {"else",     TokenKind::KwElse},
        {"float",    TokenKind::KwFloat},
        {"for",      TokenKind::KwFor},
        {"if",       TokenKind::KwIf},
        {"import",   TokenKind::KwImport},
        {"in",       TokenKind::KwIn},
        {"input",    TokenKind::KwInput},
        {"int",      TokenKind::KwInt},
        {"list",     TokenKind::KwList},
        {"not",      TokenKind::KwNot},
        {"or",       TokenKind::KwOr},
        {"pass",     TokenKind::KwPass},
        {"print",    TokenKind::KwPrint},
        {"return",   TokenKind::KwReturn},
        {"str",      TokenKind::KwStr},
        {"while",    TokenKind::KwWhile}
    };
}

Lexer::Lexer(std::string_view source, std::string filename,
             DiagnosticEngine& diag, uint32_t tab_size)
    : source_(source), filename_(std::move(filename)), diag_(diag), tab_size_(tab_size) {}

char Lexer::peek(size_t offset) const noexcept {
    if (cursor_ + offset >= source_.size()) return '\0';
    return source_[cursor_ + offset];
}

char Lexer::advance() noexcept {
    if (is_at_end()) return '\0';
    char c = source_[cursor_++];
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return c;
}

bool Lexer::match(char expected) noexcept {
    if (is_at_end() || source_[cursor_] != expected) return false;
    advance();
    return true;
}

bool Lexer::is_at_end() const noexcept {
    return cursor_ >= source_.size();
}

SourceLocation Lexer::current_location() const noexcept {
    return SourceLocation{line_, column_, cursor_};
}

SourceSpan Lexer::make_span(const SourceLocation& start) const noexcept {
    return SourceSpan{start, current_location(), filename_};
}

void Lexer::skip_comment() {
    while (!is_at_end() && peek() != '\n') {
        advance();
    }
}

void Lexer::process_indentation() {
    // Execore implements Python-style indentation synthesis. Blank lines and pure comment lines
    // do not trigger INDENT or DEDENT tokens so that trailing or intermediate whitespace is ignored.
    while (at_line_start_) {
        SourceLocation loc = current_location();
        uint32_t indent_col = 0;

        while (!is_at_end()) {
            if (peek() == ' ') {
                advance();
                ++indent_col;
            } else if (peek() == '\t') {
                advance();
                indent_col += tab_size_;
            } else {
                break;
            }
        }

        if (peek() == '#') {
            skip_comment();
        }

        if (peek() == '\r') {
            advance();
        }

        if (peek() == '\n') {
            advance();
            continue;
        }

        if (is_at_end()) {
            while (indent_stack_.size() > 1) {
                indent_stack_.pop_back();
                pending_tokens_.emplace_back(TokenKind::Dedent, "", make_span(loc));
            }
            at_line_start_ = false;
            return;
        }

        at_line_start_ = false;

        uint32_t current_indent = indent_stack_.back();
        if (indent_col > current_indent) {
            indent_stack_.push_back(indent_col);
            pending_tokens_.emplace_back(TokenKind::Indent, "", make_span(loc));
        } else if (indent_col < current_indent) {
            while (indent_stack_.size() > 1 && indent_stack_.back() > indent_col) {
                indent_stack_.pop_back();
                pending_tokens_.emplace_back(TokenKind::Dedent, "", make_span(loc));
            }
            if (indent_stack_.back() != indent_col) {
                diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                             "Inconsistent indentation level", make_span(loc));
            }
        }
    }
}

Token Lexer::next_token() {
    if (!pending_tokens_.empty()) {
        Token tok = std::move(pending_tokens_.front());
        pending_tokens_.pop_front();
        return tok;
    }

    if (at_line_start_) {
        process_indentation();
        if (!pending_tokens_.empty()) {
            Token tok = std::move(pending_tokens_.front());
            pending_tokens_.pop_front();
            return tok;
        }
    }

    return scan_token();
}

const Token& Lexer::peek_token() {
    if (pending_tokens_.empty()) {
        pending_tokens_.push_back(next_token());
    }
    return pending_tokens_.front();
}

Token Lexer::scan_token() {
    while (!is_at_end()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '#') {
            skip_comment();
        } else {
            break;
        }
    }

    SourceLocation start_loc = current_location();

    if (is_at_end()) {
        if (indent_stack_.size() > 1) {
            indent_stack_.pop_back();
            return Token(TokenKind::Dedent, "", make_span(start_loc));
        }
        return Token(TokenKind::Eof, "", make_span(start_loc));
    }

    char c = advance();

    if (c == '\n') {
        at_line_start_ = true;
        return Token(TokenKind::Newline, "\n", make_span(start_loc));
    }

    // Rewind cursor so dedicated scanners can consume the initial character directly.
    if (std::isdigit(static_cast<unsigned char>(c))) {
        --cursor_;
        --column_;
        return scan_number();
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        --cursor_;
        --column_;
        return scan_identifier_or_keyword();
    }

    if (c == '"') {
        return scan_string();
    }

    if (c == '\'') {
        return scan_char();
    }

    switch (c) {
        case '(': return Token(TokenKind::LeftParen, "(", make_span(start_loc));
        case ')': return Token(TokenKind::RightParen, ")", make_span(start_loc));
        case '[': return Token(TokenKind::LeftBracket, "[", make_span(start_loc));
        case ']': return Token(TokenKind::RightBracket, "]", make_span(start_loc));
        case ',': return Token(TokenKind::Comma, ",", make_span(start_loc));
        case ':': return Token(TokenKind::Colon, ":", make_span(start_loc));
        case '.': return Token(TokenKind::Dot, ".", make_span(start_loc));

        case '+':
            if (match('=')) return Token(TokenKind::PlusAssign, "+=", make_span(start_loc));
            return Token(TokenKind::Plus, "+", make_span(start_loc));

        case '-':
            if (match('=')) return Token(TokenKind::MinusAssign, "-=", make_span(start_loc));
            return Token(TokenKind::Minus, "-", make_span(start_loc));

        case '*':
            if (match('=')) return Token(TokenKind::StarAssign, "*=", make_span(start_loc));
            return Token(TokenKind::Star, "*", make_span(start_loc));

        case '/':
            if (match('=')) return Token(TokenKind::SlashAssign, "/=", make_span(start_loc));
            return Token(TokenKind::Slash, "/", make_span(start_loc));

        case '%':
            if (match('=')) return Token(TokenKind::PercentAssign, "%=", make_span(start_loc));
            return Token(TokenKind::Percent, "%", make_span(start_loc));

        case '!':
            if (match('=')) return Token(TokenKind::NotEqual, "!=", make_span(start_loc));
            return Token(TokenKind::Bang, "!", make_span(start_loc));

        case '=':
            if (match('=')) return Token(TokenKind::EqualEqual, "==", make_span(start_loc));
            return Token(TokenKind::Assign, "=", make_span(start_loc));

        case '<':
            if (match('=')) return Token(TokenKind::LessEqual, "<=", make_span(start_loc));
            if (match('>')) return Token(TokenKind::NotEqual, "<>", make_span(start_loc));
            return Token(TokenKind::Less, "<", make_span(start_loc));

        case '>':
            if (match('=')) return Token(TokenKind::GreaterEqual, ">=", make_span(start_loc));
            return Token(TokenKind::Greater, ">", make_span(start_loc));

        default:
            diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                         std::string("Unexpected character: '") + c + "'", make_span(start_loc));
            return Token(TokenKind::Unknown, std::string(1, c), make_span(start_loc));
    }
}

Token Lexer::scan_number() {
    SourceLocation start_loc = current_location();
    size_t start_pos = cursor_;
    bool is_float = false;

    while (!is_at_end() && std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1)))) {
        is_float = true;
        advance();
        while (!is_at_end() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    if (peek() == 'e' || peek() == 'E') {
        is_float = true;
        advance();
        if (peek() == '+' || peek() == '-') {
            advance();
        }
        if (!std::isdigit(static_cast<unsigned char>(peek()))) {
            diag_.report(DiagnosticLevel::Error, ErrorCategory::Value,
                         "Missing exponent digits in floating-point literal", make_span(start_loc));
        }
        while (!is_at_end() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    std::string text(source_.substr(start_pos, cursor_ - start_pos));
    TokenKind kind = is_float ? TokenKind::FloatLiteral : TokenKind::IntLiteral;
    return Token(kind, std::move(text), make_span(start_loc));
}

Token Lexer::scan_identifier_or_keyword() {
    SourceLocation start_loc = current_location();
    size_t start_pos = cursor_;

    while (!is_at_end() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }

    std::string text(source_.substr(start_pos, cursor_ - start_pos));
    auto it = KEYWORDS.find(text);
    TokenKind kind = (it != KEYWORDS.end()) ? it->second : TokenKind::Identifier;

    return Token(kind, std::move(text), make_span(start_loc));
}

Token Lexer::scan_string() {
    SourceLocation start_loc = current_location();
    std::string value;

    while (!is_at_end() && peek() != '"') {
        if (peek() == '\n') {
            diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                         "Unterminated string literal", make_span(start_loc));
            return Token(TokenKind::StringLiteral, std::move(value), make_span(start_loc));
        }

        char c = advance();
        if (c == '\\') {
            if (is_at_end()) break;
            char esc = advance();
            switch (esc) {
                case 'n':  value.push_back('\n'); break;
                case 't':  value.push_back('\t'); break;
                case 'r':  value.push_back('\r'); break;
                case '0':  value.push_back('\0'); break;
                case 'a':  value.push_back('\a'); break;
                case 'b':  value.push_back('\b'); break;
                case 'f':  value.push_back('\f'); break;
                case 'v':  value.push_back('\v'); break;
                case '\\': value.push_back('\\'); break;
                case '"':  value.push_back('"'); break;
                case '\'': value.push_back('\''); break;
                default:
                    value.push_back('\\');
                    value.push_back(esc);
                    break;
            }
        } else {
            value.push_back(c);
        }
    }

    if (is_at_end()) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                     "Unterminated string literal at end of file", make_span(start_loc));
    } else {
        advance(); // consume closing '"'
    }

    return Token(TokenKind::StringLiteral, std::move(value), make_span(start_loc));
}

Token Lexer::scan_char() {
    SourceLocation start_loc = current_location();
    std::string value;

    if (is_at_end() || peek() == '\'') {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                     "Empty character literal", make_span(start_loc));
        if (match('\'')) {}
        return Token(TokenKind::CharLiteral, "", make_span(start_loc));
    }

    char c = advance();
    if (c == '\\') {
        char esc = advance();
        switch (esc) {
            case 'n':  value.push_back('\n'); break;
            case 't':  value.push_back('\t'); break;
            case 'r':  value.push_back('\r'); break;
            case '0':  value.push_back('\0'); break;
            case 'a':  value.push_back('\a'); break;
            case 'b':  value.push_back('\b'); break;
            case 'f':  value.push_back('\f'); break;
            case 'v':  value.push_back('\v'); break;
            case '\\': value.push_back('\\'); break;
            case '\'': value.push_back('\''); break;
            case '"':  value.push_back('"'); break;
            default:   value.push_back(esc); break;
        }
    } else {
        value.push_back(c);
    }

    if (!match('\'')) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                     "Multi-character or unterminated character literal", make_span(start_loc));
        while (!is_at_end() && peek() != '\'' && peek() != '\n') {
            advance();
        }
        match('\'');
    }

    return Token(TokenKind::CharLiteral, std::move(value), make_span(start_loc));
}

} // namespace execore
