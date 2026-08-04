// ============================================================================
// apsColorUVARectangleRenderer.cpp — color UVA rectangle renderer (3 funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorUVARectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorUVARectangleRenderer.o):
//   ctor(cArgs) @0x804C90
//   Init        @0x804CD0
//   Render      @0x804D00
// ============================================================================
#include "apsColorUVARectangleRenderer.h"

// ============================================================================
// apsColorUVARectangleRenderer::apsColorUVARectangleRenderer — construct: run
// base UVA ctor, then override mFields (color supported).
// ea: 0x804C90
// ============================================================================
apsColorUVARectangleRenderer::apsColorUVARectangleRenderer(const apsColorUVARectangleRenderer::cArgs* args)
    : apsUVARenderer(args) {
    mFields = (args->mVelocityTracked != 0) ? 31197 : 477;
}

// ============================================================================
// apsColorUVARectangleRenderer::Init — register the vertex/pixel shaders.
// ea: 0x804CD0
// ============================================================================
void apsColorUVARectangleRenderer::Init() {
    nglDxRegisterVShader(apsColorUVARectangleRender::VS, apsColorUVARectangleRender::VShaderTable[0]);
    nglDxRegisterPShader(apsColorUVARectangleRenderPixel::PS, apsColorUVARectangleRenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsColorUVARectangleRenderer::Render — dispatch to the DefaultRender template.
// ea: 0x804D00
// ============================================================================
apsRenderer::eRenderResult apsColorUVARectangleRenderer::Render(const apsRendererRenderInfo& rinfo) {
    return this->DefaultRender<apsColorUVARectangleRenderer, apsColorUVARectangleNode>(rinfo);
}

// ============================================================================
// apsColorUVARectangleNode::GetDesc — ea: 0x804C30 (inline COMDAT)
// ============================================================================
void apsColorUVARectangleNode::GetDesc(char* buf) {
}
