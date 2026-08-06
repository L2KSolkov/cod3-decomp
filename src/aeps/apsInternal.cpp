// ============================================================================
// apsInternal.cpp — internal APS render-state + spawned-effect queue helpers.
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsInternal.cpp
//
// Port strategy:
//   - 7 clean functions ported in full (Init, Get*CoordinateSystem, and the
//     spawned-effect queue API).
//   - 5 D3D render-state/light helpers (GetLocalLights, SetupBlendAndTexture,
//     SetupFog, SetupAlphaFade, GetLightMatrices) are emitted as exact-signature
//     stubs. Their real bodies poke d3d8d driver internals (D3D__DirtyFlags,
//     D3D__TextureState, the NV097 dword_403xx method table) and walk full
//     nglLightContext/nglLightNode layouts — deferred with the render-state
//     layer (same category as the apsSimpleMeshNode/apsShrimpNode Renders).
//   - Inline COMDATs (SpawnedEffectQueue methods, apsQuaternion default ctor,
//     SetLightDir/SetLightColor, GetBlendColor) are in apsInternal.h.
// ============================================================================
#include "apsInternal.h"

// ============================================================================
// Data statics (apsInternal.o).
// ============================================================================
int apsInternal::sMeshLightCat = 0x02000000;   // @0xE49910 (observed init value)
apsInternal::SpawnedEffectQueue apsInternal::g_spawnedEffectQueue;   // @0x10DF9F0

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
// ---- D3D render-state helpers (stubs — bodies deferred) ---------------------
// The real bodies poke d3d8d driver internals and ngl light structures that
// belong to the (unported) render-state layer. Kept as exact-signature stubs
// so the 12 apsInternal.o symbols exist and the deferred node Renders can link.
// ============================================================================

// ea: 0x8035C0
void apsInternal::GetLocalLights(nglLightContext* ioLightContext, const apsSphere& iSphere) {
}

// ea: 0x8036D0
void apsInternal::SetupBlendAndTexture(nglTexture* iTexture, apsEBlendMode iBlendMode,
                                       bool bFogEnable, int alphaCutOff) {
}

// ea: 0x803990
void apsInternal::SetupFog(int fogConst, float fogNear, float fogFar,
                           float fogMin, float fogMax, bool fogEnable) {
}

// ea: 0x803A40
void apsInternal::SetupAlphaFade(int fadeConst, float fadeNear, float fadeFar) {
}

// ea: 0x803B60
void apsInternal::GetLightMatrices(math::Mat43& oLightDirMatrix, math::Mat43& oLightColorMatrix,
                                   const math::Mat43& iWToL, nglLightContext* ioLightContext,
                                   const apsSphere& iSphere) {
}
