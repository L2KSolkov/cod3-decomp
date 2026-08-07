// ============================================================================
// apsSuppliedDomains — concrete particle source domains.
// Source: c:\cod\code\tl\aeps\source\apsSuppliedDomains.cpp
// Verified against IDA (aeps_xboxr:apsSuppliedDomains.o), 26 non-inline funcs.
// All GetValue/TestValue assert iNumDimensions == 3 at their original lines.
// Spheres/hemispheres use rejection sampling over the global RNG:
//   x,y in [-r,r] via GetFloat()*2r - r, z in [0,r) via GetFloat()*r;
//   retry while x^2+y^2+z^2 >= r^2, bounded by 30 (sphere) / 100 (hemi) iters.
// GetId FourCCs + GetVersion are inline COMDATs (header), matching apsRegister.o.
// ============================================================================

#include "apsSuppliedDomains.h"

extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// apsSuppliedDomains.o data (4 gHiHat statics, global namespace).
int gHiHat_SphereDomain;        // ?gHiHat_SphereDomain@@3HA @0x10E0A58
int gHiHat_YHemisphereDomain;   // ?gHiHat_YHemisphereDomain@@3HA @0x10E0A5C
int gHiHat_ZHemisphereDomain;   // ?gHiHat_ZHemisphereDomain@@3HA @0x10E0A60
int gHiHat_DiscDomain;          // ?gHiHat_DiscDomain@@3HA @0x10E0A64

// ============================================================================
// apsDomain::Fixup — relocate this domain's vtable through the fixup table.
// ea: 0x810770. Linear scan over domains[] matching .id (vtable) -> .vtbl.
// ============================================================================
void apsDomain::Fixup(const apsFixupParams& iFixupParams) {
    unsigned int vtbl = 0;
    int v = 0;
    if (iFixupParams.numDomains > 0) {
        for (v = 0; v < iFixupParams.numDomains; ++v) {
            if (iFixupParams.domains[v].id == GetVtable())
                break;
        }
        if (v < iFixupParams.numDomains) {
            vtbl = iFixupParams.domains[v].vtbl;
        }
    }
    if (vtbl == 0) {
        tlFatal("Didn't find vtable for domain");
    }
    SetVtable(vtbl);
}

// ============================================================================
// apsSphereDomain — uniform point inside a sphere.
// ============================================================================

apsSphereDomain::apsSphereDomain(const math::Dir3& iCenter, float iRadius) {
    mSphere.mSphere.v.m128_f32[0] = iCenter.v.m128_f32[0];
    mSphere.mSphere.v.m128_f32[1] = iCenter.v.m128_f32[1];
    mSphere.mSphere.v.m128_f32[2] = iCenter.v.m128_f32[2];
    mSphere.mSphere.v.m128_f32[3] = iRadius;
}

apsSphereDomain::apsSphereDomain(const apsSphere& sphere) {
    mSphere.mSphere = sphere.mSphere;
}

void apsSphereDomain::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 70,
                  "iNumDimensions == 3",
                  "apsSphereDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float radius = mSphere.mSphere.v.m128_f32[3];
    float radiusSq = radius * radius;
    float dRadius = radius * 2.0f;

    float x = 0, y = 0, z = 0;
    int iter_count = 0;
    do {
        x = apsMath::gDefaultRandomNumberGenerator.GetFloat() * dRadius - radius;
        y = apsMath::gDefaultRandomNumberGenerator.GetFloat() * dRadius - radius;
        z = apsMath::gDefaultRandomNumberGenerator.GetFloat() * radius;
        if (radiusSq > (x * x + y * y + z * z))
            break;
        if (iter_count >= 30 &&
            _tlAssert("source/apsSuppliedDomains.cpp", 97,
                      "iter_count < 30",
                      "The probability of this happening is about 6.2e-33...go buy a lottery ticket, or fix the random number generator"))
            __debugbreak();
        iter_count++;
    } while (iter_count < 30);

    if (iter_count > gHiHat_SphereDomain)
        gHiHat_SphereDomain = iter_count;

    oOutput[0] = mSphere.mSphere.v.m128_f32[0] + x;
    oOutput[1] = mSphere.mSphere.v.m128_f32[1] + y;
    oOutput[2] = mSphere.mSphere.v.m128_f32[2] + z;
}

unsigned int apsSphereDomain::TestValue(int iNumDimensions, float* iValue) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 118,
                  "iNumDimensions == 3",
                  "apsSphereDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float dx = mSphere.mSphere.v.m128_f32[0] - iValue[0];
    float dy = mSphere.mSphere.v.m128_f32[1] - iValue[1];
    float dz = mSphere.mSphere.v.m128_f32[2] - iValue[2];
    float radius = mSphere.mSphere.v.m128_f32[3];
    return (radius * radius > (dx * dx + dy * dy + dz * dz)) ? 1 : 0;
}

// ============================================================================
// apsYHemisphereDomain — point inside sphere, y >= center.y.
// ============================================================================

apsYHemisphereDomain::apsYHemisphereDomain(const math::Dir3& iCenter, float iRadius) {
    mSphere.mSphere.v.m128_f32[0] = iCenter.v.m128_f32[0];
    mSphere.mSphere.v.m128_f32[1] = iCenter.v.m128_f32[1];
    mSphere.mSphere.v.m128_f32[2] = iCenter.v.m128_f32[2];
    mSphere.mSphere.v.m128_f32[3] = iRadius;
}

apsYHemisphereDomain::apsYHemisphereDomain(const apsSphere& sphere) {
    mSphere.mSphere = sphere.mSphere;
}

void apsYHemisphereDomain::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 158,
                  "iNumDimensions == 3",
                  "apsHemisphereDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float radius = mSphere.mSphere.v.m128_f32[3];
    float radiusSq = radius * radius;
    float dRadius = radius * 2.0f;

    float x = 0, y = 0, z = 0;
    int iter_count = 0;
    do {
        x = apsMath::gDefaultRandomNumberGenerator.GetFloat() * dRadius - radius;
        y = apsMath::gDefaultRandomNumberGenerator.GetFloat() * radius;
        z = apsMath::gDefaultRandomNumberGenerator.GetFloat() * dRadius - radius;
        if (radiusSq > (x * x + y * y + z * z))
            break;
        if (iter_count >= 100 &&
            _tlAssert("source/apsSuppliedDomains.cpp", 187,
                      "iter_count < 100",
                      "The probability of this happening is about 6.2e-33...go buy a lottery ticket, or fix the random number generator"))
            __debugbreak();
        iter_count++;
    } while (iter_count < 100);

    if (iter_count > gHiHat_YHemisphereDomain)
        gHiHat_YHemisphereDomain = iter_count;

    oOutput[0] = mSphere.mSphere.v.m128_f32[0] + x;
    oOutput[1] = mSphere.mSphere.v.m128_f32[1] + y;
    oOutput[2] = mSphere.mSphere.v.m128_f32[2] + z;
}

unsigned int apsYHemisphereDomain::TestValue(int iNumDimensions, float* iValue) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 206,
                  "iNumDimensions == 3",
                  "apsYHemisphereDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float dx = mSphere.mSphere.v.m128_f32[0] - iValue[0];
    float dy = mSphere.mSphere.v.m128_f32[1] - iValue[1];
    float dz = mSphere.mSphere.v.m128_f32[2] - iValue[2];
    float radius = mSphere.mSphere.v.m128_f32[3];
    if (dy > 0.0f)
        return 0;
    return (radius * radius > (dx * dx + dy * dy + dz * dz)) ? 1 : 0;
}

// ============================================================================
// apsZHemisphereDomain — point inside sphere, z >= center.z.
// ============================================================================

apsZHemisphereDomain::apsZHemisphereDomain(const math::Dir3& iCenter, float iRadius) {
    mSphere.mSphere.v.m128_f32[0] = iCenter.v.m128_f32[0];
    mSphere.mSphere.v.m128_f32[1] = iCenter.v.m128_f32[1];
    mSphere.mSphere.v.m128_f32[2] = iCenter.v.m128_f32[2];
    mSphere.mSphere.v.m128_f32[3] = iRadius;
}

apsZHemisphereDomain::apsZHemisphereDomain(const apsSphere& sphere) {
    mSphere.mSphere = sphere.mSphere;
}

void apsZHemisphereDomain::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 250,
                  "iNumDimensions == 3",
                  "apsHemisphereDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float radius = mSphere.mSphere.v.m128_f32[3];
    float radiusSq = radius * radius;
    float dRadius = radius * 2.0f;

    float x = 0, y = 0, z = 0;
    int iter_count = 0;
    do {
        x = apsMath::gDefaultRandomNumberGenerator.GetFloat() * dRadius - radius;
        y = apsMath::gDefaultRandomNumberGenerator.GetFloat() * dRadius - radius;
        z = apsMath::gDefaultRandomNumberGenerator.GetFloat() * radius;
        if (radiusSq > (x * x + y * y + z * z))
            break;
        if (iter_count >= 100 &&
            _tlAssert("source/apsSuppliedDomains.cpp", 277,
                      "iter_count < 100",
                      "The probability of this happening is about 6.2e-33...go buy a lottery ticket, or fix the random number generator"))
            __debugbreak();
        iter_count++;
    } while (iter_count < 100);

    if (iter_count > gHiHat_ZHemisphereDomain)
        gHiHat_ZHemisphereDomain = iter_count;

    oOutput[0] = mSphere.mSphere.v.m128_f32[0] + x;
    oOutput[1] = mSphere.mSphere.v.m128_f32[1] + y;
    oOutput[2] = mSphere.mSphere.v.m128_f32[2] + z;
}

unsigned int apsZHemisphereDomain::TestValue(int iNumDimensions, float* iValue) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 297,
                  "iNumDimensions == 3",
                  "apsZHemisphereDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float dx = mSphere.mSphere.v.m128_f32[0] - iValue[0];
    float dy = mSphere.mSphere.v.m128_f32[1] - iValue[1];
    float dz = mSphere.mSphere.v.m128_f32[2] - iValue[2];
    float radius = mSphere.mSphere.v.m128_f32[3];
    if (dz > 0.0f)
        return 0;
    return (radius * radius > (dx * dx + dy * dy + dz * dz)) ? 1 : 0;
}

// ============================================================================
// apsSphereSurfaceDomain — point on the surface of a sphere.
// ============================================================================

apsSphereSurfaceDomain::apsSphereSurfaceDomain(const math::Dir3& iCenter, float iRadius) {
    mSphere.mSphere.v.m128_f32[0] = iCenter.v.m128_f32[0];
    mSphere.mSphere.v.m128_f32[1] = iCenter.v.m128_f32[1];
    mSphere.mSphere.v.m128_f32[2] = iCenter.v.m128_f32[2];
    mSphere.mSphere.v.m128_f32[3] = iRadius;
}

apsSphereSurfaceDomain::apsSphereSurfaceDomain(const apsSphere& sphere) {
    mSphere.mSphere = sphere.mSphere;
}

void apsSphereSurfaceDomain::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 338,
                  "iNumDimensions == 3",
                  "apsSphereSurfaceDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    // Random direction in [-1,1]^3, normalized to the sphere surface.
    float x = apsMath::gDefaultRandomNumberGenerator.GetFloat() * 2.0f - 1.0f;
    float y = apsMath::gDefaultRandomNumberGenerator.GetFloat() * 2.0f - 1.0f;
    float z = apsMath::gDefaultRandomNumberGenerator.GetFloat() * 2.0f - 1.0f;

    float len = sqrtf(x * x + y * y + z * z);
    float radius = mSphere.mSphere.v.m128_f32[3];

    oOutput[0] = mSphere.mSphere.v.m128_f32[0] + (x / len) * radius;
    oOutput[1] = mSphere.mSphere.v.m128_f32[1] + (y / len) * radius;
    oOutput[2] = mSphere.mSphere.v.m128_f32[2] + (z / len) * radius;
}

unsigned int apsSphereSurfaceDomain::TestValue(int iNumDimensions, float* iValue) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 364,
                  "iNumDimensions == 3",
                  "apsSphereSurfaceDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float dx = mSphere.mSphere.v.m128_f32[0] - iValue[0];
    float dy = mSphere.mSphere.v.m128_f32[1] - iValue[1];
    float dz = mSphere.mSphere.v.m128_f32[2] - iValue[2];
    float radius = mSphere.mSphere.v.m128_f32[3];
    return (radius * radius > (dx * dx + dy * dy + dz * dz)) ? 1 : 0;
}

// ============================================================================
// apsLineDomain — uniform point on a line segment.
// ============================================================================

apsLineDomain::apsLineDomain(const math::Dir3& iMin, const math::Dir3& iMax) {
    mMin.v = iMin.v;
    mDelta.v = _mm_sub_ps(iMax.v, iMin.v);
}

void apsLineDomain::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 399,
                  "iNumDimensions == 3",
                  "apsLineDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float t = apsMath::gDefaultRandomNumberGenerator.GetFloat();
    oOutput[0] = mMin.v.m128_f32[0] + mDelta.v.m128_f32[0] * t;
    oOutput[1] = mMin.v.m128_f32[1] + mDelta.v.m128_f32[1] * t;
    oOutput[2] = mMin.v.m128_f32[2] + mDelta.v.m128_f32[2] * t;
}

unsigned int apsLineDomain::TestValue(int iNumDimensions, float* iValue) const {
    return 0;
}

// ============================================================================
// apsDiscDomain — uniform point on a disc (in the plane of iNormal).
// Ctor builds an orthonormal frame {mXOff, mYOff} in the plane from iNormal.
// ============================================================================

apsDiscDomain::apsDiscDomain(const math::Dir3& iCenter, const math::Dir3& iNormal, float iRadius) {
    mCenter.mSphere.v.m128_f32[0] = iCenter.v.m128_f32[0];
    mCenter.mSphere.v.m128_f32[1] = iCenter.v.m128_f32[1];
    mCenter.mSphere.v.m128_f32[2] = iCenter.v.m128_f32[2];
    mCenter.mSphere.v.m128_f32[3] = iRadius;

    // n = normalize(iNormal)
    math::Dir3 n;
    {
        float len = sqrtf(iNormal.v.m128_f32[0] * iNormal.v.m128_f32[0] +
                          iNormal.v.m128_f32[1] * iNormal.v.m128_f32[1] +
                          iNormal.v.m128_f32[2] * iNormal.v.m128_f32[2]);
        n.v = _mm_mul_ps(iNormal.v, _mm_set1_ps(1.0f / len));
    }

    // mXOff = cross(n, YAxis); degenerate fallback cross(n, ZAxis)
    const float nX = n.v.m128_f32[0], nY = n.v.m128_f32[1], nZ = n.v.m128_f32[2];

    mXOff.v.m128_f32[0] = -nZ;          // (0,1,0): -nz, 0, nx
    mXOff.v.m128_f32[1] = 0.0f;
    mXOff.v.m128_f32[2] = nX;
    mXOff.v.m128_f32[3] = 0.0f;

    {
        float l2 = mXOff.v.m128_f32[0] * mXOff.v.m128_f32[0] +
                   mXOff.v.m128_f32[1] * mXOff.v.m128_f32[1] +
                   mXOff.v.m128_f32[2] * mXOff.v.m128_f32[2];
        if (l2 < 0.2f) {
            mXOff.v.m128_f32[0] = nY;   // (0,0,1): ny, -nx, 0
            mXOff.v.m128_f32[1] = -nX;
            mXOff.v.m128_f32[2] = 0.0f;
            mXOff.v.m128_f32[3] = 0.0f;
        }
    }

    {
        float len = sqrtf(mXOff.v.m128_f32[0] * mXOff.v.m128_f32[0] +
                          mXOff.v.m128_f32[1] * mXOff.v.m128_f32[1] +
                          mXOff.v.m128_f32[2] * mXOff.v.m128_f32[2]);
        mXOff.v = _mm_mul_ps(mXOff.v, _mm_set1_ps(1.0f / len));
    }

    // mYOff = cross(n, mXOff), normalized
    {
        const float xX = mXOff.v.m128_f32[0], xY = mXOff.v.m128_f32[1], xZ = mXOff.v.m128_f32[2];
        mYOff.v.m128_f32[0] = nY * xZ - nZ * xY;
        mYOff.v.m128_f32[1] = nZ * xX - nX * xZ;
        mYOff.v.m128_f32[2] = nX * xY - nY * xX;
        mYOff.v.m128_f32[3] = 0.0f;
    }

    {
        float len = sqrtf(mYOff.v.m128_f32[0] * mYOff.v.m128_f32[0] +
                          mYOff.v.m128_f32[1] * mYOff.v.m128_f32[1] +
                          mYOff.v.m128_f32[2] * mYOff.v.m128_f32[2]);
        mYOff.v = _mm_mul_ps(mYOff.v, _mm_set1_ps(1.0f / len));
    }
}

void apsDiscDomain::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 448,
                  "iNumDimensions == 3",
                  "apsDiscDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float radius = mCenter.mSphere.v.m128_f32[3];
    float radiusSq = radius * radius;
    float dRadius = radius * 2.0f;

    float x = 0, y = 0;
    int iter_count = 0;
    do {
        x = apsMath::gDefaultRandomNumberGenerator.GetFloat() * dRadius - radius;
        y = apsMath::gDefaultRandomNumberGenerator.GetFloat() * dRadius - radius;
        if (radiusSq >= (x * x + y * y))
            break;
        if (iter_count >= 100 &&
            _tlAssert("source/apsSuppliedDomains.cpp", 468,
                      "iter_count < 100",
                      "Extremely low probability occurrence...panic"))
            __debugbreak();
        iter_count++;
    } while (iter_count < 100);

    if (iter_count > gHiHat_DiscDomain)
        gHiHat_DiscDomain = iter_count;

    oOutput[0] = mCenter.mSphere.v.m128_f32[0] + mXOff.v.m128_f32[0] * x + mYOff.v.m128_f32[0] * y;
    oOutput[1] = mCenter.mSphere.v.m128_f32[1] + mXOff.v.m128_f32[1] * x + mYOff.v.m128_f32[1] * y;
    oOutput[2] = mCenter.mSphere.v.m128_f32[2] + mXOff.v.m128_f32[2] * x + mYOff.v.m128_f32[2] * y;
}

unsigned int apsDiscDomain::TestValue(int iNumDimensions, float* iValue) const {
    return 0;
}

// ============================================================================
// apsCircleDomain — point on a circle (fixed radius) in the plane of iNormal.
// ============================================================================

apsCircleDomain::apsCircleDomain(const math::Dir3& iCenter, const math::Dir3& iNormal, float iRadius) {
    mCenter.mSphere.v.m128_f32[0] = iCenter.v.m128_f32[0];
    mCenter.mSphere.v.m128_f32[1] = iCenter.v.m128_f32[1];
    mCenter.mSphere.v.m128_f32[2] = iCenter.v.m128_f32[2];
    mCenter.mSphere.v.m128_f32[3] = iRadius;

    // n = normalize(iNormal)
    math::Dir3 n;
    {
        float len = sqrtf(iNormal.v.m128_f32[0] * iNormal.v.m128_f32[0] +
                          iNormal.v.m128_f32[1] * iNormal.v.m128_f32[1] +
                          iNormal.v.m128_f32[2] * iNormal.v.m128_f32[2]);
        n.v = _mm_mul_ps(iNormal.v, _mm_set1_ps(1.0f / len));
    }

    const math::Dir3 YAxis = { _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f) };
    const math::Dir3 ZAxis = { _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f) };

    mXOff.v = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(n.v, n.v, 0x12), _mm_shuffle_ps(YAxis.v, YAxis.v, 0x09)),
                         _mm_mul_ps(_mm_shuffle_ps(n.v, n.v, 0x09), _mm_shuffle_ps(YAxis.v, YAxis.v, 0x12)));

    {
        float l2 = mXOff.v.m128_f32[0] * mXOff.v.m128_f32[0] +
                   mXOff.v.m128_f32[1] * mXOff.v.m128_f32[1] +
                   mXOff.v.m128_f32[2] * mXOff.v.m128_f32[2];
        if (l2 < 0.2f) {
            mXOff.v = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(n.v, n.v, 0x12), _mm_shuffle_ps(ZAxis.v, ZAxis.v, 0x09)),
                                 _mm_mul_ps(_mm_shuffle_ps(n.v, n.v, 0x09), _mm_shuffle_ps(ZAxis.v, ZAxis.v, 0x12)));
        }
    }

    {
        float len = sqrtf(mXOff.v.m128_f32[0] * mXOff.v.m128_f32[0] +
                          mXOff.v.m128_f32[1] * mXOff.v.m128_f32[1] +
                          mXOff.v.m128_f32[2] * mXOff.v.m128_f32[2]);
        mXOff.v = _mm_mul_ps(mXOff.v, _mm_set1_ps(1.0f / len));
    }

    mYOff.v = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(n.v, n.v, 0x12), _mm_shuffle_ps(mXOff.v, mXOff.v, 0x09)),
                         _mm_mul_ps(_mm_shuffle_ps(n.v, n.v, 0x09), _mm_shuffle_ps(mXOff.v, mXOff.v, 0x12)));

    {
        float len = sqrtf(mYOff.v.m128_f32[0] * mYOff.v.m128_f32[0] +
                          mYOff.v.m128_f32[1] * mYOff.v.m128_f32[1] +
                          mYOff.v.m128_f32[2] * mYOff.v.m128_f32[2]);
        mYOff.v = _mm_mul_ps(mYOff.v, _mm_set1_ps(1.0f / len));
    }
}

void apsCircleDomain::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions != 3 &&
        _tlAssert("source/apsSuppliedDomains.cpp", 522,
                  "iNumDimensions == 3",
                  "apsCircleDomain is specific to 3 dimensions at the moment"))
        __debugbreak();

    float radius = mCenter.mSphere.v.m128_f32[3];
    float angle = apsMath::gDefaultRandomNumberGenerator.GetFloat() * 6.2831855f;

    float s = sinf(angle), c = cosf(angle);

    oOutput[0] = mCenter.mSphere.v.m128_f32[0] + mXOff.v.m128_f32[0] * (c * radius) + mYOff.v.m128_f32[0] * (s * radius);
    oOutput[1] = mCenter.mSphere.v.m128_f32[1] + mXOff.v.m128_f32[1] * (c * radius) + mYOff.v.m128_f32[1] * (s * radius);
    oOutput[2] = mCenter.mSphere.v.m128_f32[2] + mXOff.v.m128_f32[2] * (c * radius) + mYOff.v.m128_f32[2] * (s * radius);
}

unsigned int apsCircleDomain::TestValue(int iNumDimensions, float* iValue) const {
    return 0;
}
