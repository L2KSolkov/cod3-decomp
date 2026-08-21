// ============================================================================
// apsRectangleRenderer.cpp — rectangle renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsRectangleRenderer.cpp
// Verified against IDA (aeps_xboxr:apsRectangleRenderer.o):
//   ctor(cArgs) @0x8051F0
//   Init        @0x805230
//   Render      @0x805260
// ============================================================================
#include "apsRectangleRenderer.h"

// APS shader static data definitions (aeps_xboxr:apsRectangleRendererVertex.o).
// The reference has one-element shader storage and table objects.  The
// microcode payload is not present in the Win32 sources, so the table entries
// remain null-equivalent placeholders while preserving the reference layout.
static unsigned long apsRectangleRenderShader[1] = {};
static unsigned long apsRectangleRenderPixelShader[1] = {};
static unsigned long* apsRectangleRenderPixelPS[1] = {};
static const unsigned long* apsRectangleRenderVShaderTable[1] = {
    apsRectangleRenderShader
};
static const unsigned long* apsRectangleRenderPShaderTable[1] = {
    apsRectangleRenderPixelShader
};

unsigned long* apsRectangleRender::VS = apsRectangleRenderShader;
const unsigned long** apsRectangleRender::VShaderTable =
    apsRectangleRenderVShaderTable;
unsigned long** apsRectangleRenderPixel::PS = apsRectangleRenderPixelPS;
const unsigned long** apsRectangleRenderPixel::PShaderTable =
    apsRectangleRenderPShaderTable;

// tl_system.o (tl_xboxr, ported)
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// apsRectangleRenderer::apsRectangleRenderer — construct: run base billboard
// ctor, then override mFields.
// ea: 0x8051F0
// ============================================================================
apsRectangleRenderer::apsRectangleRenderer(const apsRectangleRenderer::cArgs& args)
    : apsBillboardRenderer(&args) {
    mFields = (args.mVelocityTracked != 0) ? 30933 : 213;
}

// ============================================================================
// apsRectangleRenderer::Init — register the vertex/pixel shaders.
// ea: 0x805230
// ============================================================================
void apsRectangleRenderer::Init() {
    nglDxRegisterVShader(
        apsRectangleRender::VS,
        reinterpret_cast<const unsigned int*>(apsRectangleRender::VShaderTable[0]));
    nglDxRegisterPShader(
        apsRectangleRenderPixel::PS,
        reinterpret_cast<const unsigned int*>(apsRectangleRenderPixel::PShaderTable[0]));
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

void apsRectangleRender::RegisterVShader()
{
    nglDxRegisterVShader(
        apsRectangleRender::VS,
        reinterpret_cast<const unsigned int*>(apsRectangleRender::VShaderTable[0]));
}

void apsRectangleRenderPixel::RegisterPShader()
{
    nglDxRegisterPShader(
        apsRectangleRenderPixel::PS,
        reinterpret_cast<const unsigned int*>(apsRectangleRenderPixel::PShaderTable[0]));
}

void apsRectangleRenderPixel::InitPShader()
{
    nglDxRegisterPShader(
        apsRectangleRenderPixel::PS,
        reinterpret_cast<const unsigned int*>(apsRectangleRenderPixel::PShaderTable[0]));
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
