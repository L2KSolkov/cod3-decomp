// ============================================================================
// apsBillboardNode.cpp — billboard node Render forwarder (1 func).
// Source: source/apsBillboardNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsBillboardNode.o):
//   apsBillboardNode::Render @0x813C90
// ============================================================================
#include "apsBillboardRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// apsBillboardRender::GetVShader - ea: 0x00813EB0
unsigned long apsBillboardRender::GetVShader()
{
    return VS[0];
}

// apsBillboardRenderPixel::GetPShader - ea: 0x00813EC0
unsigned long* apsBillboardRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsBillboardNode::Render — forward to the billboard node renderer.
// ea: 0x813C90
// ============================================================================
void apsBillboardNode::Render() {
    cNodeRenderer<BillboardParticle, apsBillboardNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
