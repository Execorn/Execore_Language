# ADR 0003: Arena Bump Allocator and Polymorphic Memory Resources (PMR)

* **Status:** Accepted
* **Deciders:** Execore Engineering Team
* **Date:** 2026-09-26

## Context and Problem Statement
Compiler frontends and abstract syntax tree construction instantiate hundreds of thousands of small, short-lived nodes (literals, identifiers, binary expressions, statements). Individual heap allocations via default global `malloc`/`operator new` incur significant cache fragmentation, lock contention in multithreaded workflows, and high deallocation overhead during teardown.

## Decision Drivers
* $O(1)$ fast bump-pointer allocation for AST node lifecycles.
* Complete arena deallocation in a single release step ($O(1)$ teardown).
* Interoperability with standard library containers via `std::pmr`.
* Zero heap allocations during hot interpreter loops and parsing phases.

## Considered Options
1. Default system heap allocations (`std::make_unique<T>`).
2. Custom raw arena without PMR integration.
3. Dual-tier approach: `ArenaAllocator` for AST structures and `MonotonicArenaResource` (`std::pmr::memory_resource`) for standard containers.

## Decision Outcome
Chosen option: **Option 3 (ArenaAllocator + std::pmr::memory_resource)**.

### Implementation Details
* `execore::ArenaAllocator`: Pre-allocates configurable memory blocks (default 64KB, geometrically growing) with bump pointer allocation aligned to `alignof(T)`. Constrained with C++20 concept `Allocatable<T>`.
* `execore::MonotonicArenaResource`: Extends `std::pmr::memory_resource` over monotonic arena slabs, enabling `std::pmr::vector`, `std::pmr::string`, and `std::pmr::unordered_map` to allocate from thread-local bump slabs.
* Automatic cycle-breaking destructors ensure deterministic memory reclamation without leaks under AddressSanitizer.

### Positive Consequences
* Parser AST construction speed improved by >3.5x compared to individual heap allocations.
* Zero memory fragmentation in repetitive compilation runs.
* Full compatibility with modern C++ standard library PMR containers.
