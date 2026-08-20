// ============================================================================
// apsSuppliedDomains — concrete particle source domains.
// Source: c:\cod\code\tl\aeps\source\apsSuppliedDomains.cpp
// Verified against IDA (aeps_xboxr:apsSuppliedDomains.o), 26 non-inline funcs:
//   apsDomain::Fixup               @0x810770  (?Fixup@apsDomain@@QAEXABUapsFixupParams@@@Z)
//   apsSphereDomain GetValue/TestValue        @0x8107C0 / 0x810A10
//   apsYHemisphereDomain GetValue/TestValue   @0x810B20 / 0x810D80
//   apsZHemisphereDomain GetValue/TestValue   @0x810EB0 / 0x811110
//   apsSphereSurfaceDomain GetValue/TestValue @0x811240 / 0x811420
//   apsLineDomain GetValue/TestValue          @0x811530 / 0x811610
//   apsDiscDomain GetValue/TestValue          @0x811620 / 0x8118B0
//   apsCircleDomain GetValue/TestValue        @0x811E50 / 0x8118C0
//   ctors: (Dir3,f) + (apsSphere) per sphere/hemi/surface; (Dir3,Dir3) line;
//          (Dir3,Dir3,f) disc/circle.
// Data: gHiHat_SphereDomain/YHemisphere/ZHemisphere/DiscDomain (@0x10E0A58+),
//       apsMath::FloatRand() no-arg inline COMDAT.
// GetId FourCCs: 'Sphe' 'YHSp' 'ZHSp' 'SpSu' 'Line' 'Disc' 'Crcl'
//                (apsRegister.o inline COMDATs). GetVersion = 1.0f.
// ============================================================================
#ifndef COD3_AEPS_APSSUPPLIEDDOMAINS_H
#define COD3_AEPS_APSSUPPLIEDDOMAINS_H

#include "core/math_types.h"

#include "apsAction.h"      // apsDomain, apsVirtualBase
#include "apsMath.h"        // apsMath::gDefaultRandomNumberGenerator

#include "apsRetrieveVtable.h"

namespace apsMath {
float FloatRand();
}

// apsSuppliedDomains.o data (global namespace per map mangling @@3HA):
extern int gHiHat_SphereDomain;         // ?gHiHat_SphereDomain@@3HA
extern int gHiHat_YHemisphereDomain;    // ?gHiHat_YHemisphereDomain@@3HA
extern int gHiHat_ZHemisphereDomain;    // ?gHiHat_ZHemisphereDomain@@3HA
extern int gHiHat_DiscDomain;           // ?gHiHat_DiscDomain@@3HA

// ============================================================================
// apsSphereDomain — uniform point inside a sphere (rejection sampled).
// Layout (32 bytes): apsDomain@0x00 + mSphere@0x10 (apsSphere).
// ============================================================================
class apsSphereDomain : public apsDomain {
public:
    apsSphereDomain(const math::Dir3& iCenter, float iRadius);   // ??0apsSphereDomain@@QAE@ABVDir3@math@@M@Z
    apsSphereDomain(const apsSphere& sphere);                    // ??0apsSphereDomain@@QAE@ABUapsSphere@@@Z
    void GetValue(int iNumDimensions, float* oOutput) const;     // ?GetValue@apsSphereDomain@@UBEXHPAM@Z
    unsigned int TestValue(int iNumDimensions, float* iValue) const;  // ?TestValue@apsSphereDomain@@UBEIHPAM@Z
    unsigned int GetId() const { return 0x53706865; }            // 'Sphe'
    float GetVersion() const { return 1.0f; }
    apsSphere mSphere;   // +0x10 (center.xyz + radius in w)

    APS_DECLARE_RETRIEVE_LEAF(apsSphereDomain)};
static_assert(sizeof(apsSphereDomain) == 0x20, "apsSphereDomain size mismatch");

// ============================================================================
// apsYHemisphereDomain — point inside sphere, y >= center (rejection sampled).
// ============================================================================
class apsYHemisphereDomain : public apsDomain {
public:
    apsYHemisphereDomain(const math::Dir3& iCenter, float iRadius);  // ??0apsYHemisphereDomain@@QAE@ABVDir3@math@@M@Z
    apsYHemisphereDomain(const apsSphere& sphere);                   // ??0apsYHemisphereDomain@@QAE@ABUapsSphere@@@Z
    void GetValue(int iNumDimensions, float* oOutput) const;         // ?GetValue@apsYHemisphereDomain@@UBEXHPAM@Z
    unsigned int TestValue(int iNumDimensions, float* iValue) const; // ?TestValue@apsYHemisphereDomain@@UBEIHPAM@Z
    unsigned int GetId() const { return 0x59485370; }                // 'YHSp'
    float GetVersion() const { return 1.0f; }
    apsSphere mSphere;   // +0x10

    APS_DECLARE_RETRIEVE_LEAF(apsYHemisphereDomain)};
static_assert(sizeof(apsYHemisphereDomain) == 0x20, "apsYHemisphereDomain size mismatch");

// ============================================================================
// apsZHemisphereDomain — point inside sphere, z >= center (rejection sampled).
// ============================================================================
class apsZHemisphereDomain : public apsDomain {
public:
    apsZHemisphereDomain(const math::Dir3& iCenter, float iRadius);  // ??0apsZHemisphereDomain@@QAE@ABVDir3@math@@M@Z
    apsZHemisphereDomain(const apsSphere& sphere);                   // ??0apsZHemisphereDomain@@QAE@ABUapsSphere@@@Z
    void GetValue(int iNumDimensions, float* oOutput) const;         // ?GetValue@apsZHemisphereDomain@@UBEXHPAM@Z
    unsigned int TestValue(int iNumDimensions, float* iValue) const; // ?TestValue@apsZHemisphereDomain@@UBEIHPAM@Z
    unsigned int GetId() const { return 0x5A485370; }                // 'ZHSp'
    float GetVersion() const { return 1.0f; }
    apsSphere mSphere;   // +0x10

    APS_DECLARE_RETRIEVE_LEAF(apsZHemisphereDomain)};
static_assert(sizeof(apsZHemisphereDomain) == 0x20, "apsZHemisphereDomain size mismatch");

// ============================================================================
// apsSphereSurfaceDomain — point on the surface of a sphere.
// ============================================================================
class apsSphereSurfaceDomain : public apsDomain {
public:
    apsSphereSurfaceDomain(const math::Dir3& iCenter, float iRadius);  // ??0apsSphereSurfaceDomain@@QAE@ABVDir3@math@@M@Z
    apsSphereSurfaceDomain(const apsSphere& sphere);                   // ??0apsSphereSurfaceDomain@@QAE@ABUapsSphere@@@Z
    void GetValue(int iNumDimensions, float* oOutput) const;           // ?GetValue@apsSphereSurfaceDomain@@UBEXHPAM@Z
    unsigned int TestValue(int iNumDimensions, float* iValue) const;   // ?TestValue@apsSphereSurfaceDomain@@UBEIHPAM@Z
    unsigned int GetId() const { return 0x53705375; }                  // 'SpSu'
    float GetVersion() const { return 1.0f; }
    apsSphere mSphere;   // +0x10

    APS_DECLARE_RETRIEVE_LEAF(apsSphereSurfaceDomain)};
static_assert(sizeof(apsSphereSurfaceDomain) == 0x20, "apsSphereSurfaceDomain size mismatch");

// ============================================================================
// apsLineDomain — uniform point on a line segment (mMin..mMax).
// Layout (48 bytes): apsDomain@0x00 + mMin@0x10 + mDelta@0x20.
// TestValue returns 0 (not implemented).
// ============================================================================
class apsLineDomain : public apsDomain {
public:
    apsLineDomain(const math::Dir3& iMin, const math::Dir3& iMax);  // ??0apsLineDomain@@QAE@ABVDir3@math@@0@Z
    void GetValue(int iNumDimensions, float* oOutput) const;        // ?GetValue@apsLineDomain@@UBEXHPAM@Z
    unsigned int TestValue(int iNumDimensions, float* iValue) const;    // ?TestValue@apsLineDomain@@UBEIHPAM@Z
    unsigned int GetId() const { return 0x4C696E65; }               // 'Line'
    float GetVersion() const { return 1.0f; }
    math::Dir3 mMin;    // +0x10
    math::Dir3 mDelta;  // +0x20

    APS_DECLARE_RETRIEVE_LEAF(apsLineDomain)};
static_assert(sizeof(apsLineDomain) == 0x30, "apsLineDomain size mismatch");

// ============================================================================
// apsDiscDomain — uniform point on a disc in a plane (rejection sampled).
// Layout (64 bytes): apsDomain@0x00 + mCenter@0x10 + mXOff@0x20 + mYOff@0x30.
// TestValue returns 0 (not implemented).
// ============================================================================
class apsDiscDomain : public apsDomain {
public:
    apsDiscDomain(const math::Dir3& iCenter, const math::Dir3& iNormal, float iRadius);  // ??0apsDiscDomain@@QAE@ABVDir3@math@@0M@Z
    void GetValue(int iNumDimensions, float* oOutput) const;   // ?GetValue@apsDiscDomain@@UBEXHPAM@Z
    unsigned int TestValue(int iNumDimensions, float* iValue) const;  // ?TestValue@apsDiscDomain@@UBEIHPAM@Z
    unsigned int GetId() const { return 0x44697363; }          // 'Disc'
    float GetVersion() const { return 1.0f; }
    apsSphere  mCenter;   // +0x10 (center.xyz + radius in w)
    math::Dir3 mXOff;     // +0x20 (in-plane basis)
    math::Dir3 mYOff;     // +0x30

    APS_DECLARE_RETRIEVE_LEAF(apsDiscDomain)};
static_assert(sizeof(apsDiscDomain) == 0x40, "apsDiscDomain size mismatch");

// ============================================================================
// apsCircleDomain — point on a circle in a plane (fixed radius).
// TestValue returns 0 (not implemented).
// ============================================================================
class apsCircleDomain : public apsDomain {
public:
    apsCircleDomain(const math::Dir3& iCenter, const math::Dir3& iNormal, float iRadius);  // ??0apsCircleDomain@@QAE@ABVDir3@math@@0M@Z
    void GetValue(int iNumDimensions, float* oOutput) const;   // ?GetValue@apsCircleDomain@@UBEXHPAM@Z
    unsigned int TestValue(int iNumDimensions, float* iValue) const;  // ?TestValue@apsCircleDomain@@UBEIHPAM@Z
    unsigned int GetId() const { return 0x4372636C; }          // 'Crcl'
    float GetVersion() const { return 1.0f; }
    apsSphere  mCenter;   // +0x10
    math::Dir3 mXOff;     // +0x20
    math::Dir3 mYOff;     // +0x30

    APS_DECLARE_RETRIEVE_LEAF(apsCircleDomain)};
static_assert(sizeof(apsCircleDomain) == 0x40, "apsCircleDomain size mismatch");

#endif // COD3_AEPS_APSSUPPLIEDDOMAINS_H
