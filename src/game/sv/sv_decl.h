// ============================================================================
// sv_decl.h — server globals + cross-object externs used by sv.o functions.
// Server functions live in sv_ccmds.cpp / sv_game.cpp / sv_init.cpp /
// sv_main.cpp / sv_snapshot.cpp / sv_world.cpp / sv_client.cpp.
// Source: sv.o (amalgamated game object).
// ============================================================================

#pragma once

#include "game/sv/server_types.h"
#include "game/cvar_types.h"
#include "engine/broc_types.h"

// ============================================================================
// Server globals (sv.o data section)
// ============================================================================
extern serverStatic_t svs;            // ?svs@@3UserverStatic_t@@A  0x12FA700
extern server_t       sv;             // ?sv@@3Userver_t@@A         0x12FA8A0
extern vm_s*          gvm;            // ?gvm@@3PAUvm_s@@A          0x12FA810
extern cvar_t*        sv_reloading;   // ?sv_reloading@@3PAUcvar_t@@A
extern cvar_t*        sv_showCommands;// ?sv_showCommands@@3PAUcvar_t@@A
extern cvar_t*        sv_mapname;     // ?sv_mapname@@3PAUcvar_t@@A
extern cvar_t*        sv_killserver;  // ?sv_killserver@@3PAUcvar_t@@A
extern cvar_t*        sv_serverid;    // ?sv_serverid@@3PAUcvar_t@@A
extern cvar_t*        sv_framerate_smoothing;  // ?sv_framerate_smoothing@@3PAUcvar_t@@A
extern cvar_t*        sv_gameskill;   // ?sv_gameskill@@3PAUcvar_t@@A
extern int            sv_map_restart; // ?sv_map_restart@@3HA
extern int            sv_snapshotFrameNumber; // ?sv_snapshotFrameNumber@@3HA
extern char           sv_save_filename[128]; // ?sv_save_filename@@3PADA
extern char           gNextMapArgv[64];   // ?gNextMapArgv@@3PADA
extern int            com_time;       // ?com_time@@3HA
extern int            com_inServerFrame; // ?com_inServerFrame@@3HA
extern int            bSV_AllowedAllocSkel; // ?bSV_AllowedAllocSkel@@3HA
extern bool           gExitGame;      // ?gExitGame@@3_NA
extern bool           gPumpThreads;   // ?gPumpThreads@@3_NA
extern bool           gPumpThreadsForMapChange; // ?gPumpThreadsForMapChange@@3_NA
extern bool           gMPLoadingUnthreaded;    // ?gMPLoadingUnthreaded@@3_NA
extern int            currCl;         // ?currCl@@3HA (external, common)

// ============================================================================
// Cross-object externs (unported game objects provide definitions later)
// ============================================================================
extern int            com_frameTime;            // ?com_frameTime@@3HA
extern int            com_skelTimeStamp;        // ?com_skelTimeStamp@@3HA
extern int            cvar_modifiedFlags;       // ?cvar_modifiedFlags@@3HA
extern cvar_t*        g_gameskill;              // g_gameskill
extern cvar_t*        fs_debug;                 // ?fs_debug@@3PAUcvar_t@@A

// cvar.cpp
char*  va(const char* fmt, ...);
void   Cvar_Set(const char* var_name, const char* value);
cvar_t* Cvar_Get(const char* var_name, const char* var_value, int flags);
float  Cvar_VariableValue(const char* var_name);
void   Cvar_VariableStringBuffer(const char* var_name, char* buffer, int bufsize);
char*  Cvar_InfoString(int bit);
char*  Cvar_InfoString_Big(int bit);

// common.cpp
void   Com_Printf(const char* fmt, ...);
void   Com_DPrintf(const char* fmt, ...);
void   Com_Error(int code, const char* fmt, ...);
void   Com_sprintf(char* dest, int size, const char* fmt, ...);
void   Com_Restart(void);
unsigned int Com_EventLoop(void);
int    Com_Milliseconds(void);
void   Com_InitDObj(void);
void   Com_DefaultCvar(const char* name, const char* value, int flags);

// cmd.cpp
char*  Cmd_Argv(int arg);
void   Cmd_TokenizeString(const char* text_in);
void   Cmd_ExecuteServerString(const char* text);

// fs.cpp
int    FS_ReadFile(const char* qpath, void** buffer);
void   FS_Shutdown(int closemfp);
void   FS_Restart(int checksumFeed);
void   FS_ClearMemory(void);

// net.cpp
int    Netchan_Init(void);
void   Netchan_Setup(netsrc_t sock, netchan_t* chan, netadr_t adr, int qport);
int    NET_IsLocalAddress(netadr_t adr);

// msg.cpp
int    MSG_ReadLong(msg_t* msg);
unsigned char MSG_ReadByte(msg_t* msg);
char*  MSG_ReadString(msg_t* msg);

// vm.cpp
vm_s*  VM_Create(const char* module, int (__cdecl* systemCalls)(int*));
int    VM_Call(vm_s* vm, int callnum, ...);
void   VM_Free(vm_s* vm);

// cl_*.cpp
void   CL_MapLoading(void);
void   CL_ShutdownAll(void);
void   CL_StartLoading(void);
void   CL_ConnectResponse(netadr_t from);
void   CL_ParseGamestate(Broc::string* configstrings);
void   CL_FlushDebugData(int fromServer);
void   CL_InitCGame(void);
int    CL_FirstSnapshot(void);
void   CL_ParseGamestate_cmd(void);

// Other helpers
int    Sys_Milliseconds(void);
void   SCR_UpdateScreen(void);
void   UpdateCVars(void);
int    Q_isforfilename(int c);
void   Q_strncpyz(char* dest, const char* src, int destsize);
void   XModelEnforceExist(int bEnforce);
char*  ClientConnect(DbLinkedHandle<EntityHandleDb, Entity> entity);

// fs.cpp
int    FS_FOpenFileByMode(const char* qpath, int* f, int mode);
int    FS_Read(void* buffer, int len, int f);
void   FS_FCloseFile(int f);

// common/parse helpers
const char* Com_Parse(const char** data_p);
int    Q_strcasecmp(const char* s1, const char* s2);
char   Q_CleanCharacter(char c);

// entity/player managers (namespace in the binary: ?FirstLocalClientIndex@LocalClient@@YAHXZ)
namespace LocalClient {
    int FirstLocalClientIndex(void);       // cl.o 0x52EF80
};
