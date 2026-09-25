# ADR 0004: Data-Oriented Design (DoD) via Token Structure of Arrays (SoA)

* **Status:** Accepted
* **Deciders:** Execore Engineering Team
* **Date:** 2026-09-26

## Context and Problem Statement
In traditional compiler frontends, tokens are represented as an Array of Structures (AoS): `std::vector<Token>`. Each `Token` struct contains `TokenKind` (1 byte), padding (7 bytes), `SourceLocation` (16 bytes), and string lexeme / value (32 bytes), totalling 56 to 64 bytes per token. During parser lookahead or token classification scans, only the `TokenKind` field is read, causing cache pollution and low hardware cacheline utilization (retrieving 64 bytes to inspect 1 byte).

## Decision Drivers
* Maximize L1 Data cacheline density during parser lookahead and token sequence scanning.
* Enable SIMD / vectorized batch classification of tokens (e.g. finding keywords or delimiters).
* Support zero-copy contiguous views via `std::span`.

## Considered Options
1. Retain Array of Structures (`std::vector<Token>`).
2. Structure of Arrays (`TokenBufferSoA`) storing contiguous parallel vectors for kinds, offsets, lengths, lines, and columns.
3. Hybrid Chunked AoSoA.

## Decision Outcome
Chosen option: **Option 2 (Token Structure of Arrays - `TokenBufferSoA`)**.

### Implementation Details
* `execore::TokenBufferSoA` decomposes token streams into distinct contiguous arrays:
  - `std::vector<TokenKind> kinds_`: 1 byte per token. A single 64-byte L1 cacheline holds 64 token kinds!
  - `std::vector<uint32_t> offsets_`: Byte offset into source text.
  - `std::vector<uint32_t> lengths_`: Length of token lexeme.
  - `std::vector<uint32_t> lines_`: Source line numbers.
  - `std::vector<uint32_t> columns_`: Source column numbers.
* Provides `std::span<const TokenKind>` contiguous views for vectorized linear search.
* Bidirectional bridge to classic `Token` via `at(size_t index)` and `push_back(const Token&)`.

### Positive Consequences
* Token classification and lookahead loops exhibit 100% cacheline utilization.
* Lexer throughput exceeds 100 MiB/s across synthetic and real-world source files.
* Direct integration with `std::span` allows bounds-checked zero-copy vector slices.
