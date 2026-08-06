// ============================================================================
// apsActionList — ordered list of actions applied to a particle group.
// Source: c:\cod\code\tl\aeps\source\apsActionList.cpp
// Verified against IDA (aeps_xboxr:apsActionList.o):
//   ctor       @0x808C90 (??0apsActionList@@QAE@XZ)
//   dtor       @0x808CA0 (??1apsActionList@@QAE@XZ)
//   Add        @0x808D30 (?Add@apsActionList@@QAEXPAVapsAction@@@Z)
//   Apply      @0x808830 (?Apply@apsActionList@@QAEXPAVapsGroup@@AAVapsEffect@@MMII@Z)
//   EnhancePFD @0x808BA0 (?EnhancePFD@apsActionList@@QAEXAAVapsPFD@@@Z)
//   Fixup      @0x808BE0 (?Fixup@apsActionList@@QAEXABUapsFixupParams@@@Z)
//   GetAction  @0x7F0030 (?GetAction@apsActionList@@QAEPAVapsAction@@H@Z) inline
// Layout: single apsArray<apsAction*> mActions (8 bytes). No vtable.
// g_actionList (apsActionList*) lives at 0x10E0A20.
// ============================================================================
#ifndef COD3_AEPS_APSACTIONLIST_H
#define COD3_AEPS_APSACTIONLIST_H

#include "apsAction.h"
#include "apsEffect.h"
#include "apsGroup.h"
#include "apsPFD.h"

class apsEffect;
struct apsFixupParams;

// ============================================================================
// apsActionList — ordered list of actions applied to a particle group.
// ============================================================================
class apsActionList {
public:
    apsActionList();                 // ??0apsActionList@@QAE@XZ (apsActionList.o)
    ~apsActionList();                // ??1apsActionList@@QAE@XZ

    void Add(apsAction* iAction);        // ?Add@apsActionList@@QAEXPAVapsAction@@@Z
    void Apply(apsGroup* iGroup, apsEffect& iEffect, float iElapsedTime,
               float iTimeDelta, unsigned int iNoSources,
               unsigned int doChanceToRemove);   // ?Apply@apsActionList@@QAEXPAVapsGroup@@AAVapsEffect@@MMII@Z
    void EnhancePFD(apsPFD& ioPFD);      // ?EnhancePFD@apsActionList@@QAEXAAVapsPFD@@@Z
    void Fixup(const apsFixupParams& iFixupParams);  // ?Fixup@apsActionList@@QAEXABUapsFixupParams@@@Z

    // inline accessor (emitted in apsEffectTemplate.o COMDATs):
    apsAction* GetAction(int iNum) {     // ?GetAction@apsActionList@@QAEPAVapsAction@@H@Z (ea: 0x7F0030)
        if (iNum >= mActions.mSize &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsActionList.h", 59,
                      "iNum < mActions.size()", "Index out of range"))
            __debugbreak();
        if (iNum >= 0 && iNum < mActions.mSize)
            return mActions.mElements[iNum];
        if (!_tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                       "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            return mActions.mElements[iNum];
        apsAction* result = mActions.mElements[iNum];
        __debugbreak();
        return result;
    }

    apsArray<apsAction*> mActions;  // +0x00 (8 bytes)
};
static_assert(sizeof(apsActionList) == 8, "apsActionList size mismatch");

// Data (apsActionList.o)
extern apsActionList* g_actionList;   // ?g_actionList@@3PAVapsActionList@@A (0x10E0A20)

#endif // COD3_AEPS_APSACTIONLIST_H
