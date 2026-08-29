#!/usr/bin/env python3
"""Regression tests for conservative type inventory matching."""

from __future__ import annotations

import tempfile
from pathlib import Path

import type_inventory as inventory


def main() -> int:
    original_sources = inventory.verify_ledger.source_files
    original_assertions = inventory.verify_ledger.type_assertions
    try:
        with tempfile.TemporaryDirectory(dir=inventory.ROOT) as directory:
            source = Path(directory) / "types.h"
            source.write_text(
                "struct Asserted { int value; };\n"
                "class Missing : Base { int value; };\n"
                "enum class Mode { A, B };\n",
                encoding="utf-8",
            )
            inventory.verify_ledger.source_files = lambda: [source]
            inventory.verify_ledger.type_assertions = lambda: [{
                "type": "Asserted", "check": "sizeof", "field": "",
                "expected": "4", "source": "types.h", "line": "1",
                "status": "COMPILE_ASSERT_PRESENT",
            }]
            rows = inventory.assertion_coverage()
            assert [(row["type"], row["status"]) for row in rows] == [
                ("Asserted", "ASSERTED"),
                ("Missing", "UNVERIFIED"),
                ("Mode", "UNVERIFIED"),
            ]
    finally:
        inventory.verify_ledger.source_files = original_sources
        inventory.verify_ledger.type_assertions = original_assertions
    print("type_inventory invariants: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
