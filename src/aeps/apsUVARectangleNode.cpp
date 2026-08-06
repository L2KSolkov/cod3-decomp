// ============================================================================
// apsUVARectangleNode.cpp — UVA rectangle node Render forwarder (1 func).
// Source: source/apsUVARectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsUVARectangleNode.o):
//   apsUVARectangleNode::Render @0x8189C0
// ============================================================================
#include "apsUVARectangleRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// ============================================================================
// apsUVARectangleNode::Render — forward to the UVA rectangle renderer.
// ea: 0x8189C0
// ============================================================================
void apsUVARectangleNode::Render() {
    cNodeRenderer<UVARectangleParticle, apsUVARectangleNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
