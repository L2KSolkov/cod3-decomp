// ============================================================================
// cdCharShader.h — character shader (5 non-inline funcs).
// Source: source/cdCharShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdCharShader.o):
//   InitCDCharShader  @0x7D2B70
//   ToggleCDCharShader @0x7D2BC0
//   cdCharShader::Register @0x7D2BE0
//   cdCharShader::AddNode @0x7D2C40
//   cdCharShaderNode::Render @0x7D2CB0
// ============================================================================
#ifndef COD3_RENDER_CDCHARSHADER_H
#define COD3_RENDER_CDCHARSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdCharShaderMat — character shader material (20 bytes)
// ============================================================================
struct cdCharShaderMat : nglMaterial {
    nglTexture* mDiffuse;  // +0x10
};
static_assert(sizeof(cdCharShaderMat) == 0x14, "cdCharShaderMat size mismatch");

// ============================================================================
// cdCharShaderNode — character shader render node (24 bytes)
// ============================================================================
struct cdCharShaderNode : nglShaderNode {
    cdCharShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdCharShaderNode) == 0x18, "cdCharShaderNode size mismatch");

// ============================================================================
// cdCharShader — character shader (16 bytes)
// ============================================================================
class cdCharShader : public nglShader {
public:
    virtual void Register();  // @0x7D2BE0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D2C40
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdCharShader) == 0x10, "cdCharShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdCharShaderVertex.o)
// ============================================================================
namespace cdCharShaderRender {
    extern unsigned long* VS;                 // ?VS@cdCharShaderRender@@3PAKA
    extern unsigned int const** VShaderTable;  // ?VShaderTable@cdCharShaderRender@@3PAPBIA
    extern unsigned long Shader;              // ?Shader@cdCharShaderRender@@3KA
}
namespace cdCharPixel {
    extern unsigned long** PS;                // ?PS@cdCharPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;  // ?PShaderTable@cdCharPixel@@3PAPBIA
    extern unsigned long* Shader;             // ?Shader@cdCharPixel@@3PAKA
}
namespace cdCharFullbrightPixel {
    extern unsigned long** PS;                // ?PS@cdCharFullbrightPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;  // ?PShaderTable@cdCharFullbrightPixel@@3PAPBIA
    extern unsigned long* Shader;             // ?Shader@cdCharFullbrightPixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdCharShader* gCDCharShader;  // @0x10DE55C

cdCharShader* InitCDCharShader();  // @0x7D2B70
char ToggleCDCharShader();         // @0x7D2BC0

#endif // COD3_RENDER_CDCHARSHADER_H
