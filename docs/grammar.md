# Execore Formal Grammar Specification (EBNF)

This document specifies the concrete syntax of Execore in Extended Backus-Naur Form (ISO/IEC 14977 dialect).

Execore uses an off-side rule similar to Python for statement grouping. Indentation changes emit synthetic `INDENT` and `DEDENT` tokens. Blank lines and lines with only comments are ignored during indentation tracking.

---

## 1. Program and Top-Level Structure

```ebnf
program ::= ( statement | newline )* EOF

statement ::= var_decl_stmt
            | func_decl_stmt
            | if_stmt
            | while_stmt
            | do_while_stmt
            | for_stmt
            | return_stmt
            | break_stmt
            | continue_stmt
            | pass_stmt
            | print_stmt
            | input_stmt
            | import_stmt
            | expr_stmt
```

---

## 2. Declarations and Blocks

```ebnf
var_decl_stmt ::= type_keyword identifier ( '=' assignment_expr )?
                  ( ',' identifier ( '=' assignment_expr )? )* newline

type_keyword ::= 'char' | 'int' | 'float' | 'str' | 'list'

func_decl_stmt ::= 'def' identifier '(' parameter_list? ')' block

parameter_list ::= identifier ( ',' identifier )*

block ::= newline* INDENT statement+ DEDENT
```

---

## 3. Control Flow and I/O Statements

```ebnf
if_stmt ::= 'if' expression block ( 'else' block )?

while_stmt ::= 'while' expression block

do_while_stmt ::= 'do' block 'while' expression newline

for_stmt ::= 'for' identifier 'in' expression block

return_stmt ::= 'return' expression? newline

break_stmt ::= 'break' newline

continue_stmt ::= 'continue' newline

pass_stmt ::= 'pass' newline

print_stmt ::= 'print' '-raw'? ( assignment_expr ( ',' assignment_expr )* )? newline

input_stmt ::= 'input' ( string_literal? identifier ) ( ',' string_literal? identifier )* newline

import_stmt ::= 'import' assignment_expr ( ',' assignment_expr )* newline

expr_stmt ::= expression newline
```

---

## 4. Expressions and Operator Precedence

Precedence runs from lowest (comma operator) to highest (primary expressions and postfix accessors).

| Precedence | Operator / Construct | Associativity | Description |
| :--- | :--- | :--- | :--- |
| 1 (lowest) | `,` | Left | Comma sequence evaluation |
| 2 | `=`, `+=`, `-=`, `*=`, `/=`, `%=` | Right | Assignment and compound assignment |
| 3 | `or` | Left | Short-circuit logical OR |
| 4 | `and` | Left | Short-circuit logical AND |
| 5 | `==`, `!=`, `<>`, `in` | Left | Equality, inequality, membership |
| 6 | `<`, `<=`, `>`, `>=` | Left | Relational ordering |
| 7 | `+`, `-` | Left | Addition and subtraction |
| 8 | `*`, `/`, `%` | Left | Multiplication, division, modulo |
| 9 | `+`, `-`, `!`, `not` | Right | Unary signs and logical negation |
| 10 (highest) | `(...)`, `[...]`, `[start:end]`, `.<id>(...)` | Left | Calls, indexing, slicing, method calls |

```ebnf
expression ::= assignment_expr ( ',' assignment_expr )*

assignment_expr ::= logical_or_expr ( assignment_op assignment_expr )?

assignment_op ::= '=' | '+=' | '-=' | '*=' | '/=' | '%='

logical_or_expr ::= logical_and_expr ( 'or' logical_and_expr )*

logical_and_expr ::= equality_expr ( 'and' equality_expr )*

equality_expr ::= relational_expr ( equality_op relational_expr )*

equality_op ::= '==' | '!=' | '<>' | 'in'

relational_expr ::= additive_expr ( relational_op additive_expr )*

relational_op ::= '<' | '<=' | '>' | '>='

additive_expr ::= multiplicative_expr ( ( '+' | '-' ) multiplicative_expr )*

multiplicative_expr ::= unary_expr ( ( '*' | '/' | '%' ) unary_expr )*

unary_expr ::= unary_op unary_expr
             | postfix_expr

unary_op ::= '+' | '-' | '!' | 'not'

postfix_expr ::= primary_expr ( call_suffix | index_suffix | slice_suffix | method_suffix )*

call_suffix ::= '(' argument_list? ')'

argument_list ::= assignment_expr ( ',' assignment_expr )*

index_suffix ::= '[' logical_or_expr ']'

slice_suffix ::= '[' logical_or_expr? ':' logical_or_expr? ']'

method_suffix ::= '.' identifier '(' argument_list? ')'

primary_expr ::= identifier
               | int_literal
               | float_literal
               | char_literal
               | string_literal
               | '(' expression ')'
```

---

## 5. Lexical Elements

```ebnf
identifier     ::= [a-zA-Z_] [a-zA-Z0-9_]*
int_literal    ::= [0-9]+
float_literal  ::= [0-9]+ '.' [0-9]+ ( [eE] [+-]? [0-9]+ )?
                 | [0-9]+ [eE] [+-]? [0-9]+
char_literal   ::= "'" ( [^'\\\n] | escape_sequence ) "'"
string_literal ::= '"' ( [^"\\\n] | escape_sequence )* '"'

escape_sequence ::= '\\' ( 'n' | 't' | 'r' | '0' | 'a' | 'b' | 'f' | 'v' | '\\' | '"' | "'" )

newline        ::= '\r'? '\n'
comment        ::= '#' [^\r\n]*
```
