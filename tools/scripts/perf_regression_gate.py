#!/usr/bin/env python3
"""
Execore Frontier C++2026: CI Performance Regression Gate
Evaluates benchmark results against baseline and asserts latency regression < 3.0%.
"""

import sys
import json
import argparse
from typing import Dict, Any, List, Tuple


def parse_benchmark_json(file_path: str) -> Dict[str, Dict[str, Any]]:
    with open(file_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    
    results = {}
    for bm in data.get("benchmarks", []):
        name = bm.get("name")
        cpu_time = bm.get("cpu_time", bm.get("real_time", 0.0))
        results[name] = {
            "cpu_time": cpu_time,
            "real_time": bm.get("real_time", 0.0),
            "time_unit": bm.get("time_unit", "ns"),
            "bytes_per_second": bm.get("bytes_per_second", None),
            "items_per_second": bm.get("items_per_second", None),
        }
    return results


def evaluate_regressions(
    baseline: Dict[str, Dict[str, Any]],
    candidate: Dict[str, Dict[str, Any]],
    max_regression_pct: float = 3.0
) -> Tuple[bool, List[str]]:
    passed = True
    report = []
    
    report.append(f"{'Benchmark Name':<45} | {'Baseline (ns)':<15} | {'Candidate (ns)':<15} | {'Delta (%)':<10} | {'Status'}")
    report.append("-" * 105)
    
    for name, base_data in baseline.items():
        if name not in candidate:
            report.append(f"{name:<45} | Missing in candidate benchmark results! | [SKIP]")
            continue
        
        cand_data = candidate[name]
        base_time = base_data["cpu_time"]
        cand_time = cand_data["cpu_time"]
        
        if base_time <= 0:
            continue
            
        delta_pct = ((cand_time - base_time) / base_time) * 100.0
        
        if delta_pct > max_regression_pct:
            status = f"FAIL (>{max_regression_pct:.1f}%)"
            passed = False
        elif delta_pct < -3.0:
            status = "IMPROVED"
        else:
            status = "PASS"
            
        sign = "+" if delta_pct > 0 else ""
        report.append(
            f"{name:<45} | {base_time:<15.2f} | {cand_time:<15.2f} | {sign}{delta_pct:<9.2f}% | {status}"
        )
    
    return passed, report


def run_self_test():
    print("Running CI Performance Regression Gate Self-Test...")
    base = {
        "BM_Lexer/16": {"cpu_time": 100.0, "time_unit": "ns"},
        "BM_Parser/64": {"cpu_time": 200.0, "time_unit": "ns"},
        "BM_Interpreter/Loop": {"cpu_time": 300.0, "time_unit": "ns"},
    }
    
    # 1. Passing candidate (within 3% or faster)
    cand_pass = {
        "BM_Lexer/16": {"cpu_time": 101.5, "time_unit": "ns"},  # +1.5% -> pass
        "BM_Parser/64": {"cpu_time": 190.0, "time_unit": "ns"},  # -5.0% -> improved
        "BM_Interpreter/Loop": {"cpu_time": 305.0, "time_unit": "ns"}, # +1.6% -> pass
    }
    passed, _ = evaluate_regressions(base, cand_pass, max_regression_pct=3.0)
    assert passed, "Self-test failed: expected pass but got fail"
    
    # 2. Failing candidate (>3% regression)
    cand_fail = {
        "BM_Lexer/16": {"cpu_time": 108.0, "time_unit": "ns"},  # +8.0% -> fail
        "BM_Parser/64": {"cpu_time": 200.0, "time_unit": "ns"},
        "BM_Interpreter/Loop": {"cpu_time": 300.0, "time_unit": "ns"},
    }
    passed_fail, _ = evaluate_regressions(base, cand_fail, max_regression_pct=3.0)
    assert not passed_fail, "Self-test failed: expected fail but got pass"
    
    print("✅ Performance regression gate self-test passed all assertions successfully!")


def main():
    parser = argparse.ArgumentParser(description="CI Performance Regression Gate for Execore")
    parser.add_argument("--baseline", type=str, help="Path to baseline benchmark JSON")
    parser.add_argument("--candidate", type=str, help="Path to candidate benchmark JSON")
    parser.add_argument("--max-regression", type=float, default=3.0, help="Max allowed regression percentage (default: 3.0%%)")
    parser.add_argument("--self-test", action="store_true", help="Run internal self-test validation")
    
    args = parser.parse_args()
    
    if args.self_test:
        run_self_test()
        sys.exit(0)
        
    if not args.baseline or not args.candidate:
        parser.print_help()
        sys.exit(1)
        
    baseline_data = parse_benchmark_json(args.baseline)
    candidate_data = parse_benchmark_json(args.candidate)
    
    passed, report = evaluate_regressions(baseline_data, candidate_data, args.max_regression)
    
    print("\n" + "=" * 105)
    print(" EXECORE PERFORMANCE REGRESSION GATE REPORT")
    print(f" Maximum Allowable Regression Threshold: +{args.max_regression:.1f}%")
    print("=" * 105)
    for line in report:
        print(line)
    print("=" * 105 + "\n")
    
    if not passed:
        print("❌ FAILED: Performance regression exceeded allowable threshold (+3.0%)!", file=sys.stderr)
        sys.exit(1)
    else:
        print("✅ PASSED: All benchmark measurements within acceptable latency bounds.")
        sys.exit(0)


if __name__ == "__main__":
    main()
