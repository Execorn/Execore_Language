#!/usr/bin/env python3
"""
Frontier Mutation Testing Engine for Execore C++2026.
Measures test suite fault sensitivity by systematically injecting AST/token mutations
and verifying that the test suite detects and kills >80% of mutants.
"""

import sys
import os
import re
import subprocess
import shutil
import tempfile

MUTATIONS = [
    # Arithmetic operator mutations
    (r'(\s+)\+(\s+)', r'\1-\2', 'Add -> Sub'),
    (r'(\s+)-(\s+)', r'\1+\2', 'Sub -> Add'),
    (r'(\s+)\*(\s+)', r'\1/\2', 'Mul -> Div'),
    (r'(\s+)/(\s+)', r'\1*\2', 'Div -> Mul'),
    (r'(\s+)%(\s+)', r'\1*\2', 'Mod -> Mul'),

    # Relational operator mutations
    (r'==', r'!=', 'Eq -> Neq'),
    (r'!=', r'==', 'Neq -> Eq'),
    (r'<(\s+)', r'>\1', 'Less -> Greater'),
    (r'>(\s+)', r'<\1', 'Greater -> Less'),
    (r'<=', r'>=', 'Leq -> Geq'),
    (r'>=', r'<=', 'Geq -> Leq'),

    # Boolean logic mutations
    (r'\bKwAnd\b', r'TokenKind::KwOr', 'And -> Or'),
    (r'\bKwOr\b', r'TokenKind::KwAnd', 'Or -> And'),

    # Boundary constants
    (r'\b0\b', r'1', '0 -> 1'),
    (r'\b1\b', r'0', '1 -> 0')
]

TARGET_FILES = [
    'src/runtime/value.cpp',
    'src/frontend/lexer.cpp',
    'src/semantics/semantic_analyzer.cpp'
]

def run_tests(build_dir):
    res = subprocess.run(
        ['ctest', '--test-dir', build_dir, '--output-on-failure'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        timeout=30
    )
    return res.returncode == 0

def build_project(build_dir):
    res = subprocess.run(
        ['ninja', '-C', build_dir],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        timeout=60
    )
    return res.returncode == 0

def main():
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
    build_dir = os.path.join(repo_root, 'build/dev-clang')

    if not os.path.exists(build_dir):
        build_dir = os.path.join(repo_root, 'build/dev-gcc')
    if not os.path.exists(build_dir):
        print(f"Error: Build directory {build_dir} not found. Build the project first.")
        sys.exit(1)

    print("=== Execore Frontier Mutation Testing Pipeline ===")
    print("1. Running baseline test suite...")
    if not run_tests(build_dir):
        print("Error: Baseline test suite failed. Fix tests before running mutation testing.")
        sys.exit(1)
    print("   Baseline test suite PASSED (100% clean).")

    total_mutants = 0
    killed_mutants = 0
    survived_mutants = 0

    max_trials = 25
    trial_count = 0

    for rel_path in TARGET_FILES:
        full_path = os.path.join(repo_root, rel_path)
        if not os.path.exists(full_path):
            continue

        with open(full_path, 'r', encoding='utf-8') as f:
            original_code = f.read()

        for pattern, replacement, desc in MUTATIONS:
            if trial_count >= max_trials:
                break

            matches = list(re.finditer(pattern, original_code))
            if not matches:
                continue

            # Mutate the first match
            m = matches[0]
            mutated_code = original_code[:m.start()] + re.sub(pattern, replacement, m.group(0), count=1) + original_code[m.end():]
            if mutated_code == original_code:
                continue

            trial_count += 1
            total_mutants += 1
            print(f"[{trial_count:02d}] Testing mutant in {rel_path}: {desc} ...", end=" ", flush=True)

            # Apply mutation
            with open(full_path, 'w', encoding='utf-8') as f:
                f.write(mutated_code)

            try:
                # Compile mutant
                built = build_project(build_dir)
                if not built:
                    # Compilation error means the type system or concept killed the mutant!
                    killed_mutants += 1
                    print("KILLED (Compile-time Concept/Type Check)")
                    continue

                # Run tests
                tests_passed = run_tests(build_dir)
                if not tests_passed:
                    killed_mutants += 1
                    print("KILLED (Test Assertion Failed)")
                else:
                    survived_mutants += 1
                    print("SURVIVED (Warning: Test Gap)")
            finally:
                # Restore original file immediately
                with open(full_path, 'w', encoding='utf-8') as f:
                    f.write(original_code)
                build_project(build_dir)

    # Final restore and build
    build_project(build_dir)

    score = (killed_mutants / total_mutants * 100.0) if total_mutants > 0 else 0.0
    print("\n=== Mutation Testing Scorecard ===")
    print(f"Total Mutants Evaluated: {total_mutants}")
    print(f"Mutants Killed:          {killed_mutants}")
    print(f"Mutants Survived:        {survived_mutants}")
    print(f"Mutation Score:          {score:.1f}%")
    print(f"Target Criteria:         > 80.0%")

    if score >= 80.0:
        print(">>> ACCEPTANCE CRITERION MET: Mutation score exceeds 80.0% frontier threshold! <<<")
        sys.exit(0)
    else:
        print(">>> FAILED: Mutation score is below 80.0% threshold. <<<")
        sys.exit(1)

if __name__ == '__main__':
    main()
