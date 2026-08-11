// ============================================================================
// apsColorRectangleRenderer.cpp — color rectangle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorRectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorRectangleRenderer.o):
//   ctor(cArgs) @0x804920
//   Init        @0x804960
//   Render      @0x804990
// ============================================================================
#include "apsColorRectangleRenderer.h"

// ============================================================================
// apsColorRectangleRenderer::apsColorRectangleRenderer — construct: run base
// billboard ctor, then override mFields (color supported).
// ea: 0x804920
// ============================================================================
apsColorRectangleRenderer::apsColorRectangleRenderer(const apsColorRectangleRenderer::cArgs* args)
    : apsBillboardRenderer(args) {
    mFields = (args->mVelocityTracked != 0) ? 16605 : 221;
}

// ============================================================================
// apsColorRectangleRenderer::Init — register the vertex/pixel shaders.
// ea: 0x804960
// ============================================================================
void apsColorRectangleRenderer::Init() {
    nglDxRegisterVShader(apsColorRectangleRender::VS, apsColorRectangleRender::VShaderTable[0]);
    nglDxRegisterPShader(apsColorRectangleRenderPixel::PS, apsColorRectangleRenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsColorRectangleRenderer::Render — dispatch to the DefaultRender template.
// ea: 0x804990
// ============================================================================
apsRenderer::eRenderResult apsColorRectangleRenderer::Render(const apsRendererRenderInfo& rinfo) {
    return this->DefaultRender<apsColorRectangleRenderer, apsColorRectangleNode>(rinfo);
}

// ============================================================================
// apsColorRectangleNode::GetDesc — ea: 0x8048A0 (inline COMDAT)
// ============================================================================
void apsColorRectangleNode::GetDesc(char* buf) {
}

// ============================================================================
// apsColorRectangleRenderer::GetId - apsRegister.o COMDAT
// ============================================================================
unsigned int apsColorRectangleRenderer::GetId() const {
    return 1129472884;
}

// ============================================================================
// apsColorRectangleRenderer::GetVersion - apsRegister.o COMDAT
// ============================================================================
float apsColorRectangleRenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsColorRectangleRenderer::~apsColorRectangleRenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsColorRectangleRenderer::~apsColorRectangleRenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}
