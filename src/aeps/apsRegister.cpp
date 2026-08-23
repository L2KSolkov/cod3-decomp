// ============================================================================
// apsRegister — APS object registry (6 non-inline funcs + inline helpers).
// Source: c:\cod\code\tl\aeps\source\apsRegister.cpp
// Verified against IDA (aeps_xboxr:apsRegister.o).
// apsGetRenderer/apsGetDomain/apsGetAction dispatch by FourCC id (and name),
// or build the global vtable fixup tables when called with fixups.
// ============================================================================

#include "apsRegister.h"
#include "apsSuppliedActions.h"

#include <string.h>
#include <new>

extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// apsParam inline operators (apsRegister.h declares; defined here)
// ============================================================================
apsParam::operator nglTexture*() const {
    if (mType != TEXTURE && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 91,
                                      "mType==TEXTURE", "incorrect parameter!"))
        __debugbreak();
    return mData.mTexture;
}
apsParam::operator nglMesh*() const {
    if (mType != MESH && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 92,
                                   "mType==MESH", "incorrect parameter!"))
        __debugbreak();
    return mData.mMesh;
}
apsParam::operator int() const {
    if (mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                    "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    return mData.mI32;
}
apsParam::operator float() const {
    if (mType != FLOAT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 95,
                                      "mType==FLOAT32", "incorrect parameter!"))
        __debugbreak();
    return mData.mFloat;
}
apsParam::operator math::Dir3::Packed() const {
    if (mType != VECTOR3D && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 99,
                                       "mType==VECTOR3D", "incorrect parameter!"))
        __debugbreak();
    math::Dir3::Packed r;
    r.x = mData.mVector3d[0]; r.y = mData.mVector3d[1]; r.z = mData.mVector3d[2];
    return r;
}
apsParam::operator math::Dir3() const {
    if (mType != VECTOR3D && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 100,
                                       "mType==VECTOR3D", "incorrect parameter!"))
        __debugbreak();
    math::Dir3 r;
    r.v = _mm_set_ps(0.0f, mData.mVector3d[2], mData.mVector3d[1], mData.mVector3d[0]);
    return r;
}
apsParam::operator math::Vector4::Packed() const {
    if (mType != COLOR && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 106,
                                    "mType==COLOR", "incorrect parameter!"))
        __debugbreak();
    math::Vector4::Packed r;
    r.x = mData.mColor.r * 0.0039215689f;
    r.y = mData.mColor.g * 0.0039215689f;
    r.z = mData.mColor.b * 0.0039215689f;
    r.w = mData.mColor.a * 0.0039215689f;
    return r;
}
unsigned int apsParam::operator!=(apsParam rhs) const {
    if (mType != rhs.mType)
        return 1;
    switch (mType) {
    case TEXTURE: case MESH: case INT32: case FLOAT32: case COLOR:
    case 2: case 6: case 7:
        return mData.mU32 != rhs.mData.mU32;
    case VECTOR3D:
        if (mData.mU32 != rhs.mData.mU32 &&
            (int&)mData.mVector3d[1] != (int&)rhs.mData.mVector3d[1] &&
            (int&)mData.mVector3d[2] != (int&)rhs.mData.mVector3d[2])
            return 1;
        return 0;
    default:
        if (_tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 148, "0", "unregistered type"))
            __debugbreak();
        return 1;
    }
}
math::Dir3::Packed apsParam2apsVector3Packed(const apsParam& param) {
    if (param.mType != VECTOR3D && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 99,
                                             "mType==VECTOR3D", "incorrect parameter!"))
        __debugbreak();
    math::Dir3::Packed r;
    r.x = param.mData.mVector3d[0]; r.y = param.mData.mVector3d[1]; r.z = param.mData.mVector3d[2];
    return r;
}
math::Dir3 apsParam2apsVector3(apsParam& param) {
    return (math::Dir3)param;
}

// ============================================================================
// GetEnum<apsEBlendMode, 5>
// ============================================================================
template <>
inline apsEBlendMode GetEnum<apsEBlendMode, 5>(const apsParam& p) {
    if (p.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                      "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    int v = p.mData.mI32;
    if (v < 6)
        return (apsEBlendMode)v;
    if (!_tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 210, "0", "enum out of range"))
        __debugbreak();
    return (apsEBlendMode)v;
}

// ============================================================================
// Template domains: ctors + GetValue / TestValue / GetId
// ============================================================================
template <int tDimension>
inline apsBoxDomain<tDimension>::apsBoxDomain(float* iMinMax) {
    for (int i = 0; i < tDimension; ++i) {
        mMin[i] = iMinMax[i];
        mMax[i] = iMinMax[tDimension + i];
        if (mMin[i] > mMax[i]) { float t = mMin[i]; mMin[i] = mMax[i]; mMax[i] = t; }
    }
}
template <>
inline unsigned int apsBoxDomain<1>::GetId() const { return 828654200; }
template <>
inline unsigned int apsBoxDomain<3>::GetId() const { return 862208632; }
template <>
inline unsigned int apsPointDomain<1>::GetId() const { return 828657780; }
template <>
inline unsigned int apsPointDomain<3>::GetId() const { return 862212212; }
template <>
inline unsigned int apsBumpDomain<1>::GetId() const { return 828654197; }
template <>
inline unsigned int apsBumpDomain<3>::GetId() const { return 862208629; }

template <int tDimension>
inline void apsBoxDomain<tDimension>::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions > tDimension &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsSuppliedDomains.h", 496,
                  "iNumDimensions <= tDimension",
                  "can't generate values for that many dimensions"))
        __debugbreak();
    for (int i = 0; i < iNumDimensions; ++i)
        oOutput[i] = apsMath::FloatRand(mMin[i], mMax[i]);
}
template <int tDimension>
inline unsigned int apsBoxDomain<tDimension>::TestValue(int iNumDimensions, float* iValue) const {
    int dim = (iNumDimensions > tDimension) ? tDimension : iNumDimensions;
    for (int i = 0; i < dim; ++i) {
        if (mMin[i] > iValue[i] || iValue[i] > mMax[i])
            return 0;
    }
    return 1;
}

template <int tDimension>
inline apsPointDomain<tDimension>::apsPointDomain(float* iVal) {
    for (int i = 0; i < tDimension; ++i)
        mVal[i] = iVal[i];
}
template <int tDimension>
inline void apsPointDomain<tDimension>::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions > tDimension &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsSuppliedDomains.h", 527,
                  "iNumDimensions <= tDimension",
                  "can't generate values for that many dimensions"))
        __debugbreak();
    for (int i = 0; i < iNumDimensions; ++i)
        oOutput[i] = mVal[i];
}
template <int tDimension>
inline unsigned int apsPointDomain<tDimension>::TestValue(int iNumDimensions, float* iValue) const {
    int dim = (iNumDimensions > tDimension) ? tDimension : iNumDimensions;
    for (int i = 0; i < dim; ++i) {
        if (iValue[i] != mVal[i])
            return 0;
    }
    return 1;
}

template <int tDimension>
inline apsBumpDomain<tDimension>::apsBumpDomain(float* iMinMax) {
    for (int i = 0; i < tDimension; ++i) {
        mMin[i] = iMinMax[i];
        mMax[i] = iMinMax[tDimension + i];
        if (mMin[i] > mMax[i]) { float t = mMin[i]; mMin[i] = mMax[i]; mMax[i] = t; }
    }
}
template <int tDimension>
inline void apsBumpDomain<tDimension>::GetValue(int iNumDimensions, float* oOutput) const {
    if (iNumDimensions > tDimension &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsSuppliedDomains.h", 569,
                  "iNumDimensions <= tDimension",
                  "can't generate values for that many dimensions"))
        __debugbreak();
    for (int i = 0; i < iNumDimensions; ++i) {
        float u = apsMath::gDefaultRandomNumberGenerator.GetFloat();
        float scale = (u >= 0.5f) ? ((u - 1.0f) * u * 2.0f + 1.0f) : ((1.0f - u) * u * 2.0f);
        oOutput[i] = (mMax[i] - mMin[i]) * scale + mMin[i];
    }
}
template <int tDimension>
inline unsigned int apsBumpDomain<tDimension>::TestValue(int iNumDimensions, float* iValue) const {
    int dim = (iNumDimensions > tDimension) ? tDimension : iNumDimensions;
    for (int i = 0; i < dim; ++i) {
        if (mMin[i] > iValue[i] || iValue[i] > mMax[i])
            return 0;
    }
    return 1;
}

// ============================================================================
// Create*Domain factories (placement-new so MSVC sets vptr)
// ============================================================================
apsBoxDomain<1>* CreateBoxDomain(float iMin, float iMax) {
    void* mem = apsCommon::GetAllocator()->MemAlign(12, 4);
    if (!mem) return 0;
    apsBoxDomain<1>* d = new (mem) apsBoxDomain<1>((float*)&iMin);
    return d;
}
apsBoxDomain<3>* CreateBoxDomain(const math::Dir3::Packed& iMin, const math::Dir3::Packed& iMax) {
    void* mem = apsCommon::GetAllocator()->MemAlign(28, 4);
    if (!mem) return 0;
    float buf[6]; buf[0]=iMin.x; buf[1]=iMin.y; buf[2]=iMin.z;
    buf[3]=iMax.x; buf[4]=iMax.y; buf[5]=iMax.z;
    apsBoxDomain<3>* d = new (mem) apsBoxDomain<3>(buf);
    return d;
}
apsPointDomain<1>* CreatePointDomain(float iVal) {
    void* mem = apsCommon::GetAllocator()->MemAlign(8, 4);
    if (!mem) return 0;
    apsPointDomain<1>* d = new (mem) apsPointDomain<1>(&iVal);
    return d;
}
apsPointDomain<3>* CreatePointDomain(const math::Dir3::Packed& iVal) {
    void* mem = apsCommon::GetAllocator()->MemAlign(16, 4);
    if (!mem) return 0;
    float buf[3] = { iVal.x, iVal.y, iVal.z };
    apsPointDomain<3>* d = new (mem) apsPointDomain<3>(buf);
    return d;
}
apsBumpDomain<1>* CreateBumpDomain(float iMin, float iMax) {
    void* mem = apsCommon::GetAllocator()->MemAlign(12, 4);
    if (!mem) return 0;
    apsBumpDomain<1>* d = new (mem) apsBumpDomain<1>((float*)&iMin);
    return d;
}
apsBumpDomain<3>* CreateBumpDomain(const math::Dir3::Packed& iMin, const math::Dir3::Packed& iMax) {
    void* mem = apsCommon::GetAllocator()->MemAlign(28, 4);
    if (!mem) return 0;
    float buf[6]; buf[0]=iMin.x; buf[1]=iMin.y; buf[2]=iMin.z;
    buf[3]=iMax.x; buf[4]=iMax.y; buf[5]=iMax.z;
    apsBumpDomain<3>* d = new (mem) apsBumpDomain<3>(buf);
    return d;
}

// ============================================================================
// GetNewSimpleMeshRenderer � 2 params: [0]=MESH, [1]=TEXTURE.
// ============================================================================
apsSimpleMeshRenderer* GetNewSimpleMeshRenderer(const apsArray<apsParam>& params) {
    if (params.mSize != 2 &&
        _tlAssert("source/apsRegister.cpp", 393, "2 == params.size()",
                  "invalid number of parameters"))
        __debugbreak();
    apsSimpleMeshRenderer::cArgs args;
    args.mMesh = 0;
    args.mTexture = 0;
    if ((int)params[0] != MESH &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 92,
                  "mType==MESH", "incorrect parameter!"))
        __debugbreak();
    args.mMesh = (nglMesh*)params[0];
    if ((int)params[1] != TEXTURE &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 91,
                  "mType==TEXTURE", "incorrect parameter!"))
        __debugbreak();
    args.mTexture = (nglTexture*)params[1];
    void* mem = apsCommon::GetAllocator()->MemAlign(24, 16);
    if (mem)
        return new (mem) apsSimpleMeshRenderer(&args);
    return 0;
}

// ============================================================================
// GetNewShrimpRenderer � 10 params: texture, blend, 7 ints, tint color.
// ============================================================================
apsShrimpRenderer* GetNewShrimpRenderer(const apsArray<apsParam>& params) {
    if (params.mSize != 10 &&
        _tlAssert("source/apsRegister.cpp", 406, "10 == params.size()",
                  "invalid number of parameters"))
        __debugbreak();
    apsShrimpRenderer::cArgs args;
    memset(&args, 0, 36);
    args.mTintColor.x = 1.0f; args.mTintColor.y = 1.0f;
    args.mTintColor.z = 1.0f; args.mTintColor.w = 1.0f;

    if ((int)params[0] != TEXTURE &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 91,
                  "mType==TEXTURE", "incorrect parameter!"))
        __debugbreak();
    args.mTexture = (nglTexture*)params[0];

    apsParam p = params[1];
    args.mBlendMode = GetEnum<apsEBlendMode, 5>(p);

    if ((int)params[2] != INT32 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                  "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    args.mNumFrames = (int)params[2];
    if ((int)params[3] != INT32 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                  "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    args.mNumRows = (int)params[3];
    if ((int)params[4] != INT32 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                  "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    args.mNumRotations = (int)params[4];
    if ((int)params[5] != INT32 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                  "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    args.mSpriteWidth = (int)params[5];
    if ((int)params[6] != INT32 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                  "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    args.mSpriteHeight = (int)params[6];
    if ((int)params[7] != INT32 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                  "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    args.mTextureWidth = (int)params[7];
    if ((int)params[8] != INT32 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                  "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    args.mTextureHeight = (int)params[8];

    math::Vector4::Packed tc = (math::Vector4::Packed)params[9];
    args.mTintColor.x = tc.x; args.mTintColor.y = tc.y;
    args.mTintColor.z = tc.z; args.mTintColor.w = tc.w;

    void* mem = apsCommon::GetAllocator()->MemAlign(64, 16);
    if (mem)
        return new (mem) apsShrimpRenderer(&args);
    return 0;
}

// ============================================================================
// GetNewBillboardRenderer<Class, Args> / GetNewUVARenderer<Class, Args>
// ============================================================================
template <typename T, typename TArgs>
T* GetNewBillboardRenderer(const apsArray<apsParam>& params) {
    TArgs args;
    args.mZFeatherDistance = 20.0f;
    memset(&args, 0, 20);
    memset(&args.mShaderWorksShader, 0, 12);
    args.mChanceToRemove = 1.0f;
    args.mTintColor.x = 1.0f; args.mTintColor.y = 1.0f;
    args.mTintColor.z = 1.0f; args.mTintColor.w = 1.0f;
    memset(&args.mAmbientCoeff, 0, 44);

    if ((int)params[0] != TEXTURE &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 91,
                  "mType==TEXTURE", "incorrect parameter!"))
        __debugbreak();
    args.mTexture = (nglTexture*)params[0];

    apsParam p = params[1];
    args.mBlendMode = GetEnum<apsEBlendMode, 5>(p);

    apsParam pn = params[2];
    if ((int)pn != VECTOR3D &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 99,
                  "mType==VECTOR3D", "incorrect parameter!"))
        __debugbreak();
    math::Dir3::Packed np = (math::Dir3::Packed)pn;
    args.mNormal.x = np.x; args.mNormal.y = np.y; args.mNormal.z = np.z;

    int n = params.mSize - 3;
    switch (n) {
    case 0: break;
    case 1: args.SetVelocityTracked(params[3]); break;
    case 2:
        args.SetAlphaFadeStart(params[3]);
        args.SetAlphaFadeEnd(params[4]);
        break;
    case 3:
        args.SetVelocityTracked(params[3]);
        args.SetAlphaFadeStart(params[4]);
        args.SetAlphaFadeEnd(params[5]);
        break;
    case 4:
        args.SetVelocityTracked(params[3]);
        args.SetAlphaFadeStart(params[4]);
        args.SetAlphaFadeEnd(params[5]);
        args.SetTintColor(params[6]);
        break;
    case 5:
        args.SetVelocityTracked(params[3]);
        args.SetAlphaFadeStart(params[4]);
        args.SetAlphaFadeEnd(params[5]);
        args.SetTintColor(params[6]);
        args.SetZFeatherDistance(params[7]);
        break;
    case 6:
        args.SetVelocityTracked(params[3]);
        args.SetAlphaFadeStart(params[4]);
        args.SetAlphaFadeEnd(params[5]);
        args.SetTintColor(params[6]);
        args.SetZFeatherDistance(params[7]);
        args.mShaderWorksShader = 0;
        break;
    case 7:
        args.SetVelocityTracked(params[3]);
        args.SetAlphaFadeStart(params[4]);
        args.SetAlphaFadeEnd(params[5]);
        args.SetTintColor(params[6]);
        args.SetZFeatherDistance(params[7]);
        args.mShaderWorksShader = 0;
        args.SetIsShimmer(params[9]);
        break;
    case 8:
        args.SetVelocityTracked(params[3]);
        args.SetAlphaFadeStart(params[4]);
        args.SetAlphaFadeEnd(params[5]);
        args.SetTintColor(params[6]);
        args.SetZFeatherDistance(params[7]);
        args.mShaderWorksShader = 0;
        args.SetIsShimmer(params[9]);
        args.SetUseSortedRendering(params[10]);
        break;
    case 10:
        args.SetVelocityTracked(params[3]);
        args.SetAlphaFadeStart(params[4]);
        args.SetAlphaFadeEnd(params[5]);
        args.SetTintColor(params[6]);
        args.SetZFeatherDistance(params[7]);
        args.mShaderWorksShader = 0;
        args.SetIsShimmer(params[9]);
        args.SetUseSortedRendering(params[10]);
        args.SetAmbientCoeff(params[11]);
        args.SetDiffuseCoeff(params[12]);
        break;
    case 11:
        args.SetVelocityTracked(params[3]);
        args.SetAlphaFadeStart(params[4]);
        args.SetAlphaFadeEnd(params[5]);
        args.SetTintColor(params[6]);
        args.SetZFeatherDistance(params[7]);
        args.mShaderWorksShader = 0;
        args.SetIsShimmer(params[9]);
        args.SetUseSortedRendering(params[10]);
        args.SetAmbientCoeff(params[11]);
        args.SetDiffuseCoeff(params[12]);
        args.SetChanceToRemove(params[13]);
        break;
    default:
        if (_tlAssert("source/apsRegister.cpp", 283, "0", "invalid number of parameters"))
            __debugbreak();
        break;
    }
    void* mem = apsCommon::GetAllocator()->MemAlign(sizeof(T), 16);
    if (mem)
        return new (mem) T(&args);
    return 0;
}

template <typename T, typename TArgs>
T* GetNewUVARenderer(const apsArray<apsParam>& params) {
    TArgs args;
    args.mZFeatherDistance = 20.0f;
    memset(&args, 0, 20);
    memset(&args.mShaderWorksShader, 0, 12);
    args.mChanceToRemove = 1.0f;
    args.mTintColor.x = 1.0f; args.mTintColor.y = 1.0f;
    args.mTintColor.z = 1.0f; args.mTintColor.w = 1.0f;
    memset(&args.mAmbientCoeff, 0, 44);
    args.mWidthFrames = 0;
    args.mHeightFrames = 0;

    if ((int)params[0] != TEXTURE &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 91,
                  "mType==TEXTURE", "incorrect parameter!"))
        __debugbreak();
    args.mTexture = (nglTexture*)params[0];

    apsParam p = params[1];
    args.mBlendMode = GetEnum<apsEBlendMode, 5>(p);

    if ((int)params[2] != INT32 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                  "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    args.mWidthFrames = (int)params[2];
    if ((int)params[3] != INT32 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                  "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    args.mHeightFrames = (int)params[3];

    apsParam pn = params[4];
    if ((int)pn != VECTOR3D &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 99,
                  "mType==VECTOR3D", "incorrect parameter!"))
        __debugbreak();
    math::Dir3::Packed np = (math::Dir3::Packed)pn;
    args.mNormal.x = np.x; args.mNormal.y = np.y; args.mNormal.z = np.z;

    int n = params.mSize - 5;
    switch (n) {
    case 0: break;
    case 1: args.SetVelocityTracked(params[5]); break;
    case 2:
        args.SetAlphaFadeStart(params[5]);
        args.SetAlphaFadeEnd(params[6]);
        break;
    case 3:
        args.SetVelocityTracked(params[5]);
        args.SetAlphaFadeStart(params[6]);
        args.SetAlphaFadeEnd(params[7]);
        break;
    case 4:
        args.SetVelocityTracked(params[5]);
        args.SetAlphaFadeStart(params[6]);
        args.SetAlphaFadeEnd(params[7]);
        args.SetTintColor(params[8]);
        break;
    case 5:
        args.SetVelocityTracked(params[5]);
        args.SetAlphaFadeStart(params[6]);
        args.SetAlphaFadeEnd(params[7]);
        args.SetTintColor(params[8]);
        args.SetZFeatherDistance(params[9]);
        break;
    case 6:
        args.SetVelocityTracked(params[5]);
        args.SetAlphaFadeStart(params[6]);
        args.SetAlphaFadeEnd(params[7]);
        args.SetTintColor(params[8]);
        args.SetZFeatherDistance(params[9]);
        args.mShaderWorksShader = 0;
        break;
    case 7:
        args.SetVelocityTracked(params[5]);
        args.SetAlphaFadeStart(params[6]);
        args.SetAlphaFadeEnd(params[7]);
        args.SetTintColor(params[8]);
        args.SetZFeatherDistance(params[9]);
        args.mShaderWorksShader = 0;
        args.SetIsShimmer(params[11]);
        break;
    case 8:
        args.SetVelocityTracked(params[5]);
        args.SetAlphaFadeStart(params[6]);
        args.SetAlphaFadeEnd(params[7]);
        args.SetTintColor(params[8]);
        args.SetZFeatherDistance(params[9]);
        args.mShaderWorksShader = 0;
        args.SetIsShimmer(params[11]);
        args.SetUseSortedRendering(params[12]);
        break;
    case 10:
        args.SetVelocityTracked(params[5]);
        args.SetAlphaFadeStart(params[6]);
        args.SetAlphaFadeEnd(params[7]);
        args.SetTintColor(params[8]);
        args.SetZFeatherDistance(params[9]);
        args.mShaderWorksShader = 0;
        args.SetIsShimmer(params[11]);
        args.SetUseSortedRendering(params[12]);
        args.SetAmbientCoeff(params[13]);
        args.SetDiffuseCoeff(params[14]);
        break;
    case 11:
        args.SetVelocityTracked(params[5]);
        args.SetAlphaFadeStart(params[6]);
        args.SetAlphaFadeEnd(params[7]);
        args.SetTintColor(params[8]);
        args.SetZFeatherDistance(params[9]);
        args.mShaderWorksShader = 0;
        args.SetIsShimmer(params[11]);
        args.SetUseSortedRendering(params[12]);
        args.SetAmbientCoeff(params[13]);
        args.SetDiffuseCoeff(params[14]);
        args.SetChanceToRemove(params[15]);
        break;
    default:
        if (_tlAssert("source/apsRegister.cpp", 383, "0", "invalid number of parameters"))
            __debugbreak();
        break;
    }
    void* mem = apsCommon::GetAllocator()->MemAlign(sizeof(T), 16);
    if (mem)
        return new (mem) T(&args);
    return 0;
}

// ============================================================================
// cArgs setters (inline COMDATs emitted in apsRegister.o)
// ============================================================================
void apsBillboardRenderer::cArgs::SetTexture(apsParam param) {
    if (param.mType != TEXTURE && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 91,
                                            "mType==TEXTURE", "incorrect parameter!"))
        __debugbreak();
    mTexture = param.mData.mTexture;
}
void apsBillboardRenderer::cArgs::SetNormal(apsParam param) {
    if (param.mType != VECTOR3D && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 99,
                                             "mType==VECTOR3D", "incorrect parameter!"))
        __debugbreak();
    mNormal.x = param.mData.mVector3d[0]; mNormal.y = param.mData.mVector3d[1]; mNormal.z = param.mData.mVector3d[2];
}
void apsBillboardRenderer::cArgs::SetBlendMode(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mBlendMode = (apsEBlendMode)param.mData.mI32;
}
void apsBillboardRenderer::cArgs::SetVelocityTracked(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mVelocityTracked = param.mData.mI32;
}
void apsBillboardRenderer::cArgs::SetAlphaFadeStart(apsParam param) {
    if (param.mType != FLOAT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 95,
                                            "mType==FLOAT32", "incorrect parameter!"))
        __debugbreak();
    mAlphaFadeStart = param.mData.mFloat;
}
void apsBillboardRenderer::cArgs::SetAlphaFadeEnd(apsParam param) {
    if (param.mType != FLOAT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 95,
                                            "mType==FLOAT32", "incorrect parameter!"))
        __debugbreak();
    mAlphaFadeEnd = param.mData.mFloat;
}
void apsBillboardRenderer::cArgs::SetTintColor(apsParam param) {
    if (param.mType != COLOR && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 106,
                                          "mType==COLOR", "incorrect parameter!"))
        __debugbreak();
    mTintColor.x = param.mData.mColor.r * 0.0039215689f;
    mTintColor.y = param.mData.mColor.g * 0.0039215689f;
    mTintColor.z = param.mData.mColor.b * 0.0039215689f;
    mTintColor.w = param.mData.mColor.a * 0.0039215689f;
}
void apsBillboardRenderer::cArgs::SetAmbientCoeff(apsParam param) {
    if (param.mType != COLOR && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 106,
                                          "mType==COLOR", "incorrect parameter!"))
        __debugbreak();
    mAmbientCoeff.x = param.mData.mColor.r * 0.0039215689f;
    mAmbientCoeff.y = param.mData.mColor.g * 0.0039215689f;
    mAmbientCoeff.z = param.mData.mColor.b * 0.0039215689f;
    mAmbientCoeff.w = param.mData.mColor.a * 0.0039215689f;
}
void apsBillboardRenderer::cArgs::SetDiffuseCoeff(apsParam param) {
    if (param.mType != COLOR && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 106,
                                          "mType==COLOR", "incorrect parameter!"))
        __debugbreak();
    mDiffuseCoeff.x = param.mData.mColor.r * 0.0039215689f;
    mDiffuseCoeff.y = param.mData.mColor.g * 0.0039215689f;
    mDiffuseCoeff.z = param.mData.mColor.b * 0.0039215689f;
    mDiffuseCoeff.w = param.mData.mColor.a * 0.0039215689f;
}
void apsBillboardRenderer::cArgs::SetZFeatherDistance(apsParam param) {
    if (param.mType != FLOAT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 95,
                                            "mType==FLOAT32", "incorrect parameter!"))
        __debugbreak();
    mZFeatherDistance = param.mData.mFloat;
}
void apsBillboardRenderer::cArgs::SetShaderWorksShader(apsParam param) {
    if (param.mType != 2 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                      "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mShaderWorksShader = param.mData.mShaderWorksShader;
}
void apsBillboardRenderer::cArgs::SetIsShimmer(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mIsShimmer = param.mData.mI32;
}
void apsBillboardRenderer::cArgs::SetUseSortedRendering(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mUseSortedRendering = param.mData.mI32;
}
void apsBillboardRenderer::cArgs::SetChanceToRemove(apsParam param) {
    if (param.mType != FLOAT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 95,
                                            "mType==FLOAT32", "incorrect parameter!"))
        __debugbreak();
    mChanceToRemove = param.mData.mFloat;
}
void apsUVARenderer::cArgs::SetWidthFrames(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mWidthFrames = param.mData.mI32;
}
void apsUVARenderer::cArgs::SetHeightFrames(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mHeightFrames = param.mData.mI32;
}
void apsSimpleMeshRenderer::cArgs::SetTexture(apsParam param) {
    if (param.mType != TEXTURE && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 91,
                                            "mType==TEXTURE", "incorrect parameter!"))
        __debugbreak();
    mTexture = param.mData.mTexture;
}
void apsSimpleMeshRenderer::cArgs::SetMesh(apsParam param) {
    if (param.mType != MESH && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 92,
                                         "mType==MESH", "incorrect parameter!"))
        __debugbreak();
    mMesh = param.mData.mMesh;
}
void apsShrimpRenderer::cArgs::SetTexture(apsParam param) {
    if (param.mType != TEXTURE && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 91,
                                            "mType==TEXTURE", "incorrect parameter!"))
        __debugbreak();
    mTexture = param.mData.mTexture;
}
void apsShrimpRenderer::cArgs::SetBlendMode(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mBlendMode = (apsEBlendMode)param.mData.mI32;
}
void apsShrimpRenderer::cArgs::SetNumFrames(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mNumFrames = param.mData.mI32;
}
void apsShrimpRenderer::cArgs::SetNumRows(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mNumRows = param.mData.mI32;
}
void apsShrimpRenderer::cArgs::SetNumRotations(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mNumRotations = param.mData.mI32;
}
void apsShrimpRenderer::cArgs::SetSpriteWidth(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mSpriteWidth = param.mData.mI32;
}
void apsShrimpRenderer::cArgs::SetSpriteHeight(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mSpriteHeight = param.mData.mI32;
}
void apsShrimpRenderer::cArgs::SetTextureWidth(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mTextureWidth = param.mData.mI32;
}
void apsShrimpRenderer::cArgs::SetTextureHeight(apsParam param) {
    if (param.mType != INT32 && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 94,
                                          "mType==INT32", "incorrect parameter!"))
        __debugbreak();
    mTextureHeight = param.mData.mI32;
}
void apsShrimpRenderer::cArgs::SetTintColor(apsParam param) {
    if (param.mType != COLOR && _tlAssert("c:/cod/code/tl/aeps/include\\apsRegister.h", 106,
                                          "mType==COLOR", "incorrect parameter!"))
        __debugbreak();
    mTintColor.x = param.mData.mColor.r * 0.0039215689f;
    mTintColor.y = param.mData.mColor.g * 0.0039215689f;
    mTintColor.z = param.mData.mColor.b * 0.0039215689f;
    mTintColor.w = param.mData.mColor.a * 0.0039215689f;
}

// ============================================================================
// sFixupParams statics
// ============================================================================
apsFixupParams apsRegisterStatics::sFixupParams;
bool apsRegisterStatics::sInit = false;

// FourCC ids
static const unsigned int ID_BILLBOARD            = 0x426C626F;
static const unsigned int ID_COLORBILLBOARD       = 0x43426C62;
static const unsigned int ID_RECTANGLE            = 0x52656374;
static const unsigned int ID_COLORRECTANGLE       = 0x43526374;
static const unsigned int ID_UVA                  = 0x55564152;
static const unsigned int ID_COLORUVA             = 0x43555641;
static const unsigned int ID_UVARECTANGLE         = 0x55566165;
static const unsigned int ID_COLORUVARECTANGLE    = 0x63555672;
static const unsigned int ID_SIMPLEMESH           = 0x53486568;
static const unsigned int ID_SHRIMP               = 0x53687070;
static const unsigned int ID_NULLRENDERER         = 0x4E56304C;

// ============================================================================
// apsGetRenderer
// ============================================================================
apsRenderer* apsGetRenderer(const char* name, const unsigned int* id,
                            const apsArray<apsParam>& params,
                            apsFixupParams* fixups) {
    unsigned int v4 = 0;
    if (name != 0) {
        if (id == 0) {
            v4 = 0;
        } else {
            v4 = *id;
        }
    } else if (id == 0) {
        if (fixups == 0 &&
            _tlAssert("source/apsRegister.cpp", 470,
                      "name != 0 || id != 0 || fixups != 0",
                      "no way!  one must be specified"))
            __debugbreak();
        v4 = 0;
    } else {
        v4 = *id;
    }

    if (fixups != 0)
        fixups->numRenderers = 0;

    if (v4 == 0) {
        if (fixups != 0) {
            fixups->renderers[fixups->numRenderers].id = ID_BILLBOARD;
            fixups->renderers[fixups->numRenderers].vtbl = apsBillboardRenderer::RetrieveVtable();
            ++fixups->numRenderers;
            fixups->renderers[fixups->numRenderers].id = ID_COLORBILLBOARD;
            fixups->renderers[fixups->numRenderers].vtbl = apsColorBillboardRenderer::RetrieveVtable();
            ++fixups->numRenderers;
            fixups->renderers[fixups->numRenderers].id = ID_RECTANGLE;
            fixups->renderers[fixups->numRenderers].vtbl = apsRectangleRenderer::RetrieveVtable();
            ++fixups->numRenderers;
            fixups->renderers[fixups->numRenderers].id = ID_COLORRECTANGLE;
            fixups->renderers[fixups->numRenderers].vtbl = apsColorRectangleRenderer::RetrieveVtable();
            ++fixups->numRenderers;
            fixups->renderers[fixups->numRenderers].id = ID_UVA;
            fixups->renderers[fixups->numRenderers].vtbl = apsUVARenderer::RetrieveVtable();
            ++fixups->numRenderers;
            fixups->renderers[fixups->numRenderers].id = ID_COLORUVA;
            fixups->renderers[fixups->numRenderers].vtbl = apsColorUVARenderer::RetrieveVtable();
            ++fixups->numRenderers;
            fixups->renderers[fixups->numRenderers].id = ID_UVARECTANGLE;
            fixups->renderers[fixups->numRenderers].vtbl = apsUVARectangleRenderer::RetrieveVtable();
            ++fixups->numRenderers;
            fixups->renderers[fixups->numRenderers].id = ID_COLORUVARECTANGLE;
            fixups->renderers[fixups->numRenderers].vtbl = apsColorUVARectangleRenderer::RetrieveVtable();
            ++fixups->numRenderers;
            fixups->renderers[fixups->numRenderers].id = ID_SIMPLEMESH;
            fixups->renderers[fixups->numRenderers].vtbl = apsSimpleMeshRenderer::RetrieveVtable();
            ++fixups->numRenderers;
            fixups->renderers[fixups->numRenderers].id = ID_SHRIMP;
            fixups->renderers[fixups->numRenderers].vtbl = apsShrimpRenderer::RetrieveVtable();
            ++fixups->numRenderers;
            return 0;
        }
        if (name != 0) {
            if (strcmp("BillboardRenderer", name) == 0)
                return GetNewBillboardRenderer<apsBillboardRenderer, apsBillboardRenderer::cArgs>(params);
            if (strcmp("ColorBillboardRenderer", name) == 0)
                return GetNewBillboardRenderer<apsColorBillboardRenderer, apsColorBillboardRenderer::cArgs>(params);
            if (strcmp("RectangleRenderer", name) == 0)
                return GetNewBillboardRenderer<apsRectangleRenderer, apsRectangleRenderer::cArgs>(params);
            if (strcmp("ColorRectangleRenderer", name) == 0)
                return GetNewBillboardRenderer<apsColorRectangleRenderer, apsColorRectangleRenderer::cArgs>(params);
            if (strcmp("UVARenderer", name) == 0)
                return GetNewUVARenderer<apsUVARenderer, apsUVARenderer::cArgs>(params);
            if (strcmp("ColorUVARenderer", name) == 0)
                return GetNewUVARenderer<apsColorUVARenderer, apsColorUVARenderer::cArgs>(params);
            if (strcmp("UVARectangleRenderer", name) == 0)
                return GetNewUVARenderer<apsUVARectangleRenderer, apsUVARectangleRenderer::cArgs>(params);
            if (strcmp("ColorUVARectangleRenderer", name) == 0)
                return GetNewUVARenderer<apsColorUVARectangleRenderer, apsColorUVARectangleRenderer::cArgs>(params);
            if (strcmp("SimpleMeshRenderer", name) == 0)
                return GetNewSimpleMeshRenderer(params);
            if (strcmp("ShrimpRenderer", name) == 0)
                return GetNewShrimpRenderer(params);
            if (strcmp("_NullRenderer", name) == 0)
                return 0;
        }
        if (_tlAssert("source/apsRegister.cpp", 496, "0", "Couldn't find apsRenderer"))
            __debugbreak();
        return 0;
    }

    switch (v4) {
    case ID_BILLBOARD:
        return GetNewBillboardRenderer<apsBillboardRenderer, apsBillboardRenderer::cArgs>(params);
    case ID_COLORBILLBOARD:
        return GetNewBillboardRenderer<apsColorBillboardRenderer, apsColorBillboardRenderer::cArgs>(params);
    case ID_RECTANGLE:
        return GetNewBillboardRenderer<apsRectangleRenderer, apsRectangleRenderer::cArgs>(params);
    case ID_COLORRECTANGLE:
        return GetNewBillboardRenderer<apsColorRectangleRenderer, apsColorRectangleRenderer::cArgs>(params);
    case ID_UVA:
        return GetNewUVARenderer<apsUVARenderer, apsUVARenderer::cArgs>(params);
    case ID_COLORUVA:
        return GetNewUVARenderer<apsColorUVARenderer, apsColorUVARenderer::cArgs>(params);
    case ID_UVARECTANGLE:
        return GetNewUVARenderer<apsUVARectangleRenderer, apsUVARectangleRenderer::cArgs>(params);
    case ID_COLORUVARECTANGLE:
        return GetNewUVARenderer<apsColorUVARectangleRenderer, apsColorUVARectangleRenderer::cArgs>(params);
    case ID_SIMPLEMESH:
        return GetNewSimpleMeshRenderer(params);
    case ID_SHRIMP:
        return GetNewShrimpRenderer(params);
    case ID_NULLRENDERER:
        return 0;
    }
    if (fixups == 0) {
        if (_tlAssert("source/apsRegister.cpp", 496, "0", "Couldn't find apsRenderer"))
            __debugbreak();
    }
    return 0;
}

// ============================================================================
// apsGetDomain
// ============================================================================
apsDomain* apsGetDomain(const char* name, const unsigned int* id,
                        const apsArray<apsParam>& params, apsFixupParams* fixups) {
    unsigned int v5 = 0;
    if (name != 0) {
        if (id == 0) {
            v5 = 0;
        } else {
            v5 = *id;
        }
    } else if (id == 0) {
        if (fixups == 0 &&
            _tlAssert("source/apsRegister.cpp", 505,
                      "name != 0 || id != 0 || fixups != 0",
                      "no way!  one must be specified"))
            __debugbreak();
        v5 = 0;
    } else {
        v5 = *id;
    }

    if (fixups != 0) {
        fixups->numDomains = 0;
        fixups->domains[fixups->numDomains].id = 828654200;      // apsBoxDomain<1>
        fixups->domains[fixups->numDomains].vtbl = apsBoxDomain<1>::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 862208632;      // apsBoxDomain<3>
        fixups->domains[fixups->numDomains].vtbl = apsBoxDomain<3>::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 828657780;      // apsPointDomain<1>
        fixups->domains[fixups->numDomains].vtbl = apsPointDomain<1>::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 862212212;      // apsPointDomain<3>
        fixups->domains[fixups->numDomains].vtbl = apsPointDomain<3>::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 1147761507;     // apsDiscDomain
        fixups->domains[fixups->numDomains].vtbl = apsDiscDomain::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 1131570028;     // apsCircleDomain
        fixups->domains[fixups->numDomains].vtbl = apsCircleDomain::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 1399875685;     // apsSphereDomain
        fixups->domains[fixups->numDomains].vtbl = apsSphereDomain::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 1497912176;     // apsYHemisphereDomain
        fixups->domains[fixups->numDomains].vtbl = apsYHemisphereDomain::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 1514689392;     // apsZHemisphereDomain
        fixups->domains[fixups->numDomains].vtbl = apsZHemisphereDomain::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 1399870325;     // apsSphereSurfaceDomain
        fixups->domains[fixups->numDomains].vtbl = apsSphereSurfaceDomain::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 1281977957;     // apsLineDomain
        fixups->domains[fixups->numDomains].vtbl = apsLineDomain::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 828654197;      // apsBumpDomain<1>
        fixups->domains[fixups->numDomains].vtbl = apsBumpDomain<1>::RetrieveVtable();
        ++fixups->numDomains;
        fixups->domains[fixups->numDomains].id = 862208629;      // apsBumpDomain<3>
        fixups->domains[fixups->numDomains].vtbl = apsBumpDomain<3>::RetrieveVtable();
        ++fixups->numDomains;
        return 0;
    }

    if (name != 0 && v5 == 0) {
        if (strcmp("_NullDomain", name) == 0)
            return 0;
        if (strcmp("1dBoxDomain", name) == 0) goto L_BOX1;
        if (strcmp("3dBoxDomain", name) == 0) goto L_BOX3;
        if (strcmp("RgbBoxDomain", name) == 0) goto L_BOX3;
        if (strcmp("UVAFrameRange", name) == 0) goto L_BOX1;
        if (strcmp("1dPointDomain", name) == 0) goto L_POINT1;
        if (strcmp("3dPointDomain", name) == 0) goto L_POINT3;
        if (strcmp("UVAFrame", name) == 0) goto L_POINT1;
        if (strcmp("DiscDomain", name) == 0) goto L_DISC;
        if (strcmp("CircleDomain", name) == 0) goto L_CIRCLE;
        if (strcmp("SphereDomain", name) == 0) goto L_SPHERE;
        if (strcmp("YHemisphereDomain", name) == 0) goto L_YHEMI;
        if (strcmp("ZHemisphereDomain", name) == 0) goto L_ZHEMI;
        if (strcmp("SphereSurfaceDomain", name) == 0) goto L_SSURF;
        if (strcmp("3dLineDomain", name) == 0) goto L_LINE;
        if (strcmp("1dBumpDomain", name) == 0) goto L_BUMP1;
        if (strcmp("3dBumpDomain", name) == 0) goto L_BUMP3;
        goto L_NOTFOUND;
    }

    switch (v5) {
    case 1281977957: goto L_LINE;
    case 1147761507: goto L_DISC;
    case 1131570028: goto L_CIRCLE;
    case 862208632:  goto L_BOX3;
    case 862212212:  goto L_POINT3;
    case 862208629:  goto L_BUMP3;
    case 828657780:  goto L_POINT1;
    case 828654200:  goto L_BOX1;
    case 828654197:  goto L_BUMP1;
    case 1399875685: goto L_SPHERE;
    case 1399870325: goto L_SSURF;
    case 1497912176: goto L_YHEMI;
    case 1514689392: goto L_ZHEMI;
    default: goto L_NOTFOUND;
    }

L_BOX1:
    if (params.mSize != 2 && _tlAssert("source/apsRegister.cpp", 535, "params.size() == 2",
                                       "incorrect number of parameters"))
        __debugbreak();
    return CreateBoxDomain((float)params[0], (float)params[1]);
L_BOX3:
    if (params.mSize != 2 && _tlAssert("source/apsRegister.cpp", 536, "params.size() == 2",
                                       "incorrect number of parameters"))
        __debugbreak();
    return CreateBoxDomain((math::Dir3::Packed)params[0], (math::Dir3::Packed)params[1]);
L_POINT1:
    if (params.mSize != 1 && _tlAssert("source/apsRegister.cpp", 541, "params.size() == 1",
                                       "incorrect number of parameters"))
        __debugbreak();
    return CreatePointDomain((float)params[0]);
L_POINT3:
    if (params.mSize != 1 && _tlAssert("source/apsRegister.cpp", 542, "params.size() == 1",
                                       "incorrect number of parameters"))
        __debugbreak();
    return CreatePointDomain((math::Dir3::Packed)params[0]);
L_BUMP1:
    if (params.mSize != 2 && _tlAssert("source/apsRegister.cpp", 554, "params.size() == 2",
                                       "incorrect number of parameters"))
        __debugbreak();
    return CreateBumpDomain((float)params[0], (float)params[1]);
L_BUMP3:
    if (params.mSize != 2 && _tlAssert("source/apsRegister.cpp", 555, "params.size() == 2",
                                       "incorrect number of parameters"))
        __debugbreak();
    return CreateBumpDomain((math::Dir3::Packed)params[0], (math::Dir3::Packed)params[1]);
L_DISC:
    if (params.mSize != 3 && _tlAssert("source/apsRegister.cpp", 546, "params.size() == 3",
                                       "incorrect number of parameters"))
        __debugbreak();
    {
        void* mem = apsCommon::GetAllocator()->MemAlign(64, 16);
        if (!mem) return 0;
        return new (mem) apsDiscDomain((math::Dir3)params[0], (math::Dir3)params[1], (float)params[2]);
    }
L_CIRCLE:
    if (params.mSize != 3 && _tlAssert("source/apsRegister.cpp", 547, "params.size() == 3",
                                       "incorrect number of parameters"))
        __debugbreak();
    {
        void* mem = apsCommon::GetAllocator()->MemAlign(64, 16);
        if (!mem) return 0;
        return new (mem) apsCircleDomain((math::Dir3)params[0], (math::Dir3)params[1], (float)params[2]);
    }
L_SPHERE:
    if (params.mSize != 2 && _tlAssert("source/apsRegister.cpp", 548, "params.size() == 2",
                                       "incorrect number of parameters"))
        __debugbreak();
    {
        void* mem = apsCommon::GetAllocator()->MemAlign(32, 16);
        if (!mem) return 0;
        return new (mem) apsSphereDomain((math::Dir3)params[0], (float)params[1]);
    }
L_YHEMI:
    if (params.mSize != 2 && _tlAssert("source/apsRegister.cpp", 549, "params.size() == 2",
                                       "incorrect number of parameters"))
        __debugbreak();
    {
        void* mem = apsCommon::GetAllocator()->MemAlign(32, 16);
        if (!mem) return 0;
        return new (mem) apsYHemisphereDomain((math::Dir3)params[0], (float)params[1]);
    }
L_ZHEMI:
    if (params.mSize != 2 && _tlAssert("source/apsRegister.cpp", 550, "params.size() == 2",
                                       "incorrect number of parameters"))
        __debugbreak();
    {
        void* mem = apsCommon::GetAllocator()->MemAlign(32, 16);
        if (!mem) return 0;
        return new (mem) apsZHemisphereDomain((math::Dir3)params[0], (float)params[1]);
    }
L_SSURF:
    if (params.mSize != 2 && _tlAssert("source/apsRegister.cpp", 551, "params.size() == 2",
                                       "incorrect number of parameters"))
        __debugbreak();
    {
        void* mem = apsCommon::GetAllocator()->MemAlign(32, 16);
        if (!mem) return 0;
        return new (mem) apsSphereSurfaceDomain((math::Dir3)params[0], (float)params[1]);
    }
L_LINE:
    if (params.mSize != 2 && _tlAssert("source/apsRegister.cpp", 552, "params.size() == 2",
                                       "incorrect number of parameters"))
        __debugbreak();
    {
        void* mem = apsCommon::GetAllocator()->MemAlign(48, 16);
        if (!mem) return 0;
        return new (mem) apsLineDomain((math::Dir3)params[0], (math::Dir3)params[1]);
    }
L_NOTFOUND:
    if (_tlAssert("source/apsRegister.cpp", 558, "0", "Couldn't find apsDomain"))
        __debugbreak();
    return 0;
}

// ============================================================================
// apsGetAction
// ============================================================================
// Helper: allocate + default-construct an action.
template <typename T>
static T* NewAction() {
    void* mem = apsCommon::GetAllocator()->MemAlign(28, 4);
    if (mem)
        return new (mem) T();
    return 0;
}

apsAction* apsGetAction(const char* name, const unsigned int* id, apsFixupParams* fixups) {
    unsigned int v3 = 0;
    if (name != 0) {
        if (id == 0) {
            v3 = 0;
        } else {
            v3 = *id;
        }
    } else if (id == 0) {
        if (fixups == 0 &&
            _tlAssert("source/apsRegister.cpp", 564,
                      "name != 0 || id != 0 || fixups != 0",
                      "no way!  one must be specified"))
            __debugbreak();
        v3 = 0;
    } else {
        v3 = *id;
    }

    if (fixups != 0)
        fixups->numActions = 0;

    if (v3 == 0) {
        if (fixups != 0) {
            #define ADD_ACTION(Class, idval) fixups->actions[fixups->numActions].id = idval; fixups->actions[fixups->numActions].vtbl = Class::RetrieveVtable(); ++fixups->numActions;
            ADD_ACTION(apsSourceAction, 1400005441)
            ADD_ACTION(apsBurstAction, 1114796916)
            ADD_ACTION(apsLifetimeAction, 1281975909)
            ADD_ACTION(apsAlphaFadeAction, 1097614948)
            ADD_ACTION(apsAlphaFadeInOutAction, 1095125327)
            ADD_ACTION(apsRandomAlphaFadeInOutAction, 1380337999)
            ADD_ACTION(apsLinearScaleAction, 1282298723)
            ADD_ACTION(apsLinearScaleSyncAction, 1282626413)
            ADD_ACTION(apsExponentialScaleAction, 1165513571)
            ADD_ACTION(apsLinearScaleWidthAction, 1280533335)
            ADD_ACTION(apsExponentialScaleWidthAction, 1163092823)
            ADD_ACTION(apsLinearScaleHeightAction, 1280533320)
            ADD_ACTION(apsExponentialScaleHeightAction, 1163092808)
            ADD_ACTION(apsPositionMoveAction, 1349733750)
            ADD_ACTION(apsObjectMoveAction, 1330474870)
            ADD_ACTION(apsMoveAction, 1299150437)
            ADD_ACTION(apsMoveAtFixedVelocityAction, 1296463958)
            ADD_ACTION(apsForceAction, 1181708899)
            ADD_ACTION(apsColorShiftAction, 1131172712)
            ADD_ACTION(apsVelocityDragAction, 1449935986)
            ADD_ACTION(apsAngularVelocityDragAction, 1096172658)
            ADD_ACTION(apsVectorAngularVelocityDragAction, 1447122500)
            ADD_ACTION(apsWorldPlaneReflectionAction, 1464881766)
            ADD_ACTION(apsUVAFrameAnimAction, 1431717490)
            ADD_ACTION(apsAngleTrackVelocityAction, 1097754452)
            ADD_ACTION(apsAngleTrackElementXAction, 1096041816)
            ADD_ACTION(apsAngleTrackElementYAction, 1096041817)
            ADD_ACTION(apsAngleTrackElementZAction, 1096041818)
            ADD_ACTION(apsPointAttractorAction, 1346466930)
            ADD_ACTION(apsLineAttractorAction, 1279358066)
            ADD_ACTION(apsDecayLineAttractorAction, 1145848180)
            ADD_ACTION(apsKappaTauAction, 1263821173)
            ADD_ACTION(apsSpawnAction, 1399865699)
            ADD_ACTION(apsSpawnOnDeathAction, 1399866469)
            ADD_ACTION(apsTrajectoryAction, 1416782186)
            ADD_ACTION(apsRandomSpawnAction, 1381199982)
            ADD_ACTION(apsEnvCollideAction, 1165378412)
            #undef ADD_ACTION
            return 0;
        }
        if (name != 0) {
            if (strcmp("SourceAction", name) == 0) return NewAction<apsSourceAction>();
            if (strcmp("BurstAction", name) == 0) return NewAction<apsBurstAction>();
            if (strcmp("LifetimeAction", name) == 0) return NewAction<apsLifetimeAction>();
            if (strcmp("AlphaFadeAction", name) == 0) return NewAction<apsAlphaFadeAction>();
            if (strcmp("AlphaFadeInOutAction", name) == 0) return NewAction<apsAlphaFadeInOutAction>();
            if (strcmp("RandomAlphaFadeInOutAction", name) == 0) return NewAction<apsRandomAlphaFadeInOutAction>();
            if (strcmp("LinearScaleAction", name) == 0) return NewAction<apsLinearScaleAction>();
            if (strcmp("LinearScaleSyncAction", name) == 0) return NewAction<apsLinearScaleSyncAction>();
            if (strcmp("ExponentialScaleAction", name) == 0) return NewAction<apsExponentialScaleAction>();
            if (strcmp("LinearScaleWidthAction", name) == 0) return NewAction<apsLinearScaleWidthAction>();
            if (strcmp("ExponentialScaleWidthAction", name) == 0) return NewAction<apsExponentialScaleWidthAction>();
            if (strcmp("LinearScaleHeightAction", name) == 0) return NewAction<apsLinearScaleHeightAction>();
            if (strcmp("ExponentialScaleHeightAction", name) == 0) return NewAction<apsExponentialScaleHeightAction>();
            if (strcmp("PositionMoveAction", name) == 0) return NewAction<apsPositionMoveAction>();
            if (strcmp("ObjectMoveAction", name) == 0) return NewAction<apsObjectMoveAction>();
            if (strcmp("MoveAction", name) == 0) return NewAction<apsMoveAction>();
            if (strcmp("MoveAtFixedVelocityAction", name) == 0) return NewAction<apsMoveAtFixedVelocityAction>();
            if (strcmp("ForceAction", name) == 0) return NewAction<apsForceAction>();
            if (strcmp("ColorShiftAction", name) == 0) return NewAction<apsColorShiftAction>();
            if (strcmp("VelocityDragAction", name) == 0) return NewAction<apsVelocityDragAction>();
            if (strcmp("AngularVelocityDragAction", name) == 0) return NewAction<apsAngularVelocityDragAction>();
            if (strcmp("VectorAngularVelocityDragAction", name) == 0) return NewAction<apsVectorAngularVelocityDragAction>();
            if (strcmp("WorldPlaneReflectionAction", name) == 0) return NewAction<apsWorldPlaneReflectionAction>();
            if (strcmp("UVAFrameAnimAction", name) == 0) return NewAction<apsUVAFrameAnimAction>();
            if (strcmp("AngleTrackVelocityAction", name) == 0) return NewAction<apsAngleTrackVelocityAction>();
            if (strcmp("AngleTrackElementXAction", name) == 0) return NewAction<apsAngleTrackElementXAction>();
            if (strcmp("AngleTrackElementYAction", name) == 0) return NewAction<apsAngleTrackElementYAction>();
            if (strcmp("AngleTrackElementZAction", name) == 0) return NewAction<apsAngleTrackElementZAction>();
            if (strcmp("PointAttractorAction", name) == 0) return NewAction<apsPointAttractorAction>();
            if (strcmp("LineAttractorAction", name) == 0) return NewAction<apsLineAttractorAction>();
            if (strcmp("DecayLineAttractorAction", name) == 0) return NewAction<apsDecayLineAttractorAction>();
            if (strcmp("KappaTauAction", name) == 0) return NewAction<apsKappaTauAction>();
            if (strcmp("SpawnAction", name) == 0) return NewAction<apsSpawnAction>();
            if (strcmp("SpawnOnDeathAction", name) == 0) return NewAction<apsSpawnOnDeathAction>();
            if (strcmp("TrajectoryAction", name) == 0) return NewAction<apsTrajectoryAction>();
            if (strcmp("RandomSpawnAction", name) == 0) return NewAction<apsRandomSpawnAction>();
            if (strcmp("EnvCollideAction", name) == 0) return NewAction<apsEnvCollideAction>();
        }
        if (_tlAssert("source/apsRegister.cpp", 622, "0", "Couldn't find apsAction"))
            __debugbreak();
        return 0;
    }

    #define CASE_ACTION(Class, idval) case idval: return NewAction<Class>();
    switch (v3) {
        CASE_ACTION(apsSourceAction, 1400005441)
        CASE_ACTION(apsBurstAction, 1114796916)
        CASE_ACTION(apsLifetimeAction, 1281975909)
        CASE_ACTION(apsAlphaFadeAction, 1097614948)
        CASE_ACTION(apsAlphaFadeInOutAction, 1095125327)
        CASE_ACTION(apsRandomAlphaFadeInOutAction, 1380337999)
        CASE_ACTION(apsLinearScaleAction, 1282298723)
        CASE_ACTION(apsLinearScaleSyncAction, 1282626413)
        CASE_ACTION(apsExponentialScaleAction, 1165513571)
        CASE_ACTION(apsLinearScaleWidthAction, 1280533335)
        CASE_ACTION(apsExponentialScaleWidthAction, 1163092823)
        CASE_ACTION(apsLinearScaleHeightAction, 1280533320)
        CASE_ACTION(apsExponentialScaleHeightAction, 1163092808)
        CASE_ACTION(apsPositionMoveAction, 1349733750)
        CASE_ACTION(apsObjectMoveAction, 1330474870)
        CASE_ACTION(apsMoveAction, 1299150437)
        CASE_ACTION(apsMoveAtFixedVelocityAction, 1296463958)
        CASE_ACTION(apsForceAction, 1181708899)
        CASE_ACTION(apsColorShiftAction, 1131172712)
        CASE_ACTION(apsVelocityDragAction, 1449935986)
        CASE_ACTION(apsAngularVelocityDragAction, 1096172658)
        CASE_ACTION(apsVectorAngularVelocityDragAction, 1447122500)
        CASE_ACTION(apsWorldPlaneReflectionAction, 1464881766)
        CASE_ACTION(apsUVAFrameAnimAction, 1431717490)
        CASE_ACTION(apsAngleTrackVelocityAction, 1097754452)
        CASE_ACTION(apsAngleTrackElementXAction, 1096041816)
        CASE_ACTION(apsAngleTrackElementYAction, 1096041817)
        CASE_ACTION(apsAngleTrackElementZAction, 1096041818)
        CASE_ACTION(apsPointAttractorAction, 1346466930)
        CASE_ACTION(apsLineAttractorAction, 1279358066)
        CASE_ACTION(apsDecayLineAttractorAction, 1145848180)
        CASE_ACTION(apsKappaTauAction, 1263821173)
        CASE_ACTION(apsSpawnAction, 1399865699)
        CASE_ACTION(apsSpawnOnDeathAction, 1399866469)
        CASE_ACTION(apsTrajectoryAction, 1416782186)
        CASE_ACTION(apsRandomSpawnAction, 1381199982)
        CASE_ACTION(apsEnvCollideAction, 1165378412)
    default:
        break;
    }
    #undef CASE_ACTION
    if (fixups == 0) {
        if (_tlAssert("source/apsRegister.cpp", 622, "0", "Couldn't find apsAction"))
            __debugbreak();
    }
    return 0;
}

// ============================================================================
// apsLoadEffectInplace � load an effect template from an APK image section.
// ============================================================================
apsEffectTemplate* apsLoadEffectInplace(apk::apkFile* File, apk::apkFileEntry* Entry) {
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    apsEffectTemplate* Data = (apsEffectTemplate*)Entry->GetData(File, SectionIndex, true);

    if (!apsRegisterStatics::sInit) {
        memset(&apsRegisterStatics::sFixupParams, 0, sizeof(apsRegisterStatics::sFixupParams));
        apsRegisterStatics::sInit = true;
        apsRegisterStatics::sFixupParams.basePtr = Data;
        apsArray<apsParam> emptyParams;
        apsGetAction(0, 0, &apsRegisterStatics::sFixupParams);
        apsGetDomain(0, 0, emptyParams, &apsRegisterStatics::sFixupParams);
        apsGetRenderer(0, 0, emptyParams, &apsRegisterStatics::sFixupParams);
    }
    Data->Fixup(apsRegisterStatics::sFixupParams);
    return Data;
}
