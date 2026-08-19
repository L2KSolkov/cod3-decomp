// ============================================================================
// cdWorldShader.h — world shader (5 non-inline funcs).
// Source: source/cdWorldShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdWorldShader.o):
//   InitCDWorldShader  @0x7DF0D0
//   ToggleCDWorldShader @0x7DF120
//   cdWorldShader::Register @0x7DF140
//   cdWorldShader::AddNode @0x7DF180
//   cdWorldShaderNode::Render @0x7DF220
// ============================================================================
#ifndef COD3_RENDER_CDWORLDSHADER_H
#define COD3_RENDER_CDWORLDSHADER_H

#include "cdWorldColorShader.h"  // cdWorldShaderNode (Clip + hasColorVerts)

// ============================================================================
// cdWorldShader — world shader (16 bytes)
// ============================================================================
class cdWorldShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7E01C0
    virtual void Register();  // @0x7DF140
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7DF180
};
static_assert(sizeof(cdWorldShader) == 0x10, "cdWorldShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdWorldShaderVertex.o)
// ============================================================================
namespace cdWorldRender {
    extern unsigned long VS[4][2];                       // ?VS@cdWorldRender@@3PAY01KA
    extern unsigned int const* VShaderTable[4][2];        // ?VShaderTable@cdWorldRender@@3PAY01PBIA
}
namespace cdWorldProjectedRender {
    extern unsigned long VS[2];                           // ?VS@cdWorldProjectedRender@@3PAKA
    extern unsigned int const* VShaderTable[2];            // ?VShaderTable@cdWorldProjectedRender@@3PAPBIA
}
namespace cdWorldPixel {
    extern unsigned long* PS[2][2][2];                    // ?PS@cdWorldPixel@@3PAY111PAKA
    extern unsigned int const* PShaderTable[2][2][2];     // ?PShaderTable@cdWorldPixel@@3PAY111PBIA
}
namespace cdWorldProjectedPixel {
    extern unsigned long* PS[2];                          // ?PS@cdWorldProjectedPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];            // ?PShaderTable@cdWorldProjectedPixel@@3PAPBIA
}
namespace cdWorldSolidColorPixel {
    extern unsigned long* PS[2];                          // ?PS@cdWorldSolidColorPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2];            // ?PShaderTable@cdWorldSolidColorPixel@@3PAPBIA
    extern unsigned long* Shader;                         // ?Shader@cdWorldSolidColorPixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdWorldShader* gCDWorldShader;  // @0x10DE5E4

void InitCDWorldShader();  // @0x7DF0D0
void ToggleCDWorldShader();          // @0x7DF120

#endif // COD3_RENDER_CDWORLDSHADER_H
