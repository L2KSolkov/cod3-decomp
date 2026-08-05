// ============================================================================
// cdWorldVertexLitShader.h — world vertex-lit shader (6 non-inline funcs).
// Source: source/cdWorldVertexLitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldVertexLitShader.o):
//   cdWorldVertexLitShaderMat::ctor @0x7DE770
//   InitCDWorldVertexLitShader  @0x7DE800
//   ToggleCDWorldVertexLitShader @0x7DE850
//   cdWorldVertexLitShader::Register @0x7DE870
//   cdWorldVertexLitShader::AddNode @0x7DE8D0
//   cdWorldVertexLitShaderNode::Render @0x7DE940
// ============================================================================
#ifndef COD3_RENDER_CDWORLDVERTEXLITSHADER_H
#define COD3_RENDER_CDWORLDVERTEXLITSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdWorldVertexLitShaderMat — world vertex-lit shader material (20 bytes)
// ============================================================================
struct cdWorldVertexLitShaderMat : nglMaterial {
    nglTexture* mTexture;  // +0x10

    cdWorldVertexLitShaderMat(nglTexture* iTexture);  // @0x7DE770
};
static_assert(sizeof(cdWorldVertexLitShaderMat) == 0x14, "cdWorldVertexLitShaderMat size mismatch");

// ============================================================================
// cdWorldVertexLitShaderNode — world vertex-lit shader render node (24 bytes)
// ============================================================================
struct cdWorldVertexLitShaderNode : nglShaderNode {
    cdWorldVertexLitShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdWorldVertexLitShaderNode) == 0x18, "cdWorldVertexLitShaderNode size mismatch");

// ============================================================================
// cdWorldVertexLitShader — world vertex-lit shader (16 bytes)
// ============================================================================
class cdWorldVertexLitShader : public nglShader {
public:
    virtual void Register();  // @0x7DE870
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7DE8D0
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdWorldVertexLitShader) == 0x10, "cdWorldVertexLitShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdWorldVertexLitShaderVertex.o)
// ============================================================================
namespace cdWorldVertexLitRender {
    extern unsigned long* VS;                // ?VS@cdWorldVertexLitRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdWorldVertexLitRender@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdWorldVertexLitRender@@3KA
}
namespace cdWorldVertexLitPixel {
    extern unsigned long** PS;               // ?PS@cdWorldVertexLitPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdWorldVertexLitPixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdWorldVertexLitPixel@@3PAKA
}
namespace cdWorldVertexLitFullbrightPixel {
    extern unsigned long** PS;               // ?PS@cdWorldVertexLitFullbrightPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdWorldVertexLitFullbrightPixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdWorldVertexLitFullbrightPixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdWorldVertexLitShader* gCDWorldVertexLitShader;    // @0x10DE5DC
extern cdWorldVertexLitShader* g_cdWorldVertexLitShader;   // @0x10DE5D8

void InitCDWorldVertexLitShader();   // @0x7DE800
void ToggleCDWorldVertexLitShader(); // @0x7DE850

#endif // COD3_RENDER_CDWORLDVERTEXLITSHADER_H
