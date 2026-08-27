// ============================================================================
// apsBillboardNode.cpp — billboard node Render forwarder (1 func).
// Source: source/apsBillboardNode.cpp (aeps_xboxr)
// Verified against IDA (aeps_xboxr:apsBillboardNode.o):
//   apsBillboardNode::Render @0x813C90
// ============================================================================
#include "apsBillboardRenderer.h"
#include "apsNodeRenderer.h"
#include "apsParticleTypes.h"

// apsRenderNode::TestFlags - ea: 0x00813D00
unsigned int apsRenderNode::TestFlags(unsigned int flag) const
{
    return flag & mFlags;
}

// apsRenderNode::Particles - ea: 0x00813D10
unsigned char* apsRenderNode::Particles() const
{
    return mParticles;
}

// apsRenderNode::NumParticles - ea: 0x00813D20
int apsRenderNode::NumParticles()
{
    return mNumParticles;
}

// apsRenderNode::Stride - ea: 0x00813D30
int apsRenderNode::Stride()
{
    return mStride;
}

// apsBillboardRenderer::Texture - ea: 0x00813D40
nglTexture* apsBillboardRenderer::Texture() const
{
    return mTexture;
}

// apsBillboardRenderer::Normal - ea: 0x00813D50
const math::Dir3& apsBillboardRenderer::Normal() const
{
    return mNormal;
}

// apsBillboardRenderer::WidthFrames - ea: 0x00813D60
float apsBillboardRenderer::WidthFrames() const
{
    return 1.0f;
}

// apsBillboardRenderer::MaxFrame - ea: 0x00813D70
float apsBillboardRenderer::MaxFrame() const
{
    return 0.0f;
}

// apsBillboardRenderer::InvWidthFrames - ea: 0x00813D80
float apsBillboardRenderer::InvWidthFrames() const
{
    return 1.0f;
}

// apsBillboardRenderer::InvHeightFrames - ea: 0x00813D90
float apsBillboardRenderer::InvHeightFrames() const
{
    return 1.0f;
}

// apsBillboardRenderer::AlphaFadeStart - ea: 0x00813DA0
float apsBillboardRenderer::AlphaFadeStart() const
{
    return mAlphaFadeStart;
}

// apsBillboardRenderer::AlphaFadeEnd - ea: 0x00813DB0
float apsBillboardRenderer::AlphaFadeEnd() const
{
    return mAlphaFadeEnd;
}

// apsBillboardNode::GetRenderSortBuffer - ea: 0x00813DC0
apsRenderSort::Buffer* apsBillboardNode::GetRenderSortBuffer()
{
    return mRenderSortBuffer;
}

// BillboardParticle accessors (apsBillboardNode.o)
// ea: 0x00813ED0
math::Dir3::Packed& BillboardParticle::GetPos()
{
    return mPos;
}

// ea: 0x00813EE0
math::Vector4 BillboardParticle::GetColor()
{
    math::Vector4 result;
    result.v = _mm_setr_ps(1.0f, 1.0f, 1.0f, mAlpha);
    return result;
}

// ea: 0x00813F30
float BillboardParticle::GetWidth()
{
    return mRadius;
}

// ea: 0x00813F40
float BillboardParticle::GetHeight()
{
    return mRadius;
}

// ea: 0x00813F50
float BillboardParticle::GetAngle()
{
    return mAngle;
}

// ea: 0x00813F60
float BillboardParticle::GetFrame()
{
    return 0.0f;
}

// ea: 0x00813F70
math::Vector4& BillboardParticle::GetVelocity()
{
    return mVelocity;
}

// apsBillboardRender::GetVShader - ea: 0x00813EB0
unsigned long apsBillboardRender::GetVShader()
{
    return VS[0];
}

// apsBillboardRenderPixel::GetPShader - ea: 0x00813EC0
unsigned long* apsBillboardRenderPixel::GetPShader()
{
    return reinterpret_cast<unsigned long*>(PS[0]);
}

// ============================================================================
// apsBillboardNode::Render — forward to the billboard node renderer.
// ea: 0x813C90
// ============================================================================
void apsBillboardNode::Render() {
    cNodeRenderer<BillboardParticle, apsBillboardNode> renderer;
    renderer.mNode = this;
    renderer.Render();
}
