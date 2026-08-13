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
#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

enum ELanguage {
    kLanguageEnglish = 0,
    kLanguageGerman = 1,
    kLanguageFrench = 2,
    kLanguageSpanish = 3,
    kLanguageItalian = 4,
    kLanguageJapanese = 5,
    kLanguageUnlocalized = 6,
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
};

class CStringEdPackage {
    friend int ::SE_GetFlagMask(const char* psFlagName);
public:
    int         m_bEndMarkerFound_ParseOnly;        // +0x00
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

// controller::ButtonIndex enum (full in input/controller.cpp)
class controller {
public:
    int locked_port;
    bool is_locked;
    static controller* inst();  // controller_xbox.o

    enum ButtonIndex {
        LEFTBUTTON = 0,
        DOWNBUTTON = 1,
        RIGHTBUTTON = 2,
        UPBUTTON = 3,
        SQUARE = 4,
        X = 5,
        CIRCLE = 6,
        TRIANGLE = 7,
        R1 = 8,
        L1 = 9,
        R2 = 10,
        L2 = 11,
        R3 = 12,
        L3 = 13,
        START = 14,
        SELECT = 15,
    };
};

controller::ButtonIndex mapButton(int button);
