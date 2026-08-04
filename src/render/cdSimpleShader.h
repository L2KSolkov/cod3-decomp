// ============================================================================
// cdSimpleShader.h — simple shader (6 non-inline funcs).
// Source: source/cdSimpleShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleShader.o):
//   cdSimpleShaderMat::ctor @0x7D6390
//   InitCDSimpleShader  @0x7D6420
//   ToggleCDSimpleShader @0x7D6470
//   cdSimpleShader::Register @0x7D6490
//   cdSimpleShader::AddNode @0x7D64C0
//   cdSimpleShaderNode::Render @0x7D6540
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

// ============================================================================
// Externs
// ============================================================================
extern cdSimpleShader* gCDSimpleShader;

void InitCDSimpleShader();  // @0x7D6420
void ToggleCDSimpleShader();           // @0x7D6470

#endif // COD3_RENDER_CDSIMPLESHADER_H
