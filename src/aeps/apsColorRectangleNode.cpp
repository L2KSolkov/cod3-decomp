// ============================================================================
// apsColorRectangleNode.cpp — color rectangle node Render forwarder (1 func).
// Source: source/apsColorRectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorRectangleNode.o):
//   apsColorRectangleNode::Render @0x815150
// ============================================================================
#include "apsColorRectangleRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// ============================================================================
// apsColorRectangleNode::Render — forward to the color rectangle renderer.
// ea: 0x815150
// ============================================================================
void apsColorRectangleNode::Render() {
    cNodeRenderer<ColorRectangleParticle, apsColorRectangleNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
