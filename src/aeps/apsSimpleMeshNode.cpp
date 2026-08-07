// ============================================================================
// apsSimpleMeshNode.cpp — mesh particle render node (1 non-inline func).
// Source: source/apsSimpleMeshNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsSimpleMeshNode.o): Render @0x812530.
// NOTE: the real Render drives the Xbox D3D8 mesh pipeline directly
// (D3DDevice_SetRenderState_ParameterCheck / nglDxTexCache /
// D3DDevice_SetVertexShaderConstantNotInlineFast / nglGpuDrawSection /
// gpuSetVertexShader Inputs + apsSimpleMeshRender::VS/PS).  That GPU layer
// is not ported to Win32, so this is a structural stub: it keeps the symbol
// and the per-particle transform loop shape without issuing D3D calls.
// The renderer's Render() (apsSimpleMeshRenderer.o) still inserts the node.
// ============================================================================
#include "apsSimpleMeshRenderer.h"

extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// apsSimpleMeshNode::Render
// ea: 0x812530
// ============================================================================
void apsSimpleMeshNode::Render() {
    // Stub: the Xbox D3D8 mesh draw loop is not portable. Validate inputs
    // (mirroring the original asserts) but do not issue GPU commands.
    if (mRenderer == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsSimpleMeshNode.h", 16,
                  "mRenderer", "null renderer"))
        __debugbreak();

    // Original: per-particle transform build + nglGpuDrawSection. Skipped.
    (void)mNumParticles;
    (void)mParticles;
    (void)mStride;
}
