// ============================================================================
// cdSkyShader.h — sky shader (4 non-inline funcs).
// Source: source/cdSkyShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSkyShader.o):
//   InitCDSkyShader  @0x7E0EA0
//   ToggleCDSkyShader @0x7E0EF0
//   cdSkyShader::Register @0x7E0F10
//   cdSkyShaderNode::Render @0x7E0F50
// ============================================================================
#ifndef COD3_RENDER_CDSKYSHADER_H
#define COD3_RENDER_CDSKYSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdSkyShaderMat — sky shader material (40 bytes)
// ============================================================================
struct cdSkyShaderMat : nglMaterial {
    nglTexture*    mTexture;    // +0x10
    unsigned int   mAlphaBlend; // +0x14
    float          mUScroll;    // +0x18
    float          mVScroll;    // +0x1C
    float          mZRotate;    // +0x20
    int            mDrawOrder;  // +0x24
};
static_assert(sizeof(cdSkyShaderMat) == 0x28, "cdSkyShaderMat size mismatch");

// ============================================================================
// cdSkyShaderNode — sky shader render node (24 bytes)
// ============================================================================
struct cdSkyShaderNode : nglShaderNode {
    cdSkyShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdSkyShaderNode) == 0x18, "cdSkyShaderNode size mismatch");

// ============================================================================
// cdSkyShader — sky shader (16 bytes)
// ============================================================================
class cdSkyShader : public nglShader {
public:
    virtual void Register();  // @0x7E0F10
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdSkyShader) == 0x10, "cdSkyShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdSkyShaderVertex.o)
// ============================================================================
namespace cdSkyShaderRender {
    extern unsigned long* VS;                // ?VS@cdSkyShaderRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdSkyShaderRender@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdSkyShaderRender@@3KA
}
namespace cdSkyShaderPixel {
    extern unsigned long** PS;               // ?PS@cdSkyShaderPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdSkyShaderPixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdSkyShaderPixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdSkyShader* gCDSkyShader;  // @0x10DE5FC

void InitCDSkyShader();   // @0x7E0EA0
void ToggleCDSkyShader(); // @0x7E0EF0

#endif // COD3_RENDER_CDSKYSHADER_H
