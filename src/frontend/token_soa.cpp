#include "execore/frontend/token_soa.hpp"
#include <algorithm>

namespace execore {

TokenBufferSoA::TokenBufferSoA(size_t reserve_capacity) {
    reserve(reserve_capacity);
}

void TokenBufferSoA::push_back(TokenKind kind, std::string_view lexeme, SourceSpan span) {
    kinds_.push_back(kind);
    lexemes_.push_back(lexeme);
    spans_.push_back(span);
}

void TokenBufferSoA::push_back(const Token& token) {
    push_back(token.kind(), token.lexeme(), token.span());
}

void TokenBufferSoA::clear() noexcept {
    kinds_.clear();
    lexemes_.clear();
    spans_.clear();
}

void TokenBufferSoA::reserve(size_t capacity) {
    kinds_.reserve(capacity);
    lexemes_.reserve(capacity);
    spans_.reserve(capacity);
}

Token TokenBufferSoA::get(size_t index) const noexcept {
    return Token(kinds_[index], lexemes_[index], spans_[index]);
}

size_t TokenBufferSoA::find_next(TokenKind target, size_t start_index) const noexcept {
    if (start_index >= kinds_.size()) return kinds_.size();
    auto it = std::find(kinds_.begin() + static_cast<std::ptrdiff_t>(start_index), kinds_.end(), target);
    return static_cast<size_t>(std::distance(kinds_.begin(), it));
}

size_t TokenBufferSoA::count_tokens(TokenKind target) const noexcept {
    return static_cast<size_t>(std::count(kinds_.begin(), kinds_.end(), target));
}

} // namespace execore
