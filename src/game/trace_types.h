// ============================================================================
// COD3 Trace Types — trace_t, trajectory_t
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

#include "game/game_types.h"
#include "core/math_types.h"
#include "engine/broc_types.h"

// ============================================================================
// trace_t — collision trace result (80 bytes)
// Size: 0x50 (80 bytes) — verified against IDA
// ============================================================================
struct trace_t {
    math::Position3 endpos;                           // +0x00
    math::Dir3      normal;                           // +0x10
    float    fraction;                                // +0x20
    int32_t  surfaceFlags;                            // +0x24
    int32_t  contents;                                // +0x28
    const char* shader;                               // +0x2C
    DbLinkedHandle<void, Entity> mEntity;             // +0x30
    HashString partName;                              // +0x34
    uint32_t  partGroup;                              // +0x38
    uint8_t   allsolid;                               // +0x3C
    uint8_t   startsolid;                             // +0x3D
    // pad 2
    uint8_t   _pad3E[2];                              // +0x3E
    float     decal_radius;                           // +0x40
    bool      check_decal;                            // +0x44
    // pad 3
    uint8_t   _pad45[3];                              // +0x45
    // Remaining padding to 0x50
    uint8_t   _pad48[8];                              // +0x48
};
static_assert(sizeof(trace_t) == 0x50, "trace_t size mismatch");
static_assert(offsetof(trace_t, endpos) == 0x00, "trace_t::endpos offset mismatch");
static_assert(offsetof(trace_t, fraction) == 0x20, "trace_t::fraction offset mismatch");

// ============================================================================
// hitLocation_t — body part / hit location enumeration
// ============================================================================
typedef uint32_t hitLocation_t;

// ============================================================================
// trajectory_t — entity position/angle interpolation (40 bytes)
// ============================================================================
struct trajectory_t {
    int32_t         trType;       // +0x00
    int32_t         trTime;       // +0x04
    int32_t         trDuration;   // +0x08
    math::Position3 trBase;       // +0x10
    math::Position3 trDelta;      // +0x20
};
static_assert(sizeof(trajectory_t) == 0x28, "trajectory_t size mismatch? check IDA");

// ============================================================================
// collision_context_t — collision filtering context
// Size and layout TBD — reconstruct from usage patterns
// ============================================================================
struct collision_context_t {
    uint8_t data[0x40];  // placeholder — actual size TBD during porting
};
