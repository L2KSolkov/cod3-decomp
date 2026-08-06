// ============================================================================
// apsAction.cpp — base particle-action class (5 non-inline funcs).
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsAction.cpp
//
// Port strategy (matches apsGroup.o / apsCommon.o precedent):
//   - All 5 non-inline functions verified against IDA disasm.
//   - apsArray<T>, apsVirtualBase, apsDestroy<T>, apsFixUp<T> are inline
//     COMDATs in the headers (apsAction.h), emitted per codmp_xboxr.map.
// ============================================================================
#include "apsAction.h"

#include <float.h>

// ============================================================================
// apsAction::apsAction — construct with a fixed param/domain array count.
// Params[0..1] default to [-FLT_MAX, +FLT_MAX] (the caller's min/max range).
// ea: 0x808500
// ============================================================================
apsAction::apsAction(int iNumParams, int iNumDomains, IterationStyle iIterationStyle,
                     unsigned int iRequiredParticleFields)
    : mParams(iNumParams, 0.0f),
      mDomains(iNumDomains, (apsDomain*)0) {
    mIterationStyle = iIterationStyle;
    mRequiredParticleFields = iRequiredParticleFields;
    SetParam(0, -FLT_MAX);
    SetParam(1, FLT_MAX);
}

// ============================================================================
// apsAction::~apsAction — free owned domains (virtual dtor + MemFree), then the
// param/domain arrays.
// ea: 0x808430
// ============================================================================
apsAction::~apsAction() {
    // Free any live domain objects.
    if (mDomains.mElements != 0) {
        for (int i = 0; i < mDomains.mSize; ++i) {
            apsDomain* domain = mDomains.mElements[i];
            if (domain != 0) {
                apsDestroy(domain);
                apsCommon::GetAllocator()->MemFree(domain);
            }
        }
    }
    // Free the domain array buffer (not pak-accounted).
    if (mDomains.mElements != 0) {
        int old = apsCommon::SetPakAllocs(0);
        apsCommon::GetAllocator()->MemFree(mDomains.mElements);
        apsCommon::SetPakAllocs(old);
        mDomains.mElements = 0;
        mDomains.mCapacity = 0;
        mDomains.mSize = 0;
    }
    // Free the param array buffer (not pak-accounted).
    if (mParams.mElements != 0) {
        int old = apsCommon::SetPakAllocs(0);
        apsCommon::GetAllocator()->MemFree(mParams.mElements);
        apsCommon::SetPakAllocs(old);
        mParams.mElements = 0;
        mParams.mCapacity = 0;
        mParams.mSize = 0;
    }
}

// ============================================================================
// apsAction::SetParam — store a float param (bounds-checked).
// ea: 0x8081F0
// ============================================================================
void apsAction::SetParam(int iParamNum, float iParam) {
    if (iParamNum >= mParams.mSize &&
        _tlAssert("source/apsAction.cpp", 41,
                  "iParamNum < mParams.size()", "param index out of range"))
        __debugbreak();
    mParams[iParamNum] = iParam;
}

// ============================================================================
// apsAction::SetDomain — store a domain pointer (bounds-checked).
// ea: 0x808260
// ============================================================================
void apsAction::SetDomain(int iDomainNum, apsDomain* iDomain) {
    if (iDomainNum >= mDomains.mSize &&
        _tlAssert("source/apsAction.cpp", 47,
                  "iDomainNum < mDomains.size()", "domain index out of range"))
        __debugbreak();
    mDomains[iDomainNum] = iDomain;
}

// ============================================================================
// apsAction::Fixup — relocate the vtable + arrays/domains after a load.
// Looks up this vtable in fixupParams->actions; then applies basePtr to
// mParams.mElements, mDomains.mElements and each non-null domain pointer
// (recursing into apsDomain::Fixup).
// ea: 0x8082F0
// ============================================================================
void apsAction::Fixup(const apsFixupParams& iFixupParams) {
    // Find this action's vtable in the fixup table.
    void* vtbl = 0;
    for (int i = 0; i < iFixupParams.numActions; ++i) {
        if (iFixupParams.actions[i].id == GetVtable()) {
            vtbl = (void*)iFixupParams.actions[i].vtbl;
            break;
        }
    }
    if (vtbl == 0)
        tlFatal("Didn't find vtable for action");

    SetVtable((unsigned int)vtbl);

    if (mParams.mElements != 0)
        mParams.mElements = (float*)((char*)mParams.mElements + (ptrdiff_t)iFixupParams.basePtr);
    if (mDomains.mElements != 0) {
        mDomains.mElements = (apsDomain**)((char*)mDomains.mElements + (ptrdiff_t)iFixupParams.basePtr);
        for (int i = 0; i < mDomains.mSize; ++i) {
            if (mDomains.mElements[i] != 0) {
                mDomains.mElements[i] = (apsDomain*)((char*)mDomains.mElements[i] + (ptrdiff_t)iFixupParams.basePtr);
                mDomains.mElements[i]->Fixup(iFixupParams);
            }
        }
    }
}
