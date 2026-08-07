// ============================================================================
// apsParam — 16-byte typed parameter used by the APS registry.
// Source: c:\cod\code\tl\aeps\include\apsRegister.h
// Standalone so renderer headers can declare their cArgs setters.
// ============================================================================
#ifndef COD3_AEPS_APSPARAM_H
#define COD3_AEPS_APSPARAM_H

#include "core/math_types.h"

struct nglMesh;
struct nglTexture;
class apsEffectTemplate;

// ============================================================================
// apsParamType — parameter type tags (values from IDA).
// ============================================================================
enum apsParamType {
    TEXTURE = 0,
    MESH = 1,
    INT32 = 3,
    FLOAT32 = 4,
    VECTOR3D = 5,
    COLOR = 8,
};

// ============================================================================
// apsParam — 16 bytes: mType + 12-byte mData union.
// Declared `class` (not struct) to match the map's V mangling
// (`ABV?$apsArray@VapsParam@@@@` etc.).
// ============================================================================
class apsParam {
public:
    enum apsParamType mType;   // +0x00
    union {
        nglTexture*      mTexture;   // +0x04
        nglMesh*         mMesh;
        unsigned int     mU32;
        int              mI32;
        float            mFloat;
        float            mVector3d[3];
        struct RGBA8888 {
            unsigned char r, g, b, a;
        } mColor;
        apsEffectTemplate* mEffect;
        unsigned int       mHash;
        void*              mShaderWorksShader;
    } mData;                       // +0x04 (12 bytes)

    apsParam() { mType = INT32; mData.mU32 = 0; }         // ??0apsParam@@QAE@XZ

    operator nglTexture*() const;   // ??BapsParam@@QBEPAUnglTexture@@XZ
    operator nglMesh*() const;      // ??BapsParam@@QBEPAUnglMesh@@XZ
    operator int() const;           // ??BapsParam@@QBEHXZ
    operator float() const;         // ??BapsParam@@QBEMXZ
    operator math::Dir3::Packed() const;            // ??BapsParam@@QBE?AUPacked@Dir3@math@@XZ
    operator math::Dir3() const;                    // ??BapsParam@@QBE?AVDir3@math@@XZ
    operator math::Vector4::Packed() const;         // ??BapsParam@@QBE?AUPacked@Vector4@math@@XZ
    unsigned int operator!=(apsParam rhs) const;    // ??9apsParam@@QBEIV0@@Z
};
static_assert(sizeof(apsParam) == 0x10, "apsParam size mismatch");

#endif // COD3_AEPS_APSPARAM_H
