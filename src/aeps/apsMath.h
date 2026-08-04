// ============================================================================
// apsMath — small math helpers used by the APS particle system.
// Source: c:\cod\code\tl\aeps\include\apsMath.h
// Inline functions emitted in apsGroup.o:
//   math::Length(Position3)             @0x7FE7A0
//   math::operator-(Dir3, float)        @0x7FE7F0
//   apsVector3_Zero()                   @0x7FE830
//   apsMath::XForm3d_1(Mat43, Dir3)     @0x7FE980
//   apsMath::Max<float> / Min<float>    @0x7FFFD0 / 0x7FFFF0
//   apsMath::Limit<float,float>         @0x800010
// ============================================================================
#ifndef COD3_AEPS_APSMATH_H
#define COD3_AEPS_APSMATH_H

#include <cmath>

#include "core/math_types.h"

struct apsBounds;  // defined in apsGroup.h

namespace apsMath {

// apsMath.o (non-inline): 8 AABB corner points. Defined there, unresolved here.
void GetBoxPoints(const apsBounds& iBox, math::Dir3* oPoints);

// apsMath.o (non-inline): pseudo-random float in [min,max]. Unresolved here.
float FloatRand(float min, float max);

template <typename T>
inline T Max(const T& a, const T& b) {
    return (b <= a) ? a : b;
}

template <typename T>
inline T Min(const T& a, const T& b) {
    return (b <= a) ? b : a;
}

template <typename T, typename S>
inline T Limit(const T& iValue, const S& iMin, const S& iMax) {
    T v = iValue;
    if (v < iMin) v = iMin;
    if (v > iMax) v = iMax;
    return v;
}

// matrix * vector (no translation w for result w)
inline math::Dir3 XForm3d_1(const math::Mat43& iMatrix, const math::Dir3& iSourceVec) {
    math::Dir3 result;
    result.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(iSourceVec.v, iSourceVec.v, 0), iMatrix.x.v),
            _mm_mul_ps(_mm_shuffle_ps(iSourceVec.v, iSourceVec.v, 85), iMatrix.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(iSourceVec.v, iSourceVec.v, 170), iMatrix.z.v),
            iMatrix.w.v));
    return result;
}

inline void SetIdentityMatrix(math::Mat43& mat) {
    mat.x.v = _mm_setr_ps(1.f, 0.f, 0.f, 0.f);
    mat.y.v = _mm_setr_ps(0.f, 1.f, 0.f, 0.f);
    mat.z.v = _mm_setr_ps(0.f, 0.f, 1.f, 0.f);
    mat.w.v = _mm_setzero_ps();
}

} // namespace apsMath

namespace math {

inline float Length(const math::Position3& v) {
    __m128 v1 = _mm_mul_ps(v.v, v.v);
    return sqrt(v1.m128_f32[0] + (v1.m128_f32[1] + v1.m128_f32[2]));
}

inline math::Dir3 operator-(const math::Dir3& a, float b) {
    math::Dir3 result;
    result.v = _mm_sub_ps(a.v, _mm_set1_ps(b));
    return result;
}

} // namespace math

// render.o (non-inline): sphere-vs-frustum test, 6 clip planes. Unresolved here.
bool nglIsSphereVisible(const math::Position3& Center, float Radius,
                        const math::Vector4* Clip);

inline math::Dir3 apsVector3_Zero() {
    math::Dir3 result;
    result.v = _mm_setzero_ps();
    return result;
}

#endif // COD3_AEPS_APSMATH_H
