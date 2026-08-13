// ============================================================================
// XboxLiveMenus.h - Xbox Live options menus (game_xbox.o mp/ui/*.cpp)
// Reconstructed from IDA local types. All sizes verified.
// ============================================================================

#pragma once

#include "game/ui_types.h"
#include "game/core/core_systems.h"
#include "MPLiveEngine.h"

// ============================================================================
// UIListBox - 0xAC bytes (verified against IDA; shell.o owns the impl)
// ============================================================================
struct UIListBoxRow;
struct UIListBoxDataRow;
struct PanelQuadFader {
    unsigned char _pad[0x18];
};
static_assert(sizeof(PanelQuadFader) == 0x18, "PanelQuadFader size mismatch");

struct UIListBox {
    void** __vftable;                    // +0x00
    ae_vector<UIListBoxRow*> mItemRows;  // +0x04
    ae_vector<UIListBoxDataRow*> mDataRows;  // +0x10
    bool mSelectedFlashing;              // +0x1C
    ae_vector<color32> mSelectedRowOriginalColor;      // +0x20
    ae_vector<bool> mSelectedRowColorChangeColumns;    // +0x2C
    PanelQuad* mScrollBarQuads[6];       // +0x38
    int mLastRowContainingData;          // +0x50
    int mItemRowsCount;                  // +0x54
    int mItemColumnsCount;               // +0x58
    int mDataRowsCount;                  // +0x5C
    int mTopLine;                        // +0x60
    int mSelectedLine;                   // +0x64
    int mScrollBarTopY;                  // +0x68
    int mScrollBarBottomY;               // +0x6C
    float mScrollBarYInc;                // +0x70
    bool mIsWrapping;                    // +0x74
    bool mBlockRefresh;                  // +0x75
    int mIncrementBy;                    // +0x78
    PanelQuadFader mScrollBarUpFader;    // +0x7C
    PanelQuadFader mScrollBarDownFader;  // +0x94

    UIListBox() {}
    UIListBox(int visibleRows, int visibleColumns, int maxDataRows,
              bool bIsWrapping);
    ~UIListBox();
    void UIListBoxCtor(int visibleRows, int visibleColumns, int maxDataRows,
                       bool bIsWrapping);
    void OnUp(int a2);
    void OnDown(int a2);
    void SelectLine(int line);
    void Update(float time_inc);
    void SetItem(int row, int column, FEText* text, int state);
    void SetText(int row, int column, const char* text);
    void SetAllColumnsSelectable(bool selectable);
    void Refresh();
    void RemoveAllItems();
};
static_assert(sizeof(UIListBox) == 0xAC, "UIListBox size mismatch");

// ============================================================================
// XboxLiveOptionsMenu - 0x188 (verified)
// ============================================================================
class FEComboBox;

class XboxLiveOptionsMenu : public FEMenu {
public:
    ae_array<PanelQuad*, 3> m_pBackgroundArt;   // +0x4C
    ae_array<FEText*, 5> m_pText;               // +0x58
    ae_array<PanelQuad*, 5> m_pRowArt;          // +0x6C
    ae_array<PanelQuad*, 4> m_pLineArt;         // +0x80
    ae_array<color32, 5> m_pOldTextColor;       // +0x90
    ae_array<color32, 5> m_pOldSelectedTextColor; // +0xA4
    UIListBox m_ListBox;                        // +0xB8
    PanelFile* mPanel;                          // +0x164
    int m_currSelection;                        // +0x168
    int mVersion;                               // +0x16C
    FEComboBox* m_pAppearOnline;                // +0x170
    FEMultiLineText* mHelpbar;                  // +0x174
    FEMultiLineText* mInstructionsText;         // +0x178
    bool mWidescreen;                           // +0x17C
    unsigned int friendIcon;                    // +0x180
    bool wasSignedIn;                           // +0x184
    bool waitingForSignIn;                      // +0x185
    bool mJoiningFriend;                        // +0x186

    XboxLiveOptionsMenu(FEMenuSystem* s);
    ~XboxLiveOptionsMenu();
    static XboxLiveOptionsMenu* Me();
    void Init();
    void Draw();
    void OnDeactivate(FEMenu* m);
    void OnUp(int a2, int c);
    void OnDown(int a2, int c);
    void OnLeft(int c);
    void OnRight(int c);
    void OnL1(int c);
    void OnR1(int c);
    void OnTriangle(int c);
    void OnSquare(int c);
    void OnStart(int c);
    void ButtonHeldAction();
    void UpdateSplitScreen();
    void UpdateWidescreen(BOOL widescreen);
    void WireForSignedOut();
    void WireForSignedIn();
    void SetPanelFile(PanelFile* pf);
    void OnActivate();
    void OnCross(int c);
    void OnCircle(int c);
    void UpdateDynamicText();
    void Update(float time_inc);
    void PanelFileUnloaded(PanelFile* pf);
};
static_assert(sizeof(XboxLiveOptionsMenu) == 0x188,
              "XboxLiveOptionsMenu size mismatch");

// ============================================================================
// InGameLiveOptionsMenu - 0x60 (verified)
// ============================================================================
class InGameLiveOptionsMenu : public FEMenu {
public:
    ae_array<FEText*, 2> m_pText;       // +0x4C
    bool mJoiningFriend;                // +0x54
    unsigned int friendIcon;            // +0x58
    bool wasSignedIn;                   // +0x5C
    bool waitingForSignIn;              // +0x5D

    InGameLiveOptionsMenu(FEMenuSystem* s);
    ~InGameLiveOptionsMenu();
    static InGameLiveOptionsMenu* Me();
    void OnActivate();
    void Draw();
    static bool ResponseYesReboot(int);
    static bool ResponseDoNothing(int);
    static bool ResponseNoReboot(int);
    static bool ResponseSignOut(int);
    static bool ResponseYesJoin(int);
    static bool ResponseNoJoin(int);
    void OnTriangle(int c);
    void SetPanelFile(PanelFile* pf);
    void UpdateWidescreen(bool widescreen);
    void TryFriendJoin();
    void TryReboot();
    void Update(float time_inc);
    void Select(int a2, int entry_num);
};
static_assert(sizeof(InGameLiveOptionsMenu) == 0x60,
              "InGameLiveOptionsMenu size mismatch");

// ============================================================================
// Free functions / externs
// ============================================================================
void ShowNotificationIcon(unsigned int* menuIcon, PanelQuad* inviteQuad,
                          PanelQuad* friendQuad);
char* Xbox_LaunchInfo(char* pDestCommandLine);
void RenderUIX(void*);

extern const char* szXBoxOptionReferences[5];
extern const char* szXBoxOptionDescriptionReferences[5];

// shell.o / mp.o externs used by the menus
struct OverlayMenu {
    static OverlayMenu* Me(int version);
    void SetState(int state);
    void Update(int v);
    int mAcceptMenu;
    int mBackMenu;
    int mGameListingNum;
};
struct DialogMenuSystem {
    void BringUp(const char* t, bool type_ok, bool type_yn,
                 const char* title_unloc, bool layer1);
    void AddOption(const char* t, bool (*responseFunc)(int));
    void HighlightOption(int index);
    void Reformat(bool vertical);
    int GetActiveMenu();
    DialogMenu* GetLayer(bool layer1);
};
struct DialogMenu {
    void AddOption(const char* t, bool (*responseFunc)(int));
    void Reformat(bool vertical, int viewport);
};
struct InGameMenuSystem {
    FEMenu** menus;
};
struct FEManager {
    FEMenuSystem* fems;
    struct AARMenuSystemView { FEMenu** menus; }* mAARS;
    InGameMenuSystem* GetIGMS(int client);
    DialogMenuSystem* GetDMS(int client);
    void DrawDiscError();                     // ?DrawDiscError@FEManager@@QAEXXZ
    void UpdateLoadingMenu(float percentDone);  // ?UpdateLoadingMenu@FEManager@@QAEXM@Z
};
extern FEManager g_femanager;
extern int currCl;
extern bool g_controllerConnectedErrorShown[];
extern bool g_IgnoreUIXInput;

// MPUIInterface class view lives in MPLiveEngine.h (included above).

extern int cg_widescreen_integer;
extern void Controller_LockPort(int port);
