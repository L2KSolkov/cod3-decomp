#!/usr/bin/env python3
"""Rebuild the function/type verification ledger from ground truth.

The release map and current source tree are the only inputs used to derive V1
and V2.  V3--V5 can only come from append-only rows in analysis/evidence; no
object or file status is consulted or inherited.  VERIFY*.tsv files are
derived outputs and may be deleted and regenerated at any time.
"""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import re
import shutil
import subprocess
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MAP_CANDIDATES = (ROOT / "c3_bin" / "codmp_xboxr.map", ROOT / "codmp_xboxr.map")
SOURCE_ROOTS = (ROOT / "src", ROOT / "platform")
EVIDENCE_DIR = ROOT / "analysis" / "evidence"
VERIFY = ROOT / "analysis" / "VERIFY.tsv"
SUMMARY = ROOT / "analysis" / "VERIFY_SUMMARY.tsv"
ANOMALIES = ROOT / "analysis" / "VERIFY_ANOMALIES.tsv"
TYPES = ROOT / "analysis" / "VERIFY_TYPES.tsv"

EVIDENCE_FIELDS = [
    "ida_ea", "name", "gate", "result", "session", "date", "source_ref", "detail"
]
EVIDENCE_REQUIRED = {"ida_ea", "name", "gate", "result", "session", "date", "source_ref"}
GATES = ("V1", "V2", "V3", "V4", "V5")
PASS = "PASS"

# The map has two game/engine segments.  Segment 2 is the normal image text;
# segment 9 is MP_LELCS.  Other segments are XDK/CRT and are retained in the
# manifest but excluded from the in-scope verification ledger.
SEGMENT_BASES = {2: 0x40C000, 9: 0xC8F720}
XDK_LIBS = {
    "dsoundd", "d3d8d", "xonlinesd", "xgraphicsd", "uixd", "xvoiced",
    "d3dx8d", "xapilibd", "dmusicd", "xboxkrnl", "xbdm", "uuid",
}
CRT_LIBS = {"LIBCMTD", "LIBCMT", "libcpmtd"}
ENGINE_LIBS = {
    "MPBrocCore_xboxd", "mp_level.xboxd", "phys_xboxr", "render_xboxr",
    "ngl_xboxr", "nal_xboxr", "nsl_xboxr", "nfl_xboxr", "nvl_xboxr",
    "aeps_xboxr", "mem_mp_xboxr", "core_xboxr", "tl_xboxr", "cdl_xboxr",
    "controller_xboxr", "jobqueue_xboxr", "peripherals_xboxr", "apk_xboxr",
    "inplace_xboxr", "bdCore", "bdConnection", "bdNet", "bdPeer",
    "bdSocket", "bdPlatform", "zlib_xboxr",
}
GAME_OBJECTS = {
    "g.o", "game.o", "game2.o", "shell.o", "core.o", "scr.o", "mp.o",
    "mp_shell.o", "mp_actors.o", "render.o", "anim.o", "streamer.o",
    "physics.o", "game_xbox.o", "mp_xbox.o", "cg.o", "cl.o", "sv.o",
    "nextgen.o", "CallFunctor.o",
}


@dataclass
class Function:
    ida_ea: int
    va: int
    name: str
    flags: str
    lib: str
    obj: str
    segment: int
    offset: int
    is_inline: bool
    cls: str


@dataclass
class Marker:
    address: int
    path: str
    line: int
    candidate: str
    body: str


def parse_map(path: Path) -> list[Function]:
    rows: list[Function] = []
    in_publics = False
    with path.open("r", encoding="utf-8", errors="replace") as stream:
        for raw in stream:
            line = raw.rstrip("\r\n")
            if not in_publics:
                if "Publics by Value" in line:
                    in_publics = True
                continue
            parts = line.split()
            if len(parts) < 4:
                continue
            match = re.match(r"^([0-9A-Fa-f]+):([0-9A-Fa-f]+)$", parts[0])
            if not match or not re.fullmatch(r"[0-9A-Fa-f]{8}", parts[2]):
                continue
            segment = int(match.group(1), 16)
            offset = int(match.group(2), 16)
            ida_ea = SEGMENT_BASES.get(segment, 0) + offset
            # Publics include data and linker records.  Only entries carrying
            # the map's function flag are functions; this must match
            # tools/parse_map.py exactly or the ledger denominator drifts.
            data = parts[3:]
            if not data or data[0] != "f":
                continue
            flags = "f"
            is_inline = len(data) > 1 and data[1] == "i"
            if is_inline:
                flags = "f i"
            tail = data[2:] if is_inline else data[1:]
            lib_obj = " ".join(tail)
            if ":" in lib_obj:
                lib, obj = lib_obj.split(":", 1)
            else:
                lib, obj = "", lib_obj
            if lib in ("<linker-defined>", "<absolute>"):
                continue
            cls = classify(lib.strip(), obj.strip())
            rows.append(Function(
                ida_ea, int(parts[2], 16), parts[1], flags, lib.strip(), obj.strip(),
                segment, offset, is_inline, cls,
            ))
    return rows


def classify(lib: str, obj: str) -> str:
    if lib in XDK_LIBS:
        return "xdk"
    if lib in CRT_LIBS:
        return "crt"
    if lib in ENGINE_LIBS:
        return "engine"
    if not lib and obj in GAME_OBJECTS:
        return "game"
    return "out-of-scope"


def source_files() -> list[Path]:
    result: list[Path] = []
    for root in SOURCE_ROOTS:
        if root.exists():
            result.extend(path for path in root.rglob("*")
                          if path.suffix in {".c", ".cc", ".cpp", ".h", ".hpp"})
    return sorted(result)


def function_candidate(lines: list[str], start: int) -> tuple[str, int] | None:
    """Find the first plausible definition after a marker.

    This deliberately rejects control statements and declarations ending in a
    semicolon.  Ambiguous or split signatures remain unlocated instead of
    receiving a false V1 pass.
    """
    text = ""
    for index in range(start, min(len(lines), start + 32)):
        piece = lines[index].strip()
        # A marker may trail the actual definition.  Keep code before the
        # marker, while treating a comment-only marker as a normal lead-in.
        if "//" in piece:
            piece = piece.split("//", 1)[0].strip()
        if piece.startswith("//") or not piece:
            continue
        text = (text + " " + piece).strip()
        if "(" not in text:
            continue
        if re.match(r"^(if|for|while|switch|catch|return)\b", text):
            text = ""
            continue
        if ";" in text and "{" not in text:
            # A declaration immediately following an EA marker is not an
            # implementation. Do not scan onward into the next method and
            # accidentally attach this address to an unrelated body.
            return None
        matches = list(re.finditer(r"([~A-Za-z_][A-Za-z0-9_:<>~]*)\s*\(", text))
        if matches and "{" in text:
            valid = []
            pointer = []
            for match in matches:
                candidate = match.group(1)
                prefix = text[:match.start()].strip()
                # Calls, control statements, placement-new, and expressions
                # can occur between a marker and the next definition. A
                # definition has a return/type prefix (or qualified method).
                if (candidate in {"if", "for", "while", "switch", "catch", "return", "new"}
                        or not prefix or prefix.endswith((".", "->", "="))):
                    continue
                valid.append((candidate, match))
                if text[:match.start()].rstrip().endswith("*"):
                    pointer.append((candidate, match))
            if pointer:
                return pointer[0][0], index
            if valid:
                return valid[0][0], index
            return None
        if len(text) > 240:
            text = ""
    return None


def matching_body(lines: list[str], brace_line: int) -> str:
    text = "\n".join(lines[brace_line:min(len(lines), brace_line + 250)])
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


def scan_markers() -> list[Marker]:
    markers: list[Marker] = []
    pattern = re.compile(r"//\s*ea:\s*(0x[0-9A-Fa-f]+)")
    for path in source_files():
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        for number, line in enumerate(lines):
            match = pattern.search(line)
            if not match:
                continue
            candidate_info = function_candidate(lines, number)
            if candidate_info is None:
                markers.append(Marker(int(match.group(1), 16), str(path.relative_to(ROOT)),
                                      number + 1, "", ""))
                continue
            candidate, brace_line = candidate_info
            markers.append(Marker(int(match.group(1), 16), str(path.relative_to(ROOT)),
                                  number + 1, candidate, matching_body(lines, brace_line)))
    return markers


def base_name(decorated: str) -> str:
    if decorated.startswith("??"):
        # ??0Class and ??1Class are constructor/destructor decorations.
        match = re.match(r"\?\?[01]([^@]+)", decorated)
        return match.group(1) if match else decorated
    if decorated.startswith("?"):
        match = re.match(r"\?([^@]+)", decorated)
        return match.group(1) if match else decorated
    return decorated.rsplit("::", 1)[-1].split("(", 1)[0]


def dumpbin_path() -> str | None:
    found = shutil.which("dumpbin")
    if found:
        return found
    candidates = sorted(Path(r"C:\Program Files\Microsoft Visual Studio").glob(
        r"**\VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe"))
    return str(candidates[-1]) if candidates else None


def debug_symbols() -> set[str]:
    build = ROOT / "build" / "Debug"
    binaries = sorted(build.glob("*.lib"))
    tool = dumpbin_path()
    if not binaries or tool is None:
        return set()
    symbols: set[str] = set()
    for binary in binaries:
        try:
            output = subprocess.check_output([tool, "/nologo", "/symbols", str(binary)],
                                             text=True, errors="replace")
        except (OSError, subprocess.CalledProcessError):
            continue
        for line in output.splitlines():
            if "External" in line and "|" in line:
                value = line.rsplit("|", 1)[1].strip().split(" ", 1)[0]
                if value:
                    symbols.add(value)
    return symbols


def symbol_variants(name: str) -> set[str]:
    values = {name}
    for old, new in (("QAM", "PAM"), ("QBM", "PBM"), ("QAY", "PAY"),
                     ("QBY", "PBY"), ("AAPA", "PAPA"), ("AAH", "QAH")):
        for value in tuple(values):
            values.add(value.replace(old, new))
            values.add(value.replace(new, old))
    return values


def read_evidence() -> tuple[dict[tuple[str, str, str], dict[str, str]], list[str]]:
    latest: dict[tuple[str, str, str], dict[str, str]] = {}
    errors: list[str] = []
    if not EVIDENCE_DIR.exists():
        return latest, errors
    for path in sorted(EVIDENCE_DIR.glob("*.tsv")):
        try:
            with path.open("r", encoding="utf-8", newline="") as stream:
                reader = csv.DictReader(stream, delimiter="\t")
                if reader.fieldnames != EVIDENCE_FIELDS:
                    errors.append(f"{path}: header must be {','.join(EVIDENCE_FIELDS)}")
                    continue
                for line, row in enumerate(reader, 2):
                    if any(not row.get(field, "").strip() for field in EVIDENCE_REQUIRED):
                        errors.append(f"{path}:{line}: required evidence fields are missing")
                        continue
                    if row["gate"] not in GATES:
                        errors.append(f"{path}:{line}: invalid gate {row['gate']}")
                        continue
                    if row["result"] not in {"PASS", "FAIL", "ADJUDICATE"}:
                        errors.append(f"{path}:{line}: invalid result {row['result']}")
                        continue
                    key = (row["ida_ea"].upper(), row["name"], row["gate"])
                    latest[key] = row
        except OSError as exc:
            errors.append(f"{path}: {exc}")
    return latest, errors


def body_class(body: str) -> str:
    if not body:
        return "NO_BODY"
    inner = body[1:-1] if body.startswith("{") and body.endswith("}") else body
    inner = re.sub(r"//[^\n]*|/\*.*?\*/", "", inner, flags=re.S).strip()
    if not inner:
        return "EMPTY_BODY"
    if re.fullmatch(r"(?:\(void\)\s*[^;]+;?\s*)+", inner):
        return "CAST_ONLY"
    return "REAL_BODY"


def compute_level(function: Function, hits: list[Marker], symbols: set[str],
                  evidence: dict[tuple[str, str, str], dict[str, str]]) -> tuple[dict[str, str], str]:
    """Apply gates for one function without inheriting object/file status."""
    located = len(hits) == 1 and bool(hits[0].candidate)
    if located:
        candidate = hits[0].candidate
        located = (base_name(function.name) in candidate or
                   candidate.endswith(base_name(function.name)))
    signature = bool(symbols) and bool(symbol_variants(function.name) & symbols)
    gate_state = {gate: "UNVERIFIED" for gate in GATES}
    gate_state["V1"] = PASS if located else "FAIL"
    gate_state["V2"] = PASS if signature else ("UNVERIFIED" if not symbols else "FAIL")
    level = "V0"
    if located:
        level = "V1"
        if signature:
            level = "V2"
            for gate in ("V3", "V4", "V5"):
                record = evidence.get((f"0x{function.ida_ea:08X}".upper(), function.name, gate))
                if record is None or record["result"] != PASS:
                    break
                gate_state[gate] = PASS
                level = gate
    return gate_state, level


def global_conflicts() -> list[tuple[str, str]]:
    declarations: defaultdict[str, set[str]] = defaultdict(set)
    # Anchor at the beginning of a source line.  Searching arbitrary text
    # treats every use/assignment of an IDA global as a competing declaration.
    pattern = re.compile(
        r"^\s*(?P<prefix>(?:(?:extern|static|const|volatile|unsigned|signed|long|short|int|char|float|double|bool|struct|class|enum)\s+)+)"
        r"(?:\*+\s*)?(?P<name>(?:dword|word|byte)_[0-9A-Fa-f]+)"
        r"(?P<array>\s*\[[^\]]*\])?\s*;", re.MULTILINE)
    for path in source_files():
        text = path.read_text(encoding="utf-8", errors="replace")
        for match in pattern.finditer(text):
            declaration = " ".join(match.group(0).split())
            declaration = re.sub(r"^extern\s+", "", declaration)
            declaration = re.sub(r"\[([^\]]+)\]", lambda item: _normalize_array(item.group(1)), declaration)
            declarations[match.group("name")].add(declaration)
    conflicts = []
    for name, values in sorted(declarations.items()):
        # An extern T name[] declaration is intentionally incomplete and is
        # compatible with a sized definition elsewhere in the port.
        if len(values) > 1 and any("[]" not in value for value in values):
            values = {value for value in values if "[]" not in value}
        if len(values) > 1:
            conflicts.append((name, " | ".join(sorted(values))))
    return conflicts


def _normalize_array(expression: str) -> str:
    """Normalize simple decimal/hex array extents (e.g. 0x322 == 802)."""
    compact = expression.replace(" ", "")
    if not re.fullmatch(r"[0-9A-Fa-fxX*+\-/()]+", compact):
        return f"[{expression}]"
    try:
        value = eval(compact, {"__builtins__": {}}, {})
    except (ArithmeticError, SyntaxError, ValueError):
        return f"[{expression}]"
    return f"[{int(value)}]"


def type_assertions() -> list[dict[str, str]]:
    """Collect compile-time size/offset assertions for the Track-B report."""
    rows: list[dict[str, str]] = []
    size_pattern = re.compile(
        r"static_assert\s*\(\s*sizeof\s*\(\s*([^()]+?)\s*\)\s*==\s*(0x[0-9A-Fa-f]+|\d+)")
    offset_pattern = re.compile(
        r"static_assert\s*\(\s*offsetof\s*\(\s*([^,]+?)\s*,\s*([^()]+?)\s*\)\s*==\s*(0x[0-9A-Fa-f]+|\d+)")
    for path in source_files():
        for number, line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
            match = size_pattern.search(line)
            if match:
                rows.append({"check": "sizeof", "type": match.group(1).strip(),
                             "field": "", "expected": match.group(2),
                             "source": str(path.relative_to(ROOT)), "line": str(number),
                             "status": "COMPILE_ASSERT_PRESENT"})
            match = offset_pattern.search(line)
            if match:
                rows.append({"check": "offsetof", "type": match.group(1).strip(),
                             "field": match.group(2).strip(), "expected": match.group(3),
                             "source": str(path.relative_to(ROOT)), "line": str(number),
                             "status": "COMPILE_ASSERT_PRESENT"})
    return rows


def write_tsv(path: Path, fields: list[str], rows: list[dict[str, str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.parse_args()  # Paths are intentionally fixed to this repository.

    map_path = next((path for path in MAP_CANDIDATES if path.exists()), None)
    if map_path is None:
        raise SystemExit("codmp_xboxr.map not found")
    functions = parse_map(map_path)
    in_scope = [row for row in functions if row.cls in {"game", "engine"}]
    by_ea = {row.ida_ea: row for row in in_scope}
    markers = scan_markers()
    marker_by_ea: defaultdict[int, list[Marker]] = defaultdict(list)
    for marker in markers:
        marker_by_ea[marker.address].append(marker)
    symbols = debug_symbols()
    evidence, evidence_errors = read_evidence()
    today = dt.date.today().isoformat()
    type_rows = type_assertions()

    rows: list[dict[str, str]] = []
    for function in in_scope:
        hits = marker_by_ea.get(function.ida_ea, [])
        gate_state, level = compute_level(function, hits, symbols, evidence)
        key_base = (f"0x{function.ida_ea:08X}", function.name)
        rows.append({
            "kind": "function", "ida_ea": key_base[0], "name": function.name,
            "obj": function.obj, "lib": function.lib, "class": function.cls,
            "source": hits[0].path if hits else "", "marker_line": str(hits[0].line) if hits else "",
            "V1": gate_state["V1"], "V2": gate_state["V2"], "V3": gate_state["V3"],
            "V4": gate_state["V4"], "V5": gate_state["V5"], "level": level,
            "updated": today,
        })

    anomalies: list[dict[str, str]] = []
    for marker in markers:
        if marker.address not in by_ea:
            anomalies.append({"kind": "ORPHAN_MARKER", "ida_ea": f"0x{marker.address:08X}",
                              "name": marker.candidate, "source": marker.path,
                              "detail": f"line {marker.line}"})
    for function in in_scope:
        hits = marker_by_ea.get(function.ida_ea, [])
        if len(hits) > 1:
            anomalies.append({"kind": "DUPLICATE_MARKER", "ida_ea": f"0x{function.ida_ea:08X}",
                              "name": function.name, "source": hits[0].path,
                              "detail": f"{len(hits)} source markers"})
        if len(hits) == 1 and hits[0].candidate:
            base = base_name(function.name)
            if base not in hits[0].candidate and not hits[0].candidate.endswith(base):
                anomalies.append({"kind": "MARKER_NAME_MISMATCH", "ida_ea": f"0x{function.ida_ea:08X}",
                                  "name": function.name, "source": hits[0].path,
                                  "detail": f"source candidate {hits[0].candidate}"})
        if hits and body_class(hits[0].body) in {"NO_BODY", "EMPTY_BODY", "CAST_ONLY"}:
            anomalies.append({"kind": "EMPTY_OR_CAST_BODY", "ida_ea": f"0x{function.ida_ea:08X}",
                              "name": function.name, "source": hits[0].path,
                              "detail": body_class(hits[0].body)})
    for name, detail in global_conflicts():
        anomalies.append({"kind": "GLOBAL_TYPE_CONFLICT", "ida_ea": "", "name": name,
                          "source": "src", "detail": detail})
    for error in evidence_errors:
        anomalies.append({"kind": "EVIDENCE_ERROR", "ida_ea": "", "name": "",
                          "source": "analysis/evidence", "detail": error})

    fields = ["kind", "ida_ea", "name", "obj", "lib", "class", "source", "marker_line",
              "V1", "V2", "V3", "V4", "V5", "level", "updated"]
    write_tsv(VERIFY, fields, rows)
    summary_rows: list[dict[str, str]] = []
    grouped: defaultdict[str, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        grouped[row["obj"]].append(row)
    for obj, group in sorted(grouped.items()):
        counts = Counter(row["level"] for row in group)
        summary_rows.append({"obj": obj, "rows": str(len(group)),
                             **{gate: str(counts[gate]) for gate in ("V0", "V1", "V2", "V3", "V4", "V5")}})
    write_tsv(SUMMARY, ["obj", "rows", "V0", "V1", "V2", "V3", "V4", "V5"], summary_rows)
    write_tsv(ANOMALIES, ["kind", "ida_ea", "name", "source", "detail"], anomalies)
    write_tsv(TYPES, ["check", "type", "field", "expected", "source", "line", "status"], type_rows)

    print(f"map functions: {len(functions)}")
    print(f"in-scope functions: {len(in_scope)}")
    print(f"source markers: {len(markers)}")
    print(f"debug symbols: {len(symbols)}")
    print("levels:", " ".join(f"{key}={value}" for key, value in sorted(Counter(r["level"] for r in rows).items())))
    print(f"anomalies: {len(anomalies)}")
    print(f"type assertions: {len(type_rows)}")
    return 1 if evidence_errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
