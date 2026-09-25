# ADR 0002: C++23 Monadic Error Handling via std::expected

* **Status:** Accepted
* **Deciders:** Execore Engineering Team
* **Date:** 2026-09-26

## Context and Problem Statement
Compiler frontends and parsers require expressive, zero-overhead error propagation. The initial implementation relied on raw integer return codes or exceptions for syntax errors, which led to high unwinding overhead during fuzzing and reduced code locality. Earlier attempts with `std::variant<T, E>` required verbose `std::holds_alternative` checks and lacked functional chaining primitives.

## Decision Drivers
* Zero-overhead deterministic error reporting without stack unwinding overhead.
* Railway-oriented functional composition (`and_then`, `transform`, `or_else`).
* Rich domain diagnostic payload support with source location spans.
* Clean ISO C++23 standardization alignment.

## Considered Options
1. Traditional C++ exceptions (`throw ParseError`).
2. Error codes (`int` return codes with output parameters).
3. `Result<T, E>` wrapping C++23 `std::expected<T, E>` with monadic operations.

## Decision Outcome
Chosen option: **Option 3 (C++23 `std::expected` monadic wrapper)**.

### Implementation Details
Implemented `execore::Result<T, E>` inheriting or wrapping `std::expected<T, E>` with:
* `.and_then(F&& func)`: Chains operations returning another `Result`.
* `.transform(F&& func)`: Maps the contained success value to another type.
* `.transform_error(F&& func)`: Translates error codes or diagnostics.
* `.or_else(F&& func)`: Provides fallback error recovery paths.
* Monadic type trait constraints via concepts (`DomainErrorConcept<E>`).

### Positive Consequences
* Lexical and syntactic error pipelines can chain consecutive AST productions cleanly without nested conditional checks.
* Fuzzing throughput increased drastically due to absence of exception unwinding frames in hot paths.
* Compiler warnings and diagnostics are preserved with exact line/column spans.
