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
#include "core/mem_heap.h"
#include "core/memory_types.h"
#include "ngl/nglFont.h"
#include "ngl/nglTexture.h"
#include "ngl/ngl_dx_quad.h"
#include "filesystem/apk.h"
#include "core/ae_fixed_string.h"
#include "core/tlFixedString.h"
#include "core/tlResourceDirectory.h"
#include "engine/broc_types.h"

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
};

extern void SV_SetConfigstring(int index, const char* val);  // sv.o
extern void Cvar_Set(const char* var_name, const char* value);  // core.o
class Entity;  // game_types.h
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
    EntityMin* mWorld;            // +0x44
};

// mem_heap (core_xboxr mem_lib; PakFile uses start/end/size/used_byte)
struct mem_heap {
    char*  start;      // +0x00
    char*  end;        // +0x04
    uint8_t _pad8[0x484 - 0x08];
    unsigned int size;      // +0x484
    unsigned int used_byte; // +0x488
    unsigned int high_used_byte;  // +0x48C
    mem_heap* reserve;      // +0x490
};

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

// Color (core/color.h view; RGBA float)
class Color {
public:
    float r, g, b, a;
};

// shell.o / render.o C-bridge stubs for MyRenderText
typedef int font_index;
extern font_index FEManager_FindFont(void* fe, const char* name, bool check);
extern nglFont* FEManager_GetFont(void* fe, font_index f, float scale);
extern unsigned int extract_color(const Color& col);
extern void* g_femanager_ptr;

// Cross-object stubs (shell.o / render.o)
font_index FEManager_FindFont(void* fe, const char* name, bool check)
{ (void)fe; (void)name; (void)check; return -1; }
nglFont* FEManager_GetFont(void* fe, font_index f, float scale)
{ (void)fe; (void)f; (void)scale; return nullptr; }
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
extern void tlMemFree(void* ptr);                // tl_system.o
extern unsigned int AeHash(const char* str);     // ae_hash.cpp
// InplaceTree<T,K>::Find (tree not ported yet) - file-local stubs
static unsigned int* InplaceTreeFindUInt(void* tree,
                                        const unsigned int* key);
static float* InplaceTreeFindFloat(void* tree, const char* const* key);
struct cvar_t {
    char*  name;    // +0x00
    char*  string;  // +0x04
    float  value;   // +0x08
};
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                        int flags);  // core.o cvar.cpp
extern bool ShouldConnectPaths();                 // core.o sys.cpp
extern void mem_heap_create(mem_heap* heap, void* start, void* end,
                            mem_heap* reserve);  // mem_heap.cpp
extern int mem_heap_destroy(mem_heap* heap);     // mem_heap.cpp
extern bool mem_heap_free_check_reserve(mem_heap* heap, void* ptr);  // mem_heap.cpp
extern const char* const defaultFileName;  // g_globals.cpp
namespace AeStringSupport {
void CStrToAeStr(char* dst, int* dstLen, int capacity, const char* src);
void GetFileName(char* dst, int* dstLen, const char* path, int pathLen,
                 bool includeExt);
void AeStrCopy(char* dst, int* dstLen, int dstCapacity, const char* src,
               int srcLen);
}
typedef unsigned nflState;
typedef unsigned nflFileID;
enum nflMediaID : unsigned { NFL_MEDIA_DEFAULT = 0 };  // matches filesystem/nfl.cpp
#define NFL_FILE_ID_INVALID ((nflFileID)-1)
extern nflFileID nflOpenFile(nflMediaID media, const char* name);  // filesystem/nfl.cpp
extern void nflUpdate();
extern nflState nflGetState();
extern unsigned int nflReadFile(nflFileID file, unsigned offset, void* buf,
                                unsigned size);
#define NFL_STATE_ERROR 2
extern void mem_break();  // mem_heap.cpp

// nfl async (nfl_common.o; streamer-side stubs)
typedef unsigned int nflRequestID;
#define NFL_PRIORITY_LOWEST 0
typedef void (*nflReadCallback)(unsigned int state, unsigned int);
extern nflRequestID nflReadFileAsyncWithCallBack(
    nflFileID fileID, unsigned int fileOffset, void* buffer,
    unsigned int dataSize, nflReadCallback callback);
extern void nflSetRequestPriority(nflRequestID requestID,
                                  unsigned int priority);
nflRequestID nflReadFileAsyncWithCallBack(
    nflFileID fileID, unsigned int fileOffset, void* buffer,
    unsigned int dataSize, nflReadCallback callback)
{
    (void)fileID; (void)fileOffset; (void)buffer; (void)dataSize;
    (void)callback;
    return 0;
}
void nflSetRequestPriority(nflRequestID requestID, unsigned int priority)
{
    (void)requestID; (void)priority;
}
extern nflFileID nflOpenFileEx(nflMediaID media, const char* fileName,
                               unsigned int* fileSize);  // nfl_common.o
extern void nflCancelFileRequests(nflFileID fileID);     // nfl_common.o
extern void nflCloseFile(nflFileID file);                // filesystem/nfl.cpp
nflFileID nflOpenFileEx(nflMediaID media, const char* fileName,
                        unsigned int* fileSize)
{
    (void)media; (void)fileSize;
    return nflOpenFile(media, fileName);
}
void nflCancelFileRequests(nflFileID fileID)
{
    (void)fileID;
}
struct nflRequestInfoStub {
    unsigned int bytesCompleted;  // +0x00 (nfl_common.o request info)
};
extern void nflGetRequestInfo(nflRequestID requestID,
                              nflRequestInfoStub* requestInfo);
void nflGetRequestInfo(nflRequestID requestID,
                       nflRequestInfoStub* requestInfo)
{
    (void)requestID;
    requestInfo->bytesCompleted = 0;
}
void codNflCallback(unsigned int state, unsigned int requestId);
void codNflCallback(unsigned int state, unsigned int requestId)
{
    (void)state; (void)requestId;
}

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, AC = 9 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Warning(const char* fmt, ...);
bool Error(const char* fmt, ...);
}

enum TPakId { kPakTypeNone = -1 };
#define PAK_ID_INVALID ((TPakId)-1)
#define PAK_ID_MIN ((TPakId)0)
#define PAK_ID_MAX ((TPakId)99)
enum EPakType {
    kPakTypeGlobal = 0,
    kPakTypeFrontEnd = 1,
    kPakTypeAnimation = 2,
    kPakTypeLevel = 3,
    kPakTypeCount = 10,
};
class NumBanks {
public:
    float    ps2;              // +0x00
    uint8_t  _pad4[8];         // +0x04 (NumBanks::Ps3Banks)
    float    xbox;             // +0x0C
    float    xenon;            // +0x10
    float    pcx;              // +0x14
    uint8_t  _pad18[8];        // +0x18 (NumBanks::GcBanks)

    float& to_float();         // ?to_float@NumBanks@@QAEAAMXZ
    float to_float() const;    // ?to_float@NumBanks@@QBEMXZ
};

// ae/core/BitSet.h view (word-based; the core_systems.h template is byte-based)
template <int N>
struct BitSet {
    static const int kNumWords = (N + 31) / 32;
    unsigned int mBits[kNumWords];  // +0x00

    bool Test(int v) const;
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
};


// TBankAlloc (BankManager.cpp; two 64-bit bank allocation bitmaps)
struct TBankAlloc {
    BitSet<64> mram_alloc1;  // +0x00
    BitSet<64> mram_alloc2;  // +0x08

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
    BankManager();                // ??0BankManager@@QAE@XZ
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
};

template <typename T, int N>
struct ae_sized_array {
    T m_elements[N];  // +0x00
    int m_size;       // +N*sizeof(T)

    void push_back(const T& elt)
    {
        if (m_size < N)
            m_elements[m_size++] = elt;
    }
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
    virtual ~AssetBankSet();  // defined in g_entity_misc.cpp
    AssetBankSet();           // defined in ctor_dtor.cpp
    virtual void UnloadBank(TPakId pakId);  // ?UnloadBank@AssetBankSet@@UAEXW4TPakId@@@Z (streamer.o)

    static ae_sized_array<AssetBankSet*, 24> sBankArray;  // ?sBankArray@AssetBankSet@@0V?$ae_sized_array@PAVAssetBankSet@@$0BI@@@A
    static void UnloadBanks(TPakId pakId);  // ?UnloadBanks@AssetBankSet@@SAXW4TPakId@@@Z (streamer.o 0x66B0E0)
};
ae_sized_array<AssetBankSet*, 24> AssetBankSet::sBankArray;

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

// ThroughputMeasurer (streamer.o PakFile.cpp; 16 bytes, verified IDA)
struct ThroughputMeasurer {
    int   mBytes;       // +0x00
    int   mTotalBytes;  // +0x04
    float mStart;       // +0x08
    float mTotalTime;   // +0x0C

    float safeGetTime() const;  // ?safeGetTime@ThroughputMeasurer@@QBEMXZ
    void Update(int bytes);     // ?Update@ThroughputMeasurer@@QAEXH@Z
};
ThroughputMeasurer gThroughputMeasurer;  // ?gThroughputMeasurer@@3UThroughputMeasurer@@A @ 0xF59318
unsigned int g_bytes_read = 0;           // ?g_bytes_read@@3IA @ 0xF592E4
float g_throughput = 0.0f;               // ?g_throughput@@3MA @ 0xF592E0

// ea: 0x681110
float ThroughputMeasurer::safeGetTime() const
{
    return (float)((double)__rdtsc() * 0.0000013636364 * 0.001);
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

// FEManager (shell.o; DrawDiscError + UnloadBank stubs)
class FEManager {
public:
    void DrawDiscError();  // ?DrawDiscError@FEManager@@QAEXXZ (shell.o; stub)
    void UnloadBank(TPakId pakId);  // ?UnloadBank@FEManager@@QAEXW4TPakId@@@Z (shell.o; stub)
};
// shell.o owns the real symbol; placeholder until shell.o is ported
FEManager g_femanager;
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

// nfl (nfl_xboxr) request state/id enums
enum nflRequestState {
    NFL_REQUEST_STATE_INVALID = -1,
    NFL_REQUEST_STATE_COMPLETED = 0,
    NFL_REQUEST_STATE_CANCELED = 1,
    NFL_REQUEST_STATE_TIMEOUT = 2,
    NFL_REQUEST_STATE_ERROR = 3,
    NFL_REQUEST_STATE_ACTIVE = 4,
};
typedef unsigned int nflRequestID;
typedef unsigned int nflFileID;
#define NFL_REQUEST_ID_INVALID ((nflRequestID)-1)
// nfl.cpp defines nflRequestState as unsigned; match its mangled symbol
extern unsigned int nflGetRequestState(unsigned int requestID);

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

    struct TRequestId {
        nflRequestID nflId;  // +0x00
    };

    uint8_t      _pad0[0x08];           // m_dlist_node
    const char*  mCurrDecodeFile;       // +0x08
    struct {
        char mBuff[64];  // ae_fixed_string<64>
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
    uint8_t      _padE4[0xE8 - 0xE4];
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
    void*       longName;       // +0x04
    void*       path;           // +0x08
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
struct InplaceVector {
    unsigned int mSize;  // +0x00
    T*           mList;  // +0x04
};

// PakInfoBank (streamer.o; InplaceAssetBank base, mPtrs +0x10)
class PakInfoBank {
public:
    uint8_t _pad[0x10];
    InplaceVector<const PakInfoNode*> mPtrs;  // +0x10
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

    mem_info(unsigned char* data_, int size_);  // ??0mem_info@@QAE@PAEH@Z
};

// Color32 (streamer.o; 4 bytes)
struct Color32 {
    unsigned int i;  // +0x00

    Color32(unsigned int ic);       // ??0Color32@@QAE@I@Z
    unsigned int to_ulong() const;  // ?to_ulong@Color32@@QBEIXZ
};

// reserved_dlist<T> (ae/core; intrusive node = T's first member) - verified IDA
template <typename T>
struct reserved_dlist {
    struct dlist_node {
        dlist_node* m_next;  // +0x00
        dlist_node* m_prev;  // +0x04
    };

    int         m_size;  // +0x00
    dlist_node* m_head;  // +0x04
    dlist_node* m_end;   // +0x08
    dlist_node* m_tail;  // +0x0C
};

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
    struct TThreadedPakContextStack {
        uint32_t key;            // +0x00 thread id
        TPakId stack[128];       // +0x04
        int m_size;              // +0x204
    };

    uint8_t _pad0[0x10];
    int mState;                       // +0x10 PakManager::state_e
    bool mFilled;                     // +0x14
    bool mFillingBanks;               // +0x15
    PakInfoBank* mPakInfoBank;        // +0x18
    PakInfoBank* mLevelPakInfoBank;   // +0x1C
    uint8_t _pad20[0x24 - 0x20];
    TPakId mCurrentPakId;             // +0x24
    uint8_t _pad28[0x2C - 0x28];
    TPakId mAnimPakId;                // +0x2C
    TPakId mGlobalPakId;              // +0x30
    uint8_t _pad34[0x38 - 0x34];
    TPakId mLevelPakId;               // +0x38
    TPakId mDebugPakId;               // +0x3C
    PakFile* mSlots[99];              // +0x40
    const PakInfoNode* mPakInfoPtrs[99];  // +0x1CC
    void (*mProgressCallback)(float); // +0x364
    uint8_t _pad368[0x4384 - 0x368];
    int mEnabled;                     // +0x4384
    reserved_dlist<PakFile> mActivePaks;  // +0x4388
    TThreadedPakContextStack mContextStack[1];  // +0x4398

    static PakManager* sInst;          // ?sInst@PakManager@@2PAV1@A (sv_globals.cpp)
    int mDebugRenderMode;              // +0x00 (first member; TogglePakRender)
    static unsigned int sComputeDistanceKey;  // ?sComputeDistanceKey@PakManager@@0IA
    static float sBrocPercentage;      // ?sBrocPercentage@PakManager@@0MA
    static float sWbkPercentage;       // ?sWbkPercentage@PakManager@@0MA

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
    // - ea: 0x665880 (stub)
    void Update(bool calledFromMovie);
    // - ea: 0x665B90 (stub)
    const PakInfoNode* SyncLoadFLI(EPakType t, const char* path);
    // - ea: 0x665900 (stub)
    const PakInfoNode* GetPakInfo(TPakId pakId) const;
    // - ea: 0x665960 (stub)
    const PakInfoNode* GetPakInfo(const char* long_name) const;
};

// PoolAllocator (core/PoolAllocator.h)
class PoolAllocator {
public:
    struct PoolConfig {
        unsigned short blockSize;   // +0x00
        unsigned short numBlocks;   // +0x02
        unsigned short blockAlign;  // +0x04
        char*          block;       // +0x08
    };

    PoolAllocator(const ae_sized_array<PoolConfig, 16>& cfgList,
                  unsigned int flags);  // ??0PoolAllocator@@QAE@ABV?$ae_sized_array@UPoolConfig@PoolAllocator@@$0BA@@@I@Z
    void* Allocate(unsigned int size, bool forceHeapAlloc = false);  // ?Allocate@PoolAllocator@@QAEPAXI_N@Z
    void Release(void* ptr);  // ?Release@PoolAllocator@@QAEXPAX@Z
};
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

enum eInstanceBankType {
    INSTBANK_TYPE_APK = 0,
    INSTBANK_TYPE_TEXTURE,
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

// cdResourceDirectory<T> object layout (12 bytes; vftable + flags + old dir)
struct CdResourceDirectoryView {
    void* __vftable;          // +0x00
    bool  m_enable_release;   // +0x04
    bool  m_release_once;     // +0x05
    bool  m_enable_add;       // +0x06
    void* m_old_directory;    // +0x08
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
    CdResourceDirectoryView m_anim_directory;      // +0x00
    CdResourceDirectoryView m_scnanim_directory;   // +0x0C
    CdResourceDirectoryView m_animfile_directory;  // +0x18
    CdResourceDirectoryView m_skeleton_directory;  // +0x24
    MipSettingsBank* mMipSettingsBank;  // +0x30
    InstanceBankSet* mEntries[99];  // +0x34
    static InstanceBankMgr* sInst;  // ?sInst@InstanceBankMgr@@2PAV1@A

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
    bool GetAnimOffset(const char* name, TPakId pakId, unsigned int* out_offset,
                       unsigned int* out_size);  // ?GetAnimOffset@InstanceBankMgr@@QAE_NPBDW4TPakId@@PAI2@Z
    bool GetMipScale(const char* texture,
                     float* out_value);  // ?GetMipScale@InstanceBankMgr@@QAE_NPBDAAM@Z
};
InstanceBankMgr* InstanceBankMgr::sInst = nullptr;

// Manager DecodeBank stubs (cross-object: core.o / render.o / mp_actors.o)
class XModelManager {
public:
    static XModelManager* sInst;  // defined in sv_globals.cpp
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);
};
class XModelPartsManager {
public:
    static XModelPartsManager* sInst;
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
class PathNodeMgr {
public:
    static PathNodeMgr* sInst;  // defined in sv_globals.cpp
    void DecodeLevelBank(const char* name, unsigned char* data, int size,
                         TPakId pakId);
    void DecodeZoneBank(const char* name, unsigned char* data, int size,
                        TPakId pakId);
};
class DbTablesetMgr {
public:
    static DbTablesetMgr* sInst;
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pakId);
};
class LightGridMgr {
public:
    static LightGridMgr* sInst;
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pakId);
};
class STBManager {
public:
    static STBManager* sInst;
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);  // core.o 0x4C6070 (stb.cpp)
};

XModelPartsManager* XModelPartsManager::sInst = nullptr;
AITypeManager* AITypeManager::sInst = nullptr;
DbTablesetMgr* DbTablesetMgr::sInst = nullptr;
LightGridMgr* LightGridMgr::sInst = nullptr;
STBManager* STBManager::sInst = nullptr;

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
    uint8_t _pad18[0x1C - 0x18];
    InplaceVector<const ZoneCellDesc*> mCells;  // +0x1C
    uint8_t _pad20[0x3C - 0x20];
    InplaceVector<const ZoneOverrideBrushSet*> mToggleableOverrideBoxes;  // +0x3C
    int     mNextBank;             // +0x44
    uint8_t _pad48[0x50 - 0x48];
    int     mNumToggleableOverrideBoxesHit;  // +0x50
};

// ZoneOverrideBrushSet (streamer.o; mZoneBitset +0x0C, mZoneDistances +0x14)
class ZoneOverrideBrushSet {
public:
    unsigned int mWasInside;       // +0x00
    uint8_t      _pad4[0x0C - 0x04];
    BitSet<64>   mZoneBitset;      // +0x0C
    InplaceVector<float> mZoneDistances;  // +0x14
    InplaceVector<InplaceTriple<InplaceString, const PakInfoNode*, float> >
        mNonZoneDistances;         // +0x1C

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
struct BoundingBox;
class ZoneCellBox {
public:
    const BoundingBox& GetBounds() const;  // ?GetBounds@ZoneCellBox@@QBEABVBoundingBox@@XZ
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
};

// ZoneCellDesc (streamer.o view; mZone +0x2C)
class ZoneCellDesc {
public:
    uint8_t _pad[0x20];
    unsigned int mCellId;       // +0x20
    uint8_t _pad24[0x2C - 0x24];
    const StreamZone* mZone;  // +0x2C

    const StreamZone* GetZone() const;  // ?GetZone@ZoneCellDesc@@QBEPBVStreamZone@@XZ
};

// StreamZoneManager (streamer.o; mDebugRenderMode +0x190, mInitialPosition +0x1A0)
class StreamZoneManager {
public:
    ae_array<ZoneBoundaryBank*, 99> mBankArray;  // +0x00
    struct {
        unsigned int mEnabled : 1;  // bit 0
        float zoneGraphScale;       // +0x194
    } mDebugRenderMode;             // +0x190
    math::Position3 mInitialPosition;  // +0x1A0
    uint8_t _pad1AC[0x1C0 - 0x1AC];
    int     mInitialCell;           // +0x1C0
    int     mLastCellNum;           // +0x1C4
    int     mLastListSize;          // +0x1C8
    int     mFirstBank;             // +0x1CC

    static StreamZoneManager* sInst;  // defined in sv_globals.cpp
    void SetInitialPosition(const math::Position3& pos);
    void SetInitialCell(int cell);  // ?SetInitialCell@StreamZoneManager@@QAEXH@Z
    void OnLoading(TPakId pakId);  // ?OnLoading@StreamZoneManager@@QAEXW4TPakId@@@Z (empty no-op)
    void OnUnloaded(TPakId pakId);  // ?OnUnloaded@StreamZoneManager@@QAEXW4TPakId@@@Z
    void OnUnload(TPakId pakId);  // ?OnUnload@StreamZoneManager@@QAEXW4TPakId@@@Z
    int GetNumZones() const;        // ?GetNumZones@StreamZoneManager@@QBEHXZ
    const StreamZone* GetZoneByIndex(unsigned int index) const;  // ?GetZoneByIndex@StreamZoneManager@@QBEPBVStreamZone@@I@Z
    const StreamZone* FindZone(TPakId pakId) const;  // ?FindZone@StreamZoneManager@@QBEPBVStreamZone@@W4TPakId@@@Z
    const StreamZone* FindZone(const char* name) const;  // ?FindZone@StreamZoneManager@@QBEPBVStreamZone@@PBD@Z
    const PakInfoNode* GetCellPakInfo(int cellIndex);  // ?GetCellPakInfo@StreamZoneManager@@QAEPBUPakInfoNode@@H@Z
    const StreamZone* GetCellZone(unsigned int cellIndex);  // ?GetCellZone@StreamZoneManager@@QAEPBVStreamZone@@I@Z
    void CheckpointRestart();       // ?CheckpointRestart@StreamZoneManager@@QAEXXZ
private:
    void SetTopOverrideBrushSet(ZoneBoundaryBank* bank,
                                ZoneOverrideBrushSet* zob);  // ?SetTopOverrideBrushSet@StreamZoneManager@@AAEXPAVZoneBoundaryBank@@PAVZoneOverrideBrushSet@@@Z
};

// SceneManager (render.o view; mWorldSpawn +0x1A0, mDebugRenderDist +0x1B0,
// mDebugRenderEnts +0x1B4, mDebugRenderLights +0x1B5)
struct SceneManager {
public:
    uint8_t _pad[0x1A0];
    void*   mWorldSpawn;        // +0x1A0
    uint8_t _pad1A4[0x1B0 - 0x1A4];
    float   mDebugRenderDist;   // +0x1B0
    bool    mDebugRenderEnts;   // +0x1B4
    bool    mDebugRenderLights; // +0x1B5

    static SceneManager* sInst;  // ?sInst@SceneManager@@2PAV1@A (sv_main.cpp)
    void ToggleSceneFX(float dist);  // ?ToggleSceneFX@SceneManager@@QAEXM@Z
    void ToggleRenderEnts();         // ?ToggleRenderEnts@SceneManager@@QAEXXZ
    void ToggleRenderLights();       // ?ToggleRenderLights@SceneManager@@QAEXXZ
    void ProcessWorldSpawn(const WorldSpawn& worldspawn);  // ?ProcessWorldSpawn@SceneManager@@AAEXABVWorldSpawn@@@Z
};

// ae_heap (core_xboxr; vtable+4 = Malloc(unsigned size, int align))
class ae_heap {
public:
    void** __vftable;  // +0x00
    void* Malloc(unsigned int size, int alignment);
};
extern ae_heap* gActorHeap;  // ?gActorHeap@@3PAVae_heap@@A @ 0xF00E5C

// Cross-object stubs (ae_heap core_xboxr; ported with core)
void* ae_heap::Malloc(unsigned int size, int alignment)
{
    (void)size; (void)alignment;
    return nullptr;
}
ae_heap* gActorHeap = nullptr;

// ea: 0x8A39E0 (core.o inline)
void PakManager::CreateInst()
{
    // stub: sInst = new PakManager
}

// ea: 0x8D4D40 (core.o inline)
void PakManager::DeleteInst()
{
    // stub
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
        unsigned int* v5 = InplaceTreeFindUInt(
            (void*)((char*)v4 + 0), (const unsigned int*)&name);
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
void cdDeleteTextureCallback(apk::apkFile* File, apk::apkFileEntry* Entry)
{
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
    nflSetRequestPriority(v8, NFL_PRIORITY_LOWEST);
    *buffer = (unsigned char)v8;
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
        if (!v5->MemFree(ptr, true))
            mem_heap_free(ptr);
    }
    else
    {
        mem_heap_free(ptr);
    }
}

// ============================================================================
// StartupNfl (streamer.o 0x666F50) - binary nfl API view
// ============================================================================
struct nflInitParamsBin {
    unsigned int maxFiles;     // +0x00
    unsigned int maxStreams;   // +0x04
    unsigned int maxRequests;  // +0x08
    unsigned int bufferMode;   // +0x0C
    unsigned int threadMode;   // +0x10
};
struct nflMediaAlignmentsBin {
    unsigned int mediaAlignment;        // +0x00
    unsigned int memoryAlignment;       // +0x04
    unsigned int transferSizeAlignment; // +0x08
};

enum {
    NFL_BUFFER_MODE_UNALIGNED = 0,
    NFL_THREAD_MODE_SINGLE = 0,
};
extern unsigned int nflInit(nflInitParamsBin* ip);  // nfl_common.o
extern void nflStart(void* work);
extern void nflGetMediaAlignments(unsigned int mediaID,
                                  nflMediaAlignmentsBin* ma);
extern unsigned int gNflMediaId;  // ?gNflMediaId@@3IA
void* gNflMemAlloc = nullptr;     // ?gNflMemAlloc@@3PAXA
unsigned int gNflAlignment = 0;   // ?gNflAlignment@@3IA
unsigned int gNflMediaId = 0;

extern void nflShutdown();  // nfl_common.o

// ea: 0x6658C0
void ShutdownNfl()
{
    nflShutdown();
    gNflMemAlloc = nullptr;
}

unsigned int nflInit(nflInitParamsBin* ip)
{
    (void)ip;
    return 4096;
}
void nflGetMediaAlignments(unsigned int mediaID, nflMediaAlignmentsBin* ma)
{
    (void)mediaID;
    ma->mediaAlignment = 0;
    ma->memoryAlignment = 0;
    ma->transferSizeAlignment = 0;
}

// ea: 0x666F50
void StartupNfl()
{
    nflInitParamsBin v2;
    v2.maxFiles = 128;
    v2.maxRequests = 128;
    v2.maxStreams = 1;
    v2.bufferMode = NFL_BUFFER_MODE_UNALIGNED;
    v2.threadMode = NFL_THREAD_MODE_SINGLE;
    unsigned int v0 = nflInit(&v2);
    gNflMemAlloc = mem_heap_malloc(v0);
    nflStart(gNflMemAlloc);
    nflMediaAlignmentsBin mediaAlignments;
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
        StreamZoneManager::sInst->mDebugRenderMode.mEnabled ^= 1;
    }
    else
    {
        StreamZoneManager::sInst->mDebugRenderMode.mEnabled |= 1;
        StreamZoneManager::sInst->mDebugRenderMode.zoneGraphScale =
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
    static font_index font = -1;
    static nglFont* cached_font = nullptr;
    if ((S12_3_guard & 1) == 0)
    {
        S12_3_guard |= 1;
        font = FEManager_FindFont(&g_femanager, "i_helvetica_bold", false);
        cached_font = FEManager_GetFont(&g_femanager, font, 1.0f);
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

// DebugRender (render.o; RenderText defined in g_entity_misc.cpp)
class DebugRender {
public:
    static void RenderText(const char* str, int x, int y, const Color& col,
                           float depth, float size);  // render.o 0xAAC7D0
};

// ngl debug-quad helpers (ngl.o)
extern int nglGetScreenWidth();
extern int nglGetScreenHeight();
extern void nglInitQuad(nglQuad* quad);
extern void nglSetQuadTex(nglQuad* quad, nglTexture* tex);
extern void nglSetQuadColor(nglQuad* quad, unsigned int c);
extern void nglSetQuadRect(nglQuad* quad, float x1, float y1, float x2,
                           float y2);
extern void nglListAddQuad(nglQuad* quad);
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
        DebugRender::RenderText(((InplaceString*)mPakInfo->longName)->mStr,
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
            bankProgress = nflGetRequestProgress(bank->requestId);
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

// ?CrazyTempMemBorrow@PakManager@@QAEPAXII@Z (streamer.o; stub)
void* PakManager::CrazyTempMemBorrow(unsigned int align, unsigned int size)
{
    (void)align;
    return malloc(size ? size : 1);
}

// ?CrazyTempMemGiveBack@PakManager@@QAEXPAX@Z (streamer.o; stub)
void PakManager::CrazyTempMemGiveBack(void* ptr)
{
    free(ptr);
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

// ea: 0x66C660 (needs InplaceTree::Find; stub until InplaceTree lands)
const PakInfoNode* PakManager::GetPakInfo(const char* long_name) const
{
    (void)long_name;
    return nullptr;
}
TPakId PakManager::SyncLoadPak(EPakType t, const char* path, NumBanks banks)
{
    (void)t; (void)path; (void)banks;
    return PAK_ID_INVALID;
}
TPakId PakManager::SyncLoadPak(const PakInfoNode* cpak)
{
    (void)cpak;
    return PAK_ID_INVALID;
}
void* PakManager::MemAlloc(TPakId id, unsigned int size, bool bUseActorHeap)
{
    (void)id; (void)size; (void)bUseActorHeap;
    return nullptr;
}
void PakManager::FillBanks()
{
    // stub
}
void PakManager::UnloadAll()
{
    // stub
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
void PakManager::SyncUnloadPak(TPakId id)
{
    (void)id;
}
void PakManager::Update(bool calledFromMovie)
{
    (void)calledFromMovie;
}
const PakInfoNode* PakManager::SyncLoadFLI(EPakType t, const char* path)
{
    (void)t; (void)path;
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

// ea: 0x663200
Color32::Color32(unsigned int ic)
{
    i = ic;
}

// ea: 0x663220
unsigned int Color32::to_ulong() const
{
    return i;
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

// ea: 0x6636A0
const StreamZone* ZoneCellDesc::GetZone() const { return mZone; }

// ea: 0x6636B0
const char* StreamZone::GetName() const { return mName.mStr; }

// ea: 0x6636C0
void StreamZone::SetPakInfo(const PakInfoNode* n) { mPakInfo = n; }

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
struct nalAnyPose;
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

// cdResourceDirectory<T> (streamer.o) - static accessors over the nal globals
template <typename T>
struct cdResourceDirectory {
    static tlResourceDirectory<T>* GetDirectory();
    static void SetDirectory(tlResourceDirectory<T>* dir);
};

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
        unsigned int firstEntry =
            *(unsigned int*)((char*)mCurrentApkFileTypeEntry + 0x10);
        mCurrentApkFileEntry =
            (void*)((char*)firstEntry
                    + 4 * offset * (*(int*)((char*)mCurrentApkFileTypeEntry
                                            + 8)
                                    + 1));
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
void stream_free(unsigned char* ptr)
{
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
    if (*numBanks > 0.0f)
    {
        half = true;
        int v9 = 0;
        if (numAvailableBanks + 0.5f > 0.0f)
        {
            bool v10;
            for (;;)
            {
                v10 = freeBanks1->Test(v9);
                if (v10 != freeBanks2->Test(v9))
                    break;
                if (numAvailableBanks + 0.5f <= (float)++v9)
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

// ea: 0x66F0A0
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
    mHeaderRequestId.nflId = buffer[0];
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
    if (numBanks.xbox + 0.5f > 0.0f)
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
    return mem_heap_free_check_reserve(*(mem_heap**)((char*)self + 4), ptr);
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
    requestId.nflId = v7->requestId;
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
            nflRequestInfoStub request_info;
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
                            ((InplaceString*)mPakInfo->longName)->mStr))
                        __debugbreak();
                }
                if (v23 == 0)
                    mLoadStats->readStart = __rdtsc();
                unsigned int compressedSize = bank->compressedSize;
                if (compressedSize != 0)
                {
                    bank->requestId = (unsigned char)*(
                        unsigned char*)PakReadDataAsync(
                            buffer, bank->memptr, bank->size,
                            compressedSize, bank->fileOffset);
                }
                else
                {
                    bank->requestId = (unsigned char)*(
                        unsigned char*)PakReadDataAsync(
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
void DecodeGrassInfo(const char* name, unsigned char* data, int size,
                     TPakId pakId)
{
    (void)name; (void)data; (void)size; (void)pakId;
}
void DecodeSPT(const char* name, unsigned char* data, int size, TPakId pakId)
{
    (void)name; (void)data; (void)size; (void)pakId;
}
void DecodeWIND(const char* name, unsigned char* data, int size, TPakId pakId)
{
    (void)name; (void)data; (void)size; (void)pakId;
}
void DecodeSEED(const char* name, unsigned char* data, int size, TPakId pakId)
{
    (void)name; (void)data; (void)size; (void)pakId;
}

// ea: 0x6663D0
void DecodeCharSkel(const char* name, unsigned char* data)
{
    (void)name;
    cdLoadCharSkelInplace(data);
}

// ea: 0x66BD40
void DecodeMipSettings(const char* name, unsigned char* data, int size,
                       TPakId pakId)
{
    InstanceBankMgr::sInst->DecodeMipsettings(name, data, size, pakId);
}

// ea: 0x66BD60
void DecodeInstanceBank(const char* name, unsigned char* data, int size,
                        TPakId pakId)
{
    InstanceBankMgr::sInst->DecodeInstbank(name, data, size, pakId);
}

// ea: 0x665210
void DecodeDB(const char* name, unsigned char* data, int size, TPakId pakId)
{
    DbTablesetMgr::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x665230
void DecodeXModelBank(const char* name, unsigned char* data, int size,
                      TPakId pakId)
{
    XModelManager::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x665250
void DecodeXModelPartsBank(const char* name, unsigned char* data, int size,
                           TPakId pakId)
{
    XModelPartsManager::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x665270
void DecodeAITypeBank(const char* name, unsigned char* data, int size,
                      TPakId pakId)
{
    AITypeManager::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x665290
void DecodeLightGrid(const char* name, unsigned char* data, int size,
                     TPakId pakId)
{
    LightGridMgr::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x6652C0
void DecodeLevelPath(const char* name, unsigned char* data, int size,
                     TPakId pakId)
{
    PathNodeMgr::sInst->DecodeLevelBank(name, data, size, pakId);
}

// ea: 0x6652E0
void DecodeZonePath(const char* name, unsigned char* data, int size,
                    TPakId pakId)
{
    PathNodeMgr::sInst->DecodeZoneBank(name, data, size, pakId);
}

// ea: 0x665300
void DecodeSTB(const char* name, unsigned char* data, int size, TPakId pakId)
{
    STBManager::sInst->DecodeBank(name, data, size, pakId);
}

// ea: 0x665350
void InstanceBankMgr::ReleaseInstanceBank(TPakId pakId)
{
    if (pakId >= 0 && pakId < 99)
        mEntries[pakId] = nullptr;
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
    // PtrFixupTable::Fixup(this: v2, basePtr: this) - table walk not ported;
    // the tree/vector pointers are already relative and fixed by GetBank/Find.
}

// stub: InplaceTree<T,K>::Find (tree not ported yet) - returns nullptr
static unsigned int* InplaceTreeFindUInt(void* tree, const unsigned int* key)
{
    (void)tree; (void)key;
    return nullptr;
}
static float* InplaceTreeFindFloat(void* tree, const char* const* key)
{
    (void)tree; (void)key;
    return nullptr;
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
    unsigned int* v4 = InplaceTreeFindUInt((void*)&mTree, &hash);
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
    float* v4 = InplaceTreeFindFloat((void*)&bank->mMipSettings, &texture);
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
    InstanceBankMgr::sInst->Add(INSTBANK_TYPE_TEXTURE, pakId, result,
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
PakManager* TogglePakRender()
{
    PakManager::sInst->mDebugRenderMode ^= 1;
    return PakManager::sInst;
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
