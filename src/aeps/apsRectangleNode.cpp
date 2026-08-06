// ============================================================================
// apsRectangleNode.cpp — rectangle node Render forwarder (1 func).
// Source: source/apsRectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsRectangleNode.o):
//   apsRectangleNode::Render @0x819D30
// ============================================================================
#include "apsRectangleRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// ============================================================================
// apsRectangleNode::Render — forward to the rectangle node renderer.
// ea: 0x819D30
// ============================================================================
void apsRectangleNode::Render() {
    cNodeRenderer<RectangleParticle, apsRectangleNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
