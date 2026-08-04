// ============================================================================
// apsVFC — view-frustum culling (AABB vs frustum plane tests).
// Source: c:\cod\code\tl\aeps\source\apsVFC.cpp
// All three functions verified against IDA (aeps_xboxr:apsVFC.o):
//   GetFrustumInfo     @0x802030
//   AABBvsFrustum      @0x802490
//   AABBOutsideFrustum @0x802560
// ============================================================================
#ifndef COD3_AEPS_APSVFC_H
#define COD3_AEPS_APSVFC_H

#include "core/math_types.h"

struct nglScene;

namespace VFC {

enum eStatus {
    INSIDE_FRUSTUM = 0,
    OUTSIDE_FRUSTUM = 1,
    INTERSECTS_FRUSTUM = 2,
};

// 240 bytes. Layout verified against GetFrustumInfo disasm:
//   n[6][3]            +0x00  per-plane per-axis "negative" corner picker (0/1)
//   p[6][3]            +0x48  per-plane per-axis "positive" corner picker (0/1)
//   clipPlanesF[6][4]  +0x90  negated clip planes (x, y, z, w)
struct FrustumInfo {
    int   n[6][3];
    int   p[6][3];
    float clipPlanesF[6][4];
};

// 24 bytes: min/max corner floats.
struct AABB {
    float bounds[2][3];
};

// VFC.o (non-inline). True if the box is fully outside the frustum.
bool AABBOutsideFrustum(const AABB& iAABB, const FrustumInfo& iFrustumInfo);

// VFC.o (non-inline). Full classification: inside / outside / intersects.
eStatus AABBvsFrustum(const AABB& iAABB, const FrustumInfo& iFrustumInfo);

// VFC.o (non-inline). Rebuild the per-frame frustum data from the scene's
// clip planes (nglScene::ClipPlanes @ +0x270).
void GetFrustumInfo(FrustumInfo& oFrustumInfo, const nglScene* iScene);

} // namespace VFC

#endif // COD3_AEPS_APSVFC_H
