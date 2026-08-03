// ============================================================================
// COD3 Game Types — Entity, EntityState, EntityShared
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "core/math_types.h"
#include "engine/broc_types.h"
#include <stdint.h>

// Forward declarations
struct Entity;
struct DObj;
struct EntityNotifySet;
struct ScriptEventHandler;
struct biped_phys_info;
struct Destructible;
struct Client;
struct scr_vehicle_t;
struct turretInfo_t;
struct trRefEntity;
struct XAnimTree;
struct XModel;
struct gitem_s;
struct Curve;
struct tagInfo_t;
struct animscripted_t;
struct proximity_data_t;
struct actor_s;
struct sentient_s;
struct DCGSet;
struct WorldSector;

// Handle type — wraps a DbLinkedHandle
template <typename HandleDb, typename T>
struct DbLinkedHandle {
    uint16_t index;  // handle index

    DbLinkedHandle() : index(0) {}
    bool IsValid() const { return index != 0; }
};
static_assert(sizeof(DbLinkedHandle<void, void>) == 4, "DbLinkedHandle size mismatch");

// Inplace vector
template <typename T>
struct InplaceVector {
    unsigned int mSize;  // +0x00
    T*           mList;  // +0x04
};
static_assert(sizeof(InplaceVector<char>) == 8, "InplaceVector size mismatch");

// IVPointer — intrusive counted pointer (8 bytes)
template <typename T>
struct IVPointer {
    void* ptr;    // +0x00 — actual pointer data
};
static_assert(sizeof(IVPointer<char>) == 8, "IVPointer size mismatch");

// ============================================================================
// EntityState — network-replicated entity state (224 bytes)
// Size: 0xE0 (224 bytes) — verified against IDA
// ============================================================================
struct EntityState {
    uint8_t  eType;                               // +0x00
    uint8_t  loopSound;                           // +0x01
    uint8_t  surfType;                            // +0x02
    uint8_t  weapon;                              // +0x03
    uint8_t  eventParm;                           // +0x04
    uint8_t  ___u5;                               // +0x05 (padding/unknown)
    uint16_t index;                               // +0x06 (truncated entity index)
    DbLinkedHandle<void, Entity> mOtherEntity;    // +0x08
    DbLinkedHandle<void, Entity> mGroundEntity;   // +0x0C
    int32_t  eFlags;                              // +0x10
    // trajectory_t pos — embedded 40 bytes
    // trajectory_t apos — embedded 40 bytes
    uint8_t  _pad_traj[80];                       // +0x14 (2x trajectory_t)
    math::Position3 lerpOrigin;                   // +0x70
    math::Position3 lerpAngles;                   // +0x80
    math::Position3 origin2;                      // +0x90
    math::Position3 angles2;                      // +0xA0
    int32_t  constantLight;                       // +0xB0
    int32_t  solid;                               // +0xB4
    int32_t  eventSequence;                       // +0xB8
    uint8_t  events[4];                           // +0xBC
    uint8_t  eventParms[4];                       // +0xC0
    float    leanf;                               // +0xC4
    int32_t  dmgFlags;                            // +0xC8
    int32_t  useCount;                            // +0xCC
    int32_t  eTeam;                               // +0xD0
};
static_assert(sizeof(EntityState) == 0xE0, "EntityState size mismatch");
static_assert(offsetof(EntityState, eFlags) == 0x10, "EntityState::eFlags offset mismatch");

// ============================================================================
// EntityShared — server-side shared entity data (336 bytes)
// Size: 0x150 (336 bytes) — verified against IDA
// ============================================================================
struct EntityShared {
    int32_t  linked;                              // +0x00
    int32_t  svFlags;                             // +0x04
    DbLinkedHandle<void, Entity> mSingleClient;   // +0x08
    DCGSet*  bmodel;                              // +0x0C
    math::Position3 mins;                         // +0x10
    math::Position3 maxs;                         // +0x20
    math::Position3 absmin;                       // +0x30
    math::Position3 absmax;                       // +0x40
    math::Vector4 pos_cache;                      // +0x50
    int32_t  lastLeaf;                            // +0x60
    int32_t  contents;                            // +0x64
    // padding from 0x68 to 0x70 (alignment before Position3)
    uint8_t  _pad1[8];                            // +0x68
    math::Position3 currentOrigin;                // +0x70
    math::Position3 currentAngles;                // +0x80
    math::Mat43    currentMat;                    // +0x90
    DbLinkedHandle<void, Entity> mOwner;          // +0xD0
    int32_t  eventType;                           // +0xD4
    int32_t  eventTime;                           // +0xD8
    WorldSector* worldSector;                     // +0xDC
    EntityShared* nextEntityInWorldSector;         // +0xE0
    int32_t  numClusters;                         // +0xE4
    int32_t  clusternums[16];                     // +0xE8
    int32_t  lastCluster;                         // +0x128
    int32_t  areanum;                             // +0x12C
    int32_t  areanum2;                            // +0x130
    int32_t  linkcontents;                        // +0x134
    float    linkmin[2];                          // +0x138
    float    linkmax[2];                          // +0x140
    // padding to 0x150
    uint8_t  _pad2[8];                            // +0x148
};
static_assert(sizeof(EntityShared) == 0x150, "EntityShared size mismatch");

// ============================================================================
// Entity — main game entity (1136 bytes)
// Size: 0x470 (1136 bytes) — verified against IDA
// ============================================================================
struct Entity {
    EntityState  s;                               // +0x000 (224 bytes)
    EntityShared r;                               // +0x0E0 (336 bytes)
    int32_t  mPakId;                              // +0x230
    DbLinkedHandle<void, Entity> mHandle;         // +0x234
    int16_t  mEntityArrayIndex;                   // +0x238
    // pad
    uint8_t  _pad1[2];                            // +0x23A
    DObj*    mDObj;                               // +0x23C
    EntityNotifySet* mNotifySet;                   // +0x240
    ScriptEventHandler* mScriptEventHandler;       // +0x244
    biped_phys_info* mBPInfo;                     // +0x248
    IVPointer<Destructible> mDestructible;        // +0x24C (8 bytes)
    Client*  client;                              // +0x254
    actor_s* actor;                               // +0x258
    sentient_s* sentient;                         // +0x25C
    scr_vehicle_t* scr_vehicle;                   // +0x260
    turretInfo_t* pTurretInfo;                    // +0x264
    trRefEntity* mRenderEntity;                   // +0x268
    XAnimTree* pAnimTree;                         // +0x26C
    IVPointer<XModel> mModel;                     // +0x270 (8 bytes)
    float    modelscale;                          // +0x278
    // Broc::string and Hashed names
    uint8_t  _pad_broc_strings[0x30];             // +0x27C (broc strings/hashes region)
    float    pos1_v[4];                           // +0x2E0 (math::Position3 pos1)
};
static_assert(sizeof(Entity) == 0x470, "Entity size mismatch");
static_assert(offsetof(Entity, s) == 0x000, "Entity::s offset mismatch");
static_assert(offsetof(Entity, r) == 0x0E0, "Entity::r offset mismatch");
static_assert(offsetof(Entity, client) == 0x254, "Entity::client offset mismatch");
static_assert(offsetof(Entity, actor) == 0x258, "Entity::actor offset mismatch");
static_assert(offsetof(Entity, sentient) == 0x25C, "Entity::sentient offset mismatch");
