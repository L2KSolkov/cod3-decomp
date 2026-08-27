// ============================================================================
// apsParticleTypes.h — particle types used as cNodeRenderer template args.
// Layouts are taken from the IDA type database / codmp_xboxr.xbe.h.
// ============================================================================
#ifndef COD3_AEPS_APSPARTICLETYPES_H
#define COD3_AEPS_APSPARTICLETYPES_H

#include "core/math_types.h"
#include "apsMath.h"

struct MeshParticleContext {
    MeshParticleContext();
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

    math::Dir3::Packed& GetPos();
    float GetWidth();
    float GetHeight();
    math::Vector4 GetColor();
    float GetFrame();
    float GetAngle();
};
static_assert(sizeof(ShrimpParticle) == 0x20, "ShrimpParticle size mismatch");

struct MeshParticle {
    math::Dir3::Packed mPos; // +0x00
    float mAlpha;             // +0x0C
    apsQuaternion mOrientation; // +0x10
};
static_assert(sizeof(MeshParticle) == 0x20, "MeshParticle size mismatch");

struct RectangleParticle {
    math::Dir3::Packed mPos;
    float mWidth;
    float mAlpha;
    float mAngle;
    float mHeight;
    float mAge;
    float mMaxAge;
    float mPad0;
    float mPad1;
    float mPad2;
    math::Vector4 mVelocity;
    math::Dir3::Packed& GetPos();
    math::Vector4 GetColor();
    float GetWidth();
    float GetHeight();
    float GetAngle();
    float GetFrame();
    math::Vector4& GetVelocity();
};
static_assert(sizeof(RectangleParticle) == 0x40, "RectangleParticle size mismatch");

struct BillboardParticle {
    math::Dir3::Packed mPos; // +0x00
    float mRadius;            // +0x0C
    float mAlpha;             // +0x10
    float mAngle;             // +0x14
    float mAge;               // +0x18
    float mMaxAge;            // +0x1C
    math::Vector4 mVelocity;  // +0x20

    math::Dir3::Packed& GetPos();
    math::Vector4 GetColor();
    float GetWidth();
    float GetHeight();
    float GetAngle();
    float GetFrame();
    math::Vector4& GetVelocity();
};
static_assert(sizeof(BillboardParticle) == 0x30, "BillboardParticle size mismatch");

struct ColorBillboardParticle {
    math::Dir3::Packed mPos;
    float mRadius;
    math::Vector4 mColor;
    float mAngle;
    float mAge;
    float mMaxAge;
    float mPad0;
    math::Vector4 mVelocity;
    math::Dir3::Packed& GetPos();
    math::Vector4& GetColor();
    float GetWidth();
    float GetHeight();
    float GetAngle();
    float GetFrame();
    math::Vector4& GetVelocity();
};
static_assert(sizeof(ColorBillboardParticle) == 0x40, "ColorBillboardParticle size mismatch");

struct ColorRectangleParticle {
    math::Dir3::Packed mPos;
    float mWidth;
    math::Vector4 mColor;
    float mAngle;
    float mHeight;
    float mAge;
    float mMaxAge;
    math::Vector4 mVelocity;
    math::Dir3::Packed& GetPos();
    math::Vector4& GetColor();
    float GetWidth();
    float GetHeight();
    float GetAngle();
    float GetFrame();
    math::Vector4& GetVelocity();
};
static_assert(sizeof(ColorRectangleParticle) == 0x40, "ColorRectangleParticle size mismatch");

struct UVAParticle {
    math::Dir3::Packed mPos;
    float mRadius;
    float mAlpha;
    float mAngle;
    float mFrame;
    float mAge;
    float mMaxAge;
    float mPad0;
    float mPad1;
    float mPad2;
    math::Vector4 mVelocity;
    math::Dir3::Packed& GetPos();
    math::Vector4 GetColor();
    float GetWidth();
    float GetHeight();
    float GetAngle();
    float GetFrame();
    math::Vector4& GetVelocity();
};
static_assert(sizeof(UVAParticle) == 0x40, "UVAParticle size mismatch");

struct UVARectangleParticle {
    math::Dir3::Packed mPos;
    float mWidth;
    float mAlpha;
    float mAngle;
    float mHeight;
    float mFrame;
    float mAge;
    float mMaxAge;
    float mPad0;
    float mPad1;
    math::Vector4 mVelocity;
    math::Dir3::Packed& GetPos();
    math::Vector4 GetColor();
    float GetWidth();
    float GetHeight();
    float GetAngle();
    float GetFrame();
    math::Vector4& GetVelocity();
};
static_assert(sizeof(UVARectangleParticle) == 0x40, "UVARectangleParticle size mismatch");

struct ColorUVAParticle {
    math::Dir3::Packed mPos;
    float mRadius;
    math::Vector4 mColor;
    float mAngle;
    float mFrame;
    float mAge;
    float mMaxAge;
    math::Vector4 mVelocity;
    math::Dir3::Packed& GetPos();
    math::Vector4& GetColor();
    float GetWidth();
    float GetHeight();
    float GetAngle();
    float GetFrame();
    math::Vector4& GetVelocity();
};
static_assert(sizeof(ColorUVAParticle) == 0x40, "ColorUVAParticle size mismatch");

struct ColorUVARectangleParticle {
    math::Dir3::Packed mPos;
    float mWidth;
    math::Vector4 mColor;
    float mAngle;
    float mHeight;
    float mFrame;
    float mAge;
    float mMaxAge;
    float mPad0;
    float mPad1;
    float mPad2;
    math::Vector4 mVelocity;
    math::Dir3::Packed& GetPos();
    math::Vector4& GetColor();
    float GetWidth();
    float GetHeight();
    float GetAngle();
    float GetFrame();
    math::Vector4& GetVelocity();
};
static_assert(sizeof(ColorUVARectangleParticle) == 0x50, "ColorUVARectangleParticle size mismatch");

#endif // COD3_AEPS_APSPARTICLETYPES_H
