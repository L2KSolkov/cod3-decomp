// ============================================================================
// apsRenderNode — render-list node with full transform state (176 bytes).
// Source: c:\cod\code\tl\aeps\source\apsRenderNode.cpp (inline COMDATs in
// apsSimpleMeshRenderer.o / apsShrimpRenderer.o).
// Verified against IDA local type apsRenderNode (size 0xB0).
// ============================================================================
#ifndef COD3_AEPS_APSRENDERNODE_H
#define COD3_AEPS_APSRENDERNODE_H

#include "apsMath.h"
#include "render/cdDebugVertexDef.h"
#include "core/tlFixedString.h"
#include "ngl/nglScene.h"

#include <intrin.h>

class nglRenderNode;
class apsRenderNode;

// ============================================================================
// nglRenderNode — base render-list node (12 bytes, from IDA local type):
//   vtable @0x00 (implicit), Next @0x04, SortDist/SortHash union @0x08
// ============================================================================
class nglRenderNode {
public:
    virtual ~nglRenderNode() {}
    nglRenderNode* Next;
    union {
        float SortDist;
        int   SortHash;
    };
};
static_assert(sizeof(nglRenderNode) == 0x0C, "nglRenderNode size mismatch");

// ============================================================================
// apsSphere — 16 bytes: center (xyz) + radius (w).
// ============================================================================
struct apsSphere {
    math::Vector4 mSphere;
};

// ============================================================================
// apsLight::LightInfo — per-node lighting info (48 bytes, verified against IDA)
// ============================================================================
namespace apsLight {
struct LightInfo {
    math::Vector4 m_dirToLight;   // +0x00
    math::Vector4 m_lightColor;   // +0x10
    math::Vector4 m_ambientColor; // +0x20
};
}

// ============================================================================
// apsRenderNode — render-list node (176 bytes).
// ============================================================================
class apsRenderNode : public nglRenderNode {
public:
    math::Mat43           mLocalToWorld;    // +0x10
    apsSphere             mSphere;          // +0x50
    math::Vector4         mBlendColor;      // +0x60
    unsigned char*        mParticles;       // +0x70
    int                   mNumParticles;    // +0x74
    int                   mStride;          // +0x78
    unsigned int          mFlags;           // +0x7C
    apsLight::LightInfo   mLightInfo;       // +0x80 (48 bytes)

    apsRenderNode() { mFlags = 0; }                       // ea: 0x802B60
    virtual ~apsRenderNode();                             // ea: 0x802C20
    virtual void Render();                                // ea: per-node (aps*Node.o)

    void SetFlags(unsigned int flag) { mFlags |= flag; }  // ea: 0x8026E0
    void SetMatrix(const math::Mat43* matrix);            // ea: 0x802700
    void SetSphere(const apsSphere* s) { mSphere.mSphere = s->mSphere; }  // ea: 0x8027A0
    void SetParticles(int num, unsigned char* ref, int stride);   // ea: 0x8027D0
    float GetDist(nglScene* Scene);                       // ea: 0x802630
};
static_assert(sizeof(apsRenderNode) == 0xB0, "apsRenderNode size mismatch");

// ============================================================================
// apsSphere helpers (inline COMDATs, apsSimpleMeshRenderer.o)
// ============================================================================
inline const math::Dir3::Packed& Center(const apsSphere& s) {
    return *(const math::Dir3::Packed*)&s.mSphere.v.m128_f32[0];
}
inline const float& Radius(const apsSphere& s) {
    return s.mSphere.v.m128_f32[3];
}

#endif // COD3_AEPS_APSRENDERNODE_H
