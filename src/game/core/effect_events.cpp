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
bool Warning(const char* fmt, ...);
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
// Camera shake support types (layout from IDA)
// ============================================================================
struct CameraShakeInstance {
    int   m_type;          // +0x00
    float m_magnitude;     // +0x04
    float m_magnitudeInc;  // +0x08
    float m_noiseFloats[8];// +0x0C
    float m_invDistance;   // +0x2C
    float m_time;          // +0x30
    float m_frequency;     // +0x34
    float m_movement;      // +0x38
    int   m_active;        // +0x3C
};
static_assert(sizeof(CameraShakeInstance) == 0x40,
              "CameraShakeInstance size mismatch");

struct CameraShake {
    float m_scale3D;          // +0x00
    float m_scaleCOD;         // +0x04
    int   m_scaleCOD_onlyADS; // +0x08
    CameraShakeInstance m_instanceData[5];  // +0x0C

    void StopCameraShake(CameraShakeInstance* pShake);
};
static_assert(sizeof(CameraShake) == 0x14C, "CameraShake size mismatch");

extern math::Position3 GetTagFlashPos(Entity* cent);
extern CameraShake* g_cameraShake;  // 0x00F056E8
extern int dword_F6A290[4 * 0x322];  // per-client table, 0xC88-byte stride
extern void FX_ClearFX();
extern void Scr_Notify(Entity* ent, HashString hashValue,
                       unsigned int paramcount);

CameraShake* g_cameraShake = nullptr;
int dword_F6A290[4 * 0x322];

// snd_wait (Entity +0x3C8): two HashStrings
struct SndWait {
    HashString notifyHash;  // +0x00
    HashString soundName;   // +0x04
};

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
// EffectEventSys - sound notify + fades
// ============================================================================

// ea: 0x004BCD90
void EffectEventSys::SendSpecificSoundNotify(Entity* pEnt,
                                             HashString soundName)
{
    if (pEnt != nullptr)
    {
        SndWait* sndWait = (SndWait*)&pEnt->snd_wait;
        if (sndWait->notifyHash.mHash != 0
            && sndWait->soundName.mHash == soundName.mHash)
        {
            Scr_Notify(pEnt, sndWait->notifyHash, 0);
            sndWait->notifyHash.mHash = 0;
            sndWait->soundName.mHash = 0;
        }
    }
}

// ea: 0x004BCDE0
void EffectEventSys::SendSoundNotify(Entity* pEnt)
{
    if (pEnt != nullptr)
    {
        SndWait* sndWait = (SndWait*)&pEnt->snd_wait;
        if (sndWait->notifyHash.mHash != 0)
        {
            Scr_Notify(pEnt, sndWait->notifyHash, 0);
            sndWait->notifyHash.mHash = 0;
            sndWait->soundName.mHash = 0;
        }
    }
}

// ea: 0x004C0FE0
void EffectEventSys::FadeOutEffect(AbstractEffect* effect, float seconds)
{
    effect->StartFadeOut(seconds);
    mFadingEffects.push_back(effect);
}

// ea: 0x004C1010
void EffectEventSys::AdvanceFades(float delta)
{
    for (unsigned int v3 = 0; v3 < (unsigned int)mFadingEffects.m_size; ++v3)
    {
        ASSERT_IDX(v3, 128, 154);
        if ((mFadingEffects[v3]->mCodeFlags.mVal & 1) != 0)
        {
            ASSERT_IDX(v3, 128, 154);
            AbstractEffect* effect = mFadingEffects[v3];
            effect->FrameAdvance(delta);
            ASSERT_IDX(v3, 128, 154);
            if ((effect->mCodeFlags.mVal & 1) != 0
                && effect->mFadeTime < 0.001f)
            {
                ASSERT_IDX(v3, 128, 154);
                AbstractEffect* p = mFadingEffects[v3];
                if (p != nullptr)
                    delete p;
                mFadingEffects[v3] = mFadingEffects[mFadingEffects.m_size - 1];
                if (mFadingEffects.m_size != 0)
                    --mFadingEffects.m_size;
                --v3;
            }
        }
    }
}

// ea: 0x004C1120
void EffectEventSys::SetSoundParams(SoundParams& soundParams, PendingQuery& q,
                                    float useNslDefault)
{
    soundParams.mNameRef = nullptr;
    soundParams.mHashStr = 0;
    soundParams.mVolume = useNslDefault;
    soundParams.mPakId = q.mEffectsPak;
    soundParams.mEnt.mHandle.mVal = q.mQueryEnt.mHandle.mVal;
    soundParams.mQueue = (q.mFlags.mVal & 2) != 0;
    soundParams.mCd = (q.mFlags.mVal & 4) != 0 ? &q.mCollisionInfo : nullptr;
    soundParams.mFlags = 0;
    soundParams.mMaxVoices = 2;
    soundParams.mPitch = useNslDefault;
    soundParams.mDuration = useNslDefault;
    soundParams.mSubtitle = nullptr;
    soundParams.mDialogNotify = q.mDialogNotify;
}

// ea: 0x004CA820
int EffectEventSys::IsSoundAlreadyPlaying(unsigned int mSoundNameHashStr,
                                          Entity* pEnt, int maxEffects)
{
    int playCount = 0;
    AbstractEffect* oldestEffect = nullptr;
    int oldestEffectIndex = 0;
    unsigned int oldestAbstractEffectIndex = 0;
    int highestDelayCountSoFar = 0;
    bool stopSound = false;
    if (mEffectSets.m_size == 0)
        return 0;
    for (unsigned int v6 = 0; v6 < (unsigned int)mEffectSets.m_size; ++v6)
    {
        ASSERT_IDX(v6, 512, 154);
        ActiveEffectSet* v8 = mEffectSets[v6];
        for (unsigned int v7 = 0;
             v7 < (unsigned int)v8->mEffects.m_size; ++v7)
        {
            ASSERT_IDX(v6, 512, 154);
            ASSERT_IDX(v7, 6, 148);
            AbstractEffect* v9 = v8->mEffects.m_elements[v7];
            if (v9 == nullptr || (v9->mCodeFlags.mVal & 4) == 0)
                continue;
            unsigned int v10 = v9->mEntity.mHandle.mVal & 0xFFF;
            Entity* mObject = nullptr;
            if (v10 < 0x540
                && v9->mEntity.mHandle.mVal >> 12
                       == EntityHandleDb::sInst.mElements[v10].mKey)
                mObject = EntityHandleDb::sInst.mElements[v10].mObject;
            if (pEnt == mObject
                && mSoundNameHashStr == v9->mEffectNameHashStr)
            {
                if (++playCount >= maxEffects)
                    stopSound = true;
                if (oldestEffect != nullptr)
                {
                    if (v9->mCountSinceStarted > highestDelayCountSoFar)
                    {
                        oldestEffect = v9;
                        highestDelayCountSoFar = v9->mCountSinceStarted;
                        oldestEffectIndex = (int)v6;
                        oldestAbstractEffectIndex = v7;
                    }
                    continue;
                }
                oldestEffect = v9;
                oldestAbstractEffectIndex = v7;
                highestDelayCountSoFar = v9->mCountSinceStarted;
                oldestEffectIndex = (int)v6;
            }
        }
    }
    if (stopSound)
    {
        if (oldestEffect == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
            AeAssert::gCurrentLine = 403;
            AeAssert::gCurrentExpr = "oldestEffect";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Oldest effect not setup"))
                __debugbreak();
        }
        oldestEffect->StopEffect();
        ActiveEffectSet* set = mEffectSets[oldestEffectIndex];
        ASSERT_IDX(oldestAbstractEffectIndex, 6, 154);
        AbstractEffect* p = set->mEffects[oldestAbstractEffectIndex];
        if (p != nullptr)
            delete p;
        unsigned int v14 = set->mEffects.m_size - 1;
        ASSERT_IDX(v14, 6, 154);
        set->mEffects[oldestAbstractEffectIndex] = set->mEffects[v14];
        if (set->mEffects.m_size != 0)
            --set->mEffects.m_size;
    }
    return playCount;
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
// HandleDb
// ============================================================================

// ea: 0x004E8D50
Handle HandleDb::AllocateHandle()
{
    int m_cur_val = -1;
    for (int i = 0; i < 512; ++i)
    {
        if ((mFreeBits[i >> 3] >> (i & 7)) & 1)
        {
            m_cur_val = i;
            break;
        }
    }
    if (m_cur_val >= 0x200)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
        AeAssert::gCurrentLine = 98;
        AeAssert::gCurrentExpr = "nextIndex >= 0 && nextIndex < _MaxEltements";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "index out of bounds!!! ILLEGAL array access!"))
            __debugbreak();
    }
    mFreeBits[m_cur_val >> 3] &= (unsigned char)~(1u << (m_cur_val & 7));
    Handle result;
    result.mVal = (mElements[m_cur_val].mKey << 9) | m_cur_val;
    return result;
}

// ea: 0x004E3DB0
void HandleDb::BindObjectToHandle(Handle handle, ActiveEffectSet* obj)
{
    int v3 = handle.mVal & 0x1FF;
    if (mElements[v3].mKey != (unsigned int)(handle.mVal >> 9))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
        AeAssert::gCurrentLine = 123;
        AeAssert::gCurrentExpr = "element.GetKey() == h.GetKey()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("handle was not allocated for this object"))
            __debugbreak();
    }
    mElements[v3].mObject = obj;
}

// ea: 0x004E6430
void HandleDb::ReleaseHandle(Handle h)
{
    if (h.mVal != 0)
    {
        int v3 = h.mVal & 0x1FF;
        if (mElements[v3].mKey == (unsigned int)(h.mVal >> 9))
        {
            mFreeBits[v3 >> 3] |= (unsigned char)(1u << (v3 & 7));
            mElements[v3].mObject = nullptr;
            mElements[v3].mKey = mElements[v3].mKey + 1;
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
            AeAssert::gCurrentLine = 170;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("freeing invalid handle"))
                __debugbreak();
        }
    }
}

// ============================================================================
// EffectEventSys - handle plumbing + lifecycle
// ============================================================================

// ea: 0x004C56B0
ActiveEffectSet* EffectEventSys::GetActiveEffectSet(Handle handle)
{
    int v2 = handle.mVal & 0x1FF;
    if (v2 < 0x200
        && handle.mVal >> 9 == mHandleDb.mElements[v2].mKey)
        return mHandleDb.mElements[v2].mObject;
    return nullptr;
}

// ea: 0x004CABD0
void EffectEventSys::AdjustEffect_Scale(Handle handle, const char* param,
                                        float scale)
{
    int v4 = handle.mVal & 0x1FF;
    if (v4 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v4].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v4].mObject;
        if (mObject != nullptr)
            mObject->AdjustEffect_Scale(param, scale);
    }
}

// ea: 0x004CAC10
void EffectEventSys::FastForward(Handle handle, float deltaT)
{
    int v3 = handle.mVal & 0x1FF;
    if (v3 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v3].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v3].mObject;
        if (mObject != nullptr)
            mObject->FastForward(deltaT);
    }
}

// ea: 0x004CAC50
void EffectEventSys::PlayQueuedEffect(Handle handle)
{
    int v2 = handle.mVal & 0x1FF;
    if (v2 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v2].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v2].mObject;
        if (mObject != nullptr)
            mObject->PlayQueuedEffect();
    }
}

// ea: 0x004CAC90
void EffectEventSys::StopLoopingEffects(Handle handle)
{
    int v2 = handle.mVal & 0x1FF;
    if (v2 < 0x200 && handle.mVal >> 9 == mHandleDb.mElements[v2].mKey)
    {
        ActiveEffectSet* mObject = mHandleDb.mElements[v2].mObject;
        if (mObject != nullptr)
            mObject->mFlags.mVal |= 1u;
    }
}

// ea: 0x004CB840
void EffectEventSys::ReleaseHandle(ActiveEffectSet* t)
{
    if (t->mId.mVal != 0)
    {
        mHandleDb.ReleaseHandle(t->mId);
        t->mId.mVal = 0;
    }
}

// ea: 0x004CB870
void EffectEventSys::ReleaseHandle(Handle h)
{
    if (h.mVal != 0)
        mHandleDb.ReleaseHandle(h);
}

// ea: 0x004CEDC0
void EffectEventSys::StopAll()
{
    mStoppingAll = true;
    while (mEffectSets.m_size != 0)
    {
        ActiveEffectSet* v3 = mEffectSets[0];
        if (v3 != nullptr)
        {
            v3->~ActiveEffectSet();
            ActiveEffectSet_sAllocator->Release(v3);
        }
        unsigned int v4 = mEffectSets.m_size - 1;
        ASSERT_IDX(v4, 512, 154);
        mEffectSets[0] = mEffectSets[v4];
        if (mEffectSets.m_size != 0)
            --mEffectSets.m_size;
    }
    while (mFadingEffects.m_size != 0)
    {
        AbstractEffect* v6 = mFadingEffects[0];
        if (v6 != nullptr)
            delete v6;
        unsigned int v7 = mFadingEffects.m_size - 1;
        ASSERT_IDX(v7, 128, 154);
        mFadingEffects[0] = mFadingEffects[v7];
        if (mFadingEffects.m_size != 0)
            --mFadingEffects.m_size;
    }
    FX_ClearFX();
    mStoppingAll = false;
}

// ea: 0x004CF020
void EffectEventSys::KillEffectsWithPakId(TPakId pak_id)
{
    for (unsigned int v3 = 0; v3 < (unsigned int)mEffectSets.m_size;)
    {
        ASSERT_IDX(v3, 512, 154);
        if (mEffectSets[v3]->mPakId == pak_id)
        {
            ASSERT_IDX(v3, 512, 154);
            ActiveEffectSet* v4 = mEffectSets[v3];
            if (v4 != nullptr)
            {
                v4->~ActiveEffectSet();
                ActiveEffectSet_sAllocator->Release(v4);
            }
            unsigned int v5 = mEffectSets.m_size - 1;
            ASSERT_IDX(v5, 512, 154);
            ASSERT_IDX(v3, 512, 154);
            mEffectSets[v3] = mEffectSets[v5];
            if (mEffectSets.m_size != 0)
                --mEffectSets.m_size;
        }
        else
        {
            ++v3;
        }
    }
}

// ea: 0x004CF1D0
void EffectEventSys::CollisionInfo(const CollisionDesc* col_desc, bool set_mat)
{
    mCurrentQuery->mFlags.mVal |= 4u;
    memcpy(&mCurrentQuery->mCollisionInfo, col_desc,
           sizeof(mCurrentQuery->mCollisionInfo));
    if (set_mat)
    {
        int material = col_desc->material;
        if (material > 0)  // kCollisionMaterialASPHALT
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
            AeAssert::gCurrentLine = 1558;
            AeAssert::gCurrentExpr =
                "( mat_type >= kCollisionMaterialMin && mat_type <= "
                "kCollisionMaterialMax )";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("value not in enum range"))
                __debugbreak();
        }
        CachedQuery* p_mCachedQuery = &mCurrentQuery->mCachedQuery;
        p_mCachedQuery->mSpecifiedFields.mBits[3 >> 3] |=
            (unsigned char)(1u << (3 & 7));
        p_mCachedQuery->mWeakFields.mBits[3 >> 3] |=
            (unsigned char)(1u << (3 & 7));
        p_mCachedQuery->mMATERIAL = material;
    }
}

// ea: 0x004CF290
Handle EffectEventSys::AssignHandle(ActiveEffectSet* t)
{
    Handle v4 = mHandleDb.AllocateHandle();
    mHandleDb.BindObjectToHandle(v4, t);
    t->mId = v4;
    return v4;
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

// ea: 0x004CA660
void ActiveEffectSet::FrameAdvance(float delta)
{
    if ((mFlags.mVal & 2) != 0 && mEffects.m_size != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EffectEventSys.cpp";
        AeAssert::gCurrentLine = 169;
        AeAssert::gCurrentExpr =
            "!mFlags.Test( kActiveEffectFlag_Pending ) || "
            "mEffects.size() == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("effect should be ready"))
            __debugbreak();
    }
    if ((mFlags.mVal & 2) == 0)
    {
        if ((mFlags.mVal & 1) != 0)
            DoStopLoopingEffects();
        int m_size = mEffects.m_size;
        int size = m_size;
        for (unsigned int v4 = 0; v4 < (unsigned int)m_size; ++v4)
        {
            ASSERT_IDX(v4, 6, 154);
            mEffects.m_elements[v4]->FrameAdvance(delta);
            if (m_size <= mEffects.m_size)
            {
                ASSERT_IDX(v4, 6, 154);
                AbstractEffect* effect = mEffects[v4];
                if (effect->IsFinished())
                {
                    unsigned int mVal =
                        mEffects[v4]->mEntity.mHandle.mVal;
                    unsigned int v7 = mVal & 0xFFF;
                    if (v7 < 0x540
                        && mVal >> 12 == EntityHandleDb::sInst.mElements[v7].mKey)
                    {
                        Entity* mObject =
                            EntityHandleDb::sInst.mElements[v7].mObject;
                        if (mObject != nullptr)
                        {
                            SndWait* sndWait = (SndWait*)&mObject->snd_wait;
                            if (sndWait->notifyHash.mHash != 0)
                            {
                                Scr_Notify(mObject, sndWait->notifyHash, 0);
                                sndWait->notifyHash.mHash = 0;
                                sndWait->soundName.mHash = 0;
                            }
                        }
                    }
                    ASSERT_IDX(v4, 6, 154);
                    AbstractEffect* p = mEffects[v4];
                    if (p != nullptr)
                        delete p;
                    mEffects[v4] = mEffects[mEffects.m_size - 1];
                    if (mEffects.m_size != 0)
                        --mEffects.m_size;
                    --v4;
                    m_size = --size;
                }
            }
            else
            {
                --v4;
                size = --m_size;
            }
        }
    }
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

// ============================================================================
// AbstractEffectLight virtuals
// ============================================================================

// ea: 0x004CDEF0
math::Position3 AbstractEffectLight::GetPositionOnEntity(Entity* e) const
{
    math::Position3 result;
    unsigned int mVal = mEntity.mHandle.mVal;
    unsigned int v4 = mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v4 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey)
        mObject = EntityHandleDb::sInst.mElements[v4].mObject;
    result.v = mObject->r.currentOrigin.v;
    return result;
}

// ea: 0x004CDF50
void AbstractEffectLight::FrameAdvance(float delta_t)
{
    LightEffect* mVertLight = this->mVertLight;
    if (mVertLight != nullptr)
    {
        gdLight* mLight = this->mLight;
        mTime = delta_t + mTime;
        if (mTime <= ((mLight->rampdown + mLight->rampup) + mLight->duration))
        {
            if (mTime < (mLight->rampup + mLight->duration))
            {
                if (mTime < mLight->rampup)
                    mVertLight->SetScale(mTime / mLight->rampup);
                else
                    mVertLight->SetScale(1.0f);
            }
            else
            {
                mVertLight->SetScale(
                    1.0f - ((mTime - mLight->duration) - mLight->rampup)
                               / mLight->rampdown);
            }
        }
        else
        {
            mVertLight->mActive = false;
            this->mVertLight = nullptr;
        }
        if (this->mVertLight != nullptr)
        {
            unsigned int v7 = mEntity.mHandle.mVal & 0xFFF;
            if (v7 < 0x540
                && mEntity.mHandle.mVal >> 12
                       == EntityHandleDb::sInst.mElements[v7].mKey)
            {
                Entity* mObject = EntityHandleDb::sInst.mElements[v7].mObject;
                if (mObject != nullptr)
                    this->mVertLight->mLightPos.v =
                        GetTagFlashPos(mObject).v;
            }
        }
    }
}

// ea: 0x004CE060
bool AbstractEffectLight::IsFinished()
{
    if (mLight->projlight != 0)
        return true;
    LightEffect* pProj = mProjLight;
    if (pProj == nullptr || pProj->IsLightFinished())
    {
        LightEffect* pVert = mVertLight;
        if (pVert == nullptr || pVert->IsLightFinished())
            return true;
    }
    if ((mCodeFlags.mVal & 2) != 0)
        return AbstractEffect::IsFinished();
    return false;
}

// ============================================================================
// AbstractEffectShakeAndRumble virtuals
// ============================================================================

// ea: 0x004CE720
bool AbstractEffectShakeAndRumble::IsFinished()
{
    if ((mCodeFlags.mVal & 2) == 0)
        return true;
    for (int i = 0; i < 4; ++i)
    {
        if (dword_F6A290[i * 0x322] == 2 && mShake[i] != nullptr
            && mShake[i]->m_active != 0)
        {
            RumbleManager* v4 = RumbleManager::Inst(i);
            if (v4->IsPlaying(mRumbleHandle[i]))
                return false;
        }
    }
    return true;
}

// ea: 0x004CE780
void AbstractEffectShakeAndRumble::AdjustEffect_Scale(const char* param,
                                                      float scale)
{
    if (dword_F6A290[0] == 2 && mRumbleHandle[0].mVal != 0)
    {
        RumbleManager* v3 = RumbleManager::Inst(0);
        v3->SetIntensity(mRumbleHandle[0], scale);
    }
}

// ea: 0x004CE7B0
void AbstractEffectShakeAndRumble::StopEffect()
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
            g_cameraShake->StopCameraShake(mShake[0]);
            mShake[0] = nullptr;
        }
    }
}

// ============================================================================
// AbstractEffect fades + particle IsFinished
// ============================================================================

// ea: 0x004E2D40
bool AbstractEffect::IsFinishedFading()
{
    return (mCodeFlags.mVal & 1) != 0 && mFadeTime < 0.001f;
}

// ea: 0x004CDC90
bool AbstractEffectParticle::IsFinished()
{
    if ((mCodeFlags.mVal & 2) == 0)
        return false;
    unsigned int v2 = mEntity.mHandle.mVal & 0xFFF;
    if ((v2 >= 0x540
         || mEntity.mHandle.mVal >> 12
                != EntityHandleDb::sInst.mElements[v2].mKey
         || EntityHandleDb::sInst.mElements[v2].mObject == nullptr)
        && (mFlags & 1) == 0)
    {
        return true;
    }
    if ((mCodeFlags.mVal & 1) != 0 && mFadeTime <= 0.0f)
        return true;
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle == nullptr || mParticle->mEffect == nullptr)
        return true;
    if ((mCodeFlags.mVal & 1) != 0 && !IsFinishedFading())
        return false;
    return mParticle->mEffect->IsDone() != 0;
}
