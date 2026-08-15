// ============================================================================
// COD3 Player Types — PlayerState, usercmd_s, gclient_s
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "game/game_types.h"
#include "game/trace_types.h"
#include "core/math_types.h"

// Forward
struct PlayerStateEvents;

// ============================================================================
// PlayerStateEvents — event queue (60 bytes)
// Size: 0x3C (60 bytes) — verified against IDA
// ============================================================================
struct PlayerStateEvents {
    int32_t eventSequence;         // +0x00
    int32_t events[4];            // +0x04
    int32_t eventParms[4];        // +0x14
    int32_t oldEventSequence;     // +0x24
    int32_t damageEvent;          // +0x28
    int32_t damageYaw;            // +0x2C
    int32_t damagePitch;          // +0x30
    int32_t damageCount;          // +0x34
    int32_t entityEventSequence;  // +0x38

    void Clear();  // ?Clear@PlayerStateEvents@@QAEXXZ (g.o 0x4A7560)
};
static_assert(sizeof(PlayerStateEvents) == 0x3C, "PlayerStateEvents size mismatch");

// ============================================================================
// PlayerState — per-client player state (1488 bytes)
// Size: 0x5D0 (1488 bytes) — verified against IDA (112+ members, truncated)
// Key members reconstructed. Full structure will be refined during porting.
// ============================================================================
class PlayerState {
public:
    math::Position3 origin;                // +0x000
    math::Dir3      velocity;              // +0x010
    int32_t  commandTime;                  // +0x020
    int32_t  pm_type;                      // +0x024
    int32_t  bobCycle;                     // +0x028
    int32_t  pm_flags;                     // +0x02C
    int32_t  pm_time;                      // +0x030
    int32_t  weaponTime;                   // +0x034
    int32_t  weaponDelay;                  // +0x038
    int32_t  grenadeTimeLeft;              // +0x03C
    int32_t  iFoliageSoundTime;            // +0x040
    int32_t  iFatigueSoundTime;            // +0x044
    int32_t  gravity;                      // +0x048
    float    leanf;                        // +0x04C
    int32_t  speed;                        // +0x050
    int32_t  delta_angles[3];              // +0x054
    DbLinkedHandle<EntityHandleDb, Entity> mGroundEntity;  // +0x060
    float    vLadderVec[3];               // +0x064
    int32_t  jumpTime;                     // +0x070
    float    fJumpOriginZ;                 // +0x074
    int32_t  legsAnim;                     // +0x078
    float    legsYaw;                      // +0x07C
    int32_t  torsoAnim;                    // +0x080
    int32_t  spotTime;                     // +0x084
    int32_t  respawnUntilTime;             // +0x088
    DbLinkedHandle<EntityHandleDb, Entity> mLastSpotter;     // +0x08C
    DbLinkedHandle<EntityHandleDb, Entity> mKiller;          // +0x090
    DbLinkedHandle<EntityHandleDb, Entity> mTarget;          // +0x094
    int32_t  mTargetTime;                  // +0x098
    int32_t  movementDir;                  // +0x09C
    DbLinkedHandle<EntityHandleDb, Entity> mClient;          // +0x0A0
    int32_t  weapon;                       // +0x0A4
    int32_t  weaponstate;                  // +0x0A8
    float    fWeaponPosFrac;               // +0x0AC
    bool     reloadFromEmpty;              // +0x0B0
    // pad 3
    uint8_t  _padB1[3];                    // +0x0B1
    Handle   queuedReloadSound;            // +0x0B4
    bool     queuedReloadSoundPlayStarted; // +0x0B8
    // pad 3
    uint8_t  _padB9[3];                    // +0x0B9
    int32_t  queuedReloadTimer;            // +0x0BC
    bool     reloadSoundPrequeueAttempted; // +0x0C0
    // pad 3
    uint8_t  _padC1[3];                    // +0x0C1
    int32_t  lastWeapon;                   // +0x0C4
    IVPointer<XModel> viewmodel;          // +0x0C8
    float    viewangles[3];               // +0x0D0
    int32_t  viewHeightTarget;             // +0x0DC
    float    viewHeightCurrent;            // +0x0E0
    int32_t  viewHeightLerpTime;           // +0x0E4
    int32_t  viewHeightLerpTarget;         // +0x0E8
    int32_t  viewHeightLerpDown;           // +0x0EC
    float    viewHeightLerpPosAdj;         // +0x0F0
    int32_t  eFlags;                       // +0x0F4
    PlayerStateEvents event;               // +0x0F8
    int32_t  stats[4];                     // +0x134
    int32_t  ammo[92];                     // +0x144
    int32_t  ammoclip[92];                 // +0x2B4
    int32_t  weapons[2];                   // +0x424
    char     weaponslots[10];              // +0x42C
    // pad 2
    uint8_t  _pad436[2];                   // +0x436
    int32_t  weaponrechamber[2];           // +0x438
    float    mins[3];                      // +0x440
    float    maxs[3];                      // +0x44C
    int32_t  proneViewHeight;              // +0x458
    int32_t  crouchViewHeight;             // +0x45C
    int32_t  standViewHeight;              // +0x460
    int32_t  deadViewHeight;               // +0x464
    float    walkSpeedScale;               // +0x468
    float    runSpeedScale;                // +0x46C
    float    sprintSpeedScale;             // +0x470
    float    proneSpeedScale;              // +0x474
    float    crouchSpeedScale;             // +0x478
    float    strafeSpeedScale;             // +0x47C
    float    backSpeedScale;               // +0x480
    float    leanSpeedScale;               // +0x484
    float    proneDirection;               // +0x488
    float    proneDirectionPitch;          // +0x48C
    float    proneTorsoPitch;              // +0x490
    float    fatigueScale;                 // +0x494
    int32_t  lastSprintTime;               // +0x498
    int32_t  viewlocked;                   // +0x49C
    DbLinkedHandle<EntityHandleDb, Entity> mViewLockedEntity;  // +0x4A0
    float    friction;                     // +0x4A4
    int32_t  serverCursorHint;             // +0x4A8
    int32_t  serverCursorHintVal;          // +0x4AC
    int32_t  serverCursorHintString;       // +0x4B0
    // pad 12 (0x4B4 .. 0x4C0)
    uint8_t  _pad4B4[12];                  // +0x4B4
    trace_t  serverCursorHintTrace;        // +0x4C0 (80 bytes)
    int32_t  iCompassFriendInfo;           // +0x510
    int32_t  iCompassTankInfo;             // +0x514
    float    fTorsoHeight;                 // +0x518
    float    fTorsoPitch;                  // +0x51C
    float    fWaistPitch;                  // +0x520
    int32_t  vehPos;                       // +0x524
    int32_t  vehType;                      // +0x528
    int32_t  vehSubType;                   // +0x52C
    int32_t  weapAnim;                     // +0x530
    float    aimSpreadScale;               // +0x534
    int32_t  shellshockIndex;              // +0x538
    int32_t  shellshockTime;               // +0x53C
    int32_t  shellshockDuration;           // +0x540
    float    mTimeSinceDamage;             // +0x544
    float    mHealthDelta;                 // +0x548
    int16_t  ctf_has_flag;                 // +0x54C
    int32_t  spectatorClient;              // +0x550
    int32_t  mDamageFromPlayers[16];       // +0x554
    int32_t  mLastFireWeaponTime;          // +0x594
    int32_t  mLastFireWeapon;              // +0x598
    int32_t  mAmmoDropTime;                // +0x59C
    int32_t  prevTargetPointValid;         // +0x5A0
    DbLinkedHandle<EntityHandleDb, Entity> currentTargetHandle;  // +0x5A4
    float    prevTargetRelPt[3];           // +0x5A8
    float    mClosestStickyAimDistance;    // +0x5B4
    DbLinkedHandle<EntityHandleDb, Entity> mMeleeAssistTarget;   // +0x5B8
    int16_t  mMeleeAssistSpeed;            // +0x5BC
    float    mHoldBreathScale;             // +0x5C0
    int32_t  mHoldBreathTimer;             // +0x5C4
    uint32_t mFlags;                       // +0x5C8 (Bitmask<unsigned int>)
};
static_assert(sizeof(PlayerState) == 0x5D0, "PlayerState size mismatch");
static_assert(offsetof(PlayerState, origin) == 0x000, "PlayerState::origin offset mismatch");
static_assert(offsetof(PlayerState, weapon) == 0x0A4, "PlayerState::weapon offset mismatch");
static_assert(offsetof(PlayerState, eFlags) == 0x0F4, "PlayerState::eFlags offset mismatch");
static_assert(offsetof(PlayerState, event) == 0x0F8, "PlayerStateEvents offset mismatch");
static_assert(offsetof(PlayerState, serverCursorHint) == 0x4A8, "PlayerState::serverCursorHint offset mismatch");
static_assert(offsetof(PlayerState, serverCursorHintTrace) == 0x4C0, "PlayerState::serverCursorHintTrace offset mismatch");
static_assert(offsetof(PlayerState, vehPos) == 0x524, "PlayerState::vehPos offset mismatch");
static_assert(offsetof(PlayerState, mFlags) == 0x5C8, "PlayerState::mFlags offset mismatch");

// ============================================================================
// usercmd_s — user input command (48 bytes)
// Size: 0x30 (48 bytes) — verified against IDA
// ============================================================================
struct usercmd_s {
    int32_t serverTime;      // +0x00
    int32_t buttons;         // +0x04
    int32_t weapon;          // +0x08
    int32_t angles[3];       // +0x0C
    char    forwardmove;     // +0x18
    char    rightmove;       // +0x19
    char    upmove;          // +0x1A
    // pad 1
    uint8_t _pad1B;          // +0x1B
    float   gunPitch;        // +0x1C
    float   gunYaw;          // +0x20
    float   gunXOfs;         // +0x24
    float   gunYOfs;         // +0x28
    float   gunZOfs;         // +0x2C
};
static_assert(sizeof(usercmd_s) == 0x30, "usercmd_s size mismatch");
static_assert(offsetof(usercmd_s, serverTime) == 0x00, "usercmd_s::serverTime offset mismatch");

// ============================================================================
// pmove_t — player movement context (336 bytes)
// Size: 0x150 (336 bytes) — verified against IDA
// ============================================================================
struct pmove_t {
    PlayerState* ps;                          // +0x00
    usercmd_s    cmd;                         // +0x04 (48 bytes)
    usercmd_s    oldcmd;                      // +0x34 (48 bytes)
    int          tracemask;                   // +0x64
    int          debugLevel;                  // +0x68
    float        vehicleAngles[3];            // +0x6C
    float        vehicleViewClamp[3];         // +0x78
    int          numtouch;                    // +0x84
    uint8_t      touchents[128];              // +0x88 (32x DbLinkedHandle, 4 bytes each)
    math::Position3 mins;                     // +0x110
    math::Position3 maxs;                     // +0x120
    uint8_t      watertype;                   // +0x130
    uint8_t      waterlevel;                  // +0x131
    // pad 2
    uint8_t      _pad132[2];                  // +0x132
    float        xyspeed;                     // +0x134
    int          pmove_fixed;                 // +0x138
    int          pmove_msec;                  // +0x13C
    // function pointers
    void (__cdecl* trace)(struct trace_t*, const math::Position3&, const math::Position3&,
                          const math::Position3&, const math::Position3&, const struct collision_context_t&);  // +0x140
    void (__cdecl* boxtrace)(struct trace_t*, const math::Position3&, const math::Position3&,
                             const math::Position3&, const math::Position3&, const struct collision_context_t&);  // +0x144
    void (__cdecl* capsuletrace)(struct trace_t*, const math::Position3&, const math::Position3&,
                                 const math::Position3&, const math::Position3&, const struct collision_context_t&);  // +0x148
    int (__cdecl* pointcontents)(const math::Position3&, const struct collision_context_t&);  // +0x14C
};
static_assert(sizeof(pmove_t) == 0x150, "pmove_t size mismatch");
static_assert(offsetof(pmove_t, ps) == 0x00, "pmove_t::ps offset mismatch");
static_assert(offsetof(pmove_t, cmd) == 0x04, "pmove_t::cmd offset mismatch");
static_assert(offsetof(pmove_t, oldcmd) == 0x34, "pmove_t::oldcmd offset mismatch");
static_assert(offsetof(pmove_t, tracemask) == 0x64, "pmove_t::tracemask offset mismatch");
static_assert(offsetof(pmove_t, numtouch) == 0x84, "pmove_t::numtouch offset mismatch");
static_assert(offsetof(pmove_t, touchents) == 0x88, "pmove_t::touchents offset mismatch");
static_assert(offsetof(pmove_t, mins) == 0x110, "pmove_t::mins offset mismatch");
static_assert(offsetof(pmove_t, xyspeed) == 0x134, "pmove_t::xyspeed offset mismatch");
static_assert(offsetof(pmove_t, trace) == 0x140, "pmove_t::trace offset mismatch");
static_assert(offsetof(pmove_t, pointcontents) == 0x14C, "pmove_t::pointcontents offset mismatch");
