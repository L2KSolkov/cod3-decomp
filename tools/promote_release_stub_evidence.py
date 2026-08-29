#!/usr/bin/env python3
"""Promote independently matched release no-op bodies into V3/V4 evidence.

The promotion is deliberately narrower than the stub report: empty bodies and
compiler-emitted cleanup are accepted directly, while trivial returns require
the source and release return statements to normalize identically.
"""

from __future__ import annotations

import csv
import datetime as dt
import re
from collections import defaultdict
from pathlib import Path

import release_stub_audit
import verify_ledger


ROOT = Path(__file__).resolve().parents[1]
AUDIT = ROOT / "analysis" / "RELEASE_STUB_AUDIT.tsv"
SESSION = "release-stub-promote-20260829"
DATE = dt.date.today().isoformat()


def normalized_trivial(body: str) -> str:
    inner = body[1:-1] if body.startswith("{") and body.endswith("}") else body
    inner = re.sub(r"//[^\n]*|/\*.*?\*/", "", inner, flags=re.S).strip()
    inner = re.sub(r"\s+", " ", inner)
    return inner


def release_by_ea() -> dict[int, tuple[str, str]]:
    return release_stub_audit.release_functions()


def main() -> int:
    release = release_by_ea()
    markers = verify_ledger.scan_markers()
    marker_by_ea: dict[int, list[verify_ledger.Marker]] = defaultdict(list)
    for marker in markers:
        marker_by_ea[marker.address].append(marker)
    map_path = next(path for path in verify_ledger.MAP_CANDIDATES if path.exists())
    functions = {function.ida_ea: function for function in verify_ledger.parse_map(map_path)}
    evidence, _ = verify_ledger.read_evidence()
    pending: dict[str, list[dict[str, str]]] = defaultdict(list)
    promoted = 0
    skipped = 0

    with AUDIT.open("r", encoding="utf-8", newline="") as stream:
        for row in csv.DictReader(stream, delimiter="\t"):
            if row["result"] not in {
                "RELEASE_ALSO_EMPTY", "IMPLICIT_CLEANUP", "RELEASE_ALSO_TRIVIAL"
            }:
                continue
            ea = int(row["ida_ea"], 16)
            function = functions.get(ea)
            release_entry = release.get(ea)
            if function is None or release_entry is None:
                skipped += 1
                continue
            marker = verify_ledger.canonical_hit(function, marker_by_ea.get(ea, []))
            if marker is None:
                skipped += 1
                continue
            release_name, release_body = release_entry
            result = row["result"]
            if result == "RELEASE_ALSO_TRIVIAL":
                if normalized_trivial(marker.body) != normalized_trivial(release_body):
                    skipped += 1
                    continue
            key_base = (f"0x{ea:08X}", function.name)
            detail = {
                "RELEASE_ALSO_EMPTY": "Source and release bodies are both explicit empty bodies.",
                "IMPLICIT_CLEANUP": "Source empty body is compiler-emitted cleanup; release body is the corresponding destructor/reference cleanup.",
                "RELEASE_ALSO_TRIVIAL": "Source and release bodies have the identical normalized trivial return statement.",
            }[result]
            rows = [
                {
                    "ida_ea": key_base[0], "name": key_base[1], "gate": "V3",
                    "result": "PASS", "session": SESSION, "date": DATE,
                    "source_ref": f"{row['source']}:{row['line']};c3_bin/codmp_xboxr.xbe.c:{row['ida_ea']}",
                    "detail": detail,
                },
                {
                    "ida_ea": key_base[0], "name": key_base[1], "gate": "V4",
                    "result": "PASS", "session": SESSION, "date": DATE,
                    "source_ref": f"c3_bin/codmp_xboxr.xbe.c:{row['ida_ea']};{row['source']}:{row['line']}",
                    "detail": detail,
                },
            ]
            target = ROOT / "analysis" / "evidence" / f"{function.obj}.tsv"
            for evidence_row in rows:
                key = (evidence_row["ida_ea"].upper(), evidence_row["name"], evidence_row["gate"])
                if key not in evidence:
                    pending[target.name].append(evidence_row)
                    evidence[key] = evidence_row
                    promoted += 1

    fields = verify_ledger.EVIDENCE_FIELDS
    for filename, rows in pending.items():
        target = ROOT / "analysis" / "evidence" / filename
        exists = target.exists()
        with target.open("a", encoding="utf-8", newline="") as stream:
            if not exists:
                stream.write("\t".join(fields) + "\n")
            writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
            writer.writerows(rows)
    print(f"promoted evidence rows: {promoted}")
    print(f"skipped rows: {skipped}")
    print(f"target evidence files: {len(pending)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
