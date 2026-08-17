// ============================================================================
// stub_data.cpp - StubData / MPsharedStubData / SaveGameData (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"
#include "ngl/ngl_dx_quad.h"

#include <string.h>

void j_nullsub_96() {}

extern cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                        int flags);  // core.o
extern SaveGameData* gSaveGameData;  // ?gSaveGameData@@3PAUSaveGameData@@A
extern ELanguage gLanguage;          // 0x012F03A4
extern int currCl;
extern float sNaN;                   // ?sNaN@@3MA @ 0x10F19D0
extern unsigned char unk_F6A294[4 * 3208]; // 0x00F6A294
extern float unk_F6A298[4 * 802];         // 0x00F6A298
extern float unk_F6A29C[4 * 802];         // 0x00F6A29C
extern void ApplyControllerStickConfig(int stickConfig);
extern void ApplyControllerButtonConfig(int buttonConfig);

struct MusicMgr {
public:
    static MusicMgr* sInst;  // ?sInst@MusicMgr@@2PAV1@A
    void ScaleVolume(float scale);  // ?ScaleVolume@MusicMgr@@QAEXM@Z (game.o)
};

enum ELiveState {
    kNotSignedIn = 0,
};
enum {
    UIX_LOGON_FORCE_DWORD = 0xFFFFFFFF,
};

// ea: 0x00563FE0
void StubData::Init()
{
    mSaveGameSlot = 0;
    mLevelReached = -1;
    mNextLevel = 0;
    mLanguage = 0;
    mDifficulty = 1;
    mGameComplete = false;
    mShowEnding = false;
    mSec = 0;
    mMin = 0;
    mHour = 0;
    mDay = 0;
    mSaved = false;
    mSaveId = 0;
    mSubtitles = gLanguage != kLanguageEnglish;
    mCrosshair = true;
    mFriendlyTags = true;
    mTankStyle = 1;
    mAdsToggle = false;
    mInvertAim = false;
    mVibration = true;
    mStickyAim = true;
    mHorizontalSensitivity = 24;
    mVerticalSensitivity = 32;
    int integer = Cvar_Get("cg_controllerconfig", "0", 1)->integer;
    mRatioIs4by3 = true;
    mControllerButtonConfiguration = integer;
    mControllerStickConfiguration = 0;
    mResolutionIs480p = false;
    mChannels = 1;
    mGameCompleted = false;
    mViewedCredit = false;
    mViewedSmg = 0;
    mViewedRifle = 0;
    mViewedHmg = 0;
    mViewedPistol = 0;
    mViewedGrenade = 0;
    mViewedAssault = 0;
    mViewedSniper = 0;
    mViewedCrewServed = 0;
    mViewedAntiTank = 0;
    mViewedDemolition = 0;
    mViewedLand = 0;
    mViewedAir = 0;
    mViewedSea = 0;
    mViewedArtillery = 0;
    mViewedAntiAir = 0;
    mViewedRocket = 0;
    mViewedCharacters = 0;
    mRatioIs4by3 = !nglDisplayMode.Widescreen;
    mVolume = 50;
    mMusicVolume = 50;
    mEffectVolume = 50;
    for (int i = 0; i < 14; ++i)
        mViewedArt[i] = 0;
    mMaxPlayerCntPreference = -1;
    mGameModePreference = -1;
    mMapPreference = -1;
    mAutoTeamBalancePreference = -1;
    mTeamDamagePreference = -1;
    mViewedMovies = 0;
    liveState = kNotSignedIn;
    loginMethod = UIX_LOGON_FORCE_DWORD;
    lastLoginCode = 0;
    mReturnToMain = false;
    savedStateIsValid = false;
    appearOnline = true;
    mbWasInvited = false;
    memset(mCmdLine, 0, sizeof(mCmdLine));
    int v1 = 0;
    int v5 = 0;
    for (StubData* i = &gSaveGameData[0].mStubData; this != i; i = (StubData*)((char*)i + 7156))
    {
        v1 += 7156;
        ++v5;
        if (v1 >= 28624)
            return;
    }
    mControllerPort = v5;
}

// ea: 0x005641E0
void StubData::LoadData(StubData* input)
{
    char* v2 = mProfileName;
    int v3 = (char*)input - (char*)this;
    for (int i = 16; i != 0; --i)
    {
        v2[0] = v2[v3];
        ++v2;
    }
    mSaveGameSlot = input->mSaveGameSlot;
    mLevelReached = input->mLevelReached;
    mNextLevel = input->mNextLevel;
    mLanguage = input->mLanguage;
    mDifficulty = input->mDifficulty;
    mGameComplete = input->mGameComplete;
    mShowEnding = input->mShowEnding;
    mSec = input->mSec;
    mMin = input->mMin;
    mHour = input->mHour;
    mDay = input->mDay;
    mSubtitles = input->mSubtitles;
    mCrosshair = input->mCrosshair;
    mFriendlyTags = input->mFriendlyTags;
    mTankStyle = input->mTankStyle;
    mAdsToggle = input->mAdsToggle;
    mInvertAim = input->mInvertAim;
    mVibration = input->mVibration;
    mHorizontalSensitivity = input->mHorizontalSensitivity;
    mVerticalSensitivity = input->mVerticalSensitivity;
    mControllerButtonConfiguration = input->mControllerButtonConfiguration;
    mControllerStickConfiguration = input->mControllerStickConfiguration;
    mStickyAim = input->mStickyAim;
    mRatioIs4by3 = input->mRatioIs4by3;
    mResolutionIs480p = input->mResolutionIs480p;
    mChannels = input->mChannels;
    mVolume = input->mVolume;
    mMusicVolume = input->mMusicVolume;
    mEffectVolume = input->mEffectVolume;
    mGameCompleted = input->mGameCompleted;
    mViewedCredit = input->mViewedCredit;
    mViewedSmg = input->mViewedSmg;
    mViewedRifle = input->mViewedRifle;
    mViewedHmg = input->mViewedHmg;
    mViewedPistol = input->mViewedPistol;
    mViewedGrenade = input->mViewedGrenade;
    mViewedAssault = input->mViewedAssault;
    mViewedSniper = input->mViewedSniper;
    mViewedCrewServed = input->mViewedCrewServed;
    mViewedAntiTank = input->mViewedAntiTank;
    mViewedDemolition = input->mViewedDemolition;
    mViewedLand = input->mViewedLand;
    mViewedAir = input->mViewedAir;
    mViewedSea = input->mViewedSea;
    mViewedArtillery = input->mViewedArtillery;
    mViewedAntiAir = input->mViewedAntiAir;
    mViewedRocket = input->mViewedRocket;
    mViewedCharacters = input->mViewedCharacters;
    mViewedMovies = input->mViewedMovies;
    int* mViewedArtDst = mViewedArt;
    for (int j = 14; j != 0; --j)
    {
        *mViewedArtDst = *(int*)((char*)mViewedArtDst + v3);
        ++mViewedArtDst;
    }
    mSaved = input->mSaved;
    mSaveId = input->mSaveId;
    mMaxPlayerCntPreference = input->mMaxPlayerCntPreference;
    mGameModePreference = input->mGameModePreference;
    mMapPreference = input->mMapPreference;
    mAutoTeamBalancePreference = input->mAutoTeamBalancePreference;
    mTeamDamagePreference = input->mTeamDamagePreference;
    mReturnToMain = input->mReturnToMain;
    mDisableSave = input->mDisableSave;
}

// ea: 0x00564400
void StubData::LoadDataLastMinFix(StubData* input)
{
    mSaveGameSlot = input->mSaveGameSlot;
    mLevelReached = input->mLevelReached;
    mNextLevel = input->mNextLevel;
    mLanguage = input->mLanguage;
    mDifficulty = input->mDifficulty;
    mGameComplete = input->mGameComplete;
    mShowEnding = input->mShowEnding;
    mSec = input->mSec;
    mMin = input->mMin;
    mHour = input->mHour;
    mDay = input->mDay;
    mSubtitles = input->mSubtitles;
    mCrosshair = input->mCrosshair;
    mFriendlyTags = input->mFriendlyTags;
    mTankStyle = input->mTankStyle;
    mAdsToggle = input->mAdsToggle;
    mInvertAim = input->mInvertAim;
    mVibration = input->mVibration;
    mHorizontalSensitivity = input->mHorizontalSensitivity;
    mVerticalSensitivity = input->mVerticalSensitivity;
    mControllerButtonConfiguration = input->mControllerButtonConfiguration;
    mControllerStickConfiguration = input->mControllerStickConfiguration;
    mStickyAim = input->mStickyAim;
    mRatioIs4by3 = input->mRatioIs4by3;
    mResolutionIs480p = input->mResolutionIs480p;
    mChannels = input->mChannels;
    mVolume = input->mVolume;
    mMusicVolume = input->mMusicVolume;
    mEffectVolume = input->mEffectVolume;
    mGameCompleted = input->mGameCompleted;
    mViewedCredit = input->mViewedCredit;
    mViewedSmg = input->mViewedSmg;
    mViewedRifle = input->mViewedRifle;
    mViewedHmg = input->mViewedHmg;
    mViewedPistol = input->mViewedPistol;
    mViewedGrenade = input->mViewedGrenade;
    mViewedAssault = input->mViewedAssault;
    mViewedSniper = input->mViewedSniper;
    mViewedCrewServed = input->mViewedCrewServed;
    mViewedAntiTank = input->mViewedAntiTank;
    mViewedDemolition = input->mViewedDemolition;
    mViewedLand = input->mViewedLand;
    mViewedAir = input->mViewedAir;
    mViewedSea = input->mViewedSea;
    mViewedArtillery = input->mViewedArtillery;
    mViewedAntiAir = input->mViewedAntiAir;
    mViewedRocket = input->mViewedRocket;
    mViewedCharacters = input->mViewedCharacters;
    mViewedMovies = input->mViewedMovies;
    int* mViewedArtDst = mViewedArt;
    for (int i = 14; i != 0; --i)
    {
        *mViewedArtDst = *(int*)((char*)mViewedArtDst + ((char*)input - (char*)this));
        ++mViewedArtDst;
    }
    mSaved = input->mSaved;
    mSaveId = input->mSaveId;
    mMaxPlayerCntPreference = input->mMaxPlayerCntPreference;
    mGameModePreference = input->mGameModePreference;
    mMapPreference = input->mMapPreference;
    mAutoTeamBalancePreference = input->mAutoTeamBalancePreference;
    mTeamDamagePreference = input->mTeamDamagePreference;
    mReturnToMain = input->mReturnToMain;
    mDisableSave = input->mDisableSave;
}

// ea: 0x00564610
void StubData::ApplyDifficulty()
{
}

// ea: 0x005770D0
void StubData::ApplyStubOptions()
{
    SoundDevice::sInst->SetOutputMode((SoundDevice::EOutputMode)mChannels);
    if (mControllerPort == 0
        || (controller::inst()->is_locked
            && controller::inst()->locked_port == mControllerPort))
    {
        currCl = 0;  // NS_CLIENT
        unk_F6A298[0] = (int)(mHorizontalSensitivity * 0.039999999f);
        unk_F6A29C[0] = (int)(mVerticalSensitivity * 0.039999999f);
        unk_F6A294[0] = mTankStyle != 1;
        MusicMgr::sInst->ScaleVolume(
            gSaveGameData[mControllerPort].mStubData.mVolume * 0.02f);
        SoundDevice::sInst->ScaleVolume(
            gSaveGameData[mControllerPort].mStubData.mVolume * 0.02f);
        ApplyControllerStickConfig(mControllerStickConfiguration);
        ApplyControllerButtonConfig(mControllerButtonConfiguration);
    }
    currCl = 0;  // NS_CLIENT
}

// ea: 0x00564620
void MPsharedStubData::set(StubData* stubData)
{
    strcpy(mCommand, "+stub");
    mCommand[5] = ' ';
    for (int i = 0; i < 16; ++i)
        mProfileName[i] = (char)(stubData->mProfileName[i] + 33);
    mSaveGameSlot = (char)(stubData->mSaveGameSlot + 33);
    mLevelReached = (char)(stubData->mLevelReached + 33);
    mNextLevel = (char)(stubData->mNextLevel + 33);
    mLanguage = (char)(stubData->mLanguage + 33);
    mDifficulty = (char)(stubData->mDifficulty + 33);
    mGameComplete = (char)(stubData->mGameComplete + 33);
    mShowEnding = (char)(stubData->mShowEnding + 33);
    mSec = (char)(stubData->mSec + 33);
    mMin = (char)(stubData->mMin + 33);
    mHour = (char)(stubData->mHour + 33);
    mDay = (char)(stubData->mDay + 33);
    mSubtitles = (char)(stubData->mSubtitles + 33);
    mCrosshair = (char)(stubData->mCrosshair + 33);
    mFriendlyTags = (char)(stubData->mFriendlyTags + 33);
    mTankStyle = (char)(stubData->mTankStyle + 33);
    mAdsToggle = (char)(stubData->mAdsToggle + 33);
    mInvertAim = (char)(stubData->mInvertAim + 33);
    mStickyAim = (char)(stubData->mStickyAim + 33);
    mVibration = (char)(stubData->mVibration + 33);
    mHorizontalSensitivity = (char)(stubData->mHorizontalSensitivity + 33);
    mVerticalSensitivity = (char)(stubData->mVerticalSensitivity + 33);
    mControllerButtonConfiguration =
        (char)(stubData->mControllerButtonConfiguration + 33);
    mControllerStickConfiguration =
        (char)(stubData->mControllerStickConfiguration + 33);
    mRatioIs4by3 = (char)(stubData->mRatioIs4by3 + 33);
    mResolutionIs480p = (char)(stubData->mResolutionIs480p + 33);
    mChannels = (char)(stubData->mChannels + 33);
    mVolume = (char)(stubData->mVolume + 33);
    mMusicVolume = (char)(stubData->mMusicVolume + 33);
    mEffectVolume = (char)(stubData->mEffectVolume + 33);
    mGameCompleted = (char)(stubData->mGameCompleted + 33);
    mViewedCredit = (char)(stubData->mViewedCredit + 33);
    mViewedSmg = (char)(stubData->mViewedSmg + 34);
    mViewedRifle = (char)(stubData->mViewedRifle + 34);
    mViewedHmg = (char)(stubData->mViewedHmg + 34);
    mViewedPistol = (char)(stubData->mViewedPistol + 34);
    mViewedGrenade = (char)(stubData->mViewedGrenade + 34);
    mViewedAssault = (char)(stubData->mViewedAssault + 34);
    mViewedSniper = (char)(stubData->mViewedSniper + 34);
    mViewedCrewServed = (char)(stubData->mViewedCrewServed + 34);
    mViewedAntiTank = (char)(stubData->mViewedAntiTank + 34);
    mViewedDemolition = (char)(stubData->mViewedDemolition + 34);
    mViewedLand = (char)(stubData->mViewedLand + 34);
    mViewedAir = (char)(stubData->mViewedAir + 34);
    mViewedSea = (char)(stubData->mViewedSea + 34);
    mViewedArtillery = (char)(stubData->mViewedArtillery + 34);
    mViewedAntiAir = (char)(stubData->mViewedAntiAir + 34);
    mViewedRocket = (char)(stubData->mViewedRocket + 34);
    mViewedCharacters = (char)(stubData->mViewedCharacters + 34);
    int v3 = 0;
    const int* mViewedArtSrc = stubData->mViewedArt;
    do
        mViewedArt[v3++] = (char)(*mViewedArtSrc++ + 34);
    while (v3 < 14);
    mViewedMovies = (char)(stubData->mViewedMovies + 34);
    mSaved = (char)(stubData->mSaved + 33);
    mSaveId = (char)(stubData->mSaveId + 33);
    mMaxPlayerCntPreference = (char)(stubData->mMaxPlayerCntPreference + 33);
    mGameModePreference = (char)(stubData->mGameModePreference + 33);
    mMapPreference = (char)(stubData->mMapPreference + 33);
    mAutoTeamBalancePreference =
        (char)(stubData->mAutoTeamBalancePreference + 33);
    mTeamDamagePreference = (char)(stubData->mTeamDamagePreference + 33);
    mControllerPort = (char)(stubData->mControllerPort + 34);
}

// ea: 0x005648C0
void MPsharedStubData::get(StubData* stubData) const
{
    for (int i = 0; i < 16; ++i)
        stubData->mProfileName[i] = (char)(mProfileName[i] - 33);
    stubData->mSaveGameSlot = mSaveGameSlot - 33;
    stubData->mLevelReached = mLevelReached - 33;
    stubData->mNextLevel = mNextLevel - 33;
    stubData->mLanguage = mLanguage - 33;
    stubData->mDifficulty = mDifficulty - 33;
    stubData->mGameComplete = mGameComplete != 33;
    stubData->mShowEnding = mShowEnding != 33;
    stubData->mSec = mSec - 33;
    stubData->mMin = mMin - 33;
    stubData->mHour = mHour - 33;
    stubData->mDay = mDay - 33;
    stubData->mSubtitles = mSubtitles != 33;
    stubData->mCrosshair = mCrosshair != 33;
    stubData->mFriendlyTags = mFriendlyTags != 33;
    stubData->mTankStyle = mTankStyle - 33;
    stubData->mAdsToggle = mAdsToggle != 33;
    stubData->mInvertAim = mInvertAim != 33;
    stubData->mStickyAim = mStickyAim != 33;
    stubData->mVibration = mVibration != 33;
    stubData->mHorizontalSensitivity = mHorizontalSensitivity - 33;
    stubData->mVerticalSensitivity = mVerticalSensitivity - 33;
    stubData->mControllerButtonConfiguration =
        mControllerButtonConfiguration - 33;
    stubData->mControllerStickConfiguration =
        mControllerStickConfiguration - 33;
    stubData->mRatioIs4by3 = mRatioIs4by3 != 33;
    stubData->mResolutionIs480p = mResolutionIs480p != 33;
    stubData->mChannels = mChannels - 33;
    stubData->mVolume = mVolume - 33;
    stubData->mMusicVolume = mMusicVolume - 33;
    stubData->mEffectVolume = mEffectVolume - 33;
    stubData->mGameCompleted = mGameCompleted != 33;
    stubData->mViewedCredit = mViewedCredit != 33;
    stubData->mViewedSmg = mViewedSmg - 34;
    stubData->mViewedRifle = mViewedRifle - 34;
    stubData->mViewedHmg = mViewedHmg - 34;
    stubData->mViewedPistol = mViewedPistol - 34;
    stubData->mViewedGrenade = mViewedGrenade - 34;
    stubData->mViewedAssault = mViewedAssault - 34;
    stubData->mViewedSniper = mViewedSniper - 34;
    stubData->mViewedCrewServed = mViewedCrewServed - 34;
    stubData->mViewedAntiTank = mViewedAntiTank - 34;
    stubData->mViewedDemolition = mViewedDemolition - 34;
    stubData->mViewedLand = mViewedLand - 34;
    stubData->mViewedAir = mViewedAir - 34;
    stubData->mViewedSea = mViewedSea - 34;
    stubData->mViewedArtillery = mViewedArtillery - 34;
    stubData->mViewedAntiAir = mViewedAntiAir - 34;
    stubData->mViewedRocket = mViewedRocket - 34;
    stubData->mViewedCharacters = mViewedCharacters - 34;
    int v3 = 0;
    const char* mViewedArtSrc = mViewedArt;
    int* mViewedArtDst = stubData->mViewedArt;
    do
        *mViewedArtDst++ = mViewedArtSrc[v3++] - 34;
    while (v3 < 14);
    stubData->mViewedMovies = mViewedMovies - 34;
    stubData->mSaved = mSaved != 33;
    stubData->mSaveId = mSaveId - 33;
    stubData->mMaxPlayerCntPreference = mMaxPlayerCntPreference - 33;
    stubData->mGameModePreference = mGameModePreference - 33;
    stubData->mMapPreference = mMapPreference - 33;
    stubData->mAutoTeamBalancePreference = mAutoTeamBalancePreference - 33;
    stubData->mTeamDamagePreference = mTeamDamagePreference - 33;
    stubData->mControllerPort = mControllerPort - 34;
}

// ea: 0x00564C70
void SaveGameData::Init()
{
    mCheckpointSaveExists = false;
    memset(ammo, 0, sizeof(ammo));
    memset(ammoclip, 0, sizeof(ammoclip));
    weapons[0] = 0;
    weapons[1] = 0;
    memset(weaponslots, 0, sizeof(weaponslots));
    weaponrechamber[0] = 0;
    weaponrechamber[1] = 0;
    mPlayerHealth = -1.0f;
    weapon = 0;
    mFriendlyCount = 0;
    mEvent[0] = 0;
    mCurrentMapName[0] = 0;
    mCheckpointName[0] = 0;
    mPlayerOrientation[0] = 0.0f;
    mPlayerOrientation[1] = 0.0f;
    mPlayerOrientation[2] = 0.0f;
    mPlayerPosition[0] = 0.0f;
    mPlayerPosition[1] = 0.0f;
    mPlayerPosition[2] = 0.0f;
    memset(m_title_prefix, 0, sizeof(m_title_prefix));
}

// ea: 0x00564D30
void SaveGameData::LoadData(SaveGameData* input)
{
    mStubData.LoadData(&input->mStubData);
    mCheckpointSaveExists = input->mCheckpointSaveExists;
    memcpy(ammo, input->ammo, sizeof(ammo));
    memcpy(ammoclip, input->ammoclip, sizeof(ammoclip));
    weaponrechamber[0] = input->weaponrechamber[0];
    weaponrechamber[1] = input->weaponrechamber[1];
    weapon = input->weapon;
    mPlayerHealth = input->mPlayerHealth;
    mPlayerOrientation[0] = input->mPlayerOrientation[0];
    mPlayerOrientation[1] = input->mPlayerOrientation[1];
    mPlayerOrientation[2] = input->mPlayerOrientation[2];
    mPlayerPosition[0] = input->mPlayerPosition[0];
    mPlayerPosition[1] = input->mPlayerPosition[1];
    mPlayerPosition[2] = input->mPlayerPosition[2];
    memcpy(mFriendlies, input->mFriendlies, sizeof(mFriendlies));
    char* mEventSrc = input->mEvent;
    char v4;
    do
    {
        v4 = *mEventSrc;
        mEventSrc[(char*)this - (char*)input] = *mEventSrc;
        ++mEventSrc;
    }
    while (v4 != 0);
    char* mMapSrc = input->mCurrentMapName;
    char v6;
    do
    {
        v6 = *mMapSrc;
        mMapSrc[(char*)this - (char*)input] = *mMapSrc;
        ++mMapSrc;
    }
    while (v6 != 0);
    char* mCpSrc = input->mCheckpointName;
    char v8;
    do
    {
        v8 = *mCpSrc;
        mCpSrc[(char*)this - (char*)input] = *mCpSrc;
        ++mCpSrc;
    }
    while (v8 != 0);
    mGameVarCount = input->mGameVarCount;
    memcpy(mGameVars, input->mGameVars, 0xC04u);
    memcpy(mCheckpointScriptExploded, input->mCheckpointScriptExploded,
           sizeof(mCheckpointScriptExploded));
    memcpy(m_version_number, input->m_version_number,
           sizeof(m_version_number));
    char* m_title = m_title_prefix;
    *m_title = *input->m_title_prefix;
    *(m_title + 1) = input->m_title_prefix[4];
    *(m_title + 2) = input->m_title_prefix[8];
}
