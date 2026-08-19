// ============================================================================
// cdWorldBlendPointLitShader.h — world blend point-lit shader (5 non-inline).
// Source: source/cdWorldBlendPointLitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldBlendPointLitShader.o):
//   InitCDWorldBlendPointLitShader  @0x7DA260
//   ToggleCDWorldBlendPointLitShader @0x7DA2B0
//   cdWorldBlendPointLitShader::Register @0x7DA2D0
//   cdWorldBlendPointLitShader::AddNode @0x7DA310
//   cdWorldBlendPointLitShaderNode::Render @0x7DA3A0
// ============================================================================
#ifndef COD3_RENDER_CDWORLDBLENDPOINTLITSHADER_H
#define COD3_RENDER_CDWORLDBLENDPOINTLITSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdWorldBlendPointLitShaderMat — world blend point-lit material (32 bytes)
// ============================================================================
struct cdWorldBlendPointLitShaderMat : nglMaterial {
    nglTexture* mDiffuse;    // +0x10
    nglTexture* mBlend;      // +0x14
    nglTexture* mLightmap;   // +0x18
    bool        Translucent; // +0x1C
};
static_assert(sizeof(cdWorldBlendPointLitShaderMat) == 0x20, "cdWorldBlendPointLitShaderMat size mismatch");

// ============================================================================
// cdWorldBlendPointLitShaderNode — world blend point-lit render node (24 bytes)
// ============================================================================
struct cdWorldBlendPointLitShaderNode : nglShaderNode {
    cdWorldBlendPointLitShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdWorldBlendPointLitShaderNode) == 0x18, "cdWorldBlendPointLitShaderNode size mismatch");

// ============================================================================
// cdWorldBlendPointLitShader — world blend point-lit shader (16 bytes)
// ============================================================================
class cdWorldBlendPointLitShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7DB730
    virtual void Register();  // @0x7DA2D0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7DA310
};
static_assert(sizeof(cdWorldBlendPointLitShader) == 0x10, "cdWorldBlendPointLitShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdWorldBlendPointLitShaderVertex.o)
// ============================================================================
namespace cdWorldBlendPointLitRender {
    extern unsigned long VS[2];                        // ?VS@cdWorldBlendPointLitRender@@3PAKA
    extern unsigned int const* VShaderTable[2];         // ?VShaderTable@cdWorldBlendPointLitRender@@3PAPBIA
}
namespace cdWorldBlendPointLitProjectedRender {
    extern unsigned long VS[2];                        // ?VS@cdWorldBlendPointLitProjectedRender@@3PAKA
    extern unsigned int const* VShaderTable[2];         // ?VShaderTable@cdWorldBlendPointLitProjectedRender@@3PAPBIA
}
namespace cdWorldBlendPointLitPixel {
    extern unsigned long* PS[2][2];                    // ?PS@cdWorldBlendPointLitPixel@@3PAY01PAKA
    extern unsigned int const* PShaderTable[2][2];      // ?PShaderTable@cdWorldBlendPointLitPixel@@3PAY01PBIA
}
namespace cdWorldBlendPointLitProjectedPixel {
    extern unsigned long* PS[2];                        // ?PS@cdWorldBlendPointLitProjectedPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];          // ?PShaderTable@cdWorldBlendPointLitProjectedPixel@@3PAPBIA
}
namespace cdWorldBlendPointLitSolidColorPixel {
    extern unsigned long* PS[2];                        // ?PS@cdWorldBlendPointLitSolidColorPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];          // ?PShaderTable@cdWorldBlendPointLitSolidColorPixel@@3PAPBIA
    extern unsigned long* Shader;                       // ?Shader@cdWorldBlendPointLitSolidColorPixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern int cdGetClipResult(const nglMeshSection* Section, const nglMeshNode* MeshNode, nglScene* Scene);

extern cdWorldBlendPointLitShader* gCDWorldBlendPointLitShader;  // @0x10DE5B4

void InitCDWorldBlendPointLitShader();   // @0x7DA260
void ToggleCDWorldBlendPointLitShader(); // @0x7DA2B0

#endif // COD3_RENDER_CDWORLDBLENDPOINTLITSHADER_H
