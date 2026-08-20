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
        row_class = row.get("class", "")
        if row_class in {"xdk", "crt"}:
            verification = "EXTERNAL"
        elif status in {"PORTED", "COMPLETE", "VERIFIED", "VALIDATED"} and symbol == "YES":
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

    print(f"manifest rows: {len(rows)}")
    print(f"debug libraries: {len(list(BUILD.glob('*.lib')))}")
    for verification, count in Counter(row["verification"] for row in rows).most_common():
        print(f"{verification}: {count}")
    print(f"wrote {OUT}")
    print(f"wrote {SUMMARY}")
    print(f"game-related rows: {len(game_rows)}")
    print(f"wrote {GAME_OUT}")
    print(f"wrote {GAME_SUMMARY}")


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
