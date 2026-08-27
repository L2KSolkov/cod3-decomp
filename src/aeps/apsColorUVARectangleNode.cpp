// ============================================================================
// apsColorUVARectangleNode.cpp — color UVA rect node Render forwarder (1).
// Source: source/apsColorUVARectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorUVARectangleNode.o):
//   apsColorUVARectangleNode::Render @0x817710
// ============================================================================
#include "apsColorUVARectangleRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// apsColorUVARectangleRender::GetVShader - ea: 0x00817730
unsigned long apsColorUVARectangleRender::GetVShader()
{
    return VS[0];
}

// apsColorUVARectangleRenderPixel::GetPShader - ea: 0x00817740
unsigned long* apsColorUVARectangleRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsColorUVARectangleNode::Render — forward to the color UVA rect renderer.
// ea: 0x817710
// ============================================================================
void apsColorUVARectangleNode::Render() {
    cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
