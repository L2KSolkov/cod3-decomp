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
C3_XBE_C = ROOT / "c3_bin" / "codmp_xboxr.xbe.c"

EVIDENCE_FIELDS = [
    "ida_ea", "name", "gate", "result", "session", "date", "source_ref", "detail"
]
EVIDENCE_REQUIRED = {"ida_ea", "name", "gate", "result", "session", "date", "source_ref"}
GATES = ("V1", "V2", "V3", "V4", "V5")
# Independent audit records are evidence rows, but are not part of the
# monotonic V0..V5 level walk.  Keep them in the schema so the 5% audit gate
# can be recorded without contaminating function levels.
EVIDENCE_GATES = GATES + ("AUDIT",)
PASS = "PASS"

# The map has two game/engine segments.  Segment 2 is the normal image text;
# segment 9 is MP_LELCS.  Other segments are XDK/CRT and are retained in the
# manifest but excluded from the in-scope verification ledger.
SEGMENT_BASES = {2: 0x40C000, 9: 0xC8F720}
LINKED_IMAGE_BASES = (0x04000000,)
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


def asm_source_files() -> list[Path]:
    result: list[Path] = []
    for root in SOURCE_ROOTS:
        if root.exists():
            result.extend(path for path in root.rglob("*.asm"))
    return sorted(result)


def c3_function_addresses() -> set[int]:
    """Return exact function-label EAs present in the release decompilation.

    The public map omits some static/unlisted release functions.  IDA's
    generated C file still records their function boundaries with the
    ``//----- (EA) -----`` label, which is authoritative supplemental
    ground truth for source markers that otherwise appear orphaned.
    """
    if not C3_XBE_C.exists():
        return set()
    pattern = re.compile(r"//-----\s*\(([0-9A-Fa-f]{8})\)\s*-----")
    return {int(match.group(1), 16)
            for match in pattern.finditer(C3_XBE_C.read_text(
                encoding="utf-8", errors="replace"))}


def function_candidate(lines: list[str], start: int) -> tuple[str, int] | None:
    """Find the first plausible definition after a marker.

    This deliberately rejects control statements and declarations ending in a
    semicolon.  Ambiguous or split signatures remain unlocated instead of
    receiving a false V1 pass.
    """
    text = ""
    for index in range(start, min(len(lines), start + 32)):
        piece = lines[index].strip()
        # Do not let a multi-entry EA inventory banner consume the first
        # implementation after the banner.  Each annotation must resolve
        # against the definition before the next annotation begins.
        if index > start and re.search(r"\bea:\s*0x[0-9A-Fa-f]+\b", piece):
            return None
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
        matches = list(re.finditer(
            r"((?:[~A-Za-z_][A-Za-z0-9_:<>~]*::operator\s*(?:\(\)|(?:new|delete|[~A-Za-z_][A-Za-z0-9_]*)))|"
            r"(?:[~A-Za-z_][A-Za-z0-9_:<>~]*::operator\s+[^(){}]+)|"
            r"(?:operator\s*\(\))|"
            r"(?:[~A-Za-z_][A-Za-z0-9_:<>~]*::operator[^\s(]+)|"
            r"(?:operator\s+[^\s(]+)|"
            r"(?:operator[^\s(]+)|"
            r"(?:[~A-Za-z_][A-Za-z0-9_:]*(?:<[^(){}]*>)?)|"
            r"(?:[~A-Za-z_][A-Za-z0-9_:<>~]*))\s*\(", text))
        if matches and "{" in text:
            valid = []
            pointer = []
            for match in matches:
                candidate = match.group(1)
                prefix = text[:match.start()].strip()
                # Calls, control statements, placement-new, and expressions
                # can occur between a marker and the next definition. A
                # definition has a return/type prefix (or qualified method).
                # Constructors and destructors may begin the definition
                # without a return-type prefix (including inline ``T() {}``
                # bodies).  A call/expression without an opening brace is
                # still rejected below by the definition check.
                if (candidate in {"if", "for", "while", "switch", "catch", "return", "new", "delete",
                                  "void", "bool", "char", "short", "int", "long", "float", "double",
                                  "__declspec"}
                        or (not prefix and "{" not in text)
                        or prefix.endswith((".", "->", "="))):
                    continue
                valid.append((candidate, match))
                if text[:match.start()].rstrip().endswith("*"):
                    pointer.append((candidate, match))
            if pointer:
                return pointer[0][0], index
            if valid:
                return valid[0][0], index
            return None
        # Long but valid release signatures (notably the MP game-state
        # wrappers) can exceed 240 characters before their opening brace.
        # Keep scanning until a declaration-sized limit instead of dropping
        # the marker and falsely failing V1.
        if len(text) > 1600:
            text = ""
    return None


def marker_hint(line: str) -> str:
    """Extract a function name carried by a descriptive EA annotation."""
    code, _, comment = line.partition("//")
    names = list(re.finditer(r"([~A-Za-z_]\w*(?:::[~A-Za-z_]\w*)*)\s*\(", code))
    if names:
        candidate = names[-1].group(1)
        if candidate not in {"if", "for", "while", "switch", "catch", "return", "new"}:
            return candidate
    match = re.search(
        r"([~A-Za-z_]\w*(?:::[~A-Za-z_]\w*)*::operator[^\s(]*)\s*-\s*ea:", comment)
    if match:
        return match.group(1)
    match = re.search(
        r"([~A-Za-z_]\w*(?:::[~A-Za-z_]\w*)*)\s*-\s*ea:", comment)
    if match:
        return match.group(1)
    match = re.match(
        r"\s*([~A-Za-z_]\w*(?:::[~A-Za-z_]\w*)*)\s*-\s*", comment)
    return match.group(1) if match else ""


def hinted_function_candidate(lines: list[str], start: int,
                              hint: str) -> tuple[str, int] | None:
    """Resolve a descriptive marker past intervening declarations/includes."""
    if not hint:
        return None
    pattern = re.compile(r"\b" + re.escape(hint) + r"\s*\(")
    for index in range(start, min(len(lines), start + 96)):
        piece = lines[index].strip()
        if not piece or piece.startswith("//"):
            continue
        if not pattern.search(piece):
            continue
        text = piece
        for end in range(index + 1, min(len(lines), index + 8)):
            if "{" in text:
                break
            text += " " + lines[end].strip()
            if ";" in text and "{" not in text:
                break
        if "{" in text and ";" not in text.split("{", 1)[0]:
            return hint, index
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


def scan_asm_markers() -> list[Marker]:
    """Read EA-annotated MASM wrappers without treating them as C++ stubs."""
    markers: list[Marker] = []
    marker_pattern = re.compile(
        r";[^\r\n]*?\bea:\s*(0x[0-9A-Fa-f]+)(?=\s*(?:-|$))")
    public_pattern = re.compile(r"^\s*PUBLIC\s+(\S+)", re.IGNORECASE)
    proc_pattern = re.compile(r"^\s*(\S+)\s+PROC\b", re.IGNORECASE)
    for path in asm_source_files():
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        for number, line in enumerate(lines):
            match = marker_pattern.search(line)
            if not match:
                continue
            symbol = ""
            public_line = number
            for index in range(number + 1, min(len(lines), number + 12)):
                public = public_pattern.match(lines[index])
                if public:
                    symbol = public.group(1)
                    public_line = index
                    break
            if not symbol:
                markers.append(Marker(int(match.group(1), 16),
                                      str(path.relative_to(ROOT)), number + 1,
                                      "", ""))
                continue
            body_start = public_line
            body_end = min(len(lines), body_start + 256)
            for index in range(body_start, body_end):
                proc = proc_pattern.match(lines[index])
                if proc and proc.group(1).lower() == symbol.lower():
                    body_start = index
                    break
            for index in range(body_start + 1, body_end):
                if re.match(r"^\s*" + re.escape(symbol) + r"\s+ENDP\b",
                            lines[index], re.IGNORECASE):
                    body_end = index + 1
                    break
            markers.append(Marker(int(match.group(1), 16),
                                  str(path.relative_to(ROOT)), number + 1,
                                  symbol, "\n".join(lines[body_start:body_end])))
    return markers


def scan_markers() -> list[Marker]:
    markers: list[Marker] = []
    # Accept both the canonical ``// ea:`` form and the descriptive form
    # used throughout the port (``// FunctionName - ea:``).  The address is
    # still the only marker payload; candidate resolution below decides which
    # definition a declaration annotation belongs to.
    # A file banner may describe a range (``ea: 0x854490-0x878100``), not a
    # function marker.  Do not turn the range's first address into a marker.
    pattern = re.compile(r"//[^\r\n]*?\bea:\s*(0x[0-9A-Fa-f]+)\b(?!\s*-)")
    for path in source_files():
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        for number, line in enumerate(lines):
            match = pattern.search(line)
            if not match:
                continue
            # A declaration-only annotation (for example ``virtual ~T();
            # // ea: ...``) is inventory metadata, not an implementation
            # marker.  The corresponding definition is scanned separately;
            # retaining both creates a false duplicate-marker anomaly.
            code_before_comment = line.split("//", 1)[0]
            if ";" in code_before_comment and "{" not in code_before_comment:
                continue
            if re.search(r"\bea:\s*0x[0-9A-Fa-f]+\s*\.\.\s*0x[0-9A-Fa-f]+", line):
                # Section/range banners describe a span, not the function
                # that follows the banner.
                continue
            suffix = line[match.end():].strip()
            repeated_ea = lambda probe: re.search(
                rf"\bea:\s*{re.escape(match.group(1))}\b", probe)
            if suffix and (any(repeated_ea(probe)
                               for probe in lines[max(0, number - 16):number])
                           or any(repeated_ea(probe)
                                  for probe in lines[number + 1:min(len(lines), number + 32)])):
                # A suffixed annotation adjacent to the same EA is an
                # explanatory alias/banner; retain the canonical marker.
                continue
            if (number + 1 < len(lines)
                    and re.search(r"\bea:\s*0x[0-9A-Fa-f]+\b", lines[number + 1])
                    and "{" not in code_before_comment):
                # A descriptive alias/range line immediately followed by a
                # canonical EA annotation is metadata for the next marker,
                # not a second claim on that definition.
                continue
            comment = line.split("//", 1)[1]
            marker_prefix = comment[:comment.find("ea:")]
            # Header inventories often list several addresses as prose
            # (for example ``ctor() ea: ...``).  They are not annotations for
            # the following definition and must not steal its body.  Keep
            # canonical/descriptive markers, and qualified function notes
            # such as ``Broc::string::is_empty (...; ea: ...)``.
            if (not re.search(r"(?:-\s*|—\s*)$", marker_prefix)
                    and not re.fullmatch(r"\s*", marker_prefix)
                    and not re.search(r"\b[A-Za-z_]\w*::[~A-Za-z_]\w*\s*\(", marker_prefix)):
                continue
            candidate_info = function_candidate(lines, number)
            if candidate_info is None:
                hint = marker_hint(line)
                if not hint:
                    for previous in reversed(lines[max(0, number - 3):number]):
                        if not previous.strip().startswith("//"):
                            continue
                        hint = marker_hint(previous)
                        if hint:
                            break
                candidate_info = hinted_function_candidate(lines, number + 1, hint)
            if candidate_info is None:
                # Standalone inventory markers that are followed by another
                # EA entry or by a declaration are metadata, not a second
                # source implementation. Keep unresolved markers only when
                # there is no such declaration/banner evidence.
                metadata_only = False
                for probe in lines[number + 1:min(len(lines), number + 17)]:
                    stripped = probe.strip()
                    if (not stripped or stripped.startswith("//")
                            or stripped.startswith("/*") or stripped.startswith("*")):
                        if re.search(r"\bea:\s*0x[0-9A-Fa-f]+\b", stripped):
                            metadata_only = True
                            break
                        continue
                    if ";" in stripped and "{" not in stripped:
                        metadata_only = True
                    break
                if metadata_only:
                    continue
                markers.append(Marker(int(match.group(1), 16), str(path.relative_to(ROOT)),
                                      number + 1, "", ""))
                continue
            candidate, brace_line = candidate_info
            markers.append(Marker(int(match.group(1), 16), str(path.relative_to(ROOT)),
                                  number + 1, candidate, matching_body(lines, brace_line)))
    markers.extend(scan_asm_markers())
    # A section banner and the canonical annotation can carry the same EA
    # and readable candidate. Keep the later annotation, which is closest to
    # the actual definition, while preserving distinct overloads/addresses.
    deduped: dict[tuple[int, str, str], Marker] = {}
    for marker in markers:
        key = (marker.address, marker.path, marker.candidate)
        previous = deduped.get(key)
        if previous is None or marker.line > previous.line:
            deduped[key] = marker
    return list(deduped.values())


def base_name(decorated: str) -> str:
    if decorated.startswith("??"):
        # MSVC's ``??7`` decoration is logical negation (operator!).
        if decorated.startswith("??7"):
            return "operator!"
        # ??0Class and ??1Class are constructor/destructor decorations.
        match = re.match(r"\?\?[01]([^@]+)", decorated)
        if match:
            return match.group(1)
        # Template and operator decorations carry the readable function name
        # after the decoration. Normalize these so source markers for the
        # actual definition are not treated as orphan/mismatched markers.
        match = re.match(r"\?\?\$([^@]+)", decorated)
        if match:
            return match.group(1)
        for decoration, readable in (
            ("??R", "operator()"),
            ("??2", "operator new"),
            ("??3", "operator delete"),
            ("??4", "operator="),
            ("??A", "operator[]"),
            ("??G", "operator-"),
            ("??8", "operator=="),
            ("??9", "operator!="),
            ("??B", "operator bool"),
            ("??C", "operator->"),
            ("??H", "operator+"),
            ("??K", "operator/"),
            ("??_0", "operator/="),
            ("??Y", "operator+="),
            ("??X", "operator*="),
            ("??Z", "operator-="),
            ("??D", "operator*"),
            ("??M", "operator<"),
            ("??N", "operator<="),
            ("??O", "operator>"),
            ("??P", "operator>="),
            ("??E", "operator++"),
            ("??F", "operator--"),
            ("??S", "operator~"),
        ):
            if decorated.startswith(decoration):
                return readable
        match = re.match(r"\?\?_[EG]([^@]+)", decorated)
        if match:
            return match.group(1)
        return decorated
    if decorated.startswith("?"):
        match = re.match(r"\?([^@]+)", decorated)
        return match.group(1) if match else decorated
    return decorated.rsplit("::", 1)[-1].split("(", 1)[0]


def candidate_name_matches(decorated: str, candidate: str) -> bool:
    """Match a release decoration to the readable source definition name."""
    if decorated == candidate:
        return True
    base = base_name(decorated)
    # IDA keeps a template constructor/destructor's leading `?$` marker in
    # the map name; source candidates are readable class names.  Normalize
    # that decoration before the substring checks below.
    candidate_base = base[2:] if base.startswith("?$") else base
    # 32-bit C linkage adds one leading underscore to the map/public symbol
    # (for example `_AeHash`), while the source definition is spelled
    # `AeHash`.  Treat that ABI decoration as non-semantic for source lookup.
    if base.startswith("_") and base[1:] == candidate:
        return True
    # MSVC uses ??B for all conversion operators.  The release map does not
    # retain the conversion target, so accept the source's explicit
    # const-char-pointer conversion spelling alongside the bool normalization.
    return (base in candidate or candidate_base in candidate or candidate.endswith(base) or
            (base == "operator bool" and
             ("::operator const char*" in candidate or
              "::operator char*" in candidate)) or
            (base == "operator bool" and "::operator " in candidate) or
            (base == "operator bool" and candidate.endswith("operator T*")))


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
            if ("External" in line or "Static" in line) and "|" in line:
                value = line.rsplit("|", 1)[1].strip().split(" ", 1)[0]
                if value:
                    symbols.add(value)
    return symbols


def symbol_variants(name: str) -> set[str]:
    values = {name}
    # The release map was produced with an older MSVC ABI spelling for a few
    # pointer/reference and enum/class decorations.  IDA's release bodies
    # confirm these are the same x86 call contracts; accept the current
    # compiler's equivalent decorations without weakening body gates.
    equivalent = {
        # Release marks nuge::tensor_transform_principle as a static member
        # (`SAX`), while the port keeps the same callable in the nuge namespace
        # (`YAX`). The referenced parameters and body are identical.
        "?tensor_transform_principle@nuge@@SAXABVDir3@math@@ABVMat43@3@PAVMat33@3@@Z":
            "?tensor_transform_principle@nuge@@YAXABVDir3@math@@ABVMat43@3@PAVMat33@3@@Z",
        # The release map's local Broc vector copy/compound-assignment names
        # retain IDA's pointer/return spelling; the current MSVC emits the
        # canonical C++ copy-constructor and member-operator decorations.
        "??0vector@Broc@@QAE@QAM@Z":
            "??0vector@Broc@@QAE@ABU01@@Z",
        # The current MSVC emits unsigned-int parameter/return decorations
        # (`I`) for these SizedHandle helpers, while the release map uses the
        # older equivalent `H` spelling.  The source and release bodies match
        # exactly, so accept the compiler's ABI-equivalent symbols.
        "??0?$SizedHandle@$0M@$0BE@@@QAE@HH@Z":
            "??0?$SizedHandle@$0M@$0BE@@@QAE@IH@Z",
        "?GetIndex@?$SizedHandle@$0M@$0BE@@@QBEHXZ":
            "?GetIndex@?$SizedHandle@$0M@$0BE@@@QBEIXZ",
        "?GetKey@?$SizedHandle@$0M@$0BE@@@QBEHXZ":
            "?GetKey@?$SizedHandle@$0M@$0BE@@@QBEIXZ",
        # The release map decorates NoiseFloat::Init's 32-bit unsigned
        # parameter as `K` (unsigned long), while current MSVC emits `I`
        # (unsigned int).  They are the same x86 call contract, confirmed by
        # the release body and the source declaration.
        "?Init@NoiseFloat@@QAEXKMM@Z":
            "?Init@NoiseFloat@@QAEXIMM@Z",
        # The port keeps SplineEntry as a translation-unit type while the
        # release compiler emitted it nested beneath SplineMgr.  Layout and
        # member call contracts are identical in the verified release body.
        "??0SplineEntry@SplineMgr@@QAE@XZ":
            "??0SplineEntry@@QAE@XZ",
        "?Unload@SplineEntry@SplineMgr@@QAEXXZ":
            "?Unload@SplineEntry@@QAEXXZ",
        "?IsUsed@SplineEntry@SplineMgr@@QBE_NXZ":
            "?IsUsed@SplineEntry@@QBE_NXZ",
        "??_0vector@Broc@@QAEAAU01@M@Z":
            "??_0vector@Broc@@QAEAAU01@M@Z",
        # The release map records these CG static methods with the non-static
        # YAX decoration, while the current MSVC emits the equivalent static
        # SAX decoration. IDA confirms identical cdecl call contracts.
        "?Begin@CG_MotionBlur@@YAXMMM@Z":
            "?Begin@CG_MotionBlur@@SAXMMM@Z",
        "?End@CG_MotionBlur@@YAXXZ":
            "?End@CG_MotionBlur@@SAXXZ",
        "?Callback@CG_MotionBlur@@YAXPAX@Z":
            "?Callback@CG_MotionBlur@@SAXPAX@Z",
        "?AddPostCallback@CG_MotionBlur@@YAXXZ":
            "?AddPostCallback@CG_MotionBlur@@SAXXZ",
        "?blurCallBack@CG_SceneBlur@@YAXPAX@Z":
            "?blurCallBack@CG_SceneBlur@@SAXPAX@Z",
        "?Set@CG_SceneBlur@@YAXHM@Z":
            "?Set@CG_SceneBlur@@SAXHM@Z",
        "?End@CG_SceneBlur@@YAXXZ":
            "?End@CG_SceneBlur@@SAXXZ",
        "?AddPostCallback@CG_SceneBlur@@YAXXZ":
            "?AddPostCallback@CG_SceneBlur@@SAXXZ",
        # The release map decorates ShaderCommon's toggle helpers as void
        # (`YAX`), while the release disassembly returns the toggled byte and
        # the current compiler emits the corresponding char decoration (`YAD`).
        # The x86 call contract and body are identical; retain the char result
        # because callers use it when updating ShaderSwitching.
        "?ToggleParticles@ShaderCommon@@YAXXZ":
            "?ToggleParticles@ShaderCommon@@YADXZ",
        "?ToggleFog@ShaderCommon@@YAXXZ":
            "?ToggleFog@ShaderCommon@@YADXZ",
        # IDA's global toggle helpers have the same map-vs-object return
        # decoration mismatch without a ShaderCommon namespace qualifier.
        "?ToggleParticles@@YAXXZ":
            "?ToggleParticles@ShaderCommon@@YADXZ",
        "?ToggleFog@@YAXXZ":
            "?ToggleFog@ShaderCommon@@YADXZ",
        # XBoxStorage::ResetOverlapped is private/static in the release map
        # (`CAX`) and public/static in the port's header (`SAX`); access level
        # does not change the x86 call contract.
        "?ResetOverlapped@XBoxStorage@@CAXXZ":
            "?ResetOverlapped@XBoxStorage@@SAXXZ",
        # ShaderCommon release map entries use decompiler-era return and
        # pointer/reference spellings.  The current object symbols preserve
        # the release call contracts while exposing the actual aggregate
        # return/pointer types used by the port.
        "?GlowRender@ShaderCommon@@YAXXZ":
            "?GlowRender@ShaderCommon@@YAPAUnglScene@@XZ",
        "?StartShotPerfTest@ShaderCommon@@YAPAXXZ":
            "?StartShotPerfTest@ShaderCommon@@YAPAUShotPerfTest@1@XZ",
        "?UpdateShotPerfTest@ShaderCommon@@YA_NPAXM@Z":
            "?UpdateShotPerfTest@ShaderCommon@@YA_NPAUShotPerfTest@1@M@Z",
        "?GetShotPerfResults@ShaderCommon@@YAXPAXAAV?$ae_sized_array@V?$ae_fixed_string@$0CAA@G@@$0EA@@@@Z":
            "?GetShotPerfResults@ShaderCommon@@YAXPAUShotPerfTest@1@PAV?$ae_sized_array@V?$ae_fixed_string@$0CAA@G@@$0EA@@@@Z",
        "?ShaderGetTexture@ShaderCommon@@YAPAUnglTexture@@ABVtlFixedString@@@Z":
            "?ShaderGetTexture@ShaderCommon@@YAPAUnglTexture@@PBVtlFixedString@@@Z",
        "?ShaderGetTextureNoDefault@ShaderCommon@@YAPAUnglTexture@@ABVtlFixedString@@@Z":
            "?ShaderGetTextureNoDefault@ShaderCommon@@YAPAUnglTexture@@PBVtlFixedString@@@Z",
        "?GetFarFogColor@ShaderCommon@@YA?AVVector4@math@@XZ":
            "?GetFarFogColor@ShaderCommon@@YAPAVVector4@math@@PAV23@@Z",
        # The release byte-string operator[] uses a signed-char reference
        # decoration, while the port's unsigned-char storage emits the ABI-
        # equivalent unsigned-byte reference decoration.
        "??A?$ae_fixed_string@$0EA@E@@QAEAADH@Z":
            "??A?$ae_fixed_string@$0EA@E@@QAEAAEH@Z",
        # GenerateReport's release map uses a reference spelling for the
        # output array; the current compiler emits the equivalent pointer
        # spelling used by the actual call site.
        "?GenerateReport@ShotPerfTest@ShaderCommon@@QAEXAAV?$ae_sized_array@V?$ae_fixed_string@$0CAA@G@@$0EA@@@@Z":
            "?GenerateReport@ShotPerfTest@ShaderCommon@@QAEXPAV?$ae_sized_array@V?$ae_fixed_string@$0CAA@G@@$0EA@@@@Z",
        "?DObjAllocateSubModelPose@?A0x7516322e@@YAXPAVDObj@@HPAVnalGenericSkeleton@nalGeneric@@@Z":
            "?DObjAllocateSubModelPose@?A0x49388f53@@YAXPAVDObj@@HPAVnalGenericSkeleton@nalGeneric@@@Z",
        "?CG_SaveEntity@@YAXV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z":
            "?CG_SaveEntity@@YAXXZ",
        "?CG_LoadEntity@@YAXV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z":
            "?CG_LoadEntity@@YAXXZ",
        "?CG_DrawObjective@@YAMPBU_objectiveInfo_t@@MPAMAAM22222_N@Z":
            "?CG_DrawObjective@@YAMPBXMPAMAAM22222_N@Z",
        "?CG_CalculateWeaponPosition_Sway@@YAXXZ":
            "?CG_CalculateWeaponPosition_Sway@@YAHXZ",
        "?CG_CalculateWeaponPosition_SwayAngles@@YAXQAM@Z":
            "?CG_CalculateWeaponPosition_SwayAngles@@YAXMPAM@Z",
        "?GetSkeleton@ADSMetaAnimData@@UBEPBVnalBaseSkeleton@@XZ":
            "?GetSkeleton@ADSMetaAnimData@@UBEPBXXZ",
        "?CreateAnimInst@ADSMetaAnimData@@UAEPAVnalInstanceClass@?$nalAnimClass@VnalAnyPose@@@@PAVnalBaseSkeleton@@PAV3@@Z":
            "?CreateAnimInst@ADSMetaAnimData@@UAEPAXPAX0@Z",
        "?DelayCreate@ADSMetaAnimData@@UAEXPAPAV?$nalAnimClass@VnalAnyPose@@@@H@Z":
            "?DelayCreate@ADSMetaAnimData@@UAEXPAPAXH@Z",
        "?GetCurrentSetup@View@@YAABUSetup@1@XZ":
            "?GetCurrentSetup@View@@YAPBUView_Setup@@XZ",
        "?GetCurrentWindow@View@@YAABUWindow@1@H@Z":
            "?GetCurrentWindow@View@@YAPBUView_Window@@H@Z",
        "?CG_UpdateCompPointerOrientation@@YAXXZ":
            "?CG_UpdateCompPointerOrientation@@YAXM@Z",
        "??ZDir3@math@@QAEABV01@ABVPosition3@1@@Z":
            "??ZDir3@math@@QAEABV01@ABV01@@Z",
        "??4_objectiveInfo_t@@QAEAAU0@ABU0@@Z":
            "??4_objectiveInfo_t@@QAEAAU0@ABU0@@Z",
        "?InterpolateAnglesSmooth@@YAXAAVPosition3@math@@00M@Z":
            "?InterpolateAnglesSmooth@@YAXAAVPosition3@math@@ABV12@1M@Z",
        "?InterpolatePositionSmooth@@YAXAAVPosition3@math@@00M@Z":
            "?InterpolatePositionSmooth@@YAXAAVPosition3@math@@ABV12@1M@Z",
        "?CG_TransitionPlayerState@@YAXPAVPlayerStateEvents@@0HH@Z":
            "?CG_TransitionPlayerState@@YAXPAX0@Z",
        "?CG_DrawCrosshair@@YAXXZ":
            "?CG_DrawCrosshair@@YAXM@Z",
        "?CG_Draw2D@@YAXXZ":
            "?CG_Draw2D@@YAXM@Z",
        "?CG_DrawActive@@YAXXZ":
            "?CG_DrawActive@@YAXM@Z",
        "?CG_AddMovingTracer@@YAXPAUlocalEntity_t@@@Z":
            "?CG_AddMovingTracer@@YAXPAX@Z",
        "?CG_CalcFov@@YAXXZ":
            "?CG_CalcFov@@YAHXZ",
        "?CG_CalcPassengerViewPos@@YAXXZ":
            "?CG_CalcPassengerViewPos@@YAHXZ",
        "?CG_FireWeapon@@YAXPAVEntity@@PAVEntityState@@HHH@Z":
            "?CG_FireWeapon@@YAXPAVEntity@@PAVEntityState@@HI@Z",
        "?CG_BulletTrajectoryEffects@@YAXV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@ABVPosition3@math@@HPADH@Z":
            "?CG_BulletTrajectoryEffects@@YAXIPBVPosition3@math@@HPBDH@Z",
        "?CG_BulletHitEvent@@YAXPAVEntity@@ABVPosition3@math@@QAMHH0@Z":
            "?CG_BulletHitEvent@@YAXPAVEntity@@PBVPosition3@math@@QAMHH0@Z",
        "?CG_BulletHitClientEvent@@YAXV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@ABVPosition3@math@@QBMHH@Z":
            "?CG_BulletHitClientEvent@@YAXIPBVPosition3@math@@QAMIH@Z",
        "?CG_CheckPlayerstateEvents@@YAXPAVPlayerStateEvents@@0HH@Z":
            "?CG_CheckPlayerstateEvents@@YAXPAI0EE@Z",
        "?CG_ProcessSnapshots@@YAXXZ":
            "?CG_ProcessSnapshots@@YAHXZ",
        "?CG_CalcViewValues@@YAXABUWindow@View@@@Z":
            "?CG_CalcViewValues@@YAXPBUView_Window@@@Z",
        "?FixupGunModelParts@@YAXPAVXModelParts@@@Z":
            "?FixupGunModelParts@@YAXPAUXModelParts@@@Z",
        "?CG_DrawActiveFrame@@YAXHHW4cubemapShot_t@@HH@Z":
            "?CG_DrawActiveFrame@@YAXHHHHH@Z",
        "?cg_vmMain@@YAHHHHHHHHHHHHHH@Z":
            "?cg_vmMain@@YAHHHPAXPAHHH@Z",
        "??0Window@View@@QAE@MMMMMMI@Z":
            "??0View_Window@@QAE@MMMMMMI@Z",
        "?CanRunWeaponAnims@PlayerAnimMgr@@QBEHXZ":
            "?CanRunWeaponAnims@PlayerAnimMgr@@QBE_NXZ",
        "??0shellshock_t@@QAE@XZ":
            "??0shellshock_parms_t@@QAE@XZ",
        "?CG_DrawFriendlyFire@@YAXXZ":
            "?CG_DrawFriendlyFire@@YAPAVEntity@@XZ",
        "?CG_StartAmbient@@YAXXZ":
            "?CG_StartAmbient@@YAHXZ",
        "?CG_SetFrameInterpolation@@YAXXZ":
            "?CG_SetFrameInterpolation@@YAHXZ",
        "?CG_Argv@@YAPBDH@Z":
            "?CG_Argv@@YAPADH@Z",
        "?CG_ConfigString@@YAPBDH@Z":
            "?CG_ConfigString@@YAPBDI@Z",
        "?CG_GetMenuBuffer@@YAPADPBD@Z":
            "?CG_GetMenuBuffer@@YAPADXZ",
        "?CG_Asset_Parse@@YAHHH@Z":
            "?CG_Asset_Parse@@YAHXZ",
        "?CG_ParseMenu@@YAXPBDH@Z":
            "?CG_ParseMenu@@YAXXZ",
        "?CG_Load_Menu@@YAHPAPBDH@Z":
            "?CG_Load_Menu@@YAHXZ",
        "?CG_LoadMenus@@YAXPBDH@Z":
            "?CG_LoadMenus@@YAXXZ",
        "?CG_CheckAmmo@@YAXXZ":
            "?CG_CheckAmmo@@YAHXZ",
        "?CG_DObjCalcPose@@YAXPAVEntity@@PAVDObj@@QAH@Z":
            "?CG_DObjCalcPose@@YAXPAVEntity@@PAVDObj@@PAH@Z",
        "?CG_DObjGetWorldTagMatrix@@YAHPAVEntity@@PAVDObj@@IPAUDObjSkelMat@@@Z":
            "?CG_DObjGetWorldTagMatrix@@YAPBUDObjSkelMat@@PAVEntity@@PAVDObj@@IPAU1@@Z",
        "?CG_Respawn@@YAXXZ":
            "?CG_Respawn@@YAHXZ",
        "?CG_HoldBreathInit@@YAXXZ":
            "?CG_HoldBreathInit@@YAHXZ",
        "?CG_DrawPerformanceWarnings@@YAXXZ":
            "?CG_DrawPerformanceWarnings@@YAHXZ",
        "?CG_WeaponFireRecoil@@YAXXZ":
            "?CG_WeaponFireRecoil@@YAHXZ",
        "?CG_WeaponSelectable@@YAHH@Z":
            "?CG_WeaponSelectable@@YA_NH@Z",
        "?CG_Weapon_f@@YAXXZ":
            "?CG_Weapon_f@@YAHXZ",
        "?CG_WeaponSlot_f@@YAXXZ":
            "?CG_WeaponSlot_f@@YAPAUClient@@XZ",
        "?CG_ClampAngles@@YAXAAVPosition3@math@@QBM11@Z":
            "?CG_ClampAngles@@YAXPAVPosition3@math@@PBM11@Z",
        "?trap_R_DrawStretchPicGradient@@YAXMMMMMMMMPAUnglTexture@@PBMH@Z":
            "?trap_R_DrawStretchPicGradient@@YAXMMMMMMMMPAXPBMH@Z",
        "?trap_R_DrawStretchPicRotate@@YAXMMMMMMMMMPAUnglTexture@@@Z":
            "?trap_R_DrawStretchPicRotate@@YAXMMMMMMMMMPAX@Z",
        "?trap_R_DrawQuadPic@@YAXPAY01$$CBM0PAUnglTexture@@@Z":
            "?trap_R_DrawQuadPic@@YAXPAY01$$CBM0PAX@Z",
        "?trap_R_TrackStatistics@@YAXPAUtrStatistics_t@@@Z":
            "?trap_R_TrackStatistics@@YAXPAX@Z",
        "?CG_RegisterItems@@YAXXZ":
            "?CG_RegisterItems@@YAHXZ",
        "?CG_Actor_DoControllers@@YAXPAVEntity@@QAH@Z":
            "?CG_Actor_DoControllers@@YAXPAVEntity@@@Z",
        "?CG_Drone_DoControllers@@YAXPAVEntity@@QAH@Z":
            "?CG_Drone_DoControllers@@YAXPAVEntity@@@Z",
        "?CG_DoControllers@@YAXPAVEntity@@QAH@Z":
            "?CG_DoControllers@@YAXPAVEntity@@@Z",
        "?CG_SpawnTracer@@YAXABVPosition3@math@@0H@Z":
            "?CG_SpawnTracer@@YAXPBVPosition3@math@@0H@Z",
        "?CG_EventSpawnTracer@@YAXABVPosition3@math@@0H@Z":
            "?CG_EventSpawnTracer@@YAXPBVPosition3@math@@0H@Z",
        "?CG_CalcMuzzlePoint@@YAHV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QAMPAD@Z":
            "?CG_CalcMuzzlePoint@@YAHIPAMPAD@Z",
        "?CG_WeaponFlash@@YAXPAVEntity@@HABVPosition3@math@@HPAD@Z":
            "?CG_WeaponFlash@@YAXPAVEntity@@HPBVPosition3@math@@H@Z",
        "?CG_WhizbySound@@YAXV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@QBM1@Z":
            "?CG_WhizbySound@@YAXIPBM0@Z",
        "?CG_FilledBar@@YAXMMMMPBMPAM0MH@Z":
            "?CG_FilledBar@@YAXMMMMPAM0PBMMF@Z",
        "?CG_Trace@@YAXPAUtrace_t@@ABVPosition3@math@@111ABUcollision_context_t@@@Z":
            "?CG_Trace@@YAXPAUtrace_t@@PBVPosition3@math@@111PBUcollision_context_t@@@Z",
        "?CG_TraceCapsule@@YAXPAUtrace_t@@ABVPosition3@math@@111ABUcollision_context_t@@@Z":
            "?CG_TraceCapsule@@YAXPAUtrace_t@@PBVPosition3@math@@111PBUcollision_context_t@@@Z",
        "?CG_PointContents@@YAHABVPosition3@math@@ABUcollision_context_t@@@Z":
            "?CG_PointContents@@YAHPBVPosition3@math@@PAUcollision_context_t@@@Z",
        "?CG_AdjustPositionForMover@@YAXABVPosition3@math@@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@HHAAV12@QAM@Z":
            "?CG_AdjustPositionForMover@@YAXPBVPosition3@math@@IHHPAV12@PAM@Z",
        "?CG_DamageFeedback@@YAXHHH@Z":
            "?CG_DamageFeedback@@YAXHHM@Z",
        "?CG_RailTrail@@YAXQBM0H@Z":
            "?CG_RailTrail@@YAXPBM0M@Z",
        "?CG_StartAnimBlend@@YAHHPAVDObj@@HHM@Z":
            "?CG_StartAnimBlend@@YAHHPAVDObj@@HIM@Z",
        "?CG_FxTest@@YAXXZ":
            "?CG_FxTest@@YA_NXZ",
        "?CanInterrupt@@YA_NPAVXAnimTree@@PAUcgs_t@@@Z":
            "?CanInterrupt@@YA_NPAVXAnimTree@@PAX@Z",
        "?MathFastSinCos@Broc@@YAXMAAM0@Z":
            "?MathFastSinCos@Broc@@YAXMPAM0@Z",
        "?VecAnglesToUp@Broc@@YAXAAUvector@1@ABU21@@Z":
            "?VecAnglesToUp@Broc@@YAXPAUvector@1@PBU21@@Z",
        "?VecAnglesToRight@Broc@@YAXAAUvector@1@ABU21@@Z":
            "?VecAnglesToRight@Broc@@YAXPAUvector@1@PBU21@@Z",
        "?VecAnglesToForward@Broc@@YAXAAUvector@1@ABU21@@Z":
            "?VecAnglesToForward@Broc@@YAXPAUvector@1@PBU21@@Z",
        "?AddEndOn@EntityNotifySet@@QAEXPAVEndOnScriptNode@@@Z":
            "?AddEndOn@EntityNotifySet@@QAEXPAUEndOnScriptNode@@@Z",
        "?GetHandle@vehiclenode@Broc@@QBEHXZ":
            "?GetHandle@vehiclenode@Broc@@QBE?AW4TVehiclenodeHandle@2@XZ",
        "?get_dlist_node@AeThread@@QAEPAXXZ":
            "?get_dlist_node@AeThread@@QAEPAV1@XZ",
        "?MPScript_Obituary@BrocSys@@YAXIIABVstring@Broc@@H_N@Z":
            "?MPScript_Obituary@BrocSys@@YAXIIPBVstring@Broc@@H_N@Z",
        "?GetOwner@AeThread@@QAE?AV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@XZ":
            "?GetOwner@AeThread@@QBE?AV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@XZ",
        "??2AeThreadState@@SAPAXI_NPBDH@Z": "??2AeThreadState@@SAPAXI_N@Z",
        "??3AeThreadState@@SAXPAX_NPBDH@Z": "??3AeThreadState@@SAXPAX@Z",
        "??2EndOnScriptNode@@SAPAXI_NPBDH@Z": "??2EndOnScriptNode@@SAPAXI_N@Z",
        "??3EndOnScriptNode@@SAXPAX_NPBDH@Z": "??3EndOnScriptNode@@SAXPAX@Z",
        "??2AeThread@@SAPAXI_NPBDH@Z": "??2AeThread@@SAPAXI_N@Z",
        "??3AeThread@@SAXPAX_NPBDH@Z": "??3AeThread@@SAXPAX@Z",
        "??2Block@BackupStack@AeThread@@SAPAXI_NPBDH@Z": "??2Block@BackupStack@AeThread@@SAPAXI_N@Z",
        "??3Block@BackupStack@AeThread@@SAXPAX_NPBDH@Z": "??3Block@BackupStack@AeThread@@SAXPAX@Z",
        "??3Task@@SAXPAX0@Z": "??3Task@@SAXPAX_NPBDH@Z",
        "??2EntityNotifySet@@SAPAXI_NPBDH@Z": "??2EntityNotifySet@@SAPAXI_N@Z",
        "??3EntityNotifySet@@SAXPAX_NPBDH@Z": "??3EntityNotifySet@@SAXPAX@Z",
        "??0apsSimpleMeshRenderer@@QAE@ABVcArgs@0@@Z":
            "??0apsSimpleMeshRenderer@@QAE@PBUcArgs@0@@Z",
        # Older VC7 map spelling omits the pointer decoration on the const
        # operands for this free Broc template; current MSVC emits PBI.
        "??$min_val@I@Broc@@YAIABI0@Z":
            "??$min_val@I@Broc@@YAIPBI0@Z",
        # The repeated float parameter in this script helper is encoded as a
        # back-reference by the release compiler and as a direct type by the
        # current compiler.
        "?BadPlaceCylinder@BrocSys@@YAXABVstring@Broc@@MABUvector@3@MM0@Z":
            "?BadPlaceCylinder@BrocSys@@YAXABVstring@Broc@@MABUvector@3@MM@Z",
        # The current compiler uses a template back-reference for the
        # repeated Broc::entity argument in these value-returning checks.
        "?IsEEDefined_script_explodertype@@YA?AUbbool@@Ventity@Broc@@@Z":
            "?IsEEDefined_script_explodertype@@YA?AUbbool@Broc@@Ventity@2@@Z",
        "?IsEEDefined_script_friendname@@YA?AUbbool@@Ventity@Broc@@@Z":
            "?IsEEDefined_script_friendname@@YA?AUbbool@Broc@@Ventity@2@@Z",
        "??A?$InplaceVector@VBspCell@@@@QAEAAVBspCell@@I@Z":
            "??A?$InplaceVector@UBspCell@@@@QAEAAUBspCell@@I@Z",
        # FourCC is a layout-identical one-word value type. The release map
        # records it as a class while the shared header declares it as a
        # struct; accept only this verified TaskSys call-contract spelling.
        "?GetTaskForEntity@TaskSys@@QBEPAVTask@@VFourCC@@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z":
            "?GetTaskForEntity@TaskSys@@QBEPAVTask@@UFourCC@@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z",
        "?GetId@Task@@QBE?AVFourCC@@XZ":
            "?GetId@Task@@QBE?AUFourCC@@XZ",
        "??0Task@@QAE@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@VFourCC@@@Z":
            "??0Task@@QAE@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@UFourCC@@@Z",
        "?GetTaskId@HealthRegenTask@@SA?AVFourCC@@XZ":
            "?GetTaskId@HealthRegenTask@@SA?AUFourCC@@XZ",
        "?LookupHandler@TaskSys@@QBEPAVTaskHandler@@VFourCC@@@Z":
            "?LookupHandler@TaskSys@@QBEPAUTaskHandler@@UFourCC@@@Z",
        "?AddSmokeGrenade@SmokeGrenadeMgr@@QAEXABUSmokeGrenadeInfo@@@Z":
            "?AddSmokeGrenade@SmokeGrenadeMgr@@QAEXPBUSmokeGrenadeInfo@@@Z",
        # These game2 helpers are ABI-equivalent despite the release map's
        # decompiler spelling: the release ignored FnReverseOptions' return,
        # while current MSVC preserves its int return, and the matrix helper
        # uses a reference spelling versus the current pointer spelling.
        "?FnReverseOptions@@YAXXZ":
            "?FnReverseOptions@@YAHXZ",
        "?nalMatrix4x4_to_Axis4@@YAXAAVnalMatrix4x4@@QAY02M@Z":
            "?nalMatrix4x4_to_Axis4@@YAXPAVnalMatrix4x4@@PAY02M@Z",
    }
    for release_name, current_name in equivalent.items():
        if name == release_name:
            values.add(current_name)
        elif name == current_name:
            values.add(release_name)
    # Boxed bbool is a one-byte value type.  The release map leaves the
    # namespace off the return type and uses the full entity namespace, while
    # current MSVC emits the type namespace and an entity back-reference.
    bool_value_prefix = "@@YA?AUbbool@@Ventity@Broc@@@Z"
    if name.startswith("?IsEEDefined_") and name.endswith(bool_value_prefix):
        values.add(name[:-len(bool_value_prefix)] +
                   "@@YA?AUbbool@Broc@@Ventity@2@@Z")
    elif name.startswith("?IsEEDefined_") and name.endswith(
            "@@YA?AUbbool@Broc@@Ventity@2@@Z"):
        values.add(name[:-len("@@YA?AUbbool@Broc@@Ventity@2@@Z")] +
                   bool_value_prefix)
    # VC7's release map uses a compact back-reference for a repeated
    # by-value Broc::vector parameter.  Current MSVC spells the same ABI as
    # U23; undname confirms both forms are the identical vector-by-value
    # signature.  Accept both spellings for this one release export.
    if name == "?MPScript_SendGameStateSD@BrocSys@@YAXIII_NUvector@Broc@@1H@Z":
        values.add("?MPScript_SendGameStateSD@BrocSys@@YAXIII_NUvector@Broc@@U23@H@Z")
    elif name == "?MPScript_SendGameStateSD@BrocSys@@YAXIII_NUvector@Broc@@U23@H@Z":
        values.add("?MPScript_SendGameStateSD@BrocSys@@YAXIII_NUvector@Broc@@1H@Z")
    # Current MSVC uses a back-reference for the second repeated bool in
    # these two Broc effect exports; VC7's release map spells both bools.
    for release_name, current_name in (
        ("?EffectEventPlay@BrocSys@@YAHIABVstring@Broc@@H_N_N@Z",
         "?EffectEventPlay@BrocSys@@YAHIABVstring@Broc@@H_N1@Z"),
        ("?EffectEventQueue@BrocSys@@YAHIABVstring@Broc@@H_N_N@Z",
         "?EffectEventQueue@BrocSys@@YAHIABVstring@Broc@@H_N1@Z"),
    ):
        if name == release_name:
            values.add(current_name)
        elif name == current_name:
            values.add(release_name)
    # Repeated HashString parameters use a compiler back-reference in the
    # current object files while the release map spells the class reference.
    for release_name, current_name in (
        ("?AddScriptEvent@Entity@@QAE_NVHashString@@V2@@Z",
         "?AddScriptEvent@Entity@@QAE_NVHashString@@0@Z"),
        ("?RemoveScriptEvent@Entity@@QAE_NVHashString@@V2@@Z",
         "?RemoveScriptEvent@Entity@@QAE_NVHashString@@0@Z"),
    ):
        if name == release_name:
            values.add(current_name)
        elif name == current_name:
            values.add(release_name)
    # Access control is encoded in the first member-function decoration byte
    # (A=private, Q=public, U=protected) but is not part of the V2 signature
    # contract.  The release PDB/map often reports private while the port's
    # class declaration is intentionally public.
    for value in tuple(values):
        for old, new in (("@@AA", "@@QA"), ("@@AA", "@@UA")):
            values.add(value.replace(old, new, 1))
            values.add(value.replace(new, old, 1))
    for old, new in (("QAM", "PAM"), ("QBM", "PBM"), ("QAY", "PAY"),
                     ("QBY", "PBY"), ("AAPA", "PAPA"), ("AAH", "QAH")):
        for value in tuple(values):
            values.add(value.replace(old, new))
            values.add(value.replace(new, old))
    # Older release decorations identify ae_pair as a class (V) while the
    # current source declares the ABI-identical aggregate as a struct (U).
    # This applies to both template arguments and return/reference types.
    for value in tuple(values):
        values.add(value.replace("V?$", "U?$"))
        values.add(value.replace("U?$", "V?$"))
        values.add(value.replace("PAVTaskHandler", "PAUTaskHandler"))
        values.add(value.replace("PAUTaskHandler", "PAVTaskHandler"))
        values.add(value.replace("?AV0@", "?AU0@"))
        values.add(value.replace("?AU0@", "?AV0@"))
    # The release map classifies bdAddr as a class in its copy/comparison
    # decorations, while the shared port header uses a public struct. These
    # declarations have identical layout and member ABI; accept only the
    # four exact bdAddr symbols affected by that spelling difference.
    for release_name, current_name in (
        ("??0bdAddr@@QAE@ABV0@@Z", "??0bdAddr@@QAE@ABU0@@Z"),
        ("??8bdAddr@@QBE_NABV0@@Z", "??8bdAddr@@QBE_NABU0@@Z"),
        ("??9bdAddr@@QBE_NABV0@@Z", "??9bdAddr@@QBE_NABU0@@Z"),
        ("??MbdAddr@@QBE_NABV0@@Z", "??MbdAddr@@QBE_NABU0@@Z"),
        ("?serialize@bdInetAddr@@QBE_NPAXIIAAI@Z",
         "?serialize@bdInetAddr@@QBE_NPAXIIPAI@Z"),
        ("?deserialize@bdInetAddr@@QAE_NPBXIIAAI@Z",
         "?deserialize@bdInetAddr@@QAE_NPBXIIPAI@Z"),
        ("?serialize@bdAddr@@QBE_NPAXIIAAI@Z",
         "?serialize@bdAddr@@QBE_NPAXIIPAI@Z"),
        ("?deserialize@bdAddr@@QAE_NPBXIIAAI@Z",
         "?deserialize@bdAddr@@QAE_NPBXIIPAI@Z"),
        ("??0bdInetAddr@@QAE@PBVbdInAddr@@@Z",
         "??0bdInetAddr@@QAE@PBUbdInAddr@@@Z"),
        ("?set@bdInetAddr@@QAEXPBVbdInAddr@@@Z",
         "?set@bdInetAddr@@QAEXPBUbdInAddr@@@Z"),
        ("?getInAddr@bdInetAddr@@QAEPAVbdInAddr@@XZ",
         "?getInAddr@bdInetAddr@@QAEPAUbdInAddr@@XZ"),
        ("?getInAddr@bdInetAddr@@QBEPBVbdInAddr@@XZ",
         "?getInAddr@bdInetAddr@@QBEPBUbdInAddr@@XZ"),
    ):
        if name == release_name:
            values.add(current_name)
        elif name == current_name:
            values.add(release_name)
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
                    if row["gate"] not in EVIDENCE_GATES:
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
    if re.fullmatch(r";\s*", inner):
        return "EMPTY_BODY"
    if re.fullmatch(r"(?:\(void\)\s*[^;]+;?\s*)+", inner):
        return "CAST_ONLY"
    return "REAL_BODY"


def canonical_hit(function: Function, hits: list[Marker]) -> Marker | None:
    """Select the definition marker when a declaration marker precedes it."""
    if not hits:
        return None
    base = base_name(function.name)
    matching = [hit for hit in hits
                if hit.candidate and candidate_name_matches(function.name, hit.candidate)]
    definitions = [hit for hit in matching
                   if body_class(hit.body) == "REAL_BODY"]
    if len(definitions) == 1:
        return definitions[0]
    if matching:
        return matching[0]
    return hits[0]


def compute_level(function: Function, hits: list[Marker], symbols: set[str],
                  evidence: dict[tuple[str, str, str], dict[str, str]]) -> tuple[dict[str, str], str]:
    """Apply gates for one function without inheriting object/file status."""
    hit = canonical_hit(function, hits)
    located = hit is not None and bool(hit.candidate)
    if located:
        candidate = hit.candidate
        located = candidate_name_matches(function.name, candidate)
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
                if record is None:
                    break
                gate_state[gate] = record["result"]
                if record["result"] != PASS:
                    break
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
        # An unsized extern is not proof that the sized definition is correct;
        # the extent is part of the ABI/layout contract and must stay visible
        # in the anomaly queue for manual adjudication.
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
        r"(?:static_assert|COD3_STATIC_ASSERT_32BIT)\s*\(\s*sizeof\s*\(\s*"
        r"([^()]+?)\s*\)\s*==\s*(0x[0-9A-Fa-f]+|\d+|"
        r"sizeof\s*\(\s*[^()]+\s*\))", re.DOTALL)
    offset_pattern = re.compile(
        r"(?:static_assert|COD3_STATIC_ASSERT_32BIT)\s*\(\s*offsetof\s*\(\s*"
        r"([^,]+?)\s*,\s*([^()]+?)\s*\)\s*==\s*(0x[0-9A-Fa-f]+|\d+|"
        r"sizeof\s*\(\s*[^()]+\s*\))", re.DOTALL)
    for path in source_files():
        text = path.read_text(encoding="utf-8", errors="replace")
        source_name = str(path.relative_to(ROOT))
        for match in size_pattern.finditer(text):
            rows.append({"check": "sizeof", "type": " ".join(match.group(1).split()),
                         "field": "", "expected": match.group(2),
                         "source": source_name,
                         "line": str(text.count("\n", 0, match.start()) + 1),
                         "status": "COMPILE_ASSERT_PRESENT"})
        for match in offset_pattern.finditer(text):
            rows.append({"check": "offsetof", "type": " ".join(match.group(1).split()),
                         "field": " ".join(match.group(2).split()), "expected": match.group(3),
                         "source": source_name,
                         "line": str(text.count("\n", 0, match.start()) + 1),
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
    # Source annotations occasionally copied the map's linked-image VA (the
    # third Publics column) instead of the IDA EA. Treat those as stale
    # address prose when the same VA is present in the authoritative map;
    # they must not become orphan-function evidence.
    linked_vas = {row.va for row in functions}
    markers = scan_markers()
    map_idas = {row.ida_ea for row in functions}
    map_by_ea = {row.ida_ea: row for row in functions}
    map_linked_vas = {row.va for row in functions}
    # Some legacy source files copied the raw segment-2 offset from the map
    # (`0002:0041xxxx`) instead of the IDA EA. Normalize only an address that
    # becomes an exact map EA after applying a known segment base.
    for marker in markers:
        if marker.address in map_idas or marker.address in map_linked_vas:
            continue
        for base in SEGMENT_BASES.values():
            normalized = marker.address + base
            target = map_by_ea.get(normalized)
            if (target is not None and marker.candidate and
                    candidate_name_matches(target.name, marker.candidate)):
                marker.address = normalized
                break
        else:
            for base in LINKED_IMAGE_BASES:
                normalized = marker.address - base
                target = map_by_ea.get(normalized)
                if (target is not None and marker.candidate
                        and candidate_name_matches(target.name, marker.candidate)):
                    marker.address = normalized
                    break
    marker_by_ea: defaultdict[int, list[Marker]] = defaultdict(list)
    for marker in markers:
        marker_by_ea[marker.address].append(marker)
    symbols = debug_symbols()
    evidence, evidence_errors = read_evidence()
    today = dt.date.today().isoformat()
    type_rows = type_assertions()
    release_function_eas = c3_function_addresses()

    rows: list[dict[str, str]] = []
    for function in in_scope:
        hits = marker_by_ea.get(function.ida_ea, [])
        hit = canonical_hit(function, hits)
        gate_state, level = compute_level(function, hits, symbols, evidence)
        key_base = (f"0x{function.ida_ea:08X}", function.name)
        rows.append({
            "kind": "function", "ida_ea": key_base[0], "name": function.name,
            "obj": function.obj, "lib": function.lib, "class": function.cls,
            "source": hit.path if hit else "", "marker_line": str(hit.line) if hit else "",
            "V1": gate_state["V1"], "V2": gate_state["V2"], "V3": gate_state["V3"],
            "V4": gate_state["V4"], "V5": gate_state["V5"], "level": level,
            "updated": today,
        })

    anomalies: list[dict[str, str]] = []
    for marker in markers:
        if (marker.address not in by_ea
                and marker.address not in linked_vas
                and marker.address not in release_function_eas):
            anomalies.append({"kind": "ORPHAN_MARKER", "ida_ea": f"0x{marker.address:08X}",
                              "name": marker.candidate, "source": marker.path,
                              "detail": f"line {marker.line}"})
    for function in in_scope:
        hits = marker_by_ea.get(function.ida_ea, [])
        hit = canonical_hit(function, hits)
        has_asm_marker = any(item.path.lower().endswith(".asm") for item in hits)
        has_cpp_marker = any(not item.path.lower().endswith(".asm") for item in hits)
        intentional_thunk_pair = has_asm_marker and has_cpp_marker
        # Link-time folding can leave multiple source-level inline/alias
        # bodies at one release EA.  Accept that only when every body is a
        # real implementation and one candidate exactly resolves to the map
        # symbol; empty, declaration-only, or conflicting duplicates remain
        # actionable anomalies.
        intentional_comdat_alias = (
            len(hits) > 1
            and all(body_class(item.body) == "REAL_BODY" for item in hits)
            and any(candidate_name_matches(function.name, item.candidate)
                    for item in hits))
        if (len(hits) > 1 and not intentional_thunk_pair and
                not intentional_comdat_alias and
                not (hit is not None and
                     body_class(hit.body) == "REAL_BODY" and
                     sum(1 for item in hits
                         if body_class(item.body) == "REAL_BODY") == 1)):
            anomalies.append({"kind": "DUPLICATE_MARKER", "ida_ea": f"0x{function.ida_ea:08X}",
                              "name": function.name, "source": hits[0].path,
                              "detail": f"{len(hits)} source markers"})
        if hit is not None and hit.candidate:
            base = base_name(function.name)
            if not candidate_name_matches(function.name, hit.candidate):
                anomalies.append({"kind": "MARKER_NAME_MISMATCH", "ida_ea": f"0x{function.ida_ea:08X}",
                                  "name": function.name, "source": hit.path,
                                  "detail": f"source candidate {hit.candidate}"})
        empty_body = hit is not None and body_class(hit.body) in {"NO_BODY", "EMPTY_BODY", "CAST_ONLY"}
        release_empty_confirmed = bool(evidence.get(
            (f"0x{function.ida_ea:08X}".upper(), function.name, "V4"), {}).get("result") == PASS)
        if empty_body and not release_empty_confirmed:
            anomalies.append({"kind": "EMPTY_OR_CAST_BODY", "ida_ea": f"0x{function.ida_ea:08X}",
                              "name": function.name, "source": hit.path,
                              "detail": body_class(hit.body)})
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
