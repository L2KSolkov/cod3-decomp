// ============================================================================
// cdHeatHazeShader.cpp — heat haze shader (4 non-inline funcs).
// Source: source/cdHeatHazeShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdHeatHazeShader.o):
//   ToggleCDHeatHazeShader @0x7CF460
//   InitCDHeatHazeShader  @0x7CF470 (empty)
// ============================================================================
#include "cdHeatHazeShader.h"

extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);

#include <intrin.h>

// ea: 0x007CFC10
void cdHeatHazeRender::RegisterVShader()
{
    nglDxRegisterVShader(reinterpret_cast<unsigned long*>(cdHeatHazeRender::VS),
                         reinterpret_cast<const unsigned int*>(cdHeatHazeRender::VShaderTable[0]));
    cdHeatHazeRender::Shader = cdHeatHazeRender::VS[0];
}
// ============================================================================
// File-local state (cdHeatHazeShader.o)
// ============================================================================
static bool gEnabled = true;  // @0xE3CA30 (release default)

// ============================================================================
// ToggleCDHeatHazeShader — flip the heat-haze enabled flag.
// ea: 0x7CF460
// ============================================================================
bool ToggleCDHeatHazeShader() {
    bool result = !gEnabled;
    gEnabled = result;
    return result;
}

// ============================================================================
// InitCDHeatHazeShader — no-op init.
// ea: 0x7CF470
// ============================================================================
void InitCDHeatHazeShader() {
}
