// ============================================================================
// sv_misc.cpp — remaining server functions (sv_game/sv_main/sv_world of sv.o)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"
#include "ngl/ngl_dx_gpu.h"

#include <string.h>
#include <stdlib.h>
#include <intrin.h>

// ============================================================================
// sv.o small accessors (verified against IDA)
// ============================================================================

// PakManagerContext (sv.o 0x51E0C0/0x51E0F0; RAII pak-heap context)
class PakManagerContext {
public:
    PakManagerContext(TPakId id);  // ??0PakManagerContext@@QAE@W4TPakId@@@Z
    ~PakManagerContext();          // ??1PakManagerContext@@QAE@XZ
};
PakManagerContext::PakManagerContext(TPakId id)
{
    PakManager::sInst->PushContext(id);
}
PakManagerContext::~PakManagerContext()
{
    PakManager::sInst->PopContext();
}

// cdl_object_t (g_local.h view; cflags +0x00)
struct cdl_object_t {
    int cflags;  // +0x00

    int get_cflags() const;  // ?get_cflags@cdl_object_t@@QBEHXZ (sv.o 0x51E0A0)
};

// ea: 0x51E0A0
int cdl_object_t::get_cflags() const
{
    return cflags;
}

// ea: 0x528090
bool intersect_aabb_aabb(const math::Position3* min0,
                         const math::Position3* max0,
                         const math::Position3* min1,
                         const math::Position3* max1)
{
    __m128 d = _mm_max_ps(_mm_sub_ps(min1->v, max0->v),
                          _mm_sub_ps(min0->v, max1->v));
    return (_mm_movemask_ps(_mm_cmplt_ps(d, _mm_setzero_ps())) & 7) == 7;
}

// ea: 0x51E150
void FEMenuSystem::SetSystemActive(bool active)
{
    is_active = active;
}

// ea: 0x51E1A0
void FEManager::SetInGameMenusActive(bool active, int client)
{
    InGameMenuSystem* v3 = mIGMS[client];
    if (v3 != nullptr)
        v3->is_active = active;
}

// ea: 0x528390
void FEManager::UpdateIGO(float time_inc)
{
    if (IGO != nullptr)
        IGO->Update(time_inc);
}

// ea: 0x5283A0
void FEManager::UpdateInSceneIGO(float time_inc)
{
    if (IGO != nullptr)
        IGO->UpdateInScene(time_inc);
}

// ea: 0x5283B0
bool FEManager::AARMenusActive()
{
    return mAARS != nullptr && mAARS->IsSystemActive();
}

// ea: 0x5283C0
bool FEManager::FrontEndMenusActive(int client)
{
    (void)client;
    return fems != nullptr && fems->IsSystemActive();
}

// ea: 0x5283E0
void FEManager::SetFrontEndMenusActive(bool active, int client)
{
    (void)client;
    if (fems != nullptr)
        fems->is_active = active;
}

// ea: 0x528400
AARMenuSystem* FEManager::GetAARS()
{
    return mAARS;
}

// shell.o stubs for the menu-system methods the cl.o FEManager wrappers use
bool FEMenuSystem::IsSystemActive()
{
    return is_active;
}
bool AARMenuSystem::IsSystemActive()
{
    return false;
}
// ea: 0x528020
void MultiplayerMgr::setEnableLinkCheck(bool enabled)
{
    mLinkCheckEnabled = enabled;
}

// ea: 0x528030
bool MultiplayerMgr::getEnableLinkCheck()
{
    return mLinkCheckEnabled;
}

// ea: 0x528040
void MPPlayer::SetClientIndex(int index)
{
    mClientIndex = index;
}

// ea: 0x528050
int MPPlayer::GetClientIndex()
{
    return mClientIndex;
}

// ea: 0x528190
const char* CheckpointMgr::GetCheckpointMapName()
{
    if (mCurrentMapName.mBlock != nullptr)
        return (const char*)(mCurrentMapName.mBlock + 1);
    return defaultFileName;
}

// ea: 0x51E0B0 (XDK shim)
long __stdcall D3DDevice::PersistDisplay()
{
    D3DDevice_PersistDisplay();  // void XDK shim (d3d8.h)
    return 0;  // S_OK
}

// PathNodeMgr stubs (mp_actors.o / path.o; ported later)
void PathNodeMgr::InitPaths() {}
void PathNodeMgr::ValidateAllNodes() {}
void PathNodeMgr::SetCoverNodeStatus(const Broc::string& name, int inValid)
{
    (void)name; (void)inValid;
}
void PathNodeMgr::AttachSentientToChainNode(sentient_s* pSentient,
                                            const Broc::string& targetname)
{
    (void)pSentient; (void)targetname;
}
void PathNodeMgr::ConnectPathsForEntity(Entity* ent) { (void)ent; }
void PathNodeMgr::DisconnectPathsForEntity(Entity* ent) { (void)ent; }
void PathNodeMgr::NodeList() {}
void PathNodeMgr::CheckpointResetNodes() {}

// AeThreadManager (core.o; stubs, port later)
void AeThreadManager::Execute(float deltaT)
{
    (void)deltaT;
}
void AeThreadManager::KillAllThreads()
{
    // stub
}

// ============================================================================
// Cross-object externs
// ============================================================================
enum errorParm_t;
extern void  Com_Error(errorParm_t code, const char* fmt, ...);
extern int   Com_Milliseconds(void);
extern void  CM_AdjustAreaPortalState(int area1, int area2, int open);
extern void  CM_UnlinkEntity(EntityShared* ent);
extern DCGSet* TempBoxModel(const math::Position3& mins,
                            const math::Position3& maxs,
                            int contents, int capsule);
extern void  TraceXFormed(trace_t* results, const math::Position3& start, const math::Position3& end,
                          const math::Position3& mins, const math::Position3& maxs, DCGSet* model,
                          int brushmask, const math::Position3& origin, const math::Position3& angles, int capsule);
extern void  SCR_UpdateScreen(void);
extern void  j_nullsub_35(void);
static int   SV_InitGameVM(int restart, int savegame);   // ea: 0x520110
extern void  CL_FlushDebugData(int fromServer);
extern char* Cvar_InfoString(int bit);
extern char* Cvar_InfoString_Big(int bit);
extern int   cvar_modifiedFlags;
extern void  SV_SetConfigstring(int index, const char* val);
extern void  CL_ShutdownAll(void);
extern void  CL_StartHunkUsers(void);
extern void  FS_Shutdown(int closemfp);
extern void  FS_Restart(int checksumFeed);
extern void  FS_ClearMemory(void);
extern int   Sys_Milliseconds(void);
extern void  j_nullsub_86(int phase);
extern void  SV_Shutdown(void);
extern void  LocalClient_SetNumLocalClients(int num);
extern void  CGBankManager_UnloadAll(void);
extern void  AnimBankManager_UnloadAll(void);
extern void  LiveWrapper_ClearRemotePlayers(void* handle);
extern void* MPLiveEngine_GetHandle(void);
class MPUIInterface {
public:
    static bool IsOnlineGame();  // ?IsOnlineGame@MPUIInterface@@SA_NXZ
};
extern const math::Position3& Float4_Zero_2;
unsigned __int64 sLastTime_0;            // ?sLastTime_0 (sv_game.cpp static)
unsigned int _S8_40;              // ?$S8_40 (sv_game.cpp static)
extern int   SV_GameSystemCalls(int* args);
extern void  CL_ParseGamestate(Broc::string* configstrings);

// ============================================================================
// SV_SetCheckSum — ea: 0x51F300
// ============================================================================
void SV_SetCheckSum(int checksum) {
    sv.checksum = checksum;
}

// ============================================================================
// SV_RestartGameProgs — ea: 0x520170
// ============================================================================
int SV_RestartGameProgs(int savegame) {
    if (gvm == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 1119;
        AeAssert::gCurrentExpr = "gvm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    VM_Call(gvm, 1, savegame);
    return SV_InitGameVM(1, savegame);
}

// ============================================================================
// SV_InitGameProgs — ea: 0x5201E0
// ============================================================================
void SV_InitGameProgs(int savegame) {
    gvm = VM_Create("game", SV_GameSystemCalls);
    if (gvm == NULL)
        Com_Error((errorParm_t)1, "\x15" "VM_Create on game failed");
    SV_InitGameVM(0, savegame);
}

// ============================================================================
// SV_InitGameVM — ea: 0x520110 (static)
// ============================================================================
static int SV_InitGameVM(int restart, int savegame) {
    j_nullsub_35();
    int checksum = sv.checksum;
    int v2 = Com_Milliseconds();
    int v3 = VM_Call(gvm, 0, v2, restart, savegame, checksum);
    j_nullsub_35();
    int v4 = 0;
    do {
        svs.clients[v4 / 0x1370].mEntityHandle.mHandle.mVal = 0;
        v4 += 4976;
    } while (v4 < 0x13700);
    return v3;
}

// ============================================================================
// SV_AdjustAreaPortalState — ea: 0x51EAF0
// ============================================================================
void SV_AdjustAreaPortalState(Entity* ent, int open) {
    int areanum2 = ent->r.areanum2;
    if (areanum2 != -1)
        CM_AdjustAreaPortalState(ent->r.areanum, areanum2, open);
}

// ============================================================================
// SV_inPVS — ea: 0x51EA50
// ============================================================================
int SV_inPVS(const math::Position3& p1, const math::Position3& p2) {
    (void)p1; (void)p2;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;   // JSV
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
    AeAssert::gCurrentLine = 130;
    AeAssert::gCurrentExpr = NULL;
    if (!AeAssert::IsIgnored() && AeAssert::Warning(defaultFileName))
        __debugbreak();
    return 1;
}

// ============================================================================
// SV_inPVSIgnorePortals — ea: 0x51EAA0
// ============================================================================
int SV_inPVSIgnorePortals(const float* const p1, const float* const p2) {
    (void)p1; (void)p2;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;   // JSV
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
    AeAssert::gCurrentLine = 250;
    AeAssert::gCurrentExpr = NULL;
    if (!AeAssert::IsIgnored() && AeAssert::Warning(defaultFileName))
        __debugbreak();
    return 1;
}

// ============================================================================
// SV_CheckLoadLevel — ea: 0x51F200
// ============================================================================
void SV_CheckLoadLevel(int savegame) {
    if (++com_skelTimeStamp == 0)
        com_skelTimeStamp = 1;
    if (bSV_AllowedAllocSkel != 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 1191;
        AeAssert::gCurrentExpr = "!bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 1;
    VM_Call(gvm, 9, savegame, sv.checksum);
    if (bSV_AllowedAllocSkel == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 1197;
        AeAssert::gCurrentExpr = "bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 0;
    com_time = VM_Call(gvm, 20);
}

// ============================================================================
// LoadingMenuCallback — ea: 0x51F640
// ============================================================================
void LoadingMenuCallback(float progress) {
    g_femanager.UpdateLoadingMenu(progress);
    unsigned __int64 v1;
    if ((_S8_40 & 1) != 0) {
        v1 = sLastTime_0;
    } else {
        _S8_40 |= 1u;
        v1 = __rdtsc();
        sLastTime_0 = v1;
    }
    unsigned __int64 v2 = __rdtsc();
    if ((v2 - v1) >= 24444444.0) {
        sLastTime_0 = v2;
        SCR_UpdateScreen();
    }
}

// ============================================================================
// GetShortName — ea: 0x51F710
// ============================================================================
const char* GetShortName(const char* long_name) {
    return long_name;
}

// ============================================================================
// SendClientThinkMsg — ea: 0x51F7D0
// ============================================================================
void SendClientThinkMsg() {
    if (svs.clients != NULL)
        VM_Call(gvm, 6, &svs.clients[currCl].mEntityHandle);
}

// ============================================================================
// MatrixTransposeTransformVector43 — ea: 0x51FB40
// ============================================================================
void MatrixTransposeTransformVector43(const math::Position3& in1, const float (*const in2)[3], math::Position3& out) {
    float v3 = in1.v.m128_f32[1] - (*in2)[10];
    float v4 = in1.v.m128_f32[2] - (*in2)[11];
    float v5 = in1.v.m128_f32[0] - (*in2)[9];
    out.v.m128_f32[0] = ((*in2)[2] * v4) + ((*in2)[1] * v3) + ((*in2)[0] * v5);
    out.v.m128_f32[1] = ((*in2)[5] * v4) + ((*in2)[4] * v3) + ((*in2)[3] * v5);
    out.v.m128_f32[2] = ((*in2)[8] * v4) + ((*in2)[7] * v3) + ((*in2)[6] * v5);
}

// ============================================================================
// SV_ExpandNewlines — ea: 0x51F720
// ============================================================================
static char string[1024];   // ?string  (sv_game.cpp static)
char* SV_ExpandNewlines(char* in) {
    char* v1 = in;
    char v2 = *in;
    unsigned int i = 0;
    for (; v2 != 0; v2 = *++v1) {
        if (i >= 0x3FD)
            break;
        if (v2 == 10) {
            string[i++] = 92;
            string[i] = 110;
        } else {
            if (v2 == 20 || v2 == 21)
                continue;
            string[i] = v2;
        }
        ++i;
    }
    string[i] = 0;
    return string;
}

// ============================================================================
// SV_PreFrame — ea: 0x520BA0
// ============================================================================
void SV_PreFrame(int msec) {
    CL_FlushDebugData(1);
    if ((cvar_modifiedFlags & 4) != 0) {
        char* v1 = Cvar_InfoString(4);
        SV_SetConfigstring(0, v1);
        cvar_modifiedFlags &= ~4u;
    }
    if ((cvar_modifiedFlags & 8) != 0) {
        char* v2 = Cvar_InfoString_Big(8);
        SV_SetConfigstring(1, v2);
        cvar_modifiedFlags &= ~8u;
    }
    VM_Call(gvm, 11, msec);
}

// ============================================================================
// SV_SwapClients — ea: 0x521190
// ============================================================================
void SV_SwapClients(int client1, int client2) {
    client_s tmp;
    int port1 = 4976 * client1;
    int port2 = 4976 * client2;
    // Swap netchan outgoing sequence slots [26..30] (2 ints) around
    unsigned int seq1 = *(unsigned int*)&svs.clients[client1].netchan[26];
    unsigned int seq2 = *(unsigned int*)&svs.clients[client2].netchan[26];
    memcpy(&tmp, &svs.clients[client1], sizeof(client_s));
    memcpy(&svs.clients[client1], &svs.clients[client2], sizeof(client_s));
    memcpy(&svs.clients[client2], &tmp, sizeof(client_s));
    *(unsigned int*)&svs.clients[client1].netchan[26] = seq1;
    *(unsigned int*)&svs.clients[client2].netchan[26] = seq2;
}

// ============================================================================
// SV_ReallyExitGame_f — ea: 0x5208F0
// ============================================================================
void SV_ReallyExitGame_f() {
    if (MPUIInterface::IsOnlineGame()) {
        void* Handle = MPLiveEngine_GetHandle();
        LiveWrapper_ClearRemotePlayers(Handle);
    }
    j_nullsub_86(GAME_PHASE_LOADING);
    PakManager::sInst->FillBanks();
    if (g_femanager.mIGMS[0] != NULL)
        g_femanager.mIGMS[0]->SetActiveMenu(-1);
    g_femanager.inGame = false;
    g_femanager.IGO_active = false;
    MultiplayerMgr::sInst->ExitLevel();
    if (dword_F6A290[0] == 2) {
        currCl = NS_CLIENT;
        SCR_UpdateScreen();
    }
    currCl = NS_CLIENT;
    sv_save_filename[0] = 0;
    sv_map_restart = 0;
    CL_ShutdownAll();
    LocalClient_SetNumLocalClients(1);
    SV_Shutdown();
    PakManager::sInst->UnloadAll();
    CGBankManager_UnloadAll();
    AnimBankManager_UnloadAll();
    CL_StartHunkUsers();
    FS_Shutdown(1);
    unsigned int v1 = (unsigned int)Sys_Milliseconds();
    srand(v1);
    FS_Restart(0);
    FS_ClearMemory();
    InGameMenuSystem* v2 = g_femanager.mIGMS[currCl];
    if (v2 != NULL)
        v2->is_active = false;
    j_nullsub_86(GAME_PHASE_FRONTEND);
}

// ============================================================================
// SV_ClipHandleForEntity — ea: 0x51FC00
// ============================================================================
DCGSet* SV_ClipHandleForEntity(const Entity* ent) {
    DCGSet* result = ent->r.bmodel;
    if (result == NULL) {
        int contents = ent->r.contents;
        math::Position3* p_maxs = (math::Position3*)&ent->r.maxs;
        if ((ent->r.svFlags & 0x200) != 0)
            return TempBoxModel(ent->r.mins, *p_maxs, contents, 1);
        else
            return TempBoxModel(ent->r.mins, *p_maxs, contents, 0);
    }
    return result;
}

// ============================================================================
// SV_UnlinkEntity — ea: 0x51FC60
// ============================================================================
void SV_UnlinkEntity(Entity* gEnt) {
    gEnt->r.linked = 0;
    CM_UnlinkEntity(&gEnt->r);
}

// ============================================================================
// SV_EntityContact — ea: 0x520080
// ============================================================================
int SV_EntityContact(const math::Position3& mins, const math::Position3& maxs, const Entity* gEnt, int capsule) {
    trace_t tr;
    memset(&tr, 0, sizeof(tr));
    DCGSet* v5 = SV_ClipHandleForEntity(gEnt);
    tr.fraction = 1.0f;
    math::Position3 v9 = Float4_Zero_2;
    math::Position3 zero = Float4_Zero_2;
    TraceXFormed(&tr, zero, v9, mins, maxs, v5, -1, gEnt->r.currentOrigin, gEnt->r.currentAngles, capsule);
    return (int)(tr.fraction < 1.0f);
}
