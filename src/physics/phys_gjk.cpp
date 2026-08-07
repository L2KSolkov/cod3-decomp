// ============================================================================
// phys_gjk.cpp â€” GJK simplex algorithms (5 non-inline funcs).
// Source: source/phys_gjk.cpp (phys_xboxr)
// Verified against IDA (phys_xboxr:phys_gjk.o):
//   gjk_subalgorithm @0x87A250
//   seed_simplex     @0x87A420
//   gjk              @0x87A6A0
//   collide          @0x87AD80
//   phys_collide_do_gjk_collide @0x87B280
// ============================================================================

#include "pulse_sum.h"

#include <math.h>
#include <string.h>
#include <intrin.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// phys_gjk_info::gjk_subalgorithm â€” ea: 0x87A250
// ============================================================================
int phys_gjk_info::gjk_subalgorithm(int w_set, int new_index) {
    int v3 = 0;
    int w_set_index_list[4];
    int w_set_index_list_count = 0;
    if ((w_set & 1) != 0 && new_index != 0) {
        w_set_index_list_count = 1;
        w_set_index_list[0] = 0;
        v3 = 1;
    }
    if ((w_set & 2) != 0 && new_index != 1) {
        w_set_index_list[v3++] = 1;
        w_set_index_list_count = v3;
    }
    if ((w_set & 4) != 0 && new_index != 2) {
        w_set_index_list[v3++] = 2;
        w_set_index_list_count = v3;
    }
    if ((w_set & 8) != 0 && new_index != 3) {
        w_set_index_list[v3++] = 3;
        w_set_index_list_count = v3;
    }
    int* v6 = &w_set_index_list[v3];
    int v7 = 1 << new_index;
    int v8 = 20 * (1 << new_index);
    char* v9 = (char*)this->m_set_list + v8;
    *(float*)&v9[4 * new_index] = 1.0f;
    *(int*)(v9 + 4) = 1;
    int v16 = 1 << new_index;
    int index_1 = v8;
    if (w_set_index_list_count == 0)
        return w_set;
    int* i = w_set_index_list;
    if (w_set_index_list != v6) {
        while (1) {
            this->comp_lambda_2(new_index, *i++);
            if (i == v6)
                break;
        }
        v8 = index_1;
    }
    if (*(int*)&this->m_set_list[0].m_lamda[4] + v8 != 0)
        return 1 << new_index;
    if (w_set_index_list_count == 1)
        return w_set;
    int* v11 = w_set_index_list;
    int* v12;
    if (w_set_index_list != v6 - 1) {
        do {
            v12 = v11 + 1;
            int* i2 = v11 + 1;
            if (v11 + 1 != v6) {
                index_1 = *v11;
                do {
                    this->comp_lambda_3(new_index, index_1, *v12++);
                } while (v12 != v6);
                v12 = i2;
            }
            v11 = v12;
        } while (v12 != v6 - 1);
        v7 = v16;
    }
    int* v13 = w_set_index_list;
    if (w_set_index_list == v6) {
        if (w_set_index_list_count != 2) {
            if (w_set_index_list_count != 3 &&
                _tlAssert("source/phys_gjk.cpp", 318, "w_set_index_list_count == 3", ""))
                __debugbreak();
            this->comp_lambda_4();
            for (int v14 = 0; v14 < 4; ++v14) {
                if (v14 != new_index) {
                    int result = ~(1 << v14) & 0xF;
                    if (this->m_set_list[result].m_candidate != 0)
                        return result;
                }
            }
            return w_set;
        }
        return w_set;
    }
    while (1) {
        int result = v7 | (1 << *v13);
        if (this->m_set_list[result].m_candidate != 0)
            return result;
        if (++v13 == v6)
            goto LABEL_32;
    }
LABEL_32:
    if (w_set_index_list_count != 2) {
        if (w_set_index_list_count != 3 &&
            _tlAssert("source/phys_gjk.cpp", 318, "w_set_index_list_count == 3", ""))
            __debugbreak();
        this->comp_lambda_4();
        for (int v14 = 0; v14 < 4; ++v14) {
            if (v14 != new_index) {
                int result = ~(1 << v14) & 0xF;
                if (this->m_set_list[result].m_candidate != 0)
                    return result;
            }
        }
        return w_set;
    }
    return w_set;
}

// ============================================================================
// phys_gjk_info::seed_simplex â€” ea: 0x87A420
// ============================================================================
int phys_gjk_info::seed_simplex(int cached_vert_count) {
    if (cached_vert_count <= 0 &&
        _tlAssert("source/phys_gjk.cpp", 337, "cached_vert_count > 0", ""))
        __debugbreak();
    if (cached_vert_count >= 4 &&
        _tlAssert("source/phys_gjk.cpp", 338, "cached_vert_count < 4", ""))
        __debugbreak();
    if (cached_vert_count > 0) {
        for (int i = cached_vert_count; i != 0; --i) {
            this->m_a_verts[i - 1].v = _mm_add_ps(
                _mm_sub_ps(this->m_a_verts[i].v, this->m_b_verts[i].v),
                this->m_gjk_sep_vec.v);
        }
    }
    int v24 = 0;
    if (cached_vert_count > 0) {
        float* v6 = this->m_dot_ij[0];
        __m128* v7 = &this->m_w_verts[0].v;
        float* v20 = this->m_dot_ij[0];
        __m128* v22 = &this->m_w_verts[0].v;
        do {
            float* v8 = v6;
            int v9 = cached_vert_count - v24;
            do {
                __m128 v10 = _mm_mul_ps(*v22, *v7);
                float v19 = v10.m128_f32[0]
                            + (_mm_shuffle_ps(v10, v10, 85).m128_f32[0]
                               + _mm_shuffle_ps(v10, v10, 170).m128_f32[0]);
                *v8 = v19;
                *v6 = v19;
                ++v7;
                v6 += 4;
                ++v8;
                --v9;
            } while (v9 != 0);
            v7 = v22 + 1;
            v6 = v20 + 5;
            ++v24;
            ++v22;
            v20 += 5;
        } while (v24 < cached_vert_count);
    }
    for (int j = 0; j < cached_vert_count; ++j) {
        int v12 = 1 << j;
        this->m_set_list[v12].m_lamda[j] = 1.0f;
        this->m_set_list[v12].m_candidate = 1;
    }
    if (cached_vert_count == 1)
        return 1;
    int v23 = 0;
    if (cached_vert_count - 1 > 0) {
        int v15;
        do {
            v15 = v23 + 1;
            if (v23 + 1 < cached_vert_count) {
                do {
                    this->comp_lambda_2(v23, v15++);
                } while (v15 < cached_vert_count);
                v15 = v23 + 1;
            }
            v23 = v15;
        } while (v15 < cached_vert_count - 1);
    }
    if (cached_vert_count <= 0) {
        if (cached_vert_count == 2) {
            return 3;
        } else {
            this->comp_lambda_3(0, 1, 2);
            int v17 = 0;
            while (1) {
                if (cached_vert_count - 1 <= 0) {
                    if (cached_vert_count != 3 &&
                        _tlAssert("source/phys_gjk.cpp", 384, "cached_vert_count == 3", ""))
                        __debugbreak();
                    return 7;
                }
                int v18 = v17 + 1;
                if (v17 + 1 >= cached_vert_count) {
                    v17 = v18;
                    if (v18 >= cached_vert_count - 1)
                        goto check3;
                } else {
                    while (1) {
                        int result = (1 << v17) | (1 << v18);
                        if (this->m_set_list[result].m_candidate != 0)
                            return result;
                        if (++v18 >= cached_vert_count) {
                            v18 = v17 + 1;
                            v17 = v18;
                            if (v18 >= cached_vert_count - 1)
                                goto check3;
                        }
                    }
                }
            }
        }
    } else {
        for (int v16 = 0; v16 < cached_vert_count; ++v16) {
            if (this->m_set_list[1 << v16].m_candidate != 0)
                return 1 << v16;
        }
        if (cached_vert_count == 2)
            return 3;
        this->comp_lambda_3(0, 1, 2);
    }
check3:
    if (cached_vert_count != 3 &&
        _tlAssert("source/phys_gjk.cpp", 384, "cached_vert_count == 3", ""))
        __debugbreak();
    return 7;
}

// ============================================================================
// phys_gjk_info::gjk â€” ea: 0x87A6A0
// ============================================================================
phys_gjk_info::gjk_retval_e phys_gjk_info::gjk(phys_collide_data* d,
                                               const math::Dir3* initial_support_dir,
                                               bool in_separation_loop) {
    this->m_lower_dist_sq = -34.0f;
    this->m_upper_dist_sq = 34.0f;
    this->m_gjk_iter = this->init_gjk(d, initial_support_dir, in_separation_loop);
    int v12;
    while (1) {
        __m128 v8 = _mm_mul_ps(this->m_support_dir.v, this->m_support_dir.v);
        float v72 = v8.m128_f32[0]
                    + (_mm_shuffle_ps(v8, v8, 85).m128_f32[0]
                       + _mm_shuffle_ps(v8, v8, 170).m128_f32[0]);
        float v10 = v72;
        this->m_upper_dist_sq = v72;
        if (this->m_gjk_iter != 0 && (PEN_THRESH * PEN_THRESH) > v10)
            return GJK_PENETRATING;
        int m_w_set = this->m_w_set;
        if ((m_w_set & 1) != 0) {
            if ((m_w_set & 2) != 0)
                v12 = (this->m_w_set & 4 | 8u) >> 2;
            else
                v12 = 1;
        } else {
            v12 = 0;
        }
        const phys_gjk_geom* gjk_cg1 = d->gjk_cg1;
        __m128 v62 = _mm_xor_ps(Float4_SignMask_203.v, this->m_support_dir.v);
        math::Dir3 v61;
        math::Dir3 v63;
        const math::Dir3* v14 = gjk_cg1->support(&v61, (const math::Mat43*)&v62, &v63);
        float v15 = v14->v.m128_f32[0];
        math::Dir3* v16 = &this->m_a_verts[v12];
        v16->v.m128_f32[0] = v15;
        v16->v.m128_f32[1] = v14->v.m128_f32[1];
        v16->v.m128_f32[2] = v14->v.m128_f32[2];
        v16->v.m128_f32[3] = v14->v.m128_f32[3];
        __m128 v = v16->v;
        float v70 = v16->v.m128_f32[0];
        float v82 = v.m128_f32[0];
        if (v.m128_f32[0] != v70
            || v16->v.m128_f32[1] != v16->v.m128_f32[1]
            || v16->v.m128_f32[2] != v16->v.m128_f32[2]) {
            if (_tlAssert("source/phys_gjk.cpp", 464,
                          "(m_a_verts[new_index].GetX() == m_a_verts[new_index].GetX() && m_a_verts[new_index].GetY() == m_a_verts[new_index].GetY() && m_a_verts[new_index].GetZ() == m_a_verts[new_index].GetZ())",
                          "invalid vector"))
                __debugbreak();
        }
        math::Dir3 v20;
        v20.v = this->cg2_to_cg1_xform.y.v;
        const phys_gjk_geom* gjk_cg2 = d->gjk_cg2;
        __m128 v22 = _mm_shuffle_ps(this->cg2_to_cg1_xform.x.v, v20.v, 68);
        v63.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_set1_ps(this->m_support_dir.v.m128_f32[0]),
                           _mm_shuffle_ps(v22, this->cg2_to_cg1_xform.z.v, 136)),
                _mm_mul_ps(_mm_set1_ps(this->m_support_dir.v.m128_f32[1]),
                           _mm_shuffle_ps(v22, this->cg2_to_cg1_xform.z.v, 221))),
            _mm_mul_ps(_mm_set1_ps(this->m_support_dir.v.m128_f32[2]),
                       _mm_shuffle_ps(_mm_shuffle_ps(this->cg2_to_cg1_xform.x.v, v20.v, 238),
                                      this->cg2_to_cg1_xform.z.v, 168)));
        math::Dir3 v60;
        const math::Dir3* v23 = gjk_cg2->support(&v60, &this->cg2_to_cg1_xform, &v63);
        float v24 = v23->v.m128_f32[0];
        math::Dir3* v25 = &this->m_b_loc_verts[v12];
        math::Dir3* m_w_verts = v25;
        v25->v.m128_f32[0] = v24;
        v25->v.m128_f32[1] = v23->v.m128_f32[1];
        v25->v.m128_f32[2] = v23->v.m128_f32[2];
        v25->v.m128_f32[3] = v23->v.m128_f32[3];
        __m128 v26 = v25->v;
        float v66 = v25->v.m128_f32[0];
        float v76 = v26.m128_f32[0];
        if (v26.m128_f32[0] != v66
            || v25->v.m128_f32[1] != v25->v.m128_f32[1]
            || v25->v.m128_f32[2] != v25->v.m128_f32[2]) {
            if (_tlAssert("source/phys_gjk.cpp", 470,
                          "(m_b_loc_verts[new_index].GetX() == m_b_loc_verts[new_index].GetX() && m_b_loc_verts[new_index].GetY() == m_b_loc_verts[new_index].GetY() && m_b_loc_verts[new_index].GetZ() == m_b_loc_verts[new_index].GetZ())",
                          "invalid vector"))
                __debugbreak();
        }
        __m128 v64[2];
        v64[1] = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_set1_ps(v25->v.m128_f32[0]), this->cg2_to_cg1_xform.x.v),
                _mm_mul_ps(_mm_set1_ps(v25->v.m128_f32[1]), this->cg2_to_cg1_xform.y.v)),
            _mm_add_ps(
                _mm_mul_ps(_mm_set1_ps(v25->v.m128_f32[2]), this->cg2_to_cg1_xform.z.v),
                this->cg2_to_cg1_xform.w.v));
        math::Dir3* v30 = &this->m_b_verts[v12];
        v30->v = v64[1];
        math::Dir3 v31;
        v31.v = this->m_support_dir.v;
        __m128 v32 = _mm_add_ps(_mm_sub_ps(v16->v, v30->v), this->m_gjk_sep_vec.v);
        __m128 v33 = _mm_mul_ps(v31.v, v32);
        float v83 = v33.m128_f32[0]
                    + (_mm_shuffle_ps(v33, v33, 85).m128_f32[0]
                       + _mm_shuffle_ps(v33, v33, 170).m128_f32[0]);
        v64[0] = v32;
        if (v83 > 0.0f) {
            float m_upper_dist_sq = this->m_upper_dist_sq;
            if (m_upper_dist_sq > 0.0f) {
                float v35 = (v83 / m_upper_dist_sq) * v83;
                if (v35 > this->m_lower_dist_sq) {
                    bool v36 = (this->m_flags & 1) == 0;
                    this->m_lower_dist_sq = v35;
                    if (!v36 && v35 > (this->m_gjk_sep_thresh * this->m_gjk_sep_thresh))
                        return GJK_SEPARATED;
                }
            }
        }
        if (this->m_gjk_iter != 0) {
            if (this->m_lower_dist_sq > ((this->m_upper_dist_sq * (1.0f - CONV_THRESH)) * (1.0f - CONV_THRESH)))
                return GJK_VALID;
            __m128 v37 = _mm_sub_ps(v32, v31.v);
            __m128 v38 = _mm_mul_ps(v37, v37);
            float v77 = v38.m128_f32[0]
                        + (_mm_shuffle_ps(v38, v38, 85).m128_f32[0]
                           + _mm_shuffle_ps(v38, v38, 170).m128_f32[0]);
            __m128 v39 = _mm_mul_ps(this->m_support_dir.v, this->m_support_dir.v);
            __m128 v40 = _mm_mul_ps(v37, v31.v);
            float v79 = v39.m128_f32[0]
                        + (_mm_shuffle_ps(v39, v39, 85).m128_f32[0]
                           + _mm_shuffle_ps(v39, v39, 170).m128_f32[0]);
            float v81 = v40.m128_f32[0]
                        + (_mm_shuffle_ps(v40, v40, 85).m128_f32[0]
                           + _mm_shuffle_ps(v40, v40, 170).m128_f32[0]);
            if (((v79 * v77) * 0.000027415317f) >= (v81 * v81))
                return GJK_VALID;
        }
        int v41 = 0;
        int v42 = 1;
        math::Dir3* m_w_verts2 = this->m_w_verts;
        do {
            if ((v42 & this->m_last_w_set) != 0) {
                __m128 v43 = _mm_sub_ps(v32, m_w_verts2->v);
                __m128 v44 = _mm_mul_ps(v43, v43);
                float v75 = v44.m128_f32[0]
                            + (_mm_shuffle_ps(v44, v44, 85).m128_f32[0]
                               + _mm_shuffle_ps(v44, v44, 170).m128_f32[0]);
                if (v75 < 0.0000010000001f)
                    return GJK_VALID;
            }
            ++v41;
            v42 *= 2;
            ++m_w_verts2;
        } while (v41 < 4);
        float v45 = v64[0].m128_f32[1];
        math::Dir3* v46 = &this->m_w_verts[v12];
        __m128* p_v = &v46->v;
        v46->v = v64[0];
        int v49 = this->m_w_set;
        this->m_w_set = (1 << v12) | v49;
        int v50 = 0;
        math::Dir3* v51 = this->m_w_verts;
        float* m_w_verts3 = this->m_dot_ij[v12];
        do {
            if ((v50 == v12) || ((1 << v50) & this->m_w_set) != 0) {
                __m128 v52 = _mm_mul_ps(v51->v, *p_v);
                float v71 = v52.m128_f32[0]
                            + (_mm_shuffle_ps(v52, v52, 85).m128_f32[0]
                               + _mm_shuffle_ps(v52, v52, 170).m128_f32[0]);
                *m_w_verts3 = v71;
                this->m_dot_ij[v12][v50] = v71;
            }
            ++m_w_verts3;
            ++v50;
            ++v51;
        } while (v50 < 4);
        int v57 = this->m_w_set;
        this->m_last_w_set = v57;
        int v53 = this->gjk_subalgorithm(v57, v12);
        this->m_w_set = v53;
        if (this->m_set_list[v53].m_candidate == 0)
            goto LABEL_44;
        if (v53 == 15)
            break;
        if (this->comp_v(v53, &this->m_support_dir)) {
            int v54 = this->m_gjk_iter + 1;
            this->m_gjk_iter = v54;
            if (v54 == 30)
                return GJK_VALID;
            continue;
        }
    LABEL_44:
        this->m_w_set = this->m_last_w_set & ~(1 << v12);
        return GJK_VALID;
    }
    if (this->m_lower_dist_sq <= 0.0f)
        return GJK_PENETRATING;
    this->m_w_set = this->m_last_w_set & ~(1 << v12);
    return GJK_VALID;
}

// ============================================================================
// phys_gjk_info::collide â€” ea: 0x87AD80
// ============================================================================
phys_gjk_info::gjk_retval_e phys_gjk_info::collide(phys_collide_data* d) {
    math::Dir3 v29[3];
    memset(v29, 0, 16);
    this->m_gjk_sep_vec.v = _mm_setzero_ps();
    this->m_flags |= 1;
    this->get_initial_support_dir(v29, d);
    gjk_retval_e result = this->gjk(d, v29, false);
    if (result != GJK_INVALID) {
        if (result != GJK_PENETRATING) {
            if (result < GJK_PENETRATING)
                return result;
            goto LABEL_6;
        }
        const math::Dir3* v10;
        math::Dir3 v28;
        phys_gjk_cache_info* gjk_ci = d->gjk_ci;
        if (gjk_ci != NULL && (gjk_ci->m_flags & 4) != 0)
            v10 = v29;
        else
            v10 = &gjk_sep_dir::comp_sep_dir(&v28, d, this);
        v29[1] = *v10;
        __m128 v14 = _mm_mul_ps(v29[1].v, v29[1].v);
        float v37 = v14.m128_f32[0]
                    + (_mm_shuffle_ps(v14, v14, 85).m128_f32[0]
                       + _mm_shuffle_ps(v14, v14, 170).m128_f32[0]);
        if (v37 < 0.0000000099999991f) {
            __m128 v15 = _mm_mul_ps(v29[0].v, v29[0].v);
            v37 = v15.m128_f32[0]
                  + (_mm_shuffle_ps(v15, v15, 85).m128_f32[0]
                     + _mm_shuffle_ps(v15, v15, 170).m128_f32[0]);
            if (v37 < 0.0000000099999991f)
                return GJK_INVALID;
            v29[1] = v29[0];
        }
        this->m_flags &= ~1u;
        int v38 = 0;
        math::Dir3 v27;
        math::Dir3 v26;
        while (1) {
            ++v38;
            math::Dir3 v28n;
            v28n.v = _mm_xor_ps(Float4_SignMask_203.v, v29[1].v);
            const phys_gjk_geom* gjk_cg2 = d->gjk_cg2;
            const math::Dir3* v17 = gjk_cg2->support(&v27, &this->cg2_to_cg1_xform,
                                                     &v29[1]);
            const phys_gjk_geom* gjk_cg1 = d->gjk_cg1;
            math::Dir3 v28m;
            v28m.v = v28n.v;
            const math::Dir3* v19 = gjk_cg1->support(&v26, (const math::Mat43*)&v28m, NULL);
            __m128 v20 = _mm_sub_ps(v19->v, v17->v);
            v29[0].v = v20;
            __m128 v21 = _mm_mul_ps(v29[1].v, v29[1].v);
            float v37b = v21.m128_f32[0]
                         + (_mm_shuffle_ps(v21, v21, 85).m128_f32[0]
                            + _mm_shuffle_ps(v21, v21, 170).m128_f32[0]);
            __m128 v22 = _mm_mul_ps(v29[1].v, v20);
            float v40 = v22.m128_f32[0]
                        + (_mm_shuffle_ps(v22, v22, 85).m128_f32[0]
                           + _mm_shuffle_ps(v22, v22, 170).m128_f32[0]);
            bool v25 = v38 > 1;
            float v36 = 17.0f / sqrt(v37b) - v40 / v37b;
            this->m_gjk_sep_vec.v = _mm_mul_ps(v29[1].v, _mm_set1_ps(v36));
            if (this->gjk(d, &v29[1], v25) == GJK_PENETRATING) {
                if (_tlAssert("source/phys_gjk.cpp", 1381, "retv != GJK_PENETRATING", ""))
                    __debugbreak();
                if (v38 != 1)
                    return GJK_INVALID;
                phys_gjk_cache_info* v23 = d->gjk_ci;
                if (v23 == NULL || (v23->m_flags & 8) == 0)
                    return GJK_INVALID;
                v23->m_flags &= ~8u;
                if (this->gjk(d, &v29[1], false) == GJK_PENETRATING)
                    break;
            }
            if (!this->comp_v(this->m_w_set, &v29[1]) &&
                _tlAssert("source/phys_gjk.cpp", 1394, "comp_v_retv", ""))
                __debugbreak();
            __m128 v24 = _mm_mul_ps(v29[1].v, v29[1].v);
            float v36b = v24.m128_f32[0]
                         + (_mm_shuffle_ps(v24, v24, 85).m128_f32[0]
                            + _mm_shuffle_ps(v24, v24, 170).m128_f32[0]);
            if (v36b < (PEN_THRESH * PEN_THRESH) &&
                _tlAssert("source/phys_gjk.cpp", 1395,
                          "AbsSquared(support_dir) >= phys_sqr(PEN_THRESH)", ""))
                __debugbreak();
            if (((this->m_lower_dist_sq * (1.0f - SEP_CONV_THRESH)) * (1.0f - SEP_CONV_THRESH)) < 289.0f
                || v38 >= 10)
                return GJK_PENETRATING;
        }
        if (_tlAssert("source/phys_gjk.cpp", 1388, "retv != GJK_PENETRATING", ""))
            __debugbreak();
        return GJK_INVALID;
    }
    if (_tlAssert("source/phys_gjk.cpp", 1346, "retv != GJK_INVALID", ""))
        __debugbreak();
LABEL_6:
    bool v8 = !_tlAssert("source/phys_gjk.cpp", 1350,
                         "retv == GJK_SEPARATED || retv == GJK_VALID", "");
    if (!v8)
        __debugbreak();
    return result;
}

// ============================================================================
// phys_gjk_info::phys_collide_do_gjk_collide â€” ea: 0x87B280
// ============================================================================
bool phys_gjk_info::phys_collide_do_gjk_collide(phys_collide_data* d, float sep_thresh) {
    phys_full_inv_multiply_mat(this->cg2_to_cg1_xform, *d->cg1_to_world_xform,
                               *d->cg2_to_world_xform);
    const phys_gjk_geom* gjk_cg1 = d->gjk_cg1;
    const phys_gjk_geom* gjk_cg2 = d->gjk_cg2;
    d->cg2_to_cg1_xform = &this->cg2_to_cg1_xform;
    d->cg1_cinfo_loc = &this->cg1_cinfo_loc;
    float v15 = gjk_cg1->get_geom_radius();
    this->m_gjk_sep_thresh = gjk_cg2->get_geom_radius() + v15 + sep_thresh;
    gjk_retval_e v8 = this->collide(d);
    bool v9 = false;
    __m128 v13;
    if (v8 == GJK_INVALID) {
        v9 = _tlAssert("source/phys_gjk.cpp", 1461, "0", "phys_gjk failed");
        goto LABEL_3;
    }
    if (v8 == GJK_SEPARATED) {
        this->gjk_cache_update_separated(d);
        return false;
    }
    if (v8 != GJK_VALID && v8 != GJK_PENETRATING &&
        _tlAssert("source/phys_gjk.cpp", 1472,
                  "retv == GJK_VALID || retv == GJK_PENETRATING", ""))
        __debugbreak();
    int m_w_set = this->m_w_set;
    if (m_w_set == 15) {
        v9 = _tlAssert("source/phys_gjk.cpp", 1475, "0", "phys_gjk failed");
        goto LABEL_3;
    }
    if ((m_w_set >= 15 || m_w_set <= 0) &&
        _tlAssert("source/phys_gjk.cpp", 1479, "m_w_set < 15 && m_w_set > 0", ""))
        __debugbreak();
    this->comp_closest_points(this->m_w_set, &this->cg1_cinfo_loc.m_p1,
                              &this->cg1_cinfo_loc.m_p2);
    this->comp_v(this->m_w_set, &this->cg1_cinfo_loc.m_n);
    v13 = _mm_mul_ps(this->cg1_cinfo_loc.m_n.v, this->cg1_cinfo_loc.m_n.v);
    float len = sqrt(v13.m128_f32[0]
                     + (_mm_shuffle_ps(v13, v13, 85).m128_f32[0]
                        + _mm_shuffle_ps(v13, v13, 170).m128_f32[0]));
    this->cg1_cinfo_loc.m_n.v = _mm_div_ps(this->cg1_cinfo_loc.m_n.v, _mm_set1_ps(len));
    float r1 = d->gjk_cg1->get_geom_radius();
    this->cg1_cinfo_loc.m_p1.v = _mm_sub_ps(this->cg1_cinfo_loc.m_p1.v,
                                            _mm_mul_ps(this->cg1_cinfo_loc.m_n.v, _mm_set1_ps(r1)));
    float r2 = d->gjk_cg2->get_geom_radius();
    this->cg1_cinfo_loc.m_p2.v = _mm_add_ps(this->cg1_cinfo_loc.m_p2.v,
                                            _mm_mul_ps(this->cg1_cinfo_loc.m_n.v, _mm_set1_ps(r2)));
    this->gjk_cache_update_colliding(d);
    return true;

LABEL_3:
    if (v9)
        __debugbreak();
    phys_gjk_cache_info* gjk_ci = d->gjk_ci;
    if (gjk_ci != NULL) {
        gjk_ci->m_flags |= 1u;
        d->gjk_ci->m_flags &= ~4u;
        d->gjk_ci->m_flags &= ~8u;
    }
    return false;
}
