// ============================================================================
// sv_globals.cpp — server data globals (sv.o data section)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

serverStatic_t svs;              // ?svs@@3UserverStatic_t@@A  0x12FA700
server_t       sv;               // ?sv@@3Userver_t@@A         0x12FA8A0
vm_s*          gvm;              // ?gvm@@3PAUvm_s@@A          0x12FA810
cvar_t*        sv_reloading;     // ?sv_reloading@@3PAUcvar_t@@A  0x12FA808
cvar_t*        sv_showCommands;  // ?sv_showCommands@@3PAUcvar_t@@A 0x12FA750
cvar_t*        sv_mapname;       // ?sv_mapname@@3PAUcvar_t@@A  0x12FA74C
cvar_t*        sv_killserver;    // ?sv_killserver@@3PAUcvar_t@@A 0x12FA740
cvar_t*        sv_serverid;      // ?sv_serverid@@3PAUcvar_t@@A 0x12FA784

// Typed manager singletons (sv_stubs.h class declarations)
PakManager*    PakManager::sInst = NULL;       // ?sInst@PakManager@@2PAV1@A
XModelManager* XModelManager::sInst = NULL;    // ?sInst@XModelManager@@2PAV1@A
PathNodeMgr*   PathNodeMgr::sInst = NULL;      // ?sInst@PathNodeMgr@@2PAU1@A
StreamZoneManager* StreamZoneManager::sInst = NULL;  // ?sInst@StreamZoneManager@@2PAV1@A
AeThreadManager AeThreadManager::sInst;        // ?sInst@AeThreadManager@@0V1@A (scr.o)
cvar_t*        sv_framerate_smoothing;  // ?sv_framerate_smoothing@@3PAUcvar_t@@A 0x12FA75C
cvar_t*        sv_gameskill;     // ?sv_gameskill@@3PAUcvar_t@@A 0x12FA744
int            sv_map_restart;   // ?sv_map_restart@@3HA        0x12FA748
int            sv_snapshotFrameNumber;  // ?sv_snapshotFrameNumber@@3HA 0x12FA780
char           sv_save_filename[128];  // ?sv_save_filename@@3PADA 0x12FA788
char           gNextMapArgv[64];  // ?gNextMapArgv@@3PADA       0x12FA760
int            com_time;         // ?com_time@@3HA              0x12FA754
int            com_inServerFrame;// ?com_inServerFrame@@3HA     0x12FA758
int            bSV_AllowedAllocSkel;  // ?bSV_AllowedAllocSkel@@3HA 0x12FA814
bool           gExitGame;        // ?gExitGame@@3_NA            0x12FA80F
bool           gPumpThreads;     // ?gPumpThreads@@3_NA         0x12FA80D
bool           gPumpThreadsForMapChange; // ?gPumpThreadsForMapChange@@3_NA 0x12FA80E
bool           gMPLoadingUnthreaded;    // ?gMPLoadingUnthreaded@@3_NA 0x12FA80C
