#!/usr/bin/env python3
"""Compare source offsetof assertions with release IDA member layouts.

This report is intentionally narrow.  It proves only what the source already
asserts: a named member exists in the release UDT at the asserted offset.  It
does not infer missing fields or treat a name-only match as proof when IDA has
no corresponding type.
"""

from __future__ import annotations

import csv
import json
import re
import urllib.request
from collections import Counter
from pathlib import Path

import verify_ledger


MCP_URL = "http://127.0.0.1:13337/mcp"
ROOT = Path(__file__).resolve().parents[1]
REPORT = ROOT / "analysis" / "TYPE_FIELD_AUDIT.tsv"


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
    with urllib.request.urlopen(request, timeout=60) as response:
        return json.loads(response.read().decode("utf-8")), response.headers.get(
            "Mcp-Session-Id"
        )


def inspect_types(names: list[str]) -> dict[str, dict]:
    if not names:
        return {}
    body, session_id = post({
        "jsonrpc": "2.0", "id": 1, "method": "initialize",
        "params": {
            "protocolVersion": "2025-03-26", "capabilities": {},
            "clientInfo": {"name": "type_field_audit", "version": "1.0"},
        },
    })
    if "error" in body:
        raise RuntimeError(body["error"])
    result: dict[str, dict] = {}
    for start in range(0, len(names), 50):
        queries = [
            {"name": name, "include_members": True, "max_members": 4096}
            for name in names[start:start + 50]
        ]
        body, session_id = post({
            "jsonrpc": "2.0", "id": start + 2, "method": "tools/call",
            "params": {"name": "type_inspect", "arguments": {"queries": queries}},
        }, session_id)
        if "error" in body:
            raise RuntimeError(body["error"])
        content = body.get("result", {}).get("content", [])
        text = next((item["text"] for item in content if item.get("type") == "text"), "[]")
        decoded = json.loads(text)
        if isinstance(decoded, dict):
            decoded = decoded.get("result", [])
        for row in decoded:
            if row.get("name"):
                result[row["name"]] = row
    return result


def literal_size(value: str) -> int | None:
    compact = value.replace(" ", "")
    if re.fullmatch(r"(?:0x[0-9A-Fa-f]+|[0-9]+)", compact):
        return int(compact, 0)
    return None


def matching_assertions() -> list[dict[str, str]]:
    return [row for row in verify_ledger.type_assertions()
            if row["check"] == "offsetof"]


def audit() -> list[dict[str, str]]:
    assertions = matching_assertions()
    names = sorted({row["type"] for row in assertions})
    ida = inspect_types(names)
    rows: list[dict[str, str]] = []
    for assertion in assertions:
        type_name = assertion["type"]
        field_name = assertion["field"]
        expected = literal_size(assertion["expected"])
        type_row = ida.get(type_name)
        members = {
            member.get("name"): member
            for member in ((type_row or {}).get("members") or [])
            if member.get("name")
        }
        member = members.get(field_name)
        if type_row is None or not type_row.get("exists"):
            status = "IDA_TYPE_MISSING"
            ida_offset = ""
            ida_type = ""
        elif expected is None:
            status = "NON_LITERAL_ASSERT"
            ida_offset = member.get("offset", "") if member else ""
            ida_type = member.get("type", "") if member else ""
        elif member is None:
            status = "MISSING_MEMBER"
            ida_offset = ""
            ida_type = ""
        else:
            ida_offset = member.get("offset", "")
            ida_type = member.get("type", "")
            try:
                actual = int(ida_offset, 0)
            except (TypeError, ValueError):
                actual = None
            status = "MATCHED_OFFSET" if actual == expected else "OFFSET_MISMATCH"
        rows.append({
            **assertion,
            "ida_offset": ida_offset,
            "ida_type": ida_type,
            "status": status,
        })
    return rows


def write_report(rows: list[dict[str, str]]) -> None:
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    fields = ["type", "field", "expected", "ida_offset", "ida_type",
              "source", "line", "status"]
    with REPORT.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t",
                                lineterminator="\n")
        writer.writeheader()
        writer.writerows({field: row.get(field, "") for field in fields}
                         for row in rows)


def main() -> int:
    rows = audit()
    write_report(rows)
    print(f"field assertions: {len(rows)}")
    print("status:", " ".join(f"{key}={value}" for key, value in
                               sorted(Counter(row["status"] for row in rows).items())))
    print(f"report: {REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
