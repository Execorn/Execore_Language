# Security Policy

## Supported Versions

Security updates apply to the latest stable release series:

| Version | Supported |
| :--- | :--- |
| `2.0.x` | Yes |
| `< 2.0` | No |

## Reporting a Vulnerability

Please do not report security vulnerabilities in public GitHub issues.

Use [GitHub Private Vulnerability Reporting](https://github.com/Execorn/Execore_Language/security/advisories/new) to submit reports privately. Include a minimal reproduction script (`.exe`), command-line invocation, and backtrace from an AddressSanitizer/UndefinedBehaviorSanitizer build (`cmake --preset asan-ubsan`).

You can expect an initial response within 72 hours.

## Security Architecture & Scope

Execore parses untrusted script inputs and interprets abstract syntax trees. The following defenses are integrated into the architecture:

1. **Bounds-Checked Buffer Processing**: `SourceManager` safely loads sources into contiguous buffers and enforces offset verification on all span and location calculations.
2. **Defensive Indentation & Token Parsing**: Lexical indentation stacks and Structure-of-Arrays token buffers validate indentation depth and prevent unbounded stack recursion during lexical analysis.
3. **Continuous Fuzz Testing**: LibFuzzer and standalone randomized harnesses continuously fuzz parser and lexer entrypoints (`tests/fuzz/`).
4. **Binary Hardening**: Release builds incorporate defense-in-depth flags from `cmake/Hardening.cmake`:
   - `-D_FORTIFY_SOURCE=3`: Runtime buffer bounds checking.
   - `-fstack-protector-strong`: Stack canary protection.
   - `-Wl,-z,relro -Wl,-z,now`: Full RELRO makes the GOT read-only.
   - `-Wl,-z,noexecstack`: Non-executable memory segments.
   - `-fcf-protection=full`: Hardware-enforced branch target identification.
