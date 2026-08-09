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
// scr_vehicle_t - vehicle runtime state (infoIdx at +0x178 verified vs disasm)
// ============================================================================
struct scr_vehicle_t {
    void* gunnerWeapon;   // +0x00
    void* altWeapon;      // +0x04
    int   shooter;        // +0x08
    uint8_t _pad0C[0x178 - 0x0C];
    int16_t infoIdx;      // +0x178
    uint8_t _pad17A[0x318 - 0x17A];
    int     barrelBlocked;  // +0x318
};
static_assert(offsetof(scr_vehicle_t, infoIdx) == 0x178, "scr_vehicle_t::infoIdx offset mismatch");

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
    Broc::string active;              // +0x000
    uint8_t    _pad[0x148 - 0x4];     // +0x004
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
    HashString active;                  // +0x00
    HashString activate;                // +0x04
    HashString angle_deltas;            // +0x08
    HashString animdone;                // +0x0C
    HashString bodyque;                 // +0x10
    HashString cam_vehicle_first;       // +0x14
    HashString cam_vehicle_third;       // +0x18
    HashString claimed;                 // +0x1C
    HashString combat;                  // +0x20
    HashString count;                   // +0x24
    HashString crouch;                  // +0x28
    HashString crowbar;                 // +0x2C
    HashString current;                 // +0x30
    HashString damage;                  // +0x34
    HashString deactivate;              // +0x38
    HashString death;                   // +0x3C
    uint8_t    _pad40[0x98 - 0x40];
    HashString func_door;          // +0x98
    HashString func_door_rotating; // +0x9C
    HashString func_rotating;      // +0xA0
    HashString func_tramcar;       // +0xA4
    uint8_t    _padA8[0xEC - 0xA8];
    HashString movedone;           // +0xEC
    uint8_t    _padF0[0x11C - 0xF0];
    HashString pickup;             // +0x11C
    HashString player;             // +0x120
    uint8_t    _pad124[0x144 - 0x124];
    HashString rotatedone;         // +0x144
    uint8_t    _pad148[0x208 - 0x148];
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
extern vmCvar_t g_gravity;             // g_gravity
extern vmCvar_t g_reloading;           // g_reloading
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
void G_RunThink(Entity* ent, int msec);
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
void g_Trace(trace_t* results, const math::Position3& start, const math::Position3& mins,
             const math::Position3& maxs, const math::Position3& end,
             const collision_context_t& context);
void g_TraceCapsule(trace_t* results, const math::Position3& start, const math::Position3& mins,
                    const math::Position3& maxs, const math::Position3& end,
                    const collision_context_t& context);
void g_LocationalTrace(trace_t* results, const math::Position3* start,
                       const math::Position3* end, const collision_context_t* context,
                       unsigned char* priorityMap, float coneAngleTangent);
int  SV_PointContents(const math::Position3& p, const collision_context_t& context);

// ============================================================================
// g_hudelem.cpp types/globals
// ============================================================================
enum he_type_t {
    HE_TYPE_FREE = 0,
    HE_TYPE_COUNT = 0x0F,
};
struct hudelem_t {
    int   type;         // +0x00
    int   x;            // +0x04
    int   y;            // +0x08
    float fontScale;    // +0x0C
    int   font;         // +0x10
    int   alignX;       // +0x14
    int   alignY;       // +0x18
    uint8_t color[4];   // +0x1C (hudelem_color_t)
    uint8_t fromColor[4];  // +0x20
    int   fadeStartTime;// +0x24
    int   fadeTime;     // +0x28
    int   label;        // +0x2C
    int   width;        // +0x30
    int   height;       // +0x34
    void* mTexture;     // +0x38
    int   fromWidth;    // +0x3C
    int   fromHeight;   // +0x40
    int   scaleStartTime;// +0x44
    int   scaleTime;    // +0x48
    int   fromX;        // +0x4C
    int   fromY;        // +0x50
    int   moveStartTime;// +0x54
    int   moveTime;     // +0x58
    int   time;         // +0x5C
    int   duration;     // +0x60
    float value;        // +0x64
    int   text;         // +0x68
    float sort;         // +0x6C
    float SCOORD;       // +0x70
    float TCOORD;       // +0x74
    float angle;        // +0x78
};
static_assert(sizeof(hudelem_t) == 0x7C, "hudelem_t size mismatch");
struct game_hudelem_s {
    hudelem_t elem;  // +0x00 (0x7C bytes)
};
static_assert(sizeof(game_hudelem_s) == 0x7C, "game_hudelem_s size mismatch");
extern game_hudelem_s g_hudelems[16];  // 0xEA5580
void HudElem_SetDefaults(game_hudelem_s* hud);  // ea: 0x44AFE0 (inline COMDAT)
void Scr_ParamError(unsigned int index, const char* error);

// ============================================================================
// g_actor_prone.cpp externs
// ============================================================================
struct corpseInfo_t {
    DbLinkedHandle<EntityHandleDb, Entity> mEntity;  // +0x00
    actor_prone_info_t proneInfo;                    // +0x04
};
static_assert(sizeof(corpseInfo_t) == 0x1C, "corpseInfo_t size mismatch");
struct scr_data_t {
    corpseInfo_t actorCorpseInfo[71];  // +0x000 (0x7CC bytes)
    uint8_t      _pad7CC[0x8];         // +0x7CC
    AnimTree*    generic_human_tree;   // +0x7D4 (approx; exact layout TBD)
};
extern scr_data_t g_scr_data;  // 0xEE58D0
extern const math::Position3 actorMaxs;  // 0xF99330
int  G_GetActorCorpseIndex(Entity* ent);
int  BG_ActorIsProne(actor_prone_info_t* pInfo, int iCurrentTime);
float BG_GetActorProneFraction(actor_prone_info_t* pInfo, int iCurrentTime);
int  BG_ActorGoalIsProne(actor_prone_info_t* pInfo);
enum proneCheckType_t { PCT_ACTOR = 0 };
int  BG_CheckProneValid(DbLinkedHandle<EntityHandleDb, Entity> passEntity,
                        const math::Position3& vPos, float fSize, float fHeight, float fYaw,
                        float* pfTorsoHeight, float* pfTorsoPitch, float* pfWaistPitch,
                        int bAlreadyProne, int bOnGround, const math::Dir3& vGroundNormal,
                        void (__cdecl* traceFunc)(trace_t*, const math::Position3&, const math::Position3&,
                                                  const math::Position3&, const math::Position3&,
                                                  const collision_context_t&),
                        void (__cdecl* boxTraceFunc)(trace_t*, const math::Position3&, const math::Position3&,
                                                     const math::Position3&, const math::Position3&,
                                                     const collision_context_t&),
                        int (__cdecl* pointcontents)(const math::Position3&, const collision_context_t&),
                        proneCheckType_t proneCheckType, float prone_feet_dist);

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
int  RegisterHashString(const char* txt);   // ?RegisterHashString@BrocSys@@YAHPBD@Z
}

// ============================================================================
// BrocAPI (g_scr.cpp) - artillery callback used by G_LaunchMissile
// ============================================================================
struct BrocExports {
    uint8_t _pad[0xC50];
    void (*mAnimInitialize)();  // +0xC50
    uint8_t _padC54[0xC90 - 0xC54];
    void (*mCallbackPlayerDamage)(unsigned int a1, unsigned int a2, unsigned int a3,
                                  float* a4, float* a5, int a6, int a7, int a8,
                                  hitLocation_t a9);  // +0xC90
    uint8_t _padC94[0xD44 - 0xC94];
    void (*mCallbackFireArtilleryShell)(unsigned int handle);  // +0xD44
};
struct BrocAPI {
    BrocExports mBrocExports;
};
extern BrocAPI* gpBrocAPI;  // 0xF3ABDC

// g.o data: think dispatch table (function pointers per fn_think_e)
extern void (*thinktable[])(Entity* ent, int msec);

// fn_think_e values used by g.o (verified via disasm)
enum {
    THINK__NULL = 0,
    THINK__G_ExplodeMissile = 8,
    THINK__G_IncomingMissile = 0x0B,
    THINK__G_DelayMissile = 0x0D,
    THINK__FinishSpawningItem = 6,
    THINK__G_FreeEntity = 0x0C,
    THINK__GotoPos3 = 0x0E,
    THINK__turret_think_init = 0x11,
    THINK__misc_spawner_think = 0x12,
    THINK__multi_wait = 0x13,
    THINK__Scr_Vehicle_Think = 0x19,
    THINK__Think_SpawnNewDoorTrigger = 0x1B,
    THINK__Think_SpawnNewAutoDoorTrigger = 0x1C,
    THINK_MAX = 0x1E,
};

namespace View {
bool IsSplitScreen();  // ea: 0x00693C10 (cg_misc.cpp)
}

// ============================================================================
// weapon helpers (BG_* from game2.o; extern)
// ============================================================================
int  BG_AmmoForWeapon(int iWeapon);
int  BG_ClipForWeapon(int iWeapon);
int  BG_GetNumWeapons();
int  BG_GetAmmoClipSize(int iClipIndex);
int  BG_PlayerTouchesMine(PlayerState* ps, EntityState* item, int atTime);
int  irand(int min, int max);
void G_AddLean(Entity* ent, float* point);
extern float delta;          // 0xDD7FE4 (mine test standoff distance)
extern float dword_F63C70[];  // 0xF63C70 (per-client muzzle offsets)
extern unsigned char bulletPriorityMap[];  // 0xDD55D0

// ============================================================================
// g_combat.cpp types/globals
// ============================================================================
struct vehicle_info_t {
    uint8_t _pad0[0x20];            // +0x00
    int16_t type;                   // +0x20
    int16_t subtype;                // +0x22
    uint8_t _pad24[0x30 - 0x24];
    float   bulletDamage;           // +0x30
    float   grenadeDamage;          // +0x34
    float   mineDamage;             // +0x38
    float   projectileDamage;       // +0x3C
    uint8_t _pad40[0x310 - 0x40];
};
static_assert(sizeof(vehicle_info_t) == 0x310, "vehicle_info_t size mismatch");
extern vehicle_info_t* s_vehicleInfos[];  // ?s_vehicleInfos@@3PAPAUvehicle_info_t@@A

struct hitLoc {
    const char* mName;  // +0x00
};
extern hitLoc g_hitLocs[];             // 0xDD76E0
extern float g_fHitLocDamageMult[19];  // 0xEA5380
extern int dword_EA53C8;               // 0xEA53C8

// HandleDb deref helper (matches IDA operator* / operator->)
inline Entity* HandleDbToEnt(const DbLinkedHandle<EntityHandleDb, Entity>& h) {
    unsigned int mVal = h.mHandle.mVal;
    unsigned int idx = mVal & 0xFFF;
    if (idx < 0x540 && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[idx].mKey)
        return EntityHandleDb::sInst.mElements[idx].mObject;
    return nullptr;
}

// Forward decls for config-string parsing (full types in core_systems.h)
struct ConfigString;
class ConfigStringManager {
public:
    unsigned char mData[0x190];
    void CallbackSearch(TPakId pakId, const char* type,
                        void (*callback)(const char*, const ConfigString*));
};
extern ConfigStringManager* ConfigStringManager_sInst;

// cspField_t - config-string parse field (12 bytes) - verified against IDA
struct cspField_t {
    const char* szName;     // +0x00
    int         iOffset;    // +0x04
    int         iFieldType; // +0x08
};
static_assert(sizeof(cspField_t) == 0xC, "cspField_t size mismatch");

// externs
float  AngleNormalize180(float angle);
float  AngleNormalize360(float angle);
float  AngleSubtract(float a1, float a2);
float  PitchForYawOnNormal(float fYaw, const float* vNormal);
void   gunrandom(float* x, float* y);
extern float gTanAimConeSpread;
int    Actor_CheckArmor(actor_s* pSelf, int damage, int dflags);
int    CheckArmor(Entity* ent, int damage, int dflags);
int    LogAccuracyHit(Entity* target, Entity* attacker);
int    G_IsVehicleImmune(Entity* ent, int mod);
float  Damage_Falloff(float fDistance, float fDamage, float fMinDamagePercent,
                      int iInnerRadius, int iOuterRadius);
int    G_BounceMissile(Entity* ent, trace_t* trace);
int    ParseConfigStringToStruct(unsigned char* pStruct, const cspField_t* pFieldList,
                                 int iNumFields, const ConfigString* pCfgStr,
                                 int iMaxFieldTypes, void* parseSpecialFieldType,
                                 void (*parseStrcpy)(unsigned char*, const char*, int));
void   G_HitLocStrcpy(unsigned char* out, const char* in, int size);
void   G_AddEvent(Entity* ent, int event, int eventParm);
void   G_Damage(Entity* targ, Entity* inflictor, Entity* attacker,
                const float* dir, const float* point, int damage, int dflags,
                int mod, hitLocation_t hitLoc, int weapon);

// ============================================================================
// itemType_t / gitem_s - item table entry (0x34 bytes) - verified against IDA
// ============================================================================
enum itemType_t {
    IT_BAD = 0,
    IT_WEAPON = 1,
    IT_AMMO = 2,
    IT_HEALTH = 3,
    IT_WEAPON_AMMO = 4,
    IT_WEAPON_HEALTH = 5,
    IT_FLAG = 6,
    IT_KIT = 7,   // IT_FLAG | IT_WEAPON (class kits)
};
struct gitem_s {
    unsigned int classname_hash;  // +0x00
    char*        classname;       // +0x04
    char*        pickup_sound;    // +0x08
    char*        world_model[2];  // +0x0C
    char*        icon;            // +0x14
    char*        ammoicon;        // +0x18
    char*        pickup_name;     // +0x1C
    int          quantity;        // +0x20
    itemType_t   giType;          // +0x24
    int          giTag;           // +0x28
    int          giAmmoIndex;     // +0x2C
    int          giClipIndex;     // +0x30
};
static_assert(sizeof(gitem_s) == 0x34, "gitem_s size mismatch");
static_assert(offsetof(gitem_s, giType) == 0x24, "gitem_s::giType offset mismatch");

// ============================================================================
// weaponFileInfo_t - weapon definition (0x948 bytes; g.o uses a subset)
// ============================================================================
struct weaponFileInfo_t {
    uint8_t _pad0[0x8];           // +0x000
    char*   szInternalName;       // +0x8
    uint8_t _pad8[0xB4 - 0xC];
    int     slot;                 // +0xB4
    uint8_t _padB8[0x598 - 0xB8];
    char*   szWorldModel;         // +0x598
    uint8_t _pad1[0x5C4 - 0x59C];
    int     iProjectileSpeed;     // +0x5C4
    int     iProjectileSpeedUp;   // +0x5C8
    uint8_t _pad2[0x5DC - 0x5CC];
    int     iMinDamagePercent;    // +0x5DC
    int     iDamageInnerRadius;   // +0x5E0
    int     iDamageOuterRadius;   // +0x5E4
    uint8_t _pad3[0x5F8 - 0x5E8];
    int     iProjectileDelay;     // +0x5F8
    uint8_t _pad3b[0x6E8 - 0x5FC];
    int     bTwoHanded;           // +0x6E8
    uint8_t _pad4[0x704 - 0x6EC];
    int     bNoBounce;            // +0x704
    uint8_t _pad4b[0x764 - 0x708];
    int     iAltWeaponIndex;      // +0x764
    uint8_t _pad5[0x774 - 0x768];
    int     iTriggerRadius;       // +0x774
    int     iExplosionRadius;     // +0x778
    int     iExplosionInnerDamage;// +0x77C
    int     iExplosionOuterDamage;// +0x780
    uint8_t _pad6[0x790 - 0x784];
    uint8_t projExplosion;        // +0x790
    uint8_t _pad7[0x79C - 0x791];
    int     bProjImpactExplode;   // +0x79C
    uint8_t _pad9[0x948 - 0x7A0];
};
static_assert(sizeof(weaponFileInfo_t) == 0x948, "weaponFileInfo_t size mismatch");
static_assert(offsetof(weaponFileInfo_t, bTwoHanded) == 0x6E8, "weaponFileInfo_t::bTwoHanded offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iAltWeaponIndex) == 0x764, "weaponFileInfo_t::iAltWeaponIndex offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iProjectileSpeed) == 0x5C4, "weaponFileInfo_t::iProjectileSpeed offset mismatch");
static_assert(offsetof(weaponFileInfo_t, slot) == 0xB4, "weaponFileInfo_t::slot offset mismatch");

enum {
    WEAPSLOT_SMOKE_GRENADE = 5,  // verified vs disasm G_ExplodeMissile
};
enum {
    AI_EV_GRENADE_PING = 0x0E,
    AI_EV_PROJECTILE_PING = 0x0F,
    AI_EV_PROJECTILE_IMPACT = 0x0C,
};

// ============================================================================
// weaponParms - weapon fire params (0x40 bytes) - verified against IDA
// ============================================================================
struct weaponParms {
    float forward[3];       // +0x00
    float right[3];         // +0x0C
    float up[3];            // +0x18
    float muzzleTrace[3];   // +0x24
    float gunForward[3];    // +0x30
    weaponFileInfo_t* pWeapInfo;  // +0x3C
};
static_assert(sizeof(weaponParms) == 0x40, "weaponParms size mismatch");

// ============================================================================
// g.o data
// ============================================================================
extern gitem_s bg_itemlist[];     // 0xF51EC0
extern int  itemRegistered[];     // 0xEA68A8 (bg_numItems == 137)
extern void (*gSpawnFuncs[53])(Entity* ent);  // 0xDD7338

// ============================================================================
// items/spawn helpers (defined within g.o)
// ============================================================================
void G_SpawnItem(Entity* ent, const gitem_s* item);
void G_SetModel(Entity* ent, const char* modelName, TPakId pakId, int ngIndex);
void G_DObjUpdate(Entity* ent, bool forceWeaponModel);
void SV_LinkEntity(Entity* gEnt);
void SV_UnlinkEntity(Entity* gEnt);
void G_AddEvent(Entity* ent, int event, int eventParm);
void G_Printf(const char* fmt, ...);
Entity* Drop_Item(Entity* ent, const gitem_s* item, float angle, int novelocity);

// ============================================================================
// items/script externs (game.o / game2.o / mp.o provide later)
// ============================================================================
weaponFileInfo_t* BG_GetInfoForWeapon(int iWeapon);
const gitem_s* BG_FindItem(const char* pickupName);
void SP_actor(Entity* pEnt);
void Scr_Notify(Entity* ent, HashString hashValue, int paramcount);
Handle PostEffectEventScriptCall(Entity* ent, const char* scriptId, bool queue,
                                 TPakId pakid, bool important);
void BG_EvaluateTrajectoryDelta(const trajectory_t* tr, int atTime, float* result);
void BG_EvaluateTrajectory(const trajectory_t* tr, int atTime, math::Position3& result);
void AxisToAngles(const float (*axis)[3], float* angles);
void CrossProduct(const float* v1, const float* v2, float* cross);
float VectorNormalize(float* v);
void j_nullsub_74(Entity* pSelf, int bLerp);
bool Entity_has_zone_collision(const void* self);

// ============================================================================
// g_mover.cpp types/globals
// ============================================================================
enum moverState_t {
    MOVER_POS1 = 0,
    MOVER_POS2 = 1,
    MOVER_POS3 = 2,
    MOVER_1TO2 = 3,
    MOVER_2TO1 = 4,
    MOVER_2TO3 = 5,
    MOVER_3TO2 = 6,
    MOVER_POS1ROTATE = 7,
    MOVER_POS2ROTATE = 8,
    MOVER_1TO2ROTATE = 9,
    MOVER_2TO1ROTATE = 10,
};

struct pushed_t {
    Entity* ent;        // +0x00
    float   origin[3];  // +0x04
    float   deltayaw;   // +0x10
};
static_assert(sizeof(pushed_t) == 0x14, "pushed_t size mismatch");

extern pushed_t pushed[256];   // 0xEAC948
extern pushed_t* pushed_p;     // 0xEAE2E8
extern DbLinkedHandle<EntityHandleDb, Entity> entityList[256];  // 0xEF5E20
extern DbLinkedHandle<EntityHandleDb, Entity> moveList[256];    // 0xEF5950
extern unsigned int _S68_2;    // 0xEF62EC

// dispatch tables (function pointers per mover state)
extern void (*reachedtable[3])(Entity* ent);   // REACHED_MAX == 3
extern void (*blockedtable[3])(Entity* ent, Entity* other);  // BLOCKED_MAX == 3
extern void (*thinktable[])(Entity* ent, int msec);

// externs
void SV_AdjustAreaPortalState(Entity* ent, int open);
int  SV_inPVS(const math::Position3* p1, const math::Position3* p2);
void vectoangles(const float* vec, float* angles);
float RadiusFromBounds(const math::Position3& mins, const math::Position3& maxs);
int  CM_AreaEntities(const math::Position3& mins, const math::Position3& maxs,
                     DbLinkedHandle<EntityHandleDb, Entity>* entityList,
                     int maxcount, int contentmask);
void G_Animscripted_Think(Entity* ent);
void G_SetEntityOceanHeight(Entity* pEnt);
int  ScriptMover_Updatemove(float speed, float time, math::Position3* dest);
float AngleNormalize180(float angle);
float AngleNormalize360(float angle);
void DoorRotateStartOpen(Entity* ent);
void G_MoverTeam(Entity* ent);
void G_Animscripted_Think(Entity* ent);
void j_nullsub_17(Entity* pOriginator, int eType, int iTeamFlags,
                  math::Position3* vOrigin, float fRadiusSqrd);
void j_nullsub_60(actor_s* pSelf);
void j_nullsub_83(ai_orient_t* pOrient, float fAngle);
void Sentient_InvalidateNearestNode(sentient_s* pSelf);
int  G_TryPushingEntity(Entity* check, Entity* pusher,
                        const math::Position3& move, const math::Position3& amove);
Entity* G_TestEntityPosition(Entity* ent, const math::Position3& origin);
int  G_MoverPush(Entity* pusher, const float* move, const float* amove);

// g_combat.cpp (unported; declared for g_mover callers)
void G_Damage(Entity* targ, Entity* inflictor, Entity* attacker,
              const float* dir, const float* point, int damage, int dflags,
              int mod, hitLocation_t hitLoc, int weapon);
void SentientApplyPhysicsDamage(Entity* pSelf, Entity* pInflictor, int iDamage,
                                int iMod, const float* vPosition, const float* vDir,
                                hitLocation_t hitLoc, int iWeapon);
void j_nullsub_64(Entity* pGrenade, Entity* pHitEnt);

// ============================================================================
// g_combat.cpp core (G_Damage family)
// ============================================================================
bool  IsLocalPlayer(Entity* ent);   // ?IsLocalPlayer@@YA_NPAVEntity@@@Z
float VectorNormalize2(const float* v, float* out);
extern vmCvar_t mp_friendlyfire;      // 0xEABBC8
extern vmCvar_t g_knockback;
extern vmCvar_t g_debugDamage;
extern int damageForceReductionThreshold;  // 0xDD7F40?
extern int damageForceMax;
extern int dword_F63D1C[1580 * 802];
struct cgGlobal_t {
    uint8_t _pad0[0x04];
    int teamGame;   // +0x04 (verified vs disasm)
};
extern cgGlobal_t cgGlobal;   // 0xF5FE30
float Scr_Vehicle_DamageScale(Entity* pSelf, Entity* pAttacker, Entity* pInflictor,
                              const float* point, int mod);
bool  G_IsPlayerInVehicle(Entity* player);       // ?G_IsPlayerInVehicle@@YA_NPAVEntity@@@Z
bool  IsPlayerFullySeatedInVehicle(Entity* player);
bool  G_CanPlayerBeDamagedInVehicle(Entity* player);
int   CanDamage(Entity* targ, const float* origin, Entity* inflictor);
int   G_RadiusDamage(const float* origin, Entity* inflictor, Entity* attacker,
                     float fInnerDamage, float fOuterDamage, float radius,
                     Entity* ignore, int mod);

// ============================================================================
// missile/explosion helpers
// ============================================================================
unsigned char DirToByte(const float* dir);
void  G_EntDetach(Entity* ent, const char* modelName, const char* tagName);
int   G_DObjGetWorldTagMatrix(Entity* ent, unsigned int tag_name_hash, DObjSkelMat* tagMat);
void  j_nullsub_120(Entity* pGrenade);
int   PostEffectEventWeapon(const Entity* ent, const char* weaponType, int weaponAction);
extern vmCvar_t g_debugGrenades;
extern vmCvar_t g_debugBullets;
extern vmCvar_t g_player_maxhealth;
enum {
    kActionEI_MELEE_PLAYER_LOSING = 0x400,
    kActionWEAPON_FIRE_3RD = 0x800,
};
void  G_ExplodeMissile(Entity* ent, int msec);
void  G_GrenadeTouchTriggerDamage(Entity* pActivator, const math::Position3* vStart,
                                  const math::Position3* vEnd, int iDamage, int iMOD);
Entity* G_TempEntity(const float* origin, int event);
void  G_MissileImpact(Entity* ent, trace_t* trace, const float* dir, const float* vOldOrigin);
extern Entity* g_path_owner;
namespace DebugRender {
void RenderSphere(const math::Position3* pos, float radius, const float* argb_color);
void RenderBox(const math::Position3* bmin, const math::Position3* bmax, const float* col);
}
void  G_MissileTrace(trace_t* results, const math::Position3* start,
                     const math::Position3* end,
                     DbLinkedHandle<EntityHandleDb, Entity> passEntity,
                     int contentmask, unsigned char* priorityMap);
void  j_nullsub_84(Entity* pOriginator, int eType, int iTeamFlags,
                   const float* vStart, const float* vEnd, float fRadiusSqrd);
void  G_CheckHitTriggerDamage(Entity* pActivator, const math::Position3* vStart,
                              const math::Position3* vEnd, int iDamage, int iMOD);
float VectorDistance(const float* v1, const float* v2);
void  AnglesToAxis(const float* angles, float (*axis)[3]);
void  MatrixInverse(const float (*in)[3], float (*out)[3]);
void  MatrixTransformVector(const float* in1, const float (*in2)[3], float* out);
int   G_EntLinkToWithOffset(Entity* ent, Entity* parent, const char* tagName,
                            const float* originOffset, const float* anglesOffset,
                            bool useAngles);
void  G_MissileImpact(Entity* ent, trace_t* trace, const float* dir, const float* vOldOrigin);

struct Destructible;
class IVPointer_Destructible {
public:
    Destructible* mValue;   // +0x00
    int           mPakId;   // +0x04
};
struct Destructible {
    static void DoDamage(Destructible* self, Entity* ent, int damage,
                         const math::Position3* hitp, const float* hitd,
                         int meansOfDeath, bool scriptExplode);
};

// dispatch tables
extern void (*usetable[0xE])(Entity* ent, Entity* other, Entity* activator);
extern void (*paintable[6])(Entity* ent, Entity* other, int damage, const float* point,
                            int mod, const float* dir, hitLocation_t hitLoc);
extern void (*dietable[8])(Entity* self, Entity* inflictor, Entity* attacker,
                           int damage, int mod, int weapon, const float* point,
                           const float* dir, hitLocation_t hitLoc);
Entity* SpawnHelmet(Entity* self, const float* hitP, const float* hitDir, int iDamage);
void    G_FinishDamage(Entity* targ, Entity* inflictor, Entity* attacker,
                       const float* dir, const float* point, int damage, int mod,
                       int weapon, hitLocation_t hitLoc);

// THINK table indices used by movers
enum {
    THINK__finishSpawningKeyedMover = 7,
    THINK__RespawnItem = 0x14,
    THINK__ReturnToPos1 = 0x15,
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
    void StopEffect(int handle, bool kill);  // ?StopEffect@EffectEventSys@@QAEXVHandle@@_N@Z
};

namespace BrocSys {
const char* ConvertHashToString(int hash);  // ?ConvertHashToString@BrocSys@@YAPBDH@Z
}
