// ============================================================================
// apsColorUVANode.cpp — color UVA node Render forwarder (1 func).
// Source: source/apsColorUVANode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorUVANode.o):
//   apsColorUVANode::Render @0x81AFF0
// ============================================================================
#include "apsColorUVARenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// ============================================================================
// apsColorUVANode::Render — forward to the color UVA renderer.
// ea: 0x81AFF0
// ============================================================================
void apsColorUVANode::Render() {
    cNodeRenderer<ColorUVAParticle, apsColorUVANode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
