// ============================================================================
// cdWheelMarkShader.h — wheel mark shader (7 non-inline funcs).
// Source: source/cdWheelMarkShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWheelMarkShader.o):
//   cdWheelMarkShaderMat::ctor @0x7C92B0
//   InitCDWheelMarkShader  @0x7C9330
//   ToggleCDWheelMarkShader @0x7C9380 (empty)
//   InitCDWheelMarkVertexDefBuilder @0x7C9390
//   cdWheelMarkShader::Register @0x7C93D0
//   cdWheelMarkShader::AddNode @0x7C9410
//   cdWheelMarkShaderNode::Render @0x7C9480
// ============================================================================
#ifndef COD3_RENDER_CDWHEELMARKSHADER_H
#define COD3_RENDER_CDWHEELMARKSHADER_H

#include "cdSimpleColorShader.h"
#include "ngl/ngl_dx_gpu.h"  // gpuVertexFormat, gpuCreateVertexFormat

// ============================================================================
// cdWheelMarkShaderMat — wheel mark shader material (20 bytes)
// ============================================================================
struct cdWheelMarkShaderMat : nglMaterial {
    nglTexture* mTexture;  // +0x10

    cdWheelMarkShaderMat();  // @0x7C92B0
};
static_assert(sizeof(cdWheelMarkShaderMat) == 0x14, "cdWheelMarkShaderMat size mismatch");

// ============================================================================
// cdWheelMarkShaderNode — wheel mark shader render node (28 bytes)
// ============================================================================
struct cdWheelMarkShaderNode : nglShaderNode {
    cdWheelMarkShaderMat* mMaterial;  // +0x14
    int                   Clip;       // +0x18
};
static_assert(sizeof(cdWheelMarkShaderNode) == 0x1C, "cdWheelMarkShaderNode size mismatch");

// ============================================================================
// cdWheelMarkShader — wheel mark shader (16 bytes)
// ============================================================================
class cdWheelMarkShader : public nglShader {
public:
    virtual void Register();  // @0x7C93D0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7C9410
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdWheelMarkShader) == 0x10, "cdWheelMarkShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdWheelMarkShaderVertex.o)
// ============================================================================
namespace cdWheelMarkShaderVertex {
    extern unsigned long* VS;                // ?VS@cdWheelMarkShaderVertex@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdWheelMarkShaderVertex@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdWheelMarkShaderVertex@@3KA
}
namespace cdWheelMarkShaderPixel {
    extern unsigned long** PS;               // ?PS@cdWheelMarkShaderPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdWheelMarkShaderPixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdWheelMarkShaderPixel@@3PAKA
}

// ============================================================================
// Vertex element externs (data in render_xboxr:cdWheelMarkShaderVertex.o)
// ============================================================================
extern const _D3DVERTEXSHADERINPUT cdWheelMarkVertexElements[];  // @0xE3C7A0

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern gpuVertexFormat cdWheelMarkVertexFormat;  // ?cdWheelMarkVertexFormat@@3UgpuVertexFormat@@A @0x14CD564
extern cdWheelMarkShader* gCDWheelMarkShader;    // @0x10DE074

cdWheelMarkShader* InitCDWheelMarkShader();        // @0x7C9330
char ToggleCDWheelMarkShader();                    // @0x7C9380 (empty)
_D3DVERTEXATTRIBUTEFORMAT* InitCDWheelMarkVertexDefBuilder();  // @0x7C9390

#endif // COD3_RENDER_CDWHEELMARKSHADER_H
