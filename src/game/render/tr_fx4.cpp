// ============================================================================
// tr_fx4.cpp - render.o particle sort / portal clip / proximity
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "aeps/apsGroup.h"
#include "aeps/apsEffect.h"

#include <stdint.h>

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

class BspPortal {
public:
    int numPortalVerts;      // +0x00
    math::Vector4 plane;     // +0x04
};

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

void R_ChopPortalWinding(const dpvs_plane_t* plane);          // tr_dpvs.cpp
dpvs_plane_t* R_PortalClipPlanesInternal(int iNumPoints);     // file-static

// ============================================================================
// R_PortalClipPlanes - ea: 0x006C5520
// ============================================================================
dpvs_plane_t* R_PortalClipPlanes(BspPortal* portal,
                                 const dpvs_plane_t* parentPlane,
                                 const dpvs_plane_t* planes,
                                 int iPlaneCount, int* piNumPoints)
{
    *piNumPoints = portal->numPortalVerts;
    cdl_proftimer_temp1.value = 0;
    R_ChopPortalWinding(parentPlane);
    if (*piNumPoints != 0)
    {
        __m128 v5 = _mm_mul_ps(g_dpvs.origin.v, PlaneV(portal->plane));
        float dist = _mm_shuffle_ps(PlaneV(portal->plane), PlaneV(portal->plane), 255).m128_f32[0]
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
                R_ChopPortalWinding(planes + v_1020);
                if (*piNumPoints == 0)
                    break;
                ++v_1020;
                if (v_1020 >= iPlaneCount)
                    goto clipInternal;
            }
            return nullptr;
        }
        else
        {
            R_ChopPortalWinding(g_dpvs.farPlane);
            if (*piNumPoints != 0)
            {
                int v_1020 = 0;
                if (iPlaneCount <= 0)
                    goto clipInternal;
                while (1)
                {
                    R_ChopPortalWinding(planes + v_1020);
                    if (*piNumPoints == 0)
                        break;
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
    return R_PortalClipPlanesInternal(*piNumPoints);
}
