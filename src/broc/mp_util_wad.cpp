// ============================================================================
// mp_util_wad.cpp - multiplayer Broc entity-attribute accessors + anim wrappers.
// Source: mp_util_wad.cpp (MPBrocCore_xboxd:mp_util_wad.o)
// Verified against IDA (MPBrocCore_xboxd:mp_util_wad.o).
// ============================================================================

#include "mp_util_wad.h"
#include "engine/broc_types.h"
#include <string.h>

using Broc::RandomFloatRange;
using Broc::RandomInt;
using Broc::wait;
using Broc::wait_accurate;
using Broc::thread_create;
using Broc::GetEnt;
using Broc::Delete;
using Broc::Spawn;
using Broc::SetModel;
using Broc::MoveTo;
using Broc::EffectEventPlay;

namespace mp_anim_wad {
int ResolveAnim(unsigned int treename, unsigned int animname,
                unsigned int* getVal, unsigned int setVal);
const char* ResolveAnimName(unsigned int anim);
bool ValidateAnimationIndices();
unsigned int GetBroAnim(unsigned int treename, unsigned int animname);
void RegisterHashStrings();
}

// ============================================================================
// Thread-functor externs (stubs at the bottom until each script is ported).
// ============================================================================
namespace _mp_audio {
void* PlayPainSound__functor(Broc::entity guy, Broc::bint damage);
void* PlaySound__functor(Broc::entity self, Broc::string sound, float delay);
void* PlaySoundAtLocation__functor(Broc::entity self, Broc::string sound,
                                   Broc::vector position);
void* player_dying_sounds__functor(Broc::entity player);
void* audio_crossfade_wait__functor(Broc::entity self);
void* ThreadStaticSoundPlay__functor(Broc::entity self, Broc::string name);
void* ThreadStaticSoundRandomPlay__functor(Broc::entity self,
                                           Broc::string name);
void* MoveSoundAlongLine__functor(Broc::entity toMove, Broc::vector start,
                                  Broc::vector end);
void PlayDeathSound(Broc::entity guy, Broc::entity inflictor,
                    Broc::entity attacker, Broc::bint weapon,
                    Broc::bint means_of_damage);
}
namespace _mp_loadout {
void local_player_joined(Broc::entity player);
void GiveLoadout(Broc::entity player);
void GiveSpecialWeapon(Broc::entity player, int playerClass, int rank,
                       bool isRespawn);
void UpdatePlayerModelForRank(Broc::entity player);
void DisplayYouWillSpawnWithMessage(Broc::entity self);
}
namespace _mp_shellshock {
void ShellshockOnDamage(Broc::entity self, Broc::bint cause, Broc::bint damage);
}
namespace _mp_spawnlogic {
Broc::entity* GetSpawnpointNearTeamAntiCamp(Broc::entity* result,
                                            Broc::entity* self,
                                            const Broc::string* team,
                                            Broc::dyn_array<Broc::entity>* points);
Broc::entity* GetSpawnpointNearest(Broc::entity* result,
                                   Broc::dyn_array<Broc::entity>* points,
                                   Broc::vector position, bool ignoreTeleFrag);
Broc::entity* GetSpawnpointRandom(Broc::entity* result,
                                  Broc::dyn_array<Broc::entity>* points,
                                  bool ignoreTeleFrag);
}
namespace _mp_teambalance {
Broc::string* team_balance(Broc::string* result, Broc::entity guy,
                           Broc::string team);
void team_balance(bool always);
}
namespace _mp_common {
void* StopFollowing__functor(Broc::entity self, bool blackNow);
void* QuitGameThread__functor(Broc::entity selfLevel);
void* QuitGameWithMessage__functor(Broc::entity self, HashStr message);
void* HostHasMigrated__functor(Broc::entity self);
void* RespawnPlayer__functor(Broc::entity guy, Broc::string team);
void* LocalPlayerRespawn__functor(Broc::entity player);
void* PunishedForTeamKill__functor(Broc::entity ent, bool punished);
void* reenable_medic_call__functor(Broc::entity self, int time);
void* HandleJoinAfterRoundOver__functor(Broc::entity self, int timeleft);
void* TeamChangeKillPlayer__functor(Broc::entity player);
void* FadeUpWhenLoaded__functor(Broc::entity self, Broc::entity player);
void* HealthRegenPlayerBreathing__functor(Broc::entity self, int healthCap);
void* DeathState__functor(Broc::entity player, Broc::entity team_killer,
                          int delay, bool reviveable, bool fade);
void* UpdateSpectateCritical__functor(Broc::entity guy);
void* UpdateSpectateCriticalGoingToDie__functor(Broc::entity guy);
void* UpdateSpectateDead__functor(Broc::entity guy, bool canspawn);
void* UpdateSpectateSpawn__functor(Broc::entity localPlayer);
void* SpawnLocalSpectator__functor(Broc::entity guy);
void* restart_round__functor(Broc::entity selfLevel, int waitTime);
void* finish_starting_round__functor(Broc::entity self, bool firstTime);
void* AddArtilleryObjective__functor(Broc::entity self, Broc::vector position);
void* NewHost__functor(Broc::entity self);
void* LocalPlayerIntermission__functor(Broc::entity player);
}

namespace mp_util_wad {

extern void RegisterHashString(int h, const char* txt);

void RegisterHashStrings() {
    RegisterHashString(19721985, "ClearGame");
    RegisterHashString(1635421430, "GiveSpecialAmmo");
    RegisterHashString(1394268377, "HostGameState");
    RegisterHashString(-1468779911, "KillCheckForLastManStanding");
    RegisterHashString(-918270835, "KillSpecialDispensers");
    RegisterHashString(1606023905, "KillSpecialTimers");
    RegisterHashString(-62352108, "MPCTF_ALLIES_FLAG_RETURNED");
    RegisterHashString(1954802383, "MPCTF_AXIS_FLAG_RETURNED");
    RegisterHashString(-757809199, "MPGAME_ARTILLERY_UNAVAILABLE");
    RegisterHashString(-1349369391, "MPHQ_DEFENDING_HQ");
    RegisterHashString(1634796373, "MPHQ_DESTROYING_HQ");
    RegisterHashString(1527058809, "MPHQ_LOSING_HQ");
    RegisterHashString(2055901797, "MPHQ_MUST_CLEAR_ATTACKERS");
    RegisterHashString(-1638391765, "MPHQ_MUST_CLEAR_DEFENDERS_HQ");
    RegisterHashString(139802703, "MPHQ_SETTING_UP_HQ");
    RegisterHashString(1851551928, "MPSCRIPT_ALLIES_WIN");
    RegisterHashString(-1309493746, "MPSCRIPT_ARTILLERY_FIRE");
    RegisterHashString(512252216, "MPSCRIPT_AUTOBALANCE_DENIED");
    RegisterHashString(268424555, "MPSCRIPT_AUTOBALANCE_TO_ALLIES");
    RegisterHashString(-713596442, "MPSCRIPT_AUTOBALANCE_TO_AXIS");
    RegisterHashString(-1944069453, "MPSCRIPT_AXIS_WIN");
    RegisterHashString(1777663366, "MPSCRIPT_DO_NOT_TEAMKILL");
    RegisterHashString(-448578327, "MPSCRIPT_FORGIVEN_FOR_TEAMKILL");
    RegisterHashString(-1537719130, "MPSCRIPT_GAME_OVER");
    RegisterHashString(903886296, "MPSCRIPT_HOST_CHANGED");
    RegisterHashString(-229290828, "MPSCRIPT_KICKED_FOR_TEAMKILL");
    RegisterHashString(-1174342434, "MPSCRIPT_MINE_FAILED");
    RegisterHashString(-1299494679, "MPSCRIPT_PUNISHED_FOR_TEAMKILL");
    RegisterHashString(-826761027, "MPSCRIPT_ROUND_ENDED_NO_ALLIED_PLAYERS");
    RegisterHashString(-196661561, "MPSCRIPT_ROUND_ENDED_NO_AXIS_PLAYERS");
    RegisterHashString(-1617013727, "MPSCRIPT_ROUND_ENDED_NO_OTHER_PLAYER");
    RegisterHashString(1680009965, "MPSCRIPT_STARTING_NEW_ROUND");
    RegisterHashString(-810802202, "MPSCRIPT_WAITING_FOR_ANOTHER_PLAYER");
    RegisterHashString(-863664852, "MPSCRIPT_WAITING_FOR_PLAYERS_ON_OTHER_TEAM");
    RegisterHashString(-1958431126, "MPSCRIPT_WARNED_FOR_TEAMKILL");
    RegisterHashString(1300509333, "MPSCRIPT_YOU_ARE_NOW_THE_HOST");
    RegisterHashString(893827924, "MPSCRIPT_YOU_HAVE_BEEN_KICKED");
    RegisterHashString(-53887149, "MPSCRIPT_YOU_WILL_GET_WEAPON_NEXT_SPAWN");
    RegisterHashString(1795254654, "MPWAR_FLAG_CAN_NOT_BE_CAPTURED");
    RegisterHashString(1872901735, "NextRound");
    RegisterHashString(-1647123971, "ResetGame");
    RegisterHashString(-2042933948, "RoundOver");
    RegisterHashString(1307324022, "RoundStart");
    RegisterHashString(-1343754252, "RoundStarting");
    RegisterHashString(-289527416, "SCFEndFlagLaunch");
    RegisterHashString(-1489447140, "StopWaitForNoTouchFlag");
    RegisterHashString(23543191, "_fx_fast_sound_done");
    RegisterHashString(-967276121, "audio_ambmax");
    RegisterHashString(-967275867, "audio_ambmin");
    RegisterHashString(-1424656079, "audio_ambp");
    RegisterHashString(-652932548, "audio_indoor");
    RegisterHashString(253700422, "audio_track");
    RegisterHashString(1401855861, "audio_track_p");
    RegisterHashString(-70479137, "autobalance");
    RegisterHashString(1294686002, "bTeamKilledWithArtillery");
    RegisterHashString(1710938758, "bigsplashed");
    RegisterHashString(-198128317, "capAllowedTeam");
    RegisterHashString(-1779939835, "capSpeed");
    RegisterHashString(1396144536, "capStatus");
    RegisterHashString(-1355418981, "capTeam");
    RegisterHashString(-1427096775, "cappedSinceLastDeath");
    RegisterHashString(-262367393, "damage");
    RegisterHashString(1975498379, "damage_effect");
    RegisterHashString(-814616150, "damagecritical");
    RegisterHashString(-1442837060, "damageheavy");
    RegisterHashString(-1437943561, "damagelight");
    RegisterHashString(-1844960460, "deadstop");
    RegisterHashString(122331302, "death");
    RegisterHashString(-141541364, "deathfire");
    RegisterHashString(74805188, "deathfx");
    RegisterHashString(-367395785, "deathmodel");
    RegisterHashString(1282694090, "disconnect");
    RegisterHashString(1128239878, "fire_special");
    RegisterHashString(1855837902, "fireextinguish");
    RegisterHashString(1912681025, "firstSpectate");
    RegisterHashString(-1363748623, "flagEnd");
    RegisterHashString(953253096, "flagStart");
    RegisterHashString(-94902329, "flag_dropped");
    RegisterHashString(981991772, "flag_in_minefield");
    RegisterHashString(-2025829234, "flag_pickedup");
    RegisterHashString(1328252895, "flag_time_out");
    RegisterHashString(-1353926140, "flipped");
    RegisterHashString(-89255650, "holder");
    RegisterHashString(945124834, "home_angles");
    RegisterHashString(-466622403, "home_position");
    RegisterHashString(-1702559491, "inDeath");
    RegisterHashString(128587128, "index");
    RegisterHashString(615889508, "intermission");
    RegisterHashString(761984554, "isInsideFollowClient");
    RegisterHashString(-1843290650, "killanimscript");
    RegisterHashString(-1192080693, "killsSinceLastDeath");
    RegisterHashString(-1013300123, "killspectate");
    RegisterHashString(-1567129262, "lastMedicCall");
    RegisterHashString(1985257780, "lastPainSoundTime");
    RegisterHashString(1424162999, "lastTouch");
    RegisterHashString(531017007, "last_dropped_time");
    RegisterHashString(-1207793244, "last_touch_time");
    RegisterHashString(-1700438009, "lastspawnpoint");
    RegisterHashString(1903933022, "message_when_returned");
    RegisterHashString(1406828841, "mine_explosion");
    RegisterHashString(-1095144119, "numDeaths");
    RegisterHashString(919337182, "objective_status");
    RegisterHashString(1195579518, "pickupCaptureDelayTime");
    RegisterHashString(1895485417, "playerclasschange");
    RegisterHashString(-405774630, "playerteamchange");
    RegisterHashString(891589254, "punishedTeamKills");
    RegisterHashString(-2030953967, "respawnmodel");
    RegisterHashString(-873800299, "returnedSinceLastDeath");
    RegisterHashString(290599942, "reverb");
    RegisterHashString(290604433, "revive");
    RegisterHashString(1688311674, "setting_up_hq");
    RegisterHashString(140501065, "sound");
    RegisterHashString(-1961364620, "sound_handle");
    RegisterHashString(140515529, "spawn");
    RegisterHashString(474633906, "spawnCount");
    RegisterHashString(-115163560, "spawnTime");
    RegisterHashString(493170981, "spawnscore");
    RegisterHashString(1279005857, "specialWeaponChangeClassFlag");
    RegisterHashString(349586970, "specialWeaponTime");
    RegisterHashString(-1041826087, "spectate");
    RegisterHashString(1786638118, "spectate_state_dead");
    RegisterHashString(802120496, "teamSound");
    RegisterHashString(1570888237, "team_sound_stopped");
    RegisterHashString(305160092, "treadtypechanged");
    RegisterHashString(-218764620, "trigger");
    RegisterHashString(-808269429, "turret_fire");
    RegisterHashString(-1304160045, "waiting");
    RegisterHashString(-1812252157, "weaponstr");
    RegisterHashString(714788760, "zonesloaded");
    mp_anim_wad::RegisterHashStrings();
}

// ============================================================================
// Animation wrappers (forward to mp_anim_wad)
// ============================================================================
int ResolveAnim(unsigned int treename, unsigned int animname,
               unsigned int* getVal, unsigned int setVal) {
    return mp_anim_wad::ResolveAnim(treename, animname, getVal, setVal) != 0;
}

const char* ResolveAnimName(unsigned int anim) {
    return mp_anim_wad::ResolveAnimName(anim);
}

bool ValidateAnimationIndices() {
    mp_anim_wad::ValidateAnimationIndices();
    return mp_anim_wad::ValidateAnimationIndices();
}

unsigned int GetBroAnim(unsigned int treename, unsigned int animname) {
    return mp_anim_wad::GetBroAnim(treename, animname);
}

// GetEE_flagEnd / IsEEDefined_flagEnd (key 0xAEB6D8F1)
Broc::vector* GetEE_flagEnd(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::vector>(0xAEB6D8F1);
}

Broc::bbool* IsEEDefined_flagEnd(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::vector v; const Broc::vector* val = ee->GetVal<Broc::vector>(&v, 0xAEB6D8F1);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_audio_indoor / IsEEDefined_audio_indoor (key 0xD9150A3C)
Broc::bint* GetEE_audio_indoor(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xD9150A3C);
}

Broc::bbool* IsEEDefined_audio_indoor(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xD9150A3C);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_audio_ambmax / IsEEDefined_audio_ambmax (key 0xC65889A7)
Broc::bfloat* GetEE_audio_ambmax(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0xC65889A7);
}

Broc::bbool* IsEEDefined_audio_ambmax(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0xC65889A7);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_sound / IsEEDefined_sound (key 0x085FE049)
Broc::bint* GetEE_sound(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x085FE049);
}

Broc::bbool* IsEEDefined_sound(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x085FE049);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_specialWeaponChangeClassFlag / IsEEDefined_specialWeaponChangeClassFlag (key 0x4C3C14A1)
Broc::bbool* GetEE_specialWeaponChangeClassFlag(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bbool>(0x4C3C14A1);
}

Broc::bbool* IsEEDefined_specialWeaponChangeClassFlag(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bbool v; const Broc::bbool* val = ee->GetVal<Broc::bbool>(&v, 0x4C3C14A1);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_punishedTeamKills / IsEEDefined_punishedTeamKills (key 0x35249286)
Broc::bint* GetEE_punishedTeamKills(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x35249286);
}

Broc::bbool* IsEEDefined_punishedTeamKills(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x35249286);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_firstSpectate / IsEEDefined_firstSpectate (key 0x72013241)
Broc::bbool* GetEE_firstSpectate(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bbool>(0x72013241);
}

Broc::bbool* IsEEDefined_firstSpectate(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bbool v; const Broc::bbool* val = ee->GetVal<Broc::bbool>(&v, 0x72013241);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_home_position / IsEEDefined_home_position (key 0xE42FE83D)
Broc::vector* GetEE_home_position(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::vector>(0xE42FE83D);
}

Broc::bbool* IsEEDefined_home_position(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::vector v; const Broc::vector* val = ee->GetVal<Broc::vector>(&v, 0xE42FE83D);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_waiting / IsEEDefined_waiting (key 0xB24418D3)
Broc::bfloat* GetEE_waiting(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0xB24418D3);
}

Broc::bbool* IsEEDefined_waiting(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0xB24418D3);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_last_dropped_time / IsEEDefined_last_dropped_time (key 0x1FA6AD2F)
Broc::bint* GetEE_last_dropped_time(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x1FA6AD2F);
}

Broc::bbool* IsEEDefined_last_dropped_time(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x1FA6AD2F);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_flag_in_minefield / IsEEDefined_flag_in_minefield (key 0x3A88015C)
Broc::bbool* GetEE_flag_in_minefield(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bbool>(0x3A88015C);
}

Broc::bbool* IsEEDefined_flag_in_minefield(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bbool v; const Broc::bbool* val = ee->GetVal<Broc::bbool>(&v, 0x3A88015C);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_index / IsEEDefined_index (key 0x07AA1578)
Broc::bint* GetEE_index(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x07AA1578);
}

Broc::bbool* IsEEDefined_index(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x07AA1578);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_lastPainSoundTime / IsEEDefined_lastPainSoundTime (key 0x7654A134)
Broc::bint* GetEE_lastPainSoundTime(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x7654A134);
}

Broc::bbool* IsEEDefined_lastPainSoundTime(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x7654A134);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_lastTouch / IsEEDefined_lastTouch (key 0x54E300B7)
Broc::bint* GetEE_lastTouch(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x54E300B7);
}

Broc::bbool* IsEEDefined_lastTouch(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x54E300B7);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_audio_track_p / IsEEDefined_audio_track_p (key 0x538E9F75)
Broc::bint* GetEE_audio_track_p(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x538E9F75);
}

Broc::bbool* IsEEDefined_audio_track_p(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x538E9F75);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_specialWeaponTime / IsEEDefined_specialWeaponTime (key 0x14D6461A)
Broc::bint* GetEE_specialWeaponTime(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x14D6461A);
}

Broc::bbool* IsEEDefined_specialWeaponTime(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x14D6461A);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_autobalance / IsEEDefined_autobalance (key 0xFBCC92DF)
Broc::bbool* GetEE_autobalance(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bbool>(0xFBCC92DF);
}

Broc::bbool* IsEEDefined_autobalance(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bbool v; const Broc::bbool* val = ee->GetVal<Broc::bbool>(&v, 0xFBCC92DF);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_teamSound / IsEEDefined_teamSound (key 0x2FCF6330)
Broc::bint* GetEE_teamSound(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x2FCF6330);
}

Broc::bbool* IsEEDefined_teamSound(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x2FCF6330);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_message_when_returned / IsEEDefined_message_when_returned (key 0x717BB65E)
HashStr* GetEE_message_when_returned(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<HashStr>(0x717BB65E);
}

Broc::bbool* IsEEDefined_message_when_returned(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            HashStr v; const HashStr* val = ee->GetVal<HashStr>(&v, 0x717BB65E);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_damage_effect / IsEEDefined_damage_effect (key 0x75BFB68B)
Broc::bint* GetEE_damage_effect(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x75BFB68B);
}

Broc::bbool* IsEEDefined_damage_effect(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x75BFB68B);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_spawnTime / IsEEDefined_spawnTime (key 0xF922BE58)
Broc::bint* GetEE_spawnTime(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xF922BE58);
}

Broc::bbool* IsEEDefined_spawnTime(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xF922BE58);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_pickupCaptureDelayTime / IsEEDefined_pickupCaptureDelayTime (key 0x4743187E)
Broc::bint* GetEE_pickupCaptureDelayTime(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x4743187E);
}

Broc::bbool* IsEEDefined_pickupCaptureDelayTime(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x4743187E);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_flagStart / IsEEDefined_flagStart (key 0x38D17CE8)
Broc::vector* GetEE_flagStart(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::vector>(0x38D17CE8);
}

Broc::bbool* IsEEDefined_flagStart(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::vector v; const Broc::vector* val = ee->GetVal<Broc::vector>(&v, 0x38D17CE8);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_capAllowedTeam / IsEEDefined_capAllowedTeam (key 0xF430CD43)
Broc::bint* GetEE_capAllowedTeam(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xF430CD43);
}

Broc::bbool* IsEEDefined_capAllowedTeam(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xF430CD43);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_last_touch_time / IsEEDefined_last_touch_time (key 0xB80289A4)
Broc::bint* GetEE_last_touch_time(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xB80289A4);
}

Broc::bbool* IsEEDefined_last_touch_time(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xB80289A4);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_objective_status / IsEEDefined_objective_status (key 0x36CBF8DE)
Broc::bint* GetEE_objective_status(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x36CBF8DE);
}

Broc::bbool* IsEEDefined_objective_status(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x36CBF8DE);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_spawnCount / IsEEDefined_spawnCount (key 0x1C4A56B2)
Broc::bint* GetEE_spawnCount(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x1C4A56B2);
}

Broc::bbool* IsEEDefined_spawnCount(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x1C4A56B2);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_capStatus / IsEEDefined_capStatus (key 0x53377998)
Broc::bfloat* GetEE_capStatus(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x53377998);
}

Broc::bbool* IsEEDefined_capStatus(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x53377998);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_capTeam / IsEEDefined_capTeam (key 0xAF35F29B)
Broc::bint* GetEE_capTeam(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xAF35F29B);
}

Broc::bbool* IsEEDefined_capTeam(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xAF35F29B);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_numDeaths / IsEEDefined_numDeaths (key 0xBEB96D49)
Broc::bint* GetEE_numDeaths(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xBEB96D49);
}

Broc::bbool* IsEEDefined_numDeaths(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xBEB96D49);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_killsSinceLastDeath / IsEEDefined_killsSinceLastDeath (key 0xB8F24ACB)
Broc::bint* GetEE_killsSinceLastDeath(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xB8F24ACB);
}

Broc::bbool* IsEEDefined_killsSinceLastDeath(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xB8F24ACB);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_home_angles / IsEEDefined_home_angles (key 0x385575E2)
Broc::vector* GetEE_home_angles(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::vector>(0x385575E2);
}

Broc::bbool* IsEEDefined_home_angles(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::vector v; const Broc::vector* val = ee->GetVal<Broc::vector>(&v, 0x385575E2);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_inDeath / IsEEDefined_inDeath (key 0x9A8500FD)
Broc::bint* GetEE_inDeath(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x9A8500FD);
}

Broc::bbool* IsEEDefined_inDeath(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x9A8500FD);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_sound_handle / IsEEDefined_sound_handle (key 0x8B17F374)
Broc::bint* GetEE_sound_handle(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x8B17F374);
}

Broc::bbool* IsEEDefined_sound_handle(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x8B17F374);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_capSpeed / IsEEDefined_capSpeed (key 0x95E84605)
Broc::bfloat* GetEE_capSpeed(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x95E84605);
}

Broc::bbool* IsEEDefined_capSpeed(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x95E84605);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_setting_up_hq / IsEEDefined_setting_up_hq (key 0x64A1977A)
Broc::bint* GetEE_setting_up_hq(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x64A1977A);
}

Broc::bbool* IsEEDefined_setting_up_hq(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x64A1977A);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_spawnscore / IsEEDefined_spawnscore (key 0x1D653125)
Broc::bint* GetEE_spawnscore(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x1D653125);
}

Broc::bbool* IsEEDefined_spawnscore(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x1D653125);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_bTeamKilledWithArtillery / IsEEDefined_bTeamKilledWithArtillery (key 0x4D2B5732)
Broc::bbool* GetEE_bTeamKilledWithArtillery(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bbool>(0x4D2B5732);
}

Broc::bbool* IsEEDefined_bTeamKilledWithArtillery(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bbool v; const Broc::bbool* val = ee->GetVal<Broc::bbool>(&v, 0x4D2B5732);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_cappedSinceLastDeath / IsEEDefined_cappedSinceLastDeath (key 0xAAF03B39)
Broc::bint* GetEE_cappedSinceLastDeath(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xAAF03B39);
}

Broc::bbool* IsEEDefined_cappedSinceLastDeath(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xAAF03B39);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_isInsideFollowClient / IsEEDefined_isInsideFollowClient (key 0x2D6AF62A)
Broc::bint* GetEE_isInsideFollowClient(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x2D6AF62A);
}

Broc::bbool* IsEEDefined_isInsideFollowClient(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x2D6AF62A);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_returnedSinceLastDeath / IsEEDefined_returnedSinceLastDeath (key 0xCBEADD95)
Broc::bint* GetEE_returnedSinceLastDeath(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xCBEADD95);
}

Broc::bbool* IsEEDefined_returnedSinceLastDeath(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xCBEADD95);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_lastMedicCall / IsEEDefined_lastMedicCall (key 0xA2978152)
Broc::bint* GetEE_lastMedicCall(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xA2978152);
}

Broc::bbool* IsEEDefined_lastMedicCall(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xA2978152);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_bigsplashed / IsEEDefined_bigsplashed (key 0x65FADA86)
Broc::bint* GetEE_bigsplashed(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x65FADA86);
}

Broc::bbool* IsEEDefined_bigsplashed(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x65FADA86);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_audio_ambmin / IsEEDefined_audio_ambmin (key 0xC6588AA5)
Broc::bfloat* GetEE_audio_ambmin(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0xC6588AA5);
}

// GetEE_script_sound / IsEEDefined_script_sound (key 0x4816A2BD)
Broc::string* GetEE_script_sound(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::string>(0x4816A2BD);
}

Broc::bbool* IsEEDefined_script_sound(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v; const Broc::string* val = ee->GetVal<Broc::string>(&v, 0x4816A2BD);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

Broc::bbool* IsEEDefined_audio_ambmin(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0xC6588AA5);
            bool IsDefined = Broc::IsDefined(*val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

} // namespace mp_util_wad

// ============================================================================
// _mp_airplanes - airplane flyby script (72-byte mp_plane).
// ============================================================================
namespace _mp_airplanes {
struct mp_plane {
    Broc::string plane_model;       // +0x00
    Broc::string plane_sound;       // +0x04
    Broc::bint   plane_speed;       // +0x08
    Broc::bfloat plane_min_delay;   // +0x0C
    Broc::bfloat plane_max_delay;   // +0x10
    Broc::bfloat plane_sound_delay; // +0x14
    Broc::dyn_array<Broc::vector> plane_start_orgs;  // +0x18
    Broc::dyn_array<Broc::vector> plane_end_orgs;    // +0x24
    Broc::dyn_array<float>        plane_dists;       // +0x30
    Broc::dyn_array<Broc::vector> plane_angles;      // +0x3C
};

extern void* plane_flyby__functor(Broc::entity self, mp_plane plane_struct, Broc::bint num);
extern void* plane_roll__functor(Broc::entity self);
extern void* plane_flyby_thread__functor(Broc::entity self, mp_plane plane_struct);

void plane_flyby_setup(Broc::string t_name, Broc::string model, Broc::string sound,
                       Broc::bint speed, Broc::bfloat min_delay, Broc::bfloat max_delay,
                       Broc::bfloat sound_delay);
void plane_flyby_thread(Broc::entity self, mp_plane plane_struct);
void plane_flyby(Broc::entity self, mp_plane plane_struct, Broc::bint num);
void plane_roll(Broc::entity self);
}

// ============================================================================
// Broc free helpers + gBrocAPI-backed wrappers (mp_util_wad.o COMDATs).
// ============================================================================
namespace Broc {

// IsDefined overloads - ea: 0x92F130 / 0x92F150 / 0x92F6F0
bool IsDefined(const Broc::entity& e) {
    return e.___u0 != 0;
}

bool IsDefined(const Broc::vector& v) {
    return v.x != 0.0f || v.y != 0.0f || v.z != 0.0f;
}

bool IsDefined(const Broc::string& s) {
    return s.c_str() != NULL;
}

template <typename T> bool IsDefined(const T& t) {
    return t.mVal != 0;
}

// Distance - ea: 0x934A50
float Distance(const Broc::vector* v0, const Broc::vector* v1) {
    return gBrocAPI.mVecDistance(v0, v1);
}

// VectorToAngles - ea: 0x934A80
Broc::vector* VectorToAngles(Broc::vector* result, const Broc::vector* vecIn) {
    Broc::vector vecOut;
    gBrocAPI.mVecToAngles(&vecOut, vecIn);
    *result = vecOut;
    return result;
}

// GetEnt - ea: 0x9349D0
Broc::entity* GetEnt(Broc::entity* result, const Broc::string* val, HashStr key,
                     unsigned int flags) {
    unsigned int v4 = gBrocAPI.mGetEnt(val, key.mVal, NULL, 0, flags);
    result->___u0 = v4;
    return result;
}

// Delete - ea: 0x934A20
void Delete(Broc::entity* e) {
    gBrocAPI.mDelete(e->GetHandle());
}

// operator+(string, float) - ea: 0x934830
Broc::string operator+(const Broc::string& lhs, float rhs) {
    Broc::string r(lhs);
    r += rhs;
    return r;
}

} // namespace Broc

// ============================================================================
// Boxed-type operators (mp_util_wad.o inline COMDATs)
// ============================================================================
bool operator<(Broc::bint lhs, Broc::bint rhs) {
    return lhs.mVal < rhs.mVal;
}

bool operator>(Broc::bint lhs, Broc::bint rhs) {
    return lhs.mVal > rhs.mVal;
}

bool operator==(Broc::bint lhs, Broc::bint rhs) {
    return lhs.mVal == rhs.mVal;
}

bool operator!=(Broc::bint lhs, Broc::bint rhs) {
    return lhs.mVal != rhs.mVal;
}

// ============================================================================
// _mp_airplanes::plane_flyby_thread - ea: 0x934E80
// ============================================================================
namespace _mp_airplanes {
void plane_flyby_thread(Broc::entity self, mp_plane plane_struct) {
    (void)self;
    Broc::bint count;
    while (1) {
        float fMax = (float)plane_struct.plane_max_delay;
        float fMin = (float)plane_struct.plane_min_delay;
        fMax = RandomFloatRange(fMin, fMax);
        Broc::wait(fMax);
        int v2 = Broc::size(plane_struct.plane_start_orgs);
        int v3 = RandomInt(v2);
        count = v3 + 1;
        for (Broc::bint i = 0; i.mVal < count.mVal; ++i) {
            void* ftor = plane_flyby__functor(Broc::entity(), plane_struct, i);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_airplanes.bro",
                                __LINE__, "plane_flyby", ftor);
        }
    }
}

// ============================================================================
// _mp_airplanes::plane_flyby - ea: 0x9351C0
// ============================================================================
void plane_flyby(Broc::entity self, mp_plane plane_struct, Broc::bint num) {
    Broc::string inClassname("script_model");
    Broc::entity plane;
    Broc::vector* startOrg = &plane_struct.plane_start_orgs[(unsigned int)num];
    Broc::Spawn(&plane, &inClassname, startOrg, 0);
    inClassname.~string();
    Broc::SetModel(&plane, &plane_struct.plane_model, 0);
    Broc::vector* angles = &plane_struct.plane_angles[(unsigned int)num];
    // plane.angles = *angles (entity angles assignment)
    float* dist = &plane_struct.plane_dists[(unsigned int)num];
    float time = *dist / (float)plane_struct.plane_speed;
    float randTime = RandomFloatRange(time, time * 1.05f);
    void* rollFtor = plane_roll__functor(plane);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_airplanes.bro",
                        __LINE__, "plane_roll", rollFtor);
    Broc::vector* endOrg = &plane_struct.plane_end_orgs[(unsigned int)num];
    Broc::MoveTo(&plane, endOrg, randTime, 0.0f, 0.0f);
    float soundDelay = (float)plane_struct.plane_sound_delay;
    Broc::wait_accurate(soundDelay);
    Broc::EffectEventPlay(&plane, &plane_struct.plane_sound);
    Broc::wait_accurate(randTime - soundDelay);
    Broc::Delete(&plane);
    plane_struct.plane_model.~string();
    plane_struct.plane_sound.~string();
}

// ============================================================================
// _mp_airplanes::plane_roll - ea: 0x935680
// ============================================================================
void plane_roll(Broc::entity self) {
    (void)self;
}
}

// ============================================================================
// _mp_audio - small audio helpers (AudioPrint, CallbackSetLevelAudio).
// ============================================================================
namespace _mp_audio {
extern int GetCvarInt(const char* cvar);
extern void iprintlnbold(const Broc::string& s);
extern void ReverbSetParams(const Broc::string& name, bool immediate);
extern unsigned int SoundPlay(const Broc::string& name, float volume);

// Level struct forward (full 560-byte definition deferred; only touched fields).
struct LevelAudioFields {
    Broc::string background_track;
    Broc::string reverb_setting;
    Broc::string ambient_setting;
    Broc::bfloat audio_ambient_max;
    Broc::bfloat audio_ambient_min;
};
extern LevelAudioFields* pLevelAudio;

void AudioPrint(Broc::string s) {
    if (GetCvarInt("sound_debug") != 0)
        Broc::iprintlnbold(s);
    s.~string();
}

void CallbackSetLevelAudio(const char* background_track, const char* reverb,
                           const char* ambient, int ambient_min, int ambient_max) {
    if (background_track != NULL && *background_track != 0)
        pLevelAudio->background_track = background_track;
    if (reverb != NULL && *reverb != 0)
        pLevelAudio->reverb_setting = reverb;
    if (ambient != NULL && *ambient != 0)
        pLevelAudio->ambient_setting = ambient;
    pLevelAudio->audio_ambient_max = (float)ambient_max;
    pLevelAudio->audio_ambient_min = (float)ambient_min;
}

// audio_crossfade_wait - ea: 0x937020
void audio_crossfade_wait(Broc::entity self) {
    (void)self;
    mp_util_wad::pLevel->crossfade_done = 0;
    Broc::wait(2.1f);
    mp_util_wad::pLevel->crossfade_done = 1;
    mp_util_wad::pLevel->audio_current_priority =
        mp_util_wad::pLevel->audio_change_priority.mVal + 1;
}

// track_changer - ea: 0x9370E0
void track_changer(Broc::string new_track) {
    mp_util_wad::pLevel->audio_current_priority =
        mp_util_wad::pLevel->audio_change_priority.mVal + 1;
    mp_util_wad::pLevel->background_track = new_track;
    new_track.~string();
}

// reverb_changer - ea: 0x937190
void reverb_changer(Broc::string new_reverb) {
    mp_util_wad::pLevel->audio_current_priority =
        mp_util_wad::pLevel->audio_change_priority.mVal + 1;
    mp_util_wad::pLevel->reverb_setting = new_reverb;
    new_reverb.~string();
}

// ambient_changer - ea: 0x937240
void ambient_changer(Broc::string new_ambient_setting) {
    mp_util_wad::pLevel->audio_current_priority =
        mp_util_wad::pLevel->audio_change_priority.mVal + 1;
    mp_util_wad::pLevel->ambient_setting = new_ambient_setting;
    new_ambient_setting.~string();
}

// PlaySound - ea: 0x938A90
void PlaySound(Broc::entity self, Broc::string sound, Broc::bfloat delay) {
    (void)self;
    Broc::wait((float)delay);
    Broc::SoundPlay(sound, 1.0f);
    sound.~string();
}

// audio_spawner - ea: 0x937760
void audio_spawner(Broc::entity self, Broc::string sound) {
    (void)self;
    if (Broc::IsDefined(sound)) {
        Broc::dyn_array<Broc::entity> players;
        Broc::GetLocalPlayerArray(&players);
        Broc::bint min_range(300);
        Broc::bint range(300);
        Broc::bint height(200);
        int v2 = RandomInt(360);
        Broc::vector angle(0.0f, (float)v2, 0.0f);
        Broc::vector dir;
        Broc::AnglesToForward(&dir, &angle);
        int v4 = RandomInt((int)range);
        Broc::vector v20;
        Broc::vector pos;
        Broc::vector v21;
        Broc::vector v23;
        Broc::vector v22;
        Broc::vector v24;
        Broc::vector* origin = Broc::entity_origin(&players[0], &v21);
        Broc::vector* scaled = Broc::vector_scale(&v20, &dir, (float)(v4 + min_range.mVal));
        Broc::vector* sum = Broc::vector_add(&pos, origin, scaled);
        (void)sum;
        float z = Broc::vector_get(&v23, 2) + (float)RandomInt((int)height);
        Broc::vector facing(0.0f, 0.0f, 0.0f);
        Broc::EffectEventPlay(&sound, &pos, &facing);
        players.~dyn_array();
        sound.~string();
    } else {
        sound.~string();
    }
}

// ambient_system - ea: 0x9372E0
void ambient_system(Broc::entity lvl, Broc::string spawn_package) {
    (void)lvl;
    extern void* audio_spawner__functor(Broc::entity, Broc::string);
    bool bad_min = IS_NAN((float)mp_util_wad::pLevel->audio_current_ambient_min) ||
                   !Broc::IsDefined(mp_util_wad::pLevel->audio_ambient_min) ||
                   (float)mp_util_wad::pLevel->audio_current_ambient_min < 0.09f ||
                   (float)mp_util_wad::pLevel->audio_current_ambient_min > 100000.0f;
    if (bad_min) {
        if (Broc::gBrocAPI.mWarning("c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                              "_mp_audio.bro Missing amb_min value, or the value is out of range, Setting Value to 1."))
            __debugbreak();
        mp_util_wad::pLevel->audio_current_ambient_min = 1.0f;
    } else {
        bool bad_max = IS_NAN((float)mp_util_wad::pLevel->audio_current_ambient_wait) ||
                       !Broc::IsDefined(mp_util_wad::pLevel->audio_current_ambient_wait) ||
                       (float)mp_util_wad::pLevel->audio_current_ambient_wait < 0.3f ||
                       (float)mp_util_wad::pLevel->audio_current_ambient_wait > 100000.0f;
        if (!bad_max) {
            while (1) {
                Broc::entity fMin;
                Broc::string fMax = mp_util_wad::pLevel->audio_current_ambpack;
                void* ftor = audio_spawner__functor(fMin, fMax);
                Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_audio.bro",
                                    __LINE__, "audio_spawner", ftor);
                float fMinV = (float)mp_util_wad::pLevel->audio_current_ambient_min;
                float fMaxV = (float)mp_util_wad::pLevel->audio_current_ambient_wait;
                Broc::wait(RandomFloatRange(fMinV, fMaxV));
            }
        }
        if (Broc::gBrocAPI.mWarning("c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                              "_mp_audio.bro Missing amb_max value, or the value is out of range. Setting Value to 5."))
            __debugbreak();
        mp_util_wad::pLevel->audio_current_ambient_wait = 5.0f;
    }
    spawn_package.~string();
}

// ambient_chatter_system - ea: 0x936390
void ambient_chatter_system(Broc::entity self) {
    (void)self;
    for (;;) {
        extern void* audio_spawner__functor(Broc::entity, Broc::string);
        Broc::entity lvl;
        lvl.___u0 = mp_util_wad::pLevel != NULL;
        Broc::string sound("dist_chatter");
        void* ftor = audio_spawner__functor(lvl, sound);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_audio.bro",
                            __LINE__, "_mp_audio::audio_spawner", ftor);
        Broc::wait(Broc::RandomFloatRange(1.0f, 5.0f));
    }
}

// interior_triggering_device - ea: 0x936570
void interior_triggering_device(Broc::entity trigger, Broc::entity other) {
    if (Broc::Code_IsLocalPlayer(other)) {
        if ((int)*mp_util_wad::GetEE_audio_track_p(trigger) >
            (int)mp_util_wad::pLevel->audio_change_priority) {
            mp_util_wad::pLevel->audio_change_priority =
                (int)*mp_util_wad::GetEE_audio_track_p(trigger);
            mp_util_wad::pLevel->audio_change_track =
                *mp_util_wad::GetEE_audio_track(trigger);
            mp_util_wad::pLevel->audio_change_reverb =
                *mp_util_wad::GetEE_reverb(trigger);
            mp_util_wad::pLevel->audio_change_ambpack =
                *mp_util_wad::GetEE_audio_ambp(trigger);
            mp_util_wad::pLevel->audio_change_ambient_wait =
                (float)*mp_util_wad::GetEE_audio_ambmax(trigger);
            mp_util_wad::pLevel->audio_change_ambient_min =
                (float)*mp_util_wad::GetEE_audio_ambmin(trigger);
            mp_util_wad::pLevel->audio_indoor_switch =
                (int)*mp_util_wad::GetEE_audio_indoor(trigger);
        }
    }
}

// PlayerLocation - ea: 0x936770
void PlayerLocation(Broc::entity self) {
    (void)self;
    for (;;) {
        mp_util_wad::pLevel->audio_change_priority = 0;
        Broc::wait(0.09f);
        if ((int)mp_util_wad::pLevel->audio_change_priority == 0) {
            mp_util_wad::pLevel->audio_change_track =
                mp_util_wad::pLevel->background_track;
            mp_util_wad::pLevel->audio_change_reverb =
                mp_util_wad::pLevel->reverb_setting;
            mp_util_wad::pLevel->audio_change_ambpack =
                mp_util_wad::pLevel->ambient_setting;
            mp_util_wad::pLevel->audio_change_ambient_wait =
                (float)mp_util_wad::pLevel->audio_ambient_max;
            mp_util_wad::pLevel->audio_change_ambient_min =
                (float)mp_util_wad::pLevel->audio_ambient_min;
            mp_util_wad::pLevel->audio_indoor_switch = 0;
        }
        if ((int)mp_util_wad::pLevel->audio_current_priority !=
            (int)mp_util_wad::pLevel->audio_change_priority) {
            if (mp_util_wad::pLevel->audio_current_track !=
                    mp_util_wad::pLevel->audio_change_track &&
                mp_util_wad::pLevel->audio_change_track != "none") {
                if (Broc::IsDefined(mp_util_wad::pLevel->audio_change_track)) {
                    if ((int)mp_util_wad::pLevel->crossfade_done == 1) {
                        Broc::bint snd1((int)Broc::SoundPlay(
                            mp_util_wad::pLevel->audio_change_track, 1.0f));
                        Broc::SoundCrossFade(
                            (unsigned int)(int)mp_util_wad::pLevel
                                ->audio_current_track_handle,
                            (unsigned int)(int)snd1, 2.0f);
                        Broc::entity lvl;
                        lvl.___u0 = mp_util_wad::pLevel != NULL;
                        void* ftor = audio_crossfade_wait__functor(lvl);
                        Broc::thread_create(false,
                                            "c:\\cod\\code\\script\\_mp_audio.bro",
                                            __LINE__, "audio_crossfade_wait",
                                            ftor);
                        Broc::wait(0.1f);
                        mp_util_wad::pLevel->audio_current_track_handle =
                            (int)snd1;
                        Broc::string s =
                            mp_util_wad::pLevel->audio_change_track;
                        AudioPrint(s);
                        mp_util_wad::pLevel->audio_current_track =
                            mp_util_wad::pLevel->audio_change_track;
                    }
                } else {
                    if (Broc::gBrocAPI.mWarning(
                            "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                            "_mp_audio.bro Missing audio_track in trigger. setting to none."))
                        __debugbreak();
                    mp_util_wad::pLevel->audio_change_track = "none";
                }
            }
            if (mp_util_wad::pLevel->audio_current_reverb !=
                    mp_util_wad::pLevel->audio_change_reverb &&
                mp_util_wad::pLevel->audio_change_reverb != "none") {
                if (Broc::IsDefined(mp_util_wad::pLevel->audio_change_reverb)) {
                    Broc::ReverbSetParams(mp_util_wad::pLevel->audio_change_reverb,
                                          false);
                    Broc::wait(0.1f);
                    mp_util_wad::pLevel->audio_current_reverb =
                        mp_util_wad::pLevel->audio_change_reverb;
                    Broc::string s =
                        mp_util_wad::pLevel->audio_change_reverb;
                    AudioPrint(s);
                } else {
                    Broc::string name("Preset_Noreverb");
                    Broc::ReverbSetParams(name, false);
                    name.~string();
                    if (Broc::gBrocAPI.mWarning(
                            "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                            "_mp_audio.bro Missing reverb in trigger. setting to Preset_Noreverb."))
                        __debugbreak();
                    mp_util_wad::pLevel->audio_change_reverb =
                        "Preset_Noreverb";
                }
            }
            if (mp_util_wad::pLevel->audio_current_ambpack !=
                    mp_util_wad::pLevel->audio_change_ambpack &&
                mp_util_wad::pLevel->audio_change_ambpack != "none") {
                if (!Broc::IsDefined(mp_util_wad::pLevel->audio_change_ambpack) ||
                    IS_NAN((float)mp_util_wad::pLevel->audio_change_ambient_min) ||
                    IS_NAN((float)mp_util_wad::pLevel->audio_change_ambient_wait)) {
                    if (Broc::gBrocAPI.mWarning(
                            "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                            "_mp_audio.bro Missing audio_ambp, amb_min, amb_max in the trigger. setting to defaults."))
                        __debugbreak();
                    mp_util_wad::pLevel->audio_change_ambpack = "NoAmbFX";
                    mp_util_wad::pLevel->audio_change_ambient_min = 1.0f;
                    mp_util_wad::pLevel->audio_change_ambient_wait = 5.0f;
                } else {
                    mp_util_wad::pLevel->audio_current_ambient_wait =
                        (float)mp_util_wad::pLevel->audio_change_ambient_wait;
                    mp_util_wad::pLevel->audio_current_ambient_min =
                        (float)mp_util_wad::pLevel->audio_change_ambient_min;
                    mp_util_wad::pLevel->audio_indoor_switch = 1;
                    mp_util_wad::pLevel->audio_current_ambpack =
                        mp_util_wad::pLevel->audio_change_ambpack;
                    Broc::wait(0.1f);
                    Broc::string s =
                        mp_util_wad::pLevel->audio_current_ambpack;
                    AudioPrint(s);
                    Broc::string s2 =
                        Broc::string(mp_util_wad::pLevel->audio_current_ambient_min);
                    AudioPrint(s2);
                }
            }
            mp_util_wad::pLevel->audio_current_priority =
                (int)mp_util_wad::pLevel->audio_change_priority;
        }
    }
}

// closest_point_on_line_to_point - ea: 0x937B20
Broc::bfloat closest_point_on_line_to_point(
    Broc::vector Point, Broc::vector LineStart, Broc::vector LineEnd,
    Broc::vector& out_PointOnLine) {
    Broc::vector line = LineEnd - LineStart;
    float LineMagSqrd = line.x * line.x + line.y * line.y + line.z * line.z;
    float t = ((Point.x - LineStart.x) * (LineEnd.x - LineStart.x) +
               (Point.y - LineStart.y) * (LineEnd.y - LineStart.y) +
               (Point.z - LineStart.z) * (LineEnd.z - LineStart.z)) /
              LineMagSqrd;
    if (t < 0.0f) {
        out_PointOnLine = LineStart;
    } else if (t > 1.0f) {
        out_PointOnLine = LineEnd;
    } else {
        out_PointOnLine = Broc::vector(
            LineStart.x + t * (LineEnd.x - LineStart.x),
            LineStart.y + t * (LineEnd.y - LineStart.y),
            LineStart.z + t * (LineEnd.z - LineStart.z));
    }
    return Broc::bfloat(t);
}

// StopLineSound - ea: 0x937EB0
void StopLineSound(Broc::string startOfLineEntity) {
    if (Broc::IsDefined(startOfLineEntity)) {
        HashStr key;
        Broc::string_hash(&key, &startOfLineEntity);
        Broc::entity soundMover;
        mp_util_wad::line_sound_get(&soundMover, key);
        if (Broc::IsDefined(soundMover)) {
            Broc::Delete(&soundMover);
            HashStr key2;
            Broc::string_hash(&key2, &startOfLineEntity);
            mp_util_wad::line_sound_erase(key2);
        } else {
            if (Broc::gBrocAPI.mWarning(
                    "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                    "_mp_audio::StopLineSound(): could not find line entity to stop! Aborting..."))
                __debugbreak();
        }
    } else {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio::StopLineSound(): startOfLineEntity is UNDEFINED! Aborting..."))
            __debugbreak();
    }
    startOfLineEntity.~string();
}

// StopAllLineSounds - ea: 0x938030
void StopAllLineSounds() {
    mp_util_wad::line_sound_delete_all();
}

Broc::entity* SpawnLineSound(Broc::entity* result, Broc::entity startOfLine,
                             Broc::string sound);
void StopTeamSound();

// SpawnLineSound (string start) - ea: 0x9380F0
Broc::entity* SpawnLineSound(Broc::entity* result, Broc::string startOfLineEntity,
                             Broc::string sound) {
    if (Broc::IsDefined(startOfLineEntity)) {
        HashStr key;
        key.mVal = 0x19F9F0E8u;
        Broc::entity startOfLine;
        Broc::GetEnt(&startOfLine, &startOfLineEntity, key, 0);
        if (Broc::IsDefined(startOfLine)) {
            Broc::entity soundMover;
            soundMover.___u0 = 0;
            SpawnLineSound(&soundMover, startOfLine, sound);
            HashStr mapKey;
            Broc::string_hash(&mapKey, &startOfLineEntity);
            mp_util_wad::line_sound_set(mapKey, soundMover);
            *result = soundMover;
        } else {
            if (Broc::gBrocAPI.mWarning(
                    "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                    "_mp_audio::SpawnLineSound(): Could not find start of line entity! Aborting..."))
                __debugbreak();
            *result = Broc::gEntityUndef;
        }
    } else {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio::SpawnLineSound(): startOfLineEntity is UNDEFINED! Aborting..."))
            __debugbreak();
        *result = Broc::gEntityUndef;
    }
    startOfLineEntity.~string();
    sound.~string();
    return result;
}

// SpawnLineSound (entity start) - ea: 0x938320
Broc::entity* SpawnLineSound(Broc::entity* result, Broc::entity startOfLine,
                             Broc::string sound) {
    Broc::string target;
    mp_util_wad::entity_get_target(&target, startOfLine);
    if (!Broc::IsDefined(target)) {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio::SpawnLineSound(): startOfLineEntity.target is UNDEFINED! Aborting..."))
            __debugbreak();
        target.~string();
        sound.~string();
        *result = Broc::gEntityUndef;
        return result;
    }
    if (!Broc::IsDefined(sound)) {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio::SpawnLineSound(): sound is UNDEFINED! Aborting..."))
            __debugbreak();
        target.~string();
        sound.~string();
        *result = Broc::gEntityUndef;
        return result;
    }
    Broc::string target2;
    mp_util_wad::entity_get_target(&target2, startOfLine);
    Broc::entity endOfLineEntity;
    HashStr key;
    key.mVal = 0x19F9F0E8u;
    Broc::GetEnt(&endOfLineEntity, &target2, key, 0);
    target2.~string();
    target.~string();
    if (Broc::IsAlive(&endOfLineEntity) != 0) {
        Broc::vector start;
        Broc::vector end;
        mp_util_wad::entity_get_origin(&start, startOfLine);
        mp_util_wad::entity_get_origin(&end, endOfLineEntity);
        Broc::string inClassname("script_origin");
        Broc::entity soundMover;
        Broc::Spawn(&soundMover, &inClassname, &start, INVALID_PAK_INFO);
        inClassname.~string();
        Broc::EffectEventPlay(&soundMover, &sound);
        void* ftor = MoveSoundAlongLine__functor(soundMover, start, end);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_audio.bro",
                            __LINE__, "MoveSoundAlongLine", ftor);
        *result = soundMover;
    }
    sound.~string();
    return result;
}

// MoveSoundAlongLine - ea: 0x938720
void MoveSoundAlongLine(Broc::entity toMove, Broc::vector start,
                        Broc::vector end) {
    Broc::dyn_array<Broc::entity> players;
    Broc::GetLocalPlayerArray(&players);
    Broc::entity player;
    player.___u0 = 0;
    if (Broc::size(players) > 0)
        player = players[0];
    Broc::vector pos;
    Broc::bfloat closest_dist;
    for (;;) {
        if (!Broc::IsDefined(player)) {
            Broc::dyn_array<Broc::entity> entarr;
            Broc::GetPlayerArray(&entarr);
            if (Broc::size(entarr) > 0)
                player = entarr[0];
            entarr.~dyn_array();
        }
        Broc::vector porg;
        mp_util_wad::entity_get_origin(&porg, player);
        closest_point_on_line_to_point(porg, start, end, pos);
        mp_util_wad::entity_set_origin(toMove, pos);
        if (Broc::IsDefined(pos)) {
            float dist = Broc::DistanceSquared(&porg, &pos);
            closest_dist = dist;
            if ((float)closest_dist > 65536.0f) {
                Broc::wait(2.0f);
            } else if ((float)closest_dist > 262144.0f) {
                Broc::wait(0.2f);
            } else {
                Broc::wait(0.01f);
            }
        }
    }
}

// PlayLocalDialog - ea: 0x938B20
void PlayLocalDialog(Broc::entity self, Broc::string sound, Broc::string team) {
    (void)sound;
    Broc::DialogPlay(self, &team);
    sound.~string();
    team.~string();
}

// PlayTeamDialog (3-string) - ea: 0x938BE0
void PlayTeamDialog(Broc::entity self, Broc::string team, Broc::string sound,
                    Broc::bfloat delay) {
    Broc::dyn_array<Broc::entity> players;
    Broc::GetLocalPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::string pteam;
        mp_util_wad::entity_get_team(&pteam, players[(unsigned int)(int)i]);
        if (pteam == team) {
            pteam.~string();
            players.~dyn_array();
            Broc::wait((float)delay);
            Broc::DialogPlay(self, &sound);
            team.~string();
            sound.~string();
            return;
        }
        pteam.~string();
        i = (int)i + 1;
    }
    players.~dyn_array();
    team.~string();
    sound.~string();
}

// PlayTeamDialog (4-string) - ea: 0x938E90
void PlayTeamDialog(Broc::entity self, Broc::string primaryteam,
                    Broc::string primaryteamsound,
                    Broc::string secondaryteamsound, Broc::bfloat delay) {
    Broc::dyn_array<Broc::entity> players;
    Broc::GetLocalPlayerArray(&players);
    if (Broc::size(players) == 1) {
        Broc::string team;
        mp_util_wad::entity_get_team(&team, players[0]);
        Broc::wait((float)delay);
        if (team == primaryteam)
            Broc::DialogPlay(self, &primaryteamsound);
        else
            Broc::DialogPlay(self, &secondaryteamsound);
        team.~string();
    } else {
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            Broc::string team;
            mp_util_wad::entity_get_team(&team, players[(unsigned int)(int)i]);
            if (team == primaryteam) {
                team.~string();
                Broc::wait((float)delay);
                Broc::DialogPlay(self, &primaryteamsound);
                players.~dyn_array();
                primaryteam.~string();
                primaryteamsound.~string();
                secondaryteamsound.~string();
                return;
            }
            team.~string();
            i = (int)i + 1;
        }
        Broc::wait((float)delay);
        Broc::DialogPlay(self, &secondaryteamsound);
    }
    players.~dyn_array();
    primaryteam.~string();
    primaryteamsound.~string();
    secondaryteamsound.~string();
}

// PlayTeamSoundStoppable (2-string) - ea: 0x9391B0
void PlayTeamSoundStoppable(Broc::entity self, Broc::string sound,
                            Broc::string team) {
    Broc::entity temp;
    temp.___u0 = mp_util_wad::pLevel->hack_sound_entity.___u0;
    Broc::dyn_array<Broc::entity> players;
    Broc::GetLocalPlayerArray(&players);
    if (!Broc::IsDefined(mp_util_wad::pLevel->hack_sound_entity)) {
        Broc::string inClassname("script_origin");
        Broc::entity spawnResult;
        Broc::vector origin;
        if (Broc::size(players) > 0)
            mp_util_wad::entity_get_origin(&origin, players[0]);
        mp_util_wad::pLevel->hack_sound_entity =
            *Broc::Spawn(&spawnResult, &inClassname, &origin, INVALID_PAK_INFO);
        inClassname.~string();
    }
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::string pteam;
        mp_util_wad::entity_get_team(&pteam, players[(unsigned int)(int)i]);
        if (pteam == team) {
            pteam.~string();
            StopTeamSound();
            HashStr stopLabel;
            stopLabel.mVal = 0x5DA1DA2Du;
            Broc::endon(temp, stopLabel);
            Broc::vector origin;
            mp_util_wad::entity_get_origin(&origin, players[(unsigned int)(int)i]);
            mp_util_wad::entity_set_origin(mp_util_wad::pLevel->hack_sound_entity,
                                           origin);
            HashStr soundHash;
            Broc::string_hash(&soundHash, &sound);
            int handle = Broc::EffectEventPlay(&temp, &sound, soundHash, true);
            *mp_util_wad::GetEE_teamSound(temp) = handle;
            HashStr waitHash;
            Broc::string_hash(&waitHash, &sound);
            Broc::waittill(temp, waitHash);
            *mp_util_wad::GetEE_teamSound(temp) = 0;
            players.~dyn_array();
            sound.~string();
            team.~string();
            return;
        }
        pteam.~string();
        i = (int)i + 1;
    }
    players.~dyn_array();
    sound.~string();
    team.~string();
    (void)self;
}

// PlayTeamSoundStoppable (3-string) - ea: 0x939590
void PlayTeamSoundStoppable(Broc::entity self, Broc::string primaryteam,
                            Broc::string primaryteamsound,
                            Broc::string secondaryteamsound) {
    (void)self;
    Broc::entity temp;
    temp.___u0 = mp_util_wad::pLevel->hack_sound_entity.___u0;
    Broc::string sound;
    sound = "";
    Broc::dyn_array<Broc::entity> players;
    Broc::GetLocalPlayerArray(&players);
    Broc::vector origin;
    if (Broc::size(players) > 0)
        mp_util_wad::entity_get_origin(&origin, players[0]);
    if (!Broc::IsDefined(mp_util_wad::pLevel->hack_sound_entity)) {
        Broc::string inClassname("script_origin");
        Broc::entity spawnResult;
        mp_util_wad::pLevel->hack_sound_entity =
            *Broc::Spawn(&spawnResult, &inClassname, &origin, INVALID_PAK_INFO);
        inClassname.~string();
    }
    if (Broc::size(players) == 1) {
        Broc::string team;
        mp_util_wad::entity_get_team(&team, players[0]);
        if (team == primaryteam)
            sound = primaryteamsound;
        else
            sound = secondaryteamsound;
        team.~string();
    } else {
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            Broc::string team;
            mp_util_wad::entity_get_team(&team, players[(unsigned int)(int)i]);
            if (team == primaryteam) {
                team.~string();
                mp_util_wad::entity_get_origin(&origin,
                                               players[(unsigned int)(int)i]);
                sound = primaryteamsound;
                break;
            }
            team.~string();
            i = (int)i + 1;
        }
        if (sound == "")
            sound = secondaryteamsound;
    }
    mp_util_wad::entity_set_origin(mp_util_wad::pLevel->hack_sound_entity,
                                   origin);
    StopTeamSound();
    HashStr stopLabel;
    stopLabel.mVal = 0x5DA1DA2Du;
    Broc::endon(temp, stopLabel);
    HashStr soundHash;
    Broc::string_hash(&soundHash, &sound);
    int handle = Broc::EffectEventPlay(&temp, &sound, soundHash, true);
    *mp_util_wad::GetEE_teamSound(temp) = handle;
    HashStr waitHash;
    Broc::string_hash(&waitHash, &sound);
    Broc::waittill(temp, waitHash);
    *mp_util_wad::GetEE_teamSound(temp) = 0;
    players.~dyn_array();
    sound.~string();
    primaryteam.~string();
    primaryteamsound.~string();
    secondaryteamsound.~string();
}

// StopTeamSound - ea: 0x939A50
void StopTeamSound() {
    if (Broc::IsDefined(mp_util_wad::pLevel->hack_sound_entity)) {
        Broc::bbool defined;
        mp_util_wad::IsEEDefined_teamSound(&defined,
                                           mp_util_wad::pLevel->hack_sound_entity);
        if ((bool)defined) {
            HashStr label;
            label.mVal = 0x5DA1DA2Du;
            Broc::notify(&mp_util_wad::pLevel->hack_sound_entity, label);
            int handle =
                (int)*mp_util_wad::GetEE_teamSound(
                    mp_util_wad::pLevel->hack_sound_entity);
            Broc::EffectEventStopEmitting(handle);
            *mp_util_wad::GetEE_teamSound(mp_util_wad::pLevel->hack_sound_entity) =
                0;
            Broc::wait(0.05f);
        }
    }
}

// PlayTeamSound - ea: 0x939BE0
void PlayTeamSound(Broc::entity self, Broc::string team, Broc::string teamsound,
                   Broc::string otherteamsound) {
    (void)self;
    Broc::dyn_array<Broc::entity> players;
    Broc::GetLocalPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::string pteam;
        mp_util_wad::entity_get_team(&pteam, players[(unsigned int)(int)i]);
        if (pteam == team) {
            pteam.~string();
            Broc::SoundPlay(teamsound, 1.0f);
            players.~dyn_array();
            team.~string();
            teamsound.~string();
            otherteamsound.~string();
            return;
        }
        pteam.~string();
        i = (int)i + 1;
    }
    Broc::SoundPlay(otherteamsound, 1.0f);
    players.~dyn_array();
    team.~string();
    teamsound.~string();
    otherteamsound.~string();
}

// PlaySoundAtLocation - ea: 0x939DC0
void PlaySoundAtLocation(Broc::entity self, Broc::string sound,
                         Broc::vector position) {
    (void)self;
    Broc::wait(2.5f);
    Broc::string inClassname("script_origin");
    Broc::entity temp_entity;
    Broc::Spawn(&temp_entity, &inClassname, &position, INVALID_PAK_INFO);
    inClassname.~string();
    HashStr notifyHash;
    Broc::string_hash(&notifyHash, &sound);
    Broc::EffectEventPlay(&temp_entity, &sound, notifyHash, true);
    HashStr waitHash;
    Broc::string_hash(&waitHash, &sound);
    Broc::waittill(temp_entity, waitHash);
    Broc::Delete(&temp_entity);
    sound.~string();
}

// ThreadLineSound - ea: 0x939EE0
void ThreadLineSound(Broc::entity self) {
    Broc::string startOfLineEntity;
    mp_util_wad::entity_get_targetname(&startOfLineEntity, self);
    if (!Broc::IsDefined(startOfLineEntity)) {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio::ThreadLineSound(): targetname is UNDEFINED! Aborting..."))
            __debugbreak();
    } else {
        Broc::bbool defined;
        mp_util_wad::IsEEDefined_script_sound(&defined, self);
        if ((bool)defined) {
            Broc::string sound = *mp_util_wad::GetEE_script_sound(self);
            Broc::string target;
            mp_util_wad::entity_get_targetname(&target, self);
            Broc::entity mover;
            mover.___u0 = 0;
            SpawnLineSound(&mover, target, sound);
            target.~string();
        } else {
            if (Broc::gBrocAPI.mWarning(
                    "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                    "_mp_audio::ThreadLineSound(): script_sound is UNDEFINED! Aborting..."))
                __debugbreak();
        }
    }
    startOfLineEntity.~string();
}

// ThreadStaticSound - ea: 0x93A130
void ThreadStaticSound(Broc::entity self) {
    Broc::bbool defined;
    mp_util_wad::IsEEDefined_script_sound(&defined, self);
    if ((bool)defined) {
        Broc::string sound = *mp_util_wad::GetEE_script_sound(self);
        void* ftor = ThreadStaticSoundPlay__functor(self, sound);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_audio.bro",
                            __LINE__, "_mp_audio::ThreadStaticSoundPlay",
                            ftor);
    } else if (Broc::gBrocAPI.mWarning(
                   "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                   "_mp_audio::ThreadLineSound(): script_sound is UNDEFINED! Aborting..."))
        __debugbreak();
}

// ThreadStaticSoundPlay - ea: 0x93A330
void ThreadStaticSoundPlay(Broc::entity self, Broc::string NameOfSoundToPlay) {
    (void)self;
    Broc::EffectEventPlay(&self, &NameOfSoundToPlay);
    NameOfSoundToPlay.~string();
}

// sound_repeat - ea: 0x93A3A0
void sound_repeat(Broc::entity self) {
    Broc::bbool defined;
    mp_util_wad::IsEEDefined_script_sound(&defined, self);
    if ((bool)defined) {
        Broc::string sound = *mp_util_wad::GetEE_script_sound(self);
        void* ftor = ThreadStaticSoundRandomPlay__functor(self, sound);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_audio.bro",
                            __LINE__,
                            "_mp_audio::ThreadStaticSoundRandomPlay", ftor);
    } else if (Broc::gBrocAPI.mWarning(
                   "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                   "__mp_audio::ThreadLineSound(): script_sound is UNDEFINED! Aborting..."))
        __debugbreak();
}

// ThreadStaticSoundRandomPlay - ea: 0x93A5A0
void ThreadStaticSoundRandomPlay(Broc::entity self,
                                 Broc::string NameOfSoundToPlay) {
    for (;;) {
        Broc::wait(Broc::RandomFloatRange(0.9f, 1.75f));
        Broc::EffectEventPlay(&self, &NameOfSoundToPlay);
    }
}

// player_dying_sounds - ea: 0x93C240
void player_dying_sounds(Broc::entity player) {
    Broc::string script("PLAYER_DYING");
    Broc::EffectEventPlay(&player, &script);
    script.~string();
}
}

// ============================================================================
// Entity property helpers - thin wrappers over gBrocAPI m_entity_* members.
// ============================================================================
namespace mp_util_wad {
__int16 entity_get_rank(Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_persistent_player_rank(ent.___u0);
}
void entity_set_rank(Broc::entity ent, __int16 rank) {
    Broc::gBrocAPI.m_entity_set_persistent_player_rank(ent.___u0, rank);
}
Broc::string* entity_get_team(Broc::string* result, Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_sentient_team(result, ent.___u0);
}
void entity_set_team(Broc::entity ent, const Broc::string& team) {
    Broc::gBrocAPI.m_entity_set_sentient_team(ent.___u0, team);
}
Broc::bint* entity_get_playerState(Broc::bint* result, Broc::entity ent) {
    result->mVal = Broc::gBrocAPI.m_entity_get_persistent_player_playerState(ent.___u0);
    return result;
}
void entity_set_playerState(Broc::entity ent, int state) {
    Broc::gBrocAPI.m_entity_set_persistent_player_playerState(ent.___u0, state);
}
__int16 entity_get_nextPlayerClass(Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_persistent_player_nextPlayerClass(ent.___u0);
}
void entity_set_nextPlayerClass(Broc::entity ent, __int16 cls) {
    Broc::gBrocAPI.m_entity_set_persistent_player_nextPlayerClass(ent.___u0, cls);
}
Broc::bint* entity_get_maxhealth(Broc::bint* result, Broc::entity ent) {
    result->mVal = Broc::gBrocAPI.m_entity_get_maxhealth(ent.___u0);
    return result;
}
Broc::bint* entity_get_health(Broc::bint* result, Broc::entity ent) {
    result->mVal = Broc::gBrocAPI.m_entity_get_health(ent.___u0);
    return result;
}
void entity_set_health(Broc::entity ent, int health) {
    Broc::gBrocAPI.m_entity_set_health(ent.___u0, health);
}
__int16 entity_get_playerClass(Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_persistent_player_playerClass(ent.___u0);
}
void entity_set_playerClass(Broc::entity ent, __int16 cls) {
    Broc::gBrocAPI.m_entity_set_persistent_player_playerClass(ent.___u0, cls);
}
Broc::vector* entity_get_angles(Broc::vector* result, Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_angles(result, ent.___u0);
}
void entity_set_angles(Broc::entity ent, const Broc::vector& v) {
    Broc::gBrocAPI.m_entity_set_angles(ent.___u0, v);
}
Broc::vector* entity_get_viewangles(Broc::vector* result, Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_player_viewangles(result, ent.___u0);
}
Broc::string* entity_get_target(Broc::string* result, Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_target(result, ent.___u0);
}
void entity_set_spectatorClient(Broc::entity ent, int client) {
    Broc::gBrocAPI.m_entity_set_player_spectatorClient(ent.___u0, client);
}
Broc::vector* entity_get_origin(Broc::vector* result, Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_origin(result, ent.___u0);
}
void entity_set_origin(Broc::entity ent, const Broc::vector& v) {
    Broc::gBrocAPI.m_entity_set_origin(ent.___u0, v);
}
Broc::string* entity_get_targetname(Broc::string* result, Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_targetname(result, ent.___u0);
}

// line_sound_emitters hash_map helpers (opaque until the runtime is ported).
void line_sound_set(HashStr key, Broc::entity e) {
    (void)key;
    (void)e;
}
Broc::entity* line_sound_get(Broc::entity* result, HashStr key) {
    (void)key;
    result->___u0 = 0;
    return result;
}
void line_sound_erase(HashStr key) {
    (void)key;
}
void line_sound_delete_all() {
}
}

// ============================================================================
// _mp_dm - deathmatch script.
// ============================================================================
namespace _mp_dm {
namespace _mp_common {
void SetupCallbacks(Broc::bbool teamGameType);
}
extern void* StartGame__functor(Broc::entity self);
extern void* main__functor(Broc::entity self);
extern Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* ent,
                                   const Broc::string* spawnpoint);

void main(Broc::entity self) {
    Broc::bbool team_game(false);
    Broc::Code_SetTeamGame((bool)team_game);
    mp_util_wad::pLevel->spawnTypeAllies = "spawn_deathmatch";
    mp_util_wad::pLevel->spawnTypeAxis = "spawn_deathmatch";
    mp_util_wad::pLevel->PickSpawnPoint = (void*)GetSpawnPoint;
    Broc::Code_SetShowScore(false);
    _mp_common::SetupCallbacks(team_game);
    void* started = StartGame__functor(self);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_dm.bro",
                        __LINE__, "StartGame", started);
}
}

// ============================================================================
// Sibling gametype namespaces - declarations used by _mp_common::LaunchGametype.
// Definitions (stubs until those scripts are ported) live at the bottom.
// ============================================================================
namespace _mp_tankdrive { void main(); }
namespace _mp_nano { void main(); }
namespace _mp_audio { void main(); }
namespace _mp_minefield { void* main__functor(Broc::entity self); }
namespace _mp_tdm { void* main__functor(Broc::entity self); }
namespace _mp_ctf { void* main__functor(Broc::entity self); }
namespace _mp_scf { void* main__functor(Broc::entity self); }
namespace _mp_war { void* main__functor(Broc::entity self); }
namespace _mp_hq { void* main__functor(Broc::entity self); }

// ============================================================================
// _mp_common - shared multiplayer logic.
// ============================================================================
namespace _mp_common {
extern void* main__functor(Broc::entity self);  // dispatch stub

void DebugRenderSpawnPoints();
Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* ent,
                            const Broc::string* spawnpoint);
void launch_gametype_thread(Broc::string& gametype, const char* func,
                            void* (*functor)(Broc::entity));

// Callbacks and helpers (sorted by ea, see PROGRESS for full roster).
void SetupCallbacks(Broc::bbool teamGameType);           // ea: 0x93D1A0
void CallbackHostOptionsChanged(int forceMapChange);     // ea: 0x93D3B0
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int timeTillNextRound,
                       int roundCount);                  // ea: 0x93D9C0
void CallbackGameScore(int alliesScore, int axisScore);  // ea: 0x93DFA0
void CallbackPlayerJoin(Broc::entity player, unsigned int playerState,
                        __int16 playerClass);            // ea: 0x93E060
void CallbackPlayerEnter(Broc::entity player, int hot_joiner);  // ea: 0x93E5C0
void CallbackPlayerLeave(Broc::entity leavingPlayer);    // ea: 0x93E970
void CallbackPlayerRespawnRequest(Broc::entity player, int team_allies);  // ea: 0x93EA80
void CallbackPlayerKilled(Broc::entity killedPlayer, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health);                   // ea: 0x93EC10
void CallbackPlayerAssist(Broc::entity assistplayer);    // ea: 0x93F7F0
void CallbackPainFlinch(Broc::entity player, int damage);  // ea: 0x93F860
void CallbackPlayerDamage(Broc::entity victim, Broc::entity inflictor,
                          Broc::entity attacker, const Broc::vector* damageDir,
                          const Broc::vector* point, int damage, int dflags,
                          int mod, int weapon);          // ea: 0x93F970
void CallbackPlayerDamageTeam(Broc::entity victim, Broc::entity inflictor,
                              Broc::entity attacker, const Broc::vector* damageDir,
                              const Broc::vector* point, int damage, int dflags,
                              int mod, int weapon);      // ea: 0x93FA90
void CallbackPlayerSpawn(Broc::entity player, int team_changed);  // ea: 0x93FCF0
void CallbackPlayerRevive(Broc::entity player, Broc::entity medic);  // ea: 0x940400
int CallbackCanTeamChange(Broc::entity player, int team_allies);  // ea: 0x9408B0
void TeamChangeKillPlayer(Broc::entity player);          // ea: 0x940D40
void CallbackPlayerTeamChange(Broc::entity player, int autoBalance, int denied);  // ea: 0x940EC0
void CallbackPlayerClassChange(Broc::entity player, unsigned int newPlayerClass);  // ea: 0x941270
unsigned int CallbackStopFollowing();                    // ea: 0x941350
void CallbackRoundOver(int condition, Broc::string winner);  // ea: 0x941480
void CallbackNextRound();                                // ea: 0x942050
void CallbackRestartMap();                               // ea: 0x9420E0
int CallbackDebugRender();                               // ea: 0x9421D0
void CallbackFireArtillery(Broc::entity ent, Broc::vector position);  // ea: 0x942200
void CallbackFireArtilleryShell(Broc::entity shell);     // ea: 0x943310
void CallbackDenyArtillery(Broc::entity denied);         // ea: 0x943390
void CallbackVehicleKilled(Broc::entity killedVehicle, Broc::entity inflictor,
                           Broc::entity attacker, int weapon, int mod,
                           int occupantCount);           // ea: 0x9435C0
void CallbackVehicleMantled(Broc::entity vehicle, Broc::entity mantler);  // ea: 0x9436E0
void CallbackHealthRegenRecovering(Broc::entity self);   // ea: 0x943720
void CallbackPickupItem(Broc::entity pickerupper, Broc::entity dropper);  // ea: 0x9437F0
void CallbackMineFailed(Broc::entity ent);               // ea: 0x943980
void CallbackReviveFailed();                             // ea: 0x943A00
void CallbackCallForMedic(Broc::entity ent);             // ea: 0x943A20
void CallbackPunishedForTeamKill(Broc::entity ent, int punished);  // ea: 0x943D60
void PunishedForTeamKill(Broc::entity ent, Broc::bbool punished);  // ea: 0x943E70
void CallbackSpawnButtonPressed(Broc::entity ent);       // ea: 0x944440
unsigned int CallbackQuitGame();                         // ea: 0x944580
unsigned int CallbackHostDisconnected();                 // ea: 0x9446A0
unsigned int CallbackLocalPlayerKicked();                // ea: 0x944730
unsigned int CallbackHostMigrated();                     // ea: 0x9447C0
void QuitGameWithMessage(Broc::entity self, HashStr message);  // ea: 0x9448E0
void HostHasMigrated(Broc::entity self);                 // ea: 0x944A00
void NewHost();                                          // ea: 0x944F40
void FadeUpWhenLoaded(Broc::entity self, Broc::entity player);  // ea: 0x944F90
void RespawnPlayer(Broc::entity guy, Broc::string team);  // ea: 0x9450D0
void Spectate(Broc::entity self, Broc::bint target, Broc::bbool isIntermission);  // ea: 0x945350
void DeathState(Broc::entity self, Broc::entity inflictor, Broc::bint weapon,
                Broc::bbool isPlayerKill, Broc::bbool isPlayerDamage);  // ea: 0x945420
void FollowClient(Broc::entity self, Broc::bint index);  // ea: 0x9459F0
void LocalPlayerIntermission(Broc::entity self);         // ea: 0x945BC0
void LocalPlayerRespawn(Broc::entity self);              // ea: 0x945D30
void RunFrame();                                         // ea: 0x946040
void CreateClock(Broc::bint timeLimit);                  // ea: 0x946080
void StartRound(Broc::bbool firstTime);                  // ea: 0x946110
void finish_starting_round(Broc::entity self, Broc::bbool isRoundOver);  // ea: 0x946630
void restart_round(Broc::entity self, Broc::bint timeTillNextRound);  // ea: 0x946C00
void CheckTimeLimit();                                   // ea: 0x9474B0
void CheckScoreLimit();                                  // ea: 0x9476D0
void CheckLastManStanding(Broc::string ignore);          // ea: 0x947AA0
void CheckLastManStandingEnoughPlayers(Broc::entity guy);  // ea: 0x947F40
void waitframe();                                        // ea: 0x9483A0
void DestroyHudElem(Broc::hudelem* elem);                // ea: 0x9483D0
void AddToPlayerStats(Broc::entity player, Broc::bint stat, __int16 amount);  // ea: 0x948480
void put_player_into_spectate_mode(Broc::entity guy);    // ea: 0x948A30
void SpawnLocalSpectator(Broc::entity self);             // ea: 0x948B70
void SpawnSpectator(Broc::entity self);                  // ea: 0x948FC0
void SpawnIntermission(Broc::entity self);               // ea: 0x949200
void QuitGameThread();                                   // ea: 0x9495C0
Broc::bint* GetRespawnTime(Broc::bint* result, Broc::entity player);  // ea: 0x949620
Broc::bint* GetGoingToDieTime(Broc::bint* result);       // ea: 0x949B60
void UpdateSpectateCritical(Broc::entity self);          // ea: 0x949BB0
void UpdateSpectateCriticalGoingToDie(Broc::entity self);  // ea: 0x949F80
void UpdateSpectateDead(Broc::entity self, Broc::bbool isIntermission);  // ea: 0x94A2D0
void UpdateSpectateSpawn(Broc::entity self);             // ea: 0x94A8D0
void StopFollowing(Broc::entity self, Broc::bbool blackNow);  // ea: 0x94AA60
void ProgressBarCreate(Broc::bint width);                // ea: 0x94AA80
void ProgressBarUpdate(Broc::bfloat value, Broc::bfloat time);  // ea: 0x94AD50
void ProgressBarDelete();                                // ea: 0x94AE00
void EndRound(EEndRoundCondition condition, Broc::string winner);  // ea: 0x94AE30
Broc::bbool* HasBothTeams(Broc::bbool* result);          // ea: 0x94AEC0
void WaitForTeams();                                     // ea: 0x94B210
Broc::entity* GetBestSpectateSpawn(Broc::entity* result, Broc::entity* self);  // ea: 0x94B4A0
void AudioOnPlayerDeath(Broc::entity entself, Broc::bint weapon);  // ea: 0x94B650
void HealthRegenPlayerBreathing(Broc::entity ent, Broc::bint healthCap);  // ea: 0x94B6F0
void AddArtilleryObjective(Broc::entity ent, Broc::vector position);  // ea: 0x94B900
void HandleJoinAfterRoundOver(Broc::entity self, Broc::bint timeleft);  // ea: 0x94BAE0
void PutAllPlayersIntoIntermission();                    // ea: 0x94BBD0
Broc::bbool* HasValidWeaponInSlot(Broc::bbool* result, Broc::entity player,
                                  Broc::string slot);   // ea: 0x94BE20
void SelectFirstAvailableWeapon(Broc::entity player);    // ea: 0x94BF80
void reenable_medic_call(Broc::entity self, Broc::bint time);  // ea: 0x94C220
Broc::bint* GetTimeTillNextRound(Broc::bint* result);    // ea: 0x94C380
Broc::string* GetWinningTeam(Broc::string* result);      // ea: 0x94C4A0

// SetupGameVariables - ea: 0x93D030
int SetupGameVariables() {
    mp_util_wad::pLevel->timeLimit = Broc::GetCvarInt("mp_timelimit");
    mp_util_wad::pLevel->scoreLimit = Broc::GetCvarInt("mp_scorelimit");
    mp_util_wad::pLevel->roundLimit = Broc::GetCvarInt("mp_roundlimit");
    mp_util_wad::pLevel->friendlyFire = Broc::GetCvarInt("mp_friendlyfire") != 0;
    mp_util_wad::pLevel->lastManStanding = Broc::GetCvarInt("mp_lastmanstanding") != 0;
    if (!Broc::GetCvarInt("mp_teambalance") || Broc::GetCvarInt("mp_debug"))
        mp_util_wad::pLevel->teamBalance = false;
    else
        mp_util_wad::pLevel->teamBalance = true;
    mp_util_wad::pLevel->respawnTime = Broc::GetCvarInt("mp_respawntime");
    return 0;
}

// LaunchGametype - ea: 0x93C2C0
void LaunchGametype() {
    mp_util_wad::pLevel->RenderSpawnPoints = (void*)DebugRenderSpawnPoints;
    mp_util_wad::pLevel->PickSpawnPoint = (void*)GetSpawnPoint;
    mp_util_wad::pLevel->roundStarted = false;
    mp_util_wad::pLevel->roundCount = 0;
    mp_util_wad::pLevel->roundOver = false;
    mp_util_wad::pLevel->spawnType = 0;
    mp_util_wad::pLevel->rankOn = true;
    mp_util_wad::pLevel->forceMapChange = false;
    mp_util_wad::pLevel->lastManStandingIgnore = "none";
    mp_util_wad::pLevel->startTime = 0;
    mp_util_wad::pLevel->lastSpawnPointIndex = 0;
    mp_util_wad::pLevel->roundEndMusic = -1;
    mp_util_wad::pLevel->onlyDisplayScoreOnFinalRound = false;
    mp_util_wad::pLevel->teamCantRespawn = "";
    mp_util_wad::pLevel->ArtilleryObjectiveIndex = 0;
    mp_util_wad::pLevel->nextRoundStartTime = 0;
    mp_util_wad::pLevel->playerCountAtStartOfRound = 0;
    mp_util_wad::pLevel->playersLeavingDuringRound = 0;
    mp_util_wad::pLevel->spawnColorAllies = Broc::vector(0.0f, 1.0f, 0.0f);
    mp_util_wad::pLevel->spawnColorAxis = Broc::vector(1.0f, 0.0f, 0.0f);
    Broc::Code_SetShowScore(true);
    Broc::Code_DisplayScoreBoard(false, 20);
    SetupGameVariables();
    Broc::string gametype;
    Broc::GetCvar(&gametype, "mp_gametype");
    Broc::Code_ClearPlayerStats();
    Broc::Code_ClearTeamScores();
    mp_util_wad::pLevel->roundWinner = "";
    Broc::SetCvar("cg_night", "0");
    Broc::SetCvar("cg_hudObjectiveRingTime", "10000");
    Broc::SetCvar("cg_hudObjectiveNumRings", "10");
    mp_util_wad::pLevel->mustHaveBothTeamsToStart = false;
    Broc::gBrocAPI.mBrocExports.mCallbackSetLevelAudio = _mp_audio::CallbackSetLevelAudio;
    Broc::Code_SetupLevelSpecificVariables();
    _mp_tankdrive::main();
    _mp_nano::main();
    _mp_audio::main();
    Broc::entity lvl = Broc::entity();
    void* minefield = _mp_minefield::main__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "_mp_minefield::main", minefield);
    if (gametype == "dm")
        launch_gametype_thread(gametype, "_mp_dm::main", _mp_dm::main__functor);
    else if (gametype == "tdm")
        launch_gametype_thread(gametype, "_mp_tdm::main", _mp_tdm::main__functor);
    else if (gametype == "ctf")
        launch_gametype_thread(gametype, "_mp_ctf::main", _mp_ctf::main__functor);
    else if (gametype == "scf")
        launch_gametype_thread(gametype, "_mp_scf::main", _mp_scf::main__functor);
    else if (gametype == "war")
        launch_gametype_thread(gametype, "_mp_war::main", _mp_war::main__functor);
    else if (gametype == "hq")
        launch_gametype_thread(gametype, "_mp_hq::main", _mp_hq::main__functor);
    else if (Broc::gBrocAPI.mError)
        Broc::gBrocAPI.mError("c:\\cod\\code\\script\\_mp_common.bro", __LINE__, "Unknown gametype");
    gametype.~string();
}

// ============================================================================
// _mp_common callbacks and helpers.
// ============================================================================

// SetupCallbacks - ea: 0x93D1A0
void SetupCallbacks(Broc::bbool teamGameType) {
    Broc::BrocExports& x = Broc::gBrocAPI.mBrocExports;
    x.mCallbackPlayerJoin =
        (void (*)(const Broc::entity, const unsigned int, const int))CallbackPlayerJoin;
    x.mCallbackPlayerEnter = CallbackPlayerEnter;
    x.mCallbackPlayerLeave = CallbackPlayerLeave;
    x.mCallbackPlayerKilled = CallbackPlayerKilled;
    x.mCallbackPlayerAssist = CallbackPlayerAssist;
    x.mCallbackPlayerRespawnRequest = CallbackPlayerRespawnRequest;
    x.mCallbackPlayerSpawn = CallbackPlayerSpawn;
    x.mCallbackPlayerRevive = CallbackPlayerRevive;
    x.mCallbackPlayerTeamChange = CallbackPlayerTeamChange;
    x.mCallbackCanTeamChange = CallbackCanTeamChange;
    x.mCallbackPlayerClassChange = CallbackPlayerClassChange;
    x.mCallbackRoundOver = CallbackRoundOver;
    x.mCallbackGameState = CallbackGameState;
    x.mCallbackGameScore = CallbackGameScore;
    x.mCallbackNextRound = CallbackNextRound;
    x.mCallbackRestartMap = CallbackRestartMap;
    x.mCallbackQuitGame = (void (*)())CallbackQuitGame;
    x.mCallbackHostOptionsChanged = CallbackHostOptionsChanged;
    x.mCallbackHostDisconnected = (void (*)())CallbackHostDisconnected;
    x.mCallbackHostMigrated = (void (*)())CallbackHostMigrated;
    x.mCallbackStopFollowing = (void (*)())CallbackStopFollowing;
    x.mCallbackLocalPlayerKicked = (void (*)())CallbackLocalPlayerKicked;
    x.mCallbackFireArtillery =
        (void (*)(const Broc::entity, const Broc::vector))CallbackFireArtillery;
    x.mCallbackDenyArtillery = CallbackDenyArtillery;
    x.mCallbackFireArtilleryShell = CallbackFireArtilleryShell;
    x.mCallbackVehicleKilled = CallbackVehicleKilled;
    x.mCallbackVehicleMantled = CallbackVehicleMantled;
    x.mCallbackHealthRegenRecovering = CallbackHealthRegenRecovering;
    x.mCallbackPickupItem = CallbackPickupItem;
    x.mCallbackMineFailed = CallbackMineFailed;
    x.mCallbackReviveFailed = (void (*)(const Broc::entity))CallbackReviveFailed;
    x.mCallbackCallForMedic = CallbackCallForMedic;
    x.mCallbackPunishedForTeamKill = CallbackPunishedForTeamKill;
    x.mCallbackSpawnButtonPressed = CallbackSpawnButtonPressed;
    x.mCallbackPainFlinch = CallbackPainFlinch;
    x.mCallbackDebugRender = (void (*)())CallbackDebugRender;
    if ((bool)teamGameType)
        x.mCallbackPlayerDamage = CallbackPlayerDamageTeam;
    else
        x.mCallbackPlayerDamage = CallbackPlayerDamage;
}

// CallbackGameScore - ea: 0x93DFA0
void CallbackGameScore(int alliesScore, int axisScore) {
    Broc::Code_ClearTeamScores();
    Broc::string team("allies");
    Broc::Code_IncTeamScore(&team, alliesScore);
    team.~string();
    Broc::string team2("axis");
    Broc::Code_IncTeamScore(&team2, axisScore);
    team2.~string();
}

// CallbackPlayerLeave - ea: 0x93E970
void CallbackPlayerLeave(Broc::entity leavingPlayer) {
    HashStr label;
    label.mVal = 0x4C745BCAu;  // "disconnect"
    Broc::notify(&leavingPlayer, label);
    if ((bool)mp_util_wad::pLevel->roundStarted &&
        !(bool)mp_util_wad::pLevel->roundOver) {
        mp_util_wad::pLevel->playersLeavingDuringRound =
            (int)mp_util_wad::pLevel->playersLeavingDuringRound + 1;
    }
    if (!Broc::Code_IsLocalPlayer(leavingPlayer)) {
        char* name = Broc::Code_GetPlayerName(leavingPlayer);
        Broc::iprintln(name, " ", "MPSCRIPT_DISCONNECTED");
    }
}

// CallbackPlayerAssist - ea: 0x93F7F0
void CallbackPlayerAssist(Broc::entity assistplayer) {
    AddToPlayerStats(assistplayer, Broc::bint(5), 1);
    if (Broc::Code_IsLocalPlayer(assistplayer))
        Broc::iprintln("MPGAME_GOT_ASSIST");
}

// CallbackPainFlinch - ea: 0x93F860
void CallbackPainFlinch(Broc::entity player, int damage) {
    Broc::bint health;
    mp_util_wad::entity_get_maxhealth(&health, player);
    void* ftor = _mp_audio::PlayPainSound__functor(player, Broc::bint(damage));
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "_mp_audio::PlayPainSound", ftor);
}

// CallbackVehicleMantled - ea: 0x9436E0
void CallbackVehicleMantled(Broc::entity vehicle, Broc::entity mantler) {
    (void)vehicle;
    AddToPlayerStats(mantler, Broc::bint(9), 1);
}

// CallbackVehicleKilled - ea: 0x9435C0
void CallbackVehicleKilled(Broc::entity killedVehicle, Broc::entity inflictor,
                           Broc::entity attacker, int weapon, int mod,
                           int occupantCount) {
    (void)inflictor;
    (void)weapon;
    (void)mod;
    if (occupantCount > 0 && Broc::IsPlayer(&attacker) != 0) {
        AddToPlayerStats(attacker, Broc::bint(6), 1);
        Broc::entity spotter;
        Broc::Code_GetSpotterEntity(&spotter, killedVehicle);
        if (Broc::IsDefined(spotter) && spotter.___u0 != attacker.___u0) {
            AddToPlayerStats(attacker, Broc::bint(23), 1);
            Broc::Code_ClearSpottingFromOccupants(killedVehicle);
        }
    }
}

// CallbackHealthRegenRecovering - ea: 0x943720
void CallbackHealthRegenRecovering(Broc::entity self) {
    if (Broc::Code_IsLocalPlayer(self)) {
        Broc::string script("breathing_better");
        Broc::entity lvl;
        lvl.___u0 = mp_util_wad::pLevel != NULL;
        Broc::EffectEventPlay(&lvl, &script);
        script.~string();
    }
}

// CallbackMineFailed - ea: 0x943980
void CallbackMineFailed(Broc::entity ent) {
    if (Broc::Code_IsLocalPlayer(ent))
        Broc::SetActionHint((int)0xBA00F4DE, Broc::GetPlayerIndex(ent));
}

// CallbackReviveFailed - ea: 0x943A00
void CallbackReviveFailed() {
}

// CallbackDebugRender - ea: 0x9421D0
int CallbackDebugRender() {
    return ((int (*)())mp_util_wad::pLevel->RenderSpawnPoints)();
}

// CallbackNextRound - ea: 0x942050
void CallbackNextRound() {
    Broc::Code_DebugOut("*COMMON* CallbackNextRound\n");
    if (mp_util_wad::pLevel->roundEndMusic != -1) {
        Broc::EffectEventStopEmitting(mp_util_wad::pLevel->roundEndMusic);
        mp_util_wad::pLevel->roundEndMusic = -1;
    }
    Broc::Code_DisplayScoreBoard(false, 20);
    StartRound(Broc::bbool(false));
}

// CallbackRestartMap - ea: 0x9420E0
void CallbackRestartMap() {
    mp_util_wad::pLevel->roundCount =
        (int)mp_util_wad::pLevel->roundCount - 1;
    if ((int)mp_util_wad::pLevel->roundCount < 0)
        mp_util_wad::pLevel->roundCount = 0;
    Broc::Code_ClearTeamScores();
    Broc::Code_ClearPlayerStats();
    Broc::SetTutorialTextAllPlayers((int)0x6422EAEDu);
    Broc::Code_NextRound(false);
}

// CallbackStopFollowing - ea: 0x941350
unsigned int CallbackStopFollowing() {
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    void* ftor = StopFollowing__functor(lvl, true);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "StopFollowing", ftor);
}

// CallbackQuitGame - ea: 0x944580
unsigned int CallbackQuitGame() {
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    void* ftor = QuitGameThread__functor(lvl);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "QuitGameThread", ftor);
}

// CallbackHostDisconnected - ea: 0x9446A0
unsigned int CallbackHostDisconnected() {
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    HashStr msg;
    msg.mVal = 0xA45844A6;
    void* ftor = QuitGameWithMessage__functor(lvl, msg);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "QuitGameWithMessage", ftor);
}

// CallbackLocalPlayerKicked - ea: 0x944730
unsigned int CallbackLocalPlayerKicked() {
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    HashStr msg;
    msg.mVal = 0x3546BB54u;
    void* ftor = QuitGameWithMessage__functor(lvl, msg);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "QuitGameWithMessage", ftor);
}

// CallbackHostMigrated - ea: 0x9447C0
unsigned int CallbackHostMigrated() {
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    void* ftor = HostHasMigrated__functor(lvl);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "HostHasMigrated", ftor);
}

// CallbackSpawnButtonPressed - ea: 0x944440
void CallbackSpawnButtonPressed(Broc::entity ent) {
    if (Broc::Code_IsLocalPlayer(ent)) {
        HashStr label;
        label.mVal = 0xC39A4465;
        Broc::notify(&ent, label);
        void* ftor = LocalPlayerRespawn__functor(ent);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "LocalPlayerRespawn", ftor);
    }
}

// CallbackPunishedForTeamKill - ea: 0x943D60
void CallbackPunishedForTeamKill(Broc::entity ent, int punished) {
    void* ftor = PunishedForTeamKill__functor(ent, punished != 0);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "PunishedForTeamKill", ftor);
}

// CallbackPlayerClassChange - ea: 0x941270
void CallbackPlayerClassChange(Broc::entity player, unsigned int newPlayerClass) {
    HashStr label;
    label.mVal = 0x70FACFE9u;
    Broc::notify(&player, label);
    mp_util_wad::entity_set_nextPlayerClass(player, (__int16)newPlayerClass);
    *mp_util_wad::GetEE_specialWeaponChangeClassFlag(player) = true;
    if (Broc::Code_IsLocalPlayer(player)) {
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, player);
        if ((int)state == 3)
            _mp_loadout::DisplayYouWillSpawnWithMessage(player);
    }
}

// CallbackFireArtilleryShell - ea: 0x943310
void CallbackFireArtilleryShell(Broc::entity shell) {
    Broc::string script("MP_Artillery_Fire");
    Broc::EffectEventPlay(&shell, &script);
    script.~string();
}

// CallbackDenyArtillery - ea: 0x943390
void CallbackDenyArtillery(Broc::entity denied) {
    Broc::SetActionHint((int)0xD2D4BFD1, Broc::GetPlayerIndex(denied));
    Broc::string team;
    mp_util_wad::entity_get_team(&team, denied);
    if (team == "axis") {
        Broc::string sound("Scout_SpecialFire_Denied_Axis");
        void* ftor = _mp_audio::PlaySound__functor(denied, sound, 1.0f);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "_mp_audio::PlaySound", ftor);
    } else {
        Broc::string sound("Scout_SpecialFire_Denied_Allies");
        void* ftor = _mp_audio::PlaySound__functor(denied, sound, 1.0f);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "_mp_audio::PlaySound", ftor);
    }
    team.~string();
}

// CallbackCallForMedic - ea: 0x943A20
void CallbackCallForMedic(Broc::entity ent) {
    if (Broc::Code_IsLocalPlayer(ent)) {
        Broc::bint medic_seconds(5);
        Broc::bint now;
        Broc::GetTime(&now);
        Broc::bint cutoff = (int)medic_seconds * 1000;
        if ((int)*mp_util_wad::GetEE_lastMedicCall(ent) + (int)cutoff < (int)now) {
            Broc::SetSpectateMedic(0, 0);
            void* ftor = reenable_medic_call__functor(ent, (int)medic_seconds);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "reenable_medic_call", ftor);
        }
    }
    Broc::string team;
    mp_util_wad::entity_get_team(&team, ent);
    if (team == "allies") {
        Broc::string script("american_medic");
        Broc::EffectEventPlay(&ent, &script);
        script.~string();
    } else {
        Broc::string script("german_medic");
        Broc::EffectEventPlay(&ent, &script);
        script.~string();
    }
    team.~string();
}

// CallbackPickupItem - ea: 0x9437F0
void CallbackPickupItem(Broc::entity pickerupper, Broc::entity dropper) {
    if (Broc::IsPlayer(&dropper) == 0)
        return;
    Broc::string dteam;
    Broc::string pteam;
    mp_util_wad::entity_get_team(&dteam, dropper);
    mp_util_wad::entity_get_team(&pteam, pickerupper);
    bool sameTeam = pteam == dteam;
    dteam.~string();
    pteam.~string();
    if (sameTeam && pickerupper.___u0 != dropper.___u0)
        AddToPlayerStats(dropper, Broc::bint(12), 1);
}

// CallbackPlayerRespawnRequest - ea: 0x93EA80
void CallbackPlayerRespawnRequest(Broc::entity player, int team_allies) {
    Broc::string team(team_allies != 0 ? "allies" : "axis");
    void* ftor = RespawnPlayer__functor(player, team);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "RespawnPlayer", ftor);
}

// CallbackGameState - ea: 0x93D9C0
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int timeTillNextRound,
                       int roundCount) {
    Broc::Code_DebugOut("*COMMON* CallbackGameState\n");
    if (roundStarted != 0)
        Broc::Code_DebugOut("*COMMON* CallbackGameState Round NOT STARTED\n");
    else
        Broc::Code_DebugOut("*COMMON* CallbackGameState Round STARTED\n");
    bool wasRoundOver = (bool)mp_util_wad::pLevel->roundOver;
    bool wasRoundStarted = (bool)mp_util_wad::pLevel->roundStarted;
    mp_util_wad::pLevel->timeLimit = timeLimit;
    mp_util_wad::pLevel->scoreLimit = scoreLimit;
    mp_util_wad::pLevel->roundLimit = roundLimit;
    mp_util_wad::pLevel->friendlyFire = friendlyFire != 0;
    mp_util_wad::pLevel->lastManStanding = lastManStanding != 0;
    mp_util_wad::pLevel->teamBalance = teamBalance != 0;
    mp_util_wad::pLevel->respawnTime = respawnTime;
    mp_util_wad::pLevel->roundCount = roundCount;
    mp_util_wad::pLevel->roundStarted = roundStarted != 0;
    mp_util_wad::pLevel->roundOver = timeTillNextRound != 0;
    Broc::bint now;
    Broc::GetTime(&now);
    mp_util_wad::pLevel->startTime = (int)now - 1000 * currentMatchTime;
    Broc::bint timeLeft = 60 * timeLimit - currentMatchTime;
    CreateClock(timeLeft);
    Broc::SetCvar("mp_friendlyfire", friendlyFire);
    Broc::Code_ClearTeamScores();
    Broc::string team("allies");
    Broc::Code_IncTeamScore(&team, alliesScore);
    team.~string();
    Broc::string team2("axis");
    Broc::Code_IncTeamScore(&team2, axisScore);
    team2.~string();
    if (!wasRoundStarted && (bool)mp_util_wad::pLevel->roundStarted &&
        !(bool)mp_util_wad::pLevel->roundOver) {
        Broc::dyn_array<Broc::entity> players;
        Broc::GetPlayerArray(&players);
        mp_util_wad::pLevel->playerCountAtStartOfRound = Broc::size(players);
        mp_util_wad::pLevel->playersLeavingDuringRound = 0;
        players.~dyn_array();
    }
    if (!wasRoundOver && (bool)mp_util_wad::pLevel->roundOver &&
        timeTillNextRound > 3.0) {
        Broc::entity lvl;
        lvl.___u0 = mp_util_wad::pLevel != NULL;
        void* ftor = HandleJoinAfterRoundOver__functor(lvl, timeTillNextRound);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "HandleJoinAfterRoundOver", ftor);
    }
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    HashStr label;
    label.mVal = 0x531AD8D9u;
    Broc::notify(&lvl, label);
}

// CallbackHostOptionsChanged - ea: 0x93D3B0
void CallbackHostOptionsChanged(int forceMapChange) {
    Broc::bint timeLimit(Broc::GetCvarInt("mp_timelimit"));
    Broc::bint scoreLimit(Broc::GetCvarInt("mp_scorelimit"));
    Broc::bint roundLimit(Broc::GetCvarInt("mp_roundlimit"));
    Broc::bint friendlyFire(Broc::GetCvarInt("mp_friendlyfire"));
    Broc::bint lastManStanding(Broc::GetCvarInt("mp_lastmanstanding"));
    Broc::bint teamBalance(Broc::GetCvarInt("mp_teambalance"));
    Broc::bint respawnTime(Broc::GetCvarInt("mp_respawntime"));
    Broc::bint now;
    Broc::GetTime(&now);
    Broc::bfloat timePassed = ((int)now - (int)mp_util_wad::pLevel->startTime) * 0.001f;
    if ((int)timeLimit != (int)mp_util_wad::pLevel->timeLimit) {
        CreateClock(Broc::bint((int)timeLimit * 60));
        Broc::bint t;
        Broc::GetTime(&t);
        mp_util_wad::pLevel->startTime = (int)t;
        timePassed = 0.0f;
    }
    mp_util_wad::pLevel->timeLimit = (int)timeLimit;
    mp_util_wad::pLevel->scoreLimit = (int)scoreLimit;
    mp_util_wad::pLevel->roundLimit = (int)roundLimit;
    mp_util_wad::pLevel->friendlyFire = (int)friendlyFire != 0;
    mp_util_wad::pLevel->lastManStanding = (int)lastManStanding != 0;
    mp_util_wad::pLevel->teamBalance = (int)teamBalance != 0;
    mp_util_wad::pLevel->respawnTime = (int)respawnTime;
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint timeTillNextRound;
    GetTimeTillNextRound(&timeTillNextRound);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::string team("axis");
        Broc::string team2("allies");
        int axisScore = Broc::Code_GetTeamScore(&team);
        int alliesScore = Broc::Code_GetTeamScore(&team2);
        Broc::Code_SendGameState(
            players[(unsigned int)(int)i], (float)timePassed,
            (int)mp_util_wad::pLevel->timeLimit,
            (int)mp_util_wad::pLevel->scoreLimit,
            (int)mp_util_wad::pLevel->roundLimit,
            (bool)mp_util_wad::pLevel->friendlyFire,
            (bool)mp_util_wad::pLevel->lastManStanding,
            (bool)mp_util_wad::pLevel->teamBalance,
            (int)mp_util_wad::pLevel->respawnTime, alliesScore, axisScore,
            (bool)mp_util_wad::pLevel->roundStarted,
            (int)timeTillNextRound, (int)mp_util_wad::pLevel->roundCount);
        team2.~string();
        team.~string();
        i = (int)i + 1;
    }
    players.~dyn_array();
    if (forceMapChange != 0) {
        mp_util_wad::pLevel->forceMapChange = true;
        Broc::string winner;
        GetWinningTeam(&winner);
        EndRound(kEndRoundNone, winner);
    }
}

// CreateClock - ea: 0x946080
void CreateClock(Broc::bint timeLimit) {
    if ((int)timeLimit > 0)
        Broc::Code_SetShowTime((float)(int)timeLimit);
}

// RunFrame - ea: 0x946040
void RunFrame() {
    for (;;) {
        waitframe();
        CheckTimeLimit();
        CheckScoreLimit();
    }
}

// waitframe - ea: 0x9483A0
void waitframe() {
    Broc::wait(0.05f);
}

// CheckTimeLimit - ea: 0x9474B0
void CheckTimeLimit() {
    if ((bool)mp_util_wad::pLevel->roundStarted &&
        (int)mp_util_wad::pLevel->timeLimit > 0) {
        Broc::bint now;
        Broc::GetTime(&now);
        Broc::bfloat timepassed = ((int)now - (int)mp_util_wad::pLevel->startTime) * 0.001f;
        static Broc::bfloat invMin(0.016666668f);  // 1/60 minutes per ms
        timepassed = (float)timepassed * (float)invMin;
        if (!((float)timepassed < (int)mp_util_wad::pLevel->timeLimit) &&
            !(bool)mp_util_wad::pLevel->roundOver) {
            Broc::string winner;
            GetWinningTeam(&winner);
            EndRound(kEndRoundTimeLimit, winner);
        }
    }
}

// CheckScoreLimit - ea: 0x9476D0
void CheckScoreLimit() {
    if ((int)mp_util_wad::pLevel->scoreLimit <= 0)
        return;
    bool hit = false;
    if (!Broc::Code_GetTeamGame()) {
        Broc::dyn_array<Broc::entity> players;
        Broc::GetPlayerArray(&players);
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            Broc::entity p = players[(unsigned int)(int)i];
            int score = Broc::Code_GetPlayerTotalScore(p);
            if (score >= (int)mp_util_wad::pLevel->scoreLimit) {
                hit = true;
                break;
            }
            i = (int)i + 1;
        }
        players.~dyn_array();
    } else {
        Broc::string team("allies");
        Broc::string team2("axis");
        int alliesScore = Broc::Code_GetTeamScore(&team);
        int axisScore = Broc::Code_GetTeamScore(&team2);
        team.~string();
        team2.~string();
        hit = alliesScore < (int)mp_util_wad::pLevel->scoreLimit &&
              axisScore < (int)mp_util_wad::pLevel->scoreLimit;
        if (hit)
            return;
    }
    if (!(bool)mp_util_wad::pLevel->roundOver) {
        Broc::string winner;
        GetWinningTeam(&winner);
        EndRound(kEndRoundScoreLimit, winner);
    }
}

// EndRound - ea: 0x94AE30
void EndRound(EEndRoundCondition condition, Broc::string winner) {
    if (!(bool)mp_util_wad::pLevel->roundOver)
        Broc::Code_RoundOver((int)condition, &winner);
    winner.~string();
}

// GetTimeTillNextRound - ea: 0x94C380
Broc::bint* GetTimeTillNextRound(Broc::bint* result) {
    if ((bool)mp_util_wad::pLevel->roundOver) {
        Broc::bint now;
        Broc::GetTime(&now);
        Broc::bint timeTillNextRound =
            ((int)mp_util_wad::pLevel->nextRoundStartTime - (int)now) / 1000;
        if ((int)timeTillNextRound < 0)
            timeTillNextRound = 20;
        if ((bool)mp_util_wad::pLevel->roundOver &&
            (int)timeTillNextRound == 0)
            timeTillNextRound = 1;
        result->mVal = (int)timeTillNextRound;
        return result;
    }
    result->mVal = 0;
    return result;
}

// GetGoingToDieTime - ea: 0x949B60
Broc::bint* GetGoingToDieTime(Broc::bint* result) {
    Broc::bint now;
    Broc::GetTime(&now);
    result->mVal = (int)now + 20000;
    return result;
}

// DestroyHudElem - ea: 0x9483D0
void DestroyHudElem(Broc::hudelem* elem) {
    if (Broc::IsDefined(*elem)) {
        Broc::Destroy(elem);
        elem->SetUndefined();
    }
}

// ProgressBarCreate - ea: 0x94AA80
void ProgressBarCreate(Broc::bint width) {
    Broc::hudelem tmp;
    mp_util_wad::pLevel->progress_bar = *Broc::NewHudElem(&tmp, -1);
    mp_util_wad::pLevel->progress_bar.x(160);
    mp_util_wad::pLevel->progress_bar.y(360);
    mp_util_wad::pLevel->progress_bar.sort(0.0f);
    mp_util_wad::pLevel->progress_bar.alpha(0x80);
    Broc::string s("white");
    Broc::SetShader(&mp_util_wad::pLevel->progress_bar, &s, (int)width, 32);
    s.~string();
}

// ProgressBarUpdate - ea: 0x94AD50
void ProgressBarUpdate(Broc::bfloat value, Broc::bfloat time) {
    int w = (int)((float)value * 320.0f);
    float scaleTime = (float)time;
    Broc::ScaleOverTime(&mp_util_wad::pLevel->progress_bar, scaleTime, w, 32);
}

// ProgressBarDelete - ea: 0x94AE00
void ProgressBarDelete() {
    DestroyHudElem(&mp_util_wad::pLevel->progress_bar);
}

// QuitGameThread - ea: 0x9495C0
void QuitGameThread() {
    Broc::Code_ScreenFadeToBlack(0xFAu, -1);
    Broc::wait(0.25f);
    Broc::Code_QuitGame();
}

// NewHost - ea: 0x944F40
void NewHost() {
    Broc::SetTutorialText((int)0x4D843295u, 0);
    Broc::wait(4.0f);
}

// TeamChangeKillPlayer - ea: 0x940D40
void TeamChangeKillPlayer(Broc::entity player) {
    Broc::wait(0.1f);
    for (;;) {
        Broc::bint health;
        mp_util_wad::entity_get_health(&health, player);
        if (!((int)health > 0))
            break;
        Broc::SetTakeDamage(&player, 1);
        Broc::vector origin;
        mp_util_wad::entity_get_origin(&origin, player);
        Broc::vector down(0.0f, 0.0f, -5.0f);
        Broc::vector vecIn = origin + down;
        Broc::DoDamage(&player, 10000.0f, &vecIn, HITLOC_NONE);
        Broc::wait(0.1f);
    }
}

// GetWinningTeam - ea: 0x94C4A0
Broc::string* GetWinningTeam(Broc::string* result) {
    Broc::string team("allies");
    Broc::bint allies_score(Broc::Code_GetTeamScore(&team));
    team.~string();
    Broc::string team2("axis");
    Broc::bint axis_score(Broc::Code_GetTeamScore(&team2));
    team2.~string();
    Broc::bint allies_total_score(0);
    Broc::bint axis_total_score(0);
    Broc::bint allies_kills(0);
    Broc::bint axis_kills(0);
    Broc::bint allies_deaths(0);
    Broc::bint axis_deaths(0);
    if ((int)allies_score == (int)axis_score) {
        Broc::dyn_array<Broc::entity> players;
        Broc::GetPlayerArray(&players);
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            Broc::entity p = players[(unsigned int)(int)i];
            if (Broc::IsDefined(p)) {
                Broc::string pteam;
                mp_util_wad::entity_get_team(&pteam, p);
                if (pteam == "allies") {
                    allies_total_score = (int)allies_total_score + Broc::Code_GetPlayerTotalScore(p);
                    allies_kills = (int)allies_kills + Broc::Code_GetPlayerStat(p, 3);
                    axis_deaths = (int)axis_deaths + Broc::Code_GetPlayerStat(p, 4);
                } else {
                    axis_total_score = (int)axis_total_score + Broc::Code_GetPlayerTotalScore(p);
                    axis_kills = (int)axis_kills + Broc::Code_GetPlayerStat(p, 3);
                    allies_deaths = (int)allies_deaths + Broc::Code_GetPlayerStat(p, 4);
                }
                pteam.~string();
            }
            i = (int)i + 1;
        }
        players.~dyn_array();
        allies_score = (int)allies_total_score;
        axis_score = (int)axis_total_score;
        if ((int)allies_score == (int)axis_score) {
            allies_score = (int)allies_kills;
            axis_score = (int)axis_kills;
        }
        if ((int)allies_score == (int)axis_score) {
            allies_score = (int)allies_deaths;
            axis_score = (int)axis_deaths;
        }
    }
    if ((int)allies_score > (int)axis_score)
        *result = "allies";
    else if ((int)axis_score > (int)allies_score)
        *result = "axis";
    else
        *result = "none";
    return result;
}

// HasBothTeams - ea: 0x94AEC0
Broc::bbool* HasBothTeams(Broc::bbool* result) {
    Broc::bint axis(0);
    Broc::bint allies(0);
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, p);
        if (Broc::Code_IsLocalPlayer(p) || (int)state != 0) {
            Broc::string team;
            mp_util_wad::entity_get_team(&team, p);
            if (team == "axis")
                axis = (int)axis + 1;
            Broc::string team2;
            mp_util_wad::entity_get_team(&team2, p);
            if (team2 == "allies")
                allies = (int)allies + 1;
            team.~string();
            team2.~string();
        }
        if (((int)axis != 0 && (int)allies != 0) ||
            (!Broc::Code_GetTeamGame() &&
             (int)axis + (int)allies > 1)) {
            *result = Broc::bbool(true);
            players.~dyn_array();
            return result;
        }
        i = (int)i + 1;
    }
    *result = Broc::bbool(false);
    players.~dyn_array();
    return result;
}

// WaitForTeams - ea: 0x94B210
void WaitForTeams() {
    Broc::bint start_time;
    Broc::GetTime(&start_time);
    for (;;) {
        Broc::bbool both;
        HasBothTeams(&both);
        if ((bool)both)
            break;
        Broc::bint t;
        Broc::GetTime(&t);
        mp_util_wad::pLevel->startTime = (int)t;
        int hash = Broc::Code_GetTeamGame() ? (int)0xCC85852C : (int)0xCFAC23E6;
        Broc::SetTutorialTextAllPlayers(hash);
        Broc::wait(0.1f);
    }
    Broc::bint now;
    Broc::GetTime(&now);
    if ((int)start_time + 4000 < (int)now)
        Broc::SetTutorialTextAllPlayers((int)0x6422EAEDu);
}

// HandleJoinAfterRoundOver - ea: 0x94BAE0
void HandleJoinAfterRoundOver(Broc::entity self, Broc::bint timeleft) {
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    HashStr e1;
    e1.mVal = 0xAFE7EFF4;
    Broc::endon(lvl, e1);
    HashStr e2;
    e2.mVal = 0x4DEC2E76u;
    Broc::endon(lvl, e2);
    PutAllPlayersIntoIntermission();
    Broc::wait(1.0f);
    Broc::Code_DisplayScoreBoard(true, (int)timeleft);
    (void)self;
}

// QuitGameWithMessage - ea: 0x9448E0
void QuitGameWithMessage(Broc::entity self, HashStr message) {
    Broc::CloseAllMenus(0);
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    if (self.___u0 != lvl.___u0) {
        Broc::SetTutorialText((int)message.mVal, Broc::GetPlayerIndex(self));
    } else {
        Broc::SetTutorialTextAllPlayers((int)message.mVal);
    }
    Broc::wait(5.0f);
    if (Broc::GetCvarInt("mp_debug") == 0)
        CallbackQuitGame();
}

// reenable_medic_call - ea: 0x94C220
void reenable_medic_call(Broc::entity self, Broc::bint time) {
    static const unsigned int labels[] = {
        0x4C745BCAu, 0xC1E6FED9, 0x24B5BA64u, 0x86018C9u, 0x11524591u, 0x6A7DEF26u
    };
    for (int i = 0; i < 6; i++) {
        HashStr label;
        label.mVal = labels[i];
        Broc::endon(self, label);
    }
    Broc::wait((float)(int)time);
    Broc::SetSpectateMedic(1, 0);
}

// AudioOnPlayerDeath - ea: 0x94B650
void AudioOnPlayerDeath(Broc::entity entself, Broc::bint weapon) {
    (void)weapon;
    if (Broc::Code_IsLocalPlayer(entself)) {
        Broc::string script("Player_Dying");
        Broc::EffectEventPlay(&entself, &script);
        script.~string();
    }
}

// FadeUpWhenLoaded - ea: 0x944F90
void FadeUpWhenLoaded(Broc::entity self, Broc::entity player) {
    (void)self;
    if ((bool)mp_util_wad::pLevel->roundStarted ||
        !Broc::IsDefined(mp_util_wad::pLevel->isInFadeUpWhenLoaded)) {
        mp_util_wad::pLevel->isInFadeUpWhenLoaded = 1;
        Broc::entity lvl;
        lvl.___u0 = mp_util_wad::pLevel != NULL;
        HashStr label;
        label.mVal = 0x2A9ACF98u;
        Broc::waittill(lvl, label);
        Broc::Code_ScreenFadeUp(0xFAu, Broc::GetPlayerIndex(player));
        mp_util_wad::pLevel->isInFadeUpWhenLoaded = 0;
    }
}

// PunishedForTeamKill - ea: 0x943E70
void PunishedForTeamKill(Broc::entity ent, Broc::bbool punished) {
    if (!(bool)punished) {
        Broc::SetActionHint((int)0xE5433CE9, Broc::GetPlayerIndex(ent));
        return;
    }
    *mp_util_wad::GetEE_punishedTeamKills(ent) =
        (int)*mp_util_wad::GetEE_punishedTeamKills(ent) + 1;
    bool local = Broc::Code_IsLocalPlayer(ent);
    if (local && (int)*mp_util_wad::GetEE_punishedTeamKills(ent) < 5) {
        if ((int)*mp_util_wad::GetEE_punishedTeamKills(ent) == 4) {
            Broc::SetActionHint((int)0x8B44B66A, Broc::GetPlayerIndex(ent));
            Broc::string team;
            mp_util_wad::entity_get_team(&team, ent);
            if (team == "allies") {
                Broc::string script("MP_Scream_FriendlyFire_Allies");
                Broc::EffectEventPlay(&ent, &script);
                script.~string();
            } else {
                Broc::string script("MP_Scream_FriendlyFire_Axis");
                Broc::EffectEventPlay(&ent, &script);
                script.~string();
            }
            team.~string();
        } else {
            Broc::SetActionHint((int)0xB28B48E9, Broc::GetPlayerIndex(ent));
        }
        Broc::iprintln(Broc::Code_GetPlayerName(ent), " ",
                       "MPGAME_WAS_PUNISHED_FOR_TEAMKILL");
    } else if (local) {
        Broc::string team;
        mp_util_wad::entity_get_team(&team, ent);
        if (team == "allies") {
            Broc::string name("MP_Demerit_Courtmarshal_Allies");
            Broc::SoundPlay(name, 1.0f);
            name.~string();
        } else {
            Broc::string name("MP_Demerit_Courtmarshal_Axis");
            Broc::SoundPlay(name, 1.0f);
            name.~string();
        }
        team.~string();
        if (!Broc::Code_IsRankedGame() || !Broc::Code_IsHost()) {
            Broc::wait(2.0f);
            Broc::entity lvl;
            lvl.___u0 = mp_util_wad::pLevel != NULL;
            HashStr msg;
            msg.mVal = 0xF2554CB4;
            void* ftor = QuitGameWithMessage__functor(lvl, msg);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "QuitGameWithMessage", ftor);
        }
    } else {
        Broc::iprintln(Broc::Code_GetPlayerName(ent), " ",
                       "MPGAME_WAS_PUNISHED_FOR_TEAMKILL");
    }
}

// CallbackCanTeamChange - ea: 0x9408B0
int CallbackCanTeamChange(Broc::entity player, int team_allies) {
    if (!Broc::Code_GetTeamGame())
        return 1;
    if (!(bool)mp_util_wad::pLevel->teamBalance)
        return 1;
    Broc::string new_team("allies");
    if (team_allies == 0)
        new_team = "axis";
    Broc::string cur;
    mp_util_wad::entity_get_team(&cur, player);
    bool same = cur == new_team;
    cur.~string();
    if (same) {
        new_team.~string();
        return 1;
    }
    Broc::dyn_array<Broc::entity> players;
    Broc::bint allies(0);
    Broc::bint axis(0);
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        if (p != player) {
            Broc::string pteam;
            mp_util_wad::entity_get_team(&pteam, p);
            if (pteam == "allies")
                allies = (int)allies + 1;
            else if (pteam == "axis")
                axis = (int)axis + 1;
            pteam.~string();
        }
        i = (int)i + 1;
    }
    players.~dyn_array();
    if ((int)allies >= (int)axis && new_team == "axis") {
        new_team.~string();
        return 1;
    }
    if ((int)axis >= (int)allies && new_team == "allies") {
        new_team.~string();
        return 1;
    }
    Broc::SetActionHint((int)0x1E885938u, Broc::GetPlayerIndex(player));
    new_team.~string();
    return 0;
}

// CallbackPlayerTeamChange - ea: 0x940EC0
void CallbackPlayerTeamChange(Broc::entity player, int autoBalance, int denied) {
    HashStr label;
    label.mVal = 0xE7D05EDA;
    Broc::notify(&player, label);
    if (denied == 0) {
        *mp_util_wad::GetEE_specialWeaponTime(player) = 0;
        bool local = Broc::Code_IsLocalPlayer(player);
        if (local) {
            Broc::bint state;
            mp_util_wad::entity_get_playerState(&state, player);
            if ((int)state == 3) {
                if (Broc::Code_IsInVehicle(player))
                    Broc::Code_GetOutOfVehicle(player);
                void* ftor = TeamChangeKillPlayer__functor(player);
                Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                    __LINE__, "TeamChangeKillPlayer", ftor);
            }
        }
        if (autoBalance != 0)
            *mp_util_wad::GetEE_spawnCount(player) = 0;
        if (!Broc::Code_IsLocalPlayer(player))
            _mp_loadout::UpdatePlayerModelForRank(player);
        Broc::bint state2;
        mp_util_wad::entity_get_playerState(&state2, player);
        if ((int)state2 == 4) {
            AddToPlayerStats(player, Broc::bint(4), 1);
            mp_util_wad::entity_set_playerState(player, 5);
            void* ftor = _mp_audio::player_dying_sounds__functor(player);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "_mp_audio::player_dying_sounds", ftor);
        }
    }
}

// CallbackPlayerJoin - ea: 0x93E060
void CallbackPlayerJoin(Broc::entity player, unsigned int playerState,
                        __int16 playerClass) {
    Broc::Code_DebugOut("*COMMON* CallbackPlayerJoin\n");
    bool local = Broc::Code_IsLocalPlayer(player);
    if (local) {
        Broc::SetTutorialText(-1, Broc::GetPlayerIndex(player));
        Broc::SetActionHint(-1, Broc::GetPlayerIndex(player));
        _mp_loadout::local_player_joined(player);
    }
    Broc::Code_SendInitialGameState(player);
    mp_util_wad::entity_set_playerClass(player, playerClass);
    mp_util_wad::entity_set_nextPlayerClass(player, playerClass);
    *mp_util_wad::GetEE_spawnCount(player) = 0;
    *mp_util_wad::GetEE_punishedTeamKills(player) = 0;
    *mp_util_wad::GetEE_killsSinceLastDeath(player) = 0;
    *mp_util_wad::GetEE_specialWeaponTime(player) = 0;
    *mp_util_wad::GetEE_specialWeaponChangeClassFlag(player) = false;
    if (playerState == 0) {
        if (!Broc::Code_IsLocalPlayer(player))
            SpawnIntermission(player);
    } else if (playerState == 1) {
        mp_util_wad::entity_set_playerState(player, 2);
        put_player_into_spectate_mode(player);
    } else {
        mp_util_wad::entity_set_playerState(player, (int)playerState);
        mp_util_wad::entity_set_spectatorClient(player, -1);
        Broc::vector origin;
        Broc::vector angles;
        mp_util_wad::entity_get_origin(&origin, player);
        mp_util_wad::entity_get_angles(&angles, player);
        Broc::Code_PlayerSpawn(player, &origin, &angles, 1);
        Broc::Code_SetPlayerAlive(player, Broc::GetCvarInt("g_player_maxhealth"));
    }
}

// CallbackPlayerEnter - ea: 0x93E5C0
void CallbackPlayerEnter(Broc::entity player, int hot_joiner) {
    Broc::Code_DebugOut("*COMMON* CallbackPlayerEnter\n");
    Broc::bint now;
    Broc::GetTime(&now);
    Broc::bfloat timePassed =
        ((int)now - (int)mp_util_wad::pLevel->startTime) * 0.001f;
    if (!Broc::Code_IsLocalPlayer(player)) {
        if ((float)timePassed > 3.0f)
            Broc::iprintln(Broc::Code_GetPlayerName(player), " ",
                           "MPSCRIPT_CONNECTED");
    }
    Broc::bint state;
    mp_util_wad::entity_get_playerState(&state, player);
    if ((int)state == 0) {
        *mp_util_wad::GetEE_firstSpectate(player) = hot_joiner == 0;
        put_player_into_spectate_mode(player);
    }
    Broc::bint timeTillNextRound;
    GetTimeTillNextRound(&timeTillNextRound);
    Broc::string team("axis");
    Broc::string team2("allies");
    int axisScore = Broc::Code_GetTeamScore(&team);
    int alliesScore = Broc::Code_GetTeamScore(&team2);
    Broc::Code_SendGameState(
        player, (float)timePassed, (int)mp_util_wad::pLevel->timeLimit,
        (int)mp_util_wad::pLevel->scoreLimit, (int)mp_util_wad::pLevel->roundLimit,
        (bool)mp_util_wad::pLevel->friendlyFire,
        (bool)mp_util_wad::pLevel->lastManStanding,
        (bool)mp_util_wad::pLevel->teamBalance,
        (int)mp_util_wad::pLevel->respawnTime, alliesScore, axisScore,
        (bool)mp_util_wad::pLevel->roundStarted, (int)timeTillNextRound,
        (int)mp_util_wad::pLevel->roundCount);
    team2.~string();
    team.~string();
    Broc::Code_SendVehicleStates(player);
}

// CallbackPlayerSpawn - ea: 0x93FCF0
void CallbackPlayerSpawn(Broc::entity player, int team_changed) {
    Broc::bint state;
    mp_util_wad::entity_get_playerState(&state, player);
    if ((int)state == 4)
        AddToPlayerStats(player, Broc::bint(4), 1);
    mp_util_wad::entity_set_playerState(player, 3);
    mp_util_wad::entity_set_spectatorClient(player, -1);
    *mp_util_wad::GetEE_autobalance(player) = false;
    mp_util_wad::entity_set_health(player, Broc::GetCvarInt("g_player_maxhealth"));
    *mp_util_wad::GetEE_lastMedicCall(player) = 0;
    if (team_changed != 0) {
        if (Broc::Code_IsLocalPlayer(player)) {
            Broc::string team;
            mp_util_wad::entity_get_team(&team, player);
            int hash = (team == "allies") ? (int)0xFFFD56Bu : (int)0xD57761E6;
            Broc::SetActionHint(hash, Broc::GetPlayerIndex(player));
            team.~string();
        }
    }
    HashStr label;
    label.mVal = 0x86018C9u;
    Broc::notify(&player, label);
    bool localPlayer = Broc::Code_IsLocalPlayer(player);
    _mp_loadout::GiveLoadout(player);
    *mp_util_wad::GetEE_spawnCount(player) =
        (int)*mp_util_wad::GetEE_spawnCount(player) + 1;
    Broc::bint t;
    Broc::GetTime(&t);
    *mp_util_wad::GetEE_spawnTime(player) = (int)t;
    if (localPlayer) {
        float healthCap = (float)Broc::GetCvarInt("g_player_maxhealth") * 0.35f;
        void* ftor = HealthRegenPlayerBreathing__functor(player, (int)healthCap);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "HealthRegenPlayerBreathing", ftor);
        HashStr label2;
        label2.mVal = 0x86018C9u;
        Broc::notify(&player, label2);
        Broc::entity lvl;
        lvl.___u0 = mp_util_wad::pLevel != NULL;
        void* ftor2 = FadeUpWhenLoaded__functor(lvl, player);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "FadeUpWhenLoaded", ftor2);
    } else {
        Broc::Code_SetPlayerAlive(player, Broc::GetCvarInt("g_player_maxhealth"));
    }
}

// CallbackPlayerRevive - ea: 0x940400
void CallbackPlayerRevive(Broc::entity player, Broc::entity medic) {
    mp_util_wad::entity_set_playerState(player, 3);
    mp_util_wad::entity_set_spectatorClient(player, -1);
    *mp_util_wad::GetEE_autobalance(player) = false;
    mp_util_wad::entity_set_health(player, Broc::GetCvarInt("g_player_maxhealth"));
    HashStr label;
    label.mVal = 0x11524591u;
    Broc::notify(&player, label);
    if (Broc::IsPlayer(&medic) != 0)
        AddToPlayerStats(medic, Broc::bint(11), 1);
    Broc::string weapon("mp_revive");
    Broc::Code_Obituary(player, medic, &weapon, 1, 0);
    weapon.~string();
    if (Broc::Code_IsLocalPlayer(player)) {
        Broc::string menu("weapon");
        Broc::CloseMenu(&menu, Broc::GetPlayerIndex(player));
        menu.~string();
        Broc::string menu2("spectate");
        Broc::CloseMenu(&menu2, Broc::GetPlayerIndex(player));
        menu2.~string();
        Broc::entity lvl;
        lvl.___u0 = mp_util_wad::pLevel != NULL;
        void* ftor = FadeUpWhenLoaded__functor(lvl, player);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "FadeUpWhenLoaded", ftor);
        if (Broc::Code_GetTeamGame()) {
            HashStr label2;
            label2.mVal = 0xC9444C8D;
            Broc::notify(&player, label2);
            _mp_loadout::GiveSpecialWeapon(
                player, (int)mp_util_wad::entity_get_playerClass(player),
                (int)mp_util_wad::entity_get_rank(player), true);
        }
    } else {
        Broc::Code_SetPlayerAlive(player, Broc::GetCvarInt("g_player_maxhealth"));
        Broc::string script("Plyr_Revive");
        Broc::EffectEventPlay(&player, &script);
        script.~string();
    }
}

// CallbackPlayerDamage - ea: 0x93F970
void CallbackPlayerDamage(Broc::entity player, Broc::entity inflictor,
                          Broc::entity attacker, const Broc::vector* dir,
                          const Broc::vector* point, int damage, int mod,
                          int weapon, int hitLoc) {
    if (hitLoc == 2 && mod != 11 && mod != 31)
        mod = 12;
    _mp_shellshock::ShellshockOnDamage(player, Broc::bint(mod), Broc::bint(damage));
    Broc::Code_FinishDamage(player, inflictor, attacker, dir, point, damage,
                            mod, weapon, hitLoc);
}

// CallbackPlayerDamageTeam - ea: 0x93FA90
void CallbackPlayerDamageTeam(Broc::entity player, Broc::entity inflictor,
                              Broc::entity attacker, const Broc::vector* dir,
                              const Broc::vector* point, int damage, int mod,
                              int weapon, int hitLoc) {
    bool teamKill = false;
    if (!(bool)mp_util_wad::pLevel->friendlyFire &&
        Broc::IsDefined(attacker) && Broc::IsPlayer(&attacker) != 0 &&
        attacker != player) {
        Broc::string pteam;
        Broc::string ateam;
        mp_util_wad::entity_get_team(&pteam, player);
        mp_util_wad::entity_get_team(&ateam, attacker);
        if (pteam == ateam)
            teamKill = !(Broc::Code_IsInVehicle(attacker) && mod == 2);
        pteam.~string();
        ateam.~string();
    }
    if (!teamKill)
        CallbackPlayerDamage(player, inflictor, attacker, dir, point, damage,
                             mod, weapon, hitLoc);
}

// AddToPlayerStats - ea: 0x948480
void AddToPlayerStats(Broc::entity player, Broc::bint stat, __int16 amount) {
    Broc::Code_IncPlayerStat(player, (int)stat, amount);
    Broc::bint score(Broc::Code_GetPlayerTotalScore(player));
    __int16 oldRank = mp_util_wad::entity_get_rank(player);
    if ((int)score < 30) {
        if ((int)score < 10)
            mp_util_wad::entity_set_rank(player, 0);
        else
            mp_util_wad::entity_set_rank(player, 1);
    } else {
        mp_util_wad::entity_set_rank(player, 2);
    }
    if (Broc::Code_IsLocalPlayer(player)) {
        __int16 rank = mp_util_wad::entity_get_rank(player);
        if (rank > oldRank) {
            Broc::string team;
            mp_util_wad::entity_get_team(&team, player);
            if (mp_util_wad::entity_get_rank(player) == 1) {
                if (team == "allies") {
                    Broc::string sound("MP_PromoCpl_Allies");
                    void* ftor = _mp_audio::PlaySound__functor(player, sound, 1.0f);
                    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                        __LINE__, "_mp_audio::PlaySound", ftor);
                } else {
                    Broc::string sound("MP_PromoCpl_Axis");
                    void* ftor = _mp_audio::PlaySound__functor(player, sound, 1.0f);
                    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                        __LINE__, "_mp_audio::PlaySound", ftor);
                }
            }
            if (mp_util_wad::entity_get_rank(player) == 2) {
                if (team == "allies") {
                    Broc::string sound("MP_PromoSgt_Allies");
                    void* ftor = _mp_audio::PlaySound__functor(player, sound, 1.0f);
                    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                        __LINE__, "_mp_audio::PlaySound", ftor);
                } else {
                    Broc::string sound("MP_PromoSgt_Axis");
                    void* ftor = _mp_audio::PlaySound__functor(player, sound, 1.0f);
                    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                        __LINE__, "_mp_audio::PlaySound", ftor);
                }
            }
            team.~string();
        }
    }
}

// CallbackPlayerKilled - ea: 0x93EC10
void CallbackPlayerKilled(Broc::entity killedPlayer, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health) {
    Broc::string weaponName((const char*)NULL);
    Broc::Code_GetWeaponName(weapon, &weaponName);
    Broc::entity team_killer;
    team_killer.___u0 = 0;
    if (attacker.IsDefined() && Broc::IsPlayer(&attacker) != 0) {
        if (attacker != killedPlayer) {
            bool teamKill = false;
            if (Broc::Code_GetTeamGame()) {
                Broc::string kt;
                Broc::string at;
                mp_util_wad::entity_get_team(&kt, killedPlayer);
                mp_util_wad::entity_get_team(&at, attacker);
                teamKill = kt == at;
                kt.~string();
                at.~string();
            }
            if (teamKill) {
                if (mod != 17 && mod != 18 && mod != 5 && mod != 6) {
                    if (Broc::Code_IsLocalPlayer(attacker))
                        Broc::SetActionHint((int)0x69F4FD86u,
                                            Broc::GetPlayerIndex(attacker));
                    if (Broc::Code_IsLocalPlayer(killedPlayer))
                        team_killer = attacker;
                    AddToPlayerStats(attacker, Broc::bint(8), 1);
                    *mp_util_wad::GetEE_killsSinceLastDeath(attacker) = 0;
                }
            } else {
                AddToPlayerStats(attacker, Broc::bint(3), 1);
                switch (mod) {
                case 17:
                case 18:
                    AddToPlayerStats(attacker, Broc::bint(13), 1);
                    break;
                case 3:
                case 4:
                    if (stricmp(weaponName.c_str(), "m1garand_rg") == 0 ||
                        stricmp(weaponName.c_str(), "k98_rg") == 0)
                        AddToPlayerStats(attacker, Broc::bint(15), 1);
                    else
                        AddToPlayerStats(attacker, Broc::bint(10), 1);
                    break;
                case 5:
                case 6:
                    AddToPlayerStats(attacker, Broc::bint(14), 1);
                    break;
                default:
                    break;
                }
                Broc::entity spotter;
                Broc::Code_GetSpotterEntity(&spotter, killedPlayer);
                if (Broc::IsDefined(spotter))
                    AddToPlayerStats(attacker, Broc::bint(23), 1);
            }
        }
    } else {
        if (!(bool)*mp_util_wad::GetEE_autobalance(killedPlayer))
            AddToPlayerStats(attacker, Broc::bint(7), 1);
    }
    if (Broc::IsDefined(attacker))
        *mp_util_wad::GetEE_killsSinceLastDeath(attacker) =
            (int)*mp_util_wad::GetEE_killsSinceLastDeath(attacker) + 1;
    *mp_util_wad::GetEE_killsSinceLastDeath(killedPlayer) = 0;
    *mp_util_wad::GetEE_autobalance(killedPlayer) = false;
    Broc::Code_SetRespawnMaxTime(killedPlayer, 0);
    if (mod == 25 || health <= -250) {
        AddToPlayerStats(killedPlayer, Broc::bint(4), 1);
        mp_util_wad::entity_set_playerState(killedPlayer, 5);
    } else {
        mp_util_wad::entity_set_playerState(killedPlayer, 4);
    }
    Broc::bint now;
    Broc::GetTime(&now);
    Broc::bint timeAlive =
        (int)now - (int)*mp_util_wad::GetEE_spawnTime(killedPlayer);
    if ((int)timeAlive > 1000)
        AddToPlayerStats(killedPlayer, Broc::bint(0), (__int16)((int)timeAlive / 1000));
    Broc::Code_Obituary(killedPlayer, attacker, &weaponName, mod, 0);
    _mp_audio::PlayDeathSound(killedPlayer, inflictor, attacker, weapon, mod);
    AudioOnPlayerDeath(killedPlayer, Broc::bint(mod));
    Broc::bint state;
    mp_util_wad::entity_get_playerState(&state, killedPlayer);
    if ((int)state != 2) {
        void* ftor = DeathState__functor(killedPlayer, team_killer, 3,
                                         (int)state == 4, true);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "DeathState", ftor);
    }
    Broc::string team;
    mp_util_wad::entity_get_team(&team, killedPlayer);
    CheckLastManStanding(team);
}

// CallbackRoundOver - ea: 0x941480
void CallbackRoundOver(int condition, Broc::string team) {
    Broc::Code_DebugOut("*COMMON* CallbackRoundOver\n");
    mp_util_wad::pLevel->roundOver = true;
    mp_util_wad::pLevel->roundStarted = false;
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    HashStr label;
    label.mVal = 0x863B4D44;
    Broc::notify(&lvl, label);
    Broc::dyn_array<Broc::entity> local_players;
    Broc::GetLocalPlayerArray(&local_players);
    Broc::bint i(0);
    while ((int)i < Broc::size(local_players)) {
        Broc::entity p = local_players[(unsigned int)(int)i];
        int idx = Broc::GetPlayerIndex(p);
        Broc::string menu("side_select");
        Broc::CloseMenu(&menu, idx);
        menu.~string();
        Broc::string menu2("weapon");
        Broc::CloseMenu(&menu2, idx);
        menu2.~string();
        i = (int)i + 1;
    }
    local_players.~dyn_array();
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint j(0);
    while ((int)j < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)j];
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, p);
        if ((int)state != 3) {
            Broc::bint now;
            Broc::GetTime(&now);
            Broc::bint timeAlive =
                (int)now - (int)*mp_util_wad::GetEE_spawnTime(p);
            if ((int)timeAlive > 1000)
                AddToPlayerStats(p, Broc::bint(0),
                                 (__int16)((int)timeAlive / 1000));
        }
        j = (int)j + 1;
    }
    PutAllPlayersIntoIntermission();
    Broc::bint waitTime(0);
    switch (condition) {
    case kEndRoundNone:
        mp_util_wad::pLevel->roundWinner = "none";
        break;
    case kEndRoundTimeLimit:
        if (team == "allies")
            Broc::SetTutorialTextAllPlayers((int)0x6E5C70B8u);
        else if (team == "axis")
            Broc::SetTutorialTextAllPlayers((int)0x8C1FDAB3);
        waitTime = 5;
        mp_util_wad::pLevel->roundWinner = team;
        break;
    case kEndRoundLastManStanding:
        if (team == "axis") {
            Broc::string t("axis");
            Broc::Code_IncTeamScore(&t, 1);
            t.~string();
        } else if (team == "allies") {
            Broc::string t("allies");
            Broc::Code_IncTeamScore(&t, 1);
            t.~string();
        }
        mp_util_wad::pLevel->roundWinner = team;
        break;
    case kEndRoundNoPlayers:
        if (Broc::Code_GetTeamGame())
            Broc::SetTutorialTextAllPlayers(team == "axis" ? (int)0xCEB8A0BD
                                                          : (int)0xF4472EC7);
        else
            Broc::SetTutorialTextAllPlayers((int)0x9F9E5421);
        mp_util_wad::pLevel->roundWinner = "none";
        mp_util_wad::pLevel->roundCount =
            (int)mp_util_wad::pLevel->roundCount - 1;
        if ((int)mp_util_wad::pLevel->roundCount < 0)
            mp_util_wad::pLevel->roundCount = 0;
        waitTime = 4;
        break;
    case kEndRoundScoreLimit:
        if (team == "allies")
            Broc::SetTutorialTextAllPlayers((int)0x6E5C70B8u);
        else if (team == "axis")
            Broc::SetTutorialTextAllPlayers((int)0x8C1FDAB3);
        mp_util_wad::pLevel->roundWinner = team;
        waitTime = 5;
        break;
    default:
        break;
    }
    if (mp_util_wad::pLevel->roundWinner == "allies" ||
        mp_util_wad::pLevel->roundWinner == "axis") {
        Broc::GetPlayerArray(&players);
        Broc::bint k(0);
        while ((int)k < Broc::size(players)) {
            Broc::entity p = players[(unsigned int)(int)k];
            Broc::string pteam;
            mp_util_wad::entity_get_team(&pteam, p);
            if (pteam == mp_util_wad::pLevel->roundWinner)
                Broc::Code_IncPlayerStat(p, 1, 1);
            else
                Broc::Code_IncPlayerStat(p, 2, 1);
            pteam.~string();
            k = (int)k + 1;
        }
    }
    players.~dyn_array();
    Broc::entity lvl2;
    lvl2.___u0 = mp_util_wad::pLevel != NULL;
    void* ftor = restart_round__functor(lvl2, (int)waitTime);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "restart_round", ftor);
    team.~string();
}

// CallbackFireArtillery - ea: 0x942200
void CallbackFireArtillery(Broc::entity firer, Broc::vector position) {
    if (Broc::Code_IsLocalPlayer(firer)) {
        Broc::SetActionHint((int)0xB1F2B60E, Broc::GetPlayerIndex(firer));
        __int16 rank = mp_util_wad::entity_get_rank(firer);
        Broc::string team;
        mp_util_wad::entity_get_team(&team, firer);
        const char* soundA = NULL;
        const char* soundB = NULL;
        if (rank == 0) {
            soundA = team == "axis" ? "Scout_SpecialFire_Axis"
                                    : "Scout_SpecialFire_Allies";
            soundB = team == "axis" ? "Scout_SpecialFire_Pvt_Axis"
                                    : "Scout_SpecialFire_Pvt_Allies";
        } else if (rank == 1) {
            soundA = team == "axis" ? "Scout_SpecialFire_Axis"
                                    : "Scout_SpecialFire_Allies";
            soundB = team == "axis" ? "Scout_SpecialFire_Crp_Axis"
                                    : "Scout_SpecialFire_Crp_Allies";
        } else if (rank == 2) {
            soundA = team == "axis" ? "Scout_SpecialFire_Axis"
                                    : "Scout_SpecialFire_Allies";
            soundB = team == "axis" ? "Scout_SpecialFire_Sgt_Axis"
                                    : "Scout_SpecialFire_Sgt_Allies";
        }
        if (soundA != NULL) {
            Broc::string s1(soundA);
            void* ftor1 = _mp_audio::PlaySound__functor(firer, s1, 0.0f);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "_mp_audio::PlaySound", ftor1);
            Broc::string s2(soundB);
            void* ftor2 = _mp_audio::PlaySound__functor(firer, s2, 1.5f);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "_mp_audio::PlaySound", ftor2);
        }
        team.~string();
    }
    Broc::dyn_array<Broc::entity> players;
    Broc::GetLocalPlayerArray(&players);
    Broc::bbool played_incoming_sound(false);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        Broc::string fteam;
        Broc::string pteam;
        mp_util_wad::entity_get_team(&fteam, firer);
        mp_util_wad::entity_get_team(&pteam, p);
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, p);
        if (pteam == fteam && (int)state == 3) {
            if (!(bool)played_incoming_sound) {
                Broc::entity lvl;
                lvl.___u0 = mp_util_wad::pLevel != NULL;
                Broc::string sound("incoming");
                void* ftor =
                    _mp_audio::PlaySoundAtLocation__functor(lvl, sound, position);
                Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                    __LINE__, "_mp_audio::PlaySoundAtLocation",
                                    ftor);
                played_incoming_sound = true;
            }
            Broc::entity lvl;
            lvl.___u0 = mp_util_wad::pLevel != NULL;
            void* ftor = AddArtilleryObjective__functor(lvl, position);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "AddArtilleryObjective", ftor);
        }
        fteam.~string();
        pteam.~string();
        i = (int)i + 1;
    }
    players.~dyn_array();
}

// StartRound - ea: 0x946110
void StartRound(Broc::bbool firstTime) {
    Broc::Code_DebugOut("*COMMON* StartRound\n");
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    HashStr label;
    label.mVal = 0xAFE7EFF4;
    Broc::notify(&lvl, label);
    Broc::Code_ForceControllerErrorMessageDown();
    mp_util_wad::pLevel->roundOver = false;
    mp_util_wad::pLevel->roundWinner = "";
    mp_util_wad::pLevel->forceMapChange = false;
    if ((bool)firstTime) {
        Broc::dyn_array<Broc::entity> players;
        Broc::GetLocalPlayerArray(&players);
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            Broc::entity p = players[(unsigned int)(int)i];
            Broc::string menu("spectate");
            Broc::OpenMenu(&menu, Broc::GetPlayerIndex(p));
            menu.~string();
            Broc::SetSpectateState(0, Broc::GetPlayerIndex(p));
            i = (int)i + 1;
        }
        players.~dyn_array();
    }
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint j(0);
    while ((int)j < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)j];
        *mp_util_wad::GetEE_spawnCount(p) = 0;
        *mp_util_wad::GetEE_punishedTeamKills(p) = 0;
        *mp_util_wad::GetEE_killsSinceLastDeath(p) = 0;
        put_player_into_spectate_mode(p);
        j = (int)j + 1;
    }
    players.~dyn_array();
    Broc::Code_ClearPlayerStats();
    Broc::Code_ClearTeamScores();
    Broc::entity lvl2;
    lvl2.___u0 = mp_util_wad::pLevel != NULL;
    void* ftor = finish_starting_round__functor(lvl2, (bool)firstTime);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "finish_starting_round", ftor);
}

// finish_starting_round - ea: 0x946630
void finish_starting_round(Broc::entity self, Broc::bbool firstTime) {
    (void)self;
    Broc::Code_DebugOut("*COMMON* finish_starting_round\n");
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    HashStr e1;
    e1.mVal = 0x863B4D44;
    Broc::endon(lvl, e1);
    HashStr e2;
    e2.mVal = 0x531AD8D9u;
    Broc::endon(lvl, e2);
    if (Broc::GetCvarInt("mp_debug") == 0 &&
        ((bool)mp_util_wad::pLevel->lastManStanding ||
         (bool)mp_util_wad::pLevel->mustHaveBothTeamsToStart)) {
        Broc::bbool both;
        HasBothTeams(&both);
        if (!(bool)both)
            WaitForTeams();
    }
    Broc::entity lvl2;
    lvl2.___u0 = mp_util_wad::pLevel != NULL;
    HashStr n;
    n.mVal = 0x4DEC2E76u;
    Broc::notify(&lvl2, n);
    Broc::wait(0.1f);
    Broc::entity lvl3;
    lvl3.___u0 = mp_util_wad::pLevel != NULL;
    Broc::endon(lvl3, n);
    Broc::bint t;
    Broc::GetTime(&t);
    mp_util_wad::pLevel->startTime = (int)t;
    if ((int)mp_util_wad::pLevel->roundCount >=
        (int)mp_util_wad::pLevel->roundLimit)
        mp_util_wad::pLevel->roundCount = 0;
    mp_util_wad::pLevel->roundCount =
        (int)mp_util_wad::pLevel->roundCount + 1;
    mp_util_wad::pLevel->roundStarted = true;
    CreateClock(Broc::bint((int)mp_util_wad::pLevel->timeLimit * 60));
    Broc::Code_DebugOut("*COMMON* Sending gamestate to make sure everyone is on the same page about the start round\n");
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    mp_util_wad::pLevel->playerCountAtStartOfRound = Broc::size(players);
    mp_util_wad::pLevel->playersLeavingDuringRound = 0;
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::string team("axis");
        Broc::string team2("allies");
        int axisScore = Broc::Code_GetTeamScore(&team);
        int alliesScore = Broc::Code_GetTeamScore(&team2);
        Broc::Code_SendGameState(
            players[(unsigned int)(int)i], 0.0f,
            (int)mp_util_wad::pLevel->timeLimit,
            (int)mp_util_wad::pLevel->scoreLimit,
            (int)mp_util_wad::pLevel->roundLimit,
            (bool)mp_util_wad::pLevel->friendlyFire,
            (bool)mp_util_wad::pLevel->lastManStanding,
            (bool)mp_util_wad::pLevel->teamBalance,
            (int)mp_util_wad::pLevel->respawnTime, alliesScore, axisScore,
            (bool)mp_util_wad::pLevel->roundStarted, 0,
            (int)mp_util_wad::pLevel->roundCount);
        team2.~string();
        team.~string();
        i = (int)i + 1;
    }
    players.~dyn_array();
    (void)firstTime;
}

// restart_round - ea: 0x946C00
void restart_round(Broc::entity selfLevel, Broc::bint waitTime) {
    (void)selfLevel;
    Broc::Code_ForceControllerErrorMessageDown();
    if ((int)waitTime < 1)
        waitTime = 1;
    Broc::wait((float)(int)waitTime);
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    HashStr fade;
    fade.mVal = 0x2A9ACF98u;
    Broc::waittill(lvl, fade);
    if (Broc::Code_GetTeamGame()) {
        Broc::MusicStop();
        Broc::wait(1.0f);
        if (mp_util_wad::pLevel->roundWinner.length() == 0) {
            Broc::string winner;
            GetWinningTeam(&winner);
            mp_util_wad::pLevel->roundWinner = winner;
            winner.~string();
        }
        Broc::string gametype;
        Broc::GetCvar(&gametype, "mp_gametype");
        bool isWar = gametype == "war";
        gametype.~string();
        if (isWar)
            Broc::wait(2.0f);
        if (mp_util_wad::pLevel->roundWinner == "allies") {
            Broc::string script("MX_MPVictory_Allies");
            Broc::entity lvl2;
            lvl2.___u0 = mp_util_wad::pLevel != NULL;
            mp_util_wad::pLevel->roundEndMusic =
                Broc::EffectEventPlay(&lvl2, &script);
            script.~string();
            Broc::string dialog("MP_GEN_AlliesWin");
            Broc::entity lvl3;
            lvl3.___u0 = mp_util_wad::pLevel != NULL;
            Broc::DialogPlay(lvl3, &dialog);
            dialog.~string();
        } else if (mp_util_wad::pLevel->roundWinner == "axis") {
            Broc::string script("MX_MPVictory_Axis");
            Broc::entity lvl2;
            lvl2.___u0 = mp_util_wad::pLevel != NULL;
            mp_util_wad::pLevel->roundEndMusic =
                Broc::EffectEventPlay(&lvl2, &script);
            script.~string();
            Broc::string dialog("MP_GEN_AxisWin");
            Broc::entity lvl3;
            lvl3.___u0 = mp_util_wad::pLevel != NULL;
            Broc::DialogPlay(lvl3, &dialog);
            dialog.~string();
        } else {
            // roundWinner == "none"
        }
    }
    Broc::bint now;
    Broc::GetTime(&now);
    mp_util_wad::pLevel->nextRoundStartTime = (int)now + 20000;
    if ((bool)mp_util_wad::pLevel->forceMapChange ||
        (int)mp_util_wad::pLevel->roundCount >=
            (int)mp_util_wad::pLevel->roundLimit) {
        Broc::dyn_array<Broc::entity> players;
        Broc::GetLocalPlayerArray(&players);
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            mp_util_wad::entity_set_nextPlayerClass(
                players[(unsigned int)(int)i], -1);
            i = (int)i + 1;
        }
        players.~dyn_array();
        Broc::Code_DisplayScoreBoard(true, 20);
        Broc::string name("aar_music");
        Broc::bint musicHandle((int)Broc::SoundPlay(name, 1.0f));
        name.~string();
        Broc::wait_accurate(20.0f);
        Broc::SoundStop((unsigned int)(int)musicHandle);
        Broc::Code_SettleMapVote();
        Broc::Code_SettleGameModeVote();
        Broc::Code_DisplayScoreBoard(false, 20);
        Broc::wait(0.1f);
        if (Broc::Code_NextRoundMapChanges()) {
            if (Broc::Code_GetTeamGame())
                _mp_teambalance::team_balance(true);
        } else {
            if (Broc::Code_GetTeamGame())
                _mp_teambalance::team_balance(false);
            Broc::SetTutorialTextAllPlayers((int)0x6422EAEDu);
        }
        Broc::Code_NextRound(true);
    } else {
        if (!(bool)mp_util_wad::pLevel->onlyDisplayScoreOnFinalRound) {
            Broc::Code_DisplayScoreBoard(true, 20);
            Broc::wait_accurate(20.0f);
            Broc::Code_SettleMapVote();
            Broc::Code_SettleGameModeVote();
            Broc::Code_DisplayScoreBoard(false, 20);
            Broc::wait(0.1f);
        }
        Broc::SetTutorialTextAllPlayers((int)0x6422EAEDu);
        Broc::Code_NextRound(false);
    }
}

// CheckLastManStanding - ea: 0x947AA0
void CheckLastManStanding(Broc::string team) {
    if ((bool)mp_util_wad::pLevel->lastManStanding &&
        !(bool)mp_util_wad::pLevel->roundOver &&
        (bool)mp_util_wad::pLevel->roundStarted &&
        !(mp_util_wad::pLevel->lastManStandingIgnore == team)) {
        Broc::dyn_array<Broc::entity> players;
        Broc::GetPlayerArray(&players);
        if (Broc::Code_GetTeamGame()) {
            Broc::bbool anyoneLeftTeam(false);
            Broc::bbool anyoneLeftOther(false);
            Broc::bint i(0);
            while ((int)i < Broc::size(players)) {
                Broc::entity p = players[(unsigned int)(int)i];
                Broc::bint state;
                mp_util_wad::entity_get_playerState(&state, p);
                if ((int)state == 3) {
                    Broc::string pteam;
                    mp_util_wad::entity_get_team(&pteam, p);
                    if (pteam == team)
                        anyoneLeftTeam = true;
                    else
                        anyoneLeftOther = true;
                    pteam.~string();
                }
                i = (int)i + 1;
            }
            Broc::string winner((const char*)NULL);
            if (!(bool)anyoneLeftTeam && (bool)anyoneLeftOther)
                winner = team == "allies" ? "axis" : "allies";
            if (!(bool)anyoneLeftOther || !(bool)anyoneLeftTeam)
                EndRound(kEndRoundLastManStanding, winner);
            winner.~string();
        } else {
            Broc::bint aliveCount(0);
            Broc::bint k(0);
            while ((int)k < Broc::size(players)) {
                Broc::entity p = players[(unsigned int)(int)k];
                Broc::bint state;
                mp_util_wad::entity_get_playerState(&state, p);
                if ((int)state == 3)
                    aliveCount = (int)aliveCount + 1;
                k = (int)k + 1;
            }
            if ((int)aliveCount <= 1) {
                Broc::string winner("none");
                EndRound(kEndRoundLastManStanding, winner);
            }
        }
        players.~dyn_array();
    }
    team.~string();
}

// CheckLastManStandingEnoughPlayers - ea: 0x947F40
void CheckLastManStandingEnoughPlayers(Broc::entity guy) {
    (void)guy;
    if ((bool)mp_util_wad::pLevel->lastManStanding &&
        !(bool)mp_util_wad::pLevel->roundOver &&
        (bool)mp_util_wad::pLevel->roundStarted) {
        Broc::entity lvl;
        lvl.___u0 = mp_util_wad::pLevel != NULL;
        HashStr n1;
        n1.mVal = 0xA8743279;
        Broc::notify(&lvl, n1);
        Broc::wait(0.1f);
        Broc::entity lvl2;
        lvl2.___u0 = mp_util_wad::pLevel != NULL;
        HashStr n2;
        n2.mVal = 0x4DEC2E76u;
        Broc::endon(lvl2, n2);
        Broc::entity lvl3;
        lvl3.___u0 = mp_util_wad::pLevel != NULL;
        Broc::endon(lvl3, n1);
        Broc::wait(30.0f);
        Broc::bint anyoneLeftAxis(0);
        Broc::bint anyoneLeftAllies(0);
        Broc::dyn_array<Broc::entity> players;
        Broc::GetPlayerArray(&players);
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            Broc::entity p = players[(unsigned int)(int)i];
            Broc::bint state;
            mp_util_wad::entity_get_playerState(&state, p);
            if ((int)state == 3) {
                Broc::string team;
                mp_util_wad::entity_get_team(&team, p);
                if (team == "axis")
                    anyoneLeftAxis = (int)anyoneLeftAxis + 1;
                else
                    anyoneLeftAllies = (int)anyoneLeftAllies + 1;
                team.~string();
            }
            i = (int)i + 1;
        }
        if (Broc::Code_GetTeamGame() &&
            ((int)anyoneLeftAxis == 0 || (int)anyoneLeftAllies == 0)) {
            Broc::string winner((int)anyoneLeftAxis != 0 ? "axis" : "allies");
            EndRound(kEndRoundNoPlayers, winner);
        } else if (!Broc::Code_GetTeamGame() &&
                   (int)anyoneLeftAllies + (int)anyoneLeftAxis <= 1) {
            Broc::string winner("none");
            EndRound(kEndRoundNoPlayers, winner);
        }
        players.~dyn_array();
    }
}

// PutAllPlayersIntoIntermission - ea: 0x94BBD0
void PutAllPlayersIntoIntermission() {
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        if (Broc::Code_IsLocalPlayer(p)) {
            Broc::Code_ScreenFadeToBlack(0xFAu, -1);
            void* ftor = LocalPlayerIntermission__functor(p);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "LocalPlayerIntermission", ftor);
        } else {
            SpawnIntermission(p);
        }
        i = (int)i + 1;
    }
    players.~dyn_array();
}

// LocalPlayerIntermission - ea: 0x945BC0
void LocalPlayerIntermission(Broc::entity player) {
    HashStr e1;
    e1.mVal = 0xC1E6FED9;
    Broc::endon(player, e1);
    HashStr e2;
    e2.mVal = 0x86018C9u;
    Broc::endon(player, e2);
    HashStr n;
    n.mVal = 0x24B5BA64u;
    Broc::notify(&player, n);
    *mp_util_wad::GetEE_specialWeaponTime(player) = 0;
    Broc::wait(0.25f);
    SpawnIntermission(player);
    Broc::wait(0.1f);
    Broc::entity lvl;
    lvl.___u0 = mp_util_wad::pLevel != NULL;
    void* ftor = FadeUpWhenLoaded__functor(lvl, player);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "FadeUpWhenLoaded", ftor);
}

// LocalPlayerRespawn - ea: 0x945D30
void LocalPlayerRespawn(Broc::entity player) {
    Broc::string menu("spectate");
    Broc::CloseMenu(&menu, Broc::GetPlayerIndex(player));
    menu.~string();
    if (mp_util_wad::entity_get_nextPlayerClass(player) == -1) {
        Broc::TakeAllWeapons(&player);
        Broc::string side("side_select");
        Broc::OpenMenu(&side, Broc::GetPlayerIndex(player));
        side.~string();
        HashStr w1;
        w1.mVal = 0xE7D05EDA;
        Broc::waittill(player, w1);
        Broc::string weapon("weapon");
        Broc::OpenMenu(&weapon, Broc::GetPlayerIndex(player));
        weapon.~string();
        HashStr w2;
        w2.mVal = 0x70FACFE9u;
        Broc::waittill(player, w2);
        mp_util_wad::entity_set_playerClass(
            player, mp_util_wad::entity_get_nextPlayerClass(player));
        *mp_util_wad::GetEE_specialWeaponTime(player) = 0;
    }
    Broc::Code_ScreenFadeToBlack(0xFAu, Broc::GetPlayerIndex(player));
    Broc::wait(0.25f);
    Broc::Code_RequestRespawn(Broc::GetPlayerIndex(player));
}

// FollowClient - ea: 0x9459F0
void FollowClient(Broc::entity player, Broc::bint newClient) {
    Broc::bbool defined;
    mp_util_wad::IsEEDefined_isInsideFollowClient(&defined, player);
    if (!(bool)defined) {
        *mp_util_wad::GetEE_isInsideFollowClient(player) = 1;
        if (Broc::Code_IsLocalPlayer(player)) {
            Broc::bint state;
            mp_util_wad::entity_get_playerState(&state, player);
            if ((int)state == 1) {
                Broc::Code_ScreenFadeToBlack(0xFAu, Broc::GetPlayerIndex(player));
                Broc::wait(1.0f);
                mp_util_wad::entity_set_spectatorClient(player, (int)newClient);
                Broc::wait(1.0f);
                Broc::entity lvl;
                lvl.___u0 = mp_util_wad::pLevel != NULL;
                FadeUpWhenLoaded(lvl, player);
            }
        }
        *mp_util_wad::GetEE_isInsideFollowClient(player) = 0;
    }
}

// GetRespawnTime - ea: 0x949620
Broc::bint* GetRespawnTime(Broc::bint* result, Broc::entity localPlayer) {
    Broc::bint respawnTime;
    Broc::GetTime(&respawnTime);
    if (Broc::GetCvarInt("mp_debug") != 0) {
        result->mVal = (int)respawnTime + 2000;
        return result;
    }
    if (mp_util_wad::pLevel->teamCantRespawn.length() != 0) {
        Broc::bint deathPenalty(0);
        Broc::string team;
        mp_util_wad::entity_get_team(&team, localPlayer);
        if (team == mp_util_wad::pLevel->teamCantRespawn) {
            deathPenalty = 5000 * (int)*mp_util_wad::GetEE_numDeaths(localPlayer);
            if ((int)deathPenalty >
                20000 - (int)mp_util_wad::pLevel->respawnTime * 1000)
                deathPenalty =
                    20000 - (int)mp_util_wad::pLevel->respawnTime * 1000;
        }
        team.~string();
        respawnTime = (int)respawnTime +
                      (int)mp_util_wad::pLevel->respawnTime * 1000 +
                      (int)deathPenalty;
        if (mp_util_wad::pLevel->teamCantRespawn == "reset")
            mp_util_wad::pLevel->teamCantRespawn = "";
    } else if ((int)mp_util_wad::pLevel->spawnType == 1 &&
               (int)mp_util_wad::pLevel->respawnTime > 0) {
        Broc::bint graceTime =
            (int)mp_util_wad::pLevel->startTime + 5000;
        Broc::bint now;
        Broc::GetTime(&now);
        if ((int)graceTime < (int)now) {
            Broc::bint t;
            Broc::GetTime(&t);
            Broc::bint waveNumber =
                (int)t / (1000 * (int)mp_util_wad::pLevel->respawnTime);
            respawnTime = ((int)waveNumber + 1) * 1000 *
                          (int)mp_util_wad::pLevel->respawnTime;
        }
    } else {
        if ((int)*mp_util_wad::GetEE_spawnCount(localPlayer) != 0)
            respawnTime =
                (int)respawnTime + (int)mp_util_wad::pLevel->respawnTime * 1000;
    }
    result->mVal = (int)respawnTime;
    return result;
}

// UpdateSpectateCritical - ea: 0x949BB0
void UpdateSpectateCritical(Broc::entity guy) {
    Broc::Code_DebugOut("*COMMON* UpdateSpectateCritical\n");
    Broc::wait(0.1f);
    static const unsigned int labels[] = {
        0x4C745BCAu, 0xC1E6FED9, 0x24B5BA64u, 0x86018C9u, 0x11524591u,
        0xC39A4465
    };
    for (int i = 0; i < 6; i++) {
        HashStr label;
        label.mVal = labels[i];
        Broc::endon(guy, label);
    }
    bool is_local_player = Broc::Code_IsLocalPlayer(guy);
    int playerIndex = Broc::GetPlayerIndex(guy);
    if (is_local_player)
        Broc::SetSpectateState(2, playerIndex);
    Broc::bint respawnTime;
    GetRespawnTime(&respawnTime, guy);
    for (;;) {
        Broc::bint now;
        Broc::GetTime(&now);
        if ((int)respawnTime < (int)now)
            break;
        if (is_local_player) {
            Broc::bint t;
            Broc::GetTime(&t);
            int seconds = ((int)respawnTime - (int)t + 999) / 1000;
            Broc::SetSpectateSeconds(seconds, playerIndex);
        }
        waitframe();
    }
    void* ftor = UpdateSpectateCriticalGoingToDie__functor(guy);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "UpdateSpectateCriticalGoingToDie", ftor);
}

// UpdateSpectateCriticalGoingToDie - ea: 0x949F80
void UpdateSpectateCriticalGoingToDie(Broc::entity guy) {
    Broc::wait(0.1f);
    static const unsigned int labels[] = {
        0x4C745BCAu, 0xC1E6FED9, 0x24B5BA64u, 0x86018C9u, 0x11524591u,
        0xC39A4465
    };
    for (int i = 0; i < 6; i++) {
        HashStr label;
        label.mVal = labels[i];
        Broc::endon(guy, label);
    }
    bool is_local_player = Broc::Code_IsLocalPlayer(guy);
    int playerIndex = Broc::GetPlayerIndex(guy);
    if (is_local_player)
        Broc::SetSpectateState(3, playerIndex);
    Broc::bint respawnTime;
    GetGoingToDieTime(&respawnTime);
    Broc::bint maxTime((int)respawnTime - 1000);
    if ((int)maxTime <= 0)
        maxTime = (int)respawnTime;
    Broc::Code_SetRespawnMaxTime(guy, (int)maxTime);
    for (;;) {
        Broc::bint now;
        Broc::GetTime(&now);
        if ((int)respawnTime < (int)now)
            break;
        if (is_local_player) {
            Broc::bint t;
            Broc::GetTime(&t);
            int seconds = ((int)respawnTime - (int)t + 999) / 1000;
            Broc::SetSpectateSeconds(seconds, playerIndex);
        }
        waitframe();
    }
    void* ftor = UpdateSpectateDead__functor(guy, true);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "UpdateSpectateDead", ftor);
}

// UpdateSpectateDead - ea: 0x94A2D0
void UpdateSpectateDead(Broc::entity guy, Broc::bbool canspawn) {
    HashStr n;
    n.mVal = 0x6A7DEF26u;
    Broc::notify(&guy, n);
    Broc::wait(0.1f);
    static const unsigned int labels[] = {
        0x4C745BCAu, 0xC1E6FED9, 0x24B5BA64u, 0x86018C9u, 0x11524591u,
        0xC39A4465
    };
    for (int i = 0; i < 6; i++) {
        HashStr label;
        label.mVal = labels[i];
        Broc::endon(guy, label);
    }
    Broc::bint state;
    mp_util_wad::entity_get_playerState(&state, guy);
    if ((int)state != 5) {
        AddToPlayerStats(guy, Broc::bint(4), 1);
        void* ftor = _mp_audio::player_dying_sounds__functor(guy);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "_mp_audio::player_dying_sounds", ftor);
    }
    mp_util_wad::entity_set_playerState(guy, 5);
    if (Broc::Code_IsLocalPlayer(guy)) {
        int playerIndex = Broc::GetPlayerIndex(guy);
        Broc::SetSpectateState(((bool)canspawn ? 1 : 0) + 4, playerIndex);
        Broc::bint respawnTime;
        GetRespawnTime(&respawnTime, guy);
        for (;;) {
            Broc::bint now;
            Broc::GetTime(&now);
            Broc::string menu("spectate");
            bool done = (int)respawnTime < (int)now &&
                        Broc::Code_IsMenuOpen(&menu, playerIndex) != 0;
            menu.~string();
            if (done)
                break;
            Broc::bint t;
            Broc::GetTime(&t);
            int seconds = ((int)respawnTime - (int)t + 999) / 1000;
            Broc::SetSpectateSeconds(seconds, playerIndex);
            Broc::string empty("");
            if (Broc::Code_IsMenuOpen(&empty, playerIndex) != 0) {
                Broc::string spec("spectate");
                Broc::OpenMenu(&spec, playerIndex);
                spec.~string();
            }
            empty.~string();
            waitframe();
        }
        if ((bool)canspawn) {
            void* ftor = LocalPlayerRespawn__functor(guy);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "LocalPlayerRespawn", ftor);
        } else {
            UpdateSpectateDead(guy, Broc::bbool(true));
        }
    }
}

// UpdateSpectateSpawn - ea: 0x94A8D0
void UpdateSpectateSpawn(Broc::entity localPlayer) {
    Broc::Code_DebugOut("*COMMON* UpdateSpectateSpawn\n");
    Broc::wait(0.1f);
    static const unsigned int labels[] = {
        0x4C745BCAu, 0xC1E6FED9, 0x24B5BA64u, 0x86018C9u, 0x11524591u,
        0xC39A4465
    };
    for (int i = 0; i < 6; i++) {
        HashStr label;
        label.mVal = labels[i];
        Broc::endon(localPlayer, label);
    }
    if (Broc::Code_IsLocalPlayer(localPlayer))
        Broc::SetSpectateState(1, Broc::GetPlayerIndex(localPlayer));
}

// DeathState - ea: 0x945420
void DeathState(Broc::entity player, Broc::entity team_killer, Broc::bint delay,
                Broc::bbool reviveable, Broc::bbool fade) {
    (void)fade;
    static const unsigned int labels[] = {
        0x24B5BA64u, 0x4C745BCAu, 0x11524591u, 0x86018C9u
    };
    for (int i = 0; i < 4; i++) {
        HashStr label;
        label.mVal = labels[i];
        Broc::endon(player, label);
    }
    bool is_local_player = Broc::Code_IsLocalPlayer(player);
    if (is_local_player) {
        Broc::string menu("spectate");
        Broc::OpenMenu(&menu, Broc::GetPlayerIndex(player));
        menu.~string();
        Broc::SetSpectateState(0, Broc::GetPlayerIndex(player));
    }
    Broc::wait((float)(int)delay);
    Broc::bint state;
    mp_util_wad::entity_get_playerState(&state, player);
    if ((int)state != 2) {
        HashStr n;
        n.mVal = 0xC1E6FED9;
        Broc::notify(&player, n);
        mp_util_wad::entity_set_spectatorClient(player, -1);
        mp_util_wad::entity_set_health(player, 0);
        if (Broc::IsDefined(team_killer))
            Broc::SetSpectateTeamKill(1, &team_killer,
                                      Broc::GetPlayerIndex(player));
        if ((bool)reviveable) {
            if (is_local_player)
                Broc::SetSpectateMedic(1, Broc::GetPlayerIndex(player));
            void* ftor = UpdateSpectateCritical__functor(player);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "UpdateSpectateCritical", ftor);
        } else {
            if (is_local_player)
                Broc::SetSpectateMedic(0, Broc::GetPlayerIndex(player));
            void* ftor = UpdateSpectateDead__functor(player, false);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "UpdateSpectateDead", ftor);
        }
    }
}

// Spectate - ea: 0x945350
void Spectate(Broc::entity player, Broc::bint target, Broc::bbool isIntermission) {
    (void)isIntermission;
    HashStr e1;
    e1.mVal = 0x24B5BA64u;
    Broc::endon(player, e1);
    HashStr e2;
    e2.mVal = 0x4C745BCAu;
    Broc::endon(player, e2);
    Broc::wait((float)(int)target);
    Broc::bint state;
    mp_util_wad::entity_get_playerState(&state, player);
    if ((int)state != 2)
        put_player_into_spectate_mode(player);
}

// RespawnPlayer - ea: 0x9450D0
void RespawnPlayer(Broc::entity guy, Broc::string team) {
    HashStr e1;
    e1.mVal = 0x4C745BCAu;
    Broc::endon(guy, e1);
    Broc::string new_team = team;
    if ((bool)mp_util_wad::pLevel->teamBalance && !Broc::Code_GetTeamGame()) {
        Broc::string result;
        new_team = *_mp_teambalance::team_balance(&result, guy, team);
        result.~string();
    }
    Broc::entity spawnpoint;
    spawnpoint.___u0 = 0;
    ((Broc::entity* (*)(Broc::entity*, Broc::entity*, Broc::string*))
         mp_util_wad::pLevel->PickSpawnPoint)(&spawnpoint, &guy, &new_team);
    if (!Broc::IsDefined(spawnpoint) &&
        Broc::gBrocAPI.mAssert("c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "No spawn points in the level"))
        __debugbreak();
    Broc::vector origin;
    Broc::vector angles;
    mp_util_wad::entity_get_angles(&angles, spawnpoint);
    mp_util_wad::entity_get_origin(&origin, spawnpoint);
    Broc::Code_PlayerRespawn(guy, &origin, &angles, &new_team);
    new_team.~string();
    team.~string();
}

// put_player_into_spectate_mode - ea: 0x948A30
void put_player_into_spectate_mode(Broc::entity guy) {
    if (Broc::Code_IsLocalPlayer(guy)) {
        void* ftor = SpawnLocalSpectator__functor(guy);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "SpawnLocalSpectator", ftor);
    } else {
        SpawnSpectator(guy);
    }
}

// SpawnLocalSpectator - ea: 0x948B70
void SpawnLocalSpectator(Broc::entity guy) {
    Broc::Code_DebugOut("*COMMON* SpawnLocalSpectator\n");
    if (Broc::Code_IsLocalPlayer(guy)) {
        int local_player_index = Broc::GetPlayerIndex(guy);
        static const unsigned int labels[] = {
            0x86018C9u, 0x4C745BCAu, 0x24B5BA64u, 0x863B4D44
        };
        for (int i = 0; i < 4; i++) {
            HashStr label;
            label.mVal = labels[i];
            Broc::endon(guy, label);
        }
        HashStr n;
        n.mVal = 0xC1E6FED9;
        Broc::notify(&guy, n);
        bool firstSpectate = (bool)*mp_util_wad::GetEE_firstSpectate(guy);
        if (!firstSpectate) {
            Broc::Code_ScreenFadeToBlack(0xFAu, local_player_index);
            Broc::wait(0.25f);
        }
        SpawnSpectator(guy);
        if (Broc::Code_IsLocalPlayer(guy)) {
            Broc::wait(0.1f);
            if (!firstSpectate) {
                Broc::entity lvl;
                lvl.___u0 = mp_util_wad::pLevel != NULL;
                HashStr fade;
                fade.mVal = 0x2A9ACF98u;
                Broc::waittill(lvl, fade);
                Broc::Code_ScreenFadeUp(0xFAu, local_player_index);
            }
            Broc::string menu("spectate");
            Broc::OpenMenu(&menu, local_player_index);
            menu.~string();
            void* ftor = UpdateSpectateSpawn__functor(guy);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                __LINE__, "UpdateSpectateSpawn", ftor);
        }
    }
}

// SpawnSpectator - ea: 0x948FC0
void SpawnSpectator(Broc::entity guy) {
    Broc::Code_DebugOut("*COMMON* SpawnSpectator\n");
    mp_util_wad::entity_set_playerState(guy, 1);
    mp_util_wad::entity_set_spectatorClient(guy, -1);
    mp_util_wad::entity_set_health(guy, 0);
    Broc::vector origin;
    Broc::vector angles;
    if ((bool)*mp_util_wad::GetEE_firstSpectate(guy)) {
        mp_util_wad::entity_get_origin(&origin, guy);
        mp_util_wad::entity_get_viewangles(&angles, guy);
        *mp_util_wad::GetEE_firstSpectate(guy) = false;
    } else {
        Broc::entity spawnpoint;
        GetBestSpectateSpawn(&spawnpoint, &guy);
        if (Broc::IsDefined(spawnpoint)) {
            mp_util_wad::entity_get_origin(&origin, spawnpoint);
            mp_util_wad::entity_get_angles(&angles, spawnpoint);
        }
    }
    Broc::Code_PlayerSpawn(guy, &origin, &angles, 0);
}

// SpawnIntermission - ea: 0x949200
void SpawnIntermission(Broc::entity player) {
    Broc::Code_DebugOut("*COMMON* SpawnIntermission\n");
    HashStr n;
    n.mVal = 0x24B5BA64u;
    Broc::notify(&player, n);
    mp_util_wad::entity_set_playerState(player, 2);
    mp_util_wad::entity_set_spectatorClient(player, -1);
    mp_util_wad::entity_set_health(player, 0);
    Broc::dyn_array<Broc::entity> spawnpoints;
    Broc::string val("spawn_intermission");
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&val, key.mVal, &spawnpoints, 0);
    val.~string();
    Broc::entity spawnpoint;
    spawnpoint.___u0 = 0;
    _mp_spawnlogic::GetSpawnpointRandom(&spawnpoint, &spawnpoints, true);
    Broc::vector origin;
    Broc::vector angles;
    int stopPhysics = 0;
    if (Broc::IsDefined(spawnpoint)) {
        mp_util_wad::entity_get_angles(&angles, spawnpoint);
        mp_util_wad::entity_get_origin(&origin, spawnpoint);
        stopPhysics = 0;
    } else {
        origin = Broc::vector(0.0f, 0.0f, 0.0f);
        angles = Broc::vector(0.0f, 0.0f, 0.0f);
        stopPhysics = 0;
    }
    Broc::Code_PlayerSpawn(player, &origin, &angles, stopPhysics);
    if (Broc::Code_IsLocalPlayer(player)) {
        Broc::entity lvl;
        lvl.___u0 = mp_util_wad::pLevel != NULL;
        void* ftor = FadeUpWhenLoaded__functor(lvl, player);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "FadeUpWhenLoaded", ftor);
        Broc::string menu("spectate");
        Broc::OpenMenu(&menu, Broc::GetPlayerIndex(player));
        menu.~string();
        Broc::SetSpectateState(0, Broc::GetPlayerIndex(player));
    }
    spawnpoints.~dyn_array();
}

// GetBestSpectateSpawn - ea: 0x94B4A0
Broc::entity* GetBestSpectateSpawn(Broc::entity* result, Broc::entity* self) {
    Broc::dyn_array<Broc::entity> spawnpoints;
    Broc::string val("spawn_intermission");
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&val, key.mVal, &spawnpoints, 0);
    val.~string();
    Broc::string team;
    mp_util_wad::entity_get_team(&team, *self);
    Broc::entity whereYouWouldSpawn;
    whereYouWouldSpawn.___u0 = 0;
    GetSpawnPoint(&whereYouWouldSpawn, self, &team);
    team.~string();
    if (Broc::IsDefined(whereYouWouldSpawn)) {
        Broc::vector origin;
        mp_util_wad::entity_get_origin(&origin, whereYouWouldSpawn);
        _mp_spawnlogic::GetSpawnpointNearest(result, &spawnpoints, origin,
                                             true);
    } else {
        _mp_spawnlogic::GetSpawnpointRandom(result, &spawnpoints, true);
    }
    spawnpoints.~dyn_array();
    return result;
}

// HealthRegenPlayerBreathing - ea: 0x94B6F0
void HealthRegenPlayerBreathing(Broc::entity ent, Broc::bint healthCap) {
    Broc::endon(ent, "death");
    Broc::notify(&ent, "end_HealthRegenPlayerBreathing");
    Broc::wait(2.0f);
    Broc::endon(ent, "end_HealthRegenPlayerBreathing");
    for (;;) {
        Broc::wait(0.2f);
        Broc::bint health;
        mp_util_wad::entity_get_health(&health, ent);
        if ((int)health <= 0)
            break;
        if ((int)health < (int)healthCap) {
            Broc::string script("breathing_hurt");
            Broc::EffectEventPlay(&ent, &script);
            script.~string();
            Broc::wait(0.784f);
            Broc::wait(Broc::RandomFloat(0.8f) + 0.1f);
        }
    }
}

// AddArtilleryObjective - ea: 0x94B900
void AddArtilleryObjective(Broc::entity self, Broc::vector position) {
    (void)self;
    Broc::bint index(10 + (int)mp_util_wad::pLevel->ArtilleryObjectiveIndex);
    mp_util_wad::pLevel->ArtilleryObjectiveIndex =
        ((int)mp_util_wad::pLevel->ArtilleryObjectiveIndex + 1) % 4;
    Broc::string pszString("artillery");
    Broc::string state("i_incoming_artillery_c");
    Broc::ObjectiveAdd((int)index, &state, &pszString, position, "1");
    state.~string();
    pszString.~string();
    Broc::wait(15.0f);
    Broc::ObjectiveDelete((int)index, -1);
}

// HasValidWeaponInSlot - ea: 0x94BE20
Broc::bbool* HasValidWeaponInSlot(Broc::bbool* result, Broc::entity player,
                                  Broc::string slot) {
    int ammo = Broc::GetWeaponSlotAmmo(player, &slot);
    int clip = Broc::GetWeaponSlotClipAmmo(player, &slot);
    *result = Broc::bbool(ammo != 0 || clip != 0);
    slot.~string();
    return result;
}

// SelectFirstAvailableWeapon - ea: 0x94BF80
void SelectFirstAvailableWeapon(Broc::entity player) {
    Broc::string weapon((const char*)NULL);
    if (Broc::Code_IsLocalPlayer(player)) {
        Broc::string slot_primary("primary");
        Broc::string slot_primaryb("primaryb");
        Broc::string* slots[2];
        slots[0] = &slot_primary;
        slots[1] = &slot_primaryb;
        for (int i = 0; i < 2; i++) {
            Broc::bbool valid;
            HasValidWeaponInSlot(&valid, player, *slots[i]);
            if ((bool)valid) {
                Broc::GetWeaponSlotWeapon(player, slots[i], &weapon);
                Broc::SwitchToWeapon(&player, &weapon);
                weapon.~string();
                return;
            }
        }
    }
    weapon.~string();
}

// HostHasMigrated - ea: 0x944A00
void HostHasMigrated(Broc::entity self) {
    Broc::SetTutorialTextAllPlayers((int)0x35E035D8u);
    Broc::wait(4.0f);
    if (Broc::Code_IsHost()) {
        void* ftor = NewHost__functor(self);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "NewHost", ftor);
    }
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint now;
    Broc::GetTime(&now);
    Broc::bfloat timePassed =
        ((int)now - (int)mp_util_wad::pLevel->startTime) * 0.001f;
    Broc::bint timeTillNextRound;
    GetTimeTillNextRound(&timeTillNextRound);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::string team("axis");
        Broc::string team2("allies");
        int axisScore = Broc::Code_GetTeamScore(&team);
        int alliesScore = Broc::Code_GetTeamScore(&team2);
        Broc::Code_SendGameState(
            players[(unsigned int)(int)i], (float)timePassed,
            (int)mp_util_wad::pLevel->timeLimit,
            (int)mp_util_wad::pLevel->scoreLimit,
            (int)mp_util_wad::pLevel->roundLimit,
            (bool)mp_util_wad::pLevel->friendlyFire,
            (bool)mp_util_wad::pLevel->lastManStanding,
            (bool)mp_util_wad::pLevel->teamBalance,
            (int)mp_util_wad::pLevel->respawnTime, alliesScore, axisScore,
            (bool)mp_util_wad::pLevel->roundStarted, (int)timeTillNextRound,
            (int)mp_util_wad::pLevel->roundCount);
        team2.~string();
        team.~string();
        i = (int)i + 1;
    }
    players.~dyn_array();
    if ((bool)mp_util_wad::pLevel->roundOver) {
        Broc::string winner;
        GetWinningTeam(&winner);
        Broc::Code_RoundOver(0, &winner);
        winner.~string();
    } else {
        if ((float)timePassed >=
            (int)mp_util_wad::pLevel->timeLimit * 60) {
            Broc::string winner;
            GetWinningTeam(&winner);
            EndRound(kEndRoundTimeLimit, winner);
        }
    }
}

// GetSpawnPoint - ea: 0x94B370
Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* ent,
                            const Broc::string* spawnpoint) {
    Broc::dyn_array<Broc::entity> spawnpoints;
    Broc::string spawnType = mp_util_wad::pLevel->spawnTypeAllies;
    if (*spawnpoint == "axis")
        spawnType = mp_util_wad::pLevel->spawnTypeAxis;
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&spawnType, key.mVal, &spawnpoints, 0);
    Broc::entity sp;
    sp.___u0 = 0;
    _mp_spawnlogic::GetSpawnpointNearTeamAntiCamp(&sp, ent, spawnpoint,
                                                  &spawnpoints);
    *result = sp;
    spawnType.~string();
    spawnpoints.~dyn_array();
    return result;
}

void launch_gametype_thread(Broc::string& gametype, const char* func,
                            void* (*functor)(Broc::entity)) {
    (void)gametype;
    Broc::entity lvl = Broc::entity();
    void* ftor = functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, func, ftor);
}

void DebugRenderSpawnPoints() {}

// StopFollowing - ea: 0x94AA60 (no-op body in the release binary)
void StopFollowing(Broc::entity self, Broc::bbool blackNow) {
    (void)self;
    (void)blackNow;
}
}

// ============================================================================
// Sibling gametype stubs (real implementations arrive with each script port).
// ============================================================================
namespace _mp_tankdrive { void main() {} }
namespace _mp_nano { void main() {} }
namespace _mp_audio { void main() {} }
namespace _mp_audio {
void* PlayPainSound__functor(Broc::entity guy, Broc::bint damage) {
    (void)guy;
    (void)damage;
    return NULL;
}
void* audio_crossfade_wait__functor(Broc::entity self) {
    (void)self;
    return NULL;
}
void* ThreadStaticSoundPlay__functor(Broc::entity self, Broc::string name) {
    (void)self;
    name.~string();
    return NULL;
}
void* ThreadStaticSoundRandomPlay__functor(Broc::entity self,
                                           Broc::string name) {
    (void)self;
    name.~string();
    return NULL;
}
void* MoveSoundAlongLine__functor(Broc::entity toMove, Broc::vector start,
                                  Broc::vector end) {
    (void)toMove; (void)start; (void)end;
    return NULL;
}
void* PlaySound__functor(Broc::entity self, Broc::string sound, float delay) {
    (void)self;
    (void)delay;
    sound.~string();
    return NULL;
}
void* PlaySoundAtLocation__functor(Broc::entity self, Broc::string sound,
                                   Broc::vector position) {
    (void)self;
    (void)position;
    sound.~string();
    return NULL;
}
void* player_dying_sounds__functor(Broc::entity player) {
    (void)player;
    return NULL;
}
void PlayDeathSound(Broc::entity guy, Broc::entity inflictor,
                    Broc::entity attacker, Broc::bint weapon,
                    Broc::bint means_of_damage) {
    (void)guy; (void)inflictor; (void)attacker; (void)weapon;
    (void)means_of_damage;
}
}
namespace _mp_loadout {
void local_player_joined(Broc::entity player) { (void)player; }
void GiveLoadout(Broc::entity player) { (void)player; }
void GiveSpecialWeapon(Broc::entity player, int playerClass, int rank,
                       bool isRespawn) {
    (void)player; (void)playerClass; (void)rank; (void)isRespawn;
}
void UpdatePlayerModelForRank(Broc::entity player) { (void)player; }
void DisplayYouWillSpawnWithMessage(Broc::entity self) { (void)self; }
}
namespace _mp_shellshock {
void ShellshockOnDamage(Broc::entity self, Broc::bint cause, Broc::bint damage) {
    (void)self; (void)cause; (void)damage;
}
}
namespace _mp_spawnlogic {
Broc::entity* GetSpawnpointNearTeamAntiCamp(Broc::entity* result,
                                            Broc::entity* self,
                                            const Broc::string* team,
                                            Broc::dyn_array<Broc::entity>* points) {
    (void)self; (void)team; (void)points;
    result->___u0 = 0;
    return result;
}
Broc::entity* GetSpawnpointNearest(Broc::entity* result,
                                   Broc::dyn_array<Broc::entity>* points,
                                   Broc::vector position, bool ignoreTeleFrag) {
    (void)points; (void)position; (void)ignoreTeleFrag;
    result->___u0 = 0;
    return result;
}
Broc::entity* GetSpawnpointRandom(Broc::entity* result,
                                  Broc::dyn_array<Broc::entity>* points,
                                  bool ignoreTeleFrag) {
    (void)points; (void)ignoreTeleFrag;
    result->___u0 = 0;
    return result;
}
}
namespace _mp_teambalance {
Broc::string* team_balance(Broc::string* result, Broc::entity guy,
                           Broc::string team) {
    (void)guy;
    *result = team;
    team.~string();
    return result;
}
void team_balance(bool always) { (void)always; }
}
namespace _mp_common {
void* StopFollowing__functor(Broc::entity self, bool blackNow) {
    (void)self; (void)blackNow; return NULL;
}
void* QuitGameThread__functor(Broc::entity selfLevel) {
    (void)selfLevel; return NULL;
}
void* QuitGameWithMessage__functor(Broc::entity self, HashStr message) {
    (void)self; (void)message; return NULL;
}
void* HostHasMigrated__functor(Broc::entity self) {
    (void)self; return NULL;
}
void* RespawnPlayer__functor(Broc::entity guy, Broc::string team) {
    (void)guy;
    team.~string();
    return NULL;
}
void* LocalPlayerRespawn__functor(Broc::entity player) {
    (void)player; return NULL;
}
void* PunishedForTeamKill__functor(Broc::entity ent, bool punished) {
    (void)ent; (void)punished; return NULL;
}
void* reenable_medic_call__functor(Broc::entity self, int time) {
    (void)self; (void)time; return NULL;
}
void* HandleJoinAfterRoundOver__functor(Broc::entity self, int timeleft) {
    (void)self; (void)timeleft; return NULL;
}
void* TeamChangeKillPlayer__functor(Broc::entity player) {
    (void)player; return NULL;
}
void* FadeUpWhenLoaded__functor(Broc::entity self, Broc::entity player) {
    (void)self; (void)player; return NULL;
}
void* HealthRegenPlayerBreathing__functor(Broc::entity self, int healthCap) {
    (void)self; (void)healthCap; return NULL;
}
void* DeathState__functor(Broc::entity player, Broc::entity team_killer,
                          int delay, bool reviveable, bool fade) {
    (void)player; (void)team_killer; (void)delay; (void)reviveable; (void)fade;
    return NULL;
}
void* UpdateSpectateCritical__functor(Broc::entity guy) {
    (void)guy; return NULL;
}
void* UpdateSpectateCriticalGoingToDie__functor(Broc::entity guy) {
    (void)guy; return NULL;
}
void* UpdateSpectateDead__functor(Broc::entity guy, bool canspawn) {
    (void)guy; (void)canspawn; return NULL;
}
void* UpdateSpectateSpawn__functor(Broc::entity localPlayer) {
    (void)localPlayer; return NULL;
}
void* SpawnLocalSpectator__functor(Broc::entity guy) {
    (void)guy; return NULL;
}
void* restart_round__functor(Broc::entity selfLevel, int waitTime) {
    (void)selfLevel; (void)waitTime; return NULL;
}
void* finish_starting_round__functor(Broc::entity self, bool firstTime) {
    (void)self; (void)firstTime; return NULL;
}
void* AddArtilleryObjective__functor(Broc::entity self, Broc::vector position) {
    (void)self; (void)position; return NULL;
}
void* NewHost__functor(Broc::entity self) {
    (void)self; return NULL;
}
void* LocalPlayerIntermission__functor(Broc::entity player) {
    (void)player; return NULL;
}
}
namespace _mp_minefield {
void* main__functor(Broc::entity self) { (void)self; return NULL; }
}
namespace _mp_tdm {
void* main__functor(Broc::entity self) { (void)self; return NULL; }
}
namespace _mp_ctf {
void* main__functor(Broc::entity self) { (void)self; return NULL; }
}
namespace _mp_scf {
void* main__functor(Broc::entity self) { (void)self; return NULL; }
}
namespace _mp_war {
void* main__functor(Broc::entity self) { (void)self; return NULL; }
}
namespace _mp_hq {
void* main__functor(Broc::entity self) { (void)self; return NULL; }
}
