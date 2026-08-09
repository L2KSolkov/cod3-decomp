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

// .rdata @ 0xCD67AE (2 bytes + NUL)
const char defaultFileName[] = "or";

// .rdata @ 0xD0155C / 0xD0156C (verified against XBE bytes)
extern const float colorRed[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
extern const float colorGreen[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
