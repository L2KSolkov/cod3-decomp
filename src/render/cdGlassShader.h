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
#include "ngl/nglTexture.h"
#include "ngl/ngl_lighting.h"

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
    virtual tlFixedString GetName(); // @0x7D08F0
    virtual void Register();  // @0x7D0000
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D0970
    virtual void BindMaterial(nglMaterial* Material); // @0x7D0910
};
static_assert(sizeof(cdGlassShader) == 0x10, "cdGlassShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdGlassShaderVertex.o)
// ============================================================================
namespace cdGlassRender {
    struct Params {
        math::Mat44 mLToS;           // +0x00
        math::Mat44 mLToV;           // +0x40
        math::Vector4 mSpecLightDir; // +0x80
        math::Vector4 mEyePos;       // +0x90
        float mAlpha;                // +0xA0
        unsigned char pad_A4[12];    // IDA tail padding to 0xB0.

        Params();                    // @0x7D0930
    };
    static_assert(sizeof(Params) == 0xB0, "cdGlassRender::Params size mismatch");
    template <typename T>
    void SetConstants(const T& params); // @0x7D0950

    extern unsigned long VS[2][2];                  // ?VS@cdGlassRender@@3PAY01KA
    extern unsigned int const* VShaderTable[2][2];   // ?VShaderTable@cdGlassRender@@3PAY01PBIA
    void RegisterVShader();                           // @0x7D0810
    unsigned int GetVShader(unsigned int, unsigned int); // @0x7D0840
}
namespace cdGlassPixel {
    extern unsigned long* PS[2];                     // ?PS@cdGlassPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];       // ?PShaderTable@cdGlassPixel@@3PAPBIA
    void RegisterPShader();                           // @0x7D0860
    unsigned int* GetPShader(unsigned int);            // @0x7D0890
}
namespace cdGlassSolidColorPixel {
    extern unsigned long* PS[2];                     // ?PS@cdGlassSolidColorPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];       // ?PShaderTable@cdGlassSolidColorPixel@@3PAPBIA
    void RegisterPShader();                           // @0x7D08A0
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern nglTexture* gProjShadowTex;  // ?gProjShadowTex@@3PAUnglTexture@@A (render.o)
extern void gpuSetPixelConstant(unsigned int idx, math::Vector4* data,
                                unsigned int nelements); // @0x7D0680

extern cdGlassShader* gCDGlassShader;  // @0x10DE538

void InitCDGlassShader();   // @0x7CFF90
void ToggleCDGlassShader(); // @0x7CFFE0

#endif // COD3_RENDER_CDGLASSSHADER_H
