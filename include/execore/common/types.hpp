#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace execore {

using IntType = int64_t;
using FloatType = double;
using CharType = char;

enum class TypeKind : uint8_t {
    None,
    Int,
    Float,
    Char,
    Str,
    List,
    Function,
    Custom
};

constexpr std::string_view to_string(TypeKind kind) noexcept {
    switch (kind) {
        case TypeKind::None:     return "none";
        case TypeKind::Int:      return "int";
        case TypeKind::Float:    return "float";
        case TypeKind::Char:     return "char";
        case TypeKind::Str:      return "str";
        case TypeKind::List:     return "list";
        case TypeKind::Function: return "function";
        case TypeKind::Custom:   return "custom";
    }
    return "unknown";
}

} // namespace execore
