// ============================================================================
// apsEffect / apsEffectTemplate — shared minimal views.
// Source: c:\cod\code\tl\aeps\include\apsEffect.h
// Full class definitions belong to apsEffect.o / apsEffectTemplate.o (unported).
// Only the members used by already-ported aeps objects are declared here.
// Layout verified against IDA:
//   apsEffect (128 bytes): mTemplate @0x48, mParentAgePercent @0x74.
//   apsEffectTemplate (208 bytes): mUsesUpdateLod @0x70.
// ============================================================================
#ifndef COD3_AEPS_APSEFFECT_H
#define COD3_AEPS_APSEFFECT_H

#include "core/math_types.h"

#include <cstddef>

struct apsBounds;

class apsEffectTemplate {
public:
    int GetUsesUpdateLod() const { return mUsesUpdateLod; }  // ?GetUsesUpdateLod@apsEffectTemplate@@QBEHXZ (inline)

    char _pad[0x70];            // +0x00
    int  mUsesUpdateLod;        // +0x70
};
static_assert(offsetof(apsEffectTemplate, mUsesUpdateLod) == 0x70, "apsEffectTemplate::mUsesUpdateLod offset mismatch");

class apsEffect {
public:
    void SetLocalToWorldTransform(const math::Mat43& iMatrix);  // apsEffect.o (non-inline)
    void SetParentAgePercent(float age) { mParentAgePercent = age; }  // inline COMDAT (apsInternal.o)
    static void ReportEffects();                              // ?ReportEffects@apsEffect@@SAXXZ (apsEffect.o)
    void AccumulateCollisionBounds(const apsBounds& iBounds); // apsEffect.o (non-inline)

    char  _pad0[0x48];               // +0x00
    const apsEffectTemplate* mTemplate;  // +0x48
    char  _pad4C[0x74 - 0x4C];       // +0x4C
    float mParentAgePercent;         // +0x74
};
static_assert(offsetof(apsEffect, mTemplate) == 0x48, "apsEffect::mTemplate offset mismatch");
static_assert(offsetof(apsEffect, mParentAgePercent) == 0x74, "apsEffect::mParentAgePercent offset mismatch");

#endif // COD3_AEPS_APSEFFECT_H
