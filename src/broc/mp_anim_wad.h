// ============================================================================
// mp_anim_wad.h - animation wad: hash strings + entity-attr accessors + anim wrappers.
// Source: mp_anim_wad.cpp (MPBrocCore_xboxd:mp_anim_wad.o)
// ============================================================================

#ifndef COD3_BROC_MP_ANIM_WAD_H
#define COD3_BROC_MP_ANIM_WAD_H

#include "engine/broc_types.h"

namespace mp_anim_wad {

// IDA layout: generated level/animation objects carry a vtable and an entity
// handle; the multiplayer wrappers derive from these bases.
struct Level {
    void*        vftable;
    Broc::entity entity;
    unsigned char flags;
    unsigned char _pad09[3];
};

struct Anim {
    void*        vftable;
    Broc::entity entity;
};

static_assert(sizeof(Level) == 0x0C, "mp_anim_wad::Level size mismatch");
static_assert(sizeof(Anim) == 0x08, "mp_anim_wad::Anim size mismatch");

extern Level* pLevel;
extern Anim* pAnim;

void RegisterHashStrings();
unsigned int ResolveAnim(unsigned int treename, unsigned int animname,
                         unsigned int* getVal, unsigned int setVal);
const char* ResolveAnimName(unsigned int anim);
bool ValidateAnimationIndices();
unsigned int GetBroAnim(unsigned int treename, unsigned int animname);

} // namespace mp_anim_wad

#endif // COD3_BROC_MP_ANIM_WAD_H
