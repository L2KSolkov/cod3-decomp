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

            # Independent audit rows are valid evidence but do not advance
            # the monotonic V0..V5 function level.
            with path.open("a", encoding="utf-8") as stream:
                stream.write(row("PASS", gate="AUDIT") + "\n")
            evidence, errors = ledger.read_evidence()
            assert not errors, errors
            assert evidence[("0X0040C000", "fn", "AUDIT")]["result"] == "PASS"

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
            assert ledger.base_name("??$IsDefined@I@@YA_NI@Z") == "IsDefined"
            assert ledger.base_name("??2AeThreadState@@SAPAXI_NPBDH@Z") == "operator new"
            assert ledger.base_name("??4_objectiveInfo_t@@QAEAAU0@ABU0@@Z") == "operator="

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

            operator_new = ["// AeThreadState::operator new - ea: 0x005E9430",
                            "void* AeThreadState::operator new(size_t size, bool forceHeapAlloc)",
                            "{ return sAllocator->Allocate((unsigned int)size, forceHeapAlloc); }"]
            candidate = ledger.function_candidate(operator_new, 0)
            assert candidate and candidate[0] == "AeThreadState::operator new"

            array_return = ["// InteractionController::GetHandsAngles - ea: 0x006BB6C0",
                            "const float (&InteractionController::GetHandsAngles() const)[3]",
                            "{ return mHandsAngles; }"]
            candidate = ledger.function_candidate(array_return, 0)
            assert candidate and candidate[0] == "InteractionController::GetHandsAngles"

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

            assert ("?AddScriptEvent@Entity@@QAE_NVHashString@@0@Z"
                    in ledger.symbol_variants(
                        "?AddScriptEvent@Entity@@QAE_NVHashString@@V2@@Z"))

            assert ("?MathFastSinCos@Broc@@YAXMPAM0@Z"
                    in ledger.symbol_variants(
                        "?MathFastSinCos@Broc@@YAXMAAM0@Z"))
            assert ("?VecAnglesToUp@Broc@@YAXPAUvector@1@PBU21@@Z"
                    in ledger.symbol_variants(
                        "?VecAnglesToUp@Broc@@YAXAAUvector@1@ABU21@@Z"))
            assert ("?AddEndOn@EntityNotifySet@@QAEXPAUEndOnScriptNode@@@Z"
                    in ledger.symbol_variants(
                        "?AddEndOn@EntityNotifySet@@QAEXPAVEndOnScriptNode@@@Z"))
            assert ("?MPScript_Obituary@BrocSys@@YAXIIPBVstring@Broc@@H_N@Z"
                    in ledger.symbol_variants(
                        "?MPScript_Obituary@BrocSys@@YAXIIABVstring@Broc@@H_N@Z"))
            assert ("??2AeThreadState@@SAPAXI_N@Z"
                    in ledger.symbol_variants("??2AeThreadState@@SAPAXI_NPBDH@Z"))
            assert ("?GetOwner@AeThread@@QBE?AV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@XZ"
                    in ledger.symbol_variants(
                        "?GetOwner@AeThread@@QAE?AV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@XZ"))
            assert ("?CG_DrawFriendlyFire@@YAPAVEntity@@XZ"
                    in ledger.symbol_variants("?CG_DrawFriendlyFire@@YAXXZ"))
            assert ("?CG_StartAmbient@@YAHXZ"
                    in ledger.symbol_variants("?CG_StartAmbient@@YAXXZ"))
            assert ("?CG_Argv@@YAPADH@Z"
                    in ledger.symbol_variants("?CG_Argv@@YAPBDH@Z"))
            assert ("?CG_Asset_Parse@@YAHXZ"
                    in ledger.symbol_variants("?CG_Asset_Parse@@YAHHH@Z"))
            assert ("?CG_LoadMenus@@YAXXZ"
                    in ledger.symbol_variants("?CG_LoadMenus@@YAXPBDH@Z"))
            assert ("?CG_CheckAmmo@@YAHXZ"
                    in ledger.symbol_variants("?CG_CheckAmmo@@YAXXZ"))
            assert ("?CG_DObjCalcPose@@YAXPAVEntity@@PAVDObj@@PAH@Z"
                    in ledger.symbol_variants(
                        "?CG_DObjCalcPose@@YAXPAVEntity@@PAVDObj@@QAH@Z"))
            assert ("?CG_DObjGetWorldTagMatrix@@YAPBUDObjSkelMat@@PAVEntity@@PAVDObj@@IPAU1@@Z"
                    in ledger.symbol_variants(
                        "?CG_DObjGetWorldTagMatrix@@YAHPAVEntity@@PAVDObj@@IPAUDObjSkelMat@@@Z"))
            assert ("?CG_Respawn@@YAHXZ"
                    in ledger.symbol_variants("?CG_Respawn@@YAXXZ"))
            assert ("?CG_HoldBreathInit@@YAHXZ"
                    in ledger.symbol_variants("?CG_HoldBreathInit@@YAXXZ"))
            assert ("?CG_DrawPerformanceWarnings@@YAHXZ"
                    in ledger.symbol_variants(
                        "?CG_DrawPerformanceWarnings@@YAXXZ"))
            assert ("?CG_WeaponFireRecoil@@YAHXZ"
                    in ledger.symbol_variants("?CG_WeaponFireRecoil@@YAXXZ"))
            assert ("?CG_WeaponSelectable@@YA_NH@Z"
                    in ledger.symbol_variants("?CG_WeaponSelectable@@YAHH@Z"))
            assert ("?CG_Weapon_f@@YAHXZ"
                    in ledger.symbol_variants("?CG_Weapon_f@@YAXXZ"))
            assert ("?CG_WeaponSlot_f@@YAPAUClient@@XZ"
                    in ledger.symbol_variants("?CG_WeaponSlot_f@@YAXXZ"))

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
