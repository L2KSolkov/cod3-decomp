// ============================================================================
// apsGroupManager — singleton manager for particle groups (7 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsGroupMgr.cpp
// Verified against IDA (aeps_xboxr:apsGroupMgr.o):
//   ctor(int)          @0x806770 (??0apsGroupManager@@QAE@H@Z)
//   dtor              @0x806A60 (??1apsGroupManager@@QAE@XZ)
//   CreateGroup       @0x8067F0 (?CreateGroup@apsGroupManager@@QAEPAVapsGroup@@HPAVapsRenderer@@ABVapsPFD@@I@Z)
//   DestroyGroup      @0x8068C0 (?DestroyGroup@apsGroupManager@@QAEXPAVapsGroup@@I@Z)
//   DestroyAllGroups  @0x806A30 (?DestroyAllGroups@apsGroupManager@@QAEXXZ)
//   TestAlloc         @0x8065D0 (?TestAlloc@apsGroupManager@@QAEIH@Z)
//   Report            @0x806620 (?Report@apsGroupManager@@QAEXXZ)
// Layout: apsSingleton<apsGroupManager> base @0x00, mMaxGroups @0x00,
//   mGroups (apsArray<apsGroup*>) @0x04. Size 12 bytes.
// ============================================================================
#ifndef COD3_AEPS_APSGROUPMGR_H
#define COD3_AEPS_APSGROUPMGR_H

#include "apsUtil.h"
#include "apsAction.h"   // apsArray<T>
#include "apsGroup.h"
#include "apsRenderer.h"
#include "apsPFD.h"
#include "apsMemory.h"

class apsGroupManager;

// apsSingleton<apsGroupManager> (sInstancePtr owned by apsGroupMgr.o)
extern template class apsSingleton<apsGroupManager>;

// ============================================================================
// apsGroupManager — singleton group manager.
// ============================================================================
class apsGroupManager : public apsSingleton<apsGroupManager> {
public:
    apsGroupManager(int maxGroups);              // ??0apsGroupManager@@QAE@H@Z
    ~apsGroupManager();                          // ??1apsGroupManager@@QAE@XZ

    apsGroup* CreateGroup(int iMaxNumParticles, apsRenderer* iRenderer,
                          const apsPFD& iPFD, unsigned int iLocalSpace);  // ?CreateGroup@apsGroupManager@@QAEPAVapsGroup@@HPAVapsRenderer@@ABVapsPFD@@I@Z
    void DestroyGroup(apsGroup* iGroup, unsigned int bFastDeath);  // ?DestroyGroup@apsGroupManager@@QAEXPAVapsGroup@@I@Z
    void DestroyAllGroups();                     // ?DestroyAllGroups@apsGroupManager@@QAEXXZ
    unsigned int TestAlloc(int numGroups);       // ?TestAlloc@apsGroupManager@@QAEIH@Z
    void Report();                               // ?Report@apsGroupManager@@QAEXXZ

    int mMaxGroups;              // +0x00
    apsArray<apsGroup*> mGroups; // +0x04 (8 bytes)
};
static_assert(sizeof(apsGroupManager) == 0x0C, "apsGroupManager size mismatch");

#endif // COD3_AEPS_APSGROUPMGR_H
