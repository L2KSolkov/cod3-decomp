// ============================================================================
// cdWorldPointLitShader.h — world point-lit shader (5 non-inline funcs).
// Source: source/cdWorldPointLitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldPointLitShader.o):
//   InitCDWorldPointLitShader  @0x7DBB00
//   ToggleCDWorldPointLitShader @0x7DBB50
//   cdWorldPointLitShader::Register @0x7DBB70
//   cdWorldPointLitShader::AddNode @0x7DBBB0
//   cdWorldPointLitShaderNode::Render @0x7DBC60
// ============================================================================
#ifndef COD3_RENDER_CDWORLDPOINTLITSHADER_H
#define COD3_RENDER_CDWORLDPOINTLITSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdWorldPointLitShaderMat — world point-lit shader material (28 bytes)
// ============================================================================
struct cdWorldPointLitShaderMat : nglMaterial {
    nglTexture* mDiffuse;   // +0x10
    nglTexture* mLightmap;  // +0x14
    bool        Translucent; // +0x18
};
static_assert(sizeof(cdWorldPointLitShaderMat) == 0x1C, "cdWorldPointLitShaderMat size mismatch");

// ============================================================================
// cdWorldPointLitShaderNode — world point-lit shader render node (28 bytes)
// ============================================================================
struct cdWorldPointLitShaderNode : nglShaderNode {
    cdWorldPointLitShaderMat* mMaterial;  // +0x14
    int                     Clip;       // +0x18
};
static_assert(sizeof(cdWorldPointLitShaderNode) == 0x1C, "cdWorldPointLitShaderNode size mismatch");

// ============================================================================
// cdWorldPointLitShader — world point-lit shader (16 bytes)
// ============================================================================
class cdWorldPointLitShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7DCED0
    virtual void Register();  // @0x7DBB70
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7DBBB0
};
static_assert(sizeof(cdWorldPointLitShader) == 0x10, "cdWorldPointLitShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdWorldPointLitShaderVertex.o)
// ============================================================================
namespace cdWorldPointLitRender {
    extern unsigned long VS[2];                    // ?VS@cdWorldPointLitRender@@3PAKA
    extern unsigned int const* VShaderTable[2];     // ?VShaderTable@cdWorldPointLitRender@@3PAPBIA
}
namespace cdWorldPointLitProjectedRender {
    extern unsigned long VS[2];                    // ?VS@cdWorldPointLitProjectedRender@@3PAKA
    extern unsigned int const* VShaderTable[2];     // ?VShaderTable@cdWorldPointLitProjectedRender@@3PAPBIA
}
namespace cdWorldPointLitPixel {
    extern unsigned long* PS[2][2];                // ?PS@cdWorldPointLitPixel@@3PAY01PAKA
    extern unsigned int const* PShaderTable[2][2];  // ?PShaderTable@cdWorldPointLitPixel@@3PAY01PBIA
}
namespace cdWorldPointLitProjectedPixel {
    extern unsigned long* PS[2];                    // ?PS@cdWorldPointLitProjectedPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];      // ?PShaderTable@cdWorldPointLitProjectedPixel@@3PAPBIA
}
namespace cdWorldPointLitSolidColorPixel {
    extern unsigned long* PS[2];                    // ?PS@cdWorldPointLitSolidColorPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];      // ?PShaderTable@cdWorldPointLitSolidColorPixel@@3PAPBIA
    extern unsigned long* Shader;                   // ?Shader@cdWorldPointLitSolidColorPixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern int cdGetClipResult(const nglMeshSection* Section, const nglMeshNode* MeshNode, nglScene* Scene);
extern nglTexture* nglDefaultTex;  // ?nglDefaultTex@@3PAUnglTexture@@A (ngl_texture.o)

extern cdWorldPointLitShader* gCDWorldPointLitShader;  // @0x10DE5C0

void InitCDWorldPointLitShader();   // @0x7DBB00
void ToggleCDWorldPointLitShader(); // @0x7DBB50

#endif // COD3_RENDER_CDWORLDPOINTLITSHADER_H
