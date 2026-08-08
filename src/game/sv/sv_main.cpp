// ============================================================================
// sv_main.cpp — server main loop + network channel (sv_main.cpp of sv.o)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void  G_RunFrame(int msec);
extern void  Com_Shutdown(void);
extern void  Com_EventLoop(void);
extern void  Cbuf_AddText(const char* text);
extern void  Cbuf_ExecuteText(int exec_when, const char* text);
extern void  CL_SetFrametime(int frametime, int animFrametime);
extern int   CL_IsCGameRendering(void);
extern void  SV_PreFrame(int msec);
extern int   SV_CheckLoadGame(void);
extern void  SV_SendClientMessages(void);
extern void  MSG_WriteByte(msg_t* msg, int c);
extern void  MSG_WriteLong(msg_t* msg, int c);
extern void  MSG_WriteString(msg_t* msg, const char* s);
extern void  MSG_BeginReading(msg_t* msg);
extern short MSG_ReadShort(msg_t* msg);
extern void  Netchan_Transmit(netchan_t* chan, int length, const unsigned char* data);
extern int   Netchan_Process(netchan_t* chan, msg_t* msg);
extern void  NET_OutOfBandPrint(netsrc_t sock, netadr_t adr, const char* format, ...);
extern void  SV_ExecuteClientMessage(client_s* cl, msg_t* msg);
extern const char* nullStr;
extern int   com_frameNumber;

// SV_SpawnServer externs (cross-object; core.o / game.o / filesystem)
extern char* va(const char* fmt, ...);
extern void  SV_SetExpectedHunkUsage(const char* mapname);
extern void  XModelEnforceExist(int bEnforce);
extern void  CL_MapLoading(void);
extern void  CL_ShutdownAll(void);
extern void  CL_StartLoading(void);
extern void  CL_FlushDebugData(int fromServer);
extern void  CL_InitCGame(void);
extern void  CL_FirstSnapshot(void);
extern void  CL_ConnectResponse(netadr_t from);
extern void  SV_DirectConnect(netadr_t from);
extern void  SV_ClientEnterWorld(client_s* client, int restart, int savegame);
extern void  SV_CheckLoadLevel(int savegame);
extern void  SV_SendClientMessages(void);
extern void  SV_GameSystemCalls(int* args);
extern void  SV_Startup(void);
extern void  SV_ClearServer(void);
extern void  SV_RunFrame(int msec);
extern void  SV_SetConfigstring(int index, const char* val);
extern void  SV_GetConfigstring(int index, Broc::string& str);
extern void  Com_InitDObj(void);
extern void  Com_Restart(void);
extern void  Com_Printf(const char* fmt, ...);
extern void  Com_Error(int code, const char* fmt, ...);
extern int   Com_Milliseconds(void);
extern void  Cvar_Set(const char* var_name, const char* value);
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value, int flags);
extern float Cvar_VariableValue(const char* var_name);
extern char* Cvar_InfoString(int bit);
extern char* Cvar_InfoString_Big(int bit);
extern void  FS_Shutdown(int closemfp);
extern void  FS_Restart(int checksumFeed);
extern void  FS_ClearMemory(void);
extern int   Sys_Milliseconds(void);
extern void  SCR_UpdateScreen(void);
extern void  UpdateCVars(void);
extern void  Q_strncpyz(char* dest, const char* src, int destsize);
extern void  Cmd_ExecuteServerString(const char* text);
extern void  Netchan_Init(void);
extern void  GamePause_SetAllPaused(bool paused);
extern unsigned int nflFileExists(int mediaID, const char* filename);
extern int   nglSetFrameLock(int flock);
extern void  CM_LoadMap(const char* name, int clientload, int* checksum);
extern void  FX_TermFX(void);
extern void  g_SpawnServer_Unused(void);

// PakManager extended view
struct PakInfoNode {
    int pakId;
};
extern PakManager* PakManager_sInst(void);
extern const PakInfoNode* PakManager_SyncLoadFLI(PakManager* self, int pak_type,
                                                 const char* path);
extern int  PakManager_SyncLoadPak(PakManager* self, const PakInfoNode* cpak);
extern void PakManager_SetUserDistance(PakManager* self,
                                       const PakInfoNode* cpak, float dist);
extern void PakManager_PushContext(PakManager* self, int pakId);
extern void PakManager_PopContext(PakManager* self);
extern void PakManager_FillBanks(PakManager* self);
extern void PakManager_UnloadAll(PakManager* self);

// Entity / anim / fx managers
extern void EntityManager_CreateWorld(void* self);
extern void EffectEventSys_StopAll(void* self);
extern void PathNodeMgr_CleanUpManager(void* self);
extern void PathNodeMgr_InitPaths(void* self);
extern void CGBankManager_UnloadAll(void* self);
extern void AnimBankManager_UnloadAll(void* self);
extern void AudioBankMgr_FinishLoading(void* self);
extern void InGameMenuSystem_ActivateMenu(void* self, int menu);
extern void FEManager_UpdateLoadingMenu(void* self, float percentDone);

extern int  gNflMediaId;
extern int  g_bspTree;
extern int  g_ScrFiles;
extern const PakInfoNode* sLoadingScreenInfo;
extern int  unk_F6A290;
extern bool gReturnToMenu;
extern int  sv_restartedServerId;
extern const char defaultFileName[];
extern int  MPUIInterface_BlockUntilNetReady(void);
extern int  g_controllerConnectedErrorShown[];

// cdl profilers
struct cdl_proftimer {
    void start();
    void stop();
};
extern cdl_proftimer cdl_proftimer_vmcalls;
extern cdl_proftimer cdl_proftimer_cl_msgs;

// ============================================================================
// SV_RunFrame — ea: 0x51F820
// ============================================================================
void SV_RunFrame(int msec) {
    if (++com_skelTimeStamp == 0)
        com_skelTimeStamp = 1;
    if (bSV_AllowedAllocSkel != 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
        AeAssert::gCurrentLine = 359;
        AeAssert::gCurrentExpr = "!bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 1;
    G_RunFrame(msec);
    if (bSV_AllowedAllocSkel == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
        AeAssert::gCurrentLine = 367;
        AeAssert::gCurrentExpr = "bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 0;
}

// ============================================================================
// SV_SmoothFrame — ea: 0x51F8F0
// ============================================================================
void SV_SmoothFrame() {
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)2;   // SLB
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
    AeAssert::gCurrentLine = 557;
    AeAssert::gCurrentExpr = NULL;
    if (!AeAssert::IsIgnored()) {
        if (AeAssert::Warning("Unsupported."))
            __debugbreak();
    }
}

// ============================================================================
// SV_SpawnServer â€” ea: 0x525000 (server boot sequence)
// ============================================================================
void SV_SpawnServer(const char* server, int savegame) {
    char* v2 = va("maps/%s.bsp", server);
    SV_SetExpectedHunkUsage(v2);
    if (fs_debug->integer == 2)
        Cvar_Set("fs_debug", "0");
    cvar_t* v3 = Cvar_Get("cl_xmodelcheck", "0", 33);
    XModelEnforceExist(v3->integer);
    sv_save_filename[0] = 0;
    sv_map_restart = 0;
    if (g_gameskill == nullptr)
        g_gameskill = Cvar_Get("g_gameskill", "1", 100);
    CL_MapLoading();
    CL_ShutdownAll();
    if (gvm != nullptr) {
        VM_Call(gvm, 1, 0);
        VM_Free(gvm);
        gvm = nullptr;
    }
    Com_Printf("------ Server Initialization ------\n");
    Com_Printf("Server: %s\n", server);
    SV_ClearServer();
    FS_Shutdown(1);
    unsigned int v4 = (unsigned int)Sys_Milliseconds();
    srand(v4);
    FS_Restart(0);
    FS_ClearMemory();
    com_inServerFrame = 0;
    sv.state = SS_LOADING;
    GamePause_SetAllPaused(true);
    svs.nextSnapshotEntities = 0;
    Cvar_Set("nextmap", "map_restart");
    Cvar_Set("mapname", server);
    sv.serverId = com_frameTime;
    sv.restartedServerId = com_frameTime;
    Cvar_Set("sv_serverid", va("%i", com_frameTime));
    CL_StartLoading();
    if (Cvar_VariableValue("sv_running") == 0.0f) {
        Com_Restart();
        SV_Startup();
    } else {
        Com_InitDObj();
    }

    extern void gEntFreeList_Init(int n);
    extern void gDSkelFreeList_Init(int n);
    extern void gDSkelMaxFreeList_Init(int n);
    extern void gDSkel4FreeList_Init(int n);
    extern void gDObjFreeList_Init(int n);
    extern void gRefEntFreeList_Init(int n);
    gEntFreeList_Init(350);
    gDSkelFreeList_Init(96);
    gDSkelMaxFreeList_Init(26);
    gDSkel4FreeList_Init(20);
    gDObjFreeList_Init(300);
    gRefEntFreeList_Init(300);
    extern void EntityManager_CreateWorld(void);
    EntityManager_CreateWorld();
    Netchan_Init();
    netadr_t from;
    from.type = NA_BOT;
    if (unk_F6A290 == 2) {
        netadr_t v20;
        v20.type = (netadrtype_t)2;
        memset(v20.ipx, 0, 12);
        currCl = NS_CLIENT;
        SV_DirectConnect(v20);
        v20.type = (netadrtype_t)2;
        memset(v20.ipx, 0, 12);
        CL_ConnectResponse(v20);
    }
    currCl = NS_CLIENT;
    for (int i = 0; i < 1024; ++i)
        sv.configstrings[i] = defaultFileName;
    SCR_UpdateScreen();
    if (g_bspTree != 0) {
        EffectEventSys_StopAll((void*)0);
        PakManager_UnloadAll(PakManager_sInst());
        g_bspTree = 0;
        g_ScrFiles = 0;
        FX_TermFX();
        PathNodeMgr_CleanUpManager((void*)0);
        CGBankManager_UnloadAll((void*)0);
        AnimBankManager_UnloadAll((void*)0);
        PakManager_SetUserDistance(PakManager_sInst(), sLoadingScreenInfo, 0.0f);
        PakManager_SyncLoadPak(PakManager_sInst(), sLoadingScreenInfo);
        InGameMenuSystem_ActivateMenu((void*)0, 4);
    }
    MPUIInterface_BlockUntilNetReady();
    char pakname[256];
    sprintf(pakname, "%s\\%s\\%s.cod", "mp", server, server);
    PakManager* pm = PakManager_sInst();
    extern void LoadingMenuCallback(float progress);
    extern void PakManager_SetProgressCallback(PakManager* self, void (*cb)(float));
    PakManager_SetProgressCallback(pm, LoadingMenuCallback);
    const PakInfoNode* FLI = nullptr;
    if (nflFileExists(gNflMediaId, pakname) != 0)
        goto LABEL_28;
    sprintf(pakname, "%s\\%s.cod", "mp", server);
    if (nflFileExists(gNflMediaId, pakname) != 0)
        FLI = PakManager_SyncLoadFLI(PakManager_sInst(), kPakTypeLevel, pakname);
    sprintf(pakname, "%s_test\\%s.cod", "mp", server);
    if (FLI == nullptr && nflFileExists(gNflMediaId, pakname) != 0)
        FLI = PakManager_SyncLoadFLI(PakManager_sInst(), kPakTypeLevel, pakname);
    sprintf(pakname, "%s_test\\%s\\%s.cod", "mp", server, server);
    if (FLI == nullptr) {
        if (nflFileExists(gNflMediaId, pakname) == 0) {
LABEL_29:
            gReturnToMenu = true;
            Cvar_Set("g_reloading", "1");
            Cmd_ExecuteServerString("reboot");
            goto LABEL_30;
        }
LABEL_28:
        FLI = PakManager_SyncLoadFLI(PakManager_sInst(), kPakTypeLevel, pakname);
        if (FLI != nullptr)
            goto LABEL_30;
        goto LABEL_29;
    }
LABEL_30:
    int v9 = nglSetFrameLock(2);
    PakManager_SyncLoadPak(PakManager_sInst(), FLI);
    CM_LoadMap(server, 0, &sv.checksum);
    PakManager_FillBanks(PakManager_sInst());
    nglSetFrameLock(v9);
    AudioBankMgr_FinishLoading((void*)0);
    if (FLI != nullptr) {
        PakManager_PushContext(PakManager_sInst(), FLI->pakId);
        PakManager_PopContext(PakManager_sInst());
    }
    SCR_UpdateScreen();
    gvm = VM_Create("game", (int(__cdecl*)(int*))SV_GameSystemCalls);
    if (gvm == nullptr)
        Com_Error(1, "VM_Create on game failed");
    sv.checksum = 0;
    VM_Call(gvm, 0, Com_Milliseconds(), 0, savegame, sv.checksum);
    for (int v11 = 0; v11 < 0x13700; v11 += 4976)
        svs.clients[v11 / 0x1370].mEntityHandle.mHandle.mVal = 0;
    PakManager_SetProgressCallback(PakManager_sInst(), nullptr);
    FEManager_UpdateLoadingMenu((void*)0, 0.85000002f);
    SCR_UpdateScreen();
    com_time = 0;
    for (int i = 5; i != 0; --i) {
        CL_FlushDebugData(1);
        if ((cvar_modifiedFlags & 4) != 0) {
            SV_SetConfigstring(0, Cvar_InfoString(4));
            cvar_modifiedFlags &= ~4;
        }
        if ((cvar_modifiedFlags & 8) != 0) {
            SV_SetConfigstring(1, Cvar_InfoString_Big(8));
            cvar_modifiedFlags &= ~8;
        }
        VM_Call(gvm, 11, 50);
        SV_RunFrame(50);
    }
    FEManager_UpdateLoadingMenu((void*)0, 0.94999999f);
    SCR_UpdateScreen();
    Cvar_Set("sv_paks", defaultFileName);
    Cvar_Set("sv_pakNames", defaultFileName);
    char systemInfo[1024];
    Q_strncpyz(systemInfo, Cvar_InfoString_Big(8), 1024);
    cvar_modifiedFlags &= ~8;
    SV_SetConfigstring(1, systemInfo);
    SV_SetConfigstring(0, Cvar_InfoString(4));
    cvar_modifiedFlags &= ~4;
    Cvar_Set("cg_norender", "1");
    Com_Printf("-----------------------------------\n");
    FEManager_UpdateLoadingMenu((void*)0, 1.0f);
    SCR_UpdateScreen();
    PathNodeMgr_InitPaths((void*)0);
    if (unk_F6A290 == 2) {
        currCl = NS_CLIENT;
        UpdateCVars();
        client_s* v17 = &svs.clients[currCl];
        SV_ClientEnterWorld(v17, 0, savegame);
        v17->gamestateMessageNum = *(int*)&v17->netchan[36];
        CL_ParseGamestate(sv.configstrings);
    }
    currCl = NS_CLIENT;
    client_s* clients = svs.clients;
    CL_InitCGame();
    extern void StubData_ApplyStubOptions(void* self);
    for (int i = 0; i < 4; ++i) {
        if (gSaveGameData[i].mStubData.mControllerPort == -1)
            gSaveGameData[i].mStubData.mControllerPort = 0;
        StubData_ApplyStubOptions(&gSaveGameData[i].mStubData);
    }
    FEManager_UpdateLoadingMenu((void*)0, 0.94999999f);
    SCR_UpdateScreen();
    SV_CheckLoadLevel(savegame);
    SV_SendClientMessages();
    Com_EventLoop();
    clients->gamestateMessageNum = *(int*)&clients->netchan[36];
    CL_ParseGamestate(sv.configstrings);
    CL_FirstSnapshot();
    GamePause_SetAllPaused(true);
    sv.state = SS_GAME;
}

// ============================================================================
// SV_Netchan_Transmit — ea: 0x51F940
// ============================================================================
void SV_Netchan_Transmit(client_s* client, msg_t* msg) {
    MSG_WriteByte(msg, 8);
    Netchan_Transmit((netchan_t*)client->netchan, msg->cursize, msg->data);
}

// ============================================================================
// SV_Netchan_Process — ea: 0x51F970
// ============================================================================
int SV_Netchan_Process(client_s* client, msg_t* msg) {
    return Netchan_Process((netchan_t*)client->netchan, msg);
}

// ============================================================================
// SV_UpdateServerCommandsToClient — ea: 0x51FA10
// ============================================================================
void SV_UpdateServerCommandsToClient(client_s* client, msg_t* msg) {
    int v2 = client->reliableAcknowledge + 1;
    if (v2 > client->reliableSequence) {
        client->reliableSent = client->reliableSequence;
    } else {
        int reliableSequence;
        do {
            MSG_WriteByte(msg, 5);
            MSG_WriteLong(msg, v2);
            int v3 = v2 & 0x3F;
            const char* v4;
            if (client->reliableCommands.bufSize != 0) {
                if (client->reliableCommands.commandLengths[v3] != 0)
                    v4 = client->reliableCommands.commands[v3];
                else
                    v4 = nullStr;
            } else {
                v4 = nullStr;
            }
            MSG_WriteString(msg, v4);
            reliableSequence = client->reliableSequence;
            ++v2;
        } while (v2 <= reliableSequence);
        client->reliableSent = reliableSequence;
    }
}

// ============================================================================
// SV_SendMessageToClient — ea: 0x51FAA0
// ============================================================================
void SV_SendMessageToClient(msg_t* msg, client_s* client) {
    sv_snapshotFrameNumber = com_frameNumber;
    MSG_WriteByte(msg, 8);
    Netchan_Transmit((netchan_t*)client->netchan, msg->cursize, msg->data);
}

// ============================================================================
// SV_GetFollowPlayerState — ea: 0x51FAE0
// ============================================================================
int SV_GetFollowPlayerState(int clientNum, PlayerState* ps) {
    return VM_Call(gvm, 7, clientNum, ps);
}

// ============================================================================
// SV_GetCurrentClientInfo — ea: 0x51FB00
// ============================================================================
int SV_GetCurrentClientInfo(int clientNum, PlayerState* ps) {
    return svs.clients[clientNum].state == 1 && VM_Call(gvm, 7, clientNum, ps) != 0;
}

// ============================================================================
// SV_Vid_Restart — ea: 0x51F800
// ============================================================================
void SV_Vid_Restart() {
    Cbuf_ExecuteText(2, "savegame internal\\vid_restart\n");
}

// ============================================================================
// SV_Snd_Restart — ea: 0x51F810
// ============================================================================
void SV_Snd_Restart() {
    Cbuf_ExecuteText(2, "savegame internal\\snd_restart\n");
}

// ============================================================================
// SV_PacketEvent — ea: 0x520790
// ============================================================================
void SV_PacketEvent(netadr_t from, msg_t* msg) {
    if (++com_skelTimeStamp == 0)
        com_skelTimeStamp = 1;
    if (bSV_AllowedAllocSkel != 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
        AeAssert::gCurrentLine = 244;
        AeAssert::gCurrentExpr = "!bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 1;
    MSG_BeginReading(msg);
    MSG_ReadLong(msg);
    int Short = MSG_ReadShort(msg);
    int v3 = 0;
    client_s* clients = svs.clients;
    while (clients->state == 0 || Short != v3) {
        ++v3;
        ++clients;
        if (v3 >= 16) {
            NET_OutOfBandPrint(NS_SERVER, from, "disconnect");
            goto done;
        }
    }
    client_s* v5 = &svs.clients[Short];
    if (Netchan_Process((netchan_t*)v5->netchan, msg) != 0)
        SV_ExecuteClientMessage(v5, msg);
done:
    if (bSV_AllowedAllocSkel == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_main.cpp";
        AeAssert::gCurrentLine = 276;
        AeAssert::gCurrentExpr = "bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 0;
}

// ============================================================================
// SV_Frame — ea: 0x5259C0
// ============================================================================
void SV_Frame(int msec) {
    AeThreadManager::sInst.Execute(msec * 0.001f);
    if (sv_killserver->integer) {
        Com_Shutdown();
        Cvar_Set("sv_killserver", "0");
    } else if (com_sv_running->integer) {
        if (!GamePause::IsGamePaused(currCl) && !CL_IsCGameRendering())
            GamePause::SetGamePaused(currCl, 1);
        svs.clients->serverId = sv.serverId;
        if (SV_CheckLoadGame()) {
            CL_SetFrametime(0, 0);
        } else {
            cdl_proftimer_vmcalls.start();
            if (!com_inServerFrame) {
                SV_PreFrame(msec);
                com_inServerFrame = 1;
            }
            int v1 = VM_Call(gvm, 20);
            com_time = v1;
            cdl_proftimer_vmcalls.stop();
            if (v1 <= 1879048192 && svs.nextSnapshotEntities < 2147483646 - svs.numSnapshotEntities) {
                SV_RunFrame(msec);
                cdl_proftimer_cl_msgs.start();
                SV_SendClientMessages();
                CL_SetFrametime(msec, 0);
                cdl_proftimer_cl_msgs.stop();
            } else {
                Com_Shutdown();
                Cbuf_AddText("vstr nextmap\n");
            }
        }
    } else {
        CL_SetFrametime(msec, 0);
    }
}
