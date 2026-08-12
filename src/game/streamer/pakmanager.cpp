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
enum EPakType { kPakTypeGlobal = 0 };
class NumBanks {
public:
    unsigned int mNumBanks;
};

template <typename T, int N>
struct ae_sized_array {
    T m_elements[N];  // +0x00
    int m_size;       // +N*sizeof(T)
};

class PakFile {
public:
    uint8_t _pad[0xFC];
    int mState;  // +0xFC PakFile::EState (0 = loaded, 1 = loading, 2 = unloading)

    // ?MemAlloc@PakFile@@QAEPAXII_N@Z (streamer.o 0x671F10; stub)
    void* MemAlloc(unsigned int align, unsigned int size, bool search_prereqs)
    {
        (void)align; (void)search_prereqs;
        return malloc(size ? size : 1);
    }
};

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
