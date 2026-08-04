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

    static nglScene* mBuildScene;
};

#endif // COD3_AEPS_APSCOMMON_H
