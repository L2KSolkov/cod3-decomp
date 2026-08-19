// ============================================================================
// cdAirplaneMetalShader.h — airplane metal shader (5 non-inline funcs).
// Source: source/cdAirplaneMetalShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdAirplaneMetalShader.o):
//   InitCDAirplaneMetalShader  @0x7D4130
//   ToggleCDAirplaneMetalShader @0x7D4180
//   cdAirplaneMetalShader::Register @0x7D41A0
//   cdAirplaneMetalShader::AddNode @0x7D41E0
//   cdAirplaneMetalShaderNode::Render @0x7D4250
// ============================================================================
#ifndef COD3_RENDER_CDAIRPLANEMETALSHADER_H
#define COD3_RENDER_CDAIRPLANEMETALSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdAirplaneMetalShaderMat — airplane metal shader material (36 bytes)
// ============================================================================
struct cdAirplaneMetalShaderMat : nglMaterial {
    nglTexture* mDiffuse;   // +0x10
    nglTexture* mSpecFunc;  // +0x14
    float       mAlpha;     // +0x18
    float       mUScroll;   // +0x1C
    float       mVScroll;   // +0x20
};
static_assert(sizeof(cdAirplaneMetalShaderMat) == 0x24, "cdAirplaneMetalShaderMat size mismatch");

// ============================================================================
// cdAirplaneMetalShaderNode — airplane metal shader render node (24 bytes)
// ============================================================================
struct cdAirplaneMetalShaderNode : nglShaderNode {
    cdAirplaneMetalShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdAirplaneMetalShaderNode) == 0x18, "cdAirplaneMetalShaderNode size mismatch");

// ============================================================================
// cdAirplaneMetalShader — airplane metal shader (16 bytes)
// ============================================================================
class cdAirplaneMetalShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7D4A20
    virtual void Register();  // @0x7D41A0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D41E0
};
static_assert(sizeof(cdAirplaneMetalShader) == 0x10, "cdAirplaneMetalShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdAirplaneMetalShaderVertex.o)
// ============================================================================
namespace cdAirplaneMetalRender {
    extern unsigned long* VS;                // ?VS@cdAirplaneMetalRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdAirplaneMetalRender@@3PAPBIA
}
namespace cdAirplaneMetalPixel {
    extern unsigned long** PS;               // ?PS@cdAirplaneMetalPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdAirplaneMetalPixel@@3PAPBIA
}
namespace cdAirplaneMetalSolidColorPixel {
    extern unsigned long** PS;               // ?PS@cdAirplaneMetalSolidColorPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdAirplaneMetalSolidColorPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdAirplaneMetalShader* gCDAirplaneMetalShader;  // @0x10DE56C

void InitCDAirplaneMetalShader();   // @0x7D4130
void ToggleCDAirplaneMetalShader(); // @0x7D4180

#endif // COD3_RENDER_CDAIRPLANEMETALSHADER_H
