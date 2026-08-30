// ============================================================================
// apsColorRectangleRenderer.cpp — color rectangle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorRectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorRectangleRenderer.o):
//   ctor(cArgs) @0x804920
//   Init        @0x804960
//   Render      @0x804990
// ============================================================================
#include "apsColorRectangleRenderer.h"

// apsColorRectangleRender::RegisterVShader - ea: 0x008049B0
void apsColorRectangleRender::RegisterVShader() {
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(VS), VShaderTable[0]);
}
// apsColorRectangleRenderPixel::RegisterPShader - ea: 0x008049D0
void apsColorRectangleRenderPixel::RegisterPShader() {
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(PS), PShaderTable[0]);
}
// apsColorRectangleRenderPixel::InitPShader - ea: 0x008049F0
void apsColorRectangleRenderPixel::InitPShader() {
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(PS), PShaderTable[0]);
}

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
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(apsColorRectangleRender::VS), apsColorRectangleRender::VShaderTable[0]);
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(apsColorRectangleRenderPixel::PS), apsColorRectangleRenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsColorRectangleRenderer::Render — dispatch to the DefaultRender template.
// ea: 0x804990
// ============================================================================
apsRenderer::eRenderResult apsColorRectangleRenderer::Render(const apsRendererRenderInfo& rinfo) {
    return this->DefaultRender<apsColorRectangleRenderer, apsColorRectangleNode>(rinfo);
}

// ============================================================================
// apsColorRectangleNode::GetDesc (inline COMDAT)
// ============================================================================
// ea: 0x804AB0
void apsColorRectangleNode::GetDesc(char* buf) {
    strcpy(buf, "apsColorRectangleNode");
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
