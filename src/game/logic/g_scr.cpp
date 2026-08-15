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
#include "game/logic/g_camerashake.h"

extern PoolAllocator* gCommonPoolAllocator;  // ?gCommonPoolAllocator@@3PAVPoolAllocator@@A (core.o)
extern void* mem_heap_malloc(unsigned int size);  // ?mem_heap_malloc@@YAPAXI@Z
extern bool g_indoor;                              // ?g_indoor@@3_NA (core.o)
extern float CG_GetNorthDirection();               // ?CG_GetNorthDirection@@YAMXZ (cg.o)
extern void G_FlushCorpses();                      // ?G_FlushCorpses@@YAXXZ (mp_actors.o)
extern void FX_SetRainDrops(bool on);              // ?FX_SetRainDrops@@YAX_N@Z (render.o)
class AnimBroRef;  // anim.o bro_anim ref (opaque; class tag V per binary)

// mp_level entry (mp_level.xboxd; defined in broc/mp_level.cpp)
namespace mp_level {
typedef void (__cdecl* InitScriptFn)();
InitScriptFn InitScript(BrocAPI** gamesAPIptr,
                        BrocExports& exports);  // ?InitScript@mp_level@@YAP6AXXZPAPAUBrocAPI@@AAUBrocExports@@@Z
}
typedef mp_level::InitScriptFn (__cdecl* BrocEntryFn)(BrocAPI**,
                                                      BrocExports&);

// ea: 0x005BC1A0
BrocEntryFn XboxGetEntryFunction(const char* mapname)
{
    return _stricmp(mapname, "mp_level") == 0 ? mp_level::InitScript
                                              : nullptr;
}

// TPakInfo - opaque pak info enum (scr.o; W4TPakInfo mangling)
enum TPakInfo {
    kTPakInfoInvalid = 0,
};

namespace AeStringSupport {
void Concat(char* dst, int& dstLen, int dstCapacity,
            const char* src);  // ?Concat@AeStringSupport@@YAXPADAAHHPBD@Z
void CStrToAeStr(char* dst, int& dstLen, int dstCapacity,
                 const char* src);  // ?CStrToAeStr@AeStringSupport@@YAXPADAAHHPBD@Z
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
void RegisterBroFunc(char* name, unsigned int (__cdecl* func)(void*));  // ?RegisterBroFunc@BrocHelper@@YAXPADP6AIPAX@Z (0x5BE1A0)
void SetLoadedTrees(int num);  // ?SetLoadedTrees@BrocHelper@@YAXH@Z
int  GetLoadedTrees();         // ?GetLoadedTrees@BrocHelper@@YAHXZ
void Init();                   // ?Init@BrocHelper@@YAXXZ (scr.o 0x5BE180)
void AnimationToBroLookup(const tlFixedString& tree_name, int tree_index,
                          const tlFixedString& animation_name,
                          int animation_index);  // ?AnimationToBroLookup@BrocHelper@@YAXABVtlFixedString@@H0H@Z
void AnimationToBroLookup(const char* tree_name, int tree_index,
                          unsigned int animation_name_brohashed,
                          int animation_index);  // 0x5BE040
void AnimationValidator(int numTrees);  // ?AnimationValidator@BrocHelper@@YAXH@Z (0x5BE100)
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

class WaitTilOutput;  // core_systems.h; full local view below (0xC bytes + virtuals)
struct ScriptEventHandler {
    bool RemoveEvent(HashString h, HashString callback);  // ?RemoveEvent@ScriptEventHandler@@QAE_NVHashString@@0@Z (g_game2_misc.cpp 0x4F59F0)
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
    bool AssignScriptVariable(const HashString& chk,
                              WaitTilOutput* scriptVariable);  // ?AssignScriptVariable@EntityNotifySet@@QAE_NABVHashString@@PAVWaitTilOutput@@@Z (entity_notify.cpp)

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

class AnimNotifyTask : public Task {
public:
    AnimNotifyTask(DbLinkedHandle<EntityHandleDb, Entity> h,
                   unsigned int animHash, unsigned int killHash);
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
    InplaceVector<AnimTree> anims;  // +0x00
    void* mPtrFixupTable;           // +0x08
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
bool NextRoundMapChanges();  // ?NextRoundMapChanges@MPUIInterface@@SA_NXZ (mp.o)
bool IsOnlineGame();  // mp.o
bool IsLANGame();     // mp.o
bool IsLocalGame();   // mp.o
}

// ae_heap wrapper view (streamer.o 0x684DD0; definition in pakmanager.cpp)
struct mem_heap;
struct ae_heap {
    void** __vftable;                 // +0x00
    void* Malloc(unsigned int size, int alignment);  // ?Malloc@ae_heap@@QAEPAXIH@Z
};
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
void RoundOver(int condition, const Broc::string& winner);  // 0x5BC2C0
void NextRound(bool allowChange);  // 0x5BC6A0
bool NextRoundMapChanges();        // 0x5BC6C0
void Reset(const Broc::hudelem& hudElem);  // 0x5BD730
void Destroy(const Broc::hudelem& hudElem);  // 0x5BD7A0
void PlayScriptedAnim(unsigned int entHandleVal,
                      const char* eventId);  // 0x5BD8B0
int  GetPlayerArray(unsigned int* array);  // 0x5BE6C0
int  GetLocalPlayerArray(unsigned int* array);  // 0x5BE750
void Scr_SetAngles(Entity* ent, int offset, Broc::vector* val);  // 0x5BE810
void Scr_GetAngles(Entity* ent, int offset, Broc::vector* val);  // 0x5BE870
void Scr_SetGroupName(Entity* ent, int offset, Broc::string* val);  // 0x5BE8D0
void Scr_SetTargetName(Entity* ent, int offset, Broc::string* val);  // 0x5BE920
void Scr_SetTarget(Entity* ent, int offset, Broc::string* val);  // 0x5BE970
void Scr_SetNoteWorthy(Entity* ent, int offset, Broc::string* val);  // 0x5BE9C0
void Scr_SetAnimName(Entity* ent, int offset, Broc::string* val);  // 0x5BEA10
void SetTutorialText(int hash, int viewport);  // 0x5BF8B0
void SetActionHint(int hash, int viewport);    // 0x5BF940
void SetWeaponCameraShakeScale(float scale, int onlyADS);  // 0x5BF860
void NoClip(int val);  // 0x5BFB90
unsigned int GetNumVehicles();  // 0x5BEC40
int GetNode(const Broc::string& inName, const Broc::string& key,
            int* array, int capacity);  // 0x5BEAF0
int GetVehicleNode(const Broc::string& inName, const Broc::string& key,
                   int* array, int capacity);  // 0x5BEB90
bool BROC_AttachCurveEntity(unsigned int entityHandleVal,
                            Broc::string& filename);  // 0x5BED00
int CheckWave(const Broc::string& script);  // 0x5BED90
int GetAnimFromScriptCVars();  // 0x5BEEB0
bool IsModelLoaded(const Broc::string& modelName, TPakInfo pakInfo);  // 0x5BEFB0
void ValidateLightVis(int eType);  // 0x5BF0B0
void ActorScr_Clamp_0_1(actor_s* a, int offset, float* val);  // 0x5BF110
void ActorScr_ReadOnly(actor_s* a, int offset, void* val);  // 0x5BF1D0
void SetGameVectorVar(unsigned int hashVarName,
                      const Broc::vector& vect);  // 0x5BFA30
Broc::vector GetGameVectorVar(unsigned int hashVarName);  // 0x5BFAD0
bool IsValidClientType(Entity* pEnt);  // 0x5C0B00
void MPScript_RequestRespawn(unsigned int playerID);  // 0x5C0B60
void Mover_GravityMove(Entity* pEnt, const float* const vVel,
                       float fTotalTime, float gravityOverride);  // 0x5C0910
void AnimScripted1(unsigned int entityHandleVal, unsigned int notifyName,
                   const Broc::vector& origin, const Broc::vector& angles,
                   unsigned int broanim);  // 0x5BEE00
void ActorScr_SetAnimPos(actor_s* a, int offset,
                         const unsigned int* val);  // 0x5BF320
void ActorScr_GetFavoriteEnemy(actor_s* a, int offset,
                               Broc::entity* val);  // 0x5BF3B0
void PathNode_SetType(PathNodes::PathNode* pNode, int offset,
                      Broc::string* val);  // 0x5BF470
void SentientScr_ConvertNode(sentient_s* pSelf, int offset,
                             Broc::pathnode* pNode);  // 0x5BF520
void SentientScr_GetTeam(sentient_s* pSelf, int offset,
                         Broc::string* val);  // 0x5BF5B0
void SentientScr_SetGoalRadius(sentient_s* pSelf, int offset,
                               float* val);  // 0x5BF650
void GetSplineData(const char* splineName, Broc::vector*& origins,
                   unsigned int*& eventIndices,
                   unsigned int*& eventHashes);  // 0x5BF750
void ToggleClip();  // 0x5C1800
void CloseAllMenus(int viewport);  // 0x5C1730
int  FindUnusedChildObjective();  // 0x5C1980
void DeleteAllChildrenOfObjective(int iObjective);  // 0x5C19D0
void IPrintLn(const char* txt);  // 0x5C21C0
void IPrintLnBold(const char* txt);  // 0x5C2210
void RegisterAnimation(AnimBroRef* animBroRef);  // 0x5C2260
void ThreadDebugNotice(const char* msg);  // 0x5C22E0
int  GetTeamFlags(const Broc::string& teamName,
                  const Broc::string& caller);  // 0x5C3690
Broc::string GetWeaponClassName(unsigned int weaponName);  // 0x5C3750
Broc::string GetWeaponClassNameStr(unsigned int weaponName);  // 0x5C3810
void MissionFailed(const Broc::string& reason);  // 0x5C3EB0
void Cinematic1(const Broc::string& pszCinematic, float fVal);  // 0x5C3F40
void Cinematic2(const Broc::string& pszCinematic);  // 0x5C3FF0
Broc::hudelem NewHudElem(int panelType);  // 0x5C49B0
TPakInfo GetPak(const Broc::string& longname);  // 0x5C4A80
void SetTimerUp(const Broc::hudelem& hudElem, float fVal);  // 0x5C51A0
void SetTenthsTimerUp(const Broc::hudelem& hudElem, float fVal);  // 0x5C5280
void Scr_SetHealth(Entity* ent, int offset, int* val);  // 0x5C5990
void CreateAnimNotifyTask(const Broc::entity& ent, unsigned int animHash,
                          unsigned int killHash);  // 0x5C5A70
void SentientScr_SetTeam(sentient_s* pSelf, int offset,
                         Broc::string* val);  // 0x5C5B60
void SetExploderState(int exploderNumber, int state);  // 0x5C5BB0
bool IsValidMoverType(Entity* pEnt);  // 0x5C5C20
void GiveWeapon(Entity* pSelf, const char* pszWeaponName);  // 0x5C5C60
void SetReverb(unsigned int entityHandleVal, const Broc::string& pszReverb,
               float wetlevel, float fadetime);  // 0x5C5D10
void PlayLocalSound(unsigned int entityHandleVal,
                    const Broc::string& pszSoundName);  // 0x5C5D50
Broc::hudelem gHudElement = { 0xFFFFFFFFu };
    // ?gHudElement@BrocSys@@3Vhudelem@Broc@@A (scr.o data, init -1 per IDA)
void VM_Clear();  // ?VM_Clear@@YAXXZ (0x5C1DB0, global)
void SetupLevelSpecificVariables();  // 0x5C20D0
void ObjectiveAdd3(int iObjective, const Broc::string& inState,
                   const Broc::string& pszString, const char* display,
                   int iChild, int iChildOrder, int clientIndex);  // 0x5C5E10
const int FindChildObjective(int iObjective, int iChild,
                             bool bReportNotFound);  // 0x5C5F50
void ObjectiveChildDelete(int iObjective, int iChild);  // 0x5C60B0
void ObjectiveDeleteChildren(int iObjective);  // 0x5C6150
void ObjectiveDelete(int iObjective, int clientIndex);  // 0x5C61D0
void ObjectiveChildState(int iObjective, int iChild,
                         const Broc::string& inState,
                         const char* pDisplay);  // 0x5C62C0
void ObjectiveState(int iObjective, const Broc::string& inState,
                    const char* pDisplay, int clientIndex);  // 0x5C6460
void ObjectiveStringInternal(int iObjective, const Broc::string& text,
                             int number, bool bMakeUpdateMessage,
                             bool bChild, const char* pDisplay);  // 0x5C6620
void ObjectiveString(int iObjective, const Broc::string& text, int number,
                     const char* pDisplay);  // 0x5C6890
void ObjectiveString2(int iObjective, int text, int number,
                      const char* pDisplay);  // 0x5C68C0
void ObjectiveString_NoMessage(int iObjective, const Broc::string& text,
                               int number, const char* pDisplay);  // 0x5C6930
void ObjectivePosition(int iObjective, const Broc::vector& vPos,
                       int clientIndex);  // 0x5C6960
void ObjectiveWorldState(int iObjective, const Broc::string& inState,
                         int clientIndex);  // 0x5C6A80
void ObjectiveChildCurrent(int iObjective, int iChild,
                           const char* pDisplay);  // 0x5C6C90
void ObjectiveCurrent(int iObjective, const char* pDisplay);  // 0x5C6DE0
void ObjectiveRing(int iObjective, int clientIndex);  // 0x5C6F90
void ObjectiveChildString(int iObjective, int iChild,
                          const Broc::string& text, int number,
                          const char* pDisplay);  // 0x5C7060
void ObjectiveChildString2(int iObjective, int iChild, int text,
                           int number, const char* pDisplay);  // 0x5C70E0
void ObjectiveChildString_NoMessage(int iObjective, int iChild,
                                    const Broc::string& text,
                                    int number,
                                    const char* pDisplay);  // 0x5C73D0
void ObjectiveChildPosition(int iObjective, int iChild,
                            const Broc::vector& vPos);  // 0x5C7450
void ObjectiveChildRing(int iObjective, int iChild);  // 0x5C7520
void ObjectiveChildAdd1(int iObjective, int iChild,
                        const Broc::string& state);  // 0x5C8830
void ObjectiveAdd1(int iObjective, const Broc::string& state,
                   const char* display, int iChild,
                   int iChildOrder);  // 0x5C8950
void ObjectiveChildAdd2(int iObjective, int iChild,
                        const Broc::string& state, int iString,
                        const char* display);  // 0x5C89C0
void ObjectiveAdd2(int iObjective, const Broc::string& state,
                   int iString, const char* display, int iChild,
                   int iChildOrder);  // 0x5C8B70
void ObjectiveChildAdd6(int iObjective, int iChild,
                        const Broc::string& state, int pszString,
                        const char* display);  // 0x5C8CA0
void ObjectiveAdd6(int iObjective, const Broc::string& inState,
                   int pszString, const char* display, int iChild,
                   int iChildOrder);  // 0x5C8DC0
void ObjectiveChildAdd3(int iObjective, int iChild,
                        const Broc::string& state,
                        const Broc::string& pszString,
                        const char* display);  // 0x5C8E30
void BrocDebugRender();  // 0x5BDEA0
void* CreateExtendedEntity(const char** keys, int count);  // 0x5BDF00
bool RecompileScript();  // 0x5BDFB0
void GetJoyPos(int stickIndex, float& xPos, float& yPos);  // 0x5BF6A0
void MPScript_ClearPlayerStats();  // 0x5C0B80
void MPScript_IncTeamScore(const Broc::string& team, int amount);  // 0x5C0C40
int  MPScript_GetTeamScore(const Broc::string& team);  // 0x5C0CD0
void MPScript_SendGameScore(int alliesScore, int axisScore);  // 0x5C0D50
void MPScript_HostDropItem1(int itemType, int netID,
                            const Broc::vector& pos,
                            const Broc::vector& angle);  // 0x5C0D70
void MPScript_HostDropItem2(int itemType, int netID,
                            const Broc::vector& pos,
                            const Broc::vector& angle,
                            const Broc::vector& vel);  // 0x5C0E40
void MPScript_DropItem1(int itemType, int netID, const Broc::vector& pos,
                        const Broc::vector& angle);  // 0x5C0F30
void MPScript_DropItem2(int itemType, int netID, const Broc::vector& pos,
                        const Broc::vector& angle,
                        const Broc::vector& vel);  // 0x5C0FE0
void MPScript_AreaCaptured(int netID, unsigned int team,
                           unsigned int hostOnly);  // 0x5C10C0
void MPScript_EnterGame();  // 0x5C10E0
bool MPScript_PositionWouldTelefrag(const Broc::vector& position);  // 0x5C1160
int MPScript_SpawnButtonPressed(unsigned int clientIdx);  // 0x5C11D0
void MPScript_DebugRenderText(const char* text, int x, int y);  // 0x5C1220
void MPScript_DebugRenderBox(const Broc::vector& min,
                             const Broc::vector& max,
                             const Broc::vector& color,
                             float alpha);  // 0x5C1270
void MPScript_DebugRenderSphere(const Broc::vector& point, float radius,
                                const Broc::vector& color,
                                float alpha);  // 0x5C1330
int  IsMenuOpen(const Broc::string& str, int viewport);  // 0x5C1430
int  OpenMenu(const Broc::string& str, int viewport);  // 0x5C14C0
int  OpenMenuNoMouse(unsigned int i, const Broc::string& str);  // 0x5C15F0
void CloseMenu2(const Broc::string& str, int viewport);  // 0x5C1640
void CloseAllMenus(int viewport);  // 0x5C1730
void SetSpectateState(int state, int viewport);  // 0x5C1750
void SetSpectateSeconds(int seconds, int viewport);  // 0x5C1780
void SetSpectateMedic(int medic, int viewport);  // 0x5C17B0
void SettleMapVote();  // 0x5BC450
void SettleGameModeVote();  // 0x5BC470
void InitMPCallbacks();  // ?InitMPCallbacks@BrocSys@@YAXXZ (0x5BDBA0)
unsigned int Spawn(const Broc::string& classname, const Broc::vector& origin,
                   TPakInfo pakInfo);  // 0x5C2340
unsigned int SpawnWithFlag(const Broc::string& classname,
                           const Broc::vector& origin, int iSpawnFlags,
                           TPakInfo pakInfo);  // 0x5C2590
unsigned int SpawnWithFlagAndSize(const Broc::string& classname,
                                  const Broc::vector& origin,
                                  Broc::vector& mins, Broc::vector& maxs,
                                  int iSpawnFlags,
                                  TPakInfo pakInfo);  // 0x5C27F0
void GetStartOrigin(Broc::vector& outVec, const Broc::vector& origin,
                    const Broc::vector& angles,
                    unsigned int anim);  // 0x5C38D0
void GetStartAngles(Broc::vector& outVec, const Broc::vector& origin,
                    const Broc::vector& angles,
                    unsigned int anim);  // 0x5C3A40
void GetCycleOriginOffset(Broc::vector& outVec, const Broc::vector& angles,
                          unsigned int anim);  // 0x5C3B60
void Print3D(const Broc::vector& pos, const Broc::string& text,
             const Broc::vector& col, float alpha, float scale);  // 0x5C2280
void Earthquake(float scale, float fVal, const Broc::vector& source,
                float radius, int player_index);  // 0x5C4040
void BulletTracer(const Broc::vector& vStart,
                  const Broc::vector& vEnd);  // 0x5C40F0
void MagicBullet(const Broc::string& weapName, const Broc::vector& source,
                 const Broc::vector& dest);  // 0x5C4230
float WeaponFireTime(const Broc::string& pszWeaponName);  // 0x5C44D0
void WeaponType(Broc::string& outStr,
                const Broc::string& pszWeaponName);  // 0x5C45E0
void BadPlaceDelete(const Broc::string& placeName);  // 0x5C4640
void BadPlaceCylinder(const Broc::string& placeName, float dur,
                      const Broc::vector& ori, float rad,
                      float height,
                      const Broc::string& pszTeamName);  // 0x5C46B0 (unused trailing param per PDB mangle)
void BadPlaceArcs(const Broc::string& placeName, float dur,
                  const Broc::vector& ori, float rad, float height,
                  const Broc::vector& ang, float fVal1, float fVal2,
                  const Broc::string& pszTeamName);  // 0x5C47A0
void GetNumParts(const Broc::string& modelName);  // 0x5C8040
void NotifyPakLoadOperation(const char* operation,
                            const char* longName);  // 0x5C8100
float GetAnimLength(unsigned int entityHandleVal,
                    unsigned int broanim);  // 0x5C81D0
int GetAnimFrameCount(unsigned int entityHandleVal,
                      unsigned int broanim);  // 0x5C8230
void SetProjectileSpeed(unsigned int entityHandleVal, int speed);  // 0x5C9B20
void SetWeaponPlayerUpOffset(unsigned int entityHandleVal,
                             float offset);  // 0x5C9BC0
int  FollowCycle(unsigned int entityHandleVal, int dir);  // 0x5C9C60
void EnableWeapon(unsigned int entityHandleVar);  // 0x5C9CA0
void DisableWeapon(unsigned int entityHandleVar);  // 0x5C9D30
bool ThreadIsThreadAlive(unsigned int handle);  // 0x5C9E40
void ThreadKill(unsigned int threadId);  // 0x5C9E80
void ThreadSleepFrames(int numFrames);  // 0x5C7EC0
void ThreadSleepInternal(float sleepTime);  // 0x5C7F50
void* MemAlloc(unsigned int size, unsigned int align);  // 0x5C7D30
void ThreadGetDebugInfo(Broc::string& fileline, Broc::string& func,
                        Broc::string& threadId);  // 0x5C7D90
unsigned int SoundPlay(const Broc::string& name, float volume);  // 0x5C33B0
bool AssignParameterForNotify(unsigned int entHandle, unsigned int notify,
                              WaitTilOutput* scriptVar);  // 0x5C9DC0
bool AddEventHandler(unsigned int entityHandleVal, unsigned int notifyId,
                     unsigned int callback);  // 0x5CA260
bool RemoveEventHandler(unsigned int entityHandleVal, unsigned int notifyId,
                        unsigned int callback);  // 0x5CA2B0
void RadiusDamageFromEnt(unsigned int ehandle, const Broc::vector& origin,
                         float range, float max_damage, float min_damage,
                         int damageType);  // 0x5CA710
TPakInfo GetPakEntity(unsigned int ehandle);  // 0x5CA7C0
int GetPlayerIndex(unsigned int player);  // 0x5CAA50
void* GetExtendedEntity(unsigned int handle);  // 0x5CAA90
void Scr_SetModel(Entity* ent, int offset, Broc::string* val);  // 0x5CAB00
void Mover_SetupMoveSpeed(trajectory_t* pTr, const math::Position3& vSpeed,
                          float fTotalTime, float fAccelTime,
                          float fDecelTime, math::Position3& vCurrPos,
                          float* pfSpeed, float* pfMidTime,
                          float* pfDecelTime, math::Position3& vPos1,
                          math::Position3& vPos2,
                          math::Position3& vPos3);  // 0x5C02C0
unsigned int GetEntByNum(int entnum);  // 0x5CAB50
void DrawTracer(unsigned int entityHandleVal);  // 0x5CABA0
int EffectEventWeaponPlay(unsigned int entityHandleVal,
                          unsigned int action);  // 0x5CAC20
int EffectEventPlay(unsigned int entityHandleVal, const Broc::string& script,
                    int notifyHash, bool stoppable,
                    bool important);  // 0x5CAD10
int EffectEventPlayNonEnt(const Broc::string& script,
                          const Broc::vector& pos,
                          const Broc::vector& facing, bool bImportant,
                          unsigned int entityHandle,
                          int notifyHash);  // 0x5CAE70
int EffectEventPlayDir(unsigned int entityHandleVal,
                       const Broc::string& script, const Broc::vector& dir,
                       int notifyHash, bool bUnused);  // 0x5CB000
int EffectEventQueue(unsigned int entityHandleVal,
                     const Broc::string& script, int notifyHash,
                     bool stoppable, bool important);  // 0x5CB140
int EffectEventQueueDialog(unsigned int entityHandleVal,
                           const Broc::string& script, int notifyHash,
                           bool bUnused);  // 0x5CB280
int DialogPlay(unsigned int entityHandleVal, const Broc::string& script,
               int notifyHash, bool stopPrevIfPlaying);  // 0x5CB390
int EntityExists(unsigned int entityHandleVal);  // 0x5CB5C0
int EntityIsAlive(unsigned int entityHandleVal);  // 0x5CB600
int EntityIsWounded(unsigned int entityHandleVal);  // 0x5CB660
int EntityIsPlayer(unsigned int entityHandleVal);  // 0x5CB6B0
int EntityIsAI(unsigned int entityHandleVal);  // 0x5CB700
int EntityIsSentient(unsigned int entityHandleVal);  // 0x5CB750
int EntityIsVehicle(unsigned int entityHandleVal);  // 0x5CB7A0
int EntityIsVehicleTank(unsigned int entityHandleVal);  // 0x5CB7F0
void StopAnimScripted(unsigned int entityHandleVal);  // 0x5CC050
void StartBlankState(unsigned int entityHandleVal);  // 0x5CC1A0
void StopBlankState(unsigned int entityHandleVal);  // 0x5CC1F0
void ResetAnimVariationChunkState(unsigned int entityHandleVal,
                                  unsigned int broanim);  // 0x5CC290
void GetInVehicle1(unsigned int entityHandleVal,
                   unsigned int targetEntityHandleVal, bool canDrive,
                   bool canGunner, bool lock);  // 0x5CC3B0
void GetInVehicle2(unsigned int entityHandleVal,
                   unsigned int targetEntityHandleVal, unsigned int desirePos,
                   bool lock);  // 0x5CC4C0
void GetOutVehicle(unsigned int entityHandleVal);  // 0x5CC5E0
void SceneGetOutVehicle(unsigned int entityHandleVal);  // 0x5CC680
void StartInVehicle1(unsigned int entityHandleVal,
                     unsigned int targetEntityHandleVal, bool canDrive,
                     bool canGunner, bool lock);  // 0x5CC730
void StartInVehicle2(unsigned int entityHandleVal,
                     unsigned int targetEntityHandleVal,
                     unsigned int desirePos, bool lock);  // 0x5CC7E0
void StartFollowBehavior(unsigned int entityHandleVal,
                         unsigned int targetEntityHandleVal);  // 0x5CCC50
void StopFollowBehavior(unsigned int entityHandleVal);  // 0x5CCDB0
void SetFollowFormationData(unsigned int targetEntityHandleVal,
                            unsigned int numColumns, float columnSpacing,
                            float rowSpacing,
                            float minFollowDistance);  // 0x5CCE50
void ResetVehicleFollowPositionHistoryData(
    unsigned int targetEntityHandleVal);  // 0x5CCFB0
void Attach1(unsigned int entityHandleVal, const Broc::string& modelName,
             const Broc::string& tagName, bool ignoreCollision,
             TPakInfo modelpak);  // 0x5CD050
void Attach2(unsigned int entityHandleVal, const Broc::string& modelName,
             const Broc::string& tagName, TPakInfo modelpak);  // 0x5CD1D0
void Attach3(unsigned int entityHandleVal,
             const Broc::string& modelName);  // 0x5CD1F0
void Detach1(unsigned int entityHandleVal, const Broc::string& modelName,
             const Broc::string& tagName);  // 0x5CD260
void Detach2(unsigned int entityHandleVal,
             const Broc::string& modelName);  // 0x5CD3D0
void DetachAll(unsigned int entityHandleVal);  // 0x5CD430
int  GetAttachSize(unsigned int entityHandleVal);  // 0x5CD470
void GetAttachModelName(unsigned int entityHandleVal, Broc::string& outStr,
                        int i);  // 0x5CD4E0
void GetAttachTagName(unsigned int entityHandleVal, Broc::string& outStr,
                      int i);  // 0x5CD5C0
bool GetAttachIgnoreCollision(unsigned int entityHandleVal,
                              int i);  // 0x5CD6E0
void LinkTo1(unsigned int entityHandleVal, unsigned int parentEntityHandleVal,
             const Broc::string& tagName, const Broc::vector& originOffset,
             const Broc::vector& anglesOffset,
             bool useAngles);  // 0x5CD7A0
void LinkTo2(unsigned int entityHandleVal, unsigned int parentEntityHandleVal,
             const Broc::string& tagName);  // 0x5CDA90
void LinkTo3(unsigned int entityHandleVal,
             unsigned int parentEntityHandleVal);  // 0x5CDBB0
void PlayerLinkTo1(unsigned int entityHandleVal,
                   unsigned int parentEntityHandleVal,
                   const Broc::string& tagName,
                   const Broc::vector& angleFrac);  // 0x5CDCB0
void PlayerLinkTo2(unsigned int entityHandleVal,
                   unsigned int parentEntityHandleVal,
                   const Broc::string& tagName);  // 0x5CDE20
void PlayerLinkTo3(unsigned int entityHandleVal,
                   unsigned int parentEntityHandleVal);  // 0x5CDF60
void UnLink(unsigned int entityHandleVal);  // 0x5CE090
void EnableLinkTo(unsigned int entityHandleVal);  // 0x5CE120
unsigned int DoSpawn1(unsigned int entityHandleVal,
                      const Broc::string& name, TPakInfo whichPak,
                      enumForceSpawn forceSpawn);  // 0x5CE2E0
unsigned int DoSpawn2(unsigned int entityHandleVal, TPakInfo whichPak,
                      enumForceSpawn forceSpawn);  // 0x5CE460
unsigned int StalinGradSpawn1(unsigned int entityHandleVal,
                              const Broc::string& name,
                              TPakInfo whichPak);  // 0x5CE4D0
unsigned int StalinGradSpawn2(unsigned int entityHandleVal,
                              TPakInfo whichPak);  // 0x5CE650
void GetOrigin(unsigned int entityHandleVal,
               Broc::vector& outVec);  // 0x5CE6C0
void GetEye(unsigned int entityHandleVal,
            Broc::vector& outVec);  // 0x5CE7A0
unsigned int UseBy(unsigned int entityHandleVal,
                    unsigned int otherEntityHandleVal);  // 0x5CE8B0
bool IsTouching(unsigned int entityHandleVal,
                unsigned int otherEntityHandleVal);  // 0x5CE9A0
void LockDoor(unsigned int entityHandleVal);  // 0x5CEE40
void UnLockDoor(unsigned int entityHandleVal);  // 0x5CEEA0
bool IsDoorLocked(unsigned int entityHandleVal);  // 0x5CEF00
void Delete(unsigned int entityHandleVal);  // 0x5CEF60
void SetTransparent(unsigned int entityHandleVal,
                    bool isTransparent);  // 0x5CEFD0
void SetAiType(unsigned int entityHandleVal, const Broc::string& modelName,
               TPakInfo pakInfo);  // 0x5CF250
void SetModelIndex(unsigned int entityHandleVal,
                   int iflIndex);  // 0x5CF410
float GetNormalHealth(unsigned int entityHandleVal);  // 0x5CF4A0
void SetNormalHealth(unsigned int entityHandleVal,
                     float fNormalHealth);  // 0x5CF520
void DoDamage(unsigned int entityHandleVal, float damage,
              const Broc::vector& vecIn, hitLocation_t hitLoc);  // 0x5CF6B0
void SetTakeDamage(unsigned int entityHandleVal,
                   int damage);  // 0x5CF840 (int mangle per manifest)
void InvulnerableForTime(unsigned int entityHandleVal,
                         float time);  // 0x5CF880
bool IsEntityInvulnerable(unsigned int entityHandleVal);  // 0x5CF8D0
void SetAlwaysRender(unsigned int entityHandleVal, int r);  // 0x5CF920
void Show(unsigned int entityHandleVal);  // 0x5CF960
void Hide(unsigned int entityHandleVal);  // 0x5CF9A0
int  SetContents(unsigned int entityHandleVal, int contents);  // 0x5CF9E0
void DisConnectPaths(unsigned int entityHandleVal);  // 0x5CFA30
void ConnectPaths(unsigned int entityHandleVal);  // 0x5CFAC0
void StartFiring(unsigned int entityHandleVal);  // 0x5CFB50
void StopFiring(unsigned int entityHandleVal);  // 0x5CFBC0
void ShootTurret(unsigned int entityHandleVal,
                 unsigned int turretOwnerOverride);  // 0x5CFC30
void SetMode(unsigned int entityHandleVal, int mode);  // 0x5CFCD0
unsigned int GetTurretOwner(unsigned int entityHandleVal);  // 0x5CFDB0
unsigned int GetOwner(unsigned int entityHandleVal);  // 0x5CFE30
void SetOwner(unsigned int entityHandleVal,
              unsigned int ownerHandleVal);  // 0x5CFE70
void SetTargetEntity(unsigned int entityHandleVal,
                     unsigned int otherEntityHandleVal);  // 0x5CFEF0
bool HasTargetEntity(unsigned int entityHandleVal);  // 0x5CFF90
void ClearTargetEntity(unsigned int entityHandleVal);  // 0x5D0000
void SetTurretTeam(unsigned int entityHandleVal,
                   const Broc::string& pszTeam);  // 0x5D0070
void MakeTurretUsable(unsigned int entityHandleVal);  // 0x5D0170
void MakeTurretUnusable(unsigned int entityHandleVal);  // 0x5D01E0
void SetTurretAccuracy(unsigned int entityHandleVal,
                       float accuracy);  // 0x5D0250
void SetTurretRange(unsigned int entityHandleVal, float range);  // 0x5D02C0
float GetTurretRange(unsigned int entityHandleVal);  // 0x5D0340
}

// GetEntType (0x5CA9F0) - global
void GetEntType(DbLinkedHandle<EntityHandleDb, Entity> entityHandle,
                Broc::string& type);

// scr.o batch 44 helpers (BrocEntity.cpp effect-event wrappers)
extern Handle PostEffectEventScriptCall(const Entity* ent,
                                        const char* scriptId,
                                        const Broc::vector& pos,
                                        const Broc::vector& facing,
                                        bool queue, TPakId pakid,
                                        bool important);  // 8-arg overload
extern Handle PostEffectEventScriptCall_Dir(const Entity* ent,
                                            const char* scriptId,
                                            const float* dir,
                                            bool queue);  // ?PostEffectEventScriptCall_Dir@@YA?AVHandle@@PBVEntity@@PBDQBM_N@Z
extern Handle PostEffectEventQueueDialog(const Entity* ent,
                                         const char* scriptId,
                                         int notifyHash);  // ?PostEffectEventQueueDialog@@YA?AVHandle@@PBVEntity@@PBDH@Z
extern void RegisterEffectWait(Entity* ent, int notifyHash);  // ?RegisterEffectWait@@YAXPAVEntity@@H@Z
extern void XAnimSetCompleteGoalWeight(XAnimTree* tree,
                                       unsigned int animIndex,
                                       float goalWeight, float goalTime,
                                       float rate, unsigned int notifyName,
                                       unsigned short notifyType,
                                       int bRestart);  // ?XAnimSetCompleteGoalWeight@@YAXPAVXAnimTree@@IMMMIGH@Z
static void nullsub_59(void* /*actor*/) {}
static void nullsub_65(void* /*actor*/) {}
static void nullsub_106(void* /*actor*/, int /*eState*/) {}

// scr.o batch 45 helpers (vehicle follow / attach)
extern bool VEH_AcquirePlayerFollowSlot(Entity* vehicle,
                                        Entity* follower);  // ?VEH_AcquirePlayerFollowSlot@@YA_NPAVEntity@@0@Z
extern void VEH_FillFollowHistoryBuffer(
    scr_vehicle_t* veh);  // ?VEH_FillFollowHistoryBuffer@@YAXPAUscr_vehicle_t@@@Z
extern void G_EntDetachAll(Entity* ent);  // ?G_EntDetachAll@@YAXPAVEntity@@@Z (g_dobj.cpp)
extern void XAnimResetAnimVariationChunkState(
    XAnimTree* tree, unsigned int animIndex);  // ?XAnimResetAnimVariationChunkState@@YAXPAVXAnimTree@@I@Z (nal.cpp)
extern void* gDebugEntity;  // ?gDebugEntity@@3PAXA @ 0xF3A5CC

// scr.o batch 46 helpers (link / spawn)
extern const char* G_GetEntityTypeName(Entity* ent);  // ?G_GetEntityTypeName@@YAPBDPAVEntity@@@Z (g_utils.cpp)
extern void SV_DObjDumpInfo(Entity* entity);  // ?SV_DObjDumpInfo@@YAXPAVEntity@@@Z (sv_game.cpp 0x51EC30)
extern Entity* SpawnActor(Entity* ent, const Broc::string& targetname,
                          enumForceSpawn forceSpawn,
                          TPakId pakId);  // ?SpawnActor@@YAPAVEntity@@PAV1@ABVstring@Broc@@W4enumForceSpawn@@W4TPakId@@@Z (ai_stubs.cpp)
extern void G_SetModelIndex(Entity* ent, int iflIndex);  // ?G_SetModelIndex@@YAXPAVEntity@@H@Z (g_dobj.cpp 0x453BC0)

// Scr_LoadAnimTreeAtIndex / Scr_FreeAnimTreeAtIndex (0x5C7730 / 0x5C7820)
void Scr_LoadAnimTreeAtIndex(int treeindex,
                             void* (__cdecl* Alloc)(int),
                             bool restart);
void Scr_FreeAnimTreeAtIndex(int treeindex);

// Scr_EmitAnimation (0x5C1A60) - global
void Scr_EmitAnimation(char* a, unsigned short b, unsigned int c);
// BrocAddEntityThread (0x5BE3D0) - global
void BrocAddEntityThread(Entity* ent, unsigned int fcnHash,
                         class ScriptEventParams* params);
// BrocInitEntity (0x5C5790) - global
void BrocInitEntity(
    Entity* ent,
    const InplaceVector<InplaceTreeElement<InplaceString, InplaceString>>&
        keyValuePairs);
// VM_Restart (0x5C7940) - global
vm_s* VM_Restart(vm_s* vm);

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
void BrocSys::HudSetClockInternal(int elemNum, he_type_t type,
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
// scr.o batch 30 - BrocSys wrappers (RoundOver..GetNumVehicles)
// ============================================================================

// ea: 0x005BC2C0
void BrocSys::RoundOver(int condition, const Broc::string& winner)
{
    if (winner == str_const.axis)
    {
        MultiplayerMgr::sInst->RoundOver(condition, 1);
    }
    else if (winner == str_const.allies)
    {
        MultiplayerMgr::sInst->RoundOver(condition, 2);
    }
    else
    {
        (void)(winner == str_const.neutral);
        MultiplayerMgr::sInst->RoundOver(condition, 3);
    }
}

// ea: 0x005BC6A0
void BrocSys::NextRound(bool allowChange)
{
    MultiplayerMgr::sInst->NextRound(allowChange);
}

// ea: 0x005BC6C0 (tail jmp to MPUIInterface::NextRoundMapChanges)
bool BrocSys::NextRoundMapChanges()
{
    return MPUIInterface::NextRoundMapChanges();
}

// ea: 0x005BD730
void BrocSys::Reset(const Broc::hudelem& hudElem)
{
    unsigned int mHudIndex = hudElem.___u0;
    if ((mHudIndex & 0x80000000) != 0 || mHudIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3619;
        AeAssert::gCurrentExpr =
            "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", mHudIndex))
            __debugbreak();
    }
    HudSetDefaults(&g_hudelems[mHudIndex]);
}

// ea: 0x005BD7A0
void BrocSys::Destroy(const Broc::hudelem& hudElem)
{
    int mHudIndex = (int)hudElem.___u0;
    if ((mHudIndex & 0x80000000) != 0 || mHudIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3627;
        AeAssert::gCurrentExpr =
            "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", mHudIndex))
            __debugbreak();
    }
    if (mHudIndex >= 0)
        HudFree(&g_hudelems[mHudIndex]);
}

// ea: 0x005BD8B0 (binary symbol has (unsigned int, const char*))
void BrocSys::PlayScriptedAnim(unsigned int entHandleVal, const char* eventId)
{
    (void)entHandleVal;
    (void)eventId;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
    AeAssert::gCurrentLine = 3662;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert(" Empty function for MP !! "))
        __debugbreak();
}

// ea: 0x005BE6C0
int BrocSys::GetPlayerArray(unsigned int* array)
{
    int v1 = 0;
    for (int i = 0; i < 16; ++i)
    {
        Entity* v3 = EntityManager::sInst->mPlayers[i];
        if (v3 != nullptr && v3->sentient != nullptr)
            array[v1++] = v3->mHandle.mHandle.mVal;
    }
    return v1;
}

// ea: 0x005BE750
int BrocSys::GetLocalPlayerArray(unsigned int* array)
{
    Entity* v1 = EntityManager::sInst->mPlayers[0];
    if (v1 == nullptr || v1->sentient == nullptr || !v1->IsLocalPlayer())
        return 0;
    *array = v1->mHandle.mHandle.mVal;
    return 1;
}

// ea: 0x005BE810
void BrocSys::Scr_SetAngles(Entity* ent, int offset, Broc::vector* val)
{
    (void)offset;
    if (ent->actor != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 632;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "cannot directly set the angles on AI.  Use the teleport command instead.\n"))
            __debugbreak();
    }
    G_SetAngle(ent, &val->x);
}

// ea: 0x005BE870
void BrocSys::Scr_GetAngles(Entity* ent, int offset, Broc::vector* val)
{
    (void)offset;
    Client* client = ent->client;
    if (client != nullptr)
    {
        val->x = client->ps.viewangles[0];
        val->y = client->ps.viewangles[1];
        val->z = client->ps.viewangles[2];
    }
    else
    {
        val->x = ent->r.currentAngles.v.m128_f32[0];
        val->y = ent->r.currentAngles.v.m128_f32[1];
        val->z = ent->r.currentAngles.v.m128_f32[2];
    }
}

// ea: 0x005BE8D0
void BrocSys::Scr_SetGroupName(Entity* ent, int offset, Broc::string* val)
{
    (void)offset;
    ent->mGroupName = *val;
    if (val->mBlock != nullptr)
        ent->mGroupNameHash = HashString::CalcHash(
            (const char*)(val->mBlock + 1));
    else
        ent->mGroupNameHash = HashString::CalcHash(defaultFileName);
}

// ea: 0x005BE920
void BrocSys::Scr_SetTargetName(Entity* ent, int offset, Broc::string* val)
{
    (void)offset;
    ent->targetname = *val;
    if (val->mBlock != nullptr)
        ent->targetnameHash = HashString::CalcHash(
            (const char*)(val->mBlock + 1));
    else
        ent->targetnameHash = HashString::CalcHash(defaultFileName);
}

// ea: 0x005BE970
void BrocSys::Scr_SetTarget(Entity* ent, int offset, Broc::string* val)
{
    (void)offset;
    ent->mTarget = *val;
    if (val->mBlock != nullptr)
        ent->mTargetHash = HashString::CalcHash(
            (const char*)(val->mBlock + 1));
    else
        ent->mTargetHash = HashString::CalcHash(defaultFileName);
}

// ea: 0x005BE9C0
void BrocSys::Scr_SetNoteWorthy(Entity* ent, int offset, Broc::string* val)
{
    (void)offset;
    ent->mScriptNoteworthy = *val;
    if (val->mBlock != nullptr)
        ent->mScriptNoteworthyHash = HashString::CalcHash(
            (const char*)(val->mBlock + 1));
    else
        ent->mScriptNoteworthyHash = HashString::CalcHash(defaultFileName);
}

// ea: 0x005BEA10
void BrocSys::Scr_SetAnimName(Entity* ent, int offset, Broc::string* val)
{
    (void)offset;
    ent->mAnimName = *val;
    if (val->mBlock != nullptr)
        ent->mAnimNameHash = HashString::CalcHash(
            (const char*)(val->mBlock + 1));
    else
        ent->mAnimNameHash = HashString::CalcHash(defaultFileName);
}

// ea: 0x005BF8B0
void BrocSys::SetTutorialText(int hash, int viewport)
{
    if (viewport != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 6671;
        AeAssert::gCurrentExpr = "viewport >= 0 && viewport < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "SetTutorialText called for a non local player"))
            __debugbreak();
    }
    if (viewport == 0)
        g_femanager.IGO->SetTutorialText(hash, 0);
}

// ea: 0x005BF940
void BrocSys::SetActionHint(int hash, int viewport)
{
    if (viewport != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 6693;
        AeAssert::gCurrentExpr = "viewport >= 0 && viewport < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "SetTutorialText called for a non local player"))
            __debugbreak();
    }
    if (viewport == 0)
        g_femanager.IGO->SetActionHint(hash, 0);
}

// ea: 0x005BF860
void BrocSys::SetWeaponCameraShakeScale(float scale, int onlyADS)
{
    CameraShake* v2 = &g_cameraShake[currCl];
    v2->m_scaleCOD = scale;
    v2->m_scaleCOD_onlyADS = onlyADS;
}

// ea: 0x005BFB90
void BrocSys::NoClip(int val)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr)
    {
        if (val != 0)
        {
            Player->client->noclip = 1;
            gNoClipEnabled = true;
        }
        else
        {
            Player->client->noclip = 0;
            gNoClipEnabled = false;
        }
    }
}

// ea: 0x005BEC40
unsigned int BrocSys::GetNumVehicles()
{
    int MaxVehicles = level.MaxVehicles;
    unsigned int result = 0;
    if (level.MaxVehicles != 0)
    {
        DbLinkedHandle<EntityHandleDb, Entity>* p_mEntity =
            &s_vehicles->mEntity;
        do
        {
            if (p_mEntity->mHandle.mVal != 0)
                ++result;
            p_mEntity += 468;
            --MaxVehicles;
        } while (MaxVehicles != 0);
    }
    return result;
}

// ============================================================================
// scr.o batch 31 - BrocSys wrappers (RegisterBroFunc..Mover_GravityMove)
// ============================================================================

extern void* G_GetModel(const char* modelName, TPakId pakId);
    // ?G_GetModel@@YAPAXPBDW4TPakId@@@Z (g.o 0x464C70)
extern char* Cvar_VariableString(const char* var_name);  // core.o
extern bool AttachCurveEntity(unsigned int entityHandleVal,
                              char* filename);  // g_cmd.cpp

// ea: 0x005BE1A0
void BrocHelper::RegisterBroFunc(char* name,
                                 unsigned int (__cdecl* func)(void*))
{
    BrocHelper::brocFunctionLookup* v2 = BrocHelper::broFuncLookupTable;
    for (int i = 0; i < 70; ++i)
    {
        if (v2->mHashedName == 0)
        {
            v2->mHashedName = HashString::CalcHash(name);
            v2->mFunction = func;
            return;
        }
        ++v2;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
    AeAssert::gCurrentLine = 5424;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error(
            "Out of lookup entries. Increase MAX_BRO_FUNCTION_LOOKUPS(BrocSys.h)"))
        __debugbreak();
}

// ea: 0x005BEAF0
int BrocSys::GetNode(const Broc::string& inName, const Broc::string& key,
                     int* array, int capacity)
{
    (void)capacity;
    int Node = PathNodeMgr::sInst->GetNode(inName, key, array);
    int v4 = Node;
    if (array == nullptr && Node < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 889;
        AeAssert::gCurrentExpr = "array || node >= 0";
        if (!AeAssert::IsIgnored())
        {
            const char* v5 = inName.mBlock != nullptr
                                 ? (const char*)(inName.mBlock + 1)
                                 : defaultFileName;
            const char* v6 = key.mBlock != nullptr
                                 ? (const char*)(key.mBlock + 1)
                                 : defaultFileName;
            if (AeAssert::Assert("Failed to get pathnode with %s = %s", v6, v5))
                __debugbreak();
        }
    }
    return v4;
}

// ea: 0x005BEB90
int BrocSys::GetVehicleNode(const Broc::string& inName,
                            const Broc::string& key, int* array,
                            int capacity)
{
    (void)capacity;
    int v4 = 0;
    if (inName.mBlock == nullptr || key.mBlock == nullptr)
        v4 = 1;
    int result = PathNodeMgr::sInst->GetVehicleNodeIndex(
        inName, key, array, v4);
    int node = result;
    if (array == nullptr && result < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 906;
        AeAssert::gCurrentExpr = "array || node >= 0";
        if (!AeAssert::IsIgnored())
        {
            const char* v7 = inName.mBlock != nullptr
                                 ? (const char*)(inName.mBlock + 1)
                                 : defaultFileName;
            const char* v8 = key.mBlock != nullptr
                                 ? (const char*)(key.mBlock + 1)
                                 : defaultFileName;
            if (AeAssert::Assert("Failed to get vehicle node with %s = %s",
                                 v8, v7))
                __debugbreak();
        }
        return node;
    }
    return result;
}

// ea: 0x005BED00
bool BrocSys::BROC_AttachCurveEntity(unsigned int entityHandleVal,
                                     Broc::string& filename)
{
    if (filename.mBlock != nullptr)
        return AttachCurveEntity(entityHandleVal,
                                 (char*)(filename.mBlock + 1));
    else
        return AttachCurveEntity(entityHandleVal, (char*)defaultFileName);
}

// ea: 0x005BED90
int BrocSys::CheckWave(const Broc::string& script)
{
    const char* v1 = script.mBlock != nullptr
                         ? (const char*)(script.mBlock + 1)
                         : defaultFileName;
    return SoundDevice::sInst->FindWave(v1) != NSL_WAVE_ID_INVALID;
}

// ea: 0x005BEEB0
int BrocSys::GetAnimFromScriptCVars()
{
    Broc::string animTree(Cvar_VariableString("script_animtree"));
    Broc::string animName(Cvar_VariableString("script_name"));
    if (animTree.mBlock != nullptr
        && animTree.mBlock + 1 != (Broc::string::Block*)-12
        && *(char*)(animTree.mBlock + 1) != 0
        && animName.mBlock != nullptr
        && animName.mBlock + 1 != (Broc::string::Block*)-12
        && *(char*)(animName.mBlock + 1) != 0)
    {
        unsigned int v4 = gpBrocAPI->mBrocExports.mAnimResolver(
            (const char*)(animTree.mBlock + 1),
            (const char*)(animName.mBlock + 1));
        return (int)v4;
    }
    return 0;
}

// ea: 0x005BEFB0
bool BrocSys::IsModelLoaded(const Broc::string& modelName, TPakInfo pakInfo)
{
    if (modelName.mBlock == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 2943;
        AeAssert::gCurrentExpr = "modelName.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("IsModelLoaded: model name undefined."))
            __debugbreak();
        if (modelName.mBlock == nullptr)
            return false;
    }
    TPakId v3;
    if (pakInfo != kTPakInfoInvalid)
        v3 = *(TPakId*)((char*)pakInfo + 0xB4);
    else
        v3 = CurPakId();
    if (modelName.mBlock != nullptr)
        return G_GetModel((const char*)(modelName.mBlock + 1), v3) != nullptr;
    else
        return G_GetModel(defaultFileName, v3) != nullptr;
}

// ea: 0x005BF0B0
void BrocSys::ValidateLightVis(int eType)
{
    if (eType != 0 && eType != 7)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 4731;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            const char* v1 = va(
                "(un)lockLightVis: entity type '%i' is not yet handled, get a coder to fix it\n",
                eType);
            if (AeAssert::Warning(v1))
                __debugbreak();
        }
    }
}

// ea: 0x005BF110
void BrocSys::ActorScr_Clamp_0_1(actor_s* a, int offset, float* val)
{
    float v4 = *val;
    float* v5 = (float*)((char*)a + offset);
    if (*val <= 1.0f)
    {
        if (v4 < 0.0f)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 5412;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored())
            {
                if (AeAssert::Warning(
                        "actor field clamped from %g to 0\n", v5))
                    __debugbreak();
            }
            v4 = 0.0f;
        }
        *v5 = v4;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 5407;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            if (AeAssert::Warning(
                    "actor field clamped from %g to 1\n", v5))
                __debugbreak();
        }
        *v5 = 1.0f;
    }
}

// ea: 0x005BF1D0
void BrocSys::ActorScr_ReadOnly(actor_s* a, int offset, void* val)
{
    (void)a;
    (void)offset;
    (void)val;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
    AeAssert::gCurrentLine = 5421;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("Actor field is read only!"))
        __debugbreak();
}

// ea: 0x005BFA30
void BrocSys::SetGameVectorVar(unsigned int hashVarName,
                               const Broc::vector& vect)
{
    unsigned int val[3];
    val[0] = (unsigned int&)vect.x;
    val[1] = (unsigned int&)vect.y;
    val[2] = (unsigned int&)vect.z;
    CheckpointMgr::sInst->SetGameVar(hashVarName, val, 3u);
}

// ea: 0x005BFAD0
Broc::vector BrocSys::GetGameVectorVar(unsigned int hashVarName)
{
    Broc::vector vect;
    vect.x = sNaN;
    vect.y = sNaN;
    vect.z = sNaN;
    CheckpointMgr::sInst->GetGameVar(hashVarName, (unsigned int*)&vect, 3u);
    return vect;
}

// ea: 0x005C0B00
bool BrocSys::IsValidClientType(Entity* pEnt)
{
    if (pEnt != nullptr && pEnt->client != nullptr)
        return true;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityClient.cpp";
    AeAssert::gCurrentLine = 62;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored())
    {
        const char* v2 = va("entity is not a player");
        if (AeAssert::Warning(v2))
            __debugbreak();
    }
    return false;
}

// ea: 0x005C0B60
void BrocSys::MPScript_RequestRespawn(unsigned int playerID)
{
    MultiplayerMgr::sInst->SendRespawnRequest(playerID);
}

// ea: 0x005C0910
void BrocSys::Mover_GravityMove(Entity* pEnt, const float* const vVel,
                                float fTotalTime, float gravityOverride)
{
    trajectory_t* p_pos = &pEnt->s.pos;
    pEnt->s.pos.trTime = level.time;
    pEnt->s.pos.trDuration = (int)(fTotalTime * 1000.0);
    memcpy(pEnt->s.pos.trBase,
           &pEnt->r.currentOrigin.v.m128_f32[0],
           sizeof(pEnt->s.pos.trBase));
    pEnt->s.pos.trDelta[0] = vVel[0];
    float v5 = pEnt->s.pos.trDelta[0];
    pEnt->s.pos.trDelta[1] = vVel[1];
    pEnt->s.pos.trDelta[2] = vVel[2];
    if (IS_NAN(v5) || IS_NAN(pEnt->s.pos.trDelta[1])
        || IS_NAN(pEnt->s.pos.trDelta[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityMove.cpp";
        AeAssert::gCurrentLine = 291;
        AeAssert::gCurrentExpr =
            "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    p_pos->trType = TR_GRAVITY;
    pEnt->s.pos.trGravityOverride = (int)gravityOverride;
    BG_EvaluateTrajectory(p_pos, level.time, pEnt->r.currentOrigin);
    g_LinkEntity(pEnt);
}

// ============================================================================
// scr.o batch 32 - BrocSys wrappers (AnimScripted1..Cinematic2)
// ============================================================================

extern unsigned int AeHash(const char* str);  // ?AeHash@@YAIPBD@Z (core.o)
extern const char* Info_ValueForKey(const char* s, const char* key);  // core.o
extern void Q_strncpyz(char* dest, const char* src, int destsize);  // core.o
extern vmCvar_t g_changelevel_time;  // ?g_changelevel_time@@3UvmCvar_t@@A

// SplineMgr / SplinePath view (game2.o; full in g_game2_misc.cpp)
struct SplinePath {
    void*         mSpline;       // +0x00
    unsigned int* mEventIndices; // +0x04
    unsigned int* mEventHashes;  // +0x08
};
class SplineMgr {
public:
    static SplineMgr* sInst;  // ?sInst@SplineMgr@@2PAV1@A (g_game2_misc.cpp)
    void GetSpline(const char* name, SplinePath* splinePath);  // ?GetSpline@SplineMgr@@QAEXPBDPAUSplinePath@@@Z (game2.o 0x5045C0)
};

// AnimBroRef registration list (scr.o data)
template <typename T>
struct ae_array_dynamic {
    T**            m_elements;  // +0x00
    unsigned short m_capacity;  // +0x04
    short          m_size;      // +0x06
    void push_back(T const* elt);
};
ae_array_dynamic<AnimBroRef*> gAnimRefRegList;  // scr.o data

// ea: 0x005BEE00
void BrocSys::AnimScripted1(unsigned int entityHandleVal,
                            unsigned int notifyName,
                            const Broc::vector& origin,
                            const Broc::vector& angles,
                            unsigned int broanim)
{
    (void)entityHandleVal;
    (void)notifyName;
    (void)origin;
    (void)angles;
    Broc::string s((Broc::string::Block*)nullptr);
    if ((unsigned short)broanim == 0xFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::DK;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 1691;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Animation passed into AnimScripted was not fixed up.  You probably ignored the warning when the game loaded! FOO"))
            __debugbreak();
    }
}

// ea: 0x005BF320
void BrocSys::ActorScr_SetAnimPos(actor_s* a, int offset,
                                  const unsigned int* val)
{
    (void)offset;
    int IsProne = BG_ActorGoalIsProne(&a->ProneInfo);
    if (val == nullptr)
    {
        a->mAnimPose = 0;
        return;
    }
    if (*val == hash_const.prone.mHash)
    {
        if (IsProne != 0)
            goto set_pose;
    }
    else if (IsProne == 0)
    {
        goto set_pose;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
    AeAssert::gCurrentLine = 5515;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning(
               "Entnum %d is attempting to change to bad anim_pose.",
               a->pEnt->mHandle.mHandle.mVal))
        __debugbreak();
    return;
set_pose:
    a->mAnimPose = *val;
}

// ea: 0x005BF3B0
void BrocSys::ActorScr_GetFavoriteEnemy(actor_s* a, int offset,
                                        Broc::entity* val)
{
    (void)offset;
    unsigned int mVal = 0;
    if (a != nullptr)
    {
        sentient_s* pSentient = a->pSentient;
        if (pSentient != nullptr)
            mVal = pSentient->pEnt->mHandle.mHandle.mVal;
        val->___u0 = mVal;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 5565;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("trying to get field off null actor"))
            __debugbreak();
    }
}

// ea: 0x005BF470
void BrocSys::PathNode_SetType(PathNodes::PathNode* pNode, int offset,
                               Broc::string* val)
{
    (void)pNode;
    (void)offset;
    (void)val;
    AeAssert::gCurrentAuthor = AeAssert::JRS;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
    AeAssert::gCurrentLine = 5586;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("PathNode.type is read-only"))
        __debugbreak();
}

// ea: 0x005BF520
void BrocSys::SentientScr_ConvertNode(sentient_s* pSelf, int offset,
                                      Broc::pathnode* pNode)
{
    (void)offset;
    unsigned short mValue = pSelf->mClaimedNode.mValue;
    if (mValue != 0 && mValue != 0xFFFF
        && pSelf->mClaimedNode.operator->() != nullptr)
        pNode->___u0 = mValue;
    else
        pNode->___u0 = 0;  // INVALID_PATHNODE_HANDLE
}

// ea: 0x005BF5B0
void BrocSys::SentientScr_GetTeam(sentient_s* pSelf, int offset,
                                  Broc::string* val)
{
    (void)offset;
    switch (pSelf->eTeam)
    {
    case TEAM_AXIS:
        *val = str_const.axis;
        break;
    case TEAM_ALLIES:
        *val = str_const.allies;
        break;
    case TEAM_NEUTRAL:
        *val = str_const.neutral;
        break;
    case TEAM_DEAD:
        *val = str_const.dead;
        break;
    default:
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 5677;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "SentientScr_GetTeam: default case (shouldn't happen"))
            __debugbreak();
        break;
    }
}

// ea: 0x005BF650
void BrocSys::SentientScr_SetGoalRadius(sentient_s* pSelf, int offset,
                                        float* val)
{
    (void)offset;
    float v3 = 0.0f;
    if (*val >= 0.0f)
        v3 = *val;
    Sentient_SetGoalRadius(pSelf, v3);
}

// ea: 0x005BF750
void BrocSys::GetSplineData(const char* splineName, Broc::vector*& origins,
                            unsigned int*& eventIndices,
                            unsigned int*& eventHashes)
{
    SplinePath splinePath;
    SplineMgr::sInst->GetSpline(splineName, &splinePath);
    if (splinePath.mSpline != nullptr)
    {
        origins = (Broc::vector*)splinePath.mSpline;
        eventIndices = splinePath.mEventIndices;
        eventHashes = splinePath.mEventHashes;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 6552;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "no spline named %s found, DRONES/SHRIMPS ON SPLINE WILL NOT WORK!!! See STAVRO",
                   splineName))
            __debugbreak();
        origins = nullptr;
        eventIndices = nullptr;
        eventHashes = nullptr;
    }
}

// ea: 0x005C1800
void BrocSys::ToggleClip()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr)
    {
        const char* v1;
        if (Player->client->noclip != 0)
        {
            v1 = "GAME_NOCLIPOFF";
            gNoClipEnabled = false;
        }
        else
        {
            v1 = "GAME_NOCLIPON";
            gNoClipEnabled = true;
        }
        Player->client->noclip = Player->client->noclip == 0;
        DbLinkedHandle<EntityHandleDb, Entity> v2;
        v2.mHandle.mVal = Player->mHandle.mHandle.mVal;
        const char* v3 = va("print \"%s\"", v1);
        SV_GameSendServerCommand(v2, v3);
    }
}

// ea: 0x005C1900
void ObjectiveFailedNotify()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr)
    {
        HashString v1;
        v1.mHash = AeHash("ObjectiveFailed");
        Player->Notify(v1);
    }
}

// ea: 0x005C1980
int BrocSys::FindUnusedChildObjective()
{
    int v0 = 0;
    for (;;)
    {
        char szConfigString[256];
        SV_GetConfigstring(v0 + 32, szConfigString, 256);
        if (szConfigString[0] == 0)
            break;
        if (++v0 >= 1)
            return -1;
    }
    return v0;
}

// ea: 0x005C19D0
void BrocSys::DeleteAllChildrenOfObjective(int iObjective)
{
    char szConfigString[256];
    SV_GetConfigstring(32, szConfigString, 256);
    if (szConfigString[0] != 0)
    {
        const char* v1 = Info_ValueForKey(szConfigString, "pobj");
        if (*v1 != 0 && atoi(v1) == iObjective)
        {
            szConfigString[0] = 0;
            SV_SetConfigstring(32, szConfigString);
        }
    }
}

// ea: 0x005C21C0
void BrocSys::IPrintLn(const char* txt)
{
    char buff[8192];
    sprintf(buff, "%s\n", txt);
    const char* v1 = va("%s \"%s\"", "gm", buff);
    SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(0), v1);
}

// ea: 0x005C2210
void BrocSys::IPrintLnBold(const char* txt)
{
    char buff[8192];
    sprintf(buff, "%s\n", txt);
    const char* v1 = va("%s \"%s\"", "gmb", buff);
    SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity>(0), v1);
}

// ea: 0x005C2260
void BrocSys::RegisterAnimation(struct AnimBroRef* animBroRef)
{
    gAnimRefRegList.push_back(&animBroRef);
}

// ea: 0x005C22E0
void BrocSys::ThreadDebugNotice(const char* msg)
{
    if (((AeThread*)AeThreadManager::sInst.mThreadExecuting)
            ->mFlags.mMask
        & 0x100)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1225;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("0x%08x %s",
                                 AeThreadManager::sInst.mThreadExecuting,
                                 msg))
            __debugbreak();
    }
}

// ea: 0x005C3690
int BrocSys::GetTeamFlags(const Broc::string& teamName,
                          const Broc::string& caller)
{
    if (teamName == str_const.axis)
        return 2;
    if (teamName == str_const.allies)
        return 4;
    if (teamName == str_const.neutral)
        return 8;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
    AeAssert::gCurrentLine = 2075;
    AeAssert::gCurrentExpr = nullptr;
    const char* v3 = caller.mBlock != nullptr
                         ? (const char*)(caller.mBlock + 1)
                         : defaultFileName;
    const char* v4 = teamName.mBlock != nullptr
                         ? (const char*)(teamName.mBlock + 1)
                         : defaultFileName;
    if (AeAssert::Error(
            "unknown team '%s' in %s (should be axis, allies, or neutral)",
            v4, v3))
        __debugbreak();
    return 0;
}

// ea: 0x005C3750
Broc::string BrocSys::GetWeaponClassName(unsigned int weaponName)
{
    unsigned char WeaponIndexForName = BG_GetWeaponIndexForName(weaponName);
    int v3 = WeaponIndexForName;
    if (WeaponIndexForName != 0)
    {
        int iAltWeaponIndex = WeaponIndexForName;
        while (*BG_GetInfoForWeapon(iAltWeaponIndex)->szRadiantName == 0)
        {
            iAltWeaponIndex =
                BG_GetInfoForWeapon(iAltWeaponIndex)->iAltWeaponIndex;
            if (iAltWeaponIndex == 0 || iAltWeaponIndex == v3)
            {
                const char* v4 = va(
                    "^3WARNING^7: no Radiant name found for weapon '%d' in getWeaponClassname\n",
                    weaponName);
                Com_Printf(v4);
                return Broc::string(defaultFileName);
            }
        }
        weaponFileInfo_t* InfoForWeapon =
            BG_GetInfoForWeapon(iAltWeaponIndex);
        return Broc::string(InfoForWeapon->szRadiantName);
    }
    else
    {
        if (weaponName != 0xFEFEFEFE
            && weaponName != hash_const.none.mHash)
        {
            const char* v4 = va(
                "unknown weapon '%s' in getWeaponClassname\n", weaponName);
            Com_Printf(v4);
        }
        return Broc::string(defaultFileName);
    }
}

// ea: 0x005C3810
Broc::string BrocSys::GetWeaponClassNameStr(unsigned int weaponName)
{
    unsigned char WeaponIndexForName = BG_GetWeaponIndexForName(weaponName);
    int v3 = WeaponIndexForName;
    if (WeaponIndexForName != 0)
    {
        int iAltWeaponIndex = WeaponIndexForName;
        while (*BG_GetInfoForWeapon(iAltWeaponIndex)->szRadiantName == 0)
        {
            iAltWeaponIndex =
                BG_GetInfoForWeapon(iAltWeaponIndex)->iAltWeaponIndex;
            if (iAltWeaponIndex == 0 || iAltWeaponIndex == v3)
            {
                const char* v4 = va(
                    "^3WARNING^7: no Radiant name found for weapon '%d' in getWeaponClassname\n",
                    weaponName);
                Com_Printf(v4);
                return Broc::string(defaultFileName);
            }
        }
        weaponFileInfo_t* InfoForWeapon =
            BG_GetInfoForWeapon(iAltWeaponIndex);
        return Broc::string(InfoForWeapon->szRadiantName);
    }
    else
    {
        if (weaponName != 0xFEFEFEFE
            && weaponName != hash_const.none.mHash)
        {
            const char* v4 = va(
                "unknown weapon '%s' in getWeaponClassname\n", weaponName);
            Com_Printf(v4);
        }
        return Broc::string(defaultFileName);
    }
}

// ea: 0x005C3EB0
void BrocSys::MissionFailed(const Broc::string& reason)
{
    Entity* v1 = EntityHandleDb::sInst.Find(640, hash_const.player);
    if (v1 != nullptr && (v1->flags & 1) == 0)
    {
        // j_nullsub_24(v1) - empty stub in binary
        level.bMissionSuccess = 0;
        level.bMissionFailed = 1;
        level.strMissionFailedReason.clear();
        if (reason == "INGAME_MISSIONFAIL_FF")
        {
            const char* v2 = reason.mBlock != nullptr
                                 ? (const char*)(reason.mBlock + 1)
                                 : defaultFileName;
            const char* STBString = STBManager::sInst->GetSTBString(v2);
            if (STBString != nullptr)
                level.strMissionFailedReason = STBString;
        }
    }
}

// ea: 0x005C3F40
void BrocSys::Cinematic1(const Broc::string& pszCinematic, float fVal)
{
    if (g_reloading.integer == 0)
    {
        level.exitTime = (int)((fVal * 1000.0) + 0.5);
        if ((int)((fVal * 1000.0) + 0.5) < 0)
            Scr_ParamError(1u, "exitTime cannot be negative");
        if (g_changelevel_time.value >= 0.0)
            level.exitTime =
                (int)((g_changelevel_time.value * 1000.0) + 0.5);
        level.changelevel = 1;
        const char* v2 = pszCinematic.mBlock != nullptr
                             ? (const char*)(pszCinematic.mBlock + 1)
                             : defaultFileName;
        Q_strncpyz(level.nextMap, v2, 256);
        level.bMissionSuccess = 1;
    }
}

// ea: 0x005C3FF0
void BrocSys::Cinematic2(const Broc::string& pszCinematic)
{
    if (g_reloading.integer == 0)
    {
        level.changelevel = 1;
        const char* v1 = pszCinematic.mBlock != nullptr
                             ? (const char*)(pszCinematic.mBlock + 1)
                             : defaultFileName;
        Q_strncpyz(level.nextMap, v1, 256);
        level.bMissionSuccess = 1;
    }
}

// ============================================================================
// scr.o batch 33 - BrocSys wrappers (NewHudElem..PlayLocalSound)
// ============================================================================

// weaponInfo_s view (cg.o; hADSOverlay +0xC0 verified vs disasm)
struct weaponInfo_s {
    unsigned char _pad[0xC0];
    void* hADSOverlay;  // +0xC0
};
extern weaponInfo_s* cg_weapons;  // ?cg_weapons@@3PAUweaponInfo_s@@A (cg.o)

// ea: 0x005C49B0
Broc::hudelem BrocSys::NewHudElem(int panelType)
{
    unsigned int HudElemAllocIndex = GetHudElemAllocIndex();
    Broc::hudelem result;
    if (HudElemAllocIndex == 0xFFFFFFFF)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3014;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "Out of Hudelements. Returning global element which will result in unknown script/hud behavior."))
            __debugbreak();
        result.___u0 = BrocSys::gHudElement.___u0;
        return result;
    }
    game_hudelem_s* v3 = &g_hudelems[HudElemAllocIndex];
    switch (panelType)
    {
    case 0:
        v3->elem.type = HE_TYPE_COUNT;
        result.___u0 = HudElemAllocIndex;
        break;
    case 1:
        v3->elem.type = HE_TYPE_COUNT | HE_TYPE_TEXT;
        result.___u0 = HudElemAllocIndex;
        break;
    case 2:
        v3->elem.type = HE_TYPE_CLOCK_DOWN | HE_TYPE_TIMER_DOWN;
        result.___u0 = HudElemAllocIndex;
        break;
    case 3:
        v3->elem.type = HE_TYPE_CLOCK_UP | HE_TYPE_TIMER_DOWN;
        result.___u0 = HudElemAllocIndex;
        break;
    case 4:
        v3->elem.type = HE_TYPE_COUNT | HE_TYPE_TIMER_DOWN;
        result.___u0 = HudElemAllocIndex;
        break;
    default:
        result.___u0 = HudElemAllocIndex;
        break;
    }
    return result;
}

// ea: 0x005C4A80
TPakInfo BrocSys::GetPak(const Broc::string& longname)
{
    const char* v1 = longname.mBlock != nullptr
                         ? (const char*)(longname.mBlock + 1)
                         : defaultFileName;
    const PakInfoNode* PakInfo = PakManager::sInst->GetPakInfo(v1);
    if (PakInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3043;
        AeAssert::gCurrentExpr = "result";
        if (!AeAssert::IsIgnored())
        {
            const char* v3 = longname.mBlock != nullptr
                                 ? (const char*)(longname.mBlock + 1)
                                 : defaultFileName;
            if (AeAssert::Assert("Unknown pak file '%s'", v3))
                __debugbreak();
        }
    }
    return (TPakInfo)(uintptr_t)PakInfo;
}

// ea: 0x005C51A0
void BrocSys::SetTimerUp(const Broc::hudelem& hudElem, float fVal)
{
    unsigned int mHudIndex = hudElem.___u0;
    if ((mHudIndex & 0x80000000) != 0 || mHudIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3307;
        AeAssert::gCurrentExpr =
            "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", mHudIndex))
            __debugbreak();
    }
    double v3 = fVal * 1000.0;
    game_hudelem_s* v4 = &g_hudelems[mHudIndex];
    int time = level.time;
    v4->elem.width = 0;
    v4->elem.height = 0;
    v4->elem.mTexture = nullptr;
    v4->elem.fromWidth = 0;
    v4->elem.fromHeight = 0;
    v4->elem.scaleStartTime = 0;
    v4->elem.scaleTime = 0;
    v4->elem.duration = 0;
    v4->elem.text = 0;
    v4->elem.value = 0.0f;
    v4->elem.type = HE_TYPE_TIMER_UP;
    v4->elem.time = (int)ceil(v3) + time;
}

// ea: 0x005C5280
void BrocSys::SetTenthsTimerUp(const Broc::hudelem& hudElem, float fVal)
{
    unsigned int mHudIndex = hudElem.___u0;
    if ((mHudIndex & 0x80000000) != 0 || mHudIndex >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3307;
        AeAssert::gCurrentExpr =
            "elemNum >= 0 && elemNum < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", mHudIndex))
            __debugbreak();
    }
    double v3 = fVal * 1000.0;
    game_hudelem_s* v4 = &g_hudelems[mHudIndex];
    int v5 = (int)ceil(v3);
    if (v5 <= 0)
    {
        const char* v6 = va("time %g should be > 0", v5 * 0.001);
        Scr_ParamError(0, v6);
    }
    int time = level.time;
    v4->elem.width = 0;
    v4->elem.height = 0;
    v4->elem.mTexture = nullptr;
    v4->elem.fromWidth = 0;
    v4->elem.fromHeight = 0;
    v4->elem.scaleStartTime = 0;
    v4->elem.scaleTime = 0;
    v4->elem.duration = 0;
    v4->elem.text = 0;
    v4->elem.value = 0.0f;
    v4->elem.type = HE_TYPE_TENTHS_TIMER_UP;
    v4->elem.time = v5 + time;
}

// ea: 0x005C5990
void BrocSys::Scr_SetHealth(Entity* ent, int offset, int* val)
{
    (void)offset;
    int v3 = *val;
    Client* client = ent->client;
    if (client != nullptr)
    {
        ent->health = v3;
        client->ps.stats[0] = v3;
    }
    else
    {
        if (v3 <= 0)
        {
            const char* v6;
            if (ent->targetname.mBlock == (Broc::string::Block*)-12)
            {
                v6 = "<not set>";
            }
            else
            {
                Broc::string::Block* mBlock = ent->targetname.mBlock;
                if (mBlock != nullptr)
                    v6 = (const char*)(mBlock + 1);
                else
                    v6 = defaultFileName;
            }
            const char* v7 = va(
                "self.health must be greater than 0 (tried to set %i on ent %i, name %s)\n",
                v3, ent->mHandle.mHandle.mVal, v6);
            Scr_Error(v7);
        }
        int health = ent->health;
        if (health <= 0 && ent->maxHealth != 0)
        {
            const char* v11;
            if (ent->targetname.mBlock == (Broc::string::Block*)-12)
            {
                v11 = "<not set>";
            }
            else
            {
                Broc::string::Block* v10 = ent->targetname.mBlock;
                if (v10 != nullptr)
                    v11 = (const char*)(v10 + 1);
                else
                    v11 = defaultFileName;
            }
            G_DPrintf(
                "^2Cannot set health on dead entities (health %i, max %i, ent %i, name %s)\n",
                health, ent->maxHealth, ent->mHandle.mHandle.mVal, v11);
        }
        else
        {
            ent->maxHealth = v3;
            ent->health = v3;
        }
    }
}

// ea: 0x005C5A70
void BrocSys::CreateAnimNotifyTask(const Broc::entity& ent,
                                   unsigned int animHash,
                                   unsigned int killHash)
{
    AnimNotifyTask* v3 =
        (AnimNotifyTask*)Task::sAllocator->Allocate(0x24u, false);
    AnimNotifyTask* v4;
    if (v3 != nullptr)
        v4 = new (v3) AnimNotifyTask(
            DbLinkedHandle<EntityHandleDb, Entity>((int)ent.___u0),
            animHash, killHash);
    else
        v4 = nullptr;
    TaskSys::sInst.PostTask(v4);
}

// ea: 0x005C5B60
void BrocSys::SentientScr_SetTeam(sentient_s* pSelf, int offset,
                                  Broc::string* val)
{
    (void)pSelf;
    (void)offset;
    (void)val;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
    AeAssert::gCurrentLine = 5605;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert(
               "Can not set the team from script this way in multiplayer. Call ChangePlayerTeam() instead."))
        __debugbreak();
}

// ea: 0x005C5BB0
void BrocSys::SetExploderState(int exploderNumber, int state)
{
    CheckpointMgr::sInst->mCurrentScriptExploded
        .mElements[exploderNumber] = (unsigned short)state;
}

// ea: 0x005C5C20
bool BrocSys::IsValidMoverType(Entity* pEnt)
{
    unsigned int mHash = pEnt->mClassNameHash.mHash;
    if (mHash == hash_const.script_brushmodel.mHash
        || mHash == hash_const.script_model.mHash
        || mHash == hash_const.script_origin.mHash)
    {
        return true;
    }
    const char* v2 =
        va("entity is not a script_brushmodel, script_model, or script_origin");
    Scr_Error(v2);
    return false;
}

// ea: 0x005C5C60
void BrocSys::GiveWeapon(Entity* pSelf, const char* pszWeaponName)
{
    if (IsValidClientType(pSelf))
    {
        int WeaponIndexForName = BG_GetWeaponIndexForName(pszWeaponName);
        Com_BitCheck(pSelf->client->ps.weapons, WeaponIndexForName);
        if (!BG_GetEmptySlotForWeapon(&pSelf->client->ps,
                                      WeaponIndexForName))
            Scr_ParamError(
                0, "Can not give player weapon without having an empty weapon slot\n");
        if (BG_GetInfoForWeapon(WeaponIndexForName)->type != WEAPTYPE_SPOTTER
            || cg_weapons[WeaponIndexForName].hADSOverlay)
            BG_GivePlayerWeapon(&pSelf->client->ps, WeaponIndexForName);
    }
}

// ea: 0x005C5D10
void BrocSys::SetReverb(unsigned int entityHandleVal,
                        const Broc::string& pszReverb, float wetlevel,
                        float fadetime)
{
    const char* v4 = va("reverb \"%s\" %g %g", pszReverb.mBlock, wetlevel,
                        fadetime);
    SV_GameSendServerCommand(
        DbLinkedHandle<EntityHandleDb, Entity>((int)entityHandleVal), v4);
}

// ea: 0x005C5D50
void BrocSys::PlayLocalSound(unsigned int entityHandleVal,
                             const Broc::string& pszSoundName)
{
    const char* v2 = pszSoundName.mBlock != nullptr
                         ? (const char*)(pszSoundName.mBlock + 1)
                         : defaultFileName;
    if (SoundDevice::sInst->FindWave(v2) == NSL_WAVE_ID_INVALID)
    {
        char* v3 = va("unknown sound alias '%s'", pszSoundName.mBlock);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 23;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v3))  // off_CFBB58 format
            __debugbreak();
    }
    const char* v4 = pszSoundName.mBlock != nullptr
                         ? (const char*)(pszSoundName.mBlock + 1)
                         : defaultFileName;
    unsigned char v5 = G_SoundAliasIndex(v4);
    const char* v6 = va("ls %i", v5);
    SV_GameSendServerCommand(
        DbLinkedHandle<EntityHandleDb, Entity>((int)entityHandleVal), v6);
}

// ============================================================================
// scr.o batch 34 - VM_Clear / SetupLevelSpecificVariables / objective family
// ============================================================================

// FogConfig helpers (render.o)
namespace FogConfig {
void SetRange(float n, float f);       // ?SetRange@FogConfig@@YAXMM@Z
void SetColor(float r, float g, float b);  // ?SetColor@FogConfig@@YAXMMM@Z
void SetVal(float s, float e);         // ?SetVal@FogConfig@@YAXMM@Z
}

// PakInfoNode layout subset for level audio/fog (verified vs disasm 5C20D0)
struct PakInfoNodeLocal {
    unsigned char _pad0[0x38];
    char*  audioBackgroundTrack;   // +0x38 (InplaceString.mStr)
    char*  audioReverbSetting;     // +0x3C
    char*  audioAmbientSetting;    // +0x40
    int    audioAmbientMin;        // +0x44
    int    audioAmbientMax;        // +0x48
    float  cullFog[6];             // +0x4C
    float  fog[2];                 // +0x64
    float  zfar;                   // +0x6C
    unsigned char _pad70[0x74 - 0x70];
    float  glowParamsXbox[4];      // +0x74
};

extern int Info_Validate(const char* s);  // g_info.cpp (0x611140)
extern void Info_SetValueForKey(char* s, const char* key,
                                const char* value);  // g_info.cpp (0x611170)

// Objective state table (scr.o data @ 0xF6A2B0; _objectiveInfo_t 176 bytes,
// per-client stride 3208 bytes = 802 ints)
struct objectiveInfoLocal {
    int    worldState;     // +0x00
    float  height;         // +0x04
    unsigned int entity;   // +0x08
    int    state;          // +0x0C
    float  vOrigin[3];     // +0x10
    int    ringTime;       // +0x1C
    int    ringToggle;     // +0x20
    int    displayOrder;   // +0x24
    void*  pChild;         // +0x28
    void*  pParent;        // +0x2C
    char   szString[128];  // +0x30
};
extern int dword_F6A2A0[4 * 802];  // cg_draw.cpp (per-client 802-dword block)

static unsigned char* ObjectiveBase()
{
    // objective info table @ 0xF6A2B0 (4 dwords past the DObj slot)
    return (unsigned char*)&dword_F6A2A0[0] + 0x10;
}

static objectiveInfoLocal* ObjInfo(int clientIndex, int iObjective)
{
    return (objectiveInfoLocal*)(ObjectiveBase()
                                 + 3208 * clientIndex + 176 * iObjective);
}

static void ObjectiveInfoClear(objectiveInfoLocal* p)
{
    p->worldState = 0;
    p->height = 0.0f;
    p->entity = 0;
    p->vOrigin[0] = 0.0f;
    *(int*)&p->vOrigin[1] = 0;
    p->ringTime = -1;
    p->ringToggle = 0;
    p->displayOrder = -1;
    p->pChild = nullptr;
    p->pParent = nullptr;
    p->szString[0] = 0;
}

// ea: 0x005C1DB0
void VM_Clear()
{
    memset(vmTable, 0, sizeof(vmTable));
    currentVM = nullptr;
}

// ea: 0x005C20D0
void BrocSys::SetupLevelSpecificVariables()
{
    const PakInfoNodeLocal* PakInfo =
        (const PakInfoNodeLocal*)PakManager::sInst->GetPakInfo(CurPakId());
    FogConfig::SetRange(PakInfo->cullFog[0], PakInfo->cullFog[1]);
    FogConfig::SetColor(PakInfo->cullFog[2], PakInfo->cullFog[3],
                        PakInfo->cullFog[4]);
    FogConfig::SetVal(PakInfo->fog[0], PakInfo->fog[1]);
    CVarSetFloat("r_zfar", PakInfo->zfar);
    ShaderCommon::gGlowIntensity = PakInfo->glowParamsXbox[0];
    ShaderCommon::gGlowExpansion = PakInfo->glowParamsXbox[1];
    ShaderCommon::gGlowBrighten = PakInfo->glowParamsXbox[2];
    ShaderCommon::gGlowPasses = (int)PakInfo->glowParamsXbox[3];
    void (*result)(const char*, const char*, const char*, int, int) =
        gpBrocAPI->mBrocExports.mCallbackSetLevelAudio;
    if (result != nullptr)
        result(PakInfo->audioBackgroundTrack, PakInfo->audioReverbSetting,
               PakInfo->audioAmbientSetting, PakInfo->audioAmbientMin,
               PakInfo->audioAmbientMax);
}

// ea: 0x005C5E10
void BrocSys::ObjectiveAdd3(int iObjective, const Broc::string& inState,
                            const Broc::string& pszString,
                            const char* display, int iChild,
                            int iChildOrder, int clientIndex)
{
    (void)pszString;
    (void)display;
    (void)iChild;
    (void)iChildOrder;
    if (iObjective >= 0x10)
    {
        char* v10 = va(
            "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
            iObjective, 15);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v10))  // off_CFBB58 format
            __debugbreak();
    }
    else
    {
        if (clientIndex < -1 || clientIndex >= 1)
        {
            const char* v9 = va(
                "index %i is an illegal client index. Valid indexes are -1 to %i\n",
                clientIndex, 0);
            Scr_Error(v9);
        }
        else
        {
            Broc::string state(inState);
            int iStateIndex;
            if (ObjectiveStateIndexFromString(&iStateIndex, state))
            {
                if (clientIndex >= 0)
                    *(int*)(ObjectiveBase() + 0x0C
                            + 3208 * clientIndex + 176 * iObjective) =
                        iStateIndex;
                else
                    *(int*)(ObjectiveBase() + 0x0C
                            + 176 * iObjective) = iStateIndex;
            }
        }
    }
}

// ea: 0x005C5F50
const int BrocSys::FindChildObjective(int iObjective, int iChild,
                                      bool bReportNotFound)
{
    if (iObjective > 0x10)
    {
        char* v10 = va(
            "Parent objective %i is out of range.  Range should be 0 to %i\n",
            iObjective, 16);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v10))
            __debugbreak();
        return -1;
    }
    bool bParentFound = false;
    int v4 = 32;
    do
    {
        char szConfigString[259];
        SV_GetConfigstring(v4, szConfigString, 256);
        if (szConfigString[0] != 0)
        {
            const char* v5 = Info_ValueForKey(szConfigString, "pobj");
            if (*v5 != 0)
            {
                const char* v6 = Info_ValueForKey(szConfigString, "order");
                if (atoi(v5) == (int)iObjective)
                {
                    bParentFound = true;
                    if (*v6 != 0 && atoi(v6) == iChild)
                        return v4;
                }
            }
        }
        ++v4;
    } while (v4 - 32 < 1);
    if (bReportNotFound)
    {
        const char* v8;
        if (bParentFound)
            v8 = va(
                "Parent objective %i has no child with display index %i\n",
                iObjective, iChild);
        else
            v8 = va(
                "Parent objective %i not setup/does not exist.  Cannot find it's children.\n",
                iObjective);
        Scr_Error(v8);
    }
    return -1;
}

// ea: 0x005C60B0
void BrocSys::ObjectiveChildDelete(int iObjective, int iChild)
{
    if (iChild >= 1)
    {
        int ChildObjective = FindChildObjective(iObjective, iChild, true);
        if (ChildObjective != -1)
        {
            char szConfigString[256];
            SV_GetConfigstring(ChildObjective, szConfigString, 256);
            szConfigString[0] = 0;
            SV_SetConfigstring(ChildObjective, szConfigString);
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 946;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C6150
void BrocSys::ObjectiveDeleteChildren(int iObjective)
{
    if (iObjective >= 0x10)
    {
        char* v1 = va(
            "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
            iObjective, 15);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v1))
            __debugbreak();
    }
    else
    {
        DeleteAllChildrenOfObjective(iObjective);
    }
}

// ea: 0x005C61D0
void BrocSys::ObjectiveDelete(int iObjective, int clientIndex)
{
    if (iObjective >= 0x10)
    {
        char* v4 = va(
            "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
            iObjective, 15);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v4))
            __debugbreak();
    }
    else if (clientIndex < -1 || clientIndex >= 1)
    {
        const char* v3 = va(
            "index %i is an illegal client index. Valid indexes are -1 to %i\n",
            clientIndex, 0);
        Scr_Error(v3);
    }
    else if (clientIndex >= 0)
    {
        ObjectiveInfoClear(ObjInfo(clientIndex, iObjective));
    }
    else
    {
        unsigned char* v2 = ObjectiveBase() + 176 * iObjective;
        *(int*)(v2 + 48) = 0;
        *(int*)(v2 + 8) = 0;
        *(int*)(v2 + 10) = 0;
        *(int*)(v2 + 11) = 0;
        *(int*)(v2 + 20) = 0;
        *(int*)(v2 + 4) = 0;
        *(int*)(v2 + 7) = -1;
        *(int*)(v2 + 9) = -1;
        *(int*)(v2 + 2) = 0;
        *(int*)(v2 + 0) = 0;
        *(int*)(v2 + 1) = 0;
    }
}

// ea: 0x005C62C0
void BrocSys::ObjectiveChildState(int iObjective, int iChild,
                                  const Broc::string& inState,
                                  const char* pDisplay)
{
    Broc::string state((Broc::string::Block*)nullptr);
    if (iChild >= 1)
    {
        state = inState;
        const char* v4 = state.mBlock != nullptr
                             ? (const char*)(state.mBlock + 1)
                             : defaultFileName;
        int v5 = IGOCompassWidget::ObjectiveStateIndexFromString(v4);
        if (v5 == 27)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
            AeAssert::gCurrentLine = 1060;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Illegal objective state"))
                __debugbreak();
        }
        else
        {
            int ChildObjective = FindChildObjective(iObjective, iChild, true);
            if (ChildObjective != -1)
            {
                char szConfigString[256];
                SV_GetConfigstring(ChildObjective, szConfigString, 256);
                const char* v8 = va("%i", v5);
                Info_SetValueForKey(szConfigString, "state", v8);
                SV_SetConfigstring(ChildObjective, szConfigString);
                if (pDisplay == nullptr || *pDisplay != 48)
                    Info_ValueForKey(szConfigString, "str");
            }
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 1053;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C6460
void BrocSys::ObjectiveState(int iObjective, const Broc::string& inState,
                             const char* pDisplay, int clientIndex)
{
    (void)pDisplay;
    Broc::string state((Broc::string::Block*)nullptr);
    if (iObjective >= 0x10)
    {
        char* v10 = va(
            "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
            iObjective, 15);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v10))
            __debugbreak();
    }
    else
    {
        if (clientIndex < -1 || clientIndex >= 1)
        {
            const char* v9 = va(
                "index %i is an illegal client index. Valid indexes are -1 to %i\n",
                clientIndex, 0);
            Scr_Error(v9);
        }
        else
        {
            state = inState;
            int iStateIndex;
            if (ObjectiveStateIndexFromString(&iStateIndex, state))
            {
                int* pState;
                if (clientIndex >= 0)
                    pState = (int*)(ObjectiveBase() + 0x0C
                                    + 3208 * clientIndex + 176 * iObjective);
                else
                    pState = (int*)(ObjectiveBase() + 0x0C
                                    + 176 * iObjective);
                if (state[0] != 0)
                    *pState = iStateIndex;
                else
                    *pState = 0;
            }
            else
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\BrocObjective.cpp";
                AeAssert::gCurrentLine = 1115;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Illegal objective state"))
                    __debugbreak();
            }
        }
    }
}

// ea: 0x005C6620
void BrocSys::ObjectiveStringInternal(int iObjective,
                                      const Broc::string& text, int number,
                                      bool bMakeUpdateMessage, bool bChild,
                                      const char* pDisplay)
{
    (void)bMakeUpdateMessage;
    (void)pDisplay;
    Broc::string state((Broc::string::Block*)nullptr);
    if ((bChild || iObjective >= 0) && iObjective < 16)
    {
        char szConfigString[256];
        char szString[256];
        char tmp[16];
        SV_GetConfigstring(iObjective + 16, szConfigString, 256);
        tmp[0] = 0;
        snprintf(tmp, 0xFu, "%d", number);
        if (text.mBlock)
            sprintf(szString, (const char*)(text.mBlock + 1), tmp);
        else
            sprintf(szString, defaultFileName, tmp);
        if (strlen(szString) < 256)
        {
            if (!Info_Validate(szString) || strchr(szString, 92))
            {
                const char* v9 = va(
                    "Objective strings can not have a \", a ;, or a \\ in them. Illegal objective string: %s\n",
                    szString);
                Scr_Error(v9);
            }
            else
            {
                Info_SetValueForKey(szConfigString, "str", szString);
                SV_SetConfigstring(iObjective + 16, szConfigString);
                const char* v7 = Info_ValueForKey(szConfigString, "state");
                int v8 = *v7 != 0 ? atoi(v7) : 0;
                switch (v8)
                {
                case 0:
                    state = str_const.empty;
                    break;
                case 1:
                    state = str_const.active;
                    break;
                case 2:
                    state = str_const.invisible;
                    break;
                case 3:
                    state = str_const.done;
                    break;
                case 4:
                    state = str_const.current;
                    break;
                case 5:
                    state = str_const.failed;
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            const char* v6 = va(
                "Objective strings is too long (> %i): %s\n", 255, szString);
            Scr_Error(v6);
        }
    }
    else
    {
        char* v5 = va(
            "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
            iObjective, 15);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v5))
            __debugbreak();
    }
}

// ea: 0x005C6890
void BrocSys::ObjectiveString(int iObjective, const Broc::string& text,
                              int number, const char* pDisplay)
{
    ObjectiveStringInternal(iObjective, text, number, true,
                            pDisplay != nullptr, "1");
}

// ea: 0x005C68C0
void BrocSys::ObjectiveString2(int iObjective, int text, int number,
                               const char* pDisplay)
{
    Broc::string result = GetLocalizedString(text);
    ObjectiveStringInternal(iObjective, result, number, true,
                            pDisplay != nullptr, "1");
}

// ea: 0x005C6930
void BrocSys::ObjectiveString_NoMessage(int iObjective,
                                        const Broc::string& text, int number,
                                        const char* pDisplay)
{
    ObjectiveStringInternal(iObjective, text, number, false,
                            pDisplay != nullptr, "1");
}

// ea: 0x005C6960
void BrocSys::ObjectivePosition(int iObjective, const Broc::vector& vPos,
                                int clientIndex)
{
    if (clientIndex < -1 || clientIndex >= 1)
    {
        char* v4 = va(
            "index %i is an illegal client index. Valid indexes are -1 to %i\n",
            clientIndex, 0);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v4))
            __debugbreak();
        return;
    }
    if (iObjective >= 0x10)
    {
        char tmpstr[256];
        sprintf(tmpstr,
                "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
                iObjective, 15);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 1312;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(tmpstr))
            __debugbreak();
        return;
    }
    objectiveInfoLocal* p =
        ObjInfo(clientIndex >= 0 ? clientIndex : 0, iObjective);
    p->vOrigin[0] = vPos.x;
    p->vOrigin[1] = vPos.y;
    p->vOrigin[2] = vPos.z;
}

// ea: 0x005C6A80
void BrocSys::ObjectiveWorldState(int iObjective,
                                  const Broc::string& inState,
                                  int clientIndex)
{
    if (clientIndex < -1 || clientIndex >= 1)
    {
        char* v6 = va(
            "index %i is an illegal client index. Valid indexes are -1 to %i\n",
            clientIndex, 0);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v6))
            __debugbreak();
        return;
    }
    if (iObjective >= 0x10)
    {
        char tmpstr[256];
        sprintf(tmpstr,
                "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
                iObjective, 15);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 1355;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(tmpstr))
            __debugbreak();
        return;
    }
    Broc::string state(inState);
    int iStateIndex;
    if (!ObjectiveStateIndexFromString(&iStateIndex, state))
        return;
    objectiveInfoLocal* p = ObjInfo(0, iObjective);
    if (clientIndex >= 0)
    {
        p = ObjInfo(clientIndex, iObjective);
        if (state[0] == 0)
        {
            p->worldState = 0;
            return;
        }
        p->worldState = iStateIndex;
    }
    else
    {
        if (state.mBlock != nullptr && state.mBlock->mLength != 0
            && *(char*)(state.mBlock + 1) != 0)
            p->worldState = iStateIndex;
        else
            p->worldState = 0;
    }
}

// ea: 0x005C6C90
void BrocSys::ObjectiveChildCurrent(int iObjective, int iChild,
                                    const char* pDisplay)
{
    (void)pDisplay;
    if (iChild >= 1)
    {
        if (iObjective >= 0x10)
        {
            const char* v4 = va(
                "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
                iObjective, 15);
            Scr_Error(v4);
        }
        else
        {
            int ChildObjective = FindChildObjective(iObjective, iChild, true);
            if (ChildObjective != -1)
            {
                char szConfigString[256];
                SV_GetConfigstring(ChildObjective, szConfigString, 256);
                const char* v3 = va("%i", 4);
                Info_SetValueForKey(szConfigString, "state", v3);
                SV_SetConfigstring(ChildObjective, szConfigString);
                Broc::string s((Broc::string::Block*)nullptr);
                s = str_const.current;
            }
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 1404;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C6DE0
void BrocSys::ObjectiveCurrent(int iObjective, const char* pDisplay)
{
    if (iObjective >= 0x10)
    {
        char* v8 = va(
            "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
            iObjective, 15);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v8))
            __debugbreak();
    }
    else
    {
        for (int v3 = 0; v3 < 16; ++v3)
        {
            char szConfigString[256];
            SV_GetConfigstring(v3 + 16, szConfigString, 256);
            const char* v4 = Info_ValueForKey(szConfigString, "state");
            int v5 = *v4 != 0 ? atoi(v4) : 0;
            if (v3 == iObjective)
            {
                if (v5 != 4)
                {
                    const char* v6 = va("%i", 4);
                    Info_SetValueForKey(szConfigString, "state", v6);
                }
                SV_SetConfigstring(iObjective + 16, szConfigString);
            }
            else if (v5 == 4)
            {
                const char* v7 = va("%i", 1);
                Info_SetValueForKey(szConfigString, "state", v7);
                SV_SetConfigstring(v3 + 16, szConfigString);
            }
        }
        if (pDisplay == nullptr || *pDisplay != 48)
        {
            Broc::string s((Broc::string::Block*)nullptr);
            s = str_const.current;
        }
    }
}

// ============================================================================
// scr.o batch 40 - objective children/rings/add family
// ============================================================================

int dword_F6A2D0[740];  // 0xF6A2D0 (objective ring flags; scr.o data)

// ea: 0x005C6F90
void BrocSys::ObjectiveRing(int iObjective, int clientIndex)
{
    if (iObjective >= 0x10)
    {
        char* v3 = va(
            "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
            iObjective, 15);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", v3))
            __debugbreak();
    }
    else if (clientIndex < -1 || clientIndex >= 1)
    {
        const char* v2 = va(
            "index %i is an illegal client index. Valid indexes are -1 to %i\n",
            clientIndex, 0);
        Scr_Error(v2);
    }
    else if (clientIndex >= 0)
    {
        dword_F6A2D0[802 * clientIndex + 44 * iObjective] =
            dword_F6A2D0[802 * clientIndex + 44 * iObjective] == 0;
    }
    else
    {
        dword_F6A2D0[44 * iObjective] =
            dword_F6A2D0[44 * iObjective] == 0;
    }
}

// ea: 0x005C7060
void BrocSys::ObjectiveChildString(int iObjective, int iChild,
                                   const Broc::string& text, int number,
                                   const char* pDisplay)
{
    if (iChild >= 1)
    {
        int ChildObjective = FindChildObjective(iObjective, iChild, true);
        if (ChildObjective != -1)
            ObjectiveStringInternal(ChildObjective, text, number, true, true,
                                    pDisplay);
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 1570;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C70E0
void BrocSys::ObjectiveChildString2(int iObjective, int iChild, int text,
                                    int number, const char* pDisplay)
{
    (void)pDisplay;
    if (iChild >= 1)
    {
        int ChildObjective = FindChildObjective(iObjective, iChild, true);
        if (ChildObjective != -1)
        {
            Broc::string result = GetLocalizedString(text);
            Broc::string child((Broc::string::Block*)nullptr);
            if (ChildObjective < 16)
            {
                int v7 = ChildObjective + 16;
                char buffer[256];
                char s[256];
                char string[16];
                SV_GetConfigstring(v7, buffer, 256);
                string[0] = 0;
                snprintf(string, 0xFu, "%d", number);
                if (result.mBlock != nullptr)
                    sprintf(s, (const char*)(result.mBlock + 1), string);
                else
                    sprintf(s, defaultFileName, string);
                if (strlen(s) < 256)
                {
                    if (!Info_Validate(s) || strchr(s, 92) != nullptr)
                    {
                        const char* v11 = va(
                            "Objective strings can not have a \", a ;, or a \\ in them. Illegal objective string: %s\n",
                            s);
                        Scr_Error(v11);
                    }
                    else
                    {
                        Info_SetValueForKey(buffer, "str", s);
                        SV_SetConfigstring(v7, buffer);
                        const char* v9 =
                            Info_ValueForKey(buffer, "state");
                        int v10 = *v9 != 0 ? atoi(v9) : 0;
                        switch (v10)
                        {
                        case 0:
                            child = str_const.empty;
                            break;
                        case 1:
                            child = str_const.active;
                            break;
                        case 2:
                            child = str_const.invisible;
                            break;
                        case 3:
                            child = str_const.done;
                            break;
                        case 4:
                            child = str_const.current;
                            break;
                        case 5:
                            child = str_const.failed;
                            break;
                        default:
                            break;
                        }
                    }
                }
                else
                {
                    const char* v8 = va(
                        "Objective strings is too long (> %i): %s\n", 255, s);
                    Scr_Error(v8);
                }
            }
            else
            {
                char* v6 = va(
                    "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
                    ChildObjective, 15);
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
                AeAssert::gCurrentLine = 16;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning("\x15%s", v6))
                    __debugbreak();
            }
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 1588;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C73D0
void BrocSys::ObjectiveChildString_NoMessage(int iObjective, int iChild,
                                             const Broc::string& text,
                                             int number,
                                             const char* pDisplay)
{
    if (iChild >= 1)
    {
        int ChildObjective = FindChildObjective(iObjective, iChild, true);
        if (ChildObjective != -1)
            ObjectiveStringInternal(ChildObjective, text, number, false, true,
                                    pDisplay);
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 1607;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C7450
void BrocSys::ObjectiveChildPosition(int iObjective, int iChild,
                                     const Broc::vector& vPos)
{
    if (iChild >= 1)
    {
        int ChildObjective = FindChildObjective(iObjective, iChild, true);
        int v4 = ChildObjective;
        if (ChildObjective != -1)
        {
            char szConfigString[256];
            SV_GetConfigstring(ChildObjective, szConfigString, 256);
            const char* v5 = va("%i %i %i", vPos.x, vPos.y, vPos.z);
            Info_SetValueForKey(szConfigString, "org", v5);
            SV_SetConfigstring(v4, szConfigString);
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 1625;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C7520
void BrocSys::ObjectiveChildRing(int iObjective, int iChild)
{
    if (iChild >= 1)
    {
        int ChildObjective = FindChildObjective(iObjective, iChild, true);
        int v3 = ChildObjective;
        if (ChildObjective != -1)
        {
            char szConfigString[256];
            SV_GetConfigstring(ChildObjective, szConfigString, 256);
            const char* v4 = Info_ValueForKey(szConfigString, "ring");
            int v5 = *v4 != 0 ? atoi(v4) : 0;
            const char* v6 = va("%i", v5 == 0);
            Info_SetValueForKey(szConfigString, "ring", v6);
            SV_SetConfigstring(v3, szConfigString);
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 1650;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C8830
void BrocSys::ObjectiveChildAdd1(int iObjective, int iChild,
                                 const Broc::string& state)
{
    int v3 = iChild;
    if (iChild >= 1)
    {
        int UnusedChildObjective = FindUnusedChildObjective();
        if (UnusedChildObjective == -1)
        {
            Com_Printf("^1ERROR : No free child objectives!\n");
        }
        else if (FindChildObjective(iObjective, v3, false) == -1)
        {
            Broc::string s((Broc::string::Block*)nullptr);
            ObjectiveAdd3(iObjective, state, s, defaultFileName,
                          UnusedChildObjective, v3, -1);
        }
        else
        {
            Com_Printf(
                "^1ERROR : Attempt to add an existant child objective number to a parent objective!\n");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 197;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C8950
void BrocSys::ObjectiveAdd1(int iObjective, const Broc::string& state,
                            const char* display, int iChild,
                            int iChildOrder)
{
    (void)display;
    (void)iChild;
    (void)iChildOrder;
    Broc::string s((Broc::string::Block*)nullptr);
    ObjectiveAdd3(iObjective, state, s, "1", -1, -1, -1);
}

// ea: 0x005C89C0
void BrocSys::ObjectiveChildAdd2(int iObjective, int iChild,
                                 const Broc::string& state, int iString,
                                 const char* display)
{
    int v5 = iChild;
    if (iChild >= 1)
    {
        int child = FindUnusedChildObjective();
        if (child == -1)
        {
            Com_Printf("^1ERROR : No free child objectives!\n");
        }
        else if (FindChildObjective(iObjective, v5, false) == -1)
        {
            char szString[1024];
            sprintf(szString, "%d", iString);
            size_t v6 = strlen(szString);
            if (v6 > 1)
            {
                for (size_t i = 0; i < v6; ++i)
                {
                    if (isalnum((unsigned char)szString[i]) == 0
                        && szString[i] != 95)
                    {
                        const char* v8 = va(
                            "Illegal localized string reference: %s (must contain only alpha-numeric characters and underscores",
                            szString);
                        Scr_ParamError(2u, v8);
                    }
                }
            }
            Broc::string s(szString);
            ObjectiveAdd3(iObjective, state, s, display, child, v5, -1);
        }
        else
        {
            Com_Printf(
                "^1ERROR : Attempt to add an existant child objective number to a parent objective!\n");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 230;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C8B70
void BrocSys::ObjectiveAdd2(int iObjective, const Broc::string& state,
                            int iString, const char* display, int iChild,
                            int iChildOrder)
{
    (void)display;
    (void)iChild;
    (void)iChildOrder;
    char szString[1024];
    sprintf(szString, "%d", iString);
    size_t v3 = strlen(szString);
    if (v3 > 1)
    {
        for (size_t i = 0; i < v3; ++i)
        {
            if (isalnum((unsigned char)szString[i]) == 0
                && szString[i] != 95)
            {
                char* v5 = va(
                    "Illegal localized string reference: %s (must contain only alpha-numeric characters and underscores",
                    szString);
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
                AeAssert::gCurrentLine = 23;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning("\x15%s", v5))
                    __debugbreak();
            }
        }
    }
    Broc::string pszString(szString);
    ObjectiveAdd3(iObjective, state, pszString, "1", -1, -1, -1);
}

// ea: 0x005C8CA0
void BrocSys::ObjectiveChildAdd6(int iObjective, int iChild,
                                 const Broc::string& state, int pszString,
                                 const char* display)
{
    int v5 = iChild;
    if (iChild >= 1)
    {
        int UnusedChildObjective = FindUnusedChildObjective();
        if (UnusedChildObjective == -1)
        {
            Com_Printf("^1ERROR : No free child objectives!\n");
        }
        else if (FindChildObjective(iObjective, v5, false) == -1)
        {
            Broc::string s = GetLocalizedString(pszString);
            ObjectiveAdd3(iObjective, state, s, display, UnusedChildObjective,
                          v5, -1);
        }
        else
        {
            Com_Printf(
                "^1ERROR : Attempt to add an existant child objective number to a parent objective!\n");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 299;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ea: 0x005C8DC0
void BrocSys::ObjectiveAdd6(int iObjective, const Broc::string& inState,
                            int pszString, const char* display, int iChild,
                            int iChildOrder)
{
    (void)display;
    (void)iChild;
    (void)iChildOrder;
    Broc::string result = GetLocalizedString(pszString);
    ObjectiveAdd3(iObjective, inState, result, "1", -1, -1, -1);
}

// ea: 0x005C8E30
void BrocSys::ObjectiveChildAdd3(int iObjective, int iChild,
                                 const Broc::string& state,
                                 const Broc::string& pszString,
                                 const char* display)
{
    if (iChild >= 1)
    {
        int UnusedChildObjective = FindUnusedChildObjective();
        if (UnusedChildObjective == -1)
        {
            Com_Printf("^1ERROR : No free child objectives!\n");
        }
        else if (FindChildObjective(iObjective, iChild, false) == -1)
        {
            ObjectiveAdd3(iObjective, state, pszString, display,
                          UnusedChildObjective, iChild, -1);
        }
        else
        {
            Com_Printf(
                "^1ERROR : Attempt to add an existant child objective number to a parent objective!\n");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocObjective.cpp";
        AeAssert::gCurrentLine = 333;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "Child Objective numbers must be greater than 0."))
            __debugbreak();
    }
}

// ============================================================================
// scr.o batch 35 - BrocDebugRender / MPScript family / GetJoyPos
// ============================================================================

// controller view (input/controller.cpp; binary overloads per IDA mangles)
class controller {
public:
    enum ButtonIndex {
        LEFTBUTTON = 0,
        DOWNBUTTON = 1,
        RIGHTBUTTON = 2,
        UPBUTTON = 3,
        SQUARE = 4,
        CIRCLE = 5,
        TRIANGLE = 7,
        R1 = 8,
        L1 = 9,
        R2 = 10,
        L2 = 11,
        R3 = 12,
        SELECT = 15,
    };
    enum StickIndex {
        LEFTSTICK = 0,
        RIGHTSTICK = 1,
    };
    static controller* inst();  // ?inst@controller@@SAPAV1@XZ
    bool is_locked;  // +0x1C
    void stick_value(StickIndex i_stick, int* o_x, int* o_y,
                     int* p_controller);  // ?stick_value@controller@@QAEXW4StickIndex@1@PAH11@Z
    void stick_value(int i_controller_num, StickIndex i_Stick, int* o_x,
                     int* o_y);  // ?stick_value@controller@@QAEXHW4StickIndex@1@PAH1@Z
    int button_value(int i_controller_num,
                     ButtonIndex i_button);  // ?button_value@controller@@QAEHHW4ButtonIndex@1@@Z
    bool button_pressed_clear(int index,
                              ButtonIndex btn);  // controller.o
    bool button_released_clear(int index,
                               ButtonIndex btn);  // controller.o
};

extern int dword_F6A28C[4 * 802];  // 0xF6A28C (controller-port table)
extern bool Com_ControllerValid(int controller_port);  // common.cpp

// ea: 0x005BDEA0
void BrocSys::BrocDebugRender()
{
    if (gBrocPool != nullptr && gBrocPool->IsEmpty())
    {
        Color col;
        col.r = 1.0f;
        col.g = 0.0f;
        col.b = 0.0f;
        col.a = 1.0f;
        DebugRender::RenderText("Broc Pool empty", 20, 20, col, 0.0f, 1.0f);
    }
}

// ea: 0x005BDF00
void* BrocSys::CreateExtendedEntity(const char** keys, int count)
{
    (void)keys;
    (void)count;
    if (gpBrocAPI != nullptr)
        return gpBrocAPI->mBrocExports.mCreateExtendedEntity(nullptr, 0);
    else
        return nullptr;
}

// ea: 0x005BDFB0
bool BrocSys::RecompileScript()
{
    return true;
}

// ea: 0x005BE040
void BrocHelper::AnimationToBroLookup(const char* tree_name, int tree_index,
                                      unsigned int animation_name_brohashed,
                                      int animation_index)
{
    const char* v4 = _strlwr((char*)tree_name);
    int v5 = HashString::CalcHash(v4);
    if (gpBrocAPI->mBrocExports.mAnimIndexResolver == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 5350;
        AeAssert::gCurrentExpr = "gpBrocAPI->mBrocExports.mAnimIndexResolver";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    gpBrocAPI->mBrocExports.mAnimIndexResolver(
        v5, tree_index, animation_name_brohashed, animation_index);
}

// ea: 0x005BE100
void BrocHelper::AnimationValidator(int numTrees)
{
    BrocHelper::m_treeCount = numTrees;
    if (gpBrocAPI->mBrocExports.mAnimIndexValidate == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 5392;
        AeAssert::gCurrentExpr = "gpBrocAPI->mBrocExports.mAnimIndexValidate";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    if (!gpBrocAPI->mBrocExports.mAnimIndexValidate())
        Com_Printf("===Not all animations were fixed up!\n");
}

// ea: 0x005BF6A0
void BrocSys::GetJoyPos(int stickIndex, float& xPos, float& yPos)
{
    xPos = 0.0f;
    yPos = 0.0f;
    if (controller::inst()->is_locked)
    {
        controller::inst()->stick_value((controller::StickIndex)stickIndex,
                                        (int*)&xPos, (int*)&yPos, nullptr);
    }
    else
    {
        int v7 = dword_F6A28C[802 * currCl];
        controller::inst()->stick_value(v7,
                                        (controller::StickIndex)stickIndex,
                                        (int*)&xPos, (int*)&yPos);
    }
    xPos *= 0.0078740157f;
    yPos *= 0.0078740157f;
}

// ea: 0x005C0B80
void BrocSys::MPScript_ClearPlayerStats()
{
    for (int i = 0; i < 16; ++i)
    {
        Entity* v1 = EntityManager::sInst->mPlayers[i];
        if (v1 != nullptr)
        {
            Client* client = v1->client;
            if (client != nullptr)
            {
                memset(&client->pers, 0, 0x194u);
                client->pers.mStats[6][28] = 0;
                client->pers.mBaseScore = 0;
                client->pers.rank = 0;
                client->ps.ctf_has_flag = 0;
            }
        }
    }
}

// ea: 0x005C0C40
void BrocSys::MPScript_IncTeamScore(const Broc::string& team, int amount)
{
    if (team == "allies")
    {
        cgGlobal.teamScores[2] += amount;
    }
    else if (team == "axis")
    {
        cgGlobal.teamScores[1] += amount;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityClient.cpp";
        AeAssert::gCurrentLine = 421;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("Team must be either axis or allies"))
            __debugbreak();
    }
}

// ea: 0x005C0CD0
int BrocSys::MPScript_GetTeamScore(const Broc::string& team)
{
    if (team == "allies")
        return cgGlobal.teamScores[2];
    if (team == "axis")
        return cgGlobal.teamScores[1];
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityClient.cpp";
    AeAssert::gCurrentLine = 437;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error("Team must be either axis or allies"))
        __debugbreak();
    return 0;
}

// ea: 0x005C0D50
void BrocSys::MPScript_SendGameScore(int alliesScore, int axisScore)
{
    MultiplayerMgr::sInst->SendGameScore(alliesScore, axisScore);
}

// ea: 0x005C0D70
void BrocSys::MPScript_HostDropItem1(int itemType, int netID,
                                     const Broc::vector& pos,
                                     const Broc::vector& angle)
{
    if (MultiplayerMgr::sInst->IsHost())
    {
        math::Dir3 v6;  // zero velocity
        v6.v = _mm_setzero_ps();
        math::Position3 v7;
        v7.v.m128_f32[0] = angle.x;
        v7.v.m128_f32[1] = angle.y;
        v7.v.m128_f32[2] = angle.z;
        v7.v.m128_f32[3] = 0.0f;
        math::Dir3 v5;
        v5.v = v7.v;
        v7.v.m128_f32[0] = pos.x;
        v7.v.m128_f32[1] = pos.y;
        v7.v.m128_f32[2] = pos.z;
        v7.v.m128_f32[3] = 0.0f;
        MultiplayerMgr::sInst->DropItem(itemType, &v7, &v5, &v6, netID,
                                        true, -1);
    }
}

// ea: 0x005C0E40
void BrocSys::MPScript_HostDropItem2(int itemType, int netID,
                                     const Broc::vector& pos,
                                     const Broc::vector& angle,
                                     const Broc::vector& vel)
{
    if (MultiplayerMgr::sInst->IsHost())
    {
        math::Position3 v8;
        v8.v.m128_f32[0] = vel.x;
        v8.v.m128_f32[1] = vel.y;
        v8.v.m128_f32[2] = vel.z;
        v8.v.m128_f32[3] = 0.0f;
        math::Dir3 v7;
        v7.v = v8.v;
        v8.v.m128_f32[0] = angle.x;
        v8.v.m128_f32[1] = angle.y;
        v8.v.m128_f32[2] = angle.z;
        v8.v.m128_f32[3] = 0.0f;
        math::Dir3 v6;
        v6.v = v8.v;
        v8.v.m128_f32[0] = pos.x;
        v8.v.m128_f32[1] = pos.y;
        v8.v.m128_f32[2] = pos.z;
        v8.v.m128_f32[3] = 0.0f;
        MultiplayerMgr::sInst->DropItem(itemType, &v8, &v6, &v7, netID,
                                        true, -1);
    }
}

// ea: 0x005C0F30
void BrocSys::MPScript_DropItem1(int itemType, int netID,
                                 const Broc::vector& pos,
                                 const Broc::vector& angle)
{
    math::Dir3 v6;
    v6.v = _mm_setzero_ps();
    math::Position3 v7;
    v7.v.m128_f32[0] = angle.x;
    v7.v.m128_f32[1] = angle.y;
    v7.v.m128_f32[2] = angle.z;
    v7.v.m128_f32[3] = 0.0f;
    math::Dir3 v5;
    v5.v = v7.v;
    v7.v.m128_f32[0] = pos.x;
    v7.v.m128_f32[1] = pos.y;
    v7.v.m128_f32[2] = pos.z;
    v7.v.m128_f32[3] = 0.0f;
    MultiplayerMgr::sInst->DropItem(itemType, &v7, &v5, &v6, netID, true, -1);
}

// ea: 0x005C0FE0
void BrocSys::MPScript_DropItem2(int itemType, int netID,
                                 const Broc::vector& pos,
                                 const Broc::vector& angle,
                                 const Broc::vector& vel)
{
    math::Position3 v8;
    v8.v.m128_f32[0] = vel.x;
    v8.v.m128_f32[1] = vel.y;
    v8.v.m128_f32[2] = vel.z;
    v8.v.m128_f32[3] = 0.0f;
    math::Dir3 v7;
    v7.v = v8.v;
    v8.v.m128_f32[0] = angle.x;
    v8.v.m128_f32[1] = angle.y;
    v8.v.m128_f32[2] = angle.z;
    v8.v.m128_f32[3] = 0.0f;
    math::Dir3 v6;
    v6.v = v8.v;
    v8.v.m128_f32[0] = pos.x;
    v8.v.m128_f32[1] = pos.y;
    v8.v.m128_f32[2] = pos.z;
    v8.v.m128_f32[3] = 0.0f;
    MultiplayerMgr::sInst->DropItem(itemType, &v8, &v6, &v7, netID, true, -1);
}

// ea: 0x005C10C0
void BrocSys::MPScript_AreaCaptured(int netID, unsigned int team,
                                    unsigned int hostOnly)
{
    MultiplayerMgr::sInst->AreaCaptured(netID, team, hostOnly);
}

// ea: 0x005C10E0
void BrocSys::MPScript_EnterGame()
{
    MultiplayerMgr::sInst->EnterGame();
}

// ea: 0x005C1160
bool BrocSys::MPScript_PositionWouldTelefrag(const Broc::vector& position)
{
    math::Position3 v3;
    v3.v.m128_f32[0] = position.x;
    v3.v.m128_f32[1] = position.y;
    v3.v.m128_f32[2] = position.z;
    v3.v.m128_f32[3] = 0.0f;
    return SpotWouldTelefrag(v3);
}

// ea: 0x005C11D0
int BrocSys::MPScript_SpawnButtonPressed(unsigned int clientIdx)
{
    int v1 = dword_F6A28C[802 * clientIdx];
    if (!Com_ControllerValid(v1))
        return 0;
    return controller::inst()->button_value(
               v1, (controller::ButtonIndex)(controller::SQUARE
                                             | controller::DOWNBUTTON))
           > 128 ? 1 : 0;
}

// ============================================================================
// scr.o batch 36 - MPScript debug-render + menu family
// ============================================================================

// SpectateMenu / PauseMenu (shell.o views)
class SpectateMenu {
public:
    void UpdateState();          // ?UpdateState@SpectateMenu@@QAEXXZ
    void UpdateSeconds();        // ?UpdateSeconds@SpectateMenu@@QAEXXZ
    void SetMedic(bool medic);   // ?SetMedic@SpectateMenu@@QAEX_N@Z
    void Clear();                // ?Clear@SpectateMenu@@QAEXXZ
};
class PauseMenu {
public:
    void UnPause();              // ?UnPause@PauseMenu@@QAEXXZ
};

// ea: 0x005C1220
void BrocSys::MPScript_DebugRenderText(const char* text, int x, int y)
{
    Color col;
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 1.0f;
    col.a = 1.0f;
    DebugRender::RenderText(text, x, y, col, 0.5f, 1.0f);
}

// ea: 0x005C1270
void BrocSys::MPScript_DebugRenderBox(const Broc::vector& min,
                                      const Broc::vector& max,
                                      const Broc::vector& color,
                                      float alpha)
{
    math::Position3 vMin;
    vMin.v.m128_f32[0] = min.x;
    vMin.v.m128_f32[1] = min.y;
    vMin.v.m128_f32[2] = min.z;
    vMin.v.m128_f32[3] = 0.0f;
    math::Position3 vMax;
    vMax.v.m128_f32[0] = max.x;
    vMax.v.m128_f32[1] = max.y;
    vMax.v.m128_f32[2] = max.z;
    vMax.v.m128_f32[3] = 0.0f;
    Color col;
    col.r = color.x;
    col.g = color.y;
    col.b = color.z;
    col.a = alpha;
    DebugRender::RenderBox(vMin, vMax, col);
}

// ea: 0x005C1330
void BrocSys::MPScript_DebugRenderSphere(const Broc::vector& point,
                                         float radius,
                                         const Broc::vector& color,
                                         float alpha)
{
    math::Position3 vPoint;
    vPoint.v.m128_f32[0] = point.x;
    vPoint.v.m128_f32[1] = point.y;
    vPoint.v.m128_f32[2] = point.z;
    vPoint.v.m128_f32[3] = 0.0f;
    Color col;
    col.r = color.x;
    col.g = color.y;
    col.b = color.z;
    col.a = alpha;
    DebugRender::RenderSphere(vPoint, radius, col);
}

// ea: 0x005C1430
int BrocSys::IsMenuOpen(const Broc::string& str, int viewport)
{
    const char* v2 = str.mBlock != nullptr
                         ? (const char*)(str.mBlock + 1)
                         : defaultFileName;
    if (_stricmp(v2, "spectate") == 0)
    {
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(viewport);
        return IGMS->IsMenuActive(12);
    }
    const char* v5 = str.mBlock != nullptr
                         ? (const char*)(str.mBlock + 1)
                         : defaultFileName;
    if (_stricmp(v5, defaultFileName) == 0)
    {
        InGameMenuSystem* v6 = g_femanager.GetIGMS(viewport);
        return v6->IsMenuActive(-1);
    }
    return 0;
}

// ea: 0x005C14C0
int BrocSys::OpenMenu(const Broc::string& str, int viewport)
{
    int v2 = dword_F6A28C[802 * viewport];
    controller* v3 = controller::inst();
    for (controller::ButtonIndex i = controller::LEFTBUTTON; i < (controller::ButtonIndex)16;
         i = (controller::ButtonIndex)((int)i + 1))
        v3->button_pressed_clear(v2, i);
    int v5 = dword_F6A28C[802 * viewport];
    controller* v6 = controller::inst();
    for (controller::ButtonIndex j = controller::LEFTBUTTON;
         j <= controller::SELECT;
         j = (controller::ButtonIndex)((int)j + 1))
        v6->button_released_clear(v5, j);
    const char* v8 = str.mBlock != nullptr
                         ? (const char*)(str.mBlock + 1)
                         : defaultFileName;
    if (_stricmp(v8, "weapon") == 0)
    {
        int v9 = viewport;
        *(int*)((char*)g_femanager.GetIGMS(viewport)->menus[1] + 272) = 0;
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(v9);
        IGMS->ActivateMenu(1);
        return 0;
    }
    const char* v10 = str.mBlock != nullptr
                          ? (const char*)(str.mBlock + 1)
                          : defaultFileName;
    if (_stricmp(v10, "side_select") == 0)
    {
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(viewport);
        IGMS->ActivateMenu(11);
        return 0;
    }
    const char* v12 = str.mBlock != nullptr
                          ? (const char*)(str.mBlock + 1)
                          : defaultFileName;
    if (_stricmp(v12, "spectate") == 0)
    {
        InGameMenuSystem* v13 = g_femanager.GetIGMS(viewport);
        if (!v13->IsMenuActive(12))
        {
            InGameMenuSystem* v14 = g_femanager.GetIGMS(viewport);
            ((SpectateMenu*)v14->menus[12])->Clear();
            v14->ActivateMenu(12);
        }
    }
    return 0;
}

// ea: 0x005C15F0
int BrocSys::OpenMenuNoMouse(unsigned int i, const Broc::string& str)
{
    (void)i;
    (void)str;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityClient.cpp";
    AeAssert::gCurrentLine = 1513;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("ma dead code"))
        __debugbreak();
    return 0;
}

// ea: 0x005C1640
void BrocSys::CloseMenu2(const Broc::string& str, int viewport)
{
    const char* v2 = str.mBlock != nullptr
                         ? (const char*)(str.mBlock + 1)
                         : defaultFileName;
    if (_stricmp(v2, "weapon") == 0)
    {
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(viewport);
        ((PauseMenu*)IGMS->menus[0])->UnPause();
        return;
    }
    const char* v4 = str.mBlock != nullptr
                         ? (const char*)(str.mBlock + 1)
                         : defaultFileName;
    if (_stricmp(v4, "side_select") == 0)
    {
        InGameMenuSystem* v5 = g_femanager.GetIGMS(viewport);
        ((PauseMenu*)v5->menus[0])->UnPause();
        return;
    }
    const char* v6 = str.mBlock != nullptr
                         ? (const char*)(str.mBlock + 1)
                         : defaultFileName;
    if (_stricmp(v6, "spectate") == 0)
    {
        InGameMenuSystem* v7 = g_femanager.GetIGMS(viewport);
        if (v7->IsMenuActive(12))
        {
            InGameMenuSystem* v8 = g_femanager.GetIGMS(viewport);
            v8->MakeActive(-1);
            InGameMenuSystem* v5 = g_femanager.GetIGMS(viewport);
            ((PauseMenu*)v5->menus[0])->UnPause();
            return;
        }
        InGameMenuSystem* v9 = g_femanager.GetIGMS(viewport);
        v9->ClearReturnMenu(12);
    }
}

// ea: 0x005C1730
void BrocSys::CloseAllMenus(int viewport)
{
    InGameMenuSystem* IGMS = g_femanager.GetIGMS(viewport);
    ((PauseMenu*)IGMS->menus[0])->UnPause();
}

// ea: 0x005C1750
void BrocSys::SetSpectateState(int state, int viewport)
{
    InGameMenuSystem* IGMS = g_femanager.GetIGMS(viewport);
    SpectateMenu* v2 = (SpectateMenu*)IGMS->menus[12];
    *(int*)((char*)v2 + 76) = state;
    v2->UpdateState();
}

// ea: 0x005C1780
void BrocSys::SetSpectateSeconds(int seconds, int viewport)
{
    InGameMenuSystem* IGMS = g_femanager.GetIGMS(viewport);
    SpectateMenu* v2 = (SpectateMenu*)IGMS->menus[12];
    *(int*)((char*)v2 + 0x10) = seconds;  // mSeconds (+0x10 per IDA type)
    v2->UpdateSeconds();
}

// ea: 0x005C17B0
void BrocSys::SetSpectateMedic(int medic, int viewport)
{
    InGameMenuSystem* IGMS = g_femanager.GetIGMS(viewport);
    ((SpectateMenu*)IGMS->menus[12])->SetMedic(medic != 0);
}

// ============================================================================
// scr.o batch 37 - vote settling / InitMPCallbacks / spawn family / anim
// ============================================================================

extern int G_CallSpawnEntity(Entity* ent);  // ?G_CallSpawnEntity@@YAHPAVEntity@@@Z (g_spawn.cpp)

// AAR vote menus (mp.o views)
class AARMapVote {
public:
    static void TallyVotes(void* self);  // ?TallyVotes@AARMapVote@@SAXPAV1@@Z? (mp.o)
};
class AARGameModeVote {
public:
    static void TallyVotes(void* self);  // ?TallyVotes@AARGameModeVote@@SAXPAV1@@Z (mp.o)
};

// ea: 0x005BC450
void BrocSys::SettleMapVote()
{
    if (MultiplayerMgr::sInst->IsHost())
        AARMapVote::TallyVotes(g_femanager.mAARS->menus[4]);
}

// ea: 0x005BC470
void BrocSys::SettleGameModeVote()
{
    if (MultiplayerMgr::sInst->IsHost())
        AARGameModeVote::TallyVotes(g_femanager.mAARS->menus[3]);
}

// ea: 0x005C1F30 (thunk)
void AeThreadManager::UnloadScript(void* p)
{
    (void)p;
    BrocSys::InitMPCallbacks();
}

// ea: 0x005BDBA0
void BrocSys::InitMPCallbacks()
{
    if (gpBrocAPI != nullptr)
    {
        BrocExports& e = gpBrocAPI->mBrocExports;
        e.mCallbackPlayerJoin = nullptr;
        e.mCallbackPlayerEnter = nullptr;
        e.mCallbackPlayerLeave = nullptr;
        e.mCallbackPlayerDamage = nullptr;
        e.mCallbackPainFlinch = nullptr;
        e.mCallbackPlayerKilled = nullptr;
        e.mCallbackPlayerAssist = nullptr;
        e.mCallbackPlayerRespawnRequest = nullptr;
        e.mCallbackPlayerSpawn = nullptr;
        e.mCallbackPlayerRevive = nullptr;
        e.mCallbackPlayerTeamChange = nullptr;
        e.mCallbackPlayerClassChange = nullptr;
        e.mCallbackCanTeamChange = nullptr;
        e.mCallbackVehicleKilled = nullptr;
        e.mCallbackVehicleMantled = nullptr;
        e.mCallbackRoundOver = nullptr;
        e.mCallbackNextRound = nullptr;
        e.mCallbackRestartMap = nullptr;
        e.mCallbackQuitGame = nullptr;
        e.mCallbackStopFollowing = nullptr;
        e.mCallbackFireArtillery = nullptr;
        e.mCallbackFireArtilleryShell = nullptr;
        e.mCallbackDenyArtillery = nullptr;
        e.mCallbackSpotted = nullptr;
        e.mCallbackMineFailed = nullptr;
        e.mCallbackReviveFailed = nullptr;
        e.mCallbackHealthRegenRecovering = nullptr;
        e.mCallbackPickupScriptItem = nullptr;
        e.mCallbackDropItem = nullptr;
        e.mCallbackPickupItem = nullptr;
        e.mCallbackDropFlag = nullptr;
        e.mCallbackGameState = nullptr;
        e.mCallbackZonesLoaded = nullptr;
        e.mCallbackHostOptionsChanged = nullptr;
        e.mCallbackSDHostBombRequest = nullptr;
        e.mCallbackSDBombExplosion = nullptr;
        e.mCallbackSDBombOperation = nullptr;
        e.mCallbackSDBombOperationEvent = nullptr;
        e.mCallbackHostDisconnected = nullptr;
        e.mCallbackHostMigrated = nullptr;
        e.mCallbackLocalPlayerKicked = nullptr;
        e.mCallbackGetTeamWeapon = nullptr;
        e.mCallbackGetGrenadeCount = nullptr;
        e.mCallbackGetClipCount = nullptr;
        e.mCallbackGetSlotClipCount = nullptr;
        e.mCallbackCallForMedic = nullptr;
        e.mCallbackPunishedForTeamKill = nullptr;
        e.mCallbackSpawnButtonPressed = nullptr;
        e.mCallbackGetFlagCount = nullptr;
        e.mCallbackGetTeamControllingFlag = nullptr;
        e.mCallbackGetFlagBeingCaptured = nullptr;
        e.mCallbackGetTeamCapturingFlag = nullptr;
        e.mCallbackGetFlagBeingContested = nullptr;
        e.mCallbackGetHQPercent = nullptr;
        e.mCallbackGetHQCaptureStatus = nullptr;
        e.mCallbackGetTeamCapturingHQPercent = nullptr;
        e.mCallbackGetTeamDestroyingHQPercent = nullptr;
        e.mCallbackGetFlagBreatherTime = nullptr;
        e.mCallbackShowFlagHint = nullptr;
        e.mCallbackSetLevelAudio = nullptr;
        e.mCallbackPlayerTotalScore = nullptr;
        e.mCallbackDebugRender = nullptr;
    }
}

// ea: 0x005C2340
unsigned int BrocSys::Spawn(const Broc::string& classname,
                            const Broc::vector& origin, TPakInfo pakInfo)
{
    if (IS_NAN(origin.x) || IS_NAN(origin.y) || IS_NAN(origin.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1628;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("undefined  origin fed to Spawn function"))
            __debugbreak();
        return 0;
    }
    TPakId v4 = PakInfoToPakId(pakInfo);
    Entity* v5 = G_Spawn(v4);
    v5->mClassName = classname;
    v5->mClassNameHash = HashString(v5->mClassName);
    v5->r.currentOrigin.v.m128_f32[0] = origin.x;
    v5->r.currentOrigin.v.m128_f32[1] = origin.y;
    v5->r.currentOrigin.v.m128_f32[2] = origin.z;
    if (IS_NAN(v5->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(v5->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(v5->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1614;
        AeAssert::gCurrentExpr =
            "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    v5->spawnflags = 0;
    if (origin.x == sNaN && origin.y == sNaN && origin.z == sNaN)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1617;
        AeAssert::gCurrentExpr = "origin.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Spawn called with undefined input."))
            __debugbreak();
    }
    if (G_CallSpawnEntity(v5) != 0)
    {
        return v5->mHandle.mHandle.mVal;
    }
    const char* v7 = classname.mBlock != nullptr
                         ? (const char*)(classname.mBlock + 1)
                         : defaultFileName;
    const char* v8 = va("unable to spawn \"%s\" entity", v7);
    Scr_Error(v8);
    return 0;
}

// ea: 0x005C2590
unsigned int BrocSys::SpawnWithFlag(const Broc::string& classname,
                                    const Broc::vector& origin,
                                    int iSpawnFlags, TPakInfo pakInfo)
{
    if (IS_NAN(origin.x) || IS_NAN(origin.y) || IS_NAN(origin.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1659;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "undefined origin fed to SpawnWithFlag function"))
            __debugbreak();
        return 0;
    }
    if (origin.x == sNaN && origin.y == sNaN && origin.z == sNaN)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1640;
        AeAssert::gCurrentExpr = "origin.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SpawnWithFlag called with undefined input."))
            __debugbreak();
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
        AeAssert::gCurrentLine = 1646;
        AeAssert::gCurrentExpr =
            "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    v6->spawnflags = iSpawnFlags;
    if (G_CallSpawnEntity(v6) != 0)
    {
        UpdateEntityHash(v6);
        return v6->mHandle.mHandle.mVal;
    }
    const char* v8 = classname.mBlock != nullptr
                         ? (const char*)(classname.mBlock + 1)
                         : defaultFileName;
    const char* v9 = va("unable to spawn \"%s\" entity", v8);
    Scr_Error(v9);
    return 0;
}

// ea: 0x005C27F0
unsigned int BrocSys::SpawnWithFlagAndSize(const Broc::string& classname,
                                           const Broc::vector& origin,
                                           Broc::vector& mins,
                                           Broc::vector& maxs,
                                           int iSpawnFlags, TPakInfo pakInfo)
{
    if (origin.x == sNaN && origin.y == sNaN && origin.z == sNaN)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1669;
        AeAssert::gCurrentExpr = "origin.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SpawnWithFlag called with undefined input."))
            __debugbreak();
    }
    float x = mins.x;
    if (mins.x > maxs.x)
    {
        mins.x = maxs.x;
        maxs.x = x;
    }
    float y = mins.y;
    if (y > maxs.y)
    {
        mins.y = maxs.y;
        maxs.y = y;
    }
    float z = mins.z;
    if (z > maxs.z)
    {
        mins.z = maxs.z;
        maxs.z = z;
    }
    if (IS_NAN(origin.x) || IS_NAN(origin.y) || IS_NAN(origin.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1700;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("undefined origin fed to SpawnWithFlagAndSize"))
            __debugbreak();
        return 0;
    }
    TPakId v12 = PakInfoToPakId(pakInfo);
    Entity* v13 = G_Spawn(v12);
    v13->mClassName = classname;
    v13->mClassNameHash = HashString(v13->mClassName);
    v13->r.currentOrigin.v.m128_f32[0] = origin.x;
    v13->r.currentOrigin.v.m128_f32[1] = origin.y;
    v13->r.currentOrigin.v.m128_f32[2] = origin.z;
    if (IS_NAN(v13->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(v13->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(v13->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1682;
        AeAssert::gCurrentExpr =
            "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    v13->spawnflags = iSpawnFlags;
    v13->r.mins.v.m128_f32[0] = mins.x;
    v13->r.mins.v.m128_f32[1] = mins.y;
    v13->r.mins.v.m128_f32[2] = mins.z;
    v13->r.maxs.v.m128_f32[0] = maxs.x;
    v13->r.maxs.v.m128_f32[1] = maxs.y;
    v13->r.maxs.v.m128_f32[2] = maxs.z;
    if (G_CallSpawnEntity(v13) != 0)
    {
        UpdateEntityHash(v13);
        return v13->mHandle.mHandle.mVal;
    }
    const char* v16 = classname.mBlock != nullptr
                          ? (const char*)(classname.mBlock + 1)
                          : defaultFileName;
    const char* v17 = va("unable to spawn \"%s\" entity", v16);
    Scr_Error(v17);
    return 0;
}

// ea: 0x005C38D0
void BrocSys::GetStartOrigin(Broc::vector& outVec,
                             const Broc::vector& origin,
                             const Broc::vector& angles, unsigned int anim)
{
    float axis[4][3];
    float trans[3];
    float rot[2];
    XAnimGetAbsDelta(nullptr, anim, rot, trans, 0.0f);
    axis[3][0] = origin.x;
    axis[3][1] = origin.y;
    axis[3][2] = origin.z;
    if (IS_NAN(axis[3][0]) || IS_NAN(axis[3][1]) || IS_NAN(axis[3][2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2323;
        AeAssert::gCurrentExpr =
            "!IS_NAN((axis[3])[0]) && !IS_NAN((axis[3])[1]) && !IS_NAN((axis[3])[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    AnglesToAxis(&angles.x, axis);
    MatrixTransformVector43(trans, axis, &outVec.x);
    if (IS_NAN(outVec.x) || IS_NAN(outVec.y) || IS_NAN(outVec.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2327;
        AeAssert::gCurrentExpr =
            "!IS_NAN(((*(vec3_t*)&outVec))[0]) && !IS_NAN(((*(vec3_t*)&outVec))[1]) && !IS_NAN(((*(vec3_t*)&outVec))[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
}

// ea: 0x005C3A40
void BrocSys::GetStartAngles(Broc::vector& outVec,
                             const Broc::vector& origin,
                             const Broc::vector& angles, unsigned int anim)
{
    float startAxis[3][3];
    float tempAxis[3][3];
    float axis[4][3];
    float trans[3];
    float rot[2];
    XAnimGetAbsDelta(nullptr, anim, rot, trans, 0.0f);
    axis[3][0] = origin.x;
    axis[3][1] = origin.y;
    axis[3][2] = origin.z;
    AnglesToAxis(&angles.x, axis);
    float yaw = vectosignedyaw(rot);
    YawToAxis(yaw, tempAxis);
    MatrixMultiply(tempAxis, axis, startAxis);
    AxisToAngles(startAxis, &outVec.x);
    if (IS_NAN(outVec.x) || IS_NAN(outVec.y) || IS_NAN(outVec.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2350;
        AeAssert::gCurrentExpr =
            "!IS_NAN(((*(vec3_t*)&outVec))[0]) && !IS_NAN(((*(vec3_t*)&outVec))[1]) && !IS_NAN(((*(vec3_t*)&outVec))[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
}

// ea: 0x005C3B60
void BrocSys::GetCycleOriginOffset(Broc::vector& outVec,
                                   const Broc::vector& angles,
                                   unsigned int anim)
{
    float axis[4][3];
    float startOrigin[3];
    float trans[3];
    float endOrigin[3];
    float rot[2];
    AnglesToAxis(&angles.x, axis);
    XAnimGetAbsDelta(nullptr, anim, rot, startOrigin, 0.0f);
    XAnimGetAbsDelta(nullptr, anim, rot, endOrigin, 1.0f);
    trans[0] = endOrigin[0] - startOrigin[0];
    trans[1] = endOrigin[1] - startOrigin[1];
    trans[2] = endOrigin[2] - startOrigin[2];
    MatrixTransformVector(trans, axis, &outVec.x);
    if (IS_NAN(outVec.x) || IS_NAN(outVec.y) || IS_NAN(outVec.z))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2370;
        AeAssert::gCurrentExpr =
            "!IS_NAN(((*(vec3_t*)&outVec))[0]) && !IS_NAN(((*(vec3_t*)&outVec))[1]) && !IS_NAN(((*(vec3_t*)&outVec))[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
}

// ============================================================================
// scr.o batch 39 - Print3D / weapon / badplace / anim wrappers
// ============================================================================

extern void CG_StartShakeCamera(float p, int duration, const float* src,
                                float radius,
                                int player_index);  // ?CG_StartShakeCamera@@YAXMH PBM M H@Z (cg.o)
extern void CG_EventSpawnTracer(const math::Position3* pstart,
                                const math::Position3* pend,
                                int);  // ?CG_EventSpawnTracer@@YAXABVPosition3@math@@0H@Z (cg.o)
extern const char* BG_GetWeaponTypeName(weapType_t type);  // g_bg_pmove.cpp (0x606620)
extern void Bullet_Fire(Entity* attacker, float spread, int damage,
                        weaponParms* wp, Entity* ignore,
                        float range);  // g.o
extern int XAnimIsVariationChunk(AnimTree* anims,
                                 unsigned int animIndex);  // nal.cpp (anim.o)
extern int XAnimGetFrameCount(AnimTree* anims,
                              unsigned int animIndex);  // anim.o
extern IVPointer<XModel> SV_XModelGet(const char* name);  // ?SV_XModelGet@@YA?AV?$IVPointer@VXModel@@@@PBD@Z (sv_game.cpp)

// Empty stubs in the release binary (j_nullsub_52 / j_nullsub_70)
static void BadPlaceRegister(const Broc::string* /*placeName*/) {}
static void BadPlaceRender(const Broc::string* /*placeName*/, int /*dur*/,
                           int /*teamFlags*/, BadPlaceArc* /*arc*/) {}

// ea: 0x005C2280
void BrocSys::Print3D(const Broc::vector& pos, const Broc::string& text,
                      const Broc::vector& col, float alpha, float scale)
{
    float color[4];
    color[0] = col.x;
    color[1] = col.y;
    color[2] = col.z;
    color[3] = alpha;
    const char* v7 = text.mBlock != nullptr
                         ? (const char*)(text.mBlock + 1)
                         : defaultFileName;
    g_AddDebugString(&pos.x, color, scale, v7);
}

// ea: 0x005C4040
void BrocSys::Earthquake(float scale, float fVal, const Broc::vector& source,
                         float radius, int player_index)
{
    if (source.x == sNaN && source.y == sNaN && source.z == sNaN)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2672;
        AeAssert::gCurrentExpr = "source.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Earthquake called with undefined vector."))
            __debugbreak();
    }
    CG_StartShakeCamera(scale, (int)((fVal * 1000.0) + 0.5), &source.x,
                        radius, player_index);
}

// ea: 0x005C40F0
void BrocSys::BulletTracer(const Broc::vector& vStart,
                           const Broc::vector& vEnd)
{
    if ((vStart.x == sNaN && vStart.y == sNaN && vStart.z == sNaN)
        || (vEnd.x == sNaN && vEnd.y == sNaN && vEnd.z == sNaN))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2731;
        AeAssert::gCurrentExpr = "vStart.IsDefined() && vEnd.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("BulletTracer called with undefined vector."))
            __debugbreak();
    }
    math::Position3 v4;
    v4.v.m128_f32[0] = vEnd.x;
    v4.v.m128_f32[1] = vEnd.y;
    v4.v.m128_f32[2] = vEnd.z;
    v4.v.m128_f32[3] = 0.0f;
    math::Position3 v3 = v4;
    v4.v.m128_f32[0] = vStart.x;
    v4.v.m128_f32[1] = vStart.y;
    v4.v.m128_f32[2] = vStart.z;
    v4.v.m128_f32[3] = 0.0f;
    CG_EventSpawnTracer(&v4, &v3, 0);
}

// ea: 0x005C4230
void BrocSys::MagicBullet(const Broc::string& weapName,
                          const Broc::vector& source,
                          const Broc::vector& dest)
{
    if (!weapName.mBlock
        || (source.x == sNaN && source.y == sNaN && source.z == sNaN)
        || (dest.x == sNaN && dest.y == sNaN && dest.z == sNaN))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2745;
        AeAssert::gCurrentExpr =
            "weapName.IsDefined() && source.IsDefined() && dest.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("MagicBullet called with undefined vector."))
            __debugbreak();
    }
    const char* v3 = weapName.mBlock != nullptr
                         ? (const char*)(weapName.mBlock + 1)
                         : defaultFileName;
    unsigned char WeaponIndexForName = BG_GetWeaponIndexForName(v3);
    int v5 = WeaponIndexForName;
    if (WeaponIndexForName != 0)
    {
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(WeaponIndexForName);
        weaponParms wp;
        wp.muzzleTrace[0] = source.x;
        wp.muzzleTrace[1] = source.y;
        wp.muzzleTrace[2] = source.z;
        float dir[3];
        dir[0] = dest.x - source.x;
        dir[1] = dest.y - source.y;
        float v8 = dest.z - source.z;
        wp.pWeapInfo = InfoForWeapon;
        dir[2] = v8;
        VectorNormalize(dir);
        wp.forward[0] = dir[0];
        wp.forward[1] = dir[1];
        wp.forward[2] = dir[2];
        if (IS_NAN(dir[0]) || IS_NAN(dir[1]) || IS_NAN(dir[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
            AeAssert::gCurrentLine = 2759;
            AeAssert::gCurrentExpr =
                "!IS_NAN((dir)[0]) && !IS_NAN((dir)[1]) && !IS_NAN((dir)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        weapType_t type = (weapType_t)wp.pWeapInfo->type;
        if (type != (weapType_t)0)
        {
            if (type == WEAPTYPE_GRENADE)
                Scr_Error(
                    "MagicBullet only handle weapons of type bullet currently.\n");
        }
        else
        {
            Bullet_Fire(EntityManager::sInst->mWorld, 0.0f,
                        wp.pWeapInfo->iDamage, &wp, nullptr, 0.0f);
            G_TempEntity(&source.x, 186)->s.weapon = v5;
        }
    }
    else
    {
        const char* v6 = va(
            "MagicBullet called with unknown weapon name %s\n",
            weapName.mBlock);
        Scr_Error(v6);
    }
}

// ea: 0x005C44D0
float BrocSys::WeaponFireTime(const Broc::string& pszWeaponName)
{
    const char* v1 = pszWeaponName.mBlock != nullptr
                         ? (const char*)(pszWeaponName.mBlock + 1)
                         : defaultFileName;
    unsigned char WeaponIndexForName = BG_GetWeaponIndexForName(v1);
    if (WeaponIndexForName != 0)
        return (float)(BG_GetInfoForWeapon(WeaponIndexForName)->iFireTime
                       * 0.001);
    return 0.0f;
}

// ea: 0x005C45E0
void BrocSys::WeaponType(Broc::string& outStr,
                         const Broc::string& pszWeaponName)
{
    const char* v2 = pszWeaponName.mBlock != nullptr
                         ? (const char*)(pszWeaponName.mBlock + 1)
                         : defaultFileName;
    unsigned char WeaponIndexForName = BG_GetWeaponIndexForName(v2);
    if (WeaponIndexForName != 0)
    {
        weaponFileInfo_t* InfoForWeapon =
            BG_GetInfoForWeapon(WeaponIndexForName);
        const char* WeaponTypeName =
            BG_GetWeaponTypeName((weapType_t)InfoForWeapon->type);
        outStr = WeaponTypeName;
    }
    else
    {
        outStr = "none";
    }
}

// ea: 0x005C4640
void BrocSys::BadPlaceDelete(const Broc::string& placeName)
{
    if (placeName.mBlock != nullptr
        && placeName.mBlock != (Broc::string::Block*)-12
        && *(char*)(placeName.mBlock + 1) != 0)
    {
        BadPlaceRegister(&placeName);
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s",
                                 "badplace_delete called with name \"\""))
            __debugbreak();
    }
}

// ea: 0x005C46B0
void BrocSys::BadPlaceCylinder(const Broc::string& placeName, float dur,
                               const Broc::vector& ori, float rad,
                               float height,
                               const Broc::string& pszTeamName)
{
    (void)pszTeamName;  // unused trailing param (never read in the binary)
    double v5 = dur * 1000.0;
    BadPlaceArc arc;
    arc.origin[0] = ori.z;
    arc.origin[1] = ori.y;
    arc.origin[2] = ori.z;
    arc.radius = rad;
    arc.halfheight = height * 0.5;
    arc.angle0 = 0.0f;
    arc.angle1 = 360.0f;
    Broc::string s("badplace_cylinder");
    int TeamFlags = GetTeamFlags(placeName, s);
    BadPlaceRender(&placeName, (int)ceil(v5), TeamFlags, &arc);
}

// ea: 0x005C47A0
void BrocSys::BadPlaceArcs(const Broc::string& placeName, float dur,
                           const Broc::vector& ori, float rad, float height,
                           const Broc::vector& ang, float fVal1, float fVal2,
                           const Broc::string& pszTeamName)
{
    double v9 = dur * 1000.0;
    int v10 = (int)ceil(v9);
    BadPlaceArc arc;
    arc.origin[0] = ori.z;
    arc.origin[1] = ori.y;
    arc.origin[2] = ori.z;
    arc.radius = rad;
    arc.halfheight = height * 0.5;
    float v11 = vectoyaw(&ang.x);
    float v12 = v11;
    arc.angle0 = v11 - fVal1;
    if ((v11 - fVal1) > v11)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s",
                                 "left angle < 0 in badplace_arc\n"))
            __debugbreak();
        v12 = v11;
    }
    float angle1 = v12 + fVal2;
    arc.angle1 = v12 + fVal2;
    if (v12 > (v12 + fVal2))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s",
                                 "right angle < 0 in badplace_arc\n"))
            __debugbreak();
        angle1 = arc.angle1;
    }
    if ((angle1 - arc.angle0) < 360.0f)
    {
        AngleNormalize360Accurate(arc.angle0);
        arc.angle0 = v11;
        AngleNormalize360Accurate(arc.angle1);
        arc.angle1 = v11;
    }
    else
    {
        arc.angle0 = 0.0f;
        arc.angle1 = 360.0f;
    }
    Broc::string s("badplace_arc");
    int TeamFlags = GetTeamFlags(pszTeamName, s);
    BadPlaceRender(&placeName, v10, TeamFlags, &arc);
}

// ea: 0x005C8040
void BrocSys::GetNumParts(const Broc::string& modelName)
{
    if (modelName.mBlock == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2799;
        AeAssert::gCurrentExpr = "modelName.IsDefined()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("GetNumParts called with undefined input."))
            __debugbreak();
    }
    const char* v2 = modelName.mBlock != nullptr
                         ? (const char*)(modelName.mBlock + 1)
                         : defaultFileName;
    IVPointer<XModel> result = SV_XModelGet(v2);
    XModel* mValue = result.mValue;
    ValidatePakId((TPakId)result.mPakId);
    XModelLod* v5 = mValue->lod[0];
    XModelLod** lod = mValue->lod;
    int v7 = 0;
    if (v5 == nullptr)
    {
        XModelLod** v8 = mValue->lod;
        XModelLod* v9;
        do
        {
            v9 = v8[1];
            ++v8;
            ++v7;
        } while (v9 == nullptr);
    }
    if (mValue->lod[v7]->xmodelParts != nullptr && v5 == nullptr)
    {
        XModelLod* v10;
        do
        {
            v10 = lod[1];
            ++lod;
        } while (v10 == nullptr);
    }
}

// ea: 0x005C8100
void BrocSys::NotifyPakLoadOperation(const char* operation,
                                     const char* longName)
{
    char notify[254];
    int oLen = 0;
    AeStringSupport::CStrToAeStr(notify, oLen, 254, operation);
    AeStringSupport::Concat(notify, oLen, 254, longName);
    Entity* mWorld = EntityManager::sInst->mWorld;
    if (mWorld != nullptr)
    {
        HashString v3;
        v3.mHash = HashString::CalcHash(notify);
        mWorld->Notify(v3);
    }
}

// ea: 0x005C81D0
float BrocSys::GetAnimLength(unsigned int entityHandleVal,
                             unsigned int broanim)
{
    (void)entityHandleVal;
    AnimBank* Bank = AnimBankManager::sInst->GetBank(PAK_ID_MIN);
    AnimTree* v3 = &Bank->anims[broanim >> 16];
    if (!XAnimIsVariationChunk(v3, broanim)
        && XAnimIsPrimitive(v3, broanim) == 0)
        Scr_ParamError(0, "non-primitive animation has no concept of length");
    return XAnimGetLength(v3, broanim);
}

// ea: 0x005C8230
int BrocSys::GetAnimFrameCount(unsigned int entityHandleVal,
                               unsigned int broanim)
{
    (void)entityHandleVal;
    AnimBank* Bank = AnimBankManager::sInst->GetBank(PAK_ID_MIN);
    AnimTree* v3 = &Bank->anims[broanim >> 16];
    if (!XAnimIsVariationChunk(v3, broanim)
        && XAnimIsPrimitive(v3, broanim) == 0)
        Scr_ParamError(0, "non-primitive animation has no concept of length");
    return XAnimGetFrameCount(v3, broanim);
}

// ============================================================================
// scr.o batch 41 - weapon/projectile + thread handle wrappers + globals
// ============================================================================

// ScriptEventParams - script event arguments (verified vs IDA)
class ScriptEventParams {
public:
    unsigned int ent1;  // +0x00
    unsigned int ent2;  // +0x04
    float        f1;    // +0x08
    float        f2;    // +0x0C
    float        f3;    // +0x10
    Broc::vector v1;    // +0x14
};

extern vm_s* VM_Create(const char* name,
                       int (*entry)(int*));  // sv_decl.h / g_entity_misc.cpp

// ea: 0x005C9B20
void BrocSys::SetProjectileSpeed(unsigned int entityHandleVal, int speed)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v2 < 0x540
        && entityHandleVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject)
               != nullptr)
    {
        unsigned char weapon = mObject->s.weapon;
        if (weapon != 0)
        {
            weaponFileInfo_t* InfoForWeapon =
                BG_GetInfoForWeapon(weapon);
            if (InfoForWeapon != nullptr)
                InfoForWeapon->iProjectileSpeed = speed;
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 591;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Bad entity passed to SetProjectileSpeed"))
            __debugbreak();
    }
}

// ea: 0x005C9BC0
void BrocSys::SetWeaponPlayerUpOffset(unsigned int entityHandleVal,
                                      float offset)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v2 < 0x540
        && entityHandleVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject)
               != nullptr)
    {
        unsigned char weapon = mObject->s.weapon;
        if (weapon != 0)
        {
            weaponFileInfo_t* InfoForWeapon =
                BG_GetInfoForWeapon(weapon);
            if (InfoForWeapon != nullptr)
                *(float*)((char*)InfoForWeapon + 0x540 + 8) = offset;
                // vProneOfs[2] (+0x540 per g_weaponfuncs.h)
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 609;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "Bad entity passed to SetWeaponPlayerUpOffset"))
            __debugbreak();
    }
}

// ea: 0x005C9C60
int BrocSys::FollowCycle(unsigned int entityHandleVal, int dir)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v2 < 0x540
        && entityHandleVal >> 12
               == (unsigned int)EntityHandleDb::sInst.mElements[v2].mKey)
        mObject = EntityHandleDb::sInst.mElements[v2].mObject;
    return Cmd_FollowCycle_f(mObject, dir);
}

// ea: 0x005C9CA0
void BrocSys::EnableWeapon(unsigned int entityHandleVar)
{
    unsigned int v1 = entityHandleVar & 0xFFF;
    Entity* mObject = nullptr;
    if (v1 >= 0x540
        || entityHandleVar >> 12
               != (unsigned int)EntityHandleDb::sInst.mElements[v1].mKey
        || (mObject = EntityHandleDb::sInst.mElements[v1].mObject) == nullptr
        || mObject->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 867;
        AeAssert::gCurrentExpr = "ent && ent->client";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can only call EnableWeapon on a player"))
            __debugbreak();
    }
    mObject->client->ps.pm_flags &= ~0x800000u;
}

// ea: 0x005C9D30
void BrocSys::DisableWeapon(unsigned int entityHandleVar)
{
    unsigned int v1 = entityHandleVar & 0xFFF;
    Entity* mObject = nullptr;
    if (v1 >= 0x540
        || entityHandleVar >> 12
               != (unsigned int)EntityHandleDb::sInst.mElements[v1].mKey
        || (mObject = EntityHandleDb::sInst.mElements[v1].mObject) == nullptr
        || mObject->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 875;
        AeAssert::gCurrentExpr = "ent && ent->client";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can only call DisableWeapon on a player"))
            __debugbreak();
    }
    mObject->client->ps.pm_flags |= 0x800000u;
}

// ea: 0x005C9E40
bool BrocSys::ThreadIsThreadAlive(unsigned int handle)
{
    unsigned int idx = handle & 0xFF;
    if (idx >= 0x100)
        return false;
    AeThreadManagerLayout* L =
        (AeThreadManagerLayout*)&AeThreadManager::sInst;
    auto* db = (HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb;
    return handle >> 8 == (unsigned int)db->mElements[idx].mKey
           && db->mElements[idx].mObject != nullptr;
}

// ea: 0x005C9E80
void BrocSys::ThreadKill(unsigned int threadId)
{
    unsigned int idx = threadId & 0xFF;
    AeThreadManagerLayout* L =
        (AeThreadManagerLayout*)&AeThreadManager::sInst;
    auto* db = (HandleDb<AeThread, 256, SizedHandle<8, 24>>*)L->mHandleDb;
    if (threadId >> 8 == (unsigned int)db->mElements[idx].mKey)
    {
        AeThread* mObject = db->mElements[idx].mObject;
        if (mObject != nullptr)
        {
            unsigned int v2 = mObject->mFlags.mMask | 8;
            mObject->mFlags.mMask = v2;
            v2 |= 0x40u;
            mObject->mFlags.mMask = v2;
            mObject->mFlags.mMask = v2 | 4;
        }
    }
}

// ea: 0x005C1A60
void Scr_EmitAnimation(char* a, unsigned short b, unsigned int c)
{
    (void)a;
    (void)b;
    (void)c;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_animtree.cpp";
    AeAssert::gCurrentLine = 151;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("Is this still used? (CD)"))
        __debugbreak();
}

// ea: 0x005BE3D0
void BrocAddEntityThread(Entity* ent, unsigned int fcnHash,
                         ScriptEventParams* params)
{
    if (gpBrocAPI == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 245;
        AeAssert::gCurrentExpr = "gpBrocAPI";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (gpBrocAPI->mBrocExports.mSpawnScriptThread == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 246;
        AeAssert::gCurrentExpr =
            "gpBrocAPI->mBrocExports.mSpawnScriptThread";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int v3;
    if (params != nullptr)
    {
        v3 = (int)gpBrocAPI->mBrocExports.mSpawnScriptThread(
            fcnHash, 0, Broc::entity(ent->mHandle.mHandle.mVal),
            Broc::entity(params->ent1), Broc::entity(params->ent1),
            params->f1, params->f2, params->f3, &params->v1);
    }
    else
    {
        Broc::vector undefined_vec;
        undefined_vec.x = sNaN;
        undefined_vec.y = sNaN;
        undefined_vec.z = sNaN;
        v3 = (int)gpBrocAPI->mBrocExports.mSpawnScriptThread(
            fcnHash, 0, Broc::entity(ent->mHandle.mHandle.mVal),
            Broc::entity(0), Broc::entity(0), sNaN, sNaN, sNaN,
            &undefined_vec);
    }
    if (v3 == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 260;
        AeAssert::gCurrentExpr = "found";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Couldn't find AUTO_THREAD "))
            __debugbreak();
    }
}

// ea: 0x005C5790
void BrocInitEntity(
    Entity* ent,
    const InplaceVector<InplaceTreeElement<InplaceString, InplaceString>>&
        keyValuePairs)
{
    void* v3 = nullptr;
    if (gpBrocAPI != nullptr
        && gpBrocAPI->mBrocExports.mCreateExtendedEntity != nullptr)
    {
        const char** pKey = nullptr;
        if (keyValuePairs.mSize != 0)
            pKey = (const char**)&keyValuePairs.mList[0].mKey.mStr;
        v3 = gpBrocAPI->mBrocExports.mCreateExtendedEntity(
            pKey, keyValuePairs.mSize);
    }
    ent->mBrocExtendedEntity = v3;
    UpdateEntityHash(ent);
}

// ea: 0x005C7940
vm_s* VM_Restart(vm_s* vm)
{
    if (vm->dllHandle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\vm.cpp";
        AeAssert::gCurrentLine = 122;
        AeAssert::gCurrentExpr = "vm->dllHandle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int (*systemCall)(int*) = vm->systemCall;
    char name[128];
    Q_strncpyz(name, vm->name, 128);
    VM_Free(vm);
    return VM_Create(name, systemCall);
}

// ============================================================================
// scr.o batch 42 - thread sleep / mem / sound / anim-tree load
// ============================================================================

extern void ParseNoteTracks(XAnimEntry* entry);  // nal.cpp (anim.o)
extern void XAnimSetupSyncNodes(AnimTree* anims);  // nal.cpp (anim.o)

// ea: 0x005C7EC0
void BrocSys::ThreadSleepFrames(int numFrames)
{
    AeThread* mThreadExecuting =
        (AeThread*)AeThreadManager::sInst.mThreadExecuting;
    void* mem = AeThreadStateAllocAccess::Get()->Allocate(0x18u, false);
    AeThreadState* v2 = mem != nullptr
                            ? (AeThreadState*)new (mem) AeThreadWaitFramesState(
                                  numFrames)
                            : nullptr;
    unsigned int v5 = mThreadExecuting->mFlags.mMask | 2;
    mThreadExecuting->mFlags.mMask = v5;
    v5 |= 0x10u;
    mThreadExecuting->mFlags.mMask = v5;
    mThreadExecuting->mFlags.mMask = v5 | 0x200;
    v2->m_dlist_node.mNext = mThreadExecuting->mStateControllers.m_end;
    v2->m_dlist_node.mPrev = mThreadExecuting->mStateControllers.m_tail;
    ((AeDListNode*)mThreadExecuting->mStateControllers.m_tail)->mNext =
        &v2->m_dlist_node;
    mThreadExecuting->mStateControllers.m_tail = &v2->m_dlist_node;
    ++mThreadExecuting->mStateControllers.m_size;
    LongJmp(AeThread::sBackup);
}

// ea: 0x005C7F50
void BrocSys::ThreadSleepInternal(float sleepTime)
{
    if (IS_NAN(sleepTime))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 1163;
        AeAssert::gCurrentExpr = "!IS_NAN(sleepTime)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    AeThread* mThreadExecuting =
        (AeThread*)AeThreadManager::sInst.mThreadExecuting;
    void* mem = AeThreadStateAllocAccess::Get()->Allocate(0x18u, false);
    AeThreadState* v2 = mem != nullptr
                            ? (AeThreadState*)new (mem) AeThreadWaitState(
                                  sleepTime)
                            : nullptr;
    unsigned int v5 = mThreadExecuting->mFlags.mMask | 2;
    mThreadExecuting->mFlags.mMask = v5;
    mThreadExecuting->mFlags.mMask = v5 | 0x10;
    mThreadExecuting->mFlags.mMask = v5 | 0x210;
    v2->m_dlist_node.mNext = mThreadExecuting->mStateControllers.m_end;
    v2->m_dlist_node.mPrev = mThreadExecuting->mStateControllers.m_tail;
    ((AeDListNode*)mThreadExecuting->mStateControllers.m_tail)->mNext =
        &v2->m_dlist_node;
    mThreadExecuting->mStateControllers.m_tail = &v2->m_dlist_node;
    ++mThreadExecuting->mStateControllers.m_size;
    LongJmp(AeThread::sBackup);
}

// ea: 0x005C7D30
void* BrocSys::MemAlloc(unsigned int size, unsigned int align)
{
    void* result = nullptr;
    int last = gBrocPool->mPoolSizes.m_size - 1;
    if (last <= 0)
        last = 0;
    if (size > (unsigned int)gBrocPool->mPoolSizes.m_elements[last]
        || (result = gBrocPool->Allocate(size, false)) == nullptr)
    {
        result = ((ae_heap*)gBrocHeap)->Malloc(size, align);
        if (result == nullptr)
            return mem_heap_malloc((int)align, size);
    }
    return result;
}

// ea: 0x005C7D90
void BrocSys::ThreadGetDebugInfo(Broc::string& fileline,
                                 Broc::string& func,
                                 Broc::string& threadId)
{
    AeThread* mThreadExecuting =
        (AeThread*)AeThreadManager::sInst.mThreadExecuting;
    if (mThreadExecuting != nullptr)
    {
        Broc::string fileName(mThreadExecuting->mFile);
        int v4 = 0;
        int mLength = fileName.mBlock != nullptr
                          ? fileName.mBlock->mLength
                          : 0;
        int v6 = mLength - 1;
        if (v6 >= 0)
        {
            while (fileName.mBlock == nullptr
                   || v6 >= fileName.mBlock->mLength
                   || *(char*)((char*)(fileName.mBlock + 1) + v6) != 92)
            {
                if (--v6 < 0)
                    break;
            }
            if (v6 >= 0)
                v4 = v6;
        }
        int v7 = fileName.mBlock != nullptr ? fileName.mBlock->mLength : 0;
        Broc::string result = fileName.substr(v4 + 1, v7 - v4 - 1);
        fileName = result;
        const char* v9 = fileName.mBlock != nullptr
                             ? (const char*)(fileName.mBlock + 1)
                             : defaultFileName;
        ae_formatted_string<128, unsigned char> v12("%s::Line %d", v9,
                                                    mThreadExecuting->mLine);
        fileline = (const char*)v12.mBuff;
        func = mThreadExecuting->mFuncName;
        ae_formatted_string<128, unsigned char> v13("0x%08x",
                                                    mThreadExecuting);
        threadId = (const char*)v13.mBuff;
    }
}

// ea: 0x005C33B0
unsigned int BrocSys::SoundPlay(const Broc::string& name, float volume)
{
    const char* v3 = name.mBlock != nullptr
                         ? (const char*)(name.mBlock + 1)
                         : defaultFileName;
    math::Position3 v9;
    v9.v = _mm_setzero_ps();
    math::Dir3 v10;
    v10.v = _mm_setzero_ps();
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> v5 =
        SoundDevice::sInst->PlaySound(
            v3, DbLinkedHandle<EntityHandleDb, Entity>(0), false, false, v9,
            v10, -1.0f, -1.0f, -1.0f, -1.0f);
    SoundDevice::Sound* SoundForHandle =
        SoundDevice::sInst->GetSoundForHandle(v5);
    if (SoundForHandle != nullptr)
    {
        float v8 = SoundForHandle->GetVolume() * volume;
        SoundForHandle->SetVolume(v8);
    }
    return v5.mHandle.mVal;
}

// ea: 0x005C7730
void Scr_LoadAnimTreeAtIndex(int treeindex, void* (__cdecl* Alloc)(int),
                             bool restart)
{
    (void)Alloc;
    AnimBank* Bank = AnimBankManager::sInst->GetBank(PAK_ID_MIN);
    AnimTree* v4 = &Bank->anims[treeindex];
    tlFixedString treehash(v4->name.mStr);
    for (unsigned int i = 0; i < v4->entries.mSize; ++i)
    {
        XAnimEntry* v7 = &v4->entries.mList[i];
        BrocHelper::AnimationToBroLookup(
            treehash.str, treeindex, v7->hash, (int)i);
        v7->lastAttempt = 0;
        v7->anim = nullptr;
        if (!restart)
            ParseNoteTracks(v7);
    }
    if (!restart)
        XAnimSetupSyncNodes(v4);
}

// ea: 0x005C7820
void Scr_FreeAnimTreeAtIndex(int treeindex)
{
    AnimBank* Bank = AnimBankManager::sInst->GetBank(PAK_ID_MIN);
    AnimTree* v2 = &Bank->anims[treeindex];
    for (unsigned int i = 0; i < v2->entries.mSize; ++i)
        v2->entries.mList[i].Release();
}

// ============================================================================
// scr.o batch 43 - event handlers / radius damage / pak / mover speed
// ============================================================================

// ea: 0x005C9DC0
bool BrocSys::AssignParameterForNotify(unsigned int entHandle,
                                       unsigned int notify,
                                       WaitTilOutput* scriptVar)
{
    unsigned int v3 = entHandle & 0xFFF;
    if (v3 >= 0x540)
        return false;
    if (entHandle >> 12 != EntityHandleDb::sInst.mElements[v3].mKey)
        return false;
    Entity* mObject = EntityHandleDb::sInst.mElements[v3].mObject;
    if (mObject == nullptr)
        return false;
    EntityNotifySet* mNotifySet = mObject->mNotifySet;
    if (mNotifySet == nullptr)
        return false;
    entHandle = notify;
    return ((EntityNotifySetLocal*)mNotifySet)
        ->AssignScriptVariable((const HashString&)entHandle, scriptVar);
}

// ea: 0x005CA260
bool BrocSys::AddEventHandler(unsigned int entityHandleVal,
                              unsigned int notifyId, unsigned int callback)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    Entity* mObject;
    return v3 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v3].mObject) != nullptr
        && mObject->AddScriptEvent(notifyId, callback);
}

// ea: 0x005CA2B0
bool BrocSys::RemoveEventHandler(unsigned int entityHandleVal,
                                 unsigned int notifyId,
                                 unsigned int callback)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    Entity* mObject;
    ScriptEventHandler* mScriptEventHandler;
    return v3 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v3].mObject) != nullptr
        && (mScriptEventHandler = mObject->mScriptEventHandler) != nullptr
        && mScriptEventHandler->RemoveEvent(notifyId, callback);
}

// ea: 0x005CA710
void BrocSys::RadiusDamageFromEnt(unsigned int ehandle,
                                  const Broc::vector& origin, float range,
                                  float max_damage, float min_damage,
                                  int damageType)
{
    unsigned int v6 = ehandle & 0xFFF;
    Entity* mObject = nullptr;
    if (v6 >= 0x540
        || ehandle >> 12 != EntityHandleDb::sInst.mElements[v6].mKey
        || (mObject = EntityHandleDb::sInst.mElements[v6].mObject) == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 2445;
        AeAssert::gCurrentExpr = "which";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "passing in NULL entity to RadiusDamageFromEnt!"))
            __debugbreak();
    }
    level.bPlayerIgnoreRadiusDamage = level.bPlayerIgnoreRadiusDamageLatched;
    G_RadiusDamage(&origin.x, mObject, mObject, max_damage, min_damage, range,
                   nullptr, damageType);
    level.bPlayerIgnoreRadiusDamage = 0;
}

// ea: 0x005CA7C0
TPakInfo BrocSys::GetPakEntity(unsigned int ehandle)
{
    unsigned int v1 = ehandle & 0xFFF;
    Entity* mObject;
    PakManager* v5;
    TPakId mPakId;
    if (v1 < 0x540
        && ehandle >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        mPakId = (TPakId)mObject->mPakId;
        if (mPakId == PAK_ID_INVALID)
            mPakId = CurPakId();
        v5 = PakManager::sInst;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3071;
        AeAssert::gCurrentExpr = "which";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("passing in NULL entity to GetPak!"))
            __debugbreak();
        v5 = PakManager::sInst;
        mPakId = CurPakId();
    }
    const PakInfoNode* PakInfo = v5->GetPakInfo(mPakId);
    if (PakInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 3078;
        AeAssert::gCurrentExpr = "result";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Unknown pak file"))
            __debugbreak();
    }
    return (TPakInfo)(uintptr_t)PakInfo;
}

// ea: 0x005CA9F0
void GetEntType(DbLinkedHandle<EntityHandleDb, Entity> entityHandle,
                Broc::string& type)
{
    if (entityHandle.mHandle.mVal != 0)
    {
        unsigned int v2 = entityHandle.mHandle.mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v2 < 0x540
            && entityHandle.mHandle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v2].mKey)
            mObject = EntityHandleDb::sInst.mElements[v2].mObject;
        Broc::string* p_obstacle = &str_const.obstacle;
        if ((mObject->flags & 0x4000) == 0)
            p_obstacle = &str_const.world;
        type = *p_obstacle;
    }
    else
    {
        type = str_const.none;
    }
}

// ea: 0x005CAA50
int BrocSys::GetPlayerIndex(unsigned int player)
{
    unsigned int v1 = player & 0xFFF;
    Entity* mObject = nullptr;
    if (v1 < 0x540
        && player >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
        mObject = EntityHandleDb::sInst.mElements[v1].mObject;
    return EntityManager::sInst->GetPlayerIndex(mObject);
}

// ea: 0x005CAA90
void* BrocSys::GetExtendedEntity(unsigned int handle)
{
    unsigned int v1 = handle & 0xFFF;
    if (v1 >= 0x540)
        return nullptr;
    if (handle >> 12 != EntityHandleDb::sInst.mElements[v1].mKey)
        return nullptr;
    Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
    if (mObject == nullptr)
        return nullptr;
    if (mObject->mBrocExtendedEntity == nullptr)
    {
        void* (*mCreateExtendedEntity)(const char**, int) =
            gpBrocAPI->mBrocExports.mCreateExtendedEntity;
        if (mCreateExtendedEntity != nullptr)
            mObject->mBrocExtendedEntity = mCreateExtendedEntity(nullptr, 0);
    }
    return mObject->mBrocExtendedEntity;
}

// ea: 0x005CAB00
void BrocSys::Scr_SetModel(Entity* ent, int offset, Broc::string* val)
{
    (void)offset;
    DObj* mDObj = ent->mDObj;
    if (mDObj != nullptr)
    {
        XModel* mValue = mDObj->models[0].mValue;
        TPakId mPakId = (TPakId)mDObj->models[0].mPakId;
        ValidatePakId(mPakId);
        if (mValue != nullptr)
        {
            ValidatePakId(mPakId);
            *val = mValue->name.mStr;
        }
    }
}

// ea: 0x005C02C0
void BrocSys::Mover_SetupMoveSpeed(
    trajectory_t* pTr, const math::Position3& vSpeed, float fTotalTime,
    float fAccelTime, float fDecelTime, math::Position3& vCurrPos,
    float* pfSpeed, float* pfMidTime, float* pfDecelTime,
    math::Position3& vPos1, math::Position3& vPos2, math::Position3& vPos3)
{
    if (pTr->trType != TR_STATIONARY)
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    if (fAccelTime == 0.0f && fDecelTime == 0.0f)
    {
        pTr->trTime = level.time;
        pTr->trDuration = (int)(fTotalTime * 1000.0f);
        *pfMidTime = fTotalTime;
        *pfDecelTime = 0.0f;
        pTr->trBase[0] = vCurrPos.v.m128_f32[0];
        pTr->trBase[1] = vCurrPos.v.m128_f32[1];
        pTr->trBase[2] = vCurrPos.v.m128_f32[2];
        pTr->trDelta[0] = vSpeed.v.m128_f32[0];
        pTr->trDelta[1] = vSpeed.v.m128_f32[1];
        pTr->trDelta[2] = vSpeed.v.m128_f32[2];
        if (IS_NAN(pTr->trDelta[0]) || IS_NAN(pTr->trDelta[1])
            || IS_NAN(pTr->trDelta[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityMove.cpp";
            AeAssert::gCurrentLine = 169;
            AeAssert::gCurrentExpr = "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        pTr->trType = TR_LINEAR_STOP;
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
        BG_EvaluateTrajectory(pTr, level.time + pTr->trDuration, vPos3);
    }
    else
    {
        *pfMidTime = (fTotalTime - fAccelTime) - fDecelTime;
        *pfDecelTime = fDecelTime;
        *pfSpeed = sqrtf(vSpeed.v.m128_f32[0] * vSpeed.v.m128_f32[0]
                       + vSpeed.v.m128_f32[1] * vSpeed.v.m128_f32[1]
                       + vSpeed.v.m128_f32[2] * vSpeed.v.m128_f32[2]);
        if (fAccelTime == 0.0f)
        {
            vPos1.v.m128_f32[0] = vCurrPos.v.m128_f32[0];
            vPos1.v.m128_f32[1] = vCurrPos.v.m128_f32[1];
            vPos1.v.m128_f32[2] = vCurrPos.v.m128_f32[2];
            if (*pfMidTime == 0.0f)
            {
                pTr->trTime = level.time;
                pTr->trDuration = (int)(*pfDecelTime * 1000.0f);
                pTr->trBase[0] = vCurrPos.v.m128_f32[0];
                pTr->trBase[1] = vCurrPos.v.m128_f32[1];
                pTr->trBase[2] = vCurrPos.v.m128_f32[2];
                pTr->trDelta[0] = vSpeed.v.m128_f32[0];
                pTr->trDelta[1] = vSpeed.v.m128_f32[1];
                pTr->trDelta[2] = vSpeed.v.m128_f32[2];
                if (IS_NAN(pTr->trDelta[0]) || IS_NAN(pTr->trDelta[1])
                    || IS_NAN(pTr->trDelta[2]))
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityMove.cpp";
                    AeAssert::gCurrentLine = 218;
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
                pTr->trDelta[0] = vSpeed.v.m128_f32[0];
                pTr->trDelta[1] = vSpeed.v.m128_f32[1];
                pTr->trDelta[2] = vSpeed.v.m128_f32[2];
                if (IS_NAN(pTr->trDelta[0]) || IS_NAN(pTr->trDelta[1])
                    || IS_NAN(pTr->trDelta[2]))
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityMove.cpp";
                    AeAssert::gCurrentLine = 208;
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
            pTr->trDelta[0] = vSpeed.v.m128_f32[0];
            pTr->trDelta[1] = vSpeed.v.m128_f32[1];
            pTr->trDelta[2] = vSpeed.v.m128_f32[2];
            if (IS_NAN(pTr->trDelta[0]) || IS_NAN(pTr->trDelta[1])
                || IS_NAN(pTr->trDelta[2]))
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityMove.cpp";
                AeAssert::gCurrentLine = 191;
                AeAssert::gCurrentExpr = "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid vector"))
                    __debugbreak();
            }
            int trDuration = pTr->trDuration;
            pTr->trType = TR_ACCELERATE;
            BG_EvaluateTrajectory(pTr, level.time + trDuration, vPos1);
        }
        vPos2.v.m128_f32[0] =
            (*pfMidTime * vSpeed.v.m128_f32[0]) + vPos1.v.m128_f32[0];
        vPos2.v.m128_f32[1] =
            (vSpeed.v.m128_f32[1] * *pfMidTime) + vPos1.v.m128_f32[1];
        vPos2.v.m128_f32[2] =
            (*pfMidTime * vSpeed.v.m128_f32[2]) + vPos1.v.m128_f32[2];
        if (*pfDecelTime == 0.0f)
        {
            vPos3.v.m128_f32[0] = vPos2.v.m128_f32[0];
            vPos3.v.m128_f32[1] = vPos2.v.m128_f32[1];
            vPos3.v.m128_f32[2] = vPos2.v.m128_f32[2];
        }
        else
        {
            trajectory_t tr;
            tr.trBase[0] = vPos2.v.m128_f32[0];
            tr.trBase[1] = vPos2.v.m128_f32[1];
            tr.trBase[2] = vPos2.v.m128_f32[2];
            tr.trDelta[0] = vSpeed.v.m128_f32[0];
            tr.trDelta[1] = vSpeed.v.m128_f32[1];
            tr.trDelta[2] = vSpeed.v.m128_f32[2];
            tr.trGravityOverride = 0;
            tr.trType = TR_DECCELERATE;
            tr.trTime = level.time;
            tr.trDuration = (int)(*pfDecelTime * 1000.0f);
            if (IS_NAN(tr.trDelta[0]) || IS_NAN(tr.trDelta[1])
                || IS_NAN(tr.trDelta[2]))
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntityMove.cpp";
                AeAssert::gCurrentLine = 236;
                AeAssert::gCurrentExpr = "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid vector"))
                    __debugbreak();
            }
            BG_EvaluateTrajectory(&tr, tr.trDuration + level.time, vPos3);
        }
        BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
    }
}

// ============================================================================
// scr.o batch 44 - effect events / entity state queries / blank-state
// ============================================================================

// ea: 0x005CAB50
unsigned int BrocSys::GetEntByNum(int entnum)
{
    unsigned int v1 = (unsigned int)entnum & 0xFFF;
    Entity* mObject = nullptr;
    if (entnum < 0x540 && v1 < 0x540
        && (unsigned int)entnum >> 12
               == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
        return mObject->mHandle.mHandle.mVal;
    return 0;
}

// ea: 0x005CABA0
void BrocSys::DrawTracer(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 >= 0x540
        || entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v1].mKey
        || EntityHandleDb::sInst.mElements[v1].mObject == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 931;
        AeAssert::gCurrentExpr = "e";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("EffectEventPlay - passed in entity is NULL"))
            __debugbreak();
    }
}

// ea: 0x005CAC20
int BrocSys::EffectEventWeaponPlay(unsigned int entityHandleVal,
                                   unsigned int action)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 >= 0x540
        || entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v2].mKey
        || (mObject = EntityHandleDb::sInst.mElements[v2].mObject) == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 949;
        AeAssert::gCurrentExpr = "e";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("EffectEventPlay - passed in entity is NULL"))
            __debugbreak();
        return -1;
    }
    Client* client = mObject->client;
    if (client == nullptr)
        return -1;
    gitem_s* v6 = &bg_itemlist[client->ps.weapon];
    if (action == 0)
        return PostEffectEventWeapon(mObject, v6->classname,
                                     (EAction)0x1F /* kActionVEHICLE_START */)
            .mVal;
    if (action != 1)
    {
        if (action == 2)
            return PostEffectEventWeapon(
                       mObject, v6->classname,
                       (EAction)0x21 /* kActionVEHICLE_WHEELDUST */)
                .mVal;
        return -1;
    }
    return PostEffectEventWeapon(mObject, v6->classname,
                                 (EAction)0x20 /* kActionVEHICLE_RUMBLE */)
        .mVal;
}

// ea: 0x005CAD10
int BrocSys::EffectEventPlay(unsigned int entityHandleVal,
                             const Broc::string& script, int notifyHash,
                             bool stoppable, bool important)
{
    (void)stoppable;
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 985;
        AeAssert::gCurrentExpr = "!script.is_empty()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("EffectEventPlay - passed in script is NULL"))
            __debugbreak();
    }
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
        return -1;
    const char* v7 = (const char*)(script.mBlock + 1);
    unsigned int v9 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v9 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v9].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v9].mObject) != nullptr)
    {
        if (notifyHash != 0)
        {
            if (mObject->snd_wait.notifyHash.mHash != 0)
            {
                G_DPrintf(
                    "EffectEventPlay: skipping effect %s, entity already has a sound notify\n",
                    v7);
                return -1;
            }
            RegisterEffectWait(mObject, notifyHash);
        }
        const char* v12 = script.mBlock != nullptr
                              ? (const char*)(script.mBlock + 1)
                              : defaultFileName;
        return PostEffectEventScriptCall(mObject, v12, false, PAK_ID_INVALID,
                                         important)
            .mVal;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 990;
        AeAssert::gCurrentExpr = "e";
        if (!AeAssert::IsIgnored())
        {
            const char* v11 = script.mBlock != nullptr
                                  ? (const char*)(script.mBlock + 1)
                                  : defaultFileName;
            if (AeAssert::Assert(
                    "EffectEventPlay - passed in entity is NULL (scriptid=%s)",
                    v11))
                __debugbreak();
        }
        return -1;
    }
}

// ea: 0x005CAE70
int BrocSys::EffectEventPlayNonEnt(const Broc::string& script,
                                   const Broc::vector& pos,
                                   const Broc::vector& facing,
                                   bool bImportant, unsigned int entityHandle,
                                   int notifyHash)
{
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0xD;  // DL
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 1021;
        AeAssert::gCurrentExpr = "!script.is_empty()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "EffectEventPlay (Non entity based) - passed in script is NULL"))
            __debugbreak();
    }
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
        return -1;
    Entity* mObject;
    if (entityHandle != 0)
    {
        unsigned int v8 = entityHandle & 0xFFF;
        mObject = nullptr;
        if (v8 < 0x540
            && entityHandle >> 12 == EntityHandleDb::sInst.mElements[v8].mKey)
            mObject = EntityHandleDb::sInst.mElements[v8].mObject;
        if (mObject == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0xD;  // DL
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 1030;
            AeAssert::gCurrentExpr = "e";
            if (!AeAssert::IsIgnored())
            {
                const char* v10 = script.mBlock != nullptr
                                      ? (const char*)(script.mBlock + 1)
                                      : defaultFileName;
                if (AeAssert::Assert(
                        "EffectEventPlay (Non entity based) - passed in entity is NULL (scriptid=%s)",
                        v10))
                    __debugbreak();
            }
        }
        if (mObject->snd_wait.notifyHash.mHash != 0)
            return -1;
        if (notifyHash != 0)
            RegisterEffectWait(mObject, notifyHash);
    }
    else
    {
        unsigned int v11 =
            EntityManager::sInst->mWorld->mHandle.mHandle.mVal & 0xFFF;
        Entity* v12 = nullptr;
        if (v11 < 0x540
            && EntityManager::sInst->mWorld->mHandle.mHandle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v11].mKey)
            v12 = EntityHandleDb::sInst.mElements[v11].mObject;
        mObject = v12;
    }
    const char* v13 = script.mBlock != nullptr
                          ? (const char*)(script.mBlock + 1)
                          : defaultFileName;
    return PostEffectEventScriptCall(mObject, v13, pos, facing, false,
                                     PAK_ID_INVALID, bImportant)
        .mVal;
}

// ea: 0x005CB000
int BrocSys::EffectEventPlayDir(unsigned int entityHandleVal,
                                const Broc::string& script,
                                const Broc::vector& dir, int notifyHash,
                                bool bUnused)
{
    (void)bUnused;
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 1063;
        AeAssert::gCurrentExpr = "!script.is_empty()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("EffectEventPlayDir - passed in script is NULL"))
            __debugbreak();
    }
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
        return -1;
    unsigned int v6 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v6 >= 0x540
        || entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v6].mKey
        || (mObject = EntityHandleDb::sInst.mElements[v6].mObject) == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 1068;
        AeAssert::gCurrentExpr = "e";
        if (!AeAssert::IsIgnored())
        {
            const char* v8 = script.mBlock != nullptr
                                 ? (const char*)(script.mBlock + 1)
                                 : defaultFileName;
            if (AeAssert::Assert(
                    "EffectEventPlayDir - passed in entity is NULL (scriptid=%s)",
                    v8))
                __debugbreak();
        }
        return -1;
    }
    if (mObject->snd_wait.notifyHash.mHash != 0)
        return -1;
    if (notifyHash != 0)
        RegisterEffectWait(mObject, notifyHash);
    const char* v9 = script.mBlock != nullptr
                         ? (const char*)(script.mBlock + 1)
                         : defaultFileName;
    return PostEffectEventScriptCall_Dir(mObject, v9, &dir.x, false).mVal;
}

// ea: 0x005CB140
int BrocSys::EffectEventQueue(unsigned int entityHandleVal,
                              const Broc::string& script, int notifyHash,
                              bool stoppable, bool important)
{
    (void)stoppable;
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 1091;
        AeAssert::gCurrentExpr = "!script.is_empty()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("EffectEventQueue - passed in script is NULL"))
            __debugbreak();
    }
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
        return -1;
    unsigned int v7 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v7 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v7].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v7].mObject) != nullptr)
    {
        if (notifyHash != 0)
            RegisterEffectWait(mObject, notifyHash);
        const char* v10 = script.mBlock != nullptr
                              ? (const char*)(script.mBlock + 1)
                              : defaultFileName;
        return PostEffectEventScriptCall(mObject, v10, true, PAK_ID_INVALID,
                                         important)
            .mVal;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 1096;
        AeAssert::gCurrentExpr = "e";
        if (!AeAssert::IsIgnored())
        {
            const char* v9 = script.mBlock != nullptr
                                 ? (const char*)(script.mBlock + 1)
                                 : defaultFileName;
            if (AeAssert::Assert(
                    "EffectEventQueue - passed in entity is NULL (scriptid=%s)",
                    v9))
                __debugbreak();
        }
        return -1;
    }
}

// ea: 0x005CB280
int BrocSys::EffectEventQueueDialog(unsigned int entityHandleVal,
                                    const Broc::string& script,
                                    int notifyHash, bool bUnused)
{
    (void)bUnused;
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 1121;
        AeAssert::gCurrentExpr = "!script.is_empty()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "EffectEventQueueDialog - passed in script is NULL"))
            __debugbreak();
    }
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
        return -1;
    const char* v5 = (const char*)(script.mBlock + 1);
    unsigned int v7 = entityHandleVal & 0xFFF;
    if (v7 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v7].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v7].mObject;
        if (mObject != nullptr)
            return PostEffectEventQueueDialog(mObject, v5, notifyHash).mVal;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
    AeAssert::gCurrentLine = 1126;
    AeAssert::gCurrentExpr = "e";
    if (!AeAssert::IsIgnored())
    {
        const char* v9 = script.mBlock != nullptr
                             ? (const char*)(script.mBlock + 1)
                             : defaultFileName;
        if (AeAssert::Assert(
                "EffectEventQueueDialog - passed in entity is NULL (scriptid=%s)",
                v9))
            __debugbreak();
    }
    return -1;
}

// ea: 0x005CB390
int BrocSys::DialogPlay(unsigned int entityHandleVal,
                        const Broc::string& script, int notifyHash,
                        bool stopPrevIfPlaying)
{
    unsigned int v5 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v5 >= 0x540
        || entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v5].mKey
        || (mObject = EntityHandleDb::sInst.mElements[v5].mObject) == nullptr)
    {
        EntityManager::sInst->mWorld->Notify(notifyHash);
        return -1;
    }
    if (script.mBlock == nullptr
        || script.mBlock == (Broc::string::Block*)-12
        || *((const char*)(script.mBlock + 1)) == 0)
    {
        mObject->Notify(notifyHash);
        return -1;
    }
    if (mObject->snd_wait.notifyHash.mHash != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 1274;
        AeAssert::gCurrentExpr = "e->snd_wait.notifyHash.GetHash()==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "DialogPlay - passed in entity is already got a\nsound notify on it"))
            __debugbreak();
        if (mObject->snd_wait.notifyHash.mHash != 0 && !allowOverLapping
            && !stopPrevIfPlaying)
        {
            mObject->Notify(notifyHash);
            return -1;
        }
    }
    const char* v8 = script.mBlock != nullptr
                         ? (const char*)(script.mBlock + 1)
                         : defaultFileName;
    nslWaveID Wave = SoundDevice::sInst->FindWave(v8);
    if (Wave == NSL_WAVE_ID_INVALID)
    {
        mObject->Notify(notifyHash);
        return -1;
    }
    math::Position3 pos = mObject->r.currentOrigin;
    math::Dir3 v10;
    v10.v = _mm_setzero_ps();
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> sound =
        SoundDevice::sInst->PlaySound(
            Wave, DbLinkedHandle<EntityHandleDb, Entity>(entityHandleVal), true,
            false, pos, v10, -1.0f, -1.0f, -1.0f, -1.0f);
    if (*sound == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)1;  // ARO
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 1292;
        AeAssert::gCurrentExpr = "*sound";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SoundDevice::play_sound failed"))
            __debugbreak();
    }
    if (*sound != nullptr)
    {
        if (*sound != nullptr && notifyHash != 0)
            sound->mDialogNotify.mHash = (unsigned int)notifyHash;
        return 1;
    }
    mObject->Notify(notifyHash);
    return -1;
}

// ea: 0x005CB5C0
int BrocSys::EntityExists(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    return v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && EntityHandleDb::sInst.mElements[v1].mObject != nullptr;
}

// ea: 0x005CB600
int BrocSys::EntityIsAlive(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr)
        {
            actor_s* actor = mObject->actor;
            if (actor != nullptr)
            {
                if (actor->Physics.bIsAlive)
                    return 1;
            }
            else if (mObject->health > 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

// ea: 0x005CB660
int BrocSys::EntityIsWounded(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    actor_s* actor;
    return v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr
        && (actor = mObject->actor) != nullptr
        && actor->eState[actor->iStateLevel] == AIS_WOUNDED;
}

// ea: 0x005CB6B0
int BrocSys::EntityIsPlayer(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    int result = false;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr && mObject->client != nullptr)
            return true;
    }
    return result;
}

// ea: 0x005CB700
int BrocSys::EntityIsAI(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    int result = false;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr && mObject->actor != nullptr)
            return true;
    }
    return result;
}

// ea: 0x005CB750
int BrocSys::EntityIsSentient(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    int result = false;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr && mObject->sentient != nullptr)
            return true;
    }
    return result;
}

// ea: 0x005CB7A0
int BrocSys::EntityIsVehicle(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    int result = false;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr && mObject->scr_vehicle != nullptr)
            return true;
    }
    return result;
}

// ea: 0x005CB7F0
int BrocSys::EntityIsVehicleTank(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    int result = false;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr && mObject->scr_vehicle != nullptr
            && IsVehicleTank(mObject))
            return true;
    }
    return result;
}

// ea: 0x005CC050
void BrocSys::StopAnimScripted(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr)
        {
            actor_s* actor = mObject->actor;
            if (actor != nullptr
                && actor->eSimulatedState[actor->iSimulatedStateLevel]
                       == AIS_SCRIPTEDANIM)
                nullsub_59(actor);
            animscripted_t* scripted = mObject->scripted;
            if (scripted != nullptr)
            {
                if (scripted->anim != 0)
                {
                    XAnimTree* EntAnimTree = GScr_GetEntAnimTree(mObject);
                    if (EntAnimTree == nullptr)
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\BrocEntity.cpp";
                        AeAssert::gCurrentLine = 1733;
                        AeAssert::gCurrentExpr = "pAnimTree";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                    mObject->flags &= ~0x1000000u;
                    if (scripted->fBlendOutTime <= 0.0f)
                    {
                        XAnimSetCompleteGoalWeight(EntAnimTree, scripted->anim,
                                                   1.0f, 0.0f, 1.0f, 0, 0, 0);
                        XAnimSetCompleteGoalWeight(EntAnimTree, scripted->anim,
                                                   0.0f, 0.0f, 1.0f, 0, 0, 0);
                    }
                    else
                    {
                        XAnimSetCompleteGoalWeight(
                            EntAnimTree, scripted->anim, 0.0f,
                            scripted->fBlendOutTime, 1.0f, 0, 0, 0);
                    }
                }
                mem_heap_free(scripted);
                mObject->scripted = nullptr;
            }
        }
    }
}

// ea: 0x005CC1A0
void BrocSys::StartBlankState(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr)
        {
            actor_s* actor = mObject->actor;
            if (actor != nullptr)
            {
                nullsub_65(mObject->actor);
                nullsub_106(actor, AIS_BLANK);
            }
        }
    }
}

// ea: 0x005CC1F0
void BrocSys::StopBlankState(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr)
        {
            actor_s* actor = mObject->actor;
            if (actor != nullptr
                && actor->eSimulatedState[actor->iSimulatedStateLevel]
                       == AIS_BLANK)
            {
                nullsub_106(actor, AIS_EXPOSED);
            }
            else
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 1798;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
        }
    }
}

// ============================================================================
// scr.o batch 45 - vehicle in/out / follow / attach
// ============================================================================

// ea: 0x005CC290
void BrocSys::ResetAnimVariationChunkState(unsigned int entityHandleVal,
                                           unsigned int broanim)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
        if (mObject != nullptr)
        {
            if (broanim == 0xFFFF)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 1831;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning("anim index is -1"))
                    __debugbreak();
            }
            else
            {
                if ((broanim >> 16) > (unsigned int)BrocHelper::m_treeCount)
                    Scr_Error(
                        "AnimTree is a bad value. Is greater than # of trees loaded. Check input parameters for proper notation (i.e. @ symbol)");
                XAnimTree* EntAnimTree = GScr_GetEntAnimTree(mObject);
                AnimBank* Bank = AnimBankManager::sInst->GetBank(PAK_ID_MIN);
                if (&Bank->anims[broanim >> 16]
                    != *(AnimTree**)((char*)EntAnimTree + 8))
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\BrocEntity.cpp";
                    AeAssert::gCurrentLine = 1845;
                    AeAssert::gCurrentExpr = nullptr;
                    if (AeAssert::Error(
                            "Tree Animation mismatch. Out of range memory access possible!!"))
                        __debugbreak();
                }
                gDebugEntity = mObject;
                XAnimResetAnimVariationChunkState(EntAnimTree, broanim);
            }
        }
    }
}

// ea: 0x005CC3B0
void BrocSys::GetInVehicle1(unsigned int entityHandleVal,
                            unsigned int targetEntityHandleVal, bool canDrive,
                            bool canGunner, bool lock)
{
    (void)canDrive; (void)canGunner; (void)lock;
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* v3;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (v3 = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        if (v3->actor != nullptr)
        {
            unsigned int v4 = targetEntityHandleVal & 0xFFF;
            Entity* mObject;
            if (v4 < 0x540
                && targetEntityHandleVal >> 12
                       == EntityHandleDb::sInst.mElements[v4].mKey
                && (mObject = EntityHandleDb::sInst.mElements[v4].mObject)
                       != nullptr)
            {
                if (mObject->scr_vehicle != nullptr)
                {
                    if (G_GetVehicleInfo(mObject) == nullptr)
                        Scr_Error("Target doesnt have vehicle info...");
                }
                else
                {
                    Scr_Error("Target is not a vehicle...");
                }
            }
            else
            {
                Scr_Error("Bad target entity handle");
            }
        }
        else
        {
            Scr_Error("Entity ain't no actor yo!!!");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad self entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CC4C0
void BrocSys::GetInVehicle2(unsigned int entityHandleVal,
                            unsigned int targetEntityHandleVal,
                            unsigned int desirePos, bool lock)
{
    (void)desirePos; (void)lock;
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        actor_s* actor = mObject->actor;
        if (actor != nullptr)
        {
            unsigned int mVal = actor->hVehicle.mHandle.mVal;
            unsigned int v6 = mVal & 0xFFF;
            if (v6 >= 0x540
                || mVal >> 12 != EntityHandleDb::sInst.mElements[v6].mKey
                || EntityHandleDb::sInst.mElements[v6].mObject == nullptr)
            {
                entityHandleVal = targetEntityHandleVal;
                Entity* v7 = *DbLinkedHandle<EntityHandleDb, Entity>(
                    entityHandleVal);
                if (v7 != nullptr)
                {
                    if (v7->scr_vehicle != nullptr)
                    {
                        if (G_GetVehicleInfo(v7) == nullptr)
                            Scr_Error("Target doesnt have vehicle info...");
                    }
                    else
                    {
                        Scr_Error("Target is not a vehicle...");
                    }
                }
                else
                {
                    Scr_Error("Bad target entity handle");
                }
            }
        }
        else
        {
            Scr_Error("Entity ain't no actor yo!!!");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad self entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CC5E0
void BrocSys::GetOutVehicle(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        actor_s* actor = mObject->actor;
        if (actor != nullptr)
        {
            actor->bVehicleSeatExitRequest = 1;
        }
        else
        {
            Scr_Error("Entity ain't no actor yo!!!");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad self entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CC680
void BrocSys::SceneGetOutVehicle(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        actor_s* actor = mObject->actor;
        if (actor != nullptr)
        {
            if ((mObject->s.eFlags & 0x100000) == 0)
                G_EntUnlink(mObject);
            actor->bVehicleSeatImmediate = 1;
        }
        else
        {
            Scr_Error("Entity ain't no actor yo!!!");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad self entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CC730
void BrocSys::StartInVehicle1(unsigned int entityHandleVal,
                              unsigned int targetEntityHandleVal,
                              bool canDrive, bool canGunner, bool lock)
{
    BrocSys::GetInVehicle1(entityHandleVal, targetEntityHandleVal, canDrive,
                           canGunner, lock);
    unsigned int v5 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v5 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v5].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v5].mObject) != nullptr)
    {
        actor_s* actor = mObject->actor;
        if (actor != nullptr)
            actor->bVehicleSeatImmediate = 1;
        else
            Scr_Error("Entity ain't no actor yo!!!");
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad self entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CC7E0
void BrocSys::StartInVehicle2(unsigned int entityHandleVal,
                              unsigned int targetEntityHandleVal,
                              unsigned int desirePos, bool lock)
{
    BrocSys::GetInVehicle2(entityHandleVal, targetEntityHandleVal, desirePos,
                           lock);
    unsigned int v4 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v4 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v4].mObject) != nullptr)
    {
        actor_s* actor = mObject->actor;
        if (actor != nullptr)
            actor->bVehicleSeatImmediate = true;
        else
            Scr_Error("Entity ain't no actor yo!!!");
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad self entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CCC50
void BrocSys::StartFollowBehavior(unsigned int entityHandleVal,
                                  unsigned int targetEntityHandleVal)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* v3;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (v3 = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        actor_s* actor = v3->actor;
        if (actor != nullptr)
        {
            unsigned int v5 = targetEntityHandleVal & 0xFFF;
            Entity* mObject;
            if (v5 < 0x540
                && targetEntityHandleVal >> 12
                       == EntityHandleDb::sInst.mElements[v5].mKey
                && (mObject = EntityHandleDb::sInst.mElements[v5].mObject)
                       != nullptr)
            {
                if (mObject->scr_vehicle != nullptr)
                {
                    actor->pFollowTarget = mObject;
                    if (VEH_AcquirePlayerFollowSlot(mObject, v3))
                    {
                        nullsub_106(actor, AIS_FOLLOW);
                    }
                    else
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\BrocEntity.cpp";
                        AeAssert::gCurrentLine = 2177;
                        AeAssert::gCurrentExpr = "0";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                }
                else
                {
                    Scr_Error("Target is not a vehicle...");
                }
            }
            else
            {
                Scr_Error("Bad target entity handle");
            }
        }
        else
        {
            Scr_Error("Entity ain't no actor yo!!!");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad self entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CCDB0
void BrocSys::StopFollowBehavior(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        actor_s* actor = mObject->actor;
        if (actor != nullptr)
            nullsub_106(actor, AIS_SETABLE_FIRST);
        else
            Scr_Error("Entity ain't no actor yo!!!");
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad self entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CCE50
void BrocSys::SetFollowFormationData(unsigned int targetEntityHandleVal,
                                     unsigned int numColumns,
                                     float columnSpacing, float rowSpacing,
                                     float minFollowDistance)
{
    unsigned int v5 = targetEntityHandleVal & 0xFFF;
    Entity* mObject;
    if (v5 < 0x540
        && targetEntityHandleVal >> 12
               == EntityHandleDb::sInst.mElements[v5].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v5].mObject) != nullptr)
    {
        scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
        if (scr_vehicle != nullptr)
        {
            if (scr_vehicle->follow == nullptr)
            {
                TPakId mPakId = (TPakId)mObject->mPakId;
                if (mPakId == PAK_ID_INVALID)
                    mPakId = CurPakId();
                vehicle_follow* v9 = (vehicle_follow*)PakManager::sInst
                                         ->MemAlloc(mPakId, 0x418u, false);
                vehicle_follow* v10;
                if (v9 != nullptr)
                    v10 = new (v9) vehicle_follow();
                else
                    v10 = nullptr;
                mObject->scr_vehicle->follow = v10;
            }
            mObject->scr_vehicle->follow->columns = (int)numColumns;
            mObject->scr_vehicle->follow->columnSpacing = columnSpacing;
            mObject->scr_vehicle->follow->rowSpacing = rowSpacing;
            mObject->scr_vehicle->follow->minFollowDistance =
                minFollowDistance;
            VEH_FillFollowHistoryBuffer(mObject->scr_vehicle);
        }
        else
        {
            Scr_Error("Target is not a vehicle...");
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad target entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CCFB0
void BrocSys::ResetVehicleFollowPositionHistoryData(
    unsigned int targetEntityHandleVal)
{
    unsigned int v1 = targetEntityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && targetEntityHandleVal >> 12
               == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
        if (scr_vehicle != nullptr)
            VEH_FillFollowHistoryBuffer(scr_vehicle);
        else
            Scr_Error("Target is not a vehicle...");
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 16;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "Bad target entity handle"))
            __debugbreak();
    }
}

// ea: 0x005CD050
void BrocSys::Attach1(unsigned int entityHandleVal,
                      const Broc::string& modelName,
                      const Broc::string& tagName, bool ignoreCollision,
                      TPakInfo modelpak)
{
    unsigned int v5 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v5 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v5].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v5].mObject) != nullptr)
    {
        TPakId mPakId;
        if (modelpak != kTPakInfoInvalid)
            mPakId = PakInfoToPakId(modelpak);
        else
        {
            mPakId = (TPakId)mObject->mPakId;
            if (mPakId == PAK_ID_INVALID)
                mPakId = CurPakId();
        }
        TPakId pakId = mPakId;
        if (modelName.mBlock != nullptr && tagName.mBlock != nullptr)
        {
            if (G_EntDetach(mObject,
                            (const char*)(modelName.mBlock + 1),
                            (const char*)(tagName.mBlock + 1)) != 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2298;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored())
                {
                    const char* v8 = tagName.mBlock != nullptr
                                         ? (const char*)(tagName.mBlock + 1)
                                         : defaultFileName;
                    const char* v9 = modelName.mBlock != nullptr
                                         ? (const char*)(modelName.mBlock + 1)
                                         : defaultFileName;
                    if (AeAssert::Warning(
                            "model '%s' already attached to tag '%s'", v9,
                            v8))
                        __debugbreak();
                }
            }
            const char* v11 = tagName.mBlock != nullptr
                                  ? (const char*)(tagName.mBlock + 1)
                                  : defaultFileName;
            const char* v12 = modelName.mBlock != nullptr
                                  ? (const char*)(modelName.mBlock + 1)
                                  : defaultFileName;
            if (G_EntAttach(mObject, v12, v11, ignoreCollision, pakId) == 0)
                Scr_Error("maximum attached models exceeded");
        }
        else
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 2292;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning(
                       "model is null, or tag was null.  No attachment possible."))
                __debugbreak();
        }
    }
}

// ea: 0x005CD1D0
void BrocSys::Attach2(unsigned int entityHandleVal,
                      const Broc::string& modelName,
                      const Broc::string& tagName, TPakInfo modelpak)
{
    BrocSys::Attach1(entityHandleVal, modelName, tagName, false, modelpak);
}

// ea: 0x005CD1F0
void BrocSys::Attach3(unsigned int entityHandleVal,
                      const Broc::string& modelName)
{
    Broc::string empty(defaultFileName);
    BrocSys::Attach1(entityHandleVal, modelName, empty, false,
                     kTPakInfoInvalid);
}

// ea: 0x005CD260
void BrocSys::Detach1(unsigned int entityHandleVal,
                      const Broc::string& modelName,
                      const Broc::string& tagName)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v3 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v3].mObject) != nullptr)
    {
        const char* v5 = modelName.mBlock != nullptr
                             ? (const char*)(modelName.mBlock + 1)
                             : defaultFileName;
        if (_stricmp(v5, "none") != 0)
        {
            const char* v6 = tagName.mBlock != nullptr
                                 ? (const char*)(tagName.mBlock + 1)
                                 : defaultFileName;
            const char* v7 = modelName.mBlock != nullptr
                                 ? (const char*)(modelName.mBlock + 1)
                                 : defaultFileName;
            if (G_EntDetach(mObject, v7, v6) == 0)
            {
                Com_Printf("Current attachments:\n");
                Broc::string* p_mTag = &mObject->mAttachModels[0].mTag;
                for (int i = 7; i != 0; --i)
                {
                    ValidatePakId((TPakId)(uintptr_t)p_mTag[-1].mBlock);
                    if (p_mTag[-2].mBlock != nullptr)
                    {
                        Broc::string::Block* mBlock = p_mTag->mBlock;
                        if (p_mTag->mBlock != nullptr)
                        {
                            const char* v11 = (const char*)(mBlock + 1);
                            if (mBlock != (Broc::string::Block*)-12
                                && *v11 != 0)
                            {
                                ValidatePakId(
                                    (TPakId)(uintptr_t)p_mTag[-1].mBlock);
                                Com_Printf(
                                    "model: '%s', tag: '%s'\n",
                                    *(char**)((char*)p_mTag[-2].mBlock + 0x18),
                                    v11);
                            }
                        }
                    }
                    p_mTag += 3;
                }
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2341;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored())
                {
                    const char* v12 = tagName.mBlock != nullptr
                                          ? (const char*)(tagName.mBlock + 1)
                                          : defaultFileName;
                    const char* v13 = modelName.mBlock != nullptr
                                          ? (const char*)(modelName.mBlock + 1)
                                          : defaultFileName;
                    if (AeAssert::Warning(
                            "failed to detach model '%s' from tag '%s'", v13,
                            v12))
                        __debugbreak();
                }
            }
        }
    }
}

// ea: 0x005CD3D0
void BrocSys::Detach2(unsigned int entityHandleVal,
                      const Broc::string& modelName)
{
    Broc::string empty(defaultFileName);
    BrocSys::Detach1(entityHandleVal, modelName, empty);
}

// ea: 0x005CD430
void BrocSys::DetachAll(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
        G_EntDetachAll(mObject);
}

// ea: 0x005CD470
int BrocSys::GetAttachSize(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 >= 0x540)
        return 0;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v1].mKey)
        return 0;
    Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
    if (mObject == nullptr)
        return 0;
    int v4 = 0;
    AttachModelInfo* mAttachModels = mObject->mAttachModels;
    do
    {
        ValidatePakId((TPakId)mAttachModels->mModel.mPakId);
        if (mAttachModels->mModel.mValue == nullptr)
            break;
        ++v4;
        ++mAttachModels;
    }
    while (v4 < 7);
    return v4;
}

// ============================================================================
// scr.o batch 46 - attach getters / link / spawn
// ============================================================================

// ea: 0x005CD4E0
void BrocSys::GetAttachModelName(unsigned int entityHandleVal,
                                 Broc::string& outStr, int i)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    if (v3 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v3].mObject;
        if (mObject != nullptr)
        {
            if (i >= 7
                || (ValidatePakId(
                        (TPakId)mObject->mAttachModels[i].mModel.mPakId),
                    mObject->mAttachModels[i].mModel.mValue == nullptr))
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
                AeAssert::gCurrentLine = 23;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning("\x15%s", "bad index"))
                    __debugbreak();
            }
            ValidatePakId((TPakId)mObject->mAttachModels[i].mModel.mPakId);
            outStr = mObject->mAttachModels[i].mModel.mValue->name.mStr;
        }
    }
}

// ea: 0x005CD5C0
void BrocSys::GetAttachTagName(unsigned int entityHandleVal,
                               Broc::string& outStr, int i)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    if (v3 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v3].mObject;
        if (mObject != nullptr)
        {
            if (i >= 7
                || (ValidatePakId(
                        (TPakId)mObject->mAttachModels[i].mModel.mPakId),
                    mObject->mAttachModels[i].mModel.mValue == nullptr))
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
                AeAssert::gCurrentLine = 23;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning("\x15%s",
                                         "invalid attach model name or index"))
                    __debugbreak();
            }
            Broc::string::Block* mBlock =
                mObject->mAttachModels[i].mTag.mBlock;
            Broc::string* p_mTag = &mObject->mAttachModels[i].mTag;
            if (mBlock != nullptr)
            {
                Broc::string::Block* v7 = mBlock + 1;
                if (v7 != nullptr && *((const char*)v7) != 0)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\BrocEntity.cpp";
                    AeAssert::gCurrentLine = 2397;
                    AeAssert::gCurrentExpr =
                        "pEnt->mAttachModels[i].mTag.is_empty()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
            }
            outStr = *p_mTag;
        }
    }
}

// ea: 0x005CD6E0
bool BrocSys::GetAttachIgnoreCollision(unsigned int entityHandleVal, int i)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    if (v2 >= 0x540)
        return false;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v2].mKey)
        return false;
    Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
    if (mObject == nullptr)
        return false;
    if (i >= 7
        || (ValidatePakId((TPakId)mObject->mAttachModels[i].mModel.mPakId),
            mObject->mAttachModels[i].mModel.mValue == nullptr))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
        AeAssert::gCurrentLine = 23;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("\x15%s", "invalid attach model name or index"))
            __debugbreak();
    }
    return ((1 << i) & mObject->attachIgnoreCollision) != 0;
}

// ea: 0x005CD7A0
void BrocSys::LinkTo1(unsigned int entityHandleVal,
                      unsigned int parentEntityHandleVal,
                      const Broc::string& tagName,
                      const Broc::vector& originOffset,
                      const Broc::vector& anglesOffset, bool useAngles)
{
    unsigned int v6 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v6 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v6].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v6].mObject) != nullptr)
    {
        unsigned int v8 = parentEntityHandleVal & 0xFFF;
        Entity* v9;
        if (v8 < 0x540
            && parentEntityHandleVal >> 12
                   == EntityHandleDb::sInst.mElements[v8].mKey
            && (v9 = EntityHandleDb::sInst.mElements[v8].mObject) != nullptr)
        {
            if ((mObject->flags & 0x8000) == 0)
            {
                char tmpstr[256];
                sprintf(tmpstr, "entity %s: does not support linkTo",
                        G_GetEntityTypeName(mObject));
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2430;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(tmpstr))
                    __debugbreak();
            }
            const char* v11 = tagName.mBlock != nullptr
                                  ? (const char*)(tagName.mBlock + 1)
                                  : defaultFileName;
            if (G_EntLinkToWithOffset(mObject, v9, v11, &originOffset.x,
                                      &anglesOffset.x, useAngles) == 0)
            {
                if (v9->mDObj != nullptr)
                {
                    ValidatePakId((TPakId)v9->mModel.mPakId);
                    if (v9->mModel.mValue == nullptr)
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\BrocEntity.cpp";
                        AeAssert::gCurrentLine = 2447;
                        AeAssert::gCurrentExpr = "pParent->mModel";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                    const char* v16 = tagName.mBlock != nullptr
                                          ? (const char*)(tagName.mBlock + 1)
                                          : defaultFileName;
                    unsigned int v17 = HashString::CalcHash(v16);
                    if (tagName.mBlock != nullptr
                        && SV_DObjGetBoneIndex(v9, v17) < 0)
                    {
                        SV_DObjDumpInfo(v9);
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\BrocEntity.cpp";
                        AeAssert::gCurrentLine = 2453;
                        AeAssert::gCurrentExpr = nullptr;
                        if (!AeAssert::IsIgnored())
                        {
                            XModel* v18 = v9->mModel.operator->();
                            if (AeAssert::Warning(
                                    "failed to link entity since tag '%s' does not exist in parent model '%s'",
                                    v16, v18->name.mStr))
                                __debugbreak();
                        }
                    }
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\BrocEntity.cpp";
                    AeAssert::gCurrentLine = 2456;
                    AeAssert::gCurrentExpr = nullptr;
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Warning(
                               "failed to link entity due to link cycle"))
                        __debugbreak();
                }
                else
                {
                    TPakId mPakId = (TPakId)v9->mModel.mPakId;
                    IVPointer<XModel>* p_mModel = &v9->mModel;
                    ValidatePakId(mPakId);
                    if (p_mModel->mValue == nullptr)
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\BrocEntity.cpp";
                        AeAssert::gCurrentLine = 2439;
                        AeAssert::gCurrentExpr = nullptr;
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Warning(
                                   "failed to link entity since parent has no model"))
                            __debugbreak();
                    }
                    else
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\BrocEntity.cpp";
                        AeAssert::gCurrentLine = 2443;
                        AeAssert::gCurrentExpr = nullptr;
                        if (!AeAssert::IsIgnored())
                        {
                            XModel* v15 = p_mModel->operator->();
                            if (AeAssert::Warning(
                                    "failed to link entity since parent model '%s' is invalid",
                                    v15->name.mStr))
                                __debugbreak();
                        }
                    }
                }
            }
        }
    }
}

// ea: 0x005CDA90
void BrocSys::LinkTo2(unsigned int entityHandleVal,
                      unsigned int parentEntityHandleVal,
                      const Broc::string& tagName)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v3 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v3].mObject) != nullptr)
    {
        unsigned int v5 = parentEntityHandleVal & 0xFFF;
        Entity* v6;
        if (v5 < 0x540
            && parentEntityHandleVal >> 12
                   == EntityHandleDb::sInst.mElements[v5].mKey
            && (v6 = EntityHandleDb::sInst.mElements[v5].mObject) != nullptr)
        {
            if ((mObject->flags & 0x8000) == 0)
            {
                char tmpstr[256];
                sprintf(tmpstr, "entity %s: does not support linkTo",
                        G_GetEntityTypeName(mObject));
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2476;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(tmpstr))
                    __debugbreak();
            }
            if (tagName.mBlock != nullptr)
                G_EntLinkTo(mObject, v6, (const char*)(tagName.mBlock + 1));
            else
                G_EntLinkTo(mObject, v6, defaultFileName);
        }
    }
}

// ea: 0x005CDBB0
void BrocSys::LinkTo3(unsigned int entityHandleVal,
                      unsigned int parentEntityHandleVal)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        unsigned int v4 = parentEntityHandleVal & 0xFFF;
        Entity* v5;
        if (v4 < 0x540
            && parentEntityHandleVal >> 12
                   == EntityHandleDb::sInst.mElements[v4].mKey
            && (v5 = EntityHandleDb::sInst.mElements[v4].mObject) != nullptr)
        {
            if ((mObject->flags & 0x8000) == 0)
            {
                char tmpstr[256];
                sprintf(tmpstr, "entity %s: does not support linkTo",
                        G_GetEntityTypeName(mObject));
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2499;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(tmpstr))
                    __debugbreak();
            }
            G_EntLinkTo(mObject, v5, defaultFileName);
        }
    }
}

// ea: 0x005CDCB0
void BrocSys::PlayerLinkTo1(unsigned int entityHandleVal,
                            unsigned int parentEntityHandleVal,
                            const Broc::string& tagName,
                            const Broc::vector& angleFrac)
{
    unsigned int v4 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v4 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v4].mObject) != nullptr)
    {
        unsigned int v6 = parentEntityHandleVal & 0xFFF;
        Entity* v7;
        if (v6 < 0x540
            && parentEntityHandleVal >> 12
                   == EntityHandleDb::sInst.mElements[v6].mKey
            && (v7 = EntityHandleDb::sInst.mElements[v6].mObject) != nullptr)
        {
            if (mObject->client == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2517;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("not a player entity"))
                    __debugbreak();
            }
            if ((mObject->flags & 0x8000) == 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2519;
                AeAssert::gCurrentExpr = "pEnt->flags & 0x00008000";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            mObject->client->linkAnglesFrac[0] = angleFrac.x;
            mObject->client->linkAnglesFrac[1] = angleFrac.y;
            mObject->client->linkAnglesFrac[2] = angleFrac.z;
            const char* v8 = tagName.mBlock != nullptr
                                 ? (const char*)(tagName.mBlock + 1)
                                 : defaultFileName;
            if (G_EntLinkTo(mObject, v7, v8) == 0)
                Scr_Error("failed to link entity due to link cycle");
        }
    }
}

// ea: 0x005CDE20
void BrocSys::PlayerLinkTo2(unsigned int entityHandleVal,
                            unsigned int parentEntityHandleVal,
                            const Broc::string& tagName)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v3 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v3].mObject) != nullptr)
    {
        unsigned int v5 = parentEntityHandleVal & 0xFFF;
        Entity* v6;
        if (v5 < 0x540
            && parentEntityHandleVal >> 12
                   == EntityHandleDb::sInst.mElements[v5].mKey
            && (v6 = EntityHandleDb::sInst.mElements[v5].mObject) != nullptr)
        {
            if (mObject->client == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2538;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("PlayerLinkTo2: not a player entity"))
                    __debugbreak();
            }
            if ((mObject->flags & 0x8000) == 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2540;
                AeAssert::gCurrentExpr = "pEnt->flags & 0x00008000";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            const char* v7 = tagName.mBlock != nullptr
                                 ? (const char*)(tagName.mBlock + 1)
                                 : defaultFileName;
            if (G_EntLinkTo(mObject, v6, v7) == 0)
                Scr_Error("failed to link entity due to link cycle");
        }
    }
}

// ea: 0x005CDF60
void BrocSys::PlayerLinkTo3(unsigned int entityHandleVal,
                            unsigned int parentEntityHandleVal)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        unsigned int v4 = parentEntityHandleVal & 0xFFF;
        Entity* v5;
        if (v4 < 0x540
            && parentEntityHandleVal >> 12
                   == EntityHandleDb::sInst.mElements[v4].mKey
            && (v5 = EntityHandleDb::sInst.mElements[v4].mObject) != nullptr)
        {
            if (mObject->client == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2558;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("PlayerLinkTo3: not a player entity"))
                    __debugbreak();
            }
            if ((mObject->flags & 0x8000) == 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
                AeAssert::gCurrentLine = 2560;
                AeAssert::gCurrentExpr = "pEnt->flags & 0x00008000";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            if (G_EntLinkTo(mObject, v5, defaultFileName) == 0)
                Scr_Error("failed to link entity due to link cycle");
        }
    }
}

// ea: 0x005CE090
void BrocSys::UnLink(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        if ((mObject->s.eFlags & 0x100000) != 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 2573;
            AeAssert::gCurrentExpr = "!(pEnt->s.eFlags & (1<<20))";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                       "Can't UnLink entity that is using vehicle. Call UseBy."))
                __debugbreak();
        }
        if ((mObject->s.eFlags & 0x100000) == 0)
            G_EntUnlink(mObject);
    }
}

// ea: 0x005CE120
void BrocSys::EnableLinkTo(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        if ((mObject->flags & 0x8000) != 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 2587;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("entity already has linkTo enabled"))
                __debugbreak();
        }
        const char* classname = mObject->mClassName.mBlock != nullptr
                                    ? (const char*)(mObject->mClassName.mBlock
                                                    + 1)
                                    : defaultFileName;
        if (mObject->s.eType != 0 || mObject->physicsObject != 0
            || ((mObject->nextthink != 0 || mObject->think != THINK__NULL)
                && _stricmp(classname, "trigger_multiple") != 0))
        {
            char tmpstr[256];
            sprintf(tmpstr, "entity %s:does not support enableLinkTo",
                    G_GetEntityTypeName(mObject));
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 2593;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(tmpstr))
                __debugbreak();
        }
        if (mObject->client != nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 2596;
            AeAssert::gCurrentExpr = "!pEnt->client";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        int v6 = mObject->flags | 0x8000;
        mObject->nextthink = level.time;
        mObject->think = THINK__Think_SpawnNewAutoDoorTrigger;
        mObject->flags = v6;
    }
}

// ea: 0x005CE2E0
unsigned int BrocSys::DoSpawn1(unsigned int entityHandleVal,
                               const Broc::string& name, TPakInfo whichPak,
                               enumForceSpawn forceSpawn)
{
    unsigned int v4 = entityHandleVal & 0xFFF;
    if (v4 >= 0x540)
        return 0;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v4].mKey)
        return 0;
    Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
    if (mObject == nullptr)
        return 0;
    Broc::string targetname((Broc::string::Block*)nullptr);
    if (name.mBlock != nullptr)
        targetname = name;
    if (mObject->s.eType != 12)
    {
        const char* v7;
        if (mObject->targetname.mBlock == (Broc::string::Block*)-12)
            v7 = "<unnamed>";
        else
            v7 = mObject->targetname.mBlock != nullptr
                     ? (const char*)(mObject->targetname.mBlock + 1)
                     : defaultFileName;
        const char* v9 = mObject->mClassName.mBlock != nullptr
                             ? (const char*)(mObject->mClassName.mBlock + 1)
                             : defaultFileName;
        Scr_Error(
            va("dospawn can only be called on actor spawners\n"
               "attempted to call dospawn on entity with name '%s' of type '%s' at (%.0f %.0f %.0f)\n",
               v7, v9, mObject->r.currentOrigin.v.m128_f32[0],
               mObject->r.currentOrigin.v.m128_f32[1],
               mObject->r.currentOrigin.v.m128_f32[2]));
        return 0;
    }
    if (mObject->timestamp >= level.time)
        return 0;
    TPakId v12 = whichPak != kTPakInfoInvalid
                     ? PakInfoToPakId(whichPak)
                     : PAK_ID_INVALID;
    Entity* v13 = SpawnActor(mObject, targetname, forceSpawn, v12);
    if (v13 == nullptr)
        return 0;
    mObject->timestamp = level.time;
    return v13->mHandle.mHandle.mVal;
}

// ea: 0x005CE460
unsigned int BrocSys::DoSpawn2(unsigned int entityHandleVal, TPakInfo whichPak,
                               enumForceSpawn forceSpawn)
{
    Broc::string empty((Broc::string::Block*)nullptr);
    return BrocSys::DoSpawn1(entityHandleVal, empty, whichPak, forceSpawn);
}

// ea: 0x005CE4D0
unsigned int BrocSys::StalinGradSpawn1(unsigned int entityHandleVal,
                                       const Broc::string& name,
                                       TPakInfo whichPak)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    if (v3 >= 0x540)
        return 0;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v3].mKey)
        return 0;
    Entity* mObject = EntityHandleDb::sInst.mElements[v3].mObject;
    if (mObject == nullptr)
        return 0;
    Broc::string targetname((Broc::string::Block*)nullptr);
    if (name.mBlock != nullptr)
        targetname = name;
    else
        targetname = defaultFileName;
    if (mObject->s.eType != 12)
    {
        const char* v6;
        if (mObject->targetname.mBlock == (Broc::string::Block*)-12)
            v6 = "<unnamed>";
        else
            v6 = mObject->targetname.mBlock != nullptr
                     ? (const char*)(mObject->targetname.mBlock + 1)
                     : defaultFileName;
        const char* v8 = mObject->mClassName.mBlock != nullptr
                             ? (const char*)(mObject->mClassName.mBlock + 1)
                             : defaultFileName;
        Scr_Error(
            va("dospawn can only be called on actor spawners\n"
               "attempted to call dospawn on entity with name '%s' of type '%s' at (%.0f %.0f %.0f)\n",
               v6, v8, mObject->r.currentOrigin.v.m128_f32[0],
               mObject->r.currentOrigin.v.m128_f32[1],
               mObject->r.currentOrigin.v.m128_f32[2]));
        return 0;
    }
    if (mObject->timestamp >= level.time)
        return 0;
    TPakId v11 = whichPak != kTPakInfoInvalid
                     ? PakInfoToPakId(whichPak)
                     : PAK_ID_INVALID;
    Entity* v12 = SpawnActor(mObject, targetname, FORCE_SPAWN, v11);
    if (v12 == nullptr)
        return 0;
    mObject->timestamp = level.time;
    return v12->mHandle.mHandle.mVal;
}

// ea: 0x005CE650
unsigned int BrocSys::StalinGradSpawn2(unsigned int entityHandleVal,
                                       TPakInfo whichPak)
{
    Broc::string empty((Broc::string::Block*)nullptr);
    return BrocSys::StalinGradSpawn1(entityHandleVal, empty, whichPak);
}

// ea: 0x005CE6C0
void BrocSys::GetOrigin(unsigned int entityHandleVal, Broc::vector& outVec)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        Broc::vector origin;
        if (mObject->r.bmodel != nullptr)
        {
            origin.x =
                (mObject->r.absmax.v.m128_f32[0]
                 + mObject->r.absmin.v.m128_f32[0])
                * 0.5f;
            origin.y =
                (mObject->r.absmax.v.m128_f32[1]
                 + mObject->r.absmin.v.m128_f32[1])
                * 0.5f;
            origin.z =
                (mObject->r.absmax.v.m128_f32[2]
                 + mObject->r.absmin.v.m128_f32[2])
                * 0.5f;
        }
        else
        {
            origin.x = mObject->r.currentOrigin.v.m128_f32[0];
            origin.y = mObject->r.currentOrigin.v.m128_f32[1];
            origin.z = mObject->r.currentOrigin.v.m128_f32[2];
        }
        outVec = origin;
    }
}

// ============================================================================
// scr.o batch 47 - entity state / use / health wrappers
// ============================================================================

// ea: 0x005CE7A0
void BrocSys::GetEye(unsigned int entityHandleVal, Broc::vector& outVec)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        sentient_s* sentient = mObject->sentient;
        if (sentient != nullptr)
        {
            float eye[3];
            Sentient_GetEyePosition(sentient, eye);
            outVec.x = eye[0];
            outVec.y = eye[1];
            outVec.z = eye[2];
        }
        else
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 2765;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored())
            {
                const char* classname = mObject->mClassName.mBlock != nullptr
                                            ? (const char*)(mObject
                                                                ->mClassName
                                                                .mBlock
                                                            + 1)
                                            : defaultFileName;
                if (AeAssert::Warning(
                        "getEye must be called on an AI or player, not on a '%s'",
                        classname))
                    __debugbreak();
            }
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 2759;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("getEye being called on a NULL entityt"))
            __debugbreak();
    }
}

// ea: 0x005CE8B0
unsigned int BrocSys::UseBy(unsigned int entityHandleVal,
                            unsigned int otherEntityHandleVal)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    if (v2 >= 0x540)
        return 0;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v2].mKey)
        return 0;
    Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
    if (mObject == nullptr)
        return 0;
    unsigned int v5 = otherEntityHandleVal & 0xFFF;
    if (v5 >= 0x540)
        return 0;
    if (otherEntityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v5].mKey)
        return 0;
    Entity* v6 = EntityHandleDb::sInst.mElements[v5].mObject;
    if (v6 == nullptr)
        return 0;
    Scr_NotifyFromEnt(mObject, hash_const.trigger, v6);
    unsigned char use = mObject->use;
    if (use != 0)
    {
        if (use >= 0xE)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 2791;
            AeAssert::gCurrentExpr = "pEnt->use > 0 && pEnt->use < USE_MAX";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        usetable[mObject->use](mObject, v6, v6);
    }
    return v6->mHandle.mHandle.mVal;
}

// ea: 0x005CE9A0
bool BrocSys::IsTouching(unsigned int entityHandleVal,
                         unsigned int otherEntityHandleVal)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v3 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey)
        mObject = EntityHandleDb::sInst.mElements[v3].mObject;
    if (mObject == nullptr)
        return false;
    const Entity* v5;
    if (mObject->r.bmodel != nullptr)
    {
        v5 = mObject;
        unsigned int v6 = otherEntityHandleVal & 0xFFF;
        mObject = nullptr;
        if (v6 < 0x540
            && otherEntityHandleVal >> 12
                   == EntityHandleDb::sInst.mElements[v6].mKey)
            mObject = EntityHandleDb::sInst.mElements[v6].mObject;
        if (mObject == nullptr)
            return false;
    }
    else
    {
        unsigned int v8 = otherEntityHandleVal & 0xFFF;
        Entity* v9 = nullptr;
        if (v8 < 0x540
            && otherEntityHandleVal >> 12
                   == EntityHandleDb::sInst.mElements[v8].mKey)
            v9 = EntityHandleDb::sInst.mElements[v8].mObject;
        v5 = v9;
        if (v9 == nullptr)
            return false;
    }
    if (mObject->r.maxs.v.m128_f32[0] < mObject->r.mins.v.m128_f32[0])
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 2826;
        AeAssert::gCurrentExpr = "pEnt->r.maxs[0] >= pEnt->r.mins[0]";
        if (!AeAssert::IsIgnored())
        {
            const char* classname = mObject->mClassName.mBlock != nullptr
                                        ? (const char*)(mObject->mClassName
                                                            .mBlock
                                                        + 1)
                                        : defaultFileName;
            if (AeAssert::Assert(
                    va("entnum: %d, origin: %g %g %g, classname: %s",
                       mObject->mHandle.mHandle.mVal,
                       mObject->r.currentOrigin.v.m128_f32[0],
                       mObject->r.currentOrigin.v.m128_f32[1],
                       mObject->r.currentOrigin.v.m128_f32[2], classname)))
                __debugbreak();
        }
    }
    if (mObject->r.maxs.v.m128_f32[1] < mObject->r.mins.v.m128_f32[1])
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 2827;
        AeAssert::gCurrentExpr = "pEnt->r.maxs[1] >= pEnt->r.mins[1]";
        if (!AeAssert::IsIgnored())
        {
            const char* classname = mObject->mClassName.mBlock != nullptr
                                        ? (const char*)(mObject->mClassName
                                                            .mBlock
                                                        + 1)
                                        : defaultFileName;
            if (AeAssert::Assert(
                    va("entnum: %d, origin: %g %g %g, classname: %s",
                       mObject->mHandle.mHandle.mVal,
                       mObject->r.currentOrigin.v.m128_f32[0],
                       mObject->r.currentOrigin.v.m128_f32[1],
                       mObject->r.currentOrigin.v.m128_f32[2], classname)))
                __debugbreak();
        }
    }
    if (mObject->r.maxs.v.m128_f32[2] < mObject->r.mins.v.m128_f32[2])
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 2828;
        AeAssert::gCurrentExpr = "pEnt->r.maxs[2] >= pEnt->r.mins[2]";
        if (!AeAssert::IsIgnored())
        {
            const char* classname = mObject->mClassName.mBlock != nullptr
                                        ? (const char*)(mObject->mClassName
                                                            .mBlock
                                                        + 1)
                                        : defaultFileName;
            if (AeAssert::Assert(
                    va("entnum: %d, origin: %g %g %g, classname: %s",
                       mObject->mHandle.mHandle.mVal,
                       mObject->r.currentOrigin.v.m128_f32[0],
                       mObject->r.currentOrigin.v.m128_f32[1],
                       mObject->r.currentOrigin.v.m128_f32[2], classname)))
                __debugbreak();
        }
    }
    float v20 = mObject->r.mins.v.m128_f32[0]
                + mObject->r.currentOrigin.v.m128_f32[0];
    float v21 = mObject->r.mins.v.m128_f32[1]
                + mObject->r.currentOrigin.v.m128_f32[1];
    float v22 = mObject->r.currentOrigin.v.m128_f32[2]
                + mObject->r.mins.v.m128_f32[2];
    float v23 = mObject->r.currentOrigin.v.m128_f32[0]
                + mObject->r.maxs.v.m128_f32[0];
    float v24 = mObject->r.maxs.v.m128_f32[1]
                + mObject->r.currentOrigin.v.m128_f32[1];
    float v25 = mObject->r.maxs.v.m128_f32[2]
                + mObject->r.currentOrigin.v.m128_f32[2];
    float v34[4], vMins[4];
    v34[1] = v20;
    v34[2] = v21;
    v34[3] = v22;
    vMins[1] = v23;
    vMins[2] = v24;
    vMins[3] = v25;
    if (mObject->r.bmodel != nullptr && v5->r.bmodel != nullptr)
    {
        float v26 = v5->r.currentOrigin.v.m128_f32[1]
                    + v5->r.maxs.v.m128_f32[1];
        float v27 = v5->r.currentOrigin.v.m128_f32[2]
                    + v5->r.maxs.v.m128_f32[2];
        vMins[0] = v34[0] = v5->r.currentOrigin.v.m128_f32[0]
                            + v5->r.maxs.v.m128_f32[0];
        vMins[1] = v34[1] = v26;
        vMins[2] = v34[2] = v27;
        vMins[3] = v34[3] = 0.0f;
        float v32[4] = { v20, v21, v22, 0.0f };
        float v33[4] = { v23, v24, v25, 0.0f };
        // SSE AABB overlap: max(vMins - v33, v32 - v34) < 0 on all 3 axes
        return vMins[0] < v33[0] && v32[0] < v34[0]
            && vMins[1] < v33[1] && v32[1] < v34[1]
            && vMins[2] < v33[2] && v32[2] < v34[2];
    }
    if ((mObject->r.svFlags & 0x200) != 0)
    {
        math::Position3 maxs = native_to_cdl_pos3(&vMins[1]);
        math::Position3 mins = native_to_cdl_pos3(&v34[1]);
        return g_EntityContactCapsule(mins, maxs, v5) != 0;
    }
    math::Position3 maxs = native_to_cdl_pos3(&vMins[1]);
    math::Position3 mins = native_to_cdl_pos3(&v34[1]);
    return g_EntityContact(mins, maxs, v5) != 0;
}

// ea: 0x005CEE40
void BrocSys::LockDoor(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        unsigned int mHash = mObject->mClassNameHash.mHash;
        if (mHash == hash_const.func_door_rotating.mHash
            || mHash == hash_const.func_door.mHash)
        {
            mObject->key = 1;
            UpdatePaths(mObject);
        }
    }
}

// ea: 0x005CEEA0
void BrocSys::UnLockDoor(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        unsigned int mHash = mObject->mClassNameHash.mHash;
        if (mHash == hash_const.func_door_rotating.mHash
            || mHash == hash_const.func_door.mHash)
        {
            mObject->key = 0;
            UpdatePaths(mObject);
        }
    }
}

// ea: 0x005CEF00
bool BrocSys::IsDoorLocked(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    unsigned int mHash;
    return v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr
        && ((mHash = mObject->mClassNameHash.mHash)
                == hash_const.func_door_rotating.mHash
            || mHash == hash_const.func_door.mHash)
        && mObject->key != 0;
}

// ea: 0x005CEF60
void BrocSys::Delete(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        if (mObject->client != nullptr)
            Scr_Error("Cannot delete a client entity");
        Scr_Notify(mObject, hash_const.death, 0);
        G_FreeEntity(mObject, 0);
    }
}

// ea: 0x005CEFD0
void BrocSys::SetTransparent(unsigned int entityHandleVal, bool isTransparent)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        trRefEntity& RenderEntity = mObject->GetRenderEntity();
        if (isTransparent)
            RenderEntity.mAlpha = 0.5f;
        else
            RenderEntity.mAlpha = 1.0f;
    }
}

// ea: 0x005CF250
void BrocSys::SetAiType(unsigned int entityHandleVal,
                        const Broc::string& modelName, TPakInfo pakInfo)
{
    unsigned int v3 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v3 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v3].mObject) != nullptr)
    {
        if (modelName.mBlock == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 3032;
            AeAssert::gCurrentExpr = "modelName.IsDefined()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("SetModel: model name undefined."))
                __debugbreak();
        }
        if (modelName.mBlock != nullptr)
        {
            TPakId mPakId;
            if (pakInfo != kTPakInfoInvalid)
                mPakId = PakInfoToPakId(pakInfo);
            else
            {
                mPakId = (TPakId)mObject->mPakId;
                if (mPakId == PAK_ID_INVALID)
                    mPakId = CurPakId();
            }
            bool v6 = false;
            if (mObject->client != nullptr)
            {
                mPakId = CurPakId();
                v6 = (mObject->flags & 0x400000) != 0;
                StopPhysics(mObject);
                mObject->flags &= ~0x400000u;
            }
            const char* v7 = modelName.mBlock != nullptr
                                 ? (const char*)(modelName.mBlock + 1)
                                 : defaultFileName;
            IVPointer<AIType> ait =
                AITypeManager::sInst->GetAIType(mPakId, v7, 0);
            ValidatePakId((TPakId)ait.mPakId);
            if (ait.mValue != nullptr)
            {
                ValidatePakId((TPakId)ait.mPakId);
                ait.mValue->InitPlayer(mObject, mPakId);
            }
            mObject->s.brushmodel = 0;
            SV_SetBrushModel(mObject);
            G_DObjUpdate(mObject, false);
            if (v6)
                g_UnlinkEntity(mObject);
            else
                g_LinkEntity(mObject);
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 3028;
        AeAssert::gCurrentExpr = "pEnt";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Failed to set model on NULL entitiy."))
            __debugbreak();
    }
}

// ea: 0x005CF410
void BrocSys::SetModelIndex(unsigned int entityHandleVal, int iflIndex)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        G_SetModelIndex(mObject, iflIndex);
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
        AeAssert::gCurrentLine = 3090;
        AeAssert::gCurrentExpr = "pEnt";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Failed to set model index on NULL entitiy."))
            __debugbreak();
    }
}

// ea: 0x005CF4A0
float BrocSys::GetNormalHealth(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        Client* client = mObject->client;
        if (client != nullptr)
        {
            int entityHandleVala = mObject->health;
            if (entityHandleVala != 0)
                return (float)entityHandleVala / client->pers.maxHealth;
        }
        int entityHandleValb = mObject->maxHealth;
        if (entityHandleValb != 0)
            return (float)mObject->health / entityHandleValb;
        Com_Printf(
            "WARNING: GetNormalHealth called on entity with 0 maxHealth.\n");
    }
    return 0.0f;
}

// ea: 0x005CF520
void BrocSys::SetNormalHealth(unsigned int entityHandleVal,
                              float fNormalHealth)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    if (v2 >= 0x540)
        return;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v2].mKey)
        return;
    Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
    if (mObject == nullptr)
        return;
    if (fNormalHealth <= 1.0f)
    {
        if (fNormalHealth <= 0.0f)
        {
            const char* name;
            if (mObject->targetname.mBlock == (Broc::string::Block*)-12)
                name = "<not set>";
            else
                name = mObject->targetname.mBlock != nullptr
                           ? (const char*)(mObject->targetname.mBlock + 1)
                           : defaultFileName;
            Scr_Error(
                va("setNormalHealth must be greater than 0 (tried to set %g on ent %i, name %s)\n",
                   fNormalHealth, mObject->mHandle.mHandle.mVal, name));
        }
    }
    else
    {
        fNormalHealth = 1.0f;
    }
    Client* client = mObject->client;
    if (client != nullptr)
    {
        float maxHealth = client->pers.maxHealth;
        int v13 = (int)((maxHealth * fNormalHealth) + 0.5f);
        if (v13 < 1)
            v13 = 1;
        mObject->health = v13;
        return;
    }
    int v9 = mObject->maxHealth;
    if (v9 > 0)
    {
        int health = mObject->health;
        if (health <= 0)
        {
            const char* name;
            if (mObject->targetname.mBlock == (Broc::string::Block*)-12)
                name = "<not set>";
            else
                name = mObject->targetname.mBlock != nullptr
                           ? (const char*)(mObject->targetname.mBlock + 1)
                           : defaultFileName;
            G_DPrintf(
                "^2Cannot setNormalHealth on dead entities (health %i, max %i, ent %i, name %s)\n",
                health, v9, mObject->mHandle.mHandle.mVal, name);
        }
        float maxHealth = (float)mObject->maxHealth;
        int v13 = (int)((maxHealth * fNormalHealth) + 0.5f);
        if (v13 < 1)
            v13 = 1;
        mObject->health = v13;
        return;
    }
    const char* name;
    if (mObject->targetname.mBlock == (Broc::string::Block*)-12)
        name = "<not set>";
    else
        name = mObject->targetname.mBlock != nullptr
                   ? (const char*)(mObject->targetname.mBlock + 1)
                   : defaultFileName;
    Scr_Error(
        va("entity's max health must be greater than 0 to call setNormalHealth (ent %i, name %s)\n",
           mObject->mHandle.mHandle.mVal, name));
}

// ea: 0x005CF6B0
void BrocSys::DoDamage(unsigned int entityHandleVal, float damage,
                       const Broc::vector& vecIn, hitLocation_t hitLoc)
{
    unsigned int v4 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v4 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v4].mObject) != nullptr)
    {
        if (vecIn.x == sNaN && vecIn.y == sNaN && vecIn.z == sNaN)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;  // JRS
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\BrocEntity.cpp";
            AeAssert::gCurrentLine = 3191;
            AeAssert::gCurrentExpr = "vecIn.IsDefined()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Undefined direction passed to DoDamage"))
                __debugbreak();
        }
        float x = vecIn.x;
        float y = vecIn.y;
        float z = vecIn.z;
        float source[3] = { vecIn.x, y, z };
        float from[3];
        float v10;
        if (mObject->client != nullptr)
        {
            from[0] = mObject->client->ps.origin.v.m128_f32[0] - x;
            from[1] = mObject->client->ps.origin.v.m128_f32[1] - y;
            v10 = mObject->client->ps.origin.v.m128_f32[2];
        }
        else
        {
            from[0] = mObject->r.currentOrigin.v.m128_f32[0] - x;
            from[1] = mObject->r.currentOrigin.v.m128_f32[1] - y;
            v10 = mObject->r.currentOrigin.v.m128_f32[2];
        }
        from[2] = v10 - z;
        const float* v11;
        if (0.0f == VectorNormalize(from))
            v11 = nullptr;
        else
            v11 = from;
        G_Damage(mObject, nullptr, nullptr, v11, source, (int)damage, 0, 31,
                 hitLoc, -1);
    }
}

// ea: 0x005CF840
void BrocSys::SetTakeDamage(unsigned int entityHandleVal, int damage)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
        mObject->takedamage = damage;
}

// ea: 0x005CF880
void BrocSys::InvulnerableForTime(unsigned int entityHandleVal, float time)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
        mObject->invulnerability_timeout =
            (unsigned int)(level.time - (time * -1000.0f));
}

// ea: 0x005CF8D0
bool BrocSys::IsEntityInvulnerable(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    return v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr
        && level.time < (int)mObject->invulnerability_timeout;
}

// ea: 0x005CF920
void BrocSys::SetAlwaysRender(unsigned int entityHandleVal, int r)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
        mObject->SetAlwaysRender(r != 0);
}

// ea: 0x005CF960
void BrocSys::Show(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
        mObject->flags &= ~0x400u;
}

// ea: 0x005CF9A0
void BrocSys::Hide(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
        mObject->flags |= 0x400u;
}

// ea: 0x005CF9E0
int BrocSys::SetContents(unsigned int entityHandleVal, int contents)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    if (v2 >= 0x540)
        return 0;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v2].mKey)
        return 0;
    Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
    if (mObject == nullptr)
        return 0;
    int v5 = mObject->r.contents;
    mObject->r.contents = contents;
    g_LinkEntity(mObject);
    return v5;
}

// ============================================================================
// scr.o batch 48 - paths / turrets
// ============================================================================

// ea: 0x005CFA30
void BrocSys::DisConnectPaths(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr)
        {
            if (Path_IsDynamicBlockingEntity(
                    EntityHandleDb::sInst.mElements[v1].mObject))
            {
                PathNodeMgr::sInst->DisconnectPathsForEntity(mObject);
            }
            else
            {
                if (mObject->mClassNameHash.mHash
                    == hash_const.script_brushmodel.mHash)
                    Scr_Error(
                        "script_brushmodel must have DYNAMICPATH set to disconnect paths");
                const char* v4 = mObject->mClassName.mBlock != nullptr
                                     ? (const char*)(mObject->mClassName.mBlock
                                                     + 1)
                                     : defaultFileName;
                Scr_Error(va("entity of type '%s' cannot disconnect paths",
                             v4));
            }
        }
    }
}

// ea: 0x005CFAC0
void BrocSys::ConnectPaths(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
        if (mObject != nullptr)
        {
            if (Path_IsDynamicBlockingEntity(
                    EntityHandleDb::sInst.mElements[v1].mObject) != 0)
            {
                PathNodeMgr::sInst->ConnectPathsForEntity(mObject);
            }
            else
            {
                if (mObject->mClassNameHash.mHash
                    == hash_const.script_brushmodel.mHash)
                    Scr_Error(
                        "script_brushmodel must have DYNAMICPATH set to connect paths");
                const char* v4 = mObject->mClassName.mBlock != nullptr
                                     ? (const char*)(mObject->mClassName.mBlock
                                                     + 1)
                                     : defaultFileName;
                Scr_Error(va("entity of type '%s' cannot connect paths", v4));
            }
        }
    }
}

// ea: 0x005CFB50
void BrocSys::StartFiring(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        turretInfo_t* pTurretInfo = mObject->pTurretInfo;
        if (pTurretInfo != nullptr)
        {
            pTurretInfo->turret_flags |= 4u;
        }
        else
        {
            const char* v5 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v5));
        }
    }
}

// ea: 0x005CFBC0
void BrocSys::StopFiring(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        turretInfo_t* pTurretInfo = mObject->pTurretInfo;
        if (pTurretInfo != nullptr)
        {
            pTurretInfo->turret_flags &= ~4u;
        }
        else
        {
            const char* v5 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v5));
        }
    }
}

// ea: 0x005CFC30
void BrocSys::ShootTurret(unsigned int entityHandleVal,
                          unsigned int turretOwnerOverride)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        unsigned int v4 = turretOwnerOverride & 0xFFF;
        Entity* v5 = nullptr;
        if (v4 < 0x540
            && turretOwnerOverride >> 12
                   == EntityHandleDb::sInst.mElements[v4].mKey)
            v5 = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject->pTurretInfo != nullptr)
        {
            turret_shoot(mObject, v5);
        }
        else
        {
            const char* v7 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v7));
        }
    }
}

// ea: 0x005CFCD0
void BrocSys::SetMode(unsigned int entityHandleVal, int mode)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        turretInfo_t* pTurretInfo = mObject->pTurretInfo;
        if (pTurretInfo != nullptr)
        {
            if (mode == (int)hash_const.auto_ai.mHash)
            {
                pTurretInfo->turret_flags |= 3u;
            }
            else if (mode == (int)hash_const.manual.mHash)
            {
                pTurretInfo->turret_flags &= 0xFCu;
            }
            else if (mode == (int)hash_const.manual_ai.mHash)
            {
                pTurretInfo->turret_flags =
                    (pTurretInfo->turret_flags & 0xFFFC) | 1;
            }
            else if (mode == (int)hash_const.auto_nonai.mHash)
            {
                pTurretInfo->turret_flags =
                    (pTurretInfo->turret_flags & 0xFFFC) | 2;
            }
            else
            {
                Scr_Error("unknown mode");
            }
        }
        else
        {
            const char* v6 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v6));
        }
    }
}

// ea: 0x005CFDB0
unsigned int BrocSys::GetTurretOwner(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 >= 0x540)
        return 0;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v1].mKey)
        return 0;
    Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
    if (mObject == nullptr)
        return 0;
    if (mObject->pTurretInfo == nullptr)
    {
        const char* v4 = mObject->mClassName.mBlock != nullptr
                             ? (const char*)(mObject->mClassName.mBlock + 1)
                             : defaultFileName;
        Scr_Error(va("entity type '%s' is not a turret", v4));
        return 0;
    }
    if (mObject->active == 0)
        return 0;
    return mObject->r.mOwner.mHandle.mVal;
}

// ea: 0x005CFE30
unsigned int BrocSys::GetOwner(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        return mObject->r.mOwner.mHandle.mVal;
    }
    return 0;
}

// ea: 0x005CFE70
void BrocSys::SetOwner(unsigned int entityHandleVal,
                       unsigned int ownerHandleVal)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        unsigned int v4 = ownerHandleVal & 0xFFF;
        Entity* v5;
        if (v4 < 0x540
            && ownerHandleVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey
            && (v5 = EntityHandleDb::sInst.mElements[v4].mObject) != nullptr)
        {
            mObject->r.mOwner.mHandle.mVal = v5->mHandle.mHandle.mVal;
        }
        else
        {
            mObject->r.mOwner.mHandle.mVal = 0;
        }
    }
}

// ea: 0x005CFEF0
void BrocSys::SetTargetEntity(unsigned int entityHandleVal,
                              unsigned int otherEntityHandleVal)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        unsigned int v4 = otherEntityHandleVal & 0xFFF;
        Entity* v5;
        if (v4 < 0x540
            && otherEntityHandleVal >> 12
                   == EntityHandleDb::sInst.mElements[v4].mKey
            && (v5 = EntityHandleDb::sInst.mElements[v4].mObject) != nullptr)
        {
            turretInfo_t* pTurretInfo = mObject->pTurretInfo;
            if (pTurretInfo != nullptr)
            {
                pTurretInfo->manualTarget = v5;
            }
            else
            {
                const char* v8 = mObject->mClassName.mBlock != nullptr
                                     ? (const char*)(mObject->mClassName.mBlock
                                                     + 1)
                                     : defaultFileName;
                Scr_Error(va("entity type '%s' is not a turret", v8));
            }
        }
    }
}

// ea: 0x005CFF90
bool BrocSys::HasTargetEntity(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 >= 0x540)
        return false;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v1].mKey)
        return false;
    Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
    if (mObject == nullptr)
        return false;
    turretInfo_t* pTurretInfo = mObject->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        const char* v5 = mObject->mClassName.mBlock != nullptr
                             ? (const char*)(mObject->mClassName.mBlock + 1)
                             : defaultFileName;
        Scr_Error(va("entity type '%s' is not a turret", v5));
        return false;
    }
    return pTurretInfo->manualTarget != nullptr;
}

// ea: 0x005D0000
void BrocSys::ClearTargetEntity(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        turretInfo_t* pTurretInfo = mObject->pTurretInfo;
        if (pTurretInfo != nullptr)
        {
            pTurretInfo->manualTarget = nullptr;
        }
        else
        {
            const char* v5 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v5));
        }
    }
}

// ea: 0x005D0070
void BrocSys::SetTurretTeam(unsigned int entityHandleVal,
                            const Broc::string& pszTeam)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        if (mObject->pTurretInfo != nullptr)
        {
            const char* v7 = pszTeam.mBlock != nullptr
                                 ? (const char*)(pszTeam.mBlock + 1)
                                 : defaultFileName;
            if (_stricmp(v7, "axis") == 0)
            {
                mObject->pTurretInfo->eTeam = TEAM_AXIS;
            }
            else
            {
                const char* v8 = pszTeam.mBlock != nullptr
                                     ? (const char*)(pszTeam.mBlock + 1)
                                     : defaultFileName;
                if (_stricmp(v8, "allies") == 0)
                {
                    mObject->pTurretInfo->eTeam = TEAM_ALLIES;
                }
                else
                {
                    Scr_Error(
                        va("unknown team '%s', should be 'axis' or 'allies'\n",
                           pszTeam.mBlock));
                }
            }
        }
        else
        {
            const char* v5 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v5));
        }
    }
}

// ea: 0x005D0170
void BrocSys::MakeTurretUsable(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        if (mObject->pTurretInfo != nullptr)
        {
            mObject->pTurretInfo->turret_flags |= 0x1000u;
        }
        else
        {
            const char* v4 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v4));
        }
    }
}

// ea: 0x005D01E0
void BrocSys::MakeTurretUnusable(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v1 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v1].mObject) != nullptr)
    {
        if (mObject->pTurretInfo != nullptr)
        {
            mObject->pTurretInfo->turret_flags &= ~0x1000u;
        }
        else
        {
            const char* v4 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v4));
        }
    }
}

// ea: 0x005D0250
void BrocSys::SetTurretAccuracy(unsigned int entityHandleVal, float accuracy)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        turretInfo_t* pTurretInfo = mObject->pTurretInfo;
        if (pTurretInfo != nullptr)
        {
            pTurretInfo->accuracy = accuracy;
        }
        else
        {
            const char* v6 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v6));
        }
    }
}

// ea: 0x005D02C0
void BrocSys::SetTurretRange(unsigned int entityHandleVal, float range)
{
    unsigned int v2 = entityHandleVal & 0xFFF;
    Entity* mObject;
    if (v2 < 0x540
        && entityHandleVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        turretInfo_t* pTurretInfo = mObject->pTurretInfo;
        if (pTurretInfo != nullptr)
        {
            pTurretInfo->maxRangeSquared = range * range;
        }
        else
        {
            const char* v6 = mObject->mClassName.mBlock != nullptr
                                 ? (const char*)(mObject->mClassName.mBlock
                                                 + 1)
                                 : defaultFileName;
            Scr_Error(va("entity type '%s' is not a turret", v6));
        }
    }
}

// ea: 0x005D0340
float BrocSys::GetTurretRange(unsigned int entityHandleVal)
{
    unsigned int v1 = entityHandleVal & 0xFFF;
    if (v1 >= 0x540)
        return 0.0f;
    if (entityHandleVal >> 12 != EntityHandleDb::sInst.mElements[v1].mKey)
        return 0.0f;
    Entity* mObject = EntityHandleDb::sInst.mElements[v1].mObject;
    if (mObject == nullptr)
        return 0.0f;
    turretInfo_t* pTurretInfo = mObject->pTurretInfo;
    if (pTurretInfo == nullptr)
    {
        const char* v5 = mObject->mClassName.mBlock != nullptr
                             ? (const char*)(mObject->mClassName.mBlock + 1)
                             : defaultFileName;
        Scr_Error(va("entity type '%s' is not a turret", v5));
        return 0.0f;
    }
    return sqrtf(pTurretInfo->maxRangeSquared);
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
