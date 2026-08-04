// ============================================================================
// cdFlagShader.h — flag shader (7 non-inline funcs).
// Source: source/cdFlagShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdFlagShader.o):
//   cdFlagShaderMat::ctor @0x7C9FF0
//   InitCDFlagShader  @0x7CA080
//   ToggleCDFlagShader @0x7CA160
//   CalculateFlagMatrix @0x7CA180
//   cdFlagShader::Register @0x7CA3C0
//   cdFlagShader::AddNode @0x7CA400
//   cdFlagShaderNode::Render @0x7CA470
// ============================================================================
#ifndef COD3_RENDER_CDFLAGSHADER_H
#define COD3_RENDER_CDFLAGSHADER_H

#include "cdSimpleColorShader.h"
#include "core/math_types.h"
#include <math.h>

// ============================================================================
// cdFlagShaderMat — flag shader material (24 bytes)
// ============================================================================
struct cdFlagShaderMat : nglMaterial {
    nglTexture* mTexture;   // +0x10
    int         mCullMode;  // +0x14

    cdFlagShaderMat(nglTexture* iTexture);  // @0x7C9FF0
};
static_assert(sizeof(cdFlagShaderMat) == 0x18, "cdFlagShaderMat size mismatch");

// ============================================================================
// cdFlagShaderNode — flag shader render node (24 bytes)
// ============================================================================
struct cdFlagShaderNode : nglShaderNode {
    cdFlagShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdFlagShaderNode) == 0x18, "cdFlagShaderNode size mismatch");

// ============================================================================
// cdFlagShader — flag shader (16 bytes)
// ============================================================================
class cdFlagShader : public nglShader {
public:
    virtual void Register();  // @0x7CA3C0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7CA400
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdFlagShader) == 0x10, "cdFlagShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdFlagShaderVertex.o)
// ============================================================================
namespace cdFlagVertex {
    extern unsigned long* VS;                // ?VS@cdFlagVertex@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdFlagVertex@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdFlagVertex@@3KA
}
namespace cdFlagPixel {
    extern unsigned long** PS;               // ?PS@cdFlagPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdFlagPixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdFlagPixel@@3PAKA
}

// ============================================================================
// Globals (owned by this object / shared with ShaderCommon)
// ============================================================================
extern math::Vector4 FlagSinTable[64];  // ?FlagSinTable@@3PAVVector4@math@@A @0x14CD5A0
extern unsigned int gShaderSwitchingFlags;  // @0x10DDB14 (dword after ShaderCommon::ShaderSwitching)

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdFlagShader* gCDFlagShader;  // @0x10DE094

void InitCDFlagShader();   // @0x7CA080
void ToggleCDFlagShader(); // @0x7CA160

#endif // COD3_RENDER_CDFLAGSHADER_H
