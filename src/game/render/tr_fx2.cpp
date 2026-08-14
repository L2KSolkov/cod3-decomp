// ============================================================================
// tr_fx2.cpp - render.o aps client / particle / light effect helpers
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "aeps/apsInternal.h"
#include "aeps/apsMemory.h"
#include "aeps/apsCommon.h"
#include "aeps/apsEffect.h"
#include "core/ae_array.h"

#include <stdint.h>

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
};

enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };

// mem heap helpers (core.o)
void* mem_heap_malloc(unsigned int size);
void* mem_heap_malloc(int alignment, unsigned int size);
void mem_heap_free(void* ptr);

// tlMemFree (tl library)
void tlMemFree(void* Ptr);

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
class LightEffect {
public:
    enum eType {
        PROJECTED_TEXTURE = 0x0,
        VERTEX_LIGHT = 0x1,
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
