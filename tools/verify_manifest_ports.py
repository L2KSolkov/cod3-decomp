"""Build a manifest-wide port verification checklist.

The checklist deliberately keeps the release manifest as the source of truth.
It does not infer function bodies or types; it only joins each manifest row to
the recorded object-level port status and to symbols emitted by the current
Debug static libraries.
"""

from __future__ import annotations

import csv
import os
import re
import shutil
import subprocess
from collections import Counter, defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "analysis" / "MANIFEST_CLASSIFIED.tsv"
PROGRESS = ROOT / "PROGRESS.tsv"
BUILD = ROOT / "build" / "Debug"
OUT = ROOT / "analysis" / "manifest_function_checklist.tsv"
SUMMARY = ROOT / "analysis" / "manifest_function_checklist_summary.tsv"
GAME_OUT = ROOT / "analysis" / "game_related_manifest_function_checklist.tsv"
GAME_SUMMARY = ROOT / "analysis" / "game_related_manifest_function_checklist_summary.tsv"
SHELL_FORMAT_INVENTORY = ROOT / "analysis" / "game_related_inventory.tsv"
SHELL_FORMAT_PLAN = ROOT / "analysis" / "game_related_plan.tsv"
SHELL_FORMAT_CHECKLIST = ROOT / "analysis" / "game_related_checklist.tsv"
OBJECT_TABLE_DIR = ROOT / "analysis"


def read_tsv(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def status_rank(status: str) -> int:
    return {
        "PORTED": 4,
        "COMPLETE": 4,
        "VERIFIED": 3,
        "VALIDATED": 3,
        "IN_PROGRESS": 1,
    }.get(status, 0)


def aggregate_statuses(rows: list[dict[str, str]]) -> dict[tuple[str, str], str]:
    """Return the strongest status recorded for each object/source pair."""
    result: dict[tuple[str, str], str] = {}
    for row in rows:
        source = row.get("source_cpp", "") or ""
        obj = row.get("obj", "") or ""
        # A few historical progress rows predate the current seven-column
        # format and are shifted left. Recover their object from source_cpp.
        if not obj.endswith(".o") and source.endswith(".o"):
            obj = source
        status = ""
        for candidate in row.values():
            if not candidate:
                continue
            token = candidate.split(" ", 1)[0]
            if token in {"PORTED", "COMPLETE", "VERIFIED", "VALIDATED", "IN_PROGRESS"}:
                status = token
                break
        if not source or not obj or not status:
            continue
        key = (source, obj)
        if status_rank(status) > status_rank(result.get(key, "")):
            result[key] = status.split(" ", 1)[0]
    return result


def dumpbin_symbols() -> set[str]:
    """Collect decorated external symbols from every Debug static library."""
    symbols: set[str] = set()
    dumpbin = os.environ.get("DUMPBIN", "dumpbin")
    libraries = sorted(BUILD.glob("*.lib"))
    if not libraries:
        raise SystemExit(f"no Debug libraries found in {BUILD}")
    for library in libraries:
        command = [dumpbin, "/nologo", "/symbols", str(library)]
        try:
            output = subprocess.check_output(command, text=True, errors="replace")
        except (OSError, subprocess.CalledProcessError) as exc:
            raise SystemExit(
                f"dumpbin failed for {library.name}; run from vcvars32 or set DUMPBIN"
            ) from exc
        for line in output.splitlines():
            # DUMPBIN's symbol name starts after the final "|" delimiter.
            if "External" not in line:
                continue
            if "|" not in line:
                continue
            name = line.rsplit("|", 1)[1].strip().split(" ", 1)[0]
            if name:
                symbols.add(name)
    return symbols


def symbol_compat_variants(name: str) -> set[str]:
    """Return known VC7.1/modern-MSVC pointer-decoration equivalents."""
    variants = {name}
    # VC7.1's map encoded pointer qualifiers as QAM/QBM/QAY/QBY in several
    # release signatures; current MSVC emits the corresponding P* forms.
    for old, modern in (("QAM", "PAM"), ("QBM", "PBM"),
                        ("QAY", "PAY"), ("QBY", "PBY")):
        for value in tuple(variants):
            if old in value:
                variants.add(value.replace(old, modern))
            if modern in value:
                variants.add(value.replace(modern, old))
    return variants


def source_status(
    row: dict[str, str], exact: dict[tuple[str, str], str], by_obj: dict[str, str]
) -> tuple[str, str]:
    source = row.get("source_cpp", "")
    obj = row.get("obj", "")
    if (source, obj) in exact:
        return exact[(source, obj)], "PROGRESS exact source/object aggregate"
    if obj in by_obj:
        return by_obj[obj], "PROGRESS object aggregate"
    return "UNTRACKED", "no object aggregate in PROGRESS.tsv"


def main() -> None:
    manifest = read_tsv(MANIFEST)
    progress = read_tsv(PROGRESS)
    exact = aggregate_statuses(progress)
    by_obj: dict[str, str] = {}
    for (source, obj), status in exact.items():
        if status_rank(status) > status_rank(by_obj.get(obj, "")):
            by_obj[obj] = status

    symbols = dumpbin_symbols()
    rows: list[dict[str, str]] = []
    for index, row in enumerate(manifest, 1):
        status, basis = source_status(row, exact, by_obj)
        name = row.get("name", "")
        symbol = "YES" if name in symbols else "NO"
        symbol_compat = "YES" if symbol == "YES" or any(
            candidate in symbols for candidate in symbol_compat_variants(name)
        ) else "NO"
        row_class = row.get("class", "")
        if row_class in {"xdk", "crt"}:
            verification = "EXTERNAL"
        elif status in {"PORTED", "COMPLETE", "VERIFIED", "VALIDATED"} and symbol_compat == "YES":
            verification = "VERIFIED"
        elif status in {"PORTED", "COMPLETE", "VERIFIED", "VALIDATED"}:
            verification = "PORT_STATUS_ONLY"
        elif status == "IN_PROGRESS":
            verification = "IN_PROGRESS"
        elif row_class == "engine":
            verification = "UNRESOLVED_ENGINE"
        else:
            verification = "UNTRACKED"
        rows.append(
            {
                "index": str(index),
                "ida_ea": row.get("ida_ea", ""),
                "va": row.get("va", ""),
                "name": name,
                "lib": row.get("lib", ""),
                "obj": row.get("obj", ""),
                "class": row.get("class", ""),
                "source_cpp": row.get("source_cpp", ""),
                "flags": row.get("flags", ""),
                "is_inline": row.get("is_inline", ""),
                "aggregate_status": status,
                "status_basis": basis,
                "debug_symbol": symbol,
                "debug_symbol_compat": symbol_compat,
                "verification": verification,
            }
        )

    fields = list(rows[0]) if rows else []
    with OUT.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)

    by_obj_counts: defaultdict[str, Counter[str]] = defaultdict(Counter)
    for row in rows:
        by_obj_counts[row["obj"]][row["verification"]] += 1
    with SUMMARY.open("w", encoding="utf-8", newline="") as stream:
        write_summary(stream, by_obj_counts)

    game_rows = [row for row in rows if row["class"] in {"game", "engine"}]
    with GAME_OUT.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(game_rows)
    game_counts: defaultdict[str, Counter[str]] = defaultdict(Counter)
    for row in game_rows:
        game_counts[row["obj"]][row["verification"]] += 1
    with GAME_SUMMARY.open("w", encoding="utf-8", newline="") as stream:
        write_summary(stream, game_counts)

    write_shell_format_tables(game_rows, rows)

    print(f"manifest rows: {len(rows)}")
    print(f"debug libraries: {len(list(BUILD.glob('*.lib')))}")
    for verification, count in Counter(row["verification"] for row in rows).most_common():
        print(f"{verification}: {count}")
    print(f"wrote {OUT}")
    print(f"wrote {SUMMARY}")
    print(f"game-related rows: {len(game_rows)}")
    print(f"wrote {GAME_OUT}")
    print(f"wrote {GAME_SUMMARY}")
    print(f"wrote {SHELL_FORMAT_INVENTORY}")
    print(f"wrote {SHELL_FORMAT_PLAN}")
    print(f"wrote {SHELL_FORMAT_CHECKLIST}")


def shell_status(row: dict[str, str]) -> str:
    """Map the detailed audit result to the shell.o checklist vocabulary."""
    if shell_skip_note(row):
        return "SKIPPED"
    if row["verification"] == "VERIFIED":
        return "VERIFIED"
    if row["aggregate_status"] in {"PORTED", "COMPLETE", "VALIDATED"}:
        return "PORTED"
    if row["aggregate_status"] == "IN_PROGRESS":
        return "PENDING"
    return "PENDING"


def shell_skip_note(row: dict[str, str]) -> str:
    """Return a note for library/compiler helper rows omitted from porting."""
    name = row.get("name", "")
    if (
        "std@@" in name
        or "char_traits" in name
        or "basic_string" in name
        or "basic_ostream" in name
        or "basic_istream" in name
        or "allocator" in name
        or name.startswith("??2@")
        or name.startswith("??3@")
    ):
        return "SKIPPED: STL/compiler helper; supplied by the toolchain/library"
    return ""


def shell_map_line(row: dict[str, str]) -> str:
    ida_ea = int(row["ida_ea"], 16)
    segment = 9 if ida_ea >= 0x00C00000 else 2
    # Segment 2 is the XBE text image at 0x400000; segment 9's manifest
    # offset zero is the first MP_LELCS entry at 0xC8F720.
    offset = ida_ea - (0x00C8F720 if segment == 9 else 0x00400000)
    return (
        f"{segment:04x}:{offset:08x} {row['name']} "
        f"{row['va'].removeprefix('0x')} {row['flags']} {row['obj']}"
    )


def demangle_names(names: list[str]) -> dict[str, str]:
    """Use the installed MSVC undecorator for shell-format signatures."""
    tool = shutil.which("undname")
    if tool is None:
        candidates = sorted(
            Path(r"C:\Program Files\Microsoft Visual Studio").glob(
                r"**\VC\Tools\MSVC\*\bin\Hostx64\x64\undname.exe"
            )
        )
        tool = str(candidates[-1]) if candidates else ""
    if not tool:
        return {name: name for name in names}

    result: dict[str, str] = {}
    unique = list(dict.fromkeys(names))
    for start in range(0, len(unique), 100):
        batch = unique[start : start + 100]
        try:
            output = subprocess.check_output(
                [tool, *batch], text=True, errors="replace"
            )
        except (OSError, subprocess.CalledProcessError):
            return {name: result.get(name, name) for name in unique}
        current = ""
        for line in output.splitlines():
            if line.startswith("Undecoration of :- \""):
                current = line[len("Undecoration of :- \"") : -1]
            elif line.startswith("is :- \"") and current:
                result[current] = line[len("is :- \"") : -1]
                current = ""
    return {name: result.get(name, name) for name in unique}


def write_shell_table_set(prefix: Path, rows: list[dict[str, str]],
                          signatures: dict[str, str]) -> None:
    """Emit one shell.o-shaped inventory/plan/checklist set."""
    non_inline = [row for row in rows if row["is_inline"] != "1"]
    inventory = prefix.with_name(prefix.name + "_inventory.tsv")
    plan = prefix.with_name(prefix.name + "_plan.tsv")
    checklist = prefix.with_name(prefix.name + "_checklist.tsv")
    with inventory.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(
            stream,
            fieldnames=["ida_ea", "va", "name", "signature", "family"],
            delimiter="\t",
            lineterminator="\n",
        )
        writer.writeheader()
        for row in non_inline:
            writer.writerow(
                {
                    "ida_ea": row["ida_ea"],
                    "va": row["va"],
                    "name": row["name"],
                    "signature": signatures[row["name"]],
                    "family": row.get("source_cpp", "") or row["obj"],
                }
            )

    with plan.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(
            stream,
            fieldnames=["ida_ea", "name", "signature", "source_cpp"],
            delimiter="\t",
            lineterminator="\n",
        )
        writer.writeheader()
        for row in non_inline:
            writer.writerow(
                {
                    "ida_ea": row["ida_ea"],
                    "name": row["name"],
                    "signature": signatures[row["name"]],
                    "source_cpp": row.get("source_cpp", ""),
                }
            )

    with checklist.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(
            stream,
            fieldnames=["index", "status", "map_line"],
            delimiter="\t",
            lineterminator="\n",
        )
        writer.writeheader()
        for index, row in enumerate(rows, 1):
            note = shell_skip_note(row)
            map_line = shell_map_line(row)
            if note:
                map_line += "  " + note
            writer.writerow(
                {
                    "index": index,
                    "status": shell_status(row),
                    "map_line": map_line,
                }
            )


def write_shell_format_tables(game_rows: list[dict[str, str]],
                              manifest_rows: list[dict[str, str]]) -> None:
    """Emit shell.o-shaped tables for the combined and per-object audits.

    The shell.o tables intentionally omit inline functions from inventory/plan,
    while the checklist retains every manifest row.  The manifest map identity
    is retained in each checklist line so rows remain attributable when the
    combined game/engine table is reviewed.
    """
    non_inline = [row for row in game_rows if row["is_inline"] != "1"]
    signatures = demangle_names([row["name"] for row in non_inline])
    write_shell_table_set(ROOT / "analysis" / "game_related", game_rows, signatures)
    object_rows: defaultdict[str, list[dict[str, str]]] = defaultdict(list)
    for row in manifest_rows:
        object_rows[row["obj"]].append(row)
    all_non_inline = [row for row in manifest_rows if row["is_inline"] != "1"]
    all_signatures = demangle_names([row["name"] for row in all_non_inline])
    for obj, rows in sorted(object_rows.items()):
        stem = obj.replace(".", "_")
        write_shell_table_set(OBJECT_TABLE_DIR / stem, rows, all_signatures)


def write_summary(stream, by_obj_counts: defaultdict[str, Counter[str]]) -> None:
    writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
    writer.writerow(["obj", "manifest_rows", "verified", "port_status_only", "external", "in_progress", "unresolved_engine", "untracked"])
    for obj in sorted(by_obj_counts):
        counts = by_obj_counts[obj]
        writer.writerow(
            [
                obj,
                sum(counts.values()),
                counts["VERIFIED"],
                counts["PORT_STATUS_ONLY"],
                counts["EXTERNAL"],
                counts["IN_PROGRESS"],
                counts["UNRESOLVED_ENGINE"],
                counts["UNTRACKED"],
            ]
        )


if __name__ == "__main__":
    main()
