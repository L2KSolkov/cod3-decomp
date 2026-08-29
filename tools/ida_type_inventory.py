#!/usr/bin/env python3
"""Build a source-to-IDA UDT layout inventory.

The release IDA database is the authority for type sizes.  This tool queries
the connected IDA MCP instance, then joins those sizes to source-defined UDTs
and the existing compile-time assertions.  It never invents a size when IDA
does not provide one.
"""

from __future__ import annotations

import csv
import json
import re
import urllib.request
from collections import Counter
from pathlib import Path

import type_inventory
import verify_ledger


MCP_URL = "http://127.0.0.1:13337/mcp"
ROOT = Path(__file__).resolve().parents[1]
REPORT = ROOT / "analysis" / "IDA_TYPE_INVENTORY.tsv"


def post(payload: dict, session_id: str | None = None) -> tuple[dict, str | None]:
    headers = {
        "Accept": "application/json, text/event-stream",
        "Content-Type": "application/json",
    }
    if session_id is not None:
        headers["Mcp-Session-Id"] = session_id
    request = urllib.request.Request(
        MCP_URL, data=json.dumps(payload).encode("utf-8"), headers=headers
    )
    with urllib.request.urlopen(request) as response:
        body = response.read().decode("utf-8")
        return json.loads(body), response.headers.get("Mcp-Session-Id")


def call_type_query(session_id: str | None, offset: int) -> tuple[dict, str | None]:
    body, session_id = post({
        "jsonrpc": "2.0",
        "id": offset + 10,
        "method": "tools/call",
        "params": {
            "name": "type_query",
            "arguments": {"queries": {
                "kind": "udt", "count": 100, "offset": offset,
                "include_decl": False, "include_members": False,
            }},
        },
    }, session_id)
    if "error" in body:
        raise RuntimeError(body["error"])
    content = body.get("result", {}).get("content", [])
    text = next((item["text"] for item in content if item.get("type") == "text"), "[]")
    decoded = json.loads(text)
    # The MCP bridge has returned both a bare result list and a wrapped
    # {"result": [...]} payload across versions; accept either shape.
    if isinstance(decoded, dict):
        decoded = decoded.get("result", [])
    return decoded[0], session_id


def ida_types() -> list[dict]:
    body, session_id = post({
        "jsonrpc": "2.0", "id": 1, "method": "initialize",
        "params": {
            "protocolVersion": "2025-03-26", "capabilities": {},
            "clientInfo": {"name": "ida_type_inventory", "version": "1.0"},
        },
    })
    if "error" in body:
        raise RuntimeError(body["error"])
    rows: list[dict] = []
    offset = 0
    while True:
        page, session_id = call_type_query(session_id, offset)
        rows.extend(page.get("data", []))
        next_offset = page.get("next_offset")
        if next_offset is None:
            break
        offset = int(next_offset)
    return rows


def literal_size(value: str) -> int | None:
    compact = value.replace(" ", "")
    if re.fullmatch(r"(?:0x[0-9A-Fa-f]+|[0-9]+)", compact):
        return int(compact, 0)
    return None


def main() -> int:
    ida = ida_types()
    by_name: dict[str, list[dict]] = {}
    for row in ida:
        by_name.setdefault(row.get("name", ""), []).append(row)

    assertions = verify_ledger.type_assertions()
    source_rows = type_inventory.type_definitions()
    source_name_counts = Counter(row["type"] for row in source_rows)
    output: list[dict[str, str]] = []
    for source in source_rows:
        name = source["type"]
        matches = by_name.get(name, [])
        # Prefer an assertion in the same source file.  Falling back to a
        # name-only join is safe only when that name has one source
        # definition; local views commonly reuse names such as Context,
        # PoolAllocator, and XModelManager.
        matching_assertions = [row for row in assertions
                               if row["check"] == "sizeof"
                               and row["source"] == source["source"]
                               and (row["type"] == name
                                    or row["type"].endswith("::" + name)
                                    or row["type"].startswith(name + "<"))]
        ambiguous = source_name_counts[name] > 1
        if not matching_assertions and not ambiguous:
            matching_assertions = [row for row in assertions
                                   if row["check"] == "sizeof"
                                   and (row["type"] == name
                                        or row["type"].endswith("::" + name)
                                        or row["type"].startswith(name + "<"))]
        expected = [literal_size(row["expected"]) for row in matching_assertions]
        expected = [value for value in expected if value is not None]
        ida_sizes = sorted({int(row["size"]) for row in matches
                             if row.get("size") is not None})
        if not matches:
            status = "NO_IDA_TYPE"
            ida_size = ""
        elif not expected:
            status = "MISSING_ASSERT"
            ida_size = ",".join(f"0x{value:X}" for value in ida_sizes)
        elif not ida_sizes:
            status = "IDA_SIZE_UNKNOWN"
            ida_size = ""
        elif all(value in ida_sizes for value in expected):
            status = "MATCHED_ASSERT"
            ida_size = ",".join(f"0x{value:X}" for value in ida_sizes)
        else:
            status = "SIZE_MISMATCH"
            ida_size = ",".join(f"0x{value:X}" for value in ida_sizes)
        if ambiguous and status in {"SIZE_MISMATCH", "MISSING_ASSERT"}:
            status = "AMBIGUOUS_SOURCE_NAME"
        note = ""
        if name == "tlSystemCallbacks" and source["source"].endswith(
                "src\\core\\tl_system.cpp"):
            # IDA has a 0x20 local UDT with the first eight callbacks, while
            # the release global copies 0x28 bytes and dispatches MemAlloc,
            # MemRealloc, and MemFree at +0x1C..+0x24.  The source's ten-slot
            # mirror is therefore retained and explicitly adjudicated rather
            # than silently changed to the incomplete local type.
            status = "ADJUDICATE"
            note = "IDA local UDT is 0x20; release tlSetSystemCallbacks copies 0x28"
        output.append({
            **source,
            "ida_size": ida_size,
            "asserted_sizes": ",".join(f"0x{value:X}" for value in sorted(set(expected))),
            "status": status,
            "note": note,
        })

    REPORT.parent.mkdir(parents=True, exist_ok=True)
    fields = ["kind", "type", "source", "line", "ida_size", "asserted_sizes", "status", "note"]
    with REPORT.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(output)
    print(f"IDA UDTs: {len(ida)}")
    print(f"source definitions: {len(output)}")
    print("status:", " ".join(f"{key}={value}" for key, value in
                              sorted(Counter(row["status"] for row in output).items())))
    print(f"report: {REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
