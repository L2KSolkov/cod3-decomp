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
#include <cstring>

#include "core/math_types.h"

struct apsBounds;  // defined in apsGroup.h

namespace math {
float ATan(float y, float x);
}

// ============================================================================
// apsQuaternion — 4-float quaternion (x,y,z,w).
// Struct (U mangling) per the map (?AU0@ / ABU0@ / AAU0@).
// Methods are split across objects per the map:
//   apsMath.o:            ctor(4f), ctor(Dir3,float), Normalize, Invert,
//                         operator*(quat), operator*(float), RotatePoint
//   apsSuppliedActions.o: ctor(Dir3), Set, operator+=
//   apsInternal.o:        default ctor
// ============================================================================
struct apsQuaternion {
    float x, y, z, w;

    apsQuaternion();                                              // apsInternal.o
    apsQuaternion(float _x, float _y, float _z, float _w);    // apsMath.o
    apsQuaternion(const math::Dir3& iAxis, float iAngle);     // apsMath.o
    apsQuaternion(const math::Dir3& iVector);                 // apsSuppliedActions.o
    void Set(float ix, float iy, float iz, float iw);        // apsSuppliedActions.o
    void Normalize();                                         // apsMath.o
    void Invert();                                            // apsMath.o
    apsQuaternion operator*(const apsQuaternion& iRHS) const; // apsMath.o
    apsQuaternion operator*(float iScalar) const;             // apsMath.o
    apsQuaternion& operator+=(const apsQuaternion& iRHS);     // apsSuppliedActions.o
    void RotatePoint(math::Dir3& ioPoint);                    // apsMath.o
};
static_assert(sizeof(apsQuaternion) == 16, "apsQuaternion size mismatch");

namespace apsMath {

// apsSuppliedActions.o: bit-preserving float-to-unsigned conversion.
unsigned int FloatAsInt(float f);

// apsMath.o (non-inline): 8 AABB corner points. Defined there, unresolved here.
void GetBoxPoints(const apsBounds& iBox, math::Dir3* oPoints);

// apsEffect.o (non-inline): pseudo-random float in [min,max]. Unresolved here.
float FloatRand(float min, float max);

// apsMath.o (non-inline): sqrt of a float. Defined there, unresolved here.
float Sqrt(float x);

// apsMath.o (non-inline): simultaneous sin/cos of one angle.
// Fast polynomial via math::SinCos<3,0,3,0> (game header, g.o). Defined there.
void SinCos(float r, float& s, float& c);

// apsMath.o (non-inline): build an orthonormal basis from a dir + up pair.
// Uses apsCommon::mCamera.mXFlip (apsCommon.o) for the Z handedness flip.
void CreateFromVectorsDirUp(math::Mat43& out, const math::Dir3& dir,
                            const math::Dir3& up);

// apsMath.o (non-inline): inverse of an orthonormal (rotation+translation) matrix.
math::Mat43 InverseOrtho(const math::Mat43& iMatrix);

// apsMath.o (non-inline): matrix -> quaternion (Shepperd's method).
apsQuaternion QuaternionFromMatrix(const math::Mat43& iMatrix);

// apsMath.o (non-inline): quaternion + translation -> matrix.
void GetQuaternionMatrix(math::Mat43& oMatrix, const apsQuaternion& iQuaternion,
                         const math::Dir3& iTranslate);

// apsMath.o (non-inline): pitch/yaw/roll -> quaternion, applied (roll*pitch)*yaw.
apsQuaternion QuaternionFromEuler(float iPitch, float iYaw, float iRoll);

// apsMath.o: per-instance RNG (GetInt/GetFloat emitted in apsEffect.o).
struct RandomNumberGenerator {
    unsigned int mSeed;

    RandomNumberGenerator(unsigned int seed) { mSeed = seed; }

    int GetInt() {
        int result = (12345 - 1043968403 * (int)mSeed) & 0x7FFFFFFF;
        mSeed = (unsigned int)result;
        return result;
    }

    float GetFloat() {
        unsigned int v = (12345 - 1043968403 * (int)mSeed) & 0x7FFFFFFF;
        mSeed = v;
        unsigned int bits = (v >> 8) | 0x3F800000u;
        float f;
        memcpy(&f, &bits, sizeof(f));
        return f - 1.0f;
    }

    float GetFloat(float min, float max) {
        unsigned int v = (12345 - 1043968403 * (int)mSeed) & 0x7FFFFFFF;
        mSeed = v;
        unsigned int bits = (v >> 8) | 0x3F800000u;
        float f;
        memcpy(&f, &bits, sizeof(f));
        return (f - 1.0f) * (max - min) + min;
    }
};

// apsMath.o: default global RNG instance.
extern RandomNumberGenerator gDefaultRandomNumberGenerator;

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

// ea: 0x00809560
inline double ATan(float y, float x) {
    return static_cast<double>(math::ATan(y, x));
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

// Multiply43 — oDest = iLeft * iRight (row-vector convention). ?Multiply43@apsMath@@YAXAAVMat43@math@@ABV23@1@Z
// (inline COMDAT, emitted in apsEffect.o). Verified against IDA 0x7EF780.
inline void Multiply43(math::Mat43& oDest, const math::Mat43& iLeft, const math::Mat43& iRight) {
    oDest.x.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(iLeft.x.v, _mm_shuffle_ps(iRight.x.v, iRight.x.v, 0x00)),
                   _mm_mul_ps(iLeft.y.v, _mm_shuffle_ps(iRight.x.v, iRight.x.v, 0x55))),
        _mm_add_ps(_mm_mul_ps(iLeft.z.v, _mm_shuffle_ps(iRight.x.v, iRight.x.v, 0xAA)),
                   _mm_mul_ps(iLeft.w.v, _mm_shuffle_ps(iRight.x.v, iRight.x.v, 0xFF))));
    oDest.y.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(iLeft.x.v, _mm_shuffle_ps(iRight.y.v, iRight.y.v, 0x00)),
                   _mm_mul_ps(iLeft.y.v, _mm_shuffle_ps(iRight.y.v, iRight.y.v, 0x55))),
        _mm_add_ps(_mm_mul_ps(iLeft.z.v, _mm_shuffle_ps(iRight.y.v, iRight.y.v, 0xAA)),
                   _mm_mul_ps(iLeft.w.v, _mm_shuffle_ps(iRight.y.v, iRight.y.v, 0xFF))));
    oDest.z.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(iLeft.x.v, _mm_shuffle_ps(iRight.z.v, iRight.z.v, 0x00)),
                   _mm_mul_ps(iLeft.y.v, _mm_shuffle_ps(iRight.z.v, iRight.z.v, 0x55))),
        _mm_add_ps(_mm_mul_ps(iLeft.z.v, _mm_shuffle_ps(iRight.z.v, iRight.z.v, 0xAA)),
                   _mm_mul_ps(iLeft.w.v, _mm_shuffle_ps(iRight.z.v, iRight.z.v, 0xFF))));
    oDest.w.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(iLeft.x.v, _mm_shuffle_ps(iRight.w.v, iRight.w.v, 0x00)),
                   _mm_mul_ps(iLeft.y.v, _mm_shuffle_ps(iRight.w.v, iRight.w.v, 0x55))),
        _mm_add_ps(_mm_mul_ps(iLeft.z.v, _mm_shuffle_ps(iRight.w.v, iRight.w.v, 0xAA)),
                   _mm_mul_ps(iLeft.w.v, _mm_shuffle_ps(iRight.w.v, iRight.w.v, 0xFF))));
}

} // namespace apsMath

namespace math {

// apsMath.o (non-inline): axis direction vectors. Defined there, unresolved here.
math::Dir3 DirX(float x);
math::Dir3 DirY(float y);
math::Dir3 DirZ(float z);

// math::Dir3::operator*=(const Mat33&) — declared in core/math_types.h,
// defined in apsMath.o.

// Per-lane sin/cos approximation used by apsMath::SinCos.
// Template is a game math header (g.o); declared here, defined there.
// apsMath::SinCos instantiates math::SinCos<3,0,3,0>.
template <int N0, int N1, int N2, int N3>
inline math::Vector4 SinCos(const math::Vector4& radians);

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
struct nglFrustum;
bool nglIsSphereVisible(const nglFrustum* Frustum, const math::Vector4* Center,
                        float Radius);

inline math::Dir3 apsVector3_Zero() {
    math::Dir3 result;
    result.v = _mm_setzero_ps();
    return result;
}

#endif // COD3_AEPS_APSMATH_H
