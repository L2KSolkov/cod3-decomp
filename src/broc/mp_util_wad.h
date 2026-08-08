// ============================================================================
// mp_util_wad.h - multiplayer Broc entity-attribute accessors + anim wrappers.
// Source: mp_util_wad.cpp (MPBrocCore_xboxd:mp_util_wad.o)
// ============================================================================

#ifndef COD3_BROC_MP_UTIL_WAD_H
#define COD3_BROC_MP_UTIL_WAD_H

#include "engine/broc_types.h"

namespace mp_util_wad {

void RegisterHashStrings();

// Animation wrappers (forward to mp_anim_wad).
int ResolveAnim(unsigned int treename, unsigned int animname,
                unsigned int* getVal, unsigned int setVal);
const char* ResolveAnimName(unsigned int anim);
bool ValidateAnimationIndices();
unsigned int GetBroAnim(unsigned int treename, unsigned int animname);

// Entity-attribute accessors (GetEE_<field> / IsEEDefined_<field>).
#define MP_UTIL_WAD_ACCESSOR(NAME, TYPE)                                      \
    TYPE* GetEE_##NAME(Broc::entity ent);                                     \
    Broc::bbool* IsEEDefined_##NAME(Broc::bbool* result, Broc::entity ent);

MP_UTIL_WAD_ACCESSOR(flagEnd, Broc::vector)
MP_UTIL_WAD_ACCESSOR(audio_indoor, Broc::bint)
MP_UTIL_WAD_ACCESSOR(audio_ambmax, Broc::bfloat)
MP_UTIL_WAD_ACCESSOR(sound, Broc::bint)
MP_UTIL_WAD_ACCESSOR(specialWeaponChangeClassFlag, Broc::bbool)
MP_UTIL_WAD_ACCESSOR(respawnmodel, Broc::string)
MP_UTIL_WAD_ACCESSOR(deathmodel, Broc::string)
MP_UTIL_WAD_ACCESSOR(punishedTeamKills, Broc::bint)
MP_UTIL_WAD_ACCESSOR(firstSpectate, Broc::bbool)
MP_UTIL_WAD_ACCESSOR(home_position, Broc::vector)
MP_UTIL_WAD_ACCESSOR(deathfire, Broc::string)
MP_UTIL_WAD_ACCESSOR(waiting, Broc::bfloat)
MP_UTIL_WAD_ACCESSOR(last_dropped_time, Broc::bint)
MP_UTIL_WAD_ACCESSOR(flag_in_minefield, Broc::bbool)
MP_UTIL_WAD_ACCESSOR(index, Broc::bint)
MP_UTIL_WAD_ACCESSOR(lastPainSoundTime, Broc::bint)
MP_UTIL_WAD_ACCESSOR(audio_ambp, Broc::string)
MP_UTIL_WAD_ACCESSOR(holder, Broc::entity)
MP_UTIL_WAD_ACCESSOR(flag, Broc::entity)
MP_UTIL_WAD_ACCESSOR(lastTouch, Broc::bint)
MP_UTIL_WAD_ACCESSOR(audio_track_p, Broc::bint)
MP_UTIL_WAD_ACCESSOR(specialWeaponTime, Broc::bint)
MP_UTIL_WAD_ACCESSOR(autobalance, Broc::bbool)
MP_UTIL_WAD_ACCESSOR(goal, Broc::entity)
MP_UTIL_WAD_ACCESSOR(teamSound, Broc::bint)
MP_UTIL_WAD_ACCESSOR(trigger, Broc::entity)
MP_UTIL_WAD_ACCESSOR(message_when_returned, HashStr)
MP_UTIL_WAD_ACCESSOR(damage_effect, Broc::bint)
MP_UTIL_WAD_ACCESSOR(spawnTime, Broc::bint)
MP_UTIL_WAD_ACCESSOR(pickupCaptureDelayTime, Broc::bint)
MP_UTIL_WAD_ACCESSOR(flagStart, Broc::vector)
MP_UTIL_WAD_ACCESSOR(capAllowedTeam, Broc::bint)
MP_UTIL_WAD_ACCESSOR(last_touch_time, Broc::bint)
MP_UTIL_WAD_ACCESSOR(audio_track, Broc::string)
MP_UTIL_WAD_ACCESSOR(objective_status, Broc::bint)
MP_UTIL_WAD_ACCESSOR(damagelight, Broc::string)
MP_UTIL_WAD_ACCESSOR(spawnCount, Broc::bint)
MP_UTIL_WAD_ACCESSOR(capStatus, Broc::bfloat)
MP_UTIL_WAD_ACCESSOR(capTeam, Broc::bint)
MP_UTIL_WAD_ACCESSOR(numDeaths, Broc::bint)
MP_UTIL_WAD_ACCESSOR(reverb, Broc::string)
MP_UTIL_WAD_ACCESSOR(killsSinceLastDeath, Broc::bint)
MP_UTIL_WAD_ACCESSOR(home_angles, Broc::vector)
MP_UTIL_WAD_ACCESSOR(inDeath, Broc::bint)
MP_UTIL_WAD_ACCESSOR(sound_handle, Broc::bint)
MP_UTIL_WAD_ACCESSOR(capSpeed, Broc::bfloat)
MP_UTIL_WAD_ACCESSOR(lastspawnpoint, Broc::entity)
MP_UTIL_WAD_ACCESSOR(setting_up_hq, Broc::bint)
MP_UTIL_WAD_ACCESSOR(weaponstr, Broc::string)
MP_UTIL_WAD_ACCESSOR(damagecritical, Broc::string)
MP_UTIL_WAD_ACCESSOR(spawnscore, Broc::bint)
MP_UTIL_WAD_ACCESSOR(deathfx, Broc::string)
MP_UTIL_WAD_ACCESSOR(bTeamKilledWithArtillery, Broc::bbool)
MP_UTIL_WAD_ACCESSOR(damageheavy, Broc::string)
MP_UTIL_WAD_ACCESSOR(cappedSinceLastDeath, Broc::bint)
MP_UTIL_WAD_ACCESSOR(isInsideFollowClient, Broc::bint)
MP_UTIL_WAD_ACCESSOR(returnedSinceLastDeath, Broc::bint)
MP_UTIL_WAD_ACCESSOR(lastMedicCall, Broc::bint)
MP_UTIL_WAD_ACCESSOR(bigsplashed, Broc::bint)
MP_UTIL_WAD_ACCESSOR(audio_ambmin, Broc::bfloat)

#undef MP_UTIL_WAD_ACCESSOR

} // namespace mp_util_wad

#endif // COD3_BROC_MP_UTIL_WAD_H
