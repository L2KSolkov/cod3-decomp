#!/usr/bin/env python3
"""Regression tests for release signature extraction used by the V3 sweep."""

from __future__ import annotations

import tempfile
from pathlib import Path

import release_stub_audit as audit


def main() -> int:
    original = audit.RELEASE_C
    try:
        with tempfile.TemporaryDirectory() as directory:
            audit.RELEASE_C = Path(directory) / "release.c"
            audit.RELEASE_C.write_text(
                "\n".join(
                    (
                        "//----- (00448920) --------------------------------------------------------",
                        "Color *__thiscall Color::Color(Color *this)",
                        "{",
                        "  return this;",
                        "}",
                        "//----- (004B1590) --------------------------------------------------------",
                        "TaskFunctor1<XAnimUpdateTask,float> *__thiscall",
                        "TaskFunctor1<XAnimUpdateTask,float>::TaskFunctor1<XAnimUpdateTask,float>(",
                        "        TaskFunctor1<XAnimUpdateTask,float> *this)",
                        "{",
                        "  return this;",
                        "}",
                    )
                ),
                encoding="utf-8",
            )
            functions = audit.release_functions()
            assert functions[0x00448920][0] == "Color::Color"
            assert functions[0x004B1590][0] == (
                "TaskFunctor1<XAnimUpdateTask,float>::"
                "TaskFunctor1<XAnimUpdateTask,float>"
            )
            assert audit.trivial_return("{ return nullptr; }")
            assert audit.trivial_return("{ return 0; }")
            assert audit.trivial_return("{ return 1; }")
            assert not audit.trivial_return("{ return value; }")
    finally:
        audit.RELEASE_C = original
    print("release_stub_audit invariants: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
