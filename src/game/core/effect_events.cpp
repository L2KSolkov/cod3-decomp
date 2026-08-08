// ============================================================================
// effect_events.cpp - EffectEventSys / ActiveEffectSet + effect free functions
// (core.o EffectEvents.cpp family)
// ============================================================================

#include "game/core/core_systems.h"
#include "game/core/core_globals.h"
#include "core/PoolAllocator.h"

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

// nsl voice enumeration (stub env; game reads srcId at +0x114)
typedef unsigned int nslSourceID;
enum nslSourceState { NSL_SOURCE_STATE_INVALID = 0 };
struct nslVoice;
extern unsigned int nslGetNumVoices();
extern nslVoice* nslGetVoice(unsigned int a);
extern nslSourceState nslGetSourceState(nslSourceID sid);

extern PoolAllocator* ActiveEffectSet_sAllocator;  // 0x00F00E84

namespace EffectEventSysStatics {
EffectEventSys* sInst = nullptr;  // 0x012F0380
}

PoolAllocator* ActiveEffectSet_sAllocator = nullptr;

// ============================================================================
// EffectEventSys - query param setters
// ============================================================================

// ea: 0x004BCE20
int EffectEventSys::NumberOfVoicesUsed()
{
    unsigned int NumVoices = nslGetNumVoices();
    int v2 = 0;
    for (unsigned int i = 0; i < NumVoices; ++i)
    {
        nslVoice* Voice = nslGetVoice(i);
        if (nslGetSourceState(*(nslSourceID*)((char*)Voice + 0x114))
            != NSL_SOURCE_STATE_INVALID)
            ++v2;
    }
    return v2;
}

// ea: 0x004BCE60
void EffectEventSys::TagNameIndexInfo(int index)
{
    mCurrentQuery->mBoneIndex = index;
}

// ea: 0x004BCE80
void EffectEventSys::SetEffectMatrix(math::Mat43* pMat)
{
    mCurrentQuery->mMatrix = pMat;
}

// ea: 0x004BCEA0
void EffectEventSys::SetScriptId(Broc::string val)
{
    mCurrentQuery->mScriptId = val;
}

// ea: 0x004BCF00
void EffectEventSys::SetDialogNotify(int notify)
{
    mCurrentQuery->mDialogNotify = notify;
}

// ea: 0x004BCF20
void EffectEventSys::SetQueryType(unsigned int val)
{
    mCurrentQuery->mQueryType = val;
}

// ea: 0x004BCF40
void EffectEventSys::SoundCacheType(int val)
{
    mCurrentQuery->mCacheSoundType = (int16_t)val;
}

// ea: 0x004C11A0
void EffectEventSys::IsSoundToBeQueued(bool val)
{
    if (val)
        mCurrentQuery->mFlags.mVal |= 2u;
}

// ea: 0x004C11C0
void EffectEventSys::SetQueryImportance(bool val)
{
    if (val)
        mCurrentQuery->mFlags.mVal |= 8u;
    else
        mCurrentQuery->mFlags.mVal &= ~8u;
}

// ea: 0x004C11F0
void EffectEventSys::DirectionInfo(const float* dir)
{
    mCurrentQuery->mFlags.mVal |= 1u;
    mCurrentQuery->mCollisionInfo.simple.normal.v.m128_f32[0] = dir[0];
    mCurrentQuery->mCollisionInfo.simple.normal.v.m128_f32[1] = dir[1];
    mCurrentQuery->mCollisionInfo.simple.normal.v.m128_f32[2] = dir[2];
}

// ============================================================================
// HandleDb lookups
// ============================================================================

// ea: 0x004CACD0
bool EffectEventSys::IsEffectActive(Handle handle)
{
    int v2 = handle.mVal & 0x1FF;
    bool result = false;
    if (v2 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v2].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v2].mObject;
        if (mObject != nullptr
            && ((mObject->mFlags.mVal & 2) != 0
                || mObject->mEffects.m_size != 0))
            return true;
    }
    return result;
}

// ea: 0x004CB890
bool EffectEventIsPlaying(Handle effect)
{
    int v1 = effect.mVal & 0x1FF;
    bool result = false;
    if (v1 < 0x200
        && effect.mVal >> 9
               == EffectEventSysStatics::sInst->mHandleDb.mElements[v1].mKey)
    {
        ActiveEffectSet* mObject =
            EffectEventSysStatics::sInst->mHandleDb.mElements[v1].mObject;
        if (mObject != nullptr
            && ((mObject->mFlags.mVal & 2) != 0
                || mObject->mEffects.m_size != 0))
            return true;
    }
    return result;
}

// ea: 0x004CB8E0
void EffectEventAdjustEffect_Scale(Handle effect, const char* param,
                                   float scale)
{
    int v3 = effect.mVal & 0x1FF;
    if (v3 < 0x200
        && effect.mVal >> 9
               == EffectEventSysStatics::sInst->mHandleDb.mElements[v3].mKey)
    {
        ActiveEffectSet* mObject =
            EffectEventSysStatics::sInst->mHandleDb.mElements[v3].mObject;
        if (mObject != nullptr)
            mObject->AdjustEffect_Scale(param, scale);
    }
}

// ea: 0x004CB930
void EffectEventFF(Handle effect, float deltaT)
{
    int v2 = effect.mVal & 0x1FF;
    if (v2 < 0x200
        && effect.mVal >> 9
               == EffectEventSysStatics::sInst->mHandleDb.mElements[v2].mKey)
    {
        ActiveEffectSet* mObject =
            EffectEventSysStatics::sInst->mHandleDb.mElements[v2].mObject;
        if (mObject != nullptr)
            mObject->FastForward(deltaT);
    }
}

// ea: 0x004CB980
void EffectEventPlayQueuedEffect(Handle effect)
{
    int v1 = effect.mVal & 0x1FF;
    if (v1 < 0x200
        && effect.mVal >> 9
               == EffectEventSysStatics::sInst->mHandleDb.mElements[v1].mKey)
    {
        ActiveEffectSet* mObject =
            EffectEventSysStatics::sInst->mHandleDb.mElements[v1].mObject;
        if (mObject != nullptr)
            mObject->PlayQueuedEffect();
    }
}

// ea: 0x004CF2D0
void EffectEventStopEmitting(Handle effect)
{
    EffectEventSysStatics::sInst->StopEffect(effect, false);
}

// ea: 0x004CF2F0
void EffectEventKill(Handle effect)
{
    EffectEventSysStatics::sInst->StopEffect(effect, false);
}

// ea: 0x004CEF20
void EffectEventSys::StopEffect(Handle handle, bool kill)
{
    bool saveStoppingAll = mStoppingAll;
    if (kill)
        mStoppingAll = true;
    for (unsigned int v5 = 0; v5 < (unsigned int)mEffectSets.m_size; ++v5)
    {
        ASSERT_IDX(v5, 512, 154);
        if (mEffectSets.m_elements[v5]->mId.mVal == handle.mVal)
        {
            ActiveEffectSet* v7 = mEffectSets[v5];
            if (v7 != nullptr)
            {
                v7->~ActiveEffectSet();
                ActiveEffectSet_sAllocator->Release(v7);
            }
            mEffectSets[v5] = mEffectSets[mEffectSets.m_size - 1];
            if (mEffectSets.m_size != 0)
                --mEffectSets.m_size;
            --v5;
        }
    }
    mStoppingAll = saveStoppingAll;
}

// ============================================================================
// ActiveEffectSet
// ============================================================================

// ea: 0x004C0C00
void ActiveEffectSet::AddEffect(AbstractEffect* effect)
{
    math::Mat43* mPoPtr = this->mPoPtr;
    if (mPoPtr != nullptr)
        effect->SetPoPtr(mPoPtr);
    mEffects.push_back(effect);
}

// ea: 0x004C0C30
bool ActiveEffectSet::IsFinished() const
{
    return (mFlags.mVal & 2) == 0 && mEffects.m_size == 0;
}

// ea: 0x004C0C50
bool ActiveEffectSet::IsQueued() const
{
    unsigned int v2 = 0;
    if (mEffects.m_size == 0)
        return true;
    while (true)
    {
        ASSERT_IDX(v2, 6, 148);
        if (!mEffects.m_elements[v2]->IsQueued())
            break;
        if (++v2 >= (unsigned int)mEffects.m_size)
            return true;
    }
    return false;
}

// ea: 0x004C0CD0
void ActiveEffectSet::AdjustEffect_Scale(const char* param, float scale)
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        mEffects.m_elements[i]->AdjustEffect_Scale(param, scale);
    }
}

// ea: 0x004C0D50
void ActiveEffectSet::FastForward(float deltaT)
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        mEffects.m_elements[i]->FastForward(deltaT);
    }
}

// ea: 0x004C0DD0
void ActiveEffectSet::PlayQueuedEffect()
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        mEffects.m_elements[i]->PlayQueuedEffect();
    }
}

// ea: 0x004C0E40
void ActiveEffectSet::SetPoPtr(math::Mat43* po)
{
    int m_size = mEffects.m_size;
    unsigned int v4 = 0;
    mPoPtr = po;
    if (m_size != 0)
    {
        do
        {
            ASSERT_IDX(v4, 6, 154);
            mEffects.m_elements[v4]->SetPoPtr(po);
            ++v4;
        } while (v4 < (unsigned int)mEffects.m_size);
    }
}

// ea: 0x004C0EC0
void ActiveEffectSet::StopLoopingEffects()
{
    mFlags.mVal |= 1u;
}

// ea: 0x004C0ED0
void ActiveEffectSet::DoStopLoopingEffects()
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        AbstractEffect* effect = mEffects.m_elements[i];
        if (effect->IsLooping())
        {
            ASSERT_IDX(i, 6, 154);
            if ((effect->mFlags & 1) == 0)
            {
                AbstractEffect* p = mEffects[i];
                if (p != nullptr)
                    delete p;
                mEffects[i] = mEffects[mEffects.m_size - 1];
                if (mEffects.m_size != 0)
                    --mEffects.m_size;
                --i;
            }
        }
    }
}
