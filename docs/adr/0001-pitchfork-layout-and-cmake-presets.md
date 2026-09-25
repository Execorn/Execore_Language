# ADR 0001: Pitchfork Layout and CMakePresets v8 Migration

* **Status:** Accepted
* **Deciders:** Execore Engineering Team
* **Date:** 2026-09-26

## Context and Problem Statement
The legacy codebase maintained a flat C-style root directory mixing headers, sources, build scripts, and test files with in-source build residue. This violated modern compilation practices, complicated hermetic reproducible packaging, and made compiler toolchain switching cumbersome across developer workstations, containers, and CI environments.

## Decision Drivers
* Standardized directory hierarchy adhering to the Pitchfork standard.
* Cross-platform reproducible builds across Clang 19+, GCC 15+, and different operating systems.
* Single-command configuration presets for development, sanitizers, fuzzing, and benchmarking.
* Prevention of corrupting in-source build invocations.

## Considered Options
1. Retain flat directory with manual flags in bash scripts.
2. Pitchfork layout (`include/`, `src/`, `tests/`, `tools/`, `benchmarks/`, `docs/`, `packaging/`) with modern `CMakePresets.json` v8.
3. Meson or Bazel migration.

## Decision Outcome
Chosen option: **Option 2 (Pitchfork Layout with CMakePresets v8)**.

### Positive Consequences
* Clear separation of public API headers (`include/execore/`) and private implementations (`src/`).
* In-source builds are strictly forbidden at the CMake level (`CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR` raises fatal error).
* Standardized presets (`dev-clang`, `dev-gcc`, `release-pgo`, `asan-ubsan`, `tsan`, `msan`, `fuzz`, `bench`) eliminate inconsistent manual flag configurations.
* Fast linkers (`mold`, `ld.lld`) and compiler caches (`sccache`, `ccache`) are automatically discovered and bound globally.

### Negative Consequences
* Required updating all relative `#include` statements to project-relative paths (`execore/...`).
