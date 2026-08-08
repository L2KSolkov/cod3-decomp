// ============================================================================
// mp_anim_wad.h - animation wad: hash strings + entity-attr accessors + anim wrappers.
// Source: mp_anim_wad.cpp (MPBrocCore_xboxd:mp_anim_wad.o)
// ============================================================================

#ifndef COD3_BROC_MP_ANIM_WAD_H
#define COD3_BROC_MP_ANIM_WAD_H

#include "engine/broc_types.h"

namespace mp_anim_wad {

void RegisterHashStrings();
int ResolveAnim(unsigned int treename, unsigned int animname,
                unsigned int* getVal, unsigned int setVal);
const char* ResolveAnimName(unsigned int anim);
bool ValidateAnimationIndices();
unsigned int GetBroAnim(unsigned int treename, unsigned int animname);

} // namespace mp_anim_wad

#endif // COD3_BROC_MP_ANIM_WAD_H
