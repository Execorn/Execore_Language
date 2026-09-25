#pragma once

#include <expected>
#include <concepts>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <functional>

namespace execore {

/**
 * @brief Frontier C++23 Monadic Result wrapper around std::expected<T, E>.
 *
 * Enforces zero out-parameters and explicit value-or-error monadic chaining:
 * - and_then()
 * - transform()
 * - transform_error()
 * - or_else()
 */
template <typename T, typename E>
requires (!std::is_reference_v<T> && !std::is_reference_v<E>)
class Result {
public:
    using ValueType = T;
    using ErrorType = E;
    using UnderlyingType = std::expected<T, E>;

    // Constructors
    Result(const T& val) : expected_(val) {}
    Result(T&& val) : expected_(std::move(val)) {}
    Result(const E& err) : expected_(std::unexpected(err)) {}
    Result(E&& err) : expected_(std::unexpected(std::move(err))) {}
    Result(std::unexpected<E> unexp) : expected_(std::move(unexp)) {}
    Result(std::expected<T, E> exp) : expected_(std::move(exp)) {}

    [[nodiscard]] constexpr bool has_value() const noexcept { return expected_.has_value(); }
    [[nodiscard]] constexpr bool is_ok() const noexcept { return expected_.has_value(); }
    [[nodiscard]] constexpr bool is_err() const noexcept { return !expected_.has_value(); }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return expected_.has_value(); }

    [[nodiscard]] constexpr T& value() & { return expected_.value(); }
    [[nodiscard]] constexpr const T& value() const & { return expected_.value(); }
    [[nodiscard]] constexpr T&& value() && { return std::move(expected_).value(); }

    [[nodiscard]] constexpr const E& error() const & noexcept { return expected_.error(); }
    [[nodiscard]] constexpr E& error() & noexcept { return expected_.error(); }
    [[nodiscard]] constexpr E&& error() && noexcept { return std::move(expected_).error(); }

    [[nodiscard]] constexpr T& operator*() & noexcept { return *expected_; }
    [[nodiscard]] constexpr const T& operator*() const & noexcept { return *expected_; }
    [[nodiscard]] constexpr T&& operator*() && noexcept { return *std::move(expected_); }

    [[nodiscard]] constexpr T* operator->() noexcept { return &(*expected_); }
    [[nodiscard]] constexpr const T* operator->() const noexcept { return &(*expected_); }

    template <typename U>
    requires std::convertible_to<U, T>
    [[nodiscard]] constexpr T value_or(U&& default_value) const & {
        return expected_.value_or(std::forward<U>(default_value));
    }

    template <typename U>
    requires std::convertible_to<U, T>
    [[nodiscard]] constexpr T value_or(U&& default_value) && {
        return std::move(expected_).value_or(std::forward<U>(default_value));
    }

    // Monadic: and_then (F returns Result or std::expected)
    template <typename F>
    requires std::invocable<F, const T&>
    [[nodiscard]] constexpr auto and_then(F&& f) const & {
        if (!has_value()) {
            using RetType = std::invoke_result_t<F, const T&>;
            return RetType(std::unexpected(expected_.error()));
        }
        return std::invoke(std::forward<F>(f), expected_.value());
    }

    template <typename F>
    requires std::invocable<F, T&&>
    [[nodiscard]] constexpr auto and_then(F&& f) && {
        if (!has_value()) {
            using RetType = std::invoke_result_t<F, T&&>;
            return RetType(std::unexpected(std::move(expected_).error()));
        }
        return std::invoke(std::forward<F>(f), std::move(expected_).value());
    }

    // Monadic: transform (F returns non-Result value)
    template <typename F>
    requires std::invocable<F, const T&>
    [[nodiscard]] constexpr auto transform(F&& f) const & {
        using ReturnVal = std::invoke_result_t<F, const T&>;
        if (has_value()) {
            return Result<ReturnVal, E>(std::invoke(std::forward<F>(f), expected_.value()));
        }
        return Result<ReturnVal, E>(std::unexpected(expected_.error()));
    }

    template <typename F>
    requires std::invocable<F, T&&>
    [[nodiscard]] constexpr auto transform(F&& f) && {
        using ReturnVal = std::invoke_result_t<F, T&&>;
        if (has_value()) {
            return Result<ReturnVal, E>(std::invoke(std::forward<F>(f), std::move(expected_).value()));
        }
        return Result<ReturnVal, E>(std::unexpected(std::move(expected_).error()));
    }

    // Monadic: transform_error
    template <typename F>
    requires std::invocable<F, const E&>
    [[nodiscard]] constexpr auto transform_error(F&& f) const & {
        using NewError = std::invoke_result_t<F, const E&>;
        if (!has_value()) {
            return Result<T, NewError>(std::unexpected(std::invoke(std::forward<F>(f), expected_.error())));
        }
        return Result<T, NewError>(expected_.value());
    }

    template <typename F>
    requires std::invocable<F, E&&>
    [[nodiscard]] constexpr auto transform_error(F&& f) && {
        using NewError = std::invoke_result_t<F, E&&>;
        if (!has_value()) {
            return Result<T, NewError>(std::unexpected(std::invoke(std::forward<F>(f), std::move(expected_).error())));
        }
        return Result<T, NewError>(std::move(expected_).value());
    }

    // Monadic: or_else
    template <typename F>
    requires std::invocable<F, const E&>
    [[nodiscard]] constexpr auto or_else(F&& f) const & {
        if (!has_value()) {
            return std::invoke(std::forward<F>(f), expected_.error());
        }
        return *this;
    }

    template <typename F>
    requires std::invocable<F, E&&>
    [[nodiscard]] constexpr auto or_else(F&& f) && {
        if (!has_value()) {
            return std::invoke(std::forward<F>(f), std::move(expected_).error());
        }
        return std::move(*this);
    }

    [[nodiscard]] constexpr const std::expected<T, E>& raw() const noexcept { return expected_; }
    [[nodiscard]] constexpr std::expected<T, E>& raw() noexcept { return expected_; }

private:
    std::expected<T, E> expected_;
};

// Void specialization for Result<void, E>
template <typename E>
requires (!std::is_reference_v<E>)
class Result<void, E> {
public:
    using ValueType = void;
    using ErrorType = E;

    Result() : expected_() {}
    Result(const E& err) : expected_(std::unexpected(err)) {}
    Result(E&& err) : expected_(std::unexpected(std::move(err))) {}
    Result(std::unexpected<E> unexp) : expected_(std::move(unexp)) {}

    [[nodiscard]] constexpr bool has_value() const noexcept { return expected_.has_value(); }
    [[nodiscard]] constexpr bool is_ok() const noexcept { return expected_.has_value(); }
    [[nodiscard]] constexpr bool is_err() const noexcept { return !expected_.has_value(); }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return expected_.has_value(); }

    constexpr void value() const { expected_.value(); }

    [[nodiscard]] constexpr const E& error() const & noexcept { return expected_.error(); }
    [[nodiscard]] constexpr E& error() & noexcept { return expected_.error(); }
    [[nodiscard]] constexpr E&& error() && noexcept { return std::move(expected_).error(); }

private:
    std::expected<void, E> expected_;
};

} // namespace execore
