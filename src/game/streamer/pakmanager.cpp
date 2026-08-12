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
#include "ngl/nglFont.h"

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
extern double atof(const char* nptr);
typedef unsigned nflState;
typedef unsigned nflFileID;
extern void nflUpdate();
extern nflState nflGetState();
extern unsigned int nflReadFile(nflFileID file, unsigned offset, void* buf,
                                unsigned size);
#define NFL_STATE_ERROR 2

// FEManager (shell.o; DrawDiscError only)
class FEManager {
public:
    void DrawDiscError();  // ?DrawDiscError@FEManager@@QAEXXZ (shell.o; stub)
};
// shell.o owns the real symbol; placeholder until shell.o is ported
FEManager g_femanager;

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Warning(const char* fmt, ...);
bool Error(const char* fmt, ...);
}

enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };
#define PAK_ID_INVALID ((TPakId)-1)
#define PAK_ID_MIN ((TPakId)0)
enum EPakType {
    kPakTypeGlobal = 0,
    kPakTypeFrontEnd = 1,
    kPakTypeAnimation = 2,
    kPakTypeCount = 10,
};
class NumBanks {
public:
    unsigned int mNumBanks;
};

template <typename T, int N>
struct ae_sized_array {
    T m_elements[N];  // +0x00
    int m_size;       // +N*sizeof(T)
};

// PakHeader (streamer.o PakFile.h; 48 bytes)
struct PakHeader {
    struct Section {
        char      name[8];     // +0x00
        int       sectionId;   // +0x08
        uint16_t  numBanks;    // +0x0C
        uint16_t  numFiles;    // +0x0E
        void*     banks;       // +0x10 (Section::Bank*)
        void*     files;       // +0x14 (Section::File*)
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

// PakFile (streamer.o PakFile.h; 276 bytes, verified IDA)
class PakFile {
public:
    enum EState { LOADED = 0, LOADING = 1, UNLOADING = 2 };
    enum ELoadingState { LOADING_HEADER = 0, LOADING_DATA = 2 };

    struct TRequestId {
        nflRequestID nflId;  // +0x00
    };

    uint8_t      _pad0[0x08];           // m_dlist_node
    const char*  mCurrDecodeFile;       // +0x08
    uint8_t      _pad0C[0x4C - 0x0C];   // mPath (64)
    EPakType     mPakType;              // +0x4C
    unsigned int mFilesize;             // +0x50
    nflFileID    mFileId;               // +0x54
    uint8_t      _pad58[0x78 - 0x58];   // mBankAlloc/mSerializedAlloc
    TPakId       mPakId;                // +0x78
    const PakInfoNode* mPakInfo;        // +0x7C
    PakHeader*   mHeader;               // +0x80
    int          mDefaultSectionIdx;    // +0x84
    unsigned char* mHeaderBuffer;       // +0x88
    TRequestId   mHeaderRequestId;      // +0x8C
    uint8_t      _pad90[0xE8 - 0x90];   // mApkFiles/mCloseHandle/mOnlyLoadHeader/mHeapList/mPrereqHeaps
    unsigned int mCurrentFile;          // +0xE8
    unsigned char* mCurrentFilePtr;     // +0xEC
    void*        mCurrentApk;           // +0xF0
    void*        mCurrentApkFileEntry;  // +0xF4
    void*        mCurrentApkFileTypeEntry;  // +0xF8
    EState       mState;                // +0xFC
    ELoadingState mLoadingState;        // +0x100
    enum ELoadingStateAll { LOADING_DONE = 3 };
    uint8_t      _pad104[0x110 - 0x104];  // mLooseFiles
    LoadStats*   mLoadStats;            // +0x110

    static unsigned char* sHeaderBuffer;  // ?sHeaderBuffer@PakFile@@0PAEA
    static bool sHeaderBufferUsed;        // ?sHeaderBufferUsed@PakFile@@0_NA

    // ?MemAlloc@PakFile@@QAEPAXII_N@Z (streamer.o 0x671F10; stub)
    void* MemAlloc(unsigned int align, unsigned int size, bool search_prereqs)
    {
        (void)align; (void)search_prereqs;
        return malloc(size ? size : 1);
    }

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
    uint8_t     _pad0C[0xB4 - 0x0C];
    TPakId      pakId;          // +0xB4
    unsigned int refCount;      // +0xB8
    float       distance;       // +0xBC
    float       userDistance;   // +0xC0
    unsigned int mapColor;      // +0xC4
    float       computedDistance;  // +0xC8
    unsigned int visited;       // +0xCC
    uint8_t     _padD0[0xE0 - 0xD0];
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
    void* mPakInfoBank;               // +0x18
    void* mLevelPakInfoBank;          // +0x1C
    uint8_t _pad20[0x24 - 0x20];
    TPakId mCurrentPakId;             // +0x24
    uint8_t _pad28[0x30 - 0x28];
    TPakId mGlobalPakId;              // +0x30
    uint8_t _pad34[0x38 - 0x34];
    TPakId mLevelPakId;               // +0x38
    uint8_t _pad3C[0x40 - 0x3C];
    PakFile* mSlots[99];              // +0x40
    uint8_t _pad1CC[0x364 - (0x40 + 99 * 4)];
    void (*mProgressCallback)(float); // +0x364
    uint8_t _pad368[0x4384 - 0x368];
    int mEnabled;                     // +0x4384
    uint8_t _pad4388[0x4398 - 0x4388];
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
    // - ea: 0x6655C0 (stub until PakFile/BankManager land)
    TPakId FindPakId(EPakType t) const;
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

// InstanceBankMgr (streamer.o; mEntries[99] @ +0x34)
class InstanceBankMgr {
public:
    uint8_t _pad[0x34];
    void*   mEntries[99];  // +0x34 (InstanceBankSet*[99])

    void ReleaseInstanceBank(TPakId pakId);  // ?ReleaseInstanceBank@InstanceBankMgr@@QAEXW4TPakId@@@Z
};

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

// StreamZoneManager (streamer.o; mDebugRenderMode +0x190, mInitialPosition +0x1A0)
class StreamZoneManager {
public:
    uint8_t _pad[0x190];
    struct {
        unsigned int mEnabled : 1;  // bit 0
        float zoneGraphScale;       // +0x194
    } mDebugRenderMode;             // +0x190
    math::Position3 mInitialPosition;  // +0x1A0

    static StreamZoneManager* sInst;  // defined in sv_globals.cpp
    void SetInitialPosition(const math::Position3& pos);
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

// Stubs for PakManager members that need PakFile/BankManager/InplaceTree.
TPakId PakManager::FindPakId(EPakType t) const
{
    (void)t;
    return PAK_ID_INVALID;
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
const PakInfoNode* PakManager::GetPakInfo(TPakId pakId) const
{
    (void)pakId;
    return nullptr;
}
const PakInfoNode* PakManager::GetPakInfo(const char* long_name) const
{
    (void)long_name;
    return nullptr;
}
void* PakManager::MemAlloc(TPakId id, unsigned int size, bool bUseActorHeap)
{
    (void)id; (void)size; (void)bUseActorHeap;
    return nullptr;
}
void PakManager::MemFree(TPakId id, void* ptr, bool bUseActorHeap)
{
    (void)id; (void)ptr; (void)bUseActorHeap;
}
void* PakManager::MemAlign(TPakId id, unsigned int align, unsigned int size)
{
    (void)id; (void)align; (void)size;
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
    (void)user_distances_also;
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

// ============================================================================
// PakFile small accessors (streamer.o PakFile.cpp)
// ============================================================================

unsigned char* PakFile::sHeaderBuffer = nullptr;
bool PakFile::sHeaderBufferUsed = false;

// ea: 0x664B90
float PakFile::GetLoadTime() const
{
    LoadStats* mLoadStats = this->mLoadStats;
    if (mLoadStats != nullptr)
        return mLoadStats->total;
    return 0.0f;
}

// streamer.o PakHeader helpers (cross-object; stub until PakHeader ports)
void PakHeader::FixDown() {}
void PakHeader::FixUp(bool persistentFixup, int doByteSwap)
{
    (void)persistentFixup; (void)doByteSwap;
}

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
