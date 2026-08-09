// ============================================================================
// sv_stubs.h — minimal cross-object types used by sv.o functions.
// These types belong to OTHER (unported) game objects; only the fields that
// sv.o touches are declared here so the server engine compiles standalone.
// Full definitions arrive when those objects are ported.
// ============================================================================

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "engine/broc_types.h"
#include "game/game_types.h"
#include "core/ae_array.h"

// ============================================================================
// TPakId — pak archive id enum
// ============================================================================
// ============================================================================
// DCGSet â€” collision model (opaque; only fields SV_SetBrushModel touches)
// The release decompile views the entity as a DCGSet*; fields below mirror the
// offsets the disassembly reads (ent[2] = +0x20, ent[5] = +0x50, ent[6] = +0x60).
// Full definition arrives when the collision object is ported.
// ============================================================================
struct DCGSet {
    // +0x00
    uint8_t _pad0[0x08];
    int     objects_m_count;          // +0x08 (objects.m_count high word used)
    uint8_t _pad0C[0x20 - 0x0C];
    // +0x20 (ent[2])
    int     brushes_m_count;          // +0x20
    int     brushes_m_elements;       // +0x24
    int     gjk_brushes_m_count;      // +0x28
    int     gjk_brushes_m_elements;   // +0x2C
    int     brush_sides_m_count;      // +0x30
    int     brush_sides_m_elements;   // +0x34
    int     brush_verts_m_count;      // +0x38
    int     brush_verts_m_elements;   // +0x3C
    float   radius2;                  // +0x40
    uint8_t _pad44[0x50 - 0x44];
    // +0x50 (ent[5])
    math::Position3 max;              // +0x50
    math::Position3 center;           // +0x60
    math::Position3 min;              // +0x70
    int     nboxes;                   // +0x80
};

// ============================================================================
// StubData — per-controller MP save/profile data (1216 bytes) — verified IDA
// ============================================================================
struct StubData {
    char    mProfileName[16];          // +0x000
    int     mSaveGameSlot;             // +0x010
    int     mLevelReached;             // +0x014
    int     mNextLevel;                // +0x018
    int     mLanguage;                 // +0x01C
    int     mDifficulty;               // +0x020
    bool    mGameComplete;             // +0x024
    bool    mShowEnding;               // +0x025
    int     mSec;                      // +0x028
    int     mMin;                      // +0x02C
    int     mHour;                     // +0x030
    int     mDay;                      // +0x034
    bool    mSubtitles;                // +0x038
    bool    mCrosshair;                // +0x039
    bool    mFriendlyTags;             // +0x03A
    int     mTankStyle;                // +0x03C
    bool    mAdsToggle;                // +0x040
    bool    mInvertAim;                // +0x041
    bool    mVibration;                // +0x042
    bool    mStickyAim;                // +0x043
    int     mHorizontalSensitivity;    // +0x044
    int     mVerticalSensitivity;      // +0x048
    int     mControllerButtonConfiguration;  // +0x04C
    int     mControllerStickConfiguration;   // +0x050
    bool    mRatioIs4by3;              // +0x054
    bool    mResolutionIs480p;         // +0x055
    int     mChannels;                 // +0x058
    int     mVolume;                   // +0x05C
    int     mMusicVolume;              // +0x060
    int     mEffectVolume;             // +0x064
    bool    mViewedCredit;             // +0x068
    bool    mGameCompleted;            // +0x069
    int     mViewedSmg;                // +0x06C
    int     mViewedRifle;              // +0x070
    int     mViewedHmg;                // +0x074
    int     mViewedPistol;             // +0x078
    int     mViewedGrenade;            // +0x07C
    int     mViewedAssault;            // +0x080
    int     mViewedSniper;             // +0x084
    int     mViewedCrewServed;         // +0x088
    int     mViewedAntiTank;           // +0x08C
    int     mViewedDemolition;         // +0x090
    int     mViewedLand;               // +0x094
    int     mViewedAir;                // +0x098
    int     mViewedSea;                // +0x09C
    int     mViewedArtillery;          // +0x0A0
    int     mViewedAntiAir;            // +0x0A4
    int     mViewedRocket;             // +0x0A8
    int     mViewedCharacters;         // +0x0AC
    int     mViewedArt[14];            // +0x0B0
    int     mViewedMovies;             // +0x0E8
    int     mMaxPlayerCntPreference;   // +0x0EC
    int     mGameModePreference;       // +0x0F0
    int     mMapPreference;            // +0x0F4
    int     mAutoTeamBalancePreference;// +0x0F8
    int     mTeamDamagePreference;     // +0x0FC
    bool    mSaved;                    // +0x100
    int     mSaveId;                   // +0x104
    uint8_t savedState[516];           // +0x108 (_XONLINE_LOGON_STATE, opaque)
    uint8_t loginMethod[4];            // +0x30C (_UIX_LOGON_TYPE)
    uint8_t liveState[4];              // +0x310 (ELiveState)
    uint8_t savedInvite[156];          // +0x314 (_XONLINE_ACCEPTED_GAMEINVITE, opaque)
    int     mControllerPort;           // +0x3B0
    bool    savedStateIsValid;         // +0x3B4
    int     lastLoginCode;             // +0x3B8 (HRESULT)
    bool    mReturnToMain;             // +0x3BC
    bool    appearOnline;              // +0x3BD
    bool    mbWasInvited;              // +0x3BE
    bool    mDisableSave;              // +0x3BF
    char    mCmdLine[256];             // +0x3C0
};
static_assert(sizeof(StubData) == 0x4C0, "StubData size mismatch");
static_assert(offsetof(StubData, mControllerPort) == 0x3B0, "StubData::mControllerPort offset mismatch");
static_assert(offsetof(StubData, mSec) == 0x28, "StubData::mSec offset mismatch");
static_assert(offsetof(StubData, mInvertAim) == 0x41, "StubData::mInvertAim offset mismatch");

// ============================================================================
// SaveGameData — MP save game bundle (7156 bytes) — verified IDA
// ============================================================================
struct SaveGameData {
    StubData mStubData;         // +0x000
    uint8_t  _rest[7156 - sizeof(StubData)];  // remaining fields opaque
};
static_assert(sizeof(SaveGameData) == 7156, "SaveGameData size mismatch");

// MP player / entity manager minimal views (fields used by SV_PostConnect)
struct MPPlayer;
struct MPPlayerManager;
struct MPPeer;

// ============================================================================
// ServerTime — server clock (20 bytes) — verified IDA
// ============================================================================
class ServerTime {
public:
    unsigned int mNumTicksElapsed;  // +0x00
    int          mTickMSec;         // +0x04
    float        mTickDelta;        // +0x08
    float        mTickDeltaInv;     // +0x0C
    float        mElapsedTime;      // +0x10

    static ServerTime sInst;        // ?sInst@ServerTime@@0V1@A
};
static_assert(sizeof(ServerTime) == 0x14, "ServerTime size mismatch");

// ============================================================================
// FEManager — front-end manager (1012 bytes; opaque, only sv.o fields shown)
// ============================================================================
struct IGOFrontEnd;
struct FEMenuSystem;
struct DialogMenuSystem;
struct InGameMenuSystem;
struct AARMenuSystem;
struct ProfileManager;
struct PanelQuad;
struct nglFont;
struct ControllerDisconnectedMenu;

class FEManager {
public:
    // +0x00 vftable (1 ptr)
    uint8_t _vftable[4];
    nglFont* fonts[4];                  // +0x04
    IGOFrontEnd* IGO;                   // +0x14
    ControllerDisconnectedMenu* ControllerDisconnected;  // +0x18
    FEMenuSystem* fems;                 // +0x1C
    ProfileManager* mProfileManager;    // +0x20
    PanelQuad* default_pq;              // +0x24
    float   pause_menu_timer;           // +0x28
    bool    fontsLoaded[4];             // +0x2C
    bool    start_on;                   // +0x30
    bool    debug_mode;                 // +0x31
    bool    forceMovieExit;             // +0x32
    bool    skipFE;                     // +0x33
    bool    enablePause;                // +0x34
    bool    IGO_active;                 // +0x35
    bool    inGame;                     // +0x36
    bool    menuMovieRunning;           // +0x37
    bool    legalMoviesFinished;        // +0x38
    bool    skipAllLegalMovies;         // +0x39
    bool    skipAllMovies;              // +0x3A
    bool    renderMovieOnly;            // +0x3B
    bool    mDontDrawHud;               // +0x3C
    char    loadLevel[128];             // +0x3D
    float   saveTime;                   // +0xC0
    DialogMenuSystem* mDMS[1];          // +0xC4
    InGameMenuSystem* mIGMS[1];          // +0xC8
    AARMenuSystem* mAARS;               // +0xCC
    uint8_t mPanelArray[800];           // +0xD0 (100 x sPanelPakData, opaque)
    int     mNumPanels;                 // +0x3F0
    // +0x3F4 .. 0x3F4 remaining pad
    void UpdateLoadingMenu(float percentDone);  // ?UpdateLoadingMenu@FEManager@@QAEXM@Z
};
static_assert(sizeof(FEManager) == 0x3F4, "FEManager size mismatch");

// ============================================================================
// Opaque singleton managers (fields used by sv.o only)
// ============================================================================
struct AeThreadManager {
    uint8_t _pad[2148];
    static AeThreadManager sInst;   // ?sInst@AeThreadManager@@0V1@A
    void KillAllThreads();
    void Execute(float deltaT);     // ?Execute@AeThreadManager@@QAEXM@Z
};
static_assert(sizeof(AeThreadManager) == 2148, "AeThreadManager size mismatch");

struct MultiplayerMgr {
    struct MPEntityHandle { int mVal; };  // +0x00 opaque
    MPPeer* mPeer;                  // +0x00
    uint8_t _pad[0x40 - 0x4];
    bool    mLinkCheckEnabled;      // +0x40 (field used by SV_Map_f)
    uint8_t _pad2[0x50 - 0x41];
    static MultiplayerMgr* sInst;   // ?sInst@MultiplayerMgr@@2PAV1@A
    void ExitLevel();
    void StartDevServer();
    void SpotEntity(Entity* ent);               // ?SpotEntity@MultiplayerMgr@@QAEXPAVEntity@@@Z
    void PlayerDamage(Entity* hitEntity, Entity* attacker,
                      const math::Position3& position, const math::Dir3& normal,
                      int weapon, float damage, unsigned char mod, int dflags,
                      EHitLocation hitLocation);
    void VehicleDamage(Entity* hitEntity, Entity* attacker,
                       const math::Position3& position, const math::Dir3& normal,
                       float damage, int weapon, unsigned char mod, int dflags);
    void VehicleDeath(Entity* hitEntity, Entity* killer, int weapon, int mod);
    void ProjectileExplosion(Entity* projectile, int weapon,
                             const math::Position3& position, const math::Dir3& normal,
                             unsigned char surfaceType, Entity* owner);
    void PlayerDead(Entity* player, Entity* inflictor, Entity* attacker,
                    int damage, int mod, int weapon, const float* position,
                    const float* dir, int hitLoc);   // ?PlayerDead@MultiplayerMgr@@QAEXPAVEntity@@00HHHQBM1H@Z
    void AttemptToRevivePlayer(Entity* player, Entity* medic);  // ?AttemptToRevivePlayer@MultiplayerMgr@@QAEXPAVEntity@@0@Z
    void RegisterDroppedItem(int itemType, Entity* item, Entity* owner, int a4);  // ?RegisterDroppedItem@MultiplayerMgr@@QAEXW4EDroppedItemTypes@@PAVEntity@@1H@Z
    MPEntityHandle RegisterDroppedItem(int itemType, Entity* item, Entity* owner);  // ?RegisterDroppedItem@MultiplayerMgr@@QAE?AVMPEntityHandle@@W4EDroppedItemTypes@@PAVEntity@@1@Z
    void FireMissile(int weapon, const math::Position3& position, const math::Dir3& dir,
                     MultiplayerMgr::MPEntityHandle handle);  // ?FireMissile@MultiplayerMgr@@QAEXHABVPosition3@math@@ABVDir3@3@VMPEntityHandle@@@Z
    void Step(int earlyOutInterval, bool fromThread, bool a_bFromGame);  // ?Step@MultiplayerMgr@@QAEXH_N0@Z
    bool IsLocalPlayer(Entity* player);            // ?IsLocalPlayer@MultiplayerMgr@@QAE_NPAVEntity@@@Z
    void DropWeapon(int weapon, int netIndex, const math::Position3* position,
                    const math::Position3* angles, const math::Dir3* velocity,
                    int clipCount, int ammoCount);  // ?DropWeapon@MultiplayerMgr@@QAEXHHABVPosition3@math@@1ABVDir3@2@HH@Z
    void SpreadFire(Entity* player, float gunPitch, float gunYaw,
                    float* weaponPosition, int weapon, float spread,
                    float coneAngleTangent, int seed);  // ?SpreadFire@MultiplayerMgr@@QAEXPAVEntity@@MMQAMHMHH@Z
    void GetNextDroppedItemID(void* result, int itemType, Entity* owner);  // ?GetNextDroppedItemID@MultiplayerMgr@@QAEXAAVMPEntityHandle@@W4EDroppedItemTypes@@PAVEntity@@@Z
    int  GetDroppedItemType(int itemType);   // ?GetDroppedItemType@MultiplayerMgr@@QAE?AW4EDroppedItemTypes@@W4itemType_t@@@Z
    MPEntityHandle FindDroppedItemID(int itemType, Entity* item, Entity* owner);  // ?FindDroppedItemID@MultiplayerMgr@@QAE?AVMPEntityHandle@@W4EDroppedItemTypes@@PAVEntity@@1@Z
    void PickupItem(int netIndex, int itemType, Entity* player, bool scriptFrom);  // ?PickupItem@MultiplayerMgr@@QAEXHHPAVEntity@@_N@Z
    void DropItem(int itemType, const math::Position3* position,
                  const math::Dir3* angles, const math::Dir3* velocity,
                  int netIndex, bool scriptFrom, int typeIndex);  // ?DropItem@MultiplayerMgr@@QAEXW4EDroppedItemTypes@@ABVPosition3@math@@ABVDir3@2@2H_NH@Z
    void ApplyLocalPhysicsToVehicle(Entity* vehicle, math::Position3* position,
                                    math::Position3* angles, float* velocity);  // ?ApplyLocalPhysicsToVehicle@MultiplayerMgr@@QAEXPAVEntity@@AAVPosition3@math@@1QAM@Z
    void AttemptToGetInVehicle(Entity* vehicle, Entity* player, int seatIdx,
                               int entryIdx);  // ?AttemptToGetInVehicle@MultiplayerMgr@@QAEXPAVEntity@@0HH@Z
    void AttemptVehicleSeatChange(Entity* vehicle, Entity* player, int newSeatIdx);  // ?AttemptVehicleSeatChange@MultiplayerMgr@@QAEXPAVEntity@@0H@Z
    void GetOutOfVehicle(Entity* vehicle, int seatIdx);  // ?GetOutOfVehicle@MultiplayerMgr@@QAEXPAVEntity@@H@Z
    void VehicleFireMissile(Entity* vehEnt, int weapon,
                            const math::Position3* position,
                            const math::Dir3* dir);  // ?VehicleFireMissile@MultiplayerMgr@@QAEXPAVEntity@@HABVPosition3@math@@ABVDir3@4@@Z
    void FireArtillery(Entity* attacker, int weapon,
                       const math::Position3* position, int seed, bool fire);  // ?FireArtillery@MultiplayerMgr@@QAEXPAVEntity@@HABVPosition3@math@@H_N@Z
    void VehicleMantled(Entity* vehicle, Entity* killer);  // ?VehicleMantled@MultiplayerMgr@@QAEXPAVEntity@@0@Z
    void AnimEvent(int animEvent);  // ?AnimEvent@MultiplayerMgr@@QAEXH@Z
    bool IsHost();                  // ?IsHost@MultiplayerMgr@@QAE_NXZ
    void SwapWeapon(int weapon, int netIndex, int clipCount, int ammoCount);  // ?SwapWeapon@MultiplayerMgr@@QAEXHHHH@Z
    void SwapKit(int playerClass, int netIndex);  // ?SwapKit@MultiplayerMgr@@QAEXHH@Z
    void BulletHit(const math::Position3& position, const math::Dir3& normal,
                   unsigned char surfaceType, unsigned char weapon,
                   Entity* hitEntity);  // ?BulletHit@MultiplayerMgr@@QAEXABVPosition3@math@@ABVDir3@3@EEPAVEntity@@@Z
    void BulletHitPlayer(Entity* hitEntity, Entity* attackerEntity,
                         const math::Position3& position,
                         const math::Dir3& normal, unsigned char surfaceType,
                         unsigned char weapon, short damage,
                         unsigned char damageFlags, unsigned char mod,
                         int hitLocation);  // ?BulletHitPlayer@MultiplayerMgr@@QAEXPAVEntity@@0ABVPosition3@math@@ABVDir3@4@EEFEEH@Z
    void MeleeHit(Entity* hitEntity, Entity* attackerEntity,
                  const math::Position3& position, const math::Dir3& normal,
                  unsigned char surfaceType, short damage, unsigned char mod,
                  int hitLocation);  // ?MeleeHit@MultiplayerMgr@@QAEXPAVEntity@@0ABVPosition3@math@@ABVDir3@4@EFEH@Z
};
static_assert(sizeof(MultiplayerMgr) == 80, "MultiplayerMgr size mismatch");

struct SmokeGrenadeMgr {
    static void* sInst;  // ?sInst@SmokeGrenadeMgr@@2PAV1@A @ 0xF049B4
};

struct SoundDevice {
    uint8_t _pad[31392];
    static SoundDevice* sInst;      // ?sInst@SoundDevice@@2PAV1@A
    void StopAllSounds();
    void PlaySound(const char* name, DbLinkedHandle<EntityHandleDb, Entity> ent,
                   bool a4, bool a5, const math::Position3& pos,
                   const math::Dir3& dir, float a8, float a9, float a10,
                   float a11);  // ?PlaySound@SoundDevice@@QAE?AV?$DbLinkedHandle@VSoundHandleDb@SoundDevice@@VSound@2@@@PBDV?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@_N2ABVPosition3@math@@ABVDir3@5@MMMM@Z
};
static_assert(sizeof(SoundDevice) == 31392, "SoundDevice size mismatch");

// ============================================================================
// Additional cross-object singleton managers (fields used by sv.o only)
// ============================================================================
struct CheckpointMgr {
    bool         mUsingCheckpoints;       // +0x00 (bool)
    bool         mCheckpointSaveExists;   // +0x01 (bool)
    Broc::string mCurrentMapName;         // +0x04 (Broc::string, 4 bytes)
    static CheckpointMgr* sInst;          // ?sInst@CheckpointMgr@@2PAV1@A
    void ClearSavedCheckpointData();
    void SaveCheckpoint(const char* checkpointName, bool calledFromScript);  // ?SaveCheckpoint@CheckpointMgr@@QAEXPBD_N@Z
};
static_assert(sizeof(CheckpointMgr) == 8, "CheckpointMgr size mismatch (fields used)");

struct PakManager {
    uint8_t _pad[0x28];
    unsigned int mEnabled;             // +0x28
    static PakManager* sInst;            // ?sInst@PakManager@@2PAV1@A
    void FillBanks();                    // ?FillBanks@PakManager@@QAEXXZ
    void UnloadAll();                    // ?UnloadAll@PakManager@@QAEXXZ
    bool IsUnloading(TPakId id) const;   // ?IsUnloading@PakManager@@QBE_NW4TPakId@@@Z
    void MemFree(TPakId id, void* ptr, bool bUseActorHeap);  // ?MemFree@PakManager@@QAEXW4TPakId@@PAX_N@Z
};
static_assert(sizeof(PakManager) == 0x2C, "PakManager size mismatch (opaque)");

// ============================================================================
// InGameMenuSystem — in-game menu system (56 bytes; opaque, only is_active)
// ============================================================================
struct InGameMenuSystem {
    uint8_t _pad[0x34];
    bool    is_active;                   // +0x34 (FEMenuSystem field, opaque)
    uint8_t _pad2[3];                    // +0x35
    void SetActiveMenu(int a2);          // ?SetActiveMenu@InGameMenuSystem@@QAEXH@Z
};
static_assert(sizeof(InGameMenuSystem) == 56, "InGameMenuSystem size mismatch (fields used)");



// ============================================================================
// EntityHandleDb — entity handle database (opaque; only element lookup used)
// HandleDb<Entity,1344,SizedHandle<12,20>>::DbElement = { int mKey; Entity* mObject; }
// ============================================================================
struct EntityHandleDbDbElement {
    Entity*        mObject; // +0x00
    int            mKey;    // +0x04
};
// ae_sized_array<Entity*, 4096> — fixed-capacity array (16388 bytes)
struct AeSizedEntityArray {
    Entity*     m_elements[4096];   // +0x00
    int         m_size;             // +0x4000
};
static_assert(sizeof(AeSizedEntityArray) == 16388, "AeSizedEntityArray size mismatch");

class EntityHandleDb {
public:
    uint8_t  _pad[0xA8];                 // HandleDb BitSet<1344> (168 bytes)
    EntityHandleDbDbElement mElements[0x540];  // +0xA8 (1344 * 8 = 10752)
    void     (*mDebugCallback)(int, Entity*);  // +0x2AA8
    AeSizedEntityArray mActiveList;      // +0x2AAC (16388 bytes)
    static EntityHandleDb sInst;         // ?sInst@EntityHandleDb@@0V1@A
    void Init();                         // ?Init@EntityHandleDb@@QAEXXZ
    void AssignHandle(Entity& e);        // ?AssignHandle@EntityHandleDb@@QAEXAAVEntity@@@Z
    Entity* Find(int fieldofs, HashString match);  // ?Find@EntityHandleDb@@QBEPAVEntity@@HVHashString@@@Z
    void Find(int fieldOfs, unsigned short match, ae_sized_array<Entity*, 4096>* results);  // ?Find@EntityHandleDb@@QBEXGAAV?$ae_sized_array@PAVEntity@@$0BAAA@@@@Z
    void Find(int fieldOfs, HashString match, ae_sized_array<Entity*, 4096>* results);      // ?Find@EntityHandleDb@@QBEXVHashString@@AAV?$ae_sized_array@PAVEntity@@$0BAAA@@@@Z
    void Find(int fieldOfs, const Broc::string* match, ae_sized_array<Entity*, 4096>* results);  // ?Find@EntityHandleDb@@QBEXHABVstring@Broc@@AAV?$ae_sized_array@PAVEntity@@$0BAAA@@@@Z
    Entity** Find(int fieldofs, unsigned short match, Entity** begin, Entity** end);  // ?Find@EntityHandleDb@@QBE?AVconst_iterator@?$ae_sized_array@PAVEntity@@$0BAAA@@@HGV23@0@Z
    Entity** Find(int fieldofs, HashString match, Entity** begin, Entity** end);      // ?Find@EntityHandleDb@@QBE?AVconst_iterator@?$ae_sized_array@PAVEntity@@$0BAAA@@@HVHashString@@V23@1@Z
    void Release(Entity* e);                    // ?Release@EntityHandleDb@@QAEXAAVEntity@@@Z
    Entity* Find(int fieldofs, unsigned short match);  // ?Find@EntityHandleDb@@QBEPAVEntity@@HG@Z
    Entity* Find(int fieldofs, const Broc::string& match);  // ?Find@EntityHandleDb@@QBEPAVEntity@@HABVstring@Broc@@@Z
    Entity** Find(int fieldofs, const Broc::string& match, Entity** begin, Entity** end);  // ?Find@EntityHandleDb@@QBE?AVconst_iterator@?$ae_sized_array@PAVEntity@@$0BAAA@@@HVstring@Broc@@V23@1@Z
    void Compact();                         // ?Compact@EntityHandleDb@@QAEXXZ
    void Validate();                        // ?Validate@EntityHandleDb@@AAEXXZ
};
static_assert(offsetof(EntityHandleDb, mElements) == 0xA8, "EntityHandleDb::mElements offset mismatch");
static_assert(offsetof(EntityHandleDb, mActiveList) == 0x2AAC, "EntityHandleDb::mActiveList offset mismatch");
static_assert(sizeof(EntityHandleDb) == 27312, "EntityHandleDb size mismatch");

// ============================================================================
// XModelManager — model manager (opaque)
// ============================================================================
struct XModel;
class XModelManager {
public:
    uint8_t _pad[4];
    static XModelManager* sInst;           // ?sInst@XModelManager@@2PAV1@A
    IVPointer<XModel> GetXModel(TPakId pak_id, const char* name);
};
static_assert(sizeof(XModelManager) == 4, "XModelManager size mismatch (opaque)");

// ============================================================================
// EntityManager — entity factory (opaque; only sv.o fields used)
// ============================================================================
struct EntityManager {
    uint8_t _pad[4];
    static EntityManager* sInst;            // ?sInst@EntityManager@@2PAV1@A
    Entity* GetPlayer(int idx);             // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z
    bool IsLocalPlayer(Entity* entity);     // ?IsLocalPlayer@EntityManager@@QAE_NPAVEntity@@@Z
    Entity* mPlayers[16];                   // +0x04 (player entity handles)
    Entity* mWorld;                         // +0x44
    void SwapPlayers(int eA, int eB);       // ?SwapPlayers@EntityManager@@QAEXHH@Z
    void CreatePlayers();                   // ?CreatePlayers@EntityManager@@QAEXXZ
    void CreateWorld();                     // ?CreateWorld@EntityManager@@QAEXXZ
    void DeleteAllEntities();               // ?DeleteAllEntities@EntityManager@@QAEXXZ
    int  GetPlayerIndex(Entity* entity);    // ?GetPlayerIndex@EntityManager@@QAEHPAVEntity@@@Z
};
static_assert(offsetof(EntityManager, mPlayers) == 0x04, "EntityManager::mPlayers offset mismatch");
static_assert(offsetof(EntityManager, mWorld) == 0x44, "EntityManager::mWorld offset mismatch");

// ============================================================================
// AeAssert — assertion system (namespace-style free functions + globals)
// ============================================================================
namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3 };
    extern ECoderId gCurrentAuthor;  // ?gCurrentAuthor@AeAssert@@3W4ECoderId@1@A
    extern const char* gCurrentFile;  // ?gCurrentFile@AeAssert@@3PBDB
    extern int  gCurrentLine;         // ?gCurrentLine@AeAssert@@3HA
    extern const char* gCurrentExpr;  // ?gCurrentExpr@AeAssert@@3PBDB
    bool IsIgnored(void);
    bool Assert(const char* fmtstring, ...);
    bool Warning(const char* fmtstring, ...);
}

// ============================================================================
// movie_manager — static movie helpers
// ============================================================================
struct movie_manager {
    static void load_and_play_movie(const char* movie_name, const char* sound_name);
};

// ============================================================================
// Misc enums / constants
// ============================================================================
enum EThreadOwner {
    kMainThread = 0,
};

enum EGamePhase {
    GAME_PHASE_LOADING = 0,
    GAME_PHASE_INGAME = 1,
    GAME_PHASE_FRONTEND = 2,
};

struct PathNodeMgr {
    uint8_t _pad[4];
    static PathNodeMgr* sInst;           // ?sInst@PathNodeMgr@@2PAV1@A
    void InitPaths();                    // ?InitPaths@PathNodeMgr@@QAEXXZ
    void ValidateAllNodes();             // ?ValidateAllNodes@PathNodeMgr@@QAEXXZ
    void SetCoverNodeStatus(Broc::string* name, int inValid);  // ?SetCoverNodeStatus@PathNodeMgr@@QAEXPBVstring@Broc@@H@Z
    void AttachSentientToChainNode(sentient_s* pSentient, Broc::string* targetname);
    void ConnectPathsForEntity(Entity* ent);     // ?ConnectPathsForEntity@PathNodeMgr@@QAEXPAVEntity@@@Z
    void DisconnectPathsForEntity(Entity* ent);  // ?DisconnectPathsForEntity@PathNodeMgr@@QAEXPAVEntity@@@Z
    void NodeList();                     // ?NodeList@PathNodeMgr@@QAEXXZ
};
static_assert(sizeof(PathNodeMgr) == 4, "PathNodeMgr size mismatch (opaque)");

// ============================================================================
// SceneManager â€” scene/static-model manager (opaque)
// ============================================================================
struct SceneManager {
    uint8_t _pad[4];
    static SceneManager* sInst;          // ?sInst@SceneManager@@2PAV1@A
    InplaceVector<unsigned char>* mPersistantStorage;
    void ResetAllStaticModels();         // ?ResetAllStaticModels@SceneManager@@QAEXXZ
};

struct FEMenuSystem {
    virtual void SetActiveMenu(int a2);  // ?SetActiveMenu@FEMenuSystem@@UAEXH@Z
};
static_assert(sizeof(FEMenuSystem) == 4, "FEMenuSystem size mismatch (opaque)");

// ============================================================================
// GamePause — static pause helpers
// ============================================================================
struct GamePause {
    static void SetAllPaused(bool paused);      // ?SetAllPaused@GamePause@@SAX_N@Z
    static void SetGamePaused(int client, bool paused);  // ?SetGamePaused@GamePause@@SAXH_N@Z
    static bool IsGamePaused(int client);       // ?IsGamePaused@GamePause@@SA_NH@Z
};

// ============================================================================
// VehicleNodeAllocator — vehicle node manager
// ============================================================================
struct vehicle_node_t;
struct VehicleNodeAllocator {
    uint16_t m_numNodes;        // +0x00
    uint16_t m_numBlocks;       // +0x02
    uint16_t m_currentBlockIndex;  // +0x04
    uint16_t spad;              // +0x06
    void*    m_pNodeBlocks[16]; // +0x08 (64 bytes)
    void FreeAll();   // ?FreeAll@VehicleNodeAllocator@@QAEXXZ
    void Initialize();  // ?Initialize@VehicleNodeAllocator@@QAEXXZ
    vehicle_node_t* AllocNode();  // ?AllocNode@VehicleNodeAllocator@@QAEPAUvehicle_node_t@@XZ
};

// ============================================================================
// IGOFrontEnd — in-game overlay front end
// ============================================================================
struct IGOFrontEnd {
    uint8_t _pad[168];
    void SetTutorialText(int ref, int viewport);  // ?SetTutorialText@IGOFrontEnd@@QAEXHH@Z
    void SetFuse(float total, float remain, int client);  // ?SetFuse@IGOFrontEnd@@QAEXMMH@Z
    void AddActiveGrenade(const Entity* grenade);  // ?AddActiveGrenade@IGOFrontEnd@@QAEXPBVEntity@@@Z
};
static_assert(sizeof(IGOFrontEnd) == 168, "IGOFrontEnd size mismatch");

// ============================================================================
// Memory helpers (Z_MallocInternal / Z_FreeInternal / heap)
// ============================================================================
extern void* _Z_MallocInternal(unsigned int size);
extern void  _Z_FreeInternal(void* ptr);
extern void* mem_heap_malloc(int alignment, unsigned int size);
extern void  mem_heap_free(void* ptr);

// ============================================================================
// Cross-object globals used by sv.o
// ============================================================================
extern SaveGameData* gSaveGameData;   // ?gSaveGameData@@3PAUSaveGameData@@A
extern FEManager     g_femanager;     // ?g_femanager@@3VFEManager@@A
extern bool          gQuickStart;     // ?gQuickStart@@3_NA
extern bool          gReturnToMenu;   // ?gReturnToMenu@@3_NA
extern cvar_t*       com_sv_running;  // ?com_sv_running@@3PAUcvar_t@@A
extern bool          gGodModeEnabled; // ?gGodModeEnabled@@3_NA
extern bool          gNoClipEnabled;  // ?gNoClipEnabled@@3_NA
extern bool          gIsWorkspaceMap; // ?gIsWorkspaceMap@@3_NA
extern bool          gDoNotPlayCampaignMovies; // ?gDoNotPlayCampaignMovies@@3_NA
extern int           g_networkOwner;  // ?g_networkOwner@@3W4EThreadOwner@@A
extern const char    defaultFileName[];  // ?defaultFileName
extern void          mem_heap_free(void* ptr);
extern int           unk_F6A290;      // Xbox dev/retail flag

// ============================================================================
// Cross-object functions used by sv.o
// ============================================================================
extern void   Cmd_AddServerCommand(const char* cmd_name, void (*function)());
extern void   SV_DirectConnect(netadr_t from);
extern void   SV_ClientEnterWorld(client_s* client, int restart, int savegame);
extern void   SV_CheckLoadLevel(int savegame);
extern void   SV_SendClientMessages(void);
extern void   CL_ParseGamestate(Broc::string* configstrings);
extern void   CL_ConnectResponse(netadr_t from);
extern void   CL_FirstSnapshot(void);
extern void   PathNodeMgr_InitPaths(void);
extern int    BG_GetNumWeapons(void);
extern void   CG_RegisterWeapon(int weaponNum);
extern void   j_nullsub_86(int phase);
extern void   movie_manager_load_and_play_movie(const char* movie_name, const char* sound_name);
extern   void   FEManager_PlayFadeInOranScreen(void);

// ============================================================================
// MP player / entity manager minimal views (fields used by SV_PostConnect)
// ============================================================================
struct MPPlayer {
    uint8_t _pad[4];
    int     mClientIndex;   // +0x04
    Entity* GetEntity();    // ?GetEntity@MPPlayer@@QAEPAVEntity@@XZ
};

struct MPPlayerManager {
    MPPlayer* GetPlayer(int id);
};

struct MPPeer {
    MPPlayerManager* GetPlayerManager();
};

struct MultiplayerMgr2 {
    MPPeer* mPeer;
};

extern MultiplayerMgr2* MultiplayerMgr2_sInst(void);
extern EntityManager*   EntityManager_sInst(void);
extern int              currCl;      // ?currCl@@3HA
extern bool             gExitGame;   // ?gExitGame@@3_NA
extern int              unk_F6A290;  // Xbox dev/retail flag
extern int              dword_F641E0[];

// ============================================================================
// XModel â€” minimal view for SV_PointTraceToEntity model scan
// ============================================================================
struct XModelLod;
struct nglMesh;
struct XModelParts;

// XModelLod - model LOD entry (12 bytes) - verified against IDA
struct XModelLod {
    float        dist;         // +0x00
    InplaceString filename;    // +0x04
    XModelParts* xmodelParts;  // +0x08
};
static_assert(sizeof(XModelLod) == 0x0C, "XModelLod size mismatch");

// XBoneHierarchy - bone hierarchy entry (12 bytes) - verified against IDA
struct XBoneHierarchy {
    InplaceString mName;        // +0x00
    unsigned int  mNameHash;    // +0x04
    int           mParentIndex; // +0x08
};
static_assert(sizeof(XBoneHierarchy) == 0x0C, "XBoneHierarchy size mismatch");

// XModelParts - model geometry/anim data (0x40 bytes) - verified against IDA
struct XModelParts {
    InplaceVector<math::Mat43::Packed> mTransforms;  // +0x00
    void*            mBoneInfos;                     // +0x08 InplaceVector<XBoneInfo>
    InplaceVector<XBoneHierarchy> mHierarchy;        // +0x10 InplaceVector<XBoneHierarchy>
    void*            mPartClassifications;           // +0x18
    void*            mMeshNames;                     // +0x20
    InplaceVector<nglMesh*> mMeshPtrs;               // +0x28
    int              mNumRootBones;                  // +0x30
    InplaceString    mAnimDefName;                   // +0x34
    void*            mAnimDef;                       // +0x38 nalBaseSkeleton*
    InplaceString    mName;                          // +0x3C
};

struct XModel {
    uint8_t      _pad0[0x20];  // +0x00
    XModelParts* parts;        // +0x20
    XModelLod**  lod;          // +0x24
    uint8_t      _pad28[0x38 - 0x28];
    void*        collSurfs;    // +0x38 InplaceVector<XModelCollSurf const *>
    uint8_t      _pad3C[0x40 - 0x3C];
    int          contents;     // +0x40
    uint8_t      _pad41[0x44 - 0x41];
    uint16_t     numLods;      // +0x44
    uint16_t     collLod;      // +0x46
    InplaceString name;        // +0x48
    unsigned int iflFrames;    // +0x4C (Bitmask<unsigned int>)
    static int GetNumBones(XModel* model, int lodIndex);
};

// IVPointer operator bool / operator-> (reconstructed from IDA)
template <typename T>
inline bool IVPointer_IsValid(const IVPointer<T>& p) { return p.mValue != NULL; }
template <typename T>
inline T* IVPointer_Deref(const IVPointer<T>& p) { return p.mValue; }

// ============================================================================
// Client static state (cls) â€” opaque
// ============================================================================
enum clientStateCA_t {
    CA_DISCONNECTED = 0,
    CA_ACTIVE = 1,
};

struct clientStatic_t {
    int state;  // +0x00 (clientStateCA_t)
};

extern clientStatic_t cls;  // ?cls@@3UclientStatic_t@@A
extern int   EntityManager_GetNumPlayers(void);
extern Entity* EntityManager_GetPlayerEntity(int idx);
