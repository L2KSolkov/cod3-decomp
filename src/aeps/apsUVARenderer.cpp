// ============================================================================
// apsUVARenderer.cpp — UVA sprite renderer (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsUVARenderer.cpp
// Verified against IDA (aeps_xboxr:apsUVARenderer.o):
//   ctor(cArgs) @0x805960
//   Init        @0x805A50
//   Render      @0x805A80
// ============================================================================
#include "apsUVARenderer.h"

// APS shader static data definitions (aeps_xboxr)
unsigned int* apsUVARender::VS = nullptr;
const unsigned int** apsUVARender::VShaderTable = nullptr;
unsigned int** apsUVARenderPixel::PS = nullptr;
const unsigned int** apsUVARenderPixel::PShaderTable = nullptr;

// tl_system.o (tl_xboxr, ported)
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// apsUVARenderer::apsUVARenderer — construct: run base billboard ctor, then
// set the UVA frame math.
// ea: 0x805960
// ============================================================================
apsUVARenderer::apsUVARenderer(const apsUVARenderer::cArgs* args)
    : apsBillboardRenderer(args) {
    mWidthFrames = (float)args->mWidthFrames;
    mMaxFrame = (float)(args->mWidthFrames * args->mHeightFrames - 1);
    mInvWidthFrames = 1.0f / args->mWidthFrames;
    mInvHeightFrames = 1.0f / args->mHeightFrames;
    mNormal.v = _mm_setr_ps(args->mNormal.x, args->mNormal.y, args->mNormal.z, 0.0f);
    mFields = (args->mVelocityTracked != 0) ? 31059 : 339;
}

// ============================================================================
// apsUVARenderer::Init — register the vertex/pixel shaders.
// ea: 0x805A50
// ============================================================================
void apsUVARenderer::Init() {
    if (apsUVARender::VShaderTable != NULL)
        nglDxRegisterVShader(reinterpret_cast<unsigned long*>(apsUVARender::VS), apsUVARender::VShaderTable[0]);
    if (apsUVARenderPixel::PShaderTable != NULL)
        nglDxRegisterPShader(reinterpret_cast<unsigned long**>(apsUVARenderPixel::PS), apsUVARenderPixel::PShaderTable[0]);
}

// ============================================================================
// apsUVARenderer::Render — color not supported; dispatch to DefaultRender.
// ea: 0x805A80
// ============================================================================
apsRenderer::eRenderResult apsUVARenderer::Render(const apsRendererRenderInfo& rinfo) {
    if ((rinfo.pfd->mFields & 8) != 0 &&
        _tlAssert("source/apsUVARenderer.cpp", 80,
                  "!rinfo.pfd->HasField(apsPFDField_Color)",
                  "apsUVARenderer can't handle color")) {
        __debugbreak();
    }
    return this->DefaultRender<apsUVARenderer, apsUVANode>(rinfo);
}

// ============================================================================
// apsUVANode::GetDesc — ea: 0x804290 (inline COMDAT)
// ============================================================================
void apsUVANode::GetDesc(char* buf) {
}

// ============================================================================
// apsUVARenderer::GetId - apsRegister.o COMDAT
// ============================================================================
unsigned int apsUVARenderer::GetId() const {
    return 1431716178;
}

// ============================================================================
// apsUVARenderer::GetVersion - apsRegister.o COMDAT
// ============================================================================
float apsUVARenderer::GetVersion() const {
    return 1.0f;
}

// ============================================================================
// apsUVARenderer::~apsUVARenderer - apsRegister.o COMDAT (sets base vtable)
// ============================================================================
apsUVARenderer::~apsUVARenderer() {
    *(unsigned int*)this = 0x00D384E0;  // apsVirtualBase vtable
}
