// ============================================================================
// session_menus.cpp - mp_shell.o session/overlay menu classes
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/mpshell/session_menus.h"

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

extern void tlPrintf(const char* fmt, ...);  // ?tlPrintf@@YAXPBDZZ (tl_system.o)
extern "C" void __stdcall DmGetXboxName(char* name, unsigned int* size);  // xbox_shim
extern void j_nullsub_46(void* self);  // g.o nullsub

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
