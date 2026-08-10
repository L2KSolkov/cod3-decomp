// ============================================================================
// g_winding.cpp - game.o winding helpers (cm_load.cpp / polylib)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <string.h>

// ============================================================================
// winding_t - polygon winding (points * 12 bytes + 4-byte numpoints)
// ============================================================================
struct winding_t {
    int   numpoints;   // +0x00
    float p[1][3];     // +0x04
};

extern int c_active_windings;   // ?c_active_windings@@3HA (game.o)
extern int c_peak_windings;     // ?c_peak_windings@@3HA (game.o)
extern void* _Z_MallocInternal(unsigned int size);  // core.o
extern void  _Z_FreeInternal(void* ptr);            // core.o

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

// ea: 0x00609750
void WindingPlane(winding_t* w, float* normal, float* dist)
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
void WindingBounds(winding_t* w, float* mins, float* maxs)
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
void WindingCenter(winding_t* w, float* center)
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
winding_t* BaseWindingForPlane(const float* normal, float dist)
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
void ClipWindingEpsilon(winding_t* in, float* normal, float dist,
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
