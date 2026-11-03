# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- GitHub Actions automated CI matrix workflow testing Clang, GCC, ASan/UBSan, TSan, and Release builds.
- Community health documentation: `CONTRIBUTING.md`, `SECURITY.md`, `CHANGELOG.md`, `CODE_OF_CONDUCT.md`, `SUPPORT.md`.
- `EXECORE_WARNINGS_AS_ERRORS` option in `cmake/CompilerWarnings.cmake` to eliminate CMake CLI configuration warnings.

### Fixed
- Fixed release mode test assertion stripping by adding `-UNDEBUG` compile options to all test targets.
- Fixed uncaught exception crash in `tools/execore/main.cpp` when parsing non-numeric `-t`/`--tab-size` CLI arguments.
- Fixed all absolute `file:///home/execorn/...` URIs across `docs/index.md` by converting them to repository-relative markdown paths.
- Synchronized `CMakePresets` schema version references to version 6 across documentation and ADRs.
- Fixed empty badge links `]()` in `README.md`.

## [2.0.0] - 2026-09-26

### Added
- ISO C++23 compiler frontend and tree-walk interpreter supporting off-side rule indentation.
- Static type annotations (`int`, `float`, `char`, `str`, `list`), first-class lexical closures, and dynamic method dispatch.
- Polymorphic Memory Resources (`std::pmr`) with `MonotonicArenaResource` for high-throughput zero-overhead bump allocations.
- Data-Oriented Design token streaming via `TokenBufferSoA` providing contiguous structure-of-arrays layout.
- Monadic error handling wrapper `execore::Result<T, E>` leveraging C++23 `std::expected`.
- 5-Tier verification matrix: unit tests, BDD scenarios, property-based tests, standalone fuzz regression harness, and LibFuzzer targets.
- Pitchfork layout migration (`include/`, `src/`, `tests/`, `tools/`, `benchmarks/`, `docs/`, `packaging/`).
- Standardized `CMakePresets.json` v6 with Clang, GCC, ASan/UBSan, TSan, MSan, Fuzz, and Benchmark profiles.
- Binary defense-in-depth hardening flags (`_FORTIFY_SOURCE=3`, `-fstack-protector-strong`, full RELRO, CET).
- Profile-Guided Optimization (PGO) pipeline with `llvm-profdata` and ThinLTO.
