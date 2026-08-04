// ============================================================================
// CDL Types — Core Data Library geometry primitives
// Used by cdl_common and cdl_gjk (GJK collision detection)
// Layouts verified against IDA local types (PDB).
// ============================================================================

#pragma once

#include "core/math_types.h"
#include <cstddef>

// ============================================================================
// cdlVirtual — vtable holder
// ============================================================================
struct cdlVirtual {
    void* __vftable;  // +0x00
};
static_assert(sizeof(cdlVirtual) == 0x04, "cdlVirtual size mismatch");

// ============================================================================
// cdlConvex — convex shape base (virtual support-mapping interface)
// +0x00 vtable, +0x04 surfaceFlags, +0x08 contentFlags, +0x0C checkCount,
// +0x10 m_sphere (bounds), +0x20 m_dims
// Size: 0x30 (48 bytes) — verified against IDA
// ============================================================================
struct cdlConvex {
    void*             __vftable;      // +0x00
    int               m_surfaceFlags; // +0x04
    int               m_contentFlags; // +0x08
    int               m_checkCount;   // +0x0C
    math::Vector4     m_sphere;       // +0x10 (bounding sphere/center)
    math::Dir3        m_dims;         // +0x20 (half extents)
};
static_assert(sizeof(cdlConvex) == 0x30, "cdlConvex size mismatch");
static_assert(offsetof(cdlConvex, __vftable)      == 0x00, "cdlConvex.__vftable offset mismatch");
static_assert(offsetof(cdlConvex, m_surfaceFlags) == 0x04, "cdlConvex.m_surfaceFlags offset mismatch");
static_assert(offsetof(cdlConvex, m_contentFlags) == 0x08, "cdlConvex.m_contentFlags offset mismatch");
static_assert(offsetof(cdlConvex, m_checkCount)   == 0x0C, "cdlConvex.m_checkCount offset mismatch");
static_assert(offsetof(cdlConvex, m_sphere)       == 0x10, "cdlConvex.m_sphere offset mismatch");
static_assert(offsetof(cdlConvex, m_dims)         == 0x20, "cdlConvex.m_dims offset mismatch");

// ============================================================================
// cdlAABB — axis-aligned bounding box (derives cdlConvex)
// Size: 0x30 (48 bytes) — verified against IDA
// ============================================================================
struct cdlAABB : cdlConvex {
};
static_assert(sizeof(cdlAABB) == 0x30, "cdlAABB size mismatch");

// ============================================================================
// cdlSphere — bounding sphere (derives cdlConvex)
// Size: 0x30 (48 bytes) — verified against IDA
// ============================================================================
struct cdlSphere : cdlConvex {
};
static_assert(sizeof(cdlSphere) == 0x30, "cdlSphere size mismatch");

// ============================================================================
// cdl_cinfo2 — GJK collision info output
// +0x00 pa (closest point on A), +0x10 pb (closest point on B),
// +0x20 ni (normal / direction)
// Size: 0x30 (48 bytes) — verified against IDA
// ============================================================================
struct cdl_cinfo2 {
    math::Position3 pa;  // +0x00
    math::Position3 pb;  // +0x10
    math::Dir3      ni;  // +0x20
};
static_assert(sizeof(cdl_cinfo2) == 0x30, "cdl_cinfo2 size mismatch");
static_assert(offsetof(cdl_cinfo2, pa) == 0x00, "cdl_cinfo2.pa offset mismatch");
static_assert(offsetof(cdl_cinfo2, pb) == 0x10, "cdl_cinfo2.pb offset mismatch");
static_assert(offsetof(cdl_cinfo2, ni) == 0x20, "cdl_cinfo2.ni offset mismatch");
