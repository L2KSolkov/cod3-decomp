// ============================================================================
// shell_types.h - front-end shell (shell.o) shared types
// Reconstructed from IDA local types (PDB symbol data).
// ============================================================================

#pragma once

#include <stdint.h>
#include <string>
#include <map>
#include <vector>

#include "core/math_types.h"
#include "engine/broc_types.h"
#include "game/ui_types.h"
#include "game/platform_xbox/MemoryUnitManager.h"
#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

// ============================================================================
// Profile classes (shell.o ProfileManager.cpp family) - layouts from IDA
// ============================================================================
// ProfileManager - memory-unit profile manager (108 bytes)
// Size: 0x6C - verified against IDA; derives InsertRemoveObserver
// ============================================================================
class ProfileManager : public MemoryUnitManager::InsertRemoveObserver {
public:
    friend class VKMenu;  // binary calls private FinishOperation from VKMenu
    struct Profile {
        const char* profileName;  // +0x00
        int         profileSlot;  // +0x04
    };
    enum EOperationState {
        kOperationNone = 0,
        kOperationSuccess = 1,
        kOperationFailure = 2,
    };

    ae_sized_array<Profile, 6> mProfileSlots;  // +0x04 (52 bytes)
    SaveGameData** mEnumProfileSlots;          // +0x38
    void (*mCallback)(int);                    // +0x3C
    int mNumProfiles;                          // +0x40
    int mCurrentStatus;                        // +0x44
    int mCurrentOp;                            // +0x48
    bool mSaveEnabled;                         // +0x4C
    bool mQuietSave;                           // +0x4D
    bool mOverwriting;                         // +0x4E
    int mCardId;                               // +0x50
    int mLastCardId;                           // +0x54
    int mCreateOkCardId;                       // +0x58
    int mOverwriteOkCardId;                    // +0x5C
    int mContinueWithoutSavingId;              // +0x60
    EOperationState mOperationState;           // +0x64
    float mCountdownFinishedTime;              // +0x68

    ProfileManager();                       // 0x580D30
    ~ProfileManager();                      // 0x5751E0 (non-virtual QAE)
    static ProfileManager* Me();            // 0x5751D0
    void OperationDone(bool success);       // 0x5751F0
    void Reset();                           // 0x575210
    bool DeviceIsUnformatted();             // 0x575290
    static char* TimeToString(StubData& data);  // 0x575310
    static void SplitTime(int time, int& sec, int& min, int& hour,
                          int& day);        // 0x575390
    const char* GetLoadedProfile() const;   // 0x575420
    bool HasMemCard();                      // 0x580D90
    bool IsProfileNameUsed(const char* name) const;  // 0x580DB0
    int GetProfiles(Profile** slots);       // 0x580E40
    bool EnoughFreeSpace();                 // 0x580ED0
    void SetProfile(SaveGameData* sv);      // 0x580F00
    void Format();                          // 0x58EE10
    void EnumProfiles(SaveGameData** slots);       // 0x5935B0
    void DeleteProfile(int slotNum);               // 0x5936F0
    void DeleteCorruptSave();                      // 0x593860
    void EnumProfiles(SaveGameData** slots,
                      void (*callback)(int));          // 0x594370
    void DeleteProfile(int slotNum,
                       void (*callback)(int));         // 0x5943F0
    void EnumProfilesDone(bool success); // 0x597C40
    void CreateProfile(int slotNum);                   // 0x597E30
    void CreateProfile(int slotNum,
                       void (*callback)(int));         // 0x59AFC0
    void Retry();                                      // 0x59B040
    void SaveProfile(void (*callback)(int), bool quiet);  // 0x59B130
    void Update();                                     // 0x59D5C0
    bool IsSaveEnabled() const;                        // 0x5AEE90
    void SetContinueWithoutSaving();                  // 0x5AEEA0
    bool IsContinueWithoutSaving();                   // 0x5AEEB0
    int GetStatus();                                   // 0x5AEEC0
    bool IsControllingDMS();                           // 0x5AEED0
    void SetStatus(int status);                        // 0x5AEEE0
    void SetOperation(int op);                        // 0x5AEEF0
    void SetLastCardId();                              // 0x5AEF00
    bool HasCardChanged();                             // 0x5AEF10
    void SetOverwriteOk();                             // 0x5AEF30
    bool IsOverwriteOk();                              // 0x5AEF40
    void SetCreateOk();                                // 0x5AEF50
private:
    virtual void Callback(MemoryUnitManager::eDeviceChange change,
                          int deviceID);    // 0x575270 (private virtual EAE)
    bool ActivateMemDevice();               // 0x575280
    void StartCountdown();                  // 0x5752D0
    bool IsCountdownDone() const;           // 0x5752F0
    void IssueCallback();                   // 0x575430
    static bool DialogResponseOperationDone(int);   // 0x575470
    static bool DialogResponseNoData(int);          // 0x5754C0
    static bool DialogResponseContinueNoSave(int);  // 0x575510
    static bool DialogResponseContinue(int);        // 0x575570
    bool FileExists();                      // 0x580EC0
    static bool DialogResponseDeleteCorruptSave(int);  // 0x593910
    static bool DialogResponseConfirmFormat(int);      // 0x593930
    static bool DialogResponseFormat(int);             // 0x5939E0
    void SaveProfile();                                // 0x597FF0
    static bool DialogResponseRetry(int);              // 0x59B1B0
    static bool DialogResponseConfirmCreateSave(int);  // 0x59B1C0
    static bool DialogResponseOverwrite(int);          // 0x59B1F0
    void FinishOperation();                            // 0x59D390
    void DialogDisplayFormatting();      // 0x587130
    void DialogDisplayNoMemDevice();     // 0x5871C0
    static bool DialogResponseFreeMoreBlocks(int);  // 0x587250
    void DialogDisplayAutoSave();        // 0x587260
    void DialogDisplayLoading();         // 0x58EE90
    void DialogDisplayDeleting();        // 0x58EF40
    void DialogDisplaySaving();          // 0x58EFF0
    void DialogDisplayOverwriting();     // 0x58F0B0
    void DialogDisplaySaveDone();        // 0x58F170
    void DialogDisplayDeleteDone();      // 0x58F2D0
    void DialogDisplayNoData();          // 0x58F420
    void DialogDisplayOverwrite();       // 0x58F570
    void DialogDisplayNoFreeSpace();     // 0x58F700
    void DialogDisplayLoadFailed();      // 0x58F8A0
    void DialogDisplaySaveFailed();      // 0x58F9F0
    void DialogDisplayDeleteFailed();    // 0x58FB50
    void DialogDisplayFormatFailed();    // 0x58FCA0
    void DialogDisplayCorruptData();     // 0x594470
    void DialogDisplayFormat();          // 0x594600
    void DialogDisplayFormatSuccess();   // 0x59C720
    void DialogDisplayCreateSave();      // 0x59C870
};
static_assert(sizeof(ProfileManager) == 0x6C,
              "ProfileManager size mismatch");

// ============================================================================
// ProfileMainMenu - profile list menu (140 bytes)
// Size: 0x8C - verified against IDA
// ============================================================================
class ProfileMainMenu : public FEMenu {
public:
    static const char* const kProfileTextGeoms[];  // 0xCEF72C (9)

    int      mMenuState;        // +0x4C
    int      mMenuStatus[6];    // +0x50
    SaveGameData* mSaveSlots[6];// +0x68
    const char* mSelectedProfile;  // +0x80
    PanelFile* mPanel;          // +0x84
    FEMultiLineText* mHelpBar;  // +0x88

    ProfileMainMenu(FEMenuSystem* s);     // 0x593220
    virtual ~ProfileMainMenu();           // 0x5932C0
    static ProfileMainMenu* Me();         // 0x574EB0
    SaveGameData** GetSaveSlots();
    virtual void Init();                  // 0x5B7BF0
    void ClearEntries();                  // 0x574EC0
    virtual void Update(float time_inc);  // 0x574F00
    void CreateProfile();                 // 0x574F10
    virtual void ButtonHeldAction();      // 0x574F40
    static bool DialogResponseProfileLoadOk(int);  // 0x574F70
    static bool DialogResponseProfileEdit(int);    // 0x574F80
    static bool DialogResponseDeleteCancel(int);   // 0x574F90
    virtual void Draw();                  // 0x580730
    virtual void UpdateWidescreen(bool ws);  // 0x580760
    virtual void OnTriangle(int c);       // 0x5807D0
    virtual void OnUp(int c);             // 0x5B7C00
    virtual void OnDown(int c);           // 0x5B7C20
    virtual void OnActivate(int previous);// 0x586E20
    void LoadProfilesDone();              // 0x586E50
    virtual void SetPanelFile(PanelFile* pf);  // 0x597960
    virtual void Select(int entry_num);   // 0x597B10
    static bool DialogResponseDeleteConfirm(int);  // 0x597BB0
    void DialogDisplayProfileLoading(int delaySecs);  // 0x5933A0
    static void DialogDisplayProfileLoadSuccess(int); // 0x58ECC0
    static void DeleteDone(int);          // 0x580B00
    static bool DialogResponseDelete(int);// 0x59AE70
    void DialogDisplayProfileSelected();  // 0x59C560
    virtual void OnSquare(int c);         // 0x59D1A0
private:
    void OnSelectionChange();             // 0x5807F0
    void LoadSelectedProfile(bool displayDialog);  // 0x594300
};
static_assert(sizeof(ProfileMainMenu) == 0x8C,
              "ProfileMainMenu size mismatch");

// ============================================================================
// ProfileEditMenu - profile options menu (1324 bytes)
// Size: 0x52C - verified against IDA
// ============================================================================
class ProfileEditMenu : public FEMenu {
public:
    static const char* const kProfileTextGeoms[];           // 0xCEF788 (3)
    static const char* const kProfileTextOptionGeoms[];     // 0xCEF794 (5)
    static const char* const kProfileTextOptionStrings[];   // 0xCEF7A8 (5)
    static const char* const kProfileTextInstructionStrings[];  // 0xCEF7BC (5)

    bool mNeedWrite;              // +0x4C
    PanelFile* mPanel;            // +0x50
    FEText* mProfileEditText[3];  // +0x54
    FEMultiLineText* mHelpBar;    // +0x60
    FEMultiLineText* mInstructions;  // +0x64
    StubData mPreviousSettings;   // +0x68 (1216 bytes)
    bool mExitToMain;             // +0x528
    bool mSaveDialogDisplayed;    // +0x529

    ProfileEditMenu(FEMenuSystem* s);     // 0x5934A0
    virtual ~ProfileEditMenu();           // 0x593530
    static ProfileEditMenu* Me();         // 0x574FA0
    void NeedWrite();
    virtual void Update(float time_inc);  // 0x574FB0
    virtual void Init();                  // 0x5B7C40
    virtual void OnUp(int c);             // 0x574FC0
    virtual void OnDown(int c);           // 0x575020
    virtual void OnLeft(int c);           // 0x5B7C50
    virtual void OnRight(int c);          // 0x5B7C60
    virtual void Select(int entry_num);   // 0x575080
    virtual void ButtonHeldAction();      // 0x5750E0
    static void ExitMenu(int);            // 0x5751A0
    virtual void Draw();                  // 0x580B40
    virtual void UpdateWidescreen(bool ws);  // 0x580B60
    virtual void OnActivate(int previous);// 0x580BA0
    virtual void SetPanelFile(PanelFile* pf);  // 0x586FD0
    virtual void OnTriangle(int c);       // 0x59D720
private:
    void HighlightDefault(int previousMenu);  // 0x575110
    static bool DialogResponseCancel(int);// 0x580CD0
    static bool DialogResponseSave(int);  // 0x59C6B0
    void DialogDisplayConfirmSave();          // 0x59D200
};
static_assert(sizeof(ProfileEditMenu) == 0x52C,
              "ProfileEditMenu size mismatch");

// ============================================================================
// MemCardCheckMenu - memory-card presence check menu (88 bytes)
// Size: 0x58 - verified against IDA
// ============================================================================
class MemCardCheckMenu : public FEMenu {
public:
    int  mDialogDisplayed;  // +0x4C
    bool mActivated;        // +0x50
    int  mDelayTime;        // +0x54

    MemCardCheckMenu(FEMenuSystem* s);  // 0x5923F0
    virtual ~MemCardCheckMenu();        // 0x592430
    static MemCardCheckMenu* Me();      // 0x573480
    virtual void UpdateWidescreen(bool ws);  // 0x573490
    static bool DialogResponseContinue(int);  // 0x57F6A0
    virtual void Update(float time_inc);// 0x586730
    void DialogDisplayNotifyAutoSave(); // 0x58E4C0
    static bool DialogResponseNotifyAutoSave(int);  // 0x58E610
    void DialogDisplayNoMemCard();      // 0x592440
    void DialogDisplayNoFreeSpace();    // 0x5925D0
    static bool DialogResponseRetry(int);  // 0x592760
    void SetDialogDisplayed(int dialog);
private:
    void Activate();                    // 0x5866F0
};
static_assert(sizeof(MemCardCheckMenu) == 0x58,
              "MemCardCheckMenu size mismatch");

enum ELanguage {
    kLanguageEnglish = 0,
    kLanguageGerman = 1,
    kLanguageFrench = 2,
    kLanguageSpanish = 3,
    kLanguageItalian = 4,
    kLanguageUnlocalized = 5,
    kLanguageCount = 6,
};

enum errorParm_t {
    ERR_FATAL = 0,
    ERR_DROP = 1,
    ERR_SERVERDISCONNECT = 2,
    ERR_DISCONNECT = 3,
    ERR_NEED_CD = 4,
    ERR_ENDGAME = 5,
    ERR_SCRIPT = 6,
    ERR_LOCALIZATION = 7,
};

// ============================================================================
// CStringEdPackage - string table editor package (shell.o string_ed.cpp)
// ============================================================================
int SE_GetFlagMask(const char* psFlagName);

struct SE_Entry_s {
    std::string m_strString;  // +0x00
    std::string m_strDebug;   // +0x1C
    int         m_iFlags;     // +0x38

    SE_Entry_s();             // 0x5B6A10
    ~SE_Entry_s();            // 0x5B6A70
};

class CStringEdPackage {
    friend int ::SE_GetFlagMask(const char* psFlagName);
public:
    CStringEdPackage();
    ~CStringEdPackage();

    int         m_bEndMarkerFound_ParseOnly;        // +0x00
    int EndMarkerFoundDuringParse();
    std::string m_strCurrentEntryRef_ParseOnly;     // +0x04
    std::string m_strCurrentEntryEnglish_ParseOnly; // +0x20
    std::string m_strCurrentFileRef_ParseOnly;      // +0x3C
    std::string m_strLoadingLanguage_ParseOnly;     // +0x58
    int         m_bLoadingEnglish_ParseOnly;        // +0x74
    std::map<std::string, SE_Entry_s> m_StringEntries;  // +0x78
    int         m_bLoadDebug;                       // +0x84
    std::vector<std::string> m_vstrFlagNames;       // +0x88
    std::map<std::string, int> m_mapFlagMasks;      // +0x98

    const char* ExtractLanguageFromPath(const char* psFileName);  // QAE
    int ReadLine(const char*& psParsePos, char* psDest);          // QAE
    void SetupNewFileParse(const char* psFileName, int bLoadDebug);  // QAE
    int GetFlagMask(const char* psFlagName);                       // QAE
    void Clear(int bChangingLanguages);                            // QAE
    const char* ParseLine(const char* psLine);                     // QAE
private:
    char* Filename_PathOnly(const char* psFilename);
    char* Filename_WithoutExt(const char* psFilename);
    char* Filename_WithoutPath(const char* psFilename);
    int CheckLineForKeyword(const char* psKeyword, const char*& psLine);
    void REMKill(char* psBuffer);
    const char* GetCurrentReference_ParseOnly();
    const char* ConvertCRLiterals_Read(const char* psString);
    const char* InsideQuotes(const char* psLine);
    void SetString(const char* psLocalReference, const char* psNewString,
                   int bEnglishDebug);
    void AddEntry(const char* psLocalReference);
    void AddFlagReference(const char* psLocalReference,
                          const char* psFlagName);
};

// static buffers shared by the string-ed helpers (shell.o data)
extern char sString[128];    // 0x00F6A3C0-ish
extern char sString_0[128];
extern char sString_1[128];

// ============================================================================
// system_time - wall-clock time (12 bytes, 6x uint16) - verified against IDA
// ============================================================================
class system_time {
public:
    uint16_t year;    // +0x00
    uint16_t month;   // +0x02
    uint16_t day;     // +0x04
    uint16_t hour;    // +0x06
    uint16_t minute;  // +0x08
    uint16_t second;  // +0x0A

    system_time();

    bool equals(system_time st);
    bool newer_than(system_time st);
};
static_assert(sizeof(system_time) == 12, "system_time size mismatch");

// ============================================================================
// MPsharedStubData - shared-stub encode of StubData (93 bytes) - IDA verified
// ============================================================================
struct MPsharedStubData {
    char mCommand[6];              // +0x00
    char mProfileName[16];         // +0x06
    char mSaveGameSlot;            // +0x16
    char mLevelReached;            // +0x17
    char mNextLevel;               // +0x18
    char mLanguage;                // +0x19
    char mDifficulty;              // +0x1A
    char mGameComplete;            // +0x1B
    char mShowEnding;              // +0x1C
    char mSec;                     // +0x1D
    char mMin;                     // +0x1E
    char mHour;                    // +0x1F
    char mDay;                     // +0x20
    char mSubtitles;               // +0x21
    char mCrosshair;               // +0x22
    char mFriendlyTags;            // +0x23
    char mTankStyle;               // +0x24
    char mAdsToggle;               // +0x25
    char mInvertAim;               // +0x26
    char mVibration;               // +0x27
    char mStickyAim;               // +0x28
    char mHorizontalSensitivity;   // +0x29
    char mVerticalSensitivity;     // +0x2A
    char mControllerButtonConfiguration;  // +0x2B
    char mControllerStickConfiguration;   // +0x2C
    char mRatioIs4by3;             // +0x2D
    char mResolutionIs480p;        // +0x2E
    char mChannels;                // +0x2F
    char mVolume;                  // +0x30
    char mMusicVolume;             // +0x31
    char mEffectVolume;            // +0x32
    char mGameCompleted;           // +0x33
    char mViewedCredit;            // +0x34
    char mViewedSmg;               // +0x35
    char mViewedRifle;             // +0x36
    char mViewedHmg;               // +0x37
    char mViewedPistol;            // +0x38
    char mViewedGrenade;           // +0x39
    char mViewedAssault;           // +0x3A
    char mViewedSniper;            // +0x3B
    char mViewedCrewServed;        // +0x3C
    char mViewedAntiTank;          // +0x3D
    char mViewedDemolition;        // +0x3E
    char mViewedLand;              // +0x3F
    char mViewedAir;               // +0x40
    char mViewedSea;               // +0x41
    char mViewedArtillery;         // +0x42
    char mViewedAntiAir;           // +0x43
    char mViewedRocket;            // +0x44
    char mViewedCharacters;        // +0x45
    char mViewedArt[14];           // +0x46
    char mViewedMovies;            // +0x54
    char mMaxPlayerCntPreference;  // +0x55
    char mGameModePreference;      // +0x56
    char mMapPreference;           // +0x57
    char mAutoTeamBalancePreference;  // +0x58
    char mTeamDamagePreference;    // +0x59
    char mSaved;                   // +0x5A
    char mSaveId;                  // +0x5B
    char mControllerPort;          // +0x5C

    void set(StubData* stubData);
    void get(StubData* stubData) const;
};
static_assert(sizeof(MPsharedStubData) == 0x5D, "MPsharedStubData size mismatch");

controller::ButtonIndex mapButton(int button);
