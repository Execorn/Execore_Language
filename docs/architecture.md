# Execore Architecture & Design

Execore is an interpreted programming language compiler frontend and tree-walk runtime engineered in ISO C++23.

```
Source (.exe) ──> SourceManager ──> Lexer (SoA) ──> Parser (Monadic) ──> AST ──> SemanticAnalyzer ──> Interpreter
                        │                                                   │
                        ▼                                                   ▼
                 DiagnosticEngine                                      DotExporter (Graphviz)
```

---

## 1. Frontend Pipeline

### SourceManager (`include/execore/frontend/source_manager.hpp`)
* **Buffer Ownership & Slices**: Holds raw file buffers in contiguous memory, giving downstream passes string slices (`std::string_view`) with zero heap copies.
* **Line Indexing**: Pre-computes line break byte offsets during registration. Line and column coordinates resolve via binary search in $O(\log N)$ time for error diagnostics.

### Lexer & Token Engine (`include/execore/frontend/lexer.hpp`, `token_soa.hpp`)
* **Indentation Tracking (Off-Side Rule)**: Maintains an indentation depth stack (`std::vector<size_t>`). When indentation increases, the lexer synthesizes a `TokenKind::Indent` token. When indentation decreases, it pops matching levels and synthesizes one or more `TokenKind::Dedent` tokens.
* **Configurable Tab Stops**: Supports custom tab spacing via `-t / --tab-size <N>` (default: 4 spaces).
* **Data-Oriented Structure of Arrays (`TokenBufferSoA`)**: Tokens can be laid out as parallel contiguous vectors (`kinds`, `spans`, `lexemes`) rather than an array of individual structs. This packs 64 token kinds per standard 64-byte L1 cacheline, speeding up token lookahead and keyword scans.
* **String Decoding**: Escaped sequences (`\n`, `\t`, `\r`, `\0`, `\\`, `\"`, `\'`) decode into owned string buffers without fixed size ceilings.

### Recursive Descent Parser (`include/execore/frontend/parser.hpp`)
* **Monadic Error Results**: Exposes `parse_program_monadic()` returning `Result<std::unique_ptr<Program>, ParseError>`. This wraps C++23 `std::expected` and chains downstream semantic analysis or AST serialization through `.transform()` and `.and_then()`.
* **Precedence Climbing**: Resolves binary and unary expressions through an operator precedence hierarchy:
  1. Assignment (`=`, `+=`, `-=`, `*=`, `/=`, `%=`)
  2. Logical OR (`or`)
  3. Logical AND (`and`)
  4. Equality and Membership (`==`, `!=`, `<>`, `in`)
  5. Relational (`<`, `<=`, `>`, `>=`)
  6. Additive (`+`, `-`)
  7. Multiplicative (`*`, `/`, `%`)
  8. Unary prefix (`+`, `-`, `!`, `not`)
  9. Postfix (Function calls, Indexing `[]`, Slicing `[:]`, Method calls `.method()`)
  10. Primary (Literals, Identifiers, Grouped `()`)
* **Memory Ownership**: AST nodes use `std::unique_ptr`, ensuring clear ownership trees and clean teardown.

---

## 2. Abstract Syntax Tree & Semantic Analysis

### AST Hierarchy (`include/execore/ast/`)
* **Polymorphic Hierarchy**: Base classes `ASTNode`, `Expr`, and `Stmt` define standard virtual destructors.
* **Visitor Pattern**: Implemented through `ASTVisitor`. Concrete visitors implement the tree-walk interpreter, semantic validation, and Graphviz visualization without adding methods to node classes.
* **Graphviz DOT Exporter (`execore::DotExporter`)**: Generates Graphviz DOT graphs with distinct node shapes and color palettes for visual debugging (`--emit-dot`).

### Semantic Analysis (`include/execore/semantics/`)
* **3-Tier Symbol Table**: Scopes chain together in a hierarchy:
  ```
  builtin_scope_ (chr, ord, type, len)
        ▲
        │
  global_scope_  (user global variables and functions)
        ▲
        │
  local_scope_   (block / function variables)
  ```
  Placing builtins in an enclosing parent scope allows global variables to shadow builtins without redefinition errors.
* **Validation Passes**:
  * Variable declaration enforcement before use.
  * Local scope redeclaration checks.
  * Function parameter uniqueness and call-site arity checks.
  * Control flow verification ensuring `break` and `continue` only appear inside loops.

---

## 3. Runtime Engine & Memory Architecture

### Value Representation (`include/execore/runtime/value.hpp`)
* **Inline Primitives**: `Value` uses a tagged union (`std::variant`). Primitives (`int64_t`, `double`, `char`, `std::monostate`) live inline without dynamic heap allocations.
* **Reference-Counted Objects**: Dynamic sequences (`StringObject`, `ListObject`, `FunctionObject`) use `std::shared_ptr`.
* **Polymorphic Operators**: Numeric operations promote integers to floating point when mixed. Strings and lists support repetition with `*` and concatenation with `+`.

### PMR Monotonic Arena (`include/execore/common/pmr_resource.hpp`)
* `MonotonicArenaResource` inherits from `std::pmr::memory_resource`.
* Allocates memory in growing chunks (default 64 KB). Individual deallocations are no-ops. All chunk memory reclaims in a single call to `release()`.

### Environment & Lexical Closures (`include/execore/runtime/environment.hpp`, `interpreter.cpp`)
* **Scope Frames**: Environments chain parent pointers for lexical lookup.
* **First-Class Functions**: `FunctionObject` captures its declaration environment in `closure_`.
* **Cycle Breaking**: When a function is stored in its own lexical environment, a mutual reference cycle forms. `Interpreter` registers created environments using `std::weak_ptr` and calls `Environment::clear()` during destruction to break these cycles cleanly under AddressSanitizer and LeakSanitizer.
* **Cacheline Interference Protection**: `ThreadExecutionContext` aligns to `hardware_destructive_interference_size` (64 bytes) to avoid false sharing on multi-threaded runs.

### Control Signals
* Return statements, loop breaks, and continue statements propagate through small control signal structures (`ReturnSignal`, `BreakSignal`, `ContinueSignal`) caught by statement visitor loops.

---

## 4. Diagnostics Engine (`include/execore/diagnostics/`)
* **Source Coordinates**: Reports file, line, and column numbers with severity levels (`error`, `warning`, `note`).
* **Underlines and Carets**: Fetches line content from `SourceManager` and prints ANSI-colored underlines pointing at the exact token span.
