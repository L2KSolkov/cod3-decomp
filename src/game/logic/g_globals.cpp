// ============================================================================
// g_globals.cpp - g.o data globals (level, debug/LOS state, shared constants)
// ============================================================================

#include "game/logic/g_local.h"

level_locals_t level;          // ?level@@3Ulevel_locals_t@@A @ 0xEC9650
DebugThread g_debugThread;     // ?g_debugThread@@3VDebugThread@@A @ 0xDEB5A0
void* SmokeGrenadeMgr::sInst;  // ?sInst@SmokeGrenadeMgr@@2PAV1@A @ 0xF049B4
void* DestructibleBankManager::sInst;  // ?sInst@DestructibleBankManager@@2PAV1@A
void* PhysDataBankManager::sInst;      // ?sInst@PhysDataBankManager@@2PAV1@A
float emissionRate_0 = 1.0f;   // @ 0xDD8254 (vehicle gunner overheat emission rate)
float gTanAimConeSpread;       // @ 0xEB1118 (g_weapon.cpp Bullet_Endpos scratch)
float max_intensity = 120.0f;  // @ 0xDD7FD4
float max_dist2 = 176400.0f;   // @ 0xDD7FD8
float radius = 30.0f;          // @ 0xDD8208 (Weapon_Revive_Test revive radius)
float radius_1 = 30.0f;        // @ 0xDD8268 (Weapon_Melee melee range)
float helmetBounce = 0.65f;    // @ 0xDD8210 (SpawnHelmet phys data)
float helmetFriction = 0.65f;  // @ 0xDD8214
float helmetMass = 0.035f;     // @ 0xDD8218
int   timeToAdd = 20000;       // @ 0xDD821C (helmet self-free time)
vmCvar_t g_weaponAmmoPools;    // @ 0x01297030
vmCvar_t g_weaponRespawn;      // @ 0x01296988
PoolAllocator* Task::sAllocator;  // ?sAllocator@Task@@2PAVPoolAllocator@@A @ 0x012F3EA8
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

// .rdata @ 0xCD67AE (2 bytes + NUL)
const char defaultFileName[] = "or";

// .rdata @ 0xD0155C / 0xD0156C (verified against XBE bytes)
extern const float colorRed[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
extern const float colorGreen[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
