// ============================================================================
// COD3 Core Globals - core.o data symbols with exact VAs
// Extracted from the release linker map (codmp_xboxr.map) + undname; every
// address below is the map's runtime VA for the core.o data symbol. RTTI
// type-descriptor rows (??_R0) and ngl-inline globals are omitted.
// ============================================================================

#pragma once

#include <stdio.h>

#include "core/math_types.h"
#include "core/PoolAllocator.h"
#include "core/memory_types.h"
#include "engine/broc_types.h"
#include "game/cvar_types.h"
#include "game/core/core_types.h"
#include "game/core/core_systems.h"

// ELanguage is defined elsewhere (shell/UI object); keep ABI-correct 4-byte.
typedef int ELanguage;  // TODO: enum values from IDA
struct ae_heap;

// ============================================================================
// Math / misc
// ============================================================================
extern float (*bytedirs)[3];                  // 0x011C7790
extern int   g_DOBJF_NOT_RENDERED_LAST_FRAME; // 0x011C7F2C
extern bool  gUseControllerLagFix;            // 0x011C7F30
extern const char* const gEmptyStr;           // 0x011C7F34
extern char* (*g_ctrlIconInfo)[2];            // 0x011C7F58
extern float gLensScaleAmount;                // 0x011C7FF8
extern mat3_t mat3_default;                   // 0x011C8024

// ============================================================================
// Filesystem cvars/globals (files.cpp)
// ============================================================================
extern cvar_t* fs_copyfiles;              // 0x012E5AB0
extern cvar_t* fs_cdpath;                 // 0x012E5AB4
extern cvar_t* fs_basegame;               // 0x012E5EC4
extern char*   fs_gamedir;                // 0x012E5ED0
extern cvar_t* fs_restrict;               // 0x012E5F68
extern int     fs_numServerPaks;          // 0x012E5F6C
extern int     fs_checksumFeed;           // 0x012E5F7C
extern cvar_t* fs_ignoreLozalized;        // 0x012E8EAC
extern cvar_t* fs_gamedirvar;             // 0x012E8EB0
extern int     fs_loadStack;              // 0x012E8EB4
extern cvar_t* fs_homepath;               // 0x012EFFB0
extern fileHandleData_t* fsh;             // 0x012EFFB8
extern char*   fs_bsp_gamedir;            // 0x012E64A0
extern cvar_t* fs_debug;                  // 0x012E6524
extern cvar_t* fs_basepath;               // 0x012E768C
extern searchpath_s* fs_searchpaths;      // 0x012F0334
extern filelist_s*   fs_nonpackfilelist;  // 0x012F0338
extern searchpath_s* fs_memorysearchpaths;// 0x012F033C
extern filelist_s*   fs_memorynonpackfilelist;  // 0x012F0340
extern char*   lastValidBase;             // 0x012E6600
extern char*   lastValidGame;             // 0x012EFF30

// ============================================================================
// Common (common.cpp) cvars/state
// ============================================================================
extern cvar_t* com_developer;         // 0x012E5EB8
extern cvar_t* com_statmon;           // 0x012E5EC0
extern int     com_skelTimeStamp;     // 0x012E5EC8
extern cvar_t* com_viewlog;           // 0x012E5F58
extern cvar_t* com_fixedtime;         // 0x012E5F64
extern cvar_t* com_logfile;           // 0x012E5F70
extern bool    gQuickStart;           // 0x012E5F74
extern int     com_journalDataFile;   // 0x012E5F78
extern int     com_journalFile;       // 0x012E5F80
extern cvar_t* com_timescale;         // 0x012E5F84
extern int     time_backend;          // 0x012E5F88
extern cvar_t* com_sv_running;        // 0x012E5F90
extern bool    gIsWorkspaceMap;       // 0x012E5F94
extern char*   com_consoleLines[128]; // 0x012E5F98
extern bool    gDoNotPlayCampaignMovies;  // 0x012E6418
extern _iobuf* debuglogfile;          // 0x012E641C
extern int     com_fileAccessed;      // 0x012E6520
extern char**  com_argv;              // 0x012E6528
extern char*   com_errorMessage;      // 0x012E6688
extern int     com_numConsoleLines;   // 0x012E7688
extern cvar_t* com_speeds;            // 0x012E7690
extern int     time_game;             // 0x012E7698
extern int     time_frontend;         // 0x012E76A0
extern cvar_t* com_journal;           // 0x012E8EA8
extern cvar_t* com_animCheck;         // 0x012EFB00
extern cvar_t* com_developer_script;  // 0x012EFB08
extern bool*   gControllerWarningDialogIsActive;  // 0x012EFB0C
extern int     com_argc;              // 0x012EFF20
extern cvar_t* com_maxfps;            // 0x012EFF24
extern int     com_frameTime;         // 0x012EFF28
extern int     com_fullyInitialized;  // 0x012F031C
extern int     com_errorEntered;      // 0x012F0320
extern int     com_frameNumber;       // 0x012F0324
extern float   g_screendelta;         // 0x012F0360

// ============================================================================
// Cvar system (cvar.cpp)
// ============================================================================
extern cvar_t* cl_stanceHoldTime;   // 0x012E5F50
extern cvar_t* cl_frameadvance;     // 0x012E5F60
extern cvar_t* cvar_vars;           // 0x012E5F8C
extern int     cvar_modifiedFlags;  // 0x012E7694
extern int     cvar_numIndexes;     // 0x012EFF10
extern PoolAllocator* gCommonPoolAllocator;  // 0x012EFF18
extern cvar_t* com_cl_running;      // 0x012EFF1C
extern cvar_t* cvar_cheats;
extern cvar_t cvar_indexes[630];
extern cvar_t* hashTable[256];
extern char sCvarBuff1[1024];
extern char sCvarBuff2[4096];

// ============================================================================
// Effect / dialogue / STB singletons + state
// ============================================================================
extern int     g_debug_db;             // 0x012F036C
extern int     g_debug_sync_queries;   // 0x012F0378
extern bool    g_disable_dialogue;     // 0x012F037C
extern bool    g_indoor;               // 0x012F037D
extern bool    g_debug_effects;        // 0x012F0388
extern ELanguage gLanguage;            // 0x012F03A4
extern Broc::string gNULLString;       // 0x012F03EC
extern SoundOptions gSoundOptions;     // 0x012F03F0
extern Broc::string gFootSplashEffect; // 0x012F0450
extern ParticleParams gParticleParams; // 0x012F0458
extern void*  pWeaponInfoMemory;       // 0x012F0364
extern int    iWeaponInfoSource;       // 0x012F0368

// ============================================================================
// Heaps / misc owners
// ============================================================================
extern ae_heap* gActorHeap;            // 0x012F035C
extern Entity*  gLensLightSource;      // 0x012F03E4
extern int      gLensAlphaAmount;      // 0x012F03E8

// ============================================================================
// Spinner / lens flare textures (declared as opaque pointers)
// ============================================================================
extern void*  sSpinnerFrames[];        // 0x012F03AC (nglTexture*[])
extern int    sLastSpinnerFrame;       // 0x012F03CC
extern void*  gLensFlareTextures[];    // 0x012F03D0 (nglTexture*[])

// class statics declared here for the singleton holder pattern (verified VAs)
namespace EffectEventSysStatics {
extern EffectEventSys* sInst;          // 0x012F0380
}
namespace DbTablesetMgrStatics {
extern void* sInst;                    // 0x012F0370 (DbTablesetMgr*)
}
namespace DialogueManagerStatics {
extern DialogueManager* sInst;         // 0x012F0374
}
namespace ConfigStringManagerStatics {
extern ConfigStringManager* sInst;     // 0x012F039C
}
namespace STBManagerStatics {
extern void* sInst;                    // 0x012F03A0 (STBManager*)
}
namespace CtrlIconStatics {
extern void* sInst;                    // 0x012F03A8 (CtrlIcon*)
}
namespace AnimHeapStatics {
extern AnimHeap* sInst;                // 0x012F0398
}
namespace RumbleManagerStatics {
extern RumbleManager::InstanceHolder sInstHolder;  // 0x012F042C
}
