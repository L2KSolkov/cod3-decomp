// ============================================================================
// COD3 Math Types — geometry primitives used throughout the engine
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

#include "core/sse_portable.h"

namespace math {

// Forward declarations
class Dir3;
class Position3;
class Mat33;
class Mat43;
struct TranMat43;
class DiagMat33;

// ============================================================================
// DiagMat33 - diagonal 3x3 scale (16 bytes: packed x/y/z scale + pad w).
// Size: 0x10 - verified against IDA (math::DiagMat33).
// ============================================================================
class DiagMat33 {
public:
    __m128 v;  // SSE-packed: x, y, z, w
};
static_assert(sizeof(DiagMat33) == 0x10, "DiagMat33 size mismatch");

// ============================================================================
// Dir3 — normalized direction vector (3 float + pad w, total 16 bytes)
// Size: 0x10 (16 bytes) — verified against IDA
// Note: declared `class` (not struct) to match the original binary's MSVC
// mangling (`?AVDir3` / `ABV...Dir3...`), which uses V for class types.
// ============================================================================
class Dir3 {
public:
    __m128 v;  // SSE-packed: x, y, z, w

    // apsMath.o (non-inline): row-vector * 3x3 matrix. Unresolved here.
    const math::Dir3& operator*=(const math::Mat33& m);

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
class Position3 {
public:
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
class Vector4 {
public:
    __m128 v;

    float operator[](int i) const { return v.m128_f32[i]; }
    float& operator[](int i) { return v.m128_f32[i]; }

    struct Packed {
        float x, y, z, w;
    };
};
static_assert(sizeof(Vector4) == 0x10, "Vector4 size mismatch");

// ============================================================================
// Mat43 — 4x3 affine transform matrix (64 bytes)
//   3 rotation axes (Dir3 each, 16 bytes) + 1 translation (Position3, 16 bytes)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
class Mat43 {
public:
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
// Note: declared `class` (not struct) to match the original binary's MSVC
// mangling (`ABVMat33` / `?AVMat33`), which uses V for class types.
// Members are x/y/z (Dir3 rows) per the original source.
// ============================================================================
class Mat33 {
public:
    Dir3 x;  // +0x00
    Dir3 y;  // +0x10
    Dir3 z;  // +0x20
};
static_assert(sizeof(Mat33) == 0x30, "Mat33 size mismatch");

// ============================================================================
// Mat44 — 4x4 matrix (64 bytes = 4 * Vector4)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
struct Mat44 {
    Vector4 x;  // +0x00
    Vector4 y;  // +0x10
    Vector4 z;  // +0x20
    Vector4 w;  // +0x30
};
static_assert(sizeof(Mat44) == 0x40, "Mat44 size mismatch");

// ============================================================================
// TranMat43 — translation-only matrix (16 bytes, single Position3)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
struct TranMat43 {
    __m128 v;
};
static_assert(sizeof(TranMat43) == 0x10, "TranMat43 size mismatch");

// Quaternion â€” 4-float quaternion (16 bytes)
struct Quaternion {
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08
    float w;  // +0x0C
};
static_assert(sizeof(Quaternion) == 0x10, "Quaternion size mismatch");

// ============================================================================
// com_math.h helpers (inline COMDATs; g.o / scr.o / streamer.o)
//   SinCos<3,0,3,0>          - ea: 0x4AE920
//   FastSinCos               - ea: 0x4B01F0
//   AnglesToForward(float*)  - ea: 0x4B02A0
//   AnglesToForward(Pos3)    - ea: 0x4B03A0
//   AnglesToUp               - ea: 0x4B0440
//   AnglesToRight            - ea: 0x5EE650
// Function-only AeAssert contract (no enum here: other headers declare it).
// ============================================================================

// Per-lane sin/cos of radians. Lane i computes sin when arg i != 0 (phase
// +3pi/2) and cos when arg i == 0. Only <3,0,3,0> exists in the binary.
template <int A, int B, int C, int D>
inline Vector4 SinCos(const Vector4& radians)
{
    const __m128 sign_mask = _mm_set1_ps(-0.0f);
    const __m128 shift = _mm_setr_ps(A ? 4.7123880f : 0.0f,
                                     B ? 4.7123880f : 0.0f,
                                     C ? 4.7123880f : 0.0f,
                                     D ? 4.7123880f : 0.0f);
    // Folded range reduction: t = -|x + shift| / (2pi), y = |frac(t)-0.5|-0.25.
    __m128 t = _mm_mul_ps(
        _mm_xor_ps(sign_mask,
                   _mm_andnot_ps(sign_mask, _mm_add_ps(radians.v, shift))),
        _mm_set1_ps(0.15915494f));
    __m128 y = _mm_sub_ps(
        _mm_andnot_ps(
            sign_mask,
            _mm_sub_ps(_mm_sub_ps(_mm_add_ps(_mm_sub_ps(t, _mm_set1_ps(12582912.0f)),
                                             _mm_set1_ps(12582912.0f)),
                                  t),
                       _mm_set1_ps(0.5f))),
        _mm_set1_ps(0.25f));
    __m128 y2 = _mm_mul_ps(y, y);
    __m128 y3 = _mm_mul_ps(y, y2);
    __m128 y4 = _mm_mul_ps(y2, y2);
    __m128 y5 = _mm_mul_ps(y, y4);
    __m128 y7 = _mm_mul_ps(y3, y4);
    __m128 y9 = _mm_mul_ps(y5, y4);
    Vector4 result;
    result.v = _mm_add_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(y9, _mm_set1_ps(39.710659f)),
                           _mm_mul_ps(y7, _mm_set1_ps(-76.574959f))),
                _mm_mul_ps(y5, _mm_set1_ps(81.602226f))),
            _mm_mul_ps(y3, _mm_set1_ps(-41.341675f))),
        _mm_mul_ps(y, _mm_set1_ps(6.2831850f)));
    return result;
}

} // namespace math

namespace AeAssert {
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

// ea: 0x4B01F0
inline void FastSinCos(float radians, float* psin, float* pcos)
{
    math::Vector4 in;
    in.v = _mm_setr_ps(radians, radians, radians, 0.0f);
    math::Vector4 r = math::SinCos<3, 0, 3, 0>(in);
    *psin = r.v.m128_f32[0];
    *pcos = r.v.m128_f32[1];
}

// ea: 0x4B02A0
inline void AnglesToForward(const float* const angles,
                            float* const forward)
{
    if (forward == nullptr)
    {
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float sy, cy, sp, cp;
    FastSinCos(angles[1] * 0.017453292f, &sy, &cy);
    FastSinCos(angles[0] * 0.017453292f, &sp, &cp);
    forward[0] = cp * cy;
    forward[1] = cp * sy;
    forward[2] = -sp;
}

// ea: 0x4B03A0
inline void AnglesToForward(const math::Position3& angles,
                            math::Dir3& forward)
{
    float sy, cy, sp, cp;
    FastSinCos(angles.v.m128_f32[1] * 0.017453292f, &sy, &cy);
    FastSinCos(angles.v.m128_f32[0] * 0.017453292f, &sp, &cp);
    forward.v.m128_f32[0] = cp * cy;
    forward.v.m128_f32[1] = cp * sy;
    forward.v.m128_f32[2] = -sp;
}

// ea: 0x4B0440
inline void AnglesToUp(const float* const angles, float* const up)
{
    if (up == nullptr)
    {
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float sp, cp, sr, cr;
    FastSinCos(angles[0] * 0.017453292f, &sp, &cp);
    FastSinCos(angles[2] * 0.017453292f, &sr, &cr);
    up[0] = cr * sp;
    up[1] = -sr;
    up[2] = cr * cp;
}

// ea: 0x5EE650
inline void AnglesToRight(const float* const angles,
                          float* const right)
{
    if (right == nullptr)
    {
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float sy, cy, sr, cr;
    FastSinCos(angles[1] * 0.017453292f, &sy, &cy);
    FastSinCos(angles[2] * 0.017453292f, &sr, &cr);
    right[0] = cr * sy;
    right[1] = -cr * cy;
    right[2] = -sr;
}

// ea: 0x51A630 (game2.o COMDAT) - c:\cod\code\game\com_math.h:682
// Binary assert: "beg <= end" with message "Beg must be less than end."
template <typename T>
inline T ClampRange(const T& in, const T& beg, const T& end)
{
    if (end < beg)
    {
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Beg must be less than end."))
            __debugbreak();
    }
    if (beg > in)
        return beg;
    if (in <= end)
        return in;
    return end;
}
