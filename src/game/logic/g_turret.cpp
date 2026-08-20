// ============================================================================
// g_turret.cpp - turret system (g.o: g_misc.cpp turret block)
// Reconstructed from IDA (release /O2; disasm ground truth).
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "core/ae_fixed_string.h"

// file-local hash caches (g_misc.cpp statics)
static unsigned int turretFlashTagHashes[2];        // 0xEF3818
static unsigned int tag_aim_hash;
static unsigned int tag_aim_hash_0;
static unsigned int tag_aim_hash_2;
static unsigned int tag_aim_hash_3;
static unsigned int tag_butt_hash;
static unsigned int tag_weapon_hash;
static unsigned int tag_aim_animated_hash;
static unsigned int tag_aim_animatedY_hash;
static unsigned int tag_aim_animatedP_hash;
static unsigned int TAG_FLASH_hash;
static unsigned int TAG_FLASH_hash_0;
static unsigned int TAG_FLASH_hash_1;
static unsigned int weaponinfo_hash;
static unsigned int weaponinfo_hash_0;
static unsigned int rightarc_hash;
static unsigned int leftarc_hash;
static unsigned int toparc_hash;
static unsigned int bottomarc_hash;
static unsigned int convergencetime_hash;
static unsigned int maxrange_hash;
static unsigned int damage_hash;
static unsigned int accuracy_hash;

// one-time init guard flags ($S56_3 .. $S66_1 in the binary)
static unsigned char s_initFillWeaponParms = 0;
static unsigned char s_initCanTargetPoint = 0;
static unsigned char s_initCanTargetSentient = 0;
static unsigned char s_initAimatSentientInternal = 0;
static unsigned char s_initAimatSentient = 0;
static unsigned char s_initThinkInit = 0;
static unsigned char s_initCanuseAuto = 0;
static unsigned char s_initController = 0;
static unsigned char s_initXAnimPrecache = 0;
static unsigned char s_initSpawnTurret = 0;
static unsigned char s_initSPTurret = 0;

// EAction bits used by turret effect events (verified disasm)
enum {
    kActionWEAPON_LAST_SHOT_EJECT_FULL = 0x36,  // kActionMax | kActionWEAPON_LAST_SHOT_EJECT
    kActionWEAPON_PICKUP_FULL = 0x37,           // kActionMax | kActionWEAPON_PICKUP
    kActionWEAPON_NOTE_TRACK_SOUND_C = 0x19,
    kActionWEAPON_NOTE_TRACK_SOUND_D = 0x1A,
    kActionVEHICLE_IDLE = 0x1B,
};

// scr_data_t.anim.weapons[weapon].bro_func - g_scr_data + 0x10 (anim) + 0x150 (weapons)
static void*& ScrWeaponBroFunc(unsigned int weapon)
{
    return *(void**)((unsigned char*)&g_scr_data + 0x160 + weapon * 12);
}

namespace BrocHelper {
unsigned int (*GetBroFuncByName(const char* name, bool enforceExists))(void*);  // scr.o 0x99D710
}

// ea: 0x0044BAC0
void InvalidateTurretCaches(void)
{
    turretInfo[0].obstruction = nullptr;  // dword_ED9E7C == turretInfo[0].obstruction
}

// ea: 0x0044BB10
void G_InitTurrets(void)
{
    turretInfo[0].inuse = 0;
    level.turrets = turretInfo;
}

// ea: 0x0044BB30
void turret_clientaim(Entity* self, Entity* other)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 503;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (other == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 505;
        AeAssert::gCurrentExpr = "other";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (other->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 506;
        AeAssert::gCurrentExpr = "other->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (self->active == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 507;
        AeAssert::gCurrentExpr = "self->active";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (self->r.mOwner.mHandle.mVal != other->mHandle.mHandle.mVal)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 508;
        AeAssert::gCurrentExpr = "self->r.mOwner == other->GetHandle()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    other->client->ps.viewlocked = 1;
    other->client->ps.mViewLockedEntity.mHandle.mVal = self->mHandle.mHandle.mVal;

    float localAngles[2];
    int clamped = 0;
    for (int i = 0; i < 2; ++i)
    {
        float diff = AngleSubtract(other->client->ps.viewangles[i],
                                   self->r.currentAngles.v.m128_f32[i]);
        localAngles[i] = diff;
        if (diff > pTurretInfo->arcmax[i] || pTurretInfo->arcmin[i] > diff)
        {
            if (diff > pTurretInfo->arcmax[i])
                localAngles[i] = pTurretInfo->arcmax[i];
            else
                localAngles[i] = pTurretInfo->arcmin[i];
            clamped = 1;
        }
        diff = AngleSubtract(localAngles[i], self->s.angles2.v.m128_f32[i]);
        float maxStep = ServerTime::sInst.mTickDelta * 300.0f;
        if (fabsf(diff) > maxStep)
        {
            clamped = 1;
            if (diff <= 0.0f)
                localAngles[i] = self->s.angles2.v.m128_f32[i] - maxStep;
            else
                localAngles[i] = self->s.angles2.v.m128_f32[i] + maxStep;
        }
    }
    self->s.angles2.v.m128_f32[0] = localAngles[0];
    self->s.angles2.v.m128_f32[1] = localAngles[1];
    self->s.angles2.v.m128_f32[2] = 0.0f;
    uint16_t turret_flags = pTurretInfo->turret_flags;
    if ((turret_flags & 0x800) != 0)
    {
        pTurretInfo->turret_flags = turret_flags & 0xF7FF;
        self->s.eFlags ^= 8u;
    }
    if (clamped != 0)
    {
        float angles[3];
        angles[0] = self->r.currentAngles.v.m128_f32[0] + localAngles[0];
        angles[1] = self->r.currentAngles.v.m128_f32[1] + localAngles[1];
        angles[2] = 0.0f;
        SetClientViewAngle(other, angles);
    }
    CG_mg42_DoControllers(self, true);
}

// ea: 0x0044BE60
int turret_IsFiringInternal(int state)
{
    return state != 0;
}

// ea: 0x0044BE70
int turret_IsFiring(Entity* self)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 883;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return pTurretInfo->turret_state != 0;
}

// ea: 0x0044BED0
void turret_SetTargetEnt(Entity* self, Entity* pEnt)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1222;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1223;
        AeAssert::gCurrentExpr = "pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pTurretInfo->target != pEnt)
    {
        pTurretInfo->target = pEnt;
        pTurretInfo->turret_flags &= ~8u;
        pTurretInfo->targetTime = level.time;
    }
}

// ea: 0x0044BF80
int turret_behind(Entity* self, Entity* other)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (other->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2248;
        AeAssert::gCurrentExpr = "other->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float yawSpan = (fabsf(pTurretInfo->arcmax[1]) + fabsf(pTurretInfo->arcmin[1])) * 0.5f;
    float dot = AngleNormalize180(yawSpan + (self->r.currentAngles.v.m128_f32[1]
                                            + pTurretInfo->arcmin[1]));
    float forward[3];
    YawVectors(dot, forward, nullptr);
    VectorNormalize(forward);
    float dir[3];
    dir[0] = self->r.currentOrigin.v.m128_f32[0] - other->r.currentOrigin.v.m128_f32[0];
    dir[1] = self->r.currentOrigin.v.m128_f32[1] - other->r.currentOrigin.v.m128_f32[1];
    dir[2] = 0.0f;
    float v5 = VectorNormalize(dir);
    Q_acos((forward[0] * dir[0]) + (forward[1] * dir[1]) + (forward[2] * dir[2]));
    return v5 * 180.0f * 0.31830987f <= yawSpan;
}

// ea: 0x0044C100
int G_IsTurretUsable(Entity* self, Entity* owner)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2298;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return (pTurretInfo->turret_flags & 0x1000) != 0
        && self->active == 0
        && self->pTurretInfo != nullptr
        && self->takedamage != 0
        && turret_behind(self, owner) != 0
        && owner->client->ps.grenadeTimeLeft == 0
        && owner->client->ps.mGroundEntity.mHandle.mVal != 0;
}

// ea: 0x004589A0
int Turret_FillWeaponParms(Entity* ent, Entity* activator, weaponParms* wp, int barrelNum)
{
    if (barrelNum >= 2)
        return 0;
    if (!(s_initFillWeaponParms & 1))
    {
        s_initFillWeaponParms |= 1;
        turretFlashTagHashes[0] = HashString::CalcHash("tag_flash");
        turretFlashTagHashes[1] = HashString::CalcHash("tag_flash_2");
    }
    int BoneIndex = SV_DObjGetBoneIndex(ent, turretFlashTagHashes[barrelNum]);
    if (BoneIndex < 0)
    {
        if (barrelNum == 0)
        {
            const char* v7 = ent->mClassName.c_str();
            if (v7 == nullptr)
                v7 = defaultFileName;
            Com_Printf("Couldn't find %d flash tag on turret (entity %d, classname '%s').\n",
                       0, ent->mHandle.mHandle.mVal, v7);
        }
        return 0;
    }
    DObjSkelMat flashTag;
    G_DObjGetWorldBoneIndexMatrix(ent, BoneIndex, &flashTag);
    float angles[6];
    angles[1] = ent->r.currentAngles.v.m128_f32[1] + ent->s.angles2.v.m128_f32[1];
    angles[2] = ent->r.currentAngles.v.m128_f32[2];
    angles[0] = (ent->r.currentAngles.v.m128_f32[0] + ent->s.angles2.v.m128_f32[2])
              + ent->s.angles2.v.m128_f32[0];
    AngleVectors(angles, wp->forward, wp->right, wp->up);
    memcpy(wp->gunForward, &flashTag, sizeof(wp->gunForward));
    memcpy(wp->muzzleTrace, flashTag.origin, sizeof(wp->muzzleTrace));
    return 1;
}

// ea: 0x00458B30
void clamp_playerbehindgun(Entity* self, Entity* other)
{
    float forward[3];
    float point[3];
    point[0] = self->r.currentAngles.v.m128_f32[0] + self->s.angles2.v.m128_f32[0];
    point[1] = self->r.currentAngles.v.m128_f32[1] + self->s.angles2.v.m128_f32[1];
    point[2] = self->r.currentAngles.v.m128_f32[2];
    AnglesToForward(point, forward);
    point[0] = self->r.currentOrigin.v.m128_f32[0] - (forward[0] * 34.0f);
    point[1] = self->r.currentOrigin.v.m128_f32[1] - (forward[1] * 34.0f);
    point[2] = other->r.currentOrigin.v.m128_f32[2];
    SV_UnlinkEntity(other);
    other->client->ps.origin.v.m128_f32[0] = point[0];
    other->client->ps.origin.v.m128_f32[1] = point[1];
    other->client->ps.origin.v.m128_f32[2] = point[2];
    BG_PlayerStateToEntityState(&other->client->ps, &other->s, 1);
    if (IS_NAN(other->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(other->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(other->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 485;
        AeAssert::gCurrentExpr = "!IS_NAN((other->r.currentOrigin)[0]) && !IS_NAN((other->r.currentOrigin)[1]) && !IS_NAN((ot"
                                 "her->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    other->r.currentOrigin.v.m128_f32[0] = other->client->ps.origin.v.m128_f32[0];
    other->r.currentOrigin.v.m128_f32[1] = other->client->ps.origin.v.m128_f32[1];
    other->r.currentOrigin.v.m128_f32[2] = other->client->ps.origin.v.m128_f32[2];
    g_LinkEntity(other);
}

// ea: 0x00458CE0
int turret_CanTargetPoint(Entity* self, const math::Position3& vPoint,
                          float* vSource, float* localAngles)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 995;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("missing turretinfo"))
            __debugbreak();
    }
    float vDelta[3];
    float angles[3];
    if (self->IsVisible() != 0)
    {
        if (!(s_initCanTargetPoint & 1))
        {
            s_initCanTargetPoint |= 1;
            TAG_FLASH_hash = HashString::CalcHash("tag_flash");
        }
        int BoneIndex = SV_DObjGetBoneIndex(self, TAG_FLASH_hash);
        if (BoneIndex < 0)
            return 0;
        DObjSkelMat tagFlashMat;
        G_DObjGetWorldBoneIndexMatrix(self, BoneIndex, &tagFlashMat);
        vDelta[0] = vPoint.v.m128_f32[0] - tagFlashMat.origin[0];
        vDelta[1] = vPoint.v.m128_f32[1] - tagFlashMat.origin[1];
        vDelta[2] = vPoint.v.m128_f32[2] - tagFlashMat.origin[2];
        vSource[0] = tagFlashMat.origin[0];
        vSource[1] = tagFlashMat.origin[1];
        vSource[2] = tagFlashMat.origin[2];
    }
    else
    {
        vDelta[0] = vPoint.v.m128_f32[0] - self->r.currentOrigin.v.m128_f32[0];
        vDelta[1] = vPoint.v.m128_f32[1] - self->r.currentOrigin.v.m128_f32[1];
        vDelta[2] = vPoint.v.m128_f32[2] - self->r.currentOrigin.v.m128_f32[2];
        vSource[0] = self->r.currentOrigin.v.m128_f32[0];
        vSource[1] = self->r.currentOrigin.v.m128_f32[1];
        vSource[2] = self->r.currentOrigin.v.m128_f32[2];
    }
    vectosignedangles(vDelta, angles);
    for (int i = 0; i < 2; ++i)
    {
        localAngles[i] = AngleSubtract(angles[i], self->r.currentAngles.v.m128_f32[i]);
        if (localAngles[i] > pTurretInfo->arcmax[i] || pTurretInfo->arcmin[i] > localAngles[i])
            return 0;
    }
    return 1;
}

// ea: 0x00458EF0
int turret_CanTargetSentient(Entity* self, sentient_s* sentient,
                             float* vPoint, float* vSource, float* localAngles)
{
    Sentient_GetEyePosition(sentient, vPoint);
    if (!(s_initCanTargetSentient & 1))
    {
        s_initCanTargetSentient |= 1;
        TAG_FLASH_hash_0 = HashString::CalcHash("tag_flash");
    }
    int BoneIndex = SV_DObjGetBoneIndex(self, TAG_FLASH_hash_0);
    if (BoneIndex < 0)
        return 0;
    DObjSkelMat tagFlashMat;
    G_DObjGetWorldBoneIndexMatrix(self, BoneIndex, &tagFlashMat);
    if (sentient->bIgnoreMe == 1)
        return 0;
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1054;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float vDelta[3];
    float angles[3];
    vDelta[0] = vPoint[0] - tagFlashMat.origin[0];
    vDelta[1] = vPoint[1] - tagFlashMat.origin[1];
    vDelta[2] = vPoint[2] - tagFlashMat.origin[2];
    vectosignedangles(vDelta, angles);
    float diffYaw = AngleSubtract(angles[1], self->r.currentAngles.v.m128_f32[1]);
    localAngles[1] = diffYaw;
    if (diffYaw > pTurretInfo->arcmax[1] || pTurretInfo->arcmin[1] > diffYaw)
        return 0;
    vSource[0] = tagFlashMat.origin[0];
    vSource[1] = tagFlashMat.origin[1];
    vSource[2] = tagFlashMat.origin[2];

    Entity* pEnt = sentient->pEnt;
    if (pTurretInfo->turret_state != 1)
    {
        vDelta[0] = pEnt->r.currentOrigin.v.m128_f32[0] - tagFlashMat.origin[0];
        vDelta[1] = pEnt->r.currentOrigin.v.m128_f32[1] - tagFlashMat.origin[1];
        vDelta[2] = (pEnt->r.currentOrigin.v.m128_f32[2] - tagFlashMat.origin[2]) + 2.0f;
        float pitch = AngleSubtract(vectosignedpitch(vDelta), self->r.currentAngles.v.m128_f32[0]);
        *localAngles = pitch;
        int bTooLow = 0;
        if (pitch > pTurretInfo->arcmax[0])
            bTooLow = 1;
        else if (pTurretInfo->arcmin[0] > pitch)
            bTooLow = 0;
        else
        {
            vPoint[0] = pEnt->r.currentOrigin.v.m128_f32[0];
            vPoint[1] = pEnt->r.currentOrigin.v.m128_f32[1];
            vPoint[2] = pEnt->r.currentOrigin.v.m128_f32[2] + 2.0f;
            return 1;
        }
        pitch = AngleSubtract(angles[0], self->r.currentAngles.v.m128_f32[0]);
        *localAngles = pitch;
        if (pitch > pTurretInfo->arcmax[0])
        {
            if (bTooLow == 0)
            {
                *localAngles = 0.0f;
                vPoint[2] = tagFlashMat.origin[2];
                return 1;
            }
            return 0;
        }
        if (pTurretInfo->arcmin[0] > pitch)
        {
            if (bTooLow != 0)
            {
                *localAngles = 0.0f;
                vPoint[2] = tagFlashMat.origin[2];
                return 1;
            }
            return 0;
        }
        return 1;
    }

    // turret_state == 1: eye pitch, falling back to body pitch
    float pitch = AngleSubtract(angles[0], self->r.currentAngles.v.m128_f32[0]);
    *localAngles = pitch;
    int bTooLow = 0;
    if (pitch > pTurretInfo->arcmax[0])
        bTooLow = 1;
    else if (pTurretInfo->arcmin[0] > pitch)
        bTooLow = 0;
    else
        return 1;
    vDelta[0] = pEnt->r.currentOrigin.v.m128_f32[0] - tagFlashMat.origin[0];
    vDelta[1] = pEnt->r.currentOrigin.v.m128_f32[1] - tagFlashMat.origin[1];
    vDelta[2] = (pEnt->r.currentOrigin.v.m128_f32[2] - tagFlashMat.origin[2]) + 2.0f;
    pitch = AngleSubtract(vectosignedpitch(vDelta), self->r.currentAngles.v.m128_f32[0]);
    *localAngles = pitch;
    if (pitch > pTurretInfo->arcmax[0])
    {
        if (bTooLow == 0)
        {
            *localAngles = 0.0f;
            vPoint[2] = tagFlashMat.origin[2];
            return 1;
        }
        return 0;
    }
    if (pTurretInfo->arcmin[0] > pitch)
    {
        if (bTooLow != 0)
        {
            *localAngles = 0.0f;
            vPoint[2] = tagFlashMat.origin[2];
            return 1;
        }
        return 0;
    }
    vPoint[0] = pEnt->r.currentOrigin.v.m128_f32[0];
    vPoint[1] = pEnt->r.currentOrigin.v.m128_f32[1];
    vPoint[2] = pEnt->r.currentOrigin.v.m128_f32[2] + 2.0f;
    return 1;
}

// ea: 0x00459330
void turret_think_init(Entity* self, int /*msec*/)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1895;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    self->think = THINK__turret_think;
    self->nextthink = level.time + 1;
    if (!(s_initThinkInit & 1))
    {
        s_initThinkInit |= 1;
        tag_aim_hash = HashString::CalcHash("tag_aim");
    }
    int BoneIndex = SV_DObjGetBoneIndex(self, tag_aim_hash);
    if (BoneIndex < 0)
        return;
    G_DObjCalcBone(self, BoneIndex);
    const DObjSkelMat* v5 = &SV_DObjGetMatrixArray(self)[BoneIndex];
    if (v5 == nullptr)
        return;
    if (!(s_initThinkInit & 2))
    {
        s_initThinkInit |= 2;
        tag_butt_hash = HashString::CalcHash("tag_butt");
    }
    int v6 = SV_DObjGetBoneIndex(self, tag_butt_hash);
    if (v6 < 0)
        return;
    G_DObjCalcBone(self, v6);
    const DObjSkelMat* buttMtx = &SV_DObjGetMatrixArray(self)[v6];
    if (buttMtx == nullptr)
        return;

    float v13[3][3];
    AnglesToAxis(self->r.currentAngles, v13);
    float baseMtx[4][3];
    memcpy(baseMtx[2], &self->r.currentOrigin, sizeof(baseMtx[2]));
    float transDir[3];
    transDir[0] = buttMtx->origin[0] - v5->origin[0];
    transDir[1] = buttMtx->origin[1] - v5->origin[1];
    transDir[2] = buttMtx->origin[2] - v5->origin[2];
    math::Position3 end;
    MatrixTransformVector43(v5->origin, v13, end);
    collision_context_t context(self->mHandle, DbLinkedHandle<EntityHandleDb, Entity>(), 17);
    math::Position3 mins, maxs;
    mins.v = _mm_setzero_ps();
    maxs.v = _mm_setzero_ps();
    float dir[3];
    int v7 = 0;
    for (;;)
    {
        dir[0] = v7 * -3.0f;
        dir[1] = 0.0f;
        dir[2] = 0.0f;
        float v10[3][3];
        AnglesToAxis(dir, v10);
        float angles[3];
        MatrixTransformVector(transDir, v10, angles);
        angles[0] += v5->origin[0];
        angles[1] += v5->origin[1];
        angles[2] += v5->origin[2];
        MatrixTransformVector43(angles, v13, baseMtx[3]);
        trace_t trace;
        memset(&trace, 0, sizeof(trace));
        SV_Trace(&trace, &end, &mins, &maxs, (math::Position3*)baseMtx[3], &context,
                 0, 1, bulletPriorityMap, 1, 0.0f);
        if (trace.normal.v.m128_f32[1] < 1.0f)
            break;
        if (++v7 > 30)
            return;
    }
    pTurretInfo->defaultPitch = dir[0];
}

// ea: 0x00459680
void SP_turret_XAnimPrecache(const char* classname)
{
    if (!(s_initXAnimPrecache & 1))
    {
        s_initXAnimPrecache |= 1;
        weaponinfo_hash = HashString::CalcHash("weaponinfo");
    }
    const char* weaponinfoname = nullptr;
    G_SpawnString(weaponinfo_hash, defaultFileName, &weaponinfoname);
    unsigned char WeaponIndexForName = BG_GetWeaponIndexForName(weaponinfoname);
    if (WeaponIndexForName == 0)
        Com_Error(ERR_DROP, "could not find weapon info '%s' for %s", weaponinfoname, classname);
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(WeaponIndexForName);
    if (InfoForWeapon->weapClass != 7 /* WEAPCLASS_TURRET */)
        Com_Error(ERR_DROP, "weapClass in weapon info '%s' for %s must be 'turret'",
                  weaponinfoname, classname);
    ScrWeaponBroFunc(WeaponIndexForName) = nullptr;
    if (*InfoForWeapon->szScript != 0)
        ScrWeaponBroFunc(WeaponIndexForName) = BrocHelper::GetBroFuncByName(InfoForWeapon->szScript, true);
}

// ea: 0x004623D0
int turret_canuse_auto(Entity* self, actor_s* pActor)
{
    if (pActor == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2067;
        AeAssert::gCurrentExpr = "pActor";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2070;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    sentient_s* pEnemy = pActor->pSentient->pEnemy;
    if (pEnemy == nullptr)
        return true;
    sentient_info_t* v5 = &pActor->sentientInfo[pEnemy - level.sentients];
    if (level.time - v5->iLastKnownPosTime >= 5000)
        return true;
    float v7 = pEnemy->pEnt->r.currentOrigin.v.m128_f32[0] - self->r.currentOrigin.v.m128_f32[0];
    float v6 = pEnemy->pEnt->r.currentOrigin.v.m128_f32[1] - self->r.currentOrigin.v.m128_f32[1];
    float v8 = pEnemy->pEnt->r.currentOrigin.v.m128_f32[2] - self->r.currentOrigin.v.m128_f32[2];
    if ((((v8 * v8) + (v6 * v6)) + (v7 * v7)) >= pTurretInfo->maxRangeSquared
        || VectorDistanceSquared2D(pEnemy->pEnt->r.currentOrigin,
                                   v5->vLastKnownPos) >= 4096.0f)
    {
        return true;
    }
    float vPoint[3], vSource[3];
    if (v5->VisCache.bVisible != 0)
    {
        Sentient_GetEyePosition(pEnemy, vPoint);
        if (turret_CanTargetSentient(self, pEnemy, vPoint, vSource, &vPoint[1]) != 0)
            return true;
    }
    else
    {
        float forward[3];
        AnglesToForward(self->r.currentAngles.v.m128_f32, forward);
        if ((((forward[2] * v8) + (forward[1] * v6)) + (forward[0] * v7)) >= 0.0f)
            return true;
    }
    if (!(s_initCanuseAuto & 1))
    {
        s_initCanuseAuto |= 1;
        tag_weapon_hash = HashString::CalcHash("tag_weapon");
    }
    DObjSkelMat tagMat;
    if (G_DObjGetWorldTagMatrix(self, tag_weapon_hash, &tagMat) == 0)
        return false;
    float tmp[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    tmp[2] = tagMat.axis[2][1];
    if (!(s_initCanuseAuto & 2))
    {
        s_initCanuseAuto |= 2;
        tag_aim_hash_0 = HashString::CalcHash("tag_aim");
    }
    if (G_DObjGetWorldTagMatrix(self, tag_aim_hash_0, &tagMat) == 0)
        return false;
    tmp[4] = tagMat.axis[2][3];
    tmp[0] = tmp[2] - tagMat.axis[2][1];
    tmp[1] = tmp[3] - tagMat.axis[2][2];
    VectorNormalize2D(tmp);
    tmp[2] = (tmp[0] * 30.0f) + tmp[2];
    tmp[3] = (tmp[1] * 30.0f) + tmp[3];
    math::Position3 end;
    Sentient_GetEyePosition(pEnemy, end);
    collision_context_t context(pActor->pEnt->mHandle, pEnemy->pEnt->mHandle, 0x801003);
    math::Position3 mins, maxs;
    mins.v = _mm_setzero_ps();
    maxs.v = _mm_setzero_ps();
    int hit = 0;
    g_SightTrace(&hit, *((const math::Position3*)&tmp[2]), mins, maxs, end, context);
    return hit != 0;
}

// ea: 0x00462810
int turret_canuse_manual(Entity* self, actor_s* pActor)
{
    if (pActor == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2141;
        AeAssert::gCurrentExpr = "pActor";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (self->pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2144;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    sentient_s* pEnemy = pActor->pSentient->pEnemy;
    if (pEnemy != nullptr)
    {
        float dx = pEnemy->pEnt->r.currentOrigin.v.m128_f32[0]
                 - pActor->pEnt->r.currentOrigin.v.m128_f32[0];
        float dy = pEnemy->pEnt->r.currentOrigin.v.m128_f32[1]
                 - pActor->pEnt->r.currentOrigin.v.m128_f32[1];
        float dz = pEnemy->pEnt->r.currentOrigin.v.m128_f32[2]
                 - pActor->pEnt->r.currentOrigin.v.m128_f32[2];
        if ((dx * dx) + (dy * dy) + (dz * dz) < 65536.0f)
            return turret_canuse_auto(self, pActor);
    }
    return 1;
}

// ea: 0x00462930
int turret_canuse(actor_s* pActor, Entity* pTurret)
{
    if (pTurret == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2166;
        AeAssert::gCurrentExpr = "pTurret";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pActor == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2167;
        AeAssert::gCurrentExpr = "pActor";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (Actor_IsUsingTurret(pActor) != 0)
        return 1;
    if (pTurret->active != 0)
        return 0;
    turretInfo_t* pTurretInfo = pTurret->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2175;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((pTurretInfo->turret_flags & 2) != 0)
        return turret_canuse_auto(pTurret, pActor);
    return turret_canuse_manual(pTurret, pActor);
}

// ea: 0x004776F0
void G_ClientStopUsingTurret(Entity* self)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 683;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* mObject = HandleDbToEnt(self->r.mOwner);
    if (mObject->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 686;
        AeAssert::gCurrentExpr = "owner->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    self->s.loopSound = 0;
    char prevStance = pTurretInfo->prevStance;
    if (prevStance != -1)
    {
        if (prevStance == 2)
            G_AddEvent(mObject, 167, 0);
        else if (prevStance == 1)
            G_AddEvent(mObject, 166, 0);
        else
            G_AddEvent(mObject, 165, 0);
        pTurretInfo->prevStance = -1;
    }
    SetClientOrigin(mObject, pTurretInfo->userOrigin);
    SetClientViewAngle(mObject, mObject->r.currentAngles.v.m128_f32);
    g_LinkEntity(mObject);
    mObject->client->ps.eFlags &= 0xFFFF9FFF;
    mObject->client->ps.viewlocked = 0;
    mObject->client->ps.mViewLockedEntity.mHandle.mVal = 0;
    mObject->active = 0;
    self->active = 0;
    self->r.mOwner.mHandle.mVal = 0;
    pTurretInfo->turret_flags &= 0xF7FF;
    Scr_Notify(self, hash_const.turretownerchange, 0);
}

// ea: 0x00477870
int turret_UpdateTargetAngles(Entity* self, float* desiredAngles, int bManned)
{
    if (self == nullptr)
        return 0;
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
        return 0;

    float pitch = self->s.angles2.v.m128_f32[0];
    self->s.angles2.v.m128_f32[0] = self->s.angles2.v.m128_f32[2] + self->s.angles2.v.m128_f32[0];
    int bComplete = 1;
    float fSpeed[2];
    if (bManned != 0)
    {
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(self->s.weapon);
        fSpeed[0] = InfoForWeapon->turnSpeed[0];
        fSpeed[1] = BG_GetInfoForWeapon(self->s.weapon)->turnSpeed[1];
        if (fabsf(self->s.angles2.v.m128_f32[0]) <= 17.0f)
        {
            Entity* target = pTurretInfo->target;
            if (target == nullptr
                || (target->sentient != nullptr && target->sentient->bIgnoreMe != 0))
            {
                fSpeed[0] *= 0.5f;
                fSpeed[1] *= 0.5f;
            }
        }
        else
        {
            fSpeed[0] *= 3.0f;
            fSpeed[1] *= 3.0f;
        }
    }
    else
    {
        fSpeed[0] = 200.0f;
        fSpeed[1] = 200.0f;
    }
    uint16_t turret_flags = pTurretInfo->turret_flags;
    if ((turret_flags & 0x200) != 0 && (turret_flags & 0x100) != 0 && fSpeed[0] < 360.0f)
        fSpeed[0] = 360.0f;

    float* pAngles = &self->s.angles2.v.m128_f32[0];
    for (int i = 0; i < 2; ++i)
    {
        fSpeed[i] *= ServerTime::sInst.mTickDelta;
        if (fSpeed[i] <= 0.0f)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
            AeAssert::gCurrentLine = 800;
            AeAssert::gCurrentExpr = "fSpeed[i] > 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        float fDelta = AngleSubtract(desiredAngles[i], pAngles[i]);
        float step;
        if (fDelta > fSpeed[i])
        {
            step = fSpeed[i];
            bComplete = 0;
        }
        else if (-fSpeed[i] > fDelta)
        {
            step = -fSpeed[i];
            bComplete = 0;
        }
        else
        {
            step = fDelta;
        }
        pAngles[i] += step;
    }

    float fDelta = self->s.angles2.v.m128_f32[0];
    self->s.angles2.v.m128_f32[2] = fDelta;
    fDelta = AngleSubtract(fDelta, pitch);
    float v17;
    int v18;
    if (fDelta > fSpeed[0])
    {
        v17 = fSpeed[0];
        v18 = 0;
    }
    else
    {
        if (-fSpeed[0] > fDelta)
        {
            bComplete = 0;
            v17 = -fSpeed[0];
        }
        else
        {
            v17 = fDelta;
        }
        v18 = bComplete;
    }
    float v19 = v17 + pitch;
    self->s.angles2.v.m128_f32[0] = v19;
    self->s.angles2.v.m128_f32[2] = self->s.angles2.v.m128_f32[2] - v19;
    if (v18 != 0)
        Scr_Notify(self, hash_const.turret_on_target, 0);
    return v18;
}

// ea: 0x00477B10
void turret_SetState(Entity* self, int state)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 898;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (turret_IsFiring(self) != (state != 0))
        Scr_Notify(self, hash_const.turretstatechange, 0);
    pTurretInfo->turret_state = state;
}

// ea: 0x00477BA0
void turret_ClearTargetEnt(Entity* self)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 917;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    pTurretInfo->turret_flags &= 0xB7;
    pTurretInfo->target = nullptr;
    turret_SetState(self, 0);
}

// ea: 0x00477C10
int turret_ReturnToDefaultPos(Entity* self, int bManned)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 938;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float desiredAngles[2];
    if (bManned != 0)
    {
        desiredAngles[0] = 0.0f;
        desiredAngles[1] = 0.0f;
    }
    else
    {
        desiredAngles[0] = pTurretInfo->defaultPitch;
        desiredAngles[1] = pTurretInfo->defaultYaw;
    }
    return turret_UpdateTargetAngles(self, desiredAngles, bManned);
}

// ea: 0x00477CA0
int turret_random_aim(Entity* self)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (level.time > pTurretInfo->targetTime + 4500)
    {
        int v2 = rand();
        pTurretInfo->targetTime = level.time - v2 % 3000;
        pTurretInfo->ambientTargetAngles[0] = (float)(v2 % 12) - 6.0f;
        pTurretInfo->ambientTargetAngles[1] = (float)(v2 % 40) - 20.0f;
    }
    return turret_UpdateTargetAngles(self, pTurretInfo->ambientTargetAngles, 1);
}

// ea: 0x00477D30
void turret_aimat_vector_internal(Entity* self, const math::Position3* origin,
                                  int bShoot, float* desiredAngles)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1153;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (bShoot == 0)
    {
        turret_SetState(self, 0);
    }
    else
    {
        if (pTurretInfo->turret_state != 0)
        {
            if (pTurretInfo->turret_state != 2)
                goto L17;
        }
        else
        {
            if (level.time < pTurretInfo->targetTime + 250)
            {
                if (fabsf(AngleSubtract(
                        self->s.angles2.v.m128_f32[2] + self->s.angles2.v.m128_f32[0],
                        desiredAngles[0])) >= 5.0f)
                {
                    goto L17;
                }
                if (fabsf(AngleSubtract(self->s.angles2.v.m128_f32[1],
                                        desiredAngles[1])) >= 5.0f)
                {
                    goto L17;
                }
            }
            turret_SetState(self, 2);
        }
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(self->s.weapon);
        float v7 = fabsf(AngleSubtract(
            self->s.angles2.v.m128_f32[2] + self->s.angles2.v.m128_f32[0],
            desiredAngles[0]));
        if (InfoForWeapon->turnSpeed[0] * 0.2f > v7)
        {
            if (InfoForWeapon->turnSpeed[1] * 0.2f
                > fabsf(AngleSubtract(self->s.angles2.v.m128_f32[1], desiredAngles[1])))
            {
                turret_SetState(self, 1);
            }
        }
    }
L17:
    pTurretInfo->turret_flags |= 0x40;
    pTurretInfo->targetPos[0] = origin->v.m128_f32[0];
    pTurretInfo->targetPos[1] = origin->v.m128_f32[1];
    pTurretInfo->targetPos[2] = origin->v.m128_f32[2];
}

// ea: 0x00477EC0
int turret_aimat_vector(Entity* self, const math::Position3* origin,
                        int bShoot, float* desiredAngles)
{
    float vSource[3];
    if (turret_CanTargetPoint(self, *origin, vSource, desiredAngles) != 0)
    {
        turret_aimat_vector_internal(self, origin, bShoot, desiredAngles);
        return 1;
    }
    if (self->pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1202;
        AeAssert::gCurrentExpr = "self->pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    self->pTurretInfo->turret_flags &= ~0x40;
    return 0;
}

// ea: 0x00477F60
int turret_aimat_Sentient_Internal(Entity* self, sentient_s* pEnemy, int bShoot,
                                   int missTime, float* desiredAngles)
{
    turret_SetTargetEnt(self, pEnemy->pEnt);
    float vPoint[3], vSource[3];
    if (!turret_CanTargetSentient(self, pEnemy, vPoint, vSource, desiredAngles))
        return 0;
    float origin[4];
    origin[0] = vPoint[0];
    origin[1] = vPoint[1];
    origin[2] = vPoint[2];
    origin[3] = 0.0f;
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1259;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (missTime != 0 && level.time < missTime + pTurretInfo->targetTime)
    {
        if (pTurretInfo->turret_flags & 8)
        {
            math::Position3 v20;
            v20 = native_to_cdl_pos3(pTurretInfo->missTarget);
            const math::Position3* v19 = &v20;
            if (turret_aimat_vector(self, v19, bShoot, desiredAngles) != 0)
                return 1;
        }
        else
        {
            pTurretInfo->turret_flags |= 8;
            if (!(s_initAimatSentientInternal & 1))
            {
                s_initAimatSentientInternal |= 1;
                tag_aim_hash_2 = HashString::CalcHash("tag_aim");
            }
            DObjSkelMat tagMat;
            if (G_DObjGetWorldTagMatrix(self, tag_aim_hash_2, &tagMat) == 0)
                return 0;
            float v26 = origin[1] - tagMat.origin[1];
            float v27 = tagMat.origin[0] - origin[0];
            VectorNormalize2D(&v26);
            v26 *= 32.0f;
            v27 *= 32.0f;
            float tmp[3];
            if ((tagMat.axis[0][0] * v26) + (tagMat.axis[0][1] * v27) < 0.0f)
            {
                tmp[0] = origin[0] - v26;
                tmp[1] = origin[1] - v27;
            }
            else
            {
                tmp[0] = origin[0] + v26;
                tmp[1] = origin[1] + v27;
            }
            tmp[2] = pEnemy->pEnt->r.currentOrigin.v.m128_f32[2];
            if (turret_aimat_vector(self, (const math::Position3*)tmp, bShoot, desiredAngles) != 0)
            {
                pTurretInfo->missTarget[0] = tmp[0];
                pTurretInfo->missTarget[1] = tmp[1];
                pTurretInfo->missTarget[2] = tmp[2];
                return 1;
            }
            tmp[2] = origin[2];
            if (turret_aimat_vector(self, (const math::Position3*)tmp, bShoot, desiredAngles) != 0)
            {
                pTurretInfo->missTarget[0] = tmp[0];
                pTurretInfo->missTarget[1] = tmp[1];
                pTurretInfo->missTarget[2] = origin[2];
                return 1;
            }
            pTurretInfo->missTarget[0] = origin[0];
            pTurretInfo->missTarget[1] = origin[1];
            pTurretInfo->missTarget[2] = origin[2];
        }
    }
    turret_aimat_vector_internal(self, (const math::Position3*)origin, bShoot, desiredAngles);
    return 1;
}

// ea: 0x00478270
int turret_aimat_Sentient(Entity* self, sentient_s* pEnemy, int bShoot, int missTime)
{
    float desiredAngles[2];
    if (turret_aimat_Sentient_Internal(self, pEnemy, bShoot, missTime, desiredAngles) != 0)
    {
        turret_UpdateTargetAngles(self, desiredAngles, 1);
        if (self->s.angles2.v.m128_f32[2] == 0.0f)
            return 1;
        float origin[3];
        Sentient_GetEyePosition(pEnemy, origin);
        if (!(s_initAimatSentient & 1))
        {
            s_initAimatSentient |= 1;
            TAG_FLASH_hash_1 = HashString::CalcHash("tag_flash");
        }
        int BoneIndex = SV_DObjGetBoneIndex(self, TAG_FLASH_hash_1);
        if (BoneIndex >= 0)
        {
            DObjSkelMat tagFlashMat;
            G_DObjGetWorldBoneIndexMatrix(self, BoneIndex, &tagFlashMat);
            float vDelta[3];
            vDelta[0] = origin[0] - tagFlashMat.origin[0];
            vDelta[1] = origin[1] - tagFlashMat.origin[1];
            vDelta[2] = origin[2] - tagFlashMat.origin[2];
            float minpitch = vectosignedpitch(vDelta);
            Entity* pEnt = pEnemy->pEnt;
            vDelta[0] = pEnt->r.currentOrigin.v.m128_f32[0] - tagFlashMat.origin[0];
            vDelta[1] = pEnt->r.currentOrigin.v.m128_f32[1] - tagFlashMat.origin[1];
            vDelta[2] = (pEnt->r.currentOrigin.v.m128_f32[2] - tagFlashMat.origin[2]) + 2.0f;
            float maxpitch = vectosignedpitch(vDelta);
            float v7 = minpitch <= maxpitch ? minpitch : maxpitch;
            float v8 = minpitch <= maxpitch ? maxpitch : minpitch;
            float cur = self->s.angles2.v.m128_f32[0];
            float v11;
            if (cur >= v7)
            {
                if (v8 < cur)
                {
                    float v12 = v8 - cur;
                    if (v12 > self->s.angles2.v.m128_f32[2])
                        self->s.angles2.v.m128_f32[2] = v12;
                    return 1;
                }
                v11 = 0.0f;
            }
            else
            {
                v11 = v7 - cur;
            }
            if (self->s.angles2.v.m128_f32[2] > v11)
                self->s.angles2.v.m128_f32[2] = v11;
            return 1;
        }
    }
    return 0;
}

// ea: 0x00478460
int turret_aimat_Ent(Entity* self, Entity* pEnt, int bShoot)
{
    float desiredAngles[2];
    turret_SetTargetEnt(self, pEnt);
    if (turret_aimat_vector(self, &pEnt->r.currentOrigin, bShoot, desiredAngles) == 0)
        return 0;
    turret_UpdateTargetAngles(self, desiredAngles, 1);
    return 1;
}

// ea: 0x004784C0
int turret_isTargetVisable(Entity* self, Entity* target, float* distSqr)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1416;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (target->client != nullptr && G_IsPlayerDrivingVehicle(target))
        return 0;
    float dx = self->r.currentOrigin.v.m128_f32[0] - target->r.currentOrigin.v.m128_f32[0];
    float dy = self->r.currentOrigin.v.m128_f32[1] - target->r.currentOrigin.v.m128_f32[1];
    float dz = self->r.currentOrigin.v.m128_f32[2] - target->r.currentOrigin.v.m128_f32[2];
    float distSq = (dx * dx) + (dy * dy) + (dz * dz);
    if (distSq >= pTurretInfo->maxRangeSquared)
        return 0;
    float flashOrigin[3], vSource[3], localAngles[2];
    if (target->sentient != nullptr)
    {
        if (turret_CanTargetSentient(self, target->sentient, flashOrigin, vSource,
                                     localAngles) == 0)
        {
            return 0;
        }
        Sentient_GetEyePosition(target->sentient, flashOrigin);
    }
    else
    {
        if (turret_CanTargetPoint(self, target->r.currentOrigin, vSource, localAngles) == 0)
            return 0;
        flashOrigin[0] = target->r.currentOrigin.v.m128_f32[0];
        flashOrigin[1] = target->r.currentOrigin.v.m128_f32[1];
        flashOrigin[2] = target->r.currentOrigin.v.m128_f32[2];
    }
    math::Position3 start, end, mins, maxs;
    start.v.m128_f32[0] = vSource[0];
    start.v.m128_f32[1] = vSource[1];
    start.v.m128_f32[2] = vSource[2];
    end.v.m128_f32[0] = flashOrigin[0];
    end.v.m128_f32[1] = flashOrigin[1];
    end.v.m128_f32[2] = flashOrigin[2];
    mins.v = _mm_setzero_ps();
    maxs.v = _mm_setzero_ps();
    int hit = 0;
    collision_context_t context(self->mHandle, target->mHandle, 0x801003);
    SV_SightTrace(&hit, &start, &mins, &maxs, &end, &context, 0);
    TraceDebugLine(start, end, hit, self->mHandle);
    if (hit != 0)
        return 0;
    pTurretInfo->obstruction = nullptr;
    if (distSqr != nullptr)
        *distSqr = distSq;
    return 1;
}

// ea: 0x00478710
sentient_s* turret_findBestTarget(Entity* self)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1477;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    sentient_s* result = nullptr;
    int bHavePrev = 0;
    if ((pTurretInfo->turret_flags & 0x40) != 0)
    {
        Entity* target = pTurretInfo->target;
        if (target != nullptr)
        {
            result = target->sentient;
            if (result != nullptr)
            {
                if (level.time <= pTurretInfo->targetTime + 4000)
                    return result;
                float prevDistSqr = 0.0f;
                if (result->bIgnoreMe == 0
                    && turret_isTargetVisable(self, result->pEnt, &prevDistSqr) != 0)
                {
                    bHavePrev = 1;
                }
            }
        }
    }
    int v6 = Sentient_EnemyTeam(pTurretInfo->eTeam);
    sentient_s* Sentient;
    if (pTurretInfo->prevSentTarget <= -1)
        Sentient = Sentient_FirstSentient(1 << v6);
    else
        Sentient = Sentient_NextSentient(&level.sentients[pTurretInfo->prevSentTarget],
                                         1 << v6);
    sentient_s* v9 = Sentient;
    if (Sentient == nullptr)
    {
        pTurretInfo->prevSentTarget = -1;
        return result;
    }
    pTurretInfo->prevSentTarget = (int16_t)(Sentient - level.sentients);
    if (Sentient->bIgnoreMe != 0)
        return result;
    float newDistSqr = 0.0f;
    if (turret_isTargetVisable(self, Sentient->pEnt, &newDistSqr) != 0)
    {
        if (bHavePrev == 0)
            return v9;
        float prevDx = self->r.currentOrigin.v.m128_f32[0]
                     - result->pEnt->r.currentOrigin.v.m128_f32[0];
        float prevDy = self->r.currentOrigin.v.m128_f32[1]
                     - result->pEnt->r.currentOrigin.v.m128_f32[1];
        float prevDz = self->r.currentOrigin.v.m128_f32[2]
                     - result->pEnt->r.currentOrigin.v.m128_f32[2];
        if (((prevDx * prevDx) + (prevDy * prevDy)) + (prevDz * prevDz) > newDistSqr)
            return v9;
    }
    return result;
}

// ea: 0x004788C0
void turret_think_auto_nonai(Entity* self)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1545;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    sentient_s* BestTarget = turret_findBestTarget(self);
    Entity* manualTarget = pTurretInfo->manualTarget;
    if (BestTarget != nullptr)
    {
        if (turret_aimat_Sentient(self, BestTarget, 1, pTurretInfo->convergenceTime) != 0)
            return;
        if (manualTarget == BestTarget->pEnt)
            goto L15;
    }
    if (manualTarget != nullptr
        && turret_isTargetVisable(self, manualTarget, nullptr) != 0)
    {
        sentient_s* sentient = manualTarget->sentient;
        if (sentient != nullptr)
        {
            if (turret_aimat_Sentient(self, sentient, 1, pTurretInfo->convergenceTime) != 0)
                return;
        }
        else if (turret_aimat_Ent(self, manualTarget, 1) != 0)
        {
            return;
        }
    }
L15:
    turret_ClearTargetEnt(self);
    turret_ReturnToDefaultPos(self, 1);
}

// ea: 0x00478A10
int turret_think_auto(Entity* self, actor_s* pActor)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1606;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    uint16_t turret_flags = pTurretInfo->turret_flags;
    if ((turret_flags & 0x20) == 0)
    {
        float desiredAngles[2];
        desiredAngles[0] = pTurretInfo->defaultPitch;
        desiredAngles[1] = pTurretInfo->defaultYaw;
        turret_UpdateTargetAngles(self, desiredAngles, 0);
        return 1;
    }

    sentient_s* pEnemy = pActor->pSentient->pEnemy;
    if ((turret_flags & 0x10) != 0 && pTurretInfo->detachSentient != nullptr)
        pEnemy = pTurretInfo->detachSentient;
    Entity* hit = pTurretInfo->manualTarget;
    int bTryDetach = level.time - pActor->iStateTime >= 1000;
    if (pActor->mg42stayput != 0)
        bTryDetach = false;
    sentient_info_t* v46 = nullptr;
    float vForward[3];
    float distSquared;
    float v42;
    if (pEnemy == nullptr)
    {
        sentient_s* detachSentient = pTurretInfo->detachSentient;
        if (detachSentient != nullptr
            && pActor->sentientInfo[detachSentient - level.sentients].attackTime <= level.time)
        {
            pTurretInfo->detachSentient = nullptr;
        }
        // Actor_CanAttackAll(pActor) - no-op in release
        goto L53;
    }
    v46 = &pActor->sentientInfo[pEnemy - level.sentients];
    v46->attackTime = level.time + 2000;
    v42 = pEnemy->pEnt->r.currentOrigin.v.m128_f32[1] - self->r.currentOrigin.v.m128_f32[1];
    vForward[0] = pEnemy->pEnt->r.currentOrigin.v.m128_f32[0] - self->r.currentOrigin.v.m128_f32[0];
    distSquared = pEnemy->pEnt->r.currentOrigin.v.m128_f32[2] - self->r.currentOrigin.v.m128_f32[2];
    vForward[1] = ((distSquared * distSquared) + (v42 * v42)) + (vForward[0] * vForward[0]);
    if (pEnemy->bIgnoreMe != 0 || pTurretInfo->maxRangeSquared <= vForward[1])
    {
        v46 = nullptr;
    }
    else
    {
        if (v46->VisCache.bVisible != 0
            && turret_aimat_Sentient(self, pEnemy, 1, pTurretInfo->convergenceTime) != 0)
        {
            pTurretInfo->turret_flags &= ~0x10u;
            v46->attackTime = 0;
            return 1;
        }
        if (bTryDetach)
        {
            float forward[3];
            AnglesToForward(self->r.currentAngles.v.m128_f32, forward);
            bTryDetach = (((forward[2] * distSquared) + (forward[1] * v42))
                          + (forward[0] * vForward[0])) >= 0.0f;
            sentient_s* v16 = pTurretInfo->detachSentient;
            int time;
            if (v16 != nullptr)
            {
                if (v16 != pEnemy)
                {
                    sentient_info_t* v17 = &pActor->sentientInfo[v16 - level.sentients];
                    time = level.time;
                    if (v17->attackTime > level.time)
                        goto L30;
                }
                else
                {
                    goto L30;
                }
            }
            else
            {
                time = level.time;
            }
            if (v46->VisCache.bVisible == 0 && bTryDetach)
                goto L30;
            if (time - v46->iLastKnownPosTime >= 5000)
                goto L30;
            float dx = pEnemy->pEnt->r.currentOrigin.v.m128_f32[0]
                     - v46->vLastKnownPos.v.m128_f32[0];
            float dy = pEnemy->pEnt->r.currentOrigin.v.m128_f32[1]
                     - v46->vLastKnownPos.v.m128_f32[1];
            float dz = pEnemy->pEnt->r.currentOrigin.v.m128_f32[2]
                     - v46->vLastKnownPos.v.m128_f32[2];
            if (((dx * dx) + (dy * dy)) + (dz * dz) >= 4096.0f)
                goto L30;
            math::Position3 v39, source;
            Sentient_GetEyePosition(pActor->pSentient, v39);
            Sentient_GetEyePosition(pEnemy, source);
            collision_context_t context(pActor->pEnt->mHandle, pEnemy->pEnt->mHandle,
                                        0x801003);
            math::Position3 mins, maxs;
            mins.v = _mm_setzero_ps();
            maxs.v = _mm_setzero_ps();
            int hitNum = 0;
            g_SightTrace(&hitNum, v39, mins, maxs, source, context);
            if (hitNum != 0)
                goto L30;
            pTurretInfo->detachSentient = pEnemy;
            if (vForward[1] < 65536.0f)
                v46->attackTime = 0;
        }
    }
L51:
    if (hit == pEnemy->pEnt)
        hit = 0;
L53:
    Entity* target = pTurretInfo->target;
    if (target != nullptr && target->sentient != nullptr)
    {
        sentient_info_t* v29 = &pActor->sentientInfo[target->sentient - level.sentients];
        v46 = v29;
        if (target->sentient->bIgnoreMe == 0 && level.time - v29->iLastKnownPosTime < 5000)
        {
            float v31 = (pTurretInfo->turret_flags & 0x40) != 0
                            ? pTurretInfo->targetPos[2]
                            : v29->vLastKnownPos.v.m128_f32[2] + 32.0f;
            float origin[4];
            origin[0] = v29->vLastKnownPos.v.m128_f32[0];
            origin[1] = v29->vLastKnownPos.v.m128_f32[1];
            origin[2] = v31;
            origin[3] = 0.0f;
            float desiredAngles[2];
            if (turret_aimat_vector(self, (const math::Position3*)origin, 1, desiredAngles) != 0)
            {
                turret_UpdateTargetAngles(self, desiredAngles, 1);
                v46->attackTime = 0;
                if (pTurretInfo->detachSentient == target->sentient)
                    pTurretInfo->detachSentient = nullptr;
                return 1;
            }
        }
        else
        {
            v29->iLastKnownPosTime = 0;
        }
    }
    else if (v46 == nullptr)
    {
        goto L65;
    }
    else
    {
        v46->iLastKnownPosTime = 0;
    }
L65:
    if (hit != 0)
    {
        float hdx = hit->r.currentOrigin.v.m128_f32[0] - self->r.currentOrigin.v.m128_f32[0];
        float hdy = hit->r.currentOrigin.v.m128_f32[1] - self->r.currentOrigin.v.m128_f32[1];
        float hdz = hit->r.currentOrigin.v.m128_f32[2] - self->r.currentOrigin.v.m128_f32[2];
        float hitDist = ((hdx * hdx) + (hdy * hdy)) + (hdz * hdz);
        if (pTurretInfo->maxRangeSquared > hitDist)
        {
            sentient_s* v36 = hit->sentient;
            if (v36 != nullptr)
            {
                if (pActor->sentientInfo[v36 - level.sentients].VisCache.bVisible != 0
                    && turret_aimat_Sentient(self, v36, 1, pTurretInfo->convergenceTime) != 0)
                {
                    return 1;
                }
            }
            else if (Actor_CanSeePointEx(pActor, hit->r.currentOrigin.v.m128_f32,
                                         pActor->fFovDot, pActor->fMaxSightDistSqrd,
                                         DbLinkedHandle<EntityHandleDb, Entity>()) != 0.0f
                     && turret_aimat_Ent(self, hit, 1) != 0)
            {
                return 1;
            }
        }
    }
    if (v46 == nullptr || v46->VisCache.bVisible == 0)
        turret_ClearTargetEnt(self);
    turret_random_aim(self);
    return 1;

L30:
    sentient_s* v19 = pTurretInfo->detachSentient;
    if (v19 != nullptr)
    {
        sentient_info_t* v20 = &pActor->sentientInfo[v19 - level.sentients];
        if (level.time - v20->iLastKnownPosTime < 5000)
        {
            float dx = v19->pEnt->r.currentOrigin.v.m128_f32[0]
                     - v20->vLastKnownPos.v.m128_f32[0];
            float dy = v19->pEnt->r.currentOrigin.v.m128_f32[1]
                     - v20->vLastKnownPos.v.m128_f32[1];
            float dz = v19->pEnt->r.currentOrigin.v.m128_f32[2]
                     - v20->vLastKnownPos.v.m128_f32[2];
            if (((dx * dx) + (dy * dy)) + (dz * dz) < 4096.0f)
            {
                math::Position3 v39, source;
                Sentient_GetEyePosition(pActor->pSentient, v39);
                Sentient_GetEyePosition(v19, source);
                collision_context_t context(pActor->pEnt->mHandle, v19->pEnt->mHandle,
                                            0x801003);
                math::Position3 mins, maxs;
                mins.v = _mm_setzero_ps();
                maxs.v = _mm_setzero_ps();
                int hitNum = 0;
                g_SightTrace(&hitNum, v39, mins, maxs, source, context);
                if (hitNum == 0)
                {
                    if (pTurretInfo->detachSentient == pEnemy)
                    {
                        if ((pTurretInfo->turret_flags & 0x10) != 0
                            || !bTryDetach
                            || turret_ReturnToDefaultPos(self, 1) != 0)
                        {
                            v46->attackTime = 0;
                            return 0;
                        }
                        if (vForward[1] < 65536.0f)
                        {
                            v46->attackTime = 0;
                            return 1;
                        }
                        return 1;
                    }
                }
                else
                {
                    pTurretInfo->detachSentient = nullptr;
                }
            }
            else
            {
                pTurretInfo->detachSentient = nullptr;
            }
        }
        else
        {
            pTurretInfo->detachSentient = nullptr;
        }
    }
    goto L51;
}

// ea: 0x004791F0
int turret_think_manual(Entity* self, actor_s* pActor)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1819;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pActor != nullptr)
    {
        uint16_t turret_flags = pTurretInfo->turret_flags;
        if ((turret_flags & 0x20) == 0)
        {
            turret_ReturnToDefaultPos(self, 0);
            return 1;
        }
        if (pTurretInfo->detachSentient != nullptr)
            return turret_think_auto(self, pActor);
        sentient_s* pEnemy = pActor->pSentient->pEnemy;
        if (pEnemy != nullptr)
        {
            float dx = pEnemy->pEnt->r.currentOrigin.v.m128_f32[0]
                     - self->r.currentOrigin.v.m128_f32[0];
            float dy = pEnemy->pEnt->r.currentOrigin.v.m128_f32[1]
                     - self->r.currentOrigin.v.m128_f32[1];
            float dz = pEnemy->pEnt->r.currentOrigin.v.m128_f32[2]
                     - self->r.currentOrigin.v.m128_f32[2];
            if (pTurretInfo->maxRangeSquared > ((dx * dx) + (dy * dy)) + (dz * dz))
            {
                float ex = pEnemy->pEnt->r.currentOrigin.v.m128_f32[0]
                         - pActor->pEnt->r.currentOrigin.v.m128_f32[0];
                float ey = pEnemy->pEnt->r.currentOrigin.v.m128_f32[1]
                         - pActor->pEnt->r.currentOrigin.v.m128_f32[1];
                float ez = pEnemy->pEnt->r.currentOrigin.v.m128_f32[2]
                         - pActor->pEnt->r.currentOrigin.v.m128_f32[2];
                if (((ex * ex) + (ey * ey)) + (ez * ez) < 65536.0f)
                    return turret_think_auto(self, pActor);
            }
        }
        pTurretInfo->turret_flags = turret_flags & 0xFFEF;
        if (pEnemy != nullptr)
        {
            pActor->sentientInfo[pEnemy - level.sentients].attackTime = level.time + 2000;
        }
        else
        {
            // Actor_CanAttackAll(pActor) - no-op in release
        }
    }
    Entity* manualTarget = pTurretInfo->manualTarget;
    int bShoot = (pTurretInfo->turret_flags & 4) != 0;
    if (manualTarget != nullptr)
    {
        float dx = self->r.currentOrigin.v.m128_f32[0] - manualTarget->r.currentOrigin.v.m128_f32[0];
        float dy = self->r.currentOrigin.v.m128_f32[1] - manualTarget->r.currentOrigin.v.m128_f32[1];
        float dz = self->r.currentOrigin.v.m128_f32[2] - manualTarget->r.currentOrigin.v.m128_f32[2];
        if (pTurretInfo->maxRangeSquared > ((dx * dx) + (dy * dy)) + (dz * dz))
        {
            sentient_s* sentient = manualTarget->sentient;
            if (sentient == nullptr)
            {
                turret_aimat_Ent(self, manualTarget, bShoot);
                return 1;
            }
            if (turret_aimat_Sentient(self, sentient, bShoot, 0) != 0)
                return 1;
        }
    }
    if (pActor != nullptr && manualTarget != nullptr)
    {
        turret_ReturnToDefaultPos(self, 1);
        return 1;
    }
    turret_ClearTargetEnt(self);
    return 1;
}

// ea: 0x00479460
void turret_think(Entity* self, int msec)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1952;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    self->nextthink = level.time + 1;
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(self->s.weapon);
    if (InfoForWeapon == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 1957;
        AeAssert::gCurrentExpr = "pWeap";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pTurretInfo->heat < 1.0f)
    {
        if (pTurretInfo->overheating && pTurretInfo->heat <= 0.5f)
        {
            pTurretInfo->overheating = false;
            if (pTurretInfo->overheatEffect.mVal != 0)
            {
                EffectEventSys::sInst->StopEffect(
                    Handle(pTurretInfo->overheatEffect.mVal), false);
                pTurretInfo->overheatEffect.mVal = 0;
            }
        }
    }
    else
    {
        pTurretInfo->overheating = true;
        PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                              (EAction)kActionWEAPON_PICKUP_FULL);
        Scr_Notify(self, hash_const.overheated, 0);
        if (pTurretInfo->overheatEffect.mVal != 0)
            EffectEventSys::sInst->AdjustEffect_Scale(
                Handle(pTurretInfo->overheatEffect.mVal),
                "EmissionRate", 200.0f);
    }
    float heat = pTurretInfo->heat;
    if (heat <= 0.0f)
        pTurretInfo->heat = 0.0f;
    else
        pTurretInfo->heat = heat - ((msec * InfoForWeapon->fCooldownRate) * 0.001f);
    if (pTurretInfo->heat <= 0.25f)
    {
        if (pTurretInfo->overheatEffect.mVal != 0)
        {
            EffectEventSys::sInst->StopEffect(
                Handle(pTurretInfo->overheatEffect.mVal), false);
            pTurretInfo->overheatEffect.mVal = 0;
        }
    }
    else
    {
        if (pTurretInfo->overheatEffect.mVal == 0)
        {
            pTurretInfo->overheatEffect = PostEffectEventWeapon(
                self, InfoForWeapon->szInternalName,
                (EAction)kActionWEAPON_LAST_SHOT_EJECT_FULL);
        }
        float scalea = (pTurretInfo->heat - 0.25f) * 1.333333333333333f * emissionRate;
        EffectEventSys::sInst->AdjustEffect_Scale(
            Handle(pTurretInfo->overheatEffect.mVal),
            "EmissionRate", scalea);
    }
    Entity* mObject = HandleDbToEnt(self->r.mOwner);
    if (mObject != nullptr && mObject->client != nullptr)
        return;
    uint16_t turret_flags = pTurretInfo->turret_flags;
    if ((turret_flags & 2) != 0 && (turret_flags & 1) == 0)
    {
        turret_think_auto_nonai(self);
        return;
    }
    actor_s* actor = nullptr;
    if (mObject != nullptr)
    {
        actor = mObject->actor;
        if (actor != nullptr && actor->eState[0] == AIS_TURRET)
        {
            if ((turret_flags & 2) != 0)
            {
                if (turret_think_auto(self, mObject->actor) == 0)
                {
                    // Actor_StopUseTurret(actor) - no-op in release
                }
                return;
            }
            goto L46;
        }
        actor = nullptr;
    }
    if ((turret_flags & 1) != 0)
    {
        turret_ReturnToDefaultPos(self, 0);
        return;
    }
    if ((turret_flags & 2) != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2034;
        AeAssert::gCurrentExpr = "!(pTurretInfo->turret_flags & TURRET_AUTO)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
L46:
    if (turret_think_manual(self, actor) == 0 && actor != nullptr)
    {
        // Actor_StopUseTurret(actor) - no-op in release
    }
}

// ea: 0x00479790
void turret_controller(Entity* self, int* const /*partBits*/)
{
    if (!(s_initController & 1))
    {
        s_initController |= 1;
        tag_aim_hash_3 = HashString::CalcHash("tag_aim");
    }
    if (!(s_initController & 2))
    {
        s_initController |= 2;
        tag_aim_animated_hash = HashString::CalcHash("tag_aim_animated");
    }
    if (!(s_initController & 4))
    {
        s_initController |= 4;
        tag_aim_animatedY_hash = HashString::CalcHash("tag_aim_animatedY");
    }
    if (!(s_initController & 8))
    {
        s_initController |= 8;
        tag_aim_animatedP_hash = HashString::CalcHash("tag_aim_animatedP");
    }
    float angles[3];
    angles[0] = self->s.angles2.v.m128_f32[0];
    angles[1] = self->s.angles2.v.m128_f32[1];
    angles[2] = 0.0f;
    int BoneIndex = SV_DObjGetBoneIndex(self, tag_aim_hash_3);
    if (BoneIndex >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, angles, BoneIndex, self, 0);
    int v2 = SV_DObjGetBoneIndex(self, tag_aim_animated_hash);
    if (v2 >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, angles, v2, self, 0);
    float angles2[3];
    angles2[0] = 0.0f;
    angles2[1] = angles[1];
    angles2[2] = 0.0f;
    int v3 = SV_DObjGetBoneIndex(self, tag_aim_animatedY_hash);
    if (v3 >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, angles2, v3, self, 0);
    angles2[0] = angles[0];
    angles2[1] = 0.0f;
    int v4 = SV_DObjGetBoneIndex(self, tag_aim_animatedP_hash);
    if (v4 >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, angles2, v4, self, 0);
}

// ea: 0x00479970
void G_FreeTurret(Entity* self)
{
    Entity* mObject = HandleDbToEnt(self->r.mOwner);
    if (self->pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2277;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (mObject != nullptr)
    {
        if (mObject->client != nullptr)
        {
            G_ClientStopUsingTurret(self);
        }
        else if (mObject->actor != nullptr)
        {
            // Actor_StopUseTurret(mObject->actor) - no-op in release
        }
    }
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    self->active = 0;
    pTurretInfo->inuse = 0;
    self->pTurretInfo = nullptr;
}

// ea: 0x00479A40
void turret_use(Entity* self, Entity* owner, Entity* /*activator*/)
{
    if (owner == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2324;
        AeAssert::gCurrentExpr = "owner";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (owner->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2325;
        AeAssert::gCurrentExpr = "owner->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (owner->actor != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2326;
        AeAssert::gCurrentExpr = "!owner->actor";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2329;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Client* client = owner->client;
    if (client != nullptr && (client->ps.eFlags & 0x6000) != 0)
    {
        G_ClientStopUsingTurret(self);
        return;
    }
    owner->active = 1;
    self->active = 1;
    self->r.mOwner.mHandle.mVal = owner->mHandle.mHandle.mVal;
    owner->client->ps.viewlocked = 1;
    owner->client->ps.mViewLockedEntity.mHandle.mVal = self->mHandle.mHandle.mVal;
    pTurretInfo->turret_flags |= 0x800;
    pTurretInfo->userOrigin[0] = owner->r.currentOrigin.v.m128_f32[0];
    pTurretInfo->userOrigin[1] = owner->r.currentOrigin.v.m128_f32[1];
    pTurretInfo->userOrigin[2] = owner->r.currentOrigin.v.m128_f32[2];
    if ((owner->client->ps.pm_flags & 1) != 0)
        pTurretInfo->prevStance = 2;
    else
        pTurretInfo->prevStance = (owner->client->ps.pm_flags & 2) != 0;
    if (pTurretInfo->stance == 2)
    {
        owner->client->ps.eFlags |= 0x2000u;
        owner->client->ps.eFlags &= 0xFFFFBFFF;
    }
    else if (pTurretInfo->stance == 1)
    {
        owner->client->ps.eFlags |= 0x4000u;
        owner->client->ps.eFlags &= 0xFFFFDFFF;
    }
    else
    {
        owner->client->ps.eFlags |= 0x6000u;
    }
    self->TargetAngles.v.m128_f32[0] = self->r.currentAngles.v.m128_f32[0];
    self->TargetAngles.v.m128_f32[1] = self->r.currentAngles.v.m128_f32[1];
    self->TargetAngles.v.m128_f32[2] = self->r.currentAngles.v.m128_f32[2];
    float pitch = AngleSubtract(owner->client->ps.viewangles[0],
                                self->r.currentAngles.v.m128_f32[0]);
    self->s.angles2.v.m128_f32[0] = pitch;
    float v11 = pTurretInfo->arcmax[0] * 0.33333334f;
    if (pitch > v11)
    {
        self->s.angles2.v.m128_f32[0] = v11;
    }
    else
    {
        v11 = pTurretInfo->arcmin[0] * 0.33333334f;
        if (pitch < v11)
            self->s.angles2.v.m128_f32[0] = v11;
    }
    float yaw = AngleSubtract(owner->client->ps.viewangles[1],
                              self->r.currentAngles.v.m128_f32[1]);
    self->s.angles2.v.m128_f32[1] = yaw;
    if (yaw <= pTurretInfo->arcmax[1])
    {
        if (pTurretInfo->arcmin[1] > yaw)
            self->s.angles2.v.m128_f32[1] = pTurretInfo->arcmin[1];
    }
    else
    {
        self->s.angles2.v.m128_f32[1] = pTurretInfo->arcmax[1];
    }
    float angles[3];
    angles[0] = self->r.currentAngles.v.m128_f32[0] + self->s.angles2.v.m128_f32[0];
    angles[1] = self->r.currentAngles.v.m128_f32[1] + self->s.angles2.v.m128_f32[1];
    angles[2] = 0.0f;
    SetClientViewAngle(owner, angles);
    Scr_Notify(self, hash_const.turretownerchange, 0);
}

// ea: 0x00479D90
void G_SpawnTurret(Entity* self, const char* weaponinfoname)
{
    if (turretInfo[0].inuse != 0)
        Com_Error(ERR_DROP, "Too many turrets - Tell MikeA");
    memset(turretInfo, 0, sizeof(turretInfo_t));
    self->pTurretInfo = turretInfo;
    turretInfo->obstruction = nullptr;
    turretInfo->inuse = 1;
    self->s.weapon = BG_GetWeaponIndexForName(weaponinfoname);
    if (self->s.weapon == 0)
        Com_Error(ERR_DROP, "bad weaponinfo '%s' specified for turret", weaponinfoname);
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(self->s.weapon);
    if (InfoForWeapon->weapClass != 7 /* WEAPCLASS_TURRET */)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2430;
        AeAssert::gCurrentExpr = "pWeap->weapClass == WEAPCLASS_TURRET";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    RegisterItem(self->s.weapon, 1);
    if (*InfoForWeapon->szScript != 0 && ScrWeaponBroFunc(self->s.weapon) == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2437;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            ae_formatted_string<128, unsigned char> v63(
                "function pointer for weapon %s not set - temp workaround - CRS",
                weaponinfoname);
            if (AeAssert::Warning((const char*)v63.mBuff))
                __debugbreak();
        }
    }
    if (!(s_initSpawnTurret & 1))
    {
        s_initSpawnTurret |= 1;
        rightarc_hash = HashString::CalcHash("rightarc");
    }
    turretInfo->fireTime = 0;
    turretInfo->stance = InfoForWeapon->stance;
    turretInfo->prevStance = -1;
    G_SpawnFloat(rightarc_hash, InfoForWeapon->rightArc, &turretInfo->arcmin[1]);
    turretInfo->arcmin[1] = -turretInfo->arcmin[1];
    if (turretInfo->arcmin[1] > 0.0f)
        turretInfo->arcmin[1] = 0.0f;
    turretInfo->initialYawmin = turretInfo->arcmin[1];
    if (!(s_initSpawnTurret & 2))
    {
        s_initSpawnTurret |= 2;
        leftarc_hash = HashString::CalcHash("leftarc");
    }
    G_SpawnFloat(leftarc_hash, InfoForWeapon->leftArc, &turretInfo->arcmax[1]);
    if (turretInfo->arcmax[1] < 0.0f)
        turretInfo->arcmax[1] = 0.0f;
    turretInfo->initialYawmax = turretInfo->arcmax[1];
    if (!(s_initSpawnTurret & 4))
    {
        s_initSpawnTurret |= 4;
        toparc_hash = HashString::CalcHash("toparc");
    }
    G_SpawnFloat(toparc_hash, InfoForWeapon->topArc, &turretInfo->arcmin[0]);
    turretInfo->arcmin[0] = -turretInfo->arcmin[0];
    if (turretInfo->arcmin[0] > 0.0f)
        turretInfo->arcmin[0] = 0.0f;
    if (!(s_initSpawnTurret & 8))
    {
        s_initSpawnTurret |= 8;
        bottomarc_hash = HashString::CalcHash("bottomarc");
    }
    G_SpawnFloat(bottomarc_hash, InfoForWeapon->bottomArc, &turretInfo->arcmax[0]);
    if (turretInfo->arcmax[0] < 0.0f)
        turretInfo->arcmax[0] = 0.0f;
    if (!(s_initSpawnTurret & 0x10))
    {
        s_initSpawnTurret |= 0x10;
        convergencetime_hash = HashString::CalcHash("convergencetime");
    }
    float v41;
    G_SpawnFloat(convergencetime_hash, InfoForWeapon->convergenceTime, &v41);
    if (v41 < 0.0f)
        v41 = 0.0f;
    turretInfo->convergenceTime = (int)((v41 * 1000.0f) + 0.5f);
    if (!(s_initSpawnTurret & 0x20))
    {
        s_initSpawnTurret |= 0x20;
        maxrange_hash = HashString::CalcHash("maxrange");
    }
    float maxRange;
    G_SpawnFloat(maxrange_hash, InfoForWeapon->maxRange, &maxRange);
    if (maxRange <= 0.0f)
        maxRange = 3.4028235e38f;
    turretInfo->maxRangeSquared = maxRange * maxRange;
    turretInfo->defaultPitch = 0.0f;
    if (self->health == 0)
        self->health = 100;
    if (!(s_initSpawnTurret & 0x40))
    {
        s_initSpawnTurret |= 0x40;
        damage_hash = HashString::CalcHash("damage");
    }
    G_SpawnInt(damage_hash, InfoForWeapon->iDamage, &self->damage);
    if (self->damage < 0)
        self->damage = 0;
    if (!(s_initSpawnTurret & 0x80))
    {
        s_initSpawnTurret |= 0x80;
        accuracy_hash = HashString::CalcHash("accuracy");
    }
    G_SpawnFloat(accuracy_hash, InfoForWeapon->accuracy, &turretInfo->accuracy);
    if (turretInfo->accuracy < 0.0f)
        turretInfo->accuracy = 0.0f;
    else if (turretInfo->accuracy > 100.0f)
        turretInfo->accuracy = 100.0f;
    turretInfo->turret_flags = 4099;  // TURRET_USABLE | TURRET_AUTO | TURRET_MANUAL
    turretInfo->turret_state = 0;
    turretInfo->prevSentTarget = -1;
    turretInfo->eTeam = TEAM_NEUTRAL;
    self->flags |= 0x8000;
    self->clipmask = 1;
    self->r.contents = 0x200004;
    self->r.svFlags = 128;
    self->s.eType = 10;  // ET_MG42
    G_DObjUpdate(self, false);
    self->r.mins.v.m128_f32[0] = -32.0f;
    self->r.mins.v.m128_f32[1] = -32.0f;
    self->r.mins.v.m128_f32[2] = 0.0f;
    self->r.maxs.v.m128_f32[0] = 32.0f;
    self->r.maxs.v.m128_f32[1] = 32.0f;
    self->r.maxs.v.m128_f32[2] = 56.0f;
    G_SetOrigin(self, self->r.currentOrigin);
    G_SetAngle(self, self->r.currentAngles);
    self->s.angles2.v.m128_f32[1] = 0.0f;
    self->s.angles2.v.m128_f32[0] = 0.0f;
    self->think = THINK__turret_think_init;
    self->nextthink = level.time + 1;
    self->controller = 1;
    self->use = 4;
    self->s.apos.trType = TR_LINEAR_STOP;
    self->takedamage = 1;
    self->entinfo = 2;
    if (self->r.mOwner.mHandle.mVal != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 2555;
        AeAssert::gCurrentExpr = "self->r.mOwner == TEntityHandle::NullHandle()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    SV_LinkEntity(self);
}

// ea: 0x0047A660
void SP_turret(Entity* self)
{
    if (!(s_initSPTurret & 1))
    {
        s_initSPTurret |= 1;
        weaponinfo_hash_0 = HashString::CalcHash("weaponinfo");
    }
    const char* weaponinfoname = nullptr;
    if (G_SpawnString(weaponinfo_hash_0, defaultFileName, &weaponinfoname) == 0)
        Com_Error(ERR_DROP, "no weaponinfo specified for turret");
    G_SpawnTurret(self, weaponinfoname);
}

// ea: 0x0048F570
void turret_shoot_internal(Entity* self, Entity* other)
{
    if (self->pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 564;
        AeAssert::gCurrentExpr = "self->pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (other != nullptr && other->client != nullptr)
    {
        Fire_Lead(self, other, self->damage, 0);
        other->client->ps.viewlocked = 2;
    }
    else
    {
        Fire_Lead(self, other, self->damage,
                  (self->pTurretInfo->turret_flags & 0x40) != 0);
    }
}

// ea: 0x0048F630
void turret_track(Entity* self, Entity* other)
{
    turretInfo_t* pTurretInfo = self->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 588;
        AeAssert::gCurrentExpr = "pTurretInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (self->active == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 590;
        AeAssert::gCurrentExpr = "self->active";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (self->r.mOwner.mHandle.mVal != other->mHandle.mHandle.mVal)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 591;
        AeAssert::gCurrentExpr = "self->r.mOwner == other->GetHandle()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (other->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 592;
        AeAssert::gCurrentExpr = "other->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    turret_clientaim(self, other);
    clamp_playerbehindgun(self, other);
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(self->s.weapon);
    other->client->ps.viewlocked = 1;
    pTurretInfo->fireTime -= (int16_t)ServerTime::sInst.mTickMSec;
    if ((other->client->buttons & 1) == 0 || pTurretInfo->overheating)
    {
        if (self->isFiring != 0)
        {
            self->isFiring = 0;
            if (self->effectLoopingFire.mVal != 0)
                EffectEventKill(self->effectLoopingFire);
            PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                                  (EAction)kActionVEHICLE_IDLE);
        }
    }
    else
    {
        if (pTurretInfo->fireTime <= 0)
        {
            pTurretInfo->fireTime = 0;
            pTurretInfo->heat += (InfoForWeapon->iFireTime * InfoForWeapon->fFireHeat) * 0.001f;
            gFireHeatBlur += (InfoForWeapon->iFireTime * InfoForWeapon->fFireHeat) * 0.0049999999f;
            pTurretInfo->fireTime = (int16_t)InfoForWeapon->iFireTime;
            turret_shoot_internal(self, other);
        }
        if (self->isFiring == 0)
        {
            self->isFiring = 1;
            PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                                  (EAction)kActionWEAPON_NOTE_TRACK_SOUND_C);
            self->effectLoopingFire = PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                                                           (EAction)kActionWEAPON_NOTE_TRACK_SOUND_D);
        }
    }
}

// ea: 0x0048F8D0
void turret_think_client(Entity* self)
{
    Entity* mObject = HandleDbToEnt(self->r.mOwner);
    if (mObject->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 732;
        AeAssert::gCurrentExpr = "owner->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (mObject->active == 1 && mObject->health > 0)
    {
        if (self->active == 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
            AeAssert::gCurrentLine = 736;
            AeAssert::gCurrentExpr = "self->active";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        turret_track(self, mObject);
    }
    else
    {
        G_ClientStopUsingTurret(self);
    }
}

// ea: 0x0048F9D0
void turret_shoot(Entity* self, Entity* owner)
{
    if (owner != nullptr)
    {
        turret_shoot_internal(self, owner);
    }
    else
    {
        Entity* mObject = HandleDbToEnt(self->r.mOwner);
        turret_shoot_internal(self, mObject);
    }
}

// ea: 0x0048F340
void Fire_Lead(Entity* ent, Entity* activator, int damage, int bUseAccuracy)
{
    float spread = (bUseAccuracy == 0) ? 1.0f : 0.0f;
    Entity* v5 = activator;
    if (activator == nullptr || activator->mHandle.mHandle.mVal == 0)
        activator = EntityManager::sInst->mWorld;
    sentient_s* sentient;
    Entity* v8;
    actor_s* actor;
    if (v5 == nullptr
        || (sentient = v5->sentient) == nullptr
        || (sentient = sentient->pEnemy) == nullptr
        || (v8 = sentient->pEnt) == nullptr
        || (actor = v5->actor) == nullptr
        || SmokeGrenadeMgr_EntityCanSeeEntity(SmokeGrenadeMgr::sInst, v5, v8,
                                              actor->fVisibilityThreshold * 0.75f))
    {
        int v10 = 0;
        weaponParms wp;
        for (int i = 0;
             i < 2 && Turret_FillWeaponParms(ent, activator, &wp, i) != 0;
             ++i)
        {
            wp.pWeapInfo = BG_GetInfoForWeapon(ent->s.weapon);
            if (bUseAccuracy != 0)
            {
                turretInfo_t* pTurretInfo = ent->pTurretInfo;
                if ((wp.forward[2] * (pTurretInfo->targetPos[2] - wp.muzzleTrace[2]))
                        + (wp.forward[1] * (pTurretInfo->targetPos[1] - wp.muzzleTrace[1]))
                        + (wp.forward[0] * (pTurretInfo->targetPos[0] - wp.muzzleTrace[0]))
                    > 0.0f)
                    pTurretInfo->turret_flags &= ~0x80u;
                else
                    pTurretInfo->turret_flags |= 0x80u;
                float newForward[3];
                if (rand() * 0.000030517578f
                    < pTurretInfo->accuracy * 0.0099999998f)
                    j_nullsub_54(&wp, pTurretInfo->targetPos, newForward);
                else
                    j_nullsub_47(&wp, pTurretInfo->targetPos, newForward);
                if ((newForward[2] * wp.forward[2])
                        + (newForward[1] * wp.forward[1])
                        + (newForward[0] * wp.forward[0])
                    > 0.995f)
                {
                    wp.forward[0] = newForward[0];
                    wp.forward[1] = newForward[1];
                    wp.forward[2] = newForward[2];
                }
            }
            if (wp.pWeapInfo->type != WEAPTYPE_BULLET)
                Weapon_RocketLauncher_Fire(ent, 0.0f, &wp, 5.0f, false);
            else
                Bullet_Fire(activator, spread, damage, &wp, ent, 0.0f);
            v10 = 1;
        }
        if (v10 == 0)
            return;
        G_AddEvent(ent, 197, 0);
    }
}
