// ============================================================================
// apsGroupMgr.cpp — singleton particle-group manager (7 non-inline funcs).
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsGroupMgr.cpp
//
// Port strategy (matches apsMemory.o / apsActionList.o precedent):
//   - All 7 non-inline functions verified against IDA disasm.
//   - apsMemory::PoolAllocator<apsGroup> + apsSingleton<apsGroupManager> are
//     inline COMDATs in the headers, emitted per codmp_xboxr.map.
// ============================================================================
#include "apsGroupMgr.h"

#include <stdio.h>

// tl_system.o (tl_xboxr, ported)
extern void tlPrintf(const char* fmt, ...);

// ============================================================================
// apsGroupManager::apsGroupManager — singleton init + group pool allocator.
// ea: 0x806770
// ============================================================================
apsGroupManager::apsGroupManager(int maxGroups) {
    if (apsSingleton<apsGroupManager>::sInstancePtr != 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 86,
                  "0 == sInstancePtr", "singleton already initialised"))
        __debugbreak();
    apsSingleton<apsGroupManager>::sInstancePtr = this;

    mGroups.mElements = 0;
    mGroups.mCapacity = 0;
    mGroups.mSize = 0;
    mMaxGroups = maxGroups;
    mGroups.reserve(maxGroups);

    apsMemory::PoolAllocator<apsGroup>* pa =
        (apsMemory::PoolAllocator<apsGroup>*)apsCommon::GetAllocator()->MemAlign(44, 4);
    if (pa != 0)
        new (pa) apsMemory::PoolAllocator<apsGroup>(maxGroups);
}

// ============================================================================
// apsGroupManager::CreateGroup — allocate a group, init it, add to the list.
// ea: 0x8067F0
// ============================================================================
apsGroup* apsGroupManager::CreateGroup(int iMaxNumParticles, apsRenderer* iRenderer,
                                       const apsPFD& iPFD, unsigned int iLocalSpace) {
    if (apsMemory::PoolAllocator<apsGroup>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();

    apsGroup* newGroup = apsMemory::PoolAllocator<apsGroup>::InstancePtr()->Alloc();
    if (newGroup == 0)
        return 0;

    if (newGroup->Init(iMaxNumParticles, iRenderer, iPFD, iLocalSpace) == 0) {
        newGroup->Term(0);
        if (apsMemory::PoolAllocator<apsGroup>::InstancePtr() == 0 &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        apsMemory::PoolAllocator<apsGroup>::InstancePtr()->Free(newGroup);
        return 0;
    }

    newGroup->mGroupIndex = (short)mGroups.mSize;
    mGroups.push_back(newGroup);
    return newGroup;
}

// ============================================================================
// apsGroupManager::DestroyGroup — tear down a group, return its slot to the pool.
// ea: 0x8068C0
// ============================================================================
void apsGroupManager::DestroyGroup(apsGroup* iGroup, unsigned int bFastDeath) {
    int index = iGroup->mGroupIndex;
    if ((index < 0 || index >= mGroups.mSize) &&
        _tlAssert("source/apsGroupMgr.cpp", 106,
                  "(index >= 0) && (index < mGroups.size() )", "Group is not in group list"))
        __debugbreak();
    if ((index < 0 || index >= mGroups.mSize) &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mGroups.mElements[index] != iGroup &&
        _tlAssert("source/apsGroupMgr.cpp", 109, "pGroup == iGroup", "group array mismatch"))
        __debugbreak();

    iGroup->Term(bFastDeath);
    if (apsMemory::PoolAllocator<apsGroup>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    apsMemory::PoolAllocator<apsGroup>::InstancePtr()->Free(iGroup);
    iGroup->mGroupIndex = -1;

    if (mGroups.mSize > 1) {
        if ((index < 0 || index >= mGroups.mSize) &&
            _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                      "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            __debugbreak();
        mGroups.mElements[index] = mGroups.mElements[mGroups.mSize - 1];
        if ((index < 0 || index >= mGroups.mSize) &&
            _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                      "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            __debugbreak();
        mGroups.mElements[index]->mGroupIndex = (short)index;
    }
    if (mGroups.mSize != 0)
        mGroups.mSize = mGroups.mSize - 1;
}

// ============================================================================
// apsGroupManager::DestroyAllGroups — destroy every group.
// ea: 0x806A30
// ============================================================================
void apsGroupManager::DestroyAllGroups() {
    apsGroup** first = mGroups.mElements;
    apsGroup** last = &mGroups.mElements[mGroups.mSize];
    while (first != last)
        DestroyGroup(*first++, 0);
}

// ============================================================================
// apsGroupManager::~apsGroupManager — destroy groups + pool allocator.
// ea: 0x806A60
// ============================================================================
apsGroupManager::~apsGroupManager() {
    apsGroup** first = mGroups.mElements;
    apsGroup** last = &mGroups.mElements[mGroups.mSize];
    while (first != last)
        DestroyGroup(*first++, 0);

    if (apsMemory::PoolAllocator<apsGroup>::InstancePtr() != 0) {
        apsDestroy(apsMemory::PoolAllocator<apsGroup>::InstancePtr());
        apsCommon::GetAllocator()->MemFree(apsMemory::PoolAllocator<apsGroup>::InstancePtr());
    }
    if (mGroups.mElements != 0) {
        int old = apsCommon::SetPakAllocs(0);
        apsCommon::GetAllocator()->MemFree(mGroups.mElements);
        apsCommon::SetPakAllocs(old);
        mGroups.mElements = 0;
        mGroups.mCapacity = 0;
        mGroups.mSize = 0;
    }
    if (apsSingleton<apsGroupManager>::sInstancePtr != 0) {
        apsSingleton<apsGroupManager>::sInstancePtr = 0;
    } else {
        if (_tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 104,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        apsSingleton<apsGroupManager>::sInstancePtr = 0;
    }
}

// ============================================================================
// apsGroupManager::TestAlloc — are at least numGroups slots free?
// ea: 0x8065D0
// ============================================================================
unsigned int apsGroupManager::TestAlloc(int numGroups) {
    if (apsMemory::PoolAllocator<apsGroup>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsMemory::PoolAllocator<apsGroup>::InstancePtr()->mPool.mNumBlocks
         - apsMemory::PoolAllocator<apsGroup>::InstancePtr()->mPool.mNumBlocksUsed >= numGroups;
}

// ============================================================================
// apsGroupManager::Report — print active-group summary.
// ea: 0x806620
// ============================================================================
void apsGroupManager::Report() {
    tlPrintf("active groups, %d\n", mGroups.mSize);
    apsGroup** first = mGroups.mElements;
    apsGroup** last = &mGroups.mElements[mGroups.mSize];
    int n = 0;
    while (first != last) {
        apsGroup* g = *first;
        tlPrintf("Group %d,(%d)\n", n, g->mGroupIndex);
        tlPrintf("max particles, %d\n, used, %d\n, peak, %d\n",
                 g->mMaxNumParticles, g->mNumParticles, g->mMaxUsedParticles);
        tlPrintf("last update, %f\n, delayed by, %fs\n", g->mUpdateTime, g->mUpdateDelay);
        tlPrintf("last pos, %f, %f, %f\n",
                 g->mLastPos.v.m128_f32[0], g->mLastPos.v.m128_f32[1], g->mLastPos.v.m128_f32[2]);
        const char* vis = (g->mFlags & 8) != 0 ? "visible" : "not visible";
        tlPrintf("distance from camera, %f, %s\n", g->mBoundSphereDistanceFromCamera, vis);
        tlPrintf("\n\n");
        ++first;
        ++n;
    }
}

// ============================================================================
// apsSingleton<apsGroupManager> — explicit instantiation (emits sInstancePtr).
// ============================================================================
template class apsSingleton<apsGroupManager>;
