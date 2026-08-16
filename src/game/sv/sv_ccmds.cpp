// ============================================================================
// sv_ccmds.cpp — server console commands (sv_ccmds.cpp from sv.o)
// Public funcs: SV_Difficulty*, SV_ClearLoadGame, SplitTime, UpdatePlayTime,
//   SV_GetRestartSaveGame, SV_LoadGameContinue_f, SV_Frontend_f,
//   SV_QuickStart_f, SV_InvertAim_f, SV_ExitGame_f, SV_RemoveOperatorCommands,
//   SV_MapRestart, SV_LoadGame_f, SV_LoadGameRestart_f, SV_CheckLoadGame,
//   SV_Map, SV_AddOperatorCommands, SV_Init
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// Cross-object externs (sv_ccmds-specific)
// ============================================================================
extern int  Q_strncmp(const char* s1, const char* s2, int n);
extern int  Q_stricmp(const char* s1, const char* s2);
extern int  Q_stricmpn(const char* s1, const char* s2, int n);
extern void Q_strcat(char* dest, int size, const char* src);
extern char* Q_strlwr(char* s1);
extern char* Q_strrchr(const char* string, int c);
extern void Cbuf_ExecuteText(int exec_when, const char* text);
extern int  Cvar_VariableIntegerValue(const char* var_name);
extern void Com_Memset(void* dest, int val, unsigned int count);
extern void Con_Close(void);
extern int CL_ClearState(void);
extern int CL_Restart(void);
extern int CL_ShutdownDebugData(void);
extern void R_ShutdownDebug(void);
extern void R_InitDebug(void);
extern cvar_t* com_sv_running;
bool gQuickStart;
extern bool gReturnToMenu;
bool gIsWorkspaceMap;
bool gDoNotPlayCampaignMovies;
extern int  g_networkOwner;
extern VehicleNodeAllocator g_vehicleNodeManager;  // ?g_vehicleNodeManager@@3VVehicleNodeAllocator@@A (g.o)
extern int  SV_RestartGameProgs(int savegame);
extern int  SV_CheckLoadGame(void);
extern void SV_Map_f(void);
extern void SV_SpawnServer(const char* server, int savegame);

// statics
static cvar_t* SV_MapRestart(int savegame);   // ea: 0x524670
static cvar_t* SV_MapRestart_f(void);          // ea: 0x524B10
static void SV_MapRestart_Command(void);       // ea: 0x524B50
static void SV_MapRestart_Internal(void);      // ea: 0x51E410
static void SV_Reboot_Command(void);           // ea: 0x51E580
static void SV_LoadMpExe_Command(void);        // ea: 0x51E680
static void SV_MapRestart_CheckPoint_f(void);  // ea: 0x51E780
static void SV_StringUsage_f(void);            // ea: 0x51E860
static void SV_NextMap_f(void);                // ea: 0x51E8B0
static void SV_MapRestartComp_f(void);         // ea: 0x525DD0
static void SV_Devmap_f(void);                 // ea: 0x525DB0

// ============================================================================
// SV_DifficultyEasy — ea: 0x51E220
// ============================================================================
void SV_DifficultyEasy() {
    Cvar_Set("g_gameskill", va("%d", 0));
    Cvar_Set("g_player_maxhealth", "1200");
    Cvar_Set("player_deathInvulnerableTime", "4000");
    if (sv_reloading->integer != 0)
        Cvar_Set("g_reloading", va("%d", 2));
}

// ============================================================================
// SV_DifficultyMedium — ea: 0x51E280
// ============================================================================
void SV_DifficultyMedium() {
    Cvar_Set("g_gameskill", va("%d", 1));
    Cvar_Set("g_player_maxhealth", "600");
    Cvar_Set("player_deathInvulnerableTime", "1000");
    if (sv_reloading->integer != 0)
        Cvar_Set("g_reloading", va("%d", 2));
}

// ============================================================================
// SV_DifficultyHard — ea: 0x51E2E0
// ============================================================================
void SV_DifficultyHard() {
    Cvar_Set("g_gameskill", va("%d", 2));
    Cvar_Set("g_player_maxhealth", "300");
    Cvar_Set("player_deathInvulnerableTime", "100");
    if (sv_reloading->integer != 0)
        Cvar_Set("g_reloading", va("%d", 2));
}

// ============================================================================
// SV_DifficultyGimp — ea: 0x51E340
// ============================================================================
void SV_DifficultyGimp() {
    Cvar_Set("g_gameskill", va("%d", 0));
    Cvar_Set("g_player_maxhealth", "1000");
    Cvar_Set("player_deathInvulnerableTime", "8000");
    if (sv_reloading->integer != 0)
        Cvar_Set("g_reloading", va("%d", 2));
}

// ============================================================================
// SV_DifficultyFu — ea: 0x51E3A0
// ============================================================================
void SV_DifficultyFu() {
    Cvar_Set("g_gameskill", va("%d", 3));
    Cvar_Set("g_player_maxhealth", "200");
    Cvar_Set("player_deathInvulnerableTime", "100");
    if (sv_reloading->integer != 0)
        Cvar_Set("g_reloading", va("%d", 2));
}

// ============================================================================
// SV_ClearLoadGame — ea: 0x51E400
// ============================================================================
void SV_ClearLoadGame() {
    sv_save_filename[0] = 0;
    sv_map_restart = 0;
}

// ============================================================================
// SplitTime — ea: 0x51E480
// ============================================================================
void SplitTime(int time, int& sec, int& min, int& hour, int& day) {
    int v5 = time;
    if (time > 359999)
        v5 = 359999;
    int v6 = v5 % 86400;
    day = (v5 - v5 % 86400) / 86400;
    int v7 = v5 % 86400 % 3600;
    hour = (v6 - v7) / 3600;
    min = (v7 - v7 % 60) / 60;
    sec = v7 % 60;
}

// ============================================================================
// UpdatePlayTime — ea: 0x51E510
// ============================================================================
void UpdatePlayTime() {
    StubData& d = gSaveGameData->mStubData;
    float v0 = (float)(d.mSec + 60 * (d.mMin + 60 * (d.mHour + 24 * d.mDay)))
             + (ServerTime::sInst.mElapsedTime - g_femanager.saveTime);
    if (v0 > 86400.0f)
        v0 = 86400.0f;
    SplitTime((int)v0, d.mSec, d.mMin, d.mHour, d.mDay);
}

// ============================================================================
// SV_MapRestart_Internal — ea: 0x51E410 (static)
// ============================================================================
static void SV_MapRestart_Internal() {
    gGodModeEnabled = false;
    gNoClipEnabled = false;
    Cvar_Set("checkpoint", "0");
    Cvar_Set("cl_restartdeath", "1");
    Cvar_Set("letterbox_enabled", "0");
    CheckpointMgr::sInst->mUsingCheckpoints = false;
    CheckpointMgr::sInst->ClearSavedCheckpointData();
    AeThreadManager::sInst.KillAllThreads();
    gPumpThreads = true;
}

// ============================================================================
// SV_Reboot_Command — ea: 0x51E580 (static; Xbox reboot)
// ============================================================================
static void SV_Reboot_Command() {
    // Xbox-specific reboot via XLaunchNewImageA; ported as structural stub.
}

// ============================================================================
// SV_LoadMpExe_Command — ea: 0x51E680 (static; Xbox reboot to MP exe)
// ============================================================================
static void SV_LoadMpExe_Command() {
    // Xbox-specific reboot via XLaunchNewImageA; ported as structural stub.
}

// ============================================================================
// SV_MapRestart_CheckPoint_f — ea: 0x51E780 (static)
// ============================================================================
static void SV_MapRestart_CheckPoint_f() {
}

// ============================================================================
// SV_GetRestartSaveGame — ea: 0x51E790
// ============================================================================
void SV_GetRestartSaveGame(char* save_filename) {
    char mapname[128];
    Cvar_VariableStringBuffer("mapname", mapname, 128);
    int v1 = (int)strlen(mapname);
    for (int i = 0; i < v1; ++i) {
        char v3 = mapname[i];
        if (v3 == '/')
            mapname[i] = '-';
        else if (Q_isforfilename((int)v3) == 0 && mapname[i] != '\\') {
            Com_Printf("SV_LoadGameRestart_f: '%s'.  Invalid character (%c) in filename. Must use alphanumeric characters only.\n",
                       mapname, mapname[i]);
            *save_filename = 0;
            return;
        }
    }
    Com_sprintf(save_filename, 128, "save/autosave/%s.svg", mapname);
    if (FS_ReadFile(save_filename, NULL) < 0)
        *save_filename = 0;
}

// ============================================================================
// SV_LoadGameContinue_f — ea: 0x51E850
// ============================================================================
void SV_LoadGameContinue_f() {
}

// ============================================================================
// SV_StringUsage_f — ea: 0x51E860 (static)
// ============================================================================
static void SV_StringUsage_f() {
}

// ============================================================================
// SV_Frontend_f — ea: 0x51E870
// ============================================================================
void SV_Frontend_f() {
    char* result = Cmd_Argv(1);
    if (*result == '1')
        g_femanager.debug_mode = false;
}

// ============================================================================
// SV_QuickStart_f — ea: 0x51E890
// ============================================================================
void SV_QuickStart_f() {
    gQuickStart = true;
}

// ============================================================================
// SV_InvertAim_f — ea: 0x51E8A0
// ============================================================================
void SV_InvertAim_f() {
    gSaveGameData->mStubData.mInvertAim = true;
}

// ============================================================================
// SV_NextMap_f — ea: 0x51E8B0 (static)
// ============================================================================
static void SV_NextMap_f() {
    gExitGame = false;
    AeThreadManager::sInst.KillAllThreads();
    gPumpThreadsForMapChange = true;
    const char* v0 = Cmd_Argv(1);
    strncpy(gNextMapArgv, v0, 0x20u);
}

// ============================================================================
// SV_ExitGame_f — ea: 0x51E8E0
// ============================================================================
void SV_ExitGame_f() {
    gExitGame = true;
    AeThreadManager::sInst.KillAllThreads();
    gPumpThreadsForMapChange = true;
}

// ============================================================================
// SV_RemoveOperatorCommands — ea: 0x51E900
// ============================================================================
void SV_RemoveOperatorCommands() {
}

// ============================================================================
// SV_MapRestart_f — ea: 0x524B10 (static; identical to public SV_MapRestart)
// ============================================================================
static cvar_t* SV_MapRestart_f() {
    MultiplayerMgr::sInst->ExitLevel();
    CL_FlushDebugData(1);
    CL_ShutdownDebugData();
    R_ShutdownDebug();
    R_InitDebug();
    cvar_t* result = com_sv_running;
    sv_map_restart = 1;
    if (com_sv_running->integer == 0)
        return (cvar_t*)SV_CheckLoadGame();
    return result;
}

// ============================================================================
// SV_MapRestart_Command — ea: 0x524B50 (static)
// ============================================================================
static void SV_MapRestart_Command() {
    AeThreadManager::sInst.KillAllThreads();
    gPumpThreads = true;
    MultiplayerMgr::sInst->ExitLevel();
    CL_FlushDebugData(1);
    CL_ShutdownDebugData();
    R_ShutdownDebug();
    R_InitDebug();
    sv_map_restart = 1;
    if (com_sv_running->integer == 0)
        SV_CheckLoadGame();
}

// ============================================================================
// SV_MapRestart — ea: 0x524BB0
// ============================================================================
void SV_MapRestart() {
    MultiplayerMgr::sInst->ExitLevel();
    CL_FlushDebugData(1);
    CL_ShutdownDebugData();
    R_ShutdownDebug();
    R_InitDebug();
    sv_map_restart = 1;
    if (com_sv_running->integer == 0)
        SV_CheckLoadGame();
}

// ============================================================================
// SV_MapRestartComp_f — ea: 0x525DD0 (static; thunk to SV_MapRestart_Command)
// ============================================================================
static void SV_MapRestartComp_f() {
    SV_MapRestart_Command();
}

// ============================================================================
// SV_LoadGame_f — ea: 0x524BF0
// ============================================================================
void SV_LoadGame_f() {
    int integer = sv_reloading->integer;
    if (integer != 0) {
        if (integer != 32)
            return;
        Cvar_Set("g_reloading", "0");
    }
    char* v1 = Cmd_Argv(1);
    Q_strncpyz(sv_save_filename, v1, 128);
    if (sv_save_filename[0] != 0) {
        if (Q_strncmp(sv_save_filename, "save/", 5) != 0 && Q_strncmp(sv_save_filename, "save\\", 5) != 0) {
            const char* v2 = va("save/%s", sv_save_filename);
            Q_strncpyz(sv_save_filename, v2, 128);
        }
        if (strstr(sv_save_filename, ".") == NULL || Q_strncmp(strstr(sv_save_filename, ".") + 1, "svg", 3) != 0)
            Q_strcat(sv_save_filename, 128, ".svg");
        while (strstr(sv_save_filename, "\\") != NULL)
            *strstr(sv_save_filename, "\\") = '/';
        if (com_sv_running->integer == 0)
            SV_CheckLoadGame();
    } else {
        Com_Printf("You must specify a savegame to load\n");
    }
}

// ============================================================================
// SV_LoadGameRestart_f — ea: 0x524D50
// ============================================================================
void SV_LoadGameRestart_f() {
    Cvar_Set("g_internalSaveGame", defaultFileName);
    SV_GetRestartSaveGame(sv_save_filename);
    if (sv_save_filename[0] == 0)
        Com_Error((errorParm_t)2, "EXE_ERR_NO_LAST_SAVE");
    if (com_sv_running->integer == 0)
        SV_CheckLoadGame();
}

// ============================================================================
// SV_CheckLoadGame — ea: 0x524920
// ============================================================================
int SV_CheckLoadGame() {
    char mapname[128];
    char filename[13];
    unsigned __int8* buffer;

    if (svs.clients != NULL && svs.clients->serverId != sv.serverId) {
        sv_save_filename[0] = 0;
        sv_map_restart = 0;
        return 0;
    }
    if (sv_map_restart != 0) {
        SV_MapRestart(0);
        return 1;
    }
    if (sv_save_filename[0] == 0)
        return 0;
    int v1 = 0;
    char v2;
    do {
        v2 = sv_save_filename[v1];
        filename[v1++] = v2;
    } while (v2 != 0);
    sv_save_filename[0] = 0;
    if (FS_ReadFile(filename, NULL) < 0) {
        Com_Printf("Can't find savegame %s\n", filename);
        return 0;
    }
    FS_ReadFile(filename, (void**)&buffer);
    unsigned __int8* v3 = buffer;
    memcpy(mapname, buffer + 5, buffer[4]);
    mapname[v3[4]] = 0;
    if ((Q_stricmpn(filename, "save/internal", 13) == 0 || Q_stricmpn(filename, "save\\internal", 13) == 0)
        && (v3[4] == '/' || v3[4] == '\\')) {
        Cvar_Set("g_internalSaveGame", filename);
    } else {
        Cvar_Set("g_internalSaveGame", defaultFileName);
        Cvar_Set("g_lastSaveGame", filename);
    }
    SoundDevice::sInst->StopAllSounds();
    mem_heap_free(buffer);
    if (com_sv_running->integer == 0
        || Q_stricmp(mapname, sv_mapname->string) != 0
        || SV_MapRestart(1) == 0) {
        const char* v4;
        if (Cvar_VariableIntegerValue("sv_cheats") != 0)
            v4 = va("spdevmap %s\n", filename);
        else
            v4 = va("spmap %s\n", filename);
        Cbuf_ExecuteText(2, v4);
    }
    return 1;
}

// ============================================================================
// SV_MapRestart (int) — ea: 0x524670 (static)
// ============================================================================
static cvar_t* SV_MapRestart(int savegame) {
    g_vehicleNodeManager.FreeAll();
    unsigned int v1 = 0;
    sv_save_filename[0] = 0;
    sv_map_restart = 0;
    if (com_frameTime == sv.serverId)
        return (cvar_t*)1;
    if (com_sv_running->integer == 0) {
        Com_Printf("Game is not running.\n");
        return (cvar_t*)1;
    }
    g_femanager.IGO->SetTutorialText(-1, currCl);
    Con_Close();
    com_inServerFrame = 0;
    sv.state = SS_LOADING;
    GamePause::SetGamePaused(currCl, true);
    if (SV_RestartGameProgs(1) == 0) {
        CL_ClearState();
        sv.restartedServerId = sv.serverId;
        sv.serverId = com_frameTime;
        const char* v3 = va("%i", com_frameTime);
        Cvar_Set("sv_serverid", v3);
        if (svs.clients == NULL) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_ccmds.cpp";
            AeAssert::gCurrentLine = 625;
            AeAssert::gCurrentExpr = "svs.clients";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid client index for dms"))
                __debugbreak();
        }
        do {
            client_s* v4 = &svs.clients[v1 / 0x1370];
            reliableCommands_t* p_reliableCommands = &v4->reliableCommands;
            if (p_reliableCommands->bufSize != 0) {
                _Z_FreeInternal(v4->reliableCommands.buf);
                _Z_FreeInternal(v4->reliableCommands.commandLengths);
                _Z_FreeInternal(v4->reliableCommands.commands);
                Com_Memset((unsigned int*)p_reliableCommands, 0, 4u);
            }
            v1 += 4976;
        } while (v1 < 0x13700);
        Netchan_Init();
        if (dword_F6A290[0] == 2) {
            netadr_t v7;
            *((int*)&v7) = 2;
            memset(v7.ipx, 0, 12);
            currCl = NS_CLIENT;
            SV_DirectConnect(v7);
            *((int*)&v7) = 2;
            memset(v7.ipx, 0, 12);
            CL_ConnectResponse(v7);
        }
        currCl = NS_CLIENT;
        com_time = 0;
        if (savegame == 0)
            Cvar_Set("cg_norender", "1");
        PathNodeMgr::sInst->InitPaths();
        CL_Restart();
        if (dword_F6A290[0] == 2) {
            client_s* clients = svs.clients;
            SV_ClientEnterWorld(svs.clients, 1, savegame);
            clients->gamestateMessageNum = *(int*)&clients->netchan[36];
            CL_ParseGamestate(sv.configstrings);
        }
        SV_CheckLoadLevel(savegame);
        SV_SendClientMessages();
        CL_FirstSnapshot();
        GamePause::SetAllPaused(false);
        sv.state = SS_GAME;
        return (cvar_t*)1;
    }
    if (savegame == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_ccmds.cpp";
        AeAssert::gCurrentLine = 614;
        AeAssert::gCurrentExpr = "savegame";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return (cvar_t*)0;
}

// ============================================================================
// SV_Map_f — ea: 0x525B30 (static)
// ============================================================================
static void SV_Map_f() {
    j_nullsub_86(GAME_PHASE_LOADING);
    g_femanager.inGame = true;
    bool mLinkCheckEnabled = MultiplayerMgr::sInst->mLinkCheckEnabled;
    MultiplayerMgr::sInst->mLinkCheckEnabled = false;
    bool linkCheckEnabled = mLinkCheckEnabled;
    g_femanager.IGO_active = false;
    MultiplayerMgr::sInst->ExitLevel();
    g_networkOwner = kMainThread;
    gMPLoadingUnthreaded = true;
    PakManager::sInst->FillBanks();
    const char* v1 = Cmd_Argv(1);
    if (v1 != NULL) {
        int integer = sv_reloading->integer;
        if (integer == 0 || integer == 2) {
            char lower_map[131];
            char mapname[128];
            strcpy(lower_map, v1);
            Q_strlwr(lower_map);
            const char* v3 = Q_strrchr(lower_map, '_');
            if (v3 != NULL && Q_stricmp(v3, "_workspace") == 0)
                gIsWorkspaceMap = true;
            SoundDevice::sInst->StopAllSounds();
            if (!gDoNotPlayCampaignMovies) {
                if (strnicmp(v1, "oran", 0xCu) == 0) {
                    FEManager_PlayFadeInOranScreen();
                    movie_manager::load_and_play_movie("Op_Torch", defaultFileName);
                } else if (strnicmp(v1, "gela", 0xCu) == 0) {
                    movie_manager::load_and_play_movie("Op_Husky", defaultFileName);
                } else if (strnicmp(v1, "omaha_beach", 0xCu) == 0) {
                    movie_manager::load_and_play_movie("Op_Ovrld", defaultFileName);
                }
            }
            Cvar_Get("g_gameskill", "1", 100);
            const char* v4 = Cmd_Argv(0);
            if (Q_stricmpn(v4, "sp", 2) == 0)
                v4 += 2;
            Q_stricmp(v4, "devmap");
            Q_strncpyz(mapname, lower_map, 128);
            if (g_femanager.fems != NULL)
                g_femanager.fems->SetActiveMenu(-1);
            Broc::string::Block* mBlock = CheckpointMgr::sInst->mCurrentMapName.mBlock;
            const char* v6;
            if (mBlock != NULL)
                v6 = (const char*)&mBlock[1];
            else
                v6 = defaultFileName;
            if (CheckpointMgr::sInst->mCheckpointSaveExists && v6 != NULL && stricmp(mapname, v6) == 0)
                CheckpointMgr::sInst->mUsingCheckpoints = true;
            SV_SpawnServer(mapname, 0);
            Cvar_Set("sv_cheats", "1");
            gMPLoadingUnthreaded = false;
            g_networkOwner = kMainThread;
            for (int i = 1; i < BG_GetNumWeapons(); ++i)
                CG_RegisterWeapon(i);
            g_femanager.IGO_active = true;
            MultiplayerMgr::sInst->mLinkCheckEnabled = linkCheckEnabled;
            j_nullsub_86(GAME_PHASE_INGAME);
        }
    }
}

// ============================================================================
// SV_Map — ea: 0x525DC0 (thunk to SV_Map_f)
// ============================================================================
void SV_Map() {
    SV_Map_f();
}

// ============================================================================
// SV_Devmap_f — ea: 0x525DB0 (static)
// ============================================================================
static void SV_Devmap_f() {
    MultiplayerMgr::sInst->StartDevServer();
    SV_Map_f();
}

// ============================================================================
// SV_AddOperatorCommands — ea: 0x525DE0
// ============================================================================
void SV_AddOperatorCommands() {
    static int initialized_0 = 0;
    if (initialized_0 == 0) {
        initialized_0 = 1;
        Cmd_AddServerCommand("map_restart_internal", SV_MapRestart_Internal);
        Cmd_AddServerCommand("map_restart", SV_MapRestart_Command);
        Cmd_AddServerCommand("map_restart_check", SV_MapRestart_CheckPoint_f);
        Cmd_AddServerCommand("map_restart_comp", SV_MapRestartComp_f);
        Cmd_AddServerCommand("quick_start", SV_QuickStart_f);
        Cmd_AddServerCommand("invert", SV_InvertAim_f);
        Cmd_AddServerCommand("reboot", SV_Reboot_Command);
        Cmd_AddServerCommand("load_mp_exe", SV_LoadMpExe_Command);
        Cmd_AddServerCommand("spmap", SV_Map_f);
        Cmd_AddServerCommand("map", SV_Map_f);
        Cmd_AddServerCommand("devmap", SV_Devmap_f);
        Cmd_AddServerCommand("loadgame", SV_LoadGame_f);
        Cmd_AddServerCommand("loadgame_restart", SV_LoadGameRestart_f);
        Cmd_AddServerCommand("loadgame_continue", SV_LoadGameContinue_f);
        Cmd_AddServerCommand("difficultyEasy", SV_DifficultyEasy);
        Cmd_AddServerCommand("difficultyMedium", SV_DifficultyMedium);
        Cmd_AddServerCommand("difficultyHard", SV_DifficultyHard);
        Cmd_AddServerCommand("stringUsage", SV_StringUsage_f);
        Cmd_AddServerCommand("frontend", SV_Frontend_f);
        Cmd_AddServerCommand("exitgame", SV_ExitGame_f);
        Cmd_AddServerCommand("nextmap", SV_NextMap_f);
        Cmd_AddServerCommand("difficultyGimp", SV_DifficultyGimp);
        Cmd_AddServerCommand("difficultyFu", SV_DifficultyFu);
    }
}

// ============================================================================
// SV_Init — ea: 0x525F60
// ============================================================================
void SV_Init() {
    SV_AddOperatorCommands();
    sv_gameskill = Cvar_Get("g_gameskill", "1", 36);
    sv_mapname = Cvar_Get("mapname", "nomap", 68);
    sv_showCommands = Cvar_Get("sv_showCommands", "0", 0);
    Cvar_Get("sv_cheats", "0", 72);
    sv_serverid = Cvar_Get("sv_serverid", "0", 72);
    Cvar_Get("sv_paks", defaultFileName, 72);
    Cvar_Get("sv_pakNames", defaultFileName, 72);
    Cvar_Get("sv_referencedPaks", defaultFileName, 72);
    Cvar_Get("sv_referencedPakNames", defaultFileName, 72);
    Cvar_Get("nextmap", defaultFileName, 256);
    sv_killserver = Cvar_Get("sv_killserver", "0", 0);
    sv_reloading = Cvar_Get("g_reloading", "0", 64);
    sv_framerate_smoothing = Cvar_Get("sv_framerate_smoothing", "1", 0);
}
