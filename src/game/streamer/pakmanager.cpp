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
#include "core/mem_heap.h"

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
    kPakTypeCount = 2,
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
        uint8_t _pad[0x0E];
        uint16_t numFiles;  // +0x0E
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
typedef int nflFileID;
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
    uint8_t      _padF0[0xFC - 0xF0];   // mCurrentApk/mCurrentApkFileEntry/mCurrentApkFileTypeEntry
    EState       mState;                // +0xFC
    ELoadingState mLoadingState;        // +0x100
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
    uint8_t _pad[0xBC];
    float distance;       // +0xBC
    float userDistance;   // +0xC0
};

class PakManager {
public:
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
