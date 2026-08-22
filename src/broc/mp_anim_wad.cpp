// ============================================================================
// mp_anim_wad.cpp - animation wad: hash strings + entity-attr accessors + anim wrappers.
// Source: mp_anim_wad.cpp (MPBrocCore_xboxd:mp_anim_wad.o)
// Verified against IDA (MPBrocCore_xboxd:mp_anim_wad.o).
// ============================================================================

#include "mp_anim_wad.h"
#include "engine/broc_types.h"
#include "game/logic/g_local.h"
#include <new>

// IDA types: BroAnim is a 4-byte value wrapper and AnimRef is the 12-byte
// {mAnim, mTreeNameHash, mVarNameHash} record used by generic_human.
class BroAnim {
public:
    BroAnim() : mVal(0) {}
    explicit BroAnim(unsigned int val) : mVal(val) {}
    operator unsigned int() const { return mVal; }

    unsigned int mVal;
};

namespace Broc {
class AnimRef {
public:
    AnimRef() : mAnim(), mTreeNameHash(0), mVarNameHash(0) {}
    AnimRef(int treename, int varname);
    unsigned int GetAnim() const;
    void SetAnim(unsigned int anim);
    int IsUnresolved() const;

    BroAnim mAnim;
    int mTreeNameHash;
    int mVarNameHash;
};

AnimRef::AnimRef(int treename, int varname)
    : mAnim(0xFFFFFFFFu), mTreeNameHash(treename), mVarNameHash(varname)
{
}

unsigned int AnimRef::GetAnim() const
{
    return static_cast<unsigned int>(mAnim);
}

void AnimRef::SetAnim(unsigned int anim)
{
    mAnim = BroAnim(anim);
}

int AnimRef::IsUnresolved() const
{
    return static_cast<unsigned int>(mAnim) == 0xFFFFFFFFu;
}
} // namespace Broc

// generic_human anim ref (global data at 0x10F1F84, zero initialized).
namespace generic_human {
    Broc::AnimRef c_jeep_gunner_idle;

    unsigned int ResolveAnim(unsigned int animhash, unsigned int* getVal,
                             unsigned int setVal)
    {
        if (animhash != 0xD15054F1u)
            return 0;
        if (getVal != nullptr)
            *getVal = c_jeep_gunner_idle.GetAnim();
        else
            c_jeep_gunner_idle.SetAnim(setVal);
        return c_jeep_gunner_idle.GetAnim();
    }

    const char* ResolveAnimName(unsigned int anim)
    {
        return anim == c_jeep_gunner_idle.GetAnim() ? "c_jeep_gunner_idle" : nullptr;
    }

    bool ValidateAnimationIndices()
    {
        return c_jeep_gunner_idle.IsUnresolved() != 0;
    }
}

// GetEE_* / IsEEDefined_* accessors (generated from RegisterHashStrings + manifest).

namespace mp_anim_wad {

extern void RegisterHashString(int h, const char* txt);

void RegisterHashStrings() {
    RegisterHashString(959135275, "accuracy");
    RegisterHashString(1004996222, "accuracyVsAI");
    RegisterHashString(-775523646, "accuracyVsHero");
    RegisterHashString(1884094817, "accuracyVsPlayer");
    RegisterHashString(1902049817, "accuracystationarymod");
    RegisterHashString(-710454267, "allowdeath");
    RegisterHashString(809239328, "ambient");
    RegisterHashString(1061940091, "anim_pose");
    RegisterHashString(-1789404698, "animname");
    RegisterHashString(2029520170, "animscriptedallowpain");
    RegisterHashString(-1999301637, "bravery");
    RegisterHashString(1299164282, "bulletsInClip");
    RegisterHashString(-783264527, "c_jeep_gunner_idle");
    RegisterHashString(593499891, "chainfallback");
    RegisterHashString(782343305, "chainnode");
    RegisterHashString(-145308041, "classname");
    RegisterHashString(-1383885977, "closespeed");
    RegisterHashString(121526345, "count");
    RegisterHashString(1761853315, "crash_path_target");
    RegisterHashString(1799659569, "ctf_has_flag");
    RegisterHashString(-1243675042, "damagedir");
    RegisterHashString(662902136, "damagelocation");
    RegisterHashString(-1428739438, "damagetaken");
    RegisterHashString(1908989025, "damagetype");
    RegisterHashString(-1243652432, "damageyaw");
    RegisterHashString(-771029036, "defaultsightlatency");
    RegisterHashString(81845535, "degrees");
    RegisterHashString(122342671, "delay");
    RegisterHashString(-1183584966, "derailed");
    RegisterHashString(-657094137, "desiredangle");
    RegisterHashString(-748799818, "destructible_id");
    RegisterHashString(-1088273508, "detoured");
    RegisterHashString(281519456, "detourpath");
    RegisterHashString(704427777, "detourstart");
    RegisterHashString(-1364443819, "dontavoidplayer");
    RegisterHashString(-1235188682, "dontdrawoncompass");
    RegisterHashString(-1887043232, "dontdropgrenade");
    RegisterHashString(-1268682252, "dontdropweapon");
    RegisterHashString(208505185, "drawoncompass");
    RegisterHashString(-1367240961, "dropweapon");
    RegisterHashString(-1610089431, "endswitch");
    RegisterHashString(123844798, "enemy");
    RegisterHashString(-1144695353, "eventwait_used");
    RegisterHashString(779862645, "eventwaiting");
    RegisterHashString(1471862521, "followmax");
    RegisterHashString(1471862775, "followmin");
    RegisterHashString(1868106601, "forced_pose");
    RegisterHashString(1149274092, "fovcosine");
    RegisterHashString(477421618, "friendlywait");
    RegisterHashString(-1784152939, "generic_human");
    RegisterHashString(-1151091065, "goalangletolerance");
    RegisterHashString(834188043, "goalradius");
    RegisterHashString(-169869075, "goalradiusonly");
    RegisterHashString(167522262, "grenade");
    RegisterHashString(164837824, "grenadeammo");
    RegisterHashString(-1999242017, "grenadeawareness");
    RegisterHashString(1663026360, "grenadereturnchance");
    RegisterHashString(-29171200, "grenadeweapon");
    RegisterHashString(-1929977330, "groupname");
    RegisterHashString(-101500970, "health");
    RegisterHashString(-396421546, "ignoreme");
    RegisterHashString(2088740332, "ignorepain");
    RegisterHashString(-590048114, "interactstage");
    RegisterHashString(276890917, "interval");
    RegisterHashString(1572850391, "keepOldDesiredChainNodeOdds");
    RegisterHashString(789130826, "lastscriptstate");
    RegisterHashString(-65206486, "lookforward");
    RegisterHashString(-143815117, "lookright");
    RegisterHashString(67401882, "lookup");
    RegisterHashString(757954812, "maxhealth");
    RegisterHashString(-1738944429, "maxsightdistsqrd");
    RegisterHashString(1937815420, "maxthreatdistsqrd");
    RegisterHashString(-520397964, "mg42stayput");
    RegisterHashString(133366737, "model");
    RegisterHashString(-416545575, "modelscale");
    RegisterHashString(857155366, "moveAwayAvoidPoint");
    RegisterHashString(-1256761123, "moveAwayDist");
    RegisterHashString(923143810, "nextPlayerClass");
    RegisterHashString(-123998717, "noshadow");
    RegisterHashString(802053947, "offramp_used");
    RegisterHashString(1983031123, "pacifist");
    RegisterHashString(323945608, "pacifistwait");
    RegisterHashString(-570160920, "persistent_index");
    RegisterHashString(1929158608, "personalspace");
    RegisterHashString(1261135011, "playerClass");
    RegisterHashString(1280397262, "playerState");
    RegisterHashString(790769767, "playerhasbeenhere");
    RegisterHashString(-1082303970, "proneok");
    RegisterHashString(285213864, "radius");
    RegisterHashString(285567585, "random");
    RegisterHashString(302382991, "rotate");
    RegisterHashString(-305967413, "scariness");
    RegisterHashString(1338137631, "script_accuracy");
    RegisterHashString(85206157, "script_accuracyStationaryMod");
    RegisterHashString(-469500302, "script_accuracyvsai");
    RegisterHashString(1296880469, "script_accuracyvsplayer");
    RegisterHashString(798111724, "script_additive_delay");
    RegisterHashString(22258246, "script_aitargetname");
    RegisterHashString(-972428271, "script_ambush_trigger_distance");
    RegisterHashString(-1812438827, "script_ambush_type");
    RegisterHashString(-1410402342, "script_animname");
    RegisterHashString(36005645, "script_area");
    RegisterHashString(1784372508, "script_balcony");
    RegisterHashString(574979888, "script_battle");
    RegisterHashString(-2017742405, "script_bloom_endindex");
    RegisterHashString(1465015442, "script_bloom_startindex");
    RegisterHashString(-1616373573, "script_bloom_time");
    RegisterHashString(-1857666193, "script_bravery");
    RegisterHashString(-79293189, "script_breathpuff");
    RegisterHashString(-682768631, "script_burst_max");
    RegisterHashString(-682768377, "script_burst_min");
    RegisterHashString(1190194775, "script_chain");
    RegisterHashString(-1082356191, "script_chargegoal");
    RegisterHashString(1391793838, "script_colorid_endindex");
    RegisterHashString(-696726971, "script_colorid_startindex");
    RegisterHashString(1476852782, "script_colorid_time");
    RegisterHashString(652978259, "script_damage");
    RegisterHashString(1191284611, "script_delay");
    RegisterHashString(78220712, "script_delay_max");
    RegisterHashString(78220966, "script_delay_min");
    RegisterHashString(-1008324544, "script_delayed_playerseek");
    RegisterHashString(657690791, "script_delete");
    RegisterHashString(-2059408938, "script_dontdeploy");
    RegisterHashString(36110536, "script_door");
    RegisterHashString(-1621346355, "script_effect_id");
    RegisterHashString(1413540660, "script_eventhandler");
    RegisterHashString(1862223607, "script_exploder");
    RegisterHashString(-622225415, "script_explodertype");
    RegisterHashString(-1410759342, "script_favoriteenemy");
    RegisterHashString(1193534728, "script_fb_id");
    RegisterHashString(59077314, "script_fb_max_onscreen_enemies");
    RegisterHashString(-1893629234, "script_fb_max_respawn_time");
    RegisterHashString(1031491456, "script_fb_min_onscreen_enemies");
    RegisterHashString(1490727052, "script_fb_min_respawn_time");
    RegisterHashString(979771418, "script_fb_required_kills");
    RegisterHashString(1462441453, "script_fb_respawn_chunk_size");
    RegisterHashString(-361145084, "script_fb_retreat_chunk_size");
    RegisterHashString(740920440, "script_firefx");
    RegisterHashString(1223114925, "script_fixbasepose");
    RegisterHashString(-1669734028, "script_flaktype");
    RegisterHashString(-312668646, "script_flashlight");
    RegisterHashString(1193911274, "script_float");
    RegisterHashString(1094038381, "script_followmax");
    RegisterHashString(1094038635, "script_followmin");
    RegisterHashString(-997074492, "script_friendlywave");
    RegisterHashString(-501752499, "script_friendname");
    RegisterHashString(36191999, "script_fxid");
    RegisterHashString(-730678912, "script_fxstart");
    RegisterHashString(758761816, "script_fxstop");
    RegisterHashString(1708510931, "script_grenadeawareness");
    RegisterHashString(1612269821, "script_grenades");
    RegisterHashString(-39386848, "script_hdi_index");
    RegisterHashString(519798711, "script_hdi_time");
    RegisterHashString(-26511671, "script_hdi_time2");
    RegisterHashString(813844682, "script_health");
    RegisterHashString(-1554567666, "script_health_once");
    RegisterHashString(818686976, "script_hidden");
    RegisterHashString(410076522, "script_idnumber");
    RegisterHashString(-17419190, "script_ignoreme");
    RegisterHashString(-1300405569, "script_int");
    RegisterHashString(-1703469790, "script_kill_chain");
    RegisterHashString(180881920, "script_killspawner");
    RegisterHashString(-183211362, "script_landmark");
    RegisterHashString(1182581119, "script_lmgaccuracy");
    RegisterHashString(283684109, "script_location");
    RegisterHashString(36423246, "script_mg42");
    RegisterHashString(609840935, "script_mg42auto");
    RegisterHashString(345973992, "script_mg42stayput");
    RegisterHashString(986190300, "script_min_friendlies");
    RegisterHashString(78731947, "script_moveoverride");
    RegisterHashString(-1737189029, "script_multiplier");
    RegisterHashString(-1044240128, "script_new_exploder");
    RegisterHashString(-2012813910, "script_noautoreenforcements");
    RegisterHashString(-510446089, "script_noteworthy");
    RegisterHashString(-1357222513, "script_objective");
    RegisterHashString(1247733527, "script_offradius");
    RegisterHashString(1582752702, "script_offtime");
    RegisterHashString(-1932933817, "script_pacifist");
    RegisterHashString(1122666084, "script_panzer");
    RegisterHashString(1480804105, "script_patroller");
    RegisterHashString(-1920243666, "script_personality");
    RegisterHashString(-585715223, "script_playerseek");
    RegisterHashString(1394260594, "script_prespawn_delay");
    RegisterHashString(1200559516, "script_radius");
    RegisterHashString(1200913237, "script_random");
    RegisterHashString(1027864432, "script_requires_player");
    RegisterHashString(1208989916, "script_scale");
    RegisterHashString(-1975034689, "script_seekgoal");
    RegisterHashString(463581152, "script_sightrange");
    RegisterHashString(1209443005, "script_sound");
    RegisterHashString(1209461221, "script_speed");
    RegisterHashString(-1637724269, "script_squadname");
    RegisterHashString(1251877890, "script_squadnum");
    RegisterHashString(366592294, "script_stalingradspawn");
    RegisterHashString(-1167897098, "script_startinghealth");
    RegisterHashString(1262730283, "script_string");
    RegisterHashString(-2084171521, "script_suppression");
    RegisterHashString(-1520050623, "script_tankmgaccuracy");
    RegisterHashString(36674171, "script_team");
    RegisterHashString(1210404309, "script_timer");
    RegisterHashString(-967461248, "script_turretweaponpak");
    RegisterHashString(-119166932, "script_uniquename");
    RegisterHashString(1248944443, "script_usemg42");
    RegisterHashString(190519009, "script_vehiclegroup");
    RegisterHashString(36777897, "script_wait");
    RegisterHashString(291783966, "script_waittill");
    RegisterHashString(397942103, "script_walkdist");
    RegisterHashString(60388854, "scriptstate");
    RegisterHashString(1582876018, "secondaryweapon");
    RegisterHashString(140227858, "shard");
    RegisterHashString(478061846, "spawnflags");
    RegisterHashString(-115547144, "spawnitem");
    RegisterHashString(-386372556, "spectatorClient");
    RegisterHashString(140519281, "speed");
    RegisterHashString(204506848, "startswitch");
    RegisterHashString(-1036667719, "state_change_blocked");
    RegisterHashString(-1218643921, "statechangereason");
    RegisterHashString(-1610350400, "suppressionwait");
    RegisterHashString(109780452, "takedamage");
    RegisterHashString(363985063, "target");
    RegisterHashString(435810536, "targetname");
    RegisterHashString(1455767240, "teamname");
    RegisterHashString(-1508804633, "threatbias");
    RegisterHashString(-2098354766, "transparent");
    RegisterHashString(1425343827, "updateDesireChaineNodeMax");
    RegisterHashString(1425344081, "updateDesireChaineNodeMin");
    RegisterHashString(1106874209, "useable");
    RegisterHashString(1854293024, "vehicle");
    RegisterHashString(1655552556, "vehicle_seat");
    RegisterHashString(-527452567, "vehicle_seat_enter");
    RegisterHashString(-355102774, "vehicle_sub_type");
    RegisterHashString(311230882, "vehicletype");
    RegisterHashString(-1938179723, "viewangles");
    RegisterHashString(-645163371, "visibilitythreshold");
    RegisterHashString(144045398, "voice");
    RegisterHashString(18939747, "walkdist");
    RegisterHashString(485534122, "weapon");
    RegisterHashString(324854774, "weaponinfo");
}

unsigned int ResolveAnim(unsigned int treename, unsigned int animname,
                         unsigned int* getVal, unsigned int setVal) {
    if (treename == 0x957A3A95)  // -1784152939
        return generic_human::ResolveAnim(animname, getVal, setVal);
    return 0;
}

const char* ResolveAnimName(unsigned int anim) {
    return generic_human::ResolveAnimName(anim);
}

bool ValidateAnimationIndices() {
    return generic_human::ValidateAnimationIndices();
}

unsigned int GetBroAnim(unsigned int treename, unsigned int animname) {
    unsigned int getval = 0;
    if (treename == 0x957A3A95)
        return (unsigned int)generic_human::ResolveAnim(animname, &getval, 0);
    return 0;
}

} // namespace mp_anim_wad

// _mp_setup - ea: 0x9983D0 / 0x9983F0
namespace _mp_setup {
void main() {
}

void hack_function_to_allow_the_script_to_compile() {
    extern int generic_human_c_jeep_gunner_idle;  // AnimRef global (data)
    extern void Broc_ValidateAnimRef(void* result, void* ref, const char* name, int line);
    extern void Broc_SetAnimKnob(Broc::entity* e, unsigned int anim, float gw, float gt, float rate);
    Broc::entity e;
    e.___u0 = 0;
    unsigned int result = 0;
    Broc_ValidateAnimRef(&result, &generic_human_c_jeep_gunner_idle,
                         "c_jeep_gunner_idle", 10);
    Broc_SetAnimKnob(&e, result, 1.0f, 0.1f, 1.0f);
}
}
// GetEE_script_delay / IsEEDefined_script_delay (key 0x47018F83)
Broc::bfloat* GetEE_script_delay(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x47018F83);
}

Broc::bbool* IsEEDefined_script_delay(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x47018F83);
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

// GetEE_dontdropweapon / IsEEDefined_dontdropweapon (key 0xB46171F4)
Broc::bint* GetEE_dontdropweapon(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xB46171F4);
}

Broc::bbool* IsEEDefined_dontdropweapon(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xB46171F4);
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

// GetEE_script_prespawn_delay / IsEEDefined_script_prespawn_delay (key 0x531ABA72)
Broc::bfloat* GetEE_script_prespawn_delay(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x531ABA72);
}

Broc::bbool* IsEEDefined_script_prespawn_delay(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x531ABA72);
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

// GetEE_script_burst_max / IsEEDefined_script_burst_max (key 0xD74DC709)
Broc::bfloat* GetEE_script_burst_max(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0xD74DC709);
}

Broc::bbool* IsEEDefined_script_burst_max(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0xD74DC709);
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

// GetEE_script_offradius / IsEEDefined_script_offradius (key 0x4A5EE717)
Broc::bint* GetEE_script_offradius(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x4A5EE717);
}

Broc::bbool* IsEEDefined_script_offradius(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x4A5EE717);
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

// GetEE_script_random / IsEEDefined_script_random (key 0x47947B55)
Broc::bint* GetEE_script_random(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x47947B55);
}

Broc::bbool* IsEEDefined_script_random(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x47947B55);
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

// GetEE_script_health / IsEEDefined_script_health (key 0x308248CA)
Broc::bint* GetEE_script_health(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x308248CA);
}

Broc::bbool* IsEEDefined_script_health(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x308248CA);
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

// GetEE_script_accuracyStationaryMod / IsEEDefined_script_accuracyStationaryMod (key 0x0514248D)
Broc::bfloat* GetEE_script_accuracyStationaryMod(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x0514248D);
}

Broc::bbool* IsEEDefined_script_accuracyStationaryMod(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x0514248D);
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

// GetEE_noshadow / IsEEDefined_noshadow (key 0xF89BEE03)
Broc::bint* GetEE_noshadow(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xF89BEE03);
}

Broc::bbool* IsEEDefined_noshadow(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xF89BEE03);
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

// GetEE_script_offtime / IsEEDefined_script_offtime (key 0x5E56E3BE)
Broc::bint* GetEE_script_offtime(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x5E56E3BE);
}

Broc::bbool* IsEEDefined_script_offtime(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x5E56E3BE);
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

// GetEE_script_playerseek / IsEEDefined_script_playerseek (key 0xDD16B1E9)
Broc::bint* GetEE_script_playerseek(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xDD16B1E9);
}

Broc::bbool* IsEEDefined_script_playerseek(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xDD16B1E9);
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

// GetEE_script_fixbasepose / IsEEDefined_script_fixbasepose (key 0x48E740AD)
Broc::bint* GetEE_script_fixbasepose(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x48E740AD);
}

Broc::bbool* IsEEDefined_script_fixbasepose(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x48E740AD);
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

// GetEE_script_moveoverride / IsEEDefined_script_moveoverride (key 0x04B15AAB)
Broc::bint* GetEE_script_moveoverride(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x04B15AAB);
}

Broc::bbool* IsEEDefined_script_moveoverride(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x04B15AAB);
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

// GetEE_script_stalingradspawn / IsEEDefined_script_stalingradspawn (key 0x15D9C126)
Broc::bint* GetEE_script_stalingradspawn(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x15D9C126);
}

Broc::bbool* IsEEDefined_script_stalingradspawn(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x15D9C126);
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

// GetEE_script_new_exploder / IsEEDefined_script_new_exploder (key 0xC1C22900)
Broc::bint* GetEE_script_new_exploder(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xC1C22900);
}

Broc::bbool* IsEEDefined_script_new_exploder(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xC1C22900);
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

// GetEE_script_pacifist / IsEEDefined_script_pacifist (key 0x8CC9C547)
Broc::bint* GetEE_script_pacifist(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x8CC9C547);
}

Broc::bbool* IsEEDefined_script_pacifist(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x8CC9C547);
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

// GetEE_script_fb_retreat_chunk_size / IsEEDefined_script_fb_retreat_chunk_size (key 0xEA795D04)
Broc::bint* GetEE_script_fb_retreat_chunk_size(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xEA795D04);
}

Broc::bbool* IsEEDefined_script_fb_retreat_chunk_size(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xEA795D04);
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

// GetEE_script_grenades / IsEEDefined_script_grenades (key 0x601948FD)
Broc::bint* GetEE_script_grenades(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x601948FD);
}

Broc::bbool* IsEEDefined_script_grenades(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x601948FD);
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

// GetEE_script_fb_id / IsEEDefined_script_fb_id (key 0x4723E508)
Broc::bint* GetEE_script_fb_id(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x4723E508);
}

Broc::bbool* IsEEDefined_script_fb_id(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x4723E508);
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

// GetEE_script_speed / IsEEDefined_script_speed (key 0x4816E9E5)
Broc::bint* GetEE_script_speed(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x4816E9E5);
}

Broc::bbool* IsEEDefined_script_speed(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x4816E9E5);
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

// GetEE_script_panzer / IsEEDefined_script_panzer (key 0x42EA8664)
Broc::bint* GetEE_script_panzer(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x42EA8664);
}

Broc::bbool* IsEEDefined_script_panzer(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x42EA8664);
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

// GetEE_script_tankmgaccuracy / IsEEDefined_script_tankmgaccuracy (key 0xA565DE41)
Broc::bint* GetEE_script_tankmgaccuracy(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xA565DE41);
}

Broc::bbool* IsEEDefined_script_tankmgaccuracy(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xA565DE41);
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

// GetEE_script_colorid_startindex / IsEEDefined_script_colorid_startindex (key 0xD678CA45)
Broc::bint* GetEE_script_colorid_startindex(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xD678CA45);
}

Broc::bbool* IsEEDefined_script_colorid_startindex(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xD678CA45);
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

// GetEE_script_burst_min / IsEEDefined_script_burst_min (key 0xD74DC807)
Broc::bfloat* GetEE_script_burst_min(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0xD74DC807);
}

Broc::bbool* IsEEDefined_script_burst_min(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0xD74DC807);
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

// GetEE_script_timer / IsEEDefined_script_timer (key 0x48254DD5)
Broc::bint* GetEE_script_timer(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x48254DD5);
}

Broc::bbool* IsEEDefined_script_timer(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x48254DD5);
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

// GetEE_script_noautoreenforcements / IsEEDefined_script_noautoreenforcements (key 0x8806E5AA)
Broc::bint* GetEE_script_noautoreenforcements(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x8806E5AA);
}

Broc::bbool* IsEEDefined_script_noautoreenforcements(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x8806E5AA);
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

// GetEE_script_exploder / IsEEDefined_script_exploder (key 0x6EFF46F7)
Broc::bint* GetEE_script_exploder(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x6EFF46F7);
}

Broc::bbool* IsEEDefined_script_exploder(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x6EFF46F7);
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

// GetEE_script_startinghealth / IsEEDefined_script_startinghealth (key 0xBA634DF6)
Broc::bint* GetEE_script_startinghealth(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xBA634DF6);
}

Broc::bbool* IsEEDefined_script_startinghealth(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xBA634DF6);
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

// GetEE_script_flashlight / IsEEDefined_script_flashlight (key 0xED5D0E1A)
Broc::bint* GetEE_script_flashlight(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xED5D0E1A);
}

Broc::bbool* IsEEDefined_script_flashlight(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xED5D0E1A);
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

// GetEE_script_delete / IsEEDefined_script_delete (key 0x273390A7)
Broc::bint* GetEE_script_delete(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x273390A7);
}

Broc::bbool* IsEEDefined_script_delete(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x273390A7);
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

// GetEE_script_balcony / IsEEDefined_script_balcony (key 0x6A5B5D1C)
Broc::bint* GetEE_script_balcony(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x6A5B5D1C);
}

Broc::bbool* IsEEDefined_script_balcony(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x6A5B5D1C);
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

// GetEE_script_vehiclegroup / IsEEDefined_script_vehiclegroup (key 0x0B5B16E1)
Broc::bint* GetEE_script_vehiclegroup(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x0B5B16E1);
}

Broc::bbool* IsEEDefined_script_vehiclegroup(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x0B5B16E1);
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

// GetEE_script_friendlywave / IsEEDefined_script_friendlywave (key 0xC491D9C4)
Broc::bint* GetEE_script_friendlywave(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xC491D9C4);
}

Broc::bbool* IsEEDefined_script_friendlywave(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xC491D9C4);
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

// GetEE_script_seekgoal / IsEEDefined_script_seekgoal (key 0x8A475CBF)
Broc::bint* GetEE_script_seekgoal(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x8A475CBF);
}

Broc::bbool* IsEEDefined_script_seekgoal(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x8A475CBF);
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

// GetEE_script_fb_min_respawn_time / IsEEDefined_script_fb_min_respawn_time (key 0x58DAB08C)
Broc::bfloat* GetEE_script_fb_min_respawn_time(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x58DAB08C);
}

Broc::bbool* IsEEDefined_script_fb_min_respawn_time(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x58DAB08C);
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

// GetEE_script_dontdeploy / IsEEDefined_script_dontdeploy (key 0x853FE9D6)
Broc::bint* GetEE_script_dontdeploy(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x853FE9D6);
}

Broc::bbool* IsEEDefined_script_dontdeploy(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x853FE9D6);
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

// GetEE_script_idnumber / IsEEDefined_script_idnumber (key 0x1871456A)
Broc::bint* GetEE_script_idnumber(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x1871456A);
}

Broc::bbool* IsEEDefined_script_idnumber(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x1871456A);
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

// GetEE_script_fb_max_respawn_time / IsEEDefined_script_fb_max_respawn_time (key 0x8F2182CE)
Broc::bfloat* GetEE_script_fb_max_respawn_time(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x8F2182CE);
}

Broc::bbool* IsEEDefined_script_fb_max_respawn_time(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x8F2182CE);
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

// GetEE_script_chargegoal / IsEEDefined_script_chargegoal (key 0xBF7C8E21)
Broc::bint* GetEE_script_chargegoal(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xBF7C8E21);
}

Broc::bbool* IsEEDefined_script_chargegoal(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xBF7C8E21);
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

// GetEE_script_float / IsEEDefined_script_float (key 0x4729A3EA)
Broc::bfloat* GetEE_script_float(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x4729A3EA);
}

Broc::bbool* IsEEDefined_script_float(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x4729A3EA);
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

// GetEE_script_breathpuff / IsEEDefined_script_breathpuff (key 0xFB4614FB)
Broc::bint* GetEE_script_breathpuff(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xFB4614FB);
}

Broc::bbool* IsEEDefined_script_breathpuff(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xFB4614FB);
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

// GetEE_script_hdi_time / IsEEDefined_script_hdi_time (key 0x1EFB7FB7)
Broc::bfloat* GetEE_script_hdi_time(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x1EFB7FB7);
}

Broc::bbool* IsEEDefined_script_hdi_time(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x1EFB7FB7);
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

// GetEE_script_fxstart / IsEEDefined_script_fxstart (key 0xD472B980)
Broc::bint* GetEE_script_fxstart(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xD472B980);
}

Broc::bbool* IsEEDefined_script_fxstart(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xD472B980);
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

// GetEE_script_delay_max / IsEEDefined_script_delay_max (key 0x04A98DA8)
Broc::bfloat* GetEE_script_delay_max(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x04A98DA8);
}

Broc::bbool* IsEEDefined_script_delay_max(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x04A98DA8);
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

// GetEE_script_int / IsEEDefined_script_int (key 0xB27D62BF)
Broc::bint* GetEE_script_int(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xB27D62BF);
}

Broc::bbool* IsEEDefined_script_int(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xB27D62BF);
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

// GetEE_script_requires_player / IsEEDefined_script_requires_player (key 0x3D43F770)
Broc::bint* GetEE_script_requires_player(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x3D43F770);
}

Broc::bbool* IsEEDefined_script_requires_player(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x3D43F770);
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

// GetEE_script_battle / IsEEDefined_script_battle (key 0x22457F30)
Broc::bint* GetEE_script_battle(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x22457F30);
}

Broc::bbool* IsEEDefined_script_battle(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x22457F30);
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

// GetEE_script_followmin / IsEEDefined_script_followmin (key 0x4135B46B)
Broc::bint* GetEE_script_followmin(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x4135B46B);
}

Broc::bbool* IsEEDefined_script_followmin(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x4135B46B);
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

// GetEE_script_grenadeawareness / IsEEDefined_script_grenadeawareness (key 0x65D5CED3)
Broc::bfloat* GetEE_script_grenadeawareness(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x65D5CED3);
}

Broc::bbool* IsEEDefined_script_grenadeawareness(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x65D5CED3);
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

// GetEE_script_bravery / IsEEDefined_script_bravery (key 0x9146436F)
Broc::bfloat* GetEE_script_bravery(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x9146436F);
}

Broc::bbool* IsEEDefined_script_bravery(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x9146436F);
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

// GetEE_destructible_id / IsEEDefined_destructible_id (key 0xD35E38B6)
Broc::bint* GetEE_destructible_id(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xD35E38B6);
}

Broc::bbool* IsEEDefined_destructible_id(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xD35E38B6);
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

// GetEE_dontdropgrenade / IsEEDefined_dontdropgrenade (key 0x8F860160)
Broc::bint* GetEE_dontdropgrenade(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x8F860160);
}

Broc::bbool* IsEEDefined_dontdropgrenade(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x8F860160);
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

// GetEE_script_fxstop / IsEEDefined_script_fxstop (key 0x2D39C958)
Broc::bint* GetEE_script_fxstop(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x2D39C958);
}

Broc::bbool* IsEEDefined_script_fxstop(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x2D39C958);
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

// GetEE_script_accuracyvsplayer / IsEEDefined_script_accuracyvsplayer (key 0x4D4CD355)
Broc::bint* GetEE_script_accuracyvsplayer(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x4D4CD355);
}

Broc::bbool* IsEEDefined_script_accuracyvsplayer(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x4D4CD355);
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

// GetEE_script_sightrange / IsEEDefined_script_sightrange (key 0x1BA1AFE0)
Broc::bint* GetEE_script_sightrange(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x1BA1AFE0);
}

Broc::bbool* IsEEDefined_script_sightrange(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x1BA1AFE0);
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

// GetEE_script_followmax / IsEEDefined_script_followmax (key 0x4135B36D)
Broc::bint* GetEE_script_followmax(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x4135B36D);
}

Broc::bbool* IsEEDefined_script_followmax(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x4135B36D);
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

// GetEE_script_colorid_time / IsEEDefined_script_colorid_time (key 0x5806FC2E)
Broc::bfloat* GetEE_script_colorid_time(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x5806FC2E);
}

Broc::bbool* IsEEDefined_script_colorid_time(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x5806FC2E);
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

// GetEE_script_fb_required_kills / IsEEDefined_script_fb_required_kills (key 0x3A66201A)
Broc::bint* GetEE_script_fb_required_kills(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x3A66201A);
}

Broc::bbool* IsEEDefined_script_fb_required_kills(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x3A66201A);
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

// GetEE_script_usemg42 / IsEEDefined_script_usemg42 (key 0x4A71613B)
Broc::bint* GetEE_script_usemg42(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x4A71613B);
}

Broc::bbool* IsEEDefined_script_usemg42(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x4A71613B);
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

// GetEE_script_multiplier / IsEEDefined_script_multiplier (key 0x9874995B)
Broc::bint* GetEE_script_multiplier(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x9874995B);
}

Broc::bbool* IsEEDefined_script_multiplier(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x9874995B);
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

// GetEE_script_mg42stayput / IsEEDefined_script_mg42stayput (key 0x149F24E8)
Broc::bint* GetEE_script_mg42stayput(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x149F24E8);
}

Broc::bbool* IsEEDefined_script_mg42stayput(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x149F24E8);
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

// GetEE_script_lmgaccuracy / IsEEDefined_script_lmgaccuracy (key 0x467CC17F)
Broc::bfloat* GetEE_script_lmgaccuracy(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x467CC17F);
}

Broc::bbool* IsEEDefined_script_lmgaccuracy(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x467CC17F);
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

// GetEE_script_ambush_trigger_distance / IsEEDefined_script_ambush_trigger_distance (key 0xC609EC11)
Broc::bint* GetEE_script_ambush_trigger_distance(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xC609EC11);
}

Broc::bbool* IsEEDefined_script_ambush_trigger_distance(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xC609EC11);
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

// GetEE_script_accuracy / IsEEDefined_script_accuracy (key 0x4FC25C1F)
Broc::bfloat* GetEE_script_accuracy(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x4FC25C1F);
}

Broc::bbool* IsEEDefined_script_accuracy(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x4FC25C1F);
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

// GetEE_script_damage / IsEEDefined_script_damage (key 0x26EBA853)
Broc::bint* GetEE_script_damage(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x26EBA853);
}

Broc::bbool* IsEEDefined_script_damage(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x26EBA853);
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

// GetEE_script_fb_min_onscreen_enemies / IsEEDefined_script_fb_min_onscreen_enemies (key 0x3D7B4F80)
Broc::bint* GetEE_script_fb_min_onscreen_enemies(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x3D7B4F80);
}

Broc::bbool* IsEEDefined_script_fb_min_onscreen_enemies(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x3D7B4F80);
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

// GetEE_script_health_once / IsEEDefined_script_health_once (key 0xA3572E0E)
Broc::bint* GetEE_script_health_once(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xA3572E0E);
}

Broc::bbool* IsEEDefined_script_health_once(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xA3572E0E);
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

// GetEE_script_delayed_playerseek / IsEEDefined_script_delayed_playerseek (key 0xC3E63040)
Broc::bint* GetEE_script_delayed_playerseek(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xC3E63040);
}

Broc::bbool* IsEEDefined_script_delayed_playerseek(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xC3E63040);
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

// GetEE_script_hdi_index / IsEEDefined_script_hdi_index (key 0xFDA70120)
Broc::bint* GetEE_script_hdi_index(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xFDA70120);
}

Broc::bbool* IsEEDefined_script_hdi_index(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xFDA70120);
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

// GetEE_script_radius / IsEEDefined_script_radius (key 0x478F159C)
Broc::bint* GetEE_script_radius(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x478F159C);
}

Broc::bbool* IsEEDefined_script_radius(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x478F159C);
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

// GetEE_script_min_friendlies / IsEEDefined_script_min_friendlies (key 0x3AC811DC)
Broc::bint* GetEE_script_min_friendlies(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x3AC811DC);
}

Broc::bbool* IsEEDefined_script_min_friendlies(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x3AC811DC);
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

// GetEE_script_delay_min / IsEEDefined_script_delay_min (key 0x04A98EA6)
Broc::bfloat* GetEE_script_delay_min(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x04A98EA6);
}

Broc::bbool* IsEEDefined_script_delay_min(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x04A98EA6);
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

// GetEE_script_colorid_endindex / IsEEDefined_script_colorid_endindex (key 0x52F516AE)
Broc::bint* GetEE_script_colorid_endindex(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x52F516AE);
}

Broc::bbool* IsEEDefined_script_colorid_endindex(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x52F516AE);
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

// GetEE_script_wait / IsEEDefined_script_wait (key 0x02312FA9)
Broc::bfloat* GetEE_script_wait(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x02312FA9);
}

Broc::bbool* IsEEDefined_script_wait(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x02312FA9);
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

// GetEE_script_ignoreme / IsEEDefined_script_ignoreme (key 0xFEF6344A)
Broc::bint* GetEE_script_ignoreme(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xFEF6344A);
}

Broc::bbool* IsEEDefined_script_ignoreme(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xFEF6344A);
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

// GetEE_script_fb_respawn_chunk_size / IsEEDefined_script_fb_respawn_chunk_size (key 0x572B15ED)
Broc::bint* GetEE_script_fb_respawn_chunk_size(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x572B15ED);
}

Broc::bbool* IsEEDefined_script_fb_respawn_chunk_size(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x572B15ED);
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

// GetEE_script_bloom_startindex / IsEEDefined_script_bloom_startindex (key 0x57525C92)
Broc::bint* GetEE_script_bloom_startindex(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x57525C92);
}

Broc::bbool* IsEEDefined_script_bloom_startindex(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x57525C92);
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

// GetEE_script_mg42 / IsEEDefined_script_mg42 (key 0x022BC64E)
Broc::bint* GetEE_script_mg42(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x022BC64E);
}

Broc::bbool* IsEEDefined_script_mg42(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x022BC64E);
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

// GetEE_script_bloom_endindex / IsEEDefined_script_bloom_endindex (key 0x87BBB1BB)
Broc::bint* GetEE_script_bloom_endindex(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x87BBB1BB);
}

Broc::bbool* IsEEDefined_script_bloom_endindex(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x87BBB1BB);
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

// GetEE_transparent / IsEEDefined_transparent (key 0x82EDA5B2)
Broc::bint* GetEE_transparent(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x82EDA5B2);
}

Broc::bbool* IsEEDefined_transparent(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x82EDA5B2);
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

// GetEE_script_additive_delay / IsEEDefined_script_additive_delay (key 0x2F9237EC)
Broc::bfloat* GetEE_script_additive_delay(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x2F9237EC);
}

Broc::bbool* IsEEDefined_script_additive_delay(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x2F9237EC);
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

// GetEE_script_squadnum / IsEEDefined_script_squadnum (key 0x4A9E2402)
Broc::bint* GetEE_script_squadnum(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x4A9E2402);
}

Broc::bbool* IsEEDefined_script_squadnum(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x4A9E2402);
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

// GetEE_script_patroller / IsEEDefined_script_patroller (key 0x58434709)
Broc::bint* GetEE_script_patroller(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x58434709);
}

Broc::bbool* IsEEDefined_script_patroller(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x58434709);
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

// GetEE_script_accuracyvsai / IsEEDefined_script_accuracyvsai (key 0xE403FE72)
Broc::bint* GetEE_script_accuracyvsai(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xE403FE72);
}

Broc::bbool* IsEEDefined_script_accuracyvsai(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xE403FE72);
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

// GetEE_script_ambush_type / IsEEDefined_script_ambush_type (key 0x93F860D5)
Broc::string* GetEE_script_ambush_type(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::string>(0x93F860D5);
}

Broc::bbool* IsEEDefined_script_ambush_type(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v; const Broc::string* val = ee->GetVal<Broc::string>(&v, 0x93F860D5);
            bool IsDefined = Broc::IsDefined(val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_script_mg42auto / IsEEDefined_script_mg42auto (key 0x24596F27)
Broc::bint* GetEE_script_mg42auto(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x24596F27);
}

Broc::bbool* IsEEDefined_script_mg42auto(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x24596F27);
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

// GetEE_radius / IsEEDefined_radius (key 0x110004A8)
Broc::bint* GetEE_radius(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x110004A8);
}

Broc::bbool* IsEEDefined_radius(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x110004A8);
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

// GetEE_script_killspawner / IsEEDefined_script_killspawner (key 0x0AC80A00)
Broc::bint* GetEE_script_killspawner(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x0AC80A00);
}

Broc::bbool* IsEEDefined_script_killspawner(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x0AC80A00);
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

// GetEE_script_aitargetname / IsEEDefined_script_aitargetname (key 0x0153A246)
Broc::string* GetEE_script_aitargetname(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::string>(0x0153A246);
}

Broc::bbool* IsEEDefined_script_aitargetname(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v; const Broc::string* val = ee->GetVal<Broc::string>(&v, 0x0153A246);
            bool IsDefined = Broc::IsDefined(val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_script_landmark / IsEEDefined_script_landmark (key 0xF5146A9E)
Broc::string* GetEE_script_landmark(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::string>(0xF5146A9E);
}

Broc::bbool* IsEEDefined_script_landmark(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::string v; const Broc::string* val = ee->GetVal<Broc::string>(&v, 0xF5146A9E);
            bool IsDefined = Broc::IsDefined(val);
            result->mVal = IsDefined;
        } else {
            result->mVal = false;
        }
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_script_bloom_time / IsEEDefined_script_bloom_time (key 0x9FA818BB)
Broc::bfloat* GetEE_script_bloom_time(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x9FA818BB);
}

Broc::bbool* IsEEDefined_script_bloom_time(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x9FA818BB);
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

// GetEE_dontdrawoncompass / IsEEDefined_dontdrawoncompass (key 0xB6608436)
Broc::bint* GetEE_dontdrawoncompass(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0xB6608436);
}

Broc::bbool* IsEEDefined_dontdrawoncompass(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0xB6608436);
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

// GetEE_script_walkdist / IsEEDefined_script_walkdist (key 0x17B81D57)
Broc::bfloat* GetEE_script_walkdist(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x17B81D57);
}

Broc::bbool* IsEEDefined_script_walkdist(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x17B81D57);
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

// GetEE_script_suppression / IsEEDefined_script_suppression (key 0x83C610FF)
Broc::bfloat* GetEE_script_suppression(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x83C610FF);
}

Broc::bbool* IsEEDefined_script_suppression(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x83C610FF);
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

// GetEE_script_scale / IsEEDefined_script_scale (key 0x480FB8DC)
Broc::bfloat* GetEE_script_scale(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0x480FB8DC);
}

Broc::bbool* IsEEDefined_script_scale(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0x480FB8DC);
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

// GetEE_script_hdi_time2 / IsEEDefined_script_hdi_time2 (key 0xFE6B76C9)
Broc::bfloat* GetEE_script_hdi_time2(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bfloat>(0xFE6B76C9);
}

Broc::bbool* IsEEDefined_script_hdi_time2(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bfloat v; const Broc::bfloat* val = ee->GetVal<Broc::bfloat>(&v, 0xFE6B76C9);
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

// GetEE_script_fb_max_onscreen_enemies / IsEEDefined_script_fb_max_onscreen_enemies (key 0x038572C2)
Broc::bint* GetEE_script_fb_max_onscreen_enemies(Broc::entity ent) {
    unsigned int Handle = ent.GetHandle();
    Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
    return &ee->GetRef<Broc::bint>(0x038572C2);
}

Broc::bbool* IsEEDefined_script_fb_max_onscreen_enemies(Broc::bbool* result, Broc::entity ent) {
    if (Broc::IsDefined(ent)) {
        unsigned int Handle = ent.GetHandle();
        Broc::ExtendedEntity* ee = Broc::ExtendedEntity::GetExtendedEntity(Handle);
        if (ee != NULL) {
            Broc::bint v; const Broc::bint* val = ee->GetVal<Broc::bint>(&v, 0x038572C2);
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

// GetEE_script_door - ea: 0x994CD0. Field key from IDA: 0x022700C8.
Broc::string* GetEE_script_door(Broc::string* result, Broc::pathnode node) {
    Broc::string value;
    Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetPNodeField_string(
        &value, node.GetHandle(), 0x022700C8u);
    new (result) Broc::string(*rhs);
    value.~string();
    return result;
}

// Broc::IsDefined(pathnode*) - ea: 0x994EF0.
namespace Broc {
bool IsDefined(const pathnode* node) {
    return node != nullptr && node->IsDefined();
}
}

// IsEEDefined_script_door - ea: 0x994DF0.
Broc::bbool* IsEEDefined_script_door(Broc::bbool* result,
                                      Broc::pathnode node) {
    if (Broc::IsDefined(&node)) {
        Broc::string value;
        Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetPNodeField_string(
            &value, node.GetHandle(), 0x022700C8u);
        result->mVal = Broc::IsDefined(rhs);
        value.~string();
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_script_delay - ea: 0x994F10. Field key from IDA: 0x47018F83.
Broc::bfloat* GetEE_script_delay(Broc::bfloat* result, Broc::pathnode node) {
    const float value = Broc::gBrocAPI.mBrocExports.mGetPNodeField_float(
        node.GetHandle(), 0x47018F83u);
    new (result) Broc::bfloat(value);
    return result;
}

// IsEEDefined_script_delay - ea: 0x994F90.
Broc::bbool* IsEEDefined_script_delay(Broc::bbool* result,
                                      Broc::pathnode node) {
    if (Broc::IsDefined(&node)) {
        const float raw = Broc::gBrocAPI.mBrocExports.mGetPNodeField_float(
            node.GetHandle(), 0x47018F83u);
        const Broc::bfloat value(raw);
        result->mVal = Broc::IsDefined(value);
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_spawnflags - ea: 0x995040.
__int16 GetEE_spawnflags(Broc::pathnode node) {
    return gpBrocAPI->mBrocExports.m_pnode_get_spawnflags(
        static_cast<int>(node.GetHandle()));
}

// GetEE_script_noteworthy - ea: 0x9950A0.
Broc::string* GetEE_script_noteworthy(Broc::string* result,
                                      Broc::pathnode node) {
    Broc::string value = gpBrocAPI->mBrocExports.m_pnode_get_script_noteworthy(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::string(value);
    value.~string();
    return result;
}

// IsEEDefined_script_noteworthy - ea: 0x9951A0.
Broc::bbool* IsEEDefined_script_noteworthy(Broc::bbool* result,
                                           Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_script_fb_id - ea: 0x995240. Field key from IDA: 0x4723E508.
Broc::bint* GetEE_script_fb_id(Broc::bint* result, Broc::pathnode node) {
    const int value = Broc::gBrocAPI.mBrocExports.mGetPNodeField_int(
        node.GetHandle(), 0x4723E508u);
    new (result) Broc::bint(value);
    return result;
}

// IsEEDefined_script_fb_id - ea: 0x9952C0.
Broc::bbool* IsEEDefined_script_fb_id(Broc::bbool* result,
                                      Broc::pathnode node) {
    if (Broc::IsDefined(&node)) {
        const int raw = Broc::gBrocAPI.mBrocExports.mGetPNodeField_int(
            node.GetHandle(), 0x4723E508u);
        const Broc::bint value(raw);
        result->mVal = Broc::IsDefined(value);
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_script_waittill - ea: 0x995370. Field key from IDA: 0x1164451E.
Broc::string* GetEE_script_waittill(Broc::string* result,
                                    Broc::pathnode node) {
    Broc::string value;
    Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetPNodeField_string(
        &value, node.GetHandle(), 0x1164451Eu);
    new (result) Broc::string(*rhs);
    value.~string();
    return result;
}

// IsEEDefined_script_waittill - ea: 0x995470.
Broc::bbool* IsEEDefined_script_waittill(Broc::bbool* result,
                                         Broc::pathnode node) {
    if (Broc::IsDefined(&node)) {
        Broc::string value;
        Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetPNodeField_string(
            &value, node.GetHandle(), 0x1164451Eu);
        result->mVal = Broc::IsDefined(rhs);
        value.~string();
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_target - ea: 0x995570.
Broc::string* GetEE_target(Broc::string* result, Broc::pathnode node) {
    Broc::string value = gpBrocAPI->mBrocExports.m_pnode_get_target(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::string(value);
    value.~string();
    return result;
}

// IsEEDefined_target - ea: 0x995670.
Broc::bbool* IsEEDefined_target(Broc::bbool* result,
                                Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_script_mg42 - ea: 0x995710. Field key from IDA: 0x022BC64E.
Broc::bint* GetEE_script_mg42(Broc::bint* result, Broc::pathnode node) {
    const int value = Broc::gBrocAPI.mBrocExports.mGetPNodeField_int(
        node.GetHandle(), 0x022BC64Eu);
    new (result) Broc::bint(value);
    return result;
}

// IsEEDefined_script_mg42 - ea: 0x995790.
Broc::bbool* IsEEDefined_script_mg42(Broc::bbool* result,
                                     Broc::pathnode node) {
    if (Broc::IsDefined(&node)) {
        const int raw = Broc::gBrocAPI.mBrocExports.mGetPNodeField_int(
            node.GetHandle(), 0x022BC64Eu);
        const Broc::bint value(raw);
        result->mVal = Broc::IsDefined(value);
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_on_goal - ea: 0x995840.
Broc::string* GetEE_on_goal(Broc::string* result, Broc::pathnode node) {
    Broc::string value = gpBrocAPI->mBrocExports.m_pnode_get_on_goal(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::string(value);
    value.~string();
    return result;
}

// IsEEDefined_on_goal - ea: 0x995940.
Broc::bbool* IsEEDefined_on_goal(Broc::bbool* result,
                                 Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_script_ambush_type(pathnode) - ea: 0x9959E0.
Broc::string* GetEE_script_ambush_type(Broc::string* result,
                                       Broc::pathnode node) {
    Broc::string value;
    Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetPNodeField_string(
        &value, node.GetHandle(), 0x93F860D5u);
    new (result) Broc::string(*rhs);
    value.~string();
    return result;
}

// IsEEDefined_script_ambush_type - ea: 0x995AE0.
Broc::bbool* IsEEDefined_script_ambush_type(Broc::bbool* result,
                                            Broc::pathnode node) {
    if (Broc::IsDefined(&node)) {
        Broc::string value;
        Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetPNodeField_string(
            &value, node.GetHandle(), 0x93F860D5u);
        result->mVal = Broc::IsDefined(rhs);
        value.~string();
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_radius(pathnode) - ea: 0x995BE0.
Broc::bfloat* GetEE_radius(Broc::bfloat* result, Broc::pathnode node) {
    const float value = gpBrocAPI->mBrocExports.m_pnode_get_radius(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::bfloat(value);
    return result;
}

// IsEEDefined_radius(pathnode) - ea: 0x995C50.
Broc::bbool* IsEEDefined_radius(Broc::bbool* result,
                                Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_origin - ea: 0x995CF0.
Broc::vector* GetEE_origin(Broc::vector* result, Broc::pathnode node) {
    const Broc::vector value = gpBrocAPI->mBrocExports.m_pnode_get_origin(
        static_cast<int>(node.GetHandle()));
    *result = value;
    return result;
}

// IsEEDefined_origin - ea: 0x995D70.
Broc::bbool* IsEEDefined_origin(Broc::bbool* result,
                                Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_script_chain - ea: 0x995E10. Field key from IDA: 0x46F0EE57.
Broc::string* GetEE_script_chain(Broc::string* result,
                                 Broc::pathnode node) {
    Broc::string value;
    Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetPNodeField_string(
        &value, node.GetHandle(), 0x46F0EE57u);
    new (result) Broc::string(*rhs);
    value.~string();
    return result;
}

// IsEEDefined_script_chain - ea: 0x995F10.
Broc::bbool* IsEEDefined_script_chain(Broc::bbool* result,
                                      Broc::pathnode node) {
    if (Broc::IsDefined(&node)) {
        Broc::string value;
        Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetPNodeField_string(
            &value, node.GetHandle(), 0x46F0EE57u);
        result->mVal = Broc::IsDefined(rhs);
        value.~string();
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_targetname - ea: 0x996010.
Broc::string* GetEE_targetname(Broc::string* result, Broc::pathnode node) {
    Broc::string value = gpBrocAPI->mBrocExports.m_pnode_get_targetname(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::string(value);
    value.~string();
    return result;
}

// IsEEDefined_targetname - ea: 0x996110.
Broc::bbool* IsEEDefined_targetname(Broc::bbool* result,
                                    Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_animscript - ea: 0x9961B0.
Broc::string* GetEE_animscript(Broc::string* result, Broc::pathnode node) {
    Broc::string value = gpBrocAPI->mBrocExports.m_pnode_get_animscript(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::string(value);
    value.~string();
    return result;
}

// IsEEDefined_animscript - ea: 0x9962B0.
Broc::bbool* IsEEDefined_animscript(Broc::bbool* result,
                                    Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_script_ambush_trigger_distance - ea: 0x996350.
Broc::bint* GetEE_script_ambush_trigger_distance(
    Broc::bint* result, Broc::pathnode node) {
    const int value = Broc::gBrocAPI.mBrocExports.mGetPNodeField_int(
        node.GetHandle(), 0xC609EC11u);
    new (result) Broc::bint(value);
    return result;
}

// IsEEDefined_script_ambush_trigger_distance - ea: 0x9963D0.
Broc::bbool* IsEEDefined_script_ambush_trigger_distance(
    Broc::bbool* result, Broc::pathnode node) {
    if (Broc::IsDefined(&node)) {
        const int raw = Broc::gBrocAPI.mBrocExports.mGetPNodeField_int(
            node.GetHandle(), 0xC609EC11u);
        const Broc::bint value(raw);
        result->mVal = Broc::IsDefined(value);
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_type - ea: 0x996480.
Broc::string* GetEE_type(Broc::string* result, Broc::pathnode node) {
    Broc::string value = gpBrocAPI->mBrocExports.m_pnode_get_type(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::string(value);
    value.~string();
    return result;
}

// IsEEDefined_type - ea: 0x996580.
Broc::bbool* IsEEDefined_type(Broc::bbool* result,
                              Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_angles - ea: 0x996620.
Broc::vector* GetEE_angles(Broc::vector* result, Broc::pathnode node) {
    const Broc::vector value = gpBrocAPI->mBrocExports.m_pnode_get_angles(
        static_cast<int>(node.GetHandle()));
    *result = value;
    return result;
}

// IsEEDefined_angles - ea: 0x9966A0.
Broc::bbool* IsEEDefined_angles(Broc::bbool* result,
                                Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_reservename - ea: 0x996740.
Broc::string* GetEE_reservename(Broc::string* result,
                                Broc::pathnode node) {
    Broc::string value = gpBrocAPI->mBrocExports.m_pnode_get_reservename(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::string(value);
    value.~string();
    return result;
}

// IsEEDefined_reservename - ea: 0x996840.
Broc::bbool* IsEEDefined_reservename(Broc::bbool* result,
                                     Broc::pathnode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsPathNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_script_delay(vehiclenode) - ea: 0x9968E0.
Broc::bfloat* GetEE_script_delay(Broc::bfloat* result,
                                 Broc::vehiclenode node) {
    const float value = Broc::gBrocAPI.mBrocExports.mGetVNodeField_float(
        node.GetHandle(), 0x47018F83u);
    new (result) Broc::bfloat(value);
    return result;
}

// IsEEDefined_script_delay(vehiclenode) - ea: 0x996980.
Broc::bbool* IsEEDefined_script_delay(Broc::bbool* result,
                                      Broc::vehiclenode node) {
    if (Broc::IsDefined(&node)) {
        const float raw = Broc::gBrocAPI.mBrocExports.mGetVNodeField_float(
            node.GetHandle(), 0x47018F83u);
        const Broc::bfloat value(raw);
        result->mVal = Broc::IsDefined(value);
    } else {
        result->mVal = false;
    }
    return result;
}

// Broc::IsDefined(vehiclenode*) - ea: 0x996A30.
namespace Broc {
bool IsDefined(const vehiclenode* node) {
    return node != nullptr && node->IsDefined();
}
}

// GetEE_endswitch - ea: 0x996A50. Field key from IDA: 0xA007FC29.
Broc::vehiclenode* GetEE_endswitch(Broc::vehiclenode* result,
                                   Broc::vehiclenode node) {
    Broc::vehiclenode value;
    Broc::vehiclenode* rhs =
        Broc::gBrocAPI.mBrocExports.mGetVNodeField_vehiclenode(
            &value, node.GetHandle(), 0xA007FC29u);
    result->___u0 = rhs->___u0;
    return result;
}

// IsEEDefined_endswitch - ea: 0x996AD0.
Broc::bbool* IsEEDefined_endswitch(Broc::bbool* result,
                                   Broc::vehiclenode node) {
    if (Broc::IsDefined(&node)) {
        Broc::vehiclenode value;
        Broc::gBrocAPI.mBrocExports.mGetVNodeField_vehiclenode(
            &value, node.GetHandle(), 0xA007FC29u);
        result->mVal = Broc::IsDefined(&value);
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_script_noteworthy(vehiclenode) - ea: 0x996B80.
Broc::string* GetEE_script_noteworthy(Broc::string* result,
                                      Broc::vehiclenode node) {
    Broc::string value = gpBrocAPI->mBrocExports.m_vnode_get_script_noteworthy(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::string(value);
    value.~string();
    return result;
}

// IsEEDefined_script_noteworthy(vehiclenode) - ea: 0x996C80.
Broc::bbool* IsEEDefined_script_noteworthy(Broc::bbool* result,
                                           Broc::vehiclenode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsVehicleNodeDefined(node.GetHandle()) != 0;
    return result;
}

// GetEE_detourstart - ea: 0x996D20. Field key from IDA: 0x29FCB701.
Broc::bint* GetEE_detourstart(Broc::bint* result,
                              Broc::vehiclenode node) {
    const int value = Broc::gBrocAPI.mBrocExports.mGetVNodeField_int(
        node.GetHandle(), 0x29FCB701u);
    new (result) Broc::bint(value);
    return result;
}

// IsEEDefined_detourstart - ea: 0x996DA0.
Broc::bbool* IsEEDefined_detourstart(Broc::bbool* result,
                                     Broc::vehiclenode node) {
    if (Broc::IsDefined(&node)) {
        const int raw = Broc::gBrocAPI.mBrocExports.mGetVNodeField_int(
            node.GetHandle(), 0x29FCB701u);
        const Broc::bint value(raw);
        result->mVal = Broc::IsDefined(value);
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_eventwait_used - ea: 0x996E50. Field key from IDA: 0xBBC555C7.
Broc::bint* GetEE_eventwait_used(Broc::bint* result,
                                 Broc::vehiclenode node) {
    const int value = Broc::gBrocAPI.mBrocExports.mGetVNodeField_int(
        node.GetHandle(), 0xBBC555C7u);
    new (result) Broc::bint(value);
    return result;
}

// IsEEDefined_eventwait_used - ea: 0x996ED0.
Broc::bbool* IsEEDefined_eventwait_used(Broc::bbool* result,
                                        Broc::vehiclenode node) {
    if (Broc::IsDefined(&node)) {
        const int raw = Broc::gBrocAPI.mBrocExports.mGetVNodeField_int(
            node.GetHandle(), 0xBBC555C7u);
        const Broc::bint value(raw);
        result->mVal = Broc::IsDefined(value);
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_detoured - ea: 0x996F80. Field key from IDA: 0xBF22439C.
Broc::bint* GetEE_detoured(Broc::bint* result,
                           Broc::vehiclenode node) {
    const int value = Broc::gBrocAPI.mBrocExports.mGetVNodeField_int(
        node.GetHandle(), 0xBF22439Cu);
    new (result) Broc::bint(value);
    return result;
}

// IsEEDefined_detoured - ea: 0x997000.
Broc::bbool* IsEEDefined_detoured(Broc::bbool* result,
                                  Broc::vehiclenode node) {
    if (Broc::IsDefined(&node)) {
        const int raw = Broc::gBrocAPI.mBrocExports.mGetVNodeField_int(
            node.GetHandle(), 0xBF22439Cu);
        const Broc::bint value(raw);
        result->mVal = Broc::IsDefined(value);
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_script_uniquename - ea: 0x9970B0. Field key from IDA: 0xF8E5A82C.
Broc::string* GetEE_script_uniquename(Broc::string* result,
                                      Broc::vehiclenode node) {
    Broc::string value;
    Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetVNodeField_string(
        &value, node.GetHandle(), 0xF8E5A82Cu);
    new (result) Broc::string(*rhs);
    value.~string();
    return result;
}

// IsEEDefined_script_uniquename - ea: 0x9971B0.
Broc::bbool* IsEEDefined_script_uniquename(Broc::bbool* result,
                                           Broc::vehiclenode node) {
    if (Broc::IsDefined(&node)) {
        Broc::string value;
        Broc::string* rhs = Broc::gBrocAPI.mBrocExports.mGetVNodeField_string(
            &value, node.GetHandle(), 0xF8E5A82Cu);
        result->mVal = Broc::IsDefined(rhs);
        value.~string();
    } else {
        result->mVal = false;
    }
    return result;
}

// GetEE_target(vehiclenode) - ea: 0x9972B0.
Broc::string* GetEE_target(Broc::string* result, Broc::vehiclenode node) {
    Broc::string value = gpBrocAPI->mBrocExports.m_vnode_get_target(
        static_cast<int>(node.GetHandle()));
    new (result) Broc::string(value);
    value.~string();
    return result;
}

// IsEEDefined_target(vehiclenode) - ea: 0x9973B0.
Broc::bbool* IsEEDefined_target(Broc::bbool* result,
                                Broc::vehiclenode node) {
    result->mVal = Broc::IsDefined(&node)
        && Broc::gBrocAPI.mIsVehicleNodeDefined(node.GetHandle()) != 0;
    return result;
}
