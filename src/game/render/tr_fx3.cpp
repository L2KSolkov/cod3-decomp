// ============================================================================
// tr_fx3.cpp - render.o fx-list / spawn / plane helpers
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/ae_array.h"
#include "aeps/apsEffect.h"
#include "aeps/apsMemory.h"
#include "aeps/apsCommon.h"
#include "aeps/apsDebug.h"

#include <stdint.h>

// ============================================================================
// R_PlaneForSurface - ea: 0x006C1BC0
// ============================================================================
enum surfaceType_t {
    SF_BAD = 0x0,
    SF_SKIP = 0x1,
    SF_POLY = 0x2,
};
struct cplane_s {
    float normal[3];
    float dist;
    unsigned char type;
    unsigned char signbits;
    unsigned char pad[2];
};
const int PlaneFromPoints(float* const plane, const float* const a,
                          const float* const b, const float* const c);

void R_PlaneForSurface(surfaceType_t* surfType, cplane_s* plane)
{
    if (surfType == nullptr)
    {
        plane->normal[0] = 1.0f;
        plane->normal[1] = 0.0f;
        plane->normal[2] = 0.0f;
        plane->dist = 0.0f;
        plane->type = 0;
        plane->signbits = 0;
        return;
    }
    int type = *(int*)surfType;
    if (type >= 0x14)
    {
        // dlight-style surface: verts at +0x3C, index counts at +0x44/+0x46/+0x48
        const float* verts = *(const float**)((char*)surfType + 0x3C);
        int n0 = *(short*)((char*)surfType + 0x44);
        int n1 = *(short*)((char*)surfType + 0x46);
        int n2 = *(short*)((char*)surfType + 0x48);
        const float* a = verts;
        const float* b = verts + 12 * (n2 - n0);
        const float* c = verts + 12 * (n1 - n0);
        float plane4[4];
        PlaneFromPoints(plane4, a, b, c);
        plane->normal[0] = plane4[0];
        plane->normal[1] = plane4[1];
        plane->normal[2] = plane4[2];
        plane->dist = plane4[3];
    }
    else if (type == SF_POLY)
    {
        const float* a = *(const float**)((char*)surfType + 0x0C);
        float plane4[4];
        PlaneFromPoints(plane4, a, a + 8, a + 16);
        plane->normal[0] = plane4[0];
        plane->normal[1] = plane4[1];
        plane->normal[2] = plane4[2];
        plane->dist = plane4[3];
    }
    else
    {
        plane->normal[0] = 1.0f;
        plane->normal[1] = 0.0f;
        plane->normal[2] = 0.0f;
        plane->dist = 0.0f;
        plane->type = 0;
        plane->signbits = 0;
    }
}

// ============================================================================
// RemoveLight / Cmd_PFXReport_f - ea: 0x006C86D0 / 0x006C8680
// ============================================================================
template <typename T>
class ae_vector {
public:
    T* mElements;
    int mSize;
    int mCapacity;

    void erase(T* iToErase)
    {
        for (T* p = iToErase; p + 1 < &mElements[mSize]; ++p)
            *p = p[1];
        --mSize;
    }

    // ?erase@?$ae_vector@PAVParticleEffect@@@@QAEXPAPAVParticleEffect@@0@Z
    void erase(T* iBeginErase, T* iEndErase)
    {
        if (mSize <= 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 286;
            AeAssert::gCurrentExpr = "mSize > 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("can't erase in empty vector"))
            {
                __debugbreak();
            }
        }
        T* end = &mElements[mSize];
        T* dst = iBeginErase;
        if (iBeginErase != end)
        {
            for (T* src = iEndErase; src != end; ++src)
                *dst++ = *src;
            mSize -= (int)(end - dst);
        }
    }

    // ?push_back@?$ae_vector@PAVParticleEffect@@@@QAEXABQAVParticleEffect@@@Z
    void push_back(const T& iElement)
    {
        if (mSize >= mCapacity)
        {
            int v4 = mSize + 4;
            if (mSize <= 3)
                v4 = mSize + 1;
            T* v5 = (T*)tlMemAlloc(sizeof(T) * v4, 8u, 0);
            for (int i = 0; i < mSize; ++i)
                v5[i] = mElements[i];
            if (mElements != nullptr)
            {
                tlMemFree(mElements);
                mElements = nullptr;
                mCapacity = 0;
            }
            mElements = v5;
            mCapacity = v4;
        }
        mElements[mSize++] = iElement;
    }
};

extern void* tlMemAlloc(unsigned int size, unsigned int align,
                        unsigned int flags);  // core.o
extern void tlMemFree(void* p);               // core.o

class PoolAllocator {
public:
    void Release(void* ptr);  // ?Release@PoolAllocator@@QAEXPAX@Z
};

class LightEffect {
    friend void RemoveLight(LightEffect* light);
private:
    static PoolAllocator* sAllocator;  // ?sAllocator@LightEffect@@0PAVPoolAllocator@@A @ 0xF74470
};
PoolAllocator* LightEffect::sAllocator;

ae_vector<LightEffect*> gLightEffectList;  // ?gLightEffectList@@3V?$ae_vector@PAVLightEffect@@@@A @ 0xF74CC8

void RemoveLight(LightEffect* light)
{
    LightEffect** mElements = gLightEffectList.mElements;
    LightEffect** end = &gLightEffectList.mElements[gLightEffectList.mSize];
    if (gLightEffectList.mElements != end)
    {
        do
        {
            if (*mElements == light)
                break;
            ++mElements;
        } while (mElements != end);
    }
    gLightEffectList.erase(mElements);
    if (light != nullptr)
        LightEffect::sAllocator->Release(light);
}

// ae_pair (class tag V; matches ?sArray@ParticleEffect mangling)
template <typename A, typename B>
class ae_pair {
public:
    A first;
    B second;
};

// ParticleEffect view (IDA layout; full methods in tr_fx2.cpp)
class ParticleEffect {
public:
    float cached_pos[4];       // +0x00
    int cached_cell_index;     // +0x10
    int culled;                // +0x14
    short mIndexA;             // +0x18
    short mIndexB;             // +0x1A
    apsEffect* mEffect;        // +0x1C
    void* mAbstractEffectParticle;  // +0x20
    math::Mat43* mPoPtr;       // +0x24
    void* mDObjHandle;         // +0x28
    void* mEntHandle;          // +0x2C
    short mBoneIndex;          // +0x30
    unsigned short mFlags;     // +0x32
    int mPakId;                // +0x34
    void* mRaycastData;        // +0x38

    static ParticleEffect* sArrayData;  // ?sArrayData@ParticleEffect@@2PAV1@A (tr_fx2.cpp)
    static ae_sized_array<ae_pair<short, short>, 256> sArray;  // (tr_fx2.cpp)
};

extern ParticleEffect* FX_ElectEffectToKill(apsEffectTemplate* tmpl);  // tr_fx2.cpp

ae_vector<ParticleEffect*> gParticleEffectList;  // ?gParticleEffectList@@3V?$ae_vector@PAVParticleEffect@@@@A @ 0xF75174
void tlPrintf(const char* fmt, ...);  // core.o

void Cmd_PFXReport_f()
{
    for (ParticleEffect** i = gParticleEffectList.mElements;
         i != &gParticleEffectList.mElements[gParticleEffectList.mSize]; ++i)
    {
        apsEffect* mEffect = (*i)->mEffect;
        const char* mName;
        if (mEffect != nullptr)
            mName = mEffect->mTemplate->GetName();
        else
            mName = "unknown";
        tlPrintf("0x%08x %s\n", *i, mName);
    }
}

// ============================================================================
// IsOkToSpawnNewEffect - ea: 0x006C84F0
// ============================================================================
class ae_heap {
public:
    void** __vftable;
    struct mem_heap2* GetHeapPointer();  // ?GetHeapPointer@ae_heap@@QAEPAUmem_heap@@XZ
};
struct mem_heap2 {
    uint8_t _pad[0x484];
    unsigned int size;      // +0x484
    unsigned int used_byte; // +0x488
};
extern void* gApsHeap;      // ?gApsHeap@@3PAVae_heap@@A (common.cpp)

bool IsOkToSpawnNewEffect(apsEffectTemplate* Tmpl, int juice);
bool IsOkToSpawnNewEffect(apsEffectTemplate* Tmpl, int juice)
{
    if (juice == 0)
        return false;
    bool pfx_free_pool_empty;
    if (ParticleEffect::sArrayData == nullptr || ParticleEffect::sArray.m_size == 0)
        pfx_free_pool_empty = true;
    else
        pfx_free_pool_empty = false;
    mem_heap2* v2 = ((ae_heap*)gApsHeap)->GetHeapPointer();
    bool apsHeapAboutFull = (float)v2->used_byte > (float)v2->size * 0.80000001f;
    if (apsEffect::TestAlloc(Tmpl) != 0 && !pfx_free_pool_empty && !apsHeapAboutFull)
        return true;
    ParticleEffect* v3 = FX_ElectEffectToKill(Tmpl);
    ParticleEffect* v4 = v3;
    if (v3 == nullptr)
        return true;
    apsEffect* mEffect = v3->mEffect;
    const apsEffectTemplate* mTemplate = mEffect->mTemplate;
    unsigned int key;
    if (mEffect != nullptr)
        key = mEffect->mSortKey._32;
    else
        key = 0xFFFFFFFFu;
    apsDebug::PrintWarning(apsDebug::BLUE_ALERT, "killing %08X %s to make way for %s",
                           key, mTemplate->GetName(), Tmpl->GetName());
    int mPakId = v4->mPakId;
    if (mPakId == -1)
    {
        apsEffect::Delete(v4->mEffect, 1u);
    }
    else
    {
        apsMemory::ClearBlockAllocator();
        apsCommon::SetCurrentPakId(mPakId);
        apsCommon::SetPakAllocs(1);
        apsEffect::Delete(v4->mEffect, 1u);
        apsMemory::SetBlockAllocator();
        apsCommon::SetCurrentPakId(-1);
        apsCommon::SetPakAllocs(0);
    }
    v4->mEffect = nullptr;
    return IsOkToSpawnNewEffect(Tmpl, juice - 1);
}
