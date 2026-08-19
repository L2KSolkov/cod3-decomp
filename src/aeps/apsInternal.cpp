// ============================================================================
// apsInternal.cpp — internal APS render-state + spawned-effect queue helpers.
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsInternal.cpp
//
// Port strategy:
//   - 7 clean functions ported in full (Init, Get*CoordinateSystem, and the
//     spawned-effect queue API).
//   - SetupBlendAndTexture, SetupFog, and SetupAlphaFade are ported from the
//     release D3D state paths. The light-context helpers remain deferred until
//     their full driver/light layouts are reconstructed.
//   - Inline COMDATs (SpawnedEffectQueue methods, apsQuaternion default ctor,
//     SetLightDir/SetLightColor, GetBlendColor) are in apsInternal.h.
// ============================================================================
#include "apsInternal.h"
#include "d3d8.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_state.h"

// D3D method selectors and XDK state-cache cells are owned by the Win32 shim.
extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_40344;
extern unsigned int dword_40348;
extern unsigned int dword_40350;
extern unsigned int dword_BC2CF8;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2D08;
extern unsigned int dword_BC2D0C;
extern unsigned int dword_BC2D38;
extern unsigned int dword_BC2D80;
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data statics (apsInternal.o).
// ============================================================================
int apsInternal::sMeshLightCat = 0x02000000;   // @0xE49910 (observed init value)
apsInternal::SpawnedEffectQueue apsInternal::g_spawnedEffectQueue;   // @0x10DF9F0
int Opaque = 0;                                 // @0x10DF9D8 (IDA global)

// ============================================================================
// apsQuaternion::apsQuaternion — default ctor (empty body). apsInternal.o
// owns this symbol (? ?0apsQuaternion@@QAE@XZ); apsMath.o references it UNDEF.
// ea: 0x803E20
// ============================================================================
apsQuaternion::apsQuaternion() {
}

// ============================================================================
// apsInternal::Init — record the mesh light category.
// ea: 0x803240
// ============================================================================
void apsInternal::Init(int iMeshLightCat) {
    sMeshLightCat = iMeshLightCat;
}

// ============================================================================
// apsInternal::GetViewCoordinateSystem — camera view basis from the scene
// ViewDir: forward = ViewDir, left = normalize(ViewDir x ZAxis) * XFlip,
// up = normalize(left x ViewDir). Verified against IDA disasm (SSE shuffles).
// ea: 0x803250
// ============================================================================
void apsInternal::GetViewCoordinateSystem(math::Dir3& oForward, math::Dir3& oLeft, math::Dir3& oUp) {
    __m128 v = nglBuildScene->ViewDir.v;
    __m128 v5 = _mm_shuffle_ps(v, v, 18);   // (z, x, y, x)
    __m128 v6 = _mm_shuffle_ps(v, v, 9);    // (y, z, x, x)

    __m128 zAxis = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);   // (0,0,1,0)
    __m128 v7 = _mm_sub_ps(
        _mm_mul_ps(v6, _mm_shuffle_ps(zAxis, zAxis, 18)),
        _mm_mul_ps(v5, _mm_shuffle_ps(zAxis, zAxis, 9)));   // ViewDir x ZAxis

    __m128 v8 = _mm_mul_ps(v7, v7);
    float len = sqrtf(v8.m128_f32[0] + (v8.m128_f32[1] + v8.m128_f32[2]));
    __m128 v9 = _mm_div_ps(v7, _mm_set1_ps(len));          // normalize(left)

    __m128 v10 = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(v9, v9, 9), v5),
        _mm_mul_ps(_mm_shuffle_ps(v9, v9, 18), v6));       // left x ViewDir
    __m128 v11 = _mm_mul_ps(v10, v10);
    float len2 = sqrtf(v11.m128_f32[0] + (v11.m128_f32[1] + v11.m128_f32[2]));

    oForward.v = v;
    oLeft.v = _mm_mul_ps(v9, _mm_set1_ps(apsCommon::mCamera.mXFlip));
    oUp.v = _mm_div_ps(v10, _mm_set1_ps(len2));
}

// ============================================================================
// apsInternal::GetUserCoordinateSystem — orthonormal basis from an arbitrary
// user direction. Reference up is YAxis unless userDir is near-parallel to
// ZAxis (|f.z| > 0.98), in which case a permuted fallback is used.
// Verified against IDA disasm (SSE shuffles).
// ea: 0x8033B0
// ============================================================================
void apsInternal::GetUserCoordinateSystem(const math::Dir3& userDir, math::Dir3& oForward,
                                          math::Dir3& oLeft, math::Dir3& oUp) {
    __m128 v4 = _mm_mul_ps(userDir.v, userDir.v);
    float len = sqrtf(v4.m128_f32[0] + (v4.m128_f32[1] + v4.m128_f32[2]));
    __m128 v5 = _mm_div_ps(userDir.v, _mm_set1_ps(len));   // normalized forward

    __m128 zAxis = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);     // (0,0,1,0)
    __m128 v6 = _mm_mul_ps(v5, zAxis);
    float zc = v6.m128_f32[0] + (v6.m128_f32[1] + v6.m128_f32[2]);

    __m128 up;
    if (fabsf(zc) <= 0.98000002f) {
        up = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);           // YAxis (0,1,0,0)
    } else {
        math::Dir3 fwd;
        fwd.v.m128_f32[0] = v5.m128_f32[1];
        fwd.v.m128_f32[1] = v5.m128_f32[2];
        fwd.v.m128_f32[2] = v5.m128_f32[0];
        up = fwd.v;
    }

    __m128 v8 = _mm_shuffle_ps(v5, v5, 18);   // (z, x, y, x)
    __m128 v9 = _mm_shuffle_ps(v5, v5, 9);    // (y, z, x, x)
    __m128 v10 = _mm_sub_ps(
        _mm_mul_ps(v9, _mm_shuffle_ps(up, up, 18)),
        _mm_mul_ps(v8, _mm_shuffle_ps(up, up, 9)));   // forward x up
    __m128 v11 = _mm_mul_ps(v10, v10);
    float len2 = sqrtf(v11.m128_f32[0] + (v11.m128_f32[1] + v11.m128_f32[2]));
    __m128 v12 = _mm_div_ps(v10, _mm_set1_ps(len2));   // normalize(left)

    __m128 v13 = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(v12, v12, 9), v8),
        _mm_mul_ps(_mm_shuffle_ps(v12, v12, 18), v9)); // left x forward
    __m128 v14 = _mm_mul_ps(v13, v13);
    float len3 = sqrtf(v14.m128_f32[0] + (v14.m128_f32[1] + v14.m128_f32[2]));

    oForward.v = v5;
    oLeft.v = _mm_mul_ps(v12, _mm_set1_ps(apsCommon::mCamera.mXFlip));
    oUp.v = _mm_div_ps(v13, _mm_set1_ps(len3));
}

// ============================================================================
// apsInternal::QueueSpawnedEffect — append to the deferred spawn queue.
// ea: 0x803B00
// ============================================================================
void apsInternal::QueueSpawnedEffect(int pakId, const apsEffectTemplate* effectTemplate,
                                     float startTime, const apsQuaternion& orientation,
                                     const math::Dir3& pos, float parentAgePercent) {
    g_spawnedEffectQueue.Add(pakId, effectTemplate, startTime, orientation, pos, parentAgePercent);
}

// ============================================================================
// apsInternal::RemoveFromSpawnedEffectQueueByPakId.
// ea: 0x803B30
// ============================================================================
void apsInternal::RemoveFromSpawnedEffectQueueByPakId(int pakId) {
    g_spawnedEffectQueue.RemoveByPakId(pakId);
}

// ============================================================================
// apsInternal::ClearSpawnedEffectQueue.
// ea: 0x803B50
// ============================================================================
void apsInternal::ClearSpawnedEffectQueue() {
    g_spawnedEffectQueue.Clear();
}

// ============================================================================
// apsInternal::SubmitSpawnedEffectQueue — drain the queue, spawning each effect
// through the APS client.
// ea: 0x803D90
// ============================================================================
void apsInternal::SubmitSpawnedEffectQueue() {
    g_spawnedEffectQueue.Submit();
}

// ============================================================================
// ---- D3D render-state helpers -----------------------------------------------
// ============================================================================

// ea: 0x8035C0
void apsInternal::GetLocalLights(nglLightContext* ioLightContext, const apsSphere& iSphere) {
    const int lightCategory = static_cast<int>(static_cast<signed char>(
        (static_cast<unsigned int>(sMeshLightCat) >> 24) & 0xFFu));
    const int listIndex = lightCategory - 1;
    nglLightNode* const head = &ioLightContext->Head;

    head->LocalNext = head;
    nglLightNode* light = head->Next[listIndex];
    const math::Vector4& sphere = iSphere.mSphere;
    const nglLightNode* sentinel = reinterpret_cast<const nglLightNode*>(ioLightContext);

    while (light != sentinel) {
        switch (light->Type) {
        case NGLLIGHT_POINT: {
            const __m128* nodeData = static_cast<const __m128*>(light->NodeData);
            const __m128 delta = _mm_sub_ps(nodeData[0], sphere.v);
            const __m128 squared = _mm_mul_ps(delta, delta);
            const float distanceSquared = squared.m128_f32[0] +
                squared.m128_f32[1] + squared.m128_f32[2];
            const float radius = nodeData[2].m128_f32[1] + sphere.v.m128_f32[3];
            if (radius * radius < distanceSquared) {
                light = light->Next[listIndex];
                continue;
            }
            break;
        }
        case NGLLIGHT_DIRECTIONAL:
            break;
        default:
            if (_tlAssert("source/apsInternal.cpp", 127, "0", "Invalid light type."))
                __debugbreak();
            light = light->Next[listIndex];
            continue;
        }

        light->LocalNext = head->LocalNext;
        head->LocalNext = light;
        light = light->Next[listIndex];
    }
}

// ea: 0x8136B0
unsigned int apsInternal::ClampToColor32(const math::Vector4& iBlendColor,
                                          const math::Vector4& iParticleColor) {
    const __m128 particle = _mm_mul_ps(iParticleColor.v, _mm_set1_ps(255.0f));
    const __m128 blend = iBlendColor.v;
    const __m128 blendWithAlpha = _mm_shuffle_ps(_mm_set1_ps(1.0f), blend, 0xA0);
    const __m128 factors = _mm_shuffle_ps(blend, blendWithAlpha, 0x34);
    const __m128 clamped = _mm_min_ps(
        _mm_max_ps(_mm_mul_ps(particle, factors), _mm_setzero_ps()),
        _mm_set1_ps(255.0f));

    const unsigned int r = static_cast<unsigned int>(clamped.m128_f32[0]);
    const unsigned int g = static_cast<unsigned int>(clamped.m128_f32[1]);
    const unsigned int b = static_cast<unsigned int>(clamped.m128_f32[2]);
    const unsigned int a = static_cast<unsigned int>(clamped.m128_f32[3]);
    return b | (g << 8) | (r << 16) | (a << 24);
}

// ea: 0x8036D0
void apsInternal::SetupBlendAndTexture(nglTexture* iTexture, apsEBlendMode iBlendMode,
                                       bool bFogEnable, int alphaCutOff) {
    nglDxSetTexture(0, iTexture, 1u, 3u);

    if (nglDxTexCache.Prev[0].WrapU != 3u) {
        nglDxTexCache.Prev[0].WrapU = 3u;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 3u) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSU] = 3u;
        }
    }
    if (nglDxTexCache.Prev[0].WrapV != 3u) {
        nglDxTexCache.Prev[0].WrapV = 3u;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 3u) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSV] = 3u;
        }
    }

    if (Opaque != 0) {
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40304, 0);
            dword_BC2CFC = 0;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40300, 0);
            dword_BC2D00 = 0;
        }
    } else {
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40304, 1u);
            dword_BC2CFC = 1;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40300, 1u);
            dword_BC2D00 = 1;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x204u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_4033C, 0x204u);
            dword_BC2CF8 = 0x204u;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF,
                                                     static_cast<unsigned int>(alphaCutOff)) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40340, static_cast<unsigned int>(alphaCutOff));
            dword_BC2D04 = static_cast<unsigned int>(alphaCutOff);
        }
    }

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);

    nglDxState.PrevBM = static_cast<unsigned int>(-1);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX,
                                                 static_cast<unsigned int>(bFogEnable)) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = static_cast<unsigned int>(bFogEnable);
    }

    switch (iBlendMode) {
    case apsEBlendMode_Blend:
    case apsEBlendMode_BlendWithLighting:
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, 0x8006u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40350, 0x8006u);
            dword_BC2D38 = 0x8006u;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, 0x302u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40344, 0x302u);
            dword_BC2D08 = 0x302u;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, 0x303u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40348, 0x303u);
            dword_BC2D0C = 0x303u;
        }
        break;
    case apsEBlendMode_Add:
        nglDxState.SetBlendOp(0x8006u);
        nglDxState.SetSrcBlend(0x302u);
        nglDxState.SetDestBlend(1u);
        break;
    case apsEBlendMode_Subtract:
        nglDxState.SetBlendOp(0x800Bu);
        nglDxState.SetSrcBlend(0x302u);
        nglDxState.SetDestBlend(1u);
        break;
    case apsEBlendMode_NumBlendModes:
        nglDxState.SetBlendOp(0x8006u);
        nglDxState.SetSrcBlend(1u);
        nglDxState.SetDestBlend(0x301u);
        break;
    default:
        return;
    }
}

// ea: 0x803990
void apsInternal::SetupFog(int fogConst, float fogNear, float fogFar,
                           float fogMin, float fogMax, bool fogEnable) {
    (void)fogEnable;
    float Range = fogFar - fogNear;
    if (Range == 0.0f)
        Range = 1.0f;

    float Intercept = 1.0f - fogMin;
    float Slope = ((1.0f - fogMax) - Intercept) / Range;
    Intercept -= Slope * fogNear;

    __m128 Zero = _mm_setzero_ps();
    __m128 InterceptVector = _mm_set_ss(Intercept);
    __m128 SlopeVector = _mm_set_ss(Slope);
    __m128 Packed = _mm_shuffle_ps(
        _mm_shuffle_ps(InterceptVector, SlopeVector, 0), SlopeVector, 0xE2);
    __m128 ZeroMix = _mm_shuffle_ps(Zero, Packed, 0xF0);
    Packed = _mm_shuffle_ps(Packed, ZeroMix, 0xC4);
    ZeroMix = _mm_shuffle_ps(Zero, Packed, 0xA0);
    Packed = _mm_shuffle_ps(Packed, ZeroMix, 0x34);
    D3DDevice_SetVertexShaderConstant1Fast((unsigned int)(fogConst + 96), &Packed);
}

// ea: 0x803A40
void apsInternal::SetupAlphaFade(int fadeConst, float fadeNear, float fadeFar) {
    float Range = fadeFar - fadeNear;
    __m128 Zero = _mm_setzero_ps();
    __m128 Packed;
    if (Range == 0.0f) {
        Packed = _mm_shuffle_ps(_mm_shuffle_ps(_mm_set1_ps(1.0f), Zero, 0),
                                Zero, 0xE2);
    } else {
        float InverseRange = 1.0f / Range;
        float Offset = -InverseRange * fadeNear;
        __m128 OffsetVector = _mm_set_ss(Offset);
        __m128 InverseVector = _mm_set_ss(InverseRange);
        Packed = _mm_shuffle_ps(
            _mm_shuffle_ps(OffsetVector, InverseVector, 0), InverseVector, 0xE2);
    }
    __m128 ZeroMix = _mm_shuffle_ps(Zero, Packed, 0xF0);
    Packed = _mm_shuffle_ps(Packed, ZeroMix, 0xC4);
    ZeroMix = _mm_shuffle_ps(Zero, Packed, 0xA0);
    Packed = _mm_shuffle_ps(Packed, ZeroMix, 0x34);
    D3DDevice_SetVertexShaderConstant1Fast((unsigned int)(fadeConst + 96), &Packed);
}

// ea: 0x803B60
void apsInternal::GetLightMatrices(math::Mat43& oLightDirMatrix, math::Mat43& oLightColorMatrix,
                                   const math::Mat43& iWToL, nglLightContext* ioLightContext,
                                   const apsSphere& iSphere) {
    GetLocalLights(ioLightContext, iSphere);

    math::Position3 spherePosition;
    spherePosition.v = iSphere.mSphere.v;
    nglLightNode* light = ioLightContext->Head.LocalNext;
    const nglLightNode* sentinel = reinterpret_cast<const nglLightNode*>(ioLightContext);
    const __m128 signMask = _mm_set1_ps(-0.0f);
    unsigned int lightCount = 0;

    while (light != sentinel && lightCount < 4u) {
        nglDirLightInfo lightInfo;
        nglDirLightInfo* asDirLight = nglGetLightAsDirLight(
            &lightInfo, light, spherePosition);
        if (asDirLight != nullptr) {
            const __m128 dir = asDirLight->Dir.v;
            __m128 transformed = _mm_mul_ps(
                _mm_shuffle_ps(dir, dir, 0), iWToL.x.v);
            transformed = _mm_add_ps(transformed, _mm_mul_ps(
                _mm_shuffle_ps(dir, dir, 0x55), iWToL.y.v));
            transformed = _mm_add_ps(transformed, _mm_mul_ps(
                _mm_shuffle_ps(dir, dir, 0xAA), iWToL.z.v));

            __m128 zero = _mm_setzero_ps();
            __m128 packed = _mm_shuffle_ps(
                transformed, _mm_shuffle_ps(zero, transformed, 0xA0), 0x34);
            math::Vector4* dirRows = reinterpret_cast<math::Vector4*>(&oLightDirMatrix.x);
            dirRows[lightCount].v = _mm_xor_ps(signMask, packed);

            oLightColorMatrix.x.v.m128_f32[lightCount] =
                asDirLight->Color.v.m128_f32[0];
            oLightColorMatrix.y.v.m128_f32[lightCount] =
                asDirLight->Color.v.m128_f32[1];
            oLightColorMatrix.z.v.m128_f32[lightCount] =
                asDirLight->Color.v.m128_f32[2];
            ++lightCount;
        }

        light = light->LocalNext;
    }

    for (unsigned int index = lightCount; index < 4u; ++index) {
        math::Vector4* dirRows = reinterpret_cast<math::Vector4*>(&oLightDirMatrix.x);
        dirRows[index].v = _mm_setzero_ps();
        oLightColorMatrix.x.v.m128_f32[index] = 0.0f;
        oLightColorMatrix.y.v.m128_f32[index] = 0.0f;
        oLightColorMatrix.z.v.m128_f32[index] = 0.0f;
    }

    oLightColorMatrix.w = ioLightContext->Ambient;
}
