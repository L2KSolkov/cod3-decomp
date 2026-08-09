// ============================================================================
// cg_ents.cpp - local entities, server config parsing, grenade counts (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

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
