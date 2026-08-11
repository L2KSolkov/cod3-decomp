// ============================================================================
// cdDebugShader.h — debug shader (7 non-inline funcs).
// Source: source/cdDebugShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDebugShader.o):
//   cdDebugShaderMat::ctor @0x7C6360
//   InitCDDebugShader  @0x7C63D0
//   ToggleCDDebugShader @0x7C6420
//   cdDebugShader::Register @0x7C6440
//   cdDebugShaderNode::GetSortInfo @0x7C6470
//   cdDebugShader::AddNode @0x7C6570
//   cdDebugShaderNode::Render @0x7C65C0
// ============================================================================
#ifndef COD3_RENDER_CDDEBUGSHADER_H
#define COD3_RENDER_CDDEBUGSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdDebugShaderMat — debug shader material (16 bytes, base only)
// ============================================================================
struct cdDebugShaderMat : nglMaterial {
    cdDebugShaderMat();  // @0x7C6360
};
static_assert(sizeof(cdDebugShaderMat) == 0x10, "cdDebugShaderMat size mismatch");

// ============================================================================
// cdDebugShaderNode — debug shader render node (24 bytes)
// ============================================================================
struct cdDebugShaderNode : nglShaderNode {
    cdDebugShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdDebugShaderNode) == 0x18, "cdDebugShaderNode size mismatch");

// ============================================================================
// cdDebugShader — debug shader (16 bytes)
// ============================================================================
class cdDebugShader : public nglShader {
public:
    virtual void Register();  // @0x7C6440
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7C6570
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdDebugShader) == 0x10, "cdDebugShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdDebugShaderVertex.o)
// ============================================================================
namespace cdDebugShaderRender {
    extern unsigned long* VS;                // ?VS@cdDebugShaderRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdDebugShaderRender@@3PAPBIA
}
namespace cdDebugPixel {
    extern unsigned long** PS;               // ?PS@cdDebugPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdDebugPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern void nglListAddNode(nglRenderNode* Node);  // ?nglListAddNode@@YAXPAVnglRenderNode@@@Z (unported)

extern cdDebugShader* gCDDebugShader;  // @0x10DE034

void InitCDDebugShader();  // @0x7C63D0
void ToggleCDDebugShader();          // @0x7C6420

#endif // COD3_RENDER_CDDEBUGSHADER_H
