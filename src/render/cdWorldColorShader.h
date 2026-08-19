// ============================================================================
// cdWorldColorShader.h — world color shader (4 non-inline funcs).
// Source: source/cdWorldColorShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldColorShader.o):
//   InitCDWorldColorShader @0x7D9DC0
//   ToggleCDWorldColorShader @0x7D9E10
//   cdWorldColorShader::Register @0x7D9E30
//   cdWorldColorShader::AddNode @0x7D9E40
// ============================================================================
#ifndef COD3_RENDER_CDWORLDCOLORSHADER_H
#define COD3_RENDER_CDWORLDCOLORSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdWorldShaderMat — world shader material (fwd)
// ============================================================================
struct cdWorldShaderMat;

// ============================================================================
// cdWorldShaderNode — world shader render node (32 bytes)
// ============================================================================
struct cdWorldShaderNode : nglShaderNode {
    cdWorldShaderMat* mMaterial;     // +0x14
    int               Clip;          // +0x18
    bool              hasColorVerts; // +0x1C
};
static_assert(sizeof(cdWorldShaderNode) == 0x20, "cdWorldShaderNode size mismatch");

// ============================================================================
// cdWorldColorShader — solid-color world shader (16 bytes)
// ============================================================================
class cdWorldColorShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7D9DA0
    virtual void Register();  // @0x7D9E30
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D9E40
};
static_assert(sizeof(cdWorldColorShader) == 0x10, "cdWorldColorShader size mismatch");

// ============================================================================
// Externs
// ============================================================================
extern cdWorldColorShader* gCDWorldColorShader;
extern int cdGetClipResult(const nglMeshSection* Section, const nglMeshNode* MeshNode,
                           nglScene* Scene);

void InitCDWorldColorShader();  // @0x7D9DC0
void ToggleCDWorldColorShader();               // @0x7D9E10

#endif // COD3_RENDER_CDWORLDCOLORSHADER_H
