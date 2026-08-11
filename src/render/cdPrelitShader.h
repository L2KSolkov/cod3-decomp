// ============================================================================
// cdPrelitShader.h — prelit shader (5 non-inline funcs).
// Source: source/cdPrelitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdPrelitShader.o):
//   InitCDPrelitShader  @0x7D3590
//   ToggleCDPrelitShader @0x7D35E0
//   cdPrelitShader::Register @0x7D3600
//   cdPrelitShaderNode::Render @0x7D3650
//   cdPrelitShader::AddNode @0x7D3B70
// ============================================================================
#ifndef COD3_RENDER_CDPRELITSHADER_H
#define COD3_RENDER_CDPRELITSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdPrelitShaderMat — prelit shader material (24 bytes)
// ============================================================================
struct cdPrelitShaderMat : nglMaterial {
    nglTexture* mDiffuse;    // +0x10
    nglTexture* mLightmap;   // +0x14
};
static_assert(sizeof(cdPrelitShaderMat) == 0x18, "cdPrelitShaderMat size mismatch");

// ============================================================================
// cdPrelitShaderNode — prelit shader render node (24 bytes)
// ============================================================================
struct cdPrelitShaderNode : nglShaderNode {
    cdPrelitShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdPrelitShaderNode) == 0x18, "cdPrelitShaderNode size mismatch");

// ============================================================================
// cdPrelitShader — prelit shader (16 bytes)
// ============================================================================
class cdPrelitShader : public nglShader {
public:
    virtual void Register();  // @0x7D3600
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D3B70
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdPrelitShader) == 0x10, "cdPrelitShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdPrelitShaderVertex.o)
// ============================================================================
namespace cdPrelitRender {
    extern unsigned long* VS;                 // ?VS@cdPrelitRender@@3PAKA
    extern unsigned int const** VShaderTable;  // ?VShaderTable@cdPrelitRender@@3PAPBIA
}
namespace cdPrelitPixel {
    extern unsigned long** PS;                // ?PS@cdPrelitPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;  // ?PShaderTable@cdPrelitPixel@@3PAPBIA
}
namespace cdPrelitFullbrightPixel {
    extern unsigned long** PS;                // ?PS@cdPrelitFullbrightPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;  // ?PShaderTable@cdPrelitFullbrightPixel@@3PAPBIA
}
namespace cdPrelitSolidColorPixel {
    extern unsigned long** PS;                // ?PS@cdPrelitSolidColorPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;  // ?PShaderTable@cdPrelitSolidColorPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdPrelitShader* gCDPrelitShader;  // @0x10DE564

void InitCDPrelitShader();  // @0x7D3590
void ToggleCDPrelitShader();           // @0x7D35E0

#endif // COD3_RENDER_CDPRELITSHADER_H
