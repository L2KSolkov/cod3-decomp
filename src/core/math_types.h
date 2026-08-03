// ============================================================================
// COD3 Math Types — geometry primitives used throughout the engine
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

#include <xmmintrin.h>  // __m128

namespace math {

// Forward declarations
struct Dir3;
struct Position3;
struct Mat43;
struct TranMat43;

// ============================================================================
// Dir3 — normalized direction vector (3 float + pad w, total 16 bytes)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
struct Dir3 {
    __m128 v;  // SSE-packed: x, y, z, w

    // Constant layout (for compile-time initialization)
    struct Constant {
        float x, y, z, w;
    };

    // Packed layout (3 floats, 12 bytes — for network/disk)
    struct Packed {
        float x, y, z;
    };
};
static_assert(sizeof(Dir3) == 0x10, "Dir3 size mismatch");
static_assert(sizeof(Dir3::Constant) == 0x10, "Dir3::Constant size mismatch");
static_assert(sizeof(Dir3::Packed) == 0x0C, "Dir3::Packed size mismatch");

// ============================================================================
// Position3 — point/translation vector (3 float + pad w, total 16 bytes)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
struct Position3 {
    __m128 v;  // SSE-packed: x, y, z, w

    struct Constant {
        float x, y, z, w;
    };

    struct Packed {
        float x, y, z;
    };
};
static_assert(sizeof(Position3) == 0x10, "Position3 size mismatch");
static_assert(sizeof(Position3::Constant) == 0x10, "Position3::Constant size mismatch");
static_assert(sizeof(Position3::Packed) == 0x0C, "Position3::Packed size mismatch");

// ============================================================================
// Vector4 — 4-component float vector (16 bytes)
// Size: 0x10 (16 bytes)
// ============================================================================
struct Vector4 {
    __m128 v;
};
static_assert(sizeof(Vector4) == 0x10, "Vector4 size mismatch");

// ============================================================================
// Mat43 — 4x3 affine transform matrix (64 bytes)
//   3 rotation axes (Dir3 each, 16 bytes) + 1 translation (Position3, 16 bytes)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
struct Mat43 {
    Dir3      x;  // +0x00 — right axis
    Dir3      y;  // +0x10 — forward axis
    Dir3      z;  // +0x20 — up axis
    Position3 w;  // +0x30 — translation

    // Compressed form (48 bytes: 3 * 12-byte Packed dirs + 12-byte Packed pos)
    struct Packed {
        Dir3::Packed      x;
        Dir3::Packed      y;
        Dir3::Packed      z;
        Position3::Packed w;
    };
};
static_assert(sizeof(Mat43) == 0x40, "Mat43 size mismatch");
static_assert(sizeof(Mat43::Packed) == 0x30, "Mat43::Packed size mismatch");

// ============================================================================
// Mat33 — 3x3 rotation matrix (48 bytes = 3 * Dir3)
// Size: 0x30 (48 bytes)
// ============================================================================
struct Mat33 {
    Dir3 row0;  // +0x00
    Dir3 row1;  // +0x10
    Dir3 row2;  // +0x20
};
static_assert(sizeof(Mat33) == 0x30, "Mat33 size mismatch");

// ============================================================================
// TranMat43 — translation-only matrix (16 bytes, single Position3)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
struct TranMat43 {
    __m128 v;
};
static_assert(sizeof(TranMat43) == 0x10, "TranMat43 size mismatch");

} // namespace math
