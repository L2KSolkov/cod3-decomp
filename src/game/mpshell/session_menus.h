// ============================================================================
// session_menus.h - mp_shell.o session/overlay menu classes
// Reconstructed from IDA local types (PDB symbol data). Sizes verified.
// ============================================================================

#pragma once

#include "game/shell/shell_types.h"

struct sServerCreateParams;  // full definition in session_menus.cpp
enum eGameType : int;        // full definition in session_menus.cpp
enum EPlayerClass : int {
    kPlayerClassRifleman = 0,
    kPlayerClassInfantry = 1,
    kPlayerClassAssault = 2,
    kPlayerClassMedic = 3,
    kPlayerClassScout = 4,
    kPlayerClassSupport = 5,
    kPlayerClassAntiArmor = 6,
};

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
    virtual void OnCross(int c);         // ?OnCross@CreateSessionMenu@@UAEXH@Z
    virtual void OnActivate();           // ?OnActivate@CreateSessionMenu@@UAEXXZ
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
    virtual void OnCross(int c);            // ?OnCross@CreateLanSessionMenu@@UAEXH@Z
    virtual void OnSquare(int c);           // ?OnSquare@CreateLanSessionMenu@@UAEXH@Z
    virtual void OnActivate();              // ?OnActivate@CreateLanSessionMenu@@UAEXXZ
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
    virtual void OnCross(int c);       // ?OnCross@FindSessionMenu@@UAEXH@Z
    virtual void OnActivate();         // ?OnActivate@FindSessionMenu@@UAEXXZ
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
    virtual void OnCross(int c);          // ?OnCross@FindLanSessionMenu@@UAEXH@Z
    virtual void OnActivate();            // ?OnActivate@FindLanSessionMenu@@UAEXXZ
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
protected:
    void GrabSessionName();                      // ?GrabSessionName@CreateSessionAdvancedMenu@@IAEXXZ
public:
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
protected:
    void GrabSessionName();                         // ?GrabSessionName@CreateLanSessionAdvancedMenu@@IAEXXZ
public:
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

    GameSettingsView(FEMenuSystem* s);  // ??0GameSettingsView@@QAE@PAVFEMenuSystem@@@Z (mp_shell.o 0x79CCF0)
    static GameSettingsView* Me(int version);  // ?Me@GameSettingsView@@SAPAV1@H@Z
    virtual void Update(float time_inc);  // ?Update@GameSettingsView@@UAEXM@Z
    virtual void Init();                 // ?Init@GameSettingsView@@UAEXXZ
    virtual void OnActivate();           // ?OnActivate@GameSettingsView@@UAEXXZ
    virtual void Draw();                 // ?Draw@GameSettingsView@@UAEXXZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@GameSettingsView@@UAEXPAVPanelFile@@@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@GameSettingsView@@UAEXPAVPanelFile@@@Z
    void OnDeactivate(FESplitScreenMenu* m);  // ?OnDeactivate@GameSettingsView@@QAEXPAVFESplitScreenMenu@@@Z
    virtual void OnUp(int c);            // ?OnUp@GameSettingsView@@UAEXH@Z
    virtual void OnDown(int c);          // ?OnDown@GameSettingsView@@UAEXH@Z
    virtual void OnLeft(int c);          // ?OnLeft@GameSettingsView@@UAEXH@Z
    virtual void OnRight(int c);         // ?OnRight@GameSettingsView@@UAEXH@Z
    virtual void OnStart(int c);         // ?OnStart@GameSettingsView@@UAEXH@Z
    virtual void OnL1(int c);            // ?OnL1@GameSettingsView@@UAEXH@Z
    virtual void OnR1(int c);            // ?OnR1@GameSettingsView@@UAEXH@Z
    virtual void OnCross(int c);         // ?OnCross@GameSettingsView@@UAEXH@Z
    virtual void OnTriangle(int c);      // ?OnTriangle@GameSettingsView@@UAEXH@Z
    virtual void OnCircle(int c);        // ?OnCircle@GameSettingsView@@UAEXH@Z
    virtual void OnSquare(int c);        // ?OnSquare@GameSettingsView@@UAEXH@Z
protected:
    virtual void ButtonHeldAction();     // ?ButtonHeldAction@GameSettingsView@@MAEXXZ
    virtual void SwapMenus();            // ?SwapMenus@GameSettingsView@@MAEXXZ
    void SetPanelFileMain(PanelFile* pf);  // ?SetPanelFileMain@GameSettingsView@@IAEXPAVPanelFile@@@Z
    void SetPanelFileSplitScreen(PanelFile* pf);  // ?SetPanelFileSplitScreen@GameSettingsView@@IAEXPAVPanelFile@@@Z
    void UpdateScrollBar();              // ?UpdateScrollBar@GameSettingsView@@IAEXXZ
    void UpdateOptions();                // ?UpdateOptions@GameSettingsView@@IAEXXZ
    void UpdateSplitScreenOptions(int last_highlighted);  // ?UpdateSplitScreenOptions@GameSettingsView@@IAEXH@Z
    void UpdateOption(int option, FEText* text);  // ?UpdateOption@GameSettingsView@@IAEXHPAVFEText@@@Z
};
static_assert(sizeof(GameSettingsView) == 0x11C,
              "GameSettingsView size mismatch");

class GameSettingsEdit : public FESplitScreenMenu {
public:
    GameSettingsEdit(FEMenuSystem* s);  // ??0GameSettingsEdit@@QAE@PAVFEMenuSystem@@@Z
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

    static GameSettingsEdit* Me(int version);  // ?Me@GameSettingsEdit@@SAPAV1@H@Z
    virtual void Init();                 // ?Init@GameSettingsEdit@@UAEXXZ
    virtual void Draw();                 // ?Draw@GameSettingsEdit@@UAEXXZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@GameSettingsEdit@@UAEXPAVPanelFile@@@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@GameSettingsEdit@@UAEXPAVPanelFile@@@Z
    virtual void OnLeft(int c);          // ?OnLeft@GameSettingsEdit@@UAEXH@Z
    virtual void OnL1(int c);            // ?OnL1@GameSettingsEdit@@UAEXH@Z
    virtual void OnR1(int c);            // ?OnR1@GameSettingsEdit@@UAEXH@Z
    virtual void OnCross(int c);         // ?OnCross@GameSettingsEdit@@UAEXH@Z
    virtual void OnCircle(int c);        // ?OnCircle@GameSettingsEdit@@UAEXH@Z
    virtual void OnSquare(int c);        // ?OnSquare@GameSettingsEdit@@UAEXH@Z
    virtual void OnStart(int c);         // ?OnStart@GameSettingsEdit@@UAEXH@Z
    virtual void OnActivate();           // ?OnActivate@GameSettingsEdit@@UAEXXZ
    virtual void OnTriangle(int c);      // ?OnTriangle@GameSettingsEdit@@UAEXH@Z
    virtual void Update(float time_inc); // ?Update@GameSettingsEdit@@UAEXM@Z
    void OnDeactivate(FESplitScreenMenu* m);  // ?OnDeactivate@GameSettingsEdit@@QAEXPAVFESplitScreenMenu@@@Z
    virtual void OnUp(int c);            // ?OnUp@GameSettingsEdit@@UAEXH@Z
    virtual void OnDown(int c);          // ?OnDown@GameSettingsEdit@@UAEXH@Z
    virtual void OnRight(int c);         // ?OnRight@GameSettingsEdit@@UAEXH@Z
protected:
    static bool ResponseYesApplyNow(int client);  // ?ResponseYesApplyNow@GameSettingsEdit@@KA_NH@Z
    static bool ResponseNoJustGoBackToPauseMenu(int client);  // ?ResponseNoJustGoBackToPauseMenu@GameSettingsEdit@@KA_NH@Z
    void SetGameTypeDefaults();                   // ?SetGameTypeDefaults@GameSettingsEdit@@IAEXXZ
    bool ResponseNoJustGoBackToPauseMenuHelper();  // ?ResponseNoJustGoBackToPauseMenuHelper@GameSettingsEdit@@IAE_NXZ
    bool ResponseYesApplyNowHelper();             // ?ResponseYesApplyNowHelper@GameSettingsEdit@@IAE_NXZ
    virtual void ButtonHeldAction();     // ?ButtonHeldAction@GameSettingsEdit@@MAEXXZ
    virtual void SwapMenus();            // ?SwapMenus@GameSettingsEdit@@MAEXXZ
    void UpdateMapChangeStatus(bool isAllowingMapVote);  // ?UpdateMapChangeStatus@GameSettingsEdit@@IAEX_N@Z
    void DisableTeamGameOptions(bool b);  // ?DisableTeamGameOptions@GameSettingsEdit@@IAEX_N@Z
    void GetScoreLimitsForGameType(eGameType gameType);  // ?GetScoreLimitsForGameType@GameSettingsEdit@@IAEXW4eGameType@@@Z
    void UpdateHighlight();            // ?UpdateHighlight@GameSettingsEdit@@IAEXXZ
    void SetPanelFileMain(PanelFile* pf);  // ?SetPanelFileMain@GameSettingsEdit@@IAEXPAVPanelFile@@@Z
    void SetPanelFileSplitScreen(PanelFile* pf);  // ?SetPanelFileSplitScreen@GameSettingsEdit@@IAEXPAVPanelFile@@@Z
    void UpdateScrollBar();              // ?UpdateScrollBar@GameSettingsEdit@@IAEXXZ
    void UpdateSplitScreenOptions(int last_highlighted);  // ?UpdateSplitScreenOptions@GameSettingsEdit@@IAEXH@Z
    void AddOptionsToCombos();           // ?AddOptionsToCombos@GameSettingsEdit@@IAEXXZ
};
static_assert(sizeof(GameSettingsEdit) == 0x12C,
              "GameSettingsEdit size mismatch");

// AAR variants (same layouts, own vtables)
class AARGameSettingsEdit : public GameSettingsEdit {
public:
    static AARGameSettingsEdit* Me(int version);  // ?Me@AARGameSettingsEdit@@SAPAV1@H@Z
    static bool ResponseYesApplyNow(int index);   // ?ResponseYesApplyNow@AARGameSettingsEdit@@SA_NH@Z
    static bool ResponseNoJustGoBackToPauseMenu(int client);  // ?ResponseNoJustGoBackToPauseMenu@AARGameSettingsEdit@@SA_NH@Z
    AARGameSettingsEdit(FEMenuSystem* s);  // ??0AARGameSettingsEdit@@QAE@PAVFEMenuSystem@@@Z
    virtual ~AARGameSettingsEdit();   // ??1AARGameSettingsEdit@@UAE@XZ
    virtual void Update(float time_inc);  // ?Update@AARGameSettingsEdit@@UAEXM@Z
    virtual void SetPanelFile(PanelFile* pf);     // ?SetPanelFile@AARGameSettingsEdit@@UAEXPAVPanelFile@@@Z
    virtual void OnActivate();        // ?OnActivate@AARGameSettingsEdit@@UAEXXZ
    virtual void OnTriangle(int c);   // ?OnTriangle@AARGameSettingsEdit@@UAEXH@Z
private:
    void SetTimerText();              // ?SetTimerText@AARGameSettingsEdit@@AAEXXZ
};
static_assert(sizeof(AARGameSettingsEdit) == 0x12C,
              "AARGameSettingsEdit size mismatch");

class AARGameSettingsView : public GameSettingsView {
public:
    AARGameSettingsView(FEMenuSystem* s);  // ??0AARGameSettingsView@@QAE@PAVFEMenuSystem@@@Z
    static AARGameSettingsView* Me(int version);  // ?Me@AARGameSettingsView@@SAPAV1@H@Z
    virtual ~AARGameSettingsView();   // ??1AARGameSettingsView@@UAE@XZ
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@AARGameSettingsView@@UAEXPAVPanelFile@@@Z
    virtual void Update(float time_inc);  // ?Update@AARGameSettingsView@@UAEXM@Z
    virtual void OnActivate();        // ?OnActivate@AARGameSettingsView@@UAEXXZ
private:
    void SetTimerText();              // ?SetTimerText@AARGameSettingsView@@AAEXXZ
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

    InitialLoadingMenu(FEMenuSystem* s); // ??0InitialLoadingMenu@@QAE@PAVFEMenuSystem@@@Z
    static InitialLoadingMenu* Me();     // ?Me@InitialLoadingMenu@@SAPAV1@XZ
    virtual void OnActivate();           // ?OnActivate@InitialLoadingMenu@@UAEXXZ
    virtual void Draw();                 // ?Draw@InitialLoadingMenu@@UAEXXZ
    virtual void Update(float time_inc); // ?Update@InitialLoadingMenu@@UAEXM@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@InitialLoadingMenu@@UAEXPAVPanelFile@@@Z
    virtual void Select(int entry_num);  // ?Select@InitialLoadingMenu@@UAEXH@Z
};
static_assert(sizeof(InitialLoadingMenu) == 0x50,
              "InitialLoadingMenu size mismatch");

class InstantActionMenu : public FEMenu {
public:
    InstantActionMenu(FEMenuSystem* s);  // ??0InstantActionMenu@@QAE@PAVFEMenuSystem@@@Z
    static InstantActionMenu* Me();      // ?Me@InstantActionMenu@@SAPAV1@XZ
    virtual void OnActivate();           // ?OnActivate@InstantActionMenu@@UAEXXZ
    virtual void Draw();                 // ?Draw@InstantActionMenu@@UAEXXZ
    virtual void Update(float time_inc); // ?Update@InstantActionMenu@@UAEXM@Z
    virtual void OnTriangle(int c);      // ?OnTriangle@InstantActionMenu@@UAEXH@Z
    virtual void Select(int entry_num);  // ?Select@InstantActionMenu@@UAEXH@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@InstantActionMenu@@UAEXPAVPanelFile@@@Z
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

    PlayLanMenu(FEMenuSystem* s);      // ??0PlayLanMenu@@QAE@PAVFEMenuSystem@@@Z
    static PlayLanMenu* Me();            // ?Me@PlayLanMenu@@SAPAV1@XZ
    virtual void OnDeactivate(FEMenu* m);// ?OnDeactivate@PlayLanMenu@@UAEXPAVFEMenu@@@Z
    virtual void Draw();                 // ?Draw@PlayLanMenu@@UAEXXZ
    virtual void OnTriangle(int c);      // ?OnTriangle@PlayLanMenu@@UAEXH@Z
    virtual void OnCross(int c);         // ?OnCross@PlayLanMenu@@UAEXH@Z
    virtual void OnUp(int c);            // ?OnUp@PlayLanMenu@@UAEXH@Z
    virtual void OnDown(int c);          // ?OnDown@PlayLanMenu@@UAEXH@Z
    virtual void OnActivate();           // ?OnActivate@PlayLanMenu@@UAEXXZ
    virtual void Update(float time_inc); // ?Update@PlayLanMenu@@UAEXM@Z
protected:
    void SetPreviewImage();              // ?SetPreviewImage@PlayLanMenu@@IAEXXZ
    void SetOptionText();                // ?SetOptionText@PlayLanMenu@@IAEXXZ
    void SetDescriptionText();           // ?SetDescriptionText@PlayLanMenu@@IAEXXZ
    void SetLiveOnXBox();                // ?SetLiveOnXBox@PlayLanMenu@@IAEXXZ
    void ClearPreviewImages();           // ?ClearPreviewImages@PlayLanMenu@@IAEXXZ
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

    PlayOnlineMenu(FEMenuSystem* s);   // ??0PlayOnlineMenu@@QAE@PAVFEMenuSystem@@@Z
    static PlayOnlineMenu* Me();         // ?Me@PlayOnlineMenu@@SAPAV1@XZ
    void TogglePreviewImage(int option, bool visible);  // ?TogglePreviewImage@PlayOnlineMenu@@QAEXH_N@Z
    virtual void OnDeactivate(FEMenu* m);// ?OnDeactivate@PlayOnlineMenu@@UAEXPAVFEMenu@@@Z
    virtual void OnTriangle(int c);      // ?OnTriangle@PlayOnlineMenu@@UAEXH@Z
    virtual void Draw();                 // ?Draw@PlayOnlineMenu@@UAEXXZ
    virtual void OnUp(int c);            // ?OnUp@PlayOnlineMenu@@UAEXH@Z
    virtual void OnDown(int c);          // ?OnDown@PlayOnlineMenu@@UAEXH@Z
    virtual void OnActivate();           // ?OnActivate@PlayOnlineMenu@@UAEXXZ
    virtual void Update(float time_inc); // ?Update@PlayOnlineMenu@@UAEXM@Z
    virtual void Select(int entry_num, int c);  // ?Select@PlayOnlineMenu@@UAEXHH@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@PlayOnlineMenu@@UAEXPAVPanelFile@@@Z
protected:
    void InitQuickMatchParameters(int c);  // ?InitQuickMatchParameters@PlayOnlineMenu@@IAEXH@Z
    void UpdateTextDescription(int option);  // ?UpdateTextDescription@PlayOnlineMenu@@IAEXH@Z
    void LaunchQuickMatch();             // ?LaunchQuickMatch@PlayOnlineMenu@@IAEXXZ
};
static_assert(sizeof(PlayOnlineMenu) == 0x60,
              "PlayOnlineMenu size mismatch");

class PressStartMenu : public FEMenu {
public:
    PressStartMenu(FEMenuSystem* s);     // ??0PressStartMenu@@QAE@PAVFEMenuSystem@@@Z
    static PressStartMenu* Me();         // ?Me@PressStartMenu@@SAPAV1@XZ
    virtual void OnActivate();           // ?OnActivate@PressStartMenu@@UAEXXZ
    virtual void Draw();                 // ?Draw@PressStartMenu@@UAEXXZ
    virtual void Update(float time_inc); // ?Update@PressStartMenu@@UAEXM@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@PressStartMenu@@UAEXPAVPanelFile@@@Z
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

    SessionDetailsMenu(FEMenuSystem* s);  // ??0SessionDetailsMenu@@QAE@PAVFEMenuSystem@@@Z
    static SessionDetailsMenu* Me();  // ?Me@SessionDetailsMenu@@SAPAV1@XZ
    virtual void Draw();              // ?Draw@SessionDetailsMenu@@UAEXXZ
    virtual void OnActivate();        // ?OnActivate@SessionDetailsMenu@@UAEXXZ
    virtual void Update(float time_inc);  // ?Update@SessionDetailsMenu@@UAEXM@Z
    virtual void Select(int entry_num);  // ?Select@SessionDetailsMenu@@UAEXH@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@SessionDetailsMenu@@UAEXPAVPanelFile@@@Z
protected:
    void UpdateDetails();             // ?UpdateDetails@SessionDetailsMenu@@IAEXXZ
public:
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

    SessionListMenu(FEMenuSystem* s);  // ??0SessionListMenu@@QAE@PAVFEMenuSystem@@@Z
    static SessionListMenu* Me();  // ?Me@SessionListMenu@@SAPAV1@XZ
    virtual void Draw();           // ?Draw@SessionListMenu@@UAEXXZ
    virtual void OnTriangle(int c);// ?OnTriangle@SessionListMenu@@UAEXH@Z
    virtual void OnCircle(int c);  // ?OnCircle@SessionListMenu@@UAEXH@Z
    virtual void OnCross(int c);   // ?OnCross@SessionListMenu@@UAEXH@Z
    virtual void OnSquare(int c);  // ?OnSquare@SessionListMenu@@UAEXH@Z
    virtual void OnUp(int c);      // ?OnUp@SessionListMenu@@UAEXH@Z
    virtual void OnDown(int c);    // ?OnDown@SessionListMenu@@UAEXH@Z
    virtual void Select(int entry_num);  // ?Select@SessionListMenu@@UAEXH@Z
    virtual void OnActivate();     // ?OnActivate@SessionListMenu@@UAEXXZ
    virtual void Update(float time_inc);  // ?Update@SessionListMenu@@UAEXM@Z
    virtual ~SessionListMenu();    // ??1SessionListMenu@@UAE@XZ
protected:
    void Refresh();                // ?Refresh@SessionListMenu@@IAEXXZ
    void TidyGamesList();          // ?TidyGamesList@SessionListMenu@@IAEXXZ
    void InitMenu();               // ?InitMenu@SessionListMenu@@IAEXXZ
    void UpdateGameInfo();         // ?UpdateGameInfo@SessionListMenu@@IAEXXZ
    void RepopulateSessionList();  // ?RepopulateSessionList@SessionListMenu@@IAEXXZ
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

    SessionLanListMenu(FEMenuSystem* s);  // ??0SessionLanListMenu@@QAE@PAVFEMenuSystem@@@Z
    static SessionLanListMenu* Me();  // ?Me@SessionLanListMenu@@SAPAV1@XZ
    virtual void OnCross(int c);      // ?OnCross@SessionLanListMenu@@UAEXH@Z
    virtual void Draw();              // ?Draw@SessionLanListMenu@@UAEXXZ
    virtual void OnTriangle(int c);   // ?OnTriangle@SessionLanListMenu@@UAEXH@Z
    virtual void OnCircle(int c);     // ?OnCircle@SessionLanListMenu@@UAEXH@Z
    virtual void Select(int entry_num);  // ?Select@SessionLanListMenu@@UAEXH@Z
    virtual void OnSquare(int c);     // ?OnSquare@SessionLanListMenu@@UAEXH@Z
    virtual void OnUp(int c);         // ?OnUp@SessionLanListMenu@@UAEXH@Z
    virtual void OnDown(int c);       // ?OnDown@SessionLanListMenu@@UAEXH@Z
    virtual void OnActivate();        // ?OnActivate@SessionLanListMenu@@UAEXXZ
    virtual void Update(float time_inc);  // ?Update@SessionLanListMenu@@UAEXM@Z
    virtual ~SessionLanListMenu();    // ??1SessionLanListMenu@@UAE@XZ
protected:
    void Refresh();                   // ?Refresh@SessionLanListMenu@@IAEXXZ
    void InitMenu();                  // ?InitMenu@SessionLanListMenu@@IAEXXZ
    void TidyGamesList();             // ?TidyGamesList@SessionLanListMenu@@IAEXXZ
    void UpdateGameInfo();            // ?UpdateGameInfo@SessionLanListMenu@@IAEXXZ
    void RepopulateSessionList();     // ?RepopulateSessionList@SessionLanListMenu@@IAEXXZ
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
    OverlayMenuBase(FEMenuSystem* s, int numEntries);  // ??0OverlayMenuBase@@QAE@PAVFEMenuSystem@@H@Z
    virtual void OnActivate();          // ?OnActivate@OverlayMenuBase@@UAEXXZ
    virtual void Draw();                // ?Draw@OverlayMenuBase@@UAEXXZ
    virtual void Update(float time_inc);// ?Update@OverlayMenuBase@@UAEXM@Z
    virtual void UpdateSplitScreen();   // ?UpdateSplitScreen@OverlayMenuBase@@UAEXXZ
    virtual void OnTriangle(int c);  // ?OnTriangle@OverlayMenuBase@@UAEXH@Z
    virtual void Accept();           // ?Accept@OverlayMenuBase@@UAEXXZ
};
static_assert(sizeof(OverlayMenuBase) == 0x58,
              "OverlayMenuBase size mismatch");

// ============================================================================
// InGameOverlay / AAROverlay - overlay menu family
// ============================================================================
class InGameOverlay : public OverlayMenuBase {
public:
    enum eState : int {
        NONE = 0x0,
        OVERLAY_SIGNIN_SIGNOUT = 0x1,
        OVERLAY_APPEAR_ONLINE = 0x2,
        OVERLAY_APPEAR_OFFLINE = 0x3,
        OVERLAY_TOGGLE_VOICE = 0x4,
        OVERLAY_JOIN_FRIEND = 0x5,
        OVERLAY_REBOOT_REQUIRED = 0x6,
        OVERLAY_AAR_SIGNIN_SIGNOUT = 0x7,
        OVERLAY_AAR_APPEAR_ONLINE = 0x8,
        OVERLAY_AAR_APPEAR_OFFLINE = 0x9,
        OVERLAY_AAR_TOGGLE_VOICE = 0xA,
        OVERLAY_AAR_JOIN_FRIEND = 0xB,
        OVERLAY_AAR_REBOOT_REQUIRED = 0xC,
        NUM_STATES = 0xD,
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

    InGameOverlay(FEMenuSystem* pMenuSys);  // ??0InGameOverlay@@QAE@PAVFEMenuSystem@@@Z
    static InGameOverlay* Me(int version);  // ?Me@InGameOverlay@@SAPAV1@H@Z
    void SetState(eState state);        // ?SetState@InGameOverlay@@QAEXW4eState@1@@Z
    virtual void Select(int __formal);  // ?Select@InGameOverlay@@UAEXH@Z
    virtual void OnUp(int c);           // ?OnUp@InGameOverlay@@UAEXH@Z
    virtual void OnDown(int c);         // ?OnDown@InGameOverlay@@UAEXH@Z
    virtual void OnTriangle(int c);     // ?OnTriangle@InGameOverlay@@UAEXH@Z
    virtual void OnCross(int c);        // ?OnCross@InGameOverlay@@UAEXH@Z
    virtual void Draw();                // ?Draw@InGameOverlay@@UAEXXZ
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@InGameOverlay@@UAEXPAVPanelFile@@@Z
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@InGameOverlay@@UAEXPAVPanelFile@@@Z
    virtual void Update(float time_inc); // ?Update@InGameOverlay@@UAEXM@Z
    virtual void OnCircle(int __formal);  // ?OnCircle@InGameOverlay@@UAEXH@Z
    virtual void OnSquare(int __formal);  // ?OnSquare@InGameOverlay@@UAEXH@Z
    virtual ~InGameOverlay();             // ??1InGameOverlay@@UAE@XZ
};
static_assert(sizeof(InGameOverlay) == 0x12C,
              "InGameOverlay size mismatch");

class AAROverlay : public OverlayMenuBase {
public:
    enum eState : int {
        NONE = 0x0,
        OVERLAY_SIGNIN_SIGNOUT = 0x1,
        OVERLAY_APPEAR_ONLINE = 0x2,
        OVERLAY_APPEAR_OFFLINE = 0x3,
        OVERLAY_TOGGLE_VOICE = 0x4,
        OVERLAY_JOIN_FRIEND = 0x5,
        OVERLAY_REBOOT_REQUIRED = 0x6,
        OVERLAY_AAR_SIGNIN_SIGNOUT = 0x7,
        OVERLAY_AAR_APPEAR_ONLINE = 0x8,
        OVERLAY_AAR_APPEAR_OFFLINE = 0x9,
        OVERLAY_AAR_TOGGLE_VOICE = 0xA,
        OVERLAY_AAR_JOIN_FRIEND = 0xB,
        OVERLAY_AAR_REBOOT_REQUIRED = 0xC,
        NUM_STATES = 0xD,
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

    AAROverlay(FEMenuSystem* pMenuSys);  // ??0AAROverlay@@QAE@PAVFEMenuSystem@@@Z
    static AAROverlay* Me(int version);    // ?Me@AAROverlay@@SAPAV1@H@Z
    void SetState(eState state);        // ?SetState@AAROverlay@@QAEXW4eState@1@@Z
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@AAROverlay@@UAEXPAVPanelFile@@@Z
    virtual void Select(int __formal);  // ?Select@AAROverlay@@UAEXH@Z
    virtual void OnUp(int c);           // ?OnUp@AAROverlay@@UAEXH@Z
    virtual void OnDown(int c);         // ?OnDown@AAROverlay@@UAEXH@Z
    virtual void OnTriangle(int c);     // ?OnTriangle@AAROverlay@@UAEXH@Z
    virtual void OnCross(int c);        // ?OnCross@AAROverlay@@UAEXH@Z
    virtual void Draw();                // ?Draw@AAROverlay@@UAEXXZ
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@AAROverlay@@UAEXPAVPanelFile@@@Z
    virtual void Update(float time_inc); // ?Update@AAROverlay@@UAEXM@Z
    virtual void OnCircle(int __formal);  // ?OnCircle@AAROverlay@@UAEXH@Z
    virtual void OnSquare(int __formal);  // ?OnSquare@AAROverlay@@UAEXH@Z
    virtual ~AAROverlay();                // ??1AAROverlay@@UAE@XZ
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

    MultilineOverlayMenu(FEMenuSystem* s);  // ??0MultilineOverlayMenu@@QAE@PAVFEMenuSystem@@@Z
    virtual void OnActivate();      // ?OnActivate@MultilineOverlayMenu@@UAEXXZ
    virtual void Select(int entry_num);  // ?Select@MultilineOverlayMenu@@UAEXH@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@MultilineOverlayMenu@@UAEXPAVPanelFile@@@Z
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
    virtual void Update(float time_inc);        // ?Update@MultilineFrontendOverlayMenu@@UAEXM@Z
    virtual void OnCross(int c);    // ?OnCross@MultilineFrontendOverlayMenu@@UAEXH@Z
    virtual void OnStart(int c);    // ?OnStart@MultilineFrontendOverlayMenu@@UAEXH@Z
    virtual void OnTriangle(int c); // ?OnTriangle@MultilineFrontendOverlayMenu@@UAEXH@Z
    void SetTempState(eState newState);  // ?SetTempState@MultilineFrontendOverlayMenu@@QAEXW4eState@1@@Z
    void SetState(eState state);         // ?SetState@MultilineFrontendOverlayMenu@@QAEXW4eState@1@@Z
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

    static MultilineIngameOverlayMenu* Me();  // ?Me@MultilineIngameOverlayMenu@@SAPAV1@XZ
    void SetState(eState state);              // ?SetState@MultilineIngameOverlayMenu@@QAEXW4eState@1@@Z
    virtual void Update(float time_inc);      // ?Update@MultilineIngameOverlayMenu@@UAEXM@Z
    virtual void OnStart(int c);              // ?OnStart@MultilineIngameOverlayMenu@@UAEXH@Z
    virtual void OnDeactivate(FEMenu* menu);  // ?OnDeactivate@MultilineIngameOverlayMenu@@UAEXPAVFEMenu@@@Z
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

    VoteMapMenu(FEMenuSystem* pauseMenuSystem);  // ??0VoteMapMenu@@QAE@PAVFEMenuSystem@@@Z
    virtual void OnDeactivate(FEMenu* m);  // ?OnDeactivate@VoteMapMenu@@UAEXPAVFEMenu@@@Z
    virtual void Draw();                   // ?Draw@VoteMapMenu@@UAEXXZ
    virtual void OnStart(int c);           // ?OnStart@VoteMapMenu@@UAEXH@Z
    virtual void OnActivate();             // ?OnActivate@VoteMapMenu@@UAEXXZ
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@VoteMapMenu@@UAEXPAVPanelFile@@@Z
    virtual void Select(int entryNum);     // ?Select@VoteMapMenu@@UAEXH@Z
};
static_assert(sizeof(VoteMapMenu) == 0x54,
              "VoteMapMenu size mismatch");

class VoteGameTypeMenu : public FEMenu {
public:
    void* mPlayerMgr;       // +0x4C (MPPlayerManager*)
    void* mGameTypeList;    // +0x50 (FEMenuListBox*)

    VoteGameTypeMenu(FEMenuSystem* pauseMenuSystem);  // ??0VoteGameTypeMenu@@QAE@PAVFEMenuSystem@@@Z
    virtual void OnActivate();             // ?OnActivate@VoteGameTypeMenu@@UAEXXZ
    virtual void Draw();                   // ?Draw@VoteGameTypeMenu@@UAEXXZ
    virtual void OnStart(int c);           // ?OnStart@VoteGameTypeMenu@@UAEXH@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@VoteGameTypeMenu@@UAEXPAVPanelFile@@@Z
    virtual void Select(int entryNum);     // ?Select@VoteGameTypeMenu@@UAEXH@Z
};
static_assert(sizeof(VoteGameTypeMenu) == 0x54,
              "VoteGameTypeMenu size mismatch");

// ============================================================================
// ModelMenu / WeaponSelectMenu / InGameSwitchSides
// ============================================================================
class __declspec(align(16)) ModelMenu : public FESplitScreenMenu {
public:
    DbLinkedHandle<EntityHandleDb, Entity> mClassModelEntity;  // +0x68
    int  mCurrentTeam;         // +0x6C
    int  mCurrentWeapon;       // +0x70
    int  mCurrentClass;        // +0x74
    int  mCurrentAnim;         // +0x78
    bool mCurrentAnimAds;      // +0x7C
    int  mCurrentWeaponSheet;  // +0x80
    int  mNextAnimChangeTime;  // +0x84
    bool mFirstFrame;          // +0x88
    uint8_t _pad89[0x90 - 0x89];
    float mModelPosition[4];   // +0x90 (math::Position3)
    float mModelAngles[4];     // +0xA0 (math::Dir3)
    float mDirections[8];      // +0xB0 (math::Dir3[2])
    float mColors[8];          // +0xD0 (math::Vector4[2])
    float mBrightness[2];      // +0xF0
    float mInnerRadius;        // +0xF8
    float mOuterRadius;        // +0xFC
    float mDistance;           // +0x100
    float mAnimSpeed;          // +0x104
    uint8_t _pad108[0x110 - 0x108];  // +0x108 (align gap)

    ModelMenu(FEMenuSystem* s, int num_entries);  // ??0ModelMenu@@QAE@PAVFEMenuSystem@@H@Z
    virtual ~ModelMenu();    // ??1ModelMenu@@UAE@XZ
    virtual void Update(float time_inc);  // ?Update@ModelMenu@@UAEXM@Z
    virtual void Draw3D();   // ?Draw3D@ModelMenu@@UAEXXZ
    virtual void OnActivate();  // ?OnActivate@ModelMenu@@UAEXXZ
protected:
    void DebugControls();    // ?DebugControls@ModelMenu@@IAEXXZ
    void DebugRender();      // ?DebugRender@ModelMenu@@IAEXXZ
    void AddDObjToScene();   // ?AddDObjToScene@ModelMenu@@IAEXXZ
    void SetLightBrightness(int index, float brightness);  // ?SetLightBrightness@ModelMenu@@IAEXHM@Z
    void SetLightColor(int index, const math::Vector4& color);  // ?SetLightColor@ModelMenu@@IAEXHABVVector4@math@@@Z
    void SetLightDirection(int index, const math::Dir3& dir);  // ?SetLightDirection@ModelMenu@@IAEXHABVDir3@math@@@Z
    void PlayModifierAnim(int sheet, int row, int column, bool immediate);  // ?PlayModifierAnim@ModelMenu@@IAEXHHH_N@Z
    const char* GetClassModel(int playerClass, int team);  // ?GetClassModel@ModelMenu@@IAEPBDHH@Z
    void UpdateModelPosition();         // ?UpdateModelPosition@ModelMenu@@IAEXXZ
    void UpdateClassModel(int playerclass, int team, int weapon);  // ?UpdateClassModel@ModelMenu@@IAEXHHH@Z
};
static_assert(sizeof(ModelMenu) == 0x110,
              "ModelMenu size mismatch");

class __declspec(align(16)) WeaponSelectMenu : public ModelMenu {
public:
    bool Allow_Exit;               // +0x110
    uint8_t _pad111[3];
    FEComboBox* weaponCombo;       // +0x114
    ae_array<PanelQuad*, 7> m_pClassIcons;   // +0x118
    int  m_playerclass;            // +0x134
    FEText* m_pClassOptionHeader;  // +0x138
    ae_array<FEText*, 4> m_pTextKitLine;     // +0x13C
    ae_array<FEText*, 6> m_pSlotTextLine;    // +0x14C
    ae_array<PanelQuad*, 5> m_pSlotGauge[6]; // +0x164
    MultiplayerMgr* m_pMPM;        // +0x1DC
    short m_sLocalPlayerTeam;      // +0x1E0

    WeaponSelectMenu(FEMenuSystem* pauseMenuSystem);  // ??0WeaponSelectMenu@@QAE@PAVFEMenuSystem@@@Z
    virtual void OnDeactivate(FEMenu* m);  // ?OnDeactivate@WeaponSelectMenu@@UAEXPAVFEMenu@@@Z
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@WeaponSelectMenu@@UAEXPAVPanelFile@@@Z
    virtual void OnStart(int c);           // ?OnStart@WeaponSelectMenu@@UAEXH@Z
    virtual void OnCross(int c);           // ?OnCross@WeaponSelectMenu@@UAEXH@Z
    virtual void OnUp(int c);              // ?OnUp@WeaponSelectMenu@@UAEXH@Z
    virtual void OnDown(int c);            // ?OnDown@WeaponSelectMenu@@UAEXH@Z
    virtual void Select(int entryNum);     // ?Select@WeaponSelectMenu@@UAEXH@Z
    virtual void OnTriangle(int controllerIndex);  // ?OnTriangle@WeaponSelectMenu@@UAEXH@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@WeaponSelectMenu@@UAEXPAVPanelFile@@@Z
    virtual void Update(float time_inc);  // ?Update@WeaponSelectMenu@@UAEXM@Z
    virtual void OnActivate();          // ?OnActivate@WeaponSelectMenu@@UAEXXZ
protected:
    int PlayerClassToLocalIndex(int playerclass);  // ?PlayerClassToLocalIndex@WeaponSelectMenu@@IAEHH@Z
    EPlayerClass LocalIndexToPlayerClass(int index);  // ?LocalIndexToPlayerClass@WeaponSelectMenu@@IAE?AW4EPlayerClass@@H@Z
    void ActivationToggle(bool a_bToggle);  // ?ActivationToggle@WeaponSelectMenu@@IAEX_N@Z
    void SetClassOptionHeader();           // ?SetClassOptionHeader@WeaponSelectMenu@@IAEXXZ
    void CloseMenu();                      // ?CloseMenu@WeaponSelectMenu@@IAEXXZ
    void SetClassGauges();                 // ?SetClassGauges@WeaponSelectMenu@@IAEXXZ
    void ClearClassGauges();               // ?ClearClassGauges@WeaponSelectMenu@@IAEXXZ
    void SetSwitchKit();                   // ?SetSwitchKit@WeaponSelectMenu@@IAEXXZ
    void SetPanelFileSplitScreen(PanelFile* pf);  // ?SetPanelFileSplitScreen@WeaponSelectMenu@@IAEXPAVPanelFile@@@Z
    void SetPanelFileMain(PanelFile* pf);  // ?SetPanelFileMain@WeaponSelectMenu@@IAEXPAVPanelFile@@@Z
};
static_assert(sizeof(WeaponSelectMenu) == 0x1F0,
              "WeaponSelectMenu size mismatch");

class __declspec(align(16)) InGameSwitchSides : public ModelMenu {
public:
    int  m_eTeam;                 // +0x110 (team_t)
    uint8_t _pad2[0x120 - 0x114];

    virtual ~InGameSwitchSides();          // ??1InGameSwitchSides@@UAE@XZ
    InGameSwitchSides(FEMenuSystem* s);    // ??0InGameSwitchSides@@QAE@PAVFEMenuSystem@@@Z
    virtual void Init();                          // ?Init@InGameSwitchSides@@UAEXXZ
    void OnDeactivate(ModelMenu* m);              // ?OnDeactivate@InGameSwitchSides@@QAEXPAVModelMenu@@@Z
    virtual void OnTriangle(int c);               // ?OnTriangle@InGameSwitchSides@@UAEXH@Z
    virtual void OnCross(int c);                  // ?OnCross@InGameSwitchSides@@UAEXH@Z
    virtual void OnActivate();                    // ?OnActivate@InGameSwitchSides@@UAEXXZ
    virtual void OnUp(int c);                     // ?OnUp@InGameSwitchSides@@UAEXH@Z
    virtual void OnDown(int c);                   // ?OnDown@InGameSwitchSides@@UAEXH@Z
    virtual void SetPanelFile(PanelFile* pf);     // ?SetPanelFile@InGameSwitchSides@@UAEXPAVPanelFile@@@Z
    void SwitchTeams();                           // ?SwitchTeams@InGameSwitchSides@@QAEXXZ
protected:
    static bool ResponseNoNevermind(int index);  // ?ResponseNoNevermind@InGameSwitchSides@@KA_NH@Z
    static void ResponseGoBack(int client);      // ?ResponseGoBack@InGameSwitchSides@@KAXH@Z
    static bool ResponseYesSwitch(int client);   // ?ResponseYesSwitch@InGameSwitchSides@@KA_NH@Z
    void NotifySameTeam();                        // ?NotifySameTeam@InGameSwitchSides@@IAEXXZ
    void AttemptSwitchTeam();                     // ?AttemptSwitchTeam@InGameSwitchSides@@IAEXXZ
    void PickTeam();                             // ?PickTeam@InGameSwitchSides@@IAEXXZ
    void UpdateModel();                          // ?UpdateModel@InGameSwitchSides@@IAEXXZ
    void SwapMenus();                            // ?SwapMenus@InGameSwitchSides@@MAEXXZ
    void SetPanelFileSplitScreen(PanelFile* pf); // ?SetPanelFileSplitScreen@InGameSwitchSides@@IAEXPAVPanelFile@@@Z
    void SetPanelFileMain(PanelFile* pf);        // ?SetPanelFileMain@InGameSwitchSides@@IAEXPAVPanelFile@@@Z
};
static_assert(sizeof(InGameSwitchSides) == 0x120,
              "InGameSwitchSides size mismatch");

// ============================================================================
// InGameScoreBoard - 992 bytes (0x3E0)
// ============================================================================
class InGameScoreBoard : public FEMenu {
public:
    struct sScoreboardPlayerSlot {
        int iPlayerIndex;   // +0x00
        int iScore;         // +0x04
        Entity* pEntity;    // +0x08
    };
    ae_array<sScoreboardPlayerSlot, 16> m_playerList;  // +0x4C
    bool m_bPreviousCursorState;           // +0x10C
    uint8_t _pad10D[3];
    ae_array<PanelQuad*, 15> m_pBackgroundArt;   // +0x110
    ae_array<PanelQuad*, 3> m_pTeamStripQuad;    // +0x14C
    PanelQuad* m_pScrollbarPlayerHilite;         // +0x158
    ae_array<FEText*, 12> m_pSlotTextName;       // +0x15C
    ae_array<FEText*, 13> m_pUppercaseText;  // +0x18C
    ae_array<FEText*, 12> m_pSlotClassText;      // +0x1C0
    ae_array<FEText*, 12> m_pSlotPlayerNameText; // +0x1F0
    color32 m_co32PlayerNameColor;               // +0x220
    ae_array<FEText*, 12> m_pSlotScoreText;      // +0x224
    color32 m_co32ScoreTextColor;                // +0x254
    ae_array<FEText*, 12> m_pSlotKillsText;      // +0x258
    color32 m_co32KillsTextColor;                // +0x288
    ae_array<FEText*, 12> m_pSlotDeathText;      // +0x28C
    color32 m_co32DeathTextColor;                // +0x2BC
    bool m_bShowMyTeamScore;           // +0x2C0
    int  m_iShowMyTeamScorePadOffset;  // +0x2C4
    int  m_iShowOtherTeamScorePadOffset;  // +0x2C8
    int  m_cgTeamShown;                // +0x2CC (team_t)
    int  mVersion;                     // +0x2D0
    bool m_bActivated;                 // +0x2D4
    uint8_t _pad2D5[3];
    UIPlayerListBox m_ListBox;         // +0x2D8
    uint8_t _pad3[0x3E0 - (0x2D8 + 0x108)];

    InGameScoreBoard(FEMenuSystem* pauseMenuSystem);  // ??0InGameScoreBoard@@QAE@PAVFEMenuSystem@@@Z
    virtual void Init();              // ?Init@InGameScoreBoard@@UAEXXZ
    virtual void OnTriangle(int c);   // ?OnTriangle@InGameScoreBoard@@UAEXH@Z
    virtual void OnCross(int c);      // ?OnCross@InGameScoreBoard@@UAEXH@Z
    virtual void OnSquare(int c);     // ?OnSquare@InGameScoreBoard@@UAEXH@Z
    virtual void OnUp(int c);         // ?OnUp@InGameScoreBoard@@UAEXH@Z
    virtual void OnDown(int c);       // ?OnDown@InGameScoreBoard@@UAEXH@Z
    virtual void OnActivate();        // ?OnActivate@InGameScoreBoard@@UAEXXZ
    virtual void OnSelect(int c);     // ?OnSelect@InGameScoreBoard@@UAEXH@Z
    virtual void UpdateSplitScreen(); // ?UpdateSplitScreen@InGameScoreBoard@@UAEXXZ
    virtual void OnDeactivate(FEMenu* m);  // ?OnDeactivate@InGameScoreBoard@@UAEXPAVFEMenu@@@Z
    virtual void Draw();              // ?Draw@InGameScoreBoard@@UAEXXZ
    virtual void OnButtonRelease(int c, int b);  // ?OnButtonRelease@InGameScoreBoard@@UAEXHH@Z
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@InGameScoreBoard@@UAEXPAVPanelFile@@@Z
protected:
    int GetAlliesScore();             // ?GetAlliesScore@InGameScoreBoard@@IAEHXZ
    int GetAxisScore();               // ?GetAxisScore@InGameScoreBoard@@IAEHXZ
    void SetPanelContents();          // ?SetPanelContents@InGameScoreBoard@@IAEXXZ
    virtual void SetWinningTeam(team_t team);  // ?SetWinningTeam@InGameScoreBoard@@MAEXW4team_t@@@Z
    void RecalculateWinningTeam();    // ?RecalculateWinningTeam@InGameScoreBoard@@IAEXXZ
};
static_assert(sizeof(InGameScoreBoard) == 0x3E0,
              "InGameScoreBoard size mismatch");

// ============================================================================
// AAR menu family (AARBaseMenu + scoreboards + stats)
// ============================================================================
class AARBaseMenu : public FEMenu {
public:
    int mClient;                       // +0x4C
    PanelQuadFader mLeftArrowFader;    // +0x50
    PanelQuadFader mRightArrowFader;   // +0x68
    ae_array<FEText*, 2> m_pTimerText; // +0x80
    AARBaseMenu(FEMenuSystem* s, int entry_count);  // ??0AARBaseMenu@@QAE@PAVFEMenuSystem@@H@Z
    void SetTimerText();               // ?SetTimerText@AARBaseMenu@@QAEXXZ
    virtual ~AARBaseMenu();            // ??1AARBaseMenu@@UAE@XZ
    virtual void OnStart(int c);       // ?OnStart@AARBaseMenu@@UAEXH@Z
    virtual void OnL1(int c);          // ?OnL1@AARBaseMenu@@UAEXH@Z
    virtual void OnR1(int c);          // ?OnR1@AARBaseMenu@@UAEXH@Z
    virtual void OnActivate();         // ?OnActivate@AARBaseMenu@@UAEXXZ
    virtual void Update(float time_inc);  // ?Update@AARBaseMenu@@UAEXM@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@AARBaseMenu@@UAEXPAVPanelFile@@@Z
};
static_assert(sizeof(AARBaseMenu) == 0x88,
              "AARBaseMenu size mismatch");

class AARScoreboardBase : public AARBaseMenu {
public:
    struct sScoreboardPlayerSlot {
        int iPlayerIndex;   // +0x00
        int iScore;         // +0x04
        int iKills;         // +0x08
        int iDeaths;        // +0x0C
        Entity* pEntity;    // +0x10
    };
    ae_array<sScoreboardPlayerSlot, 16> m_playerList;  // +0x88
    bool m_bPreviousCursorState;       // +0x1C8
    uint8_t _pad1[0x1CC - (0x1C8 + 1)];
    ae_array<PanelQuad*, 6> m_pYourTeamScore;  // +0x1CC
    ae_array<FEText*, 13> m_pUppercaseText;  // +0x1E4
    bool m_bShowMyTeamScore;           // +0x218
    uint8_t _pad219[3];
    int  m_iShowMyTeamScorePadOffset;  // +0x21C
    int  m_iShowOtherTeamScorePadOffset;  // +0x220
    int  m_cgTeamShown;                // +0x224 (team_t)
    UIPlayerListBox m_ListBox;         // +0x228
    bool mFirstUpdate;                 // +0x330
    int  m_iSecondaryScoreAxis[3];     // +0x334
    int  m_iSecondaryScoreAllies[3];   // +0x340

    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@AARScoreboardBase@@UAEXPAVPanelFile@@@Z
    AARScoreboardBase(FEMenuSystem* pauseMenuSystem);  // ??0AARScoreboardBase@@QAE@PAVFEMenuSystem@@@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@AARScoreboardBase@@UAEXPAVPanelFile@@@Z
    virtual void Init();                            // ?Init@AARScoreboardBase@@UAEXXZ
    virtual void OnSquare(int c);                   // ?OnSquare@AARScoreboardBase@@UAEXH@Z
    virtual void OnLeft(int c);                     // ?OnLeft@AARScoreboardBase@@UAEXH@Z
    virtual void OnRight(int c);                    // ?OnRight@AARScoreboardBase@@UAEXH@Z
    virtual void OnUp(int c);                       // ?OnUp@AARScoreboardBase@@UAEXH@Z
    virtual void OnDown(int c);                     // ?OnDown@AARScoreboardBase@@UAEXH@Z
    virtual void Update(float time_inc);            // ?Update@AARScoreboardBase@@UAEXM@Z
    virtual void Draw();                            // ?Draw@AARScoreboardBase@@UAEXXZ
    virtual void OnActivate();                      // ?OnActivate@AARScoreboardBase@@UAEXXZ
    void OnDeactivate(AARBaseMenu* m);              // ?OnDeactivate@AARScoreboardBase@@QAEXPAVAARBaseMenu@@@Z
protected:
    int GetAlliesScore();             // ?GetAlliesScore@AARScoreboardBase@@IAEHXZ
    int GetAxisScore();               // ?GetAxisScore@AARScoreboardBase@@IAEHXZ
    virtual void SetWinningTeam(team_t team);  // ?SetWinningTeam@AARScoreboardBase@@MAEXW4team_t@@@Z
    void RecalculateWinningTeam();    // ?RecalculateWinningTeam@AARScoreboardBase@@IAEXXZ
};
static_assert(sizeof(AARScoreboardBase) == 0x34C,
              "AARScoreboardBase size mismatch");

class AARScoreboardLoser : public AARScoreboardBase {
public:
    AARScoreboardLoser(FEMenuSystem* pauseMenuSystem);  // ??0AARScoreboardLoser@@QAE@PAVFEMenuSystem@@@Z
    virtual void SetWinningTeam(team_t team);  // ?SetWinningTeam@AARScoreboardLoser@@UAEXW4team_t@@@Z
    virtual void OnR1(int c);   // ?OnR1@AARScoreboardLoser@@UAEXH@Z
    virtual void OnL1(int c);   // ?OnL1@AARScoreboardLoser@@UAEXH@Z
    virtual void OnActivate();  // ?OnActivate@AARScoreboardLoser@@UAEXXZ
    virtual void Update(float time_inc);  // ?Update@AARScoreboardLoser@@UAEXM@Z
    virtual void Draw();                  // ?Draw@AARScoreboardLoser@@UAEXXZ
    virtual ~AARScoreboardLoser();        // ??1AARScoreboardLoser@@UAE@XZ
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@AARScoreboardLoser@@UAEXPAVPanelFile@@@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@AARScoreboardLoser@@UAEXPAVPanelFile@@@Z
};
static_assert(sizeof(AARScoreboardLoser) == 0x34C,
              "AARScoreboardLoser size mismatch");

class AARScoreboardWinner : public AARScoreboardBase {
public:
    AARScoreboardWinner(FEMenuSystem* pauseMenuSystem);  // ??0AARScoreboardWinner@@QAE@PAVFEMenuSystem@@@Z
    virtual void SetWinningTeam(team_t team);  // ?SetWinningTeam@AARScoreboardWinner@@UAEXW4team_t@@@Z
    virtual void OnActivate();  // ?OnActivate@AARScoreboardWinner@@UAEXXZ
    virtual void Update(float time_inc);  // ?Update@AARScoreboardWinner@@UAEXM@Z
    virtual void Draw();                  // ?Draw@AARScoreboardWinner@@UAEXXZ
    virtual void OnR1(int c);             // ?OnR1@AARScoreboardWinner@@UAEXH@Z
    virtual void OnL1(int c);             // ?OnL1@AARScoreboardWinner@@UAEXH@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@AARScoreboardWinner@@UAEXPAVPanelFile@@@Z
    virtual ~AARScoreboardWinner();       // ??1AARScoreboardWinner@@UAE@XZ
};
static_assert(sizeof(AARScoreboardWinner) == 0x34C,
              "AARScoreboardWinner size mismatch");

class AARPersonalStats : public AARBaseMenu {
public:
    ae_array<PanelQuad*, 12> m_pBackgroundArt;  // +0x88
    ae_array<PanelQuad*, 7> m_pClassIcon;  // +0xB8
    int  mPlayerClass;             // +0xD4 (EPlayerClass)
    ae_array<PanelQuad*, 2> m_pScrollArrow;  // +0xD8
    bool m_bShowScrollArrowLeft;      // +0xE0
    bool m_bShowScrollArrowRight;     // +0xE1
    bool m_bHighlightScrollArrowLeft; // +0xE2
    bool m_bHighlightScrollArrowRight;// +0xE3
    int  m_ePanelToSwitchTo;          // +0xE4
    ae_array<FEText*, 4> m_pText;     // +0xE8
    ae_array<FEText*, 14> m_pScoreText;  // +0xF8
    ae_array<FEText*, 8> m_pClassScoreText;  // +0x130
    uint8_t _pad3[0x150 - (0x130 + 32)];

    AARPersonalStats(FEMenuSystem* s);  // ??0AARPersonalStats@@QAE@PAVFEMenuSystem@@@Z
    static AARPersonalStats* Me();    // ?Me@AARPersonalStats@@SAPAV1@XZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@AARPersonalStats@@UAEXPAVPanelFile@@@Z
    virtual void Init();              // ?Init@AARPersonalStats@@UAEXXZ
    virtual void OnActivate();        // ?OnActivate@AARPersonalStats@@UAEXXZ
    virtual void Draw();              // ?Draw@AARPersonalStats@@UAEXXZ
    void OnDeactivate(AARBaseMenu* m);  // ?OnDeactivate@AARPersonalStats@@QAEXPAVAARBaseMenu@@@Z
    virtual void OnCross(int c);      // ?OnCross@AARPersonalStats@@UAEXH@Z
    virtual void OnUp(int c);         // ?OnUp@AARPersonalStats@@UAEXH@Z
    virtual void OnDown(int c);       // ?OnDown@AARPersonalStats@@UAEXH@Z
    virtual void OnLeft(int c);       // ?OnLeft@AARPersonalStats@@UAEXH@Z
    virtual void OnRight(int c);      // ?OnRight@AARPersonalStats@@UAEXH@Z
    virtual void OnR1(int c);         // ?OnR1@AARPersonalStats@@UAEXH@Z
    virtual void OnL1(int c);         // ?OnL1@AARPersonalStats@@UAEXH@Z
    virtual void Update(float time_inc);  // ?Update@AARPersonalStats@@UAEXM@Z
protected:
    void SetPanelHelpBar();           // ?SetPanelHelpBar@AARPersonalStats@@IAEXXZ
    void SetGenericScores();          // ?SetGenericScores@AARPersonalStats@@IAEXXZ
    void SetClassSpecificEntries();   // ?SetClassSpecificEntries@AARPersonalStats@@IAEXXZ
    void GetClassSpecificScore(EPlayerClass a_ePlayerClass,
                               int& a_iClassScore, int& a_iTimeAsClass,
                               int& a_iClassSpecificScore1,
                               int& a_iClassSpecificScore2);  // ?GetClassSpecificScore@AARPersonalStats@@IAEXW4EPlayerClass@@AAH111@Z
};
static_assert(sizeof(AARPersonalStats) == 0x150,
              "AARPersonalStats size mismatch");

class AARMapVote : public AARBaseMenu {
public:
    ae_array<PanelQuad*, 10> m_pBackgroundArt;  // +0x88
    ae_array<PanelQuad*, 2> m_pScrollArrow;  // +0xB0
    ae_array<PanelQuad*, 6> m_pScrollbar;    // +0xB8
    bool m_bShowScrollArrowLeft;      // +0xD0
    bool m_bShowScrollArrowRight;     // +0xD1
    bool m_bHighlightScrollArrowLeft; // +0xD2
    bool m_bHighlightScrollArrowRight;// +0xD3
    int  m_ePanelToSwitchTo;          // +0xD4
    int  m_iSelectedMap;              // +0xD8
    int  m_currentRow;                // +0xDC
    UIHighlightListBox m_ListBox;  // +0xE0
    ae_array<FEText*, 5> m_pText;  // +0x1C0
    ae_array<FEText*, 12> m_pMapNames;   // +0x1D4
    ae_array<FEText*, 12> m_pMapVotes;   // +0x204
    int* m_pMapVoteVals;                 // +0x234

    AARMapVote(FEMenuSystem* s);  // ??0AARMapVote@@QAE@PAVFEMenuSystem@@@Z
    static AARMapVote* Me();          // ?Me@AARMapVote@@SAPAV1@XZ
    void TallyVotes();                // ?TallyVotes@AARMapVote@@QAEXXZ
    static unsigned char m_FirstTimeAccessedByte;  // ?m_FirstTimeAccessedByte@AARMapVote@@1EA @ 0x1388D58
    virtual void Init();              // ?Init@AARMapVote@@UAEXXZ
    virtual void Draw();              // ?Draw@AARMapVote@@UAEXXZ
    virtual void OnCross(int c);      // ?OnCross@AARMapVote@@UAEXH@Z
    virtual void OnActivate();        // ?OnActivate@AARMapVote@@UAEXXZ
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@AARMapVote@@UAEXPAVPanelFile@@@Z
    void OnDeactivate(AARBaseMenu* __formal);  // ?OnDeactivate@AARMapVote@@QAEXPAVAARBaseMenu@@@Z
    virtual void OnR1(int c);         // ?OnR1@AARMapVote@@UAEXH@Z
    virtual void OnL1(int c);         // ?OnL1@AARMapVote@@UAEXH@Z
    virtual void OnLeft(int c);       // ?OnLeft@AARMapVote@@UAEXH@Z
    virtual void OnRight(int c);      // ?OnRight@AARMapVote@@UAEXH@Z
    virtual void OnUp(int c);         // ?OnUp@AARMapVote@@UAEXH@Z
    virtual void OnDown(int c);       // ?OnDown@AARMapVote@@UAEXH@Z
    virtual void Update(float time_inc);  // ?Update@AARMapVote@@UAEXM@Z
    virtual ~AARMapVote();            // ??1AARMapVote@@UAE@XZ
protected:
    void SelectMap(int indexMap);     // ?SelectMap@AARMapVote@@IAEXH@Z
};
static_assert(sizeof(AARMapVote) == 0x238,
              "AARMapVote size mismatch");

class AARGameModeVote : public AARBaseMenu {
public:
    ae_array<PanelQuad*, 9> m_pBackgroundArt;  // +0x88
    ae_array<PanelQuad*, 2> m_pScrollArrow;  // +0xAC
    bool m_bShowScrollArrowLeft;      // +0xB4
    bool m_bShowScrollArrowRight;     // +0xB5
    bool m_bHighlightScrollArrowLeft; // +0xB6
    bool m_bHighlightScrollArrowRight;// +0xB7
    int  m_ePanelToSwitchTo;          // +0xB8
    int  m_iSelectedMode;             // +0xBC
    int  m_currentRow;                // +0xC0
    UIHighlightListBox m_ListBox;     // +0xC4
    ae_array<FEText*, 5> m_pText;     // +0x1A4
    ae_array<FEText*, 7> m_pModeNames; // +0x1B8
    ae_array<FEText*, 7> m_pModeVotes; // +0x1D4
    ae_array<int, 7> m_pModeVoteVals;  // +0x1F0

    AARGameModeVote(FEMenuSystem* s);  // ??0AARGameModeVote@@QAE@PAVFEMenuSystem@@@Z
    static AARGameModeVote* Me();     // ?Me@AARGameModeVote@@SAPAV1@XZ
    void TallyVotes();                // ?TallyVotes@AARGameModeVote@@QAEXXZ
    virtual void Init();              // ?Init@AARGameModeVote@@UAEXXZ
    void OnDeactivate(AARBaseMenu* __formal);  // ?OnDeactivate@AARGameModeVote@@QAEXPAVAARBaseMenu@@@Z
    virtual void OnLeft(int c);       // ?OnLeft@AARGameModeVote@@UAEXH@Z
    virtual void OnRight(int c);      // ?OnRight@AARGameModeVote@@UAEXH@Z
    virtual void OnR1(int c);         // ?OnR1@AARGameModeVote@@UAEXH@Z
    virtual void OnL1(int c);         // ?OnL1@AARGameModeVote@@UAEXH@Z
    virtual void OnUp(int c);         // ?OnUp@AARGameModeVote@@UAEXH@Z
    virtual void OnDown(int c);       // ?OnDown@AARGameModeVote@@UAEXH@Z
    virtual void OnActivate();        // ?OnActivate@AARGameModeVote@@UAEXXZ
    virtual void OnCross(int c);      // ?OnCross@AARGameModeVote@@UAEXH@Z
    virtual void Draw();              // ?Draw@AARGameModeVote@@UAEXXZ
    virtual void PanelFileUnloaded(PanelFile* pPanelFile);  // ?PanelFileUnloaded@AARGameModeVote@@UAEXPAVPanelFile@@@Z
    virtual void Update(float time_inc);  // ?Update@AARGameModeVote@@UAEXM@Z
    virtual ~AARGameModeVote();       // ??1AARGameModeVote@@UAE@XZ
protected:
    void SelectMode(int indexMode);   // ?SelectMode@AARGameModeVote@@IAEXH@Z
};
static_assert(sizeof(AARGameModeVote) == 0x20C,
              "AARGameModeVote size mismatch");

// ============================================================================
// PauseMenu / HotJoinMenu / SpectateMenu
// ============================================================================
class AARPauseMenu : public FEMenu {
public:
    int m_iLastSelection;                   // +0x4C
    AARPauseMenu(FEMenuSystem* pSystem);    // ??0AARPauseMenu@@QAE@PAVFEMenuSystem@@@Z
    virtual void Update(float time_inc);    // ?Update@AARPauseMenu@@UAEXM@Z
    virtual ~AARPauseMenu();                    // ??1AARPauseMenu@@UAE@XZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@AARPauseMenu@@UAEXPAVPanelFile@@@Z
    virtual void UpdateSplitScreen();           // ?UpdateSplitScreen@AARPauseMenu@@UAEXXZ
    virtual void SetPanelFile(PanelFile* pf);   // ?SetPanelFile@AARPauseMenu@@UAEXPAVPanelFile@@@Z
    virtual void OnUp(int c);                   // ?OnUp@AARPauseMenu@@UAEXH@Z
    virtual void OnDown(int c);                 // ?OnDown@AARPauseMenu@@UAEXH@Z
    virtual void OnTriangle(int c);             // ?OnTriangle@AARPauseMenu@@UAEXH@Z
    virtual void OnStart(int c);                // ?OnStart@AARPauseMenu@@UAEXH@Z
    virtual void OnCross(int c);                // ?OnCross@AARPauseMenu@@UAEXH@Z
    virtual void OnActivate();                  // ?OnActivate@AARPauseMenu@@UAEXXZ
    virtual void Draw();                        // ?Draw@AARPauseMenu@@UAEXXZ
    virtual void ButtonHeldAction();            // ?ButtonHeldAction@AARPauseMenu@@UAEXXZ
    virtual void OnDeactivate(FEMenu* pMenu);   // ?OnDeactivate@AARPauseMenu@@UAEXPAVFEMenu@@@Z
    void UnPause(int client);                   // ?UnPause@AARPauseMenu@@QAEXH@Z
    static bool ResponseYesQuit(int client);    // ?ResponseYesQuit@AARPauseMenu@@SA_NH@Z
    static void Quit(int client);               // ?Quit@AARPauseMenu@@SAXH@Z
    void AttemptQuit();                         // ?AttemptQuit@AARPauseMenu@@QAEXXZ
    static void ResponseGoBack(int client);     // ?ResponseGoBack@AARPauseMenu@@SAXH@Z
private:
    void SetTimerText();                        // ?SetTimerText@AARPauseMenu@@AAEXXZ
};
static_assert(sizeof(AARPauseMenu) == 0x50,
              "AARPauseMenu size mismatch");

class HotJoinMenu : public FEMenu {
public:
    void* mPlayerMgr;                    // +0x4C (MPPlayerManager*)
    int   mController;                   // +0x50
    int   mVersion;                      // +0x54
    HotJoinMenu(FEMenuSystem* pauseMenuSystem);  // ??0HotJoinMenu@@QAE@PAVFEMenuSystem@@@Z
    virtual void OnDeactivate(FEMenu* m);  // ?OnDeactivate@HotJoinMenu@@UAEXPAVFEMenu@@@Z
    virtual void Update(float time_inc);   // ?Update@HotJoinMenu@@UAEXM@Z
    virtual void OnUp(int c);              // ?OnUp@HotJoinMenu@@UAEXH@Z
    virtual void OnStart(int c);           // ?OnStart@HotJoinMenu@@UAEXH@Z
    virtual void OnActivate();             // ?OnActivate@HotJoinMenu@@UAEXXZ
    virtual void UpdateSplitScreen();      // ?UpdateSplitScreen@HotJoinMenu@@UAEXXZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@HotJoinMenu@@UAEXPAVPanelFile@@@Z
    virtual void Draw();                   // ?Draw@HotJoinMenu@@UAEXXZ
    virtual void OnCross(int c);           // ?OnCross@HotJoinMenu@@UAEXH@Z
    virtual void OnDown(int c);            // ?OnDown@HotJoinMenu@@UAEXH@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@HotJoinMenu@@UAEXPAVPanelFile@@@Z
    void Join();                           // ?Join@HotJoinMenu@@QAEXXZ
};
static_assert(sizeof(HotJoinMenu) == 0x58,
              "HotJoinMenu size mismatch");

class SpectateMenu : public FEMenu {
public:
    int   mState;                        // +0x4C (ESpectatorState)
    int   mSeconds;                      // +0x50
    int   mVersion;                      // +0x54
    bool  mMedic;                        // +0x58
    bool  mTeamKill;                     // +0x59
    void* mLastTeamKiller;               // +0x5C (Entity*)
    void* mHeader;                       // +0x60 (FEText*)
    void* mMessage;                      // +0x64 (FEText*)
    void* mTime;                         // +0x68 (FEText*)
    void* mButtonPress;                  // +0x6C (FEText*)
    SpectateMenu(FEMenuSystem* pauseMenuSystem);  // ??0SpectateMenu@@QAE@PAVFEMenuSystem@@@Z
    virtual ~SpectateMenu();  // ??1SpectateMenu@@UAE@XZ
    virtual void OnDeactivate(FEMenu* m);  // ?OnDeactivate@SpectateMenu@@UAEXPAVFEMenu@@@Z
    virtual void OnStart(int c);           // ?OnStart@SpectateMenu@@UAEXH@Z
    virtual void OnSelect(int c);          // ?OnSelect@SpectateMenu@@UAEXH@Z
    virtual void OnCross(int c);           // ?OnCross@SpectateMenu@@UAEXH@Z
    virtual void OnLeft(int c);            // ?OnLeft@SpectateMenu@@UAEXH@Z
    virtual void OnSquare(int c);          // ?OnSquare@SpectateMenu@@UAEXH@Z
    virtual void OnTrueCircle(int c);      // ?OnTrueCircle@SpectateMenu@@UAEXH@Z
    virtual void OnRight(int c);           // ?OnRight@SpectateMenu@@UAEXH@Z
    virtual void OnActivate(int prev);     // ?OnActivate@SpectateMenu@@UAEXH@Z
    virtual void Update(float time_inc);   // ?Update@SpectateMenu@@UAEXM@Z
    virtual void Draw();                   // ?Draw@SpectateMenu@@UAEXXZ
    virtual void PanelFileUnloaded(PanelFile* pf);  // ?PanelFileUnloaded@SpectateMenu@@UAEXPAVPanelFile@@@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@SpectateMenu@@UAEXPAVPanelFile@@@Z
    virtual void UpdateSplitScreen();      // ?UpdateSplitScreen@SpectateMenu@@UAEXXZ
    virtual void UpdateWidescreen(bool widescreen);  // ?UpdateWidescreen@SpectateMenu@@UAEX_N@Z
    void UpdateState();                    // ?UpdateState@SpectateMenu@@QAEXXZ
    void UpdateHelpbar();                  // ?UpdateHelpbar@SpectateMenu@@QAEXXZ
};
static_assert(sizeof(SpectateMenu) == 0x70,
              "SpectateMenu size mismatch");

// PauseMenu - split-screen pause (0x6C: FESplitScreenMenu + m_iLastSelection)
class PauseMenu : public FESplitScreenMenu {
public:
    int m_iLastSelection;  // +0x68
    void SetGameSettingsText();  // ?SetGameSettingsText@PauseMenu@@QAEXXZ
    void UnPause();        // ?UnPause@PauseMenu@@QAEXXZ (shell.o)
    PauseMenu(FEMenuSystem* s);  // ??0PauseMenu@@QAE@PAVFEMenuSystem@@@Z (mp_shell.o 0x7AB7F0)
    virtual ~PauseMenu();  // ??1PauseMenu@@UAE@XZ (mp_shell.o 0x7AB850)
    virtual void OnDeactivate(FEMenu* m);  // ?OnDeactivate@PauseMenu@@UAEXPAVFEMenu@@@Z
    virtual void ButtonHeldAction();       // ?ButtonHeldAction@PauseMenu@@UAEXXZ
    virtual void Update(float time_inc);   // ?Update@PauseMenu@@UAEXM@Z
    virtual void SetPanelFile(PanelFile* pf);  // ?SetPanelFile@PauseMenu@@UAEXPAVPanelFile@@@Z
    virtual void OnActivate();             // ?OnActivate@PauseMenu@@UAEXXZ
    virtual void OnCross(int c);           // ?OnCross@PauseMenu@@UAEXH@Z
    virtual void UpdateSplitScreen();      // ?UpdateSplitScreen@PauseMenu@@UAEXXZ
    void UnPause(int client);             // ?UnPause@PauseMenu@@QAEXH@Z (mp_shell.o)
protected:
    void Quit();           // ?Quit@PauseMenu@@IAEXXZ (mp_shell.o 0x791BB0)
    void Suicide();        // ?Suicide@PauseMenu@@IAEXXZ (mp_shell.o 0x791D00)
    void TeamChange();     // ?TeamChange@PauseMenu@@IAEXXZ
    void AttemptQuit();    // ?AttemptQuit@PauseMenu@@IAEXXZ
    void AttemptTeamChange();  // ?AttemptTeamChange@PauseMenu@@IAEXXZ
    void AttemptSuicide();  // ?AttemptSuicide@PauseMenu@@IAEXXZ
    void SetPanelFileSplitScreen(PanelFile* pf);  // ?SetPanelFileSplitScreen@PauseMenu@@IAEXPAVPanelFile@@@Z
    void SetPanelFileMain(PanelFile* pf);  // ?SetPanelFileMain@PauseMenu@@IAEXPAVPanelFile@@@Z
    static bool ResponseYesTeamChange(int client);  // ?ResponseYesTeamChange@PauseMenu@@KA_NH@Z
    static void ResponseGoBack(int client);         // ?ResponseGoBack@PauseMenu@@KAXH@Z
    static bool ResponseYesQuit(int client);        // ?ResponseYesQuit@PauseMenu@@KA_NH@Z
    static bool ResponseYesSuicide(int client);     // ?ResponseYesSuicide@PauseMenu@@KA_NH@Z
};
static_assert(sizeof(PauseMenu) == 0x6C,
              "PauseMenu size mismatch");

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
    OverlayMenu(FEMenuSystem* s, int numEntries);  // ??0OverlayMenu@@QAE@PAVFEMenuSystem@@H@Z
    const eState GetState();              // ?GetState@OverlayMenu@@QAE?BW4eState@1@XZ
    void SetState(eState state);          // ?SetState@OverlayMenu@@QAEXW4eState@1@@Z
    virtual void Select(int entry_num);   // ?Select@OverlayMenu@@UAEXH@Z
    virtual void OnUp(int c);             // ?OnUp@OverlayMenu@@UAEXH@Z
    virtual void OnDown(int c);           // ?OnDown@OverlayMenu@@UAEXH@Z
    virtual void OnTriangle(int c);       // ?OnTriangle@OverlayMenu@@UAEXH@Z
    virtual void OnSquare(int c);         // ?OnSquare@OverlayMenu@@UAEXH@Z
    virtual void OnCircle(int c);         // ?OnCircle@OverlayMenu@@UAEXH@Z
    virtual void OnCross(int c);          // ?OnCross@OverlayMenu@@UAEXH@Z
    virtual void Update(float time_inc);  // ?Update@OverlayMenu@@UAEXM@Z
    virtual ~OverlayMenu();               // ??1OverlayMenu@@UAE@XZ
protected:
    void LogonUpdate();                   // ?LogonUpdate@OverlayMenu@@IAEXXZ
};
static_assert(sizeof(OverlayMenu) == 0x148,
              "OverlayMenu size mismatch");
