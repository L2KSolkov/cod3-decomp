// ============================================================================
// apsRegister — APS object registry: creates renderers/domains/actions from
// FourCC + name, and builds the vtable fixup tables.
// Source: c:\cod\code\tl\aeps\source\apsRegister.cpp + include\apsRegister.h
// Verified against IDA (aeps_xboxr:apsRegister.o), 6 non-inline funcs:
//   GetNewSimpleMeshRenderer     @0x7F3E00
//   GetNewShrimpRenderer         @0x7F3F30
//   apsGetRenderer               @0x7F43B0
//   apsGetDomain                 @0x7F47B0
//   apsGetAction                 @0x7F54F0
//   apsLoadEffectInplace         @0x7F6460
// Inline COMDATs emitted here per the map: apsParam operators, GetEnum,
// Create*Domain, GetNew*Renderer templates, template domains (Box/Point/Bump),
// RetrieveVtable + APS_VTABLE_RETRIEVING_CTOR ctors for every class.
// ============================================================================
#ifndef COD3_AEPS_APSREGISTER_H
#define COD3_AEPS_APSREGISTER_H

#include "apsAction.h"        // apsVirtualBase, apsArray, apsDomain, apsAction
#include "apsSuppliedDomains.h"
#include "apsSuppliedActions.h"
#include "apsEffect.h"        // apsEffectTemplate
#include "apsRenderer.h"
#include "apsBillboardRenderer.h"
#include "apsUVARenderer.h"
#include "apsUVARectangleRenderer.h"
#include "apsColorUVARenderer.h"
#include "apsColorUVARectangleRenderer.h"
#include "apsRectangleRenderer.h"
#include "apsColorRectangleRenderer.h"
#include "apsColorBillboardRenderer.h"
#include "apsSimpleMeshRenderer.h"
#include "apsShrimpRenderer.h"
#include "filesystem/apk.h"
#include "core/tlFixedString.h"

#include <string.h>

#include "apsRetrieveVtable.h"
#include "apsParam.h"

// ngl mesh/texture (from ngl headers)
struct nglMesh;
struct nglTexture;

// ============================================================================
// apsParam — 16-byte typed parameter (see apsParam.h).
// ============================================================================

inline math::Dir3::Packed apsParam2apsVector3Packed(const apsParam& param);  // ?apsParam2apsVector3Packed@@YA?AUPacked@Dir3@math@@ABVapsParam@@@Z
inline math::Dir3 apsParam2apsVector3(apsParam& param);                      // ?apsParam2apsVector3@@YA?AVDir3@math@@ABVapsParam@@@Z

// ============================================================================
// GetEnum<apsEBlendMode, N> — enum-valued param reader.
// ============================================================================
template <typename T, int tNumEnums>
T GetEnum(const apsParam& p);   // ??$GetEnum@W4apsEBlendMode@@$04@@YA?AW4apsEBlendMode@@ABVapsParam@@@Z

// ============================================================================
// Template domains (1/3-dimension box/point/bump). Defined here; their
// GetValue/TestValue/GetId/GetVersion/retrieve-ctors are inline COMDATs in
// apsRegister.o. All derive from apsDomain.
// ============================================================================
template <int tDimension>
class apsBoxDomain : public apsDomain {
public:
    float mMin[tDimension];   // +0x04
    float mMax[tDimension];   // +0x04+tDimension*4

    apsBoxDomain(float* iMinMax);                    // ??0?$apsBoxDomain@$00@@QAE@PAM@Z
    APS_DECLARE_RETRIEVE_LEAF_T(apsBoxDomain)
    void GetValue(int iNumDimensions, float* oOutput) const;
    unsigned int TestValue(int iNumDimensions, float* iValue) const;
    unsigned int GetId() const;
    float GetVersion() const { return 1.0f; }
};

template <int tDimension>
class apsPointDomain : public apsDomain {
public:
    float mVal[tDimension];   // +0x04

    apsPointDomain(float* iVal);                      // ??0?$apsPointDomain@$00@@QAE@PAM@Z
    APS_DECLARE_RETRIEVE_LEAF_T(apsPointDomain)
    void GetValue(int iNumDimensions, float* oOutput) const;
    unsigned int TestValue(int iNumDimensions, float* iValue) const;
    unsigned int GetId() const;
    float GetVersion() const { return 1.0f; }
};

template <int tDimension>
class apsBumpDomain : public apsDomain {
public:
    float mMin[tDimension];   // +0x04
    float mMax[tDimension];   // +0x04+tDimension*4

    apsBumpDomain(float* iMinMax);                    // ??0?$apsBumpDomain@$00@@QAE@PAM@Z
    APS_DECLARE_RETRIEVE_LEAF_T(apsBumpDomain)
    void GetValue(int iNumDimensions, float* oOutput) const;
    unsigned int TestValue(int iNumDimensions, float* iValue) const;
    unsigned int GetId() const;
    float GetVersion() const { return 1.0f; }
};

// ============================================================================
// Factory functions (inline COMDATs in apsRegister.o)
// ============================================================================
apsBoxDomain<1>*  CreateBoxDomain(float iMin, float iMax);
apsBoxDomain<3>*  CreateBoxDomain(const math::Dir3::Packed& iMin, const math::Dir3::Packed& iMax);
apsPointDomain<1>* CreatePointDomain(float iVal);
apsPointDomain<3>* CreatePointDomain(const math::Dir3::Packed& iVal);
apsBumpDomain<1>* CreateBumpDomain(float iMin, float iMax);
apsBumpDomain<3>* CreateBumpDomain(const math::Dir3::Packed& iMin, const math::Dir3::Packed& iMax);

// ============================================================================
// The 6 non-inline functions (apsRegister.o)
// ============================================================================
apsSimpleMeshRenderer* GetNewSimpleMeshRenderer(const apsArray<apsParam>& params);
apsShrimpRenderer*     GetNewShrimpRenderer(const apsArray<apsParam>& params);
apsRenderer*           apsGetRenderer(const char* name, const unsigned int* id,
                                      const apsArray<apsParam>& params,
                                      apsFixupParams* fixups);
apsDomain*             apsGetDomain(const char* name, const unsigned int* id,
                                    const apsArray<apsParam>& params,
                                    apsFixupParams* fixups);
apsAction*             apsGetAction(const char* name, const unsigned int* id,
                                    apsFixupParams* fixups);
apsEffectTemplate*     apsLoadEffectInplace(apk::apkFile* File, apk::apkFileEntry* Entry);

// sFixupParams static (apsRegister.cpp)
struct apsRegisterStatics {
    static apsFixupParams sFixupParams;   // @0x10DEDF0
    static bool sInit;                    // @0x10DF600
};

#endif // COD3_AEPS_APSREGISTER_H
