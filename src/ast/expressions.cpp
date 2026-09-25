#include "execore/ast/expressions.hpp"

namespace execore {

std::string_view to_string(UnaryOp op) noexcept {
    switch (op) {
        case UnaryOp::Not:   return "not";
        case UnaryOp::Minus: return "-";
        case UnaryOp::Plus:  return "+";
    }
    return "?";
}

std::string_view to_string(BinaryOp op) noexcept {
    switch (op) {
        case BinaryOp::Add:          return "+";
        case BinaryOp::Sub:          return "-";
        case BinaryOp::Mul:          return "*";
        case BinaryOp::Div:          return "/";
        case BinaryOp::Mod:          return "%";
        case BinaryOp::And:          return "and";
        case BinaryOp::Or:           return "or";
        case BinaryOp::Less:         return "<";
        case BinaryOp::LessEqual:    return "<=";
        case BinaryOp::Greater:      return ">";
        case BinaryOp::GreaterEqual: return ">=";
        case BinaryOp::Equal:        return "==";
        case BinaryOp::NotEqual:     return "!=";
        case BinaryOp::In:           return "in";
    }
    return "?";
}

std::string_view to_string(AssignOp op) noexcept {
    switch (op) {
        case AssignOp::Assign:        return "=";
        case AssignOp::AddAssign:     return "+=";
        case AssignOp::SubAssign:     return "-=";
        case AssignOp::MulAssign:     return "*=";
        case AssignOp::DivAssign:     return "/=";
        case AssignOp::ModAssign:     return "%=";
    }
    return "?";
}

} // namespace execore
