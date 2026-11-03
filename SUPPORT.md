# Support & Troubleshooting

## Documentation

Consult the repository documentation for in-depth information about the language design, formal grammar, and internal compiler pipeline:

- [Documentation Index](docs/index.md): Subsystem overviews and quick start guide.
- [Architecture Guide](docs/architecture.md): Frontend passes, memory allocations (`std::pmr`), and runtime environments.
- [Formal Grammar (EBNF)](docs/grammar.md): Operator precedence, block indentation grammar, and token lexical rules.

## Diagnostic & Troubleshooting Steps

1. **Verify syntax and semantics without executing**:
   ```bash
   execore --check-only script.exe
   ```
   Validates indentation structure, token sequences, and lexical bindings, outputting colorized compiler diagnostics if syntax or semantic errors occur.

2. **Inspect the generated Abstract Syntax Tree (AST)**:
   ```bash
   # Dump textual AST representation to stdout
   execore --dump-ast script.exe

   # Export Graphviz DOT diagram and render to PNG
   execore --emit-dot ast.dot script.exe
   dot -Tpng ast.dot -o ast.png
   ```

3. **Adjust indentation tab sizes**:
   If source files mix tabs and spaces or use non-standard indentation widths (e.g. 2 or 8 spaces):
   ```bash
   execore -t 2 script.exe
   execore --tab-size 8 script.exe
   ```

4. **Diagnose memory corruption or undefined behavior**:
   Compile with AddressSanitizer and UndefinedBehaviorSanitizer:
   ```bash
   cmake --preset asan-ubsan
   cmake --build --preset asan-ubsan
   ./build/asan-ubsan/execore script.exe
   ```

## Getting Help

- **Questions and Discussions**: Open a topic in [GitHub Discussions](https://github.com/Execorn/Execore_Language/discussions).
- **Bug Reports**: Open an issue using the [GitHub Issue Tracker](https://github.com/Execorn/Execore_Language/issues/new) with a minimal reproducing `.exe` script and the expected vs actual interpreter output.
