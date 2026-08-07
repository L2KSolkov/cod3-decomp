// ============================================================================
// apsPFD — particle field descriptor.
// Declares which fields a particle carries, the per-particle stride, and the
// byte offset of each field within a particle record.
// Source: c:\cod\code\tl\aeps\include\apsPFD.h
// Inline accessors emitted in apsGroup.o:
//   apsPFD::HasField  @0x7FE9F0
//   apsPFD::GetOffset @0x7FEA10   (asserts at apsPFD.h:117)
// ============================================================================
#ifndef COD3_AEPS_APSPFD_H
#define COD3_AEPS_APSPFD_H

extern bool _tlAssert(const char* file, int line, const char* expr, const char* msg);

enum apsEPFDField {
    apsPFDField_Position = 0,
    apsPFDField_Radius = 1,
    apsPFDField_Width = 2,
    apsPFDField_Color = 3,
    apsPFDField_Alpha = 4,
    apsPFDField_Orientation = 5,
    apsPFDField_Angle = 6,
    apsPFDField_Height = 7,
    apsPFDField_UVAFrame = 8,
    apsPFDField_Age = 9,
    apsPFDField_MaxAge = 10,
    apsPFDField_Pad0 = 11,
    apsPFDField_Pad1 = 12,
    apsPFDField_Pad2 = 13,
    apsPFDField_Velocity = 14,
    apsPFDField_MaxAlpha = 15,
    apsPFDField_AngularVelocity = 16,
    apsPFDField_VectorAngularVelocity = 17,
    apsPFDField_Tangent = 18,
    apsPFDField_Normal = 19,
    apsPFDField_Binormal = 20,
    apsPFDField_Curvature = 21,
    apsPFDField_Torsion = 22,
    apsPFDField_DensityTime = 23,
    apsPFDField_TrajectoryPtr = 24,
    apsPFDField_TrajectorySpeed = 25,
    apsPFDField_TrajectoryDistance = 26,
    apsPFDField_Flags = 27,
    apsPFDField_PreviousSavedPosition = 28,
    apsPFDField_LastCollisionNormal = 29,
    apsPFDField_CollisionResultID = 30,
    apsPFDField_RaycastCountdown = 31,
    apsPFDField_FirstUserField = 32,
};

// Note: declared `class` (not struct) to match the original binary's MSVC
// mangling of apsGroup::Init (`ABVapsPFD` = const class apsPFD).
class apsPFD {
public:
    unsigned int  mFields;      // bitmask: bit i set if field i present
    int           mStride;      // bytes per particle
    unsigned char mOffsets[32]; // byte offset of field i within a particle

    apsPFD();  // defined in apsPFD.o (apsPFD ctor @0x8120A0)

    void AddFields(unsigned int iFields);      // @0x8120C0
    void RecomputeOffsets();                   // @0x812020

// apsPFD.o (non-inline): byte size of the given field.
static unsigned char GetFieldByteSize(apsEPFDField iField);

    static unsigned char sElementSizes[32];    // @0x1239190

    unsigned int HasField(apsEPFDField iField) const {
        return mFields & (1 << iField);
    }

    int GetOffset(apsEPFDField iField) const {
        if ((1 << iField) & mFields)
            return mOffsets[iField];
        _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsPFD.h", 117,
                  "mFields & (1 << iField)", "Can't get offset for missing field");
        return mOffsets[iField];
    }
};

// apsEffectTemplate.o / apsSuppliedActions.o inline COMDATs:
inline unsigned int apsGetFields(apsEPFDField iVal1) { return 1 << iVal1; }        // ?apsGetFields@@YAIW4apsEPFDField@@@Z
inline unsigned int apsGetFields(apsEPFDField iVal1, apsEPFDField iVal2) { return (1 << iVal2) | (1 << iVal1); }  // ?apsGetFields@@YAIW4apsEPFDField@@0@Z
inline unsigned int apsGetFields(apsEPFDField iVal1, apsEPFDField iVal2, apsEPFDField iVal3) { return (1 << iVal3) | (1 << iVal2) | (1 << iVal1); }  // ?apsGetFields@@YAIW4apsEPFDField@@000@Z
inline unsigned int apsGetFields(apsEPFDField iVal1, apsEPFDField iVal2, apsEPFDField iVal3, apsEPFDField iVal4) { return (1 << iVal4) | (1 << iVal3) | (1 << iVal2) | (1 << iVal1); }  // ?apsGetFields@@YAIW4apsEPFDField@@0000@Z
inline unsigned int apsGetFields(apsEPFDField iVal1, apsEPFDField iVal2, apsEPFDField iVal3, apsEPFDField iVal4, apsEPFDField iVal5) { return (1 << iVal5) | (1 << iVal4) | (1 << iVal3) | (1 << iVal2) | (1 << iVal1); }  // ?apsGetFields@@YAIW4apsEPFDField@@00000@Z
inline unsigned int apsGetFields(apsEPFDField iVal1, apsEPFDField iVal2, apsEPFDField iVal3, apsEPFDField iVal4, apsEPFDField iVal5, apsEPFDField iVal6) { return (1 << iVal6) | (1 << iVal5) | (1 << iVal4) | (1 << iVal3) | (1 << iVal2) | (1 << iVal1); }  // ?apsGetFields@@YAIW4apsEPFDField@@000000@Z

#endif // COD3_AEPS_APSPFD_H
