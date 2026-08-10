// ============================================================================
// g_main.cpp - game entry + console commands (g.o: g_main.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "render/ShaderCommon.h"
#include "ngl/nglDebug.h"
#include "core/PoolAllocator.h"

extern PoolAllocator* gCommonPoolAllocator;  // core.o 0x012EFF18
extern "C" int __fpclass(float);

extern nglDebugStruct nglDebug;  // ngl_debug.o
extern int gRenderCG_2D;         // cg.o 0x011E86E4
extern int g_renderGameEntityStats;  // game2.o 0x012F3E10

// ============================================================================
// getBuildNumber - ea: 0x608FE0
// ============================================================================
static char buf[64];  // ?buf@@3PADA (game.o data 0xF44FC8)

// ea: 0x00608FE0
char* getBuildNumber()
{
    sprintf(buf, "%d %s %s", 4504, "Oct  8 2006", "00:54:43");
    return buf;
}
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

// ea: 0x0044AC30
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

// ea: 0x0044AD60
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

// ea: 0x0044AE40
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

// ea: 0x00458770
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

// ea: 0x0044ABD0
void Cmd_TextureMip_f(void)
{
    if (Cmd_Argc() == 2)
    {
        char s[256];
        Cmd_ArgvBuffer(1, s, 256);
        ShaderCommon::SetTextureSizeMipLevel(atoi(s));
    }
}

// ea: 0x00456120
void Cmd_LockPVSFlash_f(void)
{
    gEnableMeshFlash ^= 1u;
    if (gEnableMeshFlash)
        SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(), "print \"PVS lock flash is on\"");
    else
        SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(), "print \"PVS lock flash is off\"");
}

// ea: 0x004499E0
void SP_sd_axis(Entity* ent)
{
    if (_stricmp("sd", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_sd_axis;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00474FE0
void player_die(Entity* self, Entity* inflictor, Entity* attacker, int damage,
                int meansOfDeath, int iWeapon, const float* vPosition,
                const float* vDir, hitLocation_t hitLoc)
{
    if (self->client->ps.pm_type < 6)
    {
        Scr_NotifyFromEnt(self, hash_const.death, attacker);
        self->enemy = attacker;
        LookAtKiller(self, inflictor, attacker);
        Entity* mObject = HandleDbToEnt(self->client->ps.mClient);
        int PlayerIndex = mObject->GetPlayerIndex();
        g_femanager.IGO->SetFuse(-1.0f, -1.0f, PlayerIndex);
        self->client->ps.pm_type = 7 - (self->client->ps.pm_type != 1);
        MultiplayerMgr::sInst->PlayerDead(self, inflictor, attacker, damage,
                                          meansOfDeath, iWeapon, vPosition, vDir,
                                          (int)hitLoc);
    }
}

// ea: 0x004497B0
void SP_single_ctf_allies(Entity* ent)
{
    if (_stricmp("scf", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_single_ctf_allies;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449800
void SP_single_ctf_axis(Entity* ent)
{
    if (_stricmp("scf", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_single_ctf_axis;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449850
void SP_dom_allies(Entity* ent)
{
    if (_stricmp("dom", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_dom_allies;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x004498A0
void SP_dom_axis(Entity* ent)
{
    if (_stricmp("dom", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_dom_axis;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x004498F0
void SP_war_allies(Entity* ent)
{
    if (_stricmp("war", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_war_allies;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449940
void SP_war_axis(Entity* ent)
{
    if (_stricmp("war", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_war_axis;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449990
void SP_sd_allies(Entity* ent)
{
    if (_stricmp("sd", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_sd_allies;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449440
void SP_deathmatch(Entity* ent)
{
    if (_stricmp("dm", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_deathmatch;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449490
void SP_teamdeathmatch(Entity* ent)
{
    if (_stricmp("tdm", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_teamdeathmatch;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x004494E0
void SP_ctf_allies_primary(Entity* ent)
{
    if (_stricmp("ctf", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_ctf_allies_primary;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449530
void SP_ctf_allies_secondary(Entity* ent)
{
    if (_stricmp("ctf", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_ctf_allies_secondary;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449580
void SP_ctf_axis_primary(Entity* ent)
{
    if (_stricmp("ctf", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_ctf_axis_primary;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x004495D0
void SP_ctf_axis_secondary(Entity* ent)
{
    if (_stricmp("ctf", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_ctf_axis_secondary;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449620
void SP_hq_allies_primary(Entity* ent)
{
    if (_stricmp("hq", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_hq_allies_primary;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449670
void SP_hq_allies_secondary(Entity* ent)
{
    if (_stricmp("hq", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_hq_allies_secondary;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x004496C0
void SP_hq_axis_primary(Entity* ent)
{
    if (_stricmp("hq", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_hq_axis_primary;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449710
void SP_hq_axis_secondary(Entity* ent)
{
    if (_stricmp("hq", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.spawn_hq_axis_secondary;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x00449760
void HQ_Point(Entity* ent)
{
    if (_stricmp("hq", mp_gametype.string) == 0)
    {
        ent->mClassName = str_const.hq_point;
        UpdateEntityHash(ent);
    }
    else
    {
        no_really_delete_it = true;
    }
}

// ea: 0x0044ACC0
void Cmd_MemPools_f(void)
{
    tlPrintf("Common Pool Allocator\n");
    gCommonPoolAllocator->ReportAllocations();
    tlPrintf("\n\nBroc Common Pool\n");
    gBrocPool->ReportAllocations();
    tlPrintf("\n\nBroc Thread Pool Allocator\n");
    gAeThreadBackupStackAllocator->ReportAllocations();
}

// ea: 0x0044AD10
void Cmd_ShotProf_f(void)
{
    if (gShotProf == nullptr)
    {
        gRenderCG_2D = 0;
        nglDebug.ShowPerfInfo = 0;
        Cvar_Set("statusbar", "0");
        if (TimerRenderBars::sInst.mActive != 0)
            TimerRenderBars::sInst.mActive ^= 1u;
        gShotProf = ShaderCommon_StartShotPerfTest();
    }
}

// ea: 0x0044ADE0
void Cmd_ClientCommandCompletion(void (*callback)(const char*))
{
    for (unsigned int i = 0; i < 24; ++i)
        callback(sClientCommand0List[i].first);
    for (unsigned int j = 0; j < 15; ++j)
        callback(sClientCommand1List[j].first);
}

// ea: 0x0044BA30
void UpdateCVars(void)
{
    cdl_proftimer_cvar.start();
    for (int v0 = 0; v0 < gameCvarTableSize; ++v0)
    {
        CVarTable* i = &gameCvarTable[v0];
        if (i->vmCvar != nullptr)
        {
            Cvar_Update(i->vmCvar);
            int modificationCount = i->vmCvar->modificationCount;
            if (i->modificationCount != modificationCount)
                i->modificationCount = modificationCount;
        }
    }
    cdl_proftimer_cvar.stop();
}

// ea: 0x004493B0
void SP_info_player_start(Entity* ent)
{
    ent->mClassName = str_const.info_player_deathmatch;
    ent->mClassNameHash.mHash = HashString::CalcHash(ent->mClassName.c_str());
    ent->nextthink = level.time + 200;
    ent->think = THINK__G_FinishSetupSpawnPoint;
    UpdateEntityHash(ent);
}

// ea: 0x00448F50
void IntermissionClientEndFrame(Entity* ent)
{
    Client* client = ent->client;
    ent->r.svFlags = (ent->r.svFlags & 0xFFFFFFF6) | 1;
    ent->takedamage = 0;
    ent->r.contents = 0;
    client->ps.pm_flags &= ~0x80000u;
    client->ps.pm_type = 5;
    client->ps.eFlags &= 0xFFFFFDFF;
    ent->s.eType = 6;
}

// ea: 0x00450A50
void g_LinkEntity(Entity* ent)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 220;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    SV_LinkEntity(ent);
}

// ea: 0x00456010
void Cmd_Where_f(Entity* ent)
{
    if (!GamePause::IsGamePaused(currCl))
    {
        Client* client = ent->client;
        if (client != nullptr)
        {
            unsigned int mVal = ent->mHandle.mHandle.mVal;
            char* v3 = vtos(&client->ps.origin);
            SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(), va("print \"%s\"", v3));
            strcpy(cg_drawPosition->string, vtos(&ent->client->ps.origin));
        }
    }
}

// ea: 0x0044AEA0
int Cmd_PFXStats_f(void)
{
    int result = Cmd_Argc();
    if (result == 2)
    {
        char tmpstr[64];
        Cmd_ArgvBuffer(1, tmpstr, 64);
        g_renderPFXStats = strcmp(tmpstr, "1") == 0;
        return g_renderPFXStats;
    }
    g_renderPFXStats = 0;
    return result;
}

// ea: 0x00448FE0
int GetFollowPlayerState(int clientNum, PlayerState* ps)
{
    Client* client = EntityManager::sInst->GetPlayer(clientNum)->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
        AeAssert::gCurrentLine = 1691;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((client->ps.pm_flags & 0x80000) == 0)
        return 0;
    *ps = client->ps;
    return 1;
}

// ea: 0x0044A390
void Cmd_Take_f(Entity* ent)
{
    if (g_cheats->integer == 0)
    {
        DbLinkedHandle<EntityHandleDb, Entity> h;
        h.mHandle.mVal = ent->mHandle.mHandle.mVal;
        SV_GameSendServerCommand(h, va("print \"GAME_CHEATSNOTENABLED\""));
        return;
    }
    if (ent->health <= 0)
    {
        DbLinkedHandle<EntityHandleDb, Entity> h;
        h.mHandle.mVal = ent->mHandle.mHandle.mVal;
        SV_GameSendServerCommand(h, va("print \"GAME_MUSTBEALIVECOMMAND\""));
        return;
    }
    int amount = atoi(ConcatArgs(2));
    const char* name = ConcatArgs(1);
    if (name == nullptr || strlen(name) == 0)
        return;
    int take_all = 0;
    int v9 = 1;
    if (Q_stricmp(name, "all") == 0)
    {
        take_all = 1;
        goto take_health;
    }
    if (Q_stricmpn(name, "health", 6) == 0)
        goto take_health;
    if (Q_stricmp(name, "weapons") == 0)
        goto take_weapons;
    if (Q_stricmpn(name, "ammo", 4) == 0)
        goto take_ammo;
    goto take_allammo;

take_health:
    if (amount == 0 || (ent->health -= amount) < 1)
        ent->health = 1;
    if (take_all == 0)
        return;

take_weapons:
    for (v9 = 1; v9 <= BG_GetNumWeapons(); ++v9)
    {
        BG_TakePlayerWeapon(&ent->client->ps, v9);
        ent->client->ps.ammo[BG_AmmoForWeapon(v9)] = 0;
        ent->client->ps.ammoclip[BG_ClipForWeapon(v9)] = 0;
    }
    if (ent->client->ps.weapon != 0)
    {
        ent->client->ps.weapon = 0;
        BG_SelectWeaponIndex(0, ent->GetPlayerIndex());
    }
    if (take_all == 0)
        return;

take_ammo:
    if (amount != 0)
    {
        int weapon = ent->client->ps.weapon;
        if (weapon != 0)
        {
            ent->client->ps.ammo[BG_AmmoForWeapon(weapon)] -= amount;
            if (ent->client->ps.ammo[BG_AmmoForWeapon(weapon)] < 0)
            {
                ent->client->ps.ammoclip[BG_ClipForWeapon(weapon)]
                    += ent->client->ps.ammo[BG_AmmoForWeapon(weapon)];
                ent->client->ps.ammo[BG_AmmoForWeapon(weapon)] = 0;
                if (ent->client->ps.ammoclip[BG_ClipForWeapon(weapon)] < 0)
                ent->client->ps.ammoclip[BG_ClipForWeapon(weapon)] = 0;
            }
        }
    }
    else
    {
        for (int j = 1; j <= BG_GetNumWeapons(); ++j)
        {
            ent->client->ps.ammo[BG_AmmoForWeapon(j)] = 0;
            ent->client->ps.ammoclip[BG_ClipForWeapon(j)] = 0;
        }
    }
    if (take_all == 0)
        return;

take_allammo:
    if (Q_stricmpn(name, "allammo", 7) == 0 && amount != 0)
    {
        for (int i = 1; i <= BG_GetNumWeapons(); ++i)
        {
            ent->client->ps.ammo[BG_AmmoForWeapon(i)] -= amount;
            if (ent->client->ps.ammo[BG_AmmoForWeapon(i)] < 0)
            {
                ent->client->ps.ammoclip[BG_ClipForWeapon(i)]
                    += ent->client->ps.ammo[BG_AmmoForWeapon(i)];
                ent->client->ps.ammo[BG_AmmoForWeapon(i)] = 0;
                if (ent->client->ps.ammoclip[BG_ClipForWeapon(i)] < 0)
                ent->client->ps.ammoclip[BG_ClipForWeapon(i)] = 0;
            }
        }
    }
}

// ea: 0x004620F0
void misc_EntInfo(Entity* pSelf)
{
    float color[4] = {0.5f, 0.5f, 0.5f, 1.0f};
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 42;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float xyz[3];
    xyz[0] = (pSelf->r.absmax.v.m128_f32[0] + pSelf->r.absmin.v.m128_f32[0]) * 0.5f;
    xyz[1] = (pSelf->r.absmax.v.m128_f32[1] + pSelf->r.absmin.v.m128_f32[1]) * 0.5f;
    xyz[2] = (pSelf->r.absmax.v.m128_f32[2] + pSelf->r.absmin.v.m128_f32[2]) * 0.5f;
    float fInfoScale = g_entinfo_scale.value;
    Entity* v5 = EntityHandleDb::sInst.Find(640, hash_const.player);
    if (v5 != nullptr)
    {
        float* m128_f32 = v5->client->ps.origin.v.m128_f32;
        float v7 = m128_f32[1] - pSelf->r.currentOrigin.v.m128_f32[1];
        float v8 = m128_f32[2] - pSelf->r.currentOrigin.v.m128_f32[2];
        float v9 = m128_f32[0] - pSelf->r.currentOrigin.v.m128_f32[0];
        float fDist = sqrt(v8 * v8 + v7 * v7 + v9 * v9);
        if (g_entinfo_maxdist.value > 0.0f && fDist > g_entinfo_maxdist.value)
            return;
        fInfoScale = (fDist * g_entinfo_scale.value) * 0.0026041667f;
    }
    G_DebugBox(pSelf->r.absmin.v.m128_f32, pSelf->r.absmax.v.m128_f32,
               colorMagenta, 1, 0, 0);
    Broc::string::Block* mBlock = pSelf->mClassName.mBlock;
    if (mBlock == nullptr || (mBlock + 1) == nullptr
        || ((char*)&(mBlock + 1)->mBuff)[0] == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 64;
        AeAssert::gCurrentExpr = "pSelf->mClassName.IsDefined() && !pSelf->mClassName.is_empty()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    const char* v13;
    if (pSelf->targetname.mBlock == (Broc::string::Block*)-12)
    {
        v13 = "<noname>";
    }
    else
    {
        Broc::string::Block* v12 = pSelf->targetname.mBlock;
        v13 = v12 != nullptr ? (const char*)&v12[1] : &defaultFileName[0];
    }
    Broc::string::Block* v14 = pSelf->mClassName.mBlock;
    const char* v15 = (const char*)&v14[1];
    if (v14 == nullptr)
        v15 = &defaultFileName[0];
    const char* v16 = va("%i : %s : %i : %s", pSelf->mHandle.mHandle.mVal,
                         v13, pSelf->health, v15);
    CL_AddDebugString(xyz, color, fInfoScale * 0.75f, v16, 1);
}

// ea: 0x0044CF00
void G_InitObjectives()
{
    for (int i = 0; i < 17; ++i)
        SV_SetConfigstring(i + 16, nullptr);
}

// ea: 0x00458270
void UpdatePlayer()
{
    cdl_proftimer_ent_actors.start();
    for (int i = 0; i < 16; ++i)
    {
        Entity* v1 = EntityManager::sInst->mPlayers[i];
        Client* client = v1->client;
        if (client == nullptr || client->pers.connected != 2 /* CON_CONNECTED */
            || v1->r.linked == 0)
            continue;
        v1->r.mins.v.m128_f32[0] = playerMins.v.m128_f32[0];
        v1->r.mins.v.m128_f32[1] = playerMins.v.m128_f32[1];
        v1->r.mins.v.m128_f32[2] = playerMins.v.m128_f32[2];
        v1->r.maxs.v.m128_f32[0] = playerMaxs.v.m128_f32[0];
        v1->r.maxs.v.m128_f32[1] = playerMaxs.v.m128_f32[1];
        v1->r.maxs.v.m128_f32[2] = playerMaxs.v.m128_f32[2];
        int pm_flags = v1->client->ps.pm_flags;
        if ((pm_flags & 1) != 0)
            v1->r.maxs.v.m128_f32[2] = 30.0f;
        else if ((pm_flags & 2) != 0)
            v1->r.maxs.v.m128_f32[2] = 50.0f;
        v1->client->ps.mins[0] = v1->r.mins.v.m128_f32[0];
        v1->client->ps.mins[1] = v1->r.mins.v.m128_f32[1];
        v1->client->ps.mins[2] = v1->r.mins.v.m128_f32[2];
        v1->client->ps.maxs[0] = v1->r.maxs.v.m128_f32[0];
        v1->client->ps.maxs[1] = v1->r.maxs.v.m128_f32[1];
        v1->client->ps.maxs[2] = v1->r.maxs.v.m128_f32[2];
        bool v5 = v1->r.linked != 0;
        g_LinkEntity(v1);
        if (!v5)
            SV_UnlinkEntity(v1);
    }
    if (level.bounds_width != g_bounds_width.value
        || level.bounds_height_standing != g_bounds_height_standing.value)
    {
        level.bounds_width = g_bounds_width.value;
        level.bounds_height_standing = g_bounds_height_standing.value;
        playerMins.v.m128_f32[1] = g_bounds_width.value * -0.5f;
        playerMins.v.m128_f32[0] = g_bounds_width.value * -0.5f;
        playerMaxs.v.m128_f32[1] = g_bounds_width.value * 0.5f;
        playerMaxs.v.m128_f32[0] = g_bounds_width.value * 0.5f;
        playerMaxs.v.m128_f32[2] = g_bounds_height_standing.value;
        for (int j = 0; j < 16; ++j)
        {
            Entity* v7 = EntityManager::sInst->mPlayers[j];
            v7->r.mins.v.m128_f32[0] = playerMins.v.m128_f32[0];
            v7->r.mins.v.m128_f32[1] = playerMins.v.m128_f32[1];
            v7->r.mins.v.m128_f32[2] = playerMins.v.m128_f32[2];
            v7->r.maxs.v.m128_f32[0] = playerMaxs.v.m128_f32[0];
            v7->r.maxs.v.m128_f32[1] = playerMaxs.v.m128_f32[1];
            v7->r.maxs.v.m128_f32[2] = playerMaxs.v.m128_f32[2];
            int v8 = v7->client->ps.pm_flags;
            float v9;
            if ((v8 & 1) != 0)
                v9 = 30.0f;
            else if ((v8 & 2) != 0)
                v9 = 50.0f;
            else
                v9 = playerMaxs.v.m128_f32[2];
            v7->r.maxs.v.m128_f32[2] = v9;
            v7->client->ps.mins[0] = v7->r.mins.v.m128_f32[0];
            v7->client->ps.mins[1] = v7->r.mins.v.m128_f32[1];
            v7->client->ps.mins[2] = v7->r.mins.v.m128_f32[2];
            v7->client->ps.maxs[0] = v7->r.maxs.v.m128_f32[0];
            v7->client->ps.maxs[1] = v7->r.maxs.v.m128_f32[1];
            v7->client->ps.maxs[2] = v7->r.maxs.v.m128_f32[2];
            bool v10 = v7->r.linked != 0;
            SV_LinkEntity(v7);
            if (!v10)
                SV_UnlinkEntity(v7);
        }
    }
    if (level.viewheight_standing != bg_viewheight_standing.value
        || level.viewheight_crouched != bg_viewheight_crouched.value
        || level.viewheight_prone != bg_viewheight_prone.value)
    {
        level.viewheight_standing = bg_viewheight_standing.value;
        level.viewheight_crouched = bg_viewheight_crouched.value;
        level.viewheight_prone = bg_viewheight_prone.value;
        for (int k = 0; k < 16; ++k)
        {
            Entity* v12 = EntityManager::sInst->mPlayers[k];
            v12->client->ps.proneViewHeight = bg_viewheight_prone.integer;
            v12->client->ps.crouchViewHeight = bg_viewheight_crouched.integer;
            v12->client->ps.standViewHeight = bg_viewheight_standing.integer;
        }
    }
    cdl_proftimer_ent_actors.stop();
}

// ea: 0x0044B8C0
void G_RegisterCvars(void)
{
    for (int v0 = 0; v0 < gameCvarTableSize; ++v0)
    {
        CVarTable* i = &gameCvarTable[v0];
        Cvar_Register(i->vmCvar, i->cvarName, i->defaultString, i->cvarFlags);
        if (i->vmCvar != nullptr)
            i->modificationCount = i->vmCvar->modificationCount;
    }
    Cvar_VMSet((vmCvar_t*)&cg_deadscreen_backdrop, defaultFileName);
    Cvar_VMSet((vmCvar_t*)&cg_deadscreen_levelname, defaultFileName);
    Cvar_VMSet((vmCvar_t*)&cg_victoryscreen_backdrop, defaultFileName);
    Cvar_VMSet((vmCvar_t*)&cg_victoryscreen_levelname, defaultFileName);
}

// ea: 0x00467040
Entity* SelectSpawnPoint(const float* avoidPoint, float* origin, float* angles)
{
    Entity* v3 = SelectNearestDeathmatchSpawnPoint(avoidPoint);
    Entity* v4 = SelectRandomDeathmatchSpawnPoint();
    if (v4 == v3)
    {
        v4 = SelectRandomDeathmatchSpawnPoint();
        if (v4 == v3)
            v4 = SelectRandomDeathmatchSpawnPoint();
    }
    if (v4 == nullptr)
        G_Error("Couldn't find a spawn point");
    origin[0] = v4->r.currentOrigin.v.m128_f32[0];
    origin[1] = v4->r.currentOrigin.v.m128_f32[1];
    origin[2] = v4->r.currentOrigin.v.m128_f32[2] + 9.0f;
    angles[0] = v4->r.currentAngles.v.m128_f32[0];
    angles[1] = v4->r.currentAngles.v.m128_f32[1];
    angles[2] = v4->r.currentAngles.v.m128_f32[2];
    return v4;
}

// ea: 0x0044B950
void InitCvars(int restart)
{
    int integer = 0;
    int v2 = 0;
    if (restart != 0)
    {
        integer = g_gameskill->integer;
        v2 = Cvar_Get("g_player_maxhealth", "1", 0)->integer;
    }
    G_RegisterCvars();
    if (restart != 0)
    {
        Cvar_SetValue("g_gameskill", (float)integer);
        g_gameskill->integer = integer;
        Cvar_SetValue("g_player_maxhealth", (float)v2);
        g_player_maxhealth.integer = v2;
        g_player_maxhealth.value = (float)v2;
    }
    cl_aADS[0] = 1;
    cg_aWeaponSelect[0] = 0;
    cg_aWeaponSelectTime[0] = 0;
    cl_stance_ss[0] = 0;
}

// ea: 0x0044A9C0
void Cmd_UFO_f(Entity* ent)
{
    if (g_cheats->integer != 0)
    {
        if (ent->health > 0)
        {
            Client* client = ent->client;
            int ufo = client->ufo;
            const char* v7 = "GAME_UFOOFF";
            if (ufo == 0)
                v7 = "GAME_UFOON";
            client->ufo = ufo == 0;
            SV_GameSendServerCommand(ent->mHandle, va("print \"%s\"", v7));
        }
        else
        {
            SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_MUSTBEALIVECOMMAND\""));
        }
    }
    else
    {
        SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_CHEATSNOTENABLED\""));
    }
}

// ea: 0x00467570
void ClientBegin(DbLinkedHandle<EntityHandleDb, Entity> entity)
{
    Entity* mObject = HandleDbToEnt(entity);
    float* origin = mObject->client->ps.origin.v.m128_f32;
    Entity* v4 = HandleDbToEnt(entity);
    origin[0] = v4->r.currentOrigin.v.m128_f32[0];
    origin[1] = v4->r.currentOrigin.v.m128_f32[1];
    origin[2] = v4->r.currentOrigin.v.m128_f32[2];
    origin[3] = v4->r.currentOrigin.v.m128_f32[3];
    origin[52] = v4->r.currentAngles.v.m128_f32[0];
    origin[53] = v4->r.currentAngles.v.m128_f32[1];
    origin[54] = v4->r.currentAngles.v.m128_f32[2];
    origin[9] = 5;
}

// ea: 0x00449F60
void ChangePlayersMaxHealth(int newMaxHealth)
{
    int v1 = newMaxHealth;
    if (newMaxHealth < 10)
        v1 = 10;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    Cvar_Set("g_player_maxhealth", va("%i", v1));
    Player->client->pers.maxHealth = (int)(g_player_maxhealth.value + 0.5f);
    Player->client->ps.stats[2] = Player->client->pers.maxHealth;
    Player->client->ps.stats[0] = Player->client->ps.stats[2];
    Player->health = Player->client->ps.stats[0];
}

// ea: 0x004492B0
void G_ReduceAnglesError(float* angles, float* anglesError, float frametime,
                         float angleLerpRate)
{
    float change = frametime * angleLerpRate;
    for (int i = 3; i != 0; --i)
    {
        float v9 = *anglesError;
        if (*anglesError != 0.0f)
        {
            if (v9 > change)
            {
                *anglesError = v9 - change;
                *angles = AngleNormalize360Accurate(*angles + (v9 - change));
            }
            else if (-change > *anglesError)
            {
                float v10 = change + *anglesError;
                *anglesError = v10;
                *angles = AngleNormalize360Accurate(*angles + v10);
            }
            else
            {
                *anglesError = 0.0f;
            }
        }
        ++angles;
        ++anglesError;
    }
}

// ea: 0x00458120
void G_CheckLoadGame(int savegame)
{
    Cvar_Set("g_reloading", "1");
    if (savegame != 0)
    {
        PathNodeMgr::sInst->ValidateAllNodes();
        EntityManager::sInst->DeleteAllEntities();
        SceneManager_ResetAllStaticModels();
        Entity::FreeAllDObjs(true);
        G_CleanupAnimTrees();
        HudElem_DestroyAll();
        j_nullsub_93();
        Entity::FreeAllDObjs(true);
        G_CleanupAnimTrees();
    }
    SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(), "snd_fade 1 0");
    SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(), "scr_fade 0 0 0");
    Cvar_Set("g_reloading", "0");
}

// ea: 0x00456080
void Cmd_LockPVS_f(void)
{
    char s[256];
    gLockMeshList ^= 1u;
    PakManager::sInst->mEnabled ^= 1u;
    if (Cmd_Argc() == 2)
    {
        Cmd_ArgvBuffer(1, s, 256);
        if (Q_stricmp(s, "flash") == 0)
            gEnableMeshFlash = true;
    }
    if (gLockMeshList)
    {
        if (gEnableMeshFlash)
            SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(),
                                     "print \"PVS locked. (Flash is on)\"");
        else
            SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(),
                                     "print \"PVS locked\"");
    }
    else
    {
        SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(),
                                 "print \"PVS unlocked\"");
    }
}

// ea: 0x00461CB0
void G_DebugCircle(const float* center, float radius, const float* color,
                   int depthTest, int onGround, int duration)
{
    float dir[3];
    if (onGround != 0)
    {
        dir[0] = 0.0f;
        dir[1] = 0.0f;
        dir[2] = 1.0f;
    }
    else
    {
        float v6 = level.clients->ps.viewHeightCurrent + level.clients->ps.origin.v.m128_f32[2];
        dir[0] = center[0] - level.clients->ps.origin.v.m128_f32[0];
        dir[1] = center[1] - level.clients->ps.origin.v.m128_f32[1];
        dir[2] = center[2] - v6;
    }
    G_DebugCircleEx(center, radius, dir, color, depthTest, duration);
}

// ea: 0x00468C60
void DebugDumpAnims(void)
{
    if (g_dumpAnims > 0)
    {
        Entity* mObject = HandleDbToEnt(
            *(DbLinkedHandle<EntityHandleDb, Entity>*)&g_dumpAnims);
        if (mObject != nullptr)
            SV_DObjDisplayAnim(mObject);
    }
    if (MultiplayerMgr::sInst != nullptr
        && MultiplayerMgr::sInst->mPeer != nullptr
        && MultiplayerMgr::sInst->mPeer->GetPlayerManager() != nullptr)
    {
        MPPlayerManager* PlayerManager = MultiplayerMgr::sInst->mPeer->GetPlayerManager();
        if (cg_mpDebugAnimEntity < 0x10u)
        {
            MPPlayer* Player = PlayerManager->GetPlayer(cg_mpDebugAnimEntity);
            if (Player != nullptr)
                AnimationPlayer_DebugDump(Player->GetEntity());
        }
    }
}

// ea: 0x0044A7A0
void Cmd_God_f(Entity* ent)
{
    if (g_cheats->integer != 0)
    {
        if (ent->health > 0)
        {
            ent->flags ^= 1u;
            const char* v5;
            if ((ent->flags & 1) != 0)
            {
                v5 = "GAME_GODMODEON";
                gGodModeEnabled = true;
            }
            else
            {
                v5 = "GAME_GODMODEOFF";
                gGodModeEnabled = false;
            }
            SV_GameSendServerCommand(ent->mHandle, va("print \"%s\"", v5));
        }
        else
        {
            SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_MUSTBEALIVECOMMAND\""));
        }
    }
    else
    {
        SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_CHEATSNOTENABLED\""));
    }
}

// ea: 0x0044A850
void Cmd_Notarget_f(Entity* ent)
{
    if (g_cheats->integer != 0)
    {
        if (ent->health > 0)
        {
            char v5 = (char)(ent->flags ^ 2);
            ent->flags ^= 2u;
            const char* v6;
            if ((v5 & 2) != 0)
            {
                v6 = "GAME_NOTARGETON";
                gNoTargetEnabled = true;
            }
            else
            {
                v6 = "GAME_NOTARGETOFF";
                gNoTargetEnabled = false;
            }
            SV_GameSendServerCommand(ent->mHandle, va("print \"%s\"", v6));
        }
        else
        {
            SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_MUSTBEALIVECOMMAND\""));
        }
    }
    else
    {
        SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_CHEATSNOTENABLED\""));
    }
}

// ea: 0x00455F60
void Cmd_GiveAll_f(Entity* ent)
{
    ent->client->ps.stats[2] = g_player_maxhealth.integer;
    ent->health = g_player_maxhealth.integer;
    level.initializing = 1;
    for (int i = 1; i <= BG_GetNumWeapons(); ++i)
        BG_GivePlayerWeapon(&ent->client->ps, i);
    level.initializing = 0;
    for (int j = 1; j <= BG_GetNumWeapons(); ++j)
    {
        if (Com_BitCheck(ent->client->ps.weapons, j) != 0)
            Add_Ammo(ent, j, 998, 1);
    }
}

// ea: 0x004570C0
void G_DebugBox(const float* mins, const float* maxs, const float* color,
                int depthTest, int duration, int fade)
{
    float v[8][3];
    for (int i = 0; i < 8; ++i)
    {
        v[i][0] = (i & 1) != 0 ? maxs[0] : mins[0];
        v[i][1] = (i & 2) != 0 ? maxs[1] : mins[1];
        v[i][2] = (i & 4) != 0 ? maxs[2] : mins[2];
    }
    static const int edges[12][2] = {
        {0,1},{1,3},{3,2},{2,0},
        {4,5},{5,7},{7,6},{6,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    for (int i = 0; i < 12; ++i)
        CL_AddDebugLine(v[edges[i][0]], v[edges[i][1]], color, depthTest, duration, 1, fade);
}

// ea: 0x00461B10
void G_DebugBox(float* pos, float width, float r, float g, float b,
                int duration, int fade)
{
    float color[4] = { r, g, b, 1.0f };
    float mins[3];
    float maxs[3];
    mins[0] = pos[0] + width * 0.5f;
    mins[1] = pos[1] + width * 0.5f;
    mins[2] = pos[2] + width * 0.5f;
    maxs[0] = pos[0] - width * 0.5f;
    maxs[1] = pos[1] - width * 0.5f;
    maxs[2] = pos[2] - width * 0.5f;
    G_DebugBox(mins, maxs, color, 1, duration, fade);
}

// ea: 0x0044A900
void Cmd_Noclip_f(Entity* ent)
{
    if (g_cheats->integer)
    {
        if (ent->health > 0)
        {
            const char* v5;
            if (ent->client->noclip)
            {
                v5 = "GAME_NOCLIPOFF";
                gNoClipEnabled = 0;
            }
            else
            {
                v5 = "GAME_NOCLIPON";
                gNoClipEnabled = 1;
            }
            ent->client->noclip = ent->client->noclip == 0;
            SV_GameSendServerCommand(ent->mHandle, va("print \"%s\"", v5));
        }
        else
        {
            SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_MUSTBEALIVECOMMAND\""));
        }
    }
    else
    {
        SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_CHEATSNOTENABLED\""));
    }
}

// ea: 0x00465650
void Svcmd_ListEntities_f(void)
{
    int counter[18] = { 0 };
    Entity** begin = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity** end = begin + EntityHandleDb::sInst.mActiveList.m_size;
    for (Entity** p = begin; p != end; ++p)
    {
        if (*p != nullptr)
            ++counter[(*p)->s.eType];
    }
    int v2 = 0;
    for (int i = 0; i < 18; ++i)
    {
        G_Printf("%s %d\n", entityTypeNames[i], counter[i]);
        v2 += counter[i];
    }
    G_Printf("total %d of %d\n", v2, 1344);
}

// ea: 0x00470780
int ConsoleCommand(void)
{
    char cmd[128];
    Cmd_ArgvBuffer(0, cmd, 128);
    if (Q_stricmp(cmd, "entitylist") != 0)
    {
        if (Q_stricmp(cmd, "listvehicles") != 0)
        {
            if (Q_stricmp(cmd, "listnodes") != 0)
            {
                if (Q_stricmp(cmd, "listentities") != 0)
                    return 0;
                Svcmd_ListEntities_f();
                return 1;
            }
            PathNodeMgr::sInst->NodeList();
            return 1;
        }
        Svcmd_VehicleList_f();
        return 1;
    }
    Svcmd_EntityList_f();
    return 1;
}

// ea: 0x00482B50
void G_GeneralLink(Entity* ent)
{
    if (ent->tagInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 1365;
        AeAssert::gCurrentExpr = "ent->tagInfo";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    G_SetFixedLink(ent, 0);
    G_SetOrigin(ent, &ent->r.currentOrigin);
    G_SetAngle(ent, &ent->r.currentAngles);
    memcpy(ent->s.pos.trDelta, &ent->r.currentOrigin, sizeof(ent->s.pos.trDelta));
    memcpy(ent->s.apos.trDelta, &ent->r.currentAngles, sizeof(ent->s.apos.trDelta));
    ent->s.pos.trType = TR_INTERPOLATE;
    ent->s.apos.trType = TR_INTERPOLATE;
    g_LinkEntity(ent);
}

// ea: 0x0048EB30
void ClientThink(DbLinkedHandle<EntityHandleDb, Entity> entityHandle)
{
    Entity* mObject = HandleDbToEnt(entityHandle);
    mObject->client->pers.oldcmd = mObject->client->pers.cmd;
    SV_GetUsercmd(currCl, &mObject->client->pers.cmd);
    mObject->client->lastCmdTime = level.time;
    ClientThink_real(mObject);
    if (mObject->client->ps.leanf == 0.0f)
        G_RemoveHeadHitEnt(mObject);
    else
        G_UpdateHeadHitEnt(mObject);
}

// ea: 0x00455E80
void G_setfog(const char* fogstring)
{
    SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(), va("fog %s", fogstring));
    level.fFogOpaqueDist = 3.4028235e38f;
    level.fFogOpaqueDistSqrd = 3.4028235e38f;
    float fNear, fFar, fDensity;
    int clr, v3, v4, time;
    if (sscanf(fogstring, "%f %f %f %d %d %d %d", &fNear, &fFar, &fDensity,
               &clr, &v3, &v4, &time) == 7
        && fDensity >= 1.0f)
    {
        level.fFogOpaqueDist = ((fFar - fNear) * 0.82800001f) + fNear;
        level.fFogOpaqueDistSqrd = level.fFogOpaqueDist * level.fFogOpaqueDist;
    }
}

// ea: 0x00467610
void ClientDisconnect(DbLinkedHandle<EntityHandleDb, Entity> entity)
{
    Entity* mObject = HandleDbToEnt(entity);
    Client* client = mObject->client;
    Entity* v4 = HandleDbToEnt(entity);
    if (v4 == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 1230;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    v4->r.svFlags = 0;
    StopPhysics(v4);
    Sentient_Free(v4->sentient);
    v4->sentient = nullptr;
    SV_UnlinkEntity(v4);
    client->pers.connected = CON_DISCONNECTED;
}

// ea: 0x00485F20
void UpdateLinkedEntities(const ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 1000>* linkedEntities)
{
    for (int i = 0; i < linkedEntities->m_size; ++i)
    {
        Entity* mObject = HandleDbToEnt(linkedEntities->m_elements[i]);
        if (mObject != nullptr && mObject->client == nullptr && mObject->tagInfo != nullptr)
            G_GeneralLink(mObject);
    }
}

// ea: 0x004640D0
void Svcmd_EntityList_f(void)
{
    Entity** begin = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity** end = begin + EntityHandleDb::sInst.mActiveList.m_size;
    for (Entity** p = begin; p != end; ++p)
    {
        Entity* v2 = *p;
        if (v2 != nullptr && !v2->IsLocalPlayer())
        {
            G_Printf("%3i: ", v2->mHandle.mHandle.mVal);
            const char* v3 = v2->s.eType >= 0x12u
                                 ? "WARNING !! Entity Type Unknown WARNING !!!"
                                 : entityTypeNames[v2->s.eType];
            G_Printf("'%s'", v3);
            if (v2->mClassName.mBlock != nullptr && v2->mClassName.c_str() != nullptr
                && v2->mClassName.c_str()[0] != 0)
            {
                G_Printf(", '%s'", v2->mClassName.c_str());
            }
            G_Printf("\n");
        }
    }
}

// ea: 0x0048F270
void UpdateEntities(ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 1000>* linkedEntities,
                    int msec)
{
    Entity** begin = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity** end = begin + EntityHandleDb::sInst.mActiveList.m_size;
    for (Entity** p = begin; p != end; ++p)
    {
        Entity* v4 = *p;
        if (v4 != nullptr)
        {
            if (v4->tagInfo != nullptr)
            {
                if (linkedEntities->m_size == 1000)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)AeAssert::JRS;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
                    AeAssert::gCurrentLine = 3463;
                    AeAssert::gCurrentExpr = nullptr;
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Warning("Exceeded max linked entities %d\n", 1000))
                        __debugbreak();
                    G_GeneralLink(v4);
                }
                else
                {
                    linkedEntities->m_elements[linkedEntities->m_size++] = v4->mHandle;
                }
            }
            G_RunFrameForEntity(v4, msec);
        }
    }
}

// ea: 0x00461BD0
void G_DebugSphere(const float* center, float radius, const float* color,
                   int density, int depthTest, int duration)
{
    if (density > 0)
    {
        float step = 6.2831855f / density;
        for (int v7 = 0; v7 < density; ++v7)
        {
            float radians = v7 * step;
            float dir[3];
            dir[1] = sinf(radians);
            dir[0] = cosf(radians);
            float v12[3] = { sinf(radians), cosf(radians), 0.0f };
            G_DebugCircleEx(center, radius, v12, color, depthTest, duration);
        }
    }
}

// ea: 0x004677E0
void UpdateShotProf(float deltaT)
{
    if (gShotProf != nullptr
        && ShaderCommon::UpdateShotPerfTest((ShaderCommon::ShotPerfTest*)gShotProf, deltaT))
    {
        ae_sized_array<ae_fixed_string<512, unsigned short>, 64> results;
        ShaderCommon::GetShotPerfResults((ShaderCommon::ShotPerfTest*)gShotProf, &results);
        ShaderCommon::FinishShotPerfTest((ShaderCommon::ShotPerfTest*)gShotProf);
        for (int i = 0; i < results.m_size; ++i)
            tlPrintf("%s\n", results.m_elements[i].c_str());
        gShotProf = nullptr;
        gRenderCG_2D = 1;
        Cvar_Set("statusbar", "1");
        Cvar_Set("timerbars_on", "1");
        nglDebug.ShowPerfInfo = nglDebug.ShowPerfInfo != 1;
    }
}

// ea: 0x004491D0
void G_ReduceOriginError(float* origin, float* originError, float frametime)
{
    float error = ((*originError * *originError) + (originError[1] * originError[1]))
                + (originError[2] * originError[2]);
    if (error != 0.0f)
    {
        float errora = 1.0f - frametime * 300.0f / sqrtf(error);
        if (errora <= 0.0f)
        {
            originError[0] = 0.0f;
            originError[1] = 0.0f;
            originError[2] = 0.0f;
        }
        else
        {
            float v4 = *originError * errora;
            float v5 = errora * originError[1];
            originError[2] = errora * originError[2];
            originError[1] = v5;
            *originError = v4;
            *origin += v4;
            origin[1] += originError[1];
            origin[2] += originError[2];
        }
    }
}

// ea: 0x00456FE0
void G_DebugAxis(const math::Mat43* mat, unsigned int length, int duration)
{
    float start[4] = { mat->w.v.m128_f32[0], mat->w.v.m128_f32[1],
                       mat->w.v.m128_f32[2], mat->w.v.m128_f32[3] };
    float len = (float)length;
    float end[3];
    end[0] = start[0] + mat->x.v.m128_f32[0] * len;
    end[1] = start[1] + mat->x.v.m128_f32[1] * len;
    end[2] = start[2] + mat->x.v.m128_f32[2] * len;
    CL_AddDebugLine(start, end, colorRed, 1, duration, 1, 0);
    end[0] = start[0] + mat->y.v.m128_f32[0] * len;
    end[1] = start[1] + mat->y.v.m128_f32[1] * len;
    end[2] = start[2] + mat->y.v.m128_f32[2] * len;
    CL_AddDebugLine(start, end, colorGreen, 1, duration, 1, 0);
    end[0] = start[0] + mat->z.v.m128_f32[0] * len;
    end[1] = start[1] + mat->z.v.m128_f32[1] * len;
    end[2] = start[2] + mat->z.v.m128_f32[2] * len;
    CL_AddDebugLine(start, end, colorBlue, 1, duration, 1, 0);
}

// ea: 0x00483480
void G_LinkClient(Entity* ent)
{
    if (ent->client->noclip == 0 && IsPlayerFullySeatedInVehicle(ent))
    {
        if (ent->tagInfo != nullptr && ent->client != nullptr)
        {
            Client* v1 = ent->client;
            int playerState = v1->pers.playerState;
            int v4;
            if (playerState == 4 || playerState == 5)
                v4 = 7;
            else
                v4 = 1;
            v1->ps.pm_type = v4;
            G_SetFixedLink(ent, 0);
            G_SetOrigin(ent, &ent->r.currentOrigin);
            ent->s.pos.trType = TR_INTERPOLATE;
            ent->s.apos.trType = TR_INTERPOLATE;
            g_LinkEntity(ent);
            ent->client->ps.origin.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
            ent->client->ps.origin.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
            ent->client->ps.origin.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2];
        }
        else
        {
            Client* client = ent->client;
            int pm_type = client->ps.pm_type;
            if (pm_type == 1 || pm_type == 7)
                --client->ps.pm_type;
        }
    }
}

// ea: 0x0047BB40
void G_SetAnimTree(Entity* ent, AnimTree* animtree)
{
    XAnimTree* ActorAnimTree;
    if (ent->s.eType == 11)
        ActorAnimTree = G_GetActorAnimTree(ent->actor);
    else if (ent->s.eType == 13)
        ActorAnimTree = G_GetActorCorpseAnimTree(ent);
    else
        ActorAnimTree = ent->pAnimTree;
    if (ActorAnimTree != ent->pAnimTree)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_main.cpp";
        AeAssert::gCurrentLine = 708;
        AeAssert::gCurrentExpr = "G_GetEntAnimTree(ent) == ent->pAnimTree";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    XAnimTree* pAnimTree = ent->pAnimTree;
    if (animtree == nullptr)
    {
        if (pAnimTree == nullptr)
            return;
        ent->pAnimTree = nullptr;
        G_DObjUpdate(ent, false);
        G_DelayFreeAnimTree(pAnimTree);
        return;
    }
    if (pAnimTree == nullptr || (AnimTree*)XAnimGetAnims(ent->pAnimTree) != animtree)
    {
        ent->pAnimTree = (XAnimTree*)XAnimCreateTree(ent, animtree);
        G_DObjUpdate(ent, false);
        if (pAnimTree != nullptr)
            G_DelayFreeAnimTree(pAnimTree);
    }
}

// ea: 0x0046DF60
void G_VehicleClientThink(int msec)
{
    s_clientThink = 1;
    if (level.MaxVehicles != 0)
    {
        for (int i = 0; i < level.MaxVehicles; ++i)
        {
            Entity* mObject = HandleDbToEnt(s_vehicles[i].mEntity);
            if (mObject != nullptr && mObject->nextthink <= level.time)
                G_RunThink(mObject, msec);
        }
    }
    s_clientThink = 0;
}

// ea: 0x004507D0
bool ValidForGametype(void)
{
    static unsigned char s_init = 0;
    static unsigned int gametypes_hash = 0;
    if (!(s_init & 1))
    {
        s_init |= 1;
        gametypes_hash = HashString::CalcHash("gametypes");
    }
    const char* gametypes = nullptr;
    G_SpawnString(gametypes_hash, defaultFileName, &gametypes);
    if (*gametypes != 0)
    {
        char temp[64];
        strcpy(temp, gametypes);
        const char* v3 = strtok(temp, " ");
        if (v3 == nullptr)
            return 0;
        while (_stricmp(v3, mp_gametype.string) != 0)
        {
            v3 = strtok(nullptr, " ");
            if (v3 == nullptr)
                return 0;
        }
    }
    return 1;
}

// ea: 0x0046A290
void render_aabb(const math::Position3* bmin, const math::Position3* bmax,
                 const float* color)
{
    if (render)
    {
        debug_aabb v5;
        v5.bmin.v = bmin->v;
        v5.bmax.v = bmax->v;
        v5.color[0] = color[0];
        v5.color[1] = color[1];
        v5.color[2] = color[2];
        v5.color[3] = color[3];
        debug_aabbs.mElements[debug_aabbs.mSize++] = v5;
    }
}

// ea: 0x0047C110
void debug_render()
{
    for (int v2 = 0; v2 < debug_spheres.mSize; ++v2)
    {
        math::Position3 pos;
        pos.v.m128_f32[0] = debug_spheres.mElements[v2].x;
        pos.v.m128_f32[1] = debug_spheres.mElements[v2].y;
        pos.v.m128_f32[2] = debug_spheres.mElements[v2].z;
        pos.v.m128_f32[3] = 0.0f;
        DebugRender::RenderSphere(&pos, debug_spheres.mElements[v2].radius,
                                  debug_spheres.mElements[v2].color);
    }
    debug_spheres.mSize = 0;
    for (int v6 = 0; v6 < debug_aabbs.mSize; ++v6)
    {
        DebugRender::RenderBox(&debug_aabbs.mElements[v6].bmin,
                               &debug_aabbs.mElements[v6].bmax,
                               debug_aabbs.mElements[v6].color);
    }
    debug_aabbs.mSize = 0;
}

// ea: 0x00462A60
void Concussive_think(Entity* ent, int /*msec*/)
{
    if (level.time > ent->delay)
        ent->think = THINK__G_FreeEntity;
    ent->nextthink = level.time + 100;
    Entity* v1 = EntityHandleDb::sInst.Find(640, hash_const.player);
    if (v1 != nullptr)
    {
        float dx = v1->r.currentOrigin.v.m128_f32[0] - ent->r.currentOrigin.v.m128_f32[0];
        float dy = v1->r.currentOrigin.v.m128_f32[1] - ent->r.currentOrigin.v.m128_f32[1];
        float dz = v1->r.currentOrigin.v.m128_f32[2] - ent->r.currentOrigin.v.m128_f32[2];
        if (sqrtf((dx * dx) + (dy * dy) + (dz * dz)) <= 512.0f)
        {
            v1->client->ps.velocity.v.m128_f32[2] += 24.0f;
            Client* client = v1->client;
            if (client->ps.pm_time == 0)
            {
                client->ps.pm_time = 50;
                v1->client->ps.pm_flags |= 0x200u;
            }
        }
    }
}

// ea: 0x00466F40
Entity* SelectRandomDeathmatchSpawnPoint(void)
{
    ae_sized_array<Entity*, 4096> entList;
    ae_sized_array<Entity*, 128> spotList;
    entList.m_size = 0;
    spotList.m_size = 0;
    EntityHandleDb_Find<HashString>(640, hash_const.info_player_deathmatch, entList);
    for (int i = 0; i < entList.m_size; ++i)
    {
        Entity* spot = entList.m_elements[i];
        if (!SpotWouldTelefrag(&spot->r.currentOrigin))
            spotList.m_elements[spotList.m_size++] = spot;
    }
    if (spotList.m_size == 0)
        return entList.m_elements[0];
    unsigned int v4 = rand() % spotList.m_size;
    return spotList.m_elements[v4];
}

// ea: 0x00461830
Entity* SelectNearestDeathmatchSpawnPoint(const float* from)
{
    float v1 = 999999.0f;
    Entity* nearestSpot = nullptr;
    Entity** begin = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity** end = begin + EntityHandleDb::sInst.mActiveList.m_size;
    for (Entity** p = begin; p != end; ++p)
    {
        Entity* spot = *p;
        if (spot == nullptr)
            continue;
        if (spot->mClassNameHash.mHash == 0
            || spot->mClassNameHash.mHash != hash_const.info_player_deathmatch.mHash)
            continue;
        float dx = spot->r.currentOrigin.v.m128_f32[0] - from[0];
        float dy = spot->r.currentOrigin.v.m128_f32[1] - from[1];
        float dz = spot->r.currentOrigin.v.m128_f32[2] - from[2];
        float dist = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
        if (v1 > dist)
        {
            v1 = dist;
            nearestSpot = spot;
        }
    }
    return nearestSpot;
}

// ea: 0x004676E0
void Cmd_MenuResponse_f(Entity* pEnt)
{
    char szResponse[128];
    char szMenuName[128];
    if (Cmd_Argc() == 3)
    {
        Cmd_ArgvBuffer(1, szMenuName, 128);
        if (atoi(szMenuName) == 0)
            SV_GetConfigstring(627, szMenuName, 128);
        Cmd_ArgvBuffer(2, szResponse, 128);
    }
    else
    {
        szMenuName[0] = 0;
        szResponse[0] = 0;
    }
    Scr_Notify(pEnt, hash_const.menuresponse, 0);
}

// ea: 0x00456400
void LookAtKiller(Entity* self, Entity* inflictor, Entity* attacker)
{
    float dir[3];
    if (attacker != nullptr && attacker != self)
    {
        dir[0] = attacker->r.currentOrigin.v.m128_f32[0] - self->r.currentOrigin.v.m128_f32[0];
        dir[1] = attacker->r.currentOrigin.v.m128_f32[1] - self->r.currentOrigin.v.m128_f32[1];
        dir[2] = attacker->r.currentOrigin.v.m128_f32[2] - self->r.currentOrigin.v.m128_f32[2];
        self->client->ps.mKiller.mHandle.mVal = attacker->mHandle.mHandle.mVal;
        self->client->ps.stats[1] = (int)vectoyaw(dir);
        return;
    }
    if (inflictor != nullptr && inflictor != self)
    {
        dir[0] = inflictor->r.currentOrigin.v.m128_f32[0] - self->r.currentOrigin.v.m128_f32[0];
        dir[1] = inflictor->r.currentOrigin.v.m128_f32[1] - self->r.currentOrigin.v.m128_f32[1];
        dir[2] = inflictor->r.currentOrigin.v.m128_f32[2] - self->r.currentOrigin.v.m128_f32[2];
        self->client->ps.mKiller.mHandle.mVal = inflictor->mHandle.mHandle.mVal;
        self->client->ps.stats[1] = (int)vectoyaw(dir);
        return;
    }
    self->client->ps.stats[1] = (int)self->r.currentAngles.v.m128_f32[1];
    self->client->ps.mKiller.mHandle.mVal = 0;
}

// ea: 0x004670D0
Entity* SelectInitialSpawnPoint(float* origin, float* angles)
{
    Entity* v2 = nullptr;
    Entity** begin = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity** end = begin + EntityHandleDb::sInst.mActiveList.m_size;
    for (Entity** p = begin; p != end; ++p)
    {
        Entity* spot = *p;
        if (spot == nullptr)
            continue;
        if (spot->mClassNameHash.mHash == 0
            || spot->mClassNameHash.mHash != hash_const.info_player_deathmatch.mHash)
            continue;
        v2 = spot;
        if ((v2->spawnflags & 1) != 0)
            break;
    }
    if (v2 == nullptr || SpotWouldTelefrag(&v2->r.currentOrigin))
        return SelectSpawnPoint(vec3_origin, origin, angles);
    origin[0] = v2->r.currentOrigin.v.m128_f32[0];
    origin[1] = v2->r.currentOrigin.v.m128_f32[1];
    origin[2] = v2->r.currentOrigin.v.m128_f32[2] + 9.0f;
    angles[0] = v2->r.currentAngles.v.m128_f32[0];
    angles[1] = v2->r.currentAngles.v.m128_f32[1];
    angles[2] = v2->r.currentAngles.v.m128_f32[2];
    return v2;
}

// ea: 0x00467ED0
void ShowEntityInfo(void)
{
    cvar_t* result = Cvar_Get("g_entinfo", "0", 512);
    if (result->integer != 0)
    {
        for (int idx = 0; idx < 0x540; ++idx)
        {
            Entity* Object = EntityHandleDb::sInst.mElements[idx].mObject;
            if (Object == nullptr || Object->r.linked == 0)
                continue;
            uint8_t entinfo = Object->entinfo;
            if (entinfo != 0)
            {
                if (entinfo >= 3u)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
                    AeAssert::gCurrentLine = 2362;
                    AeAssert::gCurrentExpr = "ent->entinfo > 0 && ent->entinfo < ENTINFO_MAX";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                entinfotable[Object->entinfo](Object);
            }
        }
    }
}

// ea: 0x00460C50
void DebugDumpEnts(int /*a1*/, Entity* e)
{
    if (e->actor != nullptr)
        MemPrint("actor %s\n", e->mClassName.c_str());
    else if (e->item != nullptr)
        MemPrint("item %s\n", e->mClassName.c_str());
    else if (e->scr_vehicle != nullptr)
    {
        if (e->mDObj != nullptr)
            MemPrint("vehicle %s\n",
                     ((XModel*)e->mDObj->models[0].mValue)->name.mStr);
    }
    else if (e->mDObj != nullptr)
    {
        MemPrint("dobj %s\n", ((XModel*)e->mDObj->models[0].mValue)->name.mStr);
    }
    else
    {
        MemPrint("unknown %s\n", e->mClassName.c_str());
    }
}

// ea: 0x00468D00
void G_LoadLevel(void)
{
    level.initializing = 1;
    level.loading = 0;
    Client* client = EntityManager::sInst->GetPlayer(currCl)->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
        AeAssert::gCurrentLine = 2897;
        AeAssert::gCurrentExpr = "GetPlayer()->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ValidatePakId((TPakId)client->ps.viewmodel.mPakId);
    if (client->ps.viewmodel.mValue == nullptr)
    {
        Com_Printf("WARNING: no viewmodel specified for player, using default '%s'\n",
                   "xmodel/viewmodel_hands_us");
        client->ps.viewmodel = XModelManager::sInst->GetXModel(CurPakId(),
                                                               "xmodel/viewmodel_hands_us");
    }
    level.initializing = 0;
    level.snapTime = level.time;
    if (level.bRegisterItems != 0)
        SaveRegisteredItems();
}

// ea: 0x004678C0
void ClientCommand(DbLinkedHandle<EntityHandleDb, Entity> ent)
{
    Entity* mObject = HandleDbToEnt(ent);
    if (mObject == nullptr || mObject->client == nullptr)
        return;
    char cmd[1022];
    Cmd_ArgvBuffer(0, cmd, 1022);
    for (unsigned int v1 = 0; v1 < 24; ++v1)
    {
        if (ae_stricmpn(cmd, sClientCommand0List[v1].first, 0xFFFFFFF) == 0)
            return;  // dispatch table entry (commands w/o entity)
    }
    for (unsigned int i = 0; i < 15; ++i)
    {
        if (ae_stricmpn(cmd, sClientCommand1List[i].first, 0xFFFFFFF) == 0)
        {
            // dispatch with entity - routed through command handlers below
            return;
        }
    }
    SV_GameSendServerCommand(mObject->mHandle, "unrecognized command");
}

// ea: 0x00457FE0
void G_ShutdownGame(int restart)
{
    G_DPrintf("ShutdownGame:\n");
    G_DPrintf("------------------------------------------------------------\n");
    DynamicDecalMgr_DestroyAllDecals();
    SmokeGrenadeMgr_ReInitialize();
    CG_FreeWeapons();
    PathNodeMgr::sInst->ValidateAllNodes();
    EntityManager::sInst->DeleteAllEntities();
    SceneManager_ResetAllStaticModels();
    Entity::FreeAllDObjs(true);
    G_CleanupAnimTrees();
    HudElem_DestroyAll();
    cFreeList_Shutdown(&gRefEntFreeList);
    cFreeList_Shutdown(&gDObjFreeList);
    cFreeList_Shutdown(&gDSkelFreeList);
    cFreeList_Shutdown(&gDSkelMaxFreeList);
    cFreeList_Shutdown(&gDSkel4FreeList);
    cFreeList_Shutdown(&gEntFreeList);
    if (s_vehicles != nullptr)
    {
        mem_heap_free(s_vehicles);
        s_vehicles = nullptr;
        level.MaxVehicles = 0;
    }
    BG_FreeWeaponInfo();
    G_FreeInteractionInfo();
    void* v1 = InteractionController_Inst(currCl);
    InteractionController_EndInteraction(v1, 1);
    InteractionController_ClearQueue(v1);
    G_FreeScrVehicleInfo();
    if (restart == 0)
    {
        for (int i = 0; i < 16; ++i)
            g_scr_data.actorCorpseInfo[i].mEntity.mHandle.mVal = 0;
        Scr_FreePrecachedAnimTrees();
        Com_FreeWeaponInfoMemory(1, 0);
    }
}

// ea: 0x00466E00
bool SpotWouldTelefrag(const math::Position3* origin)
{
    math::Position3 mins;
    math::Position3 maxs;
    mins.v.m128_f32[0] = playerMins.v.m128_f32[0] + origin->v.m128_f32[0];
    mins.v.m128_f32[1] = playerMins.v.m128_f32[1] + origin->v.m128_f32[1];
    mins.v.m128_f32[2] = playerMins.v.m128_f32[2] + origin->v.m128_f32[2];
    maxs.v.m128_f32[0] = playerMaxs.v.m128_f32[0] + origin->v.m128_f32[0];
    maxs.v.m128_f32[1] = playerMaxs.v.m128_f32[1] + origin->v.m128_f32[1];
    maxs.v.m128_f32[2] = playerMaxs.v.m128_f32[2] + origin->v.m128_f32[2];
    int entityList[256];
    int num = CM_AreaEntities(&mins, &maxs, entityList, 256, 33555025);
    for (int v3 = 0; v3 < num; ++v3)
    {
        Entity* mObject = HandleDbToEnt(
            *(DbLinkedHandle<EntityHandleDb, Entity>*)&entityList[v3]);
        if (mObject == nullptr)
            continue;
        Client* client = mObject->client;
        if (client != nullptr && client->ps.pm_type < 6)
            return true;
        if ((mObject->actor != nullptr && mObject->health > 0)
            || (mObject->scr_vehicle != nullptr && mObject->health > 0))
            return true;
    }
    return false;
}

// ea: 0x0044A050
void G_AddLean(Entity* ent, float* point)
{
    if (ent->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 1292;
        AeAssert::gCurrentExpr = "ent->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    AddLeanToPosition(point, ent->client->ps.viewangles[1],
                      ent->client->ps.leanf, 16.0f, 20.0f);
}

// ea: 0x00492600
void G_RunFrame(int msec)
{
    commit_dobjects();
    UpdateCVars();
    if (level.actorPredictDepth != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
        AeAssert::gCurrentLine = 3592;
        AeAssert::gCurrentExpr = "!level.actorPredictDepth";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (g_performanceTest.integer == 0)
    {
        void* v2 = InteractionController_Inst(currCl);
        InteractionController_Update(v2, msec * 0.001f);
        UpdateAnims(msec);
    }
    float delta = msec * 0.001f;
    AdvanceSceneAnims(delta);
    UpdatePlayer();
    j_nullsub_20();
    PlayerAnimMgr_Update(delta);
    ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 1000> linkedEntities;
    linkedEntities.m_size = 0;
    UpdateEntities(&linkedEntities, msec);
    UpdateRigidBody(delta);
    UpdateLinkedEntities(&linkedEntities);
    if (cls.state == CA_ACTIVE)
        MultiplayerMgr::sInst->Step(0, false, true);
    if (level.actorPredictDepth != 0)
        Com_Error(ERR_DROP, "actorPredictDepth mismatch");
    if (level.maxclients > 0)
    {
        for (int v1 = 0; v1 < level.maxclients; ++v1)
        {
            Entity* v6 = EntityManager::sInst->mPlayers[v1];
            if (v6->sentient != nullptr)
            {
                gCurrentCamera = v6->IsLocalPlayer() ? 1 : 0;
                ClientEndFrame(v6, msec);
            }
        }
    }
    cdl_proftimer_ent_actors.start();
    cdl_proftimer_ent_actors.stop();
    Path_DrawDebug();
    G_DrawVehiclePaths();
    G_DrawEntityBBoxes();
    if (g_listEntity != 0)
    {
        Entity** begin = EntityHandleDb::sInst.mActiveList.m_elements;
        Entity** end = begin + EntityHandleDb::sInst.mActiveList.m_size;
        int v9 = 0;
        for (Entity** p = begin; p != end; ++p)
        {
            if (*p != nullptr)
                G_Printf("%4i: %s\n", v9++, (*p)->mClassName.c_str());
        }
        Cvar_Set("g_listEntity", "0");
    }
    ShowEntityInfo();
    DObjSetNotRenderedFlag();
}

// ea: 0x004554E0
void SpectatorThink(Entity* ent, usercmd_s* ucmd)
{
    Client* client = ent->client;
    client->oldbuttons = client->buttons;
    client->buttons = ucmd->buttons;
    pmove_t pm;
    memset(&pm, 0, sizeof(pm));
    pm.ps = &client->ps;
    pm.cmd = *ucmd;
    pm.trace = g_TraceCapsule;
    pm.boxtrace = g_TraceCapsule;
    pm.capsuletrace = g_TraceCapsule;
    pm.tracemask = 0x800011;
    pm.pointcontents = SV_PointContents;
    client->ps.pm_type = (0x100000 & client->ps.pm_flags) != 0 ? 1 : 4;
    client->ps.speed = (ent->sentient->noSpectate & 0x20) == 0 ? 400 : 0;
    Pmove(&pm, false);
    ent->r.currentOrigin.v.m128_f32[0] = client->ps.origin.v.m128_f32[0];
    ent->r.currentOrigin.v.m128_f32[1] = client->ps.origin.v.m128_f32[1];
    ent->r.currentOrigin.v.m128_f32[2] = client->ps.origin.v.m128_f32[2];
    if (!Entity_IsInRagdoll(ent))
        SV_UnlinkEntity(ent);
}

// ea: 0x00473C40
void Player_UpdateActivate(Entity* ent)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
        AeAssert::gCurrentLine = 192;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
        AeAssert::gCurrentLine = 193;
        AeAssert::gCurrentExpr = "ent->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ent->client->ps.pm_flags &= ~0x40000u;
    Entity* mObject = HandleDbToEnt(ent->client->mUseHoldEntity);
    bool v2 = false;
    if (mObject != nullptr
        && (ent->client->oldbuttons & 0x40) != 0
        && (ent->client->buttons & 0x40) == 0)
    {
        ent->client->ps.pm_flags |= 0x40000u;
        return;
    }
    if ((ent->client->latched_buttons & 0x60) != 0)
        v2 = Player_ActivateCmd(ent) != 0;
    if (mObject == nullptr && !v2)
    {
        if ((ent->client->latched_buttons & 0x40) == 0)
            return;
        ent->client->ps.pm_flags |= 0x40000u;
        return;
    }
    if ((ent->client->buttons & 0x60) != 0)
        Player_ActivateHoldCmd(ent);
}

// ea: 0x00451800
void VP_SetScriptVariable(const char* /*a1*/, const char* /*a2*/, vehicle_node_t* /*a3*/)
{
    ;
}

// ea: 0x00455380
void HealthRegen(Entity* e, float deltaT)
{
    static unsigned char s_init = 0;
    static unsigned int sDamageStr = 0;
    if (e->health > 0)
    {
        if (!(s_init & 1))
        {
            s_init |= 1;
            sDamageStr = HashString::CalcHash("damage");
        }
        if (e->mNotifySet != nullptr && EntityNotifySet_GetNotify(e->mNotifySet, sDamageStr) != nullptr)
            e->client->ps.mTimeSinceDamage = 4.0f;
        Client* client = e->client;
        float v5 = deltaT;
        if (client->ps.mTimeSinceDamage > 0.0f)
            client->ps.mTimeSinceDamage -= deltaT;
        Client* v6 = e->client;
        if (v6->ps.mTimeSinceDamage <= 0.0f && e->health < g_player_maxhealth.integer)
            v6->ps.mHealthDelta += v5 * 50.0f;
        Client* v7 = e->client;
        float mHealthDelta = v7->ps.mHealthDelta;
        if (mHealthDelta > 1.0f)
        {
            v7->ps.mHealthDelta -= mHealthDelta;
            int integer = (int)mHealthDelta + e->health;
            if (integer >= g_player_maxhealth.integer)
                integer = g_player_maxhealth.integer;
            e->health = integer;
        }
    }
}

// ea: 0x004574B0
void G_DebugArc(const float* center, float radius, float angle0, float angle1,
                const float* color, int depthTest, int duration)
{
    float step = (angle1 - angle0) * 0.06666667f;
    if (step < 0.0f)
    {
        angle0 = angle0 - 360.0f;
        step = (angle1 - angle0) * 0.06666667f;
    }
    float pts[16][3];
    for (int v10 = 0; v10 < 16; ++v10)
    {
        float radians = ((v10 * step) + angle0) * 3.1415927f * 0.0055555557f;
        float s = sinf(radians);
        float c = cosf(radians);
        pts[v10][0] = (c * radius) + center[0];
        pts[v10][1] = (s * radius) + center[1];
        pts[v10][2] = center[2];
    }
    for (int i = 0; i < 15; ++i)
        CL_AddDebugLine(pts[i], pts[i + 1], color, depthTest, duration, 1, 0);
}

// ea: 0x00457170
void G_DebugCircleEx(const float* center, float radius, const float* dir,
                     const float* color, int depthTest, int duration)
{
    float normal[3];
    VectorNormalize2(dir, normal);
    float up[3];
    PerpendicularVector(up, normal);
    float right[3];
    CrossProduct(normal, up, right);
    float pts[16][3];
    for (int v7 = 0; v7 < 16; ++v7)
    {
        float radians = v7 * 0.39269909f;
        float s = sinf(radians);
        float c = cosf(radians);
        pts[v7][0] = ((up[0] * (c * radius)) + (right[0] * (s * radius))) + center[0];
        pts[v7][1] = ((up[1] * (c * radius)) + (right[1] * (s * radius))) + center[1];
        pts[v7][2] = ((up[2] * (c * radius)) + (right[2] * (s * radius))) + center[2];
    }
    for (int i = 0; i < 16; ++i)
        CL_AddDebugLine(pts[i], pts[(i + 1) & 0xF], color, depthTest, duration, 1, 0);
}

// ea: 0x004835F0
void TossClientItems(Entity* self)
{
    if (MultiplayerMgr::sInst->IsLocalPlayer(self))
    {
        float forward[3];
        AnglesToForward(self->r.currentAngles.v.m128_f32, forward);
        Client* client = self->client;
        int v3 = client->ps.weaponslots[1];
        if (Com_BitCheck(client->ps.weapons, v3) != 0
            && v3 > 0
            && v3 <= BG_GetNumWeapons()
            && (self->client->ps.ammo[BG_AmmoForWeapon(v3)] != 0
                || self->client->ps.ammoclip[BG_ClipForWeapon(v3)] != 0))
        {
            BG_FindItemForWeapon(v3);
            Client* v4 = self->client;
            if ((v4->ps.eFlags & 0x6000) == 0 && v4->pers.playerState != 1)
            {
                Entity* v5 = Drop_Weapon(self, v3, nullptr);
                Entity* v6 = v5;
                if (v5 != nullptr)
                {
                    MultiplayerMgr::MPEntityHandle v16;
                    MultiplayerMgr::sInst->RegisterDroppedItem(1, v5, self, 0);
                    math::Position3 v14;
                    native_to_cdl_pos3(&v14, v6->s.apos.trBase);
                    int count = v6->count;
                    int count2 = v6->count2;
                    math::Dir3 v13 = native_to_cdl_dir3(v6->s.pos.trDelta);
                    math::Position3 v12;
                    native_to_cdl_pos3(&v12, v6->s.pos.trBase);
                    MultiplayerMgr::sInst->DropWeapon(v3, v16.mVal, &v12, &v14,
                                                      &v13, count2, count);
                }
            }
        }
    }
}

// ea: 0x00474710
void G_TouchEnts(Entity* ent, int numtouch, DbLinkedHandle<EntityHandleDb, Entity>* touchents)
{
    for (int i = 0; i < numtouch; ++i)
    {
        int j;
        for (j = 0; j < i; ++j)
        {
            if (touchents[j].mHandle.mVal == touchents[i].mHandle.mVal)
                break;
        }
        if (j == i)
        {
            Entity* mObject = HandleDbToEnt(touchents[i]);
            if (Scr_IsSystemActive(1) != 0)
            {
                Scr_NotifyFromEnt(ent, hash_const.touch, mObject);
                Scr_NotifyFromEnt(mObject, hash_const.touch, ent);
            }
            uint8_t touch = ent->touch;
            if (touch != 0)
            {
                if (touch >= 0xDu)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
                    AeAssert::gCurrentLine = 157;
                    AeAssert::gCurrentExpr = "ent->touch > 0 && ent->touch < TOUCH_MAX";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                touchtable[ent->touch](ent, mObject, 1);
            }
            uint8_t v8 = mObject->touch;
            if (v8 != 0)
            {
                if (v8 >= 0xDu)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
                    AeAssert::gCurrentLine = 164;
                    AeAssert::gCurrentExpr = "other->touch > 0 && other->touch < TOUCH_MAX";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                touchtable[mObject->touch](mObject, ent, 1);
            }
        }
    }
}

// ea: 0x0048D980
void Bullet_Fire(Entity* attacker, float spread, int damage, weaponParms* wp,
                 Entity* weaponEnt, float coneAngleTangent)
{
    weaponFileInfo_t* pWeapInfo = wp->pWeapInfo;
    if (attacker->actor != nullptr)
    {
        if (pWeapInfo->aiDamageMod > 0.0f)
            damage = (int)(damage * pWeapInfo->aiDamageMod);
    }
    if (attacker->client != nullptr && wp->pWeapInfo->weapClass == 17)
    {
        int time = level.time;
        math::Position3 muzzlePoint;
        CalcMuzzlePoint(attacker, &muzzlePoint);
        MultiplayerMgr::sInst->SpreadFire(attacker, attacker->client->fGunPitch,
                                          attacker->client->fGunYaw,
                                          muzzlePoint.v.m128_f32, pWeapInfo->index,
                                          spread, coneAngleTangent, time);
        G_BulletFireSpread(weaponEnt, attacker, wp, damage, spread, weaponEnt,
                           coneAngleTangent, time);
    }
    else
    {
        float end[3];
        end[0] = wp->muzzleTrace[0];
        end[1] = wp->muzzleTrace[1];
        end[2] = wp->muzzleTrace[2];
        math::Position3 weaponPos;
        Bullet_Endpos(spread, &weaponPos.v.m128_f32[2], wp);
        unsigned int mVal;
        if (weaponEnt != nullptr)
            mVal = weaponEnt->mHandle.mHandle.mVal;
        else
            mVal = EntityManager::sInst->mWorld->mHandle.mHandle.mVal;
        Bullet_Fire_Extended(DbLinkedHandle<EntityHandleDb, Entity>(), attacker, end,
                             &weaponPos.v.m128_f32[2], damage, 0, wp,
                             DbLinkedHandle<EntityHandleDb, Entity>(), coneAngleTangent);
    }
}

// ea: 0x0046F4F0
float scr_vehicle_t_GetAverageWheelSpeed(scr_vehicle_t* veh)
{
    rb_vehicle* mRBVeh = (rb_vehicle*)veh->mRBVeh;
    if (mRBVeh != nullptr)
    {
        float total = 0.0f;
        int count = 0;
        for (int i = 0; i < 6; ++i)
        {
            rigid_body_constraint_wheel* v = mRBVeh->m_wheels[i];
            if (v != nullptr && (v->m_wheel_flags & 0x10) != 0)
            {
                total += v->m_wheel_vel;
                ++count;
            }
        }
        if (count > 0)
            return total / count;
    }
    else
    {
        Entity* mObject = HandleDbToEnt(veh->mEntity);
        if (mObject != nullptr)
            return (mObject->speed * 0.1f) / 1.0f;
    }
    return 0.0f;
}

// ea: 0x00449A30
void SetClientOrigin(Entity* ent, const float* origin)
{
    if (ent->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 611;
        AeAssert::gCurrentExpr = "ent->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ent->client->ps.origin.v.m128_f32[0] = origin[0];
    ent->client->ps.origin.v.m128_f32[1] = origin[1];
    ent->client->ps.origin.v.m128_f32[2] = origin[2] + 1.0f;
    ent->client->ps.eFlags ^= 8u;
    BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, 1);
    if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 623;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    ent->r.currentOrigin.v.m128_f32[0] = ent->client->ps.origin.v.m128_f32[0];
    ent->r.currentOrigin.v.m128_f32[1] = ent->client->ps.origin.v.m128_f32[1];
    ent->r.currentOrigin.v.m128_f32[2] = ent->client->ps.origin.v.m128_f32[2];
    memcpy(ent->s.pos.trBase, &ent->client->ps.origin, sizeof(ent->s.pos.trBase));
}

// ea: 0x00454200
void G_AddEvent(Entity* ent, int event, int eventParm)
{
    if (event != 0)
    {
        if (event >= 0x100)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
            AeAssert::gCurrentLine = 2110;
            AeAssert::gCurrentExpr = "(unsigned) event < 256";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2109;
        AeAssert::gCurrentExpr = "event";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (eventParm >= 0x100)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2111;
        AeAssert::gCurrentExpr = "(unsigned) eventParm < 256";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->s.eType >= 0x12u)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2112;
        AeAssert::gCurrentExpr = "ent->s.eType < ET_EVENTS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Client* client = ent->client;
    if (client != nullptr)
    {
        client->ps.event.events[client->ps.event.eventSequence & 3] = (uint8_t)event;
        ent->client->ps.event.eventParms[ent->client->ps.event.eventSequence++ & 3] = (uint8_t)eventParm;
    }
    else
    {
        ent->s.events[ent->s.eventSequence & 3] = (uint8_t)event;
        ent->s.eventParms[ent->s.eventSequence++ & 3] = (uint8_t)eventParm;
    }
    ent->r.eventTime = level.time;
}

// ea: 0x00471100
int update_trigger_notifies(void)
{
    int result = g_performanceTest.integer;
    if (g_performanceTest.integer == 0)
    {
        while (level.triggerListSize > 0)
        {
            Entity* ent = HandleDbToEnt(level.triggerList[0].mEntity);
            Entity* mObject = HandleDbToEnt(level.triggerList[0].mOtherEntity);
            if (ent != nullptr && mObject != nullptr
                && ent->s.useCount == level.triggerList[0].useCount
                && mObject->s.useCount == level.triggerList[0].otherUseCount)
            {
                if (ent->targetname == "checkpoint")
                {
                    const char* v6 = ent->mTarget.c_str();
                    if (v6 == nullptr)
                        v6 = defaultFileName;
                    CheckpointMgr::sInst->SaveCheckpoint(v6, false);
                }
                Scr_NotifyFromEnt(ent, hash_const.trigger, mObject);
            }
            else if (ent == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_trigger.cpp";
                AeAssert::gCurrentLine = 915;
                AeAssert::gCurrentExpr = "ent";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("thou art in a trigger that hast been deleted! (verily, it wast probably unloaded)"))
                    __debugbreak();
            }
            level.triggerList[0] = level.triggerList[level.triggerListSize];
            --level.triggerListSize;
            result = 0;
        }
        level.triggerListSize = 0;
    }
    return result;
}

// ea: 0x00470BD0
void G_CheckHitTriggerDamage(Entity* pActivator, const math::Position3* vStart,
                             const math::Position3* vEnd, int iDamage, int iMOD)
{
    math::Position3 mins;
    math::Position3 maxs;
    mins.v = _mm_min_ps(vStart->v, vEnd->v);
    maxs.v = _mm_max_ps(vStart->v, vEnd->v);
    int entityList[256];
    int iNum = CM_AreaEntities(&mins, &maxs, entityList, 256, 0x400000);
    for (int v7 = 0; v7 < iNum; ++v7)
    {
        Entity* mObject = HandleDbToEnt(
            *(DbLinkedHandle<EntityHandleDb, Entity>*)&entityList[v7]);
        if (mObject != nullptr
            && mObject->takedamage != 0
            && mObject->mClassNameHash.mHash == hash_const.trigger_damage.mHash)
        {
            math::Position3 zeroMins, zeroMaxs;
            zeroMins.v = _mm_setzero_ps();
            zeroMaxs.v = _mm_setzero_ps();
            collision_context_t context;
            if (SV_SightTraceToEntity(vStart, &zeroMins, &zeroMaxs, vEnd,
                                      mObject->mHandle, &context, 1) != 0)
            {
                int h = pActivator->mHandle.mHandle.mVal;
                mObject->Notify(hash_const.damage, iDamage, (Broc::entity*)&h,
                                &iMOD, nullptr, nullptr);
                Activate_trigger_damage(mObject, pActivator, iDamage, iMOD);
                if (mObject->count == 0)
                    mObject->health = 32000;
            }
        }
    }
}

// ea: 0x00470D70
void G_GrenadeTouchTriggerDamage(Entity* pActivator, const math::Position3* vStart,
                                 const math::Position3* vEnd, int iDamage, int iMOD)
{
    math::Position3 mins, maxs;
    mins.v = _mm_min_ps(vStart->v, vEnd->v);
    maxs.v = _mm_max_ps(vStart->v, vEnd->v);
    int entityList[1344];
    int iNum = CM_AreaEntities(&mins, &maxs, entityList, 1344, 0x400000);
    for (int v7 = 0; v7 < iNum; ++v7)
    {
        Entity* mObject = HandleDbToEnt(
            *(DbLinkedHandle<EntityHandleDb, Entity>*)&entityList[v7]);
        if (mObject != nullptr
            && mObject->takedamage != 0
            && mObject->mClassNameHash.mHash == hash_const.trigger_damage.mHash
            && (mObject->flags & 0x40000) != 0)
        {
            math::Position3 zeroMins, zeroMaxs;
            zeroMins.v = _mm_setzero_ps();
            zeroMaxs.v = _mm_setzero_ps();
            collision_context_t context;
            if (SV_SightTraceToEntity(vStart, &zeroMins, &zeroMaxs, vEnd,
                                      mObject->mHandle, &context, 1) != 0)
            {
                int h = pActivator->mHandle.mHandle.mVal;
                mObject->Notify(hash_const.damage, iDamage, (Broc::entity*)&h,
                                &iMOD, nullptr, nullptr);
                Activate_trigger_damage(mObject, pActivator, iDamage, iMOD);
                if (mObject->count == 0)
                    mObject->health = 32000;
            }
        }
    }
}

// ea: 0x00470F40
int G_CheckPointInsideTriggerMount(Entity* pActivator, float* vStart, int* crouch)
{
    math::Position3 p;
    p.v.m128_f32[0] = vStart[0];
    p.v.m128_f32[1] = vStart[1];
    p.v.m128_f32[2] = vStart[2];
    int contents = CM_PointContents(&p, nullptr);
    if ((0x400000 & contents) != 0)
    {
        if (crouch != nullptr)
            *crouch = 1;
        return 1;
    }
    if ((0x1000000 & contents) != 0)
        return 1;
    int entityList[256];
    float v12[3] = { vStart[0] - 0.1f, vStart[1] - 0.1f, vStart[2] - 0.1f };
    math::Position3 vMins;
    vMins.v.m128_f32[0] = vStart[0] + 0.1f;
    vMins.v.m128_f32[1] = vStart[1] + 0.1f;
    vMins.v.m128_f32[2] = vStart[2] + 0.1f;
    int v5 = CM_AreaEntities((const math::Position3*)v12, &vMins, entityList, 256,
                             1094713352);
    for (int v6 = 0; v6 < v5; ++v6)
    {
        Entity* mObject = HandleDbToEnt(
            *(DbLinkedHandle<EntityHandleDb, Entity>*)&entityList[v6]);
        if (mObject == nullptr)
            continue;
        if (mObject->actor == nullptr)
        {
            if (mObject->mClassNameHash.mHash == hash_const.trigger_mount.mHash)
            {
                if (crouch != nullptr)
                    *crouch = mObject->spawnflags & 1;
                return 1;
            }
            if ((0x1000000 & mObject->r.contents) != 0)
                return 1;
            if ((0x400000 & mObject->r.contents) != 0)
            {
                if (crouch != nullptr)
                    *crouch = 1;
                return 1;
            }
        }
    }
    return 0;
}

// ea: 0x004671F0
void Client_Touch(Entity* pSelf, Entity* pOther)
{
    if (pSelf->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 755;
        AeAssert::gCurrentExpr = "pSelf->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 756;
        AeAssert::gCurrentExpr = "pSelf->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (Client_GetPushed(pSelf, pOther) == 0)
        pSelf->client->inControlTime = level.time;
    pSelf->client->lastTouchTime = level.time;
    actor_s* actor = pOther->actor;
    if (actor != nullptr)
    {
        if (HandleDbToEnt(actor->closeEnt) == nullptr
            && actor->bDontAvoidPlayer == 0
            && (*(int*)((char*)&actor->Physics + 0x80) & 0x2000000) != 0)
        {
            if (pOther->sentient == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
                AeAssert::gCurrentLine = 766;
                AeAssert::gCurrentExpr = "pOther->sentient";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            if (((1 << pOther->sentient->eTeam)
                 & ~(1 << Sentient_EnemyTeam(pSelf->sentient->eTeam))) != 0)
            {
                actor->closeEnt.mHandle.mVal = pSelf->mHandle.mHandle.mVal;
            }
        }
    }
}

// ea: 0x004572F0
void G_DebugCircle2Ex(const float* center, float radius, const float* dir,
                      const float* color, int depthTest, int duration)
{
    float normal[3];
    VectorNormalize2(dir, normal);
    float up[3];
    PerpendicularVector(up, normal);
    float right[3];
    CrossProduct(normal, up, right);
    float pts[16][3];
    for (int v7 = 0; v7 < 16; ++v7)
    {
        float radians = v7 * 0.39269909f;
        float s = sinf(radians);
        float c = cosf(radians);
        pts[v7][0] = ((up[0] * (c * radius)) + (right[0] * (s * radius))) + center[0];
        pts[v7][1] = ((up[1] * (c * radius)) + (right[1] * (s * radius))) + center[1];
        pts[v7][2] = ((up[2] * (c * radius)) + (right[2] * (s * radius))) + center[2];
    }
    for (int i = 0; i < 16; ++i)
        CL_AddDebugLine(pts[i], pts[(i + 1) & 0xF], color, depthTest, duration, 1, 0);
    for (int i = 0; i < 16; ++i)
        CL_AddDebugLine((float*)center, pts[(i + 1) & 0xF], color, depthTest, duration, 1, 0);
}

// ea: 0x004673B0
char* ClientConnect(DbLinkedHandle<EntityHandleDb, Entity> entity)
{
    Entity* mObject = HandleDbToEnt(entity);
    Client* client = mObject->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 854;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Client can not be null"))
            __debugbreak();
    }
    Client_Clear(client, true, true);
    client->pers.connected = CON_CONNECTING;
    int v4 = (int)(g_player_maxhealth.value + 0.5f);
    client->pers.maxHealth = v4;
    client->ps.stats[2] = v4;
    client->ps.mClient.mHandle.mVal = mObject->mHandle.mHandle.mVal;
    mObject->touch = 2;
    mObject->pain = 0;
    mObject->client = client;
    if (mObject->sentient != nullptr)
    {
        Sentient_Clean(mObject->sentient);
    }
    else
    {
        sentient_s* v7 = Sentient_Alloc();
        if (v7 == nullptr)
            G_Error("No sentient for player.\n");
        mObject->sentient = v7;
    }
    mObject->sentient->pEnt = mObject;
    mObject->sentient->eTeam = TEAM_ALLIES;
    mObject->sentient->fScariness = 1.0f;
    client->pers.playerState = 0;
    client->pers.rank = 0;
    client->ps.ctf_has_flag = 0;
    client->pers.playerClass = -1;
    client->pers.nextPlayerClass = -1;
    client->ps.eFlags = 16;
    mObject->r.svFlags = 512;
    mObject->maxHealth = (int)g_player_maxhealth.value;
    return nullptr;
}

// ea: 0x004698C0
Entity* G_TestEntityPosition(Entity* ent, const math::Position3* origin)
{
    if (IS_NAN(origin->v.m128_f32[0]) || IS_NAN(origin->v.m128_f32[1])
        || IS_NAN(origin->v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
        AeAssert::gCurrentLine = 79;
        AeAssert::gCurrentExpr = "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    unsigned int clipmask = ent->clipmask;
    if (clipmask != 0)
    {
        if (ent->r.contents == 0x4000000)
            return nullptr;
    }
    else
    {
        clipmask = 17;
    }
    int capsule = 0;
    unsigned int mVal;
    if (ent->s.eType == 3)
    {
        mVal = ent->r.mOwner.mHandle.mVal;
    }
    else if (ent->s.eType == 1)
    {
        mVal = ent->mHandle.mHandle.mVal;
        capsule = 1;
    }
    else
    {
        mVal = 0;
    }
    math::Position3 end;
    end.v.m128_f32[0] = origin->v.m128_f32[0];
    end.v.m128_f32[1] = origin->v.m128_f32[1];
    end.v.m128_f32[2] = origin->v.m128_f32[2] + 16.0f;
    collision_context_t context(DbLinkedHandle<EntityHandleDb, Entity>(), (int)clipmask);
    trace_t tr;
    memset(&tr, 0, sizeof(tr));
    SV_Trace(&tr, origin, &ent->r.mins, &ent->r.maxs, &end, &context,
             capsule, 0, nullptr, 0, 0.0f);
    if (tr.mEntity.mHandle.mVal == 0)
        return nullptr;
    return HandleDbToEnt(tr.mEntity);
}

// ea: 0x004816E0
Entity* weapon_grenadelauncher_fire(Entity* ent, int grenType, weaponParms* wp)
{
    weaponFileInfo_t* pWeapInfo = wp->pWeapInfo;
    float tossPos[3];
    tossPos[0] = pWeapInfo->iProjectileSpeed * wp->forward[0];
    tossPos[1] = pWeapInfo->iProjectileSpeed * wp->forward[1];
    tossPos[2] = pWeapInfo->iProjectileSpeed * wp->forward[2];
    tossPos[2] += pWeapInfo->iProjectileSpeedUp;
    int iFuseTime = pWeapInfo->iFuseTime;
    float start[3];
    start[0] = (wp->forward[0] * delta_0) + wp->muzzleTrace[0];
    start[1] = (wp->forward[1] * delta_0) + wp->muzzleTrace[1];
    start[2] = (wp->forward[2] * delta_0) + wp->muzzleTrace[2];
    Entity* v7 = fire_grenade(ent, start, tossPos, grenType, iFuseTime);
    VectorNormalize(tossPos);
    float v8 = ((ent->client->ps.velocity.v.m128_f32[0] * tossPos[0])
                + (ent->client->ps.velocity.v.m128_f32[1] * tossPos[1]))
               + (ent->client->ps.velocity.v.m128_f32[2] * tossPos[2]);
    v7->s.pos.trDelta[0] += v8 * tossPos[0];
    v7->s.pos.trDelta[1] += v8 * tossPos[1];
    v7->s.pos.trDelta[2] += v8 * tossPos[2];
    math::Dir3 dir;
    dir.v.m128_f32[0] = wp->forward[0];
    dir.v.m128_f32[1] = wp->forward[1];
    dir.v.m128_f32[2] = wp->forward[2];
    math::Position3 pos;
    pos.v.m128_f32[0] = start[0];
    pos.v.m128_f32[1] = start[1];
    pos.v.m128_f32[2] = start[2];
    MultiplayerMgr::MPEntityHandle h;
    h.mVal = 0;
    MultiplayerMgr::sInst->FireMissile(grenType, pos, dir, h);
    return v7;
}

// ea: 0x00456160
void StopFollowing(Entity* ent)
{
    Client* client = ent->client;
    if ((0x100000 & client->ps.pm_flags) != 0
        && client->ps.spectatorClient != -1
        && gpBrocAPI != nullptr)
    {
        if (gpBrocAPI->mBrocExports.mCallbackStopFollowing != nullptr)
            gpBrocAPI->mBrocExports.mCallbackStopFollowing();
    }
    ent->client->ps.spectatorClient = -1;
    if ((0x100000 & client->ps.pm_flags) != 0 && !Entity_IsInRagdoll(ent))
    {
        float vUp[3];
        vUp[0] = client->ps.viewangles[0];
        vUp[1] = client->ps.viewangles[1];
        vUp[2] = client->ps.viewangles[2];
        float v8[3];
        AnglesToForward(vUp, v8);
        float vUp2[3];
        AnglesToUp(vUp, vUp2);
        vUp[0] += 15.0f;
        float vAngles[3];
        vAngles[0] = client->ps.origin.v.m128_f32[0];
        vAngles[1] = client->ps.origin.v.m128_f32[1];
        vAngles[2] = client->ps.origin.v.m128_f32[2] + client->ps.viewHeightCurrent;
        G_AddLean(ent, vAngles);
        client->ps.pm_flags &= 0xFFEFFFDF;
        client->ps.eFlags &= 0xFFEF9FFF;
        client->ps.viewlocked = 0;
        client->ps.fWeaponPosFrac = 0.0f;
        G_SetOrigin(ent, vAngles);
        client->ps.origin.v.m128_f32[0] = vAngles[0];
        client->ps.origin.v.m128_f32[1] = vAngles[1];
        client->ps.origin.v.m128_f32[2] = vAngles[2];
        SetClientViewAngle(ent, vUp);
        client->ps.shellshockIndex = 0;
        client->ps.shellshockTime = 0;
        client->ps.shellshockDuration = 0;
        if (ent->IsLocalPlayer())
            g_doShellShock[ent->GetPlayerIndex()] = 0;
    }
}

// ea: 0x00481500
void Spread_Fire_Fake(Entity* attacker, float gunPitch, float gunYaw,
                      const float* weaponPosition, int weapon, float spread,
                      float coneAngleTangent, unsigned int seed)
{
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(weapon);
    if (InfoForWeapon != nullptr)
    {
        weaponParms wp;
        wp.pWeapInfo = InfoForWeapon;
        float viewang[3] = { gunPitch, gunYaw, 0.0f };
        AngleVectors(viewang, wp.forward, wp.right, wp.up);
        wp.muzzleTrace[0] = weaponPosition[0];
        wp.muzzleTrace[1] = weaponPosition[1];
        wp.muzzleTrace[2] = weaponPosition[2];
        float start[3] = { weaponPosition[0], weaponPosition[1], weaponPosition[2] };
        if (InfoForWeapon->iShotCount > 0)
        {
            bdRandomState rng;
            bdRandom_setSeed(&rng, seed);
            for (int v8 = 0; v8 < InfoForWeapon->iShotCount; ++v8)
            {
                float randomA = bdRandom_nextUInt(&rng) * 4.6566129e-10f;
                float randomB = bdRandom_nextUInt(&rng) * 4.6566129e-10f;
                float end[3];
                Bullet_Endpos(spread, end, &wp, randomA, randomB);
                Bullet_Fire_Fake_Extended(attacker->mHandle, attacker, start, end,
                                          0, 0, &wp, attacker->mHandle,
                                          coneAngleTangent);
            }
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 1318;
        AeAssert::gCurrentExpr = "info";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Spread_Fire_Fake: invalid weapon"))
            __debugbreak();
    }
}

// ea: 0x00461930
void Cmd_SetViewpos_f(Entity* ent)
{
    if (g_cheats->integer != 0)
    {
        if (Cmd_Argc() == 5)
        {
            float origin[3];
            float angles[3] = { 0.0f, 0.0f, 0.0f };
            char buffer[128];
            for (int i = 0; i < 3; ++i)
            {
                Cmd_ArgvBuffer(i + 1, buffer, 128);
                origin[i] = (float)atof(buffer);
            }
            Cmd_ArgvBuffer(4, buffer, 128);
            angles[1] = (float)atof(buffer);
            SetClientOrigin(ent, origin);
            SetClientViewAngle(ent, angles);
            g_LinkEntity(ent);
        }
        else
        {
            SV_GameSendServerCommand(ent->mHandle, "print \"usage: viewpos <x> <y> <z> <yaw>\"");
        }
    }
    else
    {
        SV_GameSendServerCommand(ent->mHandle, "print \"GAME_CHEATSNOTENABLED\"");
    }
}

// ea: 0x00455C10
void G_AddInvalidatedNode(Entity* pEnt, PathNodes::PathNode* pNode)
{
    if (pEnt != nullptr && pEnt->client != nullptr && pNode != nullptr
        && G_FindInvalidatedNode(pEnt, pNode) < 0)
    {
        if (pEnt->client->mInvalidatedNodeNum >= 5)
        {
            float curPos[3];
            curPos[0] = pEnt->r.currentOrigin.v.m128_f32[0];
            curPos[1] = pEnt->r.currentOrigin.v.m128_f32[1];
            curPos[2] = pEnt->r.currentOrigin.v.m128_f32[2];
            float fartDist = VectorDistanceSquared(pNode->mConstant.mOrigin, curPos);
            Client* client = pEnt->client;
            int iRmv = -1;
            for (int v6 = 0; v6 < client->mInvalidatedNodeNum; ++v6)
            {
                float dist;
                PathNodes::PathNode* v8 = HandleDbToNode(client->mInvalidatedNode[v6]);
                if (v8 != nullptr)
                    dist = VectorDistanceSquared(v8->mConstant.mOrigin, curPos);
                else
                    dist = 100000000.0f;
                if (dist > fartDist)
                {
                    fartDist = dist;
                    iRmv = v6;
                }
            }
            if (iRmv >= 0)
            {
                Path_MarkNodeInvalid(pNode, pEnt->sentient->eTeam);
                pEnt->client->mInvalidatedNode[iRmv].mValue = pNode->mHandle.mValue;
            }
        }
        else
        {
            Path_MarkNodeInvalid(pNode, pEnt->sentient->eTeam);
            pEnt->client->mInvalidatedNode[pEnt->client->mInvalidatedNodeNum++].mValue =
                pNode->mHandle.mValue;
        }
    }
}

// ea: 0x004679F0
int Cmd_FollowCycle_f(Entity* ent, int dir)
{
    if (dir != 1 && dir != -1)
        G_Error("Cmd_FollowCycle_f: bad dir %i", dir);
    if (ent->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_cmds.cpp";
        AeAssert::gCurrentLine = 1750;
        AeAssert::gCurrentExpr = "ent->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Client* client = ent->client;
    if (client->pers.playerState != 1)
        return -1;
    int spectatorClient = client->ps.spectatorClient;
    if (spectatorClient < 0)
        spectatorClient = 0;
    int v12 = spectatorClient;
    for (;;)
    {
        spectatorClient += dir;
        if (spectatorClient < level.maxclients)
        {
            if (spectatorClient < 0)
                spectatorClient = level.maxclients - 1;
        }
        else
        {
            spectatorClient = 0;
        }
        PlayerState ps;
        if (SV_GetCurrentClientInfo(spectatorClient, &ps) != 0)
        {
            Entity* mObject = HandleDbToEnt(
                EntityManager::sInst->mPlayers[spectatorClient]->mHandle);
            if (mObject != nullptr)
            {
                Client* v8 = mObject->client;
                if (v8 != nullptr && v8->pers.playerState == 3)
                {
                    if ((ps.pm_flags & 0x80000) == 0)
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_cmds.cpp";
                        AeAssert::gCurrentLine = 1782;
                        AeAssert::gCurrentExpr = "ps.pm_flags & (1<<19)";
                        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                    if (((1 << mObject->sentient->eTeam) & ent->sentient->noSpectate) == 0)
                        return spectatorClient;
                }
            }
        }
        if (spectatorClient == v12)
            return -1;
    }
}

// ea: 0x0048B2F0
void Cmd_Give_f(Entity* ent)
{
    if (g_cheats->integer != 0)
    {
        if (ent->health > 0)
        {
            const char* v5 = ConcatArgs(2);
            int v6 = atoi(v5);
            const char* name = ConcatArgs(1);
            if (name != nullptr && strlen(name) != 0)
            {
                int integer;
                if (v6 != 0)
                    integer = v6 + ent->health;
                else
                {
                    ent->client->ps.stats[2] = g_player_maxhealth.integer;
                    integer = g_player_maxhealth.integer;
                }
                ent->health = integer;
                level.initializing = 1;
                for (int v10 = 1; v10 <= BG_GetNumWeapons(); ++v10)
                    BG_GivePlayerWeapon(&ent->client->ps, v10);
                level.initializing = 0;
                if (v6 != 0)
                {
                    int weapon = ent->client->ps.weapon;
                    if (weapon != 0)
                        Add_Ammo(ent, weapon, v6, 1);
                }
                else
                {
                    for (int i = 1; i <= BG_GetNumWeapons(); ++i)
                    {
                        if (Com_BitCheck(ent->client->ps.weapons, i) != 0)
                            Add_Ammo(ent, i, 998, 1);
                    }
                }
                if (Q_stricmpn(name, "allammo", 7) == 0 && v6 != 0)
                {
                    for (int j = 1; j <= BG_GetNumWeapons(); ++j)
                    {
                        if (Com_BitCheck(ent->client->ps.weapons, j) != 0)
                            Add_Ammo(ent, j, v6, 1);
                    }
                }
            }
        }
        else
        {
            SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_MUSTBEALIVECOMMAND\""));
        }
    }
    else
    {
        SV_GameSendServerCommand(ent->mHandle, va("print \"GAME_CHEATSNOTENABLED\""));
    }
}

// ea: 0x00460F00
void SpectatorClientEndFrame(Entity* ent)
{
    Client* v2 = ent->client;
    ent->r.svFlags &= ~8u;
    ent->takedamage = 0;
    ent->r.contents = 0;
    v2->ps.pm_flags &= 0xFFF7FFFF;
    if (Entity_IsInRagdoll(ent))
    {
        ent->r.contents = 0x4000000;
    }
    else
    {
        ent->r.svFlags |= 1u;
        ent->s.eType = 6;
    }
    v2->fGunPitch = 0.0f;
    v2->fGunYaw = 0.0f;
    int spectatorClient = ent->client->ps.spectatorClient;
    PlayerState v14;
    if (spectatorClient < 0 || SV_GetCurrentClientInfo(spectatorClient, &v14) == 0)
    {
        StopFollowing(ent);
        return;
    }
    if ((v14.pm_flags & 0x80000) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
        AeAssert::gCurrentLine = 1621;
        AeAssert::gCurrentExpr = "ps.pm_flags & (1<<19)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (((1 << ent->sentient->eTeam) & ent->sentient->noSpectate) != 0)
    {
        StopFollowing(ent);
        return;
    }
    int eFlags = v14.eFlags;
    int viewHeightLerpTime = v2->ps.viewHeightLerpTime;
    float viewHeightTarget = v2->ps.viewHeightTarget;
    float viewHeightCurrent = v2->ps.viewHeightCurrent;
    float viewHeightLerpPosAdj = v2->ps.viewHeightLerpPosAdj;
    int clientNum = v2->ps.viewHeightLerpTarget;
    int viewHeightLerpDown = v2->ps.viewHeightLerpDown;
    v2->ps = v14;
    v2->ps.spectatorClient = spectatorClient;
    v2->ps.viewHeightTarget = viewHeightTarget;
    v2->ps.eFlags = eFlags;
    v2->ps.viewHeightLerpTime = viewHeightLerpTime;
    v2->ps.viewHeightLerpTarget = clientNum;
    v2->ps.viewHeightCurrent = viewHeightCurrent;
    v2->ps.viewHeightLerpPosAdj = viewHeightLerpPosAdj;
    v2->ps.viewHeightLerpDown = viewHeightLerpDown;
    v2->ps.pm_flags = 0x100000 | (v2->ps.pm_flags & 0xFFF7FFFF);
}

// ea: 0x00457810
void Drop_Kit(Entity* pEnt, int iPlayerClass)
{
    math::Dir3 v6;
    v6.v.m128_f32[2] = pEnt->r.currentAngles.v.m128_f32[1];
    v6.v.m128_f32[1] = 0.0f;
    v6.v.m128_f32[3] = 0.0f;
    math::Dir3 vPos;
    AnglesToForward(&v6.v.m128_f32[1], &vPos.v.m128_f32[1]);
    vPos.v.m128_f32[1] *= 150.0f;
    vPos.v.m128_f32[2] *= 150.0f;
    float v3 = ((rand() * 0.000061035156f) - 1.0f) * 50.0f;
    Client* client = pEnt->client;
    vPos.v.m128_f32[3] = (v3 + (vPos.v.m128_f32[3] * 150.0f)) + 200.0f;
    float angles[3];
    angles[1] = pEnt->r.currentOrigin.v.m128_f32[0];
    angles[2] = pEnt->r.currentOrigin.v.m128_f32[1];
    float v8 = ((pEnt->r.maxs.v.m128_f32[2] - pEnt->r.mins.v.m128_f32[2]) * 0.5f)
               + pEnt->r.currentOrigin.v.m128_f32[2];
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
        AeAssert::gCurrentLine = 1373;
        AeAssert::gCurrentExpr = "pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    MultiplayerMgr::MPEntityHandle v10;
    MultiplayerMgr::sInst->GetNextDroppedItemID(&v10, kItemTypeMax, pEnt);
    math::Position3 v5;
    v5.v.m128_f32[0] = vPos.v.m128_f32[1];
    v5.v.m128_f32[1] = vPos.v.m128_f32[2];
    v5.v.m128_f32[2] = vPos.v.m128_f32[3];
    v5.v.m128_f32[3] = 0.0f;
    vPos.v = v5.v;
    math::Dir3 v6b;
    v6b.v.m128_f32[0] = 0.0f;
    v6b.v.m128_f32[1] = v6.v.m128_f32[2];
    v6b.v.m128_f32[2] = 0.0f;
    math::Position3 v5b;
    v5b.v.m128_f32[0] = angles[1];
    v5b.v.m128_f32[1] = angles[2];
    v5b.v.m128_f32[2] = v8;
    v5b.v.m128_f32[3] = 0.0f;
    MultiplayerMgr::sInst->DropItem(3, &v5b, &v6b, &vPos, v10.mVal, false, iPlayerClass);
}

// ea: 0x00467BD0
void G_FindTeams(void)
{
    for (int idx = 0; idx < 0x540; ++idx)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[idx].mObject;
        if (mObject == nullptr)
            continue;
        if (mObject->team.mBlock == nullptr
            || mObject->team.c_str()[0] == 0
            || (mObject->flags & 0x10) != 0
            || (mObject->mClassNameHash.mHash == hash_const.func_tramcar.mHash
                && (mObject->spawnflags & 8) == 0))
        {
            continue;
        }
        mObject->teammaster = mObject;
        for (int j = idx + 1; j < 0x540; ++j)
        {
            Entity* v8 = EntityHandleDb::sInst.mElements[j].mObject;
            if (v8 == nullptr)
                continue;
            if (v8->team.mBlock != nullptr
                && v8->team.c_str()[0] != 0
                && (v8->flags & 0x10) == 0
                && strcmp(mObject->team.c_str(), v8->team.c_str()) == 0)
            {
                v8->teamchain = mObject->teamchain;
                mObject->teamchain = v8;
                v8->teammaster = mObject;
                v8->flags |= 0x10;
                if (v8->mClassNameHash.mHash == hash_const.func_tramcar.mHash)
                    SV_UnlinkEntity(v8);
            }
        }
    }
}

// ea: 0x0048CC10
void G_VehiclePopOut(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7443;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((0x100000 & client->ps.eFlags) != 0)
    {
        Entity* mObject = HandleDbToEnt(player->r.mOwner);
        if (mObject != nullptr)
        {
            if (mObject->scr_vehicle == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 7452;
                AeAssert::gCurrentExpr = "ent->scr_vehicle";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            Entity* driver = HandleDbToEnt(mObject->r.mOwner);
            if (driver != nullptr && (mObject->r.contents & 0x200000) == 0)
            {
                int eFlags = client->ps.eFlags;
                int popout;
                const char* v9;
                if ((0x400000 & eFlags) != 0)
                {
                    client->ps.eFlags = eFlags & 0xFFBFFFFF;
                    popout = mObject->scr_vehicle->boneIndex.player;
                    v9 = "tag_body";
                }
                else
                {
                    client->ps.eFlags = 0x400000 | eFlags;
                    popout = mObject->scr_vehicle->boneIndex.popout;
                    v9 = "tag_popout";
                }
                if (popout < 0)
                    Com_Error(ERR_DROP, "Vehicle %s missing popout tag", v9);
                G_EntUnlink(player);
                DObjSkelMat playerMtx;
                G_DObjGetWorldBoneIndexMatrix(mObject, popout, &playerMtx);
                SetClientOrigin(player, playerMtx.origin);
                if (G_EntLinkToWithOffset(player, mObject, v9, vec3_origin, vec3_origin,
                                          false) == 0)
                    Com_Error(ERR_DROP, "Could not link player to %s", v9);
            }
        }
    }
}

// ea: 0x00466BE0
void G_Animscripted_Think(Entity* ent)
{
    animscripted_t* v2 = ent->scripted;
    if (v2 == nullptr)
        return;
    XAnimTree* v3;
    if (ent->s.eType == 11)
        v3 = G_GetActorAnimTree(ent->actor);
    else if (ent->s.eType == 13)
        v3 = G_GetActorCorpseAnimTree(ent);
    else
        v3 = ent->pAnimTree;
    if (v3 != nullptr && v2->anim != 0)
    {
        if (v2->bStarted != 0)
        {
            if (v2->fBlendOutTime <= 0.0f)
            {
                if (XAnimHasFinished(v3, v2->anim) != 0)
                {
                    ent->flags &= 0xFEFFFFFF;
                    XAnimSetCompleteGoalWeight(v3, v2->anim, 1.0f, 0.0f, 1.0f,
                                               0, 0, nullptr);
                    XAnimSetCompleteGoalWeight(v3, v2->anim, 0.0f, 0.0f, 1.0f,
                                               0, 0, nullptr);
                    v2->anim = 0;
                }
            }
            else
            {
                float len = XAnimGetLength(Scr_GetAnims((int)XAnimGetAnims(v3)), v2->anim);
                if (len <= 0.0f
                    || ((len - v2->fBlendOutTime) / len
                        < XAnimGetTime(v3, v2->anim)))
                {
                    static unsigned char s_init = 0;
                    static unsigned int end_hash = 0;
                    if (!(s_init & 1))
                    {
                        s_init |= 1;
                        end_hash = HashString::CalcHash("end");
                    }
                    HashString h;
                    h.mHash = end_hash;
                    Scr_Notify(ent, h, 0);
                    HashString n;
                    n.mHash = v2->notifyName;
                    Scr_Notify(ent, n, 0);
                    ent->flags &= ~0x1000000u;
                    v2->anim = 0;
                }
            }
        }
        else
        {
            v2->bStarted = 1;
        }
    }
    else
    {
        mem_heap_free(v2);
        ent->scripted = nullptr;
    }
}

// ea: 0x004610F0
void G_Animscripted(Entity* ent, const float* origin, const float* angles,
                    scr_anim_s anim, scr_anim_s root, unsigned int notifyName,
                    int animMode, float fBlendInTime, float fBlendOutTime)
{
    trace_t trace;
    memset(&trace, 0, sizeof(trace));
    XAnimTree* pAnimTree = GScr_GetEntAnimTree(ent);
    if (pAnimTree == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_animscripted.cpp";
        AeAssert::gCurrentLine = 52;
        AeAssert::gCurrentExpr = "pAnimTree";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    animscripted_t* scripted = ent->scripted;
    if (scripted == nullptr)
    {
        scripted = (animscripted_t*)mem_heap_malloc(16, 0x70);
        ent->scripted = scripted;
    }
    scripted->bStarted = 0;
    if (((anim.mHandle >> 16) & 0xFFFF) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_animscripted.cpp";
        AeAssert::gCurrentLine = 66;
        AeAssert::gCurrentExpr = "anim.tree != 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Bad anim passed to anim scripted"))
            __debugbreak();
    }
    memcpy(&scripted->anim, &anim, sizeof(scr_anim_s));
    memcpy(&scripted->root, &root, sizeof(scr_anim_s));
    scripted->mode = animMode;
    scripted->notifyName = notifyName;
    bool has_zone_collision = Entity_has_zone_collision(ent);
    float axis[3][3];
    float baseOrigin[3];
    if (animMode == 1 /* ASM_DEATHPLANT */)
    {
        float v16 = origin[2] + 36.0f;
        float v18 = origin[2] - 18.0f;
        if (has_zone_collision)
        {
            collision_context_t ctx;
            ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
            ctx.pass_entity1 = ent->mHandle;
            ctx.pass_entity2.mHandle.mVal = 0;
            ctx.pass_owner1.mHandle.mVal = 0;
            ctx.pass_owner2.mHandle.mVal = 0;
            ctx.contentmask = 0x820011;
            math::Position3 start;
            start.v.m128_f32[0] = origin[0];
            start.v.m128_f32[1] = origin[1];
            start.v.m128_f32[2] = v16;
            start.v.m128_f32[3] = 0.0f;
            math::Position3 end;
            end.v.m128_f32[0] = origin[0];
            end.v.m128_f32[1] = origin[1];
            end.v.m128_f32[2] = v18;
            end.v.m128_f32[3] = 0.0f;
            SV_Trace(&trace, &start, &actorMins, &actorMaxs, &end, &ctx, 1,
                     0, nullptr, 0, 0.0f);
        }
        if (trace.normal.v.m128_f32[1] < 1.0f && has_zone_collision)
        {
            baseOrigin[0] = trace.endpos.v.m128_f32[0];
            baseOrigin[1] = trace.endpos.v.m128_f32[1];
            baseOrigin[2] = trace.endpos.v.m128_f32[2];
        }
        else
        {
            baseOrigin[0] = origin[0];
            baseOrigin[1] = origin[1];
            baseOrigin[2] = origin[2];
        }
        scripted->fHeightOfs = 0.0f;
        float axisAngles[3] = { 0.0f, angles[1], angles[2] };
        AnglesToAxis(axisAngles, axis);
        G_SetOrigin(ent, baseOrigin);
        G_SetAngle(ent, axisAngles);
        float rot[3];
        float trans[3];
        AnimTree* anims = Scr_GetAnims((anim.mHandle >> 16) & 0xFFFF);
        XAnimGetAbsDelta(anims, anim.mHandle & 0xFFFF, rot, trans, 1.0f);
        float fDelta = sqrtf(trans[0] * trans[0] + trans[1] * trans[1]
                             + trans[2] * trans[2]);
        float transOut[3];
        MatrixTransformVector43(trans, axis, transOut);
        float yaw = vectosignedyaw(rot);
        float yawAxis[3][3];
        YawToAxis(yaw, yawAxis);
        float resultAxis[3][3];
        MatrixMultiply(yawAxis, axis, resultAxis);
        float scriptedAngles[3];
        AxisToAngles(resultAxis, scriptedAngles);
        if (has_zone_collision)
        {
            collision_context_t ctx;
            ctx.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
            ctx.pass_entity1 = ent->mHandle;
            ctx.pass_entity2.mHandle.mVal = 0;
            ctx.pass_owner1.mHandle.mVal = 0;
            ctx.pass_owner2.mHandle.mVal = 0;
            ctx.contentmask = 0x820011;
            math::Position3 start;
            start.v.m128_f32[0] = transOut[0];
            start.v.m128_f32[1] = transOut[1];
            start.v.m128_f32[2] = transOut[2] - fDelta - 128.0f;
            start.v.m128_f32[3] = 0.0f;
            math::Position3 end;
            end.v.m128_f32[0] = transOut[0];
            end.v.m128_f32[1] = transOut[1];
            end.v.m128_f32[2] = transOut[2] + fDelta + 128.0f;
            end.v.m128_f32[3] = 0.0f;
            SV_Trace(&trace, &start, &actorMins, &actorMaxs, &end, &ctx, 1,
                     0, nullptr, 0, 0.0f);
        }
        if (trace.normal.v.m128_f32[1] < 1.0f && has_zone_collision)
        {
            j_nullsub_117(ent->mHandle.mHandle.mVal, 0x820011,
                          &trace.endpos.v.m128_f32[0], angles[1],
                          &scripted->fEndPitch, &scripted->fEndRoll, nullptr);
        }
        else
        {
            scripted->fEndPitch = 0.0f;
            scripted->fEndRoll = 0.0f;
        }
        AnimTree* anims2 = Scr_GetAnims((anim.mHandle >> 16) & 0xFFFF);
        if (XAnimGetLength(anims2, anim.mHandle & 0xFFFF) >= 1.0f)
            scripted->fOrientLerp = 0.0f;
        else
            scripted->fOrientLerp = -1.0f;
    }
    else
    {
        AnglesToAxis(angles, axis);
        baseOrigin[0] = origin[0];
        baseOrigin[1] = origin[1];
        baseOrigin[2] = origin[2];
    }
    math::Mat44 m;
    m.x.v = _mm_setr_ps(axis[0][0], axis[0][1], axis[0][2], 0.0f);
    m.y.v = _mm_setr_ps(axis[1][0], axis[1][1], axis[1][2], 0.0f);
    m.z.v = _mm_setr_ps(axis[2][0], axis[2][1], axis[2][2], 0.0f);
    m.w.v = _mm_setr_ps(baseOrigin[0], baseOrigin[1], baseOrigin[2], 0.0f);
    math::Quaternion quat = nalQuaternionFromMatrix(m);
    scripted->origin.quat = quat;
    scripted->origin.pos.v = m.w.v;
    if ((__fpclass(scripted->origin.pos.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(scripted->origin.pos.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(scripted->origin.pos.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_animscripted.cpp";
        AeAssert::gCurrentLine = 153;
        AeAssert::gCurrentExpr =
            "!IS_NAN((scripted->origin.p)[0]) && "
            "!IS_NAN((scripted->origin.p)[1]) && "
            "!IS_NAN((scripted->origin.p)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    scripted->offset.quat.x = 0.0f;
    scripted->offset.quat.y = 0.0f;
    scripted->offset.quat.z = 0.0f;
    scripted->offset.quat.w = 1.0f;
    scripted->offset.pos.v = _mm_setzero_ps();
    XAnimClearTreeGoalWeightsStrict(pAnimTree, root.mHandle & 0xFFFF,
                                    fBlendInTime);
    AnimTree* anims = Scr_GetAnims((anim.mHandle >> 16) & 0xFFFF);
    int isLooped = XAnimIsLooped(anims, anim.mHandle & 0xFFFF);
    XAnimSetCompleteGoalWeight(pAnimTree, anim.mHandle & 0xFFFF, 1.0f,
                               fBlendInTime,
                               1.0f, notifyName, 0,
                               (void*)(intptr_t)(isLooped == 0));
    scripted->fBlendOutTime = fBlendOutTime;
    ent->flags |= 0x1000000;
}

// ea: 0x00476AE0
int G_InitGame(int brocSys, unsigned int randomSeed, int restart,
               int savegame, int a5)
{
    g_gameIsStartingUp = 1;
    if (restart != 0)
    {
        g_doShellShock[0] = 0;
        if (currCl != NS_CLIENT)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleManager.h";
            AeAssert::gCurrentLine = 24;
            AeAssert::gCurrentExpr = "instance >= 0 && instance < 1";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid index in multiton"))
                __debugbreak();
        }
        RumbleManager_StopMotors(RumbleManager_Inst(currCl));
        Cvar_Set("timescale", "1");
        com_timescale.value = 1.0f;
        SmokeGrenadeMgr_ReInitialize();
        PathNodeMgr::sInst->ValidateAllNodes();
        EntityManager::sInst->DeleteAllEntities();
        SceneManager::sInst->ResetAllStaticModels();
        Entity::FreeAllDObjs(true);
        G_CleanupAnimTrees();
        for (int i = 0; i < 16; ++i)
        {
            if (g_hudelems[i].elem.type != HE_TYPE_FREE)
                HudElem_Free(&g_hudelems[i]);
        }
        memset(g_hudelems, 0, sizeof(g_hudelems));
        gRefEntFreeList.Shutdown();
        gDObjFreeList.Shutdown();
        gDSkelFreeList.Shutdown();
        gDSkelMaxFreeList.Shutdown();
        gDSkel4FreeList.Shutdown();
        gEntFreeList.Shutdown();
        gEntFreeList.Init(350);
        gDSkelFreeList.Init(96);
        gDSkelMaxFreeList.Init(26);
        gDSkel4FreeList.Init(20);
        gDObjFreeList.Init(300);
        gRefEntFreeList.Init(300);
        SceneManager::sInst->ResetAllStaticModels();
        EntityManager::sInst->CreateWorld();
        EntityManager::sInst->CreatePlayers();
        BrocSys::UnloadScript((void*)brocSys);
        PakManager::sInst->ResetPriorities(true);
        SceneManager::sInst->RestartPersistentArray();
        StreamZoneManager::sInst->CheckpointRestart();
        const void* pakInfo =
            PakManager::sInst->GetPakInfo(CurPakId());
        PakManager::sInst->SetUserDistance(pakInfo, 0.0f);
        gCamera[0].Restart();
    }
    CheckpointMgr* cpMgr = CheckpointMgr::sInst;
    if (cpMgr->mCheckpointSaveExists && cpMgr->mUsingCheckpoints)
    {
        int cell = R_CellForPoint(&cpMgr->mOrigin.v.m128_f32[0]);
        StreamZoneManager::sInst->Update(cell, &cpMgr->mOrigin, true);
    }
    else
    {
        StreamZoneManager::sInst->Update(
            StreamZoneManager::sInst->mInitialCell,
            &StreamZoneManager::sInst->mInitialPosition, true);
    }
    for (int i = 0; i < 2; ++i)
    {
        nglWaitForRendering();
        nglSetClearFlags(0xF3);
        char quad[0x80];
        nglInitQuad(quad);
        nglSetQuadColor(quad, 0xFFFFFFFF);
        nglListAddQuad(quad);
        SpinnerDrawFrame(false);
        nglPresent();
    }
    if (restart == 0 && (!cpMgr->mCheckpointSaveExists
                         || !cpMgr->mUsingCheckpoints))
    {
        SpinnerReset();
        goto skip_fill;
    }
    {
        int oldState = cls.state;
        cls.state = CA_LOADING;
        PakManager::sInst->mProgressCallback = (void*)GlobalPakLoadCallback;
        PakManager::sInst->FillBanks();
        PakManager::sInst->mProgressCallback = nullptr;
        cls.state = oldState;
        PathNodeMgr::sInst->CheckpointResetNodes();
        if (restart != 0)
        {
            IGOCompassWidget_SetHideCompassStar(currCl, 0, 0);
            EffectEventSys_StopAll(EffectEventSys::sInst);
            controller_stop_all_rumble(controller_inst());
            CG_ClearHudElems();
            DynamicDecalMgr_DestroyAllDecals();
            WheelMarkMgr_Reset();
            FX_InitFX();
        }
    }
skip_fill:
    DynamicDecalMgr_DestroyAllDecals();
    WheelMarkMgr_Reset();
    Cvar_SetValue("aiIgnoreDbg", 1.0f);
    Cvar_SetValue("aiCanAtkDbg", 0.0f);
    Cvar_SetValue("aiDrawState", 0.0f);
    Cvar_SetValue("aiDrawEnemyLine", 0.0f);
    Cvar_SetValue("aiDrawSight", 0.0f);
    Cvar_SetValue("aiDrawClaimed", 0.0f);
    Cvar_SetValue("aiDrawGrenade", 0.0f);
    Cvar_SetValue("aiPacifistDbg", 0.0f);
    int loadScripts;
    if (restart == 0)
    {
        BrocSys::LoadScript((void*)brocSys);
        Cvar_Set("cl_restartdeath", "0");
        loadScripts = 0;
    }
    else if (savegame == 0)
    {
        loadScripts = 0;
    }
    else
    {
        loadScripts = 1;
    }
    Swap_Init();
    bool pathsInited = level.pathsInited;
    bool pathsInvalid = level.pathsInvalid;
    bool pathsConnected = level.pathsConnected;
    level_locals_t::Clear(&level);
    level.pathsConnected = pathsConnected;
    level.initializing = 1;
    level.pathsInited = pathsInited;
    level.pathsInvalid = pathsInvalid;
    if (savegame != 0)
        loadScripts = 0;
    if (restart == 0)
        memset(&g_scr_data, 0, sizeof(g_scr_data));
    srand(randomSeed);
    Rand_Init(Sys_Milliseconds());
    InitCvars(restart);
    g_freeze_movement = 0;
    gFootSplashEffect = defaultFileName;
    playerMaxs.v.m128_f32[0] = g_bounds_width.value * 0.5f;
    playerMaxs.v.m128_f32[1] = g_bounds_width.value * 0.5f;
    playerMins.v.m128_f32[0] = g_bounds_width.value * -0.5f;
    playerMins.v.m128_f32[1] = g_bounds_width.value * -0.5f;
    playerMaxs.v.m128_f32[2] = g_bounds_height_standing.value;
    if (level.time != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
        AeAssert::gCurrentLine = 1516;
        AeAssert::gCurrentExpr = "!level.time";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (level.iNextObjectiveTime != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
        AeAssert::gCurrentLine = 1517;
        AeAssert::gCurrentExpr = "!level.iNextObjectiveTime";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BG_SetupWeaponInfo();
    g_femanager.IGO->UpdateAfterWeaponsLoaded();
    s_numVehicleInfos = 0;
    ConfigStringManager::sInst->CallbackSearch(CurPakId(), "VEHICLEFILE",
                                               ParseVehicleConfigString);
    ConfigStringManager::sInst->CallbackSearch(
        CurPakId(), "VEHICLEPHYSICSFILE", ParseVehiclePhysicsConfigString);
    sEntryPointHintIndicies[0] = -1;
    dword_DD67B8 = -1;
    dword_DD67BC = -1;
    dword_DD67C0 = -1;
    dword_DD67C4 = -1;
    dword_DD67C8 = -1;
    G_InitialParseInteractionInfo();
    level.numActorCorpses = 0;
    if (loadScripts == 0)
    {
        gpBrocAPI->mBrocExports.mAnimInitialize();
        extern void* AnimBankManager_sInst;  // ?sInst@AnimBankManager@@2PAV1@A
        extern void* AnimBankManager_GetBank(void* self, TPakId pakId);
        extern int AnimBank_anims_mSize(void* bank);
        g_xanim_num =
            AnimBank_anims_mSize(AnimBankManager_GetBank(
                AnimBankManager_sInst, (TPakId)kPakTypeLevel));
        GScr_LoadScriptsAndAnimsForEntities();
        Scr_PrecacheAnimTrees(Hunk_AllocXAnimCreate, restart != 0);
        AnimTree* generic = Scr_GetAnimTreeByName("generic_human");
        if (generic == nullptr)
            G_Error("Could not find animation tree '%s'", "generic_human");
        g_scr_data.generic_human_tree = generic;
        for (int i = 0; i < 16; ++i)
            g_scr_data.actorCorpseInfo[i].mEntity.mHandle.mVal = 0;
    }
    GScr_LoadConsts();
    level.maxclients = 16;
    for (int i = 0; i < 16; ++i)
    {
        g_clients[i].Clear(true, true);
        g_clients[i].mServerClientIndex = i;
    }
    level.clients = g_clients;
    level.sentients = g_sentients;
    G_InitSentients();
    for (int i = 0; i < level.maxclients; ++i)
        EntityManager::sInst->mPlayers[i]->client = &level.clients[i];
    if (restart == 0)
    {
        ConfigStringManager::sInst->CallbackSearch(
            CurPakId(), "MPLOCDMGTABLE", ParseHitLocDmgTableEntry);
        memset(itemRegistered, 0, 137 * sizeof(int));
        itemRegistered[0] = 1;
    }
    Path_Init();
    s_numNodes = 0;
    G_InitScrVehicles();
    turretInfo[0].inuse = 0;
    level.turrets = turretInfo;
    level.loading = (savegame != 0) + 1;
    vehicle_InitDynamicBuffers(10);
    SceneManager::sInst->InstanceEntities();
    level.loading = 0;
    CheckpointMgr::sInst->RestoreExplodedExploders();
    MultiplayerMgr::sInst->LevelLoaded();
    MP_ResolveAnims();
    G_SetupVehiclePaths(0.0f);
    G_SetupScrVehicles();
    G_FindTeams();
    level.bRegisterItems = 1;
    level.snapTime = level.time;
    SaveRegisteredItems();
    level.bDrawCompassFriendlies = 1;
    level.fFogOpaqueDist = 3.4028235e38f;
    level.fFogOpaqueDistSqrd = 3.4028235e38f;
    for (int i = 0; i < 17; ++i)
        SV_SetConfigstring(i + 16, nullptr);
    level.MissleOnlyActiveForTime = 0.0f;
    level.initializing = 0;
    CheckpointMgr::sInst->Restart();
    LensFlareInit();
    g_gameIsStartingUp = 0;
    return 0;
}

// ea: 0x004505A0
void SP_worldspawn(void)
{
    static unsigned char s_init = 0;
    static unsigned int ambienttrack_hash = 0;
    static unsigned int message_hash = 0;
    static unsigned int gravity_hash = 0;
    static unsigned int northyaw_hash = 0;
    const char* s = nullptr;
    G_SpawnString(classname_hash.mHash, defaultFileName, &s);
    if (Q_stricmp(s, "worldspawn") != 0)
        G_Error("SP_worldspawn: The first entity isn't 'worldspawn'");
    SV_SetConfigstring(2, "cod-sp");
    if (!(s_init & 1))
    {
        s_init |= 1;
        ambienttrack_hash = HashString::CalcHash("ambienttrack");
    }
    G_SpawnString(ambienttrack_hash, defaultFileName, &s);
    if (*s != 0)
        SV_SetConfigstring(3, va("n\\%s", s));
    else
        SV_SetConfigstring(3, defaultFileName);
    if (!(s_init & 2))
    {
        s_init |= 2;
        message_hash = HashString::CalcHash("message");
    }
    G_SpawnString(message_hash, defaultFileName, &s);
    SV_SetConfigstring(4, s);
    if (!(s_init & 4))
    {
        s_init |= 4;
        gravity_hash = HashString::CalcHash("gravity");
    }
    G_SpawnString(gravity_hash, "800", &s);
    Cvar_Set("g_gravity", s);
    if (!(s_init & 8))
    {
        s_init |= 8;
        northyaw_hash = HashString::CalcHash("northyaw");
    }
    G_SpawnString(northyaw_hash, defaultFileName, &s);
    if (*s != 0)
        SV_SetConfigstring(11, s);
    else
        SV_SetConfigstring(11, "0");
    Entity* mWorld = EntityManager::sInst->mWorld;
    mWorld->mClassName = "worldspawn";
    mWorld->mClassNameHash.mHash = HashString::CalcHash(mWorld->mClassName.c_str());
    UpdateEntityHash(mWorld);
}

// ea: 0x0046E310
bool G_GetTankIndex(DbLinkedHandle<EntityHandleDb, Entity> entity, int* index,
                    bool* enemy)
{
    Entity* vehicle = HandleDbToEnt(entity);
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7494;
        AeAssert::gCurrentExpr = "*entity != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (level.bDrawCompassFriendlies == 0)
        return false;
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (scr_vehicle == nullptr)
        return false;
    if (vehicle->health <= 0)
        return false;
    if (HandleDbToEnt(scr_vehicle->mEntity) == nullptr)
        return false;
    vehicle_info_t* v7 = s_vehicleInfos[scr_vehicle->infoIdx];
    int type = v7->type;
    if (type != 2 && type != 1)
        return false;
    if (index != nullptr)
        *index = (int)(scr_vehicle - level.vehicles);
    if (v7->type != 2)
        return false;
    if (G_GetVehicleOccupantCount(vehicle) == 0)
        return false;
    Entity* driver = HandleDbToEnt(vehicle->r.mOwner);
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    sentient_s* sentient = Player->sentient;
    if (sentient == nullptr)
        return false;
    bool v20 = true;
    if (driver != nullptr)
    {
        if (driver->sentient == nullptr)
            return false;
        v20 = sentient->eTeam == driver->sentient->eTeam;
    }
    for (int v13 = 0; v13 < 11; ++v13)
    {
        Entity* occupant = HandleDbToEnt(scr_vehicle->seats[v13].occupant);
        if (occupant != nullptr)
        {
            if (Player == occupant)
                return false;
            if (driver == nullptr && sentient->eTeam != occupant->sentient->eTeam)
                v20 = false;
        }
    }
    if (v20)
    {
        if (enemy != nullptr)
            *enemy = false;
        return true;
    }
    if (IsVehicleSpotted(vehicle))
    {
        if (enemy != nullptr)
            *enemy = true;
        return true;
    }
    return false;
}

// ea: 0x00481930
void Weapon_RocketLauncher_Fire(Entity* ent, float spread, weaponParms* wp,
                                float lifetime, bool explode)
{
    float fAimOffset = 0.0f;
    float dir[3];
    dir[0] = tanf(spread * 3.1415927f * 0.0055555557f) * 16.0f;
    gunrandom(&dir[2], &dir[1]);
    float v6 = wp->forward[1] * 16.0f;
    float v7 = wp->forward[0] * 16.0f;
    float v8 = wp->forward[2] * 16.0f;
    dir[2] = dir[2] * dir[0];
    float v10 = (wp->right[0] * dir[2]) + v7;
    float v11 = (wp->right[1] * dir[2]) + v6;
    float v12 = wp->right[2] * dir[2];
    float launchpos[3];
    launchpos[0] = (wp->up[0] * (dir[1] * dir[0])) + v10;
    launchpos[1] = (wp->up[1] * (dir[1] * dir[0])) + v11;
    launchpos[2] = (wp->up[2] * (dir[1] * dir[0])) + (v12 + v8);
    dir[1] = dir[1] * dir[0];
    VectorNormalize(launchpos);
    float start[3] = { wp->muzzleTrace[0], wp->muzzleTrace[1], wp->muzzleTrace[2] };
    Entity* v14 = fire_rocket(ent, start, launchpos, lifetime);
    if (!explode && v14 != nullptr)
        v14->think = THINK__G_FreeEntity;
    Client* client = ent->client;
    if (client != nullptr)
    {
        client->ps.velocity.v.m128_f32[0] -= wp->forward[0] * 64.0f;
        ent->client->ps.velocity.v.m128_f32[1] -= wp->forward[1] * 64.0f;
        ent->client->ps.velocity.v.m128_f32[2] -= wp->forward[2] * 64.0f;
    }
    math::Dir3 dirv;
    dirv.v.m128_f32[0] = launchpos[0];
    dirv.v.m128_f32[1] = launchpos[1];
    dirv.v.m128_f32[2] = launchpos[2];
    math::Position3 posv;
    posv.v.m128_f32[0] = start[0];
    posv.v.m128_f32[1] = start[1];
    posv.v.m128_f32[2] = start[2];
    MultiplayerMgr::MPEntityHandle h;
    h.mVal = 0;
    MultiplayerMgr::sInst->FireMissile(ent->s.weapon, posv, dirv, h);
}

// ea: 0x00481E00
void Weapon_ArtilleryStrike_Launch(Entity* ent, weaponParms* wp, float* center,
                                   unsigned int seed)
{
    weaponFileInfo_t* pWeapInfo = wp->pWeapInfo;
    int delay = pWeapInfo->iProjectileDelay;
    if (pWeapInfo->iProjectileCount > 15)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 2004;
        AeAssert::gCurrentExpr = "wp->pWeapInfo->iProjectileCount <= 15";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("To many artillery shells in weapon"))
            __debugbreak();
    }
    int delays[15] = { 0 };
    int dists[15] = { 0 };
    int angles[15] = { 0 };
    bdRandomState rng;
    bdRandom_setSeed(&rng, seed);
    int angle = 0;
    float v26 = 0.0f;
    int v27 = 0;
    int v8 = pWeapInfo->iProjectileRadius;
    int delayRange = pWeapInfo->iProjectileSpacingMax - pWeapInfo->iProjectileSpacingMin;
    unsigned int v9 = 2 * v8;
    for (int v6 = 0; v6 < pWeapInfo->iProjectileCount; ++v6)
    {
        angles[v6] = (int)(bdRandom_nextUInt(&rng) % 0x168);
        dists[v6] = (int)(bdRandom_nextUInt(&rng) % v9) - pWeapInfo->iProjectileRadius;
        delays[v6] = pWeapInfo->iProjectileSpacingMin
                     + (int)(bdRandom_nextUInt(&rng) % delayRange);
    }
    Scr_Notify(ent, hash_const.fireSpecial, 0);
    for (int v12 = 0; v12 < pWeapInfo->iProjectileCount; ++v12)
    {
        angle = angles[v12];
        float offset[3];
        AnglesToForward((const float*)&angle, offset);
        float end[3];
        end[0] = (dists[v12] * offset[0]) + center[0];
        end[1] = (dists[v12] * offset[1]) + center[1];
        end[2] = (dists[v12] * offset[2]) + center[2];
        fire_artillery(ent, end, delay);
        delay = delays[v12] + delay;
    }
}
