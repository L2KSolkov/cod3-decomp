// ============================================================================
// apsUVARectangleRenderer.cpp — UVA rectangle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsUVARectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsUVARectangleRenderer.o):
//   ctor(cArgs) @0x804F60
//   Init        @0x804FA0
//   Render      @0x804FD0
// ============================================================================
#include "apsUVARectangleRenderer.h"

// APS shader static data definitions (aeps_xboxr)
unsigned int* apsUVARectangleRender::VS = nullptr;
const unsigned int** apsUVARectangleRender::VShaderTable = nullptr;
unsigned int** apsUVARectangleRenderPixel::PS = nullptr;
const unsigned int** apsUVARectangleRenderPixel::PShaderTable = nullptr;

// tl_system.o (tl_xboxr, ported)
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// apsUVARectangleRenderer::apsUVARectangleRenderer — construct: run base UVA
// ctor, then override mFields.
// ea: 0x804F60
// ============================================================================
apsUVARectangleRenderer::apsUVARectangleRenderer(const apsUVARectangleRenderer::cArgs* args)
    : apsUVARenderer(args) {
    mFields = (args->mVelocityTracked != 0) ? 22997 : 469;
}

// ============================================================================
// apsUVARectangleRenderer::Init — register the vertex/pixel shaders.
// ea: 0x804FA0
// ============================================================================
void apsUVARectangleRenderer::Init() {
    nglDxRegisterVShader(apsUVARectangleRender::VS, apsUVARectangleRender::VShaderTable[0]);
    nglDxRegisterPShader(apsUVARectangleRenderPixel::PS, apsUVARectangleRenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsUVARectangleRenderer::Render — color not supported; dispatch.
// ea: 0x804FD0
// ============================================================================
apsRenderer::eRenderResult apsUVARectangleRenderer::Render(const apsRendererRenderInfo& rinfo) {
    if ((rinfo.pfd->mFields & 8) != 0 &&
        _tlAssert("source/apsUVARectangleRenderer.cpp", 48,
                  "!rinfo.pfd->HasField(apsPFDField_Color)",
                  "apsUVARectangleRenderer can't handle color")) {
        __debugbreak();
    }
    return this->DefaultRender<apsUVARectangleRenderer, apsUVARectangleNode>(rinfo);
}

// ============================================================================
// apsUVARectangleNode::GetDesc — ea: 0x804F00 (inline COMDAT)
// ============================================================================
void apsUVARectangleNode::GetDesc(char* buf) {
}

// ============================================================================
// apsUVARectangleRenderer::GetId - apsRegister.o COMDAT
// ============================================================================
unsigned int apsUVARectangleRenderer::GetId() const {
    return 1431720549;
}

// ============================================================================
// apsUVARectangleRenderer::GetVersion - apsRegister.o COMDAT
// ============================================================================
float apsUVARectangleRenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsUVARectangleRenderer::~apsUVARectangleRenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsUVARectangleRenderer::~apsUVARectangleRenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}
