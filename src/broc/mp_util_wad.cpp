// ============================================================================
// mp_util_wad.cpp - multiplayer Broc entity-attribute accessors + anim wrappers.
// Source: mp_util_wad.cpp (MPBrocCore_xboxd:mp_util_wad.o)
// Verified against IDA (MPBrocCore_xboxd:mp_util_wad.o).
// ============================================================================

#include "mp_util_wad.h"
#include "engine/broc_types.h"

namespace mp_anim_wad {
int ResolveAnim(unsigned int treename, unsigned int animname,
                unsigned int* getVal, unsigned int setVal);
const char* ResolveAnimName(unsigned int anim);
bool ValidateAnimationIndices();
unsigned int GetBroAnim(unsigned int treename, unsigned int animname);
void RegisterHashStrings();
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