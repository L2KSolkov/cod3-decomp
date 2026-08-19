// ============================================================================
// apsRectangleRenderer.cpp — rectangle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsRectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsRectangleRenderer.o):
//   ctor(cArgs) @0x8051F0
//   Init        @0x805230
//   Render      @0x805260
// ============================================================================
#include "apsRectangleRenderer.h"

// APS shader static data definitions (aeps_xboxr)
unsigned int* apsRectangleRender::VS = nullptr;
const unsigned int** apsRectangleRender::VShaderTable = nullptr;
unsigned int** apsRectangleRenderPixel::PS = nullptr;
const unsigned int** apsRectangleRenderPixel::PShaderTable = nullptr;

// tl_system.o (tl_xboxr, ported)
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// apsRectangleRenderer::apsRectangleRenderer — construct: run base billboard
// ctor, then override mFields.
// ea: 0x8051F0
// ============================================================================
apsRectangleRenderer::apsRectangleRenderer(const apsRectangleRenderer::cArgs* args)
    : apsBillboardRenderer(args) {
    mFields = (args->mVelocityTracked != 0) ? 30933 : 213;
}

// ============================================================================
// apsRectangleRenderer::Init — register the vertex/pixel shaders.
// ea: 0x805230
// ============================================================================
void apsRectangleRenderer::Init() {
    if (apsRectangleRender::VShaderTable != NULL)
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(apsRectangleRender::VS), apsRectangleRender::VShaderTable[0]);
    if (apsRectangleRenderPixel::PShaderTable != NULL)
        nglDxRegisterPShader(reinterpret_cast<unsigned long**>(apsRectangleRenderPixel::PS), apsRectangleRenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsRectangleRenderer::Render — color not supported; dispatch to DefaultRender.
// ea: 0x805260
// ============================================================================
apsRenderer::eRenderResult apsRectangleRenderer::Render(const apsRendererRenderInfo& rinfo) {
    if ((rinfo.pfd->mFields & 8) != 0 &&
        _tlAssert("source/apsRectangleRenderer.cpp", 49,
                  "!rinfo.pfd->HasField(apsPFDField_Color)",
                  "apsRectangleRenderer can't handle color")) {
        __debugbreak();
    }
    return this->DefaultRender<apsRectangleRenderer, apsRectangleNode>(rinfo);
}

// ============================================================================
// apsRectangleNode::GetDesc — ea: 0x804EA0 (inline COMDAT)
// ============================================================================
void apsRectangleNode::GetDesc(char* buf) {
}

// ============================================================================
// apsRectangleRenderer::GetId - apsRegister.o COMDAT
// ============================================================================
unsigned int apsRectangleRenderer::GetId() const {
    return 1382376308;
}

// ============================================================================
// apsRectangleRenderer::GetVersion - apsRegister.o COMDAT
// ============================================================================
float apsRectangleRenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsRectangleRenderer::~apsRectangleRenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsRectangleRenderer::~apsRectangleRenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}
