// ============================================================================
// dialog_menu.cpp - DialogMenuSystem / DialogMenu / DialogMenuDisplay
// ============================================================================

#include "game/shell/shell_types.h"

#include <string.h>
#include <stdio.h>

extern FEManager g_femanager;          // ?g_femanager@@3UFEManager@@A
extern int currCl;                     // ?currCl@@3HA @ 0xF1579C
extern const char* const defaultFileName;  // ?defaultFileName
extern void* mem_heap_malloc(int alignment, unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);  // core.o
extern float unk_F6A284[];             // @ 0xF6A284
extern float unk_F6A280[];             // @ 0xF6A280
extern int unk_F6A28C[];               // @ 0xF6A28C
extern int dword_F64158[];             // @ 0xF64158
extern float g_time_inc;               // ?g_time_inc@@3MA
extern bool gStillDrawMenus;           // ?gStillDrawMenus@@3_NA
extern bool g_controllerConnectedErrorShown[];  // ?g_controllerConnectedErrorShown@@3PA_NA
enum FULLSCREENBLUR_STATE {
    FULLSCREENBLUR_OFF = 0,
    FULLSCREENBLUR_START = 1,
    FULLSCREENBLUR_RUNNING = 2,
    FULLSCREENBLUR_FINISHED = 3,
    FULLSCREENBLUR_THISFRAMEONLY = 4,
};
extern FULLSCREENBLUR_STATE g_doFullScreenBlur[];
extern float g_fullScreenBlurAmount[]; // ?g_fullScreenBlurAmount@@3PAMA
float blur_amount_2 = 0.1f;            // @ 0xDF4458
extern int gDelayRenderForNFrames;     // ?gDelayRenderForNFrames@@3HA
extern int Sys_Milliseconds();         // ?Sys_Milliseconds@@YAHXZ

namespace View {
extern void SetViewportClipping(int clientIndex);  // cg.o
}
namespace LocalClient {
extern int ClientToPort(int client);  // ?ClientToPort@LocalClient@@YAHH@Z
}

// STBManager minimal view
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
};

// MPUIInterface statics
class MPUIInterface {
public:
    static bool mCableDisconnect;  // ?mCableDisconnect@MPUIInterface@@1_NA
    static int  mReturnMenu;       // ?mReturnMenu@MPUIInterface@@1HA
    static void ExitGame();        // ?ExitGame@MPUIInterface@@SAXXZ
};

bool MPUIInterface::mCableDisconnect = false;

// ============================================================================
// DialogMenuDisplay data
// ============================================================================
const char* const DialogMenuDisplay::kMenuOptionGeoms[] = {
    "text_option_01",
    "text_option_02",
};
const char* const DialogMenuDisplay::kMenuLineGeoms[] = {
    "bkg_line_01",
};

// ============================================================================
// DialogMenuDisplay
// ============================================================================

// ea: 0x005723B0
DialogMenuDisplay::DialogMenuDisplay(int client)
{
    mPanel = nullptr;
    mSplitScreenMenu = nullptr;
    mDialogText = nullptr;
    mSplitScreenDialogText = nullptr;
    mSplitScreenDialogTitle = nullptr;
    mOptionCount = 0;
    mOptionSelected = -1;
    mViewport = 0;
    mIsClosing = false;
    mClient = client;
    mWidescreen = false;
    mMenuOptions[0] = nullptr;
    mSplitScreenMenuOptions[0] = nullptr;
    mMenuOptions[1] = nullptr;
    mSplitScreenMenuOptions[1] = nullptr;
    mMenuLines[0] = nullptr;
}

// ea: 0x005922A0
DialogMenuDisplay::~DialogMenuDisplay()
{
    if (mDialogText != nullptr)
        mDialogText = nullptr;
    if (mPanel != nullptr)
    {
        if (mClient > 0)
        {
            delete mPanel;
        }
        mPanel = nullptr;
    }
}

// ea: 0x0059AE00
void DialogMenuDisplay::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_textbox.PANEL") == 0)
    {
        SetPanelFileSplitScreen(pf);
    }
    else if (_stricmp(pf->mName, "SP_small_textbox_ingame.PANEL") == 0)
    {
        SetPanelFileMain(pf);
    }
}

// ea: 0x005922E0
void DialogMenuDisplay::PanelFileUnloaded(PanelFile* pf)
{
    if (pf == mPanel)
    {
        if (mClient > 0 && mPanel != nullptr)
        {
            delete mPanel;
        }
        mPanel = nullptr;
    }
    if (pf == mSplitScreenMenu)
    {
        if (mSplitScreenMenu != nullptr)
        {
            delete mSplitScreenMenu;
        }
        mSplitScreenMenu = nullptr;
    }
}

// ea: 0x0057EE70
void DialogMenuDisplay::UpdateWidescreen(bool ws)
{
    if (mWidescreen != ws)
    {
        if (mPanel != nullptr)
        {
            mWidescreen = ws;
            mPanel->UpdateWidescreen(ws, 320.0f);
            if (mSplitScreenMenu != nullptr)
                mSplitScreenMenu->UpdateWidescreen(ws, 320.0f);
        }
    }
}

// ea: 0x00572400
void DialogMenuDisplay::SetText(const char* text)
{
    int v5 = mDialogText->GetBoxWidth();
    mDialogText->SetTextBox(text, v5, -1.0f);
    if (mSplitScreenDialogText != nullptr)
    {
        int v8 = mSplitScreenDialogText->GetBoxWidth();
        mSplitScreenDialogText->SetTextBox(text, v8, -1.0f);
    }
}

// ea: 0x00572460
void DialogMenuDisplay::SetTitle(const char* text)
{
    if (mSplitScreenDialogTitle != nullptr)
        mSplitScreenDialogTitle->SetText(text);
}

// ea: 0x00572480
void DialogMenuDisplay::OnActivate()
{
    mIsClosing = false;
    mOptionCount = 0;
    mOptionSelected = -1;
}

// ea: 0x00572490
DialogMenuDisplay* DialogMenuDisplay::Me(int viewport)
{
    return g_femanager.GetDMS(viewport)->mDisplay;
}

// ea: 0x005724B0
void DialogMenuDisplay::AddOption(const char* text)
{
    mOptionText[mOptionCount++] = text;
}

// ea: 0x005725D0
void DialogMenuDisplay::Reformat()
{
    for (int i = 0; i < 2; ++i)
    {
        if (mMenuOptions[i] != nullptr)
            mMenuOptions[i]->SetText(defaultFileName);
        if (mSplitScreenMenuOptions[i] != nullptr)
            mSplitScreenMenuOptions[i]->SetText(defaultFileName);
    }
    if (mMenuLines[0] != nullptr)
        mMenuLines[0]->SetShown(false);
    for (int v6 = 0; v6 < mOptionCount; ++v6)
    {
        mMenuOptions[v6]->SetText(mOptionText[v6]);
        if (mSplitScreenMenuOptions[v6] != nullptr)
            mSplitScreenMenuOptions[v6]->SetText(mOptionText[v6]);
        if (v6 > 0)
            mMenuLines[0]->SetShown(true);
    }
    if (mOptionSelected == -1)
        mOptionSelected = 0;
}

// ea: 0x0057ED70
void DialogMenuDisplay::Update(float time_inc)
{
    for (int i = 0; i < 2; ++i)
    {
        if (mMenuOptions[i] != nullptr)
            mMenuOptions[i]->Update(time_inc);
        if (mSplitScreenMenuOptions[i] != nullptr)
            mSplitScreenMenuOptions[i]->Update(time_inc);
    }
    if (mIsClosing)
    {
        int mClient = this->mClient;
        if (mClient != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.h";
            AeAssert::gCurrentLine = 147;
            AeAssert::gCurrentExpr = "client >= 0 && client < 1";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid client index for dms"))
                __debugbreak();
        }
        DialogMenuSystem* v6 = g_femanager.mDMS[mClient];
        v6->MakeActive(-1);
        if ((v6->flags & 1) == 0)
        {
            SoundDevice::sInst->UnpauseAllSounds();
            GamePause::SetGamePaused(v6->mClient, false);
        }
        if (!v6->mWasIGMSUpWhenLaunched)
        {
            InGameMenuSystem* IGMS = g_femanager.GetIGMS(currCl);
            IGMS->MakeActive(-1);
        }
        v6->flags &= ~4u;
    }
}

// ea: 0x0057EEB0
void DialogMenuDisplay::Draw()
{
    if (mIsClosing)
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(mClient);
        DMS->OnFinish();
        mIsClosing = false;
    }
    else if (mViewport != 0)
    {
        if (mSplitScreenMenu != nullptr)
        {
            if (FESplitScreenMenu::mBackground != nullptr)
                FESplitScreenMenu::mBackground->Draw();
            mSplitScreenMenu->Draw();
            mSplitScreenDialogText->Draw(false);
            mSplitScreenDialogTitle->Draw();
            for (int v7 = 0; v7 < 2; ++v7)
            {
                if (mSplitScreenMenuOptions[v7] != nullptr)
                    mSplitScreenMenuOptions[v7]->Draw(
                        v7 == mOptionSelected);
            }
        }
    }
    else
    {
        mPanel->Draw();
        mDialogText->Draw(false);
        for (int v4 = 0; v4 < 2; ++v4)
        {
            if (mMenuOptions[v4] != nullptr)
                mMenuOptions[v4]->Draw(v4 == mOptionSelected);
        }
    }
}

// ea: 0x0057EFF0
void DialogMenuDisplay::OnUp()
{
    if (mOptionCount == 1)
    {
        mOptionSelected = 0;
    }
    else if (mOptionCount == 2)
    {
        int v1 = mOptionSelected == 0;
        mOptionSelected = v1;
        SetDialogFlash(v1);
        return;
    }
    SetDialogFlash(mOptionSelected);
}

// ea: 0x0057F020
void DialogMenuDisplay::OnDown()
{
    if (mOptionCount == 1)
    {
        mOptionSelected = 0;
    }
    else if (mOptionCount == 2)
    {
        int v1 = mOptionSelected == 0;
        mOptionSelected = v1;
        SetDialogFlash(v1);
        return;
    }
    SetDialogFlash(mOptionSelected);
}

// ea: 0x0057F050
void DialogMenuDisplay::HighlightEntry(int entryNum)
{
    if (mOptionCount > 0 && mOptionCount <= 2)
        mOptionSelected = entryNum;
    SetDialogFlash(entryNum);
}

// ea: 0x0057F070
void DialogMenuDisplay::UpdateSplitScreen()
{
    if (mSplitScreenMenu != nullptr)
    {
        int v2 = 3208 * mClient;
        mViewport = unk_F6A284[v2];
        mSplitScreenMenu->MoveSplitScreen(unk_F6A284[v2], unk_F6A280[v2]);
    }
}

// ea: 0x005724D0
void DialogMenuDisplay::SetDialogFlash(int entryNum)
{
    for (int i = 0; i < mOptionCount; ++i)
    {
        if (mMenuOptions[i] != nullptr)
        {
            if (i == entryNum)
            {
                mMenuOptions[i]->SetColorMenuItem(
                    mMenuOptions[i]->GetUnselectedColor(),
                    mMenuOptions[i]->GetColor());
            }
            else
            {
                mMenuOptions[i]->SetNoFlash(
                    mMenuOptions[i]->GetUnselectedColor());
            }
        }
        if (mOptionCount < 3)
        {
            if (mSplitScreenMenuOptions[i] != nullptr)
            {
                if (i == entryNum)
                {
                    mSplitScreenMenuOptions[i]->SetFlash(
                        mSplitScreenMenuOptions[i]->GetUnselectedColor(),
                        mSplitScreenMenuOptions[i]->GetColor(), 1.0f);
                }
                else
                {
                    mSplitScreenMenuOptions[i]->SetNoFlash(
                        mSplitScreenMenuOptions[i]->GetUnselectedColor());
                }
            }
        }
    }
}

// ea: 0x0057EF80
void DialogMenuDisplay::SetPanelFileSplitScreen(PanelFile* pf)
{
    mSplitScreenMenu = pf;
    mSplitScreenDialogText =
        (FEMultiLineText*)pf->GetTextPointer("text_body");
    mSplitScreenDialogTitle = mSplitScreenMenu->GetTextPointer("text_title");
    for (int i = 0; i < 2; ++i)
    {
        FEText* v7 = mSplitScreenMenu->GetTextPointer(
            kMenuOptionGeoms[i]);
        mSplitScreenMenuOptions[i] = v7;
        if (v7 != nullptr)
            v7->AddedToMenu(true);
    }
}

// ea: 0x00596030
void DialogMenuDisplay::SetPanelFileMain(PanelFile* pf)
{
    PanelFile* v3 = mClient <= 0 ? pf : pf->Clone();
    mPanel = v3;
    mDialogText = (FEMultiLineText*)v3->GetTextPointer("text_body");
    for (int i = 0; i < 2; ++i)
    {
        FEText* TextPointer = mPanel->GetTextPointer(kMenuOptionGeoms[i]);
        mMenuOptions[i] = TextPointer;
        if (TextPointer != nullptr)
            TextPointer->AddedToMenu(true);
    }
    mMenuLines[0] = mPanel->GetPointer("bkg_line_01");
}

// ============================================================================
// DialogMenu
// ============================================================================

// ea: 0x00592340
DialogMenu::DialogMenu(FEMenuSystem* s)
    : FEMenu(s, 4, 0, 0, 8, 0)
{
    delayResponse = nullptr;
    message = Broc::string((Broc::string::Block*)nullptr);
    title = Broc::string((Broc::string::Block*)nullptr);
    flags = (int16_t)(flags | 0x82);
    mDelayMs = 0;
    default_color_scheme = 19;
    for (int i = 0; i < 4; ++i)
        AddEntry(i, defaultFileName);
    entries[0]->up = 3;
    entries[3]->down = 0;
    flags = (int16_t)(flags | 0x40);
    cur_index = 0;
    triangleResponse = nullptr;
    mClient = 0;
}

// ea: 0x00572BE0
void DialogMenu::OnActivate()
{
    FEMenu::OnActivate();
}

// ea: 0x00572BF0
void DialogMenu::Draw()
{
}

// ea: 0x00572C00
void DialogMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
    if (delayResponse != nullptr && Sys_Milliseconds() >= mDelayMs)
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
        DMS->CloseDialog();
        delayResponse(mClient);
        delayResponse = nullptr;
        mDelayMs = 0;
    }
}

// ea: 0x00572C60
void DialogMenu::UpdateWidescreen(bool ws)
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->mDisplay->UpdateWidescreen(ws);
}

// ea: 0x00572C80
void DialogMenu::Select(int entry_num)
{
    if (entry_num < cur_index && optionResponses[entry_num](mClient))
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(mClient);
        DMS->CloseDialog();
    }
}

// ea: 0x00572CC0
void DialogMenu::OnTriangle(int c)
{
    (void)c;
    if (triangleResponse != nullptr)
    {
        triangleResponse(mClient);
    }
    else
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(mClient);
        DMS->CloseDialog();
    }
}

// ea: 0x00572CF0
void DialogMenu::BringUp(Broc::string& mess, Broc::string& t, int client)
{
    title = t;
    message = mess;
    cur_index = 0;
    triangleResponse = nullptr;
    delayResponse = nullptr;
    mClient = client;
    for (int i = 0; i < 4; ++i)
        entries[i]->Disable(true);
    if (mClient != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.h";
        AeAssert::gCurrentLine = 147;
        AeAssert::gCurrentExpr = "client >= 0 && client < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid client index for dms"))
            __debugbreak();
    }
    DialogMenuDisplay* mDisplay = g_femanager.mDMS[mClient]->mDisplay;
    mDisplay->mIsClosing = false;
    mDisplay->mOptionCount = 0;
    mDisplay->mOptionSelected = -1;
    const char* messa = mess.mBlock != nullptr
                            ? (const char*)&mess.mBlock[1]
                            : defaultFileName;
    if (mClient != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.h";
        AeAssert::gCurrentLine = 147;
        AeAssert::gCurrentExpr = "client >= 0 && client < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid client index for dms"))
            __debugbreak();
    }
    DialogMenuDisplay* v9 = g_femanager.mDMS[mClient]->mDisplay;
    int v11 = v9->mDialogText->GetBoxWidth();
    v9->mDialogText->SetTextBox(messa, v11, -1.0f);
    if (v9->mSplitScreenDialogText != nullptr)
    {
        int v14 = v9->mSplitScreenDialogText->GetBoxWidth();
        v9->mSplitScreenDialogText->SetTextBox(messa, v14, -1.0f);
    }
    const char* v15 = t.mBlock != nullptr ? (const char*)&t.mBlock[1]
                                          : defaultFileName;
    if (mClient != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.h";
        AeAssert::gCurrentLine = 147;
        AeAssert::gCurrentExpr = "client >= 0 && client < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid client index for dms"))
            __debugbreak();
    }
    FEText* mSplitScreenDialogTitle =
        g_femanager.mDMS[mClient]->mDisplay->mSplitScreenDialogTitle;
    if (mSplitScreenDialogTitle != nullptr)
        mSplitScreenDialogTitle->SetText(v15);
}

// ea: 0x00572F60
void DialogMenu::CloseOnDelay(int delaySeconds, void (*delayResp)(int))
{
    mDelayMs = 1000 * delaySeconds + Sys_Milliseconds();
    delayResponse = delayResp;
}

// ea: 0x00572F90
void DialogMenu::UpdateSplitScreen()
{
}

// ea: 0x00572FA0
void DialogMenu::ButtonHeldAction()
{
    if (button_held_down == 4)
    {
        OnUp(0);
    }
    else if (button_held_down == 8)
    {
        OnDown(0);
    }
}

// ea: 0x0057F270
void DialogMenu::OnUp(int c)
{
    (void)c;
    Up();
    DialogMenuDisplay* mDisplay = g_femanager.GetDMS(mClient)->mDisplay;
    if (mDisplay->mOptionCount == 1)
    {
        mDisplay->mOptionSelected = 0;
    }
    else if (mDisplay->mOptionCount == 2)
    {
        int v4 = mDisplay->mOptionSelected == 0;
        mDisplay->mOptionSelected = v4;
        mDisplay->SetDialogFlash(v4);
        return;
    }
    mDisplay->SetDialogFlash(mDisplay->mOptionSelected);
}

// ea: 0x0057F2D0
void DialogMenu::OnDown(int c)
{
    (void)c;
    Down();
    DialogMenuDisplay* mDisplay = g_femanager.GetDMS(mClient)->mDisplay;
    if (mDisplay->mOptionCount == 1)
    {
        mDisplay->mOptionSelected = 0;
    }
    else if (mDisplay->mOptionCount == 2)
    {
        int v4 = mDisplay->mOptionSelected == 0;
        mDisplay->mOptionSelected = v4;
        mDisplay->SetDialogFlash(v4);
        return;
    }
    mDisplay->SetDialogFlash(mDisplay->mOptionSelected);
}

// ea: 0x00572EE0
void DialogMenu::AddOption(const char* t, bool (*responseFunc)(int))
{
    entries[cur_index]->SetText(t);
    optionResponses[cur_index] = responseFunc;
    entries[cur_index]->Disable(false);
    ++cur_index;
    DialogMenuDisplay* mDisplay = g_femanager.GetDMS(mClient)->mDisplay;
    mDisplay->mOptionText[mDisplay->mOptionCount++] = t;
}

// ea: 0x00572F40
void DialogMenu::Reformat(bool vertical, int viewport)
{
    (void)vertical; (void)viewport;
    DialogMenuSystem* DMS = g_femanager.GetDMS(mClient);
    DMS->mDisplay->Reformat();
}

// ============================================================================
// DialogMenuSystem
// ============================================================================

// ea: 0x005940C0
DialogMenuSystem::DialogMenuSystem(int client)
    : FEMenuSystem(2, FONT_GARAMOND)
{
    flags = 0;
    mClient = client;
    mWasIGMSUpWhenLaunched = false;
    default_color_scheme = 19;
    DialogMenuDisplay* v3 =
        (DialogMenuDisplay*)mem_heap_malloc(16, 0x4Cu);
    if (v3 != nullptr)
        mDisplay = new (v3) DialogMenuDisplay(client);
    else
        mDisplay = nullptr;
    DialogMenu* v5 = (DialogMenu*)mem_heap_malloc(16, 0x78u);
    if (v5 != nullptr)
        FEMenuSystem::Add(new (v5) DialogMenu(this));
    else
        FEMenuSystem::Add(nullptr);
    DialogMenu* v7 = (DialogMenu*)mem_heap_malloc(16, 0x78u);
    if (v7 != nullptr)
        FEMenuSystem::Add(new (v7) DialogMenu(this));
    else
        FEMenuSystem::Add(nullptr);
    mState = DMS_STATE_NONE;
}

DialogMenuSystem* DialogMenuSystem_ctor(void* mem, int client)
{
    return new (mem) DialogMenuSystem(client);
}

// ea: 0x005941D0
DialogMenuSystem::~DialogMenuSystem()
{
    if (mDisplay != nullptr)
    {
        mDisplay->mDialogText = nullptr;
        if (mDisplay->mPanel != nullptr)
        {
            if (mDisplay->mClient > 0)
            {
                delete mDisplay->mPanel;
            }
            mDisplay->mPanel = nullptr;
        }
        mem_heap_free(mDisplay);
    }
    mDisplay = nullptr;
    for (int v5 = 0; v5 < 2; ++v5)
    {
        if (menus[v5] != nullptr)
        {
            delete menus[v5];
            menus[v5] = nullptr;
        }
    }
    mem_heap_free(menus);
    menus = nullptr;
    FEMenuSystem::~FEMenuSystem();
}

// ea: 0x0057F0B0
void DialogMenuSystem::Update(float time_inc)
{
    if (mState == DMS_SIGNOUT_CONFIRMATION
        && g_controllerConnectedErrorShown[
            LocalClient::ClientToPort(currCl)])
    {
        CloseDialog();
        mState = DMS_STATE_NONE;
    }
    else
    {
        if (m_active >= 0)
        {
            FEMenu* v4 = menus[m_active];
            if (v4 != nullptr)
            {
                v4->Update(time_inc);
                UpdateButtonPresses();
            }
        }
        mDisplay->Update(time_inc);
        if (g_femanager.fems != nullptr)
            SetDefaultColorScheme(1);
        else
            SetDefaultColorScheme(4);
        g_doFullScreenBlur[mClient] = FULLSCREENBLUR_THISFRAMEONLY;
        g_fullScreenBlurAmount[mClient] = blur_amount_2;
    }
}

// ea: 0x0057F160
void DialogMenuSystem::OnButtonPress(int b, int c)
{
    if (b == 2)
    {
        if (mState == 2)
        {
            g_controllerConnectedErrorShown[c] = false;
            CloseDialog();
            mState = DMS_STATE_NONE;
        }
    }
    else
    {
        FEMenuSystem::OnButtonPress(b, c);
    }
}

// ea: 0x00572680
void DialogMenuSystem::OnButtonRelease(int b, int c)
{
    if (m_active >= 0)
    {
        if (menus[m_active] != nullptr)
            menus[m_active]->OnButtonRelease(c, b);
    }
}

// ea: 0x0057F1F0
void DialogMenuSystem::MakeActive(int index)
{
    if (GetActiveMenu() < 0)
    {
        mDisplay->mIsClosing = false;
        mDisplay->mOptionCount = 0;
        mDisplay->mOptionSelected = -1;
    }
    FEMenuSystem::MakeActive(index, mClient);
}

// ea: 0x0057F230
void DialogMenuSystem::UpdateSplitScreen()
{
    if (mDisplay->mSplitScreenMenu != nullptr)
    {
        int v3 = 3208 * mDisplay->mClient;
        mDisplay->mViewport = unk_F6A284[v3];
        mDisplay->mSplitScreenMenu->MoveSplitScreen(unk_F6A284[v3],
                                                    unk_F6A280[v3]);
    }
}

// ea: 0x00586570
void DialogMenuSystem::Draw()
{
    if (dword_F64158[1580 * mClient] >= 1.0f)
    {
        if (!gStillDrawMenus)
            return;
        gStillDrawMenus = false;
    }
    switch (mState)
    {
    case 1:
        CountDown(g_time_inc);
        break;
    case 2:
        Update(g_time_inc);
        break;
    case 3:
        mCountDown = mCountDown - 1.0f;
        if (mCountDown < 0.0f)
        {
            mState = DMS_STATE_NONE;
            CloseDialog();
        }
        return;
    default:
        break;
    }
    View::SetViewportClipping(mClient);
    if (background >= 0)
    {
        drawHelpbar = false;
        menus[background]->Draw();
        drawHelpbar = true;
    }
    if (m_active >= 0)
    {
        FEMenu* v6 = menus[m_active];
        if (v6 != nullptr)
            v6->Draw();
    }
    mDisplay->Draw();
}

// ea: 0x00586470
void DialogMenuSystem::CountDown(float time_inc)
{
    CloseDialog();
    if (mCountDown <= time_inc)
    {
        mCountDown = 0.0f;
        MPUIInterface::mCableDisconnect = true;
        MPUIInterface::mReturnMenu = 8;
        MPUIInterface::ExitGame();
        if (mState != DMS_PENDING_SHUTDOWN)
        {
            mState = DMS_PENDING_SHUTDOWN;
            mCountDown = 5.0f;
            gDelayRenderForNFrames = 0;
        }
    }
    else
    {
        mCountDown = mCountDown - time_inc;
        const char* STBString = STBManager::sInst->GetSTBString(
            "MPFRONTEND_NETWORK_ERROR_COUNTDOWN");
        char messagebuff[200];
        sprintf(messagebuff, "%s %d", STBString, (int)mCountDown);
        BringUp(messagebuff, false, false, defaultFileName, true);
        DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
        DMS->mDisplay->Reformat();
    }
}

// ea: 0x00586640
void DialogMenuSystem::SetState(eState newState)
{
    if (mState != newState)
    {
        mState = newState;
        if (newState == 1)
        {
            mCountDown = 5.9899998f;
            CountDown(g_time_inc);
        }
        else if (newState == DMS_PENDING_SHUTDOWN)
        {
            mCountDown = 5.0f;
            gDelayRenderForNFrames = 0;
        }
    }
}

// ea: 0x005726B0
void DialogMenuSystem::GeneralBringUpStuff(bool layer1)
{
    if (GetActiveMenu() < 0)
    {
        if (!GamePause::IsGamePaused(mClient))
            flags &= ~1u;
        else
            flags |= 1u;
    }
    if (!layer1 && GetActiveMenu() != 1)
    {
        if (GetActiveMenu() == 0)
            flags |= 2u;
        else
            flags &= ~2u;
    }
    mWasIGMSUpWhenLaunched = false;
    MakeActive(!layer1);
    if (g_femanager.fems == nullptr
        || !g_femanager.fems->IsSystemActive())
    {
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(mClient);
        mWasIGMSUpWhenLaunched = IGMS->IsSystemActive();
        if (g_femanager.mIGMS[mClient] != nullptr)
            g_femanager.mIGMS[mClient]->is_active = true;
        GamePause::SetGamePaused(mClient, true);
    }
}

// ea: 0x00572770
void DialogMenuSystem::SetText(const char* t)
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(mClient);
    DMS->mDisplay->SetText(t);
}

// ea: 0x00572790
void DialogMenuSystem::CloseDialog()
{
    if (GetActiveMenu() == 1 && (flags & 2) != 0)
    {
        MakeActive(0);
        flags &= ~2u;
    }
    else
    {
        if (!mWasIGMSUpWhenLaunched)
        {
            InGameMenuSystem* IGMS = g_femanager.GetIGMS(currCl);
            IGMS->MakeActive(-1);
        }
        mDisplay->mIsClosing = true;
    }
}

// ea: 0x005727E0
void DialogMenuSystem::OnFinish()
{
    MakeActive(-1);
    if ((flags & 1) == 0)
    {
        SoundDevice::sInst->UnpauseAllSounds();
        GamePause::SetGamePaused(mClient, false);
    }
    if (!mWasIGMSUpWhenLaunched)
    {
        InGameMenuSystem* IGMS = g_femanager.GetIGMS(currCl);
        IGMS->MakeActive(-1);
    }
    flags &= ~4u;
}

// ea: 0x00572830
DialogMenu* DialogMenuSystem::GetLayer(bool layer1)
{
    return (DialogMenu*)menus[!layer1];
}

// ea: 0x00572850
void DialogMenuSystem::UseSmallBackground(bool use)
{
    (void)use;
}

// ea: 0x00572860
bool DialogMenuSystem::DefaultNoResponse(int client)
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(client);
    DMS->flags &= ~8u;
    return true;
}

// ea: 0x00586690
bool DialogMenuSystem::DefaultYesResponse(int client)
{
    if (g_controllerConnectedErrorShown[LocalClient::ClientToPort(client)])
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(client);
        if (DMS->mState != DMS_STATE_NONE)
            DMS->mState = DMS_STATE_NONE;
        g_controllerConnectedErrorShown[LocalClient::ClientToPort(client)] =
            false;
    }
    DialogMenuSystem* v2 = g_femanager.GetDMS(client);
    v2->flags |= 8u;
    return true;
}

// ea: 0x00572880
void DialogMenuSystem::OnStart(int c)
{
    if (mState == 2)
    {
        g_controllerConnectedErrorShown[c] = false;
        CloseDialog();
        mState = DMS_STATE_NONE;
    }
}

// ea: 0x005728B0
int DialogMenuSystem::GetClientFromController(int c)
{
    (void)c;
    if (g_femanager.fems != nullptr
        && g_femanager.fems->IsSystemActive())
        return GetCurrentClient();
    return mClient;
}

// ea: 0x005728E0
int DialogMenuSystem::GetCurrentClient()
{
    if (g_femanager.fems != nullptr
        && g_femanager.fems->IsSystemActive())
        return 0;
    return mClient;
}

// ea: 0x00572900
int DialogMenuSystem::GetCurrentClientController()
{
    if (g_femanager.fems != nullptr
        && g_femanager.fems->IsSystemActive())
        return 0;
    return unk_F6A28C[802 * mClient];
}

// ea: 0x00572930
bool DialogMenuSystem::GetAnalogPressed(int button, int* p_controller)
{
    if (g_femanager.fems != nullptr
        && g_femanager.fems->IsSystemActive())
        return FEMenuSystem::GetAnalogPressed(button, p_controller);
    int v5 = unk_F6A28C[802 * mClient];
    int x = 0;
    int y = 0;
    controller* v6 = controller::inst();
    v6->stick_value(v5, controller::LEFTSTICK, x, y);
    controller* v7 = controller::inst();
    if (v7->button_pressed(v5, controller::UPBUTTON))
        y = -128;
    controller* v8 = controller::inst();
    if (v8->button_pressed(v5, controller::DOWNBUTTON))
        y = 128;
    controller* v9 = controller::inst();
    if (v9->button_pressed(v5, controller::LEFTBUTTON))
        x = -128;
    controller* v10 = controller::inst();
    bool v11 = v10->button_pressed(v5, controller::RIGHTBUTTON);
    int v12 = 128;
    if (!v11)
        v12 = x;
    bool result = false;
    switch (button)
    {
    case 4:
        result = y < -64;
        break;
    case 8:
        result = y >= 64;
        break;
    case 16:
        result = v12 < -64;
        break;
    case 32:
        result = v12 >= 64;
        break;
    default:
        result = false;
        break;
    }
    if (result && p_controller != nullptr)
        *p_controller = v5;
    return result;
}

// ea: 0x00572A90
bool DialogMenuSystem::GetButtonPressed(int button, int* p_controller)
{
    if (g_femanager.fems != nullptr
        && g_femanager.fems->IsSystemActive())
    {
        controller* v4 = controller::inst();
        return v4->button_pressed((controller::ButtonIndex)button,
                                  p_controller);
    }
    if (p_controller != nullptr)
        *p_controller = unk_F6A28C[802 * mClient];
    int v7 = unk_F6A28C[802 * mClient];
    controller* v6 = controller::inst();
    return v6->button_pressed(v7, (controller::ButtonIndex)button);
}

// ea: 0x00572B00
int DialogMenuSystem::GetStickValueX(int stick, int* p_controller)
{
    if (g_femanager.fems != nullptr
        && g_femanager.fems->IsSystemActive())
    {
        controller* v3 = controller::inst();
        return v3->stick_value_x((controller::StickIndex)stick,
                                 p_controller);
    }
    if (p_controller != nullptr)
        *p_controller = unk_F6A28C[802 * currCl];
    int v6 = unk_F6A28C[802 * currCl];
    controller* v5 = controller::inst();
    return v5->stick_value_x(v6, (controller::StickIndex)stick);
}

// ea: 0x00572B70
int DialogMenuSystem::GetStickValueY(int stick, int* p_controller)
{
    if (g_femanager.fems != nullptr
        && g_femanager.fems->IsSystemActive())
    {
        controller* v3 = controller::inst();
        return v3->stick_value_y((controller::StickIndex)stick,
                                 p_controller);
    }
    if (p_controller != nullptr)
        *p_controller = unk_F6A28C[802 * currCl];
    int v6 = unk_F6A28C[802 * currCl];
    controller* v5 = controller::inst();
    return v5->stick_value_y(v6, (controller::StickIndex)stick);
}

// ea: 0x0058E3B0
void DialogMenuSystem::BringUp(const char* t, bool type_ok, bool type_yn,
                               const char* title_unloc, bool layer1)
{
    Broc::string text(t);
    Broc::string title(title_unloc);
    GeneralBringUpStuff(layer1);
    DialogMenu* v7 = (DialogMenu*)menus[GetActiveMenu() != 0];
    v7->BringUp(text, title, mClient);
    if (type_ok)
    {
        v7->AddOption("INGAME_DIALOG_OK", DialogMenuSystem::DefaultYesResponse);
        DialogMenuSystem* DMS = g_femanager.GetDMS(v7->mClient);
        DMS->mDisplay->Reformat();
    }
    else if (type_yn)
    {
        v7->AddOption("INGAME_DIALOG_YES",
                      DialogMenuSystem::DefaultYesResponse);
        v7->AddOption("INGAME_DIALOG_NO",
                      DialogMenuSystem::DefaultNoResponse);
        DialogMenuSystem* DMS = g_femanager.GetDMS(v7->mClient);
        DMS->mDisplay->Reformat();
    }
}

// ea: 0x0057F1A0
void DialogMenuSystem::HighlightOption(int index)
{
    menus[-(GetActiveMenu() != 0) == -1]->highlighted = (int16_t)index;
    DialogMenuDisplay* mDisplay = this->mDisplay;
    if (mDisplay->mOptionCount > 0 && mDisplay->mOptionCount <= 2)
        mDisplay->mOptionSelected = index;
    mDisplay->SetDialogFlash(index);
}

// ea: 0x005B5670
void DialogMenuSystem::AddOption(const char* t, bool (*responseFunc)(int))
{
    int v4 = GetActiveMenu();
    ((DialogMenu*)menus[v4 != 0])->AddOption(t, responseFunc);
}

// ea: 0x004E2A50
void DialogMenuSystem::Reformat(bool vertical)
{
    int v3 = GetActiveMenu();
    DialogMenu* Layer = (DialogMenu*)menus[v3 == 0];
    Layer->Reformat(vertical, 0);
}
