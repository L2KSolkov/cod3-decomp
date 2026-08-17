// ============================================================================
// tr_fx4.cpp - render.o particle sort / portal clip / proximity
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "aeps/apsGroup.h"
#include "aeps/apsEffect.h"

#include <stdint.h>
#include <string.h>

// AeAssert (game.o)
namespace AeAssert {
enum ECoderId { COD3 = 0, JSV = 10 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

// ae_vector (matches ?gSortedParticleEffectList mangling)
template <typename T>
class ae_vector {
public:
    T* mElements;   // +0x00
    int mSize;      // +0x04
    int mCapacity;  // +0x08

    T& operator[](int i) { return mElements[i]; }
};

class ParticleEffect {
public:
    uint8_t _pad0[0x1C];
    apsEffect* mEffect;        // +0x1C
    uint8_t _pad1[0x34 - 0x20];
    int mPakId;                // +0x34
};

extern ae_vector<ParticleEffect*> gSortedParticleEffectList;  // tr_fx2.cpp
extern void FX_SortParticleEffectList(unsigned int indexLeft, int indexRight);

// ============================================================================
// FX_SortParticleEffectList - ea: 0x006C7690 (introsort, quicksort partition)
// ============================================================================
static unsigned int SortKey(ParticleEffect* pe)
{
    if (pe->mEffect != nullptr)
        return pe->mEffect->mSortKey._32;
    return 0xFFFFFFFFu;
}

void FX_SortParticleEffectList(int indexLeft, int indexRight)
{
    int v2 = (int)indexLeft;
    int v3 = indexRight;
    if ((indexRight - (int)indexLeft) > 4)
    {
        while (1)
        {
            int v4 = (v2 + v3) / 2;
            unsigned int v = SortKey(gSortedParticleEffectList[v2]);
            unsigned int mid = SortKey(gSortedParticleEffectList[v4]);
            if (v > mid)
            {
                ParticleEffect* tmp = gSortedParticleEffectList[v2];
                gSortedParticleEffectList[v2] = gSortedParticleEffectList[v4];
                gSortedParticleEffectList[v4] = tmp;
            }
            unsigned int va = SortKey(gSortedParticleEffectList[v2]);
            unsigned int vr = SortKey(gSortedParticleEffectList[v3]);
            if (va > vr)
            {
                ParticleEffect* tmp = gSortedParticleEffectList[v2];
                gSortedParticleEffectList[v2] = gSortedParticleEffectList[v3];
                gSortedParticleEffectList[v3] = tmp;
            }
            unsigned int vm = SortKey(gSortedParticleEffectList[v4]);
            unsigned int vr2 = SortKey(gSortedParticleEffectList[v3]);
            if (vm > vr2)
            {
                ParticleEffect* tmp = gSortedParticleEffectList[v4];
                gSortedParticleEffectList[v4] = gSortedParticleEffectList[v3];
                gSortedParticleEffectList[v3] = tmp;
            }
            int v16 = v3 - 1;
            ParticleEffect* pivot = gSortedParticleEffectList[v4];
            gSortedParticleEffectList[v4] = gSortedParticleEffectList[v16];
            gSortedParticleEffectList[v16] = pivot;

            int v18 = (int)indexLeft;
            unsigned int pivotKey = SortKey(gSortedParticleEffectList[v16]);
            while (1)
            {
                unsigned int v20 = pivotKey;
                do
                {
                    ++v18;
                    if (v18 < 0 || v18 >= gSortedParticleEffectList.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
                        AeAssert::gCurrentLine = 167;
                        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
                        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                            __debugbreak();
                    }
                } while (SortKey(gSortedParticleEffectList[v18]) < v20);
                do
                {
                    --v16;
                    if (v16 < 0 || v16 >= gSortedParticleEffectList.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
                        AeAssert::gCurrentLine = 167;
                        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
                        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                            __debugbreak();
                    }
                } while (SortKey(gSortedParticleEffectList[v16]) > v20);
                if (v16 < v18)
                    break;
                ParticleEffect* tmp = gSortedParticleEffectList[v18];
                gSortedParticleEffectList[v18] = gSortedParticleEffectList[v16];
                gSortedParticleEffectList[v16] = tmp;
            }
            ParticleEffect* tmp = gSortedParticleEffectList[v18];
            gSortedParticleEffectList[v18] = gSortedParticleEffectList[indexRight - 1];
            gSortedParticleEffectList[indexRight - 1] = tmp;
            FX_SortParticleEffectList(indexLeft, v16);
            indexLeft = v18 + 1;
            if ((indexRight - (v18 + 1)) <= 4)
                break;
            v3 = indexRight;
            v2 = v18 + 1;
        }
    }
}

// ============================================================================
// dpvs portal clip planes
// ============================================================================
class PoolAllocator {
public:
    void* Allocate(unsigned int s, bool forceHeapAlloc);
};

struct dpvs_plane_t {
    math::Vector4 data;          // +0x00
    unsigned char side[3];       // +0x10
    unsigned char frontal;       // +0x13
    static PoolAllocator* sAllocator;  // ?sAllocator@dpvs_plane_t@@2PAVPoolAllocator@@A @ 0xF7442C
};

PoolAllocator* dpvs_plane_t::sAllocator;

class BspCell;

class BspPortal {
public:
    dpvs_plane_t plane;           // +0x00
    BspCell* cell;                // +0x20
    math::Position3* firstPortalVert; // +0x24
    int numPortalVerts;            // +0x28
    int active;                    // +0x2C
};
static_assert(sizeof(BspPortal) == 0x30, "BspPortal size mismatch");

// local accessor (Vector4 data member)
static inline __m128 PlaneV(const math::Vector4& p) { return p.v; }

// dpvs_t view (origin +0x00, farPlane +0xD4)
extern "C" struct dpvs_t {
    math::Position3 origin;      // +0x00
    uint8_t _pad[0xD4 - 0x10];
    dpvs_plane_t* farPlane;      // +0xD4
} g_dpvs;  // tr_dpvs.cpp
struct cdl_proftimer {
    unsigned __int64 stamp;
    unsigned __int64 value;
};
extern cdl_proftimer cdl_proftimer_temp1;

static inline float Dot3(__m128 a, __m128 b)
{
    __m128 product = _mm_mul_ps(a, b);
    return product.m128_f32[0] + product.m128_f32[1] + product.m128_f32[2];
}

// IDA usercall body at 0x6BF510.  The original keeps positive/on-plane
// vertices and emits the crossing point for every non-on transition.
static const math::Position3* ClipPortalWinding(const dpvs_plane_t* plane,
                                                const math::Position3* input,
                                                int* count,
                                                math::Position3* output)
{
    const int n = *count;
    float distance[32];
    unsigned char side[32];
    int negativeCount = 0;
    int positiveCount = 0;

    for (int i = 0; i < n; ++i)
    {
        distance[i] = Dot3(input[i].v, plane->data.v) - plane->data.v.m128_f32[3];
        side[i] = 2;
        if (distance[i] < -0.005f)
        {
            side[i] = 1;
            ++negativeCount;
        }
        else if (distance[i] > 0.005f)
        {
            side[i] = 0;
            ++positiveCount;
        }
    }

    if (positiveCount == 0)
        return input;
    if (negativeCount == 0)
    {
        *count = 0;
        return nullptr;
    }

    int newCount = 0;
    for (int i = 0; i < n; ++i)
    {
        const int next = (i + 1) % n;
        if (side[i] == 0 || side[i] == 2)
            output[newCount++] = input[i];

        if (side[next] != 2 && side[next] != side[i])
        {
            const float t = distance[i] / (distance[i] - distance[next]);
            output[newCount].v = _mm_add_ps(
                input[i].v,
                _mm_mul_ps(_mm_sub_ps(input[next].v, input[i].v),
                           _mm_set1_ps(t)));
            ++newCount;
            if (newCount == 32)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
                AeAssert::gCurrentLine = 680;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("MAX_POINTS_ON_PORTAL reached (CD)"))
                    __debugbreak();
                break;
            }
        }
    }

    if (newCount < 3)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
        AeAssert::gCurrentLine = 685;
        AeAssert::gCurrentExpr = "iNewPts >= 3";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    *count = newCount;
    return output;
}

// IDA usercall body at 0x6C4FB0.  It builds one plane per non-degenerate
// portal edge in the pool supplied by dpvs_plane_t::sAllocator.
static dpvs_plane_t* BuildPortalClipPlanes(int count,
                                           const math::Position3* points)
{
    if (count == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_dpvs.cpp";
        AeAssert::gCurrentLine = 337;
        AeAssert::gCurrentExpr = "iNumPoints";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    dpvs_plane_t* result = (dpvs_plane_t*)dpvs_plane_t::sAllocator->Allocate(
        32u * (unsigned int)count, false);
    math::Dir3 normal[32];
    math::Dir3 delta[32];

    for (int i = 0; i < count; ++i)
    {
        normal[i].v = _mm_sub_ps(points[i].v, g_dpvs.origin.v);
        const int next = (i + 1) % count;
        delta[i].v = _mm_sub_ps(points[next].v, points[i].v);
    }

    dpvs_plane_t* write = result;
    for (int i = 0; i < count; ++i)
    {
        const __m128 d = delta[i].v;
        const __m128 n = normal[i].v;
        __m128 cross = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(d, d, _MM_SHUFFLE(3, 0, 2, 1)),
                       _mm_shuffle_ps(n, n, _MM_SHUFFLE(3, 1, 0, 2))),
            _mm_mul_ps(_mm_shuffle_ps(d, d, _MM_SHUFFLE(3, 1, 0, 2)),
                       _mm_shuffle_ps(n, n, _MM_SHUFFLE(3, 0, 2, 1))));
        const float lengthSquared = Dot3(cross, cross);
        if (lengthSquared < 0.000001f)
            continue;

        uint32_t bits;
        memcpy(&bits, &lengthSquared, sizeof(bits));
        bits = 0x5F3759DFu - (bits >> 1);
        float inverseLength;
        memcpy(&inverseLength, &bits, sizeof(inverseLength));
        inverseLength = (1.5f - ((lengthSquared * 0.5f)
                                  * inverseLength * inverseLength)) * inverseLength;
        cross = _mm_mul_ps(cross, _mm_set1_ps(inverseLength));

        write->data.v = cross;
        write->side[0] = cross.m128_f32[0] <= 0.0f ? 0 : 0xC;
        write->side[1] = cross.m128_f32[1] <= 0.0f ? 4 : 16;
        write->side[2] = cross.m128_f32[2] <= 0.0f ? 8 : 20;
        write->data.v.m128_f32[3] = Dot3(cross, g_dpvs.origin.v) - 0.005f;
        ++write;
    }
    return result;
}

// ============================================================================
// R_PortalClipPlanes - ea: 0x006C5520
// ============================================================================
dpvs_plane_t* R_PortalClipPlanes(BspPortal* portal,
                                 const dpvs_plane_t* parentPlane,
                                 const dpvs_plane_t* planes,
                                 int iPlaneCount, int* piNumPoints)
{
    *piNumPoints = portal->numPortalVerts;
    const math::Position3* points = portal->firstPortalVert;
    alignas(16) math::Position3 clipPoints[64];
    const math::Position3* current = points;
    math::Position3* output = clipPoints;
    cdl_proftimer_temp1.value = 0;
    current = ClipPortalWinding(parentPlane, current, piNumPoints, output);
    if (*piNumPoints != 0)
    {
        __m128 v5 = _mm_mul_ps(g_dpvs.origin.v, PlaneV(portal->plane.data));
        float dist = _mm_shuffle_ps(PlaneV(portal->plane.data), PlaneV(portal->plane.data), 255).m128_f32[0]
                   - (v5.m128_f32[0]
                      + (_mm_shuffle_ps(v5, v5, 85).m128_f32[0]
                         + _mm_shuffle_ps(v5, v5, 170).m128_f32[0]));
        if (dist < 1.0f)
        {
            *piNumPoints = iPlaneCount;
            dpvs_plane_t* result = (dpvs_plane_t*)dpvs_plane_t::sAllocator->Allocate(32 * iPlaneCount, false);
            memcpy(result, planes, 4 * ((32 * iPlaneCount) >> 2));
            return result;
        }
        if (g_dpvs.farPlane == nullptr)
        {
            int v_1020 = 0;
            if (iPlaneCount <= 0)
                goto clipInternal;
            while (1)
            {
                math::Position3* nextOutput = (output == clipPoints) ? clipPoints + 32 : clipPoints;
                current = ClipPortalWinding(planes + v_1020, current, piNumPoints, nextOutput);
                if (*piNumPoints == 0)
                    break;
                output = nextOutput;
                ++v_1020;
                if (v_1020 >= iPlaneCount)
                    goto clipInternal;
            }
            return nullptr;
        }
        else
        {
            math::Position3* nextOutput = (output == clipPoints) ? clipPoints + 32 : clipPoints;
            current = ClipPortalWinding(g_dpvs.farPlane, current, piNumPoints, nextOutput);
            if (*piNumPoints != 0)
            {
                output = nextOutput;
                int v_1020 = 0;
                if (iPlaneCount <= 0)
                    goto clipInternal;
                while (1)
                {
                    nextOutput = (output == clipPoints) ? clipPoints + 32 : clipPoints;
                    current = ClipPortalWinding(planes + v_1020, current, piNumPoints, nextOutput);
                    if (*piNumPoints == 0)
                        break;
                    output = nextOutput;
                    ++v_1020;
                    if (v_1020 >= iPlaneCount)
                        goto clipInternal;
                }
                return nullptr;
            }
        }
    }
    return nullptr;
clipInternal:
    return BuildPortalClipPlanes(*piNumPoints, current);
}
