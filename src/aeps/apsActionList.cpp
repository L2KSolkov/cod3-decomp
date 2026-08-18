// ============================================================================
// apsActionList.cpp — ordered action list for particle groups (7 non-inline).
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsActionList.cpp
//
// Port strategy (matches apsAction.o / apsGroup.o precedent):
//   - All 7 non-inline functions verified against IDA disasm.
//   - The anonymous-namespace g_updateLOD table is reconstructed as a static
//     array here (the original's $E9_3 dynamic initializer writes it).
//   - g_actionList + apsCommon::LOD()/GetChanceToRemove() inline accessors are
//     used per the map attribution.
// ============================================================================
#include "apsActionList.h"

#include <float.h>

// ============================================================================
// Anonymous-namespace visible-LOD update table. The original initialized this
// via $E9_3 (dynamic initializer @0xA6B480); reconstructed as static data.
// UpdateLOD: { m_dist, m_delay } — LOD index = first entry with dist > camera.
// ============================================================================
namespace {
struct UpdateLOD {
    float m_dist;
    float m_delay;
};

UpdateLOD g_updateLOD[] = {
    { 961.53851f,    0.016666668f },   //  1/60 s
    { 1602.5642f,    0.033333335f },   //  1/30 s
    { 2243.5898f,    0.041666668f },   //  1/24 s
    { 2884.6155f,    0.06666667f },    //  1/15 s
    { FLT_MAX,       0.1f },           //  final (never passed)
};
} // namespace

// ============================================================================
// Data (apsActionList.o)
// ============================================================================
apsActionList* g_actionList = 0;   // @0x10E0A20

// ============================================================================
// Anonymous-namespace: GetVisibleParticleUpdateDelay — pick the LOD slot whose
// m_dist exceeds distToCamera; returns that slot's delay.
// ea: 0x8087F0
// ============================================================================
float GetVisibleParticleUpdateDelay(float distToCamera, unsigned int* lodLvl) {
    unsigned int v2 = 0;
    while (g_updateLOD[v2].m_dist <= distToCamera) {
        if (++v2 >= 5)
            return 0.0f;
    }
    if (lodLvl != 0)
        *lodLvl = v2;
    return g_updateLOD[v2].m_delay;
}

// ============================================================================
// apsActionList::Apply — run the action list across the group's particle range.
// Updates the group's update-delay based on visibility/LOD, then batches the
// particles and invokes each action (skipping eSource actions when iNoSources).
// ea: 0x808830
// ============================================================================
void apsActionList::Apply(apsGroup* iGroup, apsEffect& iEffect, float iElapsedTime,
                          float iTimeDelta, unsigned int iNoSources,
                          unsigned int doChanceToRemove) {
    iGroup->TestVisibility();

    float delta = iElapsedTime - iGroup->mUpdateTime;
    unsigned int hadHitch = iGroup->mFlags & 8;
    if (delta > 0.30000001f) {
        delta = 0.30000001f;
        hadHitch = 1;
    }

    if (iGroup->mUpdateDelay <= delta) {
        if (hadHitch != 0) {
            if (iEffect.mTemplate == 0 &&
                _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182,
                          "mTemplate", "null template"))
                __debugbreak();
            float dist = (iEffect.mTemplate->GetUsesUpdateLod() != 0)
                             ? iGroup->mBoundSphereDistanceFromCamera : 0.0f;
            unsigned int lod = 0;
            iGroup->mUpdateDelay = GetVisibleParticleUpdateDelay(dist, &lod);
        } else {
            float delay = apsCommon::LOD().GetDelayForDistance(iGroup->mBoundSphereDistanceFromCamera);
            float d = delay;
            if (delay >= 0.30000001f)
                d = 0.30000001f;
            iGroup->mUpdateDelay = d;
        }
        iGroup->mUpdateTime = iElapsedTime;
        g_actionList = this;

        apsAction** first = mActions.mElements;
        apsAction** last = &mActions.mElements[mActions.mSize];

        if (iNoSources != 0) {
            if (first == last)
                return;
            while ((*first)->GetIterationStyle() == apsAction::eSource) {
                if (++first == last)
                    return;
            }
        } else {
            if (first == last)
                return;
            for (;;) {
                apsAction* action = *first;
                if (action->GetIterationStyle() != apsAction::eSource)
                    break;
                if (action->mParams.mSize <= 0 &&
                    _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                              "iIndex >= 0 && iIndex < mSize", "out of bounds"))
                    __debugbreak();
                if (iElapsedTime >= action->mParams.mElements[0]) {
                    if (action->mParams.mSize <= 1 &&
                        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
                        __debugbreak();
                    if (action->mParams.mElements[1] >= iElapsedTime)
                        action->Act(0, 0, iGroup, &iEffect, iElapsedTime, delta);
                }
                if (++first == last)
                    return;
            }
        }

        if (first != last) {
            unsigned char* cur = iGroup->GetParticles();
            unsigned char* curEnd = cur;
            if (iGroup->StartBatches(cur, curEnd, delta) != 0) {
                int savePositions = iGroup->mPFD.mFields & 0x10000000;
                apsAction** actions = first;

                while (cur != curEnd) {
                    if (savePositions != 0)
                        iGroup->SavePositionsIfNecessary(cur, curEnd);
                    if (doChanceToRemove != 0) {
                        int applied = 0, removed = 0;
                        iGroup->ApplyChanceToRemove(apsCommon::GetChanceToRemove(),
                                                    cur, curEnd, applied, removed);
                    }
                    apsAction** it = first;
                    if (it != last) {
                        do {
                        apsAction* action = *it;
                        if (action->GetIterationStyle() == apsAction::eSource &&
                            _tlAssert("source/apsActionList.cpp", 286,
                                      "((*action_it)->GetIterationStyle() != apsAction::eSource)",
                                      "source action within batched update"))
                            __debugbreak();
                            if (action->mParams.mSize <= 0 &&
                                _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                                          "iIndex >= 0 && iIndex < mSize", "out of bounds"))
                                __debugbreak();
                            if (iElapsedTime >= action->mParams.mElements[0]) {
                                if (action->mParams.mSize <= 1 &&
                                    _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                                              "iIndex >= 0 && iIndex < mSize", "out of bounds"))
                                    __debugbreak();
                                if (action->mParams.mElements[1] >= iElapsedTime)
                                    action->Act(cur, curEnd, iGroup, &iEffect, iElapsedTime, delta);
                            }
                            ++it;
                        } while (it != last);
                    }
                    iGroup->NextBatch(cur, curEnd);
                }
                iGroup->EndBatches();
                iEffect.AccumulateCollisionBounds(iGroup->mBounds);
            }
        }
    }
}

// ============================================================================
// apsActionList::EnhancePFD — OR in every action's required particle fields.
// ea: 0x808BA0
// ============================================================================
void apsActionList::EnhancePFD(apsPFD& ioPFD) {
    apsAction** first = mActions.mElements;
    apsAction** last = &mActions.mElements[mActions.mSize];
    while (first != last)
        ioPFD.AddFields((*first++)->GetRequiredParticleFields());
}

// ============================================================================
// apsActionList::Fixup — relocate mActions buffer + each action, recursing into
// apsAction::Fixup.
// ea: 0x808BE0
// ============================================================================
void apsActionList::Fixup(const apsFixupParams& iFixupParams) {
    if (mActions.mElements != 0) {
        // Serialized list pointers are offsets from this action-list object.
        mActions.mElements = (apsAction**)((char*)mActions.mElements + (ptrdiff_t)this);
        for (int i = 0; i < mActions.mSize; ++i) {
            if ((i < 0 || i >= mActions.mSize) &&
                _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                          "iIndex >= 0 && iIndex < mSize", "out of bounds"))
                __debugbreak();
            mActions.mElements[i] = (apsAction*)((char*)mActions.mElements[i] + (ptrdiff_t)this);
            if ((i < 0 || i >= mActions.mSize) &&
                _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                          "iIndex >= 0 && iIndex < mSize", "out of bounds"))
                __debugbreak();
            mActions.mElements[i]->Fixup(iFixupParams);
        }
    }
}

// ============================================================================
// apsActionList::apsActionList — zero the action array.
// ea: 0x808C90
// ============================================================================
apsActionList::apsActionList() {
    mActions.mElements = 0;
    mActions.mCapacity = 0;
    mActions.mSize = 0;
}

// ============================================================================
// apsActionList::~apsActionList — destroy each owned action, then free the list.
// ea: 0x808CA0
// ============================================================================
apsActionList::~apsActionList() {
    if (mActions.mElements != 0) {
        for (int i = 0; i < mActions.mSize; ++i) {
            apsAction* action = mActions.mElements[i];
            if (action != 0) {
                apsDestroy(action);
                apsCommon::GetAllocator()->MemFree(action);
            }
        }
    }
    if (mActions.mElements != 0) {
        int old = apsCommon::SetPakAllocs(0);
        apsCommon::GetAllocator()->MemFree(mActions.mElements);
        apsCommon::SetPakAllocs(old);
        mActions.mElements = 0;
        mActions.mCapacity = 0;
        mActions.mSize = 0;
    }
}

// ============================================================================
// apsActionList::Add — source actions prepend, everything else appends.
// ea: 0x808D30
// ============================================================================
void apsActionList::Add(apsAction* iAction) {
    if (iAction->mIterationStyle != apsAction::eSource)
        mActions.push_back(iAction);
    else
        mActions.push_front(iAction);
}
