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
#include "apsInternal.h"

#include <cmath>
#include <cstdio>
#include <cstring>

extern void tlWarning(const char* Format, ...);
extern void tlPrintf(const char* Format, ...);

// IDA global @ 0x00D3C190 (Float4_NegZAxis_123).
const __m128 Float4_NegZAxis_123 = {0.0f, 0.0f, -1.0f, 0.0f};

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

// ea: 0x008092F0
unsigned int apsMath::FloatAsInt(float f) {
    unsigned int result;
    std::memcpy(&result, &f, sizeof(result));
    return result;
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
// ea: 0x0080CFF0
void apsExponentialScaleAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                    apsGroup* ioGroup, apsEffect*, float,
                                    float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;

    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const long double logAmount =
        std::log(static_cast<long double>(mParams.mElements[2])) *
        static_cast<long double>(iTimeDelta);
    const float scaleAmount = static_cast<float>(
        (0.5L * logAmount + 1.0L) * logAmount + 1.0L);

    if ((ioGroup->mPFD.mFields & 2u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int radiusOffset = ioGroup->mPFD.mOffsets[1];
    float* radius = reinterpret_cast<float*>(iBegin + radiusOffset);
    float* endRadius = reinterpret_cast<float*>(iEnd + radiusOffset);
    while (radius != endRadius) {
        *radius *= scaleAmount;
        radius = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(radius) + stride);
    }
}

apsLinearScaleWidthAction::apsLinearScaleWidthAction()
    : apsAction(3, 0, eAsync, 4u) {}
// ea: 0x0080D0B0
void apsLinearScaleWidthAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                    apsGroup* ioGroup, apsEffect*, float,
                                    float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float scaleAmount = mParams.mElements[2] * iTimeDelta;

    if ((ioGroup->mPFD.mFields & 4u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int widthOffset = ioGroup->mPFD.mOffsets[2];
    float* width = reinterpret_cast<float*>(iBegin + widthOffset);
    float* endWidth = reinterpret_cast<float*>(iEnd + widthOffset);
    while (width != endWidth) {
        *width += scaleAmount;
        width = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(width) + stride);
    }
}

apsExponentialScaleWidthAction::apsExponentialScaleWidthAction()
    : apsAction(3, 0, eAsync, 4u) {}
// ea: 0x0080D150
void apsExponentialScaleWidthAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                         apsGroup* ioGroup, apsEffect*, float,
                                         float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const long double logAmount =
        std::log(static_cast<long double>(mParams.mElements[2])) *
        static_cast<long double>(iTimeDelta);
    const float scaleAmount = static_cast<float>(
        (0.5L * logAmount + 1.0L) * logAmount + 1.0L);

    if ((ioGroup->mPFD.mFields & 4u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int widthOffset = ioGroup->mPFD.mOffsets[2];
    float* width = reinterpret_cast<float*>(iBegin + widthOffset);
    float* endWidth = reinterpret_cast<float*>(iEnd + widthOffset);
    while (width != endWidth) {
        *width *= scaleAmount;
        width = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(width) + stride);
    }
}

apsLinearScaleHeightAction::apsLinearScaleHeightAction()
    : apsAction(3, 0, eAsync, 0x80u) {}
// ea: 0x0080D210
void apsLinearScaleHeightAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                     apsGroup* ioGroup, apsEffect*, float,
                                     float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float scaleAmount = mParams.mElements[2] * iTimeDelta;

    if ((ioGroup->mPFD.mFields & 0x80u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int heightOffset = ioGroup->mPFD.mOffsets[7];
    float* height = reinterpret_cast<float*>(iBegin + heightOffset);
    float* endHeight = reinterpret_cast<float*>(iEnd + heightOffset);
    while (height != endHeight) {
        *height += scaleAmount;
        height = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(height) + stride);
    }
}

apsExponentialScaleHeightAction::apsExponentialScaleHeightAction()
    : apsAction(3, 0, eAsync, 0x80u) {}
// ea: 0x0080D2C0
void apsExponentialScaleHeightAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                          apsGroup* ioGroup, apsEffect*, float,
                                          float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const long double logAmount =
        std::log(static_cast<long double>(mParams.mElements[2])) *
        static_cast<long double>(iTimeDelta);
    const float scaleAmount = static_cast<float>(
        (0.5L * logAmount + 1.0L) * logAmount + 1.0L);

    if ((ioGroup->mPFD.mFields & 0x80u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int heightOffset = ioGroup->mPFD.mOffsets[7];
    float* height = reinterpret_cast<float*>(iBegin + heightOffset);
    float* endHeight = reinterpret_cast<float*>(iEnd + heightOffset);
    while (height != endHeight) {
        *height *= scaleAmount;
        height = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(height) + stride);
    }
}

// ============================================================================
// Move / force / velocity actions
// ============================================================================
apsMoveAction::apsMoveAction()
    : apsAction(2, 0, eAsync, 0x14040u) {}
// ea: 0x00809EF0
void apsMoveAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                        apsGroup* ioGroup, apsEffect*, float,
                        float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if ((ioGroup->mPFD.mFields & 0x4000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int velocityOffset = ioGroup->mPFD.mOffsets[14];

    if ((ioGroup->mPFD.mFields & 0x40u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int angleOffset = ioGroup->mPFD.mOffsets[6];

    if ((ioGroup->mPFD.mFields & 0x10000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int angularVelocityOffset = ioGroup->mPFD.mOffsets[16];

    unsigned char* velocityField = iBegin + velocityOffset;
    unsigned char* angleField = iBegin + angleOffset;
    unsigned char* angularVelocityField = iBegin + angularVelocityOffset;
    while (iBegin < iEnd) {
        float* position = reinterpret_cast<float*>(iBegin);
        const float* velocity = reinterpret_cast<const float*>(velocityField);
        position[0] += velocity[0] * iTimeDelta;
        position[1] += velocity[1] * iTimeDelta;
        position[2] += velocity[2] * iTimeDelta;
        *reinterpret_cast<float*>(angleField) +=
            *reinterpret_cast<const float*>(angularVelocityField) * iTimeDelta;
        iBegin += stride;
        velocityField += stride;
        angleField += stride;
        angularVelocityField += stride;
    }
}

apsObjectMoveAction::apsObjectMoveAction()
    : apsAction(2, 0, eAsync, 0x24021u) {}

// ea: 0x0080A0E0
void apsObjectMoveAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                              apsGroup* ioGroup, apsEffect*, float,
                              float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if ((ioGroup->mPFD.mFields & (1u << apsPFDField_Velocity)) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* velocity =
        iBegin + ioGroup->mPFD.GetOffset(apsPFDField_Velocity);

    if ((ioGroup->mPFD.mFields & (1u << apsPFDField_Orientation)) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* orientation =
        iBegin + ioGroup->mPFD.GetOffset(apsPFDField_Orientation);

    if ((ioGroup->mPFD.mFields & (1u << apsPFDField_VectorAngularVelocity)) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* angularVelocity =
        iBegin + ioGroup->mPFD.GetOffset(apsPFDField_VectorAngularVelocity);

    if (iBegin == iEnd)
        return;

    const float halfTimeDelta = iTimeDelta * 0.5f;
    while (iBegin != iEnd) {
        float* position = reinterpret_cast<float*>(iBegin);
        const float* velocityValue = reinterpret_cast<const float*>(velocity);
        position[0] += velocityValue[0] * iTimeDelta;
        position[1] += velocityValue[1] * iTimeDelta;
        position[2] += velocityValue[2] * iTimeDelta;

        apsQuaternion* particleOrientation =
            reinterpret_cast<apsQuaternion*>(orientation);
        math::Dir3 currentAngularVelocity;
        const float* angularVelocityValue =
            reinterpret_cast<const float*>(angularVelocity);
        currentAngularVelocity.v = _mm_setr_ps(
            angularVelocityValue[0], angularVelocityValue[1],
            angularVelocityValue[2], 0.0f);
        particleOrientation->RotatePoint(currentAngularVelocity);

        const float qx = particleOrientation->x;
        const float qy = particleOrientation->y;
        const float qz = particleOrientation->z;
        const float qw = particleOrientation->w;
        const float wx = currentAngularVelocity.v.m128_f32[0];
        const float wy = currentAngularVelocity.v.m128_f32[1];
        const float wz = currentAngularVelocity.v.m128_f32[2];

        const float nextX = qx +
            ((((qw * wx) + (qz * wy)) + (qx * 0.0f)) - (qy * wz)) *
                halfTimeDelta;
        const float nextY = qy +
            (((((qy * 0.0f) - (qz * wx)) + (qx * wz)) + (qw * wy))) *
                halfTimeDelta;
        const float nextZ = qz +
            (((((qy * wx) + (qz * 0.0f)) - (qx * wy)) + (qw * wz))) *
                halfTimeDelta;
        const float nextW = qw +
            (((((qw * 0.0f) - (qx * wx)) - (qy * wy)) - (qz * wz))) *
                halfTimeDelta;

        const float inverseLength = 1.0f / std::sqrt(
            nextW * nextW + nextY * nextY + nextZ * nextZ + nextX * nextX);
        particleOrientation->x = nextX * inverseLength;
        particleOrientation->y = nextY * inverseLength;
        particleOrientation->z = nextZ * inverseLength;
        particleOrientation->w = nextW * inverseLength;

        iBegin += stride;
        velocity += stride;
        orientation += stride;
        angularVelocity += stride;
    }
}

apsPositionMoveAction::apsPositionMoveAction()
    : apsAction(2, 0, eAsync, 0x4001u) {}
// ea: 0x0080A420
void apsPositionMoveAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                apsGroup* ioGroup, apsEffect*, float,
                                float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if ((ioGroup->mPFD.mFields & 0x4000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();

    unsigned char* velocityField = iBegin + ioGroup->mPFD.mOffsets[14];
    if (iBegin != iEnd) {
        while (true) {
            float* position = reinterpret_cast<float*>(iBegin);
            float* velocity = reinterpret_cast<float*>(velocityField);
            position[0] += velocity[0] * iTimeDelta;
            position[1] += velocity[1] * iTimeDelta;
            position[2] += velocity[2] * iTimeDelta;
            iBegin += stride;
            velocityField += stride;
            if (iBegin == iEnd)
                break;
        }
    }
}

apsMoveAtFixedVelocityAction::apsMoveAtFixedVelocityAction()
    : apsAction(6, 0, eAsync, 0x1u) {}
// ea: 0x0080D380
void apsMoveAtFixedVelocityAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                       apsGroup* ioGroup, apsEffect*, float,
                                       float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();

    __m128 displacement = _mm_set_ps(0.0f, mParams.mElements[4],
                                     mParams.mElements[3], mParams.mElements[2]);
    if (mParams.mSize <= 5 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mElements[5] != 0.0f) {
        const math::Mat43& transform = ioGroup->mLocalToWorld;
        displacement = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(displacement, displacement, 0), transform.x.v),
                _mm_mul_ps(_mm_shuffle_ps(displacement, displacement, 85), transform.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(displacement, displacement, 170), transform.z.v));
    }
    displacement = _mm_mul_ps(displacement, _mm_set1_ps(iTimeDelta));

    while (iBegin != iEnd) {
        float* position = reinterpret_cast<float*>(iBegin);
        position[0] += displacement.m128_f32[0];
        position[1] += displacement.m128_f32[1];
        position[2] += displacement.m128_f32[2];
        iBegin += stride;
    }
}

apsForceAction::apsForceAction()
    : apsAction(6, 0, eAsync, 0x4000u) {}
// ea: 0x0080D5B0
void apsForceAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                         apsGroup* ioGroup, apsEffect*, float,
                         float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();

    __m128 force = _mm_set_ps(0.0f, mParams.mElements[4],
                              mParams.mElements[3], mParams.mElements[2]);
    force = _mm_mul_ps(force, _mm_set1_ps(iTimeDelta));
    if (mParams.mSize <= 5 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mElements[5] != 0.0f) {
        const math::Mat43& transform = ioGroup->mLocalToWorld;
        force = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(force, force, 0), transform.x.v),
                _mm_mul_ps(_mm_shuffle_ps(force, force, 85), transform.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(force, force, 170), transform.z.v));
    }

    if ((ioGroup->mPFD.mFields & 0x4000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int velocityOffset = ioGroup->mPFD.mOffsets[14];
    unsigned char* velocityField = iBegin + velocityOffset;
    const unsigned char* endVelocityField = iEnd + velocityOffset;
    while (velocityField != endVelocityField) {
        float* velocity = reinterpret_cast<float*>(velocityField);
        velocity[0] += force.m128_f32[0];
        velocity[1] += force.m128_f32[1];
        velocity[2] += force.m128_f32[2];
        velocityField += stride;
    }
}

apsColorShiftAction::apsColorShiftAction()
    : apsAction(10, 0, eAsync, 0x608u) {}
// ea: 0x0080D820
void apsColorShiftAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                              apsGroup* ioGroup, apsEffect*, float, float) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float beginTime = mParams.mElements[2];
    if (mParams.mSize <= 6 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float endTime = mParams.mElements[6];

    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mSize <= 5 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const math::Dir3 beginColor(mParams.mElements[3], mParams.mElements[4],
                                mParams.mElements[5]);

    if (mParams.mSize <= 7 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mSize <= 8 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    if (mParams.mSize <= 9 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const math::Dir3 endColor(mParams.mElements[7], mParams.mElements[8],
                              mParams.mElements[9]);
    const __m128 colorDelta = _mm_sub_ps(endColor.v, beginColor.v);

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
    if ((ioGroup->mPFD.mFields & 8u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int colorOffset = ioGroup->mPFD.mOffsets[3];

    unsigned char* particle = iBegin;
    float* maxAge = reinterpret_cast<float*>(iBegin + maxAgeOffset);
    float* color = reinterpret_cast<float*>(iBegin + colorOffset + 8);
    const __m128 minColor = _mm_setzero_ps();
    const __m128 maxColor = _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f);
    const float invTimeRange = 1.0f / (endTime - beginTime);
    while (particle != iEnd) {
        const float particleMaxAge = *maxAge;
        const float age = *reinterpret_cast<float*>(
            reinterpret_cast<unsigned char*>(maxAge) + ageOffset - maxAgeOffset);
        const float startAge = particleMaxAge * beginTime;
        if (age >= startAge && particleMaxAge * endTime >= age) {
            const float t = ((age - startAge) / particleMaxAge) * invTimeRange;
            const __m128 shiftedColor = _mm_min_ps(
                _mm_max_ps(_mm_add_ps(beginColor.v,
                                      _mm_mul_ps(colorDelta, _mm_set1_ps(t))),
                           minColor),
                maxColor);
            color[-2] = shiftedColor.m128_f32[0];
            color[-1] = _mm_shuffle_ps(shiftedColor, shiftedColor, 0x55).m128_f32[0];
            color[0] = _mm_shuffle_ps(shiftedColor, shiftedColor, 0xAA).m128_f32[0];
        }
        particle += stride;
        maxAge = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(maxAge) + stride);
        color = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(color) + stride);
    }
}

apsVelocityDragAction::apsVelocityDragAction()
    : apsAction(3, 0, eAsync, 0x4000u) {}
// ea: 0x0080DBE0
void apsVelocityDragAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                apsGroup* ioGroup, apsEffect*, float,
                                float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float v = -(mParams.mElements[2] * iTimeDelta);
    const float scale = ((((v * 0.16666667f) + 0.5f) * v + 1.0f) * v) + 1.0f;

    if ((ioGroup->mPFD.mFields & 0x4000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int velocityOffset = ioGroup->mPFD.mOffsets[14];
    float* velocity = reinterpret_cast<float*>(iBegin + velocityOffset);
    float* endVelocity = reinterpret_cast<float*>(iEnd + velocityOffset);
    while (velocity != endVelocity) {
        velocity[0] *= scale;
        velocity[1] *= scale;
        velocity[2] *= scale;
        velocity = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(velocity) + stride);
    }
}

apsAngularVelocityDragAction::apsAngularVelocityDragAction()
    : apsAction(3, 0, eAsync, 0x10000u) {}
// ea: 0x0080DCE0
void apsAngularVelocityDragAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                       apsGroup* ioGroup, apsEffect*, float,
                                       float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float v = -(mParams.mElements[2] * iTimeDelta);
    const float scale = ((((v * 0.16666667f) + 0.5f) * v + 1.0f) * v) + 1.0f;

    if ((ioGroup->mPFD.mFields & 0x10000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int angularVelocityOffset = ioGroup->mPFD.mOffsets[16];
    float* angularVelocity = reinterpret_cast<float*>(iBegin + angularVelocityOffset);
    float* endAngularVelocity = reinterpret_cast<float*>(iEnd + angularVelocityOffset);
    while (angularVelocity != endAngularVelocity) {
        *angularVelocity *= scale;
        angularVelocity = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(angularVelocity) + stride);
    }
}

apsVectorAngularVelocityDragAction::apsVectorAngularVelocityDragAction()
    : apsAction(3, 0, eAsync, 0x20000u) {}
// ea: 0x0080DDC0
void apsVectorAngularVelocityDragAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                              apsGroup* ioGroup, apsEffect*, float,
                                              float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float v = -(mParams.mElements[2] * iTimeDelta);
    const float scale = ((((v * 0.16666667f) + 0.5f) * v + 1.0f) * v) + 1.0f;

    if ((ioGroup->mPFD.mFields & 0x20000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int vectorAngularVelocityOffset = ioGroup->mPFD.mOffsets[17];
    float* vectorAngularVelocity =
        reinterpret_cast<float*>(iBegin + vectorAngularVelocityOffset);
    float* endVectorAngularVelocity =
        reinterpret_cast<float*>(iEnd + vectorAngularVelocityOffset);
    while (vectorAngularVelocity != endVectorAngularVelocity) {
        vectorAngularVelocity[0] *= scale;
        vectorAngularVelocity[1] *= scale;
        vectorAngularVelocity[2] *= scale;
        vectorAngularVelocity = reinterpret_cast<float*>(
            reinterpret_cast<unsigned char*>(vectorAngularVelocity) + stride);
    }
}

apsWorldPlaneReflectionAction::apsWorldPlaneReflectionAction()
    : apsAction(9, 0, eAsync, 0x4001u) {}
// ea: 0x0080DEC0
void apsWorldPlaneReflectionAction::Act(unsigned char* iBegin,
                                        unsigned char* iEnd,
                                        apsGroup* ioGroup, apsEffect*, float,
                                        float) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float planePointZ = mParams.mElements[4];
    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float planePointY = mParams.mElements[3];
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const math::Dir3 planeNormal(mParams.mElements[2], planePointY,
                                 planePointZ);

    const __m128 normalSquared = _mm_mul_ps(planeNormal.v, planeNormal.v);
    const float normalLength = sqrt(
        normalSquared.m128_f32[0] +
        (normalSquared.m128_f32[1] + normalSquared.m128_f32[2]));
    const __m128 normalizedNormal =
        _mm_div_ps(planeNormal.v, _mm_set1_ps(normalLength));

    if (mParams.mSize <= 7 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float planeZ = mParams.mElements[7];
    if (mParams.mSize <= 6 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float planeY = mParams.mElements[6];
    if (mParams.mSize <= 5 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const math::Dir3 planePosition(mParams.mElements[5], planeY, planeZ);
    if (mParams.mSize <= 8 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float elasticity = mParams.mElements[8];

    const __m128 signMask =
        _mm_setr_ps(-0.0f, -0.0f, -0.0f, -0.0f);
    const __m128 negativeNormal = _mm_xor_ps(signMask, normalizedNormal);
    const __m128 planeOffsetVector =
        _mm_mul_ps(negativeNormal, planePosition.v);
    const float planeOffset =
        planeOffsetVector.m128_f32[0] +
        (planeOffsetVector.m128_f32[1] + planeOffsetVector.m128_f32[2]);

    if ((ioGroup->mPFD.mFields & 0x4000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int velocityOffset = ioGroup->mPFD.mOffsets[14];
    unsigned char* velocityField = iBegin + velocityOffset;
    const unsigned char* endVelocityField = iEnd + velocityOffset;
    if ((ioGroup->mPFD.mFields & 1u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int positionOffset = ioGroup->mPFD.mOffsets[0];
    unsigned char* positionField = iBegin + positionOffset;

    while (velocityField != endVelocityField) {
        math::Dir3 position;
        const float* positionFloats = reinterpret_cast<const float*>(positionField);
        position.v = _mm_setr_ps(positionFloats[0], positionFloats[1],
                                 positionFloats[2], 0.0f);
        const __m128 positionProduct =
            _mm_mul_ps(position.v, normalizedNormal);
        const float positionDot =
            positionProduct.m128_f32[0] +
            (positionProduct.m128_f32[1] + positionProduct.m128_f32[2]);
        if (positionDot + planeOffset < 0.0f) {
            math::Dir3 velocity;
            const float* velocityFloats = reinterpret_cast<const float*>(velocityField);
            velocity.v = _mm_setr_ps(velocityFloats[0], velocityFloats[1],
                                     velocityFloats[2], 0.0f);
            const __m128 velocityProduct =
                _mm_mul_ps(velocity.v, normalizedNormal);
            const float velocityDot =
                velocityProduct.m128_f32[0] +
                (velocityProduct.m128_f32[1] + velocityProduct.m128_f32[2]);
            if (velocityDot < 0.0f) {
                velocity.v = _mm_sub_ps(
                    velocity.v,
                    _mm_mul_ps(normalizedNormal,
                                _mm_set1_ps((elasticity + 1.0f) * velocityDot)));
                float* velocityOutput = reinterpret_cast<float*>(velocityField);
                velocityOutput[0] = velocity.v.m128_f32[0];
                velocityOutput[1] = velocity.v.m128_f32[1];
                velocityOutput[2] = velocity.v.m128_f32[2];
            }
        }
        velocityField += stride;
        positionField += stride;
    }
}

apsUVAFrameAnimAction::apsUVAFrameAnimAction()
    : apsAction(6, 0, eAsync, 0x100u) {}
// ea: 0x0080E2B0
void apsUVAFrameAnimAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                apsGroup* ioGroup, apsEffect*, float,
                                float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    float frameIncrement = 0.0f;
    if (mParams.mSize <= 5 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float frameCycles = mParams.mElements[5];
    const bool fpsBasedOnFrameCycles = frameCycles > 0.0f;
    if (!fpsBasedOnFrameCycles) {
        if (mParams.mSize <= 4 &&
            _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                      "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            __debugbreak();
        frameIncrement = mParams.mElements[4] * iTimeDelta;
    }

    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float beginFrame = mParams.mElements[2];
    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float endFrame = mParams.mElements[3];
    const float framesPerCycle = endFrame - beginFrame;

    if ((ioGroup->mPFD.mFields & 0x100u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int frameOffset = ioGroup->mPFD.mOffsets[8];
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

    unsigned char* frameField = iBegin + frameOffset;
    const unsigned char* endFrameField = iEnd + frameOffset;
    unsigned char* ageField = iBegin + ageOffset;
    unsigned char* maxAgeField = iBegin + maxAgeOffset;
    while (frameField != endFrameField) {
        float* frame = reinterpret_cast<float*>(frameField);
        const float* age = reinterpret_cast<const float*>(ageField);
        const float* maxAge = reinterpret_cast<const float*>(maxAgeField);
        if (fpsBasedOnFrameCycles) {
            if (*maxAge <= *age + iTimeDelta) {
                frameIncrement = 0.0f;
            } else {
                frameIncrement = (framesPerCycle * frameCycles * iTimeDelta) / *maxAge;
            }
        }

        float nextFrame = *frame;
        if (beginFrame > nextFrame)
            nextFrame = beginFrame;
        nextFrame += frameIncrement;
        if (nextFrame >= endFrame)
            nextFrame = (nextFrame - endFrame) + beginFrame;
        if (beginFrame > nextFrame)
            nextFrame = endFrame - (beginFrame - nextFrame);
        *frame = nextFrame;

        frameField += stride;
        ageField += stride;
        maxAgeField += stride;
    }
}

// ============================================================================
// Angle-tracking actions
// ============================================================================
apsAngleTrackVelocityAction::apsAngleTrackVelocityAction()
    : apsAction(0, 0, eAsync, 0x40u) {}

// ea: 0x0080A670
// The mesh-orientation branch tracks each particle's velocity with a
// quaternion.  The release helper intentionally obtains both offsets through
// apsPFD::GetOffset, preserving its missing-field assertion semantics.
void apsAngleTrackMeshOrientation(unsigned char* iBegin, unsigned char* iEnd,
                                  apsGroup* ioGroup, apsEffect*, float) {
    const int stride = ioGroup->mPFD.mStride;
    const int velocityOffset = ioGroup->mPFD.GetOffset(apsPFDField_Velocity);
    const int orientationOffset = ioGroup->mPFD.GetOffset(apsPFDField_Orientation);

    unsigned char* velocity = iBegin + velocityOffset;
    const unsigned char* velocityEnd = iEnd + velocityOffset;
    unsigned char* orientation = iBegin + orientationOffset;

    while (velocity != velocityEnd) {
        math::Dir3 direction;
        direction.v = _mm_setr_ps(
            reinterpret_cast<float*>(velocity)[0],
            reinterpret_cast<float*>(velocity)[1],
            reinterpret_cast<float*>(velocity)[2], 0.0f);

        if ((ioGroup->mFlags & 2) != 0) {
            direction.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(direction.v, direction.v, 0),
                               ioGroup->mLocalToWorld.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(direction.v, direction.v, 85),
                               ioGroup->mLocalToWorld.y.v)),
                _mm_mul_ps(_mm_shuffle_ps(direction.v, direction.v, 170),
                           ioGroup->mLocalToWorld.z.v));
        }

        const __m128 squared = _mm_mul_ps(direction.v, direction.v);
        const float lengthSquared = squared.m128_f32[0] +
                                    (squared.m128_f32[1] + squared.m128_f32[2]);
        const float length = sqrt(lengthSquared);
        direction.v = _mm_div_ps(direction.v, _mm_set1_ps(length));

        math::Dir3 up;
        up.v = Float4_NegZAxis_123;
        math::Mat43 matrix;
        apsMath::CreateFromVectorsDirUp(matrix, direction, up);
        const apsQuaternion orientationQuaternion = apsMath::QuaternionFromMatrix(matrix);

        float* orientationFloats = reinterpret_cast<float*>(orientation);
        orientationFloats[0] = orientationQuaternion.x;
        orientationFloats[1] = orientationQuaternion.y;
        orientationFloats[2] = orientationQuaternion.z;
        orientationFloats[3] = orientationQuaternion.w;

        velocity += stride;
        orientation += stride;
    }
}

// ea: 0x0080A830
void apsAngleTrackVelocityAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                       apsGroup* ioGroup, apsEffect* iEffect,
                                       float, float iTimeDelta) {
    if ((ioGroup->mPFD.mFields & 0x20u) != 0) {
        apsAngleTrackMeshOrientation(iBegin, iEnd, ioGroup, iEffect, iTimeDelta);
        return;
    }

    const int stride = ioGroup->mPFD.mStride;
    const int velocityOffset = ioGroup->mPFD.GetOffset(apsPFDField_Velocity);
    const int angleOffset = ioGroup->mPFD.GetOffset(apsPFDField_Angle);
    const apsCommon::CameraSettings& camera = apsCommon::mCamera;
    const float rollOffset = camera.mRoll - 1.5707964f;

    unsigned char* velocity = iBegin + velocityOffset;
    const unsigned char* velocityEnd = iEnd + velocityOffset;
    unsigned char* angle = iBegin + angleOffset;

    if ((ioGroup->mFlags & 2) != 0) {
        while (velocity != velocityEnd) {
            math::Dir3 direction;
            direction.v = _mm_setr_ps(
                reinterpret_cast<float*>(velocity)[0],
                reinterpret_cast<float*>(velocity)[1],
                reinterpret_cast<float*>(velocity)[2], 0.0f);
            direction.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(direction.v, direction.v, 0),
                               ioGroup->mLocalToWorld.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(direction.v, direction.v, 85),
                               ioGroup->mLocalToWorld.y.v)),
                _mm_mul_ps(_mm_shuffle_ps(direction.v, direction.v, 170),
                           ioGroup->mLocalToWorld.z.v));

            const __m128 upProduct = _mm_mul_ps(direction.v, camera.mUp.v);
            const __m128 leftProduct = _mm_mul_ps(direction.v, camera.mLeft.v);
            const float upDot = upProduct.m128_f32[0] +
                                (upProduct.m128_f32[1] + upProduct.m128_f32[2]);
            const float leftDot = leftProduct.m128_f32[0] +
                                  (leftProduct.m128_f32[1] + leftProduct.m128_f32[2]);
            if (upDot != 0.0f || leftDot != 0.0f) {
                const double angleValue = apsMath::ATan(upDot, leftDot) +
                                           static_cast<double>(rollOffset);
                *reinterpret_cast<float*>(angle) =
                    static_cast<float>(angleValue * camera.mXFlip);
            }

            velocity += stride;
            angle += stride;
        }
    } else {
        while (velocity != velocityEnd) {
            const math::Dir3 direction(_mm_setr_ps(
                reinterpret_cast<float*>(velocity)[0],
                reinterpret_cast<float*>(velocity)[1],
                reinterpret_cast<float*>(velocity)[2], 0.0f));
            const __m128 upProduct = _mm_mul_ps(direction.v, camera.mUp.v);
            const __m128 leftProduct = _mm_mul_ps(direction.v, camera.mLeft.v);
            const float upDot = upProduct.m128_f32[0] +
                                (upProduct.m128_f32[1] + upProduct.m128_f32[2]);
            const float leftDot = leftProduct.m128_f32[0] +
                                  (leftProduct.m128_f32[1] + leftProduct.m128_f32[2]);
            if (upDot != 0.0f || leftDot != 0.0f) {
                const double angleValue = apsMath::ATan(upDot, leftDot) +
                                           static_cast<double>(rollOffset);
                *reinterpret_cast<float*>(angle) =
                    static_cast<float>(angleValue * camera.mXFlip);
            }

            velocity += stride;
            angle += stride;
        }
    }
}

apsAngleTrackElementXAction::apsAngleTrackElementXAction()
    : apsAction(0, 0, eAsync, 0x1000u) {}
// ea: 0x0080E520
void apsAngleTrackElementXAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                      apsGroup* ioGroup, apsEffect*, float, float) {
    if ((ioGroup->mPFD.mFields & 0x40u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();

    const apsCommon::CameraSettings& camera = apsCommon::mCamera;
    const math::Dir3& axis = ioGroup->mLocalToWorld.x;
    const __m128 upProduct = _mm_mul_ps(axis.v, camera.mUp.v);
    const float upDot = upProduct.m128_f32[0] +
                        (upProduct.m128_f32[1] + upProduct.m128_f32[2]);
    const __m128 leftProduct = _mm_mul_ps(axis.v, camera.mLeft.v);
    const float leftDot = leftProduct.m128_f32[0] +
                          (leftProduct.m128_f32[1] + leftProduct.m128_f32[2]);
    if (upDot != 0.0f || leftDot != 0.0f) {
        const double angle = apsMath::ATan(upDot, leftDot) +
                             static_cast<double>(camera.mRoll - 1.5707964f);
        const float angleValue = static_cast<float>(angle * camera.mXFlip);
        unsigned char* angleField = iBegin + ioGroup->mPFD.mOffsets[6];
        while (angleField < iEnd) {
            *reinterpret_cast<float*>(angleField) = angleValue;
            angleField += ioGroup->mPFD.mStride;
        }
    }
}

apsAngleTrackElementYAction::apsAngleTrackElementYAction()
    : apsAction(0, 0, eAsync, 0x1000u) {}
// ea: 0x0080E660
void apsAngleTrackElementYAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                      apsGroup* ioGroup, apsEffect*, float, float) {
    if ((ioGroup->mPFD.mFields & 0x40u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();

    const apsCommon::CameraSettings& camera = apsCommon::mCamera;
    const math::Dir3& axis = ioGroup->mLocalToWorld.y;
    const __m128 upProduct = _mm_mul_ps(axis.v, camera.mUp.v);
    const float upDot = upProduct.m128_f32[0] +
                        (upProduct.m128_f32[1] + upProduct.m128_f32[2]);
    const __m128 leftProduct = _mm_mul_ps(axis.v, camera.mLeft.v);
    const float leftDot = leftProduct.m128_f32[0] +
                          (leftProduct.m128_f32[1] + leftProduct.m128_f32[2]);
    if (upDot != 0.0f || leftDot != 0.0f) {
        const double angle = apsMath::ATan(upDot, leftDot) +
                             static_cast<double>(camera.mRoll - 1.5707964f);
        const float angleValue = static_cast<float>(angle * camera.mXFlip);
        unsigned char* angleField = iBegin + ioGroup->mPFD.mOffsets[6];
        while (angleField < iEnd) {
            *reinterpret_cast<float*>(angleField) = angleValue;
            angleField += ioGroup->mPFD.mStride;
        }
    }
}

apsAngleTrackElementZAction::apsAngleTrackElementZAction()
    : apsAction(0, 0, eAsync, 0x1000u) {}
// ea: 0x0080E7A0
void apsAngleTrackElementZAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                      apsGroup* ioGroup, apsEffect*, float, float) {
    if ((ioGroup->mPFD.mFields & 0x40u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();

    const apsCommon::CameraSettings& camera = apsCommon::mCamera;
    const math::Dir3& axis = ioGroup->mLocalToWorld.z;
    const __m128 upProduct = _mm_mul_ps(axis.v, camera.mUp.v);
    const float upDot = upProduct.m128_f32[0] +
                        (upProduct.m128_f32[1] + upProduct.m128_f32[2]);
    const __m128 leftProduct = _mm_mul_ps(axis.v, camera.mLeft.v);
    const float leftDot = leftProduct.m128_f32[0] +
                          (leftProduct.m128_f32[1] + leftProduct.m128_f32[2]);
    if (upDot != 0.0f || leftDot != 0.0f) {
        const double angle = apsMath::ATan(upDot, leftDot) +
                             static_cast<double>(camera.mRoll - 1.5707964f);
        const float angleValue = static_cast<float>(angle * camera.mXFlip);
        unsigned char* angleField = iBegin + ioGroup->mPFD.mOffsets[6];
        while (angleField < iEnd) {
            *reinterpret_cast<float*>(angleField) = angleValue;
            angleField += ioGroup->mPFD.mStride;
        }
    }
}

// ============================================================================
// Attractor actions
// ============================================================================
apsPointAttractorAction::apsPointAttractorAction()
    : apsAction(6, 0, eAsync, 0x4001u) {}

// ea: 0x0080E8E0
void apsPointAttractorAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                  apsGroup* ioGroup, apsEffect*, float, float iTimeDelta) {
    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float centerZ = mParams.mElements[4];

    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float centerY = mParams.mElements[3];

    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    math::Dir3 center;
    center.v = _mm_setr_ps(mParams.mElements[2], centerY, centerZ, 0.0f);

    if (mParams.mSize <= 5 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float force = mParams.mElements[5] * iTimeDelta;

    if ((ioGroup->mFlags & 2u) == 0)
        center = apsMath::XForm3d_1(ioGroup->mLocalToWorld, center);

    if ((ioGroup->mPFD.mFields & (1u << apsPFDField_Position)) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* position =
        iBegin + ioGroup->mPFD.GetOffset(apsPFDField_Position);

    if ((ioGroup->mPFD.mFields & (1u << apsPFDField_Velocity)) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* velocity =
        iBegin + ioGroup->mPFD.GetOffset(apsPFDField_Velocity);
    const int positionStride = ioGroup->mPFD.mStride;
    const int velocityStride = ioGroup->mPFD.mStride;

    while (position < iEnd) {
        const __m128 positionValue = _mm_setr_ps(
            reinterpret_cast<float*>(position)[0],
            reinterpret_cast<float*>(position)[1],
            reinterpret_cast<float*>(position)[2], 0.0f);
        const __m128 delta = _mm_sub_ps(positionValue, center.v);
        const __m128 squared = _mm_mul_ps(delta, delta);
        const float distanceSquared = squared.m128_f32[0] +
                                      (squared.m128_f32[1] + squared.m128_f32[2]);
        float clampedDistanceSquared = distanceSquared;
        if (clampedDistanceSquared < 0.001f)
            clampedDistanceSquared = 0.001f;

        const float distance = std::sqrt(clampedDistanceSquared);
        const __m128 normalizedDelta =
            _mm_div_ps(delta, _mm_set1_ps(distance));
        const __m128 adjustment = _mm_mul_ps(
            normalizedDelta,
            _mm_set1_ps(force / clampedDistanceSquared));

        const __m128 oldVelocity = _mm_setr_ps(
            reinterpret_cast<float*>(velocity)[0],
            reinterpret_cast<float*>(velocity)[1],
            reinterpret_cast<float*>(velocity)[2], 0.0f);
        const __m128 newVelocity = _mm_sub_ps(oldVelocity, adjustment);
        reinterpret_cast<float*>(velocity)[0] = newVelocity.m128_f32[0];
        reinterpret_cast<float*>(velocity)[1] = newVelocity.m128_f32[1];
        reinterpret_cast<float*>(velocity)[2] = newVelocity.m128_f32[2];

        position += positionStride;
        velocity += velocityStride;
    }
}

apsLineAttractorAction::apsLineAttractorAction()
    : apsAction(9, 0, eAsync, 0x4001u) {}
// ea: 0x0080EBDC
void apsLineAttractorAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                 apsGroup* ioGroup, apsEffect*, float,
                                 float iTimeDelta) {
    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float startZ = mParams.mElements[4];
    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float startY = mParams.mElements[3];
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    math::Dir3 lineStart(mParams.mElements[2], startY, startZ);

    if (mParams.mSize <= 7 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float endZ = mParams.mElements[7];
    if (mParams.mSize <= 6 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float endY = mParams.mElements[6];
    if (mParams.mSize <= 5 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    math::Dir3 lineEnd(mParams.mElements[5], endY, endZ);

    if (mParams.mSize <= 8 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float strengthDelta = mParams.mElements[8] * iTimeDelta;

    if ((ioGroup->mFlags & 2u) == 0) {
        lineStart = apsMath::XForm3d_1(ioGroup->mLocalToWorld, lineStart);
        lineEnd = apsMath::XForm3d_1(ioGroup->mLocalToWorld, lineEnd);
    }

    const __m128 lineDelta = _mm_sub_ps(lineEnd.v, lineStart.v);
    const __m128 lineDeltaSquared = _mm_mul_ps(lineDelta, lineDelta);
    const float lineLength = sqrt(
        lineDeltaSquared.m128_f32[0] +
        (lineDeltaSquared.m128_f32[1] + lineDeltaSquared.m128_f32[2]));
    const __m128 lineDirection =
        _mm_div_ps(lineDelta, _mm_set1_ps(lineLength));

    if ((ioGroup->mPFD.mFields & 1u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    float* position = reinterpret_cast<float*>(
        iBegin + ioGroup->mPFD.mOffsets[0]);
    const int stride = ioGroup->mPFD.mStride;
    if ((ioGroup->mPFD.mFields & 0x4000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    float* velocity = reinterpret_cast<float*>(
        iBegin + ioGroup->mPFD.mOffsets[14]);

    while (reinterpret_cast<unsigned char*>(position) < iEnd) {
        const __m128 particlePosition =
            _mm_setr_ps(position[0], position[1], position[2], 0.0f);
        const __m128 fromStart = _mm_sub_ps(particlePosition, lineStart.v);
        const __m128 projectionProduct =
            _mm_mul_ps(fromStart, lineDirection);
        const float projection =
            projectionProduct.m128_f32[0] +
            (projectionProduct.m128_f32[1] + projectionProduct.m128_f32[2]);
        const __m128 closestPoint = _mm_add_ps(
            lineStart.v,
            _mm_mul_ps(lineDirection, _mm_set1_ps(projection)));
        const __m128 toLine = _mm_sub_ps(particlePosition, closestPoint);
        const __m128 distanceSquaredVector = _mm_mul_ps(toLine, toLine);
        const float distanceSquared =
            distanceSquaredVector.m128_f32[0] +
            (distanceSquaredVector.m128_f32[1] + distanceSquaredVector.m128_f32[2]);
        const float clampedDistanceSquared =
            distanceSquared < 0.001f ? 0.001f : distanceSquared;
        const __m128 oldVelocity = _mm_setr_ps(
            velocity[0], velocity[1], velocity[2], 0.0f);
        const __m128 adjustment = _mm_mul_ps(
            _mm_div_ps(toLine, _mm_set1_ps(sqrt(clampedDistanceSquared))),
            _mm_set1_ps(strengthDelta / clampedDistanceSquared));
        const __m128 newVelocity = _mm_sub_ps(oldVelocity, adjustment);
        velocity[0] = newVelocity.m128_f32[0];
        velocity[1] = newVelocity.m128_f32[1];
        velocity[2] = newVelocity.m128_f32[2];

        position = reinterpret_cast<float*>(
            reinterpret_cast<unsigned char*>(position) + stride);
        velocity = reinterpret_cast<float*>(
            reinterpret_cast<unsigned char*>(velocity) + stride);
    }
}

apsDecayLineAttractorAction::apsDecayLineAttractorAction()
    : apsAction(9, 0, eAsync, 0x4001u) {}
// ea: 0x0080F050
void apsDecayLineAttractorAction::Act(unsigned char* iBegin,
                                      unsigned char* iEnd,
                                      apsGroup* ioGroup, apsEffect*, float,
                                      float iTimeDelta) {
    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float startZ = mParams.mElements[4];
    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float startY = mParams.mElements[3];
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    math::Dir3 lineStart(mParams.mElements[2], startY, startZ);

    if (mParams.mSize <= 7 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float endZ = mParams.mElements[7];
    if (mParams.mSize <= 6 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float endY = mParams.mElements[6];
    if (mParams.mSize <= 5 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    math::Dir3 lineEnd(mParams.mElements[5], endY, endZ);

    if (mParams.mSize <= 8 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float strengthDelta = mParams.mElements[8] * iTimeDelta;

    if ((ioGroup->mFlags & 2u) == 0) {
        lineStart = apsMath::XForm3d_1(ioGroup->mLocalToWorld, lineStart);
        lineEnd = apsMath::XForm3d_1(ioGroup->mLocalToWorld, lineEnd);
    }

    const __m128 lineDelta = _mm_sub_ps(lineEnd.v, lineStart.v);
    const __m128 lineDeltaSquared = _mm_mul_ps(lineDelta, lineDelta);
    const float lineLengthSquared =
        lineDeltaSquared.m128_f32[0] +
        (lineDeltaSquared.m128_f32[1] + lineDeltaSquared.m128_f32[2]);
    const float inverseLineLength = 1.0f / sqrt(lineLengthSquared);
    const __m128 lineDirection =
        _mm_mul_ps(lineDelta, _mm_set1_ps(inverseLineLength));

    if ((ioGroup->mPFD.mFields & 1u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    float* position = reinterpret_cast<float*>(
        iBegin + ioGroup->mPFD.mOffsets[0]);
    const int stride = ioGroup->mPFD.mStride;
    if ((ioGroup->mPFD.mFields & 0x4000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    float* velocity = reinterpret_cast<float*>(
        iBegin + ioGroup->mPFD.mOffsets[14]);

    while (reinterpret_cast<unsigned char*>(position) < iEnd) {
        const __m128 particlePosition =
            _mm_setr_ps(position[0], position[1], position[2], 0.0f);
        const __m128 fromStart = _mm_sub_ps(particlePosition, lineStart.v);
        const __m128 projectionProduct =
            _mm_mul_ps(fromStart, lineDirection);
        const float projection =
            projectionProduct.m128_f32[0] +
            (projectionProduct.m128_f32[1] + projectionProduct.m128_f32[2]);
        float lineFraction = projection * inverseLineLength;
        if (lineFraction >= 0.0f) {
            if (lineFraction > 1.0f)
                lineFraction = 1.0f;
        } else {
            lineFraction = 0.0f;
        }
        const float decay = 1.0f - lineFraction;

        const __m128 closestPoint = _mm_add_ps(
            lineStart.v,
            _mm_mul_ps(lineDirection, _mm_set1_ps(projection)));
        const __m128 toLine = _mm_sub_ps(particlePosition, closestPoint);
        const __m128 distanceSquaredVector = _mm_mul_ps(toLine, toLine);
        const float distanceSquared =
            distanceSquaredVector.m128_f32[0] +
            (distanceSquaredVector.m128_f32[1] + distanceSquaredVector.m128_f32[2]);
        const float clampedDistanceSquared =
            distanceSquared < 0.001f ? 0.001f : distanceSquared;
        const __m128 oldVelocity = _mm_setr_ps(
            velocity[0], velocity[1], velocity[2], 0.0f);
        const __m128 adjustment = _mm_mul_ps(
            _mm_div_ps(toLine, _mm_set1_ps(sqrt(clampedDistanceSquared))),
            _mm_set1_ps((decay / clampedDistanceSquared) * strengthDelta));
        const __m128 newVelocity = _mm_sub_ps(oldVelocity, adjustment);
        velocity[0] = newVelocity.m128_f32[0];
        velocity[1] = newVelocity.m128_f32[1];
        velocity[2] = newVelocity.m128_f32[2];

        position = reinterpret_cast<float*>(
            reinterpret_cast<unsigned char*>(position) + stride);
        velocity = reinterpret_cast<float*>(
            reinterpret_cast<unsigned char*>(velocity) + stride);
    }
}

apsKappaTauAction::apsKappaTauAction()
    : apsAction(11, 0, eAsync, 0x7C4000u) {}
// ea: 0x0080F500
void apsKappaTauAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                            apsGroup* ioGroup, apsEffect*, float, float iTimeDelta) {
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float speed = mParams.mElements[2];

    if (mParams.mSize <= 5 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float minCurvature = mParams.mElements[5];

    if (mParams.mSize <= 6 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float maxCurvature = mParams.mElements[6];

    if (mParams.mSize <= 9 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float minTorsion = mParams.mElements[9];

    if (mParams.mSize <= 10 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float maxTorsion = mParams.mElements[10];

    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float minCurvatureChange = mParams.mElements[3];

    if (mParams.mSize <= 4 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float maxCurvatureChange = mParams.mElements[4];

    if (mParams.mSize <= 7 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float minTorsionChange = mParams.mElements[7];

    if (mParams.mSize <= 8 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float maxTorsionChange = mParams.mElements[8];

    if ((ioGroup->mPFD.mFields & 0x4000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* velocityField = iBegin + ioGroup->mPFD.mOffsets[14];
    const int velocityStride = ioGroup->mPFD.mStride;

    if ((ioGroup->mPFD.mFields & 0x40000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* binormalField = iBegin + ioGroup->mPFD.mOffsets[18];
    const int binormalStride = ioGroup->mPFD.mStride;

    if ((ioGroup->mPFD.mFields & 0x80000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* normalField = iBegin + ioGroup->mPFD.mOffsets[19];
    const int normalStride = ioGroup->mPFD.mStride;

    if ((ioGroup->mPFD.mFields & 0x100000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* tangentField = iBegin + ioGroup->mPFD.mOffsets[20];
    const int tangentStride = ioGroup->mPFD.mStride;

    if ((ioGroup->mPFD.mFields & 0x200000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* curvatureField = iBegin + ioGroup->mPFD.mOffsets[21];
    const int curvatureStride = ioGroup->mPFD.mStride;

    if ((ioGroup->mPFD.mFields & 0x400000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    unsigned char* torsionField = iBegin + ioGroup->mPFD.mOffsets[22];
    const int torsionStride = ioGroup->mPFD.mStride;

    if (velocityField < iEnd) {
        const __m128 speedVector = _mm_set1_ps(speed);
        const __m128 timeDeltaVector = _mm_set1_ps(iTimeDelta);
        const float curvatureChangeRange = maxCurvatureChange - minCurvatureChange;
        while (velocityField < iEnd) {
            float* const velocity = reinterpret_cast<float*>(velocityField);
            float* const binormal = reinterpret_cast<float*>(binormalField);
            float* const normal = reinterpret_cast<float*>(normalField);
            float* const tangent = reinterpret_cast<float*>(tangentField);
            float* const curvature = reinterpret_cast<float*>(curvatureField);
            float* const torsion = reinterpret_cast<float*>(torsionField);

            const float oldCurvature = *curvature;
            const float oldTorsion = *torsion;
            const __m128 binormalVector = _mm_setr_ps(
                binormal[0], binormal[1], binormal[2], 0.0f);
            const __m128 normalVector = _mm_setr_ps(
                normal[0], normal[1], normal[2], 0.0f);
            const __m128 tangentVector = _mm_setr_ps(
                tangent[0], tangent[1], tangent[2], 0.0f);

            const __m128 newVelocity = _mm_mul_ps(binormalVector, speedVector);
            const __m128 newBinormal = _mm_add_ps(
                binormalVector,
                _mm_mul_ps(_mm_mul_ps(normalVector, _mm_set1_ps(oldCurvature)),
                           timeDeltaVector));
            const __m128 newTangent = _mm_sub_ps(
                tangentVector,
                _mm_mul_ps(_mm_mul_ps(normalVector, _mm_set1_ps(oldTorsion)),
                           timeDeltaVector));

            const __m128 newBinormalSquared = _mm_mul_ps(newBinormal, newBinormal);
            const float newBinormalLength = sqrt(
                newBinormalSquared.m128_f32[0] +
                (newBinormalSquared.m128_f32[1] + newBinormalSquared.m128_f32[2]));
            const __m128 normalizedBinormal = _mm_div_ps(
                newBinormal, _mm_set1_ps(newBinormalLength));

            const __m128 newTangentSquared = _mm_mul_ps(newTangent, newTangent);
            const float newTangentLength = sqrt(
                newTangentSquared.m128_f32[0] +
                (newTangentSquared.m128_f32[1] + newTangentSquared.m128_f32[2]));
            const __m128 normalizedTangent = _mm_div_ps(
                newTangent, _mm_set1_ps(newTangentLength));
            const __m128 newNormal = _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(normalizedBinormal, normalizedBinormal, 9),
                           _mm_shuffle_ps(normalizedTangent, normalizedTangent, 18)),
                _mm_mul_ps(_mm_shuffle_ps(normalizedBinormal, normalizedBinormal, 18),
                           _mm_shuffle_ps(normalizedTangent, normalizedTangent, 9)));

            const float newCurvature =
                (apsMath::gDefaultRandomNumberGenerator.GetFloat() *
                     curvatureChangeRange + minCurvatureChange) * iTimeDelta +
                oldCurvature;
            const float newTorsion =
                (apsMath::gDefaultRandomNumberGenerator.GetFloat() *
                     (maxTorsionChange - minTorsionChange) + minTorsionChange) *
                    iTimeDelta + oldTorsion;
            const float clampedCurvature =
                newCurvature < minCurvature ? minCurvature :
                (newCurvature > maxCurvature ? maxCurvature : newCurvature);
            const float clampedTorsion =
                newTorsion < minTorsion ? minTorsion :
                (newTorsion > maxTorsion ? maxTorsion : newTorsion);

            velocity[0] = newVelocity.m128_f32[0];
            velocity[1] = newVelocity.m128_f32[1];
            velocity[2] = newVelocity.m128_f32[2];
            normal[0] = newNormal.m128_f32[0];
            normal[1] = newNormal.m128_f32[1];
            normal[2] = newNormal.m128_f32[2];
            binormal[0] = normalizedBinormal.m128_f32[0];
            binormal[1] = normalizedBinormal.m128_f32[1];
            binormal[2] = normalizedBinormal.m128_f32[2];
            tangent[0] = normalizedTangent.m128_f32[0];
            tangent[1] = normalizedTangent.m128_f32[1];
            tangent[2] = normalizedTangent.m128_f32[2];
            *curvature = clampedCurvature;
            *torsion = clampedTorsion;

            velocityField += velocityStride;
            binormalField += binormalStride;
            normalField += normalStride;
            tangentField += tangentStride;
            curvatureField += curvatureStride;
            torsionField += torsionStride;
        }
    }
}

// ============================================================================
// Spawn (nested effects)
// ============================================================================
namespace {

// ea: 0x0080AC00
void CalculateOrientation(math::Mat43* out, const math::Dir3& forward) {
    __m128 up = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    const __m128 forwardProduct = _mm_mul_ps(up, forward.v);
    const float forwardUpDot = forwardProduct.m128_f32[0] +
                               (forwardProduct.m128_f32[1] + forwardProduct.m128_f32[2]);
    const __m128 absMask = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff));
    const float forwardUpAbs = _mm_and_ps(_mm_set_ss(forwardUpDot), absMask).m128_f32[0];
    if (1.0f - forwardUpAbs < 0.001f)
        up = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);

    const __m128 forwardZXY = _mm_shuffle_ps(forward.v, forward.v, 18);
    const __m128 forwardYZX = _mm_shuffle_ps(forward.v, forward.v, 9);
    const __m128 upZXY = _mm_shuffle_ps(up, up, 18);
    const __m128 upYZX = _mm_shuffle_ps(up, up, 9);

    const __m128 sideUnnormalized = _mm_sub_ps(
        _mm_mul_ps(forwardYZX, upZXY),
        _mm_mul_ps(forwardZXY, upYZX));
    const __m128 sideSquared = _mm_mul_ps(sideUnnormalized, sideUnnormalized);
    const float sideLength = std::sqrt(
        sideSquared.m128_f32[0] +
        (sideSquared.m128_f32[1] + sideSquared.m128_f32[2]));
    const __m128 side = _mm_div_ps(sideUnnormalized, _mm_set1_ps(sideLength));

    const __m128 forwardSideZXY = _mm_shuffle_ps(forward.v, forward.v, 18);
    const __m128 forwardSideYZX = _mm_shuffle_ps(forward.v, forward.v, 9);
    const __m128 sideZXY = _mm_shuffle_ps(side, side, 18);
    const __m128 sideYZX = _mm_shuffle_ps(side, side, 9);
    const __m128 rightUnnormalized = _mm_sub_ps(
        _mm_mul_ps(sideYZX, forwardSideZXY),
        _mm_mul_ps(sideZXY, forwardSideYZX));
    const __m128 rightSquared = _mm_mul_ps(rightUnnormalized, rightUnnormalized);
    const float rightLength = std::sqrt(
        rightSquared.m128_f32[0] +
        (rightSquared.m128_f32[1] + rightSquared.m128_f32[2]));

    out->y.v = side;
    out->x.v = _mm_div_ps(rightUnnormalized, _mm_set1_ps(rightLength));
    out->z.v = forward.v;
}

// ea: 0x0080ADA0
void OrientIdentity(math::Mat43* out, const math::Dir3::Packed*,
                    const math::Dir3::Packed*) {
    apsMath::SetIdentityMatrix(*out);
}

// ea: 0x0080ADC0
void OrientParentVelocity(math::Mat43* out,
                          const math::Dir3::Packed* parentVelocity,
                          const math::Dir3::Packed*) {
    if (parentVelocity == nullptr &&
        _tlAssert("source/apsSuppliedActions.cpp", 2578, "parentVelocity",
                  "MUST HAVE A PARENT VELOCITY POINTER"))
        __debugbreak();

    math::Dir3 direction;
    direction.v = _mm_setr_ps(parentVelocity->x, parentVelocity->y,
                              parentVelocity->z, 0.0f);
    const __m128 squared = _mm_mul_ps(direction.v, direction.v);
    const float length = std::sqrt(
        squared.m128_f32[0] +
        (squared.m128_f32[1] + squared.m128_f32[2]));
    direction.v = _mm_div_ps(direction.v, _mm_set1_ps(length));
    CalculateOrientation(out, direction);
}

// ea: 0x0080AE80
void OrientCollisionReflect(math::Mat43* out,
                            const math::Dir3::Packed* parentVelocity,
                            const math::Dir3::Packed* collisionNormal) {
    if (parentVelocity == nullptr &&
        _tlAssert("source/apsSuppliedActions.cpp", 2586, "parentVelocity",
                  "MUST HAVE A PARENT VELOCITY POINTER"))
        __debugbreak();
    if (collisionNormal == nullptr &&
        _tlAssert("source/apsSuppliedActions.cpp", 2587, "collisionNormal",
                  "MUST HAVE A COLLISION NORMAL POINTER"))
        __debugbreak();

    math::Dir3 direction;
    direction.v = _mm_setr_ps(parentVelocity->x, parentVelocity->y,
                              parentVelocity->z, 0.0f);
    const __m128 squared = _mm_mul_ps(direction.v, direction.v);
    const float length = std::sqrt(
        squared.m128_f32[0] +
        (squared.m128_f32[1] + squared.m128_f32[2]));
    const __m128 normalizedVelocity =
        _mm_div_ps(direction.v, _mm_set1_ps(length));

    const __m128 normal = _mm_setr_ps(collisionNormal->x, collisionNormal->y,
                                      collisionNormal->z, 0.0f);
    const __m128 product = _mm_mul_ps(normalizedVelocity, normal);
    const float dot = product.m128_f32[0] +
                      (product.m128_f32[1] + product.m128_f32[2]);
    direction.v = _mm_sub_ps(
        normalizedVelocity,
        _mm_mul_ps(normal, _mm_set1_ps(2.0f * dot)));
    CalculateOrientation(out, direction);
}

// ea: 0x0080AFD0
void OrientRandom(math::Mat43* out, const math::Dir3::Packed*,
                  const math::Dir3::Packed*) {
    math::Dir3 direction;
    direction.v = _mm_setr_ps(
        apsMath::gDefaultRandomNumberGenerator.GetFloat(-1.0f, 1.0f),
        apsMath::gDefaultRandomNumberGenerator.GetFloat(-1.0f, 1.0f),
        apsMath::gDefaultRandomNumberGenerator.GetFloat(-1.0f, 1.0f),
        0.0f);
    const __m128 squared = _mm_mul_ps(direction.v, direction.v);
    const float length = std::sqrt(
        squared.m128_f32[0] +
        (squared.m128_f32[1] + squared.m128_f32[2]));
    direction.v = _mm_div_ps(direction.v, _mm_set1_ps(length));
    CalculateOrientation(out, direction);
}

} // namespace

apsSpawnAction::apsSpawnAction()
    : apsAction(5, 0, eAsync, 0x800001u) {}

// ea: 0x0080FC50
void apsSpawnAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                         apsGroup* ioGroup, apsEffect* iEffect, float,
                         float iTimeDelta) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const float densityInterval = mParams.mElements[2];

    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const apsEffectTemplate* effectTemplate =
        reinterpret_cast<const apsEffectTemplate*>(
            static_cast<uintptr_t>(apsMath::FloatAsInt(mParams.mElements[3])));
    if (effectTemplate == nullptr)
        return;

    unsigned char* densityTime =
        iBegin + ioGroup->mPFD.GetOffset(apsPFDField_DensityTime);
    unsigned char* age = iBegin + ioGroup->mPFD.GetOffset(apsPFDField_Age);
    unsigned char* maxAge =
        iBegin + ioGroup->mPFD.GetOffset(apsPFDField_MaxAge);
    unsigned char* orientationField = nullptr;
    if ((ioGroup->mPFD.mFields & (1u << apsPFDField_Orientation)) != 0)
        orientationField =
            iBegin + ioGroup->mPFD.GetOffset(apsPFDField_Orientation);

    if (iBegin == iEnd)
        return;

    while (iBegin != iEnd) {
        float* density = reinterpret_cast<float*>(densityTime);
        if (*density == 0.0f) {
            if (mParams.mSize <= 4 &&
                _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                          "iIndex >= 0 && iIndex < mSize", "out of bounds"))
                __debugbreak();
            *density += mParams.mElements[4];
        }

        *density -= iTimeDelta;
        if (*density <= 0.0f) {
            *density += densityInterval;
            if (*density <= 0.0f)
                *density = densityInterval;

            const float parentAgePercent =
                *reinterpret_cast<float*>(age) /
                *reinterpret_cast<float*>(maxAge);
            math::Dir3 position;
            position.v = _mm_setr_ps(
                reinterpret_cast<float*>(iBegin)[0],
                reinterpret_cast<float*>(iBegin)[1],
                reinterpret_cast<float*>(iBegin)[2], 0.0f);

            apsQuaternion orientation(0.0f, 0.0f, 0.0f, 1.0f);
            if (orientationField != nullptr)
                orientation = *reinterpret_cast<const apsQuaternion*>(orientationField);

            apsInternal::QueueSpawnedEffect(iEffect->mId, effectTemplate,
                                            g_effectTime, orientation, position,
                                            parentAgePercent);
        }

        densityTime += stride;
        iBegin += stride;
        if (orientationField != nullptr)
            orientationField += stride;
        age += stride;
        maxAge += stride;
    }
}

apsSpawnOnDeathAction::apsSpawnOnDeathAction()
    : apsAction(4, 0, eAsync, 0x8000001u) {}

// ea: 0x0080FEB0
void apsSpawnOnDeathAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                                 apsGroup* ioGroup, apsEffect* iEffect,
                                 float, float) {
    const int stride = ioGroup->mPFD.mStride;
    if (mParams.mSize <= 2 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();

    const apsEffectTemplate* effectTemplate =
        reinterpret_cast<const apsEffectTemplate*>(
            static_cast<uintptr_t>(apsMath::FloatAsInt(mParams.mElements[2])));
    if (effectTemplate == nullptr)
        return;

    unsigned char* orientation = nullptr;
    if ((ioGroup->mPFD.mFields & 0x20u) != 0)
        orientation = iBegin + ioGroup->mPFD.GetOffset(apsPFDField_Orientation);

    unsigned char* velocity = nullptr;
    if ((ioGroup->mPFD.mFields & 0x4000u) != 0)
        velocity = iBegin + ioGroup->mPFD.GetOffset(apsPFDField_Velocity);

    unsigned char* collisionNormal = nullptr;
    if ((ioGroup->mPFD.mFields & 0x20000000u) != 0)
        collisionNormal = iBegin +
                          ioGroup->mPFD.GetOffset(apsPFDField_LastCollisionNormal);

    if (mParams.mSize <= 3 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                  "iIndex >= 0 && iIndex < mSize", "out of bounds"))
        __debugbreak();
    const int orientationMode = static_cast<int>(mParams.mElements[3]);

    typedef void (*OrientationFunction)(math::Mat43*,
                                        const math::Dir3::Packed*,
                                        const math::Dir3::Packed*);
    OrientationFunction orientFunction = &OrientIdentity;
    if (orientationMode == 1) {
        orientFunction = (velocity != nullptr) ? &OrientParentVelocity : &OrientIdentity;
    } else if (orientationMode == 2) {
        orientFunction = &OrientRandom;
    } else if (orientationMode == 3) {
        orientFunction = (velocity != nullptr && collisionNormal != nullptr)
                             ? &OrientCollisionReflect
                             : &OrientRandom;
    }

    const int flagsOffset = ioGroup->mPFD.GetOffset(apsPFDField_Flags);
    unsigned char* particle = iBegin;
    unsigned char* flags = iBegin + flagsOffset;
    while (particle != iEnd) {
        if ((*flags & 1u) != 0) {
            math::Dir3 position;
            position.v = _mm_setr_ps(
                reinterpret_cast<float*>(particle)[0],
                reinterpret_cast<float*>(particle)[1],
                reinterpret_cast<float*>(particle)[2], 0.0f);

            math::Mat43 orientationMatrix;
            orientFunction(
                &orientationMatrix,
                velocity ? reinterpret_cast<const math::Dir3::Packed*>(velocity) : nullptr,
                collisionNormal
                    ? reinterpret_cast<const math::Dir3::Packed*>(collisionNormal)
                    : nullptr);
            const apsQuaternion quaternion =
                apsMath::QuaternionFromMatrix(orientationMatrix);
            apsInternal::QueueSpawnedEffect(iEffect->mId, effectTemplate,
                                            g_effectTime, quaternion, position, -1.0f);
        }

        particle += stride;
        flags += stride;
        if (orientation != nullptr)
            orientation += stride;
        if (velocity != nullptr)
            velocity += stride;
        if (collisionNormal != nullptr)
            collisionNormal += stride;
    }
}

// ============================================================================
// Trajectory (spline)
// ============================================================================
apsTrajectoryAction::apsTrajectoryAction()
    : apsAction(4, 0, eAsync, 0xF000041u) {}
void         apsTrajectoryAction::Act(unsigned char*, unsigned char*, apsGroup*, apsEffect*, float, float) {}

// ea: 0x0080B140
void CheckSplineData(float* pData) {
    int index = 0;
    tlPrintf("---------------------------------------\n");

    unsigned int firstWord = *reinterpret_cast<unsigned int*>(pData);
    while (static_cast<int>(firstWord) != -1) {
        const unsigned int secondWord = *reinterpret_cast<unsigned int*>(pData + 1);
        if (static_cast<int>(secondWord) == -1)
            break;
        const unsigned int thirdWord = *reinterpret_cast<unsigned int*>(pData + 2);
        if (static_cast<int>(thirdWord) == -1)
            break;

        tlPrintf("%03d : %f, %f, %f [%08X %08X %08X]\n", index,
                 pData[0], pData[1], pData[2], firstWord, secondWord,
                 thirdWord);
        firstWord = *reinterpret_cast<unsigned int*>(pData + 3);
        pData += 3;
        ++index;
    }
    tlPrintf("---------------------------------------\n");
}

unsigned int apsTrajectoryAction::GetSplineInfo(float*& oPoints, float& oT,
                                                float& oScale,
                                                math::Dir3& oAxis,
                                                math::Dir3& oAxisRate) {
    float* points = oPoints;
    float remainingDistance = oT;
    const float originalDistance = oT;

    if (static_cast<int>(*reinterpret_cast<unsigned int*>(points)) != -1) {
        while (static_cast<int>(
                   *reinterpret_cast<unsigned int*>(points + 3)) != -1) {
            const math::Dir3 current(points[0], points[1], points[2]);
            const math::Dir3 next(points[3], points[4], points[5]);
            const __m128 delta = _mm_sub_ps(next.v, current.v);
            const __m128 deltaSquared = _mm_mul_ps(delta, delta);
            const float sectionLength = std::sqrt(
                deltaSquared.m128_f32[0] +
                (deltaSquared.m128_f32[1] + deltaSquared.m128_f32[2]));

            if (sectionLength > remainingDistance) {
                oPoints = points;
                oScale = sectionLength;
                oT = originalDistance;
                oAxis = current;
                oAxisRate = next;
                return 1;
            }

            const int nextNodeMarker =
                *reinterpret_cast<int*>(points + 3);
            remainingDistance -= sectionLength;
            points += 3;
            if (nextNodeMarker == -1)
                return 0;
        }
    }
    return 0;
}

// ============================================================================
// Env-collide (world raycast)
// ============================================================================
apsEnvCollideAction::apsEnvCollideAction()
    : apsAction(2, 0, eAsync, 0xF8004001u) {}
// ea: 0x0080B340
void apsEnvCollideAction::Act(unsigned char* iBegin, unsigned char* iEnd,
                              apsGroup* ioGroup, apsEffect* iEffect, float,
                              float) {
    if ((ioGroup->mPFD.mFields & 0x08000000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int flagsOffset = ioGroup->mPFD.mOffsets[27];

    if ((ioGroup->mPFD.mFields & 0x10000000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int previousPositionOffset = ioGroup->mPFD.mOffsets[28];

    if ((ioGroup->mPFD.mFields & 0x20000000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int lastCollisionNormalOffset = ioGroup->mPFD.mOffsets[29];

    if ((ioGroup->mPFD.mFields & 1u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int positionOffset = ioGroup->mPFD.mOffsets[0];

    if ((ioGroup->mPFD.mFields & 0x4000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();

    if ((ioGroup->mPFD.mFields & 0x40000000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int collisionResultIdOffset = ioGroup->mPFD.mOffsets[30];

    if ((ioGroup->mPFD.mFields & 0x80000000u) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field"))
        __debugbreak();
    const int raycastCountdownOffset = ioGroup->mPFD.mOffsets[31];

    const int stride = ioGroup->mPFD.mStride;
    for (unsigned char* particle = iBegin; particle != iEnd;
         particle += stride) {
        const float* previousPosition = reinterpret_cast<const float*>(
            particle + previousPositionOffset);
        const float* currentPosition = reinterpret_cast<const float*>(
            particle + positionOffset);
        const math::Dir3 previous(previousPosition[0], previousPosition[1],
                                  previousPosition[2]);
        const math::Dir3 current(currentPosition[0], currentPosition[1],
                                 currentPosition[2]);

        if (ioGroup->ParticleIsMarkedForRemoval(particle))
            continue;

        unsigned int& collisionResultId = *reinterpret_cast<unsigned int*>(
            particle + collisionResultIdOffset);
        if (collisionResultId != 0xFFFFFFFFu) {
            const apsEffect::RaycastResult& result =
                iEffect->GetRaycastResult(collisionResultId);
            if (result.t >= 0.0f) {
                float* position = reinterpret_cast<float*>(particle + positionOffset);
                position[0] = result.position.v.m128_f32[0];
                position[1] = result.position.v.m128_f32[1];
                position[2] = result.position.v.m128_f32[2];

                float* collisionNormal = reinterpret_cast<float*>(
                    particle + lastCollisionNormalOffset);
                collisionNormal[0] = result.normal.v.m128_f32[0];
                collisionNormal[1] = result.normal.v.m128_f32[1];
                collisionNormal[2] = result.normal.v.m128_f32[2];
                ioGroup->MarkParticleForRemoval(particle);
            }
            collisionResultId = 0xFFFFFFFFu;
        }

        unsigned int& raycastCountdown = *reinterpret_cast<unsigned int*>(
            particle + raycastCountdownOffset);
        if (raycastCountdown != 0) {
            --raycastCountdown;
            continue;
        }

        collisionResultId = iEffect->RequestRaycast(previous, current);
        if (collisionResultId == 0xFFFFFFFFu) {
            if (iEffect->mTemplate == 0 &&
                _tlAssert("c:/cod/code/tl/aeps/include\\apsEffect.h", 182,
                          "mTemplate", "null template"))
                __debugbreak();
            char warning[0x200];
            _snprintf(warning, sizeof(warning),
                      "AEPS warning: raycast request denied. template=%s",
                      iEffect->mTemplate->mName);
            tlWarning(warning);
        } else {
            raycastCountdown = apsEffect::GetRaycastCountdownMaxValue();
            *reinterpret_cast<unsigned char*>(particle + flagsOffset) |= 2u;
        }
    }
}

// ============================================================================
// File-scope global — gCheckSplineData (0x014CFF50 in .data)
// ea: 0x00BFA640 CheckSplineData(float*) references this.
// ============================================================================
int gCheckSplineData = 0;
