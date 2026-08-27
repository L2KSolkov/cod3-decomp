// ============================================================================
// apsColorRectangleNode.cpp — color rectangle node Render forwarder (1 func).
// Source: source/apsColorRectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorRectangleNode.o):
//   apsColorRectangleNode::Render @0x815150
// ============================================================================
#include "apsColorRectangleRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// apsColorRectangleRender::GetVShader - ea: 0x00815170
unsigned long apsColorRectangleRender::GetVShader()
{
    return VS[0];
}

// apsColorRectangleRenderPixel::GetPShader - ea: 0x00815180
unsigned long* apsColorRectangleRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsColorRectangleNode::Render — forward to the color rectangle renderer.
// ea: 0x815150
// ============================================================================
void apsColorRectangleNode::Render() {
    cNodeRenderer<ColorRectangleParticle, apsColorRectangleNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
