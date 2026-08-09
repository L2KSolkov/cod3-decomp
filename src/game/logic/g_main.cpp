// ============================================================================
// g_main.cpp - game entry + console commands (g.o: g_main.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <stdlib.h>
#include <string.h>

#include "render/ShaderCommon.h"
#include "ngl/nglDebug.h"

extern nglDebugStruct nglDebug;  // ngl_debug.o
extern int gRenderCG_2D;         // cg.o 0x011E86E4
extern int g_renderGameEntityStats;  // game2.o 0x012F3E10
extern int Cmd_Argc(void);
extern void Cmd_ArgvBuffer(int arg, char* buffer, int bufferLength);
extern void Com_FreeWeaponInfoMemory(int iSource, int bRestart);

// ea: 0x00448B70
void MemGraph_RenderResources(void)
{
    ;
}

// ea: 0x0044AB50
void Cmd_Wireframe_f(void)
{
    if (ShaderCommon::GetDebugRenderMode() == ShaderCommon::kDebugRenderModeWireframe)
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeNormal);
    else
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeWireframe);
}

// ea: 0x0044AB70
void Cmd_Fullbright_f(void)
{
    if (ShaderCommon::GetDebugRenderMode() == ShaderCommon::kDebugRenderModeFullbright)
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeNormal);
    else
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeFullbright);
}

// ea: 0x0044AC10
void Cmd_TextureTiling_f(void)
{
    if (ShaderCommon::GetDebugRenderMode() == ShaderCommon::kDebugRenderModeTextureTiling)
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeNormal);
    else
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeTextureTiling);
}

// ea: 0x0044AC90
void Cmd_TestFPS(void)
{
    if (TestFPS::sInst->mTesting)
        TestFPS::sInst->StopTest();
    else
        TestFPS::sInst->Test();
}

// ea: 0x0044B750
int G_GetServerSnapTime(void)
{
    return level.snapTime;
}

// ea: 0x0044B8B0
void G_EndGame(void)
{
    Com_Error(ERR_ENDGAME, "endgame");
}

// ea: 0x0044BAB0
char* GetScratchPad(void)
{
    return g_scratchpadMem;
}

// ea: 0x004508B0
void game_dllEntry(int (*syscallptr)(int, ...))
{
    syscall = syscallptr;
}

// ea: 0x00450AB0
void g_UnlinkEntity(Entity* ent)
{
    SV_UnlinkEntity(ent);
}

// ea: 0x00452D60
void SnapVectorTowards(float* /*v*/, float* /*target*/)
{
    ;
}

// ea: 0x0044AAF0
void Cmd_NGLStats_f(void)
{
    Com_Printf("nglstats: \n");
}

// ea: 0x0044AB00
bool Cmd_NGLStatDisplay_f(void)
{
    bool result = nglDebug.ShowPerfInfo != 1;
    nglDebug.ShowPerfInfo = nglDebug.ShowPerfInfo != 1;
    return result;
}

// ea: 0x0044AB10
void Cmd_ProfileShaders_f(void)
{
    nglProfileShaders();
}

// ea: 0x0044AB20
void Cmd_ProfileNodes_f(void)
{
    ;
}

// ea: 0x0044AB90
void Cmd_SolidColor_f(void)
{
    if (ShaderCommon::GetDebugRenderMode() == ShaderCommon::kDebugRenderModeSolidColor)
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeNormal);
    else
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeSolidColor);
}

// ea: 0x0044AC40
void Cmd_ToggleShader_f(void)
{
    if (Cmd_Argc() == 2)
    {
        char s[256];
        Cmd_ArgvBuffer(1, s, 256);
        ShaderCommon::ToggleShader(s);
    }
    else if (Cmd_Argc() == 1)
    {
        ShaderCommon::ToggleShader(nullptr);
    }
}

// ea: 0x0044ACB0
void Cmd_KillSound(void)
{
    SoundDevice::sInst->StopAllSounds();
}

// ea: 0x0044AD90
void Cmd_BuilderTest_f(void)
{
    ShaderCommon::ToggleShader(defaultFileName);
    ShaderCommon::ToggleShader("cdworld");
    ShaderCommon::ToggleShader("cdworldblend");
    ShaderCommon::ToggleShader("cdsimpleinstance");
    ShaderCommon::ToggleShader("cdsimplealpha");
    ShaderCommon::ToggleShader("cddecal");
    ShaderCommon::ToggleShader("cdocean");
    ShaderCommon::ToggleShader("cdriver");
    ShaderCommon::ToggleShader("cdglass");
    ShaderCommon::ToggleShader("cdworldlit");
    ShaderCommon::ToggleShader("cdblendpointlit");
    gRenderCG_2D ^= 1u;
}

// ea: 0x0044AE30
void Cmd_Thread_Debug_f(void)
{
    ;
}

// ea: 0x0044AE60
int Cmd_EntityStats_f(void)
{
    int result = Cmd_Argc();
    if (result == 2)
    {
        char tmpstr[64];
        Cmd_ArgvBuffer(1, tmpstr, 64);
        g_renderGameEntityStats = strcmp(tmpstr, "1") == 0;
        return g_renderGameEntityStats;
    }
    g_renderGameEntityStats = 0;
    return result;
}

// ea: 0x0044A780
void Cmd_Invinc_f(Entity* /*ent*/)
{
    EntityManager::sInst->GetPlayer(currCl);
}

// ea: 0x0044AB30
int Cmd_NGLFPSDisplay_f(void)
{
    int result = nglDebug.ShowPerfInfo == 2 ? 0 : 2;
    nglDebug.ShowPerfInfo = nglDebug.ShowPerfInfo == 2 ? 0 : 2;
    return result;
}

// ea: 0x0044ABB0
void Cmd_TextureSize_f(void)
{
    if (ShaderCommon::GetDebugRenderMode() == ShaderCommon::kDebugRenderModeTextureSize)
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeNormal);
    else
        ShaderCommon::SetDebugRenderMode(ShaderCommon::kDebugRenderModeTextureSize);
}

// ea: 0x0044BA00
void G_RunPreFrame(int msec)
{
    ++level.framenum;
    level.previousTime = level.time;
    level.time += msec;
}

// ea: 0x0044BA90
void G_SendClientMessages(void)
{
    level.snapTime = level.time;
    if (level.bRegisterItems != 0)
        SaveRegisteredItems();
}

// ea: 0x00448B50
void StatusBar::Init()
{
    sStatusBarActive = Cvar_Get("statusbar", "1", 256);
}

// ea: 0x00450CF0
void g_AddDebugString(float* xyz, float* color, float scale, const char* pszText)
{
    CL_AddDebugString(xyz, color, scale, pszText, 1);
}

// ea: 0x00455F40
void Cmd_Fogswitch_f(void)
{
    const char* v0 = ConcatArgs(1);
    G_setfog(v0);
}

// ea: 0x00457F60
void G_LoadAnimTreeInstances(bool /*bReload*/)
{
    for (int i = 0; i < 16; ++i)
        g_scr_data.actorCorpseInfo[i].mEntity.mHandle.mVal = 0;
}

// ea: 0x00457FC0
void G_ClearLowHunk(void)
{
    Scr_FreePrecachedAnimTrees();
    Com_FreeWeaponInfoMemory(1, 0);
}

// ea: 0x00460E90
void EntityHandleDb::Find(int fieldOfs, const Broc::string* match,
                          ae_sized_array<Entity*, 4096>* results)
{
    Broc::string v4 = *match;
    EntityHandleDb_Find<Broc::string>(fieldOfs, v4, *results);
}

// ea: 0x00460660
void EntityDeathTask::Update(Entity* e, float /*delta*/)
{
    if ((mFlags & 4) == 0)
        G_FreeEntity(e, 0);
}

// ea: 0x004541A0
EntityDeathTask::EntityDeathTask(DbLinkedHandle<EntityHandleDb, Entity> h)
    : Task(h, 0x44455448 /* 'DETH' */)
{
    __vftable = 0;  // patched by task registration in the original binary
}

// ea: 0x0044FCE0
int G_ResetEntryPointHintIndicies(void)
{
    sEntryPointHintIndicies[0] = -1;
    dword_DD67B8 = -1;
    dword_DD67BC = -1;
    dword_DD67C0 = -1;
    dword_DD67C4 = -1;
    dword_DD67C8 = -1;
    return -1;
}

// ea: 0x004541D0
void G_AddPredictableEvent(Entity* ent, int event, int eventParm)
{
    if (ent->client != nullptr)
        BG_AddPredictableEventToPlayerstate(event, eventParm, &ent->client->ps);
}

// ea: 0x00456FB0
void G_DebugLine(const float* start, const float* end, const float* color,
                 int depthTest, int duration)
{
    CL_AddDebugLine(start, end, color, depthTest, duration, 1, 0);
}

// ea: 0x00450D10
void g_AddDebugLine(const float* start, const float* end, const float* color,
                    int depthTest, int duration, int fadeOut)
{
    CL_AddDebugLine(start, end, color, depthTest, duration, 1, fadeOut);
}

// ea: 0x00448F20
void ClientIntermissionThink(Entity* ent, usercmd_s* ucmd)
{
    Client* client = ent->client;
    client->oldbuttons = client->buttons;
    client->buttons = ucmd->buttons;
}

// ea: 0x00449410
void SP_intermission(Entity* ent)
{
    ent->mClassName = str_const.spawn_intermission;
    UpdateEntityHash(ent);
}

// ea: 0x0044AAB0
void Cmd_SetSpawnPoint_f(void)
{
    if (Cmd_Argc() == 2)
    {
        char arg[128];
        Cmd_ArgvBuffer(1, arg, 128);
        atoi(arg);
    }
}

// ea: 0x004581C0
void G_XAnimUpdateEnt(Entity* ent)
{
    while (ent != nullptr
           && (ent->flags & 0x10000) == 0
           && SV_DObjUpdateServerTime(ent, ServerTime::sInst.mTickMSec * 0.001f, true))
    {
        ;
    }
}

// ea: 0x00458820 (TeleportPlayer)
void TeleportPlayer(Entity* player, const float* origin, const float* angles)
{
    SetClientOrigin(player, origin);
    SetClientViewAngle(player, angles);
    g_LinkEntity(player);
}

// ea: 0x00449380
void G_SetupSpawnPoint(Entity* pEnt)
{
    pEnt->nextthink = level.time + 200;
    pEnt->think = THINK__G_FinishSetupSpawnPoint;
}

// ea: 0x00448FB0
int G_ClientCanSpectateTeam(Entity* ent, team_t team)
{
    return ((1 << team) & ent->sentient->noSpectate) == 0;
}

// ea: 0x0046A250
void render_sphere(const math::Position3* center, float radius, const float* color)
{
    if (render)
    {
        debug_sphere v4;
        v4.x = center->v.m128_f32[0];
        v4.y = center->v.m128_f32[1];
        v4.z = center->v.m128_f32[2];
        v4.radius = radius;
        v4.color[0] = color[0];
        v4.color[1] = color[1];
        v4.color[2] = color[2];
        v4.color[3] = color[3];
        debug_spheres.mElements[debug_spheres.mSize++] = v4;
    }
}
