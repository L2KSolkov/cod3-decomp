// ============================================================================
// cdHeatHazeShader.h — heat haze shader (4 non-inline funcs).
// Source: source/cdHeatHazeShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdHeatHazeShader.o):
//   ToggleCDHeatHazeShader @0x7CF460
//   InitCDHeatHazeShader  @0x7CF470 (empty)
//   SetupCDHeatHazeShader @0x7CF480
//   RenderCDHeatHazeShader @0x7CF740
// ============================================================================
#ifndef COD3_RENDER_CDHEATHAZESHADER_H
#define COD3_RENDER_CDHEATHAZESHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// Shader data externs (defined in render_xboxr:cdHeatHazeShaderVertex.o)
// ============================================================================
namespace cdHeatHazeRender {
    extern unsigned long* VS;                // ?VS@cdHeatHazeRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdHeatHazeRender@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdHeatHazeRender@@3KA
}
namespace cdHeatHazePixel {
    extern unsigned long** PS;               // ?PS@cdHeatHazePixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdHeatHazePixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdHeatHazePixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

void ToggleCDHeatHazeShader();   // @0x7CF460
void InitCDHeatHazeShader();     // @0x7CF470 (empty)
char SetupCDHeatHazeShader();    // @0x7CF480

#endif // COD3_RENDER_CDHEATHAZESHADER_H
