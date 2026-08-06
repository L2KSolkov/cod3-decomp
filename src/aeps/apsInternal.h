// ============================================================================
// apsInternal — internal APS render-state + spawned-effect queue helpers.
// Source: c:\cod\code\tl\aeps\source\apsInternal.cpp
// Verified against IDA (aeps_xboxr:apsInternal.o):
//   Init                        @0x803240  (?Init@apsInternal@@YAXH@Z)
//   GetViewCoordinateSystem     @0x803250  (?GetViewCoordinateSystem@apsInternal@@YAXAAVDir3@math@@00@Z)
//   GetUserCoordinateSystem     @0x8033B0  (?GetUserCoordinateSystem@apsInternal@@YAXABVDir3@math@@AAV23@11@Z)
//   GetLocalLights              @0x8035C0  (?GetLocalLights@apsInternal@@YAXPAUnglLightContext@@ABUapsSphere@@@Z)
//   SetupBlendAndTexture        @0x8036D0  (?SetupBlendAndTexture@apsInternal@@YAXPAUnglTexture@@W4apsEBlendMode@@_NH@Z)
//   SetupFog                    @0x803990  (?SetupFog@apsInternal@@YAXHMMMM_N@Z)
//   SetupAlphaFade              @0x803A40  (?SetupAlphaFade@apsInternal@@YAXHMM@Z)
//   QueueSpawnedEffect          @0x803B00  (?QueueSpawnedEffect@apsInternal@@YAXHPBVapsEffectTemplate@@MABUapsQuaternion@@ABVDir3@math@@M@Z)
//   RemoveFromSpawnedEffectQueueByPakId  @0x803B30  (?RemoveFromSpawnedEffectQueueByPakId@apsInternal@@YAXH@Z)
//   ClearSpawnedEffectQueue     @0x803B50  (?ClearSpawnedEffectQueue@apsInternal@@YAXXZ)
//   GetLightMatrices            @0x803B60  (?GetLightMatrices@apsInternal@@YAXAAVMat43@math@@0ABV23@PAUnglLightContext@@ABUapsSphere@@@Z)
//   SubmitSpawnedEffectQueue    @0x803D90  (?SubmitSpawnedEffectQueue@apsInternal@@YAXXZ)
//   g_spawnedEffectQueue        @0x10DF9F0 (data, SpawnedEffectQueue 4112 bytes)
//   sMeshLightCat               @0xE49910  (data, int)
// ============================================================================
#ifndef COD3_AEPS_APSINTERNAL_H
#define COD3_AEPS_APSINTERNAL_H

#include "core/math_types.h"
#include "apsMath.h"            // apsQuaternion, apsMath
#include "apsCommon.h"          // apsCommon, apsClient (fwd)
#include "apsRenderNode.h"      // apsSphere
#include "apsShrimpRenderer.h"  // apsEBlendMode, nglLightContext (partial)

#include <cstddef>
#include <cstring>
#include <intrin.h>

struct nglLightContext;
struct nglTexture;
class apsEffect;
class apsEffectTemplate;

// Minimal apsClient view (full interface in render.o, unported). Only the
// virtual used by SpawnedEffectQueue::Submit is declared here.
// TODO(render.o): reconcile exact vtable slot order when apsClient is ported.
class apsClient {
public:
    virtual ~apsClient() = 0;   // ??1apsClient@@UAE@XZ (render.o)
    virtual apsEffect* CreateSpawnedEffectImmediate(int pakId,
        const apsEffectTemplate* effectTemplate, float startTime) = 0;  // render.o
};

// Minimal apsEffect view for apsInternal (full class in apsEffect.o, unported).
// Layout verified against IDA (128 bytes): mParentAgePercent @0x74.
class apsEffect {
public:
    void SetLocalToWorldTransform(const math::Mat43& iMatrix);  // apsEffect.o (non-inline)
    void SetParentAgePercent(float age) { mParentAgePercent = age; }  // inline COMDAT (apsInternal.o)
    static void ReportEffects();                              // ?ReportEffects@apsEffect@@SAXXZ (apsEffect.o)

    char  _pad[0x74];           // +0x00 (up to mParentAgePercent)
    float mParentAgePercent;    // +0x74
};
static_assert(offsetof(apsEffect, mParentAgePercent) == 0x74, "apsEffect::mParentAgePercent offset mismatch");

namespace apsInternal {

// ============================================================================
// SpawnedEffectQueue — fixed-capacity queue of deferred spawned effects.
// 4112 bytes: Entry[64] + m_size. Verified against IDA. `class` (V mangling)
// per the map (?g_spawnedEffectQueue@apsInternal@@3VSpawnedEffectQueue@1@A).
// ============================================================================
class SpawnedEffectQueue {
public:
    struct Entry {
        const apsEffectTemplate* m_template;        // +0x00
        float m_startTime;                          // +0x04
        apsQuaternion m_orientation;                // +0x08
        math::Dir3 m_pos;                           // +0x20
        float m_parentAgePercent;                   // +0x30
        int m_pakId;                                // +0x34

        Entry() {}                                  // ??0Entry@SpawnedEffectQueue@apsInternal@@QAE@XZ
    };
    static_assert(sizeof(Entry) == 0x40, "SpawnedEffectQueue::Entry size mismatch");

    static const unsigned int kMaxQueueSize = 0x40;

    Entry m_queue[kMaxQueueSize];  // +0x000 (4096 bytes)
    unsigned int m_size;           // +0x1000

    SpawnedEffectQueue() : m_size(0) {}                  // ??0SpawnedEffectQueue@apsInternal@@QAE@XZ

    void Clear() { m_size = 0; }                         // ?Clear@SpawnedEffectQueue@apsInternal@@QAEXXZ

    // inline COMDATs:
    void Add(int pakId, const apsEffectTemplate* effectTemplate, float startTime,
             const apsQuaternion& orientation, const math::Dir3& pos,
             float parentAgePercent) {                    // ?Add@SpawnedEffectQueue@apsInternal@@QAEXH...
        if (m_size < kMaxQueueSize) {
            Entry& e = m_queue[m_size];
            e.m_template = effectTemplate;
            e.m_startTime = startTime;
            e.m_orientation = orientation;
            e.m_pos = pos;
            e.m_parentAgePercent = parentAgePercent;
            e.m_pakId = pakId;
            ++m_size;
        }
    }
    void RemoveByPakId(int pakId) {                      // ?RemoveByPakId@SpawnedEffectQueue@apsInternal@@QAEXH@Z
        unsigned int i = 0;
        while (i < m_size) {
            if (m_queue[i].m_pakId == pakId) {
                unsigned int last = m_size - 1;
                if (i != last)
                    m_queue[i] = m_queue[last];
                --m_size;
            } else {
                ++i;
            }
        }
    }
    void Submit() {                                      // ?Submit@SpawnedEffectQueue@apsInternal@@QAEXXZ
        unsigned int count = m_size;
        if (count != 0) {
            SpawnedEffectQueue copy;
            memcpy(&copy, this, sizeof(Entry) * count);
            m_size = 0;

            apsClient* client = apsCommon::GetClient();
            for (unsigned int i = 0; i < count; ++i) {
                const Entry& e = copy.m_queue[i];
                apsEffect* effect = client->CreateSpawnedEffectImmediate(e.m_pakId, e.m_template, e.m_startTime);
                if (effect != 0) {
                    math::Mat43 matrix;
                    apsMath::GetQuaternionMatrix(matrix, e.m_orientation, e.m_pos);
                    effect->SetLocalToWorldTransform(matrix);
                    effect->SetParentAgePercent(e.m_parentAgePercent);
                }
            }
        }
    }
};
static_assert(sizeof(SpawnedEffectQueue) == 0x1010, "SpawnedEffectQueue size mismatch");

// ---- data (apsInternal.o) ----
extern int sMeshLightCat;                                       // data, 0xE49910
extern SpawnedEffectQueue g_spawnedEffectQueue;                 // ?g_spawnedEffectQueue@apsInternal@@3VSpawnedEffectQueue@1@A

// ---- non-inline functions (apsInternal.o) ----
void Init(int iMeshLightCat);                                   // ?Init@apsInternal@@YAXH@Z
void GetViewCoordinateSystem(math::Dir3& oForward, math::Dir3& oLeft, math::Dir3& oUp);   // ?GetViewCoordinateSystem@apsInternal@@YAXAAVDir3@math@@00@Z
void GetUserCoordinateSystem(const math::Dir3& userDir, math::Dir3& oForward,
                             math::Dir3& oLeft, math::Dir3& oUp);   // ?GetUserCoordinateSystem@apsInternal@@YAXABVDir3@math@@AAV23@11@Z

// ---- D3D render-state helpers (stubs: bodies require d3d8d internals) ----
void GetLocalLights(nglLightContext* ioLightContext, const apsSphere& iSphere);  // ?GetLocalLights@apsInternal@@YAXPAUnglLightContext@@ABUapsSphere@@@Z
void SetupBlendAndTexture(nglTexture* iTexture, apsEBlendMode iBlendMode,
                          bool bFogEnable, int alphaCutOff);  // ?SetupBlendAndTexture@apsInternal@@YAXPAUnglTexture@@W4apsEBlendMode@@_NH@Z
void SetupFog(int fogConst, float fogNear, float fogFar,
              float fogMin, float fogMax, bool fogEnable);   // ?SetupFog@apsInternal@@YAXHMMMM_N@Z
void SetupAlphaFade(int fadeConst, float fadeNear, float fadeFar);  // ?SetupAlphaFade@apsInternal@@YAXHMM@Z
void GetLightMatrices(math::Mat43& oLightDirMatrix, math::Mat43& oLightColorMatrix,
                      const math::Mat43& iWToL, nglLightContext* ioLightContext,
                      const apsSphere& iSphere);            // ?GetLightMatrices@apsInternal@@YAXAAVMat43@math@@0ABV23@PAUnglLightContext@@ABUapsSphere@@@Z

// ---- spawned-effect queue API ----
void QueueSpawnedEffect(int pakId, const apsEffectTemplate* effectTemplate, float startTime,
                        const apsQuaternion& orientation, const math::Dir3& pos,
                        float parentAgePercent);            // ?QueueSpawnedEffect@apsInternal@@YAXHPBVapsEffectTemplate@@MABUapsQuaternion@@ABVDir3@math@@M@Z
void RemoveFromSpawnedEffectQueueByPakId(int pakId);        // ?RemoveFromSpawnedEffectQueueByPakId@apsInternal@@YAXH@Z
void ClearSpawnedEffectQueue();                             // ?ClearSpawnedEffectQueue@apsInternal@@YAXXZ
void SubmitSpawnedEffectQueue();                            // ?SubmitSpawnedEffectQueue@apsInternal@@YAXXZ

// ---- inline COMDATs (apsInternal.o / apsShrimpRenderer.o) ----
// GetBlendColor — ambient blend color: Ambient.rgb * 1.5, alpha = 1.0.
// ea: 0x804400  (?GetBlendColor@apsInternal@@YA?AVVector4@math@@PAUnglLightContext@@@Z)
inline math::Vector4 GetBlendColor(nglLightContext* iLightContext) {
    math::Vector4 result;
    __m128 v3 = _mm_mul_ps(iLightContext->Ambient.v, _mm_set1_ps(1.5f));
    result.v = _mm_shuffle_ps(v3, _mm_shuffle_ps(_mm_set1_ps(1.0f), v3, 160), 52);
    return result;
}

// SetLightDir — store a light direction into dirMatrix column nLight.
// ea: 0x803ED0  (?SetLightDir@apsInternal@@YAXHAAVMat43@math@@ABVVector4@3@@Z)
inline void SetLightDir(int nLight, math::Mat43& dirMatrix, const math::Vector4& dir) {
    (&dirMatrix.x)[nLight].v = dir.v;
}

// SetLightColor — store a light color into colorMatrix column nLight.
// ea: 0x803F00  (?SetLightColor@apsInternal@@YAXHAAVMat43@math@@ABVVector4@3@@Z)
inline void SetLightColor(int nLight, math::Mat43& colorMatrix, const math::Vector4& color) {
    colorMatrix.x.v.m128_f32[nLight] = color.v.m128_f32[0];
    colorMatrix.y.v.m128_f32[nLight] = color.v.m128_f32[1];
    colorMatrix.z.v.m128_f32[nLight] = color.v.m128_f32[2];
}

} // namespace apsInternal

#endif // COD3_AEPS_APSINTERNAL_H
