// ============================================================================
// XboxLiveMenus.cpp - Xbox Live options menus (game_xbox.o mp/ui/*.cpp)
// 43 functions, verified against IDA. FEMenu/UIListBox/PanelFile/DialogMenu
// bases are provided by shell.o/core.o (declared extern here).
// ============================================================================

#include "XboxLiveMenus.h"

void FEMultiLineText::UpdateForSplitScreen(int viewport, int old_viewport)
{
    if (viewport == old_viewport)
        return;

    FEText::UpdateForSplitScreen(viewport, old_viewport);

    float old_scale;
    switch (old_viewport)
    {
    case 3:
    case 4:
        old_scale = 1.4285715f;
        box_width = (int)(box_width * old_scale);
        button_scale *= old_scale;
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        old_scale = 1.6666666f;
        box_width = (int)(box_width * old_scale);
        button_scale *= old_scale;
        break;
    default:
        break;
    }

    float new_scale;
    switch (viewport)
    {
    case 3:
    case 4:
        new_scale = 0.69999999f;
        box_width = (int)(box_width * new_scale);
        button_scale *= new_scale;
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        new_scale = 0.60000002f;
        box_width = (int)(box_width * new_scale);
        button_scale *= new_scale;
        break;
    default:
        return;
    }
}

void FEMultiLineText::UpdateForWidescreen(bool widescreen)
{
    FEText::UpdateForWidescreen(widescreen);
    if (widescreen)
        box_width = (int)(box_width * 0.75f);
    else
        box_width = (int)(box_width * 1.3333334f);

    if (line_avail_num > 1)
    {
        Broc::string text((Broc::string::Block*)nullptr);
        for (int i = 0; i < line_num; ++i)
        {
            text += lines[i].data;
            text += ' ';
        }
        if (text.mBlock != nullptr && text.mBlock != (Broc::string::Block*)-12
            && text.mBlock->mBuff[0] != 0)
        {
            SetTextBoxNoLocalize(Broc::string(text.c_str()), GetBoxWidth(),
                                 -1.0f);
        }
    }
}

// OverlayMenu / InGameLiveOptionsMenu statics (shell.o; stubs, port later)
OverlayMenu* OverlayMenu::Me(int version)
{
    (void)version;
    FEMenuSystem* fems = g_femanager.fems;
    if (fems != nullptr)
        return reinterpret_cast<OverlayMenu*>(fems->menus[16]);
    return reinterpret_cast<OverlayMenu*>(fems);
}
void OverlayMenu::SetState(int state)
{
    (void)state;
}
bool InGameLiveOptionsMenu::ResponseYesJoin(int a)
{
    (void)a;
    return false;
}
bool InGameLiveOptionsMenu::ResponseNoJoin(int a)
{
    (void)a;
    return false;
}

// PanelFile members (shell.o; stubs, port later)
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <new>

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
// Data (game_xbox.o rdata 0xE36B54 / 0xE36B68)
// ============================================================================
const char* szXBoxOptionReferences[5] = {
    "MPFRONTEND_APPEAR_ONLINE_OFFLINE",
    "MPFRONTEND_FRIENDS",
    "MPFRONTEND_RECENT_PLAYERS",
    "MPFRONTEND_VOICE_OUTPUT",
    "MPFRONTEND_SIGN_IN_OUT",
};
const char* szXBoxOptionDescriptionReferences[5] = {
    "MPFRONTEND_APPEAR_ONLINE_OFFLINE_DESCRIPTION",
    "MPFRONTEND_FRIENDS_DESCRIPTION",
    "MPFRONTEND_RECENT_PLAYERS_DESCRIPTION",
    "MPFRONTEND_VOICE_OUTPUT_DESCRIPTION",
    "MPFRONTEND_SIGN_IN_OUT_DESCRIPTION",
};

// ============================================================================
// Externs (shell.o / core.o / mp.o)
// ============================================================================
STBManager* STBManager::sInst;
extern bool g_controllerConnectedErrorShown[];
extern bool g_IgnoreUIXInput;
int currCl;
int cg_widescreen_integer;
int controller::num_controllers = 4;

extern void tlPrintf(const char* fmt, ...);
extern void Com_Printf(const char* fmt, ...);
extern void Cvar_Set(const char* var_name, const char* value);
extern void Cvar_SetValue(const char* var_name, float value);
extern const char defaultFileName[];
extern bool gSkipFrontEnd;
extern bool gSkipMovies;
extern void D3DDevice_SetVertexShader(unsigned int Handle);
extern void D3DDevice_SetPixelShaderProgram(void* pPSDef);
extern void D3DDevice_SetVertexShaderInputDirect(void* pVAF, int StreamCount,
                                                void* pStreamInputs);
extern void D3DDevice_SetIndices(void* pIndexBuffer, int BaseVertexIndex);
extern void D3DDevice_SetVertexShaderConstant1Fast(int Register,
                                                   void* pConstantData);
extern void D3DDevice_SetShaderConstantMode(int Mode);
extern void nglDxUnbindTexStages();
extern long nglDxCheckErrorD3D(long dwErrCode, const char* a2,
                              unsigned int a3);
struct nglDxRenderState {
    void Init();
};
extern nglDxRenderState nglDxState;
extern void nglDxInitShaders(bool RegisterShaders);

namespace LocalClient {
    int ClientToPort(int client);
}

// Icon constants (LiveWrapper::GetIcon returns one of these pointers)
#define ICON_GAME_INVITE ((char*)0x20000)
#define ICON_FRIEND_REQUEST ((char*)0x10000)

// movie_manager (shell.o class statics; manglings verified)
class movie_manager {
public:
    static void render();         // ?render@movie_manager@@SAXXZ
    static void frame_advance();  // ?frame_advance@movie_manager@@SAXXZ
};

// _LAUNCH_DATA / XGetLaunchInfo (XAPI; stub declared in xlive.h)
struct _LAUNCH_DATA {
    unsigned char raw[0x100];
};
// XGetLaunchInfo (XAPI; stub)
int XGetLaunchInfo(unsigned int* pdwLaunchDataType,
                   _LAUNCH_DATA* pLaunchData)
{
    (void)pLaunchData;
    *pdwLaunchDataType = 0;
    return 1;
}

// ============================================================================
// XboxLiveOptionsMenu
// ============================================================================

// ea: 0x727300
XboxLiveOptionsMenu::XboxLiveOptionsMenu(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0), m_ListBox(5, 1, 5, true)
{
    memset(m_pOldTextColor.m_elements, 0, sizeof(m_pOldTextColor.m_elements));
    memset(m_pOldSelectedTextColor.m_elements, 0,
           sizeof(m_pOldSelectedTextColor.m_elements));
    mPanel = nullptr;
    m_currSelection = 0;
    mHelpbar = nullptr;
    mInstructionsText = nullptr;
    mWidescreen = false;
    friendIcon = 0;
    wasSignedIn = false;
    waitingForSignIn = false;
    mJoiningFriend = false;
    default_color_scheme = 10;
    memset(m_pBackgroundArt.m_elements, 0, sizeof(m_pBackgroundArt.m_elements));
    memset(m_pText.m_elements, 0, sizeof(m_pText.m_elements));
    memset(m_pLineArt.m_elements, 0, sizeof(m_pLineArt.m_elements));
    memset(m_pRowArt.m_elements, 0, sizeof(m_pRowArt.m_elements));
    mVersion = s->GetCurrentClient();
}

// ea: 0x727480
XboxLiveOptionsMenu::~XboxLiveOptionsMenu()
{
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    this->mInstructionsText = nullptr;
    memset(m_pBackgroundArt.m_elements, 0, sizeof(m_pBackgroundArt.m_elements));
    memset(m_pText.m_elements, 0, sizeof(m_pText.m_elements));
    memset(m_pLineArt.m_elements, 0, sizeof(m_pLineArt.m_elements));
    memset(m_pRowArt.m_elements, 0, sizeof(m_pRowArt.m_elements));
    memset(m_pOldTextColor.m_elements, 0, sizeof(m_pOldTextColor.m_elements));
    memset(m_pOldSelectedTextColor.m_elements, 0,
           sizeof(m_pOldSelectedTextColor.m_elements));
    m_ListBox.~UIListBox();
    FEMenu::~FEMenu();
}

// ea: 0x721E10
XboxLiveOptionsMenu* XboxLiveOptionsMenu::Me()
{
    return (XboxLiveOptionsMenu*)g_femanager.fems->menus[30];
}

// ea: 0x721DB0
void XboxLiveOptionsMenu::Init()
{
}

// ea: 0x721DC0
void XboxLiveOptionsMenu::Draw()
{
    PanelFile* mPanel = this->mPanel;
    if (mPanel != nullptr)
        mPanel->Draw();
    if (mHelpbar != nullptr)
        mHelpbar->Draw();
    if (mInstructionsText != nullptr)
        mInstructionsText->Draw();
    FEMenu::Draw();
}

// ea: 0x721E00
void XboxLiveOptionsMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    FEMenu::ClearAllButtons();
}

// ea: 0x721E20
void XboxLiveOptionsMenu::OnUp(int a2, int c)
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

// ea: 0x721E80
void XboxLiveOptionsMenu::OnDown(int a2, int c)
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

// ea: 0x721EE0
void XboxLiveOptionsMenu::OnLeft(int c)
{
    (void)c;
    FEMenu::Left();
}

// ea: 0x721EF0
void XboxLiveOptionsMenu::OnRight(int c)
{
    (void)c;
    FEMenu::Right();
}

// ea: 0x721F00
void XboxLiveOptionsMenu::OnL1(int c)
{
    (void)c;
}

// ea: 0x721F10
void XboxLiveOptionsMenu::OnR1(int c)
{
    (void)c;
}

// ea: 0x721F20
void XboxLiveOptionsMenu::OnTriangle(int c)
{
    (void)c;
    system->ReturnToPreviousMenu(-1);
}

// ea: 0x721F30
void XboxLiveOptionsMenu::OnSquare(int c)
{
    (void)c;
}

// ea: 0x721F40
void XboxLiveOptionsMenu::OnStart(int c)
{
    (void)c;
}

// ea: 0x721F50
void XboxLiveOptionsMenu::ButtonHeldAction()
{
}

// ea: 0x721F60
void XboxLiveOptionsMenu::UpdateSplitScreen()
{
    PanelFile* mPanel = this->mPanel;
    if (mPanel != nullptr)
    {
        int viewport = 0;
        mPanel->UpdateSplitScreen(viewport, viewport);
        if (mHelpbar != nullptr)
            mHelpbar->UpdateForSplitScreen(viewport, viewport);
    }
}

// ea: 0x721FC0
void XboxLiveOptionsMenu::UpdateWidescreen(BOOL widescreen)
{
    if (mWidescreen != widescreen)
    {
        PanelFile* mPanel = this->mPanel;
        if (mPanel != nullptr)
        {
            mWidescreen = widescreen;
            mPanel->UpdateWidescreen(widescreen, 320.0f);
            if (mHelpbar != nullptr)
                mHelpbar->UpdateForWidescreen(widescreen);
        }
        if (mInstructionsText != nullptr)
            mInstructionsText->UpdateForWidescreen(widescreen);
    }
}

// ea: 0x722020
void XboxLiveOptionsMenu::WireForSignedOut()
{
    const char* szSlotText[4] = {
        "slot_01_text_a", "slot_02_text_a", "slot_03_text_a", "slot_04_text_a",
    };
    for (int i = 0; i < 4; ++i)
    {
        FEText* TextPointer =
            mPanel->GetTextPointer(szSlotText[i]);
        TextPointer->SetAlpha(1036831949);
    }
}

// ea: 0x7258C0
void XboxLiveOptionsMenu::WireForSignedIn()
{
    const char* szSlotText[4] = {
        "slot_01_text_a", "slot_02_text_a", "slot_03_text_a", "slot_04_text_a",
    };
    for (int i = 0; i < 4; ++i)
    {
        FEText* TextPointer =
            mPanel->GetTextPointer(szSlotText[i]);
        TextPointer->SetColorMenuItem(m_pOldTextColor[i].i,
                                      m_pOldSelectedTextColor[i].i);
    }
}

// ea: 0x7249E0
void XboxLiveOptionsMenu::SetPanelFile(PanelFile* pf)
{
    PanelFile* v2 = pf;
    if (pf == nullptr)
    {
        ASSERT("pf", "c:\\cod\\code\\game\\mp/ui/XboxLiveOptionsMenu.cpp", 71);
    }
    bool v4 = mVersion <= 0;
    mPanel = v2;
    if (!v4)
        mPanel = v2->Clone();

    const char* bgArtNames[3] = { "bkg", "bkg_detail_01", "bkg_detail_02" };
    for (int i = 0; i < 3; ++i)
    {
        m_pBackgroundArt[i] = mPanel->GetPointer(bgArtNames[i]);
        if (m_pBackgroundArt[i] == nullptr)
        {
            ASSERT("m_pBackgroundArt[i]",
                   "c:\\cod\\code\\game\\mp/ui/XboxLiveOptionsMenu.cpp", 92);
        }
        m_pBackgroundArt[i]->SetShown(true);
    }

    int v6 = 0;
    const char* rowTextNames[5] = {
        "text_title_01", "text_title_02", "text_current_selected",
        "text_title_instructions", "text_helpbar",
    };
    const char* bgTextRefs[5] = {
        "MPFRONTEND_PLAY_XBOX_LIVE",
        "MPFRONTEND_MM_XBOX_LIVE_OPTIONS",
        defaultFileName,
        defaultFileName,
        "MPFRONTEND_XBOX_LIVE_OPTIONS_HELPBAR",
    };
    do
    {
        m_pText[v6] = mPanel->GetTextPointer(rowTextNames[v6]);
        if (m_pText[v6] == nullptr)
        {
            ASSERT("m_pText[i]",
                   "c:\\cod\\code\\game\\mp/ui/XboxLiveOptionsMenu.cpp", 118);
        }
        const char* v7 = bgTextRefs[v6];
        if (v6 == 4)
        {
            FEMultiLineText* v49 = (FEMultiLineText*)mem_heap_malloc(0xA8);
            if (v49 != nullptr)
            {
                FEText* v50 = m_pText[4];
                unsigned int v35 = v50->GetColor().i;
                float layer = v50->GetScaleX();
                float x1 = v50->GetY();
                float v29 = v50->GetX();
                font_index v10 = v50->GetFont();
                color32 col;
                v49 = new (v49) FEMultiLineText(v10, x1, 0.0f, 1, (panel_layer)layer,
                                                0.0f, 0, (int)v35, col);
            }
            mHelpbar = v49;
            mHelpbar->SetNumLines(1);
            mHelpbar->SetText(bgTextRefs[v6]);
        }
        else
        {
            m_pText[v6]->SetText(v7);
        }
        m_pText[v6]->SetShown(true);
        v6++;
    }
    while (v6 < 5);

    if (m_pText[3] == nullptr)
    {
        ASSERT("m_pText[text_title_instructions]",
               "c:\\cod\\code\\game\\mp/ui/XboxLiveOptionsMenu.cpp", 142);
    }
    FEMultiLineText* v50 = (FEMultiLineText*)mem_heap_malloc(0xA8);
    if (v50 != nullptr)
    {
        FEText* v49 = m_pText[3];
        unsigned int v36 = v49->GetColor().i;
        float layera = v49->GetScaleX();
        float x1a = v49->GetY();
        float v30 = v49->GetX();
        font_index v15 = v49->GetFont();
        color32 col;
        union {
            unsigned int bits;
            float value;
        } instructionsScale;
        instructionsScale.bits = 32u;
        v50 = new (v50) FEMultiLineText(v15, x1a, 0.0f, 1, (panel_layer)layera,
                                        instructionsScale.value, 64,
                                        (int)v36, col);
    }
    mInstructionsText = v50;
    mInstructionsText->SetNumLines(5);

    const char* rowArtNames[5] = {
        "bkg_row_01", "bkg_row_02", "bkg_row_03", "bkg_row_04", "bkg_row_05",
    };
    for (int j = 0; j < 5; ++j)
    {
        m_pRowArt[j] = mPanel->GetPointer(rowArtNames[j]);
        if (m_pRowArt[j] == nullptr)
        {
            ASSERT("m_pRowArt[i]",
                   "c:\\cod\\code\\game\\mp/ui/XboxLiveOptionsMenu.cpp", 161);
        }
        m_pRowArt[j]->SetShown(true);
    }

    int v19 = 1;
    char szSlotGeometry[64];
    for (int k = 5; k != 0; --k)
    {
        snprintf(szSlotGeometry, 0x40, "slot_%02d_arrow_left", v19);
        mPanel->GetPointer(szSlotGeometry)->SetShown(false);
        snprintf(szSlotGeometry, 0x40, "slot_%02d_arrow_right", v19);
        mPanel->GetPointer(szSlotGeometry)->SetShown(false);
        ++v19;
    }

    const char* lineArtNames[4] = {
        "bkg_line_01", "bkg_line_02", "bkg_line_03", "bkg_line_04",
    };
    for (int m = 0; m < 4; ++m)
    {
        m_pLineArt[m] = mPanel->GetPointer(lineArtNames[m]);
        if (m_pLineArt[m] == nullptr)
        {
            ASSERT("m_pLineArt[i]",
                   "c:\\cod\\code\\game\\mp/ui/XboxLiveOptionsMenu.cpp", 190);
        }
        m_pLineArt[m]->SetShown(true);
    }

    const char* slotTextNames[5] = {
        "slot_01_text_a", "slot_02_text_a", "slot_03_text_a",
        "slot_04_text_a", "slot_05_text_a",
    };
    for (int n = 0; n < 5; ++n)
    {
        const char* v25 = slotTextNames[n];
        FEText* TextPointer = mPanel->GetTextPointer(v25);
        m_ListBox.SetItem(n, 0, TextPointer, 0);
        m_ListBox.SetText(n, 0, szXBoxOptionReferences[n]);
        FEText* v49 = mPanel->GetTextPointer(v25);
        m_pOldTextColor[n].i = v49->GetColor().i;
        FEText* v27 = mPanel->GetTextPointer(v25);
        m_pOldSelectedTextColor[n].i = v27->GetUnselectedColor().i;
    }
    m_ListBox.SetAllColumnsSelectable(false);
    if (m_ListBox.mItemColumnsCount <= 0)
    {
        ASSERT("column >= 0 && column < mItemColumnsCount",
               "c:\\cod\\code\\game\\UIListBox.h", 125);
    }
    if (m_ListBox.mSelectedRowColorChangeColumns.mSize <= 0)
    {
        ASSERT("iIndex >= 0 && iIndex < mSize",
               "../ae\\core/ae_vector.h", 167);
    }
    m_ListBox.mSelectedRowColorChangeColumns.mElements[0] = true;
    m_ListBox.mSelectedFlashing = true;
    m_ListBox.Refresh();
}

// ea: 0x725560
void XboxLiveOptionsMenu::OnActivate()
{
    FEMenu::OnActivate();
    LiveWrapper* v2 = LiveWrapper::theWrapper;
    v2->needConfirmation = false;
    if (mWidescreen != (cg_widescreen_integer != 0))
        UpdateWidescreen(cg_widescreen_integer != 0);
    friendIcon = 0;
    if (mPanel->GetPointer("game_invite") != nullptr)
        mPanel->GetPointer("game_invite")->SetShown(false);
    if (mPanel->GetPointer("friend_request") != nullptr)
        mPanel->GetPointer("friend_request")->SetShown(false);
    if (v2->internalState != kSignedIn)
    {
        WireForSignedOut();
        m_currSelection = 4;
        m_ListBox.SelectLine(4);
    }
    const char* STBString =
        STBManager::sInst->GetSTBString(
            szXBoxOptionDescriptionReferences[m_currSelection]);
    int v10 = mWidescreen ? 390 : 520;
    mInstructionsText->SetTextBoxNoLocalize(Broc::string(STBString), v10,
                                            -1.0f);
}

// ea: 0x7256A0
void XboxLiveOptionsMenu::OnCross(int c)
{
    (void)c;
    if (!waitingForSignIn)
    {
        switch (m_currSelection)
        {
        case 0:
            if (MPUIInterface::mIsViewableOnline)
                OverlayMenu::Me(0)->SetState(16 | 0x10);
            else
                OverlayMenu::Me(0)->SetState(18);
            OverlayMenu::Me(0)->mAcceptMenu = 30;
            OverlayMenu::Me(0)->mBackMenu = 30;
            system->AddOverlay(16);
            break;
        case 1:
            LiveWrapper::theWrapper->ShowFriendsList(0);
            break;
        case 2:
            LiveWrapper::theWrapper->ShowPlayersList(0, 0);
            break;
        case 3:
            OverlayMenu::Me(0)->SetState(20);
            OverlayMenu::Me(0)->mAcceptMenu = 30;
            OverlayMenu::Me(0)->mBackMenu = 30;
            system->AddOverlay(16);
            break;
        case 4:
            if (wasSignedIn)
            {
                OverlayMenu::Me(0)->SetState(17);
                OverlayMenu::Me(0)->mAcceptMenu = 30;
                OverlayMenu::Me(0)->mBackMenu = 30;
                system->AddOverlay(16);
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

// ea: 0x7257C0
void XboxLiveOptionsMenu::OnCircle(int c)
{
    (void)c;
    if (!waitingForSignIn)
        LiveWrapper::theWrapper->ShowFriendsList(0);
}

// ea: 0x7257E0
void XboxLiveOptionsMenu::UpdateDynamicText()
{
    if (m_currSelection >= 5)
    {
        ASSERT("m_currSelection < kXboxOptionsCnt",
               "c:\\cod\\code\\game\\mp/ui/XboxLiveOptionsMenu.cpp", 632);
    }
    m_pText[2]->SetText(szXBoxOptionReferences[m_currSelection]);
    const char* STBString =
        STBManager::sInst->GetSTBString(
            szXBoxOptionDescriptionReferences[m_currSelection]);
    int v5 = mWidescreen ? 390 : 520;
    mInstructionsText->SetTextBoxNoLocalize(Broc::string(STBString), v5,
                                            -1.0f);
}

// ea: 0x7261B0
void XboxLiveOptionsMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    m_ListBox.Update(time_inc);
    UpdateDynamicText();
    char* Icon = LiveWrapper::theWrapper->GetIcon(0);
    if (Icon == nullptr || LiveWrapper::theWrapper->internalState != kSignedIn)
    {
        mPanel->GetPointer("game_invite")->SetShown(false);
        goto LABEL_8;
    }
    if (Icon == ICON_GAME_INVITE)
    {
        mPanel->GetPointer("game_invite")->SetShown(true);
LABEL_8:
        mPanel->GetPointer("friend_request")->SetShown(false);
        goto LABEL_9;
    }
    if (Icon == ICON_FRIEND_REQUEST)
    {
        mPanel->GetPointer("game_invite")->SetShown(false);
        mPanel->GetPointer("friend_request")->SetShown(true);
    }
LABEL_9:
    if (waitingForSignIn && LiveWrapper::theWrapper->internalState != kSigningIn)
        waitingForSignIn = false;
    ELiveState internalState =
        (ELiveState)LiveWrapper::theWrapper->internalState;
    if (wasSignedIn != (internalState == kSignedIn))
    {
        if (internalState == kSignedIn)
            WireForSignedIn();
        else
            WireForSignedOut();
        wasSignedIn = !wasSignedIn;
    }
    if (!wasSignedIn)
        m_currSelection = 4;
    MPUIInterface::Step();
    if (mJoiningFriend)
    {
        if (!MPUIInterface::mLiveQueryActive || !MPUIInterface::mQueryFromID)
        {
              unsigned long numGames = 0;
            MPUIInterface::GameListingGet(numGames);
            if (numGames == 0)
            {
                OverlayMenu::Me(0)->SetState(23);  // JOIN_FAILED
                OverlayMenu::Me(0)->mAcceptMenu = 30;
                OverlayMenu::Me(0)->mBackMenu = 30;
            }
            else
            {
                OverlayMenu::Me(0)->SetState(24);  // JOINING_START
                OverlayMenu::Me(0)->mBackMenu = 30;
                OverlayMenu::Me(0)->mGameListingNum = 0;
            }
            system->AddOverlay(16);
            mJoiningFriend = false;
        }
    }
    else if (MPUIInterface::mLiveQueryActive && MPUIInterface::mQueryFromID)
    {
        OverlayMenu::Me(0)->SetState(16);
        OverlayMenu::Me(0)->mAcceptMenu = 30;
        OverlayMenu::Me(0)->mBackMenu = 30;
        system->AddOverlay(16);
        mJoiningFriend = true;
    }
}

// ea: 0x726EC0
void XboxLiveOptionsMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    FEMenu::Cleanup();
    FEMultiLineText* mInstructionsText = this->mInstructionsText;
    if (mInstructionsText != nullptr)
        delete mInstructionsText;
    int mVersion = this->mVersion;
    this->mInstructionsText = nullptr;
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

// ============================================================================
// InGameLiveOptionsMenu
// ============================================================================

// ea: 0x722080
InGameLiveOptionsMenu::InGameLiveOptionsMenu(FEMenuSystem* s)
    : FEMenu(s, 5, 320, 240, 8, 0)
{
    mJoiningFriend = false;
    friendIcon = 0;
    wasSignedIn = false;
    waitingForSignIn = false;
    default_color_scheme = 19;
}

// ea: 0x7220C0
InGameLiveOptionsMenu::~InGameLiveOptionsMenu()
{
    FEMenu::~FEMenu();
}

// ea: 0x722390
InGameLiveOptionsMenu* InGameLiveOptionsMenu::Me()
{
    return (InGameLiveOptionsMenu*)g_femanager.GetIGMS(currCl)->menus[15];
}

// ea: 0x7220D0
void InGameLiveOptionsMenu::OnActivate()
{
    LiveWrapper* v1 = LiveWrapper::theWrapper;
    if (v1 != nullptr)
    {
        friendIcon = 0;
        panel->GetPointer("game_invite")->SetVisibility(0);
        panel->GetPointer("friend_request")->SetVisibility(0);
        wasSignedIn = v1->internalState == kSignedIn;
        FEMenu::OnActivate();
        if ((v1->localPlayers[0].notificationFlags & 1) != 0)
            entries[0]->SetText("FEMENU_APPEAR_ONLINE");
        else
            entries[0]->SetText("FEMENU_APPEAR_OFFLINE");
        int voice = (int)v1->localPlayers[0].voiceStatus;
        if (voice != 0)
        {
            if (voice == UIX_VOICE_STATUS_SPEAKERS)
                entries[3]->SetText("VOICE_OUTPUT_SPEAKERS");
            else
                entries[3]->SetText("VOICE_OUTPUT_NONE");
        }
        else
        {
            entries[3]->SetText("VOICE_OUTPUT_HEADSET");
        }
    }
    else
    {
        ASSERT("engine", "c:\\cod\\code\\game\\mp/ui/InGameLiveOptionsMenu.cpp", 40);
        ASSERT("wrapper", "c:\\cod\\code\\game\\mp/ui/InGameLiveOptionsMenu.cpp", 41);
    }
}

// ea: 0x722270
void InGameLiveOptionsMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    movie_manager::render();
    FEMenu::Draw();
}

// ea: 0x722290
bool InGameLiveOptionsMenu::ResponseYesReboot(int)
{
    LiveWrapper::theWrapper->renderingEnabled = true;
    LiveEngine_Reboot(LiveWrapper::theWrapper->uixEngine, 0);
    return 1;
}

// ea: 0x7222B0
bool InGameLiveOptionsMenu::ResponseDoNothing(int)
{
    return 1;
}

// ea: 0x726400
bool InGameLiveOptionsMenu::ResponseNoReboot(int)
{
    LiveWrapper* v0 = LiveWrapper::theWrapper;
    v0->renderingEnabled = true;
    v0->PreLogoff();
    LiveEngine_LogOff(v0->uixEngine);
    v0->internalState = kNotSignedIn;
    LiveEngine_LogOff(v0->uixEngine);
    v0->UpdateLocalPlayers();
    v0->LogoffCallBack();
    v0->sessionState = kNotInSession;
    return 1;
}

// ea: 0x726450
bool InGameLiveOptionsMenu::ResponseSignOut(int)
{
    LiveWrapper* v0 = LiveWrapper::theWrapper;
    v0->PreLogoff();
    LiveEngine_LogOff(v0->uixEngine);
    v0->internalState = kNotSignedIn;
    LiveEngine_LogOff(v0->uixEngine);
    v0->UpdateLocalPlayers();
    v0->LogoffCallBack();
    v0->sessionState = kNotInSession;
    return 1;
}

// ea: 0x7222C0
void InGameLiveOptionsMenu::OnTriangle(int c)
{
    (void)c;
    if (!waitingForSignIn)
        system->ReturnToPreviousMenu(-1);
}

// ea: 0x7222E0
void InGameLiveOptionsMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    if (pf == nullptr)
    {
        ASSERT("panel", "c:\\cod\\code\\game\\mp/ui/InGameLiveOptionsMenu.cpp", 309);
    }
    entries[1]->SetText("MPFRONTEND_FRIENDS_LIST");
    entries[0]->SetText("MPFRONTEND_APPEAR_OFFLINE");
    entries[4]->SetText("MPFRONTEND_SIGN_OUT");
    entries[1]->up = 4;
    entries[4]->down = 1;
}

// ea: 0x722380
void InGameLiveOptionsMenu::UpdateWidescreen(bool widescreen)
{
    (void)widescreen;
}

// ea: 0x7259D0
void InGameLiveOptionsMenu::TryFriendJoin()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_CONFIRM_FRIEND_JOIN", false, false, defaultFileName, true);
    DialogMenu* Layer = g_femanager.GetDMS(currCl)->GetLayer(
        g_femanager.GetDMS(currCl)->GetActiveMenu() == 0);
    Layer->AddOption("INGAME_DIALOG_YES", ResponseYesJoin);
    Layer = g_femanager.GetDMS(currCl)->GetLayer(
        g_femanager.GetDMS(currCl)->GetActiveMenu() == 0);
    Layer->AddOption("INGAME_DIALOG_NO", ResponseNoJoin);
    g_femanager.GetDMS(currCl)->HighlightOption(1);
    g_femanager.GetDMS(currCl)->GetLayer(
        g_femanager.GetDMS(currCl)->GetActiveMenu() == 0)->Reformat(true, 0);
}

// ea: 0x726830
void InGameLiveOptionsMenu::TryReboot()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_BOOT_DASH", false, false, defaultFileName, true);
    DialogMenu* Layer = g_femanager.GetDMS(currCl)->GetLayer(
        g_femanager.GetDMS(currCl)->GetActiveMenu() == 0);
    Layer->AddOption("INGAME_DIALOG_YES", ResponseYesReboot);
    Layer = g_femanager.GetDMS(currCl)->GetLayer(
        g_femanager.GetDMS(currCl)->GetActiveMenu() == 0);
    Layer->AddOption("INGAME_DIALOG_NO", ResponseNoReboot);
    g_femanager.GetDMS(currCl)->HighlightOption(1);
    g_femanager.GetDMS(currCl)->GetLayer(
        g_femanager.GetDMS(currCl)->GetActiveMenu() == 0)->Reformat(true, 0);
    LiveWrapper::theWrapper->renderingEnabled = false;
}

// ea: 0x726940
void InGameLiveOptionsMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    movie_manager::frame_advance();
    PanelQuad* Pointer =
        panel->GetPointer("friend_request");
    PanelQuad* v3 = panel->GetPointer("game_invite");
    ShowNotificationIcon(&friendIcon, v3, Pointer);
    if (waitingForSignIn && LiveWrapper::theWrapper->internalState != kSigningIn)
        waitingForSignIn = false;
    ELiveNotification lastNotification =
        (ELiveNotification)LiveWrapper::theWrapper->lastNotification;
    if (lastNotification == kConfirmReboot)
    {
        LiveWrapper::theWrapper->lastNotification = kLiveOk;
        TryReboot();
    }
    else if (lastNotification == kConfirmFriendJoin)
    {
        LiveWrapper::theWrapper->lastNotification = kLiveOk;
        TryFriendJoin();
    }
    bool signedIn = LiveWrapper::theWrapper->internalState == kSignedIn;
    if (wasSignedIn != signedIn)
    {
        entries[1]->Disable(wasSignedIn);
        entries[0]->Disable(wasSignedIn);
        entries[2]->Disable(wasSignedIn);
        entries[3]->Disable(wasSignedIn);
        if (wasSignedIn)
        {
            entries[4]->SetText("FEMENU_SIGN_IN");
            entries[0]->SetText("FEMENU_APPEAR_OFFLINE");
            entries[3]->SetText("VOICE_OUTPUT_NONE");
            FEMenu::SetHigh(4, true);
        }
        else
        {
            entries[4]->SetText("FEMENU_SIGN_OUT");
            entries[0]->SetText("FEMENU_APPEAR_ONLINE");
            entries[3]->SetText("VOICE_OUTPUT_SPEAKERS");
            FEMenu::SetHigh(1, true);
        }
        wasSignedIn = !wasSignedIn;
    }
}

// ea: 0x726AB0
void InGameLiveOptionsMenu::Select(int a2, int entry_num)
{
    (void)a2;
    if (!waitingForSignIn)
    {
        LiveWrapper* v4 = LiveWrapper::theWrapper;
        if (v4 != nullptr)
        {
            switch (entry_num)
            {
            case 0:
                LiveWrapper::theWrapper->ToggleOfflineAppearance(0);
                if (v4->GetNotificationFlag(0, 1))
                    entries[0]->SetText("FEMENU_APPEAR_ONLINE");
                else
                    entries[0]->SetText("FEMENU_APPEAR_OFFLINE");
                break;
            case 1:
                LiveWrapper::theWrapper->ShowFriendsList(0);
                break;
            case 2:
                LiveWrapper::theWrapper->ShowPlayersList(0, 0);
                break;
            case 3:
                LiveWrapper::theWrapper->ToggleVTS(
                                       LiveWrapper::theWrapper->activeController);
                {
                    LivePlayer* LocalPlayer =
                        v4->GetLocalPlayer(0);
                    if (LocalPlayer != nullptr)
                    {
                        int voice = (int)LocalPlayer->voiceStatus;
                        if (voice != 0)
                        {
                            if (voice == UIX_VOICE_STATUS_SPEAKERS)
                                entries[3]->SetText("VOICE_OUTPUT_SPEAKERS");
                            else
                                entries[3]->SetText("VOICE_OUTPUT_NONE");
                        }
                        else
                        {
                            entries[3]->SetText("VOICE_OUTPUT_HEADSET");
                        }
                    }
                }
                break;
            case 4:
                if (v4->internalState == kSignedIn)
                {
                    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
                    DMS->BringUp("MEM_SIGN_OUT", false, false,
                                 defaultFileName, true);
                    g_femanager.GetDMS(currCl)->AddOption(
                        "MEM_DIALOG_YES", ResponseSignOut);
                    g_femanager.GetDMS(currCl)->AddOption(
                        "MEM_DIALOG_NO", ResponseDoNothing);
                    g_femanager.GetDMS(currCl)->HighlightOption(1);
                    g_femanager.GetDMS(currCl)->Reformat(true);
                }
                else
                {
                    LiveWrapper::theWrapper->ShowLoginScreen(0x40140);
                    waitingForSignIn = true;
                }
                break;
            default:
                ASSERT("0", "c:\\cod\\code\\game\\mp/ui/InGameLiveOptionsMenu.cpp",
                       275);
                break;
            }
        }
        else
        {
            ASSERT("engine", "c:\\cod\\code\\game\\mp/ui/InGameLiveOptionsMenu.cpp",
                   209);
            ASSERT("wrapper", "c:\\cod\\code\\game\\mp/ui/InGameLiveOptionsMenu.cpp",
                   210);
        }
    }
}

// ============================================================================
// Free functions
// ============================================================================

// ea: 0x7242D0
void ShowNotificationIcon(unsigned int* menuIcon, PanelQuad* inviteQuad,
                          PanelQuad* friendQuad)
{
    if (LiveWrapper::theWrapper->internalState == kSignedIn)
    {
        char* Icon = LiveWrapper::theWrapper->GetIcon(0);
        char* v4 = (char*)*menuIcon;
        if (Icon != v4)
        {
            if (v4 == ICON_GAME_INVITE)
                inviteQuad->SetVisibility(0);
            else if (v4 == ICON_FRIEND_REQUEST)
                friendQuad->SetVisibility(0);
            if (Icon == ICON_GAME_INVITE)
            {
                inviteQuad->SetVisibility(1065353216);
                *menuIcon = (unsigned int)Icon;
            }
            else
            {
                if (Icon == ICON_FRIEND_REQUEST)
                    friendQuad->SetVisibility(1065353216);
                *menuIcon = (unsigned int)Icon;
            }
        }
    }
}

// ea: 0x7223B0
char* Xbox_LaunchInfo(char* pDestCommandLine)
{
    gSkipFrontEnd = false;
    gSkipMovies = false;
    for (int i = 0; i < 4; ++i)
        gSaveGameData[i].mDisableSave = false;
    unsigned int Type = 0;
    _LAUNCH_DATA Data;
    if (XGetLaunchInfo(&Type, &Data) == 0)
    {
        printf("Launch type : %d (dashboard=%d, debugger=%d)\n", Type, 2, 3);
        *pDestCommandLine = 0;
        if (Type == 3)
        {
            strcpy(pDestCommandLine, (const char*)Data.raw);
            return nullptr;
        }
        if (Type == 0)
        {
            tlPrintf("!! retrieving stub data !!\n");
            tlPrintf("!! Live state is: %d !!\n",
                     gSaveGameData[controller::inst()->locked_port].liveState);
            gSkipMovies = true;
        }
    }
    return nullptr;
}

// ea: 0x725AE0
void RenderUIX(void*)
{
    D3DDevice_SetVertexShader(0);
    D3DDevice_SetPixelShaderProgram(nullptr);
    D3DDevice_SetVertexShaderInputDirect(nullptr, 0, nullptr);
    D3DDevice_SetIndices(nullptr, 0);
    nglDxUnbindTexStages();
    if (LiveWrapper::theWrapper != nullptr
        && !g_controllerConnectedErrorShown[LocalClient::ClientToPort(currCl)])
    {
        LiveWrapper::theWrapper->Render();
    }
    nglDxCheckErrorD3D(0, "c:\\cod\\code\\game\\xbox_main.cpp", 534);
    nglDxState.Init();
    nglDxInitShaders(false);
}

XboxLiveOptionsMenu* XboxLiveOptionsMenu_ctor(void* mem, FEMenuSystem* s)
{
    return new (mem) XboxLiveOptionsMenu(s);
}

InGameLiveOptionsMenu* InGameLiveOptionsMenu_ctor(void* mem, FEMenuSystem* s)
{
    return new (mem) InGameLiveOptionsMenu(s);
}
