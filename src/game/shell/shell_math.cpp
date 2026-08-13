// ============================================================================
// shell_math.cpp - FE matrix helpers (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"

#include <math.h>

extern float sNaN;  // ?sNaN@@3MA @ 0x10F19D0

// ReadVector3d (shell.o 0x0056AFA0) - declared here to satisfy the
// exact-mangle audit without defining it in this batch.
void ReadVector3d(Broc::vector& v, unsigned char* buffer, int& index);

// ea: 0x0056B0D0
void make_euler(math::Mat43& m, const Broc::vector& angles)
{
    float c1 = sinf(angles.x);
    float s2 = cosf(angles.x);
    float v6 = sinf(angles.y);
    float v7 = cosf(angles.y);
    float v8 = sinf(angles.z);
    float s1 = cosf(angles.z);
    float v2 = v8 * s2;
    float tempM_8 = s1 * s2;

    __m128 c2;
    c2.m128_f32[1] = v2;
    c2.m128_f32[2] = ((v8 * v7) * c1) - (s1 * v6);
    c2.m128_f32[0] = ((v8 * v6) * c1) + (s1 * v7);
    c2.m128_f32[3] = 0.0f;
    math::Dir3 tempM_52;
    tempM_52.v = c2;

    math::Dir3 v5;
    c2.m128_f32[0] = 0.0f;
    c2.m128_f32[1] = ((s1 * v6) * c1) - (v8 * v7);
    c2.m128_f32[2] = tempM_8;
    v5.v = c2;

    c2.m128_f32[0] = ((s1 * v7) * c1) + (v8 * v6);
    c2.m128_f32[1] = 0.0f;
    m.x.v = tempM_52.v;
    m.y.v = v5.v;
    m.z.v.m128_f32[0] = c2.m128_f32[0];
    m.z.v.m128_f32[1] = c2.m128_f32[1];
    m.z.v.m128_f32[2] = v6 * s2;
    m.z.v.m128_f32[3] = 0.0f;
    m.w.v.m128_f32[0] = 0.0f - c1;
    m.w.v.m128_f32[1] = v7 * s2;
    m.w.v.m128_f32[2] = 0.0f;
    m.w.v.m128_f32[3] = 0.0f;
}

// ea: 0x0056B2C0
void make_euler(math::Mat43& m, float z_angle)
{
    float v2 = cosf(z_angle);
    float v3 = sinf(z_angle);
    __m128 v5;
    v5.m128_f32[0] = v2;
    v5.m128_f32[1] = v3;
    m.x.v = v5;
    v5.m128_f32[0] = -v3;
    v5.m128_f32[1] = v2;
    m.y.v = v5;
    m.z.v.m128_f32[0] = 0.0f;
    m.z.v.m128_f32[1] = 0.0f;
    m.z.v.m128_f32[2] = 1.0f;
    m.z.v.m128_f32[3] = 0.0f;
    m.w.v.m128_f32[0] = 0.0f;
    m.w.v.m128_f32[1] = 0.0f;
    m.w.v.m128_f32[2] = 0.0f;
    m.w.v.m128_f32[3] = 0.0f;
}

// ea: 0x0057C000
void ReadMatrix3x4(math::Mat43& matrix, unsigned char* buffer, int& index)
{
    Broc::vector v8;
    v8.x = sNaN;
    v8.y = sNaN;
    v8.z = sNaN;
    ReadVector3d(v8, buffer, index);
    __m128 v3 = {v8.x, v8.y, v8.z, 0.0f};
    matrix.x.v = v3;
    ReadVector3d(v8, buffer, index);
    v3.m128_f32[0] = v8.x;
    v3.m128_f32[1] = v8.y;
    v3.m128_f32[2] = v8.z;
    matrix.y.v = v3;
    ReadVector3d(v8, buffer, index);
    v3.m128_f32[0] = v8.x;
    v3.m128_f32[1] = v8.y;
    v3.m128_f32[2] = v8.z;
    matrix.z.v = v3;
    ReadVector3d(v8, buffer, index);
    matrix.w.v.m128_f32[0] = v8.x;
    matrix.w.v.m128_f32[1] = v8.y;
    matrix.w.v.m128_f32[2] = v8.z;
    matrix.w.v.m128_f32[3] = 0.0f;
}
