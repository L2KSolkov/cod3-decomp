// ============================================================================
// apsSuppliedActions.cpp — built-in particle actions (88 funcs)
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsSuppliedActions.cpp
//
// Port strategy: stubs matching nsl_xboxr / nal_xboxr precedent.
//   - Class ABI is real: vtable slots, layout, ctor→base delegation.
//   - Method bodies are stubs pending a live particle-system harness.
//   - Anonymous-namespace helpers reconstructed inline where trivial.
//
// The ctor→apsAction(numParams,numDomains,style,requiredFields) pattern is
// verbatim from IDA — the (numParams,numDomains,requiredFields) triples are
// content-load-bearing (they drive per-particle memory layout via apsPFD),
// so every ctor reproduces its exact numbers.
// ============================================================================
#include "apsSuppliedActions.h"
#include "apsGroup.h"
#include "apsEffect.h"

namespace {

// ea: 0x00809AB0 - release helper used by source and burst emission counts.
// The RNG step and threshold comparison follow the IDA C dump/disassembly.
double ModifyEmitCountByChance(float chanceToRemoveModifier, float originalCount) {
    const float chance = apsCommon::GetChanceToRemove() * chanceToRemoveModifier;
    if (chance <= 0.0f)
        return originalCount;

    const float randomValue = apsMath::gDefaultRandomNumberGenerator.GetFloat();
    if (chance <= randomValue)
        return originalCount;

    float clampedChance = 1.0f;
    if (chance <= 1.0f)
        clampedChance = chance;
    return (1.0f - clampedChance) * originalCount;
}

// ea: 0x00809CA0 - release helper used by alpha-fade actions.
void FadeOutParticle(const apsGroup* group, unsigned char* particle,
                     float* alphaPtr, float alphaFadePerSec, float deltaSec) {
    const float alpha = *alphaPtr - (alphaFadePerSec * deltaSec);
    if (alpha >= 0.001f) {
        *alphaPtr = alpha;
    } else {
        *alphaPtr = 0.0f;
        group->MarkParticleForRemoval(particle);
    }
}

} // namespace

// ============================================================================
// apsAction base — real implementation now lives in apsAction.cpp (apsAction.o).
// ============================================================================

// ============================================================================
// apsQuaternion — apsSuppliedActions.o owns ctor(Dir3) / Set / operator+=.
// ea: 0x00809400 ctor / 0x00809480 Set / 0x008094B0 operator+=
// ============================================================================
apsQuaternion::apsQuaternion(const math::Dir3& iVector) {
    x = iVector.v.m128_f32[0];
    y = iVector.v.m128_f32[1];
    z = iVector.v.m128_f32[2];
    w = 0.0f;
}
void apsQuaternion::Set(float ix, float iy, float iz, float iw) {
    x = ix; y = iy; z = iz; w = iw;
}
apsQuaternion& apsQuaternion::operator+=(const apsQuaternion& iRHS) {
    x += iRHS.x;
    y += iRHS.y;
    z += iRHS.z;
    w += iRHS.w;
    return *this;
}

// ============================================================================
// Source actions
// (numParams, numDomains, requiredFields) triples from IDA disasm.
// ============================================================================
apsSourceAction::apsSourceAction()
    : apsAction(5, 16, eSource, 0x200u) {}
apsSourceAction::apsSourceAction(int iNumParams, int iNumDomains,
                                 unsigned int iRequiredParticleFields)
    : apsAction(iNumParams, iNumDomains, eSource,
                iRequiredParticleFields | 0x200u) {}
void apsSourceAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}
// ea: 0x0080C5C0
int apsSourceAction::GetEmissionCount(apsGroup& ioGroup, float iTimeDelta) {
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();

    const float originalCount = (mParams.mElements[2] * iTimeDelta) +
                                 ioGroup.mEmissionSpillover;
    const float modifiedCount = static_cast<float>(
        ModifyEmitCountByChance(ioGroup.mRenderer->GetChanceToRemove(), originalCount));
    const int count = static_cast<int>(modifiedCount);
    ioGroup.mEmissionSpillover = modifiedCount - static_cast<float>(count);
    return count;
}

apsBurstAction::apsBurstAction()
    : apsSourceAction(5, 16, 0x200u) {}
// ea: 0x0080C640
int apsBurstAction::GetEmissionCount(apsGroup& ioGroup, float) {
    if (ioGroup.mEmissionSpillover < 0.0f)
        return 0;

    ioGroup.mEmissionSpillover = -1.0f;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();

    const float modifiedCount = static_cast<float>(
        ModifyEmitCountByChance(ioGroup.mRenderer->GetChanceToRemove(), mParams.mElements[2]));
    return static_cast<int>(modifiedCount);
}

apsRandomSpawnAction::apsRandomSpawnAction()
    : apsSourceAction(5, 17, 0x200u) {}
// ea: 0x0080C6C0
int apsRandomSpawnAction::GetEmissionCount(apsGroup& ioGroup, float iTimeDelta) {
    const float emissionSpillover = ioGroup.mEmissionSpillover;
    int count = 0;

    if (mDomains.mSize <= 16 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();

    float delay = emissionSpillover - iTimeDelta;
    apsDomain* domain = mDomains.mElements[4];
    while (delay <= 0.0f) {
        ++count;
        domain->GetValue(1, &iTimeDelta);
        delay = iTimeDelta + delay;
    }
    ioGroup.mEmissionSpillover = delay;
    return count;
}

// ============================================================================
// Lifetime — kills particles whose age > maxage.
// Full body is a real 25-line loop; not needed until particles actually spawn.
// ============================================================================
apsLifetimeAction::apsLifetimeAction()
    : apsAction(2, 0, eAsync, 0x8000600u) {}
// ea: 0x00809BF0
void apsLifetimeAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                            apsGroup* ioGroup, apsEffect*, float, float) {
    if ((ioGroup->mPFD.mFields & 0x200u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int ageOffset = ioGroup->mPFD.mOffsets[9];

    if ((ioGroup->mPFD.mFields & 0x400u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int maxAgeOffset = ioGroup->mPFD.mOffsets[10];
    const int stride = ioGroup->mPFD.mStride;

    unsigned char* particle = iBegin;
    if (particle == iEnd)
        return;

    do {
        const float age = *reinterpret_cast<float*>(particle + ageOffset);
        const float maxAge = *reinterpret_cast<float*>(particle + maxAgeOffset);
        if (age > maxAge)
            ioGroup->MarkParticleForRemoval(particle);
        particle += stride;
    } while (particle != iEnd);
}

// ============================================================================
// Alpha fade family
// ============================================================================
apsAlphaFadeAction::apsAlphaFadeAction()
    : apsAction(2, 0, eAsync, 0x10u) {}
// ea: 0x0080C760
void apsAlphaFadeAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                             apsGroup* ioGroup, apsEffect*, float, float iTimeDelta) {
    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float beginTime = mParams.mElements[3];

    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float beginAlpha = mParams.mElements[2];

    if ((ioGroup->mPFD.mFields & 0x200u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int ageOffset = ioGroup->mPFD.mOffsets[9];

    if ((ioGroup->mPFD.mFields & 0x400u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int maxAgeOffset = ioGroup->mPFD.mOffsets[10];

    if ((ioGroup->mPFD.mFields & 0x10u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int alphaOffset = ioGroup->mPFD.mOffsets[4];

    if ((ioGroup->mPFD.mFields & 0x8000000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int flagsOffset = ioGroup->mPFD.mOffsets[27];

    if ((ioGroup->mPFD.mFields & 0x8000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int maxAlphaOffset = ioGroup->mPFD.mOffsets[15];

    const float scale = beginAlpha / (1.0f - beginTime);
    const int stride = ioGroup->mPFD.mStride;
    for (unsigned char* particle = iBegin; particle != iEnd; particle += stride) {
        float* alpha = reinterpret_cast<float*>(particle + alphaOffset);
        const float maxAlpha = *reinterpret_cast<float*>(particle + maxAlphaOffset);
        const unsigned char flags = *(particle + flagsOffset);

        if ((flags & 4u) != 0) {
            FadeOutParticle(ioGroup, particle, alpha, maxAlpha, iTimeDelta);
            continue;
        }

        const float maxAge = *reinterpret_cast<float*>(particle + maxAgeOffset);
        const float age = *reinterpret_cast<float*>(particle + ageOffset);
        float newAlpha = 0.0f;
        if ((maxAge * beginTime) <= age) {
            if (maxAge > age)
                newAlpha = (1.0f - (age / maxAge)) * scale;
        } else {
            newAlpha = beginAlpha;
        }
        *alpha = newAlpha;
    }
}

apsAlphaFadeInOutAction::apsAlphaFadeInOutAction()
    : apsAction(5, 0, eAsync, 0x8008610u) {}
// ea: 0x0080C9B0
void apsAlphaFadeInOutAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                  apsGroup* ioGroup, apsEffect*, float,
                                  float iTimeDelta) {
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float endFadeIn = mParams.mElements[2];

    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float beginFadeOut = mParams.mElements[4];

    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float maxAlpha = mParams.mElements[3];
    const float beginScale = maxAlpha / endFadeIn;
    const float endScale = maxAlpha / (1.0f - beginFadeOut);

    if ((ioGroup->mPFD.mFields & 0x200u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int ageOffset = ioGroup->mPFD.mOffsets[9];

    if ((ioGroup->mPFD.mFields & 0x10u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int alphaOffset = ioGroup->mPFD.mOffsets[4];

    if ((ioGroup->mPFD.mFields & 0x400u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int maxAgeOffset = ioGroup->mPFD.mOffsets[10];

    if ((ioGroup->mPFD.mFields & 0x8000000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int flagsOffset = ioGroup->mPFD.mOffsets[27];

    if ((ioGroup->mPFD.mFields & 0x8000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int maxAlphaOffset = ioGroup->mPFD.mOffsets[15];

    const int stride = ioGroup->mPFD.mStride;
    for (unsigned char* particle = iBegin; particle != iEnd; particle += stride) {
        float* alpha = reinterpret_cast<float*>(particle + alphaOffset);
        const unsigned char flags = *(particle + flagsOffset);
        if ((flags & 4u) != 0) {
            const float fadeAmount =
                *reinterpret_cast<float*>(particle + maxAlphaOffset) * iTimeDelta;
            const float fadedAlpha = *alpha - fadeAmount;
            if (fadedAlpha >= 0.001f) {
                *alpha = fadedAlpha;
            } else {
                *alpha = 0.0f;
                ioGroup->MarkParticleForRemoval(particle);
            }
            continue;
        }

        const float maxAge = *reinterpret_cast<float*>(particle + maxAgeOffset);
        const float age = *reinterpret_cast<float*>(particle + ageOffset);
        const float agePercent = age / maxAge;
        float newAlpha;
        if (endFadeIn <= agePercent) {
            if (beginFadeOut <= agePercent) {
                if (maxAge <= age)
                    newAlpha = 0.0f;
                else
                    newAlpha = (1.0f - agePercent) * endScale;
            } else {
                newAlpha = maxAlpha;
            }
        } else {
            newAlpha = agePercent * beginScale;
        }
        *alpha = newAlpha;
    }
}

apsRandomAlphaFadeInOutAction::apsRandomAlphaFadeInOutAction()
    : apsAction(4, 0, eAsync, 0x8008610u) {}
// ea: 0x0080CC60
void apsRandomAlphaFadeInOutAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                        apsGroup* ioGroup, apsEffect*, float,
                                        float iTimeDelta) {
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float endFadeIn = mParams.mElements[2];

    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float beginFadeOut = mParams.mElements[3];
    const float beginScale = 1.0f / endFadeIn;
    const float endScale = 1.0f / (1.0f - beginFadeOut);

    if ((ioGroup->mPFD.mFields & 0x200u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int ageOffset = ioGroup->mPFD.mOffsets[9];

    if ((ioGroup->mPFD.mFields & 0x10u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int alphaOffset = ioGroup->mPFD.mOffsets[4];

    if ((ioGroup->mPFD.mFields & 0x8000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int maxAlphaOffset = ioGroup->mPFD.mOffsets[15];

    if ((ioGroup->mPFD.mFields & 0x400u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int maxAgeOffset = ioGroup->mPFD.mOffsets[10];

    if ((ioGroup->mPFD.mFields & 0x8000000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int flagsOffset = ioGroup->mPFD.mOffsets[27];

    const int stride = ioGroup->mPFD.mStride;
    for (unsigned char* particle = iBegin; particle != iEnd; particle += stride) {
        float* alpha = reinterpret_cast<float*>(particle + alphaOffset);
        const unsigned char flags = *(particle + flagsOffset);
        const float maxAlpha = *reinterpret_cast<float*>(particle + maxAlphaOffset);

        if ((flags & 4u) != 0) {
            const float fadedAlpha = *alpha - (maxAlpha * iTimeDelta);
            if (fadedAlpha >= 0.001f) {
                *alpha = fadedAlpha;
            } else {
                *alpha = 0.0f;
                ioGroup->MarkParticleForRemoval(particle);
            }
            continue;
        }

        const float maxAge = *reinterpret_cast<float*>(particle + maxAgeOffset);
        const float age = *reinterpret_cast<float*>(particle + ageOffset);
        const float agePercent = age / maxAge;
        float newAlpha;
        if (endFadeIn <= agePercent) {
            if (beginFadeOut <= agePercent) {
                if (maxAge <= age)
                    newAlpha = 0.0f;
                else
                    newAlpha = (1.0f - agePercent) * maxAlpha * endScale;
            } else {
                newAlpha = maxAlpha;
            }
        } else {
            newAlpha = agePercent * maxAlpha * beginScale;
        }
        *alpha = newAlpha;
    }
}

// ============================================================================
// Scale actions
// ============================================================================
apsLinearScaleAction::apsLinearScaleAction()
    : apsAction(3, 0, eAsync, 2u) {}
apsLinearScaleAction::apsLinearScaleAction(int iNumParams, int iNumDomains,
                                           unsigned int iRequiredParticleFields)
    : apsAction(iNumParams, iNumDomains, eAsync,
                iRequiredParticleFields | 2u) {}
// ea: 0x00809D90
void apsLinearScaleAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                               apsGroup* ioGroup, apsEffect* iEffect,
                               float, float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    const float scaleAmount = GetScaleAmountForFrame(*iEffect, iTimeDelta);

    if ((ioGroup->mPFD.mFields & 2u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int radiusOffset = ioGroup->mPFD.mOffsets[1];

    float* radius = reinterpret_cast<float*>(iBegin + radiusOffset);
    float* endRadius = reinterpret_cast<float*>(iEnd + radiusOffset);
    while (radius != endRadius) {
        *radius += scaleAmount;
        radius = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(radius) + stride);
    }
}
// ea: 0x0080CEE0
float apsLinearScaleAction::GetScaleAmountForFrame(apsEffect&, float iTimeDelta) const {
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    return iTimeDelta * mParams.mElements[2];
}

apsLinearScaleSyncAction::apsLinearScaleSyncAction()
    : apsLinearScaleAction(5, 0, 2u) {}
// ea: 0x0080CF20
float apsLinearScaleSyncAction::GetScaleAmountForFrame(apsEffect& iEffect,
                                                       float iTimeDelta) const {
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float start = mParams.mElements[2];

    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float end = mParams.mElements[3];

    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    return static_cast<float>((std::pow(static_cast<double>(iEffect.mParentAgePercent),
                                         static_cast<double>(mParams.mElements[4])) *
                               (end - start) + start) * iTimeDelta);
}

apsExponentialScaleAction::apsExponentialScaleAction()
    : apsAction(3, 0, eAsync, 2u) {}
void apsExponentialScaleAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsLinearScaleWidthAction::apsLinearScaleWidthAction()
    : apsAction(3, 0, eAsync, 4u) {}
void apsLinearScaleWidthAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsExponentialScaleWidthAction::apsExponentialScaleWidthAction()
    : apsAction(3, 0, eAsync, 4u) {}
void apsExponentialScaleWidthAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsLinearScaleHeightAction::apsLinearScaleHeightAction()
    : apsAction(3, 0, eAsync, 0x80u) {}
void apsLinearScaleHeightAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsExponentialScaleHeightAction::apsExponentialScaleHeightAction()
    : apsAction(3, 0, eAsync, 0x80u) {}
void apsExponentialScaleHeightAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Move / force / velocity actions
// ============================================================================
apsMoveAction::apsMoveAction()
    : apsAction(2, 0, eAsync, 0x14040u) {}
void apsMoveAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsObjectMoveAction::apsObjectMoveAction()
    : apsAction(2, 0, eAsync, 0x24021u) {}
void apsObjectMoveAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsPositionMoveAction::apsPositionMoveAction()
    : apsAction(2, 0, eAsync, 0x4001u) {}
void apsPositionMoveAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsMoveAtFixedVelocityAction::apsMoveAtFixedVelocityAction()
    : apsAction(6, 0, eAsync, 0x1u) {}
void apsMoveAtFixedVelocityAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsForceAction::apsForceAction()
    : apsAction(6, 0, eAsync, 0x4000u) {}
void apsForceAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsColorShiftAction::apsColorShiftAction()
    : apsAction(10, 0, eAsync, 0x608u) {}
void apsColorShiftAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsVelocityDragAction::apsVelocityDragAction()
    : apsAction(3, 0, eAsync, 0x4000u) {}
void apsVelocityDragAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsAngularVelocityDragAction::apsAngularVelocityDragAction()
    : apsAction(3, 0, eAsync, 0x10000u) {}
void apsAngularVelocityDragAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsVectorAngularVelocityDragAction::apsVectorAngularVelocityDragAction()
    : apsAction(3, 0, eAsync, 0x20000u) {}
void apsVectorAngularVelocityDragAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsWorldPlaneReflectionAction::apsWorldPlaneReflectionAction()
    : apsAction(2, 1, eAsync, 0x40u) {}
void apsWorldPlaneReflectionAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsUVAFrameAnimAction::apsUVAFrameAnimAction()
    : apsAction(3, 0, eAsync, 0x20000u) {}
void apsUVAFrameAnimAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Angle-tracking actions
// ============================================================================
apsAngleTrackVelocityAction::apsAngleTrackVelocityAction()
    : apsAction(0, 0, eAsync, 0x40u) {}
void apsAngleTrackVelocityAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsAngleTrackElementXAction::apsAngleTrackElementXAction()
    : apsAction(0, 0, eAsync, 0x1000u) {}
void apsAngleTrackElementXAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsAngleTrackElementYAction::apsAngleTrackElementYAction()
    : apsAction(0, 0, eAsync, 0x1000u) {}
void apsAngleTrackElementYAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsAngleTrackElementZAction::apsAngleTrackElementZAction()
    : apsAction(0, 0, eAsync, 0x1000u) {}
void apsAngleTrackElementZAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Attractor actions
// ============================================================================
apsPointAttractorAction::apsPointAttractorAction()
    : apsAction(3, 1, eAsync, 0x40u) {}
void apsPointAttractorAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsLineAttractorAction::apsLineAttractorAction()
    : apsAction(3, 2, eAsync, 0x40u) {}
void apsLineAttractorAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsDecayLineAttractorAction::apsDecayLineAttractorAction()
    : apsAction(4, 2, eAsync, 0x40u) {}
void apsDecayLineAttractorAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsKappaTauAction::apsKappaTauAction()
    : apsAction(2, 0, eAsync, 0x40u) {}
void apsKappaTauAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Spawn (nested effects)
// ============================================================================
apsSpawnAction::apsSpawnAction()
    : apsAction(3, 0, eAsync, 0x14040u) {}
void apsSpawnAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

apsSpawnOnDeathAction::apsSpawnOnDeathAction()
    : apsAction(2, 0, eAsync, 0x14040u) {}
void apsSpawnOnDeathAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// Trajectory (spline)
// ============================================================================
apsTrajectoryAction::apsTrajectoryAction()
    : apsAction(4, 0, eAsync, 0x14040u) {}
void         apsTrajectoryAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}
unsigned int apsTrajectoryAction::GetSplineInfo(float*&, float&, float&,
                                                math::Dir3&, math::Dir3&) { return 0; }

// ============================================================================
// Env-collide (world raycast)
// ============================================================================
apsEnvCollideAction::apsEnvCollideAction()
    : apsAction(3, 0, eAsync, 0x14040u) {}
void apsEnvCollideAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ============================================================================
// File-scope global — gCheckSplineData (0x014CFF50 in .data)
// ea: 0x00BFA640 CheckSplineData(float*) references this.
// ============================================================================
int gCheckSplineData = 0;
