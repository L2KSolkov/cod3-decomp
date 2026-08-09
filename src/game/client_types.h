// ============================================================================
// COD3 Client Types — Client (gclient_s equivalent), clientPersistent_t
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "game/game_types.h"
#include "game/player_types.h"
#include "engine/broc_types.h"
#include "core/math_types.h"
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// clientConnected_t — connection state enum
// ============================================================================
typedef int32_t clientConnected_t;

// ============================================================================
// AnimIKFireEvent — IK fire event (8 bytes)
// Size: 0x08 (8 bytes) — verified against IDA
// ============================================================================
struct AnimIKFireEvent {
    int fireTime;    // +0x00
    int fireWeapon;  // +0x04
};
static_assert(sizeof(AnimIKFireEvent) == 8, "AnimIKFireEvent size mismatch");

// ============================================================================
// AnimIKPainEvent — IK pain event (28 bytes)
// Size: 0x1C (28 bytes) — verified against IDA
// ============================================================================
struct AnimIKPainEvent {
    int    time;        // +0x00
    int    duration;    // +0x04
    float  amplitude;   // +0x08
    float  dir[3];      // +0x0C
    bool   bIsLowBlow;  // +0x18
    uint8_t _pad19[3];  // +0x19
};
static_assert(sizeof(AnimIKPainEvent) == 0x1C, "AnimIKPainEvent size mismatch");
static_assert(offsetof(AnimIKPainEvent, dir) == 0x0C, "AnimIKPainEvent::dir offset mismatch");

// ============================================================================
// AnimLadderData_t — ladder animation state (12 bytes)
// Size: 0x0C (12 bytes) — verified against IDA
// ============================================================================
struct AnimLadderData_t {
    int    lastLadderTime;  // +0x00
    float  groundZ;         // +0x04
    float  yaw;             // +0x08
};
static_assert(sizeof(AnimLadderData_t) == 0x0C, "AnimLadderData_t size mismatch");

// ============================================================================
// PathNodes::NodeHandle — path node handle (2 bytes)
// (namespace declared in engine/broc_types.h)
// ============================================================================

// ============================================================================
// clientPersistent_t — persistent client data (536 bytes)
// Size: 0x218 (536 bytes) — verified against IDA
// ============================================================================
struct clientPersistent_t {
    int16_t mStats[7][29];                          // +0x000 (406 bytes)
    int     mBaseScore;                             // +0x198
    DbLinkedHandle<void, void> healthTaskHandle;    // +0x19C
    int16_t playerClass;                            // +0x1A0
    int16_t nextPlayerClass;                        // +0x1A2
    int     playerState;                            // +0x1A4
    int16_t rank;                                   // +0x1A8
    // pad 2
    uint8_t _pad1AA[2];                             // +0x1AA
    int     connected;                              // +0x1AC
    usercmd_s cmd;                                  // +0x1B0 (48 bytes)
    usercmd_s oldcmd;                               // +0x1E0 (48 bytes)
    int     pmoveFixed;                             // +0x210
    int     maxHealth;                              // +0x214
};
static_assert(sizeof(clientPersistent_t) == 0x218, "clientPersistent_t size mismatch");
static_assert(offsetof(clientPersistent_t, mStats) == 0x000, "clientPersistent_t::mStats offset mismatch");
static_assert(offsetof(clientPersistent_t, mBaseScore) == 0x198, "clientPersistent_t::mBaseScore offset mismatch");
static_assert(offsetof(clientPersistent_t, cmd) == 0x1B0, "clientPersistent_t::cmd offset mismatch");
static_assert(offsetof(clientPersistent_t, maxHealth) == 0x214, "clientPersistent_t::maxHealth offset mismatch");

// ============================================================================
// Client — per-player client state (2976 bytes) = COD3's gclient_s
// Size: 0xBA0 (2976 bytes) — verified against IDA (75 members)
// ============================================================================
struct Client {
    PlayerState        ps;                            // +0x000 (1488 bytes)
    clientPersistent_t pers;                          // +0x5D0 (536 bytes)
    void Clear(bool clearPersistentAlso, bool clearWeapons);  // ?Clear@Client@@QAEX_N0@Z
    int                noclip;                        // +0x7E8
    int                ufo;                           // +0x7EC
    int                bFrozen;                       // +0x7F0
    int                lastCmdTime;                   // +0x7F4
    int                buttons;                       // +0x7F8
    int                oldbuttons;                    // +0x7FC
    int                latched_buttons;               // +0x800
    // pad 4 (align to 0x810)
    uint8_t            _pad804[4];                    // +0x804
    math::Position3    oldOrigin;                     // +0x810
    float              fGunPitch;                     // +0x820
    float              fGunYaw;                       // +0x824
    float              fGunXOfs;                      // +0x828
    float              fGunYOfs;                      // +0x82C
    float              fGunZOfs;                      // +0x830
    int                damage_blood;                  // +0x834
    float              damage_from[3];                // +0x838
    int                damage_fromWorld;              // +0x844
    int                respawnTime;                   // +0x848
    float              currentAimSpreadScale;         // +0x84C
    Entity*            pHitHitEnt;                    // +0x850
    Entity*            pLookatEnt;                    // +0x854
    DbLinkedHandle<EntityHandleDb, Entity> pLookatEntLast;      // +0x858
    int                iLookatEntLastTime;            // +0x85C
    int                iLastFriendlyUseTime;          // +0x860
    float              fLastTraceDist;                // +0x864
    DbLinkedHandle<EntityHandleDb, Entity> hLastCompassFriendlyInfoEnt;  // +0x868
    DbLinkedHandle<EntityHandleDb, Entity> hLastCompassTankInfoEnt;      // +0x86C
    float              prevLinkAngles[3];             // +0x870
    float              linkAnglesFrac[3];             // +0x87C
    int                inControlTime;                 // +0x888
    int                lastTouchTime;                 // +0x88C
    DbLinkedHandle<EntityHandleDb, Entity> mUseHoldEntity;      // +0x890
    int                mUseHoldTime;                  // +0x894
    int                bDisableAutoPickup;            // +0x898
    int                pain_debounce_time;            // +0x89C
    int                invulnerableExpireTime;        // +0x8A0
    bool               invulnerableActivated;         // +0x8A4
    bool               invulnerableEnabled;           // +0x8A5
    PathNodes::NodeHandle mInvalidatedNode[5];        // +0x8A6 (10 bytes)
    int                mInvalidatedNodeNum;           // +0x8B0
    AnimIKFireEvent    AnimIKFireEvents[15];          // +0x8B4 (120 bytes)
    AnimIKPainEvent    AnimIKPainEvents[15];          // +0x92C (420 bytes)
    bool               mVehicleAnimMoving;            // +0xAD0
    bool               mVehicleAnimPauseRemoteAngles; // +0xAD1
    uint8_t            _padAD2[2];                    // +0xAD2
    int                mVehicleEntryPoint;            // +0xAD4
    bool               mVehicleAnimDisableCamera;     // +0xAD8
    uint8_t            _padAD9[3];                    // +0xAD9
    int                mVehicleNoWeaponTime;          // +0xADC
    bool               mVehicleAnimFirstPersonCam;    // +0xAE0
    uint8_t            _padAE1[3];                    // +0xAE1
    int                mServerClientIndex;            // +0xAE4
    int                mVehicleAnimRoute;             // +0xAE8
    int                mVehicleAnimStage;             // +0xAEC
    int                mVehicleAnimStageAnim;         // +0xAF0
    int                mVehicleAnimStageAnimPlayed;   // +0xAF4
    bool               mVehicleAnimGetOut;            // +0xAF8
    uint8_t            _padAF9[3];                    // +0xAF9
    float              mVehicleAnimAngleOffset[3];    // +0xAFC
    float              mStepViewYaw;                  // +0xB08
    float              mLastTorsoIKLegsYaw;           // +0xB0C
    float              mLeftFootYaw;                  // +0xB10
    float              mLeftFootLift;                 // +0xB14
    float              mRightFootYaw;                 // +0xB18
    float              mRightFootLift;                // +0xB1C
    uint8_t            mFakerootOriginMatrix[64];     // +0xB20 (nalMatrix4x4 = math::Mat44)
    bool               mFakerootOriginMatrixValid;    // +0xB60
    bool               mIKWalkingAdjustment;          // +0xB61
    uint8_t            _padB62[2];                    // +0xB62
    AnimLadderData_t   mLadderData;                   // +0xB64
    int                mProneBlockedTime;             // +0xB70
    int                mMedicNobodyToReviveTime;      // +0xB74
    int                mTankExitBlockedByMantleTime;  // +0xB78
    int                mVehicleAnimStageChangeTime;   // +0xB7C
    int                mNoDrawTime;                   // +0xB80
    float              mFootStepsThisZ[2];            // +0xB84
    float              mFootStepsLastZ[2];            // +0xB8C
    bool               mFootStepsFootGrounded[2];     // +0xB94
    uint8_t            _padB96[2];                    // +0xB96
    int                mFootStepsSurface[2];          // +0xB98
};
static_assert(sizeof(Client) == 0xBA0, "Client size mismatch");
static_assert(offsetof(Client, ps) == 0x000, "Client::ps offset mismatch");
static_assert(offsetof(Client, pers) == 0x5D0, "Client::pers offset mismatch");
static_assert(offsetof(Client, noclip) == 0x7E8, "Client::noclip offset mismatch");
static_assert(offsetof(Client, oldOrigin) == 0x810, "Client::oldOrigin offset mismatch");
static_assert(offsetof(Client, fGunPitch) == 0x820, "Client::fGunPitch offset mismatch");
static_assert(offsetof(Client, pHitHitEnt) == 0x850, "Client::pHitHitEnt offset mismatch");
static_assert(offsetof(Client, AnimIKFireEvents) == 0x8B4, "Client::AnimIKFireEvents offset mismatch");
static_assert(offsetof(Client, AnimIKPainEvents) == 0x92C, "Client::AnimIKPainEvents offset mismatch");
static_assert(offsetof(Client, mFakerootOriginMatrix) == 0xB20, "Client::mFakerootOriginMatrix offset mismatch");
static_assert(offsetof(Client, mLadderData) == 0xB64, "Client::mLadderData offset mismatch");
static_assert(offsetof(Client, mFootStepsSurface) == 0xB98, "Client::mFootStepsSurface offset mismatch");
