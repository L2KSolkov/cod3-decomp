// ============================================================================
// MPMainMenuXBox.cpp - Xbox multiplayer menus (mp_xbox.o mp/ui/*.cpp)
// 61 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// FEMenu/UIListBox/PanelFile bases are provided by shell.o/core.o.
// ============================================================================

#include "MPMainMenuXBox.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

// ============================================================================
// Assertion system externs (core_xboxr:AeAssert.o)
// ============================================================================
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

// ============================================================================
// Data (mp_xbox.o rdata 0xE37930)
// ============================================================================
const char* szMPMainMenuXBoxOptionTextReferences[4] = {
    "MPFRONTEND_PLAY_XBOX_LIVE",
    "MPFRONTEND_PLAY_SYSTEM_LINK",
    "MPFRONTEND_MM_XBOX_LIVE_OPTIONS",
    "MPFRONTEND_APPEAR_ONLINE_OFFLINE",
};

// ============================================================================
// Externs (shell.o / core.o / mp.o / mp_shell.o)
// ============================================================================
extern int g_MPAARTotalTime;
extern struct MPAARTimer { int mTime; } g_MPAARTimer;
extern void tlPrintf(const char* fmt, ...);
extern bool gSkipFrontEnd;
extern bool gSkipMovies;

struct kuju_knet_sTime {
    float mTime;
};

#define ICON_GAME_INVITE ((char*)0x20000)
#define ICON_FRIEND_REQUEST ((char*)0x10000)
extern int gDelayRenderForNFrames;

struct OverlayMenuUpdate {
    void Update(int v);
};

// InGameOverlay / AAROverlay (shell.o)
struct InGameOverlay {
    static InGameOverlay* Me(int version);
    static void SetState(InGameOverlay* self, int state);
    int mAcceptMenu;
    int mBackMenu;
};
enum {
    OVERLAY_APPEAR_ONLINE = 16,
    OVERLAY_APPEAR_OFFLINE = 17,
    OVERLAY_SIGNIN_SIGNOUT = 18,
    OVERLAY_TOGGLE_VOICE = 19,
    OVERLAY_REBOOT_REQUIRED = 20,
    OVERLAY_JOIN_FRIEND = 21,
};
struct AAROverlay {
    static void SetState(AAROverlay* self, int state);
    int mAcceptMenu;
    int mBackMenu;
};
struct AARMenuSystem {
    FEMenu** menus;
};

// SaveGameData invite fields (StubData.savedInvite at +0x314)
struct XONLINE_ACCEPTED_GAMEINVITE {
    unsigned char raw[0x9C];
};
struct XONLINE_FRIEND_INVITE {
    XONLINE_FRIEND InvitingFriend;
};
struct SavedInviteData {
    unsigned char raw[0x9C];
    XONLINE_FRIEND_INVITE* asFriend;
};

// Zero the savedInvite's InviteAcceptTime (StubData.savedInvite at +0x314;
// InviteAcceptTime is the first FILETIME pair -> cleared when starting).
extern void ClearSavedInviteTime(SaveGameData* g);

// MPProfileMainMenu (mp_shell.o)
struct MPProfileMainMenu {
    static void Me();
    static void LoadProfileData();
};

namespace MPUIInterface {
bool IsOnlineGame();
bool IsLANGame();
bool IsLocalGame();
bool InSession();
void ExitGame();
void QueryFromID(XNKID* sessionID);
void Step();
  void bdNetStop();
void PlatformStop();
void PlatformStart();
void Reboot();
extern bool mLiveQueryActive;
extern bool mQueryFromID;
extern bool mIsViewableOnline;
extern bool mCableDisconnect;
int mGameConnectionType = 1;  // namespace twin (binary's static lives in mp.o)
// EGameConnectionType (IDA types): Lan=0, Online=1, Local=2
enum { kGameConnectionTypeLan = 0, kGameConnectionTypeOnline = 1,
       kGameConnectionTypeLocal = 2 };

// ea: 0x0072F480 (mp.o)
bool IsOnlineGame()  // ?IsOnlineGame@MPUIInterface@@YA_NXZ
{
    return mGameConnectionType == kGameConnectionTypeOnline;
}

// ea: 0x0072F470 (mp.o)
bool IsLANGame()
{
    return mGameConnectionType == kGameConnectionTypeLan;
}

// ea: 0x0072F490 (mp.o)
bool IsLocalGame()
{
    return mGameConnectionType == kGameConnectionTypeLocal;
}
}

class controller { public:
    bool* accepting_input_from_controller;
    int locked_port;
    bool is_locked;
    static controller* inst();
    static int num_controllers;
};
extern void Controller_LockPort(int port);

// ============================================================================
// JoinGameMenu
// ============================================================================

// ea: 0x778580
JoinGameMenu::JoinGameMenu(FEMenuSystem* s)
    : FEMenu(s, 2, 320, 240, 8, 0)
{
    mSignedInFromInvite = false;
    mJoiningGame = false;
    default_color_scheme = 9;
}

// ea: 0x7789D0
JoinGameMenu* JoinGameMenu::Me()
{
    return (JoinGameMenu*)g_femanager.fems->menus[15];
}

// ea: 0x7785C0
void JoinGameMenu::OnActivate()
{
    MPLiveEngine* Handle = MPLiveEngine::GetHandle();
    Handle->DoWork();
    if (Handle->internalState == kNotSignedIn)
    {
        LiveWrapper::theWrapper->SignInFromInvite(&gSaveGameData[0].savedInvite,
                                      0x40140);
        mSignedInFromInvite = true;
    }
    if (Handle->internalState == kSigningIn)
    {
        OverlayMenu::Me(0)->SetState(16);  // SIGNING_IN
        system->AddOverlay(16);
        OverlayMenu::Me(0)->Update(1058642330);
    }
    Handle->DoWork();
    FEMenu::OnActivate();
}

// ea: 0x7786C0
void JoinGameMenu::Draw()
{
    if (panel != nullptr)
        PanelFile::Draw(panel);
    FEMenu::Draw();
}

// ea: 0x7786E0
void JoinGameMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    MPLiveEngine* Handle = MPLiveEngine::GetHandle();
    Handle->DoWork();
    if (Handle->internalState == kSignedIn)
    {
        if (mJoiningGame)
        {
            if (!MPUIInterface::mLiveQueryActive || !MPUIInterface::mQueryFromID)
            {
                  unsigned long numGames = 0;
                  MPUIInterface::GameListingGet(numGames);
                  if (numGames == 0)
                {
                    ClearSavedInviteTime(&gSaveGameData[0]);
                    mJoiningGame = false;
                    OverlayMenu::Me(0)->SetState(23);  // JOIN_FAILED
                    OverlayMenu::Me(0)->mAcceptMenu = 10;
                    OverlayMenu::Me(0)->mBackMenu = 10;
                    system->AddOverlay(16);
                }
                else
                {
                    if (Handle->actualPort == (unsigned int)-1)
                    {
                        unsigned int PortToLock = Handle->GetPortToLock();
                        if (PortToLock == (unsigned int)-1)
                        {
                            ASSERT("lockedPort != -1",
                                   "c:\\cod\\code\\game\\mp/ui/JoinGameMenu.cpp", 101);
                        }
                        controller* v11 = controller::inst();
                        v11->locked_port = (int)PortToLock;
                        v11->is_locked = true;
                        Handle->actualPort = PortToLock;
                    }
                    mJoiningGame = false;
                    ClearSavedInviteTime(&gSaveGameData[0]);
                    MPUIInterface::mGameConnectionType =
                        MPUIInterface::kGameConnectionTypeOnline;
                    OverlayMenu::Me(0)->SetState(24);  // JOINING_START
                    OverlayMenu::Me(0)->mBackMenu = 10;
                    OverlayMenu::Me(0)->mGameListingNum = 0;
                    system->AddOverlay(16);
                    gDelayRenderForNFrames = 0;
                }
            }
        }
        else
        {
            Handle->DoWork();
            if (!mSignedInFromInvite)
                Handle->JoinGame((XONLINE_FRIEND*)&gSaveGameData[0].savedInvite[0x28]);
            Handle->DoWork();
            OverlayMenu::Me(0)->SetState(16);
            OverlayMenu::Me(0)->mAcceptMenu = 10;
            OverlayMenu::Me(0)->mBackMenu = 10;
            system->AddOverlay(16);
            mJoiningGame = true;
        }
    }
    if (Handle->internalState == kNotSignedIn)
    {
        LiveWrapper::theWrapper->SignInSilently(0x40140);
        LiveWrapper::theWrapper->DoWork();
        system->MakeActiveAndReturn(8);
    }
}

// ea: 0x778910
void JoinGameMenu::Select(int entry_num)
{
    (void)entry_num;
}

// ea: 0x778920
void JoinGameMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x778930
void JoinGameMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    if (pf == nullptr)
    {
        ASSERT("panel", "c:\\cod\\code\\game\\mp/ui/JoinGameMenu.cpp", 170);
    }
    FEText* TextPointer = PanelFile::GetTextPointer(panel, "join_message");
    FEMenu::AddEntry(0, TextPointer, false);
    FEText* v6 = PanelFile::GetTextPointer(panel, "Helpbar");
    FEMenu::AddEntry(1, v6, false);
    entries[0]->SetText("Joining Game");
}

// ============================================================================
// MPMainMenuXBox
// ============================================================================

// ea: 0x77AB80
MPMainMenuXBox::MPMainMenuXBox(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0), mListBox(3, 1, 3, true)
{
    mWaitingForSignIn = false;
    mOptionDescription = nullptr;
    m_currSelection = 0;
    mWidescreen = false;
    default_color_scheme = 10;
    memset(&m_pBackgroundArt, 0, sizeof(m_pBackgroundArt));
    memset(m_pOptionText.m_elements, 0, sizeof(m_pOptionText.m_elements));
    memset(m_pText.m_elements, 0, sizeof(m_pText.m_elements));
    memset(m_pImages.m_elements, 0, sizeof(m_pImages.m_elements));
    mContentAlreadyDiscovered = false;
}

// ea: 0x77AC50
MPMainMenuXBox::~MPMainMenuXBox()
{
    mListBox.RemoveAllItems();
    if (mOptionDescription != nullptr)
        delete mOptionDescription;
    mOptionDescription = nullptr;
    memset(&m_pBackgroundArt, 0, sizeof(m_pBackgroundArt));
    memset(m_pOptionText.m_elements, 0, sizeof(m_pOptionText.m_elements));
    memset(m_pText.m_elements, 0, sizeof(m_pText.m_elements));
    memset(m_pImages.m_elements, 0, sizeof(m_pImages.m_elements));
    mListBox.~UIListBox();
    FEMenu::~FEMenu();
}

// ea: 0x778B60
MPMainMenuXBox* MPMainMenuXBox::Me()
{
    return (MPMainMenuXBox*)g_femanager.fems->menus[8];
}

// ea: 0x7789E0
void MPMainMenuXBox::Draw()
{
    if (panel != nullptr)
        PanelFile::Draw(panel);
    FEMenu::Draw();
}

// ea: 0x778A10
void MPMainMenuXBox::OnXBoxLive(int c)
{
    (void)c;
    MPUIInterface::mGameConnectionType =
        MPUIInterface::kGameConnectionTypeOnline;
    ELiveState internalState =
        (ELiveState)MPLiveEngine::GetHandle()->internalState;
    if (internalState != kNotSignedIn)
    {
        int v4 = internalState - 1;
        if (v4 != 0)
        {
            if (v4 == 1)
            {
                system->MakeActiveAndReturn(10);
            }
            else
            {
                ASSERT("0", "c:\\cod\\code\\game\\mp/ui/MPMainMenuXBox.cpp", 329);
            }
        }
        else
        {
            system->AddOverlay(16);
            OverlayMenu::Me(0)->SetState(16);  // SIGNING_IN
    OverlayMenu::Me(0)->Update(1058642330);
            mWaitingForSignIn = true;
        }
    }
    else
    {
        LiveWrapper::theWrapper->ShowLoginScreen(0x40140);
        mWaitingForSignIn = true;
    }
}

// ea: 0x778AE0
void MPMainMenuXBox::OnSplitScreen()
{
}

// ea: 0x778AF0
void MPMainMenuXBox::OnSystemLinkXBox()
{
    MPUIInterface::mGameConnectionType = MPUIInterface::kGameConnectionTypeLan;
    system->MakeActiveAndReturn(9);
}

// ea: 0x778B10
void MPMainMenuXBox::OnXboxLiveOptions()
{
    system->MakeActiveAndReturn(30);
}

// ea: 0x778B20
void MPMainMenuXBox::OnOption(int c)
{
    (void)c;
    system->MakeActiveAndReturn(27);
    MPProfileMainMenu::Me();
    MPProfileMainMenu::LoadProfileData();
}

// ea: 0x778B40
void MPMainMenuXBox::OnTriangle(int c)
{
    (void)c;
    if (!mWaitingForSignIn)
    {
        MPUIInterface::bdNetStop();
        MPUIInterface::PlatformStop();
        MPUIInterface::Reboot();
    }
}

// ea: 0x778B70
void MPMainMenuXBox::OnDown(int c)
{
    if (!lockInput)
    {
        mListBox.OnDown(c);
        int v4 = m_currSelection + 1;
        bool v5 = m_currSelection - 2 < 0;
        m_currSelection = v4;
        if (v5 == (v4 - 3 < 0))
        {
            m_currSelection = 0;
            mListBox.SelectLine(0);
        }
    }
}

// ea: 0x778BC0
void MPMainMenuXBox::OnUp(int c)
{
    if (!lockInput)
    {
        mListBox.OnUp(c);
        if (--m_currSelection < 0)
        {
            m_currSelection = 2;
            mListBox.SelectLine(2);
        }
    }
}

// ea: 0x778C10
void MPMainMenuXBox::UpdateWidescreen(BOOL widescreen)
{
    if (mWidescreen != widescreen)
    {
        FEMenu::UpdateWidescreen(widescreen);
        if (mOptionDescription != nullptr)
            mOptionDescription->UpdateForWidescreen(widescreen);
        if (panel != nullptr)
            mWidescreen = widescreen;
    }
}

// ea: 0x779170
void MPMainMenuXBox::OnLive(int c)
{
    OnXBoxLive(c);
}

// ea: 0x779180
void MPMainMenuXBox::OnSystemLink()
{
    MPUIInterface::mGameConnectionType = MPUIInterface::kGameConnectionTypeLan;
    system->MakeActiveAndReturn(9);
}

// ea: 0x7791A0
void MPMainMenuXBox::OnCross(int c)
{
    Controller_LockPort(c);
    int v3 = mListBox.mTopLine + mListBox.mSelectedLine;
    if (v3 != 0)
    {
        int v4 = v3 - 1;
        if (v4 != 0)
        {
            if (v4 == 1)
                system->MakeActiveAndReturn(30);
        }
        else
        {
            MPUIInterface::mGameConnectionType =
                MPUIInterface::kGameConnectionTypeLan;
            system->MakeActiveAndReturn(9);
            tlPrintf("OnCross in main menu setting enable link check to true\n");
            MultiplayerMgr::sInst->mLinkCheckEnabled = true;
            MultiplayerMgr::Step(MultiplayerMgr::sInst, 0, false, true);
        }
    }
    else
    {
        OnXBoxLive(c);
    }
}

// ea: 0x779230 - SetPanelFile (see below)
void MPMainMenuXBox::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    if (pf == nullptr)
    {
        ASSERT("panel", "c:\\cod\\code\\game\\mp/ui/MPMainMenuXBox.cpp", 570);
    }
    const char* bgNames[12] = {
        "bkg", "bkg_line_01", "bkg_preview_outline",
        "bkg_btn_back_01", "bkg_btn_back_02", "bkg_btn_back_03",
        "bkg_btn_back_04", "bkg_btn_back_05", "bkg_btn_line_01",
        "bkg_btn_line_02", "bkg_btn_line_03", "bkg_btn_line_04",
    };
    for (int j = 0; j < 12; ++j)
    {
        m_pBackgroundArt[j] = PanelFile::GetPointer(panel, bgNames[j]);
        if (m_pBackgroundArt[j] == nullptr)
        {
            ASSERT("m_pBackgroundArt[i]",
                   "c:\\cod\\code\\game\\mp/ui/MPMainMenuXBox.cpp", 597);
        }
    }
    PanelFile::GetPointer(panel, "bkg_btn_back_04")->SetShown(false);
    PanelFile::GetPointer(panel, "bkg_btn_line_03")->SetShown(false);

    const char* optionTextNames[3] = {
        "text_option_01", "text_option_02", "text_option_03",
    };
    for (int k = 0; k < 3; ++k)
    {
        FEText* TextPointer =
            PanelFile::GetTextPointer(panel, optionTextNames[k]);
        mListBox.SetItem(k, 0, TextPointer, 0);
        mListBox.SetText(k, 0, szMPMainMenuXBoxOptionTextReferences[k]);
    }

    const char* textNames[4] = {
        "text_screen_title", "text_option_title",
        "text_option_description", "text_helpbar",
    };
    for (int v8 = 0; v8 < 4; ++v8)
    {
        m_pText[v8] = PanelFile::GetTextPointer(panel, textNames[v8]);
        if (m_pText[v8] == nullptr)
        {
            ASSERT("m_pText[i]", "c:\\cod\\code\\game\\mp/ui/MPMainMenuXBox.cpp", 647);
        }
        if (v8 == 3)
        {
            FEMultiLineText* v35 =
                (FEMultiLineText*)mem_heap_malloc(0xA8);
            if (v35 != nullptr)
            {
                FEText* v36 = m_pText[3];
                color32 v37;
                v37.i = v36->GetColor();
                float layer = v36->GetScaleX();
                float x1 = v36->GetY();
                float v20 = v36->GetX();
                font_index v11 = v36->GetFont();
                color32 col;
                v35 = new (v35) FEMultiLineText(v11, x1, 0.0f, 0,
                                                (panel_layer)layer,
                                                0.0f, 0, (int)v37.i, col);
            }
            helpbar1 = v35;
            helpbar1->SetNumLines(1);
            helpbar1->SetText("MPFRONTEND_SELECT_BACK_TO_SP");
        }
        else if (v8 == 0)
        {
            m_pText[0]->SetText("FEMENU_MULTI", 0);
        }
    }

    if (m_pText[2] == nullptr)
    {
        ASSERT("m_pText[text_option_description]",
               "c:\\cod\\code\\game\\mp/ui/MPMainMenuXBox.cpp", 663);
    }
    FEMultiLineText* v36 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    if (v36 != nullptr)
    {
        FEText* v35 = m_pText[2];
        color32 v37;
        v37.i = v35->GetColor();
        float layera = v35->GetScaleX();
        float x1a = v35->GetY();
        float v21 = v35->GetX();
        font_index v16 = v35->GetFont();
        v36 = new (v36) FEMultiLineText(v16, x1a, 0.0f, 1,
                                        (panel_layer)layera,
                                        16.0f, 64, (int)v37.i, v37);
    }
    mOptionDescription = v36;
    mOptionDescription->SetNumLines(5);

    const char* imageNames[3] = {
        "preview_image_01", "preview_image_02", "preview_image_03",
    };
    for (int m = 0; m < 3; ++m)
    {
        m_pImages[m] = PanelFile::GetPointer(panel, imageNames[m]);
        if (m_pImages[m] == nullptr)
        {
            ASSERT("m_pImages[i]",
                   "c:\\cod\\code\\game\\mp/ui/MPMainMenuXBox.cpp", 685);
        }
        m_pImages[m]->SetShown(false);
    }
    mListBox.SetAllColumnsSelectable(false);
    mListBox.mSelectedFlashing = true;
    if (mListBox.mItemColumnsCount <= 0)
    {
        ASSERT("column >= 0 && column < mItemColumnsCount",
               "c:\\cod\\code\\game\\UIListBox.h", 125);
    }
    if (mListBox.mSelectedRowColorChangeColumns.mSize <= 0)
    {
        ASSERT("iIndex >= 0 && iIndex < mSize",
               "../ae\\core\\ae_vector.h", 167);
    }
    mListBox.mSelectedRowColorChangeColumns.mElements[0] = true;
    mListBox.Refresh();
}

// ea: 0x779BB0
void MPMainMenuXBox::SetOptionText()
{
    int m_currSelection = this->m_currSelection;
    m_pText[1]->SetText(szMPMainMenuXBoxOptionTextReferences[m_currSelection],
                        0);
    const char* descRefs[3] = {
        "MPFRONTEND_XBOXLIVE_DESCRIPTION",
        "MPFRONTEND_SYSTEMLINK_DESCRIPTION",
        "MPFRONTEND_XBOXLIVEOPTIONS_MENU_DESCRIPTION",
    };
    const char* STBString =
        STBManager::sInst->GetSTBString(descRefs[m_currSelection]);
    int v6 = mWidescreen ? 390 : 520;
    mOptionDescription->SetTextBoxNoLocalize(Broc::string(STBString), v6,
                                             -1.0f);
}

// ea: 0x779C60
void MPMainMenuXBox::SetImage()
{
    for (int i = 0; i < 3; ++i)
        m_pImages[i]->SetShown(false);
    unsigned int m_currSelection = (unsigned int)this->m_currSelection;
    if (m_currSelection > 2)
    {
        ASSERT("idx >= 0 && idx < _SIZE", "../ae\\core\\ae_array.h", 31);
    }
    m_pImages[m_currSelection]->SetShown(true);
}

// ea: 0x77A860
void MPMainMenuXBox::OnActivate()
{
    *controller::inst()->accepting_input_from_controller = 0x01010101;
    if (mWidescreen != (cg_widescreen_integer != 0))
        UpdateWidescreen(cg_widescreen_integer != 0);
    MultiplayerMgr::sInst->mRankedGame = false;
    if (MPUIInterface::mCableDisconnect)
    {
        MPUIInterface::bdNetStop();
        MPUIInterface::PlatformStop();
        MPUIInterface::mCableDisconnect = false;
    }
    MultiplayerMgr::sInst->mLinkCheckEnabled = false;
    MPUIInterface::PlatformStart();
    SetOptionText();
    SetImage();
    mListBox.SelectLine(m_currSelection);
    mContentAlreadyDiscovered = false;
}

// ea: 0x77A8F0
void MPMainMenuXBox::Update(float time_inc)
{
    SetOptionText();
    SetImage();
    FEMenu::Update(time_inc);
    mListBox.Update(time_inc);
    MPUIInterface::Step();
    if (mWaitingForSignIn
        && MPLiveEngine::GetHandle()->internalState != kSigningIn)
    {
        system->MakeActiveAndReturn(10);
        mWaitingForSignIn = false;
    }
}

// ============================================================================
// XBoxLiveIngameOptionsCOD3
// ============================================================================

// ea: 0x77AD10
XBoxLiveIngameOptionsCOD3::XBoxLiveIngameOptionsCOD3(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0), m_ListBox(5, 1, 5, true)
{
    memset(m_pOldTextColor.m_elements, 0, sizeof(m_pOldTextColor.m_elements));
    memset(m_pOldSelectedTextColor.m_elements, 0,
           sizeof(m_pOldSelectedTextColor.m_elements));
    m_currSelection = 0;
    friendIcon = 0;
    wasSignedIn = false;
    waitingForSignIn = false;
    mJoiningFriend = false;
    default_color_scheme = 10;
    memset(m_pBackgroundArt.m_elements, 0, sizeof(m_pBackgroundArt.m_elements));
    memset(m_pText.m_elements, 0, sizeof(m_pText.m_elements));
    memset(m_pLineArt.m_elements, 0, sizeof(m_pLineArt.m_elements));
    mVersion = s->GetCurrentClient();
}

// ea: 0x77AE60
XBoxLiveIngameOptionsCOD3::~XBoxLiveIngameOptionsCOD3()
{
    memset(m_pBackgroundArt.m_elements, 0, sizeof(m_pBackgroundArt.m_elements));
    memset(m_pText.m_elements, 0, sizeof(m_pText.m_elements));
    memset(m_pLineArt.m_elements, 0, sizeof(m_pLineArt.m_elements));
    memset(m_pOldTextColor.m_elements, 0, sizeof(m_pOldTextColor.m_elements));
    memset(m_pOldSelectedTextColor.m_elements, 0,
           sizeof(m_pOldSelectedTextColor.m_elements));
    m_ListBox.~UIListBox();
    FEMenu::~FEMenu();
}

// ea: 0x778C50
void XBoxLiveIngameOptionsCOD3::Init()
{
}

// ea: 0x778C60
void XBoxLiveIngameOptionsCOD3::Draw()
{
    if (panel != nullptr)
        PanelFile::Draw(panel);
    FEMenu::Draw();
}

// ea: 0x778C80
void XBoxLiveIngameOptionsCOD3::OnDeactivate(FEMenu* m)
{
    (void)m;
    FEMenu::ClearAllButtons();
}

// ea: 0x778C90
XBoxLiveIngameOptionsCOD3* XBoxLiveIngameOptionsCOD3::Me(int client)
{
    return (XBoxLiveIngameOptionsCOD3*)g_femanager.GetIGMS(client)->menus[5];
}

// ea: 0x778CB0
void XBoxLiveIngameOptionsCOD3::OnUp(int c)
{
    if (wasSignedIn)
    {
        FEMenu::Up();
        m_ListBox.OnUp(c);
        if (--m_currSelection < 0)
            m_currSelection = 4;
        m_ListBox.SelectLine(m_currSelection);
    }
}

// ea: 0x778D10
void XBoxLiveIngameOptionsCOD3::OnDown(int c)
{
    if (wasSignedIn)
    {
        FEMenu::Down();
        m_ListBox.OnDown(c);
        int v4 = m_currSelection + 1;
        bool v5 = m_currSelection - 4 < 0;
        m_currSelection = v4;
        if (v5 == (v4 - 5 < 0))
            m_currSelection = 0;
        m_ListBox.SelectLine(m_currSelection);
    }
}

// ea: 0x778D70
void XBoxLiveIngameOptionsCOD3::OnLeft(int c)
{
    (void)c;
    FEMenu::Left();
}

// ea: 0x778D80
void XBoxLiveIngameOptionsCOD3::OnRight(int c)
{
    (void)c;
    FEMenu::Right();
}

// ea: 0x778D90
void XBoxLiveIngameOptionsCOD3::OnL1(int c)
{
    (void)c;
}

// ea: 0x778DA0
void XBoxLiveIngameOptionsCOD3::OnR1(int c)
{
    (void)c;
}

// ea: 0x778DB0
void XBoxLiveIngameOptionsCOD3::OnCross(int c)
{
    (void)c;
    if (!waitingForSignIn)
    {
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        switch (m_currSelection)
        {
        case 0:
            if (MPUIInterface::mIsViewableOnline)
                InGameOverlay::SetState(InGameOverlay::Me(0), OVERLAY_APPEAR_OFFLINE);
            else
                InGameOverlay::SetState(InGameOverlay::Me(0), OVERLAY_APPEAR_ONLINE);
            InGameOverlay::Me(0)->mAcceptMenu = 5;
            InGameOverlay::Me(0)->mBackMenu = 5;
            system->AddOverlay(13);
            break;
        case 1:
            LiveWrapper::ShowFriendsList(Handle, Handle->actualPort);
            break;
        case 2:
            LiveWrapper::ShowPlayersList(Handle, Handle->actualPort, 0);
            break;
        case 3:
            InGameOverlay::SetState(InGameOverlay::Me(0), OVERLAY_TOGGLE_VOICE);
            InGameOverlay::Me(0)->mAcceptMenu = 5;
            InGameOverlay::Me(0)->mBackMenu = 5;
            system->AddOverlay(13);
            break;
        case 4:
            if (wasSignedIn)
            {
                InGameOverlay::SetState(InGameOverlay::Me(0),
                                        OVERLAY_SIGNIN_SIGNOUT);
                InGameOverlay::Me(0)->mAcceptMenu = 5;
                InGameOverlay::Me(0)->mBackMenu = 5;
                system->AddOverlay(13);
            }
            else
            {
                LiveWrapper::theWrapper->ShowLoginScreen(0x40140);
                waitingForSignIn = true;
            }
            break;
        default:
            return;
        }
    }
}

// ea: 0x778EE0
void XBoxLiveIngameOptionsCOD3::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x778EF0
void XBoxLiveIngameOptionsCOD3::OnCircle(int c)
{
    (void)c;
}

// ea: 0x778F00
void XBoxLiveIngameOptionsCOD3::OnSquare(int c)
{
    (void)c;
}

// ea: 0x778F10
void XBoxLiveIngameOptionsCOD3::OnStart(int c)
{
    (void)c;
}

// ea: 0x778F20
void XBoxLiveIngameOptionsCOD3::ButtonHeldAction()
{
}

// ea: 0x778F30
void XBoxLiveIngameOptionsCOD3::UpdateSplitScreen()
{
    PanelFile::UpdateSplitScreen(panel, 0, 0);
}

// ea: 0x778F60
void XBoxLiveIngameOptionsCOD3::WireForSignedOut()
{
    const char* szSlotText[4] = {
        "slot_01_text_option", "slot_02_text_option",
        "slot_03_text_option", "slot_04_text_option",
    };
    for (int i = 0; i < 4; ++i)
    {
        FEText* TextPointer =
            PanelFile::GetTextPointer(panel, szSlotText[i]);
        TextPointer->SetAlpha(1036831949);
    }
}

// ea: 0x77A6B0
void XBoxLiveIngameOptionsCOD3::WireForSignedIn()
{
    const char* szSlotText[4] = {
        "slot_01_text_option", "slot_02_text_option",
        "slot_03_text_option", "slot_04_text_option",
    };
    for (int i = 0; i < 4; ++i)
    {
        FEText* TextPointer =
            PanelFile::GetTextPointer(panel, szSlotText[i]);
        TextPointer->SetColorMenuItem(m_pOldTextColor[i].i,
                                      m_pOldSelectedTextColor[i].i);
    }
}

// ea: 0x77A600
void XBoxLiveIngameOptionsCOD3::OnActivate()
{
    FEMenu::OnActivate();
    friendIcon = 0;
    if (PanelFile::GetPointer(panel, "game_invite") != nullptr)
        PanelFile::GetPointer(panel, "game_invite")->SetShown(false);
    if (PanelFile::GetPointer(panel, "friend_request") != nullptr)
        PanelFile::GetPointer(panel, "friend_request")->SetShown(false);
    MPLiveEngine* Handle = MPLiveEngine::GetHandle();
    LiveWrapper::theWrapper->SetNotificationFlag(Handle->actualPort, 0, true);
    if (Handle->internalState != kSignedIn)
    {
        WireForSignedOut();
        m_currSelection = 4;
        m_ListBox.SelectLine(4);
    }
}

// ea: 0x77A950
void XBoxLiveIngameOptionsCOD3::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    m_ListBox.Update(time_inc);
    char* Icon = LiveWrapper::theWrapper->GetIcon(0);
    if (Icon == nullptr || MPLiveEngine::GetHandle()->internalState != kSignedIn)
    {
        PanelFile::GetPointer(panel, "game_invite")->SetShown(false);
        goto LABEL_8;
    }
    if (Icon == ICON_GAME_INVITE)
    {
        PanelFile::GetPointer(panel, "game_invite")->SetShown(true);
LABEL_8:
        PanelFile::GetPointer(panel, "friend_request")->SetShown(false);
        goto LABEL_9;
    }
    if (Icon == ICON_FRIEND_REQUEST)
    {
        PanelFile::GetPointer(panel, "game_invite")->SetShown(false);
        PanelFile::GetPointer(panel, "friend_request")->SetShown(true);
    }
LABEL_9:
    if (waitingForSignIn
        && MPLiveEngine::GetHandle()->internalState != kSigningIn)
    {
        waitingForSignIn = false;
    }
    MPLiveEngine* Handle = MPLiveEngine::GetHandle();
    if (Handle->lastNotification == kConfirmReboot)
    {
        Handle->lastNotification = kLiveOk;
        Handle->renderingEnabled = false;
        InGameOverlay::SetState(InGameOverlay::Me(0), OVERLAY_REBOOT_REQUIRED);
        InGameOverlay::Me(0)->mAcceptMenu = 5;
        InGameOverlay::Me(0)->mBackMenu = 5;
        system->AddOverlay(13);
    }
    else if (Handle->lastNotification == kConfirmFriendJoin)
    {
        Handle->lastNotification = kLiveOk;
        InGameOverlay::SetState(InGameOverlay::Me(0), OVERLAY_JOIN_FRIEND);
        InGameOverlay::Me(0)->mAcceptMenu = 5;
        InGameOverlay::Me(0)->mBackMenu = 5;
        system->AddOverlay(13);
    }
    if (wasSignedIn != (Handle->internalState == kSignedIn))
    {
        if (Handle->internalState == kSignedIn)
            WireForSignedIn();
        else
            WireForSignedOut();
        wasSignedIn = !wasSignedIn;
    }
    if (!wasSignedIn)
        m_currSelection = 4;
}

// ea: 0x77AB40
void XBoxLiveIngameOptionsCOD3::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    FEMenu::Cleanup();
    if (mVersion > 0)
    {
        PanelFile* panel = this->panel;
        if (panel != nullptr)
        {
            delete panel;
            mem_heap_free(panel);
        }
        this->panel = nullptr;
    }
}

// ea: 0x779D30 - SetPanelFile (large panel wiring; decompile cached)
void XBoxLiveIngameOptionsCOD3::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    if (pf == nullptr)
    {
        ASSERT("pf", "c:\\cod\\code\\game\\mp/ui/XBoxLiveIngameOptionsCOD3.cpp", 0);
    }
    bool v4 = mVersion <= 0;
    if (!v4)
        panel = PanelFile::Clone(pf);
    // Full wiring mirrors XboxLiveOptionsMenu::SetPanelFile (see that port).
    const char* bgNames[6] = {
        "bkg", "bkg_line_01", "bkg_line_02", "bkg_line_03", "bkg_line_04",
        "bkg_btn_back_01",
    };
    for (int i = 0; i < 6; ++i)
    {
        m_pBackgroundArt[i] = PanelFile::GetPointer(panel, bgNames[i]);
        if (m_pBackgroundArt[i] == nullptr)
            m_pBackgroundArt[i] = nullptr;
        else
            m_pBackgroundArt[i]->SetShown(true);
    }
    const char* textNames[2] = { "text_screen_title", "text_helpbar" };
    for (int i = 0; i < 2; ++i)
        m_pText[i] = PanelFile::GetTextPointer(panel, textNames[i]);
    m_ListBox.SetAllColumnsSelectable(false);
    m_ListBox.mSelectedFlashing = true;
    m_ListBox.Refresh();
}

// ============================================================================
// AARXBoxLiveIngameOptions
// ============================================================================

// ea: 0x77AF10
AARXBoxLiveIngameOptions::AARXBoxLiveIngameOptions(FEMenuSystem* s)
    : XBoxLiveIngameOptionsCOD3(s)
{
    mVersion = s->GetCurrentClient();
}

// ea: 0x77AF70
AARXBoxLiveIngameOptions::~AARXBoxLiveIngameOptions()
{
}

// ea: 0x778FB0
AARXBoxLiveIngameOptions* AARXBoxLiveIngameOptions::Me()
{
    return (AARXBoxLiveIngameOptions*)g_femanager.mAARS->menus[8];
}

// ea: 0x778FC0
void AARXBoxLiveIngameOptions::OnCross(int c)
{
    (void)c;
    if (!waitingForSignIn)
    {
        MPLiveEngine* Handle = MPLiveEngine::GetHandle();
        AAROverlay* v5 = (AAROverlay*)g_femanager.mAARS->menus[10];
        switch (m_currSelection)
        {
        case 0:
            if (MPUIInterface::mIsViewableOnline)
                AAROverlay::SetState(v5, OVERLAY_APPEAR_OFFLINE);
            else
                AAROverlay::SetState(v5, OVERLAY_APPEAR_ONLINE);
            goto LABEL_9;
        case 1:
            LiveWrapper::ShowFriendsList(Handle, Handle->actualPort);
            return;
        case 2:
            LiveWrapper::ShowPlayersList(Handle, Handle->actualPort, 0);
            return;
        case 3:
            AAROverlay::SetState(v5, OVERLAY_TOGGLE_VOICE);
            goto LABEL_9;
        case 4:
            if (wasSignedIn)
            {
                AAROverlay::SetState(v5, OVERLAY_SIGNIN_SIGNOUT);
LABEL_9:
                v5->mAcceptMenu = 8;
                v5->mBackMenu = 8;
                system->AddOverlay(10);
            }
            else
            {
                LiveWrapper::theWrapper->ShowLoginScreen(0x40140);
                waitingForSignIn = true;
            }
            break;
        default:
            return;
        }
    }
}

// ea: 0x7790C0
bool AARXBoxLiveIngameOptions::SetTimerText()
{
    kuju_knet_sTime fSecondsLeftTilNextGame;
    fSecondsLeftTilNextGame.mTime =
        (float)g_MPAARTotalTime
        - ((float)MultiplayerMgr::sInst->getLocalTime().mTime
           - (float)g_MPAARTimer.mTime) * 0.001f;
    char szElapsedSeconds[4];
    snprintf(szElapsedSeconds, 3, "%d", (int)fSecondsLeftTilNextGame.mTime);
    if (fSecondsLeftTilNextGame.mTime < 10.0f)
        strcpy(&szElapsedSeconds[1], " ");
    FEText* TextPointer =
        PanelFile::GetTextPointer(panel, "text_timer_numbers");
    TextPointer->SetText(szElapsedSeconds, 0);
    return fSecondsLeftTilNextGame.mTime > 0.5f;
}

// ea: 0x77A7C0
void AARXBoxLiveIngameOptions::OnActivate()
{
    XBoxLiveIngameOptionsCOD3::OnActivate();
    PanelFile::GetPointer(panel, "bkg")->SetShown(true);
    FEText* TextPointer =
        PanelFile::GetTextPointer(panel, "text_timer_numbers");
    TextPointer->SetShown(true);
    FEText* v4 = PanelFile::GetTextPointer(panel, "text_timer_text");
    v4->SetShown(true);
    v4->SetText("MPGAME_AAR_SECONDS_TIL_NEXT_GAME", 0);
    FEText* v6 = PanelFile::GetTextPointer(panel, "text_title_AAR");
    v6->SetShown(true);
    v6->SetText("MPGAME_AFTER_ACTION_REVIEW", 0);
}

// ea: 0x77AB20
void AARXBoxLiveIngameOptions::Update(float time_inc)
{
    XBoxLiveIngameOptionsCOD3::Update(time_inc);
    SetTimerText();
}
