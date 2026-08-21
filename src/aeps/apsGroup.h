// ============================================================================
// apsGroup — particle group: the per-effect instance of a particle field
// descriptor (apsPFD), backing particle storage, and renderer hook.
// Source: c:\cod\code\tl\aeps\source\apsGroup.cpp
// Class size: 0xE0 (224 bytes) — verified against IDA.
// Inline accessors emitted in apsGroup.o:
//   IsLocalSpace @0x7FEA80, RenderingEnabled @0x7FEAA0,
//   SetLocalSpace @0x7FEAB0, GetParticles @0x7FEAE0,
//   GetGroupLocalToWorldTransform @0x7FEA90
// ============================================================================
#ifndef COD3_AEPS_APSGROUP_H
#define COD3_AEPS_APSGROUP_H

#include "core/math_types.h"

#include "apsCommon.h"
#include "apsPFD.h"
#include "apsVFC.h"
#include "apsRenderer.h"

struct nglScene;
struct nglLightContext;
namespace apsLight { struct LightInfo; }
namespace nano { struct Dynamic_System; }

struct apsBounds {
    math::Dir3 mMin;
    math::Dir3 mMax;

    apsBounds() {}
    apsBounds(const math::Dir3& iMin, const math::Dir3& iMax) {
        mMin = iMin;
        mMax = iMax;
    }
    // reset to "empty" bounds (min = +FLT_MAX, max = -FLT_MAX)
    void Init() {
        mMin.v = _mm_set_ps(3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f);
        mMax.v = _mm_xor_ps(mMin.v, _mm_set_ps(-0.0f, -0.0f, -0.0f, -0.0f));
    }
    void Accumulate(const math::Dir3& iParticle) {
        mMin.v = _mm_min_ps(mMin.v, iParticle.v);
        mMax.v = _mm_max_ps(mMax.v, iParticle.v);
    }
    void Accumulate(const apsBounds& iBounds) {   // ?Accumulate@apsBounds@@QAEXABU1@@Z (apsEffect.o COMDAT)
        mMin.v = _mm_min_ps(mMin.v, iBounds.mMin.v);
        mMax.v = _mm_max_ps(mMax.v, iBounds.mMax.v);
    }
    void Grow(float iRadius) {
        __m128 splat = _mm_set1_ps(iRadius);
        mMin.v = _mm_sub_ps(mMin.v, splat);
        mMax.v = _mm_add_ps(mMax.v, splat);
    }
    // game2.o (non-inline): bounding-sphere radius = length(max-min) * 0.5.
    float Radius() const
    {
        __m128 d = _mm_sub_ps(mMax.v, mMin.v);
        float dx = d.m128_f32[0], dy = d.m128_f32[1], dz = d.m128_f32[2];
        return sqrtf(dx * dx + dy * dy + dz * dz) * 0.5f;
    }

    // apsMath.o (non-inline): bounding-box extent = max - min.
    math::Dir3 Size() const;

    // game2.o (non-inline): bounding-sphere (center + radius). Unresolved here.
    apsSphere Sphere() const;
};

struct apsStats;

class apsGroup {
public:
    // ---- particle group state ----
    math::Mat43           mLocalToWorld;                    // +0x00
    math::Dir3            mLastPos;                         // +0x40
    apsBounds             mBounds;                          // +0x50
    float                 mEmissionSpillover;               // +0x70
    apsPFD                mPFD;                             // +0x74
    int                   mMaxNumParticles;                 // +0x9C
    short                 mGroupIndex;                      // +0xA0
    unsigned short        mFlags;                           // +0xA2
    float                 mUpdateTime;                      // +0xA4
    float                 mUpdateDelay;                     // +0xA8
    float                 mBoundSphereDistanceFromCamera;   // +0xAC
    float                 mRootDistanceFromCamera;          // +0xB0
    float                 mNewMaxParticleRadius;            // +0xB4
    float                 mMaxParticleRadius;               // +0xB8
    unsigned char*        mFirstParticle;                   // +0xBC
    int                   mNumParticles;                    // +0xC0
    int                   mMaxUsedParticles;                // +0xC4
    apsRenderer*          mRenderer;                        // +0xC8
    nano::Dynamic_System* mNanoDynamicSystem;               // +0xCC
    bool                  mJustCreatedNanoDynamicSystem;    // +0xD0

    // ---- lifecycle ----
    apsGroup();
    ~apsGroup();
    unsigned int Init(int iMaxNumParticles, apsRenderer* iRenderer,
                      const apsPFD& iPFD, unsigned int iGroupIndex);
    void Term(unsigned int iDeleteParticles);

    // ---- transform / bounds ----
    void SetGroupLocalToWorldTransform(const math::Mat43& iLocalToWorld);
    void SetGroupPosition(const math::Dir3& iPosition);
    void SavePositionsIfNecessary(unsigned char* iBegin, unsigned char* iEnd);

    // ---- particle management ----
    void Add(unsigned char* iParticle);
    void ApplyChanceToRemove(float iChance, unsigned char* ioBegin,
                             unsigned char* ioEnd, int& oNumRemoved, int& oNumAlive);
    bool ParticleIsMarkedForRemoval(unsigned char* iParticle) const;
    void MarkParticleForRemoval(unsigned char* iParticle) const;

    // ---- batching ----
    unsigned int StartBatches(unsigned char*& ioParticle, unsigned char*& ioLast,
                              float iTimeDelta);
    void NextBatch(unsigned char*& ioParticle, unsigned char*& ioLast);
    void EndBatches();
    void TestVisibility();
    apsRenderer::eRenderResult Render(const apsLight::LightInfo& iLightInfo,
                                      const VFC::FrustumInfo& iFrustumInfo,
                                      nglLightContext* iLightContext);

    // ---- inline accessors ----
    unsigned int RenderingEnabled() const { return mFlags & 1; }
    unsigned int IsLocalSpace() const { return mFlags & 2; }
    const math::Mat43& GetGroupLocalToWorldTransform() const { return mLocalToWorld; }
    unsigned char* GetParticles() const { return mFirstParticle; }
    const apsBounds& GetBounds() const; // game2.o 0x5188D0

protected:
    void SetLocalSpace(unsigned int bLocalSpace) {
        mFlags = (mFlags & 0xFFFD) | (bLocalSpace ? 2 : 0);
    }

    friend void apsGetStats(apsStats& stats);

private:
    void Remove(unsigned char* iParticle) const;
    void AddToGlobalRemovalList(unsigned char* iParticle) const;
    void FlushGlobalRemovalList();
    void transform_bounds();

    // inline @0x7FFFC0: reset the global removal list
    void InitGlobalRemovalList() { sGlobalRemovalListSize = 0; }

    static int             sNumActiveParticles;
    static int             sMaxActiveParticles;
    static float           mTimeDelta;
    static unsigned char*  mLastBegin;
    static unsigned char*  mLastEnd;
    static apsBounds       mNewBounds;
    static int             sGlobalRemovalListSize;
    static unsigned char*  sGlobalRemovalList[200];
};

#endif // COD3_AEPS_APSGROUP_H
