// ============================================================================
// effect_events.cpp - EffectEventSys / ActiveEffectSet + effect free functions
// (core.o EffectEvents.cpp family)
// ============================================================================

#include "game/core/core_systems.h"
#include "game/core/core_globals.h"
#include "core/PoolAllocator.h"
#include "aeps/apsEffect.h"

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
// Entity lookup (HandleDb<Entity,1344,SizedHandle<12,20>>; elements at +0xA8)
// ============================================================================
class EntityHandleDb {
public:
    struct DbElement {
        Entity* mObject;  // +0x00
        int     mKey;     // +0x04
    };
    unsigned char _pad[0xA8];
    DbElement     mElements[0x540];
    static EntityHandleDb sInst;  // ?sInst@EntityHandleDb@@0V1@A
};
EntityHandleDb EntityHandleDb::sInst;

class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A
    Entity* GetPlayer(int idx);
};
EntityManager* EntityManager::sInst = nullptr;

static const char defaultFileName[] = "";

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

// ============================================================================
// AbstractEffect virtuals / getters
// ============================================================================

// ea: 0x004CC230
math::Position3 AbstractEffect::GetPosition() const
{
    math::Position3 result;
    const math::Mat43* mPoPtr = this->mPoPtr;
    if (mPoPtr != nullptr)
    {
        result.v.m128_f32[0] = mPoPtr->w.v.m128_f32[0];
        result.v.m128_f32[1] = mPoPtr->w.v.m128_f32[1];
        result.v.m128_f32[2] = mPoPtr->w.v.m128_f32[2];
        result.v.m128_f32[3] = mPoPtr->w.v.m128_f32[3];
        return result;
    }
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v7 = mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v7 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v7].mKey)
        mObject = EntityHandleDb::sInst.mElements[v7].mObject;
    result.v = mObject->r.currentOrigin.v;
    return result;
}

// ea: 0x004CC2F0
bool AbstractEffect::IsFinished()
{
    unsigned int v1 = mEntity.mHandle.mVal & 0xFFF;
    if (v1 < 0x540
        && mEntity.mHandle.mVal >> 12
               == EntityHandleDb::sInst.mElements[v1].mKey
        && EntityHandleDb::sInst.mElements[v1].mObject != nullptr)
    {
        return false;
    }
    return (mFlags & 1) == 0;
}

// ea: 0x004CC330
Broc::string AbstractEffect::GetEntityDebugString() const
{
    Broc::string r((Broc::string::Block*)nullptr);
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v4 = mVal & 0xFFF;
    if (v4 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != nullptr)
        {
            r += " ENT:";
            Broc::string::Block* mBlock = mObject->mClassName.mBlock;
            const char* v7 = mBlock ? (const char*)(mBlock + 1)
                                    : defaultFileName;
            r += v7;
        }
    }
    return r;
}

// ea: 0x004BD300
Broc::string AbstractEffectLight::GetDebugString() const
{
    return Broc::string((Broc::string::Block*)nullptr);
}

// ea: 0x004BD370
Broc::string AbstractEffectShakeAndRumble::GetDebugString() const
{
    return Broc::string((Broc::string::Block*)nullptr);
}

// ea: 0x004CE0B0
math::Position3 AbstractEffectShakeAndRumble::GetPositionOnEntity() const
{
    math::Position3 result;
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v3 = mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v3 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey)
        mObject = EntityHandleDb::sInst.mElements[v3].mObject;
    result.v = mObject->r.currentOrigin.v;
    return result;
}

// ea: 0x004CE110
float AbstractEffectShakeAndRumble::GetDistanceScale(int client)
{
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v5 = mVal & 0xFFF;
    if (v5 >= 0x540 || mVal >> 12 != EntityHandleDb::sInst.mElements[v5].mKey
        || EntityHandleDb::sInst.mElements[v5].mObject == nullptr)
        return 1.0f;
    if (mMinDist2 <= 0.0f || mMaxDist2 <= mMinDist2)
        return 1.0f;
    Entity* Player = EntityManager::sInst->GetPlayer(client);
    math::Position3 v11 = GetPositionOnEntity();
    __m128 v7 = _mm_sub_ps(Player->r.currentOrigin.v, v11.v);
    __m128 v8 = _mm_mul_ps(v7, v7);
    float v12 = v8.m128_f32[0]
                + (_mm_shuffle_ps(v8, v8, 0x55).m128_f32[0]
                   + _mm_shuffle_ps(v8, v8, 0xAA).m128_f32[0]);
    if (mMinDist2 > v12)
        return 1.0f;
    if (mMaxDist2 <= v12)
        return 0.0f;
    return 1.0f - (v12 - mMinDist2) / (mMaxDist2 - mMinDist2);
}

// ============================================================================
// AbstractEffectParticle virtuals
// ============================================================================

// ea: 0x004BD200
void AbstractEffectParticle::SetPoPtr(math::Mat43* po)
{
    mPoPtr = po;
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle != nullptr)
        mParticle->mPoPtr = po;
}

// ea: 0x004BD220
bool AbstractEffectParticle::IsLooping() const
{
    return false;
}

// ea: 0x004BD230
void AbstractEffectParticle::AdjustEffect_Scale(const char* param, float scale)
{
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle != nullptr)
    {
        apsEffect* mEffect = mParticle->mEffect;
        if (mEffect != nullptr)
        {
            int ModifierId = mEffect->GetModifierId(param);
            if (ModifierId >= 0)
            {
                float value =
                    this->mParticle->mEffect->GetModifierTemplateValue(
                        ModifierId);
                float iVal = value * scale;
                this->mParticle->mEffect->SetModifierValue(ModifierId, iVal);
            }
        }
    }
}

// ea: 0x004BD280
void AbstractEffectParticle::FastForward(float deltaT)
{
    apsEffect* mEffect = mParticle->mEffect;
    if (mEffect != nullptr)
        mEffect->FastForward(deltaT, 100);
}

// ea: 0x004C1350
void AbstractEffectParticle::StartFadeOut(float seconds)
{
    mCodeFlags.mVal |= 3u;
    mFadeStart = seconds;
    mFadeTime = seconds;
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle != nullptr)
    {
        apsEffect* mEffect = mParticle->mEffect;
        if (mEffect != nullptr)
            mEffect->StopEmitting();
    }
}

// ea: 0x004C59C0
Broc::string AbstractEffectParticle::GetDebugString() const
{
    Broc::string r((Broc::string::Block*)nullptr);
    if (mDelayTrigger <= mDelayCount)
    {
        Broc::string::Block* mBlock = mEffectName.mBlock;
        const char* v8 = mBlock ? (const char*)(mBlock + 1)
                                : defaultFileName;
        ae_formatted_string<1024, unsigned short> v10("PFX: %s", v8);
        r = (const char*)v10.mBuff;
    }
    else
    {
        Broc::string::Block* v4 = mEffectName.mBlock;
        const char* v5 = v4 ? (const char*)(v4 + 1) : defaultFileName;
        ae_formatted_string<1024, unsigned short> v10(
            "PFX: %s DELAYED %f/%f", v5, mDelayCount, mDelayTrigger);
        r = (const char*)v10.mBuff;
    }
    return r;
}

// ============================================================================
// AbstractEffectSound virtuals
// ============================================================================

// ea: 0x004C12A0
void AbstractEffectSound::StartFadeOut(float seconds)
{
    mCodeFlags.mVal |= 3u;
    mFadeStart = seconds;
    mFadeTime = seconds;
}
