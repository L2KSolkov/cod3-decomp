// ============================================================================
// apsColorUVARectangleRenderer.cpp — color UVA rectangle renderer (3 funcs).
// Source: c:\cod\code\tl\aeps\source\apsColorUVARectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsColorUVARectangleRenderer.o):
//   ctor(cArgs) @0x804C90
//   Init        @0x804CD0
//   Render      @0x804D00
// ============================================================================
#include "apsColorUVARectangleRenderer.h"

// APS shader static data definitions (aeps_xboxr)
unsigned int* apsColorUVARectangleRender::VS = nullptr;
const unsigned int** apsColorUVARectangleRender::VShaderTable = nullptr;
unsigned int** apsColorUVARectangleRenderPixel::PS = nullptr;
const unsigned int** apsColorUVARectangleRenderPixel::PShaderTable = nullptr;

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
    if (apsColorUVARectangleRender::VShaderTable != NULL)
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(apsColorUVARectangleRender::VS), apsColorUVARectangleRender::VShaderTable[0]);
    if (apsColorUVARectangleRenderPixel::PShaderTable != NULL)
        nglDxRegisterPShader(reinterpret_cast<unsigned long**>(apsColorUVARectangleRenderPixel::PS), apsColorUVARectangleRenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsColorUVARectangleRenderer::Render — dispatch to the DefaultRender template.
// ea: 0x804D00
// ============================================================================
apsRenderer::eRenderResult apsColorUVARectangleRenderer::Render(const apsRendererRenderInfo& rinfo) {
    return this->DefaultRender<apsColorUVARectangleRenderer, apsColorUVARectangleNode>(rinfo);
}

// ============================================================================
// apsColorUVARectangleNode::GetDesc (inline COMDAT)
// ============================================================================
// ea: 0x804E10
void apsColorUVARectangleNode::GetDesc(char* buf) {
    strcpy(buf, "apsColorUVARectangleNode");
}

// ============================================================================
// apsColorUVARectangleRenderer::GetId - apsRegister.o COMDAT
// ============================================================================
unsigned int apsColorUVARectangleRenderer::GetId() const {
    return 1666537074;
}

// ============================================================================
// apsColorUVARectangleRenderer::GetVersion - apsRegister.o COMDAT
// ============================================================================
float apsColorUVARectangleRenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsColorUVARectangleRenderer::~apsColorUVARectangleRenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsColorUVARectangleRenderer::~apsColorUVARectangleRenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}
