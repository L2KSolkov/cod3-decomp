// ============================================================================
// cdGlassShader.h — glass shader (4 non-inline funcs).
// Source: source/cdGlassShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGlassShader.o):
//   InitCDGlassShader  @0x7CFF90
//   ToggleCDGlassShader @0x7CFFE0
//   cdGlassShader::Register @0x7D0000
//   cdGlassShader::AddNode @0x7D0970 (inline COMDAT)
//   cdGlassShaderNode::Render @0x7D0000...
// ============================================================================
#ifndef COD3_RENDER_CDGLASSSHADER_H
#define COD3_RENDER_CDGLASSSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdGlassShaderMat — glass shader material (36 bytes)
// ============================================================================
struct cdGlassShaderMat : nglMaterial {
    nglTexture* mDiffuse;     // +0x10
    nglTexture* mEnvironment; // +0x14
    int         mCullMode;    // +0x18
    float       mAlpha;       // +0x1C
    int         mFlags;       // +0x20
};
static_assert(sizeof(cdGlassShaderMat) == 0x24, "cdGlassShaderMat size mismatch");

// ============================================================================
// cdGlassShaderNode — glass shader render node (24 bytes)
// ============================================================================
struct cdGlassShaderNode : nglShaderNode {
    cdGlassShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdGlassShaderNode) == 0x18, "cdGlassShaderNode size mismatch");

// ============================================================================
// cdGlassShader — glass shader (16 bytes)
// ============================================================================
class cdGlassShader : public nglShader {
public:
    virtual void Register();  // @0x7D0000
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D0970
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdGlassShader) == 0x10, "cdGlassShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdGlassShaderVertex.o)
// ============================================================================
namespace cdGlassRender {
    extern unsigned long VS[2][2];                  // ?VS@cdGlassRender@@3PAY01KA
    extern unsigned int const* VShaderTable[2][2];   // ?VShaderTable@cdGlassRender@@3PAY01PBIA
}
namespace cdGlassPixel {
    extern unsigned long* PS[2];                     // ?PS@cdGlassPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];       // ?PShaderTable@cdGlassPixel@@3PAPBIA
}
namespace cdGlassSolidColorPixel {
    extern unsigned long* PS[2];                     // ?PS@cdGlassSolidColorPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];       // ?PShaderTable@cdGlassSolidColorPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern nglTexture* gProjShadowTex;  // ?gProjShadowTex@@3PAUnglTexture@@A (render.o)

extern cdGlassShader* gCDGlassShader;  // @0x10DE538

void InitCDGlassShader();   // @0x7CFF90
void ToggleCDGlassShader(); // @0x7CFFE0

#endif // COD3_RENDER_CDGLASSSHADER_H
