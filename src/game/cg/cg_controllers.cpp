// ============================================================================
// cg_controllers.cpp - viewport/client controllers + DObj controllers (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"
#include "game/core/core_types.h"

#include <math.h>

extern int currCl;
extern int dword_F6A28C[4 * 802];
extern int dword_F6A290[4 * 802];
extern int SV_DObjGetBoneIndex(Entity* entity, unsigned int boneNameHash);
extern DObjSkelMat* SV_DObjGetMatrixArray(Entity* entity);
extern int G_DObjSetControlTagAngles(Entity* ent, int* partBits,
                                     unsigned int tag_name_hash,
                                     float* angles);
extern weaponFileInfo_t* BG_GetInfoForWeapon(int weapon);
class Handle;
extern void EffectEventStopEmitting(Handle handle);
extern int PostEffectEventVehicle(const Entity* ent, const char* vehicleType,
                                  int action);
extern float gTurretOldPITCH;
extern float gTurretOldYAW;
extern int gTurretState;
extern int gTurretSoundEffectHandle;
extern int dword_DF91F4;


struct cgs_t {
    int state;       // +0x00 (kLocalPlayerStateUnused = 0)
    int controller;  // +0x04
};
enum { kLocalPlayerStateUnused = 0 };

// ea: 0x00693BC0
bool View_compare_controller_sort(const cgs_t* elem1, const cgs_t* elem2)
{
    if (elem1->state == kLocalPlayerStateUnused)
        return false;
    if (elem2->state == kLocalPlayerStateUnused)
        return true;
    int controller = elem1->controller;
    return controller >= 0 && controller < elem2->controller;
}

// ea: 0x00693D40
int View_GetClientController(int clientIndex)
{
    if (clientIndex != 0)
    {
        CG_ASSERT("clientIndex >= 0 && clientIndex < 1",
                  "c:\\cod\\code\\game\\MultiView.cpp", 218);
    }
    return dword_F6A28C[802 * clientIndex];
}

static unsigned int sHelmetInit = 0;
static unsigned int helmetHash_0 = 0;

// ea: 0x00694480
void HelmetController(Entity* owner)
{
    if ((owner->mFlags & 0x10) != 0)
    {
        if ((sHelmetInit & 1) == 0)
        {
            sHelmetInit |= 1u;
            helmetHash_0 = HashString::CalcHash("Bip01 Helmet");
        }
        int BoneIndex = SV_DObjGetBoneIndex(owner, helmetHash_0);
        if (BoneIndex >= 0)
        {
            DObjSkelMat* v2 =
                &((DObjSkelMat*)SV_DObjGetMatrixArray(owner))[BoneIndex];
            v2->origin[0] = 0.0f;
            v2->origin[1] = 0.0f;
            v2->origin[2] = 0.0f;
            v2->origin[3] = 0.0f;
            v2->axis[0][0] = 0.0f;
            v2->axis[1][0] = 0.0f;
            v2->axis[2][0] = 0.0f;
            v2->axis[0][1] = 0.0f;
            v2->axis[1][1] = 0.0f;
            v2->axis[2][1] = 0.0f;
            v2->axis[0][2] = 0.0f;
            v2->axis[1][2] = 0.0f;
            v2->axis[2][2] = 0.0f;
        }
    }
}

// ea: 0x00694580
void CG_Drone_DoControllers(Entity* entity)
{
    HelmetController(entity);
}

static unsigned int sTurretTagInit = 0;
static unsigned int tag_aim_hash_4 = 0;
static unsigned int tag_aim_animated_hash_0 = 0;
static unsigned int tag_aim_animatedY_hash_0 = 0;
static unsigned int tag_aim_animatedP_hash_0 = 0;

// ea: 0x006A1380
void CG_mg42_DoControllers(Entity* entity, bool playerTurret)
{
    if (!playerTurret)
    {
        unsigned int v3 = entity->r.mOwner.mHandle.mVal & 0xFFF;
        if (v3 < 0x540
            && entity->r.mOwner.mHandle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v3].mKey)
        {
            Entity* mObject = EntityHandleDb::sInst.mElements[v3].mObject;
            if (mObject != nullptr && mObject->active == 1
                && mObject->health > 0)
                return;
        }
    }
    weaponFileInfo_t* InfoForWeapon =
        (weaponFileInfo_t*)BG_GetInfoForWeapon(entity->s.weapon);
    if (InfoForWeapon == nullptr)
    {
        CG_ASSERT("info", "c:\\cod\\code\\game\\cg_ent.cpp", 253);
    }
    if (playerTurret)
    {
        long double v6 = fabs((double)(gTurretOldPITCH
                                       - entity->s.angles2.v.m128_f32[0]));
        float dYaw = (float)fabs((double)(gTurretOldYAW
                                          - entity->s.angles2.v.m128_f32[1]));
        if (v6 > 0.1 || dYaw > 0.1)
        {
            int v7 = gTurretState;
            if (gTurretState <= 1)
            {
                gTurretState = 2;
                goto LABEL_28;
            }
            if (v7 != 2)
            {
                if (v7 == 1)
                    goto LABEL_31;
            LABEL_18:
                if (gTurretSoundEffectHandle != -1)
                {
                    EffectEventStopEmitting(
                        *(Handle*)&gTurretSoundEffectHandle);
                    gTurretSoundEffectHandle = -1;
                }
                if (dword_DF91F4 != -1)
                    dword_DF91F4 = -1;
                goto LABEL_22;
            }
        }
        else
        {
            int v7 = gTurretState;
            if (gTurretState == 2)
            {
                gTurretState = 1;
            LABEL_31:
                if (gTurretSoundEffectHandle != -1)
                {
                    EffectEventStopEmitting(
                        *(Handle*)&gTurretSoundEffectHandle);
                    gTurretSoundEffectHandle = -1;
                }
                if (dword_DF91F4 == -1)
                    dword_DF91F4 = PostEffectEventVehicle(
                        entity, InfoForWeapon->szInternalName,
                        46 /* kActionEI_MELEE_STRUGGLE */);
                goto LABEL_22;
            }
            if (gTurretState == 1)
            {
                gTurretState = 0;
                goto LABEL_18;
            }
        }
    LABEL_28:
        if (gTurretSoundEffectHandle == -1)
            gTurretSoundEffectHandle = PostEffectEventVehicle(
                entity, InfoForWeapon->szInternalName,
                45 /* kActionBAZOOKA_TRAIL */);
    }
LABEL_22:
    gTurretOldPITCH = entity->s.angles2.v.m128_f32[0];
    gTurretOldYAW = entity->s.angles2.v.m128_f32[1];
LABEL_23:
    float angles[3];
    angles[0] = entity->s.angles2.v.m128_f32[0];
    angles[1] = entity->s.angles2.v.m128_f32[1];
    angles[2] = 0.0f;
    int partBits[4] = {0, 0, 0, 0};
    if ((sTurretTagInit & 1) == 0)
    {
        sTurretTagInit |= 1u;
        tag_aim_hash_4 = HashString::CalcHash("tag_aim");
    }
    if ((sTurretTagInit & 2) == 0)
    {
        sTurretTagInit |= 2u;
        tag_aim_animated_hash_0 = HashString::CalcHash("tag_aim_animated");
    }
    if ((sTurretTagInit & 4) == 0)
    {
        sTurretTagInit |= 4u;
        tag_aim_animatedY_hash_0 =
            HashString::CalcHash("tag_aim_animatedY");
    }
    if ((sTurretTagInit & 8) == 0)
    {
        sTurretTagInit |= 8u;
        tag_aim_animatedP_hash_0 =
            HashString::CalcHash("tag_aim_animatedP");
    }
    G_DObjSetControlTagAngles(entity, partBits, tag_aim_hash_4, angles);
    G_DObjSetControlTagAngles(entity, partBits, tag_aim_animated_hash_0,
                              angles);
    angles[0] = 0.0f;
    angles[1] = entity->s.angles2.v.m128_f32[1];
    angles[2] = 0.0f;
    G_DObjSetControlTagAngles(entity, partBits, tag_aim_animatedY_hash_0,
                              angles);
    angles[0] = entity->s.angles2.v.m128_f32[0];
    angles[1] = 0.0f;
    angles[2] = 0.0f;
    G_DObjSetControlTagAngles(entity, partBits, tag_aim_animatedP_hash_0,
                              angles);
}

// ea: 0x006A1A80
void CG_DoControllers(Entity* entity)
{
    switch (entity->s.eType)
    {
    case 7u:
    case 0xBu:
    case 0xDu:
        HelmetController(entity);
        break;
    case 0xAu:
        CG_mg42_DoControllers(entity, false);
        break;
    default:
        return;
    }
}
