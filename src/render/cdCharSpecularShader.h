// ============================================================================
// cdCharSpecularShader.h — char specular shader (5 non-inline funcs).
// Source: source/cdCharSpecularShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdCharSpecularShader.o):
//   InitCDCharSpecularShader  @0x7D2130
//   ToggleCDCharSpecularShader @0x7D2180
//   cdCharSpecularShader::Register @0x7D21A0
//   cdCharSpecularShader::AddNode @0x7D21F0
//   cdCharSpecularShaderNode::Render @0x7D2260
// ============================================================================
#ifndef COD3_RENDER_CDCHARSPECULARSHADER_H
#define COD3_RENDER_CDCHARSPECULARSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdCharSpecularShaderMat — char specular shader material (32 bytes)
// ============================================================================
class cdCharSpecularShaderMat : public nglMaterial {
public:
    nglTexture* mDiffuse;       // +0x10
    nglTexture* mSpecularMask;  // +0x14
    float       mSpecularPower; // +0x18
    float       mSpecularLevel; // +0x1C
};
static_assert(sizeof(cdCharSpecularShaderMat) == 0x20, "cdCharSpecularShaderMat size mismatch");

// ============================================================================
// cdCharSpecularShaderNode — char specular shader render node (24 bytes)
// ============================================================================
struct cdCharSpecularShaderNode : nglShaderNode {
    cdCharSpecularShaderMat* mMaterial;  // +0x14

    cdCharSpecularShaderNode(nglMeshNode* iMeshNode,
                             nglMeshSection* iSection,
                             cdCharSpecularShaderMat* iMaterial); // @0x7D27E0
    virtual ~cdCharSpecularShaderNode(); // @0x7D2840
};
static_assert(sizeof(cdCharSpecularShaderNode) == 0x18, "cdCharSpecularShaderNode size mismatch");

// ============================================================================
// cdCharSpecularShader — char specular shader (16 bytes)
// ============================================================================
class cdCharSpecularShader : public nglShader {
public:
    cdCharSpecularShader(); // @0x7D26A0
    virtual ~cdCharSpecularShader(); // @0x7D2880
    virtual tlFixedString GetName(); // @0x7D26D0
    virtual void Register();  // @0x7D21A0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D21F0
};
static_assert(sizeof(cdCharSpecularShader) == 0x10, "cdCharSpecularShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdCharSpecularShaderVertex.o)
// ============================================================================
namespace cdCharSpecularShaderRender {
    extern unsigned long VS[2];                    // ?VS@cdCharSpecularShaderRender@@3PAKA
    extern unsigned int const* VShaderTable[2];     // ?VShaderTable@cdCharSpecularShaderRender@@3PAPBIA
    void RegisterShader();                          // @0x007D26F0
    void RegisterVShader();                         // @0x007D2720
    unsigned long GetVShader(unsigned int index);   // @0x007D2730
}
namespace cdCharSpecularPixel {
    extern unsigned long* PS[2];                    // ?PS@cdCharSpecularPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];      // ?PShaderTable@cdCharSpecularPixel@@3PAPBIA
    extern unsigned long* Shader;                   // ?Shader@cdCharSpecularPixel@@3PAKA
    void RegisterShader();                           // @0x007D2740
    void RegisterPShader();                          // @0x007D2760
    unsigned long* GetPShader();                     // @0x007D2780
}
namespace cdCharSpecularFullbrightPixel {
    extern unsigned long* PS[2];                    // ?PS@cdCharSpecularFullbrightPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];      // ?PShaderTable@cdCharSpecularFullbrightPixel@@3PAPBIA
    extern unsigned long* Shader;                   // ?Shader@cdCharSpecularFullbrightPixel@@3PAKA
    void RegisterShader();                           // @0x007D2790
    void RegisterPShader();                          // @0x007D27B0
    unsigned long* GetPShader();                    // @0x007D27D0
}

struct CharSpecularContext {
    CharSpecularContext(); // @0x007D2890
};

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdCharSpecularShader* gcdCharSpecularShader;  // @0x10DE554

void InitCDCharSpecularShader();   // @0x7D2130
void ToggleCDCharSpecularShader(); // @0x7D2180

#endif // COD3_RENDER_CDCHARSPECULARSHADER_H
