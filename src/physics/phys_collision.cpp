// ============================================================================
// phys_collision.cpp — collision feature helpers (1 non-inline func).
// Source: phys_xboxr:phys_collision.o
// Verified against IDA:
//   calc_feature_normal @0x878A90
//   (process @0x878C10 — large; ported separately if/when infra available)
// ============================================================================
#include "physics/phys_types.h"
#include "core/math_types.h"

#include <intrin.h>

// ============================================================================
// calc_feature_normal — find the mesh triangle whose normal is most aligned
// with the hit normal (bit-faithful to the release build's SSE shuffles).
// ea: 0x878A90
// ============================================================================
const math::Dir3 calc_feature_normal(phys_contact_manifold* cman, const math::Dir3& hitn) {
    math::Dir3 result;
    // best normal halves (x,z lanes packed as the decompiler grouped them)
    __int64 normal_12 = *(__int64*)&hitn.v.m128_f32[2];
    __int64 bestXZ = *(__int64*)&hitn.v.m128_f32[0];

    float bestScore = 0.99240386f;

    contact_manifold_mesh_point* list = cman->m_list_mesh_point;
    contact_manifold_mesh_point* end = &list[cman->m_list_mesh_point_count];

    for (contact_manifold_mesh_point* a = list; a < end; ++a) {
        math::Dir3 base = a->m_p;
        for (contact_manifold_mesh_point* b = a + 1; b < end; ++b) {
            __m128 v1 = b->m_p.v;
            v1 = _mm_sub_ps(v1, base.v);
            for (contact_manifold_mesh_point* c = b + 1; c < end; ++c) {
                __m128 v2 = c->m_p.v;
                v2 = _mm_sub_ps(v2, base.v);

                // cross = v1 x v2, computed in permuted lane order [x, z, y]
                __m128 v13 = _mm_shuffle_ps(v1, v1, 9);   // [y1, x1, z1, x1]
                __m128 v12 = _mm_shuffle_ps(v1, v1, 18);  // [z1, y1, x1, x1]
                __m128 cross = _mm_sub_ps(
                    _mm_mul_ps(v13, _mm_shuffle_ps(v2, v2, 18)),
                    _mm_mul_ps(v12, _mm_shuffle_ps(v2, v2, 9)));

                __m128 sq = _mm_mul_ps(cross, cross);
                float lenSq = sq.m128_f32[0]
                            + (_mm_shuffle_ps(sq, sq, 85).m128_f32[0]
                               + _mm_shuffle_ps(sq, sq, 170).m128_f32[0]);
                if (lenSq > 0.0000099999997f) {
                    __m128 dp = _mm_mul_ps(cross, hitn.v);
                    float dot = dp.m128_f32[0]
                              + (_mm_shuffle_ps(dp, dp, 85).m128_f32[0]
                                 + _mm_shuffle_ps(dp, dp, 170).m128_f32[0]);
                    float score = (dot / lenSq) * dot;
                    if (score >= bestScore) {
                        bestXZ = *(__int64*)&cross.m128_f32[0];
                        normal_12 = *(__int64*)&cross.m128_f32[2];
                        bestScore = score;
                    }
                }
            }
        }
    }

    result.v.m128_f32[0] = *(float*)&bestXZ;
    result.v.m128_f32[1] = *(float*)((char*)&bestXZ + 4);
    result.v.m128_f32[2] = *(float*)&normal_12;
    result.v.m128_f32[3] = *(float*)((char*)&normal_12 + 4);
    return result;
}

// ?process@phys_contact_manifold_process@@QAEXPAVphys_collide_data@@@Z
// (ea: 0x878C10, 2678B; stub until the manifold processing infra is ported)
void phys_contact_manifold_process::process(phys_collide_data* d)
{
    (void)d;
}
