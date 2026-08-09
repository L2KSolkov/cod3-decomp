// ============================================================================
// g_main.cpp - game entry + console commands (g.o: g_main.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <stdlib.h>
#include <string.h>

#include "render/ShaderCommon.h"
#include "ngl/nglDebug.h"
#include "core/PoolAllocator.h"

extern PoolAllocator* gCommonPoolAllocator;  // core.o 0x012EFF18

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
