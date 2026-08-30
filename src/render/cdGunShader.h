// ============================================================================
// cdGunShader.h — gun shader (6 non-inline funcs).
// Source: source/cdGunShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGunShader.o):
//   cdGunShaderMat::ctor @0x7CEA70
//   InitCDGunShader  @0x7CEB00
//   ToggleCDGunShader @0x7CEB50
//   cdGunShader::Register @0x7CEB70
//   cdGunShader::AddNode @0x7CEBD0
//   cdGunShaderNode::Render @0x7CEC40
// ============================================================================
#ifndef COD3_RENDER_CDGUNSHADER_H
#define COD3_RENDER_CDGUNSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdGunShaderMat — gun shader material (24 bytes)
// ============================================================================
class cdGunShaderMat : public nglMaterial {
public:
    nglTexture* mTexture;   // +0x10
    int         mCullMode;  // +0x14

    cdGunShaderMat(nglTexture* iTexture);  // @0x7CEA70
};
static_assert(sizeof(cdGunShaderMat) == 0x18, "cdGunShaderMat size mismatch");

// ============================================================================
// cdGunShaderNode — gun shader render node (24 bytes)
// ============================================================================
struct cdGunShaderNode : nglShaderNode {
    cdGunShaderMat* mMaterial;  // +0x14

    cdGunShaderNode(nglMeshNode* iMeshNode,
                    nglMeshSection* iSection,
                    cdGunShaderMat* iMaterial); // @0x7CF0D0
    virtual ~cdGunShaderNode(); // @0x7CF130
};
static_assert(sizeof(cdGunShaderNode) == 0x18, "cdGunShaderNode size mismatch");

// ============================================================================
// cdGunShader — gun shader (16 bytes)
// ============================================================================
class cdGunShader : public nglShader {
public:
    cdGunShader(); // @0x7CEF90
    virtual ~cdGunShader(); // @0x7CF170
    virtual tlFixedString GetName(); // @0x7CEFC0
    virtual void Register();  // @0x7CEB70
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7CEBD0
};
static_assert(sizeof(cdGunShader) == 0x10, "cdGunShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdGunShaderVertex.o)
// ============================================================================
namespace cdGunRender {
    extern unsigned long* VS;                 // ?VS@cdGunRender@@3PAKA
    extern unsigned int const** VShaderTable;  // ?VShaderTable@cdGunRender@@3PAPBIA
    extern unsigned long Shader;              // ?Shader@cdGunRender@@3KA
    void RegisterVShader();                   // @0x007CF000
    unsigned long GetVShader();               // @0x007CF020
}
namespace cdGunPixel {
    extern unsigned long** PS;                // ?PS@cdGunPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;  // ?PShaderTable@cdGunPixel@@3PAPBIA
    extern unsigned long* Shader;             // ?Shader@cdGunPixel@@3PAKA
    void RegisterShader();                    // @0x007CF030
    void RegisterPShader();                   // @0x007CF050
    unsigned long* GetPShader();              // @0x007CF070
}
namespace cdGunFullbrightPixel {
    extern unsigned long** PS;                // ?PS@cdGunFullbrightPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;  // ?PShaderTable@cdGunFullbrightPixel@@3PAPBIA
    extern unsigned long* Shader;             // ?Shader@cdGunFullbrightPixel@@3PAKA
    void RegisterShader();                    // @0x007CF080
    void RegisterPShader();                   // @0x007CF0A0
    unsigned long* GetPShader();              // @0x007CF0C0
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdGunShader* gCDGunShader;  // @0x10DE4D8

void InitCDGunShader();  // @0x7CEB00
void ToggleCDGunShader();        // @0x7CEB50

#endif // COD3_RENDER_CDGUNSHADER_H
