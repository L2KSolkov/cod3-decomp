// ============================================================================
// g_globals.cpp - g.o data globals (level, debug/LOS state, shared constants)
// ============================================================================

#include "game/logic/g_local.h"

level_locals_t level;          // ?level@@3Ulevel_locals_t@@A @ 0xEC9650
SaveGameData* gSaveGameData;   // ?gSaveGameData@@3PAUSaveGameData@@A @ 0xF312F0
cvar_t* g_gameskill;           // ?g_gameskill@@3PAUcvar_t@@A (g.o)
cgGlobal_t cgGlobal;           // ?cgGlobal@@3UcgGlobal_t@@A @ 0xF5FE30
game_hudelem_s g_hudelems[16];  // ?g_hudelems@@3PAUgame_hudelem_s@@A @ 0xEA5580
scr_data_t g_scr_data;          // ?g_scr_data@@3Uscr_data_t@@A @ 0xEE58D0
int g_DOBJF_NOT_RENDERED_LAST_FRAME;  // ?g_DOBJF_NOT_RENDERED_LAST_FRAME@@3HA (core.o)
vmCvar_t bg_viewheight_standing;   // ?bg_viewheight_standing@@3UvmCvar_t@@A (game.o)
vmCvar_t bg_viewheight_crouched;   // ?bg_viewheight_crouched@@3UvmCvar_t@@A (game.o @ 0x1334D40)
vmCvar_t bg_viewheight_prone;      // ?bg_viewheight_prone@@3UvmCvar_t@@A (game.o @ 0x132DF30)
vmCvar_t bg_meleeassistaspeed;     // ?bg_meleeassistaspeed@@3UvmCvar_t@@A (game.o @ 0x13336D0)
vmCvar_t g_gravity;                // ?g_gravity@@3UvmCvar_t@@A (g.o @ 0x1295240)
vmCvar_t g_debugBullets;           // ?g_debugBullets@@3UvmCvar_t@@A (g.o @ 0x12962C8)
vmCvar_t g_drawEntBBoxes;          // ?g_drawEntBBoxes@@3UvmCvar_t@@A (g.o @ 0x1296358)
vmCvar_t g_debugGrenades;          // ?g_debugGrenades@@3UvmCvar_t@@A (g.o @ 0x12963E8)
vmCvar_t g_debugProneCheckDepthCheck;  // ?g_debugProneCheckDepthCheck@@3UvmCvar_t@@A (g.o @ 0x1296AA8)
vmCvar_t g_debugProneCheck;        // ?g_debugProneCheck@@3UvmCvar_t@@A (g.o @ 0x1296D60)
vmCvar_t g_player_maxhealth;       // ?g_player_maxhealth@@3UvmCvar_t@@A (g.o @ 0x1296DF0)
vmCvar_t mp_friendlyfire;          // ?mp_friendlyfire@@3UvmCvar_t@@A (g.o @ 0x129B0C8)
vmCvar_t g_speed;                  // ?g_speed@@3UvmCvar_t@@A (g.o @ 0x129D518)
vmCvar_t g_reloading;              // ?g_reloading@@3UvmCvar_t@@A (g.o @ 0x129D9A0)
vmCvar_t memory_reportAepsStats;           // ?memory_reportAepsStats@@3UvmCvar_t@@A (game2.o @ 0x12F28B0)
vmCvar_t memory_displayAepsStats;          // ?memory_displayAepsStats@@3UvmCvar_t@@A (game2.o @ 0x12F2948)
vmCvar_t memory_reportBrocBackupStackPool; // ?memory_reportBrocBackupStackPool@@3UvmCvar_t@@A (game2.o @ 0x12F2A10)
vmCvar_t memory_showStatistics;            // ?memory_showStatistics@@3UvmCvar_t@@A (game2.o @ 0x12F2AC0)
vmCvar_t memory_reportCommonPool;          // ?memory_reportCommonPool@@3UvmCvar_t@@A (game2.o @ 0x12F2B50)
vmCvar_t memory_reportBrocPool;            // ?memory_reportBrocPool@@3UvmCvar_t@@A (game2.o @ 0x12F2BF0)
vmCvar_t sound_disableAllOtherSounds;      // ?sound_disableAllOtherSounds@@3UvmCvar_t@@A (game2.o @ 0x12F34B8)
vmCvar_t sound_showSoundStatForEntity;     // ?sound_showSoundStatForEntity@@3UvmCvar_t@@A (game2.o @ 0x12F3D58)
vmCvar_t ai_showFriendlyChains;     // ?ai_showFriendlyChains@@3UvmCvar_t@@A (g.o @ 0x1296F10)
vmCvar_t ai_showNearestNode;        // ?ai_showNearestNode@@3UvmCvar_t@@A (g.o @ 0x129D910)
vmCvar_t ai_showNodes;              // ?ai_showNodes@@3UvmCvar_t@@A (g.o @ 0x1295D18)
vmCvar_t ai_showNodesDist;          // ?ai_showNodesDist@@3UvmCvar_t@@A (g.o @ 0x129E268)
vmCvar_t bg_fallDamageMaxHeight;    // ?bg_fallDamageMaxHeight@@3UvmCvar_t@@A (game.o @ 0x13300E0)
vmCvar_t bg_fallDamageMinHeight;    // ?bg_fallDamageMinHeight@@3UvmCvar_t@@A (game.o @ 0x1334428)
vmCvar_t bg_ladder_yawcap;          // ?bg_ladder_yawcap@@3UvmCvar_t@@A (game.o @ 0x1334B00)
vmCvar_t bg_lmg_yawcap;             // ?bg_lmg_yawcap@@3UvmCvar_t@@A (game.o @ 0x132B838)
vmCvar_t bg_nofatigue;              // ?bg_nofatigue@@3UvmCvar_t@@A (game.o @ 0x1332E00)
vmCvar_t bg_prone_yawcap;           // ?bg_prone_yawcap@@3UvmCvar_t@@A (game.o @ 0x1332ED8)
vmCvar_t g_debugDamage;             // ?g_debugDamage@@3UvmCvar_t@@A (g.o @ 0x12A0388)
vmCvar_t g_debugMove;               // ?g_debugMove@@3UvmCvar_t@@A (g.o @ 0x12955B0)
vmCvar_t g_drawSmokeGren;           // ?g_drawSmokeGren@@3UvmCvar_t@@A (g.o @ 0x12956D0)
vmCvar_t g_entinfo_maxdist;         // ?g_entinfo_maxdist@@3UvmCvar_t@@A (g.o @ 0x129D7F0)
vmCvar_t g_entinfo_scale;           // ?g_entinfo_scale@@3UvmCvar_t@@A (g.o @ 0x1294520)
vmCvar_t g_knockback;               // ?g_knockback@@3UvmCvar_t@@A (g.o @ 0x1296508)
vmCvar_t g_performanceTest;         // ?g_performanceTest@@3UvmCvar_t@@A (g.o @ 0x129D488)
vmCvar_t g_performanceTestCell;     // ?g_performanceTestCell@@3UvmCvar_t@@A (g.o @ 0x129DE20)
vmCvar_t g_performanceTestDelta;    // ?g_performanceTestDelta@@3UvmCvar_t@@A (g.o @ 0x129E140)
vmCvar_t g_performanceTestDeltaAngle;  // ?g_performanceTestDeltaAngle@@3UvmCvar_t@@A (g.o @ 0x129E020)
vmCvar_t g_vehControlMode;          // ?g_vehControlMode@@3UvmCvar_t@@A (g.o @ 0x129D248)
vmCvar_t mp_gametype;               // ?mp_gametype@@3UvmCvar_t@@A (g.o @ 0x129B548)
vmCvar_t mp_headIconDistAbovePlayer;    // ?mp_headIconDistAbovePlayer@@3UvmCvar_t@@A (g.o @ 0x1296A18)
vmCvar_t mp_headIconDistAboveVehicle;   // ?mp_headIconDistAboveVehicle@@3UvmCvar_t@@A (g.o @ 0x129B8A8)
vmCvar_t mp_headIconHeight;             // ?mp_headIconHeight@@3UvmCvar_t@@A (g.o @ 0x1295888)
vmCvar_t mp_headIconMinScreenSize;      // ?mp_headIconMinScreenSize@@3UvmCvar_t@@A (g.o @ 0x1295AD8)
vmCvar_t mp_headIconReviveMaxAlphaDist; // ?mp_headIconReviveMaxAlphaDist@@3UvmCvar_t@@A (g.o @ 0x129B818)
vmCvar_t mp_headIconReviveMinAlphaDist; // ?mp_headIconReviveMinAlphaDist@@3UvmCvar_t@@A (g.o @ 0x12A02F8)
vmCvar_t mp_itemIconDistAboveItem;      // ?mp_itemIconDistAboveItem@@3UvmCvar_t@@A (g.o @ 0x1295A40)
vmCvar_t mp_itemIconHeight;             // ?mp_itemIconHeight@@3UvmCvar_t@@A (g.o @ 0x1296238)
vmCvar_t mp_itemIconMaxAlphaDist;       // ?mp_itemIconMaxAlphaDist@@3UvmCvar_t@@A (g.o @ 0x1295368)
vmCvar_t mp_itemIconMinAlphaDist;       // ?mp_itemIconMinAlphaDist@@3UvmCvar_t@@A (g.o @ 0x12A0418)
vmCvar_t mp_itemIconMinScreenSize;      // ?mp_itemIconMinScreenSize@@3UvmCvar_t@@A (g.o @ 0x12946D0)
vmCvar_t mp_objectiveFarAlpha;          // ?mp_objectiveFarAlpha@@3UvmCvar_t@@A (g.o @ 0x129BDB8)
vmCvar_t mp_objectiveFarAlphaDist;      // ?mp_objectiveFarAlphaDist@@3UvmCvar_t@@A (g.o @ 0x129DD00)
vmCvar_t mp_objectiveMaxSize;           // ?mp_objectiveMaxSize@@3UvmCvar_t@@A (g.o @ 0x1296478)
vmCvar_t mp_objectiveMinSize;           // ?mp_objectiveMinSize@@3UvmCvar_t@@A (g.o @ 0x12947F0)
vmCvar_t mp_objectiveNearAlpha;         // ?mp_objectiveNearAlpha@@3UvmCvar_t@@A (g.o @ 0x129B938)
vmCvar_t mp_objectiveNearAlphaDist;     // ?mp_objectiveNearAlphaDist@@3UvmCvar_t@@A (g.o @ 0x1294490)
vmCvar_t mp_objectiveSize;              // ?mp_objectiveSize@@3UvmCvar_t@@A (g.o @ 0x129B278)
vmCvar_t sound_debug;               // ?sound_debug@@3UvmCvar_t@@A (g.o @ 0x129DD90)
vmCvar_t g_cheats;                  // ?g_cheats@@3UvmCvar_t@@A (g.o @ 0x129B668)
vmCvar_t g_developer;               // ?g_developer@@3UvmCvar_t@@A (g.o @ 0x129BB78)
vmCvar_t g_debug_sound_aliases;     // ?g_debug_sound_aliases@@3UvmCvar_t@@A (g.o @ 0x129B308)
vmCvar_t g_dumpAnims;               // ?g_dumpAnims@@3UvmCvar_t@@A (g.o @ 0x12957F8)
vmCvar_t g_listEntity;              // ?g_listEntity@@3UvmCvar_t@@A (g.o @ 0x129B428)
ServerTime ServerTime::sInst;  // ?sInst@ServerTime@@0V1@A (game.o)
struct ServerTime_s {
    unsigned int mNumTicksElapsed;
    int          mTickMSec;
    float        mTickDelta;
    float        mTickDeltaInv;
    float        mElapsedTime;
};
ServerTime_s ServerTime_sInst;  // ?ServerTime_sInst@@3UServerTime_s@@A (common)
DebugThread g_debugThread;     // ?g_debugThread@@3VDebugThread@@A @ 0xDEB5A0
EntityHandleDb EntityHandleDb::sInst;  // ?sInst@EntityHandleDb@@0V1@A (g.o @ 0x12BB4E8)
void* SmokeGrenadeMgr::sInst;  // ?sInst@SmokeGrenadeMgr@@2PAV1@A @ 0xF049B4
void* DestructibleBankManager::sInst;  // ?sInst@DestructibleBankManager@@2PAV1@A
void* PhysDataBankManager::sInst;      // ?sInst@PhysDataBankManager@@2PAV1@A
float gStickyBoxScaleEasy = 1.25f;   // @ 0xDF5A28 (sticky aim box scale, easy)
float gStickyBoxScaleNormal = 1.0f;  // @ 0xDF5A2C
float gStickyBoxScaleHard = 0.85f;   // @ 0xDF5A30
int    iGrenadeHudTweak = 900;       // @ 0xDF8E60
int    gInteractArmsWeaponIndex;     // @ 0xF4EBF4
float  gLastGrenadeTimeLeft;         // @ 0xF4EC08
float  gCurrentGrenadeTimeLeft;      // @ 0xF4EC0C
float  ratio;                        // @ 0xF4EC10
float  player_breath_fire_delay;     // @ 0xF4EC14
vmCvar_t bg_debugWeaponState;        // @ 0xF43070
vmCvar_t bg_debugWeaponAnim;         // @ 0xF43A68
vmCvar_t bg_meleeassistrange;        // @ 0xF3E988
vmCvar_t bg_meleeassistfov;          // @ 0xF44BC8
int      g_useOnScreenSoundDebugging;  // ?g_useOnScreenSoundDebugging@@3HA @ 0xF04994
float gExtraDistanceSticky = 0.0f;   // @ 0xF4EBEC
float tangent = 0.02f;               // @ 0xDF8DC4 (sticky aim cone tangent)
float accel_slow_factor = 0.5f;      // @ 0xDF8DC8
float clostDist = 0.7f;              // @ 0xDF8DCC
float xy = 15.0f;                    // @ 0xDF8DD0
float boundingMin = 24.0f;           // @ 0xDF8DD4
float depthScale = 150.0f;           // @ 0xDF8DD8
float player_breath_hold_time = 3.0f;    // @ 0xDF6B3C (breath hold seconds)
float player_breath_gasp_time = 4.5f;    // @ 0xDF6B40
float player_breath_hold_lerp = 6.0f;    // @ 0xDF6B44
float player_breath_gasp_lerp = 4.0f;    // @ 0xDF6B48
float player_breath_gasp_scale = 1.0f;   // @ 0xDF6B4C
char* pszGameDll;                    // ?pszGameDll@@3PADA @ 0xDF5A34 (debug prints)
int   iLastState;                    // @ 0xDF8C78 (PM_Weapon_PrintWeaponState)
int   iLastAnim;                     // @ 0xDF8C7C (PM_Weapon_PrintWeaponAnim)
char  gDisableLMGHipFire;            // @ 0xF4EBFC (LMG hip-fire toggle)
float* dword_F63B8C[4 * 6320];       // ?dword_F63B8C (game.o @ 0xF63B8C)
float emissionRate_0 = 1.0f;   // @ 0xDD8254 (vehicle gunner overheat emission rate)
float gTanAimConeSpread;       // @ 0xEB1118 (g_weapon.cpp Bullet_Endpos scratch)
float max_intensity = 120.0f;  // @ 0xDD7FD4
float max_dist2 = 176400.0f;   // @ 0xDD7FD8
float radius = 30.0f;          // @ 0xDD8208 (Weapon_Revive_Test revive radius)
float radius_1 = 30.0f;        // @ 0xDD8268 (Weapon_Melee melee range)
float abovehead_tresh = 100.0f;  // @ 0xDD820C (Player_GetActivateEnt)
float decal_radius = 3.0f;     // @ 0xDD8204 (bullet impact decal radius)
float decal_radius_0 = 3.0f;   // @ 0xDD826C (Bullet_Fire_Extended decal radius)
float fudge_0 = 0.01f;         // @ 0xDD812C (push_entity AABB fudge)
float radius_0 = 15.0f;        // @ 0xDD8264 (vehicle collision push radius)
float radius_2 = 40.0f;        // @ 0xDD8260 (ClientThink tunnel radius)
float udelta = 1.0f;           // @ 0xDD81FC (VEH_Slide sphere offsets)
float fdelta = 0.6f;           // @ 0xDD8200
float helmetBounce = 0.65f;    // @ 0xDD8210 (SpawnHelmet phys data)
float helmetFriction = 0.65f;  // @ 0xDD8214
float helmetMass = 0.035f;     // @ 0xDD8218
int   timeToAdd = 20000;       // @ 0xDD821C (helmet self-free time)
vmCvar_t g_weaponAmmoPools;    // @ 0x01297030
vmCvar_t g_weaponRespawn;      // @ 0x01296988
PoolAllocator* Task::sAllocator;  // ?sAllocator@Task@@2PAVPoolAllocator@@A @ 0x012F3EA8
scr_vehicle_t s_phys;          // g_scr_vehicle.cpp static scratch
const float s_invalidAngles[3] = { 3.1415927f, 3.1415927f, 3.1415927f };  // @ 0xDD7414
float dword_DD7418 = 3.1415927f;  // @ 0xDD7418
float dword_DD741C = 3.1415927f;  // @ 0xDD741C
float minPitch = 16.0f;      // @ 0xDD822C (tank gunner min pitch)
float deltaYAWmaxs = 140.0f; // @ 0xDD8238
float deltaYAWmins = -140.0f;// @ 0xDD8244

// vehicle_rb_parameter field name -> offset table (ParseVehiclePhysicsConfigString)
vehicleVarConfig_t sVehicleVarConfig[27] = {  // @ 0xDD7608
    { "speed_max", 0x00 },           { "accel_max", 0x04 },
    { "reverse_scale", 0x08 },       { "steer_angle_max", 0x0C },
    { "steer_speed", 0x10 },         { "wheel_radius", 0x14 },
    { "susp_spring_k", 0x18 },       { "susp_damp_k", 0x1C },
    { "susp_adj", 0x20 },            { "susp_hard_limit", 0x24 },
    { "tire_fric_fwd", 0x28 },       { "tire_fric_side", 0x2C },
    { "tire_fric_brake", 0x30 },     { "tire_fric_hand_brake", 0x34 },
    { "body_mass", 0x38 },           { "mass_center_delta_x", 0x3C },
    { "mass_center_delta_y", 0x40 }, { "mass_center_delta_z", 0x44 },
    { "roll_stability", 0x48 },      { "roll_resistance", 0x4C },
    { "upright_strength", 0x50 },    { "tilt_fakey", 0x54 },
    { "peel_out_max_speed", 0x58 },  { "inertia_scale_x", 0x5C },
    { "tire_damp_coast", 0x60 },     { "tire_damp_brake", 0x64 },
    { "tire_damp_hand", 0x68 },
};

// tag hash arrays (g_scr_vehicle.cpp data, filled by static init) @ .data 0xEE62CC
static unsigned int TagHash(const char* s) { return HashString::CalcHash(s); }
unsigned int s_wheelTagHashes[6] = {
    TagHash("tag_wheel_front_left"), TagHash("tag_wheel_front_right"),
    TagHash("tag_wheel_back_left"), TagHash("tag_wheel_back_right"),
    TagHash("tag_wheel_middle_left"), TagHash("tag_wheel_middle_right"),
};
unsigned int s_gunnerFlashTagHashes[6] = { 0, 0, 0, 0, 0, 0 };  // boundary only
unsigned int s_entryPointTagHashes[6] = {
    TagHash("tag_enter_right"), TagHash("tag_enter_left"),
    TagHash("tag_enter_back"), TagHash("tag_enter_back_left"),
    TagHash("tag_enter_back_right"), TagHash("tag_wheel_front"),
};
unsigned int s_flashTagHashes[4] = {
    TagHash("tag_flash"), TagHash("tag_flash_11"),
    TagHash("tag_flash_2"), TagHash("tag_flash_3"),
};
unsigned int s_seatTagHashes[6] = {
    TagHash("tag_driver"), TagHash("tag_gunner"),
    TagHash("tag_passenger1"), TagHash("tag_passenger2"),
    TagHash("tag_passenger3"), TagHash("tag_passenger4"),
};

int g_drawDebugLos;            // @ 0xEB1108
int g_drawDebugEntityLos;      // @ 0xEB110C
int g_numLosHits;              // @ 0xEB1110
int g_numLosMisses;            // @ 0xEB1114

char line[256];                // @ 0xEF3448 (ConcatArgs scratch)
unsigned int g_HitLocConstNames[19];  // @ 0xEAEAD0 (BSS, filled at runtime)

// .data @ 0xDD7480 (verified against XBE bytes)
const char* entityTypeNames[18] = {
    "ET_GENERAL", "ET_PLAYER", "ET_ITEM", "ET_MISSILE", "ET_MOVER", "ET_PORTAL",
    "ET_INVISIBLE", "ET_SCRIPTMOVER", "ET_SOUND_BLEND", "ET_LOOP_FX", "ET_MG42",
    "ET_ACTOR", "ET_ACTOR_SPAWNER", "ET_ACTOR_CORPSE", "ET_VEHICLE",
    "ET_VEHICLE_CORPSE", "ET_VEHICLE_COLLMAP", "ET_PROP_COLLMAP",
};

// .data @ 0xDD7260 (verified against XBE bytes)
const char* gSpawnStrings[53] = {
    "sound_blend",
    "script_brushmodel",
    "script_model",
    "script_origin",
    "script_prop_collmap",
    "script_vehicle",
    "script_vehicle_collmap",
    "misc_model",
    "info_player_start",
    "info_null",
    "info_notnull",
    "info_notnull_big",
    "info_grenade_hint",
    "func_door",
    "func_static",
    "func_rotating",
    "func_bobbing",
    "func_pendulum",
    "func_group",
    "func_door_rotating",
    "trigger_use",
    "trigger_multiple",
    "trigger_friendlychain",
    "trigger_hurt",
    "trigger_once",
    "trigger_damage",
    "trigger_lookat",
    "trigger_mount",
    "light",
    "misc_mg42",
    "misc_turret",
    "props_skyportal",
    "corona",
    "spawn_intermission",
    "spawn_deathmatch",
    "spawn_teamdeathmatch",
    "spawn_ctf_allies_primary",
    "spawn_ctf_allies_secondary",
    "spawn_ctf_axis_primary",
    "spawn_ctf_axis_secondary",
    "spawn_single_ctf_allies",
    "spawn_single_ctf_axis",
    "spawn_hq_allies_primary",
    "spawn_hq_allies_secondary",
    "spawn_hq_axis_primary",
    "spawn_hq_axis_secondary",
    "hq_point",
    "spawn_dom_allies",
    "spawn_dom_axis",
    "spawn_war_allies",
    "spawn_war_axis",
    "spawn_sd_allies",
    "spawn_sd_axis",
};

HashString gSpawnHashes[53];       // @ 0xED9D30 (BSS, filled by prepare_spawns)
const char* g_key;                 // @ 0xEA6418
const char* g_value;               // @ 0xEA62F0
HashString classname_hash;         // @ 0xEE6270
bool dont_delete;                  // @ 0xEB111C
bool gCareAboutCheckpoint;         // @ 0xDD74C8
math::Position3 playerMaxs;        // @ 0xEC9640
math::Position3 playerMins;        // @ 0xEC9620
vmCvar_t g_bounds_width;           // @ 0xEA6CA8
vmCvar_t g_bounds_height_standing; // @ 0xEA7368
cFreeList<Entity> gEntFreeList;    // @ 0xF50D04

str_const_t str_const;             // @ 0xECBD30 (runtime-filled)
HashString hash_const_info_player_deathmatch;  // ?hash_const_info_player_deathmatch (g.o)

// .rdata @ 0xCD67AE - 2 NUL bytes then "sv_cheats" (verified vs XBE bytes)
const char* const defaultFileName = "\0\0sv_cheats";

// .rdata @ 0xD0155C / 0xD0156C (verified against XBE bytes)
extern const float colorRed[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
extern const float colorGreen[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
