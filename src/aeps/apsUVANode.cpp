// ============================================================================
// apsUVANode.cpp — UVA node Render forwarder (1 func).
// Source: source/apsUVANode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsUVANode.o):
//   apsUVANode::Render @0x816350
// ============================================================================
#include "apsUVARenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// apsUVANode::GetRenderSortBuffer - ea: 0x00816370
apsRenderSort::Buffer* apsUVANode::GetRenderSortBuffer()
{
    return mRenderSortBuffer;
}

// apsUVARenderer::WidthFrames - ea: 0x00816380
float apsUVARenderer::WidthFrames() const
{
    return mWidthFrames;
}

// apsUVARenderer::MaxFrame - ea: 0x00816390
float apsUVARenderer::MaxFrame() const
{
    return mMaxFrame;
}

// apsUVARenderer::InvWidthFrames - ea: 0x008163A0
float apsUVARenderer::InvWidthFrames() const
{
    return mInvWidthFrames;
}

// apsUVARenderer::InvHeightFrames - ea: 0x008163B0
float apsUVARenderer::InvHeightFrames() const
{
    return mInvHeightFrames;
}

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
