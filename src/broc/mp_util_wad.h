// ============================================================================
// mp_util_wad.h - multiplayer Broc entity-attribute accessors + anim wrappers.
// Source: mp_util_wad.cpp (MPBrocCore_xboxd:mp_util_wad.o)
// ============================================================================

#ifndef COD3_BROC_MP_UTIL_WAD_H
#define COD3_BROC_MP_UTIL_WAD_H

#include "engine/broc_types.h"

namespace mp_util_wad {

// ============================================================================
// mp_util_wad::Level - shared multiplayer level state (560 bytes).
// Verified against IDA ordinal 8663.
// ============================================================================
struct Level {
    char     _base[0x0C];                       // +0x00 (mp_anim_wad::Level base)
    unsigned char flags;                        // +0x0C
    Broc::bint last_HQ_Point_key;               // +0x10
    char     line_sound_emitters[0x28];         // +0x14 (std::hash_map, 40B)
    Broc::string spawnTypeAllies;               // +0x3C
    Broc::string audio_current_ambpack;         // +0x40
    Broc::string ambient_setting;               // +0x44
    void*    PickSpawnPoint;                    // +0x48
    Broc::string audio_change_track;            // +0x4C
    Broc::bbool onlyDisplayScoreOnFinalRound;   // +0x50
    Broc::bbool forceMapChange;                 // +0x51
    Broc::bint ArtilleryObjectiveIndex;         // +0x54
    char     _effect[0x28];                     // +0x58 (std::hash_map, 40B)
    Broc::bint startTime;                       // +0x80
    Broc::string lastManStandingIgnore;         // +0x84
    Broc::entity base_allies;                   // +0x88
    Broc::bint nextRoundStartTime;              // +0x8C
    Broc::string gametype;                      // +0x90
    Broc::bint radioTriggerTime;                // +0x94
    Broc::bint hq_stage_time;                   // +0x98
    Broc::string spawnTypeAxis;                 // +0x9C
    Broc::string audio_current_reverb;          // +0xA0
    Broc::bint roundCount;                      // +0xA4
    Broc::string roundWinner;                   // +0xA8
    Broc::bint isInFadeUpWhenLoaded;            // +0xAC
    Broc::bint lastFlagIndex;                   // +0xB0
    Broc::entity base_axis;                     // +0xB4
    Broc::string audio_current_track;           // +0xB8
    Broc::bint audio_change_priority;           // +0xBC
    Broc::bint playersLeavingDuringRound;       // +0xC0
    Broc::dyn_array<Broc::entity> HQpoints;     // +0xC4
    Broc::bfloat audio_ambient_min;             // +0xD0
    Broc::string alliesViewModel;               // +0xD4
    Broc::string reverb_setting;                // +0xD8
    Broc::bfloat audio_current_ambient_min;     // +0xDC
    Broc::bint warIndex;                        // +0xE0
    Broc::vector spawnColorAxis;                // +0xE4
    Broc::bint noCapTime;                       // +0xF0
    Broc::bint triggerIndex;                    // +0xF4
    Broc::bint crossfade_done;                  // +0xF8
    Broc::bint axis_capped;                     // +0xFC
    Broc::vector last_allies;                   // +0x100
    Broc::entity axis_flag_ent;                 // +0x10C
    Broc::bint string_time;                     // +0x110
    Broc::bint timeLimit;                       // +0x114
    Broc::bfloat audio_change_ambient_min;      // +0x118
    Broc::bint playerCountAtStartOfRound;       // +0x11C
    Broc::bint capCount;                        // +0x120
    Broc::bbool lastManStanding;                // +0x124
    Broc::bint last_radio_trigger_time;         // +0x128
    Broc::vector pointA;                        // +0x12C
    Broc::dyn_array<Broc::entity> warAreas;     // +0x138
    Broc::string teamCantRespawn;               // +0x144
    Broc::string audio_change_ambpack;          // +0x148
    Broc::bfloat audio_current_ambient_wait;    // +0x14C
    Broc::bint scoreLimit;                      // +0x150
    Broc::string AxisFlagModel;                 // +0x154
    Broc::string axis;                          // +0x158
    Broc::string axisViewModel;                 // +0x15C
    void*    RenderSpawnPoints;                 // +0x160
    Broc::bint audio_current_priority;          // +0x164
    int      roundEndMusic;                     // +0x168
    Broc::bint lastSpawnPointIndex;             // +0x16C
    Broc::dyn_array<Broc::string> models;       // +0x170
    Broc::bint allies_capped;                   // +0x17C
    Broc::bint current_flag;                    // +0x180
    Broc::bbool friendlyFire;                   // +0x184
    Broc::string allies;                        // +0x188
    Broc::string background_track;              // +0x18C
    Broc::hudelem progress_bar;                 // +0x190
    Broc::bbool rankOn;                         // +0x194
    Broc::dyn_array<Broc::entity> scfFlags;     // +0x198
    Broc::bbool pointA_isHQ;                    // +0x1A4
    Broc::bbool teamBalance;                    // +0x1A5
    Broc::string AlliesFlagModel;               // +0x1A8
    Broc::bfloat audio_ambient_max;             // +0x1AC
    Broc::bbool mustHaveBothTeamsToStart;       // +0x1B0
    Broc::bint pointsTimer;                     // +0x1B4
    Broc::entity trigger;                       // +0x1B8
    Broc::bint showFlagHint;                    // +0x1BC
    Broc::bint respawnTime;                     // +0x1C0
    Broc::bbool allies_defending;               // +0x1C4
    Broc::bint hq_stage;                        // +0x1C8
    Broc::bint roundLimit;                      // +0x1CC
    Broc::bint IncomingVO;                      // +0x1D0
    Broc::bint blinker;                         // +0x1D4
    Broc::bbool roundStarted;                   // +0x1D8
    Broc::bint pickupCaptureDelayTime;          // +0x1DC
    Broc::entity hack_sound_entity;             // +0x1E0
    Broc::bbool roundOver;                      // +0x1E4
    Broc::vector spawnColorAllies;              // +0x1E8
    Broc::vector last_axis;                     // +0x1F4
    Broc::bfloat audio_change_ambient_wait;     // +0x200
    Broc::string audio_change_reverb;           // +0x204
    Broc::bint audio_indoor_switch;             // +0x208
    Broc::string BackgroundMusicTrack;          // +0x20C
    Broc::bint audio_current_track_handle;      // +0x210
    Broc::hudelem clock;                        // +0x214
    Broc::bint FlagMusic;                       // +0x218
    Broc::bint spawnType;                       // +0x21C
    Broc::vector pointB;                        // +0x220
    Broc::entity allies_flag_ent;               // +0x22C
};
static_assert(sizeof(Level) == 0x230, "mp_util_wad::Level size mismatch");

extern Level* pLevel;  // ?pLevel@mp_util_wad@@3PAULevel@1@A @0x10F1EA0

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
MP_UTIL_WAD_ACCESSOR(script_sound, Broc::string)
MP_UTIL_WAD_ACCESSOR(teamSound, Broc::bint)
MP_UTIL_WAD_ACCESSOR(vehicletype, Broc::string)

#undef MP_UTIL_WAD_ACCESSOR

// ============================================================================
// Entity property helpers (player.rank / player.team / ... — inline wrappers
// around gBrocAPI m_entity_* members; mp_util_wad.o inline COMDATs).
// ============================================================================
__int16 entity_get_rank(Broc::entity ent);               // ea: 0x940820
void entity_set_rank(Broc::entity ent, __int16 rank);    // ea: 0x9489F0
Broc::string* entity_get_team(Broc::string* result, Broc::entity ent);  // ea: 0x938DE0
void entity_set_team(Broc::entity ent, const Broc::string& team);
Broc::bint* entity_get_playerState(Broc::bint* result, Broc::entity ent);  // ea: 0x93BDF0
void entity_set_playerState(Broc::entity ent, int state);  // ea: 0x93E450
__int16 entity_get_nextPlayerClass(Broc::entity ent);   // ea: 0x945FB0
void entity_set_nextPlayerClass(Broc::entity ent, __int16 cls);  // ea: 0x93E410
Broc::bint* entity_get_maxhealth(Broc::bint* result, Broc::entity ent);  // ea: 0x93ADE0
Broc::bint* entity_get_health(Broc::bint* result, Broc::entity ent);  // ea: 0x940E10
void entity_set_health(Broc::entity ent, int health);   // ea: 0x940250
__int16 entity_get_playerClass(Broc::entity ent);       // ea: 0x940850
void entity_set_playerClass(Broc::entity ent, __int16 cls);  // ea: 0x93E3D0
Broc::vector* entity_get_angles(Broc::vector* result, Broc::entity ent);  // ea: 0x93E340
void entity_set_angles(Broc::entity ent, const Broc::vector& v);  // ea: 0x939430
Broc::vector* entity_get_viewangles(Broc::vector* result, Broc::entity ent);  // ea: 0x9491B0
Broc::string* entity_get_target(Broc::string* result, Broc::entity ent);  // ea: 0x9388C0
void entity_set_spectatorClient(Broc::entity ent, int client);  // ea: 0x93E390
Broc::vector* entity_get_origin(Broc::vector* result, Broc::entity ent);
void entity_set_origin(Broc::entity ent, const Broc::vector& v);  // ea: 0x938A10
Broc::string* entity_get_targetname(Broc::string* result, Broc::entity ent);

// mp_util_wad::Level.line_sound_emitters hash_map helpers.
void line_sound_set(HashStr key, Broc::entity e);
Broc::entity* line_sound_get(Broc::entity* result, HashStr key);
void line_sound_erase(HashStr key);
void line_sound_delete_all();

} // namespace mp_util_wad

#endif // COD3_BROC_MP_UTIL_WAD_H
