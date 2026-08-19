// ============================================================================
// vk_menu.cpp - VKMenu (shell.o VirtualKeyboardMenu.cpp)
// ============================================================================

#undef PlaySound  // windows.h macro would mangle VKMenu::PlaySound

#include "game/shell/shell_types.h"
#include "game/platform_xbox/MemoryUnitManager.h"

#include <string.h>
#include <stdio.h>
#include <new>

extern FEManager g_femanager;          // ?g_femanager@@3UFEManager@@A
extern int currCl;                     // ?currCl@@3HA @ 0xF1579C
extern const char defaultFileName[];  // ?defaultFileName
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern SaveGameData gSaveGameData[4];    // ?gSaveGameData@@3PAUSaveGameData@@A
extern void j_nullsub_96();            // ?nullsub_96
extern float Sys_Time();               // ?Sys_Time@@YAMXZ

// STBManager minimal view
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
};

// MPProfileMainMenu minimal view (mSaveSlots)
class MPProfileMainMenu {
public:
    SaveGameData** mSaveSlots;  // +0x04 (opaque; first member after vtable)
    static MPProfileMainMenu* Me();  // ?Me@MPProfileMainMenu@@SAPAV1@XZ
};

// MPUIInterface::Step
class MPUIInterface {
public:
    static void Step();  // ?Step@MPUIInterface@@SAXXZ
};

// ============================================================================
// VKMenu
// ============================================================================

// ea: 0x005942A0
VKMenu::VKMenu(FEMenuSystem* s)
    : FEMultiMenu(s, 40, 0)
{
    flags = (int16_t)(flags | 0x80);
    mSaveDialogDisplayed = false;
    mProfileName[0] = 0;
    mCapitalized = false;
    mNameLength = 0;
    mNameValid = false;
    default_color_scheme = 19;
}

VKMenu* VKMenu_ctor(void* mem, FEMenuSystem* s)
{
    return new (mem) VKMenu(s);
}

// ea: 0x005942F0
VKMenu::~VKMenu()
{
    FEMenu::~FEMenu();
}

// ea: 0x00597740
void VKMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    if (pf != nullptr)
    {
        pf->PostUnmashFixup(PANEL_LAYER_PAUSE_MENU);
        int v3 = 1;
        PanelQuad** shadow = this->shadow;
        for (int i = 40; i != 0; --i)
        {
            char name[20];
            sprintf(name, "text%i", v3);
            word[i - 1] = this->panel->GetTextPointer(name);
            sprintf(name, "shadow%i", v3);
            *shadow++ = this->panel->GetPointer(name);
            ++v3;
        }
        disableShift = this->panel->GetPointer("shiftD");
        disableBackspace = this->panel->GetPointer("backspaceD");
        disableSpace = this->panel->GetPointer("spaceD");
        disableDone = this->panel->GetPointer("doneD");
        FEText* TextPointer = this->panel->GetTextPointer("Helpbar");
        if (helpbar1 != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\VirtualKeyboardMenu.cpp";
            AeAssert::gCurrentLine = 232;
            AeAssert::gCurrentExpr = "!helpbar1";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("no!"))
                __debugbreak();
        }
        FEMultiLineText* v14 =
            (FEMultiLineText*)mem_heap_malloc(0xA8u);
        FEMultiLineText* v15;
        if (v14 != nullptr)
        {
            int v23 = TextPointer->GetColor().i;
            float layer = TextPointer->GetScaleX();
            float x1 = TextPointer->GetY();
            float v20 = TextPointer->GetX();
            font_index v16 = TextPointer->GetFont();
            v15 = new (v14) FEMultiLineText(
                v16, x1, 0.0f, 0, (panel_layer)(int)layer, 0.0f, 0, v23,
                color32());
        }
        else
        {
            v15 = nullptr;
        }
        helpbar1 = v15;
        helpbar1->SetNumLines(1);
        helpbar1->SetText("FEMENU_OP_HELPBAR");
        FEText* v18 = this->panel->GetTextPointer("CreateProfile");
        v18->SetText("FEMENU_CREATE_SESSION");
        FEText* v19 = this->panel->GetTextPointer("profileName");
        v19->SetText("_");
    }
}

// ea: 0x005744A0
void VKMenu::OnTriangle(int c)
{
    (void)c;
    system->MakeActive(27);
}

// ea: 0x005744B0
VKMenu* VKMenu::Me()
{
    return (VKMenu*)g_femanager.fems->menus[18];
}

// ea: 0x005744C0
void VKMenu::SetEntryPositions()
{
    if (panel != nullptr)
    {
        PanelQuad** shadow = this->shadow;
        for (int i = 40; i != 0; --i)
        {
            (*shadow)->SetVisibility(0);
            ++shadow;
        }
        disableShift->SetVisibility(0);
        disableBackspace->SetVisibility(0);
        disableSpace->SetVisibility(0);
        disableDone->SetVisibility(0);
        int v6 = 0;
        FEText** word = this->word;
        do
        {
            char textString[2];
            textString[0] = (char)(v6 + 48);
            textString[1] = 0;
            AddEntry(v6, *word, 0);
            entries[v6]->SetText(textString);
            ++v6;
            ++word;
        }
        while (v6 <= 9);
        int v10 = 10;
        FEText** v11 = &this->word[10];
        do
        {
            char textString[2];
            textString[0] = (char)(v10 + 87);
            textString[1] = 0;
            AddEntry(v10, *v11, 0);
            entries[v10]->SetText(textString);
            ++v10;
            ++v11;
        }
        while (v10 <= 35);
        AddEntry(36, this->word[36], 0);
        AddEntry(37, this->word[37], 0);
        AddEntry(38, this->word[38], 0);
        AddEntry(39, this->word[39], 0);
        mCapitalized = false;
        entries[36]->SetText(defaultFileName);
        entries[37]->SetText(defaultFileName);
        entries[38]->SetText(defaultFileName);
        entries[39]->SetText(defaultFileName);
        for (int v15 = 0; v15 < 40; ++v15)
        {
            if ((v15 < 1 || v15 > 9) && (v15 < 11 || v15 > 18)
                && (v15 < 20 || v15 > 27) && (v15 < 29 || v15 > 35))
            {
                switch (v15)
                {
                case 19:
                    entries[19]->right = 36;
                    entries[v15]->right = v15 + 1;
                    break;
                case 28:
                    entries[28]->right = 37;
                    entries[v15]->right = v15 + 1;
                    break;
                case 38:
                    entries[38]->right = 37;
                    entries[38]->down = 39;
                    entries[38]->up = 31;
                    break;
                case 39:
                    entries[39]->right = 35;
                    entries[39]->up = 27;
                    break;
                default:
                    entries[v15]->right = v15 + 1;
                    break;
                }
            }
            else
            {
                entries[v15]->left = v15 - 1;
            }
            if (v15 <= 8 || (v15 >= 10 && v15 <= 17)
                || (v15 >= 19 && v15 <= 26) || (v15 >= 28 && v15 <= 34))
            {
                entries[v15]->right = v15 + 1;
            }
            else
            {
                switch (v15)
                {
                case 35:
                    entries[35]->down = 39;
                    entries[v15]->up = 26;
                    break;
                case 36:
                    entries[36]->down = 19;
                    entries[36]->up = 10;
                    entries[36]->left = 37;
                    break;
                case 37:
                    entries[37]->down = 28;
                    entries[37]->up = 36;
                    break;
                default:
                    if (v15 >= 10 && v15 <= 18)
                        entries[v15]->up = v15 - 10;
                    else if (v15 >= 19)
                        entries[v15]->up = v15 - 9;
                    if (v15 > 8)
                    {
                        if (v15 == 9)
                            entries[9]->down = 18;
                        else if (v15 < 10 || v15 > 26)
                        {
                            if (v15 == 27)
                                entries[27]->down = 39;
                            else if (v15 >= 28)
                                entries[v15]->down = 38;
                        }
                        else
                        {
                            entries[v15]->down = v15 + 9;
                        }
                    }
                    else
                    {
                        entries[v15]->down = v15 + 10;
                    }
                    break;
                }
            }
        }
        entries[0]->left = 9;
        entries[9]->down = 0;
        entries[10]->right = 18;
        entries[18]->down = 10;
        entries[36]->right = 27;
        entries[27]->down = 36;
        entries[37]->right = 39;
        entries[39]->down = 37;
        entries[38]->Disable(mNameLength == 0);
        entries[36]->Disable(mNameLength == 0);
        entries[39]->Disable(mNameLength == 0);
        SetHigh(10, true);
    }
}

// ea: 0x005749D0
void VKMenu::ChangeShift()
{
    char textString[2];
    textString[1] = 0;
    if (mCapitalized)
    {
        mCapitalized = false;
        for (int v3 = 10; v3 <= 35; ++v3)
        {
            textString[0] = (char)(v3 + 87);
            entries[v3]->SetText(textString);
        }
    }
    else
    {
        mCapitalized = true;
        for (int v3 = 10; v3 <= 35; ++v3)
        {
            textString[0] = (char)(v3 + 55);
            entries[v3]->SetText(textString);
        }
    }
}

// ea: 0x00574A50
void VKMenu::Up()
{
    int16_t highlighted = this->highlighted;
    int v3 = highlighted;
    if (highlighted == 37 && entries[36]->GetDisable())
        entries[this->highlighted]->up = 19;
    FEMenu::Up();
    if (v3 != this->highlighted)
        PlayNavigationSound();
    if (this->highlighted == 37)
        entries[37]->up = 36;
}

// ea: 0x00574AC0
void VKMenu::Down()
{
    int16_t highlighted = this->highlighted;
    int v3 = highlighted;
    if (highlighted >= 28 && highlighted <= 35
        && entries[38]->GetDisable())
    {
        entries[this->highlighted]->down = 37;
        if (entries[37]->GetDisable())
            entries[this->highlighted]->down = 39;
    }
    if (this->highlighted == 27 && entries[39]->GetDisable())
        entries[this->highlighted]->down = 35;
    FEMenu::Down();
    if (v3 != this->highlighted)
        PlayNavigationSound();
    int16_t v4 = this->highlighted;
    if (v4 >= 28 && v4 <= 35)
        entries[v4]->down = 38;
    if (this->highlighted == 27)
        entries[27]->left = 39;
}

// ea: 0x00574B90
void VKMenu::Left()
{
    int16_t highlighted = this->highlighted;
    int v3 = highlighted;
    if (highlighted == 39 && entries[35]->GetDisable())
    {
        entries[this->highlighted]->left = 37;
        if (entries[37]->GetDisable())
            entries[this->highlighted]->left = 36;
    }
    if (this->highlighted != 36 || !entries[27]->GetDisable())
    {
        if (this->highlighted == 37 && entries[39]->GetDisable())
            entries[this->highlighted]->left = 35;
        FEMenu::Left();
        if (v3 != this->highlighted)
            PlayNavigationSound();
        if (this->highlighted == 39)
            entries[39]->right = 35;
        if (this->highlighted == 37)
            entries[37]->right = 39;
    }
}

// ea: 0x00574C70
void VKMenu::Right()
{
    int16_t highlighted = this->highlighted;
    int v3 = highlighted;
    if (highlighted == 36 && entries[19]->GetDisable())
        entries[this->highlighted]->right = 39;
    if (this->highlighted == 27 && entries[36]->GetDisable())
        entries[this->highlighted]->right = 19;
    if (this->highlighted != 39 || !entries[37]->GetDisable())
    {
        FEMenu::Right();
        if (v3 != this->highlighted)
            PlayNavigationSound();
        if (this->highlighted == 36)
            entries[36]->down = 19;
        if (this->highlighted == 27)
            entries[27]->down = 36;
    }
}

// ea: 0x00574D30
void VKMenu::UpdateShadows()
{
    for (int v2 = 0; v2 < 40; ++v2)
    {
        if (v2 == highlighted)
            shadow[v2]->SetVisibility(1.0f);
        else
            shadow[v2]->SetVisibility(0.0f);
    }
    if (entries[37]->GetDisable())
        disableShift->SetVisibility(1.0f);
    else
        disableShift->SetVisibility(0.0f);
    if (entries[36]->GetDisable())
        disableBackspace->SetVisibility(1.0f);
    else
        disableBackspace->SetVisibility(0.0f);
    if (entries[38]->GetDisable())
        disableSpace->SetVisibility(1.0f);
    else
        disableSpace->SetVisibility(0.0f);
    if (entries[39]->GetDisable())
        disableDone->SetVisibility(1.0f);
    else
        disableDone->SetVisibility(0.0f);
}

// ea: 0x00574E40
void VKMenu::ButtonHeldAction()
{
    if (flags < 0)
    {
        switch (button_held_down)
        {
        case 32:
            OnRight(0);
            break;
        case 16:
            OnLeft(0);
            break;
        case 4:
            OnUp(0);
            break;
        case 8:
            OnDown(0);
            break;
        default:
            break;
        }
    }
}

// ea: 0x00574E90
bool VKMenu::DialogResponseSaveSuccess(int)
{
    g_femanager.fems->MakeActive(29);
    return true;
}

// ea: 0x00574EA0
bool VKMenu::DialogResponseNoMemCard(int)
{
    g_femanager.fems->MakeActive(27);
    return true;
}

// ea: 0x005803A0
void VKMenu::OnActivate()
{
    FEText* TextPointer = panel->GetTextPointer("CreateProfile");
    TextPointer->SetText("FEMENU_CREATE_SESSION");
    mNameLength = 0;
    mNameValid = false;
    SetEntryPositions();
    FEMenu::OnActivate();
    mProfileName[0] = 0;
    FEText* v4 = panel->GetTextPointer("profileName");
    v4->SetText("_");
    SetHigh(10, true);
    mCapitalized = false;
    mSaveDialogDisplayed = false;
}

// ea: 0x00580420
void VKMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    if (!g_femanager.menuMovieRunning)
        movie_manager::movie_done(false);
}

// ea: 0x00580440
void VKMenu::Draw()
{
    if (panel != nullptr)
    {
        panel->Draw();
        FEMenu::Draw();
    }
    disableShift->Draw();
    disableBackspace->Draw();
    disableSpace->Draw();
    disableDone->Draw();
}

// ea: 0x00580490
void VKMenu::PlaySound()
{
    math::Position3 pos;
    math::Dir3 dir;
    memset(&pos, 0, sizeof(pos));
    memset(&dir, 0, sizeof(dir));
    SoundDevice::sInst->PlaySound("UI_Type",
                                  DbLinkedHandle<EntityHandleDb, Entity>(),
                                  false, false, pos, dir, -1.0f, -1.0f,
                                  -1.0f, -1.0f);
}

// ea: 0x00580500
void VKMenu::UpdateWidescreen(bool widescreen)
{
    if (panel != nullptr)
    {
        panel->UpdateWidescreen(widescreen, 320.0f);
        helpbar1->UpdateForWidescreen(widescreen);
    }
}

// ea: 0x00580530
void VKMenu::AddCharacter(int c)
{
    entries[38]->Disable(false);
    entries[36]->Disable(false);
    entries[39]->Disable(false);
    int mNameLength = this->mNameLength;
    if (mNameLength < 15)
    {
        char v4 = 97;
        if (c > 9)
        {
            if (c > 35)
            {
                if (c == 38)
                    v4 = 32;
            }
            else
            {
                v4 = mCapitalized ? (char)(c + 55) : (char)(c + 87);
            }
        }
        else
        {
            v4 = (char)(c + 48);
        }
        mProfileName[mNameLength] = v4;
        int v6 = this->mNameLength + 1;
        this->mNameLength = v6;
        if (this->mNameLength - 15 < 0)
        {
            mProfileName[v6] = 95;
            mProfileName[this->mNameLength + 1] = 0;
        }
        else
        {
            mProfileName[v6] = 0;
        }
        FEText* TextPointer = panel->GetTextPointer("profileName");
        TextPointer->SetText(mProfileName);
        mProfileName[this->mNameLength] = 0;
        if (this->mNameLength == 15)
        {
            for (int i = 0; i <= 35; ++i)
                entries[i]->Disable(true);
            entries[37]->Disable(true);
            entries[38]->Disable(true);
            SetHigh(39, true);
        }
    }
}

// ea: 0x00580670
void VKMenu::RemoveCharacter()
{
    int mNameLength = this->mNameLength;
    if (mNameLength > 0)
    {
        int v3 = mNameLength - 1;
        this->mNameLength = v3;
        mProfileName[v3] = 95;
        mProfileName[this->mNameLength + 1] = 0;
        FEText* TextPointer = panel->GetTextPointer("profileName");
        TextPointer->SetText(mProfileName);
        mProfileName[this->mNameLength] = 0;
        if (this->mNameLength < 15)
        {
            for (int i = 0; i <= 39; ++i)
                entries[i]->Disable(false);
        }
        if (this->mNameLength == 0)
        {
            entries[38]->Disable(true);
            entries[36]->Disable(true);
            entries[39]->Disable(true);
        }
    }
}

// ea: 0x0058E950
void VKMenu::DialogDisplaySaving()
{
    if (!mSaveDialogDisplayed)
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_SAVE_WARNING", false, false, defaultFileName,
                 true);
    DialogMenuSystem* v3 = g_femanager.GetDMS(currCl);
    v3->AddOption(defaultFileName, (bool (*)(int))j_nullsub_96);
        DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
        v4->mDisplay->Reformat();
        mSaveDialogDisplayed = true;
    }
}

// ea: 0x0058EA20
void VKMenu::DialogDisplaySaveSuccess()
{
    mSaveDialogDisplayed = false;
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_SAVE_SUCCESS", false, false, defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    v2->AddOption("MEM_DIALOG_OK", VKMenu::DialogResponseSaveSuccess);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    v4->GetLayer(v4->GetActiveMenu() == 0)->highlighted = 0;
    DialogMenuDisplay* mDisplay = v4->mDisplay;
    int mOptionCount = mDisplay->mOptionCount;
    if (mOptionCount > 0 && mOptionCount <= 2)
        mDisplay->mOptionSelected = 0;
    mDisplay->SetDialogFlash(0);
    DialogMenuSystem* v7 = g_femanager.GetDMS(currCl);
    v7->AddOption(defaultFileName, (bool (*)(int))j_nullsub_96);
    DialogMenuSystem* v8 = g_femanager.GetDMS(currCl);
    v8->mDisplay->Reformat();
}

// ea: 0x0058EB70
void VKMenu::DialogDisplayNoMemDevice()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_ERROR_INSERT_CARD", false, false, defaultFileName,
                 true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    v2->AddOption("MEM_DIALOG_OK", VKMenu::DialogResponseNoMemCard);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    v4->GetLayer(v4->GetActiveMenu() == 0)->highlighted = 0;
    DialogMenuDisplay* mDisplay = v4->mDisplay;
    int mOptionCount = mDisplay->mOptionCount;
    if (mOptionCount > 0 && mOptionCount <= 2)
        mDisplay->mOptionSelected = 0;
    mDisplay->SetDialogFlash(0);
    DialogMenuSystem* v7 = g_femanager.GetDMS(currCl);
    v7->AddOption(defaultFileName, (bool (*)(int))j_nullsub_96);
    DialogMenuSystem* v8 = g_femanager.GetDMS(currCl);
    v8->mDisplay->Reformat();
}

// ea: 0x0059AE50
void VKMenu::CreateProfileDone()
{
    g_femanager.mProfileManager->CreateProfile(mSlotNum);
}

// ea: 0x0059C4E0
void VKMenu::CompleteName()
{
    for (int i = mNameLength - 1; i > 0; mProfileName[i--] = 0)
    {
        if (mProfileName[i] != 32)
            break;
    }
    if (strcmp(mProfileName, defaultFileName) == 0)
        system->MakeActive(27);
    for (int j = 0; j < 16; ++j)
        gSaveGameData[0].mStubData.mProfileName[j] = mProfileName[j];
    g_femanager.mProfileManager->CreateProfile(mSlotNum);
}

// ea: 0x0059D130
void VKMenu::Select(int entry_num)
{
    PlaySound();
    if (entry_num <= 35)
    {
        AddCharacter(entry_num);
    }
    else
    {
        switch (entry_num)
        {
        case 36:
            RemoveCharacter();
            break;
        case 37:
            ChangeShift();
            break;
        case 38:
            AddCharacter(38);
            break;
        case 39:
            CompleteName();
            break;
        default:
            break;
        }
    }
}

// ea: 0x0059D5F0
void VKMenu::Update(float time_inc)
{
    MPUIInterface::Step();
    FEMenu::Update(time_inc);
    UpdateShadows();
    ProfileManager* mProfileManager = g_femanager.mProfileManager;
    MemoryUnitManager::Service();
    if (mProfileManager->mOperationState != 0
        && Sys_Time() > mProfileManager->mCountdownFinishedTime)
    {
        mProfileManager->FinishOperation();
        mProfileManager->mOperationState =
            ProfileManager::kOperationNone;
    }
    int mCurrentStatus = g_femanager.mProfileManager->mCurrentStatus;
    switch (mCurrentStatus)
    {
    case -1:
    {
        g_femanager.mProfileManager->Reset();
        DialogDisplayNoMemDevice();
        break;
    }
    case 0:
    {
        mSaveDialogDisplayed = false;
        break;
    }
    case 1:
    {
        DialogDisplaySaving();
        break;
    }
    case 2:
    {
        g_femanager.mProfileManager->Reset();
        DialogDisplaySaveSuccess();
        break;
    }
    case 6:
    {
        g_femanager.mProfileManager->Reset();
        MPProfileMainMenu* v5 = MPProfileMainMenu::Me();
        g_femanager.mProfileManager->EnumProfiles(v5->mSaveSlots);
        break;
    }
    default:
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\VirtualKeyboardMenu.cpp";
        AeAssert::gCurrentLine = 156;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Illegal state (%d) from Profile Manager",
                mCurrentStatus))
            __debugbreak();
        break;
    }
    }
}
