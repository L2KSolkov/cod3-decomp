#!/usr/bin/env python3
"""Compare source marker bodies with the release C dump for stub triage.

This is intentionally a V3-only audit.  It does not claim equivalence; it
answers the narrower question whether an empty/cast-only source body is also
empty in the release or is a missing implementation.
"""

from __future__ import annotations

import csv
import re
from collections import Counter
from pathlib import Path

import verify_ledger


ROOT = Path(__file__).resolve().parents[1]
RELEASE_C = ROOT / "c3_bin" / "codmp_xboxr.xbe.c"
REPORT = ROOT / "analysis" / "RELEASE_STUB_AUDIT.tsv"


def matching_body(lines: list[str], brace_line: int) -> str:
    text = "\n".join(lines[brace_line:min(len(lines), brace_line + 1200)])
    open_at = text.find("{")
    if open_at < 0:
        return ""
    depth = 0
    in_string = False
    escaped = False
    end = len(text)
    for index in range(open_at, len(text)):
        char = text[index]
        if in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            continue
        if char == '"':
            in_string = True
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                end = index + 1
                break
    return text[open_at:end]


def release_functions() -> dict[int, tuple[str, str]]:
    lines = RELEASE_C.read_text(encoding="utf-8", errors="replace").splitlines()
    functions: dict[int, tuple[str, str]] = {}
    ea: int | None = None
    signature = ""
    signature_line = -1
    ea_pattern = re.compile(r"^//----- \(00([0-9A-Fa-f]{6,8})\)")
    name_pattern = re.compile(r"([~A-Za-z_]\w*(?:::[~A-Za-z_]\w*)*)\s*\(")
    for number, raw in enumerate(lines):
        match = ea_pattern.match(raw)
        if match:
            ea = int(match.group(1), 16)
            signature = ""
            signature_line = -1
            continue
        if ea is None:
            continue
        piece = raw.strip()
        if not piece or piece.startswith("//"):
            continue
        if signature_line < 0:
            signature_line = number
        signature = (signature + " " + piece).strip()
        if "{" not in signature:
            if ";" in signature or len(signature) > 600:
                ea = None
                signature = ""
                signature_line = -1
            continue
        names = list(name_pattern.finditer(signature))
        if not names:
            ea = None
            signature = ""
            signature_line = -1
            continue
        name = names[-1].group(1)
        body = matching_body(lines, signature_line)
        functions.setdefault(ea, (name, body))
        ea = None
        signature = ""
        signature_line = -1
    return functions


def main() -> int:
    release = release_functions()
    markers = verify_ledger.scan_markers()
    rows: list[dict[str, str]] = []
    for marker in markers:
        source_class = verify_ledger.body_class(marker.body)
        if source_class not in {"NO_BODY", "EMPTY_BODY", "CAST_ONLY"}:
            continue
        release_entry = release.get(marker.address)
        if release_entry is None:
            result = "RELEASE_NOT_INDEXED"
            release_name = ""
            release_class = "UNKNOWN"
        else:
            release_name, release_body = release_entry
            release_class = verify_ledger.body_class(release_body)
            result = ("CONFIRMED_REAL_RELEASE_BODY"
                      if release_class == "REAL_BODY" else
                      "RELEASE_ALSO_EMPTY")
        rows.append({
            "ida_ea": f"0x{marker.address:08X}",
            "source": marker.path,
            "line": str(marker.line),
            "source_candidate": marker.candidate,
            "source_class": source_class,
            "release_name": release_name,
            "release_class": release_class,
            "result": result,
        })
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    fields = ["ida_ea", "source", "line", "source_candidate", "source_class",
              "release_name", "release_class", "result"]
    with REPORT.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    print(f"source empty/cast markers: {len(rows)}")
    print("results:", " ".join(f"{key}={value}" for key, value in
                                sorted(Counter(row["result"] for row in rows).items())))
    print(f"report: {REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
