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
    # Release signatures include explicit template arguments (for example
    # ``TaskFunctor1<XAnimUpdateTask,float>::TaskFunctor1<...>``).  The
    # previous token pattern stopped at ``TaskFunctor1`` and then fell back
    # to the return type ``void``.  Keep angle-bracket contents opaque while
    # still stopping at the opening parenthesis of the call signature.
    component = r"[~A-Za-z_]\w*(?:<[^()\n{}]*>)?"
    name_pattern = re.compile(fr"({component}(?::{component})*)\s*\(")
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
        # Prefer the qualified function token.  Function-pointer parameter
        # declarations (for example `void (__cdecl *callback)(...)`) can
        # otherwise appear after the actual C++ function name and make a
        # constructor look like a function named `void`.
        qualified = [match.group(1) for match in names if "::" in match.group(1)]
        name = max(qualified, key=len) if qualified else names[-1].group(1)
        # A template return type can itself contain ``::``.  In that case
        # the permissive template token may include the calling convention
        # before the actual qualified function token; retain only the final
        # whitespace-delimited symbol.
        if "__" in name or " *" in name:
            name = name.split()[-1].lstrip("*&")
        # IDA emits the calling convention between the return type and the
        # qualified symbol.  Extracting the token after it handles ordinary
        # constructors (``Color::Color``) and templated constructors without
        # relying on the return type's spelling.
        calling = re.search(r"__(?:thiscall|cdecl|stdcall)\s+(.+?)\s*\(", signature)
        if calling:
            # Template arguments may contain spaces (``unsigned int``), so
            # splitting this substring on whitespace truncates the symbol.
            # The first opening parenthesis already terminates the complete
            # function token; only pointer/reference decoration is removed.
            candidate = calling.group(1).strip().lstrip("*&")
            if candidate:
                name = candidate
        body = matching_body(lines, signature_line)
        functions.setdefault(ea, (name, body))
        ea = None
        signature = ""
        signature_line = -1
    return functions


def main() -> int:
    release = release_functions()
    markers = verify_ledger.scan_markers()
    marker_by_ea: dict[int, list[verify_ledger.Marker]] = {}
    for marker in markers:
        marker_by_ea.setdefault(marker.address, []).append(marker)
    rows: list[dict[str, str]] = []
    functions = [function for function in verify_ledger.parse_map(
        next(path for path in verify_ledger.MAP_CANDIDATES if path.exists()))
                 if function.cls in {"game", "engine"}]
    for function in functions:
        marker = verify_ledger.canonical_hit(function,
                                             marker_by_ea.get(function.ida_ea, []))
        if marker is None:
            continue
        source_class = verify_ledger.body_class(marker.body)
        source_trivial = trivial_return(marker.body)
        if source_class not in {"NO_BODY", "EMPTY_BODY", "CAST_ONLY"} and not source_trivial:
            continue
        if source_trivial:
            source_class = "TRIVIAL_RETURN"
        if source_class == "NO_BODY":
            rows.append({
                "ida_ea": f"0x{marker.address:08X}",
                "source": marker.path,
                "line": str(marker.line),
                "source_candidate": marker.candidate,
                "source_class": source_class,
                "release_name": "",
                "release_class": "UNKNOWN",
                "result": "SOURCE_NOT_INDEXED",
            })
            continue
        release_entry = release.get(marker.address)
        if release_entry is None:
            result = "RELEASE_NOT_INDEXED"
            release_name = ""
            release_class = "UNKNOWN"
        else:
            release_name, release_body = release_entry
            release_class = verify_ledger.body_class(release_body)
            release_trivial = trivial_return(release_body)
            if release_trivial:
                release_class = "TRIVIAL_RETURN"
                result = "RELEASE_ALSO_TRIVIAL"
            elif release_class == "REAL_BODY" and implicit_cleanup(release_name, release_body):
                # The release compiler emits reference-count teardown for
                # by-value bdReference callback parameters, and emits base /
                # member teardown in virtual destructors.  An empty source
                # body is correct when C++ performs that work implicitly.
                result = "IMPLICIT_CLEANUP"
            elif release_class == "REAL_BODY" and not is_constructor(release_name):
                result = "CONFIRMED_REAL_RELEASE_BODY"
            elif release_class == "REAL_BODY":
                # C++ constructors can legitimately have an empty body while
                # doing all work in an initializer list; keep these separate
                # from function stubs for manual layout/source review.
                result = "CONSTRUCTOR_REVIEW"
            else:
                result = "RELEASE_ALSO_EMPTY"
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


def is_constructor(name: str) -> bool:
    parts: list[str] = []
    start = 0
    depth = 0
    index = 0
    while index < len(name):
        if name[index] == '<':
            depth += 1
        elif name[index] == '>' and depth:
            depth -= 1
        elif (name[index:index + 2] == '::' and depth == 0):
            parts.append(name[start:index])
            start = index + 2
            index += 1
        index += 1
    parts.append(name[start:])
    parts = [part for part in parts if part]
    return len(parts) >= 2 and parts[-1] == parts[-2]


def trivial_return(body: str) -> bool:
    """Return whether a function body consists only of a constant return."""
    if not body:
        return False
    inner = body[1:-1] if body.startswith("{") and body.endswith("}") else body
    inner = re.sub(r"//[^\n]*|/\*.*?\*/", "", inner, flags=re.S).strip()
    return bool(re.fullmatch(r"return\s+(?:nullptr|NULL|0|1|false|true)\s*;", inner))


def implicit_cleanup(name: str, body: str) -> bool:
    """Recognize compiler-emitted destructor/reference cleanup bodies."""
    if name.startswith("~") or "::~" in name:
        return True
    # bdReference-by-value callbacks decompile to only a refcount decrement;
    # there is no user-visible callback work to reconstruct.
    return "releaseRef" not in body and "--" in body and (
        "a2: 1" in body or "(*(a1 + 4))--" in body
    )


if __name__ == "__main__":
    raise SystemExit(main())
