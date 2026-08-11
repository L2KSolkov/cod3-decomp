// ============================================================================
// cdOceanShader.h — ocean shader (12 non-inline funcs).
// Source: source/cdOceanShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdOceanShader.o):
//   cdOceanShaderMat::ctor @0x7D7180
//   InitCDOceanShader  @0x7D7200
//   ToggleCDOceanShader @0x7D7250
//   cdOceanShaderNode::GetVShader @0x7D7E50
//   cdOceanShaderNode::GetFullbrightPShader @0x7D7E60
//   cdOceanShaderNode::GetPShader @0x7D7E70
//   cdOceanShaderNode::GetVShaderFogConstantOffset @0x7D7E90
//   cdOceanShaderNode::GetVShaderParamsStartAddress @0x7D7EA0
//   cdOceanShader::Register @0x7D7EB0
//   cdOceanShader::AddNode @0x7D7F00
//   cdOceanShaderNode::SetContext @0x7D7F70
//   cdOceanShaderNode::Render @0x7D8D30
// ============================================================================
#ifndef COD3_RENDER_CDOCEANSHADER_H
#define COD3_RENDER_CDOCEANSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdOceanShaderMat::LayerParam — per-layer texture (4 bytes)
// ============================================================================
struct cdOceanShaderMat : nglMaterial {
    struct LayerParam {
        nglTexture* mTexture;  // +0x00
    };
    static_assert(sizeof(LayerParam) == 4, "LayerParam size mismatch");

    unsigned int mFlags;           // +0x10
    LayerParam   mLayers[3];       // +0x14 (12 bytes)
    nglTexture*  mLightmapTexture; // +0x20
    int          mBankID;          // +0x24

    cdOceanShaderMat(nglTexture* iTexture);  // @0x7D7180
};
static_assert(sizeof(cdOceanShaderMat) == 0x28, "cdOceanShaderMat size mismatch");

// ============================================================================
// cdOceanShaderNode — ocean shader render node (24 bytes)
// ============================================================================
struct cdOceanShaderNode : nglShaderNode {
    cdOceanShaderMat* mMaterial;  // +0x14

    virtual unsigned int GetVShader();               // @0x7D7E50
    virtual unsigned long* GetFullbrightPShader();   // @0x7D7E60
    virtual unsigned long* GetPShader(int l2, int l3, int lm);  // @0x7D7E70
    virtual int GetVShaderFogConstantOffset();        // @0x7D7E90
    virtual int GetVShaderParamsStartAddress();       // @0x7D7EA0
};
static_assert(sizeof(cdOceanShaderNode) == 0x18, "cdOceanShaderNode size mismatch");

// ============================================================================
// cdOceanShader — ocean shader (16 bytes)
// ============================================================================
class cdOceanShader : public nglShader {
public:
    virtual void Register();  // @0x7D7EB0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D7F00
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdOceanShader) == 0x10, "cdOceanShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdOceanShaderVertex.o)
// ============================================================================
namespace cdOceanRender {
    extern unsigned long* VS;                // ?VS@cdOceanRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdOceanRender@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdOceanRender@@3KA
}
namespace cdOceanPixel {
    extern unsigned long* PS[2][2][2];                 // ?PS@cdOceanPixel@@3PAY111PAKA
    extern unsigned int const* PShaderTable[2][2][2];   // ?PShaderTable@cdOceanPixel@@3PAY111PBIA
}
namespace cdOceanPixel_Fullbright {
    extern unsigned long* PS[2];               // ?PS@cdOceanPixel_Fullbright@@3PAPAKA
    extern unsigned int const* PShaderTable[2]; // ?PShaderTable@cdOceanPixel_Fullbright@@3PAPBIA
    extern unsigned long* Shader;              // ?Shader@cdOceanPixel_Fullbright@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

namespace cdOceanGlobals {
    void Init();  // ?Init@cdOceanGlobals@@YAXXZ (cdOceanGlobals.o)
}

extern cdOceanShader* gCDOceanShader;  // @0x10DE59C

void InitCDOceanShader();   // @0x7D7200
void ToggleCDOceanShader(); // @0x7D7250

#endif // COD3_RENDER_CDOCEANSHADER_H
