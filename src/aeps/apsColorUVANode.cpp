// ============================================================================
// apsColorUVANode.cpp — color UVA node Render forwarder (1 func).
// Source: source/apsColorUVANode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorUVANode.o):
//   apsColorUVANode::Render @0x81AFF0
// ============================================================================
#include "apsColorUVARenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// apsColorUVARender::GetVShader - ea: 0x0081B010
unsigned long apsColorUVARender::GetVShader()
{
    return VS[0];
}

// apsColorUVARenderPixel::GetPShader - ea: 0x0081B020
unsigned long* apsColorUVARenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsColorUVANode::Render — forward to the color UVA renderer.
// ea: 0x81AFF0
// ============================================================================
void apsColorUVANode::Render() {
    cNodeRenderer<ColorUVAParticle, apsColorUVANode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
