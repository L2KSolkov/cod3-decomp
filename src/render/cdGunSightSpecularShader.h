// ============================================================================
// cdGunSightSpecularShader.h — gun sight specular shader (6 non-inline funcs).
// Source: source/cdGunSightSpecularShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGunSightSpecularShader.o):
//   cdGunSightSpecularShaderMat::ctor @0x7CD770
//   InitCDGunSightSpecularShader  @0x7CD800
//   ToggleCDGunSightSpecularShader @0x7CD850
//   cdGunSightSpecularShader::Register @0x7CD870
//   cdGunSightSpecularShader::AddNode @0x7CD8A0
//   cdGunSightSpecularShaderNode::Render @0x7CD910
// ============================================================================
#ifndef COD3_RENDER_CDGUNSIGHTSPECULARSHADER_H
#define COD3_RENDER_CDGUNSIGHTSPECULARSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdGunSightSpecularShaderMat — gun sight specular shader material (36 bytes)
// ============================================================================
struct cdGunSightSpecularShaderMat : nglMaterial {
    nglTexture* mDiffuseTexture;   // +0x10
    nglTexture* mSpecularTexture;  // +0x14
    float       mSpecularPower;    // +0x18
    float       mSpecularLevel;    // +0x1C
    int         mCullMode;         // +0x20

    cdGunSightSpecularShaderMat(nglTexture* iDiffuseTexture, nglTexture* iSpecularTexture);  // @0x7CD770
};
static_assert(sizeof(cdGunSightSpecularShaderMat) == 0x24, "cdGunSightSpecularShaderMat size mismatch");

// ============================================================================
// cdGunSightSpecularShaderNode — gun sight specular shader render node (24 bytes)
// ============================================================================
struct cdGunSightSpecularShaderNode : nglShaderNode {
    cdGunSightSpecularShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdGunSightSpecularShaderNode) == 0x18, "cdGunSightSpecularShaderNode size mismatch");

// ============================================================================
// cdGunSightSpecularShader — gun sight specular shader (16 bytes)
// ============================================================================
class cdGunSightSpecularShader : public nglShader {
public:
    virtual void Register();  // @0x7CD870
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7CD8A0
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdGunSightSpecularShader) == 0x10, "cdGunSightSpecularShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdGunSightSpecularShaderVertex.o)
// ============================================================================
namespace cdGunSightSpecularRender {
    extern unsigned long VS[2];                   // ?VS@cdGunSightSpecularRender@@3PAKA
    extern unsigned int const* VShaderTable[2];    // ?VShaderTable@cdGunSightSpecularRender@@3PAPBIA
}
namespace cdGunSightSpecularPixel {
    extern unsigned long* PS[2];                   // ?PS@cdGunSightSpecularPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];     // ?PShaderTable@cdGunSightSpecularPixel@@3PAPBIA
}
namespace cdGunSightSpecularFullbrightPixel {
    extern unsigned long* PS[2];                   // ?PS@cdGunSightSpecularFullbrightPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];     // ?PShaderTable@cdGunSightSpecularFullbrightPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdGunSightSpecularShader* gCDGunSightSpecularShader;  // @0x10DE4C8

void InitCDGunSightSpecularShader();   // @0x7CD800
void ToggleCDGunSightSpecularShader(); // @0x7CD850

#endif // COD3_RENDER_CDGUNSIGHTSPECULARSHADER_H
