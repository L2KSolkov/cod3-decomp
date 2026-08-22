// ============================================================================
// cl_parse.cpp - client command/input/parse tail (cl.o cl_main.cpp etc.)
// 11 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"
#include "game/game_types.h"

#include <string.h>
#include <intrin.h>

class nglFont;

// Minimal view of SoundDevice (full class in game/sv/sv_stubs.h).
class SoundDevice { public: static SoundDevice* sInst; void FrameAdvance(float delta); };


// Minimal views (full classes in game/sv/sv_stubs.h / g_local.h).
class PakManager {
public:
    static PakManager* sInst;
    void Update(bool calledFromMovie);
};  // ?sInst@PakManager@@2PAV1@A
class EffectEventSys { public: static EffectEventSys* sInst; };  // ?sInst@EffectEventSys@@2PAV1@A


// Minimal view of RumbleManager (full class in core/core_systems.h).
class RumbleManager {
public:
    static RumbleManager* Inst(int instance);  // ?Inst@RumbleManager@@SAPAV1@H@Z (g.o)
};


struct netchan_t;
class EntityManager {
public:
    static EntityManager* sInst;
    Entity* GetPlayer(int idx);
};
extern Entity* GetPlayer(int idx);
struct ClientFrameView {
    unsigned char _pad_ps[0x550];
    int spectatorClient;  // Client::ps.spectatorClient @ +0x550
    unsigned char _pad_pers[0x220];
    int playerState;      // Client::pers.playerState @ +0x774
};
static_assert(offsetof(ClientFrameView, spectatorClient) == 0x550,
              "Client::ps.spectatorClient offset mismatch");
static_assert(offsetof(ClientFrameView, playerState) == 0x774,
              "Client::pers.playerState offset mismatch");
// ============================================================================
// Externs (core.o / cl.o)
// ============================================================================
extern void Com_Printf(const char* fmt, ...);
enum errorParm_t;
extern void Com_Error(errorParm_t code, const char* fmt, ...);
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);
extern void Cbuf_AddText(const char* text);
extern void Cmd_AddCommand(const char* cmd_name, void (*function)());
extern void Cmd_AddInputCommand(const char* cmd_name, void (*function)(int, int));
extern void Cvar_Set(const char* var_name, const char* value);
extern struct cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                               int flags);
extern const char defaultFileName[];
extern int com_frameTime;
int old_com_frameTime = 0;  // ?old_com_frameTime@@3HA (cl.o @ 0xF0D1B4)
extern int dword_F6A290[4 * 802];  // defined in effect_events.cpp (core.o)

unsigned int frame_msec;  // ?frame_msec@@3IA (cl.o @ 0x12FC6D4)
extern int anykeydown;
extern int dword_F170F8;
int dword_F0D1F4[2];   // cl.o BSS
int dword_F0D1F8[2];   // cl.o BSS
int dword_F0F1FC[2];   // cl.o BSS
int dword_F0F200[2];   // cl.o BSS
char** svc_strings;    // ?svc_strings@@3PAPAD (cl.o)
char dest[128];        // cl.o BSS
char byte_F0D1FC[4 * 19528];  // cl.o BSS (server command buffers)
extern void CL_SystemInfoChanged();
int dword_F6A28C;  // ?dword_F6A28C@@3HA (cl.o active port scalar)
extern int lFirstLocalClientIndex;
extern void MSG_Init(struct msg_t* msg, unsigned char* data, int length);
extern void MSG_WriteLong(struct msg_t* msg, int c);
extern void MSG_WriteByte(struct msg_t* msg, int c);
extern void MSG_WriteString(struct msg_t* msg, const char* s);
extern void Netchan_Transmit(netchan_t* chan, int length,
                             const unsigned char* data);
extern void CL_AdjustAngles();
extern void CL_CmdButtons(usercmd_s* cmd);
extern void CL_KeyMove(usercmd_s* cmd);
extern void CL_GamepadMove(usercmd_s* cmd);
extern void CL_FinishMove(usercmd_s* cmd);
extern void CL_AddReliableCommand(const char* cmd);
extern void Field_AdjustScroll(field_t* edit);
extern void Field_Clear(field_t* edit);
extern int CL_ClearState();
extern void CL_StartHunkUsers();
extern void GamePause_SetAllPaused(bool paused);
extern float Com_GetScreenTimeDelta();
extern void SCR_UpdateScreen(float screen_time_inc);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern void CompleteCommand();
extern void SoundDevice_StopAllSounds(void* self);
extern void SoundDevice_UndampenAllSounds(void* self);
extern struct cvar_t* cl_showSend;
extern struct cvar_t* cl_nodelta;
extern struct cvar_t* cl_debugMove;
extern int com_time;
extern int Sys_Milliseconds();
extern "C" int atoi(const char* nptr);
extern void Cvar_SetCheatState();
extern void nullsub_16(const char* pakSums, const char* pakNames);
extern void nullsub_34(const char* pakSums, const char* pakNames);
extern char* Info_ValueForKey(const char* s, const char* key);
extern struct vm_s { int (__cdecl* systemCall)(int*); }* cgvm;
extern int VM_Call(struct vm_s* vm, int callnum, ...);
extern struct vm_s* VM_Create(const char* module,
                              int (__cdecl* systemCalls)(int*));
extern int CL_CgameSystemCalls(int* args);
enum fsMode_t;
extern void tlPrintf(const char* fmt, ...);
extern int MSG_ReadLong(struct msg_t* msg);
extern unsigned char MSG_ReadByte(struct msg_t* msg);
extern char* MSG_ReadString(struct msg_t* msg);
extern int dword_F0F204[2];
char byte_F0F208[4 * 19528];  // cl.o BSS
extern int dword_F0D1F4[2];
extern int dword_F0D1F8[2];
extern int dword_F0F1FC[2];
extern int dword_F0F200[2];
extern char byte_F0D1FC[];
// netadr_t (20 bytes; same layout as sv/server_types.h, local to avoid
// pulling the full server type set into the cl TU)
struct netadr_t {
    int      type;        // +0x00
    uint8_t  ip[4];       // +0x04
    uint8_t  ipx[10];     // +0x08
    uint16_t port;        // +0x12
};
static_assert(sizeof(netadr_t) == 0x14, "netadr_t size mismatch");
extern int NET_CompareAdr(netadr_t a, netadr_t b);
extern int Netchan_Process(netchan_t* chan, struct msg_t* msg);
extern const char* NET_AdrToString(netadr_t a);
extern void Com_DPrintf(const char* fmt, ...);
extern void CL_Netchan_Transmit(netchan_t* chan, struct msg_t* msg);
extern void SendClientThinkMsg();
enum nflState : unsigned;
extern nflState codNflUpdate();
extern void AudioBankMgr_Update(void* self);
extern void* AudioBankMgr_sInst;
extern void PakManager_Update(void* self, bool calledFromMovie);
extern void MusicMgr_Update(void* self, float dt);
extern void* MusicMgr_sInst;
extern void EffectEventSys_FrameAdvance(void* self, float delta);
extern void SceneManager_UpdateEffects(void* self, float delta_t);
extern void* SceneManager_sInst;
extern void EntityHandleDb_Compact(void* self);
class EntityHandleDb {
public:
    static EntityHandleDb sInst;  // ?sInst@EntityHandleDb@@0V1@A (g.o)
};
extern void StreamZoneManager_Update(void* self, int cellNum,
                                     const float* pos, bool forceReset);
extern void* StreamZoneManager_sInst;
extern void RumbleManager_FrameAdvance(void* self, float delta_time);
extern int R_CellForPoint(const float* pos);
extern void Con_RunConsole();
bool gUseControllerLagFix;
extern bool g_controllerConnectedErrorShown[];
extern int dword_F170F0;
extern int dword_F170FC;
int dword_F170EC;
extern int lFirstLocalClientIndex;
extern int lLastLocalClientIndex;
extern struct cvar_t* cl_avidemo;
extern struct cvar_t* cl_forceavidemo;
extern struct cvar_t* com_timescale;
extern struct cvar_t* com_sv_running;
extern void Cbuf_ExecuteText(int exec_when, const char* text);

struct controller_view {
    int locked_port;
    static controller_view* inst();
};
controller_view* controller_view::inst()
{
    static controller_view s = {};
    return &s;
}
struct trGlobals_t {
    unsigned char _pad[0x290];
    void* world;
};
extern trGlobals_t tr;

struct cdl_proftimer {
    unsigned __int64 stamp;
    unsigned __int64 value;
    void start() { stamp = __rdtsc(); }
    void stop() { value += __rdtsc() - stamp; }
};
extern cdl_proftimer cdl_proftimer_draw;
extern cdl_proftimer cdl_proftimer_audio;
extern cdl_proftimer cdl_proftimer_streaming;
extern cdl_proftimer cdl_proftimer_pak_mgr;
extern cdl_proftimer cdl_proftimer_music_mgr;
extern cdl_proftimer cdl_proftimer_effect_sys;
extern cdl_proftimer cdl_proftimer_rumble_mgr;
extern cdl_proftimer cdl_proftimer_scn_effect;
extern cdl_proftimer cdl_proftimer_entities;

// cls.configstrings (Broc::string[1024])
Broc::string cls_configstrings[1024];  // cl.o BSS

// snapshot ring (cl_snapshot.cpp view)
struct clSnapshotEntry2 {
    int valid;
    int snapFlags;
    int serverCommandNum;
    int serverTime;
    int parseEntitiesNum;
    unsigned char ps[1024];
};
clSnapshotEntry2 cl_snapshots[2][4];  // ?cl_snapshots@@3PAY03UclSnapshotEntry2@@A (cl.o)

// Renderer export/import interfaces (cl.o cl_main.cpp)
struct refimport_t {
    void (*Printf)(int, const char*, ...);
    void (*Error)(errorParm_t, const char*, ...);
    int (*Milliseconds)();
    void* (*Hunk_AllocInternal)(int);
    void* (*Hunk_AllocateTempMemoryInternal)(int);
    void* (*Z_MallocInternal)(int);
    void (*Z_FreeInternal)(void*);
    void (*Hunk_FreeTempMemory)(void*);
    struct cvar_t* (*Cvar_Get)(const char*, const char*, int);
    struct cvar_t* (*Cvar_FindVar)(const char*);
    void (*Cvar_Set)(const char*, const char*);
    void (*Cmd_AddCommand)(const char*, void (*)());
    void (*Cmd_RemoveCommand)(const char*);
    int (*Cmd_Argc)();
    char* (*Cmd_Argv)(int);
    void (*Cmd_ExecuteText)(int, const char*);
    int (*Com_SaveCvarsToBuffer)(const char**, int, char*, int);
    int (*Com_LoadCvarsFromBuffer)(const char**, int, const char*,
                                   const char*);
    int (*FS_FileIsInPAK)(const char*, int*);
    int (*FS_ReadFile)(const char*, void**);
    void (*FS_FreeFile)(void*);
    char** (*FS_ListFiles)(const char*, const char*, int*);
    void (*FS_FreeFileList)(char**);
    void (*FS_WriteFile)(const char*, const void*, int);
    int (*FS_FileExists)(const char*);
    int (*FS_FOpenFileByMode)(const char*, int*, enum fsMode_t);
    void (*FS_FCloseFile)(int);
    int (*FS_Read)(void*, int, int);
    int (*FS_Write)(const void*, int, int);
    class BspPlane* (*CM_GetPlaneNum)(int);
    int (*CG_GetGameModel)(short);
    void (*CG_DObjCalcPose)(void*, void*, int*);
    void (*AdjustFrom640)(float*, float*, float*, float*);
    void* (*UI_GetFontInfo)(int, float);
};
struct refexport_t2 {
    void (*Shutdown)(int);
    void (*BeginRegistration)(void*);
    void* (*RegisterModel)(void* result, const char*, int, int);
    int (*RegisterShader)(const char*, int);
    int (*RegisterShaderNoMip)(const char*, int);
    void (*LoadWorld)(const char*, int*);
    void (*SetFXImageMemory)(int);
    int (*GetFXImageMemory)();
    int (*GetImageMemory)();
    float (*GetFarPlaneDist)();
    void (*EndRegistration)();
    void (*ClearScene)();
    void (*AddPolyToScene)(void*, int, const void*);
    void (*AddLightToScene)(const float*, float, float, float, float);
    void (*SetCullDist)(float);
    void (*SetFog)(int, int, int, float, float, float, float);
    void (*RenderScene)(const void*);
    void (*ClearFlares)();
    void (*SetColor)(const float*);
    void (*DrawStretchPic)(float, float, float, float, float, float,
                           float, float, void*);
    void (*DrawStretchPicGradient)(float, float, float, float, float,
                                   float, float, float, void*,
                                   const float*, int);
    void (*DrawStretchPicRotate)(float, float, float, float, float,
                                 float, float, float, float, void*);
    void (*DrawQuadPic)(const float (*)[2], const float (*)[2], void*);
    void (*DrawStretchRaw)(int, int, int, int, int, int,
                           const unsigned char*, int, int);
    void (*UploadCinematic)(int, int, int, int, const unsigned char*,
                            int, int);
    void (*BeginFrame)();
    void (*EndFrame)(int*, int*);
    void (*SaveScreen)();
    void (*TrackStatistics)(void*);
    int (*PickShader)(const float*, const float*, char*, char*, char*, int);
    void (*ResetImageAllocations)();
    void (*FreeImageAllocations)();
    void (*CubemapShot)(const char*, int, int, float, float);
    void (*CubemapWaterShot)(const char*, int, int, float*, float*);
    void (*LocateDebugStrings)(void*, int);
    void (*LocateDebugLines)(void*, int);
    int (*Text_Width)(const char*, int, float, float, int);
    int (*Text_Height)(int, float);
    void (*Text_Paint)(float, float, int, float, const float*,
                       const char*, float, int, int);
    int (*Text_ConsoleWidth)(const short*, int, float, float, int);
    void (*Text_ConsolePaint)(float, float, int, float, const float*,
                              const short*, float, int, int);
    void (*Text_PaintWithCursor)(float, float, int, float, const float*,
                                 const char*, int, char, float, int, int);
};
struct refexport_t;

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

// msg_t (message buffer, from server_types)
struct msg_t {
    int overflowed;
    unsigned char* data;
    int maxsize;
    int cursize;
    int readcount;
};

// KeyInfo binding entry state (low 2 bits = mDown, high 30 = mRepeats)
struct KeyInfoEntry3 {
    int mState;
    char* mBoundCmdName;
};
extern KeyInfoEntry3 KeyInfo_mKeys[2][256];

// ============================================================================
// Command generation
// ============================================================================

// ea: 0x5311E0
usercmd_s CL_CreateCmd()
{
    float oldAngles[3];
    oldAngles[0] = cl[currCl].viewangles[0];
    CL_AdjustAngles();
    usercmd_s cmd;
    memset(&cmd, 0, sizeof(cmd));
    if (cl_freeze->integer == 0)
    {
        CL_CmdButtons(&cmd);
        CL_KeyMove(&cmd);
        CL_GamepadMove(&cmd);
        if (cl_viewPitchCompensate->value != 0.0f)
        {
            cl[currCl].viewangles[0] += cl_viewPitchCompensate->value;
            Cvar_Set("cl_viewPitchCompensate", "0");
        }
        if (cl_viewYawCompensate->value != 0.0f)
        {
            cl[currCl].viewangles[1] += cl_viewYawCompensate->value;
            Cvar_Set("cl_viewYawCompensate", "0");
        }
        float* viewangles = cl[currCl].viewangles;
        if (*viewangles - oldAngles[0] > 90.0f)
        {
            *viewangles = oldAngles[0] + 90.0f;
        }
        else if (oldAngles[0] - *viewangles > 90.0f)
        {
            *viewangles = oldAngles[0] - 90.0f;
        }
    }
    CL_FinishMove(&cmd);
    return cmd;
}

// ea: 0x531350
void CL_CreateNewCommands()
{
    if (cls.state == 2)  // CA_ACTIVE
    {
        if (currCl == lFirstLocalClientIndex)
        {
            frame_msec = com_frameTime - old_com_frameTime;
            if (com_frameTime - old_com_frameTime > 100)
                frame_msec = 100;
            old_com_frameTime = com_frameTime;
        }
        int v0 = cl[currCl].cmdNumber + 1;
        cl[currCl].cmdNumber = v0;
        cl[currCl].cmds[v0 & 0x3F] = CL_CreateCmd();
    }
}

// ea: 0x531400
void CL_WritePacket()
{
    if (cls.state != 3 && cls.state != 4)  // CA_CINEMATIC / CA_LOGO
    {
        usercmd_s nullcmd;
        unsigned char data[3072];
        msg_t buf;
        memset(&nullcmd, 0, sizeof(nullcmd));
        MSG_Init(&buf, data, 3072);
        MSG_WriteLong(&buf, cl[currCl].serverId);
        MSG_WriteLong(&buf, dword_F0F1FC[4882 * currCl]);
        MSG_WriteLong(&buf, dword_F0F200[4882 * currCl]);
        for (int i = dword_F0D1F8[4882 * currCl] + 1;
             i <= dword_F0D1F4[4882 * currCl]; ++i)
        {
            MSG_WriteByte(&buf, 2);
            MSG_WriteLong(&buf, i);
            MSG_WriteString(&buf,
                            &byte_F0D1FC[19528 * currCl + 128 * (i & 0x3F)]);
        }
        cl[currCl].outPackets[0].p_realtime = dword_F170F8;
        cl[currCl].outPackets[0].p_serverTime = nullcmd.serverTime;
        cl[currCl].outPackets[0].p_cmdNumber = cl[currCl].cmdNumber;
        if (cl_showSend->integer != 0)
            Com_Printf("%i ", buf.cursize);
        MSG_WriteByte(&buf, 3);
        Netchan_Transmit((netchan_t*)((char*)0xF11208 + 19528 * currCl),
                         buf.cursize, buf.data);
    }
}

// ea: 0x531590
void CL_SendCmd()
{
    if (cls.state == 2)
    {
        CL_CreateNewCommands();
        CL_WritePacket();
    }
}

// ============================================================================
// Input / key commands
// ============================================================================

// ea: 0x534140
void CL_InitInput()
{
    Cmd_AddCommand("centerview", (void(__cdecl*)())IN_CenterView);
    Cmd_AddInputCommand("+moveup", IN_UpDown);
    Cmd_AddInputCommand("-moveup", IN_UpUp);
    Cmd_AddInputCommand("+movedown", IN_DownDown);
    Cmd_AddInputCommand("-movedown", IN_DownUp);
    Cmd_AddInputCommand("+left", IN_LeftDown);
    Cmd_AddInputCommand("-left", IN_LeftUp);
    Cmd_AddInputCommand("+right", IN_RightDown);
    Cmd_AddInputCommand("-right", IN_RightUp);
    Cmd_AddInputCommand("+forward", IN_ForwardDown);
    Cmd_AddInputCommand("-forward", IN_ForwardUp);
    Cmd_AddInputCommand("+back", IN_BackDown);
    Cmd_AddInputCommand("-back", IN_BackUp);
    Cmd_AddInputCommand("+lookup", IN_LookupDown);
    Cmd_AddInputCommand("-lookup", IN_LookupUp);
    Cmd_AddInputCommand("+lookdown", IN_LookdownDown);
    Cmd_AddInputCommand("-lookdown", IN_LookdownUp);
    Cmd_AddInputCommand("+strafe", IN_StrafeDown);
    Cmd_AddInputCommand("-strafe", IN_StrafeUp);
    Cmd_AddInputCommand("+moveleft", IN_MoveleftDown);
    Cmd_AddInputCommand("-moveleft", IN_MoveleftUp);
    Cmd_AddInputCommand("+moveright", IN_MoverightDown);
    Cmd_AddInputCommand("-moveright", IN_MoverightUp);
    Cmd_AddInputCommand("+speed", IN_SpeedDown);
    Cmd_AddInputCommand("-speed", IN_SpeedUp);
    Cmd_AddInputCommand("+holdbreath", IN_HoldBreathDown);
    Cmd_AddInputCommand("-holdbreath", IN_HoldBreathUp);
    Cmd_AddInputCommand("+attack", IN_Button0Down);
    Cmd_AddInputCommand("-attack", IN_Button0Up);
    Cmd_AddInputCommand("+melee", IN_Button5Down);
    Cmd_AddInputCommand("-melee", IN_Button5Up);
    Cmd_AddInputCommand("+activate", IN_ActivateDown);
    Cmd_AddInputCommand("-activate", IN_ActivateUp);
    Cmd_AddInputCommand("+class", IN_ClassButtonDown);
    Cmd_AddInputCommand("-class", IN_ClassButtonUp);
    Cmd_AddInputCommand("+smokegrenade", IN_SmokeGrenadeAttackDown);
    Cmd_AddInputCommand("-smokegrenade", IN_SmokeGrenadeAttackUp);
    Cmd_AddInputCommand("+binoculars", IN_BinocularsDown);
    Cmd_AddInputCommand("-binoculars", IN_BinocularsUp);
    Cmd_AddInputCommand("+reload", IN_ReloadDown);
    Cmd_AddInputCommand("-reload", IN_ReloadUp);
    Cmd_AddInputCommand("+leanleft", IN_LeanLeftDown);
    Cmd_AddInputCommand("-leanleft", IN_LeanLeftUp);
    Cmd_AddInputCommand("+leanright", IN_LeanRightDown);
    Cmd_AddInputCommand("-leanright", IN_LeanRightUp);
    Cmd_AddInputCommand("+prone", IN_Wbutton6Down);
    Cmd_AddInputCommand("-prone", IN_Wbutton6Up);
    Cmd_AddInputCommand("+sprint", IN_SprintDown);
    Cmd_AddInputCommand("-sprint", IN_SprintUp);
    Cmd_AddInputCommand("+sprintbreath", IN_SprintBreathDown);
    Cmd_AddInputCommand("-sprintbreath", IN_SprintBreathUp);
    Cmd_AddInputCommand("toggle cl_run", (void(__cdecl*)(int, int))IN_ToggleADS);
    Cmd_AddCommand("+mlook", IN_MLookDown);
    Cmd_AddCommand("-mlook", (void(__cdecl*)())IN_MLookUp);
    Cmd_AddCommand("lowerstance", (void(__cdecl*)())IN_LowerStance);
    Cmd_AddCommand("raisestance", (void(__cdecl*)())IN_RaiseStance);
    Cmd_AddCommand("togglecrouch", (void(__cdecl*)())IN_ToggleCrouch);
    Cmd_AddCommand("toggleprone", (void(__cdecl*)())IN_ToggleProne);
    Cmd_AddCommand("goprone", (void(__cdecl*)())IN_GoProne);
    Cmd_AddCommand("gocrouch", (void(__cdecl*)())IN_GoCrouch);
    Cmd_AddInputCommand("+gostand", IN_GoStandDown);
    Cmd_AddInputCommand("-gostand", IN_GoStandUp);
    Cmd_AddCommand("+stance", (void(__cdecl*)())IN_Stance_Down);
    Cmd_AddCommand("-stance", (void(__cdecl*)())IN_Stance_Up);
    Cmd_AddInputCommand("+leanrightswitchnext", IN_LeanRightSwitchNextDown);
    Cmd_AddInputCommand("-leanrightswitchnext", IN_LeanRightSwitchNextUp);
    Cmd_AddInputCommand("+leanleftswitchnext", IN_LeanLeftSwitchNextDown);
    Cmd_AddInputCommand("-leanleftswitchnext", IN_LeanLeftSwitchNextUp);
    Cmd_AddInputCommand("+grenadeattack", IN_GrenadeAttackDown);
    Cmd_AddInputCommand("-grenadeattack", IN_GrenadeAttackUp);
    Cmd_AddInputCommand("+activatemoveup", IN_ActivateMoveUpDown);
    Cmd_AddInputCommand("-activatemoveup", IN_ActivateMoveUpUp);
    Cmd_AddInputCommand("+activatemelee", IN_ActivateMeleeDown);
    Cmd_AddInputCommand("-activatemelee", IN_ActivateMeleeUp);
    Cmd_AddInputCommand("+activatereload", IN_ActivateReloadDown);
    Cmd_AddInputCommand("-activatereload", IN_ActivateReloadUp);
    Cmd_AddCommand("+analogsticklean", IN_AnalogStickLeanDown);
    Cmd_AddCommand("-analogsticklean", IN_AnalogStickLeanUp);
    Cmd_AddCommand("EnableAsserts", IN_EnableAsserts);
    Cmd_AddCommand("togglepaused", IN_TogglePaused);
    cl_nodelta = Cvar_Get("cl_nodelta", "0", 0);
    cl_debugMove = Cvar_Get("cl_debugMove", "0", 0);
    memset(kbss, 0, sizeof(kbss));
    memset(kb, 0, sizeof(kb));
    cl_stance_ss[0] = 0;
    cl_altFireButtonDown_ss[0] = 0;
    cl_grenadeButtonDown_ss[0] = 0;
}

// ============================================================================
// Key events / console
// ============================================================================

// ea: 0x535120
void CL_KeyEvent(int key, int down, unsigned int time)
{
    (void)time;
    KeyInfoEntry3* e = &KeyInfo_mKeys[currCl][key];
    e->mState ^= (down ^ e->mState) & 3;
    if (down == 0)
    {
        e->mState &= 3;
        if (--anykeydown < 0)
            anykeydown = 0;
    }
    else
    {
        e->mState = (e->mState & 3) ^ ((e->mState & 0xFFFFFFFC) + 4);
        if ((e->mState & 0xFFFFFFFC) == 4)
            ++anykeydown;
    }
    if ((cls.keyCatchers & 1) != 0)
    {
        if (key == 162)  // ` key
        {
            if (down != 0)
                Con_ToggleConsole_f();
        }
        else if (key == 96 || key == 126)
        {
            if (down != 0)
                Con_ToggleConsole_f();
        }
    }
    else if (key == 162 || key == 96 || key == 126)
    {
        if (down != 0)
            Con_ToggleConsole_f();
    }
    // bound command execution
    if (down != 0 && e->mBoundCmdName != nullptr)
        Cbuf_AddText(e->mBoundCmdName);
}

// ea: 0x5355C0
void Key_ClearStates()
{
    anykeydown = 0;
    for (int i = 0; i < 256; ++i)
    {
        KeyInfo_mKeys[currCl][i].mState = 0;
    }
}

// ea: 0x534CB0
void Console_Key(int key)
{
    char temp[256];
    switch (key)
    {
    case 108:  // 'l' with ctrl
        Cbuf_AddText("clear\n");
        return;
    case 13:
    case 191:
        if (cls.state != 2 && g_consoleField.buffer[0] != 92
            && g_consoleField.buffer[0] != 47)
        {
            Q_strncpyz(temp, g_consoleField.buffer, 256);
            Com_sprintf(g_consoleField.buffer, 256, "\\%s", temp);
            ++g_consoleField.cursor;
        }
        Com_Printf("]%s\n", g_consoleField.buffer);
        if (g_consoleField.buffer[0] == 92 || g_consoleField.buffer[0] == 47)
        {
            Cbuf_AddText(&g_consoleField.buffer[1]);
        }
        else
        {
            Cbuf_AddText(g_consoleField.buffer);
        }
        Cbuf_AddText("\n");
        Field_Clear(&g_consoleField);
        Field_AdjustScroll(&g_consoleField);
        break;
    default:
        break;
    }
}

// ea: 0x5350A0
void CompleteAndExecCommand(char* command)
{
    if (strlen(command) != 0)
    {
        strcpy(g_consoleField.buffer, command);
    }
    if (strlen(g_consoleField.buffer) != 0)
    {
        CompleteCommand();
        Console_Key(191);
    }
    Field_AdjustScroll(&g_consoleField);
}

// ============================================================================
// Loading
// ============================================================================

// ea: 0x534C30
void CL_StartLoading()
{
    ASSERT("com_cl_running->integer", "c:\\cod\\code\\game\\cl_cgame.cpp",
           1713);
    CL_StartHunkUsers();
    Cvar_Set("r_uiFullScreen", "0");
    float screen_time_inc = Com_GetScreenTimeDelta();
    SCR_UpdateScreen(screen_time_inc);
    GamePause_SetAllPaused(true);
}

// ea: 0x5358C0
void CL_MapLoading()
{
    ASSERT("com_cl_running->integer", "c:\\cod\\code\\game\\cl_main.cpp", 265);
    if (com_cl_running->integer != 0)
    {
        Field_Clear(&g_consoleField);
        Con_ClearNotify();
        con.finalFrac = 0.0f;
        con.displayFrac = 0.0f;
    }
    cls.keyCatchers = 0;
    Cvar_Set("nextmap", defaultFileName);
    if (com_cl_running != nullptr && com_cl_running->integer != 0)
    {
        Cvar_Set("r_uiFullScreen", "1");
        CL_ClearState();
        cls.state = 0;  // CA_DISCONNECTED
    }
    Q_strncpyz((char*)cls.servername, "localhost", 128);
    cls.state = 1;  // CA_LOADING
    float screen_time_inc = Com_GetScreenTimeDelta();
    SCR_UpdateScreen(screen_time_inc);
    SoundDevice_StopAllSounds(SoundDevice::sInst);
    SoundDevice::sInst->FrameAdvance( 0.0f);
}

// ============================================================================
// Server-message parsing (cl.o cl_parse.cpp)
// ============================================================================

// ea: 0x52DA40
void CL_ParsePacketEntities(msg_t* msg, void* snapshot)
{
}

// ea: 0x52DA50
void CL_ParseCommandString(msg_t* msg)
{
    int Long = MSG_ReadLong(msg);
    const char* String = MSG_ReadString(msg);
    int v3 = 19528 * currCl;
    if (dword_F0F200[4882 * currCl] < Long)
    {
        dword_F0F200[4882 * currCl] = Long;
        Q_strncpyz(&byte_F0F208[128 * (Long & 0x3F) + v3], String, 128);
    }
}

// ea: 0x532D50
void CL_ParseGamestate(Broc::string* configstrings)
{
    ASSERT("com_cl_running->integer", "c:\\cod\\code\\game\\cl_parse.cpp", 257);
    if (cgvm != nullptr)
        VM_Call(cgvm, 21, com_time);
    int* p_rendererStarted = (int*)&cls_configstrings[0].mBlock;
    for (int i = 0; i < 1024; ++i)
    {
        int v3 = ((int*)configstrings)[i];
        void* v4 = (void*)&((int*)configstrings)[i];
        if (v3 == 0)
        {
            ASSERT("configstrings[i].IsDefined()",
                   "c:\\cod\\code\\game\\cl_parse.cpp", 264);
        }
        if (p_rendererStarted[0] != 0)
        {
            if (p_rendererStarted[0] == v3)
                goto next;
        }
        else if (cgvm != nullptr)
        {
            ASSERT("!cgvm", "c:\\cod\\code\\game\\cl_parse.cpp", 273);
        }
        p_rendererStarted[0] = v3;
        if (cgvm != nullptr)
            VM_Call(cgvm, 20, i);
    next:
        p_rendererStarted += 1;
        (void)v4;
    }
    CL_SystemInfoChanged();
}

// ea: 0x532C30
void CL_SystemInfoChanged()
{
    int soundStarted = (int)cls_configstrings[1].mBlock;
    if (soundStarted == 0)
    {
        ASSERT("cls.configstrings[1].IsDefined()",
               "c:\\cod\\code\\game\\cl_parse.cpp", 221);
        soundStarted = (int)cls_configstrings[1].mBlock;
    }
    const char* v1;
    if (soundStarted != 0)
    {
        v1 = (const char*)(soundStarted + 12);
        if (soundStarted == -12)
        {
            ASSERT("systemInfo", "c:\\cod\\code\\game\\cl_parse.cpp", 223);
        }
    }
    else
    {
        v1 = defaultFileName;
    }
    const char* v2 = Info_ValueForKey(v1, "sv_serverid");
    cl[0].serverId = atoi(v2);
    const char* v3 = Info_ValueForKey(v1, "sv_cheats");
    if (atoi(v3) == 0)
        Cvar_SetCheatState();
    const char* v4 = Info_ValueForKey(v1, "sv_paks");
    const char* v5 = Info_ValueForKey(v1, "sv_pakNames");
    nullsub_16(v4, v5);
    const char* v6 = Info_ValueForKey(v1, "sv_referencedPaks");
    const char* v7 = Info_ValueForKey(v1, "sv_referencedPakNames");
    nullsub_34(v6, v7);
}

// ea: 0x52F2B0
void CL_InitCGame()
{
    int t1 = Sys_Milliseconds();
    ASSERT("com_sv_running->integer", "c:\\cod\\code\\game\\cl_cgame.cpp",
           1745);
    int rendererStarted = (int)cls_configstrings[0].mBlock;
    if (rendererStarted == 0)
    {
        ASSERT("cls.configstrings[0].IsDefined()",
               "c:\\cod\\code\\game\\cl_cgame.cpp", 1748);
        rendererStarted = (int)cls_configstrings[0].mBlock;
    }
    const char* v1;
    if (rendererStarted != 0)
    {
        v1 = (const char*)(rendererStarted + 12);
        if (rendererStarted == -12)
        {
            ASSERT("info", "c:\\cod\\code\\game\\cl_cgame.cpp", 1750);
        }
    }
    else
    {
        v1 = defaultFileName;
    }
    const char* v2 = Info_ValueForKey(v1, "mapname");
    Com_sprintf(dest, 128, "maps/%s.bsp", v2);
    cgvm = VM_Create("cgame", CL_CgameSystemCalls);
    if (cgvm == nullptr)
        Com_Error((errorParm_t)1, "VM_Create on cgame failed");
    if (cls.state != 1)  // CA_LOADING
    {
        ASSERT("cls.state == CA_LOADING", "c:\\cod\\code\\game\\cl_cgame.cpp",
               1759);
    }
    if (dword_F0F1FC[4882 * currCl] != 0)
    {
        ASSERT("!clc[currCl].serverMessageSequence",
               "c:\\cod\\code\\game\\cl_cgame.cpp", 1761);
    }
    if (dword_F0F204[4882 * currCl] != 0)
    {
        ASSERT("!clc[currCl].lastExecutedServerCommand",
               "c:\\cod\\code\\game\\cl_cgame.cpp", 1762);
    }
    VM_Call(cgvm, 0);
    int v4 = Sys_Milliseconds();
    Com_Printf("CL_InitCGame: %5.2f seconds\n", (v4 - t1) * 0.001f);
    extern void re_EndRegistration();
    re_EndRegistration();
    Con_ClearNotify();
    if (cl_noprint != nullptr && cl_noprint->integer == 0)
    {
        int t1a = Sys_Milliseconds();
        tlPrintf("Total load time: %.2f\n", t1a * 0.001f);
    }
}

// ea: 0x533A90
void CL_ParseSnapshot(msg_t* msg)
{
    unsigned char v5[1536];
    memset(v5, 0, sizeof(v5));
    *(int*)&v5[1528] = dword_F0F200[4882 * currCl];  // serverCommandNum
    *(int*)&v5[8] = MSG_ReadLong(msg);               // serverTime
    *(int*)&v5[12] = dword_F0F1FC[4882 * currCl];    // messageNum
    *(int*)&v5[4] = MSG_ReadByte(msg);               // snapFlags
    *(int*)&v5[0] = 1;                               // valid
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    if (player != nullptr && player->client != nullptr)
        memcpy(&v5[32], player->client, 0x5D0);
    int v4 = cl[currCl].snap.messageNum + 1;
    if (*(int*)&v5[12] - v4 < 1 && v4 < *(int*)&v5[12])
        cl_snapshots[currCl][0].valid = 0;
    cl[currCl].snap.messageNum = *(int*)&v5[12];
    cl[currCl].snap.serverTime = *(int*)&v5[8];
    cl_snapshots[currCl][0].valid = *(int*)&v5[0];
    cl_snapshots[currCl][0].snapFlags = *(int*)&v5[4];
    cl_snapshots[currCl][0].serverCommandNum = *(int*)&v5[1528];
    cl_snapshots[currCl][0].serverTime = *(int*)&v5[8];
    memcpy(&cl_snapshots[currCl][0].ps, &v5[32],
           sizeof(cl_snapshots[currCl][0].ps));
    if (cl_shownet->integer == 3)
        Com_Printf("   snapshot:%i\n", cl[currCl].snap.messageNum);
}

// ea: 0x533BD0
void CL_ParseServerMessage(msg_t* msg)
{
    int integer = cl_shownet->integer;
    if (integer == 1)
    {
        Com_Printf("%i ", msg->cursize);
    }
    else if (integer >= 2)
    {
        Com_Printf("------------------\n");
    }
    int Long = MSG_ReadLong(msg);
    int v3 = 4882 * currCl;
    dword_F0D1F8[v3] = Long;
    int v4 = dword_F0D1F4[v3];
    if (Long < v4 - 64)
        dword_F0D1F8[v3] = v4;
    while (1)
    {
        if (msg->readcount > msg->cursize)
            Com_Error((errorParm_t)1, "CL_ParseServerMessage: read past end of server "
                         "message");
        int Byte = MSG_ReadByte(msg);
        if (Byte == 8)
            break;
        if (cl_shownet->integer >= 2)
        {
            if (svc_strings[Byte] != nullptr)
                Com_Printf("%3i %3i:%s\n", msg->readcount - 1, msg->cursize,
                           svc_strings[Byte]);
            else
                Com_Printf("%3i:BAD CMD %i\n", msg->readcount - 1, Byte);
        }
        if (Byte != 1)
        {
            if (Byte == 5)
            {
                int v6 = MSG_ReadLong(msg);
                const char* String = MSG_ReadString(msg);
                int v8 = 19528 * currCl;
                if (dword_F0F200[4882 * currCl] < v6)
                {
                    dword_F0F200[4882 * currCl] = v6;
                    Q_strncpyz(&byte_F0F208[128 * (v6 & 0x3F) + v8], String,
                               128);
                }
            }
            else if (Byte == 7)
            {
                CL_ParseSnapshot(msg);
            }
            else
            {
                Com_Error((errorParm_t)1, "CL_ParseServerMessage: Illegible server "
                             "message %d\n", Byte);
            }
        }
    }
    if (cl_shownet->integer >= 2)
        Com_Printf("%3i %3i:%s\n", msg->readcount - 1, msg->cursize,
                   "END OF MESSAGE");
}

// ea: 0x534670
void CL_PacketEvent(netadr_t from, msg_t* msg, int time)
{
int clc_lastPacketTime[2 * 4882];  // cl.o BSS
    clc_lastPacketTime[4882 * currCl] = dword_F170F8;
    if (msg->cursize < 4 || *(int*)msg->data == -1)
    {
        ASSERT("msg->cursize >= 4 && *(int *)msg->data != -1",
               "c:\\cod\\code\\game\\cl_main.cpp", 629);
    }
    if (cls.state != 0)  // CA_DISCONNECTED
    {
        if (NET_CompareAdr(from,
                           *(netadr_t*)(0xF11210 + 19528 * currCl)) != 0)
        {
            if (Netchan_Process((netchan_t*)(0xF11208 + 19528 * currCl),
                                msg) != 0)
            {
                int v4 = 4882 * currCl;
                int v5 = dword_F170F8;
                dword_F0F1FC[v4] = *(int*)msg->data;
                clc_lastPacketTime[4882 * currCl] = v5;
                CL_ParseServerMessage(msg);
            }
        }
        else
        {
            const char* v3 = NET_AdrToString(from);
            Com_DPrintf("%s:sequenced packet without connection\n", v3);
        }
    }
}

// ============================================================================
// Renderer init (cl.o cl_main.cpp)
// ============================================================================

// ea: 0x532930
void CL_InitRef()
{
    Com_Printf("----- Initializing Renderer ----\n");
    refimport_t ri;
    extern void Cmd_RemoveCommand(const char*);
    extern int Cmd_Argc();
    extern char* Cmd_Argv(int);
    extern void Cbuf_ExecuteText(int, const char*);
    extern void CL_RefPrintf(int, const char*, ...);
    extern int CL_ScaledMilliseconds();
    extern int FS_ReadFile(const char*, void**);
    extern void FS_FreeFile(void*);
    extern void FS_FreeFileList(char**);
    extern char** FS_ListFiles(const char*, const char*, int*);
    extern int FS_FileExists(const char*);
    extern int FS_FOpenFileByMode(const char*, int*, enum fsMode_t);
    extern void FS_FCloseFile(int);
    extern int FS_Read(void*, int, int);
    extern int FS_Write(const void*, int, int);
    extern class BspPlane* CM_GetPlaneNum(int);
    extern struct cvar_t* Cvar_FindVar(const char*);
    extern int Com_SaveCvarsToBuffer(const char**, int, char*, int);
    extern int Com_LoadCvarsFromBuffer(const char**, int, const char*,
                                       const char*);
    extern int CG_GetGameModel(short);
    extern void CG_DObjCalcPose(void*, void*, int*);
    extern void SCR_AdjustFrom640(float*, float*, float*, float*);
    extern nglFont* CL_GetFontInfo(int, float);
    extern refexport_t* GetRefAPI(int apiVersion, refimport_t* rimp);
    ri.Cmd_AddCommand = Cmd_AddCommand;
    ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
    ri.Cmd_Argc = Cmd_Argc;
    ri.Cmd_Argv = Cmd_Argv;
    ri.Cmd_ExecuteText = Cbuf_ExecuteText;
    ri.Printf = CL_RefPrintf;
    ri.Error = Com_Error;
    ri.Milliseconds = CL_ScaledMilliseconds;
    memset(&ri.Hunk_AllocInternal, 0, 20);
    ri.FS_ReadFile = FS_ReadFile;
    ri.FS_FreeFile = FS_FreeFile;
    ri.FS_FreeFileList = FS_FreeFileList;
    ri.FS_ListFiles = FS_ListFiles;
    ri.FS_FileExists = FS_FileExists;
    ri.FS_FOpenFileByMode = FS_FOpenFileByMode;
    ri.FS_FCloseFile = FS_FCloseFile;
    ri.FS_Read = FS_Read;
    ri.FS_Write = FS_Write;
    ri.CM_GetPlaneNum = CM_GetPlaneNum;
    ri.Cvar_Get = Cvar_Get;
    ri.Cvar_FindVar = Cvar_FindVar;
    ri.Cvar_Set = Cvar_Set;
    ri.Com_SaveCvarsToBuffer = Com_SaveCvarsToBuffer;
    ri.Com_LoadCvarsFromBuffer = Com_LoadCvarsFromBuffer;
    ri.CG_GetGameModel = CG_GetGameModel;
    ri.CG_DObjCalcPose = CG_DObjCalcPose;
    ri.AdjustFrom640 = SCR_AdjustFrom640;
    ri.UI_GetFontInfo = reinterpret_cast<void* (*)(int, float)>(CL_GetFontInfo);
    refexport_t* RefAPI = GetRefAPI(14, &ri);
    Com_Printf("-------------------------------\n");
    if (!RefAPI)
        Com_Error((errorParm_t)0, "EXE_ERR_COULDNT_INIT_REFRESH");
    // GetRefAPI fills the renderer's global refexport_t and returns its address.
    extern void GamePause_SetGamePaused(int client, bool paused);
    GamePause_SetGamePaused(currCl, 0);
}

// ============================================================================
// Frame / main loop (cl.o cl_main.cpp)
// ============================================================================

// ea: 0x5347E0
void CL_Frame(int msec, float screen_time_inc)
{
    if (com_cl_running->integer != 0)
    {
        if (cls.cddialog != 0)
        {
            cls.cddialog = 0;
        }
        else if (cls.state == 0 && (cls.keyCatchers & 2) == 0
                 && com_sv_running->integer == 0)
        {
            SoundDevice_StopAllSounds(SoundDevice::sInst);
            cls.keyCatchers = 2;
            SoundDevice_UndampenAllSounds(SoundDevice::sInst);
            Cvar_Set("g_reloading", "0");
        }
        int integer = cl_avidemo->integer;
        int v4 = msec;
        if (integer != 0 && msec != 0)
        {
            if (cls.state == 2 || cl_forceavidemo->integer != 0)
            {
                if (integer <= 0)
                    Cbuf_ExecuteText(0, "screenshotjpeg silent\n");
                else
                    Cbuf_ExecuteText(0, "screenshot silent\n");
            }
            v4 = (int)((1000.0f / (float)(integer < 0 ? -integer : integer))
                       * com_timescale->value);
            if (v4 == 0)
                v4 = 1;
        }
        bool v14 = currCl == lFirstLocalClientIndex;
        if (currCl == lFirstLocalClientIndex)
        {
            dword_F170FC = v4;
            dword_F170F8 += dword_F170F0;
        }
        if (*(&dword_F6A290[0] + 802 * currCl) == 2 && cls.state == 2)
        {
            CL_CreateNewCommands();
            CL_WritePacket();
        }
        if (gUseControllerLagFix && *(&dword_F6A290[0] + 802 * currCl) == 2)
            SendClientThinkMsg();
        if (cls.state == 2)
        {
            cl[0].serverTime = com_time;
            cl[0].oldServerTime = com_time;
        }
        cdl_proftimer_draw.start();
        if (!g_controllerConnectedErrorShown[controller_view::inst()->locked_port])
            SCR_UpdateScreen(screen_time_inc);
        cdl_proftimer_draw.stop();
        float v6 = v4 * 0.001f;
        if (v14)
        {
            cdl_proftimer_audio.start();
            codNflUpdate();
            SoundDevice::sInst->FrameAdvance( v6);
            AudioBankMgr_Update(AudioBankMgr_sInst);
            cdl_proftimer_audio.stop();
        }
        if (tr.world != nullptr && v14)
        {
            cdl_proftimer_streaming.start();
            float playerPos[4];
            Entity* player = EntityManager::sInst->GetPlayer(currCl);
            const float* p_currentOrigin =
                player->r.currentOrigin.v.m128_f32;
            float v12[3];
            v12[0] = p_currentOrigin[0];
            v12[1] = p_currentOrigin[1];
            v12[2] = p_currentOrigin[2];
            playerPos[0] = p_currentOrigin[3];
            ClientFrameView* client =
                reinterpret_cast<ClientFrameView*>(player->client);
            if (client != nullptr && client->playerState == 1
                && reinterpret_cast<ClientFrameView*>(GetPlayer(currCl)->client)
                           ->spectatorClient
                       >= 0)
            {
                int spectatorClient = client->spectatorClient;
                const float* v9 =
                    GetPlayer(spectatorClient)->r.currentOrigin.v.m128_f32;
                v12[0] = v9[0];
                v12[1] = v9[1];
                v12[2] = v9[2];
                playerPos[0] = v9[3];
            }
            int v10 = R_CellForPoint(v12);
            StreamZoneManager_Update(StreamZoneManager_sInst, v10, v12,
                                     false);
            cdl_proftimer_streaming.stop();
        }
        if (currCl == lFirstLocalClientIndex)
        {
            cdl_proftimer_pak_mgr.start();
            PakManager::sInst->Update(false);
            cdl_proftimer_pak_mgr.stop();
            cdl_proftimer_music_mgr.start();
            MusicMgr_Update(MusicMgr_sInst, v6);
            cdl_proftimer_music_mgr.stop();
            cdl_proftimer_effect_sys.start();
            EffectEventSys_FrameAdvance(EffectEventSys::sInst, v6);
            cdl_proftimer_effect_sys.stop();
        }
        cdl_proftimer_rumble_mgr.start();
        RumbleManager_FrameAdvance(RumbleManager::Inst(currCl), v6);
        cdl_proftimer_rumble_mgr.stop();
        if (v14)
        {
            cdl_proftimer_scn_effect.start();
            SceneManager_UpdateEffects(SceneManager_sInst, v6);
            cdl_proftimer_scn_effect.stop();
        }
        if (currCl == lLastLocalClientIndex)
        {
            cdl_proftimer_entities.start();
            EntityHandleDb_Compact(&EntityHandleDb::sInst);
            cdl_proftimer_entities.stop();
        }
        if (v14)
            Con_RunConsole();
        if (currCl == lLastLocalClientIndex)
            ++dword_F170EC;
    }
}
