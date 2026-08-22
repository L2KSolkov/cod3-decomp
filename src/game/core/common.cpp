// ============================================================================
// common.cpp - common/system core (core.o common.cpp, com_shared.cpp)
// Reconstructed from IDA release decompiles (ea comments below).
// ============================================================================

#include "game/core/core_types.h"
#include "game/core/core_systems.h"
#include "game/core/core_globals.h"
#include "input/controller.h"

// ?gSoundOptions@@3VSoundOptions@@A (core.o @ 0xF00EF0)
SoundOptions gSoundOptions;

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <time.h>

class XAnimTree;  // anim.o

// Minimal view of GamePause (full class in game/sv/sv_stubs.h).
struct GamePause { static bool IsGamePaused(int client); };


// Minimal view of SoundDevice (full class in game/sv/sv_stubs.h).
class SoundDevice {
public:
    static SoundDevice* sInst;
    void FrameAdvance(float delta);
    static void CreateInst();  // ?CreateInst@SoundDevice@@SAXXZ (core.o)
    static void DeleteInst();  // ?DeleteInst@SoundDevice@@SAXXZ (core.o)
};


struct NumBanks;

// Minimal view of PakManager (full class in game/sv/sv_stubs.h).
class PakManager {
public:
    static PakManager* sInst;
    const PakInfoNode* GetPakInfo(const char* long_name) const;
    void SetUserDistance(const PakInfoNode* cpak, float dist);
    void SetProgressCallback(void (*cb)(float));
    TPakId SyncLoadPak(const PakInfoNode* cpak);
    TPakId SyncLoadPak(EPakType t, const char* path, NumBanks banks);
    static void CreateInst();  // ?CreateInst@PakManager@@SAXXZ (core.o)
    static void DeleteInst();  // ?DeleteInst@PakManager@@SAXXZ (core.o)
};  // ?sInst@PakManager@@2PAV1@A


// Minimal view of InteractionController (full class in g_local.h).
class InteractionController {
public:
    static InteractionController* Inst(int instance);  // ?Inst@InteractionController@@SAPAV1@H@Z
    static InteractionController* CreateInst();  // ?CreateInst@InteractionController@@SAPAV1@XZ (core.o)
    static void DeleteInst();  // ?DeleteInst@InteractionController@@SAXXZ (core.o)
};

class PadAliasMgr {
public:
    static PadAliasMgr* CreateInst();  // ?CreateInst@PadAliasMgr@@SAXXZ
};

class GameSettings {
public:
    static GameSettings* CreateInst();  // ?CreateInst@GameSettings@@SAXXZ
};


extern int dword_F6A290[4 * 802];  // Xbox dev/retail flag array @ 0xF6A290
extern const char defaultFileName[];  // shared empty/default string @ 0xCD67AE

// ============================================================================
// Minimal network/message types (server_types.h full version in sv/)
// ============================================================================
struct msg_t {
    int overflowed;
    unsigned char* data;
    int maxsize;
    int cursize;
    int readcount;
};
struct netadr_t {
    int type;
    unsigned char ip[4];
    unsigned char ipx[10];
    unsigned short port;
};
enum netsrc_t {
    NS_CLIENT = 0,
    NS_SERVER = 1,
};

// ============================================================================
// Externs
// ============================================================================
enum errorParm_t {
    ERR_FATAL = 0,
    ERR_DROP = 1,
    ERR_DISCONNECT = 2,
};
extern void Com_Error(errorParm_t code, const char* fmt, ...);
extern void Com_Printf(const char* fmt, ...);
extern void Com_DPrintf(const char* fmt, ...);
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);
extern void Com_PrintMessage(int type, const char* msg);
extern cvar_t* Cvar_Set2(const char* var_name, const char* value, int force);
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value, int flags);
extern cvar_t* Cvar_FindVar(const char* var_name);
extern void Cvar_Set(const char* var_name, const char* value);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern void Q_strcat(char* dest, int size, const char* src);
extern int Q_stricmp(const char* s1, const char* s2);
extern int Q_stricmpn(const char* s1, const char* s2, int n);
enum msgLocErrType_t { LOCMSG_NOERR = 0, LOCMSG_ERR = 1 };
extern const char* SEH_LocalizeTextMessage(const char* pszInputBuffer,
                                           const char* pszMessageType,
                                           msgLocErrType_t errType);  // shell.o
extern int Cmd_Argc();
extern char* Cmd_Argv(int arg);
extern void Cmd_AddCommand(const char* cmd_name, void (*function)());
extern void Cmd_TokenizeString(const char* text_in);
extern void Cbuf_AddText(const char* text);
extern void Cbuf_Execute();
extern void Cbuf_Init();
extern void Cmd_Init();
extern void CL_Shutdown();
extern void CL_Init();
extern void CL_Disconnect();
extern void CL_ShutdownAll();
extern void CL_StartHunkUsers();
extern void CL_KeyEvent(int key, int down, unsigned int time);
extern void CL_CharEvent(int key);
extern void CL_MouseEvent(int dx, int dy, int time);
extern void CL_GamepadEvent(int physicalAxis, int value, int time);
extern void CL_PacketEvent(netadr_t from, msg_t* msg, int time);
extern void SV_Shutdown();
extern void SV_Init();
extern void SV_Frame(int msec);
extern void SV_PacketEvent(netadr_t from, msg_t* msg);
extern void FS_InitFilesystem();
extern void FS_Shutdown(int closemfp);
extern int FS_FOpenFileWrite(const char* filename);
extern void FS_FCloseFile(int f);
extern void FS_ForceFlush(int f);
extern void FS_Printf(int h, const char* fmt, ...);
extern int FS_Write(const void* buffer, int len, int h);
extern int FS_Read(void* buffer, int len, int f);
// FS_FOpenFileRead_Internal is static in files.cpp; local shim for journal I/O
static int FS_FOpenFileRead_Internal(const char* filename, int* file,
                                     int uniqueFILE, int streamThread)
{
    (void)filename; (void)uniqueFILE; (void)streamThread;
    *file = -1;
    return 0;
}
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);
extern char* CopyStringInternal(const char* in);
extern char* va(const char* fmt, ...);
extern int Sys_Milliseconds();
extern sysEvent_t Sys_GetEvent();
extern int NET_GetLoopPacket(netsrc_t sock, netadr_t* net_from,
                             msg_t* net_message);
extern void* _copyDWord(void* dest, int constant, unsigned int count);
extern void Com_DefaultExtension(char* path, int maxSize,
                                 const char* extension);
extern void Com_CleanupSkeletons();
extern void Com_ResetParseSessions();
extern void XAnimShutdown();
extern void XAnimInit();
extern void XAnimFreeTree(XAnimTree* tree);
extern void Sys_OutOfMemError();
extern void tlFatal(const char* fmt, ...);
extern void SoundDevice_StopAllSounds(void* self);
extern void AudioBankMgr_Update(void* self);
extern void* AudioBankMgr_sInst;
enum nflState : unsigned;
extern nflState codNflUpdate();
extern void SyncFrameBuffers();
struct FEManager; extern FEManager g_femanager;
bool gUseNfl;                       // ?gUseNfl@@3_NA (core.o)
bool g_enableControllerTest;        // ?g_enableControllerTest@@3_NA (game2.o)
bool g_controllerConnected[4];      // ?g_controllerConnected@@3PA_NA (game2.o)
bool g_controllerConnectedErrorShown[4];  // ?g_controllerConnectedErrorShown@@3PA_NA (game2.o)
bool g_controllerConnectedGamePaused[4];  // ?g_controllerConnectedGamePaused@@3PA_NA (game2.o)
extern int logfile;
extern int opening_qconsole;
extern int com_safemode;
char* rd_buffer;  // ?rd_buffer@@3PADA (core.o)
extern int rd_buffersize;
extern void (*rd_flush)(char*);
extern int last_time;
extern int lastTime;
float g_time_inc;
sysEvent_t com_pushedEvents[256];  // ?com_pushedEvents@@3PAUsysEvent_t@@A (core.o)
extern int com_pushedEventsHead;
extern int com_pushedEventsTail;
extern int printedWarning;
extern int lastErrorTime;
int errorCount;
extern int time_game;
extern int time_frontend;
extern int time_backend;
int timeClientFrame;
bool gFirstCamera;                  // ?gFirstCamera@@3_NA (cg.o)
bool* gControllerWarningDialogIsActive;  // ?gControllerWarningDialogIsActive@@3PA_NA (game2.o @ 0x12EFB0C)
extern int gScreenshotInProgress;
int com_frameTime = 0;   // ?com_frameTime@@3HA (core.o @ 0xEF2828)
int com_frameNumber = 0;  // ?com_frameNumber@@3HA (core.o)
float g_screendelta;
extern int g_bDObjInited;
char* surfaceTypeNames[23];  // ?surfaceTypeNames (core.o)
extern int com_fullyInitialized;
extern int com_fileAccessed;
extern int com_journalFile;
extern int com_journalDataFile;
extern int fs_loadStack;
extern cvar_t* com_maxfps;
extern cvar_t* com_developer;
extern cvar_t* com_developer_script;
extern cvar_t* com_logfile;
extern cvar_t* com_statmon;
cvar_t* com_timescale;  // ?com_timescale@@3PAUcvar_t@@A (core.o @ 0x12E5F84)
// --- cvar_t* data sweep (owner object in map) ---
cvar_t* com_developer;           // ?com_developer@@3PAUcvar_t@@A (core.o @ 0x12E5EB8)
cvar_t* com_developer_script;    // ?com_developer_script@@3PAUcvar_t@@A (core.o @ 0x12EFB08)
cvar_t* com_journal;             // ?com_journal@@3PAUcvar_t@@A (core.o @ 0x12E8EA8)
cvar_t* com_statmon;             // ?com_statmon@@3PAUcvar_t@@A (core.o @ 0x12E5EC0)
cvar_t* com_logfile;             // ?com_logfile@@3PAUcvar_t@@A (core.o @ 0x12E5F70)
cvar_t* com_fixedtime;           // ?com_fixedtime@@3PAUcvar_t@@A (core.o @ 0x12E5F64)
cvar_t* com_viewlog;             // ?com_viewlog@@3PAUcvar_t@@A (core.o @ 0x12E5F58)
cvar_t* com_speeds;              // ?com_speeds@@3PAUcvar_t@@A (core.o @ 0x12E7690)
cvar_t* com_maxfps;              // ?com_maxfps@@3PAUcvar_t@@A (core.o @ 0x12EFF24)
cvar_t* com_animCheck;           // ?com_animCheck@@3PAUcvar_t@@A (core.o @ 0x12EFB00)
cvar_t* cvar_vars;               // ?cvar_vars@@3PAUcvar_t@@A (core.o @ 0x12E5F8C)
cvar_t* fs_basegame;             // ?fs_basegame@@3PAUcvar_t@@A (core.o @ 0x12E5EC4)
cvar_t* fs_basepath;             // ?fs_basepath@@3PAUcvar_t@@A (core.o @ 0x12E768C)
cvar_t* fs_cdpath;               // ?fs_cdpath@@3PAUcvar_t@@A (core.o @ 0x12E5AB4)
cvar_t* fs_copyfiles;            // ?fs_copyfiles@@3PAUcvar_t@@A (core.o @ 0x12E5AB0)
cvar_t* fs_debug;                // ?fs_debug@@3PAUcvar_t@@A (core.o @ 0x12E6524)
cvar_t* fs_gamedirvar;           // ?fs_gamedirvar@@3PAUcvar_t@@A (core.o @ 0x12E8EB0)
cvar_t* fs_homepath;             // ?fs_homepath@@3PAUcvar_t@@A (core.o @ 0x12EFFB0)
cvar_t* fs_ignoreLozalized;      // ?fs_ignoreLozalized@@3PAUcvar_t@@A (core.o @ 0x12E8EAC)
cvar_t* fs_restrict;             // ?fs_restrict@@3PAUcvar_t@@A (core.o @ 0x12E5F68)
cvar_t* cl_anglespeedkey;        // ?cl_anglespeedkey@@3PAUcvar_t@@A (cl.o @ 0x13028EC)
cvar_t* cl_yawspeed;             // ?cl_yawspeed@@3PAUcvar_t@@A (cl.o @ 0x12FC4C8)
cvar_t* cl_pitchspeed;           // ?cl_pitchspeed@@3PAUcvar_t@@A (cl.o @ 0x1302848)
cvar_t* cl_stanceHoldTime;       // ?cl_stanceHoldTime@@3PAUcvar_t@@A (core.o @ 0x12E5F50)
cvar_t* cl_disable_ads;          // ?cl_disable_ads@@3PAUcvar_t@@A (cl.o @ 0x12FC4E8)
cvar_t* cl_freeze;               // ?cl_freeze@@3PAUcvar_t@@A (cl.o @ 0x1302840)
cvar_t* cl_mouseAccel;           // ?cl_mouseAccel@@3PAUcvar_t@@A (cl.o @ 0x12FC6E0)
cvar_t* cl_showMouseRate;        // ?cl_showMouseRate@@3PAUcvar_t@@A (cl.o @ 0x12FC6CC)
cvar_t* cl_shownet;              // ?cl_shownet@@3PAUcvar_t@@A (cl.o @ 0x1304C98)
cvar_t* cl_avidemo;              // ?cl_avidemo@@3PAUcvar_t@@A (cl.o @ 0x12FC148)
cvar_t* cl_capturemovie;         // ?cl_capturemovie@@3PAUcvar_t@@A (cl.o @ 0x1302844)
cvar_t* cl_debugMove;            // ?cl_debugMove@@3PAUcvar_t@@A (cl.o @ 0x13028BC)
cvar_t* cl_forceavidemo;         // ?cl_forceavidemo@@3PAUcvar_t@@A (cl.o @ 0x12FC4E0)
cvar_t* cl_frameadvance;         // ?cl_frameadvance@@3PAUcvar_t@@A (core.o @ 0x12E5F60)
cvar_t* cl_freelook;             // ?cl_freelook@@3PAUcvar_t@@A (cl.o @ 0x1304C8C)
cvar_t* cl_nodelta;              // ?cl_nodelta@@3PAUcvar_t@@A (cl.o @ 0x12FC200)
cvar_t* cl_showSend;             // ?cl_showSend@@3PAUcvar_t@@A (cl.o @ 0x12FC138)
cvar_t* cl_testAnimWeight;       // ?cl_testAnimWeight@@3PAUcvar_t@@A (cl.o @ 0x12FC208)
cvar_t* cl_viewPitchCompensate;  // ?cl_viewPitchCompensate@@3PAUcvar_t@@A (cl.o @ 0x12FC6DC)
cvar_t* cl_viewYawCompensate;    // ?cl_viewYawCompensate@@3PAUcvar_t@@A (cl.o @ 0x1304C70)
cvar_t* cm_noCurves;             // ?cm_noCurves@@3PAUcvar_t@@A (game.o @ 0x132C188)
cvar_t* cm_playerCurveClip;      // ?cm_playerCurveClip@@3PAUcvar_t@@A (game.o @ 0x1333620)
cvar_t* bg_stickyAimRender;      // ?bg_stickyAimRender@@3PAUcvar_t@@A (game.o @ 0x132DF1C)
cvar_t* cg_debugSpawnPoints;     // ?cg_debugSpawnPoints@@3PAUcvar_t@@A (game2.o @ 0x12F29F8)
cvar_t* in_debugJoystick;        // ?in_debugJoystick@@3PAUcvar_t@@A (game2.o @ 0x12F29E0)
cvar_t* in_joyBallScale;         // ?in_joyBallScale@@3PAUcvar_t@@A (game2.o @ 0x12F29E8)
cvar_t* in_joystick;             // ?in_joystick@@3PAUcvar_t@@A (game2.o @ 0x12F2944)
cvar_t* in_mouse;                // ?in_mouse@@3PAUcvar_t@@A (game2.o @ 0x12F2898)
cvar_t* in_stickLegacy;          // ?in_stickLegacy@@3PAUcvar_t@@A (game2.o @ 0x12F2CA0)
cvar_t* in_stickSouthPaw;        // ?in_stickSouthPaw@@3PAUcvar_t@@A (game2.o @ 0x12F2C9C)
cvar_t* joy_threshold;           // ?joy_threshold@@3PAUcvar_t@@A (game2.o @ 0x12F2A00)
cvar_t* m_pitch;                 // ?m_pitch@@3PAUcvar_t@@A (cl.o @ 0x12FC4F0)
cvar_t* m_yaw;                   // ?m_yaw@@3PAUcvar_t@@A (cl.o @ 0x12FC4E4)
extern cvar_t* r_showLocationalDamage;  // ?r_showLocationalDamage@@3PAUcvar_t@@A (render.o @ 0x13636DC)
extern cvar_t* r_showSkeletons;         // ?r_showSkeletons@@3PAUcvar_t@@A (render.o @ 0x13638B8)
void* pWeaponInfoMemory = nullptr;   // ?pWeaponInfoMemory@@3PAXA (core.o @ 0x12F0364)
void* gApsHeap = nullptr;            // ?gApsHeap@@3PAVae_heap@@A (render.o @ 0x1363940)
ae_heap* gActorHeap = nullptr;       // ?gActorHeap@@3PAVae_heap@@A (core.o @ 0x12F035C)
Entity* gLensLightSource = nullptr;  // ?gLensLightSource@@3PAVEntity@@A (core.o @ 0x12F03E4)
struct nglTexture;
nglTexture** gLensFlareTextures = nullptr;  // ?gLensFlareTextures@@3PAPAUnglTexture@@A (core.o @ 0x12F03D0)
Broc::string gFootSplashEffect;             // ?gFootSplashEffect@@3Vstring@Broc@@A (core.o @ 0x12F0450)
RumbleManager::InstanceHolder RumbleManager::sInstHolder;  // ?sInstHolder@RumbleManager@@2UInstanceHolder@1@A (core.o @ 0x12F042C)
AnimHeap* AnimHeap::sInst = nullptr;                  // ?sInst@AnimHeap@@0PAV1@A (core.o @ 0x00F00E98)
DialogueManager* DialogueManagerStatics::sInst = nullptr;  // ?sInst@DialogueManagerStatics@@2PAVDialogueManager@@A (core.o @ 0x12F0374)
fileHandleData_t fsh[3];                         // ?fsh@@3PAUfileHandleData_t@@A (core.o @ 0x12EFFB8)
searchpath_s* fs_searchpaths = nullptr;          // ?fs_searchpaths@@3PAUsearchpath_s@@A (core.o @ 0x12F0334)
filelist_s* fs_nonpackfilelist = nullptr;        // ?fs_nonpackfilelist@@3PAUfilelist_s@@A (core.o @ 0x12F0338)
searchpath_s* fs_memorysearchpaths = nullptr;    // ?fs_memorysearchpaths@@3PAUsearchpath_s@@A (core.o @ 0x12F033C)
filelist_s* fs_memorynonpackfilelist = nullptr;  // ?fs_memorynonpackfilelist@@3PAUfilelist_s@@A (core.o @ 0x12F0340)
float gLensScaleAmount;          // ?gLensScaleAmount@@3MA (core.o @ 0x11C7FF8)
// --- plain int data sweep (core.o family) ---
int com_journalDataFile;     // ?com_journalDataFile@@3HA (core.o @ 0x12E5F78)
int com_fileAccessed;        // ?com_fileAccessed@@3HA (core.o @ 0x12E6520)
int cvar_numIndexes;         // ?cvar_numIndexes@@3HA (core.o @ 0x12EFF10)
int fs_loadStack;            // ?fs_loadStack@@3HA (core.o @ 0x12E8EB4)
int com_journalFile;         // ?com_journalFile@@3HA (core.o)
int com_numConsoleLines;     // ?com_numConsoleLines@@3HA (core.o)
int com_fullyInitialized;    // ?com_fullyInitialized@@3HA (core.o)
int com_errorEntered;        // ?com_errorEntered@@3HA (core.o)
int com_safemode;            // ?com_safemode@@3HA (core.o)
int com_pushedEventsHead;    // ?com_pushedEventsHead@@3HA (core.o)
int com_pushedEventsTail;    // ?com_pushedEventsTail@@3HA (core.o)
int fs_numServerPaks;        // ?fs_numServerPaks@@3HA (core.o)
int fs_checksumFeed;         // ?fs_checksumFeed@@3HA (core.o)
int logfile;                 // ?logfile@@3HA (core.o)
int opening_qconsole;        // ?opening_qconsole@@3HA (core.o)
int time_game;               // ?time_game@@3HA (core.o)
int holdrand;                // ?holdrand@@3HA (core.o)
int printedWarning;          // ?printedWarning@@3HA (core.o)
int lastErrorTime;           // ?lastErrorTime@@3HA (core.o)
int rd_buffersize;           // ?rd_buffersize@@3HA (core.o)
int cmd_argc;                // ?cmd_argc@@3HA (core.o)
char fs_gamedir[128];         // ?fs_gamedir@@3PADA (core.o @ 0x12E5ED0)
char fs_bsp_gamedir[128];    // ?fs_bsp_gamedir@@3PADA (core.o @ 0x12E64A0)
char lastValidBase[128];     // ?lastValidBase@@3PADA (core.o @ 0x12E6600)
char lastValidGame[128];     // ?lastValidGame@@3PADA (core.o @ 0x12EFF30)
extern cvar_t* com_fixedtime;
extern cvar_t* com_viewlog;
extern cvar_t* com_speeds;
int cvar_modifiedFlags = 0;  // ?cvar_modifiedFlags@@3HA (core.o @ 0xEF8194)
cvar_t* com_sv_running = NULL;  // ?com_sv_running@@3PAUcvar_t@@A (core.o @ 0xEF6A90)
extern cvar_t* com_cl_running;

// ?vec3_origin@@3QBMB (core.o rdata)
extern const float vec3_origin[3] = { 0.0f, 0.0f, 0.0f };
extern cvar_t* cl_frameadvance;
extern cvar_t* cl_capturemovie;
extern cvar_t* com_journal;
char* com_consoleLines[32];   // ?com_consoleLines@@3PAPAD (core.o @ 0x12E5F98)
extern int com_numConsoleLines;
extern int com_argc;
extern char** com_argv;
extern int currCl;
enum print_msg_type_t;
extern void CL_ConsolePrint(print_msg_type_t type, const char* txt,
                            int duration, int linewidth, int flags);
extern void Key_WriteBindings(int f);
extern void PadAliasMgr_WriteBindings(int f);
extern void Cvar_WriteVariables(int f);
extern void Cvar_WriteDefaults(int f);
extern void Com_BeginParseSession(const char* filename);
extern const char* Com_Parse(const char** data_p);
struct parseInfo_t {
    char token[128];         // +0x00
    int lines;               // +0x80
    int ungetToken;          // +0x84
    int spaceDelimited;      // +0x88
    int csv;                 // +0x8C
    int negativeNumbers;     // +0x90
    int backup_lines;        // +0x94
    const char* backup_text; // +0x98
};
extern const char* Com_ParseOnLine(const char** data_p);
extern void Com_SkipRestOfLine(const char** data);
extern void Com_EndParseSession();
int controller_num_controllers()
{
    return controller::inst()->get_num_controllers();
}
namespace LocalClient { extern int ClientToPort(int client); }
extern STBManager* STBManager_sInst;
// The call-site artifact forwards to the IDA-verified member at 0x004C5E30.
const char* STBManager_GetSTBString(void* self, const char* pszReference)
{
    return static_cast<STBManager*>(self)->GetSTBString(pszReference);
}
extern void* FEManager_GetDMS(void* self, int client);
extern void DialogMenuSystem_BringUp(void* self, const char* t, bool type_ok,
                                     bool type_yn, const char* title_unloc,
                                     bool layer1);
extern void DialogMenuSystem_CloseDialog(void* self);
extern void* FEManager_GetIGMS(void* self, int client);
extern void InGameMenuSystem_ActivatePauseMenu(void* self);
// The derived call-site artifact uses FEMenuSystem::IsSystemActive at
// 0x005AFA20, which reads the inherited is_active byte at +0x2A.
bool InGameMenuSystem_IsSystemActive(void* self)
{
    return *reinterpret_cast<const bool*>(
        reinterpret_cast<const unsigned char*>(self) + 0x2A);
}
extern void GamePause_SetGamePaused(int client, bool paused);
extern void GamePause_SetAllPaused(bool paused);
extern void SoundDevice_PauseAllSounds(void* self);
extern void SoundDevice_UnpauseAllSounds(void* self);
extern void FEManager_DrawControllerError(void* self);
extern void MemoryUnitManager_Service();
extern void SyncFrameBuffers();
void InteractionController_ChangeWeaponToPending(void* inst)
{
    (void)inst;
}
extern void StatMon_Warning(int type, int duration, const char* pszShaderName);
extern void SetAnimCheck(int bAnimCheck);
extern void ServerTime_Tick();
extern void TaskSys_Update(float deltaT);
extern void* TaskSys_sInst;
extern void CurveManager_Update(void* self, float tickDelta);
extern void* CurveManager_sInst;
extern void DynamicDecalMgr_Update(void* self, float deltaTime);
extern void* DynamicDecalMgr_sInst;
extern void SmokeGrenadeMgr_Update(void* self, float deltaT);
extern void* SmokeGrenadeMgr_sInst;
extern void EntityNotifySet_UpdateList();
extern void update_trigger_notifies();
class subtitle_manager {
public:
    static void frame_advance(int time_delta);  // ?frame_advance@subtitle_manager@@SAXH@Z
};
class InspectorManager;
extern void InspectorManager_Update(InspectorManager* self);
extern InspectorManager g_inspectorManager;
extern void SendClientThinkMsg();
namespace LocalClient {
    extern void SetFirstLocalClientIndex(int index);
    extern void SetLastLocalClientIndex(int index);
}
extern void CL_RecallKeys();
extern void CL_BackUpKeys();
extern void CL_Frame(int msec, float screen_time_inc);
extern void CL_CreateNewCommands();
extern void CL_WritePacket();
extern void SoundDevice_SetNumberOfListeners(void* self, int listeners);
extern void CL_StartHunkUsers();
extern void Com_Error_f();
extern void Com_Crash_f();
extern void Com_Freeze_f();
extern void Sys_Init();
extern void Netchan_Init();
extern void VM_Init();
extern void Swap_Init();
extern void PhysInit();
extern void PhysShutdown();
extern void CL_InitKeyCommands();
extern void CL_InitGamepadCommands();
extern void CL_InitGamepadAxisBindings();
extern void InitPadAliasCommands();
extern void SEH_Init_StringEd();
extern void SEH_UpdateLanguageInfo();
extern void MI_ResetMapList();
extern int CL_PreAllocStrings();
extern void SetupPoolAllocator();
extern ae_heap* SetupActorHeap();
extern void init_dobj_trackers();
extern void XAnimInit();
extern void XAnimShutdown();
extern ae_heap* SetupActorHeap();
enum nflMediaID : unsigned { NFL_MEDIA_DEFAULT = 0 };
extern nflMediaID gNflMediaId;
extern unsigned int nflFileExists(nflMediaID mediaID, const char* filename);
// Minimal singleton views (real symbols are static members, core.o; stub
// definitions live in stubs_game.cpp until the classes are ported).
class BankManager { public: static void CreateInst(); static void DeleteInst(); };
class InstanceBankMgr { public: static void CreateInst(); static void DeleteInst(); };
class LightGridMgr { public: static void CreateInst(); static void DeleteInst(); };
class XModelManager { public: static void CreateInst(); static void DeleteInst(); };
class XModelPartsManager { public: static void CreateInst(); static void DeleteInst(); };
class DestructibleBankManager { public: static DestructibleBankManager* CreateInst(); static void DeleteInst(); };
class PhysDataBankManager { public: static PhysDataBankManager* CreateInst(); static void DeleteInst(); };
class AITypeManager { public: static void CreateInst(); static void DeleteInst(); };
class AudioBankMgr { public: static void CreateInst(); static void DeleteInst(); };
class SoundMediaMgr { public: static SoundMediaMgr* CreateInst(); static void DeleteInst(); };
struct MusicMgr { public: static void CreateInst(); static void DeleteInst(); };
class GdbFileManager { public: static void CreateInst(); static void DeleteInst(); };
class EntityManager { public: static EntityManager* CreateInst(); static void DeleteInst(); };
class SceneManager { public: static void CreateInst(); static void DeleteInst(); };
class PathNodeMgr { public: static PathNodeMgr* CreateInst(); static void DeleteInst(); };
class MultiplayerMgr { public: static void CreateInst(); static void DeleteInst(); };
class CheckpointMgr { public: static CheckpointMgr* CreateInst(); static void DeleteInst(); };
class SplineMgr { public: static SplineMgr* CreateInst(); static void DeleteInst(); };
class SmokeGrenadeMgr { public: static SmokeGrenadeMgr* CreateInst(); };
class CGBankManager { public: static void CreateInst(); static void DeleteInst(); };
class DCGBankManager { public: static void CreateInst(); static void DeleteInst(); };
class AnimBankManager { public: static AnimBankManager* CreateInst(); };
class StreamZoneManager { public: static void CreateInst(); static void DeleteInst(); };
class BinFileManager { public: static BinFileManager* CreateInst(); static void DeleteInst(); };
class CurveManager { public: static CurveManager* CreateInst(); static void DeleteInst(); };
class PlayerAnimMgr { public: static PlayerAnimMgr* CreateInst(); static void DeleteInst(); };
class DynamicDecalMgr { public: static DynamicDecalMgr* CreateInst(); static void DeleteInst(); };
class TestFPS { public: static void CreateInst(); static void DeleteInst(); };
extern void InitLights();
extern void TimerRenderBars_Init(void* self);
extern void* TimerRenderBars_sInst;
extern void EntityHandleDb_Init(void* self);
class EntityHandleDb {
public:
    static EntityHandleDb sInst;  // ?sInst@EntityHandleDb@@0V1@A (g.o)
};
extern void StatusBar_Init(void* self);
extern void FEManager_InitDialogMenuSystem(void* self);
extern void FEManager_LoadInGameMenus(void* self);
extern void FEManager_InitIGO(void* self);
extern void DebugRender_AddRenderer(void* self, void* fp);
extern void* DebugRender_sInst;  // ?sInst@DebugRender@@2V1@A @ 0xF74D20
extern void DebugDumpAnims();
extern void rb_vehicle_debug_render_all();
extern void physics_debug_render();
extern void fx_debug_render();
extern void GlobalPakLoadCallback(float progress);
extern void PakManager_SyncLoadPak(void* self, int pak_type, const char* path,
                                  void* num_banks);
extern void* PakManager_GetPakInfo(void* self, const char* long_name);
extern void PakManager_SetUserDistance(void* self, void* cpak, float dist);
extern void AudioBankMgr_FinishLoading(void* self);
extern void InspectorManager_Initialise(InspectorManager* self);
namespace LocalClient { extern void InitializeClientControllers(); }
extern void WheelMarkMgr_Init();
extern void WheelMarkMgr_Exit();
extern void DebugRender_Init(void* self);
extern void StubData_ApplyStubOptions(void* self);
struct SaveGameData;
extern SaveGameData gSaveGameData[4];
extern void Com_ControllerWarningDialog(bool activate, int client);
extern void MSG_Init(msg_t* msg, unsigned char* data, int length);
extern int generateHashValue(const char* fname);
extern void Cvar_AddCommands();
extern void FS_ShutdownSearchPaths(searchpath_s* p);
char com_errorMessage[4096]; // ?com_errorMessage@@3PADA (core.o)
extern int com_errorEntered;
extern int g_DOBJF_NOT_RENDERED_LAST_FRAME;
extern int timeBeforeEvents;
extern int timeBeforeFirstEvents;
extern int timeBeforeServer;
extern cvar_t* cl_stanceHoldTime;
extern cvar_t* com_animCheck;
const char* sBuildId = "cod3mp";  // ?sBuildId@@3PBDB (BuildId.o @ 0x122A5D0)
extern void tlPrintf(const char* fmt, ...);

// Renderer export (refexport_t from cl_scr.cpp re_api2 pattern)
struct refexport_t {
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
    void (*DrawStretchPic)(float, float, float, float, float, float, float,
                           float, void*);
    void (*DrawStretchPicGradient)(float, float, float, float, float, float,
                                   float, float, void*, const float*, int);
    void (*DrawStretchPicRotate)(float, float, float, float, float, float,
                                 float, float, float, void*);
    void (*DrawQuadPic)(const float (*)[2], const float (*)[2], void*);
    void (*DrawStretchRaw)(int, int, int, int, int, int,
                           const unsigned char*, int, int);
    void (*UploadCinematic)(int, int, int, int, const unsigned char*, int, int);
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
    void (*Text_Paint)(float, float, int, float, const float*, const char*,
                       float, int, int);
    int (*Text_ConsoleWidth)(const short*, int, float, float, int);
    void (*Text_ConsolePaint)(float, float, int, float, const float*,
                              const short*, float, int, int);
    void (*Text_PaintWithCursor)(float, float, int, float, const float*,
                                 const char*, int, char, float, int, int);
};
refexport_t re;  // ?re@@3Urefexport_t@@A (cl.o @ 0x12FC150)

// ServerTime singleton
extern struct ServerTime_s {
    unsigned int mNumTicksElapsed;
    int mTickMSec;
    float mTickDelta;
    float mTickDeltaInv;
    float mElapsedTime;
} ServerTime_sInst;

// NumBanks (pak loading; IDA local type, size 0x20)
class NumBanks {
public:
    struct Ps3Banks {
        float main;  // +0x00
        float lram;  // +0x04
    };
    struct GcBanks {
        float main;  // +0x00
        float aram;  // +0x04
    };

    float    ps2;      // +0x00
    Ps3Banks ps3;      // +0x04
    float    xbox;     // +0x0C
    float    xenon;    // +0x10
    float    pcx;      // +0x14
    GcBanks  gc;       // +0x18
};

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
extern bool gAssertsEnabled;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Error(const char* fmt, ...);
bool Warning(const char* fmt, ...);
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

// ============================================================================
// Filter / hash / time
// ============================================================================

// ea: 0x004BAC00
char* Com_StringContains(char* str1, char* str2, int casesensitive)
{
    char* v3 = str2;
    int len = (int)strlen(str1) - (int)strlen(str2);
    int i = 0;
    if (len < 0)
        return nullptr;
    int v4 = (int)(str1 - str2);
    while (1)
    {
        int j = 0;
        if (*v3 != 0)
        {
            char* v5 = v3;
            do
            {
                int v6;
                if (casesensitive != 0)
                {
                    v6 = v5[v4] == *v5;
                }
                else
                {
                    int v7 = toupper((unsigned char)v5[v4]);
                    v6 = v7 == toupper((unsigned char)*v5);
                    v3 = str2;
                }
                if (!v6)
                    break;
                char v8 = *++v5;
                ++j;
                if (v8 == 0)
                    break;
            }
            while (1);
        }
        if (v3[j] == 0)
            break;
        ++v4;
        int v9 = ++i <= len;
        ++str1;
        if (!v9)
            return nullptr;
    }
    return str1;
}

// ea: 0x004BACC0
int Com_Filter(char* filter, char* name, int casesensitive)
{
    char* v3 = filter;
    char v4 = *filter;
    if (*filter == 0)
        return 1;
    char* v5 = name;
    while (v4 != 42)
    {
        if (v4 == 63)
            goto advance;
        if (v4 != 91)
        {
            int v17;
            if (casesensitive != 0)
            {
                v17 = v4 == *v5;
            }
            else
            {
                int v18 = toupper((unsigned char)v4);
                v17 = v18 == toupper((unsigned char)*v5);
            }
            if (v17)
                goto advance;
            return 0;
        }
        char v9 = *++v3;
        if (v9 == 91)
            goto advance;
        char v10 = *v3;
        int v11 = 0;
        if (*v3 == 0)
            return 0;
        while (v11 == 0)
        {
            if (v10 == 93 && v3[1] != 93)
                return 0;
            if (v3[1] != 45 || (char)v3[2] == 0
                || ((char)v3[2] == 93 && v3[3] != 93))
            {
                int v14;
                if (casesensitive != 0)
                {
                    v14 = v10 == *v5;
                }
                else
                {
                    int filterb = toupper((unsigned char)v10);
                    int v15 = toupper((unsigned char)*v5);
                    v5 = name;
                    v14 = filterb == v15;
                }
                if (v14)
                    v11 = 1;
                ++v3;
                goto rangeCheck;
            }
            if (casesensitive == 0)
            {
                int filtera = toupper((unsigned char)v10);
                if (toupper((unsigned char)*v5) >= filtera)
                {
                    int v13 = toupper((unsigned char)*name);
                    if (v13 <= toupper((unsigned char)v3[2]))
                        v11 = 1;
                }
                v5 = name;
                v3 += 3;
                goto rangeCheck;
            }
            if (*v5 < v10 || *v5 > v3[2])
            {
                v3 += 3;
                goto rangeCheck;
            }
            v11 = 1;
            v3 += 3;
        rangeCheck:
            v10 = *v3;
            if (*v3 == 0)
            {
                if (v11 == 0)
                    return 0;
                break;
            }
        }
        for (char i = *v3; i != 0; i = *++v3)
        {
            if (i == 93 && v3[1] != 93)
                break;
        }
    advance:
        ++v3;
        ++v5;
        name = v5;
        v4 = *v3;
        if (*v3 == 0)
            return 1;
    }
    char v6 = *++v3;
    int v7 = 0;
    char buf[128];
    if (v6 != 0)
    {
        do
        {
            if (v6 == 42 || v6 == 63)
                break;
            ++v3;
            buf[v7] = v6;
            v6 = *v3;
            ++v7;
        }
        while (*v3 != 0);
    }
    buf[v7] = 0;
    if (strlen(buf) == 0)
    {
        v4 = *v3;
        if (*v3 == 0)
            return 1;
        goto advance;
    }
    char* v8 = Com_StringContains(v5, buf, casesensitive);
    if (v8 != nullptr)
    {
        v5 = &v8[strlen(buf)];
        name = v5;
        v4 = *v3;
        if (*v3 == 0)
            return 1;
        goto advance;
    }
    return 0;
}

// ea: 0x004BAEC0
int Com_FilterPath(char* filter, char* name, int casesensitive)
{
    char new_filter[128];
    char new_name[128];
    char* v3 = filter;
    int v4 = 0;
    do
    {
        char v6 = *v3;
        if (*v3 == 0)
            break;
        new_filter[v4] = v6 == 92 || v6 == 58 ? 47 : v6;
        ++v4;
        ++v3;
    }
    while (v4 < 127);
    char* v7 = name;
    new_filter[v4] = 0;
    int v8 = 0;
    do
    {
        char v10 = *v7;
        if (*v7 == 0)
            break;
        new_name[v8] = v10 == 92 || v10 == 58 ? 47 : v10;
        ++v8;
        ++v7;
    }
    while (v8 < 127);
    new_name[v8] = 0;
    return Com_Filter(new_filter, new_name, casesensitive);
}

// ea: 0x004BAF50
int Com_HashKey(char* string, int maxlen)
{
    int v2 = 0;
    int v3 = 0;
    if (maxlen > 0)
    {
        char* v4 = string;
        do
        {
            if (*v4 == 0)
                break;
            v2 += *v4 * (v4 - string + 119);
            ++v3;
            ++v4;
        }
        while (v3 < maxlen);
    }
    return v2 ^ ((v2 ^ (v2 >> 10)) >> 10);
}

// ea: 0x004BAFA0
int Com_RealTime(qtime_s* qtime)
{
    time_t now = time(nullptr);
    int result = (int)now;
    time_t t = now;
    if (qtime != nullptr)
    {
        tm* v2 = localtime(&t);
        if (v2 != nullptr)
        {
            qtime->tm_sec = v2->tm_sec;
            qtime->tm_min = v2->tm_min;
            qtime->tm_hour = v2->tm_hour;
            qtime->tm_mday = v2->tm_mday;
            qtime->tm_mon = v2->tm_mon;
            qtime->tm_year = v2->tm_year;
            qtime->tm_wday = v2->tm_wday;
            qtime->tm_yday = v2->tm_yday;
            qtime->tm_isdst = v2->tm_isdst;
        }
        return (int)t;
    }
    return result;
}

// ea: 0x004BB020
void Com_InitZoneMemory()
{
}

// ea: 0x004BB0C0
void Com_Memset(void* dest, int val, unsigned int count)
{
    memset(dest, val, count);
}

// ea: 0x004BB170
int Com_Memcmp(const void* src0, const void* src1, unsigned int count)
{
    return memcmp(src0, src1, count) == 0 ? 1 : 0;
}

// ea: 0x004BB210
enum e_prefetch {
    PRE_READ = 0,
    PRE_READ_WRITE = 1,
    PRE_WRITE = 2,
};
void Com_Prefetch(const void* s, unsigned int bytes, e_prefetch type)
{
    if (type == 0 || type == 1)  // PRE_READ / PRE_READ_WRITE
    {
        int v4 = bytes;
        if (bytes > 4096)
            v4 = 4096;
        const char* p = (const char*)s;
        for (unsigned int i = (unsigned int)((v4 + 31) >> 5); i != 0; --i)
            p += 32;
    }
}

// ea: 0x004BB310
void Com_BeginRedirect(char* buffer, int buffersize, void (*flush)(char*))
{
    if (buffer != nullptr && buffersize != 0 && flush != nullptr)
    {
        rd_buffer = buffer;
        rd_buffersize = buffersize;
        rd_flush = flush;
        *buffer = 0;
    }
}

// ea: 0x004BB340
void Com_EndRedirect()
{
    if (rd_flush != nullptr)
        rd_flush(rd_buffer);
    rd_buffer = nullptr;
    rd_buffersize = 0;
    rd_flush = nullptr;
}

// ea: 0x004BB380
void Com_PrintMessage(print_msg_type_t type, const char* msg)
{
    if (rd_buffer != nullptr)
    {
        if (type != 4)  // PMSG_LOGFILE
        {
            if (strlen(msg) + strlen(rd_buffer) > (unsigned int)(rd_buffersize - 1))
            {
                rd_flush(rd_buffer);
                *rd_buffer = 0;
            }
            Q_strcat(rd_buffer, rd_buffersize, msg);
        }
    }
    else if (type != 4)
    {
        CL_ConsolePrint((print_msg_type_t)type, msg, 0, 0, 0);
        printf("%s", msg);
    }
}

// ea: 0x004BB420
void Com_Printf(const char* fmt, ...)
{
    char msg[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, 0x1000u, fmt, ap);
    va_end(ap);
    msg[4095] = 0;
    if (rd_buffer != nullptr)
    {
        if (strlen(msg) + strlen(rd_buffer) > (unsigned int)(rd_buffersize - 1))
        {
            rd_flush(rd_buffer);
            *rd_buffer = 0;
        }
        Q_strcat(rd_buffer, rd_buffersize, msg);
    }
    else
    {
        CL_ConsolePrint((print_msg_type_t)0, msg, 0, 0, 0);
        printf("%s", msg);
    }
}

// ea: 0x004BB550
void Com_DPrintf(const char* fmt, ...)
{
    if (com_developer && com_developer->integer)
    {
        char msg[4096];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(msg, 0x1000u, fmt, ap);
        va_end(ap);
        msg[4095] = 0;
        Com_Printf("%s", msg);
    }
}

// ea: 0x004BB650
void Com_ParseCommandLine(char* commandLine)
{
    char* v1 = commandLine;
    int v2 = 1;
    com_consoleLines[0] = commandLine;
    com_numConsoleLines = 1;
    if (*commandLine != 0)
    {
        do
        {
            if (*v1 == 43 || *v1 == 10)
            {
                if (v2 == 32)
                    return;
                com_consoleLines[v2] = v1 + 1;
                com_numConsoleLines = v2 + 1;
                *v1 = 0;
                v2 = com_numConsoleLines;
            }
            char v3 = *++v1;
            if (v3 == 0)
                break;
        }
        while (1);
    }
}

// ea: 0x004BB6B0
int Com_SafeMode()
{
    int v0 = 0;
    if (com_numConsoleLines <= 0)
        return com_safemode;
    while (1)
    {
        Cmd_TokenizeString(com_consoleLines[v0]);
        const char* v1 = Cmd_Argv(0);
        if (Q_stricmp(v1, "safe") == 0)
            break;
        const char* v2 = Cmd_Argv(0);
        if (Q_stricmp(v2, "cvar_restart") == 0)
            break;
        if (++v0 >= com_numConsoleLines)
            return com_safemode;
    }
    *com_consoleLines[v0] = 0;
    return 1;
}

// ea: 0x004BB730
void Com_ForceSafeMode()
{
    com_safemode = 1;
}

// ea: 0x004BB740
int Com_AddStartupCommands()
{
    int v0 = 0;
    for (int i = 0; i < com_numConsoleLines; ++i)
    {
        const char* v2 = com_consoleLines[i];
        if (v2 != nullptr && *v2 != 0)
        {
            if (Q_stricmpn(v2, "set", 3) != 0)
                v0 = 1;
            Cbuf_AddText(com_consoleLines[i]);
            Cbuf_AddText("\n");
        }
    }
    return v0;
}

// ============================================================================
// Event queue
// ============================================================================

// ea: 0x004BB8A0
void Com_InitPushEvent()
{
    memset(com_pushedEvents, 0, sizeof(com_pushedEvents));
    com_pushedEventsHead = 0;
    com_pushedEventsTail = 0;
}

// ea: 0x004BB8C0
void Com_PushEvent(sysEvent_t* event)
{
    sysEvent_t* v1 = &com_pushedEvents[com_pushedEventsHead & 0xFF];
    if (com_pushedEventsHead - com_pushedEventsTail < 256)
    {
        printedWarning = 0;
    }
    else
    {
        if (printedWarning == 0)
        {
            printedWarning = 1;
            ASSERT("0", "c:\\cod\\code\\game\\common.cpp", 1530);
        }
        if (v1->evPtr != 0)
            mem_heap_free(v1->evPtr);
        ++com_pushedEventsTail;
    }
    v1->evTime = event->evTime;
    v1->evType = event->evType;
    v1->evValue = event->evValue;
    v1->evValue2 = event->evValue2;
    v1->evPtrLength = event->evPtrLength;
    v1->evPtr = event->evPtr;
    com_pushedEventsHead = com_pushedEventsHead + 1;
}

// ea: 0x004BB9A0
void Com_ShutdownEvents()
{
    int v0 = com_pushedEventsTail;
    while (com_pushedEventsHead > v0)
    {
        void* evPtr = com_pushedEvents[v0 & 0xFF].evPtr;
        ++v0;
        com_pushedEventsTail = v0;
        if (evPtr != nullptr)
        {
            mem_heap_free(evPtr);
            v0 = com_pushedEventsTail;
        }
    }
}

// ea: 0x004BBA10
void Com_RunAndTimeServerPacket(netadr_t* evFrom, msg_t* buf)
{
    int v2 = 0;
    if (com_speeds->integer != 0)
        v2 = Sys_Milliseconds();
    SV_PacketEvent(*evFrom, buf);
    if (com_speeds->integer != 0)
    {
        int v3 = Sys_Milliseconds() - v2;
        if (com_speeds->integer == 3)
            Com_Printf("SV_PacketEvent time: %i\n", v3);
    }
}

// ea: 0x004BBAB0
int Com_ModifyMsec(int msec)
{
    int integer = com_fixedtime->integer;
    if (integer == 0)
    {
        if (com_timescale->value == 0.0f)
            integer = msec;
        else
            integer = (int)(msec * com_timescale->value);
    }
    if (integer < 1 && com_timescale->value != 0.0f)
        return 1;
    if (integer > 1000)
        return 1000;
    if (integer < 0)
    {
        ASSERT("msec >= 0", "c:\\cod\\code\\game\\common.cpp", 2877);
    }
    return integer;
}

// ea: 0x004BBBE0
float Com_GetScreenTimeDelta()
{
    static int initialized = 0;
    if (!initialized)
    {
        initialized = 1;
        last_time = Sys_Milliseconds();
    }
    int v0 = Sys_Milliseconds();
    float result = (v0 - last_time) / 1000.0f;
    g_time_inc = (v0 - last_time) / 1000.0f;
    last_time = v0;
    return result;
}

// ============================================================================
// DObj / weapon-info memory
// ============================================================================

// ea: 0x004BBEE0
void Com_InitDObj()
{
    g_bDObjInited = 1;
}

// ea: 0x004BBEF0
void Com_ShutdownDObj()
{
    if (g_bDObjInited != 0)
        g_bDObjInited = 0;
}

// ea: 0x004BBF10
void Com_Restart()
{
    if (g_bDObjInited != 0)
        g_bDObjInited = 0;
    XAnimShutdown();
    XAnimInit();
    g_bDObjInited = 1;
}

// ea: 0x004BBF40
void Com_XAnimFreeTree(XAnimTree* animtree)
{
        XAnimFreeTree(animtree);
}

// ea: 0x004BBF80
void Com_XAnimFreeSmallTree(XAnimTree* animtree)
{
        XAnimFreeTree(animtree);
}

// ea: 0x004BBF90
void* Com_GetWeaponInfoMemory(int iSize, int* piParsed, int iSource)
{
    ASSERT("(iSource == 1) || (iSource == 2)",
           "c:\\cod\\code\\game\\common.cpp", 4174);
    ASSERT("iSize > 0", "c:\\cod\\code\\game\\common.cpp", 4175);
    ASSERT("piParsed", "c:\\cod\\code\\game\\common.cpp", 4176);
    if (iSize <= 0)
        return nullptr;
    if (pWeaponInfoMemory != nullptr)
    {
        *piParsed = iWeaponInfoSource;
        if (iWeaponInfoSource == 0)
        {
            iWeaponInfoSource = iSource;
            return pWeaponInfoMemory;
        }
    }
    else
    {
        pWeaponInfoMemory = mem_heap_malloc_ctx((unsigned int)iSize, 16,
                                                "hunk",
                                                "c:\\cod\\code\\game\\common.cpp",
                                                4192);
        memset(pWeaponInfoMemory, 0, iSize);
        iWeaponInfoSource = iSource;
        *piParsed = 0;
    }
    return pWeaponInfoMemory;
}

// ea: 0x004BC100
void Com_FreeWeaponInfoMemory(int iSource, int bRestart)
{
    ASSERT("(iSource == 1) || (iSource == 2)",
           "c:\\cod\\code\\game\\common.cpp", 4209);
    if (iSource == iWeaponInfoSource)
    {
        if (bRestart == 0)
        {
            mem_heap_free(pWeaponInfoMemory);
            pWeaponInfoMemory = nullptr;
        }
        iWeaponInfoSource = 0;
    }
}

// ea: 0x004BC190
const char* Com_SurfaceTypeToName(int iTypeIndex)
{
    if (iTypeIndex <= 0 || iTypeIndex >= 23)
        return "default";
    return surfaceTypeNames[iTypeIndex];
}

// ea: 0x004BC210
bool Com_ControllerValid(int controller_port)
{
    if (g_controllerConnectedErrorShown[controller_port])
        return false;
    return controller::inst()->controller_is_connected(controller_port);
}

// ea: 0x004BCB80
void Com_CvarDump(print_msg_type_t type)
{
    char message[8192];
    char* match;
    if (Cmd_Argc() <= 1)
        match = nullptr;
    else
        match = (char*)Cmd_Argv(1);
    if (type || (com_logfile && com_logfile->integer))
    {
        int v1 = 0;
        Com_PrintMessage(type, "=============================== CVAR DUMP "
                               "========================================\n");
        for (cvar_t* i = cvar_vars; i; ++v1)
        {
            if (!match || Com_Filter(match, i->name, 0))
            {
                if (i->latchedString)
                {
                    Com_sprintf(message, 0x2000, "      %s \"%s\" -- latched "
                                                 "\"%s\"\n",
                                i->name, i->string, i->latchedString);
                }
                else
                {
                    Com_sprintf(message, 0x2000, "      %s \"%s\"\n",
                                i->name, i->string);
                }
                Com_PrintMessage(type, message);
            }
            i = i->next;
        }
        Com_sprintf(message, 0x2000, "\n%i total cvars\n%i cvar indexes\n",
                    v1, cvar_numIndexes);
        Com_PrintMessage(type, message);
        Com_PrintMessage(type, "=============================== END CVAR DUMP "
                               "=====================================\n");
    }
}

// ea: 0x004C0390
void Com_Memcpy(void* dest, const void* src, unsigned int count)
{
    ASSERT("src", "c:\\cod\\code\\game\\com_shared.cpp", 469);
    ASSERT("dest", "c:\\cod\\code\\game\\com_shared.cpp", 470);
    if (count > 0)
        memcpy(dest, src, count);
}

// ea: 0x004C0540
void Com_Close()
{
    if (g_bDObjInited != 0)
        g_bDObjInited = 0;
    XAnimShutdown();
    if (logfile != 0)
    {
        FS_FCloseFile(logfile);
        logfile = 0;
    }
    if (com_journalFile != 0)
    {
        FS_FCloseFile(com_journalFile);
        com_journalFile = 0;
    }
}

// ea: 0x004C9060
void Com_VPrintf(const char* fmt, char* argptr)
{
    char msg[4096];
    vsnprintf(msg, 0x1000u, fmt, argptr);
    if (rd_buffer)
    {
        if (strlen(msg) + strlen(rd_buffer) > (unsigned int)(rd_buffersize - 1))
        {
            rd_flush(rd_buffer);
            *rd_buffer = 0;
        }
        Q_strcat(rd_buffer, rd_buffersize, msg);
    }
    else if (com_logfile && com_logfile->integer)
    {
        int v3 = logfile;
        if (logfile)
            goto writeLog;
        if (fs_searchpaths && !opening_qconsole)
        {
            opening_qconsole = 1;
            logfile = FS_FOpenFileWrite("etconsole.log");
            Com_Printf("logfile opened at some time\n");
            if (com_logfile->integer > 1)
                FS_ForceFlush(logfile);
            v3 = logfile;
            opening_qconsole = 0;
            if (logfile)
            {
            writeLog:
                if (fs_searchpaths)
                    FS_Write(msg, (unsigned int)strlen(msg), v3);
            }
        }
    }
}

// ea: 0x004C91C0
void Com_StartupVariable(const char* match)
{
    for (int i = 0; i < com_numConsoleLines; ++i)
    {
        Cmd_TokenizeString(com_consoleLines[i]);
        if (strcmp(Cmd_Argv(0), "set") == 0)
        {
            const char* v1 = Cmd_Argv(1);
            const char* v2 = v1;
            if (match == nullptr || strcmp(v1, match) == 0)
            {
                const char* v3 = Cmd_Argv(2);
                Cvar_Set2(v2, v3, 1);
                cvar_t* v4 = Cvar_Get(v2, "", 0);
                v4->flags |= 0x80u;
            }
        }
    }
}

// ea: 0x004C92A0
void Com_InitJournaling()
{
    Com_StartupVariable("journal");
    com_journal = Cvar_Get("journal", "0", 16);
    int integer = com_journal->integer;
    if (integer != 0)
    {
        if (integer == 1)
        {
            Com_Printf("Journaling events\n");
            com_journalFile = FS_FOpenFileWrite("journal.dat");
            com_journalDataFile = FS_FOpenFileWrite("journaldata.dat");
        }
        else if (integer == 2)
        {
            Com_Printf("Replaying journaled events\n");
            com_fileAccessed = 1;
            FS_FOpenFileRead_Internal("journal.dat", &com_journalFile, 1, 0);
            com_fileAccessed = 1;
            FS_FOpenFileRead_Internal("journaldata.dat", &com_journalDataFile,
                                      1, 0);
        }
        if (com_journalFile == 0 || com_journalDataFile == 0)
        {
            Cvar_Set2("com_journal", "0", 1);
            com_journalFile = 0;
            com_journalDataFile = 0;
            Com_Printf("Couldn't open journal files\n");
        }
    }
}

// ea: 0x004C93A0
sysEvent_t Com_GetRealEvent()
{
    sysEvent_t result;
    int ev;
    sysEventType_t evType = SE_NONE;
    int evValue = 0;
    int evValue2 = 0;
    unsigned int size = 0;
    void* buffer = nullptr;
    if (com_journal->integer == 2)
    {
        if (FS_Read((unsigned char*)&ev, 24, com_journalFile) != 24)
            Com_Error((errorParm_t)0, "EXE_ERR_JOURNAL_FILE_READ");
        unsigned int evPtrLength = size;
        if (size)
        {
            int v2 = size;
            void* v3 = mem_heap_malloc(size);
            if (!v3 && v2 > 0)
                Sys_OutOfMemError();
            memset(v3, 0, v2);
            buffer = v3;
            int v4 = FS_Read((unsigned char*)v3, size, com_journalFile);
            evPtrLength = size;
            if (v4 != (int)size)
                Com_Error((errorParm_t)0, "EXE_ERR_JOURNAL_FILE_READ");
        }
        evType = (sysEventType_t)evValue2;
        evValue = evValue;
    }
    else
    {
        sysEvent_t Event = Sys_GetEvent();
        ev = Event.evTime;
        evType = Event.evType;
        evValue = Event.evValue;
        evValue2 = Event.evValue2;
        unsigned int evPtrLength = Event.evPtrLength;
        size = evPtrLength;
        buffer = Event.evPtr;
        if (com_journal->integer == 1)
        {
            if (FS_Write((char*)&ev, 24, com_journalFile) != 24)
                Com_Error((errorParm_t)0, "EXE_ERR_JOURNAL_FILE_WRITE");
            if (size)
            {
                int v6 = FS_Write((char*)buffer, size, com_journalFile);
                if (v6 != (int)size)
                    Com_Error((errorParm_t)0, "EXE_ERR_JOURNAL_FILE_WRITE");
            }
        }
    }
    result.evTime = ev;
    result.evType = evType;
    result.evValue = evValue;
    result.evValue2 = evValue2;
    result.evPtrLength = (int)size;
    result.evPtr = buffer;
    return result;
}

// ea: 0x004C9500
sysEvent_t Com_GetEvent()
{
    sysEvent_t result;
    unsigned char v1 = (unsigned char)com_pushedEventsTail;
    if (com_pushedEventsHead <= com_pushedEventsTail)
    {
        result = Com_GetRealEvent();
        return result;
    }
    else
    {
        ++com_pushedEventsTail;
        sysEvent_t* e = &com_pushedEvents[v1 & 0xFF];
        result.evTime = e->evTime;
        result.evType = e->evType;
        result.evValue = e->evValue;
        result.evValue2 = e->evValue2;
        result.evPtrLength = e->evPtrLength;
        result.evPtr = e->evPtr;
    }
    return result;
}

// ea: 0x004C9AC0
int Com_Milliseconds()
{
    int v2 = 0;
    while (1)
    {
        sysEvent_t ev;
        sysEvent_t r2 = Com_GetRealEvent();
        sysEvent_t* RealEvent = &r2;
        sysEventType_t v1 = RealEvent->evType;
        v2 = RealEvent->evTime;
        ev.evValue = RealEvent->evValue;
        ev.evValue2 = RealEvent->evValue2;
        int v3 = RealEvent->evPtrLength;
        void* v4 = RealEvent->evPtr;
        ev.evTime = v2;
        ev.evType = v1;
        ev.evPtrLength = v3;
        ev.evPtr = v4;
        if (v1 == SE_NONE)
            break;
        Com_PushEvent(&ev);
    }
    return v2;
}

// ea: 0x004C9B10
void Com_PumpMessageLoop()
{
    sysEvent_t v11;
    sysEvent_t ev;
    v11 = Com_GetRealEvent();
    sysEvent_t* RealEvent = &v11;
    int v1 = RealEvent->evTime;
    ev.evValue = RealEvent->evValue;
    int v2 = RealEvent->evValue2;
    ev.evTime = v1;
    sysEventType_t v3 = RealEvent->evType;
    ev.evValue2 = v2;
    int v4 = RealEvent->evPtrLength;
    void* result = RealEvent->evPtr;
    ev.evType = v3;
    ev.evPtrLength = v4;
    ev.evPtr = result;
    sysEventType_t v9 = v3;
    if (v3 != SE_NONE)
    {
        do
        {
            Com_PushEvent(&ev);
            v11 = Com_GetRealEvent();
            sysEvent_t* v6 = &v11;
            int v7 = v6->evTime;
            ev.evValue = v6->evValue;
            int v8 = v6->evValue2;
            ev.evTime = v7;
            v9 = v6->evType;
            ev.evValue2 = v8;
            int v10 = v6->evPtrLength;
            result = v6->evPtr;
            ev.evType = v9;
            ev.evPtrLength = v10;
            ev.evPtr = result;
        }
        while (v9 != SE_NONE);
    }
}

// ============================================================================
// Config writing
// ============================================================================

// ea: 0x004C9E40
void Com_WriteConfigToFile(const char* filename)
{
    int v1 = FS_FOpenFileWrite(filename);
    int v2 = v1;
    if (v1 != 0)
    {
        FS_Printf(v1, "// generated by Call of Duty, do not modify\n");
        Key_WriteBindings(v2);
        PadAliasMgr_WriteBindings(v2);
        Cvar_WriteVariables(v2);
        FS_FCloseFile(v2);
    }
    else
    {
        Com_Printf("Couldn't write %s.\n", filename);
    }
}

// ea: 0x004C9EA0
void Com_WriteDefaultsToFile(const char* filename)
{
    int v1 = FS_FOpenFileWrite(filename);
    int v2 = v1;
    if (v1 != 0)
    {
        FS_Printf(v1, "// generated by Call of Duty, do not modify\n");
        Cvar_WriteDefaults(v2);
        FS_FCloseFile(v2);
    }
    else
    {
        Com_Printf("Couldn't write %s.\n", filename);
    }
}

// ea: 0x004C9EF0
void Com_WriteConfiguration()
{
    if (com_fullyInitialized != 0 && (cvar_modifiedFlags & 1) != 0)
    {
        cvar_modifiedFlags &= ~1u;
        if (!gUseNfl)
            Com_WriteConfigToFile("config\\bro.cfg");
    }
}

// ea: 0x004C9F30
void Com_WriteConfig_f()
{
    char filename[128];
    if (Cmd_Argc() == 2)
    {
        const char* v0 = Cmd_Argv(1);
        Q_strncpyz(filename, v0, 128);
        Com_DefaultExtension(filename, 128, ".cfg");
        Com_Printf("Writing %s.\n", filename);
        Com_WriteConfigToFile(filename);
    }
    else
    {
        Com_Printf("Usage: writeconfig <filename>\n");
    }
}

// ea: 0x004C9FA0
void Com_WriteDefaults_f()
{
    char filename[128];
    if (Cmd_Argc() == 2)
    {
        const char* v0 = Cmd_Argv(1);
        Q_strncpyz(filename, v0, 128);
        Com_DefaultExtension(filename, 128, ".cfg");
        Com_Printf("Writing %s.\n", filename);
        Com_WriteDefaultsToFile(filename);
    }
    else
    {
        Com_Printf("Usage: writedefaults <filename>\n");
    }
}

// ea: 0x004C9590
int Com_EventLoop()
{
    unsigned char bufData[3072];
    sysEvent_t result;
    msg_t buf;
    netadr_t evFrom;
    MSG_Init(&buf, bufData, 3072);
    result = Com_GetEvent();
    sysEvent_t* Event = &result;
    unsigned int v1 = Event->evType;
    int v2 = Event->evValue;
    unsigned int v3 = Event->evPtrLength;
    char* v4 = (char*)Event->evPtr;
    unsigned int ev = Event->evTime;
    int v5 = Event->evValue2;
    int key = v2;
    int v8 = 0;
    for (int down = v5; v1; down = v8)
    {
        switch (v1)
        {
        case 1u:  // SE_KEY
            ASSERT("!ev.evPtr", "c:\\cod\\code\\game\\common.cpp", 1673);
            CL_KeyEvent(key, down, ev);
            break;
        case 2u:  // SE_CHAR
            ASSERT("!ev.evPtr", "c:\\cod\\code\\game\\common.cpp", 1677);
            CL_CharEvent(key);
            break;
        case 3u:  // SE_MOUSE
            ASSERT("!ev.evPtr", "c:\\cod\\code\\game\\common.cpp", 1681);
            CL_MouseEvent(key, down, (int)ev);
            break;
        case 4u:  // SE_JOYSTICK
            ASSERT("!ev.evPtr", "c:\\cod\\code\\game\\common.cpp", 1691);
            CL_GamepadEvent(key, down, (int)ev);
            break;
        case 5u:  // SE_CONSOLE
            ASSERT("ev.evPtr", "c:\\cod\\code\\game\\common.cpp", 1695);
            Cbuf_AddText(v4);
            mem_heap_free(v4);
            Cbuf_AddText("\n");
            break;
        case 6u:  // SE_PACKET
            ASSERT("ev.evPtr", "c:\\cod\\code\\game\\common.cpp", 1701);
            memcpy(&evFrom, v4, sizeof(netadr_t));
            buf.cursize = (int)v3 - 20;
            if ((int)(v3 - 20) <= buf.maxsize)
            {
                memcpy(buf.data, v4 + 20, v3 - 20);
                mem_heap_free(v4);
                if (com_sv_running->integer)
                {
                    Com_RunAndTimeServerPacket(&evFrom, &buf);
                }
                else
                {
                    if (dword_F6A290[0] == 2)
                    {
                        currCl = NS_CLIENT;
                        CL_PacketEvent(evFrom, &buf, ev);
                    }
                    currCl = NS_CLIENT;
                }
            }
            else
            {
                mem_heap_free(v4);
                Com_Printf("Com_EventLoop: oversize packet\n");
            }
            break;
        default:
            ASSERT("!ev.evPtr", "c:\\cod\\code\\game\\common.cpp", 1666);
            Com_Error((errorParm_t)0, "Com_EventLoop: bad event type %i", v1);
            break;
        }
        result = Com_GetEvent();
        sysEvent_t* v6 = &result;
        v1 = v6->evType;
        int v7 = v6->evValue;
        v3 = v6->evPtrLength;
        v4 = (char*)v6->evPtr;
        ev = v6->evTime;
        v8 = v6->evValue2;
        key = v7;
    }
    if (v4)
    {
        ASSERT("!ev.evPtr", "c:\\cod\\code\\game\\common.cpp", 1635);
    }
    if (dword_F6A290[0])
    {
        currCl = NS_CLIENT;
        while (NET_GetLoopPacket(NS_CLIENT, &evFrom, &buf))
            CL_PacketEvent(evFrom, &buf, ev);
    }
    currCl = NS_CLIENT;
    while (NET_GetLoopPacket(NS_SERVER, &evFrom, &buf))
    {
        if (com_sv_running->integer)
            Com_RunAndTimeServerPacket(&evFrom, &buf);
    }
    return ev;
}

// ea: 0x004C35A0
void Com_Error(errorParm_t code, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf(com_errorMessage, fmt, ap);
    va_end(ap);
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\common.cpp";
    AeAssert::gCurrentLine = 1017;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error(com_errorMessage))
        __debugbreak();
    if (com_errorEntered != 0)
        tlFatal("recursive error after: %s", com_errorMessage);
    char szUnlocedMsg[4096];
    int v2 = 0;
    char v3 = 0;
    do
    {
        v3 = com_errorMessage[v2];
        szUnlocedMsg[v2++] = v3;
    }
    while (v3 != 0);
    com_errorEntered = 1;
    if (com_errorMessage[0] != 0)
    {
        const char* v4 = SEH_LocalizeTextMessage(com_errorMessage,
                                                 "error message",
                                                 LOCMSG_NOERR);
        if (v4 != nullptr)
            Q_strncpyz(com_errorMessage, v4, 4096);
    }
    if (fs_debug != nullptr && fs_debug->integer == 2)
        Cvar_Set2("fs_debug", "0", 1);
    Com_CleanupSkeletons();
    Com_ResetParseSessions();
    if (re.ResetImageAllocations != nullptr)
        re.ResetImageAllocations();
    fs_loadStack = 0;
    if (code == 1)  // ERR_DROP
        Cbuf_Init();
    if (iWeaponInfoSource == 1 || iWeaponInfoSource == 2)
    {
        mem_heap_free(pWeaponInfoMemory);
        pWeaponInfoMemory = nullptr;
        iWeaponInfoSource = 0;
    }
    int v5 = Sys_Milliseconds();
    if (v5 - lastErrorTime >= 100)
        errorCount = 0;
    else
        ++errorCount;
    lastErrorTime = v5;
    CL_Shutdown();
    SV_Shutdown();
    Com_Close();
    tlFatal("%s", com_errorMessage);
}

// ea: 0x004C3730
void Com_Quit_f()
{
    if (com_errorEntered == 0)
    {
        Com_CleanupSkeletons();
        CL_Shutdown();
        SV_Shutdown();
        Com_Close();
        FS_Shutdown(1);
    }
    tlFatal("Quit in Com_Quit_f");
}

// ea: 0x004BBC70
bool Com_ControllerTest(int port)
{
    if (!g_enableControllerTest)
        return true;
    int v1 = LocalClient::ClientToPort(port);
    if (!g_controllerConnected[port])
    {
        if (&g_femanager != nullptr
            && *(void**)((char*)&g_femanager + 0x18) != nullptr)  // ControllerDisconnected
        {
            if (!g_controllerConnectedErrorShown[port])
            {
                SoundDevice_PauseAllSounds(SoundDevice::sInst);
                g_controllerConnectedGamePaused[port] =
                    v1 == 0 && GamePause::IsGamePaused(0);
                if (*(bool*)((char*)&g_femanager + 0x36))  // inGame
                {
                    void* IGMS = FEManager_GetIGMS(&g_femanager, currCl);
                    if (IGMS != nullptr && !InGameMenuSystem_IsSystemActive(IGMS))
                    {
                        void* v3 = FEManager_GetIGMS(&g_femanager, v1);
                        InGameMenuSystem_ActivatePauseMenu(v3);
                    }
                }
                else if (v1 == 0)
                {
                    GamePause_SetGamePaused(0, true);
                }
                g_controllerConnectedErrorShown[port] = true;
                FEManager_DrawControllerError(&g_femanager);
            }
            MemoryUnitManager_Service();
            SoundDevice::sInst->FrameAdvance( 0.0f);
            AudioBankMgr_Update(AudioBankMgr_sInst);
            codNflUpdate();
            return false;
        }
        return true;
    }
    if (!g_controllerConnectedErrorShown[port])
        return true;
    if (controller::inst()->button_pressed_clear(port, (controller::ButtonIndex)5))
    {
        SoundDevice_UnpauseAllSounds(SoundDevice::sInst);
        SyncFrameBuffers();
        if (v1 == 0)
            GamePause_SetGamePaused(0, g_controllerConnectedGamePaused[port]);
        g_controllerConnectedErrorShown[port] = false;
    }
    return false;
}

// ea: 0x004BBDE0
bool Com_AnyControllerConnected()
{
    int v0 = controller_num_controllers();
    int v1 = 0;
    controller::inst();
    if (v0 <= 0)
        return false;
    while (!g_controllerConnected[v1])
    {
        int v2 = controller_num_controllers();
        ++v1;
        controller::inst();
        if (v1 >= v2)
            return false;
    }
    return true;
}

// ea: 0x004BBE20
bool Com_ControllerTest()
{
    if (g_enableControllerTest)
    {
        if (*(bool*)((char*)controller::inst() + 8))  // is_locked
        {
            Com_ControllerTest(*(int*)controller::inst());  // locked_port
            return true;
        }
        if (!*(bool*)((char*)&g_femanager + 0x36))  // !inGame
        {
            if (Com_AnyControllerConnected())
            {
                if (g_controllerConnectedErrorShown[0])
                {
                    bool result = controller::inst()->button_pressed_clear(0, (controller::ButtonIndex)5);
                    if (!result)
                        return result;
                    g_controllerConnectedErrorShown[0] = false;
                    SyncFrameBuffers();
                }
            }
            else if (&g_femanager != nullptr
                     && *(void**)((char*)&g_femanager + 0x18) != nullptr)
            {
                g_controllerConnectedErrorShown[0] = true;
                FEManager_DrawControllerError(&g_femanager);
                return true;
            }
        }
    }
    return true;
}

// ea: 0x004CA010
void Com_ControllerWarningDialog(bool activate, int client)
{
    gControllerWarningDialogIsActive[LocalClient::ClientToPort(client)] = activate;
    if (activate)
    {
        const char* STBString = STBManager_GetSTBString(
            STBManager_sInst, "CGAME_XBOX_CONTROLLER_DISCONNECTED1");
        const char* v3 = STBManager_GetSTBString(
            STBManager_sInst, "CGAME_XBOX_CONTROLLER_DISCONNECTED2");
        char newString[512];
        sprintf(newString, "%s %d %s", STBString,
                *(int*)controller::inst() + 1, v3);
        void* DMS = FEManager_GetDMS(&g_femanager, client);
        DialogMenuSystem_BringUp(DMS, newString, false, false, "", true);
        DialogMenuSystem_CloseDialog(DMS);
    }
    else
    {
        void* v11 = FEManager_GetDMS(&g_femanager, client);
        DialogMenuSystem_CloseDialog(v11);
    }
}

// ea: 0x004CA120
void Com_CheckControllerUnplugged(bool signedIn, int client)
{
    if (g_controllerConnected[LocalClient::ClientToPort(client)])
    {
        if (!g_controllerConnectedErrorShown[LocalClient::ClientToPort(client)]
                && gControllerWarningDialogIsActive[LocalClient::ClientToPort(client)]
            || g_controllerConnectedErrorShown[LocalClient::ClientToPort(client)]
                && controller::inst()->controller_is_connected(
                    LocalClient::ClientToPort(client)))
        {
            g_controllerConnectedErrorShown[LocalClient::ClientToPort(client)] = false;
            if (!signedIn)
            {
                gControllerWarningDialogIsActive[0] = false;
                void* DMS = FEManager_GetDMS(&g_femanager, 0);
                DialogMenuSystem_CloseDialog(DMS);
            }
        }
    }
    else if (*(&dword_F6A290[0] + 802 * client) == 2
             && !g_controllerConnectedErrorShown[LocalClient::ClientToPort(client)])
    {
        g_controllerConnectedErrorShown[LocalClient::ClientToPort(client)] = true;
        if (*(bool*)((char*)&g_femanager + 0x36))  // inGame
        {
            void* IGMS = FEManager_GetIGMS(&g_femanager, client);
            InGameMenuSystem_ActivatePauseMenu(IGMS);
        }
        else
        {
            GamePause_SetGamePaused(client, true);
        }
        if (!signedIn)
            Com_ControllerWarningDialog(true, client);
    }
}

// ea: 0x004CA240
int Com_SaveCvarsToBuffer(const char** cvarnames, int numCvars, char* buffer,
                          int bufsize)
{
    int v4 = 0;
    if (numCvars <= 0)
        return 1;
    while (1)
    {
        const char* v5 = cvarnames[v4];
        cvar_t* v6 = hashTable[generateHashValue(v5)];
        const char* string;
        if (v6 != nullptr)
        {
            while (Q_stricmp(v5, v6->name) != 0)
            {
                v6 = v6->hashNext;
                if (v6 == nullptr)
                    goto notFound;
            }
            string = v6->string;
        }
        else
        {
        notFound:
            string = defaultFileName;
        }
        int v8 = _snprintf(buffer, bufsize, "%s \"%s\"\n", cvarnames[v4], string);
        if (v8 < 0)
            return 0;
        ++v4;
        buffer += v8;
        bufsize -= v8;
        if (v4 >= numCvars)
            return 1;
    }
}

// ea: 0x004CA2E0
int Com_LoadCvarsFromBuffer(const char** cvarnames, int numCvars,
                            const char* buffer, const char* filename)
{
    int v4 = numCvars;
    if (numCvars >= 0x10000)
    {
        ASSERT("numCvars < (sizeof(bRead) / sizeof(bRead[0]))",
               "c:\\cod\\code\\game\\common.cpp", 4262);
    }
    unsigned char bRead[65536];
    memset(bRead, 0, numCvars);
    int numRead = 0;
    Com_BeginParseSession(filename);
    const char** p = &buffer;
    const char* v5 = Com_Parse(p);
    if (*v5 != 0)
    {
        do
        {
            int v6 = 0;
            if (numCvars <= 0)
            {
                Com_Printf("^3WARNING: unknown cvar '%s' in file '%s'\n",
                           v5, filename);
            }
            else
            {
                while (_stricmp(v5, cvarnames[v6]) != 0)
                {
                    if (++v6 >= numCvars)
                        goto unknownCvar;
                }
                const char* v8 = Com_ParseOnLine(p);
                Cvar_Set2(cvarnames[v6], v8, 1);
                if (bRead[v6] == 0)
                {
                    bRead[v6] = 1;
                    ++numRead;
                }
                goto skip;
            unknownCvar:
                Com_Printf("^3WARNING: unknown cvar '%s' in file '%s'\n",
                           v5, filename);
            skip:
                ;
            }
            Com_SkipRestOfLine(p);
            v5 = Com_Parse(p);
        }
        while (*v5 != 0);
        v4 = numCvars;
    }
    Com_EndParseSession();
    if (numRead == v4)
        return 1;
    Com_Printf("^1ERROR: the following cvars were not specified in file '%s'\n",
               filename);
    for (int i = 0; i < numCvars; ++i)
    {
        if (bRead[i] == 0)
            Com_Printf("^1  %s\n", cvarnames[i]);
    }
    return 0;
}

// ea: 0x004CFC10
void Com_Frame()
{
    void* v0 = InteractionController::Inst(currCl);
    InteractionController_ChangeWeaponToPending(v0);
    Com_ControllerTest();
    int v1 = 0;
    int timeBeforeServer = 0;
    int timeBeforeEvents = 0;
    int timeBeforeFirstEvents = 0;
    if (com_fullyInitialized)
    {
        if ((cvar_modifiedFlags & 1) != 0)
        {
            cvar_modifiedFlags &= ~1u;
            if (!gUseNfl)
                Com_WriteConfigToFile("config\\bro.cfg");
        }
    }
    if (com_statmon->integer && com_fileAccessed)
    {
        StatMon_Warning(1, 3000, "gfx/2d/warning@file.jpg");
        com_fileAccessed = 0;
    }
    if (com_viewlog->modified)
    {
        com_viewlog->modified = 0;
    }
    SetAnimCheck(com_animCheck->integer);
    int integer = com_maxfps->integer;
    int v3 = 1;
    if (integer > 0)
        v3 = 1000 / integer;
    int v4;
    int v5;
    int v6;
    do
    {
        v4 = Com_EventLoop();
        v5 = lastTime;
        com_frameTime = v4;
        if (lastTime > v4)
        {
            v5 = v4;
            lastTime = v4;
        }
        v6 = v4 - v5;
    }
    while (v4 - v5 < v3);
    if (cl_capturemovie->integer)
        v6 = 33;
    lastTime = v4;
    Cbuf_Execute();
    if (cl_frameadvance->integer)
        GamePause_SetGamePaused(currCl, 0);
    int realMsec = v6;
    int v7 = Com_ModifyMsec(v6);
    if (gScreenshotInProgress)
        v7 = 0;
    float screen_time_inc = v7 / 1000.0f;
    ServerTime_sInst.mNumTicksElapsed++;
    ServerTime_sInst.mTickMSec = v7;
    if (v7 <= 1)
        ServerTime_sInst.mTickMSec = 1;
    ServerTime_sInst.mTickDeltaInv = 1000.0f / v7;
    ServerTime_sInst.mTickDelta = v7 / 1000.0f;
    ServerTime_sInst.mElapsedTime += v7 / 1000.0f;
    TaskSys_Update(ServerTime_sInst.mTickDelta);
    CurveManager_Update(CurveManager_sInst, screen_time_inc);
    DynamicDecalMgr_Update(DynamicDecalMgr_sInst, screen_time_inc);
    SmokeGrenadeMgr_Update(SmokeGrenadeMgr_sInst, ServerTime_sInst.mTickDelta);
    EntityNotifySet_UpdateList();
    update_trigger_notifies();
    subtitle_manager::frame_advance(v7);
    SV_Frame(v7);
    if (*(bool*)((char*)&g_femanager + 0x36))  // inGame
        InspectorManager_Update(&g_inspectorManager);
    Com_EventLoop();
    if (!gUseControllerLagFix)
    {
        int v8 = currCl;
        if (dword_F6A290[0] == 2)
        {
            currCl = NS_CLIENT;
            SendClientThinkMsg();
        }
        currCl = v8;
    }
    Cbuf_Execute();
    float screen_time_inca = Com_GetScreenTimeDelta();
    int v9 = -1;
    int v10 = 0;
    g_screendelta = screen_time_inca;
    gFirstCamera = 1;
    g_DOBJF_NOT_RENDERED_LAST_FRAME = 1;
    if (dword_F6A290[0])
    {
        v10 = dword_F6A290[0] == 2;
        v9 = 0;
    }
    SoundDevice_SetNumberOfListeners(SoundDevice::sInst, v10);
    LocalClient::SetFirstLocalClientIndex(v9);
    LocalClient::SetLastLocalClientIndex(0);
    if (dword_F6A290[0])
    {
        currCl = NS_CLIENT;
        CL_RecallKeys();
        CL_Frame(realMsec, screen_time_inca);
        CL_BackUpKeys();
    }
    currCl = NS_CLIENT;
    if (com_statmon->integer || com_speeds->integer)
    {
        int v11 = timeClientFrame;
        int v12 = Sys_Milliseconds();
        timeClientFrame = v12;
        if (com_statmon->integer && v12 - v11 > 33 && v11)
        {
            StatMon_Warning(0, 3000, "gfx/2d/warning@fps.jpg");
        }
        if (com_speeds->integer)
        {
            Com_Printf(
                "frame:%i all:%3i sv:%3i ev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n",
                com_frameNumber, v12 - timeBeforeServer,
                timeBeforeEvents - time_game - timeBeforeServer,
                timeBeforeServer + v1 - timeBeforeEvents - timeBeforeFirstEvents,
                v12 - time_backend - time_frontend - v1, time_game,
                time_frontend, time_backend);
        }
    }
    ++com_frameNumber;
    if (cl_frameadvance->integer)
    {
        GamePause_SetGamePaused(currCl, 1);
        Cvar_Set2("cl_frameadvance", "0", 1);
        return;
    }
}

// ea: 0x004D0A80
void Com_Init(char* commandLine)
{
    tlPrintf("\n--BUILDID: %s--\n\n", sBuildId);
    gQuickStart = 0;
    dword_F6A290[0] = 2;
    SetupPoolAllocator();
    init_dobj_trackers();
    SetupActorHeap();
    memset(com_pushedEvents, 0, sizeof(com_pushedEvents));
    com_pushedEventsHead = 0;
    com_pushedEventsTail = 0;
    cvar_cheats = Cvar_Get("sv_cheats", "0", 72);
    Cvar_AddCommands();
    gIsWorkspaceMap = 0;
    Com_ParseCommandLine(commandLine);
    Swap_Init();
    Cbuf_Init();
    Cmd_Init();
    Com_StartupVariable(nullptr);
    Com_StartupVariable("developer");
    CL_InitKeyCommands();
    CL_InitGamepadCommands();
    CL_InitGamepadAxisBindings();
    InitPadAliasCommands();
    FS_InitFilesystem();
    SEH_Init_StringEd();
    SEH_UpdateLanguageInfo();
    MI_ResetMapList();
    controller::inst();
    PadAliasMgr::CreateInst();
    GameSettings::CreateInst();
    PakManager::CreateInst();
    BankManager::CreateInst();
    InstanceBankMgr::CreateInst();
    LightGridMgr::CreateInst();
    XModelManager::CreateInst();
    XModelPartsManager::CreateInst();
    DestructibleBankManager::CreateInst();
    PhysDataBankManager::CreateInst();
    AITypeManager::CreateInst();
    SoundDevice::CreateInst();
    AudioBankMgr::CreateInst();
    SoundMediaMgr::CreateInst();
    MusicMgr::CreateInst();
    StreamZoneManager::CreateInst();
    DbTablesetMgr::CreateInst();
    EffectEventSys::CreateInst();
    GdbFileManager::CreateInst();
    DialogueManager::CreateInst();
    EntityManager::CreateInst();
    SceneManager::CreateInst();
    ConfigStringManager::CreateInst();
    PathNodeMgr::CreateInst();
    STBManager::CreateInst();
    CtrlIcon::CreateInst();
    MultiplayerMgr::CreateInst();
    CheckpointMgr::CreateInst();
    SplineMgr::CreateInst();
    SmokeGrenadeMgr::CreateInst();
    _controlfp(0x300u, 0x300u);
    _controlfp(0, 0);
    PhysInit();
    CGBankManager::CreateInst();
    DCGBankManager::CreateInst();
    AnimBankManager::CreateInst();
    RumbleManager::CreateInst();
    InteractionController::CreateInst();
    BinFileManager::CreateInst();
    CurveManager::CreateInst();
    PlayerAnimMgr::CreateInst();
    DynamicDecalMgr::CreateInst();
    InitLights();
    gDoNotPlayCampaignMovies = 0;
    TimerRenderBars_Init(TimerRenderBars_sInst);
    EntityHandleDb_Init(&EntityHandleDb::sInst);
    StatusBar_Init(nullptr);
    FEManager_InitDialogMenuSystem(&g_femanager);
    FEManager_LoadInGameMenus(&g_femanager);
    FEManager_InitIGO(&g_femanager);
    DebugRender_AddRenderer(DebugRender_sInst, (void*)DebugDumpAnims);
    DebugRender_AddRenderer(DebugRender_sInst, (void*)rb_vehicle_debug_render_all);
    DebugRender_AddRenderer(DebugRender_sInst, (void*)physics_debug_render);
    DebugRender_AddRenderer(DebugRender_sInst, (void*)fx_debug_render);
    TestFPS::CreateInst();
    CL_PreAllocStrings();
    PakManager::sInst->SetProgressCallback(GlobalPakLoadCallback);
    if (nflFileExists((nflMediaID)gNflMediaId, "debug.cod"))
    {
        NumBanks v7;
        memset(&v7, 0, sizeof(v7));
        PakManager::sInst->SyncLoadPak(kPakTypeCount, "debug.cod", v7);
    }
    NumBanks v7;
    memset(&v7, 0, sizeof(v7));
    PakManager::sInst->SyncLoadPak((EPakType)0, "mp\\global.cod", v7);
    void* PakInfo = (void*)PakManager::sInst->GetPakInfo("MPAnimation");
    if (!PakInfo)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\common.cpp";
        AeAssert::gCurrentLine = 2402;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("Unknown anim pak (perhaps rebuild global pak?)"))
            __debugbreak();
    }
    PakManager::sInst->SetUserDistance((const PakInfoNode*)PakInfo, 0.0f);
    NumBanks zero_banks;
    memset(&zero_banks, 0, sizeof(zero_banks));
    PakManager::sInst->SyncLoadPak((const PakInfoNode*)PakInfo);
    DebugRender_Init(DebugRender_sInst);
    WheelMarkMgr_Init();
    PakManager::sInst->SetProgressCallback(nullptr);
    AudioBankMgr_FinishLoading(AudioBankMgr_sInst);
    Com_InitJournaling();
    InspectorManager_Initialise(&g_inspectorManager);
    Cbuf_AddText("exec language.cfg\n");
    Cbuf_AddText("exec bro.cfg\n");
    Cbuf_AddText("exec autoexec.cfg\n");
    if (Com_SafeMode())
        Cbuf_AddText("exec safemode.cfg\n");
    LocalClient::InitializeClientControllers();
    Cbuf_Execute();
    int* p_mControllerPort = &((int*)gSaveGameData)[0x3B0 / 4];
    int* end = &((int*)gSaveGameData)[0x3B0 / 4] + 1789 * 4;
    do
    {
        if (*p_mControllerPort == -1)
            *p_mControllerPort = 0;
        StubData_ApplyStubOptions((char*)p_mControllerPort - 236);
        p_mControllerPort += 1789;
    }
    while (p_mControllerPort < end);
    Com_StartupVariable(nullptr);
    if (Cvar_Get("disable_asserts", "0", 0)->integer)
        AeAssert::gAssertsEnabled = 0;
    if (fs_memorysearchpaths != fs_searchpaths)
    {
        FS_ShutdownSearchPaths(fs_memorysearchpaths);
        fs_memorysearchpaths = fs_searchpaths;
    }
    filelist_s* v4 = fs_memorynonpackfilelist;
    if (fs_memorynonpackfilelist != fs_nonpackfilelist)
    {
        if (fs_memorynonpackfilelist)
        {
            filelist_s* next = nullptr;
            do
            {
                next = v4->next;
                mem_heap_free(v4->buildBuffer);
                mem_heap_free(v4);
                v4 = next;
            }
            while (next);
        }
        fs_memorynonpackfilelist = fs_nonpackfilelist;
    }
    cvar_modifiedFlags &= ~1u;
    com_maxfps = Cvar_Get("com_maxfps", "0", 1);
    com_developer = Cvar_Get("developer", "0", 256);
    com_developer_script = Cvar_Get("developer_script", "0", 256);
    com_logfile = Cvar_Get("logfile", "2", 0);
    com_statmon = Cvar_Get("com_statmon", "0", 0);
    com_timescale = Cvar_Get("timescale", "1", 520);
    com_fixedtime = Cvar_Get("fixedtime", "0", 512);
    com_viewlog = Cvar_Get("viewlog", "0", 512);
    com_speeds = Cvar_Get("com_speeds", "0", 0);
    cl_frameadvance = Cvar_Get("cl_frameadvance", "0", 0);
    com_sv_running = Cvar_Get("sv_running", "0", 64);
    com_cl_running = Cvar_Get("cl_running", "0", 64);
    cl_stanceHoldTime = Cvar_Get("cl_stanceHoldTime", "300", 0);
    com_animCheck = Cvar_Get("com_animCheck", "0", 0);
    if (com_developer && com_developer->integer)
    {
        Cmd_AddCommand("error", Com_Error_f);
        Cmd_AddCommand("crash", Com_Crash_f);
        Cmd_AddCommand("freeze", Com_Freeze_f);
    }
    Cmd_AddCommand("quit", Com_Quit_f);
    Cmd_AddCommand("writeconfig", Com_WriteConfig_f);
    Cmd_AddCommand("writedefaults", Com_WriteDefaults_f);
    Sys_Init();
    Netchan_Init();
    XAnimInit();
    VM_Init();
    SV_Init();
    CL_Init();
    int* v6 = &((int*)gSaveGameData)[0x3B0 / 4];
    int* end2 = &((int*)gSaveGameData)[0x3B0 / 4] + 1789 * 4;
    do
    {
        if (*v6 == -1)
            *v6 = 0;
        StubData_ApplyStubOptions((char*)v6 - 236);
        v6 += 1789;
    }
    while (v6 < end2);
    com_frameTime = Com_Milliseconds();
    Com_AddStartupCommands();
    Cvar_Set2("r_uiFullScreen", "1", 1);
    CL_StartHunkUsers();
    Cvar_Set2("com_statmon", "0", 1);
    com_fullyInitialized = 1;
    Com_Printf("--- Common Initialization Complete ---\n");
}

// ea: 0x004D1580
void Com_Shutdown()
{
    WheelMarkMgr_Exit();
    TestFPS::DeleteInst();
    MultiplayerMgr::DeleteInst();
    PlayerAnimMgr::DeleteInst();
    InteractionController::DeleteInst();
    BinFileManager::DeleteInst();
    CurveManager::DeleteInst();
    CtrlIcon::DeleteInst();
    CheckpointMgr::DeleteInst();
    SplineMgr::DeleteInst();
    STBManager::DeleteInst();
    PathNodeMgr::DeleteInst();
    ConfigStringManager::DeleteInst();
    EntityManager::DeleteInst();
    SceneManager::DeleteInst();
    CGBankManager::DeleteInst();
    DCGBankManager::DeleteInst();
    DialogueManager::DeleteInst();
    EffectEventSys::DeleteInst();
    GdbFileManager::DeleteInst();
    DbTablesetMgr::DeleteInst();
    StreamZoneManager::DeleteInst();
    MusicMgr::DeleteInst();
    SoundMediaMgr::DeleteInst();
    AudioBankMgr::DeleteInst();
    SoundDevice::DeleteInst();
    XModelPartsManager::DeleteInst();
    XModelManager::DeleteInst();
    DestructibleBankManager::DeleteInst();
    PhysDataBankManager::DeleteInst();
    AITypeManager::DeleteInst();
    LightGridMgr::DeleteInst();
    InstanceBankMgr::DeleteInst();
    BankManager::DeleteInst();
    PakManager::DeleteInst();
    RumbleManager::DeleteInst();
    DynamicDecalMgr::DeleteInst();
    if (gActorHeap != nullptr)
    {
        ((void(__thiscall**)(void*, int))gActorHeap)[0](gActorHeap, 1);
        gActorHeap = nullptr;
    }
    PhysShutdown();
    CL_Disconnect();
    CL_ShutdownAll();
    SV_Shutdown();
    CL_StartHunkUsers();
}
