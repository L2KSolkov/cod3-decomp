#!/usr/bin/env python3
"""Standalone invariants for the type-field audit."""

from __future__ import annotations

import sys

import type_field_audit as audit


def main() -> int:
    assert audit.literal_size("0x38") == 0x38
    assert audit.literal_size("56") == 56
    assert audit.literal_size("sizeof(foo)") is None
    rows = [{"name": "field", "offset": "0x10", "type": "int"}]
    assert rows[0]["offset"] == "0x10"
    print("type_field_audit invariants: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
