// ============================================================================
// g_scr.cpp - script integration wrappers (g.o: g_scr_main.cpp / g_spawn.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "game/mpactors/aitype.h"

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

// TPakInfo - opaque pak info enum (scr.o; W4TPakInfo mangling)
enum TPakInfo {
    kTPakInfoInvalid = 0,
};

namespace AeStringSupport {
void Concat(char* dst, int& dstLen, int dstCapacity,
            const char* src);  // ?Concat@AeStringSupport@@YAXPADAAHHPBD@Z
}

namespace ShaderCommon {
extern bool gGlowGodRays;  // ?gGlowGodRays@ShaderCommon@@3_NA
extern int  gGlowPasses;   // ?gGlowPasses@ShaderCommon@@3HA
extern float gGlowIntensity;  // ?gGlowIntensity@ShaderCommon@@3MA
extern float gGlowExpansion;  // ?gGlowExpansion@ShaderCommon@@3MA
extern float gGlowBrighten;   // ?gGlowBrighten@ShaderCommon@@3MA
}

namespace View {
bool IsSplitScreen();  // ?IsSplitScreen@View@@YA_NXZ (cg.o)
}

namespace BrocHelper {
int m_treeCount;  // ?m_treeCount@BrocHelper@@3HA (scr.o @ 0x1329E78)
struct brocFunctionLookup {  // IDA type 5907
    unsigned int mHashedName;                  // +0x00
    unsigned int (__cdecl* mFunction)(void*);  // +0x04
};
extern brocFunctionLookup broFuncLookupTable[70];  // ?broFuncLookupTable@BrocHelper@@3PAUbrocFunctionLookup@1@A @ 0x1329E80
void SetLoadedTrees(int num);  // ?SetLoadedTrees@BrocHelper@@YAXH@Z
int  GetLoadedTrees();         // ?GetLoadedTrees@BrocHelper@@YAHXZ
void Init();                   // ?Init@BrocHelper@@YAXXZ (scr.o 0x5BE180)
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

// AudioBankMgr view (game.o; full class in g_entity_misc.cpp)
extern void* AudioBankMgr_sInst;  // ?sInst@AudioBankMgr@@2PAV1@A @ 0xF4EBD8
class AudioBankMgrLocal {
public:
    void LoadWbk(const tlFixedString& name, bool async);  // ?LoadWbk@AudioBankMgr@@QAEXABVtlFixedString@@_N@Z
    void FreeWbk(const tlFixedString& name, bool async);  // ?FreeWbk@AudioBankMgr@@QAEXABVtlFixedString@@_N@Z
};

namespace BrocSys {
void HudSetDefaults(game_hudelem_s* hud);  // ?HudSetDefaults@BrocSys@@YAXPAUgame_hudelem_s@@@Z
void HudSetClockInternal(int elemNum, he_type_t type, const char* cmdName,
                         const char* a4, float fTime, float fDur, int width,
                         int height);  // ?HudSetClockInternal@BrocSys@@YAXHW4he_type_t@@PBD1MMHH@Z
void HudSetClockInternal(unsigned int elemNum, he_type_t type,
                         const char* texturename, const char* cmdName,
                         const float fTime, const float fDur, int width,
                         int height);  // 0x5C4C50
}

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
    EntityNotifyLocal(unsigned int hashStr,
                      DbLinkedHandle<EntityHandleDb, Entity> ent, void* param)
        : mStr(hashStr), mOwner(ent), mParam(param)
    {
        m_dlist_node.mNext = nullptr;
        m_dlist_node.mPrev = nullptr;
    }
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

template <typename T, int CAPACITY>
struct ae_array {
    T m_elements[CAPACITY];  // +0x00

    T& operator[](int idx) { return m_elements[idx]; }
    const T& operator[](int idx) const { return m_elements[idx]; }
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
    void ProcessState();  // ?ProcessState@AeThread@@QAEXXZ (scr.o 0x5C8EE0)
    void Execute(float deltaT);  // ?Execute@AeThread@@QAEXM@Z (scr.o 0x5C90A0)
    bool HasEndCond(int notify) const;  // ?HasEndCond@AeThread@@QBE_NH@Z (scr.o 0x5C92B0)

private:
    static PoolAllocator* sAllocator;  // ?sAllocator@AeThread@@0PAVPoolAllocator@@A @ 0x132A0C4
    friend class AeThreadManager;
public:
    static PoolAllocator* GetAllocatorInternal()
    {
        return sAllocator;
    }
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

private:
    static PoolAllocator* sAllocator;  // ?sAllocator@AeThreadState@@0PAVPoolAllocator@@A @ 0x132A0D4
    friend struct AeThreadStateAllocAccess;
};

struct AeThreadStateAllocAccess {
    static PoolAllocator* Get() { return AeThreadState::sAllocator; }
};

PoolAllocator* AeThreadState::sAllocator;

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

// EndOnScriptNode (scr.o / AeThread.cpp; IDA type 5678)
class EndOnScriptNode {
public:
    AeDListNode m_dlist_node;  // +0x00
    Handle mThread;            // +0x08

    EndOnScriptNode(AeThread* t);  // ??0EndOnScriptNode@@QAE@PAVAeThread@@@Z (0x5DB230)
    AeThread* GetThread();         // ?GetThread@EndOnScriptNode@@QAEPAVAeThread@@XZ (0x5C9850)

private:
    static PoolAllocator* sAllocator;  // ?sAllocator@EndOnScriptNode@@0PAVPoolAllocator@@A
    friend struct AeThreadEntityNotifyState;
};

// EntityNotifySet local view (core_systems.h 0x2C bytes)
class EntityNotifySetLocal {
public:
    AeDListNode m_dlist_node;   // +0x00
    DbLinkedHandle<void, void> mEnt;  // +0x08
    AeStateList mStrings;       // +0x0C
    AeStateList mEndOnList;     // +0x1C

    EntityNotifySetLocal(Entity* e);  // ??0EntityNotifySet@@QAE@PAVEntity@@@Z
    EntityNotify* GetNotify(const HashString& chk) const;  // ?GetNotify@EntityNotifySet@@QBEPAVEntityNotify@@ABVHashString@@@Z

private:
    static PoolAllocator* sAllocator;  // ?sAllocator@EntityNotifySet@@0PAVPoolAllocator@@A
    friend struct AeThreadEntityNotifyState;
};

// Entity notify wait states (IDA types 5938/5883/6055)
struct AeThreadEntityNotifyState : AeThreadState {
    DbLinkedHandle<EntityHandleDb, Entity> mEnt;  // +0x14
    HashString mNotifyStr;                        // +0x18
    EndOnScriptNode* mEndOnNode;                  // +0x1C

    AeThreadEntityNotifyState(
        DbLinkedHandle<EntityHandleDb, Entity> ent, unsigned int label,
        AeThreadState::EAction result);  // 0x5DB280
    ~AeThreadEntityNotifyState();   // 0x5C9880
    EAction NewAction(AeThread& t) override;  // 0x5C9920
    void GetCondText(ae_fixed_string<64, unsigned char>& str) override;  // 0x5DF580
    void GetDebugTxt(ae_fixed_string<64, unsigned char>& str) override;  // 0x5DF680
};

struct AeThreadEntityNotifyTimeoutState : AeThreadState {
    DbLinkedHandle<EntityHandleDb, Entity> mEnt;  // +0x14
    HashString mNotifyStr;                        // +0x18
    float mTimeRemaining;                         // +0x1C

    AeThreadEntityNotifyTimeoutState(
        DbLinkedHandle<EntityHandleDb, Entity> ent, unsigned int label,
        float t, AeThreadState::EAction result);  // 0x5C1F40
    EAction NewAction(AeThread& t) override;  // 0x5C99A0
    void GetCondText(ae_fixed_string<64, unsigned char>& str) override;  // 0x5DF780
    void GetDebugTxt(ae_fixed_string<64, unsigned char>& str) override;  // 0x5DF8B0
};

struct AeThreadEntityNotifyMatchState : AeThreadState {
    DbLinkedHandle<EntityHandleDb, Entity> mEnt;  // +0x14
    ae_array<HashString, 4> mNotifySet;           // +0x18
    int mEventMask;                               // +0x28
    bool mWaitForAll;                             // +0x2C
    ae_array<HashString, 4> mDebugNotifys;        // +0x30

    AeThreadEntityNotifyMatchState(
        DbLinkedHandle<EntityHandleDb, Entity> ent, unsigned int label1,
        unsigned int label2, unsigned int label3, unsigned int label4,
        AeThreadState::EAction result, bool waitForAll);  // 0x5C7C10
    EAction NewAction(AeThread& t) override;  // 0x5C9A40
    void GetCondText(ae_fixed_string<64, unsigned char>& str) override;  // 0x5DF9E0
    void GetDebugTxt(ae_fixed_string<64, unsigned char>& str) override;  // 0x5DFB40
};

// AeThread execution core decls (batch 17)
void AeThread_ProcessState(AeThread* self);   // ?ProcessState@AeThread@@QAEXXZ (0x5C8EE0)
void AeThread_Execute(AeThread* self, float deltaT);  // ?Execute@AeThread@@QAEXM@Z (0x5C90A0)
bool AeThread_HasEndCond(AeThread* self, int notify);  // ?HasEndCond@AeThread@@QBE_NH@Z (0x5C92B0)

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
    void PlayIndoor(const char* name, float fadeInTime);  // ?PlayIndoor@MusicMgr@@QAEXPBDM@Z
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
unsigned int GetEntByFieldAndHash(int offsetIntoEnt, unsigned int hValue,
                                  unsigned int* array, int capacity);  // 0x5DCE70
unsigned int GetEntTarget(unsigned int hValue, unsigned int* array,
                          int capacity);  // 0x5DCF80
unsigned int GetEntTargetName(unsigned int hValue, unsigned int* array,
                              int capacity);  // 0x5DCFA0
unsigned int GetEntClassname(unsigned int hValue, unsigned int* array,
                             int capacity);  // 0x5DCFC0
unsigned int GetEntNoteWorthy(unsigned int hValue, unsigned int* array,
                              int capacity);  // 0x5DCFE0
unsigned int GetEntGroup(unsigned int hValue, unsigned int* array,
                         int capacity);  // 0x5DD000
void RegisterHashString(int hash, const char* txt);  // 0x5DFC50
unsigned int ThreadCreateInternal(const char* file, int line, const char* func,
                                  unsigned int ehandle,
                                  AeThreadFunctor* functor,
                                  bool create_handle);  // 0x5DB3C0
unsigned int ThreadExecInternal(const char* file, int line, const char* func,
                                unsigned int ehandle, AeThreadFunctor* functor,
                                bool create_handle);  // 0x5DB470
unsigned int ThreadNotifyInternal(const char* file, int line, const char* func,
                                  unsigned int ehandle, unsigned int notifyEnt,
                                  unsigned int notify,
                                  AeThreadFunctor* functor);  // 0x5DB540
void Mover_SetupMove(trajectory_t* pTr, const math::Position3& vPos,
                     float fTotalTime, float fAccelTime, float fDecelTime,
                     math::Position3& vCurrPos, float* pfSpeed,
                     float* pfMidTime, float* pfDecelTime,
                     math::Position3& vPos1, math::Position3& vPos2,
                     math::Position3& vPos3);  // 0x5BFBF0
void Mover_Move(Entity* pEnt, const math::Position3& vPos, float fTotalTime,
                float fAccelTime, float fDecelTime);  // 0x5C08B0
void Mover_Rotate(Entity* pEnt, const math::Position3& vRot,
                  float fTotalTime, float fAccelTime,
                  float fDecelTime);  // 0x5C0A30
void MoveAxis(unsigned int entityHandleVal, int iAxis, float fMove,
              float fTotalTime, float fAccelTime, float fDecelTime);  // 0x5D3180
void RotateAxis(unsigned int entityHandleVal, int iAxis, float fMove,
                float fTotalTime, float fAccelTime,
                float fDecelTime);  // 0x5D3270
void MoveX(unsigned int entityHandleVal, float fMove, float fTotalTime,
           float fAccelTime, float fDecelTime);  // 0x5D34D0
void MoveY(unsigned int entityHandleVal, float fMove, float fTotalTime,
           float fAccelTime, float fDecelTime);  // 0x5D3500
void MoveZ(unsigned int entityHandleVal, float fMove, float fTotalTime,
           float fAccelTime, float fDecelTime);  // 0x5D3530
void RotatePitch(unsigned int entityHandleVal, float fMove, float fTotalTime,
                 float fAccelTime, float fDecelTime);  // 0x5D3980
void RotateYaw(unsigned int entityHandleVal, float fMove, float fTotalTime,
               float fAccelTime, float fDecelTime);  // 0x5D39B0
void RotateRoll(unsigned int entityHandleVal, float fMove, float fTotalTime,
                float fAccelTime, float fDecelTime);  // 0x5D39E0
void SetShowTime(float time);  // 0x5BC5B0
int  ActiveMenu();             // 0x5BC660
bool Error(const char* file, int line, const char* msg);  // 0x5BC930
void Scr_ReadOnlyField(Entity* ent, int offset, void* val);  // 0x5BE7D0
void SentientScr_ReadOnly(sentient_s* pSelf, int offset, void* val);  // 0x5BF4E0
void MPScript_GetWeaponName(unsigned int weaponIndex,
                            Broc::string& weapon);  // 0x5C1120
void ThreadEntityNotify(unsigned int entityHandleVal, int notifyId);  // 0x5C9ED0
void ThreadEntityNotify(unsigned int entityHandleVal, int notifyId,
                        unsigned int entityOut);  // 0x5C9F90
void ThreadEntityNotify(unsigned int entityHandleVal, int notifyId,
                        const Broc::string& strOut);  // 0x5CA040
void ThreadEntityNotify(unsigned int entityHandleVal, int notifyId,
                        int intOut);  // 0x5CA0F0
void ThreadEntityNotify(unsigned int entityHandleVal, int notifyId,
                        float floatOut, unsigned int outEnt);  // 0x5CA1A0
void ObjectiveCompletedNotify();  // ?ObjectiveCompletedNotify@@YAXXZ (0x5C18C0)
void MusicIndoorPlay(const Broc::string& pszSoundName, float fadeInTime);  // 0x5C3370
void ReverbSetParams(const Broc::string& name, bool immediate);  // 0x5C35D0
void LoadWbk(const Broc::string& name);  // 0x5C3610
void FreeWbk(const Broc::string& name);  // 0x5C3650
int  WeaponClipSize(const Broc::string& pszWeaponName);  // 0x5C4520
int  WeaponIsSemiAuto(const Broc::string& pszWeaponName);  // 0x5C4560
int  WeaponIsBoltAction(const Broc::string& pszWeaponName);  // 0x5C45A0
int  GetHudElemAllocIndex();  // 0x5C4970
void SetClock(const Broc::hudelem& hudElem, float fTime, float fDur,
              const Broc::string& name, int width, int height);  // 0x5C5360
void SetClockUp(const Broc::hudelem& hudElem, float fTime, float fDur,
                const Broc::string& name, int width, int height);  // 0x5C53A0
void FadeOverTime(const Broc::hudelem& hudElem, float fadeTime);  // 0x5C53E0
void ScaleOverTime(const Broc::hudelem& hudElem, float scaleTime, int width,
                   int height);  // 0x5C5510
void MoveOverTime(const Broc::hudelem& hudElem, float fadeTime);  // 0x5C5650
void Scr_SetOrigin(Entity* ent, int offset, Broc::vector* val);  // 0x5C57E0
int  GetNodeClaimer(const Broc::pathnode& nodeIn);  // 0x5C5AE0
void GetMoveDelta(Broc::vector& outVec, unsigned int anim, float startTime,
                  float endTime);  // 0x5C3C70
float GetAngleDelta(unsigned int anim, float startTime, float endTime);  // 0x5C3DD0
unsigned int SpawnTriggerMount(Broc::vector& mins, Broc::vector& maxs,
                               bool crouch, TPakInfo pakInfo);  // 0x5C2BF0
unsigned int SpawnTurret(const Broc::string& classname,
                         const Broc::vector& origin,
                         const Broc::string& weaponinfoname,
                         TPakInfo pakInfo);  // 0x5C2E00
    unsigned int SpawnTurretWithAngles(const Broc::string& classname,
                                       const Broc::vector& origin,
                                       const Broc::string& weaponinfoname,
                                       const Broc::vector& angles,
                                       TPakInfo pakInfo);  // 0x5C3030
void HudSetDefaults(game_hudelem_s* hud);  // 0x5BD320
void SetValue(const Broc::hudelem& hudElem, float value);  // 0x5BD670
void SetShader(const Broc::hudelem& hudElem, const Broc::string& string,
               int width, int height);  // 0x5C5060
int  NRefsToString(int stringRef);  // 0x5BD460
int  FindConfigString(const char* text);  // 0x5BD620
void SetText(const Broc::hudelem& hudElem, const Broc::string& string);  // 0x5C4EE0
void SetModel(unsigned int entityHandleVal, const Broc::string& modelName,
              TPakInfo pakInfo);  // 0x5CF060
unsigned int SpawnVehicle(const Broc::string& modelname,
                          const Broc::string& targetName,
                          const Broc::string& vehicleType,
                          const Broc::vector& origin,
                          const Broc::vector& angles,
                          TPakInfo pakInfo);  // 0x5CA340
bool Assert(const char* file, int line, const char* msg);  // 0x5BC890
bool Warning(const char* file, int line, const char* msg);  // 0x5BC8E0
void CVarGetString(Broc::string& valueOut, const char* cvarName);  // 0x5BC970
void CVarSetString(const char* cvarName, const char* value);  // 0x5BC9D0
void CVarSetInt(const char* cvarName, int iString);  // 0x5BCA50
void CVarSetFloat(const char* cvarName, float fString);  // 0x5BCAE0
void GetDifficulty(Broc::string& outStr);  // 0x5BCCA0
void Lightning(const Broc::vector& source);  // 0x5BCDE0
void SetMissileActiveTime(float timeSec);  // 0x5BCEA0
int  ProfTick();  // 0x5BCEF0
bool IsPakLoaded(TPakInfo handle);  // 0x5BD070
void SetPakDistance(TPakInfo handle, float dist);  // 0x5BD0D0
void ClearPakDistance(TPakInfo handle);  // 0x5BD130
void SyncLoadPak(TPakInfo handle);  // 0x5BD190
bool GetAnimName(unsigned int anim, char* buff, int buffsize);  // 0x5BC730
void PrintFloat3D(const Broc::vector& pos, float number,
                  const Broc::vector& col, float alpha,
                  float scale);  // 0x5BC790
void Line(const Broc::vector& start, const Broc::vector& end,
          const Broc::vector& col, float alpha,
          int depthtest);  // 0x5BC800
void ScreenFadeToBlack(unsigned int duration, int viewport);  // 0x5BC5F0
const char* Localize(const char* txt);  // 0x5BC6E0
void GetLastParticleInfo(Broc::vector& outVec,
                         unsigned int& outHash);  // 0x5BCC20
void RadiusDamage(const Broc::vector& origin, float range,
                  float max_damage, float min_damage,
                  int damageType);  // 0x5BCD70
TPakInfo GetPakVector(float x, float y, float z);  // 0x5BCF40
void HudFree(game_hudelem_s* hud);  // 0x5BD4D0
void ValidateApiSize(int sizeofBrocAPI, int sizeofBrocExports);  // 0x5BD810
void GlowSetBrightnessAux(float val);  // 0x5BDB40
unsigned int CreateDynamicLight(const Broc::vector& lightpos,
                                const Broc::vector& color,
                                float timeInSeconds, float innerRadius,
                                float outerRadius,
                                bool flicker);  // 0x5BDA40
void SetDynamicLightPosition(unsigned int light,
                             const Broc::vector& lightpos);  // 0x5BD9C0
Broc::string GetLocalizedString(int stringHash);  // 0x5BDAC0
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

// ============================================================================
// scr.o batch 12 - entity-notify wait states
// ============================================================================

unsigned int sKillAnimScript = 0xFFFFFFFF;  // ?sKillAnimScript@@3IA @ 0xF3AC0C (init -1 per IDA bytes)

PoolAllocator* EndOnScriptNode::sAllocator;
PoolAllocator* EntityNotifySetLocal::sAllocator;

// ea: 0x005DB230
EndOnScriptNode::EndOnScriptNode(AeThread* t)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mThread.mVal = 0;
    if (t->mHandle.mVal == 0)
    {
        AeThreadManagerLayout* L =
            (AeThreadManagerLayout*)&AeThreadManager::sInst;
        HandleDb<AeThread, 256, SizedHandle<8, 24>>* db =
            (HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb;
        Handle mVal = db->AllocateHandle();
        db->BindObjectToHandle(mVal, t);
        t->mHandle = mVal;
    }
    mThread = t->mHandle;
}

// ea: 0x005C9850 (mHandleDb at sInst+0x34; index = mVal & 0xFF, key = mVal >> 8)
AeThread* EndOnScriptNode::GetThread()
{
    AeThreadManagerLayout* L =
        (AeThreadManagerLayout*)&AeThreadManager::sInst;
    HandleDb<AeThread, 256, SizedHandle<8, 24>>* db =
        (HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb;
    unsigned int mVal = mThread.mVal;
    unsigned int idx = mVal & 0xFF;
    if (idx < 256 && (mVal >> 8) == (unsigned int)db->mElements[idx].mKey)
        return db->mElements[idx].mObject;
    return nullptr;
}

// ea: 0x005DB280
AeThreadEntityNotifyState::AeThreadEntityNotifyState(
    DbLinkedHandle<EntityHandleDb, Entity> ent, unsigned int label,
    AeThreadState::EAction result)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mFinished = false;
    mResult = result;
    mEnt = ent;
    mNotifyStr.mHash = label;
    mEndOnNode = nullptr;
    if (result != kActionTerminate || label != sKillAnimScript)
        return;
    unsigned int v5 = ent.mHandle.mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v5 < 0x540
        && ent.mHandle.mVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v5].mKey)
        mObject = EntityHandleDb::sInst.mElements[v5].mObject;
    if (mObject == nullptr)
        return;
    EntityNotifySetLocal* mNotifySet =
        (EntityNotifySetLocal*)mObject->mNotifySet;
    if (mNotifySet == nullptr)
    {
        EntityNotifySetLocal* v8 =
            (EntityNotifySetLocal*)EntityNotifySetLocal::sAllocator->Allocate(
                0x2C, false);
        EntityNotifySetLocal* v9 =
            v8 != nullptr ? new (v8) EntityNotifySetLocal(mObject) : nullptr;
        mNotifySet = v9;
        mObject->mNotifySet = (EntityNotifySet*)v9;
    }
    EndOnScriptNode* v10 =
        (EndOnScriptNode*)EndOnScriptNode::sAllocator->Allocate(0x0C, false);
    EndOnScriptNode* v11 = v10 != nullptr ? new (v10) EndOnScriptNode(
                                                (AeThread*)AeThreadManager::
                                                    sInst.mThreadExecuting)
                                          : nullptr;
    mEndOnNode = v11;
    if (v11 != nullptr)
    {
        v11->m_dlist_node.mPrev = mNotifySet->mEndOnList.m_tail;
        ((AeDListNode*)mNotifySet->mEndOnList.m_tail)->mNext =
            &v11->m_dlist_node;
        mNotifySet->mEndOnList.m_tail = &v11->m_dlist_node;
        ++mNotifySet->mEndOnList.m_size;
    }
}

// ea: 0x005C9880
AeThreadEntityNotifyState::~AeThreadEntityNotifyState()
{
    EndOnScriptNode* mEndOnNode = this->mEndOnNode;
    if (mEndOnNode != nullptr)
    {
        unsigned int mVal = mEnt.mHandle.mVal;
        unsigned int v4 = mVal & 0xFFF;
        if (v4 < 0x540
            && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey)
        {
            Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
            if (mObject != nullptr)
            {
                EntityNotifySetLocal* mNotifySet =
                    (EntityNotifySetLocal*)mObject->mNotifySet;
                if (mNotifySet != nullptr)
                {
                    // reserved_dlist<EndOnScriptNode>::erase
                    mEndOnNode->m_dlist_node.mPrev->mNext =
                        mEndOnNode->m_dlist_node.mNext;
                    mEndOnNode->m_dlist_node.mNext->mPrev =
                        mEndOnNode->m_dlist_node.mPrev;
                    --mNotifySet->mEndOnList.m_size;
                }
            }
        }
        EndOnScriptNode::sAllocator->Release(mEndOnNode);
    }
}

// ea: 0x005C9920
AeThreadState::EAction AeThreadEntityNotifyState::NewAction(AeThread& t)
{
    unsigned int mVal = mEnt.mHandle.mVal;
    unsigned int v4 = mVal & 0xFFF;
    if (v4 < 0x540
        && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != nullptr)
        {
            if ((t.mFlags.mMask & 0x80) != 0
                && mObject->mNotifySet != nullptr
                && ((EntityNotifySetLocal*)mObject->mNotifySet)
                       ->GetNotify(mNotifyStr) != nullptr)
            {
                mFinished = true;
                return mResult;
            }
            return kActionNone;
        }
    }
    mFinished = true;
    return kActionTerminate;
}

// ============================================================================
// scr.o batch 13 - GetEnt* entity-lookup family (BrocEntity.cpp)
// ============================================================================

// ea: 0x005DCE70 (mangle YAIHIPAIH: int, uint, uint*, int)
static const char* BrocSysHashLookup(unsigned int hash);

unsigned int BrocSys::GetEntByFieldAndHash(int offsetIntoEnt,
                                           unsigned int hValue,
                                           unsigned int* array, int capacity)
{
    (void)capacity;
    unsigned int result = 0;
    AeSizedEntityArray& active = EntityHandleDb::sInst.mActiveList;
    Entity** p = active.m_elements;
    Entity** end = p + active.m_size;
    unsigned int entHandleToRet = 0;
    int entityCount = 0;
    while (p != end)
    {
        Entity* v6 = *p;
        if (v6 != nullptr
            && *(unsigned int*)((char*)&v6->s.eType + offsetIntoEnt)
                   == hValue)
        {
            if (array != nullptr)
            {
                array[result++] = v6->mHandle.mHandle.mVal;
            }
            else
            {
                if (v6->scr_vehicle != nullptr)
                    v6->scr_vehicle->playEngineSound = 1;
                entHandleToRet = v6->mHandle.mHandle.mVal;
                ++entityCount;
            }
        }
        ++p;
    }
    if (array == nullptr)
    {
        if (entityCount > 1)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 394;
            AeAssert::gCurrentExpr = "entityCount<=1";
            if (!AeAssert::IsIgnored())
            {
                const char* v9 = BrocSysHashLookup(hValue);
                if (AeAssert::Assert(
                        "More than one entity with key %s value exists", v9))
                    __debugbreak();
            }
        }
        return entHandleToRet;
    }
    return result;
}

// ea: 0x005DCF80
unsigned int BrocSys::GetEntTarget(unsigned int hValue, unsigned int* array,
                                  int capacity)
{
    return BrocSys::GetEntByFieldAndHash(656, hValue, array, capacity);
}

// ea: 0x005DCFA0
unsigned int BrocSys::GetEntTargetName(unsigned int hValue,
                                       unsigned int* array, int capacity)
{
    return BrocSys::GetEntByFieldAndHash(648, hValue, array, capacity);
}

// ea: 0x005DCFC0
unsigned int BrocSys::GetEntClassname(unsigned int hValue,
                                      unsigned int* array, int capacity)
{
    return BrocSys::GetEntByFieldAndHash(640, hValue, array, capacity);
}

// ea: 0x005DCFE0
unsigned int BrocSys::GetEntNoteWorthy(unsigned int hValue,
                                       unsigned int* array, int capacity)
{
    return BrocSys::GetEntByFieldAndHash(672, hValue, array, capacity);
}

// ea: 0x005DD000
unsigned int BrocSys::GetEntGroup(unsigned int hValue, unsigned int* array,
                                  int capacity)
{
    return BrocSys::GetEntByFieldAndHash(664, hValue, array, capacity);
}

// ============================================================================
// scr.o batch 14 - sHashStrings map + RegisterHashString + debug text
// ============================================================================

// BrocSys::sHashStrings (binary: stdext::hash_map<int,
// ae_fixed_string<32,unsigned char>> at 0xF3B478). Win32 port: fixed-capacity
// linear-probe map preserving register/lookup semantics.
class BrocSysHashStrings {
public:
    struct Entry {
        unsigned int mHash;                        // +0x00
        ae_fixed_string<32, unsigned char> mStr;   // +0x04
    };

    Entry mEntries[2048];  // +0x00
    int   mCount;          // +0x14000

    BrocSysHashStrings() : mCount(0) {}

    Entry* find(unsigned int hash)
    {
        for (int i = 0; i < mCount; ++i)
        {
            if (mEntries[i].mHash == hash)
                return &mEntries[i];
        }
        return nullptr;
    }

    const char* lookup(unsigned int hash)
    {
        Entry* e = find(hash);
        return e != nullptr ? (const char*)e->mStr.mBuff : nullptr;
    }

    void set(unsigned int hash, const ae_fixed_string<32, unsigned char>& s)
    {
        Entry* e = find(hash);
        if (e != nullptr)
        {
            e->mStr = s;
            return;
        }
        if (mCount < 2048)
        {
            mEntries[mCount].mHash = hash;
            mEntries[mCount].mStr = s;
            ++mCount;
        }
    }
};

// Binary: ?sHashStrings@BrocSys@@3V?$hash_map@...@@A @ 0xF3B478
BrocSysHashStrings sHashStrings;

// ea: 0x005DFD00
void BrocSys::RegisterHashString(int hash, const char* txt)
{
    BrocSysHashStrings::Entry* existing = sHashStrings.find((unsigned int)hash);
    if (existing != nullptr
        && _strnicmp((const char*)existing->mStr.mBuff, txt, 31) != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 409;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Hash collision: '%s' and '%s'", txt,
                                 existing->mStr.mBuff))
            __debugbreak();
    }
    ae_fixed_string<32, unsigned char> val(txt);
    sHashStrings.set((unsigned int)hash, val);
}

// Real lookup backing GetEntByFieldAndHash's error path
static const char* BrocSysHashLookup(unsigned int hash)
{
    return sHashStrings.lookup(hash);
}

// ea: 0x005DF580
void AeThreadEntityNotifyState::GetCondText(
    ae_fixed_string<64, unsigned char>& str)
{
    if (mResult == kActionWakeUp)
    {
        int len = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len, 63, ",(w) ");
        str.mLength = (unsigned char)len;
        const char* s = sHashStrings.lookup(mNotifyStr.mHash);
        int len2 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len2, 63,
                                s != nullptr ? s : "");
        str.mLength = (unsigned char)len2;
    }
    else if (mResult == kActionSleep)
    {
        int len = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len, 63, "(s) ");
        str.mLength = (unsigned char)len;
        const char* s = sHashStrings.lookup(mNotifyStr.mHash);
        int len2 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len2, 63,
                                s != nullptr ? s : "");
        str.mLength = (unsigned char)len2;
    }
}

// ea: 0x005DF680
void AeThreadEntityNotifyState::GetDebugTxt(
    ae_fixed_string<64, unsigned char>& str)
{
    const char* prefix = nullptr;
    if (mResult == kActionWakeUp)
        prefix = "waking up for ";
    else if (mResult == kActionTerminate)
        prefix = "terminating for ";
    else
        return;
    int len = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len, 63, prefix);
    str.mLength = (unsigned char)len;
    const char* s = sHashStrings.lookup(mNotifyStr.mHash);
    int len2 = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len2, 63,
                            s != nullptr ? s : "");
    str.mLength = (unsigned char)len2;
}

// ea: 0x005DF780
void AeThreadEntityNotifyTimeoutState::GetCondText(
    ae_fixed_string<64, unsigned char>& str)
{
    ae_formatted_string<64, unsigned char> v10(",(s) %04.3f", mTimeRemaining);
    int len = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len, 63, (const char*)v10.mBuff);
    str.mLength = (unsigned char)len;
    if (mResult == kActionWakeUp)
    {
        int len2 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len2, 63, ",(w) ");
        str.mLength = (unsigned char)len2;
        const char* s = sHashStrings.lookup(mNotifyStr.mHash);
        int len3 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len3, 63,
                                s != nullptr ? s : "");
        str.mLength = (unsigned char)len3;
    }
    else if (mResult == kActionSleep)
    {
        int len2 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len2, 63, "(s) ");
        str.mLength = (unsigned char)len2;
        const char* s = sHashStrings.lookup(mNotifyStr.mHash);
        int len3 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len3, 63,
                                s != nullptr ? s : "");
        str.mLength = (unsigned char)len3;
    }
}

// ea: 0x005DF8B0
void AeThreadEntityNotifyTimeoutState::GetDebugTxt(
    ae_fixed_string<64, unsigned char>& str)
{
    if (mResult == kActionWakeUp)
    {
        int len = str.mLength;
        if (mTimeRemaining <= 0.0f)
        {
            AeStringSupport::Concat((char*)str.mBuff, len, 63,
                                    "waking up from time wait");
            str.mLength = (unsigned char)len;
            return;
        }
        AeStringSupport::Concat((char*)str.mBuff, len, 63, "waking up for ");
        str.mLength = (unsigned char)len;
        const char* s = sHashStrings.lookup(mNotifyStr.mHash);
        int len2 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len2, 63,
                                s != nullptr ? s : "");
        str.mLength = (unsigned char)len2;
    }
    else if (mResult == kActionTerminate)
    {
        int len = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len, 63, "terminating for ");
        str.mLength = (unsigned char)len;
        const char* s = sHashStrings.lookup(mNotifyStr.mHash);
        int len2 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len2, 63,
                                s != nullptr ? s : "");
        str.mLength = (unsigned char)len2;
    }
}

// ea: 0x005DF9E0
void AeThreadEntityNotifyMatchState::GetCondText(
    ae_fixed_string<64, unsigned char>& str)
{
    if (mResult != kActionWakeUp)
        return;
    int len = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len, 63, ",(w) ");
    str.mLength = (unsigned char)len;
    for (int i = 0; i < 4; ++i)
    {
        if (mDebugNotifys[i].mHash == 0)
            break;
        if (i != 0)
        {
            int len2 = str.mLength;
            AeStringSupport::Concat((char*)str.mBuff, len2, 63, " & ");
            str.mLength = (unsigned char)len2;
        }
        const char* s = sHashStrings.lookup(mDebugNotifys[i].mHash);
        int len2 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len2, 63,
                                s != nullptr ? s : "");
        str.mLength = (unsigned char)len2;
        if ((mEventMask & (1 << i)) == 0)
        {
            int len3 = str.mLength;
            AeStringSupport::Concat((char*)str.mBuff, len3, 63, "(0)");
            str.mLength = (unsigned char)len3;
        }
        else
        {
            int len3 = str.mLength;
            AeStringSupport::Concat((char*)str.mBuff, len3, 63, "(1)");
            str.mLength = (unsigned char)len3;
        }
    }
}

// ea: 0x005DFB40
void AeThreadEntityNotifyMatchState::GetDebugTxt(
    ae_fixed_string<64, unsigned char>& str)
{
    if (mResult != kActionWakeUp)
        return;
    int len = str.mLength;
    AeStringSupport::Concat((char*)str.mBuff, len, 63, "waking up for ");
    str.mLength = (unsigned char)len;
    for (int i = 0; i < 4; ++i)
    {
        if (i != 0)
        {
            int len2 = str.mLength;
            AeStringSupport::Concat((char*)str.mBuff, len2, 63, " and ");
            str.mLength = (unsigned char)len2;
        }
        const char* s = sHashStrings.lookup(mDebugNotifys[i].mHash);
        int len2 = str.mLength;
        AeStringSupport::Concat((char*)str.mBuff, len2, 63,
                                s != nullptr ? s : "");
        str.mLength = (unsigned char)len2;
    }
}

// ea: 0x005BE180 (rep stosd 0x8C dwords = 0x230 bytes = 70 entries)
BrocHelper::brocFunctionLookup BrocHelper::broFuncLookupTable[70];
void BrocHelper::Init()
{
    memset(BrocHelper::broFuncLookupTable, 0,
           sizeof(BrocHelper::broFuncLookupTable));
    BrocHelper::m_treeCount = 0;
}

// ============================================================================
// scr.o batch 15 - thread create/exec/notify internals
// ============================================================================

// ea: 0x005DB3C0
unsigned int BrocSys::ThreadCreateInternal(const char* file, int line,
                                           const char* func,
                                           unsigned int ehandle,
                                           AeThreadFunctor* functor,
                                           bool create_handle)
{
    AeThread* v6 = (AeThread*)AeThread::GetAllocatorInternal()->Allocate(
        0x50, false);
    AeThread* v7 = v6 != nullptr
                       ? new (v6) AeThread(file, line, func, ehandle, functor,
                                           false)
                       : nullptr;
    AeThreadManager::sInst.AddThread(v7);
    if (create_handle)
    {
        AeThreadManagerLayout* L =
            (AeThreadManagerLayout*)&AeThreadManager::sInst;
        HandleDb<AeThread, 256, SizedHandle<8, 24>>* db =
            (HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb;
        Handle mVal = db->AllocateHandle();
        db->BindObjectToHandle(mVal, v7);
        v7->mHandle = mVal;
    }
    return v7->mHandle.mVal;
}

// ea: 0x005DB470
unsigned int BrocSys::ThreadExecInternal(const char* file, int line,
                                         const char* func,
                                         unsigned int ehandle,
                                         AeThreadFunctor* functor,
                                         bool create_handle)
{
    AeThread* v6 = (AeThread*)AeThread::GetAllocatorInternal()->Allocate(
        0x50, false);
    AeThread* v7 = v6 != nullptr
                       ? new (v6) AeThread(file, line, func, ehandle, functor,
                                           false)
                       : nullptr;
    AeThreadManagerLayout* L =
        (AeThreadManagerLayout*)&AeThreadManager::sInst;
    v7->m_dlist_node.mNext = L->mExecThreads.m_end;
    v7->m_dlist_node.mPrev = L->mExecThreads.m_tail;
    ((AeDListNode*)L->mExecThreads.m_tail)->mNext = &v7->m_dlist_node;
    L->mExecThreads.m_tail = &v7->m_dlist_node;
    ++L->mExecThreads.m_size;
    L->mNewThreadExec = v7;
    v7->mFlags.mMask |= 0x800;
    if (create_handle)
    {
        HandleDb<AeThread, 256, SizedHandle<8, 24>>* db =
            (HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb;
        Handle mVal = db->AllocateHandle();
        db->BindObjectToHandle(mVal, v7);
        v7->mHandle = mVal;
    }
    return v7->mHandle.mVal;
}

// ea: 0x005DB540
unsigned int BrocSys::ThreadNotifyInternal(const char* file, int line,
                                           const char* func,
                                           unsigned int ehandle,
                                           unsigned int notifyEnt,
                                           unsigned int notify,
                                           AeThreadFunctor* functor)
{
    AeThread* v7 = (AeThread*)AeThread::GetAllocatorInternal()->Allocate(
        0x50, false);
    AeThread* v8 = v7 != nullptr
                       ? new (v7) AeThread(file, line, func, ehandle, functor,
                                           false)
                       : nullptr;
    AeThreadEntityNotifyState* v9 =
        (AeThreadEntityNotifyState*)AeThreadStateAllocAccess::Get()->Allocate(
            0x20, false);
    AeThreadEntityNotifyState* v10 =
        v9 != nullptr
            ? new (v9) AeThreadEntityNotifyState(
                  DbLinkedHandle<EntityHandleDb, Entity>(Handle(notifyEnt)),
                  notify, AeThreadState::kActionWakeUp)
            : nullptr;
    unsigned int v11 = v8->mFlags.mMask | 2;
    v8->mFlags.mMask = v11;
    v8->mFlags.mMask = v11 | 0x10;
    AeDListNode* p_node = &v10->m_dlist_node;
    v8->mFlags.mMask = v11 | 0x210;
    p_node->mNext = v8->mStateControllers.m_end;
    p_node->mPrev = v8->mStateControllers.m_tail;
    ((AeDListNode*)v8->mStateControllers.m_tail)->mNext = p_node;
    ++v8->mStateControllers.m_size;
    v8->mStateControllers.m_tail = p_node;
    AeThreadManager::sInst.AddThread(v8);
    return 0;
}

// ============================================================================
// scr.o batch 16 - mover cluster + misc wrappers
// ============================================================================

extern unsigned int AeHash(const char* str);  // ae_hash.cpp (core_xboxr)

// ea: 0x005BFBF0
void BrocSys::Mover_SetupMove(trajectory_t* pTr, const math::Position3& vPos,
                              float fTotalTime, float fAccelTime,
                              float fDecelTime, math::Position3& vCurrPos,
                              float* pfSpeed, float* pfMidTime,
                              float* pfDecelTime, math::Position3& vPos1,
                              math::Position3& vPos2, math::Position3& vPos3)
{
    bool stationary = pTr->trType == TR_STATIONARY;
    float vMove = vPos.v.m128_f32[0] - vCurrPos.v.m128_f32[0];
    float v35 = vPos.v.m128_f32[1] - vCurrPos.v.m128_f32[1];
    float v36 = vPos.v.m128_f32[2] - vCurrPos.v.m128_f32[2];
    if (!stationary)
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    if (fAccelTime == 0.0f && fDecelTime == 0.0f)
    {
        pTr->trTime = level.time;
        pTr->trDuration = (int)(fTotalTime * 1000.0f);
        *pfMidTime = fTotalTime;
        *pfDecelTime = 0.0f;
        vPos3.v.m128_f32[0] = vPos.v.m128_f32[0];
        vPos3.v.m128_f32[1] = vPos.v.m128_f32[1];
        vPos3.v.m128_f32[2] = vPos.v.m128_f32[2];
        pTr->trBase[0] = vCurrPos.v.m128_f32[0];
        pTr->trBase[1] = vCurrPos.v.m128_f32[1];
        pTr->trBase[2] = vCurrPos.v.m128_f32[2];
        if (pTr->trDuration == 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityMove.cpp";
            AeAssert::gCurrentLine = 76;
            AeAssert::gCurrentExpr = "pTr->trDuration";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        float v15 = 1000.0f / pTr->trDuration;
        float v16 = v15 * vMove;
        pTr->trDelta[0] = v15 * vMove;
        pTr->trDelta[1] = v15 * v35;
        pTr->trDelta[2] = v15 * v36;
        if (IS_NAN(v16) || IS_NAN(pTr->trDelta[1])
            || IS_NAN(pTr->trDelta[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityMove.cpp";
            AeAssert::gCurrentLine = 79;
            AeAssert::gCurrentExpr = "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        pTr->trType = TR_LINEAR_STOP;
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    }
    else
    {
        float v19 = v36 * v36 + v35 * v35 + vMove * vMove;
        *pfMidTime = (fTotalTime - fAccelTime) - fDecelTime;
        *pfDecelTime = fDecelTime;
        float fDist = sqrtf(v19);
        if (((fTotalTime * 2.0f) - fAccelTime) - fDecelTime == 0.0f)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityMove.cpp";
            AeAssert::gCurrentLine = 91;
            AeAssert::gCurrentExpr =
                "(2.0f * fTotalTime) - fAccelTime - fDecelTime";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        *pfSpeed = (fDist * 2.0f)
                   / (((fTotalTime * 2.0f) - fAccelTime) - fDecelTime);
        float unit[3];
        VectorNormalize2((const float*)&vMove, unit);
        float v20 = *pfSpeed * unit[0];
        float v21 = *pfSpeed * unit[1];
        float v22 = *pfSpeed * unit[2];
        if (fAccelTime == 0.0f)
        {
            vPos1.v.m128_f32[0] = vCurrPos.v.m128_f32[0];
            vPos1.v.m128_f32[1] = vCurrPos.v.m128_f32[1];
            vPos1.v.m128_f32[2] = vCurrPos.v.m128_f32[2];
            if (*pfMidTime == 0.0f)
            {
                float v30 = v20;
                pTr->trTime = level.time;
                pTr->trDuration = (int)(*pfDecelTime * 1000.0f);
                pTr->trBase[0] = vCurrPos.v.m128_f32[0];
                pTr->trBase[1] = vCurrPos.v.m128_f32[1];
                pTr->trBase[2] = vCurrPos.v.m128_f32[2];
                pTr->trDelta[0] = v20;
                pTr->trDelta[1] = v21;
                pTr->trDelta[2] = v22;
                if (IS_NAN(v30) || IS_NAN(pTr->trDelta[1])
                    || IS_NAN(pTr->trDelta[2]))
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\BrocEntityMove.cpp";
                    AeAssert::gCurrentLine = 134;
                    AeAssert::gCurrentExpr = "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid vector"))
                        __debugbreak();
                }
                pTr->trType = TR_DECCELERATE;
            }
            else
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(*pfMidTime * 1000.0f);
                pTr->trBase[0] = vCurrPos.v.m128_f32[0];
                pTr->trBase[1] = vCurrPos.v.m128_f32[1];
                pTr->trBase[2] = vCurrPos.v.m128_f32[2];
                int v25 = pTr->trDuration;
                float v26 = *pfMidTime * v20;
                float v27 = *pfMidTime * v21;
                float v28 = *pfMidTime * v22;
                if (v25 == 0)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\BrocEntityMove.cpp";
                    AeAssert::gCurrentLine = 120;
                    AeAssert::gCurrentExpr = "pTr->trDuration";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                float v29 = 1000.0f / pTr->trDuration;
                pTr->trDelta[0] = v29 * v26;
                pTr->trDelta[1] = v29 * v27;
                pTr->trDelta[2] = v29 * v28;
                if (IS_NAN(v29 * v26) || IS_NAN(pTr->trDelta[1])
                    || IS_NAN(pTr->trDelta[2]))
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\BrocEntityMove.cpp";
                    AeAssert::gCurrentLine = 123;
                    AeAssert::gCurrentExpr = "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid vector"))
                        __debugbreak();
                }
                pTr->trType = TR_LINEAR_STOP;
            }
        }
        else
        {
            pTr->trTime = level.time;
            pTr->trDuration = (int)(fAccelTime * 1000.0f);
            pTr->trBase[0] = vCurrPos.v.m128_f32[0];
            pTr->trBase[1] = vCurrPos.v.m128_f32[1];
            pTr->trBase[2] = vCurrPos.v.m128_f32[2];
            pTr->trDelta[0] = v20;
            pTr->trDelta[1] = v21;
            pTr->trDelta[2] = v22;
            if (IS_NAN(v20) || IS_NAN(pTr->trDelta[1])
                || IS_NAN(pTr->trDelta[2]))
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\BrocEntityMove.cpp";
                AeAssert::gCurrentLine = 103;
                AeAssert::gCurrentExpr = "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid vector"))
                    __debugbreak();
            }
            int v24 = pTr->trDuration;
            pTr->trType = TR_ACCELERATE;
            BG_EvaluateTrajectory(pTr, level.time + v24, vPos1);
        }
        vPos2.v.m128_f32[0] = (*pfMidTime * v20) + vPos1.v.m128_f32[0];
        vPos2.v.m128_f32[1] = (*pfMidTime * v21) + vPos1.v.m128_f32[1];
        vPos2.v.m128_f32[2] = (*pfMidTime * v22) + vPos1.v.m128_f32[2];
        vPos3.v.m128_f32[0] = vPos.v.m128_f32[0];
        vPos3.v.m128_f32[1] = vPos.v.m128_f32[1];
        vPos3.v.m128_f32[2] = vPos.v.m128_f32[2];
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    }
}

// ea: 0x005C08B0
void BrocSys::Mover_Move(Entity* pEnt, const math::Position3& vPos,
                         float fTotalTime, float fAccelTime, float fDecelTime)
{
    BrocSys::Mover_SetupMove(&pEnt->s.pos, vPos, fTotalTime, fAccelTime,
                             fDecelTime, pEnt->r.currentOrigin, &pEnt->speed,
                             &pEnt->wait, &pEnt->delay, pEnt->pos1,
                             pEnt->pos2, pEnt->pos3);
    g_LinkEntity(pEnt);
}

// ea: 0x005C0A30
void BrocSys::Mover_Rotate(Entity* pEnt, const math::Position3& vRot,
                           float fTotalTime, float fAccelTime,
                           float fDecelTime)
{
    BrocSys::Mover_SetupMove(&pEnt->s.apos, vRot, fTotalTime, fAccelTime,
                             fDecelTime, pEnt->r.currentAngles,
                             &pEnt->closespeed, &pEnt->angle, &pEnt->random,
                             pEnt->movedir, pEnt->rotate, pEnt->TargetAngles);
    g_LinkEntity(pEnt);
}

// ea: 0x005D3180
void BrocSys::MoveAxis(unsigned int entityHandleVal, int iAxis, float fMove,
                       float fTotalTime, float fAccelTime, float fDecelTime)
{
    unsigned int v7 = entityHandleVal & 0xFFF;
    if (v7 < 0x540
        && entityHandleVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v7].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v7].mObject;
        if (mObject != nullptr)
        {
            unsigned int mHash = mObject->mClassNameHash.mHash;
            if (mHash == hash_const.script_brushmodel.mHash
                || mHash == hash_const.script_model.mHash
                || mHash == hash_const.script_origin.mHash)
            {
                if ((mObject->flags & 0x10) == 0)
                {
                    math::Position3 v11;
                    v11.v.m128_f32[0] =
                        mObject->r.currentOrigin.v.m128_f32[0];
                    v11.v.m128_f32[1] =
                        mObject->r.currentOrigin.v.m128_f32[1];
                    v11.v.m128_f32[2] =
                        mObject->r.currentOrigin.v.m128_f32[2];
                    v11.v.m128_f32[iAxis] =
                        v11.v.m128_f32[iAxis] + fMove;
                    BrocSys::Mover_Move(mObject, v11, fTotalTime, fAccelTime,
                                        fDecelTime);
                }
            }
            else
            {
                Scr_Error(va("entity is not a script_brushmodel, script_model, or script_origin"));
            }
        }
    }
}

// ea: 0x005D3270
void BrocSys::RotateAxis(unsigned int entityHandleVal, int iAxis, float fMove,
                         float fTotalTime, float fAccelTime, float fDecelTime)
{
    unsigned int v7 = entityHandleVal & 0xFFF;
    if (v7 < 0x540
        && entityHandleVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v7].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v7].mObject;
        if (mObject != nullptr)
        {
            unsigned int mHash = mObject->mClassNameHash.mHash;
            if (mHash == hash_const.script_brushmodel.mHash
                || mHash == hash_const.script_model.mHash
                || mHash == hash_const.script_origin.mHash)
            {
                if ((mObject->flags & 0x10) == 0)
                {
                    math::Position3 v11;
                    v11.v.m128_f32[0] =
                        mObject->r.currentAngles.v.m128_f32[0];
                    v11.v.m128_f32[1] =
                        mObject->r.currentAngles.v.m128_f32[1];
                    v11.v.m128_f32[2] =
                        mObject->r.currentAngles.v.m128_f32[2];
                    v11.v.m128_f32[iAxis] =
                        v11.v.m128_f32[iAxis] + fMove;
                    BrocSys::Mover_Rotate(mObject, v11, fTotalTime, fAccelTime,
                                          fDecelTime);
                }
            }
            else
            {
                Scr_Error(va("entity is not a script_brushmodel, script_model, or script_origin"));
            }
        }
    }
}

// ea: 0x005D34D0
void BrocSys::MoveX(unsigned int entityHandleVal, float fMove,
                    float fTotalTime, float fAccelTime, float fDecelTime)
{
    BrocSys::MoveAxis(entityHandleVal, 0, fMove, fTotalTime, fAccelTime,
                      fDecelTime);
}

// ea: 0x005D3500
void BrocSys::MoveY(unsigned int entityHandleVal, float fMove,
                    float fTotalTime, float fAccelTime, float fDecelTime)
{
    BrocSys::MoveAxis(entityHandleVal, 1, fMove, fTotalTime, fAccelTime,
                      fDecelTime);
}

// ea: 0x005D3530
void BrocSys::MoveZ(unsigned int entityHandleVal, float fMove,
                    float fTotalTime, float fAccelTime, float fDecelTime)
{
    BrocSys::MoveAxis(entityHandleVal, 2, fMove, fTotalTime, fAccelTime,
                      fDecelTime);
}

// ea: 0x005D3980
void BrocSys::RotatePitch(unsigned int entityHandleVal, float fMove,
                          float fTotalTime, float fAccelTime,
                          float fDecelTime)
{
    BrocSys::RotateAxis(entityHandleVal, 0, fMove, fTotalTime, fAccelTime,
                        fDecelTime);
}

// ea: 0x005D39B0
void BrocSys::RotateYaw(unsigned int entityHandleVal, float fMove,
                        float fTotalTime, float fAccelTime, float fDecelTime)
{
    BrocSys::RotateAxis(entityHandleVal, 1, fMove, fTotalTime, fAccelTime,
                        fDecelTime);
}

// ea: 0x005D39E0
void BrocSys::RotateRoll(unsigned int entityHandleVal, float fMove,
                         float fTotalTime, float fAccelTime, float fDecelTime)
{
    BrocSys::RotateAxis(entityHandleVal, 2, fMove, fTotalTime, fAccelTime,
                        fDecelTime);
}

// ea: 0x005BC5B0
void BrocSys::SetShowTime(float time)
{
    cgGlobal.gameTime = time;
    cgGlobal.gameTimeStartTime = (float)cgGlobal.time;
}

// ea: 0x005BC660
int BrocSys::ActiveMenu()
{
    InGameMenuSystem* v0 = g_femanager.mIGMS[currCl];
    if (v0 == nullptr || !v0->IsSystemActive())
        return -1;
    InGameMenuSystem* IGMS = g_femanager.GetIGMS(currCl);
    return IGMS->GetActiveMenu();
}

// ea: 0x005BC930
bool BrocSys::Error(const char* file, int line, const char* msg)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = file;
    AeAssert::gCurrentLine = line;
    AeAssert::gCurrentExpr = defaultFileName;
    return AeAssert::Error(msg);
}

// ea: 0x005BE7D0
void BrocSys::Scr_ReadOnlyField(Entity* /*ent*/, int /*offset*/, void* /*val*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
    AeAssert::gCurrentLine = 543;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error("This field is read-only!"))
        __debugbreak();
}

// ea: 0x005BF4E0
void BrocSys::SentientScr_ReadOnly(sentient_s* /*pSelf*/, int /*offset*/,
                                   void* /*val*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
    AeAssert::gCurrentLine = 5598;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("Sentient field is read only!"))
        __debugbreak();
}

// ea: 0x005C1120
void BrocSys::MPScript_GetWeaponName(unsigned int weaponIndex,
                                     Broc::string& weapon)
{
    weaponFileInfo_t* InfoForWeapon =
        weaponIndex != 0 ? BG_GetInfoForWeapon((int)weaponIndex) : nullptr;
    if (InfoForWeapon != nullptr)
        weapon = InfoForWeapon->szInternalName;
    else
        weapon = "none";
}

// ea: 0x005C1880
void ObjectiveUpdatedNotify()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr)
    {
        HashString v1;
        v1.mHash = AeHash("ObjectiveUpdated");
        Player->Notify(v1);
    }
}

// ============================================================================
// scr.o batch 18 - VM glue (vm.cpp) + scr_vm error helpers
// ============================================================================

// vm_s local view (vm.cpp; 0x8C bytes stride, name[128] at +0)
struct vm_s_local {
    char name[128];                                  // +0x00
    int (*systemCall)(int*);                         // +0x80
    int (__cdecl* entryPoint)(int, ...);             // +0x84
    void* dllHandle;                                 // +0x88
};
static_assert(sizeof(vm_s_local) == 0x8C, "vm_s_local size mismatch");

// ?vmTable@@3PAUvm_s@@A @ 0x1329AD0 (3 entries, 140-byte stride)
vm_s_local vmTable[3];

extern void game_dllEntry(int (*syscallptr)(int, ...));  // ?game_dllEntry@@YAXP6AHHZZ@Z
extern int cg_vmMain(int command, int arg0, void* arg1, int* arg2, int arg3,
                     int arg4, int arg5, int arg6, int arg7, int arg8,
                     int arg9, int arg10, int arg11, int arg12,
                     int arg13);  // ?cg_vmMain@@YAHHHHHHHHHHHHHH@Z
extern void cg_dllEntry(int (*syscallptr)(int, ...));  // ?cg_dllEntry@@YAXP6AHHZZ@Z
extern void Q_strncpyz(char* dest, const char* src, int destsize);  // ?Q_strncpyz@@YAXPADPBDH@Z
extern nglTexture* GetTextureData(const char* name, int image_type,
                                  const char* fromPak);  // ?GetTextureData@@YAPAUnglTexture@@PBDH0@Z (render.o)
extern const char* SV_GetConfigstringConst(int index);  // ?SV_GetConfigstringConst@@YAPBDH@Z (sv.o)
extern int G_LocalizedStringIndex(const char* string);  // ?G_LocalizedStringIndex@@YAHPBD@Z (g.o)
extern void G_FreeEntity(Entity* e, int msec);  // ?G_FreeEntity@@YAXPAVEntity@@H@Z (g.o)
extern void StopPhysics(Entity* e);            // ?StopPhysics@@YAXPAVEntity@@@Z (g.o)

// ea: 0x005C1B80
int VM_DllSyscall(int arg, ...)
{
    if (currentVM == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\vm.cpp";
        AeAssert::gCurrentLine = 89;
        AeAssert::gCurrentExpr = "currentVM";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return ((vm_s_local*)currentVM)->systemCall(&arg);
}

// ea: 0x005C1A60
void Scr_EmitAnimation(char* animName, unsigned int animType,
                       unsigned int animIndex)
{
    (void)animName; (void)animType; (void)animIndex;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_animtree.cpp";
    AeAssert::gCurrentLine = 151;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("Is this still used? (CD)"))
        __debugbreak();
}

// ============================================================================
// scr.o batch 19 - ThreadEntityNotify family (BrocSys.cpp)
// ============================================================================

extern PoolAllocator* WaitTilOutput_sAllocator;  // ?sAllocator@WaitTilOutput@@0PAVPoolAllocator@@A
extern PoolAllocator* EntityNotify_sAllocator;   // ?sAllocator@EntityNotify@@0PAVPoolAllocator@@A

// WaitTilOutput local layout (core_systems.h; 0xC bytes + virtuals)
class WaitTilOutputLocal {
public:
    virtual ~WaitTilOutputLocal() {}
    virtual int GetSize() { return 0; }
    void* dListNodeFiller1;  // +0x04
    void* dListNodeFiller2;  // +0x08
};

template <typename T>
class WaitTilOutputInst1Local : public WaitTilOutputLocal {
public:
    T data;  // +0x0C
    WaitTilOutputInst1Local(const T& d) : data(d)
    {
        dListNodeFiller1 = nullptr;
        dListNodeFiller2 = nullptr;
    }
    virtual int GetSize() { return 1; }
    virtual ~WaitTilOutputInst1Local() {}
};

template <typename T1, typename T2>
class WaitTilOutputInst2Local : public WaitTilOutputLocal {
public:
    T1 data1;  // +0x0C
    T2 data2;  // +0x10
    WaitTilOutputInst2Local(const T1& d1, const T2& d2) : data1(d1), data2(d2)
    {
        dListNodeFiller1 = nullptr;
        dListNodeFiller2 = nullptr;
    }
    virtual int GetSize() { return 2; }
    virtual ~WaitTilOutputInst2Local() {}
};

static void NotifyPendingPush(EntityNotifyLocal* v5)
{
    AeThreadManagerLayout* L =
        (AeThreadManagerLayout*)&AeThreadManager::sInst;
    v5->m_dlist_node.mNext = L->mPendingNotifys.m_end;
    v5->m_dlist_node.mPrev = L->mPendingNotifys.m_tail;
    ((AeDListNode*)L->mPendingNotifys.m_tail)->mNext =
        &v5->m_dlist_node;
    L->mPendingNotifys.m_tail = &v5->m_dlist_node;
    ++L->mPendingNotifys.m_size;
}

// ea: 0x005C9ED0 (2-arg: entity, notifyId)
void BrocSys::ThreadEntityNotify(unsigned int entityHandleVal, int notifyId)
{
    EntityNotifyLocal* v2 =
        (EntityNotifyLocal*)EntityNotify_sAllocator->Allocate(0x14, false);
    EntityNotifyLocal* v3 =
        v2 != nullptr
            ? new (v2) EntityNotifyLocal(
                  (unsigned int)notifyId,
                  DbLinkedHandle<EntityHandleDb, Entity>(
                      Handle(entityHandleVal)),
                  nullptr)
            : nullptr;
    NotifyPendingPush(v3);
    unsigned int v5 = entityHandleVal & 0xFFF;
    if (v5 < 0x540
        && entityHandleVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v5].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v5].mObject;
        if (mObject != nullptr)
            mObject->ExecScriptHandler(HashString((unsigned int)notifyId),
                                       nullptr);
    }
}

// ea: 0x005C9F90 (3-arg: entity, notify, uint out)
void BrocSys::ThreadEntityNotify(unsigned int entityHandleVal, int notifyId,
                                 unsigned int entityOut)
{
    EntityNotifyLocal* v3 =
        (EntityNotifyLocal*)EntityNotify_sAllocator->Allocate(0x14, false);
    EntityNotifyLocal* v5;
    if (v3 != nullptr)
    {
        WaitTilOutputInst1Local<unsigned int>* v4 =
            (WaitTilOutputInst1Local<unsigned int>*)WaitTilOutput_sAllocator
                ->Allocate(0x10, false);
        v5 = v4 != nullptr
                 ? new (v3) EntityNotifyLocal(
                       notifyId,
                       DbLinkedHandle<EntityHandleDb, Entity>(
                           Handle(entityHandleVal)),
                       (void*)(WaitTilOutputLocal*)new (v4)
                           WaitTilOutputInst1Local<unsigned int>(entityOut))
                 : nullptr;
    }
    else
    {
        v5 = nullptr;
    }
    NotifyPendingPush(v5);
}

// ea: 0x005CA040 (3-arg: entity, notify, Broc::string out)
void BrocSys::ThreadEntityNotify(unsigned int entityHandleVal, int notifyId,
                                 const Broc::string& strOut)
{
    EntityNotifyLocal* v3 =
        (EntityNotifyLocal*)EntityNotify_sAllocator->Allocate(0x14, false);
    EntityNotifyLocal* v6;
    if (v3 != nullptr)
    {
        WaitTilOutputInst1Local<Broc::string>* v4 =
            (WaitTilOutputInst1Local<Broc::string>*)WaitTilOutput_sAllocator
                ->Allocate(0x10, false);
        v6 = v4 != nullptr
                 ? new (v3) EntityNotifyLocal(
                       notifyId,
                       DbLinkedHandle<EntityHandleDb, Entity>(
                           Handle(entityHandleVal)),
                       (void*)(WaitTilOutputLocal*)new (v4)
                           WaitTilOutputInst1Local<Broc::string>(strOut))
                 : nullptr;
    }
    else
    {
        v6 = nullptr;
    }
    NotifyPendingPush(v6);
}

// ea: 0x005CA0F0 (3-arg: entity, notify, int out)
void BrocSys::ThreadEntityNotify(unsigned int entityHandleVal, int notifyId,
                                 int intOut)
{
    EntityNotifyLocal* v3 =
        (EntityNotifyLocal*)EntityNotify_sAllocator->Allocate(0x14, false);
    EntityNotifyLocal* v5;
    if (v3 != nullptr)
    {
        WaitTilOutputInst1Local<int>* v4 =
            (WaitTilOutputInst1Local<int>*)WaitTilOutput_sAllocator->Allocate(
                0x10, false);
        v5 = v4 != nullptr
                 ? new (v3) EntityNotifyLocal(
                       notifyId,
                       DbLinkedHandle<EntityHandleDb, Entity>(
                           Handle(entityHandleVal)),
                       (void*)(WaitTilOutputLocal*)new (v4)
                           WaitTilOutputInst1Local<int>(intOut))
                 : nullptr;
    }
    else
    {
        v5 = nullptr;
    }
    NotifyPendingPush(v5);
}

// ea: 0x005CA1A0 (4-arg: entity, notify, float out, uint outEnt)
void BrocSys::ThreadEntityNotify(unsigned int entityHandleVal, int notifyId,
                                 float floatOut, unsigned int outEnt)
{
    EntityNotifyLocal* v4 =
        (EntityNotifyLocal*)EntityNotify_sAllocator->Allocate(0x14, false);
    EntityNotifyLocal* v6;
    if (v4 != nullptr)
    {
        WaitTilOutputInst2Local<float, unsigned int>* v5 =
            (WaitTilOutputInst2Local<float, unsigned int>*)
                WaitTilOutput_sAllocator->Allocate(0x14, false);
        v6 = v5 != nullptr
                 ? new (v4) EntityNotifyLocal(
                       (unsigned int)notifyId,
                       DbLinkedHandle<EntityHandleDb, Entity>(
                           Handle(entityHandleVal)),
                       (void*)(WaitTilOutputLocal*)new (v5)
                           WaitTilOutputInst2Local<float, unsigned int>(
                               floatOut, outEnt))
                 : nullptr;
    }
    else
    {
        v6 = nullptr;
    }
    NotifyPendingPush(v6);
}

// ea: 0x005C18C0
void ObjectiveCompletedNotify()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr)
    {
        HashString v1;
        v1.mHash = AeHash("ObjectiveCompleted");
        Player->Notify(v1);
    }
}

// ea: 0x005C3370
void BrocSys::MusicIndoorPlay(const Broc::string& pszSoundName,
                              float fadeInTime)
{
    if (pszSoundName.mBlock != nullptr)
        MusicMgr::sInst->PlayIndoor((const char*)(pszSoundName.mBlock + 1),
                                    fadeInTime);
    else
        MusicMgr::sInst->PlayIndoor(defaultFileName, fadeInTime);
}

// ea: 0x005C35D0
void BrocSys::ReverbSetParams(const Broc::string& name, bool immediate)
{
    if (name.mBlock != nullptr)
        SoundDevice::sInst->SetReverb((const char*)(name.mBlock + 1),
                                      immediate);
    else
        SoundDevice::sInst->SetReverb(defaultFileName, immediate);
}

// ea: 0x005C3610
void BrocSys::LoadWbk(const Broc::string& name)
{
    const char* v1 = name.mBlock != nullptr
                         ? (const char*)(name.mBlock + 1)
                         : defaultFileName;
    tlFixedString v2(v1);
    ((AudioBankMgrLocal*)AudioBankMgr_sInst)->LoadWbk(v2, true);
}

// ea: 0x005C3650
void BrocSys::FreeWbk(const Broc::string& name)
{
    const char* v1 = name.mBlock != nullptr
                         ? (const char*)(name.mBlock + 1)
                         : defaultFileName;
    tlFixedString v2(v1);
    ((AudioBankMgrLocal*)AudioBankMgr_sInst)->FreeWbk(v2, true);
}

// ea: 0x005C4520
int BrocSys::WeaponClipSize(const Broc::string& pszWeaponName)
{
    const char* v1 = pszWeaponName.mBlock != nullptr
                         ? (const char*)(pszWeaponName.mBlock + 1)
                         : defaultFileName;
    int result = BG_GetWeaponIndexForName(v1);
    if (result != 0)
        return BG_GetAmmoClipSize(BG_ClipForWeapon(result));
    return result;
}

// ea: 0x005C4560
int BrocSys::WeaponIsSemiAuto(const Broc::string& pszWeaponName)
{
    const char* v1 = pszWeaponName.mBlock != nullptr
                         ? (const char*)(pszWeaponName.mBlock + 1)
                         : defaultFileName;
    int result = BG_GetWeaponIndexForName(v1);
    if (result != 0)
        return BG_GetInfoForWeapon(result)->bSemiAuto;
    return result;
}

// ea: 0x005C45A0
int BrocSys::WeaponIsBoltAction(const Broc::string& pszWeaponName)
{
    const char* v1 = pszWeaponName.mBlock != nullptr
                         ? (const char*)(pszWeaponName.mBlock + 1)
                         : defaultFileName;
    int result = BG_GetWeaponIndexForName(v1);
    if (result != 0)
        return BG_GetInfoForWeapon(result)->bBoltAction;
    return result;
}

// ea: 0x005C4970
int BrocSys::GetHudElemAllocIndex()
{
    unsigned int v1 = 0;
    while (g_hudelems[v1].elem.type != HE_TYPE_FREE)
    {
        ++v1;
        if (v1 >= 16)
            return -1;
    }
    BrocSys::HudSetDefaults(&g_hudelems[v1]);
    return (int)v1;
}

// ea: 0x005C5360
void BrocSys::SetClock(const Broc::hudelem& hudElem, float fTime, float fDur,
                       const Broc::string& name, int width, int height)
{
    const char* v6 = name.mBlock != nullptr
                         ? (const char*)(name.mBlock + 1)
                         : defaultFileName;
    BrocSys::HudSetClockInternal(hudElem.___u0, HE_TYPE_CLOCK_DOWN, v6,
                                 "setClock", fTime, fDur, width, height);
}

// ea: 0x005C53A0
void BrocSys::SetClockUp(const Broc::hudelem& hudElem, float fTime, float fDur,
                         const Broc::string& name, int width, int height)
{
    const char* v6 = name.mBlock != nullptr
                         ? (const char*)(name.mBlock + 1)
                         : defaultFileName;
    BrocSys::HudSetClockInternal(hudElem.___u0, HE_TYPE_CLOCK_UP, v6,
                                 "setClockUp", fTime, fDur, width, height);
}

// ea: 0x005C53E0
void BrocSys::FadeOverTime(const Broc::hudelem& hudElem, float fadeTime)
{
    unsigned int mHudIndex = hudElem.___u0;
    if ((mHudIndex & 0x80000000) != 0 || mHudIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3544;
        AeAssert::gCurrentExpr = "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", mHudIndex))
            __debugbreak();
    }
    float v3 = fadeTime;
    game_hudelem_s* v4 = &g_hudelems[mHudIndex];
    if (fadeTime > 0.0f)
    {
        if (fadeTime <= 60.0f)
            goto ok;
        Scr_ParamError(0, va("fade time %g > 60", fadeTime));
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 23;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("%s",
                                 va("fade time %g <= 0", fadeTime)))
            __debugbreak();
    }
    v3 = fadeTime;
ok:
    v4->elem.fadeStartTime = level.time;
    v4->elem.fadeTime = (int)((v3 * 1000.0f) + 0.5f);
    v4->elem.fromColor[0] = v4->elem.color[0];
    v4->elem.fromColor[1] = v4->elem.color[1];
    v4->elem.fromColor[2] = v4->elem.color[2];
    v4->elem.fromColor[3] = v4->elem.color[3];
}

// ea: 0x005C5510
void BrocSys::ScaleOverTime(const Broc::hudelem& hudElem, float scaleTime,
                            int width, int height)
{
    unsigned int mHudIndex = hudElem.___u0;
    if ((mHudIndex & 0x80000000) != 0 || mHudIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3577;
        AeAssert::gCurrentExpr = "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", mHudIndex))
            __debugbreak();
    }
    float v5 = scaleTime;
    game_hudelem_s* v6 = &g_hudelems[mHudIndex];
    if (scaleTime > 0.0f)
    {
        if (scaleTime <= 60.0f)
            goto ok;
        Scr_ParamError(0, va("scale time %g > 60", scaleTime));
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 23;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("%s",
                                 va("scale time %g <= 0", scaleTime)))
            __debugbreak();
    }
    v5 = scaleTime;
ok:
    int v8 = v6->elem.width;
    int v9 = v6->elem.height;
    v6->elem.scaleStartTime = level.time;
    v6->elem.scaleTime = (int)((v5 * 1000.0f) + 0.5f);
    v6->elem.fromWidth = v8;
    v6->elem.fromHeight = v9;
    v6->elem.width = width;
    v6->elem.height = height;
}

// ea: 0x005C5650
void BrocSys::MoveOverTime(const Broc::hudelem& hudElem, float fadeTime)
{
    unsigned int mHudIndex = hudElem.___u0;
    if ((mHudIndex & 0x80000000) != 0 || mHudIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3601;
        AeAssert::gCurrentExpr = "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", mHudIndex))
            __debugbreak();
    }
    float v3 = fadeTime;
    game_hudelem_s* v4 = &g_hudelems[mHudIndex];
    if (fadeTime > 0.0f)
    {
        if (fadeTime <= 60.0f)
            goto ok;
        Scr_ParamError(0, va("move time %g > 60", fadeTime));
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 23;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("%s",
                                 va("move time %g <= 0", fadeTime)))
            __debugbreak();
    }
    v3 = fadeTime;
ok:
    int x = v4->elem.x;
    int y = v4->elem.y;
    v4->elem.moveStartTime = level.time;
    v4->elem.moveTime = (int)((v3 * 1000.0f) + 0.5f);
    v4->elem.fromX = x;
    v4->elem.fromY = y;
}

// ea: 0x005C57E0
void BrocSys::Scr_SetOrigin(Entity* ent, int /*offset*/, Broc::vector* val)
{
    if (IS_NAN(val->x))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 555;
        AeAssert::gCurrentExpr = "!IS_NAN(val->x)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if (IS_NAN(val->y))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 556;
        AeAssert::gCurrentExpr = "!IS_NAN(val->y)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if (IS_NAN(val->z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 557;
        AeAssert::gCurrentExpr = "!IS_NAN(val->z)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    float org[3] = {val->x, val->y, val->z};
    if (ent->actor != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                "%s",
                "cannot directly set the origin on AI.  Use the teleport command instead.\n"))
            __debugbreak();
    }
    G_SetOrigin(ent, org);
    if (ent->sentient != nullptr)
        Sentient_InvalidateNearestNode(ent->sentient);
    if (ent->r.linked != 0)
        g_LinkEntity(ent);
}

// ea: 0x005C5AE0 (mOwner at PathNode+4, pEnt at sentient+0x234 per disasm)
int BrocSys::GetNodeClaimer(const Broc::pathnode& nodeIn)
{
    PathNodes::NodeHandle handle;
    handle.mValue = (uint16_t)nodeIn.___u0;
    PathNodes::PathNode* Node = PathNodeMgr::sInst->GetNode(handle);
    if (Node == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("%s", "Node does not exist.\n"))
            __debugbreak();
        return 0;
    }
    sentient_s* mOwner = *(sentient_s**)((char*)Node + 4);
    if (mOwner == nullptr)
        return 0;
    Entity* pEnt = *(Entity**)((char*)mOwner + 0x234);
    return (int)pEnt->mHandle.mHandle.mVal;
}

// ea: 0x005C3C70
void BrocSys::GetMoveDelta(Broc::vector& outVec, unsigned int anim,
                           float startTime, float endTime)
{
    float rot[2];
    if (endTime < 0.0f || endTime > 1.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 23;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("%s",
                                 "end time must be between 0 and 1"))
            __debugbreak();
    }
    if (startTime < 0.0f || startTime > 1.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 23;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("%s",
                                 "start time must be between 0 and 1"))
            __debugbreak();
    }
    XAnimGetRelDelta(nullptr, anim, rot, &outVec.x, startTime, endTime);
    if (IS_NAN(outVec.x) || IS_NAN(outVec.y) || IS_NAN(outVec.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2387;
        AeAssert::gCurrentExpr = "!IS_NAN(((*(vec3_t*)&outVec))[0]) && !IS_NAN(((*(vec3_t*)&outVec))[1]) && !IS_NAN(((*(vec3_t*)&outVec))[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
}

// ea: 0x005C3DD0
float BrocSys::GetAngleDelta(unsigned int anim, float startTime,
                             float endTime)
{
    float trans[3];
    float rot[2];
    if (endTime < 0.0f || endTime > 1.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 23;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("%s",
                                 "end time must be between 0 and 1"))
            __debugbreak();
    }
    if (startTime < 0.0f || startTime > 1.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 23;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("%s",
                                 "start time must be between 0 and 1"))
            __debugbreak();
    }
    XAnimGetRelDelta(nullptr, anim, rot, trans, startTime, endTime);
    return vectosignedyaw(rot);
}

// ea: 0x005C4C50
void BrocSys::HudSetClockInternal(unsigned int elemNum, he_type_t type,
                                  const char* texturename,
                                  const char* /*cmdName*/, const float fTime,
                                  const float fDur, int width, int height)
{
    if (elemNum >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3329;
        AeAssert::gCurrentExpr = "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", elemNum))
            __debugbreak();
    }
    if (type != HE_TYPE_CLOCK_DOWN && type != HE_TYPE_CLOCK_UP)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3330;
        AeAssert::gCurrentExpr = "type == HE_TYPE_CLOCK_DOWN || type == HE_TYPE_CLOCK_UP";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", type))
            __debugbreak();
    }
    float v8 = fTime * 1000.0f;
    game_hudelem_s* v9 = &g_hudelems[elemNum];
    int time = (int)ceilf(v8);
    if (time <= 0 && type != HE_TYPE_CLOCK_UP)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3337;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("time should be > 0"))
            __debugbreak();
    }
    float v10 = fDur * 1000.0f;
    int duration = (int)ceilf(v10);
    if (duration <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3343;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("duration should be > 0"))
            __debugbreak();
    }
    nglTexture* texture = GetTextureData(texturename, 0, "mp_frontEnd");
    if (!texture)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3349;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("texture not found"))
            __debugbreak();
    }
    if (width < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3354;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("width should be >= 0"))
            __debugbreak();
    }
    if (height < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3358;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("height should be >= 0"))
            __debugbreak();
    }
    v9->elem.type = type;
    v9->elem.time = time + level.time;
    v9->elem.fromWidth = 0;
    v9->elem.fromHeight = 0;
    v9->elem.scaleStartTime = 0;
    v9->elem.scaleTime = 0;
    v9->elem.text = 0;
    v9->elem.height = height;
    v9->elem.value = 0.0f;
    v9->elem.duration = duration;
    v9->elem.mTexture = texture;
    v9->elem.width = width;
}

// ============================================================================
// scr.o batch 24 - spawn family (SpawnTriggerMount / SpawnTurret)
// ============================================================================

extern void SP_trigger_mount_no_brush(Entity* pSelf, int crouch);  // ?SP_trigger_mount_no_brush@@YAXPAVEntity@@H@Z (g.o)
extern float sNaN;  // ?sNaN@@3MA (core.o)

static TPakId PakInfoToPakId(TPakInfo pakInfo)
{
    // binary: *(pakInfo + 180) when pakInfo != INVALID_PAK_INFO
    return pakInfo != kTPakInfoInvalid
               ? *(TPakId*)((char*)pakInfo + 0xB4)
               : PAK_ID_INVALID;
}

// ea: 0x005C2BF0
unsigned int BrocSys::SpawnTriggerMount(Broc::vector& mins,
                                        Broc::vector& maxs, bool crouch,
                                        TPakInfo pakInfo)
{
    Broc::vector* v4 = &maxs;
    Broc::vector* v5 = &mins;
    float x = mins.x;
    if (mins.x > maxs.x)
    {
        mins.x = maxs.x;
        v4->x = x;
    }
    float y = v5->y;
    if (y > v4->y)
    {
        v5->y = v4->y;
        v4->y = y;
    }
    float z = v5->z;
    if (z > v4->z)
    {
        v5->z = v4->z;
        v4->z = z;
    }
    TPakId v9 = PakInfoToPakId(pakInfo);
    Entity* v10 = G_Spawn(v9);
    v10->mClassName = str_const.trigger_mount;
    v10->mClassNameHash = HashString(v10->mClassName);
    v10->r.mins.v.m128_f32[0] = v5->x;
    v10->r.mins.v.m128_f32[1] = v5->y;
    v10->r.mins.v.m128_f32[2] = v5->z;
    v10->r.maxs.v.m128_f32[0] = v4->x;
    v10->r.maxs.v.m128_f32[1] = v4->y;
    v10->r.maxs.v.m128_f32[2] = v4->z;
    if (IS_NAN(v10->r.mins.v.m128_f32[0])
        || IS_NAN(v10->r.mins.v.m128_f32[1])
        || IS_NAN(v10->r.mins.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1735;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.mins)[0]) && !IS_NAN((ent->r.mins)[1]) && !IS_NAN((ent->r.mins)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(v10->r.maxs.v.m128_f32[0])
        || IS_NAN(v10->r.maxs.v.m128_f32[1])
        || IS_NAN(v10->r.maxs.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1736;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.maxs)[0]) && !IS_NAN((ent->r.maxs)[1]) && !IS_NAN((ent->r.maxs)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    SP_trigger_mount_no_brush(v10, crouch);
    UpdateEntityHash(v10);
    return v10->mHandle.mHandle.mVal;
}

// ea: 0x005C2E00
unsigned int BrocSys::SpawnTurret(const Broc::string& classname,
                                  const Broc::vector& origin,
                                  const Broc::string& weaponinfoname,
                                  TPakInfo pakInfo)
{
    if (origin.x == sNaN && origin.y == sNaN && origin.z == sNaN)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1791;
        AeAssert::gCurrentExpr = "origin.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SpawnTurret called with undefined input."))
            __debugbreak();
    }
    if (IS_NAN(origin.x) || IS_NAN(origin.y) || IS_NAN(origin.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1807;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("undefined origin passed to SpawnTurret"))
            __debugbreak();
        return 0;
    }
    TPakId v5 = PakInfoToPakId(pakInfo);
    Entity* v6 = G_Spawn(v5);
    v6->mClassName = classname;
    v6->mClassNameHash = HashString(v6->mClassName);
    v6->r.currentOrigin.v.m128_f32[0] = origin.x;
    v6->r.currentOrigin.v.m128_f32[1] = origin.y;
    v6->r.currentOrigin.v.m128_f32[2] = origin.z;
    if (IS_NAN(v6->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(v6->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(v6->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1799;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    const char* v7 = weaponinfoname.mBlock != nullptr
                         ? (const char*)(weaponinfoname.mBlock + 1)
                         : defaultFileName;
    G_SpawnTurret(v6, v7);
    UpdateEntityHash(v6);
    return v6->mHandle.mHandle.mVal;
}

// ea: 0x005C3030
unsigned int BrocSys::SpawnTurretWithAngles(
    const Broc::string& classname, const Broc::vector& origin,
    const Broc::string& weaponinfoname, const Broc::vector& angles,
    TPakInfo pakInfo)
{
    if ((origin.x == sNaN && origin.y == sNaN && origin.z == sNaN)
        || (angles.x == sNaN && angles.y == sNaN && angles.z == sNaN))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1816;
        AeAssert::gCurrentExpr = "origin.IsDefined() && angles.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Spawn called with undefined input."))
            __debugbreak();
    }
    if (IS_NAN(origin.x) || IS_NAN(origin.y) || IS_NAN(origin.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1834;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                "undefined origin passed to SpawnTurretWithAngles"))
            __debugbreak();
        return 0;
    }
    TPakId v6 = PakInfoToPakId(pakInfo);
    Entity* v7 = G_Spawn(v6);
    v7->mClassName = classname;
    v7->mClassNameHash = HashString(v7->mClassName);
    v7->r.currentOrigin.v.m128_f32[0] = origin.x;
    v7->r.currentOrigin.v.m128_f32[1] = origin.y;
    v7->r.currentOrigin.v.m128_f32[2] = origin.z;
    v7->r.currentAngles.v.m128_f32[0] = angles.x;
    v7->r.currentAngles.v.m128_f32[1] = angles.y;
    v7->r.currentAngles.v.m128_f32[2] = angles.z;
    if (IS_NAN(v7->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(v7->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(v7->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1825;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(v7->r.currentAngles.v.m128_f32[0])
        || IS_NAN(v7->r.currentAngles.v.m128_f32[1])
        || IS_NAN(v7->r.currentAngles.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1833;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentAngles)[0]) && !IS_NAN((ent->r.currentAngles)[1]) && !IS_NAN((ent->r.currentAngles)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    const char* v9 = weaponinfoname.mBlock != nullptr
                         ? (const char*)(weaponinfoname.mBlock + 1)
                         : defaultFileName;
    G_SpawnTurret(v7, v9);
    UpdateEntityHash(v7);
    return v7->mHandle.mHandle.mVal;
}

// ============================================================================
// scr.o batch 25 - HUD defaults / value / shader
// ============================================================================

// HUD per-client string/value blocks (scr.o data @ 0xEA55B0, stride 31 dwords)
int dword_EA55B0[31 * 4 * 16];
int dword_EA55B4[31 * 4 * 16];
int dword_EA55B8[31 * 4 * 16];
int dword_EA55BC[31 * 4 * 16];
int dword_EA55C0[31 * 4 * 16];
int dword_EA55C4[31 * 4 * 16];
int dword_EA55C8[31 * 4 * 16];
int dword_EA55DC[31 * 4 * 16];
int dword_EA55E0[31 * 4 * 16];
int dword_EA55E4[31 * 4 * 16];
int dword_EA55E8[31 * 4 * 16];
int dword_EA55FC[31 * 4 * 16];
int dword_EA5664[31 * 4 * 16];
int dword_EA5678[31 * 4 * 16];
int dword_EA56E0[31 * 4 * 16];
int dword_EA56F4[31 * 4 * 16];
int dword_EA575C[31 * 4 * 16];
int highWaterMark = 0;  // @ 0xF3B7A8

// ea: 0x005BD320
void BrocSys::HudSetDefaults(game_hudelem_s* hud)
{
    if (hud == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3213;
        AeAssert::gCurrentExpr = "hud";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((hud - g_hudelems) >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3214;
        AeAssert::gCurrentExpr = "hud - g_hudelems >= 0 && hud - g_hudelems < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    hud->elem.x = 0;
    hud->elem.y = 0;
    hud->elem.fontScale = 1.0f;
    hud->elem.font = 0;
    hud->elem.alignX = 0;
    hud->elem.alignY = 0;
    hud->elem.fromColor[0] = 0;
    hud->elem.fromColor[1] = 0;
    hud->elem.fromColor[2] = 0;
    hud->elem.fromColor[3] = 0;
    hud->elem.fadeStartTime = 0;
    hud->elem.fadeTime = 0;
    hud->elem.label = 0;
    hud->elem.width = 0;
    hud->elem.height = 0;
    hud->elem.mTexture = nullptr;
    hud->elem.fromWidth = 0;
    hud->elem.fromHeight = 0;
    hud->elem.scaleStartTime = 0;
    hud->elem.scaleTime = 0;
    hud->elem.time = 0;
    hud->elem.duration = 0;
    hud->elem.text = 0;
    hud->elem.type = HE_TYPE_TEXT;
    hud->elem.color[0] = 0xFF;
    hud->elem.color[1] = 0xFF;
    hud->elem.color[2] = 0xFF;
    hud->elem.color[3] = 0xFF;
    hud->elem.sort = 0.0f;
    hud->elem.value = 0.0f;
}

// ea: 0x005BD670
void BrocSys::SetValue(const Broc::hudelem& hudElem, float value)
{
    unsigned int mHudIndex = hudElem.___u0;
    if ((mHudIndex & 0x80000000) != 0 || mHudIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3531;
        AeAssert::gCurrentExpr = "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", mHudIndex))
            __debugbreak();
    }
    unsigned int v3 = 124 * mHudIndex;
    dword_EA55E4[v3 / 4] = 0;
    dword_EA55B0[v3 / 4] = 0;
    dword_EA55B4[v3 / 4] = 0;
    dword_EA55B8[v3 / 4] = 0;
    dword_EA55BC[v3 / 4] = 0;
    dword_EA55C0[v3 / 4] = 0;
    dword_EA55C4[v3 / 4] = 0;
    dword_EA55C8[v3 / 4] = 0;
    dword_EA55DC[v3 / 4] = 0;
    dword_EA55E0[v3 / 4] = 0;
    dword_EA55E8[v3 / 4] = 0;
    g_hudelems[mHudIndex].elem.type = HE_TYPE_VALUE;
    *(float*)&dword_EA55E4[v3 / 4] = value;
}

// ea: 0x005C5060
void BrocSys::SetShader(const Broc::hudelem& hudElem,
                        const Broc::string& string, int width, int height)
{
    unsigned int v4 = hudElem.___u0;
    if ((v4 & 0x80000000) != 0 || v4 >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3463;
        AeAssert::gCurrentExpr = "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", v4))
            __debugbreak();
    }
    if (v4 < 0x10)
    {
        game_hudelem_s* v5 = &g_hudelems[v4];
        if (v5->elem.type < HE_TYPE_COUNT
            || v5->elem.type > (HE_TYPE_COUNT | HE_TYPE_TIMER_DOWN))
        {
            const char* v6 = string.mBlock != nullptr
                                 ? (const char*)(string.mBlock + 1)
                                 : defaultFileName;
            nglTexture* texture = GetTextureData(v6, 0, "mp_frontEnd");
            if (width < 0)
                Scr_ParamError(1, va("width %i < 0", width));
            int v8 = height;
            if (height < 0)
            {
                Scr_ParamError(2, va("height %i < 0", height));
                v8 = height;
            }
            v5->elem.width = width;
            v5->elem.fromWidth = 0;
            v5->elem.fromHeight = 0;
            v5->elem.scaleStartTime = 0;
            v5->elem.scaleTime = 0;
            v5->elem.time = 0;
            v5->elem.duration = 0;
            v5->elem.value = 0.0f;
            v5->elem.text = 0;
            v5->elem.type = HE_TYPE_SHADER;
            v5->elem.mTexture = texture;
            v5->elem.height = v8;
        }
    }
}

// ea: 0x005BD460
int BrocSys::NRefsToString(int stringRef)
{
    int result = 0;
    for (unsigned int i = 0; i < 496; i += 124)
    {
        if (g_hudelems[i / 0x1F].elem.type == HE_TYPE_TEXT
            && dword_EA55E8[i] == stringRef)
            ++result;
        if (dword_EA55FC[i] == 1 && dword_EA5664[i] == stringRef)
            ++result;
        if (dword_EA5678[i] == 1 && dword_EA56E0[i] == stringRef)
            ++result;
        if (dword_EA56F4[i] == 1 && dword_EA575C[i] == stringRef)
            ++result;
    }
    return result;
}

// ea: 0x005BD620
int BrocSys::FindConfigString(const char* text)
{
    for (int v1 = 0; v1 < 64; ++v1)
    {
        const char* ConfigstringConst = SV_GetConfigstringConst(v1 + 660);
        if (*ConfigstringConst != 0 && _stricmp(ConfigstringConst, text) == 0)
            return v1;
    }
    return -1;
}

// ea: 0x005C4EE0
void BrocSys::SetText(const Broc::hudelem& hudElem,
                      const Broc::string& string)
{
    unsigned int mHudIndex = hudElem.___u0;
    if ((mHudIndex & 0x80000000) != 0 || mHudIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3405;
        AeAssert::gCurrentExpr = "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", mHudIndex))
            __debugbreak();
    }
    int v3 = 31 * (int)mHudIndex;
    if (*SV_GetConfigstringConst(dword_EA55E8[v3] + 660)
        && BrocSys::NRefsToString(dword_EA55E8[v3]) < 2)
    {
        const char* v7 = string.mBlock != nullptr
                             ? (const char*)(string.mBlock + 1)
                             : defaultFileName;
        int ConfigString = BrocSys::FindConfigString(v7);
        if (ConfigString != -1)
        {
            SV_SetConfigstring(dword_EA55E8[v3] + 660, nullptr);
            dword_EA55E8[v3] = ConfigString;
        }
        const char* v9 = string.mBlock != nullptr
                             ? (const char*)(string.mBlock + 1)
                             : defaultFileName;
        SV_SetConfigstring(dword_EA55E8[v3] + 660, v9);
    }
    else
    {
        dword_EA55B0[v3] = 0;
        dword_EA55B4[v3] = 0;
        dword_EA55B8[v3] = 0;
        dword_EA55BC[v3] = 0;
        dword_EA55C0[v3] = 0;
        dword_EA55C4[v3] = 0;
        dword_EA55C8[v3] = 0;
        dword_EA55DC[v3] = 0;
        dword_EA55E0[v3] = 0;
        dword_EA55E4[v3] = 0;
        dword_EA55E8[v3] = 0;
        const char* v4 = string.mBlock != nullptr
                             ? (const char*)(string.mBlock + 1)
                             : defaultFileName;
        int v5 = G_LocalizedStringIndex(v4);
        dword_EA55E8[v3] = v5;
        if (highWaterMark <= v5)
            highWaterMark = v5;
    }
}

// ============================================================================
// scr.o batch 27 - SetModel / SpawnVehicle
// ============================================================================

// ea: 0x005CF060
void BrocSys::SetModel(unsigned int entityHandleVal, const Broc::string& modelName,
                       TPakInfo pakInfo)
{
    unsigned int v3 = (unsigned int)entityHandleVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v3 < 0x540
        && (unsigned int)entityHandleVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v3].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v3].mObject) != nullptr)
    {
        if (modelName.mBlock != nullptr)
        {
            TPakId mPakId;
            if (pakInfo != kTPakInfoInvalid)
                mPakId = *(TPakId*)((char*)pakInfo + 0xB4);
            else
            {
                mPakId = (TPakId)mObject->mPakId;
                if (mPakId == PAK_ID_INVALID)
                    mPakId = CurPakId();
            }
            TPakId pakId = mPakId;
            bool v6 = false;
            if (mObject->client != nullptr)
            {
                pakId = CurPakId();
                v6 = (mObject->flags & 0x400000) != 0;
                StopPhysics(mObject);
                mObject->flags &= ~0x400000u;
            }
            const char* v7 = modelName.mBlock != nullptr
                                 ? (const char*)(modelName.mBlock + 1)
                                 : defaultFileName;
            TPakId v8 = (TPakId)mObject->mPakId;
            if (v8 == PAK_ID_INVALID)
                v8 = CurPakId();
            IVPointer<AIType> aiType =
                AITypeManager::sInst->GetAIType(v8, v7, 6);
            ValidatePakId((TPakId)aiType.mPakId);
            if (aiType.mValue != nullptr)
            {
                ValidatePakId((TPakId)aiType.mPakId);
                aiType.mValue->InitEnt(mObject);
            }
            else
            {
                const char* v9 = modelName.mBlock != nullptr
                                     ? (const char*)(modelName.mBlock + 1)
                                     : defaultFileName;
                G_SetModel(mObject, v9, pakId, 0);
            }
            mObject->s.index = 0;
            SV_SetBrushModel(mObject);
            G_DObjUpdate(mObject, false);
            if (v6)
                g_UnlinkEntity(mObject);
            else
                g_LinkEntity(mObject);
        }
        else
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 2967;
            AeAssert::gCurrentExpr = "modelName.IsDefined()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("SetModel: model name undefined."))
                __debugbreak();
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 2963;
        AeAssert::gCurrentExpr = "pEnt";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Failed to set model on NULL entitiy."))
            __debugbreak();
    }
}

// ea: 0x005CA340
unsigned int BrocSys::SpawnVehicle(const Broc::string& modelname,
                                   const Broc::string& targetName,
                                   const Broc::string& vehicleType,
                                   const Broc::vector& origin,
                                   const Broc::vector& angles,
                                   TPakInfo pakInfo)
{
    if ((origin.x == sNaN && origin.y == sNaN && origin.z == sNaN)
        || (angles.x == sNaN && angles.y == sNaN && angles.z == sNaN))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1753;
        AeAssert::gCurrentExpr = "origin.IsDefined() && angles.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SpawnVehicle called with undefined input."))
            __debugbreak();
    }
    TPakId v7 = PakInfoToPakId(pakInfo);
    if (IS_NAN(origin.x) || IS_NAN(origin.y) || IS_NAN(origin.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1781;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("undefined origin passed to SpawnVehicle"))
            __debugbreak();
        return 0;
    }
    Entity* v8 = G_Spawn(v7);
    v8->mClassName = str_const.script_vehicle;
    v8->mClassNameHash = HashString(v8->mClassName);
    const char* v9 = modelname.mBlock != nullptr
                         ? (const char*)(modelname.mBlock + 1)
                         : defaultFileName;
    G_SetModel(v8, v9, PAK_ID_INVALID, 0);
    ValidatePakId((TPakId)v8->mModel.mPakId);
    if (v8->mModel.mValue != nullptr)
    {
        v8->targetname = targetName;
        v8->r.currentOrigin.v.m128_f32[0] = origin.x;
        v8->r.currentOrigin.v.m128_f32[1] = origin.y;
        v8->r.currentOrigin.v.m128_f32[2] = origin.z;
        v8->r.currentAngles.v.m128_f32[0] = angles.x;
        v8->r.currentAngles.v.m128_f32[1] = angles.y;
        v8->r.currentAngles.v.m128_f32[2] = angles.z;
        if (IS_NAN(v8->r.currentOrigin.v.m128_f32[0])
            || IS_NAN(v8->r.currentOrigin.v.m128_f32[1])
            || IS_NAN(v8->r.currentOrigin.v.m128_f32[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
            AeAssert::gCurrentLine = 1773;
            AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        if (IS_NAN(v8->r.currentAngles.v.m128_f32[0])
            || IS_NAN(v8->r.currentAngles.v.m128_f32[1])
            || IS_NAN(v8->r.currentAngles.v.m128_f32[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
            AeAssert::gCurrentLine = 1774;
            AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentAngles)[0]) && !IS_NAN((ent->r.currentAngles)[1]) && !IS_NAN((ent->r.currentAngles)[2])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        const char* v13 = vehicleType.mBlock != nullptr
                              ? (const char*)(vehicleType.mBlock + 1)
                              : defaultFileName;
        G_SpawnVehicle(v8, v13, 0);
        UpdateEntityHash(v8);
        return v8->mHandle.mHandle.mVal;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1766;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            const char* v10 = modelname.mBlock != nullptr
                                  ? (const char*)(modelname.mBlock + 1)
                                  : defaultFileName;
            const char* v11 = targetName.mBlock != nullptr
                                  ? (const char*)(targetName.mBlock + 1)
                                  : defaultFileName;
            if (AeAssert::Warning(
                    "Unable to spawn vehicle '%s'- the model '%s' was unavailable",
                    v11, v10))
                __debugbreak();
        }
        G_FreeEntity(v8, 0);
        return 0;
    }
}

// ============================================================================
// scr.o batch 28 - small BrocSys wrappers (Assert..SyncLoadPak)
// ============================================================================

namespace BrocSys {
const char* difficultyStrings[4] = {"easy", "medium", "hard", "fu"};
}

// ea: 0x005BC890
bool BrocSys::Assert(const char* file, int line, const char* msg)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = file;
    AeAssert::gCurrentLine = line;
    AeAssert::gCurrentExpr = defaultFileName;
    return !AeAssert::IsIgnored() && AeAssert::Assert(msg);
}

// ea: 0x005BC8E0
bool BrocSys::Warning(const char* file, int line, const char* msg)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = file;
    AeAssert::gCurrentLine = line;
    AeAssert::gCurrentExpr = defaultFileName;
    return !AeAssert::IsIgnored() && AeAssert::Warning(msg);
}

// ea: 0x005BC970
void BrocSys::CVarGetString(Broc::string& valueOut, const char* cvarName)
{
    char szVar[128];
    Cvar_VariableStringBuffer(cvarName, szVar, 128);
    valueOut = szVar;
}

// ea: 0x005BC9D0
void BrocSys::CVarSetString(const char* cvarName, const char* value)
{
    char szOutString[1024];
    int v2 = 0;
    while (v2 < 1024)
    {
        if (value[v2] == 0)
            break;
        char v5 = Q_CleanCharacter(value[v2]);
        szOutString[v2] = v5;
        if (v5 == 34)
            szOutString[v2] = 39;
        ++v2;
    }
    Cvar_Register(nullptr, cvarName, value, 4096);
    Cvar_Set(cvarName, value);
}

// ea: 0x005BCA50
void BrocSys::CVarSetInt(const char* cvarName, int iString)
{
    char szString[1024];
    char szOutString[1024];
    sprintf(szString, "%d", iString);
    for (int i = 0; i < 1024; ++i)
    {
        if (szString[i] == 0)
            break;
        char v3 = Q_CleanCharacter(szString[i]);
        szOutString[i] = v3;
        if (v3 == 34)
            szOutString[i] = 39;
    }
    Cvar_Register(nullptr, cvarName, szString, 4096);
    Cvar_Set(cvarName, szString);
}

// ea: 0x005BCAE0
void BrocSys::CVarSetFloat(const char* cvarName, float fString)
{
    char szString[1024];
    char szOutString[1024];
    sprintf(szString, "%f", fString);
    for (int i = 0; i < 1024; ++i)
    {
        if (szString[i] == 0)
            break;
        char v3 = Q_CleanCharacter(szString[i]);
        szOutString[i] = v3;
        if (v3 == 34)
            szOutString[i] = 39;
    }
    Cvar_Register(nullptr, cvarName, szString, 4096);
    Cvar_Set(cvarName, szString);
}

// ea: 0x005BCCA0
void BrocSys::GetDifficulty(Broc::string& outStr)
{
    if ((unsigned int)g_gameskill->integer >= 4u)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2055;
        AeAssert::gCurrentExpr =
            "g_gameskill . integer >= DIFFICULTY_EASY && g_gameskill . integer <= DIFFICULTY_FU";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    outStr = BrocSys::difficultyStrings[g_gameskill->integer];
}

// ea: 0x005BCDE0
void BrocSys::Lightning(const Broc::vector& source)
{
    if (source.x == sNaN && source.y == sNaN && source.z == sNaN)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2680;
        AeAssert::gCurrentExpr = "source.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Lightning called with undefined vector."))
            __debugbreak();
    }
}

// ea: 0x005BCEA0
void BrocSys::SetMissileActiveTime(float timeSec)
{
    level.MissleOnlyActiveForTime = timeSec * 1000.0;
}

// ea: 0x005BCEF0
int BrocSys::ProfTick()
{
    return (int)__rdtsc();
}

// ea: 0x005BD070
bool BrocSys::IsPakLoaded(TPakInfo handle)
{
    if (handle != kTPakInfoInvalid)
    {
        return PakManager::sInst->IsLoaded(
            *(TPakId*)((char*)handle + 0xB4));
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
    AeAssert::gCurrentLine = 3089;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("IsPakLoaded: invalid pak file!"))
        __debugbreak();
    return false;
}

// ea: 0x005BD0D0
void BrocSys::SetPakDistance(TPakInfo handle, float dist)
{
    if (handle != kTPakInfoInvalid)
    {
        PakManager::sInst->SetUserDistance((const PakInfoNode*)handle, dist);
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3104;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("SetPakDistance: invalid pak file!"))
            __debugbreak();
    }
}

// ea: 0x005BD130
void BrocSys::ClearPakDistance(TPakInfo handle)
{
    if (handle != kTPakInfoInvalid)
    {
        PakManager::sInst->ClearUserDistance((const PakInfoNode*)handle);
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3117;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("ClearPakDistance: invalid pak file!"))
            __debugbreak();
    }
}

// ea: 0x005BD190
void BrocSys::SyncLoadPak(TPakInfo handle)
{
    if (handle != kTPakInfoInvalid)
    {
        const PakInfoNode* pak = (const PakInfoNode*)handle;
        PakManager::sInst->SetUserDistance(pak, 0.0);
        int v1 = 0;
        if (!PakManager::sInst->IsLoaded(*(TPakId*)((char*)pak + 0xB4)))
        {
            for (;;)
            {
                PakManager::sInst->Update(false);
                if (!PakManager::sInst->IsLoading(
                        *(TPakId*)((char*)pak + 0xB4)))
                {
                    int v2 = v1++;
                    if (v2 > 10000)
                        break;
                }
                if (PakManager::sInst->IsLoaded(
                        *(TPakId*)((char*)pak + 0xB4)))
                    return;
            }
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
            AeAssert::gCurrentLine = 3141;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("Not possible to syncload '%s'!",
                                     pak->longName.c_str()))
                __debugbreak();
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3130;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("SyncLoadPak: invalid pak file!"))
            __debugbreak();
    }
}

// ============================================================================
// scr.o batch 29 - BrocSys small wrappers (GetAnimName..GetLocalizedString)
// ============================================================================

// STBManager view (core.o; full class in core/core_systems.h, which cannot
// be included alongside g_local.h)
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A (XboxLiveMenus.cpp)
    const char* GetSTBString(const char* pszReference);  // ?GetSTBString@STBManager@@QAEPBDPBD@Z (stb.cpp)
    const char* GetSTBString(unsigned int hash);         // ?GetSTBString@STBManager@@QAEPBDI@Z (stb.cpp)
};

// AbstractEffect view (core.o; full class in core/core_systems.h)
class AbstractEffect {
public:
    math::Position3 GetPosition() const;  // ?GetPosition@AbstractEffect@@QBE?AVPosition3@math@@XZ (core.o 0x4CC230)
    unsigned char _pad[0x08];
    unsigned int  mEffectNameHashStr;     // +0x08
};
extern AbstractEffect* gLastAbstractEffectParticle;  // aeps.o data

// LightEffect view (core.o; mInnerRadius +0x54, mOuterRadius +0x58,
// mFlicker +0x3C, mFade +0x60 verified vs IDA)
struct LightEffectLocal {
    unsigned char _pad0[0x3C];
    bool mFlicker;                  // +0x3C
    unsigned char _pad1[0x54 - 0x3D];
    float mInnerRadius;             // +0x54
    float mOuterRadius;             // +0x58
    unsigned char _pad2[0x60 - 0x5C];
    bool mFade;                     // +0x60
};

extern void g_AddDebugString(const float* xyz, const float* color,
                             float scale, const char* pszText);
    // ?g_AddDebugString (g_main)
extern int R_CellForPoint(const float* pos);  // render.o
extern void* AddLight(TPakId pakId, int type, math::Position3* pos,
                      int time);  // stub in g_entity_misc.cpp
extern void LightEffect_SetColor(void* light, float r, float g, float b,
                                 float a);  // stub in effect_events.cpp

// ea: 0x005BC730
bool BrocSys::GetAnimName(unsigned int anim, char* buff, int buffsize)
{
    const char* v3 = gpBrocAPI->mBrocExports.mAnimNameResolver(anim);
    if (v3 == nullptr)
        return false;
    size_t v4 = buffsize - 1;
    if (strlen(v3) <= (size_t)(buffsize - 1))
        v4 = strlen(v3);
    strncpy(buff, v3, v4);
    buff[v4] = 0;
    return true;
}

// ea: 0x005BC790
void BrocSys::PrintFloat3D(const Broc::vector& pos, float number,
                           const Broc::vector& col, float alpha, float scale)
{
    float color[4];
    color[0] = col.x;
    color[1] = col.y;
    color[2] = col.z;
    color[3] = alpha;
    char text[12];
    sprintf(text, "%f", number);
    g_AddDebugString(&pos.x, color, scale, text);
}

// ea: 0x005BC800 (release stub: single retn)
void BrocSys::Line(const Broc::vector& start, const Broc::vector& end,
                   const Broc::vector& col, float alpha, int depthtest)
{
    (void)start;
    (void)end;
    (void)col;
    (void)alpha;
    (void)depthtest;
}

// ea: 0x005BC5F0
void BrocSys::ScreenFadeToBlack(unsigned int duration, int viewport)
{
    g_femanager.IGO->SetTutorialText(-1, viewport);
    CG_Fade(0, 0, 0, 255, cgGlobal.time, duration, viewport);
}

// ea: 0x005BC6E0
const char* BrocSys::Localize(const char* txt)
{
    const char* result = STBManager::sInst->GetSTBString(txt);
    if (result == nullptr)
        return txt;
    return result;
}

// ea: 0x005BCC20
void BrocSys::GetLastParticleInfo(Broc::vector& outVec,
                                  unsigned int& outHash)
{
    if (gLastAbstractEffectParticle != nullptr)
    {
        outHash = gLastAbstractEffectParticle->mEffectNameHashStr;
        math::Position3 pos = gLastAbstractEffectParticle->GetPosition();
        Broc::vector v3;
        v3.x = pos.v.m128_f32[0];
        v3.y = pos.v.m128_f32[1];
        v3.z = pos.v.m128_f32[2];
        outVec = v3;
    }
}

// ea: 0x005BCD70
void BrocSys::RadiusDamage(const Broc::vector& origin, float range,
                           float max_damage, float min_damage,
                           int damageType)
{
    level.bPlayerIgnoreRadiusDamage = level.bPlayerIgnoreRadiusDamageLatched;
    G_RadiusDamage(&origin.x, EntityManager::sInst->mWorld,
                   EntityManager::sInst->mWorld, max_damage, min_damage,
                   range, nullptr, damageType);
    level.bPlayerIgnoreRadiusDamage = 0;
}

// ea: 0x005BCF40
TPakInfo BrocSys::GetPakVector(float x, float y, float z)
{
    math::Position3 v10;
    v10.v.m128_f32[0] = x;
    v10.v.m128_f32[1] = y;
    v10.v.m128_f32[2] = z;
    v10.v.m128_f32[3] = 0.0f;
    int v4 = R_CellForPoint(&v10.v.m128_f32[0]);
    const PakInfoNode* PakInfo;
    if (v4 == -1)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3055;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "GetPak(%.2f, %.2f, %.2f)- point is not in world", x, y, z))
            __debugbreak();
        PakInfo = PakManager::sInst->GetPakInfo(CurPakId());
    }
    else
    {
        PakInfo = StreamZoneManager::sInst->GetCellPakInfo(v4);
    }
    const PakInfoNode* v8 = PakInfo;
    if (PakInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3062;
        AeAssert::gCurrentExpr = "result";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Unknown pak file"))
            __debugbreak();
    }
    return (TPakInfo)(uintptr_t)v8;
}

// ea: 0x005BD4D0
void BrocSys::HudFree(game_hudelem_s* hud)
{
    game_hudelem_s* v1 = hud;
    if (hud == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3270;
        AeAssert::gCurrentExpr = "hud";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((v1 - g_hudelems) >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3271;
        AeAssert::gCurrentExpr =
            "hud - g_hudelems >= 0 && hud - g_hudelems < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (v1->elem.type <= HE_TYPE_FREE
        || v1->elem.type >= (HE_TYPE_COUNT | HE_TYPE_TIMER_UP))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3272;
        AeAssert::gCurrentExpr =
            "hud->elem.type > HE_TYPE_FREE && hud->elem.type < HE_TYPE_COUNT";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", v1->elem.type))
            __debugbreak();
    }
    if (v1->elem.type == HE_TYPE_TEXT)
    {
        int text = v1->elem.text;
        if (NRefsToString(text) < 2)
        {
            char val = 0;
            SV_SetConfigstring(text + 660, &val);
        }
    }
    v1->elem.type = HE_TYPE_FREE;
}

// ea: 0x005BD810
void BrocSys::ValidateApiSize(int sizeofBrocAPI, int sizeofBrocExports)
{
    if (sizeofBrocExports != 456)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3637;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error(
                "Discrepancy between dll's sizeof(BrocExports) (%d) and game's (%d) (script probably needs to be recompiled)\n"
                "HINT: sync code\\script\\include back to your label, or sync to the latest label",
                sizeofBrocExports, 456))
            __debugbreak();
    }
    if (sizeofBrocAPI != 4924)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3639;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error(
                "Discrepancy between dll's sizeof(BrocAPI) (%d) and game's (%d) (script probably needs to be recompiled)\n"
                "HINT: sync code\\script\\include back to your label, or sync to the latest label",
                sizeofBrocAPI, 4924))
            __debugbreak();
    }
}

// ea: 0x005BDB40
void BrocSys::GlowSetBrightnessAux(float val)
{
    ShaderCommon::gGlowBrighten = val;
}

// ea: 0x005BDA40 (binary symbol returns unsigned int; body returns the
// LightEffect* handle, which the script API treats as a uint)
unsigned int BrocSys::CreateDynamicLight(const Broc::vector& lightpos,
                                         const Broc::vector& color,
                                         float timeInSeconds,
                                         float innerRadius,
                                         float outerRadius, bool flicker)
{
    if (timeInSeconds <= 0.0f)
        return 0;
    TPakId v7 = CurPakId();
    LightEffectLocal* v8 = (LightEffectLocal*)AddLight(
        v7, 1 /*VERTEX_LIGHT*/, (math::Position3*)&lightpos,
        (int)(timeInSeconds * 1000.0f));
    if (v8 != nullptr)
    {
        LightEffect_SetColor(v8, color.x, color.y, color.z, 1.0f);
        v8->mInnerRadius = innerRadius;
        v8->mOuterRadius = outerRadius;
        v8->mFade = true;
        v8->mFlicker = flicker;
    }
    return (unsigned int)v8;
}

// ea: 0x005BD9C0
void BrocSys::SetDynamicLightPosition(unsigned int light,
                                      const Broc::vector& lightpos)
{
    if (light != 0)
    {
        float* p = (float*)((char*)light + 0x10);  // LightEffect::mLightPos
        p[0] = lightpos.x;
        p[1] = lightpos.y;
        p[2] = lightpos.z;
    }
}

// ea: 0x005BDAC0
Broc::string BrocSys::GetLocalizedString(int stringHash)
{
    const char* STBString =
        STBManager::sInst->GetSTBString((unsigned int)stringHash);
    return Broc::string(STBString);
}

// ============================================================================
// scr.o batch 17 - AeThread execution core
// ============================================================================

extern void CallFunctor(AeThreadFunctor* f);  // ?CallFunctor@@YAXPAVAeThreadFunctor@@@Z
extern bool gPumpThreads;              // ?gPumpThreads@@3_NA
extern bool gPumpThreadsForMapChange;  // ?gPumpThreadsForMapChange@@3_NA

// ea: 0x005C8EE0
void AeThread::ProcessState()
{
    AeThread* self = this;
    unsigned int mVal = self->mOwner.mHandle.mVal;
    unsigned int v3 = mVal & 0xFFF;
    if (v3 < 0x540
        && mVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v3].mKey
        && EntityHandleDb::sInst.mElements[v3].mObject != nullptr
        && (self->mFlags.mMask & 0x40) == 0)
    {
        AeDListNode* m_node = self->mStateControllers.m_head;
        AeDListNode* m_next =
            m_node != nullptr ? m_node->mNext : nullptr;
        if (m_node != self->mStateControllers.m_end && m_next != nullptr)
        {
            for (;;)
            {
                AeDListNode* v6 = m_next;
                m_node = m_next;
                m_next = m_next->mNext;
                AeThreadState* state = (AeThreadState*)m_node;
                AeThreadState::EAction action = state->NewAction(*self);
                if (action == AeThreadState::kActionWakeUp)
                    goto wake;
                if (action == AeThreadState::kActionTerminate)
                    break;
            debug:
                if (action != AeThreadState::kActionNone)
                {
                    char debugTxt[64];
                    debugTxt[0] = 0;
                    state->GetDebugTxt(*(ae_fixed_string<64, unsigned char>*)
                                           debugTxt);
                    if ((self->mFlags.mMask & 0x100) != 0)
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\AeThread.cpp";
                        AeAssert::gCurrentLine = 228;
                        AeAssert::gCurrentExpr = nullptr;
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Warning("0x%08x %s", self, debugTxt))
                            __debugbreak();
                    }
                }
                if (state->mFinished)
                {
                    // reserved_dlist<AeThreadState>::erase + delete
                    state->m_dlist_node.mPrev->mNext =
                        state->m_dlist_node.mNext;
                    state->m_dlist_node.mNext->mPrev =
                        state->m_dlist_node.mPrev;
                    --self->mStateControllers.m_size;
                    delete state;
                }
                if (m_next == nullptr)
                    return;
            }
            self->mFlags.mMask |= 8;
            self->mFlags.mMask |= 0x48;
        wake:
            self->mFlags.mMask |= 4;
            self->mFlags.mMask &= 0xFFFFFFEB;
            self->mFlags.mMask |= 4;
            goto debug;
        }
    }
    else
    {
        self->mFlags.mMask |= 0x40;
        self->mFlags.mMask |= 0x48;
        if ((self->mFlags.mMask & 1) == 0)
            self->mFlags.mMask |= 4;
        if ((self->mFlags.mMask & 0x100) != 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AeThread.cpp";
            AeAssert::gCurrentLine = 204;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning(
                    "0x%08x %s", self,
                    "entity is gone (or game is restarting), terminating"))
                __debugbreak();
        }
    }
}

// ea: 0x005C92B0
bool AeThread::HasEndCond(int notify) const
{
    const AeThread* self = this;
    AeDListNode* m_head = self->mStateControllers.m_head;
    AeDListNode* m_next = m_head != nullptr ? m_head->mNext : nullptr;
    if (m_head == self->mStateControllers.m_end || m_next == nullptr)
        return false;
    while (((AeThreadState*)m_head)->mResult != AeThreadState::kActionTerminate
           || self->mHandle.mVal != (unsigned int)notify)
    {
        m_head = m_next;
        m_next = m_next->mNext;
        if (m_next == nullptr)
            return false;
    }
    return true;
}

// ea: 0x005C90A0
void AeThread::Execute(float /*deltaT*/)
{
    AeThread* self = this;
    ThreadPrintf(1, "ThreadExec %s - %d\n", self->mFile, self->mLine);
    AeThreadManager::sInst.mThreadExecuting = self;
    AeThread_ProcessState(self);
    bool v3 = (self->mFlags.mMask & 0x40) != 0;
    gpBrocAPI->mKillThread = v3 ? (void (*)())1 : (void (*)())0;
    SetJmp(AeThread::sBackup);
    if ((self->mFlags.mMask & 1) != 0 && (self->mFlags.mMask & 0x10) == 0)
    {
        self->mFlags.mMask &= ~1u;
        self->mFlags.mMask &= 0xFFFFFFFA;
        if ((self->mFlags.mMask & 0x40) != 0)
        {
            if (!gPumpThreadsForMapChange)
                ThreadPrintf(1,
                             "Killing thread that never started!\nThread created from:\n%s(%d)\n",
                             self->mFile, self->mLine);
            self->mFlags.mMask |= 8;
            AeThreadManager::sInst.mThreadExecuting = nullptr;
            return;
        }
        self->mStackStart = (unsigned int)self->mBackupStack.mEnd;  // approx: real code switches stacks
        CallFunctor(self->mFunctor);
        self->mFlags.mMask |= 8;
        if (((self->mFlags.mMask | 8) >> 8) & 1)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AeThread.cpp";
            AeAssert::gCurrentLine = 493;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("0x%08x %s", self, "finished executing"))
                __debugbreak();
            AeThreadManager::sInst.mThreadExecuting = nullptr;
            return;
        }
    }
    else if ((self->mFlags.mMask & 2) != 0 && (self->mFlags.mMask & 4) != 0)
    {
        self->mFlags.mMask &= ~2u;
        self->mFlags.mMask &= 0xFFFFFFF9;
        self->mBackupStack.Restore(self->mBackupStack.mBegin);
    }
    AeThreadManager::sInst.mThreadExecuting = nullptr;
}

// ea: 0x005C1F40
AeThreadEntityNotifyTimeoutState::AeThreadEntityNotifyTimeoutState(
    DbLinkedHandle<EntityHandleDb, Entity> ent, unsigned int label, float t,
    AeThreadState::EAction result)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mFinished = false;
    mResult = result;
    mEnt = ent;
    mNotifyStr.mHash = label;
    mTimeRemaining = t;
}

// ea: 0x005C99A0
AeThreadState::EAction AeThreadEntityNotifyTimeoutState::NewAction(
    AeThread& t)
{
    unsigned int mVal = mEnt.mHandle.mVal;
    unsigned int v4 = mVal & 0xFFF;
    if (v4 < 0x540
        && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != nullptr)
        {
            mTimeRemaining -= ServerTime::sInst.mTickDelta;
            if (mTimeRemaining <= 0.0f
                || ((t.mFlags.mMask & 0x80) != 0
                    && mObject->mNotifySet != nullptr
                    && ((EntityNotifySetLocal*)mObject->mNotifySet)
                           ->GetNotify(mNotifyStr) != nullptr))
            {
                mFinished = true;
                return mResult;
            }
            return kActionNone;
        }
    }
    mFinished = true;
    return kActionTerminate;
}

// ea: 0x005C7C10
AeThreadEntityNotifyMatchState::AeThreadEntityNotifyMatchState(
    DbLinkedHandle<EntityHandleDb, Entity> ent, unsigned int label1,
    unsigned int label2, unsigned int label3, unsigned int label4,
    AeThreadState::EAction result, bool waitForAll)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mFinished = false;
    mResult = result;
    mEnt = ent;
    mNotifySet[0].mHash = 0;
    mNotifySet[1].mHash = 0;
    mNotifySet[2].mHash = 0;
    mNotifySet[3].mHash = 0;
    mWaitForAll = waitForAll;
    mEventMask = 0;
    mDebugNotifys[0].mHash = 0;
    mDebugNotifys[1].mHash = 0;
    mDebugNotifys[2].mHash = 0;
    mDebugNotifys[3].mHash = 0;
    mNotifySet[0].mHash = label1;
    mNotifySet[1].mHash = label2;
    mNotifySet[2].mHash = label3;
    mNotifySet[3].mHash = label4;
    mDebugNotifys[0].mHash = mNotifySet[0].mHash;
    mDebugNotifys[1].mHash = mNotifySet[1].mHash;
    mDebugNotifys[2].mHash = mNotifySet[2].mHash;
    mDebugNotifys[3].mHash = mNotifySet[3].mHash;
}

// ea: 0x005C9A40
AeThreadState::EAction AeThreadEntityNotifyMatchState::NewAction(
    AeThread& t)
{
    unsigned int mVal = mEnt.mHandle.mVal;
    unsigned int v4 = mVal & 0xFFF;
    if (v4 < 0x540
        && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != nullptr)
        {
            if ((t.mFlags.mMask & 0x80) != 0 && mObject->mNotifySet != nullptr)
            {
                EntityNotifySetLocal* notifySet =
                    (EntityNotifySetLocal*)mObject->mNotifySet;
                for (int i = 0; i < 4; ++i)
                {
                    if (mNotifySet[i].mHash != 0
                        && (mEventMask & (1 << i)) == 0
                        && notifySet->GetNotify(mNotifySet[i]) != nullptr)
                    {
                        if (!mWaitForAll)
                            goto done;
                        mEventMask |= 1 << i;
                        mNotifySet[i].mHash = 0;
                    }
                }
                if ((mEventMask & 0xF) != 0xF)
                    return kActionNone;
            done:
                mFinished = true;
                return mResult;
            }
            return kActionNone;
        }
    }
    mFinished = true;
    return kActionTerminate;
}
