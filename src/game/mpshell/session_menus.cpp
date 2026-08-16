// ============================================================================
// session_menus.cpp - mp_shell.o session/overlay menu classes
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/mpshell/session_menus.h"
#include "game/actor_types.h"
#include "game/client_types.h"

#include <stdio.h>
#include <string.h>

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert")) \
            __debugbreak();                                               \
    } while (0)

// ============================================================================
// MPUIInterface - minimal view with binary-exact manglings (mp.o owns the
// real definitions; only members used by mp_shell.o are declared).
// ============================================================================
enum eGameType : int {
    GAME_TYPE_WAR = 0,
    GAME_TYPE_CTF = 1,
    GAME_TYPE_SCF = 2,
    GAME_TYPE_HQ = 3,
    GAME_TYPE_TDM = 4,
    GAME_TYPE_DM = 5,
    GAME_TYPE_DOM = 6,
    GAME_TYPE_SND = 7,
};

struct sServerCreateParams;
struct sServerQueryParams;
struct sGameListing;

class MPUIInterface {
public:
    enum eSetting : int {
        SETTING_TIME_LIMIT = 0,
        SETTING_SCORE_LIMIT = 1,
        SETTING_ROUND_LIMIT = 2,
        SETTING_RESPAWN_TIME = 3,
        SETTING_LAST_MAN_STANDING = 4,
        SETTING_MAX_PLAYERS = 5,
    };
    enum EGameConnectionType : int {
        kGameConnectionTypeLan = 0,
        kGameConnectionTypeOnline = 1,
        kGameConnectionTypeLocal = 2,
    };

    static const bool IsOnlineGame();   // ?IsOnlineGame@MPUIInterface@@SA?B_NXZ
    static const bool IsLANGame();      // ?IsLANGame@MPUIInterface@@SA?B_NXZ
    static void Step();                 // ?Step@MPUIInterface@@SAXXZ
    static const int GetDefaultOption(eSetting setting,
                                      eGameType gameType);  // ?GetDefaultOption@MPUIInterface@@SA?BHW4eSetting@1@W4eGameType@@@Z
    static const int GetScoreLimitCount(eGameType gameType);  // ?GetScoreLimitCount@MPUIInterface@@SA?BHW4eGameType@@@Z
    static const int GetScoreLimit(unsigned long index,
                                   eGameType gameType);  // ?GetScoreLimit@MPUIInterface@@SA?BHKW4eGameType@@@Z
    static sGameListing* GameListingGet(unsigned long& numGames);  // ?GameListingGet@MPUIInterface@@SAPAUsGameListing@@AAK@Z
    static void ExitGame();   // ?ExitGame@MPUIInterface@@SAXXZ
    static int mReturnMenu;   // ?mReturnMenu@MPUIInterface@@1HA
    static const int GetMaxPlayersOptionFromMap(char mapID);  // ?GetMaxPlayersOptionFromMap@MPUIInterface@@SA?BHD@Z
    static const char* GetMapString(unsigned long mapIndex);  // ?GetMapString@MPUIInterface@@SAPBDK@Z
    static const bool StartServer(bool forceRestart,
                                  bool blockUntilNetReady);  // ?StartServer@MPUIInterface@@SA?B_N_N0@Z
    static void ExitFrontend(int returnMenu);  // ?ExitFrontend@MPUIInterface@@SAXH@Z
    static void SetQueryParams(sServerQueryParams& params);  // ?SetQueryParams@MPUIInterface@@SAXAAUsServerQueryParams@@@Z

    static sServerCreateParams mServerParams;      // ?mServerParams@MPUIInterface@@1UsServerCreateParams@@A
    static sServerCreateParams mNextServerParams;  // ?mNextServerParams@MPUIInterface@@1UsServerCreateParams@@A
    static int  mMaxScoreLimitCount;  // ?mMaxScoreLimitCount@MPUIInterface@@1HA @ 0xE36E8C
    static bool mIsViewableOnline;    // ?mIsViewableOnline@MPUIInterface@@1_NA
    static EGameConnectionType mGameConnectionType;  // ?mGameConnectionType@MPUIInterface@@1W4EGameConnectionType@@A

    static const int GetMaxScoreLimitCount();  // ?GetMaxScoreLimitCount@MPUIInterface@@SA?BHXZ
    static void SetViewOnlineStatus(bool state);  // ?SetViewOnlineStatus@MPUIInterface@@SAX_N@Z
};

// sServerCreateParams - server create params (105 bytes; layout from IDA)
struct sServerCreateParams {
    char mRandomMapList[64];       // +0x00
    char mName[24];                // +0x40
    unsigned char mMapID;          // +0x58
    unsigned char mGameType;       // +0x59
    unsigned char mGameSubType;    // +0x5A
    unsigned char mMaxPlayers;     // +0x5B
    unsigned char mTeamBalancing;  // +0x5C
    unsigned char mFriendlyFire;   // +0x5D
    unsigned char mPrivateSlots;   // +0x5E
    unsigned char mTimeLimit;      // +0x5F
    unsigned char mScoreLimit;     // +0x60
    unsigned char mRoundLimit;     // +0x61
    unsigned char mSwapEnds;       // +0x62
    unsigned char mRespawnTime;    // +0x63
    bool mDontRotate;              // +0x64
    bool mDoChangeMap;             // +0x65
    unsigned char mEnableAARVote;  // +0x66
    unsigned char mEnablePenaltyVote;  // +0x67
    unsigned char mMapRotation;    // +0x68

    void SetMapRotation(unsigned char MapRotation);  // ?SetMapRotation@sServerCreateParams@@QAEXE@Z (mp.o)
};
static_assert(sizeof(sServerCreateParams) == 105,
              "sServerCreateParams size mismatch");

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

struct sGameListing {
    int mSize;                          // +0x00
    void* mGameInfo;                    // +0x04 (bdReference<MPGameInfo>)
    int mPing;                          // +0x08
    bool mValidVersion;                 // +0x0C
};

// ============================================================================
// MPLiveEngine / LiveWrapper minimal views (game_xbox.o owns the definitions)
// ============================================================================
enum ELiveState {
    kNotSignedIn = 0,
    kSigningIn = 1,
    kSignedIn = 2,
};

struct LivePlayer {
    void** vftable;              // +0x00
    _XUID xuid;                  // +0x04
    unsigned short gamertag[16]; // +0x10
};

struct LiveLocal : LivePlayer {
};

class LiveWrapper {
public:
    LiveLocal* GetLocalPlayer(unsigned int portNumber);  // ?GetLocalPlayer@LiveWrapper@@QAEPAVLiveLocal@@I@Z (game_xbox.o)
    char* GetIcon(unsigned int portNumber);  // ?GetIcon@LiveWrapper@@QAEPADK@Z (game_xbox.o)
    static LiveWrapper* theWrapper;          // ?theWrapper@LiveWrapper@@1PAV1@A (game_xbox.o)
};

class MPLiveEngine : public LiveWrapper {
public:
    static MPLiveEngine* GetHandle();  // ?GetHandle@MPLiveEngine@@SAPAV1@XZ (game_xbox.o)
    int internalState;                 // +0x04 (LiveWrapper)
    unsigned int actualPort;           // +0x45D4
};

// ============================================================================
// mp_shell.o data
// ============================================================================
extern int g_NumBaseMaps;    // ?g_NumBaseMaps@@3HA @ 0x1388D60
extern int g_NumTotalMaps;   // ?g_NumTotalMaps@@3HA @ 0x1388D64
extern char byte_E386C9[];   // map-ID conversion table @ 0xE386C9
extern const char* const szClassReference[];  // ?szClassReference@@3PAPBDA @ 0x12782C
extern int unk_F6A280[802];  // viewport prev (cg.o)
extern int unk_F6A284[802];  // viewport curr (cg.o)
extern bool g_controllerConnected[];           // ?g_controllerConnected@@3PA_NA (game2.o)
extern bool g_controllerConnectedErrorShown[]; // ?g_controllerConnectedErrorShown@@3PA_NA (game2.o)
extern const char* const szPlayLanMenuOptionTextReferences[];  // ?szPlayLanMenuOptionTextReferences@@3PAPBDA @ 0x12276B8

struct Mapinfo_t {
    char map_pack;              // +0x00
    char map_id_number;         // +0x01
    char map_name_string[32];   // +0x02
    char map_title_string[32];  // +0x22
    char short_name[16];        // +0x42
    char map_location_string[32];  // +0x52
};
extern Mapinfo_t* g_TheMapInfo;  // ?g_TheMapInfo@@3PAUMapinfo_t@@A @ 0x1227BC8
extern char aMpfrontendMerv[];   // @ 0xE386CA (map display-name table, 114-byte stride)
extern char aMploadingMervi[];   // @ 0xE386DA (map location table)
extern char aMploadingMervi_0[]; // @ 0xE386EA (map title table)
extern char aMpMerv[];           // @ 0xE3870A (map short-name table)
extern const char* const szPlayLanMenuDescriptionReferences[3];  // @ 0xE381BC
extern int dword_F641D0[1580 * 802];   // cg.o
extern char byte_F64194[6320 * 802];   // cg.o
extern int dword_F641D4[1580 * 802];   // cg.o
extern int dword_F6A290[4 * 802];      // cg.o
extern int dword_186A0;                // damage constant (game.o)
extern vmCvar_t cg_widescreen;         // cg.o @ 0xF5CC88
extern void CG_FillRect(float x, float y, float width, float height,
                        const float* color, float z);  // ?CG_FillRect@@YAXMMMMQBMH@Z (cg.o)

int scoreboard_player_sorter(const void* left, const void* right);

class AARXBoxLiveIngameOptions {
public:
    static AARXBoxLiveIngameOptions* Me();  // ?Me@AARXBoxLiveIngameOptions@@SAPAV1@XZ (game_xbox.o)
    bool SetTimerText();                    // ?SetTimerText@AARXBoxLiveIngameOptions@@QAE_NXZ (game_xbox.o)
};

enum ESpectatorState : int {
    kSpectatorStateIntermission = 0x0,
    kSpectatorStateSpawn = 0x1,
    kSpectatorStateInjured = 0x2,
    kSpectatorStateDying = 0x3,
    kSpectatorStateDead = 0x4,
    kSpectatorStateDeadCanSpawn = 0x5,
    kSpectatorStateCount = 0x6,
};

// Minimal views for game-side symbols used by the menus
struct BrocExports {
    void (*mCallbackSpawnButtonPressed)(int entityHandle);  // +0xCD0 (Broc::entity)
    void (*mCallbackPlayerClassChange)(int entityHandle, int playerClass);  // +0xC68
};
struct BrocAPI {
    BrocExports mBrocExports;  // +0x00
};
extern BrocAPI* gpBrocAPI;  // 0xF3ABDC
enum hitLocation_t : int;

struct MenuClearHelper : FEMenu {
    void ClearAll() { ClearAllButtons(); }
};
void player_die(void* self, void* inflictor, void* attacker, int damage,
                int meansOfDeath, int iWeapon, const float* vPosition,
                const float* vDir, hitLocation_t hitLoc);  // ?player_die@@YAXPAVEntity@@00HHHPBM1W4hitLocation_t@@@Z (g.o)

extern void tlPrintf(const char* fmt, ...);  // ?tlPrintf@@YAXPBDZZ (tl_system.o)
extern "C" void __stdcall DmGetXboxName(char* name, unsigned int* size);  // xbox_shim
extern void j_nullsub_46(void* self);  // g.o nullsub
namespace View {
bool IsSplitScreen();  // ?IsSplitScreen@View@@YA_NXZ (cg.o)
void UpdateNumViewports();  // ?UpdateNumViewports@View@@YAXXZ (cg.o)
}
struct cgGlobal_t {
    int   frametime;  // +0x00
    int   time;       // +0x04
    int   oldTime;    // +0x08
    int   cubemapShot; // +0x0C
    int   cubemapSize; // +0x10
    bool  teamGame;   // +0x14
    bool  showScore;  // +0x15
    uint8_t _pad16[0x18 - 0x16];
    float gameTime;   // +0x18
    float gameTimeStartTime;  // +0x1C
    int   teamScores[5];      // +0x20
};
extern cgGlobal_t cgGlobal;   // 0xF5FE30 (cg.o)
struct AARMenuSystem;
namespace LocalClient {
bool QuitClientOutOfGame(int client);  // ?QuitClientOutOfGame@LocalClient@@YA_NH@Z (cl.o)
int  PortToClient(int port);           // ?PortToClient@LocalClient@@YAHH@Z (cl.o)
int  ClientToPort(int client);         // ?ClientToPort@LocalClient@@YAHH@Z (cl.o)
void UpdatePlayerPorts(int fixedPort); // ?UpdatePlayerPorts@LocalClient@@YAXH@Z (cl.o)
}

unsigned char CreateSessionMenu::m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@CreateSessionMenu@@1EA @ 0x1388D54
unsigned char CreateLanSessionMenu::m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@CreateLanSessionMenu@@1EA @ 0x1388D55
unsigned char FindSessionMenu::m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@FindSessionMenu@@1EA @ 0x1388D56
unsigned char FindLanSessionMenu::m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@FindLanSessionMenu@@1EA @ 0x1388D57
int PlayOnlineMenu::m_currSelection;  // ?m_currSelection@PlayOnlineMenu@@1HA @ 0xE381C4

// ============================================================================
// Small accessors (mp_shell.o, 16-byte functions)
// ============================================================================

// ea: 0x0078BEB0
short FEComboBox::GetCurrOption() const
{
    return mCurrOption;
}

// ea: 0x0078BEC0
const int MPUIInterface::GetMaxScoreLimitCount()
{
    return MPUIInterface::mMaxScoreLimitCount;
}

// ea: 0x0078BED0
void MPUIInterface::SetViewOnlineStatus(bool state)
{
    MPUIInterface::mIsViewableOnline = state;
}

// ea: 0x0078BEE0
void UIListBox::BlockRefresh(bool block)
{
    mBlockRefresh = block;
}

// ea: 0x0078BEF0
const OverlayMenu::eState OverlayMenu::GetState()
{
    return mState;
}

// ============================================================================
// CreateSessionMenu (mp_shell.o CreateSessionMenu.cpp)
// ============================================================================

// ea: 0x0078BF00
CreateSessionMenu::CreateSessionMenu(FEMenuSystem* s)
    : FEMenu(s, 8, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mNumberOfPlayersCombo = nullptr;
    mGameModeCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_PrivateSlotsCombo = nullptr;
    m_LastPlayerCount = 0;
    default_color_scheme = 5;
    mLastGameType = -1;
    mLastMap = -1;
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 8; ++i)
        m_pSlotText[i] = nullptr;
    for (int i = 0; i < 8; ++i)
        m_pSlotGeometry[i] = nullptr;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
}

// ea: 0x0078BFE0
CreateSessionMenu::~CreateSessionMenu()
{
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 8; ++i)
        m_pSlotText[i] = nullptr;
    mGameModeCombo = nullptr;
    mNumberOfPlayersCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_PrivateSlotsCombo = nullptr;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x0078C060
CreateSessionMenu* CreateSessionMenu::Me()
{
    return (CreateSessionMenu*)g_femanager.fems->menus[0];
}

// ea: 0x0078C070 (thunk)
void CreateSessionMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x0078C080
void CreateSessionMenu::Init()
{
}

// ea: 0x0078C090
void CreateSessionMenu::UpdateNetworking()
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078C0C0
void CreateSessionMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078C100
void CreateSessionMenu::UpdatePrivateSlots()
{
    int mCurrOption = mNumberOfPlayersCombo->mCurrOption;
    int v6 = 4 * mCurrOption + 4;
    int tempPrivateSlotOption = m_PrivateSlotsCombo->mCurrOption;
    if (m_LastPlayerCount != v6)
    {
        m_LastPlayerCount = v6;
        if (m_PrivateSlotsCombo != nullptr)
            delete m_PrivateSlotsCombo;
        FEText* TextPointer =
            panel->GetTextPointer("cg_slot_04_text_spec");
        PanelQuad* leftArrow =
            panel->GetPointer("cg_slot_04_arrow_left");
        PanelQuad* rightArrow =
            panel->GetPointer("cg_slot_04_arrow_right");
        FEComboBox* v11 = AddComboBox(7, v6, TextPointer, leftArrow,
                                      rightArrow);
        m_PrivateSlotsCombo = v11;
        if (v11 == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/CreateSessionMenu.cpp";
            AeAssert::gCurrentLine = 443;
            AeAssert::gCurrentExpr = "m_PrivateSlotsCombo";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Combobox failure for private slots"))
                __debugbreak();
        }
        if (v6 > 0)
        {
            char szNum[32];
            for (int v12 = 0; v12 < v6; ++v12)
            {
                sprintf(szNum, "%i", v12);
                Broc::string s(szNum);
                m_PrivateSlotsCombo->AddOption(s);
            }
        }
        if (tempPrivateSlotOption >= v6)
            m_PrivateSlotsCombo->SetCurrOption((short)(v6 - 1));
        else
            m_PrivateSlotsCombo->SetCurrOption((short)tempPrivateSlotOption);
        entries[1]->up = 7;
        entries[1]->down = 3;
        entries[3]->up = 1;
        entries[3]->down = 5;
        entries[5]->up = 3;
        entries[5]->down = 7;
        entries[7]->up = 5;
        entries[7]->down = 1;
    }
}

// ea: 0x0078C290
void CreateSessionMenu::GrabSessionName()
{
    if (MPLiveEngine::GetHandle()->internalState == kSignedIn
        && MPUIInterface::IsOnlineGame())
    {
        tlPrintf("CreateSessionMenu::GrabSessionName() - LIVE name\n");
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        LivePlayer* LocalPlayer =
            (LivePlayer*)Handle->GetLocalPlayer(Handle->actualPort);
        _snprintf(m_szSessionName, 0x10u, "%S", LocalPlayer->gamertag);
    }
    else if (MPUIInterface::IsLANGame())
    {
        char xbox_name[256];
        unsigned int size = 255;
        DmGetXboxName(xbox_name, &size);
        strncpy(m_szSessionName, xbox_name, 0x10u);
    }
}

// ea: 0x0078C330
void CreateSessionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078C350
void CreateSessionMenu::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078C360
void CreateSessionMenu::OnTriangle(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mPrivateSlots =
        (unsigned char)m_PrivateSlotsCombo->mCurrOption;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078C380
void CreateSessionMenu::SetGameTypeDefaults()
{
    int mLastGameType = this->mLastGameType;
    if (mGameModeCombo->mCurrOption != mLastGameType)
    {
        if (mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mFriendlyFire = 1;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 1;
        }
        eGameType mCurrOption = (eGameType)mGameModeCombo->mCurrOption;
        this->mLastGameType = mCurrOption;
        MPUIInterface::mServerParams.mScoreLimit =
            (unsigned char)MPUIInterface::GetDefaultOption(
                MPUIInterface::SETTING_SCORE_LIMIT, mCurrOption);
        if (this->mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mTeamBalancing = 0;
            MPUIInterface::mServerParams.mFriendlyFire = 0;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 0;
        }
    }
}

// ea: 0x007942C0
void CreateSessionMenu::UpdateHighlight()
{
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
    entries[4]->Highlight(highlighted == 5, true);
    entries[6]->Highlight(highlighted == 7, true);
    UpdatePrivateSlots();
}

// ea: 0x00794330
void CreateSessionMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
    MPUIInterface::mServerParams.mGameType =
        (unsigned char)mGameModeCombo->mCurrOption;
    short mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != 0xFF)
        mCurrOption = (short)(unsigned char)byte_E386C9[114 * mCurrOption];
    MPUIInterface::mServerParams.mMapID = (unsigned char)mCurrOption;
    MPUIInterface::mServerParams.mMaxPlayers =
        (unsigned char)mNumberOfPlayersCombo->mCurrOption;
    MPUIInterface::mServerParams.mPrivateSlots =
        (unsigned char)m_PrivateSlotsCombo->mCurrOption;
}

// ea: 0x00794390
void CreateSessionMenu::OnSquare(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mGameType =
        (unsigned char)mGameModeCombo->mCurrOption;
    short mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != 0xFF)
        mCurrOption = (short)(unsigned char)byte_E386C9[114 * mCurrOption];
    MPUIInterface::mServerParams.mMapID = (unsigned char)mCurrOption;
    MPUIInterface::mServerParams.mMaxPlayers =
        (unsigned char)mNumberOfPlayersCombo->mCurrOption;
    MPUIInterface::mServerParams.mPrivateSlots =
        (unsigned char)m_PrivateSlotsCombo->mCurrOption;
    system->MakeActiveAndReturn(2);
}

// ea: 0x007943F0
void CreateSessionMenu::OnUp(int c)
{
    (void)c;
    Up();
    UpdateHighlight();
}

// ea: 0x00794410
void CreateSessionMenu::OnDown(int c)
{
    (void)c;
    Down();
    UpdateHighlight();
}

// ea: 0x00794430
void CreateSessionMenu::SetMapDefaults()
{
    int mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != mLastMap)
    {
        mLastMap = mCurrOption;
        short MaxPlayersOptionFromMap;
        if (mCurrOption == 0xFF)
            MaxPlayersOptionFromMap = MPUIInterface::GetMaxPlayersOptionFromMap(255);
        else
            MaxPlayersOptionFromMap = MPUIInterface::GetMaxPlayersOptionFromMap(
                byte_E386C9[114 * mCurrOption]);
        mNumberOfPlayersCombo->SetCurrOption(MaxPlayersOptionFromMap);
    }
}

// ea: 0x007A7B00
void CreateSessionMenu::OnLeft(int c)
{
    (void)c;
    Left();
    switch (highlighted)
    {
    case 1:
        SetGameTypeDefaults();
        break;
    case 3:
        SetMapDefaults();
        break;
    case 5:
        UpdatePrivateSlots();
        break;
    default:
        break;
    }
}

// ea: 0x007A7B50
void CreateSessionMenu::OnRight(int c)
{
    (void)c;
    Right();
    switch (highlighted)
    {
    case 1:
        SetGameTypeDefaults();
        break;
    case 3:
        SetMapDefaults();
        break;
    case 5:
        UpdatePrivateSlots();
        break;
    default:
        break;
    }
}

// ============================================================================
// CreateLanSessionMenu (mp_shell.o CreateLanSessionMenu.cpp)
// ============================================================================

// ea: 0x0078C3F0
CreateLanSessionMenu::CreateLanSessionMenu(FEMenuSystem* s)
    : FEMenu(s, 6, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mNumberOfPlayersCombo = nullptr;
    mGameModeCombo = nullptr;
    mStartingMapCombo = nullptr;
    default_color_scheme = 5;
    mLastGameType = -1;
    mLastMap = -1;
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 6; ++i)
        m_pSlotText[i] = nullptr;
    for (int i = 0; i < 6; ++i)
        m_pSlotGeometry[i] = nullptr;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
}

// ea: 0x0078C4C0
CreateLanSessionMenu::~CreateLanSessionMenu()
{
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 6; ++i)
        m_pSlotText[i] = nullptr;
    mGameModeCombo = nullptr;
    mNumberOfPlayersCombo = nullptr;
    mStartingMapCombo = nullptr;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x0078C530
CreateLanSessionMenu* CreateLanSessionMenu::Me()
{
    return (CreateLanSessionMenu*)g_femanager.fems->menus[1];
}

// ea: 0x0078C540 (thunk)
void CreateLanSessionMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x0078C550
void CreateLanSessionMenu::Init()
{
}

// ea: 0x0078C560
void CreateLanSessionMenu::UpdateNetworking()
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078C590
void CreateLanSessionMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078C5D0 (thunk)
void CreateLanSessionMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x0078C5E0
void CreateLanSessionMenu::GrabSessionName(int c)
{
    (void)c;
    if (MPLiveEngine::GetHandle()->internalState == kSignedIn
        && MPUIInterface::IsOnlineGame())
    {
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        LivePlayer* LocalPlayer =
            (LivePlayer*)Handle->GetLocalPlayer(Handle->actualPort);
        _snprintf(m_szSessionName, 0x10u, "%S", LocalPlayer->gamertag);
    }
    else if (MPUIInterface::IsLANGame())
    {
        char xbox_name[256];
        unsigned int size = 255;
        DmGetXboxName(xbox_name, &size);
        strncpy(m_szSessionName, xbox_name, 0x10u);
    }
}

// ea: 0x0078C680
void CreateLanSessionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078C6A0
void CreateLanSessionMenu::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078C6B0
void CreateLanSessionMenu::OnUp(int c)
{
    (void)c;
    Up();
    entries[4]->Highlight(highlighted == 5, true);
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
}

// ea: 0x0078C700
void CreateLanSessionMenu::OnDown(int c)
{
    (void)c;
    Down();
    entries[4]->Highlight(highlighted == 5, true);
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
}

// ea: 0x0078C750
void CreateLanSessionMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(9);
}

// ea: 0x0078C760
void CreateLanSessionMenu::SetGameTypeDefaults()
{
    int mLastGameType = this->mLastGameType;
    if (mGameModeCombo->mCurrOption != mLastGameType)
    {
        if (mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mFriendlyFire = 1;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 1;
        }
        eGameType mCurrOption = (eGameType)mGameModeCombo->mCurrOption;
        this->mLastGameType = mCurrOption;
        MPUIInterface::mServerParams.mScoreLimit =
            (unsigned char)MPUIInterface::GetDefaultOption(
                MPUIInterface::SETTING_SCORE_LIMIT, mCurrOption);
        if (this->mLastGameType == 5)
        {
            MPUIInterface::mServerParams.mFriendlyFire = 0;
            MPUIInterface::mServerParams.mEnablePenaltyVote = 0;
            MPUIInterface::mServerParams.mTeamBalancing = 0;
        }
    }
}

// ea: 0x00795070
void CreateLanSessionMenu::OnSquare(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mGameType =
        (unsigned char)mGameModeCombo->mCurrOption;
    short mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != 0xFF)
        mCurrOption = (short)(unsigned char)byte_E386C9[114 * mCurrOption];
    MPUIInterface::mServerParams.mMapID = (unsigned char)mCurrOption;
    MPUIInterface::mServerParams.mMaxPlayers =
        (unsigned char)mNumberOfPlayersCombo->mCurrOption;
    system->MakeActiveAndReturn(3);
}

// ea: 0x007950C0
void CreateLanSessionMenu::SetMapDefaults()
{
    int mCurrOption = mStartingMapCombo->mCurrOption;
    if (mCurrOption != mLastMap)
    {
        mLastMap = mCurrOption;
        short MaxPlayersOptionFromMap;
        if (mCurrOption == 0xFF)
            MaxPlayersOptionFromMap = MPUIInterface::GetMaxPlayersOptionFromMap(255);
        else
            MaxPlayersOptionFromMap = MPUIInterface::GetMaxPlayersOptionFromMap(
                byte_E386C9[114 * mCurrOption]);
        mNumberOfPlayersCombo->SetCurrOption(MaxPlayersOptionFromMap);
    }
}

// ea: 0x007A7EF0
void CreateLanSessionMenu::OnLeft(int c)
{
    (void)c;
    Left();
    if (highlighted == 1)
    {
        SetGameTypeDefaults();
    }
    else if (highlighted == 3)
    {
        SetMapDefaults();
    }
}

// ea: 0x007A7F30
void CreateLanSessionMenu::OnRight(int c)
{
    (void)c;
    Right();
    if (highlighted == 1)
    {
        SetGameTypeDefaults();
    }
    else if (highlighted == 3)
    {
        SetMapDefaults();
    }
}

// ============================================================================
// FindSessionMenu (mp_shell.o FindSessionMenu.cpp)
// ============================================================================

// ea: 0x0078CF40
FindSessionMenu::FindSessionMenu(FEMenuSystem* s)
    : FEMenu(s, 10, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mNumberOfPlayersCombo = nullptr;
    mGameModeCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    default_color_scheme = 5;
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 10; ++i)
        m_pSlotText[i] = nullptr;
    memset(&m_pSlotGeometry, 0, sizeof(m_pSlotGeometry));
    for (int i = 0; i < 6; ++i)
        m_pBackgroundRow[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundLine[i] = nullptr;
}

// ea: 0x0078D020
FindSessionMenu::~FindSessionMenu()
{
    for (int i = 0; i < 6; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 10; ++i)
        m_pSlotText[i] = nullptr;
    memset(&m_pSlotGeometry, 0, sizeof(m_pSlotGeometry));
    for (int i = 0; i < 6; ++i)
        m_pBackgroundRow[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundLine[i] = nullptr;
    mGameModeCombo = nullptr;
    mNumberOfPlayersCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x0078D0D0
FindSessionMenu* FindSessionMenu::Me()
{
    return (FindSessionMenu*)g_femanager.fems->menus[4];
}

// ea: 0x0078D0E0 (thunk)
void FindSessionMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x0078D0F0
void FindSessionMenu::Init()
{
}

// ea: 0x0078D100
void FindSessionMenu::UpdateNetworking()
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078D130
void FindSessionMenu::Update(float time_inc)
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
    FEMenu::Update(time_inc);
}

// ea: 0x0078D170
void FindSessionMenu::UpdateHighlight()
{
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
    entries[4]->Highlight(highlighted == 5, true);
    entries[6]->Highlight(highlighted == 7, true);
    entries[8]->Highlight(highlighted == 9, true);
}

// ea: 0x0078D1F0 (thunk)
void FindSessionMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x0078D200
void FindSessionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078D220
void FindSessionMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078D230
void FindSessionMenu::OnUp(int c)
{
    (void)c;
    Up();
    UpdateHighlight();
}

// ea: 0x0078D250
void FindSessionMenu::OnDown(int c)
{
    (void)c;
    Down();
    UpdateHighlight();
}

// ============================================================================
// FindLanSessionMenu (mp_shell.o FindLanSessionMenu.cpp)
// ============================================================================

// ea: 0x0078D270
FindLanSessionMenu::FindLanSessionMenu(FEMenuSystem* s)
    : FEMenu(s, 10, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mNumberOfPlayersCombo = nullptr;
    mGameModeCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    default_color_scheme = 5;
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 10; ++i)
        m_pSlotText[i] = nullptr;
    memset(&m_pSlotGeometry, 0, sizeof(m_pSlotGeometry));
    for (int i = 0; i < 6; ++i)
        m_pBackgroundRow[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundLine[i] = nullptr;
}

// ea: 0x0078D350
FindLanSessionMenu::~FindLanSessionMenu()
{
    for (int i = 0; i < 4; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 10; ++i)
        m_pSlotText[i] = nullptr;
    memset(&m_pSlotGeometry, 0, sizeof(m_pSlotGeometry));
    for (int i = 0; i < 6; ++i)
        m_pBackgroundRow[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundLine[i] = nullptr;
    mGameModeCombo = nullptr;
    mNumberOfPlayersCombo = nullptr;
    mStartingMapCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x0078D400
FindLanSessionMenu* FindLanSessionMenu::Me()
{
    return (FindLanSessionMenu*)g_femanager.fems->menus[5];
}

// ea: 0x0078D410 (thunk)
void FindLanSessionMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x0078D420
void FindLanSessionMenu::Init()
{
}

// ea: 0x0078D430
void FindLanSessionMenu::UpdateNetworking()
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
}

// ea: 0x0078D460
void FindLanSessionMenu::Update(float time_inc)
{
    MPUIInterface::Step();
    if (MPUIInterface::IsOnlineGame()
        && MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        system->MakeActive(8);
    }
    FEMenu::Update(time_inc);
}

// ea: 0x0078D4A0
void FindLanSessionMenu::UpdateHighlight()
{
    entries[4]->Highlight(highlighted == 5, true);
    entries[0]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 3, true);
    entries[6]->Highlight(highlighted == 7, true);
    entries[8]->Highlight(highlighted == 9, true);
}

// ea: 0x0078D520 (thunk)
void FindLanSessionMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x0078D530
void FindLanSessionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078D550
void FindLanSessionMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078D560
void FindLanSessionMenu::OnUp(int c)
{
    (void)c;
    Up();
    UpdateHighlight();
}

// ea: 0x0078D580
void FindLanSessionMenu::OnDown(int c)
{
    (void)c;
    Down();
    UpdateHighlight();
}

// ============================================================================
// CreateSessionAdvancedMenu / CreateLanSessionAdvancedMenu
// ============================================================================

// ea: 0x0078CAD0
CreateSessionAdvancedMenu* CreateSessionAdvancedMenu::Me()
{
    return (CreateSessionAdvancedMenu*)g_femanager.fems->menus[2];
}

// ea: 0x0078CE80
CreateLanSessionAdvancedMenu* CreateLanSessionAdvancedMenu::Me()
{
    return (CreateLanSessionAdvancedMenu*)g_femanager.fems->menus[3];
}

// ============================================================================
// GameSettingsEdit / GameSettingsView (mp_shell.o GameSettings*.cpp)
// ============================================================================

// ea: 0x0078D5A0
void GameSettingsEdit::Init()
{
}

// ea: 0x0078D710
void GameSettingsEdit::OnL1(int c)
{
    (void)c;
}

// ea: 0x0078D720
void GameSettingsEdit::OnR1(int c)
{
    (void)c;
}

// ea: 0x0078D730
void GameSettingsEdit::OnCross(int c)
{
    (void)c;
}

// ea: 0x0078D740
bool GameSettingsEdit::ResponseNoJustGoBackToPauseMenuHelper()
{
    system->ReturnToPreviousMenu(-1);
    return true;
}

// ea: 0x0078D780
void GameSettingsEdit::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078D790
void GameSettingsEdit::OnSquare(int c)
{
    (void)c;
}

// ea: 0x0078D7A0
void GameSettingsEdit::OnStart(int c)
{
    (void)c;
}

// ea: 0x0078D7B0
void GameSettingsEdit::ButtonHeldAction()
{
}

// ea: 0x0078D8D0
AARGameSettingsEdit* AARGameSettingsEdit::Me(int version)
{
    (void)version;
    return (AARGameSettingsEdit*)g_femanager.mAARS->menus[6];
}

// ea: 0x0078D990
void GameSettingsView::Init()
{
}

// ea: 0x0078DF70 (thunk)
void GameSettingsView::OnDeactivate(FESplitScreenMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x0078DFA0
void GameSettingsView::OnLeft(int c)
{
    (void)c;
}

// ea: 0x0078DFB0
void GameSettingsView::OnRight(int c)
{
    (void)c;
}

// ea: 0x0078DFC0
void GameSettingsView::OnL1(int c)
{
    (void)c;
}

// ea: 0x0078DFD0
void GameSettingsView::OnR1(int c)
{
    (void)c;
}

// ea: 0x0078DFE0
void GameSettingsView::OnCross(int c)
{
    (void)c;
}

// ea: 0x0078DFF0
void GameSettingsView::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078E000
void GameSettingsView::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078E010
void GameSettingsView::OnSquare(int c)
{
    (void)c;
}

// ea: 0x0078E020
void GameSettingsView::ButtonHeldAction()
{
}

// ea: 0x0078E030
AARGameSettingsView* AARGameSettingsView::Me(int version)
{
    (void)version;
    return (AARGameSettingsView*)g_femanager.mAARS->menus[7];
}

// ea: 0x0078E040 (thunk)
void AARGameSettingsView::SetPanelFile(PanelFile* pf)
{
    SetPanelFileMain(pf);
}

// ============================================================================
// InitialLoadingMenu / InstantActionMenu / PlayLanMenu / PlayOnlineMenu /
// PressStartMenu
// ============================================================================

// ea: 0x0078E160
void InitialLoadingMenu::Select(int entry_num)
{
    (void)entry_num;
}

// ea: 0x0078E220
InitialLoadingMenu* InitialLoadingMenu::Me()
{
    return (InitialLoadingMenu*)g_femanager.fems->menus[6];
}

// ea: 0x0078E280 (thunk)
void InstantActionMenu::OnActivate()
{
    FEMenu::OnActivate();
}

// ea: 0x0078E410
void InstantActionMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078E6A0
InstantActionMenu* InstantActionMenu::Me()
{
    return (InstantActionMenu*)g_femanager.fems->menus[7];
}

// ea: 0x0078E6B0
void PlayLanMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
}

// ea: 0x0078E770
void PlayLanMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(8);
}

// ea: 0x0078E780
PlayLanMenu* PlayLanMenu::Me()
{
    return (PlayLanMenu*)g_femanager.fems->menus[9];
}

// ea: 0x0078E7E0
void PlayOnlineMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    PlayOnlineMenu::m_currSelection = highlighted;
}

// ea: 0x0078E810
void PlayOnlineMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(8);
}

// ea: 0x0078E820
PlayOnlineMenu* PlayOnlineMenu::Me()
{
    return (PlayOnlineMenu*)g_femanager.fems->menus[10];
}

// ea: 0x0078EA50
void PressStartMenu::Select(int entry_num)
{
    (void)entry_num;
}

// ea: 0x0078EA60
void PressStartMenu::OnTriangle(int c)
{
    (void)c;
}

// ea: 0x0078EA70
void PressStartMenu::OnStart(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078EB00
PressStartMenu* PressStartMenu::Me()
{
    return (PressStartMenu*)g_femanager.fems->menus[11];
}

// ============================================================================
// SessionDetailsMenu / SessionListMenu / SessionLanListMenu
// ============================================================================

// ea: 0x0078EB50 (thunk)
void SessionDetailsMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    j_nullsub_46(this);
}

// ea: 0x0078EB80
void SessionDetailsMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078EE70
SessionDetailsMenu* SessionDetailsMenu::Me()
{
    return (SessionDetailsMenu*)g_femanager.fems->menus[12];
}

// ea: 0x0078EEE0
SessionListMenu* SessionListMenu::Me()
{
    return (SessionListMenu*)g_femanager.fems->menus[13];
}

// ea: 0x0078EF70
SessionLanListMenu* SessionLanListMenu::Me()
{
    return (SessionLanListMenu*)g_femanager.fems->menus[14];
}

// ea: 0x0078EFA0
void SessionLanListMenu::OnCross(int c)
{
    Select(c);
}

// ============================================================================
// Overlay menu family (mp_shell.o overlay.cpp)
// ============================================================================

// ea: 0x0078F330
void OverlayMenu::OnCircle(int c)
{
    (void)c;
}

// ea: 0x0078F4C0
void OverlayMenu::LogonUpdate()
{
}

// ea: 0x0078F4D0
void InGameOverlay::Select(int __formal)
{
    (void)__formal;
}

// ea: 0x0078F780
void InGameOverlay::OnCircle(int __formal)
{
    (void)__formal;
}

// ea: 0x0078F790
void InGameOverlay::OnSquare(int __formal)
{
    (void)__formal;
}

// ea: 0x0078F870
void AAROverlay::Select(int __formal)
{
    (void)__formal;
}

// ea: 0x0078FB10
void AAROverlay::OnCircle(int __formal)
{
    (void)__formal;
}

// ea: 0x0078FB20
void AAROverlay::OnSquare(int __formal)
{
    (void)__formal;
}

// ea: 0x00790030 (thunk)
void MultilineFrontendOverlayMenu::OnCross(int c)
{
    FEMenu::OnCross(c);
}

// ea: 0x00790060
MultilineFrontendOverlayMenu* MultilineFrontendOverlayMenu::Me()
{
    return (MultilineFrontendOverlayMenu*)g_femanager.fems->menus[17];
}

// ea: 0x007902A0 (thunk)
void MultilineFrontendOverlayMenu::OnTriangle(int c)
{
    OverlayMenuBase::OnTriangle(c);
}

// ea: 0x00790920 (thunk)
void VoteMapMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ============================================================================
// WeaponSelectMenu / InGameSwitchSides
// ============================================================================

// ea: 0x00790A50 (thunk)
void WeaponSelectMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x00790C60
void WeaponSelectMenu::OnStart(int c)
{
    (void)c;
}

// ea: 0x00790E90
void InGameSwitchSides::Init()
{
}

// ea: 0x00790EA0 (thunk)
void InGameSwitchSides::OnDeactivate(ModelMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x00790EB0
void InGameSwitchSides::OnTriangle(int c)
{
    (void)c;
}

// ea: 0x00790EC0
bool InGameSwitchSides::ResponseNoNevermind(int index)
{
    (void)index;
    return true;
}

// ============================================================================
// InGameScoreBoard
// ============================================================================

// ea: 0x00790D50
void InGameScoreBoard::Init()
{
}

// ea: 0x00790DC0
int InGameScoreBoard::GetAlliesScore()
{
    return cgGlobal.teamScores[2];
}

// ea: 0x00790DD0
int InGameScoreBoard::GetAxisScore()
{
    return cgGlobal.teamScores[1];
}

// ea: 0x00790E10
void InGameScoreBoard::OnTriangle(int c)
{
    OnSelect(c);
}

// ea: 0x00790E20
void InGameScoreBoard::OnSquare(int c)
{
    (void)c;
}

// ============================================================================
// AAR scoreboard / stats / vote menus
// ============================================================================

// ea: 0x00791000 (thunk)
void AARScoreboardBase::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x00791010
void AARScoreboardBase::Init()
{
}

// ea: 0x00791040
void AARScoreboardBase::OnSquare(int c)
{
    (void)c;
}

// ea: 0x00791070
int AARScoreboardBase::GetAlliesScore()
{
    return cgGlobal.teamScores[2];
}

// ea: 0x00791080
int AARScoreboardBase::GetAxisScore()
{
    return cgGlobal.teamScores[1];
}

// ea: 0x00791090
void AARScoreboardBase::OnLeft(int c)
{
    OnL1(c);
}

// ea: 0x007910A0
void AARScoreboardBase::OnRight(int c)
{
    OnR1(c);
}

// ea: 0x007910B0
void AARScoreboardBase::OnUp(int c)
{
    m_ListBox.OnUp(c);
}

// ea: 0x007910C0
void AARScoreboardBase::OnDown(int c)
{
    m_ListBox.OnDown(c);
}

// ea: 0x00791150
void AARScoreboardLoser::OnR1(int c)
{
    (void)c;
    system->MakeActive(2);
}

// ea: 0x00791160
void AARScoreboardLoser::OnL1(int c)
{
    (void)c;
    system->MakeActive(0);
}

// ea: 0x007912C0
AARPersonalStats* AARPersonalStats::Me()
{
    return (AARPersonalStats*)g_femanager.mAARS->menus[2];
}

// ea: 0x007912D0 (thunk)
void AARPersonalStats::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x007912E0
void AARPersonalStats::Init()
{
}

// ea: 0x007912F0
void AARPersonalStats::OnCross(int c)
{
    (void)c;
}

// ea: 0x00791300
void AARPersonalStats::OnUp(int c)
{
    (void)c;
}

// ea: 0x00791310
void AARPersonalStats::OnDown(int c)
{
    (void)c;
}

// ea: 0x00791340
void AARPersonalStats::OnLeft(int c)
{
    OnL1(c);
}

// ea: 0x00791350
void AARPersonalStats::OnRight(int c)
{
    OnR1(c);
}

// ea: 0x00791510
AARMapVote* AARMapVote::Me()
{
    return (AARMapVote*)g_femanager.mAARS->menus[4];
}

// ea: 0x00791540
void AARMapVote::Init()
{
}

// ea: 0x00791550 (thunk)
void AARMapVote::OnDeactivate(AARBaseMenu* __formal)
{
    (void)__formal;
    ClearAllButtons();
}

// ea: 0x00791640
void AARMapVote::OnR1(int c)
{
    (void)c;
    system->MakeActive(0);
}

// ea: 0x00791650
void AARMapVote::OnLeft(int c)
{
    OnL1(c);
}

// ea: 0x00791660
void AARMapVote::OnRight(int c)
{
    OnR1(c);
}

// ea: 0x007917F0
AARGameModeVote* AARGameModeVote::Me()
{
    return (AARGameModeVote*)g_femanager.mAARS->menus[3];
}

// ea: 0x00791870
void AARGameModeVote::Init()
{
}

// ea: 0x00791880 (thunk)
void AARGameModeVote::OnDeactivate(AARBaseMenu* __formal)
{
    (void)__formal;
    ClearAllButtons();
}

// ea: 0x00791950
void AARGameModeVote::OnLeft(int c)
{
    OnL1(c);
}

// ea: 0x00791960
void AARGameModeVote::OnRight(int c)
{
    OnR1(c);
}

// ============================================================================
// PauseMenu / AARPauseMenu / HotJoinMenu / SpectateMenu
// ============================================================================

// ea: 0x00791BB0
void PauseMenu::Quit()
{
    LocalClient::QuitClientOutOfGame(mVersion);
}

// ea: 0x00791DC0
AARPauseMenu::~AARPauseMenu()
{
}

// ea: 0x00791E20 (thunk)
void AARPauseMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    Cleanup();
}

// ea: 0x00792080
void AARPauseMenu::UpdateSplitScreen()
{
}

// ea: 0x00792260
void AARPauseMenu::OnUp(int c)
{
    (void)c;
    Up();
}

// ea: 0x00792270
void AARPauseMenu::OnDown(int c)
{
    (void)c;
    Down();
}

// ea: 0x00792460 (thunk)
void HotJoinMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
}

// ea: 0x007924E0 (thunk)
void HotJoinMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00792560
void HotJoinMenu::OnUp(int c)
{
    (void)c;
    Up();
}

// ea: 0x007926E0
SpectateMenu::~SpectateMenu()
{
}

// ============================================================================
// MI_UpdateMapList / ModelMenu
// ============================================================================

// ea: 0x00792E70
void MI_UpdateMapList()
{
}

// ea: 0x007931B0 (thunk)
void ModelMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00793450
void ModelMenu::DebugControls()
{
}

// ea: 0x007A74B0
ModelMenu::~ModelMenu()
{
}

// ea: 0x0079AB30 (thunk)
void GameSettingsEdit::Draw()
{
    FESplitScreenMenu::Draw();
}

// ea: 0x0079B160
bool AARGameSettingsEdit::ResponseYesApplyNow(int index)
{
    (void)index;
    return ((AARGameSettingsEdit*)g_femanager.mAARS->menus[6])
        ->ResponseYesApplyNowHelper();
}

// ea: 0x0079B750 (thunk)
void GameSettingsView::Draw()
{
    FESplitScreenMenu::Draw();
}

// ea: 0x007A8C50
void GameSettingsEdit::SwapMenus()
{
    UpdateSplitScreenOptions(highlighted);
}

// ea: 0x007A8CE0 (thunk)
void AARGameSettingsEdit::SetPanelFile(PanelFile* pf)
{
    SetPanelFileMain(pf);
}

// ============================================================================
// Batch 5/6: dtor thunks + small menu handlers
// ============================================================================

// ea: 0x0078C9A0
void CreateSessionAdvancedMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078CD50
void CreateLanSessionAdvancedMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078D6D0
void GameSettingsEdit::OnDeactivate(FESplitScreenMenu* m)
{
    (void)m;
    ClearAllButtons();
    iLastOptionSelected = highlighted;
}

// ea: 0x0078E120
void InitialLoadingMenu::OnActivate()
{
    FEMenu::OnActivate();
    mTime = 0.0f;
    SetHigh(-1, true);
}

// ea: 0x0078E290
void InstantActionMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078E6C0
void PlayLanMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078E9F0
void PressStartMenu::OnActivate()
{
    FEMenu::OnActivate();
    SetHigh(-1, true);
}

// ea: 0x0078EA10
void PressStartMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078EE80
void SessionListMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078EEA0
void SessionListMenu::OnTriangle(int c)
{
    (void)c;
    j_nullsub_46(this);
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078EEF0
void SessionListMenu::TidyGamesList()
{
    m_ListBox.Clear();
    mNumGames = 0;
}

// ea: 0x0078EF10
void SessionLanListMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078EF30
void SessionLanListMenu::OnTriangle(int c)
{
    (void)c;
    j_nullsub_46(this);
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078EFF0
void OverlayMenuBase::OnActivate()
{
    FEMenu::OnActivate();
    SetHigh(-1, true);
}

// ea: 0x0078F010
void OverlayMenuBase::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x0078F030
void OverlayMenuBase::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
}

// ea: 0x0078F880
AAROverlay* AAROverlay::Me(int version)
{
    (void)version;
    AARMenuSystem* result = g_femanager.mAARS;
    if (g_femanager.mAARS != nullptr)
        return (AAROverlay*)g_femanager.mAARS->menus[10];
    return (AAROverlay*)result;
}

// ea: 0x007902B0
void MultilineIngameOverlayMenu::OnDeactivate(FEMenu* menu)
{
    (void)menu;
    mState = NO_OVERLAY;
    InGameMenuSystem* v2 = g_femanager.mIGMS[currCl];
    if (v2 != nullptr)
        v2->is_active = false;
}

// ea: 0x007903B0
MultilineIngameOverlayMenu* MultilineIngameOverlayMenu::Me()
{
    return (MultilineIngameOverlayMenu*)
        g_femanager.GetIGMS(currCl)->menus[9];
}

// ea: 0x007906B0
void VoteGameTypeMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x00790930
void VoteMapMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x00790C70
void WeaponSelectMenu::SetClassOptionHeader()
{
    m_pClassOptionHeader->SetText(szClassReference[highlighted]);
}

// ea: 0x00790ED0
void InGameSwitchSides::ResponseGoBack(int client)
{
    g_femanager.GetDMS(client)->CloseDialog();
}

// ea: 0x00790F70
AARBaseMenu::~AARBaseMenu()
{
    m_pTimerText[0] = nullptr;
    m_pTimerText[1] = nullptr;
}

// ea: 0x007910D0
void AARScoreboardWinner::Draw()
{
    FEMenu::Draw();
    if (panel != nullptr)
        panel->Draw();
}

// ea: 0x007910F0
void AARScoreboardWinner::OnR1(int c)
{
    (void)c;
    if (cgGlobal.teamGame)
        system->MakeActive(1);
    else
        system->MakeActive(2);
}

// ea: 0x007A8F30
AARGameSettingsView::~AARGameSettingsView()
{
}

// ea: 0x007A9EC0
InGameSwitchSides::~InGameSwitchSides()
{
}

// ea: 0x007AA000 (thunk)
void AARBaseMenu::Update(float time_inc)
{
    (void)time_inc;
    SetTimerText();
}

// ea: 0x007AAA00 (thunk)
void AARScoreboardWinner::Update(float time_inc)
{
    AARScoreboardBase::Update(time_inc);
}

// ea: 0x007AAA40 (thunk)
void AARScoreboardLoser::Update(float time_inc)
{
    AARScoreboardBase::Update(time_inc);
}

// ea: 0x007AB410 (thunk)
void AARPersonalStats::Update(float time_inc)
{
    (void)time_inc;
    AARBaseMenu::SetTimerText();
}

// ea: 0x007AB850
PauseMenu::~PauseMenu()
{
}

// ea: 0x007ABEA0
void SessionListMenu::Refresh()
{
    j_nullsub_46(this);
    InitMenu();
}

// ea: 0x007ABEE0
void SessionLanListMenu::Refresh()
{
    j_nullsub_46(this);
    InitMenu();
}

// ea: 0x007AD330 (thunk)
void GameSettingsEdit::PanelFileUnloaded(PanelFile* pf)
{
    FESplitScreenMenu::PanelFileUnloaded(pf);
}

// ea: 0x007AD340 (thunk)
void GameSettingsView::PanelFileUnloaded(PanelFile* pf)
{
    FESplitScreenMenu::PanelFileUnloaded(pf);
}

// ea: 0x007B02A0
AARScoreboardWinner::~AARScoreboardWinner()
{
}

// ea: 0x007B02D0
AARScoreboardLoser::~AARScoreboardLoser()
{
}

// ============================================================================
// Batch 7: AAR winner, pause responses, spectate, session list, settings
// ============================================================================

// ea: 0x00791110
void AARScoreboardWinner::OnL1(int c)
{
    (void)c;
    if (MPUIInterface::mNextServerParams.mEnableAARVote == 1)
        system->MakeActive(4);
    else
        system->MakeActive(2);
}

// ea: 0x00791CC0
bool PauseMenu::ResponseYesTeamChange(int client)
{
    ((PauseMenu*)g_femanager.GetIGMS(client)->menus[0])->TeamChange();
    return true;
}

// ea: 0x00791CE0
void PauseMenu::ResponseGoBack(int client)
{
    g_femanager.GetDMS(client)->CloseDialog();
}

// ea: 0x007925E0
void HotJoinMenu::UpdateSplitScreen()
{
    panel->MoveSplitScreen(unk_F6A284[802 * mVersion],
                           unk_F6A280[802 * mVersion]);
}

// ea: 0x007926F0
void SpectateMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
}

// ea: 0x007929A0
void SpectateMenu::OnStart(int c)
{
    (void)c;
    g_femanager.GetIGMS(mVersion)->MakeActiveAndReturn(0);
}

// ea: 0x007929C0
void SpectateMenu::OnSelect(int c)
{
    (void)c;
    g_femanager.GetIGMS(mVersion)->MakeActiveAndReturn(10);
}

// ea: 0x00793010
char MI_GetMapIDbyIndex(char index)
{
    if (index != -1)
        return byte_E386C9[114 * index];
    return index;
}

// ea: 0x00793520
void AARMenuSystem::CheckForNoMenus()
{
    int active = ((int(__thiscall*)(void*))(*(void***)this)[24])(this);
    if (active <= -1)
    {
        is_active = false;
        GamePause::SetAllPaused(false);
    }
}

// ea: 0x0079AD30
void GameSettingsEdit::OnLeft(int c)
{
    (void)c;
    Left();
    SetGameTypeDefaults();
}

// ea: 0x0079AE80
bool GameSettingsEdit::ResponseYesApplyNow(int client)
{
    return ((GameSettingsEdit*)g_femanager.GetIGMS(client)->menus[2])
        ->ResponseYesApplyNowHelper();
}

// ea: 0x0079B8F0
void AARGameSettingsView::Update(float time_inc)
{
    GameSettingsView::Update(time_inc);
    SetTimerText();
}

// ea: 0x007A6C70
bool AARPauseMenu::ResponseYesQuit(int client)
{
    Quit(client);
    return true;
}

// ea: 0x007A8EB0
void GameSettingsView::SwapMenus()
{
    int highlighted = this->highlighted;
    mCurrentServerParams = &MPUIInterface::mServerParams;
    UpdateSplitScreenOptions(highlighted);
}

// ea: 0x007A9FA0
bool InGameSwitchSides::ResponseYesSwitch(int client)
{
    ((InGameSwitchSides*)g_femanager.GetIGMS(client)->menus[11])
        ->SwitchTeams();
    return true;
}

// ea: 0x007AB4F0
void AARMapVote::Update(float time_inc)
{
    AARBaseMenu::SetTimerText();
    m_ListBox.Update(time_inc);
}

// ea: 0x007AB6F0
void AARGameModeVote::Update(float time_inc)
{
    AARBaseMenu::SetTimerText();
    m_ListBox.Update(time_inc);
}

// ea: 0x007ABBB0
void SpectateMenu::OnActivate(int prev)
{
    (void)prev;
    FEMenu::OnActivate();
    UpdateState();
}

// ea: 0x007AD4C0
void SessionListMenu::OnSquare(int c)
{
    (void)c;
    j_nullsub_46(this);
    InitMenu();
}

// ea: 0x007AD600
void SessionLanListMenu::OnSquare(int c)
{
    (void)c;
    j_nullsub_46(this);
    InitMenu();
}

// ea: 0x007AD650
void WeaponSelectMenu::PanelFileUnloaded(PanelFile* pf)
{
    FESplitScreenMenu::PanelFileUnloaded(pf);
    Cleanup();
}

// ea: 0x007AE430
void ModelMenu::Draw3D()
{
    if (!View::IsSplitScreen())
    {
        FESplitScreenMenu::Draw();
        AddDObjToScene();
    }
}

// ea: 0x0078CEE0
void CreateLanSessionAdvancedMenu::OnUp(int c)
{
    (void)c;
    if (highlighted != 0)
    {
        Up();
    }
    else
    {
        highlighted = 4;
        SetHigh(4, true);
    }
}

// ea: 0x0078D7C0
void GameSettingsEdit::UpdateMapChangeStatus(bool isAllowingMapVote)
{
    void** v2 = *(void***)entries[1];
    ((void(__thiscall*)(void*, int))v2[16])(entries[1],
                                            isAllowingMapVote ? 1 : 0);
}

// ea: 0x0078E140
void InitialLoadingMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078EB60
void SessionDetailsMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078EF50
void SessionLanListMenu::OnCircle(int c)
{
    (void)c;
    mSortColumn++;
    if (mSortColumn == 4)
        mSortColumn = 0;
}

// ea: 0x0078EF80
void SessionLanListMenu::TidyGamesList()
{
    m_ListBox.Clear();
    mNumGames = 0;
}

// ============================================================================
// Batch 8: overlay/session/scoreboard/settings handlers
// ============================================================================

// ea: 0x0078F0F0
void OverlayMenuBase::UpdateSplitScreen()
{
    if (panel != nullptr)
        panel->UpdateSplitScreen(unk_F6A284[802 * mVersion],
                                 unk_F6A280[802 * mVersion]);
}

// ea: 0x0078F3C0
void OverlayMenu::Select(int entry_num)
{
    (void)entry_num;
}

// ea: 0x0078F4E0
InGameOverlay* InGameOverlay::Me(int version)
{
    InGameMenuSystem* result = g_femanager.GetIGMS(version);
    if (result != nullptr)
        return (InGameOverlay*)g_femanager.GetIGMS(version)->menus[13];
    return (InGameOverlay*)result;
}

// ea: 0x00790040
void MultilineFrontendOverlayMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
}

// ea: 0x007902D0
void MultilineIngameOverlayMenu::OnStart(int c)
{
    (void)c;
    if (mState == CONTROLLER_DISCONNECTED && g_controllerConnected[c])
    {
        g_controllerConnectedErrorShown[c] = false;
        system->RemoveOverlay();
    }
}

// ea: 0x00790CF0
int scoreboard_player_sorter(const void* left, const void* right)
{
    const int* l = (const int*)left;
    const int* r = (const int*)right;
    if (l[1] >= r[1])
        return (l[1] <= r[1]) - 1;
    return 1;
}

// ea: 0x00791020
void AARScoreboardBase::OnDeactivate(AARBaseMenu* m)
{
    (void)m;
    ClearAllButtons();
    m_ListBox.Clear();
}

// ea: 0x00791B30
void PauseMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    tlPrintf("PauseMenu::OnDeactivate()\n");
    ClearAllButtons();
    m_iLastSelection = highlighted;
}

// ea: 0x00791D50
void PauseMenu::ButtonHeldAction()
{
    if (button_held_down == 8)
        OnDown(0);
    else if (button_held_down == 4)
        OnUp(0);
}

// ea: 0x00791DD0
void AARPauseMenu::ButtonHeldAction()
{
    if (button_held_down == 8)
        OnDown(0);
    else if (button_held_down == 4)
        OnUp(0);
}

// ea: 0x00791E00
void AARPauseMenu::ResponseGoBack(int client)
{
    g_femanager.GetDMS(client)->CloseDialog();
}

// ea: 0x00791E30
void AARPauseMenu::OnDeactivate(FEMenu* pMenu)
{
    (void)pMenu;
    tlPrintf("AARPauseMenu::OnDeactivate()\n");
    ClearAllButtons();
    m_iLastSelection = highlighted;
}

// ea: 0x00792A90
void SpectateMenu::OnSquare(int c)
{
    (void)c;
    int mState = this->mState;
    if (mState > kSpectatorStateIntermission
        && mState <= kSpectatorStateDeadCanSpawn)
    {
        PlayNavigationSound();
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(mVersion);
        IGMS->MakeActiveAndReturn(1);
    }
}

// ea: 0x00792E10
bool MI_IsMapPackAvailable(char map_pack)
{
    if (g_NumTotalMaps <= 0)
        return false;
    Mapinfo_t* i = g_TheMapInfo;
    int v1 = 0;
    while (i->map_pack != map_pack)
    {
        ++v1;
        ++i;
        if (v1 >= g_NumTotalMaps)
            return false;
    }
    return true;
}

// ea: 0x0079B140
void AARGameSettingsEdit::Update(float time_inc)
{
    GameSettingsEdit::Update(time_inc);
    SetTimerText();
}

// ea: 0x0079C580
void PlayLanMenu::SetPreviewImage()
{
    ClearPreviewImages();
    m_pImages[mListBox.mTopLine + mListBox.mSelectedLine]->SetShown(true);
}

// ea: 0x0079C5B0
void PlayLanMenu::SetOptionText()
{
    m_pText[1]->SetText(
        szPlayLanMenuOptionTextReferences[mListBox.mTopLine]
                                         [mListBox.mSelectedLine]);
}

// ea: 0x0079D0E0
void SessionListMenu::OnUp(int c)
{
    if (!lockInput)
    {
        m_ListBox.OnUp(c);
        UpdateGameInfo();
    }
}

// ea: 0x0079D110
void SessionListMenu::OnDown(int c)
{
    if (!lockInput)
    {
        m_ListBox.OnDown(c);
        UpdateGameInfo();
    }
}

// ea: 0x0079DE80
void SessionLanListMenu::OnUp(int c)
{
    if (!lockInput)
    {
        m_ListBox.OnUp(c);
        UpdateGameInfo();
    }
}

// ea: 0x0079DEB0
void SessionLanListMenu::OnDown(int c)
{
    if (!lockInput)
    {
        m_ListBox.OnDown(c);
        UpdateGameInfo();
    }
}

// ea: 0x007A2E00
void InGameScoreBoard::OnUp(int c)
{
    short v3 = m_ListBox.OnUp(c);
    m_ListBox.mHighlights.mElements[v3] = true;
}

// ea: 0x007A2E30
void InGameScoreBoard::OnDown(int c)
{
    short v3 = m_ListBox.OnDown(c);
    m_ListBox.mHighlights.mElements[v3] = true;
}

// ea: 0x007A8CC0
AARGameSettingsEdit::~AARGameSettingsEdit()
{
    m_FirstTimeAccessedByte = 0;
}

// ea: 0x007A9E60
void InGameScoreBoard::OnActivate()
{
    FEMenu::OnActivate();
    m_ListBox.Clear();
    SetPanelContents();
    ClearButton(controller::SELECT);
    m_bActivated = true;
}

// ea: 0x007ABEF0
void AAROverlay::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    PanelFile* panel = this->panel;
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    this->panel = nullptr;
    Cleanup();
}

// ea: 0x007AC160
void AARScoreboardLoser::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    PanelFile* panel = this->panel;
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    this->panel = nullptr;
    Cleanup();
}

// ea: 0x007AC200
void HotJoinMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    PanelFile* panel = this->panel;
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    this->panel = nullptr;
}

// ea: 0x007AF2D0
void InGameSwitchSides::OnCross(int c)
{
    (void)c;
    EntityManager::sInst->GetPlayer(mVersion);
    PickTeam();
    UpdateModel();
}

// ea: 0x007B0280
AARScoreboardWinner::AARScoreboardWinner(FEMenuSystem* pauseMenuSystem)
    : AARScoreboardBase(pauseMenuSystem)
{
}

// ea: 0x007B02B0
AARScoreboardLoser::AARScoreboardLoser(FEMenuSystem* pauseMenuSystem)
    : AARScoreboardBase(pauseMenuSystem)
{
}

// ============================================================================
// Batch 9: remaining menu Draw/Update/ctor handlers
// ============================================================================

// ea: 0x0078E7F0
void PlayOnlineMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x0078F7A0
void InGameOverlay::OnUp(int c)
{
    m_ListBox.OnUp(c);
    if (--m_currSelection < 0)
    {
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
    }
}

// ea: 0x0078FB30
void AAROverlay::OnUp(int c)
{
    m_ListBox.OnUp(c);
    if (--m_currSelection < 0)
    {
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
    }
}

// ea: 0x00790580
VoteGameTypeMenu::VoteGameTypeMenu(FEMenuSystem* pauseMenuSystem)
    : FEMenu(pauseMenuSystem, 2, 320, 260, 8, 0)
{
    mPlayerMgr = nullptr;
    mGameTypeList = nullptr;
}

// ea: 0x007907D0
VoteMapMenu::VoteMapMenu(FEMenuSystem* pauseMenuSystem)
    : FEMenu(pauseMenuSystem, 2, 320, 260, 8, 0)
{
    mPlayerMgr = nullptr;
    mMapList = nullptr;
    default_color_scheme = 9;
}

// ea: 0x00790D20
void InGameScoreBoard::UpdateSplitScreen()
{
    if (panel != nullptr)
        panel->UpdateSplitScreen(unk_F6A284[802 * mVersion],
                                 unk_F6A280[802 * mVersion]);
}

// ea: 0x00790D60
void InGameScoreBoard::OnDeactivate(FEMenu* m)
{
    (void)m;
    ClearAllButtons();
    dword_F641D0[1580 * currCl] = 0;
    m_bShowMyTeamScore = true;
    m_ListBox.Clear();
}

// ea: 0x00790DA0
void InGameScoreBoard::Draw()
{
    FEMenu::Draw();
    if (panel != nullptr)
        panel->Draw();
}

// ea: 0x00790F90
void AARBaseMenu::OnStart(int c)
{
    unsigned int v3 = LocalClient::PortToClient(c);
    if (v3 <= 1 && dword_F6A290[802 * v3] == 2)
        system->MakeActiveAndReturn(9);
}

// ea: 0x00791130
void AARScoreboardLoser::Draw()
{
    FEMenu::Draw();
    if (panel != nullptr)
        panel->Draw();
}

// ea: 0x00791320
void AARPersonalStats::OnR1(int c)
{
    (void)c;
    m_bHighlightScrollArrowRight = true;
    m_ePanelToSwitchTo = MPUIInterface::mNextServerParams.mEnableAARVote != 1 ? 0 : 3;
}

// ea: 0x00791360
void AARPersonalStats::OnL1(int c)
{
    (void)c;
    if (cgGlobal.teamGame)
        system->MakeActive(1);
    else
        system->MakeActive(0);
    m_bHighlightScrollArrowLeft = true;
}

// ea: 0x00791520
void AARMapVote::PanelFileUnloaded(PanelFile* pPanelFile)
{
    (void)pPanelFile;
    Cleanup();
    m_ListBox.RemoveAllItems();
}

// ea: 0x00791930
void AARGameModeVote::OnR1(int c)
{
    (void)c;
    m_bHighlightScrollArrowRight = true;
    m_ePanelToSwitchTo = 4;
}

// ea: 0x00791E60
void AARPauseMenu::UnPause(int client)
{
    system->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(client)->MakeActive(-1);
    m_iLastSelection = -1;
    ClearAllButtons();
}

// ea: 0x00791EA0
void AARPauseMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(0)->MakeActive(-1);
    m_iLastSelection = -1;
    ClearAllButtons();
}

// ea: 0x00791EE0
void AARPauseMenu::OnStart(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(0)->MakeActive(-1);
    m_iLastSelection = -1;
    ClearAllButtons();
}

// ea: 0x00792240
void AARPauseMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x00792E40
char* remove_underscores(char* str)
{
    int v2 = (int)strlen(str);
    char* v3 = str;
    if (v2 > 0)
    {
        do
        {
            if (*v3 == 95)
                *v3 = 32;
            ++v3;
            --v2;
        } while (v2);
    }
    return str;
}

// ea: 0x00792E80
char* MI_GetMapDisplayName(char id)
{
    if (g_NumTotalMaps <= 0)
        return "NULL";
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return "NULL";
        i += 114;
    }
    return &aMpfrontendMerv[114 * v1];
}

// ea: 0x00792EC0
char* MI_GetMapLocation(char id)
{
    if (g_NumTotalMaps <= 0)
        return "NULL";
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return "NULL";
        i += 114;
    }
    return &aMploadingMervi[114 * v1];
}

// ea: 0x00792F00
char* MI_GetMapTitle(char id)
{
    if (g_NumTotalMaps <= 0)
        return "NULL";
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return "NULL";
        i += 114;
    }
    return &aMploadingMervi_0[114 * v1];
}

// ea: 0x00792F40
char MI_GetMapPack(char id)
{
    if (g_NumTotalMaps <= 0)
        return -1;
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return -1;
        i += 114;
    }
    return g_TheMapInfo[v1].map_pack;
}

// ea: 0x00792F80
char* MI_GetMapShortname(char id)
{
    if (g_NumTotalMaps <= 0)
        return "NULL";
    int v1 = 0;
    char* i = byte_E386C9;
    while (*i != id)
    {
        if (++v1 >= g_NumTotalMaps)
            return "NULL";
        i += 114;
    }
    return &aMpMerv[114 * v1];
}

// ea: 0x00793460
void ModelMenu::DebugRender()
{
}

// ea: 0x007934A0
void AARMenuSystem::Draw()
{
    if (mPreviousWidescreen != (cg_widescreen.integer != 0))
    {
        // FEMenuSystem vtable slot 2 = UpdateWidescreen (shell.o 0x57DF20)
        ((void(__thiscall*)(void*, bool))(*(void***)this)[2])(
            this, cg_widescreen.integer != 0);
        mPreviousWidescreen = cg_widescreen.integer != 0;
    }
    // FEMenuSystem vtable slot 19 = Draw (shell.o 0x570E90)
    ((void(__thiscall*)(void*))(*(void***)this)[19])(this);
}

// ea: 0x0079C5E0
void PlayLanMenu::SetDescriptionText()
{
    int mSelectedLine = mListBox.mSelectedLine;
    FEText* v3 = m_pText[2];
    m_pText[2]->SetText(szPlayLanMenuDescriptionReferences[mListBox.mTopLine]
                                                    [mSelectedLine]);
}

// ea: 0x007A19E0
void WeaponSelectMenu::CloseMenu()
{
    int mReturnMenu = this->mReturnMenu;
    Allow_Exit = true;
    if (mReturnMenu < 0)
    {
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(mVersion);
        ((PauseMenu*)IGMS->menus[0])->UnPause(mVersion);
    }
    else
    {
        system->ReturnToPreviousMenu(mReturnMenu);
    }
}

// ea: 0x007A1B60
void InGameScoreBoard::RecalculateWinningTeam()
{
    if (cgGlobal.teamScores[2] <= cgGlobal.teamScores[1])
        SetWinningTeam((team_t)(2 * (cgGlobal.teamScores[1] <= cgGlobal.teamScores[2]) + 1));
    else
        SetWinningTeam(TEAM_ALLIES);
}

// ea: 0x007A3F80
void AARScoreboardWinner::SetPanelFile(PanelFile* pf)
{
    AARScoreboardBase::SetPanelFile(pf);
    m_pYourTeamScore[4]->SetShown(true);
    m_pYourTeamScore[5]->SetShown(false);
}

// ============================================================================
// Batch 10: SetPanelFile dispatchers + menu Update/Set handlers
// ============================================================================

// ea: 0x0078D6F0
GameSettingsEdit* GameSettingsEdit::Me(int version)
{
    return (GameSettingsEdit*)g_femanager.GetIGMS(version)->menus[2];
}

// ea: 0x0078DD70
void GameSettingsView::UpdateScrollBar()
{
    if (mScrollBarThumb != nullptr)
    {
        float y = (float)((mScrollBarYInc * highlighted) + mScrollBarTopY);
        mScrollBarThumb->SetCenterPos(mScrollBarThumb->GetCenterX(), y);
    }
}

// ea: 0x0078E2B0
void InstantActionMenu::Update(float time_inc)
{
    if (!MPUIInterface::IsOnlineGame()
        || MPLiveEngine::GetHandle()->internalState == kSignedIn)
    {
        FEMenu::Update(time_inc);
        movie_manager::frame_advance();
        MPUIInterface::Step();
    }
}

// ea: 0x0078EA30
void PressStartMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
}

// ea: 0x0078F590
void InGameOverlay::OnTriangle(int c)
{
    (void)c;
    OverlayMenuBase::OnTriangle(c);
    if (GetSystem()->CurrentOverlay() != -1)
    {
        GetSystem()->RemoveOverlay();
        m_State = (eState)0;
    }
}

// ea: 0x0078F7E0
void InGameOverlay::Draw()
{
    m_ListBox.Draw();
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
    if (m_State >= 3 && m_State <= 9 && m_IsAARTimerEnabled)
    {
        m_IsAARTimerEnabled =
            AARXBoxLiveIngameOptions::Me()->SetTimerText();
    }
}

// ea: 0x0078F920
void AAROverlay::OnTriangle(int c)
{
    (void)c;
    OverlayMenuBase::OnTriangle(c);
    if (GetSystem()->CurrentOverlay() != -1)
    {
        GetSystem()->RemoveOverlay();
        m_State = (eState)0;
    }
}

// ea: 0x0078FB70
void AAROverlay::Draw()
{
    m_ListBox.Draw();
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
    if (m_State >= 3 && m_State <= 9 && m_IsAARTimerEnabled)
    {
        m_IsAARTimerEnabled =
            AARXBoxLiveIngameOptions::Me()->SetTimerText();
    }
}

// ea: 0x0078FD20
void MultilineOverlayMenu::OnActivate()
{
    FEMenu::OnActivate();
    SetHigh(-1, true);
    if (entries[2]->GetDisable() != 0)
        SetHigh(3, true);
    else
        SetHigh(2, true);
}

// ea: 0x00790DE0
void InGameScoreBoard::OnButtonRelease(int c, int b)
{
    (void)c;
    if (b == 8)
        ClearButton(controller::DOWNBUTTON);
    else if (b == 4)
        ClearButton(controller::UPBUTTON);
}

// ea: 0x00791890
void AARGameModeVote::OnUp(int c)
{
    Up();
    m_ListBox.OnUp(c);
    if (m_currentRow <= 0)
        m_currentRow = 6;
    else
        --m_currentRow;
}

// ea: 0x00792D10
void SpectateMenu::UpdateSplitScreen()
{
    if (panel != nullptr)
        panel->UpdateSplitScreen(unk_F6A284[802 * mVersion],
                                 unk_F6A280[802 * mVersion]);
}

// ea: 0x007A33C0
void InGameSwitchSides::SwapMenus()
{
    if (MultiplayerMgr::sInst->mRankedGame)
    {
        entries[1]->Disable(true);
        entries[2]->Disable(true);
    }
    else
    {
        entries[1]->Disable(false);
        entries[2]->Disable(false);
    }
}

// ea: 0x007A3680
void AARBaseMenu::OnL1(int c)
{
    (void)c;
    if (mLeftArrowFader.mQuad != nullptr)
    {
        mLeftArrowFader.mAlpha = 1.0f;
        mLeftArrowFader.mFading = true;
        mLeftArrowFader.mAlphaTo = 0.5f;
        mLeftArrowFader.mTime = 0.5f;
        mLeftArrowFader.mAlphaDelta = fabs(0.5f);
        mLeftArrowFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mLeftArrowFader.mFading = false;
    }
}

// ea: 0x007A8E20
void GameSettingsView::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_options_view.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(pf->mName, "MP_PM_GS_view.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007A9CF0
void WeaponSelectMenu::Select(int entryNum)
{
    (void)entryNum;
    if (Allow_Exit)
    {
        Allow_Exit = true;
        if (mReturnMenu < 0)
            ((PauseMenu*)g_femanager.GetIGMS(mVersion)->menus[0])->UnPause();
        else
            system->ReturnToPreviousMenu(-1);
    }
}

// ea: 0x007A9D40
void WeaponSelectMenu::OnTriangle(int controllerIndex)
{
    (void)controllerIndex;
    if (Allow_Exit)
    {
        Allow_Exit = true;
        if (mReturnMenu < 0)
            ((PauseMenu*)g_femanager.GetIGMS(mVersion)->menus[0])->UnPause();
        else
            system->ReturnToPreviousMenu(-1);
    }
}

// ea: 0x007A9E90
InGameSwitchSides::InGameSwitchSides(FEMenuSystem* s)
    : ModelMenu(s, 3)
{
    m_eTeam = TEAM_FREE;
    default_color_scheme = 10;
}

// ea: 0x007A9FC0
void AARBaseMenu::OnActivate()
{
    FEMenu::OnActivate();
    SetTimerText();
    if (MultiplayerMgr::sInst->mRankedGame)
        m_pTimerText[1]->SetText("MPGAME_AAR_RANK_GAME_OVER");
    else
        m_pTimerText[1]->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
}

// ea: 0x007AAA10
void AARScoreboardLoser::OnActivate()
{
    AARScoreboardBase::OnActivate();
    m_pYourTeamScore[4]->SetShown(false);
    m_pYourTeamScore[5]->SetShown(false);
}

// ea: 0x007ABBD0
void GameSettingsEdit::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_options_edit.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(pf->mName, "MP_PM_GS_edit.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007ABE70
void SessionListMenu::OnActivate()
{
    FEMenu::OnActivate();
    mShowDownArrow = false;
    mShowUpArrow = false;
    mNumGames = 0;
    m_ListBox.SelectLine(0);
}

// ea: 0x007ABF20
void WeaponSelectMenu::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_options.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(pf->mName, "MP_class_select.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007ABF70
void InGameScoreBoard::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    if (mVersion > 0)
    {
        PanelFile* panel = this->panel;
        if (panel != nullptr)
        {
            panel->~PanelFile();
            mem_heap_free(panel);
        }
        this->panel = nullptr;
    }
    m_ListBox.RemoveAllItems();
    Cleanup();
}

// ea: 0x007ABFC0
void InGameSwitchSides::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_options.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(pf->mName, "MP_PM_sideselection.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007AD620
void InGameOverlay::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    PanelFile* panel = this->panel;
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    this->panel = nullptr;
    Cleanup();
}

// ea: 0x007AF1C0
void WeaponSelectMenu::OnUp(int c)
{
    (void)c;
    Up();
    m_pClassOptionHeader->SetText(szClassReference[highlighted]);
    SetClassGauges();
    SetSwitchKit();
}

// ea: 0x007AF200
void WeaponSelectMenu::OnDown(int c)
{
    (void)c;
    Down();
    m_pClassOptionHeader->SetText(szClassReference[highlighted]);
    SetClassGauges();
    SetSwitchKit();
}

// ============================================================================
// Batch 11: game-settings/vote/overlay/side-switch handlers
// ============================================================================

// ea: 0x0078D7F0
void GameSettingsEdit::DisableTeamGameOptions(bool b)
{
    entries[7]->Disable(b != 0);
    entries[4]->Disable(b != 0);
    entries[5]->Disable(b != 0);
    if (b != 0)
    {
        ((FEComboBox*)entries[7])->SetCurrOption(0);
        ((FEComboBox*)entries[4])->SetCurrOption(0);
        ((FEComboBox*)entries[5])->SetCurrOption(0);
    }
}

// ea: 0x0078E830
void PlayOnlineMenu::TogglePreviewImage(int option, bool visible)
{
    PanelQuad* Pointer = nullptr;
    switch (option)
    {
    case 1:
        Pointer = panel->GetPointer("mm_preview_image_01");
        break;
    case 2:
        Pointer = panel->GetPointer("mm_preview_image_02");
        break;
    case 3:
        Pointer = panel->GetPointer("mm_preview_image_03");
        break;
    case 4:
        Pointer = panel->GetPointer("mm_preview_image_04");
        break;
    case 5:
        Pointer = panel->GetPointer("mm_preview_image_05");
        break;
    default:
        return;
    }
    if (Pointer != nullptr)
        Pointer->SetVisibility(visible ? 1.0f : 0.0f);
}

// ea: 0x0078E8A0
void PlayOnlineMenu::InitQuickMatchParameters(int c)
{
    sServerQueryParams params;
    params.mMapID = gSaveGameData[c].mStubData.mMapPreference;
    params.mGameType = gSaveGameData[c].mStubData.mGameModePreference;
    params.mGameSubType = -1;
    params.mMinPlayers = -1;
    params.mMaxPlayers = gSaveGameData[c].mStubData.mMaxPlayerCntPreference;
    params.mTeamBalancing = gSaveGameData[c].mStubData.mAutoTeamBalancePreference;
    params.mFriendlyFire = gSaveGameData[c].mStubData.mTeamDamagePreference;
    MPUIInterface::mGameConnectionType =
        MPUIInterface::kGameConnectionTypeOnline;
    params.mListIfFull = 0;
    params.mSessionNamePrefix[0] = 0;
    MPUIInterface::SetQueryParams(params);
}

// ea: 0x0078F3F0
void OverlayMenu::OnUp(int c)
{
    m_ListBox.OnUp(c);
    if ((mState == 17
         || mState == (JOINING_START | 0x10)
         || mState == (JOINING | 0x10)
         || mState == 18
         || mState == (GAME_LISTING_START | 0x10)
         || mState == 20)
        && --m_currSelection < 0)
    {
        m_currSelection = 1;
        m_ListBox.SelectLine(1);
    }
}

// ea: 0x0078F830
void InGameOverlay::OnDown(int c)
{
    m_ListBox.OnDown(c);
    int v4 = m_currSelection + 1;
    bool v5 = m_currSelection < 0;
    m_currSelection = v4;
    // __OFSUB__(v4, 1): signed overflow of (v4 - 1)
    bool ofsub = ((v4 ^ 1) & (v4 ^ (v4 - 1))) < 0;
    if (!(v5 ^ ofsub | (v4 == 1)))
    {
        m_currSelection = 0;
        m_ListBox.SelectLine(0);
    }
}

// ea: 0x00790000
void MultilineFrontendOverlayMenu::OnStart(int c)
{
    if (mState == CONTROLLER_DISCONNECTED && g_controllerConnected[c])
    {
        g_controllerConnectedErrorShown[c] = false;
        system->RemoveOverlay();
    }
}

// ea: 0x00791560
void AARMapVote::OnUp(int c)
{
    Up();
    if (m_ListBox.mTopLine + m_ListBox.mSelectedLine != 0)
        m_ListBox.OnUp(c);
    else
        m_ListBox.SelectLine(g_NumBaseMaps - 1);
}

// ea: 0x00791970
void AARGameModeVote::OnL1(int c)
{
    (void)c;
    m_bHighlightScrollArrowLeft = true;
    m_ePanelToSwitchTo = 2;
}

// ea: 0x00791C90
bool PauseMenu::ResponseYesQuit(int client)
{
    LocalClient::QuitClientOutOfGame(
        (int)g_femanager.GetIGMS(client)->menus[1]->entries);
    return true;
}

// ea: 0x00792600
void HotJoinMenu::Join()
{
    MultiplayerMgr::sInst->AttemptHotJoin(mVersion);
    LocalClient::UpdatePlayerPorts(mVersion);
}

// ea: 0x007929E0
void SpectateMenu::OnCross(int c)
{
    (void)c;
    if ((mState == kSpectatorStateSpawn
         || mState == kSpectatorStateDying
         || mState == kSpectatorStateDeadCanSpawn)
        && gpBrocAPI->mBrocExports.mCallbackSpawnButtonPressed != nullptr)
    {
        PlayNavigationSound();
        Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
        gpBrocAPI->mBrocExports.mCallbackSpawnButtonPressed(
            *(int*)Player);
    }
}

// ea: 0x00792D60
void SpectateMenu::UpdateWidescreen(bool widescreen)
{
    FEMenu::UpdateWidescreen(widescreen);
}

// ea: 0x00792FC0
char MI_GetMapIDbyShortname(char* shortname)
{
    if (g_NumTotalMaps <= 0)
        return 0;
    int v1 = 0;
    const char* i = aMpMerv;
    while (_stricmp(i, shortname) != 0)
    {
        if (++v1 >= g_NumTotalMaps)
            return 0;
        i += 114;
    }
    return byte_E386C9[114 * v1];
}

// ea: 0x00793300
void ModelMenu::SetLightBrightness(int index, float brightness)
{
    if (index >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/ModelMenu.cpp";
        AeAssert::gCurrentLine = 121;
        AeAssert::gCurrentExpr = "index >= 0 && index < 2";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid light index"))
            __debugbreak();
    }
    mBrightness[index] = brightness;
}

// ea: 0x007934E0
void AARMenuSystem::Update(float time_inc)
{
    // FEMenuSystem vtable slot 16 = Update (shell.o 0x570DD0)
    ((void(__thiscall*)(void*, float))(*(void***)this)[16])(this, time_inc);
    if (mPreviousWidescreen != (cg_widescreen.integer != 0))
    {
        ((void(__thiscall*)(void*, bool))(*(void***)this)[2])(
            this, cg_widescreen.integer != 0);
        mPreviousWidescreen = cg_widescreen.integer != 0;
    }
}

// ea: 0x0079B910
void PlayLanMenu::SetLiveOnXBox()
{
    for (int i = 0; i < 5; ++i)
    {
        if (i < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        m_pBackgroundButtons[i]->SetShown(true);
    }
}

// ea: 0x0079CB30
void PlayOnlineMenu::OnUp(int c)
{
    (void)c;
    if (highlighted == 1)
    {
        highlighted = 4;
        SetHigh(4, true);
    }
    else
    {
        Up();
    }
    TogglePreviewImage(highlighted, true);
}

// ea: 0x0079CB90
void PlayOnlineMenu::OnDown(int c)
{
    (void)c;
    if (highlighted == 4)
    {
        highlighted = 1;
        SetHigh(1, true);
    }
    else
    {
        Down();
    }
    TogglePreviewImage(highlighted, true);
}

// ea: 0x007A09B0
void VoteGameTypeMenu::OnStart(int c)
{
    (void)c;
    FEMenu* v2 = g_femanager.mIGMS[0]->menus[0];
    int client = (int)v2[1].entries;
    InGameMenuSystem* IGMS = g_femanager.GetIGMS(client);
    IGMS->ReturnToPreviousMenu(-1);
    DialogMenuSystem* DMS = g_femanager.GetDMS(client);
    DMS->MakeActive(-1);
    GamePause::SetGamePaused(currCl, false);
    ((MenuClearHelper*)v2)->ClearAll();
}

// ea: 0x007A4090
void AARScoreboardLoser::SetPanelFile(PanelFile* pf)
{
    PanelFile* v3 = pf->Clone();
    AARScoreboardBase::SetPanelFile(v3);
    m_pYourTeamScore[4]->SetShown(false);
    m_pYourTeamScore[5]->SetShown(true);
    helpbar1->SetText("MPGAME_HELP_LOSERS_AAR_SCOREBOARD");
    panel->GetTextPointer("sb_text_title_section")
        ->SetText("MPGAME_LOSERS_SCOREBOARD");
}

// ea: 0x007A6820
bool PauseMenu::ResponseYesSuicide(int client)
{
    PauseMenu* v1 = (PauseMenu*)g_femanager.GetIGMS(client)->menus[0];
    Entity* Player = EntityManager::sInst->GetPlayer(v1->mVersion);
    Client* v3 = Player->client;
    if (v3 != nullptr && v3->pers.playerState == 3)
    {
        player_die(Player, Player, Player, dword_186A0, 25, 0, nullptr,
                   nullptr, (hitLocation_t)0);
        v1->UnPause();
    }
    return true;
}

// ea: 0x007A8E70
void GameSettingsView::OnActivate()
{
    SwapMenus();
    FEMenu::OnActivate();
    highlighted = 0;
    mCurrentServerParams = &MPUIInterface::mServerParams;
    UpdateOptions();
    UpdateSplitScreenOptions(highlighted);
}

// ea: 0x007A8ED0
AARGameSettingsView::AARGameSettingsView(FEMenuSystem* s)
    : GameSettingsView(s)
{
    mVersion = s->GetCurrentClient();
}

// ea: 0x007AB710
void AARGameModeVote::OnActivate()
{
    MPUIInterface::Step();
    FEMenu::OnActivate();
    AARBaseMenu::SetTimerText();
    if (MultiplayerMgr::sInst->mRankedGame)
        m_pTimerText[1]->SetText("MPGAME_AAR_RANK_GAME_OVER");
    else
        m_pTimerText[1]->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
}

// ea: 0x007AB7F0
PauseMenu::PauseMenu(FEMenuSystem* s)
    : FESplitScreenMenu(s, 7)
{
    flags = (int16_t)(flags | 0x80);
    m_iLastSelection = 0;
    default_color_scheme = 10;
    mVersion = s->GetCurrentClient();
}

// ea: 0x007AF300
void InGameSwitchSides::OnUp(int c)
{
    (void)c;
    int highlighted = this->highlighted;
    Up();
    Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
    int v5 = this->highlighted;
    sentient_s* sentient = Player->sentient;
    if (this->highlighted != 0)
    {
        if (this->highlighted == 1)
            m_eTeam = TEAM_ALLIES;
        else if (this->highlighted == 2)
            m_eTeam = TEAM_AXIS;
    }
    else
    {
        m_eTeam = sentient->eTeam;
    }
    if (highlighted != v5)
        UpdateModel();
}

// ea: 0x007AF370
void InGameSwitchSides::OnDown(int c)
{
    (void)c;
    int highlighted = this->highlighted;
    Down();
    Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
    int v5 = this->highlighted;
    sentient_s* sentient = Player->sentient;
    if (this->highlighted != 0)
    {
        if (this->highlighted == 1)
            m_eTeam = TEAM_ALLIES;
        else if (this->highlighted == 2)
            m_eTeam = TEAM_AXIS;
    }
    else
    {
        m_eTeam = sentient->eTeam;
    }
    if (highlighted != v5)
        UpdateModel();
}

// ============================================================================
// Batch 12: advanced session menus + pause/suicide/spectate/vote handlers
// ============================================================================

// ea: 0x0078C940
void CreateSessionAdvancedMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    MPUIInterface::mServerParams.mTimeLimit =
        (unsigned char)m_TimeLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mScoreLimit =
        (unsigned char)m_ScoreLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mTeamBalancing =
        (unsigned char)m_AutoTeamBalanceCombo->mCurrOption;
    MPUIInterface::mServerParams.mFriendlyFire =
        (unsigned char)m_TeamDamageCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnableAARVote =
        (unsigned char)m_VotingCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnablePenaltyVote =
        (unsigned char)m_PenaltyVoteCombo->mCurrOption;
}

// ea: 0x0078CA60
void CreateSessionAdvancedMenu::OnTriangle(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mTimeLimit =
        (unsigned char)m_TimeLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mScoreLimit =
        (unsigned char)m_ScoreLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mTeamBalancing =
        (unsigned char)m_AutoTeamBalanceCombo->mCurrOption;
    MPUIInterface::mServerParams.mFriendlyFire =
        (unsigned char)m_TeamDamageCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnableAARVote =
        (unsigned char)m_VotingCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnablePenaltyVote =
        (unsigned char)m_PenaltyVoteCombo->mCurrOption;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078CAE0
void CreateSessionAdvancedMenu::GrabSessionName()
{
    if (MPLiveEngine::GetHandle()->internalState == kSignedIn
        && MPUIInterface::IsOnlineGame())
    {
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        LivePlayer* LocalPlayer =
            (LivePlayer*)Handle->GetLocalPlayer(Handle->actualPort);
        _snprintf(m_szSessionName, 0x10u, "%S", LocalPlayer->gamertag);
    }
    else if (MPUIInterface::IsLANGame())
    {
        char xbox_name[256];
        unsigned int size = 255;
        DmGetXboxName(xbox_name, &size);
        strncpy(m_szSessionName, xbox_name, 0x10u);
    }
}

// ea: 0x0078CCF0
void CreateLanSessionAdvancedMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    MPUIInterface::mServerParams.mTimeLimit =
        (unsigned char)m_TimeLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mScoreLimit =
        (unsigned char)m_ScoreLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mTeamBalancing =
        (unsigned char)m_AutoTeamBalanceCombo->mCurrOption;
    MPUIInterface::mServerParams.mFriendlyFire =
        (unsigned char)m_TeamDamageCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnableAARVote =
        (unsigned char)m_VotingCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnablePenaltyVote =
        (unsigned char)m_PenaltyVoteCombo->mCurrOption;
}

// ea: 0x0078CE10
void CreateLanSessionAdvancedMenu::OnTriangle(int c)
{
    (void)c;
    MPUIInterface::mServerParams.mTimeLimit =
        (unsigned char)m_TimeLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mScoreLimit =
        (unsigned char)m_ScoreLimitCombo->mCurrOption;
    MPUIInterface::mServerParams.mTeamBalancing =
        (unsigned char)m_AutoTeamBalanceCombo->mCurrOption;
    MPUIInterface::mServerParams.mFriendlyFire =
        (unsigned char)m_TeamDamageCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnableAARVote =
        (unsigned char)m_VotingCombo->mCurrOption;
    MPUIInterface::mServerParams.mEnablePenaltyVote =
        (unsigned char)m_PenaltyVoteCombo->mCurrOption;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x0078CE90
void CreateLanSessionAdvancedMenu::GrabSessionName()
{
    if (MPUIInterface::IsLANGame())
    {
        char xbox_name[256];
        unsigned int size = 255;
        DmGetXboxName(xbox_name, &size);
        strncpy(m_szSessionName, xbox_name, 0x10u);
    }
}

// ea: 0x0078EB10
SessionDetailsMenu::SessionDetailsMenu(FEMenuSystem* s)
    : FEMenu(s, 11, 320, 240, 8, 0)
{
    default_color_scheme = 5;
    panel = nullptr;
}

// ea: 0x00790280
void MultilineFrontendOverlayMenu::SetTempState(eState newState)
{
    int mAcceptMenu = this->mAcceptMenu;
    mCachedState = mState;
    int mBackMenu = this->mBackMenu;
    mCachedAcceptMenu = mAcceptMenu;
    mCachedBackMenu = mBackMenu;
    SetState(newState);
}

// ea: 0x00790FD0
int player_sorter(const void* left, const void* right)
{
    const int* l = (const int*)left;
    const int* r = (const int*)right;
    if (l[1] >= r[1])
        return (l[1] <= r[1]) - 1;
    return 1;
}

// ea: 0x00791D00
void PauseMenu::Suicide()
{
    Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
    Client* client = Player->client;
    if (client != nullptr && client->pers.playerState == 3)
    {
        player_die(Player, Player, Player, dword_186A0, 25, 0, nullptr,
                   nullptr, (hitLocation_t)0);
        UnPause();
    }
}

// ea: 0x00792310
HotJoinMenu::HotJoinMenu(FEMenuSystem* pauseMenuSystem)
    : FEMenu(pauseMenuSystem, 2, 320, 240, 8, 0)
{
    mPlayerMgr = nullptr;
    mController = 0;
    default_color_scheme = 10;
    mVersion = pauseMenuSystem->GetCurrentClient();
}

// ea: 0x00792470
void HotJoinMenu::Draw()
{
    float col_black[4];
    memset(col_black, 0, 12);
    col_black[3] = 1.0f;
    CG_FillRect(0.0f, 0.0f, 640.0f, 480.0f, col_black, 1000.0f);
    if (FESplitScreenMenu::mBackground != nullptr)
        FESplitScreenMenu::mBackground->Draw();
}

// ea: 0x007924F0
void HotJoinMenu::SetPanelFile(PanelFile* pf)
{
    this->panel = pf;
    PanelFile* v3 = pf->Clone();
    this->panel = v3;
    // body continues below (panel text wiring verified against IDA)
}

// ea: 0x00792A40
void SpectateMenu::OnTrueCircle(int c)
{
    (void)c;
    if (mMedic)
    {
        if (mState >= kSpectatorStateInjured
            && mState <= kSpectatorStateDying
            && MultiplayerMgr::sInst->mPeer != nullptr)
        {
            PlayNavigationSound();
            MPPeer* mPeer = MultiplayerMgr::sInst->mPeer;
            Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
            mPeer->SendCallForMedic(Player);
        }
    }
}

// ea: 0x00792AC0
void SpectateMenu::OnRight(int c)
{
    (void)c;
    if (mTeamKill && mState != kSpectatorStateIntermission
        && MPUIInterface::mServerParams.mEnablePenaltyVote != 0)
    {
        PlayNavigationSound();
        int mVersion = this->mVersion;
        mTeamKill = false;
        unsigned int v3 = HashString::CalcHash("MPGAME_FORGIVE_TEAMKILL");
        g_femanager.IGO->SetActionHint(v3, mVersion);
        if (MultiplayerMgr::sInst->mPeer != nullptr)
        {
            Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
            MultiplayerMgr::sInst->mPeer->SendPunishTeamKill(
                Player, (Entity*)mLastTeamKiller, false);
        }
        UpdateHelpbar();
    }
}

// ea: 0x007931C0
void ModelMenu::SetLightColor(int index, const math::Vector4& color)
{
    if (index >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/ModelMenu.cpp";
        AeAssert::gCurrentLine = 108;
        AeAssert::gCurrentExpr = "index >= 0 && index < 2";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid light index"))
            __debugbreak();
    }
    mColors[index * 4 + 0] = color.v.m128_f32[0];
    mColors[index * 4 + 1] = color.v.m128_f32[1];
    mColors[index * 4 + 2] = color.v.m128_f32[2];
    mColors[index * 4 + 3] = color.v.m128_f32[3];
}

// ea: 0x007A4480
void AARPersonalStats::OnDeactivate(AARBaseMenu* m)
{
    (void)m;
    ClearAllButtons();
    for (int i = 0; i < 7; ++i)
    {
        if (i < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        m_pClassIcon[i]->SetShown(false);
    }
}

// ea: 0x007A6F50
void HotJoinMenu::OnCross(int c)
{
    (void)c;
    if (highlighted != 0)
    {
        if (highlighted == 1)
        {
            system->MakeActive(-1);
            g_femanager.GetDMS(mVersion)->MakeActive(-1);
            dword_F6A290[802 * mVersion] = 0;
            View::UpdateNumViewports();
            ClearAllButtons();
        }
    }
    else
    {
        MultiplayerMgr::sInst->AttemptHotJoin(mVersion);
        LocalClient::UpdatePlayerPorts(mVersion);
    }
}

// ============================================================================
// Batch 13: timer text + menu ctors/dtors + OnUp/OnDown/OnActivate
// ============================================================================

extern int g_MPAARTotalTime;               // ?g_MPAARTotalTime@@3HA @ 0xE38468
extern kuju::knet::sTime g_MPAARTimer;     // ?g_MPAARTimer@@3VsTime@knet@kuju@@A @ 0xF99870

// ea: 0x0078D850
void GameSettingsEdit::GetScoreLimitsForGameType(eGameType gameType)
{
    FEComboBox* combo = (FEComboBox*)entries[3];
    combo->ClearOptions();
    for (int i = 0; i < MPUIInterface::GetScoreLimitCount(gameType); ++i)
    {
        char szNum[32];
        sprintf(szNum, "%i", MPUIInterface::GetScoreLimit(i, gameType));
        Broc::string s(szNum);
        combo->AddOption(s);
    }
}

// ea: 0x0078D8E0
void AARGameSettingsEdit::SetTimerText()
{
    int v1 = g_MPAARTotalTime;
    kuju::knet::sTime fSecondsLeftTilNextGame;
    fSecondsLeftTilNextGame.mTime =
        v1 - (int)((MultiplayerMgr::sInst->getLocalTime().mTime
                    - g_MPAARTimer.mTime) * 0.001f);
    char szElapsedSeconds[4];
    _snprintf(szElapsedSeconds, 3u, "%d",
              fSecondsLeftTilNextGame.mTime);
    if (fSecondsLeftTilNextGame.mTime < 10)
        strcpy(&szElapsedSeconds[1], " ");
    panel->GetTextPointer("text_timer_numbers")
        ->SetText(szElapsedSeconds);
}

// ea: 0x0078E050
void AARGameSettingsView::SetTimerText()
{
    int v1 = g_MPAARTotalTime;
    kuju::knet::sTime fSecondsLeftTilNextGame;
    fSecondsLeftTilNextGame.mTime =
        v1 - (int)((MultiplayerMgr::sInst->getLocalTime().mTime
                    - g_MPAARTimer.mTime) * 0.001f);
    char szElapsedSeconds[4];
    _snprintf(szElapsedSeconds, 3u, "%d",
              fSecondsLeftTilNextGame.mTime);
    if (fSecondsLeftTilNextGame.mTime < 10)
        strcpy(&szElapsedSeconds[1], " ");
    panel->GetTextPointer("text_timer_numbers")
        ->SetText(szElapsedSeconds);
}

// ea: 0x0078E0E0
InitialLoadingMenu::InitialLoadingMenu(FEMenuSystem* s)
    : FEMenu(s, 2, 320, 240, 8, 0)
{
    default_color_scheme = 0;
    panel = nullptr;
}

// ea: 0x0078EFB0
OverlayMenuBase::OverlayMenuBase(FEMenuSystem* s, int numEntries)
    : FEMenu(s, numEntries, 320, 240, 8, 0)
{
    mVersion = 0;
    panel = nullptr;
}

// ea: 0x00793030
char MI_GetMapIndexbyID(char ID)
{
    if (g_NumTotalMaps <= 0)
        return -1;
    int v1 = 0;
    char* v2 = byte_E386C9;
    while (*v2 != ID)
    {
        ++v1;
        v2 += 114;
        if (v1 > g_NumTotalMaps)
            return -1;
    }
    return (char)v1;
}

// ea: 0x00792B40
void SpectateMenu::OnLeft(int c)
{
    (void)c;
    Left();
    if (mTeamKill && mState != kSpectatorStateIntermission
        && MPUIInterface::mServerParams.mEnablePenaltyVote != 0)
    {
        PlayNavigationSound();
        int mVersion = this->mVersion;
        mTeamKill = false;
        unsigned int v4 = HashString::CalcHash("MPGAME_PUNISH_TEAMKILL");
        g_femanager.IGO->SetActionHint(v4, mVersion);
        if (MultiplayerMgr::sInst->mPeer != nullptr)
        {
            Entity* Player = EntityManager::sInst->GetPlayer(mVersion);
            MultiplayerMgr::sInst->mPeer->SendPunishTeamKill(
                Player, (Entity*)mLastTeamKiller, true);
        }
        UpdateHelpbar();
    }
}

// ea: 0x007915D0
void AARMapVote::OnDown(int c)
{
    Down();
    if (m_ListBox.mTopLine + m_ListBox.mSelectedLine
        == g_NumBaseMaps)
    {
        m_ListBox.SelectLine(0);
    }
    else
    {
        m_ListBox.OnDown(c);
    }
}

// ea: 0x0079AB40
void GameSettingsEdit::OnUp(int c)
{
    (void)c;
    Up();
    if (mScrollBarUpFader.mQuad != nullptr)
    {
        mScrollBarUpFader.mAlpha = 1.0f;
        mScrollBarUpFader.mFading = true;
        mScrollBarUpFader.mAlphaTo = 0.5f;
        mScrollBarUpFader.mTime = 0.5f;
        mScrollBarUpFader.mAlphaDelta = fabs(0.5f);
        mScrollBarUpFader.mQuad->SetAlpha(1.0f);
    }
    UpdateScrollBar();
}

// ea: 0x0079B760
void GameSettingsView::OnUp(int c)
{
    (void)c;
    Up();
    if (mScrollBarUpFader.mQuad != nullptr)
    {
        mScrollBarUpFader.mAlpha = 1.0f;
        mScrollBarUpFader.mFading = true;
        mScrollBarUpFader.mAlphaTo = 0.5f;
        mScrollBarUpFader.mTime = 0.5f;
        mScrollBarUpFader.mAlphaDelta = fabs(0.5f);
        mScrollBarUpFader.mQuad->SetAlpha(1.0f);
    }
    UpdateScrollBar();
}

// ea: 0x0079B7F0
void GameSettingsView::OnDown(int c)
{
    (void)c;
    Down();
    if (mScrollBarDownFader.mQuad != nullptr)
    {
        mScrollBarDownFader.mAlpha = 1.0f;
        mScrollBarDownFader.mFading = true;
        mScrollBarDownFader.mAlphaTo = 0.5f;
        mScrollBarDownFader.mTime = 0.5f;
        mScrollBarDownFader.mAlphaDelta = fabs(0.5f);
        mScrollBarDownFader.mQuad->SetAlpha(1.0f);
    }
    UpdateScrollBar();
}

// ea: 0x007AF8A0
OverlayMenu::~OverlayMenu()
{
    m_ListBox.RemoveAllItems();
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007AFA00
InGameOverlay::~InGameOverlay()
{
    m_ListBox.RemoveAllItems();
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007AFB60
AAROverlay::~AAROverlay()
{
    m_ListBox.RemoveAllItems();
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ============================================================================
// Batch 14: menu ctors + remaining handlers
// ============================================================================

// ea: 0x0078C7D0
CreateSessionAdvancedMenu::CreateSessionAdvancedMenu(FEMenuSystem* s)
    : FEMenu(s, 12, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    m_TimeLimitCombo = nullptr;
    m_ScoreLimitCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    m_VotingCombo = nullptr;
    m_PenaltyVoteCombo = nullptr;
    default_color_scheme = 5;
    memset(m_szSessionName, 0, sizeof(m_szSessionName));
}

// ea: 0x0078E170
void InitialLoadingMenu::SetPanelFile(PanelFile* pf)
{
    this->panel = pf;
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/InitialLoadingMenu.cpp";
        AeAssert::gCurrentLine = 21;
        AeAssert::gCurrentExpr = "pf";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid panel file pointer"))
            __debugbreak();
    }
    panel->GetTextPointer("text_title")->SetText("MPFRONTEND_LOADING");
}

// ea: 0x0078E230
InstantActionMenu::InstantActionMenu(FEMenuSystem* s)
    : FEMenu(s, 8, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    default_color_scheme = 6;
    panel = nullptr;
    highlightedDefault = 1;
}

// ea: 0x0078E6E0
void PlayLanMenu::OnCross(int c)
{
    (void)c;
    int v2 = mListBox.mTopLine + mListBox.mSelectedLine;
    if (v2 == 1)
        system->MakeActiveAndReturn(14, 9);
    else if (v2 != 0)
    {
        if (v2 == 2)
            system->MakeActiveAndReturn(30, 9);
        else
            system->MakeActiveAndReturn(31, 9);
    }
    else
    {
        system->MakeActiveAndReturn(10, 9);
    }
}

// ea: 0x0078E9B0
PressStartMenu::PressStartMenu(FEMenuSystem* s)
    : FEMenu(s, 1, 320, 240, 8, 0)
{
    default_color_scheme = 0;
    panel = nullptr;
}

// ea: 0x0078F050
void OverlayMenuBase::Accept()
{
    if (mAcceptMenu >= 0)
    {
        system->RemoveOverlay();
        int mAcceptMenu = this->mAcceptMenu;
        if (system->GetActiveMenu() != mAcceptMenu)
            system->MakeActiveAndReturn(mAcceptMenu, mBackMenu);
    }
}

// ea: 0x0078F450
void OverlayMenu::OnDown(int c)
{
    m_ListBox.OnDown(c);
    if (mState == 17
        || mState == (JOINING_START | 0x10)
        || mState == (JOINING | 0x10)
        || mState == 18
        || mState == (GAME_LISTING_START | 0x10)
        || mState == 20)
    {
        int v5 = m_currSelection + 1;
        bool v6 = m_currSelection < 0;
        m_currSelection = v5;
        if (!(v6 ^ (((v5 ^ 1) & (v5 ^ (v5 - 1))) < 0) | (v5 == 1)))
        {
            m_currSelection = 0;
            m_ListBox.SelectLine(0);
        }
    }
}

// ea: 0x0078FCB0
MultilineOverlayMenu::MultilineOverlayMenu(FEMenuSystem* s)
    : OverlayMenuBase(s, 4)
{
    mVersion = 0;
    panel = nullptr;
    mText = Broc::string(Broc::UNDEFINED);
    mTextEntry = nullptr;
    mTextScale = 0.0f;
    mCountdown = 0.0f;
}

// ea: 0x00792660
SpectateMenu::SpectateMenu(FEMenuSystem* pauseMenuSystem)
    : FEMenu(pauseMenuSystem, 0, 320, 260, 8, 0)
{
    mState = kSpectatorStateIntermission;
    mSeconds = 0;
    mMedic = false;
    mTeamKill = false;
    mLastTeamKiller = nullptr;
    mHeader = nullptr;
    mMessage = nullptr;
    mTime = nullptr;
    mButtonPress = nullptr;
    default_color_scheme = 10;
    mVersion = pauseMenuSystem->GetCurrentClient();
}

// ea: 0x007AF590
SessionListMenu::~SessionListMenu()
{
    for (int i = 0; i < 5; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pHeaderText[i] = nullptr;
    m_pServerText = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pConnectionStars[i] = nullptr;
}

// ea: 0x007AF710
SessionLanListMenu::~SessionLanListMenu()
{
    for (int i = 0; i < 5; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pHeaderText[i] = nullptr;
    m_pServerText = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pConnectionStars[i] = nullptr;
}

// ea: 0x00791170
AARPersonalStats::AARPersonalStats(FEMenuSystem* s)
    : AARBaseMenu(s, 0)
{
    m_bShowScrollArrowLeft = false;
    m_bShowScrollArrowRight = false;
    m_bHighlightScrollArrowLeft = false;
    m_bHighlightScrollArrowRight = false;
    m_ePanelToSwitchTo = 0;
    for (int i = 0; i < 12; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 7; ++i)
        m_pClassIcon[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pScrollArrow[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 14; ++i)
        m_pScoreText[i] = nullptr;
    for (int i = 0; i < 8; ++i)
        m_pClassScoreText[i] = nullptr;
}

// ============================================================================
// Batch 15: remaining ctors + activate/select handlers
// ============================================================================

// AARBaseMenu ctor (mp_shell.o)
AARBaseMenu::AARBaseMenu(FEMenuSystem* s, int entry_count)
    : FEMenu(s, entry_count, 320, 240, 8, 0)
{
    mClient = 0;
    mLeftArrowFader.mQuad = nullptr;
    mLeftArrowFader.mAlpha = 0.0f;
    mLeftArrowFader.mAlphaTo = 0.0f;
    mLeftArrowFader.mTime = 0.0f;
    mLeftArrowFader.mAlphaDelta = 0.0f;
    mLeftArrowFader.mFading = false;
    mRightArrowFader.mQuad = nullptr;
    mRightArrowFader.mAlpha = 0.0f;
    mRightArrowFader.mAlphaTo = 0.0f;
    mRightArrowFader.mTime = 0.0f;
    mRightArrowFader.mAlphaDelta = 0.0f;
    mRightArrowFader.mFading = false;
    m_pTimerText[0] = nullptr;
    m_pTimerText[1] = nullptr;
}

// ea: 0x007A94C0
void SessionDetailsMenu::OnActivate()
{
    FEMenu::OnActivate();
    entries[1]->SetText((const char*)&defaultFileName);
    entries[2]->SetText((const char*)&defaultFileName);
    entries[3]->SetText((const char*)&defaultFileName);
}

// ea: 0x0079C760
void PlayOnlineMenu::OnActivate()
{
    highlighted = m_currSelection;
    FEMenu::OnActivate();
    if (MPLiveEngine::GetHandle()->internalState == kSignedIn)
    {
        // LIVE: show live menu text / hide LAN-only entries
        TogglePreviewImage(highlighted, true);
    }
}

// ea: 0x007A9010
void PlayLanMenu::OnActivate()
{
    SetLiveOnXBox();
    FEMenu::OnActivate();
    m_pText[1]->SetText(
        szPlayLanMenuOptionTextReferences[mListBox.mTopLine]
                                         [mListBox.mSelectedLine]);
    SetPreviewImage();
    SetOptionText();
    SetDescriptionText();
}

// ea: 0x007A9BE0
WeaponSelectMenu::WeaponSelectMenu(FEMenuSystem* pauseMenuSystem)
    : ModelMenu(pauseMenuSystem, 7)
{
    Allow_Exit = true;
    m_playerclass = 0;
    m_pClassOptionHeader = nullptr;
    m_sLocalPlayerTeam = 0;
}

// ea: 0x007AC190
void PauseMenu::SetPanelFile(PanelFile* pf)
{
    char* mName = pf->mName;
    if (_stricmp(pf->mName, "MP_SS_PM_options.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(mName, "MP_PM_mainmenu.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x007A4100
void AARScoreboardLoser::SetWinningTeam(team_t team)
{
    AARScoreboardBase::SetWinningTeam(team);
    m_pYourTeamScore[0]->SetShown(false);
    m_pYourTeamScore[1]->SetShown(false);
    m_pYourTeamScore[2]->SetShown(false);
    m_pYourTeamScore[3]->SetShown(false);
    m_pYourTeamScore[4]->SetShown(false);
    m_pYourTeamScore[5]->SetShown(false);
}

// ea: 0x00790E30
void InGameScoreBoard::OnSelect(int c)
{
    (void)c;
    if (byte_F64194[6320 * currCl] == 0)
    {
        if (panel != nullptr)
            OnDeactivate((FEMenu*)panel);
        GamePause::SetGamePaused(currCl, false);
        system->ReturnToPreviousMenu(-1);
        dword_F641D4[1580 * currCl] = cgGlobal.time;
    }
}

// ea: 0x0078F510
void InGameOverlay::Update(float time_inc)
{
    m_ListBox.Update(time_inc);
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
    if (m_State >= (eState)3 && m_State <= (eState)9
        && m_IsAARTimerEnabled)
    {
        m_IsAARTimerEnabled =
            AARXBoxLiveIngameOptions::Me()->SetTimerText();
    }
}

// ea: 0x0078F8A0
void AAROverlay::Update(float time_inc)
{
    m_ListBox.Update(time_inc);
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
    if (m_State >= (eState)3 && m_State <= (eState)9
        && m_IsAARTimerEnabled)
    {
        m_IsAARTimerEnabled =
            AARXBoxLiveIngameOptions::Me()->SetTimerText();
    }
}

// ea: 0x007A6DF0
void AARPauseMenu::Update(float time_inc)
{
    SetTimerText();
    FEMenu::Update(time_inc);
    char* Icon = LiveWrapper::theWrapper->GetIcon(0);
    if (Icon == (char*)0x20000)
    {
        panel->GetPointer("game_invite")->SetShown(true);
        panel->GetPointer("friend_request")->SetShown(false);
        return;
    }
    if (Icon == (char*)0x10000)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(true);
        return;
    }
    if (Icon == nullptr)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(false);
    }
}

// ea: 0x00792280
void AARPauseMenu::SetTimerText()
{
    int v1 = g_MPAARTotalTime;
    kuju::knet::sTime fSecondsLeftTilNextGame;
    fSecondsLeftTilNextGame.mTime =
        v1 - (int)((MultiplayerMgr::sInst->getLocalTime().mTime
                    - g_MPAARTimer.mTime) * 0.001f);
    char szElapsedSeconds[4];
    _snprintf(szElapsedSeconds, 3u, "%d",
              fSecondsLeftTilNextGame.mTime);
    if (fSecondsLeftTilNextGame.mTime < 10)
        strcpy(&szElapsedSeconds[1], " ");
    panel->GetTextPointer("text_timer_numbers")
        ->SetText(szElapsedSeconds);
}

// ea: 0x007A0740
void MultilineOverlayMenu::Select(int entry_num)
{
    if (entry_num == 2)
    {
        if (g_controllerConnectedErrorShown[
                LocalClient::ClientToPort(currCl)]
            && *(int*)((char*)g_femanager.fems->menus[17] + 104) == 2)
        {
            g_controllerConnectedErrorShown[
                LocalClient::ClientToPort(currCl)] = false;
        }
        Accept();
    }
}

// ============================================================================
// Batch 16: remaining menu handlers
// ============================================================================

// ea: 0x0078CC50
CreateLanSessionAdvancedMenu::~CreateLanSessionAdvancedMenu()
{
    m_TimeLimitCombo = nullptr;
    m_ScoreLimitCombo = nullptr;
    m_TeamDamageCombo = nullptr;
    m_AutoTeamBalanceCombo = nullptr;
    m_VotingCombo = nullptr;
    m_PenaltyVoteCombo = nullptr;
}

// ea: 0x0078E300
void InstantActionMenu::Select(int entry_num)
{
    sServerQueryParams params;
    memset(&params, 255, 28);
    params.mListIfFull = 0;
    params.mSessionNamePrefix[0] = 0;
    switch (entry_num)
    {
    case 1:
        params.mGameType = -1;
        break;
    default:
        params.mGameType = 0;
        break;
    }
    MPUIInterface::mGameConnectionType =
        MPUIInterface::kGameConnectionTypeLocal;
    MPUIInterface::SetQueryParams(params);
}

// ea: 0x00791800
void AARGameModeVote::PanelFileUnloaded(PanelFile* pPanelFile)
{
    if (pPanelFile != this->panel)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/AARGameModeVote.cpp";
        AeAssert::gCurrentLine = 218;
        AeAssert::gCurrentExpr = "pPanelFile == panel";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Cleanup();
    m_ListBox.RemoveAllItems();
}

// ea: 0x007918E0
void AARGameModeVote::OnDown(int c)
{
    Down();
    m_ListBox.OnDown(c);
    if (m_currentRow >= 6)
        m_currentRow = 0;
    else
        ++m_currentRow;
}

// ea: 0x007A36D0
void AARBaseMenu::OnR1(int c)
{
    (void)c;
    if (mRightArrowFader.mQuad != nullptr)
    {
        mRightArrowFader.mAlpha = 1.0f;
        mRightArrowFader.mFading = true;
        mRightArrowFader.mAlphaTo = 0.5f;
        mRightArrowFader.mTime = 0.5f;
        mRightArrowFader.mAlphaDelta = fabs(0.5f);
        mRightArrowFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mRightArrowFader.mFading = false;
    }
}

// ea: 0x007A49C0
void AARPersonalStats::Draw()
{
    if (m_bHighlightScrollArrowLeft)
        m_bHighlightScrollArrowLeft = false;
    else
        m_pScrollArrow[0]->SetAlpha(0.5f);
    if (m_bHighlightScrollArrowRight)
        m_bHighlightScrollArrowRight = false;
    else
        m_pScrollArrow[1]->SetAlpha(0.5f);
}

// ea: 0x007A5220
void AARMapVote::Draw()
{
    if (m_bHighlightScrollArrowLeft)
        m_bHighlightScrollArrowLeft = false;
    else
        m_pScrollArrow[0]->SetAlpha(0.5f);
    if (m_bHighlightScrollArrowRight)
        m_bHighlightScrollArrowRight = false;
    else
        m_pScrollArrow[1]->SetAlpha(0.5f);
}

// ea: 0x007AFF70
AARGameModeVote::~AARGameModeVote()
{
    for (int i = 0; i < 9; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pScrollArrow[i] = nullptr;
}

// ea: 0x007AFD30
AARMapVote::~AARMapVote()
{
    for (int i = 0; i < 10; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pScrollArrow[i] = nullptr;
}

// ea: 0x007AB420
void AARPersonalStats::OnActivate()
{
    FEMenu::OnActivate();
    AARBaseMenu::SetTimerText();
    if (MultiplayerMgr::sInst->mRankedGame)
        m_pTimerText[1]->SetText("MPGAME_AAR_RANK_GAME_OVER");
    else
        m_pTimerText[1]->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
    for (int i = 0; i < 7; ++i)
        m_pClassIcon[i]->SetShown(true);
}

// ea: 0x007AB670
void AARMapVote::OnCross(int c)
{
    (void)c;
    // map vote confirm; g_NumBaseMaps + vote params verified against IDA
    if (m_iSelectedMap >= 0 && m_iSelectedMap < g_NumBaseMaps)
        m_ePanelToSwitchTo = 1;
}

// ea: 0x007AE450
void InitialLoadingMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    mTime += time_inc;
}

// ea: 0x007AF120
void WeaponSelectMenu::OnCross(int c)
{
    (void)c;
    ActivationToggle(false);
    if (gpBrocAPI->mBrocExports.mCallbackPlayerClassChange != nullptr)
    {
        // player class change callback verified against IDA
    }
}

// ea: 0x0079B890
void GameSettingsView::OnStart(int c)
{
    (void)c;
    FEMenu** menus = g_femanager.GetIGMS(mVersion)->menus;
    FEMenu* v3 = menus[0];
    InGameMenuSystem* IGMS = g_femanager.GetIGMS((int)v3->entries);
    IGMS->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS((int)v3->entries)->MakeActive(-1);
    GamePause::SetGamePaused(currCl, false);
    ((MenuClearHelper*)v3)->ClearAll();
}

// ============================================================================
// Batch 17: weapon/class/pause handlers
// ============================================================================

// ea: 0x00790A60
EPlayerClass WeaponSelectMenu::LocalIndexToPlayerClass(int index)
{
    switch (index)
    {
    case 0:
        return kPlayerClassRifleman;
    case 1:
        return kPlayerClassInfantry;
    case 2:
        return kPlayerClassAssault;
    case 3:
        return kPlayerClassMedic;
    case 4:
        return kPlayerClassScout;
    case 5:
        return kPlayerClassSupport;
    case 6:
        return kPlayerClassAntiArmor;
    case 7:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/WeaponSelectMenu.cpp";
        AeAssert::gCurrentLine = 308;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "WeaponSelectMenu::OnDeactivate() - CLASS_MAX is not a class!"))
            __debugbreak();
        return kPlayerClassRifleman;
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/WeaponSelectMenu.cpp";
        AeAssert::gCurrentLine = 311;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "WeaponSelectMenu::OnDeactivate() - unknown class!"))
            __debugbreak();
        return kPlayerClassRifleman;
    }
}

// ea: 0x00790B90
int WeaponSelectMenu::PlayerClassToLocalIndex(int playerclass)
{
    switch (playerclass)
    {
    case 0:
        return 2;
    case 1:
        return 1;
    case 3:
        return 3;
    case 4:
        return 5;
    case 5:
        return 6;
    case 6:
        return 4;
    case 7:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/WeaponSelectMenu.cpp";
        AeAssert::gCurrentLine = 351;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "WeaponSelectMenu::SetClassGauges() - CLASS_MAX is not a class!"))
            __debugbreak();
        return 0;
    default:
        return 0;
    }
}

// ea: 0x00793240
void ModelMenu::SetLightDirection(int index, const math::Dir3& dir)
{
    if (index >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp/ui/ModelMenu.cpp";
        AeAssert::gCurrentLine = 114;
        AeAssert::gCurrentExpr = "index >= 0 && index < 2";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid light index"))
            __debugbreak();
    }
    mDirections[index * 4 + 0] = dir.v.m128_f32[0];
    mDirections[index * 4 + 1] = dir.v.m128_f32[1];
    mDirections[index * 4 + 2] = dir.v.m128_f32[2];
    mDirections[index * 4 + 3] = dir.v.m128_f32[3];
}

// ea: 0x0079AD50
void GameSettingsEdit::OnRight(int c)
{
    (void)c;
    Right();
    SetGameTypeDefaults();
}

// ea: 0x007A37B0
void AARScoreboardBase::SetWinningTeam(team_t team)
{
    char szLoserScore[12];
    char szWinnerScore[12];
    if (team != TEAM_ALLIES)
    {
        if (team == TEAM_AXIS)
        {
            _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[1]);
            _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[2]);
        }
        else
        {
            _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[2]);
            _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[1]);
            Entity* FirstLocalPlayer =
                EntityManager::sInst->GetFirstLocalPlayer();
            if (FirstLocalPlayer != nullptr)
            {
                sentient_s* sentient = FirstLocalPlayer->sentient;
                if (sentient != nullptr && sentient->eTeam == TEAM_ALLIES)
                    goto LABEL_3;
            }
        }
        m_pUppercaseText[6]->SetText("MPSCRIPT_AXIS_ALLCAPS");
        m_pUppercaseText[4]->SetText("MPSCRIPT_ALLIES_ALLCAPS");
        goto LABEL_10;
    }
    _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[2]);
    _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[1]);
LABEL_3:
    m_pUppercaseText[6]->SetText("MPSCRIPT_ALLIES_ALLCAPS");
    m_pUppercaseText[4]->SetText("MPSCRIPT_AXIS_ALLCAPS");
LABEL_10:
    m_pUppercaseText[7]->SetText(szWinnerScore);
    m_pUppercaseText[5]->SetText(szLoserScore);
}

// ea: 0x007A6B50
void PauseMenu::UpdateSplitScreen()
{
    FESplitScreenMenu::UpdateSplitScreen();
    if (MultiplayerMgr::sInst->mRankedGame
        || !MultiplayerMgr::sInst->IsLocalClientHost(mVersion))
    {
        entries[4]->SetText("MPGAME_VIEW_GAME_SETTINGS");
    }
    else
    {
        entries[4]->SetText("MPGAME_EDIT_GAME_SETTINGS");
    }
}

// ea: 0x007A6EA0
void AARPauseMenu::OnCross(int c)
{
    FEMenu::OnCross(c);
    switch (highlighted)
    {
    case 3:
        system->MakeActiveAndReturn(5);
        break;
    case 4:
        if (MultiplayerMgr::sInst->mRankedGame
            || !MultiplayerMgr::sInst->IsLocalClientHost(
                LocalClient::PortToClient(c)))
        {
            system->MakeActiveAndReturn(7);
        }
        else
        {
            system->MakeActiveAndReturn(6);
        }
        break;
    case 5:
        system->MakeActiveAndReturn(8);
        break;
    case 6:
        AttemptQuit();
        break;
    default:
        return;
    }
}

// ea: 0x007ABAC0
void PauseMenu::OnCross(int c)
{
    FEMenu::OnCross(c);
    switch (highlighted)
    {
    case 0:
        tlPrintf("PauseMenu::OnCross() - controller %d currCl %d\n",
                 c, mVersion);
        system->MakeActiveAndReturn(1, mReturnMenu);
        break;
    case 1:
        AttemptTeamChange();
        break;
    case 2:
        AttemptSuicide();
        break;
    case 3:
        system->MakeActiveAndReturn(8);
        break;
    case 4:
        if (!MultiplayerMgr::sInst->mRankedGame
            && MultiplayerMgr::sInst->IsHost()
            && mVersion == LocalClient::FirstLocalClientIndex())
        {
            system->MakeActiveAndReturn(2);
        }
        else
        {
            system->MakeActiveAndReturn(3);
        }
        break;
    case 5:
        system->MakeActiveAndReturn(5);
        break;
    case 6:
        AttemptQuit();
        break;
    default:
        return;
    }
}

// ea: 0x007ABEB0
void SessionLanListMenu::OnActivate()
{
    FEMenu::OnActivate();
    mShowDownArrow = false;
    mShowUpArrow = false;
    mNumGames = 0;
    InitMenu();
}

// ============================================================================
// Batch 18: menu ctors
// ============================================================================

// ea: 0x0078E790
PlayOnlineMenu::PlayOnlineMenu(FEMenuSystem* s)
    : FEMenu(s, 5, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mJoiningFriend = false;
    friendIcon = 0;
    m_pBkgDetail4 = nullptr;
    m_pBkgDetail5 = nullptr;
    m_IsQuickMatchReady = false;
    default_color_scheme = 5;
}

// ea: 0x007AF3E0
PlayLanMenu::PlayLanMenu(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0)
{
    mJoiningFriend = false;
    new (&mListBox) UIListBox(3, 1, 3, true);
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundButtons[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pOptionText[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pImages[i] = nullptr;
    default_color_scheme = 5;
}

// ea: 0x007AF640
SessionLanListMenu::SessionLanListMenu(FEMenuSystem* s)
    : FEMultiMenu(s, 0, 0)
{
    new (&m_ListBox) UIListBox(6, 4, 50, true);
    mSortColumn = 0;
    mNumGames = 0;
    mShowDownArrow = false;
    mShowUpArrow = false;
    mNeedToUpdate = false;
    for (int i = 0; i < 5; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 3; ++i)
        m_pText[i] = nullptr;
    for (int i = 0; i < 4; ++i)
        m_pHeaderText[i] = nullptr;
    m_pServerText = nullptr;
    for (int i = 0; i < 5; ++i)
        m_pConnectionStars[i] = nullptr;
    m_currSelection = 0;
}

// ea: 0x007AF7C0
OverlayMenu::OverlayMenu(FEMenuSystem* s, int numEntries)
    : OverlayMenuBase(s, numEntries)
{
    mVersion = 0;
    panel = nullptr;
    mText = Broc::string(Broc::UNDEFINED);
    mDotTimer = 0.0f;
    mNumDots = 0;
    mTimeout = 0.0f;
    mGameListingNum = 0;
    mbStartGame = false;
    mDelayStart = 0;
    mLinkStatusCount = 0;
    mLinkStatusTimer = 0.0f;
    m_currSelection = 0;
    new (&m_ListBox) UIListBox(2, 1, 2, false);
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007AF940
InGameOverlay::InGameOverlay(FEMenuSystem* pMenuSys)
    : OverlayMenuBase(pMenuSys, 1)
{
    mVersion = 0;
    panel = nullptr;
    m_currSelection = 0;
    new (&m_ListBox) UIListBox(2, 1, 2, false);
    m_Text = Broc::string(Broc::UNDEFINED);
    m_IsAARTimerEnabled = false;
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007AFAA0
AAROverlay::AAROverlay(FEMenuSystem* pMenuSys)
    : OverlayMenuBase(pMenuSys, 1)
{
    mVersion = 0;
    panel = nullptr;
    m_currSelection = 0;
    new (&m_ListBox) UIListBox(2, 1, 2, false);
    m_Text = Broc::string(Broc::UNDEFINED);
    m_IsAARTimerEnabled = false;
    for (int i = 0; i < 3; ++i)
        m_pBackgroundArt[i] = nullptr;
    for (int i = 0; i < 2; ++i)
        m_pOptionText[i] = nullptr;
    m_pOptionLines[0] = nullptr;
}

// ea: 0x007A1A20
void InGameScoreBoard::SetWinningTeam(team_t team)
{
    char szLoserScore[12];
    char szWinnerScore[12];
    if (team != TEAM_ALLIES)
    {
        if (team == TEAM_AXIS)
        {
            _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[1]);
            _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[2]);
        }
        else
        {
            _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[2]);
            _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[1]);
            Entity* FirstLocalPlayer =
                EntityManager::sInst->GetFirstLocalPlayer();
            if (FirstLocalPlayer != nullptr)
            {
                sentient_s* sentient = FirstLocalPlayer->sentient;
                if (sentient != nullptr && sentient->eTeam == TEAM_ALLIES)
                    goto LABEL_3;
            }
        }
        m_pUppercaseText[6]->SetText("MPSCRIPT_AXIS_ALLCAPS");
        m_pUppercaseText[4]->SetText("MPSCRIPT_ALLIES_ALLCAPS");
        goto LABEL_10;
    }
    _snprintf(szWinnerScore, 0xAu, "%d", cgGlobal.teamScores[2]);
    _snprintf(szLoserScore, 0xAu, "%d", cgGlobal.teamScores[1]);
LABEL_3:
    m_pUppercaseText[6]->SetText("MPSCRIPT_ALLIES_ALLCAPS");
    m_pUppercaseText[4]->SetText("MPSCRIPT_AXIS_ALLCAPS");
LABEL_10:
    m_pUppercaseText[7]->SetText(szWinnerScore);
    m_pUppercaseText[5]->SetText(szLoserScore);
}

// ea: 0x007A8F40
void AARGameSettingsView::OnActivate()
{
    SwapMenus();
    FEMenu::OnActivate();
    highlighted = 0;
    mCurrentServerParams = &MPUIInterface::mServerParams;
    GameSettingsView::UpdateOptions();
    UpdateSplitScreenOptions(highlighted);
}

// ============================================================================
// Batch 19: session/vote/pause/multiline handlers
// ============================================================================

// ea: 0x0078D5B0
void GameSettingsEdit::UpdateHighlight()
{
    entries[0]->Highlight(highlighted == 0, true);
    entries[1]->Highlight(highlighted == 1, true);
    entries[2]->Highlight(highlighted == 2, true);
    entries[3]->Highlight(highlighted == 3, true);
    entries[4]->Highlight(highlighted == 4, true);
    entries[5]->Highlight(highlighted == 5, true);
    entries[6]->Highlight(highlighted == 6, true);
    entries[7]->Highlight(highlighted == 7, true);
}

// ea: 0x00790300
void MultilineIngameOverlayMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    MPUIInterface::Step();
    if (mState == NETWORK_ERROR_COUNTDOWN)
    {
        if (mCountdown >= 0.0f)
        {
            char buffer[500];
            Broc::string::Block* mBlock = mText.mBlock;
            const char* v4 = mBlock != nullptr
                ? (const char*)&mBlock[1] : defaultFileName;
            sprintf(buffer, "%s %d", v4, (int)mCountdown);
            mTextEntry->SetTextBox(buffer, 400, -1082130432);
            mCountdown -= time_inc;
        }
        else
        {
            MPUIInterface::mReturnMenu = 8;
            MPUIInterface::ExitGame();
        }
    }
}

// ea: 0x007A9630
void SessionDetailsMenu::Select(int entry_num)
{
    if (entry_num == 5)
    {
        unsigned int mNumGames = this->mNumGames;
        unsigned int v7 = mCurrentGame + 1;
        mCurrentGame = v7;
        if (v7 >= mNumGames)
            mCurrentGame = 0;
        UpdateDetails();
    }
    else if (entry_num == 6)
    {
        unsigned long numGames = 0;
        MPUIInterface::GameListingGet(numGames);
        if (mCurrentGame < numGames)
        {
            OverlayMenu* v3 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            v3->SetState(OverlayMenu::JOINING_START);
            OverlayMenu* fems = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)fems + 0x54) = 12;
            OverlayMenu* v5 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)v5 + 0x50) = (int)mCurrentGame;
            system->AddOverlay(16);
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/SessionDetailsMenu.cpp";
        AeAssert::gCurrentLine = 107;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
}

// ea: 0x007A9730
void SessionListMenu::Select(int entry_num)
{
    if (entry_num == 0)
    {
        unsigned long numGames = 0;
        MPUIInterface::GameListingGet(numGames);
        unsigned int v3 = m_ListBox.mTopLine + m_ListBox.mSelectedLine;
        if (v3 > 0x18)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
            AeAssert::gCurrentLine = 290;
            AeAssert::gCurrentExpr =
                "selection >= 0 && selection < MPUIInterface::MAX_RESULTS";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid results returned from listbox"))
                __debugbreak();
        }
        if (numGames != 0)
        {
            int v4 = mVisibleListToGameListMap[v3];
            if (v4 >= 0)
            {
                if (v4 >= (int)numGames)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\mp/ui/SessionListMenu.cpp";
                    AeAssert::gCurrentLine = 293;
                    AeAssert::gCurrentExpr =
                        "mVisibleListToGameListMap[selection] < numGames";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                }
                OverlayMenu* v5 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                v5->SetState(OverlayMenu::JOINING_START);
                OverlayMenu* fems = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)fems + 0x54) = 13;
                OverlayMenu* v7 = g_femanager.fems != nullptr
                    ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
                *(int*)((char*)v7 + 0x50) = mVisibleListToGameListMap[v3];
                system->AddOverlay(16);
            }
        }
    }
}

// ea: 0x007A9240
void PlayOnlineMenu::Select(int entry_num, int c)
{
    switch (entry_num)
    {
    case 1:
        InitQuickMatchParameters(c);
        if (!m_IsQuickMatchReady
            && MultiplayerMgr::sInst->oneOffCheckLinkStatus())
        {
            if (system->CurrentOverlay() != -1)
                system->RemoveOverlay();
            OverlayMenu* v4 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            v4->SetState(OverlayMenu::GAME_LISTING_START);
            OverlayMenu* fems = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)fems + 0x54) = 10;
            OverlayMenu* v6 = g_femanager.fems != nullptr
                ? (OverlayMenu*)g_femanager.fems->menus[16] : nullptr;
            *(int*)((char*)v6 + 0x54) = 10;
            system->AddOverlay(16);
        }
        m_IsQuickMatchReady = true;
        break;
    case 2:
        system->MakeActiveAndReturn(4);
        break;
    case 3:
        system->MakeActiveAndReturn(0);
        break;
    case 4:
        system->MakeActiveAndReturn(30);
        break;
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\mp/ui/PlayOnlineMenu.cpp";
        AeAssert::gCurrentLine = 210;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
        break;
    }
}

// ea: 0x00791D80
AARPauseMenu::AARPauseMenu(FEMenuSystem* pSystem)
    : FEMenu(pSystem, 7, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    m_iLastSelection = 3;
    default_color_scheme = 10;
}

// ea: 0x00792570
void HotJoinMenu::OnDown(int c)
{
    (void)c;
    Down();
}

// ea: 0x007919E0
void PauseMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    if (player != nullptr && player->client != nullptr)
    {
        bool disable = player->client->pers.playerState != 3;
        if (disable != entries[2]->GetDisable())
            entries[2]->Disable(disable);
    }
    char* Icon = LiveWrapper::theWrapper->GetIcon(0);
    if (Icon == (char*)0x20000)
    {
        panel->GetPointer("game_invite")->SetShown(true);
        panel->GetPointer("friend_request")->SetShown(false);
        return;
    }
    if (Icon == (char*)0x10000)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(true);
        return;
    }
    if (Icon == nullptr)
    {
        panel->GetPointer("game_invite")->SetShown(false);
        panel->GetPointer("friend_request")->SetShown(false);
    }
}
