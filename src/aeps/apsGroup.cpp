// ============================================================================
// apsGroup.cpp — particle group (22 funcs)
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsGroup.cpp
//
// Port strategy (matches apsSuppliedActions.o precedent):
//   - Class ABI is real: full 0xE0 layout, vtables, ctor/dtor/Init.
//   - Functions owned by other aeps objects (apsPFD.o, apsCommon.o,
//     apsMemory.o, apsError.o) are declared and called; their symbols stay
//     unresolved until those objects are ported (/FORCE:UNRESOLVED).
//   - Statics and globals owned by apsGroup.o are defined here.
// ============================================================================
#include "apsGroup.h"
#include "apsMath.h"
#include "apsCommon.h"
#include "apsError.h"

#include <intrin.h>
#include <string.h>

#include <cfloat>

namespace apsMemory {
    void SetBlockAllocator();
    void ClearBlockAllocator();
}

// ============================================================================
// apsGroup.o owned data
// ============================================================================
int apsGroup::sNumActiveParticles = 0;
int apsGroup::sMaxActiveParticles = 0;
float apsGroup::mTimeDelta = 0.0f;
unsigned char* apsGroup::mLastBegin = 0;
unsigned char* apsGroup::mLastEnd = 0;
apsBounds apsGroup::mNewBounds;
int apsGroup::sGlobalRemovalListSize = 0;
unsigned char* apsGroup::sGlobalRemovalList[200];

int g_hackCallCount = 0;
int g_hackCurrentPakId = 0;

// ============================================================================
// HackTurnOffAepsMemory / HackTurnOnAepsMemory — switch aps memory to/from
// the pak-block allocator (used by the runtime memory debugger).
// ea: 0x7FEB00 / 0x7FEB60
// ============================================================================
void HackTurnOffAepsMemory() {
    if (g_hackCallCount != 0) {
        if (_tlAssert("source/apsGroup.cpp", 0x42, "g_hackCallCount==0",
                      "g_hackCallCount must be 0, no nesting of this call"))
            __debugbreak();
    }
    g_hackCallCount = 1;
    g_hackCurrentPakId = apsCommon::GetCurrentPakId();
    apsMemory::SetBlockAllocator();
    apsCommon::SetCurrentPakId(-1);
    apsCommon::SetPakAllocs(0);
}

void HackTurnOnAepsMemory() {
    apsCommon::SetCurrentPakId(g_hackCurrentPakId);
    apsCommon::SetPakAllocs(1);
    apsMemory::ClearBlockAllocator();
    g_hackCallCount = 0;
}

// ============================================================================
// apsGroup::apsGroup — zero-clear particle group
// ea: 0x7FF2B0
// ============================================================================
apsGroup::apsGroup() : mPFD() {
    mBounds.Init();
}

// ea: 0x7FF2D0
apsGroup::~apsGroup() {
}

// ============================================================================
// apsGroup::Term — free particle memory, subtract from active count
// ea: 0x7FF2E0
// ============================================================================
void apsGroup::Term(unsigned int bKillQuickly) {
    apsAllocator* allocator = apsCommon::GetAllocator();
    unsigned char* firstParticle = mFirstParticle;
    if (firstParticle != 0)
        allocator->MemFree(firstParticle);
    int numParticles = mNumParticles;
    if (numParticles != 0)
        sNumActiveParticles -= numParticles;
}

// ============================================================================
// apsGroup::Init — (re)initialize the group's particle buffer
// ea: 0x7FFC80
// ============================================================================
unsigned int apsGroup::Init(int iMaxNumParticles, apsRenderer* iRenderer,
                            const apsPFD& iPFD, unsigned int iLocalSpace) {
    mRenderer = iRenderer;
    mEmissionSpillover = 0.0f;
    mPFD = iPFD;
    mMaxNumParticles = iMaxNumParticles;
    mNumParticles = 0;
    mMaxUsedParticles = 0;
    mLastPos.v.m128_f32[0] = 0.0f;
    mLastPos.v.m128_f32[1] = 0.0f;
    mBounds.Init();
    int bufferBytes = mMaxNumParticles * mPFD.mStride;
    mGroupIndex = -1;
    mNanoDynamicSystem = 0;
    mJustCreatedNanoDynamicSystem = false;
    mFlags = (iLocalSpace != 0 ? 2 : 0) | 1;

    apsAllocator* allocator = apsCommon::GetAllocator();
    unsigned char* firstParticle =
        static_cast<unsigned char*>(allocator->MemAlign(bufferBytes, 128));
    mFirstParticle = firstParticle;

    if (firstParticle != 0) {
        if (!(mPFD.mFields & 1) &&
            _tlAssert("source/apsGroup.cpp", 658,
                      "mPFD.HasField(apsPFDField_Position)",
                      "Particle must have a position"))
            __debugbreak();
        if (!(mPFD.mFields & 0x200) &&
            _tlAssert("source/apsGroup.cpp", 659,
                      "mPFD.HasField(apsPFDField_Age)",
                      "Particle must have an age"))
            __debugbreak();
        apsMath::SetIdentityMatrix(mLocalToWorld);
        mUpdateTime = 0.0f;
        mUpdateDelay = 0.0f;
        mBoundSphereDistanceFromCamera = 0.0f;
        return 1;
    }

    // Inlined apsError::Instance() — asserts sInstancePtr (apsUtil.h:109),
    // then calls AddError on the singleton.
    apsError::Instance().AddError(
        apsError::ERROR_TYPE_WARNING, "apsGroup : alloc failed for %d bytes", bufferBytes);
    return 0;
}

// ============================================================================
// apsGroup::StartBatches — begin a particle-batching pass
// ea: 0x7FF310
// ============================================================================
unsigned int apsGroup::StartBatches(unsigned char*& oBegin, unsigned char*& oEnd,
                                    float iTimeDelta) {
    if (mFirstParticle == 0)
        return 0;
    mTimeDelta = iTimeDelta;
    oBegin = mFirstParticle;
    mLastBegin = mFirstParticle;
    oEnd = &oBegin[mPFD.mStride * mNumParticles];
    mLastEnd = oEnd;
    mNewBounds.Init();
    mNewMaxParticleRadius = -3.402823466e+38f;
    sGlobalRemovalListSize = 0;
    return 1;
}

// ============================================================================
// apsGroup::NextBatch — recompute per-frame bounds over the finished batch
// ea: 0x7FF3C0
// ============================================================================
void apsGroup::NextBatch(unsigned char*& oBegin, unsigned char*& oEnd) {
    if (mFirstParticle == 0)
        return;
    int stride = mPFD.mStride;
    unsigned char* p = mLastBegin;
    unsigned char* end = mLastEnd;
    float* age = reinterpret_cast<float*>(p + mPFD.GetOffset(apsPFDField_Age));

    if (mFlags & 2) {
        // local-space particles: extent = sqrt(2) * radius
        float* radius = reinterpret_cast<float*>(p + mPFD.GetOffset(apsPFDField_Radius));
        while (p != end) {
            *age += mTimeDelta;
            float r = *radius * 1.4142135f;
            __m128 splat = _mm_set1_ps(r);
            __m128 pos = _mm_setr_ps(p[0], p[1], p[2], 0.0f);
            mNewBounds.mMin.v =
                _mm_min_ps(_mm_min_ps(mNewBounds.mMin.v, _mm_add_ps(pos, splat)),
                           _mm_sub_ps(pos, splat));
            mNewBounds.mMax.v =
                _mm_max_ps(_mm_max_ps(mNewBounds.mMax.v, _mm_add_ps(pos, splat)),
                           _mm_sub_ps(pos, splat));
            if (r > mNewMaxParticleRadius)
                mNewMaxParticleRadius = r;
            p += stride;
            age = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(age) + stride);
            radius = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(radius) + stride);
        }
    } else if (mFlags & 4) {
        // width/height particles: extent = width*0.5+height (or height*0.5+width)
        float* width = reinterpret_cast<float*>(p + mPFD.GetOffset(apsPFDField_Width));
        float* height = reinterpret_cast<float*>(p + mPFD.GetOffset(apsPFDField_Height));
        while (p != end) {
            *age += mTimeDelta;
            float w = *width;
            float h = *height;
            float half = (w <= h) ? (w * 0.5f + h) : (h * 0.5f + w);
            __m128 splat = _mm_set1_ps(half);
            __m128 pos = _mm_setr_ps(p[0], p[1], p[2], 0.0f);
            mNewBounds.mMin.v = _mm_min_ps(mNewBounds.mMin.v, _mm_sub_ps(pos, splat));
            mNewBounds.mMax.v = _mm_max_ps(mNewBounds.mMax.v, _mm_add_ps(pos, splat));
            if (half > mNewMaxParticleRadius)
                mNewMaxParticleRadius = half;
            p += stride;
            age = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(age) + stride);
            width = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(width) + stride);
            height = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(height) + stride);
        }
    } else {
        // point particles: unit extent
        __m128 ones = _mm_set1_ps(1.0f);
        while (p != end) {
            *age += mTimeDelta;
            __m128 pos = _mm_setr_ps(p[0], p[1], p[2], 0.0f);
            mNewBounds.mMin.v = _mm_min_ps(mNewBounds.mMin.v, _mm_sub_ps(pos, ones));
            mNewBounds.mMax.v = _mm_max_ps(mNewBounds.mMax.v, _mm_add_ps(pos, ones));
            p += stride;
            age = reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(age) + stride);
        }
        if (mNewMaxParticleRadius < 20.0f)
            mNewMaxParticleRadius = 20.0f;
    }

    oEnd = 0;
    oBegin = 0;
    mLastEnd = 0;
    mLastBegin = 0;
}

// ============================================================================
// apsGroup::EndBatches — commit per-frame bounds to the group
// ea: 0x7FF7D0
// ============================================================================
void apsGroup::EndBatches() {
    if (mFirstParticle == 0)
        return;
    apsBounds bounds;
    if (mNumParticles <= 0) {
        math::Dir3 pos;
        pos.v = mLocalToWorld.w.v;
        bounds = apsBounds(pos, pos);
    } else {
        bounds = mNewBounds;
    }
    mBounds = bounds;
    mMaxParticleRadius = mNewMaxParticleRadius;
    if (mFlags & 2)
        transform_bounds();
    FlushGlobalRemovalList();
}

// ============================================================================
// apsGroup::Add — append a particle to the group's particle buffer
// ea: 0x7FF870
// ============================================================================
void apsGroup::Add(unsigned char* iParticle) {
    if (mFirstParticle == 0)
        return;
    if (mNumParticles >= mMaxNumParticles)
        return;
    memcpy(&mFirstParticle[mNumParticles * mPFD.mStride], iParticle, mPFD.mStride);

    math::Dir3 pos;
    pos.v.m128_f32[0] = iParticle[0];
    pos.v.m128_f32[1] = iParticle[1];
    pos.v.m128_f32[2] = iParticle[2];
    pos.v.m128_f32[3] = 0.0f;
    apsBounds particleBounds(pos, pos);
    if (mPFD.mFields & 2) {
        float radius = *reinterpret_cast<float*>(&iParticle[mPFD.GetOffset(apsPFDField_Radius)]);
        particleBounds.Grow(radius);
    } else {
        particleBounds.Grow(1.0f);
    }
    mBounds.mMin.v = _mm_min_ps(mBounds.mMin.v, particleBounds.mMin.v);
    mBounds.mMax.v = _mm_max_ps(mBounds.mMax.v, particleBounds.mMax.v);

    int numParticles = mNumParticles + 1;
    mNumParticles = numParticles;
    if (numParticles > mMaxUsedParticles)
        mMaxUsedParticles = numParticles;
    int active = ++sNumActiveParticles;
    if (sNumActiveParticles > sMaxActiveParticles)
        sMaxActiveParticles = active;
}

// ============================================================================
// apsGroup::Remove — queue a particle for removal (processed at EndBatches)
// ea: 0x7FF9D0
// ============================================================================
void apsGroup::Remove(unsigned char* iParticle) const {
    if (mFirstParticle != 0)
        AddToGlobalRemovalList(iParticle);
}

// ============================================================================
// apsGroup::transform_bounds — world-space bounds for local-space groups:
// box-transform the 8 AABB corners through mLocalToWorld, re-min/max.
// ea: 0x7FECF0
// ============================================================================
void apsGroup::transform_bounds() {
    math::Dir3 points[8];
    apsMath::GetBoxPoints(mBounds, points);
    apsBounds newBounds;
    newBounds.Init();
    for (int i = 0; i < 8; i++)
        newBounds.Accumulate(apsMath::XForm3d_1(mLocalToWorld, points[i]));
    mBounds = newBounds;
}

// ============================================================================
// apsGroup::SetGroupLocalToWorldTransform — set the group's world transform;
// remember the previous translation in mLastPos.
// ea: 0x7FEDB0
// ============================================================================
void apsGroup::SetGroupLocalToWorldTransform(const math::Mat43& iMatrix) {
    math::Dir3 lastPos = mLastPos;
    if (lastPos.v.m128_f32[0] != 0.0f ||
        lastPos.v.m128_f32[1] != 0.0f ||
        lastPos.v.m128_f32[2] != 0.0f)
        mLastPos.v = mLocalToWorld.w.v;
    else
        mLastPos.v = iMatrix.w.v;
    mLocalToWorld = iMatrix;
}

// ============================================================================
// apsGroup::SetGroupPosition — update just the translation column.
// ea: 0x7FEEF0
// ============================================================================
void apsGroup::SetGroupPosition(const math::Dir3& pos) {
    math::Dir3 lastPos = mLastPos;
    if (lastPos.v.m128_f32[0] != 0.0f ||
        lastPos.v.m128_f32[1] != 0.0f ||
        lastPos.v.m128_f32[2] != 0.0f)
        mLastPos.v = mLocalToWorld.w.v;
    else
        mLastPos.v = pos.v;
    mLocalToWorld.w.v.m128_f32[0] = pos.v.m128_f32[0];
    mLocalToWorld.w.v.m128_f32[1] = pos.v.m128_f32[1];
    mLocalToWorld.w.v.m128_f32[2] = pos.v.m128_f32[2];
}

// ============================================================================
// apsGroup::Render — frustum-cull the group, fill the renderer info struct,
// then dispatch to the renderer's virtual Render.
// ea: 0x7FEB90
// ============================================================================
apsRenderer::eRenderResult apsGroup::Render(const apsLight::LightInfo& iLightInfo,
                                            const VFC::FrustumInfo& iFrustumInfo,
                                            nglLightContext* iLightContext) {
    if (mRenderer == 0)
        return apsRenderer::RENDERRESULT_NO_RENDERER;
    if (mNumParticles == 0)
        return apsRenderer::RENDERRESULT_NO_PARTICLES;
    if (!(mFlags & 1) || (mFlags & 4))
        return apsRenderer::RENDERRESULT_NO_PARTICLES;

    VFC::AABB groupAABB;
    groupAABB.bounds[0][0] = mBounds.mMin.v.m128_f32[0];
    groupAABB.bounds[0][1] = mBounds.mMin.v.m128_f32[1];
    groupAABB.bounds[0][2] = mBounds.mMin.v.m128_f32[2];
    groupAABB.bounds[1][0] = mBounds.mMax.v.m128_f32[0];
    groupAABB.bounds[1][1] = mBounds.mMax.v.m128_f32[1];
    groupAABB.bounds[1][2] = mBounds.mMax.v.m128_f32[2];
    if (VFC::AABBOutsideFrustum(groupAABB, iFrustumInfo))
        return apsRenderer::RENDERRESULT_NOT_VISIBLE;

    apsRendererRenderInfo renderInfo;
    renderInfo.particles = mFirstParticle;
    renderInfo.numParticles = mNumParticles;
    renderInfo.pfd = &mPFD;
    renderInfo.sphere = mBounds.Sphere();
    renderInfo.localToWorld = (mFlags & 2) ? &mLocalToWorld : 0;
    renderInfo.lightContext = iLightContext;
    renderInfo.groupDist2Camera = mRootDistanceFromCamera;
    renderInfo.lightInfo = &iLightInfo;
    renderInfo.maxParticleRadius = mMaxParticleRadius;
    return mRenderer->Render(renderInfo);
}

// ============================================================================
// apsGroup::SavePositionsIfNecessary — for particles whose Flags bit 1 is set,
// copy the current position into the PreviousSavedPosition field.
// ea: 0x7FEFE0
// ============================================================================
void apsGroup::SavePositionsIfNecessary(unsigned char* oBegin, unsigned char* oEnd) {
    if (!(mPFD.mFields & 0x10000000) &&
        _tlAssert("source/apsGroup.cpp", 254,
                  "mPFD.HasField(apsPFDField_PreviousSavedPosition)",
                  "Must have apsPFDField_PreviousSavedPosition to save to it!"))
        __debugbreak();
    if (!(mPFD.mFields & 0x8000000) &&
        _tlAssert("source/apsGroup.cpp", 255,
                  "mPFD.HasField(apsPFDField_Flags)",
                  "Must have apsPFDField_Flags to save to it!"))
        __debugbreak();
    int posOffset = mPFD.GetOffset(apsPFDField_Position);
    int prePosOffset = mPFD.GetOffset(apsPFDField_PreviousSavedPosition);
    int flagsOffset = mPFD.GetOffset(apsPFDField_Flags);
    int stride = mPFD.mStride;
    if (apsPFD::GetFieldByteSize(apsPFDField_PreviousSavedPosition) !=
            apsPFD::GetFieldByteSize(apsPFDField_Position) &&
        _tlAssert("source/apsGroup.cpp", 262,
                  "mPFD.GetFieldByteSize(apsPFDField_PreviousSavedPosition)==mPFD.GetFieldByteSize(apsPFDField_Position)",
                  "byte size must be same for pfd prePos and pos"))
        __debugbreak();
    int copySize = apsPFD::GetFieldByteSize(apsPFDField_PreviousSavedPosition);
    unsigned char* p = oBegin;
    while (p < oEnd) {
        unsigned char* flags = p + flagsOffset;
        if (*flags & 2) {
            memcpy(p + prePosOffset, p + posOffset, copySize);
            *flags &= ~2u;
        }
        p += stride;
    }
}

// ============================================================================
// apsGroup::ParticleIsMarkedForRemoval — Flags bit 0 set?
// ea: 0x7FF180
// ============================================================================
bool apsGroup::ParticleIsMarkedForRemoval(unsigned char* iParticle) const {
    if (mPFD.mFields & 0x8000000)
        return iParticle[mPFD.GetOffset(apsPFDField_Flags)] & 1;
    return false;
}

// ============================================================================
// apsGroup::AddToGlobalRemovalList — append to the shared removal queue.
// ea: 0x7FF1B0
// ============================================================================
void apsGroup::AddToGlobalRemovalList(unsigned char* iParticle) const {
    if (sGlobalRemovalListSize == 200) {
        if (_tlAssert("source/apsGroup.cpp", 424, "0",
                      "need to increase kAPS_MAX_PARTICLES_TO_REMOVE"))
            __debugbreak();
        sGlobalRemovalList[0] = iParticle;
        sGlobalRemovalListSize = 1;
    } else {
        sGlobalRemovalList[sGlobalRemovalListSize] = iParticle;
        sGlobalRemovalListSize = sGlobalRemovalListSize + 1;
    }
}

// ============================================================================
// apsGroup::FlushGlobalRemovalList — compact removed particles out of the
// buffer (each removal slot receives the current last particle).
// ea: 0x7FF210
// ============================================================================
void apsGroup::FlushGlobalRemovalList() {
    int listSize = sGlobalRemovalListSize;
    if (listSize != 0) {
        sGlobalRemovalListSize = 0;
        int numParticles = mNumParticles;
        unsigned int stride = mPFD.mStride;
        mNumParticles = numParticles - listSize;
        sNumActiveParticles -= listSize;
        unsigned char* p = &mFirstParticle[stride * (numParticles - 1)];
        unsigned char** rp = &sGlobalRemovalList[listSize - 1];
        int remaining = listSize;
        do {
            unsigned char* item = *rp;
            *rp-- = 0;
            if (p != item)
                memcpy(item, p, stride);
            p -= stride;
            --remaining;
        } while (remaining != 0);
        // The original ends with one extra store to the slot before the array
        // (lands on a zeroed static at 0x10DF60C in the original image). That
        // is an out-of-bounds write in our layout, so it is omitted here.
    }
}

// ============================================================================
// apsGroup::MarkParticleForRemoval — set Flags bit 0 and queue for removal.
// ea: 0x7FFC30
// ============================================================================
void apsGroup::MarkParticleForRemoval(unsigned char* iParticle) const {
    if (mPFD.mFields & 0x8000000) {
        int offset = mPFD.GetOffset(apsPFDField_Flags);
        if (!(iParticle[offset] & 1))
            iParticle[offset] |= 1;
    }
    if (mFirstParticle != 0)
        AddToGlobalRemovalList(iParticle);
}

// ============================================================================
// apsGroup::TestVisibility — compute bounds-center distance to the player and
// test the bounding sphere against the viewport clip planes.
// ea: 0x7FF9F0
// ============================================================================
void apsGroup::TestVisibility() {
    if (apsCommon::mBuildScene == 0) {
        mFlags = mFlags | 8;
        mBoundSphereDistanceFromCamera = 0.0f;
        return;
    }
    math::Position3 center;
    center.v = _mm_mul_ps(_mm_add_ps(mBounds.mMax.v, mBounds.mMin.v),
                          _mm_set1_ps(0.5f));
    float smallDist = mBounds.Radius();
    mFlags = mFlags & ~8u;

    apsCommon::PlayerViewPort* viewPort = apsCommon::GetPlayerViewPort(0);
    if (viewPort->mActive) {
        math::Position3 delta;
        delta.v = _mm_sub_ps(center.v, viewPort->mViewPos.v);
        float dist = math::Length(delta) - smallDist;
        if (viewPort->mProjectionX > 0.0f)
            dist /= viewPort->mProjectionX;
        if (dist < 0.0f)
            dist = 0.0f;
        if (nglIsSphereVisible(center, smallDist, viewPort->mClipPlanes)) {
            mFlags = mFlags | 8;
            if (dist < FLT_MAX) {
                mBoundSphereDistanceFromCamera = dist;
                math::Position3 rootDelta;
                rootDelta.v = _mm_sub_ps(mLocalToWorld.w.v, viewPort->mViewPos.v);
                mRootDistanceFromCamera = math::Length(rootDelta);
            }
        }
    }
}

// ============================================================================
// apsGroup::ApplyChanceToRemove — probabilistic per-particle removal based on
// age; fading particles (Flags bit 2) are skipped.
// ea: 0x7FFE00
// ============================================================================
void apsGroup::ApplyChanceToRemove(float chanceToRemove, unsigned char* oBegin,
                                   unsigned char* oEnd, int& numTotal,
                                   int& numRemoved) {
    numTotal = 0;
    numRemoved = 0;
    float fchance = mRenderer->GetChanceToRemove() * chanceToRemove;
    if (fchance > 0.0f) {
        const unsigned int fields = mPFD.mFields;
        if ((fields & 0x8000000) && (fields & 0x10) && (fields & 0x8000) &&
            (fields & 0x200) && (fields & 0x400) && (fields & 1)) {
            int ageOffset = mPFD.GetOffset(apsPFDField_Age);
            int maxAgeOffset = mPFD.GetOffset(apsPFDField_MaxAge);
            mPFD.GetOffset(apsPFDField_Position);
            int flagsOffset = mPFD.GetOffset(apsPFDField_Flags);
            int alphaOffset = mPFD.GetOffset(apsPFDField_Alpha);
            int maxAlphaOffset = mPFD.GetOffset(apsPFDField_MaxAlpha);
            int stride = mPFD.mStride;
            unsigned char* p = oBegin;
            if (p < oEnd) {
                do {
                    unsigned char* flags = p + flagsOffset;
                    float age = *reinterpret_cast<float*>(p + ageOffset);
                    float maxAge = *reinterpret_cast<float*>(p + maxAgeOffset);
                    float ageRatio = age / maxAge;
                    if (ageRatio >= 0.3f && (*flags & 5) == 0) {
                        float oneMinus = 1.0f - fchance;
                        if (oneMinus < 0.0f)
                            oneMinus = 0.0f;
                        if (oneMinus > 1.0f)
                            oneMinus = 1.0f;
                        float fadeOutDuration = oneMinus * 0.3f + 0.2f;
                        if (maxAge - age > fadeOutDuration) {
                            float r = apsMath::FloatRand(0.0f, 1.0f);
                            if (fchance > r * ageRatio) {
                                *reinterpret_cast<float*>(p + maxAlphaOffset) =
                                    *reinterpret_cast<float*>(p + alphaOffset) / fadeOutDuration;
                                *flags |= 4;
                                ++numRemoved;
                            }
                        }
                    }
                    ++numTotal;
                    p += stride;
                } while (p < oEnd);
            }
        }
    }
}
