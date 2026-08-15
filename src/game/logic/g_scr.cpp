// ============================================================================
// g_scr.cpp - script integration wrappers (g.o: g_scr_main.cpp / g_spawn.cpp)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <new>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/PoolAllocator.h"
#include "core/tlFixedString.h"
#include "core/ae_fixed_string.h"
#include "core/ae_array.h"
#include "game/AeThreadFunctor.h"
#include "game/nextgen/nextgen.h"

extern PoolAllocator* gCommonPoolAllocator;  // ?gCommonPoolAllocator@@3PAVPoolAllocator@@A (core.o)
extern void* mem_heap_malloc(unsigned int size);  // ?mem_heap_malloc@@YAPAXI@Z
extern bool g_indoor;                              // ?g_indoor@@3_NA (core.o)
extern float CG_GetNorthDirection();               // ?CG_GetNorthDirection@@YAMXZ (cg.o)
extern void G_FlushCorpses();                      // ?G_FlushCorpses@@YAXXZ (mp_actors.o)
extern void FX_SetRainDrops(bool on);              // ?FX_SetRainDrops@@YAX_N@Z (render.o)

namespace AeStringSupport {
void Concat(char* dst, int& dstLen, int dstCapacity,
            const char* src);  // ?Concat@AeStringSupport@@YAXPADAAHHPBD@Z
}

namespace ShaderCommon {
extern bool gGlowGodRays;  // ?gGlowGodRays@ShaderCommon@@3_NA
extern int  gGlowPasses;   // ?gGlowPasses@ShaderCommon@@3HA
extern float gGlowIntensity;  // ?gGlowIntensity@ShaderCommon@@3MA
extern float gGlowExpansion;  // ?gGlowExpansion@ShaderCommon@@3MA
}

namespace View {
bool IsSplitScreen();  // ?IsSplitScreen@View@@YA_NXZ (cg.o)
}

namespace BrocHelper {
int m_treeCount;  // ?m_treeCount@BrocHelper@@3HA (scr.o @ 0x1329E78)
void SetLoadedTrees(int num);  // ?SetLoadedTrees@BrocHelper@@YAXH@Z
int  GetLoadedTrees();         // ?GetLoadedTrees@BrocHelper@@YAHXZ
void AnimationToBroLookup(const tlFixedString& tree_name, int tree_index,
                          const tlFixedString& animation_name,
                          int animation_index);  // ?AnimationToBroLookup@BrocHelper@@YAXABVtlFixedString@@H0H@Z
}

// ea: 0x005BE020
void BrocHelper::SetLoadedTrees(int num)
{
    BrocHelper::m_treeCount = num;
}

// ea: 0x005BE030
int BrocHelper::GetLoadedTrees()
{
    return BrocHelper::m_treeCount;
}

// ea: 0x005BE0D0
void BrocHelper::AnimationToBroLookup(const tlFixedString& tree_name,
                                      int tree_index,
                                      const tlFixedString& animation_name,
                                      int animation_index)
{
    gpBrocAPI->mBrocExports.mAnimIndexResolver(
        tree_name.hash, tree_index, animation_name.hash, animation_index);
}

extern void __fastcall Sentient_SetGoalRadius(sentient_s* pSelf, float fRadius);  // ?Sentient_SetGoalRadius@@YIXPAUsentient_s@@M@Z
extern void __fastcall Sentient_SetGoalAngleTolerance(sentient_s* pSelf, float fTolerance);  // ?Sentient_SetGoalAngleTolerance@@YIXPAUsentient_s@@M@Z
extern bool g_controllerConnectedErrorShown[4];  // ?g_controllerConnectedErrorShown@@3PA_NA (game2.o)
extern bool gNANO_Animate;                       // ?gNANO_Animate@@3_NA (g.o)

extern void tlPrintf(const char* fmt, ...);  // ?tlPrintf@@YAXPBDZZ (core.o)
extern void tlFatal(const char* fmt, ...);   // ?tlFatal@@YAXPBDZZ (core.o)

namespace AeAssert {
extern bool gAssertsEnabled;  // ?gAssertsEnabled@AeAssert@@3_NA (core_xboxr)
}

// Binary BrocSys.cpp forward-declared hudelem as class (mangle ABVhudelem@Broc)
namespace Broc {
class hudelem;
}

// ============================================================================
// AeThread + state controllers (scr.o / AeThread.cpp) - IDA types 5428-5437
// ============================================================================
// Local layout views (core_systems.h cannot be included alongside g_local.h)
struct AeDListNode {
    AeDListNode* mNext;  // +0x00
    AeDListNode* mPrev;  // +0x04
};
struct AeStateList {  // reserved_dlist<AeThreadState> layout
    int          m_size;  // +0x00
    AeDListNode* m_head;  // +0x04
    AeDListNode* m_end;   // +0x08
    AeDListNode* m_tail;  // +0x0C (points at m_head's slot via trick)
};
struct AeThreadFlagWord {  // Bitmask<unsigned int> layout
    unsigned int mMask;    // +0x00
};

// EntityNotify local view (full type in core_systems.h; 0x14 bytes)
class EntityNotifyLocal {
public:
    AeDListNode m_dlist_node;  // +0x00
    unsigned int mStr;         // +0x08
    DbLinkedHandle<EntityHandleDb, Entity> mOwner;  // +0x0C
    void* mParam;              // +0x10
    ~EntityNotifyLocal();      // ?~EntityNotify@@QAE@XZ (g.o)
    static PoolAllocator* sAllocator;  // ?sAllocator@EntityNotify@@0PAVPoolAllocator@@A @ 0xF00E28
};

// ae_pair (class tag V per binary mangling; same shape as g_accessors.cpp)
template <typename T1, typename T2>
class ae_pair {
public:
    T1 m_first;   // +0x00
    T2 m_second;  // +0x04

    ae_pair() : m_first(), m_second() {}
    ae_pair(const T1& f, const T2& s) : m_first(f), m_second(s) {}
};

// BrocDtorBase (mp_level.xboxd; vtable[0] = Destroy)
class BrocDtorBase {
public:
    virtual void Destroy(void* inst);  // ?Destroy@BrocDtorBase@@UAEXPAX@Z
};

class AeThreadState;
class AeThread {
public:
    struct BackupStack {
        friend struct BackupStack;
        struct Block {
            uint8_t mBuff[252];  // +0x00
            Block*  mNext;       // +0xFC

        private:
            static PoolAllocator* sAllocator;   // scr.o @ 0x132A0CC
            friend struct BackupStack;
        public:
            static PoolAllocator* GetAllocator();  // g.o
            static void SetupAllocator();          // scr.o 0x5C9340
        };

        Block*       mBlockList;  // +0x00
        unsigned int mSize;       // +0x04
        unsigned int mChecksum;   // +0x08
        unsigned int mBegin;      // +0x0C
        unsigned int mEnd;        // +0x10

        BackupStack();                            // scr.o 0x5BBDB0
        ~BackupStack();                           // scr.o 0x5BC0B0
        void Backup(unsigned int stackBegin, unsigned int stackEnd);  // 0x5BBDC0
        void Restore(unsigned int stackBegin) const;                  // 0x5BBF70
    };

    struct BrocObjCreated {
        ae_sized_array<ae_pair<void*, unsigned int>, 15> list;  // +0x00
        BrocObjCreated* next;                                  // +0x7C
    };

    AeDListNode m_dlist_node;                           // +0x00
    DbLinkedHandle<EntityHandleDb, Entity> mOwner;      // +0x08
    AeThreadFunctor* mFunctor;                          // +0x0C
    AeThreadFlagWord mFlags;                            // +0x10
    Handle mHandle;                                     // +0x14
    AeStateList mStateControllers;                      // +0x18
    BrocObjCreated* mBrocCreated;                       // +0x28
    unsigned int mStackStart;                           // +0x2C
    BackupStack mBackupStack;                           // +0x30
    const char* mFuncName;                              // +0x44
    const char* mFile;                                  // +0x48
    int mLine;                                          // +0x4C

    void SetKillFlag();  // ?SetKillFlag@AeThread@@QAEXXZ (scr.o 0x5C1F00)
    void Kill();         // ?Kill@AeThread@@QAEXXZ (scr.o 0x5C1F10)
    void RegisterBrocInst(void* inst, BrocDtorBase* dtor);  // scr.o 0x5C9400
    void RegisterBrocDtor(void* inst);                      // scr.o 0x5C94C0
    void DestroyBrocInsts();                                // scr.o 0x5DAF90
    ~AeThread();                                            // scr.o 0x5DAE30
    AeThread(const char* file, int line, const char* func,
             unsigned int ehandle, AeThreadFunctor* ftor,
             bool bUseScratchpad);                          // scr.o 0x5C79D0
    void Sleep(AeThreadState* stateController);             // scr.o 0x5C7AC0

private:
    static PoolAllocator* sAllocator;  // ?sAllocator@AeThread@@0PAVPoolAllocator@@A @ 0x132A0C4
    friend class AeThreadManager;
public:
    static unsigned int* sBackup;      // ?sBackup@AeThread@@2PAIA @ 0x1329C7C
};

class AeThreadState {
public:
    enum EAction : int {
        kActionNone = 0,
        kActionSleep = 1,
        kActionWakeUp = 2,
        kActionTerminate = 3,
    };

    virtual ~AeThreadState() {}
    virtual EAction NewAction(AeThread& t) = 0;  // UAE?AW4EAction@AeThreadState@@
    virtual void GetCondText(ae_fixed_string<64, unsigned char>& str) = 0;
    virtual void GetDebugTxt(ae_fixed_string<64, unsigned char>& str) = 0;

    AeDListNode m_dlist_node;                                // +0x04
    bool mFinished;                                          // +0x0C
    EAction mResult;                                         // +0x10
};

struct AeThreadWaitState : AeThreadState {
    float mTimeRemaining;  // +0x14

    AeThreadWaitState(float t);  // ??0AeThreadWaitState@@QAE@M@Z (scr.o 0x5C1FB0)
    EAction NewAction(AeThread& t) override;  // scr.o 0x5BC150
    void GetCondText(ae_fixed_string<64, unsigned char>& str) override;  // 0x5C1FE0
    void GetDebugTxt(ae_fixed_string<64, unsigned char>& str) override;  // 0x5C7BB0
};

struct AeThreadWaitFramesState : AeThreadState {
    int mFramesRemaining;  // +0x14

    AeThreadWaitFramesState(int numFrames);  // ??0AeThreadWaitFramesState@@QAE@H@Z (0x5C2030)
    EAction NewAction(AeThread& t) override;  // 0x5BC180
    void GetCondText(ae_fixed_string<64, unsigned char>& str) override;  // 0x5C2060
    void GetDebugTxt(ae_fixed_string<64, unsigned char>& str) override;  // 0x5C7BE0
};

struct AeThreadPakNotifyState : AeThreadState {
    enum ePakState : int {
        kPakStateUnloaded = 0,
        kPakStateLoaded = 1,
    };

    const PakInfoNode* mPak;      // +0x14
    ePakState mWaitState;         // +0x18

    AeThreadPakNotifyState(const PakInfoNode* pak, ePakState state);  // 0x5C1F80
    EAction NewAction(AeThread& t) override;  // 0x5BC0F0
    void GetCondText(ae_fixed_string<64, unsigned char>& str) override;  // 0x5C7B00
    void GetDebugTxt(ae_fixed_string<64, unsigned char>& str) override;  // 0x5C7B50
};

// PakInfoNode view (streamer.o; full layout in pakmanager.cpp)
static_assert(sizeof(AeThread::BackupStack) == 0x14,
              "BackupStack size mismatch");
static_assert(sizeof(AeThread::BackupStack::Block) == 0x100,
              "BackupStack::Block size mismatch");

PoolAllocator* AeThread::BackupStack::Block::sAllocator;
PoolAllocator* AeThread::BackupStack::Block::GetAllocator()
{
    return AeThread::BackupStack::Block::sAllocator;
}

// scr.o data @ 0x11E3E00 / 0x132A0D0 (init -1 per IDA bytes)
int g_AeThread_minStackSize = -1;
int g_AeThread_maxStackSize = -1;
// file-static counter (no public symbol)
static unsigned int sRestoreId = 0;

// ea: 0x005BBDB0
AeThread::BackupStack::BackupStack()
{
    mBlockList = nullptr;
    mSize = 0;
    mBegin = 0;
    mEnd = 0;
}

// ea: 0x005BC0B0
AeThread::BackupStack::~BackupStack()
{
    while (mBlockList != nullptr)
    {
        Block* next = mBlockList->mNext;
        Block::sAllocator->Release(mBlockList);
        mBlockList = next;
    }
}

// ea: 0x005BBDC0 (disasm 5BBE80-5BBF23: 0xFC-byte block copy + checksum)
void AeThread::BackupStack::Backup(unsigned int stackBegin,
                                   unsigned int stackEnd)
{
    if (stackEnd <= stackBegin)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AeThread.cpp";
        AeAssert::gCurrentLine = 965;
        AeAssert::gCurrentExpr = "stackEnd > stackBegin";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("argument mismatch"))
            __debugbreak();
    }
    unsigned int size = stackEnd - stackBegin;
    if (size < (unsigned int)g_AeThread_minStackSize)
        g_AeThread_minStackSize = (int)size;
    if (size > (unsigned int)g_AeThread_maxStackSize)
        g_AeThread_maxStackSize = (int)size;
    mChecksum = 0;
    if (mBlockList == nullptr)
    {
        Block* v6 = (Block*)Block::sAllocator->Allocate(0x100, false);
        if (v6 != nullptr)
            v6->mNext = nullptr;
        else
            v6 = nullptr;
        mBlockList = v6;
    }
    Block* block = mBlockList;
    unsigned int stackPtr = stackBegin;
    unsigned int remaining = size;
    while (remaining != 0)
    {
        unsigned int chunk = remaining >= 0xFC ? 0xFC : remaining;
        if (chunk > 0)
        {
            unsigned int n = ((chunk - 1) >> 2) + 1;
            unsigned int* dst = (unsigned int*)block->mBuff;
            unsigned int* src = (unsigned int*)stackPtr;
            for (unsigned int k = 0; k < n; ++k)
                dst[k] = src[k];
        }
        for (unsigned int i = 0; i < chunk; ++i)
            mChecksum += block->mBuff[i];
        remaining -= chunk;
        stackPtr += chunk;
        if (remaining != 0 && block->mNext == nullptr)
        {
            Block* v12 = (Block*)Block::sAllocator->Allocate(0x100, false);
            if (v12 != nullptr)
                v12->mNext = nullptr;
            else
                v12 = nullptr;
            block->mNext = v12;
        }
        Block* prev = block;
        block = block->mNext;
        if (remaining == 0 && prev != nullptr)
            prev->mNext = nullptr;
    }
    while (block != nullptr)
    {
        Block* v14 = block;
        block = block->mNext;
        Block::sAllocator->Release(v14);
    }
    mSize = stackEnd - stackBegin;
    mBegin = stackBegin;
    mEnd = stackEnd;
}

// ea: 0x005BBF70
void AeThread::BackupStack::Restore(unsigned int stackBegin) const
{
    if (mBlockList == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AeThread.cpp";
        AeAssert::gCurrentLine = 1049;
        AeAssert::gCurrentExpr = "mBlockList";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can't restore unintialized stack!"))
            __debugbreak();
    }
    Block* block = mBlockList;
    unsigned int mSize = this->mSize;
    unsigned int stackPtr = stackBegin;
    unsigned int restoreSize = mSize;
    ++sRestoreId;
    unsigned int checksum = 0;
    while (mSize != 0)
    {
        unsigned int chunk = mSize >= 0xFC ? 0xFC : mSize;
        if (chunk > 0)
        {
            unsigned int n = ((chunk - 1) >> 2) + 1;
            unsigned int* dst = (unsigned int*)stackPtr;
            unsigned int* src = (unsigned int*)block->mBuff;
            for (unsigned int k = 0; k < n; ++k)
                dst[k] = src[k];
            mSize = restoreSize;
        }
        for (unsigned int i = 0; i < chunk; ++i)
            checksum += block->mBuff[i];
        mSize -= chunk;
        stackPtr += chunk;
        restoreSize = mSize;
        if (mSize == 0)
            break;
        block = block->mNext;
    }
    if (checksum != mChecksum)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AeThread.cpp";
        AeAssert::gCurrentLine = 1100;
        AeAssert::gCurrentExpr = "checksum == mChecksum";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Checksum bad!"))
            __debugbreak();
    }
}

// ea: 0x005C9340 (PoolConfig 0x100/0x40/0x190 per disasm)
void AeThread::BackupStack::Block::SetupAllocator()
{
    ae_sized_array<PoolAllocator::PoolConfig, 16> poolCfg;
    memset(&poolCfg, 0, sizeof(poolCfg));
    poolCfg.m_size = 0;
    PoolAllocator::PoolConfig elt;
    elt.blockSize = 0x100;
    elt.blockAlign = 0x40;
    elt.numBlocks = 0x190;
    elt.block = nullptr;
    poolCfg.push_back(elt);
    void* block = mem_heap_malloc(0x3C);
    if (block != nullptr)
        Block::sAllocator = new (block) PoolAllocator(poolCfg, 1);
    else
        Block::sAllocator = nullptr;
}

// ea: 0x005C1F00
void AeThread::SetKillFlag()
{
    mFlags.mMask |= 0x40;
    mFlags.mMask |= 0x8;
}

// ea: 0x005C1F10
void AeThread::Kill()
{
    unsigned int v1 = mFlags.mMask | 8;
    mFlags.mMask = v1;
    v1 |= 0x40;
    mFlags.mMask = v1;
    mFlags.mMask = v1 | 4;
}

// ea: 0x005C1FB0
AeThreadWaitState::AeThreadWaitState(float t)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mFinished = false;
    mResult = AeThreadState::kActionWakeUp;
    mTimeRemaining = t;
}

// ea: 0x005BC150
AeThreadState::EAction AeThreadWaitState::NewAction(AeThread& /*t*/)
{
    float v2 = mTimeRemaining - ServerTime::sInst.mTickDelta;
    mTimeRemaining = v2;
    if (v2 > 0.0f)
        return AeThreadState::kActionNone;
    mFinished = true;
    return AeThreadState::kActionWakeUp;
}

// ea: 0x005C1FE0
void AeThreadWaitState::GetCondText(ae_fixed_string<64, unsigned char>& str)
{
    ae_formatted_string<64, unsigned char> v5(",(s) %04.3f", mTimeRemaining);
    int len = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len, 63,
                            (const char*)v5.mBuff);
    str.mLength = (unsigned char)len;
}

// ea: 0x005C7BB0
void AeThreadWaitState::GetDebugTxt(ae_fixed_string<64, unsigned char>& str)
{
    int len = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len, 63, "waking up from time wait");
    str.mLength = (unsigned char)len;
}

// ea: 0x005C2030
AeThreadWaitFramesState::AeThreadWaitFramesState(int numFrames)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mFinished = false;
    mResult = AeThreadState::kActionWakeUp;
    mFramesRemaining = numFrames;
}

// ea: 0x005BC180
AeThreadState::EAction AeThreadWaitFramesState::NewAction(AeThread& /*t*/)
{
    int v2 = mFramesRemaining - 1;
    bool wasOne = mFramesRemaining == 1;
    mFramesRemaining = v2;
    if (v2 >= 0 && !wasOne)
        return AeThreadState::kActionNone;
    mFinished = true;
    return AeThreadState::kActionWakeUp;
}

// ea: 0x005C2060
void AeThreadWaitFramesState::GetCondText(
    ae_fixed_string<64, unsigned char>& str)
{
    ae_formatted_string<64, unsigned char> v5(",asleep for %04d frames",
                                              mFramesRemaining);
    int len = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len, 63,
                            (const char*)v5.mBuff);
    str.mLength = (unsigned char)len;
}

// ea: 0x005C7BE0
void AeThreadWaitFramesState::GetDebugTxt(
    ae_fixed_string<64, unsigned char>& str)
{
    int len = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len, 63, "waking up from frame_wait");
    str.mLength = (unsigned char)len;
}

// ea: 0x005C1F80
AeThreadPakNotifyState::AeThreadPakNotifyState(const PakInfoNode* pak,
                                               ePakState state)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mFinished = false;
    mResult = AeThreadState::kActionWakeUp;
    mPak = pak;
    mWaitState = state;
}

// ea: 0x005BC0F0
AeThreadState::EAction AeThreadPakNotifyState::NewAction(AeThread& /*t*/)
{
    TPakId pakId = *(TPakId*)((char*)mPak + 0xB4);
    if ((mWaitState != kPakStateLoaded
         || !PakManager::sInst->IsLoaded(pakId))
        && (mWaitState != kPakStateUnloaded
            || !PakManager::sInst->IsUnloaded(pakId)))
    {
        return AeThreadState::kActionNone;
    }
    AeThreadState::EAction result = mResult;
    mFinished = true;
    return result;
}

// ea: 0x005C7B00
void AeThreadPakNotifyState::GetCondText(
    ae_fixed_string<64, unsigned char>& str)
{
    int len = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len, 63, ",waitpak ");
    str.mLength = (unsigned char)len;
    const char* mStr = *(const char**)((char*)mPak + 4);  // longName.mStr
    int len2 = len;
    AeStringSupport::Concat((char*)str.mBuff, len2, 63, mStr);
    str.mLength = (unsigned char)len2;
}

// ea: 0x005C7B50
void AeThreadPakNotifyState::GetDebugTxt(
    ae_fixed_string<64, unsigned char>& str)
{
    if (mResult == AeThreadState::kActionWakeUp)
    {
        int len = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len, 63, "waking up for ");
        str.mLength = (unsigned char)len;
        const char* mStr = *(const char**)((char*)mPak + 4);  // longName.mStr
        int len2 = len;
        AeStringSupport::Concat((char*)str.mBuff, len2, 63, mStr);
        str.mLength = (unsigned char)len2;
    }
}


extern void EffectEventPlayQueuedEffect(Handle effect);  // ?EffectEventPlayQueuedEffect@@YAXVHandle@@@Z (game.o)
extern bool EffectEventIsPlaying(Handle effect);         // ?EffectEventIsPlaying@@YA_NVHandle@@@Z (game.o)
extern void EffectEventStopEmitting(Handle effect);      // ?EffectEventStopEmitting@@YAXVHandle@@@Z (game.o)
extern void EffectEventFF(Handle effect, float deltaT);  // ?EffectEventFF@@YAXVHandle@@M@Z (game.o)

class AnimNotifyTask {
public:
    static void RegisterFunc(const char* pKey, void (__cdecl* cbFunc)(Broc::entity));
    // ?RegisterFunc@AnimNotifyTask@@SAXPBDP6AXVentity@Broc@@@Z@Z
};

extern const char* nodeStringTable[0x13];  // ?nodeStringTable@@3PAPBDA (mp_actors.o @ 0xE37A20)
extern float gProjShadowAlpha;             // ?gProjShadowAlpha@@3MA (render.o)
extern float gProjShadowSize;              // ?gProjShadowSize@@3MA (render.o)

struct IGOCompassWidget {
    void SetHideCompassStar(int active, int index);  // ?SetHideCompassStar@IGOCompassWidget@@QAEXHH@Z
    void SetHideUpdatedText(int active, int index);  // ?SetHideUpdatedText@IGOCompassWidget@@QAEXHH@Z
    static int ObjectiveStateIndexFromString(const char* name);  // ?ObjectiveStateIndexFromString@IGOCompassWidget@@SAHPBD@Z
};

extern actor_s* __fastcall Actor_Get(DbLinkedHandle<EntityHandleDb, Entity> ent);  // ?Actor_Get@@YIPAUactor_s@@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z (mp_actors.o)

extern void CG_Fade(int r, int g, int b, int a, int time, int duration,
                    int viewport);  // ?CG_Fade@@YAXHHHHHHH@Z (cg.o)
extern bool XAnimNotetrackExists(scr_anim_s anim,
                                 const unsigned int& name);  // ?XAnimNotetrackExists@@YA_NUscr_anim_s@@ABI@Z
extern const char* Com_SurfaceTypeToName(int iTypeIndex);  // core.o

// MusicMgr view (game.o; class lives in g_entity_misc.cpp)
class MusicMgr {
public:
    static MusicMgr* sInst;  // ?sInst@MusicMgr@@2PAV1@A
    void Play(const char* name);          // ?Play@MusicMgr@@QAEXPBD@Z
    void Stop(float fadeOutTime);         // ?Stop@MusicMgr@@QAEXM@Z
    void StopIndoor(float fadeOutTime);   // ?StopIndoor@MusicMgr@@QAEXM@Z
};

// CurveManager view (game.o; class lives in g_cmd.cpp)
typedef float (__cdecl* CurveEvalFunc)(unsigned int, unsigned int, unsigned int,
                                       float, float, unsigned int);
class CurveManager {
public:
    static CurveManager* sInst;  // ?sInst@CurveManager@@2PAV1@A
    void AddKeyFunc(unsigned int type, CurveEvalFunc function);        // ?AddKeyFunc@CurveManager@@QAEXIP6AMIIIMMI@Z@Z
    void AddConditionFunc(unsigned int type, CurveEvalFunc function);  // ?AddConditionFunc@CurveManager@@QAEXIP6AMIIIMMI@Z@Z
};

// ?gpBrocAPI@@3PAUBrocAPI@@A (scr.o data @ 0xF3ABDC, BSS)
BrocAPI* gpBrocAPI = NULL;

// ?currentVM@@3PAUvm_s@@A (scr.o data @ 0xF3AC04)
vm_s* currentVM = NULL;

// ea: 0x0077D020 (mp_actors.o)
void __fastcall Sentient_GetOrigin(sentient_s* pSelf, float* const vOriginOut)
{
    if (pSelf == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 246;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 247;
        AeAssert::gCurrentExpr = "pSelf->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->actor == NULL && pSelf->pEnt->client == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 248;
        AeAssert::gCurrentExpr = "pSelf->pEnt->actor || pSelf->pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vOriginOut == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 249;
        AeAssert::gCurrentExpr = "vOriginOut";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vOriginOut[0] = pSelf->pEnt->r.currentOrigin.v.m128_f32[0];
    vOriginOut[1] = pSelf->pEnt->r.currentOrigin.v.m128_f32[1];
    vOriginOut[2] = pSelf->pEnt->r.currentOrigin.v.m128_f32[2];
}

// ea: 0x005C1DE0
int VM_Call(vm_s* vm, int callnum, ...)
{
    if (vm == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\vm.cpp";
        AeAssert::gCurrentLine = 320;
        AeAssert::gCurrentExpr = "vm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vm_s* oldVM = currentVM;
    int (*entryPoint)(int, ...) = vm->entryPoint;
    currentVM = vm;
    if (entryPoint == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\vm.cpp";
        AeAssert::gCurrentLine = 326;
        AeAssert::gCurrentExpr = "vm->entryPoint";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int args[16];
    int* p_callnum = &callnum;
    unsigned int v4 = 0;
    do
    {
        args[v4++] = p_callnum[1];
        ++p_callnum;
    } while (v4 < 0x10);
    int result = entryPoint(callnum, args[0], args[1], args[2], args[3],
                            args[4], args[5], args[6], args[7], args[8],
                            args[9], args[10], args[11], args[12], args[13],
                            args[14], args[15]);
    currentVM = oldVM;
    return result;
}

// ea: 0x005C1D30
void VM_Free(vm_s* vm)
{
    if (vm->dllHandle == NULL)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\vm.cpp";
        AeAssert::gCurrentLine = 255;
        AeAssert::gCurrentExpr = "vm->dllHandle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    memset(vm, 0, 0x8C);
    currentVM = NULL;
}

// ea: 0x005BE2B0 (scr.o)
void UpdateEntityHash(Entity* ent)
{
    ent->mClassNameHash = HashString(ent->mClassName);
    ent->mGroupNameHash = HashString::CalcHash(
        ent->mGroupName.mBlock != nullptr ? (const char*)(ent->mGroupName.mBlock + 1)
                                          : (const char*)&"");
    ent->targetnameHash = HashString::CalcHash(
        ent->targetname.mBlock != nullptr ? (const char*)(ent->targetname.mBlock + 1)
                                          : (const char*)&"");
    ent->mTargetHash = HashString::CalcHash(
        ent->mTarget.mBlock != nullptr ? (const char*)(ent->mTarget.mBlock + 1)
                                       : (const char*)&"");
    ent->mScriptNoteworthyHash = HashString::CalcHash(
        ent->mScriptNoteworthy.mBlock != nullptr
            ? (const char*)(ent->mScriptNoteworthy.mBlock + 1)
            : (const char*)&"");
    ent->mAnimNameHash = HashString::CalcHash(
        ent->mAnimName.mBlock != nullptr ? (const char*)(ent->mAnimName.mBlock + 1)
                                         : (const char*)&"");
}

// ============================================================================
// AnimBankManager / AnimBank
// ============================================================================
struct AnimBank {
    InplaceVector<XAnimEntry> anims;  // +0x00
};
class AnimBankManager {
public:
    static AnimBankManager* sInst;  // 0xF25A34
    AnimBank* GetBank(TPakId pakId);
};
AnimBankManager* AnimBankManager::sInst;  // ?sInst@AnimBankManager@@2PAV1@A (anim.o @ 0x1314F34)

// ============================================================================
// saveField_t - script save fields (8 bytes) - verified against IDA
// ============================================================================
enum saveFieldtype_t {
    SVF_NONE = 0,
    SVF_BROCSTR = 0x10,
};
struct saveField_t {
    int               ofs;   // +0x00
    saveFieldtype_t   type;  // +0x04
};
static_assert(sizeof(saveField_t) == 0x8, "saveField_t size mismatch");

saveField_t sentientFields[128];  // ?sentientFields@@3PAUsaveField_t@@A (mp_actors.o)
saveField_t actorFields[128];     // ?actorFields@@3PAUsaveField_t@@A (mp_actors.o)
extern int g_xanim_num;                 // 0xF3A778
unsigned char sConstsLoaded;     // 0xEF357B
const char* gHashStringTblTxt[173];  // ?gHashStringTblTxt (scr.o @ 0xDD6488)
extern void Scr_FreePrecachedAnimTrees();
extern void GScr_LoadScriptsAndAnimsForEntities();
extern void Scr_PrecacheAnimTrees(void* (*Alloc)(int), bool restart);
extern void* Hunk_AllocXAnimCreate(int size);
extern AnimTree* Scr_GetAnimTreeByName(const char* treename);
#ifndef PAK_ID_MIN
#define PAK_ID_MIN ((TPakId)0)
#endif

// ea: 0x00449350
void GScr_GetStartOrigin()
{
    ;
}

// ea: 0x00449360
void GScr_GetStartAngles()
{
    ;
}

// ea: 0x00449370
void GScr_GetCycleOriginOffset()
{
    ;
}

// ea: 0x0044CAE0
void Scr_FreeFields(const saveField_t* fields, unsigned char* base)
{
    if (fields->type != SVF_NONE)
    {
        const saveFieldtype_t* p_type = &fields->type;
        int v6;
        do
        {
            if (*p_type == SVF_BROCSTR)
            {
                int v3 = *(p_type - 1);
                Broc::string::Block* v4 = *(Broc::string::Block**)&base[v3];
                if (v4 != nullptr)
                {
                    v4->DecrementCount();
                    *(Broc::string::Block**)&base[v3] = nullptr;
                }
            }
            v6 = *(p_type + 2);
            p_type += 2;
        } while (v6 != 0);
    }
}

// ea: 0x0044CB30
void Scr_FreeSentientFields(sentient_s* pSentient)
{
    Scr_FreeFields(sentientFields, (unsigned char*)pSentient);
}

// ea: 0x0044CB50
void Scr_FreeActorFields(actor_s* pActor)
{
    Scr_FreeFields(actorFields, (unsigned char*)pActor);
}

// ea: 0x0044CB70
HashString GScr_AllocHash(const char* s)
{
    HashString result;
    result.mHash = HashString::CalcHash(s);
    BrocSys::RegisterHashString(s);
    return result;
}

// ea: 0x0044CBA0
void GScr_LoadConsts()
{
    if (!sConstsLoaded)
    {
        sConstsLoaded = 1;
        for (unsigned int i = 0; i < 173; ++i)
        {
            const char* v2 = gHashStringTblTxt[i];
            unsigned int v3 = HashString::CalcHash(v2);
            BrocSys::RegisterHashString(v2);
            (&hash_const.active.mHash)[i] = v3;
            (&str_const.active)[i] = v2;
        }
    }
}

// ea: 0x0044CC00
int GScr_LoadScriptAndLabel(const char* /*scriptName*/, const char* /*labelName*/, int /*mode*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_main.cpp";
    AeAssert::gCurrentLine = 82;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
    return 0;
}

// ea: 0x0044CD20
void GScr_LoadSingleAnimScript(scr_animscript_t* /*pScript*/, const char* /*szScript*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_main.cpp";
    AeAssert::gCurrentLine = 196;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("Dead Code CD"))
        __debugbreak();
}

// ea: 0x0044CD70
void GScr_FixupBroAnimScriptHooks()
{
    ;
}

// ea: 0x0044CD90
void GScr_AddFieldsForVehicleNode()
{
    ;
}

// ea: 0x0044CDD0
void GScr_FreeScripts()
{
    Scr_FreePrecachedAnimTrees();
}

// ea: 0x00450290
void GScr_AddFieldsForEntity()
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 617;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored())
    {
        if (AeAssert::Assert("ma dead code"))
            __debugbreak();
    }
}

// ea: 0x004502E0
void GScr_AddFieldsForRadiant()
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 661;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored())
    {
        if (AeAssert::Assert("ma dead code"))
            __debugbreak();
    }
}

// ea: 0x00450330
void Scr_SetGenericField(unsigned char* /*base*/, fieldtype_t /*type*/, int /*ofs*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 675;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
}

// ea: 0x00450380
void Scr_SetObjectField(unsigned int /*entnum*/, int /*offset*/, int /*type*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 685;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("DEAD CODE"))
        __debugbreak();
}

// ea: 0x004503D0
void Scr_GetEntityField(int /*entnum*/, int /*offset*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 695;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
}

// ea: 0x00450420
void Scr_GetObjectField(unsigned int /*entnum*/, int /*offset*/, int /*type*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 705;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
}

// ea: 0x00450470
void Scr_FreeEntity(Entity* /*ent*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
    AeAssert::gCurrentLine = 715;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("DEAD CODE"))
        __debugbreak();
}

// ea: 0x0045BF80
void GScr_LoadScripts(bool restart)
{
    gpBrocAPI->mBrocExports.mAnimInitialize();
    g_xanim_num = AnimBankManager::sInst->GetBank(PAK_ID_MIN)->anims.mSize;
    GScr_LoadScriptsAndAnimsForEntities();
    Scr_PrecacheAnimTrees(Hunk_AllocXAnimCreate, restart);
    AnimTree* AnimTreeByName = Scr_GetAnimTreeByName("generic_human");
    if (AnimTreeByName == nullptr)
        G_Error("Could not find animation tree '%s'", "generic_human");
    g_scr_data.generic_human_tree = AnimTreeByName;
}

// ea: 0x0045BFF0
XAnimTree* GScr_GetEntAnimTree(Entity* ent)
{
    XAnimTree* pAnimTree;
    XAnimTree* ActorAnimTree;
    if (ent->s.eType == 11)
    {
        ActorAnimTree = G_GetActorAnimTree(ent->actor);
        pAnimTree = ActorAnimTree;
    }
    else if (ent->s.eType == 13)
    {
        ActorAnimTree = G_GetActorCorpseAnimTree(ent);
        pAnimTree = ActorAnimTree;
    }
    else
    {
        pAnimTree = ent->pAnimTree;
    }
    if (pAnimTree == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_main.cpp";
        AeAssert::gCurrentLine = 700;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            const char* classname = ent->mClassName.mBlock != nullptr
                                        ? (const char*)(ent->mClassName.mBlock + 1)
                                        : &defaultFileName[0];
            const char* v5 = ent->s.eType >= 0x12u
                                 ? "WARNING !! Entity Type Unknown WARNING !!!"
                                 : entityTypeNames[ent->s.eType];
            char* v6 = va("entity of type '%s', classname '%s', origin (%f, %f, %f) does not have an animation tree",
                          v5, classname,
                          ent->r.currentOrigin.v.m128_f32[0],
                          ent->r.currentOrigin.v.m128_f32[1],
                          ent->r.currentOrigin.v.m128_f32[2]);
            if (AeAssert::Warning(v6))
                __debugbreak();
        }
    }
    return pAnimTree;
}

// ea: 0x00470600
void Scr_NotifyFromEnt(Entity* ent, HashString hashValue, Entity* fromEnt)
{
    Entity* v3 = fromEnt;
    if (fromEnt != nullptr && ent != nullptr)
    {
        unsigned int v4 = ent->mHandle.mHandle.mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v4 < 0x540 && ent->mHandle.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey)
            mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != ent)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
            AeAssert::gCurrentLine = 751;
            AeAssert::gCurrentExpr = "*ent->GetHandle() == ent";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        unsigned int fromHandle = v3->mHandle.mHandle.mVal;
        ent->Notify(hashValue, &fromHandle);
    }
}

// ea: 0x004706B0
void Scr_Notify(Entity* ent, HashString hashValue, unsigned int paramcount)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
        AeAssert::gCurrentLine = 764;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned int v2 = ent->mHandle.mHandle.mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v2 < 0x540 && ent->mHandle.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey)
        mObject = EntityHandleDb::sInst.mElements[v2].mObject;
    if (mObject != ent)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
        AeAssert::gCurrentLine = 765;
        AeAssert::gCurrentExpr = "*ent->GetHandle() == ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ent->Notify(hashValue);
}

// ============================================================================
// scr.o batch 1 - MemCount stubs + BrocSys wrappers (smallest first)
// ============================================================================

extern void tlPrint(const char* text);
extern bool gCE;               // ?gCE@@3_NA (pakmanager.cpp)
extern int dword_F6419C[4 * 1580];  // cg_draw.cpp (special recharge block)
extern int dword_F641A0[4 * 1580];
extern int dword_F641A4[4 * 1580];
extern void mem_heap_free(void* ptr);  // ?mem_heap_free@@YAXPAX@Z

namespace MPUIInterface {
void ExitGame();  // ?ExitGame@MPUIInterface@@SAXXZ (mp.o)
bool IsOnlineGame();  // mp.o
bool IsLANGame();     // mp.o
bool IsLocalGame();   // mp.o
}

// ae_heap wrapper view (streamer.o 0x684DD0; definition in pakmanager.cpp)
struct mem_heap;
struct ae_heap_wrapper {
    void* __vftable;   // +0x00
    mem_heap* mHeap;   // +0x04
    bool CheckFree(void* ptr);  // ?CheckFree@ae_heap_wrapper@@UAE_NPAX@Z
};

namespace MemCount {
// IDA types: enum MemCount::eGamePhase : int
enum eGamePhase : int {
    GAME_PHASE_FRONTEND = 0,
    GAME_PHASE_LOADING = 1,
    GAME_PHASE_INGAME = 2,
};
void Init();
void RenderTotals();
void ReportTotals();
bool IsRealLeak(const char* type, int start, int end);
void CheckForMapChangeLeaks();
void CheckForRoundtripLeaks();
void SetGamePhase(eGamePhase phase);
void SetSafeAlloc(bool val);
}  // namespace MemCount

// ea: 0x005BBC50 (retn stub)
void MemCount::Init()
{
}

// ea: 0x005BBC60 (retn stub)
void MemCount::RenderTotals()
{
}

// ea: 0x005BBC70 (retn stub)
void MemCount::ReportTotals()
{
}

// ea: 0x005BBC80 (mov al,1; ret)
bool MemCount::IsRealLeak(const char* /*type*/, int /*start*/, int /*end*/)
{
    return true;
}

// ea: 0x005BBC90 (retn stub)
void MemCount::CheckForMapChangeLeaks()
{
}

// ea: 0x005BBCA0 (retn stub)
void MemCount::CheckForRoundtripLeaks()
{
}

// ea: 0x005BBCB0 (retn stub)
void MemCount::SetGamePhase(MemCount::eGamePhase /*phase*/)
{
}

// ea: 0x005BBCC0 (retn stub)
void MemCount::SetSafeAlloc(bool /*val*/)
{
}

// ea: 0x005BBCD0
void ThreadPrintf(int bitMask, const char* Format, ...)
{
    char Work[512];
    va_list ap;
    va_start(ap, Format);
    if ((bitMask & Cvar_Get("g_scriptdebug", "0", 512)->integer) == bitMask)
    {
        vsprintf(Work, Format, ap);
        tlPrint(Work);
    }
    va_end(ap);
}

// ea: 0x005BC1D0 (empty stub)
void SetDepthOfField(bool, float, float, float, float)
{
}

// ea: 0x005BC1E0 (xor eax,eax; ret)
unsigned int CreateNanoGraph(char* /*id*/, float* const /*param1*/,
                             float* const /*param2*/)
{
    return 0;
}

namespace BrocSys {

bool allowOverLapping;  // ?allowOverLapping@BrocSys@@3_NA (scr.o @ 0x132A0F8)

// ea: 0x005BC840
float atoff(const char* s)
{
    return (float)atof(s);
}

// ea: 0x005BC9B0
float CVarGetFloat(const char* cvarName)
{
    return Cvar_VariableValue(cvarName);
}

// ea: 0x005BECB0
void SceneEffectEnable(unsigned int groupIdHash)
{
    SceneManager::sInst->EnableEffect(groupIdHash);
}

// ea: 0x005BECD0
void SceneEffectDisable(unsigned int groupIdHash)
{
    SceneManager::sInst->DisableEffect(groupIdHash);
}

// ea: 0x005BF420 (disasm: write vec->y to pNode+0x58)
void PathNode_SetAngles(PathNodes::PathNode* pNode, int /*offset*/,
                        Broc::vector* vec)
{
    *(float*)((char*)pNode + 0x58) = vec->y;
}

// ea: 0x005BF4C0 (disasm: nodeStringTable[pNode+0x28] -> Broc::string)
void PathNode_GetType(PathNodes::PathNode* pNode, int /*offset*/,
                      Broc::string* s)
{
    *s = nodeStringTable[*(int*)((char*)pNode + 0x28)];
}

// ea: 0x005BF560
void SentientScr_ConvertSentient(sentient_s* pSelf, int /*offset*/,
                                 Broc::entity* pEnt)
{
    if (pSelf->pEnt != nullptr)
        pEnt->___u0 = pSelf->pEnt->mHandle.mHandle.mVal;
}

// ea: 0x005BF820
void ObjectiveHideStar(unsigned int hideStar, unsigned int objectiveIndex)
{
    ((IGOCompassWidget*)g_femanager.IGO->compassWidget[0])
        ->SetHideCompassStar(hideStar, objectiveIndex);
}

// ea: 0x005BF840
void ObjectiveHideUpdatedText(unsigned int hideText,
                              unsigned int objectiveIndex)
{
    ((IGOCompassWidget*)g_femanager.IGO->compassWidget[0])
        ->SetHideUpdatedText(hideText, objectiveIndex);
}

// ea: 0x005BF890
void SetHUDType(hud_type type, int viewport)
{
    g_femanager.IGO->SetHUDType(type, viewport);
}

// ea: 0x005BF920
void SetTutorialTextAllPlayers(int hash)
{
    if (g_femanager.IGO != nullptr)
        g_femanager.IGO->SetTutorialText(hash, 0);
}

// ea: 0x005BFB40
void SetShadowIntensity(float i)
{
    gProjShadowAlpha = i;
}

// ea: 0x005BFB60
void SetShadowRadius(float r)
{
    gProjShadowSize = r;
}

// ea: 0x005C3460
void SoundStop(unsigned int handle)
{
    SoundDevice::sInst->ReleaseSound(
        DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound>(
            Handle(handle)));
}

// ea: 0x005C4C20
void HudSetTimerInternal(int elemNum, he_type_t type,
                         const char* /*cmdName*/, float fVal)
{
    if (elemNum >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3307;
        AeAssert::gCurrentExpr = "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", elemNum))
            __debugbreak();
    }
    if (type != HE_TYPE_TIMER_DOWN && type != HE_TYPE_TIMER_UP
        && type != HE_TYPE_TENTHS_TIMER_DOWN
        && type != HE_TYPE_TENTHS_TIMER_UP)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3308;
        AeAssert::gCurrentExpr = "type == HE_TYPE_TIMER_DOWN || type == HE_TYPE_TIMER_UP || type == HE_TYPE_TENTHS_TIMER_DOWN || type == HE_TYPE_TENTHS_TIMER_UP";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", type))
            __debugbreak();
    }
    float v4 = fVal * 1000.0f;
    game_hudelem_s* v5 = &g_hudelems[elemNum];
    int v6 = (int)ceilf(v4);
    if (v6 <= 0 && type != HE_TYPE_TIMER_UP)
        Scr_ParamError(0, va("time %g should be > 0", v6 * 0.001f));
    int time = level.time;
    v5->elem.width = 0;
    v5->elem.height = 0;
    v5->elem.mTexture = nullptr;
    v5->elem.fromWidth = 0;
    v5->elem.fromHeight = 0;
    v5->elem.scaleStartTime = 0;
    v5->elem.scaleTime = 0;
    v5->elem.duration = 0;
    v5->elem.text = 0;
    v5->elem.value = 0.0f;
    v5->elem.type = type;
    v5->elem.time = v6 + time;
}

// ea: 0x005C5180
void SetTimer(const Broc::hudelem& hudElem, float fVal)
{
    HudSetTimerInternal(hudElem.___u0, HE_TYPE_TIMER_DOWN, "setTimer", fVal);
}

// ea: 0x005C5260
void SetTenthsTimer(const Broc::hudelem& hudElem, float fVal)
{
    HudSetTimerInternal(hudElem.___u0, HE_TYPE_TENTHS_TIMER_DOWN,
                        "setTenthsTimer", fVal);
}

// ea: 0x005C5BE0
int IsExploded(int exploderNumber)
{
    return CheckpointMgr::sInst->mCurrentScriptExploded
        .mElements[exploderNumber];
}

// ea: 0x005C5C00
void SetWalkRunLoopAnimNode(unsigned int entityHandleVal,
                            unsigned int animWalkRunLoop)
{
    actor_s* v2 = Actor_Get(DbLinkedHandle<EntityHandleDb, Entity>(
        Handle(entityHandleVal)));
    if (v2 != nullptr)
        v2->animWalkRunLoop.mHandle = animWalkRunLoop;
}

// ea: 0x005C5CF0
void CloseMenu1(unsigned int entityHandleVal)
{
    SV_GameSendServerCommand(
        DbLinkedHandle<EntityHandleDb, Entity>(Handle(entityHandleVal)),
        "popupclose");
}

// ea: 0x005BC630
void ScreenFadeUp(unsigned int duration, int viewport)
{
    CG_Fade(0, 0, 0, 0, cgGlobal.time, duration, viewport);
}

// ea: 0x005BCBF0
void SoundCrossFade(unsigned int handle1, unsigned int handle2, float time)
{
    SoundDevice::sInst->CrossFade(handle1, handle2, time);
}

// ea: 0x005BCD40
void SetCullDist(float cullDist)
{
    SV_SetConfigstring(9, va("%g", cullDist));
}

// ea: 0x005BCE70
void SetFootSplashEffect(const Broc::string& effectname)
{
    gFootSplashEffect = effectname;
}

// ea: 0x005BD2F0
void HudClearTypeSettings(game_hudelem_s* hud)
{
    hud->elem.width = 0;
    hud->elem.height = 0;
    hud->elem.mTexture = nullptr;
    hud->elem.fromWidth = 0;
    hud->elem.fromHeight = 0;
    hud->elem.scaleStartTime = 0;
    hud->elem.scaleTime = 0;
    hud->elem.time = 0;
    hud->elem.duration = 0;
    hud->elem.value = 0.0f;
    hud->elem.text = 0;
}

// ea: 0x005BE7A0
unsigned int GetLevel()
{
    return EntityManager::sInst->mWorld->mHandle.mHandle.mVal;
}

// ea: 0x005BF050
void PlaySubtitle(const Broc::string& str)
{
    if (str.mBlock != nullptr)
        subtitle_manager::play_subtitle((const char*)(str.mBlock + 1),
                                        nullptr);
    else
        subtitle_manager::play_subtitle(defaultFileName, nullptr);
}

// ea: 0x005BF080
bool AnimHasNotetrack(unsigned int /*entityHandleVal*/, unsigned int broanim,
                      const unsigned int& name)
{
    return XAnimNotetrackExists(scr_anim_s{broanim}, name);
}

// ea: 0x005BF230
void ActorScr_SetTime(actor_s* a, int offset, const int* val)
{
    *(int*)((char*)a + offset) = (int)((*val * 1000.0) + 0.5);
}

// ea: 0x005BF260
void ActorScr_GetTime(actor_s* a, int offset, int* val)
{
    *val = (int)(*(int*)((char*)a + offset) * 0.001);
}

// ea: 0x005BF290
void ActorScr_SetWeapon(actor_s* a, int offset, const Broc::string* val)
{
    const char* v3 = val->mBlock != nullptr
                         ? (const char*)(val->mBlock + 1)
                         : defaultFileName;
    *(int*)((char*)a + offset) = BG_GetWeaponIndexForName(v3);
}

// ea: 0x005BF2C0
void ActorScr_GetWeapon(actor_s* a, int offset, Broc::string* val)
{
    weaponFileInfo_t* InfoForWeapon =
        BG_GetInfoForWeapon(*(int*)((char*)a + offset));
    if (InfoForWeapon != nullptr)
        *val = InfoForWeapon->szInternalName;
}

// ea: 0x005BF2F0
void ActorScr_GetGroundType(actor_s* a, int /*offset*/, Broc::string* val)
{
    if (a->Physics.iSurfaceType != 0)
        *val = Com_SurfaceTypeToName(a->Physics.iSurfaceType);
}

// ea: 0x005BF440 (disasm: read pNode+0x58 into vec->y)
void PathNode_GetAngles(PathNodes::PathNode* pNode, int /*offset*/,
                        Broc::vector* vec)
{
    float mAngle = *(float*)((char*)pNode + 0x58);
    vec->x = 0.0f;
    vec->y = mAngle;
    vec->z = 0.0f;
}

// ea: 0x005BF580
void SentientScr_ConvertSentientEnemy(sentient_s* pSelf, int /*offset*/,
                                      Broc::entity* pEnt)
{
    sentient_s* pEnemy = pSelf->pEnemy;
    if (pSelf->pEnt != nullptr && pEnemy != nullptr)
    {
        Entity* v4 = pEnemy->pEnt;
        if (v4 != nullptr)
            pEnt->___u0 = v4->mHandle.mHandle.mVal;
    }
}

// ea: 0x005BFA70
unsigned int GetGameUnsignedVar(unsigned int hashVarName)
{
    unsigned int iVal = 0;
    if (!CheckpointMgr::sInst->GetGameVar(hashVarName, &iVal, 1u))
        return 0xFEFEFEFEu;
    return iVal;
}

// ea: 0x005BFAA0
float GetGameFloatVar(unsigned int hashVarName)
{
    float val = 0.0f;
    if (CheckpointMgr::sInst->GetGameVar(hashVarName,
                                         (unsigned int*)&val, 1u))
        return val;
    return (float)NAN;
}

// ea: 0x005C1950
bool ObjectiveStateIndexFromString(int* piStateIndex,
                                   Broc::string& stateString)
{
    const char* v2 = stateString.mBlock != nullptr
                         ? (const char*)(stateString.mBlock + 1)
                         : defaultFileName;
    int v3 = IGOCompassWidget::ObjectiveStateIndexFromString(v2);
    *piStateIndex = v3;
    return v3 != 27;
}

// ea: 0x005C20A0
int GetWeaponIndex(const Broc::string& weaponName)
{
    if (weaponName.mBlock != nullptr)
        return BG_GetWeaponIndexForName((const char*)(weaponName.mBlock + 1));
    return BG_GetWeaponIndexForName(defaultFileName);
}

// ea: 0x005C2190
void MakeGameMessage(const char* pszString, const char* pszCmd)
{
    SV_GameSendServerCommand(
        DbLinkedHandle<EntityHandleDb, Entity>(Handle(0)),
        va("%s \"%s\"", pszCmd, pszString));
}

// ea: 0x005BC860
void ThreadBackupStack(unsigned int lhs)
{
    AeThread* t = (AeThread*)AeThreadManager::sInst.mThreadExecuting;
    t->mBackupStack.Backup(lhs, t->mStackStart);
}

// ============================================================================

// ea: 0x005C3340
void MusicPlay(const Broc::string& pszSoundName)
{
    if (pszSoundName.mBlock != nullptr)
        MusicMgr::sInst->Play((const char*)(pszSoundName.mBlock + 1));
    else
        MusicMgr::sInst->Play(defaultFileName);
}

// ea: 0x005C3480
bool SoundBusVolIsName(const Broc::string& name)
{
    if (name.mBlock != nullptr)
        return SoundDevice::sInst->BusVolumeIsName(
            (const char*)(name.mBlock + 1));
    return SoundDevice::sInst->BusVolumeIsName(defaultFileName);
}

// ea: 0x005C34B0
void SoundBusPitchAddBus(const Broc::string& busId)
{
    if (busId.mBlock != nullptr)
        SoundDevice::sInst->BusPitchAddBus((const char*)(busId.mBlock + 1));
    else
        SoundDevice::sInst->BusPitchAddBus(defaultFileName);
}

// ea: 0x005C34E0
void SoundBusVolAddBus(const Broc::string& busId)
{
    if (busId.mBlock != nullptr)
        SoundDevice::sInst->BusVolumeAddBus((const char*)(busId.mBlock + 1));
    else
        SoundDevice::sInst->BusVolumeAddBus(defaultFileName);
}

// ea: 0x005C3510
void SoundBusPitchRemoveBus(const Broc::string& busId)
{
    if (busId.mBlock != nullptr)
        SoundDevice::sInst->BusPitchRemoveBus(
            (const char*)(busId.mBlock + 1));
    else
        SoundDevice::sInst->BusPitchRemoveBus(defaultFileName);
}

// ea: 0x005C3540
void SoundBusVolRemoveBus(const Broc::string& busId)
{
    if (busId.mBlock != nullptr)
        SoundDevice::sInst->BusVolumeRemoveBus(
            (const char*)(busId.mBlock + 1));
    else
        SoundDevice::sInst->BusVolumeRemoveBus(defaultFileName);
}

// ea: 0x005C3570
void SoundBusVolFade(const Broc::string& busId, float pitch, float time)
{
    const char* v3 = busId.mBlock != nullptr
                         ? (const char*)(busId.mBlock + 1)
                         : defaultFileName;
    SoundDevice::sInst->BusVolumeFade(v3, pitch, time);
}

// ea: 0x005C35A0
void SoundBusPitchFade(const Broc::string& busId, float pitch, float time)
{
    const char* v3 = busId.mBlock != nullptr
                         ? (const char*)(busId.mBlock + 1)
                         : defaultFileName;
    SoundDevice::sInst->BusPitchFade(v3, pitch, time);
}

// ea: 0x005BDB90 (thunk to CG_MotionBlur::End)
void StopCurGenMotionBlur()
{
    CG_MotionBlur::End();
}

// ea: 0x005BEAE0 (return -1 stub)
int GetNodeInProximity(const Broc::vector&, float, bool, unsigned int)
{
    return -1;
}

// ea: 0x005BEC70
void EffectEventPlayQueued(unsigned int effectId)
{
    EffectEventPlayQueuedEffect(Handle(effectId));
}

// ea: 0x005BEC80
bool EffectEventIsStillPlaying(unsigned int effectId)
{
    return EffectEventIsPlaying(Handle(effectId));
}

// ea: 0x005BEC90
void EffectEventStop(unsigned int effectId)
{
    EffectEventStopEmitting(Handle(effectId));
}

// ea: 0x005BECA0
void EffectEventFastForward(unsigned int effectId, float deltaT)
{
    EffectEventFF(Handle(effectId), deltaT);
}

// ea: 0x005BECF0
void RegisterAnimNotifyFunc(const char* pAnimKey, void (__cdecl* fn)(Broc::entity))
{
    AnimNotifyTask::RegisterFunc(pAnimKey, fn);
}

// ea: 0x005BED80
void DialogPlayAllowOverlapping(bool b)
{
    allowOverLapping = b;
}

// ea: 0x005BEDE0
int IsVehicleNodeDefined(unsigned int handle)
{
    return handle != (unsigned int)-1;
}

// ea: 0x005BEDF0 (empty stub)
void AnimScripted2(unsigned int, unsigned int, const Broc::vector&,
                   const Broc::vector&, unsigned int, const Broc::string&,
                   unsigned int, bool, float, float)
{
}

// ea: 0x005BEEA0 (empty stub)
void StartScriptedAnim(unsigned int, unsigned int, const Broc::vector&,
                       const Broc::vector&, unsigned int, const Broc::string&,
                       unsigned int)
{
}

// ea: 0x005BEF90 (empty stub)
void AddFakeFriendly(unsigned int, bool)
{
}

// ea: 0x005BEFA0 (empty stub)
void RemoveFakeFriendly(unsigned int)
{
}

// ea: 0x005BF0A0
bool IsLocalHost()
{
    return MultiplayerMgr::sInst->IsHost();
}

// ea: 0x005BF730 (empty stub)
void SaveCheckpoint()
{
}

// ea: 0x005BF740
void RestoreLastCheckpoint()
{
    CheckpointMgr::sInst->RestoreLastCheckpoint();
}

// ea: 0x005BF7F0 (empty stub)
void DronesStart(const char*, const char*, int, int, int, float, float,
                 bool, bool)
{
}

// ea: 0x005BF800 (empty stub)
void DronesStop(const char*)
{
}

// ea: 0x005BF810 (empty stub)
void DronesDelete(const char*)
{
}

// ea: 0x005BFB20 (empty stub)
void SetDroneScriptControl(unsigned int, bool)
{
}

// ea: 0x005BFB30 (return 0 stub)
unsigned int GetDrones(const Broc::vector&, float, unsigned int*,
                       unsigned int)
{
    return 0;
}

// ea: 0x005BFB80
void EnableAsserts()
{
    AeAssert::gAssertsEnabled = true;
}

// ea: 0x005BFBE0 (empty stub)
void SetZFog(float, float, float)
{
}

// ea: 0x005C0AF0
void ToggleNano(bool on)
{
    gNANO_Animate = on;
}

// ea: 0x005C0C30
void MPScript_ClearTeamScores()
{
    cgGlobal.teamScores[2] = 0;
    cgGlobal.teamScores[1] = 0;
}

// ea: 0x005C1110
void MPScript_ForceControllerErrorMessageDown()
{
    g_controllerConnectedErrorShown[0] = 0;
}

// ea: 0x005C11C0 (empty stub)
void MPScript_SetCompassVisibilty(unsigned int, bool)
{
}

// ea: 0x005C17E0 (tail jmp to tlPrintf)
void DebugOut(const char* strOut)
{
    tlPrintf(strOut);
}

// ea: 0x005C17F0
void FreezeMovement(bool value)
{
    g_freeze_movement = value;
}

// ea: 0x005C1940 (return true stub)
bool PrintObjectiveUpdate(Broc::string&, int, const char*)
{
    return true;
}

// ea: 0x005C5780 (return 0 stub)
unsigned int CreateNanoForce(const Broc::string&, const Broc::vector&,
                             const Broc::vector&)
{
    return 0;
}

// ea: 0x005BCB70
void MusicStop(float fadeOutTime)
{
    MusicMgr::sInst->Stop(fadeOutTime);
}

// ea: 0x005BCB90
void MusicIndoorStop(float fadeOutTime)
{
    MusicMgr::sInst->StopIndoor(fadeOutTime);
}

// ea: 0x005BCBB0
void SoundFadeIn(unsigned int handle, float time)
{
    SoundDevice::sInst->CrossFade(0, handle, time);
}

// ea: 0x005BCBD0
void SoundFadeOut(unsigned int handle, float time)
{
    SoundDevice::sInst->CrossFade(handle, 0, time);
}

// ea: 0x005BD440
bool HudIsPanelType(game_hudelem_s* hud)
{
    return hud->elem.type >= HE_TYPE_COUNT
           && hud->elem.type <= (HE_TYPE_COUNT | HE_TYPE_TIMER_DOWN);
}

// ea: 0x005BD600
void HudSetIsVisible(bool vla)
{
    g_femanager.mDontDrawHud = !vla;
}

// ea: 0x005BDB00
void GlowSetIntensityAux(float val)
{
    ShaderCommon::gGlowIntensity = val;
}

// ea: 0x005BDB20
void GlowSetExpansionAux(float val)
{
    ShaderCommon::gGlowExpansion = val;
}

// ea: 0x005BE7B0
unsigned int GetPlayer()
{
    Entity* p = EntityManager::sInst->GetPlayer(currCl);
    return p->mHandle.mHandle.mVal;
}

// ea: 0x005BEA60
void Scr_SetByte(Entity* ent, int offset, int* val)
{
    *((unsigned char*)ent + offset) = (unsigned char)*val;
}

// ea: 0x005BEA80
void Scr_GetByte(Entity* ent, int offset, int* val)
{
    *val = *((unsigned char*)ent + offset);
}

// ea: 0x005BEAA0
void Scr_SetWord(Entity* ent, int offset, int* val)
{
    *(int*)((unsigned char*)ent + offset) = *val;
}

// ea: 0x005BEAC0
void Scr_GetWord(Entity* ent, int offset, int* val)
{
    *val = *(int*)((unsigned char*)ent + offset);
}

// ea: 0x005BED40
bool BROC_AddCurveKeyEvaluator(unsigned int type, CurveEvalFunc function)
{
    CurveManager::sInst->AddKeyFunc(type, function);
    return true;
}

// ea: 0x005BED60
bool BROC_AddCurveConditionEvaluator(unsigned int type, CurveEvalFunc function)
{
    CurveManager::sInst->AddConditionFunc(type, function);
    return true;
}

// ea: 0x005BEDC0
int IsPathNodeDefined(unsigned int handle)
{
    return handle != 0 && handle != (unsigned int)-1;
}

// ea: 0x005BF210
void ActorScr_SetGoalRadius(actor_s* a, int /*offset*/, const float* val)
{
    Sentient_SetGoalRadius(a->pSentient, *val);
}

// ea: 0x005BF680
void SentientScr_SetGoalAngleTolerance(sentient_s* pSelf, int /*offset*/,
                                       float* val)
{
    Sentient_SetGoalAngleTolerance(pSelf, *val);
}

// ea: 0x005C10F0
bool MPScript_ControllerErrorMessageUp()
{
    int v0 = 0;
    while (!g_controllerConnectedErrorShown[v0])
    {
        if (++v0 >= 4)
            return false;
    }
    return true;
}

// ea: 0x005BF9B0
void OverrideTriggerLookAtRadius(float radius)
{
    gTriggerLookAtOverride = radius;
}

// ea: 0x005BF9D0
void SaveCheckpoint(const char* checkpointName)
{
    CheckpointMgr::sInst->SaveCheckpoint(checkpointName, true);
}

// ea: 0x005BF9F0
void SetGameUnsignedVar(unsigned int hashVarName, unsigned int iVal)
{
    CheckpointMgr::sInst->SetGameVar(hashVarName, &iVal, 1u);
}

// ea: 0x005BFA10
void SetGameFloatVar(unsigned int hashVarName, float fVal)
{
    CheckpointMgr::sInst->SetGameVar(hashVarName, (unsigned int*)&fVal, 1u);
}

// ea: 0x005C1A40 (empty stub)
void UpdateNPCtoVehicleMovement(unsigned int, unsigned int)
{
}

// ea: 0x005BC580 (thunk to View::IsSplitScreen)
bool IsSplitScreen()
{
    return View::IsSplitScreen();
}

// ea: 0x005BC550 (thunk to MPUIInterface::IsLANGame)
bool IsLanGame()
{
    return MPUIInterface::IsLANGame();
}

// ea: 0x005BC560 (thunk to MPUIInterface::IsOnlineGame)
bool IsOnlineGame()
{
    return MPUIInterface::IsOnlineGame();
}

// ea: 0x005BC570 (thunk to MPUIInterface::IsLocalGame)
bool IsLocalGame()
{
    return MPUIInterface::IsLocalGame();
}

// ea: 0x005BC820
int ModXY(int x, int y)
{
    return x % y;
}

// ea: 0x005BC830
float FModXY(float x, float y)
{
    return fmodf(x, y);
}

// ea: 0x005BC880 (mov eax, AeThreadManager::sInst.mThreadExecuting)
bool ThreadIsThreadExecuting()
{
    return AeThreadManager::sInst.mThreadExecuting != nullptr;
}

// ea: 0x005BC9A0
int CVarGetInt(const char* cvarName)
{
    return Cvar_VariableIntegerValue(cvarName);
}

// ea: 0x005BCC10
void SetIndoor(bool indoor)
{
    g_indoor = indoor;
}

// ea: 0x005BCC80
unsigned int GetTime()
{
    return level.time;
}

// ea: 0x005BCC90
float GetDeltaTime()
{
    return ServerTime::sInst.mTickDelta;
}

// ea: 0x005BCD10 (thunk to CG_GetNorthDirection)
float GetNorthYaw()
{
    return CG_GetNorthDirection();
}

// ea: 0x005BCD20 (empty stub)
void GameSave(const Broc::string&)
{
}

// ea: 0x005BCD30 (empty stub)
void GameLoad(const Broc::string&)
{
}

// ea: 0x005BCDC0
void SetPlayerIgnoreRadiusDamage(bool bVal)
{
    level.bPlayerIgnoreRadiusDamageLatched = bVal;
}

// ea: 0x005BCDD0 (empty stub)
void MissionSuccess(const Broc::string&)
{
}

// ea: 0x005BCE60
void SetRainDrops(bool on)
{
    FX_SetRainDrops(on);
}

// ea: 0x005BCE90
void DrawCompassFriendlies(bool inBool)
{
    level.bDrawCompassFriendlies = inBool;
}

// ea: 0x005BCEC0
void SetMaxVehicles(int vehicles)
{
    vehicle_InitDynamicBuffers(vehicles);
}

// ea: 0x005BCED0 (empty stub)
void ProfBegin()
{
}

// ea: 0x005BCEE0 (empty stub)
void ProfEnd()
{
}

// ea: 0x005BCF10 (thunk to G_FlushCorpses)
void FlushCorpses()
{
    G_FlushCorpses();
}

// ea: 0x005BCF20 (empty stub)
void StartMemCheck()
{
}

// ea: 0x005BCF30 (empty stub)
void EndMemCheck()
{
}

// ea: 0x005BD2B0 (return 0 stub)
int ProfileDeclareID(const char*)
{
    return 0;
}

// ea: 0x005BD2C0 (empty stub)
void ProfileStart(int)
{
}

// ea: 0x005BD2D0 (empty stub)
void ProfileStop(int)
{
}

// ea: 0x005BD2E0 (empty stub)
void ProfileSetVal(int, int)
{
}

// ea: 0x005BD900 (return false stub)
bool SetMissionToTrack(const char*, bool)
{
    return false;
}

// ea: 0x005BD910 (return 0 stub)
int GetMissionStat(int, bool)
{
    return 0;
}

// ea: 0x005BD920 (return 0 stub)
int GetMissionStatAll(int)
{
    return 0;
}

// ea: 0x005BD930 (empty stub)
void SetMissionStat(int, int)
{
}

// ea: 0x005BD940 (empty stub)
void IncMissionStat(int)
{
}

// ea: 0x005BD950 (empty stub)
void DecMissionStat(int)
{
}

// ea: 0x005BD960 (return 0 stub)
float GetAvgMissionStat(int)
{
    return 0.0f;
}

// ea: 0x005BD970 (return 0 stub)
int GetMissionCompletionTime()
{
    return 0;
}

// ea: 0x005BD980 (empty stub)
void UpdateMissionCompletionTime()
{
}

// ea: 0x005BD990 (empty stub)
void SetPlayerWeaponUsed(int)
{
}

// ea: 0x005BD9A0 (return false stub)
bool WasPlayerWeaponUsed(int)
{
    return false;
}

// ea: 0x005BD9B0 (return false stub)
bool WasPlayerWeaponCategoryUsed(int, bool)
{
    return false;
}

// ea: 0x005BDA30 (mov byte ptr [light+0x3D], 1)
void RemoveDynamicLight(unsigned int light)
{
    if (light != 0)
        *((unsigned char*)light + 0x3D) = 1;
}

// ea: 0x005BDAF0 (empty stub)
void EnableNanoForces(bool)
{
}

// ea: 0x005BDB60
void GlowSetGodRaysAux(bool val)
{
    ShaderCommon::gGlowGodRays = val;
}

// ea: 0x005BDB70
void GlowSetPassesAux(int val)
{
    ShaderCommon::gGlowPasses = val;
}

// ea: 0x005BDB80
void CurGenMotionBlur(float fLevel, float fPlateauTime, float fFadeTime)
{
    CG_MotionBlur::Begin(fLevel, fPlateauTime, fFadeTime);
}

// ea: 0x005BC210
const unsigned int ConvertStringToHash(const char* str)
{
    if (str == nullptr)
        return 0;
    tlFixedString tmp(str);
    return tmp.hash;
}

// ea: 0x005BC230
void MemFree(void* p)
{
    if (gBrocPool->InPool(p))
    {
        gBrocPool->Release(p);
    }
    else if (!((ae_heap_wrapper*)gBrocHeap)->CheckFree(p))
    {
        mem_heap_free(p);
    }
}

// ea: 0x005BC280
void* PoolAlloc(unsigned int s)
{
    return gCommonPoolAllocator->Allocate(s, false);
}

// ea: 0x005BC2A0
void PoolFree(void* p)
{
    gCommonPoolAllocator->Release(p);
}

// ea: 0x005BC490
void SetSpecialRecharge(int start, int length, int playerClass, int playerIndex)
{
    int v4 = 1580 * playerIndex;
    dword_F6419C[v4] = playerClass;
    dword_F641A0[v4] = length + start;
    dword_F641A4[v4] = length;
}

// ea: 0x005BC4C0
void AdvanceSpecialRecharge(int time)
{
    int v1 = 1580 * currCl;
    int v2 = dword_F641A0[1580 * currCl] - time;
    dword_F641A0[1580 * currCl] = v2;
    if (cgGlobal.time > v2)
    {
        dword_F641A0[v1] = 0;
        dword_F641A4[v1] = 0;
    }
}

// ea: 0x005BC500
int GetSpecialRechargePlayerClass()
{
    return dword_F6419C[1580 * currCl];
}

// ea: 0x005BC520
bool SpecialEditionSkin()
{
    return gCE;
}

// ea: 0x005BC530
bool IsHost()
{
    return MultiplayerMgr::sInst->IsHost();
}

// ea: 0x005BC540
bool IsRankedGame()
{
    return MultiplayerMgr::sInst->mRankedGame;
}

// ea: 0x005BC590
void SetTeamGame(bool teamGame)
{
    cgGlobal.teamGame = teamGame;
}

// ea: 0x005BC5A0
void SetShowScore(bool showScore)
{
    cgGlobal.showScore = showScore;
}

// ea: 0x005BC5E0
bool GetTeamGame()
{
    return cgGlobal.teamGame;
}

// ea: 0x005BC6D0 (tail jmp to MPUIInterface::ExitGame)
void QuitGame()
{
    MPUIInterface::ExitGame();
}

// ea: 0x005BC700 (tail jmp to Com_Printf)
void Print(const char* txt)
{
    Com_Printf(txt);
}

// ea: 0x005BC710
void PrintLn(const char* txt)
{
    Com_Printf("%s\n", txt);
}

// ea: 0x005BC810
float SquareRootX(float v)
{
    return sqrtf(v);
}

}  // namespace BrocSys
// scr.o batch 10 - AeThread lifecycle cluster
// ============================================================================

// SizedHandle<8,24> - local twin of g_accessors.cpp (same mangling)
template <int INDEX_BITS, int KEY_BITS>
class SizedHandle {
public:
    unsigned int mVal;  // +0x00
    SizedHandle() : mVal(0) {}
    SizedHandle(int index, int key)
    {
        mVal = (unsigned int)index | ((unsigned int)key << INDEX_BITS);
    }
    SizedHandle(Handle h) { mVal = h.mVal; }
    operator Handle() const { return Handle((int)mVal); }
    int GetIndex() const { return (int)(mVal & ((1 << INDEX_BITS) - 1)); }
    int GetKey() const { return (int)(mVal >> INDEX_BITS); }
};

// HandleDb<AeThread,256,SizedHandle<8,24>> - local generic twin
template <typename T, int CAPACITY, typename H>
class HandleDb {
public:
    struct DbElement {
        T*  mObject;  // +0x00
        int mKey;     // +0x04
        DbElement() : mObject(nullptr), mKey(1) {}
        T* GetObject() const { return mObject; }
        void SetObject(T* obj) { mObject = obj; }
        int GetKey() const { return mKey; }
        void Release() { ++mKey; mObject = nullptr; }
    };
    unsigned int mFreeIndices[(CAPACITY + 31) / 32];  // +0x00
    DbElement mElements[CAPACITY];                    // +0x20
    void (*mDebugCallback)(int, T*);                  // +0x820

    HandleDb();
    void ReleaseHandle(Handle h);
    T* DereferenceHandle(Handle h) const;
    void BindObjectToHandle(Handle handle, T* obj);
    H AllocateHandle();
};

template <typename T, int CAPACITY, typename H>
HandleDb<T, CAPACITY, H>::HandleDb()
{
    for (int i = 0; i < (CAPACITY + 31) / 32; ++i)
        mFreeIndices[i] = 0;
    for (int i = 0; i < CAPACITY; ++i)
    {
        mElements[i].mObject = nullptr;
        mElements[i].mKey = 1;
        mFreeIndices[i >> 5] |= 1u << (i & 0x1F);
    }
    mDebugCallback = nullptr;
}

template <typename T, int CAPACITY, typename H>
void HandleDb<T, CAPACITY, H>::ReleaseHandle(Handle h)
{
    if (h.mVal != 0)
    {
        unsigned int v3 = h.mVal & ((1u << 8) - 1);
        if (v3 >= (unsigned int)CAPACITY)
        {
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("freeing invalid handle"))
                __debugbreak();
        }
        else
        {
            if (mElements[v3].mKey == (int)(h.mVal >> 8))
            {
                mFreeIndices[v3 >> 5] |= 1u << (v3 & 0x1F);
                mElements[v3].mObject = nullptr;
                ++mElements[v3].mKey;
                return;
            }
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("freeing invalid handle"))
                __debugbreak();
        }
    }
}

template <typename T, int CAPACITY, typename H>
T* HandleDb<T, CAPACITY, H>::DereferenceHandle(Handle h) const
{
    unsigned int v2 = h.mVal & ((1u << 8) - 1);
    if (v2 < (unsigned int)CAPACITY
        && (h.mVal >> 8) == (unsigned int)mElements[v2].mKey)
        return mElements[v2].mObject;
    return nullptr;
}

template <typename T, int CAPACITY, typename H>
void HandleDb<T, CAPACITY, H>::BindObjectToHandle(Handle handle, T* obj)
{
    unsigned int v3 = handle.mVal & ((1u << 8) - 1);
    if (v3 < (unsigned int)CAPACITY)
    {
        if (mElements[v3].mKey != (int)(handle.mVal >> 8))
        {
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "handle was not allocated for this object"))
                __debugbreak();
        }
        mElements[v3].mObject = obj;
    }
}

template <typename T, int CAPACITY, typename H>
H HandleDb<T, CAPACITY, H>::AllocateHandle()
{
    int idx = -1;
    for (int w = 0; w < (CAPACITY + 31) / 32; ++w)
    {
        unsigned int bits = mFreeIndices[w];
        if (bits != 0)
        {
            int bit = 0;
            while ((bits & 1u) == 0)
            {
                bits >>= 1;
                ++bit;
            }
            idx = w * 32 + bit;
            break;
        }
    }
    if (idx < 0 || idx >= CAPACITY)
    {
        if (!AeAssert::IsIgnored()
            && AeAssert::Error("out of handles! - Tell MikeA (MAX_GENTITIES)"))
            __debugbreak();
        return H();
    }
    mFreeIndices[idx >> 5] &= ~(1u << (idx & 0x1F));
    H h(idx, mElements[idx].mKey);
    return h;
}

// AeThreadManager layout overlay (IDA type 5741)
struct AeThreadManagerLayout {
    int            sNumThreads;     // +0x00
    AeStateList    mThreads;        // +0x04
    AeStateList    mExecThreads;    // +0x14
    AeStateList    mPendingNotifys; // +0x24
    unsigned char  mHandleDb[0x824];// +0x34
    void*          mThreadExecuting;// +0x858
    void*          mNewThreadExec;  // +0x85C
    void*          mScriptToUnload; // +0x860
};

void BrocDtorBase::Destroy(void* /*inst*/)
{
}

// SetJmp/LongJmp (register-snapshot longjmp; transcribed from disasm)
__declspec(naked) void SetJmp(unsigned int* storageAddr)
{
    __asm {
        pusha
        pushf
        mov eax, [esp+28h]      ; storageAddr
        mov ecx, [esp]
        mov [eax], ecx          ; [0] = eflags
        mov ecx, [esp+4]
        mov [eax+4], ecx        ; [1] = edi
        mov ecx, [esp+8]
        mov [eax+8], ecx        ; [2] = esi
        mov ecx, [esp+0Ch]
        mov [eax+0Ch], ecx      ; [3] = ebp
        mov ecx, [esp+10h]
        mov [eax+10h], ecx      ; [4] = esp snapshot
        mov ecx, [esp+14h]
        mov [eax+14h], ecx      ; [5] = ebx
        mov ecx, [esp+18h]
        mov [eax+18h], ecx      ; [6] = edx
        mov ecx, [esp+1Ch]
        mov [eax+1Ch], ecx      ; [7] = ecx
        mov ecx, [esp+20h]
        mov [eax+20h], ecx      ; [8] = eax
        mov ecx, [esp+24h]
        mov [eax+24h], ecx      ; [9] = return address
        popf
        popa
        retn
    }
}

__declspec(naked) void LongJmp(unsigned int* r)
{
    __asm {
        mov eax, [esp+4]
        mov ebx, [eax+10h]
        mov ecx, [eax+24h]
        mov [ebx], ecx          ; *(saved esp slot) = return address
        mov esp, [eax+10h]
        push dword ptr [eax+20h]
        push dword ptr [eax+1Ch]
        push dword ptr [eax+18h]
        push dword ptr [eax+14h]
        push dword ptr [eax+10h]
        push dword ptr [eax+0Ch]
        push dword ptr [eax+8]
        push dword ptr [eax+4]
        popf
        popa
        retn
    }
}

PoolAllocator* AeThread::sAllocator;
unsigned int* AeThread::sBackup = (unsigned int*)-1;

// DestroyBrocInstsStub (scr.o 0x5BC0E0, empty)
static void DestroyBrocInstsStub(void*)
{
}

namespace BrocSys {
void BrocObjDtor(void* ptr);  // fwd for DestroyBrocInsts
void BrocObjCtor(void* ptr, BrocDtorBase* dtor);  // fwd (defined at file end)
void KillThreadExec();  // fwd (defined at file end)
void ThreadDebug(unsigned int threadId);  // fwd (defined at file end)
}

static void BrocFree(void* p)
{
    if (gBrocPool->InPool(p))
        gBrocPool->Release(p);
    else if (!((ae_heap_wrapper*)gBrocHeap)->CheckFree(p))
        mem_heap_free(p);
}

// ea: 0x005C9400
void AeThread::RegisterBrocInst(void* inst, BrocDtorBase* dtor)
{
    if (inst < (void*)(mStackStart - 0x8000) || inst > (void*)mStackStart)
        return;
    BrocObjCreated* mBrocCreated = this->mBrocCreated;
    if (mBrocCreated != nullptr)
    {
        while (mBrocCreated->list.m_size == 15)
        {
            mBrocCreated = mBrocCreated->next;
            if (mBrocCreated == nullptr)
            {
                mBrocCreated = this->mBrocCreated;
                while (mBrocCreated->list.m_size == 15)
                {
                    mBrocCreated = mBrocCreated->next;
                    if (mBrocCreated == nullptr)
                    {
                        BrocObjCreated* v6 = new BrocObjCreated;
                        if (v6 == nullptr)
                            goto alloc_fail;
                        v6->list.m_size = 0;
                        v6->next = this->mBrocCreated;
                        goto have;
                    }
                }
                break;
            }
        }
    }
    else
    {
        BrocObjCreated* v6 = new BrocObjCreated;
        if (v6 != nullptr)
        {
            v6->list.m_size = 0;
            v6->next = nullptr;
        }
        else
        {
        alloc_fail:
            v6 = nullptr;
        }
        mBrocCreated = v6;
    }
have:
    this->mBrocCreated = mBrocCreated;
    if (mBrocCreated == nullptr)
        return;
    ae_pair<void*, unsigned int> elt(inst, (unsigned int)(void*)dtor);
    mBrocCreated->list.push_back(elt);
}

// ea: 0x005C94C0
void AeThread::RegisterBrocDtor(void* inst)
{
    if (inst < (void*)(mStackStart - 0x8000) || inst > (void*)mStackStart)
        return;
    BrocObjCreated* mBrocCreated = this->mBrocCreated;
    while (mBrocCreated != nullptr)
    {
        int v4 = mBrocCreated->list.m_size - 1;
        while (v4 >= 0)
        {
            if (v4 >= 15)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            if (mBrocCreated->list.m_elements[v4].m_first == inst)
                break;
            --v4;
        }
        if (v4 >= 0)
        {
            int m_size = mBrocCreated->list.m_size;
            if (m_size > 1 && v4 < m_size)
            {
                int v6 = m_size - 1 <= 0 ? 0 : m_size - 1;
                mBrocCreated->list.m_elements[v4] =
                    mBrocCreated->list.m_elements[v6];
            }
            if (mBrocCreated->list.m_size != 0)
                mBrocCreated->list.m_size = mBrocCreated->list.m_size - 1;
            if (mBrocCreated->list.m_size == 0
                && this->mBrocCreated->next != nullptr)
            {
                this->mBrocCreated = mBrocCreated->next;
                BrocFree(mBrocCreated);
            }
            return;
        }
        mBrocCreated = mBrocCreated->next;
    }
}

// ea: 0x005DAF90
void AeThread::DestroyBrocInsts()
{
    gpBrocAPI->mBrocObjDtor = DestroyBrocInstsStub;
    BrocObjCreated* mBrocCreated = this->mBrocCreated;
    if (mBrocCreated != nullptr)
    {
        do
        {
            for (int v2 = 0; v2 < mBrocCreated->list.m_size; ++v2)
            {
                if (v2 > 0xE)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                    AeAssert::gCurrentLine = 154;
                    AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("out of bounds"))
                        __debugbreak();
                }
                ae_pair<void*, unsigned int>& elt =
                    mBrocCreated->list.m_elements[v2];
                BrocDtorBase* dtor = (BrocDtorBase*)elt.m_second;
                dtor->Destroy(elt.m_first);
            }
            BrocObjCreated* v4 = mBrocCreated;
            mBrocCreated = mBrocCreated->next;
            BrocFree(v4);
        } while (mBrocCreated != nullptr);
        this->mBrocCreated = nullptr;
    }
    else
    {
        this->mBrocCreated = nullptr;
    }
    gpBrocAPI->mBrocObjDtor = BrocSys::BrocObjDtor;
}

// ea: 0x005DAE30
AeThread::~AeThread()
{
    // safe_for_each(mStateControllers, DelFunctor) - delete each state
    AeDListNode* node = mStateControllers.m_head;
    if (node != nullptr && node != mStateControllers.m_end)
    {
        while (node != mStateControllers.m_end)
        {
            AeDListNode* next = node->mNext;
            delete (AeThreadState*)node;
            node = next;
        }
    }
    mStateControllers.m_head->mNext = mStateControllers.m_end;
    mStateControllers.m_tail = mStateControllers.m_head;
    mStateControllers.m_size = 0;

    if (mFunctor != nullptr)
    {
        mFunctor->~AeThreadFunctor();
        gCommonPoolAllocator->Release(mFunctor);
    }
    if (mHandle.mVal != 0)
    {
        AeThreadManagerLayout* L = (AeThreadManagerLayout*)&AeThreadManager::sInst;
        ((HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb)
            ->ReleaseHandle(mHandle);
    }
    if (((mFlags.mMask & 8) == 0) && (mFlags.mMask & 1) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AeThread.cpp";
        AeAssert::gCurrentLine = 169;
        AeAssert::gCurrentExpr =
            "mFlags.Test( kFlagFinished ) | mFlags.Test( kFlagNotStarted )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("thread is still running"))
            __debugbreak();
    }
    BrocObjCreated* mBrocCreated = this->mBrocCreated;
    while (mBrocCreated != nullptr)
    {
        BrocObjCreated* v7 = mBrocCreated;
        mBrocCreated = mBrocCreated->next;
        BrocFree(v7);
    }
    --((AeThreadManagerLayout*)&AeThreadManager::sInst)->sNumThreads;
    mBackupStack.~BackupStack();
}

// ============================================================================
// AeThreadManager methods (scr.o)
// ============================================================================

// ea: 0x005C9770
void AeThreadManager::ReleaseHandle(AeThread* t)
{
    if (t->mHandle.mVal != 0)
    {
        AeThreadManagerLayout* L = (AeThreadManagerLayout*)this;
        ((HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb)
            ->ReleaseHandle(t->mHandle);
        t->mHandle.mVal = 0;
    }
}

// ea: 0x005C97A0
void AeThreadManager::ReleaseHandle(Handle h)
{
    if (h.mVal != 0)
    {
        AeThreadManagerLayout* L = (AeThreadManagerLayout*)this;
        ((HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb)
            ->ReleaseHandle(h);
    }
}

// ea: 0x005DB160
Handle AeThreadManager::AssignHandle(AeThread* t)
{
    AeThreadManagerLayout* L = (AeThreadManagerLayout*)this;
    HandleDb<AeThread, 256, SizedHandle<8, 24>>* db =
        (HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb;
    Handle v4 = db->AllocateHandle();
    db->BindObjectToHandle(v4, t);
    t->mHandle = v4;
    return v4;
}

// ea: 0x005DC120
void AeThreadManager::KillThread(AeThread* t)
{
    if (t == nullptr)
        return;
    AeThreadManagerLayout* L = (AeThreadManagerLayout*)this;
    if (L->mThreadExecuting == t)
        L->mThreadExecuting = nullptr;
    AeStateList* list = (t->mFlags.mMask & 0x800) != 0
                            ? &L->mExecThreads
                            : &L->mThreads;
    // reserved_dlist<AeThread>::erase inline
    t->m_dlist_node.mPrev->mNext = t->m_dlist_node.mNext;
    t->m_dlist_node.mNext->mPrev = t->m_dlist_node.mPrev;
    --list->m_size;
    t->~AeThread();
    AeThread::sAllocator->Release(t);
}

// ea: 0x005C97C0 (walk per disasm: compare node+0x14 handle, flag node+0x10)
void AeThreadManager::DebugThread(unsigned int threadId)
{
    AeThreadManagerLayout* L = (AeThreadManagerLayout*)this;
    if (threadId == 0 && L->mThreadExecuting != nullptr)
        ((AeThread*)L->mThreadExecuting)->mFlags.mMask |= 0x100;
    AeDListNode* node = L->mThreads.m_head;
    if (node != nullptr && node != L->mThreads.m_end)
    {
        for (AeDListNode* n = node; n != nullptr; n = n->mNext)
        {
            if (*(unsigned int*)((char*)n + 0x14) == threadId)
            {
                *(unsigned int*)((char*)n + 0x10) |= 0x100;
                return;
            }
        }
    }
    node = L->mExecThreads.m_head;
    if (node != nullptr && node != L->mExecThreads.m_end)
    {
        for (AeDListNode* n = node; n != nullptr; n = n->mNext)
        {
            if (*(unsigned int*)((char*)n + 0x14) == threadId)
            {
                *(unsigned int*)((char*)n + 0x10) |= 0x100;
                return;
            }
        }
    }
}

// ea: 0x005CA300
void BrocSys::BrocObjCtor(void* ptr, BrocDtorBase* dtor)
{
    if (AeThreadManager::sInst.mThreadExecuting != nullptr)
        ((AeThread*)AeThreadManager::sInst.mThreadExecuting)
            ->RegisterBrocInst(ptr, dtor);
}

// ea: 0x005CA320
void BrocSys::BrocObjDtor(void* ptr)
{
    if (AeThreadManager::sInst.mThreadExecuting != nullptr)
        ((AeThread*)AeThreadManager::sInst.mThreadExecuting)
            ->RegisterBrocDtor(ptr);
}

// ea: 0x005DB640
void BrocSys::KillThreadExec()
{
    ((AeThread*)AeThreadManager::sInst.mThreadExecuting)->DestroyBrocInsts();
    LongJmp(AeThread::sBackup);
}

// ea: 0x005C9E20
void BrocSys::ThreadDebug(unsigned int threadId)
{
    AeThreadManager::sInst.DebugThread(threadId);
}

// ============================================================================
// scr.o batch 11 - AeThread ctor + manager thread lists
// ============================================================================

// ea: 0x005C79D0
AeThread::AeThread(const char* file, int line, const char* func,
                   unsigned int ehandle, AeThreadFunctor* ftor,
                   bool bUseScratchpad)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mOwner.mHandle.mVal = ehandle;
    mFunctor = ftor;
    mFlags.mMask = 0;
    mHandle.mVal = 0;
    mStateControllers.m_size = 0;
    mStateControllers.m_end = nullptr;
    mStateControllers.m_head = (AeDListNode*)&mStateControllers.m_end;
    mStateControllers.m_tail = (AeDListNode*)&mStateControllers.m_head;
    mBrocCreated = nullptr;
    mBackupStack.mBlockList = nullptr;
    mBackupStack.mSize = 0;
    mBackupStack.mBegin = 0;
    mBackupStack.mEnd = 0;
    mFuncName = func;
    mFile = file;
    mLine = line;
    if (ftor == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AeThread.cpp";
        AeAssert::gCurrentLine = 103;
        AeAssert::gCurrentExpr = "ftor";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Bad functor pointer in thread create"))
            __debugbreak();
    }
    mFlags.mMask |= 1;
    if (bUseScratchpad)
        mFlags.mMask |= 0x400;
    ++((AeThreadManagerLayout*)&AeThreadManager::sInst)->sNumThreads;
}

// ea: 0x005C7AC0
void AeThread::Sleep(AeThreadState* stateController)
{
    mFlags.mMask |= 2;
    mFlags.mMask |= 0x10;
    mFlags.mMask |= 0x210;
    stateController->m_dlist_node.mNext = mStateControllers.m_end;
    stateController->m_dlist_node.mPrev = mStateControllers.m_tail;
    ((AeDListNode*)mStateControllers.m_tail)->mNext =
        &stateController->m_dlist_node;
    mStateControllers.m_tail = &stateController->m_dlist_node;
    ++mStateControllers.m_size;
}

// ea: 0x005C95F0
void AeThreadManager::AddThread(AeThread* t)
{
    if (t == nullptr)
    {
        tlPrintf("===Doing threads dump===\n");
        AeDListNode* node = ((AeThreadManagerLayout*)this)->mThreads.m_head;
        if (node != nullptr
            && node != ((AeThreadManagerLayout*)this)->mThreads.m_end)
        {
            int i = 0;
            for (AeDListNode* n = node; n != nullptr; n = n->mNext)
            {
                AeThread* th = (AeThread*)n;
                tlPrintf("%s(%d): (%03d) %s\n", th->mFile, th->mLine, i++,
                         th->mFuncName);
            }
        }
        tlFatal("out of room in pool");
        return;
    }
    AeStateList* mThreads = &((AeThreadManagerLayout*)this)->mThreads;
    t->m_dlist_node.mNext = mThreads->m_end;
    t->m_dlist_node.mPrev = mThreads->m_tail;
    ((AeDListNode*)mThreads->m_tail)->mNext = &t->m_dlist_node;
    mThreads->m_tail = &t->m_dlist_node;
    ++mThreads->m_size;
}

// ea: 0x005C96D0
void AeThreadManager::ProcessScriptNotifys()
{
    AeThreadManagerLayout* L = (AeThreadManagerLayout*)this;
    AeStateList* pending = &L->mPendingNotifys;
    EntityNotifyLocal* head = (EntityNotifyLocal*)pending->m_head;
    if (head == nullptr)
        return;
    EntityNotifyLocal* next = (EntityNotifyLocal*)((AeDListNode*)head)->mNext;
    if (head == (EntityNotifyLocal*)pending->m_end || next == nullptr)
        return;
    while (next != nullptr)
    {
        EntityNotifyLocal* v4 = head;
        head = next;
        next = (EntityNotifyLocal*)((AeDListNode*)next)->mNext;
        // reserved_dlist<EntityNotify>::erase(v4)
        ((AeDListNode*)v4)->mPrev->mNext = ((AeDListNode*)v4)->mNext;
        ((AeDListNode*)v4)->mNext->mPrev = ((AeDListNode*)v4)->mPrev;
        --pending->m_size;

        unsigned int v5 = v4->mOwner.mHandle.mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v5 < 0x540
            && v4->mOwner.mHandle.mVal >> 12
                   == (unsigned int)EntityHandleDb::sInst.mElements[v5].mKey)
            mObject = EntityHandleDb::sInst.mElements[v5].mObject;
        if (mObject != nullptr)
        {
            mObject->AddNotify((EntityNotify*)v4);
        }
        else
        {
            v4->~EntityNotifyLocal();
            EntityNotifyLocal::sAllocator->Release(v4);
        }
        if (next == nullptr)
            break;
    }
}

// ea: 0x005DB100
AeThreadManager::AeThreadManager()
{
    AeThreadManagerLayout* L = (AeThreadManagerLayout*)this;
    L->mThreads.m_end = nullptr;
    L->mThreads.m_size = 0;
    L->mThreads.m_head = (AeDListNode*)&L->mThreads.m_end;
    L->mThreads.m_tail = (AeDListNode*)&L->mThreads.m_head;
    L->mExecThreads.m_end = nullptr;
    L->mExecThreads.m_head = (AeDListNode*)&L->mExecThreads.m_end;
    L->mExecThreads.m_tail = (AeDListNode*)&L->mExecThreads.m_head;
    L->mExecThreads.m_size = 0;
    AeDListNode* p_end = (AeDListNode*)&L->mPendingNotifys.m_end;
    AeDListNode* p_head = (AeDListNode*)&L->mPendingNotifys.m_head;
    p_head->mNext = p_end;
    L->mPendingNotifys.m_tail = (AeDListNode*)p_head;
    L->mPendingNotifys.m_size = 0;
    p_end->mNext = nullptr;
    p_end->mPrev = nullptr;
    HandleDb<AeThread, 256, SizedHandle<8, 24>> db;
    memcpy(L->mHandleDb, &db, sizeof(db));
    L->mThreadExecuting = nullptr;
    L->mNewThreadExec = nullptr;
    L->mScriptToUnload = nullptr;
}
