#!/usr/bin/env python3
"""
Execore Frontier C++2026: CycloneDX v1.5 JSON SBOM Generator
Generates a compliant Software Bill of Materials (SBOM) for Execore binaries and dependencies.
"""

import os
import sys
import json
import uuid
import datetime
import argparse
import hashlib
from typing import Dict, Any, List


def calculate_sha256(filepath: str) -> str:
    h = hashlib.sha256()
    with open(filepath, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()


def generate_cyclonedx_sbom(output_path: str, binary_path: str = None) -> Dict[str, Any]:
    timestamp = datetime.datetime.now(datetime.timezone.utc).isoformat()
    bom_uuid = f"urn:uuid:{uuid.uuid4()}"

    binary_hashes = []
    if binary_path and os.path.exists(binary_path):
        binary_hashes.append({
            "alg": "SHA-256",
            "content": calculate_sha256(binary_path)
        })

    sbom = {
        "$schema": "http://cyclonedx.org/schema/bom-1.5.schema.json",
        "bomFormat": "CycloneDX",
        "specVersion": "1.5",
        "serialNumber": bom_uuid,
        "version": 1,
        "metadata": {
            "timestamp": timestamp,
            "tools": [
                {
                    "vendor": "Execore",
                    "name": "execore-sbom-generator",
                    "version": "2.0.0"
                }
            ],
            "component": {
                "bom-ref": "pkg:generic/execore@2.0.0",
                "type": "application",
                "name": "execore",
                "version": "2.0.0",
                "description": "Execore Language Engine - Frontier C++2026 Edition",
                "licenses": [
                    {
                        "license": {
                            "id": "MIT"
                        }
                    }
                ],
                "hashes": binary_hashes,
                "properties": [
                    {"name": "execore:standard", "value": "ISO C++23"},
                    {"name": "execore:build_system", "value": "CMake 3.28+"},
                    {"name": "execore:hardening", "value": "Full RELRO, _FORTIFY_SOURCE=3, stack-protector-strong, noexecstack"},
                    {"name": "execore:optimization", "value": "PGO + ThinLTO"}
                ]
            }
        },
        "components": [
            {
                "bom-ref": "pkg:cmake/execore-core@2.0.0",
                "type": "library",
                "name": "Execore::core",
                "version": "2.0.0",
                "description": "Execore static core library (compiler frontend, AST, semantics, runtime)",
                "scope": "required",
                "licenses": [{"license": {"id": "MIT"}}]
            },
            {
                "bom-ref": "pkg:cmake/execore-frontend@2.0.0",
                "type": "library",
                "name": "execore-frontend",
                "version": "2.0.0",
                "description": "Lexical analyzer, off-side indent synthesis, TokenBufferSoA, recursive descent parser",
                "scope": "required",
                "licenses": [{"license": {"id": "MIT"}}]
            },
            {
                "bom-ref": "pkg:cmake/execore-runtime@2.0.0",
                "type": "library",
                "name": "execore-runtime",
                "version": "2.0.0",
                "description": "Tree-walk interpreter, PMR monotonic arena, Environment scopes, Value engine",
                "scope": "required",
                "licenses": [{"license": {"id": "MIT"}}]
            },
            {
                "bom-ref": "pkg:github/google/benchmark@1.9.1",
                "type": "library",
                "name": "google-benchmark",
                "version": "1.9.1",
                "description": "A microbenchmark support library",
                "scope": "optional",
                "licenses": [{"license": {"id": "Apache-2.0"}}]
            }
        ],
        "dependencies": [
            {
                "ref": "pkg:generic/execore@2.0.0",
                "dependsOn": [
                    "pkg:cmake/execore-core@2.0.0"
                ]
            },
            {
                "ref": "pkg:cmake/execore-core@2.0.0",
                "dependsOn": [
                    "pkg:cmake/execore-frontend@2.0.0",
                    "pkg:cmake/execore-runtime@2.0.0"
                ]
            }
        ]
    }

    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(sbom, f, indent=2)

    return sbom


def main():
    parser = argparse.ArgumentParser(description="Generate CycloneDX v1.5 JSON SBOM for Execore")
    parser.add_argument("--output", "-o", default="packaging/sbom.cyclonedx.json", help="Output path for SBOM JSON")
    parser.add_argument("--binary", "-b", default="build/release-pgo/execore", help="Path to compiled executable")
    args = parser.parse_args()

    sbom = generate_cyclonedx_sbom(args.output, args.binary)
    print(f"✅ CycloneDX v1.5 SBOM generated successfully: {args.output}")
    print(f"   Serial Number: {sbom['serialNumber']}")
    print(f"   Components: {len(sbom['components']) + 1}")


if __name__ == "__main__":
    main()
