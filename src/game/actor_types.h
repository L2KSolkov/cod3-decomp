// ============================================================================
// COD3 Actor / Sentient Types
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

#include "game/game_types.h"
#include "engine/broc_types.h"
#include <stdint.h>

// Forward
struct sentient_s;

// ============================================================================
// team_t — team enumeration
// ============================================================================
enum team_t : int32_t {
    TEAM_FREE = 0,
    TEAM_BAD = 0,
    TEAM_AXIS = 1,
    TEAM_ALLIES = 2,
    TEAM_NEUTRAL = 3,
    TEAM_SPECTATOR = 3,
    TEAM_DEAD = 4,
    TEAM_NUM_TEAMS = 5,
};

// ============================================================================
// ai_state_e — AI state enum (4 bytes per slot)
// ============================================================================
enum ai_state_e : int32_t {
    AIS_INVALID = 0,
    AIS_KEEPCURRENT = 0,
    AIS_SETABLE_FIRST = 1,
    AIS_COVER = 1,
    AIS_EXPOSED = 2,
    AIS_TURRET = 3,
    AIS_LMG = 4,
    AIS_NONCOMBAT = 5,
    AIS_GRENADE_RESPONSE = 6,
    AIS_FOLLOW = 7,
    AIS_BLANK = 8,
    AIS_VEHICLE = 9,
    AIS_MOVEAWAY = 10,
    AIS_WOUNDED = 11,
    AIS_DEATH = 12,
    AIS_SETABLE_LAST = 12,
    AIS_PUSHABLE_FIRST = 13,
    AIS_PAIN = 13,
    AIS_CRITICAL_SECTION = 14,
    AIS_SCRIPTEDANIM = 15,
    AIS_CUSTOMANIM = 16,
    AIS_INTERACTION = 17,
    AIS_NEGOTIATION = 18,
    AIS_PUSHABLE_LAST = 18,
    AIS_COUNT = 19,
};

// ============================================================================
// ai_substate_e — AI sub-state
// ============================================================================
enum ai_substate_e : int32_t;

// ============================================================================
// ai_stance_e — stance enum
// ============================================================================
enum ai_stance_e : int32_t {
    STANCE_BAD = 0,
    STANCE_STAND = 1,
    STANCE_CROUCH = 2,
    STANCE_PRONE = 4,
    STANCE_ANY = 7,
};

// ============================================================================
// ============================================================================
// ai_event_t — AI broadcast event enum
// ============================================================================
enum ai_event_t : int32_t {
    AI_EV_BAD = 0,
    AI_EV_FIRST_POINT_EVENT = 1,
    AI_EV_FOOTSTEP = 2,
    AI_EV_FOOTSTEP_LITE = 3,
    AI_EV_NEW_ENEMY = 4,
    AI_EV_PAIN = 5,
    AI_EV_DEATH = 6,
    AI_EV_GRENADE_COOK = 7,
    AI_EV_GRENADE_PING = 8,
    AI_EV_PROJECTILE_PING = 9,
    AI_EV_GUNSHOT = 10,
    AI_EV_EXPLOSION = 11,
    AI_EV_DOOR_OPEN = 12,
    AI_EV_DOOR_KICK = 13,
    AI_EV_LAST_POINT_EVENT = 14,
    AI_EV_FIRST_LINE_EVENT = 15,
    AI_EV_BULLET = 16,
    AI_EV_PROJECTILE_IMPACT = 17,
    AI_EV_LAST_LINE_EVENT = 18,
    AI_EV_NUM_EVENTS = 19,
};

// ============================================================================
// ai_teammove_t — team move result enum
// ============================================================================
enum ai_teammove_t : int32_t {
    AI_TEAMMOVE_INVALID = -1,
    AI_TEAMMOVE_TRAVEL = 0,
    AI_TEAMMOVE_WAIT = 1,
    AI_TEAMMOVE_SLOW_DOWN = 2,
};

// ============================================================================
// ai_orient_mode_t — orientation mode enum
// ============================================================================
enum ai_orient_mode_t : int32_t {
    AI_ORIENT_INVALID = 0,
    AI_ORIENT_DONT_CHANGE = 1,
    AI_ORIENT_TO_MOTION = 2,
    AI_ORIENT_TO_ENEMY = 3,
    AI_ORIENT_TO_ENEMY_OR_MOTION = 4,
    AI_ORIENT_TO_GOAL = 5,
    AI_ORIENT_COUNT = 6,
};

// ============================================================================
// goalRadiusCheck / canChangeState_t / enumLastShot / enumForceSpawn
// ============================================================================
enum goalRadiusCheck : int32_t {
    GOAL_CHECK_PATH = 0,
    GOAL_CHECK_NOTIFY = 1,
};
enum canChangeState_t : int32_t {
    CHANGE_COVER_FORBIDDEN = 0,
    CHANGE_COVER_ALLOWED = 1,
};
enum enumLastShot : int32_t {
    LAST_SHOT_IN_CLIP = 0,
    NOT_LAST_SHOT_IN_CLIP = 1,
};
enum enumForceSpawn : int32_t {
    CHECK_SPAWN = 0,
    CHECK_SPAWN_CONSIDER_PLAYER_FACING = 1,
    FORCE_SPAWN = 2,
};

// ai_orient_t — orientation data (20 bytes)
// ============================================================================
struct ai_orient_t {
    float   yaw;          // +0x00
    float   yawTolerance; // +0x04
    float   targetYaw;    // +0x08
    float   fDesiredBodyYaw;  // +0x0C (verified vs disasm G_MoverTeam)
    int32_t orientType;   // +0x10
};
static_assert(sizeof(ai_orient_t) == 0x14, "ai_orient_t size mismatch");

// ============================================================================
// ai_transition_cmd_t — state transition command (8 bytes)
// ============================================================================
struct ai_transition_cmd_t {
    uint8_t data[8];  // placeholder — exact layout TBD during porting
};

// ============================================================================
// scr_anim_s — script animation handle (4 bytes)
// ============================================================================
struct scr_anim_s {
    uint32_t mHandle;  // +0x00
};
static_assert(sizeof(scr_anim_s) == 4, "scr_anim_s size mismatch");

// ============================================================================
// scr_animscript_t — animation script data (12 bytes)
// ============================================================================
struct scr_animscript_t {
    uint8_t data[12];  // placeholder — exact layout TBD
};
static_assert(sizeof(scr_animscript_t) == 0x0C, "scr_animscript_t size mismatch");

// ============================================================================
// ai_animmode_t — animation mode enum
// ============================================================================
typedef int32_t ai_animmode_t;

// ============================================================================
// ai_traverse_mode_t — traversal mode enum
// ============================================================================
typedef int32_t ai_traverse_mode_t;

// ============================================================================
// actor_prone_info_t — prone state data (24 bytes)
// ============================================================================
struct actor_prone_info_t {
    int   bCorpseOrientation;  // +0x00
    int   iProneTime;          // +0x04
    int   iProneTrans;         // +0x08
    float fTorsoHeight;        // +0x0C
    float fTorsoPitch;         // +0x10
    float fWaistPitch;         // +0x14
    // placeholder — exact layout TBD
};
static_assert(sizeof(actor_prone_info_t) == 0x18, "actor_prone_info_t size mismatch");

// ============================================================================
// ActorLookAt — look-at state (60 bytes)
// ============================================================================
struct ActorLookAt {
    uint8_t data[60];  // placeholder — exact layout TBD
};
static_assert(sizeof(ActorLookAt) == 0x3C, "ActorLookAt size mismatch");

// ============================================================================
// actor_physics_t — actor physics state (304 bytes)
// ============================================================================
struct actor_physics_t {
    math::Position3 vOrigin;        // +0x00
    math::Dir3      vVelocity;      // +0x10
    DbLinkedHandle<EntityHandleDb, Entity> mGroundEntity;  // +0x20
    int     iFootstepTimer;         // +0x24
    int     bHasGroundPlane;        // +0x28
    float   groundplaneSlope;       // +0x2C
    int     iSurfaceType;           // +0x30
    float   vWishDelta[3];          // +0x34
    int     bIsAlive;               // +0x40
    DbLinkedHandle<EntityHandleDb, Entity> mEntity;  // +0x44
    int     ePhysicsType;           // +0x48 (aiphys_t)
    float   fGravity;               // +0x4C
    uint8_t _pad50[0x130 - 0x50];
};
static_assert(sizeof(actor_physics_t) == 0x130, "actor_physics_t size mismatch");

// ============================================================================
// path_t — AI path data (1040 bytes)
// ============================================================================
struct pathpoint_t {
    float vOrigPoint[3];           // +0x00
    float fDir2D[2];               // +0x0C
    float fOrigLength;             // +0x14
    PathNodes::NodeHandle mNodeHandle;  // +0x18
};
static_assert(sizeof(pathpoint_t) == 0x1C, "pathpoint_t size mismatch");

struct path_t {
    pathpoint_t pts[32];           // +0x000 (0x380 bytes)
    uint8_t     tail[0x90];        // +0x380 (rest ported with path batch)
};
static_assert(sizeof(path_t) == 0x410, "path_t size mismatch");

// ============================================================================
// path_trim_t — path trim info (8 bytes)
// ============================================================================
struct path_trim_t {
    uint8_t data[8];  // placeholder
};
static_assert(sizeof(path_trim_t) == 8, "path_trim_t size mismatch");

// ============================================================================
// sentient_s — AI sentient base (368 bytes)
// Size: 0x170 (368 bytes) — verified against IDA
// ============================================================================
struct sentient_s {
    Entity*    pEnt;                               // +0x000
    team_t     eTeam;                               // +0x004
    Entity*    pGoalEnt;                            // +0x008
    float      vGoalPos[3];                         // +0x00C
    float      fGoalRadius;                         // +0x018
    float      fGoalRadiusSqrd;                     // +0x01C
    float      fGoalAngle;                          // +0x020
    float      fGoalAngleTolerance;                 // +0x024
    HashString  hGoalScriptCallback;                // +0x028
    bool       bUsedScriptCallback;                 // +0x02C
    // pad 3
    uint8_t    _pad2D[3];                           // +0x02D
    int32_t    iThreatBias;                         // +0x030
    float      fScariness;                          // +0x034
    int32_t    bIgnoreMe;                           // +0x038
    int32_t    bIgnorePlayer;                       // +0x03C
    int32_t    bIgnorePain;                         // +0x040
    int32_t    bStateChangeBlocked;                 // +0x044
    int32_t    lastShotTime;                        // +0x048
    float      fGrenadeReturnChance;                // +0x04C
    int32_t    iBulletsInClip;                      // +0x050
    int32_t    bAnimScriptedAllowPain;               // +0x054
    float      vMoveAwayAvoidPoint[3];             // +0x058
    float      fMoveAwayDist;                       // +0x064
    sentient_s* pEnemy;                             // +0x068
    int32_t    iEnemyNotifyTime;                    // +0x06C
    int16_t    iAttackerCount;                      // +0x070
    int16_t    sFlags;                              // +0x072
    PathNodes::NodeHandle mClaimedNode;             // +0x074
    int32_t    iLastClaimedNodeTime;                // +0x078
    PathNodes::NodeHandle mActualChainPos;          // +0x07C
    int32_t    iActualChainPosTime;                 // +0x080
    PathNodes::NodeHandle mDesiredChainPos;         // +0x084
    int32_t    iDesiredChainPosTime;                // +0x088
    int32_t    iUpdDesiredChainPosTimeMin;          // +0x08C
    int32_t    iUpdDesiredChainPosTimeMax;          // +0x090
    float      fKeepOldDesiredChainOdds;            // +0x094
    uint16_t   mNearestNode;                        // +0x098
    uint8_t    bNearestNodeValid;                   // +0x09A
    uint8_t    bNearestNodeBad;                     // +0x09B
    int32_t    iSpawnTime;                          // +0x09C
    int32_t    last_damage_mod;                     // +0x0A0
    int32_t    inactivityTime;                      // +0x0A4
    int32_t    inactivitySpectatorTime;             // +0x0A8
    int32_t    inactivityWarning;                   // +0x0AC
    int32_t    inactivitySpectatorWarning;          // +0x0B0
    int32_t    noSpectate;                          // +0x0B4
    bool       mEnableTerrainMappingIK;             // +0x0B8
    // pad 3
    uint8_t    _padB9[3];                           // +0x0B9
    int32_t    mLastAnimIKUpdate;                   // +0x0BC
    float      mLastTerrainMappingFootOffsetZ[2];  // +0x0C0
    int32_t    mLastTerrainMappingFootOffsetZTime;  // +0x0C8
    float      mLastTerrainMappingOriginZ;          // +0x0CC
    float      mLastTerrainMappingPelvisZ;          // +0x0D0
    // pad 12 (to 0xE0 — alignment before Position3 array)
    uint8_t    _padD4[12];                          // +0x0D4
    math::Position3 mLastTerrainMappingPos[2];      // +0x0E0
    float      mLastTerrainMappingTraceZ[2];       // +0x100
    float      mLastTerrainMappingGroundedOffset[2];// +0x108
    math::Position3 mLastTerrainMappingToePos[2];   // +0x110
    bool       mAnimIKUseLastGunOffset;             // +0x130
    // pad 3
    uint8_t    _pad131[3];                          // +0x131
    float      mLastAnimIKGunOffset[4][3];         // +0x134
    float      mLastAnimIKOffsetYaw;                // +0x164
    int32_t    mLastAnimIKOffsetTime;               // +0x168
    // pad to end
    uint8_t    _pad16C[4];                          // +0x16C
};
static_assert(sizeof(sentient_s) == 0x170, "sentient_s size mismatch");
static_assert(offsetof(sentient_s, pEnt) == 0x000, "sentient_s::pEnt offset mismatch");
static_assert(offsetof(sentient_s, eTeam) == 0x004, "sentient_s::eTeam offset mismatch");
static_assert(offsetof(sentient_s, pEnemy) == 0x068, "sentient_s::pEnemy offset mismatch");

// ============================================================================
// vis_cache_t - AI visibility cache (20 bytes) - verified against IDA
// ============================================================================
struct vis_cache_t {
    uint8_t  bVisible;          // +0x00
    uint8_t  _pad1[3];          // +0x01
    float    fVisibility;       // +0x04
    int      iLastUpdateTime;   // +0x08
    int      iLastVisTime;      // +0x0C
    int      iSightHitNum;      // +0x10
};
static_assert(sizeof(vis_cache_t) == 0x14, "vis_cache_t size mismatch");
static_assert(offsetof(vis_cache_t, bVisible) == 0x00, "vis_cache_t::bVisible offset mismatch");

// ============================================================================
// sentient_info_t - per-sentient AI knowledge (80 bytes) - verified IDA
// ============================================================================
struct sentient_info_t {
    math::Position3 vLastKnownPos;      // +0x00
    math::Position3 vKnownFromPos;      // +0x10
    vis_cache_t     VisCache;           // +0x20
    int             iLastAttackMeTime;  // +0x34
    int             iLastKnownPosTime;  // +0x38
    int             iTimeWithoutEnemyInView;  // +0x3C
    int             attackTime;         // +0x40
    PathNodes::NodeHandle mLastKnownNode;      // +0x44
    unsigned int    bLastKnownNodeValid : 1;   // +0x48
    unsigned int    bNeedsVisToPass : 1;       // +0x48
};
static_assert(sizeof(sentient_info_t) == 0x50, "sentient_info_t size mismatch");
static_assert(offsetof(sentient_info_t, vLastKnownPos) == 0x00,
              "sentient_info_t::vLastKnownPos offset mismatch");
static_assert(offsetof(sentient_info_t, VisCache) == 0x20,
              "sentient_info_t::VisCache offset mismatch");
static_assert(offsetof(sentient_info_t, iLastKnownPosTime) == 0x38,
              "sentient_info_t::iLastKnownPosTime offset mismatch");
static_assert(offsetof(sentient_info_t, attackTime) == 0x40,
              "sentient_info_t::attackTime offset mismatch");

// ============================================================================
// sentient_info_array - per-actor view of the sentient info pool (0xC0 bytes)
// ============================================================================
struct sentient_info_array {
    sentient_info_t* mInfos[48];  // +0x00

    sentient_info_t& operator[](int idx) { return *mInfos[idx]; }
    void FreeIndex(int idx);  // mp_actors.o 0x77BE50
};
static_assert(sizeof(sentient_info_array) == 0xC0, "sentient_info_array size mismatch");
// ============================================================================
// BadPlaceArc / BadPathData / BadPathManager - bad-place tracking (mp_actors.o)
// ============================================================================
struct BadPlaceArc {
    float origin[3];     // +0x00
    float radius;        // +0x0C
    float halfheight;    // +0x10
    float angle0;        // +0x14
    float angle1;        // +0x18
};
static_assert(sizeof(BadPlaceArc) == 0x1C, "BadPlaceArc size mismatch");

struct BadPathData {
    float mStartPos[3];  // +0x00
    float mGoalPos[3];   // +0x0C
    float mTime;         // +0x18
};
static_assert(sizeof(BadPathData) == 0x1C, "BadPathData size mismatch");

class BadPathManager {
public:
    int    mTotalNum;       // +0x00
    int16_t mNumBadPaths;   // +0x04
    int16_t mNumQuickExits; // +0x06
    BadPathData mBadPaths[15];  // +0x08

    void Initialize();  // ?Initialize@BadPathManager@@QAEXXZ (mp_actors.o 0x77CBC0)
};
static_assert(sizeof(BadPathManager) == 0x1AC, "BadPathManager size mismatch");


// ============================================================================
// actor_s — full AI actor (2864 bytes)
// Size: 0xB30 (2864 bytes) — verified against IDA (184 members, truncated)
// Key members only; full layout during porting.
// ============================================================================
struct actor_s {
    Entity*        pEnt;                         // +0x000
    sentient_s*    pSentient;                    // +0x004
    ai_state_e     eState[7];                    // +0x008
    int32_t        iStateLevel;                  // +0x024
    int32_t        iStateTime;                   // +0x028
    ai_substate_e  eSubState;                    // +0x02C
    int32_t        iSubStateTime;                // +0x030
    int32_t        lastCoverTime;                // +0x034
    int32_t        changeYawTime;                // +0x038
    ai_transition_cmd_t StateTransitions[15];   // +0x03C
    int32_t        iTransitionCount;             // +0x0B4
    ai_state_e     eSimulatedState[7];          // +0x0B8
    int32_t        iSimulatedStateLevel;         // +0x0D4
    int32_t        iPainTime;                    // +0x0D8
    int32_t        iCritSectTime;                // +0x0DC
    Broc::string   mCritSectName;               // +0x0E0
    int32_t        bAllowDeath;                  // +0x0E4
    int32_t        bDelayedDeath;                // +0x0E8
    int32_t        bWasNegotiation;              // +0x0EC
    float          accuracy;                     // +0x0F0
    float          accuracyStationaryMod;        // +0x0F4
    float          accuracyVsPlayer;             // +0x0F8
    float          accuracyVsAI;                 // +0x0FC
    float          accuracyVsHero;               // +0x100
    int32_t        missCount;                    // +0x104
    uint8_t        lastShotHit;                  // +0x108
    // pad 3
    uint8_t        _pad109[3];                   // +0x109
    float          lastAccuracy;                 // +0x10C
    int32_t        lastTargetTime;               // +0x110
    float          lastTargetLocation[3];       // +0x114
    Broc::string   mProperName;                 // +0x120
    unsigned int   mWeaponName;                  // +0x124
    unsigned int   mSecondaryWeaponName;         // +0x128
    Broc::string   mVoiceName;                  // +0x12C
    Broc::string   mPopedHelmetName;            // +0x130
    int32_t        iTraceCount;                  // +0x134
    float          fLookPitch;                   // +0x138
    float          fLookYaw;                     // +0x13C
    float          vLookForward[3];              // +0x140
    float          vLookRight[3];                // +0x14C
    float          vLookUp[3];                   // +0x158
    ai_orient_t    CodeOrient;                   // +0x164
    ai_orient_t    ScriptOrient;                 // +0x178
    float          fDesiredBodyYaw;              // +0x18C
    float          lastDesiredBodyYaw;           // +0x190
    int32_t        desiredBodyYawTime;           // +0x194
    scr_anim_s     aimLow;                       // +0x198
    scr_anim_s     aimLevel;                     // +0x19C
    scr_anim_s     aimHigh;                      // +0x1A0
    scr_anim_s     shootLow;                     // +0x1A4
    scr_anim_s     shootLevel;                   // +0x1A8
    scr_anim_s     shootHigh;                    // +0x1AC
    scr_anim_s     animProneLow;                 // +0x1B0
    scr_anim_s     animProneLevel;               // +0x1B4
    scr_anim_s     animProneHigh;                // +0x1B8
    scr_anim_s     animWalkRunLoop;              // +0x1BC
    float          currWalkRunLoopAnimRate;      // +0x1C0
    unsigned int   mAnimPose;                    // +0x1C4
    unsigned int   mForcePose;                   // +0x1C8
    float          fInvProneAnimLowPitch;        // +0x1CC
    float          fInvProneAnimHighPitch;       // +0x1D0
    float          fProneLastDiff;               // +0x1D4
    int32_t        bProneOK;                     // +0x1D8
    actor_prone_info_t ProneInfo;                // +0x1DC
    int32_t        iEyeInfoTime;                 // +0x1F4
    float          vEyePos[3];                   // +0x1F8
    float          vEyeDir[3];                   // +0x204
    int32_t        iMuzzleInfoTime;              // +0x210
    float          vMuzzlePos[3];                // +0x214
    float          vMuzzleDir[3];                // +0x220
    ActorLookAt    mLookAt;                      // +0x22C
    int32_t        iDamageType;                  // +0x268
    int32_t        iDamageTaken;                 // +0x26C
    int32_t        iDamageYaw;                   // +0x270
    float          damageDir[3];                 // +0x274
    unsigned int   mDamageHitLoc;                // +0x280
    int32_t        bMeleedByPlayer;              // +0x284
    ai_stance_e    eAllowedStances;              // +0x288
    uint16_t       AnimScriptHandle;             // +0x28C
    unsigned int   AnimBroScriptHandleVal;       // +0x290
    scr_animscript_t* pAnimScriptFunc;           // +0x294
    scr_animscript_t* pDesiredAnimScriptFunc;    // +0x298
    scr_animscript_t  AnimScriptSpecific;        // +0x29C
    ai_traverse_mode_t eTraverseMode;            // +0x2A8
    uint8_t        bRestartAnimScript;           // +0x2AC
    // pad 3
    uint8_t        _pad2AD[3];                   // +0x2AD
    scr_animscript_t* lastDesiredAnimScriptFunc;  // +0x2B0
    ai_animmode_t  lastDesiredAnimMode;          // +0x2B4
    ai_animmode_t  eAnimMode;                    // +0x2B8
    ai_animmode_t  eDesiredAnimMode;             // +0x2BC
    ai_animmode_t  eScriptSetAnimMode;           // +0x2C0
    uint8_t        bUseGoalWeight;               // +0x2C4
    uint8_t        bDesiredUseGoalWeight;        // +0x2C5
    // pad 10 (to +0x2D0)
    uint8_t        _pad2C6[10];                  // +0x2C6
    struct actor_physics_t Physics;              // +0x2D0 (0x130 bytes)
    uint8_t        path_data[0x410];             // +0x400 (path_t)
    float          fPathEndRadius;               // +0x810
    float          fPathEndRadiusSqrd;           // +0x814
    float          fWalkDist;                    // +0x818
    uint8_t        trim_data[8];                 // +0x81C (path_trim_t)
    int32_t        iFollowMin;                   // +0x824
    int32_t        iFollowMax;                   // +0x828
    float          fInterval;                    // +0x82C
    uint8_t        _pad830[0x83C - 0x830];
    actor_s*       pPileUpActor;                 // +0x83C
    Entity*        pPileUpEnt;                   // +0x840
    uint8_t        _pad844[0x84C - 0x844];
    int32_t        bDontAvoidPlayer;             // +0x84C
    int16_t        chainFallback;                // +0x850
    uint8_t        goalRadiusOnly;               // +0x852
    uint8_t        bAtGoal;                      // +0x853
    uint8_t        _pad854[0x8A8 - 0x854];
    float          fVisibilityThreshold;          // +0x8A8
    float          fFovDot;                      // +0x8AC
    float          fMaxSightDistSqrd;            // +0x8B0
    uint8_t        _pad8B4[0x8CC - 0x8B4];
    sentient_info_array sentientInfo;            // +0x8CC (0xC0 bytes)
    uint8_t        _pad98C[0xA2C - 0x98C];
    int            iFollowSlot;                  // +0xA2C
    uint8_t        _padA30[0xA84 - 0xA30];
    int32_t        iSpawnTime;                   // +0xA84
    uint8_t        _padA88[0xA98 - 0xA88];
    uint8_t        mg42stayput;                  // +0xA98
    uint8_t        useable;                      // +0xA99
    uint8_t        _padA9A[0xAA4 - 0xA9A];
    int16_t        iUseHintString;               // +0xAA4
    int16_t        mActorIndex;                  // +0xAA6
    uint8_t        _padAA8[0xAB4 - 0xAA8];
    DbLinkedHandle<EntityHandleDb, Entity> closeEnt;  // +0xAB4
    int32_t        moveHistoryIndex;             // +0xAB8
    // Remaining members (to +0xB30)
    uint8_t        _pad_remaining[0xB30 - 0xABC];
};
static_assert(sizeof(actor_s) == 0xB30, "actor_s size mismatch");
static_assert(offsetof(actor_s, pEnt) == 0x000, "actor_s::pEnt offset mismatch");
static_assert(offsetof(actor_s, pSentient) == 0x004, "actor_s::pSentient offset mismatch");
static_assert(offsetof(actor_s, fFovDot) == 0x8AC, "actor_s::fFovDot offset mismatch");
static_assert(offsetof(actor_s, fMaxSightDistSqrd) == 0x8B0,
              "actor_s::fMaxSightDistSqrd offset mismatch");
static_assert(offsetof(actor_s, sentientInfo) == 0x8CC, "actor_s::sentientInfo offset mismatch");
static_assert(offsetof(actor_s, pPileUpActor) == 0x83C, "actor_s::pPileUpActor offset mismatch");
static_assert(offsetof(actor_s, pPileUpEnt) == 0x840, "actor_s::pPileUpEnt offset mismatch");
static_assert(offsetof(actor_s, iSpawnTime) == 0xA84, "actor_s::iSpawnTime offset mismatch");
static_assert(offsetof(actor_s, mg42stayput) == 0xA98, "actor_s::mg42stayput offset mismatch");
static_assert(offsetof(actor_s, closeEnt) == 0xAB4, "actor_s::closeEnt offset mismatch");
static_assert(offsetof(actor_s, iUseHintString) == 0xAA4, "actor_s::iUseHintString offset mismatch");
