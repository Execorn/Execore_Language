# Execore

A custom C compiler pipeline and runtime execution engine written in C99. The codebase spans 7,021 lines of C across 36 source and header files, implementing lexical analysis with indentation tracking, recursive descent parsing over an EBNF grammar, Graphviz AST visualization, and a stack-based tree-walk evaluator.

## Architectural Pipeline

```
 Source File (.exe)
         │
         ▼
 ┌───────────────┐
 │ Library Loader│  UploadLibrary (memory buffer, line & char coordinates)
 └───────┬───────┘
         │
         ▼
 ┌───────────────┐
 │ Tokenizer     │  reader.c (42 token types, INDENT / DEDENT synthesis)
 └───────┬───────┘
         │
         ▼
 ┌───────────────┐
 │ Parser        │  execore_parser.c (Recursive descent over EBNF grammar)
 └───────┬───────┘
         │
         ▼
 ┌───────────────┐
 │ AST & Scopes  │  abstract_syntax_tree.c, id.c (26 node types, nested symbol tables)
 └───────┬───────┘
         ├──────────────────────────────┐
         ▼                              ▼
 ┌───────────────┐              ┌───────────────┐
 │ Tree Evaluator│              │ Graphviz Dump │  DumpNode() -> DOT format
 │ processor.c   │              │ (Flags -D4/8) │
 └───────┬───────┘              └───────────────┘
         │
         ▼
 ┌───────────────┐
 │ VM & Object   │  stack.c, target.c (Tagged union, reference counting, vtables)
 └───────────────┘
```

The execution flow consists of five concrete stages:

1. **Source Loading (`library.c`, `library.h`)**: `UploadLibrary` maps the input file into an in-memory buffer, tracking line numbers (`cur_line_number`), character offsets, and a pushback buffer (`PushChar`, `PeekChar`, `GetChar`).
2. **Lexical Analysis (`reader.c`, `reader.h`)**: The lexer matches 42 distinct token types. Block structure relies on Python-style significant whitespace. An indentation stack measures leading spaces per line and emits synthetic `INDENT` and `DEDENT` tokens based on the tab size parameter (`-t`, default 4 spaces).
3. **Recursive Descent Parser (`parser/execore_parser.c`, `ast/abstract_syntax_tree.c`)**: Constructs an Abstract Syntax Tree consisting of 26 node variants. Each node contains function pointers for validation (`valid_check`), compilation/execution (`compile`), and debugging dumps (`dump`).
4. **Scope Resolution (`id.c`, `id.h`)**: Lexical environments are structured as a tree of symbol tables (`add_child(bool is_nested)`, `remove_child()`). Identifiers resolve from the innermost local frame up to global scope.
5. **Runtime Engine (`processor.c`, `stack.c`, `types/target.c`)**: `CompileNode` evaluates AST nodes recursively using an explicit evaluation stack (`StackConstructor`, `StackPush`, `StackPop`). The object model (`Target`) uses tagged unions and reference counting (`DecreaseUsages`, `CopyTarget`, `AssignTarget`) with vtable function pointers for arithmetic operations and type conversions.

## Supported Types & Object Model

The runtime supports six primary data types defined in `types/target.h`:

| Type | Underlying C Representation | Memory Management | Capabilities |
|---|---|---|---|
| `int` | `int64_t` (`int_t`) | Inline in union | 64-bit integer arithmetic, bitwise operations |
| `float` | `double` (`float_t`) | Inline in union | IEEE 754 double-precision floating point |
| `char` | `char` | Inline in union | Single byte ASCII character |
| `str` | `char*` buffer + length | Heap allocated, refcounted | String slicing `[start:end]`, concatenation, length |
| `list` | `NodeList` hybrid structure | Heap allocated, refcounted | Append, insert, remove, sublist slicing, indexing |
| `nan` | Null target sentinel | Static allocation | Represents uninitialized or None targets |

Each `Target` struct includes a pointer to a `target_type` descriptor containing type-specific function pointers: `init`, `adv_init`, `method`, `to_string`, `is_true`, and arithmetic callbacks (`add`, `sub`, `mul`, `div`, `mod`).

## Grammar Specification (EBNF)

The formal language grammar implemented by `execore_parser.c`:

```ebnf
program ::= (statement | newline)* EOF

statement ::= declaration_stmnt 
            | import_stmt 
            | print_stmnt 
            | input_stmnt 
            | return_stmnt 
            | if_stmnt 
            | while_stmnt 
            | do_stmnt 
            | for_stmnt 
            | break_stmnt 
            | continue_stmnt 
            | pass_stmnt 
            | expression_stmnt

declaration_stmnt ::= variable_declaration | function_declaration

variable_declaration ::= var_type identifier ( '=' assignment_expr )? 
                         ( ',' identifier ( '=' assignment_expr )? )* newline

var_type ::= 'char' | 'int' | 'float' | 'str' | 'list'

function_declaration ::= 'def' identifier '(' (identifier ( ',' identifier )* )? ')' block

identifier ::= [a-zA-Z] ( [a-zA-Z0-9_] )*

block ::= newline INDENT statement+ DEDENT

import_stmt ::= 'import' assignment_expr ( ',' assignment_expr )* newline

print_stmnt ::= 'print' '-raw'? ( assignment_expr ( ',' assignment_expr )* )? newline

input_stmnt ::= 'input' string? identifier ( ',' string? identifier )* newline

return_stmnt ::= 'return' expression? newline

if_stmnt ::= 'if' expression block ( 'else' block )?

while_stmnt ::= 'while' expression block

do_stmnt ::= 'do' block 'while' expression newline

for_stmnt ::= 'for' identifier 'in' sequence

expression ::= assignment_expr ( ',' expression )*

assignment_expr ::= logical_or_expr ( ( '=' | '+=' | '-=' | '*=' | '/=' | '%=' ) assignment_expr )*

logical_or_expr ::= logical_and_expr ( 'or' logical_or_expr )*

logical_and_expr ::= equality_expr ( 'and' logical_and_expr )*

equality_expr ::= relational_expr ( ( '==' | '!=' | '<>' | 'in' ) equality_expr )*

relational_expr ::= additive_expr ( ( '<' | '>' | '<=' | '>=' ) relational_expr )*

additive_expr ::= mult_expr ( ( '+' | '-' ) additive_expr )*

mult_expr ::= unary_expr ( ( '*' | '/' | '%' ) mult_expr )*

unary_expr ::= ( '+' | '-' | '!' )? primary_expr

primary_expr ::= function_call | variable | constant | '(' expression ')'

function_call ::= identifier '(' (assignment_expr ( ',' assignment_expr )* )? ')'

sequence ::= ( string_variable | list_variable ) ( '[' slice ']' )?

slice ::= logical_or_expr? ':' logical_or_expr?
```

## Error Recovery & Diagnostics

`handle_err.c` defines 10 typed error codes:

```c
#define IDENTIFIER_ERROR          1
#define TYPE_ERROR                2
#define SYNTAX_ERROR              3
#define VALUE_ERROR               4
#define SYSTEM_ERROR              5
#define INDEX_ERROR               6
#define MEM_ERROR                 7
#define OPERATOR_WITH_WRONG_TYPE  8
#define ZERO_DIVISION_ERROR       9
#define LANG_ERROR               10
```

When an error triggers via `RaiseError(int number, ...)`, the diagnostic engine inspects the active AST node or reader buffer:
1. Prints the source filename and line number to `stderr`.
2. Locates the exact line offset in the mapped file buffer and echoes the offending source text.
3. Formats the error classification string and optional variadic context arguments.
4. Terminates process execution with the exact error code for shell inspection.

## Build and Execution on Arch Linux

### Prerequisites

```bash
sudo pacman -S gcc make graphviz
```

### Compilation

Source files reference internal headers via `#include "src/..."`. Compiling directly requires mapping the current directory into an include directory:

```bash
# Setup include link and compile
mkdir -p /tmp/execore_inc && ln -sf $(pwd) /tmp/execore_inc/src

gcc -std=gnu99 -fpermissive -Wno-incompatible-pointer-types \
    -I/tmp/execore_inc -I. \
    -O2 -o execore \
    main.c array.c function.c handle_err.c id.c library.c number.c \
    processor.c reader.c stack.c ast/abstract_syntax_tree.c \
    parser/execore_parser.c types/execore_string.c types/node_list.c \
    types/null_target.c types/target.c -lm
```

### CLI Flags

```
execore [options] [source file]

Options:
  -D<filter>   Debug bitmask filter (default: 8)
                 0: disable debugger
                 1: lexeme token stream trace
                 2: memory allocations and reference counts
                 4: generate Graphviz DOT dump and abort
                 8: generate Graphviz DOT dump and continue execution
                16: dump active variables to console
                32: dump active variables to file
  -t<size>     Configure indentation tab size in spaces (default: 4)
  -h           Show command line options
  -v           Show language version
```

### Code Example

```python
def factorial(n)
    if n <= 1
        return 1
    return n * factorial(n - 1)

int result = factorial(6)
print "Result is:", result
```

Run without debug noise:

```bash
./execore -D0 sample.exe
```

Generate an AST graph:

```bash
./execore -D4 sample.exe
dot -Tpng tree_dump.dot -o ast.png
```

## Engineering Trade-offs

- **Recursive Descent vs Table-Driven Parser**: A hand-written recursive descent parser gives direct control over custom error messages and source coordinate extraction. But grammar changes require manual refactoring across parser functions instead of modifying a Yacc or Bison grammar file.
- **AST Tree-Walk vs Bytecode VM**: The runtime walks AST nodes directly with a stack helper rather than emitting flat bytecode instructions. This simplifies debugging and graph generation at the expense of dispatch cache locality.
- **Tagged Unions vs Polymorphic Pointers**: Allocating `Target` values with tagged unions allows direct stack-allocated value arithmetic for integers and floats. Heap allocations occur only when manipulating dynamically sized types like strings and lists.
