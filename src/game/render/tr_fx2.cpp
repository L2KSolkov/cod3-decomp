// ============================================================================
// tr_fx2.cpp - render.o aps client / particle / light effect helpers
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "core/color.h"
#include "aeps/apsInternal.h"
#include "aeps/apsMemory.h"
#include "aeps/apsCommon.h"
#include "aeps/apsEffect.h"
#include "core/ae_array.h"
#include "game/game_types.h"

#include <stdint.h>
#include <float.h>
#include <stdlib.h>

class DObjHandleDb;
class EntityHandleDb;

// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
bool Warning(const char* fmtstring, ...);
}

// ae_pair (class tag V per ?sArray@ParticleEffect mangling)
template <typename A, typename B>
class ae_pair {
public:
    A first;
    B second;
};

// ae_vector (ae_array.h-style; ?gSortedParticleEffectList@@3V?$ae_vector@PAVParticleEffect@@@@A)
template <typename T>
class ae_vector {
public:
    T* mElements;   // +0x00
    int mSize;      // +0x04
    int mCapacity;  // +0x08

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

    void reserve(int iCapacity)
    {
        if (iCapacity > mCapacity)
        {
            T* v3 = (T*)tlMemAlloc(sizeof(T) * iCapacity, 8u, 0);
            for (int i = 0; i < mSize; ++i)
                v3[i] = mElements[i];
            if (mElements != nullptr)
            {
                tlMemFree(mElements);
                mElements = nullptr;
                mCapacity = 0;
            }
            mElements = v3;
            mCapacity = iCapacity;
        }
    }

    void resize(int iNewSize)
    {
        if (iNewSize > mCapacity)
        {
            T* v4 = (T*)tlMemAlloc(sizeof(T) * iNewSize, 8u, 0);
            for (int i = 0; i < mSize; ++i)
                v4[i] = mElements[i];
            if (mElements != nullptr)
            {
                tlMemFree(mElements);
                mElements = nullptr;
                mCapacity = 0;
            }
            mElements = v4;
            mCapacity = iNewSize;
            mSize = iNewSize;
        }
        else
        {
            mSize = iNewSize;
        }
    }
};

// mem heap helpers (core.o)
void* mem_heap_malloc(unsigned int size);
void* mem_heap_malloc(int alignment, unsigned int size);
void mem_heap_free(void* ptr);

// tlMemFree (tl library)
void tlMemFree(void* Ptr);
void* tlMemAlloc(unsigned int size, unsigned int align, unsigned int flags);

// apsEffect (apsEffect.h) - Delete declared there

// ============================================================================
// apsMemConfig - ea: 0x006C3190 / 0x006C31C0
// ============================================================================
class apsMemConfig {
public:
    apsMemConfig(TPakId pakId);
    ~apsMemConfig();
};

apsMemConfig::apsMemConfig(TPakId pakId)
{
    apsMemory::ClearBlockAllocator();
    apsCommon::SetCurrentPakId((int)pakId);
    apsCommon::SetPakAllocs(1);
}

apsMemConfig::~apsMemConfig()
{
    apsMemory::SetBlockAllocator();
    apsCommon::SetCurrentPakId(-1);
    apsCommon::SetPakAllocs(0);
}

// ============================================================================
// apsDebugRender - ea: 0x006C31E0
// ============================================================================
void apsDebugRender()
{
}

// ============================================================================
// ApsGameClient debug draws (virtual overrides; defined in apsInternal.h)
// ============================================================================
void ApsGameClient::DebugDrawBox(const math::Dir3& min, const math::Dir3& max,
                                 const math::Vector4& color)
{
    (void)min; (void)max; (void)color;
}

void ApsGameClient::DebugDrawSolidSphere(const math::Dir3& center,
                                         float radius,
                                         const math::Vector4& color)
{
    (void)center; (void)radius; (void)color;
}

void ApsGameClient::DebugDrawLine(const math::Dir3& start, const math::Dir3& end,
                                  const math::Vector4& color, float thickness)
{
    (void)start; (void)end; (void)color; (void)thickness;
}

// ============================================================================
// ParticleEffect - ea: 0x006C3220 / 0x006C32D0
// ============================================================================
class ParticleEffect {
public:
    float cached_pos[4];       // +0x00
    int cached_cell_index;     // +0x10
    int culled;                // +0x14
    short mIndexA;             // +0x18 (ae_pair)
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

    ~ParticleEffect();         // ??1ParticleEffect@@QAE@XZ
    unsigned int GetSortKey(); // ?GetSortKey@ParticleEffect@@QAEIXZ
    ParticleEffect(bool bInGame);  // ??0ParticleEffect@@QAE@_N@Z
    void Report();                 // ?Report@ParticleEffect@@QAEXXZ
    static void DestroyArray();  // ?DestroyArray@ParticleEffect@@SAXXZ
    static void InitArray();     // ?InitArray@ParticleEffect@@SAXXZ
    static ParticleEffect* New();  // ?New@ParticleEffect@@SAPAV1@XZ
    static void Delete(ParticleEffect* pEffect);  // ?Delete@ParticleEffect@@SAXPAV1@@Z
    static bool IsFreePoolEmpty();  // ?IsFreePoolEmpty@ParticleEffect@@SA_NXZ
    static ParticleEffect* sArrayData;  // ?sArrayData@ParticleEffect@@2PAV1@A @ 0xF74460
    static ae_sized_array<ae_pair<short, short>, 256> sArray;  // ?sArray@ParticleEffect@@2V?$ae_sized_array@V?$ae_pair@FF@@$0BAA@@@A @ 0xF751A8
};
static_assert(sizeof(ParticleEffect) == 0x3C, "ParticleEffect size mismatch");

ParticleEffect* ParticleEffect::sArrayData;
ae_sized_array<ae_pair<short, short>, 256> ParticleEffect::sArray;

// ParticleEffect pool statics (render.o data)
ae_vector<ParticleEffect*> gSortedParticleEffectList;  // @ 0xF74484
int gDumpParticleEffectList;                           // @ 0xF74464

// tlPrintf (core.o)
void tlPrintf(const char* fmt, ...);

ParticleEffect::ParticleEffect(bool bInGame)
{
    mDObjHandle = nullptr;
    mEntHandle = nullptr;
    mFlags = 0;
    if (!bInGame)
    {
        mIndexA = -1;
        mIndexB = -1;
    }
    mEffect = nullptr;
    mDObjHandle = nullptr;
    mEntHandle = nullptr;
    mPoPtr = nullptr;
    mPakId = -1;
    mFlags = 0;
    mFlags |= 0x10u;
    mRaycastData = nullptr;
    cached_pos[3] = -1.0f;
    culled = 0;
}

// ea: 0x006D3580
void ParticleEffect::InitArray()
{
    if (ParticleEffect::sArrayData == nullptr)
    {
        void* block = mem_heap_malloc(0x3C04u);
        ParticleEffect* v2;
        if (block != nullptr)
        {
            *(unsigned int*)block = 256;
            v2 = (ParticleEffect*)((char*)block + 4);
            for (int i = 0; i < 256; ++i)
                new (&v2[i]) ParticleEffect(false);
        }
        else
        {
            v2 = nullptr;
        }
        ParticleEffect::sArrayData = v2;
        ParticleEffect::sArray.m_size = 0;
        short v0 = 0;
        for (int i = 0; i < 256; ++i)
        {
            ParticleEffect::sArrayData[i].mIndexA = v0;
            ParticleEffect::sArrayData[i].mIndexB = v0;
            ae_pair<short, short> idx;
            idx.first = ParticleEffect::sArrayData[i].mIndexA;
            idx.second = ParticleEffect::sArrayData[i].mIndexB;
            ParticleEffect::sArray.push_back(idx);
            ++v0;
        }
    }
}

void ParticleEffect::Report()
{
    const char* mName;
    if (mEffect != nullptr)
        mName = mEffect->mTemplate->GetName();
    else
        mName = "unknown";
    if (mEffect != nullptr)
        tlPrintf("0x%08x 0x%08x %s\n", this, mEffect->mSortKey._32, mName);
    else
        tlPrintf("0x%08x 0x%08x %s\n", this, 0xFFFFFFFFu, mName);
}

ParticleEffect* ParticleEffect::New()
{
    if (ParticleEffect::sArrayData == nullptr)
        return nullptr;
    if (ParticleEffect::sArray.m_size == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
        AeAssert::gCurrentLine = 174;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored() && AeAssert::Warning("ParticleEffect pool is empty"))
            __debugbreak();
        return nullptr;
    }
    short second = ParticleEffect::sArray.m_elements[--ParticleEffect::sArray.m_size].second;
    ParticleEffect* v2 = &ParticleEffect::sArrayData[second];
    if (ParticleEffect::sArray.m_elements[ParticleEffect::sArray.m_size].first != v2->mIndexA
        || second != v2->mIndexB)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
        AeAssert::gCurrentLine = 183;
        AeAssert::gCurrentExpr = "index == pEffect->mIndex";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("index mismatch"))
            __debugbreak();
    }
    if (v2 != nullptr)
    {
        v2->mDObjHandle = nullptr;
        v2->mEntHandle = nullptr;
        v2->mFlags = 0;
        v2->mEffect = nullptr;
        v2->mDObjHandle = nullptr;
        v2->mEntHandle = nullptr;
        v2->mPoPtr = nullptr;
        v2->mPakId = -1;
        v2->mFlags = 0;
        v2->mFlags |= 0x10u;
        v2->mRaycastData = nullptr;
        v2->cached_pos[3] = -1.0f;
        v2->culled = 0;
    }
    return v2;
}

void ParticleEffect::Delete(ParticleEffect* pEffect)
{
    if (pEffect != nullptr)
    {
        if (pEffect->mIndexA == -1)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
            AeAssert::gCurrentLine = 197;
            AeAssert::gCurrentExpr = "-1 != pEffect->mIndex.first";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("bad index"))
                __debugbreak();
        }
        pEffect->~ParticleEffect();
        pEffect->mIndexA = (short)ParticleEffect::sArray.m_size;
        ae_pair<short, short> idx;
        idx.first = pEffect->mIndexA;
        idx.second = pEffect->mIndexB;
        ParticleEffect::sArray.push_back(idx);
    }
}

bool ParticleEffect::IsFreePoolEmpty()
{
    return ParticleEffect::sArrayData == nullptr || ParticleEffect::sArray.m_size == 0;
}

// ============================================================================
// FX_ElectEffectToKill - ea: 0x006C75B0
// ============================================================================
ParticleEffect* FX_ElectEffectToKill(apsEffectTemplate* tmpl)
{
    if (gSortedParticleEffectList.mSize == 0)
        return nullptr;
    ParticleEffect** v1 = &gSortedParticleEffectList.mElements[gSortedParticleEffectList.mSize - 1];
    if (v1 == gSortedParticleEffectList.mElements)
        return nullptr;
    ParticleEffect* result = nullptr;
    while (1)
    {
        apsEffect* mEffect = (*v1)->mEffect;
        if (mEffect != nullptr)
        {
            unsigned int key = mEffect->mSortKey._32;
            if ((key >> 25) <= (unsigned int)tmpl->mPriority)
                return result;
            if ((0x1000000u & key) != 0)
                break;
        }
        ParticleEffect** v5 = v1--;
        if (v5 == gSortedParticleEffectList.mElements)
            return result;
    }
    return *v1;
}

// ============================================================================
// FX_DumpParticleEffectList - ea: 0x006C7620
// ============================================================================
void FX_DumpParticleEffectList()
{
    if (gDumpParticleEffectList != 0)
    {
        ParticleEffect** mElements = gSortedParticleEffectList.mElements;
        ParticleEffect** end = &gSortedParticleEffectList.mElements[gSortedParticleEffectList.mSize];
        gDumpParticleEffectList = 0;
        if (gSortedParticleEffectList.mElements != end)
        {
            do
            {
                apsEffect* mEffect = (*mElements)->mEffect;
                const char* mName;
                if (mEffect != nullptr)
                    mName = mEffect->mTemplate->GetName();
                else
                    mName = "unknown";
                unsigned int key;
                if (mEffect != nullptr)
                    key = mEffect->mSortKey._32;
                else
                    key = 0xFFFFFFFFu;
                tlPrintf("0x%08x 0x%08x %s\n", *mElements++, key, mName);
            } while (mElements != end);
        }
    }
}

unsigned int ParticleEffect::GetSortKey()
{
    if (mEffect != nullptr)
        return mEffect->mSortKey._32;
    return 0xFFFFFFFFu;
}

ParticleEffect::~ParticleEffect()
{
    apsEffect* mEffect = this->mEffect;
    if (mEffect != nullptr)
    {
        int mPakId = this->mPakId;
        if (mPakId == -1)
        {
            apsEffect::Delete(mEffect, 0);
        }
        else
        {
            apsMemory::ClearBlockAllocator();
            apsCommon::SetCurrentPakId(mPakId);
            apsCommon::SetPakAllocs(1);
            apsEffect::Delete(this->mEffect, 0);
            apsMemory::SetBlockAllocator();
            apsCommon::SetCurrentPakId(-1);
            apsCommon::SetPakAllocs(0);
        }
        this->mEffect = nullptr;
    }
    if (this->mRaycastData != nullptr)
    {
        tlMemFree(this->mRaycastData);
        this->mRaycastData = nullptr;
    }
}

void ParticleEffect::DestroyArray()
{
    if (ParticleEffect::sArrayData != nullptr)
    {
        // Counted array: element count stored in the prefix slot's mRaycastData.
        unsigned int count = (unsigned int)sArrayData[-1].mRaycastData;
        void* p_mRaycastData = &sArrayData[-1].mRaycastData;
        for (unsigned int i = 0; i < count; ++i)
            sArrayData[i].~ParticleEffect();
        mem_heap_free(p_mRaycastData);
        ParticleEffect::sArrayData = nullptr;
    }
}

// ============================================================================
// FX_SetRainDrops - ea: 0x006C3420
// ============================================================================
bool gEnableRainDrops;  // ?gEnableRainDrops@@3_NA @ 0xF7444C

void FX_SetRainDrops(bool on)
{
    gEnableRainDrops = on;
}

// ============================================================================
// LightEffect - ea: 0x006C3480..0x006C36B0
// ============================================================================
class PoolAllocator;
class LightEffect {
public:
    enum eType {
        PROJECTED_TEXTURE = 0x0,
        VERTEX_LIGHT = 0x1,
    };
    enum eTime : int {
        FLASH = 0x1,
        FOREVER = 0xFFFFFFFF,
    };

    eType mType;                  // +0x00
    int mPakId;                   // +0x04
    math::Position3 mLightPos;    // +0x10
    float mColor[4];              // +0x20
    bool mActive;                 // +0x30
    float mMSecLifetime;          // +0x34
    float mMSecLifeOrig;          // +0x38
    bool mFlicker;                // +0x3C
    bool mKill;                   // +0x3D
    float mColorOriginal[4];      // +0x40
    float mFlickerRatio;          // +0x50
    float mInnerRadius;           // +0x54
    float mOuterRadius;           // +0x58
    float mScale;                 // +0x5C
    bool mFade;                   // +0x60

    LightEffect(eType type, float timeMS);  // ??0LightEffect@@QAE@W4eType@0@M@Z
    ~LightEffect();                         // ??1LightEffect@@QAE@XZ
    void Start();                           // ?Start@LightEffect@@QAEXXZ
    void SetColor(float r, float g, float b, float a);  // ?SetColor@LightEffect@@QAEXMMMM@Z
    static PoolAllocator* sAllocator;       // ?sAllocator@LightEffect@@0PAVPoolAllocator@@A (tr_fx3.cpp)
};

class PoolAllocator {
public:
    void* Allocate(unsigned int s, bool forceHeapAlloc);  // ?Allocate@PoolAllocator@@QAEPAXI_N@Z
    void Release(void* ptr);                              // ?Release@PoolAllocator@@QAEXPAX@Z
};

extern float gNearLightRadius;  // ?gNearLightRadius@@3MA @ 0xF74474
extern float gFarLightRadius;   // ?gFarLightRadius@@3MA @ 0xDFB158

LightEffect::LightEffect(eType type, float timeMS)
{
    mMSecLifetime = timeMS;
    mMSecLifeOrig = timeMS;
    mType = type;
    mInnerRadius = 0.0f;
    mOuterRadius = 0.0f;
    mFade = false;
    mKill = false;
    mScale = 1.0f;
}

LightEffect::~LightEffect()
{
}

void LightEffect::Start()
{
    mFlicker = false;
    mActive = true;
    mKill = false;
    mInnerRadius = gNearLightRadius;
    mOuterRadius = gFarLightRadius;
}

void LightEffect::SetColor(float r, float g, float b, float a)
{
    mColor[0] = r;
    mColor[1] = g;
    mColor[2] = b;
    mColor[3] = a;
    mColorOriginal[0] = r;
    mColorOriginal[1] = g;
    mColorOriginal[2] = b;
}

// ============================================================================
// PlayEffect / FX_Play*EffectID - ea: 0x006D3730 / 0x006D3A00 / 0x006D3A90
// ============================================================================
float gFXTime;  // ?gFXTime@@3MA (render.o)
extern ae_vector<ParticleEffect*> gParticleEffectList;  // tr_fx3.cpp
extern ae_vector<LightEffect*> gLightEffectList;         // tr_fx3.cpp
extern bool IsOkToSpawnNewEffect(apsEffectTemplate* Tmpl, int juice);  // tr_fx3.cpp
extern void Com_Printf(const char* fmt, ...);  // core.o
extern void MakeNormalVectors(const float* const forward, float* const right,
                              float* const up);  // q_math.cpp

// ea: 0x006D3730
ParticleEffect* PlayEffect(TPakId pakId, int id, math::Mat43& Mat,
                           DbLinkedHandle<DObjHandleDb, DObj> boltObjHandle,
                           DbLinkedHandle<EntityHandleDb, Entity> boltEntHandle,
                           int boltBoneIndex, bool boltAttchedToEnt)
{
    apsEffectTemplate* tmpl = (apsEffectTemplate*)id;
    if (tmpl == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
        AeAssert::gCurrentLine = 1691;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("ParticleEffect.cpp - PlayEffect() :: No effect file found"))
        {
            __debugbreak();
        }
    }
    else if (IsOkToSpawnNewEffect(tmpl, 4))
    {
        ParticleEffect* v9 = ParticleEffect::New();
        ParticleEffect* fx = v9;
        if (v9 != nullptr)
        {
            v9->mPakId = (int)pakId;
            apsEffect* v10;
            if (pakId == PAK_ID_INVALID)
            {
                v10 = apsEffect::New(tmpl, gFXTime);
            }
            else
            {
                apsMemConfig memCfg(pakId);
                v10 = apsEffect::New(tmpl, gFXTime);
            }
            if (v10 != nullptr)
            {
                v9->mEffect = v10;
                v10->SetLocalToWorldTransform(Mat);
                if (boltObjHandle.mHandle.mVal != 0
                    || boltEntHandle.mHandle.mVal != 0)
                {
                    v9->mDObjHandle = (void*)boltObjHandle.mHandle.mVal;
                    v9->mEntHandle = (void*)boltEntHandle.mHandle.mVal;
                    v9->mBoneIndex = (short)boltBoneIndex;
                    if (boltAttchedToEnt)
                        v9->mFlags |= 1u;
                }
                int element_num = 0;
                for (int v11 = 0; v11 < tmpl->mElements.mSize; ++v11)
                {
                    if (tmpl->GetElement(v11).mEndTime == 3.4028235e38f)
                    {
                        int mEndTime = 0;
                        bool v20 = true;
                        for (int v13 = 0; v13 < tmpl->mElements.mSize; ++v13)
                        {
                            if (tmpl->GetElement(v13).mEndTime < 3.4028235e38f)
                            {
                                if (tmpl->GetElement(v13).mEndTime > mEndTime)
                                    mEndTime = (int)tmpl->GetElement(v13).mEndTime;
                                v20 = false;
                            }
                        }
                        if (v20)
                            mEndTime = 10;
                        tmpl->SetEndTime(element_num, (float)mEndTime);
                    }
                    element_num = v11 + 1;
                }
                gParticleEffectList.push_back(fx);
                v9->mPakId = (int)pakId;
                return v9;
            }
            else
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
                AeAssert::gCurrentLine = 1743;
                AeAssert::gCurrentExpr = "pNewEffect != 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Effect could not be created, check aeps memory status (debug menu)"))
                {
                    __debugbreak();
                }
                Com_Printf("FX_PlayFX:-- %x failed to create apsEffect\n", id);
                ParticleEffect::Delete(v9);
                return nullptr;
            }
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
            AeAssert::gCurrentLine = 1712;
            AeAssert::gCurrentExpr = "fx != 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Particle could not be created, check aeps memory status (debug menu)"))
            {
                __debugbreak();
            }
            Com_Printf("FX_PlayFX:-- %x failed to create ParticleEffect\n", id);
            return nullptr;
        }
    }
    return nullptr;
}

// ea: 0x006D3A00
ParticleEffect* PlayEffect(TPakId pakId, int id, const float* origin)
{
    math::Mat43 v7;
    v7.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v7.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    v7.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    v7.w.v = _mm_setr_ps(origin[0], origin[1], origin[2], 0.0f);
    return PlayEffect(pakId, id, v7, DbLinkedHandle<DObjHandleDb, DObj>(),
                      DbLinkedHandle<EntityHandleDb, Entity>(), -1, false);
}

// ea: 0x006D3A90
ParticleEffect* PlayEffect(TPakId pakId, int id, const float* origin,
                           const float (*axis)[3])
{
    math::Mat43 Mat;
    Mat.x.v = _mm_setr_ps(axis[0][0], axis[0][1], axis[0][2], 0.0f);
    Mat.y.v = _mm_setr_ps(axis[1][0], axis[1][1], axis[1][2], 0.0f);
    Mat.z.v = _mm_setr_ps(axis[2][0], axis[2][1], axis[2][2], 0.0f);
    Mat.w.v = _mm_setr_ps(origin[0], origin[1], origin[2], 0.0f);
    return PlayEffect(pakId, id, Mat, DbLinkedHandle<DObjHandleDb, DObj>(),
                      DbLinkedHandle<EntityHandleDb, Entity>(), -1, false);
}

// ea: 0x006D3BA0
ParticleEffect* FX_PlaySimpleEffectID(TPakId pakId, int id,
                                      const math::Position3& org)
{
    return PlayEffect(pakId, id, org.v.m128_f32);
}

// ea: 0x006D3BB0
ParticleEffect* FX_PlayEffectID(TPakId pakId, int id,
                                const math::Position3& org,
                                const float* fwd)
{
    float axis[3];
    float up[3];
    float forward[3];
    forward[0] = fwd[0];
    forward[1] = fwd[1];
    forward[2] = fwd[2];
    MakeNormalVectors(forward, axis, up);
    return PlayEffect(pakId, id, org.v.m128_f32, &axis);
}

// ============================================================================
// FX_GetBoneIndex / FX_GetBoneOrientation / FX_PlayEntityEffectID
// ============================================================================
class DObj {
public:
    uint8_t _pad0[0xCF];
    unsigned char numBones;  // +0xCF
};

class DObjHandleDb {
public:
    struct DbElement {
        DObj* mObject;  // +0x00
        int   mKey;     // +0x04
    };
    uint8_t mFreeIndices[168];   // +0x00 (BitSet<1344>)
    DbElement mElements[1344];   // +0xA8
};
DObjHandleDb* DObjHandleDb_SInst();  // tr_dobj2.cpp (owns ?sInst@DObjHandleDb@@0V1@A)

struct orientation_t {
    float origin[3];   // +0x00
    float axis[3][3];  // +0x0C
};
struct cvar_t {
    uint8_t _pad[0x20];
    int integer;       // +0x20
};
struct DObjSkelMat;
extern cvar_t* fx_debugBolt;  // ?fx_debugBolt@@3PAUcvar_t@@A @ 0x12FC6B8 (cl.o)
extern int DObjGetBoneIndex(const DObj* obj, unsigned int boneNameHash);  // render.o
extern bool FX_GetBoneOrientation(DbLinkedHandle<DObjHandleDb, DObj> boltObjHandle,
                                  int boltBoneIndex,
                                  orientation_t* pOrient);  // render.o 0x6D79F0
extern void CG_GetDObjOrientation(DObj* dobj, float* origin_out,
                                  float (*axis_out)[3]);  // cg_dobj.cpp
extern void AxisCopy(const float (*const in)[3], float (*const out)[3]);  // q_shared
extern void CG_DObjCalcBoneGeneric(DObj* obj, int boneIndex);  // cg.o
extern DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex);  // render.o
extern void CL_AddDebugLine(const float* start, const float* end,
                            const float* color, int a4, int a5, int a6,
                            int a7);  // cl.o
extern const float* const colorRed;    // ?colorRed@@3QBMB
extern const float* const colorGreen;  // ?colorGreen@@3QBMB
extern const float* const colorBlue;   // ?colorBlue@@3QBMB
struct vm_s;
extern vm_s* cgvm;                     // ?cgvm@@3PAUvm_s@@A (cl.o)

cvar_t* fx_debugBolt = nullptr;

// ea: 0x006D79D0
int FX_GetBoneIndex(const DObj* dobj, unsigned int bone_name_hash)
{
    if (dobj != nullptr)
        return DObjGetBoneIndex(dobj, bone_name_hash);
    return -1;
}

// ea: 0x006D79F0
bool FX_GetBoneOrientation(DbLinkedHandle<DObjHandleDb, DObj> boltObjHandle,
                           int boltBoneIndex, orientation_t* pOrient)
{
    if (boltObjHandle.mHandle.mVal == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
        AeAssert::gCurrentLine = 1883;
        AeAssert::gCurrentExpr = "boltObjHandle != TDObjHandle::NullHandle()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pOrient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
        AeAssert::gCurrentLine = 1884;
        AeAssert::gCurrentExpr = "pOrient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (cgvm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
        AeAssert::gCurrentLine = 1885;
        AeAssert::gCurrentExpr = "cgvm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned int v3 = boltObjHandle.mHandle.mVal & 0xFFF;
    if (v3 >= 0x540)
        return false;
    if (boltObjHandle.mHandle.mVal >> 12
        != DObjHandleDb_SInst()->mElements[v3].mKey)
    {
        return false;
    }
    DObj* mObject = DObjHandleDb_SInst()->mElements[v3].mObject;
    if (mObject == nullptr)
        return false;
    float origin[3];
    float axis[3][3];
    CG_GetDObjOrientation(mObject, origin, axis);
    if ((_fpclass(origin[0]) & 0x297) != 0
        || (_fpclass(origin[1]) & 0x297) != 0
        || (_fpclass(origin[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
        AeAssert::gCurrentLine = 1899;
        AeAssert::gCurrentExpr = "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    for (int r = 0; r < 3; ++r)
    {
        if ((_fpclass(axis[r][0]) & 0x297) != 0
            || (_fpclass(axis[r][1]) & 0x297) != 0
            || (_fpclass(axis[r][2]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
            AeAssert::gCurrentLine = 1900 + r;
            AeAssert::gCurrentExpr = "!IS_NAN((axis[0])[0]) && !IS_NAN((axis[0])[1]) && !IS_NAN((axis[0])[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
    }
    if (boltBoneIndex < 0)
    {
        pOrient->origin[0] = origin[0];
        pOrient->origin[1] = origin[1];
        pOrient->origin[2] = origin[2];
        AxisCopy(axis, pOrient->axis);
        return true;
    }
    if (boltBoneIndex >= mObject->numBones)
        return false;
    CG_DObjCalcBoneGeneric(mObject, boltBoneIndex);
    DObjSkelMat* MatrixArray = DObjGetMatrixArray(mObject, 0);
    if (MatrixArray == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
        AeAssert::gCurrentLine = 1923;
        AeAssert::gCurrentExpr = "mtxArray";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    const float* m = (const float*)((const char*)MatrixArray
                                    + boltBoneIndex * 64);  // DObjSkelMat is 64B
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
            pOrient->axis[j][i] = (m[j * 4 + 0] * axis[0][i])
                                + (m[j * 4 + 1] * axis[1][i])
                                + (m[j * 4 + 2] * axis[2][i]);
        pOrient->origin[i] = (m[12] * axis[0][i]) + (m[13] * axis[1][i])
                           + (m[14] * axis[2][i]) + origin[i];
    }
    for (int c = 0; c < 3; ++c)
    {
        if ((_fpclass(pOrient->origin[c]) & 0x297) != 0
            || (_fpclass(pOrient->axis[0][c]) & 0x297) != 0
            || (_fpclass(pOrient->axis[1][c]) & 0x297) != 0
            || (_fpclass(pOrient->axis[2][c]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ParticleEffect.cpp";
            AeAssert::gCurrentLine = 1941;
            AeAssert::gCurrentExpr = "!IS_NAN((pOrient->origin)[0]) && !IS_NAN((pOrient->origin)[1]) && !IS_NAN((pOrient->origin)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
    }
    int integer = fx_debugBolt->integer;
    if (integer != 0)
    {
        float end[3];
        end[0] = (integer * pOrient->axis[0][0]) + pOrient->origin[0];
        end[1] = (integer * pOrient->axis[0][1]) + pOrient->origin[1];
        end[2] = (integer * pOrient->axis[0][2]) + pOrient->origin[2];
        CL_AddDebugLine(pOrient->origin, end, colorRed, 1, 0, 0, 0);
        end[0] = (fx_debugBolt->integer * pOrient->axis[1][0]) + pOrient->origin[0];
        end[1] = (fx_debugBolt->integer * pOrient->axis[1][1]) + pOrient->origin[1];
        end[2] = (fx_debugBolt->integer * pOrient->axis[1][2]) + pOrient->origin[2];
        CL_AddDebugLine(pOrient->origin, end, colorGreen, 1, 0, 0, 0);
        end[0] = (fx_debugBolt->integer * pOrient->axis[2][0]) + pOrient->origin[0];
        end[1] = (fx_debugBolt->integer * pOrient->axis[2][1]) + pOrient->origin[1];
        end[2] = (fx_debugBolt->integer * pOrient->axis[2][2]) + pOrient->origin[2];
        CL_AddDebugLine(pOrient->origin, end, colorBlue, 1, 0, 0, 0);
    }
    return true;
}

// ea: 0x006D8310
ParticleEffect* FX_PlayEntityEffectID(
    TPakId pakId, int id, const math::Position3& org,
    const float (*axis)[3],
    DbLinkedHandle<DObjHandleDb, DObj> boltObjHandle,
    DbLinkedHandle<EntityHandleDb, Entity> boltEntHandle, int boltBoneIndex)
{
    (void)axis;
    orientation_t ori;
    ori.origin[0] = org.v.m128_f32[0];
    ori.origin[1] = org.v.m128_f32[1];
    ori.origin[2] = org.v.m128_f32[2];
    memset(&ori.axis[0][0], 0, 36);
    if (boltObjHandle.mHandle.mVal != 0
        && !FX_GetBoneOrientation(boltObjHandle, boltBoneIndex, &ori))
    {
        return nullptr;
    }
    math::Mat43 Mat;
    Mat.x.v = _mm_setr_ps(ori.axis[0][0], ori.axis[0][1], ori.axis[0][2], 0.0f);
    Mat.y.v = _mm_setr_ps(ori.axis[1][0], ori.axis[1][1], ori.axis[1][2], 0.0f);
    Mat.z.v = _mm_setr_ps(ori.axis[2][0], ori.axis[2][1], ori.axis[2][2], 0.0f);
    Mat.w.v = _mm_setr_ps(ori.origin[0], ori.origin[1], ori.origin[2], 0.0f);
    return PlayEffect(pakId, id, Mat, boltObjHandle, boltEntHandle,
                      boltBoneIndex, boltBoneIndex != 0);
}

// ============================================================================
// FX_BuildSortedParticleEffectList / RenderEffectsInternal / RenderEffects
// ============================================================================
namespace VFC {
struct FrustumInfo;
void GetFrustumInfo(FrustumInfo& out, const nglScene* Scene);  // ?GetFrustumInfo@VFC@@YAXAAUFrustumInfo@1@PBUnglScene@@@Z
}
class DebugRender {
public:
    static void RenderSphere(const math::Position3& pos, float radius,
                             const Color& color);  // renderdebug.cpp
    static void RenderLine(const math::Position3& pt1,
                           const math::Position3& pt2, const Color& col,
                           float thickness);  // renderdebug.cpp
    static void RenderText(const char* str, int x, int y, const Color& col,
                           float depth, float size);  // renderdebug.cpp
};
extern void nglValidateMatrices(nglScene* Scene);  // ngl.o
extern nglLightContext* nglCreateLightContext();   // ngl_lighting.cpp
extern void nglSetAmbientLight(float r, float g, float b);  // ngl_lighting.cpp
extern void profile_reset();  // ?profile_reset@@YAXXZ (cdDebugRender.cpp)
extern void FX_SortParticleEffectList(int indexLeft,
                                      int indexRight);  // render.o 0x6C7690
extern void FX_DumpParticleEffectList();               // tr_fx2.cpp 0x6C7620
extern int g_showCulledParticles;  // ?g_showCulledParticles@@3HA
bool isInit;                       // ?isInit@@3_NA @ 0xF0C748
int dword_10DDB14;                 // @ 0x10DDB14 (FX render gate)

// ea: 0x006D7710
void FX_BuildSortedParticleEffectList()
{
    if (!isInit)
    {
        isInit = true;
        gParticleEffectList.reserve(176);
        gSortedParticleEffectList.reserve(176);
    }
    ParticleEffect** mElements = gParticleEffectList.mElements;
    ParticleEffect** v2 =
        &gParticleEffectList.mElements[gParticleEffectList.mSize];
    gSortedParticleEffectList.resize(0);
    for (; mElements != v2; ++mElements)
    {
        ParticleEffect* v3 = *mElements;
        if (v3 != nullptr && v3->mEffect != nullptr)
            gSortedParticleEffectList.push_back(v3);
    }
    if (gSortedParticleEffectList.mSize != 0)
        FX_SortParticleEffectList(0, gSortedParticleEffectList.mSize - 1);
    FX_DumpParticleEffectList();
}

// ea: 0x006D7890
void RenderEffectsInternal()
{
    nglValidateMatrices(nglBuildScene);
    nglLightContext* LightContext = nglCreateLightContext();
    nglSetAmbientLight(0.40000001f, 0.40000001f, 0.40000001f);
    VFC::FrustumInfo frustumInfo;
    VFC::GetFrustumInfo(frustumInfo, nglBuildScene);
    ParticleEffect** mElements = gParticleEffectList.mElements;
    ParticleEffect** v2 =
        &gParticleEffectList.mElements[gParticleEffectList.mSize];
    if (gParticleEffectList.mElements != v2)
    {
        do
        {
            ParticleEffect* v3 = *mElements;
            if (v3 != nullptr && (v3->mFlags & 2) == 0)
            {
                if (v3->culled != 0)
                {
                    if (g_showCulledParticles != 0)
                    {
                        apsEffect* mEffect = v3->mEffect;
                        if (mEffect != nullptr)
                        {
                            math::Position3 pos;
                            pos.v = mEffect->mLToW.w.v;
                            DebugRender::RenderSphere(pos, 15.0f,
                                                      Color(0.0f, 1.0f, 0.0f,
                                                            1.0f));
                        }
                    }
                }
                else if ((dword_10DDB14 & 1) == 0)
                {
                    v3->mEffect->Render(LightContext, frustumInfo);
                }
            }
            ++mElements;
        } while (mElements != v2);
    }
}

// ea: 0x006D79C0 (thunk)
void RenderEffects()
{
    RenderEffectsInternal();
}

// ============================================================================
// FX_ClearFX / FX_InitFX / FX_ReportFX / FX_TermFX
// ============================================================================
extern int gParticleBatchGroup;  // ?gParticleBatchGroup@@3HA @ 0xF0C700
extern int jqCreateBatchGroup();  // jobqueue.o
extern void jqDestroyBatchGroup(int group);  // jobqueue.o
extern void mem_heap_free(void* ptr);  // core.o
void FX_ReportFX();  // forward

// ea: 0x006D77B0
void FX_ClearFX()
{
    apsCommon::ClearSpawnedEffectQueue();
    ParticleEffect** mElements = gParticleEffectList.mElements;
    ParticleEffect** v1 =
        &gParticleEffectList.mElements[gParticleEffectList.mSize];
    if (gParticleEffectList.mElements != v1)
    {
        do
            ParticleEffect::Delete(*mElements++);
        while (mElements != v1);
    }
    gParticleEffectList.resize(0);
    gSortedParticleEffectList.resize(0);
    if (gParticleEffectList.mElements != nullptr)
        tlMemFree(gParticleEffectList.mElements);
    gParticleEffectList.mElements = nullptr;
    gParticleEffectList.mCapacity = 0;
    gParticleEffectList.mSize = 0;
    if (gSortedParticleEffectList.mElements != nullptr)
        tlMemFree(gSortedParticleEffectList.mElements);
    gSortedParticleEffectList.mElements = nullptr;
    gSortedParticleEffectList.mCapacity = 0;
    gSortedParticleEffectList.mSize = 0;
}

// ea: 0x006D7850
void FX_InitFX()
{
    Com_Printf("^5----- Treyarch Fx System Initialization -----\n");
    ParticleEffect::InitArray();
    FX_ClearFX();
    FX_ReportFX();
    gParticleBatchGroup = jqCreateBatchGroup();
    Com_Printf("^5----- Fx System Initialization Complete -----\n");
}

// ea: 0x006C7B60
void FX_ReportFX()
{
    ParticleEffect** mElements = gSortedParticleEffectList.mElements;
    ParticleEffect** v1 =
        &gSortedParticleEffectList.mElements[gSortedParticleEffectList.mSize];
    if (gSortedParticleEffectList.mElements != v1)
    {
        do
        {
            apsEffect* mEffect = (*mElements)->mEffect;
            const char* mName;
            if (mEffect != nullptr)
                mName = mEffect->mTemplate->mName;
            else
                mName = "";
            unsigned int _32;
            if (mEffect != nullptr)
                _32 = mEffect->mSortKey._32;
            else
                _32 = 0xFFFFFFFFu;
            tlPrintf("0x%08x 0x%08x %s\n", *mElements++, _32, mName);
        } while (mElements != v1);
    }
    apsCommon::Report();
}

// ea: 0x006C3310
void FX_TermFX()
{
    Com_Printf("^5----- Treyarch Fx System Termination -----\n");
    if (ParticleEffect::sArrayData != nullptr)
    {
        for (int i = 0; i < 256; ++i)
            ParticleEffect::sArrayData[i].~ParticleEffect();
        mem_heap_free(&ParticleEffect::sArrayData[-1]);
        ParticleEffect::sArrayData = nullptr;
    }
    jqDestroyBatchGroup(gParticleBatchGroup);
    Com_Printf("^5----- Fx System Initialization Termination -----\n");
}

// ea: 0x006DAB90
struct cdl_proftimer {
    float value;       // +0x00
    uint8_t _pad[0x10 - 0x04];
};
extern cdl_proftimer cdl_proftimer_fx_all;     // tr_stats.cpp
extern cdl_proftimer cdl_proftimer_fx_update;  // tr_stats.cpp
extern cdl_proftimer cdl_proftimer_fx_render;  // tr_stats.cpp
extern int g_renderPFXStats;  // ?g_renderPFXStats@@3HA
struct EntityView {
    uint8_t _pad[0x70];
    math::Position3 currentOrigin;  // +0x70
};
class EntityHandleDb {
public:
    struct DbElement {
        Entity* mObject;  // +0x00
        int     mKey;     // +0x04
    };
    uint8_t mFreeIndices[168];   // +0x00 (BitSet<1344>)
    DbElement mElements[1344];   // +0xA8
    static EntityHandleDb sInst; // ?sInst@EntityHandleDb@@2V1@A (g.o)
};

void fx_debug_render()
{
    if (g_renderPFXStats == 0)
        return;
    char buf[512];
    char v20[12];
    sprintf(v20, "%i paticle effects", gParticleEffectList.mSize);
    Color white(1.0f, 1.0f, 1.0f, 1.0f);
    DebugRender::RenderText(v20, 450, 240, white, 0.0f, 1.0f);
    sprintf(v20, "%5.2f total", cdl_proftimer_fx_all.value * 0.0000013636364f);
    DebugRender::RenderText(v20, 450, 260, white, 0.0f, 1.0f);
    sprintf(v20, "%5.2f update",
            cdl_proftimer_fx_update.value * 0.0000013636364f);
    DebugRender::RenderText(v20, 450, 280, white, 0.0f, 1.0f);
    sprintf(v20, "%5.2f render",
            cdl_proftimer_fx_render.value * 0.0000013636364f);
    DebugRender::RenderText(v20, 450, 300, white, 0.0f, 1.0f);
    profile_reset();

    ParticleEffect** mElements = gParticleEffectList.mElements;
    ParticleEffect** end =
        &gParticleEffectList.mElements[gParticleEffectList.mSize];
    if (gParticleEffectList.mElements == end)
        return;
    Color yellow(1.0f, 1.0f, 0.0f, 1.0f);
    Color cyan(1.0f, 0.0f, 0.0f, 1.0f);
    do
    {
        ParticleEffect* v4 = *mElements;
        DObj* mObject = nullptr;
        unsigned int mVal = (unsigned int)v4->mDObjHandle;
        unsigned int v6 = mVal & 0xFFF;
        if (v6 < 0x540
            && mVal >> 12
                   == (unsigned int)DObjHandleDb_SInst()->mElements[v6].mKey)
        {
            mObject = DObjHandleDb_SInst()->mElements[v6].mObject;
        }
        EntityView* v2 = nullptr;
        unsigned int entVal = (unsigned int)v4->mEntHandle;
        unsigned int v7 = entVal & 0xFFF;
        if (v7 < 0x540
            && entVal >> 12
                   == (unsigned int)EntityHandleDb::sInst.mElements[v7].mKey)
        {
            v2 = (EntityView*)EntityHandleDb::sInst.mElements[v7].mObject;
        }
        math::Position3 pos;
        if (mObject != nullptr || v2 != nullptr || v4->mPoPtr != nullptr)
        {
            if (v4->mPoPtr != nullptr)
            {
                pos.v = v4->mPoPtr->w.v;
            }
            else if (v4->mBoneIndex < 0)
            {
                if (v2 != nullptr)
                    pos.v = v2->currentOrigin.v;
            }
            else
            {
                DbLinkedHandle<DObjHandleDb, DObj> bh;
                bh.mHandle.mVal = (unsigned int)v4->mDObjHandle;
                if (FX_GetBoneOrientation(bh, v4->mBoneIndex,
                                          (orientation_t*)(buf + 500)))
                {
                    pos.v = _mm_setr_ps(*(float*)(buf + 500),
                                        *(float*)(buf + 504),
                                        *(float*)(buf + 508), 0.0f);
                }
            }
        }
        math::Position3 top = pos;
        top.v.m128_f32[2] += 3000.0f;
        DebugRender::RenderLine(pos, top, yellow, 5.0f);
        DebugRender::RenderSphere(pos, 11.25f, cyan);
        ++mElements;
    } while (mElements != end);
}

// ea: 0x006D3650
void RemoveDeadEffects()
{
    ParticleEffect** mElements = gParticleEffectList.mElements;
    bool v1 = false;
    ParticleEffect** end = &gParticleEffectList.mElements[gParticleEffectList.mSize];
    if (gParticleEffectList.mElements == end)
        return;
    do
    {
        ParticleEffect* v2 = *mElements;
        if (v2 == nullptr)
            goto LABEL_10;
        apsEffect* mEffect = v2->mEffect;
        bool v4 = mEffect != nullptr && mEffect->IsDone() == 0;
        if ((v2->mFlags & 0x20) != 0 || !v4)
        {
            *mElements = nullptr;
            ParticleEffect::Delete(v2);
        LABEL_10:
            v1 = true;
        }
        ++mElements;
    } while (mElements != end);
    if (v1)
    {
        ParticleEffect** v5 = gParticleEffectList.mElements;
        ParticleEffect** v6 = &gParticleEffectList.mElements[gParticleEffectList.mSize];
        if (gParticleEffectList.mElements != v6)
        {
            while (*v5 != nullptr)
            {
                if (++v5 == v6)
                    goto LABEL_23;
            }
            if (v5 != v6)
            {
                ParticleEffect** i = v5;
                for (ParticleEffect** v7 = v5 + 1; v7 != v6; ++v7)
                {
                    if (*v7 != nullptr)
                        *i++ = *v7;
                }
                v5 = i;
            }
        }
    LABEL_23:
        gParticleEffectList.erase(v5,
                                  &gParticleEffectList.mElements[gParticleEffectList.mSize]);
    }
}

// ea: 0x006D3C10
LightEffect* AddLight(TPakId pakId, LightEffect::eType type,
                      const math::Position3& pos, LightEffect::eTime time)
{
    float v4;
    if (time == LightEffect::FOREVER)
        v4 = -1000.0f;
    else
    {
        v4 = 30.0f;
        if (time != LightEffect::FLASH)
            v4 = (float)time;
    }
    LightEffect* fx = (LightEffect*)LightEffect::sAllocator->Allocate(0x70u, false);
    if (fx != nullptr)
    {
        fx->mMSecLifetime = v4;
        fx->mMSecLifeOrig = v4;
        fx->mInnerRadius = 0.0f;
        fx->mOuterRadius = 0.0f;
        fx->mType = type;
        fx->mFade = false;
        fx->mKill = false;
        fx->mScale = 1.0f;
    }
    if (fx != nullptr)
    {
        fx->mPakId = pakId;
        fx->mFlicker = false;
        fx->mActive = true;
        fx->mKill = false;
        fx->mInnerRadius = gNearLightRadius;
        fx->mOuterRadius = gFarLightRadius;
        fx->mLightPos.v = pos.v;
        gLightEffectList.push_back(fx);
    }
    return fx;
}

// ea: 0x006D3CF0
struct PakFile;
class PakManager {
public:
    static PakManager* sInst;  // ?sInst@PakManager@@2PAV1@A
    uint8_t _pad[0x40];
    PakFile* mSlots[99];       // +0x40
};
extern void nglListAddPointLight(unsigned int LightCat,
                                 const math::Position3* Pos, float Near,
                                 float Far, const math::Vector4* Color,
                                 bool isVertexPointLight);  // ngl_lighting.cpp

void UpdateLights(float timeDeltaMS)
{
    LightEffect** mElements = gLightEffectList.mElements;
    bool v18 = false;
    LightEffect** end = &gLightEffectList.mElements[gLightEffectList.mSize];
    if (gLightEffectList.mElements == end)
        return;
    do
    {
        LightEffect* v3 = *mElements;
        TPakId mPakId = (TPakId)v3->mPakId;
        if (mPakId != PAK_ID_INVALID
            && PakManager::sInst->mSlots[mPakId] != nullptr
            && !v3->mKill
            && (v3->mMSecLifetime == -1000.0f || v3->mMSecLifetime >= 0.0f)
            && v3->mActive)
        {
            math::Position3 pos;
            pos.v = v3->mLightPos.v;
            math::Vector4 color;
            color.v.m128_f32[0] = v3->mColor[0] * v3->mScale;
            color.v.m128_f32[1] = v3->mColor[1] * v3->mScale;
            color.v.m128_f32[2] = v3->mColor[2] * v3->mScale;
            color.v.m128_f32[3] = v3->mColor[3] * v3->mScale;
            if (v3->mType == LightEffect::VERTEX_LIGHT)
            {
                nglListAddPointLight(0x40000000u, &pos, v3->mInnerRadius,
                                     v3->mOuterRadius, &color, true);
            }
            else
            {
                color.v.m128_f32[0] = 1.0f;
                color.v.m128_f32[1] = 0.8f;
                color.v.m128_f32[2] = 0.2f;
                nglListAddPointLight(0x40000000u, &pos, v3->mInnerRadius,
                                     220.0f, &color, false);
            }
            if (v3->mMSecLifetime != -1000.0f)
            {
                float v8 = v3->mMSecLifetime - timeDeltaMS;
                v3->mMSecLifetime = v8;
                if (v8 > 0.0f)
                {
                    if (v3->mFade)
                    {
                        float v9 = v8 / v3->mMSecLifeOrig;
                        v3->mColor[0] = v3->mColorOriginal[0] * v9;
                        v3->mColor[1] = v3->mColorOriginal[1] * v9;
                        v3->mColor[2] = v3->mColorOriginal[2] * v9;
                    }
                }
                else
                {
                    v3->mActive = false;
                    v3->mMSecLifetime = 0.0f;
                }
            }
            if (v3->mFlicker)
            {
                if (rand() % 3 == 0)
                    v3->mFlickerRatio = (rand() * 0.000012207031f) + 0.60000002f;
                v3->mColor[0] = v3->mColorOriginal[0] * v3->mFlickerRatio;
                v3->mColor[1] = v3->mColorOriginal[1] * v3->mFlickerRatio;
                v3->mColor[2] = v3->mColorOriginal[2] * v3->mFlickerRatio;
            }
        }
        else
        {
            *mElements = nullptr;
            LightEffect::sAllocator->Release(v3);
            v18 = true;
        }
        ++mElements;
    } while (mElements != end);
    if (!v18)
        return;
    {
        LightEffect** v12 = gLightEffectList.mElements;
        LightEffect** v11 = &gLightEffectList.mElements[gLightEffectList.mSize];
        if (gLightEffectList.mElements != v11)
        {
            while (*v12 != nullptr)
            {
                if (++v12 == v11)
                    goto LABEL_33;
            }
            if (v12 != v11)
            {
                LightEffect** i = v12;
                for (LightEffect** v13 = v12 + 1; v13 != v11; ++v13)
                {
                    if (*v13 != nullptr)
                    {
                        *i = *v13;
                        ++i;
                    }
                }
                v12 = i;
            }
        }
    LABEL_33:
        gLightEffectList.erase(v12,
                               &gLightEffectList.mElements[gLightEffectList.mSize]);
    }
}
