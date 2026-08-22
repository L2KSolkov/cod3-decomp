// ============================================================================
// mp_util_wad.cpp - multiplayer Broc entity-attribute accessors + anim wrappers.
// Source: mp_util_wad.cpp (MPBrocCore_xboxd:mp_util_wad.o)
// Verified against IDA (MPBrocCore_xboxd:mp_util_wad.o).
// ============================================================================

#include "mp_util_wad.h"
#include "engine/broc_types.h"
#include "game/AeThreadFunctor.h"
#include <string.h>
#include <stdio.h>
#include <new>

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

extern const char defaultFileName[];

// mp_util_wad entry points - ea: 0x947450 / 0x947480 / 0x948450.
void MusicStop()
{
    Broc::gBrocAPI.mMusicStop(0.0f);
}

void SoundStop(unsigned int handle)
{
    Broc::gBrocAPI.mSoundStop(handle);
}

void Destroy(const Broc::hudelem* hud)
{
    Broc::gBrocAPI.mDestroy(hud);
}

Broc::hudelem* NewHudElem(Broc::hudelem* result, int panelType)
{
    Broc::hudelem temp;
    result->___u0 = Broc::gBrocAPI.mNewHudElem(&temp, panelType)->___u0;
    return result;
}

void SetShader(const Broc::hudelem* hud, const Broc::string* shader, int w, int h)
{
    Broc::gBrocAPI.mSetShader(hud, shader, w, h);
}

void ScaleOverTime(const Broc::hudelem* hud, float time, int w, int h)
{
    Broc::gBrocAPI.mScaleOverTime(hud, time, w, h);
}

namespace mp_anim_wad {
int ResolveAnim(unsigned int treename, unsigned int animname,
                unsigned int* getVal, unsigned int setVal);
const char* ResolveAnimName(unsigned int anim);
bool ValidateAnimationIndices();
unsigned int GetBroAnim(unsigned int treename, unsigned int animname);
void RegisterHashStrings();
}

extern Broc::bint* GetEE_script_idnumber(Broc::entity ent);
extern Broc::bbool* IsEEDefined_script_idnumber(Broc::bbool* result, Broc::entity ent);

// ============================================================================
// Thread-functor externs (stubs at the bottom until each script is ported).
// ============================================================================
namespace _mp_audio {
void* PlayPainSound__functor(Broc::entity guy, Broc::bint damage);
void* audio_spawner__functor(Broc::entity self, Broc::string sound);
void* ambient_system__functor(Broc::entity self, Broc::string spawn_package);
AeThreadFunctor* PlaySound__functor(Broc::entity self, Broc::string sound,
                                     Broc::bfloat delay);
AeThreadFunctor* PlaySoundAtLocation__functor(Broc::entity self,
                                               Broc::string sound,
                                               Broc::vector position);
AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::bfloat>*
PlayTeamDialog__functor(Broc::entity self, Broc::string team,
                        Broc::string sound, Broc::bfloat delay);
AeThreadFunctor5<Broc::entity, Broc::string, Broc::string, Broc::string,
                 Broc::bfloat>*
PlayTeamDialog__functor(Broc::entity self, Broc::string primaryteam,
                        Broc::string primaryteamsound,
                        Broc::string secondaryteamsound,
                        Broc::bfloat delay);
AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::string>*
PlayTeamSound__functor(Broc::entity self, Broc::string team,
                       Broc::string teamsound,
                       Broc::string otherteamsound);
AeThreadFunctor* player_dying_sounds__functor(Broc::entity player);
void* audio_crossfade_wait__functor(Broc::entity self);
void* ThreadStaticSoundPlay__functor(Broc::entity self, Broc::string name);
void* ThreadStaticSoundRandomPlay__functor(Broc::entity self,
                                           Broc::string name);
void* MoveSoundAlongLine__functor(Broc::entity toMove, Broc::vector start,
                                  Broc::vector end);
void PlayDeathSound(Broc::entity guy, Broc::entity inflictor,
                    Broc::entity attacker, Broc::bint weapon,
                    Broc::bint means_of_damage);
void* interior_triggering_device__functor(Broc::entity trigger,
                                          Broc::entity other);
void* ThreadLineSound__functor(Broc::entity self);
void* ThreadStaticSound__functor(Broc::entity self);
void* sound_repeat__functor(Broc::entity self);
void* PlayerLocation__functor(Broc::entity self);
}
namespace _mp_loadout {
void local_player_joined(Broc::entity player);
void GiveLoadout(Broc::entity player);
void GiveSpecialWeapon(Broc::entity player, Broc::bint playerClass,
                       __int16 rank, bool isRespawn);
void UpdatePlayerModelForRank(Broc::entity player);
void DisplayYouWillSpawnWithMessage(Broc::entity self);
void* SpecialClassAudio__functor(Broc::entity player);
void* ArtilleryDispenser__functor(Broc::entity player);
void* HealthAmmoDispenser__functor(Broc::entity player, Broc::string weapon,
                                   Broc::bbool health, Broc::bint rank0Time,
                                   Broc::bint rank1Time, Broc::bint rank2Time);
void* NotifyWhenTimerExpires__functor(Broc::entity player, Broc::bint time,
                                      HashStr notifyString);
}
namespace _mp_shellshock {
void main();
void ShellshockOnDamage(Broc::entity self, ::bint cause, ::bint damage);
void blur_view(Broc::entity self, ::bfloat blur_time);
}
namespace _mp_spawnlogic {
Broc::entity* GetSpawnpointRandom(Broc::entity* result,
                                  Broc::dyn_array<Broc::entity>* points);
Broc::entity* get_spawnpoint_near_team_away_from_radios(
    Broc::entity* result, Broc::entity* self, const Broc::string* team,
    Broc::dyn_array<Broc::entity>* points);
Broc::entity* get_spawnpoint_middle_third(
    Broc::entity* result, Broc::entity* self, const Broc::string* team,
    Broc::dyn_array<Broc::entity>* points);
Broc::entity* GetSpawnpointRandom(Broc::entity* result,
                                  Broc::dyn_array<Broc::entity>* points,
                                  Broc::bbool ignoreTeleFrag);
Broc::entity* GetSpawnpointInOrder(Broc::entity* result,
                                   Broc::dyn_array<Broc::entity>* points);
Broc::entity* GetSpawnpointNearTeamAntiCamp(Broc::entity* result,
                                            Broc::entity* self,
                                            const Broc::string* team,
                                            Broc::dyn_array<Broc::entity>* points);
Broc::entity* GetSpawnpointNearest(Broc::entity* result,
                                   Broc::dyn_array<Broc::entity>* points,
                                   Broc::vector position, bool ignoreTeleFrag);
Broc::entity* GetSpawnpointSemiRandom(Broc::entity* result,
                                      Broc::entity* self,
                                      const Broc::string* team,
                                      Broc::dyn_array<Broc::entity>* points);
Broc::entity* GetSpawnpointNearTeam(Broc::entity* result,
                                    Broc::entity* self,
                                    const Broc::string* team,
                                    Broc::dyn_array<Broc::entity>* points);
Broc::entity* GetSpawnpointDM(Broc::entity* result, Broc::entity* self,
                              Broc::dyn_array<Broc::entity>* points);
}
namespace _mp_teambalance {
Broc::string* PickTeam(Broc::string* result, Broc::entity player);
Broc::string* team_balance(Broc::string* result, Broc::entity guy,
                           Broc::string team);
void team_balance(Broc::bbool always);
}
namespace _mp_common {
void StartRound(Broc::bbool firstTime);
AeThreadFunctor* StopFollowing__functor(Broc::entity self, Broc::bbool blackNow);
AeThreadFunctor* QuitGameThread__functor(Broc::entity selfLevel);
AeThreadFunctor* QuitGameWithMessage__functor(Broc::entity self, HashStr message);
AeThreadFunctor* HostHasMigrated__functor(Broc::entity self);
AeThreadFunctor* RespawnPlayer__functor(Broc::entity guy, Broc::string team);
AeThreadFunctor* LocalPlayerRespawn__functor(Broc::entity player);
AeThreadFunctor* PunishedForTeamKill__functor(Broc::entity ent,
                                              Broc::bbool punished);
AeThreadFunctor* reenable_medic_call__functor(Broc::entity self,
                                              Broc::bint time);
void* HandleJoinAfterRoundOver__functor(Broc::entity self, Broc::bint timeleft);
AeThreadFunctor* TeamChangeKillPlayer__functor(Broc::entity player);
AeThreadFunctor* FadeUpWhenLoaded__functor(Broc::entity self, Broc::entity player);
AeThreadFunctor* HealthRegenPlayerBreathing__functor(Broc::entity self, Broc::bint healthCap);
AeThreadFunctor* DeathState__functor(Broc::entity player, Broc::entity team_killer,
                                     Broc::bint delay, Broc::bbool reviveable,
                                     Broc::bbool fade);
AeThreadFunctor* UpdateSpectateCritical__functor(Broc::entity guy);
AeThreadFunctor1<Broc::entity>* UpdateSpectateCriticalGoingToDie__functor(
    Broc::entity guy);
AeThreadFunctor* UpdateSpectateDead__functor(Broc::entity guy, Broc::bbool canspawn);
AeThreadFunctor1<Broc::entity>* UpdateSpectateSpawn__functor(Broc::entity localPlayer);
AeThreadFunctor1<Broc::entity>* SpawnLocalSpectator__functor(Broc::entity guy);
AeThreadFunctor* restart_round__functor(Broc::entity selfLevel, Broc::bint waitTime);
AeThreadFunctor* finish_starting_round__functor(Broc::entity self, Broc::bbool firstTime);
AeThreadFunctor* AddArtilleryObjective__functor(Broc::entity self,
                                                Broc::vector position);
AeThreadFunctor* NewHost__functor(Broc::entity self);
AeThreadFunctor1<Broc::entity>* LocalPlayerIntermission__functor(Broc::entity player);
AeThreadFunctor1<Broc::entity>* RunFrame__functor(Broc::entity selfLevel);
}

namespace mp_util_wad {

extern void RegisterHashString(int h, const char* txt);

// IDA global word_39C6FA: pointer-backed key storage for the "flag" field.
static __int16 s_flagKey;
// IDA global byte_3A6023: pointer-backed key storage for the "goal" field.
static char s_goalKey;

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
    RegisterHashString(reinterpret_cast<int>(&s_flagKey), "flag");
    RegisterHashString(-1363748623, "flagEnd");
    RegisterHashString(953253096, "flagStart");
    RegisterHashString(-94902329, "flag_dropped");
    RegisterHashString(981991772, "flag_in_minefield");
    RegisterHashString(-2025829234, "flag_pickedup");
    RegisterHashString(1328252895, "flag_time_out");
    RegisterHashString(-1353926140, "flipped");
    RegisterHashString(reinterpret_cast<int>(&s_goalKey), "goal");
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

// GetEE_script_explodertype / IsEEDefined_script_explodertype - ea: 0x98CAC0 / 0x98CB10
Broc::string* GetEE_script_explodertype(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0xDAE997F9u);
}

Broc::bbool* IsEEDefined_script_explodertype(Broc::bbool* result,
                                              Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }

    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }

    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0xDAE997F9u);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_friendname / IsEEDefined_script_friendname - ea: 0x98CC10 / 0x98CC60
Broc::string* GetEE_script_friendname(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0xE217DD4Du);
}

Broc::bbool* IsEEDefined_script_friendname(Broc::bbool* result,
                                            Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }

    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }

    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0xE217DD4Du);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_prespawn_delay / IsEEDefined_script_prespawn_delay - ea: 0x98CD60 / 0x98CDB0
Broc::bfloat* GetEE_script_prespawn_delay(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0x531ABA72u);
}

Broc::bbool* IsEEDefined_script_prespawn_delay(Broc::bbool* result,
                                                Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }

    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }

    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0x531ABA72u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_burst_max / IsEEDefined_script_burst_max - ea: 0x98CE60 / 0x98CEB0
Broc::bfloat* GetEE_script_burst_max(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0xD74DC709u);
}

Broc::bbool* IsEEDefined_script_burst_max(Broc::bbool* result,
                                           Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }

    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }

    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0xD74DC709u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_offradius / IsEEDefined_script_offradius - ea: 0x98CF60 / 0x98CFB0
Broc::bint* GetEE_script_offradius(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x4A5EE717u);
}

Broc::bbool* IsEEDefined_script_offradius(Broc::bbool* result,
                                           Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }

    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }

    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x4A5EE717u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_location / IsEEDefined_script_location - ea: 0x98D060 / 0x98D0B0
Broc::string* GetEE_script_location(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x10E8AD0Du);
}

Broc::bbool* IsEEDefined_script_location(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }

    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }

    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x10E8AD0Du);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_area / IsEEDefined_script_area - ea: 0x98D1B0 / 0x98D200
Broc::string* GetEE_script_area(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x0225670Du);
}

Broc::bbool* IsEEDefined_script_area(Broc::bbool* result,
                                      Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }

    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }

    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x0225670Du);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_uniquename / IsEEDefined_script_uniquename - ea: 0x98D300 / 0x98D350
Broc::string* GetEE_script_uniquename(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0xF8E5A82Cu);
}

Broc::bbool* IsEEDefined_script_uniquename(Broc::bbool* result,
                                            Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }

    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }

    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0xF8E5A82Cu);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_random / IsEEDefined_script_random - ea: 0x98D450 / 0x98D4A0
Broc::bint* GetEE_script_random(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x47947B55u);
}

Broc::bbool* IsEEDefined_script_random(Broc::bbool* result,
                                         Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x47947B55u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_health / IsEEDefined_script_health - ea: 0x98D550 / 0x98D5A0
Broc::bint* GetEE_script_health(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x308248CAu);
}

Broc::bbool* IsEEDefined_script_health(Broc::bbool* result,
                                         Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x308248CAu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_flaktype / IsEEDefined_script_flaktype - ea: 0x98D650 / 0x98D6A0
Broc::string* GetEE_script_flaktype(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x9C79E174u);
}

Broc::bbool* IsEEDefined_script_flaktype(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x9C79E174u);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_accuracyStationaryMod / IsEEDefined_script_accuracyStationaryMod - ea: 0x98D7A0 / 0x98D7F0
Broc::bfloat* GetEE_script_accuracyStationaryMod(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0x0514248Du);
}

Broc::bbool* IsEEDefined_script_accuracyStationaryMod(
    Broc::bbool* result, Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0x0514248Du);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_noshadow / IsEEDefined_noshadow - ea: 0x98D8A0 / 0x98D8F0
Broc::bint* GetEE_noshadow(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xF89BEE03u);
}

Broc::bbool* IsEEDefined_noshadow(Broc::bbool* result,
                                   Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xF89BEE03u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_offtime / IsEEDefined_script_offtime - ea: 0x98D9A0 / 0x98D9F0
Broc::bint* GetEE_script_offtime(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x5E56E3BEu);
}

Broc::bbool* IsEEDefined_script_offtime(Broc::bbool* result,
                                         Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x5E56E3BEu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_playerseek / IsEEDefined_script_playerseek - ea: 0x98DAA0 / 0x98DAF0
Broc::bint* GetEE_script_playerseek(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xDD16B1E9u);
}

Broc::bbool* IsEEDefined_script_playerseek(Broc::bbool* result,
                                            Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xDD16B1E9u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_fixbasepose / IsEEDefined_script_fixbasepose - ea: 0x98DCF0 / 0x98DD40
Broc::bint* GetEE_script_fixbasepose(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x48E740ADu);
}

Broc::bbool* IsEEDefined_script_fixbasepose(Broc::bbool* result,
                                             Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x48E740ADu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_objective / IsEEDefined_script_objective - ea: 0x98DDF0 / 0x98DE40
Broc::string* GetEE_script_objective(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0xAF1A6D8Fu);
}

Broc::bbool* IsEEDefined_script_objective(Broc::bbool* result,
                                           Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0xAF1A6D8Fu);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_moveoverride / IsEEDefined_script_moveoverride - ea: 0x98DF40 / 0x98DF90
Broc::bint* GetEE_script_moveoverride(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x04B15AABu);
}

Broc::bbool* IsEEDefined_script_moveoverride(Broc::bbool* result,
                                              Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x04B15AABu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_stalingradspawn / IsEEDefined_script_stalingradspawn - ea: 0x98E040 / 0x98E090
Broc::bint* GetEE_script_stalingradspawn(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x15D9C126u);
}

Broc::bbool* IsEEDefined_script_stalingradspawn(Broc::bbool* result,
                                                 Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x15D9C126u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_new_exploder / IsEEDefined_script_new_exploder - ea: 0x98E140 / 0x98E190
Broc::bint* GetEE_script_new_exploder(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xC1C22900u);
}

Broc::bbool* IsEEDefined_script_new_exploder(Broc::bbool* result,
                                              Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xC1C22900u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_squadname / IsEEDefined_script_squadname - ea: 0x98E240 / 0x98E290
Broc::string* GetEE_script_squadname(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x9E624F93u);
}

Broc::bbool* IsEEDefined_script_squadname(Broc::bbool* result,
                                           Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x9E624F93u);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_weaponinfo / IsEEDefined_weaponinfo - ea: 0x98E390 / 0x98E3E0
Broc::string* GetEE_weaponinfo(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x135CE3F6u);
}

Broc::bbool* IsEEDefined_weaponinfo(Broc::bbool* result,
                                     Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x135CE3F6u);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_pacifist / IsEEDefined_script_pacifist - ea: 0x98E4E0 / 0x98E530
Broc::bint* GetEE_script_pacifist(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x8CC9C547u);
}

Broc::bbool* IsEEDefined_script_pacifist(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x8CC9C547u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_turretweaponpak / IsEEDefined_script_turretweaponpak - ea: 0x98E5E0 / 0x98E630
Broc::string* GetEE_script_turretweaponpak(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0xC655B680u);
}

Broc::bbool* IsEEDefined_script_turretweaponpak(Broc::bbool* result,
                                                 Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0xC655B680u);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_fb_retreat_chunk_size / IsEEDefined_script_fb_retreat_chunk_size - ea: 0x98E730 / 0x98E780
Broc::bint* GetEE_script_fb_retreat_chunk_size(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xEA795D04u);
}

Broc::bbool* IsEEDefined_script_fb_retreat_chunk_size(
    Broc::bbool* result, Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xEA795D04u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_grenades / IsEEDefined_script_grenades - ea: 0x98E830 / 0x98E880
Broc::bint* GetEE_script_grenades(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x601948FDu);
}

Broc::bbool* IsEEDefined_script_grenades(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x601948FDu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_hidden / IsEEDefined_script_hidden - ea: 0x98E930 / 0x98E980
Broc::string* GetEE_script_hidden(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x30CC2C00u);
}

Broc::bbool* IsEEDefined_script_hidden(Broc::bbool* result,
                                        Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x30CC2C00u);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_fb_id / IsEEDefined_script_fb_id - ea: 0x98EA80 / 0x98EAD0
Broc::bint* GetEE_script_fb_id(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x4723E508u);
}

Broc::bbool* IsEEDefined_script_fb_id(Broc::bbool* result,
                                      Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x4723E508u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_effect_id / IsEEDefined_script_effect_id - ea: 0x98EB80 / 0x98EBD0
Broc::string* GetEE_script_effect_id(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x9F5C37CDu);
}

Broc::bbool* IsEEDefined_script_effect_id(Broc::bbool* result,
                                           Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x9F5C37CDu);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_speed / IsEEDefined_script_speed - ea: 0x98ECD0 / 0x98ED20
Broc::bint* GetEE_script_speed(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x4816E9E5u);
}

Broc::bbool* IsEEDefined_script_speed(Broc::bbool* result,
                                       Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x4816E9E5u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_panzer / IsEEDefined_script_panzer - ea: 0x98EDD0 / 0x98EE20
Broc::bint* GetEE_script_panzer(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x42EA8664u);
}

Broc::bbool* IsEEDefined_script_panzer(Broc::bbool* result,
                                        Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x42EA8664u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_tankmgaccuracy / IsEEDefined_script_tankmgaccuracy - ea: 0x98EED0 / 0x98EF20
Broc::bint* GetEE_script_tankmgaccuracy(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xA565DE41u);
}

Broc::bbool* IsEEDefined_script_tankmgaccuracy(Broc::bbool* result,
                                                Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xA565DE41u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_colorid_startindex / IsEEDefined_script_colorid_startindex - ea: 0x98EFD0 / 0x98F020
Broc::bint* GetEE_script_colorid_startindex(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xD678CA45u);
}

Broc::bbool* IsEEDefined_script_colorid_startindex(
    Broc::bbool* result, Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xD678CA45u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_waittill / IsEEDefined_script_waittill - ea: 0x98F0D0 / 0x98F120
Broc::string* GetEE_script_waittill(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x1164451Eu);
}

Broc::bbool* IsEEDefined_script_waittill(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x1164451Eu);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_ambient / IsEEDefined_ambient - ea: 0x98F220 / 0x98F270
Broc::string* GetEE_ambient(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x303C0320u);
}

Broc::bbool* IsEEDefined_ambient(Broc::bbool* result,
                                  Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x303C0320u);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_burst_min / IsEEDefined_script_burst_min - ea: 0x98F370 / 0x98F3C0
Broc::bfloat* GetEE_script_burst_min(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0xD74DC807u);
}

Broc::bbool* IsEEDefined_script_burst_min(Broc::bbool* result,
                                           Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0xD74DC807u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_timer / IsEEDefined_script_timer - ea: 0x98F470 / 0x98F4C0
Broc::bint* GetEE_script_timer(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x48254DD5u);
}

Broc::bbool* IsEEDefined_script_timer(Broc::bbool* result,
                                       Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x48254DD5u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_noautoreenforcements / IsEEDefined_script_noautoreenforcements - ea: 0x98F570 / 0x98F5C0
Broc::bint* GetEE_script_noautoreenforcements(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x8806E5AAu);
}

Broc::bbool* IsEEDefined_script_noautoreenforcements(
    Broc::bbool* result, Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x8806E5AAu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_exploder / IsEEDefined_script_exploder - ea: 0x98F670 / 0x98F6C0
Broc::bint* GetEE_script_exploder(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x6EFF46F7u);
}

Broc::bbool* IsEEDefined_script_exploder(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x6EFF46F7u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_startinghealth / IsEEDefined_script_startinghealth - ea: 0x98F770 / 0x98F7C0
Broc::bint* GetEE_script_startinghealth(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xBA634DF6u);
}

Broc::bbool* IsEEDefined_script_startinghealth(Broc::bbool* result,
                                                Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xBA634DF6u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_flashlight / IsEEDefined_script_flashlight - ea: 0x98F870 / 0x98F8C0
Broc::bint* GetEE_script_flashlight(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xED5D0E1Au);
}

Broc::bbool* IsEEDefined_script_flashlight(Broc::bbool* result,
                                            Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xED5D0E1Au);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_fxid / IsEEDefined_script_fxid - ea: 0x98F970 / 0x98F9C0
Broc::string* GetEE_script_fxid(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x02283EFFu);
}

Broc::bbool* IsEEDefined_script_fxid(Broc::bbool* result,
                                      Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x02283EFFu);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_delete / IsEEDefined_script_delete - ea: 0x98FAC0 / 0x98FB10
Broc::bint* GetEE_script_delete(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x273390A7u);
}

Broc::bbool* IsEEDefined_script_delete(Broc::bbool* result,
                                        Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x273390A7u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_balcony / IsEEDefined_script_balcony - ea: 0x98FBC0 / 0x98FC10
Broc::bint* GetEE_script_balcony(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x6A5B5D1Cu);
}

Broc::bbool* IsEEDefined_script_balcony(Broc::bbool* result,
                                         Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x6A5B5D1Cu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_vehiclegroup / IsEEDefined_script_vehiclegroup - ea: 0x98FCC0 / 0x98FD10
Broc::bint* GetEE_script_vehiclegroup(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x0B5B16E1u);
}

Broc::bbool* IsEEDefined_script_vehiclegroup(Broc::bbool* result,
                                               Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x0B5B16E1u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_friendlywave / IsEEDefined_script_friendlywave - ea: 0x98FDC0 / 0x98FE10
Broc::bint* GetEE_script_friendlywave(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xC491D9C4u);
}

Broc::bbool* IsEEDefined_script_friendlywave(Broc::bbool* result,
                                               Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xC491D9C4u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_seekgoal / IsEEDefined_script_seekgoal - ea: 0x98FEC0 / 0x98FF10
Broc::bint* GetEE_script_seekgoal(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x8A475CBFu);
}

Broc::bbool* IsEEDefined_script_seekgoal(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x8A475CBFu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_fb_min_respawn_time / IsEEDefined_script_fb_min_respawn_time - ea: 0x98FFC0 / 0x990010
Broc::bfloat* GetEE_script_fb_min_respawn_time(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0x58DAB08Cu);
}

Broc::bbool* IsEEDefined_script_fb_min_respawn_time(
    Broc::bbool* result, Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0x58DAB08Cu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_dontdeploy / IsEEDefined_script_dontdeploy - ea: 0x9900C0 / 0x990110
Broc::bint* GetEE_script_dontdeploy(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x853FE9D6u);
}

Broc::bbool* IsEEDefined_script_dontdeploy(Broc::bbool* result,
                                            Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x853FE9D6u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_idnumber / IsEEDefined_script_idnumber - ea: 0x9901C0 / 0x990210
Broc::bint* GetEE_script_idnumber(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x1871456Au);
}

Broc::bbool* IsEEDefined_script_idnumber(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x1871456Au);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_fb_max_respawn_time / IsEEDefined_script_fb_max_respawn_time - ea: 0x9902C0 / 0x990310
Broc::bfloat* GetEE_script_fb_max_respawn_time(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0x8F2182CEu);
}

Broc::bbool* IsEEDefined_script_fb_max_respawn_time(
    Broc::bbool* result, Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0x8F2182CEu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_chargegoal / IsEEDefined_script_chargegoal - ea: 0x9903C0 / 0x990410
Broc::bint* GetEE_script_chargegoal(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xBF7C8E21u);
}

Broc::bbool* IsEEDefined_script_chargegoal(Broc::bbool* result,
                                             Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xBF7C8E21u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_string / IsEEDefined_script_string - ea: 0x9904C0 / 0x990510
Broc::string* GetEE_script_string(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x4B43BC2Bu);
}

Broc::bbool* IsEEDefined_script_string(Broc::bbool* result,
                                        Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x4B43BC2Bu);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_float / IsEEDefined_script_float - ea: 0x990610 / 0x990660
Broc::bfloat* GetEE_script_float(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0x4729A3EAu);
}

Broc::bbool* IsEEDefined_script_float(Broc::bbool* result,
                                       Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0x4729A3EAu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_breathpuff / IsEEDefined_script_breathpuff - ea: 0x990710 / 0x990760
Broc::bint* GetEE_script_breathpuff(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xFB4614FBu);
}

Broc::bbool* IsEEDefined_script_breathpuff(Broc::bbool* result,
                                            Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xFB4614FBu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_hdi_time / IsEEDefined_script_hdi_time - ea: 0x990810 / 0x990860
Broc::bfloat* GetEE_script_hdi_time(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0x1EFB7FB7u);
}

Broc::bbool* IsEEDefined_script_hdi_time(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0x1EFB7FB7u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_fxstart / IsEEDefined_script_fxstart - ea: 0x990910 / 0x990960
Broc::bint* GetEE_script_fxstart(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xD472B980u);
}

Broc::bbool* IsEEDefined_script_fxstart(Broc::bbool* result,
                                         Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xD472B980u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_delay_max / IsEEDefined_script_delay_max - ea: 0x990A10 / 0x990A60
Broc::bfloat* GetEE_script_delay_max(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0x04A98DA8u);
}

Broc::bbool* IsEEDefined_script_delay_max(Broc::bbool* result,
                                           Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0x04A98DA8u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_int / IsEEDefined_script_int - ea: 0x990B10 / 0x990B60
Broc::bint* GetEE_script_int(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xB27D62BFu);
}

Broc::bbool* IsEEDefined_script_int(Broc::bbool* result,
                                     Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xB27D62BFu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_requires_player / IsEEDefined_script_requires_player - ea: 0x990C10 / 0x990C60
Broc::bint* GetEE_script_requires_player(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x3D43F770u);
}

Broc::bbool* IsEEDefined_script_requires_player(Broc::bbool* result,
                                                 Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x3D43F770u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_animname / IsEEDefined_script_animname - ea: 0x990D10 / 0x990D60
Broc::string* GetEE_script_animname(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0xABEEF7DAu);
}

Broc::bbool* IsEEDefined_script_animname(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0xABEEF7DAu);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_battle / IsEEDefined_script_battle - ea: 0x990E60 / 0x990EB0
Broc::bint* GetEE_script_battle(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x22457F30u);
}

Broc::bbool* IsEEDefined_script_battle(Broc::bbool* result,
                                        Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x22457F30u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_followmin / IsEEDefined_script_followmin - ea: 0x990F60 / 0x990FB0
Broc::bint* GetEE_script_followmin(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x4135B46Bu);
}

Broc::bbool* IsEEDefined_script_followmin(Broc::bbool* result,
                                           Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x4135B46Bu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_grenadeawareness / IsEEDefined_script_grenadeawareness - ea: 0x991060 / 0x9910B0
Broc::bfloat* GetEE_script_grenadeawareness(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0x65D5CED3u);
}

Broc::bbool* IsEEDefined_script_grenadeawareness(
    Broc::bbool* result, Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0x65D5CED3u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_bravery / IsEEDefined_script_bravery - ea: 0x991160 / 0x9911B0
Broc::bfloat* GetEE_script_bravery(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bfloat>(0x9146436Fu);
}

Broc::bbool* IsEEDefined_script_bravery(Broc::bbool* result,
                                         Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bfloat value;
    const Broc::bfloat* stored =
        ee->GetVal<Broc::bfloat>(&value, 0x9146436Fu);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_destructible_id / IsEEDefined_destructible_id - ea: 0x991260 / 0x9912B0
Broc::bint* GetEE_destructible_id(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0xD35E38B6u);
}

Broc::bbool* IsEEDefined_destructible_id(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0xD35E38B6u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_dontdropgrenade / IsEEDefined_dontdropgrenade - ea: 0x991360 / 0x9913B0
Broc::bint* GetEE_dontdropgrenade(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x8F860160u);
}

Broc::bbool* IsEEDefined_dontdropgrenade(Broc::bbool* result,
                                          Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x8F860160u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_fxstop / IsEEDefined_script_fxstop - ea: 0x991460 / 0x9914B0
Broc::bint* GetEE_script_fxstop(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x2D39C958u);
}

Broc::bbool* IsEEDefined_script_fxstop(Broc::bbool* result,
                                        Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x2D39C958u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_accuracyvsplayer / IsEEDefined_script_accuracyvsplayer - ea: 0x991560 / 0x9915B0
Broc::bint* GetEE_script_accuracyvsplayer(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x4D4CD355u);
}

Broc::bbool* IsEEDefined_script_accuracyvsplayer(Broc::bbool* result,
                                                  Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x4D4CD355u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_script_eventhandler / IsEEDefined_script_eventhandler - ea: 0x991660 / 0x9916B0
Broc::string* GetEE_script_eventhandler(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::string>(0x5440EB34u);
}

Broc::bbool* IsEEDefined_script_eventhandler(Broc::bbool* result,
                                               Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    {
        Broc::string value;
        const Broc::string* stored =
            ee->GetVal<Broc::string>(&value, 0x5440EB34u);
        result->mVal = Broc::IsDefined(stored);
    }
    return result;
}

// GetEE_script_sightrange / IsEEDefined_script_sightrange - ea: 0x9917B0 / 0x991800
Broc::bint* GetEE_script_sightrange(Broc::entity ent) {
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    return &ee->GetRef<Broc::bint>(0x1BA1AFE0u);
}

Broc::bbool* IsEEDefined_script_sightrange(Broc::bbool* result,
                                            Broc::entity ent) {
    if (!Broc::IsDefined(ent)) {
        result->mVal = false;
        return result;
    }
    const unsigned int handle = ent.GetHandle();
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(handle);
    if (ee == nullptr) {
        result->mVal = false;
        return result;
    }
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x1BA1AFE0u);
    result->mVal = Broc::IsDefined(*stored);
    return result;
}

// GetEE_flagEnd / IsEEDefined_flagEnd (key 0xAEB6D8F1)
Broc::vector* GetEE_flagEnd(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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

// GetEE_respawnmodel / IsEEDefined_respawnmodel (key 0x86F21A11)
Broc::string* GetEE_respawnmodel(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0x86F21A11);
}

Broc::bbool* IsEEDefined_respawnmodel(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0x86F21A11);
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

// GetEE_deathmodel / IsEEDefined_deathmodel (key 0xEA19FC37)
Broc::string* GetEE_deathmodel(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0xEA19FC37);
}

Broc::bbool* IsEEDefined_deathmodel(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0xEA19FC37);
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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

// GetEE_deathfire / IsEEDefined_deathfire (key 0xF790400C)
Broc::string* GetEE_deathfire(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0xF790400C);
}

Broc::bbool* IsEEDefined_deathfire(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0xF790400C);
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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

// ea: 0x009301B0
Broc::string* GetEE_audio_ambp(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0xAB157931);
}

// ea: 0x00930200
Broc::bbool* IsEEDefined_audio_ambp(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0xAB157931);
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

// ea: 0x00930300
Broc::entity* GetEE_holder(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::entity>(0xFAAE111E);
}

// ea: 0x00930350
Broc::bbool* IsEEDefined_holder(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::entity v;
            const Broc::entity* val = ee->GetVal<Broc::entity>(&v, 0xFAAE111E);
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

// ea: 0x00930400
Broc::entity* GetEE_flag(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::entity>(reinterpret_cast<unsigned int>(&s_flagKey));
}

// ea: 0x00930450
Broc::bbool* IsEEDefined_flag(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::entity v;
            const Broc::entity* val = ee->GetVal<Broc::entity>(
                &v, reinterpret_cast<unsigned int>(&s_flagKey));
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

// ea: 0x00930900
Broc::entity* GetEE_goal(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::entity>(reinterpret_cast<unsigned int>(&s_goalKey));
}

// ea: 0x00930B00
Broc::entity* GetEE_trigger(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::entity>(0xF2F5EAB4);
}

// ea: 0x00931300
Broc::string* GetEE_audio_track(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0x0F1F2946);
}

// ea: 0x00931550
Broc::string* GetEE_damagelight(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0xAA4AB8F7);
}

// ea: 0x00931AA0
Broc::string* GetEE_reverb(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0x11523406);
}

// ea: 0x009320F0
Broc::entity* GetEE_lastspawnpoint(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::entity>(0x9AA56007);
}

// ea: 0x009322F0
Broc::string* GetEE_weaponstr(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0x93FB3A03);
}

// ea: 0x00932440
Broc::string* GetEE_damagecritical(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0xCF71F1AA);
}

// ea: 0x00932690
Broc::string* GetEE_deathfx(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0x04756FC4);
}

// ea: 0x009328E0
Broc::string* GetEE_damageheavy(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0xAA000DBC);
}

// ea: 0x00930950
Broc::bbool* IsEEDefined_goal(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::entity v;
            const Broc::entity* val = ee->GetVal<Broc::entity>(
                &v, reinterpret_cast<unsigned int>(&s_goalKey));
            result->mVal = Broc::IsDefined(*val);
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// ea: 0x00930B50
Broc::bbool* IsEEDefined_trigger(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::entity v;
            const Broc::entity* val = ee->GetVal<Broc::entity>(&v, 0xF2F5EAB4);
            result->mVal = Broc::IsDefined(*val);
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// ea: 0x00931350
Broc::bbool* IsEEDefined_audio_track(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0x0F1F2946);
            result->mVal = Broc::IsDefined(*val);
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// ea: 0x009315A0
Broc::bbool* IsEEDefined_damagelight(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0xAA4AB8F7);
            result->mVal = Broc::IsDefined(*val);
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// ea: 0x00931AF0
Broc::bbool* IsEEDefined_reverb(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0x11523406);
            result->mVal = Broc::IsDefined(*val);
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// ea: 0x00932140
Broc::bbool* IsEEDefined_lastspawnpoint(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::entity v;
            const Broc::entity* val = ee->GetVal<Broc::entity>(&v, 0x9AA56007);
            result->mVal = Broc::IsDefined(*val);
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// ea: 0x00932340
Broc::bbool* IsEEDefined_weaponstr(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0x93FB3A03);
            result->mVal = Broc::IsDefined(*val);
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// ea: 0x00932490
Broc::bbool* IsEEDefined_damagecritical(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0xCF71F1AA);
            result->mVal = Broc::IsDefined(*val);
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// ea: 0x009326E0
Broc::bbool* IsEEDefined_deathfx(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0x04756FC4);
            result->mVal = Broc::IsDefined(*val);
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// ea: 0x00932930
Broc::bbool* IsEEDefined_damageheavy(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v;
            const Broc::string* val = ee->GetVal<Broc::string>(&v, 0xAA000DBC);
            result->mVal = Broc::IsDefined(*val);
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::bfloat>(0xC6588AA5);
}

// GetEE_script_sound / IsEEDefined_script_sound (key 0x4816A2BD)
Broc::string* GetEE_script_sound(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
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

// GetEE_vehicletype / IsEEDefined_vehicletype (key 0x128D01A2)
Broc::string* GetEE_vehicletype(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    if (ee == NULL)
        ee = &Broc::ExtendedEntity::nullEnt;
    return &ee->GetRef<Broc::string>(0x128D01A2u);
}

Broc::bbool* IsEEDefined_vehicletype(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v; const Broc::string* val = ee->GetVal<Broc::string>(&v, 0x128D01A2u);
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

// ea: 0x00933030
void plane_flyby_setup(Broc::string t_name, Broc::string model, Broc::string sound,
                       Broc::bint speed, Broc::bfloat min_delay, Broc::bfloat max_delay,
                       Broc::bfloat sound_delay) {
    Broc::dyn_array<Broc::entity> ents;
    HashStr targetnameKey{0x19F9F0E8u};
    Broc::GetEntArray(&t_name, targetnameKey.mVal, &ents, 0);

    const char* const scriptFile = "c:\\cod\\code\\script\\_mp_airplanes.bro";
    auto warn = [&](const Broc::string& message) {
        if (Broc::gBrocAPI.mWarning(scriptFile, __LINE__, message.c_str()))
            __debugbreak();
    };
    auto originText = [](const Broc::entity& ent) {
        Broc::vector origin;
        mp_util_wad::entity_get_origin(&origin, ent);
        Broc::string result("(");
        result += origin.x;
        result += ", ";
        result += origin.y;
        result += ", ";
        result += origin.z;
        result += " )";
        return result;
    };

    if (Broc::size(ents) < 1) {
        Broc::string message = Broc::string("AIRPLANE PATH ERROR: Could not find any entities with a targetname of ") + t_name;
        warn(message);
        return;
    }

    for (int i = 0; i < Broc::size(ents); ++i) {
        Broc::bbool defined;
        IsEEDefined_script_idnumber(&defined, ents[(unsigned int)i]);
        if (!defined) {
            Broc::string message = Broc::string("AIRPLANE PATH ERROR: script_origin @ ") + originText(ents[(unsigned int)i]);
            message += " does not have a script_idnumber, ABORTING!";
            warn(message);
            return;
        }
    }

    for (int i = 0; i < Broc::size(ents); ++i) {
        for (int j = i; j < Broc::size(ents); ++j) {
            const int idJ = (int)*GetEE_script_idnumber(ents[(unsigned int)j]);
            const int idI = (int)*GetEE_script_idnumber(ents[(unsigned int)i]);
            if (idJ > idI) {
                Broc::entity temp = ents[(unsigned int)i];
                ents[(unsigned int)i] = ents[(unsigned int)j];
                ents[(unsigned int)j] = temp;
            } else if (j != i && idJ == idI) {
                Broc::string message = Broc::string("AIRPLANE PATH ERROR: script_origin @ ") + originText(ents[(unsigned int)i]);
                message += " has the same script_idnumber as the script_origin @ ";
                message += originText(ents[(unsigned int)j]);
                message += ", ABORTING!";
                warn(message);
                return;
            }
        }
    }

    mp_plane plane_struct;
    plane_struct.plane_model = model;
    plane_struct.plane_speed = speed;
    plane_struct.plane_sound = sound;
    plane_struct.plane_min_delay = min_delay;
    plane_struct.plane_max_delay = max_delay;
    plane_struct.plane_sound_delay = sound_delay;

    Broc::entity target_ent;
    for (int i = 0; i < Broc::size(ents); ++i) {
        Broc::string target;
        mp_util_wad::entity_get_target(&target, ents[(unsigned int)i]);
        if (!Broc::IsDefined(target)) {
            Broc::string message = Broc::string("AIRPLANE PATH ERROR: script_origin @ ") + originText(ents[(unsigned int)i]);
            message += " does not have a target, ABORTING!";
            warn(message);
            return;
        }

        HashStr targetKey{0x19F9F0E8u};
        Broc::GetEnt(&target_ent, &target, targetKey, 0);
        Broc::vector start;
        Broc::vector end;
        mp_util_wad::entity_get_origin(&start, ents[(unsigned int)i]);
        mp_util_wad::entity_get_origin(&end, target_ent);
        plane_struct.plane_start_orgs.push_back(start);
        plane_struct.plane_end_orgs.push_back(end);
        plane_struct.plane_dists.push_back(Broc::Distance(&start, &end));
        Broc::vector delta = end - start;
        Broc::vector angles;
        Broc::VectorToAngles(&angles, &delta);
        plane_struct.plane_angles.push_back(angles);
        Broc::Delete(&ents[(unsigned int)i]);
        Broc::Delete(&target_ent);
    }

    Broc::entity levelEntity;
    if (mp_util_wad::pLevel != nullptr)
        levelEntity = mp_util_wad::pLevel->_base.entity;
    void* ftor = plane_flyby_thread__functor(levelEntity, plane_struct);
    Broc::thread_create(false, scriptFile, __LINE__, "plane_flyby_thread", ftor);
}
void plane_flyby_thread(Broc::entity self, mp_plane plane_struct);
void plane_flyby(Broc::entity self, mp_plane plane_struct, Broc::bint num);
void plane_roll(Broc::entity self);
}

// ============================================================================
// Broc free helpers + gBrocAPI-backed wrappers (mp_util_wad.o COMDATs).
// ============================================================================
namespace stdext {

// ea: 0x00987990. IDA hashes a HashStr by XORing its value with the map seed.
unsigned int hash_value(HashStr* key)
{
    return static_cast<unsigned int>(*key) ^ 0xDEADBEEFu;
}

} // namespace stdext

namespace Broc {

// HUD property proxy operators - ea: 0x94ABD0 / 0x94AC10 / 0x94AC50 / 0x94AC90.
const int& Broc::hudelem::__unnamed::x_struct::operator=(const int& rhs)
{
    Broc::gBrocAPI.hud_set_x(mHandle, rhs);
    return rhs;
}

const int& Broc::hudelem::__unnamed::y_struct::operator=(const int& rhs)
{
    Broc::gBrocAPI.hud_set_y(mHandle, rhs);
    return rhs;
}

const float& Broc::hudelem::__unnamed::sort_struct::operator=(const float& rhs)
{
    Broc::gBrocAPI.hud_set_sort(mHandle, rhs);
    return rhs;
}

const unsigned char& Broc::hudelem::__unnamed::alpha_struct::operator=(
    const unsigned char& rhs)
{
    Broc::gBrocAPI.hud_set_alpha(mHandle, rhs);
    return rhs;
}

// Broc::entity::__unnamed::origin_struct::Get - ea: 0x9348D0
const Broc::vector* Broc::entity::__unnamed::origin_struct::Get(
    Broc::vector* result) const {
    Broc::vector temp;
    *result = *Broc::gBrocAPI.m_entity_get_origin(&temp, mHandle);
    return result;
}

// Broc::entity::__unnamed::origin_struct::operator= - ea: 0x938A10
const Broc::vector* Broc::entity::__unnamed::origin_struct::operator=(
    const Broc::vector* rhs) {
    Broc::gBrocAPI.m_entity_set_origin(mHandle, *rhs);
    return rhs;
}

// Broc::entity::__unnamed::target_struct::Get - ea: 0x934920
const Broc::string* Broc::entity::__unnamed::target_struct::Get(
    Broc::string* result) const {
    Broc::string temp;
    Broc::string* rhs = Broc::gBrocAPI.m_entity_get_target(&temp, mHandle);
    new (result) Broc::string(*rhs);
    return result;
}

// Broc::entity::__unnamed::targetname_struct::Get - ea: 0x93A080
const Broc::string* Broc::entity::__unnamed::targetname_struct::Get(
    Broc::string* result) const {
    Broc::string temp;
    Broc::string* rhs = Broc::gBrocAPI.m_entity_get_targetname(
        &temp, mHandle);
    new (result) Broc::string(*rhs);
    temp.~string();
    return result;
}

// Broc::entity::__unnamed::classname_struct::Get - ea: 0x97D060
const Broc::string* Broc::entity::__unnamed::classname_struct::Get(
    Broc::string* result) const {
    Broc::string temp;
    Broc::string* rhs = Broc::gBrocAPI.m_entity_get_classname(
        &temp, mHandle);
    new (result) Broc::string(*rhs);
    temp.~string();
    return result;
}

// Broc::entity::__unnamed::targetname_struct::operator= - ea: 0x955090
const Broc::string& Broc::entity::__unnamed::targetname_struct::operator=(
    const Broc::string& rhs) {
    Broc::gBrocAPI.m_entity_set_targetname(mHandle, rhs);
    return rhs;
}

// Broc::entity::__unnamed::maxhealth_struct::Get - ea: 0x93ADE0
const Broc::bint* Broc::entity::__unnamed::maxhealth_struct::Get(
    Broc::bint* result) const {
    new (result) Broc::bint(Broc::gBrocAPI.m_entity_get_maxhealth(mHandle));
    return result;
}

// Broc::entity::__unnamed::health_struct::Get - ea: 0x940E10
const Broc::bint* Broc::entity::__unnamed::health_struct::Get(
    Broc::bint* result) const {
    new (result) Broc::bint(Broc::gBrocAPI.m_entity_get_health(mHandle));
    return result;
}

// Broc::entity::__unnamed::health_struct::operator= - ea: 0x940250
const int& Broc::entity::__unnamed::health_struct::operator=(
    const int& rhs) {
    Broc::gBrocAPI.m_entity_set_health(mHandle, rhs);
    return rhs;
}

// Broc::entity::__unnamed::rank_struct::Get - ea: 0x940820
__int16 Broc::entity::__unnamed::rank_struct::Get() const {
    return Broc::gBrocAPI.m_entity_get_persistent_player_rank(mHandle);
}

// Broc::entity::__unnamed::rank_struct::operator= - ea: 0x9489F0
const __int16& Broc::entity::__unnamed::rank_struct::operator=(
    const __int16& rhs) {
    Broc::gBrocAPI.m_entity_set_persistent_player_rank(mHandle, rhs);
    return rhs;
}

// Broc::entity::__unnamed::playerState_struct::Get - ea: 0x93BDF0
const Broc::bint* Broc::entity::__unnamed::playerState_struct::Get(
    Broc::bint* result) const {
    new (result) Broc::bint(
        Broc::gBrocAPI.m_entity_get_persistent_player_playerState(mHandle));
    return result;
}

// Broc::entity::__unnamed::angles_struct::Get - ea: 0x93E340
const Broc::vector* Broc::entity::__unnamed::angles_struct::Get(
    Broc::vector* result) const {
    Broc::vector temp[6];
    *result = *Broc::gBrocAPI.m_entity_get_angles(temp, mHandle);
    return result;
}

// Broc::entity::__unnamed::viewangles_struct::Get - ea: 0x9491B0
const Broc::vector* Broc::entity::__unnamed::viewangles_struct::Get(
    Broc::vector* result) const {
    Broc::vector temp[6];
    *result = *Broc::gBrocAPI.m_entity_get_player_viewangles(temp, mHandle);
    return result;
}

// Broc::entity::__unnamed::spectatorClient_struct::operator= - ea: 0x93E390
const int& Broc::entity::__unnamed::spectatorClient_struct::operator=(
    const int& rhs) {
    Broc::gBrocAPI.m_entity_set_player_spectatorClient(mHandle, rhs);
    return rhs;
}

// Broc::entity::__unnamed::playerClass_struct::Get - ea: 0x940850
__int16 Broc::entity::__unnamed::playerClass_struct::Get() const {
    return Broc::gBrocAPI.m_entity_get_persistent_player_playerClass(mHandle);
}

// Broc::entity::__unnamed::nextPlayerClass_struct::Get - ea: 0x945FB0
__int16 Broc::entity::__unnamed::nextPlayerClass_struct::Get() const {
    return Broc::gBrocAPI.m_entity_get_persistent_player_nextPlayerClass(
        mHandle);
}

// Broc::entity::__unnamed::playerClass_struct::operator= - ea: 0x93E3D0
const __int16& Broc::entity::__unnamed::playerClass_struct::operator=(
    const __int16& rhs) {
    Broc::gBrocAPI.m_entity_set_persistent_player_playerClass(mHandle, rhs);
    return rhs;
}

// CloseMenu - ea: 0x940880
void Broc::CloseMenu(const Broc::string& str, int viewport) {
    Broc::gBrocAPI.mCloseMenu2(&str, viewport);
}

// DoDamage - ea: 0x940E50
void Broc::DoDamage(const Broc::entity& e, float damage,
                    const Broc::vector& vecIn, hitLocation_t hitLoc) {
    Broc::gBrocAPI.mDoDamage(e.GetHandle(), damage, &vecIn, hitLoc);
}

// SetTakeDamage - ea: 0x940E90
void Broc::SetTakeDamage(const Broc::entity& e, int damage) {
    Broc::gBrocAPI.mSetTakeDamage(e.GetHandle(), damage);
}

// Broc::TakeAllWeapons - ea: 0x946010
void Broc::TakeAllWeapons(const Broc::entity& e) {
    Broc::gBrocAPI.mTakeAllWeapons(e.GetHandle());
}

// Broc::OpenMenu - ea: 0x945810
int Broc::OpenMenu(const Broc::string& str, int viewport) {
    return Broc::gBrocAPI.mOpenMenu(&str, viewport);
}

// Broc::CloseAllMenus - ea: 0x9449D0
void Broc::CloseAllMenus(int viewport) {
    Broc::gBrocAPI.mCloseAllMenus(viewport);
}

// Broc::SetSpectateMedic - ea: 0x943C90
void Broc::SetSpectateMedic(int medic, int viewport) {
    Broc::gBrocAPI.mSetSpectateMedic(medic, viewport);
}

// Broc::SetSpectateState - ea: 0x945840
void Broc::SetSpectateState(int state, int viewport) {
    Broc::gBrocAPI.mSetSpectateState(state, viewport);
}

// Broc::SetSpectateTeamKill - ea: 0x945870
void Broc::SetSpectateTeamKill(int team_kill, const Broc::entity& e,
                               int viewport) {
    Broc::gBrocAPI.mSetSpectateTeamKill(team_kill, e.GetHandle(), viewport);
}

// Broc::SetSpectateSeconds - ea: 0x949EB0
void Broc::SetSpectateSeconds(int seconds, int viewport) {
    Broc::gBrocAPI.mSetSpectateSeconds(seconds, viewport);
}

// Code_IncPlayerStat - ea: 0x941F40
void Broc::Code_IncPlayerStat(Broc::entity player, unsigned int index,
                              __int16 value) {
    Broc::gBrocAPI.mIncPlayerStat(player.GetHandle(), index, value);
}

// Broc::entity::__unnamed::nextPlayerClass_struct::operator= - ea: 0x93E410
const __int16& Broc::entity::__unnamed::nextPlayerClass_struct::operator=(
    const __int16& rhs) {
    Broc::gBrocAPI.m_entity_set_persistent_player_nextPlayerClass(mHandle,
                                                                    rhs);
    return rhs;
}

// Broc::entity::__unnamed::playerState_struct::operator= - ea: 0x93E450
const int& Broc::entity::__unnamed::playerState_struct::operator=(
    const int& rhs) {
    Broc::gBrocAPI.m_entity_set_persistent_player_playerState(mHandle, rhs);
    return rhs;
}

// Broc::entity::__unnamed::angles_struct::operator= - ea: 0x935490
const Broc::vector* Broc::entity::__unnamed::angles_struct::operator=(
    const Broc::vector* rhs) {
    Broc::gBrocAPI.m_entity_set_angles(mHandle, *rhs);
    return rhs;
}

// Broc::entity::operator!= - ea: 0x93F640
bool Broc::entity::operator!=(const Broc::entity& rhs) const {
    return GetHandle() != rhs.GetHandle();
}

// Code_GetPlayerName - ea: 0x93E910
const char* Broc::Code_GetPlayerName(Broc::entity player) {
    return Broc::gBrocAPI.mGetPlayerName(player.GetHandle());
}

// Code_Obituary - ea: 0x93F680
void Broc::Code_Obituary(Broc::entity target, Broc::entity attacker,
                         const Broc::string* weapon, int mod, bool teamGame) {
    Broc::gBrocAPI.mObituary(target.GetHandle(), attacker.GetHandle(), weapon,
                             mod, teamGame);
}

// Code_GetSpotterEntity - ea: 0x93F700
Broc::entity Broc::Code_GetSpotterEntity(Broc::entity ent) {
    return Broc::entity(Broc::gBrocAPI.mGetSpotterEntity(ent.GetHandle()));
}

// Code_PlayerSpawn - ea: 0x93E490
void Broc::Code_PlayerSpawn(Broc::entity player, const Broc::vector& origin,
                            const Broc::vector& angles, bool stopPhysics) {
    Broc::gBrocAPI.mPlayerSpawn(player.GetHandle(), &origin, &angles,
                                stopPhysics);
}

// Broc::Code_RoundOver - ea: 0x944E70
void Broc::Code_RoundOver(int condition, const Broc::string& winner) {
    Broc::gBrocAPI.mRoundOver(condition, &winner);
}

// Broc::Code_PlayerRespawn - ea: 0x945310
void Broc::Code_PlayerRespawn(Broc::entity player,
                              const Broc::vector& origin,
                              const Broc::vector& angles,
                              const Broc::string& team) {
    Broc::gBrocAPI.mPlayerRespawn(player.GetHandle(), &origin, &angles, &team);
}

// Broc::Code_RequestRespawn - ea: 0x945FE0
void Broc::Code_RequestRespawn(unsigned int playerID) {
    Broc::gBrocAPI.mRequestRespawn(playerID);
}

// Broc::entity::__unnamed::team_struct::Get - ea: 0x938DE0
const Broc::string* Broc::entity::__unnamed::team_struct::Get(
    Broc::string* result) const {
    Broc::string temp;
    Broc::string* rhs = Broc::gBrocAPI.m_entity_get_sentient_team(
        &temp, mHandle);
    new (result) Broc::string(*rhs);
    temp.~string();
    return result;
}

// Broc::entity::operator== - ea: 0x94FA60
bool Broc::entity::operator==(const Broc::entity& rhs) const {
    return rhs.GetHandle() == GetHandle();
}

// Broc::entity::__unnamed::ctf_has_flag_struct::operator= - ea: 0x94E2A0
const __int16& Broc::entity::__unnamed::ctf_has_flag_struct::operator=(
    const __int16& rhs) {
    Broc::gBrocAPI.m_entity_set_player_ctf_has_flag(mHandle, rhs);
    return rhs;
}

// Broc::entity::__unnamed::ctf_has_flag_struct::Get - ea: 0x94FAA0
__int16 Broc::entity::__unnamed::ctf_has_flag_struct::Get() const {
    return Broc::gBrocAPI.m_entity_get_player_ctf_has_flag(mHandle);
}

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

template <typename T> bool IsDefined(const T* t) {
    return t != nullptr && t->IsDefined();
}
template <typename T> bool IsDefined(const Broc::dyn_array<T>& ar) {
    return ar.capacity() != 0;
}
bool IsDefined(const ::bfloat& t) {
    return t.IsDefined();
}
bool IsDefined(const ::bint& t) {
    return t.IsDefined();
}
bool IsDefined(const ::bbool& t) {
    return t.IsDefined();
}
bool IsDefined(const ::HashStr& t) {
    return t.IsDefined();
}
template bool IsDefined<Broc::hudelem>(const Broc::hudelem* t);
template bool IsDefined<Broc::entity>(
    const Broc::dyn_array<Broc::entity>& ar);

// Distance - ea: 0x934A50
float Distance(const Broc::vector* v0, const Broc::vector* v1) {
    return gBrocAPI.mVecDistance(v0, v1);
}

// Length - ea: 0x94EF50
float Length(const Broc::vector* v) {
    return gBrocAPI.mVecLength(v);
}

// VectorToAngles - ea: 0x934A80
Broc::vector* VectorToAngles(Broc::vector* result, const Broc::vector* vecIn) {
    Broc::vector vecOut;
    gBrocAPI.mVecToAngles(&vecOut, vecIn);
    *result = vecOut;
    return result;
}

// RandomInt - ea: 0x9350D0
int RandomInt(int iMax) {
    return gBrocAPI.mMathsRandomInt(iMax);
}

// GetTime - ea: 0x93AE20
Broc::bint* GetTime(Broc::bint* result) {
    new (result) Broc::bint(gBrocAPI.mGetTime());
    return result;
}

// IsPlayer - ea: 0x93BE30
int IsPlayer(const Broc::entity& e) {
    return gBrocAPI.mEntityIsPlayer(e.GetHandle());
}

// IsTouching - ea: 0x95AE30
bool IsTouching(const Broc::entity* e, const Broc::entity* other) {
    unsigned int other_handle = other->GetHandle();
    unsigned int handle = e->GetHandle();
    return gBrocAPI.mIsTouching(handle, other_handle);
}

// IsVehicle - ea: 0x93BE60
int IsVehicle(const Broc::entity& e) {
    return gBrocAPI.mEntityIsVehicle(e.GetHandle());
}

// Code_GetWeaponName - ea: 0x93BE90
void Code_GetWeaponName(int weaponIndex, Broc::string& weapon) {
    gBrocAPI.mGetWeaponName(static_cast<unsigned int>(weaponIndex), &weapon);
}

// GetCvarInt - ea: 0x935970
int GetCvarInt(const char* cvar) {
    return gBrocAPI.mCVarGetInt(cvar);
}

// GetCvar - ea: 0x93CB00
Broc::string* GetCvar(Broc::string* result, const char* cvar) {
    Broc::string output((Broc::string::Block*)nullptr);
    gBrocAPI.mCVarGetString(&output, cvar);
    new (result) Broc::string(output);
    output.~string();
    return result;
}

// SetCvar - ea: 0x93CBA0
void SetCvar(const char* cvar, const char* value) {
    gBrocAPI.mCVarSetString(cvar, value);
}

// Code_GetTeamScore - ea: 0x93D920
int Broc::Code_GetTeamScore(const Broc::string& team) {
    return gBrocAPI.mGetTeamScore(team);
}

// Code_SendGameState - ea: 0x93D950
void Broc::Code_SendGameState(Broc::entity player, int currentTime, int timeLimit,
                              int scoreLimit, int roundLimit, bool friendlyFire,
                              bool lastManStanding, bool teamBalance,
                              int respawnTime, int alliesScore, int axisScore,
                              bool roundStarted, int roundOver, int roundCount) {
    gBrocAPI.mSendGameState(player.GetHandle(), currentTime, timeLimit, scoreLimit,
                            roundLimit, friendlyFire, lastManStanding, teamBalance,
                            respawnTime, alliesScore, axisScore, roundStarted,
                            roundOver, roundCount);
}

// Broc::Code_SendGameStateHQ - ea: 0x95BAB0
void Broc::Code_SendGameStateHQ(Broc::entity player, unsigned int stage,
                                const Broc::vector& vA,
                                const Broc::vector& vB,
                                unsigned int triggerIndex,
                                bool alliesDefending, bool pointAIsHQ) {
    gBrocAPI.mSendGameStateHQ(player.GetHandle(), stage, vA, vB,
                              triggerIndex, alliesDefending, pointAIsHQ);
}

// Broc::Code_SendGameStateCTF - ea: 0x94EAA0
void Broc::Code_SendGameStateCTF(
    Broc::entity player, const Broc::vector* allied_flag,
    const Broc::vector* allied_angles, Broc::entity allied_flag_holder,
    const Broc::vector* axis_flag, const Broc::vector* axis_angles,
    Broc::entity axis_flag_holder) {
    Broc::vector axisAnglesCopy = *axis_angles;
    Broc::vector axisFlagCopy = *axis_flag;
    Broc::vector alliedAnglesCopy = *allied_angles;
    Broc::vector alliedFlagCopy = *allied_flag;
    gBrocAPI.mSendGameStateCTF(
        player.GetHandle(), alliedFlagCopy, alliedAnglesCopy,
        allied_flag_holder.GetHandle(), axisFlagCopy, axisAnglesCopy,
        axis_flag_holder.GetHandle());
}

// Broc::Code_SendGameStateSCF - ea: 0x967FC0
void Broc::Code_SendGameStateSCF(Broc::entity player, int defendingTeam,
                                const Broc::vector* flag,
                                const Broc::vector* flagAngles,
                                Broc::entity flag_holder) {
    Broc::vector flagAnglesCopy = *flagAngles;
    Broc::vector flagCopy = *flag;
    gBrocAPI.mSendGameStateSCF(player.GetHandle(), defendingTeam,
                               flagCopy, flagAnglesCopy,
                               flag_holder.GetHandle());
}

// Code_IncTeamScore - ea: 0x93DE70
void Broc::Code_IncTeamScore(const Broc::string& team, int amount) {
    gBrocAPI.mIncTeamScore(team, amount);
}

// SetCvar - ea: 0x93DED0
void SetCvar(const char* cvar, int value) {
    gBrocAPI.mCVarSetInt(cvar, value);
}

// AddEventHandler - ea: 0x9360D0
void AddEventHandler(const Broc::entity& e, HashStr label, HashStr func) {
    gBrocAPI.mAddEventHandler(e.GetHandle(), label.mVal, func.mVal);
}

void AddEventHandler(Broc::entity* e, unsigned int label, unsigned int func) {
    if (e != NULL) {
        HashStr labelHash;
        HashStr funcHash;
        labelHash.mVal = label;
        funcHash.mVal = func;
        AddEventHandler(*e, labelHash, funcHash);
    }
}

// Broc::RemoveEventHandler - ea: 0x95B4F0
bool RemoveEventHandler(Broc::entity* e, HashStr label, HashStr func) {
    return gBrocAPI.mRemoveEventHandler(e->GetHandle(), label.mVal,
                                         func.mVal);
}

// SoundPlay - ea: 0x936070
unsigned int SoundPlay(const Broc::string& name, float volume) {
    return gBrocAPI.mSoundPlay(&name, volume);
}

unsigned int SoundPlay(const Broc::string* name, float volume) {
    return gBrocAPI.mSoundPlay(name, volume);
}

// Broc::ShellShock - ea: 0x96CC60
void Broc::ShellShock(Broc::entity* e, const Broc::string* shock,
                      float fVal) {
    const unsigned int handle = e->GetHandle();
    gBrocAPI.mShellShock(handle, shock, fVal);
}

// Code_IsLocalPlayer - ea: 0x936740
bool Broc::Code_IsLocalPlayer(Broc::entity player) {
    return gBrocAPI.mIsLocalPlayer(player.GetHandle());
}

// GetPlayerIndex - ea: 0x93E590
int Broc::GetPlayerIndex(Broc::entity player) {
    return gBrocAPI.mGetPlayerIndex(player.GetHandle());
}

// Code_GetPlayerInSeat - ea: 0x9725C0
void Broc::Code_GetPlayerInSeat(Broc::entity* result,
                                Broc::entity vehicle, unsigned int seat) {
    new (result) Broc::entity(gBrocAPI.mGetPlayerInSeat(
        vehicle.GetHandle(), seat));
}

// Rumble - ea: 0x972600
void Broc::Rumble(const Broc::string* lowFreqNotes, float lowFreqDuraton,
                  Broc::string highFreqNotes, float highFreqDuration,
                  int player_index) {
    gBrocAPI.mRumbleNotes(lowFreqNotes, lowFreqDuraton, &highFreqNotes,
                          highFreqDuration, player_index);
    highFreqNotes.~string();
}

// Earthquake - ea: 0x972680
void Broc::Earthquake(float scale, float duration,
                      const Broc::vector* source, float radius,
                      int player_index) {
    gBrocAPI.mEarthquake(scale, duration, source, radius, player_index);
}

// Code_ChangePlayerTeam - ea: 0x974680
void Broc::Code_ChangePlayerTeam(Broc::entity player,
                                 const Broc::string* team,
                                 bool autoBalance) {
    gBrocAPI.mChangePlayerTeam(player.GetHandle(), team, autoBalance);
}

// Code_SendGameScore - ea: 0x975D70
void Broc::Code_SendGameScore(int alliesScore, int axisScore) {
    gBrocAPI.mSendGameScore(alliesScore, axisScore);
}

// ObjectiveState - ea: 0x9769D0
void Broc::ObjectiveState(int iObjective, const Broc::string* inState,
                         const char* pDisplay, int clientIndex) {
    gBrocAPI.mObjectiveState(iObjective, inState, pDisplay, clientIndex);
}

// SoundFadeOut - ea: 0x975950
void Broc::SoundFadeOut(unsigned int handle, float time) {
    gBrocAPI.mSoundFadeOut(handle, time);
}

// ReverbSetParams - ea: 0x9360A0
void ReverbSetParams(const Broc::string& name, bool immediate) {
    gBrocAPI.mReverbSetParams(&name, immediate);
}

void ReverbSetParams(const Broc::string* name, bool immediate) {
    gBrocAPI.mReverbSetParams(name, immediate);
}

// Broc::SoundCrossFade - ea: 0x936F50
void SoundCrossFade(unsigned int handle1, unsigned int handle2, float time) {
    gBrocAPI.mSoundCrossFade(handle1, handle2, time);
}

// GetEnt - ea: 0x9349D0
Broc::entity* GetEnt(Broc::entity* result, const Broc::string* val, HashStr key,
                     unsigned int flags) {
    unsigned int v4 = gBrocAPI.mGetEnt(val, key.mVal, NULL, 0, flags);
    new (result) Broc::entity(v4);
    return result;
}

// Broc::Spawn - ea: 0x9354E0
Broc::entity* Spawn(Broc::entity* result, const Broc::string* inClassname,
                    const Broc::vector* origin, TPakInfo pakInfo) {
    unsigned int handle = gBrocAPI.mSpawn(inClassname, origin,
                                          pakInfo);
    new (result) Broc::entity(handle);
    return result;
}

// Broc::Spawn (with size/flags) - ea: 0x9550E0
Broc::entity* Spawn(Broc::entity* result, const Broc::string* inClassname,
                    const Broc::vector* origin, Broc::vector* mins,
                    Broc::vector* maxs, int iSpawnFlags, TPakInfo pakInfo) {
    unsigned int handle = gBrocAPI.mSpawnWithFlagAndSize(
        inClassname, origin, mins, maxs, iSpawnFlags, pakInfo);
    new (result) Broc::entity(handle);
    return result;
}

// Broc::LinkTo - ea: 0x955130
void LinkTo(Broc::entity* e, Broc::entity* pe) {
    gBrocAPI.mLinkTo3(e->GetHandle(), pe->GetHandle());
}

// Broc::UseButtonPressed - ea: 0x954120
int UseButtonPressed(const Broc::entity& e) {
    return gBrocAPI.mUseButtonPressed(e.GetHandle());
}

// Broc::Code_AreaCaptured - ea: 0x954940
void Broc::Code_AreaCaptured(int netID, unsigned int team,
                             unsigned int hostOnly) {
    gBrocAPI.mAreaCaptured(netID, team, hostOnly);
}

// Broc::Code_DebugRenderBox - ea: 0x94CD50
void Broc::Code_DebugRenderBox(const Broc::vector* min,
                               const Broc::vector* max,
                               const Broc::vector* color, float alpha) {
    gBrocAPI.mDebugRenderBox(min, max, color, alpha);
}

// Broc::Code_DebugRenderText - ea: 0x955AF0
void Broc::Code_DebugRenderText(const char* text, int x, int y) {
    gBrocAPI.mDebugRenderText(text, x, y);
}

// Broc::Code_DebugRenderEntityBBox - ea: 0x9562E0
void Broc::Code_DebugRenderEntityBBox(Broc::entity e,
                                      const Broc::vector* color,
                                      float alpha) {
    gBrocAPI.mDebugRenderEntityBBox(e.GetHandle(), color, alpha);
}

// Broc::Code_DebugRenderSphere - ea: 0x956320
void Broc::Code_DebugRenderSphere(const Broc::vector* point, float radius,
                                  const Broc::vector* color, float alpha) {
    gBrocAPI.mDebugRenderSphere(point, radius, color, alpha);
}

// Broc::EffectEventPlay - ea: 0x935520
int EffectEventPlay(Broc::entity* e, const Broc::string* script) {
    unsigned int handle = e->GetHandle();
    return gBrocAPI.mEffectEventPlay(handle, script, 0, false, false);
}

// Broc::notify - ea: 0x939BA0
void notify(const Broc::entity& ent, HashStr label) {
    gBrocAPI.mEntNotify(ent.GetHandle(), label.mVal);
}

// Broc::notify - ea: 0x94B8B0
void notify(const Broc::entity& ent, const char* label) {
    HashStr hash;
    Broc::string_hash(&hash, label);
    gBrocAPI.mEntNotify(ent.GetHandle(), hash.mVal);
}

// GetWeaponSlotAmmo - ea: 0x94BF20
int GetWeaponSlotAmmo(const Broc::entity& e, const Broc::string& slot) {
    return gBrocAPI.mGetWeaponSlotAmmo(e.GetHandle(), &slot);
}

// GetWeaponSlotClipAmmo - ea: 0x94BF50
int GetWeaponSlotClipAmmo(const Broc::entity& e, const Broc::string& slot) {
    return gBrocAPI.mGetWeaponSlotClipAmmo(e.GetHandle(), &slot);
}

// SwitchToWeapon - ea: 0x94C1B0
bool SwitchToWeapon(const Broc::entity& e, const Broc::string& weapon) {
    return gBrocAPI.mSwitchToWeapon(e.GetHandle(), &weapon);
}

// GetWeaponSlotWeapon - ea: 0x94C1E0
void GetWeaponSlotWeapon(const Broc::entity& e, const Broc::string& slot,
                         Broc::string& result) {
    gBrocAPI.mGetWeaponSlotWeapon(e.GetHandle(), &slot, &result);
}

// Broc::DialogPlay - ea: 0x938BA0
int DialogPlay(const Broc::entity& e, const Broc::string& script) {
    return gBrocAPI.mDialogPlay(e.GetHandle(), &script, 0, false);
}

int DialogPlay(Broc::entity e, const Broc::string* script) {
    return gBrocAPI.mDialogPlay(e.GetHandle(), script, 0, false);
}

// Broc::EffectEventPlay (notify) - ea: 0x939550
int EffectEventPlay(Broc::entity* e, const Broc::string* script,
                    HashStr notifyHash, bool stoppable) {
    return gBrocAPI.mEffectEventPlay(e->GetHandle(), script,
                                     notifyHash.mVal, stoppable, false);
}

// Broc::EffectEventStopEmitting - ea: 0x939B70
void EffectEventStopEmitting(unsigned int effectId) {
    gBrocAPI.mEffectEventStop(effectId);
}

// Broc::EffectEventPlay (non-entity) - ea: 0x937A90
int Broc::EffectEventPlay(const Broc::string& script,
                          const Broc::vector& pos,
                          const Broc::vector& facing) {
    return gBrocAPI.mEffectEventPlayNonEnt(&script, &pos, &facing,
                                           false, 0, 0);
}

// Broc::SetModel - ea: 0x935560
void SetModel(Broc::entity* e, const Broc::string* modelName, int whichPak) {
    unsigned int handle = e->GetHandle();
    gBrocAPI.mSetModel(handle, modelName, (TPakInfo)whichPak);
}

// Broc::Show - gBrocAPI mShow forwarding wrapper.
void Show(Broc::entity* e) {
    gBrocAPI.mShow(e->GetHandle());
}

// Broc::Code_DropItem - ea: 0x950B10
void Code_DropItem(int itemType, int netID, const Broc::vector* position,
                   const Broc::vector* angles,
                   const Broc::vector* velocity) {
    gBrocAPI.mDropItem2(itemType, netID, position, angles, velocity);
}

// Broc::Code_HostDropItem - ea: 0x96AE40
void Code_HostDropItem(int itemType, int netID,
                       const Broc::vector* position,
                       const Broc::vector* angles,
                       const Broc::vector* velocity) {
    gBrocAPI.mHostDropItem2(itemType, netID, position, angles, velocity);
}

// Broc::Code_PositionWouldTelefrag - ea: 0x96CF50
bool Code_PositionWouldTelefrag(const Broc::vector* position) {
    return gBrocAPI.mPositionWouldTelefrag(position);
}

// Broc::GetOrigin - ea: 0x970C60
Broc::vector* GetOrigin(Broc::vector* result, const Broc::entity* e) {
    Broc::vector outVec;
    gBrocAPI.mGetOrigin(e->GetHandle(), &outVec);
    *result = outVec;
    return result;
}

// Broc::IsVehicleFlipped - ea: 0x970E80
bool IsVehicleFlipped(const Broc::entity* e) {
    return gBrocAPI.mIsVehicleFlipped(e->GetHandle());
}

// Broc::Code_BroadcastVehicleRespawn - ea: 0x9713F0
void Code_BroadcastVehicleRespawn(Broc::entity vehicle) {
    gBrocAPI.mBroadcastVehicleRespawn(vehicle.GetHandle());
}

// Broc::Code_RespawnVehicle - ea: 0x971420
void Code_RespawnVehicle(Broc::entity* e) {
    gBrocAPI.mRespawnVehicle(e->GetHandle());
}

// Broc::TakeWeapon - ea: 0x950B50
void Broc::TakeWeapon(Broc::entity* e,
                      const Broc::string* pszWeaponName) {
    gBrocAPI.mTakeWeapon(e->GetHandle(), pszWeaponName);
}

// Broc::SwitchToLastWeapon - ea: 0x950B80
bool SwitchToLastWeapon(Broc::entity* e) {
    return gBrocAPI.mSwitchToLastWeapon(e->GetHandle());
}

// Broc::Launch - ea: 0x950C40
void Launch(Broc::entity* e, const Broc::vector* velocity) {
    gBrocAPI.mLaunch(e->GetHandle(), velocity);
}

// Broc::SetOwner - ea: 0x9528E0
void SetOwner(Broc::entity* e, Broc::entity* owner) {
    gBrocAPI.mSetOwner(e->GetHandle(), owner->GetHandle());
}

// Broc::GiveWeapon - ea: 0x952920
void GiveWeapon(Broc::entity* e, const Broc::string* pszWeaponName) {
    gBrocAPI.mGiveWeapon(e->GetHandle(), pszWeaponName);
}

// Broc::SetWeaponSlotClipAmmo - ea: 0x952950
void SetWeaponSlotClipAmmo(Broc::entity* e, const Broc::string* sSlot,
                           int iSetClipAmmo) {
    gBrocAPI.mSetWeaponSlotClipAmmo(e->GetHandle(), sSlot, iSetClipAmmo);
}

// Broc::MoveTo - ea: 0x9355A0
void MoveTo(Broc::entity* e, const Broc::vector* vPos, float totalTime,
            float accTime, float decTime) {
    unsigned int handle = e->GetHandle();
    gBrocAPI.mMoveTo(handle, vPos, totalTime, accTime, decTime);
}

// Delete - ea: 0x934A20
void Delete(const Broc::entity& e) {
    gBrocAPI.mDelete(e.GetHandle());
}

void Delete(Broc::entity* e) {
    if (e != NULL)
        Delete(*e);
}

// RotateTo - ea: 0x9358B0
void RotateTo(const Broc::entity& e, const Broc::vector& angles,
              float totalTime, float accTime, float decTime) {
    gBrocAPI.mRotateTo(e.GetHandle(), angles, totalTime, accTime, decTime);
}

void RotateTo(Broc::entity* e, const Broc::vector* angles, float totalTime,
              float accTime, float decTime) {
    if (e != NULL && angles != NULL)
        RotateTo(*e, *angles, totalTime, accTime, decTime);
}

// ea: 0x009871C0. IDA forwards entity/notify values to the Broc API slot.
bool AssignParameterForNotify(const Broc::entity& ent, HashStr signal,
                              WaitTilOutput* output)
{
    return gBrocAPI.mAssignParameterForNotify(ent.GetHandle(),
                                              static_cast<unsigned int>(signal),
                                              output);
}

// ea: 0x00987100. IDA constructs the entity output carrier on the stack,
// waits for the notify, then copies the assigned entity back to the caller.
void waittill(Broc::entity ent, HashStr signal, Broc::entity* output)
{
    __declspec(align(4)) unsigned char storage[0x10];
    WaitTilOutput* outParms =
        WaitTilOutputInst1Entity_Construct(storage, *output);
    Broc::waittill(ent, signal);
    if (AssignParameterForNotify(ent, signal, outParms))
        WaitTilOutputInst1Entity_CopyData(outParms, output);
    WaitTilOutputInst1Entity_Destroy(outParms);
}

const char* GetText(const Broc::string& str, char* /*buff*/) {
    return gBrocAPI.mLocalize(str.c_str());
}

const char* GetText(const char* txt, char* /*buff*/) {
    return gBrocAPI.mLocalize(txt);
}

char* GetText(const ::bint& val, char* buff) {
    sprintf(buff, "%d", (int)val);
    return buff;
}

char* GetText(const Broc::vector& v, char* buff) {
    sprintf(buff, "%f,%f,%f", v.x, v.y, v.z);
    return buff;
}

void PrintConcat(const Broc::string& txt, bool bold) {
    const char* value = txt.c_str();
    if (bold)
        gBrocAPI.mIPrintLnBold(value);
    else
        gBrocAPI.mIPrintLn(value);
}

void PrintLine(const Broc::string& txt) {
    gBrocAPI.mPrintLn(txt.c_str());
}

void iprintln(const char* msg) {
    Broc::string txt(static_cast<Broc::string::Block*>(NULL));
    ConcatText(txt, msg);
    PrintConcat(txt, false);
}

void iprintln(const char* a, const char* sep, const char* b) {
    Broc::string txt(static_cast<Broc::string::Block*>(NULL));
    ConcatText(txt, a, sep, b);
    PrintConcat(txt, false);
}

void iprintlnbold(const Broc::string& value) {
    Broc::string txt(static_cast<Broc::string::Block*>(NULL));
    ConcatText(txt, value);
    PrintConcat(txt, true);
}

template <>
void ConcatText<Broc::string, Broc::vector>(Broc::string& txt,
                                            const Broc::string& lhs,
                                            const Broc::vector& rhs) {
    char tmpBuf[256];
    txt = GetText(lhs, tmpBuf);
    txt += GetText(rhs, tmpBuf);
}

// operator+(string, int) - ea: 0x9658C0
Broc::string operator+(const Broc::string& lhs, int rhs) {
    Broc::string r(lhs);
    r += rhs;
    return r;
}

// operator+(string, float) - ea: 0x934830
Broc::string operator+(const Broc::string& lhs, float rhs) {
    Broc::string r(lhs);
    r += rhs;
    return r;
}

// operator+(string, vector) - ea: 0x965960
Broc::string operator+(const Broc::string& lhs, const Broc::vector& rhs) {
    Broc::string txt(static_cast<Broc::string::Block*>(nullptr));
    ConcatText(txt, lhs, rhs);
    return Broc::string(txt);
}

} // namespace Broc

// Code_IsMenuOpen - ea: 0x94A8A0
int Broc::Code_IsMenuOpen(const Broc::string& menu, int viewport) {
    return Broc::gBrocAPI.mIsMenuOpen(&menu, viewport);
}

// ObjectiveDelete - ea: 0x94BA60
void ObjectiveDelete(int iObjective, int clientIndex) {
    Broc::gBrocAPI.mObjectiveDelete(iObjective, clientIndex);
}

// ObjectiveRing - ea: 0x952990
void ObjectiveRing(int iObjective, int clientIndex) {
    Broc::gBrocAPI.mObjectiveRing(iObjective, clientIndex);
}

// ObjectiveAdd - ea: 0x94BA90
void ObjectiveAdd(int iObjective, const Broc::string& state,
                  const Broc::string& pszString, Broc::vector vPos,
                  const char* display) {
    Broc::gBrocAPI.mObjectiveAdd5(iObjective, &state, &pszString, &vPos, 0,
                                  0.0f, display, -1, -1, -1);
}

// ObjectiveAdd - ea: 0x94D2D0
void ObjectiveAdd(int iObjective, const Broc::string& state,
                  const Broc::string& pszString, Broc::vector vPos,
                  float height, int clientIndex) {
    Broc::gBrocAPI.mObjectiveAdd5(iObjective, &state, &pszString, &vPos, 0,
                                  height, "1", -1, -1, clientIndex);
}

// GetWeaponIndex - ea: 0x95EE40
int Broc::GetWeaponIndex(const Broc::string* team) {
    return Broc::gBrocAPI.mGetWeaponIndex(team);
}

// SetWeaponSlotAmmo - ea: 0x95F9B0
void Broc::SetWeaponSlotAmmo(Broc::entity* e, const Broc::string* sSlot,
                             int iSetAmmo) {
    unsigned int handle = e->GetHandle();
    Broc::gBrocAPI.mSetWeaponSlotAmmo(handle, sSlot, iSetAmmo);
}

// GetFullClipAmmoCount - ea: 0x95F9F0
int Broc::GetFullClipAmmoCount(Broc::entity* e, const Broc::string* slot) {
    unsigned int handle = e->GetHandle();
    return Broc::gBrocAPI.mGetFullClipAmmoCount(handle, slot);
}

// GetMaxAmmo - ea: 0x961BA0
int Broc::GetMaxAmmo(Broc::entity* e, const Broc::string* slot) {
    unsigned int handle = e->GetHandle();
    return Broc::gBrocAPI.mGetMaxAmmo(handle, slot);
}

// RandomIntRange - ea: 0x964890
int Broc::RandomIntRange(int iMin, int iMax) {
    return Broc::gBrocAPI.mMathsRandomIntRange(iMin, iMax);
}

// CreateNanoForce - ea: 0x964970
unsigned int Broc::CreateNanoForce(const Broc::string* id,
                                   const Broc::vector* param1,
                                   const Broc::vector* param2) {
    return Broc::gBrocAPI.mCreateNanoForce(id, param1, param2);
}

// EnableNanoForces - direct BrocAPI callback
void Broc::EnableNanoForces(bool onOff) {
    Broc::gBrocAPI.mEnableNanoForces(onOff);
}

// RadiusDamage - ea: 0x9645B0
void Broc::RadiusDamage(const Broc::vector* origin, float range,
                        float max_damage, float min_damage, int damageType) {
    Broc::gBrocAPI.mRadiusDamage(origin, range, max_damage, min_damage,
                                 damageType);
}

// bbool::operator== - ea: 0x93F610
bool bbool::operator==(bool rhs) const {
    return rhs == mVal;
}

// AnglesToForward - ea: 0x937AD0
Broc::vector AnglesToForward(const Broc::vector& angles) {
    Broc::vector result;
    Broc::gBrocAPI.mVecAnglesToForward(&result, &angles);
    return result;
}

// DistanceSquared - ea: 0x938A60
float DistanceSquared(const Broc::vector& a, const Broc::vector& b) {
    return Broc::gBrocAPI.mVecDistanceSquared(&a, &b);
}

// ============================================================================
// Boxed-type operators (mp_util_wad.o inline COMDATs)
// ============================================================================
Broc::bbool operator<(Broc::bint lhs, Broc::bint rhs) {
    return Broc::bbool(lhs.mVal < rhs.mVal);
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

Broc::bbool operator<(Broc::bint lhs, int rhs) {
    return Broc::bbool(lhs.mVal < rhs);
}

Broc::bfloat operator*(Broc::bfloat lhs, float rhs) {
    return Broc::bfloat(lhs.mVal * rhs);
}

// operator!=(bint, int) - ea: 0x93BD20
bool Broc::bint::operator!=(int rhs) const {
    return mVal != rhs;
}

// operator*(bfloat, bfloat) - ea: 0x93BD50
Broc::bfloat operator*(Broc::bfloat lhs, Broc::bfloat rhs) {
    return Broc::bfloat(lhs.mVal * rhs.mVal);
}

// operator*(float, bfloat) - ea: 0x93ACC0
Broc::bfloat operator*(float lhs, Broc::bfloat rhs) {
    return Broc::bfloat(rhs.mVal * lhs);
}

// operator*(bfloat, int) - ea: 0x93AD40
Broc::bfloat operator*(Broc::bfloat lhs, int rhs) {
    return Broc::bfloat(lhs.mVal * rhs);
}

// bint::bint(const bfloat&) - ea: 0x940290
bint::bint(const bfloat& rhs) : mVal((int)rhs.mVal) {}

// bint::bint(float) - ea: 0x925170
bint::bint(float rhs) : mVal((int)rhs) {}

// bint::operator=(int) - IDA/C3 declaration
int bint::operator=(int rhs) {
    mVal = rhs;
    return mVal;
}

// bint::operator=(unsigned int) - IDA/C3 declaration
unsigned int bint::operator=(unsigned int rhs) {
    mVal = (int)rhs;
    return rhs;
}

// bint::operator+=(int) - IDA/C3 declaration
int bint::operator+=(int rhs) {
    mVal += rhs;
    return mVal;
}

// bint::operator int() - ea: 0x97D020 inline body
bint::operator int() const {
    AssertDefined();
    return mVal;
}

// bint::operator++() - ea: 0x97D020 inline body
int bint::operator++() {
    AssertDefined();
    return mVal++;
}

// bint::operator--() - ea: 0x97D020
int bint::operator--() {
    AssertDefined();
    return mVal--;
}

// bfloat::bfloat(long double) - ea: 0x9430C0
bfloat::bfloat(long double rhs) : mVal((float)rhs) {}

// bfloat::operator=(long double) - ea: 0x9774A0
double bfloat::operator=(long double rhs) {
    mVal = (float)rhs;
    return mVal;
}

// bfloat::operator!=(bfloat) - ea: 0x97AF10
bool bfloat::operator!=(const bfloat& rhs) const {
    return mVal != rhs.mVal;
}

// bfloat::operator+=(float) - ea: 0x97B580 inline body
double bfloat::operator+=(float rhs) {
    AssertDefined();
    mVal += rhs;
    return mVal;
}

// bfloat::operator-() - ea: 0x97AF60
bfloat bfloat::operator-() const {
    return bfloat(0.0f - mVal);
}

// operator*(int, bfloat) - ea: 0x940210
bfloat operator*(int lhs, bfloat rhs) {
    return bfloat((float)lhs * rhs.mVal);
}

// operator*(bint, float) - ea: 0x971F80
bfloat operator*(bint lhs, float rhs) {
    return bfloat(lhs.mVal * rhs);
}

// bint::operator*=(int) - ea: 0x977460
int bint::operator*=(int rhs) {
    AssertDefined();
    mVal *= rhs;
    return mVal;
}

// operator*(bint, bfloat) - ea: 0x95AD50
bfloat operator*(bint lhs, bfloat rhs) {
    float lhs_value = lhs.mVal;
    float value = rhs.mVal * lhs_value;
    return bfloat(value);
}

// operator+(bint, bfloat) - ea: 0x95ADA0
bfloat operator+(bint lhs, bfloat rhs) {
    float lhs_value = lhs.mVal;
    float value = rhs.mVal + lhs_value;
    return bfloat(value);
}

// operator+(bfloat, bfloat) - ea: 0x97AFB0
bfloat operator+(bfloat lhs, bfloat rhs) {
    return bfloat(lhs.mVal + rhs.mVal);
}

// operator<(bfloat, bfloat) - ea: 0x975FF0
bbool operator<(bfloat lhs, bfloat rhs) {
    return bbool(rhs.mVal > lhs.mVal);
}

// operator<(bfloat, int) - ea: 0x9774D0
bbool operator<(bfloat lhs, int rhs) {
    return bbool(rhs > lhs.mVal);
}

// bfloat::operator!= - ea: 0x95D300
bool bfloat::operator!=(float rhs) const {
    return mVal != rhs;
}

// bfloat::operator*= - ea: 0x95D660
double bfloat::operator*=(float rhs) {
    AssertDefined();
    mVal = mVal * rhs;
    return mVal;
}

void bfloat::AssertDefined() const {}

// bfloat::IsDefined - ea: 0x95AED0
bool bfloat::IsDefined() const {
    return IS_NAN(mVal) == 0;
}

// bint::operator=(float) - ea: 0x9540F0
int bint::operator=(float rhs) {
    mVal = rhs;
    return mVal;
}

// bint::operator=(bfloat) - ea: 0x95AE70
int bint::operator=(bfloat rhs) {
    if (rhs.IsDefined()) {
        mVal = (int)rhs.mVal;
        return mVal;
    }
    mVal = bint::sUndefined;
    return bint::sUndefined;
}

unsigned int HashStr::sUndefined = 0;
float bfloat::sUndefined = 0.0f;
int bint::sUndefined = 0;
bool bbool::sUndefined = false;

// operator*(bint, int) - ea: 0x93D8E0
bint operator*(bint lhs, int rhs) {
    return bint(lhs.mVal * rhs);
}

// operator*(bint, bint) - ea: 0x949AE0
bint operator*(bint lhs, bint rhs) {
    return bint(rhs.mVal * lhs.mVal);
}

// operator*(int, bint) - ea: 0x949B20
bint operator*(int lhs, bint rhs) {
    return bint(lhs * rhs.mVal);
}

// operator+(bint, bint) - ea: 0x943C50
bint operator+(bint lhs, bint rhs) {
    return bint(lhs.mVal + rhs.mVal);
}

// operator<(bfloat, bint) - ea: 0x949670
bbool operator<(bfloat lhs, bint rhs) {
    return bbool(rhs.mVal > lhs.mVal);
}

// operator<(bint, bfloat) - ea: 0x971FC0
bbool operator<(bint lhs, bfloat rhs) {
    return bbool(rhs.mVal > lhs.mVal);
}

// operator<(int, bint) - ea: 0x949A30
bbool operator<(int lhs, bint rhs) {
    return bbool(lhs < rhs.mVal);
}

// operator+(bint, int) - ea: 0x9370B0
Broc::bint operator+(Broc::bint lhs, int rhs) {
    return Broc::bint(lhs.mVal + rhs);
}

// operator<(bfloat, float) - ea: 0x9376C0
Broc::bbool operator<(Broc::bfloat lhs, float rhs) {
    return Broc::bbool(rhs > lhs.mVal);
}

// operator>(bfloat, float) - ea: 0x937710
Broc::bbool operator>(Broc::bfloat lhs, float rhs) {
    return Broc::bbool(lhs.mVal > rhs);
}

// operator>(bint, int) - ea: 0x93AD00
Broc::bbool operator>(Broc::bint lhs, int rhs) {
    return Broc::bbool(lhs.mVal > rhs);
}

// operator<(int, bfloat) - ea: 0x93AD80
Broc::bbool operator<(int lhs, Broc::bfloat rhs) {
    return Broc::bbool(rhs.mVal > lhs);
}

// operator<(float, bfloat) - ea: 0x93BDA0
Broc::bbool operator<(float lhs, Broc::bfloat rhs) {
    return Broc::bbool(rhs.mVal > lhs);
}

// operator+(int, bint) - ea: 0x9379F0
Broc::bint operator+(int lhs, Broc::bint rhs) {
    return Broc::bint(lhs + rhs.mVal);
}

// operator+(float, bfloat) - ea: 0x937E70
Broc::bfloat operator+(float lhs, Broc::bfloat rhs) {
    return Broc::bfloat(rhs.mVal + lhs);
}

// operator>(bfloat, int) - ea: 0x9389A0
Broc::bbool operator>(Broc::bfloat lhs, int rhs) {
    return Broc::bbool(lhs.mVal > rhs);
}

// Broc::operator*(vector, float) - ea: 0x937A20
Broc::vector Broc::operator*(const Broc::vector& lhs, float rhs) {
    return Broc::vector(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
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

// _mp_airplanes::plane_flyby_thread__functor - ea: 0x934BD0
void* plane_flyby_thread__functor(Broc::entity self, mp_plane plane_struct) {
    void* storage = AeThreadFunctor::operator new(
        sizeof(AeThreadFunctor2<Broc::entity, mp_plane>));
    if (storage == NULL)
        return NULL;
    return ::new (storage)
        AeThreadFunctor2<Broc::entity, mp_plane>(plane_flyby_thread,
                                                  self, plane_struct);
}

// _mp_airplanes::plane_flyby__functor - ea: 0x935100
void* plane_flyby__functor(Broc::entity self, mp_plane plane_struct,
                           Broc::bint num) {
    void* storage = AeThreadFunctor::operator new(
        sizeof(AeThreadFunctor3<Broc::entity, mp_plane, Broc::bint>));
    if (storage == NULL)
        return NULL;
    return ::new (storage)
        AeThreadFunctor3<Broc::entity, mp_plane, Broc::bint>(
            plane_flyby, self, plane_struct, num);
}

// ============================================================================
// _mp_airplanes::plane_flyby - ea: 0x9351C0
// ============================================================================
void plane_flyby(Broc::entity self, mp_plane plane_struct, Broc::bint num) {
    Broc::string inClassname("script_model");
    Broc::entity plane;
    Broc::vector* startOrg = &plane_struct.plane_start_orgs[(unsigned int)num];
    Broc::Spawn(&plane, &inClassname, startOrg, static_cast<TPakInfo>(0));
    inClassname.~string();
    Broc::SetModel(&plane, &plane_struct.plane_model, 0);
    Broc::vector* angles = &plane_struct.plane_angles[(unsigned int)num];
    Broc::entity::__unnamed::angles_struct planeAngles = {plane.GetHandle()};
    planeAngles = angles;
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
}

// ============================================================================
// _mp_airplanes::plane_roll - ea: 0x935680
// ============================================================================
void plane_roll(Broc::entity self) {
    HashStr endLabel;
    endLabel.mVal = 0x074AA0A6u;
    Broc::endon(self, endLabel);

    ::bfloat minRoll(-20.0L);
    ::bfloat maxRoll(20.0L);
    ::bfloat roll(0.0L);
    ::bfloat minTime(0.75L);
    ::bfloat maxTime(1.5L);
    ::bfloat time(0.0L);
    for (;;) {
        roll = (long double)RandomFloatRange((float)minRoll,
                                               (float)maxRoll);
        time = (long double)RandomFloatRange((float)minTime,
                                               (float)maxTime);
        float accTime = (float)time * 0.25f;
        float decTime = (float)time * 0.25f;
        Broc::vector dest(0.0f, 0.0f, (float)roll);
        Broc::RotateTo(&self, &dest, (float)time, accTime, decTime);
        Broc::wait((float)time);
    }
}
void* plane_roll__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(plane_roll, self);
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
        dir = ::AnglesToForward(angle);
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
        Broc::EffectEventPlay(sound, pos, facing);
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
                Broc::entity fMin =
                    mp_util_wad::pLevel != nullptr
                        ? mp_util_wad::pLevel->_base.entity
                        : Broc::entity();
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
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
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
                        Broc::entity lvl =
                            mp_util_wad::pLevel != nullptr
                                ? mp_util_wad::pLevel->_base.entity
                                : Broc::entity();
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
void* PlayerLocation__functor(Broc::entity self);
void* ambient_system__functor(Broc::entity lvl, Broc::string spawn_package);
void PlayKillerWarning(Broc::entity guy, Broc::entity inflictor,
                       Broc::entity attacker, Broc::bint weapon,
                       Broc::bint means_of_damage);

// SpawnLineSound (string start) - ea: 0x9380F0
Broc::entity* SpawnLineSound(Broc::entity* result, Broc::string startOfLineEntity,
                             Broc::string sound) {
    if (Broc::IsDefined(startOfLineEntity)) {
        HashStr key(0x19F9F0E8u);
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
        Broc::Spawn(&soundMover, &inClassname, &start,
                    static_cast<TPakInfo>(INVALID_PAK_INFO));
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
    Broc::entity player(players[0]);
    Broc::vector pos;
    Broc::bfloat closest_dist;
    for (;;) {
        if (!Broc::IsDefined(player)) {
            Broc::dyn_array<Broc::entity> entarr;
            Broc::GetPlayerArray(&entarr);
            player = entarr[0];
            entarr.~dyn_array();
        }
        Broc::vector porg;
        mp_util_wad::entity_get_origin(&porg, player);
        closest_point_on_line_to_point(porg, start, end, pos);
        mp_util_wad::entity_set_origin(toMove, pos);
        if (Broc::IsDefined(pos)) {
            float dist = ::DistanceSquared(porg, pos);
            closest_dist = dist;
            if (::operator>(closest_dist, 0x100000)) {
                Broc::wait(2.0f);
            } else if (::operator>(closest_dist, 0x40000)) {
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
            *Broc::Spawn(&spawnResult, &inClassname, &origin,
                         static_cast<TPakInfo>(INVALID_PAK_INFO));
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
            *Broc::Spawn(&spawnResult, &inClassname, &origin,
                         static_cast<TPakInfo>(INVALID_PAK_INFO));
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
            Broc::notify(mp_util_wad::pLevel->hack_sound_entity, label);
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
    Broc::Spawn(&temp_entity, &inClassname, &position,
                static_cast<TPakInfo>(INVALID_PAK_INFO));
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

// main - ea: 0x9359A0
void main() {
    Broc::string LevelBG;
    LevelBG = "";
    Broc::gBrocAPI.mBrocExports.mCallbackSetLevelAudio = CallbackSetLevelAudio;
    Broc::string name("Preset_Noreverb");
    Broc::ReverbSetParams(name, false);
    name.~string();
    if (!Broc::IsDefined(mp_util_wad::pLevel->background_track)) {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio.bro level.background_track is not defined. Setting to none"))
            __debugbreak();
        mp_util_wad::pLevel->background_track = "none";
    }
    if (!Broc::IsDefined(mp_util_wad::pLevel->reverb_setting)) {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio.bro level.reverb_setting is not defined. Setting to Preset_Noreverb"))
            __debugbreak();
        mp_util_wad::pLevel->reverb_setting = "Preset_Noreverb";
    }
    if (!Broc::IsDefined(mp_util_wad::pLevel->ambient_setting)) {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio.bro level.ambient_setting is not defined. Setting to NoAmbFX"))
            __debugbreak();
        mp_util_wad::pLevel->ambient_setting = "NoAmbFX";
    }
    if (!Broc::IsDefined(mp_util_wad::pLevel->audio_ambient_max) ||
        IS_NAN((float)mp_util_wad::pLevel->audio_ambient_max)) {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio.bro level.audio_ambient_max is not defined. Setting to 0.5"))
            __debugbreak();
        mp_util_wad::pLevel->audio_ambient_max = 0.5f;
    }
    if (!Broc::IsDefined(mp_util_wad::pLevel->audio_ambient_min) ||
        IS_NAN((float)mp_util_wad::pLevel->audio_ambient_min)) {
        if (Broc::gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\_mp_audio.bro", __LINE__,
                "_mp_audio.bro level.audio_ambmin is not defined. Setting to 5.0"))
            __debugbreak();
        mp_util_wad::pLevel->audio_ambient_min = 5.0f;
    }
    mp_util_wad::pLevel->audio_current_track_handle =
        (int)Broc::SoundPlay(mp_util_wad::pLevel->background_track, 1.0f);
    Broc::ReverbSetParams(mp_util_wad::pLevel->reverb_setting, false);
    mp_util_wad::pLevel->audio_current_ambpack =
        mp_util_wad::pLevel->ambient_setting;
    mp_util_wad::pLevel->audio_current_ambient_min =
        (float)mp_util_wad::pLevel->audio_ambient_min;
    mp_util_wad::pLevel->audio_current_ambient_wait =
        (float)mp_util_wad::pLevel->audio_ambient_max;
    mp_util_wad::pLevel->audio_indoor_switch = 0;
    mp_util_wad::pLevel->audio_change_priority = 0;
    mp_util_wad::pLevel->crossfade_done = 1;
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* playerLoc = PlayerLocation__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_audio.bro",
                        __LINE__, "PlayerLocation", playerLoc);
    Broc::entity lvl2 =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    Broc::string ambient = mp_util_wad::pLevel->ambient_setting;
    void* ambSys = ambient_system__functor(lvl2, ambient);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_audio.bro",
                        __LINE__, "ambient_system", ambSys);
    Broc::dyn_array<Broc::entity> BG_Triggers;
    Broc::string val("BG_Trigger");
    HashStr key;
    key.mVal = 0x19F9F0E8u;
    Broc::GetEntArray(&val, key.mVal, &BG_Triggers, 0);
    val.~string();
    Broc::bint i(0);
    while ((int)i < Broc::size(BG_Triggers)) {
        Broc::entity t = BG_Triggers[(unsigned int)(int)i];
        if (Broc::IsDefined(t)) {
            HashStr funcHash;
            Broc::string_hash(&funcHash, "_mp_audio::interior_triggering_device");
            HashStr label;
            label.mVal = 0xF2F5EAB4;
            Broc::AddEventHandler(&t, label.mVal, funcHash.mVal);
        }
        i = (int)i + 1;
    }
    BG_Triggers.~dyn_array();
    LevelBG.~string();
}

// PlayPainSound - ea: 0x93A640
void PlayPainSound(Broc::entity guy, Broc::bint damage) {
    if ((int)damage > 100)
        return;
    static Broc::bfloat block_sounds_for_several_seconds(0.5f);
    Broc::bbool hasLast;
    mp_util_wad::IsEEDefined_lastPainSoundTime(&hasLast, guy);
    if ((bool)hasLast) {
        Broc::bint last_sound((int)*mp_util_wad::GetEE_lastPainSoundTime(guy));
        last_sound += (int)((float)block_sounds_for_several_seconds * 1000.0f);
        Broc::bint now;
        Broc::GetTime(&now);
        if ((int)last_sound > (int)now)
            return;
    }
    Broc::bfloat chance(0.2f);
    Broc::bfloat additional_play_sound_chance(0.0f);
    Broc::bbool team_damage(false);
    Broc::bint mh;
    mp_util_wad::entity_get_maxhealth(&mh, guy);
    Broc::bfloat max_health((float)(int)mh);
    Broc::bfloat percent_of_total_health_damaged =
        ::operator>(max_health, 0.0001f) ? (float)(int)damage / (float)max_health : 0.5f;
    if ((float)percent_of_total_health_damaged < 0.25f)
        chance = 0.1f;
    else if ((float)percent_of_total_health_damaged < 0.5f)
        chance = 0.2f;
    else if ((float)percent_of_total_health_damaged < 0.75f)
        chance = 0.3f;
    else
        chance = 0.4f;
    chance = 1.0f;  // binary force: chance always 1.0
    if (Broc::RandomInt(100) < (int)(100.0f * (float)chance)) {
        Broc::string sound_to_play;
        sound_to_play = "";
        Broc::bbool allies(false);
        Broc::string team;
        mp_util_wad::entity_get_team(&team, guy);
        if (team == "allies")
            allies = true;
        team.~string();
        if ((float)percent_of_total_health_damaged < 0.25f) {
            sound_to_play = (bool)allies ? "AMERICAN_LIGHT_PAIN_SOUND"
                                        : "GERMAN_LIGHT_PAIN_SOUND";
        } else if ((float)percent_of_total_health_damaged < 0.5f) {
            sound_to_play = (bool)allies ? "AMERICAN_MEDIUM_PAIN_SOUND"
                                        : "GERMAN_MEDIUM_PAIN_SOUND";
        } else if ((float)percent_of_total_health_damaged < 0.75f) {
            sound_to_play = (bool)allies ? "AMERICAN_HEAVY_PAIN_SOUND"
                                        : "GERMAN_HEAVY_PAIN_SOUND";
        } else {
            sound_to_play = (bool)allies ? "AMERICAN_CRITICAL_PAIN_SOUND"
                                        : "GERMAN_CRITICAL_PAIN_SOUND";
        }
        Broc::EffectEventPlay(&guy, &sound_to_play);
        Broc::bint now;
        Broc::GetTime(&now);
        *mp_util_wad::GetEE_lastPainSoundTime(guy) = (int)now;
        sound_to_play.~string();
    }
}

// PlayKillerCredit - ea: 0x93BEE0
void PlayKillerCredit(Broc::entity killer) {
    if (Broc::IsPlayer(killer) == 0)
        return;
    Broc::bint state;
    mp_util_wad::entity_get_playerState(&state, killer);
    if ((int)state != 3)
        return;
    Broc::string sound_to_play;
    sound_to_play = "";
    Broc::bint have_sound(0);
    Broc::string team;
    if ((int)*mp_util_wad::GetEE_killsSinceLastDeath(killer) == 5) {
        mp_util_wad::entity_get_team(&team, killer);
        sound_to_play = team == "allies" ? "american_five_kills"
                                        : "german_five_kills";
        have_sound = 1;
    } else if ((int)*mp_util_wad::GetEE_killsSinceLastDeath(killer) == 10) {
        mp_util_wad::entity_get_team(&team, killer);
        sound_to_play = team == "allies" ? "american_ten_kills"
                                        : "german_ten_kills";
        have_sound = 1;
    }
    if ((int)*mp_util_wad::GetEE_killsSinceLastDeath(killer) == 20) {
        mp_util_wad::entity_get_team(&team, killer);
        sound_to_play = team == "allies" ? "american_twenty_kills"
                                        : "german_twenty_kills";
        have_sound = 1;
    } else if ((int)have_sound == 0) {
        sound_to_play.~string();
        return;
    }
    team.~string();
    Broc::EffectEventPlay(&killer, &sound_to_play);
    sound_to_play.~string();
}

// PlayDeathSound - ea: 0x93AE80
void PlayDeathSound(Broc::entity guy, Broc::entity inflictor,
                    Broc::entity attacker, Broc::bint weapon,
                    Broc::bint means_of_damage) {
    PlayKillerWarning(guy, inflictor, attacker, weapon, means_of_damage);
    PlayKillerCredit(attacker);
    Broc::bfloat chance(1.0f);
    Broc::bfloat additional_play_sound_chance(0.0f);
    Broc::bbool team_damage(false);
    bool attackerDefined = Broc::IsDefined(attacker);
    if (attackerDefined && Broc::Code_IsLocalPlayer(attacker)) {
        Broc::string at;
        Broc::string gt;
        mp_util_wad::entity_get_team(&at, attacker);
        mp_util_wad::entity_get_team(&gt, guy);
        if (at == gt)
            team_damage = true;
        at.~string();
        gt.~string();
        additional_play_sound_chance = 0.1f;
    }
    if (Broc::Code_IsLocalPlayer(guy)) {
        if ((int)means_of_damage == 25)
            return;
        additional_play_sound_chance = 0.1f;
    }
    chance = (float)chance + (float)additional_play_sound_chance;
    if (Broc::RandomInt(100) < (int)(100.0f * (float)chance)) {
        Broc::string sound_to_play;
        sound_to_play = "";
        (void)team_damage;
        Broc::string team;
        mp_util_wad::entity_get_team(&team, guy);
        sound_to_play = team == "allies" ? "american_scream"
                                        : "german_scream";
        team.~string();
        Broc::EffectEventPlay(&guy, &sound_to_play);
        sound_to_play.~string();
    }
}

// PlayKillerWarning - ea: 0x93B270
void PlayKillerWarning(Broc::entity guy, Broc::entity inflictor,
                       Broc::entity attacker, Broc::bint weapon,
                       Broc::bint means_of_damage) {
    Broc::bfloat chance(0.25f);
    if ((int)weapon == 0 || (int)means_of_damage == 11 ||
        !Broc::Code_GetTeamGame() || Broc::IsPlayer(attacker) == 0)
        return;
    Broc::string gt;
    Broc::string at;
    mp_util_wad::entity_get_team(&gt, guy);
    mp_util_wad::entity_get_team(&at, attacker);
    bool teamKill = at == gt;
    gt.~string();
    at.~string();
    if (teamKill)
        return;
    if (Broc::RandomInt(100) >= (int)(100.0f * (float)chance))
        return;
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::entity talker;
    talker.___u0 = 0;
    Broc::bfloat distance(800.0f);
    Broc::bfloat distance_sqr((float)distance * (float)distance);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, p);
        if ((int)state == 3) {
            Broc::string pteam;
            Broc::string gteam;
            mp_util_wad::entity_get_team(&pteam, p);
            mp_util_wad::entity_get_team(&gteam, guy);
            bool diffTeam = pteam != gteam;
            pteam.~string();
            gteam.~string();
            if (!diffTeam) {
                Broc::vector porg;
                Broc::vector gorg;
                mp_util_wad::entity_get_origin(&porg, p);
                mp_util_wad::entity_get_origin(&gorg, guy);
                float dsq = ::DistanceSquared(porg, gorg);
                if (dsq < (float)distance_sqr) {
                    talker = p;
                    break;
                }
            }
        }
        i = (int)i + 1;
    }
    if (!Broc::IsDefined(talker)) {
        players.~dyn_array();
        return;
    }
    Broc::string sound_to_play;
    sound_to_play = "";
    if (Broc::IsVehicle(inflictor) != 0) {
        if ((int)means_of_damage == 20 || (int)means_of_damage == 21 ||
            (int)means_of_damage == 22 || (int)means_of_damage == 31) {
            sound_to_play.~string();
            players.~dyn_array();
            return;
        }
        Broc::string vt = *mp_util_wad::GetEE_vehicletype(inflictor);
        if (stricmp(vt.c_str(), "mp_shermantank") == 0 ||
            stricmp(vt.c_str(), "mp_panzeriv") == 0) {
            Broc::string ateam;
            mp_util_wad::entity_get_team(&ateam, attacker);
            sound_to_play = ateam == "allies" ? "american_tank"
                                             : "german_tank";
            ateam.~string();
        } else if (stricmp(vt.c_str(), "mp_wc51") == 0 ||
                   stricmp(vt.c_str(), "mp_horch") == 0) {
            sound_to_play.~string();
            vt.~string();
            players.~dyn_array();
            return;
        }
        vt.~string();
    } else {
        Broc::string weaponName;
        weaponName = "";
        Broc::Code_GetWeaponName((int)weapon, weaponName);
        Broc::string ateam;
        mp_util_wad::entity_get_team(&ateam, attacker);
        if (weaponName == "kar98_sniper" || weaponName == "springfield") {
            sound_to_play = ateam == "allies" ? "american_sniper"
                                             : "german_sniper";
        } else if (weaponName == "mg34" || weaponName == "mg30cal") {
            sound_to_play = ateam == "allies" ? "american_lmg"
                                             : "german_lmg";
        } else if (weaponName == "panzerschreck" || weaponName == "bazooka") {
            sound_to_play = ateam == "allies" ? "american_bazooka"
                                             : "german_bazooka";
        } else {
            sound_to_play = ateam == "allies" ? "american_warning"
                                             : "german_warning";
        }
        ateam.~string();
        weaponName.~string();
    }
    Broc::EffectEventPlay(&talker, &sound_to_play);
    sound_to_play.~string();
    players.~dyn_array();
}
}

const Broc::bint* Broc::entity::__unnamed::key_struct::Get(
    Broc::bint* result) const {
    new (result) Broc::bint(Broc::gBrocAPI.m_entity_get_key(mHandle));
    return result;
}

// model_struct::Get - ea: 0x970800
const Broc::string* Broc::entity::__unnamed::model_struct::Get(
    Broc::string* result) const {
    Broc::string value;
    Broc::string* rhs = Broc::gBrocAPI.m_entity_get_model(&value, mHandle);
    new (result) Broc::string(*rhs);
    value.~string();
    return result;
}

// takedamage_struct::Get - ea: 0x9712A0
const Broc::bint* Broc::entity::__unnamed::takedamage_struct::Get(
    Broc::bint* result) const {
    new (result) Broc::bint(Broc::gBrocAPI.m_entity_get_takedamage(mHandle));
    return result;
}

// rotate_struct::Get - ea: 0x9713A0
const Broc::vector* Broc::entity::__unnamed::rotate_struct::Get(
    Broc::vector* result) const {
    Broc::vector value;
    *result = *Broc::gBrocAPI.m_entity_get_rotate(&value, mHandle);
    return result;
}

// GetEE - ea: 0x925470
const Broc::ExtendedEntity* Broc::entity::GetEE() const {
    Broc::ExtendedEntity* ee =
        Broc::ExtendedEntity::GetExtendedEntity(___u0);
    return ee != nullptr ? ee : &Broc::ExtendedEntity::nullEnt;
}

// LocalEE - ea: 0x9587D0
mp_util_wad::LocalFields* Broc::entity::LocalEE() const {
    const Broc::ExtendedEntity* ee = GetEE();
    return reinterpret_cast<mp_util_wad::LocalFields*>(
        const_cast<Broc::ExtendedEntity*>(ee));
}

// operator-> - ea: 0x958790
mp_util_wad::LocalFields* Broc::entity::operator->() {
    return LocalEE();
}

// trigger_struct::operator= - ea: 0x9587F0
// flag_struct::GetRef - ea: 0x976AB0
Broc::entity& mp_util_wad::LocalFields::__unnamed::flag_struct::GetRef() {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    return ee->GetRef<Broc::entity>(reinterpret_cast<unsigned int>(&s_flagKey));
}

// trigger_struct::operator= - ea: 0x9587F0
const Broc::entity* mp_util_wad::LocalFields::__unnamed::trigger_struct::operator=(
    const Broc::entity* rhs) {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    ee->SetVal<Broc::entity>(0xF2F5EAB4u, *rhs);
    return rhs;
}

// trigger_struct::Get - ea: 0x958830
Broc::entity* mp_util_wad::LocalFields::__unnamed::trigger_struct::Get(
    Broc::entity* result) {
    const Broc::ExtendedEntity* ee =
        reinterpret_cast<const Broc::ExtendedEntity*>(
            reinterpret_cast<const unsigned char*>(this) - 0x10);
    ee->GetVal<Broc::entity>(result, 0xF2F5EAB4u);
    return result;
}

// trigger_struct::GetRef - ea: 0x976AE0
Broc::entity& mp_util_wad::LocalFields::__unnamed::trigger_struct::GetRef() {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    return ee->GetRef<Broc::entity>(0xF2F5EAB4u);
}

// capStatus_struct::operator= - ea: 0x976B10
const float&
mp_util_wad::LocalFields::__unnamed::capStatus_struct::operator=(
    const float& rhs) {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    ::bfloat value(rhs);
    ee->SetVal<::bfloat>(0x53377998u, value);
    return rhs;
}

// capTeam_struct::operator= - ea: 0x976B60
const int&
mp_util_wad::LocalFields::__unnamed::capTeam_struct::operator=(
    const int& rhs) {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    ::bint value(rhs);
    ee->SetVal<::bint>(0xAF35F29Bu, value);
    return rhs;
}

// capTeam_struct::GetRef - ea: 0x978110
::bint& mp_util_wad::LocalFields::__unnamed::capTeam_struct::GetRef() {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    return ee->GetRef<::bint>(0xAF35F29Bu);
}

// capAllowedTeam_struct::operator= - ea: 0x97BB10
const int&
mp_util_wad::LocalFields::__unnamed::capAllowedTeam_struct::operator=(
    const int& rhs) {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    ::bint value(rhs);
    ee->SetVal<::bint>(0xF430CD43u, value);
    return rhs;
}

// capStatus_struct::GetRef - ea: 0x978330
::bfloat& mp_util_wad::LocalFields::__unnamed::capStatus_struct::GetRef() {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    return ee->GetRef<::bfloat>(0x53377998u);
}

// holder_struct::GetRef - ea: 0x968040
Broc::entity& mp_util_wad::LocalFields::__unnamed::holder_struct::GetRef() {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    return ee->GetRef<Broc::entity>(0xFAAE111Eu);
}

// home_position_struct::GetRef - ea: 0x969EB0
Broc::vector& mp_util_wad::LocalFields::__unnamed::home_position_struct::GetRef() {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    return ee->GetRef<Broc::vector>(0xE42FE83Du);
}

// home_angles_struct::GetRef - ea: 0x96AE80
Broc::vector& mp_util_wad::LocalFields::__unnamed::home_angles_struct::GetRef() {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    return ee->GetRef<Broc::vector>(0x385575E2u);
}

// pickupCaptureDelayTime_struct::Get - ea: 0x96AC20
const Broc::bint*
mp_util_wad::LocalFields::__unnamed::pickupCaptureDelayTime_struct::Get(
    Broc::bint* result) const {
    const Broc::ExtendedEntity* ee = reinterpret_cast<const Broc::ExtendedEntity*>(
        reinterpret_cast<const unsigned char*>(this) - 0x10);
    Broc::bint value;
    const Broc::bint* stored =
        ee->GetVal<Broc::bint>(&value, 0x4743187Eu);
    result->mVal = stored->mVal;
    return result;
}

// pickupCaptureDelayTime_struct::operator= - ea: 0x9697E0
const int&
mp_util_wad::LocalFields::__unnamed::pickupCaptureDelayTime_struct::operator=(
    const int& rhs) {
    Broc::ExtendedEntity* ee = reinterpret_cast<Broc::ExtendedEntity*>(
        reinterpret_cast<unsigned char*>(this) - 0x10);
    Broc::bint value(rhs);
    ee->SetVal<Broc::bint>(0x4743187Eu, value);
    return rhs;
}

const int& Broc::entity::__unnamed::key_struct::operator=(
    const int& rhs) {
    Broc::gBrocAPI.m_entity_set_key(mHandle, rhs);
    return rhs;
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
Broc::string* entity_get_model(Broc::string* result, Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_model(result, ent.___u0);
}
Broc::vector* entity_get_rotate(Broc::vector* result, Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_rotate(result, ent.___u0);
}
int entity_get_takedamage(Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_takedamage(ent.___u0);
}
__int16 entity_get_ctf_has_flag(Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_player_ctf_has_flag(ent.___u0);
}
void entity_set_ctf_has_flag(Broc::entity ent, __int16 v) {
    Broc::gBrocAPI.m_entity_set_player_ctf_has_flag(ent.___u0, v);
}
int entity_get_key(Broc::entity ent) {
    return Broc::gBrocAPI.m_entity_get_key(ent.___u0);
}
void entity_set_key(Broc::entity ent, int key) {
    Broc::gBrocAPI.m_entity_set_key(ent.___u0, key);
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

// _effect hash_map helpers (opaque until the runtime is ported).
void level_effect_set(HashStr key, const Broc::string& val) {
    (void)key;
    (void)val;
}
Broc::string* level_effect_get(Broc::string* result, HashStr key) {
    (void)key;
    result->mBlock = NULL;
    return result;
}
}

// ============================================================================
// _mp_dm - deathmatch script.
// ============================================================================
namespace _mp_dm {
namespace _mp_common {
void SetupCallbacks(Broc::bbool teamGameType);
}
AeThreadFunctor1<Broc::entity>* StartGame__functor(Broc::entity self);
extern AeThreadFunctor1<Broc::entity>* main__functor(Broc::entity self);
extern Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* ent,
                                   const Broc::string* spawnpoint);
void StartGame(Broc::entity self);
void main(Broc::entity self);

// main__functor - ea: 0x93CC70
AeThreadFunctor1<Broc::entity>* main__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(main, self);
}

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

// StartGame__functor - ea: 0x9566B0
AeThreadFunctor1<Broc::entity>* StartGame__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(StartGame, self);
}

// StartGame - ea: 0x956750
void StartGame(Broc::entity self) {
    (void)self;
    ::_mp_common::StartRound(Broc::bbool(true));
    Broc::entity selfLevel =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    AeThreadFunctor1<Broc::entity>* ftor =
        ::_mp_common::RunFrame__functor(selfLevel);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_dm.bro",
                        __LINE__, "_mp_common::RunFrame", ftor);
    Broc::Code_EnterGame();
}

// GetSpawnPoint - ea: 0x9567F0
Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* self,
                            const Broc::string* team) {
    Broc::dyn_array<Broc::entity> spawnpoints;
    Broc::entity spawnpoint;
    Broc::string spawnType = mp_util_wad::pLevel->spawnTypeAllies;
    if (*team == "axis")
        spawnType = mp_util_wad::pLevel->spawnTypeAxis;
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&spawnType, key.mVal, &spawnpoints, 0);
    Broc::entity selected;
    spawnpoint.___u0 =
        ::_mp_spawnlogic::GetSpawnpointDM(&selected, self, &spawnpoints)->___u0;
    new (result) Broc::entity(spawnpoint);
    return result;
}
}

// ============================================================================
// Sibling gametype namespaces - declarations used by _mp_common::LaunchGametype.
// Definitions (stubs until those scripts are ported) live at the bottom.
// ============================================================================
namespace _mp_tankdrive { void main(); }
namespace _mp_tankdrive {
void init_all_tanks();
void init_tank(Broc::entity self);
void fire(Broc::entity self);
void BlowUpIfUnderWorld(Broc::entity self);
void BlowUpIfFlipped(Broc::entity self);
Broc::bint* VehicleRespawnClear(Broc::bint* result, Broc::entity self,
                                Broc::vector selfPos);
void HostSafeVehicleRespawn(Broc::entity self);
void death(Broc::entity self, Broc::entity attacker);
void damage(Broc::entity self, Broc::bint damage, Broc::entity attacker,
            Broc::bint mod);
void VehicleDamagedEffects(Broc::entity self);
void inactivity_blowup(Broc::entity self);
void local_player_hit_effects(Broc::entity self, Broc::bint damage,
                              Broc::bint mod);
void bigsplash(Broc::entity self);
void fireydeath(Broc::entity self, Broc::entity tank);
void deleteonextinguish(Broc::entity self);
void setup_effects(Broc::entity self);
void* BlowUpIfUnderWorld__functor(Broc::entity self);
void* BlowUpIfFlipped__functor(Broc::entity self);
void* VehicleDamagedEffects__functor(Broc::entity self);
void* deleteonextinguish__functor(Broc::entity self);
void* death__functor(Broc::entity self, Broc::entity attacker);
void* inactivity_blowup__functor(Broc::entity self);
void* fire__functor(Broc::entity self);
}
namespace _mp_nano { void main(); }
namespace _mp_nano {
void WindBlowing(Broc::entity self);
void* WindBlowing__functor(Broc::entity self);
unsigned int CreateGlobalWind(Broc::vector direction, Broc::bfloat speed);
}
namespace _mp_audio { void main(); }
namespace _mp_minefield {
void main(Broc::entity self);
void minefield_think(Broc::entity self);
void minefield_kill(Broc::entity self, Broc::entity trigger);
void* main__functor(Broc::entity self);
void* minefield_think__functor(Broc::entity self);
void* minefield_kill__functor(Broc::entity self, Broc::entity trigger);
}
namespace _mp_tdm {
void main(Broc::entity self);
void CallbackPlayerKilled(Broc::entity player, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health);
void StartGame(Broc::entity self);
void* main__functor(Broc::entity self);
void* StartGame__functor(Broc::entity self);
}
namespace _mp_ctf { void* main__functor(Broc::entity self); }
namespace _mp_scf { void* main__functor(Broc::entity self); }
namespace _mp_war { void* main__functor(Broc::entity self); }
namespace _mp_war {

static ::bfloat lWARObjectiveHeight(125.0f);
Broc::vector* vLerp(Broc::vector* result, Broc::vector a, Broc::vector b,
                    Broc::bfloat t);
void main(Broc::entity self);
void StartGame(Broc::entity self);
void PlayCapturedSounds(Broc::entity self, Broc::bint team,
                        Broc::bint originalWarIndex);
void WARScore(Broc::entity self);
void GiveTeamMembersPoints(Broc::vector origin, Broc::bfloat radius,
                           Broc::string team);
void CallbackHostMigrated();
void CallbackAreaCaptured(int index, int team);
void CallbackHostOptionsChanged(int forceMapChange);
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int roundOver,
                       int roundCount);
void CallbackPlayerKilled(Broc::entity player, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health);
void CallbackGameStateDOM(int flag0, int flag1, int flag2, int flag3,
                          int flag4);
void CallbackNextRound();
void SetupRound();
int CallbackGetFlagCount();
int CallbackGetFlagBreatherTime();
int CallbackGetTeamControllingFlag(unsigned int flagIndex);
int CallbackGetFlagBeingCaptured();
int CallbackGetTeamCapturingFlag();
int CallbackGetCapturingFlagPercent();
int CallbackGetFlagBeingContested(Broc::entity player);
int GetNumPlayersContestingFlag();
void CallbackPlayerJoin(Broc::entity player, unsigned int playerState,
                        int playerClass);
void CallbackPlayerEnter(Broc::entity player, int hot_joiner);
void CallbackRoundOver(int condition, Broc::string team);
void SendFlagStates(Broc::entity player);
void WAR_Init(Broc::entity self);
void WAR_FlagUpdate(Broc::entity self);
void WAR_TouchFlag(Broc::entity self);
    ::bfloat* GetCapSpeed(::bfloat* result, ::bint guysCapping);
void UpdateAllowedCap();
void AllowCap(::bint team, int flag_id);
void WAR_InitFlag(Broc::entity self, int flag_id);
void SortMarkers();
Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* self,
                            const Broc::string* self_team);
void WarnPlayerAboutInactiveFlag(Broc::entity self);
void CallbackDebugRender();
void DebugRenderSpawnPoints();
void* main__functor(Broc::entity self);
void* StartGame__functor(Broc::entity self);
void* WARScore__functor(Broc::entity self);
void* WAR_Init__functor(Broc::entity self);
void* WAR_InitFlag__functor(Broc::entity self, int flag_id);
void* WAR_FlagUpdate__functor(Broc::entity self);
void* WAR_TouchFlag__functor(Broc::entity self);
void* WarnPlayerAboutInactiveFlag__functor(Broc::entity self);
void* PlayCapturedSounds__functor(Broc::entity self, Broc::bint team,
                                  Broc::bint originalWarIndex);
}
namespace _mp_hq { void* main__functor(Broc::entity self); }
namespace _mp_hq {
void main(Broc::entity self);
void StartGame(Broc::entity self);
unsigned int Host_ResetStage1();
void Host_FlowControl(Broc::entity self);
void AddPoints(Broc::bint forAllies, Broc::bint forAxis);
void NoRespawnForDefenders(Broc::bbool no_respawn);
void HQ_Destroyed();
void HQ_Defended();
char Host_PickInitialPoints();
void SetupStage1();
void SetUpStage3();
int ShowHQDestroyed();
int ShowHQDefended();
int ShowHQSetUp();
void ShowSetupGraphic(Broc::entity guy);
void ShowDestructionGraphic(Broc::entity guy);
void ShowLosingHQGraphic(Broc::entity guy);
void ShowProgressBar(Broc::entity self, Broc::string colour);
void TriggerRadio(Broc::entity self);
void SwitchToRadioOnly();
void AddRadioModel();
void RemoveRadioModel();
void ClearGame();
void ResetGame(Broc::entity self);
Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* self,
                            const Broc::string* team);
void GetTriggerFromIndex();
void BroadcastGameState();
void SyncScores();
void Track_Ownership(Broc::entity self);
void String_Timer();
void CallbackGameStateHQ(unsigned int stage, Broc::vector vA, Broc::vector vB,
                         unsigned int triggerIndex, int alliesDefending,
                         int pointAIsHQ);
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int roundOver,
                       int roundCount);
void CallbackNextRound();
void CallbackRoundOver(int condition, Broc::string team);
void CallbackPlayerJoin(Broc::entity player, unsigned int playerState,
                        int playerClass);
void CallbackPlayerEnter(Broc::entity player, int hot_joiner);
void CallbackPlayerKilled(Broc::entity player, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health);
void CallbackPlayerLeave(Broc::entity player);
unsigned int CallbackHostMigrated();
void CallbackHostOptionsChanged(int forceMapChange);
int CallbackGetTeamCapturingHQPercent(Broc::entity my_player);
int CallbackGetTeamDestroyingHQPercent();
int CallbackGetHQCaptureStatus();
void CallbackDebugRender();
Broc::bint* compare(Broc::bint* result, Broc::vector* first,
                    Broc::vector* second);
int SortPoints();
Broc::bfloat* GetCapSpeed(Broc::bfloat* result, Broc::bint guysCapping);
void* main__functor(Broc::entity self);
AeThreadFunctor1<Broc::entity>* StartGame__functor(Broc::entity self);
AeThreadFunctor1<Broc::entity>* Host_FlowControl__functor(Broc::entity self);
AeThreadFunctor* Track_Ownership__functor(Broc::entity self);
AeThreadFunctor1<Broc::entity>* ResetGame__functor(Broc::entity self);
void* TriggerRadio__functor(Broc::entity self);
}
namespace _mp_scf {
void main(Broc::entity self);
void StartGame(Broc::entity self);
void SetupRound();
int IsFlagAlive(Broc::entity flag);
void ObjectiveUpdater(Broc::entity guy);
void CallbackAreaCaptured(int index, int team);
void SubRoundEnd();
void CallbackPlayerSpawn(Broc::entity player, int team_changed);
void CallbackHostMigrated();
void CallbackNextRound();
void CallbackHostOptionsChanged(int forceMapChange);
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int roundOver,
                       int roundCount);
void CallbackPlayerKilled(Broc::entity player, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health);
void CallbackPlayerLeave(Broc::entity player);
void CallbackGameStateSCF(int currentFlagIndex, Broc::vector FlagOrigin,
                          Broc::vector FlagAngles, Broc::entity FlagHolder);
int CallbackShowFlagHint();
void UpdateFlagAndTrigger(Broc::entity self, Broc::vector origin,
                          Broc::vector angles, Broc::vector velocity);
void LaunchFlagAndTrigger(Broc::entity self, Broc::vector origin,
                          Broc::vector angles, Broc::vector velocity);
void EntityOff(Broc::entity self);
void CompassUnderlay(Broc::entity p);
AeThreadFunctor1<Broc::entity>* CompassUnderlay__functor(Broc::entity p);
Broc::bbool* IsFlagAtBase(Broc::bbool* result, Broc::entity flag);
void HandlePickupFlag(Broc::entity flag, Broc::entity pickerupper, int request,
                      int playSounds);
void CallbackPickupScriptItem(int netID, Broc::entity guy, int itemIndex);
void CallbackDropItem(int netID, int entity, Broc::vector position,
                      Broc::vector angles, Broc::vector velocity);
void CallbackDropFlag(Broc::entity player);
void HandleDropFlag(Broc::entity player);
void WaitForFlagTimeOut(Broc::entity flag);
void PickupFlag(Broc::entity self, Broc::entity triggerer);
void Goal(Broc::entity self, Broc::entity triggerer);
void WaitThenPickFlagToLaunch(Broc::entity self, Broc::bfloat wait_time,
                              const char* message);
void PickFlagToLaunch();
void LaunchFlag(Broc::bint flag, Broc::vector position, Broc::vector angles,
                Broc::vector velocity);
void InitializeFlags();
int ResetFlags();
void CallbackPlayerJoin(Broc::entity player, unsigned int playerState,
                        int playerClass);
void CallbackPlayerEnter(Broc::entity player, int hot_joiner);
void UnlinkFlag(Broc::entity flag);
void DestroyIcon(Broc::entity toucher);
void WaitForNoTouchFlag(Broc::entity toucher);
void RenderFlagInfo(Broc::entity flag, Broc::bint x, Broc::bint y);
void CallbackDebugRender();
void* main__functor(Broc::entity self);
AeThreadFunctor1<Broc::entity>* StartGame__functor(Broc::entity self);
void* ObjectiveUpdater__functor(Broc::entity guy);
AeThreadFunctor3<Broc::entity, Broc::bfloat, const char*>*
WaitThenPickFlagToLaunch__functor(Broc::entity self,
                                  Broc::bfloat wait_time,
                                  const char* message);
void* WaitForFlagTimeOut__functor(Broc::entity flag);
void* WaitForNoTouchFlag__functor(Broc::entity toucher);
void* PickupFlag__functor(Broc::entity self, Broc::entity triggerer);
void* Goal__functor(Broc::entity self, Broc::entity triggerer);
}
namespace _mp_ctf {
void main(Broc::entity self);
void StartGame(Broc::entity self);
void CallbackAreaCaptured(int index, int team);
void CallbackPlayerSpawn(Broc::entity guy, int team_changed);
void SetupRound();
void CallbackHostMigrated();
void CallbackNextRound();
void CallbackHostOptionsChanged(int forceMapChange);
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int roundOver,
                       int roundCount);
int CallbackShowFlagHint();
void SwitchToSecondarySpawns(Broc::entity self);
Broc::vector* vLerp(Broc::vector* result, Broc::vector a, Broc::vector b,
                    Broc::bfloat t);
Broc::bbool* IsFlagAtBase(Broc::bbool* result, Broc::entity flag);
void TeamObjectiveUpdate(Broc::entity guy, int iTeamFlag);
void ObjectiveUpdater(Broc::entity guy);
void CallbackPlayerKilled(Broc::entity guy, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health);
void CallbackDropFlag(Broc::entity guy);
void CallbackPlayerLeave(Broc::entity guy);
void CallbackGameStateCTF(Broc::vector allied_flag, Broc::vector allied_angles,
                          Broc::entity allied_flag_holder, Broc::vector axis_flag,
                          Broc::vector axis_angles, Broc::entity axis_flag_holder);
void HandleDropFlag(Broc::entity guy);
void UpdateFlagAndTrigger(Broc::entity self, Broc::vector origin,
                          Broc::vector angles);
void LaunchFlagAndTrigger(Broc::entity self, Broc::vector origin,
                          Broc::vector angles, Broc::vector velocity);
void EntityOff(Broc::entity self);
void CompassUnderlay(Broc::entity p);
void HandlePickupFlag(int netID, Broc::entity pickerupper, Broc::bbool playSounds,
                      Broc::bbool giveStats);
void CallbackPickupScriptItem(int netID, Broc::entity guy, int itemIndex);
void CallbackDropItem(int netID, int entity, Broc::vector position,
                      Broc::vector angles, Broc::vector velocity);
void WaitForFlagTimeOut(Broc::entity flag);
void DestroyIcon(Broc::entity toucher);
void WaitForNoTouchFlag(Broc::entity toucher);
void PickupFlag(Broc::entity self);
void Goal(Broc::entity self);
void FlagThreadLauncher(Broc::entity self);
void CallbackPlayerJoin(Broc::entity guy, unsigned int playerState,
                        int playerClass);
void CallbackPlayerEnter(Broc::entity guy, int hot_joiner);
void UnlinkFlag(Broc::entity flag);
void RenderFlagInfo(Broc::entity guy, Broc::bint flagID, Broc::bint flagTeam);
void CallbackDebugRender();
Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* self,
                            const Broc::string* team);
void* main__functor(Broc::entity self);
AeThreadFunctor1<Broc::entity>* StartGame__functor(Broc::entity self);
AeThreadFunctor1<Broc::entity>* FlagThreadLauncher__functor(Broc::entity self);
AeThreadFunctor1<Broc::entity>* SwitchToSecondarySpawns__functor(Broc::entity self);
AeThreadFunctor1<Broc::entity>* ObjectiveUpdater__functor(Broc::entity guy);
AeThreadFunctor1<Broc::entity>* CompassUnderlay__functor(Broc::entity p);
AeThreadFunctor1<Broc::entity>* WaitForFlagTimeOut__functor(Broc::entity flag);
AeThreadFunctor1<Broc::entity>* WaitForNoTouchFlag__functor(Broc::entity toucher);
void* PickupFlag__functor(Broc::entity self);
void* Goal__functor(Broc::entity self);
}

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
                          const Broc::vector* point, int damage, int mod,
                          int weapon, int hitLoc);       // ea: 0x93F970
void CallbackPlayerDamageTeam(Broc::entity victim, Broc::entity inflictor,
                              Broc::entity attacker, const Broc::vector* damageDir,
                              const Broc::vector* point, int damage, int mod,
                              int weapon, int hitLoc);   // ea: 0x93FA90
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
void CallbackReviveFailed(Broc::entity ent);              // ea: 0x943A00
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
void NewHost(Broc::entity self);                         // ea: 0x944F40
void FadeUpWhenLoaded(Broc::entity self, Broc::entity player);  // ea: 0x944F90
void RespawnPlayer(Broc::entity guy, Broc::string team);  // ea: 0x9450D0
void Spectate(Broc::entity self, Broc::bint target, Broc::bbool isIntermission);  // ea: 0x945350
void DeathState(Broc::entity self, Broc::entity inflictor, Broc::bint weapon,
                Broc::bbool isPlayerKill, Broc::bbool isPlayerDamage);  // ea: 0x945420
void FollowClient(Broc::entity self, Broc::bint index);  // ea: 0x9459F0
void LocalPlayerIntermission(Broc::entity self);         // ea: 0x945BC0
void LocalPlayerRespawn(Broc::entity self);              // ea: 0x945D30
void RunFrame(Broc::entity selfLevel);                   // ea: 0x946040
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
void QuitGameThread(Broc::entity selfLevel);             // ea: 0x9495C0
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
    return mp_util_wad::pLevel->respawnTime = Broc::GetCvarInt("mp_respawntime");
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
        launch_gametype_thread(
            gametype, "_mp_dm::main",
            reinterpret_cast<void* (*)(Broc::entity)>(_mp_dm::main__functor));
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
    Broc::Code_IncTeamScore(team, alliesScore);
    team.~string();
    Broc::string team2("axis");
    Broc::Code_IncTeamScore(team2, axisScore);
    team2.~string();
}

// CallbackPlayerLeave - ea: 0x93E970
void CallbackPlayerLeave(Broc::entity leavingPlayer) {
    HashStr label;
    label.mVal = 0x4C745BCAu;  // "disconnect"
    Broc::notify(leavingPlayer, label);
    if ((bool)mp_util_wad::pLevel->roundStarted &&
        !(bool)mp_util_wad::pLevel->roundOver) {
        mp_util_wad::pLevel->playersLeavingDuringRound =
            (int)mp_util_wad::pLevel->playersLeavingDuringRound + 1;
    }
    if (!Broc::Code_IsLocalPlayer(leavingPlayer)) {
        const char* name = Broc::Code_GetPlayerName(leavingPlayer);
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
    if (occupantCount > 0 && Broc::IsPlayer(attacker) != 0) {
        AddToPlayerStats(attacker, Broc::bint(6), 1);
        Broc::entity spotter;
        spotter = Broc::Code_GetSpotterEntity(killedVehicle);
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
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
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
void CallbackReviveFailed(Broc::entity ent) {
    (void)ent;
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
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = StopFollowing__functor(lvl, true);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "StopFollowing", ftor);
}

// CallbackQuitGame - ea: 0x944580
unsigned int CallbackQuitGame() {
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = QuitGameThread__functor(lvl);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "QuitGameThread", ftor);
}

// CallbackHostDisconnected - ea: 0x9446A0
unsigned int CallbackHostDisconnected() {
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr msg;
    msg.mVal = 0xA45844A6;
    void* ftor = QuitGameWithMessage__functor(lvl, msg);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "QuitGameWithMessage", ftor);
}

// CallbackLocalPlayerKicked - ea: 0x944730
unsigned int CallbackLocalPlayerKicked() {
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr msg;
    msg.mVal = 0x3546BB54u;
    void* ftor = QuitGameWithMessage__functor(lvl, msg);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "QuitGameWithMessage", ftor);
}

// CallbackHostMigrated - ea: 0x9447C0
unsigned int CallbackHostMigrated() {
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = HostHasMigrated__functor(lvl);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                               __LINE__, "HostHasMigrated", ftor);
}

// CallbackSpawnButtonPressed - ea: 0x944440
void CallbackSpawnButtonPressed(Broc::entity ent) {
    if (Broc::Code_IsLocalPlayer(ent)) {
        HashStr label;
        label.mVal = 0xC39A4465;
        Broc::notify(ent, label);
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
    Broc::notify(player, label);
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
    if (Broc::IsPlayer(dropper) == 0)
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
    Broc::Code_IncTeamScore(team, alliesScore);
    team.~string();
    Broc::string team2("axis");
    Broc::Code_IncTeamScore(team2, axisScore);
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
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        void* ftor = HandleJoinAfterRoundOver__functor(lvl, timeTillNextRound);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "HandleJoinAfterRoundOver", ftor);
    }
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr label;
    label.mVal = 0x531AD8D9u;
    Broc::notify(lvl, label);
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
        int axisScore = Broc::Code_GetTeamScore(team);
        int alliesScore = Broc::Code_GetTeamScore(team2);
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
void RunFrame(Broc::entity selfLevel) {
    (void)selfLevel;
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
        int alliesScore = Broc::Code_GetTeamScore(team);
        int axisScore = Broc::Code_GetTeamScore(team2);
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
        Broc::Code_RoundOver((int)condition, winner);
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
    if (Broc::IsDefined<Broc::hudelem>(elem)) {
        Destroy(elem);
        elem->SetUndefined();
    }
}

// ProgressBarCreate - ea: 0x94AA80
void ProgressBarCreate(Broc::bint width) {
    Broc::hudelem tmp;
    mp_util_wad::pLevel->progress_bar = *NewHudElem(&tmp, -1);
    mp_util_wad::pLevel->progress_bar.x = 160;
    mp_util_wad::pLevel->progress_bar.y = 360;
    mp_util_wad::pLevel->progress_bar.sort = 0.0f;
    mp_util_wad::pLevel->progress_bar.alpha = 0x80;
    Broc::string s("white");
    SetShader(&mp_util_wad::pLevel->progress_bar, &s, (int)width, 32);
    s.~string();
}

// ProgressBarUpdate - ea: 0x94AD50
void ProgressBarUpdate(Broc::bfloat value, Broc::bfloat time) {
    int w = (int)((float)value * 320.0f);
    float scaleTime = (float)time;
    ScaleOverTime(&mp_util_wad::pLevel->progress_bar, scaleTime, w, 32);
}

// ProgressBarDelete - ea: 0x94AE00
void ProgressBarDelete() {
    DestroyHudElem(&mp_util_wad::pLevel->progress_bar);
}

// QuitGameThread - ea: 0x9495C0
void QuitGameThread(Broc::entity selfLevel) {
    (void)selfLevel;
    Broc::Code_ScreenFadeToBlack(0xFAu, -1);
    Broc::wait(0.25f);
    Broc::Code_QuitGame();
}

// NewHost - ea: 0x944F40
void NewHost(Broc::entity self) {
    (void)self;
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
        Broc::SetTakeDamage(player, 1);
        Broc::vector origin;
        mp_util_wad::entity_get_origin(&origin, player);
        Broc::vector down(0.0f, 0.0f, -5.0f);
        Broc::vector vecIn = origin + down;
        Broc::DoDamage(player, 10000.0f, vecIn, HITLOC_NONE);
        Broc::wait(0.1f);
    }
}

// GetWinningTeam - ea: 0x94C4A0
Broc::string* GetWinningTeam(Broc::string* result) {
    Broc::string team("allies");
    Broc::bint allies_score(Broc::Code_GetTeamScore(team));
    team.~string();
    Broc::string team2("axis");
    Broc::bint axis_score(Broc::Code_GetTeamScore(team2));
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
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
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
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
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
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
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
            HashStr msg;
            msg.mVal = 0xF2554CB4;
            void* ftor = QuitGameWithMessage__functor(ent, msg);
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
    Broc::notify(player, label);
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
        Broc::Code_PlayerSpawn(player, origin, angles, 1);
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
    int axisScore = Broc::Code_GetTeamScore(team);
    int alliesScore = Broc::Code_GetTeamScore(team2);
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
    Broc::notify(player, label);
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
        Broc::notify(player, label2);
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
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
    Broc::notify(player, label);
    if (Broc::IsPlayer(medic) != 0)
        AddToPlayerStats(medic, Broc::bint(11), 1);
    Broc::string weapon("mp_revive");
    Broc::Code_Obituary(player, medic, &weapon, 1, 0);
    weapon.~string();
    if (Broc::Code_IsLocalPlayer(player)) {
        Broc::string menu("weapon");
        Broc::CloseMenu(menu, Broc::GetPlayerIndex(player));
        menu.~string();
        Broc::string menu2("spectate");
        Broc::CloseMenu(menu2, Broc::GetPlayerIndex(player));
        menu2.~string();
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        void* ftor = FadeUpWhenLoaded__functor(lvl, player);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "FadeUpWhenLoaded", ftor);
        if (Broc::Code_GetTeamGame()) {
            HashStr label2;
            label2.mVal = 0xC9444C8D;
            Broc::notify(player, label2);
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
    _mp_shellshock::ShellshockOnDamage(player, ::bint(mod), ::bint(damage));
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
        Broc::IsDefined(attacker) && Broc::IsPlayer(attacker) != 0 &&
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
        }
    }
}

// CallbackPlayerKilled - ea: 0x93EC10
void CallbackPlayerKilled(Broc::entity killedPlayer, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health) {
    Broc::string weaponName((const char*)NULL);
    Broc::Code_GetWeaponName(weapon, weaponName);
    Broc::entity team_killer;
    team_killer.___u0 = 0;
    if (attacker.IsDefined() && Broc::IsPlayer(attacker) != 0) {
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
                spotter = Broc::Code_GetSpotterEntity(killedPlayer);
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
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr label;
    label.mVal = 0x863B4D44;
    Broc::notify(lvl, label);
    Broc::dyn_array<Broc::entity> local_players;
    Broc::GetLocalPlayerArray(&local_players);
    Broc::bint i(0);
    while ((int)i < Broc::size(local_players)) {
        Broc::entity p = local_players[(unsigned int)(int)i];
        int idx = Broc::GetPlayerIndex(p);
        Broc::string menu("side_select");
        Broc::CloseMenu(menu, idx);
        menu.~string();
        Broc::string menu2("weapon");
        Broc::CloseMenu(menu2, idx);
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
            Broc::Code_IncTeamScore(t, 1);
            t.~string();
        } else if (team == "allies") {
            Broc::string t("allies");
            Broc::Code_IncTeamScore(t, 1);
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
    Broc::entity lvl2 =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
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
                Broc::entity lvl =
                    mp_util_wad::pLevel != nullptr
                        ? mp_util_wad::pLevel->_base.entity
                        : Broc::entity();
                Broc::string sound("incoming");
                void* ftor =
                    _mp_audio::PlaySoundAtLocation__functor(lvl, sound, position);
                Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                                    __LINE__, "_mp_audio::PlaySoundAtLocation",
                                    ftor);
                played_incoming_sound = true;
            }
            Broc::entity lvl =
                mp_util_wad::pLevel != nullptr
                    ? mp_util_wad::pLevel->_base.entity
                    : Broc::entity();
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
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr label;
    label.mVal = 0xAFE7EFF4;
    Broc::notify(lvl, label);
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
            Broc::OpenMenu(menu, Broc::GetPlayerIndex(p));
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
    Broc::entity lvl2 =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = finish_starting_round__functor(lvl2, (bool)firstTime);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "finish_starting_round", ftor);
}

// finish_starting_round - ea: 0x946630
void finish_starting_round(Broc::entity self, Broc::bbool firstTime) {
    (void)self;
    Broc::Code_DebugOut("*COMMON* finish_starting_round\n");
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr e1;
    e1.mVal = 0x863B4D44;
    Broc::endon(lvl, e1);
    Broc::entity lvl2 =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr e2;
    e2.mVal = 0x531AD8D9u;
    Broc::endon(lvl2, e2);
    if (Broc::GetCvarInt("mp_debug") == 0 &&
        ((bool)mp_util_wad::pLevel->lastManStanding ||
         (bool)mp_util_wad::pLevel->mustHaveBothTeamsToStart)) {
        Broc::bbool both;
        HasBothTeams(&both);
        if (!(bool)both)
            WaitForTeams();
    }
    Broc::entity lvl3 =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr n;
    n.mVal = 0x4DEC2E76u;
    Broc::notify(lvl3, n);
    Broc::wait(0.1f);
    Broc::entity lvl4 =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    Broc::endon(lvl4, n);
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
        int axisScore = Broc::Code_GetTeamScore(team);
        int alliesScore = Broc::Code_GetTeamScore(team2);
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
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr fade;
    fade.mVal = 0x2A9ACF98u;
    Broc::waittill(lvl, fade);
    if (Broc::Code_GetTeamGame()) {
        MusicStop();
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
            Broc::entity lvl2 =
                mp_util_wad::pLevel != nullptr
                    ? mp_util_wad::pLevel->_base.entity
                    : Broc::entity();
            mp_util_wad::pLevel->roundEndMusic =
                Broc::EffectEventPlay(&lvl2, &script);
            script.~string();
            Broc::string dialog("MP_GEN_AlliesWin");
            Broc::entity lvl3 =
                mp_util_wad::pLevel != nullptr
                    ? mp_util_wad::pLevel->_base.entity
                    : Broc::entity();
            Broc::DialogPlay(lvl3, &dialog);
            dialog.~string();
        } else if (mp_util_wad::pLevel->roundWinner == "axis") {
            Broc::string script("MX_MPVictory_Axis");
            Broc::entity lvl2 =
                mp_util_wad::pLevel != nullptr
                    ? mp_util_wad::pLevel->_base.entity
                    : Broc::entity();
            mp_util_wad::pLevel->roundEndMusic =
                Broc::EffectEventPlay(&lvl2, &script);
            script.~string();
            Broc::string dialog("MP_GEN_AxisWin");
            Broc::entity lvl3 =
                mp_util_wad::pLevel != nullptr
                    ? mp_util_wad::pLevel->_base.entity
                    : Broc::entity();
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
        SoundStop((unsigned int)(int)musicHandle);
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
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        HashStr n1;
        n1.mVal = 0xA8743279;
        Broc::notify(lvl, n1);
        Broc::wait(0.1f);
        Broc::entity lvl2 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        HashStr n2;
        n2.mVal = 0x4DEC2E76u;
        Broc::endon(lvl2, n2);
        Broc::entity lvl3 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
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
    Broc::notify(player, n);
    *mp_util_wad::GetEE_specialWeaponTime(player) = 0;
    Broc::wait(0.25f);
    SpawnIntermission(player);
    Broc::wait(0.1f);
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = FadeUpWhenLoaded__functor(lvl, player);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                        __LINE__, "FadeUpWhenLoaded", ftor);
}

// LocalPlayerRespawn - ea: 0x945D30
void LocalPlayerRespawn(Broc::entity player) {
    Broc::string menu("spectate");
    Broc::CloseMenu(menu, Broc::GetPlayerIndex(player));
    menu.~string();
    if (mp_util_wad::entity_get_nextPlayerClass(player) == -1) {
        Broc::TakeAllWeapons(player);
        Broc::string side("side_select");
        Broc::OpenMenu(side, Broc::GetPlayerIndex(player));
        side.~string();
        HashStr w1;
        w1.mVal = 0xE7D05EDA;
        Broc::waittill(player, w1);
        Broc::string weapon("weapon");
        Broc::OpenMenu(weapon, Broc::GetPlayerIndex(player));
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
                Broc::entity lvl =
                    mp_util_wad::pLevel != nullptr
                        ? mp_util_wad::pLevel->_base.entity
                        : Broc::entity();
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
    Broc::notify(guy, n);
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
            Broc::string menu("");
            bool done = (int)respawnTime < (int)now &&
                        Broc::Code_IsMenuOpen(menu, playerIndex) != 0;
            if (done)
                break;
            Broc::bint t;
            Broc::GetTime(&t);
            int seconds = ((int)respawnTime - (int)t + 999) / 1000;
            Broc::SetSpectateSeconds(seconds, playerIndex);
            Broc::string empty("");
            if (Broc::Code_IsMenuOpen(empty, playerIndex) != 0) {
                Broc::string spec("spectate");
                Broc::OpenMenu(spec, playerIndex);
            }
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
        Broc::OpenMenu(menu, Broc::GetPlayerIndex(player));
        menu.~string();
        Broc::SetSpectateState(0, Broc::GetPlayerIndex(player));
    }
    Broc::wait((float)(int)delay);
    Broc::bint state;
    mp_util_wad::entity_get_playerState(&state, player);
    if ((int)state != 2) {
        HashStr n;
        n.mVal = 0xC1E6FED9;
        Broc::notify(player, n);
        mp_util_wad::entity_set_spectatorClient(player, -1);
        mp_util_wad::entity_set_health(player, 0);
        if (Broc::IsDefined(team_killer))
            Broc::SetSpectateTeamKill(1, team_killer,
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
    Broc::Code_PlayerRespawn(guy, origin, angles, new_team);
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
        Broc::notify(guy, n);
        bool firstSpectate = (bool)*mp_util_wad::GetEE_firstSpectate(guy);
        if (!firstSpectate) {
            Broc::Code_ScreenFadeToBlack(0xFAu, local_player_index);
            Broc::wait(0.25f);
        }
        SpawnSpectator(guy);
        if (Broc::Code_IsLocalPlayer(guy)) {
            Broc::wait(0.1f);
            if (!firstSpectate) {
                Broc::entity lvl =
                    mp_util_wad::pLevel != nullptr
                        ? mp_util_wad::pLevel->_base.entity
                        : Broc::entity();
                HashStr fade;
                fade.mVal = 0x2A9ACF98u;
                Broc::waittill(lvl, fade);
                Broc::Code_ScreenFadeUp(0xFAu, local_player_index);
            }
            Broc::string menu("spectate");
            Broc::OpenMenu(menu, local_player_index);
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
    Broc::Code_PlayerSpawn(guy, origin, angles, 0);
}

// SpawnIntermission - ea: 0x949200
void SpawnIntermission(Broc::entity player) {
    Broc::Code_DebugOut("*COMMON* SpawnIntermission\n");
    HashStr n;
    n.mVal = 0x24B5BA64u;
    Broc::notify(player, n);
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
    Broc::Code_PlayerSpawn(player, origin, angles, stopPhysics);
    if (Broc::Code_IsLocalPlayer(player)) {
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        void* ftor = FadeUpWhenLoaded__functor(lvl, player);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_common.bro",
                            __LINE__, "FadeUpWhenLoaded", ftor);
        Broc::string menu("spectate");
        Broc::OpenMenu(menu, Broc::GetPlayerIndex(player));
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
    Broc::notify(ent, "end_HealthRegenPlayerBreathing");
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
    ObjectiveAdd((int)index, state, pszString, position, "1");
    state.~string();
    pszString.~string();
    Broc::wait(15.0f);
    ObjectiveDelete((int)index, -1);
}

// HasValidWeaponInSlot - ea: 0x94BE20
Broc::bbool* HasValidWeaponInSlot(Broc::bbool* result, Broc::entity player,
                                  Broc::string slot) {
    int ammo = Broc::GetWeaponSlotAmmo(player, slot);
    int clip = Broc::GetWeaponSlotClipAmmo(player, slot);
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
                Broc::GetWeaponSlotWeapon(player, *slots[i], weapon);
                Broc::SwitchToWeapon(player, weapon);
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
        int axisScore = Broc::Code_GetTeamScore(team);
        int alliesScore = Broc::Code_GetTeamScore(team2);
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
        Broc::Code_RoundOver(0, winner);
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

void DebugRenderSpawnPoints() {
    if (Broc::GetCvarInt("mp_debugrenderspawnpoints") == 0)
        return;

    Broc::dyn_array<Broc::entity> spawnpoints;
    Broc::vector mins(-16.0f, -16.0f, 0.0f);
    Broc::vector maxs(16.0f, 16.0f, 72.0f);

    HashStr alliesKey(0xF756C677u);
    Broc::GetEntArray(&mp_util_wad::pLevel->spawnTypeAllies,
                     alliesKey.mVal, &spawnpoints, 0);
    Broc::vector origin;
    Broc::bint i(0);
    while ((int)i < Broc::size(spawnpoints)) {
        Broc::entity spawnpoint =
            spawnpoints[(unsigned int)(int)i];
        Broc::entity::__unnamed::origin_struct originField = {
            spawnpoint.GetHandle()};
        originField.Get(&origin);
        Broc::vector maxBox = origin + maxs;
        Broc::vector minBox = origin + mins;
        Broc::Code_DebugRenderBox(&minBox, &maxBox,
                                  &mp_util_wad::pLevel->spawnColorAllies,
                                  0.25f);
        i.operator++();
    }

    HashStr axisKey(0xF756C677u);
    Broc::GetEntArray(&mp_util_wad::pLevel->spawnTypeAxis,
                     axisKey.mVal, &spawnpoints, 0);
    Broc::bint axisIndex(0);
    while ((int)axisIndex < Broc::size(spawnpoints)) {
        Broc::entity spawnpoint =
            spawnpoints[(unsigned int)(int)axisIndex];
        Broc::entity::__unnamed::origin_struct originField = {
            spawnpoint.GetHandle()};
        originField.Get(&origin);
        Broc::vector maxBox = origin + maxs;
        Broc::vector minBox = origin + mins;
        Broc::Code_DebugRenderBox(&minBox, &maxBox,
                                  &mp_util_wad::pLevel->spawnColorAxis,
                                  0.25f);
        axisIndex.operator++();
    }

    spawnpoints.~dyn_array();
}

// StopFollowing - ea: 0x94AA60 (no-op body in the release binary)
void StopFollowing(Broc::entity self, Broc::bbool blackNow) {
    (void)self;
    (void)blackNow;
}
}

// ============================================================================
// Sibling gametype stubs (real implementations arrive with each script port).
// ============================================================================
// ============================================================================
// _mp_tankdrive - vehicle/tank behavior.
// ============================================================================
namespace _mp_tankdrive {

// main - ea: 0x970140
void main() {
    init_all_tanks();
}

// init_all_tanks - ea: 0x970160
void init_all_tanks() {
    Broc::dyn_array<Broc::entity> vehicles;
    Broc::string val("script_vehicle");
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&val, key.mVal, &vehicles, 0);
    val.~string();
    Broc::SetMaxVehicles(Broc::size(vehicles));
    Broc::bint i(0);
    while ((int)i < Broc::size(vehicles)) {
        init_tank(vehicles[(unsigned int)(int)i]);
        i = (int)i + 1;
    }
    vehicles.~dyn_array();
}

// init_tank - ea: 0x9702F0
void init_tank(Broc::entity self) {
    Broc::string model;
    mp_util_wad::entity_get_model(&model, self);
    *mp_util_wad::GetEE_respawnmodel(self) = model;
    model.~string();
    Broc::string deathmodel;
    mp_util_wad::entity_get_model(&deathmodel, self);
    Broc::bint length(deathmodel.length());
    if ((int)length > 3) {
        Broc::string lastthree =
            deathmodel.substr((unsigned int)((int)length - 3), 3);
        if (lastthree == "_mp") {
            Broc::string trimmed =
                deathmodel.substr(0, (unsigned int)((int)length - 3));
            deathmodel = trimmed;
            trimmed.~string();
        }
        lastthree.~string();
    }
    Broc::string dm = deathmodel + "_d1";
    *mp_util_wad::GetEE_deathmodel(self) = dm;
    dm.~string();
    deathmodel.~string();
    *mp_util_wad::GetEE_inDeath(self) = 0;
    Broc::bbool hasDmg;
    mp_util_wad::IsEEDefined_damage_effect(&hasDmg, self);
    if ((bool)hasDmg) {
        Broc::EffectEventStopEmitting(
            (int)*mp_util_wad::GetEE_damage_effect(self));
        *mp_util_wad::GetEE_damage_effect(self) = 0;
    }
    void* underworld = BlowUpIfUnderWorld__functor(self);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_tankdrive.bro",
                        __LINE__, "BlowUpIfUnderWorld", underworld);
    void* flipped = BlowUpIfFlipped__functor(self);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_tankdrive.bro",
                        __LINE__, "BlowUpIfFlipped", flipped);
    void* dmgFx = VehicleDamagedEffects__functor(self);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_tankdrive.bro",
                        __LINE__, "VehicleDamagedEffects", dmgFx);
    HashStr fireHash;
    Broc::string_hash(&fireHash, "_mp_tankdrive::fire");
    HashStr fireLabel;
    fireLabel.mVal = 0xCFD2C98B;
    Broc::AddEventHandler(&self, fireLabel.mVal, fireHash.mVal);
    HashStr deathHash;
    Broc::string_hash(&deathHash, "_mp_tankdrive::death");
    HashStr deathLabel;
    deathLabel.mVal = 0x74AA0A6u;
    Broc::AddEventHandler(&self, deathLabel.mVal, deathHash.mVal);
    HashStr damageHash;
    Broc::string_hash(&damageHash, "_mp_tankdrive::damage");
    HashStr damageLabel;
    damageLabel.mVal = 0xF05C975F;
    Broc::AddEventHandler(&self, damageLabel.mVal, damageHash.mVal);
    HashStr inactHash;
    Broc::string_hash(&inactHash, "_mp_tankdrive::inactivity_blowup");
    HashStr inactLabel;
    inactLabel.mVal = 0x92219DE6;
    Broc::AddEventHandler(&self, inactLabel.mVal, inactHash.mVal);
    setup_effects(self);
}

// fire - ea: 0x970A90
void fire(Broc::entity self) {
    Broc::bint health;
    mp_util_wad::entity_get_health(&health, self);
    if ((int)health > 0)
        Broc::FireTurret(&self);
}

// BlowUpIfUnderWorld - ea: 0x970B10
void BlowUpIfUnderWorld(Broc::entity self) {
    Broc::vector selfPos;
    for (;;) {
        Broc::wait(1.0f);
        if (Broc::IsLocalHost() && (bool)mp_util_wad::pLevel->roundStarted) {
            Broc::bint health;
            mp_util_wad::entity_get_health(&health, self);
            if ((int)health > 0) {
                Broc::vector pos;
                Broc::GetOrigin(&pos, &self);
                selfPos = pos;
                if (selfPos.z < -1000.0f) {
                    Broc::entity lvl =
                        mp_util_wad::pLevel != nullptr
                            ? mp_util_wad::pLevel->_base.entity
                            : Broc::entity();
                    Broc::DoDamage(lvl, 10000.0f, selfPos, HITLOC_NONE);
                }
            }
        }
    }
}

// BlowUpIfFlipped - ea: 0x970CC0
void BlowUpIfFlipped(Broc::entity self) {
    Broc::vector selfPos;
    for (;;) {
        do {
            HashStr label;
            label.mVal = 0xAF4CBA04;
            Broc::waittill(self, label);
        } while (!(bool)mp_util_wad::pLevel->roundStarted);
        Broc::bint health;
        mp_util_wad::entity_get_health(&health, self);
        if ((int)health > 0) {
            if (Broc::IsVehicleFlipped(&self)) {
                Broc::string deathfire = *mp_util_wad::GetEE_deathfire(self);
                Broc::bint firefxid((int)Broc::EffectEventPlay(&self, &deathfire));
                deathfire.~string();
                Broc::wait(10.0f);
                Broc::EffectEventStopEmitting((int)firefxid);
                if (Broc::IsLocalHost()) {
                    Broc::vector pos;
                    Broc::GetOrigin(&pos, &self);
                    selfPos = pos;
                    Broc::bint health2;
                    mp_util_wad::entity_get_health(&health2, self);
                    Broc::DoDamage(self, (float)((int)health2 + 100),
                                   selfPos, HITLOC_NONE);
                }
            }
        }
    }
}

// VehicleRespawnClear - ea: 0x970EB0
Broc::bint* VehicleRespawnClear(Broc::bint* result, Broc::entity self,
                                Broc::vector selfPos) {
    Broc::dyn_array<Broc::entity> vehicles;
    Broc::dyn_array<Broc::entity> players;
    Broc::vector pos;
    Broc::string val("script_vehicle");
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&val, key.mVal, &vehicles, 0);
    val.~string();
    Broc::bint i(0);
    while ((int)i < Broc::size(vehicles)) {
        Broc::entity v = vehicles[(unsigned int)(int)i];
        if (!(v == self) &&
            mp_util_wad::entity_get_takedamage(v) != 0) {
            Broc::vector origin;
            Broc::GetOrigin(&origin, &v);
            pos.x = origin.x;
            pos.y = origin.y;
            pos.z = selfPos.z;
            if (Broc::Distance(&pos, &selfPos) < 280.0f) {
                result->mVal = 0;
                players.~dyn_array();
                vehicles.~dyn_array();
                return result;
            }
        }
        i = (int)i + 1;
    }
    Broc::string val2("player");
    HashStr key2;
    key2.mVal = 0xF756C677;
    Broc::GetEntArray(&val2, key2.mVal, &players, 0);
    val2.~string();
    i = 0;
    while ((int)i < Broc::size(players)) {
        Broc::vector origin;
        Broc::GetOrigin(&origin, &players[(unsigned int)(int)i]);
        pos.x = origin.x;
        pos.y = origin.y;
        pos.z = selfPos.z;
        if (Broc::Distance(&pos, &selfPos) < 210.0f) {
            result->mVal = 0;
            players.~dyn_array();
            vehicles.~dyn_array();
            return result;
        }
        i = (int)i + 1;
    }
    result->mVal = 1;
    players.~dyn_array();
    vehicles.~dyn_array();
    return result;
}

// HostSafeVehicleRespawn - ea: 0x9712E0
void HostSafeVehicleRespawn(Broc::entity self) {
    Broc::vector respawn_origin;
    for (;;) {
        Broc::vector rotate;
        mp_util_wad::entity_get_rotate(&rotate, self);
        Broc::bint clear;
        VehicleRespawnClear(&clear, self, rotate);
        if ((int)clear != 0)
            break;
        Broc::wait(2.0f);
    }
    Broc::Code_RespawnVehicle(&self);
    Broc::SetTakeDamage(self, 1);
    Broc::Code_BroadcastVehicleRespawn(self);
}

// death - ea: 0x971450
void death(Broc::entity self, Broc::entity attacker) {
    if ((int)*mp_util_wad::GetEE_inDeath(self) == 0) {
        *mp_util_wad::GetEE_inDeath(self) = 1;
        Broc::bbool hasDmg;
        mp_util_wad::IsEEDefined_damage_effect(&hasDmg, self);
        if ((bool)hasDmg) {
            Broc::EffectEventStopEmitting(
                (int)*mp_util_wad::GetEE_damage_effect(self));
            *mp_util_wad::GetEE_damage_effect(self) = 0;
        }
        Broc::string deathmodel = *mp_util_wad::GetEE_deathmodel(self);
        Broc::SetModel(&self, &deathmodel, 0);
        deathmodel.~string();
        Broc::string deathfx = *mp_util_wad::GetEE_deathfx(self);
        Broc::bint deathfxid((int)Broc::EffectEventPlay(&self, &deathfx));
        deathfx.~string();
        Broc::string deathfire = *mp_util_wad::GetEE_deathfire(self);
        Broc::bint firefxid((int)Broc::EffectEventPlay(&self, &deathfire));
        deathfire.~string();
        Broc::vector origin;
        mp_util_wad::entity_get_origin(&origin, self);
        Broc::RadiusDamageFromEnt(&attacker, &origin, 512.0f, 100.0f, 1.0f,
                                  27);
        Broc::wait(10.0f);
        Broc::EffectEventStopEmitting((int)firefxid);
        Broc::string deathfx2 = *mp_util_wad::GetEE_deathfx(self);
        Broc::EffectEventPlay(&self, &deathfx2);
        deathfx2.~string();
        Broc::vector origin2;
        mp_util_wad::entity_get_origin(&origin2, self);
        Broc::RadiusDamage(&origin2, 512.0f, 100.0f, 1.0f, 27);
        Broc::wait(0.2f);
    Broc::SetTakeDamage(self, 0);
        Broc::wait(1.0f);
        Broc::string respawnmodel = *mp_util_wad::GetEE_respawnmodel(self);
        Broc::SetModel(&self, &respawnmodel, 0);
        respawnmodel.~string();
        Broc::wait(20.0f);
        if (Broc::IsLocalHost()) {
            HostSafeVehicleRespawn(self);
        } else {
            for (;;) {
                Broc::bint health;
                mp_util_wad::entity_get_health(&health, self);
                if ((int)health > 0)
                    break;
                if (Broc::IsLocalHost()) {
                    HostSafeVehicleRespawn(self);
                    break;
                }
                Broc::wait(1.0f);
            }
        }
        *mp_util_wad::GetEE_inDeath(self) = 0;
    }
}

// damage - ea: 0x971800
void damage(Broc::entity self, Broc::bint damage, Broc::entity attacker,
            Broc::bint mod) {
    Broc::bint health;
    mp_util_wad::entity_get_health(&health, self);
    if ((int)health + (int)damage > 0) {
        local_player_hit_effects(self, damage, mod);
    }
    (void)attacker;
}

// VehicleDamagedEffects - ea: 0x971890
void VehicleDamagedEffects(Broc::entity self) {
    Broc::bint maxh;
    mp_util_wad::entity_get_maxhealth(&maxh, self);
    Broc::bfloat health_50((int)maxh * 0.5f);
    Broc::bint maxh2;
    mp_util_wad::entity_get_maxhealth(&maxh2, self);
    Broc::bfloat health_25((int)maxh2 * 0.25f);
    Broc::bint maxh3;
    mp_util_wad::entity_get_maxhealth(&maxh3, self);
    Broc::bfloat health_10((int)maxh3 * 0.1f);
    Broc::bfloat health_inactivity(1.1f);
    Broc::bint old_health;
    mp_util_wad::entity_get_health(&old_health, self);
    Broc::bint damage(0);
    for (;;) {
        while (1) {
            Broc::bint health;
            mp_util_wad::entity_get_health(&health, self);
            if ((int)health >= 0)
                break;
            Broc::wait(1.0f);
        }
        _mp_common::waitframe();
        Broc::bint health;
        mp_util_wad::entity_get_health(&health, self);
        if ((float)(int)health >= (float)health_50) {
            Broc::bbool hasDmg;
            mp_util_wad::IsEEDefined_damage_effect(&hasDmg, self);
            if ((bool)hasDmg) {
                Broc::EffectEventStopEmitting(
                    (int)*mp_util_wad::GetEE_damage_effect(self));
                *mp_util_wad::GetEE_damage_effect(self) = 0;
            }
        }
        if ((int)health >= (int)old_health)
            continue;
        damage = (int)old_health - (int)health;
        if ((int)health <= 0)
            break;
        Broc::bbool need_to_change(false);
        Broc::string effect;
        effect = "";
        bool crossed = false;
        if ((float)health < (float)health_inactivity &&
            (float)(int)old_health >= (float)health_inactivity) {
            need_to_change = true;
            effect = *mp_util_wad::GetEE_deathfire(self);
        } else if ((float)health < (float)health_10 &&
                   (float)(int)old_health >= (float)health_10) {
            need_to_change = true;
            effect = *mp_util_wad::GetEE_damagecritical(self);
        } else if ((float)health < (float)health_25 &&
                   (float)(int)old_health >= (float)health_25) {
            need_to_change = true;
            effect = *mp_util_wad::GetEE_damageheavy(self);
        } else if ((float)health < (float)health_50 &&
                   (float)(int)old_health >= (float)health_50) {
            need_to_change = true;
            effect = *mp_util_wad::GetEE_damagelight(self);
        }
        (void)crossed;
        if ((bool)need_to_change) {
            Broc::bbool hasDmg;
            mp_util_wad::IsEEDefined_damage_effect(&hasDmg, self);
            if ((bool)hasDmg) {
                Broc::EffectEventStopEmitting(
                    (int)*mp_util_wad::GetEE_damage_effect(self));
                *mp_util_wad::GetEE_damage_effect(self) = 0;
            }
            *mp_util_wad::GetEE_damage_effect(self) =
                Broc::EffectEventPlay(&self, &effect);
        }
        old_health = (int)health;
        effect.~string();
    }
    (void)damage;
}

// inactivity_blowup - ea: 0x972020
void inactivity_blowup(Broc::entity self) {
    Broc::wait(10.0f);
    Broc::vector origin;
    mp_util_wad::entity_get_origin(&origin, self);
    Broc::bint health;
    mp_util_wad::entity_get_health(&health, self);
    Broc::DoDamage(self, (float)((int)health + 100), origin, HITLOC_NONE);
}

// local_player_hit_effects - ea: 0x9720A0
void local_player_hit_effects(Broc::entity self, Broc::bint damage,
                              Broc::bint mod) {
    Broc::bfloat local_damage_scalar((float)(int)damage / 500.0f);
    if ((float)local_damage_scalar > 1.0f)
        local_damage_scalar = 1.0f;
    static Broc::bfloat sTankDamageMin(0.85f);
    static Broc::bfloat sTankDamageMax(1.25f);
    static Broc::bfloat sTankDamageLength(
        Broc::operator*(Broc::bfloat(0.7f), (float)local_damage_scalar));
    static Broc::bfloat sTankDamageLowRumbleLength(
        Broc::operator*(Broc::bfloat(1.0f), (float)local_damage_scalar));
    static Broc::bfloat sTankDamageHighRumbleLength(
        Broc::operator*(Broc::bfloat(0.5f), (float)local_damage_scalar));
    Broc::bint i(0);
    while ((int)i < 3) {
        Broc::entity occupant;
        Broc::Code_GetPlayerInSeat(&occupant, self, (int)i);
        if (Broc::IsDefined(occupant)) {
            if (Broc::Code_IsLocalPlayer(occupant)) {
                int player_index = Broc::GetPlayerIndex(occupant);
                Broc::string name("IMPACT_PLAYER_TANK_HIT");
                Broc::SoundPlay(name, 1.0f);
                name.~string();
                int m = (int)mod;
                if (m == 3 || m == 4 || m == 7 || m == 8 || m == 9 ||
                    m == 10 || m == 5 || m == 6 || m == 17 || m == 18 ||
                    m == 27) {
                    Broc::vector source;
                    mp_util_wad::entity_get_origin(&source, occupant);
                    float fMax = Broc::RandomFloatRange(
                        (float)sTankDamageMin, (float)sTankDamageMax);
                    Broc::Earthquake(fMax, (float)sTankDamageLength, &source,
                                     1050.0f, player_index);
                }
                Broc::string low_rumble("zzzzzzzkkkbb");
                Broc::string high_rumble("zzzzzzzzzzzz");
                Broc::Rumble(&low_rumble, (float)sTankDamageLowRumbleLength,
                             high_rumble, (float)sTankDamageHighRumbleLength,
                             player_index);
                high_rumble.~string();
                low_rumble.~string();
            }
        }
        i = (int)i + 1;
    }
}

// bigsplash - ea: 0x9726C0
void bigsplash(Broc::entity self) {
    HashStr e1;
    e1.mVal = 0x74AA0A6u;
    Broc::endon(self, e1);
    *mp_util_wad::GetEE_bigsplashed(self) = 1;
    HashStr w;
    w.mVal = 0x12305F9Cu;
    Broc::waittill(self, w);
    *mp_util_wad::GetEE_bigsplashed(self) = 0;
}

// fireydeath - ea: 0x972780
void fireydeath(Broc::entity self, Broc::entity tank) {
    HashStr start;
    start.mVal = 0x92082334;
    Broc::waittill(tank, start);
    HashStr ext;
    ext.mVal = 0x6E9DD6CEu;
    Broc::endon(tank, ext);
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    Broc::endon(lvl, ext);
    Broc::string fireextinguish("fireextinguish");
    Broc::string target;
    mp_util_wad::entity_get_targetname(&target, tank);
    Broc::string combined = fireextinguish + target;
    Broc::entity lvl2 =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    Broc::endon(lvl2, combined);
    combined.~string();
    target.~string();
    fireextinguish.~string();
    Broc::string inClassname("script_origin");
    Broc::vector origin;
    mp_util_wad::entity_get_origin(&origin, tank);
    Broc::vector up(0.0f, 0.0f, 32.0f);
    Broc::vector spawnPos = origin + up;
    Broc::entity flameemitter;
    Broc::Spawn(&flameemitter, &inClassname, &spawnPos,
                static_cast<TPakInfo>(0));
    inClassname.~string();
    void* ftor = deleteonextinguish__functor(flameemitter);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_tankdrive.bro",
                        428, "deleteonextinguish", ftor);
    for (;;)
        Broc::wait(Broc::RandomFloat(0.15f) + 0.1f);
    (void)self;
}

// deleteonextinguish - ea: 0x972B90
void deleteonextinguish(Broc::entity self) {
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr ext;
    ext.mVal = 0x6E9DD6CEu;
    Broc::waittill(lvl, ext);
    Broc::wait(0.05f);
    Broc::Delete(&self);
}

// setup_effects - ea: 0x972C20
void setup_effects(Broc::entity self) {
    Broc::string vt = *mp_util_wad::GetEE_vehicletype(self);
    const char* deathfx = NULL;
    const char* deathfire = NULL;
    const char* damagelight = NULL;
    const char* damageheavy = NULL;
    const char* damagecritical = NULL;
    if (stricmp(vt.c_str(), "mp_shermantank") == 0) {
        deathfx = "vexplode_sherman";
        deathfire = "vfire_sherman";
        damagelight = "vdamage_sherman_50";
        damageheavy = "vdamage_sherman_25";
        damagecritical = "vdamage_sherman_10";
    } else if (stricmp(vt.c_str(), "mp_panzeriv") == 0) {
        deathfx = "vexplode_panzeriv";
        deathfire = "vfire_panzeriv";
        damagelight = "vdamage_panzeriv_50";
        damageheavy = "vdamage_panzeriv_25";
        damagecritical = "vdamage_panzeriv_10";
    } else if (stricmp(vt.c_str(), "mp_wc51") == 0) {
        deathfx = "vexplode_wc51";
        deathfire = "vfire_wc51";
        damagelight = "vdamage_wc51_50";
        damageheavy = "vdamage_wc51_25";
        damagecritical = "vdamage_wc51_10";
    } else if (stricmp(vt.c_str(), "mp_horch") == 0) {
        deathfx = "vexplode_horsch";
        deathfire = "vfire_horsch";
        damagelight = "vdamage_horsch_50";
        damageheavy = "vdamage_horsch_25";
        damagecritical = "vdamage_horsch_10";
    } else if (stricmp(vt.c_str(), "mp_motorcycle_bmw") == 0) {
        deathfx = "vexplode_motorcycle";
        deathfire = "vfire_motorcycle";
        damagelight = "vdamage_bmw_50";
        damageheavy = "vdamage_bmw_25";
        damagecritical = "vdamage_bmw_10";
    } else if (stricmp(vt.c_str(), "mp_motorcycle_harley") == 0) {
        deathfx = "vexplode_motorcycle";
        deathfire = "vfire_motorcycle";
        damagelight = "vdamage_harley_50";
        damageheavy = "vdamage_harley_25";
        damagecritical = "vdamage_harley_10";
    }
    if (deathfx != NULL) {
        *mp_util_wad::GetEE_deathfx(self) = deathfx;
        *mp_util_wad::GetEE_deathfire(self) = deathfire;
        *mp_util_wad::GetEE_damagelight(self) = damagelight;
        *mp_util_wad::GetEE_damageheavy(self) = damageheavy;
        *mp_util_wad::GetEE_damagecritical(self) = damagecritical;
    }
    vt.~string();
}

void* BlowUpIfUnderWorld__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(BlowUpIfUnderWorld, self);
}
void* BlowUpIfFlipped__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(BlowUpIfFlipped, self);
}
void* VehicleDamagedEffects__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(VehicleDamagedEffects, self);
}
void* deleteonextinguish__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(deleteonextinguish, self);
}
void* death__functor(Broc::entity self, Broc::entity attacker) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::entity>(death, self, attacker);
}
void* inactivity_blowup__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(inactivity_blowup, self);
}
void* fire__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(fire, self);
}
}
namespace _mp_nano {
void main() {
    Broc::EnableNanoForces(true);
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = WindBlowing__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_nano.bro",
                        __LINE__, "WindBlowing", ftor);
}

// WindBlowing - ea: 0x964750
void WindBlowing(Broc::entity self) {
    (void)self;
    Broc::bint angle_360(0);
    for (;;) {
        angle_360 += Broc::RandomIntRange(-5, 5);
        angle_360 += 360;
        angle_360 = (int)angle_360 % 360;
        Broc::vector ang(Broc::RandomFloatRange(0.0f, 10.0f),
                         (float)(int)angle_360, 0.0f);
        Broc::vector dir;
        dir = ::AnglesToForward(ang);
        Broc::bint speed(Broc::RandomIntRange(0, 3));
        Broc::bfloat sp((float)(int)speed);
        CreateGlobalWind(dir, sp);
        Broc::wait((float)Broc::RandomInt(1));
    }
}

void* WindBlowing__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WindBlowing, self);
}

// CreateGlobalWind - ea: 0x9648C0
unsigned int CreateGlobalWind(Broc::vector direction, Broc::bfloat speed) {
    Broc::vector param2((float)speed, 0.0f, 0.0f);
    Broc::string id("global_wind");
    unsigned int v4 = Broc::CreateNanoForce(&id, &direction, &param2);
    id.~string();
    return v4;
}
}
namespace _mp_audio {
void* audio_spawner__functor(Broc::entity self, Broc::string sound) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::string>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::string>(audio_spawner, self, sound);
}
void* ambient_system__functor(Broc::entity self, Broc::string spawn_package) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::string>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::string>(ambient_system, self, spawn_package);
}
void* interior_triggering_device__functor(Broc::entity trigger,
                                          Broc::entity other) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::entity>(interior_triggering_device, trigger, other);
}
void* ThreadLineSound__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(ThreadLineSound, self);
}
void* ThreadStaticSound__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(ThreadStaticSound, self);
}
void* sound_repeat__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(sound_repeat, self);
}
void* PlayerLocation__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(PlayerLocation, self);
}
void* PlayPainSound__functor(Broc::entity guy, Broc::bint damage) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::bint>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::bint>(PlayPainSound, guy, damage);
}
void* audio_crossfade_wait__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(audio_crossfade_wait, self);
}
void* ThreadStaticSoundPlay__functor(Broc::entity self, Broc::string name) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::string>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::string>(ThreadStaticSoundPlay, self, name);
}
void* ThreadStaticSoundRandomPlay__functor(Broc::entity self,
                                           Broc::string name) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::string>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::string>(ThreadStaticSoundRandomPlay, self, name);
}
void* MoveSoundAlongLine__functor(Broc::entity toMove, Broc::vector start,
                                  Broc::vector end) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor3<Broc::entity, Broc::vector, Broc::vector>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor3<Broc::entity, Broc::vector, Broc::vector>(MoveSoundAlongLine, toMove, start, end);
}
AeThreadFunctor* PlaySound__functor(Broc::entity self, Broc::string sound,
                                     Broc::bfloat delay) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor3<Broc::entity, Broc::string, Broc::bfloat>));
    AeThreadFunctor* result = NULL;
    if (storage != NULL)
        result = ::new (storage) AeThreadFunctor3<Broc::entity, Broc::string, Broc::bfloat>(PlaySound, self, sound, delay);
    sound.~string();
    return result;
}
AeThreadFunctor* PlaySoundAtLocation__functor(Broc::entity self,
                                               Broc::string sound,
                                               Broc::vector position) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor3<Broc::entity, Broc::string, Broc::vector>));
    AeThreadFunctor* result = NULL;
    if (storage != NULL)
        result = ::new (storage) AeThreadFunctor3<Broc::entity, Broc::string, Broc::vector>(PlaySoundAtLocation, self, sound, position);
    sound.~string();
    return result;
}
AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::bfloat>*
PlayTeamDialog__functor(Broc::entity self, Broc::string team,
                        Broc::string sound, Broc::bfloat delay) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::bfloat>));
    AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::bfloat>* result = NULL;
    if (storage != NULL)
        result = ::new (storage) AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::bfloat>(PlayTeamDialog, self, team, sound, delay);
    team.~string();
    sound.~string();
    return result;
}
AeThreadFunctor5<Broc::entity, Broc::string, Broc::string, Broc::string,
                 Broc::bfloat>*
PlayTeamDialog__functor(Broc::entity self, Broc::string primaryteam,
                        Broc::string primaryteamsound,
                        Broc::string secondaryteamsound,
                        Broc::bfloat delay) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor5<Broc::entity, Broc::string, Broc::string, Broc::string, Broc::bfloat>));
    AeThreadFunctor5<Broc::entity, Broc::string, Broc::string, Broc::string,
                     Broc::bfloat>* result = NULL;
    if (storage != NULL)
        result = ::new (storage) AeThreadFunctor5<Broc::entity, Broc::string, Broc::string, Broc::string, Broc::bfloat>(PlayTeamDialog, self, primaryteam, primaryteamsound, secondaryteamsound, delay);
    primaryteam.~string();
    primaryteamsound.~string();
    secondaryteamsound.~string();
    return result;
}
AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::string>*
PlayTeamSound__functor(Broc::entity self, Broc::string team,
                       Broc::string teamsound,
                       Broc::string otherteamsound) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::string>));
    AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::string>* result = NULL;
    if (storage != NULL)
        result = ::new (storage) AeThreadFunctor4<Broc::entity, Broc::string, Broc::string, Broc::string>(PlayTeamSound, self, team, teamsound, otherteamsound);
    team.~string();
    teamsound.~string();
    otherteamsound.~string();
    return result;
}
AeThreadFunctor* player_dying_sounds__functor(Broc::entity player) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(player_dying_sounds, player);
}
}
// ============================================================================
// _mp_loadout - class loadouts, weapons, ammo, specials.
// ============================================================================
namespace _mp_loadout {

void SetTeams(Broc::string allies, Broc::string axis);
void SetAlliesModels(Broc::bint playerClass, Broc::string model);
void SetAxisModels(Broc::bint playerClass, Broc::string model);
void SetPlayerModel(Broc::entity player);
void GiveWeapons(Broc::entity player, Broc::bint playerClass, Broc::bint rank);
void GiveAmmo(Broc::entity playerEnt, Broc::bint playerClass, Broc::bint rank,
              Broc::bbool fillClip);
void GiveAmmoPack(Broc::entity playerEnt, Broc::bint playerClass,
                  Broc::bint rank);
const char* GetGrenadeWeapon(Broc::string team, Broc::bint playerClass);
void GiveSpecialWeapon(Broc::entity player, Broc::bint playerClass,
                       __int16 rank, bool isRespawn);
void GiveMine(Broc::entity player, __int16 rank);
void GiveRifleGrenades(Broc::entity player, __int16 rank);
void GiveHealth(Broc::entity player);
void GiveAmmoWeapon(Broc::entity player);
void GiveArtillery(Broc::entity player);
void GiveSmokeGrenadeSpecial(Broc::entity player, __int16 rank);
void GiveWeaponAmmo(Broc::entity player, Broc::string slot,
                    Broc::bint playerClass, __int16 rank,
                    Broc::bbool fillClips, bool onlyOneExtra);
void GiveWeaponAmmoPack(Broc::entity player, Broc::string slot,
                        Broc::bint playerClass, __int16 rank,
                        __int16 packRank, bool bIsSmokeGrenade);
void GiveWeaponAmmoScale(Broc::entity player, Broc::string slot,
                         Broc::bint playerClass, __int16 rank,
                         Broc::bbool fillClips, float scale);
Broc::bint* GetWeaponClipCount(Broc::bint* result, Broc::string slot,
                               Broc::bint playerClass, __int16 rank,
                               Broc::string team);
Broc::bint* GetGrenadeCount(Broc::bint* result, Broc::bint playerClass,
                            __int16 rank);
Broc::bint* GetPrimaryClipCount(Broc::bint* result, Broc::bint playerClass,
                                __int16 rank, Broc::string team);
Broc::bint* GetPistolClipCount(Broc::bint* result, Broc::bint playerClass,
                               __int16 rank);
int CallbackCanPickupAmmoPack(Broc::entity playerEnt);
void CallbackGiveAmmoPack(Broc::entity playerEnt, unsigned int rank);
void CallbackPickupKit(Broc::entity playerEnt, __int16 newClass);
int CallbackGetTeamWeapon(const char* team, unsigned int playerClass);
int CallbackGetGrenadeCount(unsigned int playerClass, __int16 rank);
int CallbackGetClipCount(unsigned int playerClass, __int16 rank,
                         int allied_team);
int CallbackGetSlotClipCount(const char* slotName, unsigned int playerClass,
                             __int16 rank, int allied_team);
Broc::bfloat* GetStartingWeaponAmmoScale(Broc::bfloat* result,
                                         Broc::entity player,
                                         Broc::string slot,
                                         Broc::bint playerClass,
                                         __int16 rank);
Broc::bbool* IsFullWeaponAmmo(Broc::bbool* result, Broc::entity player,
                              Broc::string slot);
Broc::bint* GetRankCount(Broc::bint* result, Broc::bint rank,
                         Broc::bint rank0Value, Broc::bint rank1Value,
                         Broc::bint rank2Value);
Broc::bbool* GetsDualPistols(Broc::bbool* result, Broc::bint playerClass);
const char* GetAlliesWeapon(Broc::string team, Broc::bint playerClass);
const char* GetAxisWeapon(Broc::string team, Broc::bint playerClass);
const char* GetAmericanWeapon(Broc::bint playerClass);
const char* GetGermanWeapon(Broc::bint playerClass);
void GiveDualPistols(Broc::entity player, __int16 rank);
void GiveBazooka(Broc::entity player, __int16 rank);
void GiveSatchel(Broc::entity player, __int16 rank);
void NotifyWhenTimerExpires(Broc::entity player, Broc::bint time,
                            HashStr notifyString);
void SpecialClassAudio(Broc::entity player);
void HealthAmmoDispenser(Broc::entity player, Broc::string weapon,
                         Broc::bbool health, Broc::bint rank0Time,
                         Broc::bint rank1Time, Broc::bint rank2Time);
void ArtilleryDispenser(Broc::entity player);

// main - ea: 0x95D6B0
void main() {
    Broc::string gametype;
    Broc::GetCvar(&gametype, "mp_gametype");
    mp_util_wad::pLevel->gametype = gametype;
    gametype.~string();
    Broc::string axis("german");
    Broc::string allies("american");
    SetTeams(allies, axis);
}

// SetTeams - ea: 0x95D7A0
void SetTeams(Broc::string allies, Broc::string axis) {
    mp_util_wad::pLevel->allies = allies;
    mp_util_wad::pLevel->axis = axis;
    SetAlliesModels(Broc::bint(0), "mp_US_ass");
    SetAlliesModels(Broc::bint(1), "mp_US_inf");
    SetAlliesModels(Broc::bint(3), "mp_US_medic");
    SetAlliesModels(Broc::bint(2), "mp_US_rifle");
    SetAlliesModels(Broc::bint(6), "mp_US_scout");
    SetAlliesModels(Broc::bint(4), "mp_US_supp");
    SetAlliesModels(Broc::bint(5), "mp_US_tank");
    SetAxisModels(Broc::bint(0), "mp_GE_ass");
    SetAxisModels(Broc::bint(1), "mp_GE_inf");
    SetAxisModels(Broc::bint(3), "mp_GE_medic");
    SetAxisModels(Broc::bint(2), "mp_GE_rifle");
    SetAxisModels(Broc::bint(6), "mp_GE_scout");
    SetAxisModels(Broc::bint(4), "mp_GE_supp");
    SetAxisModels(Broc::bint(5), "mp_GE_tank");
    mp_util_wad::pLevel->alliesViewModel = "xmodel/viewmodel_hands_us";
    mp_util_wad::pLevel->axisViewModel = "xmodel/viewmodel_hands_us";
    Broc::BrocExports& x = Broc::gBrocAPI.mBrocExports;
    x.mCallbackCanPickupAmmoPack =
        (int (*)(const Broc::entity))CallbackCanPickupAmmoPack;
    x.mCallbackGiveAmmoPack =
        (void (*)(const Broc::entity, const unsigned int))CallbackGiveAmmoPack;
    x.mCallbackPickupKit =
        (void (*)(const Broc::entity, const unsigned int))CallbackPickupKit;
    x.mCallbackGetTeamWeapon =
        (int (*)(const char*, const unsigned int))CallbackGetTeamWeapon;
    x.mCallbackGetGrenadeCount =
        (int (*)(const unsigned int, const unsigned int))CallbackGetGrenadeCount;
    x.mCallbackGetClipCount =
        (int (*)(const unsigned int, const unsigned int,
                 const int))CallbackGetClipCount;
    x.mCallbackGetSlotClipCount =
        (int (*)(const char*, const unsigned int, const unsigned int,
                 const int))CallbackGetSlotClipCount;
    allies.~string();
    axis.~string();
}

// SetAlliesModels - ea: 0x95DD60
void SetAlliesModels(Broc::bint playerClass, Broc::string model) {
    mp_util_wad::pLevel->models[(unsigned int)(int)playerClass] = model;
    model.~string();
}

// SetAxisModels - ea: 0x95DDF0
void SetAxisModels(Broc::bint playerClass, Broc::string model) {
    mp_util_wad::pLevel->models[(unsigned int)((int)playerClass + 7)] = model;
    model.~string();
}

// GiveLoadout - ea: 0x95DE90
void GiveLoadout(Broc::entity player) {
    GiveWeapons(player, Broc::bint((int)mp_util_wad::entity_get_playerClass(player)),
                Broc::bint((int)mp_util_wad::entity_get_rank(player)));
    SetPlayerModel(player);
}

// UpdatePlayerModelForRank - ea: 0x95DF10
void UpdatePlayerModelForRank(Broc::entity player) {
    Broc::bint rank((int)mp_util_wad::entity_get_rank(player));
    Broc::bint playerClass((int)mp_util_wad::entity_get_playerClass(player));
    if ((int)playerClass == -1)
        playerClass = 2;
    Broc::string team;
    mp_util_wad::entity_get_team(&team, player);
    unsigned int idx;
    if (team == "allies")
        idx = (unsigned int)(int)playerClass;
    else
        idx = (unsigned int)((int)playerClass + 7);
    team.~string();
    Broc::SetAiType(&player, &mp_util_wad::pLevel->models[idx], 0);
    (void)rank;
}

// SetPlayerModel - ea: 0x95E0C0
void SetPlayerModel(Broc::entity player) {
    bool local = Broc::Code_IsLocalPlayer(player);
    if (local) {
        Broc::string team;
        mp_util_wad::entity_get_team(&team, player);
        if (team == "allies")
            Broc::SetViewModel(&player, &mp_util_wad::pLevel->alliesViewModel);
        else
            Broc::SetViewModel(&player, &mp_util_wad::pLevel->axisViewModel);
        team.~string();
    }
    UpdatePlayerModelForRank(player);
}

// DisplayYouWillSpawnWithMessage - ea: 0x95E210
void DisplayYouWillSpawnWithMessage(Broc::entity self) {
    Broc::SetActionHint((int)0xFCC9BF53, Broc::GetPlayerIndex(self));
}

// PlayerKill - ea: 0x961150
void PlayerKill() {
}

// GetsDualPistols - ea: 0x963E20
Broc::bbool* GetsDualPistols(Broc::bbool* result, Broc::bint playerClass) {
    (void)playerClass;
    *result = Broc::bbool(false);
    return result;
}

// GetRankCount - ea: 0x963E70
Broc::bint* GetRankCount(Broc::bint* result, Broc::bint rank,
                         Broc::bint rank0Value, Broc::bint rank1Value,
                         Broc::bint rank2Value) {
    if ((int)rank == 1)
        result->mVal = (int)rank1Value;
    else if ((int)rank == 2)
        result->mVal = (int)rank2Value;
    else
        result->mVal = (int)rank0Value;
    return result;
}

// GetAmericanWeapon - ea: 0x95F050
const char* GetAmericanWeapon(Broc::bint playerClass) {
    switch ((int)playerClass) {
    case 0: return "bar";
    case 1: return "thompson";
    case 2: return "m1garand";
    case 3: return "shotgun";
    case 4: return "mg30cal";
    case 5: return "bazooka";
    case 6: return "springfield";
    default: return "m1garand";
    }
}

// GetGermanWeapon - ea: 0x95F0F0
const char* GetGermanWeapon(Broc::bint playerClass) {
    switch ((int)playerClass) {
    case 0: return "mp44";
    case 1: return "mp40";
    case 2: return "kar98";
    case 3: return "shotgun";
    case 4: return "mg34";
    case 5: return "panzerschreck";
    case 6: return "kar98_sniper";
    default: return "kar98";
    }
}

// GetAlliesWeapon - ea: 0x95EE70
const char* GetAlliesWeapon(Broc::string team, Broc::bint playerClass) {
    if (team == "american") {
        team.~string();
        return GetAmericanWeapon(playerClass);
    }
    if (Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_loadout.bro", __LINE__,
            "The allies team is invalid.  Must be 'american'"))
        __debugbreak();
    team.~string();
    return "";
}

// GetAxisWeapon - ea: 0x95EF60
const char* GetAxisWeapon(Broc::string team, Broc::bint playerClass) {
    if (team == "german") {
        team.~string();
        return GetGermanWeapon(playerClass);
    }
    if (Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_loadout.bro", __LINE__,
            "The axis team is invalid.  Must be 'german'"))
        __debugbreak();
    team.~string();
    return "";
}

// GiveWeapons - ea: 0x95E270
void GiveWeapons(Broc::entity player, Broc::bint playerClass, Broc::bint rank) {
    Broc::TakeAllWeapons(player);
    if ((int)playerClass > 6)
        playerClass = 2;
    Broc::string weapon;
    weapon = "";
    Broc::string pistol;
    pistol = "";
    Broc::string team;
    mp_util_wad::entity_get_team(&team, player);
    if (team == "allies") {
        Broc::string allies = mp_util_wad::pLevel->allies;
        weapon = GetAlliesWeapon(allies, playerClass);
        pistol = "colt";
    } else {
        Broc::string axis = mp_util_wad::pLevel->axis;
        weapon = GetAxisWeapon(axis, playerClass);
        pistol = "p38";
    }
    team.~string();
    Broc::GiveWeapon(&player, &weapon);
    Broc::bbool dual;
    GetsDualPistols(&dual, playerClass);
    if (!(bool)dual || (int)rank == 0)
        Broc::GiveWeapon(&player, &pistol);
    if (Broc::Code_GetTeamGame()) {
        GiveSpecialWeapon(player, playerClass, (int)rank, false);
    } else {
        if (Broc::Code_IsLocalPlayer(player)) {
            Broc::Code_SetSpecialRecharge(0, 0, (int)playerClass,
                                          Broc::GetPlayerIndex(player));
            GiveSmokeGrenadeSpecial(player, (__int16)(int)rank);
        }
    }
    GiveAmmo(player, playerClass, rank, Broc::bbool(true));
    Broc::SwitchToWeapon(player, weapon);
    pistol.~string();
    weapon.~string();
}

// GiveAmmo - ea: 0x95E640
void GiveAmmo(Broc::entity playerEnt, Broc::bint playerClass, Broc::bint rank,
              Broc::bbool fillClip) {
    Broc::string slot("primary");
    GiveWeaponAmmo(playerEnt, slot, playerClass, (__int16)(int)rank,
                   fillClip, false);
    slot.~string();
    Broc::string slotb("primaryb");
    GiveWeaponAmmo(playerEnt, slotb, playerClass, (__int16)(int)rank,
                   fillClip, false);
    slotb.~string();
    Broc::string team;
    mp_util_wad::entity_get_team(&team, playerEnt);
    Broc::string grenade(GetGrenadeWeapon(team, playerClass));
    Broc::GiveWeapon(&playerEnt, &grenade);
    Broc::string gslot("grenade");
    GiveWeaponAmmo(playerEnt, gslot, playerClass, (__int16)(int)rank,
                   fillClip, false);
    gslot.~string();
    grenade.~string();
}

// GiveAmmoPack - ea: 0x95E840
void GiveAmmoPack(Broc::entity playerEnt, Broc::bint playerClass,
                  Broc::bint rank) {
    Broc::bint playerRank((int)mp_util_wad::entity_get_rank(playerEnt));
    if ((int)playerClass == 5) {
        if ((int)playerRank >= 0) {
            Broc::string slot("primary");
            GiveWeaponAmmoPack(playerEnt, slot, playerClass,
                               (__int16)(int)playerRank, (__int16)(int)rank,
                               false);
            slot.~string();
        }
    } else {
        Broc::string slot("primary");
        GiveWeaponAmmoPack(playerEnt, slot, playerClass,
                           (__int16)(int)playerRank, (__int16)(int)rank,
                           false);
        slot.~string();
    }
    Broc::string slotb("primaryb");
    GiveWeaponAmmoPack(playerEnt, slotb, playerClass, (__int16)(int)playerRank,
                       (__int16)(int)rank, false);
    slotb.~string();
    Broc::string team;
    mp_util_wad::entity_get_team(&team, playerEnt);
    Broc::string grenade(GetGrenadeWeapon(team, playerClass));
    if (grenade == "smokegrenade" || grenade == "smokegrenade_axis") {
        Broc::GiveWeapon(&playerEnt, &grenade);
        Broc::string gslot("grenade");
        GiveWeaponAmmoPack(playerEnt, gslot, playerClass,
                           (__int16)(int)playerRank, (__int16)(int)rank, true);
        gslot.~string();
    } else if ((int)playerClass != 5 && (int)rank >= 2) {
        Broc::GiveWeapon(&playerEnt, &grenade);
        Broc::string gslot("grenade");
        GiveWeaponAmmoPack(playerEnt, gslot, playerClass,
                           (__int16)(int)playerRank, (__int16)(int)rank,
                           false);
        gslot.~string();
    }
    grenade.~string();
}

// GetGrenadeWeapon - ea: 0x95F190
const char* GetGrenadeWeapon(Broc::string team, Broc::bint playerClass) {
    if (mp_util_wad::pLevel->gametype == "dm") {
        bool axis = team == "axis";
        team.~string();
        return axis ? "stielhandgranate" : "fraggrenade";
    }
    switch ((int)playerClass) {
    case 1:
    case 5:
        team.~string();
        return "stickygrenade";
    case 2:
    case 3:
    {
        bool axis = team == "axis";
        team.~string();
        return axis ? "smokegrenade_axis" : "smokegrenade";
    }
    default:
    {
        bool axis = team == "axis";
        team.~string();
        return axis ? "stielhandgranate" : "fraggrenade";
    }
    }
}

// GiveSpecialWeapon - ea: 0x95F430
void GiveSpecialWeapon(Broc::entity player, Broc::bint playerClass,
                       __int16 rank, bool isRespawn) {
    if (Broc::Code_IsLocalPlayer(player)) {
        Broc::Code_SetSpecialRecharge(0, 0, (int)playerClass,
                                      Broc::GetPlayerIndex(player));
        if ((int)playerClass != 6 ||
            (bool)*mp_util_wad::GetEE_specialWeaponChangeClassFlag(player)) {
            *mp_util_wad::GetEE_specialWeaponTime(player) = 0;
            *mp_util_wad::GetEE_specialWeaponChangeClassFlag(player) = false;
        }
        HashStr n;
        n.mVal = 0xC9444C8D;
        Broc::notify(player, n);
        switch ((int)playerClass) {
        case 0:
            if (!isRespawn)
                GiveMine(player, rank);
            break;
        case 1:
            if (!isRespawn)
                GiveMine(player, rank);
            break;
        case 2:
            if (!isRespawn)
                GiveRifleGrenades(player, rank);
            break;
        case 3:
            GiveHealth(player);
            break;
        case 4:
        case 5:
            GiveAmmoWeapon(player);
            break;
        case 6:
            if (!isRespawn)
                GiveArtillery(player);
            break;
        default:
            break;
        }
    }
}

// GiveDualPistols - ea: 0x95F6B0
void GiveDualPistols(Broc::entity player, __int16 rank) {
    if (rank != 0) {
        Broc::string weapon("coltdual");
        Broc::string team;
        mp_util_wad::entity_get_team(&team, player);
        if (team == "axis")
            weapon = "p38dual";
        team.~string();
        Broc::GiveWeapon(&player, &weapon);
        Broc::string sSlot("pistol");
        Broc::SetWeaponSlotAmmo(&player, &sSlot, 0);
        sSlot.~string();
        Broc::string s2("pistol");
        Broc::SetWeaponSlotClipAmmo(&player, &s2, 0);
        s2.~string();
        Broc::bint clipCount;
        GetRankCount(&clipCount, Broc::bint(rank), Broc::bint(2),
                     Broc::bint(2), Broc::bint(4));
        Broc::string slot("pistol");
        Broc::bint clipSize(Broc::GetFullClipAmmoCount(&player, &slot));
        slot.~string();
        Broc::bint amount((int)clipSize * (int)clipCount);
        Broc::string s3("pistol");
        Broc::SetWeaponSlotClipAmmo(&player, &s3, (int)amount);
        s3.~string();
        if ((int)amount > (int)clipSize) {
            Broc::string s4("pistol");
            Broc::SetWeaponSlotAmmo(&player, &s4, (int)amount - (int)clipSize);
            s4.~string();
        }
        weapon.~string();
    }
}

// GiveBazooka - ea: 0x95FA20
void GiveBazooka(Broc::entity player, __int16 rank) {
    Broc::string weapon("bazooka");
    Broc::string team;
    mp_util_wad::entity_get_team(&team, player);
    if (team == "axis")
        weapon = "panzerschreck";
    team.~string();
    Broc::GiveWeapon(&player, &weapon);
    Broc::string sSlot("primaryb");
    Broc::SetWeaponSlotAmmo(&player, &sSlot, 0);
    sSlot.~string();
    Broc::string s2("primaryb");
    Broc::SetWeaponSlotClipAmmo(&player, &s2, 0);
    s2.~string();
    Broc::bint ammo;
    GetRankCount(&ammo, Broc::bint(rank), Broc::bint(4), Broc::bint(4),
                 Broc::bint(4));
    Broc::string s3("primaryb");
    Broc::SetWeaponSlotClipAmmo(&player, &s3, 1);
    s3.~string();
    Broc::string s4("primaryb");
    Broc::SetWeaponSlotAmmo(&player, &s4, (int)ammo - 1);
    s4.~string();
    weapon.~string();
}

// GiveSatchel - ea: 0x95FC50
void GiveSatchel(Broc::entity player, __int16 rank) {
    Broc::string pszWeaponName("mp_satchel");
    Broc::GiveWeapon(&player, &pszWeaponName);
    pszWeaponName.~string();
    Broc::string sSlot("special");
    Broc::SetWeaponSlotAmmo(&player, &sSlot, 0);
    sSlot.~string();
    Broc::string s2("special");
    Broc::SetWeaponSlotClipAmmo(&player, &s2, 0);
    s2.~string();
    Broc::bint count;
    GetRankCount(&count, Broc::bint(rank), Broc::bint(1), Broc::bint(2),
                 Broc::bint(3));
    Broc::string s3("special");
    Broc::SetWeaponSlotClipAmmo(&player, &s3, (int)count);
    s3.~string();
}

// GiveMine - ea: 0x95FDF0
void GiveMine(Broc::entity player, __int16 rank) {
    Broc::string pszWeaponName("mp_mine");
    Broc::GiveWeapon(&player, &pszWeaponName);
    pszWeaponName.~string();
    Broc::string sSlot("special");
    Broc::SetWeaponSlotAmmo(&player, &sSlot, 0);
    sSlot.~string();
    Broc::string s2("special");
    Broc::SetWeaponSlotClipAmmo(&player, &s2, 0);
    s2.~string();
    Broc::bint count;
    GetRankCount(&count, Broc::bint(rank), Broc::bint(1), Broc::bint(2),
                 Broc::bint(3));
    Broc::string s3("special");
    Broc::SetWeaponSlotClipAmmo(&player, &s3, (int)count);
    s3.~string();
}

// GiveRifleGrenades - ea: 0x95FF90
void GiveRifleGrenades(Broc::entity player, __int16 rank) {
    Broc::string team;
    mp_util_wad::entity_get_team(&team, player);
    Broc::string pszWeaponName(team == "allies" ? "m1garand_rg" : "k98_rg");
    Broc::GiveWeapon(&player, &pszWeaponName);
    pszWeaponName.~string();
    Broc::string sSlot("special");
    Broc::SetWeaponSlotAmmo(&player, &sSlot, 0);
    sSlot.~string();
    Broc::string s2("special");
    Broc::SetWeaponSlotClipAmmo(&player, &s2, 0);
    s2.~string();
    Broc::bint count;
    GetRankCount(&count, Broc::bint(rank), Broc::bint(1), Broc::bint(2),
                 Broc::bint(3));
    Broc::string s3("special");
    Broc::SetWeaponSlotClipAmmo(&player, &s3, (int)count);
    s3.~string();
    team.~string();
}

// GiveArtillery - ea: 0x9601D0
void GiveArtillery(Broc::entity player) {
    __int16 rank = mp_util_wad::entity_get_rank(player);
    Broc::string weapon;
    if (rank == 1)
        weapon = "mp_binoculars_nospot_rank2";
    else if (rank == 2)
        weapon = "mp_binoculars_nospot_rank3";
    else
        weapon = "mp_binoculars_nospot";
    Broc::GiveWeapon(&player, &weapon);
    weapon.~string();
    Broc::string sSlot("special");
    Broc::SetWeaponSlotAmmo(&player, &sSlot, 0);
    sSlot.~string();
    Broc::string s2("special");
    Broc::SetWeaponSlotClipAmmo(&player, &s2, 0);
    s2.~string();
    void* ftor = ArtilleryDispenser__functor(player);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_loadout.bro",
                        __LINE__, "ArtilleryDispenser", ftor);
}

// GiveSmokeGrenadeSpecial - ea: 0x960460
void GiveSmokeGrenadeSpecial(Broc::entity player, __int16 rank) {
    Broc::bint count;
    GetRankCount(&count, Broc::bint(rank), Broc::bint(0), Broc::bint(1),
                 Broc::bint(1));
    if ((int)count > 0) {
        Broc::string pszWeaponName("mp_smokegrenade_special");
        Broc::GiveWeapon(&player, &pszWeaponName);
        pszWeaponName.~string();
        Broc::string sSlot("special");
        Broc::SetWeaponSlotAmmo(&player, &sSlot, 0);
        sSlot.~string();
        Broc::string s2("special");
        Broc::SetWeaponSlotClipAmmo(&player, &s2, (int)count);
        s2.~string();
    }
}

// GiveHealth - ea: 0x961170
void GiveHealth(Broc::entity player) {
    Broc::string weapon("mp_revive");
    void* ftor = HealthAmmoDispenser__functor(
        player, weapon, true, Broc::bint(3), Broc::bint(2), Broc::bint(1));
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_loadout.bro",
                        __LINE__, "HealthAmmoDispenser", ftor);
}

// GiveAmmoWeapon - ea: 0x961340
void GiveAmmoWeapon(Broc::entity player) {
    Broc::string ammo_name("mp_ammo");
    Broc::string team;
    mp_util_wad::entity_get_team(&team, player);
    if (team == "axis")
        ammo_name = "mp_ammo_axis";
    team.~string();
    void* ftor = HealthAmmoDispenser__functor(
        player, ammo_name, false, Broc::bint(5), Broc::bint(4),
        Broc::bint(3));
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_loadout.bro",
                        __LINE__, "HealthAmmoDispenser", ftor);
    ammo_name.~string();
}

// NotifyWhenTimerExpires - ea: 0x961080
void NotifyWhenTimerExpires(Broc::entity player, Broc::bint time,
                            HashStr notifyString) {
    HashStr e1;
    e1.mVal = 0xC9444C8D;
    Broc::endon(player, e1);
    HashStr e2;
    e2.mVal = 0x5FB9FAE1u;
    Broc::endon(player, e2);
    HashStr e3;
    e3.mVal = 0x74AA0A6u;
    Broc::endon(player, e3);
    Broc::wait((float)(int)time);
    Broc::notify(player, notifyString);
}

// local_player_joined - ea: 0x960E10
void local_player_joined(Broc::entity player) {
    void* ftor = SpecialClassAudio__functor(player);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_loadout.bro",
                        __LINE__, "SpecialClassAudio", ftor);
}

// SpecialClassAudio - ea: 0x960E70
void SpecialClassAudio(Broc::entity player) {
    for (;;) {
        HashStr trigger;
        trigger.mVal = 0x433F9306u;
        Broc::waittill(player, trigger);
        if (mp_util_wad::entity_get_playerClass(player) == 6) {
            Broc::bint PickALine(Broc::RandomInt(5));
            if ((int)PickALine < 4) {
                Broc::string script("Scout_SpecialFire");
                Broc::EffectEventPlay(&player, &script);
                script.~string();
            } else {
                Broc::string script("Scout_SpecialFire_B");
                Broc::EffectEventPlay(&player, &script);
                script.~string();
            }
            Broc::wait(2.0f);
            Broc::bint PickASound(Broc::RandomInt(5));
            if ((int)PickASound < 4) {
                Broc::string script("MP_GS_MortarFireB");
                Broc::EffectEventPlay(&player, &script);
                script.~string();
            } else {
                Broc::string script("MP_GS_MortarFire");
                Broc::EffectEventPlay(&player, &script);
                script.~string();
            }
        }
        Broc::wait(1.0f);
    }
}

// IsFullWeaponAmmo - ea: 0x961AB0
Broc::bbool* IsFullWeaponAmmo(Broc::bbool* result, Broc::entity player,
                              Broc::string slot) {
    int slotAmmo = Broc::GetWeaponSlotAmmo(player, slot);
    int maxSlotAmmo = Broc::GetMaxAmmo(&player, &slot);
    *result = Broc::bbool(slotAmmo >= maxSlotAmmo);
    slot.~string();
    return result;
}

// GetStartingWeaponAmmoScale - ea: 0x961BD0
Broc::bfloat* GetStartingWeaponAmmoScale(Broc::bfloat* result,
                                         Broc::entity player,
                                         Broc::string slot,
                                         Broc::bint playerClass,
                                         __int16 rank) {
    Broc::string team;
    mp_util_wad::entity_get_team(&team, player);
    Broc::bint clipCount;
    GetWeaponClipCount(&clipCount, slot, playerClass, rank, team);
    team.~string();
    Broc::bint clipSize(Broc::GetFullClipAmmoCount(&player, &slot));
    if (slot == "grenade" || slot == "smokegrenade") {
        clipSize = (int)clipCount;
        clipCount = 1;
    }
    Broc::bint amount((int)clipSize * (int)clipCount);
    int originalSlotAmmo = Broc::GetWeaponSlotAmmo(player, slot);
    int originalSlotClipAmmo = Broc::GetWeaponSlotClipAmmo(player, slot);
    float scale = (float)(originalSlotAmmo + originalSlotClipAmmo) /
                  (float)(int)amount;
    *result = Broc::bfloat(scale);
    slot.~string();
    return result;
}

// GiveWeaponAmmo - ea: 0x962060
void GiveWeaponAmmo(Broc::entity player, Broc::string slot,
                    Broc::bint playerClass, __int16 rank,
                    Broc::bbool fillClips, bool onlyOneExtra) {
    Broc::string team;
    mp_util_wad::entity_get_team(&team, player);
    Broc::bint clipCount;
    GetWeaponClipCount(&clipCount, slot, playerClass, rank, team);
    team.~string();
    Broc::bint clipSize(Broc::GetFullClipAmmoCount(&player, &slot));
    if (slot == "grenade" || slot == "smokegrenade") {
        clipSize = (int)clipCount;
        clipCount = 1;
    }
    Broc::bint amount((int)clipSize * (int)clipCount);
    int originalSlotAmmo = Broc::GetWeaponSlotAmmo(player, slot);
    int originalSlotClipAmmo = Broc::GetWeaponSlotClipAmmo(player, slot);
    if (originalSlotAmmo + originalSlotClipAmmo > (int)amount) {
        amount = originalSlotAmmo + originalSlotClipAmmo;
    } else if (onlyOneExtra) {
        if ((int)amount > originalSlotAmmo + originalSlotClipAmmo)
            amount = originalSlotAmmo + originalSlotClipAmmo + 1;
    }
    Broc::SetWeaponSlotAmmo(&player, &slot, 0);
    if ((bool)fillClips) {
        Broc::SetWeaponSlotClipAmmo(&player, &slot, 0);
        Broc::SetWeaponSlotClipAmmo(&player, &slot, (int)amount);
        if ((int)amount > (int)clipSize)
            Broc::SetWeaponSlotAmmo(&player, &slot, (int)amount - (int)clipSize);
    } else {
        Broc::SetWeaponSlotAmmo(&player, &slot, (int)amount - originalSlotClipAmmo);
    }
    slot.~string();
}

// GiveWeaponAmmoScale - ea: 0x9623D0
void GiveWeaponAmmoScale(Broc::entity player, Broc::string slot,
                         Broc::bint playerClass, __int16 rank,
                         Broc::bbool fillClips, float scale) {
    Broc::string team;
    mp_util_wad::entity_get_team(&team, player);
    Broc::bint clipCount;
    GetWeaponClipCount(&clipCount, slot, playerClass, rank, team);
    team.~string();
    Broc::bint clipSize(Broc::GetFullClipAmmoCount(&player, &slot));
    if (slot == "grenade" || slot == "smokegrenade") {
        clipSize = (int)clipCount;
        clipCount = 1;
    }
    Broc::bint amount((int)(scale * (float)(int)clipSize) * (int)clipCount);
    int originalSlotAmmo = Broc::GetWeaponSlotAmmo(player, slot);
    int originalSlotClipAmmo = Broc::GetWeaponSlotClipAmmo(player, slot);
    Broc::SetWeaponSlotAmmo(&player, &slot, 0);
    if ((bool)fillClips) {
        Broc::SetWeaponSlotClipAmmo(&player, &slot, 0);
        Broc::SetWeaponSlotClipAmmo(&player, &slot, (int)amount);
        if ((int)amount > (int)clipSize)
            Broc::SetWeaponSlotAmmo(&player, &slot, (int)amount - (int)clipSize);
    } else {
        Broc::SetWeaponSlotAmmo(&player, &slot, (int)amount - originalSlotClipAmmo);
    }
    (void)originalSlotAmmo;
    slot.~string();
}

// GiveWeaponAmmoPack - ea: 0x961DB0
void GiveWeaponAmmoPack(Broc::entity player, Broc::string slot,
                        Broc::bint playerClass, __int16 rank,
                        __int16 packRank, bool bIsSmokeGrenade) {
    Broc::bint clipCount(0);
    Broc::bint clipSize(Broc::GetFullClipAmmoCount(&player, &slot));
    if (packRank > 0)
        clipCount = 4;
    else
        clipCount = 2;
    if (bIsSmokeGrenade) {
        clipSize = 1;
        clipCount = 1;
    } else if (slot == "grenade") {
        clipSize = 1;
        clipCount = 0;
        if (packRank >= 2)
            clipCount = 1;
    }
    if (slot == "grenade") {
        int slotAmmo = Broc::GetWeaponSlotAmmo(player, slot);
        int slotClipAmmo = Broc::GetWeaponSlotClipAmmo(player, slot);
        Broc::bint currentAmmo(slotClipAmmo + slotAmmo);
        Broc::string team;
        mp_util_wad::entity_get_team(&team, player);
        Broc::bint maxAmmo;
        GetWeaponClipCount(&maxAmmo, Broc::string(slot), playerClass, rank,
                           team);
        team.~string();
        Broc::bint check((int)currentAmmo + (int)clipCount);
        if ((int)check > (int)maxAmmo)
            clipCount = 0;
    }
    if ((int)clipCount > 0) {
        Broc::bint amount((int)clipSize * (int)clipCount);
        amount = (int)amount + Broc::GetWeaponSlotAmmo(player, slot);
        Broc::SetWeaponSlotAmmo(&player, &slot, (int)amount);
    }
    slot.~string();
}

// GetWeaponClipCount - ea: 0x9626B0
Broc::bint* GetWeaponClipCount(Broc::bint* result, Broc::string slot,
                               Broc::bint playerClass, __int16 rank,
                               Broc::string team) {
    if (slot == "primary") {
        GetPrimaryClipCount(result, playerClass, rank, team);
    } else if (slot == "primaryb") {
        GetPistolClipCount(result, playerClass, rank);
        team.~string();
    } else if (slot == "grenade" || slot == "smokegrenade") {
        GetGrenadeCount(result, playerClass, rank);
        team.~string();
    } else {
        if (slot == "special")
            result->mVal = 1;
        else
            result->mVal = 0;
        team.~string();
    }
    slot.~string();
    return result;
}

// GetGrenadeCount - ea: 0x963AE0
Broc::bint* GetGrenadeCount(Broc::bint* result, Broc::bint playerClass,
                            __int16 rank) {
    switch ((int)playerClass) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        if (rank == 0)
            result->mVal = 1;
        else if (rank == 1)
            result->mVal = 2;
        else if (rank == 2)
            result->mVal = 3;
        else
            result->mVal = 0;
        break;
    default:
        result->mVal = 0;
        break;
    }
    return result;
}

// GetPrimaryClipCount - ea: 0x9633D0
Broc::bint* GetPrimaryClipCount(Broc::bint* result, Broc::bint playerClass,
                                __int16 rank, Broc::string team) {
    switch ((int)playerClass) {
    case 0:
        if (rank == 0 || rank == 1 || rank == 2) {
            if (team == "allies")
                result->mVal = 5;
            else
                result->mVal = 4;
        } else {
            result->mVal = 0;
        }
        break;
    case 1:
    case 5:
        if (rank == 0 || rank == 1 || rank == 2)
            result->mVal = 4;
        else
            result->mVal = 0;
        break;
    case 2:
        if (rank == 0 || rank == 1 || rank == 2)
            result->mVal = 5;
        else
            result->mVal = 0;
        break;
    case 3:
        if (rank == 0 || rank == 1 || rank == 2)
            result->mVal = 5;
        else
            result->mVal = 0;
        break;
    case 4:
        if (rank == 0 || rank == 1 || rank == 2)
            result->mVal = 3;
        else
            result->mVal = 0;
        break;
    case 6:
        if (rank == 0 || rank == 1 || rank == 2)
            result->mVal = 5;
        else
            result->mVal = 0;
        break;
    default:
        result->mVal = 0;
        break;
    }
    team.~string();
    return result;
}

// GetPistolClipCount - ea: 0x9639B0
Broc::bint* GetPistolClipCount(Broc::bint* result, Broc::bint playerClass,
                               __int16 rank) {
    rank = (__int16)(rank & 3);
    Broc::bbool dual;
    GetsDualPistols(&dual, playerClass);
    if ((bool)dual && rank != 0) {
        result->mVal = 0;
        return result;
    }
    if (rank == 0 || rank == 1)
        result->mVal = 4;
    else if (rank == 2)
        result->mVal = 4;
    else
        result->mVal = 0;
    return result;
}

// CallbackGetGrenadeCount - ea: 0x963A90
int CallbackGetGrenadeCount(unsigned int playerClass, __int16 rank) {
    Broc::bint count;
    GetGrenadeCount(&count, Broc::bint((int)playerClass), rank);
    return (int)count;
}

// CallbackGetTeamWeapon - ea: 0x95EC60
int CallbackGetTeamWeapon(const char* team, unsigned int playerClass) {
    Broc::string strTeam(team);
    Broc::string weapon;
    int index = 0;
    if (strTeam == "allies") {
        Broc::string allies = mp_util_wad::pLevel->allies;
        weapon = GetAlliesWeapon(allies, Broc::bint((int)playerClass));
        index = Broc::GetWeaponIndex(&weapon);
    } else if (strTeam == "axis") {
        Broc::string axis = mp_util_wad::pLevel->axis;
        weapon = GetAxisWeapon(axis, Broc::bint((int)playerClass));
        index = Broc::GetWeaponIndex(&weapon);
    }
    weapon.~string();
    strTeam.~string();
    return index;
}

// CallbackGetClipCount - ea: 0x962920
int CallbackGetClipCount(unsigned int playerClass, __int16 rank,
                         int allied_team) {
    Broc::string team(allied_team != 0 ? "allies" : "axis");
    Broc::bint count;
    GetPrimaryClipCount(&count, Broc::bint((int)playerClass), rank, team);
    return (int)count;
}

// CallbackGetSlotClipCount - ea: 0x9629E0
int CallbackGetSlotClipCount(const char* slotName, unsigned int playerClass,
                             __int16 rank, int allied_team) {
    Broc::string strSlot(slotName);
    Broc::string team(allied_team != 0 ? "allies" : "axis");
    Broc::bint count;
    GetWeaponClipCount(&count, strSlot, Broc::bint((int)playerClass), rank,
                       team);
    return (int)count;
}

// CallbackGiveAmmoPack - ea: 0x962AE0
void CallbackGiveAmmoPack(Broc::entity playerEnt, unsigned int rank) {
    if (Broc::IsPlayer(playerEnt) != 0) {
        GiveAmmoPack(playerEnt, Broc::bint((int)mp_util_wad::entity_get_playerClass(playerEnt)),
                     Broc::bint((int)rank));
    }
}

// CallbackCanPickupAmmoPack - ea: 0x963240
int CallbackCanPickupAmmoPack(Broc::entity playerEnt) {
    if (Broc::IsPlayer(playerEnt) == 0)
        return 0;
    Broc::string slot("primary");
    Broc::bbool full;
    IsFullWeaponAmmo(&full, playerEnt, slot);
    if (!(bool)full)
        return 1;
    Broc::string slotb("primaryb");
    IsFullWeaponAmmo(&full, playerEnt, slotb);
    return !(bool)full;
}

// CallbackPickupKit - ea: 0x962B60
void CallbackPickupKit(Broc::entity playerEnt, __int16 newClass) {
    if (Broc::IsPlayer(playerEnt) != 0) {
        Broc::string slot("primary");
        Broc::bfloat primaryScale;
        GetStartingWeaponAmmoScale(&primaryScale, playerEnt, slot,
                                   Broc::bint((int)mp_util_wad::entity_get_playerClass(playerEnt)),
                                   mp_util_wad::entity_get_rank(playerEnt));
        Broc::string slotb("primaryb");
        Broc::bfloat primaryBScale;
        GetStartingWeaponAmmoScale(&primaryBScale, playerEnt, slotb,
                                   Broc::bint((int)mp_util_wad::entity_get_playerClass(playerEnt)),
                                   mp_util_wad::entity_get_rank(playerEnt));
        Broc::string gslot("grenade");
        Broc::bfloat grenadeScale;
        GetStartingWeaponAmmoScale(&grenadeScale, playerEnt, gslot,
                                   Broc::bint((int)mp_util_wad::entity_get_playerClass(playerEnt)),
                                   mp_util_wad::entity_get_rank(playerEnt));
        if ((float)primaryScale > 1.0f)
            primaryScale = 1.0f;
        if ((float)primaryBScale > 1.0f)
            primaryBScale = 1.0f;
        if ((float)grenadeScale > 1.0f)
            grenadeScale = 1.0f;
        Broc::string s1("primary");
        Broc::SetWeaponSlotAmmo(&playerEnt, &s1, 0);
        s1.~string();
        Broc::string s2("primary");
        Broc::SetWeaponSlotClipAmmo(&playerEnt, &s2, 0);
        s2.~string();
        Broc::string s3("primaryb");
        Broc::SetWeaponSlotAmmo(&playerEnt, &s3, 0);
        s3.~string();
        Broc::string s4("primaryb");
        Broc::SetWeaponSlotClipAmmo(&playerEnt, &s4, 0);
        s4.~string();
        Broc::string s5("grenade");
        Broc::SetWeaponSlotAmmo(&playerEnt, &s5, 0);
        s5.~string();
        Broc::string s6("grenade");
        Broc::SetWeaponSlotClipAmmo(&playerEnt, &s6, 0);
        s6.~string();
        mp_util_wad::entity_set_playerClass(playerEnt, newClass);
        mp_util_wad::entity_set_nextPlayerClass(playerEnt, newClass);
        GiveLoadout(playerEnt);
        Broc::string p("primary");
        GiveWeaponAmmoScale(playerEnt, p,
                            Broc::bint((int)mp_util_wad::entity_get_playerClass(playerEnt)),
                            mp_util_wad::entity_get_rank(playerEnt),
                            Broc::bbool(true), (float)primaryScale);
        p.~string();
        Broc::string pb("primaryb");
        GiveWeaponAmmoScale(playerEnt, pb,
                            Broc::bint((int)mp_util_wad::entity_get_playerClass(playerEnt)),
                            mp_util_wad::entity_get_rank(playerEnt),
                            Broc::bbool(true), (float)primaryBScale);
        pb.~string();
        Broc::string g("grenade");
        GiveWeaponAmmoScale(playerEnt, g,
                            Broc::bint((int)mp_util_wad::entity_get_playerClass(playerEnt)),
                            mp_util_wad::entity_get_rank(playerEnt),
                            Broc::bbool(true), (float)grenadeScale);
        g.~string();
        slot.~string();
        slotb.~string();
        gslot.~string();
    }
}

// ArtilleryDispenser - ea: 0x9605D0
void ArtilleryDispenser(Broc::entity player) {
    HashStr e1;
    e1.mVal = 0xC9444C8D;
    Broc::endon(player, e1);
    HashStr e2;
    e2.mVal = 0x74AA0A6u;
    Broc::endon(player, e2);
    HashStr e3;
    e3.mVal = 0xC1E6FED9;
    Broc::endon(player, e3);
    HashStr e4;
    e4.mVal = 0x24B5BA64u;
    Broc::endon(player, e4);
    Broc::bint timerOrig;
    GetRankCount(&timerOrig, Broc::bint((int)mp_util_wad::entity_get_rank(player)),
                 Broc::bint(180), Broc::bint(180), Broc::bint(180));
    timerOrig = 180;
    Broc::bint timer((int)timerOrig);
    if ((int)timer != 0) {
        Broc::bint playerClass((int)mp_util_wad::entity_get_playerClass(player));
        Broc::bbool first_time(true);
        static Broc::bint additional_progress_bar_time(6000);
        for (;;) {
            if (!(bool)first_time) {
                HashStr trigger;
                trigger.mVal = 0x433F9306u;
                Broc::waittill(player, trigger);
                *mp_util_wad::GetEE_specialWeaponTime(player) = 0;
            }
            first_time = false;
            Broc::bint now;
            Broc::GetTime(&now);
            if ((int)*mp_util_wad::GetEE_specialWeaponTime(player) == 0) {
                *mp_util_wad::GetEE_specialWeaponTime(player) =
                    (int)now + (int)timer * 1000;
            } else {
                timer = ((int)*mp_util_wad::GetEE_specialWeaponTime(player) -
                         (int)now) / 1000;
            }
            if ((int)timer < 0)
                timer = 0;
            Broc::Code_SetSpecialRecharge(
                (int)*mp_util_wad::GetEE_specialWeaponTime(player) -
                    (int)timerOrig * 1000,
                (int)timerOrig * 1000 + (int)additional_progress_bar_time,
                (int)playerClass, Broc::GetPlayerIndex(player));
            HashStr notify;
            notify.mVal = 0x617A8CF6u;
            void* ntf = NotifyWhenTimerExpires__functor(player, (int)timer,
                                                        notify);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_loadout.bro",
                                __LINE__, "NotifyWhenTimerExpires", ntf);
            HashStr wait;
            wait.mVal = 0x617A8CF6u;
            Broc::waittill(player, wait);
            Broc::string sSlot("special");
    Broc::bint ammo(Broc::GetWeaponSlotAmmo(player, sSlot));
            sSlot.~string();
            if ((int)ammo == 0) {
                Broc::string script("Scout_Artillery_Ready");
                Broc::EffectEventPlay(&player, &script);
                script.~string();
                Broc::string s1("special");
                Broc::SetWeaponSlotAmmo(&player, &s1, 0);
                s1.~string();
                Broc::string s2("special");
                Broc::SetWeaponSlotClipAmmo(&player, &s2, 0);
                s2.~string();
                Broc::string s3("special");
                Broc::SetWeaponSlotClipAmmo(&player, &s3, 1);
                s3.~string();
            }
            timer = (int)timerOrig;
            Broc::Code_SetSpecialRecharge(0, 0, (int)playerClass,
                                          Broc::GetPlayerIndex(player));
        }
    }
}

// HealthAmmoDispenser - ea: 0x9614C0
void HealthAmmoDispenser(Broc::entity player, Broc::string weapon,
                         Broc::bbool health, Broc::bint rank0Time,
                         Broc::bint rank1Time, Broc::bint rank2Time) {
    HashStr e1;
    e1.mVal = 0xC9444C8D;
    Broc::endon(player, e1);
    HashStr e2;
    e2.mVal = 0x74AA0A6u;
    Broc::endon(player, e2);
    Broc::bint timer((int)rank0Time);
    __int16 rank = mp_util_wad::entity_get_rank(player);
    if (rank <= 1) {
        if (rank > 0)
            timer = (int)rank1Time;
    } else {
        timer = (int)rank2Time;
    }
    Broc::bint playerClass((int)mp_util_wad::entity_get_playerClass(player));
    Broc::bbool first_time(true);
    static Broc::bint additional_progress_bar_time(1000);
    for (;;) {
        *mp_util_wad::GetEE_specialWeaponTime(player) = 0;
        if (!(bool)first_time) {
            HashStr trigger;
            trigger.mVal = 0x433F9306u;
            Broc::waittill(player, trigger);
        }
        first_time = false;
        Broc::bint now;
        Broc::GetTime(&now);
        *mp_util_wad::GetEE_specialWeaponTime(player) =
            (int)now + (int)timer * 1000;
        Broc::Code_SetSpecialRecharge(
            (int)now, (int)timer * 1000 + (int)additional_progress_bar_time,
            (int)playerClass, Broc::GetPlayerIndex(player));
        Broc::string sSlot("special");
        Broc::SetWeaponSlotAmmo(&player, &sSlot, 0);
        sSlot.~string();
        Broc::string s2("special");
        Broc::SetWeaponSlotClipAmmo(&player, &s2, 0);
        s2.~string();
        HashStr notify;
        notify.mVal = 0x617A8CF6u;
        void* ntf = NotifyWhenTimerExpires__functor(player, (int)timer,
                                                    notify);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_loadout.bro",
                            __LINE__, "NotifyWhenTimerExpires", ntf);
        HashStr wait;
        wait.mVal = 0x617A8CF6u;
        Broc::waittill(player, wait);
        if ((bool)health) {
            Broc::string script("Medic_Healthpack_Ready");
            Broc::EffectEventPlay(&player, &script);
            script.~string();
        } else {
            Broc::string script("Support_Ammopack_Ready");
            Broc::EffectEventPlay(&player, &script);
            script.~string();
        }
        Broc::TakeWeapon(&player, &weapon);
        Broc::GiveWeapon(&player, &weapon);
        Broc::string s3("special");
        Broc::SetWeaponSlotAmmo(&player, &s3, 0);
        s3.~string();
        Broc::string s4("special");
        Broc::SetWeaponSlotClipAmmo(&player, &s4, 0);
        s4.~string();
        Broc::string s5("special");
        Broc::SetWeaponSlotClipAmmo(&player, &s5, 1);
        s5.~string();
        *mp_util_wad::GetEE_specialWeaponTime(player) = 0;
        Broc::Code_SetSpecialRecharge(0, 0, (int)playerClass,
                                      Broc::GetPlayerIndex(player));
    }
}

void* SpecialClassAudio__functor(Broc::entity player) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(SpecialClassAudio, player);
}
void* ArtilleryDispenser__functor(Broc::entity player) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(ArtilleryDispenser, player);
}
void* HealthAmmoDispenser__functor(Broc::entity player, Broc::string weapon,
                                   Broc::bbool health, Broc::bint rank0Time,
                                   Broc::bint rank1Time, Broc::bint rank2Time) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor6<Broc::entity, Broc::string, Broc::bbool, Broc::bint, Broc::bint, Broc::bint>));
    void* result = NULL;
    if (storage != NULL)
        result = ::new (storage) AeThreadFunctor6<Broc::entity, Broc::string, Broc::bbool, Broc::bint, Broc::bint, Broc::bint>(HealthAmmoDispenser, player, weapon, health, rank0Time, rank1Time, rank2Time);
    weapon.~string();
    return result;
}
void* NotifyWhenTimerExpires__functor(Broc::entity player, Broc::bint time,
                                      HashStr notifyString) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor3<Broc::entity, Broc::bint, HashStr>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor3<Broc::entity, Broc::bint, HashStr>(NotifyWhenTimerExpires, player, time, notifyString);
}
}

// ============================================================================
// _mp_ctf - capture the flag.
// ============================================================================
namespace _mp_ctf {

static Broc::bfloat lCTFObjectiveDontShow(-1.0f);
static Broc::bfloat lCTFObjectiveHeightFlag(0.0f);
static Broc::bfloat lCTFEntityOffZOffset(-1000.0f);

// main - ea: 0x94CD90
void main(Broc::entity self) {
    Broc::bbool team_game(true);
    Broc::Code_SetTeamGame((bool)team_game);
    mp_util_wad::pLevel->spawnTypeAllies = "spawn_ctf_allies_primary";
    mp_util_wad::pLevel->spawnTypeAxis = "spawn_ctf_axis_primary";
    mp_util_wad::pLevel->mustHaveBothTeamsToStart = false;
    mp_util_wad::pLevel->showFlagHint = 0;
    _mp_common::SetupCallbacks(team_game);
    mp_util_wad::pLevel->PickSpawnPoint = (void*)GetSpawnPoint;
    Broc::BrocExports& x = Broc::gBrocAPI.mBrocExports;
    x.mCallbackPlayerJoin = CallbackPlayerJoin;
    x.mCallbackPlayerEnter = CallbackPlayerEnter;
    x.mCallbackPlayerSpawn = CallbackPlayerSpawn;
    x.mCallbackGameState = CallbackGameState;
    x.mCallbackGameStateCTF = CallbackGameStateCTF;
    x.mCallbackDropItem = CallbackDropItem;
    x.mCallbackPickupScriptItem = CallbackPickupScriptItem;
    x.mCallbackDropFlag = CallbackDropFlag;
    x.mCallbackPlayerKilled = CallbackPlayerKilled;
    x.mCallbackPlayerLeave = CallbackPlayerLeave;
    x.mCallbackNextRound = CallbackNextRound;
    x.mCallbackHostOptionsChanged = CallbackHostOptionsChanged;
    x.mCallbackAreaCaptured = CallbackAreaCaptured;
    x.mCallbackHostMigrated = (void (*)())CallbackHostMigrated;
    x.mCallbackShowFlagHint = CallbackShowFlagHint;
    x.mCallbackDebugRender = (void (*)())CallbackDebugRender;
    Broc::string val("ctf_axis");
    HashStr key;
    key.mVal = 0x19F9F0E8u;
    Broc::entity e;
    mp_util_wad::pLevel->axis_flag_ent = *Broc::GetEnt(&e, &val, key, 0);
    val.~string();
    Broc::string val2("ctf_allies");
    HashStr key2;
    key2.mVal = 0x19F9F0E8u;
    Broc::entity e2;
    mp_util_wad::pLevel->allies_flag_ent = *Broc::GetEnt(&e2, &val2, key2, 0);
    val2.~string();
    mp_util_wad::pLevel->lastManStanding = false;
    void* f1 = FlagThreadLauncher__functor(mp_util_wad::pLevel->axis_flag_ent);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                        __LINE__, "FlagThreadLauncher", f1);
    void* f2 = FlagThreadLauncher__functor(mp_util_wad::pLevel->allies_flag_ent);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                        __LINE__, "FlagThreadLauncher", f2);
    Broc::string pszString("flag");
    Broc::string state("i_flag_axis_c");
    Broc::vector axisOrigin;
    mp_util_wad::entity_get_origin(&axisOrigin, mp_util_wad::pLevel->axis_flag_ent);
    ObjectiveAdd(0, state, pszString, axisOrigin,
                       (float)lCTFObjectiveDontShow, -1);
    state.~string();
    pszString.~string();
    Broc::string pszString2("flag");
    Broc::string state2("i_flag_allied_c");
    Broc::vector alliesOrigin;
    mp_util_wad::entity_get_origin(&alliesOrigin, mp_util_wad::pLevel->allies_flag_ent);
    ObjectiveAdd(1, state2, pszString2, alliesOrigin,
                       (float)lCTFObjectiveDontShow, -1);
    state2.~string();
    pszString2.~string();
    Broc::wait(1.0f);
    void* started = StartGame__functor(self);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                        __LINE__, "StartGame", started);
}

// StartGame - ea: 0x94D460
void StartGame(Broc::entity self) {
    (void)self;
    Broc::wait(0.5f);
    SetupRound();
    _mp_common::StartRound(Broc::bbool(true));
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = _mp_common::RunFrame__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                        __LINE__, "_mp_common::RunFrame", ftor);
    Broc::Code_EnterGame();
}

// CallbackAreaCaptured - ea: 0x94D5D0
void CallbackAreaCaptured(int index, int team) {
    (void)index;
    Broc::entity flag = team != 0
                            ? mp_util_wad::pLevel->allies_flag_ent
                            : mp_util_wad::pLevel->axis_flag_ent;

    Broc::bbool holderDefined;
    mp_util_wad::IsEEDefined_holder(&holderDefined, flag);
    if (!(bool)holderDefined &&
        Broc::gBrocAPI.mAssert("c:\\cod\\code\\script\\_mp_ctf.bro",
                               __LINE__, "flag holder broke"))
        __debugbreak();

    Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
    _mp_common::AddToPlayerStats(holder, Broc::bint(17), 1);

    if (team != 0) {
        Broc::string scoreTeam("axis");
        Broc::Code_IncTeamScore(scoreTeam, 1);
        scoreTeam.~string();

        Broc::entity level = mp_util_wad::pLevel != nullptr
                                  ? mp_util_wad::pLevel->_base.entity
                                  : Broc::entity();
        void* soundFtor = _mp_audio::PlayTeamSound__functor(
            level, Broc::string("axis"),
            Broc::string("MX_CTF_EnemyTeamScore"),
            Broc::string("MX_CTF_MyTeamScore"));
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                            __LINE__, "_mp_audio::PlayTeamSound", soundFtor);

        Broc::entity dialogLevel = mp_util_wad::pLevel != nullptr
                                        ? mp_util_wad::pLevel->_base.entity
                                        : Broc::entity();
        void* dialogFtor = _mp_audio::PlayTeamDialog__functor(
            dialogLevel, Broc::string("axis"),
            Broc::string("MP_CTF_AlliedFlagCaptured_Axis"),
            Broc::bfloat(0.5f));
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", dialogFtor);

        Broc::entity returnLevel = mp_util_wad::pLevel != nullptr
                                        ? mp_util_wad::pLevel->_base.entity
                                        : Broc::entity();
        void* returnFtor = _mp_audio::PlayTeamDialog__functor(
            returnLevel, Broc::string("allies"),
            Broc::string("MP_CTF_AlliedFlagCaptured_Allies"),
            Broc::bfloat(2.5f));
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", returnFtor);
        Broc::iprintln("MPCTF_AXIS_CAPTURED_FLAG");
    } else {
        Broc::string scoreTeam("allies");
        Broc::Code_IncTeamScore(scoreTeam, 1);
        scoreTeam.~string();

        Broc::entity level = mp_util_wad::pLevel != nullptr
                                  ? mp_util_wad::pLevel->_base.entity
                                  : Broc::entity();
        void* soundFtor = _mp_audio::PlayTeamSound__functor(
            level, Broc::string("allies"),
            Broc::string("MX_CTF_EnemyTeamScore"),
            Broc::string("MX_CTF_MyTeamScore"));
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                            __LINE__, "_mp_audio::PlayTeamSound", soundFtor);

        Broc::entity dialogLevel = mp_util_wad::pLevel != nullptr
                                        ? mp_util_wad::pLevel->_base.entity
                                        : Broc::entity();
        void* dialogFtor = _mp_audio::PlayTeamDialog__functor(
            dialogLevel, Broc::string("allies"),
            Broc::string("MP_CTF_AxisFlagCaptured_Allies"),
            Broc::bfloat(0.5f));
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", dialogFtor);

        Broc::entity returnLevel = mp_util_wad::pLevel != nullptr
                                        ? mp_util_wad::pLevel->_base.entity
                                        : Broc::entity();
        void* returnFtor = _mp_audio::PlayTeamDialog__functor(
            returnLevel, Broc::string("axis"),
            Broc::string("MP_CTF_AxisFlagCaptured_Axis"),
            Broc::bfloat(2.5f));
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", returnFtor);
        Broc::iprintln("MPCTF_ALLIES_CAPTURED_FLAG");
    }

    HashStr messageWhenReturned(0xFFFFFFFFu);
    *mp_util_wad::GetEE_message_when_returned(flag) = messageWhenReturned;

    Broc::vector angles;
    Broc::entity::__unnamed::angles_struct anglesField = {
        mp_util_wad::GetEE_goal(flag)->GetHandle()};
    anglesField.Get(&angles);
    Broc::vector origin;
    Broc::entity::__unnamed::origin_struct originField = {
        mp_util_wad::GetEE_goal(flag)->GetHandle()};
    originField.Get(&origin);
    UpdateFlagAndTrigger(flag, origin, angles);
    UnlinkFlag(flag);
}

// CallbackPlayerSpawn - ea: 0x94E1E0
void CallbackPlayerSpawn(Broc::entity guy, int team_changed) {
    _mp_common::CallbackPlayerSpawn(guy, team_changed);
    *mp_util_wad::GetEE_holder(guy) = Broc::gEntityUndef;
    Broc::entity::__unnamed::ctf_has_flag_struct ctfHasFlag = {
        guy.GetHandle()};
    const __int16 noFlag = 0;
    ctfHasFlag = noFlag;
    *mp_util_wad::GetEE_returnedSinceLastDeath(guy) = 0;
    *mp_util_wad::GetEE_cappedSinceLastDeath(guy) = 0;
}

// SetupRound - ea: 0x94E2E0
void SetupRound() {
    mp_util_wad::pLevel->spawnTypeAllies = "spawn_ctf_allies_primary";
    mp_util_wad::pLevel->spawnTypeAxis = "spawn_ctf_axis_primary";
    Broc::SetTutorialTextAllPlayers(-1);
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* sw = SwitchToSecondarySpawns__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                        __LINE__, "SwitchToSecondarySpawns", sw);
    *mp_util_wad::GetEE_waiting(mp_util_wad::pLevel->allies_flag_ent) = -1.0f;
    *mp_util_wad::GetEE_waiting(mp_util_wad::pLevel->axis_flag_ent) = -1.0f;
    UnlinkFlag(mp_util_wad::pLevel->allies_flag_ent);
    UnlinkFlag(mp_util_wad::pLevel->axis_flag_ent);
    Broc::vector aAngles;
    Broc::vector aOrigin;
    mp_util_wad::entity_get_angles(&aAngles, *mp_util_wad::GetEE_goal(mp_util_wad::pLevel->axis_flag_ent));
    mp_util_wad::entity_get_origin(&aOrigin, *mp_util_wad::GetEE_goal(mp_util_wad::pLevel->axis_flag_ent));
    UpdateFlagAndTrigger(mp_util_wad::pLevel->axis_flag_ent, aOrigin, aAngles);
    Broc::vector bAngles;
    Broc::vector bOrigin;
    mp_util_wad::entity_get_angles(&bAngles, *mp_util_wad::GetEE_goal(mp_util_wad::pLevel->allies_flag_ent));
    mp_util_wad::entity_get_origin(&bOrigin, *mp_util_wad::GetEE_goal(mp_util_wad::pLevel->allies_flag_ent));
    UpdateFlagAndTrigger(mp_util_wad::pLevel->allies_flag_ent, bOrigin, bAngles);
    Broc::Code_SetCompassVisibilty(0, false);
    Broc::Code_SetCompassVisibilty(1, false);
    Broc::Code_SetCompassVisibilty(2, false);
    Broc::Code_SetCompassVisibilty(3, false);
    Broc::Code_SetCompassVisibilty(4, false);
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        *mp_util_wad::GetEE_holder(p) = Broc::gEntityUndef;
        mp_util_wad::entity_set_ctf_has_flag(p, 0);
        *mp_util_wad::GetEE_last_dropped_time(p) = -1;
        i = (int)i + 1;
    }
    players.~dyn_array();
}

// CallbackHostMigrated - ea: 0x94E8B0
void CallbackHostMigrated() {
    _mp_common::CallbackHostMigrated();
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        Broc::vector aAngles;
        Broc::vector aOrigin;
        Broc::entity h = *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->axis_flag_ent);
        mp_util_wad::entity_get_angles(&aAngles, mp_util_wad::pLevel->axis_flag_ent);
        mp_util_wad::entity_get_origin(&aOrigin,
                                       mp_util_wad::pLevel->axis_flag_ent);
        Broc::vector bAngles;
        Broc::vector bOrigin;
        Broc::entity h2 = *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->allies_flag_ent);
        mp_util_wad::entity_get_angles(&bAngles, mp_util_wad::pLevel->allies_flag_ent);
        mp_util_wad::entity_get_origin(&bOrigin,
                                       mp_util_wad::pLevel->allies_flag_ent);
        Broc::Code_SendGameStateCTF(p, &aOrigin, &aAngles, h, &bOrigin, &bAngles, h2);
        i = (int)i + 1;
    }
    players.~dyn_array();
}

// CallbackNextRound - ea: 0x94EB60
void CallbackNextRound() {
    mp_util_wad::pLevel->roundOver = true;
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr n;
    n.mVal = 0x6FA23667u;
    Broc::notify(lvl, n);
    SetupRound();
    _mp_common::CallbackNextRound();
}

// CallbackHostOptionsChanged - ea: 0x94EBF0
void CallbackHostOptionsChanged(int forceMapChange) {
    _mp_common::CallbackHostOptionsChanged(forceMapChange);
    mp_util_wad::pLevel->lastManStanding = false;
}

// CallbackGameState - ea: 0x94EC30
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int roundOver,
                       int roundCount) {
    _mp_common::CallbackGameState(currentMatchTime, timeLimit, scoreLimit,
                                  roundLimit, friendlyFire, 0, teamBalance,
                                  respawnTime, alliesScore, axisScore,
                                  roundStarted, roundOver, roundCount);
    (void)lastManStanding;
}

// CallbackShowFlagHint - ea: 0x94EC90
int CallbackShowFlagHint() {
    return (int)mp_util_wad::pLevel->showFlagHint;
}

// SwitchToSecondarySpawns - ea: 0x94ECC0
void SwitchToSecondarySpawns(Broc::entity self) {
    (void)self;
    Broc::wait(5.0f);
    mp_util_wad::pLevel->spawnTypeAllies = "spawn_ctf_allies_secondary";
    mp_util_wad::pLevel->spawnTypeAxis = "spawn_ctf_axis_secondary";
}

// vLerp - ea: 0x94ED20
Broc::vector* vLerp(Broc::vector* result, Broc::vector a, Broc::vector b,
                    Broc::bfloat t) {
    *result = Broc::vector(a.x + (b.x - a.x) * (float)t,
                           a.y + (b.y - a.y) * (float)t,
                           a.z + (b.z - a.z) * (float)t);
    return result;
}

// IsFlagAtBase - ea: 0x94EE90
Broc::bbool* IsFlagAtBase(Broc::bbool* result, Broc::entity flag) {
    Broc::vector goal;
    Broc::vector f;
    mp_util_wad::entity_get_origin(&goal, *mp_util_wad::GetEE_goal(flag));
    mp_util_wad::entity_get_origin(&f, flag);
    Broc::vector baseOfs = f - goal;
    baseOfs.z = 0.0f;
    *result = Broc::bbool(Broc::Length(&baseOfs) < 0.15f);
    return result;
}

// TeamObjectiveUpdate - ea: 0x94EF80
void TeamObjectiveUpdate(Broc::entity guy, int iTeamFlag) {
    Broc::entity flag;
    Broc::bint iObjective(0);
    Broc::bint iObjectiveCarrier(0);
    Broc::string sObjectiveStateThisTeam;
    Broc::string sObjectiveStateOtherTeam;
    Broc::string sObjectiveStateNeutralThisTeam;
    Broc::string sObjectiveStateNeutralOtherTeam;
    sObjectiveStateThisTeam = "";
    sObjectiveStateOtherTeam = "";
    sObjectiveStateNeutralThisTeam = "";
    sObjectiveStateNeutralOtherTeam = "";
    Broc::string sObjectiveCarrierState;
    sObjectiveCarrierState = "";
    if (iTeamFlag == 1) {
        flag = mp_util_wad::pLevel->axis_flag_ent;
        iObjective = 0;
        iObjectiveCarrier = 2;
        sObjectiveStateThisTeam = "i_flag_axis_c";
        sObjectiveStateOtherTeam = "i_flag_allied_c";
        sObjectiveStateNeutralThisTeam = "i_neutral_axis_c";
        sObjectiveStateNeutralOtherTeam = "i_neutral_allied_c";
    } else {
        flag = mp_util_wad::pLevel->allies_flag_ent;
        iObjective = 1;
        iObjectiveCarrier = 3;
        sObjectiveStateThisTeam = "i_flag_allied_c";
        sObjectiveStateOtherTeam = "i_flag_axis_c";
        sObjectiveStateNeutralThisTeam = "i_neutral_allied_c";
        sObjectiveStateNeutralOtherTeam = "i_neutral_axis_c";
    }
    Broc::bbool atBase;
    IsFlagAtBase(&atBase, flag);
    Broc::bbool ourTeam(false);
    Broc::string team;
    mp_util_wad::entity_get_team(&team, guy);
    if (team == "axis")
        ourTeam = Broc::bbool(iTeamFlag == 1);
    else
        ourTeam = Broc::bbool(iTeamFlag == 2);
    team.~string();
    ObjectiveDelete((int)iObjective, Broc::GetPlayerIndex(guy));
    ObjectiveDelete((int)iObjectiveCarrier, Broc::GetPlayerIndex(guy));
    if (*mp_util_wad::GetEE_holder(flag) == Broc::gEntityUndef) {
        Broc::string pszString("flag");
        Broc::vector origin;
        mp_util_wad::entity_get_origin(&origin, flag);
        if ((bool)ourTeam && (bool)atBase &&
            mp_util_wad::entity_get_ctf_has_flag(guy) != 0) {
            Broc::string state("i_objective_c");
            ObjectiveAdd((int)iObjective, state, pszString, origin,
                               (float)lCTFObjectiveHeightFlag,
                               Broc::GetPlayerIndex(guy));
            state.~string();
        } else if ((bool)atBase) {
            ObjectiveAdd((int)iObjective, sObjectiveStateThisTeam, pszString, origin,
                               (float)lCTFObjectiveDontShow,
                               Broc::GetPlayerIndex(guy));
        } else {
            Broc::vector goal;
            mp_util_wad::entity_get_origin(&goal, *mp_util_wad::GetEE_goal(flag));
            ObjectiveAdd((int)iObjective, sObjectiveStateNeutralThisTeam, pszString, goal, (float)lCTFObjectiveDontShow,
                               Broc::GetPlayerIndex(guy));
            ObjectiveAdd((int)iObjectiveCarrier, sObjectiveStateThisTeam, pszString, origin, (float)lCTFObjectiveDontShow,
                               Broc::GetPlayerIndex(guy));
        }
        pszString.~string();
    } else {
        Broc::string pszString("flag");
        Broc::vector goal;
        mp_util_wad::entity_get_origin(&goal, *mp_util_wad::GetEE_goal(flag));
        ObjectiveAdd((int)iObjective, sObjectiveStateNeutralThisTeam, pszString, goal, (float)lCTFObjectiveDontShow,
                               Broc::GetPlayerIndex(guy));
        Broc::string playerTeam;
        mp_util_wad::entity_get_team(&playerTeam, guy);
        Broc::string hteam;
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        mp_util_wad::entity_get_team(&hteam, holder);
        if (hteam == playerTeam && holder != guy) {
            Broc::vector hpos;
            mp_util_wad::entity_get_origin(&hpos, holder);
            ObjectiveAdd((int)iObjectiveCarrier, sObjectiveStateThisTeam, pszString, hpos, (float)lCTFObjectiveDontShow,
                               Broc::GetPlayerIndex(guy));
        }
        hteam.~string();
        playerTeam.~string();
        pszString.~string();
    }
    sObjectiveCarrierState.~string();
    sObjectiveStateNeutralOtherTeam.~string();
    sObjectiveStateNeutralThisTeam.~string();
    sObjectiveStateOtherTeam.~string();
    sObjectiveStateThisTeam.~string();
}

// ObjectiveUpdater - ea: 0x94FAD0
void ObjectiveUpdater(Broc::entity guy) {
    Broc::bbool firstLoop(true);
    for (;;) {
        if (!(bool)firstLoop)
            Broc::wait(0.1f);
        firstLoop = false;
        if (!(bool)mp_util_wad::pLevel->roundOver &&
            (bool)mp_util_wad::pLevel->roundStarted) {
            TeamObjectiveUpdate(guy, 1);
            TeamObjectiveUpdate(guy, 2);
        } else {
            Broc::string pszString("flag");
            Broc::string state("i_flag_axis_c");
            Broc::vector pos;
            mp_util_wad::entity_get_origin(&pos, mp_util_wad::pLevel->axis_flag_ent);
            ObjectiveAdd(0, state, pszString, pos,
                               (float)lCTFObjectiveDontShow,
                               Broc::GetPlayerIndex(guy));
            state.~string();
            pszString.~string();
            Broc::string pszString2("flag");
            Broc::string state2("i_flag_allied_c");
            Broc::vector pos2;
            mp_util_wad::entity_get_origin(&pos2, mp_util_wad::pLevel->allies_flag_ent);
            ObjectiveAdd(1, state2, pszString2, pos2,
                               (float)lCTFObjectiveDontShow,
                               Broc::GetPlayerIndex(guy));
            state2.~string();
            pszString2.~string();
        }
    }
}

// CallbackPlayerKilled - ea: 0x94FDF0
void CallbackPlayerKilled(Broc::entity guy, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health) {
    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, guy);
    if ((bool)hasHolder)
        HandleDropFlag(guy);
    *mp_util_wad::GetEE_returnedSinceLastDeath(guy) = 0;
    *mp_util_wad::GetEE_cappedSinceLastDeath(guy) = 0;
    _mp_common::CallbackPlayerKilled(guy, inflictor, attacker, weapon, mod,
                                     health);
}

// CallbackDropFlag - ea: 0x94FEE0
void CallbackDropFlag(Broc::entity guy) {
    HandleDropFlag(guy);
}

// CallbackPlayerLeave - ea: 0x94FF10
void CallbackPlayerLeave(Broc::entity guy) {
    mp_util_wad::entity_set_ctf_has_flag(guy, 0);
    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, guy);
    if ((bool)hasHolder)
        HandleDropFlag(guy);
    _mp_common::CallbackPlayerLeave(guy);
}

// CallbackGameStateCTF - ea: 0x94FFA0
void CallbackGameStateCTF(Broc::vector allied_flag, Broc::vector allied_angles,
                          Broc::entity allied_flag_holder, Broc::vector axis_flag,
                          Broc::vector axis_angles, Broc::entity axis_flag_holder) {
    if (Broc::IsDefined(mp_util_wad::pLevel->axis_flag_ent)) {
        Broc::entity holder = *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->axis_flag_ent);
        if (holder != Broc::gEntityUndef && holder != axis_flag_holder)
            UnlinkFlag(holder);
    }
    Broc::string val("ctf_axis");
    HashStr key;
    key.mVal = 0x19F9F0E8u;
    Broc::entity e;
    mp_util_wad::pLevel->axis_flag_ent = *Broc::GetEnt(&e, &val, key, 0);
    val.~string();
    *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->axis_flag_ent) =
        axis_flag_holder;
    Broc::bbool defined;
    mp_util_wad::IsEEDefined_holder(&defined, mp_util_wad::pLevel->axis_flag_ent);
    if ((bool)defined) {
        Broc::entity pickerupper =
            *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->axis_flag_ent);
        __int16 hasFlag = 1;
        Broc::entity::__unnamed::ctf_has_flag_struct ctfHasFlagField = {
            pickerupper.GetHandle()};
        ctfHasFlagField = hasFlag;
        *mp_util_wad::GetEE_holder(pickerupper) =
            mp_util_wad::pLevel->axis_flag_ent;
        HandlePickupFlag(1, pickerupper, Broc::bbool(false),
                         Broc::bbool(false));
    }
    mp_util_wad::entity_set_origin(mp_util_wad::pLevel->axis_flag_ent, axis_flag);
    mp_util_wad::entity_set_angles(mp_util_wad::pLevel->axis_flag_ent, axis_angles);
    if (Broc::IsDefined(mp_util_wad::pLevel->allies_flag_ent)) {
        Broc::entity holder = *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->allies_flag_ent);
        if (holder != Broc::gEntityUndef && holder != allied_flag_holder)
            UnlinkFlag(holder);
    }
    Broc::string val2("ctf_allies");
    HashStr key2;
    key2.mVal = 0x19F9F0E8u;
    Broc::entity e2;
    mp_util_wad::pLevel->allies_flag_ent = *Broc::GetEnt(&e2, &val2, key2, 0);
    val2.~string();
    *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->allies_flag_ent) =
        allied_flag_holder;
    Broc::bbool defined2;
    mp_util_wad::IsEEDefined_holder(&defined2, mp_util_wad::pLevel->allies_flag_ent);
    if ((bool)defined2) {
        Broc::entity pickerupper =
            *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->allies_flag_ent);
        __int16 hasFlag = 1;
        Broc::entity::__unnamed::ctf_has_flag_struct ctfHasFlagField = {
            pickerupper.GetHandle()};
        ctfHasFlagField = hasFlag;
        *mp_util_wad::GetEE_holder(pickerupper) =
            mp_util_wad::pLevel->allies_flag_ent;
        HandlePickupFlag(2, pickerupper, Broc::bbool(false),
                         Broc::bbool(false));
    }
    mp_util_wad::entity_set_origin(mp_util_wad::pLevel->allies_flag_ent, allied_flag);
    mp_util_wad::entity_set_angles(mp_util_wad::pLevel->allies_flag_ent, allied_angles);
}

// UpdateFlagAndTrigger - ea: 0x950BB0
void UpdateFlagAndTrigger(Broc::entity self, Broc::vector origin,
                          Broc::vector angles) {
    mp_util_wad::entity_set_origin(self, origin);
    mp_util_wad::entity_set_angles(self, angles);
}

// LaunchFlagAndTrigger - ea: 0x950BF0
void LaunchFlagAndTrigger(Broc::entity self, Broc::vector origin,
                          Broc::vector angles, Broc::vector velocity) {
    mp_util_wad::entity_set_origin(self, origin);
    mp_util_wad::entity_set_angles(self, angles);
    Broc::Launch(&self, &velocity);
}

// EntityOff - ea: 0x950C70
void EntityOff(Broc::entity self) {
    Broc::vector off(0.0f, 0.0f, (float)lCTFEntityOffZOffset);
    Broc::vector origin;
    mp_util_wad::entity_get_origin(&origin, self);
    mp_util_wad::entity_set_origin(self, origin + off);
}

// CompassUnderlay - ea: 0x950CD0
void CompassUnderlay(Broc::entity p) {
    Broc::bint flagid(0);
    Broc::bint alliesid(0);
    Broc::bint axisid(2);
    if (mp_util_wad::pLevel->axis == "german")
        axisid = 3;
    else if (mp_util_wad::pLevel->axis == "italian")
        axisid = 2;
    else if (mp_util_wad::pLevel->axis == "vichy")
        axisid = 4;
    if (mp_util_wad::pLevel->allies == "american")
        alliesid = 0;
    else
        alliesid = 1;
    Broc::string team;
    mp_util_wad::entity_get_team(&team, p);
    if (team == "axis")
        flagid = (int)alliesid;
    else
        flagid = (int)axisid;
    team.~string();
    for (;;) {
        Broc::bbool hasHolder;
        mp_util_wad::IsEEDefined_holder(&hasHolder, p);
        if (!(bool)hasHolder || (bool)mp_util_wad::pLevel->roundOver)
            break;
        Broc::Code_SetCompassVisibilty((int)flagid, true);
        Broc::wait(0.05f);
    }
    Broc::Code_SetCompassVisibilty((int)flagid, false);
    mp_util_wad::entity_set_ctf_has_flag(p, 0);
}

// CallbackPlayerJoin - ea: 0x955170
void CallbackPlayerJoin(Broc::entity guy, unsigned int playerState,
                        int playerClass) {
    _mp_common::CallbackPlayerJoin(guy, playerState, (__int16)playerClass);
    mp_util_wad::entity_set_ctf_has_flag(guy, 0);
    *mp_util_wad::GetEE_holder(guy) = Broc::gEntityUndef;
    if (Broc::Code_IsLocalPlayer(guy)) {
        void* ftor = ObjectiveUpdater__functor(guy);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_ctf.bro",
                            __LINE__, "ObjectiveUpdater", ftor);
    }
}

// CallbackPlayerEnter - ea: 0x9552F0
void CallbackPlayerEnter(Broc::entity guy, int hot_joiner) {
    _mp_common::CallbackPlayerEnter(guy, hot_joiner);
    Broc::entity axisHolder =
        *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->axis_flag_ent);
    Broc::entity alliesHolder =
        *mp_util_wad::GetEE_holder(mp_util_wad::pLevel->allies_flag_ent);
    Broc::vector aOrigin;
    Broc::vector aAngles;
    Broc::vector bOrigin;
    Broc::vector bAngles;
    mp_util_wad::entity_get_origin(&aOrigin, mp_util_wad::pLevel->axis_flag_ent);
    mp_util_wad::entity_get_angles(&aAngles, mp_util_wad::pLevel->axis_flag_ent);
    mp_util_wad::entity_get_origin(&bOrigin, mp_util_wad::pLevel->allies_flag_ent);
    mp_util_wad::entity_get_angles(&bAngles, mp_util_wad::pLevel->allies_flag_ent);
    Broc::Code_SendGameStateCTF(guy, &bOrigin, &bAngles, alliesHolder,
                                &aOrigin, &aAngles, axisHolder);
}

// GetSpawnPoint - ea: 0x956360
Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* self,
                            const Broc::string* team) {
    Broc::dyn_array<Broc::entity> spawnpoints;
    Broc::string spawnType = mp_util_wad::pLevel->spawnTypeAllies;
    if (*team == "axis")
        spawnType = mp_util_wad::pLevel->spawnTypeAxis;
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&spawnType, key.mVal, &spawnpoints, 0);
    if (mp_util_wad::pLevel->spawnTypeAllies != "spawn_ctf_allies_primary") {
        Broc::dyn_array<Broc::entity> ctf_spawnpoints;
        Broc::string ctfType("spawn_ctf_allies_primary");
        if (*team == "axis")
            ctfType = "spawn_ctf_axis_primary";
        HashStr key2;
        key2.mVal = 0xF756C677;
        Broc::GetEntArray(&ctfType, key2.mVal, &ctf_spawnpoints, 0);
        Broc::bint i(0);
        while ((int)i < Broc::size(ctf_spawnpoints)) {
            spawnpoints.push_back(ctf_spawnpoints[(unsigned int)(int)i]);
            i = (int)i + 1;
        }
    }
    _mp_spawnlogic::GetSpawnpointNearTeam(result, self, team, &spawnpoints);
    return result;
}

// UnlinkFlag - ea: 0x955400
void UnlinkFlag(Broc::entity flag) {
    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, flag);
    if ((bool)hasHolder) {
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        Broc::bbool hasSound;
        mp_util_wad::IsEEDefined_sound_handle(&hasSound, holder);
        if ((bool)hasSound) {
            Broc::EffectEventStopEmitting(
                (int)*mp_util_wad::GetEE_sound_handle(holder));
            *mp_util_wad::GetEE_sound(holder) = 0;
        }
        Broc::string weapon = *mp_util_wad::GetEE_weaponstr(flag);
        Broc::TakeWeapon(&holder, &weapon);
        weapon.~string();
        _mp_common::SelectFirstAvailableWeapon(holder);
        mp_util_wad::entity_set_ctf_has_flag(holder, 0);
        *mp_util_wad::GetEE_holder(holder) = Broc::gEntityUndef;
    }
    *mp_util_wad::GetEE_holder(flag) = Broc::gEntityUndef;
    *mp_util_wad::GetEE_waiting(flag) = -1.0f;
}

// DestroyIcon - ea: 0x9536D0
void DestroyIcon(Broc::entity toucher) {
    HashStr n;
    n.mVal = 0xA738D71C;
    Broc::notify(toucher, n);
    mp_util_wad::pLevel->showFlagHint = 0;
}

// WaitForNoTouchFlag - ea: 0x953720
void WaitForNoTouchFlag(Broc::entity toucher) {
    HashStr n;
    n.mVal = 0xA738D71C;
    Broc::notify(toucher, n);
    Broc::wait(0.5f);
    Broc::endon(toucher, n);
    for (;;) {
        Broc::bint now;
        Broc::GetTime(&now);
        if ((int)*mp_util_wad::GetEE_last_touch_time(toucher) + 300 <
            (int)now)
            break;
        Broc::wait(0.5f);
    }
    Broc::SetTutorialText(-1, Broc::GetPlayerIndex(toucher));
    DestroyIcon(toucher);
}

// WaitForFlagTimeOut - ea: 0x953440
void WaitForFlagTimeOut(Broc::entity flag) {
    HashStr n;
    n.mVal = 0x4F2B87DFu;
    Broc::notify(flag, n);
    Broc::wait(0.1f);
    Broc::endon(flag, n);
    HashStr e1;
    e1.mVal = 0xFA57E7C7;
    Broc::endon(flag, e1);
    HashStr e2;
    e2.mVal = 0x87404C8E;
    Broc::endon(flag, e2);
    Broc::wait(25.0f);
    Broc::string tn;
    mp_util_wad::entity_get_targetname(&tn, flag);
    bool axis = tn == "axis" || tn == "ctf_axis";
    tn.~string();
    if (axis)
        Broc::Code_PickupItem(1, Broc::gEntityUndef);
    else
        Broc::Code_PickupItem(2, Broc::gEntityUndef);
}

// PickupFlag - ea: 0x953870
void PickupFlag(Broc::entity self) {
    Broc::dyn_array<Broc::entity> players;
    Broc::entity parent;
    Broc::bint now;
    Broc::GetTime(&now);
    if ((int)*mp_util_wad::GetEE_last_touch_time(self) + 100 >
            (int)now ||
        (int)*mp_util_wad::GetEE_last_touch_time(self) > (int)now) {
        Broc::bint t;
        Broc::GetTime(&t);
        *mp_util_wad::GetEE_last_touch_time(self) = (int)t;
        if ((bool)mp_util_wad::pLevel->roundStarted) {
            Broc::string tn;
            mp_util_wad::entity_get_targetname(&tn, self);
            if (tn == "axis")
                parent = mp_util_wad::pLevel->axis_flag_ent;
            tn.~string();
            Broc::string tn2;
            mp_util_wad::entity_get_targetname(&tn2, self);
            if (tn2 == "allies")
                parent = mp_util_wad::pLevel->allies_flag_ent;
            tn2.~string();
            Broc::GetPlayerArray(&players);
            Broc::bint i(0);
            while ((int)i < Broc::size(players)) {
                Broc::entity p = players[(unsigned int)(int)i];
                Broc::bint state;
                mp_util_wad::entity_get_playerState(&state, p);
                if ((int)state == 3 && Broc::Code_IsLocalPlayer(p) &&
                    !Broc::Code_IsInVehicle(p)) {
                    Broc::vector selfPos;
                    Broc::vector goalPos;
                    mp_util_wad::entity_get_origin(&selfPos, self);
                    mp_util_wad::entity_get_origin(&goalPos,
                                                   *mp_util_wad::GetEE_goal(parent));
                    Broc::bint dist((int)Broc::Distance(&goalPos, &selfPos));
                    Broc::string ptn;
                    Broc::string pteam;
                    mp_util_wad::entity_get_targetname(&ptn, self);
                    mp_util_wad::entity_get_team(&pteam, p);
                    bool sameTeam = pteam == ptn;
                    ptn.~string();
                    pteam.~string();
                    if (!((int)dist < 60 && sameTeam)) {
                        Broc::vector ppos;
                        mp_util_wad::entity_get_origin(&ppos, p);
                        dist = (int)Broc::Distance(&ppos, &selfPos);
                        if ((int)dist < 60) {
                            Broc::string ptn2;
                            Broc::string pteam2;
                            mp_util_wad::entity_get_targetname(&ptn2, self);
                            mp_util_wad::entity_get_team(&pteam2, p);
                            bool diffTeam = pteam2 != ptn2;
                            ptn2.~string();
                            pteam2.~string();
                            if (!diffTeam ||
                                Broc::UseButtonPressed(p) != 0) {
                                Broc::string atn;
                                mp_util_wad::entity_get_targetname(&atn, self);
                                if (atn == "axis")
                                    Broc::Code_PickupItem(1, p);
                                else
                                    Broc::Code_PickupItem(2, p);
                                atn.~string();
                                break;
                            }
                            Broc::bint t;
                            Broc::GetTime(&t);
                            *mp_util_wad::GetEE_last_touch_time(p) = (int)t;
                            mp_util_wad::pLevel->showFlagHint = 1;
                            void* ftor = WaitForNoTouchFlag__functor(p);
                            Broc::thread_create(false,
                                                "c:\\cod\\code\\script\\_mp_ctf.bro",
                                                __LINE__, "WaitForNoTouchFlag",
                                                ftor);
                        }
                    }
                }
                i = (int)i + 1;
            }
        }
    }
    players.~dyn_array();
}

void* main__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(main, self);
}
AeThreadFunctor1<Broc::entity>* StartGame__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(StartGame, self);
}
AeThreadFunctor1<Broc::entity>* FlagThreadLauncher__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(FlagThreadLauncher, self);
}
AeThreadFunctor1<Broc::entity>* SwitchToSecondarySpawns__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(SwitchToSecondarySpawns, self);
}
AeThreadFunctor1<Broc::entity>* ObjectiveUpdater__functor(Broc::entity guy) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(ObjectiveUpdater, guy);
}
AeThreadFunctor1<Broc::entity>* CompassUnderlay__functor(Broc::entity p) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(CompassUnderlay, p);
}
AeThreadFunctor1<Broc::entity>* WaitForFlagTimeOut__functor(Broc::entity flag) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WaitForFlagTimeOut, flag);
}
AeThreadFunctor1<Broc::entity>* WaitForNoTouchFlag__functor(Broc::entity toucher) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WaitForNoTouchFlag, toucher);
}
void* PickupFlag__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(PickupFlag, self);
}
void* Goal__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(Goal, self);
}

// Goal - ea: 0x9541F0
void Goal(Broc::entity self) {
    Broc::dyn_array<Broc::entity> players;
    Broc::entity axis_holder;
    Broc::entity allies_holder;
    Broc::string flag_weapon((const char*)NULL);
    if (!(bool)mp_util_wad::pLevel->roundStarted)
        return;

    Broc::bint dist;
    allies_holder = *mp_util_wad::GetEE_holder(
        mp_util_wad::pLevel->axis_flag_ent);
    axis_holder = *mp_util_wad::GetEE_holder(
        mp_util_wad::pLevel->allies_flag_ent);

    Broc::vector selfOrigin;
    Broc::entity::__unnamed::origin_struct selfOriginField = {
        self.GetHandle()};
    selfOriginField.Get(&selfOrigin);
    Broc::bint axis_dist;
    if (Broc::IsDefined(axis_holder)) {
        Broc::vector holderOrigin;
        Broc::entity::__unnamed::origin_struct holderOriginField = {
            axis_holder.GetHandle()};
        holderOriginField.Get(&holderOrigin);
        axis_dist = (int)Broc::Distance(&holderOrigin, &selfOrigin);
    } else {
        axis_dist = 0;
    }

    Broc::bint allies_dist;
    if (Broc::IsDefined(allies_holder)) {
        Broc::vector holderOrigin;
        Broc::entity::__unnamed::origin_struct holderOriginField = {
            allies_holder.GetHandle()};
        holderOriginField.Get(&holderOrigin);
        allies_dist = (int)Broc::Distance(&holderOrigin, &selfOrigin);
    } else {
        allies_dist = 0;
    }

    Broc::string targetName;
    Broc::entity::__unnamed::targetname_struct targetNameField = {
        self.GetHandle()};
    targetNameField.Get(&targetName);
    bool isAxisBase = targetName == "axis";
    targetName.~string();
    if (isAxisBase && Broc::Code_IsLocalPlayer(axis_holder)) {
        Broc::bint now;
        Broc::GetTime(&now);
        if ((int)*mp_util_wad::GetEE_pickupCaptureDelayTime(axis_holder) >=
            (int)now)
            return;

        Broc::string slot("flag");
        Broc::GetWeaponSlotWeapon(axis_holder, slot, flag_weapon);
        slot.~string();
        if (flag_weapon == "mp_flag_allies" && (int)axis_dist < 64) {
            Broc::vector flagOrigin;
            Broc::entity::__unnamed::origin_struct flagOriginField = {
                mp_util_wad::pLevel->axis_flag_ent.GetHandle()};
            flagOriginField.Get(&flagOrigin);
            Broc::entity goal = *mp_util_wad::GetEE_goal(
                mp_util_wad::pLevel->axis_flag_ent);
            Broc::vector goalOrigin;
            Broc::entity::__unnamed::origin_struct goalOriginField = {
                goal.GetHandle()};
            goalOriginField.Get(&goalOrigin);
            dist = (int)Broc::Distance(&goalOrigin, &flagOrigin);
            if ((int)dist < 64) {
                Broc::Code_AreaCaptured(0, 1, 0);
                *mp_util_wad::GetEE_cappedSinceLastDeath(axis_holder) = 1;
            }
        }
    }

    targetName = Broc::string((const char*)NULL);
    targetNameField.Get(&targetName);
    bool isAlliesBase = targetName == "allies";
    targetName.~string();
    if (isAlliesBase && Broc::Code_IsLocalPlayer(allies_holder)) {
        Broc::bint now;
        Broc::GetTime(&now);
        if ((int)*mp_util_wad::GetEE_pickupCaptureDelayTime(allies_holder) <
            (int)now) {
            Broc::string slot("flag");
            Broc::GetWeaponSlotWeapon(allies_holder, slot, flag_weapon);
            slot.~string();
            if (flag_weapon == "mp_flag_axis" && (int)allies_dist < 64) {
                Broc::vector flagOrigin;
                Broc::entity::__unnamed::origin_struct flagOriginField = {
                    mp_util_wad::pLevel->allies_flag_ent.GetHandle()};
                flagOriginField.Get(&flagOrigin);
                Broc::entity goal = *mp_util_wad::GetEE_goal(
                    mp_util_wad::pLevel->allies_flag_ent);
                Broc::vector goalOrigin;
                Broc::entity::__unnamed::origin_struct goalOriginField = {
                    goal.GetHandle()};
                goalOriginField.Get(&goalOrigin);
                dist = (int)Broc::Distance(&goalOrigin, &flagOrigin);
                if ((int)dist < 64) {
                    Broc::Code_AreaCaptured(0, 0, 0);
                    *mp_util_wad::GetEE_cappedSinceLastDeath(allies_holder) = 1;
                }
            }
        }
    }
}

// FlagThreadLauncher - ea: 0x954970
void FlagThreadLauncher(Broc::entity self) {
    Broc::vector mins(-20.0f, -20.0f, 0.0f);
    Broc::vector maxs(20.0f, 20.0f, 50.0f);
    Broc::vector origin;
    Broc::vector angles;
    Broc::entity::__unnamed::origin_struct originField = {
        self.GetHandle()};
    originField.Get(&origin);
    Broc::entity::__unnamed::angles_struct anglesField = {
        self.GetHandle()};
    anglesField.Get(&angles);

    Broc::string classname("trigger_multiple");
    Broc::entity trigger;
    Broc::Spawn(&trigger, &classname, &origin, &mins, &maxs, 2,
                static_cast<TPakInfo>(INVALID_PAK_INFO));
    classname.~string();
    *mp_util_wad::GetEE_trigger(self) = trigger;

    Broc::string classnameGoal("trigger_multiple");
    Broc::entity goal;
    Broc::Spawn(&goal, &classnameGoal, &origin, &mins, &maxs, 2,
                static_cast<TPakInfo>(0));
    classnameGoal.~string();
    *mp_util_wad::GetEE_goal(self) = goal;

    Broc::entity::__unnamed::angles_struct goalAngles = {
        goal.GetHandle()};
    goalAngles = &angles;

    Broc::string script("FLAG_FLAPPING");
    Broc::EffectEventPlay(&self, &script);
    script.~string();
    Broc::LinkTo(mp_util_wad::GetEE_trigger(self), &self);
    *mp_util_wad::GetEE_waiting(self) = -1.0f;

    Broc::entity::__unnamed::targetname_struct targetNameField = {
        self.GetHandle()};
    Broc::string targetName;
    targetNameField.Get(&targetName);
    bool axisFlag = targetName == "axis";
    if (!axisFlag) {
        Broc::string ctfTargetName;
        targetNameField.Get(&ctfTargetName);
        axisFlag = ctfTargetName == "ctf_axis";
        ctfTargetName.~string();
    }
    targetName.~string();

    Broc::string team(axisFlag ? "axis" : "allies");
    Broc::entity::__unnamed::targetname_struct triggerName = {
        trigger.GetHandle()};
    triggerName = team;
    Broc::entity::__unnamed::targetname_struct goalName = {
        goal.GetHandle()};
    goalName = team;

    HashStr returnedMessage(axisFlag ? 0x7483EACFu : 0xFC489514u);
    mp_util_wad::GetEE_message_when_returned(self)->mVal =
        returnedMessage.mVal;
    *mp_util_wad::GetEE_weaponstr(self) =
        axisFlag ? "mp_flag_axis" : "mp_flag_allies";
    *mp_util_wad::GetEE_holder(self) = Broc::gEntityUndef;

    HashStr pickupFunction;
    Broc::string_hash(&pickupFunction, "_mp_ctf::PickupFlag");
    HashStr eventLabel(0xF2F5EAB4u);
    Broc::AddEventHandler(mp_util_wad::GetEE_trigger(self), eventLabel.mVal,
                          pickupFunction.mVal);

    HashStr goalFunction;
    Broc::string_hash(&goalFunction, "_mp_ctf::Goal");
    Broc::AddEventHandler(mp_util_wad::GetEE_goal(self), eventLabel.mVal,
                          goalFunction.mVal);
}

// CallbackDebugRender - ea: 0x955B20
void CallbackDebugRender() {
    _mp_common::CallbackDebugRender();
    if (Broc::GetCvarInt("mp_debugrender") != 1)
        return;

    Broc::string temp((const char*)NULL);
    Broc::bint x(40);
    Broc::bint y(70);
    Broc::bint y_inc(20);
    Broc::vector green(0.0f, 1.0f, 0.0f);
    Broc::vector red(1.0f, 0.0f, 0.0f);

    Broc::bbool triggerDefined;
    mp_util_wad::IsEEDefined_trigger(
        &triggerDefined, mp_util_wad::pLevel->allies_flag_ent);
    if ((bool)triggerDefined) {
        Broc::entity trigger = *mp_util_wad::GetEE_trigger(
            mp_util_wad::pLevel->allies_flag_ent);
        Broc::vector origin;
        Broc::entity::__unnamed::origin_struct originField = {
            trigger.GetHandle()};
        originField.Get(&origin);
        Broc::Code_DebugRenderSphere(&origin, 10.0f, &green, 1.0f);
        Broc::Code_DebugRenderEntityBBox(trigger, &green, 0.4f);
    }

    mp_util_wad::IsEEDefined_trigger(
        &triggerDefined, mp_util_wad::pLevel->axis_flag_ent);
    if ((bool)triggerDefined) {
        Broc::entity trigger = *mp_util_wad::GetEE_trigger(
            mp_util_wad::pLevel->axis_flag_ent);
        Broc::vector origin;
        Broc::entity::__unnamed::origin_struct originField = {
            trigger.GetHandle()};
        originField.Get(&origin);
        Broc::Code_DebugRenderSphere(&origin, 10.0f, &red, 1.0f);
        Broc::Code_DebugRenderEntityBBox(trigger, &red, 0.4f);
    }

    RenderFlagInfo(mp_util_wad::pLevel->allies_flag_ent, x, y);
    y += (int)y_inc;
    RenderFlagInfo(mp_util_wad::pLevel->axis_flag_ent, x, y);
    y += (int)y_inc;
    y += (int)y_inc;

    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint playerIndex(0);
    while ((int)playerIndex < Broc::size(players)) {
        Broc::entity player = players[(unsigned int)(int)playerIndex];
        temp = Broc::string(playerIndex);
        temp += " Name: ";
        temp += Broc::Code_GetPlayerName(player);
        temp += " State: ";

        Broc::bint playerState;
        Broc::entity::__unnamed::playerState_struct playerStateField = {
            player.GetHandle()};
        playerStateField.Get(&playerState);
        switch ((int)playerState) {
        case 0: temp += " JOINING "; break;
        case 1: temp += " SPECTATING "; break;
        case 2: temp += " INTERMISSION "; break;
        case 3: temp += " PLAYING "; break;
        case 4: temp += " CRITICAL "; break;
        case 5: temp += " DEAD "; break;
        default: break;
        }

        temp += " Flag: ";
        Broc::bbool hasHolder;
        mp_util_wad::IsEEDefined_holder(&hasHolder, player);
        if ((bool)hasHolder) {
            temp += "true";
            Broc::entity holder = *mp_util_wad::GetEE_holder(player);
            Broc::bbool holderDefined;
            mp_util_wad::IsEEDefined_holder(&holderDefined, holder);
            if (!(bool)holderDefined ||
                *mp_util_wad::GetEE_holder(holder) != player)
                temp += " Flag IS NOT CORRECTLY CONNECTED TO THE PLAYER";
        } else {
            temp += "false";
        }

        temp += " Icon: ";
        Broc::entity::__unnamed::ctf_has_flag_struct ctfHasFlagField = {
            player.GetHandle()};
        temp += ctfHasFlagField.Get() != 0 ? "true" : "false";
        Broc::Code_DebugRenderText(temp.c_str(), (int)x, (int)y);
        y += (int)y_inc;
        ++playerIndex;
    }
}

// RenderFlagInfo - ea: 0x955620
void RenderFlagInfo(Broc::entity flag, Broc::bint x, Broc::bint y) {
    if (!Broc::IsDefined(flag))
        return;

    Broc::string text((const char*)NULL);
    if (flag == mp_util_wad::pLevel->allies_flag_ent)
        text = "Allies Flag ";
    else
        text = "Axis Flag ";

    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, flag);
    if ((bool)hasHolder) {
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        Broc::string name(Broc::Code_GetPlayerName(holder));
        text += "is held by ";
        text += name;

        Broc::bbool holderHasHolder;
        mp_util_wad::IsEEDefined_holder(&holderHasHolder, holder);
        if (!(bool)holderHasHolder ||
            *mp_util_wad::GetEE_holder(holder) != flag)
            text += ". Who DOES NOT THINK HE IS HOLDING THE FLAG.";
        name.~string();
    } else {
        Broc::bint distance(0);
        Broc::bbool hasGoal;
        mp_util_wad::IsEEDefined_goal(&hasGoal, flag);
        if ((bool)hasGoal) {
            Broc::vector flagOrigin;
            Broc::vector goalOrigin;
            Broc::entity::__unnamed::origin_struct flagOriginField = {
                flag.GetHandle()};
            flagOriginField.Get(&flagOrigin);
            Broc::entity goal = *mp_util_wad::GetEE_goal(flag);
            Broc::entity::__unnamed::origin_struct goalOriginField = {
                goal.GetHandle()};
            goalOriginField.Get(&goalOrigin);
            distance = static_cast<int>(
                Broc::Distance(&goalOrigin, &flagOrigin));
        }

        if ((int)distance < 60) {
            text += "is at the base.";
        } else {
            text += "is dropped at position (";
            Broc::vector origin;
            Broc::entity::__unnamed::origin_struct originField = {
                flag.GetHandle()};
            originField.Get(&origin);
            {
                Broc::string value(origin[0]);
                text += value;
            }
            text += ",";
            {
                Broc::string value(origin[1]);
                text += value;
            }
            text += ",";
            {
                Broc::string value(origin[2]);
                text += value;
            }
            text += ")";
        }
    }

    Broc::Code_DebugRenderText(text.c_str(), (int)x, (int)y);
    text.~string();
}

// HandleDropFlag - ea: 0x9505F0
void HandleDropFlag(Broc::entity guy) {
    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, guy);
    if (!(bool)hasHolder)
        return;

    mp_util_wad::entity_set_ctf_has_flag(guy, 0);
    static Broc::bfloat dropSpeed(200.0f);
    static Broc::bfloat dropPitch(-30.0f);

    Broc::vector angles;
    Broc::entity::__unnamed::angles_struct anglesField = {
        guy.GetHandle()};
    anglesField.Get(&angles);
    Broc::vector launchAngles((float)dropPitch, angles.y, 0.0f);
    Broc::vector velocity = ::AnglesToForward(launchAngles) *
                            (float)dropSpeed;

    if (!Broc::Code_IsLocalPlayer(guy) &&
        Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_ctf.bro", __LINE__,
            "CallbackDropFlag:  Function was not called by the local guy"))
        __debugbreak();

    Broc::entity holderForTeam = *mp_util_wad::GetEE_holder(guy);
    Broc::entity::__unnamed::targetname_struct targetnameField = {
        holderForTeam.GetHandle()};
    Broc::string holderTargetName;
    targetnameField.Get(&holderTargetName);
    int netID = (holderTargetName == "axis" ||
                 holderTargetName == "ctf_axis")
                    ? 1
                    : 2;
    holderTargetName.~string();

    Broc::vector offset(0.0f, 0.0f, 20.0f);
    Broc::vector dropOrigin;
    Broc::entity::__unnamed::origin_struct originField = {
        guy.GetHandle()};
    originField.Get(&dropOrigin);
    dropOrigin = dropOrigin + offset;
    Broc::Code_DropItem(4, netID, &dropOrigin, &angles, &velocity);

    Broc::entity holder = *mp_util_wad::GetEE_holder(guy);
    Broc::string weapon = *mp_util_wad::GetEE_weaponstr(holder);
    Broc::TakeWeapon(&guy, &weapon);
    weapon.~string();
    if (!Broc::SwitchToLastWeapon(&guy))
        _mp_common::SelectFirstAvailableWeapon(guy);
    *mp_util_wad::GetEE_holder(guy) = Broc::gEntityUndef;
}

// HandlePickupFlag - ea: 0x950F30
void HandlePickupFlag(int netID, Broc::entity pickerupper,
                      Broc::bbool playSounds, Broc::bbool giveStats) {
    Broc::string flagTeam("allies");
    Broc::entity flag = mp_util_wad::pLevel->allies_flag_ent;
    Broc::bint flagObjective(1);
    Broc::string dialogMessage("MP_CTF_AlliesTaken");
    if (netID == 1) {
        flagTeam = "axis";
        flag = mp_util_wad::pLevel->axis_flag_ent;
        flagObjective = 0;
        dialogMessage = "MP_CTF_AxisTaken";
    }

    Broc::bbool atBase;
    IsFlagAtBase(&atBase, flag);

    Broc::string pickerTeam;
    bool opposingPicker = Broc::IsDefined(pickerupper);
    if (opposingPicker) {
        Broc::entity::__unnamed::team_struct teamField = {
            pickerupper.GetHandle()};
        teamField.Get(&pickerTeam);
        opposingPicker = pickerTeam != flagTeam;
        pickerTeam.~string();
    }

    if (opposingPicker) {
        Broc::bint playerState;
        Broc::entity::__unnamed::playerState_struct playerStateField = {
            pickerupper.GetHandle()};
        playerStateField.Get(&playerState);
        if ((int)playerState != 3) {
            dialogMessage.~string();
            flagTeam.~string();
            return;
        }

        Broc::string flagteam("axis");
        Broc::string otherteam("allies");
        Broc::bint objective(3);
        if (netID == 1) {
            dialogMessage = "MP_CTF_AxisTaken";
            flagteam = "allies";
            otherteam = "axis";
            objective = 2;
        }

        Broc::bint now;
        Broc::GetTime(&now);
        *mp_util_wad::GetEE_pickupCaptureDelayTime(pickerupper) =
            (int)now + 1000;

        Broc::bbool hasSound;
        mp_util_wad::IsEEDefined_sound_handle(&hasSound, pickerupper);
        if ((bool)hasSound) {
            Broc::EffectEventStopEmitting(
                (unsigned int)(int)*mp_util_wad::GetEE_sound_handle(
                    pickerupper));
            *mp_util_wad::GetEE_sound_handle(pickerupper) = 0;
        }

        Broc::string script("FLAG_FLAPPING");
        int soundHandle = Broc::EffectEventPlay(&pickerupper, &script);
        *mp_util_wad::GetEE_sound_handle(pickerupper) = soundHandle;
        script.~string();

        if ((bool)playSounds) {
            Broc::entity level = mp_util_wad::pLevel != nullptr
                                      ? mp_util_wad::pLevel->_base.entity
                                      : Broc::entity();
            void* soundFtor = _mp_audio::PlayTeamSound__functor(
                level, Broc::string(flagteam),
                Broc::string("MX_CTF_FlagTaken"),
                Broc::string("MX_CTF_FlagTakenEnemy"));
            Broc::thread_create(false,
                                "c:\\cod\\code\\script\\_mp_ctf.bro",
                                __LINE__, "_mp_audio::PlayTeamSound",
                                soundFtor);
            if ((bool)atBase) {
                if (flagteam == "allies") {
                    Broc::entity dialogLevel = mp_util_wad::pLevel != nullptr
                                                   ? mp_util_wad::pLevel->_base.entity
                                                   : Broc::entity();
                    void* dialogFtor = _mp_audio::PlayTeamDialog__functor(
                        dialogLevel, Broc::string("allies"),
                        Broc::string("MP_CTF_AlliedGotFlag_Allies"),
                        Broc::bfloat(0.5f));
                    Broc::thread_create(false,
                                        "c:\\cod\\code\\script\\_mp_ctf.bro",
                                        __LINE__, "_mp_audio::PlayTeamDialog",
                                        dialogFtor);

                    Broc::entity returnLevel = mp_util_wad::pLevel != nullptr
                                                   ? mp_util_wad::pLevel->_base.entity
                                                   : Broc::entity();
                    void* returnFtor = _mp_audio::PlayTeamDialog__functor(
                        returnLevel, Broc::string("axis"),
                        Broc::string("MP_CTF_ReturnFlag_Axis"),
                        Broc::bfloat(1.5f));
                    Broc::thread_create(false,
                                        "c:\\cod\\code\\script\\_mp_ctf.bro",
                                        __LINE__, "_mp_audio::PlayTeamDialog",
                                        returnFtor);
                    Broc::iprintln("MPCTF_AXIS_FLAG_TAKEN");
                } else {
                    Broc::entity dialogLevel = mp_util_wad::pLevel != nullptr
                                                   ? mp_util_wad::pLevel->_base.entity
                                                   : Broc::entity();
                    void* dialogFtor = _mp_audio::PlayTeamDialog__functor(
                        dialogLevel, Broc::string("axis"),
                        Broc::string("MP_CTF_AxisGotFlag_Axis"),
                        Broc::bfloat(0.5f));
                    Broc::thread_create(false,
                                        "c:\\cod\\code\\script\\_mp_ctf.bro",
                                        __LINE__, "_mp_audio::PlayTeamDialog",
                                        dialogFtor);

                    Broc::entity returnLevel = mp_util_wad::pLevel != nullptr
                                                   ? mp_util_wad::pLevel->_base.entity
                                                   : Broc::entity();
                    void* returnFtor = _mp_audio::PlayTeamDialog__functor(
                        returnLevel, Broc::string("allies"),
                        Broc::string("MP_CTF_ReturnFlag_Allies"),
                        Broc::bfloat(1.5f));
                    Broc::thread_create(false,
                                        "c:\\cod\\code\\script\\_mp_ctf.bro",
                                        __LINE__, "_mp_audio::PlayTeamDialog",
                                        returnFtor);
                    Broc::iprintln("MPCTF_ALLIES_FLAG_TAKEN");
                }
            }
        }

        Broc::entity::__unnamed::ctf_has_flag_struct ctfHasFlag = {
            pickerupper.GetHandle()};
        const __int16 hasFlag = 1;
        ctfHasFlag = hasFlag;
        Broc::GiveWeapon(&pickerupper, mp_util_wad::GetEE_weaponstr(flag));
        Broc::string slot("flag");
        Broc::SetWeaponSlotClipAmmo(&pickerupper, &slot, 1);
        slot.~string();
        Broc::SwitchToWeapon(pickerupper,
                             *mp_util_wad::GetEE_weaponstr(flag));
        Broc::SetOwner(&flag, &pickerupper);
        EntityOff(flag);
        *mp_util_wad::GetEE_holder(flag) = pickerupper;
        *mp_util_wad::GetEE_holder(pickerupper) = flag;

        Broc::entity level = mp_util_wad::pLevel != nullptr
                                  ? mp_util_wad::pLevel->_base.entity
                                  : Broc::entity();
        Broc::DialogPlay(level, &dialogMessage);
        ObjectiveRing((int)objective, -1);
        if ((bool)giveStats && (bool)atBase)
            _mp_common::AddToPlayerStats(pickerupper, Broc::bint(16), 1);

        Broc::dyn_array<Broc::entity> players;
        Broc::GetLocalPlayerArray(&players);
        players.~dyn_array();
        otherteam.~string();
        flagteam.~string();
    }

    HashStr pickupNotify(0x87404C8Eu);
    Broc::notify(flag, pickupNotify);
    if (Broc::Code_IsLocalPlayer(pickerupper)) {
        void* ftor = CompassUnderlay__functor(pickerupper);
        Broc::thread_create(false,
                            "c:\\cod\\code\\script\\_mp_ctf.bro",
                            __LINE__, "CompassUnderlay", ftor);
    }

    Broc::string objectiveState((const char*)NULL);
    objectiveState = netID == 1 ? "i_neutral_axis_c" : "i_neutral_allied_c";
    Broc::string pickerTeamForReturn;
    bool returnPath = !Broc::IsDefined(pickerupper);
    if (!returnPath) {
        Broc::entity::__unnamed::team_struct teamField = {
            pickerupper.GetHandle()};
        teamField.Get(&pickerTeamForReturn);
        returnPath = pickerTeamForReturn == flagTeam;
        pickerTeamForReturn.~string();
    }

    if (returnPath) {
        Broc::string returnedState((const char*)NULL);
        if (Broc::IsDefined(pickerupper)) {
            returnedState = netID == 1 ? "i_flag_axis_c" : "i_flag_allied_c";
            if ((bool)playSounds) {
                if (netID == 1) {
                    Broc::entity level = mp_util_wad::pLevel != nullptr
                                              ? mp_util_wad::pLevel->_base.entity
                                              : Broc::entity();
                    void* ftor = _mp_audio::PlayTeamDialog__functor(
                        level, Broc::string("axis"),
                        Broc::string("MP_CTF_AxisFlagReturned_Axis"),
                        Broc::bfloat(0.1f));
                    Broc::thread_create(false,
                                        "c:\\cod\\code\\script\\_mp_ctf.bro",
                                        __LINE__, "_mp_audio::PlayTeamDialog",
                                        ftor);
                    Broc::entity level2 = mp_util_wad::pLevel != nullptr
                                               ? mp_util_wad::pLevel->_base.entity
                                               : Broc::entity();
                    ftor = _mp_audio::PlayTeamDialog__functor(
                        level2, Broc::string("allies"),
                        Broc::string("MP_CTF_AxisFlagReturned_Allies"),
                        Broc::bfloat(0.1f));
                    Broc::thread_create(false,
                                        "c:\\cod\\code\\script\\_mp_ctf.bro",
                                        __LINE__, "_mp_audio::PlayTeamDialog",
                                        ftor);
                    Broc::iprintln("MPCTF_AXIS_FLAG_RETURNED");
                } else {
                    Broc::entity level = mp_util_wad::pLevel != nullptr
                                              ? mp_util_wad::pLevel->_base.entity
                                              : Broc::entity();
                    void* ftor = _mp_audio::PlayTeamDialog__functor(
                        level, Broc::string("allies"),
                        Broc::string("MP_CTF_AlliedFlagReturned_Allies"),
                        Broc::bfloat(0.1f));
                    Broc::thread_create(false,
                                        "c:\\cod\\code\\script\\_mp_ctf.bro",
                                        __LINE__, "_mp_audio::PlayTeamDialog",
                                        ftor);
                    Broc::entity level2 = mp_util_wad::pLevel != nullptr
                                               ? mp_util_wad::pLevel->_base.entity
                                               : Broc::entity();
                    ftor = _mp_audio::PlayTeamDialog__functor(
                        level2, Broc::string("axis"),
                        Broc::string("MP_CTF_AlliedFlagReturned_Axis"),
                        Broc::bfloat(0.1f));
                    Broc::thread_create(false,
                                        "c:\\cod\\code\\script\\_mp_ctf.bro",
                                        __LINE__, "_mp_audio::PlayTeamDialog",
                                        ftor);
                    Broc::iprintln("MPCTF_ALLIES_FLAG_RETURNED");
                }
            }
        }

        *mp_util_wad::GetEE_returnedSinceLastDeath(pickerupper) = 1;
        if ((bool)giveStats)
            _mp_common::AddToPlayerStats(pickerupper, Broc::bint(16), 1);

        *mp_util_wad::GetEE_holder(flag) = Broc::gEntityUndef;
        Broc::vector angles;
        Broc::vector origin;
        Broc::entity::__unnamed::angles_struct anglesField = {
            mp_util_wad::GetEE_goal(flag)->GetHandle()};
        anglesField.Get(&angles);
        Broc::entity::__unnamed::origin_struct originField = {
            mp_util_wad::GetEE_goal(flag)->GetHandle()};
        originField.Get(&origin);
        UpdateFlagAndTrigger(flag, origin, angles);
        returnedState.~string();
    }

    objectiveState.~string();
    dialogMessage.~string();
    flagTeam.~string();
}

// CallbackPickupScriptItem - ea: 0x952A60
void CallbackPickupScriptItem(int netID, Broc::entity guy, int itemIndex) {
    Broc::bbool playSounds(true);
    Broc::bbool giveStats(itemIndex == 0);
    Broc::entity flag = mp_util_wad::pLevel->allies_flag_ent;
    Broc::string flagTeam("allies");
    if (netID == 1) {
        flag = mp_util_wad::pLevel->axis_flag_ent;
        flagTeam = "axis";
    }

    Broc::entity currentHolder = *mp_util_wad::GetEE_holder(flag);
    if (currentHolder == Broc::gEntityUndef) {
        Broc::vector goalOrigin;
        Broc::vector flagOrigin;
        Broc::entity::__unnamed::origin_struct goalOriginField = {
            mp_util_wad::GetEE_goal(flag)->GetHandle()};
        goalOriginField.Get(&goalOrigin);
        Broc::entity::__unnamed::origin_struct flagOriginField = {
            flag.GetHandle()};
        flagOriginField.Get(&flagOrigin);
        if (Broc::Distance(&goalOrigin, &flagOrigin) < 1.0f) {
            Broc::vector pickerOrigin;
            Broc::entity::__unnamed::origin_struct pickerOriginField = {
                guy.GetHandle()};
            pickerOriginField.Get(&pickerOrigin);
            if (Broc::Distance(&flagOrigin, &pickerOrigin) <= 256.0f) {
                if (itemIndex != 0) {
                    if (!Broc::Code_IsHost()) {
                        Broc::string pickerTeam;
                        bool reject = !Broc::Code_IsLocalPlayer(guy);
                        if (!reject && Broc::IsDefined(guy)) {
                            Broc::entity::__unnamed::team_struct teamField = {
                                guy.GetHandle()};
                            teamField.Get(&pickerTeam);
                            reject = pickerTeam == flagTeam;
                            pickerTeam.~string();
                        }
                        if (reject) {
                            flagTeam.~string();
                            return;
                        }
                    } else {
                        Broc::Code_PickupItem(netID, guy);
                    }
                }
                HandlePickupFlag(netID, guy, playSounds, giveStats);
                flagTeam.~string();
                return;
            }
        }
    }

    if (*mp_util_wad::GetEE_holder(flag) != guy) {
        if (itemIndex != 0) {
            flagTeam.~string();
            return;
        }

        UnlinkFlag(flag);
        Broc::string pickerTeam;
        Broc::string holderTeam;
        Broc::entity::__unnamed::team_struct pickerTeamField = {
            guy.GetHandle()};
        pickerTeamField.Get(&pickerTeam);
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        Broc::entity::__unnamed::team_struct holderTeamField = {
            holder.GetHandle()};
        holderTeamField.Get(&holderTeam);
        if (holderTeam == pickerTeam)
            playSounds = false;
        holderTeam.~string();
        pickerTeam.~string();
    } else {
        playSounds = false;
    }

    HandlePickupFlag(netID, guy, playSounds, giveStats);
    flagTeam.~string();
}

// CallbackDropItem - ea: 0x952F30
void CallbackDropItem(int netID, int entity, Broc::vector position,
                      Broc::vector angles, Broc::vector velocity) {
    (void)entity;
    Broc::entity flag;
    Broc::bint flagObjective;
    Broc::string flagState((const char*)NULL);
    if (netID == 1) {
        Broc::string name("ctf_axis");
        HashStr key(0x19F9F0E8u);
        Broc::entity found;
        flag = *Broc::GetEnt(&found, &name, key, 0);
        flagObjective = 2;
        flagState = "i_flag_axis_c";
        name.~string();
    }
    if (netID == 2) {
        Broc::string name("ctf_allies");
        HashStr key;
        key.mVal = 0x19F9F0E8u;
        Broc::entity found;
        flag = *Broc::GetEnt(&found, &name, key, 0);
        flagObjective = 3;
        flagState = "i_flag_allied_c";
        name.~string();
    }

    if (Broc::Length(&velocity) >= 0.1f)
        LaunchFlagAndTrigger(flag, position, angles, velocity);
    else
        UpdateFlagAndTrigger(flag, position, angles);

    HashStr dropNotify(0xFA57E7C7u);
    Broc::notify(flag, dropNotify);

    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, flag);
    if ((bool)hasHolder) {
        Broc::bint now;
        Broc::GetTime(&now);
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        *mp_util_wad::GetEE_last_dropped_time(holder) = (int)now;
        Broc::entity::__unnamed::ctf_has_flag_struct ctfHasFlag = {
            holder.GetHandle()};
        const __int16 noFlag = 0;
        ctfHasFlag = noFlag;
        *mp_util_wad::GetEE_holder(holder) = Broc::gEntityUndef;
        *mp_util_wad::GetEE_holder(flag) = Broc::gEntityUndef;

        void* ftor = WaitForFlagTimeOut__functor(flag);
        Broc::thread_create(false,
                            "c:\\cod\\code\\script\\_mp_ctf.bro",
                            __LINE__, "WaitForFlagTimeOut", ftor);
        Broc::string sound("MX_CTF_FlagDropped");
        Broc::SoundPlay(&sound, 1.0f);
        sound.~string();
        if (netID == 1)
            Broc::iprintln("MPCTF_AXIS_FLAG_DROPPED");
        else
            Broc::iprintln("MPCTF_ALLIES_FLAG_DROPPED");
    }
    flagState.~string();
}
}

// ============================================================================
// _mp_scf - single capture the flag.
// ============================================================================
namespace _mp_scf {

static Broc::bfloat lSCFObjectiveDontShow(-1.0f);

// main - ea: 0x9649A0
void main(Broc::entity self) {
    Broc::Code_DebugOut("*SCF* main\n");
    Broc::bbool team_game(true);
    Broc::Code_SetTeamGame((bool)team_game);
    mp_util_wad::pLevel->spawnTypeAllies = "spawn_single_ctf_allies";
    mp_util_wad::pLevel->spawnTypeAxis = "spawn_single_ctf_axis";
    mp_util_wad::pLevel->mustHaveBothTeamsToStart = false;
    mp_util_wad::pLevel->showFlagHint = 0;
    mp_util_wad::pLevel->lastManStanding = false;
    mp_util_wad::pLevel->roundLimit = 1;
    mp_util_wad::pLevel->current_flag = 0;
    _mp_common::SetupCallbacks(team_game);
    Broc::BrocExports& x = Broc::gBrocAPI.mBrocExports;
    x.mCallbackPlayerJoin = CallbackPlayerJoin;
    x.mCallbackPlayerEnter = CallbackPlayerEnter;
    x.mCallbackPlayerSpawn = CallbackPlayerSpawn;
    x.mCallbackGameState = CallbackGameState;
    x.mCallbackGameStateSCF = CallbackGameStateSCF;
    x.mCallbackDropItem = CallbackDropItem;
    x.mCallbackDropFlag = CallbackDropFlag;
    x.mCallbackPickupScriptItem = CallbackPickupScriptItem;
    x.mCallbackPlayerKilled = CallbackPlayerKilled;
    x.mCallbackPlayerLeave = CallbackPlayerLeave;
    x.mCallbackNextRound = CallbackNextRound;
    x.mCallbackHostOptionsChanged = CallbackHostOptionsChanged;
    x.mCallbackAreaCaptured = CallbackAreaCaptured;
    x.mCallbackHostMigrated = (void (*)())CallbackHostMigrated;
    x.mCallbackShowFlagHint = CallbackShowFlagHint;
    x.mCallbackDebugRender = (void (*)())CallbackDebugRender;
    Broc::dyn_array<Broc::entity> tempFlags;
    Broc::string val("scf_flag_point");
    HashStr key;
    key.mVal = 0x19F9F0E8u;
    Broc::GetEntArray(&val, key.mVal, &tempFlags, 0);
    val.~string();
    Broc::bint i(0);
    while ((int)i < Broc::size(tempFlags)) {
        Broc::entity f = tempFlags[(unsigned int)(int)i];
        mp_util_wad::pLevel->scfFlags[(unsigned int)(int)i] = f;
        i = (int)i + 1;
    }
    if (Broc::size(mp_util_wad::pLevel->scfFlags) == 0 &&
        Broc::gBrocAPI.mError(
            "c:\\cod\\code\\script\\_mp_scf.bro", __LINE__,
            "No scf_flag_point entities in the level."))
        __debugbreak();
    Broc::string baseAllies("scf_base_allies");
    HashStr key2;
    key2.mVal = 0x19F9F0E8u;
    Broc::entity e;
    mp_util_wad::pLevel->base_allies = *Broc::GetEnt(&e, &baseAllies, key2, 0);
    baseAllies.~string();
    Broc::string baseAxis("scf_base_axis");
    HashStr key3;
    key3.mVal = 0x19F9F0E8u;
    Broc::entity e2;
    mp_util_wad::pLevel->base_axis = *Broc::GetEnt(&e2, &baseAxis, key3, 0);
    baseAxis.~string();
    if (!Broc::IsDefined(mp_util_wad::pLevel->base_allies) &&
        Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_scf.bro", __LINE__,
            "No scf_base_allies entity in the level."))
        __debugbreak();
    if (!Broc::IsDefined(mp_util_wad::pLevel->base_axis) &&
        Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_scf.bro", __LINE__,
            "No scf_base_axis entity in the level."))
        __debugbreak();
    InitializeFlags();
    Broc::wait(1.0f);
    void* started = StartGame__functor(self);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_scf.bro",
                        __LINE__, "StartGame", started);
    tempFlags.~dyn_array();
}

// StartGame - ea: 0x965AA0
void StartGame(Broc::entity self) {
    (void)self;
    Broc::Code_DebugOut("*SCF* StartGame\n");
    Broc::wait(0.5f);
    SetupRound();
    _mp_common::StartRound(Broc::bbool(true));
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = _mp_common::RunFrame__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_scf.bro",
                        __LINE__, "_mp_common::RunFrame", ftor);
    Broc::Code_EnterGame();
}

// SetupRound - ea: 0x965B60
void SetupRound() {
    Broc::Code_DebugOut("*SCF* SetupRound\n");
    ResetFlags();
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        *mp_util_wad::GetEE_holder(p) = Broc::gEntityUndef;
        mp_util_wad::entity_set_ctf_has_flag(p, 0);
        *mp_util_wad::GetEE_last_dropped_time(p) = -1;
        i = (int)i + 1;
    }
    players.~dyn_array();
    HashStr goalHash;
    Broc::string_hash(&goalHash, "_mp_scf::Goal");
    HashStr label;
    label.mVal = 0xF2F5EAB4;
    Broc::RemoveEventHandler(&mp_util_wad::pLevel->base_allies, label,
                             goalHash);
    Broc::RemoveEventHandler(&mp_util_wad::pLevel->base_axis, label,
                             goalHash);
    Broc::AddEventHandler(&mp_util_wad::pLevel->base_allies, label.mVal,
                          goalHash.mVal);
    Broc::AddEventHandler(&mp_util_wad::pLevel->base_axis, label.mVal,
                          goalHash.mVal);
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = WaitThenPickFlagToLaunch__functor(lvl, 5.0f,
                                                   "MPSCF_FLAG_SPAWNED");
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_scf.bro",
                        __LINE__, "WaitThenPickFlagToLaunch", ftor);
}

// IsFlagAlive - ea: 0x965F50
int IsFlagAlive(Broc::entity flag) {
    Broc::vector origin;
    mp_util_wad::entity_get_origin(&origin, flag);
    return origin.z > -5000.0f;
}

// CallbackNextRound - ea: 0x968070
void CallbackNextRound() {
    Broc::Code_DebugOut("*SCF* CallbackNextRound\n");
    mp_util_wad::pLevel->roundOver = true;
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr n;
    n.mVal = 0x6FA23667u;
    Broc::notify(lvl, n);
    SetupRound();
    _mp_common::CallbackNextRound();
}

// CallbackHostOptionsChanged - ea: 0x968110
void CallbackHostOptionsChanged(int forceMapChange) {
    Broc::Code_DebugOut("*SCF* CallbackHostOptionsChanged\n");
    _mp_common::CallbackHostOptionsChanged(forceMapChange);
    mp_util_wad::pLevel->lastManStanding = false;
}

// CallbackGameState - ea: 0x968160
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int roundOver,
                       int roundCount) {
    Broc::Code_DebugOut("*SCF* CallbackGameState\n");
    _mp_common::CallbackGameState(currentMatchTime, timeLimit, scoreLimit,
                                  roundLimit, friendlyFire, 0, teamBalance,
                                  respawnTime, alliesScore, axisScore,
                                  roundStarted, roundOver, roundCount);
    (void)lastManStanding;
}

// CallbackPlayerKilled - ea: 0x9681D0
void CallbackPlayerKilled(Broc::entity player, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health) {
    Broc::Code_DebugOut("*SCF* CallbackPlayerKilled\n");
    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, player);
    if ((bool)hasHolder)
        HandleDropFlag(player);
    _mp_common::CallbackPlayerKilled(player, inflictor, attacker, weapon, mod,
                                     health);
}

// CallbackPlayerLeave - ea: 0x968290
void CallbackPlayerLeave(Broc::entity player) {
    Broc::Code_DebugOut("*SCF* CallbackPlayerLeave\n");
    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, player);
    if ((bool)hasHolder)
        HandleDropFlag(player);
    _mp_common::CallbackPlayerLeave(player);
}

// CallbackGameStateSCF - ea: 0x968320
void CallbackGameStateSCF(int currentFlagIndex, Broc::vector FlagOrigin,
                          Broc::vector FlagAngles, Broc::entity FlagHolder) {
    Broc::Code_DebugOut("*SCF* CallbackGameStateSCF\n");
    ResetFlags();
    if (Broc::IsDefined(FlagHolder)) {
        if (currentFlagIndex < 0 ||
            currentFlagIndex >= Broc::size(mp_util_wad::pLevel->scfFlags)) {
            if (Broc::gBrocAPI.mAssert(
                    "c:\\cod\\code\\script\\_mp_scf.bro", __LINE__,
                    "CallbackGameStateSCF:Invalid flag index"))
                __debugbreak();
        }
        Broc::entity flag =
            mp_util_wad::pLevel->scfFlags[(unsigned int)(int)mp_util_wad::pLevel->current_flag];
        HandlePickupFlag(flag, FlagHolder, 1, 0);
        mp_util_wad::pLevel->current_flag = currentFlagIndex;
    } else {
        Broc::vector velocity(0.0f, 0.0f, 50.0f);
        LaunchFlag(Broc::bint(currentFlagIndex), FlagOrigin, FlagAngles,
                   velocity);
    }
}

// CallbackShowFlagHint - ea: 0x9684B0
int CallbackShowFlagHint() {
    return (int)mp_util_wad::pLevel->showFlagHint;
}

// UpdateFlagAndTrigger - ea: 0x9684E0
void UpdateFlagAndTrigger(Broc::entity self, Broc::vector origin,
                          Broc::vector angles, Broc::vector velocity) {
    (void)velocity;
    Broc::Code_DebugOut("*SCF* UpdateFlagAndTrigger\n");
    mp_util_wad::entity_set_origin(self, origin);
    mp_util_wad::entity_set_angles(self, angles);
}

// LaunchFlagAndTrigger - ea: 0x968530
void LaunchFlagAndTrigger(Broc::entity self, Broc::vector origin,
                          Broc::vector angles, Broc::vector velocity) {
    Broc::Code_DebugOut("*SCF* LaunchFlagAndTrigger\n");
    mp_util_wad::entity_set_origin(self, origin);
    mp_util_wad::entity_set_angles(self, angles);
    Broc::Launch(&self, &velocity);
}

// EntityOff - ea: 0x968590
void EntityOff(Broc::entity self) {
    Broc::vector origin;
    mp_util_wad::entity_get_origin(&origin, self);
    if (origin.z > -5000.0f) {
        Broc::vector off(0.0f, 0.0f, -10000.0f);
        mp_util_wad::entity_set_origin(self, origin + off);
    }
}

// IsFlagAtBase - ea: 0x968870
Broc::bbool* IsFlagAtBase(Broc::bbool* result, Broc::entity flag) {
    Broc::vector home = *mp_util_wad::GetEE_home_position(flag);
    Broc::vector f;
    mp_util_wad::entity_get_origin(&f, flag);
    Broc::vector baseOfs = f - home;
    baseOfs.z = 0.0f;
    *result = Broc::bbool(Broc::Length(&baseOfs) < 0.1f);
    return result;
}

// SubRoundEnd - ea: 0x967A20
void SubRoundEnd() {
    Broc::Code_DebugOut("*SCF* RoundEndMessage\n");
    mp_util_wad::pLevel->roundStarted = false;
    Broc::wait(4.0f);
    Broc::Code_NextRound(false);
}

// CallbackPlayerSpawn - ea: 0x967A80
void CallbackPlayerSpawn(Broc::entity player, int team_changed) {
    Broc::Code_DebugOut("*SCF* CallbackPlayerSpawn\n");
    _mp_common::CallbackPlayerSpawn(player, team_changed);
    *mp_util_wad::GetEE_holder(player) = Broc::gEntityUndef;
    __int16 noFlag = 0;
    Broc::entity::__unnamed::ctf_has_flag_struct ctfHasFlagField = {
        player.GetHandle()};
    ctfHasFlagField = noFlag;
    if (Broc::Code_IsLocalPlayer(player)) {
        Broc::entity flag =
            mp_util_wad::pLevel->scfFlags[(unsigned int)(int)mp_util_wad::pLevel->current_flag];
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        if (Broc::IsDefined(holder)) {
            Broc::string hteam;
            Broc::string pteam;
            mp_util_wad::entity_get_team(&hteam, holder);
            mp_util_wad::entity_get_team(&pteam, player);
            if (hteam == pteam)
                mp_util_wad::pLevel->showFlagHint = 1;
            hteam.~string();
            pteam.~string();
        }
    }
}

// CallbackHostMigrated - ea: 0x967DC0
void CallbackHostMigrated() {
    Broc::Code_DebugOut("*SCF* CallbackHostMigrated\n");
    _mp_common::CallbackHostMigrated();
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity flag =
            mp_util_wad::pLevel->scfFlags[(unsigned int)(int)mp_util_wad::pLevel->current_flag];
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        Broc::vector origin;
        Broc::vector angles;
        mp_util_wad::entity_get_origin(&origin, flag);
        mp_util_wad::entity_get_angles(&angles, flag);
        Broc::Code_SendGameStateSCF(
            players[(unsigned int)(int)i],
            (int)mp_util_wad::pLevel->current_flag, &origin, &angles, holder);
        i = (int)i + 1;
    }
    players.~dyn_array();
}

// CallbackPlayerJoin - ea: 0x96B5A0
void CallbackPlayerJoin(Broc::entity player, unsigned int playerState,
                        int playerClass) {
    _mp_common::CallbackPlayerJoin(player, playerState, (__int16)playerClass);
    mp_util_wad::entity_set_ctf_has_flag(player, 0);
    *mp_util_wad::GetEE_holder(player) = Broc::gEntityUndef;
    if (Broc::Code_IsLocalPlayer(player)) {
        void* ftor = ObjectiveUpdater__functor(player);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_scf.bro",
                            __LINE__, "ObjectiveUpdater", ftor);
    }
}

// CallbackPlayerEnter - ea: 0x96B720
void CallbackPlayerEnter(Broc::entity player, int hot_joiner) {
    _mp_common::CallbackPlayerEnter(player, hot_joiner);
    Broc::entity flag =
        mp_util_wad::pLevel->scfFlags[(unsigned int)(int)mp_util_wad::pLevel->current_flag];
    Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
    Broc::vector origin;
    Broc::vector angles;
    mp_util_wad::entity_get_origin(&origin, flag);
    mp_util_wad::entity_get_angles(&angles, flag);
    Broc::Code_SendGameStateSCF(player, (int)mp_util_wad::pLevel->current_flag,
                                &origin, &angles, holder);
}

// CompassUnderlay - ea: 0x968610
void CompassUnderlay(Broc::entity p) {
    Broc::bint flagid(0);
    Broc::bint alliesid(0);
    Broc::bint axisid(2);
    if (mp_util_wad::pLevel->axis == "german")
        axisid = 3;
    else if (mp_util_wad::pLevel->axis == "italian")
        axisid = 2;
    else if (mp_util_wad::pLevel->axis == "vichy")
        axisid = 4;
    if (mp_util_wad::pLevel->allies == "american")
        alliesid = 0;
    else
        alliesid = 1;
    Broc::string team;
    mp_util_wad::entity_get_team(&team, p);
    if (team == "axis")
        flagid = (int)alliesid;
    else
        flagid = (int)axisid;
    team.~string();
    for (;;) {
        Broc::bbool hasHolder;
        mp_util_wad::IsEEDefined_holder(&hasHolder, p);
        if (!(bool)hasHolder || (bool)mp_util_wad::pLevel->roundOver)
            break;
        Broc::Code_SetCompassVisibilty((int)flagid, true);
        Broc::wait(0.05f);
    }
    Broc::Code_SetCompassVisibilty((int)flagid, false);
    mp_util_wad::entity_set_ctf_has_flag(p, 0);
}

// WaitThenPickFlagToLaunch - ea: 0x96AC60
void WaitThenPickFlagToLaunch(Broc::entity self, Broc::bfloat wait_time,
                              const char* message) {
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr n;
    n.mVal = 0xEEBE2988;
    Broc::notify(lvl, n);
    Broc::wait(0.1f);
    Broc::endon(lvl, n);
    Broc::wait((float)wait_time);
    Broc::iprintln(message);
    PickFlagToLaunch();
    (void)self;
}

// PickFlagToLaunch - ea: 0x96AD50
void PickFlagToLaunch() {
    Broc::bint index(Broc::RandomIntRange(0, Broc::size(mp_util_wad::pLevel->scfFlags)));
    Broc::vector vel(0.0f, 0.0f, 0.0f);
    Broc::entity flag =
        mp_util_wad::pLevel->scfFlags[(unsigned int)(int)index];
    Broc::vector home = *mp_util_wad::GetEE_home_position(flag);
    Broc::vector homeAngles = *mp_util_wad::GetEE_home_angles(flag);
    Broc::Code_HostDropItem(4, (int)index, &home, &homeAngles, &vel);
}

// LaunchFlag - ea: 0x96AEB0
void LaunchFlag(Broc::bint flag, Broc::vector position, Broc::vector angles,
                Broc::vector velocity) {
    ResetFlags();
    mp_util_wad::pLevel->current_flag = (int)flag;
    ObjectiveRing(2, -1);
    Broc::entity f = mp_util_wad::pLevel->scfFlags[(unsigned int)(int)flag];
    if (Broc::Length(&velocity) >= 0.1f)
        LaunchFlagAndTrigger(f, position, angles, velocity);
    else
        UpdateFlagAndTrigger(f, position, angles, velocity);
}

// InitializeFlags - ea: 0x96B020
void InitializeFlags() {
    Broc::Code_DebugOut("*SCF* InitializeFlags\n");
    Broc::vector mins(-20.0f, -20.0f, 0.0f);
    Broc::vector maxs(20.0f, 20.0f, 50.0f);
    Broc::bint i(0);
    while ((int)i < Broc::size(mp_util_wad::pLevel->scfFlags)) {
        Broc::entity flag = mp_util_wad::pLevel->scfFlags[(unsigned int)(int)i];
        Broc::vector origin;
        Broc::vector angles;
        mp_util_wad::entity_get_origin(&origin, flag);
        mp_util_wad::entity_get_angles(&angles, flag);
        *mp_util_wad::GetEE_home_position(flag) = origin;
        *mp_util_wad::GetEE_home_angles(flag) = angles;
        *mp_util_wad::GetEE_holder(flag) = Broc::gEntityUndef;
        Broc::string inClassname("trigger_multiple");
        Broc::entity trig;
        Broc::Spawn(&trig, &inClassname, &origin, &mins, &maxs, 2,
                    static_cast<TPakInfo>(0));
        inClassname.~string();
        *mp_util_wad::GetEE_trigger(flag) = trig;
        Broc::LinkTo(mp_util_wad::GetEE_trigger(flag), &flag);
        Broc::string script("FLAG_FLAPPING");
        Broc::EffectEventPlay(&flag, &script);
        script.~string();
        HashStr pickHash;
        Broc::string_hash(&pickHash, "_mp_scf::PickupFlag");
        HashStr label;
        label.mVal = 0xF2F5EAB4;
        Broc::AddEventHandler(mp_util_wad::GetEE_trigger(flag), label.mVal,
                              pickHash.mVal);
        i = (int)i + 1;
    }
    ResetFlags();
}

// ResetFlags - ea: 0x96B3C0
int ResetFlags() {
    Broc::Code_DebugOut("*SCF* ResetFlags\n");
    Broc::bint i(0);
    int result = false;
    while ((int)i < Broc::size(mp_util_wad::pLevel->scfFlags)) {
        Broc::entity flag = mp_util_wad::pLevel->scfFlags[(unsigned int)(int)i];
        Broc::bbool hasHolder;
        mp_util_wad::IsEEDefined_holder(&hasHolder, flag);
        if ((bool)hasHolder) {
            Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
            Broc::bbool hasSound;
            mp_util_wad::IsEEDefined_sound_handle(&hasSound, holder);
            if ((bool)hasSound) {
                Broc::EffectEventStopEmitting(
                    (int)*mp_util_wad::GetEE_sound_handle(holder));
                *mp_util_wad::GetEE_sound_handle(holder) = 0;
            }
        }
        UnlinkFlag(flag);
        EntityOff(flag);
        i = (int)i + 1;
    }
    return result;
}

// UnlinkFlag - ea: 0x96B850
void UnlinkFlag(Broc::entity flag) {
    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, flag);
    if ((bool)hasHolder) {
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        Broc::bbool hasSound;
        mp_util_wad::IsEEDefined_sound_handle(&hasSound, holder);
        if ((bool)hasSound) {
            Broc::EffectEventStopEmitting(
                (int)*mp_util_wad::GetEE_sound_handle(holder));
            *mp_util_wad::GetEE_sound_handle(holder) = 0;
        }
        Broc::string w1("mp_flag_axis");
        Broc::TakeWeapon(&holder, &w1);
        w1.~string();
        Broc::string w2("mp_flag_allies");
        Broc::TakeWeapon(&holder, &w2);
        w2.~string();
        _mp_common::SelectFirstAvailableWeapon(holder);
        mp_util_wad::entity_set_ctf_has_flag(holder, 0);
        *mp_util_wad::GetEE_holder(holder) = Broc::gEntityUndef;
    }
    *mp_util_wad::GetEE_holder(flag) = Broc::gEntityUndef;
}

// DestroyIcon - ea: 0x96BB30
void DestroyIcon(Broc::entity toucher) {
    HashStr n;
    n.mVal = 0xA738D71C;
    Broc::notify(toucher, n);
    mp_util_wad::pLevel->showFlagHint = 0;
}

// WaitForNoTouchFlag - ea: 0x96BB80
void WaitForNoTouchFlag(Broc::entity toucher) {
    HashStr n;
    n.mVal = 0xA738D71C;
    Broc::notify(toucher, n);
    Broc::wait(0.5f);
    Broc::endon(toucher, n);
    for (;;) {
        Broc::bint now;
        Broc::GetTime(&now);
        if ((int)*mp_util_wad::GetEE_last_touch_time(toucher) + 300 <
            (int)now)
            break;
        Broc::wait(0.5f);
    }
    Broc::SetTutorialText(-1, Broc::GetPlayerIndex(toucher));
    DestroyIcon(toucher);
}

// WaitForFlagTimeOut - ea: 0x96A270
void WaitForFlagTimeOut(Broc::entity flag) {
    HashStr n;
    n.mVal = 0x4F2B87DFu;
    Broc::notify(flag, n);
    Broc::wait(0.1f);
    Broc::endon(flag, n);
    HashStr e1;
    e1.mVal = 0xFA57E7C7;
    Broc::endon(flag, e1);
    HashStr e2;
    e2.mVal = 0x87404C8E;
    Broc::endon(flag, e2);
    Broc::wait(25.0f);
    Broc::string tn;
    mp_util_wad::entity_get_targetname(&tn, flag);
    bool axis = tn == "axis" || tn == "ctf_axis";
    tn.~string();
    if (axis)
        Broc::Code_PickupItem(1, Broc::gEntityUndef);
    else
        Broc::Code_PickupItem(2, Broc::gEntityUndef);
}

// PickupFlag - ea: 0x96A390
void PickupFlag(Broc::entity self, Broc::entity triggerer) {
    (void)triggerer;
    Broc::bint now;
    Broc::GetTime(&now);
    if ((int)*mp_util_wad::GetEE_last_touch_time(self) + 100 > (int)now ||
        (int)*mp_util_wad::GetEE_last_touch_time(self) > (int)now) {
        Broc::bint t;
        Broc::GetTime(&t);
        *mp_util_wad::GetEE_last_touch_time(self) = (int)t;
        if ((bool)mp_util_wad::pLevel->roundStarted) {
            Broc::dyn_array<Broc::entity> players;
            Broc::GetPlayerArray(&players);
            Broc::bint i(0);
            while ((int)i < Broc::size(players)) {
                Broc::entity p = players[(unsigned int)(int)i];
                Broc::bint state;
                mp_util_wad::entity_get_playerState(&state, p);
                if ((int)state == 3 && Broc::Code_IsLocalPlayer(p) &&
                    !Broc::Code_IsInVehicle(p)) {
                    Broc::vector selfPos;
                    Broc::vector ppos;
                    mp_util_wad::entity_get_origin(&selfPos, self);
                    mp_util_wad::entity_get_origin(&ppos, p);
                    if (Broc::Distance(&ppos, &selfPos) < 60.0f) {
                        if (Broc::UseButtonPressed(p) != 0) {
                            Broc::Code_PickupItem(
                                (int)mp_util_wad::pLevel->current_flag + 1, p);
                            break;
                        }
                        Broc::bint t2;
                        Broc::GetTime(&t2);
                        *mp_util_wad::GetEE_last_touch_time(p) = (int)t2;
                        mp_util_wad::pLevel->showFlagHint = 1;
                        void* ftor = WaitForNoTouchFlag__functor(p);
                        Broc::thread_create(false,
                                            "c:\\cod\\code\\script\\_mp_scf.bro",
                                            __LINE__, "WaitForNoTouchFlag",
                                            ftor);
                    }
                }
                i = (int)i + 1;
            }
            players.~dyn_array();
        }
    }
}

void* main__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(main, self);
}
AeThreadFunctor1<Broc::entity>* StartGame__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(StartGame, self);
}
void* ObjectiveUpdater__functor(Broc::entity guy) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(ObjectiveUpdater, guy);
}
AeThreadFunctor3<Broc::entity, Broc::bfloat, const char*>*
WaitThenPickFlagToLaunch__functor(Broc::entity self,
                                  Broc::bfloat wait_time,
                                  const char* message) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor3<Broc::entity, Broc::bfloat, const char*>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor3<Broc::entity, Broc::bfloat, const char*>(WaitThenPickFlagToLaunch, self, wait_time, message);
}
void* WaitForFlagTimeOut__functor(Broc::entity flag) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WaitForFlagTimeOut, flag);
}
void* WaitForNoTouchFlag__functor(Broc::entity toucher) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WaitForNoTouchFlag, toucher);
}
void* PickupFlag__functor(Broc::entity self, Broc::entity triggerer) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::entity>(PickupFlag, self, triggerer);
}
void* Goal__functor(Broc::entity self, Broc::entity triggerer) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::entity>(Goal, self, triggerer);
}

// CallbackAreaCaptured - ea: 0x9670F0
void CallbackAreaCaptured(int index, int team) {
    (void)index;
    Broc::bbool playSounds(true);
    Broc::string myteam("axis");
    Broc::string otherteam("allies");
    if (team == 1) {
        myteam = "allies";
        otherteam = "axis";
    }

    Broc::Code_DebugOut("*SCF* CallbackAreaCaptured\n");
    Broc::entity flag = mp_util_wad::pLevel->scfFlags[
        (unsigned int)(int)mp_util_wad::pLevel->current_flag];
    if (!Broc::IsDefined(flag) &&
        Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_scf.bro", __LINE__ + 16,
            "flag holder broke"))
        __debugbreak();

    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, flag);
    if ((bool)hasHolder) {
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        _mp_common::AddToPlayerStats(holder, Broc::bint(19), 1);
    }

    if (team != 0) {
        myteam = "axis";
        Broc::string scoreTeam("axis");
        Broc::Code_IncTeamScore(scoreTeam, 1);
        scoreTeam.~string();

        if ((bool)playSounds) {
            Broc::entity level = mp_util_wad::pLevel != nullptr
                                      ? mp_util_wad::pLevel->_base.entity
                                      : Broc::entity();
            void* ftor = _mp_audio::PlayTeamSound__functor(
                level, myteam, Broc::string("MX_CTF_EnemyTeamScore"),
                Broc::string("MX_CTF_MyTeamScore"));
            Broc::thread_create(false,
                                "c:\\cod\\code\\script\\_mp_scf.bro",
                                __LINE__ + 44, "_mp_audio::PlayTeamSound",
                                ftor);

            Broc::entity level2 = mp_util_wad::pLevel != nullptr
                                       ? mp_util_wad::pLevel->_base.entity
                                       : Broc::entity();
            void* dialogFtor = _mp_audio::PlayTeamDialog__functor(
                level2, myteam,
                Broc::string("MP_SFCTF_EnemyCaptured_Allies"),
                Broc::string("MP_SFCTF_FriendlyCaptured_Axis"),
                Broc::bfloat(0.5f));
            Broc::thread_create(false,
                                "c:\\cod\\code\\script\\_mp_scf.bro",
                                __LINE__ + 45, "_mp_audio::PlayTeamDialog",
                                dialogFtor);
        }
        Broc::iprintln("MPSCF_AXIS_CAPTURED_FLAG");
    } else {
        myteam = "allies";
        Broc::string scoreTeam("allies");
        Broc::Code_IncTeamScore(scoreTeam, 1);
        scoreTeam.~string();

        if ((bool)playSounds) {
            Broc::entity level = mp_util_wad::pLevel != nullptr
                                      ? mp_util_wad::pLevel->_base.entity
                                      : Broc::entity();
            void* ftor = _mp_audio::PlayTeamSound__functor(
                level, myteam, Broc::string("MX_CTF_EnemyTeamScore"),
                Broc::string("MX_CTF_MyTeamScore"));
            Broc::thread_create(false,
                                "c:\\cod\\code\\script\\_mp_scf.bro",
                                __LINE__ + 31, "_mp_audio::PlayTeamSound",
                                ftor);

            Broc::entity level2 = mp_util_wad::pLevel != nullptr
                                       ? mp_util_wad::pLevel->_base.entity
                                       : Broc::entity();
            void* dialogFtor = _mp_audio::PlayTeamDialog__functor(
                level2, myteam,
                Broc::string("MP_SFCTF_EnemyCaptured_Axis"),
                Broc::string("MP_SFCTF_FriendlyCaptured_Allies"),
                Broc::bfloat(0.5f));
            Broc::thread_create(false,
                                "c:\\cod\\code\\script\\_mp_scf.bro",
                                __LINE__ + 32, "_mp_audio::PlayTeamDialog",
                                dialogFtor);
        }
        Broc::iprintln("MPSCF_ALLIES_CAPTURED_FLAG");
    }

    ResetFlags();
    Broc::entity level = mp_util_wad::pLevel != nullptr
                              ? mp_util_wad::pLevel->_base.entity
                              : Broc::entity();
    Broc::bint waitSeconds(30);
    Broc::bfloat waitTime(waitSeconds);
    void* ftor = WaitThenPickFlagToLaunch__functor(
        level, waitTime, "MPSCF_FLAG_SPAWNED");
    Broc::thread_create(false,
                        "c:\\cod\\code\\script\\_mp_scf.bro",
                        __LINE__ + 57, "WaitThenPickFlagToLaunch", ftor);
}

// CallbackDropFlag - ea: 0x969EE0
void CallbackDropFlag(Broc::entity player) {
    HandleDropFlag(player);
}

// HandleDropFlag - ea: 0x969F10
void HandleDropFlag(Broc::entity player) {
    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, player);
    if (!(bool)hasHolder)
        return;

    mp_util_wad::entity_set_ctf_has_flag(player, 0);
    Broc::bint now;
    Broc::GetTime(&now);
    *mp_util_wad::GetEE_last_dropped_time(player) = (int)now;

    static Broc::bfloat speed(200.0f);
    static Broc::bfloat pitch_0(-30.0f);
    Broc::vector playerAngles;
    Broc::entity::__unnamed::angles_struct anglesField = {
        player.GetHandle()};
    anglesField.Get(&playerAngles);
    Broc::vector launchAngles((float)pitch_0, playerAngles.y, 0.0f);
    Broc::vector velocity = ::AnglesToForward(launchAngles) * (float)speed;

    Broc::vector playerOrigin;
    Broc::entity::__unnamed::origin_struct originField = {
        player.GetHandle()};
    originField.Get(&playerOrigin);
    Broc::vector dropOffset(0.0f, 0.0f, 20.0f);
    Broc::vector dropOrigin = playerOrigin + dropOffset;
    Broc::Code_DropItem(
        4, (int)mp_util_wad::pLevel->current_flag, &dropOrigin,
        &playerAngles, &velocity);

    Broc::string axisWeapon("mp_flag_axis");
    Broc::TakeWeapon(&player, &axisWeapon);
    axisWeapon.~string();
    Broc::string alliesWeapon("mp_flag_allies");
    Broc::TakeWeapon(&player, &alliesWeapon);
    alliesWeapon.~string();

    if (!Broc::SwitchToLastWeapon(&player))
        _mp_common::SelectFirstAvailableWeapon(player);
    *mp_util_wad::GetEE_holder(player) = Broc::gEntityUndef;
}

// HandlePickupFlag - ea: 0x968920
// ea: 0x00968920
void HandlePickupFlag(Broc::entity flag, Broc::entity pickerupper, int request,
                      int playSounds) {
    Broc::string myteam(defaultFileName);
    Broc::bbool atBase;
    IsFlagAtBase(&atBase, flag);

    if (Broc::IsDefined(pickerupper)) {
        Broc::bint playerState;
        Broc::entity::__unnamed::playerState_struct playerStateField = {
            pickerupper.GetHandle()};
        playerStateField.Get(&playerState);
        if ((int)playerState != 3 || Broc::Code_IsInVehicle(pickerupper)) {
            myteam.~string();
            return;
        }

        Broc::string flagteam("axis");
        Broc::string otherteam("allies");
        Broc::bint now;
        Broc::GetTime(&now);
        Broc::bint captureTime = Broc::operator+(now, 1000);
        mp_util_wad::pLevel->_base.entity->pickupCaptureDelayTime =
            (int)captureTime;
        ObjectiveRing(2, -1);

        Broc::bbool hasSound;
        mp_util_wad::IsEEDefined_sound_handle(&hasSound, pickerupper);
        if ((bool)hasSound) {
            Broc::EffectEventStopEmitting(
                (unsigned int)(int)*mp_util_wad::GetEE_sound_handle(
                    pickerupper));
            *mp_util_wad::GetEE_sound_handle(pickerupper) = 0;
        }

        Broc::string script("FLAG_FLAPPING");
        int soundHandle = Broc::EffectEventPlay(&pickerupper, &script);
        *mp_util_wad::GetEE_sound_handle(pickerupper) = soundHandle;
        script.~string();

        Broc::string pickerTeam;
        Broc::entity::__unnamed::team_struct pickerTeamField = {
            pickerupper.GetHandle()};
        pickerTeamField.Get(&pickerTeam);
        bool axisPicker = pickerTeam == "axis";
        pickerTeam.~string();

        if (axisPicker) {
            myteam = "axis";
            bool isLocalPlayer = Broc::Code_IsLocalPlayer(pickerupper);
            if (!isLocalPlayer) {
                Broc::dyn_array<Broc::entity> players;
                Broc::GetLocalPlayerArray(&players);
                Broc::bint i(0);
                while ((int)i < Broc::size(players)) {
                    Broc::string pickerTeamForPlayer;
                    Broc::string playerTeam;
                    pickerTeamField.Get(&pickerTeamForPlayer);
                    Broc::entity::__unnamed::team_struct playerTeamField = {
                        players[(unsigned int)(int)i].GetHandle()};
                    playerTeamField.Get(&playerTeam);
                    bool sameTeam = playerTeam == pickerTeamForPlayer;
                    (void)sameTeam;
                    playerTeam.~string();
                    pickerTeamForPlayer.~string();
                    i = (int)i + 1;
                }
                players.~dyn_array();
            }
            if (playSounds != 0) {
                Broc::entity level = mp_util_wad::pLevel != nullptr
                                          ? mp_util_wad::pLevel->_base.entity
                                          : Broc::entity();
                void* ftor = _mp_audio::PlayTeamSound__functor(
                    level, myteam,
                    Broc::string("MX_SFCTF_FlagTakenEnemy"),
                    Broc::string("MX_SFCTF_FlagTaken"));
                Broc::thread_create(false,
                                    "c:\\cod\\code\\script\\_mp_scf.bro",
                                    __LINE__, "_mp_audio::PlayTeamSound",
                                    ftor);
                if ((bool)atBase)
                    Broc::iprintln("MPSCF_FLAG_TAKEN_AXIS");
            }
        } else {
            myteam = "allies";
            flagteam = "allies";
            bool isLocalPlayer = Broc::Code_IsLocalPlayer(pickerupper);
            if (!isLocalPlayer) {
                Broc::dyn_array<Broc::entity> players;
                Broc::GetLocalPlayerArray(&players);
                Broc::bint i(0);
                while ((int)i < Broc::size(players)) {
                    Broc::string pickerTeamForPlayer;
                    Broc::string playerTeam;
                    pickerTeamField.Get(&pickerTeamForPlayer);
                    Broc::entity::__unnamed::team_struct playerTeamField = {
                        players[(unsigned int)(int)i].GetHandle()};
                    playerTeamField.Get(&playerTeam);
                    bool sameTeam = playerTeam == pickerTeamForPlayer;
                    (void)sameTeam;
                    playerTeam.~string();
                    pickerTeamForPlayer.~string();
                    i = (int)i + 1;
                }
                players.~dyn_array();
            }
            if (playSounds != 0) {
                Broc::entity level = mp_util_wad::pLevel != nullptr
                                          ? mp_util_wad::pLevel->_base.entity
                                          : Broc::entity();
                void* ftor = _mp_audio::PlayTeamSound__functor(
                    level, myteam,
                    Broc::string("MX_SFCTF_FlagTakenEnemy"),
                    Broc::string("MX_SFCTF_FlagTaken"));
                Broc::thread_create(false,
                                    "c:\\cod\\code\\script\\_mp_scf.bro",
                                    __LINE__, "_mp_audio::PlayTeamSound",
                                    ftor);
                if ((bool)atBase)
                    Broc::iprintln("MPSCF_FLAG_TAKEN_ALLIES");
            }
        }

        Broc::string pickerTeamForWeapon;
        pickerTeamField.Get(&pickerTeamForWeapon);
        bool axisWeapon = pickerTeamForWeapon == "axis";
        pickerTeamForWeapon.~string();
        if (axisWeapon) {
            Broc::string weapon("mp_flag_axis");
            Broc::GiveWeapon(&pickerupper, &weapon);
            weapon.~string();
        } else {
            Broc::string weapon("mp_flag_allies");
            Broc::GiveWeapon(&pickerupper, &weapon);
            weapon.~string();
        }

        Broc::string slot("flag");
        Broc::SetWeaponSlotClipAmmo(&pickerupper, &slot, 1);
        slot.~string();

        Broc::string pickerTeamForSwitch;
        pickerTeamField.Get(&pickerTeamForSwitch);
        bool axisSwitch = pickerTeamForSwitch == "axis";
        pickerTeamForSwitch.~string();
        if (axisSwitch) {
            Broc::string weapon("mp_flag_axis");
            Broc::SwitchToWeapon(pickerupper, weapon);
            weapon.~string();
        } else {
            Broc::string weapon("mp_flag_allies");
            Broc::SwitchToWeapon(pickerupper, weapon);
            weapon.~string();
        }

        __int16 hasFlag = 1;
        Broc::entity::__unnamed::ctf_has_flag_struct ctfHasFlagField = {
            pickerupper.GetHandle()};
        ctfHasFlagField = hasFlag;
        EntityOff(flag);
        EntityOff(*mp_util_wad::GetEE_trigger(flag));
        *mp_util_wad::GetEE_holder(flag) = pickerupper;
        *mp_util_wad::GetEE_holder(pickerupper) = flag;
        Broc::SetOwner(&flag, &pickerupper);

        if ((bool)atBase && request == 0)
            _mp_common::AddToPlayerStats(pickerupper, Broc::bint(18), 1);

        if (Broc::Code_IsLocalPlayer(pickerupper)) {
            void* ftor = CompassUnderlay__functor(pickerupper);
            Broc::thread_create(false,
                                "c:\\cod\\code\\script\\_mp_scf.bro",
                                __LINE__, "CompassUnderlay", ftor);
        }

        otherteam.~string();
        flagteam.~string();
    } else {
        EntityOff(flag);
        EntityOff(*mp_util_wad::GetEE_trigger(flag));
        Broc::entity level = mp_util_wad::pLevel != nullptr
                                  ? mp_util_wad::pLevel->_base.entity
                                  : Broc::entity();
        Broc::bfloat waitTime(0.1f);
        void* ftor = WaitThenPickFlagToLaunch__functor(
            level, waitTime, "MPSCF_FLAG_RETURNED");
        Broc::thread_create(false,
                            "c:\\cod\\code\\script\\_mp_scf.bro",
                            __LINE__, "WaitThenPickFlagToLaunch", ftor);
    }
    myteam.~string();
}

// CallbackPickupScriptItem - ea: 0x969830
void CallbackPickupScriptItem(int netID, Broc::entity guy, int itemIndex) {
    Broc::Code_DebugOut("*SCF* CallbackPickupScriptItem\n");

    if ((netID < 0 || netID >= Broc::size(mp_util_wad::pLevel->scfFlags)) &&
        Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_scf.bro", __LINE__ + 3,
            "Invalid flag pickup index"))
        __debugbreak();

    Broc::entity flag =
        mp_util_wad::pLevel->scfFlags[(unsigned int)netID];
    if (!Broc::IsDefined(flag) &&
        Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_scf.bro", __LINE__ + 5,
            "Could not get flag entity"))
        __debugbreak();

    Broc::bbool playSounds(true);
    Broc::bbool atBase;
    IsFlagAtBase(&atBase, flag);

    Broc::vector pickerOrigin;
    Broc::vector flagOrigin;
    Broc::entity::__unnamed::origin_struct pickerOriginField = {
        guy.GetHandle()};
    pickerOriginField.Get(&pickerOrigin);
    Broc::entity::__unnamed::origin_struct flagOriginField = {
        flag.GetHandle()};
    flagOriginField.Get(&flagOrigin);

    if (*mp_util_wad::GetEE_holder(flag) != Broc::gEntityUndef ||
        ((bool)atBase &&
         Broc::Distance(&pickerOrigin, &flagOrigin) > 256.0f)) {
        if (*mp_util_wad::GetEE_holder(flag) == guy) {
            playSounds = false;
        } else if (itemIndex == 0) {
            UnlinkFlag(flag);
            playSounds = false;
        }

        HashStr pickupNotify(0x87404C8Eu);
        Broc::notify(flag, pickupNotify);
        HandlePickupFlag(flag, guy, itemIndex, (bool)playSounds);
        return;
    }

    if (itemIndex == 0)
        goto notify_pickup;

    if (Broc::Code_IsHost()) {
        Broc::Code_PickupItem(netID, guy);
        goto notify_pickup;
    }

    if (Broc::Code_IsLocalPlayer(guy))
        goto notify_pickup;
    return;

notify_pickup:
    {
        HashStr pickupNotify(0x87404C8Eu);
        Broc::notify(flag, pickupNotify);
        HandlePickupFlag(flag, guy, itemIndex, (bool)playSounds);
    }
}

// CallbackDropItem - ea: 0x969B30
void CallbackDropItem(int itemType, int netID, Broc::vector locator,
                      Broc::vector angles, Broc::vector velocity) {
    Broc::Code_DebugOut("*SCF* CallbackDropItem\n");
    (void)itemType;

    Broc::entity flag;
    if ((netID < 0 || netID > Broc::size(mp_util_wad::pLevel->scfFlags)) &&
        Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_scf.bro", __LINE__ + 5,
            "Invalid drop item ID"))
        __debugbreak();

    flag = mp_util_wad::pLevel->scfFlags[(unsigned int)netID];

    HashStr dropNotify(0xFA57E7C7u);
    Broc::notify(flag, dropNotify);

    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, flag);
    if ((bool)hasHolder && !(bool)mp_util_wad::pLevel->roundOver) {
        Broc::string sound("MX_SFCTF_FlagDropped");
        Broc::SoundPlay(&sound, 1.0f);
        sound.~string();
        Broc::iprintln("MPSCF_FLAG_DROPPED");
    }

    LaunchFlag(Broc::bint(netID), locator, angles, velocity);
    ObjectiveRing(2, -1);

    Broc::vector& home = flag->home_position.GetRef();
    if (Broc::Distance(&locator, &home) > 2.0f ||
        Broc::Length(&velocity) > 1.0f) {
        void* ftor = WaitForFlagTimeOut__functor(flag);
        Broc::thread_create(false,
                            "c:\\cod\\code\\script\\_mp_scf.bro",
                            __LINE__ + 27, "WaitForFlagTimeOut", ftor);
    }
}

// Goal - ea: 0x96A6E0
void Goal(Broc::entity self, Broc::entity triggerer) {
    Broc::dyn_array<Broc::entity> players;
    Broc::entity flag_holder;

    if (!(bool)mp_util_wad::pLevel->roundStarted ||
        Broc::IsPlayer(triggerer) == 0)
        return;

    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, triggerer);
    if (!(bool)hasHolder || Broc::Code_IsInVehicle(triggerer) ||
        !Broc::Code_IsLocalPlayer(triggerer))
        return;

    Broc::bint captureDelay;
    mp_util_wad::pLevel->_base.entity->pickupCaptureDelayTime.Get(
        &captureDelay);
    Broc::bint now;
    Broc::GetTime(&now);
    if ((int)captureDelay >= (int)now)
        return;

    HashStr goalHash;
    Broc::string_hash(&goalHash, "_mp_scf::Goal");
    HashStr eventLabel(0xF2F5EAB4u);
    Broc::RemoveEventHandler(&self, eventLabel, goalHash);

    {
        Broc::string triggererTeam;
        Broc::entity::__unnamed::team_struct triggererTeamField = {
            triggerer.GetHandle()};
        triggererTeamField.Get(&triggererTeam);
        bool captureAxis = false;
        if (triggererTeam == "axis") {
            Broc::entity::__unnamed::targetname_struct baseTargetnameField = {
                self.GetHandle()};
            Broc::string baseTargetname;
            baseTargetnameField.Get(&baseTargetname);
            captureAxis = baseTargetname == "scf_base_allies";
        }
        if (captureAxis) {
            Broc::Code_AreaCaptured(0, 1, 0);
            Broc::wait(1.0f);
        }
    }

    {
        Broc::string triggererTeam;
        Broc::entity::__unnamed::team_struct triggererTeamField = {
            triggerer.GetHandle()};
        triggererTeamField.Get(&triggererTeam);
        bool captureAllies = false;
        if (triggererTeam == "allies") {
            Broc::entity::__unnamed::targetname_struct baseTargetnameField = {
                self.GetHandle()};
            Broc::string baseTargetname;
            baseTargetnameField.Get(&baseTargetname);
            captureAllies = baseTargetname == "scf_base_axis";
        }
        if (captureAllies) {
            Broc::Code_AreaCaptured(0, 0, 0);
            Broc::wait(1.0f);
        }
    }

    Broc::string_hash(&goalHash, "_mp_scf::Goal");
    Broc::AddEventHandler(&self, eventLabel.mVal, goalHash.mVal);
}

// RenderFlagInfo - ea: 0x96BCD0
void RenderFlagInfo(Broc::entity flag, Broc::bint x, Broc::bint y) {
    if (!Broc::IsDefined(flag))
        return;

    Broc::string text((const char*)NULL);
    text = "Flag Key: ";
    Broc::bint key;
    Broc::entity::__unnamed::key_struct keyField = {flag.GetHandle()};
    keyField.Get(&key);
    text += (int)key;

    Broc::bbool hasHolder;
    mp_util_wad::IsEEDefined_holder(&hasHolder, flag);
    if ((bool)hasHolder) {
        Broc::entity holder = *mp_util_wad::GetEE_holder(flag);
        Broc::string name(Broc::Code_GetPlayerName(holder));
        text += " is held by ";
        text += name;

        Broc::bbool holderHasHolder;
        mp_util_wad::IsEEDefined_holder(&holderHasHolder, holder);
        if (!(bool)holderHasHolder ||
            *mp_util_wad::GetEE_holder(holder) != flag)
            text += ". Who DOES NOT THINK HE IS HOLDING THE FLAG.";
        name.~string();
    } else {
        Broc::vector flagOrigin;
        Broc::entity::__unnamed::origin_struct originField = {
            flag.GetHandle()};
        originField.Get(&flagOrigin);
        Broc::vector* home = mp_util_wad::GetEE_home_position(flag);
        ::bint distance(Broc::Distance(home, &flagOrigin));
        if ((int)distance < 60) {
            text += " is at the base.";
        } else {
            text += "is dropped at position (";
            {
                Broc::string value(flagOrigin[0]);
                text += value;
            }
            text += ",";
            {
                Broc::string value(flagOrigin[1]);
                text += value;
            }
            text += ",";
            {
                Broc::string value(flagOrigin[2]);
                text += value;
            }
            text += ")";
        }
    }

    Broc::Code_DebugRenderText(text.c_str(), (int)x, (int)y);
    text.~string();
}

// CallbackDebugRender - ea: 0x96C140
void CallbackDebugRender() {
    _mp_common::CallbackDebugRender();
    if (Broc::GetCvarInt("mp_debugrender") != 1)
        return;

    Broc::string temp((const char*)NULL);
    Broc::bint x(40);
    Broc::bint y(70);
    Broc::bint y_inc(20);
    Broc::bint i(0);
    Broc::vector green(0.0f, 1.0f, 0.0f);

    while ((int)i < Broc::size(mp_util_wad::pLevel->scfFlags)) {
        Broc::entity flag = mp_util_wad::pLevel->scfFlags[(unsigned int)(int)i];
        if (Broc::IsDefined(flag)) {
            Broc::entity trigger = *mp_util_wad::GetEE_trigger(flag);
            Broc::vector triggerOrigin;
            Broc::entity::__unnamed::origin_struct triggerOriginField = {
                trigger.GetHandle()};
            triggerOriginField.Get(&triggerOrigin);
            Broc::Code_DebugRenderSphere(&triggerOrigin, 10.0f, &green, 1.0f);
            Broc::Code_DebugRenderEntityBBox(trigger, &green, 0.4f);
            RenderFlagInfo(flag, x, y);
            y += (int)y_inc;
        }
        ++i;
    }

    if (Broc::IsDefined(mp_util_wad::pLevel->base_allies)) {
        Broc::vector origin;
        Broc::entity::__unnamed::origin_struct originField = {
            mp_util_wad::pLevel->base_allies.GetHandle()};
        originField.Get(&origin);
        Broc::Code_DebugRenderSphere(&origin, 10.0f, &green, 1.0f);
        Broc::Code_DebugRenderEntityBBox(mp_util_wad::pLevel->base_allies,
                                         &green, 0.4f);
    }

    Broc::vector red(1.0f, 0.0f, 0.0f);
    if (Broc::IsDefined(mp_util_wad::pLevel->base_axis)) {
        Broc::vector origin;
        Broc::entity::__unnamed::origin_struct originField = {
            mp_util_wad::pLevel->base_axis.GetHandle()};
        originField.Get(&origin);
        Broc::Code_DebugRenderSphere(&origin, 10.0f, &red, 1.0f);
        Broc::Code_DebugRenderEntityBBox(mp_util_wad::pLevel->base_axis,
                                         &red, 0.4f);
    }

    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint playerIndex(0);
    while ((int)playerIndex < Broc::size(players)) {
        Broc::entity player = players[(unsigned int)(int)playerIndex];
        temp = Broc::string(playerIndex);
        temp += " Name: ";
        temp += Broc::Code_GetPlayerName(player);
        temp += " State: ";

        Broc::bint playerState;
        Broc::entity::__unnamed::playerState_struct playerStateField = {
            player.GetHandle()};
        playerStateField.Get(&playerState);
        switch ((int)playerState) {
        case 0: temp += " JOINING "; break;
        case 1: temp += " SPECTATING "; break;
        case 2: temp += " INTERMISSION "; break;
        case 3: temp += " PLAYING "; break;
        case 4: temp += " CRITICAL "; break;
        case 5: temp += " DEAD "; break;
        default: break;
        }

        temp += " Flag: ";
        Broc::bbool hasHolder;
        mp_util_wad::IsEEDefined_holder(&hasHolder, player);
        if ((bool)hasHolder) {
            temp += "true";
            Broc::entity holder = *mp_util_wad::GetEE_holder(player);
            Broc::bbool holderDefined;
            mp_util_wad::IsEEDefined_holder(&holderDefined, holder);
            if (!(bool)holderDefined ||
                *mp_util_wad::GetEE_holder(holder) != player)
                temp += " Flag IS NOT CORRECTLY CONNECTED TO THE PLAYER";
        } else {
            temp += "false";
        }

        temp += " Icon: ";
        Broc::entity::__unnamed::ctf_has_flag_struct ctfHasFlagField = {
            player.GetHandle()};
        temp += ctfHasFlagField.Get() != 0 ? "true" : "false";
        Broc::Code_DebugRenderText(temp.c_str(), (int)x, (int)y);
        y += (int)y_inc;
        ++playerIndex;
    }
}

// ObjectiveUpdater - ea: 0x965F90
void ObjectiveUpdater(Broc::entity guy) {
    Broc::bbool firstLoop(true);
    for (;;) {
        if (!(bool)firstLoop)
            Broc::wait(0.1f);
        firstLoop = false;
        Broc::string pszString("flag");
        Broc::string state("i_flag_axis_c");
        Broc::vector pos;
        mp_util_wad::entity_get_origin(&pos, mp_util_wad::pLevel->base_axis);
        ObjectiveAdd(1, state, pszString, pos,
                           (float)lSCFObjectiveDontShow,
                           Broc::GetPlayerIndex(guy));
        state.~string();
        pszString.~string();
        Broc::string pszString2("flag");
        Broc::string state2("i_flag_allied_c");
        Broc::vector pos2;
        mp_util_wad::entity_get_origin(&pos2, mp_util_wad::pLevel->base_allies);
        ObjectiveAdd(0, state2, pszString2, pos2,
                           (float)lSCFObjectiveDontShow,
                           Broc::GetPlayerIndex(guy));
        state2.~string();
        pszString2.~string();
        ObjectiveDelete(2, Broc::GetPlayerIndex(guy));
    }
}
}

// ============================================================================
// _mp_hq - headquarters game type.
// ============================================================================
namespace _mp_hq {

static Broc::bfloat lHQObjectiveHeight(30.0f);

// main - ea: 0x956920
void main(Broc::entity self) {
    Broc::Code_DebugOut("*HQ* main\n");
    Broc::bbool team_game(true);
    Broc::Code_SetTeamGame((bool)team_game);
    mp_util_wad::pLevel->mustHaveBothTeamsToStart = false;
    mp_util_wad::pLevel->lastManStanding = false;
    mp_util_wad::pLevel->spawnTypeAllies = "spawn_hq_allies_primary";
    mp_util_wad::pLevel->spawnTypeAxis = "spawn_hq_axis_primary";
    _mp_common::SetupCallbacks(team_game);
    mp_util_wad::pLevel->PickSpawnPoint = (void*)GetSpawnPoint;
    Broc::BrocExports& x = Broc::gBrocAPI.mBrocExports;
    x.mCallbackPlayerJoin = CallbackPlayerJoin;
    x.mCallbackPlayerEnter = CallbackPlayerEnter;
    x.mCallbackGameState = CallbackGameState;
    x.mCallbackGameStateHQ = CallbackGameStateHQ;
    x.mCallbackPlayerKilled = CallbackPlayerKilled;
    x.mCallbackPlayerLeave = CallbackPlayerLeave;
    x.mCallbackNextRound = CallbackNextRound;
    x.mCallbackRoundOver = CallbackRoundOver;
    x.mCallbackHostOptionsChanged = CallbackHostOptionsChanged;
    x.mCallbackHostMigrated = (void (*)())CallbackHostMigrated;
    x.mCallbackDebugRender = (void (*)())CallbackDebugRender;
    x.mCallbackGetHQCaptureStatus = CallbackGetHQCaptureStatus;
    x.mCallbackGetTeamCapturingHQPercent = CallbackGetTeamCapturingHQPercent;
    x.mCallbackGetTeamDestroyingHQPercent =
        CallbackGetTeamDestroyingHQPercent;
    Broc::SetTutorialText(-1, 0);
    Broc::string val("hq_point");
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&val, key.mVal, &mp_util_wad::pLevel->HQpoints, 0);
    val.~string();
    SortPoints();
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        *mp_util_wad::GetEE_numDeaths(p) = 0;
        *mp_util_wad::GetEE_setting_up_hq(p) = 0;
        i = (int)i + 1;
    }
    players.~dyn_array();
    Broc::wait(1.0f);
    void* started = StartGame__functor(self);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                        __LINE__, "StartGame", started);
}

// StartGame - ea: 0x956D40
void StartGame(Broc::entity self) {
    (void)self;
    Broc::Code_DebugOut("*HQ* StartGame\n");
    Broc::wait(0.5f);
    mp_util_wad::pLevel->hq_stage = 0;
    mp_util_wad::pLevel->radioTriggerTime = 0;
    mp_util_wad::pLevel->triggerIndex = 0;
    mp_util_wad::pLevel->teamCantRespawn = "";
    mp_util_wad::pLevel->last_HQ_Point_key = 0;
    _mp_common::StartRound(Broc::bbool(true));
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = _mp_common::RunFrame__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                        __LINE__, "_mp_common::RunFrame", ftor);
    if (Broc::IsLocalHost())
        Host_ResetStage1();
    Broc::Code_EnterGame();
}

// Host_ResetStage1 - ea: 0x956EB0
unsigned int Host_ResetStage1() {
    Broc::Code_DebugOut("*HQ* Host_ResetStage1\n");
    Host_PickInitialPoints();
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* ftor = Host_FlowControl__functor(lvl);
    return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                               __LINE__, "Host_FlowControl", ftor);
}

// Host_FlowControl - ea: 0x956FE0
void Host_FlowControl(Broc::entity self) {
    (void)self;
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr e1;
    e1.mVal = 0x9DD2E1FD;
    Broc::endon(lvl, e1);
    HashStr e2;
    e2.mVal = 0x863B4D44;
    Broc::endon(lvl, e2);
    while (!(bool)mp_util_wad::pLevel->roundStarted ||
           (bool)mp_util_wad::pLevel->roundOver)
        Broc::wait(0.1f);
    if ((int)mp_util_wad::pLevel->hq_stage <= 1) {
        Broc::Code_DebugOut("*HQ* StageTimer 1\n");
        mp_util_wad::pLevel->hq_stage = 1;
        SetupStage1();
        BroadcastGameState();
        Broc::wait(5.0f);
        Broc::iprintln("MPHQ_TIME_TILL_RADIO_SPAWN");
        mp_util_wad::pLevel->hq_stage_time = 15;
        Broc::wait((float)(int)mp_util_wad::pLevel->hq_stage_time);
    }
    if ((int)mp_util_wad::pLevel->hq_stage <= 2) {
        Broc::Code_DebugOut("*HQ* StageTimer 2\n");
        mp_util_wad::pLevel->hq_stage = 2;
        SwitchToRadioOnly();
        BroadcastGameState();
        while ((int)mp_util_wad::pLevel->hq_stage < 3)
            Broc::wait(1.0f);
    }
    if ((int)mp_util_wad::pLevel->hq_stage < 5) {
        Broc::Code_DebugOut("*HQ* StageTimer 3\n");
        BroadcastGameState();
        SetUpStage3();
        mp_util_wad::pLevel->hq_stage_time = 90;
        mp_util_wad::pLevel->hq_stage_time =
            (int)mp_util_wad::pLevel->hq_stage_time - 1;
        while ((int)mp_util_wad::pLevel->hq_stage_time > -1 &&
               (int)mp_util_wad::pLevel->hq_stage != 5) {
            Broc::wait(1.0f);
            mp_util_wad::pLevel->hq_stage_time =
                (int)mp_util_wad::pLevel->hq_stage_time - 1;
            if ((bool)mp_util_wad::pLevel->allies_defending)
                AddPoints(Broc::bint(0), Broc::bint(1));
            else
                AddPoints(Broc::bint(1), Broc::bint(0));
        }
        SyncScores();
    }
    if ((int)mp_util_wad::pLevel->hq_stage == 5) {
        Broc::Code_DebugOut("*HQ* StageTimer 5\n");
        BroadcastGameState();
        HQ_Destroyed();
    } else {
        Broc::Code_DebugOut("*HQ* StageTimer 4\n");
        mp_util_wad::pLevel->hq_stage = 4;
        BroadcastGameState();
        HQ_Defended();
    }
}

// AddPoints - ea: 0x9573E0
void AddPoints(Broc::bint forAllies, Broc::bint forAxis) {
    if ((int)forAllies != 0) {
        Broc::string team("allies");
        Broc::Code_IncTeamScore(team, (int)forAllies);
    }
    if ((int)forAxis != 0) {
        Broc::string team("axis");
        Broc::Code_IncTeamScore(team, (int)forAxis);
    }
}

// NoRespawnForDefenders - ea: 0x9574D0
void NoRespawnForDefenders(Broc::bbool no_respawn) {
    if ((bool)no_respawn) {
        if ((bool)mp_util_wad::pLevel->allies_defending)
            mp_util_wad::pLevel->teamCantRespawn = "allies";
        else
            mp_util_wad::pLevel->teamCantRespawn = "axis";
    } else {
        mp_util_wad::pLevel->teamCantRespawn = "reset";
    }
}

// Host_PickInitialPoints - ea: 0x958390
char Host_PickInitialPoints() {
    Broc::Code_DebugOut("*HQ* Host_PickInitialPoints\n");
    Broc::bint num_potential_points(Broc::size(mp_util_wad::pLevel->HQpoints));
    if ((int)num_potential_points < 1)
        return 0;
    Broc::bint firstHQpoint(Broc::RandomInt((int)num_potential_points));
    Broc::entity eA = mp_util_wad::pLevel->HQpoints[(unsigned int)(int)firstHQpoint];
    Broc::vector p;
    mp_util_wad::entity_get_origin(&p, eA);
    mp_util_wad::pLevel->pointA = p;
    mp_util_wad::pLevel->pointA_isHQ = true;
    Broc::string target;
    mp_util_wad::entity_get_target(&target, eA);
    if (!Broc::IsDefined(target) &&
        Broc::gBrocAPI.mAssert(
            "c:\\cod\\code\\script\\_mp_hq.bro", __LINE__,
            "target not defined for HQ points"))
        __debugbreak();
    target.~string();
    if ((bool)mp_util_wad::pLevel->pointA_isHQ) {
        Broc::string t2;
        mp_util_wad::entity_get_target(&t2, eA);
        HashStr key;
        key.mVal = 0x19F9F0E8u;
        Broc::entity trig;
        Broc::entity levelEntityForTrigger =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        *mp_util_wad::GetEE_trigger(levelEntityForTrigger) =
            *Broc::GetEnt(&trig, &t2, key, 0);
        t2.~string();
    }
    Broc::entity levelEntity =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    Broc::bbool hasTrig;
    mp_util_wad::IsEEDefined_trigger(&hasTrig, levelEntity);
    if ((bool)hasTrig) {
        Broc::entity trig = *mp_util_wad::GetEE_trigger(levelEntity);
        mp_util_wad::pLevel->triggerIndex = (int)mp_util_wad::entity_get_key(trig);
    }
    return (char)((bool)hasTrig);
}

// SetupStage1 - ea: 0x958870
void SetupStage1() {
    RemoveRadioModel();
    NoRespawnForDefenders(Broc::bbool(false));
}

// ShowHQDestroyed - ea: 0x958EE0
int ShowHQDestroyed() {
    Broc::Code_DebugOut("*HQ* ShowHQDestroyed\n");
    if ((bool)mp_util_wad::pLevel->allies_defending) {
        Broc::iprintln("MPHQ_ALLIED_HQ_SHUTDOWN");
        Broc::iprintln("MPHQ_AXIS_AWARD_POINTS");
    } else {
        Broc::iprintln("MPHQ_AXIS_HQ_SHUTDOWN");
        Broc::iprintln("MPHQ_ALLIES_AWARD_POINTS");
    }
    return (mp_util_wad::pLevel->string_time = 3000);
}

// ShowHQDefended - ea: 0x958F90
int ShowHQDefended() {
    Broc::Code_DebugOut("*HQ* ShowHQDefended\n");
    if ((bool)mp_util_wad::pLevel->allies_defending)
        Broc::iprintln("MPHQ_ALLIED_HQ_DEFENDED");
    else
        Broc::iprintln("MPHQ_AXIS_HQ_DEFENDED");
    return (mp_util_wad::pLevel->string_time = 3000);
}

// ShowHQSetUp - ea: 0x959010
int ShowHQSetUp() {
    Broc::Code_DebugOut("*HQ* ShowHQSetUp\n");
    mp_util_wad::pLevel->radioTriggerTime = 0;
    if ((bool)mp_util_wad::pLevel->allies_defending)
        Broc::iprintln("MPHQ_ALLIES_SETUP");
    else
        Broc::iprintln("MPHQ_AXIS_SETUP");
    return (mp_util_wad::pLevel->string_time = 3000);
}

// ShowSetupGraphic - ea: 0x9590B0
void ShowSetupGraphic(Broc::entity guy) {
    Broc::SetActionHint((int)0x855384Fu, Broc::GetPlayerIndex(guy));
    mp_util_wad::pLevel->string_time = 600;
}

// ShowDestructionGraphic - ea: 0x959120
void ShowDestructionGraphic(Broc::entity guy) {
    Broc::SetActionHint((int)0x61710355u, Broc::GetPlayerIndex(guy));
    mp_util_wad::pLevel->string_time = 600;
}

// ShowLosingHQGraphic - ea: 0x959190
void ShowLosingHQGraphic(Broc::entity guy) {
    Broc::SetActionHint((int)0x5B051179u, Broc::GetPlayerIndex(guy));
    mp_util_wad::pLevel->string_time = 600;
}

// ShowProgressBar - ea: 0x959200
void ShowProgressBar(Broc::entity self, Broc::string colour) {
    (void)self;
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr e;
    e.mVal = 0x863B4D44;
    Broc::endon(lvl, e);
    Broc::bint val_last_time(0);
    Broc::bint lifeMS(600);
    for (;;) {
        if ((int)mp_util_wad::pLevel->radioTriggerTime == (int)val_last_time)
            lifeMS -= 200;
        val_last_time = (int)mp_util_wad::pLevel->radioTriggerTime;
        if ((int)mp_util_wad::pLevel->radioTriggerTime == 0)
            break;
        if ((int)lifeMS < 0)
            break;
        Broc::wait(0.2f);
    }
    mp_util_wad::pLevel->radioTriggerTime = 0;
    colour.~string();
}

// ClearGame - ea: 0x95B2C0
void ClearGame() {
    Broc::Code_DebugOut("*HQ* ClearGame\n");
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr n;
    n.mVal = 0x12CEF01u;
    Broc::notify(lvl, n);
    mp_util_wad::pLevel->hq_stage = 0;
    ObjectiveDelete(0, -1);
    RemoveRadioModel();
    HashStr trigHash;
    Broc::string_hash(&trigHash, "_mp_hq::TriggerRadio");
    HashStr label;
    label.mVal = 0xF2F5EAB4;
    Broc::RemoveEventHandler(mp_util_wad::GetEE_trigger(lvl), label,
                             trigHash);
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        *mp_util_wad::GetEE_numDeaths(p) = 0;
        *mp_util_wad::GetEE_setting_up_hq(p) = 0;
        i = (int)i + 1;
    }
    players.~dyn_array();
}

// ResetGame - ea: 0x95B540
void ResetGame(Broc::entity self) {
    (void)self;
    Broc::Code_DebugOut("*HQ* ResetGame\n");
    ClearGame();
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr n;
    n.mVal = 0x9DD2E1FD;
    Broc::notify(lvl, n);
    Broc::wait(3.0f);
    if (Broc::IsLocalHost() && !(bool)mp_util_wad::pLevel->roundOver) {
        Broc::wait(1.0f);
        Host_ResetStage1();
    }
}

// GetSpawnPoint - ea: 0x95B610
Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* self,
                            const Broc::string* team) {
    Broc::dyn_array<Broc::entity> spawnpoints;
    Broc::string spawnType = mp_util_wad::pLevel->spawnTypeAllies;
    if (*team == "axis")
        spawnType = mp_util_wad::pLevel->spawnTypeAxis;
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&spawnType, key.mVal, &spawnpoints, 0);
    _mp_spawnlogic::get_spawnpoint_near_team_away_from_radios(
        result, self, team, &spawnpoints);
    spawnType.~string();
    spawnpoints.~dyn_array();
    return result;
}

// GetTriggerFromIndex - ea: 0x95B740
void GetTriggerFromIndex() {
    Broc::dyn_array<Broc::entity> triggers;
    Broc::string triggerType("trigger_multiple");
    HashStr key;
    key.mVal = 0xF756C677;
    Broc::GetEntArray(&triggerType, key.mVal, &triggers, 0);
    Broc::bint i(0);
    while ((int)i < Broc::size(triggers)) {
        Broc::entity t = triggers[(unsigned int)(int)i];
        Broc::bint k;
        if ((int)mp_util_wad::entity_get_key(t) ==
            (int)mp_util_wad::pLevel->triggerIndex) {
            Broc::entity level_entity = mp_util_wad::pLevel->_base.entity;
            *mp_util_wad::GetEE_trigger(level_entity) = t;
            Broc::Code_DebugOut("*HQ* got trigger\n");
            triggerType.~string();
            triggers.~dyn_array();
            return;
        }
        i = (int)i + 1;
    }
    Broc::Code_DebugOut("*HQ* failed to find trigger\n");
    triggerType.~string();
    triggers.~dyn_array();
}

// BroadcastGameState - ea: 0x95B920
void BroadcastGameState() {
    if (Broc::IsLocalHost()) {
        Broc::dyn_array<Broc::entity> players;
        Broc::GetPlayerArray(&players);
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            Broc::Code_SendGameStateHQ(
                players[(unsigned int)(int)i],
                (unsigned int)(int)mp_util_wad::pLevel->hq_stage,
                mp_util_wad::pLevel->pointA,
                mp_util_wad::pLevel->pointB,
                (unsigned int)(int)mp_util_wad::pLevel->triggerIndex,
                (bool)mp_util_wad::pLevel->allies_defending,
                (bool)mp_util_wad::pLevel->pointA_isHQ);
            i = (int)i + 1;
        }
        players.~dyn_array();
    }
}

// SyncScores - ea: 0x95BB30
void SyncScores() {
    Broc::Code_DebugOut("*HQ* SyncScores\n");
    Broc::bint now;
    Broc::GetTime(&now);
    Broc::bfloat timePassed =
        ((int)now - (int)mp_util_wad::pLevel->startTime) * 0.001f;
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::string team("axis");
        Broc::string team2("allies");
        int axisScore = Broc::Code_GetTeamScore(team);
        int alliesScore = Broc::Code_GetTeamScore(team2);
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
            (bool)mp_util_wad::pLevel->roundOver,
            (int)mp_util_wad::pLevel->roundCount);
        team2.~string();
        team.~string();
        i = (int)i + 1;
    }
    players.~dyn_array();
}

// String_Timer - ea: 0x95BF20
void String_Timer() {
    Broc::Code_DebugOut("*HQ* String_Timer\n");
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr e;
    e.mVal = 0x12CEF01u;
    Broc::endon(lvl, e);
    for (;;) {
        Broc::wait(0.2f);
        if ((int)mp_util_wad::pLevel->string_time > 0)
            mp_util_wad::pLevel->string_time -= 200;
        else
            Broc::SetTutorialTextAllPlayers(-1);
    }
}

// CallbackGameStateHQ - ea: 0x95C010
void CallbackGameStateHQ(unsigned int stage, Broc::vector vA, Broc::vector vB,
                         unsigned int triggerIndex, int alliesDefending,
                         int pointAIsHQ) {
    Broc::Code_DebugOut("*HQ* CallbackGameStateHQ\n");
    if ((int)mp_util_wad::pLevel->hq_stage < (int)stage) {
        if ((int)mp_util_wad::pLevel->hq_stage < 1 && stage != 0) {
            Broc::Code_DebugOut("*HQ* going to stage 1\n");
            mp_util_wad::pLevel->pointA = vA;
            mp_util_wad::pLevel->pointB = vB;
            SetupStage1();
        }
        if ((int)mp_util_wad::pLevel->hq_stage < 2 && stage >= 2) {
            Broc::Code_DebugOut("*HQ* going to stage 2\n");
            mp_util_wad::pLevel->triggerIndex = (int)triggerIndex;
            mp_util_wad::pLevel->pointA_isHQ = pointAIsHQ != 0;
            GetTriggerFromIndex();
            SwitchToRadioOnly();
            Broc::string script("MX_HQ_HQReady");
            Broc::entity lvl2;
            lvl2 = mp_util_wad::pLevel != nullptr
                       ? mp_util_wad::pLevel->_base.entity
                       : Broc::entity();
            Broc::EffectEventPlay(&lvl2, &script);
            script.~string();
        }
        if ((int)mp_util_wad::pLevel->hq_stage < 3 && stage >= 3) {
            Broc::Code_DebugOut("*HQ* going to stage 3\n");
            mp_util_wad::pLevel->allies_defending = alliesDefending != 0;
            SetUpStage3();
            Broc::entity lvl =
                mp_util_wad::pLevel != nullptr
                    ? mp_util_wad::pLevel->_base.entity
                    : Broc::entity();
            void* ftor = Track_Ownership__functor(lvl);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                                __LINE__, "Track_Ownership", ftor);
        }
        mp_util_wad::pLevel->hq_stage = (int)stage;
        if (stage == 4) {
            Broc::Code_DebugOut("*HQ* going to stage 4\n");
            HQ_Defended();
        }
        if (stage == 5) {
            Broc::Code_DebugOut("*HQ* going to stage 5\n");
            HQ_Destroyed();
        }
    }
}

// CallbackGameState - ea: 0x95C460
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int roundOver,
                       int roundCount) {
    Broc::Code_DebugOut("*HQ* CallbackGameState\n");
    _mp_common::CallbackGameState(currentMatchTime, timeLimit, scoreLimit,
                                  roundLimit, friendlyFire, 0, teamBalance,
                                  respawnTime, alliesScore, axisScore,
                                  roundStarted, roundOver, roundCount);
    (void)lastManStanding;
}

// CallbackNextRound - ea: 0x95C4D0
void CallbackNextRound() {
    Broc::Code_DebugOut("*HQ* CallbackNextRound\n");
    if (!(bool)mp_util_wad::pLevel->roundOver) {
        Broc::Code_DebugOut(
            "*HQ* CallbackNextRound called when round not considered over, clearing\n");
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        HashStr n;
        n.mVal = 0x863B4D44;
        Broc::notify(lvl, n);
        mp_util_wad::pLevel->roundOver = true;
        ClearGame();
    }
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr n2;
    n2.mVal = 0x6FA23667u;
    Broc::notify(lvl, n2);
    _mp_common::CallbackNextRound();
    if (Broc::IsLocalHost())
        Host_ResetStage1();
}

// CallbackRoundOver - ea: 0x95C5F0
void CallbackRoundOver(int condition, Broc::string team) {
    Broc::Code_DebugOut("*HQ* CallbackRoundOver\n");
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr n;
    n.mVal = 0x863B4D44;
    Broc::notify(lvl, n);
    mp_util_wad::pLevel->roundOver = true;
    ClearGame();
    _mp_common::CallbackRoundOver(condition, team);
    team.~string();
}

// CallbackPlayerJoin - ea: 0x95C6F0
void CallbackPlayerJoin(Broc::entity player, unsigned int playerState,
                        int playerClass) {
    _mp_common::CallbackPlayerJoin(player, playerState, (__int16)playerClass);
}

// CallbackPlayerEnter - ea: 0x95C740
void CallbackPlayerEnter(Broc::entity player, int hot_joiner) {
    Broc::Code_DebugOut("*HQ* CallbackPlayerEnter\n");
    _mp_common::CallbackPlayerEnter(player, hot_joiner);
    Broc::Code_SendGameStateHQ(
        player, (unsigned int)(int)mp_util_wad::pLevel->hq_stage,
        mp_util_wad::pLevel->pointA, mp_util_wad::pLevel->pointB,
        (unsigned int)(int)mp_util_wad::pLevel->triggerIndex,
        (bool)mp_util_wad::pLevel->allies_defending,
        (bool)mp_util_wad::pLevel->pointA_isHQ);
}

// CallbackPlayerKilled - ea: 0x95C820
void CallbackPlayerKilled(Broc::entity player, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health) {
    Broc::Code_DebugOut("*HQ* CallbackPlayerKilled\n");
    _mp_common::CallbackPlayerKilled(player, inflictor, attacker, weapon, mod,
                                     health);
    Broc::string team;
    mp_util_wad::entity_get_team(&team, player);
    if (Broc::Code_IsLocalPlayer(player) &&
        team == mp_util_wad::pLevel->teamCantRespawn) {
        Broc::Code_DebugOut("*HQ* was killed but cannot respawn yet\n");
        *mp_util_wad::GetEE_numDeaths(player) =
            (int)*mp_util_wad::GetEE_numDeaths(player) + 1;
    }
    team.~string();
    if (Broc::IsLocalHost() &&
        (int)mp_util_wad::pLevel->hq_stage == 3) {
        Broc::bint numDefenders(0);
        Broc::dyn_array<Broc::entity> players;
        Broc::GetPlayerArray(&players);
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            Broc::entity p = players[(unsigned int)(int)i];
            Broc::bint state;
            mp_util_wad::entity_get_playerState(&state, p);
            if ((int)state == 3) {
                Broc::string pteam;
                mp_util_wad::entity_get_team(&pteam, p);
                bool defender =
                    (bool)mp_util_wad::pLevel->allies_defending
                        ? pteam == "allies"
                        : pteam == "axis";
                pteam.~string();
                if (defender)
                    numDefenders = (int)numDefenders + 1;
            }
            i = (int)i + 1;
        }
        if ((int)numDefenders == 0)
            mp_util_wad::pLevel->hq_stage = 5;
        players.~dyn_array();
    }
}

// CallbackPlayerLeave - ea: 0x95CCE0
void CallbackPlayerLeave(Broc::entity player) {
    Broc::Code_DebugOut("*HQ* CallbackPlayerLeave\n");
    _mp_common::CallbackPlayerLeave(player);
}

// CallbackHostMigrated - ea: 0x95CD20
unsigned int CallbackHostMigrated() {
    Broc::Code_DebugOut("*HQ* CallbackHostMigrated\n");
    _mp_common::CallbackHostMigrated();
    BroadcastGameState();
    if (Broc::IsLocalHost()) {
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        void* ftor = Host_FlowControl__functor(lvl);
        return Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                                   __LINE__, "Host_FlowControl", ftor);
    }
    return 0;
}

// CallbackHostOptionsChanged - ea: 0x95CDD0
void CallbackHostOptionsChanged(int forceMapChange) {
    Broc::Code_DebugOut("*HQ* CallbackHostOptionsChanged\n");
    _mp_common::CallbackHostOptionsChanged(forceMapChange);
    mp_util_wad::pLevel->lastManStanding = false;
}

// CallbackGetTeamCapturingHQPercent - ea: 0x95CE20
int CallbackGetTeamCapturingHQPercent(Broc::entity my_player) {
    if ((int)mp_util_wad::pLevel->radioTriggerTime <= 0)
        return 0;
    Broc::bint setting((int)*mp_util_wad::GetEE_setting_up_hq(my_player));
    if ((int)setting != 1)
        return 0;
    *mp_util_wad::GetEE_setting_up_hq(my_player) = 0;
    Broc::bfloat pct((float)(int)mp_util_wad::pLevel->radioTriggerTime / 15000.0f);
    if ((float)pct > 1.0f)
        pct = 1.0f;
    return (int)((float)pct * 10000.0f);
}

// CallbackGetTeamDestroyingHQPercent - ea: 0x95CF90
int CallbackGetTeamDestroyingHQPercent() {
    Broc::bint last_time((int)mp_util_wad::pLevel->last_radio_trigger_time);
    Broc::bint current_time;
    Broc::GetTime(&current_time);
    Broc::bint time_delta((int)current_time - (int)last_time);
    if ((int)time_delta > 250)
        mp_util_wad::pLevel->radioTriggerTime = 0;
    if ((int)mp_util_wad::pLevel->radioTriggerTime <= 0)
        return 0;
    Broc::bfloat pct((float)(int)mp_util_wad::pLevel->radioTriggerTime / 15000.0f);
    if ((float)pct > 1.0f)
        pct = 1.0f;
    return (int)((float)pct * 10000.0f);
}

// CallbackGetHQCaptureStatus - ea: 0x95D130
int CallbackGetHQCaptureStatus() {
    if ((int)mp_util_wad::pLevel->hq_stage != 3)
        return 0;
    return (bool)mp_util_wad::pLevel->allies_defending ? 1 : -1;
}

// CallbackDebugRender - ea: 0x95D1A0
void CallbackDebugRender() {
    _mp_common::CallbackDebugRender();
}

// compare - ea: 0x95D1C0
Broc::bint* compare(Broc::bint* result, Broc::vector* first,
                    Broc::vector* second) {
    Broc::bfloat delta(second->x - first->x);
    if ((float)delta != 0.0f ||
        (delta = second->y - first->y, (float)delta != 0.0f)) {
        result->mVal = (int)(float)delta;
        return result;
    }
    delta = second->z - first->z;
    result->mVal = (float)delta != 0.0f ? (int)(float)delta : 1;
    return result;
}

// SortPoints - ea: 0x95D350
int SortPoints() {
    int result = Broc::size(mp_util_wad::pLevel->HQpoints);
    if (result >= 2) {
        Broc::entity temp;
        Broc::bint i(0);
        while ((int)i < Broc::size(mp_util_wad::pLevel->HQpoints)) {
            for (Broc::bint j((int)i); (int)j <
                                      Broc::size(mp_util_wad::pLevel->HQpoints);
                 j = (int)j + 1) {
                Broc::vector a;
                Broc::vector b;
                mp_util_wad::entity_get_origin(
                    &a, mp_util_wad::pLevel->HQpoints[(unsigned int)(int)i]);
                mp_util_wad::entity_get_origin(
                    &b, mp_util_wad::pLevel->HQpoints[(unsigned int)(int)j]);
                Broc::bint c;
                compare(&c, &a, &b);
                if ((int)c > 0) {
                    temp = mp_util_wad::pLevel->HQpoints[(unsigned int)(int)i];
                    mp_util_wad::pLevel->HQpoints[(unsigned int)(int)i] =
                        mp_util_wad::pLevel->HQpoints[(unsigned int)(int)j];
                    mp_util_wad::pLevel->HQpoints[(unsigned int)(int)j] = temp;
                }
            }
            i = (int)i + 1;
        }
    }
    return result;
}

// GetCapSpeed - ea: 0x95D5A0
Broc::bfloat* GetCapSpeed(Broc::bfloat* result, Broc::bint guysCapping) {
    Broc::bfloat value(1.0f);
    Broc::bfloat total(0.0f);
    Broc::bint i(0);
    while ((int)i < (int)guysCapping) {
        total += (float)value;
        value = (float)value * 0.75f;
        i = (int)i + 1;
    }
    *result = (float)total;
    return result;
}

void* main__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(main, self);
}
AeThreadFunctor1<Broc::entity>* StartGame__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(StartGame, self);
}
AeThreadFunctor1<Broc::entity>* Host_FlowControl__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(Host_FlowControl, self);
}
AeThreadFunctor* Track_Ownership__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(Track_Ownership, self);
}
AeThreadFunctor1<Broc::entity>* ResetGame__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(ResetGame, self);
}
void* TriggerRadio__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(TriggerRadio, self);
}

// HQ_Destroyed - ea: 0x957570
void HQ_Destroyed() {
    Broc::Code_DebugOut("*HQ* HQ_Destroyed\n");
    mp_util_wad::pLevel->radioTriggerTime = 0;
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        *mp_util_wad::GetEE_setting_up_hq(p) = 0;
        i = (int)i + 1;
    }
    ShowHQDestroyed();
    if ((bool)mp_util_wad::pLevel->allies_defending)
        AddPoints(Broc::bint(0), Broc::bint(10));
    else
        AddPoints(Broc::bint(10), Broc::bint(0));

    Broc::string script("MX_HQ_HQDestroyed");
    Broc::entity* levelEntity =
        mp_util_wad::pLevel != nullptr
            ? &mp_util_wad::pLevel->_base.entity
            : nullptr;
    Broc::EffectEventPlay(levelEntity, &script);
    script.~string();

    if ((bool)mp_util_wad::pLevel->allies_defending) {
        Broc::entity lvl1 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team1("allies");
        Broc::string sound1("MP_HQ_Destroyed_Allies");
        Broc::bfloat delay1(2.14f);
        void* ftor1 =
            _mp_audio::PlayTeamDialog__functor(lvl1, team1, sound1, delay1);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor1);

        Broc::entity lvl2 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team2("axis");
        Broc::string sound2("MP_HQ_DestroyedEnemy_Axis");
        Broc::bfloat delay2(2.14f);
        void* ftor2 =
            _mp_audio::PlayTeamDialog__functor(lvl2, team2, sound2, delay2);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor2);
    } else {
        Broc::entity lvl1 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team1("axis");
        Broc::string sound1("MP_HQ_Destroyed_Axis");
        Broc::bfloat delay1(2.14f);
        void* ftor1 =
            _mp_audio::PlayTeamDialog__functor(lvl1, team1, sound1, delay1);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor1);

        Broc::entity lvl2 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team2("allies");
        Broc::string sound2("MP_HQ_DestroyedEnemy_Allies");
        Broc::bfloat delay2(2.14f);
        void* ftor2 =
            _mp_audio::PlayTeamDialog__functor(lvl2, team2, sound2, delay2);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor2);
    }

    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* reset = ResetGame__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                        __LINE__, "ResetGame", reset);
    players.~dyn_array();
}

// HQ_Defended - ea: 0x957D10
void HQ_Defended() {
    Broc::Code_DebugOut("*HQ* HQ_Defended\n");
    mp_util_wad::pLevel->radioTriggerTime = 0;
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        *mp_util_wad::GetEE_setting_up_hq(p) = 0;
        i = (int)i + 1;
    }
    ShowHQDefended();
    Broc::string script("MX_HQ_HQDefended");
    Broc::entity* levelEntity =
        mp_util_wad::pLevel != nullptr
            ? &mp_util_wad::pLevel->_base.entity
            : nullptr;
    Broc::EffectEventPlay(levelEntity, &script);
    script.~string();

    if ((bool)mp_util_wad::pLevel->allies_defending) {
        Broc::entity lvl1 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team1("allies");
        Broc::string sound1("MP_HQ_FriendlyDefended_Allies");
        Broc::bfloat delay1(1.2f);
        void* ftor1 =
            _mp_audio::PlayTeamDialog__functor(lvl1, team1, sound1, delay1);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor1);

        Broc::entity lvl2 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team2("axis");
        Broc::string sound2("MP_HQ_EnemyDefended_Axis");
        Broc::bfloat delay2(1.2f);
        void* ftor2 =
            _mp_audio::PlayTeamDialog__functor(lvl2, team2, sound2, delay2);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor2);
    } else {
        Broc::entity lvl1 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team1("axis");
        Broc::string sound1("MP_HQ_FriendlyDefended_Axis");
        Broc::bfloat delay1(1.2f);
        void* ftor1 =
            _mp_audio::PlayTeamDialog__functor(lvl1, team1, sound1, delay1);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor1);

        Broc::entity lvl2 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team2("allies");
        Broc::string sound2("MP_HQ_EnemyDefended_Allies");
        Broc::bfloat delay2(1.2f);
        void* ftor2 =
            _mp_audio::PlayTeamDialog__functor(lvl2, team2, sound2, delay2);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor2);
    }

    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    void* reset = ResetGame__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                        __LINE__, "ResetGame", reset);
    players.~dyn_array();
}

// SetUpStage3 - ea: 0x9588A0
void SetUpStage3() {
    ObjectiveDelete(0, -1);
    if ((bool)mp_util_wad::pLevel->pointA_isHQ) {
        Broc::string pszString("flag");
        Broc::string state("i_HQ_captured_c");
        ObjectiveAdd(0, state, pszString, mp_util_wad::pLevel->pointA,
                     (float)lHQObjectiveHeight, -1);
        state.~string();
        pszString.~string();
    }
    NoRespawnForDefenders(Broc::bbool(true));
    Broc::string script("MX_HQ_HQEstablished");
    Broc::entity* levelEntity =
        mp_util_wad::pLevel != nullptr
            ? &mp_util_wad::pLevel->_base.entity
            : nullptr;
    Broc::EffectEventPlay(levelEntity, &script);
    script.~string();

    if ((bool)mp_util_wad::pLevel->allies_defending) {
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team("allies");
        Broc::string sound("MP_HQ_EstablishFriendly_Allies");
        Broc::bfloat delay(1.2f);
        void* ftor =
            _mp_audio::PlayTeamDialog__functor(lvl, team, sound, delay);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor);

        Broc::entity lvl2 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team2("axis");
        Broc::string sound2("MP_HQ_EstablishEnemy_Axis");
        Broc::bfloat delay2(1.2f);
        void* ftor2 =
            _mp_audio::PlayTeamDialog__functor(lvl2, team2, sound2, delay2);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor2);
    } else {
        Broc::entity lvl =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team("axis");
        Broc::string sound("MP_HQ_EstablishFriendly_Axis");
        Broc::bfloat delay(1.2f);
        void* ftor =
            _mp_audio::PlayTeamDialog__functor(lvl, team, sound, delay);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor);

        Broc::entity lvl2 =
            mp_util_wad::pLevel != nullptr
                ? mp_util_wad::pLevel->_base.entity
                : Broc::entity();
        Broc::string team2("allies");
        Broc::string sound2("MP_HQ_EstablishEnemy_Allies");
        Broc::bfloat delay2(1.2f);
        void* ftor2 =
            _mp_audio::PlayTeamDialog__functor(lvl2, team2, sound2, delay2);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_hq.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog", ftor2);
    }
    mp_util_wad::pLevel->radioTriggerTime = 0;
    ShowHQSetUp();
}

// SwitchToRadioOnly - ea: 0x95AF10
void SwitchToRadioOnly() {
    ObjectiveDelete(0, -1);
    if ((bool)mp_util_wad::pLevel->pointA_isHQ) {
        Broc::string pszString("flag");
        Broc::string state("i_objective_c");
        ObjectiveAdd(0, state, pszString, mp_util_wad::pLevel->pointA,
                     (float)lHQObjectiveHeight, -1);
        state.~string();
        pszString.~string();
    }
    AddRadioModel();
    HashStr funcHash;
    Broc::string_hash(&funcHash, "_mp_hq::TriggerRadio");
    HashStr label;
    label.mVal = 0xF2F5EAB4;
    Broc::AddEventHandler(mp_util_wad::GetEE_trigger(
                              mp_util_wad::pLevel->_base.entity),
                          label.mVal, funcHash.mVal);
    Broc::string script("MX_HQ_HQReady");
    Broc::entity* levelEntity =
        mp_util_wad::pLevel != nullptr
            ? &mp_util_wad::pLevel->_base.entity
            : nullptr;
    Broc::EffectEventPlay(levelEntity, &script);
    script.~string();
}

// AddRadioModel - ea: 0x95B0F0
void AddRadioModel() {
    Broc::string val("hq_radio");
    HashStr key;
    key.mVal = 0x19F9F0E8u;
    Broc::entity radio;
    Broc::GetEnt(&radio, &val, key, 0);
    val.~string();
    if (!Broc::IsDefined(radio))
        return;
    if ((bool)mp_util_wad::pLevel->pointA_isHQ)
        mp_util_wad::entity_set_origin(radio, mp_util_wad::pLevel->pointA);
}

// RemoveRadioModel - ea: 0x95B1E0
void RemoveRadioModel() {
    Broc::string val("hq_radio");
    HashStr key;
    key.mVal = 0x19F9F0E8u;
    Broc::entity radio;
    Broc::GetEnt(&radio, &val, key, 0);
    val.~string();
    if (!Broc::IsDefined(radio))
        return;
    Broc::vector hidden(0.0f, 0.0f, -10000.0f);
    mp_util_wad::entity_set_origin(radio, hidden);
}

// TriggerRadio - ea: 0x9593E0
void TriggerRadio(Broc::entity self) {
    static bool initialized = false;
    static Broc::bint last_time;
    if (!initialized) {
        initialized = true;
        Broc::GetTime(&last_time);
    }

    Broc::bint this_time;
    Broc::GetTime(&this_time);
    Broc::bint time_delta((int)this_time - (int)last_time);
    last_time = this_time;
    mp_util_wad::pLevel->last_radio_trigger_time = this_time;

    if ((int)time_delta > 250 || (int)time_delta < 0) {
        time_delta = 0;
        mp_util_wad::pLevel->radioTriggerTime = 0;
        Broc::dyn_array<Broc::entity> stale_players;
        Broc::GetPlayerArray(&stale_players);
        for (int i = 0; i < Broc::size(stale_players); ++i) {
            Broc::entity player = stale_players[(unsigned int)i];
            *mp_util_wad::GetEE_setting_up_hq(player) = 0;
        }
        stale_players.~dyn_array();
    }

    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint current_time;
    for (int i = 0; i < Broc::size(players); ++i) {
        Broc::GetTime(&current_time);
        mp_util_wad::entity_set_key(
            players[(unsigned int)i], (int)current_time);
    }

    if ((int)mp_util_wad::pLevel->hq_stage == 2) {
        int num_axis_triggering = 0;
        int num_allies_triggering = 0;
        Broc::dyn_array<Broc::entity> touching_players;
        Broc::GetPlayerArray(&touching_players);

        for (int i = 0; i < Broc::size(touching_players); ++i) {
            Broc::entity player = touching_players[(unsigned int)i];
            Broc::bint player_state;
            mp_util_wad::entity_get_playerState(&player_state, player);
            if ((int)player_state != 3 ||
                !Broc::IsTouching(&player, &self) ||
                Broc::Code_IsInVehicle(player))
                continue;

            *mp_util_wad::GetEE_setting_up_hq(player) = 1;
            Broc::string team;
            mp_util_wad::entity_get_team(&team, player);
            bool is_axis = team == "axis";
            team.~string();
            if (is_axis) {
                if (num_allies_triggering > 0) {
                    *mp_util_wad::GetEE_setting_up_hq(player) = 0;
                    mp_util_wad::pLevel->radioTriggerTime = 0;
                    touching_players.~dyn_array();
                    players.~dyn_array();
                    return;
                }
                ++num_axis_triggering;
                mp_util_wad::pLevel->allies_defending = false;
            } else {
                if (num_axis_triggering > 0) {
                    *mp_util_wad::GetEE_setting_up_hq(player) = 0;
                    mp_util_wad::pLevel->radioTriggerTime = 0;
                    touching_players.~dyn_array();
                    players.~dyn_array();
                    return;
                }
                mp_util_wad::pLevel->allies_defending = true;
                ++num_allies_triggering;
            }
        }

        if (num_allies_triggering == 0 || num_axis_triggering == 0) {
            if ((int)mp_util_wad::pLevel->radioTriggerTime == 0) {
                Broc::string script("MP_HQ_Build");
                Broc::entity* level_entity =
                    mp_util_wad::pLevel != nullptr
                        ? &mp_util_wad::pLevel->_base.entity
                        : nullptr;
                Broc::EffectEventPlay(level_entity, &script);
                script.~string();
            }
            Broc::bfloat cap_speed;
            GetCapSpeed(&cap_speed,
                        Broc::bint(num_allies_triggering +
                                   num_axis_triggering));
            mp_util_wad::pLevel->radioTriggerTime =
                (int)mp_util_wad::pLevel->radioTriggerTime +
                (int)((int)time_delta * (float)cap_speed);

            for (int i = 0; i < Broc::size(touching_players); ++i) {
                Broc::entity player = touching_players[(unsigned int)i];
                Broc::bint player_state;
                mp_util_wad::entity_get_playerState(&player_state, player);
                if ((int)player_state == 3 &&
                    Broc::IsTouching(&player, &self) &&
                    !Broc::Code_IsInVehicle(player))
                    ShowSetupGraphic(player);
            }

            if ((int)mp_util_wad::pLevel->radioTriggerTime > 15000 &&
                Broc::IsLocalHost()) {
                mp_util_wad::pLevel->hq_stage = 3;
                mp_util_wad::pLevel->radioTriggerTime = 0;
                for (int i = 0; i < Broc::size(touching_players); ++i) {
                    Broc::entity player = touching_players[(unsigned int)i];
                    *mp_util_wad::GetEE_setting_up_hq(player) = 0;
                }
            }
        }
        touching_players.~dyn_array();
        players.~dyn_array();
        return;
    }

    if ((int)mp_util_wad::pLevel->hq_stage == 3) {
        int num_attackers_triggering = 0;
        int num_defenders_triggering = 0;
        int net_attackers_triggering = 0;
        Broc::dyn_array<Broc::entity> all_players;
        Broc::GetPlayerArray(&all_players);

        for (int i = 0; i < Broc::size(all_players); ++i) {
            Broc::entity player = all_players[(unsigned int)i];
            Broc::bint player_state;
            mp_util_wad::entity_get_playerState(&player_state, player);
            if ((int)player_state != 3 ||
                !Broc::IsTouching(&player, &self) ||
                Broc::Code_IsInVehicle(player))
                continue;

            Broc::string team;
            mp_util_wad::entity_get_team(&team, player);
            bool is_defender =
                (team == "allies" &&
                 (bool)mp_util_wad::pLevel->allies_defending) ||
                (team == "axis" &&
                 !(bool)mp_util_wad::pLevel->allies_defending);
            if (is_defender) {
                ++num_defenders_triggering;
            } else {
                bool is_attacker =
                    (team == "allies" &&
                     !(bool)mp_util_wad::pLevel->allies_defending) ||
                    (team == "axis" &&
                     (bool)mp_util_wad::pLevel->allies_defending);
                if (is_attacker) {
                    *mp_util_wad::GetEE_setting_up_hq(player) = 1;
                    ++num_attackers_triggering;
                }
            }
            team.~string();
        }

        net_attackers_triggering =
            num_attackers_triggering - num_defenders_triggering;
        if (net_attackers_triggering < 0)
            net_attackers_triggering = 0;
        if (net_attackers_triggering == 0 &&
            num_attackers_triggering == 0)
            mp_util_wad::pLevel->radioTriggerTime = 0;
        if ((int)mp_util_wad::pLevel->radioTriggerTime == 0 &&
            net_attackers_triggering > 0) {
            Broc::string script("MP_HQ_Build");
            Broc::entity* level_entity =
                mp_util_wad::pLevel != nullptr
                    ? &mp_util_wad::pLevel->_base.entity
                    : nullptr;
            Broc::EffectEventPlay(level_entity, &script);
            script.~string();
        }
        mp_util_wad::pLevel->radioTriggerTime =
            (int)mp_util_wad::pLevel->radioTriggerTime +
            (int)time_delta * net_attackers_triggering;

        for (int i = 0; i < Broc::size(all_players); ++i) {
            Broc::entity player = all_players[(unsigned int)i];
            Broc::bint player_state;
            mp_util_wad::entity_get_playerState(&player_state, player);
            if ((int)player_state != 3)
                continue;

            Broc::string team;
            mp_util_wad::entity_get_team(&team, player);
            bool is_defender =
                (team == "allies" &&
                 (bool)mp_util_wad::pLevel->allies_defending) ||
                (team == "axis" &&
                 !(bool)mp_util_wad::pLevel->allies_defending);
            if (is_defender) {
                if (!Broc::IsTouching(&player, &self) ||
                    Broc::Code_IsInVehicle(player)) {
                    if (net_attackers_triggering > 0)
                        ShowLosingHQGraphic(player);
                } else if (num_attackers_triggering > 0) {
                    Broc::SetActionHint(
                        (int)0x7A8A9265u, Broc::GetPlayerIndex(player));
                } else if (num_defenders_triggering > 0) {
                    Broc::SetActionHint(
                        (int)0x9E58202Bu, Broc::GetPlayerIndex(player));
                }
            } else if (Broc::IsTouching(&player, &self) &&
                       !Broc::Code_IsInVehicle(player)) {
                if (net_attackers_triggering > 0) {
                    ShowDestructionGraphic(player);
                } else if (num_defenders_triggering > 0) {
                    Broc::SetActionHint(
                        (int)0x9E58202Bu, Broc::GetPlayerIndex(player));
                }
            }
            team.~string();
        }

        if ((int)mp_util_wad::pLevel->radioTriggerTime > 15000 &&
            Broc::IsLocalHost())
            mp_util_wad::pLevel->hq_stage = 5;
        all_players.~dyn_array();
    }
    players.~dyn_array();
}

// Track_Ownership - ea: 0x95BE00
void Track_Ownership(Broc::entity self) {
    (void)self;
    Broc::Code_DebugOut("*HQ* Track_Ownership\n");
    Broc::entity lvl =
        mp_util_wad::pLevel != nullptr
            ? mp_util_wad::pLevel->_base.entity
            : Broc::entity();
    HashStr n;
    n.mVal = 0x12CEF01u;
    Broc::endon(lvl, n);
    for (;;) {
        do {
            Broc::wait(1.0f);
        } while ((int)mp_util_wad::pLevel->hq_stage != 3);
        if ((bool)mp_util_wad::pLevel->allies_defending)
            AddPoints(Broc::bint(0), Broc::bint(1));
        else
            AddPoints(Broc::bint(1), Broc::bint(0));
    }
}
}

// ============================================================================
// _mp_war - war game type (flag domination).
// ============================================================================
namespace _mp_war {

// vLerp - ea: 0x9746C0
Broc::vector* vLerp(Broc::vector* result, Broc::vector a, Broc::vector b,
                    Broc::bfloat t) {
    *result = Broc::vector(a.x + (b.x - a.x) * (float)t,
                           a.y + (b.y - a.y) * (float)t,
                           a.z + (b.z - a.z) * (float)t);
    return result;
}

// main - ea: 0x974830
void main(Broc::entity self) {
    Broc::bbool team_game(true);
    Broc::Code_SetTeamGame((bool)team_game);
    mp_util_wad::pLevel->RenderSpawnPoints = (void*)DebugRenderSpawnPoints;
    Broc::Code_SetShowScore(false);
    mp_util_wad::pLevel->spawnTypeAllies = "spawn_war_allies";
    mp_util_wad::pLevel->spawnTypeAxis = "spawn_war_axis";
    _mp_common::SetupCallbacks(team_game);
    Broc::BrocExports& x = Broc::gBrocAPI.mBrocExports;
    x.mCallbackPlayerJoin = CallbackPlayerJoin;
    x.mCallbackPlayerEnter = CallbackPlayerEnter;
    x.mCallbackPlayerKilled = CallbackPlayerKilled;
    x.mCallbackAreaCaptured = CallbackAreaCaptured;
    x.mCallbackGameState = CallbackGameState;
    x.mCallbackGameStateDOM = CallbackGameStateDOM;
    x.mCallbackNextRound = CallbackNextRound;
    x.mCallbackHostOptionsChanged = CallbackHostOptionsChanged;
    x.mCallbackRoundOver = CallbackRoundOver;
    x.mCallbackHostMigrated = (void (*)())CallbackHostMigrated;
    x.mCallbackGetFlagCount = CallbackGetFlagCount;
    x.mCallbackGetTeamControllingFlag = CallbackGetTeamControllingFlag;
    x.mCallbackGetFlagBeingCaptured = CallbackGetFlagBeingCaptured;
    x.mCallbackGetTeamCapturingFlag = CallbackGetTeamCapturingFlag;
    x.mCallbackGetCapturingFlagPercent = CallbackGetCapturingFlagPercent;
    x.mCallbackGetFlagBeingContested = CallbackGetFlagBeingContested;
    x.mCallbackGetFlagBreatherTime = CallbackGetFlagBreatherTime;
    x.mCallbackDebugRender = (void (*)())CallbackDebugRender;
    mp_util_wad::pLevel->PickSpawnPoint = (void*)GetSpawnPoint;
    Broc::SetCvar("cg_hudObjectiveRingTime", "1500");
    Broc::SetCvar("cg_hudObjectiveNumRings", "1");
    Broc::string val("war_flag");
    HashStr key;
    key.mVal = 0x19F9F0E8u;
    Broc::GetEntArray(&val, key.mVal, &mp_util_wad::pLevel->warAreas, 0);
    val.~string();
    SortMarkers();
    Broc::bint i(0);
    while ((int)i < Broc::size(mp_util_wad::pLevel->warAreas)) {
        WAR_InitFlag(mp_util_wad::pLevel->warAreas[(unsigned int)(int)i],
                     Broc::bint((int)i));
        i = (int)i + 1;
    }
    mp_util_wad::pLevel->lastFlagIndex =
        Broc::size(mp_util_wad::pLevel->warAreas) - 1;
    if ((int)mp_util_wad::pLevel->lastFlagIndex < 0 &&
        Broc::gBrocAPI.mError(
            "c:\\cod\\code\\script\\_mp_war.bro", __LINE__,
            "No War flags in the level!"))
        __debugbreak();
    mp_util_wad::pLevel->spawnType = 1;
    mp_util_wad::pLevel->lastManStanding = false;
    if (mp_util_wad::pLevel->axis == "german")
        mp_util_wad::pLevel->AxisFlagModel = "p_mp_dom_flag_hanging_gr";
    else if (mp_util_wad::pLevel->axis == "italian")
        mp_util_wad::pLevel->AxisFlagModel = "p_mp_dom_flag_hanging_it";
    else if (mp_util_wad::pLevel->axis == "vichy")
        mp_util_wad::pLevel->AxisFlagModel = "p_mp_dom_flag_hanging_fr";
    if (mp_util_wad::pLevel->allies == "american")
        mp_util_wad::pLevel->AlliesFlagModel = "p_mp_dom_flag_hanging_us";
    else
        mp_util_wad::pLevel->AlliesFlagModel = "p_mp_dom_flag_hanging_fr";
    void* started = StartGame__functor(self);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                        __LINE__, "StartGame", started);
}

// StartGame - ea: 0x974DF0
void StartGame(Broc::entity self) {
    (void)self;
    Broc::wait(0.5f);
    SetupRound();
    _mp_common::StartRound(Broc::bbool(true));
    Broc::entity lvl = mp_util_wad::pLevel != nullptr
                           ? mp_util_wad::pLevel->_base.entity
                           : Broc::entity();
    void* ftor = _mp_common::RunFrame__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                        __LINE__, "_mp_common::RunFrame", ftor);
    Broc::Code_EnterGame();
    Broc::entity lvl2 = mp_util_wad::pLevel != nullptr
                            ? mp_util_wad::pLevel->_base.entity
                            : Broc::entity();
    void* score = WARScore__functor(lvl2);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                        __LINE__, "WARScore", score);
}

// CallbackHostMigrated - ea: 0x976050
void CallbackHostMigrated() {
    _mp_common::CallbackHostMigrated();
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        SendFlagStates(players[(unsigned int)(int)i]);
        i = (int)i + 1;
    }
    players.~dyn_array();
}

// CallbackHostOptionsChanged - ea: 0x976BB0
void CallbackHostOptionsChanged(int forceMapChange) {
    _mp_common::CallbackHostOptionsChanged(forceMapChange);
    mp_util_wad::pLevel->lastManStanding = false;
}

// CallbackGameState - ea: 0x976BF0
void CallbackGameState(int currentMatchTime, int timeLimit, int scoreLimit,
                       int roundLimit, int friendlyFire, int lastManStanding,
                       int teamBalance, int respawnTime, int alliesScore,
                       int axisScore, int roundStarted, int roundOver,
                       int roundCount) {
    _mp_common::CallbackGameState(currentMatchTime, timeLimit, scoreLimit,
                                  roundLimit, friendlyFire, 0, teamBalance,
                                  respawnTime, alliesScore, axisScore,
                                  roundStarted, roundOver, roundCount);
    (void)lastManStanding;
}

// CallbackPlayerKilled - ea: 0x976C50
void CallbackPlayerKilled(Broc::entity player, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health) {
    _mp_common::CallbackPlayerKilled(player, inflictor, attacker, weapon, mod,
                                     health);
}

// CallbackNextRound - ea: 0x977520
void CallbackNextRound() {
    Broc::entity lvl = mp_util_wad::pLevel != nullptr
                           ? mp_util_wad::pLevel->_base.entity
                           : Broc::entity();
    HashStr n;
    n.mVal = 0x6FA23667u;
    Broc::notify(lvl, n);
    SetupRound();
    _mp_common::CallbackNextRound();
}

// CallbackPlayerJoin - ea: 0x978620
void CallbackPlayerJoin(Broc::entity player, unsigned int playerState,
                        int playerClass) {
    _mp_common::CallbackPlayerJoin(player, playerState, (__int16)playerClass);
}

// CallbackPlayerEnter - ea: 0x978660
void CallbackPlayerEnter(Broc::entity player, int hot_joiner) {
    _mp_common::CallbackPlayerEnter(player, hot_joiner);
    SendFlagStates(player);
    if (Broc::Code_IsLocalPlayer(player)) {
        void* ftor = WarnPlayerAboutInactiveFlag__functor(player);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                            __LINE__, "WarnPlayerAboutInactiveFlag", ftor);
    }
}

// CallbackRoundOver - ea: 0x9787B0
void CallbackRoundOver(int condition, Broc::string team) {
    if (condition == 5) {
        Broc::string otherTeam("allies");
        if (team == "allies") {
            Broc::SetTutorialTextAllPlayers((int)0x6E5C70B8u);
            otherTeam = "axis";
        } else {
            Broc::SetTutorialTextAllPlayers((int)0x8C1FDAB3);
        }
        Broc::bint scadder(Broc::Code_GetTeamScore(otherTeam));
        Broc::Code_IncTeamScore(team, (int)scadder);
        mp_util_wad::pLevel->roundWinner = team;
        otherTeam.~string();
    }
    Broc::SoundFadeOut((unsigned int)(int)mp_util_wad::pLevel->FlagMusic, 0.5f);
    _mp_common::CallbackRoundOver(condition, team);
    team.~string();
}

// CallbackGetFlagCount - ea: 0x977F10
int CallbackGetFlagCount() {
    if (!(bool)mp_util_wad::pLevel->roundStarted)
        return 0;
    if (mp_util_wad::pLevel->warAreas.mCapacity != 0)
        return Broc::size(mp_util_wad::pLevel->warAreas);
    return 0;
}

// CallbackGetFlagBreatherTime - ea: 0x977F90
int CallbackGetFlagBreatherTime() {
    if (!(bool)mp_util_wad::pLevel->roundStarted)
        return 1;
    Broc::bint now;
    Broc::GetTime(&now);
    return (int)mp_util_wad::pLevel->noCapTime > (int)now;
}

// CallbackGetTeamControllingFlag - ea: 0x978020
int CallbackGetTeamControllingFlag(unsigned int flagIndex) {
    if (!(bool)mp_util_wad::pLevel->roundStarted)
        return 0;
    if (flagIndex >= (unsigned int)Broc::size(mp_util_wad::pLevel->warAreas))
        return 0;
    Broc::entity area = mp_util_wad::pLevel->warAreas[flagIndex];
    Broc::entity& trigger = area->flagEnd.GetRef();
    int team = (int)trigger->capTeam.GetRef();
    if (team == -1)
        return -1;
    return team == 1;
}

// CallbackGetFlagBeingCaptured - ea: 0x978140
int CallbackGetFlagBeingCaptured() {
    if ((int)mp_util_wad::pLevel->warIndex < 0)
        return -1;
    int index = (int)mp_util_wad::pLevel->warIndex;
    if (index >= Broc::size(mp_util_wad::pLevel->warAreas))
        return -1;
    return index;
}

// CallbackGetTeamCapturingFlag - ea: 0x9781E0
int CallbackGetTeamCapturingFlag() {
    if ((int)mp_util_wad::pLevel->warIndex < 0)
        return 0;
    int index = (int)mp_util_wad::pLevel->warIndex;
    if (index >= Broc::size(mp_util_wad::pLevel->warAreas))
        return 0;
    Broc::entity area = mp_util_wad::pLevel->warAreas[(unsigned int)index];
    Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
    float status = (float)*mp_util_wad::GetEE_capStatus(trigger);
    if (status > 0.0f)
        return 1;
    if (status < 0.0f)
        return -1;
    return 0;
}

// CallbackGetCapturingFlagPercent - ea: 0x978360
int CallbackGetCapturingFlagPercent() {
    if ((int)mp_util_wad::pLevel->warIndex < 0)
        return 0;
    int index = (int)mp_util_wad::pLevel->warIndex;
    if (index >= Broc::size(mp_util_wad::pLevel->warAreas))
        return 0;
    Broc::entity area = mp_util_wad::pLevel->warAreas[(unsigned int)index];
    Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
    return (int)((float)*mp_util_wad::GetEE_capStatus(trigger) * 100.0f);
}

// CallbackGetFlagBeingContested - ea: 0x978470
int CallbackGetFlagBeingContested(Broc::entity player) {
    if (mp_util_wad::pLevel->warAreas.mCapacity == 0)
        return 0;
    if ((int)mp_util_wad::pLevel->warIndex < 0)
        return 0;
    int index = (int)mp_util_wad::pLevel->warIndex;
    if (index >= Broc::size(mp_util_wad::pLevel->warAreas))
        return 0;

    Broc::bint playerState;
    mp_util_wad::entity_get_playerState(&playerState, player);
    if ((int)playerState != 3)
        return 0;

    Broc::entity area =
        mp_util_wad::pLevel->warAreas[(unsigned int)index];
    Broc::entity& trigger = area->flagEnd.GetRef();
    if (!Broc::IsTouching(&trigger, &player))
        return 0;
    if (Broc::Code_IsInVehicle(player))
        return 0;
    return GetNumPlayersContestingFlag();
}

// GetNumPlayersContestingFlag - ea: 0x9785F0
int GetNumPlayersContestingFlag() {
    return (int)mp_util_wad::pLevel->capCount;
}

// WARScore - ea: 0x975980
void WARScore(Broc::entity self) {
    (void)self;
    Broc::bint winningTeam(0);
    Broc::bint points(0);
    Broc::bint pointsTimer(0);
    for (;;) {
        while (!(bool)mp_util_wad::pLevel->roundStarted ||
               (bool)mp_util_wad::pLevel->roundOver ||
               (int)mp_util_wad::pLevel->warIndex ==
                   (int)mp_util_wad::pLevel->lastFlagIndex / 2) {
            pointsTimer = -1;
            Broc::wait(1.0f);
        }
        if ((int)mp_util_wad::pLevel->warIndex <
            (int)mp_util_wad::pLevel->lastFlagIndex / 2)
            winningTeam = -1;
        else
            winningTeam = 1;
        if ((int)pointsTimer >= 0)
            break;
        pointsTimer = 10;
        Broc::wait(1.0f);
    }
    for (;;) {
        pointsTimer -= 1;
        if ((int)pointsTimer <= 0) {
            pointsTimer = 10;
            points = (int)mp_util_wad::pLevel->warIndex -
                     (int)mp_util_wad::pLevel->lastFlagIndex / 2;
            if ((int)points < 0)
                points = -points;
            if ((int)points > 2)
                points = 2;
            if ((int)winningTeam == -1) {
                Broc::string team("axis");
                Broc::string team2("allies");
                Broc::bint axisScore(
                    Broc::Code_GetTeamScore(team) + (int)points);
                int alliesScore = Broc::Code_GetTeamScore(team2);
                Broc::Code_SendGameScore(alliesScore, (int)axisScore);
                team.~string();
                team2.~string();
            } else {
                Broc::string team("allies");
                Broc::string team2("axis");
                int axisScore = Broc::Code_GetTeamScore(team2);
                Broc::bint alliesScore(
                    Broc::Code_GetTeamScore(team) + (int)points);
                Broc::Code_SendGameScore((int)alliesScore, axisScore);
                team.~string();
                team2.~string();
            }
        }
        Broc::wait(1.0f);
    }
}

// GiveTeamMembersPoints - ea: 0x975DA0
void GiveTeamMembersPoints(Broc::vector origin, Broc::bfloat radius,
                           Broc::string team) {
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, p);
        if ((int)state == 3) {
            Broc::vector ppos;
            mp_util_wad::entity_get_origin(&ppos, p);
            Broc::bfloat dist(Broc::Distance(&ppos, &origin));
            if ((float)dist < (float)radius) {
                Broc::string pteam;
                mp_util_wad::entity_get_team(&pteam, p);
                if (pteam == team)
                    _mp_common::AddToPlayerStats(p, Broc::bint(20), 2);
                pteam.~string();
            }
        }
        i = (int)i + 1;
    }
    players.~dyn_array();
    team.~string();
}

// CallbackGameStateDOM - ea: 0x976CB0
void CallbackGameStateDOM(int flag0, int flag1, int flag2, int flag3,
                          int flag4) {
    Broc::entity area0 = mp_util_wad::pLevel->warAreas[0];
    Broc::entity trigger0 = *mp_util_wad::GetEE_trigger(area0);
    *mp_util_wad::GetEE_capStatus(trigger0) =
        (float)flag0 / 32767.0f;
    Broc::entity area1 = mp_util_wad::pLevel->warAreas[1];
    Broc::entity trigger1 = *mp_util_wad::GetEE_trigger(area1);
    *mp_util_wad::GetEE_capStatus(trigger1) =
        (float)flag1 / 32767.0f;
    Broc::entity area2 = mp_util_wad::pLevel->warAreas[2];
    Broc::entity trigger2 = *mp_util_wad::GetEE_trigger(area2);
    *mp_util_wad::GetEE_capStatus(trigger2) =
        (float)flag2 / 32767.0f;
    if (Broc::size(mp_util_wad::pLevel->warAreas) > 3) {
        Broc::entity area3 = mp_util_wad::pLevel->warAreas[3];
        Broc::entity trigger3 = *mp_util_wad::GetEE_trigger(area3);
        *mp_util_wad::GetEE_capStatus(trigger3) =
            (float)flag3 / 32767.0f;
        Broc::entity area4 = mp_util_wad::pLevel->warAreas[4];
        Broc::entity trigger4 = *mp_util_wad::GetEE_trigger(area4);
        *mp_util_wad::GetEE_capStatus(trigger4) =
            (float)flag4 / 32767.0f;
    }

    Broc::bbool currentHeightOwnsFlag[5] = {true, true, true, true, true};
    currentHeightOwnsFlag[0] = (flag0 & 1) != 0;
    currentHeightOwnsFlag[1] = (flag1 & 1) != 0;
    currentHeightOwnsFlag[2] = (flag2 & 1) != 0;
    currentHeightOwnsFlag[3] = (flag3 & 1) != 0;
    currentHeightOwnsFlag[4] = (flag4 & 1) != 0;

    mp_util_wad::pLevel->warIndex = -1;
    Broc::bint i(0);
    while ((int)i < Broc::size(mp_util_wad::pLevel->warAreas)) {
        Broc::entity area = mp_util_wad::pLevel->warAreas[(unsigned int)(int)i];
        Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
        int ownsFlag = (bool)currentHeightOwnsFlag[(unsigned int)(int)i] ? 1 : 0;
        *mp_util_wad::GetEE_capTeam(trigger) = ownsFlag;

        if ((int)*mp_util_wad::GetEE_capTeam(trigger) == 0)
            mp_util_wad::pLevel->warIndex = i;
        if ((float)*mp_util_wad::GetEE_capStatus(trigger) < 0.0f)
            *mp_util_wad::GetEE_capTeam(trigger) =
                -(int)*mp_util_wad::GetEE_capTeam(trigger);

        int capTeam = (int)*mp_util_wad::GetEE_capTeam(trigger);
        if (capTeam == 1) {
            Broc::entity flag = *mp_util_wad::GetEE_flag(trigger);
            Broc::SetModel(&flag, &mp_util_wad::pLevel->AlliesFlagModel,
                           INVALID_PAK_INFO);
        } else if (capTeam == -1) {
            Broc::entity flag = *mp_util_wad::GetEE_flag(trigger);
            Broc::SetModel(&flag, &mp_util_wad::pLevel->AxisFlagModel,
                           INVALID_PAK_INFO);
        }

        float capStatus = (float)*mp_util_wad::GetEE_capStatus(trigger);
        if (capStatus < 0.99989998f) {
            if (capStatus <= -0.99989998f)
                *mp_util_wad::GetEE_capStatus(trigger) = -1.0f;
        } else {
            *mp_util_wad::GetEE_capStatus(trigger) = 1.0f;
        }
        i = (int)i + 1;
    }
    UpdateAllowedCap();
}

// SendFlagStates - ea: 0x978960
void SendFlagStates(Broc::entity player) {
    Broc::bbool currentHeightOwnsFlag[5] = {true, true, true, true, true};
    Broc::bint i(0);
    while ((int)i < 5) {
        bool ownsFlag = false;
        if ((int)i < Broc::size(mp_util_wad::pLevel->warAreas)) {
            Broc::entity area =
                mp_util_wad::pLevel->warAreas[(unsigned int)(int)i];
            Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
            ownsFlag = *mp_util_wad::GetEE_capTeam(trigger) != 0;
        }
        currentHeightOwnsFlag[(unsigned int)(int)i] = ownsFlag;
        i = (int)i + 1;
    }

    int currentHeight[5] = {0, 0, 0, 0, 0};
    Broc::bint lhs(0);
    while ((int)lhs < 5) {
        if ((int)lhs < Broc::size(mp_util_wad::pLevel->warAreas)) {
            Broc::entity area =
                mp_util_wad::pLevel->warAreas[(unsigned int)(int)lhs];
            Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
            Broc::bfloat status = *mp_util_wad::GetEE_capStatus(trigger);
            Broc::bfloat scaled((float)status * 32767.0f);
            currentHeight[(int)lhs] = (int)(float)scaled;
            if ((bool)currentHeightOwnsFlag[(unsigned int)(int)lhs])
                currentHeight[(int)lhs] |= 1;
            else
                currentHeight[(int)lhs] &= ~1;
        }
        lhs = (int)lhs + 1;
    }

    Broc::Code_SendGameStateDOM(player, currentHeight[0], currentHeight[1],
                                currentHeight[2], currentHeight[3],
                                currentHeight[4]);
}

// WAR_Init - ea: 0x978D10
void WAR_Init(Broc::entity self) {
    (void)self;
    Broc::entity level = mp_util_wad::pLevel != nullptr
                             ? mp_util_wad::pLevel->_base.entity
                             : Broc::entity();
    HashStr endLabel;
    endLabel.mVal = 0x6FA23667u;
    Broc::endon(level, endLabel);
    mp_util_wad::pLevel->blinker = 0;
    while (!(bool)mp_util_wad::pLevel->roundOver) {
        Broc::wait(1.0f);
        mp_util_wad::pLevel->blinker =
            (int)mp_util_wad::pLevel->blinker == 0 ? 1 : 0;

        Broc::bint alliesCapped(0);
        Broc::bint axisCapped(0);
        Broc::bint i(0);
        while ((int)i < Broc::size(mp_util_wad::pLevel->warAreas)) {
            Broc::entity area = mp_util_wad::pLevel->warAreas[
                (unsigned int)(int)i];
            Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
            float capStatus = (float)*mp_util_wad::GetEE_capStatus(trigger);
            if (capStatus >= 0.9999f)
                alliesCapped = (int)alliesCapped + 1;
            if (capStatus <= -0.9999f)
                axisCapped = (int)axisCapped + 1;
            i = (int)i + 1;
        }

        mp_util_wad::pLevel->axis_capped = axisCapped;
        mp_util_wad::pLevel->allies_capped = alliesCapped;
        int areaCount = Broc::size(mp_util_wad::pLevel->warAreas);
        if ((int)mp_util_wad::pLevel->axis_capped == areaCount) {
            _mp_common::EndRound(kEndRoundDomAllFlagsCapped,
                                 Broc::string("axis"));
        } else if ((int)mp_util_wad::pLevel->allies_capped == areaCount) {
            _mp_common::EndRound(kEndRoundDomAllFlagsCapped,
                                 Broc::string("allies"));
        }
    }
}

// WAR_FlagUpdate - ea: 0x9790C0
void WAR_FlagUpdate(Broc::entity self) {
    *mp_util_wad::GetEE_objective_status(self) = 0;

    Broc::entity flag = *mp_util_wad::GetEE_flag(self);
    Broc::entity::__unnamed::origin_struct flagOriginField = {
        flag.GetHandle()};
    Broc::vector flagOrigin;
    flagOriginField.Get(&flagOrigin);

    Broc::bint lastBlinker(0);
    Broc::bfloat lastCapStatus(0.0f);
    Broc::bint lastTime;
    Broc::GetTime(&lastTime);
    Broc::bfloat delta(0.0f);
    for (;;) {
        Broc::entity::__unnamed::angles_struct anglesField = {
            self.GetHandle()};
        Broc::vector angles;
        anglesField.Get(&angles);
        (void)::AnglesToForward(angles);
        Broc::vector moveTo = flagOrigin;

        float capStatus = (float)*mp_util_wad::GetEE_capStatus(self);
        if (capStatus < 1.0f) {
            if (capStatus <= -1.0f &&
                (int)*mp_util_wad::GetEE_capTeam(self) != -1) {
                Broc::Code_AreaCaptured(
                    (int)*mp_util_wad::GetEE_index(self), 0, 1);
            }
        } else if ((int)*mp_util_wad::GetEE_capTeam(self) != 1) {
            Broc::Code_AreaCaptured(
                (int)*mp_util_wad::GetEE_index(self), 1, 1);
        }

        float speed = Broc::GetCvarInt("mp_debug") ? 0.2f : 0.05f;
        Broc::bint now;
        Broc::GetTime(&now);
        delta = (float)((int)now - (int)lastTime) * 0.001f;
        Broc::GetTime(&lastTime);

        bool atBottom = capStatus >= -0.001f && capStatus <= 0.001f;
        float capSpeed = (float)*mp_util_wad::GetEE_capSpeed(self);
        capStatus += capSpeed * speed * (float)delta;
        *mp_util_wad::GetEE_capStatus(self) = capStatus;
        capStatus = (float)*mp_util_wad::GetEE_capStatus(self);

        if (atBottom && (capStatus < -0.001f || capStatus > 0.001f)) {
            Broc::string myTeam("axis");
            Broc::string otherTeam("allies");
            if (capStatus > 0.0f) {
                myTeam = "allies";
                otherTeam = "axis";
            }

            if ((int)mp_util_wad::pLevel->warIndex ==
                    (int)mp_util_wad::pLevel->lastFlagIndex &&
                capStatus > 0.0f) {
                Broc::string battleSong("MX_War_LastBattlesong");
                mp_util_wad::pLevel->FlagMusic =
                    (int)Broc::SoundPlay(battleSong, 1.0f);
                Broc::entity level = mp_util_wad::pLevel != nullptr
                                          ? mp_util_wad::pLevel->_base.entity
                                          : Broc::entity();
                void* ftor = _mp_audio::PlayTeamDialog__functor(
                    level, myTeam,
                    Broc::string("MP_WAR_FinalPosAtk_Team_Allies"),
                    Broc::string("MP_WAR_FinalPosDef_Team_Axis"),
                    Broc::bfloat(1.0f));
                Broc::thread_create(
                    false, "c:\\cod\\code\\script\\_mp_war.bro", __LINE__,
                    "_mp_audio::PlayTeamDialog", ftor);
            } else if ((int)mp_util_wad::pLevel->warIndex == 0 &&
                       capStatus < 0.0f) {
                Broc::string battleSong("MX_War_LastBattlesong");
                mp_util_wad::pLevel->FlagMusic =
                    (int)Broc::SoundPlay(battleSong, 1.0f);
                Broc::entity level = mp_util_wad::pLevel != nullptr
                                          ? mp_util_wad::pLevel->_base.entity
                                          : Broc::entity();
                void* ftor = _mp_audio::PlayTeamDialog__functor(
                    level, myTeam,
                    Broc::string("MP_WAR_FinalPosAtk_Team_Axis"),
                    Broc::string("MP_WAR_FinalPosDef_Team_Allies"),
                    Broc::bfloat(1.0f));
                Broc::thread_create(
                    false, "c:\\cod\\code\\script\\_mp_war.bro", __LINE__,
                    "_mp_audio::PlayTeamDialog", ftor);
            } else {
                Broc::SoundFadeOut(
                    (unsigned int)(int)mp_util_wad::pLevel->FlagMusic, 0.5f);
                Broc::entity level = mp_util_wad::pLevel != nullptr
                                          ? mp_util_wad::pLevel->_base.entity
                                          : Broc::entity();
                void* ftor = _mp_audio::PlayTeamSound__functor(
                    level, myTeam, Broc::string("MX_War_Enemy_Take_Flag"),
                    Broc::string("MX_War_Ally_Take_Flag"));
                Broc::thread_create(
                    false, "c:\\cod\\code\\script\\_mp_war.bro", __LINE__,
                    "_mp_audio::PlayTeamSound", ftor);
            }
        }

        if ((int)*mp_util_wad::GetEE_capTeam(self) != 0) {
            if (capStatus >= 0.0f &&
                (int)*mp_util_wad::GetEE_capTeam(self) < 0) {
                Broc::Code_AreaCaptured(
                    (int)*mp_util_wad::GetEE_index(self), 2, 1);
            } else if (capStatus <= 0.0f &&
                       (int)*mp_util_wad::GetEE_capTeam(self) > 0) {
                Broc::Code_AreaCaptured(
                    (int)*mp_util_wad::GetEE_index(self), 2, 1);
            }
        } else if (capStatus > 0.001f) {
            Broc::entity flagEntity = *mp_util_wad::GetEE_flag(self);
            Broc::SetModel(&flagEntity, &mp_util_wad::pLevel->AlliesFlagModel,
                           0);
        } else if (capStatus < -0.001f) {
            Broc::entity flagEntity = *mp_util_wad::GetEE_flag(self);
            Broc::SetModel(&flagEntity, &mp_util_wad::pLevel->AxisFlagModel,
                           0);
        }

        Broc::bint nowForTouch;
        Broc::GetTime(&nowForTouch);
        bool movingOnItsOwn =
            (int)nowForTouch > (int)*mp_util_wad::GetEE_lastTouch(self) +
                                   10000 &&
            (int)*mp_util_wad::GetEE_capTeam(self) == 0;

        Broc::bbool hasFlag;
        mp_util_wad::IsEEDefined_flag(&hasFlag, self);
        if ((bool)hasFlag &&
            (int)*mp_util_wad::GetEE_index(self) ==
                (int)mp_util_wad::pLevel->warIndex) {
            if (movingOnItsOwn && capStatus > (float)lastCapStatus) {
                if ((float)*mp_util_wad::GetEE_capStatus(self) !=
                    (float)lastCapStatus) {
                    ObjectiveDelete((int)mp_util_wad::pLevel->warIndex, -1);
                    Broc::string pszString("flag");
                    Broc::string state = mp_util_wad::pLevel->blinker == 1
                                             ? Broc::string("i_flag_allied_c")
                                             : Broc::string("i_objective_c");
                    Broc::vector objectivePosition;
                    mp_util_wad::entity_get_origin(
                        &objectivePosition,
                        mp_util_wad::pLevel->warAreas[
                            (unsigned int)(int)mp_util_wad::pLevel->warIndex]);
                    ObjectiveAdd((int)mp_util_wad::pLevel->warIndex, state,
                                 pszString, objectivePosition,
                                 (float)lWARObjectiveHeight, -1);
                }
            } else if (!movingOnItsOwn && capStatus < (float)lastCapStatus) {
                if ((float)*mp_util_wad::GetEE_capStatus(self) !=
                    (float)lastCapStatus) {
                    ObjectiveDelete((int)mp_util_wad::pLevel->warIndex, -1);
                    Broc::string pszString("flag");
                    Broc::string state = mp_util_wad::pLevel->blinker == 1
                                             ? Broc::string("i_flag_axis_c")
                                             : Broc::string("i_objective_c");
                    Broc::vector objectivePosition;
                    mp_util_wad::entity_get_origin(
                        &objectivePosition,
                        mp_util_wad::pLevel->warAreas[
                            (unsigned int)(int)mp_util_wad::pLevel->warIndex]);
                    ObjectiveAdd((int)mp_util_wad::pLevel->warIndex, state,
                                 pszString, objectivePosition,
                                 (float)lWARObjectiveHeight, -1);
                }
            } else if ((int)mp_util_wad::pLevel->blinker !=
                       (int)lastBlinker) {
                ObjectiveDelete((int)mp_util_wad::pLevel->warIndex, -1);
                Broc::string pszString("flag");
                Broc::string state("i_objective_c");
                Broc::vector objectivePosition;
                mp_util_wad::entity_get_origin(
                    &objectivePosition,
                    mp_util_wad::pLevel->warAreas[
                        (unsigned int)(int)mp_util_wad::pLevel->warIndex]);
                ObjectiveAdd((int)mp_util_wad::pLevel->warIndex, state,
                             pszString, objectivePosition,
                             (float)lWARObjectiveHeight, -1);
            }
        }

        lastBlinker = (int)mp_util_wad::pLevel->blinker;
        lastCapStatus = (float)*mp_util_wad::GetEE_capStatus(self);

        capSpeed = (float)*mp_util_wad::GetEE_capSpeed(self);
        if (capSpeed < 0.0001f && capSpeed > -0.0001f) {
            Broc::bbool hasSound;
            mp_util_wad::IsEEDefined_sound(&hasSound, self);
            if ((bool)hasSound) {
                Broc::EffectEventStopEmitting(
                    (unsigned int)(int)*mp_util_wad::GetEE_sound(self));
                *mp_util_wad::GetEE_sound(self) = 0;
            }
        } else {
            Broc::bbool hasSound;
            mp_util_wad::IsEEDefined_sound(&hasSound, self);
            if (!(bool)hasSound) {
                Broc::string script("MP_FlagCap_loop");
                HashStr notifyHash;
                notifyHash.mVal = 0x01673D97u;
                Broc::entity flagEntity = *mp_util_wad::GetEE_flag(self);
                *mp_util_wad::GetEE_sound(self) = Broc::EffectEventPlay(
                    &flagEntity, &script, notifyHash, true);
            }
        }
        *mp_util_wad::GetEE_capSpeed(self) = 0.0f;

        capStatus = (float)*mp_util_wad::GetEE_capStatus(self);
        if (capStatus < 0.0f) {
            Broc::vector start = *mp_util_wad::GetEE_flagStart(self);
            Broc::vector end = *mp_util_wad::GetEE_flagEnd(self);
            moveTo = vLerp(&moveTo, start, end, Broc::bfloat(-capStatus))[0];
        } else {
            Broc::vector start = *mp_util_wad::GetEE_flagStart(self);
            Broc::vector end = *mp_util_wad::GetEE_flagEnd(self);
            moveTo = vLerp(&moveTo, start, end, Broc::bfloat(capStatus))[0];
        }
        Broc::entity flagEntity = *mp_util_wad::GetEE_flag(self);
        Broc::MoveTo(&flagEntity, &moveTo, 0.05f, 0.05f, 0.0f);

        if (movingOnItsOwn) {
            if (capStatus > 0.01f) {
                *mp_util_wad::GetEE_capSpeed(self) = -1.3333334f;
            } else if (capStatus < -0.01f) {
                *mp_util_wad::GetEE_capSpeed(self) = -0.01f;
            } else if (atBottom) {
                Broc::SoundFadeOut(
                    (unsigned int)(int)mp_util_wad::pLevel->FlagMusic, 0.5f);
                _mp_audio::StopTeamSound();
                *mp_util_wad::GetEE_capSpeed(self) = 0.0f;
                *mp_util_wad::GetEE_capStatus(self) = 0.0f;
            }
        }
        _mp_common::waitframe();
    }
}

// WAR_TouchFlag - ea: 0x97B000
void WAR_TouchFlag(Broc::entity self) {
    Broc::dyn_array<Broc::entity> players;
    if (!(bool)mp_util_wad::pLevel->roundStarted)
        return;

    const Broc::bint* index = mp_util_wad::GetEE_index(self);
    Broc::bint now;
    if ((int)mp_util_wad::pLevel->warIndex != (int)*index)
        return;
    Broc::GetTime(&now);
    if ((int)mp_util_wad::pLevel->noCapTime > (int)now)
        return;

    Broc::GetPlayerArray(&players);
    Broc::bint axis_capping(0);
    Broc::bint allies_capping(0);
    *mp_util_wad::GetEE_capSpeed(self) = 0;

    for (Broc::bint i(0); (int)i < Broc::size(players); i = (int)i + 1) {
        Broc::entity player = players[(unsigned int)(int)i];
        Broc::bint playerState;
        mp_util_wad::entity_get_playerState(&playerState, player);
        if ((int)playerState != 3 || !Broc::IsTouching(&player, &self) ||
            Broc::Code_IsInVehicle(player))
            continue;

        Broc::GetTime(&now);
        *mp_util_wad::GetEE_lastTouch(self) = now;
        Broc::string team;
        mp_util_wad::entity_get_team(&team, player);
        if (team == "axis")
            axis_capping = (int)axis_capping + 1;
        else
            allies_capping = (int)allies_capping + 1;
    }

    if ((int)allies_capping != 0 && (int)axis_capping != 0) {
        mp_util_wad::pLevel->capCount = 0;
        return;
    }

    if ((int)allies_capping != 0) {
        mp_util_wad::pLevel->capCount = allies_capping;
        if ((float)*mp_util_wad::GetEE_capStatus(self) < 1.0f) {
            ::bfloat capSpeed(0.0f);
            ::bint count((int)allies_capping);
            GetCapSpeed(&capSpeed, count);
            *mp_util_wad::GetEE_capSpeed(self) = (float)capSpeed;
        }
    } else if ((int)axis_capping != 0) {
        mp_util_wad::pLevel->capCount = axis_capping;
        if ((float)*mp_util_wad::GetEE_capStatus(self) > -1.0f) {
            ::bfloat capSpeed(0.0f);
            ::bint count((int)axis_capping);
            GetCapSpeed(&capSpeed, count);
            *mp_util_wad::GetEE_capSpeed(self) = -(float)capSpeed;
        }
    }
}

// GetCapSpeed - ea: 0x97B580
::bfloat* GetCapSpeed(::bfloat* result, ::bint guysCapping) {
    ::bfloat value(1.0f);
    ::bfloat total(0.0f);
    ::bint i(0);
    while ((int)i < (int)guysCapping) {
        total += (float)value;
        value = (float)value * 0.75f;
        i = (int)i + 1;
    }
    *result = (float)total;
    return result;
}

// UpdateAllowedCap - ea: 0x97B640
void UpdateAllowedCap() {
    ::bint i(0);
    while ((int)i < Broc::size(mp_util_wad::pLevel->warAreas)) {
        if (i == (int)mp_util_wad::pLevel->warIndex) {
            int flagId = (int)i;
            ::bint height(0);
            AllowCap(height, flagId);
            Broc::string pszString("flag");
            Broc::string state("i_objective_c");
            Broc::entity area =
                mp_util_wad::pLevel->warAreas[(unsigned int)flagId];
            Broc::entity::__unnamed::origin_struct originField = {
                area.GetHandle()};
            Broc::vector origin;
            originField.Get(&origin);
            ObjectiveAdd(flagId, state, pszString, origin,
                         (float)lWARObjectiveHeight, -1);
        } else {
            int flagId = (int)i;
            ::bint height(-999);
            AllowCap(height, flagId);
            ObjectiveDelete(flagId, -1);
        }
        ++i;
    }
}

// AllowCap - ea: 0x97B850
void AllowCap(::bint team, int flag_id) {
    int rhs = (int)team;
    Broc::entity& area =
        mp_util_wad::pLevel->warAreas[(unsigned int)flag_id];
    Broc::entity& trigger = area->flagEnd.GetRef();
    trigger->capAllowedTeam = rhs;
    int rhs2 = (int)team;
    area->capAllowedTeam = rhs2;
    if (team == -999) {
        ObjectiveDelete(flag_id, -1);
    } else {
        Broc::string modelName("p_mp_dom_flag_hanging_nt");
        Broc::entity& flag = trigger->flag.GetRef();
        Broc::SetModel(&flag, &modelName, INVALID_PAK_INFO);
        Broc::Show(&flag);
        trigger->capStatus = 0.0f;
        trigger->capTeam = 0;
    }
}

// WAR_InitFlag - ea: 0x97BB60
void WAR_InitFlag(Broc::entity self, int flag_id) {
    Broc::entity::__unnamed::target_struct targetField = {
        self.GetHandle()};
    Broc::string target;
    targetField.Get(&target);
    if (!Broc::IsDefined(target))
        return;

    Broc::dyn_array<Broc::entity> stuff;
    Broc::entity trigger;
    Broc::string targetValue;
    targetField.Get(&targetValue);
    HashStr targetKey;
    targetKey.mVal = 0x19F9F0E8u;
    Broc::GetEnt(&trigger, &targetValue, targetKey, 0);
    *mp_util_wad::GetEE_trigger(self) = trigger;
    targetValue.~string();

    Broc::bbool triggerDefined;
    if (!(bool)*mp_util_wad::IsEEDefined_trigger(&triggerDefined, self)) {
        stuff.~dyn_array();
        target.~string();
        return;
    }

    Broc::entity linked = *mp_util_wad::GetEE_trigger(self);
    *mp_util_wad::GetEE_index(linked) = flag_id;

    Broc::string classname("script_model");
    Broc::entity::__unnamed::origin_struct originField = {
        self.GetHandle()};
    Broc::vector origin;
    originField.Get(&origin);
    Broc::entity flag;
    Broc::Spawn(&flag, &classname, &origin,
                static_cast<TPakInfo>(INVALID_PAK_INFO));
    *mp_util_wad::GetEE_flag(linked) = flag;
    classname.~string();

    Broc::vector angles;
    Broc::entity::__unnamed::angles_struct anglesField = {
        self.GetHandle()};
    anglesField.Get(&angles);
    Broc::entity::__unnamed::angles_struct flagAngles = {flag.GetHandle()};
    flagAngles = &angles;

    Broc::string modelName("p_mp_dom_flag_hanging_nt");
    Broc::SetModel(&flag, &modelName, INVALID_PAK_INFO);
    modelName.~string();

    Broc::string targetname;
    Broc::entity::__unnamed::targetname_struct targetnameField = {
        linked.GetHandle()};
    targetnameField.Get(&targetname);
    Broc::GetEntArray(&targetname, 0x15B1F8A7u, &stuff, 0);
    targetname.~string();

    Broc::vector offsetStart(0.0f, 0.0f, 40.0f);
    originField.Get(&origin);
    *mp_util_wad::GetEE_flagStart(linked) = origin + offsetStart;
    Broc::vector offsetEnd(0.0f, 0.0f, 120.0f);
    originField.Get(&origin);
    *mp_util_wad::GetEE_flagEnd(linked) = origin + offsetEnd;

    *mp_util_wad::GetEE_capSpeed(linked) = flag_id;
    Broc::bint now;
    Broc::GetTime(&now);
    *mp_util_wad::GetEE_lastTouch(self) = now;
    *mp_util_wad::GetEE_capStatus(self) = 0.0f;
    *mp_util_wad::GetEE_capTeam(self) = 0;
    *mp_util_wad::GetEE_capAllowedTeam(self) = -999;
    *mp_util_wad::GetEE_capStatus(linked) = 0.0f;
    *mp_util_wad::GetEE_capTeam(linked) = 0;
    *mp_util_wad::GetEE_capAllowedTeam(linked) = -999;

    Broc::string script("FLAG_FLAPPING");
    Broc::EffectEventPlay(&self, &script);
    script.~string();
    Broc::wait(0.01f);

    void* updated = WAR_FlagUpdate__functor(linked);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                        __LINE__, "WAR_FlagUpdate",
                        reinterpret_cast<AeThreadFunctor*>(updated));

    HashStr touchLabel;
    touchLabel.mVal = 0xF2F5EAB4u;
    HashStr touchFunction;
    Broc::string_hash(&touchFunction, "_mp_war::WAR_TouchFlag");
    Broc::AddEventHandler(linked, touchLabel, touchFunction);
    stuff.~dyn_array();
    target.~string();
}

// SortMarkers - ea: 0x97C410
void SortMarkers() {
    int result = Broc::size(mp_util_wad::pLevel->warAreas);
    if (result >= 2) {
        Broc::entity temp;
        Broc::bint i(0);
        while ((int)i < Broc::size(mp_util_wad::pLevel->warAreas)) {
            for (Broc::bint j((int)i); (int)j <
                                      Broc::size(mp_util_wad::pLevel->warAreas);
                 j = (int)j + 1) {
                Broc::entity& a =
                    mp_util_wad::pLevel->warAreas[(unsigned int)(int)i];
                Broc::entity& b =
                    mp_util_wad::pLevel->warAreas[(unsigned int)(int)j];
                Broc::entity::__unnamed::key_struct aKey = {
                    a.GetHandle()};
                Broc::bint aValue;
                aKey.Get(&aValue);
                Broc::entity::__unnamed::key_struct bKey = {
                    b.GetHandle()};
                Broc::bint bValue;
                bKey.Get(&bValue);
                if ((int)aValue > (int)bValue) {
                    temp = a;
                    a = b;
                    b = temp;
                }
            }
            i = (int)i + 1;
        }
    }
}

// GetSpawnPoint - ea: 0x97C650
Broc::entity* GetSpawnPoint(Broc::entity* result, Broc::entity* self,
                            const Broc::string* self_team) {
    Broc::dyn_array<Broc::entity> spawnpoints;
    Broc::dyn_array<Broc::entity> temp_spawnpoints;
    Broc::entity spawnpoint;
    Broc::string spawnType = mp_util_wad::pLevel->spawnTypeAllies;
    Broc::bint team(1);
    Broc::string selfTeam;
    Broc::entity::__unnamed::team_struct selfTeamField = {
        self->GetHandle()};
    selfTeamField.Get(&selfTeam);
    if (selfTeam == "axis") {
        spawnType = mp_util_wad::pLevel->spawnTypeAxis;
        team = -1;
    }

    if ((int)mp_util_wad::pLevel->warIndex < 0 ||
        (int)mp_util_wad::pLevel->warIndex >
            (int)mp_util_wad::pLevel->lastFlagIndex)
        return _mp_common::GetSpawnPoint(result, self, self_team);

    Broc::entity area = mp_util_wad::pLevel->warAreas[
        (unsigned int)(int)mp_util_wad::pLevel->warIndex];
    Broc::entity& trigger = area->flagEnd.GetRef();
    Broc::string targetname;
    Broc::entity::__unnamed::targetname_struct targetnameField = {
        trigger.GetHandle()};
    targetnameField.Get(&targetname);
    Broc::GetEntArray(&targetname, 0x15B1F8A7u, &temp_spawnpoints, 0);

    for (Broc::bint i(0); (int)i < Broc::size(temp_spawnpoints);
         i = (int)i + 1) {
        Broc::entity candidate =
            temp_spawnpoints[(unsigned int)(int)i];
        Broc::entity::__unnamed::classname_struct classnameField = {
            candidate.GetHandle()};
        Broc::string classname;
        classnameField.Get(&classname);
        if (classname == spawnType)
            Broc::push(spawnpoints, candidate);
    }

    if (Broc::size(spawnpoints) == 0) {
        if ((int)team == 1) {
            for (Broc::bint scan((int)mp_util_wad::pLevel->warIndex);
                 (int)scan >= 0 && Broc::size(spawnpoints) == 0;
                 scan = (int)scan - 1) {
                Broc::entity candidateArea = mp_util_wad::pLevel->warAreas[
                    (unsigned int)(int)scan];
                Broc::entity candidateTrigger =
                    *mp_util_wad::GetEE_trigger(candidateArea);
                if ((int)*mp_util_wad::GetEE_capTeam(candidateTrigger) !=
                    (int)team)
                    continue;
                Broc::string candidateTarget;
                Broc::entity::__unnamed::targetname_struct candidateTargetField = {
                    candidateTrigger.GetHandle()};
                candidateTargetField.Get(&candidateTarget);
                Broc::GetEntArray(&candidateTarget, 0x15B1F8A7u,
                                  &temp_spawnpoints, 0);
                for (Broc::bint j(0); (int)j < Broc::size(temp_spawnpoints);
                     j = (int)j + 1) {
                    Broc::entity candidate =
                        temp_spawnpoints[(unsigned int)(int)j];
                    Broc::entity::__unnamed::classname_struct classnameField = {
                        candidate.GetHandle()};
                    Broc::string classname;
                    classnameField.Get(&classname);
                    if (classname == spawnType)
                        Broc::push(spawnpoints, candidate);
                }
            }
        } else {
            for (Broc::bint scan((int)mp_util_wad::pLevel->warIndex);
                 (int)scan < Broc::size(mp_util_wad::pLevel->warAreas) &&
                 Broc::size(spawnpoints) == 0;
                 scan = (int)scan + 1) {
                Broc::entity candidateArea = mp_util_wad::pLevel->warAreas[
                    (unsigned int)(int)scan];
                Broc::entity candidateTrigger =
                    *mp_util_wad::GetEE_trigger(candidateArea);
                if ((int)*mp_util_wad::GetEE_capTeam(candidateTrigger) !=
                    (int)team)
                    continue;
                Broc::string candidateTarget;
                Broc::entity::__unnamed::targetname_struct candidateTargetField = {
                    candidateTrigger.GetHandle()};
                candidateTargetField.Get(&candidateTarget);
                Broc::GetEntArray(&candidateTarget, 0x15B1F8A7u,
                                  &temp_spawnpoints, 0);
                for (Broc::bint j(0); (int)j < Broc::size(temp_spawnpoints);
                     j = (int)j + 1) {
                    Broc::entity candidate =
                        temp_spawnpoints[(unsigned int)(int)j];
                    Broc::entity::__unnamed::classname_struct classnameField = {
                        candidate.GetHandle()};
                    Broc::string classname;
                    classnameField.Get(&classname);
                    if (classname == spawnType)
                        Broc::push(spawnpoints, candidate);
                }
            }
        }
    }

    if (Broc::size(spawnpoints) != 0) {
        _mp_spawnlogic::GetSpawnpointRandom(&spawnpoint, &spawnpoints);
        *result = spawnpoint;
        return result;
    }
    return _mp_common::GetSpawnPoint(result, self, self_team);
}

// WarnPlayerAboutInactiveFlag - ea: 0x97D110
void WarnPlayerAboutInactiveFlag(Broc::entity self) {
    *mp_util_wad::GetEE_lastTouch(self) = 0;
    Broc::bbool alreadyPlayedVO(false);
    for (;;) {
        Broc::bbool still_touching(false);
        Broc::bint i(0);
        while ((int)i < Broc::size(mp_util_wad::pLevel->warAreas)) {
            Broc::bint state;
            Broc::entity::__unnamed::playerState_struct playerStateField = {
                self.GetHandle()};
            playerStateField.Get(&state);
            if ((int)state == 3 &&
                (int)i != (int)mp_util_wad::pLevel->warIndex) {
                Broc::entity area =
                    mp_util_wad::pLevel->warAreas[(unsigned int)(int)i];
                Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
                if (Broc::IsTouching(&self, &trigger) &&
                    !Broc::Code_IsInVehicle(self))
                    still_touching = true;
            }
            i = (int)i + 1;
        }

        if (!(bool)still_touching)
            alreadyPlayedVO = false;
        if ((bool)still_touching) {
            if ((int)*mp_util_wad::GetEE_lastTouch(self) == 0) {
                Broc::bint now;
                Broc::GetTime(&now);
                *mp_util_wad::GetEE_lastTouch(self) = now;
            }

            Broc::bint now;
            Broc::GetTime(&now);
            if ((int)*mp_util_wad::GetEE_lastTouch(self) + 10000 <
                (int)now) {
                Broc::SetActionHint((int)0x6B01697Eu,
                                    Broc::GetPlayerIndex(self));
                if (!(bool)alreadyPlayedVO) {
                    Broc::string team;
                    Broc::entity::__unnamed::team_struct teamField = {
                        self.GetHandle()};
                    teamField.Get(&team);
                    if (team == "allies") {
                        Broc::string name("MP_WAR_WrongPos_Indiv_Allies");
                        Broc::SoundPlay(name, 1.0f);
                        name.~string();
                    } else {
                        Broc::string name("MP_WAR_WrongPos_Indiv_Axis");
                        Broc::SoundPlay(name, 1.0f);
                        name.~string();
                    }
                    team.~string();
                    alreadyPlayedVO = true;
                }
            }
        } else if ((int)*mp_util_wad::GetEE_lastTouch(self) > 0) {
            *mp_util_wad::GetEE_lastTouch(self) = 0;
            Broc::SetActionHint(-1, Broc::GetPlayerIndex(self));
        }
        Broc::wait(1.0f);
    }
}

// CallbackDebugRender - ea: 0x97D620
void CallbackDebugRender() {
    _mp_common::CallbackDebugRender();
    if (Broc::GetCvarInt("mp_debugrender") != 1)
        return;

    Broc::bint x(40);
    Broc::bint y(20);
    Broc::bint y_inc(20);
    Broc::string temp;

    if ((int)mp_util_wad::pLevel->warIndex < 0 ||
        (int)mp_util_wad::pLevel->warIndex >
            (int)mp_util_wad::pLevel->lastFlagIndex) {
        int index = (int)mp_util_wad::pLevel->warIndex;
        Broc::entity area =
            mp_util_wad::pLevel->warAreas[(unsigned int)index];
        Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
        float capSpeed = (float)*mp_util_wad::GetEE_capSpeed(trigger);
        Broc::bbool capping((int)mp_util_wad::pLevel->capCount != 0);

        temp = "War Flag Index: ";
        temp += index;
        Broc::Code_DebugRenderText(temp.c_str(), (int)x, (int)y);
        y += (int)y_inc;

        temp = "Players Capping: ";
        temp += (int)mp_util_wad::pLevel->capCount;
        Broc::Code_DebugRenderText(temp.c_str(), (int)x, (int)y);
        y += (int)y_inc;

        float capStatus = (float)*mp_util_wad::GetEE_capStatus(trigger);
        temp = "Cap Status: ";
        temp += capStatus;
        Broc::Code_DebugRenderText(temp.c_str(), (int)x, (int)y);
        y += (int)y_inc;

        float leftToGo = capSpeed > 0.0f
                             ? 1.0f - capStatus
                             : (-1.0f - capStatus) * -1.0f;
        temp = "Time Til Cap: ";
        if ((bool)capping)
            temp += (leftToGo * 20.0f) / capSpeed;
        else
            temp += "0";
        Broc::Code_DebugRenderText(temp.c_str(), (int)x, (int)y);
        y += (int)y_inc;

        temp = "Cap Speed: ";
        temp += capSpeed;
        Broc::Code_DebugRenderText(temp.c_str(), (int)x, (int)y);

        if ((bool)capping) {
            Broc::dyn_array<Broc::entity> players;
            Broc::GetPlayerArray(&players);
            for (int i = 0; i < Broc::size(players); ++i) {
                Broc::entity player = players[(unsigned int)i];
                Broc::bint playerState;
                mp_util_wad::entity_get_playerState(&playerState, player);
                if ((int)playerState != 3 ||
                    !Broc::IsTouching(&player, &trigger) ||
                    Broc::Code_IsInVehicle(player))
                    continue;

                Broc::string team;
                mp_util_wad::entity_get_team(&team, player);
                const Broc::vector* color =
                    team == "axis" ? &mp_util_wad::pLevel->spawnColorAxis
                                    : &mp_util_wad::pLevel->spawnColorAllies;
                Broc::vector origin;
                mp_util_wad::entity_get_origin(&origin, player);
                Broc::Code_DebugRenderSphere(&origin, 20.0f, color, 0.5f);
                team.~string();
            }
            players.~dyn_array();
        }
    }

    Broc::bint allies_capped(0);
    Broc::bint axis_capped(0);
    for (int i = 0; i < Broc::size(mp_util_wad::pLevel->warAreas); ++i) {
        Broc::entity area = mp_util_wad::pLevel->warAreas[(unsigned int)i];
        Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
        float capStatus = (float)*mp_util_wad::GetEE_capStatus(trigger);

        y += (int)y_inc;
        temp = "Cap Status  ";
        temp += i;
        temp += ": ";
        temp += capStatus;
        Broc::Code_DebugRenderText(temp.c_str(), (int)x, (int)y);

        if (capStatus >= 0.99989998f)
            ++allies_capped;
        if (capStatus <= -0.99989998f)
            ++axis_capped;
    }

    y += (int)y_inc;
    temp = "Cap Totals:  allies ";
    temp += (int)allies_capped;
    temp += " axis ";
    temp += (int)axis_capped;
    Broc::Code_DebugRenderText(temp.c_str(), (int)x, (int)y);

    for (int i = 0; i < Broc::size(mp_util_wad::pLevel->warAreas); ++i) {
        Broc::entity area = mp_util_wad::pLevel->warAreas[(unsigned int)i];
        Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
        Broc::vector color(1.0f, 0.0f, 0.0f);
        Broc::Code_DebugRenderEntityBBox(trigger, &color, 0.1f);
    }
    mp_util_wad::pLevel->capCount = 0;
    temp.~string();
}

// DebugRenderSpawnPoints - ea: 0x97E280
void DebugRenderSpawnPoints() {
    if (Broc::GetCvarInt("mp_debugrenderspawnpoints") != 0) {
        Broc::dyn_array<Broc::entity> spawnpoints;
        Broc::vector mins(-16.0f, -16.0f, 0.0f);
        Broc::vector maxs(16.0f, 16.0f, 72.0f);
        if ((int)mp_util_wad::pLevel->warIndex >= 0 &&
            (int)mp_util_wad::pLevel->warIndex <=
                (int)mp_util_wad::pLevel->lastFlagIndex) {
            Broc::entity area =
                mp_util_wad::pLevel->warAreas[(unsigned int)(int)mp_util_wad::pLevel->warIndex];
            Broc::entity& trigger = area->flagEnd.GetRef();
            Broc::string tn;
            Broc::entity::__unnamed::targetname_struct targetnameField = {
                trigger.GetHandle()};
            targetnameField.Get(&tn);
            HashStr key;
            key.mVal = 0x15B1F8A7u;
            Broc::GetEntArray(&tn, key.mVal, &spawnpoints, 0);
            tn.~string();

            Broc::vector origin;
            for (Broc::bint i(0); (int)i < Broc::size(spawnpoints);
                 i = (int)i + 1) {
                Broc::entity spawnpoint = spawnpoints[(unsigned int)i];
                Broc::entity::__unnamed::classname_struct classnameField = {
                    spawnpoint.GetHandle()};
                Broc::string classname;
                classnameField.Get(&classname);
                if (classname == mp_util_wad::pLevel->spawnTypeAllies) {
                    Broc::entity::__unnamed::origin_struct originField = {
                        spawnpoint.GetHandle()};
                    originField.Get(&origin);
                    Broc::vector maxBox = origin + maxs;
                    Broc::vector minBox = origin + mins;
                    Broc::Code_DebugRenderBox(
                        &minBox, &maxBox,
                        &mp_util_wad::pLevel->spawnColorAllies, 0.25f);
                }
                classname.~string();
            }

            for (Broc::bint i(0); (int)i < Broc::size(spawnpoints);
                 i = (int)i + 1) {
                Broc::entity spawnpoint = spawnpoints[(unsigned int)i];
                Broc::entity::__unnamed::classname_struct classnameField = {
                    spawnpoint.GetHandle()};
                Broc::string classname;
                classnameField.Get(&classname);
                if (classname == mp_util_wad::pLevel->spawnTypeAxis) {
                    Broc::entity::__unnamed::origin_struct originField = {
                        spawnpoint.GetHandle()};
                    originField.Get(&origin);
                    Broc::vector maxBox = origin + maxs;
                    Broc::vector minBox = origin + mins;
                    Broc::Code_DebugRenderBox(
                        &minBox, &maxBox,
                        &mp_util_wad::pLevel->spawnColorAxis, 0.25f);
                }
                classname.~string();
            }
        }
        spawnpoints.~dyn_array();
    }
}

void* main__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(main, self);
}
void* StartGame__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(StartGame, self);
}
void* WARScore__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WARScore, self);
}
void* WAR_Init__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WAR_Init, self);
}
void* WAR_InitFlag__functor(Broc::entity self, int flag_id) {
    (void)self; (void)flag_id;
    return NULL;
}
void* WAR_FlagUpdate__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WAR_FlagUpdate, self);
}
void* WAR_TouchFlag__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WAR_TouchFlag, self);
}
void* WarnPlayerAboutInactiveFlag__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(WarnPlayerAboutInactiveFlag, self);
}
void* PlayCapturedSounds__functor(Broc::entity self, Broc::bint team,
                                  Broc::bint originalWarIndex) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor3<Broc::entity, Broc::bint, Broc::bint>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor3<Broc::entity, Broc::bint, Broc::bint>(PlayCapturedSounds, self, team, originalWarIndex);
}

// PlayCapturedSounds - ea: 0x974FA0
void PlayCapturedSounds(Broc::entity self, Broc::bint team,
                        Broc::bint originalWarIndex) {
    Broc::string myteam("axis");
    Broc::string otherteam("allies");
    if ((int)team == 1) {
        myteam = "allies";
        otherteam = "axis";
    }
    if ((int)originalWarIndex == 0 && (int)team == 0) {
        Broc::SoundFadeOut((unsigned int)(int)mp_util_wad::pLevel->FlagMusic,
                           0.5f);
        Broc::entity level = mp_util_wad::pLevel != nullptr
                                 ? mp_util_wad::pLevel->_base.entity
                                 : Broc::entity();
        Broc::string sound("MX_War_LastFlagVictory");
        Broc::bfloat delay(0.0f);
        AeThreadFunctor* soundFunctor =
            _mp_audio::PlaySound__functor(level, sound, delay);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                            __LINE__, "_mp_audio::PlaySound", soundFunctor);
        Broc::string primaryTeam(myteam);
        Broc::string primarySound("MP_WAR_GreatTeamwork_Axis");
        Broc::string secondarySound("MP_Confirm_Lose_Allies");
        Broc::bfloat dialogDelay(2.5f);
        AeThreadFunctor* dialogFunctor =
            _mp_audio::PlayTeamDialog__functor(
                level, primaryTeam, primarySound, secondarySound, dialogDelay);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog",
                            dialogFunctor);
        dialogDelay.~bfloat();
        secondarySound.~string();
        primarySound.~string();
        primaryTeam.~string();
        delay.~bfloat();
        sound.~string();
    }
    else if ((int)originalWarIndex == (int)mp_util_wad::pLevel->lastFlagIndex
             && (int)team == 1) {
        Broc::SoundFadeOut((unsigned int)(int)mp_util_wad::pLevel->FlagMusic,
                           0.5f);
        Broc::entity level = mp_util_wad::pLevel != nullptr
                                 ? mp_util_wad::pLevel->_base.entity
                                 : Broc::entity();
        Broc::string sound("MX_War_LastFlagVictory");
        Broc::bfloat delay(0.0f);
        AeThreadFunctor* soundFunctor =
            _mp_audio::PlaySound__functor(level, sound, delay);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                            __LINE__, "_mp_audio::PlaySound", soundFunctor);
        Broc::string primaryTeam(myteam);
        Broc::string primarySound("MP_WAR_GreatTeamwork_Allies");
        Broc::string secondarySound("MP_Confirm_Lose_Axis");
        Broc::bfloat dialogDelay(2.5f);
        AeThreadFunctor* dialogFunctor =
            _mp_audio::PlayTeamDialog__functor(
                level, primaryTeam, primarySound, secondarySound, dialogDelay);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog",
                            dialogFunctor);
        dialogDelay.~bfloat();
        secondarySound.~string();
        primarySound.~string();
        primaryTeam.~string();
        delay.~bfloat();
        sound.~string();
    }
    else {
        Broc::SoundFadeOut((unsigned int)(int)mp_util_wad::pLevel->FlagMusic,
                           0.5f);
        Broc::entity level = mp_util_wad::pLevel != nullptr
                                 ? mp_util_wad::pLevel->_base.entity
                                 : Broc::entity();
        Broc::string teamSound((int)team == 1
                                   ? "MX_War_Ally_Secured_Flag"
                                   : "MX_War_Enemy_Secured_Flag");
        Broc::string otherTeamSound((int)team == 1
                                        ? "MX_War_Enemy_Secured_Flag"
                                        : "MX_War_Ally_Secured_Flag");
        AeThreadFunctor* soundFunctor =
            _mp_audio::PlayTeamSound__functor(
                level, myteam, teamSound, otherTeamSound);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                            __LINE__, "_mp_audio::PlayTeamSound",
                            soundFunctor);
        Broc::string primaryTeam(myteam);
        Broc::string primarySound((int)team == 1
                                      ? "MP_WAR_PositionCapt_Allies"
                                      : "MP_WAR_PositionCapt_Axis");
        Broc::string secondarySound((int)team == 1
                                        ? "MP_WAR_PositionLost_Axis"
                                        : "MP_WAR_PositionLost_Allies");
        Broc::bfloat dialogDelay(1.0f);
        AeThreadFunctor* dialogFunctor =
            _mp_audio::PlayTeamDialog__functor(
                level, primaryTeam, primarySound, secondarySound, dialogDelay);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                            __LINE__, "_mp_audio::PlayTeamDialog",
                            dialogFunctor);
        dialogDelay.~bfloat();
        secondarySound.~string();
        primarySound.~string();
        primaryTeam.~string();
        otherTeamSound.~string();
        teamSound.~string();
    }
    (void)self;
    myteam.~string();
    otherteam.~string();
}

// CallbackAreaCaptured - ea: 0x976150
void CallbackAreaCaptured(int index, int team) {
    if (index >= 0 &&
        index < Broc::size(mp_util_wad::pLevel->warAreas)) {
        mp_util_wad::pLevel->warIndex = index;
        Broc::bint now;
        Broc::GetTime(&now);
        mp_util_wad::pLevel->noCapTime =
            (int)now + (Broc::GetCvarInt("mp_debug") != 0 ? 1000 : 10000);
        mp_util_wad::pLevel->blinker = 1;
        (void)team;
    }
}

// SetupRound - ea: 0x977590
void SetupRound() {
    Broc::entity level = mp_util_wad::pLevel != nullptr
                             ? mp_util_wad::pLevel->_base.entity
                             : Broc::entity();
    void* inited = WAR_Init__functor(level);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_war.bro",
                        __LINE__, "WAR_Init", inited);

    mp_util_wad::pLevel->capCount = 0;
    mp_util_wad::pLevel->warIndex =
        (int)mp_util_wad::pLevel->lastFlagIndex / 2;
    Broc::bint now;
    Broc::GetTime(&now);
    mp_util_wad::pLevel->noCapTime =
        (int)now + (Broc::GetCvarInt("mp_debug") != 0 ? 1000 : 30000);

    Broc::bint i(0);
    while ((int)i < Broc::size(mp_util_wad::pLevel->warAreas)) {
        Broc::entity area = mp_util_wad::pLevel->warAreas[(unsigned int)(int)i];
        Broc::entity trigger = *mp_util_wad::GetEE_trigger(area);
        Broc::bbool triggerDefined;
        mp_util_wad::IsEEDefined_trigger(&triggerDefined, area);
        if ((bool)triggerDefined) {
            Broc::string modelName("p_mp_dom_flag_hanging_nt");
            Broc::entity flag = *mp_util_wad::GetEE_flag(trigger);
            Broc::SetModel(&flag, &modelName, 0);

            if ((int)i == (int)mp_util_wad::pLevel->warIndex) {
                Broc::string state("i_objective_c");
                Broc::string pszString("flag");
                Broc::vector origin;
                mp_util_wad::entity_get_origin(&origin, area);
                ObjectiveAdd((int)i, state, pszString, origin,
                             (float)lWARObjectiveHeight, -1);
                *mp_util_wad::GetEE_capTeam(trigger) = 0;
                *mp_util_wad::GetEE_capStatus(trigger) = 0.0f;
            } else if ((int)i < (int)mp_util_wad::pLevel->warIndex) {
                ObjectiveDelete((int)i, -1);
                Broc::entity claimedFlag =
                    *mp_util_wad::GetEE_flag(trigger);
                Broc::SetModel(&claimedFlag,
                               &mp_util_wad::pLevel->AlliesFlagModel, 0);
                *mp_util_wad::GetEE_capTeam(trigger) = 1;
                *mp_util_wad::GetEE_capStatus(trigger) = 1.0f;
            } else if ((int)i > (int)mp_util_wad::pLevel->warIndex) {
                ObjectiveDelete((int)i, -1);
                Broc::entity claimedFlag =
                    *mp_util_wad::GetEE_flag(trigger);
                Broc::SetModel(&claimedFlag,
                               &mp_util_wad::pLevel->AxisFlagModel, 0);
                *mp_util_wad::GetEE_capTeam(trigger) = -1;
                *mp_util_wad::GetEE_capStatus(trigger) = -1.0f;
            }

            Broc::entity::__unnamed::origin_struct flagOrigin = {
                flag.GetHandle()};
            flagOrigin = mp_util_wad::GetEE_flagStart(trigger);
        }
        i = (int)i + 1;
    }

    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    UpdateAllowedCap();
}
}
namespace _mp_shellshock {
void main() {
}

// ShellshockOnDamage - ea: 0x96C9C0
void ShellshockOnDamage(Broc::entity self, ::bint cause, ::bint damage) {
    if ((int)cause == 27 || (int)cause == 3 || (int)cause == 4 ||
        (int)cause == 5 || (int)cause == 6 || (int)cause == 17 ||
        (int)cause == 18 || (int)cause == 9 || (int)cause == 10) {
        Broc::bint time(0);
        if ((int)damage < 90) {
            if ((int)damage < 50) {
                if ((int)damage < 25) {
                    if ((int)damage > 10)
                        time = 1;
                } else {
                    blur_view(self, ::bfloat(0.25f));
                    time = 2;
                }
            } else {
                blur_view(self, ::bfloat(1.0f));
                time = 3;
            }
        } else {
            blur_view(self, ::bfloat(2.0f));
            time = 4;
        }
        if ((int)time != 0) {
            Broc::string shock("default");
            Broc::ShellShock(&self, &shock, (float)(int)time);
            shock.~string();
        }
    }
}

// blur_view - ea: 0x96CCA0
void blur_view(Broc::entity self, ::bfloat blur_time) {
    (void)self;
    (void)blur_time;
}
}
// ============================================================================
// _mp_spawnlogic - spawn-point selection.
// ============================================================================
namespace _mp_spawnlogic {

// GetSpawnpointRandom (no ignoreTeleFrag) - ea: 0x96CCC0
Broc::entity* GetSpawnpointRandom(Broc::entity* result,
                                  Broc::dyn_array<Broc::entity>* spawnpoints) {
    if (Broc::GetCvarInt("cg_debugSpawnPoints") <= 0)
        GetSpawnpointRandom(result, spawnpoints, Broc::bbool(false));
    else
        GetSpawnpointInOrder(result, spawnpoints);
    return result;
}

// GetSpawnpointRandom - ea: 0x96CD30
Broc::entity* GetSpawnpointRandom(Broc::entity* result,
                                  Broc::dyn_array<Broc::entity>* spawnpoints,
                                  Broc::bbool ignoreTeleFrag) {
    if (Broc::size(*spawnpoints) == 0) {
        result->___u0 = 0;
        return result;
    }
    Broc::bint i(0);
    Broc::bint j(0);
    Broc::bint count(Broc::size(*spawnpoints));
    Broc::entity spawnpoint;
    spawnpoint.___u0 = 0;
    i = 0;
    while ((int)i < (int)count) {
        j = Broc::RandomInt((int)count);
        Broc::entity tmp = (*spawnpoints)[(unsigned int)(int)i];
        (*spawnpoints)[(unsigned int)(int)i] =
            (*spawnpoints)[(unsigned int)(int)j];
        (*spawnpoints)[(unsigned int)(int)j] = tmp;
        i = (int)i + 1;
    }
    i = 0;
    while ((int)i < Broc::size(*spawnpoints)) {
        spawnpoint = (*spawnpoints)[(unsigned int)(int)i];
        if (!(bool)ignoreTeleFrag) {
            Broc::vector origin;
            mp_util_wad::entity_get_origin(&origin, spawnpoint);
            if (!Broc::Code_PositionWouldTelefrag(&origin))
                break;
        }
        i = (int)i + 1;
    }
    *result = spawnpoint;
    return result;
}

// GetSpawnpointInOrder - ea: 0x96D180
Broc::entity* GetSpawnpointInOrder(Broc::entity* result,
                                   Broc::dyn_array<Broc::entity>* spawnpoints) {
    Broc::bint maxCount(Broc::size(*spawnpoints));
    Broc::bint currentIndex((int)mp_util_wad::pLevel->lastSpawnPointIndex);
    if ((int)currentIndex >= (int)maxCount) {
        mp_util_wad::pLevel->lastSpawnPointIndex = 0;
        Broc::iprintlnbold(Broc::string("Starting over at first spawn point."));
    }
    Broc::iprintlnbold(Broc::string("Moving to next spawn point."));
    if ((int)currentIndex < 0 || (int)currentIndex >= (int)maxCount) {
        if (Broc::gBrocAPI.mAssert("c:\\cod\\code\\script\\_mp_spawnlogic.bro",
                                   __LINE__, "invalid spawn point"))
            __debugbreak();
    }
    unsigned int idx =
        (unsigned int)(int)mp_util_wad::pLevel->lastSpawnPointIndex;
    mp_util_wad::pLevel->lastSpawnPointIndex =
        (int)mp_util_wad::pLevel->lastSpawnPointIndex + 1;
    *result = (*spawnpoints)[idx];
    return result;
}

// GetSpawnpointNearest - ea: 0x96CF80
Broc::entity* GetSpawnpointNearest(Broc::entity* result,
                                   Broc::dyn_array<Broc::entity>* spawnpoints,
                                   Broc::vector position,
                                   Broc::bbool ignoreTeleFrag) {
    if (Broc::size(*spawnpoints) == 0) {
        result->___u0 = 0;
        return result;
    }
    Broc::bint i(0);
    Broc::entity spawnpoint;
    spawnpoint.___u0 = 0;
    Broc::bint dist(0x1869F);
    Broc::bint tempDist(0);
    i = 0;
    while ((int)i < Broc::size(*spawnpoints)) {
        Broc::vector origin;
        mp_util_wad::entity_get_origin(&origin, (*spawnpoints)[(unsigned int)(int)i]);
        tempDist = (int)Broc::Distance(&origin, &position);
        if ((int)tempDist < (int)dist) {
            if ((bool)ignoreTeleFrag ||
                !Broc::Code_PositionWouldTelefrag(&origin)) {
                dist = (int)tempDist;
                spawnpoint = (*spawnpoints)[(unsigned int)(int)i];
            }
        }
        i = (int)i + 1;
    }
    if (Broc::IsDefined(spawnpoint))
        *result = spawnpoint;
    else
        GetSpawnpointRandom(result, spawnpoints, ignoreTeleFrag);
    return result;
}

// GetSpawnpointNearTeamAntiCamp - ea: 0x96DD50
Broc::entity* GetSpawnpointNearTeamAntiCamp(Broc::entity* result,
                                            Broc::entity* self,
                                            const Broc::string* team,
                                            Broc::dyn_array<Broc::entity>* spawnpoints) {
    Broc::bint spawn_camp_time(12000);
    Broc::bint now;
    Broc::GetTime(&now);
    Broc::bint lastSpawn((int)*mp_util_wad::GetEE_spawnTime(*self));
    if ((int)lastSpawn + (int)spawn_camp_time > (int)now) {
        Broc::bfloat anti_camp_pick_chance(0.5f);
        if (Broc::RandomInt(100) <
            (int)(100.0f * (float)anti_camp_pick_chance)) {
            GetSpawnpointSemiRandom(result, self, team, spawnpoints);
            return result;
        }
    }
    GetSpawnpointNearTeam(result, self, team, spawnpoints);
    return result;
}

// GetSpawnpointSemiRandom - ea: 0x96E570
Broc::entity* GetSpawnpointSemiRandom(Broc::entity* result, Broc::entity* self,
                                      const Broc::string* team,
                                      Broc::dyn_array<Broc::entity>* spawnpoints) {
    (void)team;
    if (Broc::GetCvarInt("cg_debugSpawnPoints") > 0) {
        GetSpawnpointInOrder(result, spawnpoints);
        return result;
    }
    if (Broc::size(*spawnpoints) == 0) {
        result->___u0 = 0;
        return result;
    }
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    Broc::entity player;
    player.___u0 = 0;
    Broc::dyn_array<Broc::entity> aliveplayers;
    i = 0;
    while ((int)i < Broc::size(players)) {
        player = players[(unsigned int)(int)i];
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, player);
        if ((int)state == 3 && !(player == *self))
            aliveplayers.push_back(player);
        i = (int)i + 1;
    }
    Broc::entity spawnpoint;
    spawnpoint.___u0 = 0;
    if (Broc::size(aliveplayers) <= 0) {
        GetSpawnpointRandom(&spawnpoint, spawnpoints);
    } else {
        Broc::bint j(0);
        Broc::dyn_array<Broc::entity> semirandomspawns;
        i = 0;
        while ((int)i < Broc::size(*spawnpoints)) {
            Broc::vector sp;
            mp_util_wad::entity_get_origin(&sp, (*spawnpoints)[(unsigned int)(int)i]);
            if (!Broc::Code_PositionWouldTelefrag(&sp)) {
                j = 0;
                while ((int)j < Broc::size(aliveplayers)) {
                    Broc::vector ap;
                    mp_util_wad::entity_get_origin(&ap,
                                                   aliveplayers[(unsigned int)(int)j]);
                    Broc::vector sp2;
                    mp_util_wad::entity_get_origin(
                        &sp2, (*spawnpoints)[(unsigned int)(int)i]);
                    if (::DistanceSquared(sp2, ap) > 4000000.0f) {
                        semirandomspawns.push_back(
                            (*spawnpoints)[(unsigned int)(int)i]);
                        break;
                    }
                    j = (int)j + 1;
                }
            }
            i = (int)i + 1;
        }
        if (Broc::size(semirandomspawns) <= 0)
            GetSpawnpointRandom(&spawnpoint, spawnpoints);
        else
            GetSpawnpointRandom(&spawnpoint, &semirandomspawns);
        semirandomspawns.~dyn_array();
    }
    *result = spawnpoint;
    aliveplayers.~dyn_array();
    players.~dyn_array();
    return result;
}

// GetSpawnpointNearTeam - ea: 0x96DE80
Broc::entity* GetSpawnpointNearTeam(Broc::entity* result, Broc::entity* self,
                                    const Broc::string* team,
                                    Broc::dyn_array<Broc::entity>* spawnpoints) {
    Broc::bbool player_on_my_team(false);
    if (Broc::GetCvarInt("cg_debugSpawnPoints") > 0) {
        GetSpawnpointInOrder(result, spawnpoints);
        return result;
    }
    if (Broc::size(*spawnpoints) == 0) {
        result->___u0 = 0;
        return result;
    }
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    Broc::entity player;
    player.___u0 = 0;
    Broc::dyn_array<Broc::entity> aliveplayers;
    i = 0;
    while ((int)i < Broc::size(players)) {
        player = players[(unsigned int)(int)i];
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, player);
        if ((int)state == 3 && !(player == *self)) {
            aliveplayers.push_back(player);
            Broc::string pteam;
            mp_util_wad::entity_get_team(&pteam, player);
            if (pteam == *team)
                player_on_my_team = true;
            pteam.~string();
        }
        i = (int)i + 1;
    }
    if (!(bool)player_on_my_team) {
        GetSpawnpointSemiRandom(result, self, team, spawnpoints);
        aliveplayers.~dyn_array();
        players.~dyn_array();
        return result;
    }
    Broc::entity spawnpoint;
    spawnpoint.___u0 = 0;
    if (Broc::size(aliveplayers) <= 0) {
        GetSpawnpointRandom(&spawnpoint, spawnpoints);
    } else {
        Broc::bint distlargest(-33554432);
        Broc::bint dist(0);
        Broc::bfloat weight(0.0f);
        Broc::bint j(0);
        Broc::entity bestposition;
        bestposition.___u0 = 0;
        i = 0;
        while ((int)i < Broc::size(*spawnpoints)) {
            Broc::vector sp;
            mp_util_wad::entity_get_origin(&sp, (*spawnpoints)[(unsigned int)(int)i]);
            if (!Broc::Code_PositionWouldTelefrag(&sp)) {
                dist = 0;
                if (Broc::size(aliveplayers) > 0) {
                    j = 0;
                    while ((int)j < Broc::size(aliveplayers)) {
                        weight = 1.0f;
                        Broc::string pteam;
                        mp_util_wad::entity_get_team(
                            &pteam, aliveplayers[(unsigned int)(int)j]);
                        if (pteam == *team) {
                            player_on_my_team = true;
                            weight = (float)weight * -2.0f;
                        } else {
                            weight = (float)weight * 1.0f;
                        }
                        pteam.~string();
                        weight = (float)weight *
                                 (Broc::RandomFloat(0.4f) + 0.8f);
                        Broc::vector ap;
                        Broc::vector sp2;
                        mp_util_wad::entity_get_origin(
                            &ap, aliveplayers[(unsigned int)(int)j]);
                        mp_util_wad::entity_get_origin(
                            &sp2, (*spawnpoints)[(unsigned int)(int)i]);
                        float d = Broc::Distance(&sp2, &ap);
                        dist = (int)dist + (int)(d * (float)weight);
                        j = (int)j + 1;
                    }
                }
                if ((int)dist > (int)distlargest) {
                    distlargest = (int)dist;
                    bestposition = (*spawnpoints)[(unsigned int)(int)i];
                }
            }
            i = (int)i + 1;
        }
        spawnpoint = bestposition;
    }
    if (!Broc::IsDefined(spawnpoint))
        GetSpawnpointRandom(&spawnpoint, spawnpoints);
    *result = spawnpoint;
    aliveplayers.~dyn_array();
    players.~dyn_array();
    return result;
}

// GetSpawnpointDM - ea: 0x96D2C0
Broc::entity* GetSpawnpointDM(Broc::entity* result, Broc::entity* self,
                              Broc::dyn_array<Broc::entity>* spawnpoints) {
    if (Broc::GetCvarInt("cg_debugSpawnPoints") > 0) {
        GetSpawnpointInOrder(result, spawnpoints);
        return result;
    }
    if (Broc::size(*spawnpoints) == 0) {
        result->___u0 = 0;
        return result;
    }
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    Broc::entity player;
    player.___u0 = 0;
    Broc::dyn_array<Broc::entity> aliveplayers;
    i = 0;
    while ((int)i < Broc::size(players)) {
        player = players[(unsigned int)(int)i];
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, player);
        if ((int)state == 3 && !(player == *self))
            aliveplayers.push_back(player);
        i = (int)i + 1;
    }
    if (Broc::size(aliveplayers) <= 0) {
        GetSpawnpointRandom(result, spawnpoints);
        aliveplayers.~dyn_array();
        players.~dyn_array();
        return result;
    }
    Broc::dyn_array<Broc::entity> filteredspawnpoints;
    Broc::dyn_array<int> filteredspawnpointsscore;
    i = 0;
    while ((int)i < Broc::size(*spawnpoints)) {
        Broc::vector origin;
        mp_util_wad::entity_get_origin(&origin,
                                       (*spawnpoints)[(unsigned int)(int)i]);
        if (!Broc::Code_PositionWouldTelefrag(&origin)) {
            Broc::bbool hasLast;
            mp_util_wad::IsEEDefined_lastspawnpoint(&hasLast, *self);
            if (!(bool)hasLast ||
                !(*mp_util_wad::GetEE_lastspawnpoint(*self) ==
                  (*spawnpoints)[(unsigned int)(int)i])) {
                filteredspawnpoints.push_back(
                    (*spawnpoints)[(unsigned int)(int)i]);
                filteredspawnpointsscore.push_back(0);
            }
        }
        i = (int)i + 1;
    }
    if (Broc::size(filteredspawnpoints) == 0) {
        GetSpawnpointRandom(result, spawnpoints);
        filteredspawnpointsscore.~dyn_array();
        filteredspawnpoints.~dyn_array();
        aliveplayers.~dyn_array();
        players.~dyn_array();
        return result;
    }
    Broc::bint shortest(100000);
    Broc::bint current(0);
    Broc::bint j(0);
    i = 0;
    while ((int)i < Broc::size(filteredspawnpoints)) {
        shortest = 1000000;
        j = 0;
        while ((int)j < Broc::size(aliveplayers)) {
            Broc::vector ap;
            Broc::vector sp;
            mp_util_wad::entity_get_origin(
                &ap, aliveplayers[(unsigned int)(int)j]);
            mp_util_wad::entity_get_origin(
                &sp, filteredspawnpoints[(unsigned int)(int)i]);
            current = (int)Broc::Distance(&sp, &ap);
            if ((int)current < (int)shortest)
                shortest = (int)current;
            j = (int)j + 1;
        }
        filteredspawnpointsscore[(unsigned int)(int)i] = (int)shortest + 1;
        i = (int)i + 1;
    }
    Broc::bint newsize(Broc::size(filteredspawnpoints) / 3);
    if ((int)newsize < 1)
        newsize = 1;
    Broc::bint total(0);
    Broc::bint bestscore(0);
    Broc::dyn_array<Broc::entity> newspawnpoints;
    Broc::dyn_array<int> newspawnpointsscore;
    i = 0;
    while ((int)i < (int)newsize) {
        j = 0;
        while ((int)j < Broc::size(filteredspawnpoints)) {
            current = filteredspawnpointsscore[(unsigned int)(int)j];
            if ((int)current > (int)bestscore)
                bestscore = (int)current;
            j = (int)j + 1;
        }
        j = 0;
        while ((int)j < Broc::size(filteredspawnpoints)) {
            if (filteredspawnpointsscore[(unsigned int)(int)j] ==
                (int)bestscore) {
                newspawnpoints.push_back(
                    filteredspawnpoints[(unsigned int)(int)j]);
                newspawnpointsscore.push_back(
                    filteredspawnpointsscore[(unsigned int)(int)j]);
                total = (int)total +
                        filteredspawnpointsscore[(unsigned int)(int)j];
                filteredspawnpointsscore[(unsigned int)(int)j] = 0;
                bestscore = 0;
                break;
            }
            j = (int)j + 1;
        }
        i = (int)i + 1;
    }
    Broc::bint randnum(Broc::RandomInt((int)total));
    Broc::entity spawnpoint;
    spawnpoint.___u0 = 0;
    i = 0;
    while ((int)i < Broc::size(newspawnpoints)) {
        randnum = (int)randnum - newspawnpointsscore[(unsigned int)(int)i];
        spawnpoint = newspawnpoints[(unsigned int)(int)i];
        if ((int)randnum < 0)
            break;
        i = (int)i + 1;
    }
    mp_util_wad::GetEE_lastspawnpoint(*self)->___u0 = spawnpoint.___u0;
    *result = spawnpoint;
    newspawnpointsscore.~dyn_array();
    newspawnpoints.~dyn_array();
    filteredspawnpointsscore.~dyn_array();
    filteredspawnpoints.~dyn_array();
    aliveplayers.~dyn_array();
    players.~dyn_array();
    return result;
}
}

// get_spawnpoint_near_team_away_from_radios - ea: 0x96E9C0
namespace _mp_spawnlogic {
Broc::entity* get_spawnpoint_near_team_away_from_radios(
    Broc::entity* result, Broc::entity* self, const Broc::string* team,
    Broc::dyn_array<Broc::entity>* spawnpoints) {
    if (Broc::size(*spawnpoints) == 0) {
        result->___u0 = 0;
        return result;
    }
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::dyn_array<Broc::entity> aliveplayers;
    Broc::bint i(0);
    Broc::entity player;
    player.___u0 = 0;
    while ((int)i < Broc::size(players)) {
        player = players[(unsigned int)(int)i];
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, player);
        if ((int)state == 3 && !(player == *self))
            aliveplayers.push_back(player);
        i = (int)i + 1;
    }
    Broc::entity spawnpoint;
    spawnpoint.___u0 = 0;
    if (Broc::size(aliveplayers) <= 0) {
        get_spawnpoint_middle_third(&spawnpoint, self, team, spawnpoints);
    } else {
        i = 0;
        while ((int)i < Broc::size(*spawnpoints)) {
            Broc::entity sp = (*spawnpoints)[(unsigned int)(int)i];
            Broc::vector spPos;
            mp_util_wad::entity_get_origin(&spPos, sp);
            if (Broc::Code_PositionWouldTelefrag(&spPos)) {
                *mp_util_wad::GetEE_spawnscore(sp) = -37483274;
            } else {
                *mp_util_wad::GetEE_spawnscore(sp) = 0;
                Broc::bint j(0);
                while ((int)j < Broc::size(aliveplayers)) {
                    Broc::entity ap = aliveplayers[(unsigned int)(int)j];
                    Broc::vector apPos;
                    mp_util_wad::entity_get_origin(&apPos, ap);
                    Broc::bfloat dist(Broc::Distance(&spPos, &apPos));
                    Broc::string ateam;
                    mp_util_wad::entity_get_team(&ateam, ap);
                    if (ateam == *team) {
                        Broc::bfloat bias(1.6f);
                        if ((int)mp_util_wad::pLevel->hq_stage == 3) {
                            bool defender =
                                (ateam == "allies" &&
                                 (bool)mp_util_wad::pLevel->allies_defending) ||
                                (ateam == "axis" &&
                                 !(bool)mp_util_wad::pLevel->allies_defending);
                            if (defender)
                                bias = 0.5f;
                        }
                        *mp_util_wad::GetEE_spawnscore(sp) -=
                            (int)((float)dist * (float)bias);
                    } else if ((float)dist > 850.0f) {
                        *mp_util_wad::GetEE_spawnscore(sp) +=
                            (int)((float)dist * 0.8f);
                    } else {
                        *mp_util_wad::GetEE_spawnscore(sp) = -37483274;
                    }
                    ateam.~string();
                    j = (int)j + 1;
                }
                if ((int)mp_util_wad::pLevel->hq_stage == 3 &&
                    ((*team == "allies" &&
                      !(bool)mp_util_wad::pLevel->allies_defending) ||
                     (*team == "axis" &&
                      (bool)mp_util_wad::pLevel->allies_defending))) {
                    Broc::vector hq = (bool)mp_util_wad::pLevel->pointA_isHQ
                                          ? mp_util_wad::pLevel->pointA
                                          : mp_util_wad::pLevel->pointB;
                    Broc::bfloat dist(Broc::Distance(&spPos, &hq));
                    if ((float)dist <= 700.0f)
                        *mp_util_wad::GetEE_spawnscore(sp) = -37483274;
                }
            }
            i = (int)i + 1;
        }
        // Selection-sort spawnpoints by ascending score.
        Broc::entity temp;
        i = 0;
        while ((int)i < Broc::size(*spawnpoints)) {
            for (Broc::bint j((int)i); (int)j < Broc::size(*spawnpoints);
                 j = (int)j + 1) {
                Broc::entity a = (*spawnpoints)[(unsigned int)(int)i];
                Broc::entity b = (*spawnpoints)[(unsigned int)(int)j];
                if ((int)*mp_util_wad::GetEE_spawnscore(b) <
                    (int)*mp_util_wad::GetEE_spawnscore(a)) {
                    temp = a;
                    (*spawnpoints)[(unsigned int)(int)i] = b;
                    (*spawnpoints)[(unsigned int)(int)j] = temp;
                }
            }
            i = (int)i + 1;
        }
        i = 0;
        while ((int)i < Broc::size(*spawnpoints)) {
            Broc::entity sp = (*spawnpoints)[(unsigned int)(int)i];
            Broc::vector spPos;
            mp_util_wad::entity_get_origin(&spPos, sp);
            if (!Broc::Code_PositionWouldTelefrag(&spPos) &&
                (int)*mp_util_wad::GetEE_spawnscore(sp) != -37483274) {
                *result = sp;
                aliveplayers.~dyn_array();
                players.~dyn_array();
                return result;
            }
            i = (int)i + 1;
        }
        get_spawnpoint_middle_third(&spawnpoint, self, team, spawnpoints);
    }
    *result = spawnpoint;
    aliveplayers.~dyn_array();
    players.~dyn_array();
    return result;
}

// get_spawnpoint_middle_third - ea: 0x96F6C0
Broc::entity* get_spawnpoint_middle_third(
    Broc::entity* result, Broc::entity* self, const Broc::string* team,
    Broc::dyn_array<Broc::entity>* spawnpoints) {
    if (Broc::size(*spawnpoints) == 0) {
        result->___u0 = 0;
        return result;
    }
    Broc::dyn_array<Broc::vector> badspot;
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint axiscount(-1);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        Broc::bint state;
        mp_util_wad::entity_get_playerState(&state, p);
        if ((int)state != 1 && (int)state != 4 && (int)state != 5 &&
            !(p == *self)) {
            Broc::string pteam;
            mp_util_wad::entity_get_team(&pteam, p);
            if (pteam != *team) {
                axiscount = (int)axiscount + 1;
                Broc::vector pos;
                mp_util_wad::entity_get_origin(&pos, p);
                badspot.push_back(pos);
            }
            pteam.~string();
        }
        i = (int)i + 1;
    }
    if (Broc::size(badspot) <= 0) {
        GetSpawnpointRandom(result, spawnpoints);
        players.~dyn_array();
        badspot.~dyn_array();
        return result;
    }
    i = 0;
    while ((int)i < Broc::size(*spawnpoints)) {
        Broc::entity sp = (*spawnpoints)[(unsigned int)(int)i];
        Broc::vector spPos;
        mp_util_wad::entity_get_origin(&spPos, sp);
        Broc::bint closest(1000000);
        if (Broc::Code_PositionWouldTelefrag(&spPos)) {
            *mp_util_wad::GetEE_spawnscore(sp) = -1;
        } else {
            *mp_util_wad::GetEE_spawnscore(sp) = 0;
            Broc::bint j(0);
            while ((int)j < Broc::size(badspot)) {
                Broc::vector b = badspot[(unsigned int)(int)j];
                Broc::vector scaled(b.x, b.y, b.z * 5.0f);
                Broc::vector spScaled(spPos.x, spPos.y, spPos.z * 5.0f);
                Broc::bfloat distancescore(Broc::Distance(&spScaled, &scaled));
                if ((int)j > (int)axiscount)
                    distancescore = (float)distancescore * 4.0f;
                if ((float)distancescore < (float)closest)
                    closest = (int)(float)distancescore;
                j = (int)j + 1;
            }
            *mp_util_wad::GetEE_spawnscore(sp) = (int)closest;
        }
        i = (int)i + 1;
    }
    // Sort ascending by score.
    Broc::entity temp;
    i = 0;
    while ((int)i < Broc::size(*spawnpoints)) {
        for (Broc::bint j((int)i); (int)j < Broc::size(*spawnpoints);
             j = (int)j + 1) {
            Broc::entity a = (*spawnpoints)[(unsigned int)(int)i];
            Broc::entity b = (*spawnpoints)[(unsigned int)(int)j];
            if ((int)*mp_util_wad::GetEE_spawnscore(b) <
                (int)*mp_util_wad::GetEE_spawnscore(a)) {
                temp = a;
                (*spawnpoints)[(unsigned int)(int)i] = b;
                (*spawnpoints)[(unsigned int)(int)j] = temp;
            }
        }
        i = (int)i + 1;
    }
    Broc::bint firsthalf(Broc::size(*spawnpoints) / 2);
    Broc::bint lastsixth(Broc::size(*spawnpoints) -
                         Broc::size(*spawnpoints) / 6);
    Broc::dyn_array<Broc::entity> GoodSpawnPoints;
    for (i = (int)firsthalf; (int)i < (int)lastsixth; i = (int)i + 1)
        GoodSpawnPoints.push_back((*spawnpoints)[(unsigned int)(int)i]);
    if (Broc::size(GoodSpawnPoints) >= 1)
        GetSpawnpointRandom(result, &GoodSpawnPoints);
    else
        GetSpawnpointRandom(result, spawnpoints);
    GoodSpawnPoints.~dyn_array();
    players.~dyn_array();
    badspot.~dyn_array();
    return result;
}
}

// ============================================================================
// _mp_teambalance - team balancing.
// ============================================================================
namespace _mp_teambalance {

// main - ea: 0x973690
void main() {
}

// PickTeam - ea: 0x9736B0
Broc::string* PickTeam(Broc::string* result, Broc::entity player) {
    Broc::bint allies(0);
    Broc::bint axis(0);
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        if (!(p == player)) {
            Broc::bint state;
            mp_util_wad::entity_get_playerState(&state, p);
            if ((int)state != 0) {
                Broc::string team;
                mp_util_wad::entity_get_team(&team, p);
                if (team == "allies")
                    allies = (int)allies + 1;
                else if (team == "axis")
                    axis = (int)axis + 1;
                team.~string();
            }
        }
        i = (int)i + 1;
    }
    if ((int)allies < (int)axis)
        *result = "allies";
    else if ((int)allies > (int)axis)
        *result = "axis";
    else {
        Broc::string team("allies");
        allies = Broc::Code_GetTeamScore(team);
        team.~string();
        Broc::string team2("axis");
        axis = Broc::Code_GetTeamScore(team2);
        team2.~string();
        if ((int)allies < (int)axis)
            *result = "allies";
        else if ((int)allies > (int)axis)
            *result = "axis";
        else
            *result = Broc::RandomFloat(1.0f) >= 0.5f ? "allies" : "axis";
    }
    players.~dyn_array();
    return result;
}

// team_balance - ea: 0x973BA0
Broc::string* team_balance(Broc::string* result, Broc::entity guy,
                           Broc::string team) {
    Broc::bint allies(0);
    Broc::bint axis(0);
    Broc::dyn_array<Broc::entity> players;
    Broc::GetPlayerArray(&players);
    Broc::bint i(0);
    while ((int)i < Broc::size(players)) {
        Broc::entity p = players[(unsigned int)(int)i];
        if (p == guy) {
            if (team == "allies")
                allies = (int)allies + 1;
            if (team == "axis")
                axis = (int)axis + 1;
        } else {
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
    Broc::bint balance_allowance(2);
    if ((int)allies < (int)axis + (int)balance_allowance) {
        if ((int)axis >= (int)allies + (int)balance_allowance &&
            team == "axis") {
            *result = "allies";
            players.~dyn_array();
            team.~string();
            return result;
        }
    } else if (team == "allies") {
        *result = "axis";
        players.~dyn_array();
        team.~string();
        return result;
    }
    *result = team;
    players.~dyn_array();
    team.~string();
    return result;
}

// team_balance (always) - ea: 0x973FA0
void team_balance(Broc::bbool always) {
    if ((bool)mp_util_wad::pLevel->teamBalance || (bool)always) {
        Broc::dyn_array<Broc::entity> players;
        Broc::GetPlayerArray(&players);
        Broc::dyn_array<Broc::entity> allies;
        Broc::dyn_array<Broc::entity> axis;
        Broc::bint i(0);
        while ((int)i < Broc::size(players)) {
            Broc::entity p = players[(unsigned int)(int)i];
            Broc::string team;
            mp_util_wad::entity_get_team(&team, p);
            if (team == "allies")
                allies.push_back(p);
            else if (team == "axis")
                axis.push_back(p);
            team.~string();
            i = (int)i + 1;
        }
        Broc::bint size_allies(Broc::size(allies));
        Broc::bint size_axis(Broc::size(axis));
        Broc::bint total((int)size_allies - (int)size_axis);
        Broc::bint half_total((int)total / 2);
        if ((int)size_allies < (int)size_axis + 2) {
            if ((int)size_axis >= (int)size_allies + 2) {
                Broc::bint rhs((int)size_axis - (int)half_total);
                Broc::dyn_array<Broc::entity> ar;
                Broc::bint k(0);
                while ((int)k < (int)rhs) {
                    Broc::entity pick = axis[(unsigned int)Broc::RandomInt(
                        Broc::size(axis))];
                    ar.push_back(pick);
                    k = (int)k + 1;
                }
                Broc::bint m(0);
                while ((int)m < Broc::size(ar)) {
                    Broc::string team("allies");
                    Broc::Code_ChangePlayerTeam(ar[(unsigned int)(int)m],
                                                &team, false);
                    team.~string();
                    m = (int)m + 1;
                }
                ar.~dyn_array();
            }
        } else {
            Broc::bint over_count((int)size_allies - (int)half_total);
            Broc::dyn_array<Broc::entity> balanced;
            Broc::bint k(0);
            while ((int)k < (int)over_count) {
                Broc::entity pick = allies[(unsigned int)Broc::RandomInt(
                    Broc::size(allies))];
                balanced.push_back(pick);
                k = (int)k + 1;
            }
            Broc::bint m(0);
            while ((int)m < Broc::size(balanced)) {
                Broc::string team("axis");
                Broc::Code_ChangePlayerTeam(balanced[(unsigned int)(int)m],
                                            &team, false);
                team.~string();
                m = (int)m + 1;
            }
            balanced.~dyn_array();
        }
        axis.~dyn_array();
        allies.~dyn_array();
        players.~dyn_array();
    }
}
}
namespace _mp_common {
AeThreadFunctor* StopFollowing__functor(Broc::entity self, Broc::bbool blackNow) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::bbool>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::bbool>(StopFollowing, self, blackNow);
}
AeThreadFunctor* QuitGameThread__functor(Broc::entity selfLevel) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(QuitGameThread, selfLevel);
}
AeThreadFunctor* QuitGameWithMessage__functor(Broc::entity self, HashStr message) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, HashStr>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, HashStr>(QuitGameWithMessage, self, message);
}
AeThreadFunctor* HostHasMigrated__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(HostHasMigrated, self);
}
AeThreadFunctor* RespawnPlayer__functor(Broc::entity guy, Broc::string team) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::string>));
    if (storage == NULL)
        return NULL;
    AeThreadFunctor* result = ::new (storage) AeThreadFunctor2<Broc::entity, Broc::string>(RespawnPlayer, guy, team);
    team.~string();
    return result;
}
AeThreadFunctor* LocalPlayerRespawn__functor(Broc::entity player) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(LocalPlayerRespawn, player);
}
AeThreadFunctor* PunishedForTeamKill__functor(Broc::entity ent,
                                              Broc::bbool punished) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::bbool>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::bbool>(PunishedForTeamKill, ent, punished);
}
AeThreadFunctor* reenable_medic_call__functor(Broc::entity self,
                                              Broc::bint time) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::bint>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::bint>(reenable_medic_call, self, time);
}
void* HandleJoinAfterRoundOver__functor(Broc::entity self, Broc::bint timeleft) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::bint>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::bint>(HandleJoinAfterRoundOver, self, timeleft);
}
AeThreadFunctor* TeamChangeKillPlayer__functor(Broc::entity player) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(TeamChangeKillPlayer, player);
}
AeThreadFunctor* FadeUpWhenLoaded__functor(Broc::entity self, Broc::entity player) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::entity>(FadeUpWhenLoaded, self, player);
}
AeThreadFunctor* HealthRegenPlayerBreathing__functor(Broc::entity self, Broc::bint healthCap) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::bint>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::bint>(HealthRegenPlayerBreathing, self, healthCap);
}
AeThreadFunctor* DeathState__functor(Broc::entity player, Broc::entity team_killer,
                                     Broc::bint delay, Broc::bbool reviveable,
                                     Broc::bbool fade) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor5<Broc::entity, Broc::entity, Broc::bint, Broc::bbool, Broc::bbool>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor5<Broc::entity, Broc::entity, Broc::bint, Broc::bbool, Broc::bbool>(DeathState, player, team_killer, delay, reviveable, fade);
}
AeThreadFunctor* UpdateSpectateCritical__functor(Broc::entity guy) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(UpdateSpectateCritical, guy);
}
AeThreadFunctor1<Broc::entity>* UpdateSpectateCriticalGoingToDie__functor(
    Broc::entity guy) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(UpdateSpectateCriticalGoingToDie, guy);
}
AeThreadFunctor* UpdateSpectateDead__functor(Broc::entity guy, Broc::bbool canspawn) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::bbool>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::bbool>(UpdateSpectateDead, guy, canspawn);
}
AeThreadFunctor1<Broc::entity>* UpdateSpectateSpawn__functor(Broc::entity localPlayer) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(UpdateSpectateSpawn, localPlayer);
}
AeThreadFunctor1<Broc::entity>* SpawnLocalSpectator__functor(Broc::entity guy) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(SpawnLocalSpectator, guy);
}
AeThreadFunctor* restart_round__functor(Broc::entity selfLevel, Broc::bint waitTime) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::bint>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::bint>(restart_round, selfLevel, waitTime);
}
AeThreadFunctor* finish_starting_round__functor(Broc::entity self, Broc::bbool firstTime) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::bbool>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::bbool>(finish_starting_round, self, firstTime);
}
AeThreadFunctor* AddArtilleryObjective__functor(Broc::entity self,
                                                Broc::vector position) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::vector>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::vector>(AddArtilleryObjective, self, position);
}
AeThreadFunctor* NewHost__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(NewHost, self);
}
AeThreadFunctor1<Broc::entity>* LocalPlayerIntermission__functor(Broc::entity player) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(LocalPlayerIntermission, player);
}
AeThreadFunctor1<Broc::entity>* RunFrame__functor(Broc::entity selfLevel) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(RunFrame, selfLevel);
}
}

// ============================================================================
// _mp_minefield.
// ============================================================================
namespace _mp_minefield {
void* main__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(main, self);
}

// main - ea: 0x963EE0
void main(Broc::entity self) {
    (void)self;
    Broc::dyn_array<Broc::entity> minefields;
    Broc::string val("minefield");
    HashStr key;
    key.mVal = 0x19F9F0E8u;
    Broc::GetEntArray(&val, key.mVal, &minefields, 0);
    val.~string();
    if (Broc::size(minefields) <= 0) {
        minefields.~dyn_array();
        return;
    }
    HashStr effectKey;
    effectKey.mVal = 0x53DA8129u;
    mp_util_wad::level_effect_set(effectKey, "artillery_generic");
    Broc::bint i(0);
    while ((int)i < Broc::size(minefields)) {
        void* ftor = minefield_think__functor(minefields[(unsigned int)(int)i]);
        Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_minefield.bro",
                            __LINE__, "minefield_think", ftor);
        i = (int)i + 1;
    }
    minefields.~dyn_array();
}

void* minefield_think__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(minefield_think, self);
}

// minefield_think - ea: 0x964150
void minefield_think(Broc::entity self) {
    Broc::entity target;
    target.___u0 = 0;
    for (;;) {
        do {
            HashStr label;
            label.mVal = 0xF2F5EAB4;
            Broc::waittill(self, label, &target);
        } while (Broc::IsSentient(&target) == 0 &&
                 Broc::IsVehicle(target) == 0);
        if (!(bool)*mp_util_wad::GetEE_flag_in_minefield(target)) {
            void* ftor = minefield_kill__functor(self, target);
            Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_minefield.bro",
                                __LINE__, "minefield_kill", ftor);
        }
    }
}

void* minefield_kill__functor(Broc::entity self, Broc::entity trigger) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor2<Broc::entity, Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor2<Broc::entity, Broc::entity>(minefield_kill, self, trigger);
}

// minefield_kill - ea: 0x964330
void minefield_kill(Broc::entity self, Broc::entity trigger) {
    *mp_util_wad::GetEE_flag_in_minefield(self) = true;
    Broc::string script("minefield_click");
    Broc::EffectEventPlay(&self, &script);
    script.~string();
    float delay = Broc::RandomFloat(0.5f) + 0.5f;
    Broc::wait(delay);
    if (Broc::IsTouching(&self, &trigger)) {
        float zero = 0.0f;
        Broc::bint health;
        mp_util_wad::entity_get_health(&health, self);
        if ((int)health > 0) {
            Broc::vector origin;
            mp_util_wad::entity_get_origin(&origin, self);
            origin.z += 10.0f;
            Broc::bfloat range(300.0f);
            Broc::bfloat maxdamage(2500.0f);
            Broc::bfloat mindamage(50.0f);
            HashStr effectKey;
            effectKey.mVal = 0x53DA8129u;
            Broc::string effect;
            mp_util_wad::level_effect_get(&effect, effectKey);
            Broc::EffectEventPlay(&self, &effect);
            effect.~string();
            Broc::bint h2;
            mp_util_wad::entity_get_health(&h2, self);
            float max_damage = (float)((int)h2 + 100);
            Broc::RadiusDamage(&origin, (float)range, max_damage,
                               (float)mindamage, 27);
        }
        (void)zero;
    }
    *mp_util_wad::GetEE_flag_in_minefield(self) = false;
}
}

// ============================================================================
// _mp_tdm.
// ============================================================================
namespace _mp_tdm {
void* main__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(main, self);
}

// main - ea: 0x973280
void main(Broc::entity self) {
    Broc::bbool team_game(true);
    Broc::Code_SetTeamGame((bool)team_game);
    mp_util_wad::pLevel->spawnTypeAllies = "spawn_teamdeathmatch";
    mp_util_wad::pLevel->spawnTypeAxis = "spawn_teamdeathmatch";
    _mp_common::SetupCallbacks(team_game);
    Broc::gBrocAPI.mBrocExports.mCallbackPlayerKilled =
        (void (*)(const Broc::entity, const Broc::entity, const Broc::entity,
                  int, int, int))CallbackPlayerKilled;
    void* started = StartGame__functor(self);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_tdm.bro",
                        __LINE__, "StartGame", started);
}

void* StartGame__functor(Broc::entity self) {
    void* storage = AeThreadFunctor::operator new(sizeof(AeThreadFunctor1<Broc::entity>));
    if (storage == NULL)
        return NULL;
    return ::new (storage) AeThreadFunctor1<Broc::entity>(StartGame, self);
}

// CallbackPlayerKilled - ea: 0x9733F0
void CallbackPlayerKilled(Broc::entity player, Broc::entity inflictor,
                          Broc::entity attacker, int weapon, int mod,
                          int health) {
    _mp_common::CallbackPlayerKilled(player, inflictor, attacker, weapon,
                                     mod, health);
    bool enemyKill = false;
    if (Broc::IsDefined(attacker) && attacker != player) {
        Broc::string pteam;
        Broc::string ateam;
        mp_util_wad::entity_get_team(&pteam, player);
        mp_util_wad::entity_get_team(&ateam, attacker);
        enemyKill = ateam != pteam;
        pteam.~string();
        ateam.~string();
    }
    if (enemyKill) {
        Broc::string ateam;
        mp_util_wad::entity_get_team(&ateam, attacker);
        Broc::Code_IncTeamScore(ateam, 1);
        ateam.~string();
    }
}

// StartGame - ea: 0x9735E0
void StartGame(Broc::entity self) {
    (void)self;
    Broc::wait(0.5f);
    _mp_common::StartRound(Broc::bbool(true));
    Broc::entity lvl = mp_util_wad::pLevel != nullptr
                           ? mp_util_wad::pLevel->_base.entity
                           : Broc::entity();
    void* ftor = _mp_common::RunFrame__functor(lvl);
    Broc::thread_create(false, "c:\\cod\\code\\script\\_mp_tdm.bro",
                        __LINE__, "_mp_common::RunFrame", ftor);
    Broc::Code_EnterGame();
}
}
