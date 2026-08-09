"""Generate game2.o weaponFuncs helpers + full weaponFileInfo_t layout.

Source of truth: analysis/MANIFEST.tsv (game2.o entries) + IDA local type
weaponFileInfo_t (0x948 bytes). Each helper follows the uniform release-build
pattern verified by decompiling reticleCenterSize/duckedOfsF/adsSpread/etc:

    int weaponFuncs::X_Function(int v) {
        weaponFileInfo_t* p = BG_GetPlayerWeaponInfo();
        if (!p) return 0;
        int r = v + p->FIELD; p->FIELD = r; return r;
    }

Float variants take `float v` and return `double` (x87 ABI).
"""

import re

MANIFEST = r"analysis\MANIFEST.tsv"

# weaponFileInfo_t members in offset order (name, offset, ctype, size)
# From IDA local type dump; full 0x948 layout.
MEMBERS = [
    ("index", 0x000, "int", 4),
    ("internalNameHash", 0x004, "unsigned int", 4),
    ("szInternalName", 0x008, "char*", 4),
    ("szDisplayName", 0x00C, "char*", 4),
    ("szOverlayName", 0x010, "char*", 4),
    ("szGunXModel", 0x014, "char*", 4),
    ("szHandXModel", 0x018, "char*", 4),
    ("szAttachModel1", 0x01C, "char*", 4),
    ("szAttachModel2", 0x020, "char*", 4),
    ("szAttachModel3", 0x024, "char*", 4),
    ("szAttachModel4", 0x028, "char*", 4),
    ("szAttachModel5", 0x02C, "char*", 4),
    ("szAttachTag1", 0x030, "char*", 4),
    ("szAttachTag2", 0x034, "char*", 4),
    ("szAttachTag3", 0x038, "char*", 4),
    ("szAttachTag4", 0x03C, "char*", 4),
    ("szAttachTag5", 0x040, "char*", 4),
    ("szXAnims", 0x044, "char*", 0x64),          # char*[25]
    ("szModeName", 0x0A8, "char*", 4),
    ("type", 0x0AC, "int", 4),
    ("weapClass", 0x0B0, "int", 4),
    ("slot", 0x0B4, "int", 4),
    ("bSlotStackable", 0x0B8, "int", 4),
    ("stance", 0x0BC, "int", 4),
    ("ammoType", 0x0C0, "int", 4),
    ("pickupWithoutSelect", 0x0C4, "int", 4),
    ("mFireSoundMatches", 0x0C8, "unsigned char", 0x208),   # ae_sized_array x2
    ("mFireParticleMatches", 0x2D0, "unsigned char", 0x208),
    ("szReticleCenter", 0x4D8, "char*", 4),
    ("szReticleSide", 0x4DC, "char*", 4),
    ("iReticleCenterSize", 0x4E0, "int", 4),
    ("iReticleSideSize", 0x4E4, "int", 4),
    ("iReticleMinOfs", 0x4E8, "int", 4),
    ("vSprintMove", 0x4EC, "float", 0x0C),
    ("vSprintRot", 0x4F8, "float", 0x0C),
    ("vStandMove", 0x504, "float", 0x0C),
    ("vStandRot", 0x510, "float", 0x0C),
    ("vDuckedOfs", 0x51C, "float", 0x0C),
    ("vDuckedMove", 0x528, "float", 0x0C),
    ("vDuckedRot", 0x534, "float", 0x0C),
    ("vProneOfs", 0x540, "float", 0x0C),
    ("vProneMove", 0x54C, "float", 0x0C),
    ("vProneRot", 0x558, "float", 0x0C),
    ("fPosMoveRate", 0x564, "float", 4),
    ("fPosProneMoveRate", 0x568, "float", 4),
    ("fSprintMoveMinSpeed", 0x56C, "float", 4),
    ("fStandMoveMinSpeed", 0x570, "float", 4),
    ("fDuckedMoveMinSpeed", 0x574, "float", 4),
    ("fProneMoveMinSpeed", 0x578, "float", 4),
    ("fPosRotRate", 0x57C, "float", 4),
    ("fPosProneRotRate", 0x580, "float", 4),
    ("fSprintRotMinSpeed", 0x584, "float", 4),
    ("fStandRotMinSpeed", 0x588, "float", 4),
    ("fDuckedRotMinSpeed", 0x58C, "float", 4),
    ("fProneRotMinSpeed", 0x590, "float", 4),
    ("szRadiantName", 0x594, "char*", 4),
    ("szWorldModel", 0x598, "char*", 4),
    ("szPickupModel", 0x59C, "char*", 4),
    ("szHudIcon", 0x5A0, "char*", 4),
    ("szModeIcon", 0x5A4, "char*", 4),
    ("szAmmoIcon", 0x5A8, "char*", 4),
    ("iStartAmmo", 0x5AC, "int", 4),
    ("szAmmoName", 0x5B0, "char*", 4),
    ("iAmmoIndex", 0x5B4, "int", 4),
    ("szClipName", 0x5B8, "char*", 4),
    ("iClipIndex", 0x5BC, "int", 4),
    ("iMaxAmmo", 0x5C0, "int", 4),
    ("iClipSize", 0x5C4, "int", 4),
    ("szSharedAmmoCapName", 0x5C8, "char*", 4),
    ("iSharedAmmoCapIndex", 0x5CC, "int", 4),
    ("iSharedAmmoCap", 0x5D0, "int", 4),
    ("iDamage", 0x5D4, "int", 4),
    ("iTakeDamage", 0x5D8, "int", 4),
    ("iMinDamagePercent", 0x5DC, "int", 4),
    ("iDamageInnerRadius", 0x5E0, "int", 4),
    ("iDamageOuterRadius", 0x5E4, "int", 4),
    ("iMeleeDamage", 0x5E8, "int", 4),
    ("iDamageType", 0x5EC, "int", 4),
    ("iFireDelay", 0x5F0, "int", 4),
    ("iMeleeDelay", 0x5F4, "int", 4),
    ("iFireTime", 0x5F8, "int", 4),
    ("iRechamberTime", 0x5FC, "int", 4),
    ("iRechamberBoltTime", 0x600, "int", 4),
    ("iHoldFireTime", 0x604, "int", 4),
    ("iMeleeTime", 0x608, "int", 4),
    ("iReloadTime", 0x60C, "int", 4),
    ("iReloadEmptyTime", 0x610, "int", 4),
    ("iReloadAddTime", 0x614, "int", 4),
    ("iReloadStartTime", 0x618, "int", 4),
    ("iReloadStartAddTime", 0x61C, "int", 4),
    ("iReloadEndTime", 0x620, "int", 4),
    ("iDropTime", 0x624, "int", 4),
    ("iRaiseTime", 0x628, "int", 4),
    ("iAltDropTime", 0x62C, "int", 4),
    ("iAltRaiseTime", 0x630, "int", 4),
    ("iFuseTime", 0x634, "int", 4),
    ("fSensitivityScale", 0x638, "float", 4),
    ("fMoveSpeedScale", 0x63C, "float", 4),
    ("fAdsZoomFov", 0x640, "float", 4),
    ("fAdsSensitivityScale", 0x644, "float", 4),
    ("fAdsZoomInFrac", 0x648, "float", 4),
    ("fAdsZoomOutFrac", 0x64C, "float", 4),
    ("szOverlayShader", 0x650, "char*", 4),
    ("overlayReticle", 0x654, "int", 4),
    ("fOverlayWidth", 0x658, "float", 4),
    ("fOverlayHeight", 0x65C, "float", 4),
    ("fAdsBobFactor", 0x660, "float", 4),
    ("fAdsViewBobMult", 0x664, "float", 4),
    ("fHipSpreadStandMin", 0x668, "float", 4),
    ("fHipSpreadDuckedMin", 0x66C, "float", 4),
    ("fHipSpreadProneMin", 0x670, "float", 4),
    ("fHipSpreadMax", 0x674, "float", 4),
    ("fHipSpreadDecayRate", 0x678, "float", 4),
    ("fHipSpreadFireAdd", 0x67C, "float", 4),
    ("fHipSpreadTurnAdd", 0x680, "float", 4),
    ("fHipSpreadMoveAdd", 0x684, "float", 4),
    ("fHipSpreadDuckedDecay", 0x688, "float", 4),
    ("fHipSpreadProneDecay", 0x68C, "float", 4),
    ("fHipReticleSidePos", 0x690, "float", 4),
    ("iAdsTransInTime", 0x694, "int", 4),
    ("iAdsTransOutTime", 0x698, "int", 4),
    ("fAdsIdleAmount", 0x69C, "float", 4),
    ("fHipIdleAmount", 0x6A0, "float", 4),
    ("fIdleCrouchFactor", 0x6A4, "float", 4),
    ("fIdleProneFactor", 0x6A8, "float", 4),
    ("fGunMaxPitch", 0x6AC, "float", 4),
    ("fGunMaxYaw", 0x6B0, "float", 4),
    ("swayMaxAngle", 0x6B4, "float", 4),
    ("swayLerpSpeed", 0x6B8, "float", 4),
    ("swayPitchScale", 0x6BC, "float", 4),
    ("swayYawScale", 0x6C0, "float", 4),
    ("swayHorizScale", 0x6C4, "float", 4),
    ("swayVertScale", 0x6C8, "float", 4),
    ("swayShellShockScale", 0x6CC, "float", 4),
    ("adsSwayMaxAngle", 0x6D0, "float", 4),
    ("adsSwayLerpSpeed", 0x6D4, "float", 4),
    ("adsSwayPitchScale", 0x6D8, "float", 4),
    ("adsSwayYawScale", 0x6DC, "float", 4),
    ("adsSwayHorizScale", 0x6E0, "float", 4),
    ("adsSwayVertScale", 0x6E4, "float", 4),
    ("bTwoHanded", 0x6E8, "int", 4),
    ("bRifleBullet", 0x6EC, "int", 4),
    ("bSemiAuto", 0x6F0, "int", 4),
    ("bBoltAction", 0x6F4, "int", 4),
    ("bADSPositionInfo", 0x6F8, "int", 4),
    ("bRechamberWhileAds", 0x6FC, "int", 4),
    ("bCookOffHold", 0x700, "int", 4),
    ("bNoBounce", 0x704, "int", 4),
    ("bNoTumble", 0x708, "int", 4),
    ("bCanMantle", 0x70C, "int", 4),
    ("bSmoke", 0x710, "int", 4),
    ("bOffHand", 0x714, "int", 4),
    ("bCloth", 0x718, "int", 4),
    ("bClipOnly", 0x71C, "int", 4),
    ("bWideListIcon", 0x720, "int", 4),
    ("bADSFire", 0x724, "int", 4),
    ("bADSOnly", 0x728, "int", 4),
    ("bAnimateCamReload", 0x72C, "int", 4),
    ("bAnimateCamMelee", 0x730, "int", 4),
    ("bAnimateCamFire", 0x734, "int", 4),
    ("bDoNotDrop", 0x738, "int", 4),
    ("bCanSpot", 0x73C, "int", 4),
    ("bHoldToFire", 0x740, "int", 4),
    ("szKillIcon", 0x744, "char*", 4),
    ("bWideKillIcon", 0x748, "int", 4),
    ("bNoPartialReload", 0x74C, "int", 4),
    ("bSegmentedReload", 0x750, "int", 4),
    ("iReloadAmmoAdd", 0x754, "int", 4),
    ("iReloadStartAdd", 0x758, "int", 4),
    ("bSwirlControl", 0x75C, "int", 4),
    ("szAltWeaponName", 0x760, "char*", 4),
    ("iAltWeaponIndex", 0x764, "int", 4),
    ("iShotCount", 0x768, "int", 4),
    ("iDropAmmoMin", 0x76C, "int", 4),
    ("iDropAmmoMax", 0x770, "int", 4),
    ("iTriggerRadius", 0x774, "int", 4),
    ("iExplosionRadius", 0x778, "int", 4),
    ("iExplosionInnerDamage", 0x77C, "int", 4),
    ("iExplosionOuterDamage", 0x780, "int", 4),
    ("iProjectileSpeed", 0x784, "int", 4),
    ("iProjectileSpeedUp", 0x788, "int", 4),
    ("szProjectileModel", 0x78C, "char*", 4),
    ("projExplosion", 0x790, "int", 4),
    ("szProjExplosionEffect", 0x794, "char*", 4),
    ("szProjExplosionSound", 0x798, "char*", 4),
    ("bProjImpactExplode", 0x79C, "int", 4),
    ("lobWeapon", 0x7A0, "int", 4),
    ("iProjectileCount", 0x7A4, "int", 4),
    ("iProjectileRadius", 0x7A8, "int", 4),
    ("iProjectileDelay", 0x7AC, "int", 4),
    ("iProjectileSpacingMin", 0x7B0, "int", 4),
    ("iProjectileSpacingMax", 0x7B4, "int", 4),
    ("iProjectileDLight", 0x7B8, "int", 4),
    ("vProjectileColor", 0x7BC, "float", 0x0C),
    ("fAdsAimPitch", 0x7C8, "float", 4),
    ("fAdsCrosshairInFrac", 0x7CC, "float", 4),
    ("fAdsCrosshairOutFrac", 0x7D0, "float", 4),
    ("fAdsGunKickPitchMin", 0x7D4, "float", 4),
    ("fAdsGunKickPitchMax", 0x7D8, "float", 4),
    ("fAdsGunKickYawMin", 0x7DC, "float", 4),
    ("fAdsGunKickYawMax", 0x7E0, "float", 4),
    ("fAdsGunKickAccel", 0x7E4, "float", 4),
    ("fAdsGunKickSpeedMax", 0x7E8, "float", 4),
    ("fAdsGunKickSpeedDecay", 0x7EC, "float", 4),
    ("fAdsGunKickStaticDecay", 0x7F0, "float", 4),
    ("fAdsViewKickPitchMin", 0x7F4, "float", 4),
    ("fAdsViewKickPitchMax", 0x7F8, "float", 4),
    ("fAdsViewKickYawMin", 0x7FC, "float", 4),
    ("fAdsViewKickYawMax", 0x800, "float", 4),
    ("fAdsViewKickCenterSpeed", 0x804, "float", 4),
    ("fAdsViewScatterMin", 0x808, "float", 4),
    ("fAdsViewScatterMax", 0x80C, "float", 4),
    ("fAdsSpread", 0x810, "float", 4),
    ("fAdsSpreadDucked", 0x814, "float", 4),
    ("fAdsSpreadProne", 0x818, "float", 4),
    ("fHipGunKickPitchMin", 0x81C, "float", 4),
    ("fHipGunKickPitchMax", 0x820, "float", 4),
    ("fHipGunKickYawMin", 0x824, "float", 4),
    ("fHipGunKickYawMax", 0x828, "float", 4),
    ("fHipGunKickAccel", 0x82C, "float", 4),
    ("fHipGunKickSpeedMax", 0x830, "float", 4),
    ("fHipGunKickSpeedDecay", 0x834, "float", 4),
    ("fHipGunKickStaticDecay", 0x838, "float", 4),
    ("fHipViewKickPitchMin", 0x83C, "float", 4),
    ("fHipViewKickPitchMax", 0x840, "float", 4),
    ("fHipViewKickYawMin", 0x844, "float", 4),
    ("fHipViewKickYawMax", 0x848, "float", 4),
    ("fHipViewKickCenterSpeed", 0x84C, "float", 4),
    ("fHipViewScatterMin", 0x850, "float", 4),
    ("fHipViewScatterMax", 0x854, "float", 4),
    ("aiEffectiveRange", 0x858, "float", 4),
    ("aiMissRange", 0x85C, "float", 4),
    ("aiDamageMod", 0x860, "float", 4),
    ("iPositionReloadTransTime", 0x864, "int", 4),
    ("iPositionTransBlendTime", 0x868, "int", 4),
    ("leftArc", 0x86C, "float", 4),
    ("rightArc", 0x870, "float", 4),
    ("topArc", 0x874, "float", 4),
    ("bottomArc", 0x878, "float", 4),
    ("accuracy", 0x87C, "float", 4),
    ("turnSpeed", 0x880, "float", 0x08),
    ("convergenceTime", 0x888, "float", 4),
    ("maxRange", 0x88C, "float", 4),
    ("fAnimHorRotateInc", 0x890, "float", 4),
    ("fPlayerPositionDist", 0x894, "float", 4),
    ("szUseHintString", 0x898, "char*", 4),
    ("iUseHintStringIndex", 0x89C, "int", 4),
    ("fTurretFov", 0x8A0, "float", 4),
    ("fFireHeat", 0x8A4, "float", 4),
    ("fCooldownRate", 0x8A8, "float", 4),
    ("horizViewJitter", 0x8AC, "float", 4),
    ("vertViewJitter", 0x8B0, "float", 4),
    ("fBulletConeAngle", 0x8B4, "float", 4),
    ("fAdsBulletConeAngle", 0x8B8, "float", 4),
    ("szScript", 0x8BC, "char*", 4),
    ("fOOPosAnimLength", 0x8C0, "float", 0x08),
    ("fAnimIKOffsetTime", 0x8C8, "float", 4),
    ("fAnimIKOffsetForce", 0x8CC, "float", 4),
    ("fAnimIKOffsetDist", 0x8D0, "float", 4),
    ("fAnimIKPitchTime", 0x8D4, "float", 4),
    ("fAnimIKPitchForce", 0x8D8, "float", 4),
    ("fAnimIKPitchAngle", 0x8DC, "float", 4),
    ("fAnimIKTorsoRecoilPitchTime", 0x8E0, "float", 4),
    ("fAnimIKTorsoRecoilPitchForce", 0x8E4, "float", 4),
    ("fAnimIKTorsoRecoilPitchAngle", 0x8E8, "float", 4),
    ("pDecals", 0x8EC, "void*", 0x5C),        # gdDecal*[23]
]


def member_expr(member):
    """C++ member declaration with exact size; arrays use [N]."""
    name, off, ctype, size = member
    if size == 4:
        return "    %s %s;  // +0x%03X\n" % (ctype, name, off)
    n = size // 4 if ctype in ("int", "float", "char*", "unsigned int", "void*") else size
    if ctype == "unsigned char":
        return "    unsigned char %s[0x%X];  // +0x%03X\n" % (name, size, off)
    return "    %s %s[%d];  // +0x%03X\n" % (ctype, name, n, off)


def gen_header():
    out = []
    out.append("// ============================================================================")
    out.append("// g_weaponfuncs.h - weaponFuncs stat helpers (game2.o)")
    out.append("// weaponFileInfo_t is the full 0x948 IDA layout (layout twin of the g.o subset")
    out.append("// in g_local.h; never include both in one TU).")
    out.append("// ============================================================================")
    out.append("")
    out.append("#pragma once")
    out.append("")
    out.append("#include <stddef.h>")
    out.append("")
    out.append("struct weaponFileInfo_t {")
    for m in MEMBERS:
        out.append(member_expr(m).rstrip("\n"))
    out.append("};")
    out.append("static_assert(sizeof(weaponFileInfo_t) == 0x948, "
               '"weaponFileInfo_t size mismatch");')
    out.append("")
    out.append("weaponFileInfo_t* BG_GetPlayerWeaponInfo();  // game.o 0xA1E6B0")
    out.append("")
    out.append("namespace weaponFuncs {")
    out.append("")
    # Declarations for every helper (int and float variants).
    seen = set()
    for line in open(MANIFEST, encoding="utf-8"):
        parts = line.rstrip("\n").split("\t")
        if len(parts) < 8 or parts[0] == "ida_ea":
            continue
        if parts[3] != "f" or parts[5] != "game2.o":
            continue
        name = parts[2]
        if not name.startswith("?") or "weaponFuncs" not in name:
            continue
        fn = name.split("@")[0][1:]
        if fn in seen:
            continue
        seen.add(fn)
        is_float = "YAMM" in name
        if is_float:
            out.append("double %s(float v);" % fn)
        else:
            out.append("int %s(int v);" % fn)
    out.append("")
    out.append("} // namespace weaponFuncs")
    out.append("")
    return "\n".join(out)


INT_FIELDS = set()
FLOAT_FIELDS = set()
ARRAY_FIELDS = {
    "vSprintMove": 3, "vSprintRot": 3, "vStandMove": 3, "vStandRot": 3,
    "vDuckedOfs": 3, "vDuckedMove": 3, "vDuckedRot": 3,
    "vProneOfs": 3, "vProneMove": 3, "vProneRot": 3,
}


def field_for_func(fname):
    """Map weaponFuncs helper name -> (field, is_array, array_index)."""
    stem = fname
    if stem.endswith("_Function"):
        stem = stem[:-len("_Function")]
    # Array families with F/R/U (forward/right/up) and P/Y/R (pitch/yaw/roll)
    arr_families = {
        "duckedOfs": "vDuckedOfs",
        "proneOfs": "vProneOfs",
        "standMove": "vStandMove",
        "duckedMove": "vDuckedMove",
        "proneMove": "vProneMove",
        "proneRot": "vProneRot",
    }
    for fstem, field in arr_families.items():
        for idx, suf in enumerate(("F", "R", "U")):
            if stem == fstem + suf:
                return field, idx
        for idx, suf in enumerate(("P", "Y", "R")):
            if stem == fstem + suf:
                return field, idx
    # direct scalar fields
    scalars = {
        "reticleCenterSize": "iReticleCenterSize",
        "reticleSideSize": "iReticleSideSize",
        "reticleMinOfs": "iReticleMinOfs",
        "standMoveMinSpeed": "fStandMoveMinSpeed",
        "duckedMoveMinSpeed": "fDuckedMoveMinSpeed",
        "proneMoveMinSpeed": "fProneMoveMinSpeed",
        "posProneRotRate": "fPosProneRotRate",
        "proneRotMinSpeed": "fProneRotMinSpeed",
        "damage": "iDamage",
        "meleeDamage": "iMeleeDamage",
        "sensitivityScale": "fSensitivityScale",
        "damageInnerRadius": "iDamageInnerRadius",
        "damageOuterRadius": "iDamageOuterRadius",
        "minDamagePercent": "iMinDamagePercent",
        "fireDelay": "iFireDelay",
        "meleeDelay": "iMeleeDelay",
        "fireTime": "iFireTime",
        "rechamberTime": "iRechamberTime",
        "rechamberBoltTime": "iRechamberBoltTime",
        "holdFireTime": "iHoldFireTime",
        "meleeTime": "iMeleeTime",
        "reloadTime": "iReloadTime",
        "reloadEmptyTime": "iReloadEmptyTime",
        "reloadAddTime": "iReloadAddTime",
        "reloadStartTime": "iReloadStartTime",
        "reloadStartAddTime": "iReloadStartAddTime",
        "reloadEndTime": "iReloadEndTime",
        "dropTime": "iDropTime",
        "raiseTime": "iRaiseTime",
        "altDropTime": "iAltDropTime",
        "altRaiseTime": "iAltRaiseTime",
        "fuseTime": "iFuseTime",
        "moveSpeedScale": "fMoveSpeedScale",
        "gunMaxPitch": "fGunMaxPitch",
        "gunMaxYaw": "fGunMaxYaw",
        "swayMaxAngle": "swayMaxAngle",
        "swayLerpSpeed": "swayLerpSpeed",
        "swayPitchScale": "swayPitchScale",
        "swayYawScale": "swayYawScale",
        "swayHorizScale": "swayHorizScale",
        "swayVertScale": "swayVertScale",
        "swayShellShockScale": "swayShellShockScale",
        "adsSwayMaxAngle": "adsSwayMaxAngle",
        "adsSwayLerpSpeed": "adsSwayLerpSpeed",
        "adsSwayPitchScale": "adsSwayPitchScale",
        "adsSwayYawScale": "adsSwayYawScale",
        "adsSwayHorizScale": "adsSwayHorizScale",
        "adsSwayVertScale": "adsSwayVertScale",
        "takedamage": "iTakeDamage",
        "explosionRadius": "iExplosionRadius",
        "explosionInnerDamage": "iExplosionInnerDamage",
        "explosionOuterDamage": "iExplosionOuterDamage",
        "projectileSpeed": "iProjectileSpeed",
        "projectileSpeedUp": "iProjectileSpeedUp",
        "triggerRadius": "iTriggerRadius",
        "adsTransInTime": "iAdsTransInTime",
        "adsTransOutTime": "iAdsTransOutTime",
        "adsIdleAmount": "fAdsIdleAmount",
        "adsZoomFov": "fAdsZoomFov",
        "adsSensitivityScale": "fAdsSensitivityScale",
        "adsZoomInFrac": "fAdsZoomInFrac",
        "adsZoomOutFrac": "fAdsZoomOutFrac",
        "adsOverlayWidth": "fOverlayWidth",
        "adsOverlayHeight": "fOverlayHeight",
        "adsBobFactor": "fAdsBobFactor",
        "adsViewBobMult": "fAdsViewBobMult",
        "adsAimPitch": "fAdsAimPitch",
        "adsCrosshairInFrac": "fAdsCrosshairInFrac",
        "adsCrosshairOutFrac": "fAdsCrosshairOutFrac",
        "adsReloadTransTime": "iPositionReloadTransTime",
        "adsTransBlendTime": "iPositionTransBlendTime",
        "adsGunKickPitchMin": "fAdsGunKickPitchMin",
        "adsGunKickPitchMax": "fAdsGunKickPitchMax",
        "adsGunKickYawMin": "fAdsGunKickYawMin",
        "adsGunKickYawMax": "fAdsGunKickYawMax",
        "adsGunKickAccel": "fAdsGunKickAccel",
        "adsGunKickSpeedMax": "fAdsGunKickSpeedMax",
        "adsGunKickSpeedDecay": "fAdsGunKickSpeedDecay",
        "adsGunKickStaticDecay": "fAdsGunKickStaticDecay",
        "adsViewKickPitchMin": "fAdsViewKickPitchMin",
        "adsViewKickPitchMax": "fAdsViewKickPitchMax",
        "adsViewKickYawMin": "fAdsViewKickYawMin",
        "adsViewKickYawMax": "fAdsViewKickYawMax",
        "adsViewKickCenterSpeed": "fAdsViewKickCenterSpeed",
        "adsSpread": "fAdsSpread",
        "adsSpreadDucked": "fAdsSpreadDucked",
        "adsSpreadProne": "fAdsSpreadProne",
        "hipSpreadStandMin": "fHipSpreadStandMin",
        "hipSpreadDuckedMin": "fHipSpreadDuckedMin",
        "hipSpreadProneMin": "fHipSpreadProneMin",
        "hipSpreadMax": "fHipSpreadMax",
        "hipSpreadDecayRate": "fHipSpreadDecayRate",
        "hipSpreadFireAdd": "fHipSpreadFireAdd",
        "hipSpreadTurnAdd": "fHipSpreadTurnAdd",
        "hipSpreadMoveAdd": "fHipSpreadMoveAdd",
        "hipSpreadDuckedDecay": "fHipSpreadDuckedDecay",
        "hipSpreadProneDecay": "fHipSpreadProneDecay",
        "hipReticleSidePos": "fHipReticleSidePos",
        "hipIdleAmount": "fHipIdleAmount",
        "hipGunKickPitchMin": "fHipGunKickPitchMin",
        "hipGunKickPitchMax": "fHipGunKickPitchMax",
        "hipGunKickYawMin": "fHipGunKickYawMin",
        "hipGunKickYawMax": "fHipGunKickYawMax",
        "hipGunKickAccel": "fHipGunKickAccel",
        "hipGunKickSpeedMax": "fHipGunKickSpeedMax",
        "hipGunKickSpeedDecay": "fHipGunKickSpeedDecay",
        "hipGunKickStaticDecay": "fHipGunKickStaticDecay",
        "hipViewKickPitchMin": "fHipViewKickPitchMin",
        "hipViewKickPitchMax": "fHipViewKickPitchMax",
        "hipViewKickYawMin": "fHipViewKickYawMin",
        "hipViewKickYawMax": "fHipViewKickYawMax",
        "hipViewKickCenterSpeed": "fHipViewKickCenterSpeed",
        "aiEffectiveRange": "aiEffectiveRange",
        "aiMissRange": "aiMissRange",
        "aiDamageMod": "aiDamageMod",
        "bulletConeAngle": "fBulletConeAngle",
        "adsBulletConeAngle": "fAdsBulletConeAngle",
        "animIKOffsetTime": "fAnimIKOffsetTime",
        "animIKOffsetForce": "fAnimIKOffsetForce",
        "animIKOffsetDist": "fAnimIKOffsetDist",
        "animIKPitchTime": "fAnimIKPitchTime",
        "animIKPitchForce": "fAnimIKPitchForce",
        "animIKPitchAngle": "fAnimIKPitchAngle",
        "animIKTorsoRecoilPitchTime": "fAnimIKTorsoRecoilPitchTime",
        "animIKTorsoRecoilPitchForce": "fAnimIKTorsoRecoilPitchForce",
        "animIKTorsoRecoilPitchAngle": "fAnimIKTorsoRecoilPitchAngle",
        # MP duplicates map to the same fields
        "damageMP": "iDamage",
        "meleeDamageMP": "iMeleeDamage",
        "damageInnerRadiusMP": "iDamageInnerRadius",
        "damageOuterRadiusMP": "iDamageOuterRadius",
        "minDamagePercentMP": "iMinDamagePercent",
        "fireDelayMP": "iFireDelay",
        "meleeDelayMP": "iMeleeDelay",
        "fireTimeMP": "iFireTime",
        "meleeTimeMP": "iMeleeTime",
        "explosionRadiusMP": "iExplosionRadius",
        "explosionInnerDamageMP": "iExplosionInnerDamage",
        "explosionOuterDamageMP": "iExplosionOuterDamage",
        "maxAmmoMP": "iMaxAmmo",
        "sensitivityScaleMP": "fSensitivityScale",
        "adsZoomFovMP": "fAdsZoomFov",
        "adsTransInTimeMP": "iAdsTransInTime",
        "adsTransOutTimeMP": "iAdsTransOutTime",
        "adsSensitivityScaleMP": "fAdsSensitivityScale",
        "adsSpreadMP": "fAdsSpread",
        "adsSpreadDuckedMP": "fAdsSpreadDucked",
        "adsSpreadProneMP": "fAdsSpreadProne",
        "hipSpreadStandMinMP": "fHipSpreadStandMin",
        "hipSpreadDuckedMinMP": "fHipSpreadDuckedMin",
        "hipSpreadProneMinMP": "fHipSpreadProneMin",
        "hipSpreadMaxMP": "fHipSpreadMax",
        "hipSpreadDecayRateMP": "fHipSpreadDecayRate",
        "hipSpreadFireAddMP": "fHipSpreadFireAdd",
        "hipSpreadTurnAddMP": "fHipSpreadTurnAdd",
        "hipSpreadMoveAddMP": "fHipSpreadMoveAdd",
        "hipSpreadDuckedDecayMP": "fHipSpreadDuckedDecay",
        "hipSpreadProneDecayMP": "fHipSpreadProneDecay",
    }
    if stem in scalars:
        return scalars[stem], -1
    return None, -1


def gen_cpp():
    out = []
    out.append("// ============================================================================")
    out.append("// g_weaponfuncs.cpp - weaponFuncs stat adjusters (game2.o)")
    out.append("// Uniform release-build pattern (verified against IDA):")
    out.append("//   X_Function(v): info = BG_GetPlayerWeaponInfo(); if (!info) return 0;")
    out.append("//   info->FIELD += v; return info->FIELD;")
    out.append("// ============================================================================")
    out.append("")
    out.append('#include "game/logic/g_weaponfuncs.h"')
    out.append("")
    out.append("namespace weaponFuncs {")
    out.append("")

    entries = []
    for line in open(MANIFEST, encoding="utf-8"):
        parts = line.rstrip("\n").split("\t")
        if len(parts) < 8 or parts[0] == "ida_ea":
            continue
        if parts[3] != "f" or parts[5] != "game2.o":
            continue
        name = parts[2]
        if not name.startswith("?") or "weaponFuncs" not in name:
            continue
        mangled = name
        fn = name.split("@")[0][1:]
        ea = parts[0]
        # int variants: YAHH (int -> int); float variants: YAMM (float -> double)
        is_float = "YAMM" in mangled
        entries.append((ea, fn, is_float))

    entries.sort()
    written = set()
    for ea, fn, is_float in entries:
        if fn in written:
            continue
        written.add(fn)
        field, arr_idx = field_for_func(fn)
        if field is None:
            print("SKIP (no field map):", fn)
            continue
        out.append("// ea: %s" % ea)
        if is_float:
            out.append("double %s(float v)" % fn)
        else:
            out.append("int %s(int v)" % fn)
        out.append("{")
        out.append("    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();")
        out.append("    if (info == nullptr)")
        out.append("        return 0;")
        if arr_idx >= 0:
            out.append("    %s r = v + info->%s[%d];" %
                       ("float" if is_float else "int", field, arr_idx))
            out.append("    info->%s[%d] = r;" % (field, arr_idx))
        else:
            out.append("    %s r = v + info->%s;" %
                       ("float" if is_float else "int", field))
            out.append("    info->%s = r;" % field)
        out.append("    return r;")
        out.append("}")
        out.append("")

    out.append("} // namespace weaponFuncs")
    out.append("")
    return "\n".join(out)


# vehicle_info_t fields (IDA 0x310 layout) used by vehicleFuncs helpers
VEHICLE_INFO_FIELDS = {
    # int fields (YAHH)
    "steerWheels": ("int", False),
    "quadBarrel": ("int", False),
    "spClientSeat": ("int", False),
    "hudIndex": ("int", False),
    "numSeats": ("int", False),
    "health": ("int", False),
    "vehicleAnimMatrixColumn": ("int", False),
    "inactiveBlowupSeconds": ("int", False),
    # float fields (YAMM)
    "bulletDamage": ("float", False),
    "grenadeDamage": ("float", False),
    "mineDamage": ("float", False),
    "projectileDamage": ("float", False),
    "texScrollScale": ("float", False),
    "maxSpeed": ("float", False),
    "accel": ("float", False),
    "rotRate": ("float", False),
    "rotAccel": ("float", False),
    "maxBodyPitch": ("float", False),
    "maxBodyRoll": ("float", False),
    "collisionDamage": ("float", False),
    "collisionSpeed": ("float", False),
    "suspensionTravel": ("float", False),
    "boundsRadius": ("float", False),
    "boundsHeight": ("float", False),
    "boundsLength": ("float", False),
    "turretHorizSpanLeft": ("float", False),
    "turretHorizSpanRight": ("float", False),
    "turretVertSpanUp": ("float", False),
    "turretVertSpanDown": ("float", False),
    "turretRotRate": ("float", False),
    "turretSwirlLerpRate": ("float", False),
    "turretSwirlPitchFactor": ("float", False),
    "turretGunnerVertSpanUp": ("float", False),
    "turretGunnerVertSpanDown": ("float", False),
    "engineSndSpeed": ("float", False),
    "cameraZOffset": ("float", False),
    "cameraFPHeightOffset": ("float", False),
    "cameraFPFwdOffset": ("float", False),
    "cameraFPHeightLerp": ("float", False),
    "cameraChaseOffsetX": ("float", False),
    "cameraChaseOffsetY": ("float", False),
    "cameraChaseOffsetZ": ("float", False),
    "cameraChaseRadiusInner": ("float", False),
    "cameraChaseRadiusOuter": ("float", False),
    "cameraVehViewRadius": ("float", False),
    "cameraVehViewMaxPitch": ("float", False),
    "cameraVehViewMaxPitchDistAdj": ("float", False),
    "cameraVehViewFwdBackRatio": ("float", False),
    "cameraVehViewMoveInPitch": ("float", False),
    "camLinkedPitchFactor": ("float", False),
    "pitchBasedCamOffsetX": ("float", False),
    "pitchBasedCamOffsetZ": ("float", False),
    "hatchOpenAngleRight": ("float", False),
    "hatchOpenAngleLeft": ("float", False),
    # float-signature but int field (decompile: int v3 = texScroll + v)
    "texScroll": ("float2int", False),      # binary: texureScroll_Function
    "texScrollScale": ("float", False),     # binary: texureScrollScale_Function
}

# plain vehicle physics helpers: name -> vehicle_rb_parameter member
RB_PARAM_FIELDS = {
    "speed_max": "m_speed_max",
    "accel_max": "m_accel_max",
    "reverse_scale": "m_reverse_scale",
    "steer_angle_max": "m_steer_angle_max",
    "steer_speed": "m_steer_speed",
    "wheel_radius": "m_wheel_radius",
    "susp_spring_k": "m_susp_spring_k",
    "susp_damp_k": "m_susp_damp_k",
    "susp_adj": "m_susp_adj",
    "susp_hard_limit": "m_susp_hard_limit",
    "tire_fric_fwd": "m_tire_fric_fwd",
    "tire_fric_side": "m_tire_fric_side",
    "tire_fric_brake": "m_tire_fric_brake",
    "tire_fric_hand_brake": "m_tire_fric_hand_brake",
    "body_mass": "m_body_mass",
    "mass_center_delta_x": "m_mass_center_delta_x",
    "mass_center_delta_y": "m_mass_center_delta_y",
    "mass_center_delta_z": "m_mass_center_delta_z",
    "roll_stability": "m_roll_stability",
    "roll_resistance": "m_roll_resistance",
    "upright_strength": "m_upright_strength",
    "tilt_fakey": "m_tilt_fakey",
    "peel_out_max_speed": "m_peel_out_max_speed",
    "inertia_scale_x": "m_inertia_scale_x",
    "tire_damp_coast": "m_tire_damp_coast",
    "tire_damp_brake": "m_tire_damp_brake",
    "tire_damp_hand": "m_tire_damp_hand",
}


def gen_vehicle_header():
    out = []
    out.append("// ============================================================================")
    out.append("// g_vehiclefuncs.h - vehicleFuncs + vehicle physics stat helpers (game2.o)")
    out.append("// Layouts verified against IDA local types.")
    out.append("// ============================================================================")
    out.append("")
    out.append("#pragma once")
    out.append("")
    out.append("#include <stddef.h>")
    out.append("")
    out.append("// vehicle_info_t - 0x310 (IDA verified; subset for stat helpers)")
    out.append("struct vehicle_info_t {")
    out.append("    char name[0x20];             // +0x00")
    out.append("    short type;                  // +0x20")
    out.append("    short subtype;               // +0x22")
    out.append("    int steerWheels;             // +0x24")
    out.append("    int texScroll;               // +0x28")
    out.append("    int quadBarrel;              // +0x2C")
    out.append("    float bulletDamage;          // +0x30")
    out.append("    float grenadeDamage;         // +0x34")
    out.append("    float mineDamage;            // +0x38")
    out.append("    float projectileDamage;      // +0x3C")
    out.append("    int spClientSeat;            // +0x40")
    out.append("    int numSeats;                // +0x44")
    out.append("    int hudIndex;                // +0x48")
    out.append("    float texScrollScale;        // +0x4C")
    out.append("    float maxSpeed;              // +0x50")
    out.append("    float accel;                 // +0x54")
    out.append("    float rotRate;               // +0x58")
    out.append("    float rotAccel;              // +0x5C")
    out.append("    float maxBodyPitch;          // +0x60")
    out.append("    float maxBodyRoll;           // +0x64")
    out.append("    float collisionDamage;       // +0x68")
    out.append("    float collisionSpeed;        // +0x6C")
    out.append("    float suspensionTravel;      // +0x70")
    out.append("    float boundsRadius;          // +0x74")
    out.append("    float boundsHeight;          // +0x78")
    out.append("    float boundsLength;          // +0x7C")
    out.append("    int health;                  // +0x80")
    out.append("    unsigned char _pad84[0x164 - 0x84];")
    out.append("    float turretHorizSpanLeft;   // +0x164")
    out.append("    float turretHorizSpanRight;  // +0x168")
    out.append("    float turretVertSpanUp;      // +0x16C")
    out.append("    float turretVertSpanDown;    // +0x170")
    out.append("    float turretRotRate;         // +0x174")
    out.append("    float turretSwirlLerpRate;   // +0x178")
    out.append("    float turretSwirlPitchFactor;// +0x17C")
    out.append("    float turretGunnerVertSpanUp;   // +0x180")
    out.append("    float turretGunnerVertSpanDown; // +0x184")
    out.append("    float engineSndSpeed;        // +0x188")
    out.append("    unsigned char _pad18C[0x1B0 - 0x18C];")
    out.append("    float cameraZOffset;         // +0x1B0")
    out.append("    float cameraFPHeightOffset;  // +0x1B4")
    out.append("    float cameraFPFwdOffset;     // +0x1B8")
    out.append("    float cameraFPHeightLerp;    // +0x1BC")
    out.append("    float cameraChaseOffsetX;    // +0x1C0")
    out.append("    float cameraChaseOffsetY;    // +0x1C4")
    out.append("    float cameraChaseOffsetZ;    // +0x1C8")
    out.append("    float cameraChaseRadiusInner;   // +0x1CC")
    out.append("    float cameraChaseRadiusOuter;   // +0x1D0")
    out.append("    float cameraVehViewRadius;      // +0x1D4")
    out.append("    float cameraVehViewMaxPitch;    // +0x1D8")
    out.append("    float cameraVehViewMaxPitchDistAdj;  // +0x1DC")
    out.append("    float cameraVehViewFwdBackRatio;     // +0x1E0")
    out.append("    float cameraVehViewMoveInPitch;     // +0x1E4")
    out.append("    float camLinkedPitchFactor;         // +0x1E8")
    out.append("    float pitchBasedCamOffsetX;         // +0x1EC")
    out.append("    float pitchBasedCamOffsetZ;         // +0x1F0")
    out.append("    unsigned char _pad1F4[0x280 - 0x1F4];")
    out.append("    int vehicleAnimMatrixColumn;        // +0x280")
    out.append("    unsigned char _pad284[0x2C4 - 0x284];")
    out.append("    float hatchOpenAngleRight;   // +0x2C4")
    out.append("    float hatchOpenAngleLeft;    // +0x2C8")
    out.append("    unsigned char _pad2CC[0x30C - 0x2CC];")
    out.append("    int inactiveBlowupSeconds;   // +0x30C")
    out.append("};")
    out.append("static_assert(sizeof(vehicle_info_t) == 0x310, "
               '"vehicle_info_t size mismatch");')
    out.append("")
    out.append("vehicle_info_t* VEH_GetPlayerVehicleInfo();  // game.o")
    out.append("")
    out.append("namespace vehicleFuncs {")
    for fname in VEHICLE_INFO_FIELDS:
        ftype, _ = VEHICLE_INFO_FIELDS[fname]
        sig = {"int": "int %s(int v);", "float": "double %s(float v);",
               "bool": "bool %s(bool v);",
               "float2int": "double %s(float v);"}[ftype]
        decl_name = fname + "_Function"
        if fname == "texScroll":
            decl_name = "texureScroll_Function"
        elif fname == "texScrollScale":
            decl_name = "texureScrollScale_Function"
        out.append(sig % decl_name)
    out.append("} // namespace vehicleFuncs")
    out.append("")
    # vehicle_rb_parameter (physics) helpers
    out.append("// vehicle_rb_parameter - 0xD0 (IDA verified; subset)")
    out.append("struct vehicle_rb_parameter {")
    out.append("    float m_speed_max;          // +0x00")
    out.append("    float m_accel_max;          // +0x04")
    out.append("    float m_reverse_scale;      // +0x08")
    out.append("    float m_steer_angle_max;    // +0x0C")
    out.append("    float m_steer_speed;        // +0x10")
    out.append("    float m_wheel_radius;       // +0x14")
    out.append("    float m_susp_spring_k;      // +0x18")
    out.append("    float m_susp_damp_k;        // +0x1C")
    out.append("    float m_susp_adj;           // +0x20")
    out.append("    float m_susp_hard_limit;    // +0x24")
    out.append("    float m_tire_fric_fwd;      // +0x28")
    out.append("    float m_tire_fric_side;     // +0x2C")
    out.append("    float m_tire_fric_brake;    // +0x30")
    out.append("    float m_tire_fric_hand_brake;  // +0x34")
    out.append("    float m_body_mass;          // +0x38")
    out.append("    float m_mass_center_delta_x;  // +0x3C")
    out.append("    float m_mass_center_delta_y;  // +0x40")
    out.append("    float m_mass_center_delta_z;  // +0x44")
    out.append("    float m_roll_stability;     // +0x48")
    out.append("    float m_roll_resistance;    // +0x4C")
    out.append("    float m_upright_strength;   // +0x50")
    out.append("    float m_tilt_fakey;         // +0x54")
    out.append("    float m_peel_out_max_speed; // +0x58")
    out.append("    float m_inertia_scale_x;    // +0x5C")
    out.append("    float m_tire_damp_coast;    // +0x60")
    out.append("    float m_tire_damp_brake;    // +0x64")
    out.append("    float m_tire_damp_hand;     // +0x68")
    out.append("};")
    out.append("// NOTE: subset view - only the stat fields used by the helpers;")
    out.append("// full 0xD0 layout includes m_traction_type/m_name/bbox.")
    out.append("")
    out.append("struct rb_vehicle {")
    out.append("    unsigned char _pad[0x250];      // +0x000")
    out.append("    vehicle_rb_parameter* m_parameter;  // +0x250")
    out.append("    void update_parms(vehicle_rb_parameter* params,"
               " bool initialization);  // ?update_parms@rb_vehicle@@QAEXPAV"
               "vehicle_rb_parameter@@_N@Z")
    out.append("};")
    out.append("rb_vehicle* GetPlayerRBVehicle();  // game2.o")
    for fname in RB_PARAM_FIELDS:
        out.append("double %s_Function(float f);" % fname)
    out.append("")
    return "\n".join(out)


def gen_vehicle_cpp():
    out = []
    out.append("// ============================================================================")
    out.append("// g_vehiclefuncs.cpp - vehicleFuncs + vehicle physics stat helpers (game2.o)")
    out.append("// ============================================================================")
    out.append("")
    out.append('#include "game/logic/g_vehiclefuncs.h"')
    out.append("")
    out.append("namespace vehicleFuncs {")
    out.append("")
    for line in open(MANIFEST, encoding="utf-8"):
        parts = line.rstrip("\n").split("\t")
        if len(parts) < 8 or parts[0] == "ida_ea":
            continue
        if parts[3] != "f" or parts[5] != "game2.o":
            continue
        name = parts[2]
        if not name.startswith("?") or "vehicleFuncs" not in name:
            continue
        fn = name.split("@")[0][1:]
        stem = fn[:-len("_Function")] if fn.endswith("_Function") else fn
        # binary typo aliases
        if stem == "texureScroll":
            stem = "texScroll"
        if stem == "texureScrollScale":
            stem = "texScrollScale"
        if stem not in VEHICLE_INFO_FIELDS:
            print("SKIP vehicle (no field):", fn)
            continue
        ftype, _ = VEHICLE_INFO_FIELDS[stem]
        out.append("// ea: %s" % parts[0])
        if ftype == "bool":
            out.append("bool %s(bool v)" % fn)
            out.append("{")
            out.append("    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();")
            out.append("    if (info == nullptr)")
            out.append("        return false;")
            out.append("    info->%s += v ? 1 : 0;" % stem)
            out.append("    return info->%s != 0;" % stem)
            out.append("}")
        elif ftype == "float2int":
            out.append("double %s(float v)" % fn)
            out.append("{")
            out.append("    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();")
            out.append("    if (info == nullptr)")
            out.append("        return 0;")
            out.append("    int r = (int)v + info->%s;" % stem)
            out.append("    info->%s = r;" % stem)
            out.append("    return r;")
            out.append("}")
        elif ftype == "float":
            out.append("double %s(float v)" % fn)
            out.append("{")
            out.append("    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();")
            out.append("    if (info == nullptr)")
            out.append("        return 0;")
            out.append("    float r = v + info->%s;" % stem)
            out.append("    info->%s = r;" % stem)
            out.append("    return r;")
            out.append("}")
        else:
            out.append("int %s(int v)" % fn)
            out.append("{")
            out.append("    vehicle_info_t* info = VEH_GetPlayerVehicleInfo();")
            out.append("    if (info == nullptr)")
            out.append("        return 0;")
            out.append("    int r = v + info->%s;" % stem)
            out.append("    info->%s = r;" % stem)
            out.append("    return r;")
            out.append("}")
        out.append("")
    out.append("} // namespace vehicleFuncs")
    out.append("")
    out.append("// ============================================================================")
    out.append("// vehicle physics stat helpers (GetPlayerRBVehicle + vehicle_rb_parameter)")
    out.append("// ============================================================================")
    out.append("")
    for line in open(MANIFEST, encoding="utf-8"):
        parts = line.rstrip("\n").split("\t")
        if len(parts) < 8 or parts[0] == "ida_ea":
            continue
        if parts[3] != "f" or parts[5] != "game2.o":
            continue
        name = parts[2]
        if not name.startswith("?") or "vehicleFuncs" in name:
            continue
        fn = name.split("@")[0][1:]
        stem = fn[:-len("_Function")] if fn.endswith("_Function") else fn
        if stem not in RB_PARAM_FIELDS:
            continue
        out.append("// ea: %s" % parts[0])
        out.append("double %s_Function(float f)" % stem)
        out.append("{")
        out.append("    rb_vehicle* vehicle = GetPlayerRBVehicle();")
        out.append("    if (vehicle == nullptr)")
        out.append("        return 0;")
        out.append("    vehicle_rb_parameter* params = vehicle->m_parameter;")
        out.append("    if (params == nullptr)")
        out.append("        return 0;")
        out.append("    params->%s = params->%s + f;" % (RB_PARAM_FIELDS[stem],
                                                        RB_PARAM_FIELDS[stem]))
        out.append("    if (f != 0.0f)")
        out.append("        vehicle->update_parms(params, false);")
        out.append("    return params->%s;" % RB_PARAM_FIELDS[stem])
        out.append("}")
        out.append("")
    return "\n".join(out)


if __name__ == "__main__":
    with open(r"src\game\logic\g_weaponfuncs.h", "w", encoding="utf-8",
              newline="\n") as f:
        f.write(gen_header())
    with open(r"src\game\logic\g_weaponfuncs.cpp", "w", encoding="utf-8",
              newline="\n") as f:
        f.write(gen_cpp())
    with open(r"src\game\logic\g_vehiclefuncs.h", "w", encoding="utf-8",
              newline="\n") as f:
        f.write(gen_vehicle_header())
    with open(r"src\game\logic\g_vehiclefuncs.cpp", "w", encoding="utf-8",
              newline="\n") as f:
        f.write(gen_vehicle_cpp())
    print("generated weaponfuncs + vehiclefuncs")
