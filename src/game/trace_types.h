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
    DbLinkedHandle<EntityHandleDb, Entity> mEntity;             // +0x30
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
// collision_context_t — collision filtering context
// Size: 0x18 (24 bytes) — verified against IDA (vtbl + derived data)
// ============================================================================
struct collision_context_t {
    struct collision_context_t_vtbl* __vftable;  // +0x00
    DbLinkedHandle<EntityHandleDb, Entity> pass_entity1;    // +0x04
    DbLinkedHandle<EntityHandleDb, Entity> pass_entity2;    // +0x08
    int   contentmask;                            // +0x0C
    // +0x10 .. 0x18 derived-specific data
    uint8_t _pad10[8];                            // +0x10
};
static_assert(sizeof(collision_context_t) == 0x18, "collision_context_t size mismatch");
