// ============================================================================
// apsCommon — shared APS globals / helpers.
// Source: c:\cod\code\tl\aeps\source\apsCommon.cpp
// Verified against IDA (aeps_xboxr:apsCommon.o):
//   apsAllocator::MemAlign @0x7EB370  (?MemAlign@apsAllocator@@UBEPAXII@Z)
//   apsAllocator::MemFree  @0x7EB3A0  (?MemFree@apsAllocator@@UBEXPAX@Z)
//   apsAllocator::MemResize@0x7EB3C0  (?MemResize@apsAllocator@@UBEPAXPAXII@Z)
//   apsCommon::Init        @0x7EB8D0  (?Init@apsCommon@@SAXIHI@Z)
//   apsCommon::SetupFrame  @0x7EB3E0  (?SetupFrame@apsCommon@@SAXABVMat43@math@@M@Z)
//   apsCommon::Term        @0x7EBA40  (?Term@apsCommon@@SAXXZ)
//   apsCommon::Report      @0x7EB750  (?Report@apsCommon@@SAXXZ)
//   ... (all 22 non-inline funcs owned by apsCommon.o)
// ============================================================================
#ifndef COD3_AEPS_APSCOMMON_H
#define COD3_AEPS_APSCOMMON_H

#include "core/math_types.h"
#include "apsUtil.h"

#include <intrin.h>

struct nglScene;
struct nglTexture;
class apsClient;

// ============================================================================
// apsLOD — level-of-detail delay table (44 bytes, verified against IDA).
//   mBaseRate @0x00, mIdleRate @0x04, mRanges[4] @0x08 (8 bytes each),
//   mEnabled @0x28.
// ============================================================================
class apsLOD {
public:
    struct Range {
        float mDistance;  // +0x00
        float mDelay;     // +0x04
    };

    static const int kNumLODs = 4;

    float  mBaseRate;  // +0x00
    float  mIdleRate;  // +0x04
    Range  mRanges[kNumLODs];  // +0x08
    unsigned int mEnabled;     // +0x28

    void SetBaseRate(float rate) { mBaseRate = rate; }                          // ea: 0x7EB0F0 (COMDAT)
    void SetIdleRate(float rate) { mIdleRate = rate; }                          // ea: 0x7EB100 (COMDAT)
    void SetRange(int range, float dist, float rate) {                       // ea: 0x7EB120 (COMDAT)
        if (range >= kNumLODs &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsCommon.h", 64,
                      "(range >= 0) && (range < kNumLODs)", "invalid Aeps LOD index"))
            __debugbreak();
        mRanges[range].mDistance = dist;
        mRanges[range].mDelay = mBaseRate * rate;
    }
    void SetDefaults() {                                                       // ea: 0x7EB180 (COMDAT)
        mBaseRate = 0.033333335f;
        mEnabled = 0;
        mRanges[0].mDistance = 500.0f;
        mRanges[0].mDelay = mBaseRate;
        mRanges[1].mDistance = 1000.0f;
        mRanges[1].mDelay = mBaseRate * 2.0f;
        mRanges[2].mDistance = 2000.0f;
        mRanges[2].mDelay = mBaseRate * 4.0f;
        mRanges[3].mDistance = 4000.0f;
        mRanges[3].mDelay = mBaseRate * 8.0f;
        mIdleRate = 16.0f;
    }
    void SetEnabled(unsigned int bEnabled) { mEnabled = bEnabled; }            // ea: 0x7EB210 (COMDAT)

    float GetDelayForDistance(float dist) const {                              // ea: 0x808730 (COMDAT)
        if (mEnabled == 0)
            return 0.0f;
        int v3 = 0;
        for (const Range* i = mRanges; i->mDistance <= dist; ++i) {
            if (++v3 >= kNumLODs)
                return mIdleRate * mBaseRate;
        }
        return mRanges[v3].mDelay;
    }
};
static_assert(sizeof(apsLOD) == 0x2C, "apsLOD size mismatch");

// ============================================================================
// apsAllocator — virtual allocator interface. 4 bytes (vtable only).
// vftable slots: [0]=~dtor, [1]=MemAlign, [2]=MemFree, [3]=MemResize.
// ============================================================================
class apsAllocator {
public:
    virtual ~apsAllocator() {}                      // ??1apsAllocator@@UAE@XZ
    virtual void* MemAlign(unsigned int iSize, unsigned int iAlignment) const;  // @0x7EB370
    virtual void MemFree(void* iPtr) const;         // @0x7EB3A0
    virtual void* MemResize(void* iPtr, unsigned int iNewSize, unsigned int iAlignment) const;  // @0x7EB3C0
};
static_assert(sizeof(apsAllocator) == 4, "apsAllocator size mismatch");

// ============================================================================
// apsCommon — shared APS globals / helpers.
// ============================================================================
class apsCommon {
public:
    static const int MAX_NUM_VIEWPORTS = 1;

    // Camera view-basis / handedness settings used by apsMath::CreateFromVectorsDirUp.
    // Layout verified against IDA (64 bytes):
    //   mDir @0x00, mUp @0x10, mLeft @0x20, mRoll @0x30, mXFlip @0x34
    struct CameraSettings {
        math::Dir3 mDir;    // +0x00
        math::Dir3 mUp;     // +0x10
        math::Dir3 mLeft;   // +0x20
        float mRoll;        // +0x30
        float mXFlip;       // +0x34

        CameraSettings() {}                             // ??0CameraSettings@apsCommon@@QAE@XZ
        void Init() {                                   // ea: 0x7EB270 (COMDAT)
            mDir.v = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);    // +Y (0,1,0,0)
            mUp.v = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);     // +Z (0,0,1,0)
            mLeft.v = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);   // +X (1,0,0,0)
            mRoll = 0.0f;
            mXFlip = 1.0f;
        }
        math::Dir3& Dir()  { return mDir; }             // ea: 0x7EB220 (COMDAT)
        math::Dir3& Up()   { return mUp; }              // ea: 0x7EB230 (COMDAT)
        math::Dir3& Left() { return mLeft; }            // ea: 0x7EB240 (COMDAT)
        float& Roll()      { return mRoll; }            // ea: 0x7EB250 (COMDAT)
        float& XFlip()     { return mXFlip; }           // ea: 0x7EB260 (COMDAT)
    };
    static_assert(sizeof(CameraSettings) == 0x40, "CameraSettings size mismatch");

    // Per-player view state used by apsGroup::TestVisibility.
    // Layout verified against IDA (128 bytes):
    //   mViewPos @0x00, mClipPlanes[6] @0x10, mProjectionX @0x70, mActive @0x74
    struct PlayerViewPort {
        math::Position3 mViewPos;
        math::Vector4   mClipPlanes[6];
        float           mProjectionX;
        bool            mActive;

        PlayerViewPort() : mProjectionX(1.0f), mActive(false) {}  // ??0PlayerViewPort@apsCommon@@QAE@XZ
    };
    static_assert(sizeof(PlayerViewPort) == 0x80, "PlayerViewPort size mismatch");

    // ---- non-inline static methods (apsCommon.o) ----
    static apsAllocator* GetAllocator();                    // ?GetAllocator@apsCommon@@SAPAVapsAllocator@@XZ
    static apsAllocator* GetDefaultAllocator();             // ?GetDefaultAllocator@apsCommon@@SAPAVapsAllocator@@XZ
    static apsAllocator* SetAllocator(apsAllocator* i_allocator);   // ?SetAllocator@apsCommon@@SAPAVapsAllocator@@PAV2@@Z
    static apsAllocator* GetBlockAllocator();               // ?GetBlockAllocator@apsCommon@@SAPAVapsAllocator@@XZ
    static void          SetBlockAllocator(apsAllocator* i_allocator); // ?SetBlockAllocator@apsCommon@@SAXPAVapsAllocator@@@Z

    static int  GetCurrentPakId();                          // ?GetCurrentPakId@apsCommon@@SAHXZ
    static int  SetCurrentPakId(int id);                    // ?SetCurrentPakId@apsCommon@@SAHH@Z
    static int  PakAllocs();                                // ?PakAllocs@apsCommon@@SAHXZ
    static int  SetPakAllocs(int value);                    // ?SetPakAllocs@apsCommon@@SAHH@Z

    static void Init(unsigned int bDebug, int iMeshLightCat, unsigned int iMaxParticles);  // ?Init@apsCommon@@SAXIHI@Z
    static void Term();                                     // ?Term@apsCommon@@SAXXZ
    static void Report();                                   // ?Report@apsCommon@@SAXXZ
    static void InitShaders();                              // ?InitShaders@apsCommon@@SAXXZ
    static void SetupFrame(const math::Mat43& iWorldToView, float xFlip);  // ?SetupFrame@apsCommon@@SAXABVMat43@math@@M@Z

    static void SetClient(apsClient* client);               // ?SetClient@apsCommon@@SAXPAVapsClient@@@Z

    static PlayerViewPort* GetPlayerViewPort(unsigned int playerId);  // ?GetPlayerViewPort@apsCommon@@SAPAUPlayerViewPort@1@I@Z

    static void SubmitSpawnedEffectQueue();                 // ?SubmitSpawnedEffectQueue@apsCommon@@SAXXZ
    static void RemoveFromSpawnedEffectQueueByPakId(int pakId);  // ?RemoveFromSpawnedEffectQueueByPakId@apsCommon@@SAXH@Z
    static void ClearSpawnedEffectQueue();                  // ?ClearSpawnedEffectQueue@apsCommon@@SAXXZ

    // ---- inline accessors (COMDATs emitted in various objects) ----
    static unsigned int Initialised() { return mFlags & 1; }  // ?Initialised@apsCommon@@SAIXZ
    static nglScene* GetBuildScene() { return mBuildScene; }  // ?GetBuildScene@apsCommon@@SAPAUnglScene@@XZ (apsGroup.o)
    static nglScene* GetShimmerScene() { return mShimmerScene; }  // ?GetShimmerScene@apsCommon@@SAPAUnglScene@@XZ (apsColorRectangleRenderer.o)
    static CameraSettings& Camera() { return mCamera; }     // ?Camera@apsCommon@@SAAAUCameraSettings@1@XZ (apsInternal.o)
    static apsLOD& LOD() { return mLOD; }                   // ?LOD@apsCommon@@SAAAVapsLOD@@XZ (apsActionList.o)
    static float GetChanceToRemove() { return mChanceToRemove; }  // ?GetChanceToRemove@apsCommon@@SAMXZ (apsActionList.o)
    static float* (*GetSplineCallback())(unsigned int) { return mSplineCallback; }  // ?GetSplineCallback@apsCommon@@SAP6APAMI@ZXZ (apsSuppliedActions.o)
    static void SetSplineCallback(float* (*iCallback)(unsigned int)) { mSplineCallback = iCallback; }  // ?SetSplineCallback@apsCommon@@SAXP6APAMI@Z@Z (game2.o)
    static apsClient* GetClient() { return mApsClient; }    // ?GetClient@apsCommon@@SAPAVapsClient@@XZ (g.o)
    static void SaveBuildScenePtr(nglScene* iScene) { mBuildScene = iScene; }  // ?SaveBuildScenePtr@apsCommon@@SAXPAUnglScene@@@Z (render.o)
    static void InvalidateBuildScenePtr() { mBuildScene = 0; }  // ?InvalidateBuildScenePtr@apsCommon@@SAXXZ (render.o)

private:
    // flag helpers (private static per map: ?SetFlag@apsCommon@@KAXI@Z)
    static void SetFlag(unsigned int flag) { mFlags |= flag; }    // ea: 0x7EB300
    static void ClearFlag(unsigned int flag) { mFlags &= ~flag; } // ea: 0x7EB320
    static int  IsFlagSet(unsigned int flag) { return (mFlags & flag) != 0; }  // ea: 0x7EB340

public:
    // NOTE: map shows these statics with @@0 (private) mangling
    // (?mCamera@apsCommon@@0UCameraSettings@1@A, ?mBuildScene@apsCommon@@0PAUnglScene@@A,
    //  ?mShimmerScene@apsCommon@@0PAUnglScene@@A). apsGroup.cpp and the renderer
    //  headers touch them directly, so they are kept public here (mirrors the
    //  existing apsGroup port). TODO(apsCommon.o): reconcile access specifiers.
    static CameraSettings mCamera;      // @0x10DED10 (data)
    static nglScene* mBuildScene;       // @0x10DECFC (data)
    static nglScene* mShimmerScene;     // @0x10DECF0 (data)

private:
    static apsAllocator* mCurAllocator;   // ?mCurAllocator@apsCommon@@0PAVapsAllocator@@A
    static int  mCurPakId;                // ?mCurPakId@apsCommon@@0HA
    static apsAllocator mDefAllocator;    // ?mDefAllocator@apsCommon@@0VapsAllocator@@A
    static apsLOD mLOD;                   // ?mLOD@apsCommon@@0VapsLOD@@A
    static apsAllocator* mBlockAllocator; // ?mBlockAllocator@apsCommon@@0PAVapsAllocator@@A
    static float* (*mSplineCallback)(unsigned int);  // ?mSplineCallback@apsCommon@@0P6APAMI@ZA
    static unsigned int mFlags;           // ?mFlags@apsCommon@@0IA
    static int  mPakAllocs;               // ?mPakAllocs@apsCommon@@0HA
    static nglTexture* mDepthBufferTexture;  // ?mDepthBufferTexture@apsCommon@@0PAUnglTexture@@A
    static apsClient* mApsClient;         // ?mApsClient@apsCommon@@0PAVapsClient@@A
    static float mChanceToRemove;         // ?mChanceToRemove@apsCommon@@0MA
    static PlayerViewPort mViewPort[MAX_NUM_VIEWPORTS];  // ?mViewPort@apsCommon@@0PAUPlayerViewPort@1@A
};

#endif // COD3_AEPS_APSCOMMON_H
