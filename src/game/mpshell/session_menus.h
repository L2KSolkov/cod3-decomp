// ============================================================================
// session_menus.h - mp_shell.o session/overlay menu classes
// Reconstructed from IDA local types (PDB symbol data). Sizes verified.
// ============================================================================

#pragma once

#include "game/shell/shell_types.h"

struct sServerCreateParams;  // full definition in session_menus.cpp

// ============================================================================
// CreateSessionMenu - 344 bytes (0x158), verified against IDA
// ============================================================================
class CreateSessionMenu : public FEMenu {
public:
    ae_array<PanelQuad*, 6> m_pBackgroundArt;      // +0x4C
    ae_array<FEText*, 4>    m_pText;               // +0x64
    ae_array<FEText*, 8>    m_pSlotText;           // +0x74
    ae_array<PanelQuad*, 8> m_pSlotGeometry;       // +0x94
    int  m_iNumberOfPlayersConversion[30];         // +0xB4
    FEComboBox* mNumberOfPlayersCombo;             // +0x12C
    FEComboBox* mGameModeCombo;                    // +0x130
    FEComboBox* mStartingMapCombo;                 // +0x134
    FEComboBox* m_PrivateSlotsCombo;               // +0x138
    int  mLastGameType;                            // +0x13C
    int  mLastMap;                                 // +0x140
    char m_szSessionName[16];                      // +0x144
    int  m_LastPlayerCount;                        // +0x154

protected:
    static unsigned char m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@CreateSessionMenu@@1EA @ 0x1388D54
public:

    CreateSessionMenu(FEMenuSystem* s);  // ??0CreateSessionMenu@@QAE@PAVFEMenuSystem@@@Z
    static CreateSessionMenu* Me();      // ?Me@CreateSessionMenu@@SAPAV1@XZ
    void SetGameTypeDefaults();          // ?SetGameTypeDefaults@CreateSessionMenu@@QAEXXZ
    void SetMapDefaults();               // ?SetMapDefaults@CreateSessionMenu@@QAEXXZ
protected:
    void UpdatePrivateSlots();           // ?UpdatePrivateSlots@CreateSessionMenu@@IAEXXZ
    void UpdateHighlight();              // ?UpdateHighlight@CreateSessionMenu@@IAEXXZ
    void GrabSessionName();              // ?GrabSessionName@CreateSessionMenu@@IAEXXZ
    void UpdateNetworking();             // ?UpdateNetworking@CreateSessionMenu@@IAEXXZ

public:
    virtual ~CreateSessionMenu();        // ??1CreateSessionMenu@@UAE@XZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@CreateSessionMenu@@UAEXPAVPanelFile@@@Z
    virtual void Init();                 // ?Init@CreateSessionMenu@@UAEXXZ
    virtual void Draw();                 // ?Draw@CreateSessionMenu@@UAEXXZ
    virtual void Update(float time_inc); // ?Update@CreateSessionMenu@@UAEXM@Z
    virtual void OnCircle(int c);        // ?OnCircle@CreateSessionMenu@@UAEXH@Z
    virtual void OnTriangle(int c);      // ?OnTriangle@CreateSessionMenu@@UAEXH@Z
    virtual void OnSquare(int c);        // ?OnSquare@CreateSessionMenu@@UAEXH@Z
    virtual void OnUp(int c);            // ?OnUp@CreateSessionMenu@@UAEXH@Z
    virtual void OnDown(int c);          // ?OnDown@CreateSessionMenu@@UAEXH@Z
    virtual void OnLeft(int c);          // ?OnLeft@CreateSessionMenu@@UAEXH@Z
    virtual void OnRight(int c);         // ?OnRight@CreateSessionMenu@@UAEXH@Z
    virtual void OnDeactivate(FEMenu* m);// ?OnDeactivate@CreateSessionMenu@@UAEXPAVFEMenu@@@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@CreateSessionMenu@@UAEXPAVPanelFile@@@Z
};
static_assert(sizeof(CreateSessionMenu) == 0x158,
              "CreateSessionMenu size mismatch");

// ============================================================================
// CreateLanSessionMenu - 312 bytes (0x138), verified against IDA
// ============================================================================
class CreateLanSessionMenu : public FEMenu {
public:
    ae_array<PanelQuad*, 4> m_pBackgroundArt;      // +0x4C
    ae_array<FEText*, 4>    m_pText;               // +0x5C
    ae_array<FEText*, 6>    m_pSlotText;           // +0x6C
    ae_array<PanelQuad*, 6> m_pSlotGeometry;       // +0x84
    int  m_iNumberOfPlayersConversion[30];         // +0x9C
    FEComboBox* mNumberOfPlayersCombo;             // +0x114
    FEComboBox* mGameModeCombo;                    // +0x118
    FEComboBox* mStartingMapCombo;                 // +0x11C
    int  mLastGameType;                            // +0x120
    int  mLastMap;                                 // +0x124
    char m_szSessionName[16];                      // +0x128

protected:
    static unsigned char m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@CreateLanSessionMenu@@1EA @ 0x1388D55
public:

    CreateLanSessionMenu(FEMenuSystem* s);  // ??0CreateLanSessionMenu@@QAE@PAVFEMenuSystem@@@Z
    static CreateLanSessionMenu* Me();      // ?Me@CreateLanSessionMenu@@SAPAV1@XZ
    void SetGameTypeDefaults();             // ?SetGameTypeDefaults@CreateLanSessionMenu@@QAEXXZ
    void SetMapDefaults();                  // ?SetMapDefaults@CreateLanSessionMenu@@QAEXXZ
protected:
    void GrabSessionName(int c);            // ?GrabSessionName@CreateLanSessionMenu@@IAEXH@Z
    void UpdateNetworking();                // ?UpdateNetworking@CreateLanSessionMenu@@IAEXXZ

public:
    virtual ~CreateLanSessionMenu();        // ??1CreateLanSessionMenu@@UAE@XZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@CreateLanSessionMenu@@UAEXPAVPanelFile@@@Z
    virtual void Init();                    // ?Init@CreateLanSessionMenu@@UAEXXZ
    virtual void Draw();                    // ?Draw@CreateLanSessionMenu@@UAEXXZ
    virtual void Update(float time_inc);    // ?Update@CreateLanSessionMenu@@UAEXM@Z
    virtual void OnCircle(int c);           // ?OnCircle@CreateLanSessionMenu@@UAEXH@Z
    virtual void OnTriangle(int c);         // ?OnTriangle@CreateLanSessionMenu@@UAEXH@Z
    virtual void OnSquare(int c);           // ?OnSquare@CreateLanSessionMenu@@UAEXH@Z
    virtual void OnUp(int c);               // ?OnUp@CreateLanSessionMenu@@UAEXH@Z
    virtual void OnDown(int c);             // ?OnDown@CreateLanSessionMenu@@UAEXH@Z
    virtual void OnLeft(int c);             // ?OnLeft@CreateLanSessionMenu@@UAEXH@Z
    virtual void OnRight(int c);            // ?OnRight@CreateLanSessionMenu@@UAEXH@Z
    virtual void OnDeactivate(FEMenu* m);   // ?OnDeactivate@CreateLanSessionMenu@@UAEXPAVFEMenu@@@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@CreateLanSessionMenu@@UAEXPAVPanelFile@@@Z
};
static_assert(sizeof(CreateLanSessionMenu) == 0x138,
              "CreateLanSessionMenu size mismatch");

// ============================================================================
// FindSessionMenu - 268 bytes (0x10C), verified against IDA
// ============================================================================
class FindSessionMenu : public FEMenu {
public:
    ae_array<PanelQuad*, 6> m_pBackgroundArt;      // +0x4C
    ae_array<FEText*, 4>    m_pText;               // +0x64
    ae_array<FEText*, 10>   m_pSlotText;           // +0x74
    ae_array<PanelQuad*, 12> m_pSlotGeometry;      // +0x9C
    ae_array<PanelQuad*, 6> m_pBackgroundRow;      // +0xCC
    ae_array<PanelQuad*, 5> m_pBackgroundLine;     // +0xE4
    FEComboBox* mNumberOfPlayersCombo;             // +0xF8
    FEComboBox* mGameModeCombo;                    // +0xFC
    FEComboBox* mStartingMapCombo;                 // +0x100
    FEComboBox* m_AutoTeamBalanceCombo;            // +0x104
    FEComboBox* m_TeamDamageCombo;                 // +0x108

protected:
    static unsigned char m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@FindSessionMenu@@1EA @ 0x1388D56
public:

    FindSessionMenu(FEMenuSystem* s);  // ??0FindSessionMenu@@QAE@PAVFEMenuSystem@@@Z
    static FindSessionMenu* Me();      // ?Me@FindSessionMenu@@SAPAV1@XZ
protected:
    void UpdateNetworking();           // ?UpdateNetworking@FindSessionMenu@@IAEXXZ
    void UpdateHighlight();            // ?UpdateHighlight@FindSessionMenu@@IAEXXZ

public:
    virtual ~FindSessionMenu();        // ??1FindSessionMenu@@UAE@XZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@FindSessionMenu@@UAEXPAVPanelFile@@@Z
    virtual void Init();               // ?Init@FindSessionMenu@@UAEXXZ
    virtual void Draw();               // ?Draw@FindSessionMenu@@UAEXXZ
    virtual void Update(float time_inc);  // ?Update@FindSessionMenu@@UAEXM@Z
    virtual void OnTriangle(int c);    // ?OnTriangle@FindSessionMenu@@UAEXH@Z
    virtual void OnUp(int c);          // ?OnUp@FindSessionMenu@@UAEXH@Z
    virtual void OnDown(int c);        // ?OnDown@FindSessionMenu@@UAEXH@Z
    virtual void OnDeactivate(FEMenu* m);  // ?OnDeactivate@FindSessionMenu@@UAEXPAVFEMenu@@@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@FindSessionMenu@@UAEXPAVPanelFile@@@Z
};
static_assert(sizeof(FindSessionMenu) == 0x10C,
              "FindSessionMenu size mismatch");

// ============================================================================
// FindLanSessionMenu - 260 bytes (0x104), verified against IDA
// ============================================================================
class FindLanSessionMenu : public FEMenu {
public:
    ae_array<PanelQuad*, 4> m_pBackgroundArt;      // +0x4C
    ae_array<FEText*, 4>    m_pText;               // +0x5C
    ae_array<FEText*, 10>   m_pSlotText;           // +0x6C
    ae_array<PanelQuad*, 12> m_pSlotGeometry;      // +0x94
    ae_array<PanelQuad*, 6> m_pBackgroundRow;      // +0xC4
    ae_array<PanelQuad*, 5> m_pBackgroundLine;     // +0xDC
    FEComboBox* mNumberOfPlayersCombo;             // +0xF0
    FEComboBox* mGameModeCombo;                    // +0xF4
    FEComboBox* mStartingMapCombo;                 // +0xF8
    FEComboBox* m_AutoTeamBalanceCombo;            // +0xFC
    FEComboBox* m_TeamDamageCombo;                 // +0x100

protected:
    static unsigned char m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@FindLanSessionMenu@@1EA @ 0x1388D57
public:

    FindLanSessionMenu(FEMenuSystem* s);  // ??0FindLanSessionMenu@@QAE@PAVFEMenuSystem@@@Z
    static FindLanSessionMenu* Me();      // ?Me@FindLanSessionMenu@@SAPAV1@XZ
protected:
    void UpdateNetworking();              // ?UpdateNetworking@FindLanSessionMenu@@IAEXXZ
    void UpdateHighlight();               // ?UpdateHighlight@FindLanSessionMenu@@IAEXXZ

public:
    virtual ~FindLanSessionMenu();        // ??1FindLanSessionMenu@@UAE@XZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@FindLanSessionMenu@@UAEXPAVPanelFile@@@Z
    virtual void Init();                  // ?Init@FindLanSessionMenu@@UAEXXZ
    virtual void Draw();                  // ?Draw@FindLanSessionMenu@@UAEXXZ
    virtual void Update(float time_inc);  // ?Update@FindLanSessionMenu@@UAEXM@Z
    virtual void OnTriangle(int c);       // ?OnTriangle@FindLanSessionMenu@@UAEXH@Z
    virtual void OnUp(int c);             // ?OnUp@FindLanSessionMenu@@UAEXH@Z
    virtual void OnDown(int c);           // ?OnDown@FindLanSessionMenu@@UAEXH@Z
    virtual void OnDeactivate(FEMenu* m); // ?OnDeactivate@FindLanSessionMenu@@UAEXPAVFEMenu@@@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@FindLanSessionMenu@@UAEXPAVPanelFile@@@Z
};
static_assert(sizeof(FindLanSessionMenu) == 0x104,
              "FindLanSessionMenu size mismatch");

// ============================================================================
// CreateSessionAdvancedMenu / CreateLanSessionAdvancedMenu - 288 (0x120)
// ============================================================================
class CreateSessionAdvancedMenu : public FEMenu {
public:
    ae_array<PanelQuad*, 4> m_pBackgroundArt;      // +0x4C
    ae_array<PanelQuad*, 6> m_pBackgroundRow;      // +0x5C
    ae_array<PanelQuad*, 5> m_pBackgroundLine;     // +0x74
    ae_array<FEText*, 4>    m_pText;               // +0x88
    ae_array<FEText*, 12>   m_pSlotText;           // +0x98
    ae_array<PanelQuad*, 12> m_pSlotArrow;         // +0xC8
    FEComboBox* m_TimeLimitCombo;                  // +0xF8
    FEComboBox* m_ScoreLimitCombo;                 // +0xFC
    FEComboBox* m_AutoTeamBalanceCombo;            // +0x100
    FEComboBox* m_TeamDamageCombo;                 // +0x104
    FEComboBox* m_VotingCombo;                     // +0x108
    FEComboBox* m_PenaltyVoteCombo;                // +0x10C
    char m_szSessionName[16];                      // +0x110

    CreateSessionAdvancedMenu(FEMenuSystem* s);  // ??0CreateSessionAdvancedMenu@@QAE@PAVFEMenuSystem@@@Z
    static CreateSessionAdvancedMenu* Me();      // ?Me@CreateSessionAdvancedMenu@@SAPAV1@XZ
    void GrabSessionName();                      // ?GrabSessionName@CreateSessionAdvancedMenu@@IAEXXZ
    virtual ~CreateSessionAdvancedMenu();        // ??1CreateSessionAdvancedMenu@@UAE@XZ
    virtual void OnDeactivate(FEMenu* m);        // ?OnDeactivate@CreateSessionAdvancedMenu@@UAEXPAVFEMenu@@@Z
    virtual void Draw();                         // ?Draw@CreateSessionAdvancedMenu@@UAEXXZ
    virtual void Update(float time_inc);         // ?Update@CreateSessionAdvancedMenu@@UAEXM@Z
    virtual void OnTriangle(int c);              // ?OnTriangle@CreateSessionAdvancedMenu@@UAEXH@Z
    virtual void OnActivate();                   // ?OnActivate@CreateSessionAdvancedMenu@@UAEXXZ
    virtual void SetPanelFile(PanelFile* pf);    // ?SetPanelFile@CreateSessionAdvancedMenu@@UAEXPAVPanelFile@@@Z
};
static_assert(sizeof(CreateSessionAdvancedMenu) == 0x120,
              "CreateSessionAdvancedMenu size mismatch");

class CreateLanSessionAdvancedMenu : public FEMenu {
public:
    ae_array<PanelQuad*, 4> m_pBackgroundArt;      // +0x4C
    ae_array<PanelQuad*, 6> m_pBackgroundRow;      // +0x5C
    ae_array<PanelQuad*, 5> m_pBackgroundLine;     // +0x74
    ae_array<FEText*, 4>    m_pText;               // +0x88
    ae_array<FEText*, 12>   m_pSlotText;           // +0x98
    ae_array<PanelQuad*, 12> m_pSlotArrow;         // +0xC8
    FEComboBox* m_TimeLimitCombo;                  // +0xF8
    FEComboBox* m_ScoreLimitCombo;                 // +0xFC
    FEComboBox* m_AutoTeamBalanceCombo;            // +0x100
    FEComboBox* m_TeamDamageCombo;                 // +0x104
    FEComboBox* m_VotingCombo;                     // +0x108
    FEComboBox* m_PenaltyVoteCombo;                // +0x10C
    char m_szSessionName[16];                      // +0x110

    CreateLanSessionAdvancedMenu(FEMenuSystem* s);  // ??0CreateLanSessionAdvancedMenu@@QAE@PAVFEMenuSystem@@@Z
    static CreateLanSessionAdvancedMenu* Me();      // ?Me@CreateLanSessionAdvancedMenu@@SAPAV1@XZ
    void GrabSessionName();                         // ?GrabSessionName@CreateLanSessionAdvancedMenu@@IAEXXZ
    virtual ~CreateLanSessionAdvancedMenu();        // ??1CreateLanSessionAdvancedMenu@@UAE@XZ
    virtual void OnDeactivate(FEMenu* m);           // ?OnDeactivate@CreateLanSessionAdvancedMenu@@UAEXPAVFEMenu@@@Z
    virtual void Draw();                            // ?Draw@CreateLanSessionAdvancedMenu@@UAEXXZ
    virtual void Update(float time_inc);            // ?Update@CreateLanSessionAdvancedMenu@@UAEXM@Z
    virtual void OnTriangle(int c);                 // ?OnTriangle@CreateLanSessionAdvancedMenu@@UAEXH@Z
    virtual void OnUp(int c);                       // ?OnUp@CreateLanSessionAdvancedMenu@@UAEXH@Z
    virtual void OnDown(int c);                     // ?OnDown@CreateLanSessionAdvancedMenu@@UAEXH@Z
    virtual void OnActivate();                      // ?OnActivate@CreateLanSessionAdvancedMenu@@UAEXXZ
    virtual void SetPanelFile(PanelFile* pf);       // ?SetPanelFile@CreateLanSessionAdvancedMenu@@UAEXPAVPanelFile@@@Z
};
static_assert(sizeof(CreateLanSessionAdvancedMenu) == 0x120,
              "CreateLanSessionAdvancedMenu size mismatch");

// ============================================================================
// GameSettingsEdit / GameSettingsView - split-screen settings menus
// ============================================================================
class GameSettingsView : public FESplitScreenMenu {
public:
    int mScrollBarTopY;                  // +0x68
    int mScrollBarBottomY;               // +0x6C
    int mScrollBarYInc;                  // +0x70
    PanelQuad* mScrollBarThumb;          // +0x74
    FEText mSafeText;                    // +0x78
    PanelQuadFader mScrollBarUpFader;    // +0xE8
    PanelQuadFader mScrollBarDownFader;  // +0x100
    sServerCreateParams* mCurrentServerParams;  // +0x118

    virtual void Init();                 // ?Init@GameSettingsView@@UAEXXZ
    void OnDeactivate(FESplitScreenMenu* m);  // ?OnDeactivate@GameSettingsView@@QAEXPAVFESplitScreenMenu@@@Z
    virtual void OnLeft(int c);          // ?OnLeft@GameSettingsView@@UAEXH@Z
    virtual void OnRight(int c);         // ?OnRight@GameSettingsView@@UAEXH@Z
    virtual void OnL1(int c);            // ?OnL1@GameSettingsView@@UAEXH@Z
    virtual void OnR1(int c);            // ?OnR1@GameSettingsView@@UAEXH@Z
    virtual void OnCross(int c);         // ?OnCross@GameSettingsView@@UAEXH@Z
    virtual void OnTriangle(int c);      // ?OnTriangle@GameSettingsView@@UAEXH@Z
    virtual void OnCircle(int c);        // ?OnCircle@GameSettingsView@@UAEXH@Z
    virtual void OnSquare(int c);        // ?OnSquare@GameSettingsView@@UAEXH@Z
protected:
    virtual void ButtonHeldAction();     // ?ButtonHeldAction@GameSettingsView@@MAEXXZ
    void SetPanelFileMain(PanelFile* pf);  // ?SetPanelFileMain@GameSettingsView@@IAEXPAVPanelFile@@@Z
};
static_assert(sizeof(GameSettingsView) == 0x11C,
              "GameSettingsView size mismatch");

class GameSettingsEdit : public FESplitScreenMenu {
public:
    int mVersion;                        // +0x68
    sServerCreateParams* mNextServerParams;  // +0x6C
    int iLastOptionSelected;             // +0x70
    int mLastGameType;                   // +0x74
    int mScrollBarTopY;                  // +0x78
    int mScrollBarBottomY;               // +0x7C
    int mScrollBarYInc;                  // +0x80
    PanelQuad* mScrollBarThumb;          // +0x84
    FEText mSafeText;                    // +0x88
    PanelQuadFader mScrollBarUpFader;    // +0xF8
    PanelQuadFader mScrollBarDownFader;  // +0x110
    unsigned char m_ucLastHighlighted;   // +0x128
    unsigned char m_FirstTimeAccessedByte;  // +0x129

    virtual void Init();                 // ?Init@GameSettingsEdit@@UAEXXZ
    virtual void OnL1(int c);            // ?OnL1@GameSettingsEdit@@UAEXH@Z
    virtual void OnR1(int c);            // ?OnR1@GameSettingsEdit@@UAEXH@Z
    virtual void OnCross(int c);         // ?OnCross@GameSettingsEdit@@UAEXH@Z
    virtual void OnCircle(int c);        // ?OnCircle@GameSettingsEdit@@UAEXH@Z
    virtual void OnSquare(int c);        // ?OnSquare@GameSettingsEdit@@UAEXH@Z
    virtual void OnStart(int c);         // ?OnStart@GameSettingsEdit@@UAEXH@Z
protected:
    bool ResponseNoJustGoBackToPauseMenuHelper();  // ?ResponseNoJustGoBackToPauseMenuHelper@GameSettingsEdit@@IAE_NXZ
    virtual void ButtonHeldAction();     // ?ButtonHeldAction@GameSettingsEdit@@MAEXXZ
    void SetPanelFileMain(PanelFile* pf);  // ?SetPanelFileMain@GameSettingsEdit@@IAEXPAVPanelFile@@@Z
};
static_assert(sizeof(GameSettingsEdit) == 0x12C,
              "GameSettingsEdit size mismatch");

// AAR variants (same layouts, own vtables)
class AARGameSettingsEdit : public GameSettingsEdit {
public:
    static AARGameSettingsEdit* Me(int version);  // ?Me@AARGameSettingsEdit@@SAPAV1@H@Z
};
static_assert(sizeof(AARGameSettingsEdit) == 0x12C,
              "AARGameSettingsEdit size mismatch");

class AARGameSettingsView : public GameSettingsView {
public:
    static AARGameSettingsView* Me(int version);  // ?Me@AARGameSettingsView@@SAPAV1@H@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@AARGameSettingsView@@UAEXPAVPanelFile@@@Z
};
static_assert(sizeof(AARGameSettingsView) == 0x11C,
              "AARGameSettingsView size mismatch");

// ============================================================================
// InitialLoadingMenu / InstantActionMenu / PlayLanMenu / PlayOnlineMenu /
// PressStartMenu - FE menu stubs (mp_shell.o)
// ============================================================================
class InitialLoadingMenu : public FEMenu {
public:
    float mTime;                         // +0x4C

    static InitialLoadingMenu* Me();     // ?Me@InitialLoadingMenu@@SAPAV1@XZ
    virtual void Select(int entry_num);  // ?Select@InitialLoadingMenu@@UAEXH@Z
};
static_assert(sizeof(InitialLoadingMenu) == 0x50,
              "InitialLoadingMenu size mismatch");

class InstantActionMenu : public FEMenu {
public:
    static InstantActionMenu* Me();      // ?Me@InstantActionMenu@@SAPAV1@XZ
    virtual void OnActivate();           // ?OnActivate@InstantActionMenu@@UAEXXZ
    virtual void OnTriangle(int c);      // ?OnTriangle@InstantActionMenu@@UAEXH@Z
};
static_assert(sizeof(InstantActionMenu) == 0x4C,
              "InstantActionMenu size mismatch");

class PlayLanMenu : public FEMenu {
public:
    bool mJoiningFriend;                 // +0x4C
    ae_array<PanelQuad*, 3> m_pBackgroundArt;   // +0x50
    ae_array<PanelQuad*, 5> m_pBackgroundButtons;  // +0x5C
    ae_array<FEText*, 4>    m_pText;     // +0x70
    ae_array<FEText*, 3>    m_pOptionText;  // +0x80
    ae_array<PanelQuad*, 3> m_pImages;   // +0x8C
    UIListBox mListBox;                  // +0x98

    static PlayLanMenu* Me();            // ?Me@PlayLanMenu@@SAPAV1@XZ
    virtual void OnDeactivate(FEMenu* m);// ?OnDeactivate@PlayLanMenu@@UAEXPAVFEMenu@@@Z
    virtual void OnTriangle(int c);      // ?OnTriangle@PlayLanMenu@@UAEXH@Z
};
static_assert(sizeof(PlayLanMenu) == 0x144,
              "PlayLanMenu size mismatch");

class PlayOnlineMenu : public FEMenu {
public:
    static int m_currSelection;          // ?m_currSelection@PlayOnlineMenu@@1HA @ 0xE381C4
    bool mJoiningFriend;                 // +0x4C
    unsigned int friendIcon;             // +0x50
    PanelQuad* m_pBkgDetail4;            // +0x54
    PanelQuad* m_pBkgDetail5;            // +0x58
    bool m_IsQuickMatchReady;            // +0x5C

    static PlayOnlineMenu* Me();         // ?Me@PlayOnlineMenu@@SAPAV1@XZ
    virtual void OnDeactivate(FEMenu* m);// ?OnDeactivate@PlayOnlineMenu@@UAEXPAVFEMenu@@@Z
    virtual void OnTriangle(int c);      // ?OnTriangle@PlayOnlineMenu@@UAEXH@Z
};
static_assert(sizeof(PlayOnlineMenu) == 0x60,
              "PlayOnlineMenu size mismatch");

class PressStartMenu : public FEMenu {
public:
    static PressStartMenu* Me();         // ?Me@PressStartMenu@@SAPAV1@XZ
    virtual void Select(int entry_num);  // ?Select@PressStartMenu@@UAEXH@Z
    virtual void OnTriangle(int c);      // ?OnTriangle@PressStartMenu@@UAEXH@Z
    virtual void OnStart(int c);         // ?OnStart@PressStartMenu@@UAEXH@Z
};
static_assert(sizeof(PressStartMenu) == 0x4C,
              "PressStartMenu size mismatch");

// ============================================================================
// SessionDetailsMenu / SessionListMenu / SessionLanListMenu
// ============================================================================
class SessionDetailsMenu : public FEMenu {
public:
    unsigned int mNumGames;    // +0x4C
    unsigned int mCurrentGame; // +0x50

    static SessionDetailsMenu* Me();  // ?Me@SessionDetailsMenu@@SAPAV1@XZ
    virtual void OnDeactivate(FEMenu* m);  // ?OnDeactivate@SessionDetailsMenu@@UAEXPAVFEMenu@@@Z
    virtual void OnTriangle(int c);       // ?OnTriangle@SessionDetailsMenu@@UAEXH@Z
};
static_assert(sizeof(SessionDetailsMenu) == 0x54,
              "SessionDetailsMenu size mismatch");

class SessionListMenu : public FEMultiMenu {
public:
    unsigned int mSortColumn;          // +0x4C
    unsigned int mNumGames;            // +0x50
    bool mShowDownArrow;               // +0x54
    bool mShowUpArrow;                 // +0x55
    bool mNeedToUpdate;                // +0x56
    uint8_t _pad57[1];                 // +0x57
    UIListBox m_ListBox;               // +0x58
    ae_array<PanelQuad*, 5> m_pBackgroundArt;   // +0x104
    ae_array<FEText*, 3> m_pText;               // +0x118
    ae_array<FEText*, 4> m_pHeaderText;         // +0x124
    FEText* m_pServerText;                      // +0x134
    ae_array<PanelQuad*, 5> m_pConnectionStars; // +0x138
    int mVisibleListToGameListMap[25];          // +0x14C
    int m_currSelection;                        // +0x1B0

    static SessionListMenu* Me();  // ?Me@SessionListMenu@@SAPAV1@XZ
};
static_assert(sizeof(SessionListMenu) == 0x1B4,
              "SessionListMenu size mismatch");

class SessionLanListMenu : public FEMultiMenu {
public:
    UIListBox m_ListBox;               // +0x4C
    unsigned int mSortColumn;          // +0xF8
    unsigned int mNumGames;            // +0xFC
    bool mShowDownArrow;               // +0x100
    bool mShowUpArrow;                 // +0x101
    bool mNeedToUpdate;                // +0x102
    uint8_t _pad103[1];                // +0x103
    ae_array<PanelQuad*, 5> m_pBackgroundArt;   // +0x104
    ae_array<FEText*, 3> m_pText;               // +0x118
    ae_array<FEText*, 4> m_pHeaderText;         // +0x124
    FEText* m_pServerText;                      // +0x134
    ae_array<PanelQuad*, 5> m_pConnectionStars; // +0x138
    int mVisibleListToGameListMap[25];          // +0x14C
    int m_currSelection;                        // +0x1B0

    static SessionLanListMenu* Me();  // ?Me@SessionLanListMenu@@SAPAV1@XZ
    virtual void OnCross(int c);      // ?OnCross@SessionLanListMenu@@UAEXH@Z
};
static_assert(sizeof(SessionLanListMenu) == 0x1B4,
              "SessionLanListMenu size mismatch");

// ============================================================================
// OverlayMenuBase - FEMenu + 3 ints (0x58), verified against IDA
// ============================================================================
class OverlayMenuBase : public FEMenu {
public:
    int mVersion;      // +0x4C
    int mAcceptMenu;   // +0x50
    int mBackMenu;     // +0x54
    virtual void OnTriangle(int c);  // ?OnTriangle@OverlayMenuBase@@UAEXH@Z
};
static_assert(sizeof(OverlayMenuBase) == 0x58,
              "OverlayMenuBase size mismatch");

// ============================================================================
// InGameOverlay / AAROverlay - overlay menu family
// ============================================================================
class InGameOverlay : public OverlayMenuBase {
public:
    enum eState : int {
        NO_OVERLAY = 0x0,
        NETWORK_ERROR_COUNTDOWN = 0x1,
        CONTROLLER_DISCONNECTED = 0x2,
    };

    eState m_State;                  // +0x58
    int m_currSelection;             // +0x5C
    UIListBox m_ListBox;             // +0x60
    Broc::string m_Text;             // +0x10C
    bool m_IsAARTimerEnabled;        // +0x110
    uint8_t _pad111[3];              // +0x111
    ae_array<PanelQuad*, 3> m_pBackgroundArt;  // +0x114
    ae_array<FEText*, 2> m_pOptionText;        // +0x120
    ae_array<PanelQuad*, 1> m_pOptionLines;    // +0x128

    virtual void Select(int __formal);  // ?Select@InGameOverlay@@UAEXH@Z
    virtual void OnCircle(int __formal);  // ?OnCircle@InGameOverlay@@UAEXH@Z
    virtual void OnSquare(int __formal);  // ?OnSquare@InGameOverlay@@UAEXH@Z
};
static_assert(sizeof(InGameOverlay) == 0x12C,
              "InGameOverlay size mismatch");

class AAROverlay : public OverlayMenuBase {
public:
    enum eState : int {
        NO_OVERLAY = 0x0,
        NETWORK_ERROR_COUNTDOWN = 0x1,
        CONTROLLER_DISCONNECTED = 0x2,
    };

    eState m_State;                  // +0x58
    int m_currSelection;             // +0x5C
    UIListBox m_ListBox;             // +0x60
    Broc::string m_Text;             // +0x10C
    bool m_IsAARTimerEnabled;        // +0x110
    uint8_t _pad111[3];              // +0x111
    ae_array<PanelQuad*, 3> m_pBackgroundArt;  // +0x114
    ae_array<FEText*, 2> m_pOptionText;        // +0x120
    ae_array<PanelQuad*, 1> m_pOptionLines;    // +0x128

    virtual void Select(int __formal);  // ?Select@AAROverlay@@UAEXH@Z
    virtual void OnCircle(int __formal);  // ?OnCircle@AAROverlay@@UAEXH@Z
    virtual void OnSquare(int __formal);  // ?OnSquare@AAROverlay@@UAEXH@Z
};
static_assert(sizeof(AAROverlay) == 0x12C,
              "AAROverlay size mismatch");

// ============================================================================
// MultilineOverlayMenu family
// ============================================================================
class MultilineOverlayMenu : public OverlayMenuBase {
public:
    Broc::string mText;             // +0x58
    FEMultiLineText* mTextEntry;    // +0x5C
    float mTextScale;               // +0x60
    float mCountdown;               // +0x64
};
static_assert(sizeof(MultilineOverlayMenu) == 0x68,
              "MultilineOverlayMenu size mismatch");

class MultilineFrontendOverlayMenu : public MultilineOverlayMenu {
public:
    enum eState : int {
        NO_OVERLAY = 0x0,
        NETWORK_ERROR_COUNTDOWN = 0x1,
        CONTROLLER_DISCONNECTED = 0x2,
    };

    eState mState;                  // +0x68
    eState mCachedState;            // +0x6C
    int mCachedAcceptMenu;          // +0x70
    int mCachedBackMenu;            // +0x74

    static MultilineFrontendOverlayMenu* Me();  // ?Me@MultilineFrontendOverlayMenu@@SAPAV1@XZ
    virtual void OnCross(int c);    // ?OnCross@MultilineFrontendOverlayMenu@@UAEXH@Z
    virtual void OnTriangle(int c); // ?OnTriangle@MultilineFrontendOverlayMenu@@UAEXH@Z
};
static_assert(sizeof(MultilineFrontendOverlayMenu) == 0x78,
              "MultilineFrontendOverlayMenu size mismatch");

class MultilineIngameOverlayMenu : public MultilineOverlayMenu {
public:
    enum eState : int {
        NO_OVERLAY = 0x0,
        NETWORK_ERROR_COUNTDOWN = 0x1,
        CONTROLLER_DISCONNECTED = 0x2,
    };

    eState mState;                  // +0x68
};
static_assert(sizeof(MultilineIngameOverlayMenu) == 0x6C,
              "MultilineIngameOverlayMenu size mismatch");

// ============================================================================
// VoteMapMenu / WeaponSelectMenu
// ============================================================================
class VoteMapMenu : public FEMenu {
public:
    void* mPlayerMgr;      // +0x4C (MPPlayerManager*)
    void* mMapList;        // +0x50 (FEMenuListBox*)

    virtual void OnDeactivate(FEMenu* m);  // ?OnDeactivate@VoteMapMenu@@UAEXPAVFEMenu@@@Z
};
static_assert(sizeof(VoteMapMenu) == 0x54,
              "VoteMapMenu size mismatch");

// ============================================================================
// OverlayMenu - front-end overlay (IDA verified; size 0x18C)
// ============================================================================
class OverlayMenu : public OverlayMenuBase {
public:
    enum eState : int {
        SIGNING_IN = 0x1,
        GAME_LISTING = 0x2,
        GAME_LISTING_START = 0x3,
        NO_GAMES = 0x4,
        JOINING_START = 0x5,
        JOINING = 0x6,
        JOIN_REFUSED = 0x7,
        JOIN_FAILED = 0x8,
        CANNOT_CONNECT_TO_HOST = 0x9,
        CANNOT_CONNECT_TO_PEERS = 0xA,
        JOIN_SUCCESS = 0xB,
        BDNET_STARTING = 0xC,
        BDNET_START_FAILED = 0xD,
        FROM_ID_QUERYING = 0xE,
    };

    eState mState;                      // +0x58
    float  mDotTimer;                   // +0x5C
    int    mNumDots;                    // +0x60
    Broc::string mText;                 // +0x64
    float  mTimeout;                    // +0x68
    int    mGameListingNum;             // +0x6C
    bool   mbStartGame;                 // +0x70
    int    mDelayStart;                 // +0x74
    int    mLinkStatusCount;            // +0x78
    float  mLinkStatusTimer;            // +0x7C
    int    m_currSelection;             // +0x80
    UIListBox m_ListBox;                // +0x84 (172 bytes)
    ae_array<PanelQuad*, 3> m_pBackgroundArt;  // +0x130
    ae_array<FEText*, 2> m_pOptionText;        // +0x13C
    ae_array<PanelQuad*, 1> m_pOptionLines;    // +0x144

    static OverlayMenu* Me(int version);  // ?Me@OverlayMenu@@SAPAV1@H@Z
    const eState GetState();              // ?GetState@OverlayMenu@@QAE?BW4eState@1@XZ
    void SetState(eState state);          // ?SetState@OverlayMenu@@QAEXW4eState@1@@Z
    void LogonUpdate();                   // ?LogonUpdate@OverlayMenu@@IAEXXZ
    virtual void OnCircle(int c);         // ?OnCircle@OverlayMenu@@UAEXH@Z
};
static_assert(sizeof(OverlayMenu) == 0x148,
              "OverlayMenu size mismatch");
