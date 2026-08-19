// ============================================================================
// cdGunSightShader.h — gun sight shader (6 non-inline funcs).
// Source: source/cdGunSightShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGunSightShader.o):
//   cdGunSightShaderMat::ctor @0x7CE100
//   InitCDGunSightShader  @0x7CE190
//   ToggleCDGunSightShader @0x7CE1E0
//   cdGunSightShader::Register @0x7CE200
//   cdGunSightShader::AddNode @0x7CE240
//   cdGunSightShaderNode::Render @0x7CE2B0
// ============================================================================
#ifndef COD3_RENDER_CDGUNSIGHTSHADER_H
#define COD3_RENDER_CDGUNSIGHTSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdGunSightShaderMat — gun sight shader material (24 bytes)
// ============================================================================
struct cdGunSightShaderMat : nglMaterial {
    nglTexture* mTexture;   // +0x10
    int         mCullMode;  // +0x14

    cdGunSightShaderMat(nglTexture* iTexture);  // @0x7CE100
};
static_assert(sizeof(cdGunSightShaderMat) == 0x18, "cdGunSightShaderMat size mismatch");

// ============================================================================
// cdGunSightShaderNode — gun sight shader render node (24 bytes)
// ============================================================================
struct cdGunSightShaderNode : nglShaderNode {
    cdGunSightShaderMat* mMaterial;  // +0x14

    void Render() override;           // @0x7CE2B0
};
static_assert(sizeof(cdGunSightShaderNode) == 0x18, "cdGunSightShaderNode size mismatch");

// ============================================================================
// cdGunSightShader — gun sight shader (16 bytes)
// ============================================================================
class cdGunSightShader : public nglShader {
public:
    virtual void Register();  // @0x7CE200
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7CE240
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdGunSightShader) == 0x10, "cdGunSightShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdGunSightShaderVertex.o)
// ============================================================================
namespace cdGunSightRender {
    extern unsigned int VS[1];               // ?VS@cdGunSightRender@@3PAKA
    extern unsigned int const* VShaderTable[1]; // ?VShaderTable@cdGunSightRender@@3PAPBIA
}
namespace cdGunSightPixel {
    extern unsigned int* PS[1];              // ?PS@cdGunSightPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[1]; // ?PShaderTable@cdGunSightPixel@@3PAPBIA
}
namespace cdGunSightFullbrightPixel {
    extern unsigned int* PS[1];              // ?PS@cdGunSightFullbrightPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[1]; // ?PShaderTable@cdGunSightFullbrightPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdGunSightShader* gCDGunSightShader;  // @0x10DE4D0

void InitCDGunSightShader();   // @0x7CE190
void ToggleCDGunSightShader(); // @0x7CE1E0

#endif // COD3_RENDER_CDGUNSIGHTSHADER_H
