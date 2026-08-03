// ============================================================================
// CDL Types — Core Data Library geometry primitives
// Used by cdl_common and cdl_gjk (GJK collision detection)
// ============================================================================

#pragma once

#include "core/math_types.h"

// Axis-aligned bounding box (center + half-extents)
struct cdlAABB {
    math::Position3 center;       // +0x00
    math::Position3 halfExtents;  // +0x10
};
static_assert(sizeof(cdlAABB) == 0x20, "cdlAABB size mismatch");

// Sphere
struct cdlSphere {
    math::Position3 center;  // +0x00
    float           radius;  // +0x10 (w component of Position3)
};
static_assert(sizeof(cdlSphere) == 0x14, "cdlSphere size mismatch");

// GJK collision info output
struct cdl_cinfo2 {
    float          distance;      // +0x00
    math::Dir3     normal;        // +0x04 (16 bytes, packed)
    math::Position3 pointA;       // +0x14
    math::Position3 pointB;       // +0x24
    int            iterationCount;// +0x34
};
static_assert(sizeof(cdl_cinfo2) == 0x38, "cdl_cinfo2 size mismatch");

// Convex shape — interface for GJK support mapping
struct cdlConvex {
    void* vtable;  // virtual function table
    // virtual math::Position3 support(const math::Dir3& dir) const = 0;
};
