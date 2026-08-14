// ============================================================================
// tr_fx2.cpp - render.o aps client / particle / light effect helpers
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "aeps/apsInternal.h"
#include "aeps/apsMemory.h"
#include "aeps/apsCommon.h"
#include "aeps/apsEffect.h"

#include <stdint.h>

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
    static void DestroyArray();  // ?DestroyArray@ParticleEffect@@SAXXZ
    static ParticleEffect* sArrayData;  // ?sArrayData@ParticleEffect@@2PAV1@A @ 0xF74460
};
static_assert(sizeof(ParticleEffect) == 0x3C, "ParticleEffect size mismatch");

ParticleEffect* ParticleEffect::sArrayData;

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
