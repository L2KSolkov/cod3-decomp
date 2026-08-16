// ============================================================================
// MPLiveEngine.h - multiplayer Xbox Live engine (game_xbox.o MPLiveEngine.cpp)
// Reconstructed from IDA local types (PDB symbol data). All sizes verified.
// ============================================================================

#pragma once

#include "XboxLive.h"
#include "xlive.h"
#include "bd/bd_types.h"
#include "core/mem_heap.h"

// ============================================================================
// SaveGameData - minimal view (shell.o owns the real symbol; array of 4)
// Only the StubData fields used by game_xbox.o are declared; size 0x1BF4.
// ============================================================================
struct SaveGameData {
    unsigned char _pad0[0x108];
    unsigned char savedState[0x204];  // +0x108 (_XONLINE_LOGON_STATE)
    UIX_LOGON_TYPE loginMethod;       // +0x30C (_UIX_LOGON_TYPE)
    int liveState;                  // +0x310 (StubData.liveState)
    unsigned char savedInvite[0x9C];  // +0x314 (XONLINE_ACCEPTED_GAMEINVITE)
    int mControllerPort;            // +0x3B0 (StubData.mControllerPort)
    bool savedStateIsValid;         // +0x3B4
    int lastLoginCode;              // +0x3B8 (HRESULT)
    bool mReturnToMain;             // +0x3BC
    bool appearOnline;              // +0x3BD
    bool mbWasInvited;              // +0x3BE
    bool mDisableSave;              // +0x3BF
    unsigned char _pad4[0x1BF4 - 0x3C0];
};
static_assert(sizeof(SaveGameData) == 0x1BF4, "SaveGameData size mismatch");
extern SaveGameData gSaveGameData[4];

// ============================================================================
// MPPlayerSet - 16-player bitmask (2 bytes, verified)
// ============================================================================
struct MPPlayerSet {
    unsigned short mBitPlayers;     // +0x00

    MPPlayerSet() : mBitPlayers(0) {}
    MPPlayerSet(unsigned int index) { set(index); }
    MPPlayerSet(const MPPlayerSet& set) : mBitPlayers(set.mBitPlayers) {}

    void set(unsigned int index);               // inline, ea 0x72A480
    bool containsPlayer(unsigned int index) const;  // inline, ea 0x72A580
    unsigned int lowestPlayerIndex() const;     // extern mp.o (ea 0x730350)
    unsigned int highestPlayerIndex() const;    // extern mp.o (ea 0x7302D0)
    unsigned short getBitfield() const { return mBitPlayers; }
    void setBitfield(unsigned short v) { mBitPlayers = v; }
};
static_assert(sizeof(MPPlayerSet) == 0x2, "MPPlayerSet size mismatch");

// ============================================================================
// QueryInterface - base for session queries (4 bytes, vtable only)
// Slots verified: 0=Process, 1=Cancel, 3=Done, 5=IsRunning, 6=IsProbing,
// 7=Probe. CFromIDQuery/CDefaultQuery derive from this (mp.o).
// ============================================================================
struct QueryInterface {
    void** vftable;  // +0x00

    HRESULT Process() { return ((HRESULT(__thiscall*)(QueryInterface*))vftable[0])(this); }
    void Cancel() { ((void(__thiscall*)(QueryInterface*))vftable[1])(this); }
    bool Done() { return ((bool(__thiscall*)(QueryInterface*))vftable[3])(this) != 0; }
    bool Succeeded() { return ((bool(__thiscall*)(QueryInterface*))vftable[4])(this) != 0; }
    bool IsRunning() { return ((bool(__thiscall*)(QueryInterface*))vftable[5])(this) != 0; }
    bool IsProbing() { return ((bool(__thiscall*)(QueryInterface*))vftable[6])(this) != 0; }
    HRESULT Probe() { return ((HRESULT(__thiscall*)(QueryInterface*))vftable[7])(this); }
};

// ============================================================================
// CFromIDQuery - query a session by ID (0xD0 bytes, verified)
// ============================================================================
#pragma pack(push, 1)
struct XOnlineMatchSessionRecord {
    unsigned char reserved[90];    // +0x00 (attribute parse target)
    XNKID SessionID;               // +0x5A (90)
    XNKEY KeyExchangeKey;          // +0x62 (98)
    XNADDR HostAddress;            // +0x72 (114)
    unsigned int dwPublicOpen;     // +0x96 (150)
    unsigned int dwPrivateOpen;    // +0x9A (154)
    unsigned int dwPublicFilled;   // +0x9E (158)
    unsigned int dwPrivateFilled;  // +0xA2 (162)
    unsigned int qosInfo;          // +0xA6 (166)
};
#pragma pack(pop)
static_assert(sizeof(XOnlineMatchSessionRecord) == 170,
              "XOnlineMatchSessionRecord size mismatch");

struct CFromIDQueryResults {
    unsigned char v[170];   // +0x00 (one session record)
    unsigned int  m_dwSize; // +0xAC

    XOnlineMatchSessionRecord& operator[](unsigned int i)
    {
        return *(XOnlineMatchSessionRecord*)&v[170 * i];
    }
    void Remove(unsigned int i);  // inline, ea 0x7299F0
};

class CFromIDQuery : public QueryInterface {
public:
    enum STATE {
        STATE_IDLE = 0,
        STATE_RUNNING = 1,
        STATE_PROBING_CONNECTIVITY = 2,
        STATE_PROBING_BANDWIDTH = 3,
        STATE_DONE = 4,
    };

    CFromIDQueryResults Results;       // +0x04
    const XNADDR* m_rgpXnAddr[1];      // +0xB4
    const XNKID* m_rgpXnKid[1];        // +0xB8
    const XNKEY* m_rgpXnKey[1];        // +0xBC
    XNQOS* m_pXnQos;                   // +0xC0
    STATE m_State;                     // +0xC4
    HRESULT m_hrQuery;                 // +0xC8
    XONLINETASK_HANDLE m_hSearchTask;  // +0xCC

    CFromIDQuery();
    ~CFromIDQuery();
    void Cancel();
    void Clear();
    HRESULT Query(unsigned __int64 SessionID);
    HRESULT Probe();
    HRESULT Process();
};
static_assert(sizeof(CFromIDQuery) == 0xD0, "CFromIDQuery size mismatch");

// ============================================================================
// CDefaultQuery - matchmaking query (0x11E0 bytes, verified)
// ============================================================================
struct CDefaultQueryResults {
    unsigned char v[25 * 170];  // +0x00 (25 session records)
    unsigned int  m_dwSize;     // +0x109C

    XOnlineMatchSessionRecord& operator[](unsigned int i)
    {
        return *(XOnlineMatchSessionRecord*)&v[170 * i];
    }
    void Remove(unsigned int i);  // inline, ea 0x7298C0
};

class CDefaultQuery : public QueryInterface {
public:
    enum STATE {
        STATE_IDLE = 0,
        STATE_RUNNING = 1,
        STATE_PROBING_CONNECTIVITY = 2,
        STATE_PROBING_BANDWIDTH = 3,
        STATE_DONE = 4,
    };

    CDefaultQueryResults Results;       // +0x04
    const XNADDR* m_rgpXnAddr[25];      // +0x10A4
    const XNKID* m_rgpXnKid[25];        // +0x1108
    const XNKEY* m_rgpXnKey[25];        // +0x116C
    XNQOS* m_pXnQos;                    // +0x11D0
    STATE m_State;                      // +0x11D4
    HRESULT m_hrQuery;                  // +0x11D8
    XONLINETASK_HANDLE m_hSearchTask;   // +0x11DC

    CDefaultQuery();
    ~CDefaultQuery();
    void Cancel();
    void Clear();
    HRESULT Query(unsigned __int64 queryGameType,
                  unsigned __int64 queryGameMap,
                  unsigned __int64 queryGameVersion,
                  unsigned __int64 QueryFriendlyFire,
                  unsigned __int64 queryTeamBalancing,
                  unsigned __int64 querySubType,
                  unsigned __int64 queryMaxPlayers,
                  unsigned __int64 queryMinPlayers);
    HRESULT Probe();
    HRESULT Process();
};
static_assert(sizeof(CDefaultQuery) == 0x11E0, "CDefaultQuery size mismatch");

// ============================================================================
// MP network classes (mp.o / shell.o - extern views for MPLiveEngine)
// ============================================================================
struct MPPlayer {
    unsigned char _pad0[0x88];
    XUID xuid;                     // +0x88
};

struct MPPlayerManager;

class MPPeer {
public:
    MPPlayerManager* GetPlayerManager();  // extern mp.o
};

class Entity;  // game_types.h
namespace math { class Position3; class Dir3; }
enum hitLocation_t;
namespace kuju { namespace knet { class sTime; } }

struct MultiplayerMgr {
    MPPeer* mPeer;                 // +0x00
    bool mRankedGame;              // +0x35
    bool mLinkCheckEnabled;        // +0x40
    bool IsHost();                 // ?IsHost@MultiplayerMgr@@QAE_NXZ
    int  GetDroppedItemType(int itemType);  // ?GetDroppedItemType@MultiplayerMgr@@QAE?AW4EDroppedItemTypes@@W4itemType_t@@@Z
    class MPEntityHandle {
    public:
        int mVal;
    };
    MPEntityHandle FindDroppedItemID(int itemType, Entity* item, Entity* owner);
    MPEntityHandle RegisterDroppedItem(int itemType, Entity* item, Entity* owner);
    void RegisterDroppedItem(int itemType, Entity* item, Entity* owner, int extra);
    void GetNextDroppedItemID(void* result, int itemType, Entity* owner);
    void DropHotJoiningPlayers();  // ?DropHotJoiningPlayers@MultiplayerMgr@@QAEXXZ
    // mp.o member stubs (sv_stubs.h has the full declarations)
    void ExitLevel();                                     // ?ExitLevel@MultiplayerMgr@@QAEXXZ
    void StartDevServer();                                // ?StartDevServer@MultiplayerMgr@@QAEXXZ
    void MapRestart();                                    // ?MapRestart@MultiplayerMgr@@QAEXXZ
    void SpotEntity(Entity* ent);                         // ?SpotEntity@MultiplayerMgr@@QAEXPAVEntity@@@Z
    void PlayerDamage(Entity* hitEntity, Entity* attacker,
                      const math::Position3& position, const math::Dir3& normal,
                      int weapon, float damage, unsigned char mod, int dflags,
                      hitLocation_t hitLocation);
    void VehicleDamage(Entity* hitEntity, Entity* attacker,
                       const math::Position3& position, const math::Dir3& normal,
                       float damage, int weapon, unsigned char mod, int dflags);
    void VehicleDeath(Entity* hitEntity, Entity* killer, int weapon, int mod);
    void ProjectileExplosion(Entity* projectile, int weapon,
                             const math::Position3& position,
                             const math::Dir3& normal,
                             unsigned char surfaceType, Entity* owner);
    void PlayerDead(Entity* player, Entity* inflictor, Entity* attacker,
                    int damage, int mod, int weapon, const float* position,
                    const float* dir, int hitLoc);
    void AttemptToRevivePlayer(Entity* player, Entity* medic);
    void FireMissile(int weapon, const math::Position3& position,
                     const math::Dir3& dir, MultiplayerMgr::MPEntityHandle handle);
    void Step(int earlyOutInterval, bool fromThread, bool a_bFromGame);
    bool IsLocalPlayer(Entity* player);
    bool IsLocalPlayer(const Entity* player);
    void DropWeapon(int weapon, int netIndex, const math::Position3* position,
                    const math::Position3* angles, const math::Dir3* velocity,
                    int clipCount, int ammoCount);
    void SpreadFire(Entity* player, float gunPitch, float gunYaw,
                    float* weaponPosition, int weapon, float spread,
                    float coneAngleTangent, int seed);
    void PickupItem(int netIndex, int itemType, Entity* player, bool scriptFrom);
    void ApplyLocalPhysicsToVehicle(Entity* vehicle, math::Position3* position,
                                    math::Position3* angles, float* velocity);
    void AttemptToGetInVehicle(Entity* vehicle, Entity* player, int seatIdx,
                               int entryIdx);
    void AttemptVehicleSeatChange(Entity* vehicle, Entity* player, int newSeatIdx);
    void GetOutOfVehicle(Entity* vehicle, int seatIdx);
    void VehicleFireMissile(Entity* vehEnt, int weapon,
                            const math::Position3* position,
                            const math::Dir3* dir);
    void FireArtillery(Entity* attacker, int weapon,
                       const math::Position3* position, int seed, bool fire);
    void VehicleMantled(Entity* vehicle, Entity* killer);
    void AnimEvent(int animEvent);                        // ?AnimEvent@MultiplayerMgr@@QAEXH@Z
    void SwapWeapon(int weapon, int netIndex, int clipCount, int ammoCount);
    void SwapKit(int playerClass, int netIndex);
    void SetPlayerPos(const Entity* player, float* pos);
    void LevelLoaded();                                   // ?LevelLoaded@MultiplayerMgr@@QAEXXZ
    void BulletHit(const math::Position3& position, const math::Dir3& normal,
                   unsigned char surfaceType, unsigned char weapon,
                   Entity* hitEntity);
    void BulletHitPlayer(Entity* hitEntity, Entity* attackerEntity,
                         const math::Position3& position,
                         const math::Dir3& normal, unsigned char surfaceType,
                         unsigned char weapon, short damage,
                         unsigned char damageFlags, unsigned char mod,
                         int hitLocation);
    void MeleeHit(Entity* hitEntity, Entity* attackerEntity,
                  const math::Position3& position, const math::Dir3& normal,
                  unsigned char surfaceType, short damage, unsigned char mod,
                  int hitLocation);
    static MultiplayerMgr* sInst;  // mp.o data
    static void Step(MultiplayerMgr* self, int earlyOutInterval,
                     bool fromThread, bool a_bFromGame);
    kuju::knet::sTime getLocalTime();
};

#ifndef COD3_KUJU_KNET_SSTIME_DEFINED
#define COD3_KUJU_KNET_SSTIME_DEFINED
namespace kuju {
namespace knet {
class sTime {
public:
    int mTime;
};
}
}
#endif

struct kuju_sTime {
    int mTime;
};

struct MPPlayerManager {
    MPPlayer* GetLocalPlayer(int nLocalPlayer);  // extern mp.o
    MPPlayer* GetPlayer(int id);                 // extern mp.o
    MPPlayer* GetPlayer(unsigned char id);       // extern mp.o
    MPPlayerSet allPlayers();                    // extern mp.o
    void Send(bdReference<bdMessage> message, MPPlayerSet players,
              bool reliable);                    // extern mp.o
    void SendOthers(bdReference<bdMessage> message, bool reliable);  // extern mp.o
};

// MPUIInterface - class statics/methods owned by mp.o (mangled as class
// statics: ?x@MPUIInterface@@1..., methods ?x@MPUIInterface@@SA...).
struct sGameListing;

class MPUIInterface {
public:
    enum EGameConnectionType : int {
        kGameConnectionTypeLan = 0,
        kGameConnectionTypeOnline = 1,
        kGameConnectionTypeLocal = 2,
    };
    static bool IsOnlineGame();            // ?IsOnlineGame@MPUIInterface@@SA_NXZ
    static bool InSession();               // ?InSession@MPUIInterface@@SA?B_NXZ
    static void ExitGame();                // ?ExitGame@MPUIInterface@@SAXXZ
    static void QueryFromID(XNKID* sessionID);  // ?QueryFromID@MPUIInterface@@SAXPAUXNKID@@@Z
    static void Step();                    // ?Step@MPUIInterface@@SAXXZ
    static sGameListing* GameListingGet(unsigned long& numGames);  // ?GameListingGet@MPUIInterface@@SAPAUsGameListing@@AAK@Z
    static bool BlockUntilNetReady();      // ?BlockUntilNetReady@MPUIInterface@@SA_NXZ
    static bool mLiveQueryActive;   // ?mLiveQueryActive@MPUIInterface@@1_NA
    static bool mQueryFromID;       // ?mQueryFromID@MPUIInterface@@1_NA
    static bool mIsViewableOnline;  // ?mIsViewableOnline@MPUIInterface@@1_NA
    static EGameConnectionType mGameConnectionType;  // ?mGameConnectionType@MPUIInterface@@1W4EGameConnectionType@@A
    static bool mInSession;         // ?mInSession@MPUIInterface@@1_NA
};

// game_xbox.o / game2.o globals used by MPLiveEngine
extern bool g_controllerConnectedErrorShown[];
extern bool g_IgnoreUIXInput;
extern int nIgnoreInputFrames;
extern bool bUIXInputDelay;

// ============================================================================
// MPLiveEngine - LiveWrapper + matchmaking session + voice (0x4600, verified)
// ============================================================================
class MPLiveEngine : public LiveWrapper {
public:
    CSession liveSession;             // +0x44D0
    QueryInterface* currentQuery;     // +0x45D0
    unsigned int actualPort;          // +0x45D4
    bool invited;                     // +0x45D8
    MPPlayerSet* mListeners;          // +0x45DC
    XNKID keySet[3];                  // +0x45E0
    int keyIndex;                     // +0x45F8
    unsigned char _pad_end[4];        // +0x45FC (binary size 0x4600)

    MPLiveEngine();
    ~MPLiveEngine();

    bool CanHear(const XUID* talker, const XUID* listener);
    void ClearKeys();
    void DoWork();
    void FreeSlot(bool isPrivate);
    unsigned int GetLockedPort();  // ?GetLockedPort@MPLiveEngine@@QAEKXZ (sv.o 0x51E190)
    static MPLiveEngine* GetHandle();
    unsigned int GetPortToLock();
    unsigned int GetVoiceData(unsigned int consoleID, unsigned int buffer);
    unsigned int GetVoiceDataSize(unsigned int consoleID);
    bool HandleInput(unsigned int port, const XINPUT_STATE* controllerInput);
    void HandleOutgoingVoice(unsigned int dwLocalPort, unsigned int dwSize,
                             const unsigned char* pData);
    void HandleSessionCreation();
    void JoinGame(XONLINE_FRIEND* joinee);
    void LeaveLiveSession();
    void LogoffCallBack();
    void LogonCallBack();
    bool OccupyPrivateSlot();
    bool OccupyPublicSlot();
    int RegisterKey(const XNKID* sessionID, const XNKEY* securityKey);
    void RunQuery(QueryInterface* newQuery);
    void SendCommunicatorStatus(UIX_VOICE_STATUS_TYPE commStatus);
    void SendMuteUpdate(XUID* muter, XUID* mutee, bool isMuted);
    void SetRemoteListeners(const MPPlayerSet* listeners);
    void StartLiveSession(sServerCreateParams* sessionParams,
                          unsigned char publicOccupied,
                          unsigned char privateOccupied);
    void SubmitVoiceData(const XUID& talker, void* buffer,
                         unsigned long bufferLength);  // ?SubmitVoiceData@MPLiveEngine@@UAEXABU_XUID@@PAXK@Z
};
static_assert(sizeof(MPLiveEngine) == 0x4600, "MPLiveEngine size mismatch");
