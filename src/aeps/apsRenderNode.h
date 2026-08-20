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
#include "ngl/nglRenderNode.h"

#include <intrin.h>

class apsRenderNode;

// ============================================================================
// apsSphere — 16 bytes: center (xyz) + radius (w).
// ============================================================================
struct apsSphere {
    math::Vector4 mSphere;

    apsSphere();
    apsSphere(const math::Dir3::Packed& center, float radius);
    void Set(const math::Dir3::Packed& center, float radius);
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
    virtual void Render() = 0;                            // pure slot in apsRenderNode_vtbl

    void SetFlags(unsigned int flag) { mFlags |= flag; }  // ea: 0x8026E0
    void SetMatrix(const math::Mat43* matrix);            // ea: 0x802700
    const math::Mat43& Matrix() const;                     // ea: 0x812CD0
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
