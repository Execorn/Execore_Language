#pragma once

#include "execore/common/source_location.hpp"
#include <string>
#include <string_view>
#include <cstdint>

namespace execore {

enum class TokenKind : uint8_t {
    // End of file and formatting
    Eof,
    Newline,
    Indent,
    Dedent,

    // Literals
    IntLiteral,
    FloatLiteral,
    CharLiteral,
    StringLiteral,
    Identifier,

    // Type keywords
    KwChar,
    KwInt,
    KwFloat,
    KwStr,
    KwList,

    // Control flow keywords
    KwDef,
    KwIf,
    KwElse,
    KwWhile,
    KwDo,
    KwFor,
    KwIn,
    KwReturn,
    KwPass,
    KwBreak,
    KwContinue,
    KwPrint,
    KwInput,
    KwImport,

    // Logical operator keywords
    KwAnd,
    KwOr,
    KwNot,

    // Arithmetic operators
    Plus,
    Minus,
    Star,
    Slash,
    Percent,

    // Assignment & Compound assignment
    Assign,
    PlusAssign,
    MinusAssign,
    StarAssign,
    SlashAssign,
    PercentAssign,

    // Relational operators
    EqualEqual,
    NotEqual,       // != and <>
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    // Unary/Logical
    Bang,           // !

    // Punctuation & Delimiters
    Comma,
    Colon,
    Dot,
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,

    // Error / Unknown
    Unknown
};

[[nodiscard]] std::string_view to_string(TokenKind kind) noexcept;

class Token {
public:
    Token() = default;
    Token(TokenKind kind, std::string_view lexeme, SourceSpan span)
        : kind_(kind), lexeme_(lexeme), span_(std::move(span)) {}

    [[nodiscard]] TokenKind kind() const noexcept { return kind_; }
    [[nodiscard]] const std::string& lexeme() const noexcept { return lexeme_; }
    [[nodiscard]] const SourceSpan& span() const noexcept { return span_; }

    [[nodiscard]] bool is(TokenKind kind) const noexcept { return kind_ == kind; }
    [[nodiscard]] bool is_not(TokenKind kind) const noexcept { return kind_ != kind; }
    [[nodiscard]] bool is_one_of(TokenKind k1, TokenKind k2) const noexcept {
        return kind_ == k1 || kind_ == k2;
    }
    template <typename... Rest>
    [[nodiscard]] bool is_one_of(TokenKind k1, TokenKind k2, Rest... rest) const noexcept {
        return kind_ == k1 || is_one_of(k2, rest...);
    }

    [[nodiscard]] bool is_literal() const noexcept {
        return kind_ == TokenKind::IntLiteral || kind_ == TokenKind::FloatLiteral ||
               kind_ == TokenKind::CharLiteral || kind_ == TokenKind::StringLiteral;
    }

    [[nodiscard]] bool is_type_keyword() const noexcept {
        return kind_ == TokenKind::KwChar || kind_ == TokenKind::KwInt ||
               kind_ == TokenKind::KwFloat || kind_ == TokenKind::KwStr ||
               kind_ == TokenKind::KwList;
    }

    [[nodiscard]] bool is_assignment_op() const noexcept {
        return kind_ == TokenKind::Assign || kind_ == TokenKind::PlusAssign ||
               kind_ == TokenKind::MinusAssign || kind_ == TokenKind::StarAssign ||
               kind_ == TokenKind::SlashAssign || kind_ == TokenKind::PercentAssign;
    }

private:
    TokenKind kind_{TokenKind::Unknown};
    std::string lexeme_{};
    SourceSpan span_{};
};

} // namespace execore
