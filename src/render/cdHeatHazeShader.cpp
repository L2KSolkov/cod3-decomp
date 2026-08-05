// ============================================================================
// cdHeatHazeShader.cpp — heat haze shader (4 non-inline funcs).
// Source: source/cdHeatHazeShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdHeatHazeShader.o):
//   ToggleCDHeatHazeShader @0x7CF460
//   InitCDHeatHazeShader  @0x7CF470 (empty)
// ============================================================================
#include "cdHeatHazeShader.h"

#include <intrin.h>

// ============================================================================
// File-local state (cdHeatHazeShader.o)
// ============================================================================
static bool gEnabled;  // @0xE3CA30

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
