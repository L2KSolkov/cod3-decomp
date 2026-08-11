// ============================================================================
// cdWaterShader.h — water shader (6 non-inline funcs).
// Source: source/cdWaterShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWaterShader.o):
//   cdWaterShaderMat::ctor @0x7D95E0
//   InitCDWaterShader  @0x7D9670
//   ToggleCDWaterShader @0x7D96C0
//   cdWaterShader::Register @0x7D96E0
//   cdWaterShader::AddNode @0x7D9720
//   cdWaterShaderNode::Render @0x7D9790
// ============================================================================
#ifndef COD3_RENDER_CDWATERSHADER_H
#define COD3_RENDER_CDWATERSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdWaterShaderMat — water shader material (36 bytes)
// ============================================================================
struct cdWaterShaderMat : nglMaterial {
    nglTexture* mDiffuse;    // +0x10
    nglTexture* mLightmap;   // +0x14
    float       mUScroll;    // +0x18
    float       mVScroll;    // +0x1C
    int         mDrawOrder;  // +0x20

    cdWaterShaderMat(nglTexture* iTexture);  // @0x7D95E0
};
static_assert(sizeof(cdWaterShaderMat) == 0x24, "cdWaterShaderMat size mismatch");

// ============================================================================
// cdWaterShaderNode — water shader render node (24 bytes)
// ============================================================================
struct cdWaterShaderNode : nglShaderNode {
    cdWaterShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdWaterShaderNode) == 0x18, "cdWaterShaderNode size mismatch");

// ============================================================================
// cdWaterShader — water shader (16 bytes)
// ============================================================================
class cdWaterShader : public nglShader {
public:
    virtual void Register();  // @0x7D96E0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D9720
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdWaterShader) == 0x10, "cdWaterShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdWaterShaderVertex.o)
// ============================================================================
namespace cdWaterRender {
    extern unsigned long* VS;                // ?VS@cdWaterRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdWaterRender@@3PAPBIA
}
namespace cdWaterPixel {
    extern unsigned long** PS;               // ?PS@cdWaterPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdWaterPixel@@3PAPBIA
}
namespace cdWaterFullbrightPixel {
    extern unsigned long** PS;               // ?PS@cdWaterFullbrightPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdWaterFullbrightPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdWaterShader* gCDWaterShader;  // @0x10DE5A4

void InitCDWaterShader();   // @0x7D9670
void ToggleCDWaterShader(); // @0x7D96C0

#endif // COD3_RENDER_CDWATERSHADER_H
