// ============================================================================
// ctor_dtor.cpp - class ctors/dtors + inline object init (core.o ctor_dtor)
// ============================================================================

#include "game/core/core_systems.h"
#include "game/core/core_globals.h"
#include "core/PoolAllocator.h"
#include "aeps/apsEffect.h"

#include <string.h>

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT_IDX(idx, cap, line)                                         \
    do {                                                                   \
        if ((idx) < 0 || (idx) >= (cap)) {                                 \
            AeAssert::gCurrentAuthor = AeAssert::COD3;                     \
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";             \
            AeAssert::gCurrentLine = (line);                               \
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";        \
            if (!AeAssert::IsIgnored()                                     \
                && AeAssert::Assert("out of bounds"))                      \
                __debugbreak();                                            \
        }                                                                  \
    } while (0)

extern unsigned int AeHash(const char* str);
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
struct mem_heap;
extern void mem_heap_create(mem_heap* heap, void* start, void* end,
                            mem_heap* reserve);
extern PoolAllocator* ActiveEffectSet_sAllocator;  // 0x00F00E84
extern int dword_F6A290[4 * 0x322];
extern void CameraShake_StopCameraShake(void* self, void* pShake);
extern void* g_cameraShake;
extern void SoundDevice_ReleaseSound(void* sInst, void* s);
extern void* SoundDevice_sInst;
struct SoundHandleDbLocal {
    struct El {
        void* mObject;  // +0x00
        unsigned int mKey;  // +0x04
    };
    El mElements[512];
};
extern SoundHandleDbLocal SoundHandleDb_sInst;
extern void* controller_inst();
extern void controller_stop_all_rumble(void* self);
struct tlSystemCallbacks;
extern void tlSetSystemCallbacks(const tlSystemCallbacks* callbacks);
extern void* AssetBankSet_ctor(void* self);
extern void AssetBankSet_dtor(void* self);
extern void* InplaceAssetBankSet_ConfigStringBank_ctor(void* self);
extern void* InplaceAssetBankSet_StringTableBank_ctor(void* self);
extern void RemoveLight(void* light);

namespace EffectEventSysStatics {
extern EffectEventSys* sInst;
}

// Sentinel-list layout used by the notify/rumble dlist containers
struct RealDList {
    void* m_head;  // points at first node's m_next (or &m_end)
    void* m_end;   // null sentinel
    void* m_tail;  // points at last node's m_next (or &m_head)
    int   m_size;
};
struct DNode {
    DNode* m_prev;
    DNode* m_next;
};

static RealDList sEntityNotifySet;

class EntityHandleDb {
public:
    struct DbElement {
        Entity* mObject;  // +0x00
        int     mKey;     // +0x04
    };
    unsigned char _pad[0xA8];
    DbElement     mElements[0x540];
    static EntityHandleDb sInst;
};

// ============================================================================
// EntityNotify / EntityNotifySet
// ============================================================================

// core.o data (0xF00E28 / 0xF00E2C)
PoolAllocator* EntityNotify::sAllocator = nullptr;
PoolAllocator* EntityNotifySet::sAllocator = nullptr;

// ea: 0x004BDAA0
EntityNotify::EntityNotify(unsigned int hashStr,
                           DbLinkedHandle<EntityHandleDb, Entity> ent,
                           WaitTilOutput* param)
{
    mStr = hashStr;
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mOwner = ent;
    mParam = param;
}

// ea: 0x004B5650
EntityNotify::~EntityNotify()
{
    if (mParam != nullptr)
        delete mParam;
    mParam = nullptr;
}

// ea: 0x004C1D80
EntityNotifySet::EntityNotifySet(Entity* e)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mEnt.mHandle.mVal = e->mHandle.mHandle.mVal;
    RealDList* strings = (RealDList*)&mStrings;
    strings->m_end = nullptr;
    strings->m_head = &strings->m_end;
    strings->m_tail = &strings->m_head;
    strings->m_size = 0;
    RealDList* endOn = (RealDList*)&mEndOnList;
    endOn->m_end = nullptr;
    endOn->m_head = &endOn->m_end;
    endOn->m_tail = &endOn->m_head;
    endOn->m_size = 0;
    DNode* m_head = (DNode*)sEntityNotifySet.m_head;
    DNode* m_next = m_head != nullptr ? m_head->m_next : nullptr;
    if (m_next != nullptr)
    {
        while (m_head != (DNode*)this)
        {
            m_head = m_next;
            m_next = m_next->m_next;
            if (m_next == nullptr)
                goto LABEL_9;
        }
        if (m_dlist_node.mNext != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityNotifySet.cpp";
            AeAssert::gCurrentLine = 38;
            AeAssert::gCurrentExpr =
                "sEntityNotifySet.find( this ) == sEntityNotifySet.end()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("hmm?"))
                __debugbreak();
        }
    }
LABEL_9:
    m_dlist_node.mNext = (reserved_dlist<EntityNotifySet>::dlist_node*)&sEntityNotifySet.m_end;
    m_dlist_node.mPrev = (reserved_dlist<EntityNotifySet>::dlist_node*)sEntityNotifySet.m_tail;
    *(DNode**)sEntityNotifySet.m_tail = (DNode*)&m_dlist_node;
    ++sEntityNotifySet.m_size;
    sEntityNotifySet.m_tail = &m_dlist_node;
}

// ea: 0x004CEAE0
EntityNotifySet::~EntityNotifySet()
{
    // unlink from sEntityNotifySet
    DNode* node = (DNode*)&m_dlist_node;
    if (node->m_prev != nullptr)
        node->m_prev->m_next = node->m_next;
    else
        sEntityNotifySet.m_head = node->m_next;
    if (node->m_next != nullptr)
        node->m_next->m_prev = node->m_prev;
    else
        sEntityNotifySet.m_tail = node->m_prev;
    if (sEntityNotifySet.m_size > 0)
        --sEntityNotifySet.m_size;
    // destroy string nodes
    RealDList* strings = (RealDList*)&mStrings;
    DNode* cur = (DNode*)strings->m_head;
    while (cur != nullptr && cur != (DNode*)&strings->m_end)
    {
        DNode* next = cur->m_next;
        delete (EntityNotify*)cur;
        cur = next;
    }
    strings->m_head = &strings->m_end;
    strings->m_tail = &strings->m_head;
    strings->m_size = 0;
    unsigned int mVal = mEnt.mHandle.mVal;
    unsigned int v7 = mVal & 0xFFF;
    if (v7 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v7].mKey
        && EntityHandleDb::sInst.mElements[v7].mObject != nullptr)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v7].mObject;
        mObject->mNotifySet = nullptr;
    }
}

// ============================================================================
// DialogueManager / ConfigStringManager / STBManager
// ============================================================================

// ea: 0x004C0B40
DialogueManager::DialogueManager()
{
    AssetBankSet_ctor(this);
    for (int i = 0; i < 99; ++i)
        mBanks[i] = nullptr;
}

// ea: 0x004BCD80
DialogueManager::~DialogueManager()
{
    AssetBankSet_dtor(this);
}

// ea: 0x004C5C60
ConfigStringManager::ConfigStringManager()
{
    InplaceAssetBankSet_ConfigStringBank_ctor(this);
}

// ea: 0x004C1440
ConfigStringManager::~ConfigStringManager()
{
    AssetBankSet_dtor(this);
}

// ea: 0x004C5CE0
void* STBManager_Ctor(void* self)
{
    return InplaceAssetBankSet_StringTableBank_ctor(self);
}

// ea: 0x004C14D0
void STBManager_Dtor(void* self)
{
    AssetBankSet_dtor(self);
}

// ============================================================================
// CtrlIcon / HashString
// ============================================================================

// ea: 0x004BD6E0
CtrlIcon::CtrlIcon()
{
}

// ea: 0x004BD6F0
CtrlIcon::~CtrlIcon()
{
}

// ea: 0x004C1450
HashString::HashString(Broc::string& str)
{
    if (str.mBlock != nullptr)
        mHash = AeHash((const char*)(str.mBlock + 1));
    else
        mHash = AeHash("");
}

// ============================================================================
// AnimHeap
// ============================================================================

// ea: 0x004C1380
AnimHeap::AnimHeap()
{
    mBlock = mem_heap_malloc(0x100000);
    mem_heap_create((mem_heap*)mHeap, mBlock, (char*)mBlock + 0x100000,
                    nullptr);
    AnimHeapStatics::sInst = this;
}

// ea: 0x004BD610
AnimHeap::~AnimHeap()
{
    mem_heap_free(mBlock);
}

// ============================================================================
// RumbleManager
// ============================================================================

// ea: 0x004BCF60
RumbleManager::InstanceHolder::InstanceHolder()
{
    sInst[0] = nullptr;
}

// ea: 0x004C56F0
RumbleManager::RumbleManager(int client)
{
    mNextHandle.mVal = 1;
    for (int i = 0; i < 2; ++i)
    {
        RealDList* list = (RealDList*)&mRumbleLists[i];
        list->m_end = nullptr;
        list->m_head = &list->m_end;
        list->m_tail = &list->m_head;
        list->m_size = 0;
    }
    mClient = client;
    mLastTimeNotRumbling = 0;
    mDontRumbleAgainUntil = 0;
}

// ea: 0x004CF310
RumbleManager::~RumbleManager()
{
    controller_stop_all_rumble(controller_inst());
    for (int i = 0; i < 2; ++i)
    {
        RealDList* list = (RealDList*)&mRumbleLists[i];
        DNode* cur = (DNode*)list->m_head;
        while (cur != nullptr && cur != (DNode*)&list->m_end)
        {
            DNode* next = cur->m_next;
            delete (RumbleEffectInstance*)cur;
            cur = next;
        }
        list->m_head = &list->m_end;
        list->m_tail = &list->m_head;
        list->m_size = 0;
    }
}

// ============================================================================
// AbstractEffect family
// ============================================================================

// ea: 0x004C1240
AbstractEffect::AbstractEffect(TPakId pak_id, DbLinkedHandle<void, void> ent,
                               int flags, float delay_trigger)
{
    mEffectName = Broc::string((Broc::string::Block*)nullptr);
    mEntity.mHandle.mVal = 0;
    mCodeFlags.mVal = 0;
    mPoPtr = nullptr;
    mDelayTrigger = delay_trigger;
    mCountSinceStarted = 0;
    mEffectNameHashStr = 0;
    mCodeFlags.mVal = 0;
    mPakId = pak_id;
    mEntity = ent;
    mFlags = flags;
    mDelayCount = 0.0f;
}

// ea: 0x004BD160
AbstractEffect::~AbstractEffect()
{
    mEffectNameHashStr = 0;
    mEffectName.clear();
}

// ea: 0x004CC3F0
AbstractEffectSound::~AbstractEffectSound()
{
    unsigned int mVal = mSound.mHandle.mVal;
    unsigned int v3 = mVal & 0xFFF;
    if (v3 < 0x200 && mVal >> 12 == SoundHandleDb_sInst.mElements[v3].mKey
        && SoundHandleDb_sInst.mElements[v3].mObject != nullptr)
    {
        SoundDevice_ReleaseSound(SoundDevice_sInst,
                                 SoundHandleDb_sInst.mElements[v3].mObject);
    }
    mEffectNameHashStr = 0;
    AbstractEffect::~AbstractEffect();
}

// ea: 0x004C12C0
AbstractEffectParticle::~AbstractEffectParticle()
{
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle != nullptr)
    {
        apsEffect* mEffect = mParticle->mEffect;
        if (mEffect != nullptr)
        {
            if (EffectEventSysStatics::sInst->mStoppingAll
                || (mFlags & 8) == 0)
                mParticle->mFlags.mVal |= 0x20u;
            else
                mEffect->StopEmitting();
        }
        else
        {
            this->mParticle = nullptr;
        }
    }
    mEffectNameHashStr = 0;
    AbstractEffect::~AbstractEffect();
}

// ea: 0x004BD2A0
AbstractEffectLight::~AbstractEffectLight()
{
    mEffectNameHashStr = 0;
    RemoveLight(mProjLight);
    RemoveLight(mVertLight);
    AbstractEffect::~AbstractEffect();
}

// ea: 0x004CF890
AbstractEffectShakeAndRumble::AbstractEffectShakeAndRumble(
    TPakId pak_id, DbLinkedHandle<void, void> ent, float delay_trigger,
    int flags, float time, float freq, float movement, float nextDelay,
    float rumble, float blur, float minDist, float maxDist,
    float steadyDuration, float rampUpTime, float rampDownTime,
    bool useHighFreqVib, bool rumbleEnabled, EUserBoneId bone)
{
    mEffectName = Broc::string((Broc::string::Block*)nullptr);
    mEntity.mHandle.mVal = 0;
    mCodeFlags.mVal = 0;
    mDelayTrigger = delay_trigger;
    mPakId = pak_id;
    mEntity = ent;
    mFlags = flags;
    mPoPtr = nullptr;
    mDelayCount = 0.0f;
    mCountSinceStarted = 0;
    mEffectNameHashStr = 0;
    mCodeFlags.mVal = 0;
    mTime = time;
    mFreq = freq;
    mMovement = movement;
    mNextDelay = nextDelay;
    mRumble = rumble;
    mBlur = blur;
    mMinDist2 = minDist * minDist;
    mSteadyDuration = steadyDuration;
    mRampUpTime = rampUpTime;
    mMaxDist2 = maxDist * maxDist;
    mRampDownTime = rampDownTime;
    mUseHighFreqVibrator = useHighFreqVib;
    mRumbleEnabled = rumbleEnabled;
    mBone = bone;
    mRumbleHandle[0].mVal = 0;
    mType = AeHash("AbstractEffectShakeAndRumble");
    mShake[0] = nullptr;
    FrameAdvance(0.0f);
}

// ea: 0x004CF9D0
AbstractEffectShakeAndRumble::~AbstractEffectShakeAndRumble()
{
    if (dword_F6A290[0] == 2)
    {
        if (mRumbleHandle[0].mVal != 0)
        {
            RumbleManager* v2 = RumbleManager::Inst(0);
            v2->Remove(mRumbleHandle[0]);
            mRumbleHandle[0].mVal = 0;
        }
        if (mShake[0] != nullptr)
        {
            CameraShake_StopCameraShake(g_cameraShake, mShake[0]);
            mShake[0] = nullptr;
        }
    }
    mEffectNameHashStr = 0;
    AbstractEffect::~AbstractEffect();
}

// ============================================================================
// ActiveEffectSet / EffectEventSys
// ============================================================================

// ea: 0x004CEC60
ActiveEffectSet::~ActiveEffectSet()
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        AbstractEffect* effect = mEffects[i];
        if (EffectEventSysStatics::sInst->mStoppingAll
            || (effect->mFlags & 8) == 0)
        {
            delete effect;
        }
        else
        {
            effect->StartFadeOut(2.0f);
            EffectEventSysStatics::sInst->mFadingEffects.push_back(effect);
        }
    }
    if ((mFlags.mVal & 4) != 0)
    {
        ((PoolAllocator*)gCommonPoolAllocator)->Release(mPoPtr);
        mPoPtr = nullptr;
    }
    if (mId.mVal != 0)
        EffectEventSysStatics::sInst->mHandleDb.ReleaseHandle(mId);
}

// ea: 0x004D0130
EffectEventSys::EffectEventSys()
{
    mEffectSets.m_size = 0;
    mFadingEffects.m_size = 0;
    memset(&mPendingQueries, 0, sizeof(mPendingQueries));
    mCurrentQuery = nullptr;
    mStoppingAll = false;
    for (int i = 0; i < 32; ++i)
    {
        mEffectRefs.m_elements[i].first = 0;
        mEffectRefs.m_elements[i].second = 0;
    }
    mEffectRefs.m_size = 0;
    memset(&mHandleDb.mFreeBits, 0xFF, sizeof(mHandleDb.mFreeBits));
    for (int i = 0; i < 512; ++i)
    {
        mHandleDb.mElements[i].mObject = nullptr;
        mHandleDb.mElements[i].mKey = 0;
    }
    mDebuggingLevel = 0;
}

// ea: 0x004D01E0
EffectEventSys::~EffectEventSys()
{
    for (unsigned int i = 0; i < (unsigned int)mEffectSets.m_size; ++i)
    {
        ASSERT_IDX(i, 512, 154);
        ActiveEffectSet* v3 = mEffectSets[i];
        if (v3 != nullptr)
        {
            v3->~ActiveEffectSet();
            ActiveEffectSet_sAllocator->Release(v3);
        }
    }
    for (unsigned int j = 0; j < (unsigned int)mFadingEffects.m_size; ++j)
    {
        ASSERT_IDX(j, 128, 154);
        AbstractEffect* v5 = mFadingEffects[j];
        if (v5 != nullptr)
            delete v5;
    }
}

// ============================================================================
// TlSystemCallbacks
// ============================================================================

// ea: 0x004D0A20
TlSystemCallbacks::TlSystemCallbacks()
{
    mTlCallbacks.ReadFile =
        (bool (*)(const char*, tlFileBuf*, unsigned int, unsigned int))ReadFile;
    mTlCallbacks.ReleaseFile =
        (void (*)(tlFileBuf*))ReleaseFile;
    mTlCallbacks.CriticalError =
        (void (*)(const char*))CriticalError;
    mTlCallbacks.Warning = (void (*)(const char*))Warning;
    mTlCallbacks.DebugPrint = (void (*)(const char*))DebugPrint;
    mTlCallbacks.MemAlloc = (void* (*)(unsigned int, unsigned int,
                                       unsigned int))MemAlloc;
    mTlCallbacks.MemFree = (void (*)(void*))MemFree;
    mTlCallbacks.MemRealloc =
        (void* (*)(void*, unsigned int, unsigned int, unsigned int))MemRealloc;
    mTlCallbacks.FinalPrint = nullptr;
    mTlCallbacks.LinkConnected = (bool (*)())LinkConnected;
    tlSetSystemCallbacks((const tlSystemCallbacks*)this);
}
