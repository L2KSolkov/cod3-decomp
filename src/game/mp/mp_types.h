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

namespace kuju {
namespace knet {
class sTime {
public:
    int mTime;  // +0x00
};
}
}

// MPPlayerSet - 16-player bitmask (2 bytes)
class MPPlayerSet {
public:
    unsigned short mBitPlayers;  // +0x00

    MPPlayerSet() : mBitPlayers(0) {}
    bool containsPlayer(unsigned int index) const;  // ?containsPlayer@MPPlayerSet@@QBE_NI@Z (mp.o 0x72A580)
};

// ============================================================================
// eVoteType / MPVote - round vote state (mp.o)
// ============================================================================
enum eVoteType : int {
    kNoVote = 0,  // kNoVote == 0 per IsVoteOngoing disasm (0x735990)
};

class MPVote {
public:
    kuju::knet::sTime voteStartTime;    // +0x00
    eVoteType         mVoteType;        // +0x04
    unsigned char     voteIndex;        // +0x08
    unsigned char     mYesVotes;        // +0x09
    unsigned char     mNoVotes;         // +0x0A
    unsigned char     callerIndex;      // +0x0B
    unsigned char     voteSubject;      // +0x0C
    unsigned char     eligableVoters;   // +0x0D
    uint8_t           _pad0E[2];
    int               arrPlayerMapVotes[16];  // +0x10
    bool              localVoted;             // +0x50
};
static_assert(sizeof(MPVote) == 0x54, "MPVote size mismatch");

// ============================================================================
// cThreadSleep - release no-op sleep helpers (mp.o 0x7305B0)
// ============================================================================
class cThreadSleep {
public:
    static void sleepSeconds(unsigned long secs);        // ?sleepSeconds@cThreadSleep@@SAXK@Z
    static void sleepMilliseconds(unsigned long msecs); // ?sleepMilliseconds@cThreadSleep@@SAXK@Z
};

// ============================================================================
// cBezierTrajectoryInterpolator - kuju spline interpolation (160 bytes, IDA).
// Ctor (0x73EF10) zeroes mInitialDate/mTimeInterval only.
// ============================================================================
namespace kuju {
class cBezierTrajectoryInterpolator {
public:
    uint8_t mBezier[64];          // +0x00 kuju::cBezier
    uint8_t mLinear[64];          // +0x40 tLinearInterpolator<math::Position3>
    uint8_t mLinearSpeed[16];     // +0x80 math::Dir3
    int     mInterpolationType;   // +0x90
    float   mInitialDate;         // +0x94
    float   mTimeInterval;        // +0x98
    uint8_t _pad9C[0xA0 - 0x9C];

    cBezierTrajectoryInterpolator();  // ??0cBezierTrajectoryInterpolator@kuju@@QAE@XZ (mp.o 0x73EF10)
};
static_assert(sizeof(cBezierTrajectoryInterpolator) == 160,
              "cBezierTrajectoryInterpolator size mismatch");
}

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
    uint8_t       _pad18[0x20 - 0x18];
    math::Position3 mNetPosition;            // +0x20
    math::Dir3      mNetSpeed;               // +0x30
    math::Dir3      mNetAngularVelocity;     // +0x40
    float mNetHeading;        // +0x50
    float mNetPitch;          // +0x54
    float mNetRoll;           // +0x58
    float mNetSteering;       // +0x5C
    int   mDriverState;       // +0x60
    int   mGunnerState;       // +0x64
    kuju::knet::sTime mLastReceivedTime;   // +0x68
    unsigned int mNbReceivedMessages;      // +0x6C
    bool  mReceived;                       // +0x70
    uint8_t _pad71[3];
    float mAverageUpdateInterval;          // +0x74
    uint8_t _pad78[0x80 - 0x78];
    kuju::cBezierTrajectoryInterpolator mInterpolator;  // +0x80 (160 bytes)
    int   mInterpolationState;             // +0x120
    uint8_t _pad124[0x130 - 0x124];
    math::Position3 mInterpolatedPosition;        // +0x130
    math::Dir3      mInterpolatedSpeed;           // +0x140
    math::Dir3      mInterpolatedAngularVelocity; // +0x150
    float mInterpolatedPitch;     // +0x160
    float mInterpolatedRoll;      // +0x164
    float mInterpolatedHeading;   // +0x168
    float mInterpolatedSteering;  // +0x16C
    kuju::knet::sTime mLastInterpolatedTime;  // +0x170
    int   mLastLocalNetworkTime;   // +0x174
    int   mLastRemoteNetworkTime;  // +0x178
    int   mLastDeltaDifference;    // +0x17C
    math::Dir3 mLastRemoteVelocity;  // +0x180
    uint8_t _pad190[0x210 - 0x190];

    static unsigned char GetNullId();   // ?GetNullId@MPVehicle@@SAEXZ
    unsigned char GetId() const;        // ?GetId@MPVehicle@@QBEEXZ
    void SetId(unsigned char id);       // ?SetId@MPVehicle@@QAEXE@Z
    bool IsOccupied() const;            // ?IsOccupied@MPVehicle@@QBE_NXZ
    static bool IsValid(unsigned char id);  // ?IsValid@MPVehicle@@SA_NE@Z
    void ClearOccupants();              // ?ClearOccupants@MPVehicle@@QAEXXZ (mp.o 0x72E1A0)
    void SetInvalid();                  // ?SetInvalid@MPVehicle@@QAEXXZ (mp.o 0x736EC0)
    bool IsFullyOccupied() const;       // ?IsFullyOccupied@MPVehicle@@QBE_NXZ (mp.o 0x72E480)
    bool IsSeatOccupied(int vehSeatIdx, bool ConsiderEachPositionUnique) const;  // ?IsSeatOccupied@MPVehicle@@QBE_NH_N@Z (mp.o 0x72E4A0)
    void Reset(bool clearOccupant);     // ?Reset@MPVehicle@@QAEX_N@Z (mp.o 0x72E1C0)
    MPVehicle();                        // ??0MPVehicle@@QAE@XZ (mp.o 0x7482E0)
    ~MPVehicle();                       // ??1MPVehicle@@QAE@XZ
};
static_assert(sizeof(MPVehicle) == 0x210, "MPVehicle size mismatch");

// ============================================================================
// MPPlayerItems - per-player dropped item lists (4 x ae_vector, 48 bytes)
// ============================================================================
class MPPlayerItems {
public:
    struct sDroppedItem {
        struct {
            unsigned int mVal;  // +0x00
        } handle;               // DbLinkedHandle<EntityHandleDb, Entity>
        unsigned int time;  // +0x04
        void Destroy();  // ?Destroy@sDroppedItem@MPPlayerItems@@QAEXXZ (mp.o 0x754BA0)
    };

    ae_vector<sDroppedItem> mDroppedWeapons;  // +0x00
    ae_vector<sDroppedItem> mDroppedSupport;  // +0x0C
    ae_vector<sDroppedItem> mDroppedMines;    // +0x18
    ae_vector<sDroppedItem> mDroppedKits;     // +0x24

    Entity* FindItem(EDroppedItemTypes item, short id);  // ?FindItem@MPPlayerItems@@QAEPAVEntity@@W4EDroppedItemTypes@@F@Z (mp.o)
    void SetItem(EDroppedItemTypes item, short id, Entity* ent);  // ?SetItem@MPPlayerItems@@QAEXW4EDroppedItemTypes@@FPAVEntity@@@Z (mp.o)
    void RemoveAll();  // ?RemoveAll@MPPlayerItems@@QAEXXZ (mp.o 0x754C90)

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

// ============================================================================
// MPGameInfo - game listing info (bdGameInfo + 4 slot counters at +0x28)
// ============================================================================
class MPGameInfo {
public:
    uint8_t _pad[0x28];
    unsigned char m_publicOpen;    // +0x28
    unsigned char m_privateOpen;   // +0x29
    unsigned char m_publicFilled;  // +0x2A
    unsigned char m_privateFilled; // +0x2B

    void getSlots(unsigned char& publicOpen, unsigned char& privateOpen,
                  unsigned char& publicFilled,
                  unsigned char& privateFilled) const;  // ?getSlots@MPGameInfo@@QBEXAAE000@Z (mp.o 0x730510)
};

// EGameConnectionType (mp.o)
enum EGameConnectionType : int {
    kGameConnectionTypeLan = 0,
    kGameConnectionTypeOnline = 1,
    kGameConnectionTypeLocal = 2,
};

struct sServerCreateParams;  // defined below MPUIInterface
struct sServerQueryParams;   // defined below MPUIInterface

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
    static void Reboot();                // ?Reboot@MPUIInterface@@SAXXZ (mp.o 0x72F380)
    static void ResolveVote();           // ?ResolveVote@MPUIInterface@@SAXXZ (mp.o 0x73D6C0)
    static void SetServerParams(const sServerCreateParams& a_ServerParams);  // ?SetServerParams@MPUIInterface@@SAXABUsServerCreateParams@@@Z (mp.o 0x730240)
    static void SetQueryParams(sServerQueryParams& params);  // ?SetQueryParams@MPUIInterface@@SAXAAUsServerQueryParams@@@Z (mp.o 0x72F520)
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
    static struct sServerQueryParams mQueryParams;  // ?mQueryParams@MPUIInterface@@1UsServerQueryParams@@A
};

// sServerCreateParams - host session setup (mMapID at +0x58)
struct sServerCreateParams {
    char mRandomMapList[64];  // +0x00
    char mName[24];           // +0x40
    unsigned char mMapID;     // +0x58
    unsigned char mGameType;  // +0x59
};

// sServerQueryParams - LAN query filters (44 bytes)
struct sServerQueryParams {
    unsigned int mGameType;       // +0x00
    unsigned int mMapID;          // +0x04
    unsigned int mMaxPlayers;     // +0x08
    unsigned int mMinPlayers;     // +0x0C
    unsigned int mGameSubType;    // +0x10
    unsigned int mTeamBalancing;  // +0x14
    unsigned int mFriendlyFire;   // +0x18
    char mSessionNamePrefix[16];  // +0x1C
    unsigned int mListIfFull;     // +0x2C
};

// EDroppedItemTypes (mp.o); enumerators kept out of the global scope to avoid
// colliding with the anonymous enums in g_local.h.
enum EDroppedItemTypes : int;

// ============================================================================
// MP options menus (shell FE menu subclasses; mp.o vtable overrides)
// ============================================================================
// Minimal front-end views (mp.o overrides; full types live in ui_types.h).
class FEText;
class FEMultiLineText {
public:
    virtual ~FEMultiLineText();      // ??1FEMultiLineText@@UAE@XZ (shell.o)
    virtual void UpdateForWidescreen(bool widescreen);  // ?UpdateForWidescreen@FEMultiLineText@@UAEX_N@Z (shell.o)
};

class UIListBox {
public:
    uint8_t _pad[172];  // +0x00 (full type in ui_types.h)
    void RemoveAllItems();  // ?RemoveAllItems@UIListBox@@QAEXXZ (shell.o 0x5816B0)
};
static_assert(sizeof(UIListBox) == 172, "UIListBox size mismatch");

class PanelFile {
public:
    void UpdateWidescreen(bool widescreen, float about_x);  // ?UpdateWidescreen@PanelFile@@QAEX_NM@Z (shell.o)
};

// FEMenu base (0x4C) - minimal view; members/virtuals used by mp.o overrides
class FEMenu {
public:
    void**        entries;              // +0x04
    FEMenuSystem* system;               // +0x08
    uint8_t       _pad0C[0x32 - 0x0C];
    char          button_held_down;     // +0x32
    uint8_t       _pad33[0x48 - 0x33];
    PanelFile*    panel;                // +0x48

    virtual void PanelFileUnloaded(PanelFile* pf);  // slot 1 shell.o 0x5B7570
    virtual void UpdateWidescreen(bool widescreen); // slot 2 shell.o 0x57DF20
    virtual void OnUp(int c);                        // slot 32 shell.o 0x5AE910
    virtual void OnDown(int c);                      // slot 33 shell.o 0x5AE920
    virtual void OnTriangle(int c);                  // slot 46 shell.o
    void Cleanup();                 // ?Cleanup@FEMenu@@QAEXXZ (shell.o)
};
static_assert(sizeof(FEMenu) == 0x4C, "FEMenu size mismatch");

// ProfileManager (shell.o) - methods used by mp.o profile menus
class ProfileManager {
public:
    static ProfileManager* Me();             // ?Me@ProfileManager@@SAPAV1@XZ (shell.o 0x5751D0)
    void Reset();                            // ?Reset@ProfileManager@@QAEXXZ (shell.o 0x575210)
    void EnumProfiles(SaveGameData** slots); // ?EnumProfiles@ProfileManager@@QAEXQAPAUSaveGameData@@@Z (shell.o 0x5935B0)
    const char* GetLoadedProfile() const;    // ?GetLoadedProfile@ProfileManager@@QBEPBDXZ (shell.o 0x575420)
};

// MusicMgr (game.o) - used by MPUIInterface::Reboot
class MusicMgr {
public:
    static MusicMgr* sInst;   // ?sInst@MusicMgr@@2PAV1@A (game.o)
    void Stop(float fadeOutTime);  // ?Stop@MusicMgr@@QAEXM@Z (game.o 0x221830)
};

// controller (controller.o) - locked_port at +0x18 (IDA)
class controller {
public:
    uint8_t _pad[0x18];
    int     locked_port;          // +0x18
    static controller* inst();    // ?inst@controller@@SAPAV1@XZ (controller.o)
};

class MPOptionsScreenMenu {
public:
    static MPOptionsScreenMenu* Me();  // ?Me@MPOptionsScreenMenu@@SAPAV1@XZ
    virtual void Select(int entry_num);  // ?Select@MPOptionsScreenMenu@@UAEXH@Z
    virtual void OnCross(int c);         // ?OnCross@MPOptionsScreenMenu@@UAEXH@Z
    virtual void Update(float time_inc); // ?Update@MPOptionsScreenMenu@@UAEXM@Z
};

class MPOptionsSoundMenu : public FEMenu {
public:
    FEText* mSoundText[4];      // +0x4C
    int     mOutputVal;         // +0x5C
    int     mMusicVal;          // +0x60
    int     mEffectsVal;        // +0x64
    FEMultiLineText* mInstructionsText;  // +0x68
    bool    mWidescreen;        // +0x6C

    static MPOptionsSoundMenu* Me();  // ?Me@MPOptionsSoundMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsSoundMenu@@UAEXM@Z
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPOptionsSoundMenu@@UAEX_N@Z (mp.o 0x731560)
};

class MPOptionsControlsMenu : public FEMenu {
public:
    FEText* mControlsText[4];   // +0x4C
    FEMultiLineText* mInstructionsText;  // +0x5C
    bool    mWidescreen;        // +0x60

    static MPOptionsControlsMenu* Me();  // ?Me@MPOptionsControlsMenu@@SAPAV1@XZ
    virtual void Update(float time_inc);  // ?Update@MPOptionsControlsMenu@@UAEXM@Z
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPOptionsControlsMenu@@UAEX_N@Z (mp.o 0x7320F0)
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
class MPProfileEditMenu : public FEMenu {
public:
    bool    mNeedWrite;          // +0x4C
    bool    mWidescreen;         // +0x4D
    FEText* mProfileEditText[4]; // +0x50
    FEMultiLineText* mInstructionsText;  // +0x60
    UIListBox mListBox;          // +0x64 (172 bytes)

    static MPProfileEditMenu* Me();  // ?Me@MPProfileEditMenu@@SAPAV1@XZ
    static bool DialogResponseOk(int index);  // ?DialogResponseOk@MPProfileEditMenu@@SA_NH@Z
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@MPProfileEditMenu@@UAEXPAVPanelFile@@@Z (mp.o 0x733640)
    virtual void OnTriangle(int c);                          // ?OnTriangle@MPProfileEditMenu@@UAEXH@Z (mp.o 0x733770)
    virtual void ButtonHeldAction();                         // ?ButtonHeldAction@MPProfileEditMenu@@UAEXXZ (mp.o 0x733890)
};

class MPProfileMainMenu : public FEMenu {
public:
    int      mMenuState;         // +0x4C
    int      mMenuStatus[6];     // +0x50
    SaveGameData* mSaveSlots[6]; // +0x68
    const char*   mSelectedProfile;  // +0x80
    PanelFile*    mPanel;        // +0x84
    FEMultiLineText* mHelpBar;   // +0x88

    static MPProfileMainMenu* Me();  // ?Me@MPProfileMainMenu@@SAPAV1@XZ
    static bool DialogResponseDeleteCancel(int index);  // ?DialogResponseDeleteCancel@MPProfileMainMenu@@SA_NH@Z
    static bool DialogResponseProfileEdit(int index);   // ?DialogResponseProfileEdit@MPProfileMainMenu@@SA_NH@Z
    static bool DialogResponseNoMemCard(int index);     // ?DialogResponseNoMemCard@MPProfileMainMenu@@SA_NH@Z
    static void LoadProfileData();   // ?LoadProfileData@MPProfileMainMenu@@SAXXZ (mp.o 0x733AF0)
    virtual void Select(int entry_num);  // ?Select@MPProfileMainMenu@@UAEXH@Z (mp.o 0x764210)
    virtual void OnCross(int c);         // ?OnCross@MPProfileMainMenu@@UAEXH@Z (mp.o 0x733D80)
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@MPProfileMainMenu@@UAEX_N@Z (mp.o 0x733B20)
};

static_assert(sizeof(MPOptionsSoundMenu) == 0x70, "MPOptionsSoundMenu size mismatch");
static_assert(sizeof(MPOptionsControlsMenu) == 0x64, "MPOptionsControlsMenu size mismatch");
static_assert(sizeof(MPProfileEditMenu) == 0x110, "MPProfileEditMenu size mismatch");
static_assert(sizeof(MPProfileMainMenu) == 0x8C, "MPProfileMainMenu size mismatch");

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
    virtual ~cVoiceNetworkManager();  // ??1cVoiceNetworkManager@knetuser@kuju@@UAE@XZ
    uint8_t _pad[4];
    void*   mVoiceHandlerInterface;  // +0x08
    void deinitialise();             // ?deinitialise@cVoiceNetworkManager@knetuser@kuju@@QAEXXZ
    void update(const kuju::knet::sTime& time);             // ?update@cVoiceNetworkManager@knetuser@kuju@@QAEXABVsTime@knet@3@@Z (mp.o 0x7500E0)
    void updateVoiceNetwork(const kuju::knet::sTime& time); // ?updateVoiceNetwork@cVoiceNetworkManager@knetuser@kuju@@AAEXABVsTime@knet@3@@Z (mp.o 0x7500C0)
private:
    void checkForPendingPacketsAwaitingHandling(const kuju::knet::sTime& time);  // (mp.o 0x735250)
    void checkForPendingPacketsAwaitingDispatch(const kuju::knet::sTime& time);  // (mp.o 0x750000)
};
}

namespace kvoicemanager {
class cVoiceManager {
public:
    virtual ~cVoiceManager();  // ??1cVoiceManager@kvoicemanager@kuju@@UAE@XZ (mp.o 0x7348B0)
    int             mInitialised;  // +0x04
    kuju::knetuser::cVoiceNetworkManager mVoiceNetworkManager;  // +0x08
    uint8_t         _pad2[0x266C - (0x08 + sizeof(kuju::knetuser::cVoiceNetworkManager))];
    MPPlayerSet     mRemoteListeners;  // +0x266C
    MPPlayerSet     mConnectedPlayers; // +0x266E
    kuju::knet::sTime mLastNetworkDispatchTime;      // +0x2670
    kuju::knet::sTime mRealLastNetworkDispatchTime;  // +0x2674
    unsigned char   mEncodeBuffer[2500];             // +0x2678
    unsigned int    mEncodeDstOffset;                // +0x303C
    unsigned int    mNetworkDispatchOffset;          // +0x3040
    unsigned int    mPlaybackActive;                 // +0x3044
    unsigned int    mPlaybackDataAvailable;          // +0x3048
    kuju::knet::sTime mPlaybackDataAvailableStartTime;  // +0x304C

    void deinitialise();   // ?deinitialise@cVoiceManager@kvoicemanager@kuju@@QAEXXZ
    void loadIRXModules(); // ?loadIRXModules@cVoiceManager@kvoicemanager@kuju@@QAEXXZ
    void setRemoteListeners(MPPlayerSet& players);  // ?setRemoteListeners@cVoiceManager@kvoicemanager@kuju@@QAEXAAVMPPlayerSet@@@Z
    virtual void receiveVoiceData(unsigned int fromPlayerIndex,
                                  unsigned char* buffer,
                                  unsigned int length);  // ?receiveVoiceData@cVoiceManager@kvoicemanager@kuju@@UAEXKPAEK@Z (mp.o 0x7348F0)
private:
    void startSystem();    // ?startSystem@cVoiceManager@kvoicemanager@kuju@@AAEXXZ (mp.o 0x734910)
    void stopSystem();     // ?stopSystem@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void startLoopback();  // ?startLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void stopLoopback();   // ?stopLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
    void updateLoopback(); // ?updateLoopback@cVoiceManager@kvoicemanager@kuju@@AAEXXZ
};
}
}
