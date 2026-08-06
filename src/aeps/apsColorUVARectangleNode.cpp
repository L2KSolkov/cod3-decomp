// ============================================================================
// apsColorUVARectangleNode.cpp — color UVA rect node Render forwarder (1).
// Source: source/apsColorUVARectangleNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsColorUVARectangleNode.o):
//   apsColorUVARectangleNode::Render @0x817710
// ============================================================================
#include "apsColorUVARectangleRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// ============================================================================
// apsColorUVARectangleNode::Render — forward to the color UVA rect renderer.
// ea: 0x817710
// ============================================================================
void apsColorUVARectangleNode::Render() {
    cNodeRenderer<ColorUVARectangleParticle, apsColorUVARectangleNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
