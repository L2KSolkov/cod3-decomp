// ============================================================================
// cdPropellerShader.h — propeller shader (6 non-inline funcs).
// Source: source/cdPropellerShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdPropellerShader.o):
//   cdPropellerShaderMat::ctor @0x7D0D50
//   InitCDPropellerShader  @0x7D0DE0
//   ToggleCDPropellerShader @0x7D0E30
//   cdPropellerShader::Register @0x7D0E50
//   cdPropellerShader::AddNode @0x7D0E90
//   cdPropellerShaderNode::Render @0x7D0F10
// ============================================================================
#ifndef COD3_RENDER_CDPROPELLERSHADER_H
#define COD3_RENDER_CDPROPELLERSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdPropellerShaderMat — propeller shader material (28 bytes)
// ============================================================================
struct cdPropellerShaderMat : nglMaterial {
    nglTexture* mTexture;   // +0x10
    int         mCullMode;  // +0x14
    float       mRotation;  // +0x18

    cdPropellerShaderMat(nglTexture* iTexture);  // @0x7D0D50
};
static_assert(sizeof(cdPropellerShaderMat) == 0x1C, "cdPropellerShaderMat size mismatch");

// ============================================================================
// cdPropellerShaderNode — propeller shader render node (24 bytes)
// ============================================================================
struct cdPropellerShaderNode : nglShaderNode {
    cdPropellerShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdPropellerShaderNode) == 0x18, "cdPropellerShaderNode size mismatch");

// ============================================================================
// cdPropellerShader — propeller shader (16 bytes)
// ============================================================================
class cdPropellerShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7D12E0
    virtual void Register();  // @0x7D0E50
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D0E90
};
static_assert(sizeof(cdPropellerShader) == 0x10, "cdPropellerShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdPropellerShaderVertex.o)
// ============================================================================
namespace cdPropellerRender {
    extern unsigned long* VS;                // ?VS@cdPropellerRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdPropellerRender@@3PAPBIA
}
namespace cdPropellerPixel {
    extern unsigned long** PS;               // ?PS@cdPropellerPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdPropellerPixel@@3PAPBIA
}
namespace cdPropellerFullbrightPixel {
    extern unsigned long** PS;               // ?PS@cdPropellerFullbrightPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdPropellerFullbrightPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdPropellerShader* gCDPropellerShader;  // @0x10DE540

void InitCDPropellerShader();   // @0x7D0DE0
void ToggleCDPropellerShader(); // @0x7D0E30

#endif // COD3_RENDER_CDPROPELLERSHADER_H
