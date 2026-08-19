// ============================================================================
// cdSimpleShader.h — simple shader (12 non-inline funcs).
// Source: source/cdSimpleShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleShader.o):
//   cdSimpleShaderMat::ctor @0x7D6390
//   InitCDSimpleShader  @0x7D6420
//   ToggleCDSimpleShader @0x7D6470
//   cdSimpleShader::Register @0x7D6490
//   cdSimpleShader::AddNode @0x7D64C0
//   cdSimpleShaderNode::Render @0x7D6540
//   cdSimpleRender::RegisterVShader @0x7D6910
//   cdSimpleRender::GetVShader @0x7D6940
//   cdSimplePixel::RegisterPShader @0x7D6950
//   cdSimplePixel::GetPShader @0x7D6970
//   cdSimpleFullbrightPixel::RegisterPShader @0x7D6980
//   cdSimpleFullbrightPixel::GetPShader @0x7D69A0
// ============================================================================
#ifndef COD3_RENDER_CDSIMPLESHADER_H
#define COD3_RENDER_CDSIMPLESHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdSimpleShaderMat — simple shader material (24 bytes)
// ============================================================================
struct cdSimpleShaderMat : nglMaterial {
    nglTexture* mTexture;   // +0x10
    int         mCullMode;  // +0x14

    cdSimpleShaderMat(nglTexture* iTexture);  // @0x7D6390
};
static_assert(sizeof(cdSimpleShaderMat) == 0x18, "cdSimpleShaderMat size mismatch");

// ============================================================================
// cdSimpleShaderNode is defined in cdSimpleColorShader.h
// ============================================================================

// ============================================================================
// cdSimpleShader — simple shader (16 bytes)
// ============================================================================
class cdSimpleShader : public nglShader {
public:
    virtual void Register();  // @0x7D6490
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D64C0
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdSimpleShader) == 0x10, "cdSimpleShader size mismatch");

// Shader data tables are the exact IDA-declared objects.  Their Xbox
// microcode payloads remain null until the Phase 6 shader translation pass.
namespace cdSimpleRender {
    extern unsigned int VS[2];
    extern const unsigned int* VShaderTable[2];
    void RegisterVShader();  // @0x7D6910
    unsigned int GetVShader(unsigned int index);  // @0x7D6940
}
namespace cdSimplePixel {
    extern unsigned int* PS[1];
    extern const unsigned int* PShaderTable[1];
    void RegisterPShader();  // @0x7D6950
    unsigned int* GetPShader();  // @0x7D6970
}
namespace cdSimpleFullbrightPixel {
    extern unsigned int* PS[1];
    extern const unsigned int* PShaderTable[1];
    void RegisterPShader();  // @0x7D6980
    unsigned int* GetPShader();  // @0x7D69A0
}

extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);

// ============================================================================
// Externs
// ============================================================================
extern cdSimpleShader* gCDSimpleShader;

void InitCDSimpleShader();  // @0x7D6420
void ToggleCDSimpleShader();           // @0x7D6470

#endif // COD3_RENDER_CDSIMPLESHADER_H
