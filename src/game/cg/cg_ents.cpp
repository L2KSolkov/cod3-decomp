// ============================================================================
// cg_ents.cpp - local entities, server config parsing, grenade counts (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"

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
