// ============================================================================
// apsEffect — runtime particle effect (37 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsEffect.cpp
// Verified against IDA (aeps_xboxr:apsEffect.o).
// Pool: apsMemory::PoolAllocator<apsEffect> singleton (128-byte effects).
// ============================================================================

#include "apsEffect.h"
#include "apsActionList.h"
#include "apsGroup.h"
#include "apsGroupMgr.h"
#include "apsMemory.h"
#include "apsCommon.h"
#include "apsInternal.h"
#include "apsError.h"
#include "apsMath.h"

#include <string.h>

extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void tlPrintf(const char* fmt, ...);

// ============================================================================
// globals / data
// ============================================================================
float g_effectTime;   // ?g_effectTime@@3MA @0x14CE2E0

// apsMemory::PoolAllocator<apsEffect> singleton (COMDATs emitted here)
// declared in apsMemory.h; sInstancePtr lives at 0x10DEDDC.

// ============================================================================
// apsBlockArray<T> template methods (inline COMDATs emitted in apsEffect.o)
// ============================================================================
template <typename T>
apsBlockArray<T>::apsBlockArray(int iSize, const T& iFillValue) {
    mElements = construct_array(iSize, iSize);
    mCapacity = (short)iSize;
    mSize = (short)iSize;
    for (int i = 0; i < iSize; ++i)
        mElements[i] = iFillValue;
}

template <typename T>
int apsBlockArray<T>::reserve(int iCapacity) {
    if (iCapacity <= mCapacity)
        return 1;
    T* buf = resize_array(iCapacity, mSize);
    if (buf == 0)
        return 0;
    mElements = buf;
    mCapacity = (short)iCapacity;
    return 1;
}

template <typename T>
int apsBlockArray<T>::push_back(const T& iElement) {
    if (mSize < mCapacity) {
        mElements[mSize] = iElement;
        ++mSize;
        return 1;
    }
    int newCapacity = mSize + 4;
    T* buf = resize_array(newCapacity, mSize);
    if (buf == 0)
        return 0;
    mElements = buf;
    mCapacity = (short)newCapacity;
    mElements[mSize] = iElement;
    ++mSize;
    return 1;
}

template <typename T>
T* apsBlockArray<T>::construct_array(int iNumber) {
    int old = apsCommon::SetCurrentPakId(-1);
    int pak = apsCommon::SetPakAllocs(1);
    T* buf = (T*)apsCommon::GetAllocator()->MemAlign(sizeof(T) * iNumber, 4);
    apsCommon::SetPakAllocs(pak);
    apsCommon::SetCurrentPakId(old);
    return buf;
}

template <typename T>
T* apsBlockArray<T>::construct_array(int iNumber, int iSize) {
    int old = apsCommon::SetCurrentPakId(-1);
    int pak = apsCommon::SetPakAllocs(1);
    T* buf = (T*)apsCommon::GetAllocator()->MemAlign(sizeof(T) * iNumber, 4);
    apsCommon::SetPakAllocs(pak);
    apsCommon::SetCurrentPakId(old);
    if (buf != 0) {
        for (int i = 0; i < iSize; ++i)
            new (&buf[i]) T();
    }
    return buf;
}

template <typename T>
T* apsBlockArray<T>::resize_array(int iCapacity, int iSize) {
    if (mElements == 0)
        return 0;
    int old = apsCommon::SetCurrentPakId(-1);
    int pak = apsCommon::SetPakAllocs(1);
    T* buf = (T*)apsCommon::GetAllocator()->MemResize(mElements, sizeof(T) * iCapacity, 4);
    apsCommon::SetPakAllocs(pak);
    apsCommon::SetCurrentPakId(old);
    return buf;
}

template <typename T>
void apsBlockArray<T>::destroy_all() {
    if (mElements != 0) {
        int old = apsCommon::SetCurrentPakId(-1);
        int pak = apsCommon::SetPakAllocs(1);
        apsCommon::GetAllocator()->MemFree(mElements);
        apsCommon::SetPakAllocs(pak);
        apsCommon::SetCurrentPakId(old);
        mElements = 0;
        mCapacity = 0;
        mSize = 0;
    }
}

template int apsBlockArray<apsGroup*>::size() const;
template apsGroup*& apsBlockArray<apsGroup*>::operator[](int);

// ============================================================================
// `anonymous namespace'::GetLightInfo
// ============================================================================
namespace {
__declspec(noinline) void GetLightInfo(const apsEffect* effect, apsLight::LightInfo* outInfo) {
    math::Position3 pos;
    pos.v = effect->mLToW.w.v;
    apsCommon::GetClient()->GetLightInfoAtPosition(*(math::Dir3*)&pos, *outInfo);
}
}

// ============================================================================
// ctor / dtor / Construct / Destruct
// ============================================================================
apsEffect::apsEffect(const apsEffectTemplate* iTemplate, float iStartTime) {
    mStartTime = iStartTime;
    mLastTime = 0.0f;
    mTemplate = iTemplate;
    mStopEmitting = 0;
    mNumCreated = 0;
    mFlags = 0;
    if (mTemplate == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182, "mTemplate", "null template"))
        __debugbreak();

    float fill = 0.0f;
    mGroups = apsBlockArray<apsGroup*>((int)mTemplate->mElements.mSize, *(apsGroup**)&fill);
    mModifiers.mElements = 0;
    mModifiers.mCapacity = 0;
    mModifiers.mSize = 0;
    apsMath::SetIdentityMatrix(mLToW);
    apsEffectTemplate* tmpl = const_cast<apsEffectTemplate*>(mTemplate);

    int mSize = (int)mTemplate->mModifiers.mSize;
    if (mSize != 0)
        mModifiers.reserve(mSize);
    for (int v = 0; v < mSize; ++v) {
        const apsEffectTemplate::Modifier& tm = mTemplate->GetModifier(v);
        apsActionList* actionList = mTemplate->GetElement(tm.mElementNum).mActionList;
        Modifier new_modifier;
        new_modifier.mAction = actionList->GetAction(tm.mActionNum);
        new_modifier.mParamNum = tm.mParamNum;
        if ((new_modifier.mParamNum < 0 || new_modifier.mParamNum >= new_modifier.mAction->mParams.mSize) &&
            _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                      "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            __debugbreak();
        new_modifier.mVal = new_modifier.mAction->mParams.mElements[new_modifier.mParamNum];
        mModifiers.push_back(new_modifier);
    }

    ++tmpl->mRefCount;
    ++tmpl->mStats.mTotalInstances;
    int total = tmpl->mStats.mTotalInstances;
    int active = tmpl->mStats.mNumActiveInstances;
    int maxActive = tmpl->mStats.mMaxActiveInstances;
    int v15 = active + 1;
    tmpl->mStats.mTotalInstances = total + 1;
    tmpl->mStats.mNumActiveInstances = v15;
    if (v15 > maxActive)
        tmpl->mStats.mMaxActiveInstances = v15;
    mParentAgePercent = -1.0f;
    mCollisionData = 0;
    mRaycastCountdown = 0;
    mTimeToNextRemoveCheck = 0.0f;
}

void apsEffect::TheRealInitializeLOL() {
    mParentAgePercent = -1.0f;
    mCollisionData = 0;
    mRaycastCountdown = 0;
    mTimeToNextRemoveCheck = 0.0f;
}

unsigned int apsEffect::Construct(const apsEffectTemplate* iTemplate, float iStartTime) {
    mParentAgePercent = -1.0f;
    mCollisionData = 0;
    mRaycastCountdown = 0;
    mTimeToNextRemoveCheck = 0.0f;
    mId = apsCommon::GetCurrentPakId();
    mStartTime = *((float*)&iStartTime);
    mLastTime = 0.0f;
    mTemplate = iTemplate;
    mStopEmitting = 0;
    mNumCreated = 0;
    mFlags = 0;
    apsMath::SetIdentityMatrix(mLToW);
    if (mTemplate == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182, "mTemplate", "null template"))
        __debugbreak();

    int mSize = (int)mTemplate->mElements.mSize;
    if (mTemplate->mElements.mSize != 0) {
        if (mGroups.reserve((int)mTemplate->mElements.mSize) == 0) {
            ++const_cast<apsEffectTemplate*>(mTemplate)->mStats.mNumFailedElements;
            return 0;
        }
    } else {
        if (apsSingleton<apsError>::InstancePtr() == 0 &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        apsSingleton<apsError>::InstancePtr()->AddError(apsError::ERROR_TYPE_WARNING,
                           "apsEffect : effect %s has no elements", mTemplate->mName);
    }
    if (mSize > 0) {
        float* zero = 0;
        do {
            mGroups.push_back((apsGroup*&)zero);
            --mSize;
        } while (mSize != 0);
    }

    int v9 = (int)mTemplate->mModifiers.mSize;
    if (v9 == 0 || mModifiers.reserve(v9) != 0) {
        for (int v11 = 0; v11 < v9; ++v11) {
            const apsEffectTemplate::Modifier& tm = mTemplate->GetModifier(v11);
            apsActionList* actionList = mTemplate->GetElement(tm.mElementNum).mActionList;
            Modifier new_modifier;
            new_modifier.mAction = actionList->GetAction(tm.mActionNum);
            new_modifier.mParamNum = tm.mParamNum;
            if ((new_modifier.mParamNum < 0 || new_modifier.mParamNum >= new_modifier.mAction->mParams.mSize) &&
                _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                          "iIndex >= 0 && iIndex < mSize", "out of bounds"))
                __debugbreak();
            new_modifier.mVal = new_modifier.mAction->mParams.mElements[new_modifier.mParamNum];
            mModifiers.push_back(new_modifier);
        }
        ++const_cast<apsEffectTemplate*>(mTemplate)->mRefCount;
        ++const_cast<apsEffectTemplate*>(mTemplate)->mStats.mTotalInstances;
        int total = mTemplate->mStats.mTotalInstances;
        int active = mTemplate->mStats.mNumActiveInstances;
        int maxActive = mTemplate->mStats.mMaxActiveInstances;
        int v21 = active + 1;
        const_cast<apsEffectTemplate*>(mTemplate)->mStats.mTotalInstances = total + 1;
        const_cast<apsEffectTemplate*>(mTemplate)->mStats.mNumActiveInstances = v21;
        if (v21 > maxActive)
            const_cast<apsEffectTemplate*>(mTemplate)->mStats.mMaxActiveInstances = v21;
        return 1;
    } else {
        ++const_cast<apsEffectTemplate*>(mTemplate)->mStats.mNumFailedModifiers;
        return 0;
    }
}

unsigned int apsEffect::Destruct(unsigned int bKillQuickly) {
    if (bKillQuickly != 0)
        mFlags |= 4u;
    this->~apsEffect();
    return 1;
}

apsEffect::~apsEffect() {
    apsGroupManager* groupManager = apsSingleton<apsGroupManager>::InstancePtr();
    unsigned int bFastDeath = mFlags & 4;
    apsGroup** it = mGroups.mElements;
    apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
    for (; it != it_end; ++it) {
        apsGroup* g = *it;
        if (groupManager != 0 && g != 0)
            groupManager->DestroyGroup(g, bFastDeath);
    }

    if (mModifiers.mElements != 0) {
        int old = apsCommon::SetCurrentPakId(-1);
        int pak = apsCommon::SetPakAllocs(1);
        apsCommon::GetAllocator()->MemFree(mModifiers.mElements);
        apsCommon::SetPakAllocs(pak);
        apsCommon::SetCurrentPakId(old);
        mModifiers.mElements = 0;
        mModifiers.mCapacity = 0;
        mModifiers.mSize = 0;
    }
    if (mGroups.mElements != 0) {
        int old = apsCommon::SetCurrentPakId(-1);
        int pak = apsCommon::SetPakAllocs(1);
        apsCommon::GetAllocator()->MemFree(mGroups.mElements);
        apsCommon::SetPakAllocs(pak);
        apsCommon::SetCurrentPakId(old);
        mGroups.mElements = 0;
        mGroups.mCapacity = 0;
        mGroups.mSize = 0;
    }

    if (mTemplate != 0)
        --const_cast<apsEffectTemplate*>(mTemplate)->mRefCount;
    if (mTemplate != 0)
        --const_cast<apsEffectTemplate*>(mTemplate)->mStats.mNumActiveInstances;
    if (mCollisionData != 0) {
        apsMemFree(mCollisionData);
        mCollisionData = 0;
        mRaycastCountdown = 0;
    }
    if (mModifiers.mElements != 0) {
        int old = apsCommon::SetCurrentPakId(-1);
        int pak = apsCommon::SetPakAllocs(1);
        apsCommon::GetAllocator()->MemFree(mModifiers.mElements);
        apsCommon::SetPakAllocs(pak);
        apsCommon::SetCurrentPakId(old);
        mModifiers.mElements = 0;
        mModifiers.mCapacity = 0;
        mModifiers.mSize = 0;
    }
    if (mGroups.mElements != 0) {
        int old = apsCommon::SetCurrentPakId(-1);
        int pak = apsCommon::SetPakAllocs(1);
        apsCommon::GetAllocator()->MemFree(mGroups.mElements);
        apsCommon::SetPakAllocs(pak);
        apsCommon::SetCurrentPakId(old);
        mGroups.mElements = 0;
        mGroups.mCapacity = 0;
        mGroups.mSize = 0;
    }
}

// ============================================================================
// pool (static)
// ============================================================================
void apsEffect::InitPool(int maxEffects) {
    void* mem = apsCommon::GetAllocator()->MemAlign(44, 16);
    if (mem != 0)
        new (mem) apsMemory::PoolAllocator<apsEffect>(maxEffects);
}

void apsEffect::TermPool() {
    if (apsSingleton<apsMemory::PoolAllocator<apsEffect>>::InstancePtr() != 0) {
        apsMemory::PoolAllocator<apsEffect>* p = apsSingleton<apsMemory::PoolAllocator<apsEffect>>::InstancePtr();
        p->~PoolAllocator();
        apsCommon::GetAllocator()->MemFree(p);
    }
}

apsEffect* apsEffect::New(const apsEffectTemplate* iTemplate, float iStartTime) {
    apsMemory::PoolAllocator<apsEffect>* pool = apsSingleton<apsMemory::PoolAllocator<apsEffect>>::InstancePtr();
    if (pool == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    apsEffect* v2 = (apsEffect*)pool->mPool.Alloc(128);
    if (v2 == 0) {
        if (apsSingleton<apsError>::InstancePtr() == 0 &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        apsSingleton<apsError>::InstancePtr()->AddError(apsError::ERROR_TYPE_WARNING,
                           "apsEffect : pool is empty");
    } else if (v2->Construct(iTemplate, *(int*)&iStartTime) == 0) {
        if (apsSingleton<apsError>::InstancePtr() == 0 &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        apsSingleton<apsError>::InstancePtr()->AddError(apsError::ERROR_TYPE_WARNING,
                           "apsEffect : failed to create effect %s", iTemplate->mName);
        v2->~apsEffect();
        return 0;
    }
    return v2;
}

void apsEffect::Delete(apsEffect* pEffect, unsigned int bKillQuickly) {
    if (pEffect != 0) {
        if (bKillQuickly != 0)
            pEffect->mFlags |= 4u;
        pEffect->~apsEffect();
        apsMemory::PoolAllocator<apsEffect>* pool = apsSingleton<apsMemory::PoolAllocator<apsEffect>>::InstancePtr();
        if (pool == 0 &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        pool->mPool.Free(pEffect);
    }
}

unsigned int apsEffect::TestAlloc(apsEffectTemplate* tmpl) {
    apsMemory::PoolAllocator<apsEffect>* pool = apsSingleton<apsMemory::PoolAllocator<apsEffect>>::InstancePtr();
    if (pool == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    if (pool->mPool.mNumBlocks == pool->mPool.mNumBlocksUsed)
        return 0;
    apsGroupManager* gm = apsSingleton<apsGroupManager>::InstancePtr();
    if (gm == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return gm->TestAlloc((int)tmpl->mElements.mSize);
}

unsigned int apsEffect::IsPoolEmpty() {
    apsMemory::PoolAllocator<apsEffect>* pool = apsSingleton<apsMemory::PoolAllocator<apsEffect>>::InstancePtr();
    if (pool == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return (unsigned int)(pool->mPool.mNumBlocksUsed == pool->mPool.mNumBlocks);
}

int apsEffect::GetNumActiveEffects() {
    apsMemory::PoolAllocator<apsEffect>* pool = apsSingleton<apsMemory::PoolAllocator<apsEffect>>::InstancePtr();
    if (pool != 0)
        return pool->mPool.mNumBlocksUsed;
    if (_tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsSingleton<apsMemory::PoolAllocator<apsEffect>>::InstancePtr()->mPool.mNumBlocksUsed;
}

// ============================================================================
// misc
// ============================================================================
void apsEffect::StopEmitting() { mStopEmitting = 1; }

// ea: 0x00518650
int apsEffect::IsVisible()
{
    return mFlags & 2;
}

unsigned int apsEffect::GetRaycastCountdownMaxValue() { return 3; }

void apsEffect::IncrementRaycastCountdown() {
    unsigned int v = mRaycastCountdown + 1;
    mRaycastCountdown = v;
    if (v > 3)
        mRaycastCountdown = 0;
}

int apsEffect::GetModifierId(const char* name) {
    if (mTemplate == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182, "mTemplate", "null template"))
        __debugbreak();
    tlFixedString v6(name);
    return mTemplate->GetModifierNumber(v6);
}

const apsEffect::RaycastResult& apsEffect::GetRaycastResult(unsigned int id) const {
    if (id >= MAX_RAYCAST_LIST_SIZE &&
        _tlAssert("source/apsEffect.cpp", 1063, "id < MAX_RAYCAST_LIST_SIZE", "id < MAX_RAYCAST_LIST_SIZE"))
        __debugbreak();
    if (mCollisionData == 0 &&
        _tlAssert("source/apsEffect.cpp", 1064, "mCollisionData", "mCollisionData"))
        __debugbreak();
    return mCollisionData->mRaycastResults[id];
}

void apsEffect::AccumulateCollisionBounds(const apsBounds& bounds) {
    if (mCollisionData != 0) {
        mCollisionData->mBounds.mMin.v = _mm_min_ps(mCollisionData->mBounds.mMin.v, bounds.mMin.v);
        mCollisionData->mBounds.mMax.v = _mm_max_ps(mCollisionData->mBounds.mMax.v, bounds.mMax.v);
    }
}

unsigned int apsEffect::RequestRaycast(const math::Dir3& start, const math::Dir3& end) {
    if (mCollisionData == 0) {
        CollisionData* v5 = (CollisionData*)apsMemAlloc(0xC30, 0x10, 0);
        if (v5 != 0) {
            v5->mBounds.Init();
        } else {
            v5 = 0;
        }
        mCollisionData = v5;
        mCollisionData->mNumRaycastRequests = 0;
        for (int i = 0; i < 32; ++i)
            mCollisionData->mRaycastRequests[i].resultID = (unsigned int)i;
        for (int j = 0; j < 32; j += 8) {
            mCollisionData->mRaycastResults[j + 0].t = -1.0f;
            mCollisionData->mRaycastResults[j + 1].t = -1.0f;
            mCollisionData->mRaycastResults[j + 2].t = -1.0f;
            mCollisionData->mRaycastResults[j + 3].t = -1.0f;
            mCollisionData->mRaycastResults[j + 4].t = -1.0f;
            mCollisionData->mRaycastResults[j + 5].t = -1.0f;
            mCollisionData->mRaycastResults[j + 6].t = -1.0f;
            mCollisionData->mRaycastResults[j + 7].t = -1.0f;
        }
    }
    unsigned int n = mCollisionData->mNumRaycastRequests;
    if (n >= MAX_RAYCAST_LIST_SIZE)
        return (unsigned int)-1;
    mCollisionData->mRaycastRequests[n].start.v = start.v;
    mCollisionData->mRaycastRequests[n].end.v = end.v;
    unsigned int result = mCollisionData->mNumRaycastRequests;
    mCollisionData->mNumRaycastRequests = result + 1;
    return result;
}

void apsEffect::ReportTemplate() {
    const apsEffectTemplate* t = mTemplate;
    if (t != 0 && t->mStats.mbReported == 0) {
        int i = 0;
        apsGroup** it = mGroups.mElements;
        apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
        for (; it != it_end; ++it) {
            if (*it != 0)
                i += (*it)->mPFD.mStride * (*it)->mMaxNumParticles;
        }
        tlPrintf("Effect Template, %s\nsize estimate,        %d bytes\nactual size,          %d bytes\n",
                 mTemplate->mName, mTemplate->mMemoryNeeded, i);
        const_cast<apsEffectTemplate*>(mTemplate)->Report();
        tlPrintf("\n\n");
    }
}

void apsEffect::SetLocalToWorldTransform(const math::Mat43& iMatrix) {
    if (mTemplate == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182, "mTemplate", "null template"))
        __debugbreak();
    apsMath::Multiply43(mLToW, mTemplate->mLToW, iMatrix);
    apsGroup** it = mGroups.mElements;
    apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
    for (; it != it_end; ++it) {
        if (*it != 0)
            (*it)->SetGroupLocalToWorldTransform(mLToW);
    }
}

int apsEffect::CountParticles() {
    int result = 0;
    apsGroup** it = mGroups.mElements;
    apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
    for (; it != it_end; ++it) {
        if (*it != 0)
            result += (*it)->mNumParticles;
    }
    return result;
}

void apsEffect::SetModifierValue(int iNum, float iVal) {
    if (iNum >= mModifiers.mSize &&
        _tlAssert("source/apsEffect.cpp", 837, "iNum < (int)mModifiers.size()", "Index out of range"))
        __debugbreak();
    if ((iNum < 0 || iNum >= mModifiers.mSize) &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 505,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    mModifiers.mElements[iNum].mVal = iVal;
}

float apsEffect::GetModifierValue(int iNum) {
    if (iNum >= mModifiers.mSize &&
        _tlAssert("source/apsEffect.cpp", 846, "iNum < (int)mModifiers.size()", "Index out of range"))
        __debugbreak();
    if ((iNum < 0 || iNum >= mModifiers.mSize) &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 505,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    return mModifiers.mElements[iNum].mVal;
}

float apsEffect::GetModifierTemplateValue(int iNum) {
    if (iNum >= mModifiers.mSize &&
        _tlAssert("source/apsEffect.cpp", 855, "iNum < (int)mModifiers.size()", "Index out of range"))
        __debugbreak();
    if (mTemplate == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182, "mTemplate", "null template"))
        __debugbreak();
    int elementNum = mTemplate->GetModifier(iNum).mElementNum;
    int mActionNum = mTemplate->GetModifier(iNum).mActionNum;
    int mParamNum = mTemplate->GetModifier(iNum).mParamNum;
    const apsEffectTemplate::Element& el = mTemplate->GetElement(elementNum);
    apsAction* action = el.mActionList->GetAction(mActionNum);
    return action->GetParam(mParamNum);
}

void apsEffect::swap_modifiers() {
    apsEffect::Modifier* it = mModifiers.mElements;
    apsEffect::Modifier* it_end = &mModifiers.mElements[mModifiers.mSize];
    for (; it != it_end; ++it) {
        int mParamNum = it->mParamNum;
        if ((mParamNum < 0 || mParamNum >= it->mAction->mParams.mSize) &&
            _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                      "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            __debugbreak();
        float cur_val = it->mAction->mParams.mElements[mParamNum];
        it->mAction->SetParam(it->mParamNum, it->mVal);
        it->mVal = cur_val;
    }
}

void apsEffect::Render(nglLightContext* iLightContext, const VFC::FrustumInfo& frustumInfo) {
    mFlags &= ~2u;
    apsLight::LightInfo linfo;
    bool dynLit = false;
    apsGroup** it = mGroups.mElements;
    apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
    for (; it != it_end; ++it) {
        apsGroup* g = *it;
        if (g != 0) {
            if (!dynLit && g->mRenderer != 0 && g->mRenderer->mIsDynamicallyLit != 0) {
                math::Position3 pos;
                pos.v = mLToW.w.v;
                apsCommon::GetClient()->GetLightInfoAtPosition(*(math::Dir3*)&pos, linfo);
                dynLit = true;
            }
            if (g->Render(linfo, frustumInfo, iLightContext) == apsRenderer::RENDERRESULT_VISIBLE)
                mFlags |= 2u;
        }
    }
}

// ea: 0x007EE300
void apsEffect::GetBounds(apsBounds& iBounds) {
    apsBounds v9;
    v9.Init();
    apsGroup** it = mGroups.mElements;
    apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
    if (it != it_end) {
        do {
            apsGroup* g = *it;
            if (g != 0 && g->mNumParticles != 0) {
                v9.mMin.v = _mm_min_ps(v9.mMin.v, g->mBounds.mMin.v);
                v9.mMax.v = _mm_max_ps(v9.mMax.v, g->mBounds.mMax.v);
            }
            ++it;
        } while (it != it_end);
    }
    iBounds = v9;
}

void apsEffect::SetPosition(const math::Dir3& pos) {
    apsGroup** it = mGroups.mElements;
    apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
    for (; it != it_end; ++it) {
        if (*it != 0)
            (*it)->SetGroupPosition(pos);
    }
}

void apsEffect::SetCulled(unsigned int bCulled) {
    apsGroup** it = mGroups.mElements;
    apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
    for (; it != it_end; ++it) {
        if (*it != 0)
            (*it)->mFlags = (unsigned short)(((*it)->mFlags & 0xFFEF) | (bCulled != 0 ? 0x10 : 0));
    }
}

void apsEffect::Report(int index) {
    if (mTemplate == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182, "mTemplate", "null template"))
        __debugbreak();
    const char* name = mTemplate->mName;
    if (mTemplate == (const apsEffectTemplate*)-116)
        name = "unknown";
    const char* status = "emitting";
    if (mStopEmitting != 0)
        status = "stopping";
    int total = 0;
    apsGroup** it = mGroups.mElements;
    apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
    for (; it != it_end; ++it) {
        if (*it != 0)
            total += (*it)->mNumParticles;
    }
    tlPrintf("Effect %d\n"
             "name,            %s\n"
             "num elements,    %d\n"
             "total particles, %d\n"
             "start time,      %f\n"
             "status,          %s\n"
             "sortkey,         %08x\n",
             index, name, mGroups.mSize, total, mStartTime, status, mSortKey._32);
    apsBounds b;
    b.Init();
    GetBounds(b);
    tlPrintf("min bound,       %f, %f, %f\n",
             b.mMin.v.m128_f32[0], b.mMin.v.m128_f32[1], b.mMin.v.m128_f32[2]);
    tlPrintf("max bound,       %f, %f, %f\n\n",
             b.mMax.v.m128_f32[0], b.mMax.v.m128_f32[1], b.mMax.v.m128_f32[2]);
    int idx = 0;
    apsGroup** j = mGroups.mElements;
    apsGroup** j_end = &mGroups.mElements[mGroups.mSize];
    for (; j != j_end; ++idx) {
        apsGroup* g = *j;
        tlPrintf(",Element, %d\n", idx);
        if (g != 0) {
            tlPrintf(",  max particles,  %10d\n,  used,           %10d\n,  peak,           %10d\n",
                     g->mMaxNumParticles, g->mNumParticles, g->mMaxUsedParticles);
            tlPrintf(",  last update,   %f\n,  delayed,       %fs\n", g->mUpdateTime, g->mUpdateDelay);
            tlPrintf(",  last pos,      %f, %f, %f\n",
                     g->mLastPos.v.m128_f32[0], g->mLastPos.v.m128_f32[1], g->mLastPos.v.m128_f32[2]);
            const char* vis = "visible";
            if ((g->mFlags & 8) == 0)
                vis = "not visible";
            tlPrintf(",  distance,      %f\n,  %s\n", g->mBoundSphereDistanceFromCamera, vis);
        } else {
            tlPrintf(",  empty\n");
        }
        tlPrintf("\n\n");
        ++j;
    }
    tlPrintf("\n\n");
}

void apsEffect::Update(float iCurTime) {
    if (mTemplate == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182, "mTemplate", "null template"))
        __debugbreak();
    float local_time = (iCurTime - mStartTime) * mTemplate->mTimeScale;
    float begin_time = local_time - mLastTime;
    if (local_time >= 0.0f && begin_time >= 0.0f) {
        if (mModifiers.mSize > 0)
            swap_modifiers();
        g_effectTime = iCurTime;

        apsGroup** it = mGroups.mElements;
        apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
        apsGroup** itSave = it;
        for (; it != it_end; ++it) {
            apsGroup* g = *it;
            if (g != 0 && (g->mFlags & 4) != 0) {
                if (apsSingleton<apsGroupManager>::InstancePtr() == 0 &&
                    _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                              "sInstancePtr", "singleton not initialised"))
                    __debugbreak();
                apsSingleton<apsGroupManager>::InstancePtr()->DestroyGroup(g, 0);
                it_end = &mGroups.mElements[mGroups.mSize];
                *it = 0;
            }
        }

        mTimeToNextRemoveCheck -= begin_time;
        unsigned int doChanceToRemove = 0;
        if (mTimeToNextRemoveCheck <= 0.0f) {
            mTimeToNextRemoveCheck = apsMath::gDefaultRandomNumberGenerator.GetFloat() * 0.1f;
            doChanceToRemove = 1;
        }

        int v9 = 0;
        apsGroup** v10 = mGroups.mElements;
        apsGroup** v25 = &mGroups.mElements[mGroups.mSize];
        if (v10 != v25) {
            while (1) {
                const apsEffectTemplate::Element& el = mTemplate->GetElement(v9);
                apsGroup* g = *v10;
                float end_time = el.mEndTime;
                if (g != 0) {
                    apsRenderer* renderer = mTemplate->GetElement(v9).mRenderer;
                    if (renderer != 0) {
                        g->mRenderer = renderer;
                        g->mFlags = (unsigned short)((g->mFlags & 0xFFFE) |
                                    (mTemplate->GetEnableRendering(v9) != 0 ? 1 : 0));
                    } else {
                        g->mFlags = (unsigned short)(g->mFlags & 0xFFFE);
                        g->mRenderer = 0;
                    }
                    apsActionList* actionList = mTemplate->GetElement(v9).mActionList;
                    const apsEffectTemplate::Element& el2 = mTemplate->GetElement(v9);
                    actionList->Apply(g, *this, local_time - el2.mBeginTime, begin_time,
                                      (unsigned int)(mStopEmitting != 0), doChanceToRemove);
                    if (local_time >= end_time)
                        g->mFlags = (unsigned short)(g->mFlags | 4u);
                }
                ++v9;
                if (++v10 == v25)
                    break;
            }
        }

        int v16 = 0;
        apsGroup** v15 = mGroups.mElements;
        apsGroup** it_end2 = &mGroups.mElements[mGroups.mSize];
        if (v15 != it_end2) {
            do {
                float mEndTime = mTemplate->GetElement(v16).mEndTime;
                if (*v15 == 0) {
                    float mBeginTime = mTemplate->GetElement(v16).mBeginTime;
                    if (mStopEmitting == 0 && local_time >= mBeginTime && mEndTime > local_time) {
                        const apsEffectTemplate::Element& el3 = mTemplate->GetElement(v16);
                        const apsEffectTemplate::Element& el4 = mTemplate->GetElement(v16);
                        apsActionList* actionList2 = mTemplate->GetElement(v16).mActionList;
                        apsRenderer* renderer2 = mTemplate->GetElement(v16).mRenderer;
                        int mMaxNumParticles = mTemplate->GetElement(v16).mMaxNumParticles;
                        if (apsSingleton<apsGroupManager>::InstancePtr() == 0 &&
                            _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                                      "sInstancePtr", "singleton not initialised"))
                            __debugbreak();
                        apsGroup* group = apsSingleton<apsGroupManager>::InstancePtr()->CreateGroup(
                            mMaxNumParticles, renderer2, actionList2->mActions.mSize ? el4.mPFD : el4.mPFD,
                            (el3.mFlags & 2) != 0);
                        if (group != 0) {
                            if ((mFlags & 1) != 0) {
                                apsRenderer* r = group->mRenderer;
                                if (r != 0 && r->IsCameraFacing() == 0) {
                                    if (group->mRenderer != 0)
                                        group->mRenderer->SetScreenFacingNormal(mLToW.z);
                                }
                            }
                            group->SetGroupLocalToWorldTransform(mLToW);
                            *v15 = group;
                            ++mNumCreated;
                            group->mUpdateTime = local_time - mBeginTime;
                            const apsEffectTemplate::Element& el5 = mTemplate->GetElement(v16);
                            el5.mActionList->Apply(group, *this, local_time - mBeginTime,
                                                   local_time - mBeginTime, 0, 0);
                        }
                    }
                }
                ++v15;
                ++v16;
            } while (v15 != it_end2);
        }

        if (mModifiers.mSize > 0)
            swap_modifiers();
        mLastTime = local_time;
    }
}

// ea: 0x007EF460
unsigned int apsEffect::IsDone() {
    if (mStopEmitting != 0) {
        int total = 0;
        apsGroup** it = mGroups.mElements;
        apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
        if (it == it_end)
            return 1;
        do {
            if (*it != 0)
                total += (*it)->mNumParticles;
            ++it;
        } while (it != it_end);
        if (total == 0)
            return 1;
    }
    if (mTemplate == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182, "mTemplate", "null template"))
        __debugbreak();
    int mSize = (int)mTemplate->mElements.mSize;
    int v6 = 0;
    if (mSize <= 0)
        return 1;
    while (1) {
        if (mTemplate->GetElement(v6).mEndTime > mLastTime)
            break;
        if (++v6 >= mSize)
            return 1;
    }
    return 0;
}

void apsEffect::FastForward(float deltaT, int numIncr) {
    mStartTime = mStartTime - deltaT;
    float target = mLastTime + deltaT;
    while (target > mLastTime)
        Update(mLastTime + mStartTime + 0.30000001f);
}

void apsEffect::CalcSortKey() {
    float dist = 1.0e19f;
    apsGroup** it = mGroups.mElements;
    apsGroup** it_end = &mGroups.mElements[mGroups.mSize];
    for (; it != it_end; ++it) {
        if (*it != 0 && dist > (*it)->mBoundSphereDistanceFromCamera)
            dist = (*it)->mBoundSphereDistanceFromCamera;
    }
    int v5 = (int)dist;
    if (dist >= 0) {
        if (v5 >= 0xFFFFFF)
            v5 = 0xFFFFFF;
    } else {
        v5 = 0;
    }
    unsigned int v6 = (unsigned int)v5 ^ mSortKey._32;
    mSortKey._32 ^= 0xFFFFFF & v6;
    if (mTemplate == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182, "mTemplate", "null template"))
        __debugbreak();
    int priority = mTemplate->mPriority;
    mSortKey._32 = (0x1000000 & ~((unsigned int)mFlags << 23)) |
                   (0xFFFFFF & mSortKey._32 & 0xFEFFFFFF) |
                   (((unsigned int)priority) << 25);
}

void apsEffect::ReportEffects() {
    tlPrintf("Effects\n\n");
    apsMemory::PoolAllocator<apsEffect>* pool = apsSingleton<apsMemory::PoolAllocator<apsEffect>>::InstancePtr();
    if (pool == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    apsMemory::Pool* p_pool = &pool->mPool;
    p_pool->Report();
    for (int i = 0; i < p_pool->mNumBlocks; ++i) {
        apsEffect* block = (apsEffect*)p_pool->GetActiveBlock(i);
        if (block != 0)
            const_cast<apsEffectTemplate*>(block->mTemplate)->BeginReport();
    }
    for (int j = 0; j < p_pool->mNumBlocks; ++j) {
        apsEffect* v4 = (apsEffect*)p_pool->GetActiveBlock(j);
        if (v4 != 0)
            v4->ReportTemplate();
    }
    int result = p_pool->mNumBlocks;
    for (int k = 0; k < p_pool->mNumBlocks; ++k) {
        apsEffect* v7 = (apsEffect*)p_pool->GetActiveBlock(k);
        if (v7 != 0)
            v7->Report(k);
        result = p_pool->mNumBlocks;
    }
}
