// ============================================================================
// pakmanager.cpp - PakManager simple accessors (streamer.o subset)
// Verified against IDA (streamer.o). The full PakManager cluster depends on
// PakFile/BankManager/InplaceTree; only the self-contained members are ported
// here (the rest stay as placeholder LNKs).
// ============================================================================

#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <intrin.h>
#include <new>
#include "core/mem_heap.h"
#include "core/memory_types.h"
#include "core/PoolAllocator.h"
#include "core/color.h"
#include "ngl/nglFont.h"
#include "ngl/nglTexture.h"
#include "ngl/ngl_mesh.h"
#include "ngl/ngl_dx_quad.h"
#include "filesystem/apk.h"
#include "core/ae_fixed_string.h"
#include "core/PoolAllocator.h"
#include "core/tlFixedString.h"
#include "core/tlResourceDirectory.h"
#include "engine/broc_types.h"
#include "input/controller.h"

extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);
extern PoolAllocator* gCommonPoolAllocator;
extern void CurveManager_SetupAllocator(PoolAllocator* allocator);
extern void EntityNotify_SetupAllocators(PoolAllocator* allocator);
extern void PtrFixupTable_Fixup(void* self, void* basePtr);

// PakFile bank flags (PakFile.cpp; header flags + runtime state)
#define PAK_BANK_FLAG_APK_HEADER 0x80      // bank is apk-backed (header)
#define PAK_BANK_FLAG_READ_DONE  0x10000   // async read finished
#define PAK_BANK_FLAG_MEM         0x20000  // memory allocated
#define PAK_BANK_FLAG_APK_LOADED  0x40000  // apk loaded in place

// WorldSpawn (SceneEntity base + key-value strings; streamer.o)
struct WorldSpawn {
    uint8_t _pad[0xE4];
    InplaceString ambienttrack;  // +0xE4
    InplaceString message;       // +0xE8
    InplaceString gravity;       // +0xEC
    InplaceString northyaw;      // +0xF0
    float ambient;               // +0xF4
    uint8_t _padF8[0x10C - 0xF8];
    float sundiffusecolor[3];    // +0x10C
    float sundirection[3];       // +0x118
};

extern void SV_SetConfigstring(int index, const char* val);  // sv.o
extern void Cvar_Set(const char* var_name, const char* value);  // core.o
enum errorParm_t;
extern void Com_Error(errorParm_t code, const char* fmt, ...);  // core.o
// IVPointer<T> (game_types.h; intrusive counted pointer, 8 bytes)
struct XModel;
struct ScriptEventHandler;
class Destructible;
class trRefEntity;
class ScriptEventParams;
template <typename T>
class IVPointer {
public:
    T*           mValue;   // +0x00
    unsigned int mPakId;   // +0x04

    bool operator!() const { return mValue == nullptr; }

    T* GetRawPtr();  // ?GetRawPtr@?$IVPointer@VXModel@@@@QAEPAVXModel@@XZ (0x685AA0)
};

// Entity view for the Notify dispatcher + ConvertEntity field copies
// (full type in game_types.h; offsets verified against IDA)
class Entity {
public:
    uint8_t _pad[0xE0];  // EntityState s
    struct Shared {
        uint8_t _pad[0x70];
        math::Position3 currentOrigin;  // +0x70
        math::Position3 currentAngles;  // +0x80
    };
    Shared r;           // +0xE0 (EntityShared)
    uint8_t _pad160[0x230 - 0x160];
    int32_t mPakId;     // +0x230
    uint8_t _pad234[0x244 - 0x234];
    ScriptEventHandler* mScriptEventHandler;  // +0x244
    uint8_t _pad248[0x24C - 0x248];
    IVPointer<Destructible> mDestructible;  // +0x24C (8 bytes)
    uint8_t _pad254[0x258 - 0x254];
    void*   mActor;      // +0x258 (actor_s*)
    uint8_t _pad25C[0x260 - 0x25C];
    void*   mScrVehicle;  // +0x260 (scr_vehicle_t*)
    uint8_t _pad264[0x270 - 0x264];
    IVPointer<XModel> mModel;  // +0x270
    uint8_t _pad278[0x27C - 0x278];
    Broc::string mClassName;  // +0x27C
    HashString mClassNameHash;  // +0x280
    uint8_t _pad284[0x294 - 0x284];
    Broc::string mGroupName;  // +0x294
    uint8_t _pad29C[0x2A4 - 0x29C];
    Broc::string mAnimName;   // +0x2A4
    uint8_t _pad2AC[0x3BE - 0x2AC];
    int16_t mPersistentIndex;  // +0x3BE

    void Notify(HashString h);  // ?Notify@Entity@@QAEXVHashString@@@Z (g_entity_misc.cpp)
    int GetPakId() const { return mPakId; }  // g.o inline
    void SetScriptEventHandler(class ScriptEventHandler* n);  // ?SetScriptEventHandler@Entity@@QAEXPAVScriptEventHandler@@@Z
    class trRefEntity& GetRenderEntity();  // ?GetRenderEntity@Entity@@QAEAAVtrRefEntity@@XZ (game.o)
    void ExecScriptHandler(HashString h,
                           ScriptEventParams* params);  // ?ExecScriptHandler@Entity@@QAEXVHashString@@PAVScriptEventParams@@@Z (game.o)
};
extern void UpdateEntityHash(Entity* ent);  // ?UpdateEntityHash@@YAXPAVEntity@@@Z (g_scr.cpp)

// Entity minimal view (mClassName +0x27C, mClassNameHash +0x280)
struct EntityMin {
    uint8_t _pad[0x27C];
    Broc::string mClassName;   // +0x27C
    HashString mClassNameHash; // +0x280
};

// Minimal str_const_t view (full type in game/logic/g_local.h).
// worldspawn at +0x23C verified against IDA str_const_t member list.
struct str_const_t {
    uint8_t _pad[0x23C];
    Broc::string worldspawn;  // +0x23C
};
extern str_const_t str_const;  // ?str_const@@3Ustr_const_t@@A @ 0xECBD30 (g_globals.cpp)

class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A (effect_events.cpp)
    uint8_t _pad[0x04];
    Entity* mPlayers[16];         // +0x04
    EntityMin* mWorld;            // +0x44
    Entity* GetPlayer(int idx);   // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z (g_entity_misc.cpp)
};
extern int currCl;  // ?currCl@@3HA (platform_xbox/XboxLiveMenus.cpp)

extern int dword_F6A290[4 * 0x322];  // Xbox dev/retail flag array @ 0xF6A290 (defined in effect_events.cpp)
extern int R_CellForPoint(const math::Position3* pos);  // render.o (g_entity_misc.cpp)
extern void G_SetOrigin(Entity* ent, const math::Position3& origin);  // g.o (g_active.cpp)
extern void G_SetAngle(Entity* ent, const math::Position3& angle);    // g.o (g_active.cpp)
extern unsigned short G_NewString(const char* str);  // g.o (g_utils.cpp)
extern double VectorNormalize(math::Dir3& v);  // core.o (q_math.cpp)
extern unsigned char bulletPriorityMap[];     // g.o (g_game2_misc.cpp)
namespace View {
bool IsSplitScreen();  // cg_misc.cpp ?IsSplitScreen@View@@YA_NXZ
}
struct trace_t;
struct collision_context_t;
void g_LocationalTrace(trace_t* results, const math::Position3& start,
                       const math::Position3& end,
                       const collision_context_t& context,
                       unsigned char* priorityMap,
                       float coneAngleTangent);  // g.o (g_trace.cpp)

// RenderInstanceGroups globals (render.o / cg.o / streamer.o)
// phys_static_array<T,CAP> (phys_types.h view; class tag V matches the
// binary mangling; m_slot_array at +CAP*sizeof(T), m_alloc_count +4)
template <typename T, int CAP>
class phys_static_array {
public:
    char m_buffer[CAP * sizeof(T)];   // +0x00
    T* const m_slot_array;            // +CAP*sizeof(T)
    int m_alloc_count;                // +CAP*sizeof(T)+4

    phys_static_array() : m_slot_array((T*)m_buffer), m_alloc_count(0) {}
    T& operator[](int i) { return ((T*)m_buffer)[i]; }
};
struct trGlobals_t {
    uint8_t _pad[0x10];
    float viewParmsOrigin[3];  // +0x10 (viewParms.or.origin)
    uint8_t _pad1C[0x290 - 0x1C];
    void*   world;             // +0x290 (world_t*)
};
extern trGlobals_t tr;  // ?tr@@3UtrGlobals_t@@A (render.o @ 0x13642D0)
extern bool gFirstCamera;  // cg.o (common.cpp)
int gFrameToggle = 0;      // ?gFrameToggle@@3HA @ 0xF5930C (streamer.o)
extern int g_camera_cell;  // render.o @ 0x11E993C
int g_camera_cell = 0;     // stub until render.o lands
extern float gZoomRatio;   // cg.o (cg_view.cpp)
extern phys_static_array<phys_static_array<math::Vector4, 20>, 64> portals;
phys_static_array<phys_static_array<math::Vector4, 20>, 64> portals;  // render.o @ 0x11EA840
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);  // phys_types.h

// EntityHandleDb (game_types.h view; sInst defined in g_globals.cpp)
class EntityHandleDb {
public:
    struct DbElement {
        Entity* mObject;  // +0x00
        int     mKey;     // +0x04
    };
    uint8_t     mFreeIndices[0xA8];  // +0x00 (BitSet<1344>)
    DbElement   mElements[0x540];    // +0xA8
    static EntityHandleDb sInst;  // ?sInst@EntityHandleDb@@0V1@A
};

// collision_context_t (trace_types.h view; 24 bytes)
struct collision_context_t_vtbl {
    bool (__cdecl* filter)(collision_context_t* self, Entity* ent);
};
struct collision_context_t {
    struct collision_context_t_vtbl* __vftable;  // +0x00
    unsigned int pass_entity1;                   // +0x04
    unsigned int pass_entity2;                   // +0x08
    unsigned int pass_owner1;                    // +0x0C
    unsigned int pass_owner2;                    // +0x10
    int          contentmask;                    // +0x14
};
// trace_t (trace_types.h view; startsolid +0x3D, contents +0x28)
struct trace_t {
    uint8_t _pad[0x28];
    int32_t contents;  // +0x28
};

// ngl scratch-mesh helpers (ngl_meshedit.cpp / ngl_aux.cpp / ngl_texture.cpp)
extern gpuVertexFormat cdscratch_vertex_format;  // render/cdScratchVertexDef.cpp @ 0x14CD510
extern void nglAddMeshSection(nglMesh* Mesh, nglMeshSection* Section,
                              nglMaterial* Material, int Flags);
extern nglMesh* auxCreateScratchMesh(int flags, int num);
extern nglTexture* nglGetTexture(const tlFixedString& FileName);

// cdScratchMaterial (render/cdScratchShader.h view; base nglMaterial 16 bytes)
struct cdScratchMaterial {
    uint8_t       _pad[0x10];  // nglMaterial base
    nglTexture*   Texture;     // +0x10
    unsigned int  BlendMode;   // +0x14
    int           MapFlags;    // +0x18
    bool          HeatHaze;    // +0x1C

    cdScratchMaterial(nglTexture* tex, unsigned int BlendMode, int mapflags,
                      bool HeatHaze);  // ??0cdScratchMaterial@@QAE@PAVnglTexture@@IH_N@Z (render.o 0x7C5660)
};

// cdscratch_vertex (cdScratchVertexDef.cpp; 24 bytes, elem offsets 0/12/20)
struct cdscratch_vertex {
    float        Position_x;  // +0x00
    float        Position_y;  // +0x04
    float        Position_z;  // +0x08
    unsigned int Color;       // +0x0C
    float        TexCoord_x;  // +0x10
    float        TexCoord_y;  // +0x14
};

// nglMeshIterator<V,I> (render_xboxr; streamer.o COMDATs 0x6839B0-0x684400)
template <typename V, typename I>
class nglMeshIterator {
public:
    nglMeshSection* section;      // +0x00
    I*              index;        // +0x04
    V*              vertex_base;  // +0x08
    V*              vertex;       // +0x0C
    int             cur_index;    // +0x10

    nglMeshIterator(nglMeshSection* section);  // 0x6839B0
    void BeginStrip(int strip);                // 0x6839F0
    const nglMeshIterator& operator++();       // 0x683A30
    void WriteFlush();                         // 0x683A60
    void WritePosition(float x, float y, float z);  // 0x6843A0
    void WriteColor(unsigned int color);            // 0x6843E0
    void WriteTexCoord(float u, float v);           // 0x684400
};

// ea: 0x6839B0
template <typename V, typename I>
nglMeshIterator<V, I>::nglMeshIterator(nglMeshSection* section)
{
    this->section = section;
    index = (I*)nglLockSectionIndices(section);
    V* v3 = (V*)nglLockSectionVertices(section);
    vertex_base = v3;
    vertex = v3;
    cur_index = 0;
}

// ea: 0x6839F0
template <typename V, typename I>
void nglMeshIterator<V, I>::BeginStrip(int strip)
{
    (void)strip;
    int cur_index = this->cur_index;
    if (cur_index > 0)
    {
        *index = (I)(cur_index - 1);
        I* v3 = index + 1;
        index = v3;
        *v3 = (I)this->cur_index;
        ++index;
    }
}

// ea: 0x683A30
template <typename V, typename I>
const nglMeshIterator<V, I>& nglMeshIterator<V, I>::operator++()
{
    *index = (I)cur_index;
    int cur_index = this->cur_index;
    ++index;
    V* vertex = this->vertex;
    this->cur_index = cur_index + 1;
    this->vertex = vertex + 1;
    return *this;
}

// ea: 0x683A60
template <typename V, typename I>
void nglMeshIterator<V, I>::WriteFlush()
{
    nglUnlockSectionIndices(this->section);
    nglUnlockSectionVertices(this->section);
}

// ea: 0x6843A0
template <typename V, typename I>
void nglMeshIterator<V, I>::WritePosition(float x, float y, float z)
{
    vertex->Position_x = x;
    vertex->Position_y = y;
    vertex->Position_z = z;
}

// ea: 0x6843E0
template <typename V, typename I>
void nglMeshIterator<V, I>::WriteColor(unsigned int color)
{
    vertex->Color = color;
}

// ea: 0x684400
template <typename V, typename I>
void nglMeshIterator<V, I>::WriteTexCoord(float u, float v)
{
    vertex->TexCoord_x = u;
    vertex->TexCoord_y = v;
}

// ea: 0x686A00
template <typename I>
void WriteVertex(nglMeshIterator<cdscratch_vertex, I>& it, float x, float y,
                 float z, unsigned int Col, float u, float v)
{
    it.vertex->Position_x = x;
    it.vertex->Position_y = y;
    it.vertex->Position_z = z;
    it.vertex->Color = Col;
    it.vertex->TexCoord_x = u;
    it.vertex->TexCoord_y = v;
}

// mem_heap (core_xboxr mem_lib; PakFile uses start/end/size/used_byte)
struct mem_heap {
    char*  start;      // +0x00
    char*  end;        // +0x04
    uint8_t _pad8[0x484 - 0x08];
    unsigned int size;      // +0x484
    unsigned int used_byte; // +0x488
    unsigned int high_used_byte;  // +0x48C
    mem_heap* reserve;      // +0x490
    int total_allocs;       // +0x494
    int total_frees;        // +0x498
};
static_assert(sizeof(mem_heap) == 0x49C, "mem_heap size mismatch");

template <typename T, int N>
struct ae_array {
    T   m_elements[N];  // +0x00
    int m_size;         // +N*sizeof(T)
};

template <typename T, int N>
T& ae_array_get(ae_array<T, N>& a, int idx)
{
    if (idx >= N)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
        AeAssert::gCurrentLine = 154;
        AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    return a.m_elements[idx];
}

// shell.o / render.o C-bridge stubs for MyRenderText
enum font_index : int;
extern font_index FEManager_FindFont(void* fe, const char* name, bool check);
extern nglFont* FEManager_GetFont(void* fe, font_index f);
extern nglFont* FEManager_GetFont(void* fe, font_index f, float scale);
extern nglFont* FEManager_GetFont(void* fe, int f, float scale);
extern unsigned int extract_color(const Color& col);
extern void* g_femanager_ptr;

// Cross-object stubs (shell.o / render.o)
unsigned int extract_color(const Color& col)
{
    unsigned int r = (unsigned int)(col.r * 255.0f);
    unsigned int g = (unsigned int)(col.g * 255.0f);
    unsigned int b = (unsigned int)(col.b * 255.0f);
    unsigned int a = (unsigned int)(col.a * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

extern int Cmd_Argc();       // core.o
extern char* Cmd_Argv(int arg);  // core.o
extern void Com_Printf(const char* fmt, ...);  // core.o
extern double atof(const char* nptr);
extern void tlPrintf(const char* fmt, ...);      // tl_system.o
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);  // tl_system.o
extern char* va(const char* fmt, ...);  // ?va@@YAPADPBDZZ (g_q_shared.cpp)
extern void tlMemFree(void* ptr);                // tl_system.o
extern "C" unsigned int AeHash(const char* str);     // ae_hash.cpp
struct cvar_t {
    char*  name;      // +0x00
    char*  string;    // +0x04
    char*  resetString;        // +0x08
    char*  latchedString;      // +0x0C
    int    flags;              // +0x10
    int    modified;           // +0x14
    int    modificationCount;  // +0x18
    float  value;              // +0x1C
    int    integer;            // +0x20
    cvar_t* next;              // +0x24
    cvar_t* hashNext;          // +0x28
};
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                        int flags);  // core.o cvar.cpp
extern void Cmd_AddCommand(const char* cmd_name, void (*function)());
                                                // core.o (common.cpp)
extern void Cmd_RemoveCommand(const char* cmd_name);  // core.o (common.cpp)
extern void DebugRender_AddRenderer(void* self, void* fp);  // core.o
extern void* DebugRender_sInst;  // ?sInst@DebugRender@@2V1@A @ 0xF74D20
unsigned char* stream_alloc(int size, bool aram);  // streamer.o
void stream_free(unsigned char* ptr, bool aram = false);  // streamer.o
void ConsoleSetPakDistance();   // streamer.o
void ConsoleClearPakDistance(); // streamer.o
void Console_ToggleSceneFX();    // streamer.o
void Console_ToggleRenderEnts(); // streamer.o
void Console_ToggleRenderLights(); // streamer.o
void TogglePakRender();         // streamer.o
void cdInitApk();               // streamer.o (apk support)
extern void Com_StripExtension(const char* in, char* out);  // g_q_shared.cpp
extern bool ShouldConnectPaths();                 // core.o sys.cpp
extern void mem_heap_create(mem_heap* heap, void* start, void* end,
                            mem_heap* reserve);  // mem_heap.cpp
extern int mem_heap_destroy(mem_heap* heap);     // mem_heap.cpp
extern bool mem_heap_free_check_reserve(mem_heap* heap, void* ptr);  // mem_heap.cpp
extern const char defaultFileName[];  // g_globals.cpp
namespace AeStringSupport {
void CStrToAeStr(char* dst, int* const dstLen, int capacity, const char* src);
void GetFileName(char* dst, int* const dstLen, const char* path, int pathLen,
                 bool includeExt);
void AeStrCopy(char* dst, int* const dstLen, int dstCapacity, const char* src,
               int srcLen);
}
enum nflState : unsigned {
    NFL_STATE_INVALID = 0xFFFFFFFFu,
    NFL_STATE_IDLE = 0,
    NFL_STATE_BUSY = 1,
    NFL_STATE_ERROR = 2,
};
enum nflFileID : unsigned { NFL_FILE_ID_INVALID = 0xFFFFFFFFu };
enum nflMediaID : unsigned {
    NFL_MEDIA_DEFAULT = 0,
    NFL_MEDIA_ID_DISC = 1,
    NFL_MEDIA_ID_HOST = 2,
    NFL_MEDIA_ID_LINK = 4,
};
enum nflRequestState : unsigned {
    NFL_REQUEST_STATE_INVALID = 0xFFFFFFFFu,
    NFL_REQUEST_STATE_COMPLETED = 0,
    NFL_REQUEST_STATE_CANCELED = 1,
    NFL_REQUEST_STATE_TIMEOUT = 2,
    NFL_REQUEST_STATE_ERROR = 3,
    NFL_REQUEST_STATE_ACTIVE = 4,
};
enum nflRequestID : unsigned { NFL_REQUEST_ID_INVALID = 0xFFFFFFFFu };
enum nflPriority : unsigned {
    NFL_PRIORITY_LOWEST = 0,
    NFL_PRIORITY_LOW = 1,
    NFL_PRIORITY_NORMAL = 2,
    NFL_PRIORITY_HIGH = 3,
    NFL_PRIORITY_HIGHEST = 4,
};
extern nflFileID nflOpenFile(nflMediaID media, const char* name);  // filesystem/nfl.cpp
extern void nflUpdate();
extern nflState nflGetState();
extern unsigned int nflReadFile(nflFileID file, unsigned offset, void* buf,
                                unsigned size);
extern void mem_break();  // mem_heap.cpp

// nfl async (nfl_common.o)
#define NFL_PRIORITY_LOWEST 0
typedef void (*nflReadCallback)(nflRequestState, nflRequestID, void*);
extern nflRequestID nflReadFileAsyncWithCallBack(
    nflFileID fileID, unsigned int fileOffset, void* buffer,
    unsigned int dataSize, nflReadCallback callback);
extern void nflSetRequestPriority(nflRequestID requestID,
                                  nflPriority priority);
extern nflFileID nflOpenFileEx(nflMediaID media, const char* fileName,
                               unsigned int* fileSize);  // nfl_common.o
extern void nflCancelFileRequests(nflFileID fileID);     // nfl_common.o
extern void nflCloseFile(nflFileID file);                // filesystem/nfl.cpp
extern unsigned int nflFileExists(nflMediaID media,
                                  const char* name);     // filesystem/nfl.cpp
struct nflRequestInfo {
    unsigned int bytesCompleted;
    unsigned int timeElapsed;
    unsigned int activeTimeElapsed;
};
extern nflRequestInfo* nflGetRequestInfo(nflRequestID requestID,
                                         nflRequestInfo* requestInfo);
void codNflCallback(nflRequestState state, nflRequestID requestId, void* userData);
void codNflCallback(nflRequestState state, nflRequestID requestId, void* userData)
{
    (void)state; (void)requestId; (void)userData;
}

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10, AC = 9 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
extern const char* gCurrentPakFile;      // ?gCurrentPakFile@AeAssert@@3PBDB (core.o)
extern const char* gCurrentPakResource;  // ?gCurrentPakResource@AeAssert@@3PBDB (core.o)
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Warning(const char* fmt, ...);
bool Error(const char* fmt, ...);
}
// core.o owns the real globals; defined here until core lands
const char* AeAssert::gCurrentPakFile = nullptr;
const char* AeAssert::gCurrentPakResource = nullptr;

enum TPakId { kPakTypeNone = -1 };
#define PAK_ID_INVALID ((TPakId)-1)
#define PAK_ID_MIN ((TPakId)0)
#define PAK_ID_MAX ((TPakId)99)
class PakFile;
void G_ParseInteractionInfo(TPakId pakId)
{
    (void)pakId;  // stub: game.o
}

// Handle (game_types.h; class for VHandle mangling)
class Handle {
public:
    unsigned int mVal;  // +0x00
};

// AbstractEffect (core_systems.h; vtable slot order must match: dtor,
// SetPoPtr, IsQueued, IsLooping, AdjustEffect_Scale, FastForward,
// PlayQueuedEffect, StartFadeOut, FrameAdvance, IsFinished, StopEffect,
// GetDebugString)
class AbstractEffect {
public:
    virtual ~AbstractEffect();
    virtual void SetPoPtr(math::Mat43* po);
    virtual bool IsQueued() const;
    virtual bool IsLooping() const;
    virtual void AdjustEffect_Scale(const char* param, float scale);
    virtual void FastForward(float deltaT);
    virtual void PlayQueuedEffect();
    virtual void StartFadeOut(float time);
    virtual void FrameAdvance(float deltaT);
    virtual bool IsFinished();
    virtual void StopEffect();
    virtual Broc::string GetDebugString() const;  // effect_events.cpp
};

class ActiveEffectSet {
public:
    struct EffectsArray {
        AbstractEffect* m_elements[6];
        int m_size;
    };
    EffectsArray mEffects;  // +0x00
    void SetPoPtr(math::Mat43* po);  // ?SetPoPtr@ActiveEffectSet@@QAEXPAVMat43@math@@@Z (effect_events.cpp)
};

class EffectEventSys {
public:
    static EffectEventSys* sInst;  // ?sInst@EffectEventSys@@2PAV1@A (g_cmd.cpp)
    void StopEffect(unsigned int handle, bool kill);  // game.o; stub
    ActiveEffectSet* GetActiveEffectSet(Handle handle);  // ?GetActiveEffectSet@EffectEventSys@@QAEPAVActiveEffectSet@@VHandle@@@Z (effect_events.cpp)
    void StopEffect(Handle handle, bool kill);  // ?StopEffect@EffectEventSys@@QAEXVHandle@@_N@Z (effect_events.cpp)
};
void EffectEventSys::StopEffect(unsigned int handle, bool kill)
{
    (void)handle; (void)kill;  // stub: game.o
}
// core.o 0x4D2340 (effect_events.cpp)
extern Handle PostEffectEventScriptCall(const Entity* ent, const char* scriptId,
                                        bool queue, TPakId pakid,
                                        const bool important);
extern float flrand(float min, float max);  // q_math.cpp ?flrand@@YAMMM@Z
extern void R_DestroyStaticModels(TPakId pakId);  // render.o
void R_DestroyStaticModels(TPakId pakId)
{
    (void)pakId;  // stub: render.o
}
extern void CM_DestroyStaticModels(TPakId pakId);  // render.o
typedef void (*PakDecoder)(const char* name, unsigned char* data,
                           unsigned int size, TPakId pakId, PakFile* pak);
PakDecoder GetDecoder(const char* ext);  // streamer.o (defined below)
void cdInvokeMultiApkCallbacks(const char* name, const char* file_ext,
                               TPakId pakId, apk::apkFile* File,
                               apk::apkFileEntry* Entry);  // streamer.o
// Decoders (DecodePakFile.cpp; defined below)
void DecodeAITypeBank(const char* name, unsigned char* data,
                      unsigned int size, TPakId pakId, PakFile* pak);
void DecodeGDB(const char* name, unsigned char* data, unsigned int size,
               TPakId pakId, PakFile* pak);
void DecodeBSP(const char* name, unsigned char* data, unsigned int size,
               TPakId pakId, PakFile* pak);
void DecodeInstanceBank(const char* name, unsigned char* data,
                        unsigned int size, TPakId pakId, PakFile* pak);
void DecodeCGBank(const char* name, unsigned char* data, unsigned int size,
                  TPakId pakId, PakFile* pak);
void DecodeDB(const char* name, unsigned char* data, unsigned int size,
              TPakId pakId, PakFile* pak);
void DecodeGrassInfo(const char* name, unsigned char* data,
                     unsigned int size, TPakId pakId, PakFile* pak);
void DecodeDestructible(const char* name, unsigned char* data,
                        unsigned int size, TPakId pakId, PakFile* pak);
void DecodeDCGBank(const char* name, unsigned char* data, unsigned int size,
                   TPakId pakId, PakFile* pak);
void DecodeDialogueBank(const char* name, unsigned char* data,
                        unsigned int size, TPakId pakId, PakFile* pak);
void DecodeFLI(const char* name, unsigned char* data, unsigned int size,
               TPakId pakId, PakFile* pak);
void DecodeSPT(const char* name, unsigned char* data, unsigned int size,
               TPakId pakId, PakFile* pak);
void DecodeScene(const char* name, unsigned char* data, unsigned int size,
                 TPakId pakId, PakFile* pak);
void DecodeWIND(const char* name, unsigned char* data, unsigned int size,
                TPakId pakId, PakFile* pak);
void DecodePhysData(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak);
void DecodeSplineGroup(const char* name, unsigned char* data,
                       unsigned int size, TPakId pakId, PakFile* pak);
void DecodeXModelBank(const char* name, unsigned char* data,
                      unsigned int size, TPakId pakId, PakFile* pak);
void DecodeSTB(const char* name, unsigned char* data, unsigned int size,
               TPakId pakId, PakFile* pak);
void DecodeBin(const char* name, unsigned char* data, unsigned int size,
               TPakId pakId, PakFile* pak);
void DecodeZoneBoundaryBank(const char* name, unsigned char* data,
                            unsigned int size, TPakId pakId, PakFile* pak);
void DecodeMipSettings(const char* name, unsigned char* data,
                       unsigned int size, TPakId pakId, PakFile* pak);
void DecodeLevelPath(const char* name, unsigned char* data,
                     unsigned int size, TPakId pakId, PakFile* pak);
void DecodeAnimBank(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak);
void DecodeHeap(const char* name, unsigned char* data, unsigned int size,
                TPakId pakId, PakFile* pak);
void DecodeSEED(const char* name, unsigned char* data, unsigned int size,
                TPakId pakId, PakFile* pak);
void DecodeZonePath(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak);
void DecodeTexture(const char* name, unsigned char* data, unsigned int size,
                   TPakId pakId, PakFile* pak);
void DecodeXModelPartsBank(const char* name, unsigned char* data,
                           unsigned int size, TPakId pakId, PakFile* pak);
void DecodeAnimMatrix(const char* name, unsigned char* data,
                      unsigned int size, TPakId pakId, PakFile* pak);
void DecodeFont(const char* name, unsigned char* data, unsigned int size,
                TPakId pakId, PakFile* pak);
void DecodeAnim(const char* name, unsigned char* data, unsigned int size,
                TPakId pakId, PakFile* pak);
void DecodeLightGrid(const char* name, unsigned char* data,
                     unsigned int size, TPakId pakId, PakFile* pak);
void DecodePanel(const char* name, unsigned char* data, int size,
                 TPakId pakId, PakFile* pak);
void DecodeSkeleton(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak);
void DecodeCharSkel(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak);
void DecodeConfigStrings(const char* name, unsigned char* data,
                         unsigned int size, TPakId pakId, PakFile* pak);
enum EPakType {
    kPakTypeGlobal = 0,
    kPakTypeFrontEnd = 1,
    kPakTypeAnimation = 2,
    kPakTypeLevel = 3,
    kPakTypeZone = 4,
    kPakTypeCommon = 5,
    kPakTypeVehicle = 6,
    kPakTypeCharacter = 7,
    kPakTypeWeapon = 8,
    kPakTypeUnknown = 9,
    kPakTypeCount = 10,
};
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
    float    ps2;              // +0x00
    Ps3Banks ps3;              // +0x04
    float    xbox;             // +0x0C
    float    xenon;            // +0x10
    float    pcx;              // +0x14
    GcBanks  gc;               // +0x18

    NumBanks() = default;
    NumBanks(float count, float aram);
    float& to_float();         // ?to_float@NumBanks@@QAEAAMXZ
    float to_float() const;    // ?to_float@NumBanks@@QBEMXZ
};

// ae/core/BitSet.h view (word-based; the core_systems.h template is byte-based)
template <int N>
struct BitSet {
    static const int kNumWords = (N + 31) / 32;
    unsigned int mBits[kNumWords];  // +0x00

    bool Test(int v) const;
    static int GetCapacity() { return N; }  // ?GetCapacity@?$BitSet@$0EA@@@SAHXZ (0x681B70)
    void Add(int v) { mBits[v >> 5] |= (1u << (v & 0x1F)); }
    void Rmv(int v) { mBits[v >> 5] &= ~(1u << (v & 0x1F)); }
    bool IsEmpty() const
    {
        for (int i = 0; i < kNumWords; ++i)
        {
            if (mBits[i] != 0)
                return false;
        }
        return true;
    }

    // BitSet<N>::iterator (core.o HandleDb.h; m_src +0, m_cur_word +4,
    // m_word_idx +8, m_cur_val +0xC)
    struct iterator {
        const BitSet<N>* m_src;       // +0x00
        unsigned int m_cur_word;      // +0x04
        int m_word_idx;               // +0x08
        int m_cur_val;                // +0x0C

        iterator() : m_src(nullptr), m_cur_word(0), m_word_idx(-1),
                     m_cur_val(-1) {}
        int operator*() const { return m_cur_val; }  // ea: 0x4ACFA0
        void operator++()  // ea: 0x4B1360
        {
            if (m_word_idx == -1)
                return;
            if (m_cur_word == 0)
            {
                while (m_word_idx < kNumWords - 1)
                {
                    ++m_word_idx;
                    m_cur_word = m_src->mBits[m_word_idx];
                    if (m_cur_word != 0)
                        break;
                }
            }
            if (m_cur_word != 0)
            {
                unsigned long v6;
                _BitScanForward(&v6, m_cur_word);
                m_cur_val = (int)v6 + 32 * m_word_idx;
                m_cur_word &= ~(1u << v6);
            }
            else
            {
                m_cur_val = -1;
                m_word_idx = -1;
            }
        }
    };
    iterator begin() const
    {
        iterator it;
        it.m_src = this;
        it.m_cur_word = mBits[0];
        it.m_word_idx = 0;
        it.m_cur_val = -1;
        return it;
    }
    BitSet<N> operator~() const
    {
        BitSet<N> r;
        for (int i = 0; i < kNumWords; ++i)
            r.mBits[i] = ~mBits[i];
        return r;
    }
};


struct mem_info;
template <typename T, int N> class ae_sized_array;

// TBankAlloc (BankManager.cpp; two 64-bit bank allocation bitmaps)
struct TBankAlloc {
    BitSet<64> mram_alloc1;  // +0x00
    BitSet<64> mram_alloc2;  // +0x08

    TBankAlloc() : mram_alloc1(), mram_alloc2() {}  // 0x5B6810

    bool IsEmpty() const;   // ?IsEmpty@TBankAlloc@@QBE_NXZ
    void Clear();           // ?Clear@TBankAlloc@@QAEXXZ
    float ToFloat() const;  // ?ToFloat@TBankAlloc@@QBEMXZ
};

// BankManager (BankManager.cpp; mNumMramBanks +0x10, mFreeBanks +0x00)
class BankManager {
public:
    TBankAlloc mFreeBanks;        // +0x00
    float      mNumMramBanks;     // +0x10
    unsigned int mMramBankSize;   // +0x14
    unsigned char* mMramArena;    // +0x18
    float      mLowestFreeAmount; // +0x1C
    TBankAlloc m_last_alloc;      // +0x20

    static BankManager* sInst;    // ?sInst@BankManager@@2PAV1@A @ 0xF592F4
    static void* operator new(size_t size, void* p);
    static BankManager* Inst();
    static void CreateInst();     // ?CreateInst@BankManager@@SAXXZ
    static void DeleteInst();     // ?DeleteInst@BankManager@@SAXXZ
    BankManager();                // ??0BankManager@@QAE@XZ
    ~BankManager();               // ??1BankManager@@AAE@XZ @ 0x66B1D0
    float GetNumBanks() const;    // ?GetNumBanks@BankManager@@QBEMXZ
    unsigned int get_bank_size() const;  // ?get_bank_size@BankManager@@QBEIXZ
    TBankAlloc get_free_banks() const;   // ?get_free_banks@BankManager@@QBE?AUTBankAlloc@@XZ
    unsigned char* GetMramArena();       // ?GetMramArena@BankManager@@QAEPAEXZ
    bool MemInBank(void* ptr) const;     // ?MemInBank@BankManager@@QBE_NPAX@Z
    float get_low_water_mark() const;    // ?get_low_water_mark@BankManager@@QBEMXZ
    NumBanks get_free_count() const;  // ?get_free_count@BankManager@@QBE?AVNumBanks@@XZ
    bool can_alloc(NumBanks num_banks) const;  // ?can_alloc@BankManager@@QBE_NVNumBanks@@@Z
    int get_alloc_count(const TBankAlloc& bat) const;  // ?get_alloc_count@BankManager@@QBEHABUTBankAlloc@@@Z
    void release(TBankAlloc& alloc);  // ?release@BankManager@@QAEXAAUTBankAlloc@@@Z
    TBankAlloc Allocate(NumBanks num_banks);  // ?Allocate@BankManager@@QAE?AUTBankAlloc@@VNumBanks@@@Z
    void GetAllocs(TBankAlloc bat, const NumBanks& numBanks,
                   ae_sized_array<mem_info, 32>* output);  // ?GetAllocs@BankManager@@QBEXUTBankAlloc@@ABVNumBanks@@AAV?$ae_sized_array@Vmem_info@@$0CA@@@@Z
    mem_info get_alloc(const TBankAlloc& bat, int which,
                       bool mram) const;  // ?get_alloc@BankManager@@QBE?AVmem_info@@ABUTBankAlloc@@H_N@Z
};

template <typename T, int N>
T& ae_array_get(ae_sized_array<T, N>& a, int idx)
{
    if (idx >= N)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
        AeAssert::gCurrentLine = 148;
        AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    return a.m_elements[idx];
}

// Cross-object bridge stubs (render.o / fx.o / anim.o / shell.o)
extern void R_RefreshCell(int cellIndex, bool clear);  // render.o
void R_RefreshCell(int cellIndex, bool clear)
{
    (void)cellIndex; (void)clear;
}
extern void FX_KillEffects(TPakId pakId);  // fx.o
void FX_KillEffects(TPakId pakId)
{
    (void)pakId;
}
extern void StopAllSceneAnims(TPakId pakId);  // anim.o
void StopAllSceneAnims(TPakId pakId)
{
    (void)pakId;
}

// AssetBankSet (streamer.o; sBankArray @ 0xF59348)
struct AssetBankSet {
    AssetBankSet();
    virtual ~AssetBankSet();  // defined in g_entity_misc.cpp
    virtual void UnloadBank(TPakId pakId);  // ?UnloadBank@AssetBankSet@@UAEXW4TPakId@@@Z (streamer.o)

    static ae_sized_array<AssetBankSet*, 24> sBankArray;  // ?sBankArray@AssetBankSet@@0V?$ae_sized_array@PAVAssetBankSet@@$0BI@@@A
    static void UnloadBanks(TPakId pakId);  // ?UnloadBanks@AssetBankSet@@SAXW4TPakId@@@Z (streamer.o 0x66B0E0)
};
ae_sized_array<AssetBankSet*, 24> AssetBankSet::sBankArray;

// ea: 0x006663A0
AssetBankSet::AssetBankSet()
{
    AssetBankSet::sBankArray.push_back(this);
}

// AssetBankSet dtor body (0x675850) - removes `self` from sBankArray.
// Shared by the AssetBankSet/SceneManager/StreamZoneManager dtor chain.
static void AssetBankSetRemoveSelf(void* self)
{
    int m_size = AssetBankSet::sBankArray.m_size;
    if (m_size != 0)
    {
        AssetBankSet** v2 = AssetBankSet::sBankArray.m_elements;
        AssetBankSet** v3 = &AssetBankSet::sBankArray.m_elements[m_size];
        while (v2 != v3)
        {
            if (*v2 == (AssetBankSet*)self)
                break;
            ++v2;
        }
        if (v2 != v3)
        {
            int v4 = (int)(v2 - AssetBankSet::sBankArray.m_elements);
            if (AssetBankSet::sBankArray.m_size > 1
                && v4 < AssetBankSet::sBankArray.m_size)
            {
                AssetBankSet::sBankArray.m_elements[v4] =
                    AssetBankSet::sBankArray
                        .m_elements[AssetBankSet::sBankArray.m_size - 1];
                m_size = AssetBankSet::sBankArray.m_size;
            }
            if (m_size)
                AssetBankSet::sBankArray.m_size = m_size - 1;
        }
    }
}

// ea: 0x675850
AssetBankSet::~AssetBankSet()
{
    AssetBankSetRemoveSelf(this);
}

// controller / MultiplayerMgr / nglDebug bridges (cross-object)

struct MultiplayerMgr;
MultiplayerMgr* MultiplayerMgr_sInst = nullptr;
extern bool gMPLoadingUnthreaded;
void MultiplayerMgr_Step(MultiplayerMgr* self, int earlyOutInterval,
                         bool fromThread, bool a_bFromGame)
{
    (void)self; (void)earlyOutInterval; (void)fromThread; (void)a_bFromGame;
}

struct NglDebugBridge {
    int DisableDuplicateMaterialWarning;
};
NglDebugBridge nglDebug;  // nglDebug.DisableDuplicateMaterialWarning

// ea: 0x66B0E0
void AssetBankSet::UnloadBanks(TPakId pakId)
{
    for (int i = 0; i < sBankArray.m_size; ++i)
        sBankArray.m_elements[i]->UnloadBank(pakId);
}

// stub: AssetBankSet::UnloadBank (overridden by derived managers)
void AssetBankSet::UnloadBank(TPakId pakId)
{
    (void)pakId;
}

// Void-return bridge used by the game-side constructors. The release
// constructor above owns the actual vtable and bank-array initialization.
void AssetBankSet_ctor(void* self)
{
    new (self) AssetBankSet();
}

// ThroughputMeasurer (streamer.o PakFile.cpp; 16 bytes, verified IDA)
struct ThroughputMeasurer {
    int   mBytes;       // +0x00
    int   mTotalBytes;  // +0x04
    float mStart;       // +0x08
    float mTotalTime;   // +0x0C

    float safeGetTime() const;  // ?safeGetTime@ThroughputMeasurer@@QBEMXZ
    void Update(int bytes);     // ?Update@ThroughputMeasurer@@QAEXH@Z
    void stop(int bytes);       // ?stop@ThroughputMeasurer@@QAEXH@Z
    void start();               // ?start@ThroughputMeasurer@@QAEXXZ (0x681180)
    void reset();               // ?reset@ThroughputMeasurer@@QAEXXZ (0x6810E0)
};
ThroughputMeasurer gThroughputMeasurer;  // ?gThroughputMeasurer@@3UThroughputMeasurer@@A @ 0xF59318
unsigned int g_bytes_read = 0;           // ?g_bytes_read@@3IA @ 0xF592E4
float g_throughput = 0.0f;               // ?g_throughput@@3MA @ 0xF592E0
int max_work = 20;                       // ?max_work@@3HA @ 0xDF91C0

// ea: 0x681110
float ThroughputMeasurer::safeGetTime() const
{
    return (float)((double)__rdtsc() * 0.0000013636364 * 0.001);
}

// ea: 0x684FB0
void ThroughputMeasurer::stop(int bytes)
{
    Update(bytes);
    float Time = safeGetTime();
    int v4 = bytes + mTotalBytes;
    mTotalTime = Time - mStart + mTotalTime;
    mStart = 0.0f;
    mTotalBytes = v4;
    mBytes = 0;
}

// ea: 0x6811A0
void ThroughputMeasurer::Update(int bytes)
{
    int v3 = bytes + mTotalBytes;
    mBytes = bytes;
    g_bytes_read = v3 / 1024;
    float elapsed = safeGetTime() - mStart;
    float v4 = mTotalTime + elapsed;
    if (v4 == 0.0f)
        g_throughput = 0.0f;
    else
        g_throughput = (float)g_bytes_read / v4;
}

// ea: 0x681180
void ThroughputMeasurer::start()
{
    mStart = safeGetTime();
    mBytes = 0;
}

// ea: 0x6810E0
void ThroughputMeasurer::reset()
{
    g_throughput = 0.0f;
    g_bytes_read = 0;
    mBytes = 0;
    mTotalBytes = 0;
    mStart = 0.0f;
    mTotalTime = 0.0f;
}

// FEManager (shell.o; DrawDiscError + UnloadBank stubs)
class FEManager {
public:
    font_index FindFont(const char* font_filename, bool checkfileext);
    nglFont* GetFont(font_index f);
    nglFont* GetFont(font_index f, float scale);
    void DrawDiscError();  // ?DrawDiscError@FEManager@@QAEXXZ (shell.o; stub)
    void UnloadBank(TPakId pakId);  // ?UnloadBank@FEManager@@QAEXW4TPakId@@@Z (shell.o; stub)
    void UpdateButtonFontForLanguage();  // ?UpdateButtonFontForLanguage@FEManager@@QAEXXZ
};
extern FEManager g_femanager;

// ea: 0x665470 callers use these C bridges; dispatch to the original
// FEManager methods rather than manufacturing a missing-font result.
font_index FEManager_FindFont(void* fe, const char* name, bool check)
{
    return static_cast<FEManager*>(fe)->FindFont(name, check);
}

nglFont* FEManager_GetFont(void* fe, font_index f, float scale)
{
    return static_cast<FEManager*>(fe)->GetFont(f, scale);
}

nglFont* FEManager_GetFont(void* fe, font_index f)
{
    return static_cast<FEManager*>(fe)->GetFont(f);
}

nglFont* FEManager_GetFont(void* fe, int f, float scale)
{
    return static_cast<FEManager*>(fe)->GetFont(
        static_cast<font_index>(f), scale);
}

void FEManager::UnloadBank(TPakId pakId)
{
    (void)pakId;
}

// PakHeader (streamer.o PakFile.h; 48 bytes)
struct PakHeader {
    struct File;
    struct Bank;

    struct Section {
        char      name[8];     // +0x00
        int       sectionId;   // +0x08
        uint16_t  numBanks;    // +0x0C
        uint16_t  numFiles;    // +0x0E
        Bank*     banks;       // +0x10 (Section::Bank*)
        File*     files;       // +0x14 (Section::File*)
    };

    struct File {
        const char* shortName;   // +0x00
        const char* longName;    // +0x04
        unsigned int fileSize;   // +0x08
        unsigned int bankIndex;  // +0x0C
        unsigned int bankOffset; // +0x10
    };

    struct Bank {
        unsigned int fileOffset;    // +0x00
        unsigned int size;          // +0x04
        unsigned int capacity;      // +0x08
        unsigned int flags;         // +0x0C
        unsigned int compressedSize; // +0x10
        int          requestId;     // +0x14
        unsigned char* memptr;      // +0x18
        unsigned int   memsize;     // +0x1C
    };

    unsigned int id;             // +0x00
    float        version;        // +0x04
    Section*     sections;       // +0x08
    unsigned int numSections;    // +0x0C
    unsigned int persistentSize; // +0x10
    unsigned int headerShortSize; // +0x14
    unsigned int headerLongSize; // +0x18
    char         packedBy[16];   // +0x1C
    unsigned int packedInfo;     // +0x2C

    void FixDown();  // ?FixDown@PakHeader@@QAEXXZ (streamer.o 0x666F40; stub)
    void FixUp(bool persistentFixup, int doByteSwap);  // ?FixUp@PakHeader@@QAEXXZ (streamer.o; stub)
};

// LoadStats (streamer.o PakFile.h; 32 bytes)
struct LoadStats {
    uint64_t totalStart;   // +0x00
    float    total;        // +0x08
    uint64_t readStart;    // +0x10
    float    readTotal;    // +0x18

    LoadStats();                       // ??0LoadStats@@QAE@XZ
    static float GetTime(unsigned __int64 start,
                         unsigned __int64 end);  // ?GetTime@LoadStats@@SAM_K0@Z
};

extern nflRequestState nflGetRequestState(nflRequestID requestID);

struct PakInfoNode;
class PoolAllocator;

// ae_vector<T> (ae/core/ae_vector.h; 12 bytes) - grow policy per IDA
template <typename T>
struct ae_vector {
    T*  mElements;   // +0x00
    int mCapacity;   // +0x04
    int mSize;       // +0x08

    void push_back(const T& iElement)
    {
        if (mSize >= mCapacity)
        {
            int v4 = mSize + 4;
            if (mSize <= 3)
                v4 = mSize + 1;
            T* v9 = (T*)tlMemAlloc(sizeof(T) * v4, 8, 0);
            for (int v5 = 0; v5 < mSize; ++v5)
                v9[v5] = mElements[v5];
            if (mElements != nullptr)
            {
                tlMemFree(mElements);
                mElements = nullptr;
                mCapacity = 0;
            }
            mCapacity = v4;
            mElements = v9;
        }
        mElements[mSize++] = iElement;
    }

    // ?resize@?$ae_vector@...@@@@QAEXH@Z (0x687470 GlowSprites / 0x687550 GlowBeam)
    void resize(int iNewSize)
    {
        if (iNewSize > mCapacity)
        {
            T* v8 = (T*)tlMemAlloc(sizeof(T) * iNewSize, 8u, 0);
            for (int v4 = 0; v4 < mSize; ++v4)
                v8[v4] = mElements[v4];
            if (mElements != nullptr)
            {
                tlMemFree(mElements);
                mElements = nullptr;
                mCapacity = 0;
            }
            mCapacity = iNewSize;
            mSize = iNewSize;
            mElements = v8;
        }
        else
        {
            mSize = iNewSize;
        }
    }

    // ?construct_array@?$ae_vector@...@@@@AAE...@@HH@Z (0x683E10)
    T* construct_array(int iCapacity, int iSize)
    {
        (void)iSize;
        return (T*)tlMemAlloc(sizeof(T) * iCapacity, 8u, 0);
    }

    // ?construct_array@?$ae_vector@...@@@@AAE...@@H@Z (0x684150)
    T* construct_array(int iNumber)
    {
        return (T*)tlMemAlloc(sizeof(T) * iNumber, 8u, 0);
    }

    // ?destroy_all@?$ae_vector@...@@@@AAEXXZ (0x683E40)
    void destroy_all()
    {
        if (mElements != nullptr)
        {
            tlMemFree(mElements);
            mElements = nullptr;
            mCapacity = 0;
        }
    }
};

// PakFile (streamer.o PakFile.h; 276 bytes, verified IDA)
class PakFile {
public:
    enum EState {
        LOADED = 0,
        LOADING = 1,
        UNLOADING = 2,
        UNLOADED = 3,
    };
    enum ELoadingState { LOADING_HEADER = 0, LOADING_DATA = 2 };
    enum EUnloadingState {
        UNLOADING_SERIALIZED = 4,
        UNLOADING_INPLACE = 5,
        UNLOADING_DONE = 6,
        LOADING_TOC = 7,
    };

    struct TRequestId {
        nflRequestID nflId;  // +0x00
    };

    struct {
        void* m_next;  // +0x00
        void* m_prev;  // +0x04
    } m_dlist_node;                    // +0x00 (reserved_dlist node)
    static int get_dlist_node_offset();  // ?get_dlist_node_offset@PakFile@@SAHXZ
    const char*  mCurrDecodeFile;       // +0x08
    struct {
        char           mBuff[63];  // ae_fixed_string<64,unsigned char>
        unsigned char  mLength;    // +0x4B
    } mPath;                            // +0x0C
    EPakType     mPakType;              // +0x4C
    unsigned int mFilesize;             // +0x50
    nflFileID    mFileId;               // +0x54
    TBankAlloc   mBankAlloc;            // +0x58
    TBankAlloc   mSerializedAlloc;      // +0x68
    TPakId       mPakId;                // +0x78
    const PakInfoNode* mPakInfo;        // +0x7C
    PakHeader*   mHeader;               // +0x80
    int          mDefaultSectionIdx;    // +0x84
    unsigned char* mHeaderBuffer;       // +0x88
    TRequestId   mHeaderRequestId;      // +0x8C
    ae_vector<apk::apkFile*> mApkFiles; // +0x90
    bool         mCloseHandle;          // +0x9C
    bool         mOnlyLoadHeader;       // +0x9D
    ae_sized_array<mem_heap*, 12> mHeapList;  // +0xA0
    ae_sized_array<PakFile*, 4>   mPrereqHeaps;  // +0xD4
    unsigned int mCurrentFile;          // +0xE8
    unsigned char* mCurrentFilePtr;     // +0xEC
    void*        mCurrentApk;           // +0xF0
    void*        mCurrentApkFileEntry;  // +0xF4
    void*        mCurrentApkFileTypeEntry;  // +0xF8
    EState       mState;                // +0xFC
    ELoadingState mLoadingState;        // +0x100
    enum ELoadingStateAll { LOADING_DONE = 3 };
    ae_vector<void*> mLooseFiles;       // +0x104
    LoadStats*   mLoadStats;            // +0x110

    static unsigned char* sHeaderBuffer;  // ?sHeaderBuffer@PakFile@@0PAEA
    static bool sHeaderBufferUsed;        // ?sHeaderBufferUsed@PakFile@@0_NA
    static unsigned int sHeaderBufferSize;  // ?sHeaderBufferSize@PakFile@@0IA
    static PoolAllocator* sAllocator;     // ?sAllocator@PakFile@@0PAVPoolAllocator@@A @ 0xF592EC

    void* get_dlist_node();     // ?get_dlist_node@PakFile@@QAEPAXXZ
    static void* operator new(size_t size, bool forceHeapAlloc,
                              const char* file, int line);  // ??2PakFile@@SAPAXI_NPBDH@Z
    static void operator delete(void* ptr, bool forceHeapAlloc,
                                const char* file, int line);  // ??3PakFile@@SAXPAX_NPBDH@Z
    static void operator delete(void* ptr);  // ??3PakFile@@SAXPAX@Z
    TPakId GetId() const;       // ?GetId@PakFile@@QBE?AW4TPakId@@XZ
    EPakType GetType() const;   // ?GetType@PakFile@@QBE?AW4EPakType@@XZ
    nflFileID GetFileId() const;
    const PakHeader* GetHeader() const;
    bool IsLoading() const;     // ?IsLoading@PakFile@@QBE_NXZ
    bool IsLoaded() const;      // ?IsLoaded@PakFile@@QBE_NXZ
    bool IsUnloading() const;   // ?IsUnloading@PakFile@@QBE_NXZ
    bool IsUnloaded() const;    // ?IsUnloaded@PakFile@@QBE_NXZ
    bool IsCancelled() const;   // ?IsCancelled@PakFile@@QBE_NXZ

    // ?MemAlloc@PakFile@@QAEPAXII_N@Z (streamer.o 0x671F10; stub)
    void* MemAlloc(unsigned int align, unsigned int size, bool search_prereqs);
    // ea: 0x666110
    bool MemFree(void* ptr, bool search_prereqs);
    // ea: 0x6661D0
    bool IsInPakHeap(void* pPtr);
    // ea: 0x666270
    const PakHeader::Section* GetSection(const char* type) const;
    // ea: 0x666320
    void* PakReadDataAsync(unsigned char* buffer,
                           unsigned char* uncompressedSize,
                           unsigned int compressedSize,
                           unsigned int offset, unsigned int isHeader);
    // ea: 0x665EC0
    void GetHeapUsage(int& used, int& size) const;  // ?GetHeapUsage@PakFile@@QBEXAAH0@Z
    // ea: 0x665F60
    mem_heap* CreateHeap(unsigned char* heap_start,
                         unsigned int heap_size);  // ?CreateHeap@PakFile@@AAEPAUmem_heap@@PAEI@Z
    // ea: 0x66AFF0
    void AddHeap(unsigned char* heap_start,
                 unsigned int heap_size);  // ?AddHeap@PakFile@@QAEXPAEI@Z
    // ea: 0x66E400
    static void SetupAllocator();  // ?SetupAllocator@PakFile@@SAXXZ
    // ea: 0x66A900
    void RenderDebug();  // ?RenderDebug@PakFile@@ABEXXZ
    // ea: 0x66E930
    void CreateHeaps();  // ?CreateHeaps@PakFile@@AAEXXZ
    // ea: 0x674810
    void InitiateDataLoad();  // ?InitiateDataLoad@PakFile@@AAEXXZ
    // ea: 0x66A120
    void InitiateHeaderRead();  // ?InitiateHeaderRead@PakFile@@AAEXXZ
    // ea: 0x66E540
    void CancelLoading();  // ?CancelLoading@PakFile@@AAEXXZ
    // ea: 0x6746C0
    void AsyncLoad(NumBanks numBanks);  // ?AsyncLoad@PakFile@@QAEXVNumBanks@@@Z
    // ea: 0x6747D0
    void AsyncUnload();  // ?AsyncUnload@PakFile@@QAEXXZ
    // ea: 0x66E7C0
    void BeginAsyncUnload();  // ?BeginAsyncUnload@PakFile@@AAEXXZ
    // ea: 0x66E860
    void UnloadSerialized();  // ?UnloadSerialized@PakFile@@AAEXXZ
    // ea: 0x66A380
    void UpdateReads();  // ?UpdateReads@PakFile@@AAEXXZ
    // ea: 0x66A790
    bool IsNextFileReady();  // ?IsNextFileReady@PakFile@@AAE_NXZ
    // ea: 0x66EE30
    void DestroyHeaps();  // ?DestroyHeaps@PakFile@@AAEXXZ
    // ea: 0x675580
    void UnloadInplace();  // ?UnloadInplace@PakFile@@AAEXXZ
    // ea: 0x675630
    void FinishUnload();  // ?FinishUnload@PakFile@@AAEXXZ
    // ea: 0x6775A0
    PakFile(EPakType pak_type, const char* path, TPakId id,
            NumBanks numBanks);  // ??0PakFile@@QAE@W4EPakType@@PBDW4TPakId@@VNumBanks@@@Z
    ~PakFile();  // ??1PakFile@@QAE@XZ @ 0x678FA0
    // ea: 0x67A380
    void UpdateLoading();  // ?UpdateLoading@PakFile@@AAEXXZ
    // ea: 0x67A150
    void Decode(const char* name, int size, TPakId pakId);  // ?Decode@PakFile@@AAEXPBDHW4TPakId@@@Z
    // ea: 0x67AD70
    void Update();  // ?Update@PakFile@@QAEXXZ
    // ea: 0x677810
    void SyncUnload();  // ?SyncUnload@PakFile@@AAEXXZ
    // ea: 0x677750
    void UpdateUnloading();  // ?UpdateUnloading@PakFile@@AAEXXZ
    // ea: 0x675410
    void AddApk(apk::apkFile* apk);  // ?AddApk@PakFile@@QAEXPAVapkFile@apk@@@Z
    // ea: 0x6754A0
    void ReleaseApks();  // ?ReleaseApks@PakFile@@QAEXXZ

    // ea: 0x664B90
    float GetLoadTime() const;
    // ea: 0x664BC0
    float GetProgress() const;
    // ea: 0x664C30
    bool IsCancelOk() const;
    // ea: 0x664C40
    void CopyHeader();
    // ea: 0x664CD0
    void DetermineNextFile();
    // ea: 0x664E50
    const PakInfoNode* GetInfo();
    // ea: 0x664E80
    void ValidateRange(void* data);
    // ea: 0x664EC0
    bool IsRequestValid(TRequestId* req) const;
    // ea: 0x664EE0
    void SetRequestInvalid(TRequestId* req) const;
    // ea: 0x664EF0
    bool IsRequestDone(TRequestId* requestId) const;
};

struct PakInfoNode;

struct PakInfoNode {
    EPakType    pakType;        // +0x00
    InplaceString longName;     // +0x04
    InplaceString path;         // +0x08
    NumBanks    numBanks;       // +0x0C
    struct {
        unsigned int mSize;          // +0x2C
        const PakInfoNode** mList;   // +0x30
    } prereqs;                       // +0x2C (InplaceVector)
    uint8_t     _pad34[0xB4 - 0x34];
    TPakId      pakId;          // +0xB4
    unsigned int refCount;      // +0xB8
    float       distance;       // +0xBC
    float       userDistance;   // +0xC0
    unsigned int mapColor;      // +0xC4
    float       computedDistance;  // +0xC8
    unsigned int visited;       // +0xCC
    BitSet<99>* prereqPakIds;   // +0xD0
    uint8_t     _padD4[0xE0 - 0xD4];
};

// InplaceVector<T> (ae/inplace/InplaceVector.h; full definition in
// game/game_types.h) - minimal view
template <typename T>
class InplaceVector {
public:
    unsigned int mSize;  // +0x00
    T*           mList;  // +0x04
};

// InplaceTree<T,K> (ae/inplace/InplaceTree.h; mSize +0x00, m_array +0x04)
// element = { mKey(T), mVal(K) }, 8 bytes; "used" = non-zero element.
template <typename T, typename K>
struct InplaceTree {
    struct Element {
        T mKey;  // +0x00
        K mVal;  // +0x04
    };
    unsigned int mSize;    // +0x00
    Element*     m_array;  // +0x04

    bool IsUsed(unsigned int index) const
    {
        if (index >= mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
            AeAssert::gCurrentLine = 211;
            AeAssert::gCurrentExpr = "index >= 0 && index < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        const unsigned char* p = (const unsigned char*)&m_array[index];
        return p[0] != 0 || p[1] != 0 || p[2] != 0 || p[3] != 0
               || p[4] != 0 || p[5] != 0 || p[6] != 0 || p[7] != 0;
    }
};

// InplaceTree<InplaceString,unsigned int>::Find<char const *> (0x4E50F0)
static unsigned int* InplaceTreeFindStringUInt(
    const InplaceTree<InplaceString, unsigned int>& tree, const char* key)
{
    if (tree.mSize == 0)
        return nullptr;
    unsigned int v3 = 0;
    while (1)
    {
        if (!tree.IsUsed(v3))
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
            AeAssert::gCurrentLine = 101;
            AeAssert::gCurrentExpr = "IsUsed(index)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("index must be used"))
                __debugbreak();
        }
        const InplaceString& mKey = tree.m_array[v3].mKey;
        if (_stricmp(mKey.mStr, key) == 0)
            break;
        v3 = _stricmp(mKey.mStr, key) >= 0 ? 2 * v3 + 1 : 2 * v3 + 2;
        if (v3 >= tree.mSize || !tree.IsUsed(v3) || v3 >= tree.mSize)
            return nullptr;
    }
    return &tree.m_array[v3].mVal;
}

// InplaceTree<unsigned int,unsigned int>::Find<unsigned int>
static unsigned int* InplaceTreeFindUInt(
    const InplaceTree<unsigned int, unsigned int>& tree,
    const unsigned int* key)
{
    if (tree.mSize == 0)
        return nullptr;
    unsigned int v3 = 0;
    while (1)
    {
        if (!tree.IsUsed(v3))
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
            AeAssert::gCurrentLine = 101;
            AeAssert::gCurrentExpr = "IsUsed(index)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("index must be used"))
                __debugbreak();
        }
        const unsigned int& mKey = tree.m_array[v3].mKey;
        if (mKey == *key)
            break;
        v3 = mKey >= *key ? 2 * v3 + 1 : 2 * v3 + 2;
        if (v3 >= tree.mSize || !tree.IsUsed(v3) || v3 >= tree.mSize)
            return nullptr;
    }
    return &tree.m_array[v3].mVal;
}

// InplaceTree<InplaceString,float>::Find<char const *>
static float* InplaceTreeFindFloat(
    const InplaceTree<InplaceString, float>& tree, const char* key)
{
    if (tree.mSize == 0)
        return nullptr;
    unsigned int v3 = 0;
    while (1)
    {
        if (!tree.IsUsed(v3))
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
            AeAssert::gCurrentLine = 101;
            AeAssert::gCurrentExpr = "IsUsed(index)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("index must be used"))
                __debugbreak();
        }
        const InplaceString& mKey = tree.m_array[v3].mKey;
        if (_stricmp(mKey.mStr, key) == 0)
            break;
        v3 = _stricmp(mKey.mStr, key) >= 0 ? 2 * v3 + 1 : 2 * v3 + 2;
        if (v3 >= tree.mSize || !tree.IsUsed(v3) || v3 >= tree.mSize)
            return nullptr;
    }
    return &tree.m_array[v3].mVal;
}

// PakInfoBank (streamer.o; InplaceAssetBank base, mPtrs +0x10)
class PakInfoBank {
public:
    uint8_t _pad[0x08];
    uint8_t mTree[8];        // +0x08 (InplaceTree<InplaceString,unsigned int>)
    InplaceVector<const PakInfoNode*> mPtrs;  // +0x10
    void*   mPtrFixupTable;  // +0x18
    char    sku[4];          // +0x1C

    void Fixup();  // InplaceAssetBank<PakInfoNode,...>::Fixup @ 0x685680
};

// InplaceTriple (ae/inplace; used by ZoneOverrideBrushSet::mNonZoneDistances)
template <typename A, typename B, typename C>
struct InplaceTriple {
    A a;  // +0x00
    B b;  // +0x04
    C c;  // +0x08
};

// mem_info (BankManager.cpp; 8 bytes)
struct mem_info {
    unsigned char* data;  // +0x00
    int            size;  // +0x04

    mem_info() : data(nullptr), size(0) {}
    mem_info(unsigned char* data_, int size_);  // ??0mem_info@@QAE@PAEH@Z
};

// reserved_dlist<T> (ae/core; intrusive node = T's first member) - verified IDA
template <typename T>
class reserved_dlist {
public:
    struct dlist_node {
        dlist_node* m_next;  // +0x00
        dlist_node* m_prev;  // +0x04

        // ?pop@dlist_node@?$reserved_dlist@VPakFile@@@@QAEXXZ (0x683DF0)
        void pop()
        {
            m_next->m_prev = m_prev;
            m_prev->m_next = m_next;
        }
    };

    int         m_size;  // +0x00
    dlist_node* m_head;  // +0x04
    dlist_node* m_end;   // +0x08
    dlist_node* m_tail;  // +0x0C

    // ?node_to_object@?$reserved_dlist@VPakFile@@@@SAPAVPakFile@@PAUdlist_node@1@@Z (0x683DC0)
    static T* node_to_object(dlist_node* dlist_node)
    {
        return (T*)dlist_node;
    }

    // ?get_head@?$reserved_dlist@VPakFile@@@@QAEPAUdlist_node@1@XZ (0x6840A0)
    dlist_node* get_head()
    {
        return m_head;
    }

    class iterator {
    public:
        dlist_node* m_node;  // +0x00
        dlist_node* m_next;  // +0x04

        // ??0iterator@...@QAE@PAUdlist_node@1@0@Z (0x683F70)
        iterator(dlist_node* cur, dlist_node* next)
            : m_node(cur), m_next(next) {}
        // ??0iterator@...@QAE@PAVT@@@Z (0x6840F0)
        iterator(T* obj)
            : m_node((dlist_node*)obj),
              m_next(((dlist_node*)obj)->m_next) {}
        // ??0iterator@...@QAE@AAV1@@Z (0x686660)
        iterator(reserved_dlist<T>* dlist)
        {
            m_node = dlist->m_head;
            m_next = (dlist->m_head != nullptr) ? dlist->m_head->m_next
                                                : nullptr;
            if (dlist->m_head == dlist->m_end)
            {
                m_next = nullptr;
                m_node = nullptr;
            }
        }

        // ??Diterator@...@QAEPAVT@@XZ (0x686210)
        T* operator*()
        {
            return (T*)m_node;
        }

        // ?compare@iterator@...@QBE_NABV12@@Z (0x683F90)
        bool compare(const iterator& rhs) const
        {
            return rhs.m_next == m_next;
        }
    };

    class const_iterator {
    public:
        dlist_node* m_node;  // +0x00
        dlist_node* m_next;  // +0x04

        // ??0const_iterator@?$reserved_dlist@VPakFile@@@@QAE@PBUdlist_node@1@0@Z (g.o 0x4AE560)
        const_iterator(const dlist_node* cur, const dlist_node* next);

        // ??0const_iterator@...@QAE@ABViterator@1@@Z (0x6837A0)
        const_iterator(const iterator& it)
            : m_node(it.m_node), m_next(it.m_next) {}
        // ??0const_iterator@?$reserved_dlist@VPakFile@@@@QAE@ABV1@@Z (g.o 0x4B1920)
        const_iterator(const const_iterator& rhs);

        // ??Dconst_iterator@...@QBEPBVT@@XZ (0x686200)
        const T* operator*() const
        {
            return (const T*)m_node;
        }
        // ??Cconst_iterator@?$reserved_dlist@VPakFile@@@@QBEPBVPakFile@@XZ (g.o 0x4B1320)
        const T* operator->() const
        {
            return (const T*)m_node;
        }
        // ??9const_iterator@?$reserved_dlist@VPakFile@@@@QBE_NABV01@@Z (g.o 0x4B1330)
        bool operator!=(const const_iterator& rhs) const
        {
            return m_next != rhs.m_next;
        }

        // ??Econst_iterator@?$reserved_dlist@VPakFile@@@@QAEAAV01@XZ (g.o 0x4ACF00)
        const_iterator& operator++();

        // ?compare@const_iterator@?$reserved_dlist@VPakFile@@@@QBE_NABV12@@Z (g.o 0x4AE580)
        bool compare(const const_iterator& rhs) const;
    };

    // ?node_to_object@?$reserved_dlist@VPakFile@@@@SAPBVPakFile@@PBUdlist_node@1@@Z (g.o 0x4AE480)
    static const T* node_to_object(const dlist_node* dlist_node);

    // ?get_head@?$reserved_dlist@VPakFile@@@@QBEPBUdlist_node@1@XZ (g.o 0x4AE7B0)
    const dlist_node* get_head() const { return m_head; }
    // ?empty@?$reserved_dlist@VPakFile@@@@QBE_NXZ (g.o 0x4AE7C0)
    bool empty() const { return m_head == m_end; }
    // ?validate@?$reserved_dlist@VPakFile@@@@QBEXXZ (g.o 0x4AE7E0)
    void validate() const;
    // ?end@?$reserved_dlist@VPakFile@@@@QBE?AVconst_iterator@1@XZ (g.o 0x4B11F0)
    const_iterator end() const;
    // ?begin@?$reserved_dlist@VPakFile@@@@QBE?AVconst_iterator@1@XZ (g.o 0x4B2760)
    const_iterator begin() const;

    // ?begin@?$reserved_dlist@VPakFile@@@@QAE?AViterator@1@XZ (0x686E60)
    iterator begin()
    {
        iterator result(nullptr, nullptr);
        result.m_node = m_head;
        result.m_next = (m_head != nullptr) ? m_head->m_next : nullptr;
        if (m_head == m_end)
        {
            result.m_next = nullptr;
            result.m_node = nullptr;
        }
        return result;
    }
};

// g.o explicit specializations (0x4ACF00 / 0x4AE480)
template <>
reserved_dlist<PakFile>::const_iterator&
reserved_dlist<PakFile>::const_iterator::operator++()
{
    if (m_next == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/reserved_dlist.h";
        AeAssert::gCurrentLine = 501;
        AeAssert::gCurrentExpr = "m_next != 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Please add a descriptive string"))
            __debugbreak();
    }
    m_node = m_next;
    m_next = m_next->m_next;
    return *this;
}

template <>
const PakFile* reserved_dlist<PakFile>::node_to_object(
    const reserved_dlist<PakFile>::dlist_node* dlist_node)
{
    return (const PakFile*)dlist_node;
}

template <>
reserved_dlist<PakFile>::const_iterator::const_iterator(
    const reserved_dlist<PakFile>::dlist_node* cur,
    const reserved_dlist<PakFile>::dlist_node* next)
    : m_node((dlist_node*)cur), m_next((dlist_node*)next)
{
}

template <>
bool reserved_dlist<PakFile>::const_iterator::compare(
    const reserved_dlist<PakFile>::const_iterator& rhs) const
{
    return rhs.m_next == m_next;
}

template <>
reserved_dlist<PakFile>::const_iterator::const_iterator(
    const reserved_dlist<PakFile>::const_iterator& rhs)
    : m_node(rhs.m_node), m_next(rhs.m_next)
{
}

template <>
void reserved_dlist<PakFile>::validate() const
{
}

template <>
reserved_dlist<PakFile>::const_iterator reserved_dlist<PakFile>::end() const
{
    const_iterator result((const dlist_node*)m_end, (const dlist_node*)nullptr);
    result.m_node = m_end;
    result.m_next = nullptr;
    return result;
}

template <>
reserved_dlist<PakFile>::const_iterator reserved_dlist<PakFile>::begin() const
{
    const_iterator result((const dlist_node*)m_head,
                          (const dlist_node*)((m_head != nullptr) ? m_head->m_next : nullptr));
    result.m_node = m_head;
    result.m_next = (m_head != nullptr) ? m_head->m_next : nullptr;
    if (m_head == m_end)
    {
        result.m_next = nullptr;
        result.m_node = nullptr;
    }
    return result;
}

// Force emission of remaining inline members (get_head/empty/validate/operator->/!=)
template class reserved_dlist<PakFile>;

class PakManager {
public:
    enum state_e {
        STATE_UNLOADED = 0,
        STATE_LOADING = 1,
        STATE_LOADED = 2,
        STATE_UNLOADING = 3,
    };
    static void CreateInst();  // ?CreateInst@PakManager@@SAXXZ (core.o)
    static void DeleteInst();  // ?DeleteInst@PakManager@@SAXXZ (core.o)
    static void* operator new(size_t size, void* p);
    struct TThreadedPakContextStack {
        uint32_t key;            // +0x00 thread id
        TPakId stack[128];       // +0x04
        int m_size;              // +0x204
    };

    struct {
        unsigned int bankUsage;    // +0x00 (bit 0 toggled by TogglePakRender)
        cvar_t*      progressbar;  // +0x04
        cvar_t*      paks;         // +0x08
        cvar_t*      heaps;        // +0x0C
    } mDebugRenderMode;            // +0x00
    int mState;                       // +0x10 PakManager::state_e
    bool mFilled;                     // +0x14
    bool mFillingBanks;               // +0x15
    PakInfoBank* mPakInfoBank;        // +0x18
    PakInfoBank* mLevelPakInfoBank;   // +0x1C
    TPakId mPakIdServer;              // +0x20
    TPakId mCurrentPakId;             // +0x24
    const PakInfoNode* mCurSection;   // +0x28
    TPakId mAnimPakId;                // +0x2C
    TPakId mGlobalPakId;              // +0x30
    TPakId mCurVehiclePakId;          // +0x34
    TPakId mLevelPakId;               // +0x38
    TPakId mDebugPakId;               // +0x3C
    PakFile* mSlots[99];              // +0x40
    const PakInfoNode* mPakInfoPtrs[99];  // +0x1CC
    BitSet<32> mSectionStates;        // +0x358
    PakInfoNode* mCurrentPakInfo;     // +0x35C
    const PakInfoNode* mNextPakInfo;  // +0x360
    void (*mProgressCallback)(float); // +0x364
    char fli_buffer[0x4000];          // +0x368
    bool mRequestedStopLoading;       // +0x4368
    struct {                          // ae_vector<ae_pair<tlFixedString,bool>>
        void* mElements;              // +0x436C
        int   mCapacity;              // +0x4370
        int   mSize;                  // +0x4374
    } mAudioBanks;                    // +0x436C
    struct {                          // ae_vector<PakManager::TStbData>
        void* mElements;              // +0x4378
        int   mCapacity;              // +0x437C
        int   mSize;                  // +0x4380
    } mStbVector;                     // +0x4378
    int mEnabled;                     // +0x4384
    reserved_dlist<PakFile> mActivePaks;  // +0x4388
    TThreadedPakContextStack mContextStack[1];  // +0x4398
    int mNumZonesLoaded;              // +0x45A0

    static PakManager* sInst;          // ?sInst@PakManager@@2PAV1@A (sv_globals.cpp)
    static PakManager* Inst();         // ?Inst@PakManager@@SAPAV1@XZ
    PakFile* GetPakFile(TPakId id);    // ?GetPakFile@PakManager@@QAEPAVPakFile@@W4TPakId@@@Z
    void ToggleEnabled();              // ?ToggleEnabled@PakManager@@QAEXXZ
    const reserved_dlist<PakFile>& GetActivePaks() const;  // ?GetActivePaks@PakManager@@QBEABV?$reserved_dlist@VPakFile@@@@XZ
    static unsigned int sComputeDistanceKey;  // ?sComputeDistanceKey@PakManager@@0IA
    static float sBrocPercentage;      // ?sBrocPercentage@PakManager@@0MA
    static float sWbkPercentage;       // ?sWbkPercentage@PakManager@@0MA

    PakManager();               // ??0PakManager@@QAE@XZ @ 0x6791F0
    ~PakManager();              // ??1PakManager@@QAE@XZ @ 0x66FB80
    // - ea: 0x66C620
    void Reset();
    // - ea: 0x670480 (large; stub)
    void DebugRender();
    // - ea: 0x687700
    static void SingletonDebugRender();

    // - ea: 0x666C50
    const ae_sized_array<TPakId, 128>& GetContextStack() const;
    // - ea: 0x666D00
    void PushContext(TPakId pakId);
    // - ea: 0x66CB90
    TPakId PopContext();
    // - ea: 0x66CB60
    TPakId GetTopContext() const;
    // - ea: 0x665420
    void SetUserDistance(const PakInfoNode* cpak, float dist);
    // - ea: 0x665450
    void ClearUserDistance(const PakInfoNode* cpak);
    // - ea: 0x4A5050 (g.o inline)
    void SetProgressCallback(void (*cb)(float));
    // - ea: 0x4B45E0 (core.o inline)
    TPakId GetGlobalPakId() const;
    TPakId GetCurrentPakId() const;
    // - ea: 0x6657A0
    bool IsLoaded(TPakId id) const;
    // - ea: 0x665830
    bool IsUnloading(TPakId id) const;
    // - ea: 0x665710
    void SetSoundProgress(float t);
    // - ea: 0x665760
    void SetBrocProgress(float t);
    // - ea: 0x665570
    float GetDistance(PakInfoNode* node) const;
    // - ea: 0x666950
    void SetDistance(const PakInfoNode* cpak, float dist, bool force);
    // - ea: 0x6669E0
    PakInfoNode* GetUnloadedPrereq(const PakInfoNode* pak) const;
    // - ea: 0x66C8B0
    PakInfoNode* GetBestUnloadedPak();
    // - ea: 0x6635E0
    bool IsFillingBanks() const;
    // - ea: 0x6635F0
    TPakId GetAnimPakId() const;
    // - ea: 0x663600
    bool IsValid(TPakId id) const;
    // - ea: 0x663630
    TPakId GetLevelPakId() const;
    // - ea: 0x663640
    PakInfoNode* UnConst(const PakInfoNode* pak) const;
    // - ea: 0x6656B0
    NumBanks GetNumBanks(const PakInfoNode* pdt) const;
    // - ea: 0x6657D0
    bool IsUnloaded(TPakId id) const;
    // - ea: 0x665800
    bool IsLoading(TPakId id) const;
    // - ea: 0x665850
    void LoadWbk(tlFixedString audioBank, bool async);
    // - ea: 0x671900
    const char* GetPakName(TPakId id) const;
    // - ea: 0x6719E0
    bool IsLoaded(const char* long_name) const;
    // - ea: 0x671A70
    void CopyContextStack(ae_sized_array<TPakId, 32>* ret) const;
    // - ea: 0x66FC30
    void GetPakPrerequisites(TPakId pakId, ae_sized_array<TPakId, 32>* ret,
                             BitSet<99>* seen) const;
    // - ea: 0x666A40
    void RegisterPakLoaded(PakInfoNode* pak);
    // - ea: 0x666B90
    void RegisterPakUnloaded(PakInfoNode* pak);
    // - ea: 0x6655C0 (stub until PakFile/BankManager land)
    TPakId FindPakId(EPakType t) const;
    // - ea: 0x671CC0
    TPakId FindPakId(const char* pak_name) const;
    // - ea: 0x671D60
    void GetActivePakIds(ae_vector<TPakId>* id_set) const;
    // - ea: 0x671DF0
    int GetUnloadableBankCount(bool mram) const;
    // - ea: 0x671ED0 (stub: real impl walks mActivePaks and calls
    // PakFile::MemAlloc; the active-pak list is not ported yet)
    void* CrazyTempMemBorrow(unsigned int align, unsigned int size);
    // - ea: 0x671F30 (stub)
    void  CrazyTempMemGiveBack(void* ptr);
    // - ea: 0x665B70 (stub)
    TPakId SyncLoadPak(EPakType t, const char* path, NumBanks banks);
    // - ea: 0x665D10 (stub)
    TPakId SyncLoadPak(const PakInfoNode* cpak);
    // - ea: 0x665480 (stub until PakFile/BankManager land)
    void* MemAlloc(TPakId id, unsigned int size, bool bUseActorHeap);
    // - ea: 0x6654D0 (stub)
    void MemFree(TPakId id, void* ptr, bool bUseActorHeap);
    // - ea: 0x665520 (stub)
    void* MemAlign(TPakId id, unsigned int align, unsigned int size);
    // - ea: 0x6656E0 (stub)
    void FillBanks();
    // - ea: 0x6656C0 (stub)
    void UnloadAll();
    // - ea: 0x665760 (stub)
    void ResetPriorities(bool user_distances_also);
    // - ea: 0x665670 (stub)
    void SyncUnloadPak(TPakId id);
    // - ea: 0x665B90 (stub)
    const PakInfoNode* SyncLoadFLI(EPakType t, const char* path);
    // - ea: 0x677F40
    PakFile* AsyncLoadPakHeader(EPakType pak_type, const char* path,
                                NumBanks num_banks);
    // - ea: 0x665900 (stub)
    const PakInfoNode* GetPakInfo(TPakId pakId) const;
    // - ea: 0x66C660
    const PakInfoNode* GetPakInfo(const char* long_name) const;
    // - ea: 0x675980
    void DecodeFLI(const char* name, PakInfoBank* data, int size,
                   TPakId pakId);  // ?DecodeFLI@PakManager@@QAEXPBDPAEHW4TPakId@@@Z
    // - ea: 0x66FE10
    void UpdateInfo();  // ?UpdateInfo@PakManager@@AAEXXZ
    // - ea: 0x671360
    PakInfoNode* GetBestUnloadablePak();  // ?GetBestUnloadablePak@PakManager@@ABEPAUPakInfoNode@@XZ
    // - ea: 0x665410
    void ClearFileLocationInfo();  // ?ClearFileLocationInfo@PakManager@@QAEXXZ
    // - ea: 0x67B060
    void Update(bool calledFromMovie);  // ?Update@PakManager@@QAEX_N@Z
    // - ea: 0x671600
    void ProgressUpdate();  // ?ProgressUpdate@PakManager@@AAEXXZ
    // - ea: 0x671480
    void UpdateLoading();  // ?UpdateLoading@PakManager@@AAEXXZ
    // - ea: 0x679390
    void UpdateUnloading();  // ?UpdateUnloading@PakManager@@AAEXXZ
    // - ea: 0x678040
    void UpdateNormal();  // ?UpdateNormal@PakManager@@AAEXXZ
    // - ea: 0x677C90
    TPakId AsyncLoadPak(EPakType pak_type, const char* path,
                        NumBanks num_banks);  // ?AsyncLoadPak@PakManager@@QAE?AW4TPakId@@W4EPakType@@PBDVNumBanks@@@Z
    // - ea: 0x670370
    void AsyncUnloadPak(TPakId id);  // ?AsyncUnloadPak@PakManager@@QAEXW4TPakId@@@Z
};

// ea: 0x4B44D0
void* PakManager::operator new(size_t /*size*/, void* p)
{
    return p;
}

extern PoolAllocator* gPakMemHeapAllocator;  // ?gPakMemHeapAllocator@@3PAVPoolAllocator@@A @ 0xF592F0
PoolAllocator* PakFile::sAllocator = nullptr;  // ?sAllocator@PakFile@@0PAVPoolAllocator@@A @ 0xF592EC

// TlSystemCallbacks (core_systems.h; LockTlAllocsToPakHeap only)
class TlSystemCallbacks {
public:
    static bool LockTlAllocsToPakHeap(bool s, bool once);  // ?LockTlAllocsToPakHeap@TlSystemCallbacks@@SA_N_N0@Z (sys.cpp)
};

// PakHeapContext (PakFile.cpp; mPakId +0, mLastState +4)
class PakHeapContext {
public:
    TPakId mPakId;      // +0x00
    bool   mLastState;  // +0x04

    PakHeapContext(TPakId id, bool once);  // ??0PakHeapContext@@QAE@W4TPakId@@_N@Z
    ~PakHeapContext();                     // ??1PakHeapContext@@QAE@XZ
};

// GlowSprites / GlowBeam (streamer.o render lists; verified IDA)
struct GlowSprites {
    float pos[3];          // +0x00
    float rad;             // +0x0C
    unsigned int color;    // +0x10
};
struct GlowBeam {
    float pos[3];          // +0x00
    float dir[3];          // +0x0C
    float rad;             // +0x18
    unsigned int color;    // +0x1C
};

ae_vector<GlowSprites> GlowSpritesList;  // ?GlowSpritesList@@3V?$ae_vector@UGlowSprites@@@@A @ 0xF59328
ae_vector<GlowBeam> GlowBeamsList;       // ?GlowBeamsList@@3V?$ae_vector@UGlowBeam@@@@A @ 0xF59334
nglTexture* gGlowTexture = nullptr;      // ?gGlowTexture@@3PAUnglTexture@@A @ 0x13487C8

enum eInstanceBankType {
    INSTBANK_TYPE_TEXTURE = 0,
    INSTBANK_TYPE_FONT,
    INSTBANK_TYPE_MESHFILE,
    INSTBANK_TYPE_MESH,
    INSTBANK_TYPE_ANIMFILE,
    INSTBANK_TYPE_ANIM,
    INSTBANK_TYPE_SCNANIM,
    INSTBANK_TYPE_ANIMOFFSET,
    INSTBANK_TYPE_SKELETON,
    INSTBANK_TYPE_EFFECT,
    INSTBANK_TYPE_FX,
    INSTBANK_TYPE_DISCTEX,
    INSTBANK_TYPE_DISCTEXSIZE,
};
// Values verified against DecodeInstbank's type-string table (TEXTURE=0 ..
// DISCTEXSIZE=12) and the raw pushes at the cdGet*/cdLoad* call sites. The
// earlier "APK=0, TEXTURE=1, ..." layout was shifted by one.

class InstanceBankSet;

// InstanceBank (streamer.o InstanceBank.h; 32 bytes, verified IDA)
struct InstanceBank {
    struct IbEntry {
        InplaceString name;      // +0x00
        unsigned int  ptr;       // +0x04
    };

    int      mType;       // +0x00
    char     mTypeStr[12]; // +0x04
    uint8_t  mTree[8];    // +0x10 (InplaceTree<unsigned int,unsigned int>; stub)
    InplaceVector<IbEntry> mEntries;  // +0x18

    int strnicmp(const char* str1, const char* str2, int len) const;  // ?strnicmp@InstanceBank@@QBEHPBD0H@Z
    eInstanceBankType GetType() const;  // ?GetType@InstanceBank@@QBE?AW4eInstanceBankType@@XZ
    const char* GetTypeStr() const;     // ?GetTypeStr@InstanceBank@@QBEPBDXZ
    int Find(const char* str, unsigned int hash) const;  // ?Find@InstanceBank@@QBEHPBDI@Z
    unsigned int Size() const;          // ?Size@InstanceBank@@QBEIXZ
    void Enumerate(void (*Callback)(const char*, eInstanceBankType, void*,
                                    void*),
                   void* userdata);  // ?Enumerate@InstanceBank@@QAEXP6AXPBDW4eInstanceBankType@@PAX2@Z2@Z
};

// InstanceBankSet (streamer.o InstanceBank.h; 20 bytes, verified IDA)
struct InstanceBankSet {
    unsigned int mId;       // +0x00
    float        mVersion;  // +0x04
    InplaceVector<InstanceBank> mInstanceBanks;  // +0x08
    void*        mPtrFixupTable;  // +0x10

    void Fixup();  // ?Fixup@InstanceBankSet@@QAEXXZ (stub)
    InstanceBank& GetBank(eInstanceBankType type);  // ?GetBank@InstanceBankSet@@QAEAAVInstanceBank@@W4eInstanceBankType@@@Z
    unsigned int* FindEntry(eInstanceBankType type, const char* str,
                            unsigned int hash);  // ?FindEntry@InstanceBankSet@@QAEPAIW4eInstanceBankType@@PBDI@Z
    void Enumerate(void (*Callback)(const char*, eInstanceBankType, void*,
                                    void*),
                   void* userdata);  // ?Enumerate@InstanceBankSet@@QAEXP6AXPBDW4eInstanceBankType@@PAX2@Z2@Z
};

// nal resource types (nal.h; forward decls)
class nalAnyPose;
struct nalAnimFile;  // struct to match nal.cpp externs (PAU mangling)
class nalSceneAnim;
class nalBaseSkeleton;
template <typename T> class nalAnimClass;

// cdResourceDirectory<T> (streamer.o; verified IDA: vftable +0x00,
// m_old_directory +0x04, m_enable_release +0x08, m_enable_add +0x09,
// m_release_once +0x0A). Subclass of tlResourceDirectory<T> so the
// vftable slot order matches (Find slot 2, Add slot 3).
template <typename T>
struct cdResourceDirectory : tlResourceDirectory<T> {
    tlResourceDirectory<T>* m_old_directory;  // +0x04
    bool m_enable_release;    // +0x08
    bool m_enable_add;        // +0x09
    bool m_release_once;      // +0x0A

    // ?Add@?$cdResourceDirectory@V...@@@UAEPAV...@@PAV...@@Z (streamer.o)
    virtual T* Add(T* DataPtr) override;
    // ?Find@?$cdResourceDirectory@V...@@@UAEPAV...@@ABVtlFixedString@@@Z
    virtual T* Find(const tlFixedString& key) override;

    void EnableRelease(bool enabled);  // ?EnableRelease@?$cdResourceDirectory@V...@@@QAEX_N@Z

    static tlResourceDirectory<T>* GetDirectory();   // ?GetDirectory@?$cdResourceDirectory@V...@@@SAPAV...@@XZ
    static void SetDirectory(tlResourceDirectory<T>* dir);  // ?SetDirectory@?$cdResourceDirectory@V...@@@SAXPAV...@@@Z

    virtual const char* DirectoryName() override;      // ?DirectoryName@?$cdResourceDirectory@V...@@@UAEPBDXZ
    virtual bool Del(T* const DataPtr) override;       // ?Del@?$cdResourceDirectory@V...@@@UAE_NQAV...@@@Z
    virtual void ReleaseAll(bool warn, bool system, int maxforce) override;  // ?ReleaseAll@?$cdResourceDirectory@V...@@@UAEX_N0H@Z
};

// MipSettingsBank (streamer.o; mMipSettings +0x0C)
struct MipSettingsBank {
    unsigned int mId;          // +0x00
    float        mVersion;     // +0x04
    TPakId       mPakId;       // +0x08
    uint8_t      mMipSettings[8];  // +0x0C (InplaceTree<InplaceString,float>; stub)
    void*        mPtrFixupTable;   // +0x14

    void Fixup();  // ?Fixup@MipSettingsBank@@QAEXXZ
};

// InstanceBankMgr (streamer.o; mEntries[99] @ +0x34)
class InstanceBankMgr {
public:
    static void* operator new(unsigned int s, void* p); // core.o 0x004DC4A0
    cdResourceDirectory<nalAnimClass<nalAnyPose>> m_anim_directory;  // +0x00
    cdResourceDirectory<nalSceneAnim>             m_scnanim_directory;   // +0x0C
    cdResourceDirectory<nalAnimFile>              m_animfile_directory;  // +0x18
    cdResourceDirectory<nalBaseSkeleton>          m_skeleton_directory;  // +0x24
    MipSettingsBank* mMipSettingsBank;  // +0x30
    InstanceBankSet* mEntries[99];  // +0x34
    static InstanceBankMgr* sInst;  // ?sInst@InstanceBankMgr@@2PAV1@A
    static void CreateInst();  // ?CreateInst@InstanceBankMgr@@SAXXZ
    static void DeleteInst();  // ?DeleteInst@InstanceBankMgr@@SAXXZ

    void ReleaseInstanceBank(TPakId pakId);  // ?ReleaseInstanceBank@InstanceBankMgr@@QAEXW4TPakId@@@Z
    void DecodeInstbank(const char* name, unsigned char* data, int size,
                        TPakId pakId);  // ?DecodeInstbank@InstanceBankMgr@@QAEXPBDPAEHW4TPakId@@@Z
    void ReleaseSkeletons(TPakId pakId);  // ?ReleaseSkeletons@InstanceBankMgr@@QAEXW4TPakId@@@Z
    void ReleaseAnims(TPakId pakId);  // ?ReleaseAnims@InstanceBankMgr@@QAEXW4TPakId@@@Z
    void DecodeMipsettings(const char* name, unsigned char* data, int size,
                           TPakId pakId);  // ?DecodeMipsettings@InstanceBankMgr@@QAEXPBDPAEHW4TPakId@@@Z
    void RegisterAnimOffset(const char* name, unsigned int offset,
                            unsigned int size,
                            TPakId pakId);  // ?RegisterAnimOffset@InstanceBankMgr@@QAEXPBDIIW4TPakId@@@Z
    void Enumerate(TPakId pakId, void (*Callback)(const char*,
                                                  eInstanceBankType, void*,
                                                  void*),
                   void* userdata);  // ?Enumerate@InstanceBankMgr@@QAEXW4TPakId@@P6AXPBDW4eInstanceBankType@@PAX2@Z2@Z
    unsigned int Add(eInstanceBankType type, TPakId pakId,
                     const tlFixedString& name,
                     unsigned int data);  // ?Add@InstanceBankMgr@@QAEIW4eInstanceBankType@@W4TPakId@@ABVtlFixedString@@I@Z
    unsigned int Get(eInstanceBankType type, TPakId pakId,
                     const tlFixedString* name);  // ?Get@InstanceBankMgr@@QBEIW4eInstanceBankType@@W4TPakId@@ABVtlFixedString@@@Z
    unsigned int Get(eInstanceBankType type, TPakId pakId,
                     unsigned int hash);  // ?Get@InstanceBankMgr@@QBEIW4eInstanceBankType@@W4TPakId@@I@Z
    InstanceBankMgr();   // ??0InstanceBankMgr@@QAE@XZ @ 0x66BDE0
    ~InstanceBankMgr();  // ??1InstanceBankMgr@@QAE@XZ @ 0x66BE80
    bool GetAnimOffset(const char* name, TPakId pakId, unsigned int* out_offset,
                       unsigned int* out_size);  // ?GetAnimOffset@InstanceBankMgr@@QAE_NPBDW4TPakId@@PAI2@Z
    bool GetMipScale(const char* texture,
                     float* out_value);  // ?GetMipScale@InstanceBankMgr@@QAE_NPBDAAM@Z
};
InstanceBankMgr* InstanceBankMgr::sInst = nullptr;

void* InstanceBankMgr::operator new(unsigned int, void* p)
{
    return p;
}

// ea: 0x004DC4B0 (core.o inline)
void InstanceBankMgr::CreateInst()
{
    if (sInst != nullptr
        && _tlAssert("c:\\cod\\code\\game\\InstanceBankMgr.h", 144,
                     "sInst==0", "singleton already created!"))
        __debugbreak();

    void* memory = mem_heap_malloc_ctx(
        0x1C0u, 4, "pak", "c:\\cod\\code\\game\\InstanceBankMgr.h", 144);
    if (memory != nullptr)
        sInst = new (memory) InstanceBankMgr();
    else
        sInst = nullptr;
}

// ea: 0x004E2630 (core.o inline)
void InstanceBankMgr::DeleteInst()
{
    if (sInst == nullptr
        && _tlAssert("c:\\cod\\code\\game\\InstanceBankMgr.h", 144,
                     "sInst!=0", "singleton not created!"))
        __debugbreak();
    if (sInst != nullptr)
    {
        sInst->~InstanceBankMgr();
        mem_heap_free(sInst);
    }
    sInst = nullptr;
}

// per-instantiation bank type / name offset for cdResourceDirectory
template <typename T> struct CdResourceTraits;
template <> struct CdResourceTraits<nalAnimClass<nalAnyPose>> {
    static const int kBankType = 5;  // IDA: cdResourceDirectory::Add pushes 5
    static const int kNameOffset = 0x08;  // DataPtr->Name (IDA: nalAnimClass::Name)
    static const char* const kDirectoryName;  // "cdAnimDirectory"
};
template <> struct CdResourceTraits<nalSceneAnim> {
    static const int kBankType = 6;  // IDA: cdResourceDirectory::Add pushes 6
    static const int kNameOffset = 0x10;  // DataPtr->Header.Name
    static const char* const kDirectoryName;  // "cdSceneAnimDirectory"
};
template <> struct CdResourceTraits<nalAnimFile> {
    static const int kBankType = 4;  // IDA: cdResourceDirectory::Add pushes 4
    static const int kNameOffset = 0x10;  // DataPtr->Header.Name
    static const char* const kDirectoryName;  // "cdAnimFileDirectory"
};
template <> struct CdResourceTraits<nalBaseSkeleton> {
    static const int kBankType = 8;  // IDA: cdResourceDirectory::Add pushes 8
    static const int kNameOffset = 0x08;  // DataPtr->Name
    static const char* const kDirectoryName;  // "cdSkeletonDirectory"
};
const char* const CdResourceTraits<nalAnimClass<nalAnyPose>>::kDirectoryName = "cdAnimDirectory";
const char* const CdResourceTraits<nalSceneAnim>::kDirectoryName = "cdSceneAnimDirectory";
const char* const CdResourceTraits<nalAnimFile>::kDirectoryName = "cdAnimFileDirectory";
const char* const CdResourceTraits<nalBaseSkeleton>::kDirectoryName = "cdSkeletonDirectory";

// ea: 0x66F6F0/0x66F760/0x66F7D0/0x66F840
template <typename T>
T* cdResourceDirectory<T>::Add(T* DataPtr)
{
    if (!m_enable_add)
        return DataPtr;
    ae_sized_array<TPakId, 128>& ContextStack =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    int m_size = ContextStack.m_size;
    if (m_size != 0
        && ContextStack.m_elements[m_size - 1] != PAK_ID_INVALID)
    {
        TPakId v6 = ContextStack.m_elements[m_size - 1];
        return (T*)InstanceBankMgr::sInst->Add(
            (eInstanceBankType)CdResourceTraits<T>::kBankType, v6,
            *(tlFixedString*)((char*)DataPtr
                              + CdResourceTraits<T>::kNameOffset),
            (unsigned int)DataPtr);
    }
    return m_old_directory->Add(DataPtr);
}

// ea: 0x679130/0x679160/0x679190/0x6791C0
template <typename T>
T* cdResourceDirectory<T>::Find(const tlFixedString& key)
{
    tlResourceDirectory<T>* m_old_directory = this->m_old_directory;
    if (m_old_directory == nullptr)
        return (T*)InstanceBankMgr::sInst->Get(
            (eInstanceBankType)CdResourceTraits<T>::kBankType,
            PAK_ID_INVALID, &key);
    T* result = m_old_directory->Find(key);
    if (result == nullptr)
        return (T*)InstanceBankMgr::sInst->Get(
            (eInstanceBankType)CdResourceTraits<T>::kBankType,
            PAK_ID_INVALID, &key);
    return result;
}

// ea: 0x6827B0/0x682940/0x682AD0/0x682E40
template <typename T>
void cdResourceDirectory<T>::EnableRelease(bool enabled)
{
    m_enable_release = enabled;
}

// ea: 0x685B00/0x685D10/0x685F20/0x682D40
template <typename T>
bool cdResourceDirectory<T>::Del(T* const DataPtr)
{
    (void)DataPtr;
    return true;
}

// ea: 0x685B70/0x685D80/0x685F90/0x682DB0
template <typename T>
void cdResourceDirectory<T>::ReleaseAll(bool warn, bool system, int maxforce)
{
    (void)warn; (void)system; (void)maxforce;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.h";
    AeAssert::gCurrentLine = 100;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("this probably shouldn't be called anywhere"))
        __debugbreak();
}

// ea: 0x685AF0/0x685D00/0x685F10/0x682D30
template <typename T>
const char* cdResourceDirectory<T>::DirectoryName()
{
    return CdResourceTraits<T>::kDirectoryName;
}

// Manager DecodeBank stubs (cross-object: core.o / render.o / mp_actors.o)
class XModelManager {
public:
    static XModelManager* sInst;  // defined in sv_globals.cpp
    static void* operator new(unsigned int size, void* p);
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);
    IVPointer<XModel> GetXModel(TPakId pak_id, const char* name);  // render.o 0xAC54D0; stub
};
IVPointer<XModel> XModelManager::GetXModel(TPakId pak_id, const char* name)
{
    (void)pak_id; (void)name;
    IVPointer<XModel> result = { nullptr, PAK_ID_INVALID };
    return result;  // stub: render.o
}
class XModelPartsManager {
public:
    static XModelPartsManager* sInst;
    static void* operator new(unsigned int size, void* p);
    static XModelPartsManager* Inst();  // ?Inst@XModelPartsManager@@SAPAV1@XZ
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);
};
class AITypeManager {
public:
    static AITypeManager* sInst;
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);
};
struct vehicle_node_t;  // full def below (g_local.h view)
extern int s_numNodes;  // g.o 0xEA5DD0
extern vehicle_node_t* s_nodes[];  // g.o 0xEAEDF8
class PathNodeMgr {
public:
    static PathNodeMgr* sInst;  // defined in sv_globals.cpp
    int GetVehicleNodeIndex(vehicle_node_t* pNode);  // ?GetVehicleNodeIndex@PathNodeMgr@@QAEHPAUvehicle_node_t@@@Z (stub)
    void SetCoverNodeStatus(const Broc::string& name,
                            int inValid);  // ?SetCoverNodeStatus@PathNodeMgr@@QAEXABV?$string@Broc@@H@Z (sv_misc.cpp)
    void DecodeLevelBank(const char* name, unsigned char* data, int size,
                         TPakId pakId);
    void DecodeZoneBank(const char* name, unsigned char* data, int size,
                        TPakId pakId);
};
class DbTablesetBank;

template <typename BankT>
class InplaceAssetBankSet : public AssetBankSet {
public:
    BankT* mBankArray[99];

    InplaceAssetBankSet() : AssetBankSet()
    {
        memset(mBankArray, 0, sizeof(mBankArray));
    }

    virtual ~InplaceAssetBankSet() {}

    virtual void UnloadBank(TPakId pakId)
    {
        int index = (int)pakId;
        if (index >= 0 && index < 99 && mBankArray[index] != nullptr)
        {
            OnBankUnloaded(mBankArray[index]);
            mBankArray[index] = nullptr;
        }
    }

    virtual void OnBankUnloaded(BankT* bank)
    {
        (void)bank;
    }
};

class DbTablesetMgr : public InplaceAssetBankSet<DbTablesetBank> {
public:
    static DbTablesetMgr* sInst;
    static void* operator new(unsigned int size, void* p);
    static DbTablesetMgr* Inst();
    static DbTablesetMgr* CreateInst();
    static void DeleteInst();
    DbTablesetMgr();
    virtual ~DbTablesetMgr();
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pakId);
};
class LightGridMgr {
public:
    static LightGridMgr* sInst;
    static void* operator new(unsigned int size, void* p);
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pakId);

    struct TOC;  // LightGrid::TOC (render.o; opaque)
    const TOC* GetLightGrid(const math::Position3& posArg,
                            int* pCellNum);  // render.o 0x6C93F0; stub
    void SampleLightGrid(const TOC& toc, int cellidx,
                         const math::Position3& pos, math::Mat44* dir,
                         math::Mat44* color);  // render.o 0x6C9E20; stub
};
const LightGridMgr::TOC* LightGridMgr::GetLightGrid(
    const math::Position3& posArg, int* pCellNum)
{
    (void)posArg; (void)pCellNum;
    return nullptr;  // stub: render.o
}
void LightGridMgr::SampleLightGrid(const TOC& toc, int cellidx,
                                   const math::Position3& pos,
                                   math::Mat44* dir, math::Mat44* color)
{
    (void)toc; (void)cellidx; (void)pos; (void)dir; (void)color;
}
class STBManager {
public:
    static STBManager* sInst;
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);  // core.o 0x4C6070 (stb.cpp)
};

XModelPartsManager* XModelPartsManager::sInst = nullptr;
AITypeManager* AITypeManager::sInst = nullptr;
DbTablesetMgr* DbTablesetMgr::sInst = nullptr;

// ea: 0x004DD040
void* DbTablesetMgr::operator new(unsigned int size, void* p)
{
    (void)size;
    return p;
}

// ea: 0x004DD050
DbTablesetMgr* DbTablesetMgr::Inst()
{
    return DbTablesetMgr::sInst;
}

static_assert(sizeof(InplaceAssetBankSet<DbTablesetBank>) == 0x190,
              "InplaceAssetBankSet<DbTablesetBank> size mismatch");
static_assert(sizeof(DbTablesetMgr) == 0x190,
              "DbTablesetMgr size mismatch");

// ea: 0x004E5F80
DbTablesetMgr::DbTablesetMgr()
    : InplaceAssetBankSet<DbTablesetBank>()
{
}

// ea: 0x004E5FD0
DbTablesetMgr::~DbTablesetMgr()
{
}

// ea: 0x004E8910
DbTablesetMgr* DbTablesetMgr::CreateInst()
{
    if (DbTablesetMgr::sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\DbTablesetMgr.h";
        AeAssert::gCurrentLine = 13;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    void* memory = mem_heap_malloc_ctx(
        0x190u, 4, "core", "c:\\cod\\code\\game\\DbTablesetMgr.h", 13);
    if (memory != nullptr)
    {
        DbTablesetMgr* result = new (memory) DbTablesetMgr();
        DbTablesetMgr::sInst = result;
        return result;
    }
    DbTablesetMgr::sInst = nullptr;
    return nullptr;
}

// ea: 0x004DD060
void DbTablesetMgr::DeleteInst()
{
    DbTablesetMgr* instance = DbTablesetMgr::sInst;
    if (instance == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\DbTablesetMgr.h";
        AeAssert::gCurrentLine = 13;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }
    if (instance != nullptr)
    {
        instance->~DbTablesetMgr();
        mem_heap_free(instance);
    }
    DbTablesetMgr::sInst = nullptr;
}
LightGridMgr* LightGridMgr::sInst = nullptr;

// ea: 0x004B4E30
void* XModelManager::operator new(unsigned int /*size*/, void* p)
{
    return p;
}

// ea: 0x004B4FC0
void* XModelPartsManager::operator new(unsigned int /*size*/, void* p)
{
    return p;
}

// ea: 0x004B5170
void* LightGridMgr::operator new(unsigned int /*size*/, void* p)
{
    return p;
}

void XModelManager::DecodeBank(const char* name, unsigned char* data,
                               int size, TPakId pak_id)
{ (void)name; (void)data; (void)size; (void)pak_id; }
void XModelPartsManager::DecodeBank(const char* name, unsigned char* data,
                                    int size, TPakId pak_id)
{ (void)name; (void)data; (void)size; (void)pak_id; }
void AITypeManager::DecodeBank(const char* name, unsigned char* data,
                               int size, TPakId pak_id)
{ (void)name; (void)data; (void)size; (void)pak_id; }
void PathNodeMgr::DecodeLevelBank(const char* name, unsigned char* data,
                                  int size, TPakId pakId)
{ (void)name; (void)data; (void)size; (void)pakId; }
void PathNodeMgr::DecodeZoneBank(const char* name, unsigned char* data,
                                 int size, TPakId pakId)
{ (void)name; (void)data; (void)size; (void)pakId; }
int PathNodeMgr::GetVehicleNodeIndex(vehicle_node_t* pNode)
{
    // ea: 0x0077F3D0
    int result = 0;
    if (s_numNodes <= 0)
        return -1;
    while (s_nodes[result] != pNode)
    {
        if (++result >= s_numNodes)
            return -1;
    }
    return result;
}
void DbTablesetMgr::DecodeBank(const char* name, unsigned char* data,
                               int size, TPakId pakId)
{ (void)name; (void)data; (void)size; (void)pakId; }
void LightGridMgr::DecodeBank(const char* name, unsigned char* data,
                              int size, TPakId pakId)
{ (void)name; (void)data; (void)size; (void)pakId; }

// ZoneOverrideBrushSet / ZoneBoundaryBank (streamer.o views;
// mToggleableOverrideBoxes +0x3C, mNumToggleableOverrideBoxesHit +0x50)
class ZoneOverrideBrushSet;
class StreamZone;
class ZoneCellDesc;
class ZdNode;

// Bounds-checked InplaceVector access (InplaceVector.h:81, inlined in release)
template <typename T>
static T& InplaceVectorAt(InplaceVector<T>& vec, unsigned int index)
{
    if (index >= vec.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
        AeAssert::gCurrentLine = 81;
        AeAssert::gCurrentExpr = "index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
    }
    if (index >= vec.mSize)
        index = 0;
    return vec.mList[index];
}

class ZoneBoundaryBank {
public:
    uint8_t _pad[0x10];
    InplaceVector<const StreamZone*> mPtrs;  // +0x10 (InplaceAssetBank base)
    void*   mPtrFixupTable;                  // +0x18
    InplaceVector<const ZoneCellDesc*> mCells;  // +0x1C
    int     mInitialCell;                    // +0x24
    math::Position3::Packed mInitialPosition; // +0x28
    InplaceVector<ZoneOverrideBrushSet*> mOverrideBoxes;  // +0x34
    InplaceVector<const ZoneOverrideBrushSet*> mToggleableOverrideBoxes;  // +0x3C
    int     mNextBank;             // +0x44
    TPakId  mPakId;                // +0x48
    const ZdNode* mActiveNode;     // +0x4C
    int     mNumToggleableOverrideBoxesHit;  // +0x50

    void Fixup();  // InplaceAssetBank<StreamZone,...>::Fixup (streamer.o)
    const ZdNode* GetZdNode(int cellId,
                            const math::Position3* position);  // ?GetZdNode@ZoneBoundaryBank@@QAEPBVZdNode@@HABVPosition3@math@@@Z
};

// ZoneOverrideBrush (streamer.o; mAabb +0x00, mPlanes +0x20, size 0x40)
struct ZoneOverrideBrush {
    uint8_t mAabb[0x20];   // +0x00 (BoundingBox)
    uint8_t mPlanes[0x08]; // +0x20
    uint8_t _pad28[0x40 - 0x28];

    bool IsInside(const math::Position3& point);  // ?IsInside@ZoneOverrideBrush@@QBE_NABVPosition3@math@@@Z (stub)
    bool IsInsidePlane(const math::Position3& point,
                       const math::Vector4& vec) const;  // ?IsInsidePlane@ZoneOverrideBrush@@QBE_NABVPosition3@math@@ABVVector4@3@@Z
};

// ZoneOverrideBrushSet (streamer.o; mZoneBitset +0x0C, mZoneDistances +0x14)
class ZoneOverrideBrushSet {
public:
    unsigned int mWasInside;       // +0x00
    InplaceVector<ZoneOverrideBrush> mBrushes;  // +0x04
    BitSet<64>   mZoneBitset;      // +0x0C
    InplaceVector<float> mZoneDistances;  // +0x14
    InplaceVector<InplaceTriple<InplaceString, const PakInfoNode*, float> >
        mNonZoneDistances;         // +0x1C

    bool IsInside(const math::Position3& point);  // ?IsInside@ZoneOverrideBrushSet@@QBE_NABVPosition3@math@@@Z (stub)

    const BitSet<64>& GetZoneBitset() const;      // ?GetZoneBitset@ZoneOverrideBrushSet@@QBEABV?$BitSet@$0EA@@@XZ
    const InplaceVector<float>& GetZoneDistances() const;  // ?GetZoneDistances@ZoneOverrideBrushSet@@QBEABV?$InplaceVector@M@@XZ
    InplaceVector<InplaceTriple<InplaceString, const PakInfoNode*, float> >&
        GetNonZoneDistances();    // ?GetNonZoneDistances@ZoneOverrideBrushSet@@QAEAAV?$InplaceVector@...@@XZ
    bool WasInside() const;       // ?WasInside@ZoneOverrideBrushSet@@QBE_NXZ
    void SetInside(bool isInside);  // ?SetInside@ZoneOverrideBrushSet@@QAEX_N@Z
};

// ZdNode (streamer.o; mZone +0x10, mZoneBitset +0x18, mZoneDistances +0x20)
class ZdNode {
public:
    math::Position3 mPosition;     // +0x00
    const StreamZone* mZone;       // +0x10
    uint8_t _pad14[0x18 - 0x14];
    BitSet<64> mZoneBitset;        // +0x18
    InplaceVector<float> mZoneDistances;  // +0x20

    const math::Position3& GetPosition() const;   // ?GetPosition@ZdNode@@QBEABVPosition3@math@@XZ
    const StreamZone* GetStreamZone() const;      // ?GetStreamZone@ZdNode@@QBEPBVStreamZone@@XZ
    const BitSet<64>& GetZoneBitset() const;      // ?GetZoneBitset@ZdNode@@QBEABV?$BitSet@$0EA@@@XZ
    const InplaceVector<float>& GetZoneDistances() const;  // ?GetZoneDistances@ZdNode@@QBEABV?$InplaceVector@M@@XZ
};

// ZoneCellBox (streamer.o; GetBounds returns this)
// BoundingBox (32 bytes; vmin/vmax Position3, verified IDA)
struct BoundingBox {
    math::Position3 vmin;  // +0x00
    math::Position3 vmax;  // +0x10

    BoundingBox()  // ??0BoundingBox@@QAE@XZ (game2.o 0x517200)
    {
        vmin.v = _mm_set1_ps(3.4028235e38f);
        vmax.v = _mm_set1_ps(-3.4028235e38f);
    }
    ~BoundingBox();  // ??1BoundingBox@@QAE@XZ (game2.o 0x517290)
    math::Position3 center() const;  // ?center@BoundingBox@@QBE?AVPosition3@math@@XZ (game2.o 0x5172A0)
    float width() const;  // ?width@BoundingBox@@QBEMXZ (game2.o 0x5172F0)
    float height() const;  // ?height@BoundingBox@@QBEMXZ (game2.o 0x517320)
    bool intersect(const math::Position3& p) const;
    void accumulate(const math::Position3& p);  // ?accumulate@BoundingBox@@QAEXABVPosition3@math@@@Z
};
static_assert(sizeof(ZoneBoundaryBank) == 0x54,
              "ZoneBoundaryBank layout mismatch");

// XModel / StaticModel / BSP views (render.o + game.o; offsets verified IDA)
struct XModelParts;
struct XModelLod {
    uint8_t _pad[0x08];
    XModelParts* xmodelParts;  // +0x08
};
struct XModelParts {
    uint8_t _pad[0x10];
    struct {
        int  mSize;   // +0x10
        void* mList;  // +0x14
    } mHierarchy;
    uint8_t _pad18[0x28 - 0x18];
    struct {
        int      mSize;  // +0x28
        nglMesh** mList; // +0x2C
    } mMeshes;
};
struct XModel {
    const char* name;     // +0x00
    XModel*     resolved; // +0x04 (cached GetXModel result)
    uint8_t     _pad8[0x24 - 0x08];
    XModelLod** lod;      // +0x24 (null-terminated array)
    uint8_t     _pad28[0x40 - 0x28];
    int         contents; // +0x40
};
struct BspNode {
    uint8_t _pad[0x10];
};
struct BspTree {
    uint8_t _pad[0x08];
    struct {
        int      mSize;  // +0x08
        BspNode* mList;  // +0x0C
    } mNodes;
};
// world_t (render.o; bspTree +0x100, mSky +0x108, size 0x10C)
struct world_t {
    uint8_t _pad[0x100];
    BspTree* bspTree;  // +0x100
    uint8_t _pad104[0x108 - 0x104];
    void*   mSky;      // +0x108
};

// class tag matches ?CM_LinkStaticModel@@YAXPAVStaticModel@@@Z
class StaticModel {
public:
    uint8_t _pad0[0x68];
    XModel* xmodel;        // +0x68
    float   axis[3][3];    // +0x6C
    uint8_t _pad78[0x90 - 0x78];
    float   origin[3];     // +0x90
    float   scale;         // +0x9C
    float   absmin[3];     // +0xA0
    float   absmax[3];     // +0xAC
    uint8_t _padB8[0xE0 - 0xB8];
    TPakId  pakId;         // +0xE0
    uint8_t _padE4[0xE8 - 0xE4];
    int     instance;      // +0xE8
};

void ValidatePakId(TPakId pakId);  // defined below (0x6653A0)

// ea: 0x685AA0
template <typename T>
T* IVPointer<T>::GetRawPtr()
{
    ValidatePakId((TPakId)mPakId);
    return mValue;
}

// render.o entry points (stubs until render.o lands)
extern void R_GetXModelBounds(XModel* m, float (*axis)[3],
                              BoundingBox& bounds);  // render.o 0xAB1DC0
void R_GetXModelBounds(XModel* m, float (*axis)[3], BoundingBox& bounds)
{
    (void)m; (void)axis; (void)bounds;
}
extern void R_FilterModelIntoCells_r(world_t* world, BspNode* node,
                                     StaticModel* psm,
                                     const math::Position3& mins,
                                     const math::Position3& maxs);  // render.o 0xAB62C0
void R_FilterModelIntoCells_r(world_t* world, BspNode* node,
                              StaticModel* psm, const math::Position3& mins,
                              const math::Position3& maxs)
{
    (void)world; (void)node; (void)psm; (void)mins; (void)maxs;
}
extern void XModelGetBasePose(IVPointer<XModel> model,
                              math::Mat43* mat);  // render.o 0xABA270
void XModelGetBasePose(IVPointer<XModel> model, math::Mat43* mat)
{
    (void)model; (void)mat;
}
extern int XModelGetStaticBounds(IVPointer<XModel> model, float (*axis)[3],
                                 math::Position3& mins,
                                 math::Position3& maxs,
                                 const math::Mat43* bones,
                                 int nbones);  // render.o 0xABB0A0
int XModelGetStaticBounds(IVPointer<XModel> model, float (*axis)[3],
                          math::Position3& mins, math::Position3& maxs,
                          const math::Mat43* bones, int nbones)
{
    (void)model; (void)axis; (void)mins; (void)maxs; (void)bones; (void)nbones;
    return 0;
}
extern IVPointer<XModel> gDefaultXmodel;  // render.o @ 0x11EA6C8
IVPointer<XModel> gDefaultXmodel = { nullptr, PAK_ID_INVALID };
extern void CM_LinkStaticModel(StaticModel* staticModel);  // game.o (g_cm_load.cpp)
extern const math::Position3 native_to_cdl_pos3(const float* v);  // g.o inline 0x4AF1C0
extern BspTree* g_bspTree;  // game.o @ 0xF743DC

extern void AngleVectors(const float* const angles, float* const forward,
                         float* const right, float* const up);  // core.o (q_math.cpp)
extern double VectorNormalize(float* const v);  // core.o (q_math.cpp)

// ProcessEntity cross-object externs (game.o / scr.o / physics.o)
class Destructible {
public:
    void Initialize(Entity* ent, bool reInit);  // ?Initialize@Destructible@@QAEXPAVEntity@@_N@Z (g_entity_misc.cpp)
    void AddPiece(Entity* ent,
                  const char* exploderType);  // ?AddPiece@Destructible@@QAEXPAVEntity@@PBD@Z (g_physics.cpp)
    static void InvalidateCoverNode(Entity* ent);  // ?InvalidateCoverNode@Destructible@@SAXPAVEntity@@@Z (g_physics.cpp)
};
void Destructible::Initialize(Entity* ent, bool reInit)
{
    (void)ent; (void)reInit;  // stub: game.o (g_entity_misc.cpp self-version)
}
extern void G_FreeEntity(Entity* e, int msec);  // g.o (g_active.cpp)
extern Entity* G_Spawn(TPakId pakId);  // g.o (g_spawn.cpp)
extern bool ValidForGametype(void);  // g.o (g_main.cpp)
extern void G_CallEntitySpawnFunction(Entity* ent);  // g.o (g_spawn.cpp)
extern void BrocAddEntityThread(Entity* ent,
                                const char* fcnName);  // scr.o
void BrocAddEntityThread(Entity* ent, const char* fcnName)
{
    (void)ent; (void)fcnName;  // stub: scr.o
}
extern IVPointer<Destructible> DestructibleBankManager_GetDestructible(
    void* self, TPakId pak_id, const char* name);  // physics.o (g_checkpoint.cpp)
extern bool dont_delete;            // g.o (g_globals.cpp)
extern bool no_really_delete_it;    // g.o (g_globals.cpp)
extern void* gApsHeap;              // ?gApsHeap@@3PAVae_heap@@A (render.o @ 0x1363940; defined in common.cpp)
extern void AnglesToAxis(const math::Position3& angles,
                         float (*const axis)[3]);  // core.o (q_math.cpp)
extern void nglGetStringDimensions(nglFont* Font, const char* Text,
                                   unsigned int* Width, unsigned int* Height,
                                   float ScaleX, float ScaleY);  // ngl_font.o
extern nglFont* nglSysFont;  // ngl_font.o

// StreamZoneManager.cpp debug statics
static int only_this = -1;
static bool reassign_colors = false;
struct ScriptEventHandler {
    unsigned char m_dlist_node[8];  // +0x00
    uint8_t _pad8[0x40 - 0x08];
    ScriptEventHandler* mNext;      // +0x40

    ScriptEventHandler();  // ?ScriptEventHandler@@QAE@XZ (g_game2_misc.cpp 0x4F9860)
    bool AddEvent(HashString h, const char* callback);  // ?AddEvent@ScriptEventHandler@@QAE_NVHashString@@PBD@Z
};
extern void* ScriptEventHandler_sAllocator;  // ?sAllocator@ScriptEventHandler (g.o)
extern int _stricmp(const char* a, const char* b);  // core.o
extern int _strnicmp(const char* a, const char* b, size_t n);  // core.o
// hash_const (runtime-filled hash constants; mirrors str_const_t layout)
struct hash_const_t {
    uint8_t _pad[0x1D4];
    HashString spawned;      // +0x1D4
    uint8_t _pad1D8[0x26C - 0x1D8];
    HashString zonesloaded;  // +0x26C
};
extern hash_const_t hash_const;  // ?hash_const@@3Uhash_const_t@@A @ 0xED2AB0

extern world_t s_worldData;  // ?s_worldData@@3Uworld_t@@A @ 0xF74B98
world_t s_worldData;
class ZoneCellBox {
public:
    BoundingBox mAabb;                       // +0x00
    uint8_t _pad20[0x24 - 0x20];
    InplaceVector<const ZdNode*> mZdNodes;   // +0x24
    const BoundingBox& GetBounds() const;  // ?GetBounds@ZoneCellBox@@QBEABVBoundingBox@@XZ
    const ZdNode* GetZdNode(const math::Position3& position);
};

// StreamZone (streamer.o view; mName +0x20, mPakInfo +0x24)
class StreamZone {
public:
    uint8_t _pad[0x20];
    InplaceString mName;         // +0x20
    const PakInfoNode* mPakInfo;  // +0x24
    uint8_t _pad28[0x30 - 0x28];
    InplaceVector<const ZoneCellDesc*> mCells;  // +0x30

    const char* GetName() const;              // ?GetName@StreamZone@@QBEPBDXZ
    void SetPakInfo(const PakInfoNode* n);    // ?SetPakInfo@StreamZone@@QBEXPBUPakInfoNode@@@Z
    const PakInfoNode* GetPakInfo() const;     // ?GetPakInfo@StreamZone@@QBEPBUPakInfoNode@@XZ (game2.o 0x5173A0)
};

// ZoneCellDesc (streamer.o view; mZone +0x2C)
class ZoneCellDesc {
public:
    BoundingBox mAabb;          // +0x00
    unsigned int mCellId;       // +0x20
    InplaceVector<const ZoneCellBox*> mCellBoxes;  // +0x24
    const StreamZone* mZone;  // +0x2C

    unsigned int GetCellId() const;  // ?GetCellId@ZoneCellDesc@@QBEIXZ (game2.o 0x517380)
    const StreamZone* GetZone() const;  // ?GetZone@ZoneCellDesc@@QBEPBVStreamZone@@XZ
    const BoundingBox& GetBounds() const;  // ?GetBounds@ZoneCellDesc@@QBEABVBoundingBox@@XZ (game2.o 0x517390)
    bool BoundsIntersect(const math::Position3& p) const;  // stub (BoundingBox::intersect)
    const ZdNode* GetZdNode(const math::Position3& position,
                            bool force);  // ?GetZdNode@ZoneCellDesc@@QAEPBVZdNode@@ABVPosition3@math@@_N@Z
};

// StreamZoneManager (streamer.o; verified IDA: anonymous 0x190-byte head
// region with bank elements at +0x04, mDebugRenderMode +0x190,
// mInitialPosition +0x1A0, mLastPosition +0x1B0, mFirstBank +0x1CC,
// mListSize +0x1D0, total size 0x1E0)
class StreamZoneManager {
public:
    uint8_t _pad0[0x04];  // anonymous head region (bank array starts at +0x04)
    ae_array<ZoneBoundaryBank*, 99> mBankArray;  // +0x04 (elements at +0x04)
    unsigned int mDebugRenderMode;  // +0x190 (bit0 ZoneGraph, bit1 active
                                    // node sphere, bit2 zone names)
    float zoneGraphScale;           // +0x194
    math::Position3 mInitialPosition;  // +0x1A0
    uint8_t _pad1AC[0x1B0 - 0x1AC];
    math::Position3 mLastPosition;  // +0x1B0
    uint8_t _pad1BC[0x1C0 - 0x1BC];
    int     mInitialCell;           // +0x1C0
    int     mLastCellNum;           // +0x1C4
    int     mLastListSize;          // +0x1C8
    int     mFirstBank;             // +0x1CC
    int     mListSize;              // +0x1D0

    static void CreateInst();
    static void DeleteInst();
    static void* operator new(size_t size, void* p);
    StreamZoneManager();       // ??0StreamZoneManager@@QAE@XZ @ 0x678250
    ~StreamZoneManager();      // ??1StreamZoneManager@@UAE@XZ @ 0x6782F0
    static void SingletonDebugRender();  // ?SingletonDebugRender@StreamZoneManager@@SAXXZ @ 0x687640
    void DebugRender();        // ?DebugRender@StreamZoneManager@@QAEXXZ @ 0x66CD60 (stub)
    void RenderZoneGraph(const ZoneBoundaryBank* zbs);  // ?RenderZoneGraph@StreamZoneManager@@QBEXPBVZoneBoundaryBank@@@Z @ 0x6678F0

    static StreamZoneManager* sInst;  // defined in sv_globals.cpp
    void SetInitialPosition(const math::Position3& pos);
    void SetInitialCell(int cell);  // ?SetInitialCell@StreamZoneManager@@QAEXH@Z
    void OnLoading(TPakId pakId);  // ?OnLoading@StreamZoneManager@@QAEXW4TPakId@@@Z (empty no-op)
    void OnLoaded(TPakId pakId);  // ?OnLoaded@StreamZoneManager@@QAEXW4TPakId@@@Z
    void OnUnloaded(TPakId pakId);  // ?OnUnloaded@StreamZoneManager@@QAEXW4TPakId@@@Z
    void OnUnload(TPakId pakId);  // ?OnUnload@StreamZoneManager@@QAEXW4TPakId@@@Z
    int GetNumZones() const;        // ?GetNumZones@StreamZoneManager@@QBEHXZ
    const StreamZone* GetZoneByIndex(unsigned int index) const;  // ?GetZoneByIndex@StreamZoneManager@@QBEPBVStreamZone@@I@Z
    const StreamZone* FindZone(TPakId pakId) const;  // ?FindZone@StreamZoneManager@@QBEPBVStreamZone@@W4TPakId@@@Z
    const StreamZone* FindZone(const char* name) const;  // ?FindZone@StreamZoneManager@@QBEPBVStreamZone@@PBD@Z
    const PakInfoNode* GetCellPakInfo(int cellIndex);  // ?GetCellPakInfo@StreamZoneManager@@QAEPBUPakInfoNode@@H@Z
    const StreamZone* GetCellZone(unsigned int cellIndex);  // ?GetCellZone@StreamZoneManager@@QAEPBVStreamZone@@I@Z
    void CheckpointRestart();       // ?CheckpointRestart@StreamZoneManager@@QAEXXZ
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pakId);  // ?DecodeBank@StreamZoneManager@@QAEXPBDPAEHW4TPakId@@@Z @ 0x6795C0
    void SetPriorities();           // ?SetPriorities@StreamZoneManager@@QAEXXZ @ 0x675B40
    void Update(int cellNum, const math::Position3* pos,
                bool forceReset);   // ?Update@StreamZoneManager@@QAEXHABVPosition3@math@@_N@Z @ 0x678350
private:
    void SetTopOverrideBrushSet(ZoneBoundaryBank* bank,
                                ZoneOverrideBrushSet* zob);  // ?SetTopOverrideBrushSet@StreamZoneManager@@AAEXPAVZoneBoundaryBank@@PAVZoneOverrideBrushSet@@@Z
    void SetDistances(ZoneBoundaryBank* bank, BitSet<64> bits,
                      InplaceVector<float>* dists,
                      bool force);  // ?SetDistances@StreamZoneManager@@AAEXPAVZoneBoundaryBank@@V?$BitSet@$0EA@@@ABV?$InplaceVector@M@@_N@Z
    void SetDistances(ZoneBoundaryBank* bank,
                      ZoneOverrideBrushSet* zob);  // ?SetDistances@StreamZoneManager@@AAEXPAVZoneBoundaryBank@@PAVZoneOverrideBrushSet@@@Z
};

// SceneBank (scenemanager.cpp; verified IDA: mSceneHeapSize +0x08,
// mSceneHeap +0x50, size 0x58)
struct InstanceGroup {
    InplaceString modelName;            // +0x00
    void*         instanceList;         // +0x04 (InstanceListNode*)
    void*         insts;                // +0x08
};
// VehicleNode (scenemanager.cpp; verified IDA: size 0x38)
struct InplaceTreeElementKV {
    InplaceString mKey;  // +0x00
    InplaceString mVal;  // +0x04
};
template <typename K, typename V>
class InplaceTreeElement {
public:
    K mKey;  // +0x00
    V mVal;  // +0x04
};

extern void G_ReplaceSpawnVars(
    const InplaceVector<InplaceTreeElement<unsigned int, InplaceString>>&
        keyValuePairs);  // g.o (g_spawn.cpp)
extern void BrocInitEntity(
    Entity* ent,
    const InplaceVector<InplaceTreeElement<InplaceString, InplaceString>>*
        keyValuePairs);  // scr.o
void BrocInitEntity(
    Entity* ent,
    const InplaceVector<InplaceTreeElement<InplaceString, InplaceString>>*
        keyValuePairs)
{
    (void)ent; (void)keyValuePairs;  // stub: scr.o
}
struct VehicleNode {
    InplaceString name;               // +0x00
    InplaceString target;             // +0x04
    InplaceString script_noteworthy;  // +0x08
    float origin[3];                  // +0x0C
    float angles[3];                  // +0x18
    float speed;                      // +0x24
    float lookAhead;                  // +0x28
    uint8_t rotated;                  // +0x2C
    InplaceVector<InplaceTreeElementKV> mKeyValuePairs;  // +0x30
};

// vehicle_node_t (g_local.h; local copy for streamer.o)
struct vehicle_node_t {
    Broc::string mName;             // +0x00
    Broc::string mTarget;           // +0x04
    float        speed;             // +0x08
    float        lookAhead;         // +0x0C
    Broc::string script_noteworthy; // +0x10
    float        origin[3];         // +0x14
    float        dir[3];            // +0x20
    float        angles[3];         // +0x2C
    float        length;            // +0x38
    int          nextIdx;           // +0x3C
};
extern vehicle_node_t* SP_create_info_vehicle_node(void);  // g_scr_vehicle.cpp
// BrocAPI local view (broc_types.h's is Broc::BrocAPI; the mangled
// ?gpBrocAPI@@3PAUBrocAPI@@A needs a global-scope `struct` tag)
struct BrocExports {
    uint8_t _pad[0x34];
    void (*mSetVNodeField_string)(unsigned int, unsigned int,
                                  Broc::string);  // +0x34
    uint8_t _pad38[0x3C - 0x38];
    void (*mSetVNodeField_int)(unsigned int, unsigned int, int);  // +0x3C
    uint8_t _pad40[0x44 - 0x40];
    void (*mSetVNodeField_float)(unsigned int, unsigned int,
                                 float);  // +0x44
};
struct BrocAPI {
    uint8_t    _pad[0x94];
    unsigned int (*mGetEnt)(Broc::string* name, unsigned int hash,
                            void* a3, void* a4, int a5);  // +0x94
    uint8_t    _pad98[0xBE8 - 0x98];
    BrocExports mBrocExports;  // +0xBE8
};
extern BrocAPI* gpBrocAPI;  // ?gpBrocAPI@@3PAUBrocAPI@@A @ 0xF3ABDC
struct cdSimpleInstance {
    void*  Mesh;           // +0x00
    void*  Section;        // +0x04
    void*  Material;       // +0x08
    int    NInstances;     // +0x0C
    float  mMinLodDist2;   // +0x10
    float  mMaxLodDist2;   // +0x14
    void*  Insts;          // +0x18
    int*   cells;          // +0x1C
    void Destroy();  // ?Destroy@cdSimpleInstance@@QAEXXZ (cdSimpleInstance.cpp)
    void Render();   // ?Render@cdSimpleInstance@@QAEXXZ (render.o 0x7C50F0)
    void Create(nglMesh* mesh, nglMeshSection* section,
                int numInstances);  // render.o 0x7C4CB0; stub
    void Add(const math::Mat43& mat, float scale, const math::Mat44& dir,
             const math::Mat44& color, ae_sized_array<int*, 384>& flags,
             int cellNum);  // render.o 0x7C4D20; stub
    void Finalize(ae_sized_array<int*, 384>& flags);  // render.o 0x7C4FD0; stub
};
struct InstanceListNode {
    cdSimpleInstance instance;          // +0x00 (0x20 bytes)
    uint8_t _pad20[0x24 - 0x20];
    float  mMinDist;                    // +0x24
    float  mMaxDist;                    // +0x28
    int*   mInstanceData;               // +0x2C ({count @ +8, data @ +0xC})
    unsigned int** mRadii;              // +0x30 (per-instance flag words)
    InstanceListNode* next;             // +0x34
};

// InstanceData (scenemanager.cpp; stride 0x28, verified IDA)
struct InstanceData {
    float cullA[3];     // +0x00
    float cullB[3];     // +0x0C
    float worldPos[3];  // +0x18
    float pad;          // +0x24
};

struct SceneEffectGroup;

// SceneEffect (scenemanager.cpp; state machine fields verified IDA)
struct SceneEffect {
    math::Mat43  mPo;           // +0x00 (w row = position)
    const char*  mScriptId;       // +0x40
    SceneEffectGroup* mGroupPtr;  // +0x44 (group hash until resolved)
    unsigned int mFlags;          // +0x48
    unsigned short mLoopDelayMin; // +0x4C
    unsigned short mLoopDelayMax; // +0x4E
    unsigned int mEffectHandle;   // +0x50
    unsigned int mState;          // +0x54 (0 = kStateUninitialized)
    float        mDelayCountdown; // +0x58

    float GetLoopDelayMin() const;  // ?GetLoopDelayMin@SceneEffect@@QBEMXZ
    float GetLoopDelayMax() const;  // ?GetLoopDelayMax@SceneEffect@@QBEMXZ
};
// SceneLight (scenemanager.cpp; verified IDA: size 0x30)
struct SceneLight {
    math::Position3 mPosition;   // +0x00
    float mRed;                  // +0x10
    float mGreen;                // +0x14
    float mBlue;                 // +0x18
    float mGlowRadius;           // +0x1C
    float mGlowIntensity;        // +0x20
    float mGlowFade;             // +0x24
    void* m_target;              // +0x28
    unsigned int mReserved1;     // +0x2C
};
class SceneBank {
public:
    unsigned int mId;              // +0x00
    float        mVersion;         // +0x04
    unsigned int mSceneHeapSize;  // +0x08
    void*        mWorldSpawn;      // +0x0C
    InplaceVector<void*> mSceneEntities;      // +0x10
    InplaceVector<void*> mStaticModels;       // +0x18
    InplaceVector<InstanceGroup> mInstanceGroups;  // +0x20
    InplaceVector<VehicleNode> mVehicleNodes; // +0x28
    InplaceVector<SceneEffect> mSceneEffects;      // +0x30
    InplaceVector<void*> mSceneEffectGroups;       // +0x38
    InplaceVector<SceneLight> mSceneLights;        // +0x40
    InplaceVector<void*> mPersistentStorage;       // +0x48
    unsigned char* mSceneHeap;    // +0x50
    void*       mPtrFixupTable;   // +0x54

    int GetSceneHeapSize();  // ?GetSceneHeapSize@SceneBank@@QAEHXZ
    void Fixup();            // ?Fixup@SceneBank@@QAEXXZ @ 0x685260
};

// PtrFixupTable (inplace.cpp; mList +0, mSize +4)
class PtrFixupTable {
public:
    unsigned int mSize;  // +0x00
    void*        mList;  // +0x04

    void Fixup(const void* basePtr);  // ?Fixup@PtrFixupTable@@QAEXPBX@Z (inplace.cpp 0x7E18D0)
};

// ea: 0x681660
float SceneEffect::GetLoopDelayMin() const
{
    return (float)mLoopDelayMin * 0.01f;
}

// ea: 0x681680
float SceneEffect::GetLoopDelayMax() const
{
    return (float)mLoopDelayMax * 0.01f;
}

// ea: 0x6816A0
int SceneBank::GetSceneHeapSize()
{
    return mSceneHeapSize;
}

// ea: 0x685260
void SceneBank::Fixup()
{
    if (mId != 1396917582)  // FourCC('SCEN')
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Scene.h";
        AeAssert::gCurrentLine = 721;
        AeAssert::gCurrentExpr = "mId == FourCC('SCEN')";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("this is not a scene bank!"))
            __debugbreak();
    }
    if (mVersion != 1.34f)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Scene.h";
        AeAssert::gCurrentLine = 722;
        AeAssert::gCurrentExpr = "mVersion == 1.34f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("incorrect version!"))
            __debugbreak();
    }
    if ((uintptr_t)mPtrFixupTable >= 0x10000000)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Scene.h";
        AeAssert::gCurrentLine = 723;
        AeAssert::gCurrentExpr =
            "((unsigned)mPtrFixupTable<0x10000000)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Fixup offset is unusually large"))
            __debugbreak();
    }
    PtrFixupTable* v2 =
        (PtrFixupTable*)((char*)this + (uintptr_t)mPtrFixupTable);
    mPtrFixupTable = v2;
    v2->Fixup(this);
}

// SceneEffectGroup (scenemanager.cpp; 12 bytes, verified IDA)
struct SceneEffectGroup {
    InplaceString mName;    // +0x00
    unsigned int  mHash;    // +0x04
    unsigned int  mActive;  // +0x08
};

// CheckpointMgr minimal view (full type in sv_stubs.h)
class CheckpointMgr {
public:
    bool mUsingCheckpoints;  // +0x00
    uint8_t _pad01[3];
    uint8_t _pad04[0x574 - 0x04];
    bool mCheckpointSaveExists;  // +0x574
    static CheckpointMgr* sInst;  // ?sInst@CheckpointMgr@@2PAV1@A (sv_main.cpp)

    void RestoreSceneEntity(Entity* pEnt);  // g_checkpoint.cpp (game.o 0x609010)
    bool ExploderCheckpointExploded(int exploderId);  // g_checkpoint.cpp 0x622330
    bool PrecludeExploderPiece(const char* exploderType,
                               int exploderId);  // g_checkpoint.cpp 0x622370
};
// The singleton storage is defined by sv.o in sv_main.cpp.  This reduced
// streamer view only references that canonical symbol.

// DebugRender (render.o; RenderText/RenderSphere defined in g_entity_misc.cpp)
class DebugRender {
public:
    static void RenderText(const char* str, int x, int y, const Color& col,
                           float depth, float size);  // render.o 0xAAC7D0
    static void RenderSphere(const math::Position3& pos, float radius,
                             const Color& argb);  // render.o 0x6D4AF0
    static void RenderText3D(const math::Position3& wpos, const Color& col,
                             float scale, const char* format,
                             ...);  // render.o 0xAB36C0
    static void RenderLine(const math::Position3& pt1,
                           const math::Position3& pt2, const Color& col,
                           float thickness);  // render.o 0xAC7AB0
    static void RenderQuad2D(float l, float t, float r, float b, float z,
                             const Color& col);  // render.o 0xAAC730
};

// SceneManager (render.o view; mWorldSpawn +0x1A0, mDebugRenderDist +0x1B0,
// mDebugRenderEnts +0x1B4, mDebugRenderLights +0x1B5)
struct SceneEntity;
class SceneManager {
public:
    uint8_t _pad[0x04];                      // vftable (AssetBankSet base)
    void*   mPlayerFootstepMaterial;         // +0x04
    int     mPlayerFootstepNumMatches;       // +0x08
    ae_array<SceneBank*, 99> mBankArray;       // +0x0C
    InplaceVector<SceneEffectGroup>* mSceneEffectGroups;  // +0x198
    InplaceVector<unsigned char>* mPersistantStorage;     // +0x19C
    void*   mWorldSpawn;        // +0x1A0
    int     mEffectCount;       // +0x1A4
    int     mEffectDelayFrames; // +0x1A8
    int     mLoadedIdsCount;    // +0x1AC
    float   mDebugRenderDist;   // +0x1B0
    bool    mDebugRenderEnts;   // +0x1B4
    bool    mDebugRenderLights; // +0x1B5

    static SceneManager* sInst;  // ?sInst@SceneManager@@2PAV1@A (sv_main.cpp)
    void ToggleSceneFX(float dist);  // ?ToggleSceneFX@SceneManager@@QAEXM@Z
    void ToggleRenderEnts();         // ?ToggleRenderEnts@SceneManager@@QAEXXZ
    void ToggleRenderLights();       // ?ToggleRenderLights@SceneManager@@QAEXXZ
    void RestartPersistentArray();   // ?RestartPersistentArray@SceneManager@@QAEXXZ
    void ResetAllStaticModels();     // ?ResetAllStaticModels@SceneManager@@QAEXXZ
    void EnableEffect(unsigned int hash);  // ?EnableEffect@SceneManager@@QAEXI@Z
    void DisableEffect(unsigned int hash); // ?DisableEffect@SceneManager@@QAEXI@Z
    SceneBank* GetBank(TPakId pakId);      // ?GetBank@SceneManager@@AAEPAVSceneBank@@W4TPakId@@@Z
    void AddBank(TPakId pakId, SceneBank* bank);  // ?AddBank@SceneManager@@AAEXW4TPakId@@PAVSceneBank@@@Z
    void UnloadInstanceGroups(TPakId pakId);  // ?UnloadInstanceGroups@SceneManager@@AAEXW4TPakId@@@Z
    void UnloadBank(TPakId pakId);  // ?UnloadBank@SceneManager@@EAEXW4TPakId@@@Z
    void DebugRenderLights();  // ?DebugRenderLights@SceneManager@@QAEXXZ
    void ProcessEffects(TPakId pakId, SceneBank* bank);  // ?ProcessEffects@SceneManager@@AAEXW4TPakId@@PAVSceneBank@@@Z
    void PostProcess(TPakId pakId);  // ?PostProcess@SceneManager@@AAEXW4TPakId@@@Z @ 0x6787E0
    void UpdateEffects(float delta_t);  // ?UpdateEffects@SceneManager@@QAEXM@Z @ 0x6690B0
    void DebugRenderFX();  // ?DebugRenderFX@SceneManager@@QAEXXZ @ 0x669530
    void InstanceEntities();  // ?InstanceEntities@SceneManager@@QAEXXZ @ 0x6788F0
    void ConvertEntity(SceneEntity* source, Entity* dest);  // ?ConvertEntity@SceneManager@@AAEXPAVSceneEntity@@PAVEntity@@@Z @ 0x673D60
    void RenderLightGlows();  // ?RenderLightGlows@SceneManager@@QAEXXZ @ 0x675EA0
    void RenderInstanceGroups();  // ?RenderInstanceGroups@SceneManager@@QAEXXZ @ 0x669AF0
    void DebugRenderEnts();  // ?DebugRenderEnts@SceneManager@@QAEXXZ @ 0x672170
    void ProcessInstanceGroup(TPakId pakId, void* group);  // ?ProcessInstanceGroup@SceneManager@@AAEXW4TPakId@@AAVInstanceGroup@@@Z @ 0x673190
    void ProcessStaticModel(TPakId pakId, StaticModel& model);  // ?ProcessStaticModel@SceneManager@@AAEXW4TPakId@@AAVStaticModel@@@Z @ 0x66D670
    void ProcessEntity(TPakId pakId, int entIdx);  // ?ProcessEntity@SceneManager@@AAEXW4TPakId@@H@Z @ 0x676C50
    void ProcessVehicleNode(TPakId pakId, unsigned int nodeIdx);  // ?ProcessVehicleNode@SceneManager@@AAEXW4TPakId@@H@Z @ 0x66DCA0
    SceneManager();            // ??0SceneManager@@QAE@XZ @ 0x6786A0
    ~SceneManager();           // ??1SceneManager@@UAE@XZ @ 0x675E10
    static void CreateInst();  // ?CreateInst@SceneManager@@SAXXZ
    static void DeleteInst();  // ?DeleteInst@SceneManager@@SAXXZ
    static void SingletonDebugRender();  // ?SingletonDebugRender@SceneManager@@SAXXZ @ 0x687880
    void DebugRender();        // ?DebugRender@SceneManager@@QAEXXZ @ 0x675E20 (stub)
    void ProcessWorldSpawn(const WorldSpawn& worldspawn);  // ?ProcessWorldSpawn@SceneManager@@AAEXABVWorldSpawn@@@Z
    void DecodeScene(const char* name, unsigned char* data, int size,
                     TPakId pakId);  // ?DecodeScene@SceneManager@@QAEXPBDPAEHW4TPakId@@@Z @ 0x679D20
};

ae_sized_array<TPakId, 99> loaded_ids;  // ?loaded_ids@@3V?$ae_sized_array@W4TPakId@@$0GD@@@A @ 0xF593B0

// ae_heap (core_xboxr; vtable+4 = Malloc(unsigned size, int align))
class ae_heap {
public:
    void** __vftable;  // +0x00
    mem_heap mHeap;     // +0x04 (IDA ae_heap::mHeap)
    ae_heap(unsigned int size);
    void* Malloc(unsigned int size, unsigned int alignment);
    void* Malloc(unsigned int size, int alignment);
    void Free(void* ptr);  // ?Free@ae_heap@@QAEXPAX@Z (core_xboxr)
    bool CheckFree(void* ptr);
    mem_heap* GetHeapPointer();  // ?GetHeapPointer@ae_heap@@QAEPAUmem_heap@@XZ (core_xboxr)
};
static_assert(sizeof(ae_heap) == 0x4A0, "ae_heap size mismatch");
extern ae_heap* gActorHeap;  // ?gActorHeap@@3PAVae_heap@@A @ 0xF00E5C
extern void* gBrocHeap;       // ?gBrocHeap@@3PAVae_heap@@A @ 0xF3ABE0 (g_globals.cpp)

// The release vtable has the scalar-deleting destructor followed by the four
// ae_heap virtuals.  The local class declarations in older reconstructed
// objects are non-virtual, so this adapter supplies the verified table while
// preserving their existing direct-call ABI.
class ae_heap_vtable_adapter {
public:
    virtual ~ae_heap_vtable_adapter() {}
    virtual void* Malloc(unsigned int size, unsigned int alignment)
    {
        return reinterpret_cast<ae_heap*>(this)->Malloc(size, alignment);
    }
    virtual void Free(void* ptr)
    {
        reinterpret_cast<ae_heap*>(this)->Free(ptr);
    }
    virtual bool CheckFree(void* ptr)
    {
        return reinterpret_cast<ae_heap*>(this)->CheckFree(ptr);
    }
    virtual mem_heap* GetHeapPointer()
    {
        return reinterpret_cast<ae_heap*>(this)->GetHeapPointer();
    }
};
static ae_heap_vtable_adapter s_ae_heap_vtable_adapter;

// ae_heap (core_xboxr; reconstructed from IDA 0x7BBE40-0x7BC00B)
ae_heap::ae_heap(unsigned int size)
{
    __vftable = *reinterpret_cast<void***>(&s_ae_heap_vtable_adapter);
    void* block = mem_heap_malloc(size);
    mem_heap_create(&mHeap, block, static_cast<char*>(block) + size,
                    nullptr);
}
void* ae_heap::Malloc(unsigned int size, unsigned int alignment)
{
    if (size + mHeap.used_byte <= mHeap.size)
        return mem_heap_malloc(&mHeap, static_cast<int>(alignment), size);
    return nullptr;
}
void* ae_heap::Malloc(unsigned int size, int alignment)
{
    return Malloc(size, (unsigned int)alignment);
}
void ae_heap::Free(void* ptr)
{
    mem_heap_free(&mHeap, ptr);
}
bool ae_heap::CheckFree(void* ptr)
{
    if (ptr < mHeap.start || ptr >= mHeap.end)
        return false;
    mem_heap_free(&mHeap, ptr);
    return true;
}
mem_heap* ae_heap::GetHeapPointer()
{
    return &mHeap;
}

// ea: 0x4BB280 (core.o; exact IDA reconstruction)
ae_heap* SetupActorHeap()
{
    ae_heap* result = gActorHeap;
    if (gActorHeap == nullptr)
    {
        ae_heap* storage = (ae_heap*)mem_heap_malloc(0x4A0u);
        if (storage != nullptr)
        {
            result = new (storage) ae_heap(0x8000u);
            gActorHeap = result;
        }
        else
        {
            gActorHeap = nullptr;
            return nullptr;
        }
    }
    return result;
}

// ea: 0x8A39E0 (core.o inline)
void PakManager::CreateInst()
{
    if (sInst != nullptr
        && _tlAssert("c:\\cod\\code\\game\\PakManager.h", 76,
                     "sInst==0", "singleton already created!"))
        __debugbreak();

    void* memory = mem_heap_malloc_ctx(
        0x45A4u, 4, "pak", "c:\\cod\\code\\game\\PakManager.h", 76);
    if (memory != nullptr)
        sInst = new (memory) PakManager();
    else
        sInst = nullptr;
}

// ea: 0x004A5020 / 0x004A5030 / 0x004A5040 (g.o accessors; defined here
// against the real streamer.o class)
PakManager* PakManager::Inst()
{
    return PakManager::sInst;
}
void PakManager::ToggleEnabled()
{
    this->mEnabled ^= 1u;
}
const reserved_dlist<PakFile>& PakManager::GetActivePaks() const
{
    return this->mActivePaks;
}
int PakFile::get_dlist_node_offset()
{
    return 0;
}

// ea: 0x8D4D40 (core.o inline)
void PakManager::DeleteInst()
{
    if (sInst == nullptr
        && _tlAssert("c:\\cod\\code\\game\\PakManager.h", 76,
                     "sInst!=0", "singleton not created!"))
        __debugbreak();
    if (sInst != nullptr)
    {
        sInst->~PakManager();
        mem_heap_free(sInst);
    }
    sInst = nullptr;
}

unsigned int PakManager::sComputeDistanceKey = 1;
float PakManager::sBrocPercentage = 0.1f;
float PakManager::sWbkPercentage = 0.1f;
PoolAllocator* gPakMemHeapAllocator = nullptr;  // ?gPakMemHeapAllocator@@3PAVPoolAllocator@@A @ 0xF592F0

// ea: 0x666C50
const ae_sized_array<TPakId, 128>& PakManager::GetContextStack() const
{
    uint32_t CurrentThreadId = GetCurrentThreadId();
    int v3 = -1;
    int v4 = 0;
    const int* p_m_size = &mContextStack[0].m_size;
    do
    {
        if (*(p_m_size - 129) == (int)CurrentThreadId)
            return *(const ae_sized_array<TPakId, 128>*)(p_m_size - 128);
        if (v3 == -1 && *p_m_size == 0)
            v3 = v4;
        ++v4;
        p_m_size += 130;
    } while (v4 == 0);
    if (v3 == -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 2439;
        AeAssert::gCurrentExpr = "first_avail!=-1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Too many threads trying to create Context stacks"))
            __debugbreak();
    }
    const_cast<TThreadedPakContextStack*>(&mContextStack[v3])->key = CurrentThreadId;
    return *(const ae_sized_array<TPakId, 128>*)&mContextStack[v3].stack;
}

// ea: 0x666D00
void PakManager::PushContext(TPakId pakId)
{
    ae_sized_array<TPakId, 128>& stack =
        (ae_sized_array<TPakId, 128>&)GetContextStack();
    stack.m_elements[stack.m_size++] = pakId;
}

// ea: 0x66CB90
TPakId PakManager::PopContext()
{
    ae_sized_array<TPakId, 128>& stack =
        (ae_sized_array<TPakId, 128>&)GetContextStack();
    int m_size = stack.m_size;
    TPakId result = stack.m_elements[(m_size - 1) & ((m_size - 1 <= 0) - 1)];
    if (m_size != 0)
        stack.m_size = m_size - 1;
    return result;
}

// ea: 0x66CB60
TPakId PakManager::GetTopContext() const
{
    const ae_sized_array<TPakId, 128>& stack = GetContextStack();
    int m_size = stack.m_size;
    if (m_size != 0)
        return stack.m_elements[m_size - 1 <= 0 ? PAK_ID_MIN : m_size - 1];
    return (TPakId)-1;
}

// ea: 0x665420
void PakManager::SetUserDistance(const PakInfoNode* cpak, float dist)
{
    if (cpak != NULL)
    {
        const_cast<PakInfoNode*>(cpak)->userDistance = dist;
        ++sComputeDistanceKey;
    }
}

// ea: 0x665450
void PakManager::ClearUserDistance(const PakInfoNode* cpak)
{
    const_cast<PakInfoNode*>(cpak)->userDistance = 3.4028235e38f;
}

// ea: 0x4A5050
void PakManager::SetProgressCallback(void (*cb)(float))
{
    mProgressCallback = cb;
}

// ea: 0x4B45E0
TPakId PakManager::GetGlobalPakId() const
{
    return mGlobalPakId;
}

// ea: 0x4B4620
TPakId PakManager::GetCurrentPakId() const
{
    return mCurrentPakId;
}

// ea: 0x6657A0
bool PakManager::IsLoaded(TPakId id) const
{
    return id != PAK_ID_INVALID && mSlots[id] != NULL && mSlots[id]->mState == 0;
}

// ea: 0x665830
bool PakManager::IsUnloading(TPakId id) const
{
    if (mCurrentPakId != id)
        return false;
    return mState == 1;  // STATE_UNLOADING
}

// ea: 0x665710
void PakManager::SetSoundProgress(float t)
{
    void (*cb)(float) = mProgressCallback;
    if (cb != NULL)
        cb(((1.0f - sWbkPercentage) - sBrocPercentage) + (sWbkPercentage * t));
}

// ea: 0x665760
void PakManager::SetBrocProgress(float t)
{
    void (*cb)(float) = mProgressCallback;
    if (cb != NULL)
        cb((sBrocPercentage * t) + (1.0f - sBrocPercentage));
}

// ea: 0x665870 (empty no-op)
void NflMessage()
{
}

// ea: 0x665880
void NflError(const char* msg)
{
    printf("NFL ERROR: %s\n", msg);
}

// ea: 0x6658A0
void NflWarning(const char* msg)
{
    printf("NFL WARNING: %s\n", msg);
}

// ea: 0x6659D0
void StreamZoneManager::SetInitialPosition(const math::Position3& pos)
{
    mInitialPosition.v = pos.v;
}

// ea: 0x665A00 (empty no-op)
void StreamZoneManager::OnUnloaded(TPakId pakId)
{
    (void)pakId;
}

// ea: 0x6659C0
void StreamZoneManager::SetInitialCell(int cell)
{
    mInitialCell = cell;
}

// ae_array<ZoneBoundaryBank*,99>::operator[] const (../ae/core/ae_array.h:25)
static ZoneBoundaryBank* ZoneBankAt(const StreamZoneManager& szm, int idx)
{
    if (idx > 0x62)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
        AeAssert::gCurrentLine = 25;
        AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    return szm.mBankArray.m_elements[idx];
}

// ae_array<ZoneBoundaryBank*,99>::operator[] (../ae/core/ae_array.h:31)
static ZoneBoundaryBank*& ZoneBankRef(StreamZoneManager& szm, unsigned int idx)
{
    if (idx > 0x62)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
        AeAssert::gCurrentLine = 31;
        AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    return szm.mBankArray.m_elements[idx];
}

// InplaceVector<const T*>::operator[] const (../ae/inplace/InplaceVector.h:91)
template <typename T>
static const T& InplaceVectorAtConst(const InplaceVector<T>& vec,
                                     unsigned int index)
{
    if (index >= vec.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
        AeAssert::gCurrentLine = 91;
        AeAssert::gCurrentExpr = "index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
        if (index >= vec.mSize)
            index = 0;
    }
    return vec.mList[index];
}

// ea: 0x6672B0
void StreamZoneManager::CheckpointRestart()
{
    for (unsigned int i = mFirstBank; i != -1;
         i = mBankArray.m_elements[i]->mNextBank)
    {
        ZoneBankRef(*this, i)->mNumToggleableOverrideBoxesHit = 0;
    }
}

// ea: 0x667380
const PakInfoNode* StreamZoneManager::GetCellPakInfo(int cellIndex)
{
    if (mFirstBank == -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\StreamZoneManager.cpp";
        AeAssert::gCurrentLine = 467;
        AeAssert::gCurrentExpr = "mFirstBank!=-1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("sanity check"))
            __debugbreak();
    }
    if (cellIndex < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\StreamZoneManager.cpp";
        AeAssert::gCurrentLine = 468;
        AeAssert::gCurrentExpr = "cellIndex >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("sanity check"))
            __debugbreak();
    }
    ZoneBoundaryBank* bank = ZoneBankRef(*this, mFirstBank);
    const ZoneCellDesc* cell =
        InplaceVectorAt(bank->mCells, (unsigned int)cellIndex);
    return cell->mZone->mPakInfo;
}

// ea: 0x667450
const StreamZone* StreamZoneManager::GetCellZone(unsigned int cellIndex)
{
    if (mFirstBank == -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\StreamZoneManager.cpp";
        AeAssert::gCurrentLine = 482;
        AeAssert::gCurrentExpr = "mFirstBank!=-1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("sanity check"))
            __debugbreak();
    }
    ZoneBoundaryBank* bank = ZoneBankRef(*this, mFirstBank);
    const ZoneCellDesc* cell =
        InplaceVectorAt(bank->mCells, cellIndex);
    return cell->mZone;
}

// ea: 0x6688C0
const StreamZone* StreamZoneManager::FindZone(TPakId pakId) const
{
    unsigned int mFirstBank = (unsigned int)this->mFirstBank;
    if (mFirstBank == (unsigned int)-1)
        return nullptr;

    for (;;)
    {
        ZoneBoundaryBank* v5 = ZoneBankAt(*this, mFirstBank);
        unsigned int v8 = mFirstBank;
        if (v5->mPtrs.mSize != 0)
        {
            unsigned int v4 = 0;
            for (;;)
            {
                const StreamZone* result =
                    InplaceVectorAtConst(v5->mPtrs, v4);
                if (result->mPakInfo->pakId == pakId)
                    return result;
                if (++v4 >= v5->mPtrs.mSize)
                    break;
            }
        }
        int i = ZoneBankAt(*this, v8)->mNextBank;
        if (i == -1)
            return nullptr;
        mFirstBank = (unsigned int)i;
    }
}

// ea: 0x66CEF0
const StreamZone* StreamZoneManager::FindZone(const char* name) const
{
    unsigned int mFirstBank = (unsigned int)this->mFirstBank;
    if (mFirstBank == (unsigned int)-1)
        return nullptr;
    while (1)
    {
        if (mFirstBank > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 25;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        ZoneBoundaryBank* v4 = mBankArray.m_elements[mFirstBank];
        unsigned int* v5 = InplaceTreeFindStringUInt(
            *(const InplaceTree<InplaceString, unsigned int>*)
                ((char*)v4 + 8),
            name);
        if (v5 != nullptr)
        {
            const StreamZone* result = v4->mPtrs.mList[*v5];
            if (result != nullptr)
                return result;
        }
        mFirstBank = (unsigned int)mBankArray.m_elements[mFirstBank]->mNextBank;
        if (mFirstBank == (unsigned int)-1)
            return nullptr;
    }
}

// ea: 0x668A30
const StreamZone* StreamZoneManager::GetZoneByIndex(
    unsigned int index) const
{
    ZoneBoundaryBank* v2 = ZoneBankAt(*this, PakManager::sInst->mLevelPakId);
    if (v2 == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\StreamZoneManager.cpp";
        AeAssert::gCurrentLine = 821;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("unable to get Zone, ZBB may not be loaded"))
            __debugbreak();
    }
    unsigned int mSize = v2->mPtrs.mSize;
    if (index >= mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::AC;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\StreamZoneManager.cpp";
        AeAssert::gCurrentLine = 823;
        AeAssert::gCurrentExpr = "index < bank->GetNumZones()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid zone index."))
            __debugbreak();
    }
    return InplaceVectorAtConst(v2->mPtrs, index);
}

// ea: 0x668AE0
int StreamZoneManager::GetNumZones() const
{
    ZoneBoundaryBank* v1 = ZoneBankAt(*this, PakManager::sInst->mLevelPakId);
    if (v1 == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\StreamZoneManager.cpp";
        AeAssert::gCurrentLine = 833;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("unable to get Zone, ZBB may not be loaded"))
            __debugbreak();
    }
    return v1->mPtrs.mSize;
}

// ea: 0x666FE0
void StreamZoneManager::SetTopOverrideBrushSet(ZoneBoundaryBank* bank,
                                               ZoneOverrideBrushSet* zob)
{
    InplaceVector<const ZoneOverrideBrushSet*>& boxes =
        bank->mToggleableOverrideBoxes;
    unsigned int mSize;

    if (bank->mNumToggleableOverrideBoxesHit == 0
        || InplaceVectorAt(boxes,
                           bank->mNumToggleableOverrideBoxesHit - 1) != zob)
    {
        int v6 = 0;
        if (bank->mNumToggleableOverrideBoxesHit <= 0)
            goto not_in_hit_region;

        while (InplaceVectorAt(boxes, v6) != zob)
        {
            if (++v6 >= bank->mNumToggleableOverrideBoxesHit)
                goto not_in_hit_region;
        }
        if (v6 < bank->mNumToggleableOverrideBoxesHit - 1)
        {
            do
            {
                const ZoneOverrideBrushSet* v13 =
                    InplaceVectorAt(boxes, v6 + 1);
                InplaceVectorAt(boxes, v6++) = v13;
            } while (v6 < bank->mNumToggleableOverrideBoxesHit - 1);
        }
        InplaceVectorAt(boxes, bank->mNumToggleableOverrideBoxesHit - 1) = zob;
        return;

    not_in_hit_region:
        unsigned int v7 = bank->mNumToggleableOverrideBoxesHit;
        mSize = boxes.mSize;
        if (v7 >= mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\StreamZoneManager.cpp";
            AeAssert::gCurrentLine = 344;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored() && AeAssert::Warning("box not in list?"))
                __debugbreak();
            return;
        }
        for (;;)
        {
            if (InplaceVectorAt(boxes, v7) == zob)
                break;
            if (++v7 >= mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\StreamZoneManager.cpp";
                AeAssert::gCurrentLine = 344;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored() && AeAssert::Warning("box not in list?"))
                    __debugbreak();
                return;
            }
        }
        const ZoneOverrideBrushSet* displaced =
            InplaceVectorAt(boxes, bank->mNumToggleableOverrideBoxesHit);
        InplaceVectorAt(boxes, v7) = displaced;
        InplaceVectorAt(boxes, bank->mNumToggleableOverrideBoxesHit) = zob;
        ++bank->mNumToggleableOverrideBoxesHit;
    }
}

// ea: 0x665A10
void SceneManager::ToggleSceneFX(float dist)
{
    float v2 = dist;
    if (dist == -1.0f)
    {
        if (mDebugRenderDist != 0.0f)
        {
            mDebugRenderDist = 0.0f;
            return;
        }
        goto LABEL_5;
    }
    if (dist == 1.0f)
    LABEL_5:
        v2 = 500.0f;
    mDebugRenderDist = v2;
}

// ea: 0x665A70
void SceneManager::ToggleRenderEnts()
{
    mDebugRenderEnts = !mDebugRenderEnts;
}

// ea: 0x665A90
void SceneManager::ToggleRenderLights()
{
    mDebugRenderLights = !mDebugRenderLights;
}

// ea: 0x668C30
void SceneManager::RestartPersistentArray()
{
    InplaceVector<unsigned char>* storage = mPersistantStorage;
    if (storage == nullptr)
        return;
    if (CheckpointMgr::sInst->mUsingCheckpoints
        && CheckpointMgr::sInst->mCheckpointSaveExists)
    {
        for (unsigned int i = 0; i < storage->mSize; ++i)
        {
            if (storage->mList[i] == 1)
                mPersistantStorage->mList[i] = 0;
            storage = mPersistantStorage;
        }
    }
    else
    {
        for (unsigned int j = 0; j < storage->mSize; ++j)
        {
            storage->mList[j] = 0;
            storage = mPersistantStorage;
        }
    }
}

// ea: 0x668CB0
void SceneManager::ResetAllStaticModels()
{
    for (int i = 0; i < mLoadedIdsCount; ++i)
    {
        if (i > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        if ((unsigned int)loaded_ids.m_elements[i] > 0x62u)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
    }
}

// ea: 0x668ED0
void SceneManager::EnableEffect(unsigned int hash)
{
    if (mSceneEffectGroups == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 335;
        AeAssert::gCurrentExpr = "mSceneEffectGroups";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no groups! maybe rebuild bsp?"))
            __debugbreak();
    }
    if (mSceneEffectGroups != nullptr)
    {
        unsigned int v4 = 0;
        if (mSceneEffectGroups->mSize != 0)
        {
            while (1)
            {
                if (mSceneEffectGroups->mList[v4].mHash == hash)
                    break;
                if (++v4 >= mSceneEffectGroups->mSize)
                    goto not_found;
            }
            mSceneEffectGroups->mList[v4].mActive = 1;
            return;
        }
not_found:
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 348;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Unable to find any group with this id"))
            __debugbreak();
    }
}

// ea: 0x668FC0
void SceneManager::DisableEffect(unsigned int hash)
{
    if (mSceneEffectGroups == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 355;
        AeAssert::gCurrentExpr = "mSceneEffectGroups";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no groups! maybe rebuild bsp?"))
            __debugbreak();
    }
    if (mSceneEffectGroups != nullptr)
    {
        unsigned int v4 = 0;
        if (mSceneEffectGroups->mSize != 0)
        {
            while (1)
            {
                if (mSceneEffectGroups->mList[v4].mHash == hash)
                    break;
                if (++v4 >= mSceneEffectGroups->mSize)
                    goto not_found;
            }
            mSceneEffectGroups->mList[v4].mActive = 0;
            return;
        }
not_found:
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 368;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Unable to find any group with this id"))
            __debugbreak();
    }
}

// ea: 0x66A0B0
SceneBank* SceneManager::GetBank(TPakId pakId)
{
    if (mBankArray.m_elements[pakId] == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 2566;
        AeAssert::gCurrentExpr = "mBankArray[(int)pakId] != 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no bank for this pak id!"))
            __debugbreak();
    }
    return mBankArray.m_elements[pakId];
}

// ea: 0x66DFE0
void SceneManager::AddBank(TPakId pakId, SceneBank* bank)
{
    if (mBankArray.m_elements[pakId] != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 2550;
        AeAssert::gCurrentExpr = "mBankArray[(int)pakId] == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "We already have a bank for this pak id!"))
            __debugbreak();
    }
    mBankArray.m_elements[pakId] = bank;
    if (bank->mSceneHeap != nullptr)
    {
        if (pakId > 0x62)
            ((PakFile*)nullptr)->AddHeap(bank->mSceneHeap,
                                         bank->mSceneHeapSize);
        else
            PakManager::sInst->mSlots[pakId]->AddHeap(bank->mSceneHeap,
                                                      bank->mSceneHeapSize);
    }
}

// ea: 0x676B20
void SceneManager::UnloadInstanceGroups(TPakId pakId)
{
    PakHeapContext heapContext(pakId, false);
    SceneBank* v4 = mBankArray.m_elements[pakId];
    InplaceVector<InstanceGroup>* p_mInstanceGroups =
        &v4->mInstanceGroups;
    if (p_mInstanceGroups->mSize != 0)
    {
        for (unsigned int i = 0; i < p_mInstanceGroups->mSize; ++i)
        {
            InstanceListNode* instanceList =
                (InstanceListNode*)p_mInstanceGroups->mList[i]
                    .instanceList;
            if (instanceList != nullptr)
            {
                InstanceListNode* next;
                do
                {
                    next = instanceList->next;
                    instanceList->instance.Destroy();
                    ae_sized_array<TPakId, 128>& ContextStack =
                        (ae_sized_array<TPakId, 128>&)
                            PakManager::sInst->GetContextStack();
                    TPakId v11 = PAK_ID_INVALID;
                    if (ContextStack.m_size != 0)
                        v11 = ContextStack
                                  .m_elements[ContextStack.m_size - 1];
                    PakManager::sInst->MemFree(v11,
                                               instanceList->mRadii,
                                               false);
                    PakManager::sInst->MemFree(pakId, instanceList, false);
                    instanceList = next;
                } while (next != nullptr);
            }
            p_mInstanceGroups->mList[i].instanceList = nullptr;
        }
    }
    if (heapContext.mPakId != PAK_ID_INVALID)
    {
        TlSystemCallbacks::LockTlAllocsToPakHeap(heapContext.mLastState,
                                                 false);
        ae_sized_array<TPakId, 128>& ContextStack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst
                ->GetContextStack();
        if (ContextStack.m_size != 0)
            ContextStack.m_size = ContextStack.m_size - 1;
    }
}

// ea: 0x6773B0
void SceneManager::UnloadBank(TPakId pakId)
{
    if (mBankArray.m_elements[pakId] != nullptr)
    {
        UnloadInstanceGroups(pakId);
        SceneBank* v4 = mBankArray.m_elements[pakId];
        InplaceVector<SceneEffect>* p_mSceneEffects =
            &v4->mSceneEffects;
        if (v4->mSceneEffects.mSize != 0)
        {
            for (unsigned int v6 = 0; v6 < p_mSceneEffects->mSize; ++v6)
            {
                EffectEventSys::sInst->StopEffect(
                    p_mSceneEffects->mList[v6].mEffectHandle, true);
            }
        }
        if (v4->mSceneEffectGroups.mSize != 0)
            mSceneEffectGroups = nullptr;
        if (v4->mPersistentStorage.mSize != 0)
            mPersistantStorage = nullptr;
        mBankArray.m_elements[pakId] = nullptr;
        R_DestroyStaticModels(pakId);
        CM_DestroyStaticModels(pakId);
        for (int i = 0; i < mLoadedIdsCount; ++i)
        {
            if (i > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            if (loaded_ids.m_elements[i] == pakId)
            {
                if (i < mLoadedIdsCount - 1)
                {
                    loaded_ids.m_elements[i] =
                        loaded_ids.m_elements[mLoadedIdsCount - 1];
                    --i;
                }
                --mLoadedIdsCount;
            }
        }
    }
}

// ea: 0x669A20
void SceneManager::DebugRenderLights()
{
    if (!mDebugRenderLights)
        return;
    EntityManager::sInst->GetPlayer(currCl);
    for (int v3 = 0; v3 < 99; ++v3)
    {
        SceneBank* v4 = mBankArray.m_elements[v3];
        if (v4 != nullptr)
        {
            InplaceVector<SceneLight>* v5 = &v4->mSceneLights;
            for (unsigned int i = 0; i < v5->mSize; ++i)
            {
                SceneLight* v7 = &v5->mList[i];
                Color v10;
                v10.r = v7->mRed;
                v10.g = v7->mGreen;
                v10.b = v7->mBlue;
                v10.a = v7->mGlowIntensity * 0.5f;
                DebugRender::RenderSphere(v7->mPosition, v7->mGlowRadius,
                                          v10);
            }
        }
    }
}

// ea: 0x668D80
void SceneManager::ProcessEffects(TPakId pakId, SceneBank* bank)
{
    if (bank->mSceneEffectGroups.mSize != 0)
    {
        if (mSceneEffectGroups != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
            AeAssert::gCurrentLine = 308;
            AeAssert::gCurrentExpr = "!mSceneEffectGroups";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                       "More than one scene file with an effect group list!"))
                __debugbreak();
        }
        mSceneEffectGroups = (InplaceVector<SceneEffectGroup>*)&bank
                                 ->mSceneEffectGroups;
    }
    InplaceVector<SceneEffect>* p_mSceneEffects = &bank->mSceneEffects;
    for (unsigned int v5 = 0; v5 < p_mSceneEffects->mSize; ++v5)
    {
        SceneEffect* v6 = &p_mSceneEffects->mList[v5];
        v6->mState = 0;  // kStateUninitialized
        v6->mDelayCountdown = (float)mEffectDelayFrames;
        int mEffectCount = this->mEffectCount;
        this->mEffectCount = mEffectCount + 1;
        if (mEffectCount > 20)
        {
            this->mEffectCount = 0;
            mEffectDelayFrames = mEffectDelayFrames + 1;
        }
    }
    unsigned int mLoadedIdsCount = this->mLoadedIdsCount;
    this->mLoadedIdsCount = mLoadedIdsCount + 1;
    if (mLoadedIdsCount > 0x62)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
        AeAssert::gCurrentLine = 154;
        AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    loaded_ids.m_elements[mLoadedIdsCount] = pakId;
}

// ea: 0x6690B0
void SceneManager::UpdateEffects(float delta_t)
{
    if (delta_t == 0.0f)
        return;
    mEffectCount = 0;
    mEffectDelayFrames = 0;
    const Entity* level = (const Entity*)EntityManager::sInst->mWorld;
    for (int v3 = 0; v3 < mLoadedIdsCount; ++v3)
    {
        TPakId pakId = loaded_ids.m_elements[v3];
        SceneBank* bank = mBankArray.m_elements[pakId];
        InplaceVector<SceneEffect>* p_mSceneEffects = &bank->mSceneEffects;
        for (unsigned int i = 0; i < p_mSceneEffects->mSize; ++i)
        {
            SceneEffect* effect = &p_mSceneEffects->mList[i];
            if (effect->mState == 5)
                continue;
            ActiveEffectSet* fxset =
                EffectEventSys::sInst->GetActiveEffectSet(
                    Handle{effect->mEffectHandle});
            int v18;
            bool v19;
            unsigned int v20;
            float tmin;
            float tmaxa;
            Handle h2;
            switch (effect->mState)
            {
            case 0:
                effect->mDelayCountdown -= 1.0f;
                if (effect->mDelayCountdown > 0.0f)
                    break;
                {
                    unsigned int tmax = (unsigned int)effect->mGroupPtr;
                    if (tmax == 0)
                        goto UpdateEffects_L23;
                    if (mSceneEffectGroups == nullptr)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\scenemanager.cpp";
                        AeAssert::gCurrentLine = 405;
                        AeAssert::gCurrentExpr = "mSceneEffectGroups";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(
                                   "no scene file loaded with effect group list!"))
                            __debugbreak();
                    }
                    effect->mGroupPtr = nullptr;
                    if (mSceneEffectGroups == nullptr)
                        goto UpdateEffects_L23;
                    unsigned int v14 = 0;
                    if (mSceneEffectGroups->mSize != 0)
                    {
                        while (mSceneEffectGroups->mList[v14].mHash != tmax)
                        {
                            if (++v14 >= mSceneEffectGroups->mSize)
                                goto UpdateEffects_L18;
                        }
                        effect->mGroupPtr = &mSceneEffectGroups->mList[v14];
                    }
                UpdateEffects_L18:
                    if (effect->mGroupPtr == nullptr)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\scenemanager.cpp";
                        AeAssert::gCurrentLine = 417;
                        AeAssert::gCurrentExpr = "effect.mGroupPtr";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(
                                   "Didn't find effect group for hash 0x%08x",
                                   tmax))
                            __debugbreak();
                    }
                }
            UpdateEffects_L23:
                if (fxset != nullptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::ARO;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\scenemanager.cpp";
                    AeAssert::gCurrentLine = 421;
                    AeAssert::gCurrentExpr = "!fxset";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(
                               "shouldn't be initialized already"))
                        __debugbreak();
                }
                {
                    SceneEffectGroup* mVal = effect->mGroupPtr;
                    if (mVal == nullptr || mVal->mActive != 0)
                    {
                        Handle h;
                        h.mVal = PostEffectEventScriptCall(
                            level, effect->mScriptId, false, pakId,
                            false).mVal;
                        effect->mEffectHandle = h.mVal;
                        fxset = EffectEventSys::sInst->GetActiveEffectSet(h);
                        if (fxset == nullptr)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::ARO;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\scenemanager.cpp";
                            AeAssert::gCurrentLine = 430;
                            AeAssert::gCurrentExpr = "fxset";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert(
                                       "should always have a fx set here, even if the trigger does nothing"))
                                __debugbreak();
                        }
                        effect->mState = 1;
                    }
                    else
                    {
                        effect->mState = 4;
                    }
                }
                goto UpdateEffects_L59;

            case 1:
                if (fxset != nullptr)
                    goto UpdateEffects_L58;
                effect->mState = 5;
                break;

            case 2:
                v18 = (effect->mGroupPtr != nullptr)
                          ? effect->mGroupPtr->mActive
                          : 1;
                v19 = (v18 != 0);
                if (fxset != nullptr)
                {
                    if (!v19)
                    {
                        EffectEventSys::sInst->StopEffect(
                            Handle{effect->mEffectHandle}, false);
                        effect->mEffectHandle = 0;
                    UpdateEffects_L53:
                        effect->mState = 4;
                        break;
                    }
                    goto UpdateEffects_L59;
                }
                else
                {
                    if (!v19)
                        goto UpdateEffects_L53;
                    v20 = effect->mFlags;
                    if ((v20 & 1) == 0 && (v20 & 4) != 0)
                    {
                        effect->mState = 5;
                        break;
                    }
                    effect->mFlags = v20 | 4;
                    if (effect->mLoopDelayMax != 0)
                    {
                        tmin = (float)effect->mLoopDelayMin * 0.01f;
                        tmaxa =
                            flrand(tmin, (float)effect->mLoopDelayMax * 0.01f);
                        if (tmaxa != 0.0f)
                        {
                            effect->mState = 3;
                            effect->mDelayCountdown = tmaxa;
                            break;
                        }
                    }
                }
            UpdateEffects_L50:
                effect->mState = 2;
                h2.mVal = PostEffectEventScriptCall(
                    level, effect->mScriptId, false, pakId,
                    false).mVal;
                effect->mEffectHandle = h2.mVal;
                fxset = EffectEventSys::sInst->GetActiveEffectSet(h2);
                goto UpdateEffects_L59;

            case 3:
                effect->mDelayCountdown -= delta_t;
                if (effect->mDelayCountdown > 0.0f)
                    goto UpdateEffects_L59;
                goto UpdateEffects_L50;

            case 4:
                if (effect->mGroupPtr != nullptr
                    && effect->mGroupPtr->mActive != 0)
                    goto UpdateEffects_L58;
                goto UpdateEffects_L59;

            default:
                goto UpdateEffects_L59;
            }

        UpdateEffects_L58:
            effect->mState = 2;
        UpdateEffects_L59:
            if (fxset != nullptr)
                fxset->SetPoPtr((math::Mat43*)effect);
        }
    }
}

// ea: 0x669530
void SceneManager::DebugRenderFX()
{
    if (mDebugRenderDist == 0.0f)
        return;
    int active = 0;
    int total = 0;
    const Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    math::Position3 playerPos = Player->r.currentOrigin;
    char buf[512];
    for (int bankIdx = 0; bankIdx < 99; ++bankIdx)
    {
        SceneBank* bank = mBankArray.m_elements[bankIdx];
        if (bank == nullptr)
            continue;
        InplaceVector<SceneEffect>* p_mSceneEffects = &bank->mSceneEffects;
        for (unsigned int i = 0; i < p_mSceneEffects->mSize; ++i)
        {
            ++total;
            SceneEffect* effect = &p_mSceneEffects->mList[i];
            math::Position3 pos;
            pos.v = effect->mPo.w.v;
            math::Position3 delta;
            delta.v = _mm_sub_ps(pos.v, playerPos.v);
            __m128 d2 = _mm_mul_ps(delta.v, delta.v);
            float dist = sqrtf(d2.m128_f32[0]
                               + (_mm_shuffle_ps(d2, d2, 85).m128_f32[0]
                                  + _mm_shuffle_ps(d2, d2, 170).m128_f32[0]));

            Broc::string dbg;
            sprintf(buf, "effect: %s\n", effect->mScriptId);
            dbg += buf;
            sprintf(buf, "position: %d %d %d\n",
                    (int)pos.v.m128_f32[0], (int)pos.v.m128_f32[1],
                    (int)pos.v.m128_f32[2]);
            dbg += buf;

            const char* stateStr = nullptr;
            switch (effect->mState)
            {
            case 0: stateStr = "uninitialized"; break;
            case 1: stateStr = "playing"; ++active; break;
            case 2: stateStr = "loop delay"; break;
            case 3: stateStr = "stopped"; break;
            case 4: stateStr = "dead"; break;
            default: break;
            }
            sprintf(buf, "state: %s\n", stateStr);
            dbg += buf;

            if (effect->mGroupPtr != nullptr)
            {
                sprintf(buf, "group: %s\n", effect->mGroupPtr->mName.mStr);
                dbg += buf;
                const char* groupState =
                    effect->mGroupPtr->mActive ? "enabled" : "disabled";
                sprintf(buf, "groupstate: %s\n", groupState);
                dbg += buf;
            }

            if ((effect->mFlags & 1) != 0)
            {
                sprintf(buf, "loop: %.2f - %.2f\n",
                        (float)effect->mLoopDelayMin * 0.01f,
                        (float)effect->mLoopDelayMax * 0.01f);
                dbg += buf;
            }

            if (effect->mState == 2)
            {
                sprintf(buf, "countdown: %.2f\n", effect->mDelayCountdown);
                dbg += buf;
            }

            ActiveEffectSet* fxset =
                EffectEventSys::sInst->GetActiveEffectSet(
                    Handle{effect->mEffectHandle});
            if (fxset != nullptr)
            {
                for (int j = 0; j < fxset->mEffects.m_size; ++j)
                {
                    Broc::string fxdbg = fxset->mEffects.m_elements[j]
                                             ->GetDebugString();
                    dbg += fxdbg;
                    dbg += "\n";
                }
            }
            else
            {
                dbg += "[no active fx]\n";
            }

            DebugRender::RenderSphere(pos, 16.0f,
                                      Color(1.0f, 1.0f, 0.0f, 0.5f));
            if (mDebugRenderDist >= dist)
            {
                const char* txt =
                    dbg.mBlock != nullptr ? dbg.GetBuff() : defaultFileName;
                DebugRender::RenderText3D(pos, Color(1.0f, 1.0f, 1.0f, 1.0f),
                                          1.0f, txt);
            }
        }
    }

    char summary[32];
    sprintf(summary, "Active FX: %d/%d\n", active, total);
    DebugRender::RenderText(summary, 20, 20,
                            Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, 1.0f);
}

// SceneEntity (scenemanager.cpp; mType +0x00, m_origin +0x08, m_angles +0x50)
struct SceneEntity {
    enum {
        kSceneObject_spawn_intermission = 0,
    };

    int              mType;      // +0x00
    const char*      m_classname;  // +0x04
    math::Position3  m_origin;   // +0x08
    int              m_spawnflags;  // +0x18
    uint8_t          _pad1C[0x28 - 0x1C];
    InplaceString    m_target;    // +0x28
    InplaceString    m_targetname;  // +0x2C
    uint8_t          _pad30[0x50 - 0x30];
    math::Position3  m_angles;   // +0x50
    uint8_t          _pad60[0xB0 - 0x60];
    InplaceString    m_groupName;  // +0xB0
    InplaceString    m_scriptNoteworthy;  // +0xB8
    InplaceString    m_animName;  // +0xC4
    int16_t          m_persistent_index;  // +0xCC
    uint8_t          _padCE[0xD0 - 0xCE];
    BitSet<29>       mSpecifiedFields;  // +0xD0
    InplaceVector<InplaceTreeElementKV> mKeyValuePairs;  // +0xD4
    InplaceVector<InplaceTreeElement<unsigned int, InplaceString> >
        mHashPairs;  // +0xDC
};

// ea: 0x6788F0
void SceneManager::InstanceEntities()
{
    for (int pakId = PAK_ID_MIN; pakId < PAK_ID_MAX; ++pakId)
    {
        SceneBank* bank = mBankArray.m_elements[pakId];
        if (bank == nullptr)
            continue;
        if (bank->mWorldSpawn != nullptr)
            ProcessWorldSpawn(*(WorldSpawn*)bank->mWorldSpawn);
        for (unsigned int i = 0; i < bank->mSceneEntities.mSize; ++i)
            ProcessEntity((TPakId)pakId, (int)i);
        for (unsigned int j = 0; j < bank->mVehicleNodes.mSize; ++j)
            ProcessVehicleNode((TPakId)pakId, j);
    }

    TPakId mLevelPakId = PakManager::sInst->mLevelPakId;
    SceneBank* bank = mBankArray.m_elements[mLevelPakId];
    unsigned int intermissionIndices[16];
    int count = 0;
    for (unsigned int i = 0; i < bank->mSceneEntities.mSize; ++i)
    {
        if (((SceneEntity*)bank->mSceneEntities.mList[i])->mType
            == SceneEntity::kSceneObject_spawn_intermission)
        {
            if (count < 16)
            {
                intermissionIndices[count++] = i;
            }
            else
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 174;
                AeAssert::gCurrentExpr = "m_size < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("no room left in array"))
                    __debugbreak();
            }
        }
    }
    if (count != 0)
    {
        unsigned int pick = (unsigned int)(rand() % count);
        if (pick >= 16)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        SceneEntity* ent = (SceneEntity*)
            bank->mSceneEntities.mList[intermissionIndices[pick]];
        math::Position3 pos = ent->m_origin;
        math::Position3 angles = ent->m_angles;
        int cell = R_CellForPoint(&pos);
        StreamZoneManager::sInst->mInitialPosition = pos;
        StreamZoneManager::sInst->mInitialCell = cell;
        StreamZoneManager::sInst->Update(cell, &pos, true);
        Entity* player = EntityManager::sInst->mPlayers[0];
        if (player != nullptr && dword_F6A290[0] == 2)
        {
            G_SetOrigin(player, pos);
            G_SetAngle(player, angles);
        }
    }
}

// ea: 0x6816F0
void Entity::SetScriptEventHandler(ScriptEventHandler* n)
{
    if (mScriptEventHandler == nullptr)
    {
        mScriptEventHandler = n;
        return;
    }
    AeAssert::gCurrentAuthor = AeAssert::JRS;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.h";
    AeAssert::gCurrentLine = 339;
    AeAssert::gCurrentExpr = "!mScriptEventHandler";
    if (AeAssert::IsIgnored())
    {
        mScriptEventHandler = n;
    }
    else
    {
        if (AeAssert::Assert(defaultFileName))
            __debugbreak();
        mScriptEventHandler = n;
    }
}

// ea: 0x673D60
void SceneManager::ConvertEntity(SceneEntity* source, Entity* dest)
{
    struct EntityFieldCopy {
        int srcOffset;
        int destOffset;
        int size;
        int type;
        int field;
    };
    // field table from 0x673DA2 (src, dest, size, type, specified-field bit)
    static const EntityFieldCopy kFields[29] = {
        { 4,   0x27C, 4,  0x0E, 0  },  // mClassName string
        { 8,   0x150, 12, 0x05, 1  },  // currentOrigin
        { 20,  0x270, 4,  0x0C, 2  },  // mModel
        { 24,  0x2C0, 4,  0x00, 3  },  // spawnflags
        { 28,  0x31C, 4,  0x03, 4  },
        { 32,  0x320, 4,  0x03, 5  },
        { 36,  0x28C, 4,  0x0E, 6  },  // mTarget string
        { 40,  0x284, 4,  0x0E, 7  },  // targetname string
        { 56,  0x318, 2,  0x04, 8  },  // G_NewString ushort
        { 60,  0x380, 4,  0x03, 9  },
        { 64,  0x384, 4,  0x03, 10 },
        { 68,  0x36C, 4,  0x00, 11 },
        { 72,  0x358, 4,  0x00, 12 },
        { 76,  0x360, 4,  0x00, 13 },
        { 80,  0x160, 12, 0x05, 14 },  // currentAngles
        { 96,  0x390, 12, 0x05, 15 },
        { 108, 0x314, 4,  0x03, 16 },
        { 112, 0x31C, 4,  0x03, 17 },
        { 116, 0x278, 4,  0x03, 18 },  // modelscale
        { 144, 0x3B4, 4,  0x00, 19 },
        { 148, 0x388, 4,  0x03, 20 },
        { 164, 0x36C, 4,  0x00, 21 },
        { 168, 0x3B8, 4,  0x0E, 22 },  // string
        { 176, 0x294, 4,  0x0E, 23 },  // mGroupName string
        { 184, 0x29C, 4,  0x0E, 24 },  // mScriptNoteworthy string
        { 188, 0x35C, 4,  0x00, 25 },
        { 196, 0x2A4, 4,  0x0E, 26 },  // string
        { 204, 0x3BE, 2,  0x01, 27 },
        { 200, 0x2B8, 4,  0x00, 28 },
    };

    for (int i = 0; i < 29; ++i)
    {
        const EntityFieldCopy& f = kFields[i];
        if (!source->mSpecifiedFields.Test(f.field))
            continue;
        switch (f.type)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 5:
        case 0xF:
            memcpy((char*)dest + f.destOffset, (char*)source + f.srcOffset,
                   f.size);
            break;

        case 4:
        {
            const char* v4 = *(const char**)((char*)source + f.srcOffset);
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
            AeAssert::gCurrentLine = 2491;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Is this still used? (CD)"))
                __debugbreak();
            *(unsigned short*)((char*)dest + f.destOffset) =
                G_NewString(v4);
            break;
        }

        case 0xC:
        {
            const char* v7 = *(const char**)((char*)source + f.srcOffset);
            if (v7 == nullptr)
                break;
            if (*v7 == '*')
            {
                int v8 = atoi(v7 + 1);
                if (v8 != (unsigned short)v8)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\scenemanager.cpp";
                    AeAssert::gCurrentLine = 2515;
                    AeAssert::gCurrentExpr =
                        "modelIndex == (unsigned short) modelIndex";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                *(unsigned short*)((char*)dest + 0x06) =
                    (unsigned short)v8;  // s.index.brushmodel
            }
            else if (*(const int*)v7 == 0x6F5F3373)
            {
                *(int*)((char*)dest + 0x2C4) |= 0x400;  // flags
                *(unsigned short*)((char*)dest + 0x06) =
                    (unsigned short)atoi(v7 + 7);
            }
            else
            {
                TPakId PakId = (TPakId)dest->GetPakId();
                dest->mModel = XModelManager::sInst->GetXModel(PakId, v7);
                if (dest->mModel.mValue == nullptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::ARO;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\scenemanager.cpp";
                    AeAssert::gCurrentLine = 2526;
                    AeAssert::gCurrentExpr = "dest->mModel";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(
                               "Unable to get model '%s' for entity", v7))
                        __debugbreak();
                }
                if (*(const int*)v7 == 0x6F5F34F3)
                {
                    *(unsigned short*)((char*)dest + 0x06) =
                        (unsigned short)atoi(v7 + 7);
                }
            }
            break;
        }

        case 0xE:
        {
            Broc::string v5(*(const char**)((char*)source + f.srcOffset));
            *(Broc::string*)((char*)dest + f.destOffset) = v5;
            break;
        }

        default:
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
            AeAssert::gCurrentLine = 2540;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("Unknown entity field type!"))
                __debugbreak();
            break;
        }
    }
}

// ea: 0x6787E0
void SceneManager::PostProcess(TPakId pakId)
{
    SceneBank* Bank = GetBank(pakId);
    void* mWorldSpawn = Bank->mWorldSpawn;
    if (mWorldSpawn != nullptr)
        ProcessWorldSpawn(*(WorldSpawn*)mWorldSpawn);
    for (unsigned int i = 0; i < Bank->mInstanceGroups.mSize; ++i)
    {
        ProcessInstanceGroup(pakId, &Bank->mInstanceGroups.mList[i]);
    }
    for (unsigned int v7 = 0; v7 < Bank->mStaticModels.mSize; ++v7)
    {
        ProcessStaticModel(pakId,
                           ((StaticModel*)Bank->mStaticModels.mList)[v7]);
    }
    extern int cls_state;  // ?cls_state@@3HA (cl_debug.cpp)
    if (cls_state == 0 /* CA_ACTIVE */ || cls_state == 5 /* CA_MAP_RESTART */)
    {
        for (unsigned int j = 0; j < Bank->mSceneEntities.mSize; ++j)
            ProcessEntity(pakId, (int)j);
    }
    ProcessEffects(pakId, Bank);
}

// ea: 0x673190
void SceneManager::ProcessInstanceGroup(TPakId pakId, void* groupPtr)
{
    InstanceGroup* group = (InstanceGroup*)groupPtr;
    PakHeapContext heapContext(pakId, false);
    IVPointer<XModel> model =
        XModelManager::sInst->GetXModel(pakId, group->modelName.mStr);
    ValidatePakId((TPakId)model.mPakId);
    if (model.mValue == nullptr)
        model = gDefaultXmodel;
    ValidatePakId((TPakId)model.mPakId);
    if (model.mValue == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 1635;
        AeAssert::gCurrentExpr = "model";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Model does not exist in pak"))
            __debugbreak();
        if (heapContext.mPakId != PAK_ID_INVALID)
        {
            TlSystemCallbacks::LockTlAllocsToPakHeap(heapContext.mLastState,
                                                     false);
            ae_sized_array<TPakId, 128>& stack =
                (ae_sized_array<TPakId, 128>&)PakManager::sInst
                    ->GetContextStack();
            if (stack.m_size != 0)
                stack.m_size = stack.m_size - 1;
        }
        return;
    }

    ValidatePakId((TPakId)model.mPakId);
    XModel* xmBase = model.mValue;
    XModelLod** lod = xmBase->lod;

    auto firstLod = [&]() -> XModelLod* {
        int idx = 0;
        if (xmBase->lod[0] == nullptr)
        {
            XModelLod** v = xmBase->lod;
            do
            {
                ++v;
                ++idx;
            } while (*v == nullptr);
        }
        return xmBase->lod[idx];
    };

    XModelLod* first = firstLod();
    if (first->xmodelParts != nullptr
        && first->xmodelParts->mHierarchy.mSize > 32)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 1642;
        AeAssert::gCurrentExpr = "xmBase->GetNumBones() <= 32";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("static XModel %s has too many bones.",
                                xmBase->name))
            __debugbreak();
    }

    static math::Mat43 bones[64];
    XModelGetBasePose(model, bones);
    int lastLodDist = 0;
    for (int lodIdx = 0;; ++lodIdx)
    {
        XModelLod* curLod = firstLod();
        int boneCount = 0;
        if (curLod->xmodelParts != nullptr)
            boneCount = curLod->xmodelParts->mHierarchy.mSize;
        if (lodIdx >= boneCount)
            break;

        nglMesh* mesh = curLod->xmodelParts->mMeshes.mList[lodIdx];
        if (mesh != nullptr)
        {
            ae_sized_array<nglMeshLOD, 8> lodMeshes;
            nglMeshLOD firstLodMesh;
            firstLodMesh.Mesh = nullptr;
            firstLodMesh.Range = 0.0f;
            lodMeshes.push_back(firstLodMesh);
            lodMeshes.m_elements[lodMeshes.m_size - 1].Mesh = mesh;
            for (unsigned int v31 = 0; v31 < mesh->NLODs; ++v31)
                lodMeshes.push_back(mesh->LODs[v31]);

            float prevMax = 0.0f;
            for (int i = 0; i < lodMeshes.m_size; ++i)
            {
                nglMesh* v33 = lodMeshes.m_elements[i].Mesh;
                if (v33 == nullptr)
                    continue;
                nglMeshSection* Section = v33->Sections->Section;
                InstanceListNode* node = (InstanceListNode*)
                    PakManager::sInst->MemAlloc(pakId, 0x38, false);
                InplaceVector<InstanceData>* insts =
                    (InplaceVector<InstanceData>*)&group->insts;
                unsigned int mSize = insts->mSize;
                node->instance.Create(v33, Section, (int)mSize);
                node->mMinDist = prevMax;
                float Range = (i >= lodMeshes.m_size - 1)
                                  ? 99999.0f
                                  : lodMeshes.m_elements[i + 1].Range;
                node->mMaxDist = Range;
                float sectionRadius =
                    Section->Sphere.v.m128_f32[3];
                float v39 = Range - sectionRadius;
                node->mMaxDist = v39;
                *(InstanceGroup**)((char*)node + 0x2C) = group;
                node->next = (InstanceListNode*)group->instanceList;
                group->instanceList = node;
                prevMax = v39;

                ae_sized_array<int*, 384> flags;
                flags.m_size = 0;
                if (insts->mSize != 0)
                {
                    for (unsigned int instIdx = 0;
                         instIdx < insts->mSize; ++instIdx)
                    {
                        InstanceData* inst = &insts->mList[instIdx];
                        math::Position3 origin;
                        origin.v = _mm_setr_ps(inst->cullA[0], inst->cullA[1],
                                               inst->cullA[2], 0.0f);
                        int cellNum = 0;
                        const LightGridMgr::TOC* LightGrid =
                            LightGridMgr::sInst->GetLightGrid(origin,
                                                              &cellNum);
                        float lightColor[16];
                        float lightDir[16];
                        memset(lightColor, 0, sizeof(lightColor));
                        memset(lightDir, 0, sizeof(lightDir));
                        WorldSpawn* ws = (WorldSpawn*)mWorldSpawn;
                        lightColor[12] = ws->ambient;
                        lightColor[13] = ws->ambient;
                        lightColor[14] = ws->ambient;
                        if (LightGrid != nullptr && cellNum >= 0)
                        {
                            LightGridMgr::sInst->SampleLightGrid(
                                *LightGrid, cellNum, origin,
                                (math::Mat44*)lightDir,
                                (math::Mat44*)lightColor);
                        }
                        else
                        {
                            float forward[3], right[3], up[3];
                            AngleVectors(ws->sundirection, forward, right,
                                         up);
                            VectorNormalize(forward);
                            if (lightDir[0] == 0.0f)
                            {
                                lightDir[0] = 0.0f;
                                lightDir[1] = 0.0f;
                                lightDir[2] = -1.0f;
                                lightColor[0] = ws->sundiffusecolor[0];
                                lightColor[1] = ws->sundiffusecolor[1];
                                lightColor[2] = ws->sundiffusecolor[2];
                            }
                        }

                        __m128 xaxis = _mm_setr_ps(inst->cullB[0],
                                                   inst->cullB[1],
                                                   inst->cullB[2], 0.0f);
                        __m128 yaxis = _mm_setr_ps(inst->worldPos[0],
                                                   inst->worldPos[1],
                                                   inst->worldPos[2], 0.0f);
                        __m128 Mx = bones[0].x.v;
                        __m128 My = bones[0].y.v;
                        __m128 Mz = bones[0].z.v;
                        __m128 Mt = bones[1].x.v;
                        math::Mat43 instMat;
                        instMat.x.v = _mm_add_ps(
                            _mm_add_ps(
                                _mm_mul_ps(_mm_shuffle_ps(xaxis, xaxis, 0),
                                           Mx),
                                _mm_mul_ps(_mm_shuffle_ps(xaxis, xaxis, 0x55),
                                           My)),
                            _mm_mul_ps(_mm_shuffle_ps(xaxis, xaxis, 0xAA),
                                       Mz));
                        instMat.y.v = _mm_add_ps(
                            _mm_add_ps(
                                _mm_mul_ps(_mm_shuffle_ps(yaxis, yaxis, 0),
                                           Mx),
                                _mm_mul_ps(_mm_shuffle_ps(yaxis, yaxis, 0x55),
                                           My)),
                            _mm_mul_ps(_mm_shuffle_ps(yaxis, yaxis, 0xAA),
                                       Mz));
                        instMat.z.v = _mm_add_ps(
                            _mm_add_ps(
                                _mm_mul_ps(_mm_shuffle_ps(origin.v, origin.v,
                                                           0),
                                           Mx),
                                _mm_mul_ps(_mm_shuffle_ps(origin.v, origin.v,
                                                           0x55),
                                           My)),
                            _mm_add_ps(
                                _mm_mul_ps(_mm_shuffle_ps(origin.v, origin.v,
                                                           0xAA),
                                           Mz),
                                Mt));

                        __m128 sphere = v33->Sphere.v;
                        __m128 cross = _mm_sub_ps(
                            _mm_mul_ps(_mm_shuffle_ps(xaxis, xaxis, 0x09),
                                       _mm_shuffle_ps(yaxis, yaxis, 0x12)),
                            _mm_mul_ps(_mm_shuffle_ps(xaxis, xaxis, 0x12),
                                       _mm_shuffle_ps(yaxis, yaxis, 0x09)));
                        __m128 cellPos = _mm_add_ps(
                            _mm_add_ps(
                                _mm_mul_ps(
                                    _mm_shuffle_ps(sphere, sphere, 0), xaxis),
                                _mm_mul_ps(
                                    _mm_shuffle_ps(sphere, sphere, 0x55),
                                    yaxis)),
                            _mm_add_ps(
                                _mm_mul_ps(
                                    _mm_shuffle_ps(sphere, sphere, 0xAA),
                                    cross),
                                origin.v));
                        math::Position3 cellPos3;
                        cellPos3.v = cellPos;
                        int cell = R_CellForPoint(&cellPos3);
                        node->instance.Add(instMat, inst->pad,
                                           *(const math::Mat44*)lightDir,
                                           *(const math::Mat44*)lightColor,
                                           flags, cell);
                    }
                }

                node->instance.Finalize(flags);
                unsigned int v59 = 4 * insts->mSize;
                void* v62;
                if (pakId != PAK_ID_INVALID)
                {
                    PakFile* slot = PakManager::sInst->mSlots[pakId];
                    if (slot == nullptr)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\PakManager.cpp";
                        AeAssert::gCurrentLine = 2583;
                        AeAssert::gCurrentExpr =
                            "PakManager::Inst()->IsValid( id )";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(
                                   "bad pak id for MemAlloc"))
                            __debugbreak();
                    }
                    PakFile* v61 = pakId > 0x62 ? nullptr : slot;
                    v62 = v61 ? v61->MemAlloc(0x10, v59, true) : nullptr;
                }
                else
                {
                    v62 = nullptr;
                }
                if (v62 == nullptr)
                    v62 = mem_heap_malloc(0x10, v59);
                *(void**)((char*)node + 0x30) = v62;
                for (unsigned int v65 = 0; v65 < insts->mSize; ++v65)
                {
                    if (v65 >= 0x180)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "../ae\\core/ae_array.h";
                        AeAssert::gCurrentLine = 154;
                        AeAssert::gCurrentExpr =
                            "idx >= 0 && idx < _CAPACITY";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("out of bounds"))
                            __debugbreak();
                    }
                    ((int**)v62)[v65] = flags.m_elements[v65];
                }
            }
        }
    }

    if (heapContext.mPakId != PAK_ID_INVALID)
    {
        TlSystemCallbacks::LockTlAllocsToPakHeap(heapContext.mLastState,
                                                 false);
        ae_sized_array<TPakId, 128>& stack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst
                ->GetContextStack();
        if (stack.m_size != 0)
            stack.m_size = stack.m_size - 1;
    }
}

// ea: 0x66D670
void SceneManager::ProcessStaticModel(TPakId pakId, StaticModel& model)
{
    XModel* xmodel = model.xmodel;
    model.pakId = pakId;
    if (xmodel == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 1524;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Static model without model!"))
            __debugbreak();
        return;
    }

    if (xmodel->resolved == nullptr)
    {
        IVPointer<XModel> xmodptr =
            XModelManager::sInst->GetXModel(pakId, xmodel->name);
        ValidatePakId((TPakId)xmodptr.mPakId);
        if (xmodptr.mValue == nullptr)
            xmodptr = gDefaultXmodel;
        ValidatePakId((TPakId)xmodptr.mPakId);
        if (xmodptr.mValue != nullptr)
        {
            ValidatePakId((TPakId)xmodptr.mPakId);
            xmodel->resolved = xmodptr.mValue;
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
            AeAssert::gCurrentLine = 1510;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning(
                       "scene contains %s but mesh is not\nin pakfile",
                       xmodel->name))
                __debugbreak();
            return;
        }
    }

    model.xmodel = xmodel->resolved;
    if (model.xmodel == nullptr)
        return;

    BoundingBox bounds;
    if (model.instance == 0)
    {
        R_GetXModelBounds(model.xmodel, model.axis, bounds);
        math::Position3 o = native_to_cdl_pos3(model.origin);
        bounds.vmin.v = _mm_add_ps(bounds.vmin.v, o.v);
        bounds.vmax.v = _mm_add_ps(bounds.vmax.v, o.v);
        ((world_t*)tr.world)->bspTree = g_bspTree;
        BspNode* node =
            &((world_t*)tr.world)->bspTree->mNodes.mList[0];
        R_FilterModelIntoCells_r((world_t*)tr.world, node, &model,
                                 bounds.vmin, bounds.vmax);
        model.absmin[0] = bounds.vmin.v.m128_f32[0];
        model.absmin[1] = bounds.vmin.v.m128_f32[1];
        model.absmin[2] = bounds.vmin.v.m128_f32[2];
        model.absmax[0] = bounds.vmax.v.m128_f32[0];
        model.absmax[1] = bounds.vmax.v.m128_f32[1];
        model.absmax[2] = bounds.vmax.v.m128_f32[2];
    }

    XModel* v14 = model.xmodel;
    XModelLod** lod = v14->lod;
    int v16 = 0;
    if (v14->lod[0] == nullptr)
    {
        XModelLod** v17 = v14->lod;
        do
        {
            ++v17;
            ++v16;
        } while (*v17 == nullptr);
    }
    XModelLod* v18 = v14->lod[v16];
    int nbones = PAK_ID_MIN;
    if (v18->xmodelParts != nullptr)
    {
        if (*lod == nullptr)
        {
            XModelLod* v20;
            do
            {
                v20 = lod[1];
                ++lod;
                ++nbones;
            } while (v20 == nullptr);
        }
        nbones = v14->lod[nbones]->xmodelParts->mHierarchy.mSize;
    }

    static math::Mat43 bones[64];
    if (nbones > 64)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
        AeAssert::gCurrentLine = 1565;
        AeAssert::gCurrentExpr = "nbones <= MAX_BONES";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("static XModel %s has too many bones.",
                                model.xmodel->name))
            return;
    }
    else
    {
        IVPointer<XModel> v27;
        v27.mValue = model.xmodel;
        v27.mPakId = model.pakId;
        XModelGetBasePose(v27, bones);
        math::Position3 mins;
        math::Position3 maxs;
        if (XModelGetStaticBounds(v27, model.axis, mins, maxs, bones,
                                  nbones) != 0)
        {
            math::Position3 origin;
            origin.v = _mm_setr_ps(model.origin[0], model.origin[1],
                                   model.origin[2], 0.0f);
            __m128 scale = _mm_set1_ps(model.scale);
            __m128 amin = _mm_add_ps(_mm_mul_ps(mins.v, scale), origin.v);
            __m128 amax = _mm_add_ps(_mm_mul_ps(maxs.v, scale), origin.v);
            model.absmin[0] = amin.m128_f32[0];
            model.absmin[1] = amin.m128_f32[1];
            model.absmin[2] = amin.m128_f32[2];
            model.absmax[0] = amax.m128_f32[0];
            model.absmax[1] = amax.m128_f32[1];
            model.absmax[2] = amax.m128_f32[2];
            if (model.instance == 0)
            {
                math::Position3 bmin;
                bmin.v = _mm_setr_ps(model.absmin[0], model.absmin[1],
                                     model.absmin[2], 0.0f);
                math::Position3 bmax;
                bmax.v = _mm_setr_ps(model.absmax[0], model.absmax[1],
                                     model.absmax[2], 0.0f);
                bounds.accumulate(bmin);
                bounds.accumulate(bmax);
                model.absmin[0] = bounds.vmin.v.m128_f32[0];
                model.absmin[1] = bounds.vmin.v.m128_f32[1];
                model.absmin[2] = bounds.vmin.v.m128_f32[2];
                model.absmax[0] = bounds.vmax.v.m128_f32[0];
                model.absmax[1] = bounds.vmax.v.m128_f32[1];
                model.absmax[2] = bounds.vmax.v.m128_f32[2];
            }
            if (model.xmodel->contents != 0)
                CM_LinkStaticModel(&model);
        }
    }
}

// ea: 0x676C50
void SceneManager::ProcessEntity(TPakId pakId, int entIdx)
{
    SceneBank* Bank = GetBank(pakId);
    SceneEntity* scnEnt =
        (SceneEntity*)Bank->mSceneEntities.mList[entIdx];
    const char* auto_thread = nullptr;
    ScriptEventHandler* event_handler = nullptr;
    int transparent = 0;
    int noShadow = 0;
    Broc::string exploderType;
    Broc::string exploderId;

    InplaceVector<InplaceTreeElementKV>* p_mKeyValuePairs =
        &scnEnt->mKeyValuePairs;
    for (unsigned int v5 = 0; v5 < p_mKeyValuePairs->mSize; ++v5)
    {
        InplaceTreeElementKV* v8 = &p_mKeyValuePairs->mList[v5];
        if (_stricmp(v8->mKey.mStr, "exploder_type") == 0)
            exploderType = v8->mVal.mStr;
        if (_stricmp(v8->mKey.mStr, "exploder_id") == 0)
            exploderId = v8->mVal.mStr;
    }

    int persIndex = scnEnt->m_persistent_index;
    bool wasExploded = false;
    if (persIndex != -1)
    {
        bool v10 = false;
        if (CheckpointMgr::sInst->mCheckpointSaveExists
            && CheckpointMgr::sInst->mUsingCheckpoints)
        {
            if (mPersistantStorage != nullptr)
                v10 = mPersistantStorage->mList[persIndex] == 2;
        }
        if ((mPersistantStorage != nullptr
             && mPersistantStorage->mList[persIndex] != 0)
            || v10)
        {
            if (exploderId.mBlock != nullptr)
            {
                const char* v13 = (const char*)(exploderId.mBlock + 1);
                if (exploderId.mBlock != (Broc::string::Block*)-12
                    && *v13 != 0)
                {
                    int v14 = atoi(v13);
                    if (v14 != 0 && exploderType.mBlock
                                    != (Broc::string::Block*)-12
                        && CheckpointMgr::sInst->ExploderCheckpointExploded(
                            v14))
                    {
                        const char* v15 = (exploderId.mBlock != nullptr)
                                              ? (const char*)
                                                    (exploderId.mBlock + 1)
                                              : defaultFileName;
                        IVPointer<Destructible> d =
                            DestructibleBankManager_GetDestructible(
                                nullptr, pakId, v15);
                        ValidatePakId((TPakId)persIndex);
                        *(int*)((char*)d.mValue + 36) |= 0x400000;
                        if (scnEnt->m_target.mStr != nullptr)
                            PathNodeMgr::sInst->SetCoverNodeStatus(
                                Broc::string(scnEnt->m_target.mStr), 1);
                    }
                }
            }
            goto ProcessEntity_Done;
        }
    }

    InplaceVector<InplaceTreeElementKV>* keyValuePairs =
        &scnEnt->mKeyValuePairs;
    static unsigned int transparent_hash;
    static unsigned int noshadow_hash;
    static unsigned int modelscale_hash;
    static int hash_init = 0;
    if ((hash_init & 1) == 0)
    {
        hash_init |= 1;
        transparent_hash = AeHash("transparent");
    }
    if ((hash_init & 2) == 0)
    {
        hash_init |= 2;
        noshadow_hash = AeHash("noshadow");
    }
    if ((hash_init & 4) == 0)
    {
        hash_init |= 4;
        modelscale_hash = AeHash("modelscale");
    }

    for (unsigned int v17 = 0; v17 < scnEnt->mHashPairs.mSize; ++v17)
    {
        InplaceTreeElement<unsigned int, InplaceString>* v19 =
            &scnEnt->mHashPairs.mList[v17];
        if (v19->mKey == transparent_hash)
            transparent = (_stricmp(v19->mVal.mStr, "1") == 0);
        else if (v19->mKey == noshadow_hash)
            noShadow = (_stricmp(v19->mVal.mStr, "1") == 0);
    }

    ScriptEventHandler* v20 = event_handler;
    for (unsigned int i = 0; i < keyValuePairs->mSize; ++i)
    {
        InplaceTreeElementKV* mList = &keyValuePairs->mList[i];
        const char* mStr = mList->mKey.mStr;
        if (_stricmp(mStr, "auto_thread") == 0)
        {
            if (_stricmp(mList->mVal.mStr, "_load::main_trigger") != 0)
                auto_thread = mList->mVal.mStr;
        }
        else if (_strnicmp(mStr, "on_", 3) == 0)
        {
            if (v20 == nullptr)
            {
                ScriptEventHandler* v26 = (ScriptEventHandler*)
                    ((PoolAllocator*)ScriptEventHandler_sAllocator)
                        ->Allocate(0x44, false);
                event_handler = v26;
                v20 = v26;
            }
            HashString h;
            h.mHash = HashString::CalcHash(mStr + 3);
            v20->AddEvent(h, mList->mVal.mStr);
        }
    }

    Entity* v32 = G_Spawn(pakId);
    if (event_handler != nullptr)
        v32->SetScriptEventHandler(event_handler);
    if (transparent != 0)
        *(float*)((char*)&v32->GetRenderEntity() + 0xF0) = 0.5f;
    if (noShadow != 0)
        *(unsigned char*)((char*)&v32->GetRenderEntity() + 0xFB) |= 2;
    v32->mPersistentIndex = (int16_t)persIndex;
    ConvertEntity(scnEnt, v32);
    v32->mClassNameHash.mHash = HashString(v32->mClassName).mHash;
    G_ReplaceSpawnVars(scnEnt->mHashPairs);
    if (!ValidForGametype())
    {
        G_FreeEntity(v32, 0);
        goto ProcessEntity_Done;
    }

    if (exploderId.mBlock != nullptr
        && exploderId.mBlock != (Broc::string::Block*)-12
        && *((const char*)(exploderId.mBlock + 1)) != 0)
    {
        int v37 = atoi((const char*)(exploderId.mBlock + 1));
        if (v37 != 0)
        {
            const char* v38 = (exploderType.mBlock != nullptr
                                   && exploderType.mBlock
                                          != (Broc::string::Block*)-12)
                                  ? (const char*)(exploderType.mBlock + 1)
                                  : defaultFileName;
            if (CheckpointMgr::sInst->PrecludeExploderPiece(v38, v37))
            {
                const char* v39 =
                    (exploderId.mBlock != nullptr)
                        ? (const char*)(exploderId.mBlock + 1)
                        : defaultFileName;
                TPakId mPakId = (TPakId)v32->mPakId;
                if (mPakId == PAK_ID_INVALID)
                    mPakId = PakManager::sInst->mLevelPakId;
                IVPointer<Destructible> d =
                    DestructibleBankManager_GetDestructible(nullptr, mPakId,
                                                            v39);
                ValidatePakId((TPakId)persIndex);
                *(int*)((char*)d.mValue + 36) |= 0x400000;
                Destructible::InvalidateCoverNode(v32);
                G_FreeEntity(v32, 0);
                goto ProcessEntity_Done;
            }
        }
    }

    if (CheckpointMgr::sInst->mCheckpointSaveExists
        && CheckpointMgr::sInst->mUsingCheckpoints)
        CheckpointMgr::sInst->RestoreSceneEntity(v32);
    dont_delete = true;
    no_really_delete_it = false;
    G_CallEntitySpawnFunction(v32);
    dont_delete = false;
    if (no_really_delete_it)
    {
        G_FreeEntity(v32, 0);
        goto ProcessEntity_Done;
    }
    G_SetOrigin(v32, v32->r.currentOrigin);
    G_SetAngle(v32, v32->r.currentAngles);
    BrocInitEntity(
        v32,
        (const InplaceVector<InplaceTreeElement<InplaceString, InplaceString>>*)
            keyValuePairs);
    if (auto_thread != nullptr)
        BrocAddEntityThread(v32, auto_thread);
    v32->ExecScriptHandler(hash_const.spawned, nullptr);
    if (exploderId.mBlock != nullptr)
    {
        const char* v41 = (exploderId.mBlock != nullptr)
                              ? (const char*)(exploderId.mBlock + 1)
                              : defaultFileName;
        TPakId mLevelPakId = (TPakId)v32->mPakId;
        if (mLevelPakId == PAK_ID_INVALID)
            mLevelPakId = PakManager::sInst->mLevelPakId;
        IVPointer<Destructible> d =
            DestructibleBankManager_GetDestructible(nullptr, mLevelPakId,
                                                    v41);
        Destructible* mValue = d.mValue;
        v32->mDestructible.mValue = mValue;
        v32->mDestructible.mPakId = d.mPakId;
        *(int*)((char*)v32 + 0x2B8) = 1;  // takedamage
        if (d.mPakId != PAK_ID_INVALID
            && PakManager::sInst->mSlots[d.mPakId] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 236;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("bad/old pak id"))
                __debugbreak();
        }
        if (mValue != nullptr)
        {
            Destructible* v47 = v32->mDestructible.mValue;
            ValidatePakId((TPakId)v32->mDestructible.mPakId);
            v47->Initialize(v32, true);
            const char* v48 =
                (exploderType.mBlock != nullptr)
                    ? (const char*)(exploderType.mBlock + 1)
                    : defaultFileName;
            Destructible* v49 = v32->mDestructible.mValue;
            ValidatePakId((TPakId)v32->mDestructible.mPakId);
            v49->AddPiece(v32, v48);
        }
    }

ProcessEntity_Done:
    ;
}

// ea: 0x66DCA0
void SceneManager::ProcessVehicleNode(TPakId pakId, unsigned int nodeIdx)
{
    SceneBank* Bank = GetBank(pakId);
    VehicleNode* v4 = &Bank->mVehicleNodes.mList[nodeIdx];
    vehicle_node_t* info_vehicle_node = SP_create_info_vehicle_node();
    vehicle_node_t* node = info_vehicle_node;
    if (v4->name.mStr != nullptr)
        info_vehicle_node->mName = v4->name.mStr;
    if (v4->target.mStr != nullptr)
        info_vehicle_node->mTarget = v4->target.mStr;
    if (v4->script_noteworthy.mStr != nullptr)
        info_vehicle_node->script_noteworthy = v4->script_noteworthy.mStr;
    unsigned int v8 =
        *(unsigned int*)((char*)info_vehicle_node + 0x3C);
    info_vehicle_node->origin[0] = v4->origin[0];
    info_vehicle_node->origin[1] = v4->origin[1];
    info_vehicle_node->origin[2] = v4->origin[2];
    info_vehicle_node->angles[0] = v4->angles[0];
    info_vehicle_node->angles[1] = v4->angles[1];
    info_vehicle_node->angles[2] = v4->angles[2];
    info_vehicle_node->speed = v4->speed;
    *(unsigned int*)((char*)info_vehicle_node + 0x3C) =
        (v8 ^ ((unsigned int)v4->rotated << 28)) & 0x30000000 ^ v8;
    info_vehicle_node->lookAhead = v4->lookAhead;

    unsigned int i = 0;
    if (v4->mKeyValuePairs.mSize != 0)
    {
        while (1)
        {
            InplaceTreeElementKV* kv =
                &v4->mKeyValuePairs.mList[i];
            const char* v12 = kv->mKey.mStr;
            const char* v13 = kv->mVal.mStr;
            unsigned int v14 = AeHash(v12);
            int val_int = atoi(v13);
            float val_float = (float)atof(v13);
            unsigned int nodeHandle =
                (unsigned int)PathNodeMgr::sInst->GetVehicleNodeIndex(node);
            if (_stricmp(v12, "friendlywait") == 0
                || _stricmp(v12, "playerhasbeenhere") == 0
                || _stricmp(v12, "detoured") == 0
                || _stricmp(v12, "detourstart") == 0
                || _stricmp(v12, "detourpath") == 0
                || _stricmp(v12, "endswitch") == 0
                || _stricmp(v12, "startswitch") == 0)
            {
                gpBrocAPI->mBrocExports.mSetVNodeField_int(
                    nodeHandle, v14, val_int);
            }
            else if (_stricmp(v12, "script_delay") == 0)
            {
                gpBrocAPI->mBrocExports.mSetVNodeField_float(
                    nodeHandle, v14, val_float);
            }
            else if (_stricmp(v12, "crash_path_target") == 0
                     || _stricmp(v12, "derailed") == 0)
            {
                gpBrocAPI->mBrocExports.mSetVNodeField_int(
                    nodeHandle, v14, val_int);
            }
            else if (_stricmp(v12, "script_uniquename") == 0
                     || _stricmp(v12, "groupname") == 0)
            {
                Broc::string v18(v13);
                gpBrocAPI->mBrocExports.mSetVNodeField_string(
                    nodeHandle, v14, v18);
            }
            else
            {
                char tmpstr[128];
                sprintf(tmpstr,
                        "vehiclenode field KEY=%s, VAL=%s) will be inaccessable",
                        v12, v13);
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\scenemanager.cpp";
                AeAssert::gCurrentLine = 2113;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(tmpstr))
                    __debugbreak();
            }
            if (++i >= v4->mKeyValuePairs.mSize)
                break;
        }
    }
    info_vehicle_node = node;
    if (info_vehicle_node->mName.mBlock == nullptr
        || info_vehicle_node->mName.mBlock
               == (Broc::string::Block*)-12
        || *(char*)((char*)info_vehicle_node->mName.mBlock + 0x0C) == 0)
    {
        Com_Error((errorParm_t)2 /* ERR_DROP */,
                  "Vehicle path node( %f, %f, %f ) found with no name\n\n",
                  info_vehicle_node->origin[0],
                  info_vehicle_node->origin[1],
                  info_vehicle_node->origin[2]);
    }
    float speed = info_vehicle_node->speed;
    if (speed >= 0.0f)
        info_vehicle_node->speed = speed * 17.6f;
}

// ea: 0x6786A0
SceneManager::SceneManager()
{
    AssetBankSet::sBankArray.push_back((AssetBankSet*)this);
    mSceneEffectGroups = nullptr;
    mPersistantStorage = nullptr;
    mWorldSpawn = nullptr;
    mEffectCount = 0;
    mEffectDelayFrames = 0;
    mLoadedIdsCount = 0;
    mDebugRenderDist = 0.0f;
    mDebugRenderEnts = false;
    mDebugRenderLights = false;
    DebugRender_AddRenderer(DebugRender_sInst,
                            (void*)&SceneManager::SingletonDebugRender);
    Cmd_AddCommand("SceneFX", Console_ToggleSceneFX);
    Cmd_AddCommand("RenderEnts", Console_ToggleRenderEnts);
    Cmd_AddCommand("RenderLights", Console_ToggleRenderLights);
    mPlayerFootstepMaterial = nullptr;
    mPlayerFootstepNumMatches = 1;
    for (unsigned int i = 0; i < 99; ++i)
        mBankArray.m_elements[i] = nullptr;
}

// ea: 0x004DD0F0
void SceneManager::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SceneManager.h";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }

    SceneManager* result = static_cast<SceneManager*>(
        mem_heap_malloc_ctx(0x1B8u, 4, "pak",
                            "c:\\cod\\code\\game\\SceneManager.h", 16));
    if (result != nullptr)
    {
        result = new (result) SceneManager();
        sInst = result;
    }
    else
    {
        sInst = nullptr;
    }
}

// ea: 0x004DD1F0
void SceneManager::DeleteInst()
{
    if (sInst == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SceneManager.h";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }

    if (sInst != nullptr)
        delete sInst;
    sInst = nullptr;
}

// ea: 0x675E10
SceneManager::~SceneManager()
{
    AssetBankSetRemoveSelf(this);
}

// ea: 0x687880
void SceneManager::SingletonDebugRender()
{
    SceneManager::sInst->DebugRender();
}

// ea: 0x675E20
void SceneManager::DebugRender()
{
    if (mPlayerFootstepNumMatches == 0)
    {
        const char* material = (const char*)mPlayerFootstepMaterial;
        char* v2 = va("no sound for footstep: %s", material);
        DebugRender::RenderText(v2, 200, 250, Color(1.0f, 1.0f, 1.0f, 1.0f),
                                0.0f, 1.0f);
    }
    DebugRenderFX();
    DebugRenderEnts();
    DebugRenderLights();
}

// ea: 0x668B30
void Console_ToggleSceneFX()
{
    if (Cmd_Argc() > 1)
    {
        float v4 = (float)atof(Cmd_Argv(1));
        if (v4 == -1.0f)
        {
            if (SceneManager::sInst->mDebugRenderDist != 0.0f)
            {
                SceneManager::sInst->mDebugRenderDist = 0.0f;
                return;
            }
        }
        else if (v4 != 1.0f)
        {
            SceneManager::sInst->mDebugRenderDist = v4;
            return;
        }
        SceneManager::sInst->mDebugRenderDist = 500.0f;
    }
    else
    {
        float v3 = 0.0f;
        if (SceneManager::sInst->mDebugRenderDist == 0.0f)
            v3 = 500.0f;
        SceneManager::sInst->mDebugRenderDist = v3;
    }
}

// ea: 0x668BF0
void Console_ToggleRenderEnts()
{
    SceneManager::sInst->mDebugRenderEnts =
        !SceneManager::sInst->mDebugRenderEnts;
}

// ea: 0x668C10
void Console_ToggleRenderLights()
{
    SceneManager::sInst->mDebugRenderLights =
        !SceneManager::sInst->mDebugRenderLights;
}

// ea: 0x665AB0
void SceneManager::ProcessWorldSpawn(const WorldSpawn& worldspawn)
{
    mWorldSpawn = (void*)&worldspawn;
    SV_SetConfigstring(2, "cod-sp");
    SV_SetConfigstring(3, worldspawn.ambienttrack.mStr);
    SV_SetConfigstring(4, worldspawn.message.mStr);
    const char* mStr = worldspawn.gravity.mStr;
    if (mStr == nullptr)
        mStr = "800";
    Cvar_Set("g_gravity", mStr);
    const char* v4 = worldspawn.northyaw.mStr;
    if (v4 == nullptr)
        v4 = "0";
    SV_SetConfigstring(11, v4);
    EntityMin* mWorld = EntityManager::sInst->mWorld;
    mWorld->mClassName = str_const.worldspawn;
    mWorld->mClassNameHash = HashString(mWorld->mClassName);
    UpdateEntityHash(reinterpret_cast<Entity*>(mWorld));
}

// apk texture helpers (ngl/streamer)
extern bool nglCanReleaseTexture(nglTexture* Tex);  // ngl_dx_tex

// ea: 0x665B60
void cdDestroyApk(apk::apkFile* file)
{
    apk::apkDeleteFile(file);
}

// ea: 0x665C10
void cdDestroyTexture(nglTexture* tex)
{
    apk::apkDeleteFile(tex->File);
}

unsigned char* gCharSkel = (unsigned char*)-1;  // ?gCharSkel@@3PAEA @ 0xF59314

// ea: 0x665D70
void cdLoadCharSkelInplace(void* data)
{
    if (gCharSkel != (unsigned char*)-1)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\apkSupport.cpp";
        AeAssert::gCurrentLine = 371;
        AeAssert::gCurrentExpr = "gCharSkel == (uint8*)0xFFFFFFFF";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("character skeleton is already loaded!"))
            __debugbreak();
    }
    gCharSkel = nullptr;
    apk::apkLoadFileInPlace(data, true);
}

// ea: 0x665B70
nglTexture* cdLoadTexureInplace(void* data)
{
    apk::apkFile* FileInPlace = apk::apkLoadFileInPlace(data, true);
    if (FileInPlace != nullptr)
    {
        tlFixedString name("image");
        int SectionIndex = FileInPlace->GetSectionIndex(name);
        apk::apkFileEntry* FirstFile =
            FileInPlace->GetFirstFile(0x584554u);
        return (nglTexture*)FirstFile->GetData(FileInPlace, SectionIndex,
                                               true);
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\apkSupport.cpp";
    AeAssert::gCurrentLine = 106;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("Problem loading texture"))
        __debugbreak();
    return nullptr;
}

// ea: 0x665C30
void cdDeleteTextureCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                             void* UserData)
{
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglTexture* Data = (nglTexture*)Entry->GetData(File, SectionIndex, true);
    nglTexture* v4 = Data;
    if ((Data->Flags & 8) != 0)
    {
        if (!nglCanReleaseTexture(Data))
        {
            tlWarning("NGL: Texture %s destroyed while still referenced by the async renderer.\n",
                      v4->FileName->str);
            ngliWaitForResource();
        }
        ngliUnloadTexture(File, Entry);
    }
}

// ea: 0x665FE0
void* PakFile::MemAlloc(unsigned int align, unsigned int size,
                        bool search_prereqs)
{
    unsigned int v4 = size;
    if (size == (unsigned int)-1)
        v4 = 1024;
    if (mState == (EState)3
        || (mState == (EState)2 && mLoadingState != (ELoadingState)4))
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 2028;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("Trying to allocate from unloading pakfile!"))
            __debugbreak();
        mem_break();
        return nullptr;
    }
    int v7 = 0;
    if (mHeapList.m_size <= 0)
        goto LABEL_10;
    while (1)
    {
        mem_heap* v8 = ae_array_get(mHeapList, v7);
        if (v4 + v8->used_byte < v8->size)
        {
            void* result = mem_heap_malloc(v8, align, v4);
            if (result != nullptr)
                return result;
        }
        if (++v7 >= mHeapList.m_size)
            goto LABEL_10;
    }
LABEL_10:
    if (search_prereqs && mPrereqHeaps.m_size > 0)
    {
        for (int v10 = 0; v10 < mPrereqHeaps.m_size; ++v10)
        {
            PakFile* v12 = ae_array_get(mPrereqHeaps, v10);
            void* result =
                v12->MemAlloc(align, v4, true);
            if (result != nullptr)
                return result;
        }
    }
    return nullptr;
}

// ea: 0x666110
bool PakFile::MemFree(void* ptr, bool search_prereqs)
{
    if (ptr == nullptr)
        return true;
    int v5 = 0;
    mem_heap* v6 = nullptr;
    if (mHeapList.m_size <= 0)
        goto LABEL_8;
    while (1)
    {
        v6 = ae_array_get(mHeapList, v5);
        if (v6 != nullptr && ptr >= v6->start && ptr < v6->end)
            break;
        if (++v5 >= mHeapList.m_size)
            goto LABEL_8;
    }
    mem_heap_free(v6, ptr);
    return true;
LABEL_8:
    if (!search_prereqs)
        return false;
    if (mPrereqHeaps.m_size <= 0)
        return false;
    for (int v7 = 0; v7 < mPrereqHeaps.m_size; ++v7)
    {
        PakFile* v9 = ae_array_get(mPrereqHeaps, v7);
        if (v9->MemFree(ptr, true))
            return true;
    }
    return false;
}

// ea: 0x6661D0
bool PakFile::IsInPakHeap(void* pPtr)
{
    unsigned int v3 = 0;
    if (mHeapList.m_size <= 0)
        return false;
    while (1)
    {
        mem_heap* v4 = ae_array_get(mHeapList, (int)v3);
        if (pPtr >= v4->start && pPtr < v4->end)
            break;
        if (++v3 >= (unsigned int)mHeapList.m_size)
            return false;
    }
    return true;
}

// ea: 0x666270
const PakHeader::Section* PakFile::GetSection(const char* type) const
{
    PakHeader* mHeader = this->mHeader;
    unsigned int v4 = 0;
    if (mHeader->numSections != 0)
    {
        while (1)
        {
            if (strncmp(type, mHeader->sections[v4].name, 8) == 0)
                return &mHeader->sections[v4];
            ++v4;
            if (v4 >= mHeader->numSections)
                goto LABEL_5;
        }
    }
LABEL_5:
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
    AeAssert::gCurrentLine = 2270;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("no section with this name: %s in %s", type,
                             mPath.mBuff))
        __debugbreak();
    return nullptr;
}

// ea: 0x666320
void* PakFile::PakReadDataAsync(unsigned char* buffer,
                                unsigned char* uncompressedSize,
                                unsigned int compressedSize,
                                unsigned int offset, unsigned int isHeader)
{
    if (uncompressedSize == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 2280;
        AeAssert::gCurrentExpr = "buffer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Out of memory!"))
            __debugbreak();
    }
    nflRequestID v8 = nflReadFileAsyncWithCallBack(
        mFileId, isHeader, uncompressedSize, compressedSize, codNflCallback);
    nflSetRequestPriority(v8, (nflPriority)NFL_PRIORITY_LOWEST);
    *(unsigned int*)buffer = (unsigned int)v8;
    return buffer;
}

// ea: 0x665EC0
void PakFile::GetHeapUsage(int& used, int& size) const
{
    used = 0;
    size = 0;
    for (int i = 0; i < mHeapList.m_size; ++i)
    {
        if (i >= 0xC)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 148;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        mem_heap* v5 = mHeapList.m_elements[i];
        used += v5->used_byte;
        size += v5->size;
    }
}

// ea: 0x665F60
mem_heap* PakFile::CreateHeap(unsigned char* heap_start,
                              unsigned int heap_size)
{
    if (heap_size > 0x2000 && mHeapList.m_size < 11)
    {
        unsigned char* v4 = heap_size + heap_start;
        unsigned char* v5 =
            (unsigned char*)(((uintptr_t)heap_start + 31) & 0xFFFFFFE0u);
        mem_heap* v6 =
            (mem_heap*)gPakMemHeapAllocator->Allocate(0x49Cu, false);
        mem_heap_create(v6, v5, v4, nullptr);
        mHeapList.push_back(v6);
        tlPrintf("pak: heap is created at %X end %X size %i\n",
                 v5, v4, v4 - v5);
        return v6;
    }
    return nullptr;
}

// ea: 0x66BD80
void DecodeHeap(const char* name, unsigned char* data, unsigned int size,
                TPakId pakId, PakFile* pak)
{
    (void)name; (void)pakId;
    int m_size = pak->mHeapList.m_size;
    mem_heap* v6 =
        m_size != 0 ? pak->mHeapList.m_elements[m_size - 1] : nullptr;
    mem_heap* Heap = pak->CreateHeap(data, size);
    if (Heap != nullptr && v6 != nullptr)
    {
        Heap->reserve = v6->reserve;
        v6->reserve = Heap;
    }
}

// ea: 0x66B040
PakHeapContext::PakHeapContext(TPakId id, bool once)
{
    mPakId = id;
    if (id != PAK_ID_INVALID)
    {
        if (PakManager::sInst->mSlots[id] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
            AeAssert::gCurrentLine = 2417;
            AeAssert::gCurrentExpr = "PakManager::Inst()->IsValid( id )";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("bad pak id for alloc"))
                __debugbreak();
        }
        mLastState = TlSystemCallbacks::LockTlAllocsToPakHeap(true, once);
        ae_sized_array<TPakId, 128>& ContextStack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
        ContextStack.push_back(id);
    }
}

// ea: 0x66F060
PakHeapContext::~PakHeapContext()
{
    if (mPakId != PAK_ID_INVALID)
    {
        TlSystemCallbacks::LockTlAllocsToPakHeap(mLastState, false);
        ae_sized_array<TPakId, 128>& ContextStack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
        int m_size = ContextStack.m_size;
        if (m_size != 0)
            ContextStack.m_size = m_size - 1;
    }
}

// ea: 0x6720B0
void AddGlowSprite(float* pos, float rad, unsigned int col)
{
    GlowSprites s;
    s.pos[0] = pos[0];
    s.pos[1] = pos[1];
    s.pos[2] = pos[2];
    s.rad = rad;
    s.color = col;
    GlowSpritesList.push_back(s);
}

// ea: 0x672100
void AddGlowBeam(float* pos, float* dir, float rad, unsigned int col)
{
    GlowBeam s;
    s.pos[0] = pos[0];
    s.pos[1] = pos[1];
    s.pos[2] = pos[2];
    s.dir[0] = dir[0];
    s.dir[1] = dir[1];
    s.dir[2] = dir[2];
    s.rad = rad;
    s.color = col;
    GlowBeamsList.push_back(s);
}

// ea: 0x66D320
void RenderGlowSprite(const math::Position3& center, float radius,
                      unsigned int clr)
{
    const math::Mat43* mtx = &nglGetMatrix_ViewToWorld(nglBuildScene);
    __m128 xcol = _mm_mul_ps(mtx->x.v, _mm_set1_ps(radius));
    __m128 ycol = mtx->y.v;

    cdScratchMaterial* mat = (cdScratchMaterial*)nglListAlloc(0x20, 0x10);
    if (mat != nullptr)
    {
        tlFixedString name("dynamiclight");
        nglTexture* tex = nglGetTexture(name);
        new (mat) cdScratchMaterial(tex, 0x64078600u, 2, false);
    }

    nglMesh* ScratchMesh = auxCreateScratchMesh(0x40000, 1);
    nglMeshSection* ScratchSection =
        nglCreateScratchSection(6, 4, 4, &cdscratch_vertex_format);
    nglAddMeshSection(ScratchMesh, ScratchSection, (nglMaterial*)mat, 1);

    unsigned short* idx = (unsigned short*)nglLockSectionIndices(ScratchSection);
    unsigned int* verts = (unsigned int*)nglLockSectionVertices(ScratchSection);

    __m128 v0 = _mm_sub_ps(_mm_sub_ps(center.v, xcol), ycol);
    verts[0] = v0.m128_u32[0];
    verts[1] = v0.m128_u32[1];
    verts[2] = v0.m128_u32[2];
    verts[5] = clr;
    verts[3] = 0;
    verts[4] = 0;
    idx[0] = 0;
    verts += 6;

    __m128 v1 = _mm_sub_ps(_mm_add_ps(center.v, xcol), ycol);
    verts[0] = v1.m128_u32[0];
    verts[1] = v1.m128_u32[1];
    verts[2] = v1.m128_u32[2];
    verts[5] = clr;
    verts[3] = 0x3F800000;  // u = 1.0
    verts[4] = 0;
    idx[1] = 1;
    verts += 6;

    __m128 v2 = _mm_add_ps(_mm_sub_ps(center.v, xcol), ycol);
    verts[0] = v2.m128_u32[0];
    verts[1] = v2.m128_u32[1];
    verts[2] = v2.m128_u32[2];
    verts[5] = clr;
    verts[3] = 0;
    verts[4] = 0x3F800000;  // v = 1.0
    idx[2] = 2;
    verts += 6;

    __m128 v3 = _mm_add_ps(_mm_add_ps(center.v, xcol), ycol);
    verts[0] = v3.m128_u32[0];
    verts[1] = v3.m128_u32[1];
    verts[2] = v3.m128_u32[2];
    verts[5] = clr;
    verts[3] = 0x3F800000;  // u = 1.0
    verts[4] = 0x3F800000;  // v = 1.0
    idx[3] = 3;

    nglUnlockSectionIndices(ScratchSection);
    nglUnlockSectionVertices(ScratchSection);

    math::Mat43 localToWorld;
    localToWorld.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    localToWorld.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    localToWorld.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    nglListAddMesh(ScratchMesh, localToWorld, nullptr, nullptr, nullptr);
}

// ea: 0x66CFA0
void RenderGlowBeam(const math::Position3& center,
                    const math::Position3& center2, float radius,
                    unsigned int clr)
{
    (void)clr;  // beam colors are hardcoded in the binary
    const math::Mat43* mtx = &nglGetMatrix_ViewToWorld(nglBuildScene);
    __m128 xcol = _mm_mul_ps(mtx->x.v, _mm_set1_ps(radius));
    __m128 ycol = mtx->y.v;

    cdScratchMaterial* mat = (cdScratchMaterial*)nglListAlloc(0x20, 0x10);
    if (mat != nullptr)
    {
        tlFixedString name("dynamiclight");
        nglTexture* tex = nglGetTexture(name);
        new (mat) cdScratchMaterial(tex, 0x64078600u, 2, false);
    }

    nglMesh* ScratchMesh = auxCreateScratchMesh(0x40000, 1);
    nglMeshSection* ScratchSection =
        nglCreateScratchSection(6, 4, 4, &cdscratch_vertex_format);
    nglAddMeshSection(ScratchMesh, ScratchSection, (nglMaterial*)mat, 1);

    unsigned short* idx = (unsigned short*)nglLockSectionIndices(ScratchSection);
    unsigned int* verts = (unsigned int*)nglLockSectionVertices(ScratchSection);

    __m128 v0 = _mm_sub_ps(_mm_sub_ps(center.v, xcol), ycol);
    verts[0] = v0.m128_u32[0];
    verts[1] = v0.m128_u32[1];
    verts[2] = v0.m128_u32[2];
    verts[5] = 0x80909090;
    verts[3] = 0;
    verts[4] = 0x3E99999A;  // v = 0.3
    idx[0] = 0;
    verts += 6;

    __m128 v1 = _mm_sub_ps(_mm_sub_ps(center2.v, xcol), ycol);
    verts[0] = v1.m128_u32[0];
    verts[1] = v1.m128_u32[1];
    verts[2] = v1.m128_u32[2];
    verts[5] = 0x10101010;
    verts[3] = 0;
    verts[4] = 0x3F000000;  // v = 0.5
    idx[1] = 1;
    verts += 6;

    __m128 v2 = _mm_sub_ps(_mm_add_ps(center.v, xcol), ycol);
    verts[0] = v2.m128_u32[0];
    verts[1] = v2.m128_u32[1];
    verts[2] = v2.m128_u32[2];
    verts[5] = 0x80909090;
    verts[3] = 0x3F800000;  // u = 1.0
    verts[4] = 0x3E99999A;  // v = 0.3
    idx[2] = 2;
    verts += 6;

    __m128 v3 = _mm_add_ps(_mm_add_ps(center2.v, xcol), ycol);
    verts[0] = v3.m128_u32[0];
    verts[1] = v3.m128_u32[1];
    verts[2] = v3.m128_u32[2];
    verts[5] = 0x10101010;
    verts[3] = 0x3F800000;  // u = 1.0
    verts[4] = 0x3F000000;  // v = 0.5
    idx[3] = 3;

    nglUnlockSectionIndices(ScratchSection);
    nglUnlockSectionVertices(ScratchSection);

    math::Mat43 localToWorld;
    localToWorld.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    localToWorld.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    localToWorld.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    nglListAddMesh(ScratchMesh, localToWorld, nullptr, nullptr, nullptr);
}

// ea: 0x675EA0
void SceneManager::RenderLightGlows()
{
    if (gGlowTexture == nullptr)
    {
        tlFixedString name("dynamiclight");
        gGlowTexture = nglGetTexture(name);
    }
    if (View::IsSplitScreen())
        return;

    for (int i = 0; i < GlowSpritesList.mSize; ++i)
    {
        GlowSprites& s = GlowSpritesList.mElements[i];
        RenderGlowSprite(*(math::Position3*)s.pos, s.rad, s.color);
    }
    GlowSpritesList.resize(0);

    for (int v7 = 0; v7 < GlowBeamsList.mSize; ++v7)
    {
        GlowBeam& b = GlowBeamsList.mElements[v7];
        math::Position3 center;
        center.v = _mm_loadu_ps(b.pos);
        math::Position3 center2;
        center2.v.m128_f32[0] = b.dir[0] + b.pos[0];
        center2.v.m128_f32[1] = b.dir[1] + b.pos[1];
        center2.v.m128_f32[2] = b.dir[2] + b.pos[2];
        center2.v.m128_f32[3] = 0.0f;
        RenderGlowBeam(center, center2, b.rad, b.color);
    }
    GlowBeamsList.resize(0);

    for (int bankIdx = 0; bankIdx < 99; ++bankIdx)
    {
        SceneBank* bank = mBankArray.m_elements[bankIdx];
        if (bank == nullptr)
            continue;
        if (bank->mSceneLights.mSize == 0)
            continue;
        for (unsigned int li = 0; li < bank->mSceneLights.mSize; ++li)
        {
            SceneLight* light = &bank->mSceneLights.mList[li];
            const char* target = (const char*)light->m_target;
            if (target != nullptr)
            {
                Broc::string name(target);
                unsigned int v20 =
                    gpBrocAPI->mGetEnt(&name,
                                       HashString::CalcHash("targetname"),
                                       nullptr, 0, 1);
                unsigned int v21 = v20 & 0xFFF;
                Entity* targetEnt = nullptr;
                if (v21 < 0x540
                    && v20 >> 12 == EntityHandleDb::sInst.mElements[v21].mKey
                    && (targetEnt =
                            EntityHandleDb::sInst.mElements[v21].mObject)
                           != nullptr)
                {
                    const math::Mat43* mtx =
                        &nglGetMatrix_ViewToWorld(nglBuildScene);
                    __m128 xrow = mtx->x.v;
                    __m128 yrow = mtx->y.v;
                    float radius = light->mGlowRadius;
                    __m128 xcol = _mm_mul_ps(xrow, _mm_set1_ps(radius));
                    __m128 xcol3 = _mm_mul_ps(xrow, _mm_set1_ps(radius * 3.0f));

                    cdScratchMaterial* mat =
                        (cdScratchMaterial*)nglListAlloc(0x20, 0x10);
                    if (mat != nullptr)
                    {
                        tlFixedString texname("dynamiclight");
                        nglTexture* tex = nglGetTexture(texname);
                        new (mat) cdScratchMaterial(tex, 0x64078600u, 2,
                                                    false);
                    }

                    nglMesh* ScratchMesh = auxCreateScratchMesh(0x40000, 1);
                    nglMeshSection* ScratchSection = nglCreateScratchSection(
                        6, 4, 4, &cdscratch_vertex_format);
                    nglAddMeshSection(ScratchMesh, ScratchSection,
                                      (nglMaterial*)mat, 1);

                    unsigned short* idx = (unsigned short*)
                        nglLockSectionIndices(ScratchSection);
                    unsigned int* verts = (unsigned int*)
                        nglLockSectionVertices(ScratchSection);

                    math::Position3 lightPos = light->mPosition;
                    math::Position3 entPos = targetEnt->r.currentOrigin;

                    __m128 v0 =
                        _mm_sub_ps(_mm_sub_ps(lightPos.v, xcol), yrow);
                    verts[0] = v0.m128_u32[0];
                    verts[1] = v0.m128_u32[1];
                    verts[2] = v0.m128_u32[2];
                    verts[5] = 0x80909090;
                    verts[3] = 0;
                    verts[4] = 0x3E99999A;  // v = 0.3
                    idx[0] = 0;
                    verts += 6;

                    __m128 v1 =
                        _mm_sub_ps(_mm_sub_ps(entPos.v, xcol3), yrow);
                    verts[0] = v1.m128_u32[0];
                    verts[1] = v1.m128_u32[1];
                    verts[2] = v1.m128_u32[2];
                    verts[5] = 0x10101010;
                    verts[3] = 0;
                    verts[4] = 0x3F000000;  // v = 0.5
                    idx[1] = 1;
                    verts += 6;

                    __m128 v2 =
                        _mm_sub_ps(_mm_add_ps(lightPos.v, xcol), yrow);
                    verts[0] = v2.m128_u32[0];
                    verts[1] = v2.m128_u32[1];
                    verts[2] = v2.m128_u32[2];
                    verts[5] = 0x80909090;
                    verts[3] = 0x3F800000;  // u = 1.0
                    verts[4] = 0x3E99999A;  // v = 0.3
                    idx[2] = 2;
                    verts += 6;

                    __m128 v3 =
                        _mm_sub_ps(_mm_add_ps(entPos.v, xcol), yrow);
                    verts[0] = v3.m128_u32[0];
                    verts[1] = v3.m128_u32[1];
                    verts[2] = v3.m128_u32[2];
                    verts[5] = 0x10101010;
                    verts[3] = 0x3F800000;  // u = 1.0
                    verts[4] = 0x3F000000;  // v = 0.5
                    idx[3] = 3;

                    nglUnlockSectionIndices(ScratchSection);
                    nglUnlockSectionVertices(ScratchSection);

                    math::Mat43 localToWorld;
                    localToWorld.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
                    localToWorld.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
                    localToWorld.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
                    nglListAddMesh(ScratchMesh, localToWorld, nullptr,
                                   nullptr, nullptr);

                    RenderGlowSprite(lightPos, light->mGlowRadius,
                                     0x80909090);
                }
                else
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\scenemanager.cpp";
                    AeAssert::gCurrentLine = 1211;
                    AeAssert::gCurrentExpr = "target";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Bad entity handle"))
                        __debugbreak();
                }
            }
            else
            {
                math::Position3 projected;
                nglProjectPoint(&projected, &light->mPosition, nglBuildScene);
                const math::Mat43* mtx =
                    &nglGetMatrix_ViewToWorld(nglBuildScene);
                __m128 zrow = mtx->z.v;
                __m128 wrow = _mm_loadu_ps((const float*)((const char*)mtx + 0x30));

                bool offscreen = false;
                if (projected.v.m128_f32[0] > (float)nglGetScreenWidth() + 2.0f
                    || projected.v.m128_f32[0] < -2.0f)
                    offscreen = true;
                if (projected.v.m128_f32[1] > (float)nglGetScreenHeight() + 2.0f
                    || projected.v.m128_f32[1] < -2.0f)
                    offscreen = true;

                __m128 v54 = _mm_sub_ps(light->mPosition.v, wrow);
                if (zrow.m128_f32[0] * v54.m128_f32[0]
                        + zrow.m128_f32[1] * v54.m128_f32[1]
                        + zrow.m128_f32[2] * v54.m128_f32[2]
                    >= 0.0f)
                {
                    __m128 v55 = _mm_mul_ps(v54, v54);
                    float dist = sqrtf(v55.m128_f32[0]
                                       + (_mm_shuffle_ps(v55, v55, 85)
                                              .m128_f32[0]
                                          + _mm_shuffle_ps(v55, v55, 170)
                                                .m128_f32[0]));
                    VectorNormalize(*(math::Dir3*)&v54);
                    if (dist < 5.0f || dist > 3000.0f)
                        offscreen = true;

                    math::Position3 end;
                    end.v = wrow;
                    math::Position3 start = light->mPosition;

                    if (currCl >= 16)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\EntityManager.h";
                        AeAssert::gCurrentLine = 19;
                        AeAssert::gCurrentExpr = "idx<16";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bounds check"))
                            __debugbreak();
                    }
                    collision_context_t context;
                    context.__vftable =
                        (collision_context_t_vtbl*)0x00CD8F6C;
                    context.pass_entity1 =
                        *(unsigned int*)((char*)EntityManager::sInst
                                         ->mPlayers[currCl]
                                         + 0x234);  // mHandle
                    context.pass_entity2 = 0;
                    context.pass_owner1 = 0;
                    context.pass_owner2 = 0;
                    context.contentmask = 0x2802033;

                    trace_t trace;
                    g_LocationalTrace(&trace, start, end, context,
                                      bulletPriorityMap, 0.0f);

                    float fade = light->mGlowFade;
                    if (trace.contents != 0 || offscreen)
                        fade -= 0.1f;
                    else
                        fade += 0.1f;
                    light->mGlowFade = fade;
                    if (fade < 0.0f)
                        light->mGlowFade = 0.0f;
                    if (light->mGlowFade > 1.0f)
                        light->mGlowFade = 1.0f;

                    if (light->mGlowFade != 0.0f)
                    {
                        math::Vector4 v72;
                        v72.v = _mm_setr_ps(light->mGlowRadius, 0.0f, dist,
                                            1.0f);
                        math::Mat44 m;
                        nglGetMatrix(&m, NGLMTX_PROJECTION, nglBuildScene);
                        __m128 v60 = _mm_add_ps(
                            _mm_add_ps(
                                _mm_mul_ps(_mm_shuffle_ps(v72.v, v72.v, 0),
                                           m.x.v),
                                _mm_mul_ps(_mm_shuffle_ps(v72.v, v72.v, 85),
                                           m.y.v)),
                            _mm_add_ps(
                                _mm_mul_ps(_mm_shuffle_ps(v72.v, v72.v, 170),
                                           m.z.v),
                                _mm_mul_ps(_mm_shuffle_ps(v72.v, v72.v, 255),
                                           m.w.v)));
                        float screenX = ((float)nglGetScreenWidth() / 2.0f
                                         / v60.m128_f32[3])
                                        * v60.m128_f32[0];
                        float x1 = projected.v.m128_f32[0] - screenX;
                        float x2 = projected.v.m128_f32[0] + screenX;
                        float y1 = projected.v.m128_f32[1] - screenX;
                        float y2 = projected.v.m128_f32[1] + screenX;

                        nglQuad q;
                        nglInitQuad(&q);
                        nglSetQuadRect(&q, x1, y1, x2, y2);
                        nglSetQuadZ(&q, 0.0f);
                        nglSetQuadTex(&q, gGlowTexture);
                        unsigned int r = (unsigned int)(light->mRed * 255.0f);
                        unsigned int g = (unsigned int)(light->mGreen * 255.0f);
                        unsigned int b = (unsigned int)(light->mBlue * 255.0f);
                        unsigned int a = (unsigned int)
                            (light->mGlowIntensity * light->mGlowFade
                             * 255.0f);
                        nglSetQuadColor(&q,
                                        r | (g << 8) | (b << 16)
                                            | (a << 24));
                        nglSetQuadBlend(&q, 0x64078600u);
                        nglListAddQuad(&q);
                    }
                }
            }
        }
    }
}

// ea: 0x669AF0
void SceneManager::RenderInstanceGroups()
{
    math::Position3 cameraPos;
    cameraPos.v = _mm_setr_ps(tr.viewParmsOrigin[0], tr.viewParmsOrigin[1],
                              tr.viewParmsOrigin[2], 0.0f);
    if (gFirstCamera)
        ++gFrameToggle;
    int instanceLods = Cvar_Get("instance_lods", "1", 256)->integer;
    int cullInstances = Cvar_Get("cull_instances", "1", 256)->integer;
    if (View::IsSplitScreen())
        cullInstances = 0;

    for (int bankIdx = 0; bankIdx < 99; ++bankIdx)
    {
        SceneBank* bank = mBankArray.m_elements[bankIdx];
        if (bank == nullptr)
            continue;
        InplaceVector<InstanceGroup>* groups = &bank->mInstanceGroups;
        for (unsigned int gi = 0; gi < groups->mSize; ++gi)
        {
            InstanceListNode* node =
                (InstanceListNode*)groups->mList[gi].instanceList;
            if (node == nullptr)
                continue;
            for (; node != nullptr; node = node->next)
            {
                int visibleCount = 0;
                __m128 meshData =
                    *(__m128*)((const char*)node->instance.Mesh + 0x20);
                float max2 =
                    _mm_shuffle_ps(meshData, meshData, 255).m128_f32[0] * 1.1f;
                if (instanceLods != 0)
                {
                    float min2 = node->mMinDist * node->mMinDist;
                    float maxDist2 =
                        node->mMaxDist * node->mMaxDist * 1.05f;
                    int* array = node->mInstanceData;
                    if (array[2] == 0)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "../ae\\inplace/InplaceVector.h";
                        AeAssert::gCurrentLine = 81;
                        AeAssert::gCurrentExpr = "index < mSize";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bounds check"))
                            __debugbreak();
                    }
                    InstanceData* inst = (InstanceData*)array[3];
                    unsigned int** radii = node->mRadii;
                    int count = array[2];
                    for (int group = 0; group < count; ++group)
                    {
                        __m128 worldPos =
                            _mm_setr_ps(inst[group].worldPos[0],
                                        inst[group].worldPos[1],
                                        inst[group].worldPos[2], 0.0f);
                        __m128 d = _mm_sub_ps(worldPos, cameraPos.v);
                        __m128 d2 = _mm_mul_ps(d, d);
                        float dist2 =
                            (d2.m128_f32[0]
                             + (_mm_shuffle_ps(d2, d2, 85).m128_f32[0]
                                + _mm_shuffle_ps(d2, d2, 170).m128_f32[0]))
                            * gZoomRatio;
                        unsigned int* radius = radii[group];
                        int flag = 1;  // culled
                        if (dist2 >= min2 && maxDist2 >= dist2)
                        {
                            int cell = node->instance.cells[group];
                            bool portalVisible = true;
                            if (cullInstances != 0 && g_camera_cell >= 0
                                && g_camera_cell != cell && cell >= 0)
                            {
                                portalVisible = false;
                                if (portals.m_alloc_count > 0)
                                {
                                    __m128 cullA =
                                        _mm_setr_ps(inst[group].cullA[0],
                                                    inst[group].cullA[1],
                                                    inst[group].cullA[2],
                                                    0.0f);
                                    __m128 cullB =
                                        _mm_setr_ps(inst[group].cullB[0],
                                                    inst[group].cullB[1],
                                                    inst[group].cullB[2],
                                                    0.0f);
                                    // cross term per binary shuffle pattern:
                                    // (cullA.y*cullB.z - cullA.z*cullB.y,
                                    //  0, -(that))
                                    __m128 cross = _mm_sub_ps(
                                        _mm_mul_ps(
                                            _mm_shuffle_ps(cullA, cullA, 0x09),
                                            _mm_shuffle_ps(cullB, cullB, 0x12)),
                                        _mm_mul_ps(
                                            _mm_shuffle_ps(cullA, cullA, 0x12),
                                            _mm_shuffle_ps(cullB, cullB, 0x09)));
                                    __m128 xv = _mm_shuffle_ps(meshData,
                                                               meshData, 0);
                                    __m128 yv = _mm_shuffle_ps(meshData,
                                                               meshData, 0x55);
                                    __m128 zv = _mm_shuffle_ps(meshData,
                                                               meshData, 0xAA);
                                    __m128 v24 = _mm_add_ps(
                                        _mm_add_ps(
                                            _mm_mul_ps(xv, cullA),
                                            _mm_mul_ps(yv, cullB)),
                                        _mm_add_ps(_mm_mul_ps(zv, cross),
                                                   worldPos));
                                    for (int planes = 0;
                                         planes < portals.m_alloc_count;
                                         ++planes)
                                    {
                                        phys_static_array<math::Vector4, 20>&
                                            inner = portals[planes];
                                        int j2 = 0;
                                        for (; j2 < inner.m_alloc_count; ++j2)
                                        {
                                            if (j2 < 0
                                                || j2 >= inner.m_alloc_count)
                                            {
                                                if (_tlAssert(
                                                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                                                        114,
                                                        "i >= 0 && i < m_alloc_count",
                                                        defaultFileName))
                                                    __debugbreak();
                                            }
                                            __m128 plane =
                                                inner.m_slot_array[j2].v;
                                            float dot =
                                                plane.m128_f32[0]
                                                    * v24.m128_f32[0]
                                                + plane.m128_f32[1]
                                                      * v24.m128_f32[1]
                                                + plane.m128_f32[2]
                                                      * v24.m128_f32[2];
                                            if (-max2 > dot - plane.m128_f32[3])
                                                break;
                                        }
                                        if (j2 == inner.m_alloc_count)
                                        {
                                            portalVisible = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (portalVisible)
                            {
                                flag = 0;
                                ++visibleCount;
                            }
                        }
                        int bit = gFrameToggle & 1;
                        *radius = (flag << bit) | (*radius & (2 - bit));
                    }
                }
                else if (node->instance.mMinLodDist2 == 0.0f)
                {
                    for (int k = 0; k < node->instance.NInstances; ++k)
                        *node->mRadii[k] = 0;
                    visibleCount = node->instance.NInstances;
                }
                if (visibleCount != 0)
                    node->instance.Render();
            }
        }
    }
}

// ea: 0x672170
void SceneManager::DebugRenderEnts()
{
    if (!mDebugRenderEnts)
        return;

    int simpleEnts = 0;
    int pak = 0;                 // "PakReq" count
    int unstreamedWasActor = 0;  // "Main" count
    int main = 0;                // "Actor" count
    int unstreamedWasPreReq = 0; // "Simple" count
    int radius = 0;              // "From PreReq" count
    int maxLength = 0;           // "From Actor" count
    int fromPak = 0;             // v99: "From Pak" count
    int total = 0;
    float unstreamedSimple[3];   // v79/v80/numDrawn: Pak/Zone/Main simple mem
    float memUseSimp[5];
    float memUseComp[5];
    float pakCount = 0.0f;       // memUseComp[4]
    memset(memUseSimp, 0, sizeof(memUseSimp));
    memUseComp[0] = 0.0f;
    memUseComp[1] = 0.0f;
    memUseComp[4] = 0.0f;
    unstreamedSimple[0] = 0.0f;
    unstreamedSimple[1] = 0.0f;
    unstreamedSimple[2] = 0.0f;

    const Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    math::Position3 playerPos = Player->r.currentOrigin;
    int renderLimit = 0;

    BitSet<1344> freeBits =
        ~*(BitSet<1344>*)&EntityHandleDb::sInst.mFreeIndices;
    BitSet<1344>::iterator it = freeBits.begin();
    ++it;
    while (it.m_cur_val != -1 || it.m_word_idx != -1)
    {
        int idx = *it;
        ++total;
        bool overLimit = (++renderLimit > 200);

        Entity* entity = EntityHandleDb::sInst.mElements[idx].mObject;
        math::Position3 origin = entity->r.currentOrigin;
        math::Position3 d;
        d.v = _mm_sub_ps(origin.v, playerPos.v);
        __m128 d2 = _mm_mul_ps(d.v, d.v);
        float zone = sqrtf(d2.m128_f32[0]
                           + (_mm_shuffle_ps(d2, d2, 85).m128_f32[0]
                              + _mm_shuffle_ps(d2, d2, 170).m128_f32[0]));
        bool within5000 = (zone <= 5000.0f);
        bool within12000 = (zone <= 12000.0f);

        Broc::string heapLabel;   // v101
        Broc::string pakLabel;    // v106
        pakLabel = defaultFileName;

        TPakId entityPak = (TPakId)entity->mPakId;
        if (entityPak == PAK_ID_INVALID)
            entityPak = PakManager::sInst->mLevelPakId;
        PakFile* slot = (entityPak <= 0x62)
                            ? PakManager::sInst->mSlots[entityPak]
                            : nullptr;

        int v16;
        if (slot != nullptr && slot->IsInPakHeap(entity))
            v16 = 0;
        else
        {
            unsigned char* arena = BankManager::sInst->mMramArena;
            if (arena != nullptr && (unsigned char*)entity >= arena
                && (unsigned char*)entity
                       < arena + (unsigned int)(BankManager::sInst
                                                    ->mMramBankSize
                                                * BankManager::sInst
                                                      ->mNumMramBanks))
                v16 = 1;
            else
            {
                mem_heap* h = gActorHeap->GetHeapPointer();
                if (h != nullptr && (unsigned char*)entity
                                           >= (unsigned char*)h->start
                    && (unsigned char*)entity < (unsigned char*)h->end)
                    v16 = 4;
                else
                {
                    overLimit = false;
                    v16 = 2;
                }
            }
        }

        memUseComp[3] = 8.0f;
        float* actorMem =
            (v16 <= 2) ? &memUseSimp[v16 + 2] : &memUseComp[v16 - 3];
        if (entity->mActor != nullptr)
        {
            memUseComp[3] = 30.0f;
            *actorMem += 13.9f;
        }
        else if (entity->mScrVehicle != nullptr)
        {
            memUseComp[3] = 20.0f;
            *actorMem += 11.67f;
        }
        else
        {
            float* simpleMem =
                (v16 <= 2) ? &unstreamedSimple[v16] : &memUseSimp[v16 - 3];
            *simpleMem += 1.12f;
            ++unstreamedWasPreReq;
        }

        if (v16 != 2)
        {
            TPakId v21 = (TPakId)entity->mPakId;
            if (v21 == PAK_ID_INVALID)
                v21 = PakManager::sInst->mLevelPakId;
            if (v21 == PakManager::sInst->mLevelPakId)
            {
                if (v16 != 0)
                {
                    int v22 = v16 - 1;
                    if (v22 == 0)
                    {
                        pakLabel = "Zone";
                        ++radius;
                        v16 = 3;
                        goto DebugRenderEnts_Switch;
                    }
                    if (v22 == 3)
                    {
                        pakLabel = "Actor";
                        ++maxLength;
                        v16 = 3;
                        goto DebugRenderEnts_Switch;
                    }
                }
                else
                {
                    pakLabel = "Pak";
                    ++fromPak;
                }
                v16 = 3;
            }
        }

    DebugRenderEnts_Switch:
        Color entCol;
        switch (v16)
        {
        case 0:
            entCol = Color(0.0f, 1.0f, 0.0f, 0.5f);
            heapLabel = "Pak";
            pakCount += 1.0f;
            break;
        case 1:
            entCol = Color(1.0f, 1.0f, 0.0f, 0.5f);
            heapLabel = "Zone";
            ++pak;
            break;
        case 2:
            entCol = Color(1.0f, 0.0f, 0.0f, 0.5f);
            heapLabel = "Main";
            ++unstreamedWasActor;
            break;
        case 3:
            entCol = Color(1.0f, 0.5f, 0.0f, 0.5f);
            heapLabel = "Unstreamed";
            ++simpleEnts;
            break;
        default:
            entCol = Color(0.0f, 0.0f, 1.0f, 0.5f);
            heapLabel = "Actor";
            ++main;
            break;
        }

        math::Position3 up = origin;
        up.v.m128_f32[2] += 5000.0f;
        if (within5000 && !overLimit)
            DebugRender::RenderLine(origin, up, entCol, 2.2f);

        if (within12000 && !overLimit)
        {
            DebugRender::RenderSphere(origin, memUseComp[3], entCol);
            if (zone < 500.0f)
            {
                Broc::string label;
                if (entity->mClassName.mBlock != nullptr)
                    label += Broc::string("classname: ") + entity->mClassName
                             + "\n";
                if (entity->mGroupName.mBlock != nullptr)
                    label += Broc::string("groupname: ") + entity->mGroupName
                             + "\n";
                if (entity->mAnimName.mBlock != nullptr)
                    label += Broc::string("animname: ") + entity->mAnimName
                             + "\n";
                TPakId v33 = (TPakId)entity->mPakId;
                if (v33 == PAK_ID_INVALID)
                    v33 = PakManager::sInst->mLevelPakId;
                if (v33 <= 0x62)
                {
                    PakFile* v34 = PakManager::sInst->mSlots[v33];
                    if (v34 != nullptr)
                    {
                        label += Broc::string("pak: ")
                                 + Broc::string(v34->mPath.mBuff) + "\n";
                    }
                }
                label += Broc::string("heap: ") + heapLabel + "\n";
                if (!(pakLabel == defaultFileName))
                    label += Broc::string("(") + pakLabel + ")\n";
                if (label.mBlock == nullptr)
                    label = "[ent here]";

                float fade;
                if (zone >= 250.0f)
                {
                    if (zone <= 1000.0f)
                        fade = 1.0f - ((zone - 250.0f) * 0.0013333333f);
                    else
                        fade = 0.2f;
                }
                else
                {
                    fade = 1.0f;
                }
                const char* txt =
                    label.mBlock != nullptr ? label.GetBuff() : defaultFileName;
                DebugRender::RenderText3D(origin, Color(1.0f, 1.0f, 1.0f, fade),
                                          1.0f, txt);
            }
        }
        ++it;
    }

    char summary[256];
    int nonSimple = total - unstreamedWasPreReq;
    sprintf(summary,
            "Entities: %d\nSimple: %d (%0.2f kb)\nActors: %d (%0.2f kb)\n",
            total, unstreamedWasPreReq, unstreamedWasPreReq * 1.12,
            nonSimple, nonSimple * 13.9);
    DebugRender::RenderText(summary, 20, 50, Color(1.0f, 1.0f, 1.0f, 1.0f),
                            0.0f, 1.0f);

    int pakCountInt = (int)pakCount;
    sprintf(summary, "Pak: %d (%0.2f kb, %0.2f kb) %0.2f kb\n", pakCountInt,
            unstreamedSimple[0], memUseSimp[2],
            memUseSimp[2] + unstreamedSimple[0]);
    DebugRender::RenderText(summary, 20, 102, Color(0.5f, 0.0f, 1.0f, 0.0f),
                            0.0f, 1.0f);

    sprintf(summary, "PakReq: %d (%0.2f kb, %0.2f kb) %0.2f kb\n", pak,
            unstreamedSimple[1], memUseSimp[3],
            memUseSimp[3] + unstreamedSimple[1]);
    DebugRender::RenderText(summary, 20, 118, Color(1.0f, 1.0f, 0.0f, 1.0f),
                            0.0f, 1.0f);

    sprintf(summary, "Actor: %d (%0.2f kb, %0.2f kb) %0.2f kb\n", main,
            memUseSimp[1], memUseComp[1], memUseComp[1] + memUseSimp[1]);
    DebugRender::RenderText(summary, 20, 134, Color(0.0f, 0.0f, 1.0f, 1.0f),
                            0.0f, 1.0f);

    sprintf(summary, "Unstreamed: %d (From Pak %d, From PreReq %d, From Actor %d)\n",
            simpleEnts, fromPak, radius, maxLength);
    DebugRender::RenderText(summary, 20, 150,
                            Color(0.5f, 1.0f, 0.5f, 0.0f), 0.0f, 1.0f);

    sprintf(summary, "Main: %d (%0.2f kb, %0.2f kb) %0.2f kb\n",
            unstreamedWasActor, unstreamedSimple[2], memUseSimp[4],
            memUseSimp[4] + unstreamedSimple[2]);
    DebugRender::RenderText(summary, 20, 166, Color(1.0f, 0.0f, 0.0f, 1.0f),
                            0.0f, 1.0f);

    // stacked distribution bar (Pak / PakReq / Actor / Unstreamed / Main)
    float pakFrac = 0.0f;
    float pakReqFrac = 0.0f;
    float actorFrac = 0.0f;
    float mainFrac = 0.0f;
    float unstreamedFrac = 0.0f;
    if (pakCountInt != 0)
        pakFrac = pakCountInt / (float)total;
    if (pak != 0)
        pakReqFrac = pak / (float)total;
    if (simpleEnts != 0)
        unstreamedFrac = simpleEnts / (float)total;
    if (unstreamedWasActor != 0)
        mainFrac = unstreamedWasActor / (float)total;
    if (main != 0)
        actorFrac = main / (float)total;
    float w0 = total * pakFrac;
    float w1 = total * pakReqFrac;
    float w2 = total * actorFrac;
    float w3 = total * unstreamedFrac;
    float w4 = total * mainFrac;
    DebugRender::RenderQuad2D(20.0f, 20.0f, 20.0f + w0, 40.0f, -5.0f,
                              Color(0.0f, 1.0f, 0.0f, 1.0f));
    DebugRender::RenderQuad2D(20.0f + w0, 20.0f, 20.0f + w0 + w1, 40.0f,
                              -5.0f, Color(1.0f, 1.0f, 0.0f, 1.0f));
    DebugRender::RenderQuad2D(20.0f + w0 + w1, 20.0f, 20.0f + w0 + w1 + w2,
                              40.0f, -5.0f, Color(0.0f, 0.0f, 1.0f, 1.0f));
    DebugRender::RenderQuad2D(20.0f + w0 + w1 + w2, 20.0f,
                              20.0f + w0 + w1 + w2 + w3, 40.0f, -5.0f,
                              Color(1.0f, 0.5f, 0.0f, 1.0f));
    DebugRender::RenderQuad2D(20.0f + w0 + w1 + w2 + w3, 20.0f,
                              20.0f + w0 + w1 + w2 + w3 + w4, 40.0f, -5.0f,
                              Color(1.0f, 0.0f, 0.0f, 1.0f));
}

// ea: 0x679D20
void SceneManager::DecodeScene(const char* name, unsigned char* data,
                               int size, TPakId pakId)
{
    (void)name; (void)size;
    SceneBank* bank = (SceneBank*)data;
    bank->Fixup();
    AddBank(pakId, bank);
    InplaceVector<unsigned char>* storage =
        (InplaceVector<unsigned char>*)&bank->mPersistentStorage;
    for (unsigned int v7 = 0; v7 < storage->mSize; ++v7)
        storage->mList[v7] = 0;
    PostProcess(pakId);
    if (storage->mSize != 0)
    {
        if (mPersistantStorage != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scenemanager.cpp";
            AeAssert::gCurrentLine = 260;
            AeAssert::gCurrentExpr = "!mPersistantStorage";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                       "TOO MANY SCENEBANKS WITH PERSISTANT STORAGE ARRAYS"))
                __debugbreak();
        }
        mPersistantStorage = storage;
    }
}

// ea: 0x679DD0
void DecodeZoneBoundaryBank(const char* name, unsigned char* data, int size,
                            TPakId pakId)
{
    StreamZoneManager::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x679DF0
void DecodeScene(const char* name, unsigned char* data, int size,
                 TPakId pakId)
{
    SceneManager::sInst->DecodeScene(name, data, size, pakId);
}

// ea: 0x6663F0
void InstanceBankMgr::DecodeInstbank(const char* name, unsigned char* data,
                                     int size, TPakId pakId)
{
    (void)name; (void)size;
    InstanceBankSet* bank = (InstanceBankSet*)data;
    if (mEntries[pakId] != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 183;
        AeAssert::gCurrentExpr = "mEntries[pakId] == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Instance bank already loaded!"))
            __debugbreak();
    }
    bank->Fixup();
    const char* types[13] = {
        "TEXTURE", "FONT", "MESHFILE", "MESH", "ANIMFILE", "ANIM",
        "SCNANIM", "ANIMOFFSET", "SKELETON", "EFFECT", "FX", "DISCTEX",
        "DISCTEXSIZE",
    };
    for (int i = INSTBANK_TYPE_TEXTURE; i <= INSTBANK_TYPE_DISCTEXSIZE; ++i)
    {
        InstanceBank* Bank = &bank->GetBank((eInstanceBankType)i);
        if (_stricmp(Bank->mTypeStr, types[i]) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
            AeAssert::gCurrentLine = 200;
            AeAssert::gCurrentExpr = "!stricmp(bank.GetTypeStr(), types[i])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Bad instance bank!"))
                __debugbreak();
        }
    }
    mEntries[pakId] = bank;
}

// ea: 0x666A40
void PakManager::RegisterPakLoaded(PakInfoNode* pak)
{
    if (pak->pakId == PAK_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 1968;
        AeAssert::gCurrentExpr = "pak->pakId != PAK_ID_INVALID";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("this fcn does not assign the pakId"))
            __debugbreak();
    }
    unsigned int v2 = 0;
    if ((pak->refCount & 0x80000000) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 1971;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("negative ref count!"))
            __debugbreak();
    }
    unsigned int mSize = pak->prereqs.mSize;
    ++pak->refCount;
    if (mSize != 0)
    {
        do
        {
            PakInfoNode* v6 = (PakInfoNode*)pak->prereqs.mList[v2];
            TPakId pakId = v6->pakId;
            if (pakId == PAK_ID_INVALID || mSlots[pakId] == nullptr
                || mSlots[pakId]->mState != PakFile::LOADED)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 1977;
                AeAssert::gCurrentExpr = "IsLoaded(prereq->pakId)";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                           "prereq better be loaded or we've got problems"))
                    __debugbreak();
            }
            RegisterPakLoaded(v6);
            ++v2;
        } while (v2 < pak->prereqs.mSize);
    }
}

// ea: 0x666B90
void PakManager::RegisterPakUnloaded(PakInfoNode* pak)
{
    --pak->refCount;
    unsigned int v3 = 0;
    if (pak->prereqs.mSize != 0)
    {
        do
        {
            PakInfoNode* v5 = (PakInfoNode*)pak->prereqs.mList[v3];
            TPakId pakId = v5->pakId;
            if (pakId == PAK_ID_INVALID || mSlots[pakId] == nullptr
                || mSlots[pakId]->mState != PakFile::LOADED)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 1993;
                AeAssert::gCurrentExpr = "IsLoaded(prereq->pakId)";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                           "prereq better be loaded or we've got problems"))
                    __debugbreak();
            }
            RegisterPakUnloaded(v5);
            ++v3;
        } while (v3 < pak->prereqs.mSize);
    }
}

// ea: 0x666DF0
void* PakManager::MemAlign(TPakId id, unsigned int align,
                           unsigned int size)
{
    if (id == PAK_ID_INVALID)
        return mem_heap_malloc((int)align, size);
    if (sInst->mSlots[id] == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 2617;
        AeAssert::gCurrentExpr = "PakManager::Inst()->IsValid( id )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("bad pak id for MemAlign"))
            __debugbreak();
    }
    PakFile* v5 = id > 0x62 ? nullptr : mSlots[id];
    void* result = v5->MemAlloc(align, size, true);
    if (result == nullptr)
        return mem_heap_malloc((int)align, size);
    return result;
}

// ea: 0x666E90
void PakManager::MemFree(TPakId id, void* ptr, bool bUseActorHeap)
{
    (void)bUseActorHeap;
    if (id != PAK_ID_INVALID)
    {
        if (sInst->mSlots[id] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 2640;
            AeAssert::gCurrentExpr = "PakManager::Inst()->IsValid( id )";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("bad pak id for free"))
                __debugbreak();
        }
        PakFile* v5 = id > 0x62 ? nullptr : mSlots[id];
        if (v5->MemFree(ptr, true))
            return;
    }
    mem_heap* v6 = gActorHeap->GetHeapPointer();
    if (v6 != nullptr && ptr >= v6->start && ptr < v6->end)
        mem_heap_free(v6, ptr);
    else
        mem_heap_free(ptr);
}

// ============================================================================
// StartupNfl (streamer.o 0x666F50) - binary nfl API view
// ============================================================================
struct nflInitParams {
    unsigned int maxFiles;     // +0x00
    unsigned int maxStreams;   // +0x04
    unsigned int maxRequests;  // +0x08
    unsigned int bufferMode;   // +0x0C
    unsigned int threadMode;   // +0x10

};
struct nflMediaAlignments {
    unsigned int mediaAlignment;        // +0x00
    unsigned int memoryAlignment;       // +0x04
    unsigned int transferSizeAlignment; // +0x08
};
struct nfdDriver;

enum {
    NFL_BUFFER_MODE_UNALIGNED = 2,
    NFL_BUFFER_MODE_ALL = 3,
    NFL_THREAD_MODE_SINGLE = 0,
};
extern unsigned int nflInit(const nflInitParams* ip);  // nfl_common.o
extern void nflStart(void* work);
extern nflMediaAlignments* nflGetMediaAlignments(nflMediaID mediaID,
                                                 nflMediaAlignments* ma);
extern nflMediaID gNflMediaId;    // ?gNflMediaId@@3W4nflMediaID@@A
void* gNflMemAlloc = nullptr;     // ?gNflMemAlloc@@3PAXA
unsigned int gNflAlignment = 0;   // ?gNflAlignment@@3IA
nflMediaID gNflMediaId = NFL_MEDIA_ID_DISC;

extern void nflShutdown();  // nfl_common.o

// ea: 0x6658C0
void ShutdownNfl()
{
    nflShutdown();
    gNflMemAlloc = nullptr;
}

// ea: 0x666F50
void StartupNfl()
{
    nflInitParams v2;
    v2.maxFiles = 128;
    v2.maxRequests = 128;
    v2.maxStreams = 1;
    v2.bufferMode = NFL_BUFFER_MODE_UNALIGNED;
    v2.threadMode = NFL_THREAD_MODE_SINGLE;
    unsigned int v0 = nflInit(&v2);
    gNflMemAlloc = mem_heap_malloc(v0);
    nflStart(gNflMemAlloc);
    nflMediaAlignments mediaAlignments;
    nflGetMediaAlignments(gNflMediaId, &mediaAlignments);
    unsigned int mediaAlignment = mediaAlignments.mediaAlignment;
    if (mediaAlignments.mediaAlignment <= mediaAlignments.memoryAlignment)
        mediaAlignment = mediaAlignments.memoryAlignment;
    if (mediaAlignment <= mediaAlignments.transferSizeAlignment)
        gNflAlignment = mediaAlignments.transferSizeAlignment;
    else if (mediaAlignments.mediaAlignment
             <= mediaAlignments.memoryAlignment)
        gNflAlignment = mediaAlignments.memoryAlignment;
    else
        gNflAlignment = mediaAlignments.mediaAlignment;
}

// ea: 0x665960
void ToggleZoneGraph()
{
    if (Cmd_Argc() <= 1)
    {
        StreamZoneManager::sInst->mDebugRenderMode ^= 1;
    }
    else
    {
        StreamZoneManager::sInst->mDebugRenderMode |= 1;
        StreamZoneManager::sInst->zoneGraphScale =
            (float)atof(Cmd_Argv(1));
    }
}

// ea: 0x6658D0
nflState codNflUpdate()
{
    nflUpdate();
    nflState result = nflGetState();
    if (result == NFL_STATE_ERROR)
        g_femanager.DrawDiscError();
    return result;
}

// ea: 0x6658F0
unsigned int codNflReadFile(nflFileID fileID, unsigned int fileOffset,
                            void* buffer, unsigned int dataSize)
{
    unsigned int result = nflReadFile(fileID, fileOffset, buffer, dataSize);
    if (result == 0)
        g_femanager.DrawDiscError();
    return result;
}

// ea: 0x665470
void MyRenderText(const char* str, int x, int y, const Color& col,
                  float depth, float size)
{
    static int S12_3_guard = 0;
    static font_index font = static_cast<font_index>(-1);
    static nglFont* cached_font = nullptr;
    if ((S12_3_guard & 1) == 0)
    {
        S12_3_guard |= 1;
        font = FEManager_FindFont(&g_femanager, "i_helvetica_bold", false);
        cached_font = FEManager_GetFont(&g_femanager, font);
    }
    nglFont* Font = cached_font;
    if (nglSysFont != nullptr)
        Font = nglSysFont;
    unsigned int color = extract_color(col);
    Color dark_float_color;
    memset(&dark_float_color, 0, 12);
    dark_float_color.a = col.a;
    unsigned int v8 = extract_color(dark_float_color);
    float cola = (float)y;
    float ya = (float)x;
    nglListAddString(Font, str, ya, cola, depth, v8, size, size);
    nglListAddString(Font, str, ya, cola, depth, color, size, size);
}

// ngl debug-quad helpers (ngl.o)
extern int nglGetScreenWidth();
extern int nglGetScreenHeight();
extern void nglInitQuad(nglQuad* quad);
extern void nglSetQuadTex(nglQuad* quad, nglTexture* tex);
extern void nglSetQuadColor(nglQuad* quad, unsigned int c);
extern void nglSetQuadRect(nglQuad* quad, float x1, float y1, float x2,
                           float y2);
extern void nglListAddQuad(nglQuad* quad);
extern void ngliProcessTexture(apk::apkFile* File, apk::apkFileEntry* Entry);
extern void auxFontDirectoryDelete(nglFont* Font);  // ngl_aux.cpp
extern void FEManagerSetFont(nglFont* f, const char* font_filename);  // shell.o
class apsEffectTemplate;  // class (apsEffect.h); matches ?PAV mangling
extern apsEffectTemplate* sMissingParticleEffect;  // ?sMissingParticleEffect@@3PAVapsEffectTemplate@@A @ 0xF59310
extern apsEffectTemplate* apsLoadEffectInplace(apk::apkFile* File,
                                               apk::apkFileEntry* Entry);  // apsRegister.cpp
extern float nflGetRequestProgress(nflRequestID requestID);  // filesystem/nfl.cpp

// ea: 0x66A900
void PakFile::RenderDebug()
{
    if (mLoadingState != LOADING_DATA)
        return;

    const float x1 = 0.2f;
    const float y1 = 0.88f;
    const float x2 = 0.8f;
    const float y2 = 0.9f;
    static double width_per_byte = 0.0027343746342313943;  // @ 0xDF912C

    float ScreenWidth = (float)nglGetScreenWidth();
    float ScreenHeight = (float)nglGetScreenHeight();

    const PakInfoNode* mPakInfo;
    if (mPakType == kPakTypeCount)
        mPakInfo = nullptr;
    else
    {
        if (this->mPakInfo == nullptr)
            this->mPakInfo = PakManager::sInst->GetPakInfo(mPakId);
        mPakInfo = this->mPakInfo;
    }
    if (mPakInfo != nullptr)
    {
        Color white = {1.0f, 1.0f, 1.0f, 1.0f};
        DebugRender::RenderText(mPakInfo->longName.mStr,
                                (int)(x1 * ScreenWidth),
                                (int)(y1 * ScreenHeight - 22.0f), white,
                                0.0f, 1.0f);
    }

    float RequestProgress =
        (float)mCurrentFile
        / (float)mHeader->sections[mDefaultSectionIdx].numFiles;
    if (RequestProgress < 0.0f || RequestProgress > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 1387;
        AeAssert::gCurrentExpr = "done>=0.0f && done<=1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("sanity check"))
            __debugbreak();
    }

    nglQuad* v8 = (nglQuad*)nglListAlloc(0x60, 0x10);
    nglInitQuad(v8);
    nglSetQuadTex(v8, nullptr);
    nglSetQuadColor(v8, 0xFF800000);
    nglSetQuadRect(v8, x1 * ScreenWidth, y1 * ScreenHeight,
                   (x1 + (x2 - x1) * RequestProgress) * ScreenWidth,
                   y2 * ScreenHeight);
    nglListAddQuad(v8);

    nglQuad* v9 = (nglQuad*)nglListAlloc(0x60, 0x10);
    nglInitQuad(v9);
    nglSetQuadTex(v9, nullptr);
    nglSetQuadColor(v9, 0xFF808080);
    nglSetQuadRect(v9, (x1 + (x2 - x1) * RequestProgress) * ScreenWidth,
                   y1 * ScreenHeight, x2 * ScreenWidth, y2 * ScreenHeight);
    nglListAddQuad(v9);

    PakHeader::Section& sec = mHeader->sections[mDefaultSectionIdx];
    float xstart = x1;
    for (unsigned int i = 0; i < sec.numBanks; ++i)
    {
        PakHeader::Bank* bank = &sec.banks[i];
        float bankProgress;
        if (bank->requestId == NFL_REQUEST_ID_INVALID)
        {
            bankProgress = 0.0f;
        }
        else
        {
            bankProgress = (float)nflGetRequestProgress((nflRequestID)bank->requestId);
        }
        if ((bank->flags & PAK_BANK_FLAG_READ_DONE) != 0)
            bankProgress = 1.0f;
        else if (bankProgress == -1.0f)
            bankProgress = 0.0f;

        float xend1 =
            xstart + (float)((double)bank->memsize * width_per_byte);
        float xend2 =
            xstart + (float)((double)(int)bank->size * width_per_byte);

        nglQuad* v22 = (nglQuad*)nglListAlloc(0x60, 0x10);
        nglInitQuad(v22);
        nglSetQuadTex(v22, nullptr);
        nglSetQuadColor(v22, 0xFF008000);
        nglSetQuadRect(v22, xstart * ScreenWidth, y1 * ScreenHeight,
                       (xstart + (xend2 - xstart) * bankProgress)
                           * ScreenWidth,
                       y2 * ScreenHeight);
        nglListAddQuad(v22);

        nglQuad* v23 = (nglQuad*)nglListAlloc(0x60, 0x10);
        nglInitQuad(v23);
        nglSetQuadTex(v23, nullptr);
        nglSetQuadColor(v23, 0xFF808080);
        nglSetQuadRect(v23,
                       (xstart + (xend2 - xstart) * bankProgress)
                           * ScreenWidth,
                       y1 * ScreenHeight, xend2 * ScreenWidth,
                       y2 * ScreenHeight);
        nglListAddQuad(v23);

        xstart = xend1 + 0.01f;
    }
}

// ea: 0x671ED0
void* PakManager::CrazyTempMemBorrow(unsigned int align, unsigned int size)
{
    reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* i =
        m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head == mActivePaks.m_end || i == nullptr)
        return nullptr;
    while (1)
    {
        PakFile* pak = (PakFile*)m_head;
        if (pak->mState == PakFile::LOADED)
        {
            void* result = pak->MemAlloc(align, size, false);
            if (result != nullptr)
                return result;
        }
        m_head = i;
        i = i->m_next;
        if (i == nullptr)
            return nullptr;
    }
}

// ea: 0x671F30
void PakManager::CrazyTempMemGiveBack(void* ptr)
{
    reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* i =
        m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head == mActivePaks.m_end || i == nullptr)
        goto LABEL_8;
    while (!((PakFile*)m_head)->MemFree(ptr, false))
    {
        m_head = i;
        i = i->m_next;
        if (i == nullptr)
            goto LABEL_8;
    }
    return;
LABEL_8:
    {
        mem_heap* v4 = ((ae_heap*)gBrocHeap)->GetHeapPointer();
        if (v4 != nullptr && ptr >= v4->start && ptr < v4->end)
        {
            mem_heap_free(v4, ptr);
        }
        else
        {
            v4 = gActorHeap->GetHeapPointer();
            if (v4 != nullptr && ptr >= v4->start && ptr < v4->end)
                mem_heap_free(v4, ptr);
        }
    }
}

// ea: 0x671C60
TPakId PakManager::FindPakId(EPakType t) const
{
    reserved_dlist<PakFile>::dlist_node* m_node = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_node != nullptr ? m_node->m_next : nullptr;
    if (m_node == mActivePaks.m_end || m_next == nullptr)
        return PAK_ID_INVALID;
    while (((PakFile*)m_node)->mPakType != t)
    {
        m_node = m_next;
        m_next = m_next->m_next;
        if (m_next == nullptr)
            return PAK_ID_INVALID;
    }
    return ((PakFile*)m_node)->mPakId;
}

// ea: 0x671CC0
TPakId PakManager::FindPakId(const char* pak_name) const
{
    reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head == mActivePaks.m_end || m_next == nullptr)
        return PAK_ID_INVALID;
    while (1)
    {
        const PakInfoNode* v4 = ((PakFile*)m_head)->mPakInfo;
        if (v4 != nullptr
            && _stricmp((const char*)v4->longName, pak_name) == 0)
            return ((PakFile*)m_head)->mPakId;
        m_head = m_next;
        m_next = m_next->m_next;
        if (m_next == nullptr)
            return PAK_ID_INVALID;
    }
}

// ea: 0x004B45F0
PakFile* PakManager::GetPakFile(TPakId id)
{
    if (id < 0 || id > 0x62)
        return nullptr;
    return mSlots[id];
}

// ea: 0x6666B0
const PakInfoNode* PakManager::GetPakInfo(TPakId pakId) const
{
    if (mPakInfoBank == nullptr || pakId == PAK_ID_INVALID)
        return nullptr;
    const PakInfoNode* result = mPakInfoPtrs[pakId];
    if (result != nullptr)
        return result;
    PakFile* v5 = mSlots[pakId];
    if (v5 != nullptr)
    {
        if (v5->mPakType == kPakTypeCount)
            return nullptr;
        if (v5->mPakInfo != nullptr)
        {
            result = v5->GetInfo();
            const_cast<PakManager*>(this)->mPakInfoPtrs[pakId] = result;
            return result;
        }
    }
    for (unsigned int v6 = 0; v6 < mPakInfoBank->mPtrs.mSize; ++v6)
    {
        result = mPakInfoBank->mPtrs.mList[v6];
        if (result->pakId == pakId)
        {
            const_cast<PakManager*>(this)->mPakInfoPtrs[pakId] = result;
            return result;
        }
    }
    PakInfoBank* level = mLevelPakInfoBank;
    for (unsigned int v8 = 0; v8 < level->mPtrs.mSize; ++v8)
    {
        result = level->mPtrs.mList[v8];
        if (result->pakId == pakId)
        {
            const_cast<PakManager*>(this)->mPakInfoPtrs[pakId] = result;
            return result;
        }
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
    AeAssert::gCurrentLine = 610;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("no info for pak id '%d'", pakId))
        __debugbreak();
    return nullptr;
}

// ea: 0x66C660
const PakInfoNode* PakManager::GetPakInfo(const char* long_name) const
{
    if (mPakInfoBank == nullptr)
        return nullptr;
    const char* v4 = long_name;

    PakInfoBank* level = mLevelPakInfoBank;
    const PakInfoNode* result = nullptr;
    unsigned int* v6 = nullptr;
    if (level == nullptr
        || (v6 = InplaceTreeFindStringUInt(
                *(const InplaceTree<InplaceString, unsigned int>*)(void*)
                    level->mTree,
                long_name)) == nullptr
        || (result = level->mPtrs.mList[*v6]) == nullptr)
    {
        PakInfoBank* mainBank = mPakInfoBank;
        unsigned int* v8 = InplaceTreeFindStringUInt(
            *(const InplaceTree<InplaceString, unsigned int>*)(void*)
                mainBank->mTree,
            v4);
        if (v8 == nullptr
            || (result = mainBank->mPtrs.mList[*v8]) == nullptr)
        {
            if (mPakInfoBank != nullptr)
            {
                for (unsigned int v10 = 0; v10 < mPakInfoBank->mPtrs.mSize;
                     ++v10)
                {
                    const PakInfoNode* v12 = mPakInfoBank->mPtrs.mList[v10];
                    if (strstr(v12->path.mStr, v4)
                        != nullptr)
                        return v12;
                }
            }
            PakInfoBank* v13 = mLevelPakInfoBank;
            if (v13 != nullptr)
            {
                for (unsigned int v14 = 0; v14 < v13->mPtrs.mSize; ++v14)
                {
                    const PakInfoNode* v12 = v13->mPtrs.mList[v14];
                    if (_stricmp(v12->path.mStr, v4) == 0)
                        return v12;
                }
            }
            return nullptr;
        }
    }
    return result;
}

// ea: 0x685680
void PakInfoBank::Fixup()
{
    if ((uintptr_t)mPtrFixupTable >= 0x10000000)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
        AeAssert::gCurrentLine = 122;
        AeAssert::gCurrentExpr = "((unsigned)mPtrFixupTable<0x10000000)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Fixup offset is unusually large"))
            __debugbreak();
    }
    PtrFixupTable* fixupTable =
        (PtrFixupTable*)((char*)this + (uintptr_t)mPtrFixupTable);
    mPtrFixupTable = fixupTable;
    fixupTable->Fixup(this);
}

// ELanguage (core_globals.h ABI twin; values verified vs LanguageStr disasm)
enum ELanguage : int {
    kLanguageEnglish = 0,
    kLanguageGerman = 1,
    kLanguageFrench = 2,
    kLanguageSpanish = 3,
    kLanguageItalian = 4,
    kLanguageUnlocalized = 5,
    kLanguageCount = 6,
};
extern ELanguage gLanguage;  // ?gLanguage@@3W4ELanguage@@A @ 0xF00EA4
bool gCE = false;            // ?gCE@@3_NA @ 0xF91714
bool gUKBuild = false;       // ?gUKBuild@@3_NA @ 0xF91D18
extern unsigned int XGetLanguage();  // xbox platform
unsigned int XGetLanguage()
{
    return 1;  // Xbox language ID 1 is English; GetXboxLanguage subtracts 1.
}
extern void FEManager_UpdateButtonFontForLanguage(void* self);
void FEManager_UpdateButtonFontForLanguage(void* self)
{
    static_cast<FEManager*>(self)->UpdateButtonFontForLanguage();
}

// BrocSys (broc; NotifyPakLoaded/Unloaded)
namespace BrocSys {
void NotifyPakLoaded(const char* longName);
void NotifyPakUnloaded(const char* longName);
void NotifyPakLoadOperation(const char* operation, const char* longName);
}
// ea: 0x005C81A0
void BrocSys::NotifyPakUnloaded(const char* longName)
{
    BrocSys::NotifyPakLoadOperation("unloaded_", longName);
}
// ea: 0x005C8180
void BrocSys::NotifyPakLoaded(const char* longName)
{
    BrocSys::NotifyPakLoadOperation("loaded_", longName);
}

// AudioBankMgr minimal view (full class + stubs in g_entity_misc.cpp)
class AudioBankMgr {
public:
    static AudioBankMgr* sInst;  // ?sInst@AudioBankMgr@@2PAV1@A (g_entity_misc.cpp)
    void RegisterWbk(const tlFixedString& name, const char* path,
                     ELanguage lang, TPakId pak);
    void LoadWbk(const tlFixedString& name, bool async);
};

// hash_const (runtime-filled hash constants; mirrors str_const_t layout).
// ea: 0x675980
void PakManager::DecodeFLI(const char* name, PakInfoBank* data, int size,
                           TPakId pakId)
{
    (void)name; (void)size; (void)pakId;
    data->Fixup();
    if (data->mPtrs.mList[1]->pakType == (EPakType)3)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 426;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Shouldn't be here"))
            __debugbreak();
    }
    else
    {
        if (mPakInfoBank != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 430;
            AeAssert::gCurrentExpr = "mPakInfoBank==0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("fli already loaded!"))
                __debugbreak();
        }
        mPakInfoBank = data;
    }
    char* sku = mPakInfoBank->sku;
    if (gLanguage == kLanguageEnglish)
    {
        if (strstr(sku, "de") != nullptr)
        {
            gLanguage = kLanguageGerman;
        }
        else if (strstr(sku, "eu") != nullptr)
        {
            if (XGetLanguage() == 5)
                gLanguage = kLanguageFrench;
            else
                gLanguage = XGetLanguage() != 6 ? kLanguageFrench
                                                : kLanguageItalian;
        }
        else
        {
            gLanguage = kLanguageEnglish;
        }
    }
    if (strstr(sku, "ck") != nullptr || strstr(sku, "ce") != nullptr)
        gCE = true;
    if (strstr(sku, "ck") != nullptr || strstr(sku, "uk") != nullptr)
        gUKBuild = true;

    // Win32 policy: the Xbox SKU branch above can select a non-English
    // language, but this port's game-data/audio set is English-only.
    gLanguage = kLanguageEnglish;
    FEManager_UpdateButtonFontForLanguage(&g_femanager);
    UpdateInfo();
}

// ea: 0x66FE10
void PakManager::UpdateInfo()
{
    reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* i =
        m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head == mActivePaks.m_end)
    {
        i = nullptr;
        m_head = nullptr;
    }
    for (; i != nullptr; i = i->m_next)
    {
        PakFile* pak = (PakFile*)m_head;
        TPakId pakId = pak->mPakId;
        const char* path = pak->mPath.mBuff;
        for (unsigned int v5 = 0; v5 < mPakInfoBank->mPtrs.mSize; ++v5)
        {
            const PakInfoNode* node = mPakInfoBank->mPtrs.mList[v5];
            if (strstr(node->path.mStr, path) != nullptr)
            {
                if (node->pakId != PAK_ID_INVALID && node->pakId != pakId)
                {
                    AeAssert::gCurrentAuthor = AeAssert::ARO;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\PakManager.cpp";
                    AeAssert::gCurrentLine = 737;
                    AeAssert::gCurrentExpr = "(*mPakInfoBank)[i]->pakId == pakId";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("multiply loaded pak!"))
                        __debugbreak();
                }
                const_cast<PakInfoNode*>(node)->pakId = pakId;
                if (node->refCount == 0)
                    ++const_cast<PakInfoNode*>(node)->refCount;
            }
        }
        if (mLevelPakInfoBank != nullptr)
        {
            for (unsigned int v19 = 0;
                 v19 < mLevelPakInfoBank->mPtrs.mSize; ++v19)
            {
                const PakInfoNode* node =
                    mLevelPakInfoBank->mPtrs.mList[v19];
                if (strstr(node->path.mStr, path)
                    != nullptr)
                {
                    if (node->pakId != PAK_ID_INVALID
                        && node->pakId != pakId)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\PakManager.cpp";
                        AeAssert::gCurrentLine = 754;
                        AeAssert::gCurrentExpr =
                            "(*mLevelPakInfoBank)[i]->pakId == pakId";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("multiply loaded pak!"))
                            __debugbreak();
                    }
                    const_cast<PakInfoNode*>(node)->pakId = pakId;
                    if (node->refCount == 0)
                        ++const_cast<PakInfoNode*>(node)->refCount;
                }
            }
        }
        if (i == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/reserved_dlist.h";
            AeAssert::gCurrentLine = 501;
            AeAssert::gCurrentExpr = "m_next != 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Please add a descriptive string"))
                __debugbreak();
        }
        m_head = i;
    }
}

// ============================================================================
// PakManager load/unload state machine (streamer.o PakManager.cpp)
// ============================================================================

// ea: 0x6775A0
PakFile::PakFile(EPakType pak_type, const char* path, TPakId id,
                 NumBanks numBanks)
{
    const char* v11 = path;
    m_dlist_node.m_next = nullptr;
    m_dlist_node.m_prev = nullptr;
    int oLen = 0;
    AeStringSupport::CStrToAeStr(mPath.mBuff, &oLen, 63, v11);
    mPath.mLength = (unsigned char)oLen;
    mPakType = pak_type;
    mFilesize = 0;
    mFileId = NFL_FILE_ID_INVALID;
    mBankAlloc.Clear();
    mSerializedAlloc.Clear();
    mPakId = id;
    mPakInfo = nullptr;
    mHeader = nullptr;
    mDefaultSectionIdx = -1;
    mHeaderBuffer = nullptr;
    mApkFiles.mElements = nullptr;
    mApkFiles.mCapacity = 0;
    mApkFiles.mSize = 0;
    mOnlyLoadHeader = false;
    mCloseHandle = true;
    mHeapList.m_size = 0;
    mPrereqHeaps.m_size = 0;
    mCurrentFile = -1;
    mCurrentFilePtr = nullptr;
    mState = LOADING;
    mLoadingState = (ELoadingState)LOADING_DONE;
    mLooseFiles.mElements = nullptr;
    mLooseFiles.mCapacity = 0;
    mLooseFiles.mSize = 0;
    tlPrintf("pak: loading [%d] %s\n", id, mPath.mBuff);
    LoadStats* v7 = (LoadStats*)mem_heap_malloc(0x20u);
    if (v7 != nullptr)
    {
        v7->totalStart = 0;
        v7->total = 0.0f;
        v7->readStart = 0;
        v7->readTotal = 0.0f;
    }
    mLoadStats = v7;
    mLoadStats->totalStart = __rdtsc();
    mHeaderRequestId.nflId = NFL_REQUEST_ID_INVALID;
    g_bytes_read = 0;
    gThroughputMeasurer.mBytes = 0;
    gThroughputMeasurer.mTotalBytes = 0;
    g_throughput = 0.0f;
    gThroughputMeasurer.mStart = 0.0f;
    gThroughputMeasurer.mTotalTime = 0.0f;
    AsyncLoad(numBanks);
}

// ea: 0x678FA0
PakFile::~PakFile()
{
    EState mState = this->mState;
    if (mState != PakFile::LOADED)
    {
        int v3 = (int)mState - 1;
        if (v3 != 0)
        {
            if (v3 == 1)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 303;
                AeAssert::gCurrentExpr = nullptr;
                if (AeAssert::Error(
                        "attempted to destroy PakFile while unloading"))
                    __debugbreak();
            }
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
            AeAssert::gCurrentLine = 298;
            AeAssert::gCurrentExpr = nullptr;
            if (AeAssert::Error(
                    "attempted to destroy PakFile while loading"))
                __debugbreak();
        }
    }
    else
    {
        BeginAsyncUnload();
        UnloadSerialized();
        UnloadInplace();
        FinishUnload();
    }
    if (mHeaderBuffer != PakFile::sHeaderBuffer)
    {
        unsigned char* mHeaderBuffer = this->mHeaderBuffer;
        if (mPakType == kPakTypeFrontEnd)
            gActorHeap->Free(mHeaderBuffer);
        else
            mem_heap_free(mHeaderBuffer);
        this->mHeaderBuffer = nullptr;
        mHeader = nullptr;
    }
    mem_heap** p_mHeapList = mHeapList.m_elements;
    mem_heap** v5 = &mHeapList.m_elements[mHeapList.m_size];
    if (p_mHeapList != v5)
    {
        do
        {
            gPakMemHeapAllocator->Release(*p_mHeapList++);
        } while (p_mHeapList != v5);
    }
    if (mLooseFiles.mElements != nullptr)
    {
        tlMemFree(mLooseFiles.mElements);
        mLooseFiles.mElements = nullptr;
        mLooseFiles.mCapacity = 0;
    }
    if (mApkFiles.mElements != nullptr)
    {
        tlMemFree(mApkFiles.mElements);
        mApkFiles.mElements = nullptr;
        mApkFiles.mCapacity = 0;
    }
}

// ea: 0x677C90
TPakId PakManager::AsyncLoadPak(EPakType pak_type, const char* path,
                                NumBanks num_banks)
{
    nflMediaID v4 = (nflMediaID)gNflMediaId;
    if (nflFileExists(v4, path) != 0
        || (GetPakInfo(path) != nullptr
            && (path = GetPakInfo(path)->path.mStr,
                nflFileExists(v4, path) != 0)))
    {
        TPakId mPakIdServer = this->mPakIdServer;
        const TPakId pakId = mPakIdServer;
        PakFile* v8 =
            (PakFile*)PakFile::sAllocator->Allocate(0x114u, false);
        PakFile* v9 = v8;
        if (v9 != nullptr)
            v9->PakFile::PakFile(pak_type, path, mPakIdServer, num_banks);
        v9->m_dlist_node.m_next =
            (void*)&mActivePaks.m_end;
        reserved_dlist<PakFile>::dlist_node* m_tail = mActivePaks.m_tail;
        v9->m_dlist_node.m_prev = (void*)m_tail;
        m_tail->m_next =
            (reserved_dlist<PakFile>::dlist_node*)&v9->m_dlist_node;
        mActivePaks.m_size += 1;
        mActivePaks.m_tail =
            (reserved_dlist<PakFile>::dlist_node*)&v9->m_dlist_node;
        mSlots[mPakIdServer] = v9;
        if (v9->mPakType == kPakTypeZone)
        {
            int v13 = mNumZonesLoaded + 1;
            mNumZonesLoaded = v13;
            if (v13 >= mActivePaks.m_size)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 1202;
                AeAssert::gCurrentExpr =
                    "mNumZonesLoaded < mActivePaks.size()";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                           "error calculating num zones loaded"))
                    __debugbreak();
            }
        }
        TPakId v14 = mPakIdServer;
        do
            v14 = (TPakId)((v14 + 1) % 99);
        while (mSlots[v14] != nullptr);
        if (v14 > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 1212;
            AeAssert::gCurrentExpr =
                "new_server >= 0 && new_server < PAK_ID_MAX";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("server out of range!"))
                __debugbreak();
        }
        if (mSlots[v14] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 1213;
            AeAssert::gCurrentExpr = "mSlots[new_server] == 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("slot already used!"))
                __debugbreak();
        }
        this->mPakIdServer = v14;
        mState = STATE_LOADING;
        mCurrentPakId = pakId;
        if (pak_type != kPakTypeGlobal)
        {
            switch (pak_type)
            {
            case kPakTypeAnimation:
                mAnimPakId = pakId;
                break;
            case kPakTypeLevel:
                mLevelPakId = pakId;
                break;
            case kPakTypeCount:
                mDebugPakId = pakId;
                break;
            default:
                break;
            }
        }
        else
        {
            mGlobalPakId = pakId;
        }
        return pakId;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 1176;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("FILE NOT FOUND: %s:\n"
                            "check your \"asset manager\" output for details",
                            path))
            __debugbreak();
        tlPrintf("FILE NOT FOUND: %s\n", path);
        return PAK_ID_INVALID;
    }
}

// ea: 0x67B290
TPakId PakManager::SyncLoadPak(EPakType pak_type, const char* path,
                               NumBanks num_banks)
{
    TPakId Pak = AsyncLoadPak(pak_type, path, num_banks);
    if (Pak == PAK_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 985;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("couldn't load pak %s!", path))
            __debugbreak();
        return PAK_ID_INVALID;
    }
    do
    {
        controller::inst()->poll();
        Update(false);
    } while (mState == STATE_LOADING);
    return Pak;
}

// ea: 0x67B1C0
TPakId PakManager::SyncLoadPak(const PakInfoNode* cpak)
{
    TPakId result = cpak->pakId;
    if (result == PAK_ID_INVALID || mSlots[result] == nullptr
        || mSlots[result]->mState != PakFile::LOADED)
    {
        const_cast<PakInfoNode*>(cpak)->userDistance = 0.0f;
        ++sComputeDistanceKey;
        const PakInfoNode* i;
        while ((i = GetUnloadedPrereq(cpak)) != nullptr)
            SyncLoadPak(i);
        mCurrentPakInfo = const_cast<PakInfoNode*>(cpak);
        TPakId Pak = AsyncLoadPak(cpak->pakType,
                                  cpak->path.mStr,
                                  cpak->numBanks);
        const_cast<PakInfoNode*>(cpak)->pakId = Pak;
        if (Pak != PAK_ID_INVALID)
        {
            do
            {
                Update(false);
                controller::inst()->poll();
            } while (mState == STATE_LOADING);
        }
        return cpak->pakId;
    }
    return result;
}

// ea: 0x67B170
void PakManager::FillBanks()
{
    mFilled = false;
    mRequestedStopLoading = false;
    mFillingBanks = true;
    do
    {
        Update(false);
        controller::inst()->poll();
        MultiplayerMgr_Step(MultiplayerMgr_sInst, 0, !gMPLoadingUnthreaded,
                            true);
    } while (!mFilled);
    mFillingBanks = false;
}

// ea: 0x67B140
void PakManager::UnloadAll()
{
    ResetPriorities(true);
    mFilled = false;
    do
        Update(false);
    while (!mFilled);
}

// ea: 0x67B340
void PakManager::SyncUnloadPak(TPakId id)
{
    AsyncUnloadPak(id);
    do
        Update(false);
    while (mState == STATE_UNLOADING);
    if (mSlots[id] == nullptr)
    {
        reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
        reserved_dlist<PakFile>::dlist_node* v4 =
            m_head != nullptr ? m_head->m_next : nullptr;
        if (m_head != mActivePaks.m_end && v4 != nullptr)
        {
            while (((PakFile*)m_head)->mPakId != id)
            {
                m_head = v4;
                v4 = v4->m_next;
                if (v4 == nullptr)
                    return;
            }
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 1345;
            AeAssert::gCurrentExpr = nullptr;
            if (AeAssert::Error(
                    "SyncUnloadPak: active pak not in mSlots list!"))
                __debugbreak();
        }
    }
}

// ea: 0x670370
void PakManager::AsyncUnloadPak(TPakId id)
{
    mCurrentPakId = id;
    mState = STATE_UNLOADING;
    PakFile* v3 = mSlots[id];
    if (id == PAK_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 1309;
        AeAssert::gCurrentExpr = "id != PAK_ID_INVALID";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Make sure the level you are loading has "
                                "been built by AssetManager\n"))
            __debugbreak();
    }
    if (v3->mPakType != kPakTypeCount)
    {
        if (v3->mPakInfo == nullptr)
            v3->mPakInfo = PakManager::sInst->GetPakInfo(v3->mPakId);
        if (v3->mPakInfo != nullptr && v3->mPakType != kPakTypeCount)
            mCurrentPakInfo = const_cast<PakInfoNode*>(v3->mPakInfo);
        else
            mCurrentPakInfo = nullptr;
    }
    else
    {
        mCurrentPakInfo = nullptr;
    }
    if (v3->mPakType == kPakTypeZone && --mNumZonesLoaded < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 1316;
        AeAssert::gCurrentExpr = "mNumZonesLoaded >= 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("error calculating num zones loaded"))
            __debugbreak();
    }
    v3->BeginAsyncUnload();
}

// ea: 0x67B060
void PakManager::Update(bool calledFromMovie)
{
    (void)calledFromMovie;
    if (mEnabled == 0)
        return;
    nglDebug.DisableDuplicateMaterialWarning = 1;
    ProgressUpdate();
    reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head != (reserved_dlist<PakFile>::dlist_node*)&mActivePaks.m_end
        && m_next != nullptr)
    {
        do
        {
            PakFile* pak = (PakFile*)m_head;
            if (pak->mState == PakFile::LOADING)
                pak->UpdateLoading();
            else if (pak->mState == PakFile::UNLOADING)
                pak->UpdateUnloading();
            m_head = m_next;
            m_next = m_next->m_next;
        } while (m_next != nullptr);
    }
    if (mState != STATE_LOADING)
    {
        int v6 = mState - 1;
        if (v6 != 0)
        {
            if (v6 == 1)
            {
                UpdateNormal();
            }
            else
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 1463;
                AeAssert::gCurrentExpr = nullptr;
                if (AeAssert::Error("unknown PakManager state"))
                    __debugbreak();
            }
        }
        else
        {
            UpdateUnloading();
        }
    }
    else
    {
        UpdateLoading();
    }
}

// ea: 0x671600
void PakManager::ProgressUpdate()
{
    if (mProgressCallback == nullptr)
        return;
    float total = BankManager::sInst->mNumMramBanks;
    float count = 0.0f;
    reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head
        == (reserved_dlist<PakFile>::dlist_node*)&mActivePaks.m_end)
    {
        m_next = nullptr;
        m_head = nullptr;
    }
    if (m_next != nullptr)
    {
        while (1)
        {
            PakFile* pak = (PakFile*)m_head;
            if (pak->mState != PakFile::LOADING
                || pak->mDefaultSectionIdx == -1)
            {
                count += pak->mBankAlloc.ToFloat();
            }
            else if (pak->mLoadingState == PakFile::LOADING_DATA
                     || pak->mLoadingState == PakFile::LOADING_DONE)
            {
                // count loaded banks of the section currently being read
                PakHeader::Section& sec =
                    pak->mHeader->sections[pak->mDefaultSectionIdx];
                unsigned int mramSize = BankManager::sInst->mMramBankSize;
                for (unsigned int i = 0; i < sec.numBanks; ++i)
                {
                    PakHeader::Bank& bank = sec.banks[i];
                    if ((bank.flags & PAK_BANK_FLAG_READ_DONE) != 0)
                    {
                        if (bank.memsize == mramSize)
                            count += 1.0f;
                        else if (bank.memsize == (mramSize >> 1))
                            count += 0.5f;
                    }
                }
            }
            if (m_next == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/reserved_dlist.h";
                AeAssert::gCurrentLine = 501;
                AeAssert::gCurrentExpr = "m_next != 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Please add a descriptive string"))
                    __debugbreak();
            }
            m_head = m_next;
            if (m_next->m_next == nullptr)
                break;
            m_next = m_next->m_next;
        }
    }
    mProgressCallback((1.0f - sBrocPercentage - sWbkPercentage)
                      * (count / total));
}

// ea: 0x66C620
void PakManager::Reset()
{
    mFilled = false;
    mNextPakInfo = nullptr;
    mPakIdServer = PAK_ID_MIN;
    mCurSection = nullptr;
    mCurrentPakInfo = nullptr;
    const PakInfoNode** mPakInfoPtrs = this->mPakInfoPtrs;
    for (int i = 100; i != 0; --i)
    {
        *(mPakInfoPtrs - 99) = nullptr;
        *mPakInfoPtrs++ = nullptr;
    }
    mSectionStates.mBits[0] = 0;
}

// ea: 0x6791F0
PakManager::PakManager()
{
    mPakInfoBank = nullptr;
    mLevelPakInfoBank = nullptr;
    mAnimPakId = PAK_ID_INVALID;
    mGlobalPakId = PAK_ID_INVALID;
    mLevelPakId = PAK_ID_INVALID;
    mDebugPakId = PAK_ID_INVALID;
    mSectionStates.mBits[0] = 0;
    mProgressCallback = nullptr;
    mRequestedStopLoading = false;
    mAudioBanks.mElements = nullptr;
    mAudioBanks.mCapacity = 0;
    mAudioBanks.mSize = 0;
    mStbVector.mElements = nullptr;
    mStbVector.mCapacity = 0;
    mStbVector.mSize = 0;
    mEnabled = 1;
    mActivePaks.m_end = nullptr;
    mActivePaks.m_size = 0;
    mActivePaks.m_head =
        (reserved_dlist<PakFile>::dlist_node*)&mActivePaks.m_end;
    mActivePaks.m_tail =
        (reserved_dlist<PakFile>::dlist_node*)&mActivePaks.m_head;
    mContextStack[0].m_size = 0;
    mNumZonesLoaded = 0;
    memset(&mDebugRenderMode, 0, sizeof(mDebugRenderMode));
    Cmd_AddCommand("PakRender", TogglePakRender);
    mDebugRenderMode.progressbar = Cvar_Get("progressbar", "0", 0);
    mDebugRenderMode.paks = Cvar_Get("renderpaks", "0", 0);
    mDebugRenderMode.heaps = Cvar_Get("renderheaps", "0", 0);
    Reset();
    PakFile::sHeaderBufferSize = 36864;
    PakFile::sHeaderBuffer = stream_alloc(36864, false);
    for (int i = 99; i != 0; --i)
    {
        mSlots[99 - i] = nullptr;
        mPakInfoPtrs[99 - i] = nullptr;
    }
    DebugRender_AddRenderer(DebugRender_sInst,
                            (void*)&PakManager::SingletonDebugRender);
    Cmd_AddCommand("SetPakDistance", ConsoleSetPakDistance);
    Cmd_AddCommand("ClearPakDistance", ConsoleClearPakDistance);
    memset(mContextStack, 0, sizeof(TThreadedPakContextStack));
    cdInitApk();
}

// ea: 0x66FB80
PakManager::~PakManager()
{
    Cmd_RemoveCommand("PakRender");
    unsigned char* v4 = PakFile::sHeaderBuffer;
    PakFile::sHeaderBufferSize = 0;
    PakFile::sHeaderBuffer = nullptr;
    stream_free(v4, false);
    if (mStbVector.mElements != nullptr)
    {
        tlMemFree(mStbVector.mElements);
        mStbVector.mElements = nullptr;
        mStbVector.mCapacity = 0;
    }
    if (mAudioBanks.mElements != nullptr)
    {
        tlMemFree(mAudioBanks.mElements);
        mAudioBanks.mElements = nullptr;
        mAudioBanks.mCapacity = 0;
    }
}

// ea: 0x66C770 (defined below)
float PrintBankUsage(char* res_buf, float total_banks,
                     const TBankAlloc* alloc, bool mram);

// ea: 0x670480
void PakManager::DebugRender()
{
    if (BankManager::sInst == nullptr)
        return;

    // progressbar section: RenderDebug for every loading pak
    if (mDebugRenderMode.progressbar != nullptr
        && mDebugRenderMode.progressbar->integer != 0)
    {
        for (PakFile* pak = (PakFile*)mActivePaks.m_head;
             pak != nullptr
             && (reserved_dlist<PakFile>::dlist_node*)pak
                    != mActivePaks.m_end;
             pak = (PakFile*)pak->m_dlist_node.m_next)
        {
            if (pak->mState == PakFile::LOADING)
                pak->RenderDebug();
        }
    }

    float textScale = 0.9f;  // textScale_0 @ 0xDF9130
    if ((mDebugRenderMode.bankUsage & 1) != 0)
    {
        char v74[1024];
        char v73[512];
        float x = 20.0f;
        float y = 20.0f;

        sprintf(v74, "ACTIVE PAKS:\n");
        MyRenderText(v74, (int)x, (int)y, Color(1.0f, 1.0f, 1.0f, 1.0f),
                     0.0f, textScale);
        y += 14.0f;

        float totalBanks = BankManager::sInst->mNumMramBanks;
        TBankAlloc freeAlloc = BankManager::sInst->mFreeBanks;
        double freeCount = PrintBankUsage(v73, totalBanks, &freeAlloc, true);
        sprintf(v74, "%s free: %1.1f/%1.1f \n", v73, freeCount, totalBanks);
        MyRenderText(v74, (int)x, (int)y, Color(1.0f, 1.0f, 1.0f, 1.0f),
                     0.0f, textScale);
        y += 14.0f;

        for (PakFile* pak = (PakFile*)mActivePaks.m_head;
             pak != nullptr
             && (reserved_dlist<PakFile>::dlist_node*)pak
                    != mActivePaks.m_end;
             pak = (PakFile*)pak->m_dlist_node.m_next)
        {
            const char* v17;
            if (pak->mPakType == kPakTypeCount)
            {
                v17 = (const char*)pak + 12;  // mPath.mBuff
            }
            else
            {
                if (pak->mPakInfo == nullptr)
                    pak->mPakInfo = PakManager::sInst->GetPakInfo(pak->mPakId);
                if (pak->mPakInfo == nullptr)
                    v17 = (const char*)pak + 12;
                else
                    v17 =
                        pak->mPakInfo->path.mStr;
            }

            ae_fixed_string<256, unsigned short> v77;
            int oLen = 0;
            AeStringSupport::CStrToAeStr((char*)v77.mBuff, &oLen, 254, v17);
            v77.mLength = (unsigned char)oLen;
            ae_fixed_string<256, unsigned short> v75 =
                v77.get_file_name(true);
            NumBanks v78;
            v78.xenon = *(float*)&pak->mBankAlloc.mram_alloc1.mBits[0];
            v78.pcx = *(float*)&pak->mBankAlloc.mram_alloc1.mBits[1];
            v78.gc.main = *(float*)&pak->mBankAlloc.mram_alloc2.mBits[0];
            v78.gc.aram = *(float*)&pak->mBankAlloc.mram_alloc2.mBits[1];
            if (totalBanks + 0.5f >= 256.0f)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 1646;
                AeAssert::gCurrentExpr =
                    "((float)((int)(BankManager::Inst()->GetNumBanks()+0.5f)))<256";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("check"))
                    __debugbreak();
            }
            PrintBankUsage(v73, totalBanks, (const TBankAlloc*)&v78.xenon,
                           true);
            sprintf(v74, "%s%s", v73, v75.mBuff);

            float loadTime = 0.0f;
            if (pak->mLoadStats != nullptr)
                loadTime = pak->mLoadStats->total;
            const PakInfoNode* pdt = (pak->mPakType == kPakTypeCount)
                                         ? nullptr
                                         : pak->mPakInfo;
            if (pdt == nullptr && pak->mPakType != kPakTypeCount)
            {
                pak->mPakInfo = PakManager::sInst->GetPakInfo(pak->mPakId);
                pdt = pak->mPakInfo;
            }
            float banks = 0.0f;
            if (pdt != nullptr)
                banks = pdt->numBanks.xbox;
            if (pdt == nullptr
                || GetDistance((PakInfoNode*)pdt) == 3.4028235e38f)
            {
                sprintf(v74, "%s %2.1fbnk (inf) (%1.2fs)\n", v74, banks,
                        loadTime);
            }
            else if (GetDistance((PakInfoNode*)pdt) >= 50.0)
            {
                sprintf(v74, "%s %2.1fbnk (%1.fm) (%1.2fs)\n", v74, banks,
                        GetDistance((PakInfoNode*)pdt) * 0.0254, loadTime);
            }
            else
            {
                sprintf(v74, "%s %2.1fbnk (%1.f\") (%1.2fs)\n", v74, banks,
                        GetDistance((PakInfoNode*)pdt), loadTime);
            }
            MyRenderText(v74, (int)x, (int)y, Color(1.0f, 1.0f, 1.0f, 1.0f),
                         0.0f, textScale);
            y += 14.0f;
        }

        if (mNextPakInfo != nullptr && mNextPakInfo != mCurrentPakInfo)
        {
            ae_fixed_string<256, unsigned short> v77;
            int oLen = 0;
            AeStringSupport::CStrToAeStr(
                (char*)v77.mBuff, &oLen, 254,
                mNextPakInfo->path.mStr);
            v77.mLength = (unsigned char)oLen;
            ae_fixed_string<256, unsigned short> v75 =
                v77.get_file_name(true);
            char v78[32];
            int v90 = 31;
            AeStringSupport::AeStrCopy(v78, &v90, 31, (const char*)v75.mBuff,
                                       v75.mLength);
            if (GetDistance((PakInfoNode*)mNextPakInfo)
                == 3.4028235e38f)
            {
                NumBanks nb = GetNumBanks(mNextPakInfo);
                sprintf(v74, "---next:---\n%s %1.1fbnk (inf)\n", v78,
                        nb.xbox);
            }
            else
            {
                NumBanks nb = GetNumBanks(mNextPakInfo);
                sprintf(v74, "---next:---\n%s %1.1fbnk (%1.fm)\n", v78,
                        nb.xbox,
                        GetDistance((PakInfoNode*)mNextPakInfo) * 0.0254);
            }
            MyRenderText(v74, (int)x, (int)y,
                         Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, textScale);
            y += 48.0f;
        }

        sprintf(v74, "Max Used:%3.2f\tLow water mark:%3.2f\n",
                BankManager::sInst->mNumMramBanks
                    - BankManager::sInst->mLowestFreeAmount,
                BankManager::sInst->mLowestFreeAmount);
        MyRenderText(v74, (int)x, (int)y, Color(1.0f, 1.0f, 1.0f, 1.0f),
                     0.0f, textScale);
    }

    if (mDebugRenderMode.paks != nullptr
        && mDebugRenderMode.paks->integer != 0)
    {
        char v76[256];
        float x = 34.0f;
        float y = 40.0f;
        NumBanks freeCount = BankManager::sInst->get_free_count();
        sprintf(v76, "used(%1.1f) free(%1.1f)",
                BankManager::sInst->mNumMramBanks - freeCount.xbox,
                freeCount.xbox);
        MyRenderText(v76, (int)x, (int)y, Color(1.0f, 1.0f, 1.0f, 1.0f),
                     0.0f, 0.9f);
        y += 11.0f;

        for (PakFile* pak = (PakFile*)mActivePaks.m_head;
             pak != nullptr
             && (reserved_dlist<PakFile>::dlist_node*)pak
                    != mActivePaks.m_end;
             pak = (PakFile*)pak->m_dlist_node.m_next)
        {
            const PakInfoNode* pdt =
                (pak->mPakType == kPakTypeCount)
                    ? nullptr
                    : pak->mPakInfo;
            if (pdt == nullptr && pak->mPakType != kPakTypeCount)
            {
                pak->mPakInfo = PakManager::sInst->GetPakInfo(pak->mPakId);
                pdt = pak->mPakInfo;
            }
            if (pdt == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 2005;
                AeAssert::gCurrentExpr = "pdt != 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
            }
            TBankAlloc v79[2];
            memcpy(v79, (const char*)pdt + 12, sizeof(v79));
            sprintf(v76, "%2.1f %3d %8s",
                    *(float*)&v79[0].mram_alloc2.mBits[1], pak->mPakId,
                    (const char*)pdt->longName);
            MyRenderText(v76, (int)x, (int)y, Color(1.0f, 1.0f, 1.0f, 1.0f),
                         0.0f, 0.9f);
            y += 15.0f;
        }
    }

    if (mDebugRenderMode.heaps != nullptr
        && mDebugRenderMode.heaps->integer != 0)
    {
        char v76[256];
        float x = 26.0f;
        float y = 20.0f;
        unsigned int totalUsed = 0;
        unsigned int totalSize = 0;

        for (PakFile* pak = (PakFile*)mActivePaks.m_head;
             pak != nullptr
             && (reserved_dlist<PakFile>::dlist_node*)pak
                    != mActivePaks.m_end;
             pak = (PakFile*)pak->m_dlist_node.m_next)
        {
            const PakInfoNode* pdt =
                (pak->mPakType == kPakTypeCount)
                    ? nullptr
                    : pak->mPakInfo;
            if (pdt == nullptr && pak->mPakType != kPakTypeCount)
            {
                pak->mPakInfo = PakManager::sInst->GetPakInfo(pak->mPakId);
                pdt = pak->mPakInfo;
            }
            for (int v55 = 0; v55 < pak->mHeapList.m_size; ++v55)
            {
                if (v55 >= 0xC)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                    AeAssert::gCurrentLine = 148;
                    AeAssert::gCurrentExpr =
                        "idx >= 0 && idx < _CAPACITY";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("out of bounds"))
                        __debugbreak();
                }
                mem_heap* heap = pak->mHeapList.m_elements[v55];
                if (heap == nullptr)
                    continue;
                sprintf(v76, "[%d] %10s %7d(%7d)", pak->mPakId,
                        (const char*)pdt->longName, heap->used_byte,
                        heap->size);
                totalUsed += heap->used_byte;
                totalSize += heap->size;
                MyRenderText(v76, (int)x, (int)y,
                             Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, textScale);
                y += 15.0f;
            }
        }

        sprintf(v76, "%10s %7d(%7d)", "TOTAL", totalUsed, totalSize);
        MyRenderText(v76, (int)x, (int)y, Color(1.0f, 1.0f, 1.0f, 1.0f),
                     0.0f, textScale);
        y += 15.0f;

        mem_heap* apsHeap = nullptr;
        if (gApsHeap != nullptr)
            apsHeap = ((ae_heap*)gApsHeap)->GetHeapPointer();
        sprintf(v76, "apsHeap %7d(%7d)",
                apsHeap ? apsHeap->used_byte : 0,
                apsHeap ? apsHeap->size : 0);
        MyRenderText(v76, (int)x, (int)y, Color(1.0f, 1.0f, 1.0f, 1.0f),
                     0.0f, textScale);
    }
}

// ea: 0x687700
void PakManager::SingletonDebugRender()
{
    PakManager::sInst->DebugRender();
}

// apk callbacks (apkSupport.cpp)

// ngl_sysfont/ngl_default/charskel/missing_effect (apkSupport.cpp statics)
static tlFixedString ngl_sysfont_name("ngl_sysfont");
static tlFixedString ngl_default_name("ngl_default");
static tlFixedString charskel_name("charskel");
static tlFixedString missing_effect_name("missing_effect");

// ea: 0x66E090
void cdLoadTextureCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                           void* UserData)
{
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglTexture* Data = (nglTexture*)Entry->GetData(File, SectionIndex, true);
    if (Data != nullptr)
    {
        ngliProcessTexture(File, Entry);
        Data->Flags |= 8;
        if (*Data->FileName == ngl_default_name)
            nglDefaultTex = Data;
        ae_sized_array<TPakId, 128>& ContextStack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst
                ->GetContextStack();
        TPakId v8 = PAK_ID_INVALID;
        if (ContextStack.m_size != 0)
            v8 = ContextStack.m_elements[ContextStack.m_size - 1];
        InstanceBankMgr::sInst->Add(INSTBANK_TYPE_TEXTURE, v8,
                                    *Data->FileName, (unsigned int)Data);
    }
}

// ea: 0x66E170
void cdLoadMeshCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                        void* UserData)
{
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglMesh* Data = (nglMesh*)Entry->GetData(File, SectionIndex, true);
    if (Data != nullptr)
    {
        Data->File = File;
        Data->LastFrameRef = -1;
        nglProcessMesh(Data, Entry);
        Data->Flags |= 0x1010000;
        ae_sized_array<TPakId, 128>& ContextStack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst
                ->GetContextStack();
        TPakId v6 = PAK_ID_INVALID;
        if (ContextStack.m_size != 0)
            v6 = ContextStack.m_elements[ContextStack.m_size - 1];
        InstanceBankMgr::sInst->Add(INSTBANK_TYPE_MESH, v6, *Data->Name,
                                    (unsigned int)Data);
    }
}

// ea: 0x665CB0
void cdDeleteMeshCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                          void* UserData)
{
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglMesh* Data = (nglMesh*)Entry->GetData(File, SectionIndex, true);
    if ((Data->Flags & 0x10000) != 0)
    {
        if (!nglCanReleaseMesh(Data))
        {
            tlWarning("NGL: Mesh %s destroyed while still referenced by the async renderer.\n",
                      Data->Name->str);
            ngliWaitForResource();
        }
        nglUnloadMesh(Data);
    }
}

// ea: 0x665DE0
void cdLoadSkelCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                        void* UserData)
{
    (void)UserData;
    if (*(tlFixedString*)Entry->Name == charskel_name)
    {
        tlFixedString name("image");
        int SectionIndex = File->GetSectionIndex(name);
        gCharSkel =
            (unsigned char*)Entry->GetData(File, SectionIndex, true);
    }
}

// ea: 0x665E60 (empty no-op)
void cdDeleteSkelCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                          void* UserData)
{
    (void)File; (void)Entry; (void)UserData;
}

// ea: 0x66E2A0
void cdLoadFontCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                        void* UserData)
{
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglFont* Data = (nglFont*)Entry->GetData(File, SectionIndex, true);
    if (Data != nullptr)
    {
        int* p_Width = &Data->Texture->Width;
        Data->MapFlags = 1;
        Data->BlendMode = 0x64CF8600;
        float invW = 1.0f / (float)p_Width[0];
        float invH = 1.0f / (float)p_Width[1];
        for (int i = 0; i < Data->Header.NumGlyphs; ++i)
        {
            Data->TexCoords[4 * i + 0] *= invW;
            Data->TexCoords[4 * i + 1] *= invH;
            Data->TexCoords[4 * i + 2] *= invW;
            Data->TexCoords[4 * i + 3] *= invH;
        }
        if (*Data->FileName == ngl_sysfont_name)
        {
            nglSysFont = Data;
        }
        else
        {
            FEManagerSetFont(Data, Data->FileName->str);
        }
        ae_sized_array<TPakId, 128>& ContextStack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst
                ->GetContextStack();
        TPakId v13 = PAK_ID_INVALID;
        if (ContextStack.m_size != 0)
            v13 = ContextStack.m_elements[ContextStack.m_size - 1];
        InstanceBankMgr::sInst->Add(INSTBANK_TYPE_FONT, v13,
                                    *Data->FileName, (unsigned int)Data);
    }
}

// ea: 0x665D30
void cdDeleteFontCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                          void* UserData)
{
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglFont* Data = (nglFont*)Entry->GetData(File, SectionIndex, true);
    auxFontDirectoryDelete(Data);
}

// ea: 0x665E70
void cdLoadMaterialCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                            void* UserData)
{
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglMaterial* Data = (nglMaterial*)Entry->GetData(File, SectionIndex, true);
    nglProcessMaterial(Data);
}

// ea: 0x665EB0 (empty no-op)
void cdDeleteMaterialCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                              void* UserData)
{
    (void)File; (void)Entry; (void)UserData;
}

// ea: 0x66E200
void cdLoadParticleCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                            void* UserData)
{
    (void)UserData;
    apsEffectTemplate* EffectInplace = apsLoadEffectInplace(File, Entry);
    if (*(tlFixedString*)Entry->Name == missing_effect_name)
        sMissingParticleEffect = EffectInplace;
    ae_sized_array<TPakId, 128>& ContextStack =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    TPakId v6 = PAK_ID_INVALID;
    if (ContextStack.m_size != 0)
        v6 = ContextStack.m_elements[ContextStack.m_size - 1];
    InstanceBankMgr::sInst->Add(INSTBANK_TYPE_EFFECT, v6,
                                *(tlFixedString*)Entry->Name,
                                (unsigned int)EffectInplace);
}

// ea: 0x665D20 (empty no-op)
void cdDeleteParticleCallback(apk::apkFile* File, apk::apkFileEntry* Entry,
                              void* UserData)
{
    (void)File; (void)Entry; (void)UserData;
}

// ea: 0x678EF0
void cdInitApk()
{
    extern void nglSetResourceCallback(
        void* (*Callback)(const tlFixedString&, unsigned int));
    extern void* apkGetResource(const tlFixedString& FileName,
                                unsigned int FourCC);
    nglSetResourceCallback(apkGetResource);
    apk::apkSetResourceCallback(apkGetResource);
    apk::apkRegisterFileType(0x584554u, 3u, cdLoadTextureCallback,
                             cdDeleteTextureCallback, nullptr);
    apk::apkRegisterFileType(0x4853454Du, 2u, cdLoadMeshCallback,
                             cdDeleteMeshCallback, nullptr);
    apk::apkRegisterFileType(0x4C454B53u, 2u, cdLoadSkelCallback,
                             cdDeleteSkelCallback, nullptr);
    apk::apkRegisterFileType(0x544E4F46u, 1u, cdLoadFontCallback,
                             cdDeleteFontCallback, nullptr);
    apk::apkRegisterFileType(0x54414Du, 2u, cdLoadMaterialCallback,
                             cdDeleteMaterialCallback, nullptr);
    apk::apkRegisterFileType(0x53504541u, 1u, cdLoadParticleCallback,
                             cdDeleteParticleCallback, nullptr);
}

// Resource getters (apkSupport.cpp; decode impls port with the ngl batch)
class apsEffectTemplate;
class nalBaseSkeleton;
template <typename T> class nalAnimClass;
class nalAnyPose;
struct MultiApk;
const tlFixedString* GetKey(const nglMesh* m);  // ngl_internal.cpp
void GetAllPaks(ae_sized_array<TPakId, 32>* ret);  // streamer.o (defined below)
void GetPakPrerequisites(TPakId pakId,
                         ae_sized_array<TPakId, 32>* ret);  // streamer.o
apsEffectTemplate* sMissingParticleEffect =
    nullptr;  // ?sMissingParticleEffect@@3PAVapsEffectTemplate@@A @ 0xF59310
static tlFixedString none_tfs("None");
static tlFixedString null_tfs("(null)");
struct nglMaterial;
extern nglMaterial* nglGetMaterial(const tlFixedString& Name, bool Warn);

// ea: 0x677850
unsigned int InstanceBankMgr::Get(eInstanceBankType type, TPakId pakId,
                                  const tlFixedString* name)
{
    ae_sized_array<TPakId, 32> prereqs;
    prereqs.m_size = 0;
    if (type == INSTBANK_TYPE_ANIMFILE || type == INSTBANK_TYPE_ANIMOFFSET)
    {
        GetAllPaks(&prereqs);
    }
    else if (pakId == PAK_ID_INVALID)
    {
        if (PakManager::sInst != nullptr)
            PakManager::sInst->CopyContextStack(&prereqs);
    }
    else
    {
        GetPakPrerequisites(pakId, &prereqs);
    }
    if (prereqs.m_size <= 0)
        return 0;
    for (unsigned int v4 = 0; v4 < (unsigned int)prereqs.m_size; ++v4)
    {
        if (v4 >= 0x20)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        TPakId v5 = prereqs.m_elements[v4];
        InstanceBankSet* v6 = mEntries[v5];
        if (v5 != PAK_ID_INVALID && v6 != nullptr)
        {
            unsigned int* Entry =
                v6->FindEntry(type, name->str, name->hash);
            if (Entry != nullptr)
                return *Entry;
        }
    }
    return 0;
}

// ea: 0x677960
unsigned int InstanceBankMgr::Get(eInstanceBankType type, TPakId pakId,
                                  unsigned int hash)
{
    ae_sized_array<TPakId, 32> prereqs;
    prereqs.m_size = 0;
    if (type == INSTBANK_TYPE_ANIMFILE || type == INSTBANK_TYPE_ANIMOFFSET)
    {
        GetAllPaks(&prereqs);
    }
    else if (pakId == PAK_ID_INVALID)
    {
        if (PakManager::sInst != nullptr)
            PakManager::sInst->CopyContextStack(&prereqs);
    }
    else
    {
        GetPakPrerequisites(pakId, &prereqs);
    }
    if (prereqs.m_size <= 0)
        return 0;
    for (unsigned int v4 = 0; v4 < (unsigned int)prereqs.m_size; ++v4)
    {
        if (v4 >= 0x20)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        TPakId v5 = prereqs.m_elements[v4];
        InstanceBankSet* v6 = mEntries[v5];
        if (v5 != PAK_ID_INVALID && v6 != nullptr)
        {
            unsigned int* Entry = v6->FindEntry(type, nullptr, hash);
            if (Entry != nullptr)
                return *Entry;
        }
    }
    return 0;
}

// ea: 0x677A60
nglTexture* cdGetTexture(TPakId pakId, const tlFixedString& name)
{
    nglTexture* result = (nglTexture*)InstanceBankMgr::sInst->Get(
        INSTBANK_TYPE_TEXTURE, pakId, &name);
    if (result == nullptr)
        return nglTextureDirectory.Find(name);
    return result;
}

// ea: 0x677A90
nglMesh* cdGetMesh(TPakId pakId, const tlFixedString& name)
{
    nglMesh* result = (nglMesh*)InstanceBankMgr::sInst->Get(
        INSTBANK_TYPE_MESHFILE, pakId, &name);
    if (result == nullptr)
        return nglMeshDirectory.Find(name);
    return result;
}

// ea: 0x677AC0
apsEffectTemplate* cdGetEffectTemplate(TPakId pakId,
                                       const tlFixedString& name)
{
    apsEffectTemplate* result = (apsEffectTemplate*)
        InstanceBankMgr::sInst->Get(INSTBANK_TYPE_EFFECT, pakId, &name);
    if (result == nullptr)
    {
        apsEffectTemplate* v3 = sMissingParticleEffect;
        if (sMissingParticleEffect == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
            AeAssert::gCurrentLine = 827;
            AeAssert::gCurrentExpr = "tpl";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Couldn't find particle effect %s",
                                    name.str))
                __debugbreak();
        }
        return v3;
    }
    return result;
}

// ea: 0x677B40
nglFont* cdGetFont(TPakId pakId, const tlFixedString& name)
{
    nglFont* result = (nglFont*)InstanceBankMgr::sInst->Get(
        INSTBANK_TYPE_FONT, pakId, &name);
    if (result == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 834;
        AeAssert::gCurrentExpr = "font";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Couldn't find font %s", name.str))
            __debugbreak();
        return nglFontDirectory.Find(name);
    }
    return result;
}

// ea: 0x677BC0
MultiApk* cdGetMeshFile(TPakId pakId, const tlFixedString& name)
{
    return (MultiApk*)InstanceBankMgr::sInst->Get(
        INSTBANK_TYPE_MESHFILE, pakId, &name);
}

// ea: 0x677BE0
nalAnimClass<nalAnyPose>* cdGetAnim(TPakId pakId,
                                    const tlFixedString& name)
{
    return (nalAnimClass<nalAnyPose>*)InstanceBankMgr::sInst->Get(
        INSTBANK_TYPE_ANIMFILE, pakId, &name);
}

// ea: 0x677C00
nalAnimClass<nalAnyPose>* cdGetAnim(unsigned int hash)
{
    return (nalAnimClass<nalAnyPose>*)InstanceBankMgr::sInst->Get(
        INSTBANK_TYPE_ANIMFILE, PAK_ID_INVALID, hash);
}

// ea: 0x677C20
nalBaseSkeleton* cdGetSkeleton(TPakId pakId, const tlFixedString& name)
{
    unsigned int v2 = InstanceBankMgr::sInst->Get(
        INSTBANK_TYPE_SKELETON, pakId, &name);
    if (v2 == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 871;
        AeAssert::gCurrentExpr = "skel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Couldn't find skel %s", name.str))
            __debugbreak();
    }
    return (nalBaseSkeleton*)v2;
}

// ea: 0x678D40
void* cdGetResource(const tlFixedString& FileName, unsigned int FourCC,
                    bool ExtraSafety)
{
    if (FileName.hash == 0)
        return FourCC != 0x584554u ? nullptr : nglDefaultTex;
    ae_sized_array<TPakId, 128>& ContextStack =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    TPakId v6 = PAK_ID_INVALID;
    if (ContextStack.m_size != 0)
        v6 = ContextStack.m_elements[ContextStack.m_size - 1];
    if (FourCC > 0x4C454B53u)
    {
        if (FourCC == 0x53504541u)
            return cdGetEffectTemplate(v6, FileName);
        if (FourCC != 0x544E4F46u)
            return nullptr;
        void* result = cdGetFont(v6, FileName);
        if (result == nullptr)
            return nglGetFont(FileName);
        return result;
    }
    if (FourCC == 0x4C454B53u)
        return gCharSkel;
    if (FourCC == 0x54414Du)
        return nglGetMaterial(FileName, true);
    if (FourCC != 0x584554u)
    {
        if (FourCC == 0x4853454Du)
            return cdGetMesh(v6, FileName);
        return nullptr;
    }
    if (FileName == none_tfs || FileName == null_tfs)
        return nglDefaultTex;
    void* result = cdGetTexture(v6, FileName);
    if (result == nullptr && ExtraSafety)
    {
        tlPrintf("Unable to locate texture resource %s - assigning default texture.\n",
                 FileName.str);
        return nglDefaultTex;
    }
    return result;
}

// ea: 0x678ED0
void* apkGetResource(const tlFixedString& FileName, unsigned int FourCC)
{
    return cdGetResource(FileName, FourCC, true);
}

// ea: 0x677500
apk::apkFile* cdLoadApkInplace(const char* name, unsigned char* data,
                               int size, TPakId pakId, PakFile* pak)
{
    (void)name; (void)size;
    PakHeapContext heap_ctx(pakId, false);
    apk::apkFile* FileInPlace =
        apk::apkLoadFileInPlace(data, true);
    pak->AddApk(FileInPlace);
    if (heap_ctx.mPakId != PAK_ID_INVALID)
    {
        TlSystemCallbacks::LockTlAllocsToPakHeap(heap_ctx.mLastState, false);
        ae_sized_array<TPakId, 128>& ContextStack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst
                ->GetContextStack();
        if (ContextStack.m_size != 0)
            ContextStack.m_size = ContextStack.m_size - 1;
    }
    return FileInPlace;
}

// ea: 0x671360
PakInfoNode* PakManager::GetBestUnloadablePak()
{
    reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_head != nullptr ? m_head->m_next : nullptr;
    PakInfoNode* ret = nullptr;
    float max_dist = 0.0f;
    if (m_head == mActivePaks.m_end || m_next == nullptr)
        return nullptr;
    while (1)
    {
        PakFile* pak = (PakFile*)m_head;
        const PakInfoNode* m_prev = nullptr;
        if (pak->mPakId != mDebugPakId)
        {
            if (pak->mPakType != kPakTypeCount)
            {
                if (pak->mPakInfo == nullptr)
                    pak->mPakInfo = PakManager::sInst->GetPakInfo(pak->mPakId);
                m_prev = pak->mPakInfo;
            }
            if (pak->mLoadingState == (PakFile::ELoadingState)8)
                return (PakInfoNode*)m_prev;
            float v5;
            if (m_prev->refCount <= 1)
                v5 = GetDistance((PakInfoNode*)m_prev);
            else
                v5 = 0.0f;
            if (v5 > max_dist)
            {
                max_dist = v5;
                ret = (PakInfoNode*)m_prev;
            }
        }
        if (m_next == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/reserved_dlist.h";
            AeAssert::gCurrentLine = 501;
            AeAssert::gCurrentExpr = "m_next != 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Please add a descriptive string"))
                __debugbreak();
        }
        m_head = m_next;
        if (m_next->m_next == nullptr)
            return ret;
        m_next = m_next->m_next;
    }
}

// ea: 0x671480
void PakManager::UpdateLoading()
{
    bool v2 = false;
    if (mPakInfoBank != nullptr && mActivePaks.m_size > 2
        && mLevelPakInfoBank != nullptr)
    {
        PakInfoNode* BestUnloadedPak = GetBestUnloadedPak();
        PakInfoNode* v4 = BestUnloadedPak;
        float hp_dist = BestUnloadedPak != nullptr
                            ? GetDistance(BestUnloadedPak)
                            : 3.4028235e38f;
        const PakInfoNode* mCurSection = this->mCurSection;
        if (mCurSection == nullptr)
            mCurSection = mCurrentPakInfo;
        float cur_dist = GetDistance((PakInfoNode*)mCurSection);
        v2 = cur_dist > hp_dist;
        if (cur_dist == 0.0f)
            v2 = false;
        if (v4 == mCurrentPakInfo)
            v2 = false;
    }
    TPakId mCurrentPakId = this->mCurrentPakId;
    PakFile* v7 = mSlots[mCurrentPakId];
    if ((mCurrentPakId != PAK_ID_INVALID && v7 != nullptr
         && v7->mState == PakFile::LOADED)
        || (v7->mState == PakFile::UNLOADING
            && v7->mLoadingState
                   != (PakFile::ELoadingState)PakFile::UNLOADING_SERIALIZED)
        || (v2 && v7->mLoadingState == PakFile::LOADING_DATA))
    {
        if (mCurrentPakInfo != nullptr)
        {
            if (mCurrentPakInfo->pakId != mCurrentPakId)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 2145;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning("probable streaming problem here"))
                    __debugbreak();
            }
            RegisterPakLoaded(mCurrentPakInfo);
        }
        if (v2)
        {
            AsyncUnloadPak(mCurrentPakId);
        }
        else
        {
            TPakId v9 = mCurrentPakId;
            mCurrentPakInfo = nullptr;
            StreamZoneManager::sInst->OnLoaded(v9);
            mState = (state_e)2;  // STATE_NORMAL
            mCurrentPakId = PAK_ID_INVALID;
            mCurSection = nullptr;
        }
    }
}

// ea: 0x679390
void PakManager::UpdateUnloading()
{
    TPakId mCurrentPakId = this->mCurrentPakId;
    if (mCurrentPakId == PAK_ID_INVALID || mSlots[mCurrentPakId] == nullptr
        || mSlots[mCurrentPakId]->mState == PakFile::UNLOADED)
    {
        PakFile* v4 = mSlots[mCurrentPakId];
        if (!v4->mBankAlloc.IsEmpty())
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 2182;
            AeAssert::gCurrentExpr = "p->mBankAlloc.IsEmpty()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("unreleased banks!"))
                __debugbreak();
        }
        // reserved_dlist<PakFile>::erase (streamer.o 0x686EB0); the
        // find(obj) != end() debug assert (reserved_dlist.h:418) is elided
        reserved_dlist<PakFile>::dlist_node* node =
            (reserved_dlist<PakFile>::dlist_node*)&v4->m_dlist_node;
        node->m_next->m_prev = node->m_prev;
        node->m_prev->m_next = node->m_next;
        --mActivePaks.m_size;

        if (mCurrentPakInfo != nullptr)
        {
            if (mCurrentPakInfo->pakId != this->mCurrentPakId)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 2190;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning("probable streaming problem here"))
                    __debugbreak();
            }
            RegisterPakUnloaded(mCurrentPakInfo);
            if (mCurrentPakInfo->refCount != 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 2200;
                AeAssert::gCurrentExpr = "mCurrentPakInfo->refCount == 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                           "somehow a still-used pak has been unloaded"))
                    __debugbreak();
            }
            mCurrentPakInfo->pakId = PAK_ID_INVALID;
            mCurrentPakInfo = nullptr;
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 2206;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("no current pak info!"))
                __debugbreak();
        }
        const PakInfoNode* mPakInfo;
        if (v4->mPakType == kPakTypeCount)
            mPakInfo = nullptr;
        else
        {
            if (v4->mPakInfo == nullptr)
                v4->mPakInfo = PakManager::sInst->GetPakInfo(v4->mPakId);
            mPakInfo = v4->mPakInfo;
        }
        switch (mPakInfo->pakType)
        {
        case kPakTypeGlobal:
            mGlobalPakId = PAK_ID_INVALID;
            break;
        case kPakTypeAnimation:
            mAnimPakId = PAK_ID_INVALID;
            break;
        case kPakTypeLevel:
            mLevelPakId = PAK_ID_INVALID;
            break;
        case kPakTypeCount:
            mDebugPakId = PAK_ID_INVALID;
            break;
        default:
            break;
        }
        mSlots[this->mCurrentPakId] = nullptr;
        mPakInfoPtrs[this->mCurrentPakId] = nullptr;
        mCurrentPakId = PAK_ID_INVALID;
        mState = (state_e)2;  // STATE_NORMAL
        mCurSection = nullptr;
        v4->~PakFile();
        PakFile::sAllocator->Release(v4);
    }
}

// ea: 0x678040
void PakManager::UpdateNormal()
{
    ++sComputeDistanceKey;
    PakInfoNode* hp = GetBestUnloadedPak();
    PakInfoNode* v4 = GetBestUnloadablePak();
    PakInfoNode* lp = v4;
    float lp_dist = v4 != nullptr ? GetDistance(v4) : 3.4028235e38f;
    float hp_dist = hp != nullptr ? GetDistance(hp) : 3.4028235e38f;
    if (v4 != nullptr)
    {
        PakFile* v5 = mSlots[v4->pakId];
        if (v5 != nullptr
            && v5->mLoadingState == (PakFile::ELoadingState)8)
            lp_dist = 3.4028235e38f;
    }
    bool unload = false;
    bool v6 = false;
    if (v4 != nullptr && lp_dist == 3.4028235e38f)
    {
        v6 = true;
        goto decide;
    }
    mNextPakInfo = hp;
    if (hp_dist != 3.4028235e38f)
    {
        NumBanks hp_num_banks = GetNumBanks(hp);
        float xbox = hp_num_banks.xbox;
        if (xbox <= BankManager::sInst->get_free_count().xbox)
        {
            mCurrentPakInfo = hp;
            TPakId Pak = AsyncLoadPak(hp->pakType,
                                      hp->path.mStr,
                                      hp->numBanks);
            hp->pakId = Pak;
            mCurSection = hp;
            mFilled = false;
            return;
        }
        v4 = lp;
        v6 = unload;
        goto decide;
    }
    mFilled = true;
    if (EntityManager::sInst->mWorld != nullptr)
        ((Entity*)EntityManager::sInst->mWorld)
            ->Notify(hash_const.zonesloaded);
    return;

decide:
    if (v4 != nullptr && (v6 || lp_dist > hp_dist))
    {
        mFilled = false;
        mCurrentPakInfo = v4;
        if (v4->pakType == kPakTypeGlobal)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 2082;
            AeAssert::gCurrentExpr = "lp->pakType != kPakTypeGlobal";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("sanity check"))
                __debugbreak();
        }
        AsyncUnloadPak(v4->pakId);
        mCurrentPakId = v4->pakId;
        mCurSection = v4;
    }
    else
    {
        mFilled = true;
        if (EntityManager::sInst->mWorld != nullptr)
            ((Entity*)EntityManager::sInst->mWorld)
                ->Notify(hash_const.zonesloaded);
    }
}

// ea: 0x677750
void PakFile::UpdateUnloading()
{
    switch (mLoadingState)
    {
    case (ELoadingState)UNLOADING_SERIALIZED:
        tlPrintf("Unloading serialized assets from '%s'\n", mPath.mBuff);
        UnloadSerialized();
        mLoadingState = (ELoadingState)UNLOADING_INPLACE;
        break;
    case (ELoadingState)UNLOADING_INPLACE:
        tlPrintf("Unloading inplace assets from '%s'\n", mPath.mBuff);
        UnloadInplace();
        mLoadingState = (ELoadingState)(UNLOADING_DONE | LOADING_TOC);
        break;
    case (ELoadingState)(UNLOADING_DONE | LOADING_TOC):
        tlPrintf("Finished unloading of '%s'\n", mPath.mBuff);
        FinishUnload();
        if (mPakType == kPakTypeCount)
        {
            BrocSys::NotifyPakUnloaded(nullptr);
        }
        else
        {
            if (mPakInfo == nullptr)
                mPakInfo = PakManager::sInst->GetPakInfo(mPakId);
            BrocSys::NotifyPakUnloaded(
                mPakInfo->longName.mStr);
        }
        break;
    default:
        break;
    }
}

// RecordResourceType (streamer.o PakFile.h; verified against IDA enum)
enum RecordResourceType {
    Resource_VertexBuffer = 0,
    Resource_Texture = 1,
    Resource_Palette = 2,
    Resource_ColorBuffer = 3,
    Resource_ZetaBuffer = 4,
    Resource_Reserved = 5,
    Resource_MAX = 6,
};
RecordResourceType GetWorkAmount(const char* name);  // streamer.o (defined below)

// ea: 0x67A380
void PakFile::UpdateLoading()
{
    nflUpdate();
    if (nflGetState() == NFL_STATE_ERROR)
        g_femanager.DrawDiscError();
    ELoadingState mLoadingState = this->mLoadingState;
    bool fillingBanks = PakManager::sInst->mFillingBanks;
    unsigned int work_done = 0;
    if (mLoadingState != PakFile::LOADING_HEADER)
    {
        int v3 = (int)mLoadingState - 2;
        if (v3 != 0)
        {
            if (v3 == 1)  // LOADING_DONE
            {
                tlPrintf("pak: done loading %s\n", mPath.mBuff);
                unsigned __int64 v4 = __rdtsc();
                mLoadStats->total =
                    (float)((double)(v4 - mLoadStats->totalStart)
                            * 0.0000000013636364);
                if (mCloseHandle)
                {
                    nflCloseFile(mFileId);
                    mFileId = NFL_FILE_ID_INVALID;
                }
                mState = PakFile::LOADED;
                if (mPakType != kPakTypeCount)
                {
                    const PakInfoNode* Info = GetInfo();
                    BrocSys::NotifyPakLoaded(
                        Info->longName.mStr);
                }
                G_ParseInteractionInfo(mPakId);
                unsigned int numSections = mHeader->numSections;
                int sectionIdx = mDefaultSectionIdx + 1;
                if (sectionIdx < (int)numSections)
                {
                    work_done = 24 * (unsigned int)sectionIdx;
                    do
                    {
                        PakHeader::Section* sec =
                            (PakHeader::Section*)((char*)mHeader->sections
                                                  + work_done);
                        tlFixedString audioBank;
                        memset(&audioBank, 0, sizeof(audioBank));
                        if (strncmp((const char*)sec->name, "AUDIO", 8)
                            == 0)
                        {
                            if (sec->numFiles != 0)
                            {
                                for (unsigned int fileIdx = 0;
                                     fileIdx < sec->numFiles; ++fileIdx)
                                {
                                    PakHeader::File* v13 =
                                        &sec->files[fileIdx];
                                    const char* v57 = v13->shortName;
                                    ELanguage lang = kLanguageUnlocalized;
                                    if (strstr(v57, ".wen") != nullptr)
                                        lang = kLanguageEnglish;
                                    else if (strstr(v13->shortName, ".wde")
                                             != nullptr)
                                        lang = kLanguageGerman;
                                    else if (strstr(v13->shortName, ".wfr")
                                             != nullptr)
                                        lang = kLanguageFrench;
                                    else if (strstr(v13->shortName, ".wes")
                                             != nullptr)
                                        lang = kLanguageSpanish;
                                    else if (strstr(v13->shortName, ".wit")
                                             != nullptr)
                                        lang = kLanguageItalian;
                                    ae_fixed_string<128, unsigned char> oBuff;
                                    int oLen = 0;
                                    AeStringSupport::CStrToAeStr(
                                        (char*)oBuff.mBuff, &oLen, 127,
                                        v13->shortName);
                                    oBuff.mLength = (unsigned char)oLen;
                                    ae_fixed_string<128, unsigned char>
                                        fname = oBuff.get_file_name(true);
                                    tlFixedString v60(
                                        (const char*)fname.mBuff);
                                    char path[128];
                                    strcpy(path, mPath.mBuff);
                                    char* slash = strrchr(path, '\\');
                                    if (slash != nullptr)
                                        slash[1] = 0;
                                    strcat(path, v13->shortName);
                                    AudioBankMgr::sInst->RegisterWbk(
                                        v60, path, lang, mPakId);
                                    if (fileIdx == 0)
                                        audioBank =
                                            tlFixedString(v60.str);
                                }
                            }
                            tlFixedString v64 = audioBank;
                            AudioBankMgr::sInst->LoadWbk(v64, true);
                        }
                        else if (strncmp((const char*)sec->name, "ANIM", 8)
                                 == 0)
                        {
                            for (unsigned int v28 = 0; v28 < sec->numFiles;
                                 ++v28)
                            {
                                PakHeader::File* v13 = &sec->files[v28];
                                InstanceBankMgr::sInst->RegisterAnimOffset(
                                    v13->shortName,
                                    sec->banks[0].fileOffset
                                        + v13->bankOffset,
                                    v13->fileSize, mPakId);
                            }
                        }
                        else if (strncmp((const char*)sec->name, "DTEX", 8)
                                 == 0)
                        {
                            if (sec->numFiles != 0)
                            {
                                for (unsigned int bankIdx = 0;
                                     bankIdx < sec->numFiles; ++bankIdx)
                                {
                                    PakHeader::File* v31 =
                                        &sec->files[bankIdx];
                                    Com_StripExtension(v31->shortName,
                                                       (char*)v31->shortName);
                                    tlFixedString v66(v31->shortName);
                                    InstanceBankMgr::sInst->Add(
                                        INSTBANK_TYPE_DISCTEX, mPakId, v66,
                                        sec->banks[0].fileOffset
                                            + v31->bankOffset);
                                    tlFixedString v65(v31->shortName);
                                    InstanceBankMgr::sInst->Add(
                                        INSTBANK_TYPE_DISCTEXSIZE, mPakId, v65,
                                        v31->fileSize);
                                }
                            }
                        }
                        work_done += 24;
                        ++sectionIdx;
                    } while (sectionIdx < (int)mHeader->numSections);
                }
                if (mDefaultSectionIdx != -1)
                {
                    PakHeader::Section* v37 =
                        &mHeader->sections[mDefaultSectionIdx];
                    if (v37->numBanks != 0)
                    {
                        for (unsigned int bankIdx = 0;
                             bankIdx < v37->numBanks; ++bankIdx)
                        {
                            PakHeader::Bank& v40 = v37->banks[bankIdx];
                            unsigned int v39 = v40.flags;
                            if ((v39 & 4) != 0)  // serialized bank flag
                            {
                                if ((v39 & PAK_BANK_FLAG_APK_LOADED) != 0)
                                {
                                    v40.flags &= 0xFFFBFFFF;
                                    apk::apkDeleteFile(
                                        (apk::apkFile*)((char*)v40.memptr
                                                        + 8));
                                }
                                if ((v40.flags & PAK_BANK_FLAG_MEM) != 0)
                                    stream_free(v40.memptr);
                                v40.memptr = nullptr;
                                v40.memsize = 0;
                            }
                        }
                    }
                }
                if (!mSerializedAlloc.IsEmpty())
                {
                    BankManager::sInst->release(mSerializedAlloc);
                    if (!mSerializedAlloc.IsEmpty())
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\PakFile.cpp";
                        AeAssert::gCurrentLine = 1213;
                        AeAssert::gCurrentExpr =
                            "mSerializedAlloc.IsEmpty()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(
                                   "bank manager should clear this bitset"))
                            __debugbreak();
                    }
                }
                CopyHeader();
            }
        }
        else  // LOADING_DATA
        {
            while (IsNextFileReady())
            {
                if (!fillingBanks && work_done >= (unsigned int)max_work)
                    break;
                PakHeader* mHeader = this->mHeader;
                unsigned int mCurrentFile = this->mCurrentFile;
                PakHeader::File* v52 =
                    &mHeader->sections[mDefaultSectionIdx]
                         .files[mCurrentFile];
                const char* v71 = v52->shortName;
                int size = v52->fileSize;
                mCurrDecodeFile = v52->longName;
                tlPrintf("pak: [% 3d/%d] %s\n", mCurrentFile + 1,
                         mHeader->sections[mDefaultSectionIdx].numFiles,
                         v52->shortName);
                work_done += (unsigned int)GetWorkAmount(v71);
                Decode(v71, size, mPakId);
                ++this->mCurrentFile;
                DetermineNextFile();
                if (mCurrentFilePtr == nullptr && mCurrentApk == nullptr)
                    break;
            }
            mCurrDecodeFile = nullptr;
        }
    }
    else if (IsRequestDone(&mHeaderRequestId))  // LOADING_HEADER
    {
        gThroughputMeasurer.stop(0x8000);
        mHeaderBuffer = this->mHeaderBuffer;
        mHeader = (PakHeader*)mHeaderBuffer;
        mHeader->FixUp(false, 1);
        if (mHeader->numSections != 0)
        {
            if (mHeader->headerShortSize > PakFile::sHeaderBufferSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 1004;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning(
                           "resizing pak header buffer to %d",
                           (mHeader->headerShortSize + 0x7FFF)
                               & 0xFFFF8000))
                    __debugbreak();
                PakFile::sHeaderBufferSize =
                    (mHeader->headerShortSize + 0x7FFF) & 0xFFFF8000;
                PakFile::sHeaderBuffer =
                    stream_alloc(PakFile::sHeaderBufferSize, false);
                stream_free(mHeaderBuffer);
                mHeaderBuffer = PakFile::sHeaderBuffer;
                codNflReadFile(mFileId, 0, mHeaderBuffer,
                               PakFile::sHeaderBufferSize);
                mHeader = (PakHeader*)mHeaderBuffer;
                mHeader->FixUp(false, 1);
            }
            mHeaderRequestId.nflId = NFL_REQUEST_ID_INVALID;
            AeAssert::gCurrentPakFile = mPath.mBuff;
            AeAssert::gCurrentPakResource = nullptr;
            if (mHeader->id != 1179664715)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 1021;
                AeAssert::gCurrentExpr = "mHeader->id == ('FPAK')";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Doesn't seem to be a pak"))
                    __debugbreak();
            }
            if (mHeader->version > 2.0599999f)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 1022;
                AeAssert::gCurrentExpr = "! (mHeader->version > (2.06f))";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                           "Version mismatch: pak is newer than game"))
                    __debugbreak();
            }
            if (mHeader->version < 2.0599999f)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 1023;
                AeAssert::gCurrentExpr = "! (mHeader->version < (2.06f))";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Version mismatch: old pak"))
                    __debugbreak();
            }
            AeAssert::gCurrentPakFile = nullptr;
            AeAssert::gCurrentPakResource = nullptr;
            tlPrintf("pak: %d files, %d sections\n",
                     mHeader->sections->numFiles, mHeader->numSections);
            if (mOnlyLoadHeader)
            {
                tlPrintf("pak header: done loading %s\n", mPath.mBuff);
                mState = PakFile::LOADED;
            }
            else
            {
                InitiateDataLoad();
            }
        }
        else
        {
            tlPrintf("empty pakfile %s\n", mPath.mBuff);
            mLoadingState = (ELoadingState)PakFile::LOADING_DONE;
        }
    }
}

// ea: 0x67AD70
void PakFile::Update()
{
    if (mState == PakFile::LOADING)
    {
        UpdateLoading();
    }
    else if (mState == PakFile::UNLOADING)
    {
        UpdateUnloading();
    }
}

// ea: 0x677810
void PakFile::SyncUnload()
{
    BeginAsyncUnload();
    UnloadSerialized();
    UnloadInplace();
    FinishUnload();
}

// ea: 0x679E10
PakDecoder GetDecoder(const char* ext)
{
    tlFixedString v4(ext);
    unsigned int hash = v4.hash;
    switch (hash)
    {
    case 0x36FEC0:  return DecodeAITypeBank;         // "aitb"
    case 0x1C36D:   return DecodeGDB;                // "gdb"
    case 0x1B048:   return DecodeBSP;
    case 0xDEB:     return DecodeInstanceBank;       // "ib"
    case 0xD2A:     return DecodeCGBank;             // "cg"
    case 0xD46:     return DecodeDB;                 // "db"
    case 0x1AE92:   return DecodeGrassInfo;          // no-op decoder
    case 0x1B8CA:   return DecodeDestructible;
    case 0x1B68E:   return DecodeDCGBank;            // "dcg"
    case 0x1B7B7:   return DecodeDialogueBank;       // "dlg"
    case 0x1C03B:   return DecodeFLI;                // "fli"
    case 0x1F817:   return DecodeSPT;                // "spt"
    case 0x1F664:   return DecodeScene;              // "scn"
    case 0x1E118:   return DecodeWIND;               // no-op decoder
    case 0x1EA51:   return DecodePhysData;           // "phy"
    case 0x1F80A:   return DecodeSplineGroup;        // "spg"
    case 0x20CE7:   return DecodeXModelBank;         // "xmb"
    case 0x1F889:   return DecodeSTB;                // "stb"
    case 0x2031E:   return DecodeBin;
    case 0x213FE:   return DecodeZoneBoundaryBank;   // "zbb"
    case 0x72AF43B: return DecodeMipSettings;
    case 0x430E52:  return DecodeWIND;               // "wind"
    case 0x3D24B8:  return DecodeLevelPath;          // "lpth"
    case 0x372D49:  return DecodeAnimBank;
    case 0x3AC1FE:  return DecodeHeap;               // "heap"
    case 0x40CAA1:  return DecodeSEED;               // "seed"
    case 0x44D206:  return DecodeZonePath;
    case 0x437CB3:  return DecodeTexture;
    case 0x43ABF7:  return DecodeXModelPartsBank;    // "xmpb"
    case 0x66417A2: return DecodeAnimMatrix;
    case 0x1F124251: return DecodeFont;              // "xbfont"
    case 0x1F0F7F7F: return DecodeAnim;
    case 0x8B3348B: return DecodeTexture;            // "xbtex"
    case 0x7DCC452: return DecodeLightGrid;          // "lgrid"
    case 0x821CA90:
        return reinterpret_cast<PakDecoder>(DecodePanel); // "panel"
    case 0x1F195109: return DecodeSkeleton;
    case 0x8DB2340D: return DecodeCharSkel;          // "charskel"
    case 0xEE5EED49: return DecodeConfigStrings;     // "cfgstr"
    default:
        break;
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DecodePakFile.cpp";
    AeAssert::gCurrentLine = 422;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("no decoder for type '%s'", ext))
        __debugbreak();
    return nullptr;
}

// ea: 0x67A150
void PakFile::Decode(const char* name, int size, TPakId pakId)
{
    TPakId elt = mPakId;
    ae_sized_array<TPakId, 128>& ContextStack =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    ContextStack.push_back(elt);
    AeAssert::gCurrentPakFile = mPath.mBuff;
    AeAssert::gCurrentPakResource = name;
    const char* v7 = name;
    if (*name != 0)
    {
        do
            ++v7;
        while (*v7 != 0);
    }
    if (*v7 != '.')
    {
        do
        {
            if (v7 <= name)
                break;
            --v7;
        } while (*v7 != '.');
    }
    apk::apkFile* mCurrentApk = (apk::apkFile*)this->mCurrentApk;
    unsigned char* mCurrentFilePtr = this->mCurrentFilePtr;
    const char* v12 = v7 + 1;
    if (mCurrentApk != nullptr)
    {
        mCurrentApk->ApplyReferencesForEntry(
            (apk::apkFileEntry*)mCurrentApkFileEntry);
        apk::apkFileTypeEntry* mCurrentApkFileTypeEntry =
            (apk::apkFileTypeEntry*)this->mCurrentApkFileTypeEntry;
        if (mCurrentApkFileTypeEntry->Type == 1230261325)
        {
            cdInvokeMultiApkCallbacks(name, v12, pakId, mCurrentApk,
                                      (apk::apkFileEntry*)mCurrentApkFileEntry);
            goto LABEL_20;
        }
        if (!mCurrentApk->InvokeFileLoadCallback(mCurrentApkFileTypeEntry,
                                                 (apk::apkFileEntry*)mCurrentApkFileEntry))
        {
            tlFixedString v18(v12);
            if (mCurrentApkFileTypeEntry->Type != v18.hash)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 1506;
                AeAssert::gCurrentExpr =
                    "mCurrentApkFileTypeEntry->Type == tlFixedString(file_ext).GetHash()";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("uh oh!"))
                    __debugbreak();
            }
            tlFixedString v18b("image");
            int SectionIndex = mCurrentApk->GetSectionIndex(v18b);
            mCurrentFilePtr = (unsigned char*)
                ((apk::apkFileEntry*)mCurrentApkFileEntry)
                    ->GetData(mCurrentApk, SectionIndex, true);
        }
        else
        {
            goto LABEL_20;
        }
    }
    {
        PakDecoder Decoder = GetDecoder(v12);
        if (Decoder != nullptr)
        {
            Decoder(name, mCurrentFilePtr, size, pakId, this);
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
            AeAssert::gCurrentLine = 1521;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning(
                       "failed to find a decoder for file %s", name))
                __debugbreak();
        }
    }
LABEL_20:
    AeAssert::gCurrentPakFile = nullptr;
    AeAssert::gCurrentPakResource = nullptr;
    if (ContextStack.m_size != 0)
        ContextStack.m_size = ContextStack.m_size - 1;
}

void* PakManager::MemAlloc(TPakId id, unsigned int size, bool bUseActorHeap)
{
    if (id != PAK_ID_INVALID)
    {
        if (sInst->mSlots[id] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 2583;
            AeAssert::gCurrentExpr = "PakManager::Inst()->IsValid( id )";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("bad pak id for MemAlloc"))
                __debugbreak();
        }
        PakFile* v5 = id > 0x62 ? nullptr : mSlots[id];
        void* result = v5->MemAlloc(0x10u, size, true);
        if (result != nullptr)
            return result;
    }
    if (!bUseActorHeap)
        return mem_heap_malloc(16, size);
    void* result = gActorHeap->Malloc(size, 16);
    if (result == nullptr)
        return mem_heap_malloc(16, size);
    return result;
}
void PakManager::ResetPriorities(bool user_distances_also)
{
    PakInfoBank* bank = mPakInfoBank;
    for (int i = 0; i < (int)bank->mPtrs.mSize; ++i)
    {
        const_cast<PakInfoNode*>(bank->mPtrs.mList[i])->distance =
            3.4028235e38f;
        if (user_distances_also)
            const_cast<PakInfoNode*>(mPakInfoBank->mPtrs.mList[i])
                ->userDistance = 3.4028235e38f;
        bank = mPakInfoBank;
    }
    PakInfoBank* level = mLevelPakInfoBank;
    for (unsigned int v8 = 0; v8 < level->mPtrs.mSize; ++v8)
    {
        const_cast<PakInfoNode*>(level->mPtrs.mList[v8])->distance =
            3.4028235e38f;
        if (user_distances_also)
            const_cast<PakInfoNode*>(mLevelPakInfoBank->mPtrs.mList[v8])
                ->userDistance = 3.4028235e38f;
        level = mLevelPakInfoBank;
    }
}

// ea: 0x665410
void PakManager::ClearFileLocationInfo()
{
    mPakInfoBank = nullptr;
}

// ea: 0x677F40
PakFile* PakManager::AsyncLoadPakHeader(EPakType pak_type, const char* path,
                                        NumBanks num_banks)
{
    PakManager* v4 = this;
    if (nflFileExists((nflMediaID)gNflMediaId, path) != 0)
    {
        PakFile* v6 = (PakFile*)PakFile::sAllocator->Allocate(0x114u, false);
        PakFile* result;
        if (v6 != nullptr)
        {
            v6->PakFile::PakFile(pak_type, path, PAK_ID_INVALID, num_banks);
            result = v6;
            v4 = this;
        }
        else
        {
            result = nullptr;
        }
        v4->mState = STATE_LOADING;
        v4->mCurrentPakId = PAK_ID_INVALID;
        return result;
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
    AeAssert::gCurrentLine = 1272;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error(
            "FILE NOT FOUND: %s:\ncheck your \"asset manager\" output for details",
            path))
        __debugbreak();
    tlPrintf("FILE NOT FOUND: %s\n", path);
    return nullptr;
}

// ea: 0x67AD90
const PakInfoNode* PakManager::SyncLoadFLI(EPakType t, const char* path)
{
    NumBanks num_banks = {};
    PakFile* pak = AsyncLoadPakHeader(t, path, num_banks);
    if (pak == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 1125;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("couldn't load pak %s!", path))
            __debugbreak();
        return nullptr;
    }
    pak->mOnlyLoadHeader = true;
    while (mEnabled != 0)
    {
        if (pak->mState == PakFile::LOADING)
            pak->UpdateLoading();
        else if (pak->mState == PakFile::UNLOADING)
            pak->UpdateUnloading();
        if (pak->mState == PakFile::LOADED)
        {
            PakHeader* mHeader = pak->mHeader;
            int sectionIdx = -1;
            if (mHeader->numSections != 0)
            {
                for (unsigned int v7 = 0; v7 < mHeader->numSections; ++v7)
                {
                    if (strncmp(mHeader->sections[v7].name, "FLI", 8) == 0)
                        sectionIdx = (int)v7;
                }
                if (sectionIdx != -1)
                {
                    PakHeader::Section& sec = mHeader->sections[sectionIdx];
                    unsigned int fileSize = sec.files->fileSize;
                    int bankIndex = sec.files->bankIndex;
                    NumBanks oneBank;
                    oneBank.ps2 = 1.0f;
                    oneBank.ps3.main = 1.0f;
                    oneBank.ps3.lram = 0.0f;
                    oneBank.xbox = 1.0f;
                    oneBank.xenon = 1.0f;
                    oneBank.pcx = 1.0f;
                    oneBank.gc.main = 1.0f;
                    oneBank.gc.aram = 0.0f;
                    PakFile::TRequestId id;
                    id.nflId = (nflRequestID)sec.banks[bankIndex].fileOffset;
                    TBankAlloc bankAlloc = {};
                    TBankAlloc v31 = BankManager::sInst->Allocate(oneBank);
                    bankAlloc = v31;
                    mem_info v30 = BankManager::sInst->get_alloc(bankAlloc, 0,
                                                                 true);
                    v31.mram_alloc2.mBits[0] = (unsigned int)v30.data;
                    v31.mram_alloc2.mBits[1] = (unsigned int)v30.size;
                    PakHeader::Bank& bank = sec.banks[bankIndex];
                    if (bank.compressedSize != 0)
                        id.nflId = (nflRequestID)
                            *(unsigned int*)pak->PakReadDataAsync(
                                (unsigned char*)&id, v30.data, fileSize,
                                bank.compressedSize, id.nflId);
                    else
                        id.nflId = (nflRequestID)
                            *(unsigned int*)pak->PakReadDataAsync(
                                (unsigned char*)&id, v30.data,
                                (fileSize + 0x7FFF) & 0xFFFF8000, 0,
                                id.nflId);
                    do
                    {
                        nflUpdate();
                        if (nflGetState() == NFL_STATE_ERROR)
                            g_femanager.DrawDiscError();
                    } while (!pak->IsRequestDone(&id));
                    memcpy(fli_buffer, v30.data, fileSize);
                    BankManager::sInst->release(bankAlloc);
                    nflCloseFile(pak->mFileId);
                    PakFile::sHeaderBufferUsed = false;
                    ((PakInfoBank*)fli_buffer)->Fixup();
                    mLevelPakInfoBank = (PakInfoBank*)fli_buffer;
                    return GetPakInfo(path);
                }
            }
            return nullptr;
        }
    }
    return nullptr;
}
void PakManager_MemFree(TPakId id, void* ptr, bool bUseActorHeap)
{
    (void)id; (void)ptr; (void)bUseActorHeap;
}

// ea: 0x666950
void PakManager::SetDistance(const PakInfoNode* cpak, float dist, bool force)
{
    if (cpak != nullptr)
    {
        float v4 = dist;
        if (force || dist <= cpak->distance)
        {
            const_cast<PakInfoNode*>(cpak)->distance = dist;
            if (dist == -1.0f)
                v4 = 3.4028235e38f;
            for (unsigned int v6 = 0; v6 < cpak->prereqs.mSize; ++v6)
                SetDistance(cpak->prereqs.mList[v6], v4, false);
        }
    }
}

// ea: 0x6669E0
PakInfoNode* PakManager::GetUnloadedPrereq(const PakInfoNode* pak) const
{
    unsigned int v3 = 0;
    if (pak->prereqs.mSize == 0)
        return nullptr;
    while (1)
    {
        PakInfoNode* result = (PakInfoNode*)pak->prereqs.mList[v3];
        TPakId pakId = result->pakId;
        if (pakId == PAK_ID_INVALID)
            break;
        PakFile* v7 = mSlots[pakId];
        if (v7 == nullptr || v7->mState == PakFile::UNLOADED)
            break;
        if (++v3 >= pak->prereqs.mSize)
            return nullptr;
    }
    return (PakInfoNode*)pak->prereqs.mList[v3];
}

// ea: 0x66C8B0
PakInfoNode* PakManager::GetBestUnloadedPak()
{
    PakInfoNode* ret = nullptr;
    float min_dist = 3.4028235e38f;
    for (unsigned int v4 = 0; v4 < mPakInfoBank->mPtrs.mSize; ++v4)
    {
        PakInfoNode* v6 = (PakInfoNode*)mPakInfoBank->mPtrs.mList[v4];
        TPakId pakId = v6->pakId;
        PakFile* v8 = pakId != PAK_ID_INVALID ? mSlots[pakId] : nullptr;
        bool unloaded = pakId == PAK_ID_INVALID || v8 == nullptr
                        || v8->mState == PakFile::UNLOADED;
        if (unloaded && 3.4028235e38f != GetDistance(v6)
            && (ret == nullptr || min_dist > GetDistance(v6)
                || (min_dist == GetDistance(v6)
                    && GetUnloadedPrereq(v6) == v6
                    && GetUnloadedPrereq(ret) != ret)))
        {
            min_dist = GetDistance(v6);
            ret = v6;
        }
    }
    PakInfoNode* v3 = ret;
    PakInfoBank* level = mLevelPakInfoBank;
    if (level != nullptr)
    {
        for (unsigned int i = 0; i < level->mPtrs.mSize; ++i)
        {
            PakInfoNode* v14 = (PakInfoNode*)level->mPtrs.mList[i];
            TPakId v15 = v14->pakId;
            PakFile* v16 = v15 != PAK_ID_INVALID ? mSlots[v15] : nullptr;
            bool unloaded2 = v15 == PAK_ID_INVALID || v16 == nullptr
                             || v16->mState == PakFile::UNLOADED;
            if (unloaded2 && 3.4028235e38f != GetDistance(v14)
                && (ret == nullptr || min_dist > GetDistance(v14)
                    || (min_dist == GetDistance(v14)
                        && GetUnloadedPrereq(v14) == v14
                        && GetUnloadedPrereq(ret) != ret)))
            {
                min_dist = GetDistance(v14);
                ret = v14;
            }
            level = mLevelPakInfoBank;
        }
        v3 = ret;
    }
    if (v3 == nullptr)
        return nullptr;
    TPakId v18 = v3->pakId;
    if (v18 != PAK_ID_INVALID)
    {
        PakFile* v19 = mSlots[v18];
        if (v19 != nullptr && v19->mState == PakFile::LOADED)
            return v3;
    }
    PakInfoNode* UnloadedPrereq = GetUnloadedPrereq(v3);
    if (UnloadedPrereq == nullptr)
        UnloadedPrereq = ret;
    if (3.4028235e38f != GetDistance(UnloadedPrereq)
        && UnloadedPrereq->pakType == kPakTypeGlobal)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 1891;
        AeAssert::gCurrentExpr =
            "GetDistance(prereq) == kFloat32Max || prereq->pakType!=kPakTypeGlobal";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("sanity check"))
            __debugbreak();
    }
    return UnloadedPrereq;
}

// ea: 0x66C770
float PrintBankUsage(char* res_buf, float total_banks,
                     const TBankAlloc* alloc, bool mram)
{
    char buf[1024];
    float num_banks = 0.0f;
    int last = (int)(total_banks + 0.5f);
    int v5 = 0;
    for (; v5 < last; ++v5)
    {
        char v6 = '.';
        if (mram)
        {
            if (alloc->mram_alloc1.Test(v5))
                num_banks += 0.5f;
            if (alloc->mram_alloc2.Test(v5))
                num_banks += 0.5f;
            if (alloc->mram_alloc1.Test(v5)
                && alloc->mram_alloc2.Test(v5))
                v6 = 'X';
            else if (alloc->mram_alloc1.Test(v5))
                v6 = '/';
            else if (alloc->mram_alloc2.Test(v5))
                v6 = '\\';
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 1529;
            AeAssert::gCurrentExpr = nullptr;
            if (AeAssert::Error("no aram part"))
                __debugbreak();
        }
        buf[v5] = v6;
    }
    buf[last] = 0;
    sprintf(res_buf, "[%s]", buf);
    return num_banks;
}

// ea: 0x671900
const char* PakManager::GetPakName(TPakId id) const
{
    PakFile* v2 = mSlots[id];
    if (v2 != nullptr)
    {
        if (v2->mPakType == kPakTypeCount)
            return *(const char**)4;  // dead branch in original (reads addr 4)
        if (v2->mPakInfo == nullptr)
            v2->mPakInfo = PakManager::sInst->GetPakInfo(v2->mPakId);
        return (const char*)v2->mPakInfo->longName;
    }
    reserved_dlist<PakFile>::dlist_node* m_node = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_node != nullptr ? m_node->m_next : nullptr;
    if (m_node != mActivePaks.m_end && m_next != nullptr)
    {
        while (1)
        {
            if (((PakFile*)m_node)->mPakId == id)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 2323;
                AeAssert::gCurrentExpr = nullptr;
                if (AeAssert::Error(
                        "active pak id found with NULL mSlots data"))
                    __debugbreak();
            }
            m_node = m_next;
            m_next = m_next->m_next;
            if (m_next == nullptr)
                break;
        }
    }
    return "<invalid pak id>";
}

// ea: 0x6719E0
bool PakManager::IsLoaded(const char* long_name) const
{
    reserved_dlist<PakFile>::dlist_node* m_node = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_node != nullptr ? m_node->m_next : nullptr;
    if (m_node == mActivePaks.m_end || m_next == nullptr)
        return false;
    while (1)
    {
        PakFile* pak = (PakFile*)m_node;
        const PakInfoNode* m_prev;
        if (pak->mPakType == kPakTypeCount)
        {
            m_prev = nullptr;
        }
        else
        {
            if (pak->mPakInfo == nullptr)
                pak->mPakInfo = PakManager::sInst->GetPakInfo(pak->mPakId);
            m_prev = pak->mPakInfo;
        }
        if (_stricmp((const char*)m_prev->longName, long_name) == 0)
            return true;
        m_node = m_next;
        m_next = m_next->m_next;
        if (m_next == nullptr)
            return false;
    }
}

// ea: 0x671D60
void PakManager::GetActivePakIds(ae_vector<TPakId>* id_set) const
{
    reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head != mActivePaks.m_end && m_next != nullptr)
    {
        do
        {
            TPakId id = ((PakFile*)m_head)->mPakId;
            id_set->push_back(id);
            m_head = m_next;
            m_next = m_next->m_next;
        } while (m_next != nullptr);
    }
}

// ea: 0x671DF0
int PakManager::GetUnloadableBankCount(bool mram) const
{
    (void)mram;
    reserved_dlist<PakFile>::dlist_node* m_head = mActivePaks.m_head;
    int count = 0;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head == mActivePaks.m_end || m_next == nullptr)
        return 0;
    do
    {
        PakFile* pak = (PakFile*)m_head;
        const PakInfoNode* m_prev;
        if (pak->mPakType == kPakTypeCount)
        {
            m_prev = nullptr;
        }
        else
        {
            if (pak->mPakInfo == nullptr)
                pak->mPakInfo = PakManager::sInst->GetPakInfo(pak->mPakId);
            m_prev = pak->mPakInfo;
        }
        if (0.0f != GetDistance(const_cast<PakInfoNode*>(m_prev)))
            count += BankManager::sInst->get_alloc_count(pak->mBankAlloc);
        m_head = m_next;
        m_next = m_next->m_next;
    } while (m_next != nullptr);
    return count;
}

// ea: 0x671A70
void PakManager::CopyContextStack(ae_sized_array<TPakId, 32>* ret) const
{
    TPakId pakId = mAnimPakId;
    ((ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack())
        .push_back(pakId);
    pakId = mGlobalPakId;
    ((ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack())
        .push_back(pakId);
    pakId = mDebugPakId;
    ((ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack())
        .push_back(pakId);
    pakId = mLevelPakId;
    ((ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack())
        .push_back(pakId);

    BitSet<99> used_paks;
    memset(&used_paks, 0, sizeof(used_paks));
    const ae_sized_array<TPakId, 128>& v10 = GetContextStack();
    for (int i = v10.m_size - 1; i >= 0; --i)
    {
        TPakId v12 = v10.m_elements[i];
        pakId = v12;
        if (v12 != PAK_ID_INVALID && !used_paks.Test(v12))
        {
            if (v12 > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
                AeAssert::gCurrentLine = 2412;
                AeAssert::gCurrentExpr =
                    "pakId > PAK_ID_INVALID && pakId < PAK_ID_MAX";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("bounds check"))
                    __debugbreak();
            }
            used_paks.Add(v12);
            ret->push_back(pakId);
        }
    }

    ae_sized_array<TPakId, 128>& s0 =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    if (s0.m_size != 0)
        s0.m_size -= 1;
    ae_sized_array<TPakId, 128>& s1 =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    if (s1.m_size != 0)
        s1.m_size -= 1;
    ae_sized_array<TPakId, 128>& s2 =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    if (s2.m_size != 0)
        s2.m_size -= 1;
    ae_sized_array<TPakId, 128>& s3 =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    if (s3.m_size != 0)
        s3.m_size -= 1;
}

// ea: 0x66FC30
void PakManager::GetPakPrerequisites(TPakId pakId,
                                     ae_sized_array<TPakId, 32>* ret,
                                     BitSet<99>* seen) const
{
    if (pakId == PAK_ID_INVALID)
        return;
    ret->push_back(pakId);
    seen->Add(pakId);
    if (pakId == mDebugPakId)
        return;
    if (mPakInfoBank == nullptr)
        return;
    const PakInfoNode* PakInfo = GetPakInfo(pakId);
    if (PakInfo == nullptr)
        return;
    if (PakInfo->prereqPakIds != nullptr)
    {
        for (int i = PAK_ID_MIN; i < PAK_ID_MAX; ++i)
        {
            if (PakInfo->prereqPakIds->Test(i) && !seen->Test(i))
            {
                seen->Add(i);
                pakId = (TPakId)i;
                ret->push_back(pakId);
            }
        }
        return;
    }

    unsigned int mSize = PakInfo->prereqs.mSize;
    BitSet<99> prereqPakIds;
    ae_sized_array<TPakId, 32> ids;
    memset(&prereqPakIds, 0, sizeof(prereqPakIds));
    ids.m_size = 0;
    for (unsigned int v6 = 0; v6 < mSize; ++v6)
    {
        TPakId v13 = PakInfo->prereqs.mList[v6]->pakId;
        if (!prereqPakIds.Test(v13))
            GetPakPrerequisites(v13, &ids, &prereqPakIds);
    }
    TPakId v14 = PakInfo->pakId;
    if (v14 != PAK_ID_MIN)
    {
        BitSet<99>* v15 =
            (BitSet<99>*)PakManager::sInst->MemAlloc(v14, 0x10u, false);
        if (v15 != nullptr)
            memset(v15, 0, sizeof(*v15));
        const_cast<PakInfoNode*>(PakInfo)->prereqPakIds = v15;
        if (v15 != nullptr)
        {
            v15->mBits[0] = prereqPakIds.mBits[0];
            v15->mBits[1] = prereqPakIds.mBits[1];
            v15->mBits[2] = prereqPakIds.mBits[2];
            v15->mBits[3] = prereqPakIds.mBits[3];
        }
    }
    for (int j = 0; j < ids.m_size; ++j)
    {
        TPakId v18 = ids.m_elements[j];
        if (prereqPakIds.Test(v18) && !seen->Test(v18))
        {
            seen->Add(v18);
            ret->push_back(v18);
        }
    }
}

// ============================================================================
// Free functions (streamer.o)
// ============================================================================

// ea: 0x665400
TPakId CurPakId()
{
    return PakManager::sInst->mLevelPakId;
}

// ea: 0x6653A0
void ValidatePakId(TPakId pakId)
{
    if (pakId != PAK_ID_INVALID && PakManager::sInst->mSlots[pakId] == NULL)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 236;
        AeAssert::gCurrentExpr = NULL;
        if (!AeAssert::IsIgnored() && AeAssert::Warning("bad/old pak id"))
            __debugbreak();
    }
}

// ea: 0x66FB10
void GetAllPaks(ae_sized_array<TPakId, 32>* ret)
{
    if (PakManager::sInst == nullptr)
        return;
    reserved_dlist<PakFile>::dlist_node* m_node =
        PakManager::sInst->mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* m_next =
        m_node != nullptr ? m_node->m_next : nullptr;
    if (m_node == PakManager::sInst->mActivePaks.m_end || m_next == nullptr)
        return;
    while (1)
    {
        TPakId elt = ((PakFile*)m_node)->mPakId;
        ret->push_back(elt);
        m_node = m_next;
        m_next = m_next->m_next;
        if (m_next == nullptr)
            break;
    }
}

// ea: 0x6758C0
void GetPakPrerequisites(TPakId pakId, ae_sized_array<TPakId, 32>* ret)
{
    if (PakManager::sInst != nullptr)
    {
        BitSet<99> seenPaks;
        memset(&seenPaks, 0, sizeof(seenPaks));
        PakManager::sInst->GetPakPrerequisites(pakId, ret, &seenPaks);
        TPakId anim_pak_id = PakManager::sInst->mAnimPakId;
        if (anim_pak_id != PAK_ID_INVALID && !seenPaks.Test(anim_pak_id))
            ret->push_back(anim_pak_id);
        anim_pak_id = PakManager::sInst->mGlobalPakId;
        if (anim_pak_id != PAK_ID_INVALID && !seenPaks.Test(anim_pak_id))
            ret->push_back(anim_pak_id);
    }
    else
    {
        ret->push_back(pakId);
    }
}

// ea: 0x675960
void get_context_stack(ae_sized_array<TPakId, 32>* ret)
{
    if (PakManager::sInst != nullptr)
        PakManager::sInst->CopyContextStack(ret);
}

// ea: 0x66FA30
void ConsoleSetPakDistance()
{
    if (Cmd_Argc() == 3)
    {
        const char* v0 = Cmd_Argv(1);
        const char* v1 = Cmd_Argv(2);
        float dist = (float)atof(v1);
        const PakInfoNode* PakInfo =
            PakManager::sInst->GetPakInfo(v0);
        if (PakInfo != nullptr)
        {
            const_cast<PakInfoNode*>(PakInfo)->userDistance = dist;
            ++PakManager::sComputeDistanceKey;
        }
        else
        {
            Com_Printf("SetPakDistance: Unknown pakfile '%s'\n", v0);
        }
    }
    else
    {
        Com_Printf("SetPakDistance: sets pak distance "
                   "'setpakdistance mons1_zone01 1'\n");
    }
}

// ea: 0x66FAB0
void ConsoleClearPakDistance()
{
    if (Cmd_Argc() == 2)
    {
        const char* v0 = Cmd_Argv(1);
        const PakInfoNode* PakInfo =
            PakManager::sInst->GetPakInfo(v0);
        if (PakInfo != nullptr)
            const_cast<PakInfoNode*>(PakInfo)->userDistance = 3.4028235e38f;
        else
            Com_Printf("ClearPakDistance: Unknown pakfile '%s'\n", v0);
    }
    else
    {
        Com_Printf("ClearPakDistance: undo SetPakDistance "
                   "'clearpakdistance mons1_zone01'\n");
    }
}

// ============================================================================
// PakFile small accessors (streamer.o PakFile.cpp)
// ============================================================================

unsigned char* PakFile::sHeaderBuffer = nullptr;
bool PakFile::sHeaderBufferUsed = false;
unsigned int PakFile::sHeaderBufferSize = 0;

// ea: 0x664B90
float PakFile::GetLoadTime() const
{
    LoadStats* mLoadStats = this->mLoadStats;
    if (mLoadStats != nullptr)
        return mLoadStats->total;
    return 0.0f;
}

// ea: 0x663330
void PakHeader::FixDown()
{
    if (numSections != 0)
    {
        for (unsigned int sectionIdx = 0; sectionIdx < numSections;
             ++sectionIdx)
        {
            Section* v1 = &sections[sectionIdx];
            for (int v2 = 0; v2 < v1->numFiles; ++v2)
            {
                File* v6 = &v1->files[v2];
                v6->shortName =
                    (const char*)((const char*)v6->shortName - (const char*)this);
                v6->longName = v6->longName != nullptr
                                   ? (const char*)((const char*)v6->longName
                                                   - (const char*)this)
                                   : nullptr;
            }
            v1->banks =
                (Bank*)((const char*)v1->banks - (const char*)this);
            v1->files =
                (File*)((const char*)v1->files - (const char*)this);
        }
    }
    sections = (Section*)((const char*)sections - (const char*)this);
}

// ea: 0x6633E0
void PakHeader::FixUp(bool persistentFixup, int doByteSwap)
{
    (void)doByteSwap;
    sections = (Section*)((char*)sections + (uintptr_t)this);
    for (unsigned int sectionIdx = 0; sectionIdx < numSections; ++sectionIdx)
    {
        Section* v8 = &sections[sectionIdx];
        v8->banks = (Bank*)((char*)v8->banks + (uintptr_t)this);
        v8->files = (File*)((char*)v8->files + (uintptr_t)this);
        if (persistentFixup)
        {
            v8->files = nullptr;
        }
        else
        {
            for (int v9 = 0; v9 < v8->numFiles; ++v9)
            {
                File* v12 = &v8->files[v9];
                v12->shortName =
                    (const char*)((char*)v12->shortName + (uintptr_t)this);
                v12->longName = v12->longName != nullptr
                                    ? (const char*)((char*)v12->longName
                                                    + (uintptr_t)this)
                                    : nullptr;
            }
        }
    }
}

// ============================================================================
// Small accessor cluster (streamer.o 0x6631D0 - 0x6638D0)
// ============================================================================

// ea: 0x6631D0
bool InplaceString::empty() const
{
    return mStr == nullptr;
}

// ea: 0x6631E0
mem_info::mem_info(unsigned char* data_, int size_)
{
    data = data_;
    size = size_;
}

// ea: 0x4B4350
NumBanks::NumBanks(float count, float aram)
{
    ps2 = count;
    xbox = count;
    xenon = count;
    pcx = count;
    ps3.main = count;
    ps3.lram = aram;
    gc.main = count;
    gc.aram = aram;
}

// ea: 0x663230 / 0x663240
float& NumBanks::to_float() { return xbox; }
float NumBanks::to_float() const { return xbox; }

// ea: 0x663250
float BankManager::GetNumBanks() const
{
    return mNumMramBanks;
}

// ea: 0x663260
unsigned int BankManager::get_bank_size() const
{
    return mMramBankSize;
}

// ea: 0x663270
TBankAlloc BankManager::get_free_banks() const
{
    return mFreeBanks;
}

// ea: 0x6632A0
unsigned char* BankManager::GetMramArena()
{
    return mMramArena;
}

// ea: 0x6632B0
bool BankManager::MemInBank(void* ptr) const
{
    unsigned char* mMramArena = this->mMramArena;
    return ptr >= mMramArena
           && ptr < &mMramArena[(unsigned int)(mMramBankSize * mNumMramBanks)];
}

// ea: 0x663300
float BankManager::get_low_water_mark() const
{
    return mLowestFreeAmount;
}

// ea: 0x663310
void APKBANK_UNPACK(unsigned int packed, unsigned short* typeIdx,
                    unsigned short* fileIdx)
{
    *typeIdx = (unsigned short)(packed >> 16);
    *fileIdx = (unsigned short)packed;
}

// ea: 0x6634A0
void* PakFile::get_dlist_node()
{
    return this;
}

// ea: 0x6634B0
void* PakFile::operator new(size_t size, bool forceHeapAlloc,
                            const char* file, int line)
{
    (void)file; (void)line;
    return PakFile::sAllocator->Allocate((unsigned int)size, forceHeapAlloc);
}

// ea: 0x6634D0 / 0x6634F0
void PakFile::operator delete(void* ptr, bool forceHeapAlloc,
                              const char* file, int line)
{
    (void)forceHeapAlloc; (void)file; (void)line;
    PakFile::sAllocator->Release(ptr);
}
void PakFile::operator delete(void* ptr)
{
    PakFile::sAllocator->Release(ptr);
}

// ea: 0x663510
TPakId PakFile::GetId() const { return mPakId; }

// ea: 0x4B44B0
nflFileID PakFile::GetFileId() const { return mFileId; }

// ea: 0x4B44C0
const PakHeader* PakFile::GetHeader() const { return mHeader; }

// ea: 0x663520
EPakType PakFile::GetType() const { return mPakType; }

// ea: 0x663530
bool PakFile::IsLoading() const { return mState == 1; }

// ea: 0x663550
bool PakFile::IsLoaded() const { return mState == LOADED; }

// ea: 0x663570
bool PakFile::IsUnloading() const
{
    return mState == 2 && mLoadingState != 4;  // UNLOADING_SERIALIZED
}

// ea: 0x6635A0
bool PakFile::IsUnloaded() const { return mState == UNLOADED; }

// ea: 0x6635C0
bool PakFile::IsCancelled() const { return mLoadingState == 8; }

// ea: 0x6635E0
bool PakManager::IsFillingBanks() const { return mFillingBanks; }

// ea: 0x6635F0
TPakId PakManager::GetAnimPakId() const { return mAnimPakId; }

// ea: 0x663600
bool PakManager::IsValid(TPakId id) const
{
    return id != PAK_ID_INVALID && mSlots[id] != nullptr;
}

// ea: 0x663630
TPakId PakManager::GetLevelPakId() const { return mLevelPakId; }

// ea: 0x663640
PakInfoNode* PakManager::UnConst(const PakInfoNode* pak) const
{
    return const_cast<PakInfoNode*>(pak);
}

// ea: 0x663650
const math::Position3& ZdNode::GetPosition() const { return mPosition; }

// ea: 0x663660
const StreamZone* ZdNode::GetStreamZone() const { return mZone; }

// ea: 0x663670
const BitSet<64>& ZdNode::GetZoneBitset() const { return mZoneBitset; }

// ea: 0x663680
const InplaceVector<float>& ZdNode::GetZoneDistances() const
{
    return mZoneDistances;
}

// ea: 0x663690
const BoundingBox& ZoneCellBox::GetBounds() const
{
    return *(const BoundingBox*)this;
}

// ea: 0x00517290
BoundingBox::~BoundingBox()
{
}

// ea: 0x005172A0
math::Position3 BoundingBox::center() const
{
    math::Position3 result;
    result.v = _mm_mul_ps(_mm_add_ps(vmax.v, vmin.v), _mm_set1_ps(0.5f));
    return result;
}

// ea: 0x005172F0
float BoundingBox::width() const
{
    return vmax.v.m128_f32[0] - vmin.v.m128_f32[0];
}

// ea: 0x00517320
float BoundingBox::height() const
{
    return vmax.v.m128_f32[1] - vmin.v.m128_f32[1];
}

// ea: 0x00517380
unsigned int ZoneCellDesc::GetCellId() const
{
    return mCellId;
}

// ea: 0x00517390
const BoundingBox& ZoneCellDesc::GetBounds() const
{
    return mAabb;
}

// ea: 0x684460
void BoundingBox::accumulate(const math::Position3& p)
{
    if (vmin.v.m128_f32[0] > p.v.m128_f32[0])
        vmin.v.m128_f32[0] = p.v.m128_f32[0];
    if (vmin.v.m128_f32[1] > p.v.m128_f32[1])
        vmin.v.m128_f32[1] = p.v.m128_f32[1];
    if (vmin.v.m128_f32[2] > p.v.m128_f32[2])
        vmin.v.m128_f32[2] = p.v.m128_f32[2];
    if (p.v.m128_f32[0] > vmax.v.m128_f32[0])
        vmax.v.m128_f32[0] = p.v.m128_f32[0];
    if (p.v.m128_f32[1] > vmax.v.m128_f32[1])
        vmax.v.m128_f32[1] = p.v.m128_f32[1];
    if (p.v.m128_f32[2] > vmax.v.m128_f32[2])
        vmax.v.m128_f32[2] = p.v.m128_f32[2];
}

// ea: 0x684900
bool ZoneOverrideBrush::IsInsidePlane(const math::Position3& point,
                                      const math::Vector4& vec) const
{
    __m128 v3 = _mm_mul_ps(point.v, vec.v);
    return _mm_shuffle_ps(vec.v, vec.v, 255).m128_f32[0]
           > (v3.m128_f32[0] + (_mm_shuffle_ps(v3, v3, 85).m128_f32[0]
                                + _mm_shuffle_ps(v3, v3, 170).m128_f32[0]));
}

// ea: 0x6636A0
const StreamZone* ZoneCellDesc::GetZone() const { return mZone; }

// ea: 0x6636B0
const char* StreamZone::GetName() const { return mName.mStr; }

// ea: 0x6636C0
void StreamZone::SetPakInfo(const PakInfoNode* n) { mPakInfo = n; }

// ea: 0x005173A0
const PakInfoNode* StreamZone::GetPakInfo() const { return mPakInfo; }

// ea: 0x6636D0
const BitSet<64>& ZoneOverrideBrushSet::GetZoneBitset() const
{
    return mZoneBitset;
}

// ea: 0x6636E0
const InplaceVector<float>& ZoneOverrideBrushSet::GetZoneDistances() const
{
    return mZoneDistances;
}

// ea: 0x6636F0
InplaceVector<InplaceTriple<InplaceString, const PakInfoNode*, float> >&
ZoneOverrideBrushSet::GetNonZoneDistances()
{
    return mNonZoneDistances;
}

// ea: 0x663700
bool ZoneOverrideBrushSet::WasInside() const { return mWasInside != 0; }

// ea: 0x663710
void ZoneOverrideBrushSet::SetInside(bool isInside)
{
    mWasInside = isInside;
}

// ea: 0x663720 (empty no-op)
void StreamZoneManager::OnLoading(TPakId pakId)
{
    (void)pakId;
}

// ea: 0x6676E0
void StreamZoneManager::OnUnload(TPakId pakId)
{
    int i = mFirstBank;
    unsigned int mFirstBank = (unsigned int)this->mFirstBank;
    if (mFirstBank == (unsigned int)-1)
        return;
    while (1)
    {
        ZoneBoundaryBank* v4 = ZoneBankAt(*this, mFirstBank);
        unsigned int v11 = mFirstBank;
        for (unsigned int v5 = 0; v5 < v4->mPtrs.mSize; ++v5)
        {
            StreamZone* zone = (StreamZone*)v4->mPtrs.mList[v5];
            const PakInfoNode* mPakInfo = zone->mPakInfo;
            if (mPakInfo != nullptr && mPakInfo->pakId == pakId)
            {
                for (unsigned int v9 = 0; v9 < zone->mCells.mSize; ++v9)
                {
                    const ZoneCellDesc* cell = zone->mCells.mList[v9];
                    R_RefreshCell(cell->mCellId, true);
                }
            }
        }
        i = ZoneBankAt(*this, v11)->mNextBank;
        if (i == -1)
            break;
        mFirstBank = (unsigned int)ZoneBankAt(*this, v11)->mNextBank;
    }
}

// ea: 0x6674D0
void StreamZoneManager::OnLoaded(TPakId pakId)
{
    int i = mFirstBank;
    unsigned int mFirstBank = (unsigned int)this->mFirstBank;
    if (mFirstBank == (unsigned int)-1)
        return;
    while (1)
    {
        ZoneBoundaryBank* v4 = ZoneBankAt(*this, mFirstBank);
        unsigned int v11 = mFirstBank;
        for (unsigned int v5 = 0; v5 < v4->mPtrs.mSize; ++v5)
        {
            StreamZone* zone = (StreamZone*)v4->mPtrs.mList[v5];
            const PakInfoNode* mPakInfo = zone->mPakInfo;
            if (mPakInfo != nullptr && mPakInfo->pakId == pakId)
            {
                for (unsigned int v9 = 0; v9 < zone->mCells.mSize; ++v9)
                {
                    const ZoneCellDesc* cell = zone->mCells.mList[v9];
                    R_RefreshCell(cell->mCellId, false);
                }
            }
        }
        i = ZoneBankAt(*this, v11)->mNextBank;
        if (i == -1)
            break;
        mFirstBank = (unsigned int)ZoneBankAt(*this, v11)->mNextBank;
    }
}

// ea: 0x678250
StreamZoneManager::StreamZoneManager()
{
    mLastCellNum = -1;
    mFirstBank = -1;
    mLastListSize = 0;
    mListSize = 0;
    AssetBankSet::sBankArray.push_back((AssetBankSet*)this);
    for (unsigned int i = 0; i < 99; ++i)
        mBankArray.m_elements[i] = nullptr;
    DebugRender_AddRenderer(DebugRender_sInst,
                            (void*)&StreamZoneManager::SingletonDebugRender);
    mDebugRenderMode = 0;
    zoneGraphScale = 0.0f;
    Cmd_AddCommand("ZoneGraph", ToggleZoneGraph);
}

// ea: 0x004DCD20
void* StreamZoneManager::operator new(size_t size, void* p)
{
    (void)size;
    return p;
}

// ea: 0x00684670
bool BoundingBox::intersect(const math::Position3& p) const
{
    return p.v.m128_f32[0] >= vmin.v.m128_f32[0]
        && p.v.m128_f32[1] >= vmin.v.m128_f32[1]
        && p.v.m128_f32[2] >= vmin.v.m128_f32[2]
        && vmax.v.m128_f32[0] >= p.v.m128_f32[0]
        && vmax.v.m128_f32[1] >= p.v.m128_f32[1]
        && vmax.v.m128_f32[2] >= p.v.m128_f32[2];
}

// ea: 0x4DCD30 (core.o inline)
void StreamZoneManager::CreateInst()
{
    if (sInst != nullptr
        && _tlAssert("c:\\cod\\code\\game\\StreamZoneManager.h", 31,
                     "sInst==0", "singleton already created!"))
        __debugbreak();

    void* memory = mem_heap_malloc_ctx(
        0x1E0u, 16, "pak", "c:\\cod\\code\\game\\StreamZoneManager.h", 31);
    if (memory != nullptr)
        sInst = new (memory) StreamZoneManager();
    else
        sInst = nullptr;
}

// ea: 0x4DCE30 (core.o inline)
void StreamZoneManager::DeleteInst()
{
    if (sInst == nullptr
        && _tlAssert("c:\\cod\\code\\game\\StreamZoneManager.h", 31,
                     "sInst!=0", "singleton not created!"))
        __debugbreak();
    if (sInst != nullptr)
    {
        sInst->~StreamZoneManager();
        mem_heap_free(sInst);
    }
    sInst = nullptr;
}

// ea: 0x6782F0
StreamZoneManager::~StreamZoneManager()
{
    Cmd_RemoveCommand("ZoneGraph");
    AssetBankSetRemoveSelf(this);
}

// ea: 0x687640
void StreamZoneManager::SingletonDebugRender()
{
    StreamZoneManager::sInst->DebugRender();
}

// ea: 0x66CD60
void StreamZoneManager::DebugRender()
{
    int mFirstBank = this->mFirstBank;
    while (mFirstBank != -1)
    {
        if (mFirstBank > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        ZoneBoundaryBank* v3 = mBankArray.m_elements[mFirstBank];
        int v7 = 4 * mFirstBank;
        if ((mDebugRenderMode & 2) != 0)
        {
            Color argb_color;
            argb_color.r = 0.0f;
            argb_color.g = 1.0f;
            argb_color.b = 0.0f;
            argb_color.a = 1.0f;
            DebugRender::RenderSphere(v3->mActiveNode->mPosition, 0.5f,
                                      argb_color);
        }
        if ((mDebugRenderMode & 1) != 0)
            RenderZoneGraph(v3);
        if ((mDebugRenderMode & 4) != 0)
        {
            for (unsigned int j = 0; j < v3->mPtrs.mSize; ++j)
            {
                // zone-name render pass (empty in the release build)
            }
        }
        mFirstBank = mBankArray.m_elements[v7 / 4]->mNextBank;
    }
}

// ea: 0x6678F0
void StreamZoneManager::RenderZoneGraph(const ZoneBoundaryBank* zbs)
{
    static const float size = 1.0f;    // @ 0xDF9050
    static const float scale_0 = 0.8f; // @ 0xDF904C

    float h = (mDebugRenderMode & 8) != 0 ? 4.5f : 0.02f;
    if (zoneGraphScale != 0.0f)
        h *= zoneGraphScale;
    int ScreenWidth = nglGetScreenWidth();
    int ScreenHeight = nglGetScreenHeight();
    math::Position3 cam = mLastPosition;

    auto project = [&](const math::Position3& p) -> math::Position3 {
        math::Position3 r;
        r.v.m128_f32[0] = (p.v.m128_f32[0] - cam.v.m128_f32[0]) * h
                          + ScreenWidth * 0.5f;
        r.v.m128_f32[1] = -(p.v.m128_f32[1] - cam.v.m128_f32[1]) * h
                          + ScreenHeight * 0.5f;
        r.v.m128_f32[2] = 0.0f;
        r.v.m128_f32[3] = 0.0f;
        return r;
    };

    unsigned int i = 0;
    if (zbs->mPtrs.mSize != 0)
    {
        do
        {
            if (only_this == -1 || (int)i == only_this)
            {
                const StreamZone* z = zbs->mPtrs.mList[i];
                math::Position3 centroid;
                centroid.v = _mm_setzero_ps();
                int nodeCount = 0;
                for (unsigned int cellIdx = 0;
                     cellIdx < z->mCells.mSize; ++cellIdx)
                {
                    const ZoneCellDesc* cell = z->mCells.mList[cellIdx];
                    math::Position3 l = project(cell->mAabb.vmin);
                    math::Position3 r2 = project(cell->mAabb.vmax);

                    if (reassign_colors)
                    {
                        unsigned int rc;
                        ((unsigned char*)&rc)[0] =
                            (unsigned char)(rand() % 255);
                        ((unsigned char*)&rc)[1] =
                            (unsigned char)(rand() % 255);
                        ((unsigned char*)&rc)[2] =
                            (unsigned char)(rand() % 255);
                        ((unsigned char*)&rc)[3] = 0xFF;
                        *(unsigned int*)((char*)z->mPakInfo + 0xC4) = rc;
                    }

                    TPakId pakId = z->mPakInfo->pakId;
                    Color cellCol;
                    if (pakId == PAK_ID_INVALID
                        || PakManager::sInst->mSlots[pakId] == nullptr
                        || PakManager::sInst->mSlots[pakId]->mState
                               != PakManager::STATE_LOADED)
                    {
                        if (PakManager::sInst->mCurrentPakId == pakId
                            && PakManager::sInst->mState
                                   == PakManager::STATE_LOADING)
                        {
                            cellCol = Color(1.0f, 1.0f, 0.0f, 0.5f);
                        }
                        else
                        {
                            unsigned int mc = z->mPakInfo->mapColor;
                            cellCol = Color(
                                ((mc >> 16) & 0xFF) / 255.0f,
                                ((mc >> 8) & 0xFF) / 255.0f,
                                (mc & 0xFF) / 255.0f, 0.5f);
                        }
                    }
                    else
                    {
                        cellCol = Color(0.0f, 1.0f, 0.0f, 0.5f);
                    }

                    DebugRender::RenderQuad2D(
                        l.v.m128_f32[0], r2.v.m128_f32[1],
                        r2.v.m128_f32[0], l.v.m128_f32[1], 1.0f, cellCol);

                    for (unsigned int boxIdx = 0;
                         boxIdx < cell->mCellBoxes.mSize; ++boxIdx)
                    {
                        const ZoneCellBox* box =
                            cell->mCellBoxes.mList[boxIdx];
                        for (unsigned int nIdx = 0;
                             nIdx < box->mZdNodes.mSize; ++nIdx)
                        {
                            const ZdNode* node = box->mZdNodes.mList[nIdx];
                            ++nodeCount;
                            math::Position3 p = project(node->mPosition);
                            centroid.v = _mm_add_ps(centroid.v, p.v);
                            if (node == zbs->mActiveNode)
                            {
                                DebugRender::RenderQuad2D(
                                    p.v.m128_f32[0] - size,
                                    p.v.m128_f32[1] + size,
                                    p.v.m128_f32[0] + size,
                                    p.v.m128_f32[1] - size, 1.0f,
                                    Color(1.0f, 0.0f, 0.0f, 0.5f));
                                DebugRender::RenderQuad2D(
                                    l.v.m128_f32[0], r2.v.m128_f32[1],
                                    r2.v.m128_f32[0], l.v.m128_f32[1], 1.0f,
                                    cellCol);
                            }
                            else
                            {
                                DebugRender::RenderQuad2D(
                                    p.v.m128_f32[0] - size,
                                    p.v.m128_f32[1] + size,
                                    p.v.m128_f32[0] + size,
                                    p.v.m128_f32[1] - size, 1.0f, cellCol);
                            }
                        }
                    }
                }

                unsigned int w = 0;
                unsigned int hgt = 0;
                nglGetStringDimensions(nglSysFont, z->mName.mStr, &w, &hgt,
                                       scale_0, scale_0);
                float hgtF = (float)(int)hgt;
                if (hgtF < 0.0f)
                    hgtF += 4294967300.0f;
                math::Position3 avg;
                if (nodeCount != 0)
                    avg.v = _mm_div_ps(centroid.v, _mm_set1_ps((float)nodeCount));
                else
                    avg.v = _mm_setzero_ps();
                DebugRender::RenderText(
                    z->mName.mStr, (int)(avg.v.m128_f32[0] - w * 0.5f),
                    (int)(avg.v.m128_f32[1] - hgtF * 0.5f),
                    Color(0.98f, 0.98f, 0.98f, 1.0f), 0.0f, scale_0);
                char distStr[32];
                if (z->mPakInfo->distance == 3.4028235e38f)
                    sprintf(distStr, "inf");
                else
                    sprintf(distStr, "%1.fm",
                            z->mPakInfo->distance * 0.0254);
                DebugRender::RenderText(
                    distStr, (int)avg.v.m128_f32[0],
                    (int)(avg.v.m128_f32[1] + 10.0),
                    Color(0.98f, 0.98f, 0.98f, 1.0f), 0.0f, scale_0);
            }
            ++i;
        } while (i < zbs->mPtrs.mSize);
    }

    if (currCl >= 16)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityManager.h";
        AeAssert::gCurrentLine = 19;
        AeAssert::gCurrentExpr = "idx<16";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
    }
    Entity* player = EntityManager::sInst->mPlayers[currCl];
    if (player == nullptr)
        return;

    math::Position3 playerPos = player->r.currentOrigin;
    math::Position3 pp = project(playerPos);
    float axis[3][3];
    AnglesToAxis(player->r.currentAngles, axis);
    math::Position3 fwd;
    fwd.v = _mm_setr_ps(playerPos.v.m128_f32[0] + axis[0][0] * 50.0f,
                        playerPos.v.m128_f32[1] + axis[0][1] * 50.0f,
                        playerPos.v.m128_f32[2] + axis[0][2] * 50.0f,
                        0.0f);
    math::Position3 fp = project(fwd);
    DebugRender::RenderQuad2D(pp.v.m128_f32[0] - 4.0f,
                              pp.v.m128_f32[1] + 4.0f,
                              pp.v.m128_f32[0] + 4.0f,
                              pp.v.m128_f32[1] - 4.0f, 0.0f,
                              Color(1.0f, 0.0f, 0.0f, 1.0f));
    DebugRender::RenderQuad2D(fp.v.m128_f32[0] - 2.0f,
                              fp.v.m128_f32[1] + 2.0f,
                              fp.v.m128_f32[0] + 2.0f,
                              fp.v.m128_f32[1] - 2.0f, 0.0f,
                              Color(1.0f, 0.0f, 0.0f, 1.0f));
    if (reassign_colors)
        reassign_colors = false;
}

// ea: 0x6795C0
void StreamZoneManager::DecodeBank(const char* name, unsigned char* data,
                                   int size, TPakId pakId)
{
    (void)name; (void)size;
    ZoneBoundaryBank* bank = (ZoneBoundaryBank*)data;
    bank->Fixup();
    bank->mPakId = pakId;
    for (unsigned int i = 0; i < bank->mPtrs.mSize; ++i)
    {
        StreamZone* zone = (StreamZone*)bank->mPtrs.mList[i];
        const PakInfoNode* n =
            PakManager::sInst->GetPakInfo(zone->mName.mStr);
        if (n == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\StreamZoneManager.cpp";
            AeAssert::gCurrentLine = 100;
            AeAssert::gCurrentExpr = "n";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                       "Unknown zone '%s' (remake level pakfile)",
                       zone->mName.mStr))
                __debugbreak();
        }
        zone->mPakInfo = n;
    }
    for (unsigned int o = 0; o < bank->mOverrideBoxes.mSize; ++o)
    {
        ZoneOverrideBrushSet* zob = bank->mOverrideBoxes.mList[o];
        for (unsigned int j = 0; j < zob->mNonZoneDistances.mSize; ++j)
        {
            InplaceTriple<InplaceString, const PakInfoNode*, float>* t =
                &zob->mNonZoneDistances.mList[j];
            const PakInfoNode* n =
                PakManager::sInst->GetPakInfo(t->a.mStr);
            if (n == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\StreamZoneManager.cpp";
                AeAssert::gCurrentLine = 113;
                AeAssert::gCurrentExpr = "n";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                           "Unknown pak '%s' in zone override box",
                           t->a.mStr))
                    __debugbreak();
            }
            t->b = n;
        }
    }
    for (unsigned int o = 0; o < bank->mToggleableOverrideBoxes.mSize; ++o)
    {
        ZoneOverrideBrushSet* zob =
            (ZoneOverrideBrushSet*)
                bank->mToggleableOverrideBoxes.mList[o];
        for (unsigned int j = 0; j < zob->mNonZoneDistances.mSize; ++j)
        {
            InplaceTriple<InplaceString, const PakInfoNode*, float>* t =
                &zob->mNonZoneDistances.mList[j];
            const PakInfoNode* n =
                PakManager::sInst->GetPakInfo(t->a.mStr);
            if (n == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\StreamZoneManager.cpp";
                AeAssert::gCurrentLine = 127;
                AeAssert::gCurrentExpr = "n";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                           "Unknown pak '%s' in (toggleable) zone override box",
                           t->a.mStr))
                    __debugbreak();
            }
            t->b = n;
        }
    }
    if (mBankArray.m_elements[pakId] != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\InplaceAssetBankSet.h";
        AeAssert::gCurrentLine = 109;
        AeAssert::gCurrentExpr = "mBankArray[(int)pakId] == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "We already have a bank for this pak id!"))
            __debugbreak();
    }
    mBankArray.m_elements[pakId] = bank;
    int curIdx = mFirstBank;
    if (curIdx == -1)
    {
        mFirstBank = pakId;
    }
    else
    {
        while (mBankArray.m_elements[curIdx]->mNextBank != -1)
            curIdx = mBankArray.m_elements[curIdx]->mNextBank;
        if (curIdx == -1)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\StreamZoneManager.cpp";
            AeAssert::gCurrentLine = 146;
            AeAssert::gCurrentExpr = "curIdx != -1";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Not possible!"))
                __debugbreak();
        }
        mBankArray.m_elements[curIdx]->mNextBank = pakId;
    }
    ++mListSize;
    mInitialCell = bank->mInitialCell;
    mInitialPosition = bank->mInitialPosition;
    Update(mInitialCell, &mInitialPosition, false);

    tlFixedString sky("sky");
    TPakId levelPakId = PAK_ID_INVALID;
    reserved_dlist<PakFile>::dlist_node* node =
        PakManager::sInst->mActivePaks.m_head;
    reserved_dlist<PakFile>::dlist_node* next =
        node != nullptr ? node->m_next : nullptr;
    if (node != PakManager::sInst->mActivePaks.m_end && next != nullptr)
    {
        while (1)
        {
            PakFile* pak = (PakFile*)node;
            if (pak->mPakType == kPakTypeLevel)
            {
                levelPakId = pak->mPakId;
                break;
            }
            if (next == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/reserved_dlist.h";
                AeAssert::gCurrentLine = 501;
                AeAssert::gCurrentExpr = "m_next != 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Please add a descriptive string"))
                    __debugbreak();
            }
            node = next;
            if (next->m_next == nullptr)
                break;
            next = next->m_next;
        }
    }
    nglMesh* mesh = (nglMesh*)InstanceBankMgr::sInst->Get(
        INSTBANK_TYPE_MESH, levelPakId, &sky);
    if (mesh == nullptr)
        mesh = nglMeshDirectory.Find(sky);
    s_worldData.mSky = mesh;
}

// InplaceAssetBank<StreamZone,...>::Fixup (streamer.o 0x6795E3)
void ZoneBoundaryBank::Fixup()
{
    if ((uintptr_t)mPtrFixupTable >= 0x10000000)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
        AeAssert::gCurrentLine = 122;
        AeAssert::gCurrentExpr =
            "((unsigned)mPtrFixupTable<0x10000000)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Fixup offset is unusually large"))
            __debugbreak();
    }
    mPtrFixupTable = (char*)this + (uintptr_t)mPtrFixupTable;
    PtrFixupTable_Fixup(mPtrFixupTable, this);
}

// ea: 0x66CCD0
void StreamZoneManager::SetDistances(ZoneBoundaryBank* bank, BitSet<64> bits,
                                     InplaceVector<float>* dists, bool force)
{
    InplaceVector<const StreamZone*>* p_mPtrs = &bank->mPtrs;
    bool bConnectPaths = ShouldConnectPaths();
    unsigned int v6 = 0;
    for (unsigned int v7 = 0; v7 < p_mPtrs->mSize; ++v7)
    {
        if (bits.Test((int)v7))
        {
            float v8 =
                bConnectPaths ? 1.0f : dists->mList[v6++];
            const StreamZone* zone = p_mPtrs->mList[v7];
            PakManager::sInst->SetDistance(zone->mPakInfo, v8, force);
        }
        else
        {
            if (bConnectPaths)
            {
                const StreamZone* zone = p_mPtrs->mList[v7];
                PakManager::sInst->SetDistance(zone->mPakInfo, 1.0f, force);
            }
        }
    }
}

// ea: 0x671FC0
void StreamZoneManager::SetDistances(ZoneBoundaryBank* bank,
                                     ZoneOverrideBrushSet* zob)
{
    SetDistances(bank, zob->mZoneBitset, &zob->mZoneDistances, true);
    InplaceVector<InplaceTriple<InplaceString, const PakInfoNode*, float> >*
        p_mNonZoneDistances = &zob->mNonZoneDistances;
    bool v15 = ShouldConnectPaths();
    for (unsigned int j = 0; j < p_mNonZoneDistances->mSize; ++j)
    {
        InplaceTriple<InplaceString, const PakInfoNode*, float>* v5 =
            &p_mNonZoneDistances->mList[j];
        const PakInfoNode* second = v5->b;
        float third = v5->c;
        if (v15)
        {
            third = 1.0f;
        }
        else if (third == -1.0f)
        {
            third = 3.4028235e38f;
        }
        PakManager* v12 = PakManager::sInst;
        if (second != nullptr)
        {
            const_cast<PakInfoNode*>(second)->distance = third;
            float banka = 3.4028235e38f;
            if (third != -1.0f)
                banka = third;
            for (unsigned int v9 = 0; v9 < second->prereqs.mSize; ++v9)
            {
                v12->SetDistance(second->prereqs.mList[v9], banka, false);
            }
        }
    }
}

// ea: 0x6849A0
const ZdNode* ZoneBoundaryBank::GetZdNode(int cellId,
                                          const math::Position3* position)
{
    if (cellId < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ZoneBoundaryBank.h";
        AeAssert::gCurrentLine = 554;
        AeAssert::gCurrentExpr = "cellId >= 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid cell id"))
            __debugbreak();
    }
    const ZoneCellDesc* v4 = mCells.mList[cellId];
    if (!v4->BoundsIntersect(*position))
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ZoneBoundaryBank.h";
        AeAssert::gCurrentLine = 559;
        AeAssert::gCurrentExpr = "zcd->GetBounds().intersect(position)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("bad bounding box on cell?"))
            __debugbreak();
    }
    return const_cast<ZoneCellDesc*>(v4)->GetZdNode(*position, false);
}

bool ZoneCellDesc::BoundsIntersect(const math::Position3& p) const
{
    return mAabb.intersect(p);
}

const ZdNode* ZoneCellBox::GetZdNode(const math::Position3& position)
{
    const ZdNode* nearest = nullptr;
    float nearestDistance = 3.4028235e38f;
    for (unsigned int i = 0; i < mZdNodes.mSize; ++i)
    {
        const ZdNode* node = mZdNodes.mList[i];
        __m128 delta = _mm_sub_ps(node->mPosition.v, position.v);
        __m128 squared = _mm_mul_ps(delta, delta);
        float distance = squared.m128_f32[0]
                       + _mm_shuffle_ps(squared, squared, 85).m128_f32[0]
                       + _mm_shuffle_ps(squared, squared, 170).m128_f32[0];
        if (nearestDistance > distance)
        {
            nearestDistance = distance;
            nearest = node;
        }
    }
    return nearest;
}

const ZdNode* ZoneCellDesc::GetZdNode(const math::Position3& position,
                                      bool force)
{
    (void)force;
    const ZdNode* result = nullptr;
    for (unsigned int i = 0; i < mCellBoxes.mSize; ++i)
    {
        ZoneCellBox* box = const_cast<ZoneCellBox*>(mCellBoxes.mList[i]);
        if (box->mAabb.intersect(position))
            result = box->GetZdNode(position);
    }
    return result;
}

// ea: 0x00686B20
bool ZoneOverrideBrushSet::IsInside(const math::Position3& point)
{
    InplaceVector<ZoneOverrideBrush>* brushes = &mBrushes;
    unsigned int index = 0;
    if (brushes->mSize == 0)
        return false;
    while (true)
    {
        ZoneOverrideBrush* brush = &brushes->mList[index];
        if (brush->IsInside(point))
            break;
        if (++index >= brushes->mSize)
            return false;
    }
    return true;
}

// ea: 0x00686AB0
bool ZoneOverrideBrush::IsInside(const math::Position3& point)
{
    const BoundingBox* aabb = reinterpret_cast<const BoundingBox*>(mAabb);
    if (!aabb->intersect(point))
        return false;
    const InplaceVector<math::Vector4>* planes =
        reinterpret_cast<const InplaceVector<math::Vector4>*>(mPlanes);
    for (unsigned int index = 0; index < planes->mSize; ++index)
    {
        if (!IsInsidePlane(point, planes->mList[index]))
            return false;
    }
    return true;
}

// disable_default_streaming (StreamZoneManager.cpp static cvar)
static cvar_t* disable_default_streaming = nullptr;
static bool s_disable_default_streaming_init = false;

// ea: 0x675B40
void StreamZoneManager::SetPriorities()
{
    PakManager::sInst->ResetPriorities(false);
    int mFirstBank = this->mFirstBank;
    if (mFirstBank == -1)
        return;
    while (1)
    {
        if (mFirstBank > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        ZoneBoundaryBank* v4 = mBankArray.m_elements[mFirstBank];
        const ZdNode* mActiveNode = v4->mActiveNode;
        const ZdNode* zn = mActiveNode;
        cvar_t* v6;
        if (s_disable_default_streaming_init)
        {
            v6 = disable_default_streaming;
        }
        else
        {
            s_disable_default_streaming_init = true;
            v6 = Cvar_Get("disable_default_streaming", "0", 0);
            disable_default_streaming = v6;
        }
        if (mActiveNode != nullptr && v6 != nullptr && v6->integer == 0)
        {
            SetDistances(v4, mActiveNode->mZoneBitset,
                         (InplaceVector<float>*)&mActiveNode->mZoneDistances,
                         false);
        }
        for (int v7 = 0; v7 < v4->mNumToggleableOverrideBoxesHit; ++v7)
        {
            SetDistances(v4,
                         (ZoneOverrideBrushSet*)
                             v4->mToggleableOverrideBoxes.mList[v7]);
        }
        mActiveNode = zn;
        for (unsigned int v9 = 0; v9 < v4->mOverrideBoxes.mSize; ++v9)
        {
            ZoneOverrideBrushSet* zob = v4->mOverrideBoxes.mList[v9];
            if (zob->mWasInside != 0)
                SetDistances(v4, zob);
        }
        mActiveNode = zn;
        if (mActiveNode != nullptr)
        {
            const StreamZone* mZone = mActiveNode->mZone;
            if (mZone != nullptr)
            {
                const PakInfoNode* mPakInfo = mZone->mPakInfo;
                if (mPakInfo != nullptr)
                {
                    const_cast<PakInfoNode*>(mPakInfo)->distance = 0.0f;
                    for (unsigned int v16 = 0;
                         v16 < mPakInfo->prereqs.mSize; ++v16)
                    {
                        PakManager::sInst->SetDistance(
                            mPakInfo->prereqs.mList[v16], 0.0f, false);
                    }
                }
            }
        }
        int i = mBankArray.m_elements[mFirstBank]->mNextBank;
        if (i == -1)
            break;
        mFirstBank = i;
    }
}

// ea: 0x678350
void StreamZoneManager::Update(int cellNum, const math::Position3* pos,
                               bool forceReset)
{
    if (cellNum == -1)
        return;
    mLastPosition.v = pos->v;
    int mLastListSize = this->mLastListSize;
    int mListSize = this->mListSize;
    bool resetPriorities = mLastListSize != mListSize;
    this->mLastListSize = mListSize;
    int mFirstBank = this->mFirstBank;
    mLastCellNum = cellNum;
    bool exists_active_zdnode = false;
    int b = mFirstBank;
    if (mFirstBank != -1)
    {
        while (1)
        {
            ZoneBoundaryBank* v12 = mBankArray.m_elements[mFirstBank];
            const ZdNode* lastNode = v12->mActiveNode;
            v12->mActiveNode = v12->GetZdNode(cellNum, pos);
            for (unsigned int v13 = 0; v13 < v12->mOverrideBoxes.mSize;
                 ++v13)
            {
                ZoneOverrideBrushSet* v15 = v12->mOverrideBoxes.mList[v13];
                bool IsInside = v15->IsInside(*pos);
                bool v17 = v15->mWasInside != 0;
                v15->mWasInside = IsInside;
                if (IsInside != v17)
                    resetPriorities = true;
            }
            for (int o = 0; o < (int)v12->mToggleableOverrideBoxes.mSize;
                 ++o)
            {
                ZoneOverrideBrushSet* v20 =
                    (ZoneOverrideBrushSet*)
                        v12->mToggleableOverrideBoxes.mList[o];
                bool v22 = false;
                for (unsigned int v30 = 0; v30 < v20->mBrushes.mSize; ++v30)
                {
                    if (v20->mBrushes.mList[v30].IsInside(*pos))
                    {
                        v22 = true;
                        break;
                    }
                }
                bool v23 = v20->mWasInside != 0;
                v20->mWasInside = v22;
                if (v22 && !v23)
                {
                    SetTopOverrideBrushSet(v12, v20);
                    resetPriorities = true;
                }
            }
            const ZdNode* mActiveNode = v12->mActiveNode;
            if (lastNode != mActiveNode)
                resetPriorities = true;
            if (mActiveNode != nullptr)
                exists_active_zdnode = true;
            int v25 = b;
            if (b > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            b = mBankArray.m_elements[b]->mNextBank;
            if (mBankArray.m_elements[v25]->mNextBank == -1)
                break;
            mFirstBank = b;
        }
    }
    if (this->mFirstBank != -1 && !exists_active_zdnode)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\StreamZoneManager.cpp";
        AeAssert::gCurrentLine = 294;
        AeAssert::gCurrentExpr = "exists_active_zdnode";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("There must be at least one active ZdNode"))
            __debugbreak();
    }
    if (resetPriorities || forceReset)
        SetPriorities();
}

// ea: 0x663730
int InstanceBank::strnicmp(const char* str1, const char* str2, int len) const
{
    const char* v4 = str1;
    if (*str1 != 0)
    {
        int v5 = str2 - str1;
        while (v4[v5] != 0 && len != 0)
        {
            char v6 = (char)tolower((unsigned char)*v4);
            char v7 = (char)tolower((unsigned char)v4[v5]);
            if (v6 != v7)
                return v6 < v7;
            char v8 = *++v4;
            --len;
            if (v8 == 0)
                return false;
        }
    }
    return false;
}

// ea: 0x6637B0
eInstanceBankType InstanceBank::GetType() const
{
    return (eInstanceBankType)mType;
}

// ea: 0x6637C0
const char* InstanceBank::GetTypeStr() const { return mTypeStr; }

// ea: 0x6637D0
XModelPartsManager* XModelPartsManager::Inst()
{
    return XModelPartsManager::sInst;
}

// ea: 0x6638E0
int tlFixedString::Order(const tlFixedString& rhs) const
{
    const unsigned int* l = &hash;
    const unsigned int* r = &rhs.hash;
    for (int v2 = 0; v2 < 8; ++v2)
    {
        if (l[v2] != r[v2])
            return r[v2] < l[v2] ? 1 : -1;
    }
    return 0;
}

// ea: 0x663F70
LoadStats::LoadStats()
{
    totalStart = 0;
    total = 0.0f;
    readStart = 0;
    readTotal = 0.0f;
}

// ea: 0x663FA0
float LoadStats::GetTime(unsigned __int64 start, unsigned __int64 end)
{
    return (float)((end - start) * 0.000000001363636402376034);
}

// ea: 0x6656B0
NumBanks PakManager::GetNumBanks(const PakInfoNode* pdt) const
{
    if (pdt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
        AeAssert::gCurrentLine = 2005;
        AeAssert::gCurrentExpr = "pdt != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    return pdt->numBanks;
}

// ea: 0x6657D0
bool PakManager::IsUnloaded(TPakId id) const
{
    return id == PAK_ID_INVALID || mSlots[id] == nullptr
           || mSlots[id]->mState == PakFile::UNLOADED;
}

// ea: 0x665800
bool PakManager::IsLoading(TPakId id) const
{
    return mCurrentPakId == id && mState == STATE_LOADING;
}

// ea: 0x665850
void PakManager::LoadWbk(tlFixedString audioBank, bool async)
{
    // Cross-object: AudioBankMgr (game.o) owns the real symbol; stub until
    // the audio bridge lands.
    (void)audioBank; (void)async;
}

// ============================================================================
// nal resource directory globals + accessors (streamer.o 0x6637E0 - 0x6638D0)
// ============================================================================
class nalAnyPose;
struct nalAnimFile;
class nalSceneAnim;
class nalBaseSkeleton;
template <typename T> class nalAnimClass;

tlResourceDirectory<nalAnimClass<nalAnyPose>>* nalAnimDirectory = nullptr;      // @ 0x10E95FC
tlResourceDirectory<nalAnimFile>* nalAnimFileDirectory = nullptr;               // @ 0x10E95F8
tlResourceDirectory<nalBaseSkeleton>* nalSkeletonDirectory = nullptr;           // @ 0x10EC618
tlResourceDirectory<nalSceneAnim>* nalSceneAnimDirectory = nullptr;             // @ 0x10E95F4
extern int nalReleaseSkeleton(nalBaseSkeleton* skeleton);  // nal.cpp
extern int nalReleaseAnimFile(nalAnimFile* file);          // nal.cpp
extern int nalReleaseSceneAnim(nalSceneAnim* sceneAnim);   // nal.cpp
extern nalBaseSkeleton* nalLoadSkeletonInPlace(nalBaseSkeleton* data);  // nal.cpp
extern nalAnimFile* nalLoadAnimFileInPlace(const tlFixedString& FileName,
                                           void* Data);      // nal.cpp
extern void XAnimEntryInvalidate();  // anim.o 0x92D5E0
void XAnimEntryInvalidate()  // stub until anim.o lands
{
}
tlFixedString GetName(const char* name);  // defined below (0x66F3C0)
// XAnimRelease (anim.o 0x5513E0; stub)
extern void XAnimRelease(nalAnimClass<nalAnyPose>* anim);
void XAnimRelease(nalAnimClass<nalAnyPose>* anim)
{
    (void)anim;
}

// ea: 0x6637E0 / 0x6637F0
tlResourceDirectory<nalAnimClass<nalAnyPose>>* nalGetAnimDirectory()
{
    return nalAnimDirectory;
}
void nalSetAnimDirectory(tlResourceDirectory<nalAnimClass<nalAnyPose>>* dir)
{
    nalAnimDirectory = dir;
}

// ea: 0x663800 / 0x663810
tlResourceDirectory<nalAnimFile>* nalGetAnimFileDirectory()
{
    return nalAnimFileDirectory;
}
void nalSetAnimFileDirectory(tlResourceDirectory<nalAnimFile>* dir)
{
    nalAnimFileDirectory = dir;
}

// ea: 0x663820 / 0x663830
tlResourceDirectory<nalBaseSkeleton>* nalGetSkeletonDirectory()
{
    return nalSkeletonDirectory;
}
void nalSetSkeletonDirectory(tlResourceDirectory<nalBaseSkeleton>* dir)
{
    nalSkeletonDirectory = dir;
}

// ea: 0x663840 / 0x663850
tlResourceDirectory<nalSceneAnim>* nalGetSceneAnimDirectory()
{
    return nalSceneAnimDirectory;
}
void nalSetSceneAnimDirectory(tlResourceDirectory<nalSceneAnim>* dir)
{
    nalSceneAnimDirectory = dir;
}

// nal resource views (nal.h; minimal fields for GetKeyPtr, verified IDA:
// vtable +0x00, Name +0x08 for nalBaseSkeleton / nalAnimClass<nalAnyPose>;
// Header.Name +0x10 for nalAnimFile / nalSceneAnim)
class nalBaseSkeleton {
public:
    void* __vftable;       // +0x00
    tlFixedString Name;    // +0x08
};

template <typename T>
class nalAnimClass {
public:
    void* __vftable;       // +0x00
    tlFixedString Name;    // +0x08
};

struct nalAnimFile {
public:
    uint8_t _pad[0x10];
    tlFixedString Name;    // +0x10 (Header.Name)
};

class nalSceneAnim {
public:
    uint8_t _pad[0x10];
    tlFixedString Name;    // +0x10 (Header.Name)
};

// ea: 0x6817A0 / 0x6817B0 / 0x6817C0 / 0x6817E0 (ngl texture/mesh/font
// overloads already live in render/ngl_aux.cpp)
const tlFixedString* GetKeyPtr(const nalBaseSkeleton* baseskeleton)
{
    return &baseskeleton->Name;
}
const tlFixedString* GetKeyPtr(const nalAnimClass<nalAnyPose>* baseanim)
{
    return &baseanim->Name;
}
const tlFixedString* GetKeyPtr(const nalAnimFile* animfile)
{
    return &animfile->Name;
}
const tlFixedString* GetKeyPtr(const nalSceneAnim* sceneanim)
{
    return &sceneanim->Name;
}

// cdResourceDirectory<T> static accessors over the nal globals
// (ea: 0x663860..0x6638D0)
template <>
tlResourceDirectory<nalAnimClass<nalAnyPose>>*
cdResourceDirectory<nalAnimClass<nalAnyPose>>::GetDirectory()
{
    return nalAnimDirectory;
}
template <>
void cdResourceDirectory<nalAnimClass<nalAnyPose>>::SetDirectory(
    tlResourceDirectory<nalAnimClass<nalAnyPose>>* dir)
{
    nalAnimDirectory = dir;
}

template <>
tlResourceDirectory<nalSceneAnim>*
cdResourceDirectory<nalSceneAnim>::GetDirectory()
{
    return nalSceneAnimDirectory;
}
template <>
void cdResourceDirectory<nalSceneAnim>::SetDirectory(
    tlResourceDirectory<nalSceneAnim>* dir)
{
    nalSceneAnimDirectory = dir;
}

template <>
tlResourceDirectory<nalAnimFile>*
cdResourceDirectory<nalAnimFile>::GetDirectory()
{
    return nalAnimFileDirectory;
}
template <>
void cdResourceDirectory<nalAnimFile>::SetDirectory(
    tlResourceDirectory<nalAnimFile>* dir)
{
    nalAnimFileDirectory = dir;
}

template <>
tlResourceDirectory<nalBaseSkeleton>*
cdResourceDirectory<nalBaseSkeleton>::GetDirectory()
{
    return nalSkeletonDirectory;
}
template <>
void cdResourceDirectory<nalBaseSkeleton>::SetDirectory(
    tlResourceDirectory<nalBaseSkeleton>* dir)
{
    nalSkeletonDirectory = dir;
}

// ============================================================================
// math SSE COMDATs (streamer.o 0x663930 - 0x663EF0)
// ============================================================================
namespace math {

// ea: 0x663930
float LengthSquared(const Position3& v)
{
    __m128 v1 = _mm_mul_ps(v.v, v.v);
    return v1.m128_f32[0] + (_mm_shuffle_ps(v1, v1, 0x55).m128_f32[0]
                             + _mm_shuffle_ps(v1, v1, 0xAA).m128_f32[0]);
}

// ea: 0x663990
Vector4 operator+(const Vector4& a, const Position3& b)
{
    Vector4 r;
    r.v = _mm_add_ps(a.v, _mm_shuffle_ps(b.v,
                                         _mm_shuffle_ps(_mm_set1_ps(1.0f),
                                                        b.v, 0xA0),
                                         0x34));
    return r;
}

// ea: 0x6639E0
Vector4 operator/(const Vector4& a, float b)
{
    Vector4 r;
    r.v = _mm_div_ps(a.v, _mm_shuffle_ps(_mm_set1_ps(b), _mm_set1_ps(b), 0));
    return r;
}

// ea: 0x663A20
Mat44::Mat44(const Vector4& _x, const Vector4& _y, const Vector4& _z,
             const Vector4& _w)
{
    x.v = _x.v;
    y.v = _y.v;
    z.v = _z.v;
    w.v = _w.v;
}

// ea: 0x663AC0 / 0x663AD0 / 0x663AE0 / 0x663AF0
Vector4& Mat44::GetX() { return x; }
Vector4& Mat44::GetY() { return y; }
Vector4& Mat44::GetZ() { return z; }
Vector4& Mat44::GetW() { return w; }

// ea: 0x663B00
Mat44::Mat44(const Mat33& m)
{
    x.v = _mm_shuffle_ps(_mm_setzero_ps(), m.x.v, 0xA0);
    y.v = _mm_shuffle_ps(_mm_setzero_ps(), m.y.v, 0xA0);
    z.v = _mm_shuffle_ps(_mm_setzero_ps(), m.z.v, 0xA0);
    w.v = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);  // Float4_WAxis
}

// ea: 0x663B80 / 0x663C00
Vector4 Mul(const Position3& v, const Mat44& m)
{
    Vector4 r;
    r.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v.v, v.v, 0x00), m.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(v.v, v.v, 0x55), m.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v.v, v.v, 0xAA), m.z.v),
                   m.w.v));
    return r;
}
Vector4 operator*(const Position3& v, const Mat44& m)
{
    return Mul(v, m);
}

// ea: 0x663C80
const Vector4& Vector4::operator*=(const Mat44& m)
{
    __m128 v = this->v;
    this->v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v, v, 0x00), m.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(v, v, 0x55), m.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v, v, 0xAA), m.z.v),
                   _mm_mul_ps(_mm_shuffle_ps(v, v, 0xFF), m.w.v)));
    return *this;
}

// ea: 0x663D10
Mat44 Mul(const Mat44& a, const Mat33& b)
{
    __m128 v3 = _mm_shuffle_ps(_mm_setzero_ps(), b.x.v, 0xA0);
    __m128 v4 = _mm_shuffle_ps(_mm_setzero_ps(), b.y.v, 0xA0);
    __m128 v6 = _mm_shuffle_ps(_mm_setzero_ps(), b.z.v, 0xA0);
    __m128 waxis = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);  // Float4_WAxis

    Vector4 tmp_4, tmp_20;
    tmp_4.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(a.y.v, a.y.v, 0x00), v3),
                   _mm_mul_ps(_mm_shuffle_ps(a.y.v, a.y.v, 0x55), v4)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(a.y.v, a.y.v, 0xAA), v6),
                   _mm_mul_ps(_mm_shuffle_ps(a.y.v, a.y.v, 0xFF), waxis)));
    Vector4 v7;
    v7.v = a.w.v;
    tmp_20.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(a.z.v, a.z.v, 0x00), v3),
                   _mm_mul_ps(_mm_shuffle_ps(a.z.v, a.z.v, 0x55), v4)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(a.z.v, a.z.v, 0xAA), v6),
                   _mm_mul_ps(_mm_shuffle_ps(a.z.v, a.z.v, 0xFF), waxis)));

    Mat44 result;
    result.x.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(a.x.v, a.x.v, 0x00), v3),
                   _mm_mul_ps(_mm_shuffle_ps(a.x.v, a.x.v, 0x55), v4)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(a.x.v, a.x.v, 0xAA), v6),
                   _mm_mul_ps(_mm_shuffle_ps(a.x.v, a.x.v, 0xFF), waxis)));
    result.y = tmp_4;
    result.z = tmp_20;
    result.w.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v7.v, v7.v, 0x00), v3),
                   _mm_mul_ps(_mm_shuffle_ps(v7.v, v7.v, 0x55), v4)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v7.v, v7.v, 0xAA), v6),
                   _mm_mul_ps(_mm_shuffle_ps(v7.v, v7.v, 0xFF), waxis)));
    return result;
}

// ea: 0x681450
Mat33 AxisSinCosToRotMat(const Dir3& v, float s, float c)
{
    // R(-angle) with axis v: rows built from {c, s*axis} cross terms.
    __m128 v6 = _mm_sub_ps(
        v.v, _mm_mul_ps(v.v, _mm_shuffle_ps(_mm_set1_ps(c), _mm_set1_ps(c), 0)));
    __m128 v7 = _mm_mul_ps(
        v.v, _mm_shuffle_ps(_mm_set1_ps(s), _mm_set1_ps(s), 0));
    __m128 v8 = _mm_xor_ps(_mm_set1_ps(-0.0f), v7);  // -axis*s
    __m128 v12 = _mm_setr_ps(c,
                             _mm_shuffle_ps(v7, v7, 170).m128_f32[0],
                             _mm_shuffle_ps(v8, v8, 85).m128_f32[0],
                             0.0f);
    __m128 v11 = _mm_setr_ps(_mm_shuffle_ps(v8, v8, 170).m128_f32[0],
                             c,
                             v7.m128_f32[0],
                             0.0f);
    __m128 tmp_36 = _mm_setr_ps(_mm_shuffle_ps(v7, v7, 85).m128_f32[0],
                                v8.m128_f32[0],
                                c,
                                0.0f);
    Mat33 result;
    result.x.v = _mm_add_ps(v12, _mm_mul_ps(v6, _mm_shuffle_ps(v.v, v.v, 0)));
    result.y.v = _mm_add_ps(v11, _mm_mul_ps(v6, _mm_shuffle_ps(v.v, v.v, 85)));
    result.z.v = _mm_add_ps(tmp_36, _mm_mul_ps(v6, _mm_shuffle_ps(v.v, v.v, 170)));
    return result;
}

// ea: 0x685000
Mat33 AxisAngleToRotMat(const Dir3& axis, float angle)
{
    Vector4 radians;
    radians.v = _mm_set1_ps(angle);
    Vector4 sc = SinCos<3, 0, 0, 0>(radians);
    return AxisSinCosToRotMat(axis, sc.v.m128_f32[0], sc.v.m128_f32[1]);
}

// ea: 0x51E110
Vector4 Vector4_One()
{
    Vector4 r;
    r.v = _mm_set1_ps(1.0f);
    return r;
}

}  // namespace math

// ea: 0x664BC0
float PakFile::GetProgress() const
{
    if (mState != LOADING)
        return 1.0f;
    if (mLoadingState == LOADING_DATA)
        return (float)mCurrentFile
               / mHeader->sections[mDefaultSectionIdx].numFiles;
    return 0.0f;
}

// ea: 0x664C30
bool PakFile::IsCancelOk() const
{
    return mLoadingState == LOADING_DATA;
}

// ea: 0x664C40
void PakFile::CopyHeader()
{
    PakHeader* copy;
    if (mPakType == kPakTypeFrontEnd)
        copy = (PakHeader*)gActorHeap->Malloc(mHeader->persistentSize, 4);
    else
        copy = (PakHeader*)mem_heap_malloc(mHeader->persistentSize);
    mHeader->FixDown();
    memcpy(copy, mHeader, mHeader->persistentSize);
    sHeaderBufferUsed = false;
    mHeaderBuffer = (unsigned char*)copy;
    mHeader = copy;
    copy->FixUp(true, 0);
}

// ea: 0x664CD0
void PakFile::DetermineNextFile()
{
    if (mDefaultSectionIdx == -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 1261;
        AeAssert::gCurrentExpr = "mDefaultSectionIdx != -1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no default section!"))
            __debugbreak();
    }
    PakHeader::Section* sections = mHeader->sections;
    PakHeader::Section& sec = sections[mDefaultSectionIdx];
    mCurrentFilePtr = nullptr;
    mCurrentApk = nullptr;
    mCurrentApkFileEntry = nullptr;
    mCurrentApkFileTypeEntry = nullptr;
    unsigned int mCurrentFile = this->mCurrentFile;
    if (mCurrentFile == sec.numFiles)
    {
        mLoadingState = (ELoadingState)LOADING_DONE;
        return;
    }
    PakHeader::File* file =
        &((PakHeader::File*)sec.files)[mCurrentFile];
    unsigned int bankIdx = file->bankIndex;
    if (bankIdx >= sec.numBanks)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 1280;
        AeAssert::gCurrentExpr = "bankIdx >= 0 && bankIdx < mHeader->sections[mDefaultSectionIdx].numBanks";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("bank idx out of bounds!"))
            __debugbreak();
    }
    PakHeader::Bank* banks = (PakHeader::Bank*)sec.banks;
    PakHeader::Bank& bank = banks[bankIdx];
    if ((bank.flags & 0x80) == 0)
    {
        mCurrentFilePtr = &bank.memptr[file->bankOffset];
    }
    else if ((bank.flags & 0x40000) != 0)
    {
        // apk-embedded bank: file type entry inside the apk file.
        void* apk = (void*)((char*)bank.memptr + 8);
        mCurrentApk = apk;
        unsigned int offset = file->bankOffset;
        unsigned int nSections = *(unsigned int*)((char*)apk + 8);
        void* fileTypes = *(void**)((char*)apk + 0x10);
        mCurrentApkFileTypeEntry =
            (void*)((char*)fileTypes + 4 * (nSections + 5) * (offset >> 16));
        apk::apkFileTypeEntry* typeEntry =
            (apk::apkFileTypeEntry*)mCurrentApkFileTypeEntry;
        uint32_t fileIndex = (uint16_t)offset;
        mCurrentApkFileEntry =
            (void*)((char*)typeEntry->FirstEntry
                    + 4 * fileIndex * (typeEntry->NSections + 1));
    }
}

// ea: 0x664E50
const PakInfoNode* PakFile::GetInfo()
{
    if (mPakType == kPakTypeCount)
        return nullptr;
    if (mPakInfo == nullptr)
        mPakInfo = PakManager::sInst->GetPakInfo(mPakId);
    return mPakInfo;
}

// ea: 0x664E80
void PakFile::ValidateRange(void* data)
{
    (void)data;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
    AeAssert::gCurrentLine = 1821;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error("trying to free data not inside bank!"))
        __debugbreak();
}

// ea: 0x664EC0
bool PakFile::IsRequestValid(TRequestId* req) const
{
    return req->nflId != NFL_REQUEST_ID_INVALID;
}

// ea: 0x664EE0
void PakFile::SetRequestInvalid(TRequestId* req) const
{
    req->nflId = NFL_REQUEST_ID_INVALID;
}

// ea: 0x664EF0
bool PakFile::IsRequestDone(TRequestId* requestId) const
{
    int RequestState = (int)nflGetRequestState(requestId->nflId);
    switch ((nflRequestState)RequestState)
    {
    case NFL_REQUEST_STATE_INVALID:
    case NFL_REQUEST_STATE_COMPLETED:
        break;
    case NFL_REQUEST_STATE_CANCELED:
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 2391;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("nfl request was cancelled"))
            __debugbreak();
        break;
    case NFL_REQUEST_STATE_TIMEOUT:
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 2394;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("nfl request was timeout"))
            __debugbreak();
        break;
    case NFL_REQUEST_STATE_ERROR:
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 2397;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("nfl request was error"))
            __debugbreak();
        break;
    default:
        if (RequestState != NFL_REQUEST_STATE_ACTIVE)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
            AeAssert::gCurrentLine = 2400;
            AeAssert::gCurrentExpr = "request_state == NFL_REQUEST_STATE_ACTIVE";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("sanity check"))
                __debugbreak();
        }
        break;
    }
    return RequestState == NFL_REQUEST_STATE_INVALID;
}

// ============================================================================
// Free streamer helpers (streamer.o)
// ============================================================================

// ea: 0x664000
const char* GetFileExt(const char* name)
{
    const char* result = name;
    if (*name != 0)
    {
        do
            ++result;
        while (*result != 0);
    }
    if (*result != '.')
    {
        do
        {
            if (result <= name)
                break;
            --result;
        } while (*result != '.');
    }
    return result;
}

// ea: 0x664030
RecordResourceType GetWorkAmount(const char* name)
{
    const char* v1 = GetFileExt(name);

    static const char* const tex_exts[] = {
        ".ifl", ".pctex", ".pcxtex", ".ps2tex", ".ps3tex", ".xbtex",
        ".xetex", ".pcdtex", ".pcxdtex", ".ps2dtex", ".ps3dtex",
        ".xbdtex", ".xedtex", ".icn", ".pclm", ".pcxlm", ".ps2lm",
        ".ps3lm", ".xblm", ".xelm", ".pcfont", ".pcxfont", ".ps2font",
        ".ps3font", ".xbfont", ".xefont", ".pcxfx", ".xefx", ".ps3fx",
        ".charskel", ".pcmesh", ".pcxmesh", ".ps2mesh", ".ps3mesh",
        ".xbmesh", ".xemesh", ".gcmesh", ".spt",
    };
    for (const char* ext : tex_exts)
    {
        if (_stricmp(v1, ext) == 0)
            return Resource_Texture;
    }

    static const char* const cell_exts[] = {
        ".pccell", ".pcxcell", ".ps2cell", ".ps3cell", ".xbcell",
        ".xecell", ".gccell",
    };
    for (const char* ext : cell_exts)
    {
        if (_stricmp(v1, ext) == 0)
            return (RecordResourceType)10;  // cell
    }

    static const char* const anim_exts[] = {
        ".pcanim", ".pcxanim", ".ps2anim", ".ps3anim", ".xbanim",
        ".xeanim", ".gcanim", ".pcsanim", ".pcxsanim", ".ps2sanim",
        ".ps3sanim", ".xbsanim", ".xesanim", ".gcsanim", ".pcskel",
        ".pcxskel", ".ps2skel", ".ps3skel", ".xbskel", ".xeskel",
        ".gcskel",
    };
    for (const char* ext : anim_exts)
    {
        if (_stricmp(v1, ext) == 0)
            return Resource_Texture;
    }

    static const char* const db_exts[] = {
        ".db", ".dbb", ".gdb", ".gdbb", ".dlg", ".dlgb", ".btr",
        ".btrb", ".fli", ".ib", ".lgrid", ".lgridb", ".bmips",
        ".xmpb", ".xmpbb", ".xmb", ".xmbb", ".aitb", ".aitbb",
    };
    for (const char* ext : db_exts)
    {
        if (_stricmp(v1, ext) == 0)
            return Resource_VertexBuffer;
    }

    if (_stricmp(v1, ".atrb") == 0 || _stricmp(v1, ".atrbb") == 0)
        return Resource_Texture;

    if (_stricmp(v1, ".cfgstr") == 0 || _stricmp(v1, ".cfgstrb") == 0
        || _stricmp(v1, ".zbb") == 0 || _stricmp(v1, ".zbbb") == 0)
        return Resource_VertexBuffer;

    if (_stricmp(v1, ".scn") == 0 || _stricmp(v1, ".scnb") == 0)
        return (RecordResourceType)99;  // scene

    if (_stricmp(v1, ".heap") == 0)
        return Resource_VertexBuffer;

    if (_stricmp(v1, ".bpe") == 0 || _stricmp(v1, ".bpeb") == 0
        || _stricmp(v1, ".bgi") == 0 || _stricmp(v1, ".bgib") == 0)
        return Resource_Texture;

    if (_stricmp(v1, ".wbk") == 0 || _stricmp(v1, ".wen") == 0
        || _stricmp(v1, ".wde") == 0 || _stricmp(v1, ".wfr") == 0
        || _stricmp(v1, ".wes") == 0 || _stricmp(v1, ".wit") == 0
        || _stricmp(v1, ".lpth") == 0 || _stricmp(v1, ".zpth") == 0)
        return Resource_VertexBuffer;

    if (_stricmp(v1, ".panel") == 0)
        return (RecordResourceType)10;

    if (_stricmp(v1, ".spg") == 0 || _stricmp(v1, ".spgb") == 0
        || _stricmp(v1, ".drng") == 0 || _stricmp(v1, ".drngb") == 0)
        return Resource_Texture;

    if (_stricmp(v1, ".rel") != 0 && _stricmp(v1, ".stb") != 0
        && _stricmp(v1, ".stbb") != 0)
    {
        if (_stricmp(v1, ".cg") != 0 && _stricmp(v1, ".cgb") != 0
            && _stricmp(v1, ".dcg") != 0 && _stricmp(v1, ".dcgb") != 0
            && _stricmp(v1, ".ncg") != 0 && _stricmp(v1, ".ncgb") != 0)
        {
            if (_stricmp(v1, ".dtr") != 0 && _stricmp(v1, ".dtrb") != 0
                && _stricmp(v1, ".phy") != 0
                && _stricmp(v1, ".phyb") != 0
                && _stricmp(v1, ".bindic") != 0
                && _stricmp(v1, ".bindicb") != 0
                && _stricmp(v1, ".mpanim") != 0
                && _stricmp(v1, ".mpanimb") != 0
                && _stricmp(v1, ".vce") != 0
                && _stricmp(v1, ".bvce") != 0
                && _stricmp(v1, ".csv") != 0)
            {
                if (_stricmp(v1, ".wind") != 0)
                    _stricmp(v1, ".seed");
            }
            return Resource_VertexBuffer;
        }
        return Resource_Texture;
    }
    return Resource_VertexBuffer;
}

// ea: 0x665080
unsigned char* stream_alloc(int size, bool aram)
{
    if (size == 0)
        return nullptr;
    if (size <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 64;
        AeAssert::gCurrentExpr = "size > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid size"))
            __debugbreak();
    }
    if (aram)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 66;
        AeAssert::gCurrentExpr = "!aram";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("attempting to allocate from aram!"))
            __debugbreak();
    }
    void* v4 = mem_heap_malloc(4096, size);
    if (v4 == nullptr)
    {
        char msg_buff[128];
        sprintf(msg_buff, "stream_alloc: out of memory! %d", size);
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 121;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error(msg_buff))
            __debugbreak();
    }
    return (unsigned char*)v4;
}

// ea: 0x6651A0
void stream_free(unsigned char* ptr, bool aram)
{
    (void)aram;
    if (PakFile::sHeaderBuffer == ptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 133;
        AeAssert::gCurrentExpr = "PakFile::sHeaderBuffer != ptr";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("trying to free reserved buffer"))
            __debugbreak();
    }
    if (ptr != nullptr)
        mem_heap_free(ptr);
}

// ea: 0x4E3560 (core.o BitSet<64>::Test; same source line for every N)
template <int N>
bool BitSet<N>::Test(int v) const
{
    if (v >> 5 >= kNumWords)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/BitSet.h";
        AeAssert::gCurrentLine = 123;
        AeAssert::gCurrentExpr = "idx < GetNumWords()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Please add a descriptive string"))
            __debugbreak();
    }
    return ((1 << (v & 0x1F)) & mBits[v >> 5]) != 0;
}

// ea: 0x66B120
bool TBankAlloc::IsEmpty() const
{
    for (int v1 = 0; v1 < 2; ++v1)
    {
        if (mram_alloc1.mBits[v1] != 0)
            return false;
    }
    for (int v2 = 0; v2 < 2; ++v2)
    {
        if (mram_alloc2.mBits[v2] != 0)
            return false;
    }
    return true;
}

// ea: 0x66B150
void TBankAlloc::Clear()
{
    mram_alloc1.mBits[1] = 0;
    mram_alloc1.mBits[0] = 0;
    mram_alloc2.mBits[1] = 0;
    mram_alloc2.mBits[0] = 0;
}

// ea: 0x66B160
float TBankAlloc::ToFloat() const
{
    float c = 0.0f;
    for (int v2 = 0; v2 < 64; ++v2)
    {
        if (mram_alloc1.Test(v2))
            c += 0.5f;
        if (mram_alloc2.Test(v2))
            c += 0.5f;
    }
    return c;
}

// ea: 0x66BC50
NumBanks BankManager::get_free_count() const
{
    NumBanks count;
    memset(&count, 0, sizeof(count));
    bool hasHalf = false;
    int v3 = 0;
    float limit = mNumMramBanks + 0.5f;
    if (limit > 0.0f)
    {
        do
        {
            bool v5 = mFreeBanks.mram_alloc1.Test(v3);
            bool v6 = mFreeBanks.mram_alloc2.Test(v3);
            if (v5 && v6)
            {
                count.xbox += 1.0f;
            }
            else if (v5 != v6 && !hasHalf)
            {
                count.xbox += 0.5f;
                hasHalf = true;
            }
            ++v3;
        } while (limit > v3);
    }
    return count;
}

// ea: 0x66F390
bool BankManager::can_alloc(NumBanks num_banks) const
{
    return num_banks.xbox <= get_free_count().xbox;
}

// ea: 0x66B910
int BankManager::get_alloc_count(const TBankAlloc& bat) const
{
    int count = 0;
    if (mNumMramBanks + 0.5f > 0.0f)
    {
        int v4 = 0;
        do
        {
            if (bat.mram_alloc1.Test(v4) || bat.mram_alloc2.Test(v4))
                ++count;
            ++v4;
        } while (mNumMramBanks + 0.5f > v4);
        return count;
    }
    return 0;
}

BankManager* BankManager::sInst = nullptr;  // ?sInst@BankManager@@2PAV1@A @ 0xF592F4

// ea: 0x004B43C0
void BankManager::CreateInst()
{
    if (sInst != nullptr
        && _tlAssert("c:\\cod\\code\\game\\BankManager.h", 69,
                     "sInst==0", "singleton already created!"))
        __debugbreak();

    void* memory = mem_heap_malloc_ctx(
        0x30u, 4, "bank", "c:\\cod\\code\\game\\BankManager.h", 69);
    if (memory != nullptr)
        sInst = new (memory) BankManager();
    else
        sInst = nullptr;
}

// ea: 0x004E57B0
void BankManager::DeleteInst()
{
    if (sInst == nullptr
        && _tlAssert("c:\\cod\\code\\game\\BankManager.h", 69,
                     "sInst!=0", "singleton not created!"))
        __debugbreak();
    if (sInst != nullptr)
    {
        sInst->~BankManager();
        mem_heap_free(sInst);
    }
    sInst = nullptr;
}

void* g_bank_space = nullptr;      // ?g_bank_space@@3PAXA @ 0xF592CC
int   g_bank_space_size = 0;       // ?g_bank_space_size@@3HA @ 0xF592D0

// ea: 0x66B5E0 (BankManager.cpp file-static)
static void InternalAllocate(float numAvailableBanks, BitSet<64>* freeBanks1,
                             BitSet<64>* freeBanks2, BitSet<64>* alloc1,
                             BitSet<64>* alloc2, float* numBanks, bool lram)
{
    for (int i = 0; *numBanks >= 1.0f; ++i)
    {
        if (numAvailableBanks <= (float)i)
            break;
        if (freeBanks1->Test(i) && freeBanks2->Test(i))
        {
            freeBanks1->Rmv(i);
            alloc1->Add(i);
            freeBanks2->Rmv(i);
            alloc2->Add(i);
            *numBanks -= 1.0f;
        }
    }
    bool half = false;
    // The reference enters the half-bank path only when a fractional bank
    // remains.  It compares the value with its truncation, rather than merely
    // testing for a positive request; the latter can consume a half bank for
    // an exhausted whole-bank request and leads to the allocator assertion.
    const float wholeBanks = (float)(int)(*numBanks);
    if (*numBanks > wholeBanks)
    {
        half = true;
        int v9 = 0;
        const float availableHalfBanks =
            (float)(int)(numAvailableBanks + 0.5f);
        if (availableHalfBanks > 0.0f)
        {
            bool v10;
            for (;;)
            {
                v10 = freeBanks1->Test(v9);
                if (v10 != freeBanks2->Test(v9))
                    break;
                if (availableHalfBanks <= (float)++v9)
                    goto scan_free1;
            }
            if (v10)
            {
                freeBanks1->Rmv(v9);
                alloc1->Add(v9);
            }
            else
            {
                freeBanks2->Rmv(v9);
                alloc2->Add(v9);
            }
            *numBanks -= 0.5f;
            half = false;
        }
        else
        {
scan_free1:
            int v11 = 0;
            if (numAvailableBanks > 0.0f)
            {
                while (1)
                {
                    if (v11 >> 5 >= 2)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "../ae\\core/BitSet.h";
                        AeAssert::gCurrentLine = 123;
                        AeAssert::gCurrentExpr = "idx < GetNumWords()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Please add a descriptive string"))
                            __debugbreak();
                    }
                    if (((1 << (v11 & 0x1F))
                         & freeBanks1->mBits[v11 >> 5]) != 0)
                        break;
                    if (numAvailableBanks <= (float)++v11)
                        goto done;
                }
                freeBanks1->Rmv(v11);
                alloc1->Add(v11);
                *numBanks -= 0.5f;
                half = false;
            }
        }
    }
done:
    if (!lram && half && *numBanks == 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 634;
        AeAssert::gCurrentExpr = "!half || numBanks != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("unsuccessful alloc"))
            __debugbreak();
    }
}

// ea: 0x66F230
TBankAlloc BankManager::Allocate(NumBanks num_banks)
{
    BitSet<64> alloc;
    BitSet<64> alloc2;
    alloc.mBits[1] = 0;
    alloc.mBits[0] = 0;
    alloc2.mBits[1] = 0;
    alloc2.mBits[0] = 0;
    float numBanks = num_banks.xbox;
    InternalAllocate(mNumMramBanks, &mFreeBanks.mram_alloc1,
                     &mFreeBanks.mram_alloc2, &alloc, &alloc2, &numBanks,
                     false);
    if (numBanks != 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 668;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("main ram banks exhausted!"))
            __debugbreak();
    }
    if (alloc.IsEmpty() && alloc2.IsEmpty())
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 699;
        AeAssert::gCurrentExpr = "!alloc.IsEmpty()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("bad alloc!"))
            __debugbreak();
    }
    m_last_alloc.mram_alloc1 = alloc;
    m_last_alloc.mram_alloc2 = alloc2;
    float freeCount = get_free_count().xbox;
    if (mLowestFreeAmount > freeCount)
        mLowestFreeAmount = freeCount;
    TBankAlloc result;
    result.mram_alloc1 = alloc;
    result.mram_alloc2 = alloc2;
    return result;
}

// ea: 0x66B2F0 (BankManager.cpp file-static)
static void InternalGetAllocs(float numAvailableBanks, float* numBanks,
                              unsigned int bankSize,
                              unsigned char* arena, BitSet<64>* alloc1,
                              BitSet<64>* alloc2,
                              ae_sized_array<mem_info, 32>* output)
{
    int v7 = 0;
    unsigned char* v8 = arena;
    while (*numBanks >= 1.0f)
    {
        if (numAvailableBanks <= (float)v7)
            break;
        if (alloc1->Test(v7) && alloc2->Test(v7))
        {
            alloc1->Rmv(v7);
            alloc2->Rmv(v7);
            *numBanks -= 1.0f;
            mem_info mi = mem_info(v8, (int)bankSize);
            output->push_back(mi);
        }
        ++v7;
        v8 += bankSize;
    }
    if (*numBanks > 0.0f)
    {
        int v9 = 0;
        float v16 = numAvailableBanks + 0.5f;
        if (v16 > 0.0f)
        {
            bool v10;
            for (;;)
            {
                v10 = alloc1->Test(v9);
                if (v10 != alloc2->Test(v9))
                    break;
                if (v16 <= (float)++v9)
                    goto scan_alloc1;
            }
            mem_info mi = mem_info(
                v10 ? &arena[bankSize * v9]
                    : &arena[(bankSize >> 1) + bankSize * v9],
                (int)(bankSize >> 1));
            (v10 ? alloc1 : alloc2)->Rmv(v9);
            output->push_back(mi);
            *numBanks -= 0.5f;
            return;
        }
scan_alloc1:
        {
            int v11 = 0;
            if (numAvailableBanks > 0.0f)
            {
                while (1)
                {
                    if (v11 >> 5 >= 2)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "../ae\\core/BitSet.h";
                        AeAssert::gCurrentLine = 123;
                        AeAssert::gCurrentExpr = "idx < GetNumWords()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(
                                   "Please add a descriptive string"))
                            __debugbreak();
                    }
                    if (((1 << (v11 & 0x1F))
                         & alloc1->mBits[v11 >> 5]) != 0)
                        break;
                    if (numAvailableBanks <= (float)++v11)
                        return;
                }
                alloc1->Rmv(v11);
                mem_info mi = mem_info(&arena[bankSize * v11],
                                       (int)(bankSize >> 1));
                output->push_back(mi);
                *numBanks -= 0.5f;
            }
        }
    }
}

// ea: 0x66B540
void BankManager::GetAllocs(TBankAlloc bat, const NumBanks& numBanks,
                            ae_sized_array<mem_info, 32>* output)
{
    float numBanksLocal = numBanks.xbox;
    InternalGetAllocs(mNumMramBanks, &numBanksLocal, mMramBankSize,
                      mMramArena, &bat.mram_alloc1, &bat.mram_alloc2,
                      output);
    if (!bat.IsEmpty())
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 564;
        AeAssert::gCurrentExpr = "bat.IsEmpty()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Banks left over!"))
            __debugbreak();
    }
}

// ea: 0x66B9A0
mem_info BankManager::get_alloc(const TBankAlloc& bat, int which,
                                bool mram) const
{
    (void)mram;
    if (which < 0 || which >= get_alloc_count(bat))
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 801;
        AeAssert::gCurrentExpr =
            "0 <= which && which < get_alloc_count(bat)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    int v6 = -1;
    int v7 = 0;
    int group_index = -1;
    unsigned char* arena = mMramArena;
    bool half = false;
    int index = -1;
    if (mNumMramBanks + 0.5f > 0.0f)
    {
        // full-bank groups (both alloc1+alloc2)
        while (!bat.mram_alloc1.Test(v7) || !bat.mram_alloc2.Test(v7)
               || ++v6 != which)
        {
            if (mNumMramBanks + 0.5f <= (float)++v7)
            {
                group_index = v6;
                goto half_search;
            }
        }
        index = v7;
    }
    else
    {
half_search:
        if (mNumMramBanks + 0.5f <= 0.0f)
            goto not_found;
        for (int v8 = 0;; ++v8)
        {
            bool v9 = bat.mram_alloc2.Test(v8);
            if (bat.mram_alloc1.Test(v8) != v9 && ++group_index == which)
            {
                half = true;
                index = v8;
                break;
            }
            if (mNumMramBanks + 0.5f <= (float)(v8 + 1))
            {
                v6 = group_index;
                goto not_found;
            }
        }
    }
    {
        unsigned int v10 = 0;
        if (half && bat.mram_alloc2.Test(index))
            v10 = mMramBankSize >> 1;
        unsigned char* v11 = &arena[v10 + (unsigned int)index * mMramBankSize];
        unsigned int v12 = mMramBankSize >> (half ? 1 : 0);
        if (v11 == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
            AeAssert::gCurrentLine = 919;
            AeAssert::gCurrentExpr = "mi.data != 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("no alloc?"))
                __debugbreak();
        }
        if ((((uintptr_t)v11 + 4095) & 0xFFFFF000) != (uintptr_t)v11)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
            AeAssert::gCurrentLine = 922;
            AeAssert::gCurrentExpr =
                "( (uint32(mi.data) + (4*1024-1)) & ~(4*1024-1)) == "
                "uint32(mi.data)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("bank is not aligned!"))
                __debugbreak();
        }
        mem_info result = mem_info(v11, (int)v12);
        return result;
    }
not_found:
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 912;
        AeAssert::gCurrentExpr = "found";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("bank alloc index out of range?"))
            __debugbreak();
        mem_info result = mem_info(nullptr, 0);
        return result;
    }
}

// ea: 0x4B43A0
void* BankManager::operator new(size_t /*size*/, void* p)
{
    return p;
}

// ea: 0x4B43B0
BankManager* BankManager::Inst()
{
    return sInst;
}

BankManager::BankManager()
{
    mFreeBanks.Clear();
    mNumMramBanks = 0.0f;
    mMramBankSize = 0;
    mMramArena = nullptr;
    m_last_alloc.Clear();

    char buf[16];
    sprintf(buf, "%f", 34.5);
    cvar_t* v2 = Cvar_Get("num_banks", buf, 16);
    int num_banks = 1107951616;  // 34.5f bits
    if (v2 != nullptr && v2->value != 0.0)
        num_banks = *(int*)&v2->value;
    float v3 = 160.0f;
    if (!ShouldConnectPaths())
        v3 = *(float*)&num_banks;
    mNumMramBanks = v3;
    mMramBankSize = 0xB0000;
    mFreeBanks.Clear();
    int v4 = 0;
    if (mNumMramBanks + 0.5f > 0.0f)
    {
        do
        {
            mFreeBanks.mram_alloc1.Add(v4);
            if (v4 != (int)mNumMramBanks)
                mFreeBanks.mram_alloc2.Add(v4);
            ++v4;
        } while ((int)(mNumMramBanks + 0.5f) > v4);
    }
    mMramArena = stream_alloc((int)(mMramBankSize * mNumMramBanks), false);
    g_bank_space_size = (int)(mMramBankSize * mNumMramBanks);
    g_bank_space = mMramArena;
    mLowestFreeAmount = get_free_count().xbox;
}

// ea: 0x66B1D0
BankManager::~BankManager()
{
    int v2 = 0;
    if ((mNumMramBanks + 0.5f) > 0.0f)
    {
        do
        {
            if (!mFreeBanks.mram_alloc1.Test(v2)
                || mFreeBanks.mram_alloc2.Test(v2))
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\BankManager.cpp";
                AeAssert::gCurrentLine = 408;
                AeAssert::gCurrentExpr = nullptr;
                if (AeAssert::Error("unreleased banks!"))
                    __debugbreak();
            }
            ++v2;
        } while ((mNumMramBanks + 0.5f) > v2);
    }
    unsigned char* mMramArena = this->mMramArena;
    if (PakFile::sHeaderBuffer == mMramArena)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BankManager.cpp";
        AeAssert::gCurrentLine = 133;
        AeAssert::gCurrentExpr = "PakFile::sHeaderBuffer != ptr";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("trying to free reserved buffer"))
            __debugbreak();
    }
    if (mMramArena != nullptr)
        mem_heap_free(mMramArena);
}

// ea: 0x66B850
void BankManager::release(TBankAlloc& alloc)
{
    int v2 = 0;
    if (mNumMramBanks + 0.5f > 0.0f)
    {
        BankManager* v5 = this;
        do
        {
            bool v4 = alloc.mram_alloc1.Test(v2);
            bool v7 = alloc.mram_alloc2.Test(v2);
            if (v4)
            {
                alloc.mram_alloc1.Rmv(v2);
                v5 = this;
                mFreeBanks.mram_alloc1.Add(v2);
            }
            else
            {
                v5 = this;
            }
            if (v7)
            {
                alloc.mram_alloc2.Rmv(v2);
                v5->mFreeBanks.mram_alloc2.Add(v2);
            }
            ++v2;
        } while (v5->mNumMramBanks + 0.5f > v2);
    }
}

// ea: 0x66E400
void PakFile::SetupAllocator()
{
    ae_sized_array<PoolAllocator::PoolConfig, 16> cfgList;
    memset(&cfgList, 0, sizeof(cfgList));
    cfgList.m_size = 0;
    PoolAllocator::PoolConfig elt;
    elt.blockSize = 276;
    elt.blockAlign = 4;
    elt.numBlocks = 12;
    elt.block = nullptr;
    cfgList.push_back(elt);
    void* block = mem_heap_malloc(0x3Cu);
    if (block != nullptr)
        PakFile::sAllocator =
            new (block) PoolAllocator(cfgList, 1u);
    else
        PakFile::sAllocator = nullptr;

    ae_sized_array<PoolAllocator::PoolConfig, 16> v7;
    memset(&v7, 0, sizeof(v7));
    v7.m_size = 0;
    elt.blockSize = 1180;
    elt.blockAlign = 4;
    elt.numBlocks = 16;
    elt.block = nullptr;
    v7.push_back(elt);
    void* result = mem_heap_malloc(0x3Cu);
    if (result != nullptr)
        gPakMemHeapAllocator = new (result) PoolAllocator(v7, 1u);
    else
        gPakMemHeapAllocator = nullptr;
}

// ea: 0x004C8EB0
// SetupPoolAllocator's common pool stage is required before Com_Init creates
// CurveManager and the other pool-backed systems.
void SetupPoolAllocator()
{
    ae_sized_array<PoolAllocator::PoolConfig, 16> cfgList;
    memset(&cfgList, 0, sizeof(cfgList));
    cfgList.m_size = 0;

    PoolAllocator::PoolConfig elt;
    elt.blockSize = 20;
    elt.blockAlign = 4;
    elt.numBlocks = 4096;
    elt.block = nullptr;
    cfgList.push_back(elt);

    elt.blockSize = 32;
    elt.blockAlign = 16;
    elt.numBlocks = 2048;
    elt.block = nullptr;
    cfgList.push_back(elt);

    elt.blockSize = 80;
    elt.blockAlign = 16;
    elt.numBlocks = 2048;
    elt.block = nullptr;
    cfgList.push_back(elt);

    elt.blockSize = 128;
    elt.blockAlign = 16;
    elt.numBlocks = 1024;
    elt.block = nullptr;
    cfgList.push_back(elt);

    elt.blockSize = 256;
    elt.blockAlign = 16;
    elt.numBlocks = 450;
    elt.block = nullptr;
    cfgList.push_back(elt);

    void* block = mem_heap_malloc(0x3Cu);
    PoolAllocator* common = nullptr;
    if (block != nullptr)
        common = new (block) PoolAllocator(cfgList, 2u);

    gCommonPoolAllocator = common;
    EntityNotify_SetupAllocators(common);
    CurveManager_SetupAllocator(common);
    PakFile::SetupAllocator();
}

// ea: 0x66AFF0
void PakFile::AddHeap(unsigned char* heap_start, unsigned int heap_size)
{
    int m_size = mHeapList.m_size;
    mem_heap* v4 =
        m_size != 0 ? mHeapList.m_elements[m_size - 1] : nullptr;
    mem_heap* Heap = CreateHeap(heap_start, heap_size);
    if (Heap != nullptr && v4 != nullptr)
    {
        Heap->reserve = v4->reserve;
        v4->reserve = Heap;
    }
}

// ============================================================================
// PakFile streaming internals (streamer.o PakFile.cpp)
// ============================================================================

// ea: 0x66A120
void PakFile::InitiateHeaderRead()
{
    if (mHeader != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 366;
        AeAssert::gCurrentExpr = "mHeader==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can't load a pakfile again!"))
            __debugbreak();
    }
    if (sHeaderBufferUsed)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 369;
        AeAssert::gCurrentExpr = "!sHeaderBufferUsed";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Loading 2 pakfiles at once?!"))
            __debugbreak();
    }
    sHeaderBufferUsed = true;
    mHeaderBuffer = sHeaderBuffer;
    unsigned char buffer[4];
    mFileId = nflOpenFileEx((nflMediaID)gNflMediaId, mPath.mBuff,
                            &mFilesize);
    if (mFileId == NFL_FILE_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 394;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("error opening file '%s'", mPath.mBuff))
            __debugbreak();
        mHeaderBuffer = nullptr;
        mState = LOADED;
        sHeaderBufferUsed = false;
        return;
    }
    if (mFilesize == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 400;
        AeAssert::gCurrentExpr = "mFilesize != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("incorrect filesize"))
            __debugbreak();
    }
    if (sHeaderBufferSize < 0x8000)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 403;
        AeAssert::gCurrentExpr =
            "sHeaderBufferSize > 0 && sHeaderBufferSize >= (32*1024)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("size check"))
            __debugbreak();
    }
    PakReadDataAsync(buffer, mHeaderBuffer, sHeaderBufferSize, 0, false);
    mHeaderRequestId.nflId = (nflRequestID)(*(unsigned int*)buffer);
    gThroughputMeasurer.mStart = gThroughputMeasurer.safeGetTime();
    gThroughputMeasurer.mBytes = 0;
    if (mHeaderRequestId.nflId == NFL_REQUEST_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 409;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("invalid request (requesting header)!"))
            __debugbreak();
    }
    mLoadingState = LOADING_HEADER;
}

// ea: 0x66E540
void PakFile::CancelLoading()
{
    nflUpdate();
    if (nflGetState() == NFL_STATE_ERROR)
        g_femanager.DrawDiscError();
    if (mFileId != NFL_FILE_ID_INVALID)
    {
        nflCancelFileRequests(mFileId);
        mHeaderRequestId.nflId = NFL_REQUEST_ID_INVALID;
    }
    PakHeader* mHeader = this->mHeader;
    if (mHeader != nullptr)
    {
        for (int v3 = 0; v3 < mHeader->sections[mDefaultSectionIdx].numBanks;
             ++v3)
        {
            PakHeader::Bank* v7 =
                &mHeader->sections[mDefaultSectionIdx].banks[v3];
            if (v7->requestId != NFL_REQUEST_ID_INVALID)
                v7->requestId = NFL_REQUEST_ID_INVALID;
        }
    }
    if (!mSerializedAlloc.IsEmpty())
        BankManager::sInst->release(mSerializedAlloc);
    if (!mSerializedAlloc.IsEmpty())
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 924;
        AeAssert::gCurrentExpr = "mSerializedAlloc.IsEmpty()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("bank manager should clear this bitset"))
            __debugbreak();
    }
    PakHeader::Section* v15 = &mHeader->sections[mDefaultSectionIdx];
    for (int bankIdx = 0; bankIdx < v15->numBanks; ++bankIdx)
    {
        PakHeader::Bank* v18 = &v15->banks[bankIdx];
        if ((v18->flags & 0x40000) != 0)
        {
            v18->flags &= ~0x40000u;
            apk::apkDeleteFile((apk::apkFile*)((char*)v18->memptr + 8));
        }
        if ((v18->flags & 0x20000) != 0)
        {
            unsigned char* memptr = v18->memptr;
            if (sHeaderBuffer == memptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\BankManager.cpp";
                AeAssert::gCurrentLine = 133;
                AeAssert::gCurrentExpr = "PakFile::sHeaderBuffer != ptr";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("trying to free reserved buffer"))
                    __debugbreak();
            }
            if (memptr != nullptr)
                mem_heap_free(memptr);
            v18->memptr = nullptr;
            v18->memsize = 0;
        }
    }
    CopyHeader();
    sHeaderBufferUsed = false;
    nflUpdate();
    if (nflGetState() == NFL_STATE_ERROR)
        g_femanager.DrawDiscError();
    if (mCloseHandle)
    {
        nflCloseFile(mFileId);
        mFileId = NFL_FILE_ID_INVALID;
    }
    mState = LOADED;
    mLoadingState = (ELoadingState)8;
}

// ea: 0x6746C0
void PakFile::AsyncLoad(NumBanks numBanks)
{
    tlPrintf("pak: loading '%s' (%.1f banks)\n", mPath.mBuff, numBanks.xbox);
    const int roundedBankCount = (int)(numBanks.xbox + 0.5f);
    if ((float)roundedBankCount > 0.0f)
    {
        mBankAlloc = BankManager::sInst->Allocate(numBanks);
        if (mBankAlloc.IsEmpty())
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
            AeAssert::gCurrentLine = 350;
            AeAssert::gCurrentExpr = "!mBankAlloc.IsEmpty()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("empty allocation from bank manager?!"))
                __debugbreak();
        }
    }
    mState = LOADING;
    mLoadingState = LOADING_HEADER;
    InitiateHeaderRead();
}

// ea: 0x6747D0
void PakFile::AsyncUnload()
{
    tlPrintf("pak: async unloading '%s'\n", mPath.mBuff);
    if (mState == LOADING)
        CancelLoading();
    mState = UNLOADING;
    mLoadingState = (ELoadingState)4;  // UNLOADING_SERIALIZED
}

// ea: 0x66E7C0
void PakFile::BeginAsyncUnload()
{
    if (mState == LOADING)
        CancelLoading();
    if (mState != LOADED)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 1558;
        AeAssert::gCurrentExpr = "mState == STATE_LOADED";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can't unload a pak that isn't fully loaded"))
            __debugbreak();
    }
    tlPrintf("pak: unloading '%s' with id '%d'\n", mPath.mBuff, mPakId);
    TPakId pakId = mPakId;
    mLoadingState = (ELoadingState)4;  // UNLOADING_SERIALIZED
    mState = UNLOADING;
    StreamZoneManager::sInst->OnUnload(pakId);
}

// ea: 0x66E860
void PakFile::UnloadSerialized()
{
    ae_sized_array<TPakId, 128>& ContextStack =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    ContextStack.push_back(mPakId);
    TPakId pakId = mPakId;
    AssetBankSet::UnloadBanks(pakId);
    g_femanager.UnloadBank(this->mPakId);
    FX_KillEffects(this->mPakId);
    StopAllSceneAnims(this->mPakId);
    void** mElements = mLooseFiles.mElements;
    void** i = &mElements[mLooseFiles.mSize];
    for (; mElements != i; ++mElements)
        mem_heap_free(*mElements);
    ae_sized_array<TPakId, 128>& v5 =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    if (v5.m_size != 0)
        v5.m_size -= 1;
}

ae_sized_array<ae_heap_base*, 32> gPakHeaps;  // ?gPakHeaps@@3V?$ae_sized_array@PAVae_heap_base@@$0CA@@@A

// ae_heap_wrapper bridge vtable (core.o funcs not yet ported; GetHeapPointer
// at slot 4 per the binary's ??_7ae_heap_wrapper@@6B@ @ 0xD0A290)
static void* __fastcall Wrapper_VecDtor(void* self, int flags)
{
    (void)flags;
    mem_heap_free(self);
    return self;
}
static void* __fastcall Wrapper_Malloc(void* self, unsigned int size,
                                       unsigned int align)
{
    return mem_heap_malloc(*(mem_heap**)((char*)self + 4), (int)align, size);
}
static void __fastcall Wrapper_Free(void* self, void* ptr)
{
    mem_heap_free(*(mem_heap**)((char*)self + 4), ptr);
}
static bool __fastcall Wrapper_CheckFree(void* self, void* ptr)
{
    // ae_heap_base::MemCheckFree semantics (0x7BBF40)
    mem_heap* heap = *(mem_heap**)((char*)self + 4);
    if (ptr < heap->start || ptr >= heap->end)
        return false;
    mem_heap_free(heap, ptr);
    return true;
}
static mem_heap* __fastcall Wrapper_GetHeapPointer(void* self)
{
    return *(mem_heap**)((char*)self + 4);
}
static void* s_ae_heap_wrapper_vftable[5] = {
    (void*)Wrapper_VecDtor,
    (void*)Wrapper_Malloc,
    (void*)Wrapper_Free,
    (void*)Wrapper_CheckFree,
    (void*)Wrapper_GetHeapPointer,
};

// ea: 0x684DD0
bool ae_heap_wrapper::CheckFree(void* ptr)
{
    if (ptr < mHeap->start || ptr >= mHeap->end)
        return false;
    mem_heap_free(mHeap, ptr);
    return true;
}

// ea: 0x66E930
void PakFile::CreateHeaps()
{
    if (mBankAlloc.IsEmpty())
        return;
    if (mDefaultSectionIdx == -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 1943;
        AeAssert::gCurrentExpr = "mDefaultSectionIdx != -1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("bad"))
            __debugbreak();
    }
    PakHeader::Section* sect = &mHeader->sections[mDefaultSectionIdx];
    mem_heap* last_heap = nullptr;
    for (int i = sect->numBanks - 1; i >= 0; --i)
    {
        PakHeader::Bank* bank = &sect->banks[i];
        unsigned int flags = bank->flags;
        unsigned int v11 = (bank->size + 0x7FFF) & 0xFFFF8000;
        if ((flags & 4) == 0 && (flags & 8) == 0)
        {
            unsigned char* memptr = bank->memptr;
            if (memptr != nullptr)
            {
                unsigned int memsize = bank->memsize;
                int v14 = (int)memsize - (int)v11;
                if (memsize != 0 && v14 > 0)
                {
                    mem_heap* Heap = CreateHeap(&memptr[v11], v14);
                    if (Heap != nullptr)
                    {
                        if (last_heap != nullptr)
                            last_heap->reserve = Heap;
                        last_heap = Heap;
                    }
                }
            }
        }
        if (mHeapList.m_size == 12)
            break;
    }

    const PakInfoNode* pakInfo = PakManager::sInst->GetPakInfo(mPakId);
    for (unsigned int v19 = 0; v19 < pakInfo->prereqs.mSize; ++v19)
    {
        const PakInfoNode* prereq = pakInfo->prereqs.mList[v19];
        TPakId id = prereq->pakId;
        PakFile* v22 =
            (id >= 0 && id <= 0x62) ? PakManager::sInst->mSlots[id] : nullptr;
        if (v22 != nullptr && v22->mHeapList.m_size > 0)
            mPrereqHeaps.push_back(v22);
    }

    if (mPakType != kPakTypeLevel)
        return;

    for (unsigned int k = 0; k < mHeapList.m_size; ++k)
    {
        void* v26 = mem_heap_malloc(8u);
        if (v26 != nullptr)
        {
            *(void***)v26 = s_ae_heap_wrapper_vftable;
            *(mem_heap**)((char*)v26 + 4) = mHeapList.m_elements[k];
        }
        if (gPakHeaps.m_size >= 32)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 174;
            AeAssert::gCurrentExpr = "m_size < _CAPACITY";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("no room left in array"))
                __debugbreak();
            continue;
        }
        gPakHeaps.m_elements[gPakHeaps.m_size] = (ae_heap_base*)v26;
        ++gPakHeaps.m_size;
    }

    for (unsigned int v30 = 0; v30 < pakInfo->prereqs.mSize; ++v30)
    {
        const PakInfoNode* prereq = pakInfo->prereqs.mList[v30];
        TPakId id = prereq->pakId;
        PakFile* v32 =
            (id >= 0 && id <= 0x62) ? PakManager::sInst->mSlots[id] : nullptr;
        for (unsigned int m = 0; m < v32->mHeapList.m_size; ++m)
        {
            void* v34 = mem_heap_malloc(8u);
            if (v34 != nullptr)
            {
                *(void***)v34 = s_ae_heap_wrapper_vftable;
                *(mem_heap**)((char*)v34 + 4) = v32->mHeapList.m_elements[m];
            }
            if (gPakHeaps.m_size >= 32)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 174;
                AeAssert::gCurrentExpr = "m_size < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("no room left in array"))
                    __debugbreak();
                continue;
            }
            gPakHeaps.m_elements[gPakHeaps.m_size] = (ae_heap_base*)v34;
            ++gPakHeaps.m_size;
        }
    }
}

// ea: 0x674810
void PakFile::InitiateDataLoad()
{
    ae_sized_array<mem_info, 32> bank_allocations;
    memset(&bank_allocations, 0, sizeof(bank_allocations));
    bank_allocations.m_size = 0;
    mDefaultSectionIdx = -1;
    for (unsigned int v7 = 0; v7 < mHeader->numSections; ++v7)
    {
        if (mHeader->sections[v7].name[0] == 0)
            mDefaultSectionIdx = (int)v7;
    }
    if (mDefaultSectionIdx == -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 468;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("pakfile with no default section?"))
            __debugbreak();
        return;
    }
    PakHeader::Section* sect = &mHeader->sections[mDefaultSectionIdx];

    if (!mBankAlloc.IsEmpty())
    {
        const PakInfoNode* mPakInfo =
            mPakType == kPakTypeCount
                ? nullptr
                : (this->mPakInfo != nullptr
                       ? this->mPakInfo
                       : (this->mPakInfo =
                              PakManager::sInst->GetPakInfo(mPakId)));
        BankManager::sInst->GetAllocs(mBankAlloc, mPakInfo->numBanks,
                                      &bank_allocations);

        unsigned int last = BankManager::sInst->mMramBankSize;
        int numBanks = sect->numBanks;
        NumBanks needed;
        needed.xbox = 0.0f;
        for (int i = 0; i < numBanks; ++i)
        {
            unsigned int flags = sect->banks[i].flags;
            if ((flags & 4) != 0)
                continue;  // serialized bank does not use mram
            bool half = (i == numBanks - 1
                         || ((flags ^ sect->banks[i + 1].flags) & 0x1A) != 0)
                        && sect->banks[i].size <= (last >> 1);
            if ((flags & 2) != 0)
                needed.xbox += half ? 0.5f : 1.0f;
        }

        if (needed.xbox > mPakInfo->numBanks.xbox)
        {
            BankManager::sInst->release(mBankAlloc);
            const_cast<PakInfoNode*>(mPakInfo)->numBanks.xbox = needed.xbox;
            NumBanks v74 = mPakInfo->numBanks;
            if (v74.xbox > BankManager::sInst->get_free_count().xbox)
            {
                tlPrintf("pak: cannot allocate pak's actual needed %1.1f "
                         "banks; aborting load\n",
                         mPakInfo->numBanks.xbox);
                CancelLoading();
                return;
            }
            mBankAlloc = BankManager::sInst->Allocate(mPakInfo->numBanks);
            bank_allocations.m_size = 0;
            BankManager::sInst->GetAllocs(mBankAlloc, mPakInfo->numBanks,
                                          &bank_allocations);
            tlPrintf("pak: Re-allocated to %1.1f banks\n",
                     mPakInfo->numBanks.xbox);
        }

        char buf[1024];
        int last2 = (int)(BankManager::sInst->mNumMramBanks + 0.5f);
        for (int v34 = 0; v34 < last2; ++v34)
        {
            char v36 = '.';
            if (mBankAlloc.mram_alloc1.Test(v34)
                && mBankAlloc.mram_alloc2.Test(v34))
                v36 = 'X';
            else if (mBankAlloc.mram_alloc1.Test(v34))
                v36 = '/';
            else if (mBankAlloc.mram_alloc2.Test(v34))
                v36 = '\\';
            buf[v34] = v36;
        }
        buf[last2] = 0;
        tlPrintf("pak: have alloc [%s] from BankManager\n", buf);

        for (int j = 0; j < bank_allocations.m_size; ++j)
        {
            if (j >= 0x20)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            tlPrintf("[%X]", bank_allocations.m_elements[j].data);
            if (bank_allocations.m_elements[j].data == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 619;
                AeAssert::gCurrentExpr = nullptr;
                if (AeAssert::Error("data is null!"))
                    __debugbreak();
            }
            if (bank_allocations.m_elements[j].size == 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 621;
                AeAssert::gCurrentExpr = nullptr;
                if (AeAssert::Error("size is 0!"))
                    __debugbreak();
            }
        }
        tlPrintf("\n");
    }

    int count = 0;
    for (int i = 0; i < sect->numBanks; ++i)
    {
        PakHeader::Bank* v43 = &sect->banks[i];
        bool is_serialized = (v43->flags & 4) != 0;
        bool is_aram = (v43->flags & 8) != 0;
        if (is_aram)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
            AeAssert::gCurrentLine = 635;
            AeAssert::gCurrentExpr = "!is_aram";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Aram bank found on non-gc pakfile!"))
                __debugbreak();
        }
        unsigned char* v53 = nullptr;
        unsigned int v52 = 0;
        bool has_bank_alloc = !mBankAlloc.IsEmpty();
        if (!is_serialized)
        {
            if (count < bank_allocations.m_size)
            {
                unsigned char* v46 = bank_allocations.m_elements[count].data;
                v43->memptr = v46;
                if (v46 == nullptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                    AeAssert::gCurrentLine = 642;
                    AeAssert::gCurrentExpr = "bk.memptr != 0";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Memory NOT Allocated"))
                        __debugbreak();
                }
                unsigned int size = bank_allocations.m_elements[count].size;
                v43->memsize = size;
                if (size < v43->size)
                {
                    AeAssert::gCurrentAuthor = AeAssert::ARO;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                    AeAssert::gCurrentLine = 644;
                    AeAssert::gCurrentExpr = "bk.memsize >= bk.size";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("bank is too small!"))
                        __debugbreak();
                }
                count = count + 1;
            }

            // IDA: when the pak owns a bank allocation, nonserialized banks
            // retain the BankManager memory and skip the fallback path.
            if (has_bank_alloc)
            {
                v43->requestId = NFL_REQUEST_ID_INVALID;
                continue;
            }
        }

        if (is_aram)
        {
            v52 = (v43->size + 0x7FFF) & 0xFFFF8000;
            v53 = stream_alloc((int)v52, true);
        }
        else
        {
            if (is_serialized
                && v43->size < BankManager::sInst->mMramBankSize)
            {
                NumBanks one;
                one.ps2 = 1.0f;
                one.ps3.main = 1.0f;
                one.ps3.lram = 0.0f;
                one.xbox = 1.0f;
                one.xenon = 1.0f;
                one.pcx = 1.0f;
                one.gc.main = 1.0f;
                one.gc.aram = 0.0f;
                mSerializedAlloc = BankManager::sInst->Allocate(one);
                if (mSerializedAlloc.IsEmpty())
                {
                    AeAssert::gCurrentAuthor = AeAssert::ARO;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\PakFile.cpp";
                    AeAssert::gCurrentLine = 675;
                    AeAssert::gCurrentExpr =
                        "!mSerializedAlloc.IsEmpty()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(
                               "Couldn't allocate serialized bank!"))
                        __debugbreak();
                }
                mem_info ser =
                    BankManager::sInst->get_alloc(mSerializedAlloc, 0, true);
                v53 = ser.data;
                v52 = (unsigned int)ser.size;
            }
            else if (is_serialized)
            {
                tlPrintf("serialized assets too big for bank!");
            }
            if (v53 == nullptr)
            {
                v52 = (v43->size + 0x7FFF) & 0xFFFF8000;
                v53 = stream_alloc((int)v52, false);
                v43->flags |= PAK_BANK_FLAG_MEM;
            }
        }
        v43->memptr = v53;
        v43->memsize = v52;
        if (v52 != 0 && v53 == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
            AeAssert::gCurrentLine = 703;
            AeAssert::gCurrentExpr = "bk.memsize == 0 || bk.memptr != 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Memory NOT Allocated"))
                __debugbreak();
        }
        v43->requestId = NFL_REQUEST_ID_INVALID;
    }

    if (sect->numBanks == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 720;
        AeAssert::gCurrentExpr = "sect.numBanks > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("no banks???"))
            __debugbreak();
    }

    unsigned int v66 = (unsigned int)count;
    for (; v66 < bank_allocations.m_size; ++v66)
    {
        if (v66 >= 0x20)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        if (bank_allocations.m_elements[v66].data != nullptr)
        {
            int m_size = mHeapList.m_size;
            mem_heap* v69 =
                m_size != 0 ? mHeapList.m_elements[m_size - 1] : nullptr;
            mem_heap* Heap = CreateHeap(
                bank_allocations.m_elements[v66].data,
                bank_allocations.m_elements[v66].size);
            if (Heap != nullptr && v69 != nullptr)
            {
                Heap->reserve = v69->reserve;
                v69->reserve = Heap;
            }
        }
    }
    mLoadingState = LOADING_DATA;
    UpdateReads();
    mCurrentFile = 0;
    DetermineNextFile();
    CreateHeaps();
}

// ea: 0x66A790
bool PakFile::IsNextFileReady()
{
    PakHeader::File* v2 =
        &mHeader->sections[mDefaultSectionIdx].files[mCurrentFile];
    UpdateReads();
    int bankIndex = v2->bankIndex;
    if (bankIndex < 0
        || bankIndex >= mHeader->sections[mDefaultSectionIdx].numBanks)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 1315;
        AeAssert::gCurrentExpr =
            "bankIdx >= 0 && bankIdx < "
            "mHeader->sections[mDefaultSectionIdx].numBanks";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("bank idx out of bounds!"))
            __debugbreak();
    }
    PakHeader::Bank* v7 =
        &mHeader->sections[mDefaultSectionIdx].banks[bankIndex];
    if ((v7->flags & PAK_BANK_FLAG_READ_DONE) != 0)
        return true;
    TRequestId requestId;
    requestId.nflId = (nflRequestID)v7->requestId;
    if (requestId.nflId != NFL_REQUEST_ID_INVALID)
    {
        if (IsRequestDone(&requestId))
            return true;
        if ((v7->flags & PAK_BANK_FLAG_APK_HEADER) == 0)
        {
            if (v2->fileSize + (unsigned int)mCurrentFilePtr
                    - (unsigned int)v7->memptr
                > v7->size)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 1339;
                AeAssert::gCurrentExpr =
                    "needed_offset <= uint32(bank.size)";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("sanity check"))
                    __debugbreak();
            }
            nflRequestInfo request_info;
            nflGetRequestInfo(requestId.nflId, &request_info);
            gThroughputMeasurer.Update((int)request_info.bytesCompleted);
        }
    }
    return false;
}

// ea: 0x66EE30
void PakFile::DestroyHeaps()
{
    int i = 0;
    if (mHeapList.m_size > 0)
    {
        while (1)
        {
            mem_heap* v3 = ae_array_get(mHeapList, i);
            unsigned int v4 = 0;
            if (gPakHeaps.m_size > 0)
            {
                while (1)
                {
                    if (v4 >= 0x20)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                        AeAssert::gCurrentLine = 154;
                        AeAssert::gCurrentExpr =
                            "idx >= 0 && idx < _CAPACITY";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("out of bounds"))
                            __debugbreak();
                    }
                    ae_heap_wrapper* w =
                        (ae_heap_wrapper*)gPakHeaps.m_elements[v4];
                    void** wvt = *(void***)w;
                    mem_heap* hp =
                        ((mem_heap* (__fastcall*)(void*))wvt[4])(w);
                    if (hp == v3)
                        break;
                    if (++v4 >= (unsigned int)gPakHeaps.m_size)
                        goto heap_not_registered;
                }
                int m_size = gPakHeaps.m_size;
                ae_heap_wrapper* v6 =
                    (ae_heap_wrapper*)gPakHeaps.m_elements[v4];
                if (gPakHeaps.m_size > 1
                    && v4 < (unsigned int)gPakHeaps.m_size)
                {
                    gPakHeaps.m_elements[v4] =
                        gPakHeaps.m_elements[gPakHeaps.m_size - 1];
                    m_size = gPakHeaps.m_size;
                }
                if (m_size != 0)
                    gPakHeaps.m_size = m_size - 1;
                if (v6 != nullptr)
                {
                    // scalar-deleting dtor through vtable (ae_heap_base)
                    void** v6vt = *(void***)v6;
                    ((void* (__fastcall*)(void*, int))v6vt[0])(v6, 1);
                }
            }
heap_not_registered:
            if (v3->used_byte != 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                AeAssert::gCurrentLine = 2144;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning(
                           "Pakfile heap has unreleased allocations (%d bytes)",
                           v3->used_byte))
                    __debugbreak();
            }
            tlPrintf("pak: heap is destroyed at %X end %X size %i\n",
                     v3->start, v3->end, v3->size);
            mem_heap_destroy(v3);
            gPakMemHeapAllocator->Release(v3);
            if (++i >= mHeapList.m_size)
                break;
        }
    }
    mHeapList.m_size = 0;
}

// ea: 0x675580
void PakFile::UnloadInplace()
{
    ae_sized_array<TPakId, 128>& ContextStack =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    ContextStack.push_back(mPakId);
    ReleaseApks();
    InstanceBankMgr::sInst->ReleaseAnims(mPakId);
    InstanceBankMgr::sInst->ReleaseSkeletons(mPakId);
    InstanceBankMgr::sInst->mEntries[mPakId] = nullptr;
    ae_sized_array<TPakId, 128>& v3 =
        (ae_sized_array<TPakId, 128>&)PakManager::sInst->GetContextStack();
    if (v3.m_size != 0)
        v3.m_size -= 1;
}

// ea: 0x675630
void PakFile::FinishUnload()
{
    if (mPakType == kPakTypeGlobal)
        PakManager::sInst->mPakInfoBank = nullptr;
    const PakInfoNode* mPakInfo = this->mPakInfo;
    if (mPakInfo != nullptr)
    {
        BitSet<99>* prereqPakIds = mPakInfo->prereqPakIds;
        if (prereqPakIds != nullptr)
        {
            PakManager::sInst->MemFree(mPakId, prereqPakIds, false);
            const_cast<PakInfoNode*>(this->mPakInfo)->prereqPakIds = nullptr;
        }
    }

    int v2 = 0;
    for (int sectionIdx = 0; sectionIdx < mHeader->numSections; ++sectionIdx)
    {
        PakHeader::Section* v5 = &mHeader->sections[sectionIdx];
        if (v5->numBanks != 0)
        {
            for (int v6 = 0; v2 < v5->numBanks; ++v2, ++v6)
            {
                PakHeader::Bank* v9 = &v5->banks[v6];
                if ((v9->flags & PAK_BANK_FLAG_APK_LOADED) != 0)
                {
                    v9->flags &= ~PAK_BANK_FLAG_APK_LOADED;
                    apk::apkDeleteFile(
                        (apk::apkFile*)((char*)v9->memptr + 8));
                }
            }
            v2 = 0;
        }
    }
    DestroyHeaps();
    for (int sectionIdxa = 0; sectionIdxa < mHeader->numSections;
         ++sectionIdxa)
    {
        PakHeader::Section* v11 = &mHeader->sections[sectionIdxa];
        for (int bankIdx = 0; bankIdx < v11->numBanks; ++bankIdx)
        {
            PakHeader::Bank* v14 = &v11->banks[bankIdx];
            if ((v14->flags & PAK_BANK_FLAG_MEM) != 0)
            {
                unsigned char* memptr = v14->memptr;
                if (sHeaderBuffer == memptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\BankManager.cpp";
                    AeAssert::gCurrentLine = 133;
                    AeAssert::gCurrentExpr = "PakFile::sHeaderBuffer != ptr";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("trying to free reserved buffer"))
                        __debugbreak();
                }
                if (memptr != nullptr)
                    mem_heap_free(memptr);
                v14->memptr = nullptr;
            }
        }
    }
    if (mBankAlloc.IsEmpty())
    {
        mState = UNLOADED;
        return;
    }
    BankManager::sInst->release(mBankAlloc);
    mState = UNLOADED;
}

// ea: 0x66C010
void InstanceBankMgr::ReleaseAnims(TPakId pakId)
{
    m_animfile_directory.m_enable_release = true;
    m_anim_directory.m_enable_release = true;
    m_scnanim_directory.m_enable_release = true;
    InstanceBankSet* entry = mEntries[pakId];
    if (entry != nullptr)
    {
        InstanceBank& meshBank = entry->GetBank(INSTBANK_TYPE_MESH);
        for (unsigned int i = 0; i < meshBank.mEntries.mSize; ++i)
        {
            InstanceBank::IbEntry& ibe = meshBank.mEntries.mList[i];
            nalAnimFile* ptr = (nalAnimFile*)ibe.ptr;
            if (ptr != nullptr)
            {
                // minimal nalAnimFile/nalAnimClass views (NextAnim +0x04,
                // FirstAnim +0x34) - full types in animation/nal.cpp
                struct NalAnimFileView {
                    uint8_t _pad[0x34];
                    void* FirstAnim;  // +0x34
                };
                struct NalAnimClassView {
                    void* vftable;    // +0x00
                    void* NextAnim;   // +0x04
                };
                for (void* j = ((NalAnimFileView*)ptr)->FirstAnim;
                     j != nullptr;
                     j = ((NalAnimClassView*)j)->NextAnim)
                {
                    void* old_dir = m_anim_directory.m_old_directory;
                    void** odvt = *(void***)old_dir;
                    ((void (__thiscall*)(void*, void*))odvt[4])(old_dir, j);
                    XAnimRelease((nalAnimClass<nalAnyPose>*)j);
                }
                nalReleaseAnimFile(ptr);
                ibe.ptr = 0;
            }
        }
        InstanceBank& animfileBank = entry->GetBank(INSTBANK_TYPE_ANIMFILE);
        for (unsigned int k = 0; k < animfileBank.mEntries.mSize; ++k)
            animfileBank.mEntries.mList[k].ptr = 0;

        InstanceBank& animBank = entry->mInstanceBanks.mList[6];
        for (unsigned int v17 = 0; v17 < animBank.mEntries.mSize; ++v17)
        {
            InstanceBank::IbEntry& ibe = animBank.mEntries.mList[v17];
            if (ibe.ptr != 0)
            {
                nalReleaseSceneAnim((nalSceneAnim*)ibe.ptr);
                ibe.ptr = 0;
            }
        }
        InstanceBank& scnBank = entry->mInstanceBanks.mList[7];
        for (unsigned int v26 = 0; v26 < scnBank.mEntries.mSize; ++v26)
        {
            InstanceBank::IbEntry& ibe = scnBank.mEntries.mList[v26];
            if (ibe.ptr != 0)
            {
                PakManager::sInst->MemFree(pakId, (void*)ibe.ptr, false);
                ibe.ptr = 0;
            }
        }
    }
    m_animfile_directory.m_enable_release = false;
    m_anim_directory.m_enable_release = false;
    m_scnanim_directory.m_enable_release = false;
}

// ea: 0x66A380
void PakFile::UpdateReads()
{
    if (mDefaultSectionIdx == -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
        AeAssert::gCurrentLine = 782;
        AeAssert::gCurrentExpr = "mDefaultSectionIdx != -1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UpdateReads with no default section"))
            __debugbreak();
    }
    nflUpdate();
    if (nflGetState() == NFL_STATE_ERROR)
        g_femanager.DrawDiscError();

    PakHeader::Section& sec = mHeader->sections[mDefaultSectionIdx];
    int num_streams = 0;
    unsigned char buffer[8];

    for (int bankIdx = 0; bankIdx < sec.numBanks; ++bankIdx)
    {
        PakHeader::Bank* bank = &sec.banks[bankIdx];
        if (bank->requestId == NFL_REQUEST_ID_INVALID)
            continue;
        if (!IsRequestDone((TRequestId*)&bank->requestId))
        {
            ++num_streams;
            continue;
        }
        bank->requestId = NFL_REQUEST_ID_INVALID;
        bank->flags |= PAK_BANK_FLAG_READ_DONE;
        if (bankIdx == sec.numBanks - 1)
        {
            unsigned __int64 now = __rdtsc();
            mLoadStats->readTotal =
                (float)((double)(now - mLoadStats->readStart)
                        * 0.000000001363636402376034);
        }
        if ((bank->flags & PAK_BANK_FLAG_APK_HEADER) != 0)
        {
            apk::apkLoadFileInPlace(bank->memptr, false);
            bank->flags |= PAK_BANK_FLAG_APK_LOADED;
            DetermineNextFile();
        }
        unsigned int v5 = (bank->size + 0x7FFF) & 0xFFFF8000;
        gThroughputMeasurer.Update((int)v5);
        float v6 = gThroughputMeasurer.safeGetTime()
                   - gThroughputMeasurer.mStart
                   + gThroughputMeasurer.mTotalTime;
        gThroughputMeasurer.mStart = 0.0f;
        gThroughputMeasurer.mTotalTime = v6;
        gThroughputMeasurer.mTotalBytes += (int)v5;
        gThroughputMeasurer.mBytes = 0;
    }

    if (num_streams >= 1)
        return;

    int v8 = 0;
    int v23 = 0;
    if (sec.numBanks == 0)
        return;
    for (;;)
    {
        PakHeader::Bank* bank = &sec.banks[v8];
        if (bank->requestId == NFL_REQUEST_ID_INVALID
            && (bank->flags & PAK_BANK_FLAG_READ_DONE) == 0)
        {
            unsigned int size = bank->size;
            if (size != 0)
            {
                unsigned int memsize = bank->memsize;
                unsigned int v15 = (size + 0x7FFF) & 0xFFFF8000;
                if (v15 > memsize)
                {
                    const PakInfoNode* mPakInfo;
                    if (mPakType == kPakTypeCount)
                    {
                        mPakInfo = nullptr;
                    }
                    else
                    {
                        if (mPakInfo == nullptr)
                            mPakInfo = PakManager::sInst->GetPakInfo(mPakId);
                    }
                    AeAssert::gCurrentAuthor = AeAssert::ARO;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                    AeAssert::gCurrentLine = 837;
                    AeAssert::gCurrentExpr = nullptr;
                    if (AeAssert::Error(
                            "%s's banks don't match the global pak's banks\n"
                            "Try rebuilding global pak",
                            mPakInfo->longName.mStr))
                        __debugbreak();
                }
                if (v23 == 0)
                    mLoadStats->readStart = __rdtsc();
                unsigned int compressedSize = bank->compressedSize;
                if (compressedSize != 0)
                {
                    bank->requestId = *(unsigned int*)PakReadDataAsync(
                            buffer, bank->memptr, bank->size,
                            compressedSize, bank->fileOffset);
                }
                else
                {
                    bank->requestId = *(unsigned int*)PakReadDataAsync(
                            &buffer[4], bank->memptr, v15, 0,
                            bank->fileOffset);
                }
                gThroughputMeasurer.mStart =
                    gThroughputMeasurer.safeGetTime();
                gThroughputMeasurer.mBytes = 0;
                if (bank->requestId == NFL_REQUEST_ID_INVALID)
                {
                    AeAssert::gCurrentAuthor = AeAssert::ARO;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakFile.cpp";
                    AeAssert::gCurrentLine = 852;
                    AeAssert::gCurrentExpr = nullptr;
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Warning(
                               "invalid request (requesting data)!"))
                        __debugbreak();
                }
                ++num_streams;
            }
        }
        if (num_streams == 1)
            break;
        v8 = ++v23;
        if (v23 >= sec.numBanks)
            break;
    }
}

// ea: 0x675410
void PakFile::AddApk(apk::apkFile* apk)
{
    PakHeapContext heap_ctx(mPakId, false);
    mApkFiles.push_back(apk);
}

// ea: 0x6754A0
void PakFile::ReleaseApks()
{
    PakHeapContext heap_ctx(mPakId, false);
    apk::apkFile** mElements = mApkFiles.mElements;
    apk::apkFile** v3 = &mElements[mApkFiles.mSize];
    while (mElements != v3)
        apk::apkDeleteFile(*mElements++);
    if (mApkFiles.mElements != nullptr)
    {
        tlMemFree(mApkFiles.mElements);
        mApkFiles.mElements = nullptr;
        mApkFiles.mCapacity = 0;
    }
}

// ea: 0x6652B0 / 0x665320 / 0x665330 / 0x665340 (empty no-ops)
void DecodeGrassInfo(const char* name, unsigned char* data, unsigned int size,
                     TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeSPT(const char* name, unsigned char* data, unsigned int size, TPakId pakId,
               PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeWIND(const char* name, unsigned char* data, unsigned int size, TPakId pakId,
                PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeSEED(const char* name, unsigned char* data, unsigned int size, TPakId pakId,
                PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}

// ea: 0x6663D0
void DecodeCharSkel(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak)
{
    (void)name; (void)size; (void)pakId; (void)pak;
    cdLoadCharSkelInplace(data);
}

// ea: 0x66BD40
void DecodeMipSettings(const char* name, unsigned char* data, unsigned int size,
                       TPakId pakId, PakFile* pak)
{
    (void)pak;
    InstanceBankMgr::sInst->DecodeMipsettings(name, data, size, pakId);
}

// ea: 0x66BD60
void DecodeInstanceBank(const char* name, unsigned char* data, unsigned int size,
                        TPakId pakId, PakFile* pak)
{
    (void)pak;
    InstanceBankMgr::sInst->DecodeInstbank(name, data, size, pakId);
}

// ea: 0x665210
void DecodeDB(const char* name, unsigned char* data, unsigned int size, TPakId pakId,
              PakFile* pak)
{
    (void)pak;
    DbTablesetMgr::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x665230
void DecodeXModelBank(const char* name, unsigned char* data, unsigned int size,
                      TPakId pakId, PakFile* pak)
{
    (void)pak;
    XModelManager::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x665250
void DecodeXModelPartsBank(const char* name, unsigned char* data, unsigned int size,
                           TPakId pakId, PakFile* pak)
{
    (void)pak;
    XModelPartsManager::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x665270
void DecodeAITypeBank(const char* name, unsigned char* data, unsigned int size,
                      TPakId pakId, PakFile* pak)
{
    (void)pak;
    AITypeManager::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x665290
void DecodeLightGrid(const char* name, unsigned char* data, unsigned int size,
                     TPakId pakId, PakFile* pak)
{
    (void)pak;
    LightGridMgr::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x6652C0
void DecodeLevelPath(const char* name, unsigned char* data, unsigned int size,
                     TPakId pakId, PakFile* pak)
{
    (void)pak;
    PathNodeMgr::sInst->DecodeLevelBank(name, data, size, pakId);
}

// ea: 0x6652E0
void DecodeZonePath(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak)
{
    (void)pak;
    PathNodeMgr::sInst->DecodeZoneBank(name, data, size, pakId);
}

// ea: 0x665300
void DecodeSTB(const char* name, unsigned char* data, unsigned int size, TPakId pakId,
               PakFile* pak)
{
    (void)pak;
    STBManager::sInst->DecodeBank(name, data, size, pakId);
}

// Additional decoders (DecodePakFile.cpp; stubs until the decode impls land)
void DecodeGDB(const char* name, unsigned char* data, unsigned int size, TPakId pakId,
               PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeBSP(const char* name, unsigned char* data, unsigned int size, TPakId pakId,
               PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeCGBank(const char* name, unsigned char* data, unsigned int size,
                  TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeDestructible(const char* name, unsigned char* data, unsigned int size,
                        TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeDCGBank(const char* name, unsigned char* data, unsigned int size,
                   TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeDialogueBank(const char* name, unsigned char* data, unsigned int size,
                        TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeFLI(const char* name, unsigned char* data, unsigned int size, TPakId pakId,
               PakFile* pak)
{
    (void)pak;
    PakManager::sInst->DecodeFLI(name, (PakInfoBank*)data, (int)size, pakId);
}
void DecodeScene(const char* name, unsigned char* data, unsigned int size,
                 TPakId pakId, PakFile* pak)
{
    (void)pak;
    DecodeScene(name, data, (int)size, pakId);
}
void DecodePhysData(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeSplineGroup(const char* name, unsigned char* data, unsigned int size,
                       TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeBin(const char* name, unsigned char* data, unsigned int size, TPakId pakId,
               PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeZoneBoundaryBank(const char* name, unsigned char* data, unsigned int size,
                            TPakId pakId, PakFile* pak)
{
    (void)pak;
    DecodeZoneBoundaryBank(name, data, (int)size, pakId);
}
void DecodeAnimBank(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeAnimMatrix(const char* name, unsigned char* data, unsigned int size,
                      TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeFont(const char* name, unsigned char* data, unsigned int size,
                TPakId pakId, PakFile* pak)
{
    // ea: 0x677840
    cdLoadApkInplace(name, data, (int)size, pakId, pak);
}
// ea: 0x66F590
void DecodeAnim(const char* name, unsigned char* data, unsigned int size,
                TPakId pakId, PakFile* pak)
{
    (void)size; (void)pak;
    PakHeapContext heap_ctx(pakId, false);
    XAnimEntryInvalidate();
    tlFixedString nm = GetName(name);
    if (nalLoadAnimFileInPlace(nm, data) == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DecodePakFile.cpp";
        AeAssert::gCurrentLine = 147;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("pak: failed loading anim %s", name))
            __debugbreak();
    }
    if (heap_ctx.mPakId != PAK_ID_INVALID)
    {
        TlSystemCallbacks::LockTlAllocsToPakHeap(heap_ctx.mLastState, false);
        ae_sized_array<TPakId, 128>& ContextStack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst
                ->GetContextStack();
        if (ContextStack.m_size != 0)
            ContextStack.m_size = ContextStack.m_size - 1;
    }
}
// ea: 0x66F4C0
void DecodeSkeleton(const char* name, unsigned char* data, unsigned int size,
                    TPakId pakId, PakFile* pak)
{
    (void)size; (void)pak;
    PakHeapContext heap_ctx(pakId, false);
    if (nalLoadSkeletonInPlace(reinterpret_cast<nalBaseSkeleton*>(data)) == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DecodePakFile.cpp";
        AeAssert::gCurrentLine = 129;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("pak: failed loading skeleton %s", name))
            __debugbreak();
    }
    if (heap_ctx.mPakId != PAK_ID_INVALID)
    {
        TlSystemCallbacks::LockTlAllocsToPakHeap(heap_ctx.mLastState, false);
        ae_sized_array<TPakId, 128>& ContextStack =
            (ae_sized_array<TPakId, 128>&)PakManager::sInst
                ->GetContextStack();
        if (ContextStack.m_size != 0)
            ContextStack.m_size = ContextStack.m_size - 1;
    }
}
void DecodeConfigStrings(const char* name, unsigned char* data, unsigned int size,
                         TPakId pakId, PakFile* pak)
{
    (void)name; (void)data; (void)size; (void)pakId; (void)pak;
}
void DecodeTexture(const char* name, unsigned char* data, unsigned int size,
                   TPakId pakId, PakFile* pak)
{
    // ea: 0x677830
    cdLoadApkInplace(name, data, (int)size, pakId, pak);
}

// ea: 0x665350
void InstanceBankMgr::ReleaseInstanceBank(TPakId pakId)
{
    if (pakId >= 0 && pakId < 99)
        mEntries[pakId] = nullptr;
}

// ea: 0x66BDE0
InstanceBankMgr::InstanceBankMgr()
{
    m_anim_directory.m_enable_release = false;
    m_anim_directory.m_release_once = false;
    m_anim_directory.m_enable_add = true;
    m_anim_directory.m_old_directory = nalAnimDirectory;
    nalAnimDirectory = (tlResourceDirectory<nalAnimClass<nalAnyPose>>*)
                           &m_anim_directory;
    m_scnanim_directory.m_enable_release = false;
    m_scnanim_directory.m_enable_add = true;
    m_scnanim_directory.m_release_once = false;
    m_scnanim_directory.m_old_directory = nalSceneAnimDirectory;
    nalSceneAnimDirectory =
        (tlResourceDirectory<nalSceneAnim>*)&m_scnanim_directory;
    m_animfile_directory.m_enable_release = false;
    m_animfile_directory.m_enable_add = true;
    m_animfile_directory.m_release_once = false;
    m_animfile_directory.m_old_directory = nalAnimFileDirectory;
    nalAnimFileDirectory =
        (tlResourceDirectory<nalAnimFile>*)&m_animfile_directory;
    m_skeleton_directory.m_enable_release = false;
    m_skeleton_directory.m_enable_add = true;
    m_skeleton_directory.m_release_once = false;
    m_skeleton_directory.m_old_directory = nalSkeletonDirectory;
    nalSkeletonDirectory =
        (tlResourceDirectory<nalBaseSkeleton>*)&m_skeleton_directory;
    mMipSettingsBank = nullptr;
    memset(mEntries, 0, sizeof(mEntries));
}

// ea: 0x66BE80
InstanceBankMgr::~InstanceBankMgr()
{
    nalSkeletonDirectory = (tlResourceDirectory<nalBaseSkeleton>*)
                               m_skeleton_directory.m_old_directory;
    nalAnimFileDirectory =
        (tlResourceDirectory<nalAnimFile>*)m_animfile_directory
            .m_old_directory;
    nalSceneAnimDirectory = (tlResourceDirectory<nalSceneAnim>*)
                                m_scnanim_directory.m_old_directory;
    nalAnimDirectory = (tlResourceDirectory<nalAnimClass<nalAnyPose>>*)
                           m_anim_directory.m_old_directory;
}

// ea: 0x684BE0
void InstanceBankSet::Fixup()
{
    if (mId != 1229537875)  // FourCC('IIBS')
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBank.h";
        AeAssert::gCurrentLine = 229;
        AeAssert::gCurrentExpr = "mId == FourCC('IIBS')";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("not an instance bank set"))
            __debugbreak();
    }
    if (mVersion != 2.01f)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBank.h";
        AeAssert::gCurrentLine = 230;
        AeAssert::gCurrentExpr = "mVersion == 2.01f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("incorrect version instance bank"))
            __debugbreak();
    }
    if ((uintptr_t)mPtrFixupTable >= 0x10000000)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBank.h";
        AeAssert::gCurrentLine = 231;
        AeAssert::gCurrentExpr = "((uint32)mPtrFixupTable<0x10000000)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Fixup offset is unusually large"))
            __debugbreak();
    }
    void* v2 = (char*)this + (uintptr_t)mPtrFixupTable;
    mPtrFixupTable = v2;
    ((PtrFixupTable*)v2)->Fixup(this);
}

// ea: 0x684B50
InstanceBank& InstanceBankSet::GetBank(eInstanceBankType type)
{
    InstanceBank& bank = mInstanceBanks.mList[type];
    if (bank.mType != (int)type)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBank.h";
        AeAssert::gCurrentLine = 223;
        AeAssert::gCurrentExpr = "mInstanceBanks[int(type)].GetType() == type";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("bad data!"))
            __debugbreak();
    }
    return bank;
}

// ea: 0x686BC0
int InstanceBank::Find(const char* str, unsigned int hash) const
{
    if (hash == 0 && str != nullptr)
        hash = AeHash(str);
    unsigned int* v4 = InplaceTreeFindUInt(
        *(const InplaceTree<unsigned int, unsigned int>*)(void*)&mTree,
        &hash);
    if (v4 == nullptr)
        return -1;
    const IbEntry& v6 = InplaceVectorAtConst(mEntries, *v4);
    if (str != nullptr && strnicmp(v6.name.mStr, str, 27) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBank.h";
        AeAssert::gCurrentLine = 77;
        AeAssert::gCurrentExpr =
            "!str || strnicmp(ie.name.c_str(), str, 27)==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("hashes match but strings don't- '%s' and '%s'",
                                v6.name.mStr, str))
            __debugbreak();
    }
    return (int)*v4;
}

// ea: 0x686CA0
unsigned int* InstanceBankSet::FindEntry(eInstanceBankType type,
                                         const char* str, unsigned int hash)
{
    InstanceBank& v4 = mInstanceBanks.mList[type];
    if (v4.mType != (int)type)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBank.h";
        AeAssert::gCurrentLine = 214;
        AeAssert::gCurrentExpr = "ib.GetType() == type";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("bad data!"))
            __debugbreak();
    }
    int v5 = v4.Find(str, hash);
    if (v5 == -1)
        return nullptr;
    return &v4.mEntries.mList[v5].ptr;
}

// ea: 0x684AC0
unsigned int InstanceBank::Size() const
{
    return mEntries.mSize;
}

// ea: 0x684AF0
void InstanceBank::Enumerate(void (*Callback)(const char*, eInstanceBankType,
                                              void*, void*),
                             void* userdata)
{
    for (unsigned int v4 = 0; v4 < mEntries.mSize; ++v4)
    {
        IbEntry& v6 = mEntries.mList[v4];
        Callback(v6.name.mStr, (eInstanceBankType)mType, (void*)v6.ptr,
                 userdata);
    }
}

// ea: 0x684D20
void InstanceBankSet::Enumerate(void (*Callback)(const char*,
                                                 eInstanceBankType, void*,
                                                 void*),
                                void* userdata)
{
    for (unsigned int v4 = 0; v4 < mInstanceBanks.mSize; ++v4)
        mInstanceBanks.mList[v4].Enumerate(Callback, userdata);
}

// ea: 0x666610
void InstanceBankMgr::Enumerate(TPakId pakId,
                                void (*Callback)(const char*,
                                                 eInstanceBankType, void*,
                                                 void*),
                                void* userdata)
{
    if (pakId == PAK_ID_INVALID)
    {
        InstanceBankSet** mEntriesPtr = mEntries;
        for (int i = 99; i != 0; --i)
        {
            if (*mEntriesPtr != nullptr)
                (*mEntriesPtr)->Enumerate(Callback, userdata);
            ++mEntriesPtr;
        }
    }
    else
    {
        if (mEntries[pakId] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
            AeAssert::gCurrentLine = 547;
            AeAssert::gCurrentExpr = "mEntries[pakId]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("no instance bank!"))
                __debugbreak();
        }
        mEntries[pakId]->Enumerate(Callback, userdata);
    }
}

// ea: 0x66BEF0
unsigned int InstanceBankMgr::Add(eInstanceBankType type, TPakId pakId,
                                  const tlFixedString& name,
                                  unsigned int data)
{
    if (pakId == PAK_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 268;
        AeAssert::gCurrentExpr = "pakId != PAK_ID_INVALID";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("must supply a pak id!"))
            __debugbreak();
    }
    if (mEntries[pakId] == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 269;
        AeAssert::gCurrentExpr = "mEntries[pakId] != 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("No instance bank for this pak id"))
            __debugbreak();
    }
    unsigned int* Entry = mEntries[pakId]->FindEntry(type, name.str,
                                                     name.hash);
    if (Entry != nullptr)
    {
        unsigned int v8 = *Entry;
        *Entry = data;
        return v8;
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
    AeAssert::gCurrentLine = 274;
    AeAssert::gCurrentExpr = "slot";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("No ib entry for %s", name.str))
        __debugbreak();
    return 0;
}

// ea: 0x66C590
bool InstanceBankMgr::GetAnimOffset(const char* name, TPakId pakId,
                                    unsigned int* out_offset,
                                    unsigned int* out_size)
{
    if (pakId == PAK_ID_INVALID)
        return false;
    InstanceBankSet* v5 = mEntries[pakId];
    if (v5 == nullptr)
        return false;
    unsigned int* Entry = v5->FindEntry(INSTBANK_TYPE_SCNANIM, name, 0);
    if (Entry == nullptr)
        return false;
    if (*Entry == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 491;
        AeAssert::gCurrentExpr = "*obj";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("shouldn't be possible"))
            __debugbreak();
    }
    *out_offset = **(unsigned int**)Entry;
    *out_size = ((unsigned int*)*Entry)[1];
    if (*out_offset == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 494;
        AeAssert::gCurrentExpr = "*out_offset";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad offset!"))
            __debugbreak();
    }
    return true;
}

// ea: 0x66C5F0
bool InstanceBankMgr::GetMipScale(const char* texture, float* out_value)
{
    MipSettingsBank* bank = mMipSettingsBank;
    if (bank == nullptr)
        return false;
    float* v4 = InplaceTreeFindFloat(
        *(const InplaceTree<InplaceString, float>*)(void*)&bank->mMipSettings,
        texture);
    if (v4 == nullptr)
        return false;
    *out_value = *v4;
    return true;
}

// ea: 0x6665A0
void InstanceBankMgr::ReleaseSkeletons(TPakId pakId)
{
    m_skeleton_directory.m_enable_release = true;
    InstanceBankSet* v3 = mEntries[pakId];
    if (v3 != nullptr)
    {
        InstanceBank& bank = v3->GetBank(INSTBANK_TYPE_ANIMOFFSET);
        for (unsigned int i = 0; i < bank.mEntries.mSize; ++i)
        {
            InstanceBank::IbEntry& v6 = bank.mEntries.mList[i];
            nalReleaseSkeleton((nalBaseSkeleton*)v6.ptr);
            v6.ptr = 0;
        }
    }
    m_skeleton_directory.m_enable_release = false;
}

// ea: 0x684E40
void MipSettingsBank::Fixup()
{
    if (mId != 1296650323)  // FourCC('MIPS')
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MipSettings.h";
        AeAssert::gCurrentLine = 28;
        AeAssert::gCurrentExpr = "mId == FourCC('MIPS')";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("not an mipsettings bank"))
            __debugbreak();
    }
    if (mVersion != 1.01f)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MipSettings.h";
        AeAssert::gCurrentLine = 29;
        AeAssert::gCurrentExpr = "mVersion == 1.01f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("incorrect version mipsettings bank"))
            __debugbreak();
    }
    if ((uintptr_t)mPtrFixupTable >= 0x10000000)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MipSettings.h";
        AeAssert::gCurrentLine = 30;
        AeAssert::gCurrentExpr = "((uint32)mPtrFixupTable<0x10000000)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Fixup offset is unusually large"))
            __debugbreak();
    }
    mPtrFixupTable = (char*)this + (uintptr_t)mPtrFixupTable;
}

// ea: 0x666530
void InstanceBankMgr::DecodeMipsettings(const char* name, unsigned char* data,
                                        int size, TPakId pakId)
{
    (void)name; (void)size;
    MipSettingsBank* bank = (MipSettingsBank*)data;
    bank->Fixup();
    bank->mPakId = pakId;
    if (mMipSettingsBank != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 218;
        AeAssert::gCurrentExpr = "mMipSettingsBank == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("mip settings bank already used!"))
            __debugbreak();
    }
    mMipSettingsBank = bank;
}

// ea: 0x66F8B0
void InstanceBankMgr::RegisterAnimOffset(const char* name,
                                         unsigned int offset,
                                         unsigned int size, TPakId pakId)
{
    char cpy[128];
    char dstBuff[128];
    char oBuff[128];
    int oLen = 0;
    AeStringSupport::CStrToAeStr(cpy, &oLen, 127, name);
    cpy[127] = (char)oLen;
    dstBuff[0] = 0;
    int nameLen = 0;
    AeStringSupport::GetFileName(dstBuff, &nameLen, cpy, oLen, true);
    oLen = 0;
    AeStringSupport::AeStrCopy(oBuff, &oLen, 127, dstBuff, nameLen);
    oBuff[127] = (char)oLen;
    memcpy(cpy, oBuff, sizeof(cpy));

    if (mEntries[pakId] == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 471;
        AeAssert::gCurrentExpr = "mEntries[(int)pakId]";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no instance bank!"))
            __debugbreak();
    }
    unsigned int* v7 =
        (unsigned int*)PakManager::sInst->MemAlloc(pakId, 8u, false);
    v7[0] = offset;
    v7[1] = size;
    unsigned int* Entry =
        mEntries[pakId]->FindEntry(INSTBANK_TYPE_SCNANIM, cpy, 0);
    if (Entry == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InstanceBankMgr.cpp";
        AeAssert::gCurrentLine = 476;
        AeAssert::gCurrentExpr = "slot";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("no anim offset for %s", cpy))
            __debugbreak();
    }
    *Entry = (unsigned int)v7;
}

// ============================================================================
// GetName / RegisterMesh / GetMultiApkFcn (streamer.o 0x66F3C0 - 0x66F670)
// ============================================================================
// ea: 0x4E67D0 (streamer.o COMDAT; ae_fixed_string<128,unsigned char>)
template <>
ae_fixed_string<128, unsigned char>
ae_fixed_string<128, unsigned char>::get_file_name(bool truncExt) const
{
    ae_fixed_string<128, unsigned char> result;
    char r[127];
    r[0] = 0;
    int dstLen = 0;
    AeStringSupport::GetFileName(r, &dstLen, (const char*)mBuff, mLength,
                                 truncExt);
    int oLen = 0;
    AeStringSupport::AeStrCopy((char*)result.mBuff, &oLen, 127, r, dstLen);
    result.mLength = (unsigned char)oLen;
    return result;
}

// ea: 0x686FF0 (streamer.o COMDAT; ae_fixed_string<512,unsigned short>)
template <>
ae_fixed_string<512, unsigned short>
ae_fixed_string<512, unsigned short>::get_file_name(bool truncExt) const
{
    ae_fixed_string<512, unsigned short> result;
    char r[510];
    r[0] = 0;
    int dstLen = 0;
    AeStringSupport::GetFileName(r, &dstLen, (const char*)mBuff, mLength,
                                 truncExt);
    int oLen = 0;
    AeStringSupport::AeStrCopy((char*)result.mBuff, &oLen, 510, r, dstLen);
    result.mLength = (unsigned char)oLen;
    return result;
}

// ea: 0x686FF0 (streamer.o COMDAT; ae_fixed_string<256,unsigned short>)
template <>
ae_fixed_string<256, unsigned short>
ae_fixed_string<256, unsigned short>::get_file_name(bool truncExt) const
{
    ae_fixed_string<256, unsigned short> result;
    char r[254];
    r[0] = 0;
    int dstLen = 0;
    AeStringSupport::GetFileName(r, &dstLen, (const char*)mBuff, mLength,
                                 truncExt);
    int oLen = 0;
    AeStringSupport::AeStrCopy((char*)result.mBuff, &oLen, 254, r, dstLen);
    result.mLength = (unsigned char)oLen;
    return result;
}

// ea: 0x66F3C0
tlFixedString GetName(const char* name)
{
    ae_fixed_string<512, unsigned short> cpy;
    int oLen = 0;
    AeStringSupport::CStrToAeStr((char*)cpy.mBuff, &oLen, 510, name);
    cpy.mLength = (unsigned char)oLen;
    cpy = cpy.get_file_name(true);
    char* p_cpy = (char*)cpy.mBuff;
    if (p_cpy[0] != 0)
    {
        char v3;
        do
        {
            if (p_cpy[0] == 46)
                p_cpy[0] = 0;
            v3 = p_cpy[1];
            ++p_cpy;
        } while (v3 != 0);
    }
    tlFixedString str((const char*)cpy.mBuff);
    return str;
}

struct MultiApk;
typedef void (*MultiApkCallback)(const char*, MultiApk*, TPakId);

// MultiApk reference tables (apkSupport.cpp; verified against IDA)
struct MultiApkObject {
    tlFixedString* name;  // +0x00
    void* ptr;            // +0x04
};
struct MultiApkReference {
    unsigned int    Type;      // +0x00
    unsigned int    NObjects;  // +0x04
    MultiApkObject* Objects;   // +0x08
};
struct MultiApk {
    unsigned int       NReferences;  // +0x00
    MultiApkReference* References;   // +0x04
};

// ea: 0x66F480
void RegisterMesh(const char* name, MultiApk* file, TPakId pakId)
{
    tlFixedString result = GetName(name);
    InstanceBankMgr::sInst->Add(INSTBANK_TYPE_MESHFILE, pakId, result,
                                 (unsigned int)file);
}

// ea: 0x66F670
MultiApkCallback GetMultiApkFcn(const char* ext)
{
    tlFixedString v3(ext);
    if (v3.hash == 521171546 || v3.hash == 521531143)
        return RegisterMesh;
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DecodePakFile.cpp";
    AeAssert::gCurrentLine = 443;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("no multi-apk callback for type '%s'", ext))
        __debugbreak();
    return nullptr;
}

// ea: 0x6744B0
void cdInvokeMultiApkCallbacks(const char* name, const char* file_ext,
                               TPakId pakId, apk::apkFile* File,
                               apk::apkFileEntry* Entry)
{
    tlFixedString image("image");
    int SectionIndex = File->GetSectionIndex(image);
    MultiApk* multi =
        (MultiApk*)Entry->GetData(File, SectionIndex, true);
    unsigned int refIdx = 0;
    if (multi->NReferences != 0)
    {
        do
        {
            MultiApkReference* ref = &multi->References[refIdx];
            apk::apkFileTypeEntry* typeEntry =
                File->GetFileTypeEntry(ref->Type);
            if (typeEntry == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\apkSupport.cpp";
                AeAssert::gCurrentLine = 61;
                AeAssert::gCurrentExpr = "typeEntry";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Bad apk!"))
                    __debugbreak();
            }
            apk::apkFileEntry* firstFile =
                typeEntry->FirstEntry;
            unsigned int objIdx = 0;
            if (ref->NObjects != 0)
            {
                while (1)
                {
                    apk::apkFileEntry* entry = nullptr;
                    MultiApkObject* obj =
                        (MultiApkObject*)((char*)ref->Objects + 8 * objIdx);
                    if (firstFile != nullptr)
                    {
                        apk::apkFileEntry* cur = firstFile;
                        do
                        {
                            const unsigned int* a =
                                (const unsigned int*)cur->Name;
                            const unsigned int* b =
                                (const unsigned int*)obj->name;
                            int v17 = 0;
                            while (a[v17] == b[v17])
                            {
                                ++v17;
                                if (v17 >= 8)
                                {
                                    entry = cur;
                                    break;
                                }
                            }
                            if (entry != nullptr)
                                break;
                            cur = File->GetNextFile(ref->Type, cur);
                        } while (cur != nullptr);
                    }
                    File->ApplyReferencesForEntry(entry);
                    if (entry == nullptr)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\apkSupport.cpp";
                        AeAssert::gCurrentLine = 83;
                        AeAssert::gCurrentExpr = "entry";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bad apk!"))
                            __debugbreak();
                    }
                    if (!File->InvokeFileLoadCallback(typeEntry, entry))
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\apkSupport.cpp";
                        AeAssert::gCurrentLine = 85;
                        AeAssert::gCurrentExpr = "found";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(
                                   "Didn't find load callback for %s",
                                   obj->name->str))
                            __debugbreak();
                    }
                    if (++objIdx >= ref->NObjects)
                        break;
                }
            }
            ++refIdx;
        } while (refIdx < multi->NReferences);
    }
    MultiApkCallback MultiApkFcn = GetMultiApkFcn(file_ext);
    if (MultiApkFcn != nullptr)
        MultiApkFcn(name, multi, pakId);
}

// ea: 0x665370
void TogglePakRender()
{
    PakManager::sInst->mDebugRenderMode.bankUsage ^= 1;
}

// ea: 0x665390
unsigned long PakGetThreadId()
{
    return GetCurrentThreadId();
}

// ea: 0x665570
float PakManager::GetDistance(PakInfoNode* node) const
{
    if (node->visited == sComputeDistanceKey)
        return node->computedDistance;
    float userDistance = node->userDistance;
    if (userDistance == 3.4028235e38f)
        userDistance = node->distance;
    EPakType pakType = node->pakType;
    bool is_unloadable = pakType >= kPakTypeGlobal
                         && (pakType <= kPakTypeAnimation
                             || pakType == kPakTypeCount);
    float distance = userDistance;
    if (is_unloadable && node->pakId == PAK_ID_INVALID)
        return 3.4028235e38f;
    if (node->userDistance == -1.0f
        || (node->distance == -1.0f && node->userDistance == 3.4028235e38f))
    {
        if (is_unloadable)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PakManager.cpp";
            AeAssert::gCurrentLine = 1797;
            AeAssert::gCurrentExpr = "is_unloadable_type(node->pakType)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                       "global/hero shouldn't have -1 user distance!"))
                __debugbreak();
        }
        return 3.4028235e38f;
    }
    if (is_unloadable && node->pakId != PAK_ID_INVALID)
    {
        userDistance = 0.0f;
        distance = 0.0f;
    }
    node->visited = sComputeDistanceKey;
    node->computedDistance = userDistance;
    return distance;
}
