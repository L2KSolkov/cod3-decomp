// ============================================================================
// cl_snapshot.cpp - snapshot getters + cgame time (cl.o cl_cgame.cpp etc.)
// 7 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"
#include "game/snapshot_types.h"

#include "game/game_types.h"
#include "game/ui_types.h"

#include <string.h>

struct PakInfoNode {
    uint8_t _pad00[0xB4];
    TPakId pakId;
};

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
enum errorParm_t;
extern void Com_Error(errorParm_t code, const char* fmt, ...);
extern int com_time;
struct cvar_t;
extern cvar_t* com_cl_running;
extern struct vm_s { int (__cdecl* systemCall)(int*); }* gvm;
extern int VM_Call(struct vm_s* vm, int callnum, ...);
extern void* mem_heap_malloc_ctx(int alignment, unsigned int size,
                                 const char* ctx, const char* file, int line);
extern void nglWaitForRendering();
extern void PakManager_ClearUserDistance(void* self, const void* cpak);
extern void PakManager_SyncUnloadPak(void* self, int id);
const PakInfoNode* sLoadingScreenInfo = nullptr;  // ?sLoadingScreenInfo@@3PBUPakInfoNode@@B (cl.o @ 0x1304CAC)
extern void GamePause_SetAllPaused(bool paused);

// IDA FEManager layout: only the IGO and mIGMS fields are needed here.
class FEManager {
public:
    unsigned char _pad00[0x14];
    IGOFrontEnd* IGO;                         // +0x14
    unsigned char _pad18[0xB0];
    InGameMenuSystem* mIGMS[1];               // +0xC8
};
static_assert(offsetof(FEManager, IGO) == 0x14,
              "FEManager::IGO offset mismatch");
static_assert(offsetof(FEManager, mIGMS) == 0xC8,
              "FEManager::mIGMS offset mismatch");
extern FEManager g_femanager;

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
    const char* renderer_string;
    const char* vendor_string;
    const char* version_string;
    const char* extensions_string;
    const char* wgl_extensions_string;
    int maxTextureSize;
    int maxActiveTextures;
    int maxHardwareLights;
    int colorBits;
    int depthBits;
    int stencilBits;
    int deviceSupportsGamma;
    int anisotropicAvailable;
    float maxAnisotropy;
    int ARB_texture_env_add;
    int ARB_texture_cube_map;
    int ARB_texture_env_combine;
    int ARB_texture_env_dot3;
    int ARB_vertex_buffer_object;
    int ARB_vertex_program;
    int EXT_rescale_normal;
    int NVFogAvailable;
    int NVFogMode;
    int NV_vertex_array_range;
    int NV_fence;
    int NV_register_combiners;
    int NV_texture_shader;
    int ATIMaxTruformTess;
    int ATINormalMode;
    int ATIPointMode;
    int ATI_vertex_array_object;
    int ATI_element_array;
    int ATI_fragment_shader;
    int vidWidth;
    int vidHeight;
    float windowAspect;
    int displayFrequency;
    int isFullscreen;
    int stereoEnabled;
    int textureFilterAnisotropicAvailable;
};
static_assert(sizeof(glconfig_t) == 0xA0, "glconfig_t size mismatch");
glconfig_t unk_F17118;  // ?unk_F17118@@3Uglconfig_t@@A (cl.o)

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
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cl_cgame.cpp";
    AeAssert::gCurrentLine = 163;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("Is this still called?"))
        __debugbreak();
    memcpy(mem_heap_malloc_ctx(16, len, "hunk",
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
        Com_Error((errorParm_t)1, "CL_GetSnapshot: bad snapshot number");
    const clSnapshot& ringSnapshot = cl[currCl].snapshots[0];
    if (cl[currCl].snap.messageNum - snapshotNumber >= 1
        || ringSnapshot.valid == 0
        || cl[currCl].parseEntitiesNum - ringSnapshot.parseEntitiesNum >= 2048)
    {
        return 0;
    }
    snapshot->snapFlags = ringSnapshot.snapFlags;
    snapshot->serverCommandSequence = ringSnapshot.serverCommandNum;
    snapshot->serverTime = ringSnapshot.serverTime;
    memcpy(&snapshot->ps, &ringSnapshot.ps, sizeof(snapshot->ps));
    return 1;
}

// ea: 0x528E70
int CL_FirstSnapshot()
{
    if (com_cl_running->integer == 0)
    {
        ASSERT("com_cl_running->integer", "c:\\cod\\code\\game\\cl_cgame.cpp", 1828);
    }
    nglWaitForRendering();
    PakManager::sInst->ClearUserDistance((const PakInfoNode*)sLoadingScreenInfo);
    if (PAK_ID_INVALID != sLoadingScreenInfo->pakId)
        PakManager::sInst->SyncUnloadPak(sLoadingScreenInfo->pakId);
    GamePause_SetAllPaused(false);
    InGameMenuSystem* igms = g_femanager.mIGMS[currCl];
    if (igms != nullptr)
        igms->is_active = false;
    g_femanager.IGO->Update(0.0f);
    g_femanager.mIGMS[0]->ReturnToPreviousMenu(-1);
    if (dword_F6A290[0] == 2)
        g_femanager.mIGMS[0]->ActivateMenu(12);
    cls.state = 2;  // CA_ACTIVE
    cl[0].serverTime = com_time;
    cl[0].oldServerTime = com_time;
    return com_time;
}

// ea: 0x528F60
void CL_SetCGameTime()
{
    if (cls.state == 2)  // CA_ACTIVE
    {
        cl[0].serverTime = com_time;
        cl[0].oldServerTime = com_time;
    }
}
