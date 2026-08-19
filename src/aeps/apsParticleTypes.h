// ============================================================================
// apsParticleTypes.h — particle types used as cNodeRenderer template args.
// Layouts are taken from the IDA type database / codmp_xboxr.xbe.h.
// ============================================================================
#ifndef COD3_AEPS_APSPARTICLETYPES_H
#define COD3_AEPS_APSPARTICLETYPES_H

#include "core/math_types.h"
#include "apsMath.h"

struct MeshParticleContext {
    math::Mat43 mLToS;          // +0x00
    math::Mat43 mLightDir;      // +0x40
    math::Mat43 mLightColor;    // +0x80
    math::Vector4 mAlpha;       // +0xC0
};
static_assert(sizeof(MeshParticleContext) == 0xD0, "MeshParticleContext size mismatch");

struct ShrimpParticle {
    math::Dir3::Packed mPos; // +0x00
    float mWidth;             // +0x0C
    float mAlpha;             // +0x10
    float mAngle;             // +0x14
    float mHeight;            // +0x18
    float mFrame;             // +0x1C
};
static_assert(sizeof(ShrimpParticle) == 0x20, "ShrimpParticle size mismatch");

struct MeshParticle {
    math::Dir3::Packed mPos; // +0x00
    float mAlpha;             // +0x0C
    apsQuaternion mOrientation; // +0x10
};
static_assert(sizeof(MeshParticle) == 0x20, "MeshParticle size mismatch");

struct BillboardParticle {};
struct RectangleParticle {};
struct ColorBillboardParticle {};
struct ColorRectangleParticle {};
struct UVAParticle {};
struct UVARectangleParticle {};
struct ColorUVAParticle {};
struct ColorUVARectangleParticle {};

#endif // COD3_AEPS_APSPARTICLETYPES_H
