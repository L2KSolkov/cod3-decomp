#!/usr/bin/env python3
"""Inventory source-defined C++ types and their layout-assertion coverage.

This is deliberately conservative: it records a type as UNVERIFIED when no
matching sizeof/offsetof assertion exists and never infers a release size from
the declaration text.
"""

from __future__ import annotations

import csv
import re
from collections import Counter
from pathlib import Path

import verify_ledger


ROOT = Path(__file__).resolve().parents[1]
REPORT = ROOT / "analysis" / "TYPE_INVENTORY.tsv"

TYPE_DEFINITION = re.compile(
    r"^\s*(?P<kind>struct|class|union|enum(?:\s+class)?)\s+"
    r"(?P<name>[A-Za-z_]\w*(?:<[^{};]*>)?(?:::[A-Za-z_]\w*)?)\s*"
    r"(?:final\s*)?(?:\:[^{]*)?\{"
)


def normalize(value: str) -> str:
    return " ".join(value.split())


def type_definitions() -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for path in verify_ledger.source_files():
        for number, line in enumerate(path.read_text(
                encoding="utf-8", errors="replace").splitlines(), 1):
            match = TYPE_DEFINITION.match(line)
            if match is None:
                continue
            rows.append({
                "kind": normalize(match.group("kind")),
                "type": normalize(match.group("name")),
                "source": str(path.relative_to(ROOT)),
                "line": str(number),
            })
    return rows


def assertion_coverage() -> list[dict[str, str]]:
    assertions = verify_ledger.type_assertions()
    definitions = type_definitions()
    rows: list[dict[str, str]] = []
    for definition in definitions:
        name = definition["type"]
        matches = [row for row in assertions
                   if row["type"] == name
                   or row["type"].endswith("::" + name)
                   or row["type"].startswith(name + "<")]
        rows.append({
            **definition,
            "assertions": str(len(matches)),
            "status": "ASSERTED" if matches else "UNVERIFIED",
        })
    return rows


def write_report(rows: list[dict[str, str]]) -> None:
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    with REPORT.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream,
                                fieldnames=["kind", "type", "source", "line",
                                            "assertions", "status"],
                                delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main() -> int:
    rows = assertion_coverage()
    write_report(rows)
    counts = Counter(row["status"] for row in rows)
    print(f"type definitions: {len(rows)}")
    print("coverage:", " ".join(f"{key}={value}"
                                  for key, value in sorted(counts.items())))
    print(f"report: {REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
