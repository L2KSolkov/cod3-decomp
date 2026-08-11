// ============================================================================
// cdRiverShader.h — river shader (10 non-inline funcs).
// Source: source/cdRiverShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdRiverShader.o):
//   cdRiverShaderMat::ctor @0x7D6BC0
//   InitCDRiverShader  @0x7D6BE0
//   ToggleCDRiverShader @0x7D6C30
//   cdRiverShaderNode::GetVShader @0x7D6CA0
//   cdRiverShaderNode::GetFullbrightPShader @0x7D6CB0
//   cdRiverShaderNode::GetPShader @0x7D6CC0
//   cdRiverShaderNode::GetVShaderParamsStartAddress @0x7D6CE0
//   cdRiverShaderNode::GetVShaderFogConstantOffset @0x7D6CF0
//   cdRiverShader::Register @0x7D6C50
//   cdRiverShader::AddNode @0x7D6D00
//   cdRiverShaderNode::Render @0x7D6D70
// ============================================================================
#ifndef COD3_RENDER_CDRIVERSHADER_H
#define COD3_RENDER_CDRIVERSHADER_H

#include "cdOceanShader.h"

// ============================================================================
// cdRiverShaderMat — river shader material (40 bytes, derives ocean mat)
// ============================================================================
struct cdRiverShaderMat : cdOceanShaderMat {
    cdRiverShaderMat(nglTexture* iTexture);  // @0x7D6BC0 (delegates to cdOceanShaderMat)
};
static_assert(sizeof(cdRiverShaderMat) == 0x28, "cdRiverShaderMat size mismatch");

// ============================================================================
// cdRiverShaderNode — river shader render node (24 bytes, derives ocean node)
// ============================================================================
struct cdRiverShaderNode : cdOceanShaderNode {
    virtual unsigned int GetVShader();               // @0x7D6CA0
    virtual unsigned long* GetFullbrightPShader();   // @0x7D6CB0
    virtual unsigned long* GetPShader(int l2, int l3, int lm);  // @0x7D6CC0
    virtual int GetVShaderParamsStartAddress();       // @0x7D6CE0
    virtual int GetVShaderFogConstantOffset();        // @0x7D6CF0
};
static_assert(sizeof(cdRiverShaderNode) == 0x18, "cdRiverShaderNode size mismatch");

// ============================================================================
// cdRiverShader — river shader (16 bytes, derives cdOceanShader)
// ============================================================================
class cdRiverShader : public cdOceanShader {
public:
    virtual void Register();  // @0x7D6C50
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D6D00
};
static_assert(sizeof(cdRiverShader) == 0x10, "cdRiverShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdRiverShaderVertex.o)
// ============================================================================
namespace cdRiverRender {
    extern unsigned long* VS;                // ?VS@cdRiverRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdRiverRender@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdRiverRender@@3KA
}
namespace cdRiverPixel {
    extern unsigned long* PS[2][2][2];               // ?PS@cdRiverPixel@@3PAY111PAKA
    extern unsigned int const* PShaderTable[2][2][2]; // ?PShaderTable@cdRiverPixel@@3PAY111PBIA
}
namespace cdRiverPixel_Fullbright {
    extern unsigned long* PS[2];               // ?PS@cdRiverPixel_Fullbright@@3PAPAKA
    extern unsigned int const* PShaderTable[2]; // ?PShaderTable@cdRiverPixel_Fullbright@@3PAPBIA
    extern unsigned long* Shader;              // ?Shader@cdRiverPixel_Fullbright@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdRiverShader* gCDRiverShader;  // @0x10DE594

void InitCDRiverShader();   // @0x7D6BE0
void ToggleCDRiverShader(); // @0x7D6C30

#endif // COD3_RENDER_CDRIVERSHADER_H
