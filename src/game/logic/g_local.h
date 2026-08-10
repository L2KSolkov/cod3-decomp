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
#include <intrin.h>
#include <string.h>

// ae_vector<T> - dynamic array (12 bytes) - verified against IDA
template <typename T>
struct ae_vector {
    T*  mElements;  // +0x00
    int mCapacity;  // +0x04
    int mSize;      // +0x08
};
static_assert(sizeof(ae_vector<char>) == 0x0C, "ae_vector size mismatch");

// ============================================================================
// scr_vehicle_t - vehicle runtime state (infoIdx at +0x178 verified vs disasm)
// ============================================================================
struct vehicleAnimStage_t {
    int startTag;  // +0x00
    int endTag;    // +0x04
    int animRow;   // +0x08
    int flags;     // +0x0C
};
static_assert(sizeof(vehicleAnimStage_t) == 0x10, "vehicleAnimStage_t size mismatch");

struct vehicleAnimRoute_t {
    int   vehPosSrc;   // +0x00
    int   vehPosDest;  // +0x04
    int   stages[1];   // +0x08 (numStages entries)
    int   numStages;   // +0x0C
    float animSpeedScale;  // +0x10
    int   flags;       // +0x14
};
static_assert(sizeof(vehicleAnimRoute_t) == 0x18, "vehicleAnimRoute_t size mismatch");

struct vehicleAnimMap_t {
    struct vehicleAnimTag_t {
        unsigned int hash;   // +0x00
    }* tags;                 // +0x00
    vehicleAnimStage_t* stages;  // +0x04
    int* entryTags;          // +0x08
    int  numEntryTags;       // +0x0C
    vehicleAnimRoute_t* routes;  // +0x10
    int  numRoutes;          // +0x14
    int* exitMap;            // +0x18
};
static_assert(sizeof(vehicleAnimMap_t) == 0x1C, "vehicleAnimMap_t size mismatch");

struct vehicle_path_node_t {
    Broc::string mName;    // +0x00
    Broc::string mTarget;  // +0x04
    float   speed;             // +0x08
    float   lookAhead;         // +0x0C
    Broc::string script_noteworthy;  // +0x10
    float   origin[3];         // +0x14
    float   dir[3];            // +0x20
    float   angles[3];         // +0x2C
    float   length;            // +0x38
    int     nextIdx;           // +0x3C
};
static_assert(sizeof(vehicle_path_node_t) == 0x40, "vehicle_path_node_t size mismatch");

struct vehicle_pathpos_t {
    int16_t nodeIdx;    // +0x00
    int16_t endOfPath;  // +0x02
    float   frac;       // +0x04
    float   speed;      // +0x08
    float   lookAhead;  // +0x0C
    float   slide;      // +0x10
    float   origin[3];  // +0x14
    float   angles[3];  // +0x20
    float   lookPos[3]; // +0x2C
    vehicle_path_node_t switchNode[2];  // +0x38
};
static_assert(sizeof(vehicle_pathpos_t) == 0xB8, "vehicle_pathpos_t size mismatch");

struct vehicleSeat_t {
    int      flags;      // +0x00
    DbLinkedHandle<EntityHandleDb, Entity> occupant;  // +0x04
    int      boneIndex;  // +0x08
    int      weapon;     // +0x0C
    float    heat;       // +0x10
    Handle   overheatEffect;  // +0x14
    uint8_t  gunMounted; // +0x18
    uint8_t  overheating;// +0x19
    uint8_t  firing;     // +0x1A
    uint8_t  _pad1B;     // +0x1B
};
static_assert(sizeof(vehicleSeat_t) == 0x1C, "vehicleSeat_t size mismatch");

// vehicle_follow - vehicle follow formation data (1048 bytes) - verified IDA
struct vehicle_follow {
    int      numFollowingActors;        // +0x00
    DbLinkedHandle<EntityHandleDb, Entity> claimedSlotEntityHandleList[6];  // +0x04
    float    slotGoalPosition[6][3];    // +0x1C
    float    positionHistory[40][3];    // +0x64
    int      historyBufferFront;        // +0x244
    int      numHistoryBufferEntries;   // +0x248
    int      rows;                      // +0x24C
    int      columns;                   // +0x250
    float    rowSpacing;                // +0x254
    float    columnSpacing;             // +0x258
    float    minFollowDistance;         // +0x25C
    int      actualRows;                // +0x260
    int      actualColumns;             // +0x264
    float    relativeFormation[6][6][3];// +0x268
};
static_assert(sizeof(vehicle_follow) == 0x418, "vehicle_follow size mismatch");

struct scr_vehicle_t {
    vehicle_pathpos_t pathPos;   // +0x00
    uint8_t _padB8[0xC0 - 0xB8];
    struct vehicle_physic_t {
        math::Position3 origin;     // +0x00
        math::Position3 prevOrigin; // +0x10
        math::Position3 angles;     // +0x20
        math::Position3 prevAngles; // +0x30
        math::Dir3      vel;        // +0x40
        math::Dir3      rotVel;     // +0x50
        float           wheelZVel[6];  // +0x60
        float           wheelZPos[6];  // +0x78
        int             wheelSurfType[6];  // +0x90
        uint8_t         _padA8[0xB0 - 0xA8];
    } phys;                          // +0xC0 (0xB0 bytes)
    DbLinkedHandle<EntityHandleDb, Entity> mEntity;  // +0x170
    DbLinkedHandle<EntityHandleDb, Entity> mPhysicsOwner;  // +0x174
    int16_t infoIdx;      // +0x178
    int16_t waitNode;     // +0x17A
    float   waitSpeed;    // +0x17C
    int     fireTime;     // +0x180
    int     altFireTime;  // +0x184
    int     gunnerFireTime;  // +0x188
    uint8_t _pad18C[0x194 - 0x18C];
    int     drawOnCompass;   // +0x194
    int     drawAsEnemy;     // +0x198
    uint8_t _pad19C[0x1A0 - 0x19C];
    int     gunnerWeapon;     // +0x1A0
    int     shooter;          // +0x1A4
    uint8_t _pad1A8[0x1B4 - 0x1A8];
    int     mMantleTime;  // +0x1B4
    DbLinkedHandle<EntityHandleDb, Entity> mMantleEntity;  // +0x1B8
    math::Position3 respawn_origin;  // +0x1C0
    math::Position3 respawn_angles;  // +0x1D0
    vehicleSeat_t seats[11];  // +0x1E0 (0x134 bytes)
    float   barrelOffset;   // +0x314
    int     barrelBlocked;  // +0x318
    int     manualMode;    // +0x31C
    float   manualSpeed;   // +0x320
    float   manualAccel;   // +0x324
    float   manualTime;    // +0x328
    float   wheelRadius;   // +0x32C
    float   wheelPitch;     // +0x330
    int     hasTarget;      // +0x334
    DbLinkedHandle<EntityHandleDb, Entity> mTargetEnt;  // +0x338
    float   targetOrigin[3];    // +0x33C
    float   targetOffset[3];    // +0x348
    int     hasGunnerTarget;    // +0x354
    DbLinkedHandle<EntityHandleDb, Entity> mGunnerTargetEnt;  // +0x358
    float   gunnerTargetOrigin[3];  // +0x35C
    float   gunnerTargetOffset[3];  // +0x368
    float   joltDir[2];     // +0x374
    float   joltTime;       // +0x37C
    float   joltWave;       // +0x380
    int     playEngineSound; // +0x384
    DbLinkedHandle<EntityHandleDb, Entity> mIdleSndEnt;  // +0x388
    DbLinkedHandle<EntityHandleDb, Entity> mEngineSndEnt;  // +0x38C
    Broc::string mProperName;  // +0x390
    Handle  mSoundEffectHandle[6];  // +0x394
    Handle  mWheel_ParticleEffectHandle[6];  // +0x3AC (0x18 bytes)
    Handle  mRumbleEffectHandle;  // +0x3C4
    int     playersAttached;  // +0x3C8
    int     lastOccupantTime;  // +0x3C8 (alias; only read when playersAttached==0)
    float   idleSndLerp;    // +0x3CC
    float   engineSndLerp;  // +0x3D0
    float   brakeSndLerp;   // +0x3D4
    float   hornSndLerp;    // +0x3D8
    struct LerpedVariables {
        math::Position3 mBodyPosition;  // +0x00
        math::Position3 mTurretAngles;  // +0x10
        math::Position3 mGunnerAngles;  // +0x20
        float mSteeringAngle;           // +0x30
        float mHatchAngleRight;         // +0x34
        float mHatchAngleLeft;          // +0x38
        float _pad3C;                   // +0x3C
    } current;                          // +0x3E0 (0x3C bytes)
    struct LerpedVariables next;        // +0x420 (0x40 bytes)
    struct VehicleBoneIndex {
        int player;           // +0x00
        int detach;           // +0x04
        int popout;           // +0x08
        int body;             // +0x0C
        int turret;           // +0x10
        int barrel;           // +0x14
        int coax;             // +0x18
        int gunner_barrel;    // +0x1C
        int gunner_player;    // +0x20
        int steering_wheel;   // +0x24
        int gunner_flash;     // +0x28
        int flash[4];         // +0x2C
        int wheel[6];         // +0x3C
        int entryPoint[6];    // +0x54
        int hatchLeft;        // +0x6C
        int hatchRight;       // +0x70
        int leader;           // +0x74
    } boneIndex;              // +0x460 (120 bytes)
    int     turretHitNum;     // +0x4D8
    int     numWaitNotify;    // +0x4DC
    int     lastCollision;    // +0x4E0
    int     lastNoCollision;  // +0x4E4
    uint8_t _pad4E8[0x510 - 0x4E8];
    int     crashSound;       // +0x510
    float   crashVolume;      // +0x514
    void*   mRBVeh;           // +0x518 rb_vehicle*
    uint8_t _pad51C[0x554 - 0x51C];
    float   mUseRadius;       // +0x554
    uint8_t mHasEntryPoints;  // +0x558
    bool    mHatchOpen;        // +0x559
    uint8_t _pad55A[0x560 - 0x55A];
    int     noEntryTime;      // +0x560
    int     noExitTime;       // +0x564
    int     forceGunnerCrouchTime;  // +0x568
    vehicleAnimMap_t* animMap;  // +0x56C
    vehicle_follow* follow;   // +0x570
    uint8_t wheel_polies[0x750 - 0x574];  // cdl_poly_inl_t[6] (untyped)
    bool    hasGround;     // +0x750 (static s_phys scratch)
    uint8_t _pad751[0x760 - 0x751];
    trace_t groundTrace;   // +0x760 (static s_phys scratch)
    static int sDebugMantle;  // ?sDebugMantle@scr_vehicle_t@@2HA
    static int sRenderEntryPoints;  // ?sRenderEntryPoints@scr_vehicle_t@@2HA
    static int sDebugAnims;     // ?sDebugAnims@scr_vehicle_t@@2HA

    vehicleAnimStage_t* GetRouteStage(int routeIdx, int stage);  // ?GetRouteStage@scr_vehicle_t@@QAEPAUvehicleAnimStage_t@@HH@Z
    float GetAnimSpeedScale(Client* client);  // ?GetAnimSpeedScale@scr_vehicle_t@@QAEMPAUClient@@@Z
    float GetThrottle();                      // ?GetThrottle@scr_vehicle_t@@QAEMXZ
    int   GetStageAnim(Client* client);       // ?GetStageAnim@scr_vehicle_t@@QAEHPAUClient@@@Z
    int   GetSwitchPosRoute(int seatIdx, int fromPos, bool hasFlag);  // ?GetSwitchPosRoute@scr_vehicle_t@@QAEHHH_N@Z
    bool  IsPhysicsPaused();                  // ?IsPhysicsPaused@scr_vehicle_t@@QAE_NXZ
    bool  IsPhysicsStable();                  // ?IsPhysicsStable@scr_vehicle_t@@QAE_NXZ
    int   GetMantleHintStringIndex();         // ?GetMantleHintStringIndex@scr_vehicle_t@@QAEHXZ
    bool  IsOppositeTeamInVehicle(int team);  // ?IsOppositeTeamInVehicle@scr_vehicle_t@@QAE_NH@Z
    void  Mantled(Entity* player);            // ?Mantled@scr_vehicle_t@@QAEXPAVEntity@@@Z
    bool  SetAnimRouteStage(Entity* player, Entity* ent, int routeIdx,
                            int stageIdx);    // ?SetAnimRouteStage@scr_vehicle_t@@QAE_NPAVEntity@@0HH@Z
    bool  CanUseVehicle(Entity* player, float* distToUsePoint, int* entryPoint);  // ?CanUseVehicle@scr_vehicle_t@@QAE_NPAVEntity@@AAMAAH@Z
    bool  CanMantleVehicle(Entity* player);   // ?CanMantleVehicle@scr_vehicle_t@@QAE_NPAVEntity@@@Z
    bool  LetHatchClose();                    // ?LetHatchClose@scr_vehicle_t@@QAE_NXZ
    void  AssignPhysics(Entity* player);      // ?AssignPhysics@scr_vehicle_t@@QAEXPAVEntity@@@Z
    int   GetEntryRoute(int seatIdx, int entryIdx, bool hasFlag);  // ?GetEntryRoute@scr_vehicle_t@@QAEHHH_N@Z
    int   GetEntryHintStringIndex(Entity* vehicle, unsigned int entryPosition);  // ?GetEntryHintStringIndex@scr_vehicle_t@@QAEHPAVEntity@@I@Z
    void  CollisionDamage(Entity* ent, const math::Position3* pos,
                          const math::Position3* dir, float intensity);  // ?CollisionDamage@scr_vehicle_t@@QAEXPAVEntity@@ABVPosition3@math@@1M@Z
    void  ReleasePhysics(Entity* player);     // ?ReleasePhysics@scr_vehicle_t@@QAEXPAVEntity@@@Z
    void  DebugRender();                      // ?DebugRender@scr_vehicle_t@@QAEXXZ
    void  UpdateAnimRoute(Entity* ent, Entity* player);  // ?UpdateAnimRoute@scr_vehicle_t@@QAEXPAVEntity@@0@Z
};
static_assert(offsetof(scr_vehicle_t, infoIdx) == 0x178, "scr_vehicle_t::infoIdx offset mismatch");
static_assert(offsetof(scr_vehicle_t, boneIndex) == 0x460, "scr_vehicle_t::boneIndex offset mismatch");
static_assert(offsetof(scr_vehicle_t, mRBVeh) == 0x518, "scr_vehicle_t::mRBVeh offset mismatch");
static_assert(offsetof(scr_vehicle_t, animMap) == 0x56C, "scr_vehicle_t::animMap offset mismatch");

void Use_Item(Entity* ent, Entity* other, Entity* activator);
void RespawnItem(Entity* ent);

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

    trRefEntity(int foo);    // ??0trRefEntity@@QAE@H@Z (game.o 0x6618B0)
    void* operator new(size_t s, void* p) { return p; }  // placement
    void SetInSnapshot();    // ?SetInSnapshot@trRefEntity@@QAEXXZ (render.o)
    bool IsInSnapshot() const;  // ?IsInSnapshot@trRefEntity@@QBE_NXZ (render.o)
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

    static void Clear(level_locals_t* self);       // ?Clear@level_locals_t@@QAEXXZ
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
    void Render();   // ?Render@DebugThread@@QAEXXZ (game2.o 0x50A050)
    void DisplayEntitySound(const math::Position3* entityPos, int xpos,
                            int ypos, int yinc, float scale);  // ea: 0x4F8AB0
    char* DisplayMessage(char* msg, int xpos, int ypos, float r, float g,
                         float b, float scale, float alphaMin);
    void Update();   // ?Update@DebugThread@@QAEXXZ (game2.o 0x4F46A0)
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
extern const float colorYellow[4];     // 0xD0159C
extern const float colorCyan[4];       // 0xD015DC
extern const float colorMdCyan[4];     // 0xD015FC
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
extern vmCvar_t bg_viewheight_standing;       // game.o
extern vmCvar_t bg_viewheight_crouched;       // game.o
extern vmCvar_t bg_viewheight_prone;          // game.o

// cdl_proftimer - profile timing accumulator (game.o)
struct cdl_proftimer {
    float    value;      // +0x00
    uint32_t _pad[3];    // +0x04
    uint64_t stamp;      // +0x10
    void start() { stamp = __rdtsc(); }           // ea: 0x004A91A0 (inline)
    void stop() { value += (float)(__rdtsc() - stamp); }  // ea: 0x004A91E0 (inline)
};

// ============================================================================
// vehicle / scratch / debug globals
// ============================================================================
struct vehicle_node_t {
    Broc::string mName;             // +0x00
    Broc::string mTarget;           // +0x04
    float        speed;             // +0x08
    float        lookAhead;         // +0x0C
    Broc::string script_noteworthy; // +0x10
    float        origin[3];         // +0x14
    float        dir[3];            // +0x20
    float        angles[3];         // +0x2C
    float        length;            // +0x38
    int          nextIdx;           // +0x3C
};
static_assert(sizeof(vehicle_node_t) == 0x40, "vehicle_node_t size mismatch");

// debug render (g.o) - debug_sphere batch
struct debug_sphere {
    float x, y, z, radius;  // +0x00
    float color[4];         // +0x10
};
extern ae_vector<debug_sphere> debug_spheres;  // g.o 0xED2A98
extern int render;                             // g.o 0xDD725C

extern char* g_scratchpadMem;              // 0xEA81C0
extern int   s_numNodes;                   // 0xEA5DD0
extern vehicle_node_t* s_nodes[];          // 0xEAEDF8
extern const float s_invalidAngles[3];     // @ 0xDD7414 (all pi)
extern float dword_DD7418;                 // @ 0xDD7418
extern float dword_DD741C;                 // @ 0xDD741C
struct vehicle_info_t;
extern vehicle_info_t* s_vehicleInfos[];   // 0xEA7638
extern scr_vehicle_t* s_vehicles;          // 0xEAE044
extern int sEntryPointHintIndicies[6];     // 0xECCCF4
extern int dword_DD67B8;                   // 0xDD67B8
extern int dword_DD67BC;                   // 0xDD67BC
extern int dword_DD67C0;                   // 0xDD67C0
extern int dword_DD67C4;                   // 0xDD67C4
extern int dword_DD67C8;                   // 0xDD67C8
extern int (*syscall)(int, ...);           // 0xDF9D70 (cg.o)
void DebugDumpEnts(int a1, Entity* ent);   // g.o 0x450150

// game2.o FPS test harness (full layout 0xAD90, verified against IDA)
struct TestFPS {
    unsigned char _pad[0xABE0];       // mStats[1000] data
    int mStats_size;                  // +0xABE0 (mStats.m_size)
    unsigned char _pad2[0xAC48 - 0xABE4];  // mCells data
    int mCells_size;                  // +0xAC48 (mCells.m_size)
    bool mTesting;                    // +0xAC4C
    int mBlock;                       // +0xAC50
    struct Position3Packed {
        float x, y, z;
    } mCurrentPosition;               // +0xAC54
    int mCurrentAngle;                // +0xAC60
    int mCellIndex;                   // +0xAC64
    int mCellX;                       // +0xAC68
    int mCellXDelta;                  // +0xAC6C
    int mCellY;                       // +0xAC70
    int mZoneIndex;                   // +0xAC74
    int mDeltaAngle;                  // +0xAC78
    float mDelta;                     // +0xAC7C
    float mDeltaInverse;              // +0xAC80
    int mCurrentPositionIndex;        // +0xAC84
    char mLastFile[0x100];            // +0xAC88
    Handle mPlayerHandle;             // +0xAD88
    void* mFile;                      // +0xAD8C (_iobuf*)
    static TestFPS* sInst;  // ?sInst@TestFPS@@2PAV1@A
    TestFPS();              // ?TestFPS@TestFPS@@QAE@XZ (game2.o 0x4FEC20)
    ~TestFPS();             // ?~TestFPS@TestFPS@@QAE@XZ (game2.o 0x4EBFF0)
    void GetPath(char* path);  // ?GetPath@TestFPS@@AAEXPAD@Z (game2.o 0x4EC020)
    void GetFilename(char* filename);  // ?GetFilename@TestFPS@@AAEXPAD@Z (game2.o 0x4F6F70)
    void OutputStats();  // ?OutputStats@TestFPS@@AAEXXZ (game2.o 0x4FEC80)
    void GatherMetrics();  // ?GatherMetrics@TestFPS@@QAEXXZ (game2.o 0x501990)
    void NextPosition();   // ?NextPosition@TestFPS@@AAEXXZ (game2.o 0x501A60)
    bool CheckForFloor(const math::Position3* position, trace_t* trace,
                       float zMin);  // ea: 0x4F6FC0
    void Test();            // ?Test@TestFPS@@QAEXXZ
    void StopTest();        // ?StopTest@TestFPS@@QAEXXZ
    void PositionCamera(pmove_t* pm);  // ?PositionCamera@TestFPS@@QAEXPAUpmove_t@@@Z
};
static_assert(sizeof(TestFPS) == 0xAD90, "TestFPS size mismatch");

// g_main.cpp entry / console commands
void game_dllEntry(int (*syscallptr)(int, ...));
void Cmd_TestFPS(void);
void Cmd_NGLStats_f(void);
bool Cmd_NGLStatDisplay_f(void);
void Cmd_ProfileShaders_f(void);
void Cmd_ProfileNodes_f(void);
void Cmd_Wireframe_f(void);
void Cmd_Fullbright_f(void);
void Cmd_SolidColor_f(void);
void Cmd_ToggleShader_f(void);
void Cmd_KillSound(void);
void Cmd_BuilderTest_f(void);
void Cmd_Thread_Debug_f(void);
int  Cmd_EntityStats_f(void);
void Cmd_TextureTiling_f(void);
void G_EndGame(void);
char* GetScratchPad(void);
int   G_GetServerSnapTime(void);
void  G_InitVehiclePaths(void);
vehicle_node_t* GetVehicleNode(int idx);
vehicle_info_t* VEH_GetInfo(int idx);
void  g_UnlinkEntity(Entity* ent);
void  SnapVectorTowards(float* /*v*/, float* /*target*/);
void  G_SetClientSound(Entity* ent);
void  G_RunClient(Entity* ent);
int   ClientInactivityTimer(Entity* ent);
int   ClientSpectatorInactivityTimer(Entity* ent);
bool  Player_CheckFriendlyFireUse(PlayerState* ps);
void  respawn(Entity* ent);
void  handleDeathInvulnerability(Entity* ent, int a2, int a3);
void  Fill_Clip(PlayerState* ps, int weapon);
void  MemGraph_RenderResources(void);
int   G_StealVehicleSeat(Entity* ent, Entity* veh, int seat, bool bForce);
void  G_FreeVehicleSeat(Entity* ent, Entity* veh, int seat);
int   G_RequestVehicleSeat(Entity* ent, Entity* veh, HashString seat, bool bForce);
int   G_RequestVehicleBestSeat(Entity* ent, Entity* veh, bool bForce, bool bPassenger, bool bCanDrive);
int   G_GetNonPVSTankInfo(float* origin, DbLinkedHandle<EntityHandleDb, Entity> ent);
void  Scr_Vehicle_OccupantStartEntering(scr_vehicle_t* veh, const Entity* ent, int seat);
void  Scr_Vehicle_OccupantStartExiting(scr_vehicle_t* veh, const Entity* ent, int seat);
void  Scr_Vehicle_OccupantIsSeat(scr_vehicle_t* veh, const Entity* ent, int seat);
void  Scr_Vehicle_OccupantIsOut(scr_vehicle_t* veh, Entity* ent, int seat);

template <typename T>
class cFreeList {
public:
    int mFree;   // +0x00
    int mUsed;   // +0x04
    T*  mpFree;  // +0x08

    void Init(int num);     // ?Init@?$cFreeList@...@@QAEXH@Z core.o
    void Shutdown();        // ?Shutdown@?$cFreeList@...@@QAEXXZ core.o
    T* Alloc();             // ?Alloc@?$cFreeList@...@@QAEPA...XZ (core.o)
};
extern cFreeList<Entity> gEntFreeList;        // 0xF50D04

template <typename K, typename V>
struct InplaceTreeElement {
    K mKey;  // +0x00
    V mVal;  // +0x04
};

// InplaceTree - binary heap tree over InplaceTreeElement (8-byte entries).
// Verified against IDA InplaceTree.h (Find<const char*> at 0x4B1BC0,
// IsUsed at 0x4AEB00).
template <typename K, typename V>
struct InplaceTree {
    unsigned int mSize;              // +0x00
    InplaceTreeElement<K, V>* mElements;  // +0x04

    bool IsUsed(unsigned int index) const
    {
        const unsigned char* p = (const unsigned char*)&mElements[index];
        for (unsigned int i = 0; i < sizeof(InplaceTreeElement<K, V>); ++i)
        {
            if (p[i] != 0)
                return true;
        }
        return false;
    }

    template <typename TKey>
    V* Find(const TKey& key) const
    {
        unsigned int index = 0;
        if (index >= mSize)
            return nullptr;
        for (;;)
        {
            if (_stricmp(mElements[index].mKey.mStr, key) == 0)
                return &mElements[index].mVal;
            if (_stricmp(key, mElements[index].mKey.mStr) < 0)
                index = index * 2 + 2;
            else
                index = index * 2 + 1;
            if (index >= mSize)
                return nullptr;
            if (!IsUsed(index))
                return nullptr;
        }
    }
};

template <typename T>
void EntityHandleDb_Find(unsigned int fieldOfs, T match, ae_sized_array<Entity*, 4096>& results);

// ============================================================================
// str_const_t - shared script constant strings (0x2B4) - verified against IDA
// ============================================================================
struct str_const_t {
    Broc::string active;              // +0x000
    uint8_t    _pad[0xB0 - 0x4];      // +0x004
    Broc::string grenade;             // +0x0B0
    uint8_t    _padB4[0xBC - 0xB4];
    Broc::string info_player_deathmatch;  // +0xBC
    uint8_t    _padC0[0xF4 - 0xC0];
    Broc::string noclass;             // +0xF4 (verified vs Entity ctor disasm)
    uint8_t    _padF8[0x120 - 0xF8];
    Broc::string player;              // +0x120
    uint8_t    _pad124[0x13C - 0x124];
    Broc::string rocket;              // +0x13C
    uint8_t    _pad140[0x148 - 0x140];
    Broc::string sound_blend;         // +0x148
    uint8_t    _pad14C[0x168 - 0x14C];
    Broc::string spawn_intermission;  // +0x168
    Broc::string spawn_deathmatch;        // +0x16C
    Broc::string spawn_teamdeathmatch;    // +0x170
    Broc::string spawn_ctf_allies_primary;    // +0x174
    Broc::string spawn_ctf_allies_secondary;  // +0x178
    Broc::string spawn_ctf_axis_primary;      // +0x17C
    Broc::string spawn_ctf_axis_secondary;    // +0x180
    Broc::string spawn_single_ctf_allies;  // +0x184
    Broc::string spawn_single_ctf_axis;    // +0x188
    Broc::string spawn_hq_allies_primary;   // +0x18C
    Broc::string spawn_hq_allies_secondary; // +0x190
    Broc::string spawn_hq_axis_primary;     // +0x194
    Broc::string spawn_hq_axis_secondary;   // +0x198
    Broc::string spawn_dom_allies;         // +0x19C
    Broc::string spawn_dom_axis;           // +0x1A0
    Broc::string spawn_war_allies;         // +0x1A4
    Broc::string spawn_war_axis;           // +0x1A8
    Broc::string spawn_sd_allies;          // +0x1AC
    Broc::string spawn_sd_axis;            // +0x1B0
    Broc::string hq_point;                 // +0x1B4
    uint8_t    _pad1B8[0x1FC - 0x1B8];
    Broc::string tempEntity;          // +0x1FC
    uint8_t    _pad200[0x298 - 0x200];
    Broc::string smoke_grenade;       // +0x298
    uint8_t    _pad29C[0x2B4 - 0x29C];
};
static_assert(sizeof(str_const_t) == 0x2B4, "str_const_t size mismatch");
static_assert(offsetof(str_const_t, spawn_intermission) == 0x168,
              "str_const_t::spawn_intermission offset mismatch");
static_assert(offsetof(str_const_t, spawn_sd_axis) == 0x1B0,
              "str_const_t::spawn_sd_axis offset mismatch");
static_assert(offsetof(str_const_t, spawn_deathmatch) == 0x16C,
              "str_const_t::spawn_deathmatch offset mismatch");
static_assert(offsetof(str_const_t, spawn_hq_allies_primary) == 0x18C,
              "str_const_t::spawn_hq_allies_primary offset mismatch");
static_assert(offsetof(str_const_t, hq_point) == 0x1B4,
              "str_const_t::hq_point offset mismatch");
static_assert(offsetof(str_const_t, info_player_deathmatch) == 0xBC,
              "str_const_t::info_player_deathmatch offset mismatch");
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
    uint8_t    _padA8[0xB0 - 0xA8];
    HashString trigger_mount;      // +0xB0 (44)
    uint8_t    _padB4[0xBC - 0xB4];
    HashString info_player_deathmatch;  // +0xBC (47)
    uint8_t    _padC0[0xDC - 0xC0];
    HashString menuresponse;            // +0xDC (55)
    uint8_t    _padE0[0xEC - 0xE0];
    HashString movedone;           // +0xEC
    uint8_t    _padF0[0x11C - 0xF0];
    HashString pickup;             // +0x11C
    HashString player;             // +0x120
    uint8_t    _pad124[0x12C - 0x124];
    HashString reached_end_node;   // +0x12C
    HashString reached_wait_node;  // +0x130
    HashString reached_wait_speed; // +0x134
    HashString fireSpecial;             // +0x138 (78)
    uint8_t    _pad13C[0x144 - 0x13C];
    HashString rotatedone;         // +0x144
    uint8_t    _pad148[0x150 - 0x148];
    HashString script_model;       // +0x150
    uint8_t    _pad154[0x204 - 0x154];
    HashString touch;              // +0x204 (81)
    HashString trigger;            // +0x208
    HashString trigger_use;        // +0x20C
    HashString trigger_damage;     // +0x210
    HashString trigger_lookat;     // +0x214 (133)
    uint8_t    _pad218[0x224 - 0x218];
    HashString turret_on_target;   // +0x224 (137)
    HashString player_on_vehicle;  // +0x228 (138)
    HashString player_off_vehicle; // +0x22C (139)
    uint8_t    _pad230[0x248 - 0x230];
    HashString turret_on_vistarget; // +0x248 (146)
    uint8_t    _pad24C[0x250 - 0x24C];
    HashString turretstatechange;  // +0x250 (148)
    HashString turretownerchange;  // +0x254 (149)
    HashString killanimscript;     // +0x258 (150)
    uint8_t    _pad25C[0x284 - 0x25C];
    HashString overheated;         // +0x284 (161)
    uint8_t    _pad288[0x2B4 - 0x288];
};
static_assert(sizeof(hash_const_t) == 0x2B4, "hash_const_t size mismatch");
static_assert(offsetof(hash_const_t, turret_on_target) == 0x224,
              "hash_const_t::turret_on_target offset mismatch");
static_assert(offsetof(hash_const_t, turretstatechange) == 0x250,
              "hash_const_t::turretstatechange offset mismatch");
static_assert(offsetof(hash_const_t, turretownerchange) == 0x254,
              "hash_const_t::turretownerchange offset mismatch");
static_assert(offsetof(hash_const_t, overheated) == 0x284,
              "hash_const_t::overheated offset mismatch");
static_assert(offsetof(hash_const_t, fireSpecial) == 0x138,
              "hash_const_t::fireSpecial offset mismatch");
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
extern vmCvar_t g_entinfo_scale;       // ?g_entinfo_scale@@3UvmCvar_t@@A @ 0xEA5020
extern vmCvar_t g_entinfo_maxdist;     // ?g_entinfo_maxdist@@3UvmCvar_t@@A @ 0xEAE2F0
extern const float colorMagenta[4];    // @ 0xD015CC
extern vmCvar_t g_vehicleDrawPath;     // ?g_vehicleDrawPath@@3UvmCvar_t@@A @ 0xEA66F8
extern vmCvar_t g_drawEntBBoxes;       // g.o
extern vmCvar_t g_vehicleDebug;        // g.o
extern int s_newDebugLine;             // g.o
extern int com_frameNumber;              // 0x012F0324
extern vmCvar_t g_gravity;             // g_gravity
extern vmCvar_t g_speed;               // g_speed
extern vmCvar_t pmove_msec;            // pmove_msec
extern vmCvar_t pmove_fixed;           // pmove_fixed
extern vmCvar_t g_debugMove;           // g_debugMove
extern float    radius_2;              // g.o @ 0xDD8260
extern int    mem_get_used_bytes(int heap_name);   // mem_heap
extern int    mem_get_free_bytes(int heap_name);   // mem_heap
extern int    MEM_HEAP_NONE;                        // mem_heap
extern float  mainLWM;                              // g.o
extern float  brocLWM;                              // g.o
extern void*  gBrocHeap;                            // core.o
extern void*  gApsHeap;                             // core.o
extern float  textScale;                            // render.o
extern float  fontScale;                            // render.o
extern float  alpha;                                // render.o
extern float  barWidth;                             // render.o
extern float  spacing;                              // render.o
extern float  tickWidth;                            // render.o
extern float  left;                                 // render.o
extern float  MBRenderScale;                        // render.o
extern float  textOffset;                           // render.o
extern float  bigHeapScale;                         // render.o
extern int    gRenderMemGraph;                      // render.o
extern float nglPerfInfo_FPS;                       // ngl.o (offset 0)
extern vmCvar_t g_reloading;           // g_reloading
extern void    Scr_Error(const char* error);  // scr.o
extern void    tlPrintf(const char* fmt, ...);  // core.o
extern PoolAllocator* gBrocPool;       // core.o
extern void*   gShotProf;              // g.o 0x... (ShotPerfTest*)
extern cvar_t* gStatusBar;             // core.o

struct TimerRenderBars {
    uint8_t      _pad[0x10];
    unsigned int mTimeLo;  // +0x10 (rdtsc low at TimeGameAdvanceBegin)
    unsigned int mTimeHi;  // +0x14 (rdtsc high)
    uint8_t      _pad2[0x48 - 0x18];
    int mActive;   // +0x48
    static TimerRenderBars sInst;  // ?sInst@TimerRenderBars@@0V1@A (render.o 0x011EA668)
    void ToggleActive();  // ?ToggleActive@TimerRenderBars@@QAEXXZ (inline)
    void TimeGameAdvanceBegin() {  // ea: 0x72A9D0 (inline)
        unsigned __int64 t = __rdtsc();
        mTimeLo = (unsigned int)t;
        mTimeHi = (unsigned int)(t >> 32);
    }
};

extern void Cvar_Set(const char* var_name, const char* value);  // core.o
extern void* ShaderCommon_StartShotPerfTest();                  // render.o
extern void* gShotProf;                       // g.o 0x012A05DC
extern PoolAllocator* gBrocPool;              // scr.o 0x0132A0E4
extern PoolAllocator* gAeThreadBackupStackAllocator;  // core.o 0xF3ABCC
struct ClientCmdPair {
    const char* first;  // +0x00
};
extern ClientCmdPair sClientCommand0List[24];  // g.o .rdata
extern ClientCmdPair sClientCommand1List[15];  // g.o .rdata
void Cmd_MemPools_f(void);          // g.o 0x44ACC0
void Cmd_ShotProf_f(void);          // g.o 0x44AD10
void Cmd_ClientCommandCompletion(void (*callback)(const char*));  // g.o 0x44ADE0

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
DObjSkelMat* DObjSkelMatrixMultiply(DObjSkelMat* result, const DObjSkelMat* in1,
                                    const DObjSkelMat* in2);
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
void G_DObjCalcPose(Entity* ent);           // g.o
void CG_ResetLowHealthOverlay(int client);  // cg.o
int  CG_SelectFirstWeaponInSlotWithLocalIndex(int bNext, int bIgnoreEmpty,
                                              int localIdx);  // cg.o
int  CG_SelectFirstWeaponNotInSlotWithLocalIndex(int bNext, int bIgnoreEmpty,
                                                 int localIdx);  // cg.o
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
void g_SightTrace(int* hitNum, const math::Position3* start, const math::Position3* mins,
                  const math::Position3* maxs, const math::Position3* end,
                  const collision_context_t* context);
void TraceDebugLine(const math::Position3* start, const math::Position3* end,
                    int hitNum, DbLinkedHandle<EntityHandleDb, Entity> entityHandle);

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
struct ae_formatted_string_256w {
    unsigned short mBuff[256];  // +0x00
    int mLength;                // +0x200
};
struct PakInfoNode {
    Broc::string longName;  // +0x00
};
struct PakFile {
    struct dlist_node {
        PakFile* m_next;  // +0x00
        PakFile* m_prev;  // +0x04
    } m_dlist_node;      // +0x00
    static void GetHeapUsage(PakFile* self, int* used, int* size);  // streamer.o
    static const PakInfoNode* GetInfo(PakFile* self);  // streamer.o
};
struct PakDList {
    PakFile* m_head;   // +0x00
    PakFile* m_end;    // +0x04
};
extern int  apsMemory_GetPoolInfo(int nPool, int* size, int* capacity,
                                  int* used, int* peak);  // aeps_xboxr
extern int  PoolAllocator_GetMemRemaining(void* self);  // core.o
extern int  PoolAllocator_GetMemSize(void* self);       // core.o
extern int gPakHeaps_m_size;       // core.o
extern void* gPakHeaps_elements[32];  // core.o
extern float iMemUsed, iMemFree, memPeak;  // g.o statics
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
    int          levelscript;             // +0x000
    int          scripted_init;           // +0x004
    uint8_t      _pad8[0x60C - 0x8];      // generic_human/anim/classMap
    corpseInfo_t actorCorpseInfo[16];     // +0x60C (0x1C0 bytes)
    uint8_t      _pad7CC[0x7D4 - 0x7CC];
    AnimTree*    generic_human_tree;      // +0x7D4
};
static_assert(offsetof(scr_data_t, actorCorpseInfo) == 0x60C,
              "scr_data_t::actorCorpseInfo offset mismatch");
extern scr_data_t g_scr_data;  // 0xEE58D0
extern const math::Position3 actorMaxs;  // 0xF99330
extern const math::Position3 actorMins;  // 0xF99330-relative (mp_actors.o)
int  G_GetActorCorpseIndex(Entity* ent);
int  BG_ActorIsProne(actor_prone_info_t* pInfo, int iCurrentTime);
float BG_GetActorProneFraction(actor_prone_info_t* pInfo, int iCurrentTime);
int  BG_ActorGoalIsProne(actor_prone_info_t* pInfo);
// Values verified vs disasm BG_CheckProneValid: the client water check runs
// when proneCheckType == 0 and the trace contentmask adds 0xFFE0 when != 0.
enum proneCheckType_t { PCT_CLIENT = 0, PCT_ACTOR = 1 };
int  BG_CheckProneValid(DbLinkedHandle<EntityHandleDb, Entity> passEntity,
                        const math::Position3* vPos, float fSize, float fHeight, float fYaw,
                        float* pfTorsoHeight, float* pfTorsoPitch, float* pfWaistPitch,
                        int bAlreadyProne, int bOnGround, const math::Dir3* vGroundNormal,
                        void (__cdecl* traceFunc)(trace_t*, const math::Position3*, const math::Position3*,
                                                  const math::Position3*, const math::Position3*,
                                                  const collision_context_t&),
                        void (__cdecl* boxTraceFunc)(trace_t*, const math::Position3*, const math::Position3*,
                                                     const math::Position3*, const math::Position3*,
                                                     const collision_context_t&),
                        int (__cdecl* pointcontents)(const math::Position3*, const collision_context_t&),
                        proneCheckType_t proneCheckType, float prone_feet_dist);
int  BG_CheckProne(DbLinkedHandle<EntityHandleDb, Entity> passEntity,
                   const math::Position3* vPos, float fSize, float fHeight,
                   float fYaw, float* pfTorsoHeight, float* pfTorsoPitch,
                   float* pfWaistPitch, int bAlreadyProne, int bOnGround,
                   const math::Dir3* vGroundNormal,
                   void (__cdecl* traceFunc)(trace_t*, const math::Position3*,
                                             const math::Position3*, const math::Position3*,
                                             const math::Position3*, const collision_context_t&),
                   void (__cdecl* boxTraceFunc)(trace_t*, const math::Position3*,
                                                const math::Position3*, const math::Position3*,
                                                const math::Position3*, const collision_context_t&),
                   int (__cdecl* pointcontents)(const math::Position3*,
                                                const collision_context_t&),
                   proneCheckType_t proneCheckType, float prone_feet_dist);  // game.o 0x6146F0
int  PM_VerifyPronePosition(const math::Position3& vFallbackOrg,
                            const math::Position3& vFallbackVel);  // game.o 0x615B50
void PM_UpdatePronePitch();                                     // game.o 0x6156B0

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
void Init();                                 // ?Init@BrocSys@@YAXXZ (scr.o)
void CopyExtendedEntity(const Entity* source, Entity* dest);  // ?CopyExtendedEntity@BrocSys@@YAXPBVEntity@@PAV2@@Z
int  RegisterHashString(const char* txt);   // ?RegisterHashString@BrocSys@@YAHPBD@Z
void UnloadScript(void* self);              // ?UnloadScript@BrocSys@@QAEXPAV1@@Z
void LoadScript(void* self);                // ?LoadScript@BrocSys@@QAEXPAV1@@Z
}

// ============================================================================
// BrocAPI (g_scr.cpp) - artillery callback used by G_LaunchMissile
// ============================================================================
struct BrocExports {
    uint8_t _pad[0x90];
    void (*mAnimDebug)(Broc::entity);       // +0x90 (game2.o inspector)
    uint8_t _pad94[0xD8 - 0x94];
    void (*mCallbackMineFailed)(unsigned int);            // +0xD8
    uint8_t _padDC[0x128 - 0xDC];
    void (*mCallbackHealthRegenRecovering)(Broc::entity); // +0xEC
    uint8_t _padF0[0x128 - 0xF0];
    void (*mCallbackDropFlag)(unsigned int);              // +0x128
    uint8_t _pad12C[0x154 - 0x12C];
    void (*mCallbackStopFollowing)();  // +0x154
    uint8_t _pad158[0x518 - 0x158];
    int (*mRumble)(float lowFreqDelay, float lowFreqRumbleIntensity,
                   float lowFreqSteadyDuration, float lowFreqRampUpTime,
                   float lowFreqRampDownTime, float highFreqDelay,
                   float highFreqDuration, int a8);  // +0x518 (PlayRumble)
    uint8_t _pad51C[0x5A4 - 0x51C];
    void (*mMissionFailed)(Broc::string* msg);            // +0x5A4
    uint8_t _pad5A8[0x81C - 0x5A8];
    void (*mShellShock)(unsigned int ent, Broc::string* shock,
                        float fVal);                       // +0x81C
    uint8_t _pad820[0xB70 - 0x820];
    void (*mFireTurret)(unsigned int ent, bool fire);      // +0xB70
    uint8_t _padB74[0xC50 - 0xB74];
    void (*mAnimInitialize)();  // +0xC50
    uint8_t _padC54[0xC58 - 0xC54];
    const char* (*mAnimNameResolver)(unsigned int animHash);  // +0xC58 (DebugThread::Render)
    uint8_t _padC5C[0xC90 - 0xC5C];
    void (*mCallbackPlayerDamage)(unsigned int a1, unsigned int a2, unsigned int a3,
                                  float* a4, float* a5, int a6, int a7, int a8,
                                  hitLocation_t a9);  // +0xC90
    uint8_t _padC94[0xD44 - 0xC94];
    void (*mCallbackFireArtilleryShell)(unsigned int handle);  // +0xD44
    uint8_t _padD48[0xD58 - 0xD48];
    int (*mCallbackGetSlotClipCount)(const char*, unsigned int,
                                     unsigned int, int);  // +0xD58 (mp_loadout)
    void (*mCallbackGiveAmmoPack)(unsigned int ent, unsigned int count);  // +0xD5C
    uint8_t _padD60[0xD68 - 0xD60];
    void (*mCallbackPickupKit)(unsigned int ent, unsigned int count);  // +0xD68
};
struct BrocAPI {
    BrocExports mBrocExports;
};
extern BrocAPI* gpBrocAPI;  // 0xF3ABDC

// ============================================================================
// RumbleEffect layout twin + rumble shims (full types live in
// core/core_systems.h, which cannot be included alongside g_local.h)
// ============================================================================
enum ERumbleMotorID {
    kRumbleMin = 0,
    kRumbleLEFT = 0,
    kRumbleRIGHT = 1,
    kRumbleMax = 1,
};
struct RumbleEffect {
    struct RumbleData {
        bool  enabled;             // +0x00
        unsigned char _pad[0x4 - 0x1];
        float delay;               // +0x04
        float intensity;           // +0x08
        float ramp_up_duration;    // +0x0C
        float steady_duration;     // +0x10
        float ramp_down_duration;  // +0x14
        void* rumble_notes;        // +0x18
        unsigned int m_flags;      // +0x1C

        RumbleData() : enabled(false), delay(0.0f), intensity(0.0f),
                       ramp_up_duration(0.0f), steady_duration(0.0f),
                       ramp_down_duration(0.0f), rumble_notes(nullptr),
                       m_flags(0) {}
    };
    static_assert(sizeof(RumbleData) == 0x20, "RumbleData size mismatch");
    RumbleData mRumbleDataArray[2];  // +0x00
};
static_assert(sizeof(RumbleEffect) == 0x40, "RumbleEffect size mismatch");
extern void* RumbleManager_Inst(int instance);              // core.o
extern void RumbleEffect_SetIntensity(void* self, int rumbleID,
                                      float intensity);     // core.o
extern void RumbleManager_Play(void* self, void* effect,
                               float intensity);            // core.o

// g.o data: think dispatch table (function pointers per fn_think_e)
extern void (*thinktable[])(Entity* ent, int msec);

// fn_think_e values used by g.o (verified via disasm)
enum {
    THINK__NULL = 0,
    THINK__Actor_CorpseThink = 1,
    THINK__Actor_Think = 2,
    THINK__BodySink = 3,
    THINK__Concussive_think = 4,
    THINK__G_FinishSetupSpawnPoint = 5,
    THINK__FinishSpawningItem = 6,
    THINK__finishSpawningKeyedMover = 7,
    THINK__G_ExplodeMissile = 8,
    THINK__G_DelayMissile = 9,
    THINK__G_LaunchMissile = 10,
    THINK__G_IncomingMissile = 0x0B,
    THINK__G_FreeEntity = 0x0C,
    THINK__GotoPos3 = 0x0D,
    THINK__hurt_think = 0x0E,
    THINK__turret_think = 0x0F,
    THINK__turret_think_init = 0x10,
    THINK__misc_spawner_think = 0x11,
    THINK__multi_wait = 0x12,
    THINK__RespawnItem = 0x13,
    THINK__ReturnToPos1 = 0x14,
    THINK__ReturnToPos1Rotate = 0x15,
    THINK__ReturnToPos2 = 0x16,
    THINK__Scr_Vehicle_Init = 0x17,
    THINK__Scr_Vehicle_Think = 0x18,
    THINK__Think_MatchTeam = 0x19,
    THINK__Think_SpawnNewDoorTrigger = 0x1A,
    THINK__Think_SpawnNewAutoDoorTrigger = 0x1B,
    THINK__Think_GeneralLink = 0x1C,
    THINK__Think_EnableMine = 0x1D,
    THINK_MAX = 0x1E,
};

namespace View {
bool IsSplitScreen();  // ea: 0x00693C10 (cg_misc.cpp)
}

// ============================================================================
// weapon helpers (BG_* from game2.o; extern)
// ============================================================================
enum weapSlot_t : int;  // full definition below (after weaponFileInfo_t)
int  BG_AmmoForWeapon(int iWeapon);
int  BG_ClipForWeapon(int iWeapon);
void BG_GetRandomAmmoCounts(int* ammo, int* clip, int weaponIndex);  // game2.o
const char* BG_GetAmmoTypeName(int iAmmoIndex);       // game.o 0x6071B0
weapSlot_t BG_GetEmptySlotForWeapon(const PlayerState* pPS, int iWeaponIndex);  // game.o 0x6074F0
weapSlot_t BG_GetStackSlotForWeapon(const PlayerState* pPS, int iWeaponIndex,
                                    weapSlot_t preferedSlot);  // game.o 0x607570
weapSlot_t BG_IsPlayerWeaponInSlot(const PlayerState* pPS, int iWeaponIndex,
                                   int bAnyMode);    // game.o 0x616AA0
int  BG_CanItemBeGrabbed(const EntityState* ent, const PlayerState* ps,
                         int bTouched);              // game.o 0x6278C0
int  BG_GetNumWeapons();
int  BG_GetAmmoClipSize(int iClipIndex);
int  BG_FillInWeaponItems();                  // game.o 0x616040
int  BG_SetupAmmoIndexes();                   // game.o 0x6161E0
int  BG_SetupSharedAmmoIndexes();             // game.o 0x6163E0
int  BG_SetupClipIndexes();                   // game.o 0x6164A0
int  compare_weaponfile_names(const void* pe1, const void* pe2);  // game.o 0x6166A0
bool BG_IsLMGMounted(const PlayerState* ps);  // game.o 0x6166D0
bool BG_IsCookingOffGrenade(const PlayerState* ps);  // game.o 0x616750
int  BG_GetAmmoTypeForName(const char* pszName);    // game.o 0x6167E0
int  BG_GetAmmoClipForName(const char* pszName);    // game.o 0x616840
bool BG_PlayerTouchesMine(PlayerState* ps, EntityState* item, int atTime);
bool BG_PlayerTouchesItem(PlayerState* ps, EntityState* item, int atTime);  // game.o 0x621820
int  BG_WeaponIsClipOnly(int iWeapon);            // game.o 0x607A50
int  BG_GetAmmoTypeMax(int iAmmoIndex);           // game.o 0x607080
int  BG_GetSharedAmmoCapSize(int iCapIndex);      // game.o 0x607150
int  BG_GetMaxPickupableAmmo(const PlayerState* pPS, int iWeaponIndex);  // game.o 0x616B70
int  BG_SetPlayerWeaponForSlot(PlayerState* pPS, int iWeaponIndex);  // game.o 0x616A10
int  BG_GetTotalAmmoReserve(const PlayerState* pPS, int iWeaponIndex);  // game.o 0x616D50
int  BG_GetTotalAmmo(const PlayerState* pPS, int iWeaponIndex);  // game.o 0x616F10
int  BG_TakePlayerWeapon(PlayerState* pPS, int iWeaponIndex);  // game.o 0x621F60
int  BG_SelectWeaponIndex(int iWeaponIndex, int client);  // game.o 0x6076E0
int  BG_GetWeaponForInfo(void* pWeapInfo);  // game.o 0x607050
float BG_GetMinSpreadForWeapon(PlayerState* pPS, int iWeaponIndex, int iTime,
                               bool bAds);   // game.o
float BG_GetConeAngleForWeapon(PlayerState* pPS, int iWeaponIndex, int iTime,
                               bool bAds);   // game.o
unsigned char BG_GetWeaponIndexForName(const char* name);  // game.o 0x6073A0
unsigned char BG_GetWeaponIndexForName(unsigned int name);  // game.o 0x607310
bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);  // game.o 0x604A60
int  irand(int min, int max);
void G_AddLean(Entity* ent, float* point);
extern float delta;          // 0xDD7FE4 (mine test standoff distance)
extern float dword_F63C70[];  // 0xF63C70 (per-client muzzle offsets)
extern unsigned char bulletPriorityMap[];  // 0xDD55D0
extern unsigned char riflePriorityMap[];   // g.o .rdata
extern float gTriggerLookAtOverride;       // @ 0xDF4914

// ============================================================================
// g_combat.cpp types/globals
// ============================================================================
struct vehicle_info_t {
    char    name[32];               // +0x00
    int16_t type;                   // +0x20
    int16_t subtype;                // +0x22
    uint8_t _pad24[0x30 - 0x24];
    float   bulletDamage;           // +0x30
    float   grenadeDamage;          // +0x34
    float   mineDamage;             // +0x38
    float   projectileDamage;       // +0x3C
    int     spClientSeat;           // +0x40
    int     numSeats;               // +0x44
    uint8_t _pad48[0x50 - 0x48];
    float   maxSpeed;               // +0x50
    float   accel;                  // +0x54
    float   rotRate;                // +0x58
    float   rotAccel;               // +0x5C
    float   maxBodyPitch;           // +0x60
    float   maxBodyRoll;            // +0x64
    float   collisionDamage;        // +0x68
    float   collisionSpeed;         // +0x6C
    float   suspensionTravel;       // +0x70
    float   boundsRadius;           // +0x74
    float   boundsHeight;           // +0x78
    float   boundsLength;           // +0x7C
    int     health;                 // +0x80
    uint8_t _pad84[0x180 - 0x84];
    float   turretGunnerVertSpanUp; // +0x180
    float   turretGunnerVertSpanDown; // +0x184
    float   engineSndSpeed;         // +0x188
    uint8_t _pad18C[0x190 - 0x18C];
    math::Position3 mins;           // +0x190
    math::Position3 maxs;           // +0x1A0
    uint8_t _pad1B0[0x25C - 0x1B0];
    char    mMantleHintString[32];  // +0x25C
    int     mMantleHintStringIndex; // +0x27C
    int     vehicleAnimMatrixColumn;  // +0x280
    uint8_t _pad284[0x2EC - 0x284];
    char    nameOverlay[32];        // +0x2EC
    int     inactiveBlowupSeconds;  // +0x30C
};
static_assert(sizeof(vehicle_info_t) == 0x310, "vehicle_info_t size mismatch");
static_assert(offsetof(vehicle_info_t, maxSpeed) == 0x50, "vehicle_info_t::maxSpeed offset mismatch");
static_assert(offsetof(vehicle_info_t, mMantleHintStringIndex) == 0x27C,
              "vehicle_info_t::mMantleHintStringIndex offset mismatch");
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
struct ConfigString {
    InplaceString mName;          // +0x00
    unsigned int  mNumKeyValues;  // +0x04
    InplaceTree<InplaceString, InplaceString> mStringMap;  // +0x08
};
static_assert(sizeof(ConfigString) == 0x10, "ConfigString size mismatch");
class ConfigStringManager {
public:
    unsigned char mData[0x190];
    static ConfigStringManager* sInst;  // ?sInst@ConfigStringManager@@0PAV1@A
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
extern cspField_t s_vehicleFields[73];  // g.o .rdata @ 0xDD6EF0
extern vehicleAnimMap_t* vehicleAnimMaps[6];  // g.o .data @ 0xDD6E6C

// externs
float  AngleNormalize180(float angle);
float  AngleNormalize360(float angle);
float  AngleDelta(float a1, float a2);              // core.o 0x4B9CD0
float  LerpAngle(float a1, float a2, float frac);   // core.o
float  flrand(float min, float max);                // core.o
int    Cvar_VariableIntegerValue(const char* var_name);  // core.o
int    R_CellForPoint(const math::Position3* pos);  // render.o 0x6C52B0
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
    int     index;                // +0x000
    unsigned int internalNameHash; // +0x004
    char*   szInternalName;       // +0x8
    char*   szDisplayName;        // +0xC
    char*   szOverlayName;        // +0x10
    uint8_t _pad14[0xAC - 0x14];
    int     type;                 // +0xAC (weapType_t; WEAPTYPE_BULLET == 0)
    int     weapClass;            // +0xB0 (weapClass_t; WEAPCLASS_TURRET == 7)
    int     slot;                 // +0xB4
    int     bSlotStackable;       // +0xB8
    int     stance;               // +0xBC (weapStance_t)
    int     ammoType;             // +0xC0 (weapAmmoType_t; WEAPAMMOTYPE_UMG == 5)
    int     pickupWithoutSelect;  // +0xC4
    uint8_t _padC8[0x594 - 0xC8];
    char*   szRadiantName;        // +0x594
    char*   szWorldModel;         // +0x598
    char*   szPickupModel;        // +0x59C
    char*   szHudIcon;            // +0x5A0
    uint8_t _pad5A4[0x5A8 - 0x5A4];
    char*   szAmmoIcon;           // +0x5A8
    int     iStartAmmo;           // +0x5AC
    char*   szAmmoName;           // +0x5B0
    int     iAmmoIndex;           // +0x5B4
    char*   szClipName;           // +0x5B8
    int     iClipIndex;           // +0x5BC
    int     iMaxAmmo;             // +0x5C0
    int     iClipSize;            // +0x5C4
    char*   szSharedAmmoCapName;  // +0x5C8
    int     iSharedAmmoCapIndex;  // +0x5CC
    int     iSharedAmmoCap;       // +0x5D0
    int     iDamage;              // +0x5D4
    uint8_t _pad1c0[0x5DC - 0x5D8];
    int     iMinDamagePercent;    // +0x5DC
    int     iDamageInnerRadius;   // +0x5E0
    int     iDamageOuterRadius;   // +0x5E4
    int     iMeleeDamage;         // +0x5E8
    uint8_t _pad1ec[0x5F0 - 0x5EC];
    int     iFireDelay;           // +0x5F0
    uint8_t _pad1f4[0x5F8 - 0x5F4];
    int     iFireTime;            // +0x5F8
    uint8_t _pad2[0x634 - 0x5FC];
    int     iFuseTime;            // +0x634
    uint8_t _pad2b[0x640 - 0x638];
    float   fAdsZoomFov;          // +0x640
    uint8_t _pad644[0x668 - 0x644];
    float   fHipSpreadStandMin;   // +0x668
    float   fHipSpreadDuckedMin;  // +0x66C
    float   fHipSpreadProneMin;   // +0x670
    float   fHipSpreadMax;        // +0x674
    float   fHipSpreadDecayRate;  // +0x678
    float   fHipSpreadFireAdd;    // +0x67C
    float   fHipSpreadTurnAdd;    // +0x680
    float   fHipSpreadMoveAdd;    // +0x684
    float   fHipSpreadDuckedDecay;// +0x688
    float   fHipSpreadProneDecay; // +0x68C
    uint8_t _pad690[0x6E8 - 0x690];
    int     bTwoHanded;           // +0x6E8
    int     bRifleBullet;         // +0x6EC
    int     bSemiAuto;            // +0x6F0
    int     bBoltAction;          // +0x6F4
    int     bADSPositionInfo;     // +0x6F8
    int     bRechamberWhileAds;   // +0x6FC  (disasm PM_CanStartADSAnim)
    int     bCookOffHold;         // +0x700
    int     bNoBounce;            // +0x704
    int     bNoTumble;            // +0x708
    int     bCanMantle;           // +0x70C
    int     bSmoke;               // +0x710
    int     bOffHand;             // +0x714
    int     bCloth;               // +0x718
    int     bClipOnly;            // +0x71C
    int     bWideListIcon;        // +0x720
    int     bADSFire;             // +0x724
    int     bADSOnly;             // +0x728
    int     bAnimateCamReload;    // +0x72C
    int     bAnimateCamMelee;     // +0x730
    int     bAnimateCamFire;      // +0x734
    int     bDoNotDrop;           // +0x738
    int     bCanSpot;             // +0x73C
    int     bHoldToFire;          // +0x740
    char*   szKillIcon;           // +0x744
    int     bWideKillIcon;        // +0x748
    int     bNoPartialReload;     // +0x74C
    int     bSegmentedReload;     // +0x750
    int     iReloadAmmoAdd;       // +0x754
    int     iReloadStartAdd;      // +0x758
    int     bSwirlControl;        // +0x75C
    char*   szAltWeaponName;      // +0x760
    int     iAltWeaponIndex;      // +0x764
    int     iShotCount;           // +0x768
    int     iDropAmmoMin;         // +0x76C
    int     iDropAmmoMax;         // +0x770
    int     iTriggerRadius;       // +0x774
    int     iExplosionRadius;     // +0x778
    int     iExplosionInnerDamage;// +0x77C
    int     iExplosionOuterDamage;// +0x780
    int     iProjectileSpeed;     // +0x784
    int     iProjectileSpeedUp;   // +0x788
    char*   szProjectileModel;    // +0x78C
    uint8_t projExplosion;        // +0x790
    uint8_t _pad7[0x79C - 0x791];
    int     bProjImpactExplode;   // +0x79C
    uint8_t _pad7a0[0x7A4 - 0x7A0];
    int     iProjectileCount;     // +0x7A4
    int     iProjectileRadius;    // +0x7A8
    int     iProjectileDelay;     // +0x7AC
    int     iProjectileSpacingMin;  // +0x7B0
    int     iProjectileSpacingMax;  // +0x7B4
    uint8_t _pad7b0[0x810 - 0x7B8];
    float   fAdsSpread;           // +0x810
    float   fAdsSpreadDucked;     // +0x814
    float   fAdsSpreadProne;      // +0x818
    uint8_t _pad81C[0x860 - 0x81C];
    float   aiDamageMod;          // +0x860
    uint8_t _pad864[0x86C - 0x864];
    float   leftArc;              // +0x86C
    float   rightArc;             // +0x870
    float   topArc;               // +0x874
    float   bottomArc;            // +0x878
    float   accuracy;             // +0x87C
    float   turnSpeed[2];         // +0x880
    float   convergenceTime;      // +0x888
    float   maxRange;             // +0x88C
    uint8_t _pad9[0x898 - 0x890];
    char*   szUseHintString;      // +0x898
    int     iUseHintStringIndex;  // +0x89C
    uint8_t _pad8a0[0x8A4 - 0x8A0];
    float   fFireHeat;            // +0x8A4
    float   fCooldownRate;        // +0x8A8
    uint8_t _pad9b[0x8B4 - 0x8AC];
    float   fBulletConeAngle;     // +0x8B4
    float   fAdsBulletConeAngle;  // +0x8B8
    char*   szScript;             // +0x8BC
    float   fOOPosAnimLength[2];  // +0x8C0
    float   fAnimIKOffsetTime;    // +0x8C8
    float   fAnimIKOffsetForce;   // +0x8CC
    float   fAnimIKOffsetDist;    // +0x8D0
    float   fAnimIKPitchTime;     // +0x8D4
    float   fAnimIKPitchForce;    // +0x8D8
    uint8_t _pad10[0x8EC - 0x8DC];
    struct gdDecal* pDecals[23];  // +0x8EC
};
static_assert(sizeof(weaponFileInfo_t) == 0x948, "weaponFileInfo_t size mismatch");
static_assert(offsetof(weaponFileInfo_t, index) == 0x0, "weaponFileInfo_t::index offset mismatch");
static_assert(offsetof(weaponFileInfo_t, aiDamageMod) == 0x860, "weaponFileInfo_t::aiDamageMod offset mismatch");
static_assert(offsetof(weaponFileInfo_t, bTwoHanded) == 0x6E8, "weaponFileInfo_t::bTwoHanded offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iAltWeaponIndex) == 0x764, "weaponFileInfo_t::iAltWeaponIndex offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iProjectileSpeed) == 0x784, "weaponFileInfo_t::iProjectileSpeed offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iProjectileSpeedUp) == 0x788, "weaponFileInfo_t::iProjectileSpeedUp offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iProjectileDelay) == 0x7AC, "weaponFileInfo_t::iProjectileDelay offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iFireTime) == 0x5F8, "weaponFileInfo_t::iFireTime offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iMinDamagePercent) == 0x5DC, "weaponFileInfo_t::iMinDamagePercent offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iDamageInnerRadius) == 0x5E0, "weaponFileInfo_t::iDamageInnerRadius offset mismatch");
static_assert(offsetof(weaponFileInfo_t, iDamageOuterRadius) == 0x5E4, "weaponFileInfo_t::iDamageOuterRadius offset mismatch");
static_assert(offsetof(weaponFileInfo_t, weapClass) == 0xB0, "weaponFileInfo_t::weapClass offset mismatch");
static_assert(offsetof(weaponFileInfo_t, stance) == 0xBC, "weaponFileInfo_t::stance offset mismatch");
static_assert(offsetof(weaponFileInfo_t, turnSpeed) == 0x880, "weaponFileInfo_t::turnSpeed offset mismatch");
static_assert(offsetof(weaponFileInfo_t, szScript) == 0x8BC, "weaponFileInfo_t::szScript offset mismatch");
static_assert(offsetof(weaponFileInfo_t, slot) == 0xB4, "weaponFileInfo_t::slot offset mismatch");

enum weapSlot_t : int {
    WEAPSLOT_NONE = 0,
    WEAPSLOT_PRIMARY = 1,        // verified vs disasm Drop_Weapon
    WEAPSLOT_PRIMARYB = 2,
    WEAPSLOT_PISTOL = 3,
    WEAPSLOT_GRENADE = 4,
    WEAPSLOT_SMOKE_GRENADE = 5,
    WEAPSLOT_INTERACT = 6,   // verified vs name table szWeapSlotNames
    WEAPSLOT_BINOCS = 7,     // "binocular"
    WEAPSLOT_SATCHEL = 8,    // "flag" (name table quirk)
    WEAPSLOT_SPECIAL = 9,
};
enum {
    WEAPCLASS_TURRET = 7,     // verified vs disasm BG_GivePlayerWeapon
    WEAPCLASS_NON_PLAYER = 9, // verified vs disasm BG_GivePlayerWeapon
    WEAPCLASS_GRENADE = 5,  // verified vs disasm Pickup_Weapon
    WEAPCLASS_LMG = 3,      // verified vs disasm Bullet_Fire_Extended / BG_IsLMGMounted
    WEAPCLASS_SPOTTER = 8,  // verified vs disasm PM_UpdateAimDownSightLerp
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

void   j_nullsub_37(actor_s* pSelf, weaponParms* wp);   // g.o
Entity* Actor_Grenade_IsValidTrajectory(actor_s* pSelf, const float* vFrom,
                                        const float* vVelocity,
                                        const float* vGoal);  // mp_actors.o

// ============================================================================
// turretInfo_t - turret runtime state (0x8C bytes) - verified against IDA
// ============================================================================
struct turretInfo_t {
    uint8_t  inuse;              // +0x00
    uint8_t  turret_state;       // +0x01 (0 idle, 1 aligned, 2 firing)
    uint16_t turret_flags;       // +0x02
    int16_t  fireTime;           // +0x04
    Entity*  manualTarget;       // +0x08
    Entity*  target;             // +0x0C
    float    targetPos[3];       // +0x10
    int      targetTime;         // +0x1C
    float    missTarget[3];      // +0x20
    float    arcmin[2];          // +0x2C
    float    arcmax[2];          // +0x34
    float    defaultPitch;       // +0x3C
    float    defaultYaw;         // +0x40
    int      convergenceTime;    // +0x44
    float    maxRangeSquared;    // +0x48
    sentient_s* detachSentient;  // +0x4C
    char     stance;             // +0x50
    char     prevStance;         // +0x51
    int16_t  prevSentTarget;     // +0x52
    float    accuracy;           // +0x54
    float    userOrigin[3];      // +0x58
    team_t   eTeam;              // +0x64
    float    ambientTargetAngles[2];  // +0x68
    float    pitchCap;           // +0x70
    const void* obstruction;     // +0x74 cdlConvex*
    float    initialYawmin;      // +0x78
    float    initialYawmax;      // +0x7C
    float    heat;               // +0x80
    bool     overheating;        // +0x84
    Handle   overheatEffect;     // +0x88
};
static_assert(sizeof(turretInfo_t) == 0x8C, "turretInfo_t size mismatch");
static_assert(offsetof(turretInfo_t, turret_state) == 0x01, "turretInfo_t::turret_state offset mismatch");
static_assert(offsetof(turretInfo_t, turret_flags) == 0x02, "turretInfo_t::turret_flags offset mismatch");
static_assert(offsetof(turretInfo_t, arcmin) == 0x2C, "turretInfo_t::arcmin offset mismatch");
static_assert(offsetof(turretInfo_t, arcmax) == 0x34, "turretInfo_t::arcmax offset mismatch");
static_assert(offsetof(turretInfo_t, defaultPitch) == 0x3C, "turretInfo_t::defaultPitch offset mismatch");
static_assert(offsetof(turretInfo_t, maxRangeSquared) == 0x48, "turretInfo_t::maxRangeSquared offset mismatch");
static_assert(offsetof(turretInfo_t, detachSentient) == 0x4C, "turretInfo_t::detachSentient offset mismatch");
static_assert(offsetof(turretInfo_t, eTeam) == 0x64, "turretInfo_t::eTeam offset mismatch");
static_assert(offsetof(turretInfo_t, heat) == 0x80, "turretInfo_t::heat offset mismatch");
static_assert(offsetof(turretInfo_t, overheatEffect) == 0x88, "turretInfo_t::overheatEffect offset mismatch");

// turret flags (bits observed via disasm)
enum {
    TURRET_MANUAL = 0x1,
    TURRET_AUTO = 0x2,
    TURRET_HAS_TARGET = 0x4,
    TURRET_MISSING = 0x8,
    TURRET_20 = 0x20,          // manned/active
    TURRET_ON_TARGET = 0x40,   // last shot on target
    TURRET_800 = 0x800,        // view locked to client
    TURRET_USABLE = 0x1000,
};

// g.o turret data (defined in g_turret.cpp / g_globals.cpp)
extern turretInfo_t turretInfo[1];   // 0xED9E08
extern float emissionRate;           // 0xDD8220 (turret overheat particle emission rate)
extern float emissionRate_0;         // 0xDD8254 (vehicle gunner overheat emission rate)
extern float max_intensity;          // 0xDD7FD4 (120.0 - explosion impulse cap)
extern float max_dist2;              // 0xDD7FD8 (176400.0 - explosion falloff distance sq)
extern float radius;                 // 0xDD8208 (30.0 - revive trace radius)
extern float minPitch;               // 0xDD822C (16.0 - tank gunner min pitch)
extern float deltaYAWmaxs;           // 0xDD8238 (140.0)
extern float deltaYAWmins;           // 0xDD8244 (-140.0)
extern float gFireHeatBlur;          // 0xF616EC (cg.o global, blurred by turret fire)
extern float vec3_origin[3];         // core.o q_math.cpp

// turret family (g.o: g_misc.cpp turret block -> g_turret.cpp)
void InvalidateTurretCaches(void);
void G_InitTurrets(void);
void G_SpawnTurret(Entity* self, const char* weaponinfoname);
void G_FreeTurret(Entity* self);
void G_ClientStopUsingTurret(Entity* self);
void SP_turret(Entity* self);
void SP_turret_XAnimPrecache(const char* classname);
int  Turret_FillWeaponParms(Entity* ent, Entity* activator, weaponParms* wp, int barrelNum);
int  turret_IsFiring(Entity* self);
int  turret_IsFiringInternal(int state);
void turret_SetTargetEnt(Entity* self, Entity* pEnt);
int  turret_behind(Entity* self, Entity* other);
int  G_IsTurretUsable(Entity* self, Entity* owner);
int  turret_CanTargetPoint(Entity* self, const math::Position3* vPoint,
                           float* vSource, float* localAngles);
int  turret_CanTargetSentient(Entity* self, sentient_s* sentient,
                              float* vPoint, float* vSource, float* localAngles);
int  turret_aimat_vector(Entity* self, const math::Position3* origin,
                         int bShoot, float* desiredAngles);
void turret_aimat_vector_internal(Entity* self, const math::Position3* origin,
                                  int bShoot, float* desiredAngles);
int  turret_aimat_Sentient(Entity* self, sentient_s* pEnemy, int bShoot, int missTime);
int  turret_aimat_Sentient_Internal(Entity* self, sentient_s* pEnemy, int bShoot,
                                    int missTime, float* desiredAngles);
int  turret_aimat_Ent(Entity* self, Entity* pEnt, int bShoot);
int  turret_isTargetVisable(Entity* self, Entity* target, float* distSqr);
sentient_s* turret_findBestTarget(Entity* self);
int  turret_UpdateTargetAngles(Entity* self, float* desiredAngles, int bManned);
int  turret_ReturnToDefaultPos(Entity* self, int bManned);
int  turret_random_aim(Entity* self);
int  turret_canuse_auto(Entity* self, actor_s* pActor);
int  turret_canuse_manual(Entity* self, actor_s* pActor);
int  turret_canuse(actor_s* pActor, Entity* pTurret);
void turret_think_auto_nonai(Entity* self);
int  turret_think_auto(Entity* self, actor_s* pActor);
int  turret_think_manual(Entity* self, actor_s* pActor);
void turret_think(Entity* self, int msec);
void turret_think_init(Entity* self, int msec);
void turret_think_client(Entity* self);
void turret_controller(Entity* self, int* partBits);
void turret_track(Entity* self, Entity* other);
void turret_shoot(Entity* self, Entity* owner);
void turret_shoot_internal(Entity* self, Entity* other);
void turret_clientaim(Entity* self, Entity* other);
void clamp_playerbehindgun(Entity* self, Entity* other);

// externs pulled in by the turret family
void CG_mg42_DoControllers(Entity* entity, bool playerTurret);        // cg.o 0x6A1380
void Fire_Lead(Entity* ent, Entity* activator, int damage, int bUseAccuracy);  // g.o 0x48F340
void SetClientOrigin(Entity* ent, const float* origin);
void SetClientViewAngle(Entity* ent, const float* angle);
void BG_PlayerStateToEntityState(PlayerState* ps, EntityState* s, int snap);
void BG_PlayerStateToEntityStateExtrapolate(PlayerState* ps, EntityState* s,
                                            int time, int snap);  // game.o
void IntermissionClientEndFrame(Entity* ent);  // g.o 0x456A00
void SpectatorClientEndFrame(Entity* ent);     // g.o 0x456B00
unsigned char BG_GetWeaponIndexForName(const char* pszName);
void Sentient_GetEyePosition(sentient_s* pSelf, float* vEyePosOut);      // mp_actors.o
void Sentient_GetEyePosition(sentient_s* pSelf, math::Position3& vEyePosOut);  // mp_actors.o
void Sentient_GetOrigin(sentient_s* pSelf, float* vOriginOut);           // mp_actors.o
void Sentient_UpdateActualChainPos(sentient_s* pSelf);                   // mp_actors.o
void G_SetClientContents(Entity* pEnt);                                  // g.o
void P_DamageFeedback(Entity* player);                                   // g.o
int  G_GetNonPVSFriendlyInfo(const float* vPosition, int iOldInfo);      // g.o
Entity* G_GetFriendlyIndexActor(int iFriendlyIndex);                     // g.o
team_t Sentient_EnemyTeam(team_t eTeam);
sentient_s* Sentient_FirstSentient(int iTeamFlags);
sentient_s* Sentient_NextSentient(sentient_s* pPrevSentient, int iTeamFlags);
float Actor_CanSeePointEx(actor_s* pSelf, const float* vPoint, float fFovDot,
                          float fMaxDistSqrd,
                          DbLinkedHandle<EntityHandleDb, Entity> ignoreEntity);  // mp_actors.o
bool G_IsPlayerDrivingVehicle(Entity* player);
float VectorDistanceSquared2D(const math::Position3* p1, const math::Position3* p2);
const math::Position3* native_to_cdl_pos3(math::Position3* result, const float* v);
void G_DObjSetLocalTagInternal_0(const float* trans, const float* angles, int bone,
                                 Entity* ent, int a5);

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
int  Pickup_Weapon(Entity* ent, Entity* other, int* piMakeNoise, int bTouched);  // g.o 0x484F20
int  Pickup_Ammo(Entity* ent, Entity* other, int bTouched);          // g.o 0x475890
int  Pickup_Weapon_Ammo(Entity* ent, Entity* other);                 // g.o 0x44B3B0
int  Pickup_Kit(Entity* ent, Entity* other, int bTouched);           // g.o 0x44B3F0

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
float VectorNormalize2D(float* v);
float VectorNormalize(math::Dir3* v);                    // core.o
float VectorNormalize2(const math::Dir3* in, math::Dir3* out);  // core.o
float Q_acos(float c);
void  YawVectors(float yaw, float* forward, float* right);
float vectosignedpitch(const float* vec);
void  vectosignedangles(float* vec, float* angles);
void  G_DObjCalcBone(Entity* ent, int boneIndex);
bool  G_DObjGetWorldBoneIndexMatrix(Entity* ent, int boneIndex, DObjSkelMat* tagMat);
void  RegisterItem(unsigned int iItemIndex, int bUpdateCS);
int   Actor_IsUsingTurret(actor_s* pSelf);
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
bool push_entity(Entity* ent, Entity* vehicle);  // g.o 0x463A20
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
    int  frametime;  // +0x00
    int  time;       // +0x04
    int  oldTime;    // +0x08
    int  cubemapShot; // +0x0C
    int  cubemapSize; // +0x10
    bool teamGame;   // +0x14 (verified vs disasm)
    bool showScore;  // +0x15
    uint8_t _pad16[0x18 - 0x16];
    float gameTime;  // +0x18
    float gameTimeStartTime;  // +0x1C
    int   teamScores[5];      // +0x20
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
void Weapon_MeleeHitShock(Entity* traceEnt);                  // g.o 0x464AC0
bool Actor_IsMeleeInteractable(const actor_s* pSelf);         // mp_actors.o
int  CheckActorInteraction(Entity& ent, const char* interactionName);  // mp_actors.o
int   G_EntDetach(Entity* ent, const char* modelName, const char* tagName);
int   G_DObjGetWorldTagMatrix(Entity* ent, unsigned int tag_name_hash, DObjSkelMat* tagMat);
void  j_nullsub_120(Entity* pGrenade);
Handle PostEffectEventWeapon(const Entity* ent, const char* weaponType, int weaponAction);
Handle PostEffectEventVehicleWheel(const Entity* ent, const char* vehicleType,
                                   int action, int mat_type,
                                   unsigned int wheel_tag_hash);  // core.o 0x4D32D0
struct CollisionDesc;
Handle PostEffectEventProjExplode(const Entity* ent, const char* weaponType,
                                  const CollisionDesc* col_desc);  // core.o
void  EffectEventKill(Handle effect);
int   EffectEventStopEmitting(int effectId);
void  EffectEventAdjustEffect_Scale(Handle effect, const char* param,
                                    float scale);   // core.o 0x4CB8E0
void  Com_Error(int code, const char* fmt, ...);
void  Com_Printf(const char* fmt, ...);
enum {
    ERR_FATAL = 0,
    ERR_DROP = 1,
    ERR_SERVERDISCONNECT = 2,
    ERR_DISCONNECT = 3,
    ERR_NEED_CD = 4,
    ERR_ENDGAME = 5,
    ERR_SCRIPT = 6,
    ERR_LOCALIZATION = 7,
};
void  AngleVectors(const float* angles, float* forward, float* right, float* up);
void  AngleVectors(const math::Position3* angles, float* forward, float* right,
                   float* up);  // core.o q_math.cpp
void  VectorInverse(float* v);  // core.o
void  VectorNormalizeFast(float* v);   // core.o 0x4BDF70
void  AnglesSubtract(const math::Position3* v1, const math::Position3* v2,
                     math::Position3* v3);   // core.o 0x4B99F0
void  ApplyPhysics(Entity* hitEnt, const math::Position3* hitp,
                   const math::Dir3* hitd, float force, bool local_hitp,
                   hitLocation_t hitLoc);   // physics.o 0x70D380
extern vmCvar_t g_debugGrenades;
extern vmCvar_t g_debugBullets;
extern vmCvar_t g_debugProneCheck;             // ?g_debugProneCheck (game.o)
extern vmCvar_t g_debugProneCheckDepthCheck;   // ?g_debugProneCheckDepthCheck (game.o)
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
void RenderQuad2D(float l, float t, float r, float b, float z,
                  const float* col);  // render.o
void RenderText(const char* str, int x, int y, const float* col,
                float depth, float size);  // render.o
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
void  MatrixTransposeTransformVector(const float* in1, const float (*in2)[3],
                                     float* out);  // core.o
void  MatrixTransformVector43(const float* in1, const float (*in2)[3],
                              math::Position3* out);
void  MatrixTransformVector43(const float* in1, const float (*in2)[3],
                              float* out);
void  MatrixMultiply(const float (*in1)[3], const float (*in2)[3],
                     float (*out)[3]);
void  MatrixMultiply43(const float (*in1)[3], const float (*in2)[3],
                       float (*out)[3]);
void  MatrixTranspose(const float (*in)[3], float (*out)[3]);
void  Axis4ToAngles(const float (*axis)[4], float* angles);  // core.o
float vectosignedyaw(float* vec);                 // core.o
void  YawToAxis(float yaw, float (*axis)[3]);     // core.o
XAnimTree* GScr_GetEntAnimTree(Entity* ent);      // g_scr.cpp
void  RotatePointAroundVector(float* result, const float* axis,
                              const float* src, float angle);  // core.o
void  MatrixInverseOrthogonal43(const float (*in)[3], float (*out)[3]);
void  VEH_UpdateControllers(Entity* entity, int msec);  // g.o 0x46DE90
int   VEH_FindValidDismountSpot(Entity* ent, float* mins, float* maxs,
                                float* origin, Entity* player,
                                bool bOriginInput);  // g.o 0x45CE60
void  G_UpdateVehicleTags(Entity* ent);                 // g.o 0x45E290
void  G_FreeVehicle(Entity* ent);                       // g.o 0x46DC50
void  vehicle_InitDynamicBuffers(unsigned short vehicles);  // g.o 0x46FF60
void  G_DrawVehiclePaths(void);                         // g.o 0x464980
void  VP_DrawPath(const vehicle_pathpos_t* vpp);        // g.o 0x464710
void  VP_AddDebugLine(const float* start, const float* end, int forceDraw);  // g.o
void  G_UpdateTagInfoOfChildren(Entity* parent, int bHasDObj);  // g.o 0x4603D0
void  G_UpdateTagInfo(Entity* ent, int bParentHasDObj);  // g.o 0x460330
void  G_UpdateTags(Entity* ent, int bHasDObj);           // g.o 0x464C40
void  G_CalcTagParentAxis(Entity* ent, float (*parentAxis)[3]);  // g.o 0x482270
void  G_CalcTagParentRelAxis(Entity* ent, float (*parentRelAxis)[3]);  // g.o 0x4823C0
void  G_CalcTagAxis(Entity* ent, int bAnglesOnly);      // g.o 0x482440
char* ConcatArgs(int start);                            // g.o 0x839790
void  G_setfog(const char* fogstring);                  // g.o 0x845380
void  SaveRegisteredItems(void);                        // g.o 0x83AA80
void  Scr_FreePrecachedAnimTrees(void);                 // scr.o 0x9B6E10
void  VEH_UnlinkPlayer(Entity* player, bool setOrigin); // g.o 0x86F4B0
void  SV_GameSendServerCommand(DbLinkedHandle<EntityHandleDb, Entity> entityHandle,
                               const char* text);       // sv.o
void  CL_AddDebugString(float* xyz, float* color, float scale, const char* pszText,
                        int fromServer);                // cl.o
void  G_FreeEntity(Entity* e, int msec);                // g.o (g_active.cpp)
cvar_t* Cvar_Get(const char* var_name, const char* var_value, int flags);  // core.o
void    Cvar_Update(vmCvar_t* vmCvar);   // core.o
extern CVarTable gameCvarTable[];  // g.o 0x011C4CF8
extern int gameCvarTableSize;      // g.o
extern cdl_proftimer cdl_proftimer_cvar;  // game.o 0x0133E0C8
Entity* VEH_GetEntity(unsigned int entityHandleVal);  // g.o 0x...
void    FastSinCos(float radians, float* psin, float* pcos);  // core.o
int     VEH_GetVehicleInfo(const char* name);  // g.o 0x44D480 (returns index, -1 if not found)
void    G_VehInitPathPos(vehicle_pathpos_t* vpp);  // g.o 0x... (g_scr_vehicle.cpp)
Entity* G_IsVehicleUnusable(Entity* player);        // g.o 0x46E040
bool    G_IsPlayerVehicleGunner(Entity* player);    // g.o 0x46E200
void    Cmd_Where_f(Entity* ent);                   // g.o 0x456010
int     Cmd_PFXStats_f(void);                       // g.o 0x44AEA0
int     G_EntryPointSeatAssociation(Entity* vehicle, int entryPosition);  // g.o 0x46F9F0
vehicle_info_t* VEH_GetPlayerVehicleInfo(void);     // g.o 0x470490
int16_t VEH_GetPlayerVehicleInfo(const char* name); // g.o 0x44D4E0
int     G_InitScrVehicles(void);                    // g.o 0x45E1D0
int     G_InitialParseInteractionInfo(void);         // g.o
void    G_InitSentients(void);                       // g.o
void    GScr_LoadScriptsAndAnimsForEntities(void);   // g.o
unsigned char GScr_LoadConsts(void);                 // g.o
void    Scr_PrecacheAnimTrees(void* (*alloc)(void*, unsigned int), int restart);  // g.o
AnimTree* Scr_GetAnimTreeByName(const char* treename);  // g.o
void*   Hunk_AllocXAnimCreate(void* self, unsigned int size);  // g.o
void    BG_SetupWeaponInfo(void);                    // game.o
void    ParseHitLocDmgTableEntry(const char* name, const ConfigString* cfg);  // g.o
void    Path_Init(void);                             // g.o
void    G_SetupScrVehicles(void);                    // g.o
void    MP_ResolveAnims(void);                       // mp.o
void    LensFlareInit(void);                         // render.o
int     Swap_Init(void);                             // game.o
void    Rand_Init(unsigned int seed);                // core.o
void    HudElem_Free(game_hudelem_s* hud);           // g.o
void    CG_ClearHudElems(void);                      // cg.o
void    WheelMarkMgr_Reset(void);                    // render.o
void    FX_InitFX(void);                             // fx.o
void    GlobalPakLoadCallback(int progress);         // g.o
int     R_CellForPoint(const float* pos);            // render.o
extern int g_gameIsStartingUp;                       // g.o
extern int g_freeze_movement;                        // g.o
extern Broc::string gFootSplashEffect;               // g.o
extern int g_xanim_num;                              // g.o
extern int s_numNodes;                               // g.o 0xEA5DD0
void    VEH_StopWheelEffects(Entity* ent);          // g.o 0x44DBD0
void    VEH_UpdateWheelParticleEffects(Entity* ent, int wheelIndex);  // g.o 0x45C7F0
void    VEH_UpdateSounds(Entity* ent, int msec);   // g.o 0x46D560
void    VEH_Strcpy(unsigned char* pMember, const char* pKeyValue, int);  // g.o 0x44D350
void    ParseVehicleConfigString(const char* name, const ConfigString* cfgstr);  // g.o 0x44EC90
void    ParseVehiclePhysicsConfigString(const char* name, const ConfigString* cfgstr);  // g.o 0x463730
void    VEH_DebugBox(const math::Position3* pos, float width, float r,
                     float g, float b);             // g.o 0x45C3A0
extern int g_renderPFXStats;                        // game2.o
extern int sEntryPointSeatAssociation[4];           // g.o
extern const char* hintStrings[17];                 // g.o .rdata
extern cvar_t* cg_drawPosition;                     // cg.o
extern char* va(const char* fmt, ...);              // core.o
void  Use_BinaryMover(Entity* ent, Entity* other, Entity* activator);  // g.o (g_mover.cpp)
void  G_Activate(Entity* ent, Entity* activator);   // g.o 0x48CB90
int   GetFollowPlayerState(int clientNum, PlayerState* ps);  // g.o 0x448FE0
int   G_GetVehicleSeatCount(Entity* ent);           // g.o 0x45E930
int   G_GetVehicleOccupantCount(Entity* ent);       // g.o 0x44F2A0
void  Scr_Vehicle_GetOut(Entity* vehicle, Entity* occupant, int health);  // g.o 0x4811F0
void  G_RegisterCvars(void);                        // g.o 0x44B8C0
Entity* SelectNearestDeathmatchSpawnPoint(const float* from);  // g.o 0x850D30
Entity* SelectRandomDeathmatchSpawnPoint(void);     // g.o 0x856440
Entity* SelectSpawnPoint(const float* avoidPoint, float* origin, float* angles);  // g.o 0x467040
void  VEH_UnlinkPlayerDropped(Entity* ent);         // g.o 0x4807D0
Entity* fire_mine(Entity* self, const float* position, const float* dir, int weapon);  // g.o 0x86A5E0
Entity* weapon_mine_fire(Entity* ent, int weapon, weaponParms* wp);  // g.o 0x4818A0
extern int cg_deadscreen_backdrop;    // cg.o vmCvar_t
extern int cg_deadscreen_levelname;   // cg.o
extern int cg_victoryscreen_backdrop; // cg.o
extern int cg_victoryscreen_levelname;// cg.o
void  Cvar_Register(vmCvar_t* vmCvar, const char* varName, const char* defaultValue,
                    int flags);                      // core.o
void  Cvar_VMSet(vmCvar_t* vmCvar, const char* value);  // core.o
void  Cvar_SetValue(const char* var_name, float value);  // core.o
void  G_DebugCircle(const float* center, float radius, const float* color,
                    int depthTest, int onGround, int duration);  // g.o 0x461CB0
void  G_DebugCircleEx(const float* center, float radius, const float* dir,
                      const float* color, int depthTest, int duration);  // cg.o
void  DebugDumpAnims(void);                          // g.o 0x468C60
void  Cmd_LockPVS_f(void);                           // g.o 0x456080
void  ChangePlayersMaxHealth(int newMaxHealth);      // g.o 0x449F60
void  Scr_Vehicle_GetIn(Entity* vehicle, Entity* occupant, int health,
                        unsigned int seatIdx, int entryIdx);  // g.o 0x4918E0
void  G_ReduceAnglesError(float* angles, float* anglesError, float frametime,
                          float angleLerpRate);      // g.o 0x4492B0
void  G_CheckLoadGame(int savegame);                 // g.o 0x458120
void  VEH_RotateWheels(Entity* self, vehicle_info_t* info);  // g.o 0x480CF0
extern int g_dumpAnims;                              // g.o vmCvar
extern int cg_mpDebugAnimEntity;                     // cg.o vmCvar
extern int gLockMeshList;                            // g.o
extern int gEnableMeshFlash;                         // g.o
float AngleNormalize360Accurate(float angle);        // core.o
void  SV_DObjDisplayAnim(Entity* entity);            // sv.o
void  j_nullsub_93(void);                            // g.o
int   HudElem_DestroyAll(void);                      // g.o (g_hudelem.cpp)
extern int TAG_WHEEL_FRONT_LEFT;                     // g.o enum
extern int TAG_WHEEL_FRONT_RIGHT;                    // g.o enum
extern float r;                                      // g.o @ 0xDD8228
extern int VEH_GetWheelOrigin(Entity* ent);          // g.o 0x45C4B0
extern bool collide_segment_poly(const math::Position3& p0,
                                 const math::Position3& p1,
                                 void* poly, void* cinfo);  // game.o
extern void collide_segment(const void* data, const math::Position3& p0,
                            const math::Position3& p1, float* t, int* sflags,
                            int* cflags, void* poly);  // game.o 0x60A290B0
extern void filter_proximity_brushes(const math::Position3& lo,
                                     const math::Position3& hi, int contents,
                                     const proximity_data_t& in,
                                     proximity_data_t& out);  // game.o
extern void UpdateWheelMarks(Entity* owner, int wheel_id, bool wheel_state,
                             const math::Position3& hitp,
                             const math::Dir3& hitn);  // physics.o
extern Handle PostEffectEventVehicle(const Entity* ent,
                                     const char* vehicleType,
                                     int action);  // core.o
extern void j_nullsub_50(void* self);  // g.o
extern int kActionVEHICLE_BRAKE;       // core.o enum (40)
extern int TAG_WHEEL_FRONT_RIGHT;                    // g.o enum
void  EntityManager_DeleteAllEntities(void);         // game.o
void  SceneManager_ResetAllStaticModels(void);       // render.o
void  AnimationPlayer_DebugDump(Entity* ent);        // anim.o
void  Cmd_God_f(Entity* ent);                        // g.o 0x44A7A0
void  Cmd_Notarget_f(Entity* ent);                   // g.o 0x44A850
void  Cmd_Take_f(Entity* ent);                       // g.o 0x44A390
void  misc_EntInfo(Entity* pSelf);                   // g.o 0x4620F0
void  VEH_StopAllEffects(Entity* ent);               // g.o 0x44DC50
void  G_DebugBox(const float* mins, const float* maxs, const float* color,
                 int depthTest, int duration, int fade);  // g.o 0x4570C0
void  G_DebugBox(float* pos, float width, float r, float g, float b,
                 int duration, int fade);            // g.o 0x461B10
void  Cmd_GiveAll_f(Entity* ent);                    // g.o 0x455F60
void  hurt_touch(Entity* self, Entity* other, int bTouched);  // g.o 0x489110
void  G_Trigger(Entity* self, Entity* other);        // g.o (g_trigger.cpp)
int   BG_GivePlayerWeapon(PlayerState* pPS, int iWeaponIndex);  // game.o 0x6168A0
int   Com_BitCheck(const int* array, int bitNum);    // core.o ?Com_BitCheck@@YAHQBHH@Z
int   Add_Ammo(Entity* ent, int weapon, int count, int fillClip);  // g.o 0x44B1A0
void  EntityHandleDb_Compact(void* self);            // g.o 0x454B00
extern bool gNoTargetEnabled;                        // g.o
extern bool gGodModeEnabled;                         // g.o (sv_stubs.h has it)
void  Cmd_Noclip_f(Entity* ent);                     // g.o 0x44A900
void  Svcmd_ListEntities_f(void);                    // g.o 0x465650
int   ConsoleCommand(void);                          // g.o 0x470780
int   G_InitGame(int randomSeed, int restart, int savegame, int checksum);  // g.o 0x476AE0
void  G_RunPreFrame(int msec);                      // g.o
void  G_SendClientMessages(void);                   // g.o
char* game_vmMain(int command, void* arg0, PlayerState* arg1, int arg2,
                  int arg3);                        // g.o 0x48EBF0
void  G_GeneralLink(Entity* ent);                    // g.o 0x482B50
void  ClientThink(DbLinkedHandle<EntityHandleDb, Entity> entityHandle);  // g.o 0x48EB30
void  ClientSpawn(Entity* ent, float* origin, float* angles, bool stopPhysics,
                  void* isRevive);             // g.o 0x491F10
void  ClientEndFrame(Entity* ent, int msec);   // g.o 0x491B60
void  G_setfog(const char* fogstring);               // g.o 0x455E80
void  ClientDisconnect(DbLinkedHandle<EntityHandleDb, Entity> entity);  // g.o 0x467610
vehicle_info_t* G_GetVehicleInfoName(int16_t index); // g.o 0x44F100
void  G_SetFixedLink(Entity* ent, int eAngles);      // g.o (g_utils.cpp)
void  G_SetPlayerFixedLink(Entity* ent);             // g.o 0x482780
int   G_EntAttach(Entity* ent, const char* modelName, const char* tagName,
                  int ignoreCollision, TPakId modelpak);  // g.o 0x482020
void  Svcmd_VehicleList_f(void);                     // g.o
void  Svcmd_EntityList_f(void);                      // g.o
void  SV_GetUsercmd(int clientNum, usercmd_s* cmd);  // sv.o
void  ClientThink_real(Entity* ent);                 // g.o
void  G_RemoveHeadHitEnt(Entity* pSelf);             // g.o
void  G_UpdateHeadHitEnt(Entity* pSelf);             // g.o
void  StopPhysics(Entity* e);                        // g.o
void  Sentient_Free(sentient_s* sentient);           // mp_actors.o
enum {
    CON_DISCONNECTED = 0,
    CON_CONNECTING = 1,
};
void  UpdateLinkedEntities(const ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 1000>* linkedEntities);  // g.o 0x485F20
void  Svcmd_EntityList_f(void);                      // g.o 0x4640D0
void  Scr_Vehicle_Pain(Entity* pSelf, Entity* pAttacker, int damage, const float* point,
                       int mod, const float* dir, hitLocation_t hitLoc);  // g.o 0x45E9B0
void  CalcMuzzlePoints(Entity* ent, weaponParms* wp);  // g.o 0x453620
void  UpdateEntities(ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 1000>* linkedEntities,
                     int msec);                     // g.o 0x48F270
void  G_DebugSphere(const float* center, float radius, const float* color,
                    int density, int depthTest, int duration);  // g.o 0x461BD0
void  UpdateShotProf(float deltaT);                  // g.o 0x4677E0
void  G_ReduceOriginError(float* origin, float* originError, float frametime);  // g.o 0x4491D0
void  G_RunFrameForEntity(Entity* ent, int msec);   // g.o
void  VEH_JoltBody(Entity* ent, math::Position3* dir, float intensity,
                   float speedFrac, float decel);   // g.o
void  CalcMuzzlePoint(Entity* ent, math::Position3* muzzlePoint);  // g.o 0x4534F0
void  G_DebugAxis(const math::Mat43* mat, unsigned int length, int duration);  // g.o 0x456FE0
void  G_LinkClient(Entity* ent);                 // g.o 0x483480
void  Spotting(Entity* ent);                     // g.o 0x472130
void  G_SetAnimTree(Entity* ent, AnimTree* animtree);  // g.o 0x47BB40
void  G_VehicleClientThink(int msec);            // g.o 0x46DF60
bool  ValidForGametype(void);                    // g.o 0x4507D0
void  render_aabb(const math::Position3* bmin, const math::Position3* bmax,
                  const float* color);           // g.o 0x46A290
Client* G_IsVehicleUsable(Entity* ent, Entity* player, bool speedCheck);  // g.o 0x480880
int16_t G_GetVehicleInfoIndex(const char* name); // g.o 0x44F010
extern int s_clientThink;                        // g.o
extern int lastGunnerCrouchMsg;                  // g.o
extern scr_vehicle_t s_phys;                     // g.o
extern int byte_A00000;                          // g.o .data
extern vmCvar_t cg_redFlashTime;                 // cg.o
extern int dword_F64018[4 * 1580];               // cg.o @ 0xF64018
extern void CG_StartShakeCamera(float p, int duration, const float* src,
                                float radius, int client);  // cg.o
extern float radius_1;                           // g.o @ 0xDD8268
extern float abovehead_tresh;                    // g.o @ 0xDD820C
extern float helmetBounce;                       // g.o @ 0xDD8210
extern float helmetFriction;                     // g.o @ 0xDD8214
extern float helmetMass;                         // g.o @ 0xDD8218
extern int   timeToAdd;                          // g.o @ 0xDD821C
extern float decal_radius;                       // g.o @ 0xDD8204
extern float decal_radius_0;                     // g.o @ 0xDD826C
extern float fudge_0;                            // g.o @ 0xDD812C
extern float radius_0;                           // g.o @ 0xDD8264
extern float udelta;                             // g.o @ 0xDD81FC
extern float fdelta;                             // g.o @ 0xDD8200
extern vmCvar_t g_weaponAmmoPools;               // g.o
extern vmCvar_t g_weaponRespawn;                 // g.o
extern vmCvar_t com_timescale;                   // core.o
extern int Sys_Milliseconds(void);               // core.o
extern void level_locals_t_Clear(level_locals_t* self);  // g.o
extern void IGOCompassWidget_SetHideCompassStar(int viewport, int active,
                                                int index);  // shell.o
extern void Client_Clear(void* self, bool clearPersistentAlso,
                          bool clearWeapons);  // g.o
extern Client g_clients[16];                   // g.o
extern sentient_s g_sentients[16];             // g.o

// ============================================================================
// gdDecal / DynamicDecalMgr (render.o) - used by Bullet_Fire_Fake_Extended
// ============================================================================
struct gdDecal {
    float  level1_radius;      // +0x00
    void*  level1_cg_texture;  // +0x04 (nglTexture*)
    void*  level1_ng_texture;  // +0x08
    float  level2_radius;      // +0x0C
    void*  level2_cg_texture;  // +0x10
    void*  level2_ng_texture;  // +0x14
    float  level3_radius;      // +0x18
    void*  level3_cg_texture;  // +0x1C
    void*  level3_ng_texture;  // +0x20
};
static_assert(sizeof(gdDecal) == 0x24, "gdDecal size mismatch");
struct DynamicDecalMgr {
    static void* sInst;  // ?sInst@DynamicDecalMgr@@2PAV1@A @ 0xF74478
    void Add(void* texture, float zBias, bool alphaBlend, int maxNum,
             const math::Position3& pos, const math::Position3& normal,
             float radius, float angle, const float* color,
             bool isHighPriority);  // ?Add@DynamicDecalMgr@@QAEXPAUnglTexture@@M_NHABVPosition3@math@@2MMABVColor@@1@Z
};
void CG_BulletHitEvent(Entity* entity, const math::Position3* origin,
                       const float* normal, int weapon, int surfType,
                       Entity* hitEnt);  // cg.o
void CG_BulletHitClientEvent(
    DbLinkedHandle<EntityHandleDb, Entity> sourceEntity,
    const math::Position3& position, const float* normal, int surfType,
    int weapon);  // cg.o
void CG_EventSpawnTracer(const math::Position3* pstart,
                         const math::Position3* pend, int weapon);  // cg.o
void CG_FireWeapon(Entity* attacker, EntityState* attackerState, int event,
                   int barrel, int inWeapon);  // cg.o
void* controller_inst();                                   // controller_xboxr
int  controller_button_pressed(void* self, int i_controller_num,
                               int i_button);              // controller_xboxr
math::Quaternion nalQuaternionFromMatrix(const math::Mat44& m);  // nal_xboxr
void j_nullsub_117(unsigned int entity, int iClipMask, const float* vOrigin,
                   float fYaw, float* pfCorpsePitch, float* pfCorpseRoll,
                   float* pfCorpseHeight);  // g.o (null)
void  G_RunThink(Entity* ent, int msec);         // g.o
int   XAnimGetAnims(AnimTree* tree);             // anim.o
void* XAnimCreateTree(Entity* ent, AnimTree* anims);  // anim.o
extern const float colorBlue[4];                 // g.o .rdata
struct debug_aabb {
    math::Position3 bmin;  // +0x00
    math::Position3 bmax;  // +0x10
    float color[4];        // +0x20
};
extern ae_vector<debug_aabb> debug_aabbs;        // g.o
bool  CanMantleVehicle(scr_vehicle_t* veh, Entity* player);  // g.o
void  G_DelayFreeAnimTree(XAnimTree* tree);      // g.o (g_dobj.cpp)
void  commit_dobjects(void);                     // g.o (g_dobj.cpp)
void  DObjSetNotRenderedFlag(void);              // g.o (g_dobj.cpp)
void  UpdateCVars(void);                         // g.o (g_main.cpp)
void  ShowEntityInfo(void);                      // g.o (g_main.cpp)
void  G_DrawVehiclePaths(void);                  // g.o
void  G_DrawEntityBBoxes(void);                  // g.o
void  Path_DrawDebug(void);                      // g.o
void  ClientEndFrame(Entity* ent, int msec);     // g.o
void  PlayerAnimMgr_Update(float deltaT);        // game.o
void  Concussive_think(Entity* ent, int msec);   // g.o 0x462A60
Entity* SelectRandomDeathmatchSpawnPoint(void);  // g.o 0x466F40
Entity* SelectNearestDeathmatchSpawnPoint(const float* from);  // g.o 0x461830 (redeclared above)
DbLinkedHandle<EntityHandleDb, Entity> G_GetTankEntNum(int index);  // g.o 0x46E540
void  Cmd_MenuResponse_f(Entity* pEnt);          // g.o 0x4676E0
void  Scr_Vehicle_Die(Entity* pSelf, Entity* pInflictor, Entity* pAttacker,
                      int damage, int mod, int weapon, const float* position,
                      const float* dir, hitLocation_t hitLoc);  // g.o 0x488BE0
void  EntityHandleDb_Validate(void* self);       // g.o 0x466350
extern HashString hash_const_info_player_deathmatch;  // helper (defined in g_globals.cpp)
void  SV_GetConfigstring(int index, char* buffer, int bufferSize);  // sv.o
int   VEH_ParseSpecificField(unsigned char* pStruct, const char* pValue, int fieldType);  // g.o 0x44D370
void  VEH_InvalidateCaches(void);                 // g.o 0x46C9E0
void  LookAtKiller(Entity* self, Entity* inflictor, Entity* attacker);  // g.o 0x456400 (redecl)
void  SpectatorThink(Entity* ent, usercmd_s* ucmd);  // g.o 0x4554E0
Entity* SelectInitialSpawnPoint(float* origin, float* angles);  // g.o 0x4670D0
void  ShowEntityInfo(void);                       // g.o 0x467ED0
void  G_RunFrame(int msec);                       // g.o 0x492600
extern const char* s_vehicleTypeNames[6];         // g.o
extern const char* s_vehicleSubTypeNames[9];      // g.o
void  Pmove(pmove_t* pmove, bool isThisThePredictStep);  // game.o
bool  tunnel_test(pmove_t* pmove, float radius, const float* p0,
                  const float* p1);  // g.o
void  ClientImpacts(Entity* ent, pmove_t* pmove);  // g.o
void  Client_ClaimNode(Entity* ent);  // g.o
void  G_TouchTriggersAndVehicles(Entity* pEnt, const math::Position3* origin,
                                 const void* context);  // g.o
struct player_collision_context_t;  // pmove context (opaque)
extern void (*entinfotable[3])(Entity* ent);      // g.o
float vectoyaw(const float* vec);                 // core.o
struct TouchEntityData;

// CDL collision types (cdl_types.h / cgbank.h) - used by collide_sphere +
// push_in_world
struct cdl_object_t {
    int      cflags;        // +0x00
    int      sflags;        // +0x04
    float    center[3];     // +0x08 (Position3::Packed)
    float    box_radius[3]; // +0x14 (Dir3::Packed)
    float    sphere_radius; // +0x20
};
static_assert(sizeof(cdl_object_t) == 0x24, "cdl_object_t size mismatch");
struct cdl_brush_t {
    uint16_t first_side;  // +0x00
    uint16_t num_sides;   // +0x02
};
struct cdlPlane { int packed[4]; };  // 16 bytes
struct cdl_patch_t {
    uint16_t first_index;  // +0x00
    uint16_t num_inds;     // +0x02
};
struct proxy_obj_t {
    uint16_t oi;  // +0x00
    uint8_t  bi;  // +0x02
    uint8_t  ti;  // +0x03
};
struct cdl_array_t {
    int   m_count;     // +0x00
    void* m_elements;  // +0x04
};

struct proximity_data_t {
    math::Position3 lo;              // +0x000
    math::Position3 hi;              // +0x010
    uint8_t         boxesBuf[0x400]; // +0x020 (256 * proxy_obj_t)
    proxy_obj_t* const* boxes_slot;  // +0x420
    int             boxes_count;     // +0x424
    uint8_t         _pad428[0x430 - 0x428];
    uint8_t         brushesBuf[0x400];  // +0x430 (256 * proxy_obj_t)
    proxy_obj_t* const* brushes_slot;   // +0x830
    int             brushes_count;      // +0x834
    uint8_t         _pad838[0x840 - 0x838];
    uint8_t         poliesBuf[0x1000];  // +0x840 (128 * bounded_proxy_obj_t)
    proxy_obj_t* const* polies_slot;    // +0x1840
    int             polies_count;       // +0x1844
    uint8_t         _pad1848[0x1850 - 0x1848];
    static void* operator new(size_t s, TPakId pakID);  // ??2proximity_data_t@@SAPAXIW4TPakId@@@Z (game.o 0x60BFE0)
    static void operator delete(void* ptr, TPakId pakID);  // ??3proximity_data_t@@SAXPAXW4TPakId@@@Z (game.o 0x60C000)
};
static_assert(sizeof(proximity_data_t) == 0x1850,
              "proximity_data_t size mismatch");
struct CGBank {
    math::Position3 min;      // +0x00
    math::Position3 max;      // +0x10
    math::Position3 center;   // +0x20
    float radius;             // +0x30
    float radius2;            // +0x34
    uint16_t nboxes;          // +0x38
    uint16_t nbrushes;        // +0x3A
    cdl_array_t objects;      // +0x3C
    cdl_array_t brushes;      // +0x44
    cdl_array_t gjk_brushes;  // +0x4C
    cdl_array_t patches;      // +0x54
    cdl_array_t gjk_patches;  // +0x5C
    cdl_array_t brush_sides;  // +0x64
    cdl_array_t brush_verts;  // +0x6C
    cdl_array_t patch_inds;   // +0x74
    cdl_array_t patch_verts;  // +0x7C
    uint8_t _pad84[0xC0 - 0x84];
    void* rtree_data;         // +0xC4
};
static_assert(sizeof(CGBank) == 0xD0, "CGBank size mismatch");
struct CGBankManager : public AssetBankSet {
    static void* sInst;  // ?sInst@CGBankManager@@2PAV1@A
    uint8_t _pad4[8];     // +0x04 mDebugRenderMode
    int mCount;           // +0x0C
    CGBank* mBankArray[99];  // +0x10
    virtual ~CGBankManager();  // ??1CGBankManager@@UAE@XZ (game.o 0x611B70)
    void UnloadAll();     // ?UnloadAll@CGBankManager@@QAEXXZ (game.o)
};

bool collide_sphere_brush(const float* sphere_center, float sphere_radius,
                          const cdl_object_t* obj, const cdlPlane* sides,
                          int nsides, float* new_sphere_center);  // game.o
bool collide_sphere_box(const float* sphere_center, float sphere_radius,
                        const cdl_object_t* box,
                        float* new_sphere_center);  // game.o
bool new_push_out_sphere_triangle(const math::Position3& sphere_center,
                                  float sphere_radius,
                                  const math::Position3& v0,
                                  const math::Position3& v1,
                                  const math::Position3& v2,
                                  const math::Dir3& normal,
                                  math::Position3& new_sphere_center);  // game.o 0x60D860
bool collide_ray_triangle(const math::Position3& p0, const math::Dir3& u0,
                          const math::Position3& v0,
                          const math::Position3& v1,
                          const math::Position3& v2, float cur_t,
                          float* t);  // game.o 0x60D2B0
bool xtest_sphere_triangle(const math::Position3& sphere_center,
                           float sphere_radius,
                           const math::Position3& v0,
                           const math::Position3& v1,
                           const math::Position3& v2,
                           const math::Dir3& normal);  // game.o 0x60D4A0
math::Vector4 calc_normal(const math::Position3& v0, const math::Position3& v1,
                          const math::Position3& v2);  // game.o 0x60C400
bool is_plane_ok(const math::Position3& hitp, const math::Dir3& hitn,
                 unsigned int hitoffs, const math::Dir3& n,
                 unsigned int offs, float radius);   // game.o 0x60C020
bool can_place_decal(const math::Position3& p, const math::Dir3& n,
                     const math::Position3& bmin, const math::Position3& bmax,
                     const cdlPlane* sides, unsigned int nsides,
                     float decal_radius);  // game.o 0x61A900
bool TestPointInBox(const math::Position3& p, const math::Position3& bmin,
                    const math::Position3& bmax);  // game.o 0x61CBD0
int TestPointInBrush(const math::Position3& p, const math::Position3& bmin,
                     const math::Position3& bmax, const cdlPlane* sides,
                     unsigned int nsides);  // game.o 0x61CC30
// cdl_vinfo_t - patch vertex info (10 bytes) - verified against IDA
struct cdl_vinfo_t {
    int16_t  vbase[3];    // +0x00 (signed grid coords)
    uint16_t first_vert;  // +0x06
    uint16_t num_verts;   // +0x08
};
void unpack_poly(const CGBank* bank, const cdl_vinfo_t* vinfo,
                 const unsigned char* pvi, math::Position3& v0,
                 math::Position3& v1,
                 math::Position3& v2);  // game.o 0x6292E0
void unpack(const CGBank* bank, unsigned int pi,
            math::Position3* verts);  // game.o 0x622B60
void CM_ValidateAllWorldSectors();    // game.o 0x633010
int CM_AreaEntities(const math::Position3& mins,
                    const math::Position3& maxs,
                    DbLinkedHandle<EntityHandleDb, Entity>* entityList,
                    int maxcount, int contentmask);  // game.o 0x6331A0
int CM_TransformedPointContents(const math::Position3& p, DCGSet* model,
                                const math::Position3& origin,
                                const math::Position3& angles);  // game.o 0x632E00
void CM_PointTraceToEntities(pointtrace_t* clip,
                             const TouchEntityData& entities);  // game.o 0x633230
void CM_PointTraceToEntities(pointtrace_t* clip,
                             const collision_context_t& context);  // game.o 0x622790
bool intersect_segment_aabb(const math::Position3& p0,
                            const math::Position3& p1,
                            const math::Position3& lo,
                            const math::Position3& hi,
                            const math::Position3& bmin,
                            const math::Position3& bmax);  // game.o 0x61EA00
bool collide_sphere_poly(const math::Position3& c, float r,
                         const math::Position3& v0,
                         const math::Position3& v1,
                         const math::Position3& v2,
                         const math::Vector4& plane);  // game.o 0x61B980
struct traceWork_t;
void TestBoxInBrush(traceWork_t* tw, const math::Position3& bmin,
                    const math::Position3& bmax, const cdlPlane* sides,
                    unsigned int nsides,
                    unsigned int cflags);  // game.o 0x61CD30
void TestBoxInBox(traceWork_t* tw, const math::Position3& bmin,
                  const math::Position3& bmax,
                  unsigned int cflags);  // game.o 0x61D5C0
void TestInLeaf(traceWork_t* tw, const DCGSet* set);  // game.o 0x623D40
unsigned int SightTraceThroughLeaf(traceWork_t* tw,
                                   const DCGSet* set);  // game.o 0x624070
void TracePointThroughLeaf(traceWork_t* tw, const DCGSet* set);  // game.o 0x622FD0
void TraceSphereThroughLeaf(traceWork_t* tw, const DCGSet* set);  // game.o 0x622C80
void TestBoundingBoxInCapsule(traceWork_t* tw);  // game.o 0x623320
int TraceSphereThroughSphere(traceWork_t* tw,
                             const math::Position3& vStart,
                             const math::Position3& vEnd,
                             const math::Position3& vStationary,
                             float radius);  // game.o 0x61C1A0
int TraceCylinderThroughCylinder(traceWork_t* tw,
                                 const math::Position3& vStationary,
                                 float fStationaryHalfHeight,
                                 float radius);  // game.o 0x61C3D0
void TraceCapsuleThroughCapsule(traceWork_t* tw);  // game.o 0x61C690
void TraceBoundingBoxThroughCapsule(traceWork_t* tw);  // game.o 0x61C8D0
DCGSet* TempBoxModel(const math::Position3* mins,
                     const math::Position3* maxs, int contents,
                     int capsule);  // game.o 0x618670
bool collide_velocity_sphere_poly(const math::Position3& c0,
                                  math::Position3& c1,
                                  const math::Dir3& ndir, float r,
                                  const math::Position3& v0,
                                  const math::Position3& v1,
                                  const math::Position3& v2,
                                  const math::Vector4& plane,
                                  bool& insolid);  // game.o 0x61BBB0
char InitEntitiesBSP();  // game.o 0x6199C0
struct leafList_s;
void GetLeaves(leafList_s* ll, unsigned int nodeIndex, float* mindist);  // game.o 0x619050
int CM_BoxLeafnums(math::Vector4& cached_pos, int& cached_leaf,
                   const math::Position3* pos, const math::Position3* mins,
                   const math::Position3* maxs, int* list, int listsize,
                   int* lastLeaf);  // game.o 0x6194A0
void CM_LoadMap(const char* name, int clientload, int* checksum);  // game.o 0x618390
DCGSet* ClipHandleToDCGSet(TPakId pakId, int handle);  // game.o 0x622C00
int CM_LoadLump(int lumpnum, char** pBuf);          // game.o 0x618410
void CM_FreeLump();                                  // game.o 0x6093B0
int CM_NumClusters();                                // game.o 0x6093C0
void CM_ModelBounds(DCGSet* mod, math::Position3& mins,
                    math::Position3& maxs);          // game.o 0x6093D0
void CM_CreateStaticModel(const char* name, TPakId pakId, float*& axis,
                          float*& origin, float*& scale);  // game.o 0x60AF40
void CM_TraceStaticModel(StaticModel* sm, trace_t* results,
                         const math::Position3& start,
                         const math::Position3& end,
                         int contentmask);  // game.o 0x618980
void CM_LinkStaticModel(StaticModel* staticModel);  // game.o 0x60BB20
int  CM_UnlinkStaticModels(TPakId pakId, WorldSector* node);  // game.o 0x60AF90
void CM_DestroyStaticModels(TPakId pakId);  // game.o 0x60B020
void CM_CapsuleAreaEntities(TouchEntityData& entities, WorldSector* node,
                            float p1f, float p2f,
                            const math::Position3& p1,
                            const math::Position3& p2, float radius,
                            const collision_context_t& context);  // game.o 0x619AF0
int CM_ClipSightTraceToEntities(sightclip_t* clip,
                                const collision_context_t& context);  // game.o 0x61A800
int SV_ClipSightToEntity(sightclip_t* clip, EntityShared* check);  // sv.o 0x522450
extern StaticModel* g_static_model;   // ?g_static_model@@3PAVStaticModel@@A (game.o)
void DecodeBin(const char* name, unsigned char* data, int size,
               TPakId pakId);  // game.o 0x6180D0
void RotatePoint(math::Position3& point, math::Position3* matrix);  // game.o 0x60C250
void TransposeMatrix(math::Position3* matrix, math::Position3* transpose);  // game.o 0x60C330
void CreateRotationMatrix(const math::Position3& angles,
                          math::Position3* matrix);   // game.o 0x60C370
float point_to_segment_dist2(const math::Position3& c, const math::Position3& a,
                             const math::Position3& b);  // game.o 0x60E300
int trace_point_through_sphere(const math::Position3& p, const math::Dir3& ud,
                               const math::Position3& ctr, float r, float* t,
                               math::Position3& q);  // game.o 0x60DE10
bool trace_sphere_through_sphere(const math::Position3& c0, float r0,
                                 const math::Position3& c1, float r1,
                                 const math::Dir3& v0,
                                 float* t);  // game.o 0x60DF10
int trace_point_through_cylinder(const math::Position3& sa,
                                 const math::Position3& sb,
                                 const math::Position3& p,
                                 const math::Position3& q, float r,
                                 float* t);  // game.o 0x60DFE0
bool sight_trace_point_patch(const math::Position3* verts,
                             const unsigned char* inds,
                             unsigned short ninds,
                             const math::Position3& p0,
                             const math::Position3& p1,
                             const math::Dir3& dir);  // game.o 0x60DD10

void query_proximity_data(const math::Position3& lo, const math::Position3& hi,
                          proximity_data_t& out);      // game.o 0x60A25BC0
void filter_proximity_data(const math::Position3& lo, const math::Position3& hi,
                           int contents, const proximity_data_t& in,
                           proximity_data_t& out);     // game.o 0x60A0D100
bool push_sphere_in_world(math::Position3& pos, float radius,
                          const proximity_data_t& proximity_data,
                          TouchEntityData* entities);  // game.o 0x60A25FC0
void TracePoint(const proximity_data_t& data, trace_t* results,
                const math::Position3& start, const math::Position3& end,
                int brushmask);  // game.o 0x60A30D80
bool push_in_world(math::Position3& pos, float radius,
                   const proximity_data_t& proximity_data,
                   TouchEntityData& entities);  // g.o 0x46E640
bool push_in_world(pmove_t& pm, float radius,
                   const collision_context_t& context);  // g.o
void prepare_collision_objects(Entity* ent, const math::Position3* p0,
                               const math::Position3* p1, float radius,
                               int mask, proximity_data_t* proximity_data,
                               TouchEntityData* entities);  // g.o 0x45C5F0
bool collide_sphere(Entity* vehicle, const proximity_data_t& proximity_data,
                    const TouchEntityData& entities,
                    const math::Position3& incenter, float radius,
                    math::Position3& outcenter);  // g.o 0x46B3C0
void  DebugDumpEnts(int a1, Entity* e);            // g.o 0x460C50 (redecl above)
void  SpectatorThink(Entity* ent, usercmd_s* ucmd);  // g.o 0x4554E0 (redecl above)
void  G_LoadLevel(void);                           // g.o 0x468D00
void  ClientCommand(DbLinkedHandle<EntityHandleDb, Entity> ent);  // g.o 0x4678C0
void  G_ShutdownGame(int restart);                 // g.o 0x457FE0
bool  SpotWouldTelefrag(const math::Position3* origin);  // g.o 0x466E00
void  G_AddLean(Entity* ent, float* point);        // g.o 0x44A050
void  AddLeanToPosition(float* vPosition, float fViewYaw, float fLeanFrac,
                        float fViewRoll, float fLeanDist);  // game.o
void  MemPrint(const char* fmt, ...);              // core.o
void  DynamicDecalMgr_DestroyAllDecals(void);      // render.o
void  SmokeGrenadeMgr_ReInitialize(void);          // game.o
void  RumbleManager_StopMotors(void* self);        // core.o
void  controller_stop_all_rumble(void* self);      // controller_xboxr
void  EffectEventSys_StopAll(void* self);          // core.o
void  nglWaitForRendering(void);                   // ngl.o
void  nglSetClearFlags(unsigned int flags);        // ngl.o
void  nglInitQuad(void* quad);                     // ngl.o
void  nglSetQuadColor(void* quad, unsigned int color);  // ngl.o
void  nglListAddQuad(void* quad);                  // ngl.o
void  nglPresent(void);                            // ngl.o
void  SpinnerDrawFrame(bool bEndFrame);            // cg.o
void  SpinnerReset(void);                          // cg.o
void  CG_FreeWeapons(void);                        // cg.o
void  BG_FreeWeaponInfo(void);                     // game.o
void  G_FreeInteractionInfo(void);                 // g.o
int   ae_stricmpn(const char* s1, const char* s2, int n);  // core.o ae_string_support.cpp
void  G_RunFrame(int msec);                       // g.o 0x492600
void  SpectatorThink(Entity* ent, usercmd_s* ucmd);  // g.o 0x4554E0
void  Player_UpdateActivate(Entity* ent);         // g.o 0x473C40
void  VP_SetScriptVariable(const char* a1, const char* a2, vehicle_node_t* a3);  // g.o 0x451800
bool  VEH_VehicleTouchesMine(Entity* vehicle, EntityState* item);  // g.o 0x44FB90
void  Scr_Vehicle_Controller(Entity* pSelf);      // g.o 0x480970
void  HealthRegen(Entity* e, float deltaT);       // g.o 0x455380
void  Bullet_Fire(Entity* attacker, float spread, int damage, weaponParms* wp,
                  Entity* weaponEnt, float coneAngleTangent);  // g.o 0x48D980
void  Weapon_ItemHealth_Fire(Entity* ent, int grenType, weaponParms* wp);  // g.o 0x45F630
void  Weapon_ItemAmmo_Fire(Entity* ent, int grenType, weaponParms* wp);    // g.o 0x45FB40
void  G_BulletFireSpread(const Entity* source, Entity* attacker, weaponParms* wp,
                         int damage, float spread, Entity* weaponEnt,
                         float coneAngleTangent, unsigned int seed);  // g.o 0x48D730
void  Weapon_Melee(Entity* ent, weaponParms* wp);  // g.o 0x4891C0
void  FireWeaponMelee(Entity* ent);                // g.o 0x48AA50
void  FireWeapon(Entity* ent);                     // g.o 0x48DAE0
void  ClientEvents(Entity* ent, float oldEventSequence);  // g.o 0x48DE60
void  UpdateAnims(int msec);                      // g.o
void  AdvanceSceneAnims(float delta);             // g.o
void  UpdatePlayer(void);                         // g.o 0x458270
void  UpdateAnims(int msec);                      // g.o 0x4690B0
void  j_nullsub_20(void);                         // g.o
void  UpdateRigidBody(float delta_t);             // g.o
void  ClientEndFrame(Entity* ent, int msec);      // g.o

// cg.o / anim.o cross-object
void  CG_DoControllers(Entity* entity, int* partBits);
void  VEH_UpdateControllers(Entity* entity, int msec);
void  Path_DrawDebug(void);                       // g.o
void  G_DrawVehiclePaths(void);                   // g.o
void  G_DrawEntityBBoxes(void);                   // g.o
void  G_BulletFireSpread(Entity* source, Entity* attacker, weaponParms* wp,
                         int damage, float spread, Entity* weaponEnt,
                         float coneAngleTangent, int seed);  // g.o
int   Player_ActivateCmd(Entity* ent);            // g.o
void  Player_ActivateHoldCmd(Entity* ent);        // g.o
void  MultiplayerMgr_SpreadFire(void* self, Entity* player, float gunPitch,
                                float gunYaw, float* weaponPosition, int weapon,
                                float spread, float coneAngleTangent, int seed);  // mp.o
extern int g_listEntity;                          // g.o
extern vmCvar_t g_performanceTest;                // g.o
extern cdl_proftimer cdl_proftimer_ent_actors;    // game.o
extern cdl_proftimer cdl_proftimer_dobj_anim;     // game.o

// anim.o task-handler system (external; opaque views)
struct TaskHandler;
struct TaskFunctor {
    void* __vftable;
};
struct TaskFunctor1_Anim : TaskFunctor {
    void* fn;
    float deltaT;
};
struct TaskFunctor1_XAnim : TaskFunctor {
    void* fn;
    float deltaT;
};
extern TaskHandler* AnimationUpdateTask_sHandler(void);
extern TaskHandler* XAnimUpdateTask_sHandler(void);
void TaskHandler_Update(TaskHandler* h, float deltaT, TaskFunctor* ftor);
void AnimQueue_ExecuteMatrixQueue(void);
void AnimQueue_ClearMatrixQueue(void);
void DObjUpdateLod(Entity* e);
extern int gCurrentCamera;                        // g.o
extern Camera gCamera[];                          // g.o (stride 0x1F0)
void  InteractionController_Update(void* self, float deltaT);  // cl.o
void  PlayerAnimMgr_Update(float deltaT);          // game.o
void  MultiplayerMgr_Step(void* self, int earlyOutInterval, bool fromThread,
                          bool a_bFromGame);      // mp.o
void* EntityNotifySet_GetNotify(void* self, unsigned int chk);  // core.o
bool  Entity_IsInRagdoll(Entity* ent);            // game.o
void  G_DebugArc(const float* center, float radius, float angle0, float angle1,
                 const float* color, int depthTest, int duration);  // g.o 0x4574B0
void  TossClientItems(Entity* self);              // g.o 0x4835F0
void  G_VehInitPathPos(vehicle_pathpos_t* vpp);   // g.o 0x4526D0
void  G_DebugCircleEx(const float* center, float radius, const float* dir,
                      const float* color, int depthTest, int duration);  // g.o 0x457170
void  G_TouchEnts(Entity* ent, int numtouch, DbLinkedHandle<EntityHandleDb, Entity>* touchents);  // g.o 0x474710
float VectorNormalize2(const float* v, float* out);  // core.o
void  PerpendicularVector(float* dst, const float* src);  // core.o
void  CrossProduct(const float* v1, const float* v2, float* cross);  // core.o
const gitem_s* BG_FindItemForWeapon(int weapon);  // game.o 0x612E70
bool  PM_CanSimulateFiringWeapon(int iWeapon);    // game.o 0x614700
bool  PM_CanStartADSAnim();                       // game.o 0x617120
int   PM_InteruptWeaponWithProneMove();           // game.o 0x6171B0
int   PM_InteruptWeaponWithSprintMove();          // game.o 0x617250
void  PM_StartWeaponAnim(int anim);               // game.o 0x607EC0
void  PM_ContinueWeaponAnim(int anim);            // game.o 0x607F80
void  BG_GetSpreadForWeapon(const PlayerState* ps, int weaponIndex,
                            float* minSpread, float* maxSpread);  // game.o 0x615C90
void  GetSurfaceTypeSounds(const char* pszType, nslWaveID* sounds);  // game.o 0x612DB0
void  MultiplayerMgr_IsLocalPlayer(void* self, Entity* player);  // mp.o
void  MultiplayerMgr_DropWeapon(void* self, int weapon, int netIndex,
                                const math::Position3* position,
                                const math::Dir3* angles,
                                const math::Dir3* velocity, int clipCount,
                                int ammoCount);  // mp.o
const math::Dir3 native_to_cdl_dir3(const float* v);  // ?native_to_cdl_dir3@@YA?BVDir3@math@@QBM@Z (g.o inline)
inline const math::Dir3 native_to_cdl_dir3(const float* v)
{
    math::Dir3 v3;
    v3.v.m128_f32[0] = v[0];
    v3.v.m128_f32[1] = v[1];
    v3.v.m128_f32[2] = v[2];
    return v3;
}
extern void (*touchtable[0xD])(Entity* ent, Entity* other, int bTouched);  // g.o
enum { kItemTypeWeapons = 1 };                    // EDroppedItemTypes
void  G_BulletFireSpread(Entity* source, Entity* attacker, weaponParms* wp,
                         int damage, float spread, Entity* weaponEnt,
                         float coneAngleTangent, int seed);  // g.o
void  Bullet_Fire_Extended(DbLinkedHandle<EntityHandleDb, Entity> sourceEntity,
                           Entity* attacker, const float* start, const float* end,
                           int damage, int recursion, weaponParms* wp,
                           DbLinkedHandle<EntityHandleDb, Entity> weaponEntity,
                           float coneAngleTangent);  // g.o 0x48D980 (same family)
float scr_vehicle_t_GetAverageWheelSpeed(scr_vehicle_t* veh);  // g.o 0x46F4F0
void  G_VehSetUpPathPos(vehicle_pathpos_t* vpp, int16_t nodeIdx);  // g.o 0x452870
void  G_CheckHitTriggerDamage(Entity* pActivator, const math::Position3* vStart,
                              const math::Position3* vEnd, int iDamage,
                              int iMOD);           // g.o 0x470BD0
int   G_SpawnVehicle(Entity* ent, const char* typeName);  // g.o 0x488280
void  VEH_InitEntity(Entity* ent, scr_vehicle_t* veh, int16_t infoIdx);  // g.o
void  VEH_InitVehicle(scr_vehicle_t* veh);         // g.o
void  Activate_trigger_damage(Entity* pEnt, Entity* pOther, int iDamage, int iMOD);  // g.o
int   update_trigger_notifies(void);               // g.o 0x471100
void  SaveCheckpoint(const char* checkpointName, bool calledFromScript);  // g.o
void  SetClientOrigin(Entity* ent, const float* origin);  // g.o 0x449A30
void  G_EntUnlink(Entity* ent);                  // g.o 0x460190
void  G_VehSetSwitchNode(vehicle_pathpos_t* vpp, int16_t srcNodeIdx, uint16_t dstNodeIdx);  // g.o 0x452A10
void  VP_CopyNode(vehicle_node_t* src, vehicle_path_node_t* dst);  // g.o
void  G_GrenadeTouchTriggerDamage(Entity* pActivator, const math::Position3* vStart,
                                  const math::Position3* vEnd, int iDamage,
                                  int iMOD);      // g.o 0x470D70
int   G_CheckPointInsideTriggerMount(Entity* pActivator, float* vStart, int* crouch);  // g.o 0x470F40
void  Client_Touch(Entity* pSelf, Entity* pOther);  // g.o 0x4671F0
void  G_DebugCircle2Ex(const float* center, float radius, const float* dir,
                       const float* color, int depthTest, int duration);  // g.o 0x4572F0
int   CM_PointContents(const math::Position3* p, void* model);  // sv.o
int   GetEntityTouchTriggerType(Entity* pEnt);  // g.o 0x448BE0
int   g_EntityContactCapsule(const math::Position3* mins, const math::Position3* maxs,
                             const Entity* ent);  // g.o 0x450AC0
int   CM_AreaEntities(const math::Position3* mins, const math::Position3* maxs,
                      int* entityList, int maxcount, int contentmask);  // sv.o (redecl)
int   Client_GetPushed(Entity* pSelf, Entity* pOther);  // g.o
void  VEH_InitEntity(Entity* ent, scr_vehicle_t* veh, int16_t infoIdx);  // g.o (redecl)
char* ClientConnect(DbLinkedHandle<EntityHandleDb, Entity> entity);  // g.o 0x4673B0
Entity* G_TestEntityPosition(Entity* ent, const math::Position3* origin);  // g.o 0x4698C0
Entity* weapon_grenadelauncher_fire(Entity* ent, int grenType, weaponParms* wp);  // g.o 0x4816E0
void  StopFollowing(Entity* ent);                // g.o 0x456160
void  Spread_Fire_Fake(Entity* attacker, float gunPitch, float gunYaw,
                       const float* weaponPosition, int weapon, float spread,
                       float coneAngleTangent, unsigned int seed);  // g.o 0x481500
void  Cmd_SetViewpos_f(Entity* ent);             // g.o 0x461930
void  G_AddInvalidatedNode(Entity* pEnt, PathNodes::PathNode* pNode);  // g.o 0x455C10
void  Sentient_Clean(sentient_s* sentient);      // mp_actors.o
sentient_s* Sentient_Alloc(void);                // mp_actors.o
void  Client_Clear(void* client, bool clearPersistentAlso, bool clearWeapons);  // game.o
Entity* fire_grenade(Entity* self, const float* start, const float* dir,
                     int grenadeWPID, int time);  // g.o
void  AnglesToUp(const float* angles, float* up); // core.o
void  bdRandom_setSeed(void* self, unsigned int seed);  // bd
unsigned int bdRandom_nextUInt(void* self);       // bd
void  j_nullsub_87(void* self);                   // g.o
extern int g_doShellShock[16];                    // g.o
extern float delta_0;                             // g.o
int   Client_GetPushed(Entity* pSelf, Entity* pOther);  // g.o
struct bdRandomState { unsigned int v[4]; };      // opaque
PathNodes::PathNode* HandleDbToNode(PathNodes::NodeHandle h);  // helper
void  Bullet_Endpos(float spread, float* end, weaponParms* wp, float randomA,
                    float randomB);               // g.o overload
int   G_FindInvalidatedNode(Entity* pEnt, const PathNodes::PathNode* pNode);  // g.o (g_dobj.cpp)
int   Cmd_FollowCycle_f(Entity* ent, int dir);   // g.o 0x4679F0
int   SV_GetCurrentClientInfo(int clientNum, PlayerState* ps);  // sv.o
void  VEH_RemoveVehicle(void* v);                 // phys_xboxr
void  VEH_LinkPlayer(Entity* ent, Entity* player, int seatIdx, int entryIdx,
                     int fromPos);                 // g.o 0x490A60
void  VEH_UpdateClient(Entity* ent, int msec);    // g.o
void  VEH_VerifyPosition(Entity* ent);            // g.o
void  VEH_UpdateParticlesRBVeh(Entity* ent);      // g.o
void  VEH_UpdateWeapon(Entity* ent);              // g.o
void  VEH_UpdateAim(Entity* ent);                 // g.o
void  VEH_UpdateAltWeapon(Entity* ent, int msec); // g.o
void  VEH_UpdateGunnerWeapon(Entity* ent);        // g.o
void  VEH_UpdateSteering(Entity* ent);            // g.o
void  VEH_UpdateHatch(Entity* ent, int msec);     // g.o
void  VEH_UpdateFollow(Entity* ent);              // g.o
void  VEH_UpdateShaderTime(Entity* ent);          // g.o
void  Scr_Vehicle_Think(Entity* pSelf, int msec); // g.o 0x490ED0
void  VEH_UpdatePath(Entity* ent, int msec);     // g.o 0x47F8B0
void  VEH_UpdateOverHeat(Entity* self, int msec);// g.o 0x47F630
void  ChiefMammalInChargeOfVehicleDamageAndPushOut(Entity* pSelf);  // g.o 0x488420
void  UpdateAnimRoute(scr_vehicle_t* veh, Entity* ent, Entity* player);  // g.o 0x48D200
void  VEH_SetPosition(Entity* ent, const math::Position3* origin,
                      const math::Position3* angles,
                      const float* vel);  // g.o 0x46A370
void  SP_script_vehicle(Entity* pSelf);          // g.o 0x488CE0
void  VEH_Backup(Entity* ent);                   // g.o
int   VP_GetNodeIndex(Broc::string* name, math::Position3* origin);  // g.o
void  VP_GetLookAheadXYZ(const vehicle_pathpos_t* vpp, float* lookXYZ);  // g.o
void  vectoangles(const float* vec, float* angles);  // core.o
int   VP_UpdatePathPos(Entity* pEnt, vehicle_pathpos_t* vpp, float* dir,
                       bool overrideSpeed, int waitNode);  // g.o
void  VP_GetAngles(vehicle_pathpos_t* vpp, float* angles);  // g.o
int   G_VehUpdatePathPos(Entity* pEnt, vehicle_pathpos_t* vpp, bool overrideSpeed,
                         int msec, int waitNode);  // g.o 0x45F030
unsigned int Scr_Vehicle_SeatChange(Entity* occupant, unsigned int newSeatIdx);  // g.o 0x491980
void  VEH_RespawnVehicle(Entity* ent);           // g.o 0x488ED0
void  Cmd_Give_f(Entity* ent);                   // g.o 0x48B2F0
void  SpectatorClientEndFrame(Entity* ent);      // g.o 0x460F00
int   Q_stricmpn(const char* s1, const char* s2, int n);  // core.o
void  Drop_Kit(Entity* pEnt, int iPlayerClass);  // g.o 0x457810
void  G_FindTeams(void);                         // g.o 0x467BD0
void  G_VehiclePopOut(Entity* player);           // g.o 0x48CC10
int   G_GetHintStringIndex(int* piIndex, const char* pszString);  // g.o
extern const char* sEntryPointHintText[6];       // g.o
void  G_Animscripted_Think(Entity* ent);         // g.o 0x466BE0
void  XAnimSetCompleteGoalWeight(XAnimTree* tree, unsigned int animIndex,
                                 float goalWeight, float goalTime, float rate,
                                 unsigned int notifyName, unsigned int notifyType,
                                 void* bRestart);  // anim.o
void  MultiplayerMgr_GetNextDroppedItemID(void* self, void* result, int itemType,
                                          Entity* owner);  // mp.o
void  MultiplayerMgr_DropItem(void* self, int itemType, const math::Position3* position,
                              const math::Dir3* angles, const math::Dir3* velocity,
                              int netIndex, bool scriptFrom, int typeIndex);  // mp.o
enum { kItemTypeMax = 3 };                       // EDroppedItemTypes
void  SP_worldspawn(void);                       // g.o 0x4505A0
bool  G_GetTankIndex(DbLinkedHandle<EntityHandleDb, Entity> entity, int* index,
                     bool* enemy);               // g.o 0x46E310
bool  IsVehicleSpotted(Entity* vehicle);         // g.o
void  Weapon_RocketLauncher_Fire(Entity* ent, float spread, weaponParms* wp,
                                 float lifetime, bool explode);  // g.o 0x481930
Entity* fire_rocket(Entity* self, const float* start, const float* dir, float lifetime);  // g.o
void  gunrandom(float* x, float* y);             // core.o
void  Weapon_ArtilleryStrike_Launch(Entity* ent, weaponParms* wp, float* center,
                                    unsigned int seed);  // g.o 0x481E00
void  fire_artillery(Entity* i_Self, const float* i_StrikePoint, int i_Delay);  // g.o
void  j_nullsub_54(weaponParms* wp, const float* target, float* out);  // g.o
void  j_nullsub_47(weaponParms* wp, const float* target, float* out);  // g.o
int   SmokeGrenadeMgr_EntityCanSeeEntity(void* self, Entity* ent, Entity* targEnt,
                                         float visThreshold);  // game.o
enum {
    WEAPTYPE_BULLET = 0,
    WEAPTYPE_GRENADE = 1,  // verified vs disasm BG_IsCookingOffGrenade
    WEAPTYPE_ITEM = 4,  // verified vs disasm Drop_Weapon
    WEAPTYPE_INTERACT = 6,  // verified vs disasm UpdateAnimRoute
    WEAPTYPE_GAS = 5,   // verified vs disasm PM_StartWeaponAnim
};
void  Scr_Vehicle_Init(Entity* pSelf, int msec); // g.o 0x480AC0
void  VEH_GroundPlant(Entity* ent, int gravity, int msec);  // g.o
struct TouchEntityData {
    int      num;        // +0x00
    uint8_t  _pad4[0x10 - 0x4];
    math::Position3 mins;   // +0x10
    math::Position3 maxs;   // +0x20
    DbLinkedHandle<EntityHandleDb, Entity> touch[128];  // +0x30
};
static_assert(sizeof(TouchEntityData) == 0x230, "TouchEntityData size mismatch");

struct useList_t {
    Entity* ent;   // +0x00
    float   score; // +0x04
};
void  G_DoTouchTriggers(Entity* ent, const math::Position3* origin,
                        TouchEntityData* tData, collision_context_t* context);  // g.o 0x474C90
void  G_TouchVehicles(Entity* ent, const math::Position3* origin,
                      TouchEntityData* tData, collision_context_t* context);  // g.o 0x4748A0
int   Player_GetActivateEnt(Entity* pEnt, useList_t* useList);  // g.o 0x473D90
void  Player_UpdateCursorHints(Entity* ent);                    // g.o 0x482C10
extern bool gGrenadeCanBePickedUp;  // g.o
void  SP_script_model(Entity* pSelf);      // g.o 0x47BD80
float Scr_Vehicle_DamageScale(Entity* pSelf, Entity* pAttacker, Entity* pInflictor,
                              const float* point, int mod);  // g.o 0x44F7E0
void  MultiplayerMgr_ApplyLocalPhysicsToVehicle(void* self, Entity* vehicle,
                                                math::Position3* position,
                                                math::Position3* angles,
                                                float* velocity);  // mp.o
extern unsigned int s_wheelTagHashes[6];         // g.o @ 0xEE62CC
extern unsigned int s_gunnerFlashTagHashes[6];   // g.o @ 0xEE62E4
extern unsigned int s_entryPointTagHashes[6];    // g.o @ 0xEE62F4
extern unsigned int s_flashTagHashes[4];         // g.o @ 0xEE630C
extern unsigned int s_seatTagHashes[6];          // g.o @ 0xEE631C
bool  Entity_has_zone_collision(const void* self);  // game.o
float VectorDistance(const float* v1, const float* v2);  // core.o
void  InteractionController_ClearQueue(void* self);  // cl.o
int   CM_AreaEntities(const math::Position3* mins, const math::Position3* maxs,
                      int* entityList, int maxcount, int contentmask);  // sv.o
void  cFreeList_Shutdown(void* freelist);          // core.o
extern cFreeList<trRefEntity> gRefEntFreeList;     // g.o
extern cFreeList<DObj> gDObjFreeList;              // g.o
extern cFreeList<void> gDSkelFreeList;             // g.o
extern cFreeList<void> gDSkelMaxFreeList;          // g.o
extern cFreeList<void> gDSkel4FreeList;            // g.o
void  G_FreeInteractionInfo(void);                 // g.o
bool  IsPlayerFullySeatedInVehicle(Entity* player);  // cl.o
enum { kItemTypeMines = 0 };                        // EDroppedItemTypes
void* InteractionController_Inst(int instance);      // cl.o
float InteractionController_GetRotation(void* self); // cl.o
void  InteractionController_EndInteraction(void* self, int wasInteracting);  // cl.o
void  InitCvars(int restart);                        // g.o 0x44B950
void  Cmd_UFO_f(Entity* ent);                        // g.o 0x44A9C0
bool  G_IsPlayerInVehicle(Entity* player);           // g.o 0x46E0B0
void  Scr_Vehicle_Use(Entity* pEnt, Entity* pOther);  // g.o 0x480DA0
bool  VEH_AcquirePlayerFollowSlot(Entity* vehicle, Entity* follower);  // g.o 0x44E440
void  VEH_ReleasePlayerFollowSlot(Entity* vehicle, Entity* follower);  // g.o 0x46CF00
const float (*VEH_GetPlayerFollowGoalPosition(const Entity* vehicle,
                                              const Entity* follower))[3];  // g.o 0x44E6A0
void  VEH_GenerateRelativeFormationTable(scr_vehicle_t* veh);  // g.o 0x44DFE0
void  VEH_UpdateFollowFormation(scr_vehicle_t* veh, float requiredDistance);  // g.o 0x44E260
int   VEH_GetGenericDistancedFollowHistoryIndex(scr_vehicle_t* veh,
                                                const float* origin,
                                                float requiredDistance,
                                                int startingIndex);  // g.o 0x44E120
vehicle_node_t* SP_create_info_vehicle_node(void);   // g.o 0x45F210
float Scr_Vehicle_CalcSpeed(const scr_vehicle_t* pVehicle);  // g.o 0x44F3D0
int16_t VP_GetNodeIndex(const Broc::string* name, float* origin);  // g.o 0x451950
float VP_CalcNodeSpeed(int16_t nodeIdx);            // g.o 0x451A50
float VP_CalcNodeLookAhead(int16_t nodeIdx);        // g.o 0x451B60
void  VP_CalcNodeAngles(int16_t nodeIdx, float* angles);  // g.o 0x451C70
void  G_SetupVehiclePaths(float v);                  // g.o 0x452440
void  ClientBegin(DbLinkedHandle<EntityHandleDb, Entity> entity);  // g.o 0x467570
extern cvar_t* g_gameskill;           // g.o (cvar_t* per sv_decl.h)
extern vmCvar_t g_player_maxhealth;   // g.o
extern int cl_aADS[4];                // cl.o
extern int cg_aWeaponSelect[4];       // cg.o
extern int cg_aWeaponSelectTime[4];   // cg.o
extern int cl_stance_ss[4];           // cl.o
extern VehicleNodeAllocator g_vehicleNodeManager;  // g.o
extern const float s_invalidAngles[3];  // g.o .rdata
extern float dword_DD7418;            // g.o .rdata
extern float dword_DD741C;            // g.o .rdata
void  BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm,
                                          PlayerState* ps);  // game.o 0x9F3A40
void  VEH_LinkPlayer(Entity* ent, Entity* player, int seatIdx, int entryIdx,
                     int fromPos);                          // g.o 0x87FF60
void  G_AddPredictableEvent(Entity* ent, int event, int eventParm);   // g.o 0x4541D0
void  VEH_SetupCollmap(Entity* ent);                        // g.o 0x44D8C0
void  G_VehFreePathPos(vehicle_pathpos_t* vpp);             // g.o 0x452840
void  G_DebugLine(const float* start, const float* end, const float* color,
                  int depthTest, int duration);             // g.o 0x456FB0
vehicle_info_t* G_GetVehicleInfo(Entity* ent);              // g.o 0x45E900
void  VEH_PlayerInteractionEntry(Entity* vehicle);           // g.o 0x490EA0
void  G_FreeAnimTreeInstances(void);                         // g.o 0x457F90
void  TeleportPlayer(Entity* player, const float* origin, const float* angles);  // g.o 0x458770
void  G_SetupSpawnPoint(Entity* pEnt);                       // g.o 0x449380
float VEH_GetMaxSpeed(scr_vehicle_t* veh);                   // g.o 0x44F3A0
int   G_ClientCanSpectateTeam(Entity* ent, team_t team);     // g.o 0x448FB0
void  g_AddDebugLine(const float* start, const float* end, const float* color,
                     int depthTest, int duration, int fadeOut);  // g.o 0x450D10
void  ClientIntermissionThink(Entity* ent, usercmd_s* ucmd);  // g.o 0x448F20
void  SP_intermission(Entity* ent);                          // g.o 0x449410
void  Cmd_SetSpawnPoint_f(void);                             // g.o 0x44AAB0
void  G_XAnimUpdateEnt(Entity* ent);                         // g.o 0x4581C0
void  render_sphere(const math::Position3* center, float radius, const float* color);  // g.o 0x46A250
int   G_EntLinkTo(Entity* ent, Entity* parent, const char* tagName);      // g.o 0x48AFF0
int   G_EntLinkTo(Entity* ent, Entity* parent, unsigned int tag_name_hash);  // g.o 0x48B030
int   G_EntLinkToWithOffsetHash(Entity* ent, Entity* parent, unsigned int tag_name_hash,
                                const float* originOffset, const float* anglesOffset,
                                bool useAngles);             // g.o 0x48B070
int   G_EntLinkToWithOffset(Entity* ent, Entity* parent, const char* tagName,
                            const float* originOffset, const float* anglesOffset,
                            bool useAngles);                 // g.o 0x48B160
void  LookAtKiller(Entity* self, Entity* inflictor, Entity* attacker);  // g.o 0x845900
void  player_die(Entity* self, Entity* inflictor, Entity* attacker, int damage,
                 int meansOfDeath, int iWeapon, const float* vPosition,
                 const float* vDir, hitLocation_t hitLoc);    // g.o 0x474FE0
void  Touch_Item(Entity* ent, Entity* other, int bTouched);    // g.o 0x4859C0
Entity* Drop_Weapon(Entity* pEnt, int iWeaponIndex, const char* pszTag);  // g.o 0x475E40
void  Touch_Item_Auto(Entity* ent, Entity* other, int bTouched);  // g.o 0x48B4E0
void  Cmd_Kill_f(Entity* ent);                                 // g.o 0x483560
void  Cmd_DropWeapon_f(Entity* pSelf);                         // g.o 0x4835B0
void  Cmd_LockPVSFlash_f(void);                                // g.o 0x456120
void  Cmd_TextureMip_f(void);                                  // g.o 0x44ABD0
void  SP_sd_axis(Entity* ent);                                 // g.o 0x4499E0
void  G_FreeScrVehicleInfo(void);                              // g.o 0x44EFD0
void  G_FreeScrVehicles(void);                                 // g.o 0x45E240
int   G_FreeVehiclePaths(void);                                // g.o 0x4523F0
int   IsVehicleTank(Entity* ent);                              // g.o 0x44D320
void  UpdatePaths(Entity* ent);                                // g.o 0x44C7E0
void  scr_vehicle_t_CollisionDamage(scr_vehicle_t* veh, Entity* ent,
                                    const math::Position3* pos,
                                    const math::Position3* dir, float intensity);  // g.o 0x4890C0
extern int gEnableMeshFlash;          // g.o 0x...
extern int s_numVehicleInfos;         // g.o
extern bool no_really_delete_it;      // g.o
extern int dword_186A0;               // game.o
extern vmCvar_t mp_gametype;          // mp.o ?mp_gametype@@3UvmCvar_t@@A
void  VehicleNodeAllocator_Initialize(void* self);             // g.o 0x452BC0
bool  Weapon_Revive_Test(Entity* ent, void* wp, Entity** traceEnt);  // g.o 0x... (used by cg)
void  Weapon_Revive(Entity* ent, int grenType, weaponParms* wp);     // g.o 0x4720E0
float Bullet_Endpos(float spread, float* end, const weaponParms* wp);  // g.o 0x... (g_combat.cpp)
void  Bullet_Fire_Fake_Extended(DbLinkedHandle<EntityHandleDb, Entity> sourceEntity,
                                Entity* attacker, const float* start, const float* end,
                                int damage, int recursion, weaponParms* wp,
                                DbLinkedHandle<EntityHandleDb, Entity> weaponEntity,
                                float coneAngleTangent);            // g.o 0x4712A0
void  Bullet_Fire_Fake(Entity* attacker, float spread, int damage,
                       weaponParms* wp, Entity* weaponEnt,
                       float coneAngleTangent);                    // g.o 0x4814A0
void  G_ParseScrVehicleInfo(void);                                 // g.o 0x4639D0
void  ParseVehicleConfigString(const char* szKey, const ConfigString* pCfgStr);        // g.o 0x44EC90
void  ParseVehiclePhysicsConfigString(const char* szKey, const ConfigString* pCfgStr); // g.o 0x463730

struct StatusBar {
    static cvar_t* sStatusBarActive;  // ?sStatusBarActive@StatusBar@@3PAUcvar_t@@A
    static void Init();               // ?Init@StatusBar@@YAXXZ
    static void Render();             // ?Render@StatusBar@@SAXXZ
};

struct MemGraph {
    static void Render();             // ?Render@MemGraph@@SAXXZ
};

namespace AeStringSupport {
extern void CStrToAeStr(char* oBuff, int* oLen, int capacity,
                        const char* src);
extern void SubStr(char* oBuff, int* oLen, const char* src, int begin,
                   int len, int srcCapacity);
}


struct rb_extra_info {
    void* m_rb;  // +0x00 rb_vehicle*
};

enum traction_type_e {
    TRACTION_TYPE_FRONT = 0,
    TRACTION_TYPE_BACK = 1,
    TRACTION_TYPE_ALL_WD = 2,
};

// vehicle_rb_parameter (phys_xboxr) - verified against IDA (size 0xD0)
struct vehicle_rb_parameter {
    float m_speed_max;            // +0x00
    float m_accel_max;            // +0x04
    float m_reverse_scale;        // +0x08
    float m_steer_angle_max;      // +0x0C
    float m_steer_speed;          // +0x10
    float m_wheel_radius;         // +0x14
    float m_susp_spring_k;        // +0x18
    float m_susp_damp_k;          // +0x1C
    float m_susp_adj;             // +0x20
    float m_susp_hard_limit;      // +0x24
    float m_tire_fric_fwd;        // +0x28
    float m_tire_fric_side;       // +0x2C
    float m_tire_fric_brake;      // +0x30
    float m_tire_fric_hand_brake; // +0x34
    float m_body_mass;            // +0x38
    float m_mass_center_delta_x;  // +0x3C
    float m_mass_center_delta_y;  // +0x40
    float m_mass_center_delta_z;  // +0x44
    float m_roll_stability;       // +0x48
    float m_roll_resistance;      // +0x4C
    float m_upright_strength;     // +0x50
    float m_tilt_fakey;           // +0x54
    float m_peel_out_max_speed;   // +0x58
    float m_inertia_scale_x;      // +0x5C
    float m_tire_damp_coast;      // +0x60
    float m_tire_damp_brake;      // +0x64
    float m_tire_damp_hand;       // +0x68
    traction_type_e m_traction_type;  // +0x6C
    char  m_name[64];             // +0x70
    math::Position3 m_bbox_min;   // +0xB0
    math::Position3 m_bbox_max;   // +0xC0

    static vehicle_rb_parameter* GetRBVehParameter(const char* name);  // physics.o
    static vehicle_rb_parameter* AddRBVehParameter(const char* name);  // physics.o
};
static_assert(sizeof(vehicle_rb_parameter) == 0xD0,
              "vehicle_rb_parameter size mismatch");

struct vehicleVarConfig_t {
    const char* name;    // +0x00
    unsigned int offset; // +0x04
};
extern vehicleVarConfig_t sVehicleVarConfig[27];  // g.o @ 0xDD7608
struct rb_vehicle {
    uint8_t _pad[0x250];
    vehicle_rb_parameter* m_parameter;  // +0x250
    float   m_throttle;  // +0x254
    uint8_t _pad258[0x280 - 0x258];
    unsigned int m_flags;  // +0x280
    uint8_t _pad284[0x320 - 0x284];
    struct rigid_body_constraint_wheel* m_wheels[8];  // +0x320

    static int sRenderAllVehicles;  // ?sRenderAllVehicles@rb_vehicle@@2HA (physics.o)
    static void remove_vehicle(rb_vehicle* v);  // ?remove_vehicle@rb_vehicle@@SAXQAV1@@Z physics.o
    static void end_path(rb_vehicle* v);        // physics.o
};

struct rigid_body_constraint_wheel {
    float   m_origin[4];   // +0x00
    float   m_wheel_vel;   // +0x10
    uint8_t _pad14[0x30 - 0x14];
    unsigned int m_wheel_flags;  // +0x30
};
void rb_vehicle_get_velocity(rb_vehicle* self, float* result);        // phys_xboxr
void rb_vehicle_unpause_physics(rb_vehicle* self);                    // phys_xboxr
void rb_vehicle_update_from_network(rb_vehicle* self, math::Position3* position,
                                    math::Position3* angles, math::Dir3* vel,
                                    math::Dir3* aVel);                // phys_xboxr

// Task - task system base (28 bytes) - verified against IDA
struct Task {
    void*       __vftable;         // +0x00
    uint8_t     _dlist[8];         // +0x04
    unsigned int mTaskId;          // +0x0C FourCC
    DbLinkedHandle<EntityHandleDb, Entity> mEntityHandle;  // +0x10
    Handle      mTaskHandle;       // +0x14
    unsigned int mFlags;           // +0x18 Bitmask<unsigned int>

    Task(DbLinkedHandle<EntityHandleDb, Entity> h, unsigned int idTask);  // game.o
    Task(DbLinkedHandle<EntityHandleDb, Entity> h, int idTask);  // game2.o 0x4F9970
    static class PoolAllocator* sAllocator;  // ?sAllocator@Task@@2PAVPoolAllocator@@A @ 0x012F3EA8
};
static_assert(sizeof(Task) == 0x1C, "Task size mismatch");

struct TaskSys {
    static TaskSys* sInst;  // ?sInst@TaskSys@@0V1@A @ 0x012F4120
    void PostTask(Task* t);  // ?PostTask@TaskSys@@QAEXPAVTask@@@Z game2.o
};

struct EntityDeathTask : Task {
    EntityDeathTask(DbLinkedHandle<EntityHandleDb, Entity> h);  // ??0EntityDeathTask@@QAE@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@@Z
    void Update(Entity* e, float delta);    // ?Update@EntityDeathTask@@UAEXPAVEntity@@M@Z
};
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
    static void Initialize(Destructible* self, Entity* ent, bool reInit);
};

struct PhysData {
    InplaceString mName;          // +0x00
    float         mMass;          // +0x04
    float         mBounce;        // +0x08
    float         mFric;          // +0x0C
    void*         mConstraints;   // +0x10 (InplaceVector<PhysConstraint>)
};
static_assert(sizeof(PhysData) == 0x14, "PhysData size mismatch");
struct DestructibleBankManager {
    static void* sInst;
    IVPointer<Destructible> GetDestructible(TPakId pak_id, const char* name);
};
struct PhysDataBankManager {
    static void* sInst;
    IVPointer<PhysData> GetPhysData(TPakId pak_id, const char* name);
};
void CalculatePhysData(Entity* ent, IVPointer<PhysData> physData);  // physics.o 0x7030B0

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

// ============================================================================
// g_trigger.cpp externs
// ============================================================================
void SV_SetBrushModel(Entity* ent);
int  Q_strcasecmp(const char* s1, const char* s2);
char* getBuildNumber();                     // game.o 0x608FE0
float random();
int  Scr_IsSystemActive(unsigned char sys);
void Scr_NotifyFromEnt(Entity* ent, HashString hashValue, Entity* fromEnt);

// g.o data: DObj controller dispatch table @ 0xDD57C0 (anim.o provides funcs)
extern void (*controllertable[4])(Entity* ent, int* partBits);

// DObj - server-side dynamic object (minimal view; full layout in cg_local.h)
struct DObj {
    uint8_t      _pad0[0x70];      // +0x00
    void*        skel;             // +0x70
    uint8_t      _pad74[0x80 - 0x74];  // +0x74 (animToModel, gameId, ignoreCollision)
    IVPointer<XModel> models[8];   // +0x80
    int          mPakId;           // +0xC0 (TPakId)
    IVPointerRaw mPhysData;        // +0xC4
    uint8_t      _padCC[0xCE - 0xCC];
    unsigned char numModels;       // +0xCE
    uint8_t      _padCF[0xD0 - 0xCF];
    Entity*      mEntity;          // +0xD0
    uint8_t      _padD4[0xE4 - 0xD4];
    unsigned int mFlags;           // +0xE4

    void* operator new(size_t s);  // ??2DObj@@SAPAXI@Z (render.o)
    void* operator new(size_t s, void* p) { return p; }  // placement
    void operator delete(void* p); // ??3DObj@@SAXPAX@Z (render.o)
    void operator delete(void* p, size_t) { DObj::operator delete(p); }  // matching placement
    DObj(int pakId);               // ??0DObj@@QAE@W4TPakId@@@Z (render.o)
    ~DObj();                       // ??1DObj@@QAE@XZ (render.o)
    int GetBoneParent(int boneIndex);          // ?GetBoneParent@DObj@@QAEHH@Z (render.o)
    const math::Mat43::Packed& GetBaseRelMat(int boneIndex);  // ?GetBaseRelMat@DObj@@QAEABUPacked@Mat43@math@@H@Z (render.o)
};
static_assert(sizeof(DObj) == 0xE8, "DObj size mismatch");

extern cdl_proftimer cdl_proftimer_dobj_anim;    // game.o 0x0132C318

// g.o DObj tracking vectors (g_utils.cpp)
extern ae_vector<DbLinkedHandle<EntityHandleDb, Entity>> dobjects;            // 0x012C4D14
extern ae_vector<DbLinkedHandle<EntityHandleDb, Entity>> del_pending_dobjects;  // 0x012D4DC0
extern ae_vector<DbLinkedHandle<EntityHandleDb, Entity>> add_pending_dobjects;  // 0x012C1FA4
void G_FreeVehicleRefs(Entity* ent);              // g.o 0x45D3A0 (g_scr_vehicle.cpp)

// core.o (effect_events.cpp) - sound notify
class EffectEventSys {
public:
    static EffectEventSys* sInst;  // ?sInst@EffectEventSys@@2PAV1@A @ 0xF00E80
    void SendSoundNotify(Entity* pEnt);  // ea: 0x004BCDE0
    void SendSpecificSoundNotify(Entity* pEnt, HashString soundName);  // ?SendSpecificSoundNotify@EffectEventSys@@QAEXPAVEntity@@VHashString@@@Z
    void StopEffect(int handle, bool kill);  // ?StopEffect@EffectEventSys@@QAEXVHandle@@_N@Z
    void AdjustEffect_Scale(int handle, const char* param, float scale);  // ?AdjustEffect_Scale@EffectEventSys@@QAEXVHandle@@PBDM@Z
};

namespace BrocSys {
const char* ConvertHashToString(int hash);  // ?ConvertHashToString@BrocSys@@YAPBDH@Z
}
