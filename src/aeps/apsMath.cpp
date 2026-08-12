// ============================================================================
// apsMath.cpp — APS math helpers (apsMath.o, 21 funcs + 1 global).
// Reconstructed from codmp_xboxr.xbe (release build /O2).
// Source: c:\cod\code\tl\aeps\source\apsMath.cpp
//
// Ownership (map): apsMath.o owns every symbol defined here. Shared types:
//   apsQuaternion  — struct, methods split apsMath.o / apsSuppliedActions.o /
//                    apsInternal.o (see apsMath.h)
//   RandomNumberGenerator — GetInt/GetFloat emitted in apsEffect.o
//   apsCommon::mCamera     — defined in apsCommon.o (unresolved until then)
//   math::SinCos<...>      — game header template (g.o); apsMath::SinCos
//                    instantiates math::SinCos<3,0,3,0>, unresolved here.
// ============================================================================
#include "apsMath.h"

#include "apsGroup.h"    // apsBounds
#include "apsCommon.h"   // apsCommon::mCamera

#include <cmath>

namespace apsMath {
// ?FloatRand@apsMath@@YAMMM@Z (apsEffect.o; pseudo-random float in [min,max])
float FloatRand(float min, float max)
{
    if (max <= min)
        return min;
    return min + (max - min) * ((float)rand() / (float)RAND_MAX);
}
}  // namespace apsMath

namespace math {

// ============================================================================
// Axis direction vectors. ea: 0x00806ED0 / 0x00806F00 / 0x00806F30
// ============================================================================
math::Dir3 math::DirX(float x) {
    math::Dir3 result;
    result.v = _mm_setr_ps(x, 0.0f, 0.0f, 0.0f);
    return result;
}

math::Dir3 math::DirY(float y) {
    math::Dir3 result;
    result.v = _mm_setr_ps(0.0f, y, 0.0f, 0.0f);
    return result;
}

math::Dir3 math::DirZ(float z) {
    math::Dir3 result;
    result.v = _mm_setr_ps(0.0f, 0.0f, z, 0.0f);
    return result;
}

// ============================================================================
// Dir3 * Mat33 (row vector transform). ea: 0x00806F60
// ============================================================================
const math::Dir3& math::Dir3::operator*=(const math::Mat33& m) {
    this->v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(this->v, this->v, 0), m.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(this->v, this->v, 85), m.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(this->v, this->v, 170), m.z.v));
    return *this;
}

} // namespace math

namespace apsMath {

// ============================================================================
// RandomNumberGenerator global. ea: data @0x010E0A18 (4 bytes, mSeed)
// ============================================================================
RandomNumberGenerator gDefaultRandomNumberGenerator = RandomNumberGenerator(0);

// ============================================================================
// Sqrt. ea: 0x00806FC0
// ============================================================================
float apsMath::Sqrt(float x) {
    return sqrt(x);
}

// ============================================================================
// SinCos — fast simultaneous sin/cos of one angle.
// ea: 0x00807F90
// Builds a 4-lane vector of r, runs the per-lane game SinCos<3,0,3,0>
// polynomial (lane 0 = sin, lane 1 = cos), returns lanes 0 and 1.
// ============================================================================
void apsMath::SinCos(float r, float& s, float& c) {
    math::Vector4 radians;
    radians.v = _mm_set_ps(r, r, r, r);
    math::Vector4 result = math::SinCos<3, 0, 3, 0>(radians);
    s = result.v.m128_f32[0];
    c = result.v.m128_f32[1];
}

// ============================================================================
// Orthonormal basis from a forward (dir) and reference (up) vector.
// ea: 0x00807200
//   x = dir
//   z = normalize(cross(dir, up)) * mXFlip   (handedness flip from camera)
//   y = normalize(cross(z_axis, dir))
//   w = 0
// ============================================================================
void apsMath::CreateFromVectorsDirUp(math::Mat43& out, const math::Dir3& dir,
                                     const math::Dir3& up) {
    // z-axis = normalize(cross(dir, up))
    __m128 d = dir.v;
    __m128 u = up.v;
    __m128 crossZU = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(d, d, 9), _mm_shuffle_ps(u, u, 18)),
        _mm_mul_ps(_mm_shuffle_ps(d, d, 18), _mm_shuffle_ps(u, u, 9)));
    __m128 sq = _mm_mul_ps(crossZU, crossZU);
    float len = sqrt(sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2]);
    __m128 zAxis = _mm_div_ps(crossZU, _mm_set1_ps(len));

    // y-axis = normalize(cross(z_axis, dir))
    __m128 crossYD = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(zAxis, zAxis, 9), _mm_shuffle_ps(d, d, 18)),
        _mm_mul_ps(_mm_shuffle_ps(zAxis, zAxis, 18), _mm_shuffle_ps(d, d, 9)));
    sq = _mm_mul_ps(crossYD, crossYD);
    len = sqrt(sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2]);
    __m128 yAxis = _mm_div_ps(crossYD, _mm_set1_ps(len));

    out.x.v = dir.v;
    out.y.v = yAxis;
    out.z.v = _mm_mul_ps(zAxis, _mm_set1_ps(apsCommon::mCamera.mXFlip));
    out.w.v = _mm_setzero_ps();
}

} // namespace apsMath

// ============================================================================
// Quaternion rotation of a point: p' = q * p * conj(q). ea: 0x00807370
// Standard implementation (q assumed normalized).
// ============================================================================
void apsQuaternion::RotatePoint(math::Dir3& ioPoint) {
    const float px = ioPoint.v.m128_f32[0];
    const float py = ioPoint.v.m128_f32[1];
    const float pz = ioPoint.v.m128_f32[2];

    // t = 2 * cross(q_vec, p)
    const float tx = 2.0f * (y * pz - z * py);
    const float ty = 2.0f * (z * px - x * pz);
    const float tz = 2.0f * (x * py - y * px);

    // p' = p + w*t + cross(q_vec, t)
    ioPoint.v.m128_f32[0] = px + w * tx + (y * tz - z * ty);
    ioPoint.v.m128_f32[1] = py + w * ty + (z * tx - x * tz);
    ioPoint.v.m128_f32[2] = pz + w * tz + (x * ty - y * tx);
}

namespace apsMath {

// ============================================================================
// 8 AABB corner points. ea: 0x00807560
// Order: min, min+dx, min+dy, min+dz, max-dx, max-dy, max-dz, max.
// ============================================================================
void apsMath::GetBoxPoints(const apsBounds& iBox, math::Dir3* oPoints) {
    math::Dir3 d;
    d.v = _mm_sub_ps(iBox.mMax.v, iBox.mMin.v);
    oPoints[0].v = iBox.mMin.v;
    oPoints[1].v = _mm_add_ps(iBox.mMin.v, _mm_shuffle_ps(d.v, d.v, 0x00));
    oPoints[2].v = _mm_add_ps(iBox.mMin.v, _mm_shuffle_ps(d.v, d.v, 0x55));
    oPoints[3].v = _mm_add_ps(iBox.mMin.v, _mm_shuffle_ps(d.v, d.v, 0xAA));
    oPoints[4].v = _mm_sub_ps(iBox.mMax.v, _mm_shuffle_ps(d.v, d.v, 0x00));
    oPoints[5].v = _mm_sub_ps(iBox.mMax.v, _mm_shuffle_ps(d.v, d.v, 0x55));
    oPoints[6].v = _mm_sub_ps(iBox.mMax.v, _mm_shuffle_ps(d.v, d.v, 0xAA));
    oPoints[7].v = iBox.mMax.v;
}

// ============================================================================
// Inverse of an orthonormal matrix: transpose rotation, negate translation.
// ea: 0x00807740
// Standard implementation (R^T, t' = -R^T * t).
// ============================================================================
math::Mat43 apsMath::InverseOrtho(const math::Mat43& iMatrix) {
    math::Mat43 result;

    // Transpose the 3x3 rotation.
    result.x.v = _mm_setr_ps(iMatrix.x.v.m128_f32[0], iMatrix.y.v.m128_f32[0],
                             iMatrix.z.v.m128_f32[0], 0.0f);
    result.y.v = _mm_setr_ps(iMatrix.x.v.m128_f32[1], iMatrix.y.v.m128_f32[1],
                             iMatrix.z.v.m128_f32[1], 0.0f);
    result.z.v = _mm_setr_ps(iMatrix.x.v.m128_f32[2], iMatrix.y.v.m128_f32[2],
                             iMatrix.z.v.m128_f32[2], 0.0f);

    // t' = -(R^T * t)
    __m128 t = iMatrix.w.v;
    result.w.v = _mm_xor_ps(
        _mm_set_ps(-0.0f, -0.0f, -0.0f, -0.0f),
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(t, t, 0x00), result.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(t, t, 0x55), result.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(t, t, 0xAA), result.z.v)));
    return result;
}

// ============================================================================
// Matrix -> quaternion (Shepperd's method). ea: 0x00807840
// Standard implementation.
// ============================================================================
apsQuaternion apsMath::QuaternionFromMatrix(const math::Mat43& iMatrix) {
    const float m00 = iMatrix.x.v.m128_f32[0];
    const float m01 = iMatrix.x.v.m128_f32[1];
    const float m02 = iMatrix.x.v.m128_f32[2];
    const float m10 = iMatrix.y.v.m128_f32[0];
    const float m11 = iMatrix.y.v.m128_f32[1];
    const float m12 = iMatrix.y.v.m128_f32[2];
    const float m20 = iMatrix.z.v.m128_f32[0];
    const float m21 = iMatrix.z.v.m128_f32[1];
    const float m22 = iMatrix.z.v.m128_f32[2];

    const float trace = m00 + m11 + m22;
    apsQuaternion q;

    if (trace > 0.0f) {
        float s = sqrt(trace + 1.0f);
        q.w = s * 0.5f;
        s = 0.5f / s;
        q.x = (m21 - m12) * s;
        q.y = (m02 - m20) * s;
        q.z = (m10 - m01) * s;
    } else {
        int i = 0, j = 1, k = 2;
        const float diag[3] = { m00, m11, m22 };
        if (diag[1] > diag[0]) { i = 1; j = 2; k = 0; }
        if (diag[2] > diag[i]) { i = 2; j = 0; k = 1; }

        float s = sqrt(diag[i] - diag[j] - diag[k] + 1.0f);
        float r[3];
        r[i] = s * 0.5f;
        float t = (s != 0.0f) ? (0.5f / s) : 0.0f;
        r[j] = 0.0f; r[k] = 0.0f;

        const float m[3][3] = {
            { m00, m01, m02 },
            { m10, m11, m12 },
            { m20, m21, m22 },
        };
        r[j] = (m[j][i] + m[i][j]) * t;
        r[k] = (m[k][i] + m[i][k]) * t;
        q.x = r[0];
        q.y = r[1];
        q.z = r[2];
        q.w = (m[k][j] - m[j][k]) * t;
    }
    return q;
}

// ============================================================================
// Quaternion (+ translation) -> matrix. ea: 0x00807AC0
// Standard implementation; sqrt(2) scaling yields the 2*() rotation terms.
// ============================================================================
void apsMath::GetQuaternionMatrix(math::Mat43& oMatrix,
                                  const apsQuaternion& iQuaternion,
                                  const math::Dir3& iTranslate) {
    const float x = iQuaternion.x;
    const float y = iQuaternion.y;
    const float z = iQuaternion.z;
    const float w = iQuaternion.w;
    const float s = 1.4142135f;  // sqrt(2)

    const float vx = x * s;
    const float vy = y * s;
    const float vz = z * s;
    const float vw = w * s;

    oMatrix.x.v = _mm_setr_ps(1.0f - (vy * vy) - (vz * vz),
                              (vx * vy) - (vw * vz),
                              (vw * vy) + (vx * vz), 0.0f);
    oMatrix.y.v = _mm_setr_ps((vw * vz) + (vx * vy),
                              1.0f - (vx * vx) - (vz * vz),
                              (vy * vz) - (vw * vx), 0.0f);
    oMatrix.z.v = _mm_setr_ps((vx * vz) - (vw * vy),
                              (vw * vx) + (vy * vz),
                              1.0f - (vx * vx) - (vy * vy), 0.0f);
    oMatrix.w.v = iTranslate.v;
}

} // namespace apsMath

// ============================================================================
// Axis-angle -> quaternion. ea: 0x00807C60
// w = cos(a/2), xyz = axis * sin(a/2), then normalized.
// ============================================================================
apsQuaternion::apsQuaternion(const math::Dir3& iAxis, float iAngle) {
    float s, c;
    apsMath::SinCos(iAngle * 0.5f, s, c);

    this->w = c;
    this->x = iAxis.v.m128_f32[0] * s;
    this->y = iAxis.v.m128_f32[1] * s;
    this->z = iAxis.v.m128_f32[2] * s;

    float inv = 1.0f / sqrt(x * x + y * y + z * z + w * w);
    x *= inv; y *= inv; z *= inv; w *= inv;
}

namespace apsMath {

// ============================================================================
// Pitch/yaw/roll -> quaternion, applied (roll * pitch) * yaw.
// ea: 0x00807D70
// ============================================================================
apsQuaternion apsMath::QuaternionFromEuler(float iPitch, float iYaw, float iRoll) {
    apsQuaternion roll(math::DirZ(1.0f), iRoll);
    apsQuaternion pitch(math::DirX(1.0f), iPitch);
    apsQuaternion yaw(math::DirY(1.0f), iYaw);
    return (roll * pitch) * yaw;
}

} // namespace apsMath

// ============================================================================
// apsQuaternion methods owned by apsMath.o (struct is at global scope).
// ============================================================================

// ctor(4 floats). ea: 0x00807000
apsQuaternion::apsQuaternion(float _x, float _y, float _z, float _w) {
    x = _x; y = _y; z = _z; w = _w;
}

// Normalize. ea: 0x00807030
void apsQuaternion::Normalize() {
    float inv = 1.0f / sqrt(x * x + y * y + z * z + w * w);
    x *= inv; y *= inv; z *= inv; w *= inv;
}

// Invert — conjugate (negate vector part). ea: 0x00807080
void apsQuaternion::Invert() {
    x = -x; y = -y; z = -z;
}

// Hamilton product. ea: 0x008070B0
apsQuaternion apsQuaternion::operator*(const apsQuaternion& iRHS) const {
    apsQuaternion result;
    result.w = (w * iRHS.w) - (iRHS.x * x) - (y * iRHS.y) - (z * iRHS.z);
    result.x = (w * iRHS.x) + (x * iRHS.w) + (y * iRHS.z) - (z * iRHS.y);
    result.y = (iRHS.y * w) - (iRHS.z * x) + (y * iRHS.w) + (z * iRHS.x);
    result.z = (iRHS.z * w) + (iRHS.y * x) - (y * iRHS.x) + (z * iRHS.w);
    return result;
}

// Scalar multiply. ea: 0x008071A0
apsQuaternion apsQuaternion::operator*(float iScalar) const {
    apsQuaternion result;
    result.x = x * iScalar;
    result.y = y * iScalar;
    result.z = z * iScalar;
    result.w = w * iScalar;
    return result;
}

// ============================================================================
// apsBounds::Size — AABB extent (max - min). ea: 0x00806FD0
// Defined in apsMath.o; declared in apsGroup.h.
// ============================================================================
math::Dir3 apsBounds::Size() const {
    math::Dir3 result;
    result.v = _mm_sub_ps(mMax.v, mMin.v);
    return result;
}
