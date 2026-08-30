// ============================================================================
// cdDecalShader.h — decal shader (6 non-inline funcs).
// Source: source/cdDecalShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDecalShader.o):
//   cdDecalShaderMat::ctor @0x7D1720
//   InitCDDecalShader  @0x7D17B0
//   ToggleCDDecalShader @0x7D1800
//   cdDecalShader::Register @0x7D1820
//   cdDecalShader::AddNode @0x7D1860
//   cdDecalShaderNode::Render @0x7D18E0
// ============================================================================
#ifndef COD3_RENDER_CDDECALSHADER_H
#define COD3_RENDER_CDDECALSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdDecalShaderMat — decal shader material (28 bytes)
// ============================================================================
class cdDecalShaderMat : public nglMaterial {
public:
    nglTexture* mTexture;   // +0x10
    float       mZBias;     // +0x14
    int         mCullMode;  // +0x18

    cdDecalShaderMat(nglTexture* iTexture);  // @0x7D1720
};
static_assert(sizeof(cdDecalShaderMat) == 0x1C, "cdDecalShaderMat size mismatch");

// ============================================================================
// cdDecalShaderNode — decal shader render node (24 bytes)
// ============================================================================
struct cdDecalShaderNode : nglShaderNode {
    cdDecalShaderMat* mMaterial;  // +0x14

    cdDecalShaderNode(nglMeshNode* iMeshNode,
                      nglMeshSection* iSection,
                      cdDecalShaderMat* iMaterial); // @0x7D1DB0
    virtual ~cdDecalShaderNode(); // @0x7D1E10

    void Render() override;       // @0x7D18E0
};
static_assert(sizeof(cdDecalShaderNode) == 0x18, "cdDecalShaderNode size mismatch");

// ============================================================================
// cdDecalShader — decal shader (16 bytes)
// ============================================================================
class cdDecalShader : public nglShader {
public:
    cdDecalShader(); // @0x7D1CC0
    virtual ~cdDecalShader(); // @0x7D1E50
    virtual tlFixedString GetName(); // @0x7D1CF0
    virtual void Register();  // @0x7D1820
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D1860
};
static_assert(sizeof(cdDecalShader) == 0x10, "cdDecalShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdDecalShaderVertex.o)
// ============================================================================
namespace cdDecalRender {
    extern unsigned long* VS;                       // ?VS@cdDecalRender@@3PAKA
    extern unsigned int const** VShaderTable;        // ?VShaderTable@cdDecalRender@@3PAPBIA
    void RegisterVShader();                           // @0x007D1D10
    unsigned long GetVShader();                       // @0x007D1D30
}
namespace cdDecalPixel {
    extern unsigned long** PS;                       // ?PS@cdDecalPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;         // ?PShaderTable@cdDecalPixel@@3PAPBIA
    void RegisterPShader();                            // @0x007D1D40
    unsigned long* GetPShader();                       // @0x007D1D60
}
namespace cdDecalFullbrightPixel {
    extern unsigned long** PS;                       // ?PS@cdDecalFullbrightPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;         // ?PShaderTable@cdDecalFullbrightPixel@@3PAPBIA
    void RegisterPShader();                            // @0x007D1D70
    unsigned long* GetPShader();                       // @0x007D1D90
}

struct DecalContext {
    DecalContext(); // @0x007D1DA0
};

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdDecalShader* gCDDecalShader;   // @0x14CDA4C
extern cdDecalShader* g_cdDecalShader;  // @0x14CDA48

void InitCDDecalShader();  // @0x7D17B0
void ToggleCDDecalShader();          // @0x7D1800

#endif // COD3_RENDER_CDDECALSHADER_H
