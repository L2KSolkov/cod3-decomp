// ============================================================================
// cdSimpleColorShader.h — simple color shader (4 non-inline funcs).
// Source: source/cdSimpleColorShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleColorShader.o):
//   InitCDSimpleColorShader @0x7D5F10
//   ToggleCDSimpleColorShader @0x7D5F60
//   cdSimpleColorShader::Register @0x7D5F80
//   cdSimpleColorShader::AddNode @0x7D5F90
// ============================================================================
#ifndef COD3_RENDER_CDSIMPLECOLORSHADER_H
#define COD3_RENDER_CDSIMPLECOLORSHADER_H

#include "cdDebugVertexDef.h"  // tlInitList, gpuVertexFormat
#include "d3d8.h"
#include "ngl/nglScene.h"
#include "aeps/apsRenderNode.h"  // nglRenderNode
#include "core/tlFixedString.h"

// ============================================================================
// Forward declarations
// ============================================================================
struct nglMeshNode;
struct nglMeshSection;
struct nglMaterial;
struct cdSimpleShaderMat;

// ============================================================================
// nglShaderNode — shader render node (20 bytes)
// ============================================================================
struct nglShaderNode : nglRenderNode {
    nglMeshNode*     MeshNode;  // +0x0C
    nglMeshSection*  Section;   // +0x10
};
static_assert(sizeof(nglShaderNode) == 0x14, "nglShaderNode size mismatch");

// ============================================================================
// nglMaterial — material base (16 bytes, verified against IDA)
// ============================================================================
struct nglMaterial {
    tlFixedString* Name;          // +0x00
    nglShader*     Shader;        // +0x04
    int            BinaryVersion; // +0x08
    void*          RuntimeData;   // +0x0C
};
static_assert(sizeof(nglMaterial) == 0x10, "nglMaterial size mismatch");

// ============================================================================
// cdSimpleShaderNode — simple shader render node (28 bytes)
// ============================================================================
struct cdSimpleShaderNode : nglShaderNode {
    cdSimpleShaderMat* mMaterial;     // +0x14
    bool               hasColorVerts; // +0x18
};

// ============================================================================
// cdSimpleShaderMat — simple shader material (fwd)
// ============================================================================

// ============================================================================
// nglShader — base shader (16 bytes: tlInitList + Disabled + ID)
// ============================================================================
struct nglShader : tlInitList {
    bool Disabled;  // +0x08
    int  ID;        // +0x0C
};

// ============================================================================
// ShaderCommon::ShaderSwitching — 4-byte union of per-shader toggle flags
// ============================================================================
namespace ShaderCommon {
union ShaderSwitching_t {
    struct {
        unsigned char __s0[4];
    };
};
extern ShaderSwitching_t ShaderSwitching;  // @0x10DDB10
}

// ============================================================================
// cdSimpleColorShader — solid-color shader (16 bytes)
// ============================================================================
class cdSimpleColorShader : public nglShader {
public:
    virtual void Register();  // @0x7D5F80
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D5F90
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdSimpleColorShader) == 0x10, "cdSimpleColorShader size mismatch");

// ============================================================================
// Externs
// ============================================================================
extern void* mem_heap_malloc(unsigned int size);
extern void* nglListAlloc(unsigned Bytes, unsigned Alignment);
extern nglScene* nglBuildScene;
extern cdSimpleColorShader* gCDSimpleColorShader;

cdSimpleColorShader* InitCDSimpleColorShader();  // @0x7D5F10
char ToggleCDSimpleColorShader();                // @0x7D5F60

#endif // COD3_RENDER_CDSIMPLECOLORSHADER_H
