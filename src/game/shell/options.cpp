// ============================================================================
// options.cpp - FESplitScreenMenu + InGameOptionsMenu/AARInGameOptionsMenu
// + OptionsGameplayMenu/Controls/Sound/Stick/Button + GammaScreenMenu
// ============================================================================

#include "game/shell/shell_types.h"
#include "core/tlFixedString.h"

#include <string.h>
#include <stdio.h>
#include <intrin.h>
#include <new>

extern FEManager g_femanager;          // ?g_femanager@@3UFEManager@@A
extern int currCl;                     // ?currCl@@3HA @ 0xF1579C
extern const char defaultFileName[];  // 0xCD67AE
extern void* mem_heap_malloc(int alignment, unsigned int size);  // core.o
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);  // core.o
extern SaveGameData gSaveGameData[4];    // ?gSaveGameData@@3PAUSaveGameData@@A
float g_GammaRamp = 1.0f;              // ?g_GammaRamp@@3MA (render.o)
extern int g_MPAARTotalTime;           // mp.o
extern kuju::knet::sTime g_MPAARTimer;  // mp.o
extern float unk_F6A284[4 * 802];      // @ 0xF6A284 (cg.o)
extern float unk_F6A27C[4 * 802];      // @ 0xF6A27C (cg.o; old viewports)
extern unsigned char unk_F6A294[4 * 3208];  // @ 0xF6A294 (cg.o)
extern int cl_aADS[2];                  // ?cl_aADS@@3PAHA (cl.o)
extern void ApplyControllerStickConfig(int stickConfig);    // game2.o
extern void ApplyControllerButtonConfig(int buttonConfig);  // game2.o

namespace LocalClient {
extern int ClientToPort(int client);  // ?ClientToPort@LocalClient@@YAHH@Z
}

// STBManager minimal view (full class in core/core_systems.h)
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
};

// MusicMgr minimal view
struct MusicMgr {
public:
    static MusicMgr* sInst;  // ?sInst@MusicMgr@@2PAV1@A @ 0xF4EBE4
    void ScaleVolume(float scale);  // ?ScaleVolume@MusicMgr@@QAEXM@Z (game.o)
};

struct kuju_sTime {
    int mTime;
};

// PakInfoNode (pakId at +0xB4; full node in streamer)
struct PakInfoNode {
    uint8_t _pad[0xB4];
    TPakId  pakId;   // +0xB4
};

extern nglTexture* cdGetTexture(TPakId pakId,
                                const tlFixedString& name);  // streamer.o

// KeyInfo key-binding table (cl.o); entry = {mDown/mRepeats bits, cmd name}
struct KeyInfoEntry3 {
    int  mState;          // mDown:2, mRepeats:30
    char* mBoundCmdName;  // +0x04
};
extern KeyInfoEntry3 KeyInfo_mKeys[2][256];

// ============================================================================
// shell.o data (verified VAs from IDA)
// ============================================================================
const char* const pszLabelTexts[] = {
    "MPGAME_STICK_LAYOUT",
    "MPGAME_BUTTON_LAYOUT",
    "MPGAME_HORIZONTAL_SENSITIVITY",
    "MPGAME_VERTICAL_SENSITIVITY",
    "MPGAME_INVERT_AIM",
    "MPGAME_TOGGLE_ADS",
    "MPGAME_STICKY_AIM",
    "MPGAME_ALTERNATE_TANK_CONTROLS",
    "MPGAME_VIBRATION",
    "MPGAME_RESTORE_DEFAULTS",
};  // ?pszLabelTexts@@3PAPBDA @ 0xDF394C

const char* const pszStickLayout[] = {
    "FEMENU_COP_STICK_DEFAULT",
    "FEMENU_COP_STICK_SOUTHPAW",
    "FEMENU_COP_STICK_LEGACY",
    "FEMENU_COP_STICK_LEGACYSOUTHPAW",
};  // ?pszStickLayout@@3PAPBDA @ 0xDF3974

const char* const pszButtonLayout[] = {
    "FEMENU_COP_BUTTON_DEFAULT",
    "FEMENU_COP_BUTTON_SOUTHPAW",
    "FEMENU_COP_BUTTON_SCI_FI",
    "FEMENU_COP_BUTTON_LEGACY",
};  // ?pszButtonLayout@@3PAPBDA @ 0xDF3984

const char* const pszInvertAim[] = {
    "MPGAME_DISABLE",
    "MPGAME_ENABLE",
};  // ?pszInvertAim@@3PAPBDA @ 0xDF3994

const char* const pszToggleADS[] = {
    "MPGAME_DISABLE",
    "MPGAME_ENABLE",
};  // ?pszToggleADS@@3PAPBDA @ 0xDF399C

const char* const pszStickyAim[] = {
    "MPGAME_DISABLE",
    "MPGAME_ENABLE",
};  // ?pszStickyAim@@3PAPBDA @ 0xDF39A4

const char* const pszAlternateTankControls[] = {
    "MPGAME_DISABLE",
    "MPGAME_ENABLE",
};  // ?pszAlternateTankControls@@3PAPBDA @ 0xDF39AC

const char* const pszVibration[] = {
    "MPGAME_DISABLE",
    "MPGAME_ENABLE",
};  // ?pszVibration@@3PAPBDA @ 0xDF39B4

const char* const g_XBoxBadDiskWarnings[] = {
    "There is a problem with the disc you are using.",
    "It may be dirty or damaged.",
    "Le disque utilise presente une anomalie.",
    "Il est peut-etre sale ou endommage.",
    "Bei der benutzten CD ist ein Problem aufgetreten.",
    "Moglicherweise ist sie verschmutzt oder beschadigt.",
    "Il disco in uso ha qualche problema.",
    "Potrebbe essere sporco o danneggiato.",
    "Hay un problema con el disco que esta usando.",
    "Puede estar sucio o danado.",
};  // ?g_XBoxBadDiskWarnings@@3PAPADA @ 0xDF39BC

// ============================================================================
// FESplitScreenMenu
// ============================================================================

PanelFile* FESplitScreenMenu::mBackground = nullptr;  // @ 0xF99868

// ea: 0x007A7150
FESplitScreenMenu::FESplitScreenMenu(FEMenuSystem* pSystem, int num_entries)
    : FEMenu(pSystem, num_entries, 320, 240, 8, 0)
{
    mViewport = 0;
    mSplitScreenMenu = nullptr;
    mMainTextEntries.m_elements = nullptr;
    mMainTextEntries.m_capacity = 0;
    mMainTextEntries.m_size = 0;
    mSplitScreenTextEntries.m_elements = nullptr;
    mSplitScreenTextEntries.m_capacity = 0;
    mSplitScreenTextEntries.m_size = 0;
    mVersion = pSystem->GetCurrentClient();
}

// ea: 0x007A7290
FESplitScreenMenu::~FESplitScreenMenu()
{
    mem_heap_free(mSplitScreenTextEntries.m_elements);
    mem_heap_free(mMainTextEntries.m_elements);
    FEMenu::~FEMenu();
}

// ea: 0x00793100
void FESplitScreenMenu::Draw()
{
    PanelFile* mSplitScreenMenu;
    if (mViewport != 0)
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(mVersion);
        if (DMS->IsSystemActive())
            return;
        if (FESplitScreenMenu::mBackground != nullptr)
            FESplitScreenMenu::mBackground->Draw();
        mSplitScreenMenu = this->mSplitScreenMenu;
    }
    else
    {
        mSplitScreenMenu = panel;
    }
    if (mSplitScreenMenu != nullptr)
        mSplitScreenMenu->Draw();
    FEMenu::Draw();
}

// ea: 0x007930E0
void FESplitScreenMenu::OnActivate()
{
    SwapMenus();
    FEMenu::OnActivate();
}

// ea: 0x00793060
void FESplitScreenMenu::UpdateSplitScreen()
{
    bool wasSplitScreen = mViewport != 0;
    if (mSplitScreenMenu != nullptr)
    {
        int v5 = 3208 * mVersion;
        mViewport = (int)unk_F6A284[802 * mVersion];
        mSplitScreenMenu->MoveSplitScreen(
            mViewport, (int)unk_F6A27C[1 + 802 * mVersion]);
        if (wasSplitScreen != (mViewport != 0))
            SwapMenus();
    }
}

// ea: 0x007930D0 (empty base)
void FESplitScreenMenu::SetPanelFile(PanelFile* pf)
{
    (void)pf;
}

// ea: 0x005AF6D0
bool FESplitScreenMenu::IsSplitScreen()
{
    return mViewport != 0;
}

// ea: 0x00793150
void FESplitScreenMenu::AddMainEntry(int index, FEText* text)
{
    AddEntry(index, text, false);
}

// ea: 0x00793170
FEComboBox* FESplitScreenMenu::AddMainComboBox(int index, int numOptions,
                                               FEText* t, FEText* label,
                                               PanelQuad* leftArrow,
                                               PanelQuad* rightArrow)
{
    return AddComboBox(index, numOptions, t, label, leftArrow, rightArrow);
}

// ea: 0x00793180
FESlider* FESplitScreenMenu::AddMainSlider(int index, PanelQuad* bar,
                                           FEText* label)
{
    return AddSlider(index, bar, label);
}

// ea: 0x00793190
FESlider* FESplitScreenMenu::AddMainSlider(int index, FEText* barText,
                                           FEText* label)
{
    return AddSlider(index, barText, label);
}

// ea: 0x007931A0
void FESplitScreenMenu::SetPanelFileBackground(PanelFile* pf)
{
    FESplitScreenMenu::mBackground = pf;
}

// ea: 0x007A7300
void FESplitScreenMenu::AddSplitScreenEntry(int index, FEText* text)
{
    AddEntry(index, text, false);
    mSplitScreenTextEntries[index] = entries[index];
}

// ea: 0x007A7330
FEComboBox* FESplitScreenMenu::AddSplitScreenComboBox(
    int index, int numOptions, FEText* t, FEText* label,
    PanelQuad* leftArrow, PanelQuad* rightArrow)
{
    FEMenuEntry* v8 = AddComboBox(index, numOptions, t, label, leftArrow,
                                  rightArrow);
    mSplitScreenTextEntries[index] = v8;
    return (FEComboBox*)v8;
}

// ea: 0x007A7370
FESlider* FESplitScreenMenu::AddSplitScreenSlider(int index, PanelQuad* bar,
                                                  FEText* label)
{
    FESlider* v5 = AddSlider(index, bar, label);
    mSplitScreenTextEntries[index] = v5;
    return v5;
}

// ea: 0x007A73B0
FESlider* FESplitScreenMenu::AddSplitScreenSlider(int index,
                                                  FEText* barText,
                                                  FEText* label)
{
    FEMenuEntry* v5 = AddSlider(index, barText, label);
    mSplitScreenTextEntries[index] = v5;
    return (FESlider*)v5;
}

// ea: 0x005B33D0
void FESplitScreenMenu::DrawBackground()
{
    if (FESplitScreenMenu::mBackground != nullptr)
        FESplitScreenMenu::mBackground->Draw();
}

// ea: 0x007AC280
void FESplitScreenMenu::PanelFileUnloaded(PanelFile* pf)
{
    for (int i = 0; i < num_entries; ++i)
        entries[i] = nullptr;
    PanelFile* panel = this->panel;
    if (panel != nullptr)
    {
        if (strcmp(pf->mName, panel->mName) == 0)
        {
            if (mVersion > 0)
            {
                panel->~PanelFile();
                mem_heap_free(panel);
            }
            this->panel = nullptr;
            for (int i = 0; i < mMainTextEntries.m_size; ++i)
            {
                FEMenuEntry* v8 = mMainTextEntries[i];
                if (v8 != nullptr)
                    delete v8;
                mMainTextEntries[i] = nullptr;
            }
            goto sound_release;
        }
    }
    if (mSplitScreenMenu != nullptr
        && strcmp(pf->mName, mSplitScreenMenu->mName) == 0)
    {
        mSplitScreenMenu = nullptr;
        for (int i = 0; i < mSplitScreenTextEntries.m_size; ++i)
        {
            FEMenuEntry* v11 = mSplitScreenTextEntries[i];
            if (v11 != nullptr)
                delete v11;
            mSplitScreenTextEntries[i] = nullptr;
        }
    }
    else if (FESplitScreenMenu::mBackground != nullptr
             && strcmp(pf->mName,
                       FESplitScreenMenu::mBackground->mName) == 0)
    {
        FESplitScreenMenu::mBackground = nullptr;
    }
sound_release:
    unsigned int mVal = *(unsigned int*)&sound;
    unsigned int v13 = mVal & 0xFFF;
    if (v13 < 0x200
        && (mVal >> 12) == SoundDevice::SoundHandleDb::sInst.mElements[v13].mKey
        && SoundDevice::SoundHandleDb::sInst.mElements[v13].mObject != nullptr)
    {
        unsigned int v14 = mVal & 0xFFF;
        SoundDevice::Sound* mObject = nullptr;
        if (v14 < 0x200
            && (mVal >> 12) == SoundDevice::SoundHandleDb::sInst.mElements[v14].mKey)
            mObject = SoundDevice::SoundHandleDb::sInst.mElements[v14].mObject;
        SoundDevice::sInst->ReleaseSound(mObject);
    }
    sound = nullptr;
    entries = nullptr;
    num_entries = 0;
    if (helpbar1 != nullptr)
        delete helpbar1;
    if (helpbar2 != nullptr)
        delete helpbar2;
    if (helpbar3 != nullptr)
        delete helpbar3;
    helpbar1 = nullptr;
    helpbar2 = nullptr;
    helpbar3 = nullptr;
}

// ============================================================================
// InGameOptionsMenu
// ============================================================================

// ea: 0x0056E720
InGameOptionsMenu::InGameOptionsMenu(FEMenuSystem* s)
    : FESplitScreenMenu(s, 10)
{
    mScrollBarTopY = 0;
    mScrollBarBottomY = 0;
    mScrollBarYInc = 0;
    mScrollBarThumb = nullptr;
    mScrollBarUpFader.mQuad = nullptr;
    mScrollBarUpFader.mFading = false;
    mScrollBarUpFader.mAlphaTo = 1.0f;
    mScrollBarUpFader.mTime = 0.0f;
    mScrollBarUpFader.mAlphaDelta = 0.0f;
    mScrollBarUpFader.mAlpha = 0.0f;
    mScrollBarDownFader.mQuad = nullptr;
    mScrollBarDownFader.mFading = false;
    mScrollBarDownFader.mAlphaTo = 1.0f;
    mScrollBarDownFader.mTime = 0.0f;
    mScrollBarDownFader.mAlphaDelta = 0.0f;
    mScrollBarDownFader.mAlpha = 0.0f;
    flags = (int16_t)(flags | 0x100);
    m_ucLastHighlighted = 0;
}

// ea: 0x0056E7F0
InGameOptionsMenu::~InGameOptionsMenu()
{
    mSafeText.~FEText();
    FESplitScreenMenu::~FESplitScreenMenu();
}

// ea: 0x0056E850
void InGameOptionsMenu::Init()
{
}

// ea: 0x0056E860
void InGameOptionsMenu::OnActivate()
{
    int port = LocalClient::ClientToPort(mVersion);
    entries[0]->SetValue(
        gSaveGameData[port].mStubData.mControllerStickConfiguration);
    entries[1]->SetValue(
        gSaveGameData[port].mStubData.mControllerButtonConfiguration);
    entries[2]->SetValue(
        gSaveGameData[port].mStubData.mHorizontalSensitivity);
    entries[3]->SetValue(
        gSaveGameData[port].mStubData.mVerticalSensitivity);
    entries[4]->SetValue(gSaveGameData[port].mStubData.mInvertAim);
    entries[6]->SetValue(gSaveGameData[port].mStubData.mStickyAim);
    entries[5]->SetValue(gSaveGameData[port].mStubData.mAdsToggle);
    entries[7]->SetValue(
        gSaveGameData[port].mStubData.mTankStyle == 0);
    entries[8]->SetValue(gSaveGameData[port].mStubData.mVibration);
    FESplitScreenMenu::OnActivate();
    SetHigh(m_ucLastHighlighted, true);
}

// ea: 0x0056EA30 (thunk)
void InGameOptionsMenu::Draw()
{
    FESplitScreenMenu::Draw();
}

// ea: 0x0056EA40
void InGameOptionsMenu::ButtonHeldAction()
{
    flags = (int16_t)(flags & ~0x100);
    if (button_held_down == 4)
    {
        OnUp(0);
    }
    else if (button_held_down == 8)
    {
        OnDown(0);
    }
    else if (highlighted == 2 || highlighted == 3)
    {
        if (button_held_down == 32)
        {
            flags = (int16_t)(flags | 0x100);
            OnRight(0);
        }
        else if (button_held_down == 16)
        {
            flags = (int16_t)(flags | 0x100);
            OnLeft(0);
        }
    }
}

// ea: 0x0056EAB0
void InGameOptionsMenu::UpdateScrollBar()
{
    if (mScrollBarThumb != nullptr)
    {
        int v3 = highlighted < 0 ? 0 : highlighted;
        mScrollBarThumb->SetCenterPos(mScrollBarThumb->GetCenterX(),
                                      mScrollBarYInc * v3 + mScrollBarTopY);
    }
}

// ea: 0x0056EB10
InGameOptionsMenu* InGameOptionsMenu::Me(int version)
{
    return (InGameOptionsMenu*)g_femanager.GetIGMS(version)->menus[8];
}

// ea: 0x0056EB30
void InGameOptionsMenu::OnCross(int c)
{
    (void)c;
    if (highlighted == 9)
    {
        entries[0]->SetValue(0);
        entries[1]->SetValue(0);
        entries[4]->SetValue(0);
        entries[5]->SetValue(0);
        entries[6]->SetValue(1);
        entries[7]->SetValue(1);
        entries[8]->SetValue(1);
        entries[2]->SetValue(24);
        entries[3]->SetValue(32);
    }
}

// ea: 0x0056EBE0
void InGameOptionsMenu::OnStart(int c)
{
    (void)c;
}

// ea: 0x0056EBF0
bool InGameOptionsMenu::ResponseYesApplyNowHelper()
{
    m_ucLastHighlighted = (uint8_t)highlighted;
    int port = LocalClient::ClientToPort(mVersion);
    gSaveGameData[port].mStubData.mStickyAim =
        entries[6]->GetValue() != 0;
    gSaveGameData[port].mStubData.mHorizontalSensitivity =
        entries[2]->GetValue();
    gSaveGameData[port].mStubData.mVerticalSensitivity =
        entries[3]->GetValue();
    gSaveGameData[port].mStubData.mInvertAim =
        entries[4]->GetValue() != 0;

    int v9 = 7156 * port;
    if (entries[0]->GetValue()
        != gSaveGameData[port].mStubData.mControllerStickConfiguration)
    {
        int v10 = entries[0]->GetValue();
        gSaveGameData[port].mStubData.mControllerStickConfiguration = v10;
        ApplyControllerStickConfig(entries[0]->GetValue());
    }
    if (entries[1]->GetValue()
        != gSaveGameData[port].mStubData.mControllerButtonConfiguration)
    {
        int v14 = entries[1]->GetValue();
        gSaveGameData[port].mStubData.mControllerButtonConfiguration = v14;
        ApplyControllerButtonConfig(entries[1]->GetValue());
    }
    (void)v9;

    int v16 = system->GetCurrentClient();
    if (gSaveGameData[port].mStubData.mAdsToggle
        != (entries[5]->GetValue() != 0))
    {
        cl_aADS[v16] = 1;
        gSaveGameData[port].mStubData.mAdsToggle =
            entries[5]->GetValue() != 0;
        ApplyControllerButtonConfig(entries[1]->GetValue());
    }
    gSaveGameData[port].mStubData.mAdsToggle =
        entries[5]->GetValue() != 0;
    unk_F6A294[3208 * v16] = entries[7]->GetValue() != 0;
    gSaveGameData[port].mStubData.mTankStyle = entries[7]->GetValue();
    gSaveGameData[port].mStubData.mVibration =
        entries[8]->GetValue() != 0;
    system->ReturnToPreviousMenu(-1);
    return true;
}

// ea: 0x0056EEB0
bool InGameOptionsMenu::ResponseNoJustGoBackToPauseMenuHelper()
{
    system->ReturnToPreviousMenu(-1);
    return true;
}

// ea: 0x0056EEC0
bool InGameOptionsMenu::ResponseYesApplyNow(int client)
{
    InGameMenuSystem* IGMS = g_femanager.GetIGMS(client);
    return ((InGameOptionsMenu*)IGMS->menus[8])
        ->ResponseYesApplyNowHelper();
}

// ea: 0x0056EEE0
bool InGameOptionsMenu::ResponseNoJustGoBackToPauseMenu(int client)
{
    FEMenu* v1 = g_femanager.GetIGMS(client)->menus[8];
    v1->system->ReturnToPreviousMenu(-1);
    return true;
}

// ea: 0x0057D300
void InGameOptionsMenu::AddOptionsToCombos()
{
    for (const char* const* p = pszStickLayout;
         p < pszStickLayout + (sizeof(pszStickLayout) / sizeof(pszStickLayout[0]));
         ++p)
    {
        Broc::string v16(*p);
        ((FEComboBox*)entries[0])->AddOption(v16);
    }

    for (const char* const* p = pszButtonLayout;
         p < pszButtonLayout + (sizeof(pszButtonLayout) / sizeof(pszButtonLayout[0]));
         ++p)
    {
        Broc::string v16(*p);
        ((FEComboBox*)entries[1])->AddOption(v16);
    }

    for (const char* const* p = pszInvertAim;
         p < pszInvertAim + (sizeof(pszInvertAim) / sizeof(pszInvertAim[0]));
         ++p)
    {
        Broc::string v16(*p);
        ((FEComboBox*)entries[4])->AddOption(v16);
    }

    for (const char* const* p = pszToggleADS;
         p < pszToggleADS + (sizeof(pszToggleADS) / sizeof(pszToggleADS[0]));
         ++p)
    {
        Broc::string v16(*p);
        ((FEComboBox*)entries[5])->AddOption(v16);
    }

    for (const char* const* p = pszStickyAim;
         p < pszStickyAim + (sizeof(pszStickyAim) / sizeof(pszStickyAim[0]));
         ++p)
    {
        Broc::string v16(*p);
        ((FEComboBox*)entries[6])->AddOption(v16);
    }

    for (const char* const* p = pszAlternateTankControls;
         p < pszAlternateTankControls
                 + (sizeof(pszAlternateTankControls)
                    / sizeof(pszAlternateTankControls[0]));
         ++p)
    {
        Broc::string v16(*p);
        ((FEComboBox*)entries[7])->AddOption(v16);
    }

    for (const char* const* p = pszVibration;
         p < pszVibration + (sizeof(pszVibration) / sizeof(pszVibration[0]));
         ++p)
    {
        Broc::string v16(*p);
        ((FEComboBox*)entries[8])->AddOption(v16);
    }
}

// ea: 0x0057D450
void InGameOptionsMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);

    PanelQuad* mQuad = mScrollBarUpFader.mQuad;
    if (mQuad != nullptr && mScrollBarUpFader.mFading)
    {
        if (mScrollBarUpFader.mAlphaTo <= mScrollBarUpFader.mAlpha)
        {
            if (mScrollBarUpFader.mAlpha <= mScrollBarUpFader.mAlphaTo)
                goto up_setalpha;
            mScrollBarUpFader.mAlpha -=
                (time_inc / mScrollBarUpFader.mTime)
                * mScrollBarUpFader.mAlphaDelta;
            if (mScrollBarUpFader.mAlphaTo < mScrollBarUpFader.mAlpha)
                goto up_setalpha;
            mScrollBarUpFader.mAlpha = mScrollBarUpFader.mAlphaTo;
        }
        else
        {
            mScrollBarUpFader.mAlpha +=
                (time_inc / mScrollBarUpFader.mTime)
                * mScrollBarUpFader.mAlphaDelta;
            if (mScrollBarUpFader.mAlpha < mScrollBarUpFader.mAlphaTo)
                goto up_setalpha;
            mScrollBarUpFader.mAlpha = mScrollBarUpFader.mAlphaTo;
        }
        mScrollBarUpFader.mFading = false;
    up_setalpha:
        mQuad->SetAlpha(mScrollBarUpFader.mAlpha);
    }

down_fader:
    PanelQuad* v9 = mScrollBarDownFader.mQuad;
    if (v9 != nullptr && mScrollBarDownFader.mFading)
    {
        if (mScrollBarDownFader.mAlphaTo <= mScrollBarDownFader.mAlpha)
        {
            if (mScrollBarDownFader.mAlpha > mScrollBarDownFader.mAlphaTo)
            {
                mScrollBarDownFader.mAlpha -=
                    (time_inc / mScrollBarDownFader.mTime)
                    * mScrollBarDownFader.mAlphaDelta;
                if (mScrollBarDownFader.mAlphaTo < mScrollBarDownFader.mAlpha)
                {
                    v9->SetAlpha(mScrollBarDownFader.mAlpha);
                    goto helpbar_toggle;
                }
                mScrollBarDownFader.mAlpha = mScrollBarDownFader.mAlphaTo;
                mScrollBarDownFader.mFading = false;
            }
        }
        else
        {
            mScrollBarDownFader.mAlpha +=
                (time_inc / mScrollBarDownFader.mTime)
                * mScrollBarDownFader.mAlphaDelta;
            if (mScrollBarDownFader.mAlpha >= mScrollBarDownFader.mAlphaTo)
            {
                mScrollBarDownFader.mAlpha = mScrollBarDownFader.mAlphaTo;
                mScrollBarDownFader.mFading = false;
            }
        }
        v9->SetAlpha(mScrollBarDownFader.mAlpha);
    }

helpbar_toggle:
    if (highlighted == 9)
    {
        helpbar1->SetShown(false);
        helpbar2->SetShown(true);
    }
    else
    {
        helpbar2->SetShown(false);
        helpbar1->SetShown(true);
    }
}

// ea: 0x0057D630 (thunk)
void InGameOptionsMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    FEMenu::ClearAllButtons();
}

// ea: 0x005851D0
void InGameOptionsMenu::SetHigh(int index, bool anim)
{
    FEMenu::SetHigh(index, anim);
    if (mScrollBarThumb != nullptr)
    {
        int v5 = highlighted < 0 ? 0 : highlighted;
        mScrollBarThumb->SetCenterPos(mScrollBarThumb->GetCenterX(),
                                      mScrollBarYInc * v5 + mScrollBarTopY);
    }
}

// ea: 0x00591B00
void InGameOptionsMenu::OnTriangle(int c)
{
    (void)c;
    int port = LocalClient::ClientToPort(mVersion);
    if (entries[0]->GetValue()
            != gSaveGameData[port].mStubData.mControllerStickConfiguration
        || entries[1]->GetValue()
            != gSaveGameData[port].mStubData.mControllerButtonConfiguration
        || entries[4]->GetValue()
            != gSaveGameData[port].mStubData.mInvertAim
        || entries[5]->GetValue()
            != gSaveGameData[port].mStubData.mAdsToggle
        || entries[6]->GetValue()
            != gSaveGameData[port].mStubData.mStickyAim
        || entries[7]->GetValue()
            != gSaveGameData[port].mStubData.mTankStyle
        || entries[8]->GetValue()
            != gSaveGameData[port].mStubData.mVibration
        || entries[2]->GetValue()
            != gSaveGameData[port].mStubData.mHorizontalSensitivity
        || entries[3]->GetValue()
            != gSaveGameData[port].mStubData.mVerticalSensitivity)
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(mVersion);
        DMS->BringUp("MPGAME_APPLY_PREFERENCES_NOW", false, false,
                     "MPGAME_CONTROLLER", true);
        DialogMenuSystem* v22 = g_femanager.GetDMS(mVersion);
        int v23 = v22->GetActiveMenu();
        ((DialogMenu*)v22->menus[v23 != 0])->AddOption(
            "MPGAME_APPLY_NOW", InGameOptionsMenu::ResponseYesApplyNow);
        DialogMenuSystem* v24 = g_femanager.GetDMS(mVersion);
        int v25 = v24->GetActiveMenu();
        ((DialogMenu*)v24->menus[v25 != 0])->AddOption(
            "MPGAME_DONT_APPLY",
            InGameOptionsMenu::ResponseNoJustGoBackToPauseMenu);
        g_femanager.GetDMS(mVersion);
        DialogMenuSystem* v26 = g_femanager.GetDMS(mVersion);
        v26->menus[-(v26->GetActiveMenu() != 0) == -1]->highlighted = 1;
        DialogMenuDisplay* mDisplay = v26->mDisplay;
        int mOptionCount = mDisplay->mOptionCount;
        if (mOptionCount > 0 && mOptionCount <= 2)
            mDisplay->mOptionSelected = 1;
        mDisplay->SetDialogFlash(1);
        DialogMenuSystem* v29 = g_femanager.GetDMS(mVersion);
        int v30 = v29->GetActiveMenu();
        DialogMenu* Layer = (DialogMenu*)v29->menus[v30 != 0];
        DialogMenuSystem* v31 = g_femanager.GetDMS(Layer->mClient);
        v31->mDisplay->Reformat();
    }
    else
    {
        system->ReturnToPreviousMenu(-1);
    }
}

// ea: 0x00595690
void InGameOptionsMenu::OnUp(int c)
{
    (void)c;
    int highlighted_prev = highlighted;
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
    else
    {
        mScrollBarUpFader.mFading = false;
    }
    UpdateSplitScreenOptions(highlighted_prev);
}

// ea: 0x00595720
void InGameOptionsMenu::OnDown(int c)
{
    (void)c;
    int highlighted_prev = highlighted;
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
    else
    {
        mScrollBarDownFader.mFading = false;
    }
    UpdateSplitScreenOptions(highlighted_prev);
}

// ea: 0x00595200
void InGameOptionsMenu::UpdateSplitScreenOptions(int last_highlighted)
{
    if (mViewport == 0)
        return;

    int v5;
    if (highlighted >= 2)
    {
        if (num_entries - highlighted >= 3)
            v5 = last_highlighted <= highlighted ? highlighted - 2
                                                 : highlighted - 1;
        else
            v5 = num_entries - 4;
    }
    else
    {
        v5 = 0;
    }
    int current = v5;

    for (int v7 = 0; v7 < num_entries; ++v7)
    {
        if (v7 == 2 || v7 == 3)
        {
            FESlider* v12 = (FESlider*)entries[v7];
            v12->SetText(&mSafeText);
            v12->mBarText = &mSafeText;
            if (v12->mBar != nullptr)
                v12->mBar->Mask(
                    (float)v12->mValue / (float)v12->mMax,
                    RIGHT_MASK, 1.0f);
            if (v12->mBarText != nullptr)
            {
                char string[4];
                sprintf(string, "%i%%", v12->mValue);
                v12->mBarText->SetText(string);
            }
        }
        else if (v7 == 9)
        {
            entries[9]->SetText(&mSafeText);
        }
        else
        {
            FEComboBox* v11 = (FEComboBox*)entries[v7];
            v11->text = &mSafeText;
            v11->mLabel = &mSafeText;
            v11->mScrollBarLeftFader.mQuad = nullptr;
            v11->mScrollBarRightFader.mQuad = nullptr;
        }
    }

    int v15 = 1;
    int v52 = 4;
    do
    {
        char name[24];
        sprintf(name, "option_0%i_text_title", v15);
        FEText* TextPointer =
            mSplitScreenMenu->GetTextPointer(name);
        FEText* tempText = TextPointer;
        TextPointer->SetText(pszLabelTexts[current]);

        sprintf(name, "option_0%i_arrow_right", v15);
        PanelQuad* Pointer = mSplitScreenMenu->GetPointer(name);
        sprintf(name, "option_0%i_arrow_left", v15);
        PanelQuad* leftArrow = mSplitScreenMenu->GetPointer(name);
        sprintf(name, "option_0%i_text_edit", v15);
        FEText* selection = mSplitScreenMenu->GetTextPointer(name);
        Pointer->SetShown(true);
        leftArrow->SetShown(true);
        selection->SetShown(true);

        if (current == 2 || current == 3)
        {
            sprintf(name, "option_0%i_text_edit", v15);
            FEText* v34 = mSplitScreenMenu->GetTextPointer(name);
            FESlider* slider = (FESlider*)entries[current];
            slider->SetText(v34);
            slider->mBarText = v34;
            if (slider->mBar != nullptr)
            {
                float mask = (float)slider->mValue / (float)slider->mMax;
                if (mask < 0.0f)
                    mask = 0.0f;
                else if (mask > 1.0f)
                    mask = 1.0f;
                slider->mBar->pqs.mElements[0]->Mask(
                    mask, RIGHT_MASK, 1.0f, slider->mBar->sc_x);
            }
            if (slider->mBarText != nullptr)
            {
                char buf[8];
                sprintf(buf, "%i%%", slider->mValue);
                slider->mBarText->SetText(buf);
            }
        }
        else if (current == 9)
        {
            sprintf(name, "option_0%i_text_title", v15);
            FEText* v25 = mSplitScreenMenu->GetTextPointer(name);
            entries[9]->SetText(v25);
            Pointer->SetShown(false);
            leftArrow->SetShown(false);
            selection->SetShown(false);
        }
        else
        {
            sprintf(name, "option_0%i_text_edit", v15);
            FEText* v27 = mSplitScreenMenu->GetTextPointer(name);
            FEMenuEntry* v28 = entries[current];
            v28->text = v27;
            v28->color_scheme_index = 0;
            leftArrow->SetAlpha(0.5f);
            Pointer->SetAlpha(0.5f);
            entries[current]->SetValue(entries[current]->GetValue());
        }
        entries[current]->AdjustColor();
        ++v15;
        ++current;
        --v52;
    } while (v52 != 1);

    SetHigh(highlighted, true);
    if (mScrollBarThumb != nullptr)
    {
        int v44 = highlighted < 0 ? 0 : highlighted;
        mScrollBarThumb->SetCenterPos(mScrollBarThumb->GetCenterX(),
                                      mScrollBarYInc * v44 + mScrollBarTopY);
    }
}

// ea: 0x0059AC60
void InGameOptionsMenu::SetPanelFile(PanelFile* pf)
{
    if (_stricmp(pf->mName, "MP_SS_PM_options_edit.PANEL") == 0)
        SetPanelFileSplitScreen(pf);
    else if (_stricmp(pf->mName, "MP_PM_controller.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x0059ACB0
void InGameOptionsMenu::SwapMenus()
{
    FESplitScreenMenu::SwapMenus();
    UpdateSplitScreenOptions(highlighted);
}

// ============================================================================
// AARInGameOptionsMenu
// ============================================================================

// ea: 0x0056EF10
AARInGameOptionsMenu::AARInGameOptionsMenu(FEMenuSystem* s)
    : InGameOptionsMenu(s)
{
    mVersion = s->GetCurrentClient();
}

// ea: 0x0056EF70 (copy ctor; asserts - never called)
AARInGameOptionsMenu::AARInGameOptionsMenu(
    const AARInGameOptionsMenu& s)
    : InGameOptionsMenu(s.system)
{
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameOptionsMenu.cpp";
    AeAssert::gCurrentLine = 681;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error("This funtion should not be called."))
        __debugbreak();
}

// ea: 0x0056EFF0
AARInGameOptionsMenu::~AARInGameOptionsMenu()
{
    InGameOptionsMenu::~InGameOptionsMenu();
}

// ea: 0x0056F000
AARInGameOptionsMenu* AARInGameOptionsMenu::Me(int version)
{
    (void)version;
    return (AARInGameOptionsMenu*)
        ((FEMenuSystem*)g_femanager.mAARS)->menus[5];
}

// ea: 0x0056F010
bool AARInGameOptionsMenu::ResponseAARYesApplyNow(int client)
{
    (void)client;
    return ((AARInGameOptionsMenu*)
                ((FEMenuSystem*)g_femanager.mAARS)->menus[5])
        ->ResponseYesApplyNowHelper();
}

// ea: 0x0056F020
bool AARInGameOptionsMenu::ResponseAARNoJustGoBackToPauseMenu(int client)
{
    (void)client;
    FEMenu* v0 = ((FEMenuSystem*)g_femanager.mAARS)->menus[5];
    v0->system->ReturnToPreviousMenu(-1);
    return true;
}

// ea: 0x00585240
void AARInGameOptionsMenu::Update(float time_inc)
{
    InGameOptionsMenu::Update(time_inc);
    SetTimerText();
}

// ea: 0x0057D640
void AARInGameOptionsMenu::SetTimerText()
{
    struct kuju_knet_sTime {
        float mTime;
    };
    kuju_knet_sTime fSecondsLeftTilNextGame;
    fSecondsLeftTilNextGame.mTime =
        (float)g_MPAARTotalTime
        - ((float)MultiplayerMgr::sInst->getLocalTime().mTime
           - (float)g_MPAARTimer.mTime) * 0.001f;
    char szElapsedSeconds[4];
    snprintf(szElapsedSeconds, 3, "%d",
             (int)fSecondsLeftTilNextGame.mTime);
    if (fSecondsLeftTilNextGame.mTime < 10.0f)
        strcpy(&szElapsedSeconds[1], " ");
    FEText* TextPointer = panel->GetTextPointer("text_timer_numbers");
    TextPointer->SetText(szElapsedSeconds);
}

// ea: 0x00591E10
void AARInGameOptionsMenu::OnTriangle(int c)
{
    (void)c;
    int port = LocalClient::ClientToPort(mVersion);
    if (entries[0]->GetValue()
            != gSaveGameData[port].mStubData.mControllerStickConfiguration
        || entries[1]->GetValue()
            != gSaveGameData[port].mStubData.mControllerButtonConfiguration
        || entries[4]->GetValue()
            != gSaveGameData[port].mStubData.mInvertAim
        || entries[5]->GetValue()
            != gSaveGameData[port].mStubData.mAdsToggle
        || entries[6]->GetValue()
            != gSaveGameData[port].mStubData.mStickyAim
        || entries[7]->GetValue()
            != gSaveGameData[port].mStubData.mTankStyle
        || entries[8]->GetValue()
            != gSaveGameData[port].mStubData.mVibration
        || entries[2]->GetValue()
            != (int)((float)gSaveGameData[port]
                         .mStubData.mHorizontalSensitivity
                     * 0.02f * 100.0f)
        || entries[3]->GetValue()
            != (int)((float)gSaveGameData[port]
                         .mStubData.mVerticalSensitivity
                     * 0.02f * 100.0f))
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(mVersion);
        DMS->BringUp("MPGAME_APPLY_SETTINGS_NOW", false, false,
                     "MPGAME_CONTROLLER", true);
        DialogMenuSystem* v22 = g_femanager.GetDMS(mVersion);
        int v23 = v22->GetActiveMenu();
        ((DialogMenu*)v22->menus[v23 != 0])->AddOption(
            "MPGAME_APPLY_NOW",
            AARInGameOptionsMenu::ResponseAARYesApplyNow);
        DialogMenuSystem* v24 = g_femanager.GetDMS(mVersion);
        int v25 = v24->GetActiveMenu();
        ((DialogMenu*)v24->menus[v25 != 0])->AddOption(
            "MPGAME_DONT_APPLY",
            AARInGameOptionsMenu::ResponseAARNoJustGoBackToPauseMenu);
        g_femanager.GetDMS(mVersion);
        DialogMenuSystem* v26 = g_femanager.GetDMS(mVersion);
        v26->menus[-(v26->GetActiveMenu() != 0) == -1]->highlighted = 1;
        DialogMenuDisplay* mDisplay = v26->mDisplay;
        int mOptionCount = mDisplay->mOptionCount;
        if (mOptionCount > 0 && mOptionCount <= 2)
            mDisplay->mOptionSelected = 1;
        mDisplay->SetDialogFlash(1);
        DialogMenuSystem* v29 = g_femanager.GetDMS(mVersion);
        int v30 = v29->GetActiveMenu();
        DialogMenu* Layer = (DialogMenu*)v29->menus[v30 != 0];
        DialogMenuSystem* v31 = g_femanager.GetDMS(Layer->mClient);
        v31->mDisplay->Reformat();
    }
    else
    {
        system->ReturnToPreviousMenu(-1);
    }
}

// ea: 0x005957B0
void AARInGameOptionsMenu::SetPanelFile(PanelFile* pf)
{
    if (strcmp(pf->mName, "MP_AAR_PM_controller.PANEL") == 0)
        SetPanelFileMain(pf);
}

// ea: 0x005957E0
void AARInGameOptionsMenu::OnActivate()
{
    InGameOptionsMenu::OnActivate();
    panel->GetPointer("bkg")->SetShown(true);
    FEText* TextPointer = panel->GetTextPointer("text_timer_numbers");
    TextPointer->SetShown(true);
    FEText* v4 = panel->GetTextPointer("text_timer_text");
    v4->SetShown(true);
    FEText* v5 = panel->GetTextPointer("text_timer_text");
    const char* STBString = STBManager::sInst->GetSTBString(
        "MPGAME_AAR_SECONDS_TIL_NEXT_GAME");
    v5->SetText(STBString);
    FEText* v8 = panel->GetTextPointer("text_title_AAR");
    v8->SetShown(true);
    FEText* v9 = panel->GetTextPointer("text_title_AAR");
    const char* v11 = STBManager::sInst->GetSTBString(
        "MPGAME_AFTER_ACTION_REVIEW");
    v9->SetText(v11);
}

// ============================================================================
// InGameOptionsMenu panel builders
// ============================================================================

// ea: 0x00594900
void InGameOptionsMenu::SetPanelFileMain(PanelFile* pf)
{
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameOptionsMenu.cpp";
        AeAssert::gCurrentLine = 115;
        AeAssert::gCurrentExpr = "pf";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid panel file pointer"))
            __debugbreak();
    }
    bool v3 = mVersion <= 0;
    panel = pf;
    if (!v3)
        panel = pf->Clone();

    FEText* TextPointer = panel->GetTextPointer("text_helpbar");
    FEMultiLineText* v6 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v6 != nullptr)
    {
        color32 col;
        int v47 = TextPointer->GetColor().i;
        float layer = TextPointer->GetScaleX();
        float x1 = TextPointer->GetY();
        float v34 = TextPointer->GetX();
        font_index v5 = TextPointer->GetFont();
        v6 = new (v6) FEMultiLineText(v5, x1, 0.0f, 1,
                                      (panel_layer)layer, 0.0f, 0, v47, col);
    }
    helpbar1 = v6;
    v6->SetNumLines(1);
    helpbar1->SetText("MPGAME_HELPBAR_BACK");

    FEMultiLineText* v9 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v9 != nullptr)
    {
        color32 col2;
        int v48 = TextPointer->GetColor().i;
        float layera = TextPointer->GetScaleX();
        float x1a = TextPointer->GetY();
        float v35 = TextPointer->GetX();
        font_index v8 = TextPointer->GetFont();
        v9 = new (v9) FEMultiLineText(v8, x1a, 0.0f, 1,
                                      (panel_layer)layera, 0.0f, 0, v48,
                                      col2);
    }
    helpbar2 = v9;
    v9->SetNumLines(1);
    helpbar2->SetText("MPFRONTEND_HELP_SELECT_BACK_MOVEUD");

    panel->GetTextPointer("text_title")->SetText("MPGAME_CONTROLLER");
    panel->GetTextPointer("slot_01_text_option")->SetText(pszLabelTexts[0]);
    panel->GetTextPointer("slot_02_text_option")->SetText(pszLabelTexts[1]);
    panel->GetTextPointer("slot_03_text_option")->SetText(pszLabelTexts[2]);
    panel->GetTextPointer("slot_04_text_option")->SetText(pszLabelTexts[3]);
    panel->GetTextPointer("slot_05_text_option")->SetText(pszLabelTexts[4]);
    panel->GetTextPointer("slot_06_text_option")->SetText(pszLabelTexts[5]);
    panel->GetTextPointer("slot_07_text_option")->SetText(pszLabelTexts[6]);
    panel->GetTextPointer("slot_08_text_option")->SetText(pszLabelTexts[7]);
    panel->GetTextPointer("slot_09_text_option")->SetText(pszLabelTexts[8]);
    panel->GetTextPointer("slot_10_text_option")->SetText(pszLabelTexts[9]);

    AddMainComboBox(
        0, 4, panel->GetTextPointer("slot_01_text_spec"),
        panel->GetTextPointer("slot_01_text_option"),
        panel->GetPointer("slot_01_arrow_left"),
        panel->GetPointer("slot_01_arrow_right"));
    AddMainComboBox(
        1, 4, panel->GetTextPointer("slot_02_text_spec"),
        panel->GetTextPointer("slot_02_text_option"),
        panel->GetPointer("slot_02_arrow_left"),
        panel->GetPointer("slot_02_arrow_right"));
    FESlider* v25 = AddMainSlider(
        2, panel->GetPointer("slot_03_gauge_use"),
        panel->GetTextPointer("slot_03_text_option"));
    v25->mMin = 0;
    v25->mMax = 37;
    v25->mEnableSound = false;
    FESlider* v27 = AddMainSlider(
        3, panel->GetPointer("slot_04_gauge_use"),
        panel->GetTextPointer("slot_04_text_option"));
    v27->mMin = 0;
    v27->mMax = 50;
    v27->mEnableSound = false;
    AddMainComboBox(
        4, 2, panel->GetTextPointer("slot_05_text_spec"),
        panel->GetTextPointer("slot_05_text_option"),
        panel->GetPointer("slot_05_arrow_left"),
        panel->GetPointer("slot_05_arrow_right"));
    AddMainComboBox(
        5, 2, panel->GetTextPointer("slot_06_text_spec"),
        panel->GetTextPointer("slot_06_text_option"),
        panel->GetPointer("slot_06_arrow_left"),
        panel->GetPointer("slot_06_arrow_right"));
    AddMainComboBox(
        6, 2, panel->GetTextPointer("slot_07_text_spec"),
        panel->GetTextPointer("slot_07_text_option"),
        panel->GetPointer("slot_07_arrow_left"),
        panel->GetPointer("slot_07_arrow_right"));
    AddMainComboBox(
        7, 2, panel->GetTextPointer("slot_08_text_spec"),
        panel->GetTextPointer("slot_08_text_option"),
        panel->GetPointer("slot_08_arrow_left"),
        panel->GetPointer("slot_08_arrow_right"));
    AddMainComboBox(
        8, 2, panel->GetTextPointer("slot_09_text_spec"),
        panel->GetTextPointer("slot_09_text_option"),
        panel->GetPointer("slot_09_arrow_left"),
        panel->GetPointer("slot_09_arrow_right"));
    AddMainEntry(9, panel->GetTextPointer("slot_10_text_option"));
    AddOptionsToCombos();
    entries[0]->up = 9;
    entries[9]->down = 0;
}

// ea: 0x00594E70
void InGameOptionsMenu::SetPanelFileSplitScreen(PanelFile* pf)
{
    mSplitScreenMenu = pf;
    pf->GetTextPointer("text_title")->SetText("MPGAME_CONTROLLER");

    AddSplitScreenComboBox(
        0, 4, pf->GetTextPointer("option_01_text_edit"),
        pf->GetTextPointer("option_01_text_edit"),
        pf->GetPointer("option_01_arrow_left"),
        pf->GetPointer("option_01_arrow_left"));
    AddSplitScreenComboBox(
        1, 4, pf->GetTextPointer("option_01_text_edit"),
        pf->GetTextPointer("option_02_text_edit"),
        pf->GetPointer("option_02_arrow_left"),
        pf->GetPointer("option_01_arrow_left"));
    AddSplitScreenSlider(
        2, pf->GetTextPointer("option_01_text_title"),
        pf->GetTextPointer("option_01_text_title"));
    AddSplitScreenSlider(
        3, pf->GetTextPointer("option_01_text_title"),
        pf->GetTextPointer("option_01_text_title"));
    AddSplitScreenComboBox(
        4, 2, pf->GetTextPointer("option_01_text_edit"),
        pf->GetTextPointer("option_01_text_edit"),
        pf->GetPointer("option_01_arrow_left"),
        pf->GetPointer("option_01_arrow_left"));
    AddSplitScreenComboBox(
        5, 2, pf->GetTextPointer("option_01_text_edit"),
        pf->GetTextPointer("option_01_text_edit"),
        pf->GetPointer("option_01_arrow_left"),
        pf->GetPointer("option_01_arrow_left"));
    AddSplitScreenComboBox(
        6, 2, pf->GetTextPointer("option_01_text_edit"),
        pf->GetTextPointer("option_01_text_edit"),
        pf->GetPointer("option_01_arrow_left"),
        pf->GetPointer("option_01_arrow_left"));
    AddSplitScreenComboBox(
        7, 2, pf->GetTextPointer("option_01_text_edit"),
        pf->GetTextPointer("option_01_text_edit"),
        pf->GetPointer("option_01_arrow_left"),
        pf->GetPointer("option_01_arrow_left"));
    AddSplitScreenComboBox(
        8, 2, pf->GetTextPointer("option_01_text_edit"),
        pf->GetTextPointer("option_01_text_edit"),
        pf->GetPointer("option_01_arrow_left"),
        pf->GetPointer("option_01_arrow_left"));
    AddSplitScreenEntry(9, pf->GetTextPointer("option_01_text_title"));
    AddOptionsToCombos();

    FEText* v15 = pf->GetTextPointer("option_01_text_edit");
    mSafeText.CopyFrom(v15);
    mSafeText.SetShown(false);
    entries[0]->up = 9;
    entries[9]->down = 0;

    mScrollBarUpFader.mQuad =
        pf->GetPointer("scroll_arrow_up");
    mScrollBarDownFader.mQuad =
        pf->GetPointer("scroll_arrow_down");
    mScrollBarThumb = pf->GetPointer("scroll_indicator");
    Broc::vector min_coords =
        pf->GetPointer("scroll_indicator_reference")->GetMin();
    mScrollBarTopY = (int)min_coords.y;
    Broc::vector max_coords =
        pf->GetPointer("scroll_indicator_reference")->GetMax();
    mScrollBarBottomY = (int)max_coords.y;
    mScrollBarYInc =
        (int)((max_coords.y - min_coords.y) / (num_entries - 1.0f));
    if (mScrollBarThumb != nullptr)
    {
        int v32 = highlighted < 0 ? 0 : highlighted;
        mScrollBarThumb->SetCenterPos(
            mScrollBarThumb->GetCenterX(),
            mScrollBarYInc * v32 + mScrollBarTopY);
    }
}

// ============================================================================
// OptionsGameplayMenu
// ============================================================================

const char* const OptionsGameplayMenu::kGameplayTextGeoms[] = {
    "text_title_main",
    "text_title_profile",
    "text_current_selected",
};  // @ 0xCEF3CC

const char* const OptionsGameplayMenu::kGameplayOptionGeoms[] = {
    "slot_01_text_a",
    "slot_02_text_a",
    "slot_03_text_a",
    "slot_04_text_a",
};  // @ 0xCEF3D8

const char* const OptionsGameplayMenu::kGameplayOptionToggleGeoms[] = {
    "slot_01_text_b",
    "slot_02_text_b",
    "slot_03_text_b",
    "slot_04_text_b",
};  // @ 0xCEF3E8

const char* const OptionsGameplayMenu::kGameplayArrowGeoms[] = {
    "slot_01_arrow_left", "slot_01_arrow_right",
    "slot_02_arrow_left", "slot_02_arrow_right",
    "slot_03_arrow_left", "slot_03_arrow_right",
    "slot_04_arrow_left", "slot_04_arrow_right",
};  // @ 0xCEF3F8

const char* const OptionsGameplayMenu::kGameplayOptionStrings[] = {
    "FEMENU_COP_SUBTITLES",
    "FEMENU_COP_CROSSHAIR",
    "FEMENU_COP_SFRIENDLY_TAGS",
    "FEMENU_COP_STICKYAIM",
};  // @ 0xCEF418

const char* const OptionsGameplayMenu::kGameplayOptionToggleStrings[] = {
    "FEMENU_COP_ON", "FEMENU_COP_OFF",
    "FEMENU_COP_ENABLE", "FEMENU_COP_DISABLE",
    "FEMENU_COP_SHOW", "FEMENU_COP_NO_SHOW",
    "FEMENU_COP_ON", "FEMENU_COP_OFF",
};  // @ 0xCEF428

const char* const OptionsGameplayMenu::kGameplayInstructionStrings[] = {
    "FEMENU_GAMEPLAY_INST_SUBTITLES",
    "FEMENU_GAMEPLAY_INST_CROSSHAIR",
    "FEMENU_GAMEPLAY_INST_FRIENDLYTAGS",
    "FEMENU_GAMEPLAY_INST_STICKYAIM",
};  // @ 0xCEF448

// ea: 0x00592800
OptionsGameplayMenu::OptionsGameplayMenu(FEMenuSystem* s)
    : FEMenu(s, 4, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x82);
    mPanel = nullptr;
    mHelpBar = nullptr;
    mGameplayInstructions = nullptr;
    mOnOffMenu = nullptr;
    mFlashTimer = 0.0f;
    default_color_scheme = 19;
    FEMenu* v3 = (FEMenu*)mem_heap_malloc(16, 0x4Cu);
    FEMenu* v4 = v3 != nullptr
                     ? new (v3) FEMenu(s, 4, 320, 240, 8, 0)
                     : nullptr;
    mOnOffMenu = v4;
    v4->SetDefaultColorScheme(19);
}

// ea: 0x005928C0
OptionsGameplayMenu::~OptionsGameplayMenu()
{
    if (mPanel != nullptr)
    {
        mPanel->~PanelFile();
        mem_heap_free(mPanel);
    }
    mPanel = nullptr;
    if (mHelpBar != nullptr)
        delete mHelpBar;
    mHelpBar = nullptr;
    if (mOnOffMenu != nullptr)
        delete mOnOffMenu;
    mOnOffMenu = nullptr;
    FEMenu::~FEMenu();
}

// ea: 0x005734A0
OptionsGameplayMenu* OptionsGameplayMenu::Me()
{
    return (OptionsGameplayMenu*)g_femanager.fems->menus[19];
}

// ea: 0x005B7930
void OptionsGameplayMenu::OnLeft(int c)
{
    (void)c;
    AdjustOptions(false);
}

// ea: 0x005B79E0
void OptionsGameplayMenu::OnRight(int c)
{
    (void)c;
    AdjustOptions(true);
}

// ea: 0x005734B0
void OptionsGameplayMenu::AdjustOptions(bool up)
{
    mGameplayOptions[highlighted] = !mGameplayOptions[highlighted];
    mOnOffText[highlighted]->SetText(
        kGameplayOptionToggleStrings[2 * highlighted
                                     + !mGameplayOptions[highlighted]]);
    mFlashTimer = 0.1f;
    mOnOffArrows[2 * highlighted]->SetAlpha(up ? 0.5f : 1.0f);
    mOnOffArrows[2 * highlighted + 1]->SetAlpha(up ? 1.0f : 0.5f);
}

// ea: 0x00573580
void OptionsGameplayMenu::OnUp(int c)
{
    (void)c;
    Up();
    mGameplayText[2]->SetText(kGameplayOptionStrings[highlighted]);
    int v4 = mGameplayInstructions->GetBoxWidth();
    mGameplayInstructions->SetTextBox(
        kGameplayInstructionStrings[highlighted], v4, -1.0f);
    mOnOffMenu->SetHigh(highlighted, true);
}

// ea: 0x005735F0
void OptionsGameplayMenu::OnDown(int c)
{
    (void)c;
    Down();
    mGameplayText[2]->SetText(kGameplayOptionStrings[highlighted]);
    int v4 = mGameplayInstructions->GetBoxWidth();
    mGameplayInstructions->SetTextBox(
        kGameplayInstructionStrings[highlighted], v4, -1.0f);
    mOnOffMenu->SetHigh(highlighted, true);
}

// ea: 0x00573660
void OptionsGameplayMenu::SetDefaultOptions()
{
    int selectedText[4];
    mGameplayOptions[0] = gSaveGameData[0].mStubData.mSubtitles;
    selectedText[0] = !gSaveGameData[0].mStubData.mSubtitles;
    mGameplayOptions[1] = gSaveGameData[0].mStubData.mCrosshair;
    selectedText[1] = !gSaveGameData[0].mStubData.mCrosshair + 2;
    mGameplayOptions[2] = gSaveGameData[0].mStubData.mFriendlyTags;
    selectedText[2] = !gSaveGameData[0].mStubData.mFriendlyTags + 4;
    mGameplayOptions[3] = gSaveGameData[0].mStubData.mStickyAim;
    selectedText[3] = !gSaveGameData[0].mStubData.mStickyAim + 6;
    for (int i = 0; i < 4; ++i)
        mOnOffText[i]->SetText(kGameplayOptionToggleStrings[selectedText[i]]);
}

// ea: 0x00573720
void OptionsGameplayMenu::Update(float time_inc)
{
    if (mFlashTimer > 0.0f)
    {
        mFlashTimer -= time_inc;
        if (mFlashTimer <= 0.0f)
        {
            for (int i = 0; i < 4; ++i)
            {
                mOnOffArrows[2 * i]->SetAlpha(0.5f);
                mOnOffArrows[2 * i + 1]->SetAlpha(0.5f);
            }
        }
    }
    FEMenu::Update(time_inc);
    mOnOffMenu->Update(time_inc);
}

// ea: 0x005737A0
void OptionsGameplayMenu::ButtonHeldAction()
{
    if (button_held_down == 4)
        OnUp(0);
    else if (button_held_down == 8)
        OnDown(0);
}

// ea: 0x0057F6C0
void OptionsGameplayMenu::OnActivate()
{
    FEMenu::OnActivate();
    SetHigh(0, false);
    mOnOffMenu->SetHigh(0, false);
    StubData* v2;
    if (gSaveGameData[0].mStubData.mProfileName[0] != 0)
    {
        v2 = &gSaveGameData[0].mStubData;
    }
    else
    {
        v2 = nullptr;
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\OptionsGameplayMenu.cpp";
        AeAssert::gCurrentLine = 131;
        AeAssert::gCurrentExpr = "profile";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("NULL profile being edited"))
            __debugbreak();
    }
    const char* STBString = STBManager::sInst->GetSTBString(
        "MEM_PROFILE_GAMEPLAY");
    char text[256];
    snprintf(text, 0x100, "%s %s", STBString, v2->mProfileName);
    mGameplayText[1]->SetTextNoLocalize(text);
    mGameplayText[2]->SetText(kGameplayOptionStrings[highlighted]);
    int v6 = mGameplayInstructions->GetBoxWidth();
    mGameplayInstructions->SetTextBox(
        kGameplayInstructionStrings[highlighted], v6, -1.0f);
    SetDefaultOptions();
    mFlashTimer = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
        mOnOffArrows[2 * i]->SetAlpha(0.5f);
        mOnOffArrows[2 * i + 1]->SetAlpha(0.5f);
    }
}

// ea: 0x0057F810
void OptionsGameplayMenu::Draw()
{
    mPanel->Draw();
    mHelpBar->Draw(false);
    mOnOffMenu->Draw();
    FEMenu::Draw();
}

// ea: 0x0057F840
void OptionsGameplayMenu::UpdateWidescreen(bool widescreen)
{
    if (mPanel != nullptr)
    {
        mPanel->UpdateWidescreen(widescreen, 320.0f);
        mHelpBar->UpdateForWidescreen(widescreen);
        mHelpBar->SetText("FEMENU_HELPBAR_LRSELECT");
    }
}

// ea: 0x0057F880
void OptionsGameplayMenu::OnTriangle(int c)
{
    (void)c;
    char v2 = 0;
    if (gSaveGameData[0].mStubData.mSubtitles != mGameplayOptions[0])
    {
        gSaveGameData[0].mStubData.mSubtitles = mGameplayOptions[0];
        v2 = 1;
    }
    if (gSaveGameData[0].mStubData.mCrosshair != mGameplayOptions[1])
    {
        gSaveGameData[0].mStubData.mCrosshair = mGameplayOptions[1];
        v2 = 1;
    }
    if (gSaveGameData[0].mStubData.mFriendlyTags != mGameplayOptions[2])
    {
        gSaveGameData[0].mStubData.mFriendlyTags = mGameplayOptions[2];
        v2 = 1;
    }
    if (gSaveGameData[0].mStubData.mStickyAim != mGameplayOptions[3])
    {
        gSaveGameData[0].mStubData.mStickyAim = mGameplayOptions[3];
        *((uint8_t*)g_femanager.fems->menus[29] + 0x4C) = 1;
    }
    else if (v2 == 1)
    {
        *((uint8_t*)g_femanager.fems->menus[29] + 0x4C) = 1;
    }
    system->MakeActive(29);
}

// ea: 0x00596440
void OptionsGameplayMenu::SetPanelFile(PanelFile* pf)
{
    mPanel = pf;
    FEText* TextPointer = pf->GetTextPointer("text_helpbar");
    FEMultiLineText* v5 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v5 != nullptr)
    {
        color32 col;
        int v26 = TextPointer->GetColor().i;
        float layer = TextPointer->GetScaleX();
        float x1 = TextPointer->GetY();
        float v23 = TextPointer->GetX();
        font_index v6 = TextPointer->GetFont();
        v5 = new (v5) FEMultiLineText(v6, x1, 0.0f, 0,
                                      (panel_layer)layer, 0.0f, 0, v26,
                                      col);
    }
    mHelpBar = v5;
    v5->SetNumLines(1);
    mHelpBar->SetText("FEMENU_HELPBAR_LRSELECT");

    for (int i = 0; i < 3; ++i)
        mGameplayText[i] = mPanel->GetTextPointer(kGameplayTextGeoms[i]);
    mGameplayText[0]->SetText("FEMENU_OP_GAMEPLAY_TITLE");
    mGameplayInstructions =
        (FEMultiLineText*)mPanel->GetTextPointer("text_title_instructions");
    for (int i = 0; i < 4; ++i)
    {
        FEText* v12 = mPanel->GetTextPointer(kGameplayOptionGeoms[i]);
        AddEntry(i, v12, false);
        entries[i]->SetText(kGameplayOptionStrings[i]);
    }
    for (int v14 = 0; v14 < 4; ++v14)
    {
        FEText* v16 = mPanel->GetTextPointer(
            kGameplayOptionToggleGeoms[v14]);
        mOnOffText[v14] = v16;
        mOnOffMenu->AddEntry(v14, v16, false);
        mOnOffArrows[2 * v14] =
            mPanel->GetPointer(kGameplayArrowGeoms[2 * v14]);
        mOnOffArrows[2 * v14 + 1] =
            mPanel->GetPointer(kGameplayArrowGeoms[2 * v14 + 1]);
    }
    mPanel->GetPointer("slot_05_text_a")->SetShown(false);
    mPanel->GetPointer("slot_05_text_b")->SetShown(false);
    mPanel->GetPointer("slot_05_arrow_left")->SetShown(false);
    mPanel->GetPointer("slot_05_arrow_right")->SetShown(false);
    mPanel->GetPointer("bkg_row_05")->SetShown(false);
    mPanel->GetPointer("bkg_line_04")->SetShown(false);
}

// ============================================================================
// OptionsControlsMenu
// ============================================================================

const char* const OptionsControlsMenu::kControlsTextGeoms[] = {
    "text_title_main",
    "text_title_profile",
    "text_current_selected",
};  // @ 0xCEF458

const char* const OptionsControlsMenu::kControlsOptionGeoms[] = {
    "slot_01_text_a", "slot_02_text_a", "slot_03_text_a",
    "slot_04_text_a", "slot_05_text_a", "slot_06_text_a",
    "slot_07_text_a", "slot_08_text_a",
};  // @ 0xCEF464

const char* const OptionsControlsMenu::kControlsOptionToggleGeoms[] = {
    "slot_01_text_b", "slot_02_text_b", "slot_03_text_b",
    "slot_04_text_b", "slot_05_text_b", "slot_06_text_b",
    "slot_07_text_b", "slot_08_text_b",
};  // @ 0xCEF484

const char* const OptionsControlsMenu::kControlsArrowGeoms[] = {
    "slot_01_arrow_left", "slot_01_arrow_right",
    "slot_02_arrow_left", "slot_02_arrow_right",
    "slot_03_arrow_left", "slot_03_arrow_right",
    "slot_04_arrow_left", "slot_04_arrow_right",
    "slot_05_arrow_left", "slot_05_arrow_right",
    "slot_06_arrow_left", "slot_06_arrow_right",
    "slot_07_arrow_left", "slot_07_arrow_right",
    "slot_08_arrow_left", "slot_08_arrow_right",
};  // @ 0xCEF4A8

const char* const OptionsControlsMenu::kControlsOptionStrings[] = {
    "FEMENU_COP_STICKLAYOUT",
    "FEMENU_COP_BUTTONLAYOUT",
    "FEMENU_COP_HORIZONTALSENS",
    "FEMENU_COP_VERTICALSENS",
    "FEMENU_COP_INVERTAIM",
    "FEMENU_COP_TOGGLEADS",
    "FEMENU_COP_ALTTANKCONTROL",
    "FEMENU_COP_VIBRATION",
};  // @ 0xCEF4E8

const char* const OptionsControlsMenu::kButtonLayoutStrings[] = {
    "FEMENU_COP_BUTTON_DEFAULT",
    "FEMENU_COP_BUTTON_DEFAULT2",
    "FEMENU_COP_BUTTON_DEFAULT3",
    "FEMENU_COP_BUTTON_DEFAULT4",
};  // ?kButtonLayoutStrings@OptionsControlsMenu@@2QBQBDB @ 0xCEF518

const char* const OptionsControlsMenu::kOptionToggleStrings[] = {
    "FEMENU_COP_ENABLE",
    "FEMENU_COP_DISABLE",
};  // ?kOptionToggleStrings@OptionsControlsMenu@@0QBQBDB @ 0xCEF528

const char* const OptionsControlsMenu::kControlsInstructionStrings[] = {
    "FEMENU_CONTROLS_INST_STICKLAYOUT",
    "FEMENU_CONTROLS_INST_BUTTONLAYOUT",
    "FEMENU_CONTROLS_INST_HORIZONTALSENS",
    "FEMENU_CONTROLS_INST_VERTICALSENS",
    "FEMENU_CONTROLS_INST_INVERTAIM",
    "FEMENU_CONTROLS_INST_TOGGLEADS",
    "FEMENU_CONTROLS_INST_ALTTANKCONTROL",
    "FEMENU_CONTROLS_INST_VIBRATION",
};  // @ 0xCEF530

// ea: 0x00592950
OptionsControlsMenu::OptionsControlsMenu(FEMenuSystem* s)
    : FEMenu(s, 8, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x82);
    mPanel = nullptr;
    mHelpBar = nullptr;
    mInstructionText = nullptr;
    mOnOffMenu = nullptr;
    mHorizontalSensGauge = nullptr;
    mVerticalSensGauge = nullptr;
    mStickVal = 0;
    mButtonVal = 0;
    mHorizontalSensVal = 0;
    mVerticalSensVal = 0;
    mFlashTimer = 0.0f;
    default_color_scheme = 19;
    FEMenu* v3 = (FEMenu*)mem_heap_malloc(16, 0x4Cu);
    FEMenu* v4 = v3 != nullptr
                     ? new (v3) FEMenu(s, 8, 320, 240, 8, 0)
                     : nullptr;
    mOnOffMenu = v4;
    v4->SetDefaultColorScheme(19);
}

// ea: 0x00592A30
OptionsControlsMenu::~OptionsControlsMenu()
{
    if (mPanel != nullptr)
    {
        mPanel->~PanelFile();
        mem_heap_free(mPanel);
    }
    mPanel = nullptr;
    if (mHelpBar != nullptr)
        delete mHelpBar;
    mHelpBar = nullptr;
    if (mOnOffMenu != nullptr)
        delete mOnOffMenu;
    mOnOffMenu = nullptr;
    FEMenu::~FEMenu();
}

// ea: 0x005737D0
OptionsControlsMenu* OptionsControlsMenu::Me()
{
    return (OptionsControlsMenu*)g_femanager.fems->menus[20];
}

// ea: 0x005B7AA0
void OptionsControlsMenu::OnLeft(int c)
{
    (void)c;
    AdjustOptions(false);
}

// ea: 0x005B7AB0
void OptionsControlsMenu::OnRight(int c)
{
    (void)c;
    AdjustOptions(true);
}

// ea: 0x005737E0
void OptionsControlsMenu::Update(float time_inc)
{
    if (mFlashTimer > 0.0f)
    {
        mFlashTimer -= time_inc;
        if (mFlashTimer <= 0.0f)
        {
            for (int i = 0; i < 8; ++i)
            {
                mOnOffArrows[2 * i]->SetAlpha(0.5f);
                mOnOffArrows[2 * i + 1]->SetAlpha(0.5f);
            }
        }
    }
    FEMenu::Update(time_inc);
    mOnOffMenu->Update(time_inc);
}

// ea: 0x00573860
void OptionsControlsMenu::ButtonHeldAction()
{
    flags = (int16_t)(flags & ~0x100);
    if (button_held_down == 4)
    {
        OnUp(0);
    }
    else if (button_held_down == 8)
    {
        OnDown(0);
    }
    else if (highlighted == 2 || highlighted == 3)
    {
        if (button_held_down == 32)
        {
            flags = (int16_t)(flags | 0x100);
            OnRight(0);
        }
        else if (button_held_down == 16)
        {
            flags = (int16_t)(flags | 0x100);
            OnLeft(0);
        }
    }
}

// ea: 0x005738D0
void OptionsControlsMenu::OnUp(int c)
{
    (void)c;
    Up();
    mControlsText[2]->SetText(kControlsOptionStrings[highlighted]);
    int v4 = mInstructionText->GetBoxWidth();
    mInstructionText->SetTextBox(
        kControlsInstructionStrings[highlighted], v4, -1.0f);
    mOnOffMenu->SetHigh(highlighted, true);
    mHorizontalSensGauge->SetAlpha(highlighted == 2 ? 1.0f : 0.5f);
    mVerticalSensGauge->SetAlpha(highlighted == 3 ? 1.0f : 0.5f);
    mHelpBar->SetText(highlighted > 1 ? "FEMENU_HELPBAR_LRSELECT"
                                      : "FEMENU_OP_HELPBAR");
}

// ea: 0x005739D0
void OptionsControlsMenu::OnDown(int c)
{
    (void)c;
    Down();
    mControlsText[2]->SetText(kControlsOptionStrings[highlighted]);
    int v4 = mInstructionText->GetBoxWidth();
    mInstructionText->SetTextBox(
        kControlsInstructionStrings[highlighted], v4, -1.0f);
    mOnOffMenu->SetHigh(highlighted, true);
    mHorizontalSensGauge->SetAlpha(highlighted == 2 ? 1.0f : 0.5f);
    mVerticalSensGauge->SetAlpha(highlighted == 3 ? 1.0f : 0.5f);
    mHelpBar->SetText(highlighted > 1 ? "FEMENU_HELPBAR_LRSELECT"
                                      : "FEMENU_OP_HELPBAR");
}

// ea: 0x00573AD0
void OptionsControlsMenu::Select(int entry_num)
{
    if (entry_num != 0)
    {
        if (entry_num == 1)
            system->MakeActive(25);
    }
    else
    {
        system->MakeActive(24);
    }
}

// ea: 0x00573B00
void OptionsControlsMenu::HighlightDefault(int previousMenu)
{
    bool v3 = previousMenu == 25;
    SetHigh(v3, false);
    mOnOffMenu->SetHigh(v3, false);
    mHelpBar->SetText("FEMENU_OP_HELPBAR");
    mHorizontalSensGauge->SetAlpha(0.5f);
    mVerticalSensGauge->SetAlpha(0.5f);
}

// ea: 0x005867A0
void OptionsControlsMenu::SetDefaultOptions()
{
    mStickVal = gSaveGameData[0].mStubData.mControllerStickConfiguration;
    switch (gSaveGameData[0].mStubData.mControllerStickConfiguration)
    {
    case 0:
        mOnOffText[0]->SetText("FEMENU_COP_STICK_DEFAULT");
        break;
    case 1:
        mOnOffText[0]->SetText("FEMENU_COP_STICK_SOUTHPAW");
        break;
    case 2:
        mOnOffText[0]->SetText("FEMENU_COP_STICK_LEGACY");
        break;
    case 3:
        mOnOffText[0]->SetText("FEMENU_COP_STICK_LEGACYSOUTHPAW");
        break;
    default:
        break;
    }
    mButtonVal = gSaveGameData[0].mStubData.mControllerButtonConfiguration;
    mOnOffText[1]->SetText(kButtonLayoutStrings[mButtonVal]);

    mHorizontalSensVal =
        gSaveGameData[0].mStubData.mHorizontalSensitivity;
    mHorizontalSensGauge->Mask(1.0f, RIGHT_MASK, 1.0f);
    mHorizontalSensGauge->Mask(mHorizontalSensVal * 0.02f,
                               RIGHT_MASK, 1.0f);
    mVerticalSensVal =
        gSaveGameData[0].mStubData.mVerticalSensitivity;
    mVerticalSensGauge->Mask(1.0f, RIGHT_MASK, 1.0f);
    mVerticalSensGauge->Mask(mVerticalSensVal * 0.02f,
                             RIGHT_MASK, 1.0f);

    mToggleVal[0] = gSaveGameData[0].mStubData.mInvertAim;
    mOnOffText[4]->SetText(
        kOptionToggleStrings[!gSaveGameData[0].mStubData.mInvertAim]);
    mToggleVal[1] = gSaveGameData[0].mStubData.mAdsToggle;
    mOnOffText[5]->SetText(
        kOptionToggleStrings[!gSaveGameData[0].mStubData.mAdsToggle]);
    mToggleVal[2] = gSaveGameData[0].mStubData.mTankStyle == 1;
    mOnOffText[6]->SetText(
        kOptionToggleStrings[gSaveGameData[0].mStubData.mTankStyle != 1]);
    mToggleVal[3] = gSaveGameData[0].mStubData.mVibration;
    mOnOffText[7]->SetText(
        kOptionToggleStrings[!gSaveGameData[0].mStubData.mVibration]);
}

// ea: 0x00586980
void OptionsControlsMenu::AdjustOptions(bool up)
{
    switch (highlighted)
    {
    case 2:
        mHorizontalSensVal += 2 * up - 1;
        if (mHorizontalSensVal < 0)
            mHorizontalSensVal = 0;
        if (mHorizontalSensVal > 50)
            mHorizontalSensVal = 50;
        AdjustGauge(mHorizontalSensGauge, mHorizontalSensVal);
        break;
    case 3:
        mVerticalSensVal += 2 * up - 1;
        if (mVerticalSensVal < 0)
            mVerticalSensVal = 0;
        if (mVerticalSensVal > 50)
            mVerticalSensVal = 50;
        AdjustGauge(mVerticalSensGauge, mVerticalSensVal);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        mToggleVal[highlighted - 4] = !mToggleVal[highlighted - 4];
        mOnOffText[highlighted]->SetText(
            kOptionToggleStrings[mToggleVal[highlighted - 4] == 0]);
        break;
    default:
        break;
    }
    mFlashTimer = 0.1f;
    mOnOffArrows[2 * highlighted]->SetAlpha(up ? 0.5f : 1.0f);
    mOnOffArrows[2 * highlighted + 1]->SetAlpha(up ? 1.0f : 0.5f);
}

// ea: 0x0058E630
void OptionsControlsMenu::OnActivate(int previous)
{
    FEMenu::OnActivate();
    HighlightDefault(previous);
    StubData* v3;
    if (gSaveGameData[0].mStubData.mProfileName[0] != 0)
    {
        v3 = &gSaveGameData[0].mStubData;
    }
    else
    {
        v3 = nullptr;
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\OptionsControlsMenu.cpp";
        AeAssert::gCurrentLine = 173;
        AeAssert::gCurrentExpr = "profile";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("NULL profile being edited"))
            __debugbreak();
    }
    const char* STBString = STBManager::sInst->GetSTBString(
        "MEM_PROFILE_CONTROLS");
    char text[256];
    snprintf(text, 0x100, "%s %s", STBString, v3->mProfileName);
    mControlsText[1]->SetTextNoLocalize(text);
    mControlsText[2]->SetText(kControlsOptionStrings[highlighted]);
    int v7 = mInstructionText->GetBoxWidth();
    mInstructionText->SetTextBox(
        kControlsInstructionStrings[highlighted], v7, -1.0f);
    SetDefaultOptions();
    mFlashTimer = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        mOnOffArrows[2 * i]->SetAlpha(0.5f);
        mOnOffArrows[2 * i + 1]->SetAlpha(0.5f);
    }
}

// ea: 0x0057F900
void OptionsControlsMenu::Draw()
{
    mPanel->Draw();
    mHelpBar->Draw(false);
    mOnOffMenu->Draw();
    FEMenu::Draw();
}

// ea: 0x0057F930
void OptionsControlsMenu::UpdateWidescreen(bool widescreen)
{
    if (mPanel != nullptr)
    {
        mPanel->UpdateWidescreen(widescreen, 320.0f);
        mHelpBar->UpdateForWidescreen(widescreen);
        mHorizontalSensGauge->SetXYInitialToCurrentPos();
        mVerticalSensGauge->SetXYInitialToCurrentPos();
    }
}

// ea: 0x0057F980
void OptionsControlsMenu::AdjustGauge(PanelQuad* pq, int setting)
{
    pq->Mask(1.0f, RIGHT_MASK, 1.0f);
    pq->Mask(setting * 0.02f, RIGHT_MASK, 1.0f);
}

// ea: 0x0057F9C0
void OptionsControlsMenu::OnTriangle(int c)
{
    (void)c;
    char v2 = 0;
    if (mHorizontalSensVal
        != gSaveGameData[0].mStubData.mHorizontalSensitivity)
    {
        gSaveGameData[0].mStubData.mHorizontalSensitivity =
            mHorizontalSensVal;
        v2 = 1;
    }
    if (mVerticalSensVal
        != gSaveGameData[0].mStubData.mVerticalSensitivity)
    {
        gSaveGameData[0].mStubData.mVerticalSensitivity = mVerticalSensVal;
        v2 = 1;
    }
    if (mToggleVal[0] != gSaveGameData[0].mStubData.mInvertAim)
    {
        gSaveGameData[0].mStubData.mInvertAim = mToggleVal[0];
        v2 = 1;
    }
    if (mToggleVal[1] != gSaveGameData[0].mStubData.mAdsToggle)
    {
        gSaveGameData[0].mStubData.mAdsToggle = mToggleVal[1];
        v2 = 1;
    }
    if (mToggleVal[2] != (gSaveGameData[0].mStubData.mTankStyle == 1))
    {
        gSaveGameData[0].mStubData.mTankStyle =
            gSaveGameData[0].mStubData.mTankStyle != 1;
        v2 = 1;
    }
    if (mToggleVal[3] != gSaveGameData[0].mStubData.mVibration)
    {
        gSaveGameData[0].mStubData.mVibration = mToggleVal[3];
        *((uint8_t*)g_femanager.fems->menus[29] + 0x4C) = 1;
    }
    else if (v2 == 1)
    {
        *((uint8_t*)g_femanager.fems->menus[29] + 0x4C) = 1;
    }
    system->MakeActive(29);
}

// ea: 0x005966A0
void OptionsControlsMenu::SetPanelFile(PanelFile* pf)
{
    mPanel = pf;
    FEText* TextPointer = pf->GetTextPointer("text_helpbar");
    FEMultiLineText* v5 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v5 != nullptr)
    {
        color32 col;
        int v22 = TextPointer->GetColor().i;
        float layer = TextPointer->GetScaleX();
        float x1 = TextPointer->GetY();
        float v19 = TextPointer->GetX();
        font_index v6 = TextPointer->GetFont();
        v5 = new (v5) FEMultiLineText(v6, x1, 0.0f, 0,
                                      (panel_layer)layer, 0.0f, 0, v22,
                                      col);
    }
    mHelpBar = v5;
    v5->SetNumLines(1);

    for (int i = 0; i < 3; ++i)
        mControlsText[i] = mPanel->GetTextPointer(kControlsTextGeoms[i]);
    mControlsText[0]->SetText("FEMENU_OP_CONTROLS_TITLE");
    mInstructionText =
        (FEMultiLineText*)mPanel->GetTextPointer("text_title_instructions");
    for (int i = 0; i < 8; ++i)
    {
        FEText* v12 = mPanel->GetTextPointer(kControlsOptionGeoms[i]);
        AddEntry(i, v12, false);
        entries[i]->SetText(kControlsOptionStrings[i]);
    }
    for (int v14 = 0; v14 < 8; ++v14)
    {
        FEText* v16 = mPanel->GetTextPointer(
            kControlsOptionToggleGeoms[v14]);
        mOnOffText[v14] = v16;
        mOnOffMenu->AddEntry(v14, v16, false);
        mOnOffArrows[2 * v14] =
            mPanel->GetPointer(kControlsArrowGeoms[2 * v14]);
        mOnOffArrows[2 * v14 + 1] =
            mPanel->GetPointer(kControlsArrowGeoms[2 * v14 + 1]);
    }
    mHorizontalSensGauge =
        mPanel->GetPointer("slot_03_gauge_fill");
    mVerticalSensGauge =
        mPanel->GetPointer("slot_04_gauge_fill");
}

// ============================================================================
// OptionsSoundMenu
// ============================================================================

const char* const OptionsSoundMenu::kSoundTextGeoms[] = {
    "text_title_main",
    "text_title_profile",
    "text_current_selected",
    "text_title_instructions",
};  // @ 0xCEF550

const char* const OptionsSoundMenu::kSoundOptionGeoms[] = {
    "slot_01_text_a",
    "slot_02_text_a",
};  // @ 0xCEF560

const char* const OptionsSoundMenu::kSoundOptionStrings[] = {
    "FEMENU_COP_OUTPUT",
    "FEMENU_COP_VOLUME",
};  // @ 0xCEF568

const char* const OptionsSoundMenu::kSoundToggleStrings[] = {
    "FEMENU_COP_SOUND_STEREO",
    "FEMENU_COP_SOUND_DOLBY",
    "FEMENU_COP_SOUND_5.1",
    "FEMENU_COP_SOUND_MONO",
};  // @ 0xCEF570

const char* const OptionsSoundMenu::kSoundInstructionStrings[] = {
    "FEMENU_SOUND_INST_OUTPUT",
    "FEMENU_SOUND_INST_VOLUME",
};  // @ 0xCEF580

// ea: 0x00592AC0
OptionsSoundMenu::OptionsSoundMenu(FEMenuSystem* s)
    : FEMenu(s, 2, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x82);
    mPanel = nullptr;
    mHelpBar = nullptr;
    mOnOffMenu = nullptr;
    mOutputVal = 0;
    mVolumeVal = 0;
    mFlashTimer = 0.0f;
    default_color_scheme = 19;
    FEMenu* v3 = (FEMenu*)mem_heap_malloc(16, 0x4Cu);
    FEMenu* v4 = v3 != nullptr
                     ? new (v3) FEMenu(s, 1, 320, 240, 8, 0)
                     : nullptr;
    mOnOffMenu = v4;
    v4->SetDefaultColorScheme(19);
}

// ea: 0x00592B80
OptionsSoundMenu::~OptionsSoundMenu()
{
    if (mPanel != nullptr)
    {
        mPanel->~PanelFile();
        mem_heap_free(mPanel);
    }
    mPanel = nullptr;
    if (mHelpBar != nullptr)
        delete mHelpBar;
    mHelpBar = nullptr;
    if (mOnOffMenu != nullptr)
        delete mOnOffMenu;
    mOnOffMenu = nullptr;
    FEMenu::~FEMenu();
}

// ea: 0x00573B80
OptionsSoundMenu* OptionsSoundMenu::Me()
{
    return (OptionsSoundMenu*)g_femanager.fems->menus[22];
}

// ea: 0x005B7AD0
void OptionsSoundMenu::OnLeft(int c)
{
    (void)c;
    AdjustOptions(false);
}

// ea: 0x005B7AE0
void OptionsSoundMenu::OnRight(int c)
{
    (void)c;
    AdjustOptions(true);
}

// ea: 0x00573B90
void OptionsSoundMenu::Update(float time_inc)
{
    if (mFlashTimer > 0.0f)
    {
        mFlashTimer -= time_inc;
        if (mFlashTimer <= 0.0f)
        {
            mOnOffArrows[0]->SetAlpha(0.5f);
            mOnOffArrows[1]->SetAlpha(0.5f);
        }
    }
    FEMenu::Update(time_inc);
    mOnOffMenu->Update(time_inc);
}

// ea: 0x00573BF0
void OptionsSoundMenu::OnUp(int c)
{
    (void)c;
    Up();
    mSoundText[2]->SetText(kSoundOptionStrings[highlighted]);
    mSoundText[3]->SetText(kSoundInstructionStrings[highlighted]);
    mVolumeGauge->SetAlpha(highlighted == 1 ? 1.0f : 0.5f);
    mOnOffMenu->SetHigh(highlighted, false);
}

// ea: 0x00573C80
void OptionsSoundMenu::OnDown(int c)
{
    (void)c;
    Down();
    mSoundText[2]->SetText(kSoundOptionStrings[highlighted]);
    mSoundText[3]->SetText(kSoundInstructionStrings[highlighted]);
    mVolumeGauge->SetAlpha(highlighted == 1 ? 1.0f : 0.5f);
    mOnOffMenu->SetHigh(highlighted, false);
}

// ea: 0x00573D10
void OptionsSoundMenu::ButtonHeldAction()
{
    flags = (int16_t)(flags & ~0x100);
    if (button_held_down == 4)
    {
        OnUp(0);
    }
    else if (button_held_down == 8)
    {
        OnDown(0);
    }
    else if (highlighted == 1)
    {
        if (button_held_down == 32)
        {
            flags = (int16_t)(flags | 0x100);
            OnRight(0);
        }
        else if (button_held_down == 16)
        {
            flags = (int16_t)(flags | 0x100);
            OnLeft(0);
        }
    }
}

// ea: 0x00586B10
void OptionsSoundMenu::SetDefaultOptions()
{
    FEText* TextPointer = mPanel->GetTextPointer("slot_01_text_b");
    TextPointer->SetText("FEMENU_COP_SOUND_STEREO");
    mVolumeVal = gSaveGameData[0].mStubData.mVolume;
    mVolumeGauge->Mask(mVolumeVal * 0.02f, RIGHT_MASK, 1.0f);
}

// ea: 0x00586B60
void OptionsSoundMenu::AdjustOptions(bool up)
{
    if (highlighted != 0)
    {
        if (highlighted == 1)
        {
            mVolumeVal += 2 * up - 1;
            if (mVolumeVal < 0)
                mVolumeVal = 0;
            if (mVolumeVal > 50)
                mVolumeVal = 50;
            mVolumeGauge->Mask(mVolumeVal * 0.02f, RIGHT_MASK, 1.0f);
            MusicMgr::sInst->ScaleVolume(mVolumeVal * 0.02f);
            SoundDevice::sInst->ScaleVolume(mVolumeVal * 0.02f);
        }
    }
    else
    {
        mOutputVal += up ? 1 : -1;
        if (mOutputVal < 0)
            mOutputVal = 0;
        if (mOutputVal > 3)
            mOutputVal = 3;
        mPanel->GetTextPointer("slot_01_text_b")->SetText(
            kSoundToggleStrings[mOutputVal]);
    }
    if (highlighted == 0)
    {
        mFlashTimer = 0.1f;
        mOnOffArrows[0]->SetAlpha(up ? 0.5f : 1.0f);
        mOnOffArrows[1]->SetAlpha(up ? 1.0f : 0.5f);
    }
}

// ea: 0x0058E770
void OptionsSoundMenu::OnActivate()
{
    FEMenu::OnActivate();
    SetHigh(0, false);
    mOnOffMenu->SetHigh(0, false);
    StubData* v2;
    if (gSaveGameData[0].mStubData.mProfileName[0] != 0)
    {
        v2 = &gSaveGameData[0].mStubData;
    }
    else
    {
        v2 = nullptr;
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\OptionsSoundMenu.cpp";
        AeAssert::gCurrentLine = 103;
        AeAssert::gCurrentExpr = "profile";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("NULL profile being edited"))
            __debugbreak();
    }
    const char* STBString = STBManager::sInst->GetSTBString(
        "MEM_PROFILE_SOUND");
    char text[256];
    snprintf(text, 0x100, "%s %s", STBString, v2->mProfileName);
    mSoundText[1]->SetText(text);
    mSoundText[2]->SetText(kSoundOptionStrings[highlighted]);
    mSoundText[3]->SetText(kSoundInstructionStrings[highlighted]);
    mPanel->GetTextPointer("slot_01_text_b")->SetText(
        "FEMENU_COP_SOUND_STEREO");
    mVolumeVal = gSaveGameData[0].mStubData.mVolume;
    mVolumeGauge->Mask(mVolumeVal * 0.02f, RIGHT_MASK, 1.0f);
    mFlashTimer = 0.0f;
    mOnOffArrows[0]->SetAlpha(0.5f);
    mOnOffArrows[1]->SetAlpha(0.5f);
    mVolumeGauge->SetAlpha(0.5f);
}

// ea: 0x0057FA90
void OptionsSoundMenu::Draw()
{
    mPanel->Draw();
    mHelpBar->Draw(false);
    mOnOffMenu->Draw();
    FEMenu::Draw();
}

// ea: 0x0057FAC0
void OptionsSoundMenu::UpdateWidescreen(bool widescreen)
{
    if (mPanel != nullptr)
    {
        mPanel->UpdateWidescreen(widescreen, 320.0f);
        mHelpBar->UpdateForWidescreen(widescreen);
        mHelpBar->SetText("FEMENU_HELPBAR_LRSELECT");
        mVolumeGauge->SetXYInitialToCurrentPos();
    }
}

// ea: 0x0057FB10
void OptionsSoundMenu::AdjustGauge(PanelQuad* pq, int setting)
{
    pq->Mask(setting * 0.02f, RIGHT_MASK, 1.0f);
}

// ea: 0x0057FB40
void OptionsSoundMenu::OnTriangle(int c)
{
    (void)c;
    if (mVolumeVal != gSaveGameData[0].mStubData.mVolume)
    {
        gSaveGameData[0].mStubData.mVolume = mVolumeVal;
        gSaveGameData[0].mStubData.mMusicVolume = mVolumeVal;
        gSaveGameData[0].mStubData.mEffectVolume = mVolumeVal;
        *((uint8_t*)g_femanager.fems->menus[29] + 0x4C) = 1;
    }
    system->MakeActive(29);
}

// ea: 0x00596890
void OptionsSoundMenu::SetPanelFile(PanelFile* pf)
{
    mPanel = pf;
    FEText* TextPointer = pf->GetTextPointer("text_helpbar");
    FEMultiLineText* v5 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v5 != nullptr)
    {
        color32 col;
        int v37 = TextPointer->GetColor().i;
        float layer = TextPointer->GetScaleX();
        float x1 = TextPointer->GetY();
        float v34 = TextPointer->GetX();
        font_index v6 = TextPointer->GetFont();
        v5 = new (v5) FEMultiLineText(v6, x1, 0.0f, 0,
                                      (panel_layer)layer, 0.0f, 0, v37,
                                      col);
    }
    mHelpBar = v5;
    v5->SetNumLines(1);
    mHelpBar->SetText("FEMENU_HELPBAR_LRSELECT");

    for (int i = 0; i < 4; ++i)
        mSoundText[i] = mPanel->GetTextPointer(kSoundTextGeoms[i]);
    mSoundText[0]->SetText("FEMENU_OP_SOUND_TITLE");
    for (int i = 0; i < 2; ++i)
    {
        FEText* v12 = mPanel->GetTextPointer(kSoundOptionGeoms[i]);
        AddEntry(i, v12, false);
        entries[i]->SetText(kSoundOptionStrings[i]);
    }
    FEText* v15 = mPanel->GetTextPointer("slot_01_text_b");
    mOnOffMenu->AddEntry(0, v15, false);
    mOnOffArrows[0] = mPanel->GetPointer("slot_01_arrow_left");
    mOnOffArrows[1] = mPanel->GetPointer("slot_01_arrow_right");
    mVolumeGauge = mPanel->GetPointer("slot_02_gauge_fill");
    mPanel->GetPointer("bkg_line_03")->SetShown(false);
    mPanel->GetPointer("bkg_line_04")->SetShown(false);
    mPanel->GetPointer("bkg_row_04")->SetShown(false);
    mPanel->GetPointer("bkg_row_05")->SetShown(false);
    mPanel->GetPointer("slot_04_text_a")->SetShown(false);
    mPanel->GetPointer("slot_04_gauge_back")->SetShown(false);
    mPanel->GetPointer("slot_04_gauge_edge")->SetShown(false);
    mPanel->GetPointer("slot_04_gauge_fill")->SetShown(false);
    mPanel->GetPointer("slot_05_text_a")->SetShown(false);
    mPanel->GetPointer("slot_05_gauge_back")->SetShown(false);
    mPanel->GetPointer("slot_05_gauge_edge")->SetShown(false);
    mPanel->GetPointer("slot_05_gauge_fill")->SetShown(false);
}

// ============================================================================
// OptionsStickMenu
// ============================================================================

const char* const OptionsStickMenu::kStickTextGeoms[] = {
    "text_title_main",
    "text_current_selected",
    "text_current_instructions",
};  // @ 0xCEF588

const char* const OptionsStickMenu::kStickOptionGeoms[] = {
    "text_option_01", "text_option_02", "text_option_03",
    "text_option_04", "text_option_05", "text_option_06",
};  // @ 0xCEF594

const char* const OptionsStickMenu::kStickTextures[] = {
    "SP_OP_con_xb_stick_A",
    "SP_OP_con_xb_stick_B",
    "SP_OP_con_xb_stick_C",
    "SP_OP_con_xb_stick_D",
};  // @ 0xCEF5AC

const char* const OptionsStickMenu::kStickOptionStrings[] = {
    "SP_OP_con_xb_stick_A",
    "SP_OP_con_xb_stick_B",
    "SP_OP_con_xb_stick_C",
    "SP_OP_con_xb_stick_D",
    "FEMENU_COP_STICK_FORWARD",
    "FEMENU_COP_STICK_STRAFE",
    "FEMENU_COP_STICK_BACK",
    "FEMENU_COP_STICK_LOOKUP",
    "FEMENU_COP_STICK_ROTATE",
    "FEMENU_COP_STICK_LOOKDOWN",
};  // @ 0xCEF5BC

const char* const OptionsStickMenu::kStickSelectedStrings[] = {
    "FEMENU_COP_STICK_DEFAULT",
    "FEMENU_COP_STICK_LEGACY",
    "FEMENU_COP_STICK_SOUTHPAW",
    "FEMENU_COP_STICK_LEGACYSOUTHPAW",
};  // @ 0xCEF5D4

const char* const OptionsStickMenu::kStickInstructionsStrings[] = {
    "FEMENU_COP_STICK_INST_DEFAULT",
    "FEMENU_COP_STICK_INST_LEGACY",
    "FEMENU_COP_STICK_INST_SOUTHPAW",
    "FEMENU_COP_STICK_INST_LEGACYSOUTHPAW",
};  // @ 0xCEF5E4

const int OptionsStickMenu::kStickLayouts[4][6] = {
    {0, 1, 2, 3, 4, 5},
    {0, 4, 2, 3, 1, 5},
    {3, 4, 5, 0, 1, 2},
    {3, 1, 5, 0, 4, 2},
};  // ?kStickLayouts@OptionsStickMenu@@0QAY05$$CBHA @ 0xCEF5F8

// ea: 0x00592C10
OptionsStickMenu::OptionsStickMenu(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mPanel = nullptr;
    mHelpBar = nullptr;
    mFlashTimer = 0.0f;
    mSelectedStickLayout = 0;
    default_color_scheme = 19;
}

// ea: 0x00592C70
OptionsStickMenu::~OptionsStickMenu()
{
    mPanel = nullptr;
    if (mHelpBar != nullptr)
        delete mHelpBar;
    mHelpBar = nullptr;
    FEMenu::~FEMenu();
}

// ea: 0x00573D80
OptionsStickMenu* OptionsStickMenu::Me()
{
    return (OptionsStickMenu*)g_femanager.fems->menus[24];
}

// ea: 0x005B7B20
void OptionsStickMenu::OnLeft(int c)
{
    (void)c;
    AdjustOptions(false);
}

// ea: 0x005B7B30
void OptionsStickMenu::OnRight(int c)
{
    (void)c;
    AdjustOptions(true);
}

// ea: 0x00573D90
void OptionsStickMenu::Update(float time_inc)
{
    if (mFlashTimer > 0.0f)
    {
        mFlashTimer -= time_inc;
        if (mFlashTimer <= 0.0f)
        {
            mOnOffArrows[0]->SetAlpha(0.5f);
            mOnOffArrows[1]->SetAlpha(0.5f);
        }
    }
    FEMenu::Update(time_inc);
}

// ea: 0x00573DF0
void OptionsStickMenu::SetDefaultOptions()
{
    switch (gSaveGameData[controller::inst()->locked_port]
                .mStubData.mControllerStickConfiguration)
    {
    case 0:
        mSelectedStickLayout = 0;
        break;
    case 1:
        mSelectedStickLayout = 2;
        break;
    case 2:
        mSelectedStickLayout = 1;
        break;
    case 3:
        mSelectedStickLayout = 3;
        break;
    default:
        break;
    }
    mStickText[1]->SetText(kStickSelectedStrings[mSelectedStickLayout]);
    mStickText[2]->SetText(kStickInstructionsStrings[mSelectedStickLayout]);
    for (int i = 0; i < 6; ++i)
    {
        if ((mSelectedStickLayout == 2 || mSelectedStickLayout == 3)
            && i == 3)
            mOptionText[3]->SetText("FEMENU_COP_STICK_FORWARDLB");
        else
            mOptionText[i]->SetText(
                kStickOptionStrings[
                    kStickLayouts[mSelectedStickLayout][i]]);
    }
}

// ea: 0x00573ED0
void OptionsStickMenu::AdjustOptions(bool up)
{
    int v4 = mSelectedStickLayout + (up ? 1 : -1);
    mSelectedStickLayout = v4;
    if (v4 < 0)
        v4 = 3;
    mSelectedStickLayout = v4;
    mSelectedStickLayout = v4 > 3 ? 0 : v4;
    for (int i = 0; i < 6; ++i)
    {
        int v6 = mSelectedStickLayout;
        if ((v6 == 2 || v6 == 3) && i == 3)
            mOptionText[3]->SetText("FEMENU_COP_STICK_FORWARDLB");
        else
            mOptionText[i]->SetText(
                kStickOptionStrings[kStickLayouts[v6][i]]);
    }
    mStickText[1]->SetText(kStickSelectedStrings[mSelectedStickLayout]);
    mStickText[2]->SetText(kStickInstructionsStrings[mSelectedStickLayout]);
    tlFixedString name(kStickTextures[mSelectedStickLayout]);
    nglTexture* Texture = cdGetTexture(mPakId, name);
    mStickImage->SetTexture(Texture);
    mFlashTimer = 0.1f;
    mOnOffArrows[up]->SetAlpha(1.0f);
    mOnOffArrows[!up]->SetAlpha(0.5f);
}

// ea: 0x00574030
void OptionsStickMenu::ButtonHeldAction()
{
    if (button_held_down == 32)
        OnRight(0);
    else if (button_held_down == 16)
        OnLeft(0);
}

// ea: 0x0057FB80
void OptionsStickMenu::OnActivate()
{
    mOnOffArrows[0]->SetAlpha(0.5f);
    mOnOffArrows[1]->SetAlpha(0.5f);
    mFlashTimer = 0.0f;
    SetDefaultOptions();
}

// ea: 0x0057FBC0
void OptionsStickMenu::Draw()
{
    mPanel->Draw();
    mHelpBar->Draw(false);
    FEMenu::Draw();
}

// ea: 0x0057FBE0
void OptionsStickMenu::UpdateWidescreen(bool widescreen)
{
    if (mPanel != nullptr)
        mPanel->UpdateWidescreen(widescreen, 320.0f);
    if (mHelpBar != nullptr)
    {
        mHelpBar->UpdateForWidescreen(widescreen);
        mHelpBar->SetText("FEMENU_HELPBAR_LRSELECT");
    }
}

// ea: 0x0057FC30
void OptionsStickMenu::OnTriangle(int c)
{
    (void)c;
    int mSelectedStickLayout = this->mSelectedStickLayout;
    int& stickConfig = gSaveGameData[controller::inst()->locked_port]
                           .mStubData.mControllerStickConfiguration;
    if (mSelectedStickLayout != 0)
    {
        if (mSelectedStickLayout == 1)
        {
            if (stickConfig != 2)
            {
                stickConfig = 2;
                goto dirty;
            }
        }
        else if (mSelectedStickLayout == 2)
        {
            if (stickConfig != 1)
            {
                stickConfig = 1;
                goto dirty;
            }
        }
        else if (mSelectedStickLayout == 3 && stickConfig != 3)
        {
            stickConfig = 3;
            goto dirty;
        }
    }
    else if (stickConfig != 0)
    {
        stickConfig = 0;
    dirty:
        *((uint8_t*)g_femanager.fems->menus[29] + 0x4C) = 1;
    }
    system->MakeActive(20);
}

// ea: 0x00596B30
void OptionsStickMenu::SetPanelFile(PanelFile* pf)
{
    mPanel = pf;
    FEText* TextPointer = pf->GetTextPointer("text_helpbar");
    FEMultiLineText* v5 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v5 != nullptr)
    {
        color32 col;
        int v17 = TextPointer->GetColor().i;
        float layer = TextPointer->GetScaleX();
        float x1 = TextPointer->GetY();
        float v14 = TextPointer->GetX();
        font_index v6 = TextPointer->GetFont();
        v5 = new (v5) FEMultiLineText(v6, x1, 0.0f, 0,
                                      (panel_layer)layer, 0.0f, 0, v17,
                                      col);
    }
    mHelpBar = v5;
    v5->SetNumLines(1);
    mHelpBar->SetText("FEMENU_HELPBAR_LRSELECT");

    for (int i = 0; i < 3; ++i)
        mStickText[i] = mPanel->GetTextPointer(kStickTextGeoms[i]);
    mStickText[0]->SetText("FEMENU_OP_STICK_TITLE");
    for (int i = 0; i < 6; ++i)
        mOptionText[i] = mPanel->GetTextPointer(kStickOptionGeoms[i]);
    mOnOffArrows[0] = mPanel->GetPointer("arrow_left");
    mOnOffArrows[1] = mPanel->GetPointer("arrow_right");
    mPakId = PakManager::sInst->GetPakInfo("mp_FrontEnd")->pakId;
    mStickImage = mPanel->GetPointer("image_stick_layout");
}

// ============================================================================
// OptionsButtonMenu
// ============================================================================

const char* const OptionsButtonMenu::kButtonTextGeoms[] = {
    "text_title_main",
    "text_current_selected",
    "text_current_instructions",
};  // @ 0xCEF658

const char* const OptionsButtonMenu::kButtonOptionGeoms[] = {
    "text_option_01", "text_option_02", "text_option_03",
    "text_option_04", "text_option_05", "text_option_06",
    "text_option_07", "text_option_08", "text_option_09",
    "text_option_10", "text_option_11", "text_option_12",
    "text_option_13", "text_option_14",
};  // @ 0xCEF664

const char* const OptionsButtonMenu::kButtonOptionStrings[] = {
    "FEMENU_COP_BUTTON_ADS",
    "MPFRONTEND_COP_SPRINT",
    "MPFRONTEND_COP_SCOREBOARD",
    "MPFRONTEND_COP_MP_MENU",
    "FEMENU_COP_BUTTON_FIREWEAPON",
    "FEMENU_COP_BUTTON_RELOAD",
    "FEMENU_COP_BUTTON_MELEE",
    "MPFRONTEND_COP_CHANGE_STANCE",
    "MPFRONTEND_COP_JUMP_STANCE_UP",
    "MPFRONTEND_COP_CLASS_ABILITY",
    "FEMENU_COP_BUTTON_SWAPWEAPON",
    "FEMENU_COP_BUTTON_GRENADE",
    " ",
    " ",
};  // @ 0xCEF69C

const char* const OptionsButtonMenu::kButtonSelectedStrings[] = {
    "FEMENU_COP_BUTTON_DEFAULT",
    "FEMENU_COP_BUTTON_SOUTHPAW",
    "FEMENU_COP_BUTTON_SCI_FI",
    "FEMENU_COP_BUTTON_LEGACY",
};  // @ 0xCEF6D4

const char* const OptionsButtonMenu::kButtonInstructionsStrings[] = {
    "FEMENU_COP_BUTTON_INST_DEFAULT",
    "FEMENU_COP_BUTTON_INST_SOUTHPAW",
    "FEMENU_COP_BUTTON_INST_SCI_FI",
    "FEMENU_COP_BUTTON_INST_LEGACY",
};  // @ 0xCEF6E4

const int OptionsButtonMenu::kButtonMapTable[] = {
    216, 220, 27, 153, 215, 211, 214, 213, 212, 217, 218, 219, 156, 157,
};  // ?kButtonMapTable@OptionsButtonMenu@@0QBHB @ 0xCEF6F4

namespace {

// Runtime-initialized action-hash table ($E20_1 static initializer in the
// binary; ae_pair<HashString,int>[19] @ 0xF31250).
struct OptionsButtonActionPair {
    HashString first;
    int        second;
};
OptionsButtonActionPair kActionTable[19];

int InitButtonActionTable()
{
    static const char* const names[19] = {
        "+attack", "+melee", "+grenadeattack", "+class", "weapnext",
        "+weapnext", "+speed", "+binoculars", "togglemenu",
        "togglepaused", "toggle cl_paused", "+reload",
        "+activatereload", "lowerstance", "+stance", "+moveup",
        "+gostand", "+smokegrenade", "+sprintbreath",
    };
    static const int values[19] = {
        4, 6, 11, 9, 10, 10, 0, 1, 2, 3, 3, 5, 5, 7, 7, 8, 8, 11, 1,
    };
    for (int i = 0; i < 19; ++i)
    {
        kActionTable[i].first.mHash = HashString::CalcHash(names[i]);
        kActionTable[i].second = values[i];
    }
    return 0;
}
int g_actionTableInit = InitButtonActionTable();

}  // namespace

// ea: 0x00592CE0
OptionsButtonMenu::OptionsButtonMenu(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x80);
    mPanel = nullptr;
    mHelpBar = nullptr;
    mConfigRead = false;
    mSelectedButtonLayout = 0;
    mFlashTimer = 0.0f;
    default_color_scheme = 19;
}

// ea: 0x00592D40
OptionsButtonMenu::~OptionsButtonMenu()
{
    mPanel = nullptr;
    if (mHelpBar != nullptr)
        delete mHelpBar;
    mHelpBar = nullptr;
    FEMenu::~FEMenu();
}

// ea: 0x00574060
OptionsButtonMenu* OptionsButtonMenu::Me()
{
    return (OptionsButtonMenu*)g_femanager.fems->menus[25];
}

// ea: 0x005B7B70
void OptionsButtonMenu::OnLeft(int c)
{
    (void)c;
    AdjustOptions(false);
}

// ea: 0x005B7B80
void OptionsButtonMenu::OnRight(int c)
{
    (void)c;
    AdjustOptions(true);
}

// ea: 0x00574070
void OptionsButtonMenu::Update(float time_inc)
{
    if (mFlashTimer > 0.0f)
    {
        mFlashTimer -= time_inc;
        if (mFlashTimer <= 0.0f)
        {
            mOnOffArrows[0]->SetAlpha(0.5f);
            mOnOffArrows[1]->SetAlpha(0.5f);
        }
    }
    FEMenu::Update(time_inc);
}

// ea: 0x005740D0
void OptionsButtonMenu::SetDefaultOptions()
{
    controller* v2 = controller::inst();
    int v4 = gSaveGameData[v2->locked_port]
                 .mStubData.mControllerButtonConfiguration;
    mSelectedButtonLayout = v4;
    mButtonText[1]->SetText(kButtonSelectedStrings[v4]);
    mButtonText[2]->SetText(kButtonInstructionsStrings[mSelectedButtonLayout]);
    for (int v5 = 0; v5 < 14; ++v5)
    {
        mOptionText[v5]->SetText(
            kButtonOptionStrings[
                kButtonLayouts[mSelectedButtonLayout][v5]]);
    }
}

// ea: 0x00574160
void OptionsButtonMenu::AdjustOptions(bool up)
{
    int v4 = mSelectedButtonLayout + (up ? 1 : -1);
    mSelectedButtonLayout = v4;
    if (v4 < 0)
        v4 = 3;
    mSelectedButtonLayout = v4;
    mSelectedButtonLayout = v4 > 3 ? 0 : v4;
    for (int i = 0; i < 14; ++i)
    {
        mOptionText[i]->SetText(
            kButtonOptionStrings[kButtonLayouts[mSelectedButtonLayout][i]]);
    }
    mButtonText[1]->SetText(kButtonSelectedStrings[mSelectedButtonLayout]);
    mButtonText[2]->SetText(kButtonInstructionsStrings[mSelectedButtonLayout]);
    mFlashTimer = 0.1f;
    mOnOffArrows[up]->SetAlpha(1.0f);
    mOnOffArrows[!up]->SetAlpha(0.5f);
}

// ea: 0x00574270
void OptionsButtonMenu::ButtonHeldAction()
{
    if (button_held_down == 32)
        OnRight(0);
    else if (button_held_down == 16)
        OnLeft(0);
}

// ea: 0x0057FE20
int OptionsButtonMenu::GetButtonAction(int buttonNum)
{
    int v7 = kButtonMapTable[buttonNum];
    char* mBoundCmdName = KeyInfo_mKeys[currCl][v7].mBoundCmdName;
    unsigned int v4 = HashString::CalcHash(mBoundCmdName);
    for (int i = 0; i < 0x13; ++i)
    {
        if (v4 == kActionTable[i].first.mHash)
            return kActionTable[i].second;
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\OptionsButtonMenu.cpp";
    AeAssert::gCurrentLine = 420;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored())
    {
        if (mBoundCmdName == nullptr)
            mBoundCmdName = "NONE";
        if (AeAssert::Assert(
                "No button mapping found for action '%s'",
                mBoundCmdName))
            __debugbreak();
    }
    return 12;
}

// ea: 0x00586CD0
void OptionsButtonMenu::ReadControllerConfig()
{
    for (int v1 = 0; v1 < 4; ++v1)
    {
        ApplyControllerButtonConfig(v1);
        for (int i = 0; i < 14; ++i)
            kButtonLayouts[v1][i] = GetButtonAction(i);
    }
}

// ea: 0x0058E8F0
void OptionsButtonMenu::OnActivate()
{
    mOnOffArrows[0]->SetAlpha(0.5f);
    mOnOffArrows[1]->SetAlpha(0.5f);
    mFlashTimer = 0.0f;
    if (!mConfigRead)
    {
        ReadControllerConfig();
        mConfigRead = true;
    }
    SetDefaultOptions();
}

// ea: 0x0057FD40
void OptionsButtonMenu::Draw()
{
    mPanel->Draw();
    mHelpBar->Draw(false);
    FEMenu::Draw();
}

// ea: 0x0057FD70
void OptionsButtonMenu::UpdateWidescreen(bool widescreen)
{
    if (mPanel != nullptr)
    {
        mPanel->UpdateWidescreen(widescreen, 320.0f);
        mHelpBar->UpdateForWidescreen(widescreen);
        mHelpBar->SetText("FEMENU_HELPBAR_LRSELECT");
    }
}

// ea: 0x0057FDC0
void OptionsButtonMenu::OnTriangle(int c)
{
    (void)c;
    controller* v3 = controller::inst();
    if (mSelectedButtonLayout
        != gSaveGameData[v3->locked_port]
               .mStubData.mControllerButtonConfiguration)
    {
        gSaveGameData[v3->locked_port]
            .mStubData.mControllerButtonConfiguration =
            mSelectedButtonLayout;
        *((uint8_t*)g_femanager.fems->menus[29] + 0x4C) = 1;
    }
    system->MakeActive(20);
}

// ea: 0x00596CB0
void OptionsButtonMenu::SetPanelFile(PanelFile* pf)
{
    mPanel = pf;
    FEText* TextPointer = pf->GetTextPointer("text_helpbar");
    FEMultiLineText* v5 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v5 != nullptr)
    {
        color32 col;
        int v17 = TextPointer->GetColor().i;
        float layer = TextPointer->GetScaleX();
        float x1 = TextPointer->GetY();
        float v14 = TextPointer->GetX();
        font_index v6 = TextPointer->GetFont();
        v5 = new (v5) FEMultiLineText(v6, x1, 0.0f, 0,
                                      (panel_layer)layer, 0.0f, 0, v17,
                                      col);
    }
    mHelpBar = v5;
    v5->SetNumLines(1);
    mHelpBar->SetText("FEMENU_HELPBAR_LRSELECT");

    for (int i = 0; i < 3; ++i)
        mButtonText[i] = mPanel->GetTextPointer(kButtonTextGeoms[i]);
    mButtonText[0]->SetText("FEMENU_OP_BUTTON_TITLE");
    for (int i = 0; i < 14; ++i)
        mOptionText[i] = mPanel->GetTextPointer(kButtonOptionGeoms[i]);
    mOnOffArrows[0] = mPanel->GetPointer("arrow_left");
    mOnOffArrows[1] = mPanel->GetPointer("arrow_right");
}

// ============================================================================
// GammaScreenMenu
// ============================================================================

// ea: 0x00592DB0
GammaScreenMenu::GammaScreenMenu(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0)
{
    mPanel = nullptr;
    mHelpBar = nullptr;
    mGauge = nullptr;
    mGaugeArrowLeft = nullptr;
    mGaugeArrowRight = nullptr;
    mCurrentTexture = 0;
    mFlashTimer = 0.0f;
    default_color_scheme = 19;
    flags = (int16_t)((flags & 0xFE7F) | 0x80);
}

// ea: 0x00592E20
GammaScreenMenu::~GammaScreenMenu()
{
    mPanel = nullptr;
    if (mHelpBar != nullptr)
        delete mHelpBar;
    mHelpBar = nullptr;
    FEMenu::~FEMenu();
}

// ea: 0x005742A0
GammaScreenMenu* GammaScreenMenu::Me()
{
    return (GammaScreenMenu*)g_femanager.fems->menus[26];
}

// ea: 0x005B7B90
void GammaScreenMenu::Init()
{
}

// ea: 0x005B7BA0
void GammaScreenMenu::OnUp(int c)
{
    (void)c;
}

// ea: 0x005B7BB0
void GammaScreenMenu::OnDown(int c)
{
    (void)c;
}

// ea: 0x005742B0
void GammaScreenMenu::Update(float time_inc)
{
    if (mFlashTimer > 0.0f)
    {
        mFlashTimer -= time_inc;
        if (mFlashTimer <= 0.0f)
        {
            mGaugeArrowLeft->SetAlpha(0.5f);
            mGaugeArrowRight->SetAlpha(0.5f);
        }
    }
    FEMenu::Update(time_inc);
}

// ea: 0x00574310
void GammaScreenMenu::OnTriangle(int c)
{
    (void)c;
    system->MakeActive(29);
}

// ea: 0x00574320
void GammaScreenMenu::OnLeft(int c)
{
    (void)c;
    mFlashTimer = 0.25f;
    mGaugeArrowLeft->SetAlpha(1.0f);
    mGaugeArrowRight->SetAlpha(0.5f);
}

// ea: 0x00574360
void GammaScreenMenu::OnRight(int c)
{
    (void)c;
    mFlashTimer = 0.25f;
    mGaugeArrowLeft->SetAlpha(0.5f);
    mGaugeArrowRight->SetAlpha(1.0f);
}

// ea: 0x005743A0
void GammaScreenMenu::OnSquare(int c)
{
    (void)c;
}

// ea: 0x005743B0
void GammaScreenMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
}

// ea: 0x005743C0
void GammaScreenMenu::ButtonHeldAction()
{
    if (button_held_down == 16)
        OnLeft(0);
    else if (button_held_down == 32)
        OnRight(0);
}

// ea: 0x0057FED0
void GammaScreenMenu::Draw()
{
    mPanel->Draw();
    mHelpBar->Draw(false);
}

// ea: 0x0057FEF0
void GammaScreenMenu::AdjustGauge()
{
    mGauge->Mask((g_GammaRamp - 0.5f) * 0.4f, RIGHT_MASK, 1.0f);
}

// ea: 0x0057FF20
void GammaScreenMenu::OnActivate()
{
    FEMenu::OnActivate();
    mGaugeArrowLeft->SetAlpha(0.5f);
    mGaugeArrowRight->SetAlpha(0.5f);
    AdjustGauge();
}

// ea: 0x0057FF80
void GammaScreenMenu::UpdateWidescreen(bool widescreen)
{
    if (mPanel != nullptr)
    {
        mPanel->UpdateWidescreen(widescreen, 320.0f);
        mHelpBar->UpdateForWidescreen(widescreen);
        mHelpBar->SetText("FEMENU_HELPBAR_BACK");
    }
}

// ea: 0x00596E20
void GammaScreenMenu::SetPanelFile(PanelFile* pf)
{
    mPanel = pf;
    FEText* TextPointer = pf->GetTextPointer("text_helpbar");
    FEMultiLineText* v5 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v5 != nullptr)
    {
        color32 col;
        int v21 = TextPointer->GetColor().i;
        float layer = TextPointer->GetScaleX();
        float x1 = TextPointer->GetY();
        float v18 = TextPointer->GetX();
        font_index v6 = TextPointer->GetFont();
        v5 = new (v5) FEMultiLineText(v6, x1, 0.0f, 0,
                                      (panel_layer)layer, 0.0f, 0, v21,
                                      col);
    }
    mHelpBar = v5;
    v5->SetNumLines(1);
    mHelpBar->SetText("FEMENU_HELPBAR_BACK");
    mPanel->GetTextPointer("text_title_main")->SetText(
        "FEMENU_OP_GAMMA_TITLE");
    mPanel->GetTextPointer("text_title_profile")->SetText(
        "FEMENU_OP_GAMMA_INST");
    mGauge = mPanel->GetPointer("gauge_fill");
    mGaugeArrowLeft = mPanel->GetPointer("gauge_arrow_left");
    mGaugeArrowRight = mPanel->GetPointer("gauge_arrow_right");
    mGauge->SetShown(false);
    mGaugeArrowLeft->SetShown(false);
    mGaugeArrowRight->SetShown(false);
    mPanel->GetPointer("gauge_back_01")->SetShown(false);
    mPanel->GetPointer("gauge_back_02")->SetShown(false);
}

InGameOptionsMenu* InGameOptionsMenu_ctor(void* mem, FEMenuSystem* s)
{
    return new (mem) InGameOptionsMenu(s);
}

OptionsStickMenu* OptionsStickMenu_ctor(void* mem, FEMenuSystem* s)
{
    return new (mem) OptionsStickMenu(s);
}

OptionsButtonMenu* OptionsButtonMenu_ctor(void* mem, FEMenuSystem* s)
{
    return new (mem) OptionsButtonMenu(s);
}

GammaScreenMenu* GammaScreenMenu_ctor(void* mem, FEMenuSystem* s)
{
    return new (mem) GammaScreenMenu(s);
}
