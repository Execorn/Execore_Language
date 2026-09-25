#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace execore {

struct SourceLocation {
    uint32_t line{1};
    uint32_t column{1};
    uint32_t offset{0};

    constexpr bool operator==(const SourceLocation& other) const noexcept {
        return line == other.line && column == other.column && offset == other.offset;
    }
};

struct SourceSpan {
    SourceLocation start{};
    SourceLocation end{};
    std::string_view filename{};

    [[nodiscard]] constexpr bool is_valid() const noexcept {
        return start.line > 0 && end.line >= start.line;
    }

    [[nodiscard]] std::string to_string() const;
};

} // namespace execore
