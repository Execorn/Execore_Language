#pragma once

#include "execore/frontend/token.hpp"
#include "execore/frontend/source_manager.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <deque>

namespace execore {

class Lexer {
public:
    Lexer(std::string_view source, std::string filename,
          DiagnosticEngine& diag, uint32_t tab_size = 4);

    Token next_token();
    const Token& peek_token();

    [[nodiscard]] const std::string& filename() const noexcept { return filename_; }
    [[nodiscard]] uint32_t tab_size() const noexcept { return tab_size_; }

private:
    char peek(size_t offset = 0) const noexcept;
    char advance() noexcept;
    bool match(char expected) noexcept;
    bool is_at_end() const noexcept;

    void skip_comment();
    Token scan_token();
    void process_indentation();

    Token scan_number();
    Token scan_identifier_or_keyword();
    Token scan_string();
    Token scan_char();

    SourceLocation current_location() const noexcept;
    SourceSpan make_span(const SourceLocation& start) const noexcept;

    std::string_view source_;
    std::string filename_;
    DiagnosticEngine& diag_;
    uint32_t tab_size_{4};

    uint32_t cursor_{0};
    uint32_t line_{1};
    uint32_t column_{1};

    bool at_line_start_{true};
    std::vector<uint32_t> indent_stack_{0};
    std::deque<Token> pending_tokens_;
};

} // namespace execore
