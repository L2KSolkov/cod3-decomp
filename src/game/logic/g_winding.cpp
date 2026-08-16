// ============================================================================
// g_winding.cpp - game.o winding helpers (cm_load.cpp / polylib)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// proximity_data_t placement allocators - ea: 0x60BFE0..0x60C000
// ============================================================================
// ea: 0x0060BFE0
void* proximity_data_t::operator new(size_t s, TPakId pakID)
{
    return PakManager::sInst->MemAlloc(pakID, (unsigned int)s, true);
}

// ea: 0x0060C000
void proximity_data_t::operator delete(void* ptr, TPakId pakID)
{
    PakManager::sInst->MemFree(pakID, ptr, true);
}

// ============================================================================
// winding_t - polygon winding (points * 12 bytes + 4-byte numpoints)
// ============================================================================
struct winding_t {
    int   numpoints;   // +0x00
    float p[1][3];     // +0x04
};

int c_active_windings;   // ?c_active_windings@@3HA (game.o)
int c_peak_windings;     // ?c_peak_windings@@3HA (game.o)
extern "C" void* _Z_MallocInternal(int size);  // core.o
extern "C" void  _Z_FreeInternal(void* ptr);            // core.o
extern void Com_Memcpy(void* dest, const void* src, unsigned int count);
    // ?Com_Memcpy@@YAXPAD0H@Z
winding_t* CopyWinding(winding_t* w);  // ea: 0x609880 (defined below)

// cdl_array<cdlPlane> (cdl_mem.h; layout verified by IDA types)
template <typename T>
class cdl_array {
public:
    unsigned int m_count;
    T*           m_elements;
};

// ea: 0x0062A040
void clip_winding(ae_sized_array<math::Position3, 256>& winding_ref,
                  const cdlPlane& clip)
{
    ae_sized_array<math::Position3, 256>* winding = &winding_ref;
    unsigned int m_size = winding->m_size;
    float dists[257];
    int side[257];
    int side_count[3] = { 0, 0, 0 };
    for (unsigned int i = 0; i < m_size; ++i)
    {
        float d = (*winding)[i].v.m128_f32[0] * clip.data.m128_f32[0]
                + (*winding)[i].v.m128_f32[1] * clip.data.m128_f32[1]
                + (*winding)[i].v.m128_f32[2] * clip.data.m128_f32[2]
                - clip.data.m128_f32[3];
        dists[i] = d;
        if (d <= 0.1f)
        {
            if (d >= -0.1f)
                side[i] = 2;
            else
                side[i] = 1;
        }
        else
            side[i] = 0;
        ++side_count[side[i]];
    }
    dists[m_size] = dists[0];
    side[m_size] = side[0];
    if (side_count[0] != 0)
    {
        if (side_count[1] == 0)
            return;  // fully inside; keep winding unchanged
        ae_sized_array<math::Position3, 256> out;
        out.m_size = 0;
        for (unsigned int i = 0; i < m_size; ++i)
        {
            if (side[i] == 2)
            {
                out.push_back((*winding)[i]);
            }
            else
            {
                if (side[i] == 0)
                    out.push_back((*winding)[i]);
                unsigned int next = (i + 1) % m_size;
                if (side[next] != 2 && side[next] != side[i])
                {
                    float frac = dists[i] / (dists[i] - dists[next]);
                    math::Position3 p;
                    for (int c = 0; c < 3; ++c)
                    {
                        if (clip.data.m128_f32[c] == 1.0f)
                            p.v.m128_f32[c] = clip.data.m128_f32[3];
                        else if (clip.data.m128_f32[c] == -1.0f)
                            p.v.m128_f32[c] = -clip.data.m128_f32[3];
                        else
                            p.v.m128_f32[c] =
                                ((*winding)[next].v.m128_f32[c]
                                 - (*winding)[i].v.m128_f32[c])
                                    * frac
                                + (*winding)[i].v.m128_f32[c];
                    }
                    p.v.m128_f32[3] = 0.0f;
                    out.push_back(p);
                }
            }
        }
        *winding = out;
    }
    else
    {
        winding->m_size = 0;
    }
}

// ea: 0x00620010
void init_winding(const cdlPlane& plane,
                  ae_sized_array<math::Position3, 256>& winding_ref)
{
    ae_sized_array<math::Position3, 256>* winding = &winding_ref;
    float org[3];
    org[0] = fabs(plane.data.m128_f32[0]);
    org[1] = fabs(plane.data.m128_f32[1]);
    org[2] = fabs(plane.data.m128_f32[2]);
    float axis[3];
    if (org[2] <= org[org[1] > org[0] ? 1 : 0])
    {
        axis[0] = 0.0f;
        axis[1] = 0.0f;
        axis[2] = 1.0f;
    }
    else
    {
        axis[0] = 1.0f;
        axis[1] = 0.0f;
        axis[2] = 0.0f;
    }
    float v16 =
        axis[0] * plane.data.m128_f32[0]
        + axis[1] * plane.data.m128_f32[1]
        + axis[2] * plane.data.m128_f32[2];
    float v8[3];
    v8[0] = axis[0] - plane.data.m128_f32[0] * v16;
    v8[1] = axis[1] - plane.data.m128_f32[1] * v16;
    v8[2] = axis[2] - plane.data.m128_f32[2] * v16;
    float len = sqrt(v8[0] * v8[0] + v8[1] * v8[1] + v8[2] * v8[2]);
    if (len != 0.0f)
    {
        v8[0] /= len;
        v8[1] /= len;
        v8[2] /= len;
    }
    float v12[3];
    v12[0] = v8[1] * plane.data.m128_f32[2]
           - v8[2] * plane.data.m128_f32[1];
    v12[1] = v8[2] * plane.data.m128_f32[0]
           - v8[0] * plane.data.m128_f32[2];
    v12[2] = v8[0] * plane.data.m128_f32[1]
           - v8[1] * plane.data.m128_f32[0];
    v12[0] *= 131072.0f;
    v12[1] *= 131072.0f;
    v12[2] *= 131072.0f;
    math::Position3 p;
    p.v.m128_f32[0] = plane.data.m128_f32[0] * plane.data.m128_f32[3]
        + v8[0] * 131072.0f + v12[0];
    p.v.m128_f32[1] = plane.data.m128_f32[1] * plane.data.m128_f32[3]
        + v8[1] * 131072.0f + v12[1];
    p.v.m128_f32[2] = plane.data.m128_f32[2] * plane.data.m128_f32[3]
        + v8[2] * 131072.0f + v12[2];
    p.v.m128_f32[3] = 0.0f;
    winding->push_back(p);
    p.v.m128_f32[0] = plane.data.m128_f32[0] * plane.data.m128_f32[3]
        - v8[0] * 131072.0f + v12[0];
    p.v.m128_f32[1] = plane.data.m128_f32[1] * plane.data.m128_f32[3]
        - v8[1] * 131072.0f + v12[1];
    p.v.m128_f32[2] = plane.data.m128_f32[2] * plane.data.m128_f32[3]
        - v8[2] * 131072.0f + v12[2];
    winding->push_back(p);
    p.v.m128_f32[0] = plane.data.m128_f32[0] * plane.data.m128_f32[3]
        - v8[0] * 131072.0f - v12[0];
    p.v.m128_f32[1] = plane.data.m128_f32[1] * plane.data.m128_f32[3]
        - v8[1] * 131072.0f - v12[1];
    p.v.m128_f32[2] = plane.data.m128_f32[2] * plane.data.m128_f32[3]
        - v8[2] * 131072.0f - v12[2];
    winding->push_back(p);
    p.v.m128_f32[0] = plane.data.m128_f32[0] * plane.data.m128_f32[3]
        + v8[0] * 131072.0f - v12[0];
    p.v.m128_f32[1] = plane.data.m128_f32[1] * plane.data.m128_f32[3]
        + v8[1] * 131072.0f - v12[1];
    p.v.m128_f32[2] = plane.data.m128_f32[2] * plane.data.m128_f32[3]
        + v8[2] * 131072.0f - v12[2];
    winding->push_back(p);
}

// ea: 0x0062A6B0
void calc_winding(const cdl_array<cdlPlane>& planes, int plane_index,
                  ae_sized_array<math::Position3, 256>& winding_ref)
{
    ae_sized_array<math::Position3, 256>* winding = &winding_ref;
    const cdlPlane* v4 = &planes.m_elements[plane_index];
    init_winding(*v4, winding_ref);
    int m_count = planes.m_count;
    bool v19 = false;
    for (unsigned int v6 = 0; v6 < (unsigned int)m_count; ++v6)
    {
        if (v6 == plane_index)
        {
            v19 = true;
        }
        else
        {
            const cdlPlane* v8 = &planes.m_elements[v6];
            float v17 = v4->data.m128_f32[0] * v8->data.m128_f32[0]
                      + v4->data.m128_f32[1] * v8->data.m128_f32[1]
                      + v4->data.m128_f32[2] * v8->data.m128_f32[2];
            if (v17 <= 0.99989998f
                || fabs(v4->data.m128_f32[3] - v8->data.m128_f32[3])
                    >= 0.001f)
            {
                cdlPlane clip;
                clip.data = _mm_xor_ps(
                    *(__m128*)v8, _mm_set1_ps(-0.0f));
                clip.data.m128_f32[3] = -v8->data.m128_f32[3];
                clip_winding(winding_ref, clip);
                if (winding->m_size < 3)
                    return;
            }
            else if (v19)
            {
                winding->m_size = 0;
                return;
            }
        }
    }
}

// ea: 0x006095A0
winding_t* AllocWinding(int points)
{
    int v1 = ++c_active_windings;
    if (c_active_windings > c_peak_windings)
        c_peak_windings = v1;
    return (winding_t*)_Z_MallocInternal(12 * points + 4);
}

// ea: 0x006095E0
void FreeWinding(winding_t* w)
{
    if (w->numpoints == -559030611)
        Com_Error(ERR_FATAL, "FreeWinding: freed a freed winding");
    w->numpoints = -559030611;
    --c_active_windings;
    _Z_FreeInternal(w);
}

// ea: 0x00609620
void RemoveColinearPoints(winding_t* w)
{
    float p[64][3];
    int nump = 0;
    if (w->numpoints > 0)
    {
        int v3 = 0;
        do
        {
            int numpoints = w->numpoints;
            int v13 = v3 + 1;
            int j = (v3 + 1) % numpoints;
            float v1[3];
            v1[0] = w->p[j][0] - w->p[v3][0];
            v1[1] = w->p[j][1] - w->p[v3][1];
            v1[2] = w->p[j][2] - w->p[v3][2];
            int prev = (numpoints + v3 - 1) % numpoints;
            float v2[3];
            v2[0] = w->p[v3][0] - w->p[prev][0];
            v2[1] = w->p[v3][1] - w->p[prev][1];
            v2[2] = w->p[v3][2] - w->p[prev][2];
            VectorNormalize2(v1, v1);
            VectorNormalize2(v2, v2);
            if (v2[2] * v1[2] + v2[1] * v1[1] + v2[0] * v1[0] < 0.999f)
            {
                p[nump][0] = w->p[v3][0];
                p[nump][1] = w->p[v3][1];
                p[nump][2] = w->p[v3][2];
                ++nump;
            }
            v3 = v13;
        } while (v3 < w->numpoints);
    }
    if (nump != w->numpoints)
    {
        w->numpoints = nump;
        memcpy(w->p, p, 12 * nump);
    }
}

// ea: 0x0060AA40
void AddWindingToConvexHull(winding_t* w, winding_t** hull,
                            float* const normal)
{
    if (*hull == nullptr)
    {
        *hull = CopyWinding(w);
        return;
    }
    int numpoints = (*hull)->numpoints;
    float hullPoints[128][3];
    Com_Memcpy((char*)hullPoints, (char*)(*hull)->p,
               12 * (*hull)->numpoints);
    int i = 0;
    if (w->numpoints > 0)
    {
        float* v7 = w->p[0];
        float* v51 = w->p[0];
        do
        {
            int v8 = 0;
            float hullDirs[128][3];
            if (numpoints > 0)
            {
                int v9 = 0;
                do
                {
                    int v11 = (v8 + 1) % numpoints;
                    float dir[3];
                    dir[0] = hullPoints[v11][0] - hullPoints[v9][0];
                    dir[1] = hullPoints[v11][1] - hullPoints[v9][1];
                    dir[2] = hullPoints[v11][2] - hullPoints[v9][2];
                    VectorNormalize2(dir, dir);
                    CrossProduct(normal, dir, hullDirs[v9]);
                    v8 = v8 + 1;
                    ++v9;
                } while (v8 < numpoints);
                v7 = v51;
            }
            int v12 = 0;
            int v13 = 0;
            int hullSide[128];
            float newHullPoints[128][3];
            if (numpoints > 0)
            {
                int v14 = 0;
                do
                {
                    float v15 = v7[1] - hullPoints[v14][1];
                    float v16 = *v7 - hullPoints[v14][0];
                    float v17 = hullDirs[v14][2] * (v7[2] - hullPoints[v14][2]);
                    float v18 = hullDirs[v14][1] * v15;
                    float v19 = (v17 + v18) + (hullDirs[v14][0] * v16);
                    if (v19 >= 0.1f)
                        v12 = 1;
                    hullSide[v13++] = v19 >= -0.1f;
                    ++v14;
                } while (v13 < numpoints);
                if (v12 != 0)
                {
                    int m = 0;
                    for (; m < numpoints; ++m)
                    {
                        if (hullSide[m % numpoints] == 0
                            && hullSide[(m + 1) % numpoints] != 0)
                            break;
                    }
                    if (m != numpoints)
                    {
                        int v21 = (m + 1) % numpoints;
                        newHullPoints[0][0] = *v7;
                        newHullPoints[0][1] = v7[1];
                        newHullPoints[0][2] = v7[2];
                        int numNew = 1;
                        int v23 = 0;
                        int j = v21;
                        if (numpoints >= 4)
                        {
                            int v55 = v21 + 2;
                            int v52 = ((numpoints - 4) >> 2) + 1;
                            int k = 4 * v52;
                            do
                            {
                                if (hullSide[(v55 - 2) % numpoints] == 0
                                    || hullSide[(v55 - 1) % numpoints] == 0)
                                {
                                    int v27 = (v55 - 1) % numpoints;
                                    float* copy = hullPoints[v27];
                                    newHullPoints[numNew][0] = copy[0];
                                    newHullPoints[numNew][1] = copy[1];
                                    newHullPoints[numNew][2] = copy[2];
                                    ++numNew;
                                    if (hullSide[v27] == 0)
                                        goto L54;
                                }
                                if (hullSide[v55 % numpoints] == 0)
                                {
                                L54:
                                    int v28 = v55 % numpoints;
                                    float* copy = hullPoints[v55 % numpoints];
                                    newHullPoints[numNew][0] = copy[0];
                                    newHullPoints[numNew][1] = copy[1];
                                    newHullPoints[numNew][2] = copy[2];
                                    ++numNew;
                                    if (hullSide[v28] == 0)
                                        goto L53;
                                }
                                if (hullSide[(v55 + 1) % numpoints] == 0)
                                {
                                L53:
                                    int v29 = (v55 + 1) % numpoints;
                                    float* copy = hullPoints[v29];
                                    newHullPoints[numNew][0] = copy[0];
                                    newHullPoints[numNew][1] = copy[1];
                                    newHullPoints[numNew][2] = copy[2];
                                    ++numNew;
                                    if (hullSide[v29] == 0)
                                        goto L33;
                                }
                                if (hullSide[(v55 + 2) % numpoints] == 0)
                                {
                                L33:
                                    float* copy = hullPoints[(v55 + 2) % numpoints];
                                    newHullPoints[numNew][0] = copy[0];
                                    newHullPoints[numNew][1] = copy[1];
                                    newHullPoints[numNew][2] = copy[2];
                                    ++numNew;
                                }
                                v55 += 4;
                                --v52;
                            } while (v52 != 1);
                            v23 = k;
                            v21 = j;
                            v7 = v51;
                        }
                        if (v23 < numpoints)
                        {
                            int v35 = v23 + v21 + 1;
                            int v52 = numpoints - v23;
                            do
                            {
                                if (hullSide[(v35 - 1) % numpoints] == 0
                                    || hullSide[v35 % numpoints] == 0)
                                {
                                    float* v37 = hullPoints[v35 % numpoints];
                                    newHullPoints[numNew][0] = v37[0];
                                    newHullPoints[numNew][1] = v37[1];
                                    newHullPoints[numNew][2] = v37[2];
                                    ++numNew;
                                }
                                ++v35;
                                --v52;
                            } while (v52 != 0);
                            v7 = v51;
                        }
                        numpoints = numNew;
                        Com_Memcpy((char*)hullPoints, (char*)newHullPoints,
                                   12 * numNew);
                    }
                }
            }
            v7 += 3;
            ++i;
            v51 = v7;
        } while (i < w->numpoints);
    }
    winding_t* v40 = *hull;
    if (v40->numpoints == -559030611)
        Com_Error(ERR_FATAL, "FreeWinding: freed a freed winding");
    v40->numpoints = -559030611;
    --c_active_windings;
    _Z_FreeInternal(v40);
    int v41 = ++c_active_windings;
    if (c_active_windings > c_peak_windings)
        c_peak_windings = v41;
    winding_t* v42 = (winding_t*)_Z_MallocInternal(12 * numpoints + 4);
    v42->numpoints = numpoints;
    *hull = v42;
    Com_Memcpy((char*)v42->p, (char*)hullPoints, 12 * numpoints);
}

// ea: 0x00609750
void WindingPlane(winding_t* w, float* const normal, float* dist)
{
    float v1[3];
    v1[0] = w->p[1][0] - w->p[0][0];
    v1[1] = w->p[1][1] - w->p[0][1];
    v1[2] = w->p[1][2] - w->p[0][2];
    float v2[3];
    v2[0] = w->p[2][0] - w->p[0][0];
    v2[1] = w->p[2][1] - w->p[0][1];
    v2[2] = w->p[2][2] - w->p[0][2];
    CrossProduct(v2, v1, normal);
    VectorNormalize2(normal, normal);
    *dist = ((normal[2] * w->p[0][2]) + (normal[1] * w->p[0][1]))
        + (w->p[0][0] * normal[0]);
}

// ea: 0x00609810
float WindingArea(winding_t* w)
{
    float total = 0.0f;
    if (w->numpoints > 2)
    {
        for (int v1 = 2; v1 < w->numpoints; ++v1)
        {
            float d1[3];
            d1[0] = w->p[v1 - 1][0] - w->p[0][0];
            d1[1] = w->p[v1 - 1][1] - w->p[0][1];
            d1[2] = w->p[v1 - 1][2] - w->p[0][2];
            float d2[3];
            d2[0] = w->p[v1][0] - w->p[0][0];
            d2[1] = w->p[v1][1] - w->p[0][1];
            d2[2] = w->p[v1][2] - w->p[0][2];
            float cross[3];
            CrossProduct(d1, d2, cross);
            total = sqrtf(cross[2] * cross[2] + cross[1] * cross[1]
                          + cross[0] * cross[0]) * 0.5f + total;
        }
    }
    return total;
}

// ea: 0x006098F0
void WindingBounds(winding_t* w, float* const mins, float* const maxs)
{
    mins[2] = 131072.0f;
    mins[1] = 131072.0f;
    mins[0] = 131072.0f;
    maxs[2] = -131072.0f;
    maxs[1] = -131072.0f;
    maxs[0] = -131072.0f;
    for (int v3 = 0; v3 < w->numpoints; ++v3)
    {
        float v5 = w->p[v3][0];
        if (mins[0] > v5)
            mins[0] = v5;
        if (v5 > maxs[0])
            maxs[0] = v5;
        float v6 = w->p[v3][1];
        if (mins[1] > v6)
            mins[1] = v6;
        if (v6 > maxs[1])
            maxs[1] = v6;
        float v7 = w->p[v3][2];
        if (mins[2] > v7)
            mins[2] = v7;
        if (v7 > maxs[2])
            maxs[2] = v7;
    }
}

// ea: 0x006099B0
void WindingCenter(winding_t* w, float* const center)
{
    center[0] = 0.0f;
    center[1] = 0.0f;
    center[2] = 0.0f;
    for (int v2 = 0; v2 < w->numpoints; ++v2)
    {
        center[0] = w->p[v2][0] + center[0];
        center[1] = w->p[v2][1] + center[1];
        center[2] = w->p[v2][2] + center[2];
    }
    float v4 = 1.0f / w->numpoints;
    center[0] = center[0] * v4;
    center[1] = center[1] * v4;
    center[2] = center[2] * v4;
}

// ea: 0x00609A60
winding_t* BaseWindingForPlane(float* const normal, float dist)
{
    int v3 = -1;
    float max = -131072.0f;
    float v = fabsf(normal[0]);
    if (v > -131072.0f)
    {
        v3 = 0;
        max = v;
    }
    float va = fabsf(normal[1]);
    if (va > max)
    {
        v3 = 1;
        max = va;
    }
    if (fabsf(normal[2]) <= max)
    {
        if (v3 == -1)
            Com_Error(ERR_DROP, "BaseWindingForPlane: no axis found");
    }
    else
    {
        v3 = 2;
    }
    float v4 = 0.0f;
    float v5 = 0.0f;
    if (v3 >= 0)
    {
        if (v3 <= 1)
            v5 = 1.0f;
        else
            v4 = 1.0f;
    }
    float vup[3];
    float v6 = 0.0f - (((normal[0] * v4) + (v5 * normal[2]))
                       + (0.0f * normal[1]));
    vup[0] = (v6 * normal[0]) + v4;
    vup[1] = (v6 * normal[1]) + 0.0f;
    vup[2] = (v6 * normal[2]) + v5;
    VectorNormalize2(vup, vup);
    float org = normal[0] * dist;
    float org_4 = dist * normal[1];
    float org_8 = dist * normal[2];
    float vright[3];
    CrossProduct(vup, normal, vright);
    vup[0] = vup[0] * 131072.0f;
    vup[1] = vup[1] * 131072.0f;
    vup[2] = vup[2] * 131072.0f;
    vright[0] = vright[0] * 131072.0f;
    vright[1] = vright[1] * 131072.0f;
    int v9 = c_active_windings + 1;
    bool v10 = c_active_windings + 1 <= c_peak_windings;
    vright[2] = vright[2] * 131072.0f;
    ++c_active_windings;
    if (!v10)
        c_peak_windings = v9;
    winding_t* result = (winding_t*)_Z_MallocInternal(52);
    result->p[0][0] = org - vright[0];
    result->p[0][1] = org_4 - vright[1];
    result->p[0][2] = org_8 - vright[2];
    result->p[0][0] = vup[0] + result->p[0][0];
    result->p[0][1] = result->p[0][1] + vup[1];
    result->p[0][2] = result->p[0][2] + vup[2];
    result->p[1][0] = vright[0] + org;
    result->p[1][1] = vright[1] + org_4;
    result->p[1][2] = vright[2] + org_8;
    result->p[1][0] = result->p[1][0] + vup[0];
    result->p[1][1] = result->p[1][1] + vup[1];
    result->p[1][2] = result->p[1][2] + vup[2];
    result->p[2][0] = vright[0] + org;
    result->p[2][1] = vright[1] + org_4;
    result->p[2][2] = vright[2] + org_8;
    result->p[2][0] = result->p[2][0] - vup[0];
    result->p[2][1] = result->p[2][1] - vup[1];
    result->p[2][2] = result->p[2][2] - vup[2];
    result->p[3][0] = org - vright[0];
    result->p[3][1] = org_4 - vright[1];
    result->p[3][2] = org_8 - vright[2];
    result->p[3][0] = result->p[3][0] - vup[0];
    result->p[3][1] = result->p[3][1] - vup[1];
    result->p[3][2] = result->p[3][2] - vup[2];
    result->numpoints = 4;
    return result;
}

// ea: 0x00609DA0
winding_t* CopyWinding(winding_t* w)
{
    int numpoints = w->numpoints;
    int v2 = ++c_active_windings;
    if (c_active_windings > c_peak_windings)
        c_peak_windings = v2;
    winding_t* v3 = (winding_t*)_Z_MallocInternal(12 * numpoints + 4);
    memcpy(v3, w, 12 * w->numpoints + 4);
    return v3;
}

// ea: 0x00609E00
winding_t* ReverseWinding(winding_t* w)
{
    int numpoints = w->numpoints;
    int v2 = ++c_active_windings;
    if (c_active_windings > c_peak_windings)
        c_peak_windings = v2;
    winding_t* result = (winding_t*)_Z_MallocInternal(12 * numpoints + 4);
    for (int v5 = 0; v5 < w->numpoints; ++v5)
    {
        result->p[v5][0] = w->p[w->numpoints - v5 - 1][0];
        result->p[v5][1] = w->p[w->numpoints - v5 - 1][1];
        result->p[v5][2] = w->p[w->numpoints - v5 - 1][2];
    }
    result->numpoints = w->numpoints;
    return result;
}

// ea: 0x00609E80
void ClipWindingEpsilon(winding_t* in, float* const normal, float dist,
                        float epsilon, winding_t** front, winding_t** back)
{
    int numpoints = in->numpoints;
    float dists[68];
    int sides[68];
    float v42 = 0.0f;
    float mid = 0.0f;
    int v43 = 0;
    int v8 = 0;
    if (in->numpoints > 0)
    {
        do
        {
            float v11 =
                (((in->p[v8][2] * normal[2]) + (in->p[v8][0] * normal[0]))
                 + (normal[1] * in->p[v8][1])) - dist;
            dists[v8] = v11;
            if (v11 <= epsilon)
            {
                if ((0.0f - epsilon) <= v11)
                    sides[v8] = 2;
                else
                    sides[v8] = 1;
            }
            else
            {
                sides[v8] = 0;
            }
            int v12 = sides[v8];
            ++v8;
            if (v12 == 0)
                ++v42;
            else if (v12 == 1)
                ++v43;
            else
                ++mid;
        } while (v8 < numpoints);
    }
    sides[v8] = sides[0];
    dists[v8] = dists[0];
    *back = nullptr;
    *front = nullptr;
    if (mid == 0.0f)
    {
        int v15 = in->numpoints;
        int v16 = ++c_active_windings;
        if (c_active_windings > c_peak_windings)
            c_peak_windings = v16;
        winding_t* v17 = (winding_t*)_Z_MallocInternal(12 * v15 + 4);
        memcpy(v17, in, 12 * in->numpoints + 4);
        *back = v17;
        return;
    }
    if (v42 == 0.0f)
    {
        *front = CopyWinding(in);
        return;
    }
    int v18 = in->numpoints + 4;
    int v19 = c_active_windings + 1;
    bool v9 = c_active_windings + 1 <= c_peak_windings;
    int maxpts = v18;
    ++c_active_windings;
    if (!v9)
        c_peak_windings = v19;
    int v20 = 12 * v18 + 4;
    winding_t* v21 = (winding_t*)_Z_MallocInternal(v20);
    *front = v21;
    int v22 = ++c_active_windings;
    if (c_active_windings > c_peak_windings)
        c_peak_windings = v22;
    winding_t* v23 = (winding_t*)_Z_MallocInternal(v20);
    *back = v23;
    for (int i = 0; i < in->numpoints; ++i)
    {
        int v25 = sides[i];
        if (v25 == 2)
        {
            v21->p[v21->numpoints][0] = in->p[i][0];
            v21->p[v21->numpoints][1] = in->p[i][1];
            v21->p[v21->numpoints][2] = in->p[i][2];
            ++v21->numpoints;
            v23->p[v23->numpoints][0] = in->p[i][0];
            v23->p[v23->numpoints][1] = in->p[i][1];
            v23->p[v23->numpoints][2] = in->p[i][2];
            ++v23->numpoints;
        }
        else
        {
            if (v25 == 0)
            {
                v21->p[v21->numpoints][0] = in->p[i][0];
                v21->p[v21->numpoints][1] = in->p[i][1];
                v21->p[v21->numpoints][2] = in->p[i][2];
                ++v21->numpoints;
            }
            if (sides[i] == 1)
            {
                v23->p[v23->numpoints][0] = in->p[i][0];
                v23->p[v23->numpoints][1] = in->p[i][1];
                v23->p[v23->numpoints][2] = in->p[i][2];
                ++v23->numpoints;
            }
            int v26 = sides[i + 1];
            if (v26 == 2 || v26 == sides[i])
                continue;
            float v27 = dists[i] - dists[i + 1];
            float v28 = dists[i];
            float v30 = v28 / v27;
            float midp[3];
            for (int c = 0; c < 3; ++c)
            {
                if (normal[c] == 1.0f)
                    midp[c] = dist;
                else if (normal[c] == -1.0f)
                    midp[c] = 0.0f - dist;
                else
                    midp[c] = ((in->p[(i + 1) % in->numpoints][c]
                                - in->p[i][c]) * v30) + in->p[i][c];
            }
            v21->p[v21->numpoints][0] = midp[0];
            v21->p[v21->numpoints][1] = midp[1];
            v21->p[v21->numpoints][2] = midp[2];
            ++v21->numpoints;
            v23->p[v23->numpoints][0] = midp[0];
            v23->p[v23->numpoints][1] = midp[1];
            v23->p[v23->numpoints][2] = midp[2];
            ++v23->numpoints;
        }
    }
    if (v21->numpoints > maxpts || v23->numpoints > maxpts)
        Com_Error(ERR_DROP, "ClipWinding: points exceeded estimate");
    if (v21->numpoints > 64 || v23->numpoints > 64)
        Com_Error(ERR_DROP, "ClipWinding: MAX_POINTS_ON_WINDING");
}

// ea: 0x00609550
void pw(winding_t* w)
{
    for (int v1 = 0; v1 < w->numpoints; ++v1)
    {
        printf("(%5.1f, %5.1f, %5.1f)\n", w->p[v1][0], w->p[v1][1],
               w->p[v1][2]);
    }
}

// ea: 0x0060A2A0
void ChopWindingInPlace(winding_t** inout, float* const normal, float dist,
                        float epsilon)
{
    winding_t* v4 = *inout;
    int numpoints = (*inout)->numpoints;
    winding_t* in = *inout;
    float dists[68];
    int sides[68];
    float mid[3];
    memset(mid, 0, sizeof(mid));
    int v6 = 0;
    if (numpoints > 0)
    {
        do
        {
            float v8 = (((in->p[v6][2] * normal[2])
                         + (in->p[v6][0] * normal[0]))
                        + (in->p[v6][1] * normal[1])) - dist;
            dists[v6] = v8;
            if (v8 <= epsilon)
            {
                if ((0.0f - epsilon) <= v8)
                    sides[v6] = 2;
                else
                    sides[v6] = 1;
            }
            else
            {
                sides[v6] = 0;
            }
            ++*(int*)&mid[sides[v6]];
            ++v6;
        } while (v6 < numpoints);
    }
    float v9 = dists[0];
    sides[v6] = sides[0];
    dists[v6] = v9;
    if (*(int*)&mid[0] == 0)
    {
        if (numpoints == -559030611)
            Com_Error(ERR_FATAL, "FreeWinding: freed a freed winding");
        v4->numpoints = -559030611;
        --c_active_windings;
        _Z_FreeInternal(v4);
        *inout = nullptr;
        return;
    }
    if (*(int*)&mid[1] == 0)
        return;
    int v10 = c_active_windings + 1;
    bool v11 = c_active_windings + 1 <= c_peak_windings;
    int maxpts = numpoints + 4;
    ++c_active_windings;
    if (!v11)
        c_peak_windings = v10;
    winding_t* v12 = (winding_t*)_Z_MallocInternal(12 * (numpoints + 4) + 4);
    int v13 = 0;
    if (v4->numpoints > 0)
    {
        while (1)
        {
            int v15 = sides[v13];
            if (v15 == 2)
            {
                v12->p[v12->numpoints][0] = in->p[v13][0];
                v12->p[v12->numpoints][1] = in->p[v13][1];
                v12->p[v12->numpoints][2] = in->p[v13][2];
                ++v12->numpoints;
            }
            else
            {
                if (v15 == 0)
                {
                    v12->p[v12->numpoints][0] = in->p[v13][0];
                    v12->p[v12->numpoints][1] = in->p[v13][1];
                    v12->p[v12->numpoints][2] = in->p[v13][2];
                    ++v12->numpoints;
                }
                int v16 = sides[v13 + 1];
                if (v16 == 2 || v16 == v15)
                    goto LABEL_33;
                float v18 = dists[v13] / (dists[v13] - dists[v13 + 1]);
                int v17 = (v13 + 1) % in->numpoints;
                for (int c = 0; c < 3; ++c)
                {
                    if (normal[c] == 1.0f)
                        mid[c] = dist;
                    else if (normal[c] == -1.0f)
                        mid[c] = 0.0f - dist;
                    else
                        mid[c] = ((in->p[v17][c] - in->p[v13][c]) * v18)
                            + in->p[v13][c];
                }
                v12->p[v12->numpoints][0] = mid[0];
                v12->p[v12->numpoints][1] = mid[1];
                v12->p[v12->numpoints][2] = mid[2];
                ++v12->numpoints;
            }
        LABEL_33:
            ++v13;
            if (v13 >= in->numpoints)
            {
                v4 = in;
                break;
            }
        }
    }
    if (v12->numpoints > maxpts)
        Com_Error(ERR_DROP, "ClipWinding: points exceeded estimate");
    if (v12->numpoints > 64)
        Com_Error(ERR_DROP, "ClipWinding: MAX_POINTS_ON_WINDING");
    if (v4->numpoints == -559030611)
        Com_Error(ERR_FATAL, "FreeWinding: freed a freed winding");
    v4->numpoints = -559030611;
    --c_active_windings;
    _Z_FreeInternal(v4);
    *inout = v12;
}

// ea: 0x0060A610
winding_t* ChopWinding(winding_t* in, float* const normal, float dist)
{
    winding_t* v3 = in;
    winding_t* b = nullptr;
    ClipWindingEpsilon(in, normal, dist, 0.1f, &in, &b);
    if (v3->numpoints == -559030611)
        Com_Error(ERR_FATAL, "FreeWinding: freed a freed winding");
    v3->numpoints = -559030611;
    --c_active_windings;
    _Z_FreeInternal(v3);
    winding_t* v4 = b;
    if (b != nullptr)
    {
        if (b->numpoints == -559030611)
            Com_Error(ERR_FATAL, "FreeWinding: freed a freed winding");
        v4->numpoints = -559030611;
        --c_active_windings;
        _Z_FreeInternal(v4);
    }
    return in;
}

// ea: 0x0060A6B0
void CheckWinding(winding_t* w)
{
    if (w->numpoints < 3)
        Com_Error(ERR_DROP, "CheckWinding: %i points", w->numpoints);
    float st7_3 = WindingArea(w);
    if (st7_3 < 1.0f)
        Com_Error(ERR_DROP, "CheckWinding: %f area", st7_3);
    float v2[3];
    v2[0] = w->p[1][0] - w->p[0][0];
    v2[1] = w->p[1][1] - w->p[0][1];
    v2[2] = w->p[1][2] - w->p[0][2];
    float v1[3];
    v1[0] = w->p[2][0] - w->p[0][0];
    v1[1] = w->p[2][1] - w->p[0][1];
    v1[2] = w->p[2][2] - w->p[0][2];
    float facenormal[3];
    CrossProduct(v1, v2, facenormal);
    VectorNormalize2(facenormal, facenormal);
    float facedist = ((w->p[0][0] * facenormal[0])
                      + (w->p[0][1] * facenormal[1]))
        + (facenormal[2] * w->p[0][2]);
    for (int i = 0; i < w->numpoints; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (w->p[i][j] > 131072.0f || w->p[i][j] < -131072.0f)
                Com_Error(ERR_DROP, "CheckFace: BUGUS_RANGE: %f", w->p[i][j]);
        }
        float v6 = (((w->p[i][2] * facenormal[2])
                     + (w->p[i][1] * facenormal[1]))
                    + (w->p[i][0] * facenormal[0])) - facedist;
        int v7 = i + 1 == w->numpoints ? 0 : i + 1;
        if (v6 < -0.1f || v6 > 0.1f)
            Com_Error(ERR_DROP, "CheckWinding: point off plane");
        float dir[3];
        dir[0] = w->p[v7][0] - w->p[i][0];
        dir[1] = w->p[v7][1] - w->p[i][1];
        dir[2] = w->p[v7][2] - w->p[i][2];
        if (sqrtf(dir[2] * dir[2] + dir[1] * dir[1] + dir[0] * dir[0]) < 0.1f)
            Com_Error(ERR_DROP, "CheckWinding: degenerate edge");
        float edgenormal[3];
        CrossProduct(facenormal, dir, edgenormal);
        VectorNormalize2(edgenormal, edgenormal);
        float v9 = (((w->p[i][2] * edgenormal[2])
                     + (w->p[i][1] * edgenormal[1]))
                    + (w->p[i][0] * edgenormal[0])) + 0.1f;
        for (int v10 = 0; v10 < w->numpoints; ++v10)
        {
            if (v10 != i
                && (((w->p[v10][2] * edgenormal[2])
                     + (w->p[v10][0] * edgenormal[0]))
                    + (w->p[v10][1] * edgenormal[1])) > v9)
                Com_Error(ERR_DROP, "CheckWinding: non-convex");
        }
    }
}

// ea: 0x0060A980
int WindingOnPlaneSide(winding_t* w, float* const normal, float dist)
{
    int v3 = 0;
    int v4 = 0;
    for (int v5 = 0; v5 < w->numpoints; ++v5)
    {
        float v7 = (((w->p[v5][0] * normal[0]) + (w->p[v5][2] * normal[2]))
                    + (w->p[v5][1] * normal[1])) - dist;
        if (v7 >= -0.1f)
        {
            if (v7 > 0.1f)
            {
                if (v4 != 0)
                    return -2;
                v3 = 1;
            }
        }
        else
        {
            if (v3 != 0)
                return -2;
            v4 = 1;
        }
    }
    if (v4 != 0)
        return 1;
    if (v3 != 0)
        return 0;
    return 2;
}

// ============================================================================
// Collision math helpers (cdl / cm_trace helpers)
// ============================================================================
float thresh2;  // ?thresh2@@3MA (game.o)
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);  // tl_xboxr

// ea: 0x0060C020
bool is_plane_ok(const math::Position3& hitp, const math::Dir3& hitn,
                 float hitoffs, const math::Dir3& n,
                 float offs, float radius)
{
    float dnd = (hitn.v.m128_f32[0] - n.v.m128_f32[0])
            * (hitn.v.m128_f32[0] - n.v.m128_f32[0])
        + (hitn.v.m128_f32[1] - n.v.m128_f32[1])
            * (hitn.v.m128_f32[1] - n.v.m128_f32[1])
        + (hitn.v.m128_f32[2] - n.v.m128_f32[2])
            * (hitn.v.m128_f32[2] - n.v.m128_f32[2]);
    if (thresh2 > dnd
        && thresh2 > ((float)(int)hitoffs - (float)(int)offs)
            * ((float)(int)hitoffs - (float)(int)offs))
        return true;
    float dotp = (hitp.v.m128_f32[0] * n.v.m128_f32[0])
        + (hitp.v.m128_f32[1] * n.v.m128_f32[1])
        + (hitp.v.m128_f32[2] * n.v.m128_f32[2]);
    if (fabsf(dotp - (float)(int)offs) > radius)
        return true;
    float cross[3];
    cross[0] = hitn.v.m128_f32[1] * n.v.m128_f32[2]
        - hitn.v.m128_f32[2] * n.v.m128_f32[1];
    cross[1] = hitn.v.m128_f32[2] * n.v.m128_f32[0]
        - hitn.v.m128_f32[0] * n.v.m128_f32[2];
    cross[2] = hitn.v.m128_f32[0] * n.v.m128_f32[1]
        - hitn.v.m128_f32[1] * n.v.m128_f32[0];
    float v20 = cross[0] * cross[0] + cross[1] * cross[1]
        + cross[2] * cross[2];
    if (thresh2 > v20)
        return true;
    // Project hitp onto the line of intersection of the two planes.
    float t = ((hitoffs * n.v.m128_f32[0] - offs * hitn.v.m128_f32[0])
                   * cross[0]
               + (hitoffs * n.v.m128_f32[1] - offs * hitn.v.m128_f32[1])
                   * cross[1]
               + (hitoffs * n.v.m128_f32[2] - offs * hitn.v.m128_f32[2])
                   * cross[2])
        / v20;
    float proj[3];
    proj[0] = hitp.v.m128_f32[0] - t * cross[0];
    proj[1] = hitp.v.m128_f32[1] - t * cross[1];
    proj[2] = hitp.v.m128_f32[2] - t * cross[2];
    float d2 = proj[0] * proj[0] + proj[1] * proj[1] + proj[2] * proj[2];
    float along = proj[0] * cross[0] + proj[1] * cross[1]
        + proj[2] * cross[2];
    return (d2 - (along / v20) * (along / v20)) > (radius * radius);
}

// ea: 0x0060C250
void RotatePoint(math::Position3& point, math::Position3* const matrix)
{
    math::Position3 v5;
    v5.v.m128_f32[0] = matrix[0].v.m128_f32[0] * point.v.m128_f32[0]
        + matrix[0].v.m128_f32[1] * point.v.m128_f32[1]
        + matrix[0].v.m128_f32[2] * point.v.m128_f32[2];
    v5.v.m128_f32[1] = matrix[1].v.m128_f32[0] * point.v.m128_f32[0]
        + matrix[1].v.m128_f32[1] * point.v.m128_f32[1]
        + matrix[1].v.m128_f32[2] * point.v.m128_f32[2];
    v5.v.m128_f32[2] = matrix[2].v.m128_f32[0] * point.v.m128_f32[0]
        + matrix[2].v.m128_f32[1] * point.v.m128_f32[1]
        + matrix[2].v.m128_f32[2] * point.v.m128_f32[2];
    point.v = v5.v;
}

// ea: 0x0060C330
void TransposeMatrix(math::Position3* const matrix,
                     math::Position3* const transpose)
{
    for (int i = 0; i < 3; ++i)
    {
        transpose[i].v.m128_f32[0] = matrix[0].v.m128_f32[i];
        transpose[i].v.m128_f32[1] = matrix[1].v.m128_f32[i];
        transpose[i].v.m128_f32[2] = matrix[2].v.m128_f32[i];
    }
}

// ea: 0x0060C370
void CreateRotationMatrix(const math::Position3& angles,
                          math::Position3* const matrix)
{
    float mat[3][3];
    AngleVectors(angles, mat[0], mat[1], mat[2]);
    VectorInverse(mat[1]);
    matrix[0].v.m128_f32[0] = mat[0][0];
    matrix[0].v.m128_f32[1] = mat[0][1];
    matrix[0].v.m128_f32[2] = mat[0][2];
    memcpy(&matrix[1], mat[1], 12);
    memcpy(&matrix[2], mat[2], 12);
}

// ea: 0x0060C400
math::Vector4 calc_normal(const math::Position3& v0,
                          const math::Position3& v1,
                          const math::Position3& v2)
{
    math::Vector4 result;
    float e1[3], e2[3], n[3];
    e1[0] = v2.v.m128_f32[0] - v0.v.m128_f32[0];
    e1[1] = v2.v.m128_f32[1] - v0.v.m128_f32[1];
    e1[2] = v2.v.m128_f32[2] - v0.v.m128_f32[2];
    e2[0] = v1.v.m128_f32[0] - v0.v.m128_f32[0];
    e2[1] = v1.v.m128_f32[1] - v0.v.m128_f32[1];
    e2[2] = v1.v.m128_f32[2] - v0.v.m128_f32[2];
    n[0] = e1[1] * e2[2] - e1[2] * e2[1];
    n[1] = e1[2] * e2[0] - e1[0] * e2[2];
    n[2] = e1[0] * e2[1] - e1[1] * e2[0];
    float len2 = n[0] * n[0] + n[1] * n[1] + n[2] * n[2];
    float d = 0.0f;
    if (len2 <= 0.0000001f)
    {
        result.v.m128_f32[0] = 0.0f;
        result.v.m128_f32[1] = 0.0f;
        result.v.m128_f32[2] = 0.0f;
        result.v.m128_f32[3] = 0.0f;
        return result;
    }
    float inv = 1.0f / sqrtf(len2);
    result.v.m128_f32[0] = n[0] * inv;
    result.v.m128_f32[1] = n[1] * inv;
    result.v.m128_f32[2] = n[2] * inv;
    d = 0.0f - (result.v.m128_f32[0] * v0.v.m128_f32[0]
                 + result.v.m128_f32[1] * v0.v.m128_f32[1]
                 + result.v.m128_f32[2] * v0.v.m128_f32[2]);
    result.v.m128_f32[3] = d;
    return result;
}

// ea: 0x0060E300
float point_to_segment_dist2(const math::Position3& c,
                             const math::Position3& a,
                             const math::Position3& b)
{
    float v3[3], v4[3], v5[3];
    v3[0] = c.v.m128_f32[0] - a.v.m128_f32[0];
    v3[1] = c.v.m128_f32[1] - a.v.m128_f32[1];
    v3[2] = c.v.m128_f32[2] - a.v.m128_f32[2];
    v4[0] = c.v.m128_f32[0] - b.v.m128_f32[0];
    v4[1] = c.v.m128_f32[1] - b.v.m128_f32[1];
    v4[2] = c.v.m128_f32[2] - b.v.m128_f32[2];
    v5[0] = b.v.m128_f32[0] - a.v.m128_f32[0];
    v5[1] = b.v.m128_f32[1] - a.v.m128_f32[1];
    v5[2] = b.v.m128_f32[2] - a.v.m128_f32[2];
    float v13 = v3[0] * v5[0] + v3[1] * v5[1] + v3[2] * v5[2];
    if (v13 > 0.0f)
    {
        float v12 = v5[0] * v5[0] + v5[1] * v5[1] + v5[2] * v5[2];
        if (v13 < v12)
            return (v3[0] * v3[0] + v3[1] * v3[1] + v3[2] * v3[2])
                - v13 / v12 * v13;
        return v4[0] * v4[0] + v4[1] * v4[1] + v4[2] * v4[2];
    }
    return v3[0] * v3[0] + v3[1] * v3[1] + v3[2] * v3[2];
}

// ea: 0x0060D2B0
bool collide_ray_triangle(const math::Position3& p0, const math::Dir3& u0,
                          const math::Position3& v0,
                          const math::Position3& v1,
                          const math::Position3& v2, float cur_t, float* t)
{
    if ((cur_t < 0.0f || cur_t > 1.0f)
        && _tlAssert("c:\\cod\\code\\game\\CollisionMgr.cpp", 3792,
                     "cur_t >= 0.0f && cur_t <= 1.0f", ""))
        __debugbreak();
    float e1[3], e2[3];
    e1[0] = v1.v.m128_f32[0] - v0.v.m128_f32[0];
    e1[1] = v1.v.m128_f32[1] - v0.v.m128_f32[1];
    e1[2] = v1.v.m128_f32[2] - v0.v.m128_f32[2];
    e2[0] = v2.v.m128_f32[0] - v0.v.m128_f32[0];
    e2[1] = v2.v.m128_f32[1] - v0.v.m128_f32[1];
    e2[2] = v2.v.m128_f32[2] - v0.v.m128_f32[2];
    float du0[3] = { -u0.v.m128_f32[0], -u0.v.m128_f32[1],
                     -u0.v.m128_f32[2] };
    float cross[3];
    cross[0] = e1[1] * e2[2] - e1[2] * e2[1];
    cross[1] = e1[2] * e2[0] - e1[0] * e2[2];
    cross[2] = e1[0] * e2[1] - e1[1] * e2[0];
    float det = cross[0] * du0[0] + cross[1] * du0[1] + cross[2] * du0[2];
    if (fabsf(det) < 0.000099999997f)
        return false;
    float s[3];
    s[0] = p0.v.m128_f32[0] - v0.v.m128_f32[0];
    s[1] = p0.v.m128_f32[1] - v0.v.m128_f32[1];
    s[2] = p0.v.m128_f32[2] - v0.v.m128_f32[2];
    float inv = 1.0f / det;
    float tval = inv * (cross[0] * s[0] + cross[1] * s[1] + cross[2] * s[2]);
    if (tval < 0.0f || tval > cur_t)
        return false;
    float q[3];
    q[0] = s[1] * du0[2] - s[2] * du0[1];
    q[1] = s[2] * du0[0] - s[0] * du0[2];
    q[2] = s[0] * du0[1] - s[1] * du0[0];
    float u = 0.0f - inv * (q[0] * e2[0] + q[1] * e2[1] + q[2] * e2[2]);
    if (u < 0.0f)
        return false;
    float v = inv * (q[0] * e1[0] + q[1] * e1[1] + q[2] * e1[2]);
    if (v < 0.0f || (v + u) > 1.0f)
        return false;
    *t = tval;
    return true;
}

// ea: 0x0060D4A0
bool xtest_sphere_triangle(const math::Position3& sphere_center,
                           float sphere_radius,
                           const math::Position3& v0,
                           const math::Position3& v1,
                           const math::Position3& v2,
                           const math::Dir3& normal)
{
    float c[3] = { sphere_center.v.m128_f32[0],
                   sphere_center.v.m128_f32[1],
                   sphere_center.v.m128_f32[2] };
    float r2 = sphere_radius * sphere_radius;
    float n[3] = { normal.v.m128_f32[0], normal.v.m128_f32[1],
                   normal.v.m128_f32[2] };
    float v0c[3] = { v0.v.m128_f32[0] - c[0], v0.v.m128_f32[1] - c[1],
                     v0.v.m128_f32[2] - c[2] };
    float v1c[3] = { v1.v.m128_f32[0] - c[0], v1.v.m128_f32[1] - c[1],
                     v1.v.m128_f32[2] - c[2] };
    float v2c[3] = { v2.v.m128_f32[0] - c[0], v2.v.m128_f32[1] - c[1],
                     v2.v.m128_f32[2] - c[2] };
    float d = v0c[0] * n[0] + v0c[1] * n[1] + v0c[2] * n[2];
    if (d > sphere_radius || (0.0f - sphere_radius) > d)
        return false;
    float c0 = v0c[0] * v0c[0] + v0c[1] * v0c[1] + v0c[2] * v0c[2];
    float c1 = v0c[0] * v1c[0] + v0c[1] * v1c[1] + v0c[2] * v1c[2];
    float c2 = v0c[0] * v2c[0] + v0c[1] * v2c[1] + v0c[2] * v2c[2];
    float v43 = v1c[0] * v1c[0] + v1c[1] * v1c[1] + v1c[2] * v1c[2];
    float cc = v1c[0] * v2c[0] + v1c[1] * v2c[1] + v1c[2] * v2c[2];
    float v40 = v2c[0] * v2c[0] + v2c[1] * v2c[1] + v2c[2] * v2c[2];
    if (c1 >= c0 && c2 >= c0)
        return r2 >= c0;
    if (c1 >= v43 && cc >= v43)
        return r2 >= v43;
    if (c2 >= v40 && cc >= v40)
        return r2 >= v40;
    float w[3] = { 0.0f, 0.0f, 0.0f };
    if (v43 >= c1 && c0 >= c1)
    {
        if ((cc * (c0 - c1) + c2 * (v43 - c1)) >= (v43 * c0 - c1 * c1))
        {
            float t1 = c0 - c1;
            float t2 = v43 - c1;
            float inv = 1.0f / (t1 + t2);
            for (int i = 0; i < 3; ++i)
                w[i] = v0c[i] * (inv * t1) + v1c[i] * (inv * t2);
            return r2 >= (w[0] * w[0] + w[1] * w[1] + w[2] * w[2]);
        }
    }
    if (v40 >= c2 && c0 >= c2)
    {
        if ((cc * (c0 - c2) + c1 * (v40 - c2)) >= (v40 * c0 - c2 * c2))
        {
            float t1 = c0 - c2;
            float t2 = v40 - c2;
            float inv = 1.0f / (t1 + t2);
            for (int i = 0; i < 3; ++i)
                w[i] = v0c[i] * (inv * t1) + v2c[i] * (inv * t2);
            return r2 >= (w[0] * w[0] + w[1] * w[1] + w[2] * w[2]);
        }
    }
    if (v40 >= cc && v43 >= cc)
    {
        if ((c2 * (v43 - cc) + c1 * (v40 - cc)) >= (v40 * v43 - cc * cc))
        {
            float t1 = v40 - cc;
            float t2 = v43 - cc;
            float inv = 1.0f / (t1 + t2);
            for (int i = 0; i < 3; ++i)
                w[i] = v1c[i] * (inv * t1) + v2c[i] * (inv * t2);
            return r2 >= (w[0] * w[0] + w[1] * w[1] + w[2] * w[2]);
        }
    }
    return true;
}

// ea: 0x0060D860
bool new_push_out_sphere_triangle(const math::Position3& sphere_center,
                                  float sphere_radius,
                                  const math::Position3& v0,
                                  const math::Position3& v1,
                                  const math::Position3& v2,
                                  const math::Dir3& normal,
                                  math::Position3& new_sphere_center)
{
    float c[3] = { sphere_center.v.m128_f32[0],
                   sphere_center.v.m128_f32[1],
                   sphere_center.v.m128_f32[2] };
    float n[3] = { normal.v.m128_f32[0], normal.v.m128_f32[1],
                   normal.v.m128_f32[2] };
    float v0c[3] = { v0.v.m128_f32[0] - c[0], v0.v.m128_f32[1] - c[1],
                     v0.v.m128_f32[2] - c[2] };
    float v1c[3] = { v1.v.m128_f32[0] - c[0], v1.v.m128_f32[1] - c[1],
                     v1.v.m128_f32[2] - c[2] };
    float v2c[3] = { v2.v.m128_f32[0] - c[0], v2.v.m128_f32[1] - c[1],
                     v2.v.m128_f32[2] - c[2] };
    float d = v0c[0] * n[0] + v0c[1] * n[1] + v0c[2] * n[2];
    if (d > sphere_radius || (0.0f - sphere_radius) > d)
        return false;
    float r2 = sphere_radius * sphere_radius;
    float nhitn_sq = v0c[0] * v0c[0] + v0c[1] * v0c[1] + v0c[2] * v0c[2];
    float hitn_12 = v0c[0] * v1c[0] + v0c[1] * v1c[1] + v0c[2] * v1c[2];
    float v41 = v0c[0] * v2c[0] + v0c[1] * v2c[1] + v0c[2] * v2c[2];
    float v46 = v1c[0] * v1c[0] + v1c[1] * v1c[1] + v1c[2] * v1c[2];
    float c0 = v1c[0] * v2c[0] + v1c[1] * v2c[1] + v1c[2] * v2c[2];
    float v42 = v2c[0] * v2c[0] + v2c[1] * v2c[1] + v2c[2] * v2c[2];
    float best[3] = { v0c[0], v0c[1], v0c[2] };
    float bestsq = nhitn_sq;
    if (!(hitn_12 >= nhitn_sq && v41 >= nhitn_sq))
    {
        if (hitn_12 >= v46 && c0 >= v46)
        {
            best[0] = v1c[0]; best[1] = v1c[1]; best[2] = v1c[2];
            bestsq = v46;
        }
        else if (v41 >= v42 && c0 >= v42)
        {
            best[0] = v2c[0]; best[1] = v2c[1]; best[2] = v2c[2];
            bestsq = v42;
        }
        else
        {
            float w[3];
            if (v46 >= hitn_12 && nhitn_sq >= hitn_12)
            {
                if ((c0 * (nhitn_sq - hitn_12)
                     + v41 * (v46 - hitn_12))
                    >= (v46 * nhitn_sq - hitn_12 * hitn_12))
                {
                    float t1 = nhitn_sq - hitn_12;
                    float t2 = v46 - hitn_12;
                    float inv = 1.0f / (t1 + t2);
                    for (int i = 0; i < 3; ++i)
                        w[i] = v0c[i] * (inv * t1) + v1c[i] * (inv * t2);
                    bestsq = w[0] * w[0] + w[1] * w[1] + w[2] * w[2];
                    if (bestsq <= 0.000099999997f || r2 < bestsq || d >= 0.0f)
                        return false;
                    float len = sqrtf(bestsq);
                    float k = (sphere_radius - len + 0.001f) / len;
                    new_sphere_center.v.m128_f32[0] = c[0] - v0c[0] * k;
                    new_sphere_center.v.m128_f32[1] = c[1] - v0c[1] * k;
                    new_sphere_center.v.m128_f32[2] = c[2] - v0c[2] * k;
                    return true;
                }
            }
            if (v42 >= v41 && nhitn_sq >= v41)
            {
                if ((c0 * (nhitn_sq - v41) + hitn_12 * (v42 - v41))
                    >= (v42 * nhitn_sq - v41 * v41))
                {
                    float t1 = nhitn_sq - v41;
                    float t2 = v42 - v41;
                    float inv = 1.0f / (t1 + t2);
                    for (int i = 0; i < 3; ++i)
                        w[i] = v0c[i] * (inv * t1) + v2c[i] * (inv * t2);
                    bestsq = w[0] * w[0] + w[1] * w[1] + w[2] * w[2];
                    if (bestsq <= 0.000099999997f || r2 < bestsq || d >= 0.0f)
                        return false;
                    float len = sqrtf(bestsq);
                    float k = (sphere_radius - len + 0.001f) / len;
                    new_sphere_center.v.m128_f32[0] = c[0] - v0c[0] * k;
                    new_sphere_center.v.m128_f32[1] = c[1] - v0c[1] * k;
                    new_sphere_center.v.m128_f32[2] = c[2] - v0c[2] * k;
                    return true;
                }
            }
            if (v42 >= c0 && v46 >= c0)
            {
                if ((v41 * (v46 - c0) + hitn_12 * (v42 - c0))
                    >= (v42 * v46 - c0 * c0))
                {
                    float t1 = v42 - c0;
                    float t2 = v46 - c0;
                    float inv = 1.0f / (t1 + t2);
                    for (int i = 0; i < 3; ++i)
                        w[i] = v1c[i] * (inv * t1) + v2c[i] * (inv * t2);
                    bestsq = w[0] * w[0] + w[1] * w[1] + w[2] * w[2];
                    if (bestsq <= 0.000099999997f || r2 < bestsq || d >= 0.0f)
                        return false;
                    float len = sqrtf(bestsq);
                    float k = (sphere_radius - len + 0.001f) / len;
                    new_sphere_center.v.m128_f32[0] = c[0] - v1c[0] * k;
                    new_sphere_center.v.m128_f32[1] = c[1] - v1c[1] * k;
                    new_sphere_center.v.m128_f32[2] = c[2] - v1c[2] * k;
                    return true;
                }
            }
            // Push out along the face normal.
            float k = (d + sphere_radius) + 0.001f;
            new_sphere_center.v.m128_f32[0] = c[0] + n[0] * k;
            new_sphere_center.v.m128_f32[1] = c[1] + n[1] * k;
            new_sphere_center.v.m128_f32[2] = c[2] + n[2] * k;
            return true;
        }
    }
    if (bestsq > 0.000099999997f && r2 >= bestsq && d < 0.0f)
    {
        float len = sqrtf(bestsq);
        float k = (sphere_radius - len + 0.001f) / len;
        new_sphere_center.v.m128_f32[0] = c[0] - best[0] * k;
        new_sphere_center.v.m128_f32[1] = c[1] - best[1] * k;
        new_sphere_center.v.m128_f32[2] = c[2] - best[2] * k;
        return true;
    }
    return false;
}

// ea: 0x0060DE10
int trace_point_through_sphere(const math::Position3& p, const math::Dir3& ud,
                               const math::Position3& ctr, float r, float& t,
                               math::Position3& q)
{
    float v6[3];
    v6[0] = p.v.m128_f32[0] - ctr.v.m128_f32[0];
    v6[1] = p.v.m128_f32[1] - ctr.v.m128_f32[1];
    v6[2] = p.v.m128_f32[2] - ctr.v.m128_f32[2];
    float v13 = v6[0] * ud.v.m128_f32[0] + v6[1] * ud.v.m128_f32[1]
        + v6[2] * ud.v.m128_f32[2];
    float v9 = (v6[0] * v6[0] + v6[1] * v6[1] + v6[2] * v6[2]) - (r * r);
    if (v9 > 0.0f && v13 > 0.0f)
        return 0;
    float v12 = (v13 * v13) - v9;
    if (v12 < 0.0f)
        return 0;
    float v11 = -v13 - sqrtf(v12);
    t = v11;
    if (v11 < 0.0f)
        t = 0.0f;
    q.v.m128_f32[0] = p.v.m128_f32[0] + ud.v.m128_f32[0] * t;
    q.v.m128_f32[1] = p.v.m128_f32[1] + ud.v.m128_f32[1] * t;
    q.v.m128_f32[2] = p.v.m128_f32[2] + ud.v.m128_f32[2] * t;
    return 1;
}

// ea: 0x0060DF10
bool trace_sphere_through_sphere(const math::Position3& c0, float r0,
                                 const math::Position3& c1, float r1,
                                 const math::Dir3& v0, float& t)
{
    float v14 = sqrtf(v0.v.m128_f32[0] * v0.v.m128_f32[0]
                      + v0.v.m128_f32[1] * v0.v.m128_f32[1]
                      + v0.v.m128_f32[2] * v0.v.m128_f32[2]);
    if (v14 <= 0.001f)
        return false;
    math::Dir3 q_4;
    q_4.v.m128_f32[0] = v0.v.m128_f32[0] / v14;
    q_4.v.m128_f32[1] = v0.v.m128_f32[1] / v14;
    q_4.v.m128_f32[2] = v0.v.m128_f32[2] / v14;
    math::Position3 q;
    return trace_point_through_sphere(c0, q_4, c1, r0 + r1, t, q) != 0
        && v14 >= t;
}

// ea: 0x0060DFE0
int trace_point_through_cylinder(const math::Position3& sa,
                                 const math::Position3& sb,
                                 const math::Position3& p,
                                 const math::Position3& q, float r, float* t)
{
    float v6[3] = { q.v.m128_f32[0] - p.v.m128_f32[0],
                    q.v.m128_f32[1] - p.v.m128_f32[1],
                    q.v.m128_f32[2] - p.v.m128_f32[2] };
    float v7[3] = { sa.v.m128_f32[0] - p.v.m128_f32[0],
                    sa.v.m128_f32[1] - p.v.m128_f32[1],
                    sa.v.m128_f32[2] - p.v.m128_f32[2] };
    float v8[3] = { sb.v.m128_f32[0] - sa.v.m128_f32[0],
                    sb.v.m128_f32[1] - sa.v.m128_f32[1],
                    sb.v.m128_f32[2] - sa.v.m128_f32[2] };
    float v27 = v7[0] * v6[0] + v7[1] * v6[1] + v7[2] * v6[2];
    float a = v8[0] * v6[0] + v8[1] * v6[1] + v8[2] * v6[2];
    float v24 = v6[0] * v6[0] + v6[1] * v6[1] + v6[2] * v6[2];
    float v12 = v27;
    if (v27 >= 0.0f || (a + v27) >= 0.0f)
    {
        float v13 = v24;
        if (v27 <= v24 || (a + v27) <= v24)
        {
            float b = v8[0] * v8[0] + v8[1] * v8[1] + v8[2] * v8[2];
            float v25 = b * v24 - a * a;
            float v28 = v7[0] * v8[0] + v7[1] * v8[1] + v7[2] * v8[2];
            float v17 = (v7[0] * v7[0] + v7[1] * v7[1] + v7[2] * v7[2])
                - (r * r);
            float v18 = (v13 * v17) - (v12 * v12);
            if (fabsf(v25) >= 0.000099999997f)
            {
                float v23 = (v28 * v13) - (a * v12);
                if (((v23 * v23) - (v18 * v25)) >= 0.0f)
                {
                    float v26 = (-v23 - sqrtf((v23 * v23) - (v18 * v25)))
                        / v25;
                    *t = v26;
                    if (v26 >= 0.0f && v26 <= 1.0f)
                    {
                        float v20 = (v26 * a) + v12;
                        if (v20 >= 0.0f)
                        {
                            if (v20 <= v13)
                                return 1;
                            if (a < 0.0f)
                            {
                                float v22 = (v13 - v12) / a;
                                *t = v22;
                                if ((((((v28 - a) * 2.0f) + (v22 * b)) * v22)
                                     + ((v13 + v17) - (v12 * 2.0f))) <= 0.0f)
                                    return 1;
                            }
                        }
                        else if (a > 0.0f)
                        {
                            float v21 = 0.0f - (v12 / a);
                            *t = v21;
                            if ((((((v21 * b) + v28) * v21) * 2.0f) + v17)
                                <= 0.0f)
                                return 1;
                        }
                    }
                }
            }
            else if (v18 <= 0.0f)
            {
                if (v12 < 0.0f)
                {
                    *t = 0.0f - (v28 / b);
                    return 1;
                }
                if (v12 > v13)
                {
                    *t = (a - v28) / b;
                    return 1;
                }
                *t = 0.0f;
                return 1;
            }
        }
    }
    return 0;
}

// ea: 0x0060DD10
bool sight_trace_point_patch(const math::Position3* verts,
                             const unsigned char* inds,
                             unsigned short ninds,
                             const math::Position3& p0,
                             const math::Position3& p1,
                             const math::Dir3& dir)
{
    float v27[3];
    v27[0] = 1.0f;
    if (ninds == 0)
        return false;
    const unsigned char* v7 = inds + 1;
    while (1)
    {
        math::Position3 v0 = verts[*(v7 - 1)];
        math::Position3 v1 = verts[*v7];
        math::Position3 v2 = verts[v7[1]];
        float tri[3][3];
        tri[0][0] = v0.v.m128_f32[0];
        tri[0][1] = v0.v.m128_f32[1];
        tri[0][2] = v0.v.m128_f32[2];
        tri[1][0] = v1.v.m128_f32[0];
        tri[1][1] = v1.v.m128_f32[1];
        tri[1][2] = v1.v.m128_f32[2];
        tri[2][0] = v2.v.m128_f32[0];
        tri[2][1] = v2.v.m128_f32[1];
        tri[2][2] = v2.v.m128_f32[2];
        if (collide_ray_triangle(
                p0, dir, *(const math::Position3*)tri[0],
                *(const math::Position3*)tri[1],
                *(const math::Position3*)tri[2], v27[0], v27))
            return true;
        v7 += 3;
        if (v7 - inds >= ninds)
            return false;
    }
}

// ============================================================================
// unpack_poly - ea: 0x6292E0 (CollisionMgr.cpp)
// ============================================================================
// vi4 - packed vertex (3 x 11-bit + pad, 4 bytes) - verified vs IDA
typedef unsigned int vi4;

// ============================================================================
// unpack (physics.o) - ea: 0x6FF680 (cdl_gjk.cpp / cdl_common)
// ============================================================================
// ea: 0x006FF680
void unpack(const cdl_vinfo_t* vinfo, const cdl_array_t* verts,
            math::Dir3* vert_list)
{
    math::Position3 base;
    base.v.m128_f32[0] = (float)vinfo->vbase[0];
    base.v.m128_f32[1] = (float)vinfo->vbase[1];
    base.v.m128_f32[2] = (float)vinfo->vbase[2];
    base.v.m128_f32[3] = 0.0f;
    int num_verts = vinfo->num_verts;
    unsigned int first_vert = vinfo->first_vert;
    if (first_vert >= (unsigned int)verts->m_count
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    vi4* v5 = &((vi4*)verts->m_elements)[first_vert];
    if (num_verts != 0)
    {
        math::Position3 delta;
        delta.v.m128_f32[3] = 0.0f;
        do
        {
            delta.v.m128_f32[0] = (float)(*v5 & 0x7FF) * 0.25f;
            delta.v.m128_f32[1] = (float)((*v5 >> 11) & 0x7FF) * 0.25f;
            delta.v.m128_f32[2] = (float)(*v5 >> 22) * 0.25f;
            vert_list->v = _mm_add_ps(base.v, delta.v);
            ++vert_list;
            ++v5;
            --num_verts;
        } while (num_verts != 0);
    }
}

// ea: 0x006292E0
void unpack_poly(const CGBank* bank, const cdl_vinfo_t* vinfo,
                 const unsigned char* pvi, math::Position3& v0,
                 math::Position3& v1, math::Position3& v2)
{
    unsigned int v6 = vinfo->first_vert + *pvi;
    if (v6 >= (unsigned int)bank->patch_verts.m_count
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    unsigned int v7 = ((unsigned int*)bank->patch_verts.m_elements)[v6];

    math::Position3 vbase;
    vbase.v.m128_f32[0] = (float)vinfo->vbase[0];
    vbase.v.m128_f32[1] = (float)vinfo->vbase[1];
    vbase.v.m128_f32[2] = (float)vinfo->vbase[2];
    vbase.v.m128_f32[3] = 0.0f;

    const unsigned char* pvia = pvi + 1;
    float scale = 0.25f;
    v0.v = _mm_add_ps(vbase.v,
                      _mm_mul_ps(
                          _mm_setr_ps((float)(v7 & 0x7FF),
                                      (float)((v7 >> 11) & 0x7FF),
                                      (float)(v7 >> 22), 0.0f),
                          _mm_set1_ps(scale)));

    unsigned int v10 = vinfo->first_vert + *pvia;
    if (v10 >= (unsigned int)bank->patch_verts.m_count
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    unsigned int v11 = ((unsigned int*)bank->patch_verts.m_elements)[v10];
    v1.v = _mm_add_ps(vbase.v,
                      _mm_mul_ps(
                          _mm_setr_ps((float)(v11 & 0x7FF),
                                      (float)((v11 >> 11) & 0x7FF),
                                      (float)(v11 >> 22), 0.0f),
                          _mm_set1_ps(scale)));

    unsigned int v12 = vinfo->first_vert + pvia[1];
    if (v12 >= (unsigned int)bank->patch_verts.m_count
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    unsigned int v13 = ((unsigned int*)bank->patch_verts.m_elements)[v12];
    v2.v = _mm_add_ps(vbase.v,
                      _mm_mul_ps(
                          _mm_setr_ps((float)(v13 & 0x7FF),
                                      (float)((v13 >> 11) & 0x7FF),
                                      (float)(v13 >> 22), 0.0f),
                          _mm_set1_ps(scale)));
}
