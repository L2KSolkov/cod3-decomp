// ============================================================================
// g_local.h - game logic (g.o) shared types and globals
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "game/game_types.h"
#include "game/player_types.h"
#include "game/client_types.h"
#include "game/actor_types.h"
#include "game/trace_types.h"
#include "game/cvar_types.h"
#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"
#include "game/core/core_types.h"
#include "engine/broc_types.h"

#include <stddef.h>
#include <stdint.h>

// ============================================================================
// trRefEntity - render entity (0x104 bytes) - verified against IDA (subset)
// ============================================================================
struct trRefEntity {
    uint8_t _pad[0xF4];      // +0x00
    int16_t mWaterHeightOffset;  // +0xF4
    uint8_t _padF6[0xFC - 0xF6];
    uint8_t iflIndex;        // +0xFC
    uint8_t _padFD[3];       // +0xFD
    int32_t mSnapshotId;     // +0x100
};
static_assert(sizeof(trRefEntity) == 0x104, "trRefEntity size mismatch");
static_assert(offsetof(trRefEntity, iflIndex) == 0xFC, "trRefEntity::iflIndex offset mismatch");
static_assert(offsetof(trRefEntity, mWaterHeightOffset) == 0xF4, "trRefEntity::mWaterHeightOffset offset mismatch");

// ============================================================================
// AnimTree - animation set (0x38 bytes) - verified against IDA (subset)
// ============================================================================
struct XAnimEntry {
    unsigned int hash;        // +0x00
    unsigned short numAnims;  // +0x04
    unsigned short parent;    // +0x06
    void* anim;               // +0x08 nalGeneric::nalGenericAnim*
    void* notify;             // +0x0C
    int   lastAttempt;        // +0x10
    unsigned char ucLastChosenChild;  // +0x14
};
struct AnimTree {
    void* name;               // +0x00 InplaceString
    XAnimEntry entries[2];    // +0x04 InplaceVector<XAnimEntry>
    int entriesSize;          // +0x38
};

// ============================================================================
// trigger_info_t - per-pair trigger bookkeeping (16 bytes) - verified IDA
// ============================================================================
struct trigger_info_t {
    DbLinkedHandle<EntityHandleDb, Entity> mEntity;       // +0x00
    DbLinkedHandle<EntityHandleDb, Entity> mOtherEntity;  // +0x04
    int useCount;                                         // +0x08
    int otherUseCount;                                    // +0x0C
};
static_assert(sizeof(trigger_info_t) == 0x10, "trigger_info_t size mismatch");

// ============================================================================
// level_locals_t - per-level game state (0x2688 bytes) - verified IDA
// ============================================================================
struct level_locals_t {
    Client*  clients;                              // +0x000
    int      num_entities;                         // +0x004
    sentient_s* sentients;                         // +0x008
    scr_vehicle_t* vehicles;                       // +0x00C
    turretInfo_t* turrets;                         // +0x010
    actor_s* actors[32];                           // +0x014
    int      maxclients;                           // +0x094
    int      framenum;                             // +0x098
    int      time;                                 // +0x09C
    int      previousTime;                         // +0x0A0
    int      snapTime;                             // +0x0A4
    int      numActorCorpses;                      // +0x0A8
    int      spawning;                             // +0x0AC
    int      numSpawnVars;                         // +0x0B0
    struct SpawnVar {
        unsigned int key;                          // +0x00
        const char*  value;                        // +0x04
    } spawnVars[64];                               // +0x0B4 (0x200 bytes)
    int      numSpawnVarChars;                     // +0x2B4
    char     spawnVarChars[2048];                  // +0x2B8
    int      reloadDelayTime;                      // +0xAB8
    int      iNextObjectiveTime;                   // +0xABC
    int      changelevel;                          // +0xAC0
    int      endgame;                              // +0xAC4
    int      bMissionSuccess;                      // +0xAC8
    int      bMissionFailed;                       // +0xACC
    Broc::string strMissionFailedReason;           // +0xAD0
    int      savepersist;                          // +0xAD4
    int      exitTime;                             // +0xAD8
    char     nextMap[256];                         // +0xADC
    float    fFogOpaqueDist;                       // +0xBDC
    float    fFogOpaqueDistSqrd;                   // +0xBE0
    int      iGrenadeHintCount;                    // +0xBE4
    int      remapCount;                           // +0xBE8
    int      iSearchFrame;                         // +0xBEC
    int      loading;                              // +0xBF0 (loading_t)
    int      actorPredictDepth;                    // +0xBF4
    float    bounds_width;                         // +0xBF8
    float    bounds_height_standing;               // +0xBFC
    float    viewheight_standing;                  // +0xC00
    float    viewheight_crouched;                  // +0xC04
    float    viewheight_prone;                     // +0xC08
    float    MissleOnlyActiveForTime;              // +0xC0C
    uint16_t MaxVehicles;                          // +0xC10
    int      bRegisterItems;                       // +0xC14
    int      bDrawCompassFriendlies;               // +0xC18
    int      bPlayerIgnoreRadiusDamage;            // +0xC1C
    int      bPlayerIgnoreRadiusDamageLatched;     // +0xC20
    bool     pathsInvalid;                         // +0xC24
    bool     pathsInited;                          // +0xC25
    bool     pathsConnected;                       // +0xC26
    uint8_t  _padC27;                              // +0xC27
    int      initializing;                         // +0xC28
    int      newAssetLoaded;                       // +0xC2C
    uint8_t  cachedTagMat[0x4C];                   // +0xC30
    trigger_info_t triggerList[256];               // +0xC7C (0x1000 bytes)
    int      triggerListSize;                      // +0x1C7C
    int      delayFreeAnimTreeCount;               // +0x1C80
    XAnimTree* delayFreeAnimTree[512];             // +0x1C84 (0x800 bytes)
    int      delayClearAnimTreeCount;              // +0x2484
    XAnimTree* delayClearAnimTree[128];            // +0x2488 (0x200 bytes)
};
static_assert(sizeof(level_locals_t) == 0x2688, "level_locals_t size mismatch");
static_assert(offsetof(level_locals_t, time) == 0x09C, "level_locals_t::time offset mismatch");
static_assert(offsetof(level_locals_t, initializing) == 0xC28, "level_locals_t::initializing offset mismatch");
static_assert(offsetof(level_locals_t, triggerList) == 0xC7C, "level_locals_t::triggerList offset mismatch");
static_assert(offsetof(level_locals_t, delayFreeAnimTree) == 0x1C84, "level_locals_t::delayFreeAnimTree offset mismatch");

// ============================================================================
// DebugThread - debug thread picker state (20 bytes) - verified IDA
// ============================================================================
struct DebugThread {
    DbLinkedHandle<EntityHandleDb, Entity> m_entityHandle;  // +0x00
    int m_menuScrollStartIndex;                             // +0x04
    int m_menuMaxOnPage;                                    // +0x08
    int m_displayThreads;                                   // +0x0C
    int m_active;                                           // +0x10
};
static_assert(sizeof(DebugThread) == 0x14, "DebugThread size mismatch");

// ============================================================================
// g.o data globals (defined in g_globals.cpp)
// ============================================================================
extern level_locals_t level;           // ?level@@3Ulevel_locals_t@@A   0xEC9650
extern DebugThread g_debugThread;      // ?g_debugThread@@3VDebugThread@@A 0xDEB5A0
extern int g_drawDebugLos;             // 0xEB1108
extern int g_drawDebugEntityLos;       // 0xEB110C
extern int g_numLosHits;               // 0xEB1110
extern int g_numLosMisses;             // 0xEB1114
extern const char defaultFileName[];   // 0xCD67AE ("or")
extern const float colorRed[4];        // 0xD0155C {1,0,0,1}
extern const float colorGreen[4];      // 0xD0156C {0,1,0,1}
extern char line[256];                 // 0xEF3448 (ConcatArgs scratch)
extern unsigned int g_HitLocConstNames[19];  // 0xEAEAD0 (BSS, filled by ParseHitLocDmgTableEntry)
extern const char* entityTypeNames[18];      // 0xDD7480
extern const char* gSpawnStrings[53];         // 0xDD7260
extern HashString gSpawnHashes[53];           // 0xED9D30 (BSS)
extern const char* g_key;                     // 0xEA6418
extern const char* g_value;                   // 0xEA62F0
extern HashString classname_hash;             // 0xEE6270
extern bool dont_delete;                      // 0xEB111C
extern bool gCareAboutCheckpoint;             // 0xDD74C8
extern math::Position3 playerMaxs;            // 0xEC9640
extern math::Position3 playerMins;            // 0xEC9620
extern vmCvar_t g_bounds_width;               // 0xEA6CA8
extern vmCvar_t g_bounds_height_standing;     // 0xEA7368

template <typename T>
class cFreeList {
public:
    int mFree;   // +0x00
    int mUsed;   // +0x04
    T*  mpFree;  // +0x08
};
extern cFreeList<Entity> gEntFreeList;        // 0xF50D04

template <typename T, unsigned int CAP>
struct ae_sized_array {
    T   m_elements[CAP];  // +0x00
    int m_size;           // +CAP*sizeof(T)
};

template <typename K, typename V>
struct InplaceTreeElement {
    K mKey;  // +0x00
    V mVal;  // +0x04
};

template <typename T>
void EntityHandleDb_Find(unsigned int fieldOfs, T match, ae_sized_array<Entity*, 4096>& results);

// ============================================================================
// str_const_t - shared script constant strings (0x2B4) - verified against IDA
// ============================================================================
struct str_const_t {
    uint8_t    _pad[0x148];           // +0x000
    Broc::string sound_blend;         // +0x148
    uint8_t    _pad14C[0x1FC - 0x14C];
    Broc::string tempEntity;          // +0x1FC
    uint8_t    _pad200[0x2B4 - 0x200];
};
static_assert(sizeof(str_const_t) == 0x2B4, "str_const_t size mismatch");
extern str_const_t str_const;         // 0xECBD30

// ============================================================================
// hash_const_t - runtime-filled script hash constants (0x2B4, mirrors str_const)
// Filled by GScr_LoadConsts (same 173-entry order as str_const_t).
// ============================================================================
struct hash_const_t {
    uint8_t    _pad[0x34];
    HashString damage;             // +0x34
    uint8_t    _pad38[0x98 - 0x38];
    HashString func_door;          // +0x98
    HashString func_door_rotating; // +0x9C
    uint8_t    _padA0[0xA4 - 0xA0];
    HashString func_tramcar;       // +0xA4
    uint8_t    _padA8[0x208 - 0xA8];
    HashString trigger;            // +0x208
    HashString trigger_use;        // +0x20C
    HashString trigger_damage;     // +0x210
    uint8_t    _pad214[0x2B4 - 0x214];
};
static_assert(sizeof(hash_const_t) == 0x2B4, "hash_const_t size mismatch");
extern hash_const_t hash_const;    // 0xED2AB0

// ============================================================================
// Spawn field parsing (ent_field_t + fieldtype_t) - verified against IDA
// ============================================================================
enum fieldtype_t {
    F_INT = 0,
    F_SHORT = 1,
    F_BYTE = 2,
    F_FLOAT = 3,
    F_STRING = 4,
    F_VECTOR = 5,
    F_MODEL = 0xC,
    F_BROCSTR = 0xE,
    F_NONE = -1
};
struct ent_field_t {
    const char* name;      // +0x00
    int         ofs;       // +0x04
    fieldtype_t type;      // +0x08
    void (*callback)(Entity*, int);  // +0x0C
};
static_assert(sizeof(ent_field_t) == 0x10, "ent_field_t size mismatch");

// hitLocation_t is Broc's EHitLocation (HITLOC_NONE == 0, HITLOC_NUM == 0x13)
typedef EHitLocation hitLocation_t;

// Cross-object externs used by g_utils.cpp (game.o / scr.o provide later)
extern cvar_t* g_cheats;               // g_cheats
extern cvar_t* g_developer;            // g_developer
extern cvar_t* g_debug_sound_aliases;  // g_debug_sound_aliases
extern void    Scr_Error(const char* error);  // scr.o

// ============================================================================
// sv.o collision entry points (sv_world.cpp / sv_misc.cpp)
// ============================================================================
void SV_Trace(trace_t* results, const math::Position3* start, const math::Position3* mins,
              const math::Position3* maxs, const math::Position3* end,
              const collision_context_t* context, int capsule, int bLocational,
              unsigned char* priorityMap, int staticmodels, float coneAngleTangent);
void SV_SightTrace(int* hit, const math::Position3* start, const math::Position3* mins,
                   const math::Position3* maxs, const math::Position3* end,
                   const collision_context_t* context, int capsule);
int  SV_SightTraceToEntity(const math::Position3* start, const math::Position3* mins,
                           const math::Position3* maxs, const math::Position3* end,
                           DbLinkedHandle<EntityHandleDb, Entity> entity,
                           const collision_context_t* context, int capsule);
int  SV_EntityContact(const math::Position3& mins, const math::Position3& maxs,
                      const Entity* gEnt, int capsule);

// cl.o debug-line helper (cl_debug.cpp)
void CL_AddDebugLine(const float* start, const float* end, const float* color,
                     int depthTest, int duration, int fromServer, int fadeOut);

// ============================================================================
// anim.o (unported) - XAnim / DObj core entry points
// ============================================================================
AnimTree* XAnimGetAnims(XAnimTree* tree);
AnimTree* Scr_GetAnims(int index);
int       Scr_GetAnimsIndex(AnimTree* anims);
void      XAnimClearTreeGoalWeights(XAnimTree* tree, unsigned int animIndex, float blendTime);
void      XAnimClearGoalWeight(XAnimTree* tree, unsigned int animIndex, float blendTime);
void      XAnimClearTreeGoalWeightsStrict(XAnimTree* tree, unsigned int animIndex, float blendTime);
void      XAnimSetAnimRate(XAnimTree* tree, unsigned int animIndex, float rate);
void      XAnimSetTime(XAnimTree* tree, unsigned int animIndex, float time);
int       XAnimHasTime(AnimTree* anims, unsigned int animIndex);
int       XAnimIsPrimitive(AnimTree* anims, unsigned int animIndex);
float     XAnimGetLength(AnimTree* anims, unsigned int animIndex);
void      XAnimCalcAbsDelta(XAnimTree* tree, unsigned int animIndex, float* rot, float* trans);
void      XAnimGetRelDelta(AnimTree* anims, unsigned int animIndex, float* rot, float* trans,
                           float time1, float time2);
void      XAnimGetAbsDelta(AnimTree* anims, unsigned int animIndex, float* rot, float* trans, float time);
int       XAnimIsLooped(AnimTree* anims, unsigned int animIndex);
bool      XAnimNotetrackExists(AnimTree* anims, unsigned int animIndex, const unsigned int& name);
float     XAnimGetTime(XAnimTree* tree, unsigned int animIndex);
float     XAnimGetWeight(XAnimTree* tree, unsigned int animIndex);
int       XAnimHasFinished(XAnimTree* tree, unsigned int animIndex);
int       XAnimGetNumChildren(AnimTree* anims, unsigned int animIndex);
unsigned int XAnimGetChildAt(AnimTree* anims, unsigned int animIndex, unsigned int childIndex);
const char*  XAnimGetAnimName(AnimTree* anims, unsigned int animIndex);
void      XAnimClearTree(XAnimTree* tree);
void      Com_XAnimFreeSmallTree(XAnimTree* animtree);

// ============================================================================
// sv.o / anim.o DObj server helpers
// ============================================================================
bool      SV_DObjUpdateServerTime(Entity* entity, float dtime, bool bNotify);
bool      SV_DObjCreateSkelForBones(Entity* entity);
bool      SV_DObjCreateSkelForBone(Entity* entity, int boneIndex);
void      SV_DObjCalcAnim(Entity* entity, int iPhase);
void      SV_DObjCalcSkel(Entity* entity, int* partBits);
void      SV_DObjGetHierarchyBits(Entity* entity, int boneIndex, int* partBits);
DObjSkelMat* SV_DObjGetMatrixArray(Entity* entity);
int       SV_DObjGetBoneIndex(Entity* entity, unsigned int boneNameHash);
void      AnglesToAxis(const math::Position3* angles, float (*axis)[3]);
void      DObjSkel2MatrixMultiply43(const DObjSkelMat* in1, const float (*in2)[3], DObjSkelMat* out);
void      ValidatePakId(TPakId pakId);

XAnimTree* G_GetActorAnimTree(actor_s* actor);
XAnimTree* G_GetActorCorpseAnimTree(Entity* ent);
void       G_EntUnlink(Entity* ent);

// ============================================================================
// g_active.cpp / g_spawn.cpp helpers (defined within g.o)
// ============================================================================
void G_SetOrigin(Entity* ent, const float* origin);
void G_SetOrigin(Entity* ent, const math::Position3* origin);
void G_SetAngle(Entity* ent, const float* angle);
void G_SetAngle(Entity* ent, const math::Position3* angle);
void G_SetMovedir(math::Position3* angles, math::Position3* movedir);
void g_LinkEntity(Entity* ent);
void g_UnlinkEntity(Entity* ent);
void G_FreeEntity(Entity* e, int msec);
Entity* G_Spawn(TPakId pakId);
void UpdateEntityHash(Entity* ent);
int  G_SpawnString(unsigned int key, const char* defaultString, const char** out);
bool G_SpawnString(unsigned int key, const char** out);
int  G_SpawnFloat(unsigned int key, float default_value, float* out);
int  G_SpawnInt(unsigned int key, int default_value, int* out);
int  G_SpawnVector(unsigned int key, const float* default_value, float* out);
unsigned char G_SoundAliasIndex(const char* name);

// g_utils.cpp (defined within g.o)
void G_Printf(const char* fmt, ...);
void G_DPrintf(const char* fmt, ...);
void G_Error(const char* fmt, ...);
void G_Error_Localized(const char* fmt, ...);
char* vtos(const float* v);
char* vtos(const math::Position3* v);
void G_CleanupAnimTrees();

// sv.o
void SV_SetConfigstring(int index, const char* val);

// ============================================================================
// Cross-object externs
// ============================================================================
TPakId CurPakId();
bool ShouldConnectPaths();
void SV_UnlinkEntity(Entity* gEnt);
void mem_heap_free(void* ptr);
void AnglesToForward(const float* angles, float* forward);
int  Q_stricmp(const char* s1, const char* s2);
void Path_MarkNodeInvalid(PathNodes::PathNode* pNode, int eTeam);
float VectorDistanceSquared(const float* p1, const float* p2);

namespace cdOceanGlobals {
float GetHeight(int bankID, float x, float y);  // ea: 0x7C0B70
}

namespace BrocSys {
const char* ConvertHashToString(int hash);  // ?ConvertHashToString@BrocSys@@YAPBDH@Z
void CopyExtendedEntity(const Entity* source, Entity* dest);  // ?CopyExtendedEntity@BrocSys@@YAXPBVEntity@@PAV2@@Z
}

// g.o data: think dispatch table (function pointers per fn_think_e)
extern void (*thinktable[])(Entity* ent, int msec);

// fn_think_e values used by g.o (verified via disasm)
enum {
    THINK__NULL = 0,
    THINK__G_FreeEntity = 0x0C,
    THINK__GotoPos3 = 0x0E,
    THINK__turret_think_init = 0x11,
    THINK__misc_spawner_think = 0x12,
    THINK__Scr_Vehicle_Think = 0x19,
    THINK__Think_SpawnNewDoorTrigger = 0x1B,
    THINK__Think_SpawnNewAutoDoorTrigger = 0x1C,
    THINK_MAX = 0x1E,
};

// ============================================================================
// g_trigger.cpp externs
// ============================================================================
void SV_SetBrushModel(Entity* ent);
int  Q_strcasecmp(const char* s1, const char* s2);
float random();
int  Scr_IsSystemActive(unsigned char sys);
void Scr_NotifyFromEnt(Entity* ent, HashString hashValue, Entity* fromEnt);

// g.o data: DObj controller dispatch table @ 0xDD57C0 (anim.o provides funcs)
extern void (*controllertable[4])(Entity* ent, int* partBits);

// core.o (effect_events.cpp) - sound notify
class EffectEventSys {
public:
    static EffectEventSys* sInst;  // ?sInst@EffectEventSys@@2PAV1@A @ 0xF00E80
    void SendSoundNotify(Entity* pEnt);  // ea: 0x004BCDE0
};

namespace BrocSys {
const char* ConvertHashToString(int hash);  // ?ConvertHashToString@BrocSys@@YAPBDH@Z
}
