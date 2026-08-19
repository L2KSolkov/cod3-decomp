// ============================================================================
// ngl_dx_shader.cpp - vertex shader constants + shader registration (9 funcs).
// Source: src/dx/ngl_dx_shader.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_dx_shader.o).
// ============================================================================

#include "ngl/ngl_dx_shader.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_fsaa.h"
#include "d3d8.h"

#include <intrin.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern nglScene* nglBuildScene;                       // ngl_scene.o
extern void nglGetDirLightMatrix(nglMeshNode* MeshNode, math::Mat44* Dir,
                                 math::Mat44* Color);  // ngl_lighting.o
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data (ngl_dx_shader.o)
// ============================================================================
static __m128 BonesArray_1[3 * 48 + 1];   // 0x10E4B40
static __m128 BonesArray_2[3 * 64 + 1];   // 0x10E5450
static __m128 BonesArray_3[3 * 48 + 1];   // 0x10E6060

// ============================================================================
// nglDxRegisterVShader / nglDxRegisterPShader
// ============================================================================
void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode) {
    *VS = (unsigned long)Microcode;
}

void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode) {
    *PS = (unsigned long*)Microcode;
}

// ============================================================================
// Row-vector matrix multiply helpers (reconstructed from the IDA SSE chains).
// ============================================================================
static __m128 RowMul(const __m128 r, const math::Mat43* m) {
    return _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(r, r, 0), m->x.v),
                   _mm_mul_ps(_mm_shuffle_ps(r, r, 85), m->y.v)),
        _mm_mul_ps(_mm_shuffle_ps(r, r, 170), m->z.v));
}

static math::Mat43 MulMat43(const math::Mat43& A, const math::Mat43& B) {
    math::Mat43 r;
    r.x.v = RowMul(A.x.v, &B);
    r.y.v = RowMul(A.y.v, &B);
    r.z.v = RowMul(A.z.v, &B);
    r.w.v = _mm_add_ps(RowMul(A.w.v, &B), A.w.v);
    return r;
}

static math::Mat43 MulMat43World(const math::Mat43& M, const math::Mat43& W) {
    math::Mat43 r;
    r.x.v = RowMul(M.x.v, &W);
    r.y.v = RowMul(M.y.v, &W);
    r.z.v = RowMul(M.z.v, &W);
    r.w.v = _mm_add_ps(RowMul(M.w.v, &W), W.w.v);
    return r;
}

// Transpose a Mat43 into 3 constant rows (3x3 transposed + w handled by caller).
static void Mat43ToConstants(const math::Mat43& M, __m128* out) {
    __m128 a = _mm_shuffle_ps(M.x.v, M.y.v, 0x44);
    __m128 b = _mm_shuffle_ps(M.z.v, M.w.v, 0x44);
    out[0] = _mm_shuffle_ps(a, b, 0x88);
    out[1] = _mm_shuffle_ps(a, b, 0xDD);
    out[2] = _mm_shuffle_ps(_mm_shuffle_ps(M.x.v, M.y.v, 0xEE),
                            _mm_shuffle_ps(M.z.v, M.w.v, 0xEE), 0x88);
}

static const __m128 kIdentityX = { 1.0f, 0.0f, 0.0f, 0.0f };
static const __m128 kIdentityY = { 0.0f, 1.0f, 0.0f, 0.0f };
static const __m128 kIdentityZ = { 0.0f, 0.0f, 1.0f, 0.0f };

// ============================================================================
// nglDxSetBonesWorld - ea: 0x84DF20
// ============================================================================
void nglDxSetBonesWorld(int p, nglMeshNode* MeshNode, nglMeshSection* Section) {
    int NBones = Section->NBones;
    nglMeshParams* MeshParams = MeshNode->MeshParams;
    math::Mat43* MeshBones = MeshParams->Bones;
    if (NBones > 48
        && _tlAssert("src/dx/ngl_dx_shader.cpp", 215, "NBones <= MAX_BONES",
                     "Too many bones in mesh section for vertex shader constants!"))
        __debugbreak();
    __m128* dst = BonesArray_1;
    if (NBones != 0) {
        unsigned int Flags = MeshParams->Flags;
        nglSkeleton* Skeleton = (nglSkeleton*)MeshNode->Mesh->Skeleton;
        if ((Flags & 4) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                const math::Mat43* Rel = &MeshBones[Section->BoneIndices[i]];
                const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
                math::Mat43 M = MulMat43World(Bone->InvRest, *Rel);
                Mat43ToConstants(M, dst);
            }
        } else if ((Flags & 8) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                const math::Mat43* Rel = &MeshBones[Section->BoneIndices[i]];
                const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
                math::Mat43 M = MulMat43World(MulMat43World(Bone->InvRest, *Rel),
                                              MeshNode->LocalToWorld);
                Mat43ToConstants(M, dst);
            }
        } else if ((Flags & 0x10) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                math::Mat43 M = MulMat43World(MeshBones[Section->BoneIndices[i]],
                                              MeshNode->LocalToWorld);
                Mat43ToConstants(M, dst);
            }
        } else {
            for (int i = 0; i < NBones; ++i, dst += 3)
                Mat43ToConstants(MeshNode->LocalToWorld, dst);
        }
    } else {
        Mat43ToConstants(MeshNode->LocalToWorld, dst);
        Mat43ToConstants(MeshNode->LocalToWorld, dst + 3);
        NBones = 2;
    }
    int v71 = 3 * NBones;
    int v72 = p + 96;
    if (v71 == 1)
        D3DDevice_SetVertexShaderConstant1Fast(v72, BonesArray_1);
    else
        D3DDevice_SetVertexShaderConstantNotInlineFast(v72, BonesArray_1, 4 * v71);
}

// ============================================================================
// nglMeshNode::GetWToLNoScale (inline COMDAT, cdDynamicDecalShader.o).
// ============================================================================
static math::Mat43* MeshNode_GetWToLNoScale(const nglMeshNode* This, math::Mat43* result) {
    math::Mat43 LToWNoScale;
    if ((This->MeshParams->Flags & 2) != 0) {
        __m128 Scale = This->MeshParams->Scale.v;
        __m128 rcp = _mm_rcp_ps(Scale);
        __m128 inv = _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(rcp, Scale)), rcp);
        LToWNoScale.x.v = _mm_mul_ps(This->LocalToWorld.x.v, _mm_shuffle_ps(inv, inv, 0));
        LToWNoScale.y.v = _mm_mul_ps(This->LocalToWorld.y.v, _mm_shuffle_ps(inv, inv, 85));
        LToWNoScale.z.v = _mm_mul_ps(This->LocalToWorld.z.v, _mm_shuffle_ps(inv, inv, 170));
        LToWNoScale.w.v = This->LocalToWorld.w.v;
    } else {
        LToWNoScale = This->LocalToWorld;
    }
    math::Mat43 T;
    __m128 y = LToWNoScale.y.v;
    __m128 z = LToWNoScale.z.v;
    __m128 w = LToWNoScale.w.v;
    __m128 v7 = _mm_shuffle_ps(LToWNoScale.x.v, y, 0x44);
    __m128 v8 = _mm_shuffle_ps(v7, z, 0xDD);
    __m128 v16 = _mm_shuffle_ps(_mm_shuffle_ps(LToWNoScale.x.v, y, 0xEE), z, 0xA8);
    __m128 v9 = _mm_shuffle_ps(v7, z, 0x88);
    T.x.v = _mm_shuffle_ps(v9, _mm_setzero_ps(), 0xE4);
    T.y.v = v8;
    T.z.v = v16;
    T.w.v = _mm_xor_ps(_mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
                       _mm_add_ps(
                           _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(w, w, 0), v9),
                                      _mm_mul_ps(_mm_shuffle_ps(w, w, 85), v8)),
                           _mm_mul_ps(_mm_shuffle_ps(w, w, 170), v16)));
    *result = T;
    return result;
}

// ============================================================================
// nglDxSetupVShaderFog - ea: 0x84E7E0
// ============================================================================
void nglDxSetupVShaderFog(int VSReg, nglMeshNode* MeshNode, float FogNear, float FogFar,
                          float FogMin, float FogMax) {
    __m128 v11;
    v11.m128_f32[2] = 1.0f / (FogFar - FogNear);
    v11.m128_f32[0] = FogMin;
    v11.m128_f32[1] = FogNear;
    v11.m128_f32[3] = FogMax - FogMin;
    __m128 v10 = v11;
    math::Mat43 WToLNoScale;
    MeshNode_GetWToLNoScale(MeshNode, &WToLNoScale);
    __m128 v8 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(nglBuildScene->ViewToWorld.w.v,
                                             nglBuildScene->ViewToWorld.w.v, 0),
                              WToLNoScale.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(nglBuildScene->ViewToWorld.w.v,
                                             nglBuildScene->ViewToWorld.w.v, 85),
                              WToLNoScale.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(nglBuildScene->ViewToWorld.w.v,
                                             nglBuildScene->ViewToWorld.w.v, 170),
                              WToLNoScale.z.v),
                   WToLNoScale.w.v));
    v11 = _mm_shuffle_ps(v8, _mm_shuffle_ps(_mm_set1_ps(1.0f), v8, 160), 52);
    __m128 data[2];
    data[0] = v10;
    data[1] = v11;
    D3DDevice_SetVertexShaderConstantNotInlineFast(VSReg + 96, data, 8);
}

// ============================================================================
// nglDxSetupVShaderLights - ea: 0x84E8C0
// ============================================================================
void nglDxSetupVShaderLights(int VSReg, nglMeshNode* MeshNode) {
    math::Mat44 Dir;
    math::Mat44 Color;
    nglGetDirLightMatrix(MeshNode, &Dir, &Color);
    D3DDevice_SetVertexShaderConstantNotInlineFast(VSReg + 96, &Dir, 0x80);
}

// ============================================================================
// nglDxRegisterInternalShaders - ea: 0x84E910
// ============================================================================
void nglDxRegisterInternalShaders() {
    // microcode tables are null until Phase 6; skip rather than deref null
    if (nglDOFPixelShader::PShaderTable != nullptr)
    {
        nglDOFPixelShader::PS[0] = (unsigned int*)nglDOFPixelShader::PShaderTable[0];
        nglDOFPixelShader::Shader = (unsigned int*)nglDOFPixelShader::PShaderTable[0];
    }
    if (nglGlowShaderPixelPreFX::PShaderTable != nullptr)
    {
        nglGlowShaderPixelPreFX::PS[0] = (unsigned int*)nglGlowShaderPixelPreFX::PShaderTable[0];
        nglGlowShaderPixelPreFX::Shader = (unsigned int*)nglGlowShaderPixelPreFX::PShaderTable[0];
    }
    if (nglGlowShaderPixelFX::PShaderTable != nullptr)
    {
        nglGlowShaderPixelFX::PS[0] = (unsigned int*)nglGlowShaderPixelFX::PShaderTable[0];
        nglGlowShaderPixelFX::Shader = (unsigned int*)nglGlowShaderPixelFX::PShaderTable[0];
    }
    if (nglGlowShaderPixelPostFX::PShaderTable != nullptr)
    {
        nglGlowShaderPixelPostFX::PS[0] = (unsigned int*)nglGlowShaderPixelPostFX::PShaderTable[0];
        nglGlowShaderPixelPostFX::Shader = (unsigned int*)nglGlowShaderPixelPostFX::PShaderTable[0];
    }
}

// ============================================================================
// nglDxInitShaders - ea: 0x84E950
// ============================================================================
void nglDxInitShaders(bool RegisterShaders) {
    D3DDevice_SetShaderConstantMode(0x11u);
    unsigned int v2[4];
    v2[0] = 0;
    v2[1] = 1056964608;   // 0.5f
    v2[2] = 1065353216;   // 1.0f
    v2[3] = 0x40000000;   // 2.0f
    D3DDevice_SetVertexShaderConstant1Fast(1, v2);
    v2[0] = 1078530011;
    v2[1] = 1056964608;
    v2[2] = 1086918619;
    v2[3] = 1042479491;
    D3DDevice_SetVertexShaderConstant1Fast(2, v2);
    v2[0] = 1065353216;
    v2[1] = (unsigned int)-1090519040;
    v2[2] = 1026206379;
    v2[3] = (unsigned int)-1162474655;
    D3DDevice_SetVertexShaderConstant1Fast(3, v2);
    v2[0] = 1065353216;
    v2[1] = (unsigned int)-1104500053;
    v2[2] = 1007192201;
    v2[3] = (unsigned int)-1185936127;
    D3DDevice_SetVertexShaderConstant1Fast(4, v2);
    if (RegisterShaders)
        nglDxRegisterInternalShaders();
}

// ============================================================================
// nglDxSetupVShaderBones - ea: 0x84EAF0
// ============================================================================
void nglDxSetupVShaderBones(int VSReg, nglMeshNode* MeshNode, nglMeshSection* Section) {
    float v73[4];
    v73[0] = 765.00305f;
    v73[1] = (float)VSReg;
    v73[2] = nglFSAAParams.v.m128_f32[0];
    v73[3] = nglFSAAParams.v.m128_f32[1];
    D3DDevice_SetVertexShaderConstant1Fast(0, v73);
    nglMeshParams* MeshParams = MeshNode->MeshParams;
    nglSkeleton* Skeleton = (nglSkeleton*)MeshNode->Mesh->Skeleton;
    int NBones = Section->NBones;
    math::Mat43* Bones = MeshParams->Bones;
    if (NBones > 64
        && _tlAssert("src/dx/ngl_dx_shader.cpp", 113, "NBones <= MAX_BONES",
                     "Too many bones in mesh section for vertex shader constants."))
        __debugbreak();
    unsigned int Flags = MeshParams->Flags;
    __m128* dst = BonesArray_2;
    if ((Flags & 4) != 0) {
        math::Mat43 WToLNoScale;
        MeshNode_GetWToLNoScale(MeshNode, &WToLNoScale);
        for (int i = 0; i < NBones; ++i, dst += 3) {
            const math::Mat43* Rel = &Bones[Section->BoneIndices[i]];
            const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
            math::Mat43 M = MulMat43World(MulMat43World(Bone->InvRest, *Rel),
                                          WToLNoScale);
            Mat43ToConstants(M, dst);
        }
    } else if ((Flags & 8) != 0) {
        for (int i = 0; i < NBones; ++i, dst += 3) {
            const math::Mat43* Rel = &Bones[Section->BoneIndices[i]];
            const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
            math::Mat43 M = MulMat43World(Bone->InvRest, *Rel);
            Mat43ToConstants(M, dst);
        }
    } else if ((Flags & 0x10) != 0) {
        for (int i = 0; i < NBones; ++i, dst += 3)
            Mat43ToConstants(Bones[Section->BoneIndices[i]], dst);
    } else {
        for (int i = 0; i < NBones; ++i, dst += 3) {
            dst[0] = kIdentityX;
            dst[1] = kIdentityY;
            dst[2] = kIdentityZ;
        }
    }
    int v72 = 3 * NBones;
    if (v72 == 1)
        D3DDevice_SetVertexShaderConstant1Fast(VSReg + 96, BonesArray_2);
    else
        D3DDevice_SetVertexShaderConstantNotInlineFast(VSReg + 96, BonesArray_2, 4 * v72);
}

// ============================================================================
// nglDxSetBonesLocal - ea: 0x84F1E0
// ============================================================================
void nglDxSetBonesLocal(int p, nglMeshNode* MeshNode, nglMeshSection* Section) {
    nglMeshParams* MeshParams = MeshNode->MeshParams;
    int NBones = Section->NBones;
    math::Mat43* Bones = MeshParams->Bones;
    if (NBones > 48
        && _tlAssert("src/dx/ngl_dx_shader.cpp", 161, "NBones <= MAX_BONES",
                     "Too many bones in mesh section for vertex shader constants!"))
        __debugbreak();
    __m128* dst = BonesArray_3;
    if (NBones != 0) {
        unsigned int Flags = MeshParams->Flags;
        nglSkeleton* Skeleton = (nglSkeleton*)MeshNode->Mesh->Skeleton;
        if ((Flags & 4) != 0) {
            math::Mat43 WToLNoScale;
            MeshNode_GetWToLNoScale(MeshNode, &WToLNoScale);
            for (int i = 0; i < NBones; ++i, dst += 3) {
                const math::Mat43* Rel = &Bones[Section->BoneIndices[i]];
                const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
                math::Mat43 M = MulMat43World(MulMat43World(Bone->InvRest, *Rel),
                                              WToLNoScale);
                Mat43ToConstants(M, dst);
            }
        } else if ((Flags & 8) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                const math::Mat43* Rel = &Bones[Section->BoneIndices[i]];
                const nglSkeletonBone* Bone = &Skeleton->Bones[Section->BoneIndices[i]];
                math::Mat43 M = MulMat43World(Bone->InvRest, *Rel);
                Mat43ToConstants(M, dst);
            }
        } else if ((Flags & 0x10) != 0) {
            for (int i = 0; i < NBones; ++i, dst += 3)
                Mat43ToConstants(Bones[Section->BoneIndices[i]], dst);
        } else {
            for (int i = 0; i < NBones; ++i, dst += 3) {
                dst[0] = kIdentityX;
                dst[1] = kIdentityY;
                dst[2] = kIdentityZ;
            }
        }
    } else {
        NBones = 1;
        BonesArray_3[0] = kIdentityX;
        BonesArray_3[1] = kIdentityY;
        BonesArray_3[2] = kIdentityZ;
    }
    int v93 = 3 * NBones;
    if (v93 == 1)
        D3DDevice_SetVertexShaderConstant1Fast(p + 96, BonesArray_3);
    else
        D3DDevice_SetVertexShaderConstantNotInlineFast(p + 96, BonesArray_3, 4 * v93);
}
