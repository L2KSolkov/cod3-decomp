// ============================================================================
// apsColorUVARenderer.cpp — color UVA renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorUVARenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorUVARenderer.o):
//   ctor(cArgs) @0x805480
//   Init        @0x8054C0
//   Render      @0x8054F0
// ============================================================================
#include "apsColorUVARenderer.h"

// ============================================================================
// apsColorUVARenderer::apsColorUVARenderer — construct: run base UVA ctor,
// then override mFields (color supported).
// ea: 0x805480
// ============================================================================
apsColorUVARenderer::apsColorUVARenderer(const apsColorUVARenderer::cArgs* args)
    : apsUVARenderer(args) {
    mFields = (args->mVelocityTracked != 0) ? 16731 : 347;
}

// ============================================================================
// apsColorUVARenderer::Init — register the vertex/pixel shaders.
// ea: 0x8054C0
// ============================================================================
void apsColorUVARenderer::Init() {
    nglDxRegisterVShader(apsColorUVARender::VS, apsColorUVARender::VShaderTable[0]);
    nglDxRegisterPShader(apsColorUVARenderPixel::PS, apsColorUVARenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsColorUVARenderer::Render — dispatch to the DefaultRender template.
// ea: 0x8054F0
// ============================================================================
apsRenderer::eRenderResult apsColorUVARenderer::Render(const apsRendererRenderInfo& rinfo) {
    return this->DefaultRender<apsColorUVARenderer, apsColorUVANode>(rinfo);
}

// ============================================================================
// apsColorUVANode::GetDesc — ea: 0x8053F0 (inline COMDAT)
// ============================================================================
void apsColorUVANode::GetDesc(char* buf) {
}
