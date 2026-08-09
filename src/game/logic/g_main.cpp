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
    if ((mFlags.mMask & 4) == 0)
        G_FreeEntity(e, 0);
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
