// ============================================================================
// cdDynamicDecalShader.h — dynamic decal shader (6 non-inline funcs).
// Source: source/cdDynamicDecalShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDynamicDecalShader.o):
//   cdDynamicDecalShaderMat::ctor @0x7CBD80
//   InitCDDynamicDecalShader  @0x7CBE10
//   ToggleCDDynamicDecalShader @0x7CBE60
//   cdDynamicDecalShader::Register @0x7CBE80
//   cdDynamicDecalShader::AddNode @0x7CBE90
//   cdDynamicDecalShaderNode::Render @0x7CBF10
// ============================================================================
#ifndef COD3_RENDER_CDDYNAMICDECALSHADER_H
#define COD3_RENDER_CDDYNAMICDECALSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdDynamicDecalShaderMat — dynamic decal shader material (28 bytes)
// ============================================================================
struct cdDynamicDecalShaderMat : nglMaterial {
    nglTexture* mTexture;      // +0x10
    float       mZbias;        // +0x14
    bool        mAlphaBlend;   // +0x18

    cdDynamicDecalShaderMat();  // @0x7CBD80
};
static_assert(sizeof(cdDynamicDecalShaderMat) == 0x1C, "cdDynamicDecalShaderMat size mismatch");

// ============================================================================
// cdDynamicDecalShaderNode — dynamic decal shader render node (28 bytes)
// ============================================================================
struct cdDynamicDecalShaderNode : nglShaderNode {
    cdDynamicDecalShaderMat* mMaterial;  // +0x14
    int                     Clip;       // +0x18
};
static_assert(sizeof(cdDynamicDecalShaderNode) == 0x1C, "cdDynamicDecalShaderNode size mismatch");

// ============================================================================
// cdDynamicDecalShader — dynamic decal shader (16 bytes)
// ============================================================================
class cdDynamicDecalShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7CD290
    virtual void Register();  // @0x7CBE80
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7CBE90
};
static_assert(sizeof(cdDynamicDecalShader) == 0x10, "cdDynamicDecalShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdDynamicDecalShaderVertex.o)
// ============================================================================
namespace cdDynamicDecalRender {
    extern unsigned long VS[2];                    // ?VS@cdDynamicDecalRender@@3PAKA
    extern unsigned int const* VShaderTable[2];     // ?VShaderTable@cdDynamicDecalRender@@3PAPBIA
}
namespace cdDynamicDecalPixel {
    extern unsigned long* PS[2];                    // ?PS@cdDynamicDecalPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];      // ?PShaderTable@cdDynamicDecalPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern unsigned int gShaderSwitchingFlags;  // @0x10DDB14 (dword after ShaderCommon::ShaderSwitching)
extern cdDynamicDecalShader* gCDDynamicDecalShader;  // @0x10DE4A4

void InitCDDynamicDecalShader();   // @0x7CBE10
void ToggleCDDynamicDecalShader(); // @0x7CBE60

#endif // COD3_RENDER_CDDYNAMICDECALSHADER_H
