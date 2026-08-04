// ============================================================================
// apsVFC.cpp — view-frustum culling (AABB vs frustum plane tests).
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsVFC.cpp
//
// All three functions were verified against the IDA disassembly
// (aeps_xboxr:apsVFC.o). GetFrustumInfo reads the scene's clip planes
// through a byte-exact view of nglScene (+0x270), which keeps this object
// compilable before ngl_xboxr:ngl_scene.o is ported.
// ============================================================================
#include "apsVFC.h"

namespace {

// nglScene is a full render-object class (ngl_xboxr:ngl_scene.o, not yet
// ported). GetFrustumInfo only reads its ClipPlanes array, which IDA places
// at +0x270 as 6 x math::Vector4 (16 bytes each).
struct nglSceneClipPlanesView {
    unsigned char _pad[0x270];
    math::Vector4 ClipPlanes[6];
};

} // namespace

namespace VFC {

// ============================================================================
// VFC::GetFrustumInfo — copy the scene's clip planes (negated) into the
// cached FrustumInfo and precompute the per-axis corner pickers n[]/p[].
// ea: 0x802030
// ============================================================================
void GetFrustumInfo(FrustumInfo& oFrustumInfo, const nglScene* iScene) {
    const nglSceneClipPlanesView* scene = reinterpret_cast<const nglSceneClipPlanesView*>(iScene);

    for (int i = 0; i < 6; ++i) {
        oFrustumInfo.clipPlanesF[i][0] = 0.0f - scene->ClipPlanes[i].v.m128_f32[0];
        oFrustumInfo.clipPlanesF[i][1] = 0.0f - scene->ClipPlanes[i].v.m128_f32[1];
        oFrustumInfo.clipPlanesF[i][2] = 0.0f - scene->ClipPlanes[i].v.m128_f32[2];
        oFrustumInfo.clipPlanesF[i][3] = 0.0f - scene->ClipPlanes[i].v.m128_f32[3];

        oFrustumInfo.n[i][0] = oFrustumInfo.clipPlanesF[i][0] < 0.0f;
        oFrustumInfo.n[i][1] = oFrustumInfo.clipPlanesF[i][1] < 0.0f;
        oFrustumInfo.n[i][2] = oFrustumInfo.clipPlanesF[i][2] < 0.0f;

        oFrustumInfo.p[i][0] = 1 - oFrustumInfo.n[i][0];
        oFrustumInfo.p[i][1] = 1 - oFrustumInfo.n[i][1];
        oFrustumInfo.p[i][2] = 1 - oFrustumInfo.n[i][2];
    }
}

// ============================================================================
// VFC::AABBvsFrustum — classify an AABB against the cached frustum.
// Returns INSIDE_FRUSTUM if every negative corner is inside, INTERSECTS if
// some positive corner is in front of a plane, OUTSIDE otherwise.
// ea: 0x802490
// ============================================================================
eStatus AABBvsFrustum(const AABB& iAABB, const FrustumInfo& iFrustumInfo) {
    eStatus result = INSIDE_FRUSTUM;
    for (int i = 0; i < 6; ++i) {
        float negDist =
            iAABB.bounds[iFrustumInfo.n[i][0]][0] * iFrustumInfo.clipPlanesF[i][0] +
            iAABB.bounds[iFrustumInfo.n[i][1]][1] * iFrustumInfo.clipPlanesF[i][1] +
            iAABB.bounds[iFrustumInfo.n[i][2]][2] * iFrustumInfo.clipPlanesF[i][2] -
            iFrustumInfo.clipPlanesF[i][3];
        if (negDist > 0.0f)
            return OUTSIDE_FRUSTUM;

        float posDist =
            iAABB.bounds[iFrustumInfo.p[i][0]][0] * iFrustumInfo.clipPlanesF[i][0] +
            iAABB.bounds[iFrustumInfo.p[i][1]][1] * iFrustumInfo.clipPlanesF[i][1] +
            iAABB.bounds[iFrustumInfo.p[i][2]][2] * iFrustumInfo.clipPlanesF[i][2] -
            iFrustumInfo.clipPlanesF[i][3];
        if (posDist > 0.0f)
            result = INTERSECTS_FRUSTUM;
    }
    return result;
}

// ============================================================================
// VFC::AABBOutsideFrustum — true if the AABB is fully outside the frustum
// (any plane has its negative corner strictly in front).
// ea: 0x802560
// ============================================================================
bool AABBOutsideFrustum(const AABB& iAABB, const FrustumInfo& iFrustumInfo) {
    for (int i = 0; i < 6; ++i) {
        float dist =
            iAABB.bounds[iFrustumInfo.n[i][0]][0] * iFrustumInfo.clipPlanesF[i][0] +
            iAABB.bounds[iFrustumInfo.n[i][1]][1] * iFrustumInfo.clipPlanesF[i][1] +
            iAABB.bounds[iFrustumInfo.n[i][2]][2] * iFrustumInfo.clipPlanesF[i][2] -
            iFrustumInfo.clipPlanesF[i][3];
        if (dist > 0.0f)
            return true;
    }
    return false;
}

} // namespace VFC
