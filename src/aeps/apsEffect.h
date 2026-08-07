// ============================================================================
// apsEffect / apsEffectTemplate — shared minimal views.
// Source: c:\cod\code\tl\aeps\include\apsEffect.h
// apsEffectTemplate full class (208 bytes) belongs to apsEffectTemplate.o
// (31 non-inline funcs, ported). apsEffect (128 bytes) belongs to apsEffect.o.
// Layout verified against IDA.
// ============================================================================
#ifndef COD3_AEPS_APSEFFECT_H
#define COD3_AEPS_APSEFFECT_H

#include "core/math_types.h"

#include "apsAction.h"      // apsArray<T>, apsFixUp, apsDestroy, apsAction
#include "apsPFD.h"         // apsPFD
#include "apsError.h"       // AEPS_VECTOR_NEW
#include "apsRenderNode.h"  // apsSphere, apsLight::LightInfo
#include "apsVFC.h"         // VFC::FrustumInfo
#include "apsGroup.h"       // apsGroup, apsBounds (full defs)
#include "core/tlFixedString.h"

#include <cstddef>

class apsRenderer;
class apsActionList;
struct nglLightContext;

// ============================================================================
// apsEffectTemplate — particle-effect template (208 bytes).
//   mLToW@0x00 (Mat43), mBoundingSphere@0x40, mElements@0x50, mModifiers@0x58,
//   mRefCount@0x60, mMemoryNeeded@0x64, mTimeScale@0x68, mPriority@0x6C,
//   mUsesUpdateLod@0x70, mName@0x74 (char[60]), mProfileId@0xB0, mStats@0xB4.
// ============================================================================
class apsEffectTemplate {
public:
    // ---- Element (72 bytes) ----
    struct Element {
        int          mMaxNumParticles;   // +0x00
        float        mBeginTime;         // +0x04
        float        mEndTime;           // +0x08
        int          mFlags;             // +0x0C
        char*        mNanoGraphName;     // +0x10
        float        mNanoWeight;        // +0x14
        apsRenderer* mRenderer;          // +0x18
        apsActionList* mActionList;      // +0x1C
        apsPFD       mPFD;               // +0x20 (40 bytes)
        Element();                       // ??0Element@apsEffectTemplate@@QAE@XZ
        int RenderingEnabled() const { return mFlags & 1; }              // ?RenderingEnabled@Element@apsEffectTemplate@@QBEIXZ
        void SetRenderingEnable(unsigned int bRender) { mFlags = (mFlags & 0xFFFFFFFE) | (bRender != 0); }  // ?SetRenderingEnable@Element@apsEffectTemplate@@QAEXI@Z
        void SetLocalSpace(unsigned int bLocalSpace) { mFlags = (mFlags & 0xFFFFFFFD) | (bLocalSpace != 0 ? 2 : 0); }  // ?SetLocalSpace@Element@apsEffectTemplate@@QAEXI@Z
        void SetNanoWeight(float weight) { mNanoWeight = weight; }       // ?SetNanoWeight@Element@apsEffectTemplate@@QAEXM@Z
        int GetMemoryNeeded() { return mMaxNumParticles * mPFD.mStride; }  // ?GetMemoryNeeded@Element@apsEffectTemplate@@QAEHXZ
        const apsPFD& GetPFD() const { return mPFD; }                    // ?GetPFD@Element@apsEffectTemplate@@QBEABVapsPFD@@XZ
        float BeginTime() const { return mBeginTime; }                   // ?BeginTime@Element@apsEffectTemplate@@QBEMXZ
        float EndTime() const { return mEndTime; }                       // ?EndTime@Element@apsEffectTemplate@@QBEMXZ
        int MaxNumParticles() const { return mMaxNumParticles; }         // ?MaxNumParticles@Element@apsEffectTemplate@@QBEHXZ
        apsRenderer* Renderer() const { return mRenderer; }              // ?Renderer@Element@apsEffectTemplate@@QBEPAVapsRenderer@@XZ
        apsActionList* ActionList() const { return mActionList; }        // ?ActionList@Element@apsEffectTemplate@@QBEPAVapsActionList@@XZ

        void RebuildPFD();                                               // ?RebuildPFD@Element@apsEffectTemplate@@QAEXXZ
        void SetNanoGraphName(const char* name);                         // ?SetNanoGraphName@Element@apsEffectTemplate@@QAEXPBD@Z
        void Free();                                                     // ?Free@Element@apsEffectTemplate@@QAEXXZ
    };
    static_assert(sizeof(Element) == 0x48, "apsEffectTemplate::Element size mismatch");

    // ---- Modifier (44 bytes) ----
    struct Modifier {
        tlFixedString mName;      // +0x00 (32 bytes)
        int           mElementNum;// +0x20
        int           mActionNum; // +0x24
        int           mParamNum;  // +0x28

        Modifier();                                    // ??0Modifier@apsEffectTemplate@@QAE@XZ
        Modifier& operator=(const Modifier& rhs);      // ??4Modifier@apsEffectTemplate@@QAEAAU01@ABU01@@Z
    };
    static_assert(sizeof(Modifier) == 0x2C, "apsEffectTemplate::Modifier size mismatch");
    // ---- Stats (24 bytes) ----
    struct Stats {
        int          mTotalInstances;      // +0x00
        int          mNumActiveInstances;  // +0x04
        int          mMaxActiveInstances;  // +0x08
        int          mNumFailedElements;   // +0x0C
        int          mNumFailedModifiers;  // +0x10
        unsigned int mbReported;           // +0x14

        void Reset();                        // ?Reset@Stats@apsEffectTemplate@@QAEXXZ
        void IncTotalInstances() { ++mTotalInstances; }        // ?IncTotalInstances@Stats@apsEffectTemplate@@QAEXXZ
        void IncActiveInstances() {                            // ?IncActiveInstances@Stats@apsEffectTemplate@@QAEXXZ
            ++mNumActiveInstances;
            if (mNumActiveInstances > mMaxActiveInstances)
                mMaxActiveInstances = mNumActiveInstances;
        }
        void DecActiveInstances() { --mNumActiveInstances; }   // ?DecActiveInstances@Stats@apsEffectTemplate@@QAEXXZ
        void IncFailedElements() { ++mNumFailedElements; }     // ?IncFailedElements@Stats@apsEffectTemplate@@QAEXXZ
        void IncFailedModifiers() { ++mNumFailedModifiers; }   // ?IncFailedModifiers@Stats@apsEffectTemplate@@QAEXXZ
    };
    static_assert(sizeof(Stats) == 0x18, "apsEffectTemplate::Stats size mismatch");

    // ---- data ----
    math::Mat43 mLToW;            // +0x00
    apsSphere   mBoundingSphere;  // +0x40
    apsArray<Element>  mElements; // +0x50
    apsArray<Modifier> mModifiers;// +0x58
    int         mRefCount;        // +0x60
    int         mMemoryNeeded;    // +0x64
    float       mTimeScale;       // +0x68
    int         mPriority;        // +0x6C
    int         mUsesUpdateLod;   // +0x70
    char        mName[60];        // +0x74
    unsigned int mProfileId;      // +0xB0
    Stats       mStats;           // +0xB4

    static const float kDefaultTimeScale;  // ?kDefaultTimeScale@apsEffectTemplate@@2MB (1.0f)
    static int sMaxElements;              // ?sMaxElements@apsEffectTemplate@@2HA @0x10DF988
    static int sMaxModifiers;             // ?sMaxModifiers@apsEffectTemplate@@2HA @0x10DF98C

    // ---- apsEffectTemplate.o (non-inline) ----
    apsEffectTemplate(const char* name);   // ??0apsEffectTemplate@@QAE@PBD@Z
    ~apsEffectTemplate();                  // ??1apsEffectTemplate@@QAE@XZ

    void SetLocalToWorldTransform(const math::Mat43& iMatrix);  // ?SetLocalToWorldTransform@apsEffectTemplate@@QAEXABVMat43@math@@@Z
    void SetBeginTime(int iElementNum, float iVal);  // ?SetBeginTime@apsEffectTemplate@@QAEXHM@Z
    void SetEndTime(int iElementNum, float iVal);    // ?SetEndTime@apsEffectTemplate@@QAEXHM@Z
    void SetLocalSpace(int iElementNum, unsigned int iVal);  // ?SetLocalSpace@apsEffectTemplate@@QAEXHI@Z
    void SetNanoGraphName(int iElementNum, const char* iVal); // ?SetNanoGraphName@apsEffectTemplate@@QAEXHPBD@Z
    void SetNanoWeight(int iElementNum, float iVal);   // ?SetNanoWeight@apsEffectTemplate@@QAEXHM@Z
    int GetModifierNumber(const tlFixedString& iName) const; // ?GetModifierNumber@apsEffectTemplate@@QBEHABVtlFixedString@@@Z
    void SetModifier(int iModifierNum, const tlFixedString& iName, int iElementNum, int iActionNum, int iParamNum);  // ?SetModifier@apsEffectTemplate@@QAEXHABVtlFixedString@@HHH@Z
    void SetEnableRendering(int iElementNum, unsigned int iRender);  // ?SetEnableRendering@apsEffectTemplate@@QAEXHI@Z
    unsigned int GetEnableRendering(int iElementNum) const;   // ?GetEnableRendering@apsEffectTemplate@@QBEIH@Z
    void Fixup(const apsFixupParams& iFixupParams);     // ?Fixup@apsEffectTemplate@@QAEXABUapsFixupParams@@@Z
    void CalcMemoryNeeded();                             // ?CalcMemoryNeeded@apsEffectTemplate@@QAEXXZ
    unsigned int TestAlloc();                            // ?TestAlloc@apsEffectTemplate@@QAEIXZ
    unsigned int MemoryMatch(apsEffectTemplate* tmpl);   // ?MemoryMatch@apsEffectTemplate@@QAEIPAV1@@Z
    void SetNumElements(int iNum);                       // ?SetNumElements@apsEffectTemplate@@QAEXH@Z
    int AddElement();                                    // ?AddElement@apsEffectTemplate@@QAEHXZ
    void RemoveElement(int iNum);                        // ?RemoveElement@apsEffectTemplate@@QAEXH@Z
    void SetMaxNumParticles(int iElementNum, int iVal);  // ?SetMaxNumParticles@apsEffectTemplate@@QAEXHH@Z
    void SetRenderer(int iElementNum, apsRenderer* iRenderer);  // ?SetRenderer@apsEffectTemplate@@QAEXHPAVapsRenderer@@@Z
    void SetActionList(int iElementNum, apsActionList* iActionList);  // ?SetActionList@apsEffectTemplate@@QAEXHPAVapsActionList@@@Z
    void SetElementData(int iElementNum, int iMaxNumParticles, float iBeginTime, float iEndTime, apsRenderer* iRenderer, apsActionList* iActionList, unsigned int iLocalSpace);  // ?SetElementData@apsEffectTemplate@@QAEXHHMMPAVapsRenderer@@PAVapsActionList@@I@Z
    void SetNumModifiers(int iNum);                      // ?SetNumModifiers@apsEffectTemplate@@QAEXH@Z
    int AddModifier();                                   // ?AddModifier@apsEffectTemplate@@QAEHXZ
    void RemoveModifier(int iModifierNum);               // ?RemoveModifier@apsEffectTemplate@@QAEXH@Z
    void BeginReport();                                  // ?BeginReport@apsEffectTemplate@@QAEXXZ
    void Report();                                       // ?Report@apsEffectTemplate@@QAEXXZ

    // ---- inline accessors (COMDATs in apsEffect.o / apsRegister.o) ----
    void SetName(const char* name);   // ?SetName@apsEffectTemplate@@QAEXPBD@Z (inline COMDAT)
    const math::Mat43& GetLocalToWorldTransform() const { return mLToW; }  // ?GetLocalToWorldTransform@apsEffectTemplate@@QBEABVMat43@math@@XZ
    const apsSphere& GetBoundingSphere() const { return mBoundingSphere; }
    float GetTimeScale() const { return mTimeScale; }   // ?GetTimeScale@apsEffectTemplate@@QBEMXZ
    int GetPriority() const { return mPriority; }       // ?GetPriority@apsEffectTemplate@@QBEHXZ
    const char* GetName() const { return mName; }       // ?GetName@apsEffectTemplate@@QBEPBDXZ
    int GetNumElements() const { return mElements.mSize; }  // ?GetNumElements@apsEffectTemplate@@QBEHXZ
    const Element& GetElement(int iIndex) const { return mElements.mElements[iIndex]; }  // ?GetElement@apsEffectTemplate@@ABEABUElement@1@H@Z
    int GetNumModifiers() const { return mModifiers.mSize; }  // ?GetNumModifiers@apsEffectTemplate@@QBEHXZ
    const Modifier& GetModifier(int iIndex) const { return mModifiers.mElements[iIndex]; }  // ?GetModifier@apsEffectTemplate@@ABEABUModifier@1@H@Z
    int GetMaxNumParticles(int iElementNum) const { return mElements.mElements[iElementNum].mMaxNumParticles; }  // ?GetMaxNumParticles@apsEffectTemplate@@QBEHH@Z
    float GetBeginTime(int iElementNum) const { return mElements.mElements[iElementNum].mBeginTime; }  // ?GetBeginTime@apsEffectTemplate@@QBEMH@Z
    float GetEndTime(int iElementNum) const { return mElements.mElements[iElementNum].mEndTime; }  // ?GetEndTime@apsEffectTemplate@@QBEMH@Z
    apsRenderer* GetRenderer(int iElementNum) const { return mElements.mElements[iElementNum].mRenderer; }  // ?GetRenderer@apsEffectTemplate@@QBEPAVapsRenderer@@H@Z
    apsActionList* GetActionList(int iElementNum) const { return mElements.mElements[iElementNum].mActionList; }  // ?GetActionList@apsEffectTemplate@@QBEPAVapsActionList@@H@Z
    unsigned int GetLocalSpace(int iElementNum) const { return mElements.mElements[iElementNum].mFlags & 2; }  // ?GetLocalSpace@apsEffectTemplate@@QBEIH@Z
    const apsPFD& GetPFD(int iElementNum) const { return mElements.mElements[iElementNum].mPFD; }  // ?GetPFD@apsEffectTemplate@@QBEABVapsPFD@@H@Z
    int GetElementNumber(int iModifierNum) const { return mModifiers.mElements[iModifierNum].mElementNum; }  // ?GetElementNumber@apsEffectTemplate@@QBEHH@Z
    int GetActionNumber(int iModifierNum) const { return mModifiers.mElements[iModifierNum].mActionNum; }  // ?GetActionNumber@apsEffectTemplate@@QBEHH@Z
    int GetParamNumber(int iModifierNum) const { return mModifiers.mElements[iModifierNum].mParamNum; }  // ?GetParamNumber@apsEffectTemplate@@QBEHH@Z
    int GetMemoryNeeded() const { return mMemoryNeeded; }  // ?GetMemoryNeeded@apsEffectTemplate@@QBEHXZ
    int GetUsesUpdateLod() const { return mUsesUpdateLod; }  // ?GetUsesUpdateLod@apsEffectTemplate@@QBEHXZ
    void AddRef() const { ++const_cast<apsEffectTemplate*>(this)->mRefCount; }  // ?AddRef@apsEffectTemplate@@QBEXXZ
    void RemoveRef() const { --const_cast<apsEffectTemplate*>(this)->mRefCount; }  // ?RemoveRef@apsEffectTemplate@@QBEXXZ
    Stats& GetStats() { return mStats; }                   // ?GetStats@apsEffectTemplate@@QAEAAUStats@1@XZ
};
static_assert(sizeof(apsEffectTemplate) == 0xD0, "apsEffectTemplate size mismatch");

// ---- inline COMDAT definitions (emitted in apsEffectTemplate.o) ----

inline apsEffectTemplate::Element::Element() {      // ??0Element@apsEffectTemplate@@QAE@XZ
    mMaxNumParticles = 10;
    mBeginTime = -1.0f;
    mEndTime = 3.4028235e38f;
    mFlags = 1;
    mNanoGraphName = 0;
    mNanoWeight = 0.0f;
    mRenderer = 0;
    mActionList = 0;
    mPFD.apsPFD::apsPFD();
}

inline apsEffectTemplate::Modifier::Modifier() {    // ??0Modifier@apsEffectTemplate@@QAE@XZ
    mName.hash = 0;
    for (int i = 0; i < 28; ++i) mName.str[i] = 0;
    mElementNum = 0;
    mActionNum = 0;
    mParamNum = 0;
}

inline apsEffectTemplate::Modifier& apsEffectTemplate::Modifier::operator=(const Modifier& rhs) {  // ??4Modifier@apsEffectTemplate@@QAEAAU01@ABU01@@Z
    mName = rhs.mName;
    mElementNum = rhs.mElementNum;
    mActionNum = rhs.mActionNum;
    mParamNum = rhs.mParamNum;
    return *this;
}

inline void apsEffectTemplate::Stats::Reset() {     // ?Reset@Stats@apsEffectTemplate@@QAEXXZ
    mTotalInstances = 0;
    mNumActiveInstances = 0;
    mMaxActiveInstances = 0;
    mNumFailedElements = 0;
    mNumFailedModifiers = 0;
    mbReported = 0;
}

inline void apsEffectTemplate::SetName(const char* name) {  // ?SetName@apsEffectTemplate@@QAEXPBD@Z (inline COMDAT)
    int v = 0;
    if (name != 0 && *name != 0) {
        while (v < 60) {
            mName[v] = name[v];
            char c = name[++v];
            if (c == 0) {
                if (v >= 60) return;
                break;
            }
        }
    }
    memset(&mName[v], 0, 60 - v);
}

// ============================================================================
// apsEffect — runtime particle effect (128 bytes, apsEffect.o).
// ============================================================================
// apsClient is declared in apsInternal.h (minimal view); its
// GetLightInfoAtPosition virtual (render.o) is used by apsEffect.

// render.o externs
extern void* apsMemAlloc(unsigned int size, unsigned int align, unsigned int flags);  // ?apsMemAlloc@@YAPAXIII@Z
extern void  apsMemFree(void* ptr);                                                    // ?apsMemFree@@YAXPAX@Z

// ============================================================================
// apsBlockArray<T> — block array (8 bytes, same layout as apsArray but uses
// SetCurrentPakId(-1)/SetPakAllocs(1) around allocations). Inline COMDATs in
// apsEffect.o (ctor/reserve/push_back/construct_array/resize_array/destroy_all).
// ============================================================================
template <typename T>
class apsBlockArray {
public:
    T*    mElements;   // +0x00
    short mCapacity;   // +0x04
    short mSize;       // +0x06

    apsBlockArray() : mElements(0), mCapacity(0), mSize(0) {}          // ??0?$apsBlockArray@T@@QAE@XZ
    apsBlockArray(int iSize, const T& iFillValue);                     // ??0?$apsBlockArray@T@@QAE@HABT@@@Z
    ~apsBlockArray() { destroy_all(); }                                // ??1?$apsBlockArray@T@@QAE@XZ

    int size() const { return mSize; }                                 // ?size@...QBEHXZ
    T& operator[](int iIndex) {                                        // ??A@...QAEAAATH@Z
        if ((iIndex < 0 || iIndex >= mSize) &&
            _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 505,
                      "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            __debugbreak();
        return mElements[iIndex];
    }
    T* begin() { return mElements; }                                   // ?begin@...QAEPATXZ
    T* end() { return &mElements[mSize]; }                             // ?end@...QAEPATXZ

    int reserve(int iCapacity);                                        // ?reserve@...QAEIH@Z
    int push_back(const T& iElement);                                  // ?push_back@...QAEIABT@@@Z

protected:
    T* construct_array(int iNumber);           // ?construct_array@...AAEPATH@Z
    T* construct_array(int iNumber, int iSize);  // ?construct_array@...AAEPATHH@Z
    T* resize_array(int iCapacity, int iSize);   // ?resize_array@...AAEPATHH@Z
    void destroy_all();                         // ?destroy_all@...QAEXXZ
};

// ============================================================================
// apsEffect — runtime particle effect (128 bytes).
//   mLToW@0x00, mStartTime@0x40, mLastTime@0x44, mTemplate@0x48,
//   mStopEmitting@0x4C, mNumCreated@0x50, mFlags@0x54, mSortKey@0x58,
//   mGroups@0x5C, mModifiers@0x64, mCollisionData@0x6C, mRaycastCountdown@0x70,
//   mParentAgePercent@0x74, mTimeToNextRemoveCheck@0x78, mId@0x7C.
// ============================================================================
class apsEffect {
public:
    struct Modifier {
        apsAction* mAction;   // +0x00
        int        mParamNum; // +0x04
        float      mVal;      // +0x08
    };

    struct SortKey {
        union {
            struct {
                unsigned int sortkey0 : 24;
                unsigned int flags : 1;
                unsigned int priority : 7;
            } fields;
            unsigned int _32;
        };
    };

    struct RaycastRequest {
        math::Dir3   start;     // +0x00
        math::Dir3   end;       // +0x10
        unsigned int resultID;  // +0x20
    };

    struct RaycastResult {
        math::Dir3 normal;    // +0x00
        math::Dir3 position;  // +0x10
        float      t;         // +0x20
    };

    struct CollisionData {
        apsBounds       mBounds;              // +0x00 (32 bytes)
        RaycastRequest  mRaycastRequests[32]; // +0x20 (1536)
        RaycastResult   mRaycastResults[32];  // +0x620 (1536)
        unsigned int    mNumRaycastRequests;  // +0xC20
    };
    static_assert(sizeof(CollisionData) == 0xC30, "apsEffect::CollisionData size mismatch");

    static const int MAX_RAYCAST_LIST_SIZE = 16;
    static const int MAX_COLLISION_RAYCAST_REQUESTS = 32;

    // ---- data ----
    math::Mat43             mLToW;            // +0x00
    float                   mStartTime;       // +0x40
    float                   mLastTime;        // +0x44
    const apsEffectTemplate* mTemplate;       // +0x48
    int                     mStopEmitting;    // +0x4C
    int                     mNumCreated;      // +0x50
    int                     mFlags;           // +0x54
    SortKey                 mSortKey;         // +0x58
    apsBlockArray<apsGroup*> mGroups;         // +0x5C
    apsBlockArray<Modifier>  mModifiers;      // +0x64
    CollisionData*          mCollisionData;   // +0x6C
    unsigned int            mRaycastCountdown;// +0x70
    float                   mParentAgePercent;// +0x74
    float                   mTimeToNextRemoveCheck;  // +0x78
    int                     mId;              // +0x7C

    // ---- apsEffect.o (non-inline) ----
    apsEffect(const apsEffectTemplate* iTemplate, float iStartTime);  // ??0apsEffect@@QAE@PBVapsEffectTemplate@@M@Z
    ~apsEffect();                                                     // ??1apsEffect@@QAE@XZ
protected:
    void TheRealInitializeLOL();    // ?TheRealInitializeLOL@apsEffect@@IAEXXZ
protected:
private:
    void swap_modifiers();               // ?swap_modifiers@apsEffect@@AAEXXZ
protected:
    unsigned int Destruct(unsigned int bKillQuickly);  // ?Destruct@apsEffect@@IAEII@Z
    unsigned int Construct(const apsEffectTemplate* iTemplate, float iStartTime);  // ?Construct@apsEffect@@IAEIPBVapsEffectTemplate@@M@Z
public:
    void StopEmitting();            // ?StopEmitting@apsEffect@@QAEXXZ
    int  GetModifierId(const char* name);  // ?GetModifierId@apsEffect@@QAEHPBD@Z
    const RaycastResult& GetRaycastResult(unsigned int id) const;  // ?GetRaycastResult@apsEffect@@QBEABURaycastResult@1@I@Z
    void AccumulateCollisionBounds(const apsBounds& bounds);  // ?AccumulateCollisionBounds@apsEffect@@QAEXABUapsBounds@@@Z
    void IncrementRaycastCountdown();    // ?IncrementRaycastCountdown@apsEffect@@QAEXXZ
    static unsigned int GetRaycastCountdownMaxValue();  // ?GetRaycastCountdownMaxValue@apsEffect@@SAIXZ
    static unsigned int IsPoolEmpty();   // ?IsPoolEmpty@apsEffect@@SAIXZ
    static unsigned int TestAlloc(apsEffectTemplate* tmpl);  // ?TestAlloc@apsEffect@@SAIPAVapsEffectTemplate@@@Z
    static int GetNumActiveEffects();    // ?GetNumActiveEffects@apsEffect@@SAHXZ
    void ReportTemplate();               // ?ReportTemplate@apsEffect@@QAEXXZ
    void SetLocalToWorldTransform(const math::Mat43& iMatrix);  // ?SetLocalToWorldTransform@apsEffect@@QAEXABVMat43@math@@@Z
    int  CountParticles();               // ?CountParticles@apsEffect@@QAEHXZ
    void SetModifierValue(int iNum, float iVal);  // ?SetModifierValue@apsEffect@@QAEXHM@Z
    float GetModifierValue(int iNum);    // ?GetModifierValue@apsEffect@@QAEMH@Z
    float GetModifierTemplateValue(int iNum);  // ?GetModifierTemplateValue@apsEffect@@QAEMH@Z
    void Render(nglLightContext* iLightContext, const VFC::FrustumInfo& frustumInfo);  // ?Render@apsEffect@@QAEXPAUnglLightContext@@ABUFrustumInfo@VFC@@@Z
    void GetBounds(apsBounds& iBounds);  // ?GetBounds@apsEffect@@QAEXAAUapsBounds@@@Z
    void SetPosition(const math::Dir3& pos);  // ?SetPosition@apsEffect@@QAEXABVDir3@math@@@Z
    void SetCulled(unsigned int bCulled); // ?SetCulled@apsEffect@@QAEXI@Z
    void Report(int index);              // ?Report@apsEffect@@QAEXH@Z
    void Update(float iCurTime);         // ?Update@apsEffect@@QAEXM@Z
    unsigned int IsDone();               // ?IsDone@apsEffect@@QAEIXZ
    void FastForward(float deltaT, int numIncr);  // ?FastForward@apsEffect@@QAEXMH@Z
    void CalcSortKey();                  // ?CalcSortKey@apsEffect@@QAEXXZ
    unsigned int RequestRaycast(const math::Dir3& start, const math::Dir3& end);  // ?RequestRaycast@apsEffect@@QAEIABVDir3@math@@0@Z
    static void ReportEffects();         // ?ReportEffects@apsEffect@@SAXXZ
    static void InitPool(int maxEffects); // ?InitPool@apsEffect@@SAXH@Z
    static apsEffect* New(const apsEffectTemplate* iTemplate, float iStartTime);  // ?New@apsEffect@@SAPAV1@PBVapsEffectTemplate@@M@Z
    static void Delete(apsEffect* pEffect, unsigned int bKillQuickly);  // ?Delete@apsEffect@@SAXPAV1@I@Z
    static void TermPool();              // ?TermPool@apsEffect@@SAXXZ

    // ---- inline COMDATs (apsEffect.o) ----
    int IsStopping() const { return mStopEmitting; }                     // ?IsStopping@apsEffect@@QAEHXZ
    const apsEffectTemplate& Template() const { return *mTemplate; }     // ?Template@apsEffect@@QBEABVapsEffectTemplate@@XZ
    void SetParentAgePercent(float age) { mParentAgePercent = age; }     // ?SetParentAgePercent@apsEffect@@QAEXM@Z (inline COMDAT, apsInternal.o)
};
static_assert(sizeof(apsEffect) == 0x80, "apsEffect size mismatch");

// g_effectTime — global current effect time (?g_effectTime@@3MA @0x14CE2E0).
extern float g_effectTime;

// AEPS_VECTOR_NEW<apsEffect> specialization — zeroes the pool blocks directly
// (mParentAgePercent = -1, arrays empty), matching IDA 0x7F06F0. Used by
// apsMemory::PoolAllocator<apsEffect> in apsEffect::InitPool.
template <>
inline apsEffect* AEPS_VECTOR_NEW<apsEffect>(int iNum, int iAlignment) {
    apsEffect* result = (apsEffect*)apsCommon::GetAllocator()->MemAlign(iNum * sizeof(apsEffect), iAlignment);
    if (iNum > 0) {
        apsEffect* p = result;
        do {
            p->mTemplate = 0;
            p->mStopEmitting = 0;
            p->mNumCreated = 0;
            p->mFlags = 0;
            p->mGroups.mElements = 0;
            p->mGroups.mCapacity = 0;
            p->mGroups.mSize = 0;
            p->mModifiers.mElements = 0;
            p->mModifiers.mCapacity = 0;
            p->mModifiers.mSize = 0;
            p->mParentAgePercent = -1.0f;
            ++p;
            --iNum;
        } while (iNum != 0);
    }
    return result;
}

// `anonymous namespace'::GetLightInfo — effect->light info helper.
void GetLightInfo(const apsEffect* effect, apsLight::LightInfo* outInfo);

#endif // COD3_AEPS_APSEFFECT_H
