// ============================================================================
// cg_ents.cpp - local entities, server config parsing, grenade counts (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int currCl;
extern int cgGlobal_time;
extern int dword_F62960[4 * 1580];
extern int dword_F610E4;
extern int dword_F610E8;
extern int dword_F610EC;
extern int dword_F610F0;
extern int dword_F610F4;
extern int dword_F610F8;
extern int dword_F610FC;
extern int dword_F61100;
extern int dword_F61104;
extern int dword_F61108;
extern int dword_F6110C;
extern int dword_F61110;
extern int dword_F61114;
extern int dword_F61118;
extern int dword_F6111C;
extern int dword_F61120;
extern int dword_F61124;
extern int dword_F61128;
extern int dword_F6112C;
extern int dword_F61130;
extern void* EntityManager_mPlayers[16];
extern void* EntityManager_sInst;
extern const char* CL_GetConfigStringC(int index);
extern char* Info_ValueForKey(const char* s, const char* key);
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);
extern unsigned int HashString_CalcHash(const char* str);
extern int Com_BitCheck(const int* array, int bitNum);
extern int BG_GetNumWeapons();
extern int BG_WeaponAmmo(const PlayerState* pPS, int iWeapon);
extern int BG_AmmoForWeapon(int iWeapon);
extern void* BG_GetInfoForWeapon(int weapon);
extern Entity* EntityManager_GetPlayer(void* mgr, int idx);
extern void CG_DebugBox(const float* mins, const float* maxs,
                        const float* color, int depthTest, int duration);
extern void CG_Error(const char* msg, ...);
extern void HelmetController(Entity* owner);
extern char cgsGlobal_mapname[128];

struct localEntity_t {
    localEntity_t* next;  // +0x00
    localEntity_t* prev;  // +0x04
};
extern localEntity_t* cg_localEntities[128];
extern localEntity_t cg_activeLocalEntities;
extern localEntity_t* cg_freeLocalEntities;

struct _cmd_t {
    unsigned int hVal;  // +0x00
};
extern _cmd_t gCG_ServerCommands[16];

extern const float colorRed[4];

// ea: 0x0068B330
void CG_InitLocalEntities()
{
    memset(cg_localEntities, 0, sizeof(cg_localEntities));
    cg_activeLocalEntities.next = &cg_activeLocalEntities;
    cg_activeLocalEntities.prev = &cg_activeLocalEntities;
    cg_freeLocalEntities = cg_localEntities[0];
    for (int i = 0; i < 127; ++i)
        ((localEntity_t*)cg_localEntities[i])->next =
            (localEntity_t*)cg_localEntities[i + 1];
    ((localEntity_t*)cg_localEntities[127])->next = nullptr;
}

// ea: 0x0068B5A0
const char* CG_ConfigString(unsigned int index)
{
    if (index < 0x400)
        return CL_GetConfigStringC(index);
    CG_ASSERT(nullptr, "c:\\cod\\code\\game\\cg_main.cpp", 1215);
    return nullptr;
}

// ea: 0x0068B900
int CG_CheckAmmo()
{
    int v0 = dword_F62960[1580 * currCl];
    int v1 = *(int*)(v0 + 1076);
    if (v1 != 0 || *(int*)(v0 + 1080) != 0)
    {
        int v3 = 0;
        int v4 = 1;
        int result = BG_GetNumWeapons();
        if (result > 1)
        {
            do
            {
                if (((1 << v4) & v1) != 0)
                {
                    result = 1000
                             * *(int*)(dword_F62960[1580 * currCl]
                                       + 4 * BG_AmmoForWeapon(v4) + 340);
                    v3 += result;
                    if (v3 >= 5000)
                        break;
                }
                ++v4;
                result = BG_GetNumWeapons();
            } while (v4 < result);
        }
        return result;
    }
    return 0;
}

// ea: 0x0068BB10
void CG_ParseServerinfo()
{
    const char* ConfigString = CL_GetConfigStringC(0);
    const char* v1 = Info_ValueForKey(ConfigString, "mapname");
    Com_sprintf(cgsGlobal_mapname, 128, "maps/%s.bsp", v1);
}

// ea: 0x0068BCF0
void CG_InitServerCommandHashVals()
{
    memset(gCG_ServerCommands, 0, sizeof(gCG_ServerCommands));
    gCG_ServerCommands[0].hVal = HashString_CalcHash("startCam");
    dword_F610E4 = HashString_CalcHash("stopCam");
    dword_F610E8 = HashString_CalcHash("cp");
    dword_F610EC = HashString_CalcHash("cs");
    dword_F610F0 = HashString_CalcHash("print");
    dword_F610F4 = HashString_CalcHash("gm");
    dword_F610F8 = HashString_CalcHash("gmb");
    dword_F610FC = HashString_CalcHash("object_update");
    dword_F61100 = HashString_CalcHash("object_complete");
    dword_F61104 = HashString_CalcHash("opendeadscreen");
    dword_F61108 = HashString_CalcHash("openvictoryscreen");
    dword_F6110C = HashString_CalcHash("clientLevelShot");
    dword_F61110 = HashString_CalcHash("saveshot");
    dword_F61114 = HashString_CalcHash("mu_play");
    dword_F61118 = HashString_CalcHash("mu_stop");
    dword_F6111C = HashString_CalcHash("snd_fade");
    dword_F61120 = HashString_CalcHash("scr_fade");
    dword_F61124 = HashString_CalcHash("fog");
    dword_F61128 = HashString_CalcHash("ls");
    dword_F6112C = HashString_CalcHash("popupopen");
    dword_F61130 = HashString_CalcHash("popupclose");
}

// ea: 0x006927D0
int CG_GetGrenadeCount()
{
    int grenAmmo = 0;
    int v0 = 1;
    if (BG_GetNumWeapons() < 1)
        return 0;
    do
    {
        if (Com_BitCheck(((Entity*)EntityManager_mPlayers[currCl])->client->ps
                             .weapons,
                         v0) != 0
            && ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->weapClass
                   == 11 /* WEAPCLASS_GRENADE */
            && ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->slot
                   == 12 /* WEAPSLOT_GRENADE */)
        {
            Entity* Player =
                EntityManager_GetPlayer(EntityManager_sInst, currCl);
            grenAmmo += BG_WeaponAmmo(&Player->client->ps, v0);
        }
        ++v0;
    } while (v0 <= BG_GetNumWeapons());
    return grenAmmo;
}

// ea: 0x006928E0
int CG_GetSmokeGrenadeCount()
{
    int grenAmmo = 0;
    int v0 = 1;
    if (BG_GetNumWeapons() < 1)
        return 0;
    do
    {
        if (Com_BitCheck(((Entity*)EntityManager_mPlayers[currCl])->client->ps
                             .weapons,
                         v0) != 0
            && ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->weapClass
                   == 11 /* WEAPCLASS_GRENADE */
            && ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->slot
                   == 13 /* WEAPSLOT_SMOKE_GRENADE */)
        {
            Entity* Player =
                EntityManager_GetPlayer(EntityManager_sInst, currCl);
            grenAmmo += BG_WeaponAmmo(&Player->client->ps, v0);
        }
        ++v0;
    } while (v0 <= BG_GetNumWeapons());
    return grenAmmo;
}

// ea: 0x006929F0
int CG_GetSpecialGrenadeCount()
{
    int grenAmmo = 0;
    int v0 = 1;
    if (BG_GetNumWeapons() < 1)
        return 0;
    do
    {
        if (Com_BitCheck(((Entity*)EntityManager_mPlayers[currCl])->client->ps
                             .weapons,
                         v0) != 0
            && (((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->weapClass
                    == 11 /* WEAPCLASS_GRENADE */
                || ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->weapClass
                       == 19 /* WEAPCLASS_NUM */)
            && ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->slot
                   == 15 /* WEAPSLOT_SPECIAL */)
        {
            Entity* Player =
                EntityManager_GetPlayer(EntityManager_sInst, currCl);
            grenAmmo += BG_WeaponAmmo(&Player->client->ps, v0);
        }
        ++v0;
    } while (v0 <= BG_GetNumWeapons());
    return grenAmmo;
}

// ea: 0x00692C30
bool CG_VehicleActive()
{
    Entity* p = EntityManager_GetPlayer(EntityManager_sInst, currCl);
    return p != nullptr && (p->client->ps.eFlags & 0x100000) != 0;
}

// ea: 0x00694560
void CG_Actor_DoControllers(Entity* entity)
{
    HelmetController(entity);
}

// ea: 0x006945A0
void CG_ActorSpawner(Entity* entity)
{
    float mins[3];
    mins[0] = entity->s.lerpOrigin.v.m128_f32[0];
    mins[1] = entity->s.lerpOrigin.v.m128_f32[1];
    mins[2] = entity->s.lerpOrigin.v.m128_f32[2];
    float v8 = mins[0] - 16.0f;
    float v9 = mins[1] - 16.0f;
    float maxs[3];
    maxs[0] = mins[0] + 16.0f;
    maxs[1] = mins[1] + 16.0f;
    maxs[2] = mins[2] + 72.0f;
    CG_DebugBox(&v8, maxs, colorRed, 1, 0);
}

// ea: 0x006979F0
void CG_FreeLocalEntity(localEntity_t* le)
{
    if (le->prev == nullptr)
        CG_Error("CG_FreeLocalEntity: not active");
    le->prev->next = le->next;
    le->next->prev = le->prev;
    le->next = cg_freeLocalEntities;
    cg_freeLocalEntities = le;
}

// ea: 0x00697A30
localEntity_t* CG_AllocLocalEntity()
{
    localEntity_t* v0 = cg_freeLocalEntities;
    if (cg_freeLocalEntities == nullptr)
    {
        localEntity_t* prev = cg_activeLocalEntities.prev;
        if (prev->prev == nullptr)
            CG_Error("CG_FreeLocalEntity: not active");
        prev->prev->next = prev->next;
        prev->next->prev = prev->prev;
        v0 = prev;
        prev->next = cg_freeLocalEntities;
        cg_freeLocalEntities = prev;
    }
    cg_freeLocalEntities = v0->next;
    memset(v0, 0, sizeof(localEntity_t));
    v0->next = cg_activeLocalEntities.next;
    v0->prev = &cg_activeLocalEntities;
    cg_activeLocalEntities.next->prev = v0;
    cg_activeLocalEntities.next = v0;
    return v0;
}

// ea: 0x00697B40
void CG_CrosshairPlayer(unsigned int* result)
{
    *result = 0;
}

struct refEntity_t2 {
    int   reType;
    int   renderfx;
    float lightingOrigin[3];
    float axis[3][3];
    float scale;
    float origin[3];
    float oldorigin[3];
    void* obj;
    void* entity;
};
struct trajectory_t2 {
    int   trType;
    int   trTime;
    float trBase[3];
    float trDelta[3];
};
struct localEntityFull {
    localEntity_t* next;
    localEntity_t* prev;
    int  leFlags;
    int  leType;
    int  endTime;
    float lifeRate;
    float color[4];
    refEntity_t2 refEntity;
    trajectory_t2 pos;
};
extern int cg_railTrailTime;
extern int cg_tracerChance;
extern int dword_DF6ADC[6];
extern float tracer_info_speed[6];
extern void AxisClear(float (*axis)[3]);
extern void VectorNormalize(float* v);
extern float VectorDistance(const float* v1, const float* v2);
extern void PerpendicularVector(float* dst, const float* src);
extern void CrossProduct(const float* v1, const float* v2, float* cross);
extern void FastSinCos(float radians, float* psin, float* pcos);

// ea: 0x00699610
void CG_RailTrail2(const float* color, const float* start, const float* end)
{
    if (cg_railTrailTime > 0)
    {
        localEntityFull* v3 = (localEntityFull*)CG_AllocLocalEntity();
        v3->leType = 0;
        v3->endTime = cg_railTrailTime + cgGlobal_time;
        v3->lifeRate = 1.0f / (float)cg_railTrailTime;
        v3->refEntity.reType = 6 /* RT_RAIL_CORE */;
        v3->refEntity.origin[0] = start[0];
        v3->refEntity.origin[1] = start[1];
        v3->refEntity.origin[2] = start[2];
        v3->refEntity.oldorigin[0] = end[0];
        v3->refEntity.oldorigin[1] = end[1];
        v3->refEntity.oldorigin[2] = end[2];
        v3->color[0] = color[0];
        v3->color[1] = color[1];
        v3->color[2] = color[2];
        v3->color[3] = 1.0f;
        AxisClear(v3->refEntity.axis);
    }
}

// ea: 0x006996D0
void CG_RailTrail(const float* start, const float* end, float type)
{
    int color[4] = {0, 1065353216, 1065353216, 1065353216};
    if (type == 0.0f)
        color[0] = 0;
    else
    {
        switch ((int)type)
        {
        case 1:
            color[0] = 1065353216;
            color[1] = 0;
            color[2] = 0;
            CG_RailTrail2((const float*)color, start, end);
            return;
        case 0x45:
            color[0] = 1065353216;
            color[1] = 1065353216;
            color[2] = 0;
            CG_RailTrail2((const float*)color, start, end);
            return;
        case 0x46:
            color[0] = 0;
            color[1] = 1065353216;
            color[2] = 0;
            CG_RailTrail2((const float*)color, start, end);
            return;
        case 0x47:
            color[0] = 0;
            color[1] = 0;
            color[2] = 1065353216;
            CG_RailTrail2((const float*)color, start, end);
            return;
        case 0x48:
            color[0] = 1065353216;
            color[1] = 1065353216;
            color[2] = 0;
            CG_RailTrail2((const float*)color, start, end);
            return;
        case 2:
        case 3:
            if ((int)type == 3)
            {
                color[0] = 1065353216;
                color[1] = 0;
                color[2] = 0;
            }
            {
                float v1[3] = {start[0] - (start[0] - end[0]),
                               start[1], start[2]};
                float v2[3] = {start[0], start[1] - (start[1] - end[1]),
                               start[2]};
                float normal[3] = {start[0], start[1],
                                   start[2] - (start[2] - end[2])};
                float diff[3] = {start[0] - end[0], start[1] - end[1],
                                 start[2] - end[2]};
                float up[3] = {end[0] + diff[0], end[1], end[2]};
                float right[3] = {end[0], end[1] + diff[1], end[2]};
                float v6[3] = {end[0], end[1], end[2] + diff[2]};
                CG_RailTrail2((const float*)color, start, v1);
                CG_RailTrail2((const float*)color, start, v2);
                CG_RailTrail2((const float*)color, start, normal);
                CG_RailTrail2((const float*)color, end, up);
                CG_RailTrail2((const float*)color, end, right);
                CG_RailTrail2((const float*)color, end, v6);
                CG_RailTrail2((const float*)color, v2, v6);
                CG_RailTrail2((const float*)color, v6, v1);
                CG_RailTrail2((const float*)color, v1, right);
                CG_RailTrail2((const float*)color, v2, up);
                CG_RailTrail2((const float*)color, up, normal);
                CG_RailTrail2((const float*)color, normal, right);
            }
            return;
        default:
            break;
        }
        if ((int)type == 5)
        {
            float radius = start[0];
            float normal[3] = {0.0f, 0.0f, 1.0f};
            float right[3], up[3];
            PerpendicularVector(right, normal);
            CrossProduct(normal, right, up);
            float v[48];
            for (int v17 = 0; v17 < 16; ++v17)
            {
                float s, c;
                FastSinCos(v17 * 0.39269909f, &s, &c);
                v[v17 * 3 + 0] =
                    (right[0] * (c * radius) + up[0] * (s * radius)) + end[0];
                v[v17 * 3 + 1] =
                    (right[1] * (c * radius) + up[1] * (s * radius)) + end[1];
                v[v17 * 3 + 2] =
                    (right[2] * (c * radius) + up[2] * (s * radius)) + end[2];
            }
            for (int i = 0; i < 16; ++i)
                CG_RailTrail2((const float*)color, &v[i * 3],
                              &v[3 * ((i + 1) & 0xF)]);
        }
    }
}

// ea: 0x0069AAE0
void CG_SpawnTracer(const math::Position3* pstart,
                    const math::Position3* pend, int ammo)
{
    float start[3] = {pstart->v.m128_f32[0], pstart->v.m128_f32[1],
                      pstart->v.m128_f32[2]};
    float end[3] = {pend->v.m128_f32[0], pend->v.m128_f32[1],
                    pend->v.m128_f32[2]};
    float dir[3] = {end[0] - start[0], end[1] - start[1],
                    end[2] - start[2]};
    VectorNormalize(dir);
    if (ammo >= 6)
        CG_ASSERT("ammo < WEAPAMMOTYPE_NUM",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 4495);
    int v8 = dword_DF6ADC[4 * ammo];
    end[0] = end[0] - (v8 * dir[0]);
    end[1] = end[1] - (v8 * dir[1]);
    end[2] = end[2] - (v8 * dir[2]);
    v8 >>= 1;
    start[0] = (v8 * dir[0]) + start[0];
    start[1] = (v8 * dir[1]) + start[1];
    start[2] = (v8 * dir[2]) + start[2];
    float dist = VectorDistance(start, end);
    localEntityFull* v9 = (localEntityFull*)CG_AllocLocalEntity();
    v9->leFlags = ammo;
    v9->leType = 2;
    float speed = tracer_info_speed[ammo];
    int time = cgGlobal_time;
    v9->endTime = cgGlobal_time - (int)((dist / speed) * -1000.0f);
    v9->pos.trType = 2 /* TR_LINEAR */;
    v9->pos.trTime = time + 30;
    v9->pos.trBase[0] = start[0];
    v9->pos.trBase[1] = start[1];
    v9->pos.trBase[2] = start[2];
    v9->pos.trDelta[0] = speed * dir[0];
    v9->pos.trDelta[1] = speed * dir[1];
    v9->pos.trDelta[2] = speed * dir[2];
}

// ea: 0x0069CB80
void CG_EventSpawnTracer(const math::Position3* pstart,
                         const math::Position3* pend, int weapon)
{
    float tracerChance = *(float*)&cg_tracerChance;
    weaponFileInfoFull* InfoForWeapon =
        (weaponFileInfoFull*)BG_GetInfoForWeapon(weapon);
    if (InfoForWeapon != nullptr)
    {
        int ammoType = InfoForWeapon->ammoType;
        if (ammoType == 2 /* WEAPAMMOTYPE_LMG */
            || ammoType == 3 /* WEAPAMMOTYPE_HMG */)
            tracerChance = 0.69999999f;
        if ((tracerChance * 32.0f) > (rand() & 0x1F))
            CG_SpawnTracer(pstart, pend, ammoType);
    }
    else
    {
        CG_ASSERT("wp.pWeapInfo", "c:\\cod\\code\\game\\cg_event.cpp", 133);
    }
}

struct trace_t {
    float fraction;      // +0x20
    unsigned int mEntity; // +0x30
};
struct collision_context_t {
    int contentmask;  // +0x00
};
struct snapshot_t {
    int serverTime;  // +0x00
    unsigned char ps[0x5D0];
};
extern int dword_F6295C[4 * 1580];
extern int dword_F62944[4 * 1580];
extern int cg_numSolidEntities;
extern unsigned int cg_solidEntities[1024];
extern int cg_norender;
extern bool g_enableControllerTest;
extern void Trace(trace_t* results, const math::Position3* start,
                  const math::Position3* end, const math::Position3* mins,
                  const math::Position3* maxs, void* model, int brushmask,
                  int capsule, void* sphere);
extern int CM_PointContents(const math::Position3* p, void* model);
extern int CM_TransformedPointContents(const math::Position3* p, void* model,
                                       const math::Position3* origin,
                                       const math::Position3* angles);
extern void CG_ClipMoveToEntities(const math::Position3* start,
                                  const math::Position3* mins,
                                  const math::Position3* maxs,
                                  const math::Position3* end,
                                  const collision_context_t* context,
                                  int capsule, trace_t* tr);
extern void CG_DamageFeedback(int yawByte, int pitchByte, float damage);
extern void Cvar_Set(const char* var_name, const char* value);
extern void SoundDevice_UnpauseAllSounds(void* sInst);
extern void* SoundDevice_sInst;
extern int Key_GetCatcher();
extern void Key_SetCatcher(int catcher);
extern void* EntityHandleDb_mActiveList;
extern char cgsGlobal_shellshockParms[0x7C];
class EntityHandleDbLocal;
class EntityHandleDbLocal {
public:
    struct DbElement {
        Entity* mObject;
        int mKey;
    };
    DbElement mElements[0x540];
};
extern EntityHandleDbLocal EntityHandleDb_sInst;
static Entity* EntityHandleDb_Get(unsigned int handleVal)
{
    unsigned int v = handleVal & 0xFFF;
    if (v < 0x540 && handleVal >> 12 == EntityHandleDb_sInst.mElements[v].mKey)
        return EntityHandleDb_sInst.mElements[v].mObject;
    return nullptr;
}
extern int cgGlobal_oldTime;

// ea: 0x006A23D0
void CG_Trace(trace_t* result, const math::Position3* start,
              const math::Position3* mins, const math::Position3* maxs,
              const math::Position3* end,
              const collision_context_t* context)
{
    int contentmask = context->contentmask;
    trace_t v9;
    memset(&v9, 0, sizeof(v9));
    Trace(&v9, start, end, mins, maxs, nullptr, contentmask, 0, nullptr);
    v9.mEntity =
        v9.fraction == 1.0f ? 0 : (unsigned int)EntityHandleDb_mActiveList;
    CG_ClipMoveToEntities(start, mins, maxs, end, context, 0, &v9);
    *result = v9;
}

// ea: 0x006A2480
void CG_TraceCapsule(trace_t* result, const math::Position3* start,
                     const math::Position3* mins,
                     const math::Position3* maxs,
                     const math::Position3* end,
                     const collision_context_t* context)
{
    int contentmask = context->contentmask;
    trace_t v9;
    memset(&v9, 0, sizeof(v9));
    Trace(&v9, start, end, mins, maxs, nullptr, contentmask, 0, nullptr);
    v9.mEntity =
        v9.fraction == 1.0f ? 0 : (unsigned int)EntityHandleDb_mActiveList;
    CG_ClipMoveToEntities(start, mins, maxs, end, context, 1, &v9);
    *result = v9;
}

// ea: 0x006A2530
int CG_PointContents(const math::Position3* point,
                     collision_context_t* context)
{
    int v17 = CM_PointContents(point, nullptr);
    for (int i = 0; i < cg_numSolidEntities; ++i)
    {
        unsigned int v4 = cg_solidEntities[i] & 0xFFF;
        if (v4 < 0x540
            && cg_solidEntities[i] >> 12
                   == EntityHandleDb_sInst.mElements[v4].mKey)
        {
            Entity* mObject = EntityHandleDb_sInst.mElements[v4].mObject;
            if (mObject != nullptr && mObject->s.solid == 0xFFFFFF
                && mObject->r.bmodel != nullptr)
            {
                v17 |= CM_TransformedPointContents(
                    point, mObject->r.bmodel, &mObject->s.lerpOrigin,
                    &mObject->s.lerpAngles);
            }
        }
    }
    return v17 & context->contentmask;
}

// ea: 0x0069D780
void CG_TransitionPlayerState(void* ps, void* ops)
{
    if (((unsigned int*)ps)[0] != ((unsigned int*)ops)[0]
        && ((unsigned int*)ps)[3] != 0)
        CG_DamageFeedback(((unsigned int*)ps)[1], ((unsigned int*)ps)[2],
                          (float)((unsigned int*)ps)[3]);
}

// ea: 0x0069D7B0
void CG_BuildSolidList()
{
    int v0 = dword_F62960[1580 * currCl];
    cg_numSolidEntities = 0;
    if (v0 == 0)
        CG_ASSERT("snap", "c:\\cod\\code\\game\\cg_predict.cpp", 47);
    int count = *(int*)((char*)&EntityHandleDb_sInst + 0x2AB0);
    for (int i = 0; i < count; ++i)
    {
        Entity* ent = *(Entity**)((char*)&EntityHandleDb_sInst + 0x2AB4
                                  + 4 * i);
        if (ent != nullptr)
        {
            if ((ent->s.solid != 0xFFFFFF || (ent->s.eFlags & 2) == 0)
                && ent->s.eType != 2 && ent->r.bmodel != nullptr)
            {
                if (cg_numSolidEntities < 1024)
                {
                    cg_solidEntities[cg_numSolidEntities] =
                        *(unsigned int*)((char*)ent + 564);
                    ++cg_numSolidEntities;
                }
                else
                {
                    CG_ASSERT("0", "c:\\cod\\code\\game\\cg_predict.cpp", 80);
                }
            }
        }
    }
}

// ea: 0x0069D8D0
void CG_SetInitialSnapshot(snapshot_t* snap)
{
    if (snap == nullptr)
        CG_ASSERT("snap", "c:\\cod\\code\\game\\cg_snapshot.cpp", 66);
    int v1 = 1580 * currCl;
    dword_F6295C[v1] = 1;
    dword_F62960[v1] = (int)snap;
    dword_F62944[v1] = *(int*)((char*)snap + 0xA0);
    int count = *(int*)((char*)&EntityHandleDb_sInst + 0x2AB0);
    for (int i = 0; i < count; ++i)
    {
        Entity* ent = *(Entity**)((char*)&EntityHandleDb_sInst + 0x2AB4
                                  + 4 * i);
        if (ent != nullptr)
            ent->currentValid = 0;
    }
    cgGlobal_time = snap->serverTime;
    cgGlobal_oldTime = cgGlobal_time;
    if (cg_norender != 0)
    {
        g_enableControllerTest = true;
        Cvar_Set("cg_norender", "0");
        SoundDevice_UnpauseAllSounds(SoundDevice_sInst);
        int Catcher = Key_GetCatcher();
        Key_SetCatcher(Catcher & 0xFFFFFFFD);
    }
}

extern void CG_RegisterItems();
extern void CG_ParseCullDist();
extern void CG_NorthDirectionChanged();
extern void CG_RegisterServerShader(int num);
extern void CG_ParseObjectiveChange(int iNum);
extern int CG_LoadShellShockCvars(const char* name);
extern void CG_SetShellShockParmsFromCvars(void* parms);
extern void CG_CheckOpenWaitingScriptMenu();
extern void CG_ServerCommand();
extern int CL_GetServerCommand(int serverCommandNumber);
extern int dword_F6294C[4 * 1580];
extern void* RE_RegisterModel(void* result, const char* name, int pakId,
                              int imagetype);
extern int CurPakId();
extern void VectorNormalize2(const float* v, float* out);
extern void PostEffectEventScriptCall(const Entity* ent, const char* scriptId,
                                      bool queue, int pakid, bool important);
extern float dword_F63C70[4 * 1580];
extern float dword_F63C74[4 * 1580];
extern float dword_F63C78[4 * 1580];
extern float* gCamera_mLastTagCamMat_w;

// ea: 0x006A34D0
void CG_ConfigStringModifiedInternal(int num)
{
    const char* v2 = CG_ConfigString(num);
    if (num == 8)
    {
        CG_RegisterItems();
    }
    else if (num == 3)
    {
        const char* ConfigString = CL_GetConfigStringC(3);
        Info_ValueForKey(ConfigString, "n");
        Info_ValueForKey(ConfigString, "t");
    }
    else if (num != 0)
    {
        if (num == 9)
        {
            CG_ParseCullDist();
        }
        else if (num < 33 || num >= 161)
        {
            if (num < 225 || num >= 305)
            {
                if (num < 561 || num >= 563)
                {
                    if (num < 16 || num >= 33)
                    {
                        if (num != 627)
                        {
                            if (num < 724 || num >= 980)
                            {
                                if (num == 11)
                                    CG_NorthDirectionChanged();
                                else if (num == 12)
                                {
                                    char wind_str[128];
                                    strcpy(wind_str, CL_GetConfigStringC(12));
                                    float f[4];
                                    sscanf(wind_str, "%f %f %f %f", &f[0],
                                           &f[1], &f[2], &f[3]);
                                    CG_ASSERT("0 && \"FX_SetWind GONE\"",
                                              "c:\\cod\\code\\game\\"
                                              "cg_servercmds.cpp",
                                              193);
                                }
                            }
                            else
                            {
                                CG_RegisterServerShader(num);
                            }
                        }
                    }
                    else
                    {
                        CG_ParseObjectiveChange(num);
                    }
                }
                else if (v2 != nullptr && v2[0] != 0)
                {
                    if (CG_LoadShellShockCvars(v2) != 0)
                        CG_SetShellShockParmsFromCvars(
                            (char*)cgsGlobal_shellshockParms + (num - 561));
                }
            }
        }
        else
        {
            int v6 = CurPakId();
            void* result;
            void* v7 = RE_RegisterModel(&result, v2, v6, 7);
            gCamera_mLastTagCamMat_w[2 * num] = *(float*)v7;
            gCamera_mLastTagCamMat_w[2 * num + 1] = *(float*)((char*)v7 + 4);
        }
    }
    else
    {
        CG_ParseServerinfo();
    }
}

// ea: 0x006A3B60
void CG_ExecuteNewServerCommands(int latestSequence)
{
    CG_CheckOpenWaitingScriptMenu();
    int* i = &dword_F6294C[1580 * currCl];
    while (*i < latestSequence)
    {
        int v2 = *i + 1;
        *i = v2;
        if (CL_GetServerCommand(v2) != 0)
            CG_ServerCommand();
    }
}

// ea: 0x006AB640
void CG_WhizbySound(unsigned int sourceEntity, const float* vStart,
                    const float* vEnd)
{
    float vDelta[3] = {vEnd[0] - vStart[0], vEnd[1] - vStart[1],
                       vEnd[2] - vStart[2]};
    float vDir[3];
    VectorNormalize2(vDelta, vDir);
    float v4 = ((dword_F63C78[1580 * currCl] - vStart[2]) * vDir[2]
                + (dword_F63C74[1580 * currCl] - vStart[1]) * vDir[1])
               + (dword_F63C70[1580 * currCl] - vStart[0]) * vDir[0];
    if (v4 >= 64.0f
        && (v4 + 64.0f) <= ((vDir[2] * vDelta[2]) + (vDir[1] * vDelta[1])
                            + (vDir[0] * vDelta[0])))
    {
        float v5 = ((v4 * vDir[0]) + vStart[0])
                   - dword_F63C70[1580 * currCl];
        float v6 = ((v4 * vDir[1]) + vStart[1])
                   - dword_F63C74[1580 * currCl];
        float v13 = ((v4 * vDir[2]) + vStart[2])
                    - dword_F63C78[1580 * currCl];
        if (sqrtf(v13 * v13 + v6 * v6 + v5 * v5) <= 140.0f)
        {
            Entity* v7 = EntityHandleDb_Get(sourceEntity);
            if (v7 == nullptr)
                CG_ASSERT("cent", "c:\\cod\\code\\game\\cg_weapons.cpp", 4472);
            PostEffectEventScriptCall(v7, "WhizBySound", false, -1, false);
        }
    }
}

extern void* cg_weapons;
extern void* cg_items;
extern void* bg_itemlist;
extern void* Entity_GetRefEntity(Entity* ent);
extern void RE_AddRefEntityToScene(void* ent, int iCellNum);
extern void AnglesToAxis(const math::Position3* angles, float (*axis)[3]);
extern void CG_LockLightingOrigin(Entity* ent, void* refEnt);
extern void CG_RegisterItemVisuals(int itemNum);
extern void CG_Error(const char* msg, ...);
extern int BG_GetNumWeapons();

// ea: 0x00689CD0
void CG_Missile(Entity* entity)
{
    if (entity->s.eFlags >= 0)
    {
        if (entity->s.weapon > BG_GetNumWeapons())
            entity->s.weapon = 0;
        weaponInfo_s* v23 = &((weaponInfo_s*)cg_weapons)[entity->s.weapon];
        void* mDObj = entity->mDObj;
        if (mDObj != nullptr)
        {
            refEntity_t* RefEntity =
                (refEntity_t*)Entity_GetRefEntity(entity);
            RefEntity->origin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->origin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->origin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            RefEntity->oldorigin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->oldorigin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->oldorigin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            RefEntity->renderfx = v23->missileRenderfx | 0x40;
            AnglesToAxis(&entity->s.lerpAngles, RefEntity->axis);
            RefEntity->obj = mDObj;
            RefEntity->entity = entity;
            RefEntity->reType = 1;
            RE_AddRefEntityToScene(RefEntity, -1);
        }
    }
}

// ea: 0x00689E70
void CG_Mover(Entity* entity)
{
    if (entity->s.eFlags >= 0)
    {
        bool v2 = entity->s.solid == 0xFFFFFF;
        void* mDObj = entity->mDObj;
        if (v2 || mDObj != nullptr)
        {
            refEntity_t* RefEntity =
                (refEntity_t*)Entity_GetRefEntity(entity);
            RefEntity->origin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->origin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->origin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            RefEntity->oldorigin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->oldorigin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->oldorigin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            AnglesToAxis(&entity->s.lerpAngles, RefEntity->axis);
            RefEntity->renderfx = 64;
            RefEntity->obj = mDObj;
            RefEntity->entity = entity;
            RefEntity->reType = 1;
            RE_AddRefEntityToScene(RefEntity, -1);
        }
    }
}

// ea: 0x00689FF0
void CG_ScriptMover(Entity* entity)
{
    if (entity->s.eFlags >= 0)
    {
        bool v2 = entity->s.solid == 0xFFFFFF;
        void* mDObj = entity->mDObj;
        if (v2 || mDObj != nullptr)
        {
            refEntity_t* RefEntity =
                (refEntity_t*)Entity_GetRefEntity(entity);
            int eFlags = entity->s.eFlags;
            if ((eFlags & 0x40000000) != 0)
            {
                entity->s.eFlags = eFlags & 0xBFFFFFFF;
                RefEntity->origin[0] = entity->s.lerpOrigin.v.m128_f32[0];
                RefEntity->origin[1] = entity->s.lerpOrigin.v.m128_f32[1];
                RefEntity->origin[2] = entity->s.lerpOrigin.v.m128_f32[2];
                RefEntity->oldorigin[0] = entity->s.lerpOrigin.v.m128_f32[0];
                RefEntity->oldorigin[1] = entity->s.lerpOrigin.v.m128_f32[1];
                RefEntity->oldorigin[2] = entity->s.lerpOrigin.v.m128_f32[2];
                AnglesToAxis(&entity->s.lerpAngles, RefEntity->axis);
            }
            RefEntity->renderfx = 64;
            RefEntity->reType = 1;
            RefEntity->entity = entity;
            RefEntity->obj = mDObj;
            if (v2)
            {
                RE_AddRefEntityToScene(RefEntity, -1);
            }
            else
            {
                CG_LockLightingOrigin(entity, RefEntity);
                RE_AddRefEntityToScene(RefEntity, -1);
            }
        }
    }
}

// ea: 0x0068A540
void CG_Vehicle(Entity* entity)
{
    if (entity->s.eFlags >= 0)
    {
        void* mDObj = entity->mDObj;
        if (mDObj != nullptr)
        {
            refEntity_t* RefEntity =
                (refEntity_t*)Entity_GetRefEntity(entity);
            memcpy(RefEntity->origin, &entity->r.currentOrigin, 12);
            memcpy(RefEntity->oldorigin, &entity->r.currentOrigin, 12);
            AnglesToAxis(&entity->r.currentAngles, RefEntity->axis);
            RefEntity->lightingOrigin[0] =
                entity->r.currentOrigin.v.m128_f32[0];
            RefEntity->lightingOrigin[1] =
                entity->r.currentOrigin.v.m128_f32[1];
            RefEntity->lightingOrigin[2] =
                entity->r.currentOrigin.v.m128_f32[2] + 32.0f;
            RefEntity->renderfx = 128;
            RefEntity->obj = mDObj;
            RefEntity->entity = entity;
            RefEntity->reType = 1;
            int eFlags = entity->s.eFlags;
            if ((eFlags & 0x100000) != 0 && (eFlags & 0x400) == 0)
                RefEntity->renderfx = 16;
            RE_AddRefEntityToScene(RefEntity, -1);
        }
    }
}

// ea: 0x00696C20
void CG_Item(Entity* entity)
{
    int brushmodel = *(int*)((char*)&entity->s + 6) & 0xFF;
    if (brushmodel >= 0x89)
        CG_Error("Bad item index %i on entity", brushmodel);
    if (entity->s.eFlags >= 0)
    {
        if (((unsigned char*)cg_items)[8 * brushmodel] != 0)
        {
            void* mDObj = entity->mDObj;
            if (mDObj != nullptr)
            {
                refEntity_t* RefEntity =
                    (refEntity_t*)Entity_GetRefEntity(entity);
                RefEntity->scale = 0.0f;
                if (((unsigned int*)bg_itemlist)[13 * brushmodel + 4]
                    == 1 /* IT_WEAPON */)
                {
                    AnglesToAxis(&entity->s.lerpAngles, RefEntity->axis);
                    RefEntity->scale = 1.5f;
                }
                else
                {
                    AnglesToAxis(&entity->s.lerpAngles, RefEntity->axis);
                }
                RefEntity->origin[0] = entity->s.lerpOrigin.v.m128_f32[0];
                RefEntity->origin[1] = entity->s.lerpOrigin.v.m128_f32[1];
                RefEntity->origin[2] = entity->s.lerpOrigin.v.m128_f32[2];
                RefEntity->oldorigin[0] = entity->s.lerpOrigin.v.m128_f32[0];
                RefEntity->oldorigin[1] = entity->s.lerpOrigin.v.m128_f32[1];
                RefEntity->oldorigin[2] = entity->s.lerpOrigin.v.m128_f32[2];
                RefEntity->entity = entity;
                RefEntity->reType = 1;
                RE_AddRefEntityToScene(RefEntity, -1);
            }
        }
        else
        {
            CG_RegisterItemVisuals(brushmodel);
        }
    }
}

extern void CG_General(Entity* entity);
extern void CG_Portal(Entity* entity);
extern void CG_mg42(Entity* entity);
extern void CG_WeaponUpdateLoopingSound(Entity* entity);
extern void G_GetVehicleInfo(Entity* ent);
extern void G_CalcTagParentAxis(Entity* ent, float (*parentAxis)[3]);
extern bool IsPlayerFullySeatedInVehicle(Entity* player);
extern int AnimationPlayer_IsPartialIdle(void* player, bool checkLooping);
extern int Entity_GetPlayerIndex(Entity* ent);
extern bool Camera_IsTweening(void* cam);
extern bool IsLocalPlayer(Entity* ent);
extern int level_time;
extern int dword_F62964[4 * 1580];
extern int dword_F6355C[4 * 1580];
extern float dword_F63C70[4 * 1580];
extern float* gCamera;
extern int dword_180000;
extern unsigned int head_hash_0;
extern unsigned int HashString_CalcHash(const char* str);
extern float VectorDistance(const float* v1, const float* v2);
extern int G_DObjGetWorldTagMatrix(Entity* ent, unsigned int tag_name_hash,
                                   float* tagMat);
extern Entity* GetPlayer2(int idx);
void CG_Player(Entity* entity);
void CG_Actor(Entity* entity);

// ea: 0x00689A80
void CG_EntityEffects()
{
}

// ea: 0x006A1AD0
void CG_ProcessEntity(Entity* entity)
{
    switch (entity->s.eType)
    {
    case 0u: CG_General(entity); break;
    case 1u: CG_Player(entity); break;
    case 2u: CG_Item(entity); break;
    case 3u: CG_Missile(entity); break;
    case 4u: CG_Mover(entity); break;
    case 5u: CG_Portal(entity); break;
    case 6u:
    case 8u:
    case 9u:
        return;
    case 7u: CG_ScriptMover(entity); break;
    case 0xAu: CG_mg42(entity); break;
    case 0xBu:
    case 0xDu: CG_Actor(entity); break;
    case 0xCu: CG_ActorSpawner(entity); break;
    case 0xEu:
    case 0xFu: CG_Vehicle(entity); break;
    default:
        CG_Error("Bad entity type: %i\n", entity->s.eType);
        break;
    }
}

// ea: 0x0069B160
void CG_Actor(Entity* entity)
{
    int eFlags = entity->s.eFlags;
    if ((eFlags & 0x80u) == 0 && (eFlags & 0x100000) == 0)
    {
        void* mDObj = entity->mDObj;
        if (mDObj != nullptr)
        {
            refEntity_t* RefEntity =
                (refEntity_t*)Entity_GetRefEntity(entity);
            RefEntity->origin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->origin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->origin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            RefEntity->lightingOrigin[0] =
                entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->lightingOrigin[1] =
                entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->lightingOrigin[2] =
                entity->s.lerpOrigin.v.m128_f32[2] + 32.0f;
            RefEntity->renderfx = 128;
            RefEntity->oldorigin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->oldorigin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->oldorigin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            AnglesToAxis(&entity->s.lerpAngles, RefEntity->axis);
            RefEntity->reType = 1;
            RefEntity->obj = mDObj;
            RefEntity->entity = entity;
            if ((entity->s.eFlags & 0x100) != 0)
                RefEntity->renderfx |= 0x20u;
            RE_AddRefEntityToScene(RefEntity, -1);
            CG_WeaponUpdateLoopingSound(entity);
        }
    }
}

static unsigned int sHeadHashInit = 0;

// ea: 0x0069FAA0
void CG_Player(Entity* entity)
{
    int eFlags = entity->s.eFlags;
    if ((eFlags & 0x80u) == 0
        && (dword_F6355C[1580 * currCl] != 0
            || (eFlags & 0x100000) != 0
            || *(int*)(dword_F62964[1580 * currCl] + 52) >= 6
            || entity != EntityManager_GetPlayer(EntityManager_sInst,
                                                 currCl)))
    {
        if ((entity->s.eFlags & 0x100000) == 0)
            goto LABEL_72;
        Entity* mObject = EntityHandleDb_Get(entity->r.mOwner.mHandle.mVal);
        G_GetVehicleInfo(mObject);
        if (entity != EntityManager_GetPlayer(EntityManager_sInst, currCl))
            goto LABEL_31;
        if (IsPlayerFullySeatedInVehicle(entity))
        {
            Client* client = entity->client;
            if (client->ps.vehType != 1 && client->ps.vehPos == 0)
                goto LABEL_31;
        }
        if ((sHeadHashInit & 1) == 0)
        {
            sHeadHashInit |= 1u;
            head_hash_0 = HashString_CalcHash("bip01 head");
        }
        float tagMtx[31];
        G_DObjGetWorldTagMatrix(entity, head_hash_0, tagMtx);
        float v6 = entity->client->ps.vehType != 2 ? 64.0f : 26.0f;
        if (v6 <= VectorDistance(&tagMtx[12],
                                 &dword_F63C70[1580 * currCl]))
        {
        LABEL_31:
            if (!IsPlayerFullySeatedInVehicle(entity)
                || entity->client->ps.vehType != 2
                || entity->client->ps.vehPos != 0)
            {
                if (entity != EntityManager_GetPlayer(EntityManager_sInst,
                                                      currCl)
                    || dword_F6355C[1580 * currCl] != 0
                    || !IsPlayerFullySeatedInVehicle(entity)
                    || entity->client->ps.vehType != 2
                    || (Camera_IsTweening(
                            &((char*)gCamera)[0x1F0
                                              * Entity_GetPlayerIndex(
                                                  entity)])
                        || ((entity->client->ps.vehPos != 6
                             && entity->client->ps.vehPos != 1)
                            || !IsLocalPlayer(entity))
                               && (*(int*)((char*)&gCamera[currCl] + 0x194)
                                       != 1
                                   || entity != GetPlayer2(currCl))))
                {
                LABEL_72:
                    if (entity->sentient != nullptr)
                    {
                        Client* v10 = entity->client;
                        int playerState = v10->pers.playerState;
                        if (playerState != 0 && playerState != 2)
                        {
                            void* obj = entity->mDObj;
                            if (obj != nullptr)
                            {
                                int time = level_time;
                                if (v10->mNoDrawTime > level_time)
                                {
                                    v10->mNoDrawTime = 0;
                                    time = level_time;
                                }
                                Client* v13 = entity->client;
                                if (v13->mNoDrawTime <= time - 200)
                                {
                                    void* v14 =
                                        ((void**)obj)[4];  // animPlayers[0]
                                    if (v14 == nullptr
                        || AnimationPlayer_IsPartialIdle(v14, true) != 0)
                                    {
                                        v13->mNoDrawTime = time;
                                        return;
                                    }
                                    refEntity_t* RefEntity =
                                        (refEntity_t*)Entity_GetRefEntity(
                                            entity);
                                    float v16 =
                                        entity->r.currentOrigin.v.m128_f32[2];
                                    float v17 =
                                        entity->r.currentOrigin.v.m128_f32[0];
                                    float v18 =
                                        entity->r.currentOrigin.v.m128_f32[1];
                                    RefEntity->origin[0] = v17;
                                    RefEntity->origin[1] = v18;
                                    RefEntity->origin[2] = v16;
                                    RefEntity->lightingOrigin[0] = v17;
                                    RefEntity->lightingOrigin[1] = v18;
                                    RefEntity->lightingOrigin[2] = v16 + 32.0f;
                                    RefEntity->oldorigin[0] = v17;
                                    RefEntity->oldorigin[1] = v18;
                                    RefEntity->oldorigin[2] = v16;
                                    math::Position3* p_currentAngles;
                                    if ((entity->s.eFlags & 0x100000) != 0)
                                    {
                                        Entity* v21 = EntityHandleDb_Get(
                                            entity->r.mOwner.mHandle.mVal);
                                        if (v21 == nullptr
                                            || v21->scr_vehicle == nullptr)
                                            CG_ASSERT(
                                                "vehicle && "
                                                "vehicle->scr_vehicle",
                                                "c:\\cod\\code\\game\\"
                                                "cg_player.cpp",
                                                399);
                                        G_GetVehicleInfo(v21);
                                        if (entity->tagInfo != nullptr)
                                        {
                                            if (IsPlayerFullySeatedInVehicle(
                                                    entity))
                                            {
                                                float parentAxis[4][3];
                                                G_CalcTagParentAxis(
                                                    entity, parentAxis);
                                                RefEntity->axis[0][0] =
                                                    parentAxis[0][0];
                                                RefEntity->axis[0][1] =
                                                    parentAxis[0][1];
                                                RefEntity->axis[0][2] =
                                                    parentAxis[0][2];
                                                RefEntity->axis[1][0] =
                                                    parentAxis[1][0];
                                                RefEntity->axis[1][1] =
                                                    parentAxis[1][1];
                                                RefEntity->axis[1][2] =
                                                    parentAxis[1][2];
                                                RefEntity->axis[2][0] =
                                                    parentAxis[2][0];
                                                RefEntity->axis[2][1] =
                                                    parentAxis[2][1];
                                                RefEntity->axis[2][2] =
                                                    parentAxis[2][2];
                                                goto LABEL_60;
                                            }
                                        LABEL_56:
                                            AnglesToAxis(
                                                &entity->r.currentAngles,
                                                RefEntity->axis);
                                        LABEL_60:
                                            RefEntity->renderfx = 128;
                                            RefEntity->reType = 1;
                                            RefEntity->obj = obj;
                                            RefEntity->entity = entity;
                                            if ((entity->s.eFlags & 0x100)
                                                != 0)
                                                RefEntity->renderfx = 160;
                                            RE_AddRefEntityToScene(RefEntity,
                                                                   -1);
                                            CG_WeaponUpdateLoopingSound(
                                                entity);
                                            return;
                                        }
                                        p_currentAngles =
                                            &entity->r.currentAngles;
                                    }
                                    else
                                    {
                                        GetPlayer2(currCl);
                                        entity->r.currentAngles.v.m128_f32[0] =
                                            0.0f;
                                        entity->r.currentAngles.v.m128_f32[1] =
                                            entity->client
                                                ->mLastTorsoIKLegsYaw;
                                        entity->r.currentAngles.v.m128_f32[2] =
                                            0.0f;
                                        p_currentAngles =
                                            &entity->r.currentAngles;
                                    }
                                    AnglesToAxis(p_currentAngles,
                                                 RefEntity->axis);
                                    goto LABEL_60;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

extern void CG_AddScaleFade(void* le);
extern void CG_DrawTracer(const math::Position3* _start,
                          const math::Position3* _finish, float width);
extern void BG_EvaluateTrajectory(const void* tr, int atTime,
                                  math::Position3* result);
extern void VectorNormalize2(const float* v, float* out);
extern int dword_DF6ADC[6];
extern int dword_DF6AE0[6];
extern int dword_F6400C[4 * 1580];
extern int dword_F6413C[4 * 1580];
extern int dword_F63BB8[4 * 1580];
extern int dword_F63BBC[4 * 1580];
extern int dword_F63BC0[4 * 1580];
extern int dword_F63BE8[4 * 1580];
extern int dword_F63BEC[4 * 1580];
extern int dword_F63BF0[4 * 1580];
extern int dword_F63C18[4 * 1580];
extern int dword_F63C1C[4 * 1580];
extern int dword_F63C20[4 * 1580];
extern float unk_F63BC4[4 * 1580 * 4];
extern float unk_F63BF4[4 * 6320];
extern float unk_F63C24[4 * 6320];
extern int dword_F63554[4 * 1580];
extern void j_nullsub_89(void* obj, float dtime);
extern void CL_DObjInvalidateSkels();
extern int cg_addentities;
extern void* TestFPS_sInst;
extern int Entity_IsInSnapshot(Entity* ent);
extern void CG_CalcEntityLerpPositions(Entity* cent);

// ea: 0x006A1BC0
void CG_AddMovingTracer(void* le)
{
    int v2 = 4 * ((localEntityFull*)le)->leFlags;
    float v3 = *(float*)&dword_DF6ADC[v2];
    float dir[3];
    dir[0] = *(float*)&dword_DF6AE0[v2];
    dir[1] = v3;
    dir[2] = 0.0f;
    math::Position3 end;
    BG_EvaluateTrajectory(&((localEntityFull*)le)->pos, cgGlobal_time, &end);
    float v6[3];
    VectorNormalize2(((localEntityFull*)le)->pos.trDelta, v6);
    float v4[3];
    v4[0] = (v6[0] * dir[1]) + end.v.m128_f32[0];
    v4[1] = (v6[1] * dir[1]) + end.v.m128_f32[1];
    v4[2] = (v6[2] * dir[1]) + end.v.m128_f32[2];
    CG_DrawTracer(&end, (math::Position3*)v4, dir[0]);
}

// ea: 0x006A1C80
void CG_AddLocalEntities()
{
    localEntity_t* prev = cg_activeLocalEntities.prev;
    dword_F6400C[1580 * currCl] = 0;
    if (prev != &cg_activeLocalEntities)
    {
        localEntity_t* v2;
        do
        {
            v2 = prev->prev;
            if (cgGlobal_time < ((localEntityFull*)prev)->endTime)
            {
                int leType = ((localEntityFull*)prev)->leType;
                if (leType)
                {
                    if (leType == 1)
                    {
                        CG_AddScaleFade(prev);
                    }
                    else if (leType == 2)
                    {
                        CG_AddMovingTracer(prev);
                    }
                    else
                    {
                        CG_Error("Bad leType: %i", leType);
                    }
                }
                else
                {
                    RE_AddRefEntityToScene(&((localEntityFull*)prev)->refEntity,
                                           -1);
                }
            }
            else
            {
                if (v2 == nullptr)
                    CG_Error("CG_FreeLocalEntity: not active");
                prev->prev->next = prev->next;
                prev->next->prev = prev->prev;
                prev->next = cg_freeLocalEntities;
                cg_freeLocalEntities = prev;
            }
            prev = v2;
        } while (v2 != &cg_activeLocalEntities);
    }
}

// ea: 0x006AC270
void CG_AddPacketEntities()
{
    CL_DObjInvalidateSkels();
    int count = *(int*)((char*)&EntityHandleDb_sInst + 0x2AB0);
    for (int i = 0; i < count; ++i)
    {
        Entity* ent = *(Entity**)((char*)&EntityHandleDb_sInst + 0x2AB4
                                  + 4 * i);
        if (ent != nullptr && ent->mDObj != nullptr)
            j_nullsub_89(ent->mDObj, dword_F63554[1580 * currCl] * 0.001f);
    }
    int v4 = 1580 * currCl;
    dword_F6413C[v4] = 0;
    dword_F63BB8[v4] = 0;
    float ang0 = (360 * (cgGlobal_time & 0xFFF)) * 0.00024420026f;
    memcpy(&dword_F63BBC[v4], &ang0, 4);
    dword_F63BC0[v4] = 0;
    dword_F63BE8[v4] = 0;
    float ang1 = (360 * (cgGlobal_time & 0x7FF)) * 0.00048828125f;
    memcpy(&dword_F63BEC[v4], &ang1, 4);
    dword_F63BF0[v4] = 0;
    dword_F63C18[v4] = 0;
    float ang2 = (360 * (cgGlobal_time & 0x3FF)) * 0.0009765625f;
    memcpy(&dword_F63C1C[v4], &ang2, 4);
    dword_F63C20[v4] = 0;
    float f0 = *(float*)&dword_F63BB8[v4];
    float f1 = *(float*)&dword_F63BBC[v4];
    float f2 = *(float*)&dword_F63BC0[v4];
    float a0[3] = {f0, f1, f2};
    AnglesToAxis((const math::Position3*)a0,
                 (float (*)[3])(unk_F63BC4 + v4 * 4));
    float g0 = *(float*)&dword_F63BE8[1580 * currCl];
    float g1 = *(float*)&dword_F63BEC[1580 * currCl];
    float g2 = *(float*)&dword_F63BF0[1580 * currCl];
    float a1[3] = {g0, g1, g2};
    AnglesToAxis((const math::Position3*)a1,
                 (float (*)[3])(unk_F63BF4 + 6320 * currCl));
    float h0 = *(float*)&dword_F63C18[1580 * currCl];
    float h1 = *(float*)&dword_F63C1C[1580 * currCl];
    float h2 = *(float*)&dword_F63C20[1580 * currCl];
    float a2[3] = {h0, h1, h2};
    AnglesToAxis((const math::Position3*)a2,
                 (float (*)[3])(unk_F63C24 + 6320 * currCl));
    for (int i = 0; i < count; ++i)
    {
        Entity* v9 = *(Entity**)((char*)&EntityHandleDb_sInst + 0x2AB4
                                 + 4 * i);
        if (v9 != nullptr && Entity_IsInSnapshot(v9)
            && TestFPS_sInst == nullptr && cg_addentities != 0
            && v9->s.eType < 0x12u)
        {
            CG_CalcEntityLerpPositions(v9);
            CG_ProcessEntity(v9);
        }
    }
}
