// ============================================================================
// apsUVANode.cpp — UVA node Render forwarder (1 func).
// Source: source/apsUVANode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsUVANode.o):
//   apsUVANode::Render @0x816350
// ============================================================================
#include "apsUVARenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// apsUVARender::GetVShader - ea: 0x008163C0
unsigned long apsUVARender::GetVShader()
{
    return VS[0];
}

// apsUVARenderPixel::GetPShader - ea: 0x008163D0
unsigned long* apsUVARenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsUVANode::Render — forward to the UVA node renderer.
// ea: 0x816350
// ============================================================================
void apsUVANode::Render() {
    cNodeRenderer<UVAParticle, apsUVANode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
