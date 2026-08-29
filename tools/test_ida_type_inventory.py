#!/usr/bin/env python3
"""Regression tests for the IDA type inventory's transport parsing."""

from __future__ import annotations

import json

import ida_type_inventory as inventory


def main() -> int:
    assert inventory.literal_size("0x28") == 0x28
    assert inventory.literal_size(" 40 ") == 40
    assert inventory.literal_size("sizeof(Foo)") is None

    original_post = inventory.post
    try:
        payload = {
            "result": [{
                "data": [{"name": "PhysData", "size": 24}],
                "next_offset": None,
            }]
        }
        inventory.post = lambda request, session_id=None: ({
            "result": {"content": [{
                "type": "text", "text": json.dumps(payload),
            }]}
        }, "test-session")
        page, session_id = inventory.call_type_query(None, 0)
        assert page["data"][0]["name"] == "PhysData"
        assert page["next_offset"] is None
        assert session_id == "test-session"
    finally:
        inventory.post = original_post

    print("ida_type_inventory invariants: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
