// ============================================================================
// apsShrimpNode.cpp — shrimp particle render node (1 non-inline func).
// Source: source/apsShrimpNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsShrimpNode.o): Render @0x812D90.
// NOTE: the real Render drives the Xbox D3D8 sprite/quad pipeline directly.
// That GPU layer is not ported to Win32, so this is a structural stub: it
// keeps the symbol. The renderer's Render() (apsShrimpRenderer.o) still
// inserts the node.
// ============================================================================
#include "apsShrimpRenderer.h"

extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// apsShrimpNode::Render
// ea: 0x812D90
// ============================================================================
void apsShrimpNode::Render() {
    // Stub: the Xbox D3D8 shrimp draw loop is not portable. Skip GPU commands.
    if (mRenderer == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsShrimpNode.h", 14,
                  "mRenderer", "null renderer"))
        __debugbreak();

    (void)mNumParticles;
    (void)mParticles;
    (void)mStride;
}
