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

    void check_for_decal(float radius);  // ?check_for_decal@trace_t@@QAEXM@Z (g.o 0x4A4FD0)
    bool decal_ok();                     // ?decal_ok@trace_t@@QAE_NXZ (g.o 0x4A4FF0)
    void Clear();                        // ?Clear@trace_t@@QAEXXZ (g.o 0x4AEC20)
};
static_assert(sizeof(trace_t) == 0x50, "trace_t size mismatch");
static_assert(offsetof(trace_t, endpos) == 0x00, "trace_t::endpos offset mismatch");
static_assert(offsetof(trace_t, fraction) == 0x20, "trace_t::fraction offset mismatch");

// ============================================================================
// collision_context_t — collision filtering context
// Size: 0x18 (24 bytes) — verified against IDA (vtbl + derived data)
// ============================================================================
struct collision_context_t;

struct collision_context_t_vtbl {
    bool (__cdecl* filter)(collision_context_t* self, Entity* ent);
};

// ============================================================================
struct collision_context_t {
    struct collision_context_t_vtbl* __vftable;  // +0x00
    DbLinkedHandle<EntityHandleDb, Entity> pass_entity1;    // +0x04
    DbLinkedHandle<EntityHandleDb, Entity> pass_entity2;    // +0x08
    DbLinkedHandle<EntityHandleDb, Entity> pass_owner1;     // +0x0C
    DbLinkedHandle<EntityHandleDb, Entity> pass_owner2;     // +0x10
    int   contentmask;                            // +0x14

    collision_context_t();                       // ??0collision_context_t@@QAE@XZ
    collision_context_t(int mask);               // ??0collision_context_t@@QAE@H@Z
    collision_context_t(DbLinkedHandle<EntityHandleDb, Entity> handle, int mask);  // ??0collision_context_t@@QAE@V?$DbLinkedHandle@...@@H@Z
    collision_context_t(DbLinkedHandle<EntityHandleDb, Entity> handle1,
                        DbLinkedHandle<EntityHandleDb, Entity> handle2,
                        int mask);                // ??0collision_context_t@@QAE@V?$DbLinkedHandle@...@@0H@Z
    bool filter(Entity* ent);  // ?filter@collision_context_t@@UBE_NPAVEntity@@@Z (g.o 0x4AF020)
};
static_assert(sizeof(collision_context_t) == 0x18, "collision_context_t size mismatch");
static_assert(offsetof(collision_context_t, pass_entity1) == 0x04,
              "collision_context_t::pass_entity1 offset mismatch");
static_assert(offsetof(collision_context_t, pass_entity2) == 0x08,
              "collision_context_t::pass_entity2 offset mismatch");
static_assert(offsetof(collision_context_t, pass_owner1) == 0x0C,
              "collision_context_t::pass_owner1 offset mismatch");
static_assert(offsetof(collision_context_t, pass_owner2) == 0x10,
              "collision_context_t::pass_owner2 offset mismatch");
static_assert(offsetof(collision_context_t, contentmask) == 0x14,
              "collision_context_t::contentmask offset mismatch");

// player_collision_context_t - pmove collision context (g_bg_pmove.cpp)
class player_collision_context_t : public collision_context_t {
public:
    player_collision_context_t();  // default (pmove local usage)
    player_collision_context_t(DbLinkedHandle<EntityHandleDb, Entity> handle,
                               int mask);  // ??0player_collision_context_t@@QAE@V?$DbLinkedHandle@...@@H@Z (g.o 0x4B0000)
    bool filter(Entity* ent) const;  // ?filter@player_collision_context_t@@UBE_NPAVEntity@@@Z
};
