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

struct nglScene;
struct nglLightContext;
namespace apsLight { struct LightInfo; }
namespace VFC {
    struct FrustumInfo;
    // 24 bytes: min/max corner floats. Layout verified against IDA.
    struct AABB {
        float bounds[2][3];
    };
    // VFC.o (non-inline): true if the box is fully outside the frustum.
    bool AABBOutsideFrustum(const AABB& iAABB, const FrustumInfo& iFrustumInfo);
}
namespace nano { struct Dynamic_System; }

struct apsRendererRenderInfo;

// Minimal apsRenderer (full class in apsRenderer.h once that object is
// ported). The virtual layout is preserved so calls through a real engine
// renderer resolve to the right vtable slot (verified against the vftable):
//   slot 0  ~apsRenderer
//   slot 1  Render(apsRendererRenderInfo const&)   (call [vtable+4] in apsGroup.o)
//   slot 2  GetId
//   slot 3  GetVersion
//   slot 4  IsCameraFacing
//   slot 5  SetScreenFacingNormal
//   slot 6  GetChanceToRemove                       (call [vtable+0x18])
//   slot 7  GetMeshRadius
class apsRenderer {
public:
    enum eRenderResult {
        RENDERRESULT_NOT_VISIBLE = 0,
        RENDERRESULT_VISIBLE = 1,
        RENDERRESULT_NO_PARTICLES = 2,
        RENDERRESULT_NO_RENDERER = 3,
    };

    virtual ~apsRenderer();                                        // slot 0
    virtual eRenderResult Render(const apsRendererRenderInfo& iInfo) = 0;  // slot 1
    virtual unsigned int GetId() const = 0;                        // slot 2
    virtual float GetVersion() const = 0;                          // slot 3
    virtual int IsCameraFacing() const = 0;                        // slot 4
    virtual void SetScreenFacingNormal(const math::Dir3& iNormal) = 0;  // slot 5
    virtual float GetChanceToRemove() const = 0;                   // slot 6
    virtual bool GetMeshRadius(float& oRadius) const = 0;          // slot 7
};

struct apsSphere {
    math::Vector4 mSphere;
};

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
    void Grow(float iRadius) {
        __m128 splat = _mm_set1_ps(iRadius);
        mMin.v = _mm_sub_ps(mMin.v, splat);
        mMax.v = _mm_add_ps(mMax.v, splat);
    }
    // game2.o (non-inline): bounding-sphere radius = length(max-min) * 0.5.
    // Unresolved here; apsGroup.o calls it (via 0x41F650).
    float Radius() const;

    // game2.o (non-inline): bounding-sphere (center + radius). Unresolved here.
    apsSphere Sphere() const;
};

struct apsRendererRenderInfo {
    unsigned char*             particles;          // +0x00
    int                        numParticles;       // +0x04
    const apsPFD*              pfd;                // +0x08
    apsSphere                  sphere;             // +0x10
    const math::Mat43*         localToWorld;       // +0x20
    nglLightContext*           lightContext;       // +0x24
    float                      groupDist2Camera;   // +0x28
    const apsLight::LightInfo* lightInfo;          // +0x2C
    float                      maxParticleRadius;  // +0x30

    apsRendererRenderInfo() {
        particles = 0;
        numParticles = 0;
        pfd = 0;
        localToWorld = 0;
        lightContext = 0;
        lightInfo = 0;
    }
};

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

protected:
    void SetLocalSpace(unsigned int bLocalSpace) {
        mFlags = (mFlags & 0xFFFD) | (bLocalSpace ? 2 : 0);
    }

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
