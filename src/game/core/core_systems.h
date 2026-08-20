// ============================================================================
// COD3 Core Systems - effect events, rumble, DB query, configstring, notify
// Reconstructed from IDA local types (PDB symbol data) via ida-pro-mcp.
// All sizes and offsets verified against IDA unless marked TODO.
// ============================================================================

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "core/ae_array.h"
#include "core/ae_fixed_string.h"
#include "core/math_types.h"
#include "core/tlFixedString.h"
#include "engine/broc_types.h"
#include "game/game_types.h"
#include "game/core/core_types.h"

// TPakId is defined fully in game/sv/sv_stubs.h; forward-declare the enum so
// this header stays standalone (C++11 allows enum : int forward decls).
enum TPakId : int;

// ============================================================================
// Enum placeholders - values must be fetched from IDA when porting bodies.
// Typedef'd as int to keep ABI size (4 bytes) without guessing values.
// ============================================================================
typedef int EEffectContext;      // TODO: enum values from IDA
// Enum tags match binary manglings (W4E*); values reconstructed.
enum ECollisionMaterial : int {
    kCollisionMaterialMin = 0,
    kCollisionMaterialASPHALT = 1,
    kCollisionMaterialFLESH = 4,
};
enum EStanceType : int { kStanceStand = 0, kStanceCrouch = 1, kStanceProne = 2 };
enum EWeaponClass : int { kWeaponClassNone = 0, kWeaponClassBullet = 1 };
enum EAction : int { kActionNone = 0, kActionPrimary = 1, kActionSecondary = 2 };
enum EUserBoneId : int { kUserBoneIdMin = 0 };  // TODO: enum values from IDA
enum nslWaveID : int;            // TODO: enum values from IDA

struct ParticleEffect;
struct apsEffect;
struct gdLight;
struct LightEffect;
struct CameraShakeInstance;
struct EndOnScriptNode;
struct DbStringHashTable;
class DbTable;
struct DbQuery;
class EntityHandleDb;
class DbRow;
class DbGraphNode;
class DbQueryResults;
class DbTableSet;
class StringTableEntry;
class EffectEventSys;

// ============================================================================
// Bitmask<T> - typed flag word (sizeof(T) bytes)
// ============================================================================
template <typename T>
struct Bitmask {
    T mVal;  // +0x00
    Bitmask() : mVal(0) {}
    explicit Bitmask(T v) : mVal(v) {}

    // ?Test@?$Bitmask@I@@QBE_NH@Z (g.o 0x4ACD50)
    bool Test(int b) const { return ((1 << b) & mVal) != 0; }
    // ?IsEmpty@?$Bitmask@I@@QBE_NXZ (g.o 0x4ACDE0)
    bool IsEmpty() const { return mVal == 0; }
    // ?Clear@?$Bitmask@I@@QAEXXZ (g.o 0x4ACDF0)
    void Clear() { mVal = 0; }
};

// ============================================================================
// BitSet<N> - N-bit set packed into bytes
// ============================================================================
template <int N>
struct BitSet {
    unsigned char mBits[(N + 7) / 8];  // +0x00

    enum EInitializer : int32_t {
        UNINITIALIZED = 0x0,
    };

    BitSet() { memset(mBits, 0, sizeof(mBits)); }
    BitSet(EInitializer) {}  // ??0?$BitSet@$0FEA@@@QAE@W4EInitializer@0@@Z (g.o 0x4AE780)

    // ?GetWord@?$BitSet@$0FEA@@@QBEIH@Z / ?GetNumWords@?$BitSet@$0FEA@@@SAHXZ (g.o)
    unsigned int GetWord(int idx) const { return ((unsigned int*)mBits)[idx]; }
    static int GetNumWords() { return (N + 31) / 32; }
    void Clear();  // ?Clear@?$BitSet@$0FEA@@@QAEXXZ (g.o 0x4AE790)
    void Add(int v);  // ?Add@?$BitSet@$0FEA@@@QAEXH@Z (g.o 0x4B1750)
    void Rmv(int v);  // ?Rmv@?$BitSet@$0FEA@@@QAEXH@Z (g.o 0x4B17E0)
    BitSet<N> operator~() const;  // ??S?$BitSet@$0FEA@@@QBE?AV0@XZ (g.o 0x4B1870)

    class iterator {
    public:
        BitSet<N>* m_src;        // +0x00
        int m_word_idx;          // +0x04
        unsigned int m_cur_val;  // +0x08
        unsigned int m_cur_word; // +0x0C

        iterator() : m_src(nullptr), m_word_idx(0), m_cur_val(0), m_cur_word(0) {}
        iterator(const BitSet<N>* src)  // ??0iterator@?$BitSet@$0FEA@@@QAE@ABV1@@Z (g.o 0x4B1970)
        {
            m_src = (BitSet<N>*)src;
            m_cur_word = ((const unsigned int*)src->mBits)[0];
            m_word_idx = 0;
            m_cur_val = (unsigned int)-1;
            operator++();
        }
        bool compare(const iterator& rhs)  // ?compare@iterator@?$BitSet@$0FEA@@@QAE_NABV12@@Z (g.o 0x4AE5A0)
        {
            return m_cur_val == rhs.m_cur_val && m_word_idx == rhs.m_word_idx;
        }
        bool operator!=(const iterator& rhs)  // ??9iterator@?$BitSet@$0FEA@@@QAE_NABV01@@Z (g.o 0x4B1400)
        {
            return m_cur_val != rhs.m_cur_val || m_word_idx != rhs.m_word_idx;
        }
        void operator++()  // ??Eiterator@?$BitSet@$0FEA@@@QAEXXZ
        {
            while (m_word_idx < GetNumWords())
            {
                if (m_cur_word != 0)
                {
                    unsigned long idx;
                    _BitScanForward(&idx, m_cur_word);
                    m_cur_val = m_word_idx * 32 + (int)idx;
                    m_cur_word &= m_cur_word - 1;
                    return;
                }
                ++m_word_idx;
                if (m_word_idx < GetNumWords())
                    m_cur_word = ((const unsigned int*)m_src->mBits)[m_word_idx];
            }
            m_cur_val = (unsigned int)-1;
            m_word_idx = -1;
        }
    };

    iterator begin() const  // ?begin@?$BitSet@$0FEA@@@QBE?AViterator@1@XZ (g.o 0x4B2630)
    {
        iterator it(this);
        return it;
    }
    iterator end() const  // ?end@?$BitSet@$0FEA@@@QBE?AViterator@1@XZ (g.o 0x4B0EC0)
    {
        iterator it;
        it.m_src = nullptr;
        it.m_word_idx = -1;
        it.m_cur_val = (unsigned int)-1;
        it.m_cur_word = 0;
        return it;
    }
};

// ============================================================================
// reserved_dlist<T> - intrusive doubly-linked list (16 bytes)
// The dlist_node type is 8 bytes (prev/next). The 8-byte tail is reserved
// (likely head/tail links or count) - TODO verify when porting list code.
// ============================================================================
template <typename T>
struct reserved_dlist {
    struct dlist_node {
        dlist_node* mNext;  // +0x00
        dlist_node* mPrev;  // +0x04

        dlist_node() : mNext(nullptr), mPrev(nullptr) {}
        dlist_node(dlist_node* prev, dlist_node* next)
            : mNext(next), mPrev(prev) {}  // ??0dlist_node@?$reserved_dlist@VWaitTilOutput@@@@QAE@PAU01@0@Z (g.o 0x4AE510)
    };
    int         m_size;  // +0x00
    dlist_node* m_head;  // +0x04
    dlist_node* m_end;   // +0x08
    dlist_node* m_tail;  // +0x0C

    void validate() const;  // ?validate@?$reserved_dlist@VEntityNotify@@@@QBEXXZ (g.o 0x4AE530)
    void push_back(T* obj);  // ?push_back@?$reserved_dlist@VEntityNotify@@@@QAEXPAVEntityNotify@@@Z (g.o 0x4B12D0)
};
static_assert(sizeof(reserved_dlist<int>) == 0x10,
              "reserved_dlist size mismatch");

template <typename T>
void reserved_dlist<T>::validate() const
{
}

template <typename T>
void reserved_dlist<T>::push_back(T* obj)
{
    dlist_node* node = (dlist_node*)obj;
    node->mNext = m_end;
    node->mPrev = m_tail;
    m_tail->mNext = node;
    m_tail = node;
    ++m_size;
}

template <int N>
void BitSet<N>::Clear()
{
    for (int i = GetNumWords() - 1; i >= 0; --i)
        ((unsigned int*)mBits)[i] = 0;
}

template <int N>
void BitSet<N>::Add(int v)
{
    if ((v >> 5) >= GetNumWords())
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/BitSet.h";
        AeAssert::gCurrentLine = 99;
        AeAssert::gCurrentExpr = "idx < GetNumWords()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Please add a descriptive string"))
            __debugbreak();
    }
    ((unsigned int*)mBits)[v >> 5] |= 1u << (v & 0x1F);
}

template <int N>
void BitSet<N>::Rmv(int v)
{
    if ((v >> 5) >= GetNumWords())
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/BitSet.h";
        AeAssert::gCurrentLine = 107;
        AeAssert::gCurrentExpr = "idx < GetNumWords()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Please add a descriptive string"))
            __debugbreak();
    }
    ((unsigned int*)mBits)[v >> 5] &= ~(1u << (v & 0x1F));
}

template <int N>
BitSet<N> BitSet<N>::operator~() const
{
    BitSet<N> r;
    for (int i = 0; i < GetNumWords(); ++i)
        ((unsigned int*)r.mBits)[i] = ~((const unsigned int*)mBits)[i];
    return r;
}


// ============================================================================
// SimpleCollisionDesc - impact point + normal (32 bytes)
// Size: 0x20 (32 bytes) - verified against IDA
// ============================================================================
struct SimpleCollisionDesc {
    math::Position3 coord;   // +0x00
    math::Position3 normal;  // +0x10
};
static_assert(sizeof(SimpleCollisionDesc) == 0x20,
              "SimpleCollisionDesc size mismatch");

// ============================================================================
// CollisionDesc - collision description (48 bytes)
// Size: 0x30 (48 bytes) - verified against IDA
// ============================================================================
struct CollisionDesc {
    SimpleCollisionDesc simple;  // +0x00
    ECollisionMaterial material; // +0x20
};
static_assert(sizeof(CollisionDesc) == 0x30, "CollisionDesc size mismatch");

// ============================================================================
// SoundParams - effect sound parameters (56 bytes)
// Size: 0x38 (56 bytes) - verified against IDA
// ============================================================================
class SoundParams {
public:
    TPakId                       mPakId;        // +0x00
    DbLinkedHandle<void, void>   mEnt;          // +0x04
    int                          mFlags;        // +0x08
    float                        mDelayTrigger; // +0x0C
    const char*                  mNameRef;      // +0x10
    CollisionDesc*               mCd;           // +0x14
    float                        mDuration;     // +0x18
    float                        mVolume;       // +0x1C
    float                        mPitch;        // +0x20
    int                          mMaxVoices;    // +0x24
    int                          mQueue;        // +0x28
    int                          mHashStr;      // +0x2C
    int                          mDialogNotify; // +0x30
    const char*                  mSubtitle;     // +0x34
};
static_assert(sizeof(SoundParams) == 0x38, "SoundParams size mismatch");
static_assert(offsetof(SoundParams, mDuration) == 0x18,
              "SoundParams::mDuration offset mismatch");

// ============================================================================
// AbstractEffect - effect base (52 bytes)
// Size: 0x34 (52 bytes) - verified against IDA
// ============================================================================
class AbstractEffect {
public:
    virtual ~AbstractEffect();
    AbstractEffect() { memset(this, 0, sizeof(AbstractEffect)); }
    AbstractEffect(TPakId pak_id, DbLinkedHandle<EntityHandleDb, Entity> ent, int flags,
                   float delay_trigger);  // ea: 0x004C1240
    virtual void SetPoPtr(math::Mat43* po);
    virtual bool IsQueued() const;
    virtual bool IsLooping() const;
    virtual void AdjustEffect_Scale(const char* param, float scale);
    virtual void FastForward(float deltaT);
    virtual void PlayQueuedEffect();
    virtual void StartFadeOut(float time);
    virtual void FrameAdvance(float deltaT);
    virtual bool IsFinished();
    virtual void StopEffect();
    virtual Broc::string GetDebugString() const;
    math::Position3 GetPosition() const;        // ea: 0x004CC230
    Broc::string GetEntityDebugString() const;  // ea: 0x004CC330
    bool IsFinishedFading();                    // ea: 0x004E2D40
    Broc::string                 mEffectName;        // +0x04
    unsigned int                 mEffectNameHashStr; // +0x08
    TPakId                       mPakId;             // +0x0C
    DbLinkedHandle<EntityHandleDb, Entity> mEntity;  // +0x10
    int                          mFlags;             // +0x14
    unsigned int                 mType;              // +0x18
    const math::Mat43*           mPoPtr;             // +0x1C
    float                        mFadeStart;         // +0x20
    float                        mFadeTime;          // +0x24
    float                        mDelayTrigger;      // +0x28
    float                        mDelayCount;        // +0x2C
    int16_t                      mCountSinceStarted; // +0x30
    Bitmask<uint16_t>            mCodeFlags;         // +0x32
};
static_assert(sizeof(AbstractEffect) == 0x34, "AbstractEffect size mismatch");
static_assert(offsetof(AbstractEffect, mPoPtr) == 0x1C,
              "AbstractEffect::mPoPtr offset mismatch");

// ============================================================================
// AbstractEffectSound - sound effect (160 bytes)
// Size: 0xA0 (160 bytes) - verified against IDA
// ============================================================================
struct AbstractEffectSound : AbstractEffect {
    unsigned char _pad[0x40 - 0x34];
    math::Position3 mCdPos;    // +0x40
    SoundParams     mSoundParams;  // +0x50
    unsigned int    mFlags;    // +0x88
    float           mFinalPitch;   // +0x8C
    uint16_t        mRetryCount;   // +0x90
    unsigned char   _pad2[0x94 - 0x92];
    const char*     mSubtitle;     // +0x94
    DbLinkedHandle<void, void> mSound;  // +0x98 (SoundDevice::Sound handle)
    nslWaveID       mWaveHdl;      // +0x9C

    AbstractEffectSound(TPakId pakId, DbLinkedHandle<EntityHandleDb, Entity> ent,
                        int flags, float delayTrigger,
                        SoundParams& soundParams);  // ea: 0x004CF3E0
    ~AbstractEffectSound();  // ea: 0x004CC3F0
    void StartFadeOut(float seconds);  // ea: 0x004C12A0
    void SetPoPtr(math::Mat43* po);        // ea: 0x004CD050
    bool IsQueued() const;                // ea: 0x004CD0C0
    bool IsFinished();                    // ea: 0x004CD120
    bool IsLooping() const;               // ea: 0x004CD1B0
    void AdjustEffect_Scale(const char* param,
                            float scale); // ea: 0x004CD230
    void PlayQueuedEffect();              // ea: 0x004CD350
    void StopEffect();                    // ea: 0x004CD4C0
    Broc::string GetDebugString() const;  // ea: 0x004CD580
    void FrameAdvance(float delta_t);     // ea: 0x004CC4B0
};
static_assert(sizeof(AbstractEffectSound) == 0xA0,
              "AbstractEffectSound size mismatch");

// ============================================================================
// AbstractEffectParticle - particle effect (56 bytes)
// Size: 0x38 (56 bytes) - verified against IDA
// ============================================================================
struct AbstractEffectParticle : AbstractEffect {
    ParticleEffect* mParticle;  // +0x34

    AbstractEffectParticle();   // ea: 0x004CF600
    ~AbstractEffectParticle();  // ea: 0x004C12C0
    void SetPoPtr(math::Mat43* po);               // ea: 0x004BD200
    bool IsLooping() const;                       // ea: 0x004BD220
    void AdjustEffect_Scale(const char* param,
                            float scale);         // ea: 0x004BD230
    void FastForward(float deltaT);               // ea: 0x004BD280
    void StartFadeOut(float seconds);             // ea: 0x004C1350
    Broc::string GetDebugString() const;          // ea: 0x004C59C0
    bool IsFinished();                            // ea: 0x004CDC90
    void FrameAdvance(float delta_t);             // ea: 0x004CD740
};
static_assert(sizeof(AbstractEffectParticle) == 0x38,
              "AbstractEffectParticle size mismatch");

// ============================================================================
// ParticleEffect - active particle system instance (60 bytes)
// Size: 0x3C - verified against IDA
// ============================================================================
struct ParticleEffect {
    float      cached_pos[4];       // +0x00
    int        cached_cell_index;   // +0x10
    int        culled;              // +0x14
    unsigned int mIndex;            // +0x18 (ae_pair<short,short>)
    apsEffect* mEffect;             // +0x1C
    AbstractEffectParticle* mAbstractEffectParticle;  // +0x20
    math::Mat43* mPoPtr;            // +0x24
    DbLinkedHandle<void, void> mDObjHandle;  // +0x28
    DbLinkedHandle<void, void> mEntHandle;   // +0x2C
    int16_t    mBoneIndex;          // +0x30
    Bitmask<unsigned short> mFlags; // +0x32
    TPakId     mPakId;              // +0x34
    void*      mRaycastData;        // +0x38

};
static_assert(sizeof(ParticleEffect) == 0x3C, "ParticleEffect size mismatch");
static_assert(offsetof(ParticleEffect, mEffect) == 0x1C,
              "ParticleEffect::mEffect offset mismatch");

// ============================================================================
// AbstractEffectLight - dynamic light effect (68 bytes)
// Size: 0x44 (68 bytes) - verified against IDA
// ============================================================================
struct AbstractEffectLight : AbstractEffect {
    struct Params;
    gdLight*      mLight;     // +0x34
    LightEffect*  mProjLight; // +0x38
    LightEffect*  mVertLight; // +0x3C
    float         mTime;      // +0x40

    AbstractEffectLight(Params& params);  // ea: 0x004CDD20
    ~AbstractEffectLight();               // ea: 0x004BD2A0
    Broc::string GetDebugString() const;  // ea: 0x004BD300
    void FrameAdvance(float delta_t);     // ea: 0x004CDF50
    bool IsFinished();                    // ea: 0x004CE060
private:
    math::Position3 GetPositionOnEntity(Entity* e) const;  // ea: 0x004CDEF0
};
static_assert(sizeof(AbstractEffectLight) == 0x44,
              "AbstractEffectLight size mismatch");

// ============================================================================
// AbstractEffectShakeAndRumble - camera shake + rumble effect (112 bytes)
// Size: 0x70 (112 bytes) - verified against IDA
// ============================================================================
enum ERumbleMotorID;
class RumbleEffectInstanceHandle {
public:
    int mVal;  // +0x00
};
static_assert(sizeof(RumbleEffectInstanceHandle) == 0x4,
              "RumbleEffectInstanceHandle size mismatch");

struct AbstractEffectShakeAndRumble : AbstractEffect {
    struct Params;
    float mTime;              // +0x34
    float mFreq;              // +0x38
    float mMovement;          // +0x3C
    float mNextDelay;         // +0x40
    float mRumble;            // +0x44
    float mBlur;              // +0x48
    float mMinDist2;          // +0x4C
    float mMaxDist2;          // +0x50
    float mSteadyDuration;    // +0x54
    float mRampUpTime;        // +0x58
    float mRampDownTime;      // +0x5C
    bool  mUseHighFreqVibrator;  // +0x60
    bool  mRumbleEnabled;     // +0x61
    unsigned char _pad[0x64 - 0x62];
    EUserBoneId mBone;        // +0x64
    RumbleEffectInstanceHandle mRumbleHandle[1];  // +0x68
    CameraShakeInstance* mShake[1];               // +0x6C

    AbstractEffectShakeAndRumble(Params& params);  // ea: 0x004CF6F0
    AbstractEffectShakeAndRumble(
        TPakId pak_id, DbLinkedHandle<EntityHandleDb, Entity> ent, float delay_trigger,
        int flags, float time, float freq, float movement, float nextDelay,
        float rumble, float blur, float minDist, float maxDist,
        float steadyDuration, float rampUpTime, float rampDownTime,
        bool useHighFreqVib, bool rumbleEnabled,
        EUserBoneId bone);  // ea: 0x004CF890
    ~AbstractEffectShakeAndRumble();  // ea: 0x004CF9D0
    Broc::string GetDebugString() const;          // ea: 0x004BD370
    bool IsFinished();                            // ea: 0x004CE720
    void AdjustEffect_Scale(const char* param,
                            float scale);         // ea: 0x004CE780
    void StopEffect();                            // ea: 0x004CE7B0
    void FrameAdvance(float delta_t);             // ea: 0x004CE230
private:
    math::Position3 GetPositionOnEntity() const;  // ea: 0x004CE0B0
    float GetDistanceScale(int client);           // ea: 0x004CE110
};
static_assert(sizeof(AbstractEffectShakeAndRumble) == 0x70,
              "AbstractEffectShakeAndRumble size mismatch");

// ============================================================================
// ActiveEffectSet - set of active effects for one query (44 bytes)
// Size: 0x2C (44 bytes) - verified against IDA
// ============================================================================
class ActiveEffectSet {
public:
    ae_sized_array<AbstractEffect*, 6> mEffects;  // +0x00
    TPakId                             mPakId;    // +0x1C
    math::Mat43*                       mPoPtr;    // +0x20
    Handle                             mId;       // +0x24
    Bitmask<unsigned int>              mFlags;    // +0x28

    ActiveEffectSet(TPakId pak_id);                 // ea: 0x004D00E0
    ~ActiveEffectSet();
    void AddEffect(AbstractEffect* effect);             // ea: 0x004C0C00
    bool IsFinished() const;                            // ea: 0x004C0C30
    bool IsQueued() const;                              // ea: 0x004C0C50
    void FrameAdvance(float delta);                     // ea: 0x004CA660
    void AdjustEffect_Scale(const char* param,
                            float scale);               // ea: 0x004C0CD0
    void FastForward(float deltaT);                     // ea: 0x004C0D50
    void PlayQueuedEffect();                            // ea: 0x004C0DD0
    void SetPoPtr(math::Mat43* po);                     // ea: 0x004C0E40
    void StopLoopingEffects();                          // ea: 0x004C0EC0
    void GetDebugFxList(Entity* ent,
                        std::vector<std::string>& fx) const;  // ea: 0x004D3AD0
private:
    void DoStopLoopingEffects();                        // ea: 0x004C0ED0
};
static_assert(sizeof(ActiveEffectSet) == 0x2C, "ActiveEffectSet size mismatch");
static_assert(offsetof(ActiveEffectSet, mPakId) == 0x1C,
              "ActiveEffectSet::mPakId offset mismatch");

// HandleDb<ActiveEffectSet,512,SizedHandle<9,23>> (elements at +0x40)
struct HandleDb {
    struct Element {
        ActiveEffectSet* mObject;  // +0x00
        unsigned int     mKey;     // +0x04
    };
    unsigned char mFreeBits[0x40]; // +0x00 BitSet<512> free-index bitmap
    Element       mElements[512];  // +0x40
    unsigned int  _tail;           // +0x1040

    Handle AllocateHandle();                       // ea: 0x004E8D50
    void BindObjectToHandle(Handle handle,
                            ActiveEffectSet* obj); // ea: 0x004E3DB0
    void ReleaseHandle(Handle h);                  // ea: 0x004E6430
};
static_assert(sizeof(HandleDb) == 0x1044, "HandleDb size mismatch");

// ============================================================================
// EffectEventSys - effect event system singleton (41856 bytes)
// Size: 0xA380 - verified against IDA
// ============================================================================
class EffectEventSys {
public:
    static EffectEventSys* sInst;       // ?sInst@EffectEventSys@@2PAV1@A @ 0x00F00E80
    static EffectEventSys* CreateInst();  // ?CreateInst@EffectEventSys@@SAXXZ (core.o)
    static void DeleteInst();  // ?DeleteInst@EffectEventSys@@SAXXZ (core.o)
    struct CachedQuery {
        BitSet<49>    mSpecifiedFields;  // +0x000
        BitSet<49>    mWeakFields;       // +0x008
        char          mWEAPON_ID[128];   // +0x010 (DbQueryString)
        int           mMYMATERIAL;       // +0x090
        float         mMIN_DIST;         // +0x094
        float         mMAX_DIST;         // +0x098
        int           mBARREL;           // +0x09C
        int           mSTANCE;           // +0x0A0
        char          mVEHICLE_ID[128];  // +0x0A4 (DbQueryString)
        int           mACTION;           // +0x124
        int           mWEAPON_CLASS;     // +0x128
        int           mFOOTSTEP;         // +0x12C
        char          mSCRIPT_ID[128];   // +0x130 (DbQueryString)
        int           mCONTEXT;          // +0x1B0
        int           mMATERIAL;         // +0x1B4

        void ConstructQuery(DbQuery* query);  // ea: 0x004E8390
    };
    static_assert(sizeof(CachedQuery) == 0x1B8, "CachedQuery size mismatch");

    struct PendingQuery {
        EEffectContext mType;          // +0x000
        CachedQuery    mCachedQuery;   // +0x004
        CollisionDesc  mCollisionInfo; // +0x1C0
        DbLinkedHandle<EntityHandleDb, Entity> mQueryEnt;  // +0x1F0
        TPakId         mEffectsPak;    // +0x1F4
        Handle         mEffect;        // +0x1F8
        Broc::string   mScriptId;      // +0x1FC
        int            mDialogNotify;  // +0x200
        math::Mat43*   mMatrix;        // +0x204
        int            mBoneIndex;     // +0x208
        int            mQueryType;     // +0x20C
        Bitmask<uint16_t> mFlags;      // +0x210
        int16_t        mCacheSoundType;// +0x212
        unsigned char  _tail[0x220 - 0x214];  // TODO verify
    };
    static_assert(sizeof(PendingQuery) == 0x220, "PendingQuery size mismatch");

    struct EffectRef {
        unsigned int first;   // +0x00
        int          second;  // +0x04
    };
    static_assert(sizeof(EffectRef) == 0x8, "EffectRef size mismatch");

    ae_sized_array<ActiveEffectSet*, 512> mEffectSets;    // +0x0000 (0x804)
    ae_sized_array<AbstractEffect*, 128> mFadingEffects;  // +0x0804 (0x204)
    unsigned char _pad1[0xA10 - 0xA08];                   // 16-byte align gap
    ae_sized_array<PendingQuery, 64> mPendingQueries;     // +0x0A10 (0x8810)
    PendingQuery*       mCurrentQuery;   // +0x9220
    bool                mStoppingAll;    // +0x9224
    int                 mDebuggingLevel; // +0x9228
    ae_sized_array<EffectRef, 32> mEffectRefs;  // +0x922C (0x104)
    HandleDb            mHandleDb;          // +0x9330
    unsigned char       _tail[0xC];         // TODO verify (to 0xA380)

    EffectEventSys();                       // ea: 0x004D0130
    ~EffectEventSys();                      // ea: 0x004D01E0
    int NumberOfVoicesUsed();                    // ea: 0x004BCE20
    void TagNameIndexInfo(int index);            // ea: 0x004BCE60
    void SetEffectMatrix(math::Mat43* pMat);     // ea: 0x004BCE80
    void SetScriptId(Broc::string val);          // ea: 0x004BCEA0
    void SetDialogNotify(int notify);            // ea: 0x004BCF00
    void SetQueryType(unsigned int val);         // ea: 0x004BCF20
    void SoundCacheType(int val);                // ea: 0x004BCF40
    void IsSoundToBeQueued(bool val);            // ea: 0x004C11A0
    void SetQueryImportance(bool val);           // ea: 0x004C11C0
    void DirectionInfo(const float* dir);        // ea: 0x004C11F0
    void FadeOutEffect(AbstractEffect* effect,
                       float seconds);           // ea: 0x004C0FE0
    void SendSpecificSoundNotify(Entity* pEnt,
                                 HashString soundName);  // ea: 0x004BCD90
    void SendSoundNotify(Entity* pEnt);          // ea: 0x004BCDE0
    ActiveEffectSet* GetActiveEffectSet(Handle handle);  // ea: 0x004C56B0
    int IsSoundAlreadyPlaying(unsigned int mSoundNameHashStr, Entity* pEnt,
                              int maxEffects);   // ea: 0x004CA820
    bool IsEffectActive(Handle handle);          // ea: 0x004CACD0
    void AdjustEffect_Scale(Handle handle, const char* param,
                            float scale);        // ea: 0x004CABD0
    void FastForward(Handle handle, float deltaT);  // ea: 0x004CAC10
    void PlayQueuedEffect(Handle handle);        // ea: 0x004CAC50
    void StopLoopingEffects(Handle handle);      // ea: 0x004CAC90
    void ReleaseHandle(ActiveEffectSet* t);      // ea: 0x004CB840
    void ReleaseHandle(Handle h);                // ea: 0x004CB870
    void StopAll();                              // ea: 0x004CEDC0
    void StopEffect(Handle handle, bool kill);   // ea: 0x004CEF20
    void KillEffectsWithPakId(TPakId pak_id);    // ea: 0x004CF020
    void CollisionInfo(const CollisionDesc& col_desc,
                       bool set_mat);            // ea: 0x004CF1D0
    Handle AssignHandle(ActiveEffectSet* t);     // ea: 0x004CF290
    void BeginEffectQuery(const Entity* ent,
                          TPakId override_pak);  // ea: 0x004D1720
    Handle ExecEffectQuery();                    // ea: 0x004D1A60
    Handle TriggerNamedEffect(const Entity* ent, const char* name,
                              TPakId override_pak);  // ea: 0x004D1CB0
    void FrameAdvance(float delta);              // ea: 0x004D3930
    void GetDebugFxList(Entity* ent,
                        std::vector<std::string>& fx) const;  // ea: 0x004D3C70

private:
    void AdvanceFades(float delta);              // ea: 0x004C1010
    void SetSoundParams(SoundParams& soundParams, PendingQuery& q,
                        float useNslDefault);    // ea: 0x004C1120
    void ExecPendingQuery(PendingQuery& q);      // ea: 0x004D13C0
    void ExecutePendingQueries();                // ea: 0x004D1680
    void GetEffectTables(TPakId pak, const char* ts_name, const char* type,
                         ae_sized_array<const DbTable*, 16>& tables);
                                                       // ea: 0x004CAD20
    void GetEffectTables(TPakId pak, const char* ts_name, const char* ts_global,
                         const char* type,
                         ae_sized_array<const DbTable*, 16>& tables);
                                                       // ea: 0x004CAE70
    int QueryGDEvents(const char* event, PendingQuery& q,
                      ActiveEffectSet* fx, float delay);
                                                       // ea: 0x004D0330
    int QueryEventTable(PendingQuery& q, ActiveEffectSet* fx, float distSq);
                                                       // ea: 0x004D11B0
};
static_assert(sizeof(EffectEventSys) == 0xA380, "EffectEventSys size mismatch");

// ============================================================================
// Rumble types (core.o rumble.cpp)
// ============================================================================
struct RumbleEffectInstance {
    reserved_dlist<RumbleEffectInstance>::dlist_node m_dlist_node;  // +0x00
    RumbleEffectInstanceHandle m_handle;      // +0x08
    float m_cur_time;          // +0x0C
    float m_intensity;         // +0x10
    float m_base_intensity;    // +0x14
    float m_ramp_up_end;       // +0x18
    float m_steady_end;        // +0x1C
    float m_ramp_down_end;     // +0x20
    float m_duration;          // +0x24
    Broc::string m_rumble_notes;  // +0x28
    Bitmask<unsigned int> m_flags;  // +0x2C
};
static_assert(sizeof(RumbleEffectInstance) == 0x30,
              "RumbleEffectInstance size mismatch");

class RumbleEffect {
public:
    struct RumbleData {
        bool  enabled;             // +0x00
        unsigned char _pad[0x4 - 0x1];
        float delay;               // +0x04
        float intensity;           // +0x08
        float ramp_up_duration;    // +0x0C
        float steady_duration;     // +0x10
        float ramp_down_duration;  // +0x14
        Broc::string rumble_notes; // +0x18
        Bitmask<unsigned int> m_flags;  // +0x1C

        RumbleData() : enabled(false), delay(0.0f), intensity(0.0f),
                       ramp_up_duration(0.0f), steady_duration(0.0f),
                       ramp_down_duration(0.0f), rumble_notes(),
                       m_flags(0) {}
    };
    static_assert(sizeof(RumbleData) == 0x20, "RumbleData size mismatch");
    RumbleData mRumbleDataArray[2];  // +0x00

    RumbleEffect() {}
    bool GetEnabled(ERumbleMotorID rumbleID) const;  // ?GetEnabled@RumbleEffect@@QBE_NW4ERumbleMotorID@@@Z (core.o 0x4DE110)
    float GetDelay(ERumbleMotorID rumbleID) const;              // ?GetDelay@RumbleEffect@@QBEMW4ERumbleMotorID@@@Z (core.o 0x4DE090)
    float GetIntensity(ERumbleMotorID rumbleID) const;          // ?GetIntensity@RumbleEffect@@QBEMW4ERumbleMotorID@@@Z (core.o 0x4DE190)
    float GetRampDownDuration(ERumbleMotorID rumbleID) const;   // ?GetRampDownDuration@RumbleEffect@@QBEMW4ERumbleMotorID@@@Z (core.o 0x4DE210)
    float GetRampUpDuration(ERumbleMotorID rumbleID) const;     // ?GetRampUpDuration@RumbleEffect@@QBEMW4ERumbleMotorID@@@Z (core.o 0x4DE290)
    float GetSteadyDuration(ERumbleMotorID rumbleID) const;     // ?GetSteadyDuration@RumbleEffect@@QBEMW4ERumbleMotorID@@@Z (core.o 0x4DE310)
    void SetLooping(ERumbleMotorID rumbleID, bool looping);     // ?SetLooping@RumbleEffect@@QAEXW4ERumbleMotorID@@_N@Z (cg.o 0x6BBDE0)
    void SetDelay(ERumbleMotorID rumbleID, float new_delay);              // ?SetDelay@RumbleEffect@@QAEXW4ERumbleMotorID@@M@Z (g.o 0x4A8A70)
    void SetEnabled(ERumbleMotorID rumbleID, bool new_enabled);           // ?SetEnabled@RumbleEffect@@QAEXW4ERumbleMotorID@@_N@Z (g.o 0x4A8B60)
    void SetIntensity(ERumbleMotorID rumbleID, float new_intensity);      // ?SetIntensity@RumbleEffect@@QAEXW4ERumbleMotorID@@M@Z (g.o 0x4A8BE0)
    void SetRampDownDuration(ERumbleMotorID rumbleID, float new_duration);  // ?SetRampDownDuration@RumbleEffect@@QAEXW4ERumbleMotorID@@M@Z (g.o 0x4A8CE0)
    void SetRampUpDuration(ERumbleMotorID rumbleID, float new_duration);  // ?SetRampUpDuration@RumbleEffect@@QAEXW4ERumbleMotorID@@M@Z (g.o 0x4A8DD0)
    void SetSteadyDuration(ERumbleMotorID rumbleID, float new_duration);  // ?SetSteadyDuration@RumbleEffect@@QAEXW4ERumbleMotorID@@M@Z (g.o 0x4A8EC0)

private:
    void Initialize();  // ?Initialize@RumbleEffect@@AAEXXZ (g.o 0x4A8FB0)
};
static_assert(sizeof(RumbleEffect) == 0x40, "RumbleEffect size mismatch");

class RumbleManager {
public:
    static RumbleManager* CreateInst();  // ?CreateInst@RumbleManager@@SAPAV1@XZ (core.o)
    static void DeleteInst();  // ?DeleteInst@RumbleManager@@SAXXZ (core.o)
    struct InstanceHolder;
    static InstanceHolder sInstHolder;  // ?sInstHolder@RumbleManager@@2UInstanceHolder@1@A
    static RumbleManager* Inst(int instance);  // ea: 0x004A9DA0
    RumbleEffectInstanceHandle mNextHandle;            // +0x00
    reserved_dlist<RumbleEffectInstance> mRumbleLists[2];  // +0x04
    int mClient;                  // +0x24
    int mLastTimeNotRumbling;     // +0x28
    int mDontRumbleAgainUntil;    // +0x2C
    RumbleManager(int client);    // ea: 0x004C56F0
    ~RumbleManager();             // ea: 0x004CF310
    void StopMotors();
    RumbleEffectInstanceHandle Play(const RumbleEffect& effect, float intensity);
    RumbleEffectInstanceHandle Play(const RumbleEffect& effect,
                                    float min_distance, float max_distance,
                                    float distance);
    bool IsPlaying(RumbleEffectInstanceHandle handle) const;
    float TimeLeft(RumbleEffectInstanceHandle handle) const;
    void SetIntensity(RumbleEffectInstanceHandle handle, float intensity);
    void SetDistance(RumbleEffectInstanceHandle handle, float min_distance,
                     float max_distance, float distance);
    void FrameAdvance(float delta_time);
    void Reset();
    void Remove(RumbleEffectInstanceHandle handle);
private:
    RumbleEffectInstanceHandle BumpHandle();
};
static_assert(sizeof(RumbleManager) == 0x30, "RumbleManager size mismatch");

// ============================================================================
// DB query types (core.o db.cpp)
// ============================================================================
struct DbField {
    unsigned char m_column_type;  // +0x00
    unsigned char m_match_type;   // +0x01
    uint16_t      mId;            // +0x02
};
static_assert(sizeof(DbField) == 0x4, "DbField size mismatch");

class DbRow {
public:
    int16_t*  mValueRow;     // +0x00
    void*     mTable;        // +0x04 (DbTable*)
    int16_t   mRowIndex;     // +0x08
    int16_t   mColUsedNum;   // +0x0A
};
static_assert(sizeof(DbRow) == 0xC, "DbRow size mismatch");

struct DbColumn {
    uint16_t mId;                // +0x00
    uint16_t mNumElements;       // +0x02
    unsigned int mElementSize;   // +0x04
    void*  mElements;            // +0x08
    DbStringHashTable* mStringHash;  // +0x0C
};
static_assert(sizeof(DbColumn) == 0x10, "DbColumn size mismatch");

struct DbSchema {
    uint16_t        mNumColumnTypes;  // +0x00
    unsigned char   _pad[0x4 - 0x2];
    const unsigned char* mColumnTypes;  // +0x04
    const unsigned char* mMatchTypes;   // +0x08
    BitSet<255>     mPlaceholder;       // +0x0C
    const char**    mColumnNames;       // +0x2C
};
static_assert(sizeof(DbSchema) == 0x30, "DbSchema size mismatch");

class DbQueryResults;

class DbTable {
public:
    char   mName[30];        // +0x00
    uint16_t mNumColumns;    // +0x1E
    DbColumn** mColumns;     // +0x20
    DbRow*  mRows;           // +0x24
    int     mNumRows;        // +0x28
    void*   mIndexRoot;      // +0x2C (DbGraphNode*)
    DbSchema* mSchema;       // +0x30
};
static_assert(sizeof(DbTable) == 0x34, "DbTable size mismatch");

class DbGraphNode {
public:
    unsigned char mAttachments[0xC];  // +0x00 (NodeAttach)
    uint16_t      mFieldId;           // +0x0C
    unsigned char _pad[2];            // +0x0E
};
static_assert(sizeof(DbGraphNode) == 0x10, "DbGraphNode size mismatch");

struct DbFieldSet {
    virtual ~DbFieldSet();
    void Clear();
    const DbField* GetFieldByIdx(unsigned int idx) const;
    const DbField* GetFieldById(unsigned int field_id) const;
    unsigned int mNumParams;      // +0x04
    DbField*     mFields[64];     // +0x08
    unsigned char mIdToIdxMap[255];  // +0x108
    BitSet<255>  mSpecifiedById;  // +0x208
    BitSet<64>   mWeakByIdx;      // +0x228
    BitSet<255>  mWeakById;       // +0x230
};
static_assert(sizeof(DbFieldSet) == 0x250, "DbFieldSet size mismatch");

struct DbQuery {
    friend class EffectEventSys;
    virtual ~DbQuery();
    const DbTable* mDb;            // +0x04
    DbFieldSet     mConstraints;   // +0x08
    bool           mAutomaticFail; // +0x258
    char           mConstraintBuffer[512];  // +0x259
    unsigned int   mConstraintPos; // +0x45C
    void Execute(DbQueryResults& results);
    void Reset();
private:
    void ResetConstraints();
protected:
    int CompareField(int colId, const void* db_value);
    bool TestField(int colId, const void* db_value);
    void AcceptMatchingLeaf(const DbGraphNode* node, DbQueryResults& results);
    virtual void FindMatches(DbQueryResults& results);
};
static_assert(sizeof(DbQuery) == 0x460, "DbQuery size mismatch");

class DbQueryResults {
public:
    ae_sized_array<DbRow*, 64> mMatches;      // +0x000
    ae_sized_array<DbRow*, 64> mMatchesSpec;  // +0x104
    int  mMaxNumFields;            // +0x208
    DbRow* GetRandomResult();
    DbRow* GetRandomResultSpecific();
    DbRow* GetResult(unsigned int idx);
};
static_assert(sizeof(DbQueryResults) == 0x20C, "DbQueryResults size mismatch");

// ============================================================================
// ConfigString types (core.o configstring.cpp)
// ============================================================================
class ConfigString {
public:
    InplaceString mName;          // +0x00
    unsigned int  mNumKeyValues;  // +0x04
    unsigned char mStringMap[8];  // +0x08 (InplaceTree<InplaceString,InplaceString>)
};
static_assert(sizeof(ConfigString) == 0x10, "ConfigString size mismatch");

struct ConfigStringPtr {
    ConfigString* mValue;  // +0x00
    TPakId        mPakId;  // +0x04
};
static_assert(sizeof(ConfigStringPtr) == 0x8, "ConfigStringPtr size mismatch");

struct ConfigStringBank {
    unsigned char mData[0x1C];  // InplaceAssetBank<ConfigString,InplaceTree<...>>
};
static_assert(sizeof(ConfigStringBank) == 0x1C, "ConfigStringBank size mismatch");

class ConfigStringManager {
public:
    static void CreateInst();  // ?CreateInst@ConfigStringManager@@SAXXZ (core.o)
    static void DeleteInst();  // ?DeleteInst@ConfigStringManager@@SAXXZ (core.o)
    unsigned char mData[0x18C];  // InplaceAssetBankSet<ConfigStringBank>
    static ConfigStringManager* sInst;  // ?sInst@ConfigStringManager@@2PAV1@A (core.o @ 0x12F039C)
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pakId);  // ea: 0x004C5C80
    IVPointer<ConfigString> GetConfigString(TPakId pakId, const char* name,
                                            const char* type);  // ea: 0x004CE8D0
    void CallbackSearch(TPakId pakId, const char* type,
                        void (*callback)(const char*, const ConfigString*));
private:
    ConfigStringManager();  // ea: 0x004C5C60
    virtual ~ConfigStringManager(); // ea: 0x004C1440
};
static_assert(sizeof(ConfigStringManager) == 0x190,
              "ConfigStringManager size mismatch");

// ============================================================================
// STBManager - string table bank manager (0x190 bytes, core.o STBManager.cpp)
// ============================================================================
class STBManager {
public:
    static void CreateInst();  // ?CreateInst@STBManager@@SAXXZ (core.o)
    static void DeleteInst();  // ?DeleteInst@STBManager@@SAXXZ (core.o)
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    void* mBankArray[99];      // +0x04 (ae_array<StringTableBank*,99>)

    const StringTableEntry* GetSTBEntry(TPakId pakId, unsigned int hash);
    const StringTableEntry* GetSTBEntry(unsigned int hash);
    const StringTableEntry* GetSTBEntry(const char* pszReference);
    const char* GetSTBString(const char* pszReference);  // ?GetSTBString@STBManager@@QAEPBDPBD@Z
    const char* GetSTBString(unsigned int hash);
    const char* GetSTBString(TPakId pakId, unsigned int hash);
    unsigned int GetSTBFlags(const char* pszReference);
    unsigned int GetSTBFlags(unsigned int hash);
    unsigned int GetSTBFlags(TPakId pakId, unsigned int hash);
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);
private:
    STBManager();            // ea: 0x004C5CE0
    virtual ~STBManager();   // ea: 0x004C14D0
};
static_assert(sizeof(STBManager) == 0x190, "STBManager size mismatch");

// ============================================================================
// DbTablesetMgr - database tableset lookup (core.o STBManager.cpp)
// ============================================================================
class DbTablesetMgr {
public:
    static DbTablesetMgr* CreateInst();  // ?CreateInst@DbTablesetMgr@@SAXXZ (core.o)
    static void DeleteInst();  // ?DeleteInst@DbTablesetMgr@@SAXXZ (core.o)
    IVPointer<DbTableSet> GetTableSet(TPakId pakId,
                                      const char* id) const;  // ea: 0x004CA630
};

// ============================================================================
// EntityNotify types (core.o entity_notify.cpp)
// ============================================================================
class WaitTilOutput {
public:
    WaitTilOutput();  // ??0WaitTilOutput@@QAE@XZ (g.o 0x4B1680)
    virtual ~WaitTilOutput();
    virtual int GetSize();  // ?GetSize@WaitTilOutput@@UAEHXZ
    void* dListNodeFiller1;  // +0x04
    void* dListNodeFiller2;  // +0x08
    static void* operator new(size_t size, bool forceHeapAlloc,
                              const char* file, int line);  // ??2WaitTilOutput@@SAPAXI_NPBDH@Z
    static void* operator new(size_t size);  // ??2WaitTilOutput@@SAPAXI@Z
    static void operator delete(void* ptr, bool forceHeapAlloc,
                                const char* file, int line);  // ??3WaitTilOutput@@SAXPAX_NPBDH@Z
    static void operator delete(void* ptr);  // ??3WaitTilOutput@@SAXPAX@Z
    static PoolAllocator* sAllocator;  // ?sAllocator@WaitTilOutput@@0PAVPoolAllocator@@A
};
static_assert(sizeof(WaitTilOutput) == 0xC, "WaitTilOutput size mismatch");

class PoolAllocator;
class EntityHandleDb;

class EntityNotify {
public:
    reserved_dlist<EntityNotify>::dlist_node m_dlist_node;  // +0x00
    unsigned int mStr;        // +0x08
    DbLinkedHandle<EntityHandleDb, Entity> mOwner;  // +0x0C
    WaitTilOutput* mParam;    // +0x10

    EntityNotify(unsigned int hashStr, DbLinkedHandle<EntityHandleDb, Entity> ent,
                 WaitTilOutput* param);  // ea: 0x004BDAA0
    ~EntityNotify();                     // ea: 0x004B5650
    void* get_dlist_node();              // ?get_dlist_node@EntityNotify@@QAEPAXXZ (g.o 0x4A5B70)
    static void* operator new(size_t size, bool forceHeapAlloc,
                              const char* file, int line);  // ??2EntityNotify@@SAPAXI_NPBDH@Z (g.o 0x4A5BF0)
    static void operator delete(void* ptr);  // ??3EntityNotify@@SAXPAX@Z (core.o 0x4B3F90)
    static void operator delete(void* ptr, bool forceHeapAlloc,
                                const char* file, int line);  // ??3EntityNotify@@SAXPAX_NPBDH@Z (g.o 0x4A5C60)
    static PoolAllocator* sAllocator;    // ?sAllocator@EntityNotify@@0PAVPoolAllocator@@A @ 0xF00E28
};
static_assert(sizeof(EntityNotify) == 0x14, "EntityNotify size mismatch");

class EntityNotifySet {
public:
    reserved_dlist<EntityNotifySet>::dlist_node m_dlist_node;  // +0x00
    DbLinkedHandle<void, void> mEnt;      // +0x08
    reserved_dlist<EntityNotify> mStrings;  // +0x0C
    reserved_dlist<EndOnScriptNode> mEndOnList;  // +0x1C

    EntityNotifySet(Entity* e);  // ea: 0x004C1D80
    ~EntityNotifySet();          // ea: 0x004CEAE0
    void AddNotify(const HashString& h,
                   DbLinkedHandle<EntityHandleDb, Entity> owner);
    EntityNotify* GetNotify(const HashString& chk) const;
    bool AssignScriptVariable(const HashString& chk,
                              WaitTilOutput* scriptVariable);
    bool IsFinished();
    void KillEndOnThreads();
    static void UpdateList();
    static PoolAllocator* sAllocator;    // ?sAllocator@EntityNotifySet@@0PAVPoolAllocator@@A @ 0xF00E2C
};
static_assert(sizeof(EntityNotifySet) == 0x2C, "EntityNotifySet size mismatch");

// ============================================================================
// Effect params + light/rumble support types (verified against IDA)
// ============================================================================
struct gdLight {
    int   vertlight;  // +0x00
    float color_r;    // +0x04
    float color_g;    // +0x08
    float color_b;    // +0x0C
    int   flicker;    // +0x10
    float inner_rad;  // +0x14
    float outer_rad;  // +0x18
    float rampup;     // +0x1C
    float duration;   // +0x20
    float rampdown;   // +0x24
    int   projlight;  // +0x28
};
static_assert(sizeof(gdLight) == 0x2C, "gdLight size mismatch");

struct gdShakeRumble {
    float time;             // +0x00
    float freq;             // +0x04
    float movement;         // +0x08
    float nextDelay;        // +0x0C
    float rumble;           // +0x10
    float blur;             // +0x14
    float minDist;          // +0x18
    float maxDist;          // +0x1C
    float steadyDuration;   // +0x20
    float rampUpTime;       // +0x24
    float rampDownTime;     // +0x28
    int   useHighFreqVib;   // +0x2C
    int   rumbleEnabled;    // +0x30
    InplaceString bone;     // +0x34
};
static_assert(sizeof(gdShakeRumble) == 0x38, "gdShakeRumble size mismatch");

struct LightEffect {
    int           mType;          // +0x00 (LightEffect::eType)
    TPakId        mPakId;         // +0x04
    unsigned char _pad[0x10 - 0x08];
    math::Position3 mLightPos;    // +0x10
    float         mColor[4];      // +0x20
    bool          mActive;        // +0x30
    unsigned char _pad2[0x34 - 0x31];
    float         mMSecLifetime;  // +0x34
    float         mMSecLifeOrig;  // +0x38
    bool          mFlicker;       // +0x3C
    bool          mKill;          // +0x3D
    unsigned char _pad3[0x40 - 0x3E];
    float         mColorOriginal[4];  // +0x40
    float         mFlickerRatio;  // +0x50
    float         mInnerRadius;   // +0x54
    float         mOuterRadius;   // +0x58
    float         mScale;         // +0x5C
    bool          mFade;          // +0x60

    void SetScale(float s);       // ?SetScale@LightEffect@@QAEXM@Z
    bool IsLightFinished();       // ?IsLightFinished@LightEffect@@QAE_NXZ
};
static_assert(sizeof(LightEffect) == 0x70, "LightEffect size mismatch");

struct AbstractEffectLight::Params {
    TPakId     pakId;        // +0x00
    DbLinkedHandle<void, void> ent;  // +0x04
    float      delayTrigger; // +0x08
    int        flags;        // +0x0C
    gdLight*   light;        // +0x10
};
static_assert(sizeof(AbstractEffectLight::Params) == 0x14,
              "AbstractEffectLight::Params size mismatch");

struct AbstractEffectShakeAndRumble::Params {
    TPakId     pakId;        // +0x00
    DbLinkedHandle<void, void> ent;  // +0x04
    float      delayTrigger; // +0x08
    int        flags;        // +0x0C
    gdShakeRumble* shakeRumble;  // +0x10
};
static_assert(sizeof(AbstractEffectShakeAndRumble::Params) == 0x14,
              "AbstractEffectShakeAndRumble::Params size mismatch");

// ============================================================================
// DB support types (verified against IDA)
// ============================================================================
struct DbStringHashTable {
    unsigned int  mBucketVals[1024];  // +0x000
    unsigned char mBucketAmts[256];   // +0x1000
};
static_assert(sizeof(DbStringHashTable) == 0x1100,
              "DbStringHashTable size mismatch");

class DbTableSet {
public:
    char         mName[32];     // +0x00
    DbTable*     mTables;       // +0x20
    unsigned int mNumTables;    // +0x24
};
static_assert(sizeof(DbTableSet) == 0x28, "DbTableSet size mismatch");

// ============================================================================
// Pak/dialogue/file-support types (verified against IDA)
// ============================================================================
struct TBankAlloc {
    BitSet<64> mram_alloc1;  // +0x00
    BitSet<64> mram_alloc2;  // +0x08

    TBankAlloc() : mram_alloc1(), mram_alloc2() {}  // 0x5B6810
};
static_assert(sizeof(TBankAlloc) == 0x10, "TBankAlloc size mismatch");

struct LoadStats {
    uint64_t totalStart;   // +0x00
    float    total;        // +0x08
    uint64_t readStart;    // +0x10
    float    readTotal;    // +0x18
};
static_assert(sizeof(LoadStats) == 0x20, "LoadStats size mismatch");

struct tlFileBuf {
    unsigned char* Buf;       // +0x00
    unsigned int   Size;      // +0x04
    unsigned int   UserData;  // +0x08
};
static_assert(sizeof(tlFileBuf) == 0xC, "tlFileBuf size mismatch");

struct TlSystemCallbacks {
    static bool sWarningsEnabled;
    static bool sLockAllocsToPakHeap;
    static bool sLockAllocsToPakHeapOnce;

    // tl callback table (10 slots; copied verbatim by tlSetSystemCallbacks)
    struct CallbackTable {
        bool (*ReadFile)(const char* filename, tlFileBuf* fileBuf,
                         unsigned int align, unsigned int flags);   // +0x00
        void (*ReleaseFile)(tlFileBuf* fileBuf);                     // +0x04
        void (*CriticalError)(const char* txt);                      // +0x08
        void (*Warning)(const char* txt);                            // +0x0C
        void (*DebugPrint)(const char* txt);                         // +0x10
        void (*FinalPrint)(const char* txt);                         // +0x14
        bool (*LinkConnected)();                                     // +0x18
        void* (*MemAlloc)(unsigned int size, unsigned int align,
                          unsigned int flags);                       // +0x1C
        void* (*MemRealloc)(void* ptr, unsigned int size,
                            unsigned int align,
                            unsigned int flags);                     // +0x20
        void (*MemFree)(void* ptr);                                  // +0x24
    } mTlCallbacks;  // +0x00

    typedef void* (__cdecl* TlMemAllocCbfn)(unsigned int size,
                                            unsigned int align,
                                            unsigned int flags);
    typedef void (__cdecl* TlMemFreeCbfn)(void* ptr);

    TlSystemCallbacks();  // ea: 0x004D0A20
    TlMemAllocCbfn SetMemAllocCbfn(TlMemAllocCbfn cbfn);  // ea: 0x004BD400
    TlMemFreeCbfn SetMemFreeCbfn(TlMemFreeCbfn cbfn);     // ea: 0x004BD420

    static bool LockTlAllocsToPakHeap(bool s, bool once);
    static bool ReadFile(const char* filename, tlFileBuf* fileBuf,
                         unsigned int offset, unsigned int len);
    static void ReleaseFile(tlFileBuf* fileBuf);
    static void* MemRealloc(void* Ptr, unsigned int size,
                            unsigned int align, unsigned int flags);
    static void* MemAlloc(unsigned int size, unsigned int align,
                          unsigned int flags);
    static void MemFree(void* ptr);
    static void LinkFrame();
    static bool LinkConnected();
    static void DebugPrint(const char* txt);
    static void CriticalError(const char* txt);  // ea: 0x004CFA60
    static void Warning(const char* txt);        // ea: 0x004CFAF0

private:
    static bool IgnoreAssertion(
        const char* tlAssertText,
        ae_fixed_string<256, unsigned short>& assertText,
        ae_fixed_string<256, unsigned short>& assertExp,
        ae_fixed_string<256, unsigned short>& assertFile,
        int& assertLine);  // ea: 0x004CE800
    static bool ParseTlAssertString(
        const char* tlAssertText,
        ae_fixed_string<256, unsigned short>& assertMessage,
        ae_fixed_string<256, unsigned short>& assertExpression,
        ae_fixed_string<256, unsigned short>& fileName,
        int& line);  // ea: 0x004C5AA0
};
static_assert(sizeof(TlSystemCallbacks) == 0x28,
              "TlSystemCallbacks size mismatch");

class StringTableEntry {
public:
    unsigned int mHash;   // +0x00
    unsigned int mFlags;  // +0x04
    InplaceString mLoc;   // +0x08
};
static_assert(sizeof(StringTableEntry) == 0xC, "StringTableEntry size mismatch");

struct nalHeap {
    virtual ~nalHeap();  // __vftable at +0x00
};
static_assert(sizeof(nalHeap) == 0x4, "nalHeap size mismatch");

struct AnimHeap : nalHeap {
    void*       mBlock;  // +0x04
    unsigned char mHeap[0x49C];  // +0x08 mem_heap

    AnimHeap();   // ea: 0x004C1380
    virtual ~AnimHeap();  // ea: 0x004BD610
    virtual void* Allocate(int size);   // ea: 0x004BD660 (UAE)
    virtual void Free(void* ptr, int size);  // ea: 0x004BD690 (UAE)
    static void LinkAnimHeap();
};
static_assert(sizeof(AnimHeap) == 0x4A4, "AnimHeap size mismatch");

struct AssetBankSet {
    AssetBankSet();       // ??0AssetBankSet@@QAE@XZ (streamer.o @ 0x006663A0)
    virtual ~AssetBankSet();  // __vftable at +0x00
};
static_assert(sizeof(AssetBankSet) == 0x4, "AssetBankSet size mismatch");

class CtrlIcon {
public:
    static CtrlIcon* CreateInst();  // ?CreateInst@CtrlIcon@@SAPAV1@XZ (core.o)
    static void DeleteInst();  // ?DeleteInst@CtrlIcon@@SAXXZ (core.o)
    static CtrlIcon* sInst;  // ?sInst@CtrlIcon@@2PAV1@A
    char mScratchBuffer[2048];  // +0x00
    bool ContainsIconTag(const char* text);
    const char* TranslateIconTag(const char* text);  // ea: 0x004BD730
    bool ExtractIconTag(const char* text, char* preTagString,
                        char** postTagString, char** tagString);  // ea: 0x004BD7A0
    CtrlIcon();   // ea: 0x004BD6E0
    ~CtrlIcon();  // ea: 0x004BD6F0
};
static_assert(sizeof(CtrlIcon) == 0x800, "CtrlIcon size mismatch");

struct DialogueBank {
    unsigned char mData[0x1C];  // InplaceAssetBank<DialogueInstance,InplaceTree<unsigned int,unsigned int>>
};
static_assert(sizeof(DialogueBank) == 0x1C, "DialogueBank size mismatch");

struct DialogueManager : AssetBankSet {
    static void CreateInst();  // ?CreateInst@DialogueManager@@SAXXZ (core.o)
    static void DeleteInst();  // ?DeleteInst@DialogueManager@@SAXXZ (core.o)
    DialogueBank* mBanks[99];  // +0x04 ae_array<DialogueBank*,99>
    virtual void UnloadBank(TPakId pakId);
    void DecodeDialogueBank(const char* name, unsigned char* data, int size,
                            TPakId pakId);
    const char* GetDialogue(unsigned int hash) const;
    DialogueManager();   // ea: 0x004C0B40
    ~DialogueManager();  // ea: 0x004BCD80
};
static_assert(sizeof(DialogueManager) == 0x190, "DialogueManager size mismatch");

// ============================================================================
// PakFile - pak archive handle (276 bytes)
// Size: 0x114 (276 bytes) - verified against IDA. Nested apk/nfl/pak types are
// forward-declared; members touching them stay opaque until those objects port.
// ============================================================================
// EPakType (IDA local enum; values 0..9, Count is the debug-pak sentinel).
// Binary mangles EPakType as W4 (enum), not H (int typedef).
enum EPakType {
    kPakTypeGlobal = 0,
    kPakTypeFrontEnd = 1,
    kPakTypeAnimation = 2,
    kPakTypeLevel = 3,
    kPakTypeZone = 4,
    kPakTypeCommon = 5,
    kPakTypeVehicle = 6,
    kPakTypeCharacter = 7,
    kPakTypeWeapon = 8,
    kPakTypeUnknown = 9,
    kPakTypeCount = 10,
    kPakTypeMin = 0,
    kPakTypeMax = 9,
    kPakTypeInvalid = 0xFFFFFFFFu,
};
enum nflFileID : unsigned { NFL_FILE_ID_INVALID = 0xFFFFFFFFu };
typedef int TRequestId;   // TODO: type from IDA
struct PakInfoNode;
struct PakHeader;

class PakFile {
public:
    reserved_dlist<PakFile>::dlist_node m_dlist_node;  // +0x00
    const char* mCurrDecodeFile;      // +0x08
    unsigned char mPath[0x40];        // +0x0C ae_fixed_string<64,unsigned char>
    EPakType      mPakType;           // +0x4C
    unsigned int  mFilesize;          // +0x50
    nflFileID     mFileId;            // +0x54
    TBankAlloc    mBankAlloc;         // +0x58
    TBankAlloc    mSerializedAlloc;   // +0x68
    TPakId        mPakId;             // +0x78
    const PakInfoNode* mPakInfo;      // +0x7C
    PakHeader*    mHeader;            // +0x80
    int           mDefaultSectionIdx; // +0x84
    unsigned char* mHeaderBuffer;     // +0x88
    TRequestId    mHeaderRequestId;   // +0x8C
    unsigned char mApkFiles[0xC];     // +0x90 ae_vector<apk::apkFile*>
    bool          mCloseHandle;       // +0x9C
    bool          mOnlyLoadHeader;    // +0x9D
    unsigned char _pad[0xA0 - 0x9E];
    unsigned char mHeapList[0x34];    // +0xA0 ae_sized_array<mem_heap*,12>
    unsigned char mPrereqHeaps[0x14]; // +0xD4 ae_sized_array<PakFile*,4>
    unsigned int  mCurrentFile;       // +0xE8
    unsigned char* mCurrentFilePtr;   // +0xEC
    void*         mCurrentApk;        // +0xF0 apk::apkFile*
    void*         mCurrentApkFileEntry;   // +0xF4 apk::apkFileEntry*
    void*         mCurrentApkFileTypeEntry;  // +0xF8 apk::apkFileTypeEntry*
    int           mState;             // +0xFC PakFile::EState
    int           mLoadingState;      // +0x100 PakFile::ELoadingState
    unsigned char mLooseFiles[0xC];   // +0x104 ae_vector<void*>
    LoadStats*    mLoadStats;         // +0x110
};
static_assert(sizeof(PakFile) == 0x114, "PakFile size mismatch");

// ============================================================================
// File-system handle/list types (core.o files.cpp)
// ============================================================================
struct qfile_gus {
    void* file;       // +0x00 (opaque FILE*)
};
static_assert(sizeof(qfile_gus) == 0x4, "qfile_gus size mismatch");

struct qfile_us {
    qfile_gus file;    // +0x00
    int       unique;  // +0x04
};
static_assert(sizeof(qfile_us) == 0x8, "qfile_us size mismatch");

struct fileHandleData_t {
    qfile_us handleFiles;   // +0x00
    int      handleSync;    // +0x08
    int      baseOffset;    // +0x0C
    int      fileSize;      // +0x10
    int      zipFilePos;    // +0x14
    pack_t*  zipFile;       // +0x18
    int      streamed;      // +0x1C
    char     name[256];     // +0x20
};
static_assert(sizeof(fileHandleData_t) == 0x120,
              "fileHandleData_t size mismatch");
static_assert(offsetof(fileHandleData_t, name) == 0x20,
              "fileHandleData_t::name offset mismatch");

struct fileInList_s {
    fileData_s     data;  // +0x00
    fileInList_s*  next;  // +0x0C
};
static_assert(sizeof(fileInList_s) == 0x10, "fileInList_s size mismatch");

struct filelist_s {
    char           dir[128];     // +0x00
    int            numfiles;     // +0x80
    int            hashSize;     // +0x84
    fileInList_s** hashTable;    // +0x88
    fileInList_s*  buildBuffer;  // +0x8C
    filelist_s*    next;         // +0x90
};
static_assert(sizeof(filelist_s) == 0x94, "filelist_s size mismatch");
static_assert(offsetof(filelist_s, numfiles) == 0x80,
              "filelist_s::numfiles offset mismatch");

// ============================================================================
// SoundOptions - effect sound toggles (56 bytes)
// Size: 0x38 (56 bytes) - verified against IDA
// ============================================================================
class SoundOptions {
public:
    int mFxDontPlayFootSteps;     // +0x00
    int mFxDontPlayGearRattle;    // +0x04
    int mFxDontPlayLanding;       // +0x08
    int mFxDontPlayScriptCall;    // +0x0C
    int mFxDontPlayScriptCall_Dir;// +0x10
    int mFxDontPlayWeapon;        // +0x14
    int mFxDontPlayBulletHit;     // +0x18
    int mFxDontPlayGrenadeBounce; // +0x1C
    int mFxDontPlayProjExplode;   // +0x20
    int mFxDontPlayVehicle;       // +0x24
    int mFxDontPlayTurret;        // +0x28
    int mFxDontPlayVehicleWheel;  // +0x2C
    int mFxDontPlayLightFlash;    // +0x30
    int mFxDontPlayMusic;         // +0x34
};
static_assert(sizeof(SoundOptions) == 0x38, "SoundOptions size mismatch");

// ============================================================================
// ParticleParams - particle effect spawn params (76 bytes)
// Size: 0x4C (76 bytes) - verified against IDA
// ============================================================================
struct ParticleParams {
    TPakId                       mPakId;       // +0x00
    DbLinkedHandle<void, void>   mEnt;         // +0x04
    int                          mFlags;       // +0x08
    float                        mDelayTrigger;// +0x0C
    char*                        mNameRef;     // +0x10
    CollisionDesc*               mCd;          // +0x14
    tlFixedString                mBoneName;    // +0x18
    int                          mBoneIndex;   // +0x38
    bool                         mHasDirection;// +0x3C
    unsigned char                _pad[0x40 - 0x3D];
    int                          mParticleId;  // +0x40
    int                          mQueue;       // +0x44
    bool                         mUpdatePosOnly;  // +0x48
};
static_assert(sizeof(ParticleParams) == 0x4C, "ParticleParams size mismatch");
static_assert(offsetof(ParticleParams, mBoneName) == 0x18,
              "ParticleParams::mBoneName offset mismatch");

// ============================================================================
// ServerTime - server tick/time state (20 bytes)
// Size: 0x14 (20 bytes) - verified against IDA
// ============================================================================
struct ServerTime {
    unsigned int mNumTicksElapsed;  // +0x00
    int          mTickMSec;         // +0x04
    float        mTickDelta;        // +0x08
    float        mTickDeltaInv;     // +0x0C
    float        mElapsedTime;      // +0x10

    float GetElapsedTime() const { return mElapsedTime; }  // ?GetElapsedTime@ServerTime@@QBEMXZ (sv.o 0x51E1D0; inline COMDAT)
};
static_assert(sizeof(ServerTime) == 0x14, "ServerTime size mismatch");

struct RumbleManager::InstanceHolder {
    RumbleManager* sInst[1];  // +0x00
    InstanceHolder();  // ea: 0x004BCF60
};
static_assert(sizeof(RumbleManager::InstanceHolder) == 0x4,
              "RumbleManager::InstanceHolder size mismatch");
