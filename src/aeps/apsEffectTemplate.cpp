// ============================================================================
// apsEffectTemplate — particle-effect template (31 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsEffectTemplate.cpp
// Verified against IDA (aeps_xboxr:apsEffectTemplate.o). All funcs exact.
// Data: kDefaultTimeScale(1.0f), sMaxElements, sMaxModifiers.
// Inline COMDATs (Element/Modifier ctor, SetName, Stats::Reset, apsArray<>
// methods, apsGetFields, apsRenderer::GetRequiredParticleFields) live in the
// headers and emit in this object per the map.
// ============================================================================

#include "apsEffect.h"
#include "apsActionList.h"
#include "apsRenderer.h"
#include "apsMemory.h"      // apsMemory::TestAlloc

#include <malloc.h>     // _alloca
#include <string.h>     // strcmp, memcpy, memset

extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void tlPrintf(const char* fmt, ...);

// ============================================================================
// static data
// ============================================================================
const float apsEffectTemplate::kDefaultTimeScale = 1.0f;
int apsEffectTemplate::sMaxElements;
int apsEffectTemplate::sMaxModifiers;

// ============================================================================
// ctor — name-only. apsEffectTemplate(const char* name).
// ============================================================================
apsEffectTemplate::apsEffectTemplate(const char* name) {
    math::Dir3 zero = apsVector3_Zero();
    mBoundingSphere.mSphere.v = _mm_set_ps(1.0f, zero.v.m128_f32[2],
                                           zero.v.m128_f32[1], zero.v.m128_f32[0]);
    mElements.mElements = 0;
    mElements.mCapacity = 0;
    mElements.mSize = 0;
    mModifiers.mElements = 0;
    mModifiers.mCapacity = 0;
    mModifiers.mSize = 0;
    mRefCount = 0;
    mMemoryNeeded = 0;
    mPriority = 64;
    mUsesUpdateLod = 1;
    apsMath::SetIdentityMatrix(mLToW);
    SetName(name);
    mStats.mTotalInstances = 0;
    mStats.mNumActiveInstances = 0;
    mStats.mMaxActiveInstances = 0;
    mStats.mNumFailedElements = 0;
    mStats.mNumFailedModifiers = 0;
    mStats.mbReported = 0;
}

// ============================================================================
// dtor
// ============================================================================
apsEffectTemplate::~apsEffectTemplate() {
    if (mRefCount != 0 &&
        _tlAssert("source/apsEffectTemplate.cpp", 81,
                  "mRefCount == 0",
                  "apsEffectTemplate deleted while still referenced by apsEffect"))
        __debugbreak();

    int num = mElements.mSize;
    int i = 0;
    while (i < num) {
        Element* el = &mElements.mElements[i];
        if (el->mRenderer != 0) {
            el->mRenderer->~apsRenderer();
            apsCommon::GetAllocator()->MemFree(el->mRenderer);
        }
        if (el->mActionList != 0) {
            el->mActionList->~apsActionList();
            apsCommon::GetAllocator()->MemFree(el->mActionList);
        }
        if (el->mNanoGraphName != 0) {
            apsCommon::GetAllocator()->MemFree(el->mNanoGraphName);
            el->mNanoGraphName = 0;
        }
        ++i;
    }

    if (mModifiers.mElements != 0) {
        int old = apsCommon::SetPakAllocs(0);
        apsCommon::GetAllocator()->MemFree(mModifiers.mElements);
        apsCommon::SetPakAllocs(old);
        mModifiers.mElements = 0;
        mModifiers.mCapacity = 0;
        mModifiers.mSize = 0;
    }
    if (mElements.mElements != 0) {
        int old = apsCommon::SetPakAllocs(0);
        apsCommon::GetAllocator()->MemFree(mElements.mElements);
        apsCommon::SetPakAllocs(old);
        mElements.mElements = 0;
        mElements.mCapacity = 0;
        mElements.mSize = 0;
    }
}

// ============================================================================
// Element::RebuildPFD — combine action/renderer/base fields into mPFD.
// ============================================================================
void apsEffectTemplate::Element::RebuildPFD() {
    if (mActionList != 0)
        mActionList->EnhancePFD(mPFD);
    if (mRenderer != 0)
        mPFD.AddFields(mRenderer->GetRequiredParticleFields());
    mPFD.AddFields(0x600);
}

// ============================================================================
// Element::SetNanoGraphName — strdup-style via apsCommon allocator.
// ============================================================================
void apsEffectTemplate::Element::SetNanoGraphName(const char* name) {
    if (mNanoGraphName != 0) {
        if (strcmp(mNanoGraphName, name) == 0)
            return;
        apsCommon::GetAllocator()->MemFree(mNanoGraphName);
        mNanoGraphName = 0;
    }
    if (name != 0) {
        unsigned int len = (unsigned int)strlen(name);
        if (len != 0) {
            char* buf = (char*)apsCommon::GetAllocator()->MemAlign(len + 1, 4);
            mNanoGraphName = buf;
            if (buf == 0 &&
                _tlAssert("source/apsEffectTemplate.cpp", 314,
                          "mNanoGraphName", "couldn't allocate space for mNanoGraphName"))
                __debugbreak();
            memcpy(mNanoGraphName, name, len + 1);
        }
    }
}

// ============================================================================
// BeginReport / Report
// ============================================================================
void apsEffectTemplate::BeginReport() {
    mStats.mbReported = 0;
    tlPrintf("maxElements,             %d\nmaxModifiers,            %d\n",
             sMaxElements, sMaxModifiers);
}

void apsEffectTemplate::Report() {
    tlPrintf("priority,             %d\n"
             "total instances,      %d\n"
             "active instances,     %d\n"
             "max active instances, %d\n"
             "failed elements,      %d\n"
             "failed modifiers,     %d\n",
             mPriority, mStats.mTotalInstances, mStats.mNumActiveInstances,
             mStats.mMaxActiveInstances, mStats.mNumFailedElements,
             mStats.mNumFailedModifiers);
    mStats.mbReported = 1;
}

// ============================================================================
// SetLocalToWorldTransform
// ============================================================================
void apsEffectTemplate::SetLocalToWorldTransform(const math::Mat43& iMatrix) {
    mLToW = iMatrix;
}

// ============================================================================
// per-element setters (assert index then assign)
// ============================================================================
void apsEffectTemplate::SetBeginTime(int iElementNum, float iVal) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 150,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    mElements[iElementNum].mBeginTime = iVal;
}

void apsEffectTemplate::SetEndTime(int iElementNum, float iVal) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 156,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    mElements[iElementNum].mEndTime = iVal;
}

void apsEffectTemplate::SetLocalSpace(int iElementNum, unsigned int iVal) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 162,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    mElements[iElementNum].SetLocalSpace(iVal);
}

void apsEffectTemplate::SetNanoGraphName(int iElementNum, const char* iVal) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 169,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    mElements[iElementNum].SetNanoGraphName(iVal);
}

void apsEffectTemplate::SetNanoWeight(int iElementNum, float iVal) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 176,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    mElements[iElementNum].SetNanoWeight(iVal);
}

// ============================================================================
// GetModifierNumber — linear scan by name hash.
// ============================================================================
int apsEffectTemplate::GetModifierNumber(const tlFixedString& iName) const {
    int i = 0;
    for (; i < mModifiers.mSize; ++i) {
        bool match = true;
        for (int h = 0; h < 8; ++h) {
            if ((&mModifiers.mElements[i].mName.hash)[h] != (&iName.hash)[h]) {
                match = false;
                break;
            }
        }
        if (match)
            return i;
    }
    return -1;
}

// ============================================================================
// SetModifier
// ============================================================================
void apsEffectTemplate::SetModifier(int iModifierNum, const tlFixedString& iName,
                                    int iElementNum, int iActionNum, int iParamNum) {
    if (iModifierNum >= mModifiers.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 268,
                  "iModifierNum < (int)mModifiers.size()", "Invalid index"))
        __debugbreak();
    mModifiers[iModifierNum].mName = iName;
    mModifiers[iModifierNum].mElementNum = iElementNum;
    mModifiers[iModifierNum].mActionNum = iActionNum;
    mModifiers[iModifierNum].mParamNum = iParamNum;
}

// ============================================================================
// SetEnableRendering / GetEnableRendering
// ============================================================================
void apsEffectTemplate::SetEnableRendering(int iElementNum, unsigned int iRender) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 278,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    mElements[iElementNum].SetRenderingEnable(iRender);
}

unsigned int apsEffectTemplate::GetEnableRendering(int iElementNum) const {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 284,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    return mElements[iElementNum].RenderingEnabled();
}

// ============================================================================
// Fixup — relocate internal pointers by this base address, fixup renderers.
// ============================================================================
void apsEffectTemplate::Fixup(const apsFixupParams& iFixupParams) {
    mElements.mElements = (Element*)((char*)mElements.mElements + (ptrdiff_t)this);
    for (int i = 0; i < mElements.mSize; ++i) {
        Element& el = mElements.mElements[i];
        if (el.mNanoGraphName != 0)
            el.mNanoGraphName = (char*)((char*)el.mNanoGraphName + (ptrdiff_t)this);
        if (el.mRenderer != 0) {
            el.mRenderer = (apsRenderer*)((char*)el.mRenderer + (ptrdiff_t)this);
            el.mRenderer->Fixup(iFixupParams);
        }
        if (el.mActionList != 0) {
            el.mActionList = (apsActionList*)((char*)el.mActionList + (ptrdiff_t)this);
            el.mActionList->Fixup(iFixupParams);
        }
    }
    if (mModifiers.mSize != 0)
        mModifiers.mElements = (Modifier*)((char*)mModifiers.mElements + (ptrdiff_t)this);
}

// ============================================================================
// CalcMemoryNeeded / TestAlloc / MemoryMatch
// ============================================================================
void apsEffectTemplate::CalcMemoryNeeded() {
    int effectSize = 0;
    Element* it = mElements.mElements;
    Element* it_end = &mElements.mElements[mElements.mSize];
    if (it == it_end) {
        mMemoryNeeded = 0;
        return;
    }
    do {
        if (it->mActionList != 0)
            it->mActionList->EnhancePFD(it->mPFD);
        if (it->mRenderer != 0)
            it->mPFD.AddFields(it->mRenderer->GetRequiredParticleFields());
        it->mPFD.AddFields(0x600);
        effectSize = it->mMaxNumParticles * it->mPFD.mStride + effectSize;
        ++it;
    } while (it != it_end);
    mMemoryNeeded = effectSize;
}

unsigned int apsEffectTemplate::TestAlloc() {
    Element* it = mElements.mElements;
    Element* it_end = &mElements.mElements[mElements.mSize];
    if (it == it_end)
        return 1;
    while (apsMemory::TestAlloc(it->mPFD.mStride * it->mMaxNumParticles)) {
        if (++it == it_end)
            return 1;
    }
    return 0;
}

unsigned int apsEffectTemplate::MemoryMatch(apsEffectTemplate* tmpl) {
    if (tmpl == this)
        return 1;
    int* pMemNeeded = (int*)_alloca(4 * mElements.mSize);
    if (pMemNeeded == 0 &&
        _tlAssert("source/apsEffectTemplate.cpp", 558,
                  "pMemNeeded", "failed to allocate stack memory"))
        __debugbreak();
    Element* it = mElements.mElements;
    int* dst = pMemNeeded;
    for (Element* i = &mElements.mElements[mElements.mSize]; it != i; ++dst) {
        *dst = it->mMaxNumParticles * it->mPFD.mStride;
        ++it;
    }
    int mSize = mElements.mSize;
    Element* t_it = tmpl->mElements.mElements;
    Element* t_end = &t_it[tmpl->mElements.mSize];
    if (t_it == t_end)
        return 1;
    while (1) {
        int best = -1;
        int closest = 0x40000000;
        int v10 = 0;
        if (mSize <= 0)
            break;
        do {
            int need = pMemNeeded[v10];
            if (need >= t_it->mMaxNumParticles * t_it->mPFD.mStride && need < closest) {
                closest = pMemNeeded[v10];
                best = v10;
            }
            ++v10;
        } while (v10 < mSize);
        if (best < 0)
            break;
        pMemNeeded[best] = -1;
        if (++t_it == t_end)
            return 1;
    }
    return 0;
}

// ============================================================================
// Element::Free
// ============================================================================
void apsEffectTemplate::Element::Free() {
    if (mRenderer != 0) {
        mRenderer->~apsRenderer();
        apsCommon::GetAllocator()->MemFree(mRenderer);
    }
    if (mActionList != 0) {
        mActionList->~apsActionList();
        apsCommon::GetAllocator()->MemFree(mActionList);
    }
    if (mNanoGraphName != 0) {
        apsCommon::GetAllocator()->MemFree(mNanoGraphName);
        mNanoGraphName = 0;
    }
}

// ============================================================================
// SetNumElements / AddElement / RemoveElement
// ============================================================================
void apsEffectTemplate::SetNumElements(int iNum) {
    if (iNum < mElements.mSize) {
        for (int i = 0; i < mElements.mSize; ++i) {
            mElements.mElements[i].Free();
        }
    }
    mElements.resize(iNum);
    if (iNum > sMaxElements)
        sMaxElements = iNum;
    CalcMemoryNeeded();
}

int apsEffectTemplate::AddElement() {
    int mSize = mElements.mSize;
    mElements.resize(mSize + 1);
    if (mElements.mSize > sMaxElements)
        sMaxElements = mElements.mSize;
    return mSize;
}

void apsEffectTemplate::RemoveElement(int iNum) {
    if (iNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 128,
                  "iNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    mElements.mElements[iNum].Free();
    mElements.erase(&mElements.mElements[iNum]);
    CalcMemoryNeeded();
}

// ============================================================================
// SetMaxNumParticles / SetRenderer / SetActionList / SetElementData
// ============================================================================
void apsEffectTemplate::SetMaxNumParticles(int iElementNum, int iVal) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 142,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    mElements[iElementNum].mMaxNumParticles = iVal;
    CalcMemoryNeeded();
}

void apsEffectTemplate::SetRenderer(int iElementNum, apsRenderer* iRenderer) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 184,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    if (mElements[iElementNum].mRenderer != 0) {
        mElements[iElementNum].mRenderer->~apsRenderer();
        apsCommon::GetAllocator()->MemFree(mElements[iElementNum].mRenderer);
    }
    mElements[iElementNum].mRenderer = iRenderer;
    CalcMemoryNeeded();
}

void apsEffectTemplate::SetActionList(int iElementNum, apsActionList* iActionList) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 192,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    if (mElements[iElementNum].mActionList != 0) {
        mElements[iElementNum].mActionList->~apsActionList();
        apsCommon::GetAllocator()->MemFree(mElements[iElementNum].mActionList);
    }
    mElements[iElementNum].mActionList = iActionList;
    CalcMemoryNeeded();
}

void apsEffectTemplate::SetElementData(int iElementNum, int iMaxNumParticles,
                                       float iBeginTime, float iEndTime,
                                       apsRenderer* iRenderer,
                                       apsActionList* iActionList,
                                       unsigned int iLocalSpace) {
    if (iElementNum >= mElements.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 208,
                  "iElementNum < (int)mElements.size()", "Invalid index"))
        __debugbreak();
    Element& el = mElements[iElementNum];
    el.mMaxNumParticles = iMaxNumParticles;
    el.mBeginTime = iBeginTime;
    el.mEndTime = iEndTime;
    el.SetLocalSpace(iLocalSpace);
    if (el.mRenderer != 0) {
        el.mRenderer->~apsRenderer();
        apsCommon::GetAllocator()->MemFree(el.mRenderer);
    }
    el.mRenderer = iRenderer;
    if (el.mActionList != 0) {
        el.mActionList->~apsActionList();
        apsCommon::GetAllocator()->MemFree(el.mActionList);
    }
    el.mActionList = iActionList;
    CalcMemoryNeeded();
}

// ============================================================================
// SetNumModifiers / AddModifier / RemoveModifier
// ============================================================================
void apsEffectTemplate::SetNumModifiers(int iNum) {
    mModifiers.resize(iNum);
    if (iNum > sMaxModifiers)
        sMaxModifiers = iNum;
}

int apsEffectTemplate::AddModifier() {
    int mSize = mModifiers.mSize;
    mModifiers.resize(mSize + 1);
    if (mModifiers.mSize > sMaxModifiers)
        sMaxModifiers = mModifiers.mSize;
    return mSize;
}

void apsEffectTemplate::RemoveModifier(int iModifierNum) {
    if (iModifierNum >= mModifiers.mSize &&
        _tlAssert("source/apsEffectTemplate.cpp", 246,
                  "iModifierNum < (int)mModifiers.size()", "Invalid index"))
        __debugbreak();
    mModifiers.erase(&mModifiers.mElements[iModifierNum]);
}
