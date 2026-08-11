// ============================================================================
// apsColorBillboardRenderer.cpp — color billboard renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorBillboardRenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorBillboardRenderer.o):
//   ctor(cArgs) @0x8056E0
//   Init        @0x805720
//   Render      @0x805750
// ============================================================================
#include "apsColorBillboardRenderer.h"

// APS shader static data definitions (aeps_xboxr)
unsigned int* apsColorBillboardRender::VS = nullptr;
const unsigned int** apsColorBillboardRender::VShaderTable = nullptr;
unsigned int** apsColorBillboardRenderPixel::PS = nullptr;
const unsigned int** apsColorBillboardRenderPixel::PShaderTable = nullptr;

// ============================================================================
// apsColorBillboardRenderer::apsColorBillboardRenderer — construct: run the
// base billboard ctor, then override mFields (color supported).
// ea: 0x8056E0
// ============================================================================
apsColorBillboardRenderer::apsColorBillboardRenderer(const apsColorBillboardRenderer::cArgs* args)
    : apsBillboardRenderer(args) {
    // base ctor already set mFields for billboard; override with color flags
    mFields = (args->mVelocityTracked != 0) ? 18523 : 91;
}

// ============================================================================
// apsColorBillboardRenderer::Init — register the vertex/pixel shaders.
// ea: 0x805720
// ============================================================================
void apsColorBillboardRenderer::Init() {
    nglDxRegisterVShader(apsColorBillboardRender::VS, apsColorBillboardRender::VShaderTable[0]);
    nglDxRegisterPShader(apsColorBillboardRenderPixel::PS, apsColorBillboardRenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsColorBillboardRenderer::Render — dispatch to the DefaultRender template.
// ea: 0x805750
// ============================================================================
apsRenderer::eRenderResult apsColorBillboardRenderer::Render(const apsRendererRenderInfo& rinfo) {
    return this->DefaultRender<apsColorBillboardRenderer, apsColorBillboardNode>(rinfo);
}

// ============================================================================
// apsColorBillboardNode::GetDesc — ea: 0x805670 (inline COMDAT)
// ============================================================================
void apsColorBillboardNode::GetDesc(char* buf) {
}

// ============================================================================
// apsColorBillboardRenderer::GetId - apsRegister.o COMDAT
// ============================================================================
unsigned int apsColorBillboardRenderer::GetId() const {
    return 1128426594;
}

// ============================================================================
// apsColorBillboardRenderer::GetVersion - apsRegister.o COMDAT
// ============================================================================
float apsColorBillboardRenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsColorBillboardRenderer::~apsColorBillboardRenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsColorBillboardRenderer::~apsColorBillboardRenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}
