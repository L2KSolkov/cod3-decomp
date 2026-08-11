// ============================================================================
// apsRenderer.cpp — particle renderer base (3 funcs).
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsRenderer.cpp
//
// Port strategy:
//   - nglBuildScene (data) and nglIsSphereVisible + tlFatal live in ngl.o /
//     core / render.o — declared extern, unresolved here.
//   - nglScene render-list fields accessed via a byte-exact view struct.
// ============================================================================
#include "apsRenderer.h"
#include "apsMath.h"

// ngl.o (data): the active build scene. Unresolved here.
extern nglScene* nglBuildScene;

// core / tl_system.cpp (non-inline): fatal error (varargs). Defined already.
extern void tlFatal(const char* iFormat, ...);

// Byte-exact view of the nglScene render-list region (offsets from AddNode
// disasm): ClipPlanes @0x270, OpaqueRenderList @0x314, TransRenderList @0x318,
// OpaqueListCount @0x31C, TransListCount @0x320.
struct nglSceneView {
    unsigned char  _pad0[0x270];
    math::Vector4  ClipPlanes[6];       // +0x270 (60h bytes)
    unsigned char  _pad1[0x314 - 0x2D0];
    nglRenderNode* OpaqueRenderList;    // +0x314
    nglRenderNode* TransRenderList;     // +0x318
    int            OpaqueListCount;     // +0x31C
    int            TransListCount;      // +0x320
};

// ============================================================================
// apsRenderer::AddNode — push a render node onto the build scene's opaque or
// translucent list depending on the sign of mPriority, and track the count.
// ea: 0x8120e0
// ============================================================================
void apsRenderer::AddNode(apsRenderNode* iNode, float iDist) {
    nglSceneView* scene = reinterpret_cast<nglSceneView*>(nglBuildScene);
    if (mPriority >= 0) {
        iNode->SortDist = iDist;
        iNode->Next = scene->TransRenderList;
        scene->TransRenderList = iNode;
        ++scene->TransListCount;
    } else {
        iNode->SortHash = -mPriority;
        iNode->Next = scene->OpaqueRenderList;
        scene->OpaqueRenderList = iNode;
        ++scene->OpaqueListCount;
    }
}

// ============================================================================
// apsRenderer::SphereIsVisible — sphere-vs-frustum test against the scene's
// clip planes (this is unused here; the caller is a collection helper).
// ea: 0x812160
// ============================================================================
unsigned int apsRenderer::SphereIsVisible(const apsSphere& iSphere, nglScene* iScene) {
    math::Position3 center;
    center.v.m128_f32[0] = iSphere.mSphere.v.m128_f32[0];
    center.v.m128_f32[1] = iSphere.mSphere.v.m128_f32[1];
    center.v.m128_f32[2] = iSphere.mSphere.v.m128_f32[2];
    center.v.m128_f32[3] = 0.0f;
    const nglSceneView* scene = reinterpret_cast<const nglSceneView*>(iScene);
    return nglIsSphereVisible((const nglFrustum*)scene->ClipPlanes,
                              (const math::Vector4*)&center,
                              iSphere.mSphere.v.m128_f32[3])
               ? 1
               : 0;
}

// ============================================================================
// apsRenderer::Fixup — locate the renderer's real vtable by matching the
// current vtable pointer against apsFixupParams.renderers[].id, then install
// it. tlFatal on "Didn't find vtable for renderer".
// ea: 0x8121f0
// ============================================================================
void apsRenderer::Fixup(const apsFixupParams& iFixupParams) {
    void* curVtbl = *reinterpret_cast<void**>(this);
    void* foundVtbl = 0;
    if (iFixupParams.numRenderers > 0) {
        const apsFixupParams::Lookup* renderers = iFixupParams.renderers;
        for (int i = 0; i < iFixupParams.numRenderers; ++i) {
            if (renderers[i].id == reinterpret_cast<unsigned int>(curVtbl)) {
                foundVtbl = reinterpret_cast<void*>(renderers[i].vtbl);
                break;
            }
        }
    }
    if (foundVtbl == 0)
        tlFatal("Didn't find vtable for renderer");
    *reinterpret_cast<void**>(this) = foundVtbl;
}
