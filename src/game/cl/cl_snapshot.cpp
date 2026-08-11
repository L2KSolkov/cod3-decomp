// ============================================================================
// cl_snapshot.cpp - snapshot getters + cgame time (cl.o cl_cgame.cpp etc.)
// 7 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"

#include "game/game_types.h"

#include <string.h>

struct PakInfoNode;

// Minimal views (full classes in game/sv/sv_stubs.h / g_local.h).
class PakManager {
public:
    static PakManager* sInst;
    void ClearUserDistance(const PakInfoNode* cpak);
    void SyncUnloadPak(TPakId id);
};  // ?sInst@PakManager@@2PAV1@A
class EffectEventSys { public: static EffectEventSys* sInst; };  // ?sInst@EffectEventSys@@2PAV1@A


extern int dword_F6A290[4 * 802];  // Xbox dev/retail flag array @ 0xF6A290

// ============================================================================
// Externs
// ============================================================================
extern void Com_Error(int code, const char* fmt, ...);
extern int com_time;
struct cvar_t;
extern cvar_t* com_cl_running;
extern struct vm_s { int (__cdecl* systemCall)(int*); }* gvm;
extern int VM_Call(struct vm_s* vm, int callnum, ...);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);
extern void nglWaitForRendering();
extern void PakManager_ClearUserDistance(void* self, const void* cpak);
extern void PakManager_SyncUnloadPak(void* self, int id);
struct PakInfoNode;
const PakInfoNode* const sLoadingScreenInfo = nullptr;  // ?sLoadingScreenInfo@@3PBUPakInfoNode@@B (cl.o @ 0x1304CAC)
extern void GamePause_SetAllPaused(bool paused);
extern void InGameMenuSystem_ActivateMenu(void* self, int menu);

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

// ============================================================================
// glconfig_t / snapshot types (cl.o data)
// ============================================================================
struct glconfig_t {
    int data[64];
};
struct playerStateSnapshot_t {
    unsigned char data[1024];
};
struct snapshot_t {
    int snapFlags;
    int serverCommandSequence;
    int serverTime;
    playerStateSnapshot_t ps;
};
glconfig_t unk_F17118;  // ?unk_F17118@@3Uglconfig_t@@A (cl.o)

// cl[] snapshot fields
struct clSnapshotFields {
    int messageNum;
    int serverTime;
    int parseEntitiesNum;
};
struct clSnapshotEntry {
    int valid;
    int snapFlags;
    int serverCommandNum;
    int serverTime;
    int parseEntitiesNum;
    playerStateSnapshot_t ps;
};
// Extend the cl[] view: snap.messageNum/serverTime + snapshots[] ring
int cl_parseEntitiesNum[2];  // cl.o BSS
extern clSnapshotEntry cl_snapshots[2][4];

// ============================================================================
// Snapshot getters
// ============================================================================

// ea: 0x528440
void CL_GetGlconfig(glconfig_t* glconfig)
{
    *glconfig = unk_F17118;
}

// ea: 0x52E250
void CL_GetGlconfigUI(glconfig_t* config)
{
    *config = unk_F17118;
}

// ea: 0x528670
void CL_SaveWrite(const void* buffer, unsigned int len)
{
    ASSERT("0", "c:\\cod\\code\\game\\cl_cgame.cpp", 163);
    memcpy(mem_heap_malloc_ctx(len, 16, "hunk",
                               "c:\\cod\\code\\game\\cl_cgame.cpp", 165),
           buffer, len);
}

// ea: 0x5286F0
void CL_LoadRead(const void* buffer, int len)
{
    VM_Call(gvm, 21, buffer, len);
}

// ea: 0x528710
void CL_GetCurrentSnapshotNumber(int* snapshotNumber, int* serverTime)
{
    *snapshotNumber = cl[currCl].snap.messageNum;
    *serverTime = cl[currCl].snap.serverTime;
}

// ea: 0x528750
int CL_GetSnapshot(int snapshotNumber, snapshot_t* snapshot)
{
    if (snapshotNumber > cl[currCl].snap.messageNum)
        Com_Error(1, "CL_GetSnapshot: bad snapshot number");
    if (cl[currCl].snap.messageNum - snapshotNumber >= 1
        || cl_snapshots[currCl][0].valid == 0
        || cl_parseEntitiesNum[currCl]
               - cl_snapshots[currCl][0].parseEntitiesNum >= 2048)
    {
        return 0;
    }
    snapshot->snapFlags = cl_snapshots[currCl][0].snapFlags;
    snapshot->serverCommandSequence = cl_snapshots[currCl][0].serverCommandNum;
    snapshot->serverTime = cl_snapshots[currCl][0].serverTime;
    memcpy(&snapshot->ps, &cl_snapshots[currCl][0].ps, sizeof(snapshot->ps));
    return 1;
}

// ea: 0x528E70
int CL_FirstSnapshot()
{
    if (com_cl_running == 0)
    {
        ASSERT("com_cl_running->integer", "c:\\cod\\code\\game\\cl_cgame.cpp", 1828);
    }
    nglWaitForRendering();
    PakManager::sInst->ClearUserDistance((const PakInfoNode*)sLoadingScreenInfo);
    if (PAK_ID_INVALID != *(int*)sLoadingScreenInfo)
        PakManager::sInst->SyncUnloadPak((TPakId)*(int*)sLoadingScreenInfo);
    GamePause_SetAllPaused(false);
    extern void* g_femanager_IGMS_cur();
    extern void g_femanager_IGO_Update(int);
    if (g_femanager_IGMS_cur() != nullptr)
        g_femanager_IGO_Update(0);
    if (dword_F6A290[0] == 2)
        InGameMenuSystem_ActivateMenu((void*)0, 12);
    cls.state = 2;  // CA_ACTIVE
    cl[0].serverTime = com_time;
    cl[0].oldServerTime = com_time;
    return com_time;
}

// ea: 0x528F60
int CL_SetCGameTime()
{
    if (cls.state == 2)  // CA_ACTIVE
    {
        cl[0].serverTime = com_time;
        cl[0].oldServerTime = com_time;
        return com_time;
    }
    return 0;
}
