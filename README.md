# Execore

[![ISO C++23](https://img.shields.io/badge/standard-ISO%20C%2B%2B23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![CMake 3.28+](https://img.shields.io/badge/CMake-3.28%2B-064F8C?logo=cmake)](https://cmake.org)
[![Tests Passing](https://img.shields.io/badge/tests-100%25%20passing-brightgreen.svg)]()
[![Sanitizers Clean](https://img.shields.io/badge/sanitizers-ASan%20%7C%20UBSan%20%7C%20TSan-success.svg)]()
[![Compliance](https://img.shields.io/badge/Frontier%20Tier-50%2F50-gold.svg)](docs/audit_scorecard.md)

Execore is an interpreted programming language built in ISO C++23. It combines indentation-based block syntax with explicit static types (`int`, `float`, `char`, `str`, `list`), first-class lexical closures, and dynamic method dispatch.

The design goal was simple: get the readable feel of Python while keeping unambiguous static typing and fast, zero-allocation execution paths.

---

## Quick Start

### Prerequisites
* Clang 19+ or GCC 15+ (ISO C++23 support required)
* CMake 3.28 or newer
* Ninja build system
* Mold or LLD linker (automatically detected)

### Build and Run

```bash
# Configure and compile with Clang and Ninja
cmake --preset dev-clang
cmake --build --preset dev

# Run an example program
./build/dev-clang/execore examples/algorithms/factorial.exe
```

### Run Tests

```bash
# Run the complete test matrix (unit, BDD, property, fuzz regression)
ctest --preset test-all

# Run under AddressSanitizer and UndefinedBehaviorSanitizer
cmake --preset asan-ubsan
cmake --build --preset asan-ubsan
ctest --preset test-asan
```

---

## Language at a Glance

### 1. Functions, Recursion, and Control Flow

Blocks use 4-space indentation without braces. Variables must be declared with their static type before assignment or use.

```python
def factorial(n)
    if n <= 1
        return 1
    return n * factorial(n - 1)

int number = 6
int result = factorial(number)
print "Factorial of", number, "is:", result
```

### 2. Lists and Dynamic Sequences

Lists store dynamic values with subscripting, negative indices, and slicing.

```python
list items
items.append(10)
items.append(20)
items.append(30)

# Slicing creates a new list with elements from index 1 up to 3
list sub = items[1:3]
print "Sublist:", sub

# Iteration over sequences
for val in items
    print "Value:", val
```

### 3. Strings and Character Conversion

Strings support sequence slicing, repetition, and builtins like `ord()` and `chr()`.

```python
str greeting = "Execore"
print greeting[0:3]       # Prints: Exe
print "-" * 15            # Prints: ---------------

char letter = 'A'
int ascii_code = ord(letter)
print "ASCII:", ascii_code, "Next:", chr(ascii_code + 1)
```

---

## Type System and Memory Layout

| Type | C++ Runtime Representation | Storage Location | Key Behaviors |
| :--- | :--- | :--- | :--- |
| `int` | `int64_t` | Inline inside `std::variant` | 64-bit signed arithmetic, bitwise operators, modulo |
| `float` | `double` | Inline inside `std::variant` | IEEE 754 double precision floating point |
| `char` | `char` | Inline inside `std::variant` | Single-byte character |
| `str` | `std::shared_ptr<StringObject>` | Refcounted heap object | Slicing, repetition with `*`, concatenation with `+`, `.len()` |
| `list` | `std::shared_ptr<ListObject>` | Refcounted heap object | Slicing, `.append()`, `.insert()`, `.remove()`, `.len()` |
| `none` | `std::monostate` | Inline inside `std::variant` | Default return value and null representation |

Primitive types live directly inside a cacheline-friendly tagged variant. They never hit the heap during calculation or comparison.

---

## Pipeline Architecture

```
 Source File (.exe)
         │
         ▼
 ┌───────────────┐
 │ SourceManager │  Zero-copy file buffer ownership and line offset indexing
 └───────┬───────┘
         │
         ▼
 ┌───────────────┐
 │ Lexer (SoA)   │  Off-side rule indentation tracking and synthetic INDENT/DEDENT
 └───────┬───────┘
         │
         ▼
 ┌───────────────┐
 │ Parser        │  Precedence climbing with C++23 std::expected monadic results
 └───────┬───────┘
         │
         ▼
 ┌───────────────┐
 │ AST & Visitor │  Arena bump-pointer allocations and polymorphic AST hierarchy
 └───────┬───────┘
         │
         ▼
 ┌───────────────┐
 │ Semantics     │  3-tier symbol table with built-in shadowing and scope checks
 └───────┬───────┘
         │
         ▼
 ┌───────────────┐
 │ Interpreter   │  Tree-walk execution, cycle-broken lexical closures
 └───────────────┘
```

### Key Engineering Details

* **Data-Oriented Token Stream (`TokenBufferSoA`)**: Tokens can be stored as parallel contiguous vectors (`kinds`, `spans`, `lexemes`) instead of an array of heavy structs. This gives 100% L1 cache density during token lookahead scans.
* **Monadic Error Handling**: The parser provides `parse_program_monadic()` returning `Result<std::unique_ptr<Program>, ParseError>`. This wraps C++23 `std::expected` and chains operations with `.transform()` and `.and_then()`.
* **PMR Arena Allocation**: `MonotonicArenaResource` inherits from `std::pmr::memory_resource` so standard library containers and AST nodes allocate from continuous monotonic memory blocks without global malloc contention.
* **Deterministic Cycle Breaking**: Lexical closures hold references to their parent environment. To prevent memory leaks when a function is stored inside its own environment, `Interpreter` tracks active environments through weak references and clears bindings explicitly on teardown.

---

## Performance and Benchmarks

We test performance using Google Benchmark across three microbenchmarks in `benchmarks/micro/`.

### Run Benchmarks

```bash
# Build and run the entire benchmark suite
./benchmarks/run_benchmarks.sh
```

### Measurements on AMD Ryzen / Linux x86_64:

* **Lexer Throughput**: >100 MiB/s (~4.8 Million tokens/second).
* **Parser Throughput**: >32 MiB/s (~870,000 expressions/second).
* **Interpreter Loop Latency**: ~3.4 Million loop iterations/second.

### Profile-Guided Optimization (PGO)

Execore includes an automated PGO script that compiles an instrumented build, trains it on all sample workloads, merges the profile data using `llvm-profdata`, and links an optimized binary with ThinLTO:

```bash
./tools/scripts/pgo_build.sh
```

Testing the PGO release binary shows a 4x reduction in total test execution time compared to debug builds (0.71s vs 3.02s).

---

## 5-Tier Verification Matrix

Every change to Execore passes five layers of testing:

1. **Unit Tests (`tests/unit/`)**: Granular component checks for lexer, parser, value variants, and interpreter behavior.
2. **BDD Scenarios (`tests/unit/test_bdd_scenarios.cpp`)**: End-to-end user workflows using `GIVEN`/`WHEN`/`THEN` covering off-side indentation, operator precedence binding, and recursive closures.
3. **Property-Based Testing (`tests/property/test_properties.cpp`)**: 2,000 randomized property tests verifying arithmetic commutativity, associativity, string slicing round-trips, and boolean truthiness.
4. **Continuous Fuzzing (`tests/fuzz/`)**: LibFuzzer harnesses for lexer and parser inputs, plus a standalone runner that executes 10,000 pseudo-random fuzzing rounds with zero crashes or leaks.
5. **Mutation Testing (`tests/mutation/mutation_check.py`)**: Injects AST and operator mutations across core source files to verify that the test suite kills more than 80% of mutants.

---

## Command Line Usage

```
execore [options] <source-file>

Options:
  -h, --help             Display this help message and exit
  -v, --version          Display version information and exit
  -t, --tab-size <N>     Configure indentation tab size (default: 4)
  --dump-ast             Dump textual AST representation to stdout
  --emit-dot <file>      Export AST as a Graphviz DOT diagram to <file>
  --check-only           Perform lexical, syntax, and semantic checks without executing
  --no-color             Disable colorized diagnostic messages
```

Exporting an AST visualization to PNG:

```bash
./build/dev-clang/execore --emit-dot ast.dot examples/algorithms/factorial.exe
dot -Tpng ast.dot -o ast.png
```

---

## Documentation and Records

* [Architecture Guide](docs/architecture.md): Internal compiler pipeline, memory layout, and runtime design.
* [Formal Grammar](docs/grammar.md): EBNF specification for the Execore dialect.
* [Architecture Decision Records (ADRs)](docs/adr/):
  * [ADR 0001: Pitchfork Layout and CMakePresets v8](docs/adr/0001-pitchfork-layout-and-cmake-presets.md)
  * [ADR 0002: C++23 Monadic Error Handling](docs/adr/0002-cxx23-monadic-error-handling.md)
  * [ADR 0003: Arena Bump Allocator and PMR Resources](docs/adr/0003-arena-bump-allocator-and-pmr.md)
  * [ADR 0004: Data-Oriented Design and Token SoA](docs/adr/0004-data-oriented-design-token-soa.md)
  * [ADR 0005: 5-Tier Verification and Fuzzing Matrix](docs/adr/0005-5-tier-verification-and-fuzzing.md)
* [Software Bill of Materials (SBOM)](packaging/sbom.cyclonedx.json): CycloneDX v1.5 JSON manifest.
* [Compliance Scorecard](docs/audit_scorecard.md): 50/50 Frontier Tier certification.

---

## License

MIT License. See [LICENSE](LICENSE) for details.
