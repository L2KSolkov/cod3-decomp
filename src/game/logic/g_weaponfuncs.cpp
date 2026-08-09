// ============================================================================
// g_weaponfuncs.cpp - weaponFuncs stat adjusters (game2.o)
// Uniform release-build pattern (verified against IDA):
//   X_Function(v): info = BG_GetPlayerWeaponInfo(); if (!info) return 0;
//   info->FIELD += v; return info->FIELD;
// ============================================================================

#include "game/logic/g_weaponfuncs.h"

namespace weaponFuncs {

// ea: 0x004EC8D0
int reticleCenterSize_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iReticleCenterSize;
    info->iReticleCenterSize = r;
    return r;
}

// ea: 0x004EC900
int reticleSideSize_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iReticleSideSize;
    info->iReticleSideSize = r;
    return r;
}

// ea: 0x004EC930
int reticleMinOfs_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iReticleMinOfs;
    info->iReticleMinOfs = r;
    return r;
}

// ea: 0x004EC960
double duckedOfsF_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vDuckedOfs[0];
    info->vDuckedOfs[0] = r;
    return r;
}

// ea: 0x004EC9A0
double duckedOfsR_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vDuckedOfs[1];
    info->vDuckedOfs[1] = r;
    return r;
}

// ea: 0x004EC9E0
double duckedOfsU_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vDuckedOfs[2];
    info->vDuckedOfs[2] = r;
    return r;
}

// ea: 0x004ECA20
double proneOfsF_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vProneOfs[0];
    info->vProneOfs[0] = r;
    return r;
}

// ea: 0x004ECA60
double proneOfsR_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vProneOfs[1];
    info->vProneOfs[1] = r;
    return r;
}

// ea: 0x004ECAA0
double proneOfsU_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vProneOfs[2];
    info->vProneOfs[2] = r;
    return r;
}

// ea: 0x004ECAE0
double standMoveF_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vStandMove[0];
    info->vStandMove[0] = r;
    return r;
}

// ea: 0x004ECB20
double standMoveR_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vStandMove[1];
    info->vStandMove[1] = r;
    return r;
}

// ea: 0x004ECB60
double standMoveU_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vStandMove[2];
    info->vStandMove[2] = r;
    return r;
}

// ea: 0x004ECBA0
double duckedMoveF_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vDuckedMove[0];
    info->vDuckedMove[0] = r;
    return r;
}

// ea: 0x004ECBE0
double duckedMoveR_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vDuckedMove[1];
    info->vDuckedMove[1] = r;
    return r;
}

// ea: 0x004ECC20
double duckedMoveU_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vDuckedMove[2];
    info->vDuckedMove[2] = r;
    return r;
}

// ea: 0x004ECC60
double proneMoveF_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vProneMove[0];
    info->vProneMove[0] = r;
    return r;
}

// ea: 0x004ECCA0
double proneMoveR_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vProneMove[1];
    info->vProneMove[1] = r;
    return r;
}

// ea: 0x004ECCE0
double proneMoveU_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vProneMove[2];
    info->vProneMove[2] = r;
    return r;
}

// ea: 0x004ECD20
double proneRotP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vProneRot[0];
    info->vProneRot[0] = r;
    return r;
}

// ea: 0x004ECD60
double proneRotY_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vProneRot[1];
    info->vProneRot[1] = r;
    return r;
}

// ea: 0x004ECDA0
double proneRotR_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->vProneRot[1];
    info->vProneRot[1] = r;
    return r;
}

// ea: 0x004ECDE0
double standMoveMinSpeed_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fStandMoveMinSpeed;
    info->fStandMoveMinSpeed = r;
    return r;
}

// ea: 0x004ECE20
double duckedMoveMinSpeed_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fDuckedMoveMinSpeed;
    info->fDuckedMoveMinSpeed = r;
    return r;
}

// ea: 0x004ECE60
double proneMoveMinSpeed_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fProneMoveMinSpeed;
    info->fProneMoveMinSpeed = r;
    return r;
}

// ea: 0x004ECEA0
double posProneRotRate_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fPosProneRotRate;
    info->fPosProneRotRate = r;
    return r;
}

// ea: 0x004ECEE0
double proneRotMinSpeed_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fProneRotMinSpeed;
    info->fProneRotMinSpeed = r;
    return r;
}

// ea: 0x004ECF20
int damage_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iDamage;
    info->iDamage = r;
    return r;
}

// ea: 0x004ECF50
int meleeDamage_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iMeleeDamage;
    info->iMeleeDamage = r;
    return r;
}

// ea: 0x004ECF80
double sensitivityScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fSensitivityScale;
    info->fSensitivityScale = r;
    return r;
}

// ea: 0x004ECFC0
int damageInnerRadius_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iDamageInnerRadius;
    info->iDamageInnerRadius = r;
    return r;
}

// ea: 0x004ECFF0
int damageOuterRadius_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iDamageOuterRadius;
    info->iDamageOuterRadius = r;
    return r;
}

// ea: 0x004ED020
int minDamagePercent_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iMinDamagePercent;
    info->iMinDamagePercent = r;
    return r;
}

// ea: 0x004ED050
int fireDelay_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iFireDelay;
    info->iFireDelay = r;
    return r;
}

// ea: 0x004ED080
int meleeDelay_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iMeleeDelay;
    info->iMeleeDelay = r;
    return r;
}

// ea: 0x004ED0B0
int fireTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iFireTime;
    info->iFireTime = r;
    return r;
}

// ea: 0x004ED0E0
int rechamberTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iRechamberTime;
    info->iRechamberTime = r;
    return r;
}

// ea: 0x004ED110
int rechamberBoltTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iRechamberBoltTime;
    info->iRechamberBoltTime = r;
    return r;
}

// ea: 0x004ED140
int holdFireTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iHoldFireTime;
    info->iHoldFireTime = r;
    return r;
}

// ea: 0x004ED170
int meleeTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iMeleeTime;
    info->iMeleeTime = r;
    return r;
}

// ea: 0x004ED1A0
int reloadTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iReloadTime;
    info->iReloadTime = r;
    return r;
}

// ea: 0x004ED1D0
int reloadEmptyTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iReloadEmptyTime;
    info->iReloadEmptyTime = r;
    return r;
}

// ea: 0x004ED200
int reloadAddTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iReloadAddTime;
    info->iReloadAddTime = r;
    return r;
}

// ea: 0x004ED230
int reloadStartTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iReloadStartTime;
    info->iReloadStartTime = r;
    return r;
}

// ea: 0x004ED260
int reloadStartAddTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iReloadStartAddTime;
    info->iReloadStartAddTime = r;
    return r;
}

// ea: 0x004ED290
int reloadEndTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iReloadEndTime;
    info->iReloadEndTime = r;
    return r;
}

// ea: 0x004ED2C0
int dropTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iDropTime;
    info->iDropTime = r;
    return r;
}

// ea: 0x004ED2F0
int raiseTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iRaiseTime;
    info->iRaiseTime = r;
    return r;
}

// ea: 0x004ED320
int altDropTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iAltDropTime;
    info->iAltDropTime = r;
    return r;
}

// ea: 0x004ED350
int altRaiseTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iAltRaiseTime;
    info->iAltRaiseTime = r;
    return r;
}

// ea: 0x004ED380
int fuseTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iFuseTime;
    info->iFuseTime = r;
    return r;
}

// ea: 0x004ED3B0
double moveSpeedScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fMoveSpeedScale;
    info->fMoveSpeedScale = r;
    return r;
}

// ea: 0x004ED3F0
double gunMaxPitch_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fGunMaxPitch;
    info->fGunMaxPitch = r;
    return r;
}

// ea: 0x004ED430
double gunMaxYaw_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fGunMaxYaw;
    info->fGunMaxYaw = r;
    return r;
}

// ea: 0x004ED470
double swayMaxAngle_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->swayMaxAngle;
    info->swayMaxAngle = r;
    return r;
}

// ea: 0x004ED4B0
double swayLerpSpeed_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->swayLerpSpeed;
    info->swayLerpSpeed = r;
    return r;
}

// ea: 0x004ED4F0
double swayPitchScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->swayPitchScale;
    info->swayPitchScale = r;
    return r;
}

// ea: 0x004ED530
double swayYawScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->swayYawScale;
    info->swayYawScale = r;
    return r;
}

// ea: 0x004ED570
double swayHorizScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->swayHorizScale;
    info->swayHorizScale = r;
    return r;
}

// ea: 0x004ED5B0
double swayVertScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->swayVertScale;
    info->swayVertScale = r;
    return r;
}

// ea: 0x004ED5F0
double swayShellShockScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->swayShellShockScale;
    info->swayShellShockScale = r;
    return r;
}

// ea: 0x004ED630
double adsSwayMaxAngle_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->adsSwayMaxAngle;
    info->adsSwayMaxAngle = r;
    return r;
}

// ea: 0x004ED670
double adsSwayLerpSpeed_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->adsSwayLerpSpeed;
    info->adsSwayLerpSpeed = r;
    return r;
}

// ea: 0x004ED6B0
double adsSwayPitchScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->adsSwayPitchScale;
    info->adsSwayPitchScale = r;
    return r;
}

// ea: 0x004ED6F0
double adsSwayYawScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->adsSwayYawScale;
    info->adsSwayYawScale = r;
    return r;
}

// ea: 0x004ED730
double adsSwayHorizScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->adsSwayHorizScale;
    info->adsSwayHorizScale = r;
    return r;
}

// ea: 0x004ED770
double adsSwayVertScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->adsSwayVertScale;
    info->adsSwayVertScale = r;
    return r;
}

// ea: 0x004ED7B0
int takedamage_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iTakeDamage;
    info->iTakeDamage = r;
    return r;
}

// ea: 0x004ED7E0
int explosionRadius_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iExplosionRadius;
    info->iExplosionRadius = r;
    return r;
}

// ea: 0x004ED810
int explosionInnerDamage_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iExplosionInnerDamage;
    info->iExplosionInnerDamage = r;
    return r;
}

// ea: 0x004ED840
int explosionOuterDamage_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iExplosionOuterDamage;
    info->iExplosionOuterDamage = r;
    return r;
}

// ea: 0x004ED870
int projectileSpeed_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iProjectileSpeed;
    info->iProjectileSpeed = r;
    return r;
}

// ea: 0x004ED8A0
int projectileSpeedUp_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iProjectileSpeedUp;
    info->iProjectileSpeedUp = r;
    return r;
}

// ea: 0x004ED8D0
int triggerRadius_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iTriggerRadius;
    info->iTriggerRadius = r;
    return r;
}

// ea: 0x004ED900
int adsTransInTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iAdsTransInTime;
    info->iAdsTransInTime = r;
    return r;
}

// ea: 0x004ED930
int adsTransOutTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iAdsTransOutTime;
    info->iAdsTransOutTime = r;
    return r;
}

// ea: 0x004ED960
double adsIdleAmount_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsIdleAmount;
    info->fAdsIdleAmount = r;
    return r;
}

// ea: 0x004ED9A0
double adsZoomFov_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsZoomFov;
    info->fAdsZoomFov = r;
    return r;
}

// ea: 0x004ED9E0
double adsSensitivityScale_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsSensitivityScale;
    info->fAdsSensitivityScale = r;
    return r;
}

// ea: 0x004EDA20
double adsZoomInFrac_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsZoomInFrac;
    info->fAdsZoomInFrac = r;
    return r;
}

// ea: 0x004EDA60
double adsZoomOutFrac_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsZoomOutFrac;
    info->fAdsZoomOutFrac = r;
    return r;
}

// ea: 0x004EDAA0
double adsOverlayWidth_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fOverlayWidth;
    info->fOverlayWidth = r;
    return r;
}

// ea: 0x004EDAE0
double adsOverlayHeight_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fOverlayHeight;
    info->fOverlayHeight = r;
    return r;
}

// ea: 0x004EDB20
double adsBobFactor_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsBobFactor;
    info->fAdsBobFactor = r;
    return r;
}

// ea: 0x004EDB60
double adsViewBobMult_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsViewBobMult;
    info->fAdsViewBobMult = r;
    return r;
}

// ea: 0x004EDBA0
double adsAimPitch_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsAimPitch;
    info->fAdsAimPitch = r;
    return r;
}

// ea: 0x004EDBE0
double adsCrosshairInFrac_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsCrosshairInFrac;
    info->fAdsCrosshairInFrac = r;
    return r;
}

// ea: 0x004EDC20
double adsCrosshairOutFrac_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsCrosshairOutFrac;
    info->fAdsCrosshairOutFrac = r;
    return r;
}

// ea: 0x004EDC60
int adsReloadTransTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iPositionReloadTransTime;
    info->iPositionReloadTransTime = r;
    return r;
}

// ea: 0x004EDC90
int adsTransBlendTime_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iPositionTransBlendTime;
    info->iPositionTransBlendTime = r;
    return r;
}

// ea: 0x004EDCC0
double adsGunKickPitchMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsGunKickPitchMin;
    info->fAdsGunKickPitchMin = r;
    return r;
}

// ea: 0x004EDD00
double adsGunKickPitchMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsGunKickPitchMax;
    info->fAdsGunKickPitchMax = r;
    return r;
}

// ea: 0x004EDD40
double adsGunKickYawMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsGunKickYawMin;
    info->fAdsGunKickYawMin = r;
    return r;
}

// ea: 0x004EDD80
double adsGunKickYawMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsGunKickYawMax;
    info->fAdsGunKickYawMax = r;
    return r;
}

// ea: 0x004EDDC0
double adsGunKickAccel_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsGunKickAccel;
    info->fAdsGunKickAccel = r;
    return r;
}

// ea: 0x004EDE00
double adsGunKickSpeedMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsGunKickSpeedMax;
    info->fAdsGunKickSpeedMax = r;
    return r;
}

// ea: 0x004EDE40
double adsGunKickSpeedDecay_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsGunKickSpeedDecay;
    info->fAdsGunKickSpeedDecay = r;
    return r;
}

// ea: 0x004EDE80
double adsGunKickStaticDecay_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsGunKickStaticDecay;
    info->fAdsGunKickStaticDecay = r;
    return r;
}

// ea: 0x004EDEC0
double adsViewKickPitchMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsViewKickPitchMin;
    info->fAdsViewKickPitchMin = r;
    return r;
}

// ea: 0x004EDF00
double adsViewKickPitchMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsViewKickPitchMax;
    info->fAdsViewKickPitchMax = r;
    return r;
}

// ea: 0x004EDF40
double adsViewKickYawMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsViewKickYawMin;
    info->fAdsViewKickYawMin = r;
    return r;
}

// ea: 0x004EDF80
double adsViewKickYawMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsViewKickYawMax;
    info->fAdsViewKickYawMax = r;
    return r;
}

// ea: 0x004EDFC0
double adsViewKickCenterSpeed_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsViewKickCenterSpeed;
    info->fAdsViewKickCenterSpeed = r;
    return r;
}

// ea: 0x004EE000
double adsSpread_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsSpread;
    info->fAdsSpread = r;
    return r;
}

// ea: 0x004EE040
double adsSpreadDucked_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsSpreadDucked;
    info->fAdsSpreadDucked = r;
    return r;
}

// ea: 0x004EE080
double adsSpreadProne_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsSpreadProne;
    info->fAdsSpreadProne = r;
    return r;
}

// ea: 0x004EE0C0
double hipSpreadStandMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadStandMin;
    info->fHipSpreadStandMin = r;
    return r;
}

// ea: 0x004EE100
double hipSpreadDuckedMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadDuckedMin;
    info->fHipSpreadDuckedMin = r;
    return r;
}

// ea: 0x004EE140
double hipSpreadProneMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadProneMin;
    info->fHipSpreadProneMin = r;
    return r;
}

// ea: 0x004EE180
double hipSpreadMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadMax;
    info->fHipSpreadMax = r;
    return r;
}

// ea: 0x004EE1C0
double hipSpreadDecayRate_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadDecayRate;
    info->fHipSpreadDecayRate = r;
    return r;
}

// ea: 0x004EE200
double hipSpreadFireAdd_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadFireAdd;
    info->fHipSpreadFireAdd = r;
    return r;
}

// ea: 0x004EE240
double hipSpreadTurnAdd_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadTurnAdd;
    info->fHipSpreadTurnAdd = r;
    return r;
}

// ea: 0x004EE280
double hipSpreadMoveAdd_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadMoveAdd;
    info->fHipSpreadMoveAdd = r;
    return r;
}

// ea: 0x004EE2C0
double hipSpreadDuckedDecay_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadDuckedDecay;
    info->fHipSpreadDuckedDecay = r;
    return r;
}

// ea: 0x004EE300
double hipSpreadProneDecay_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadProneDecay;
    info->fHipSpreadProneDecay = r;
    return r;
}

// ea: 0x004EE340
double hipReticleSidePos_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipReticleSidePos;
    info->fHipReticleSidePos = r;
    return r;
}

// ea: 0x004EE380
double hipIdleAmount_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipIdleAmount;
    info->fHipIdleAmount = r;
    return r;
}

// ea: 0x004EE3C0
double hipGunKickPitchMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipGunKickPitchMin;
    info->fHipGunKickPitchMin = r;
    return r;
}

// ea: 0x004EE400
double hipGunKickPitchMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipGunKickPitchMax;
    info->fHipGunKickPitchMax = r;
    return r;
}

// ea: 0x004EE440
double hipGunKickYawMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipGunKickYawMin;
    info->fHipGunKickYawMin = r;
    return r;
}

// ea: 0x004EE480
double hipGunKickYawMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipGunKickYawMax;
    info->fHipGunKickYawMax = r;
    return r;
}

// ea: 0x004EE4C0
double hipGunKickAccel_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipGunKickAccel;
    info->fHipGunKickAccel = r;
    return r;
}

// ea: 0x004EE500
double hipGunKickSpeedMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipGunKickSpeedMax;
    info->fHipGunKickSpeedMax = r;
    return r;
}

// ea: 0x004EE540
double hipGunKickSpeedDecay_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipGunKickSpeedDecay;
    info->fHipGunKickSpeedDecay = r;
    return r;
}

// ea: 0x004EE580
double hipGunKickStaticDecay_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipGunKickStaticDecay;
    info->fHipGunKickStaticDecay = r;
    return r;
}

// ea: 0x004EE5C0
double hipViewKickPitchMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipViewKickPitchMin;
    info->fHipViewKickPitchMin = r;
    return r;
}

// ea: 0x004EE600
double hipViewKickPitchMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipViewKickPitchMax;
    info->fHipViewKickPitchMax = r;
    return r;
}

// ea: 0x004EE640
double hipViewKickYawMin_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipViewKickYawMin;
    info->fHipViewKickYawMin = r;
    return r;
}

// ea: 0x004EE680
double hipViewKickYawMax_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipViewKickYawMax;
    info->fHipViewKickYawMax = r;
    return r;
}

// ea: 0x004EE6C0
double hipViewKickCenterSpeed_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipViewKickCenterSpeed;
    info->fHipViewKickCenterSpeed = r;
    return r;
}

// ea: 0x004EE700
double aiEffectiveRange_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->aiEffectiveRange;
    info->aiEffectiveRange = r;
    return r;
}

// ea: 0x004EE740
double aiMissRange_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->aiMissRange;
    info->aiMissRange = r;
    return r;
}

// ea: 0x004EE780
double aiDamageMod_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->aiDamageMod;
    info->aiDamageMod = r;
    return r;
}

// ea: 0x004EE7C0
double bulletConeAngle_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fBulletConeAngle;
    info->fBulletConeAngle = r;
    return r;
}

// ea: 0x004EE800
double adsBulletConeAngle_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsBulletConeAngle;
    info->fAdsBulletConeAngle = r;
    return r;
}

// ea: 0x004EE840
double animIKOffsetTime_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAnimIKOffsetTime;
    info->fAnimIKOffsetTime = r;
    return r;
}

// ea: 0x004EE880
double animIKOffsetForce_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAnimIKOffsetForce;
    info->fAnimIKOffsetForce = r;
    return r;
}

// ea: 0x004EE8C0
double animIKOffsetDist_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAnimIKOffsetDist;
    info->fAnimIKOffsetDist = r;
    return r;
}

// ea: 0x004EE900
double animIKPitchTime_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAnimIKPitchTime;
    info->fAnimIKPitchTime = r;
    return r;
}

// ea: 0x004EE940
double animIKPitchForce_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAnimIKPitchForce;
    info->fAnimIKPitchForce = r;
    return r;
}

// ea: 0x004EE980
double animIKPitchAngle_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAnimIKPitchAngle;
    info->fAnimIKPitchAngle = r;
    return r;
}

// ea: 0x004EE9C0
double animIKTorsoRecoilPitchTime_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAnimIKTorsoRecoilPitchTime;
    info->fAnimIKTorsoRecoilPitchTime = r;
    return r;
}

// ea: 0x004EEA00
double animIKTorsoRecoilPitchForce_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAnimIKTorsoRecoilPitchForce;
    info->fAnimIKTorsoRecoilPitchForce = r;
    return r;
}

// ea: 0x004EEA40
double animIKTorsoRecoilPitchAngle_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAnimIKTorsoRecoilPitchAngle;
    info->fAnimIKTorsoRecoilPitchAngle = r;
    return r;
}

// ea: 0x004EEA80
int damageMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iDamage;
    info->iDamage = r;
    return r;
}

// ea: 0x004EEAB0
int meleeDamageMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iMeleeDamage;
    info->iMeleeDamage = r;
    return r;
}

// ea: 0x004EEAE0
int damageInnerRadiusMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iDamageInnerRadius;
    info->iDamageInnerRadius = r;
    return r;
}

// ea: 0x004EEB10
int damageOuterRadiusMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iDamageOuterRadius;
    info->iDamageOuterRadius = r;
    return r;
}

// ea: 0x004EEB40
int minDamagePercentMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iMinDamagePercent;
    info->iMinDamagePercent = r;
    return r;
}

// ea: 0x004EEB70
int fireDelayMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iFireDelay;
    info->iFireDelay = r;
    return r;
}

// ea: 0x004EEBA0
int meleeDelayMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iMeleeDelay;
    info->iMeleeDelay = r;
    return r;
}

// ea: 0x004EEBD0
int fireTimeMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iFireTime;
    info->iFireTime = r;
    return r;
}

// ea: 0x004EEC00
int meleeTimeMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iMeleeTime;
    info->iMeleeTime = r;
    return r;
}

// ea: 0x004EEC30
int explosionRadiusMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iExplosionRadius;
    info->iExplosionRadius = r;
    return r;
}

// ea: 0x004EEC60
int explosionInnerDamageMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iExplosionInnerDamage;
    info->iExplosionInnerDamage = r;
    return r;
}

// ea: 0x004EEC90
int explosionOuterDamageMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iExplosionOuterDamage;
    info->iExplosionOuterDamage = r;
    return r;
}

// ea: 0x004EECC0
int maxAmmoMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iMaxAmmo;
    info->iMaxAmmo = r;
    return r;
}

// ea: 0x004EECF0
double sensitivityScaleMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fSensitivityScale;
    info->fSensitivityScale = r;
    return r;
}

// ea: 0x004EED30
double adsZoomFovMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsZoomFov;
    info->fAdsZoomFov = r;
    return r;
}

// ea: 0x004EED70
int adsTransInTimeMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iAdsTransInTime;
    info->iAdsTransInTime = r;
    return r;
}

// ea: 0x004EEDA0
int adsTransOutTimeMP_Function(int v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    int r = v + info->iAdsTransOutTime;
    info->iAdsTransOutTime = r;
    return r;
}

// ea: 0x004EEDD0
double adsSensitivityScaleMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsSensitivityScale;
    info->fAdsSensitivityScale = r;
    return r;
}

// ea: 0x004EEE10
double adsSpreadMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsSpread;
    info->fAdsSpread = r;
    return r;
}

// ea: 0x004EEE50
double adsSpreadDuckedMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsSpreadDucked;
    info->fAdsSpreadDucked = r;
    return r;
}

// ea: 0x004EEE90
double adsSpreadProneMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fAdsSpreadProne;
    info->fAdsSpreadProne = r;
    return r;
}

// ea: 0x004EEED0
double hipSpreadStandMinMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadStandMin;
    info->fHipSpreadStandMin = r;
    return r;
}

// ea: 0x004EEF10
double hipSpreadDuckedMinMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadDuckedMin;
    info->fHipSpreadDuckedMin = r;
    return r;
}

// ea: 0x004EEF50
double hipSpreadProneMinMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadProneMin;
    info->fHipSpreadProneMin = r;
    return r;
}

// ea: 0x004EEF90
double hipSpreadMaxMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadMax;
    info->fHipSpreadMax = r;
    return r;
}

// ea: 0x004EEFD0
double hipSpreadDecayRateMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadDecayRate;
    info->fHipSpreadDecayRate = r;
    return r;
}

// ea: 0x004EF010
double hipSpreadFireAddMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadFireAdd;
    info->fHipSpreadFireAdd = r;
    return r;
}

// ea: 0x004EF050
double hipSpreadTurnAddMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadTurnAdd;
    info->fHipSpreadTurnAdd = r;
    return r;
}

// ea: 0x004EF090
double hipSpreadMoveAddMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadMoveAdd;
    info->fHipSpreadMoveAdd = r;
    return r;
}

// ea: 0x004EF0D0
double hipSpreadDuckedDecayMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadDuckedDecay;
    info->fHipSpreadDuckedDecay = r;
    return r;
}

// ea: 0x004EF110
double hipSpreadProneDecayMP_Function(float v)
{
    weaponFileInfo_t* info = BG_GetPlayerWeaponInfo();
    if (info == nullptr)
        return 0;
    float r = v + info->fHipSpreadProneDecay;
    info->fHipSpreadProneDecay = r;
    return r;
}

} // namespace weaponFuncs
