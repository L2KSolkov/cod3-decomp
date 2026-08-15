// ============================================================================
// mp_types.h - multiplayer game-mode (mp.o) shared types
// Reconstructed from IDA local types (PDB symbol data); offsets verified via
// disasm of the release binary.
// ============================================================================

#pragma once

#include <stdint.h>

#include "bd/bdSession.h"
#include "bd/bd_types.h"
#include "bd/bdDiscovery.h"
#include "core/ae_array.h"

class Entity;  // game_types.h

// MPPlayerSet - 16-player bitmask (2 bytes)
class MPPlayerSet {
public:
    unsigned short mBitPlayers;  // +0x00

    MPPlayerSet() : mBitPlayers(0) {}
};

// ============================================================================
// cThreadSleep - release no-op sleep helpers (mp.o 0x7305B0)
// ============================================================================
class cThreadSleep {
public:
    static void sleepSeconds(unsigned long secs);        // ?sleepSeconds@cThreadSleep@@SAXK@Z
    static void sleepMilliseconds(unsigned long msecs); // ?sleepMilliseconds@cThreadSleep@@SAXK@Z
};

// ============================================================================
// MPVehicle - multiplayer vehicle (id + entity + seats + net state)
// ============================================================================
class MPVehicle {
public:
    unsigned char mId;        // +0x00
    uint8_t       _pad1[3];
    void*         mEntity;    // +0x04
    uint8_t       _seats[0x0C];  // +0x08 (11 seats)
    int           mNumOccupants;  // +0x14

    static unsigned char GetNullId();   // ?GetNullId@MPVehicle@@SAEXZ
    unsigned char GetId() const;        // ?GetId@MPVehicle@@QBEEXZ
    void SetId(unsigned char id);       // ?SetId@MPVehicle@@QAEXE@Z
    bool IsOccupied() const;            // ?IsOccupied@MPVehicle@@QBE_NXZ
    static bool IsValid(unsigned char id);  // ?IsValid@MPVehicle@@SA_NE@Z
    void ClearOccupants();              // ?ClearOccupants@MPVehicle@@QAEXXZ (mp.o 0x72E1A0)
    void SetInvalid();                  // ?SetInvalid@MPVehicle@@QAEXXZ (mp.o 0x736EC0)
    bool IsFullyOccupied() const;       // ?IsFullyOccupied@MPVehicle@@QBE_NXZ (mp.o 0x72E480)
    bool IsSeatOccupied(int vehSeatIdx, bool ConsiderEachPositionUnique) const;  // ?IsSeatOccupied@MPVehicle@@QBE_NH_N@Z (mp.o 0x72E4A0)
    ~MPVehicle();                       // ??1MPVehicle@@QAE@XZ
};

// ============================================================================
// MPPlayerItems - per-player dropped item lists (4 x ae_vector, 48 bytes)
// ============================================================================
class MPPlayerItems {
public:
    struct sDroppedItem {
        void* handle;   // +0x00 DbLinkedHandle<EntityHandleDb, Entity>
        unsigned int time;  // +0x04
    };

    ae_vector<sDroppedItem> mDroppedWeapons;  // +0x00
    ae_vector<sDroppedItem> mDroppedSupport;  // +0x0C
    ae_vector<sDroppedItem> mDroppedMines;    // +0x18
    ae_vector<sDroppedItem> mDroppedKits;     // +0x24

    Entity* FindItem(EDroppedItemTypes item, short id);  // ?FindItem@MPPlayerItems@@QAEPAVEntity@@W4EDroppedItemTypes@@F@Z (mp.o)
    void SetItem(EDroppedItemTypes item, short id, Entity* ent);  // ?SetItem@MPPlayerItems@@QAEXW4EDroppedItemTypes@@FPAVEntity@@@Z (mp.o)

private:
    ae_vector<sDroppedItem>& GetItemList(EDroppedItemTypes item);  // ?GetItemList@MPPlayerItems@@AAEAAV?$ae_vector@UsDroppedItem@MPPlayerItems@@@@W4EDroppedItemTypes@@@Z (mp.o 0x72E020)
};

// ============================================================================
// MPLanDiscovery - LAN session discovery results
// ============================================================================
class MPLanDiscovery {
public:
    uint8_t _pad[0x40];
    bdDiscoveryClient mDiscoveryClient;  // +0x40
    unsigned int mNumResults;  // +0x48

    unsigned int GetNumResults() const;  // ?GetNumResults@MPLanDiscovery@@QBEIXZ
    bool IsDone();                       // ?IsDone@MPLanDiscovery@@QAE_NXZ (mp.o 0x72CB60)
};

// EGameConnectionType (mp.o)
enum EGameConnectionType : int {
    kGameConnectionTypeLan = 0,
    kGameConnectionTypeOnline = 1,
    kGameConnectionTypeLocal = 2,
};

struct sServerCreateParams;  // defined below MPUIInterface

// ============================================================================
// MPUIInterface - multiplayer shell/UI static interface
// ============================================================================
class MPUIInterface {
public:
    static void getLocalAddresses(bdArray<bdInetAddr>& addrs);  // ?getLocalAddresses@MPUIInterface@@SAXAAV?$bdArray@VbdInetAddr@@@@@Z
    static void Logging(bool enabled);   // ?Logging@MPUIInterface@@SAX_N@Z
    static int  GetTimeLimitCount();     // ?GetTimeLimitCount@MPUIInterface@@SAHXZ
    static int  GetRoundLimitCount();    // ?GetRoundLimitCount@MPUIInterface@@SAHXZ
    static int  GetMaxPlayersCount();    // ?GetMaxPlayersCount@MPUIInterface@@SAHXZ
    static int  GetRespawnTimeCount();   // ?GetRespawnTimeCount@MPUIInterface@@SAHXZ
    static int  GetReturnMenu();         // ?GetReturnMenu@MPUIInterface@@SAHXZ
    static void GameListingEnd();        // ?GameListingEnd@MPUIInterface@@SAXXZ
    static void StartDevice();           // ?StartDevice@MPUIInterface@@SAXXZ
    static void PlatformStop();          // ?PlatformStop@MPUIInterface@@SAXXZ
    static void SetServerParams(const sServerCreateParams& a_ServerParams);  // ?SetServerParams@MPUIInterface@@SAXABUsServerCreateParams@@@Z (mp.o 0x730240)
    static bool NextRoundMapChanges();   // ?NextRoundMapChanges@MPUIInterface@@SA_NXZ
    static bool NextRoundMapRestart();   // ?NextRoundMapRestart@MPUIInterface@@SA_NXZ (mp.o 0x7300A0)
    static const bool IsGameListingComplete(); // ?IsGameListingComplete@MPUIInterface@@SA?B_NXZ (mp.o 0x72F730)

    static int  mReturnMenu;  // ?mReturnMenu@MPUIInterface@@1HA @ 0xF0A124
    static bool mKicked;      // ?mKicked@MPUIInterface@@1_NA @ 0xF0A128
    static struct sServerCreateParams mServerParams;     // ?mServerParams@MPUIInterface@@1UsServerCreateParams@@A
    static struct sServerCreateParams mNextServerParams; // ?mNextServerParams@MPUIInterface@@1UsServerCreateParams@@A
    static EGameConnectionType mGameConnectionType;  // ?mGameConnectionType@MPUIInterface@@1W4EGameConnectionType@@A
    static bool mLanDiscoveryActive;  // ?mLanDiscoveryActive@MPUIInterface@@1_NA
    static bool mLiveQueryActive;     // ?mLiveQueryActive@MPUIInterface@@1_NA
};

// sServerCreateParams - host session setup (mMapID at +0x58)
struct sServerCreateParams {
    char mRandomMapList[64];  // +0x00
    char mName[24];           // +0x40
    unsigned char mMapID;     // +0x58
    unsigned char mGameType;  // +0x59
};

// EDroppedItemTypes (mp.o); enumerators kept out of the global scope to avoid
// colliding with the anonymous enums in g_local.h.
enum EDroppedItemTypes : int;

// ============================================================================
// MP options menus (shell FE menu subclasses; mp.o vtable overrides)
// ============================================================================
class MPOptionsScreenMenu {
public:
    static MPOptionsScreenMenu* Me();  // ?Me@MPOptionsScreenMenu@@SAPAV1@XZ
    virtual void Select(int entry_num);  // ?Select@MPOptionsScreenMenu@@UAEXH@Z
    virtual void OnCross(int c);         // ?OnCross@MPOptionsScreenMenu@@UAEXH@Z
    virtual void Update(float time_inc); // ?Update@MPOptionsScreenMenu@@UAEXM@Z
};

class MPOptionsSoundMenu {
public:
    static MPOptionsSoundMenu* Me();  // ?Me@MPOptionsSoundMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsSoundMenu@@UAEXM@Z
};

class MPOptionsControlsMenu {
public:
    static MPOptionsControlsMenu* Me();  // ?Me@MPOptionsControlsMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsControlsMenu@@UAEXM@Z
};

class MPOptionsGameplayMenu {
public:
    static MPOptionsGameplayMenu* Me();  // ?Me@MPOptionsGameplayMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsGameplayMenu@@UAEXM@Z
};

class MPOptionsPreferencesMenu {
public:
    static MPOptionsPreferencesMenu* Me();  // ?Me@MPOptionsPreferencesMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsPreferencesMenu@@UAEXM@Z
};

// ============================================================================
// MP profile menus
// ============================================================================
class MPProfileEditMenu {
public:
    static MPProfileEditMenu* Me();  // ?Me@MPProfileEditMenu@@SAPAV1@XZ
    static bool DialogResponseOk(int index);  // ?DialogResponseOk@MPProfileEditMenu@@SA_NH@Z
};

class MPProfileMainMenu {
public:
    static MPProfileMainMenu* Me();  // ?Me@MPProfileMainMenu@@SAPAV1@XZ
    static bool DialogResponseDeleteCancel(int index);  // ?DialogResponseDeleteCancel@MPProfileMainMenu@@SA_NH@Z
    static bool DialogResponseProfileEdit(int index);   // ?DialogResponseProfileEdit@MPProfileMainMenu@@SA_NH@Z
    static bool DialogResponseNoMemCard(int index);     // ?DialogResponseNoMemCard@MPProfileMainMenu@@SA_NH@Z
    virtual void Select(int entry_num);  // ?Select@MPProfileMainMenu@@UAEXH@Z (mp.o 0x764210)
    virtual void OnCross(int c);         // ?OnCross@MPProfileMainMenu@@UAEXH@Z (mp.o 0x733D80)
};

// ============================================================================
// kuju utility / voice classes (mp.o)
// ============================================================================
namespace kuju {
class cBezier {
public:
    cBezier();  // ??0cBezier@kuju@@QAE@XZ
};

namespace knetuser {
class cVoiceNetworkManager {
public:
    uint8_t _pad[8];
    void*   mVoiceHandlerInterface;  // +0x08
    void deinitialise();             // ?deinitialise@cVoiceNetworkManager@knetuser@kuju@@QAEXXZ
};
}

namespace kvoicemanager {
class cVoiceManager {
public:
    uint8_t         _pad[4];
    int             mInitialised;  // +0x04
    kuju::knetuser::cVoiceNetworkManager mVoiceNetworkManager;  // +0x08
    uint8_t         _pad2[0x266C - (0x08 + sizeof(kuju::knetuser::cVoiceNetworkManager))];
    unsigned short  mRemoteListeners;  // +0x266C

    void deinitialise();   // ?deinitialise@cVoiceManager@kvoicemanager@kuju@@QAEXXZ
    void loadIRXModules(); // ?loadIRXModules@cVoiceManager@kvoicemanager@kuju@@QAEXXZ
    void setRemoteListeners(MPPlayerSet& players);  // ?setRemoteListeners@cVoiceManager@kvoicemanager@kuju@@QAEXAAVMPPlayerSet@@@Z
private:
    void stopSystem();     // ?stopSystem@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void startLoopback();  // ?startLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void stopLoopback();   // ?stopLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void updateLoopback(); // ?updateLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
};
}

// cBezierTrajectoryInterpolator - kuju spline interpolation (ctor zeros two
// doubles at 0x73EF10).
class cBezierTrajectoryInterpolator {
public:
    double mInitialDate;   // +0x00
    double mTimeInterval;  // +0x08

    cBezierTrajectoryInterpolator();  // ??0cBezierTrajectoryInterpolator@kuju@@QAE@XZ
};
}
