// ============================================================================
// MPMainMenuXBox.h - Xbox multiplayer menu classes (mp_xbox.o mp/ui/*.cpp)
// Reconstructed from IDA local types. All sizes verified.
// ============================================================================

#pragma once

#include "game/ui_types.h"
#include "game/platform_xbox/MPLiveEngine.h"

// ============================================================================
// JoinGameMenu - 0x50 (verified)
// ============================================================================
class JoinGameMenu : public FEMenu {
public:
    bool mSignedInFromInvite;   // +0x4C
    bool mJoiningGame;          // +0x4D

    JoinGameMenu(FEMenuSystem* s);
    static JoinGameMenu* Me();
    void OnActivate();
    void Draw();
    void Update(float time_inc);
    void Select(int entry_num);
    void OnTriangle(int c);
    void SetPanelFile(PanelFile* pf);
};
static_assert(sizeof(JoinGameMenu) == 0x50, "JoinGameMenu size mismatch");

// ============================================================================
// MPMainMenuXBox - 0x160 (verified)
// ============================================================================
class MPMainMenuXBox : public FEMenu {
public:
    bool mWaitingForSignIn;              // +0x4C
    bool mContentAlreadyDiscovered;      // +0x4D
    ae_array<PanelQuad*, 12> m_pBackgroundArt;  // +0x50
    ae_array<FEText*, 3> m_pOptionText;  // +0x80
    UIListBox mListBox;                  // +0x8C
    ae_array<FEText*, 4> m_pText;        // +0x138
    ae_array<PanelQuad*, 3> m_pImages;   // +0x148
    FEMultiLineText* mOptionDescription; // +0x154
    int m_currSelection;                 // +0x158
    bool mWidescreen;                    // +0x15C

    MPMainMenuXBox(FEMenuSystem* s);
    ~MPMainMenuXBox();
    static MPMainMenuXBox* Me();
    void Draw();
    void OnXBoxLive(int c);
    void OnSplitScreen();
    void OnSystemLinkXBox();
    void OnXboxLiveOptions();
    void OnOption(int c);
    void OnTriangle(int c);
    void OnDown(int c);
    void OnUp(int c);
    void UpdateWidescreen(BOOL widescreen);
    void OnLive(int c);
    void OnSystemLink();
    void OnCross(int c);
    void SetPanelFile(PanelFile* pf);
    void SetOptionText();
    void SetImage();
    void OnActivate();
    void Update(float time_inc);
};
static_assert(sizeof(MPMainMenuXBox) == 0x160, "MPMainMenuXBox size mismatch");

// ============================================================================
// XBoxLiveIngameOptionsCOD3 - 0x164 (verified)
// ============================================================================
class XBoxLiveIngameOptionsCOD3 : public FEMenu {
public:
    ae_array<PanelQuad*, 6> m_pBackgroundArt;  // +0x4C
    ae_array<FEText*, 2> m_pText;              // +0x64
    ae_array<PanelQuad*, 4> m_pLineArt;        // +0x6C
    ae_array<color32, 5> m_pOldTextColor;      // +0x7C
    ae_array<color32, 5> m_pOldSelectedTextColor;  // +0x90
    UIListBox m_ListBox;                       // +0xA4
    int m_currSelection;                       // +0x150
    int mVersion;                              // +0x154
    FEComboBox* m_pAppearOnline;               // +0x158
    unsigned int friendIcon;                   // +0x15C
    bool wasSignedIn;                          // +0x160
    bool waitingForSignIn;                     // +0x161
    bool mJoiningFriend;                       // +0x162

    XBoxLiveIngameOptionsCOD3(FEMenuSystem* s);
    ~XBoxLiveIngameOptionsCOD3();
    static XBoxLiveIngameOptionsCOD3* Me(int version);
    void Init();
    void Draw();
    void OnDeactivate(FEMenu* m);
    void OnUp(int c);
    void OnDown(int c);
    void OnLeft(int c);
    void OnRight(int c);
    void OnL1(int c);
    void OnR1(int c);
    void OnCross(int c);
    void OnTriangle(int c);
    void OnCircle(int c);
    void OnSquare(int c);
    void OnStart(int c);
    void ButtonHeldAction();
    void UpdateSplitScreen();
    void WireForSignedOut();
    void SetPanelFile(PanelFile* pf);
    void OnActivate();
    void WireForSignedIn();
    void Update(float time_inc);
    void PanelFileUnloaded(PanelFile* pf);
};
static_assert(sizeof(XBoxLiveIngameOptionsCOD3) == 0x164,
              "XBoxLiveIngameOptionsCOD3 size mismatch");

// ============================================================================
// AARXBoxLiveIngameOptions - 0x164, derives from XBoxLiveIngameOptionsCOD3
// ============================================================================
class AARXBoxLiveIngameOptions : public XBoxLiveIngameOptionsCOD3 {
public:
    AARXBoxLiveIngameOptions(FEMenuSystem* s);
    ~AARXBoxLiveIngameOptions();
    static AARXBoxLiveIngameOptions* Me();
    void OnCross(int c);
    bool SetTimerText();
    void OnActivate();
    void Update(float time_inc);
};
static_assert(sizeof(AARXBoxLiveIngameOptions) == 0x164,
              "AARXBoxLiveIngameOptions size mismatch");
