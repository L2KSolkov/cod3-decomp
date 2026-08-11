// ============================================================================
// cdWorldBlendShader.h — world blend shader (5 non-inline funcs).
// Source: source/cdWorldBlendShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldBlendShader.o):
//   InitCDWorldBlendShader  @0x7DD2A0
//   ToggleCDWorldBlendShader @0x7DD2F0
//   cdWorldBlendShader::Register @0x7DD310
//   cdWorldBlendShader::AddNode @0x7DD350
//   cdWorldBlendShaderNode::Render @0x7DD3E0
// ============================================================================
#ifndef COD3_RENDER_CDWORLDBLENDSHADER_H
#define COD3_RENDER_CDWORLDBLENDSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdWorldBlendShaderMat — world blend shader material (32 bytes)
// ============================================================================
struct cdWorldBlendShaderMat : nglMaterial {
    nglTexture* mDiffuse;   // +0x10
    nglTexture* mBlend;     // +0x14
    nglTexture* mLightmap;  // +0x18
    bool        Translucent; // +0x1C
};
static_assert(sizeof(cdWorldBlendShaderMat) == 0x20, "cdWorldBlendShaderMat size mismatch");

// ============================================================================
// cdWorldBlendShaderNode — world blend shader render node (24 bytes)
// ============================================================================
struct cdWorldBlendShaderNode : nglShaderNode {
    cdWorldBlendShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdWorldBlendShaderNode) == 0x18, "cdWorldBlendShaderNode size mismatch");

// ============================================================================
// cdWorldBlendShader — world blend shader (16 bytes)
// ============================================================================
class cdWorldBlendShader : public nglShader {
public:
    virtual void Register();  // @0x7DD310
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7DD350
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdWorldBlendShader) == 0x10, "cdWorldBlendShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdWorldBlendShaderVertex.o)
// ============================================================================
namespace cdWorldBlendRender {
    extern unsigned long VS[2];                    // ?VS@cdWorldBlendRender@@3PAKA
    extern unsigned int const* VShaderTable[2];     // ?VShaderTable@cdWorldBlendRender@@3PAPBIA
}
namespace cdWorldBlendProjectedRender {
    extern unsigned long VS[2];                    // ?VS@cdWorldBlendProjectedRender@@3PAKA
    extern unsigned int const* VShaderTable[2];     // ?VShaderTable@cdWorldBlendProjectedRender@@3PAPBIA
}
namespace cdWorldBlendPixel {
    extern unsigned long* PS[2][2];                // ?PS@cdWorldBlendPixel@@3PAY01PAKA
    extern unsigned int const* PShaderTable[2][2];  // ?PShaderTable@cdWorldBlendPixel@@3PAY01PBIA
}
namespace cdWorldBlendProjectedPixel {
    extern unsigned long* PS[2];                    // ?PS@cdWorldBlendProjectedPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];      // ?PShaderTable@cdWorldBlendProjectedPixel@@3PAPBIA
}
namespace cdWorldBlendSolidColorPixel {
    extern unsigned long* PS[2];                    // ?PS@cdWorldBlendSolidColorPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];      // ?PShaderTable@cdWorldBlendSolidColorPixel@@3PAPBIA
    extern unsigned long* Shader;                   // ?Shader@cdWorldBlendSolidColorPixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern int cdGetClipResult(const nglMeshSection* Section, const nglMeshNode* MeshNode, nglScene* Scene);

extern cdWorldBlendShader* gCDWorldBlendShader;  // @0x10DE5CC

void InitCDWorldBlendShader();   // @0x7DD2A0
void ToggleCDWorldBlendShader(); // @0x7DD2F0

#endif // COD3_RENDER_CDWORLDBLENDSHADER_H
