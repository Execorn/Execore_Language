#pragma once

#include <concepts>
#include <type_traits>
#include <string_view>
#include <string>
#include <memory>
#include <expected>

namespace execore {

// Forward declarations
class ASTVisitor;
class ASTNode;
struct SourceSpan;

/**
 * @brief Concept constraining types that carry source location information.
 */
template <typename T>
concept SourceSpanned = requires(const T& obj) {
    { obj.span() } -> std::convertible_to<SourceSpan>;
};

/**
 * @brief Concept constraining AST nodes that accept visitor traversal.
 */
template <typename T>
concept ASTVisitable = requires(T& node, ASTVisitor& visitor) {
    { node.accept(visitor) } -> std::same_as<void>;
};

/**
 * @brief Concept constraining types suitable for allocation inside arena/bump pools.
 */
template <typename T>
concept Allocatable = (!std::is_array_v<T>) && (!std::is_reference_v<T>);

/**
 * @brief Concept constraining numeric runtime types (integers and floats).
 */
template <typename T>
concept NumericValue = std::integral<T> || std::floating_point<T>;

/**
 * @brief Concept constraining types printable to standard diagnostics and streams.
 */
template <typename T>
concept Printable = requires(const T& obj) {
    { obj.to_string() } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for error categories and domain errors.
 */
template <typename E>
concept DomainErrorConcept = requires(const E& err) {
    { to_string(err) } -> std::convertible_to<std::string_view>;
};

} // namespace execore
