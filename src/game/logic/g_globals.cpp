// ============================================================================
// g_globals.cpp - g.o data globals (level, debug/LOS state, shared constants)
// ============================================================================

#include "game/logic/g_local.h"

level_locals_t level;          // ?level@@3Ulevel_locals_t@@A @ 0xEC9650
DebugThread g_debugThread;     // ?g_debugThread@@3VDebugThread@@A @ 0xDEB5A0

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
