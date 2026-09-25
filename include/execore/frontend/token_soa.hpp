#pragma once

#include "execore/frontend/token.hpp"
#include <vector>
#include <string_view>
#include <span>
#include <cstdint>
#include <cstddef>

namespace execore {

/**
 * @brief Structure-of-Arrays (SoA) Token Buffer for high-performance vectorized lexing and parsing.
 *
 * Decomposes array-of-structures (AoS) Tokens into contiguous parallel arrays:
 * - kinds_: 1-byte elements, optimal for branch prediction and SIMD keyword scanning
 * - lexemes_: string_view pointers and lengths
 * - spans_: byte-offset source coordinates
 */
class TokenBufferSoA {
public:
    TokenBufferSoA() = default;
    explicit TokenBufferSoA(size_t reserve_capacity);

    void push_back(TokenKind kind, std::string_view lexeme, SourceSpan span);
    void push_back(const Token& token);

    [[nodiscard]] size_t size() const noexcept { return kinds_.size(); }
    [[nodiscard]] bool empty() const noexcept { return kinds_.empty(); }
    void clear() noexcept;
    void reserve(size_t capacity);

    [[nodiscard]] Token get(size_t index) const noexcept;
    [[nodiscard]] Token operator[](size_t index) const noexcept { return get(index); }

    [[nodiscard]] TokenKind kind(size_t index) const noexcept { return kinds_[index]; }
    [[nodiscard]] std::string_view lexeme(size_t index) const noexcept { return lexemes_[index]; }
    [[nodiscard]] const SourceSpan& span(size_t index) const noexcept { return spans_[index]; }

    // Direct contiguous slice access for SIMD / algorithm projections
    [[nodiscard]] std::span<const TokenKind> kinds() const noexcept { return kinds_; }
    [[nodiscard]] std::span<const std::string_view> lexemes() const noexcept { return lexemes_; }
    [[nodiscard]] std::span<const SourceSpan> spans() const noexcept { return spans_; }

    [[nodiscard]] size_t find_next(TokenKind target, size_t start_index = 0) const noexcept;
    [[nodiscard]] size_t count_tokens(TokenKind target) const noexcept;

private:
    std::vector<TokenKind> kinds_;
    std::vector<std::string_view> lexemes_;
    std::vector<SourceSpan> spans_;
};

} // namespace execore
