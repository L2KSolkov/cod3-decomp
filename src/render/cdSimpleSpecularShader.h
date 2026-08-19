// ============================================================================
// cdSimpleSpecularShader.h — simple specular shader (7 non-inline funcs).
// Source: source/cdSimpleSpecularShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleSpecularShader.o):
//   cdSimpleSpecularShaderMat::ctor @0x7D4DD0
//   InitCDSimpleSpecularShader  @0x7D4E60
//   ToggleCDSimpleSpecularShader @0x7D4EB0
//   cdSimpleSpecularShader::Register @0x7D4ED0
//   GetEyePos @0x7D4F00
//   cdSimpleSpecularShader::AddNode @0x7D4FF0
//   cdSimpleSpecularShaderNode::Render @0x7D5060
// ============================================================================
#ifndef COD3_RENDER_CDSIMPLESPECULARSHADER_H
#define COD3_RENDER_CDSIMPLESPECULARSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdSimpleSpecularShaderMat — simple specular shader material (36 bytes)
// ============================================================================
struct cdSimpleSpecularShaderMat : nglMaterial {
    nglTexture* mDiffuseTexture;   // +0x10
    nglTexture* mSpecularTexture;  // +0x14
    float       mSpecularPower;    // +0x18
    float       mSpecularLevel;    // +0x1C
    int         mCullMode;         // +0x20

    cdSimpleSpecularShaderMat(nglTexture* iDiffuseTexture, nglTexture* iSpecularTexture);  // @0x7D4DD0
};
static_assert(sizeof(cdSimpleSpecularShaderMat) == 0x24, "cdSimpleSpecularShaderMat size mismatch");

// ============================================================================
// cdSimpleSpecularShaderNode — simple specular shader render node (24 bytes)
// ============================================================================
struct cdSimpleSpecularShaderNode : nglShaderNode {
    cdSimpleSpecularShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdSimpleSpecularShaderNode) == 0x18, "cdSimpleSpecularShaderNode size mismatch");

// ============================================================================
// cdSimpleSpecularShader — simple specular shader (16 bytes)
// ============================================================================
class cdSimpleSpecularShader : public nglShader {
public:
    virtual void Register();  // @0x7D4ED0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D4FF0
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdSimpleSpecularShader) == 0x10, "cdSimpleSpecularShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdSimpleSpecularShaderVertex.o)
// ============================================================================
namespace cdSimpleSpecularRender {
    extern unsigned long VS[2];                   // ?VS@cdSimpleSpecularRender@@3PAKA
    extern unsigned int const* VShaderTable[2];    // ?VShaderTable@cdSimpleSpecularRender@@3PAPBIA
}
namespace cdSimpleSpecularPixel {
    extern unsigned long* PS[2];                   // ?PS@cdSimpleSpecularPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];     // ?PShaderTable@cdSimpleSpecularPixel@@3PAPBIA
}
namespace cdSimpleSpecularFullbrightPixel {
    extern unsigned long* PS[2];                   // ?PS@cdSimpleSpecularFullbrightPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];     // ?PShaderTable@cdSimpleSpecularFullbrightPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
void GetEyePos(nglMeshNode* meshNode, math::Vector4& eyePos);  // @0x7D4F00

extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdSimpleSpecularShader* gCDSimpleSpecularShader;  // @0x10DE574

void InitCDSimpleSpecularShader();  // @0x7D4E60
void ToggleCDSimpleSpecularShader();                   // @0x7D4EB0

#endif // COD3_RENDER_CDSIMPLESPECULARSHADER_H
