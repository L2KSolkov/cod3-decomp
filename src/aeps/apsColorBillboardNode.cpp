// ============================================================================
// apsColorBillboardNode.cpp — color billboard node Render forwarder (1 func).
// Source: source/apsColorBillboardNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorBillboardNode.o):
//   apsColorBillboardNode::Render @0x81C2A0
// ============================================================================
#include "apsColorBillboardRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// apsColorBillboardRender::GetVShader - ea: 0x0081C2C0
unsigned long apsColorBillboardRender::GetVShader()
{
    return VS[0];
}

// apsColorBillboardRenderPixel::GetPShader - ea: 0x0081C2D0
unsigned long* apsColorBillboardRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsColorBillboardNode::Render — forward to the color billboard renderer.
// ea: 0x81C2A0
// ============================================================================
void apsColorBillboardNode::Render() {
    cNodeRenderer<ColorBillboardParticle, apsColorBillboardNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
