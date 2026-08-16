// ============================================================================
// session_menus.h - mp_shell.o session/overlay menu classes
// Reconstructed from IDA local types (PDB symbol data). Sizes verified.
// ============================================================================

#pragma once

#include "game/shell/shell_types.h"

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
// OverlayMenuBase - FEMenu + 3 ints (0x58), verified against IDA
// ============================================================================
class OverlayMenuBase : public FEMenu {
public:
    int mVersion;      // +0x4C
    int mAcceptMenu;   // +0x50
    int mBackMenu;     // +0x54
};
static_assert(sizeof(OverlayMenuBase) == 0x58,
              "OverlayMenuBase size mismatch");

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
};
static_assert(sizeof(OverlayMenu) == 0x148,
              "OverlayMenu size mismatch");
