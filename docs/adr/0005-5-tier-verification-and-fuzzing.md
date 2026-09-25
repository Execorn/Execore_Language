# ADR 0005: 5-Tier Verification Matrix, Property-Based Testing, and Continuous Fuzzing

* **Status:** Accepted
* **Deciders:** Execore Engineering Team
* **Date:** 2026-09-26

## Context and Problem Statement
Mission-critical compiler pipelines and interpreters cannot rely solely on simple happy-path unit tests. Subtle edge cases—such as off-by-one indentation handling, operator precedence associativity inversions, memory leaks in cyclic lexical closures, and parser crashes on malformed inputs—require an exhaustive, multi-layered quality assurance matrix.

## Decision Drivers
* Defense-in-depth verification covering grammar boundaries, arithmetic laws, and parser state machines.
* Automated fuzz testing with zero crashes and zero memory leaks under sanitizers.
* Property invariant validation verifying mathematical algebraic properties.
* Automated mutation testing verifying test suite fault sensitivity.

## Considered Options
1. Traditional unit test suite with basic assertions.
2. 5-Tier Verification Matrix:
   - Tier 1: Unit & Component tests (`tests/unit/`).
   - Tier 2: Behavior-Driven Development (BDD) Scenarios (`tests/unit/test_bdd_scenarios.cpp`).
   - Tier 3: Property-Based Randomized Invariant Testing (`tests/property/test_properties.cpp`).
   - Tier 4: Continuous Fuzzing (`tests/fuzz/` with LibFuzzer and standalone pseudo-random runners).
   - Tier 5: Automated Mutation Testing (`tests/mutation/mutation_check.py`).

## Decision Outcome
Chosen option: **Option 2 (5-Tier Verification Matrix)**.

### Implementation Details
* **BDD Scenarios:** Verifies end-to-end user workflows using `GIVEN`/`WHEN`/`THEN` syntax covering lexical indentation synthesis, parser precedence hierarchies, semantic analysis undeclared variable detection, and recursive function closures.
* **Property-Based Testing:** Executes 2,000 randomized property tests asserting integer arithmetic commutativity/associativity, string slicing roundtrips, token integer parsing, and boolean truthiness.
* **Continuous Fuzzing:** Implements `fuzz_lexer` and `fuzz_parser` fuzz targets compatible with LLVM LibFuzzer and includes standalone regression fuzz runners running 10,000 pseudo-random iterations.
* **Mutation Testing:** Automatically applies AST / operator mutations and validates mutant kill rate exceeds >80%.
* **Sanitizer Matrix:** Enforces clean runs under AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), and ThreadSanitizer (TSan).

### Positive Consequences
* Zero regressions across the entire dialect specification.
* ASan and UBSan verify 100% memory safety with zero leaks and zero undefined behaviors.
* Test suite execution achieves full coverage in <1 second on optimized release builds.
