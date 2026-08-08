// ============================================================================
// COD3 Core Systems - effect events, rumble, DB query, configstring, notify
// Reconstructed from IDA local types (PDB symbol data) via ida-pro-mcp.
// All sizes and offsets verified against IDA unless marked TODO.
// ============================================================================

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "core/ae_array.h"
#include "core/math_types.h"
#include "engine/broc_types.h"
#include "game/game_types.h"

// TPakId is defined fully in game/sv/sv_stubs.h; forward-declare the enum so
// this header stays standalone (C++11 allows enum : int forward decls).
enum TPakId : int;

// ============================================================================
// Enum placeholders - values must be fetched from IDA when porting bodies.
// Typedef'd as int to keep ABI size (4 bytes) without guessing values.
// ============================================================================
typedef int EEffectContext;      // TODO: enum values from IDA
typedef int ECollisionMaterial;  // TODO: enum values from IDA
typedef int EStanceType;         // TODO: enum values from IDA
typedef int EWeaponClass;        // TODO: enum values from IDA
typedef int EAction;             // TODO: enum values from IDA
typedef int EUserBoneId;         // TODO: enum values from IDA
typedef int nslWaveID;           // TODO: enum values from IDA

struct ParticleEffect;
struct gdLight;
struct LightEffect;
struct CameraShakeInstance;
struct EndOnScriptNode;
struct DbStringHashTable;

// ============================================================================
// Bitmask<T> - typed flag word (sizeof(T) bytes)
// ============================================================================
template <typename T>
struct Bitmask {
    T mVal;  // +0x00
    Bitmask() : mVal(0) {}
    explicit Bitmask(T v) : mVal(v) {}
};

// ============================================================================
// BitSet<N> - N-bit set packed into bytes
// ============================================================================
template <int N>
struct BitSet {
    unsigned char mBits[(N + 7) / 8];  // +0x00
    BitSet() { memset(mBits, 0, sizeof(mBits)); }
};

// ============================================================================
// reserved_dlist<T> - intrusive doubly-linked list (16 bytes)
// The dlist_node type is 8 bytes (prev/next). The 8-byte tail is reserved
// (likely head/tail links or count) - TODO verify when porting list code.
// ============================================================================
template <typename T>
struct reserved_dlist {
    struct dlist_node {
        dlist_node* mPrev;  // +0x00
        dlist_node* mNext;  // +0x04
    };
    dlist_node    mRoot;    // +0x00
    unsigned char _tail[8]; // +0x08 (TODO verify)
};
static_assert(sizeof(reserved_dlist<int>) == 0x10,
              "reserved_dlist size mismatch");

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
struct SoundParams {
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
struct AbstractEffect {
    virtual ~AbstractEffect();
    Broc::string                 mEffectName;        // +0x04
    unsigned int                 mEffectNameHashStr; // +0x08
    TPakId                       mPakId;             // +0x0C
    DbLinkedHandle<void, void>   mEntity;            // +0x10
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
};
static_assert(sizeof(AbstractEffectSound) == 0xA0,
              "AbstractEffectSound size mismatch");

// ============================================================================
// AbstractEffectParticle - particle effect (56 bytes)
// Size: 0x38 (56 bytes) - verified against IDA
// ============================================================================
struct AbstractEffectParticle : AbstractEffect {
    ParticleEffect* mParticle;  // +0x34
};
static_assert(sizeof(AbstractEffectParticle) == 0x38,
              "AbstractEffectParticle size mismatch");

// ============================================================================
// AbstractEffectLight - dynamic light effect (68 bytes)
// Size: 0x44 (68 bytes) - verified against IDA
// ============================================================================
struct AbstractEffectLight : AbstractEffect {
    gdLight*      mLight;     // +0x34
    LightEffect*  mProjLight; // +0x38
    LightEffect*  mVertLight; // +0x3C
    float         mTime;      // +0x40
};
static_assert(sizeof(AbstractEffectLight) == 0x44,
              "AbstractEffectLight size mismatch");

// ============================================================================
// AbstractEffectShakeAndRumble - camera shake + rumble effect (112 bytes)
// Size: 0x70 (112 bytes) - verified against IDA
// ============================================================================
struct RumbleEffectInstanceHandle {
    int mVal;  // +0x00
};
static_assert(sizeof(RumbleEffectInstanceHandle) == 0x4,
              "RumbleEffectInstanceHandle size mismatch");

struct AbstractEffectShakeAndRumble : AbstractEffect {
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
};
static_assert(sizeof(AbstractEffectShakeAndRumble) == 0x70,
              "AbstractEffectShakeAndRumble size mismatch");

// ============================================================================
// ActiveEffectSet - set of active effects for one query (44 bytes)
// Size: 0x2C (44 bytes) - verified against IDA
// ============================================================================
struct ActiveEffectSet {
    ae_sized_array<AbstractEffect*, 6> mEffects;  // +0x00
    TPakId                             mPakId;    // +0x1C
    math::Mat43*                       mPoPtr;    // +0x20
    Handle                             mId;       // +0x24
    Bitmask<unsigned int>              mFlags;    // +0x28
};
static_assert(sizeof(ActiveEffectSet) == 0x2C, "ActiveEffectSet size mismatch");
static_assert(offsetof(ActiveEffectSet, mPakId) == 0x1C,
              "ActiveEffectSet::mPakId offset mismatch");

// ============================================================================
// EffectEventSys - effect event system singleton (41856 bytes)
// Size: 0xA380 - verified against IDA
// ============================================================================
struct EffectEventSys {
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
    };
    static_assert(sizeof(CachedQuery) == 0x1B8, "CachedQuery size mismatch");

    struct PendingQuery {
        EEffectContext mType;          // +0x000
        CachedQuery    mCachedQuery;   // +0x004
        CollisionDesc  mCollisionInfo; // +0x1C0
        DbLinkedHandle<void, void> mQueryEnt;   // +0x1F0
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
    unsigned char       mHandleDb[0x1044];  // +0x9330 HandleDb<ActiveEffectSet,512,SizedHandle<9,23>>
    unsigned char       _tail[0xC];         // TODO verify (to 0xA380)
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

struct RumbleEffect {
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
    };
    static_assert(sizeof(RumbleData) == 0x20, "RumbleData size mismatch");
    RumbleData mRumbleDataArray[2];  // +0x00
};
static_assert(sizeof(RumbleEffect) == 0x40, "RumbleEffect size mismatch");

struct RumbleManager {
    RumbleEffectInstanceHandle mNextHandle;            // +0x00
    reserved_dlist<RumbleEffectInstance> mRumbleLists[2];  // +0x04
    int mClient;                  // +0x24
    int mLastTimeNotRumbling;     // +0x28
    int mDontRumbleAgainUntil;    // +0x2C
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

struct DbRow {
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

struct DbTable {
    char   mName[30];        // +0x00
    uint16_t mNumColumns;    // +0x1E
    DbColumn** mColumns;     // +0x20
    DbRow*  mRows;           // +0x24
    int     mNumRows;        // +0x28
    void*   mIndexRoot;      // +0x2C (DbGraphNode*)
    DbSchema* mSchema;       // +0x30
};
static_assert(sizeof(DbTable) == 0x34, "DbTable size mismatch");

struct DbGraphNode {
    unsigned char mAttachments[0xC];  // +0x00 (NodeAttach)
    uint16_t      mFieldId;           // +0x0C
    unsigned char _pad[2];            // +0x0E
};
static_assert(sizeof(DbGraphNode) == 0x10, "DbGraphNode size mismatch");

struct DbFieldSet {
    virtual ~DbFieldSet();
    unsigned int mNumParams;      // +0x04
    DbField*     mFields[64];     // +0x08
    unsigned char mIdToIdxMap[255];  // +0x108
    BitSet<255>  mSpecifiedById;  // +0x208
    BitSet<64>   mWeakByIdx;      // +0x228
    BitSet<255>  mWeakById;       // +0x230
};
static_assert(sizeof(DbFieldSet) == 0x250, "DbFieldSet size mismatch");

struct DbQuery {
    virtual ~DbQuery();
    const DbTable* mDb;            // +0x04
    DbFieldSet     mConstraints;   // +0x08
    bool           mAutomaticFail; // +0x258
    char           mConstraintBuffer[512];  // +0x259
    unsigned int   mConstraintPos; // +0x45C
};
static_assert(sizeof(DbQuery) == 0x460, "DbQuery size mismatch");

struct DbQueryResults {
    ae_sized_array<DbRow*, 64> mMatches;      // +0x000
    ae_sized_array<DbRow*, 64> mMatchesSpec;  // +0x104
    int  mMaxNumFields;            // +0x208
};
static_assert(sizeof(DbQueryResults) == 0x20C, "DbQueryResults size mismatch");

// ============================================================================
// ConfigString types (core.o configstring.cpp)
// ============================================================================
struct ConfigString {
    InplaceString mName;          // +0x00
    unsigned int  mNumKeyValues;  // +0x04
    unsigned char mStringMap[8];  // +0x08 (InplaceTree<InplaceString,InplaceString>)
};
static_assert(sizeof(ConfigString) == 0x10, "ConfigString size mismatch");

struct ConfigStringBank {
    unsigned char mData[0x1C];  // InplaceAssetBank<ConfigString,InplaceTree<...>>
};
static_assert(sizeof(ConfigStringBank) == 0x1C, "ConfigStringBank size mismatch");

struct ConfigStringManager {
    unsigned char mData[0x190];  // InplaceAssetBankSet<ConfigStringBank>
};
static_assert(sizeof(ConfigStringManager) == 0x190,
              "ConfigStringManager size mismatch");

// ============================================================================
// EntityNotify types (core.o entity_notify.cpp)
// ============================================================================
struct WaitTilOutput {
    virtual ~WaitTilOutput();
    void* dListNodeFiller1;  // +0x04
    void* dListNodeFiller2;  // +0x08
};
static_assert(sizeof(WaitTilOutput) == 0xC, "WaitTilOutput size mismatch");

struct EntityNotify {
    reserved_dlist<EntityNotify>::dlist_node m_dlist_node;  // +0x00
    unsigned int mStr;        // +0x08
    DbLinkedHandle<void, void> mOwner;  // +0x0C
    WaitTilOutput* mParam;    // +0x10
};
static_assert(sizeof(EntityNotify) == 0x14, "EntityNotify size mismatch");

struct EntityNotifySet {
    reserved_dlist<EntityNotifySet>::dlist_node m_dlist_node;  // +0x00
    DbLinkedHandle<void, void> mEnt;      // +0x08
    reserved_dlist<EntityNotify> mStrings;  // +0x0C
    reserved_dlist<EndOnScriptNode> mEndOnList;  // +0x1C
};
static_assert(sizeof(EntityNotifySet) == 0x2C, "EntityNotifySet size mismatch");
