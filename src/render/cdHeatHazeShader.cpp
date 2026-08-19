// ============================================================================
// cdHeatHazeShader.cpp — heat haze shader (4 non-inline funcs).
// Source: source/cdHeatHazeShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdHeatHazeShader.o):
//   ToggleCDHeatHazeShader @0x7CF460
//   InitCDHeatHazeShader  @0x7CF470 (empty)
// ============================================================================
#include "cdHeatHazeShader.h"

#include <intrin.h>

// Shader static data definitions (render_xboxr cd*Shader.o)
namespace cdHeatHazeRender {
    unsigned long* VS = nullptr;
    unsigned int const** VShaderTable = nullptr;
    unsigned long Shader = 0;
}
namespace cdHeatHazePixel {
    unsigned long** PS = nullptr;
    unsigned int const** PShaderTable = nullptr;
    unsigned long* Shader = nullptr;
}

// ============================================================================
// File-local state (cdHeatHazeShader.o)
// ============================================================================
static bool gEnabled = true;  // @0xE3CA30 (release default)

// ============================================================================
// ToggleCDHeatHazeShader — flip the heat-haze enabled flag.
// ea: 0x7CF460
// ============================================================================
void ToggleCDHeatHazeShader() {
    gEnabled = !gEnabled;
}

// ============================================================================
// InitCDHeatHazeShader — no-op init.
// ea: 0x7CF470
// ============================================================================
void InitCDHeatHazeShader() {
}
