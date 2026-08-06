// ============================================================================
// apsCommon — shared APS globals / helpers.
// Source: c:\cod\code\tl\aeps\source\apsCommon.cpp
// apsCommon::GetBuildScene  @0x7FE9E0  (inline, emitted in apsGroup.o)
// apsCommon::mBuildScene    @0x10DECFC (data)
// ============================================================================
#ifndef COD3_AEPS_APSCOMMON_H
#define COD3_AEPS_APSCOMMON_H

#include "core/math_types.h"

struct nglScene;

// Minimal apsAllocator (full class in apsCommon.o once ported).
// vftable slots (verified from apsGroup.o calls): [0]=~dtor, [1]=MemAlign, [2]=MemFree.
class apsAllocator {
public:
    virtual ~apsAllocator();
    virtual void* MemAlign(unsigned int iSize, unsigned int iAlignment);
    virtual void MemFree(void* iPtr);
};

class apsCommon {
public:
    // Camera view-basis / handedness settings used by apsMath::CreateFromVectorsDirUp.
    // Layout verified against IDA (64 bytes):
    //   mDir @0x00, mUp @0x10, mLeft @0x20, mRoll @0x30, mXFlip @0x34
    struct CameraSettings {
        math::Dir3 mDir;    // +0x00
        math::Dir3 mUp;     // +0x10
        math::Dir3 mLeft;   // +0x20
        float mRoll;        // +0x30
        float mXFlip;       // +0x34
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
    };

    static nglScene* GetBuildScene() { return mBuildScene; }

    static apsAllocator* GetAllocator();
    static int GetCurrentPakId();
    static void SetCurrentPakId(int iId);
    static void SetPakAllocs(int iValue);

    // apsCommon.o (non-inline): per-player view port. Unresolved here.
    static PlayerViewPort* GetPlayerViewPort(unsigned int playerId);

    // NOTE: map shows these statics with @@0 (private) mangling
    // (?mCamera@apsCommon@@0UCameraSettings@1@A, ?mBuildScene@apsCommon@@0PAUnglScene@@A,
    //  ?mShimmerScene@apsCommon@@0PAUnglScene@@A). apsGroup.cpp and the renderer
    //  headers touch them directly, so they are kept public here (mirrors the
    //  existing apsGroup port). TODO(apsCommon.o): reconcile access specifiers.
    static CameraSettings mCamera;   // apsCommon.o (unresolved here)
    static nglScene* mBuildScene;    // apsCommon.o (unresolved here)
    static nglScene* mShimmerScene;  // apsCommon.o (unresolved here)
};

#endif // COD3_AEPS_APSCOMMON_H
