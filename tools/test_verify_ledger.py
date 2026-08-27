#!/usr/bin/env python3
"""Small invariant tests for the verification ledger's safety rules."""

from __future__ import annotations

import tempfile
from pathlib import Path

import verify_ledger as ledger


def row(result: str, gate: str = "V4", source_ref: str = "test.txt") -> str:
    return "\t".join(("0x0040C000", "fn", gate, result, "self-test", "2026-08-26", source_ref, ""))


def main() -> int:
    original = ledger.EVIDENCE_DIR
    try:
        with tempfile.TemporaryDirectory() as directory:
            ledger.EVIDENCE_DIR = Path(directory)
            path = ledger.EVIDENCE_DIR / "invariants.tsv"
            path.write_text("\t".join(ledger.EVIDENCE_FIELDS) + "\n" + row("FAIL") + "\n", encoding="utf-8")
            evidence, errors = ledger.read_evidence()
            assert not errors, errors
            assert evidence[("0X0040C000", "fn", "V4")]["result"] == "FAIL"

            # A later PASS supersedes the earlier FAIL for the same key; no
            # object/file aggregate can affect an unrelated function key.
            with path.open("a", encoding="utf-8") as stream:
                stream.write(row("PASS") + "\n")
            evidence, errors = ledger.read_evidence()
            assert not errors, errors
            assert evidence[("0X0040C000", "fn", "V4")]["result"] == "PASS"

            function = ledger.Function(0x0040C000, 0, "fn", "f", "", "game.o", 2, 0, False, "game")
            marker = ledger.Marker(0x0040C000, "test.cpp", 1, "fn", "{ return 1; }")
            states, level = ledger.compute_level(function, [marker], {"fn"}, evidence)
            assert states["V4"] == "UNVERIFIED" and level == "V2"

            states, level = ledger.compute_level(
                function, [marker], {"fn"},
                {("0X0040C000", "fn", "V3"): dict(
                    gate="V3", result="FAIL")})
            assert states["V3"] == "FAIL" and level == "V2"
            assert ledger.body_class("{ ; }") == "EMPTY_BODY"

            # Explicit V4 release evidence is sufficient to clear an empty
            # body from the unreviewed-stub anomaly class.
            assert evidence[("0X0040C000", "fn", "V4")]["result"] == "PASS"

            # Definition comments may carry a descriptive function name
            # before the EA; marker parsing must retain that address.
            assert ledger.re.search(
                r"//[^\r\n]*?\bea:\s*(0x[0-9A-Fa-f]+)",
                "// AnimIK::ApplyFootIK - ea: 0x004FB510")

            long_signature = [
                "// ea: 0x005D4CB0",
                "void BrocSys::MPScript_SendGameState(",
                "    unsigned int player, int currentTime, int timeLimit, int scoreLimit,",
                "    int roundLimit, bool friendlyFire, bool lastManStanding, bool teamBalance,",
                "    int respawnTime, int alliesScore, int axisScore, bool roundStarted,",
                "    int roundOver, int roundCount)",
                "{ return; }",
            ]
            candidate = ledger.function_candidate(long_signature, 0)
            assert candidate and candidate[0] == "BrocSys::MPScript_SendGameState"

            inline_ctor = ["// ea: 0x00687A90", "refdef_s() {}"]
            candidate = ledger.function_candidate(inline_ctor, 0)
            assert candidate and candidate[0] == "refdef_s"

            # Release map symbols may carry private member access (AA) while
            # the port exposes the same signature publicly (QA).  Access is
            # not a V2 signature mismatch.
            assert ("?ApplyFootIK@AnimIK@@AAEXPAVEntity@@AAVnalMatrix4x4@@1@Z"
                    in ledger.symbol_variants(
                        "?ApplyFootIK@AnimIK@@QAEXPAVEntity@@AAVnalMatrix4x4@@1@Z"))

            # The VC7 release map's compact repeated-vector spelling and the
            # current MSVC U23 spelling demangle to the same ABI.
            assert ("?MPScript_SendGameStateSD@BrocSys@@YAXIII_NUvector@Broc@@U23@H@Z"
                    in ledger.symbol_variants(
                        "?MPScript_SendGameStateSD@BrocSys@@YAXIII_NUvector@Broc@@1H@Z"))

            # The current MSVC compact bool back-reference is equivalent to
            # the release map's repeated bool spelling for effect exports.
            assert ("?EffectEventPlay@BrocSys@@YAHIABVstring@Broc@@H_N1@Z"
                    in ledger.symbol_variants(
                        "?EffectEventPlay@BrocSys@@YAHIABVstring@Broc@@H_N_N@Z"))

            # Missing source_ref is rejected rather than silently granting a gate.
            path.write_text("\t".join(ledger.EVIDENCE_FIELDS) + "\n" + row("PASS", source_ref="") + "\n", encoding="utf-8")
            _, errors = ledger.read_evidence()
            assert errors and "required evidence fields" in errors[0]
    finally:
        ledger.EVIDENCE_DIR = original
    print("verify_ledger invariants: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
