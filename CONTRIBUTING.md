# Contributing to Execore

Contributions, language grammar enhancements, runtime optimizations, and bug reports are welcome.

## Prerequisites

Execore is written in ISO C++23 and requires a modern toolchain:
- **Compiler**: Clang 19+ or GCC 15+ with full C++23 support (`std::expected`, `std::pmr`, `std::span`, monadic operations)
- **Build System**: CMake 3.28+ and Ninja
- **Linker**: Mold or LLD (automatically detected by `cmake/Linker.cmake`)
- **Optional**: `sccache` or `ccache` for accelerated compilation

## Development Workflow

Execore manages configurations via standardized CMake Presets:

1. **Configure and compile development debug build**:
   ```bash
   cmake --preset dev-clang
   cmake --build --preset dev
   ```

2. **Execute the complete test suite**:
   ```bash
   ctest --preset test-all
   ```

3. **Verify under Sanitizers (ASan + UBSan + LSan)**:
   ```bash
   cmake --preset asan-ubsan
   cmake --build --preset asan-ubsan
   ctest --preset test-asan
   ```

4. **Verify under ThreadSanitizer (TSan)**:
   ```bash
   cmake --preset tsan
   cmake --build --preset tsan
   ctest --preset test-tsan
   ```

5. **Release Build with ThinLTO and Hardening**:
   ```bash
   cmake --preset release-pgo
   cmake --build --preset release-pgo
   ctest --test-dir build/release-pgo --output-on-failure
   ```

## Architecture and Code Guidelines

- **Pitchfork Layout**: Public headers reside under `include/execore/`, internal translation units under `src/`, executable tools under `tools/`, and test harnesses under `tests/`.
- **Memory Management**: Hot AST nodes and values utilize polymorphic bump allocation (`std::pmr` and `MonotonicArenaResource`). Avoid unmanaged raw heap allocations on execution paths.
- **Error Handling**: Use monadic `execore::Result<T, E>` backed by `std::expected` for syntax and analysis pipelines.
- **Indentation Handling**: Indentation logic belongs to `Lexer` and `TokenBufferSoA`. Ensure token streams retain exact source locations (`SourceLocation` and `SourceSpan`) for diagnostic reporting.
- **Code Formatting**: Format all touched C++ files using `clang-format` before staging:
  ```bash
  clang-format -i include/execore/**/*.hpp src/**/*.cpp tests/**/*.cpp tools/**/*.cpp
  ```
- **Commit Messages**: Follow Conventional Commits format (`feat:`, `fix:`, `perf:`, `docs:`, `build:`, `ci:`, `test:`, `refactor:`) in imperative mood.
