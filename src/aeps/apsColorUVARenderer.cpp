// ============================================================================
// apsColorUVARenderer.cpp — color UVA renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorUVARenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorUVARenderer.o):
//   ctor(cArgs) @0x805480
//   Init        @0x8054C0
//   Render      @0x8054F0
// ============================================================================
#include "apsColorUVARenderer.h"

// apsColorUVARender::RegisterVShader - ea: 0x00805510
void apsColorUVARender::RegisterVShader() {
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(VS), VShaderTable[0]);
}
// apsColorUVARenderPixel::RegisterPShader - ea: 0x00805530
void apsColorUVARenderPixel::RegisterPShader() {
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(PS), PShaderTable[0]);
}
// apsColorUVARenderPixel::InitPShader - ea: 0x00805550
void apsColorUVARenderPixel::InitPShader() {
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(PS), PShaderTable[0]);
}

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
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(apsColorUVARender::VS), apsColorUVARender::VShaderTable[0]);
    nglDxRegisterPShader(reinterpret_cast<unsigned long**>(apsColorUVARenderPixel::PS), apsColorUVARenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsColorUVARenderer::Render — dispatch to the DefaultRender template.
// ea: 0x8054F0
// ============================================================================
apsRenderer::eRenderResult apsColorUVARenderer::Render(const apsRendererRenderInfo& rinfo) {
    return this->DefaultRender<apsColorUVARenderer, apsColorUVANode>(rinfo);
}

// ============================================================================
// apsColorUVANode::GetDesc — ea: 0x805580 (inline COMDAT)
// ============================================================================
void apsColorUVANode::GetDesc(char* buf) {
    strcpy(buf, "apsColorUVANode");
}

// ============================================================================
// apsColorUVARenderer::GetId - apsRegister.o COMDAT
// ============================================================================
unsigned int apsColorUVARenderer::GetId() const {
    return 1129666113;
}

// ============================================================================
// apsColorUVARenderer::GetVersion - apsRegister.o COMDAT
// ============================================================================
float apsColorUVARenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsColorUVARenderer::~apsColorUVARenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsColorUVARenderer::~apsColorUVARenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}
