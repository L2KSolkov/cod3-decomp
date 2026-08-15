// ============================================================================
// mp_types.h - multiplayer game-mode (mp.o) shared types
// Reconstructed from IDA local types (PDB symbol data); offsets verified via
// disasm of the release binary.
// ============================================================================

#pragma once

#include <stdint.h>

#include "bd/bdSession.h"
#include "bd/bd_types.h"

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
    ~MPVehicle();                       // ??1MPVehicle@@QAE@XZ
};

// ============================================================================
// MPLanDiscovery - LAN session discovery results
// ============================================================================
class MPLanDiscovery {
public:
    uint8_t _pad[0x48];
    unsigned int mNumResults;  // +0x48

    unsigned int GetNumResults() const;  // ?GetNumResults@MPLanDiscovery@@QBEIXZ
};

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

    static int  mReturnMenu;  // ?mReturnMenu@MPUIInterface@@1HA @ 0xF0A124
    static bool mKicked;      // ?mKicked@MPUIInterface@@1_NA @ 0xF0A128
};

// ============================================================================
// MP options menus (shell FE menu subclasses; mp.o vtable overrides)
// ============================================================================
class MPOptionsScreenMenu {
public:
    virtual void Select(int entry_num);  // ?Select@MPOptionsScreenMenu@@UAEXH@Z
    virtual void OnCross(int c);         // ?OnCross@MPOptionsScreenMenu@@UAEXH@Z
    virtual void Update(float time_inc); // ?Update@MPOptionsScreenMenu@@UAEXM@Z
};

class MPOptionsSoundMenu {
public:
    virtual void Update(float time_inc);  // ?Update@MPOptionsSoundMenu@@UAEXM@Z
};

class MPOptionsControlsMenu {
public:
    virtual void Update(float time_inc);  // ?Update@MPOptionsControlsMenu@@UAEXM@Z
};

class MPOptionsGameplayMenu {
public:
    virtual void Update(float time_inc);  // ?Update@MPOptionsGameplayMenu@@UAEXM@Z
};

class MPOptionsPreferencesMenu {
public:
    virtual void Update(float time_inc);  // ?Update@MPOptionsPreferencesMenu@@UAEXM@Z
};

// ============================================================================
// MP profile menus
// ============================================================================
class MPProfileEditMenu {
public:
    static bool DialogResponseOk(int index);  // ?DialogResponseOk@MPProfileEditMenu@@SA_NH@Z
};

class MPProfileMainMenu {
public:
    static bool DialogResponseDeleteCancel(int index);  // ?DialogResponseDeleteCancel@MPProfileMainMenu@@SA_NH@Z
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
    uint8_t _pad[4];
    int     mInitialised;  // +0x04
    kuju::knetuser::cVoiceNetworkManager mVoiceNetworkManager;  // +0x08

    void deinitialise();   // ?deinitialise@cVoiceManager@kvoicemanager@kuju@@QAEXXZ
    void loadIRXModules(); // ?loadIRXModules@cVoiceManager@kvoicemanager@kuju@@QAEXXZ
private:
    void stopSystem();     // ?stopSystem@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void startLoopback();  // ?startLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void stopLoopback();   // ?stopLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void updateLoopback(); // ?updateLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
};
}
}
