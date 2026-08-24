// ============================================================================
// cg_ents.cpp - local entities, server config parsing, grenade counts (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Minimal view of SoundDevice (full class in game/sv/sv_stubs.h).
class SoundDevice { public: static SoundDevice* sInst; };  // ?sInst@SoundDevice@@2PAV1@A


extern const char* CL_GetConfigString(int index);  // ?CL_GetConfigString@@YAPBDH@Z (cl.o)


// Minimal view of EntityManager (full class in game/sv/sv_stubs.h).
class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A (game.o)
    Entity* GetPlayer(int idx);   // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z
    Entity* mPlayers[16];         // +0x04
    bool IsLocalPlayer(Entity* entity);  // ?IsLocalPlayer@EntityManager@@QAE_NPAVEntity@@@Z
};

class Camera;

extern int currCl;
extern int cgGlobal_time;
extern int dword_F62960[4 * 1580];
int dword_F610E4;
int dword_F610E8;
int dword_F610EC;
int dword_F610F0;
int dword_F610F4;
int dword_F610F8;
int dword_F610FC;
int dword_F61100;
int dword_F61104;
int dword_F61108;
int dword_F6110C;
int dword_F61110;
int dword_F61114;
int dword_F61118;
int dword_F6111C;
int dword_F61120;
int dword_F61124;
int dword_F61128;
int dword_F6112C;
int dword_F61130;
extern char* Info_ValueForKey(const char* s, const char* key);
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);
extern int Com_BitCheck(const int* const array, int bitNum);
extern int BG_GetNumWeapons();
extern int BG_WeaponAmmo(const PlayerState* pPS, int iWeapon);
extern int BG_AmmoForWeapon(int iWeapon);
extern weaponFileInfo_t* BG_GetInfoForWeapon(int weapon);
extern void CG_DebugBox(const float* mins, const float* maxs,
                        const float* color, int depthTest, int duration);
extern void CG_Error(const char* msg, ...);
extern void HelmetController(Entity* owner);
char cgsGlobal_mapname[128];  // cg.o BSS

struct localEntity_t {
    localEntity_t* prev;  // +0x00
    localEntity_t* next;  // +0x04
    int16_t leType;       // +0x08
    int16_t leFlags;      // +0x0A
    int endTime;          // +0x0C
    trajectory_t pos;      // +0x10
    float radius;          // +0x38
    float lifeRate;        // +0x3C
    refEntity_t refEntity; // +0x40
    float color[4];         // +0xA0
};
static_assert(sizeof(localEntity_t) == 0xB0, "localEntity_t size mismatch");
localEntity_t cg_localEntities[128];  // ?cg_localEntities@@3PAUlocalEntity_t@@A (cg.o)
localEntity_t cg_activeLocalEntities;  // ?cg_activeLocalEntities@@3UlocalEntity_t@@A (cg.o)
localEntity_t* cg_freeLocalEntities = nullptr;  // ?cg_freeLocalEntities@@3PAUlocalEntity_t@@A (cg.o)

struct _cmd_t {
    unsigned int hVal;  // +0x00
};
_cmd_t gCG_ServerCommands[16];  // ?gCG_ServerCommands@@3PAU_cmd_t@@A (cg.o @ 0x13505E0)

extern const float colorRed[4];

// ea: 0x0068B330
void CG_InitLocalEntities()
{
    memset(cg_localEntities, 0, sizeof(cg_localEntities));
    cg_activeLocalEntities.next = &cg_activeLocalEntities;
    cg_activeLocalEntities.prev = &cg_activeLocalEntities;
    cg_freeLocalEntities = cg_localEntities;
    for (int i = 0; i < 127; ++i)
        cg_localEntities[i].next = &cg_localEntities[i + 1];
    cg_localEntities[127].next = nullptr;
}

// ea: 0x0068B5A0
const char* CG_ConfigString(unsigned int index)
{
    if (index < 0x400)
        return CL_GetConfigString(index);
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
    const char* ConfigString = CL_GetConfigString(0);
    const char* v1 = Info_ValueForKey(ConfigString, "mapname");
    Com_sprintf(cgsGlobal_mapname, 128, "maps/%s.bsp", v1);
}

// ea: 0x0068BCF0
void CG_InitServerCommandHashVals()
{
    memset(gCG_ServerCommands, 0, sizeof(gCG_ServerCommands));
    gCG_ServerCommands[0].hVal = HashString::CalcHash("startCam");
    dword_F610E4 = HashString::CalcHash("stopCam");
    dword_F610E8 = HashString::CalcHash("cp");
    dword_F610EC = HashString::CalcHash("cs");
    dword_F610F0 = HashString::CalcHash("print");
    dword_F610F4 = HashString::CalcHash("gm");
    dword_F610F8 = HashString::CalcHash("gmb");
    dword_F610FC = HashString::CalcHash("object_update");
    dword_F61100 = HashString::CalcHash("object_complete");
    dword_F61104 = HashString::CalcHash("opendeadscreen");
    dword_F61108 = HashString::CalcHash("openvictoryscreen");
    dword_F6110C = HashString::CalcHash("clientLevelShot");
    dword_F61110 = HashString::CalcHash("saveshot");
    dword_F61114 = HashString::CalcHash("mu_play");
    dword_F61118 = HashString::CalcHash("mu_stop");
    dword_F6111C = HashString::CalcHash("snd_fade");
    dword_F61120 = HashString::CalcHash("scr_fade");
    dword_F61124 = HashString::CalcHash("fog");
    dword_F61128 = HashString::CalcHash("ls");
    dword_F6112C = HashString::CalcHash("popupopen");
    dword_F61130 = HashString::CalcHash("popupclose");
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
        if (Com_BitCheck(((Entity*)EntityManager::sInst->mPlayers[currCl])->client->ps
                             .weapons,
                         v0) != 0
            && ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->weapClass
                   == 11 /* WEAPCLASS_GRENADE */
            && ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->slot
                   == 12 /* WEAPSLOT_GRENADE */)
        {
            Entity* Player =
                EntityManager::sInst->GetPlayer( currCl);
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
        if (Com_BitCheck(((Entity*)EntityManager::sInst->mPlayers[currCl])->client->ps
                             .weapons,
                         v0) != 0
            && ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->weapClass
                   == 11 /* WEAPCLASS_GRENADE */
            && ((weaponFileInfoFull*)BG_GetInfoForWeapon(v0))->slot
                   == 13 /* WEAPSLOT_SMOKE_GRENADE */)
        {
            Entity* Player =
                EntityManager::sInst->GetPlayer( currCl);
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
        if (Com_BitCheck(((Entity*)EntityManager::sInst->mPlayers[currCl])->client->ps
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
                EntityManager::sInst->GetPlayer( currCl);
            grenAmmo += BG_WeaponAmmo(&Player->client->ps, v0);
        }
        ++v0;
    } while (v0 <= BG_GetNumWeapons());
    return grenAmmo;
}

// ea: 0x00692C30
bool CG_VehicleActive()
{
    Entity* p = EntityManager::sInst->GetPlayer( currCl);
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
struct vmCvar_t {
    int   integer;  // +0x00
    float value;    // +0x04
};
extern vmCvar_t cg_railTrailTime;
extern vmCvar_t cg_tracerChance;
extern int dword_DF6ADC[6];
float tracer_info_speed[6];
extern void AxisClear(float (*const axis)[3]);
extern double VectorNormalize(float* const v);
extern double VectorDistance(const float* const v1, const float* const v2);
extern void PerpendicularVector(float* const dst, const float* const src);
extern void CrossProduct(const float* v1, const float* v2, float* cross);
extern void FastSinCos(float radians, float* psin, float* pcos);

// ea: 0x00699610
void CG_RailTrail2(const float* color, const float* start, const float* end)
{
    if (cg_railTrailTime.integer > 0)
    {
        localEntityFull* v3 = (localEntityFull*)CG_AllocLocalEntity();
        v3->leType = 0;
        v3->endTime = cg_railTrailTime.integer + cgGlobal_time;
        v3->lifeRate = 1.0f / (float)cg_railTrailTime.integer;
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
int cg_numSolidEntities;
unsigned int cg_solidEntities[1024];  // cg.o BSS
extern struct vmCvar_t cg_norender;
extern bool g_enableControllerTest;
struct sphere_t;
extern void Trace(trace_t* results, const math::Position3& start,
                  const math::Position3& end, const math::Position3& mins,
                  const math::Position3& maxs, DCGSet* model, int brushmask,
                  int capsule, sphere_t* sphere);
class DCGSet;
extern int CM_PointContents(const math::Position3& p, DCGSet* model);
extern int CM_TransformedPointContents(const math::Position3& p,
                                       DCGSet* model,
                                       const math::Position3& origin,
                                       const math::Position3& angles);
extern void CG_ClipMoveToEntities(const math::Position3* start,
                                  const math::Position3* mins,
                                  const math::Position3* maxs,
                                  const math::Position3* end,
                                  const collision_context_t* context,
                                  int capsule, trace_t* tr);
extern void CG_DamageFeedback(int yawByte, int pitchByte, float damage);
extern void Cvar_Set(const char* var_name, const char* value);
extern void SoundDevice_UnpauseAllSounds(void* sInst);
extern int Key_GetCatcher();
extern void Key_SetCatcher(int catcher);
void* EntityHandleDb_mActiveList = nullptr;  // cg.o BSS artifact
char cgsGlobal_shellshockParms[0x7C];  // cg.o BSS
static Entity* EntityHandleDb_Get(unsigned int handleVal)
{
    unsigned int v = handleVal & 0xFFF;
    if (v < 0x540 && handleVal >> 12 == EntityHandleDb::sInst.mElements[v].mKey)
        return EntityHandleDb::sInst.mElements[v].mObject;
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
    Trace(&v9, *start, *end, *mins, *maxs, nullptr, contentmask, 0, nullptr);
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
    Trace(&v9, *start, *end, *mins, *maxs, nullptr, contentmask, 0, nullptr);
    v9.mEntity =
        v9.fraction == 1.0f ? 0 : (unsigned int)EntityHandleDb_mActiveList;
    CG_ClipMoveToEntities(start, mins, maxs, end, context, 1, &v9);
    *result = v9;
}

// ea: 0x006A2530
int CG_PointContents(const math::Position3* point,
                     collision_context_t* context)
{
    int v17 = CM_PointContents(*point, nullptr);
    for (int i = 0; i < cg_numSolidEntities; ++i)
    {
        unsigned int v4 = cg_solidEntities[i] & 0xFFF;
        if (v4 < 0x540
            && cg_solidEntities[i] >> 12
                   == EntityHandleDb::sInst.mElements[v4].mKey)
        {
            Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
            if (mObject != nullptr && mObject->s.solid == 0xFFFFFF
                && mObject->r.bmodel != nullptr)
            {
                v17 |= CM_TransformedPointContents(
                    *point, mObject->r.bmodel, mObject->s.lerpOrigin,
                    mObject->s.lerpAngles);
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
    int count = *(int*)((char*)&EntityHandleDb::sInst + 0x2AB0);
    for (int i = 0; i < count; ++i)
    {
        Entity* ent = *(Entity**)((char*)&EntityHandleDb::sInst + 0x2AB4
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
    int count = *(int*)((char*)&EntityHandleDb::sInst + 0x2AB0);
    for (int i = 0; i < count; ++i)
    {
        Entity* ent = *(Entity**)((char*)&EntityHandleDb::sInst + 0x2AB4
                                  + 4 * i);
        if (ent != nullptr)
            ent->currentValid = 0;
    }
    cgGlobal_time = snap->serverTime;
    cgGlobal_oldTime = cgGlobal_time;
    if (*(int*)&cg_norender != 0)
    {
        g_enableControllerTest = true;
        Cvar_Set("cg_norender", "0");
        SoundDevice_UnpauseAllSounds(SoundDevice::sInst);
        int Catcher = Key_GetCatcher();
        Key_SetCatcher(Catcher & 0xFFFFFFFD);
    }
}

extern int CG_RegisterItems();
extern void CG_ParseCullDist();
extern int CG_NorthDirectionChanged();
extern void CG_RegisterServerShader(int num);
extern void CG_ParseObjectiveChange(int iNum);
extern int CG_LoadShellShockCvars(const char* name);
struct shellshock_parms_t;
extern void CG_SetShellShockParmsFromCvars(shellshock_parms_t* parms);
extern void CG_CheckOpenWaitingScriptMenu();
extern void CG_ServerCommand();
extern int CL_GetServerCommand(int serverCommandNumber);
extern int dword_F6294C[4 * 1580];
extern void* RE_RegisterModel(void* result, const char* name, int pakId,
                              int imagetype);
extern TPakId CurPakId();  // defined in streamer/pakmanager.cpp
extern double VectorNormalize2(const float* const v,
                                    float* const out);
extern Handle PostEffectEventScriptCall(const Entity* ent,
                                        const char* scriptId, bool queue,
                                        TPakId pakid, const bool important);
extern float dword_F63C70[4 * 1580];
extern float dword_F63C74[4 * 1580];
extern float dword_F63C78[4 * 1580];
float* gCamera_mLastTagCamMat_w;

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
        const char* ConfigString = CL_GetConfigString(3);
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
                                    strcpy(wind_str, CL_GetConfigString(12));
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
                            (shellshock_parms_t*)((char*)
                                cgsGlobal_shellshockParms + (num - 561)));
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
            PostEffectEventScriptCall(v7, "WhizBySound", false,
                                      (TPakId)-1, false);
        }
    }
}

extern weaponInfo_s cg_weapons[];
extern itemInfo_t cg_items[];
extern struct gitem_s* bg_itemlist;
extern void RE_AddRefEntityToScene(void* ent, int iCellNum);
extern void AnglesToAxis(const float* const angles,
                         float (*const axis)[3]);
extern void CG_LockLightingOrigin(Entity* ent, refEntity_t* refEnt);
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
                (refEntity_t*)&entity->GetRefEntity();
            RefEntity->origin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->origin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->origin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            RefEntity->oldorigin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->oldorigin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->oldorigin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            RefEntity->renderfx = v23->missileRenderfx | 0x40;
            AnglesToAxis((const float*)&entity->s.lerpAngles, RefEntity->axis);
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
                (refEntity_t*)&entity->GetRefEntity();
            RefEntity->origin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->origin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->origin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            RefEntity->oldorigin[0] = entity->s.lerpOrigin.v.m128_f32[0];
            RefEntity->oldorigin[1] = entity->s.lerpOrigin.v.m128_f32[1];
            RefEntity->oldorigin[2] = entity->s.lerpOrigin.v.m128_f32[2];
            AnglesToAxis((const float*)&entity->s.lerpAngles, RefEntity->axis);
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
                (refEntity_t*)&entity->GetRefEntity();
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
                AnglesToAxis((const float*)&entity->s.lerpAngles, RefEntity->axis);
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
                (refEntity_t*)&entity->GetRefEntity();
            memcpy(RefEntity->origin, &entity->r.currentOrigin, 12);
            memcpy(RefEntity->oldorigin, &entity->r.currentOrigin, 12);
            AnglesToAxis((const float*)&entity->r.currentAngles, RefEntity->axis);
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
                    (refEntity_t*)&entity->GetRefEntity();
                RefEntity->scale = 0.0f;
                if (((unsigned int*)bg_itemlist)[13 * brushmodel + 4]
                    == 1 /* IT_WEAPON */)
                {
                    AnglesToAxis((const float*)&entity->s.lerpAngles, RefEntity->axis);
                    RefEntity->scale = 1.5f;
                }
                else
                {
                    AnglesToAxis((const float*)&entity->s.lerpAngles, RefEntity->axis);
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
struct vehicle_info_t;
struct DObjSkelMat;
extern vehicle_info_t* G_GetVehicleInfo(Entity* ent);
extern void G_CalcTagParentAxis(Entity* ent, float (*parentAxis)[3]);
extern bool IsPlayerFullySeatedInVehicle(Entity* player);
// AnimationPlayer_IsPartialIdle artifact (real member: AnimationPlayer::IsPartialIdle)
int AnimationPlayer_IsPartialIdle(void* player, bool checkLooping)
{
    return ((AnimationPlayer*)player)->IsPartialIdle(checkLooping) ? 1 : 0;
}
struct CameraView {
    uint8_t _pad[0x118];
    float mTweenTime;      // +0x114
    float mTweenDuration;  // +0x118
    bool IsTweening()      // ?IsTweening@Camera@@QAE_NXZ (cg.o 0x68EBB0)
    {
        return mTweenDuration > mTweenTime;
    }
};
extern int level_time;
extern int dword_F62964[4 * 1580];
extern int dword_F6355C[4 * 1580];
extern float dword_F63C70[4 * 1580];
extern Camera* gCamera;
extern int dword_180000;
unsigned int head_hash_0;
extern double VectorDistance(const float* const v1, const float* const v2);
extern int G_DObjGetWorldTagMatrix(Entity* ent, unsigned int tag_name_hash,
                                   DObjSkelMat* tagMat);
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
                (refEntity_t*)&entity->GetRefEntity();
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
            AnglesToAxis((const float*)&entity->s.lerpAngles, RefEntity->axis);
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
            || entity != EntityManager::sInst->GetPlayer(
                                                 currCl)))
    {
        if ((entity->s.eFlags & 0x100000) == 0)
            goto LABEL_72;
        Entity* mObject = EntityHandleDb_Get(entity->r.mOwner.mHandle.mVal);
        G_GetVehicleInfo(mObject);
        if (entity != EntityManager::sInst->GetPlayer( currCl))
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
            head_hash_0 = HashString::CalcHash("bip01 head");
        }
        float tagMtx[31];
        G_DObjGetWorldTagMatrix(entity, head_hash_0, (DObjSkelMat*)tagMtx);
        float v6 = entity->client->ps.vehType != 2 ? 64.0f : 26.0f;
        if (v6 <= VectorDistance(&tagMtx[12],
                                 &dword_F63C70[1580 * currCl]))
        {
        LABEL_31:
            if (!IsPlayerFullySeatedInVehicle(entity)
                || entity->client->ps.vehType != 2
                || entity->client->ps.vehPos != 0)
            {
                bool camTweening =
                    ((CameraView*)((char*)gCamera
                                   + 0x1F0 * entity->GetPlayerIndex()))
                        ->IsTweening();
                if (entity != EntityManager::sInst->GetPlayer(
                                                      currCl)
                    || dword_F6355C[1580 * currCl] != 0
                    || !IsPlayerFullySeatedInVehicle(entity)
                    || entity->client->ps.vehType != 2
                    || camTweening
                        || ((entity->client->ps.vehPos != 6
                             && entity->client->ps.vehPos != 1)
                            || !entity->IsLocalPlayer())
                               && (*(int*)((char*)gCamera + 0x1F0 * currCl + 0x194)
                                       != 1
                                    || entity != EntityManager::sInst->GetPlayer(currCl)))
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
                                        (refEntity_t*)&entity->GetRefEntity();
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
                                                (const float*)&entity->r
                                                    .currentAngles,
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
                                        EntityManager::sInst->GetPlayer(currCl);
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
                                    AnglesToAxis((const float*)p_currentAngles,
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
extern void CG_DrawTracer(const math::Position3& _start,
                          const math::Position3& _finish, float width);
struct trajectory_t;
extern void BG_EvaluateTrajectory(const trajectory_t* tr, int atTime,
                                  math::Position3& result);
extern double VectorNormalize2(const float* const v,
                                    float* const out);
int dword_DF6ADC[6];  // cg.o BSS
int dword_DF6AE0[6];  // cg.o BSS
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
extern char CL_DObjInvalidateSkels();
extern vmCvar_t cg_addentities;
extern void* TestFPS_sInst;
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
    BG_EvaluateTrajectory((const trajectory_t*)
                              &((localEntityFull*)le)->pos,
                          cgGlobal_time, end);
    float v6[3];
    VectorNormalize2(((localEntityFull*)le)->pos.trDelta, v6);
    float v4[3];
    v4[0] = (v6[0] * dir[1]) + end.v.m128_f32[0];
    v4[1] = (v6[1] * dir[1]) + end.v.m128_f32[1];
    v4[2] = (v6[2] * dir[1]) + end.v.m128_f32[2];
    CG_DrawTracer(end, *(math::Position3*)v4, dir[0]);
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
    int count = *(int*)((char*)&EntityHandleDb::sInst + 0x2AB0);
    for (int i = 0; i < count; ++i)
    {
        Entity* ent = *(Entity**)((char*)&EntityHandleDb::sInst + 0x2AB4
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
    AnglesToAxis((const float*)a0,
                 (float (*)[3])(unk_F63BC4 + v4 * 4));
    float g0 = *(float*)&dword_F63BE8[1580 * currCl];
    float g1 = *(float*)&dword_F63BEC[1580 * currCl];
    float g2 = *(float*)&dword_F63BF0[1580 * currCl];
    float a1[3] = {g0, g1, g2};
    AnglesToAxis((const float*)a1,
                 (float (*)[3])(unk_F63BF4 + 6320 * currCl));
    float h0 = *(float*)&dword_F63C18[1580 * currCl];
    float h1 = *(float*)&dword_F63C1C[1580 * currCl];
    float h2 = *(float*)&dword_F63C20[1580 * currCl];
    float a2[3] = {h0, h1, h2};
    AnglesToAxis((const float*)a2,
                 (float (*)[3])(unk_F63C24 + 6320 * currCl));
    for (int i = 0; i < count; ++i)
    {
        Entity* v9 = *(Entity**)((char*)&EntityHandleDb::sInst + 0x2AB4
                                 + 4 * i);
        if (v9 != nullptr && v9->IsInSnapshot()
            && TestFPS_sInst == nullptr && cg_addentities.integer != 0
            && v9->s.eType < 0x12u)
        {
            CG_CalcEntityLerpPositions(v9);
            CG_ProcessEntity(v9);
        }
    }
}

extern vmCvar_t cg_debugEvents;
extern void CG_Printf(const char* msg, ...);
extern void CG_EntityEvent(Entity* entity, int event, int bPredict);
extern void CG_EntityPreEvent(Entity* entity, int event);
extern void CG_FireWeapon(Entity* attacker, EntityState* attackerState,
                          int event, unsigned int barrel);
extern void CG_EjectWeaponBrass(Entity* entity, int event);
enum EAction : int { kActionNone = 0, kActionPrimary = 1, kActionSecondary = 2 };
extern Handle PostEffectEventWeapon(const Entity* ent, const char* weaponType,
                                    EAction weaponAction);
extern void ByteToDir(unsigned int b, float* const dir);
extern void CG_BulletHitEvent(Entity* entity, const math::Position3* origin,
                              float* const normal, int weapon, int surfType,
                              Entity* hitEnt);
extern void CG_BulletHitClientEvent(unsigned int sourceEntity,
                                    const math::Position3* position,
                                    float* const normal,
                                    unsigned int surfType,
                                    int weapon);
extern void CG_StartShakeCamera(float p, int duration, const float* src,
                                float radius, int client);
extern int dword_F63BA4[4 * 1580];
const char** pEventNamesList;  // ?pEventNamesList (cl.o)

// ea: 0x006AD640
void CG_CheckEvents(Entity* entity)
{
    if (entity->s.eType <= 0x12u)
    {
        int eventSequence = entity->s.eventSequence;
        if (eventSequence != 0)
        {
            if (eventSequence - entity->previousEventSequence < 0)
                entity->previousEventSequence = 0;
            if (eventSequence - entity->previousEventSequence > 4)
                entity->previousEventSequence = eventSequence - 4;
            int previousEventSequence = entity->previousEventSequence;
            if (previousEventSequence < eventSequence)
            {
                unsigned char eventParm = entity->s.eventParm;
                do
                {
                    int v4 = previousEventSequence & 3;
                    int v5 = entity->s.events[v4];
                    entity->s.eventParm = entity->s.eventParms[v4];
                    CG_EntityEvent(entity, v5, 0);
                    ++previousEventSequence;
                } while (previousEventSequence != entity->s.eventSequence);
                eventSequence = entity->s.eventSequence;
                entity->s.eventParm = eventParm;
            }
            entity->previousEventSequence = eventSequence;
        }
        else
        {
            entity->previousEventSequence = 0;
        }
    }
    else
    {
        if (entity->s.eventSequence != 0)
            CG_ASSERT("!entity->s.eventSequence",
                      "c:\\cod\\code\\game\\cg_event.cpp", 830);
        if (entity->previousEventSequence == 0)
        {
            entity->previousEventSequence = 1;
            CG_EntityEvent(entity, entity->s.eType - 18, 0);
        }
    }
}

// ea: 0x006AD770
void CG_CheckPreEvents(Entity* entity)
{
    if (entity->s.eType <= 0x12u)
    {
        int eventSequence = entity->s.eventSequence;
        if (eventSequence != 0)
        {
            if (eventSequence - entity->previousPreEventSequence < 0)
                entity->previousPreEventSequence = 0;
            if (eventSequence - entity->previousPreEventSequence > 4)
                entity->previousPreEventSequence = eventSequence - 4;
            int previousPreEventSequence = entity->previousPreEventSequence;
            if (previousPreEventSequence < eventSequence)
            {
                unsigned char eventParm = entity->s.eventParm;
                if (previousPreEventSequence != eventSequence)
                {
                    do
                    {
                        int v4 = previousPreEventSequence & 3;
                        int v6 = entity->s.events[v4];
                        entity->s.eventParm = entity->s.eventParms[v4];
                        CG_EntityPreEvent(entity, v6);
                        ++previousPreEventSequence;
                    } while (previousPreEventSequence != entity->s.eventSequence);
                }
                entity->s.eventParm = eventParm;
                entity->previousPreEventSequence = entity->s.eventSequence;
            }
            else
            {
                entity->previousPreEventSequence = eventSequence;
            }
        }
        else
        {
            entity->previousPreEventSequence = 0;
        }
    }
    else
    {
        if (entity->s.eventSequence != 0)
            CG_ASSERT("!entity->s.eventSequence",
                      "c:\\cod\\code\\game\\cg_event.cpp", 893);
        if (entity->previousPreEventSequence == 0)
        {
            entity->previousPreEventSequence = 1;
            CG_EntityPreEvent(entity, entity->s.eType - 18);
        }
    }
}

// ea: 0x006ADB70
void CG_CheckPlayerstateEvents(unsigned int* ps, unsigned int* ops,
                               unsigned char eFlags,
                               unsigned char old_eFlags)
{
    int iOldEvents[4];
    int iOldEventSequence;
    if (((old_eFlags ^ eFlags) & 4) != 0)
    {
        memset(iOldEvents, 0, sizeof(iOldEvents));
        iOldEventSequence = 0;
    }
    else
    {
        iOldEventSequence = ops[0];
        iOldEvents[0] = ops[1];
        iOldEvents[1] = ops[2];
        iOldEvents[2] = ops[3];
        iOldEvents[3] = ops[4];
    }
    Entity* Player =
        EntityManager::sInst->GetPlayer( currCl);
    int v8 = ps[0] - 4;
    if (v8 != ps[0])
    {
        int v9 = v8 - iOldEventSequence;
        int v10 = iOldEventSequence - v8;
        do
        {
            if (v9 >= 0 || v10 < 4 && ps[1 + (v8 & 3)] != iOldEvents[v8 & 3])
            {
                int v11 = ps[1 + (v8 & 3)];
                Player->s.eventParm = ps[5 + (v8 & 3)];
                CG_EntityEvent(Player, v11, 1);
                ++dword_F63BA4[1580 * currCl];
                v9 = v8 - iOldEventSequence;
            }
            ++v8;
            ++v9;
            --v10;
        } while (v8 != ps[0]);
    }
}

extern int dword_F62954[4 * 1580];
extern int* dword_F62958;
extern void* CG_ReadNextSnapshot();
extern void CG_SetNextSnap(void* snap);
extern void CG_TransitionSnapshot();
extern void CL_GetCurrentSnapshotNumber(int* snapshotNumber,
                                        int* serverTime);
struct vmCvar_t;
extern void Cvar_VMSet(vmCvar_t* vmCvar, const char* value);
extern int G_GetServerSnapTime();
extern int CG_SetFrameInterpolation();

extern vmCvar_t fs_debug_vm;

struct CollisionDesc {
    math::Position3 coord;    // +0x00
    math::Position3 normal;   // +0x10
    int material;             // +0x20
};

int dword_DF6AE4[4 * 6];  // cg.o BSS
extern int CG_CalcMuzzlePoint(unsigned int entity, float* muzzle,
                              char* flashTag);
extern const math::Position3 native_to_cdl_pos3(const float* v);
extern void CG_SpawnTracer(const math::Position3* pstart,
                           const math::Position3* pend, int ammo);
extern void CG_WhizbySound(unsigned int sourceEntity, const float* vStart,
                           const float* vEnd);
enum EWeaponClass : int { kWeaponClassNone = 0, kWeaponClassBullet = 1 };
extern Handle PostEffectEventBulletHit(const Entity* ent,
                                       EWeaponClass weaponClass,
                                       const CollisionDesc& col_desc);
extern Handle PostEffectEventScriptCall(const Entity* ent,
                                        const char* scriptId, bool queue,
                                        TPakId pakid, const bool important);
extern const char** s_barrelTags;
const char** s_gunnerBarrelTags;  // ?s_gunnerBarrelTags (game.o)

// ea: 0x006ABAA0
void CG_BulletTrajectoryEffects(unsigned int sourceEntity,
                                const math::Position3* position,
                                int surfType, const char* flashTag,
                                int weapon)
{
    if (sourceEntity == 0)
        CG_ASSERT("sourceEntity != TEntityHandle::NullHandle()",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 4726);
    Entity* mObj = EntityHandleDb_Get(sourceEntity);
    if (mObj == nullptr)
        CG_ASSERT("*sourceEntity", "c:\\cod\\code\\game\\cg_weapons.cpp",
                  4727);
    if (dword_F62960[1580 * currCl] != 0
        && *(float*)&cg_tracerChance > 0.0f)
    {
        float muzzle[3];
        if (CG_CalcMuzzlePoint(sourceEntity, muzzle, (char*)flashTag) != 0)
        {
            Entity* v8 = EntityHandleDb_Get(sourceEntity);
            if (v8 != nullptr)
            {
                if (v8 != EntityManager::sInst->GetPlayer(
                                                  currCl)
                    && (*(int*)(dword_F62960[1580 * currCl] + 60)
                        & 0x180000)
                           == 0
                    || (EntityManager::sInst->GetPlayer( currCl),
                        EntityHandleDb_Get(sourceEntity)
                            != EntityManager::sInst->GetPlayer(
                                                       currCl)))
                {
                    void* InfoForWeapon =
                        (void*)BG_GetInfoForWeapon(weapon);
                    if (InfoForWeapon == nullptr)
                        CG_ASSERT("wp.pWeapInfo",
                                  "c:\\cod\\code\\game\\cg_weapons.cpp",
                                  4751);
                    int ammoType = *(int*)((char*)InfoForWeapon + 0xA8);
                    if (ammoType >= 6)
                        CG_ASSERT("ammotype < WEAPAMMOTYPE_NUM",
                                  "c:\\cod\\code\\game\\cg_weapons.cpp",
                                  4754);
                    int v16 = dword_DF6AE4[4 * ammoType];
                    if ((*(float*)&v16 * 100.0f) > (float)(rand() % 100))
                    {
                        math::Position3 v14;
                        v14 = native_to_cdl_pos3(muzzle);
                        CG_SpawnTracer(&v14, position, ammoType);
                    }
                }
                if (EntityHandleDb_Get(sourceEntity) != v8)
                    CG_ASSERT("*sourceEntity == entity",
                              "c:\\cod\\code\\game\\cg_weapons.cpp", 4771);
                if (v8 != EntityManager::sInst->GetPlayer(
                                                  currCl))
                    CG_WhizbySound(sourceEntity, muzzle,
                                   position->v.m128_f32);
            }
        }
    }
}

// ea: 0x006ABD80
void CG_BulletHitEvent(Entity* entity, const math::Position3* origin,
                       float* normal, int weapon, int surfType,
                       Entity* hitEnt)
{
    if (entity != nullptr)
    {
        float v17 = normal[0];
        float v18 = normal[1];
        float v19 = normal[2];
        if (surfType > 22)
            CG_ASSERT("surfType >= 0 && surfType < 23",
                      "c:\\cod\\code\\game\\cg_weapons.cpp", 4795);
        int ammoType = *(int*)((char*)BG_GetInfoForWeapon(weapon) + 0xA8);
        Entity* ent = hitEnt != nullptr ? hitEnt : entity;
        Client* client = entity->client;
        if (client != nullptr && (0x100000 & client->ps.eFlags) != 0
            && client->ps.vehPos <= 1)
        {
            Entity* v21 =
                EntityHandleDb_Get(entity->r.mOwner.mHandle.mVal);
            if (v21 == nullptr)
                CG_ASSERT("vehicle", "c:\\cod\\code\\game\\cg_weapons.cpp",
                          4811);
            if (v21->scr_vehicle == nullptr)
                CG_ASSERT("vehicle->scr_vehicle",
                          "c:\\cod\\code\\game\\cg_weapons.cpp", 4812);
            CollisionDesc v16;
            v16.coord.v = origin->v;
            v16.normal.v = _mm_setr_ps(v17, v18, v19, 0.0f);
            v16.material = surfType;
            PostEffectEventBulletHit(ent, (EWeaponClass)ammoType, v16);
            if (entity->client->ps.vehPos != 0)
                CG_BulletTrajectoryEffects(v21->mHandle.mHandle.mVal, origin,
                                           surfType,
                                           (char*)"tag_gunner_flash",
                                           weapon);
            else
                CG_BulletTrajectoryEffects(v21->mHandle.mHandle.mVal, origin,
                                           surfType, (char*)"tag_guncoax",
                                           weapon);
        }
        else
        {
            CollisionDesc v16;
            v16.coord.v = origin->v;
            v16.normal.v = _mm_setr_ps(v17, v18, v19, 0.0f);
            v16.material = surfType;
            PostEffectEventBulletHit(ent, (EWeaponClass)ammoType, v16);
            void* scr_vehicle = entity->scr_vehicle;
            if (scr_vehicle != nullptr)
            {
                if (*(int*)((char*)scr_vehicle + 0x19C) == weapon)
                {
                    CG_BulletTrajectoryEffects(
                        entity->mHandle.mHandle.mVal, origin, surfType,
                        (char*)"tag_guncoax", weapon);
                }
                else
                {
                    int shooter = *(int*)((char*)scr_vehicle + 0x1A4);
                    int fireBarrel = *(int*)((char*)scr_vehicle + 0x18C);
                    unsigned int mVal = entity->mHandle.mHandle.mVal;
                    if (shooter == 1)
                        CG_BulletTrajectoryEffects(
                            mVal, origin, surfType,
                            s_gunnerBarrelTags[fireBarrel], weapon);
                    else
                        CG_BulletTrajectoryEffects(
                            mVal, origin, surfType,
                            s_barrelTags[fireBarrel], weapon);
                }
            }
            else
            {
                CG_BulletTrajectoryEffects(entity->mHandle.mHandle.mVal,
                                           origin, surfType, s_barrelTags[0],
                                           weapon);
            }
        }
    }
}

// ea: 0x006AC070
void CG_BulletHitClientEvent(unsigned int sourceEntity,
                             const math::Position3* position,
                             float* normal, unsigned int surfType,
                             int weapon)
{
    if (sourceEntity == 0)
        CG_ASSERT("sourceEntity != TEntityHandle::NullHandle()",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 4854);
    if (surfType > 0x16)
        CG_ASSERT("surfType >= 0 && surfType < 23",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 4855);
    float v15 = normal[0];
    float v16 = normal[1];
    float v17 = normal[2];
    int ammoType = *(int*)((char*)BG_GetInfoForWeapon(weapon) + 0xA8);
    Entity* mObject = EntityHandleDb_Get(sourceEntity);
    if (mObject != nullptr && mObject->client != nullptr
        && EntityManager::sInst->IsLocalPlayer(mObject))
    {
        PostEffectEventScriptCall(mObject, "PLAYER_HIT_SUCCESS", false,
                                  (TPakId)0 /* PAK_ID_INVALID */, false);
    }
    CollisionDesc v14;
    v14.coord.v = position->v;
    v14.normal.v = _mm_setr_ps(v15, v16, v17, 0.0f);
    v14.material = surfType;
    Entity* Player = EntityManager::sInst->GetPlayer( currCl);
    PostEffectEventBulletHit(Player, (EWeaponClass)ammoType, v14);
    CG_BulletTrajectoryEffects(sourceEntity, position, surfType,
                               s_barrelTags[0], weapon);
}

enum EStanceType : int { kStanceStand = 0, kStanceCrouch = 1, kStanceProne = 2 };
extern Handle PostEffectEventFootstep(const Entity* ent, EStanceType stanceType,
                                      const CollisionDesc& col);
extern Handle PostEffectEventGearRattle(const Entity* ent, EStanceType stanceType,
                                        const CollisionDesc& col);
extern void SoundMediaMgr_PlayLandingSound(void* self, Entity* ent,
                                           int surfaceType, bool damage);
extern void* SoundMediaMgr_sInst;
extern PlayerState& GetPlayerState(int idx);
extern int cl_stance_ss[4];
extern vmCvar_t cg_stanceTemp;
extern vmCvar_t cg_nopredict;
extern int dword_F63BA8[4 * 1580];
extern int dword_F63BAC[4 * 1580];
extern float dword_F63BB0[4 * 1580];
extern int dword_F63BB4[4 * 1580];
extern Handle PostEffectEventWeaponReload(const Entity* ent,
                                          const char* weaponType,
                                          EAction weaponAction, bool queue);
extern Handle PostEffectEventWeapon(const Entity* ent, const char* weaponType,
                                    EAction weaponAction);
extern void CG_ItemPickup(int itemNum);
extern void CG_OutOfAmmoChange();
extern Handle PostEffectEventGrenadeBounce(const Entity* ent,
                                           const char* weaponType,
                                           const CollisionDesc& col_desc);
extern Handle PostEffectEventProjExplode(const Entity* ent,
                                         const char* weaponType,
                                         const CollisionDesc& col_desc);
extern void CG_EventSpawnTracer(const math::Position3* pstart,
                                const math::Position3* pend, int weapon);
extern void CG_RailTrail(const float* start, const float* end, float type);
extern void VEH_NetAltWeaponStatus(Entity* ent, int status);
extern vmCvar_t bg_fallDamageMaxHeight;
extern vmCvar_t bg_fallDamageMinHeight;
extern Entity* GetPlayer(int idx);

static CollisionDesc MakeCollisionDesc(const math::Position3* c,
                                       const float* n, int m)
{
    CollisionDesc d;
    d.coord.v = c->v;
    d.normal.v = _mm_setr_ps(n[0], n[1], n[2], 0.0f);
    d.material = m;
    return d;
}

// ea: 0x006AC480
void CG_EntityEvent(Entity* entity, int event, int bPredict)
{
    if (event == 0)
    {
        if (cg_debugEvents.integer != 0)
            CG_Printf("CG_EntityEvent:ZERO EVENT\n");
        return;
    }
    int eventParm = entity->s.eventParm;
    if (cg_debugEvents.integer != 0)
        CG_Printf("ent:0x%08x  event:%3i ", entity->mHandle.mHandle.mVal,
                  event);
    if (event <= 0)
        CG_ASSERT("event > 0", "c:\\cod\\code\\game\\cg_event.cpp", 175);
    if (event >= 223)
        CG_ASSERT("event < EV_MAX_EVENTS",
                  "c:\\cod\\code\\game\\cg_event.cpp", 176);
    if (cg_debugEvents.integer != 0)
        CG_Printf("CG_EntityEvent:%s\n", pEventNamesList[event]);
    unsigned int clientHandle = entity->mHandle.mHandle.mVal;
    float footstepPos[3];
    float zero[3] = {0.0f, 0.0f, 0.0f};
    if (bPredict != 0)
        goto label_43;
    if (event >= 1 && event < 24)
    {
        footstepPos[0] = entity->s.pos.trBase[0];
        footstepPos[1] = entity->s.pos.trBase[1];
        footstepPos[2] = entity->s.pos.trBase[2];
        CollisionDesc d = MakeCollisionDesc(
            (const math::Position3*)footstepPos, zero, event - 1);
        PostEffectEventFootstep(entity, (EStanceType)1, d);
        CollisionDesc d2 = MakeCollisionDesc(
            (const math::Position3*)footstepPos, zero, -1);
        PostEffectEventGearRattle(entity, (EStanceType)1, d2);
        return;
    }
    if (event >= 70 && event < 93)
    {
        footstepPos[0] = entity->s.pos.trBase[0];
        footstepPos[1] = entity->s.pos.trBase[1];
        footstepPos[2] = entity->s.pos.trBase[2];
        CollisionDesc d = MakeCollisionDesc(
            (const math::Position3*)footstepPos, zero, event - 70);
        PostEffectEventFootstep(entity, (EStanceType)0, d);
        CollisionDesc d2 = MakeCollisionDesc(
            (const math::Position3*)footstepPos, zero, -1);
        PostEffectEventGearRattle(entity, (EStanceType)0, d2);
        return;
    }
    if (event >= 24)
    {
        if (event < 47)
        {
            footstepPos[0] = entity->s.pos.trBase[0];
            footstepPos[1] = entity->s.pos.trBase[1];
            footstepPos[2] = entity->s.pos.trBase[2];
            CollisionDesc d = MakeCollisionDesc(
                (const math::Position3*)footstepPos, zero, event - 24);
            PostEffectEventFootstep(entity, (EStanceType)2, d);
            CollisionDesc d2 = MakeCollisionDesc(
                (const math::Position3*)footstepPos, zero, -1);
            PostEffectEventGearRattle(entity, (EStanceType)2, d2);
            return;
        }
        if (event < 70)
        {
            footstepPos[0] = entity->s.pos.trBase[0];
            footstepPos[1] = entity->s.pos.trBase[1];
            footstepPos[2] = entity->s.pos.trBase[2];
            CollisionDesc d = MakeCollisionDesc(
                (const math::Position3*)footstepPos, zero, event - 47);
            PostEffectEventFootstep(entity, (EStanceType)3, d);
            CollisionDesc d2 = MakeCollisionDesc(
                (const math::Position3*)footstepPos, zero, -1);
            PostEffectEventGearRattle(entity, (EStanceType)3, d2);
            return;
        }
    }
    if (event < 93)
        goto label_43;
    if (event < 116)
    {
        footstepPos[0] = entity->s.pos.trBase[0];
        footstepPos[1] = entity->s.pos.trBase[1];
        footstepPos[2] = entity->s.pos.trBase[2];
        CollisionDesc d = MakeCollisionDesc(
            (const math::Position3*)footstepPos, zero, event - 93);
        PostEffectEventFootstep(entity, (EStanceType)1, d);
        CollisionDesc d2 = MakeCollisionDesc(
            (const math::Position3*)footstepPos, zero, -1);
        PostEffectEventGearRattle(entity, (EStanceType)1, d2);
        return;
    }
    if (event < 139)
    {
        SoundMediaMgr_PlayLandingSound(SoundMediaMgr_sInst, entity,
                                       event - 116, false);
        if (clientHandle
            == *(unsigned int*)((char*)&GetPlayerState(currCl) + 0xA0))
        {
            int v14 = 1580 * currCl;
            int time = cgGlobal_time;
            *(float*)&dword_F63BB0[v14] = 0.0f - (float)eventParm;
            dword_F63BB4[v14] = time;
        }
        return;
    }
    if (event >= 162)
    {
    label_43:
        switch (event)
        {
        case 162:
            if (bPredict == 0)
                PostEffectEventScriptCall(entity, "Body_Bushes", false,
                                          (TPakId)0, false);
            break;
        case 165:
            if (cg_stanceTemp.integer == 0)
                cl_stance_ss[currCl] = 0;
            break;
        case 166:
            if (cg_stanceTemp.integer == 0)
                cl_stance_ss[currCl] = 1;
            break;
        case 167:
            if (cg_stanceTemp.integer == 0)
                cl_stance_ss[currCl] = 2;
            break;
        case 168:
            if (clientHandle
                    == *(unsigned int*)((char*)&GetPlayerState(currCl) + 0xA0)
                && cg_nopredict.integer == 0)
            {
                int v20 = cgGlobal_time;
                int v21 = 1580 * currCl;
                int v22 = cgGlobal_time - dword_F63BAC[1580 * currCl];
                float v23;
                if (v22 >= 100)
                    v23 = 0.0f;
                else
                    v23 = (((100 - v22) * *(float*)&dword_F63BA8[1580 * currCl])
                           * 0.0099999998f)
                          * 0.89999998f;
                float v24 = (float)(eventParm - 128) + v23;
                *(float*)&dword_F63BA8[1580 * currCl] = v24;
                if (v24 <= 24.0f)
                {
                    if (*(float*)&dword_F63BA8[v21] < -16.0f)
                        *(int*)&dword_F63BA8[v21] = -1048576000;
                    dword_F63BAC[v21] = v20;
                }
                else
                {
                    *(int*)&dword_F63BA8[v21] = 1103101952;
                    dword_F63BAC[v21] = v20;
                }
            }
            break;
        case 171:
        case 172:
        case 173:
            if (bPredict == 0)
            {
                eventParm = entity->s.eventParm;
                if (eventParm != 0 && eventParm < 137)
                {
                    if (event == 171)
                        PostEffectEventScriptCall(entity, "WEAPON_PICKUP",
                                                  false, (TPakId)0, false);
                    else if (event == 173)
                        PostEffectEventScriptCall(entity, "AMMO_PICKUP", false,
                                                  (TPakId)0, false);
                    Entity* v26 =
                        EntityHandleDb_Get(clientHandle);
                    if (v26 == GetPlayer(currCl))
                        CG_ItemPickup(eventParm);
                }
            }
            break;
        case 174:
            {
                Entity* v29 = EntityHandleDb_Get(clientHandle);
                if (v29 == GetPlayer(currCl))
                    CG_OutOfAmmoChange();
            }
            break;
        case 178:
            if (bPredict == 0)
            {
                void* InfoForWeapon =
                    (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventWeaponReload(entity,
                    *(const char**)((char*)InfoForWeapon + 8),
                    (EAction)0x10 /* kActionWEAPON_RELOAD_START */, false);
            }
            break;
        case 179:
            if (bPredict == 0)
            {
                void* v28 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventWeaponReload(entity,
                    *(const char**)((char*)v28 + 8),
                    (EAction)0x11 /* kActionWEAPON_RELOAD_END */, false);
            }
            break;
        case 180:
            {
                void* v30 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventWeapon(entity,
                                      *(const char**)((char*)v30 + 8),
                                      (EAction)0x13 /* kActionWEAPON_RAISE */);
            }
            break;
        case 182:
            if (bPredict == 0)
            {
                void* v31 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventWeapon(entity,
                                      *(const char**)((char*)v31 + 8),
                                      (EAction)0x12 /* kActionWEAPON_ALT_SWITCH */);
            }
            break;
        case 183:
            if (bPredict == 0)
            {
                void* v32 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventWeapon(entity,
                                      *(const char**)((char*)v32 + 8),
                                      (EAction)0x15 /* kActionWEAPON_DEPLOY */);
            }
            break;
        case 184:
            if (bPredict == 0)
            {
                void* v33 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventWeapon(entity,
                                      *(const char**)((char*)v33 + 8),
                                      (EAction)0x16 /* kActionWEAPON_BREAKDOWN */);
            }
            break;
        case 190:
            if (bPredict != 0)
            {
                void* v34 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventWeapon(entity,
                                      *(const char**)((char*)v34 + 8),
                                      (EAction)0x0D /* kActionWEAPON_RECHAMBER */);
            }
            break;
        case 191:
            if (bPredict != 0)
                CG_EjectWeaponBrass(entity, event);
            break;
        case 192:
            if (bPredict == 0)
            {
                void* v35 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventWeapon(
                    entity, *(const char**)((char*)v35 + 8),
                    (EAction)0x17 /* kActionWEAPON_NOTE_TRACK_SOUND_A */);
            }
            break;
        case 194:
            if (bPredict == 0)
            {
                void* v36 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventWeapon(
                    entity, *(const char**)((char*)v36 + 8),
                    (EAction)0x18 /* kActionWEAPON_NOTE_TRACK_SOUND_B */);
            }
            break;
        case 197:
            if (bPredict != 0)
                CG_FireWeapon(entity, &entity->s, event, 0);
            break;
        case 198:
            if (bPredict != 0)
            {
                CG_FireWeapon(entity, &entity->s, event, 0);
                CG_FireWeapon(entity, &entity->s, event, 1);
            }
            break;
        case 199:
            if (bPredict != 0)
            {
                CG_FireWeapon(entity, &entity->s, event, 2);
                CG_FireWeapon(entity, &entity->s, event, 3);
            }
            break;
        case 200:
            {
                int weapon = entity->s.weapon;
                math::Position3 v76;
                v76 = native_to_cdl_pos3(entity->s.pos.trBase);
                CG_EventSpawnTracer(&v76, &entity->s.origin2, weapon);
            }
            break;
        case 206:
            if (entity->s.surfType >= 0x17)
                CG_ASSERT("es->surfType >= 0 && es->surfType < 23",
                          "c:\\cod\\code\\game\\cg_event.cpp", 571);
            ByteToDir(entity->s.eventParm, (float*)&clientHandle);
            if (bPredict == 0)
            {
                float nrm[3];
                ByteToDir(entity->s.eventParm, nrm);
                CollisionDesc d = MakeCollisionDesc(
                    &entity->r.currentOrigin, nrm,
                    *(unsigned char*)((char*)&entity->s + 5));
                void* v43 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                PostEffectEventGrenadeBounce(
                    entity, *(const char**)((char*)v43 + 8), d);
            }
            break;
        case 210:
        case 211:
            if (entity->s.surfType >= 0x17)
                CG_ASSERT("es->surfType >= 0 && es->surfType < 23",
                          "c:\\cod\\code\\game\\cg_event.cpp", 595);
            ByteToDir(entity->s.eventParm, (float*)&clientHandle);
            {
                int v44 = *(unsigned char*)((char*)&entity->s + 5);
                if (v44 != 8)
                {
                    float nrm[3];
                    ByteToDir(entity->s.eventParm, nrm);
                    CollisionDesc d = MakeCollisionDesc(
                        &entity->r.currentOrigin, nrm,
                        entity->s.surfType);
                    if (v44 == 1)
                    {
                        entity->flags &= ~0x400;
                        entity->s.eFlags &= ~0x80u;
                    }
                    void* v51 = (void*)BG_GetInfoForWeapon(entity->s.weapon);
                    PostEffectEventProjExplode(
                        entity, *(const char**)((char*)v51 + 8), d);
                    if (eventParm != 0)
                    {
                        entity->r.eventType |= 4u;
                        entity->r.eventTime = eventParm;
                    }
                    else
                    {
                        entity->r.eventType |= 1u;
                    }
                }
            }
            break;
        case 212:
        case 213:
            CG_ASSERT("0", "c:\\cod\\code\\game\\cg_event.cpp", 655);
            break;
        case 214:
            CG_RailTrail(entity->s.origin2.v.m128_f32,
                         entity->s.pos.trBase, entity->s.dmgFlags);
            break;
        case 221:
            VEH_NetAltWeaponStatus(entity, eventParm);
            break;
        case 222:
            if (bPredict == 0)
            {
                float nrm[3] = {0.0f, 0.0f, 1.0f};
                CollisionDesc d = MakeCollisionDesc(
                    &entity->r.currentOrigin, nrm, entity->s.surfType);
                PostEffectEventProjExplode(entity, "fraggrenade", d);
            }
            break;
        default:
            break;
        }
    }
    else
    {
        SoundMediaMgr_PlayLandingSound(SoundMediaMgr_sInst, entity,
                                       event - 139, true);
        if (clientHandle
            == *(unsigned int*)((char*)&GetPlayerState(currCl) + 0xA0))
        {
            float v16 =
                ((*(float*)&bg_fallDamageMaxHeight
                  - *(float*)&bg_fallDamageMinHeight)
                 * (eventParm * 0.0099999998f))
                + *(float*)&bg_fallDamageMinHeight;
            if (v16 > 12.0f)
            {
                int v17 = (int)((((v16 - 12.0f) * 0.03846154f) + 1.0f)
                                * 4.0f);
                if (v17 > 24)
                    v17 = 24;
                if (v17 <= 0)
                    return;
                int v18 = 1580 * currCl;
                int v19 = cgGlobal_time;
                *(float*)&dword_F63BB0[v18] = 0.0f - (float)v17;
                dword_F63BB4[v18] = v19;
            }
        }
    }
}

// ea: 0x006A1730
void CG_AdjustPositionForMover(const math::Position3* in, unsigned int mover,
                               int fromTime, int toTime, math::Position3* out,
                               float* outDeltaAngles)
{
    if (outDeltaAngles != nullptr)
    {
        outDeltaAngles[1] = 0.0f;
        outDeltaAngles[0] = 0.0f;
    }
    unsigned int v7 = mover & 0xFFF;
    Entity* mObject = nullptr;
    if (v7 < 0x540
        && (mover >> 12) == EntityHandleDb::sInst.mElements[v7].mKey)
        mObject = EntityHandleDb::sInst.mElements[v7].mObject;
    if (mObject == nullptr
        || (mObject->s.eType != 4 && mObject->s.eType != 7))
    {
        out->v.m128_f32[0] = in->v.m128_f32[0];
        out->v.m128_f32[1] = in->v.m128_f32[1];
        out->v.m128_f32[2] = in->v.m128_f32[2];
        return;
    }
    math::Position3 fromPos, fromAngles, toPos, toAngles;
    BG_EvaluateTrajectory(&mObject->s.pos, fromTime, fromPos);
    BG_EvaluateTrajectory(&mObject->s.apos, fromTime, fromAngles);
    BG_EvaluateTrajectory(&mObject->s.pos, toTime, toPos);
    BG_EvaluateTrajectory(&mObject->s.apos, toTime, toAngles);
    out->v.m128_f32[0] = in->v.m128_f32[0]
                         + (toPos.v.m128_f32[0] - fromPos.v.m128_f32[0]);
    out->v.m128_f32[1] = in->v.m128_f32[1]
                         + (toPos.v.m128_f32[1] - fromPos.v.m128_f32[1]);
    out->v.m128_f32[2] = in->v.m128_f32[2]
                         + (toPos.v.m128_f32[2] - fromPos.v.m128_f32[2]);
    if (outDeltaAngles != nullptr)
    {
        outDeltaAngles[0] = toAngles.v.m128_f32[0]
                            - fromAngles.v.m128_f32[0];
        outDeltaAngles[1] = toAngles.v.m128_f32[1]
                            - fromAngles.v.m128_f32[1];
        outDeltaAngles[2] = toAngles.v.m128_f32[2]
                            - fromAngles.v.m128_f32[2];
    }
}

// ea: 0x006AD370
void CG_EntityPreEvent(Entity* entity, int event)
{
    if (event <= 0)
        CG_ASSERT("event > 0", "c:\\cod\\code\\game\\cg_event.cpp", 753);
    if (event >= 223)
        CG_ASSERT("event < EV_MAX_EVENTS",
                  "c:\\cod\\code\\game\\cg_event.cpp", 754);
    if (cg_debugEvents.integer != 0)
    {
        CG_Printf("ent:0x%08x  preevent:%3i CG_EntityPreEvent:%s\n",
                  entity->mHandle.mHandle.mVal, event,
                  pEventNamesList[event]);
    }
    switch (event)
    {
    case 186:
    case 187:
    case 189:
        if (EntityManager::sInst->IsLocalPlayer(entity))
            goto fire_weapon;
        if (entity->s.eType == 14)
        {
            Entity* v4 =
                EntityHandleDb_Get(entity->r.mOwner.mHandle.mVal);
            if (v4 != nullptr && v4->IsLocalPlayer())
                goto fire_weapon;
        }
        break;
    case 190:
        {
            void* InfoForWeapon =
                (void*)BG_GetInfoForWeapon(entity->s.weapon);
            PostEffectEventWeapon(
                entity, *(const char**)((char*)InfoForWeapon + 8),
                (EAction)13 /* kActionWEAPON_RECHAMBER */);
        }
        break;
    case 191:
        CG_EjectWeaponBrass(entity, event);
        break;
    case 197:
    fire_weapon:
        CG_FireWeapon(entity, &entity->s, event, 0);
        break;
    case 198:
        CG_FireWeapon(entity, &entity->s, event, 0);
        CG_FireWeapon(entity, &entity->s, event, 1);
        break;
    case 199:
        CG_FireWeapon(entity, &entity->s, event, 2);
        CG_FireWeapon(entity, &entity->s, event, 3);
        break;
    case 203:
        {
            float reflect[3];
            ByteToDir(entity->s.eventParm, reflect);
            float dir2[3];
            ByteToDir(*(unsigned char*)((char*)&entity->s + 5), dir2);
            Entity* v6 =
                EntityHandleDb_Get(entity->s.mOtherEntity.mHandle.mVal);
            CG_BulletHitEvent(entity, &entity->r.currentOrigin, reflect,
                              entity->s.weapon, entity->s.surfType, v6);
        }
        break;
    case 204:
    case 205:
        {
            int weapon = entity->s.weapon;
            int surfType = entity->s.surfType;
            float reflect_unused[3] = {0.0f, 0.0f, 0.0f};
            CG_BulletHitClientEvent(
                entity->s.mOtherEntity.mHandle.mVal, &entity->s.lerpOrigin,
                reflect_unused, surfType, weapon);
        }
        break;
    case 220:
        CG_StartShakeCamera(entity->s.angles2.v.m128_f32[0], 1000,
                            entity->r.currentOrigin.v.m128_f32,
                            entity->s.angles2.v.m128_f32[1], currCl);
        break;
    default:
        break;
    }
}

// ea: 0x006AE040
int CG_ProcessSnapshots()
{
    int n = 0;
    CL_GetCurrentSnapshotNumber(&n, &dword_F62958[1580 * currCl]);
    int v1 = n;
    int v2 = 1580 * currCl;
    int v3 = dword_F62954[1580 * currCl];
    if (n != v3)
    {
        if (n < v3)
        {
            CG_Error("CG_ProcessSnapshots: n < cg[currCl].latestSnapshotNum");
            v1 = n;
        }
        v2 = 1580 * currCl;
        dword_F62954[1580 * currCl] = v1;
    }
    int ServerSnapTime = 0;
    if (dword_F62960[v2] != 0)
    {
        CG_SetFrameInterpolation();
        while (1)
        {
            CG_ASSERT("cg[currCl].snap",
                      "c:\\cod\\code\\game\\cg_snapshot.cpp", 412);
            CG_ASSERT("cg[currCl].nextSnap",
                      "c:\\cod\\code\\game\\cg_snapshot.cpp", 413);
            if (dword_F62964[1580 * currCl] == dword_F62960[1580 * currCl])
            {
                void* NextSnapshot = CG_ReadNextSnapshot();
                if (NextSnapshot == nullptr)
                    break;
                CG_ASSERT("cg[currCl].snap",
                          "c:\\cod\\code\\game\\cg_snapshot.cpp", 422);
                if (*(int*)((char*)NextSnapshot + 4)
                        - *(int*)(dword_F62960[1580 * currCl] + 4)
                    < 0)
                {
                    CG_Error("CG_ProcessSnapshots: Server time went "
                             "backwards\n");
                }
                CG_SetNextSnap(NextSnapshot);
            }
            if (cgGlobal_time - *(int*)(dword_F62960[1580 * currCl] + 4) >= 0
                && cgGlobal_time - *(int*)(dword_F62964[1580 * currCl] + 4)
                       < 0)
                break;
            CG_TransitionSnapshot();
        }
        CG_ASSERT("cg[currCl].snap",
                  "c:\\cod\\code\\game\\cg_snapshot.cpp", 438);
        CG_ASSERT("cg[currCl].nextSnap",
                  "c:\\cod\\code\\game\\cg_snapshot.cpp", 439);
        int v10 = dword_F62964[1580 * currCl];
        if (v10 != dword_F62960[1580 * currCl]
            && *(int*)(v10 + 4) - cgGlobal_time <= 0)
        {
            CG_ASSERT("cg[currCl].nextSnap == cg[currCl].snap || "
                      "cg[currCl].nextSnap->serverTime - cgGlobal.time > 0",
                      "c:\\cod\\code\\game\\cg_snapshot.cpp", 440);
        }
        if (cgGlobal_time - *(int*)(dword_F62960[1580 * currCl] + 4) < 0)
        {
            CG_ASSERT("cgGlobal.time - cg[currCl].snap->serverTime >= 0",
                      "c:\\cod\\code\\game\\cg_snapshot.cpp", 441);
        }
        ServerSnapTime = G_GetServerSnapTime();
        if (*(int*)(dword_F62964[1580 * currCl] + 4) != ServerSnapTime)
        {
            CG_ASSERT("cg[currCl].nextSnap->serverTime == "
                      "G_GetServerSnapTime()",
                      "c:\\cod\\code\\game\\cg_snapshot.cpp", 442);
        }
    }
    else
    {
        void* v4 = CG_ReadNextSnapshot();
        CG_ASSERT("snap", "c:\\cod\\code\\game\\cg_snapshot.cpp", 378);
        CG_SetInitialSnapshot((snapshot_t*)v4);
        CG_SetNextSnap(v4);
        CG_ASSERT("cg[currCl].snap",
                  "c:\\cod\\code\\game\\cg_snapshot.cpp", 385);
        CG_ASSERT("cg[currCl].nextSnap",
                  "c:\\cod\\code\\game\\cg_snapshot.cpp", 386);
        if (*(int*)(dword_F62964[1580 * currCl] + 4) != G_GetServerSnapTime())
        {
            CG_ASSERT("cg[currCl].nextSnap->serverTime == "
                      "G_GetServerSnapTime()",
                      "c:\\cod\\code\\game\\cg_snapshot.cpp", 387);
        }
        CG_TransitionSnapshot();
        if (fs_debug_vm.integer == 0)
            Cvar_VMSet(&fs_debug_vm, "2");
        CG_ASSERT("cg[currCl].snap",
                  "c:\\cod\\code\\game\\cg_snapshot.cpp", 397);
        CG_ASSERT("cg[currCl].nextSnap",
                  "c:\\cod\\code\\game\\cg_snapshot.cpp", 398);
        int v6 = dword_F62964[1580 * currCl];
        if (v6 != dword_F62960[1580 * currCl]
            && *(int*)(v6 + 4) - cgGlobal_time <= 0)
        {
            CG_ASSERT("cg[currCl].nextSnap == cg[currCl].snap || "
                      "cg[currCl].nextSnap->serverTime - cgGlobal.time > 0",
                      "c:\\cod\\code\\game\\cg_snapshot.cpp", 399);
        }
        if (cgGlobal_time - *(int*)(dword_F62960[1580 * currCl] + 4) < 0)
        {
            CG_ASSERT("cgGlobal.time - cg[currCl].snap->serverTime >= 0",
                      "c:\\cod\\code\\game\\cg_snapshot.cpp", 400);
        }
        ServerSnapTime = G_GetServerSnapTime();
        if (*(int*)(dword_F62964[1580 * currCl] + 4) != ServerSnapTime)
        {
            CG_ASSERT("cg[currCl].nextSnap->serverTime == "
                      "G_GetServerSnapTime()",
                      "c:\\cod\\code\\game\\cg_snapshot.cpp", 401);
        }
    }
    return ServerSnapTime;
}
