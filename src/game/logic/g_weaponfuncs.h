// ============================================================================
// g_weaponfuncs.h - weaponFuncs stat helpers (game2.o)
// weaponFileInfo_t is the full 0x948 IDA layout (layout twin of the g.o subset
// in g_local.h; never include both in one TU).
// ============================================================================

#pragma once

#include <stddef.h>

struct weaponFileInfo_t {
    int index;  // +0x000
    unsigned int internalNameHash;  // +0x004
    char* szInternalName;  // +0x008
    char* szDisplayName;  // +0x00C
    char* szOverlayName;  // +0x010
    char* szGunXModel;  // +0x014
    char* szHandXModel;  // +0x018
    char* szAttachModel1;  // +0x01C
    char* szAttachModel2;  // +0x020
    char* szAttachModel3;  // +0x024
    char* szAttachModel4;  // +0x028
    char* szAttachModel5;  // +0x02C
    char* szAttachTag1;  // +0x030
    char* szAttachTag2;  // +0x034
    char* szAttachTag3;  // +0x038
    char* szAttachTag4;  // +0x03C
    char* szAttachTag5;  // +0x040
    char* szXAnims[25];  // +0x044
    char* szModeName;  // +0x0A8
    int type;  // +0x0AC
    int weapClass;  // +0x0B0
    int slot;  // +0x0B4
    int bSlotStackable;  // +0x0B8
    int stance;  // +0x0BC
    int ammoType;  // +0x0C0
    int pickupWithoutSelect;  // +0x0C4
    unsigned char mFireSoundMatches[0x208];  // +0x0C8
    unsigned char mFireParticleMatches[0x208];  // +0x2D0
    char* szReticleCenter;  // +0x4D8
    char* szReticleSide;  // +0x4DC
    int iReticleCenterSize;  // +0x4E0
    int iReticleSideSize;  // +0x4E4
    int iReticleMinOfs;  // +0x4E8
    float vSprintMove[3];  // +0x4EC
    float vSprintRot[3];  // +0x4F8
    float vStandMove[3];  // +0x504
    float vStandRot[3];  // +0x510
    float vDuckedOfs[3];  // +0x51C
    float vDuckedMove[3];  // +0x528
    float vDuckedRot[3];  // +0x534
    float vProneOfs[3];  // +0x540
    float vProneMove[3];  // +0x54C
    float vProneRot[3];  // +0x558
    float fPosMoveRate;  // +0x564
    float fPosProneMoveRate;  // +0x568
    float fSprintMoveMinSpeed;  // +0x56C
    float fStandMoveMinSpeed;  // +0x570
    float fDuckedMoveMinSpeed;  // +0x574
    float fProneMoveMinSpeed;  // +0x578
    float fPosRotRate;  // +0x57C
    float fPosProneRotRate;  // +0x580
    float fSprintRotMinSpeed;  // +0x584
    float fStandRotMinSpeed;  // +0x588
    float fDuckedRotMinSpeed;  // +0x58C
    float fProneRotMinSpeed;  // +0x590
    char* szRadiantName;  // +0x594
    char* szWorldModel;  // +0x598
    char* szPickupModel;  // +0x59C
    char* szHudIcon;  // +0x5A0
    char* szModeIcon;  // +0x5A4
    char* szAmmoIcon;  // +0x5A8
    int iStartAmmo;  // +0x5AC
    char* szAmmoName;  // +0x5B0
    int iAmmoIndex;  // +0x5B4
    char* szClipName;  // +0x5B8
    int iClipIndex;  // +0x5BC
    int iMaxAmmo;  // +0x5C0
    int iClipSize;  // +0x5C4
    char* szSharedAmmoCapName;  // +0x5C8
    int iSharedAmmoCapIndex;  // +0x5CC
    int iSharedAmmoCap;  // +0x5D0
    int iDamage;  // +0x5D4
    int iTakeDamage;  // +0x5D8
    int iMinDamagePercent;  // +0x5DC
    int iDamageInnerRadius;  // +0x5E0
    int iDamageOuterRadius;  // +0x5E4
    int iMeleeDamage;  // +0x5E8
    int iDamageType;  // +0x5EC
    int iFireDelay;  // +0x5F0
    int iMeleeDelay;  // +0x5F4
    int iFireTime;  // +0x5F8
    int iRechamberTime;  // +0x5FC
    int iRechamberBoltTime;  // +0x600
    int iHoldFireTime;  // +0x604
    int iMeleeTime;  // +0x608
    int iReloadTime;  // +0x60C
    int iReloadEmptyTime;  // +0x610
    int iReloadAddTime;  // +0x614
    int iReloadStartTime;  // +0x618
    int iReloadStartAddTime;  // +0x61C
    int iReloadEndTime;  // +0x620
    int iDropTime;  // +0x624
    int iRaiseTime;  // +0x628
    int iAltDropTime;  // +0x62C
    int iAltRaiseTime;  // +0x630
    int iFuseTime;  // +0x634
    float fSensitivityScale;  // +0x638
    float fMoveSpeedScale;  // +0x63C
    float fAdsZoomFov;  // +0x640
    float fAdsSensitivityScale;  // +0x644
    float fAdsZoomInFrac;  // +0x648
    float fAdsZoomOutFrac;  // +0x64C
    char* szOverlayShader;  // +0x650
    int overlayReticle;  // +0x654
    float fOverlayWidth;  // +0x658
    float fOverlayHeight;  // +0x65C
    float fAdsBobFactor;  // +0x660
    float fAdsViewBobMult;  // +0x664
    float fHipSpreadStandMin;  // +0x668
    float fHipSpreadDuckedMin;  // +0x66C
    float fHipSpreadProneMin;  // +0x670
    float fHipSpreadMax;  // +0x674
    float fHipSpreadDecayRate;  // +0x678
    float fHipSpreadFireAdd;  // +0x67C
    float fHipSpreadTurnAdd;  // +0x680
    float fHipSpreadMoveAdd;  // +0x684
    float fHipSpreadDuckedDecay;  // +0x688
    float fHipSpreadProneDecay;  // +0x68C
    float fHipReticleSidePos;  // +0x690
    int iAdsTransInTime;  // +0x694
    int iAdsTransOutTime;  // +0x698
    float fAdsIdleAmount;  // +0x69C
    float fHipIdleAmount;  // +0x6A0
    float fIdleCrouchFactor;  // +0x6A4
    float fIdleProneFactor;  // +0x6A8
    float fGunMaxPitch;  // +0x6AC
    float fGunMaxYaw;  // +0x6B0
    float swayMaxAngle;  // +0x6B4
    float swayLerpSpeed;  // +0x6B8
    float swayPitchScale;  // +0x6BC
    float swayYawScale;  // +0x6C0
    float swayHorizScale;  // +0x6C4
    float swayVertScale;  // +0x6C8
    float swayShellShockScale;  // +0x6CC
    float adsSwayMaxAngle;  // +0x6D0
    float adsSwayLerpSpeed;  // +0x6D4
    float adsSwayPitchScale;  // +0x6D8
    float adsSwayYawScale;  // +0x6DC
    float adsSwayHorizScale;  // +0x6E0
    float adsSwayVertScale;  // +0x6E4
    int bTwoHanded;  // +0x6E8
    int bRifleBullet;  // +0x6EC
    int bSemiAuto;  // +0x6F0
    int bBoltAction;  // +0x6F4
    int bADSPositionInfo;  // +0x6F8
    int bRechamberWhileAds;  // +0x6FC
    int bCookOffHold;  // +0x700
    int bNoBounce;  // +0x704
    int bNoTumble;  // +0x708
    int bCanMantle;  // +0x70C
    int bSmoke;  // +0x710
    int bOffHand;  // +0x714
    int bCloth;  // +0x718
    int bClipOnly;  // +0x71C
    int bWideListIcon;  // +0x720
    int bADSFire;  // +0x724
    int bADSOnly;  // +0x728
    int bAnimateCamReload;  // +0x72C
    int bAnimateCamMelee;  // +0x730
    int bAnimateCamFire;  // +0x734
    int bDoNotDrop;  // +0x738
    int bCanSpot;  // +0x73C
    int bHoldToFire;  // +0x740
    char* szKillIcon;  // +0x744
    int bWideKillIcon;  // +0x748
    int bNoPartialReload;  // +0x74C
    int bSegmentedReload;  // +0x750
    int iReloadAmmoAdd;  // +0x754
    int iReloadStartAdd;  // +0x758
    int bSwirlControl;  // +0x75C
    char* szAltWeaponName;  // +0x760
    int iAltWeaponIndex;  // +0x764
    int iShotCount;  // +0x768
    int iDropAmmoMin;  // +0x76C
    int iDropAmmoMax;  // +0x770
    int iTriggerRadius;  // +0x774
    int iExplosionRadius;  // +0x778
    int iExplosionInnerDamage;  // +0x77C
    int iExplosionOuterDamage;  // +0x780
    int iProjectileSpeed;  // +0x784
    int iProjectileSpeedUp;  // +0x788
    char* szProjectileModel;  // +0x78C
    int projExplosion;  // +0x790
    char* szProjExplosionEffect;  // +0x794
    char* szProjExplosionSound;  // +0x798
    int bProjImpactExplode;  // +0x79C
    int lobWeapon;  // +0x7A0
    int iProjectileCount;  // +0x7A4
    int iProjectileRadius;  // +0x7A8
    int iProjectileDelay;  // +0x7AC
    int iProjectileSpacingMin;  // +0x7B0
    int iProjectileSpacingMax;  // +0x7B4
    int iProjectileDLight;  // +0x7B8
    float vProjectileColor[3];  // +0x7BC
    float fAdsAimPitch;  // +0x7C8
    float fAdsCrosshairInFrac;  // +0x7CC
    float fAdsCrosshairOutFrac;  // +0x7D0
    float fAdsGunKickPitchMin;  // +0x7D4
    float fAdsGunKickPitchMax;  // +0x7D8
    float fAdsGunKickYawMin;  // +0x7DC
    float fAdsGunKickYawMax;  // +0x7E0
    float fAdsGunKickAccel;  // +0x7E4
    float fAdsGunKickSpeedMax;  // +0x7E8
    float fAdsGunKickSpeedDecay;  // +0x7EC
    float fAdsGunKickStaticDecay;  // +0x7F0
    float fAdsViewKickPitchMin;  // +0x7F4
    float fAdsViewKickPitchMax;  // +0x7F8
    float fAdsViewKickYawMin;  // +0x7FC
    float fAdsViewKickYawMax;  // +0x800
    float fAdsViewKickCenterSpeed;  // +0x804
    float fAdsViewScatterMin;  // +0x808
    float fAdsViewScatterMax;  // +0x80C
    float fAdsSpread;  // +0x810
    float fAdsSpreadDucked;  // +0x814
    float fAdsSpreadProne;  // +0x818
    float fHipGunKickPitchMin;  // +0x81C
    float fHipGunKickPitchMax;  // +0x820
    float fHipGunKickYawMin;  // +0x824
    float fHipGunKickYawMax;  // +0x828
    float fHipGunKickAccel;  // +0x82C
    float fHipGunKickSpeedMax;  // +0x830
    float fHipGunKickSpeedDecay;  // +0x834
    float fHipGunKickStaticDecay;  // +0x838
    float fHipViewKickPitchMin;  // +0x83C
    float fHipViewKickPitchMax;  // +0x840
    float fHipViewKickYawMin;  // +0x844
    float fHipViewKickYawMax;  // +0x848
    float fHipViewKickCenterSpeed;  // +0x84C
    float fHipViewScatterMin;  // +0x850
    float fHipViewScatterMax;  // +0x854
    float aiEffectiveRange;  // +0x858
    float aiMissRange;  // +0x85C
    float aiDamageMod;  // +0x860
    int iPositionReloadTransTime;  // +0x864
    int iPositionTransBlendTime;  // +0x868
    float leftArc;  // +0x86C
    float rightArc;  // +0x870
    float topArc;  // +0x874
    float bottomArc;  // +0x878
    float accuracy;  // +0x87C
    float turnSpeed[2];  // +0x880
    float convergenceTime;  // +0x888
    float maxRange;  // +0x88C
    float fAnimHorRotateInc;  // +0x890
    float fPlayerPositionDist;  // +0x894
    char* szUseHintString;  // +0x898
    int iUseHintStringIndex;  // +0x89C
    float fTurretFov;  // +0x8A0
    float fFireHeat;  // +0x8A4
    float fCooldownRate;  // +0x8A8
    float horizViewJitter;  // +0x8AC
    float vertViewJitter;  // +0x8B0
    float fBulletConeAngle;  // +0x8B4
    float fAdsBulletConeAngle;  // +0x8B8
    char* szScript;  // +0x8BC
    float fOOPosAnimLength[2];  // +0x8C0
    float fAnimIKOffsetTime;  // +0x8C8
    float fAnimIKOffsetForce;  // +0x8CC
    float fAnimIKOffsetDist;  // +0x8D0
    float fAnimIKPitchTime;  // +0x8D4
    float fAnimIKPitchForce;  // +0x8D8
    float fAnimIKPitchAngle;  // +0x8DC
    float fAnimIKTorsoRecoilPitchTime;  // +0x8E0
    float fAnimIKTorsoRecoilPitchForce;  // +0x8E4
    float fAnimIKTorsoRecoilPitchAngle;  // +0x8E8
    void* pDecals[23];  // +0x8EC
};
static_assert(sizeof(weaponFileInfo_t) == 0x948, "weaponFileInfo_t size mismatch");

weaponFileInfo_t* BG_GetPlayerWeaponInfo();  // game.o 0xA1E6B0

namespace weaponFuncs {

int reticleCenterSize_Function(int v);
int reticleSideSize_Function(int v);
int reticleMinOfs_Function(int v);
float duckedOfsF_Function(float v);
float duckedOfsR_Function(float v);
float duckedOfsU_Function(float v);
float proneOfsF_Function(float v);
float proneOfsR_Function(float v);
float proneOfsU_Function(float v);
float standMoveF_Function(float v);
float standMoveR_Function(float v);
float standMoveU_Function(float v);
float duckedMoveF_Function(float v);
float duckedMoveR_Function(float v);
float duckedMoveU_Function(float v);
float proneMoveF_Function(float v);
float proneMoveR_Function(float v);
float proneMoveU_Function(float v);
float proneRotP_Function(float v);
float proneRotY_Function(float v);
float proneRotR_Function(float v);
float standMoveMinSpeed_Function(float v);
float duckedMoveMinSpeed_Function(float v);
float proneMoveMinSpeed_Function(float v);
float posProneRotRate_Function(float v);
float proneRotMinSpeed_Function(float v);
int damage_Function(int v);
int meleeDamage_Function(int v);
float sensitivityScale_Function(float v);
int damageInnerRadius_Function(int v);
int damageOuterRadius_Function(int v);
int minDamagePercent_Function(int v);
int fireDelay_Function(int v);
int meleeDelay_Function(int v);
int fireTime_Function(int v);
int rechamberTime_Function(int v);
int rechamberBoltTime_Function(int v);
int holdFireTime_Function(int v);
int meleeTime_Function(int v);
int reloadTime_Function(int v);
int reloadEmptyTime_Function(int v);
int reloadAddTime_Function(int v);
int reloadStartTime_Function(int v);
int reloadStartAddTime_Function(int v);
int reloadEndTime_Function(int v);
int dropTime_Function(int v);
int raiseTime_Function(int v);
int altDropTime_Function(int v);
int altRaiseTime_Function(int v);
int fuseTime_Function(int v);
float moveSpeedScale_Function(float v);
float gunMaxPitch_Function(float v);
float gunMaxYaw_Function(float v);
float swayMaxAngle_Function(float v);
float swayLerpSpeed_Function(float v);
float swayPitchScale_Function(float v);
float swayYawScale_Function(float v);
float swayHorizScale_Function(float v);
float swayVertScale_Function(float v);
float swayShellShockScale_Function(float v);
float adsSwayMaxAngle_Function(float v);
float adsSwayLerpSpeed_Function(float v);
float adsSwayPitchScale_Function(float v);
float adsSwayYawScale_Function(float v);
float adsSwayHorizScale_Function(float v);
float adsSwayVertScale_Function(float v);
int takedamage_Function(int v);
int explosionRadius_Function(int v);
int explosionInnerDamage_Function(int v);
int explosionOuterDamage_Function(int v);
int projectileSpeed_Function(int v);
int projectileSpeedUp_Function(int v);
int triggerRadius_Function(int v);
int adsTransInTime_Function(int v);
int adsTransOutTime_Function(int v);
float adsIdleAmount_Function(float v);
float adsZoomFov_Function(float v);
float adsSensitivityScale_Function(float v);
float adsZoomInFrac_Function(float v);
float adsZoomOutFrac_Function(float v);
float adsOverlayWidth_Function(float v);
float adsOverlayHeight_Function(float v);
float adsBobFactor_Function(float v);
float adsViewBobMult_Function(float v);
float adsAimPitch_Function(float v);
float adsCrosshairInFrac_Function(float v);
float adsCrosshairOutFrac_Function(float v);
int adsReloadTransTime_Function(int v);
int adsTransBlendTime_Function(int v);
float adsGunKickPitchMin_Function(float v);
float adsGunKickPitchMax_Function(float v);
float adsGunKickYawMin_Function(float v);
float adsGunKickYawMax_Function(float v);
float adsGunKickAccel_Function(float v);
float adsGunKickSpeedMax_Function(float v);
float adsGunKickSpeedDecay_Function(float v);
float adsGunKickStaticDecay_Function(float v);
float adsViewKickPitchMin_Function(float v);
float adsViewKickPitchMax_Function(float v);
float adsViewKickYawMin_Function(float v);
float adsViewKickYawMax_Function(float v);
float adsViewKickCenterSpeed_Function(float v);
float adsSpread_Function(float v);
float adsSpreadDucked_Function(float v);
float adsSpreadProne_Function(float v);
float hipSpreadStandMin_Function(float v);
float hipSpreadDuckedMin_Function(float v);
float hipSpreadProneMin_Function(float v);
float hipSpreadMax_Function(float v);
float hipSpreadDecayRate_Function(float v);
float hipSpreadFireAdd_Function(float v);
float hipSpreadTurnAdd_Function(float v);
float hipSpreadMoveAdd_Function(float v);
float hipSpreadDuckedDecay_Function(float v);
float hipSpreadProneDecay_Function(float v);
float hipReticleSidePos_Function(float v);
float hipIdleAmount_Function(float v);
float hipGunKickPitchMin_Function(float v);
float hipGunKickPitchMax_Function(float v);
float hipGunKickYawMin_Function(float v);
float hipGunKickYawMax_Function(float v);
float hipGunKickAccel_Function(float v);
float hipGunKickSpeedMax_Function(float v);
float hipGunKickSpeedDecay_Function(float v);
float hipGunKickStaticDecay_Function(float v);
float hipViewKickPitchMin_Function(float v);
float hipViewKickPitchMax_Function(float v);
float hipViewKickYawMin_Function(float v);
float hipViewKickYawMax_Function(float v);
float hipViewKickCenterSpeed_Function(float v);
float aiEffectiveRange_Function(float v);
float aiMissRange_Function(float v);
float aiDamageMod_Function(float v);
float bulletConeAngle_Function(float v);
float adsBulletConeAngle_Function(float v);
float animIKOffsetTime_Function(float v);
float animIKOffsetForce_Function(float v);
float animIKOffsetDist_Function(float v);
float animIKPitchTime_Function(float v);
float animIKPitchForce_Function(float v);
float animIKPitchAngle_Function(float v);
float animIKTorsoRecoilPitchTime_Function(float v);
float animIKTorsoRecoilPitchForce_Function(float v);
float animIKTorsoRecoilPitchAngle_Function(float v);
int damageMP_Function(int v);
int meleeDamageMP_Function(int v);
int damageInnerRadiusMP_Function(int v);
int damageOuterRadiusMP_Function(int v);
int minDamagePercentMP_Function(int v);
int fireDelayMP_Function(int v);
int meleeDelayMP_Function(int v);
int fireTimeMP_Function(int v);
int meleeTimeMP_Function(int v);
int explosionRadiusMP_Function(int v);
int explosionInnerDamageMP_Function(int v);
int explosionOuterDamageMP_Function(int v);
int maxAmmoMP_Function(int v);
float sensitivityScaleMP_Function(float v);
float adsZoomFovMP_Function(float v);
int adsTransInTimeMP_Function(int v);
int adsTransOutTimeMP_Function(int v);
float adsSensitivityScaleMP_Function(float v);
float adsSpreadMP_Function(float v);
float adsSpreadDuckedMP_Function(float v);
float adsSpreadProneMP_Function(float v);
float hipSpreadStandMinMP_Function(float v);
float hipSpreadDuckedMinMP_Function(float v);
float hipSpreadProneMinMP_Function(float v);
float hipSpreadMaxMP_Function(float v);
float hipSpreadDecayRateMP_Function(float v);
float hipSpreadFireAddMP_Function(float v);
float hipSpreadTurnAddMP_Function(float v);
float hipSpreadMoveAddMP_Function(float v);
float hipSpreadDuckedDecayMP_Function(float v);
float hipSpreadProneDecayMP_Function(float v);

} // namespace weaponFuncs
