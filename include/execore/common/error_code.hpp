#pragma once

#include <cstdint>
#include <string_view>

namespace execore {

enum class ErrorCategory : uint8_t {
    Syntax = 1,
    Type,
    Identifier,
    Value,
    ZeroDivision,
    Index,
    System,
    Memory,
    Runtime
};

constexpr std::string_view to_string(ErrorCategory cat) noexcept {
    switch (cat) {
        case ErrorCategory::Syntax:       return "SyntaxError";
        case ErrorCategory::Type:         return "TypeError";
        case ErrorCategory::Identifier:   return "IdentifierError";
        case ErrorCategory::Value:        return "ValueError";
        case ErrorCategory::ZeroDivision: return "ZeroDivisionError";
        case ErrorCategory::Index:        return "IndexError";
        case ErrorCategory::System:       return "SystemError";
        case ErrorCategory::Memory:       return "MemoryError";
        case ErrorCategory::Runtime:      return "RuntimeError";
    }
    return "Error";
}

} // namespace execore
