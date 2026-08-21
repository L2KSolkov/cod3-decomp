// ============================================================================
// profile.cpp - ProfileManager / ProfileMainMenu / ProfileEditMenu /
// MemCardCheckMenu (shell.o ProfileManager.cpp family)
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/platform_xbox/MemoryUnitManager.h"

#include <string.h>
#include <stdio.h>
#include <intrin.h>
#include <new>

extern FEManager g_femanager;          // ?g_femanager@@3UFEManager@@A
extern int currCl;                     // ?currCl@@3HA @ 0xF1579C
extern const char defaultFileName[];  // ?defaultFileName
extern void* mem_heap_malloc(int alignment, unsigned int size);  // core.o
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);  // core.o
extern SaveGameData gSaveGameData[4];    // ?gSaveGameData@@3PAUSaveGameData@@A
extern bool g_enableControllerTest;    // ?g_enableControllerTest@@3_NA
extern float Sys_Time();               // ?Sys_Time@@YAMXZ
extern int Sys_Milliseconds();         // ?Sys_Milliseconds@@YAHXZ
extern char* va(const char* fmt, ...); // core.o
extern void j_nullsub_96();            // ?nullsub_96

namespace LocalClient {
extern int ClientToPort(int client);  // ?ClientToPort@LocalClient@@YAHH@Z
}

bool ProfileManager::IsSaveEnabled() const { return mSaveEnabled; }
void ProfileManager::SetContinueWithoutSaving() { mContinueWithoutSavingId = mCardId; }
bool ProfileManager::IsContinueWithoutSaving() { return mCardId == mContinueWithoutSavingId; }
int ProfileManager::GetStatus() { return mCurrentStatus; }
bool ProfileManager::IsControllingDMS() { return mCallback != nullptr; }
void ProfileManager::SetStatus(int status) { mCurrentStatus = status; }
void ProfileManager::SetOperation(int op) { mCurrentOp = op; }
void ProfileManager::SetLastCardId() { mLastCardId = mCardId; }
bool ProfileManager::HasCardChanged() { return mLastCardId != mCardId; }
void ProfileManager::SetOverwriteOk() { mOverwriteOkCardId = mCardId; }
bool ProfileManager::IsOverwriteOk() { return mCardId == mOverwriteOkCardId; }
void ProfileManager::SetCreateOk() { mCreateOkCardId = mCardId; }

// STBManager minimal view (full class in core/core_systems.h)
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
};

// Minimal GameSettings view (full class in game_settings.cpp)
class GameSettings {
public:
    static GameSettings* sInst;       // ?sInst@GameSettings@@2PAV1@A
    bool m_damaged_save;              // +0x291
    bool does_file_exist();           // 0x575EB0
    bool enough_space();              // 0x575D00
    bool is_corrupt(SaveGameData* gd);// 0x575F50
    void get_insufficient_space_error(char* str, bool from_fe);  // 0x575E10
    void dashboard_reboot_to_free_blocks();  // 0x581010
    MemoryUnitManager::eStatus save();       // 0x587380
    MemoryUnitManager::eStatus del(int slot_num);  // 0x587520
    MemoryUnitManager::eStatus load_all(SaveGameData** const saves);  // 0x5877B0
};

// ============================================================================
// Data (shell.o rdata, verified VAs)
// ============================================================================
const char* const ProfileMainMenu::kProfileTextGeoms[] = {
    "text_title_main",
    "text_title_instructions",
    "text_current_selected",
    "text_profile_01",
    "text_profile_02",
    "text_profile_03",
    "text_profile_04",
    "text_profile_05",
    "text_profile_06",
};  // ?kProfileTextGeoms@ProfileMainMenu@@0QBQBDB @ 0xCEF72C

const char* const ProfileEditMenu::kProfileTextGeoms[] = {
    "text_title_main",
    "text_title_profile",
    "text_option_selected",
};  // ?kProfileTextGeoms@ProfileEditMenu@@0QBQBDB @ 0xCEF788

const char* const ProfileEditMenu::kProfileTextOptionGeoms[] = {
    "text_option_01",
    "text_option_02",
    "text_option_03",
    "text_option_04",
    "text_option_05",
};  // ?kProfileTextOptionGeoms@ProfileEditMenu@@0QBQBDB @ 0xCEF794

const char* const ProfileEditMenu::kProfileTextOptionStrings[] = {
    "FEMENU_OP_GAMEPLAY",
    "FEMENU_OP_CONTROLS",
    "FEMENU_OP_PREFERENCES",
    "FEMENU_OP_SOUND",
    "FEMENU_OP_SCREEN",
};  // ?kProfileTextOptionStrings@ProfileEditMenu@@0QBQBDB @ 0xCEF7A8

const char* const ProfileEditMenu::kProfileTextInstructionStrings[] = {
    "FEMENU_OP_INST_GAMEPLAY",
    "FEMENU_OP_INST_CONTROLS",
    "FEMENU_OP_INST_PREFERENCES",
    "FEMENU_OP_INST_SOUND",
    "FEMENU_OP_INST_SCREEN",
};  // ?kProfileTextInstructionStrings@ProfileEditMenu@@0QBQBDB @ 0xCEF7BC

// ============================================================================
// Shared dialog idioms (exact code shape from 0x587130 disasm)
// ============================================================================
static DialogMenu* ActiveDialogLayer(DialogMenuSystem* DMS)
{
    return (DialogMenu*)DMS->menus[DMS->GetActiveMenu() != 0];
}

static void SetDialogNullTriangle(DialogMenuSystem* DMS)
{
    ActiveDialogLayer(DMS)->triangleResponse =
        (void (*)(int))j_nullsub_96;
}

static void ReformActiveDialog(DialogMenuSystem* DMS)
{
    int v12 = DMS->GetActiveMenu();
    DialogMenu* Layer = (DialogMenu*)DMS->menus[v12 != 0];
    g_femanager.GetDMS(Layer->mClient)->mDisplay->Reformat();
}

static void HighlightActiveOption(DialogMenuSystem* DMS, int index)
{
    DMS->menus[-(DMS->GetActiveMenu() != 0) == -1]->highlighted =
        (int16_t)index;
    DialogMenuDisplay* mDisplay = DMS->mDisplay;
    if (mDisplay->mOptionCount > 0 && mDisplay->mOptionCount <= 2)
        mDisplay->mOptionSelected = index;
    mDisplay->SetDialogFlash(index);
}

// ============================================================================
// ProfileManager
// ============================================================================

// ea: 0x00580D30
ProfileManager::ProfileManager()
{
    mProfileSlots.m_size = 0;
    mCallback = nullptr;
    mNumProfiles = 0;
    mCurrentStatus = 0;
    mCurrentOp = -1;
    mSaveEnabled = true;
    mQuietSave = false;
    mOverwriting = false;
    mCardId = 1;
    mLastCardId = 0;
    mCreateOkCardId = 0;
    mOverwriteOkCardId = 0;
    mContinueWithoutSavingId = 0;
    mOperationState = kOperationNone;
    mCountdownFinishedTime = 0.0f;
    MemoryUnitManager::RegisterInsertRemoveObserver(this);
}

// ea: 0x005751E0
ProfileManager::~ProfileManager()
{
    MemoryUnitManager::RegisterInsertRemoveObserver(nullptr);
}

// ea: 0x00575270
void ProfileManager::Callback(MemoryUnitManager::eDeviceChange change,
                              int deviceID)
{
    (void)change;
    (void)deviceID;
}

// ea: 0x005751D0
ProfileManager* ProfileManager::Me()
{
    return g_femanager.mProfileManager;
}

// ea: 0x005751F0
void ProfileManager::OperationDone(bool success)
{
    mOperationState = (EOperationState)(!success + 1);
}

// ea: 0x00575210
void ProfileManager::Reset()
{
    if (mCurrentStatus == 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 765;
        AeAssert::gCurrentExpr = "GetStatus() != kStatusBusy";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Profile Manager illegal status reset"))
            __debugbreak();
        mCurrentStatus = 0;
    }
    else
    {
        mCurrentStatus = 0;
    }
}

// ea: 0x00575280
bool ProfileManager::ActivateMemDevice()
{
    MemoryUnitManager::SetActiveMemoryUnit(8);
    return true;
}

// ea: 0x00575290
bool ProfileManager::DeviceIsUnformatted()
{
    MemoryUnitManager::MemoryUnitInfo mui;
    MemoryUnitManager::eStatus MemoryUnitInfo =
        MemoryUnitManager::GetMemoryUnitInfo(&mui);
    return MemoryUnitInfo == MemoryUnitManager::eUnformatted
           || (MemoryUnitInfo == MemoryUnitManager::eSuccess
               && mui.formated == 0);
}

// ea: 0x005752D0
void ProfileManager::StartCountdown()
{
    mCountdownFinishedTime = Sys_Time() + 1.0f;
}

// ea: 0x005752F0
bool ProfileManager::IsCountdownDone() const
{
    return Sys_Time() > mCountdownFinishedTime;
}

// ea: 0x00575310
char* ProfileManager::TimeToString(StubData& data)
{
    int v1 = data.mHour + 24 * data.mDay;
    if (v1 > 999)
    {
        v1 = 999;
        data.mDay = 41;
        data.mHour = 15;
    }
    const char* v3 = defaultFileName;
    if (data.mSec <= 9)
        v3 = "0";
    const char* v5 = defaultFileName;
    if (data.mMin <= 9)
        v5 = "0";
    const char* v6 = defaultFileName;
    if (v1 <= 9)
        v6 = "0";
    return va("%s%i:%s%i:%s%i", v6, v1, v5, data.mMin, v3, data.mSec);
}

// ea: 0x00575390
void ProfileManager::SplitTime(int time, int& sec, int& min, int& hour,
                               int& day)
{
    if (time > 0x57E3F)
        time = 0x57E3F;
    day = (time - time % 86400) / 86400;
    int v6 = time % 86400;
    int v7 = v6 % 3600;
    hour = (v6 - v7) / 3600;
    min = (v7 - v7 % 60) / 60;
    sec = v7 % 60;
}

// ea: 0x00575420
const char* ProfileManager::GetLoadedProfile() const
{
    return gSaveGameData[0].mStubData.mProfileName[0] != 0
               ? gSaveGameData[0].mStubData.mProfileName
               : nullptr;
}

// ea: 0x00575430
void ProfileManager::IssueCallback()
{
    g_femanager.mProfileManager->Reset();
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->CloseDialog();
    if (mCallback != nullptr)
    {
        mCallback = nullptr;
        mCallback(0);
    }
}

// Shared response helper (mCallback fire + reset + close dialog)
static void FinishCallback()
{
    ProfileManager* mProfileManager = g_femanager.mProfileManager;
    g_femanager.mProfileManager->Reset();
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->CloseDialog();
    if (mProfileManager->mCallback != nullptr)
    {
        mProfileManager->mCallback = nullptr;
        mProfileManager->mCallback(0);
    }
}

// ea: 0x00575470
bool ProfileManager::DialogResponseOperationDone(int client)
{
    (void)client;
    FinishCallback();
    return g_femanager.mProfileManager->mCallback == nullptr;
}

// ea: 0x005754C0
bool ProfileManager::DialogResponseNoData(int client)
{
    (void)client;
    FinishCallback();
    return g_femanager.mProfileManager->mCallback == nullptr;
}

// ea: 0x00575510
bool ProfileManager::DialogResponseContinueNoSave(int client)
{
    (void)client;
    g_femanager.mProfileManager->mSaveEnabled = false;
    g_femanager.mProfileManager->mContinueWithoutSavingId =
        g_femanager.mProfileManager->mCardId;
    FinishCallback();
    return g_femanager.mProfileManager->mCallback == nullptr;
}

// ea: 0x00575570
bool ProfileManager::DialogResponseContinue(int client)
{
    (void)client;
    FinishCallback();
    return g_femanager.mProfileManager->mCallback == nullptr;
}

// ea: 0x00580D90
bool ProfileManager::HasMemCard()
{
    MemoryUnitManager::SetActiveMemoryUnit(8);
    mSaveEnabled = true;
    return true;
}

// ea: 0x00580DB0
bool ProfileManager::IsProfileNameUsed(const char* name) const
{
    for (unsigned int v3 = 0; v3 < (unsigned int)mNumProfiles; ++v3)
    {
        if (_stricmp(mProfileSlots[v3].profileName, name) == 0)
            return true;
    }
    return false;
}

// ea: 0x00580E40
int ProfileManager::GetProfiles(Profile** slots)
{
    for (int i = 0; i < mNumProfiles; ++i)
        slots[i] = &mProfileSlots[i];
    return mNumProfiles;
}

// ea: 0x00580EC0
bool ProfileManager::FileExists()
{
    return GameSettings::sInst->does_file_exist();
}

// ea: 0x00580ED0
bool ProfileManager::EnoughFreeSpace()
{
    if (GameSettings::sInst->does_file_exist())
        return true;
    return GameSettings::sInst->enough_space();
}

// ea: 0x00580F00
void ProfileManager::SetProfile(SaveGameData* sv)
{
    if (sv != nullptr)
    {
        gSaveGameData[0].LoadData(sv);
        gSaveGameData[0].mStubData.mSaved = true;
        gSaveGameData[0].mStubData.ApplyStubOptions();
    }
    else
    {
        gSaveGameData[0].Init();
        gSaveGameData[0].mStubData.Init();
    }
}

// ea: 0x0058EE10
void ProfileManager::Format()
{
    if (mCallback == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 424;
        AeAssert::gCurrentExpr = "mCallback";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Should have been called from internal menu system"))
            __debugbreak();
    }
    DialogDisplayFormatting();
    mCurrentStatus = 1;
    mCountdownFinishedTime = Sys_Time() + 1.0f;
    mCurrentOp = 5;
    mLastCardId = mCardId;
}

// ea: 0x005935B0
void ProfileManager::EnumProfiles(SaveGameData** slots)
{
    if (mCurrentStatus != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 87;
        AeAssert::gCurrentExpr = "GetStatus() == kStatusIdle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Profile Manager unable to service EnumProfiles request. "
                "Current status: %d",
                mCurrentStatus))
            __debugbreak();
    }
    mCurrentOp = 1;
    mEnumProfileSlots = slots;
    mNumProfiles = 0;
    MemoryUnitManager::SetActiveMemoryUnit(8);
    MemoryUnitManager::MemoryUnitInfo storageInfo;
    MemoryUnitManager::eStatus MemoryUnitInfo =
        MemoryUnitManager::GetMemoryUnitInfo(&storageInfo);
    if (MemoryUnitInfo != MemoryUnitManager::eUnformatted
        && (MemoryUnitInfo != MemoryUnitManager::eSuccess
            || storageInfo.formated != 0))
    {
        if (GameSettings::sInst->does_file_exist()
            || GameSettings::sInst->enough_space())
        {
            if (GameSettings::sInst->does_file_exist())
            {
                mCurrentStatus = 1;
                mCountdownFinishedTime = Sys_Time() + 1.0f;
                GameSettings::sInst->load_all(slots);
            }
            else
            {
                mCurrentStatus = -3;
                if (mCallback != nullptr)
                    IssueCallback();
            }
        }
        else
        {
            mCurrentStatus = -2;
            if (mCallback != nullptr)
                DialogDisplayNoFreeSpace();
        }
    }
    else
    {
        mCurrentStatus = -5;
        if (mCallback != nullptr)
            DialogDisplayNoData();
    }
}

// ea: 0x005936F0
void ProfileManager::DeleteProfile(int slotNum)
{
    for (unsigned int v3 = 0; v3 < (unsigned int)mNumProfiles; ++v3)
    {
        if (mProfileSlots[v3].profileSlot == slotNum)
        {
            unsigned int v4 = mNumProfiles - 1;
            mProfileSlots[v3].profileName =
                mProfileSlots[v4].profileName;
            mProfileSlots[v3].profileSlot =
                mProfileSlots[v4].profileSlot;
            --mNumProfiles;
            break;
        }
    }
    if (mLastCardId == mCardId)
    {
        GameSettings::sInst->del(slotNum);
        mCountdownFinishedTime = Sys_Time() + 1.0f;
        mCurrentStatus = 1;
        mCurrentOp = 2;
        mLastCardId = mCardId;
    }
    else
    {
        DialogDisplayDeleteFailed();
    }
}

// ea: 0x00593860
void ProfileManager::DeleteCorruptSave()
{
    if (mCallback == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 408;
        AeAssert::gCurrentExpr = "mCallback";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Should have been called from internal menu system"))
            __debugbreak();
    }
    DialogDisplayDeleting();
    mCurrentStatus = 1;
    mCountdownFinishedTime = Sys_Time() + 1.0f;
    mCurrentOp = 3;
    mLastCardId = mCardId;
    if (MemoryUnitManager::DeleteGame("Profiles")
        != MemoryUnitManager::eSuccess)
        g_femanager.mProfileManager->mOperationState = kOperationFailure;
    GameSettings::sInst->m_damaged_save = false;
}

// ea: 0x00593910
bool ProfileManager::DialogResponseDeleteCorruptSave(int client)
{
    (void)client;
    if (g_femanager.mProfileManager->mLastCardId
        == g_femanager.mProfileManager->mCardId)
        g_femanager.mProfileManager->DeleteCorruptSave();
    else
        g_femanager.mProfileManager->DialogDisplayDeleteFailed();
    return false;
}

// ea: 0x00593930
bool ProfileManager::DialogResponseConfirmFormat(int client)
{
    (void)client;
    if (g_femanager.mProfileManager->mLastCardId
        == g_femanager.mProfileManager->mCardId)
    {
        DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
        SetDialogNullTriangle(DMS);
        ReformActiveDialog(DMS);
    }
    else
    {
        g_femanager.mProfileManager->DialogDisplayFormatFailed();
    }
    return false;
}

// ea: 0x005939E0
bool ProfileManager::DialogResponseFormat(int client)
{
    (void)client;
    if (g_femanager.mProfileManager->mLastCardId
        == g_femanager.mProfileManager->mCardId)
        g_femanager.mProfileManager->Format();
    else
        g_femanager.mProfileManager->DialogDisplayFormatFailed();
    return false;
}

// ea: 0x00594370
void ProfileManager::EnumProfiles(SaveGameData** slots,
                                  void (*callback)(int))
{
    if (mCallback != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 73;
        AeAssert::gCurrentExpr = "!mCallback";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Another request is in progress!"))
            __debugbreak();
    }
    mLastCardId = mCardId;
    mCallback = callback;
    DialogDisplayLoading();
    EnumProfiles(slots);
}

// ea: 0x005943F0
void ProfileManager::DeleteProfile(int slotNum, void (*callback)(int))
{
    if (mCallback != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 365;
        AeAssert::gCurrentExpr = "!mCallback";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Another request is in progress!"))
            __debugbreak();
    }
    mLastCardId = mCardId;
    mCallback = callback;
    DialogDisplayDeleting();
    DeleteProfile(slotNum);
}

// ea: 0x00597C40
void ProfileManager::EnumProfilesDone(bool success)
{
    g_enableControllerTest = true;
    if (success)
    {
        int v3 = 0;
        for (;;)
        {
            SaveGameData* v4 = mEnumProfileSlots[v3];
            if (GameSettings::sInst->is_corrupt(v4))
                break;
            if (v3 == v4->mStubData.mSaveGameSlot
                && v4->mStubData.mProfileName[0] != 0)
            {
                Profile* v5 = &mProfileSlots[mNumProfiles];
                v5->profileName = v4->mStubData.mProfileName;
                v5->profileSlot = v3;
                ++mNumProfiles;
                g_femanager.saveTime = ServerTime::sInst.mElapsedTime;
            }
            if (++v3 >= 6)
            {
                mOverwriteOkCardId = mCardId;
                mCurrentStatus = 2;
                if (g_femanager.mProfileManager->mCurrentStatus == 1)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\ProfileManager.cpp";
                    AeAssert::gCurrentLine = 765;
                    AeAssert::gCurrentExpr =
                        "GetStatus() != kStatusBusy";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(
                            "Profile Manager illegal status reset"))
                        __debugbreak();
                }
                g_femanager.mProfileManager->mCurrentStatus = 0;
                DialogMenuSystem* v8 = g_femanager.mDMS[currCl];
                if (v8->GetActiveMenu() == 1 && (v8->flags & 2) != 0)
                {
                    v8->MakeActive(0);
                    v8->flags &= ~2;
                }
                else
                {
                    if (!v8->mWasIGMSUpWhenLaunched)
                        g_femanager.GetIGMS(currCl)->MakeActive(-1);
                    v8->mDisplay->mIsClosing = true;
                }
                if (mCallback != nullptr)
                {
                    mCallback = nullptr;
                    mCallback(0);
                }
                return;
            }
        }
        mCurrentStatus = -4;
        if (mCallback != nullptr)
        {
            DialogDisplayCorruptData();
            Reset();
        }
    }
    else
    {
        mCurrentStatus = 2;
        MemoryUnitManager::MemoryUnitInfo storageInfo;
        if (GameSettings::sInst->m_damaged_save
            || MemoryUnitManager::GetMemoryUnitInfo(&storageInfo)
                   == MemoryUnitManager::eDamagedMedium)
            DialogDisplayCorruptData();
        else
            DialogDisplayLoadFailed();
    }
}

// ea: 0x00597E30
void ProfileManager::CreateProfile(int slotNum)
{
    mCurrentOp = 0;
    if (mContinueWithoutSavingId == mCardId)
    {
        g_femanager.mProfileManager->Reset();
        DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
        DMS->CloseDialog();
        if (mCallback != nullptr)
        {
            mCallback = nullptr;
            mCallback(0);
        }
    }
    else
    {
        MemoryUnitManager::SetActiveMemoryUnit(8);
        MemoryUnitManager::MemoryUnitInfo storageInfo;
        MemoryUnitManager::eStatus MemoryUnitInfo =
            MemoryUnitManager::GetMemoryUnitInfo(&storageInfo);
        if (MemoryUnitInfo != MemoryUnitManager::eUnformatted
            && (MemoryUnitInfo != MemoryUnitManager::eSuccess
                || storageInfo.formated != 0))
        {
            if (GameSettings::sInst->does_file_exist()
                || GameSettings::sInst->enough_space())
            {
                if (mOverwriteOkCardId == mCardId
                    || !GameSettings::sInst->does_file_exist())
                {
                    gSaveGameData[0].Init();
                    StubData newData;
                    gSaveGameData[0].Init();
                    gSaveGameData[0].mStubData.LoadDataLastMinFix(&newData);
                    gSaveGameData[0].mStubData.ApplyStubOptions();
                    int v13 = (int)(ServerTime::sInst.mElapsedTime
                                    - g_femanager.saveTime);
                    gSaveGameData[0].mStubData.mSaveGameSlot = slotNum;
                    g_femanager.saveTime =
                        ServerTime::sInst.mElapsedTime
                        - g_femanager.saveTime;
                    SplitTime(v13, gSaveGameData[0].mStubData.mSec,
                              gSaveGameData[0].mStubData.mMin,
                              gSaveGameData[0].mStubData.mHour,
                              gSaveGameData[0].mStubData.mDay);
                    mCurrentStatus = 1;
                    mCountdownFinishedTime = Sys_Time() + 1.0f;
                    GameSettings::sInst->save();
                }
                else
                {
                    mCurrentStatus = -6;
                    if (mCallback != nullptr)
                        DialogDisplayOverwrite();
                }
            }
            else
            {
                mCurrentStatus = -2;
                if (mCallback != nullptr)
                    DialogDisplayNoFreeSpace();
            }
        }
        else
        {
            mCurrentStatus = -5;
            if (mCallback != nullptr)
                DialogDisplayFormat();
        }
    }
}

// ea: 0x00597FF0
void ProfileManager::SaveProfile()
{
    mCurrentOp = 4;
    if (mContinueWithoutSavingId == mCardId)
    {
        g_femanager.mProfileManager->Reset();
        DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
        DMS->CloseDialog();
        if (mCallback != nullptr)
        {
            mCallback = nullptr;
            mCallback(0);
        }
    }
    else
    {
        MemoryUnitManager::SetActiveMemoryUnit(8);
        MemoryUnitManager::MemoryUnitInfo storageInfo;
        MemoryUnitManager::eStatus MemoryUnitInfo =
            MemoryUnitManager::GetMemoryUnitInfo(&storageInfo);
        if (MemoryUnitInfo != MemoryUnitManager::eUnformatted
            && (MemoryUnitInfo != MemoryUnitManager::eSuccess
                || storageInfo.formated != 0))
        {
            if (GameSettings::sInst->does_file_exist()
                || GameSettings::sInst->enough_space())
            {
                if (mOverwriteOkCardId == mCardId
                    || !GameSettings::sInst->does_file_exist())
                {
                    GameSettings::sInst->save();
                    mCurrentStatus = 1;
                    mCountdownFinishedTime = Sys_Time() + 1.0f;
                    mLastCardId = mCardId;
                }
                else
                {
                    mCurrentStatus = -6;
                    if (mCallback != nullptr)
                        DialogDisplayOverwrite();
                }
            }
            else
            {
                mCurrentStatus = -2;
                if (mCallback != nullptr)
                    DialogDisplayNoFreeSpace();
            }
        }
        else
        {
            mCurrentStatus = -5;
            if (mCallback != nullptr)
                DialogDisplayFormat();
        }
    }
}

// ea: 0x0059AFC0
void ProfileManager::CreateProfile(int slotNum, void (*callback)(int))
{
    if (mCallback != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 240;
        AeAssert::gCurrentExpr = "!mCallback";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Another request is in progress!"))
            __debugbreak();
    }
    mLastCardId = mCardId;
    mCallback = callback;
    DialogDisplaySaving();
    CreateProfile(slotNum);
}

// ea: 0x0059B040
void ProfileManager::Retry()
{
    if (mCallback == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 440;
        AeAssert::gCurrentExpr = "mCallback";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("No request in progress!"))
            __debugbreak();
    }
    Reset();
    mLastCardId = mCardId;
    switch (mCurrentOp)
    {
    case 0:
        DialogDisplaySaving();
        CreateProfile(gSaveGameData[0].mStubData.mSaveGameSlot);
        break;
    case 1:
        DialogDisplayLoading();
        EnumProfiles(mEnumProfileSlots);
        break;
    case 4:
        DialogDisplaySaving();
        SaveProfile();
        break;
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 464;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("unhandled current op (%d)", mCurrentOp))
            __debugbreak();
        break;
    }
}

// ea: 0x0059B130
void ProfileManager::SaveProfile(void (*callback)(int), bool quiet)
{
    if (mCallback != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileManager.cpp";
        AeAssert::gCurrentLine = 475;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                "Can't SaveProfile- another operation is already in "
                "progress!"))
            __debugbreak();
    }
    else
    {
        mCallback = callback;
        mQuietSave = quiet;
        mLastCardId = mCardId;
        if (!quiet)
            DialogDisplaySaving();
        SaveProfile();
    }
}

// ea: 0x0059B1B0
bool ProfileManager::DialogResponseRetry(int client)
{
    (void)client;
    g_femanager.mProfileManager->Retry();
    return false;
}

// ea: 0x0059B1C0
bool ProfileManager::DialogResponseConfirmCreateSave(int client)
{
    (void)client;
    if (g_femanager.mProfileManager->mLastCardId
        == g_femanager.mProfileManager->mCardId)
    {
        g_femanager.mProfileManager->mCreateOkCardId =
            g_femanager.mProfileManager->mCardId;
        g_femanager.mProfileManager->Retry();
    }
    else
    {
        g_femanager.mProfileManager->DialogDisplaySaveFailed();
    }
    return false;
}

// ea: 0x0059B1F0
bool ProfileManager::DialogResponseOverwrite(int client)
{
    (void)client;
    if (g_femanager.mProfileManager->mLastCardId
        == g_femanager.mProfileManager->mCardId)
    {
        g_femanager.mProfileManager->mOverwriteOkCardId =
            g_femanager.mProfileManager->mCardId;
        g_femanager.mProfileManager->Retry();
    }
    else
    {
        g_femanager.mProfileManager->DialogDisplaySaveFailed();
    }
    return false;
}

// ea: 0x0059D390
void ProfileManager::FinishOperation()
{
    int mCurrentOp = this->mCurrentOp;
    bool v3 = mOperationState == kOperationSuccess;
    if (mCurrentOp == 1)
    {
        EnumProfilesDone(v3);
    }
    else if (v3)
    {
        switch (mCurrentOp)
        {
        case 0:
            mCurrentStatus = 6;
            if (mCallback != nullptr)
                DialogDisplaySaveDone();
            break;
        case 2:
            mCurrentStatus = 7;
            if (mCallback != nullptr)
                DialogDisplayDeleteDone();
            break;
        case 3:
            mCurrentStatus = 8;
            if (mCallback != nullptr)
                DialogDisplayDeleteDone();
            break;
        case 4:
            mSaveEnabled = true;
            mCurrentStatus = 5;
            if (mCallback != nullptr)
            {
                if (mQuietSave)
                    IssueCallback();
                else
                    DialogDisplaySaveDone();
            }
            break;
        case 5:
            mCurrentStatus = 9;
            if (mCallback != nullptr)
            {
                DialogDisplayFormatSuccess();
                mCurrentOp = 4;
            }
            break;
        default:
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\ProfileManager.cpp";
            AeAssert::gCurrentLine = 670;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("unhandled current op (%d)",
                                     this->mCurrentOp))
                __debugbreak();
            break;
        }
    }
    else
    {
        switch (mCurrentOp)
        {
        case 0:
            mCurrentStatus = 6;
            if (mCallback != nullptr)
                DialogDisplaySaveFailed();
            break;
        case 2:
            mCurrentStatus = 7;
            if (mCallback != nullptr)
                DialogDisplayDeleteFailed();
            break;
        case 3:
            mCurrentStatus = 8;
            if (mCallback != nullptr)
                DialogDisplayDeleteFailed();
            break;
        case 4:
            mCurrentStatus = 5;
            if (mCallback != nullptr)
            {
                if (mQuietSave)
                {
                    mSaveEnabled = false;
                    IssueCallback();
                }
                else
                {
                    DialogDisplaySaveFailed();
                }
            }
            break;
        case 5:
            mCurrentStatus = 9;
            if (mCallback != nullptr)
                DialogDisplayFormatFailed();
            break;
        default:
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\ProfileManager.cpp";
            AeAssert::gCurrentLine = 729;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("unhandled current op (%d)",
                                     this->mCurrentOp))
                __debugbreak();
            break;
        }
    }
}

// ea: 0x0059D5C0
void ProfileManager::Update()
{
    MemoryUnitManager::Service();
    if (mOperationState != kOperationNone
        && Sys_Time() > mCountdownFinishedTime)
    {
        FinishOperation();
        mOperationState = kOperationNone;
    }
}

// ea: 0x00587130
void ProfileManager::DialogDisplayFormatting()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    SetDialogNullTriangle(DMS);
    ReformActiveDialog(DMS);
}

// ea: 0x005871C0
void ProfileManager::DialogDisplayNoMemDevice()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    SetDialogNullTriangle(DMS);
    ReformActiveDialog(DMS);
}

// ea: 0x00587250
bool ProfileManager::DialogResponseFreeMoreBlocks(int client)
{
    (void)client;
    GameSettings::sInst->dashboard_reboot_to_free_blocks();
    return true;
}

// ea: 0x00587260
void ProfileManager::DialogDisplayAutoSave()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(DMS)->AddOption(
        "MEM_DIALOG_OK", ProfileManager::DialogResponseContinue);
    HighlightActiveOption(DMS, 0);
    SetDialogNullTriangle(DMS);
    ReformActiveDialog(DMS);
}

// ea: 0x0058EE90
void ProfileManager::DialogDisplayLoading()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_CHECK_WARNING", false, false,
                 defaultFileName, true);
    SetDialogNullTriangle(DMS);
    ReformActiveDialog(DMS);
}

// ea: 0x0058EF40
void ProfileManager::DialogDisplayDeleting()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_DELETE_WARNING", false, false,
                 defaultFileName, true);
    SetDialogNullTriangle(DMS);
    ReformActiveDialog(DMS);
}

// ea: 0x0058EFF0
void ProfileManager::DialogDisplaySaving()
{
    mOverwriting = false;
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_SAVING", false, false, defaultFileName, true);
    SetDialogNullTriangle(DMS);
    ReformActiveDialog(DMS);
}

// ea: 0x0058F0B0
void ProfileManager::DialogDisplayOverwriting()
{
    mOverwriting = true;
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_OVERWRITE_WARNING", false, false,
                 defaultFileName, true);
    SetDialogNullTriangle(DMS);
    ReformActiveDialog(DMS);
}

// ea: 0x0058F170
void ProfileManager::DialogDisplaySaveDone()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp(mOverwriting ? "MEM_OVERWRITE_SUCCESS"
                              : "MEM_SAVE_SUCCESS",
                 false, false, defaultFileName, true);
    DialogMenuSystem* v3 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v3)->AddOption(
        "MEM_DIALOG_OK", ProfileManager::DialogResponseOperationDone);
    HighlightActiveOption(v3, 0);
    SetDialogNullTriangle(v3);
    ReformActiveDialog(v3);
}

// ea: 0x0058F2D0
void ProfileManager::DialogDisplayDeleteDone()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_DELETE_SUCCESSFUL_XBOX", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_DIALOG_OK", ProfileManager::DialogResponseOperationDone);
    HighlightActiveOption(v2, 0);
    SetDialogNullTriangle(v2);
    ReformActiveDialog(v2);
}

// ea: 0x0058F420
void ProfileManager::DialogDisplayNoData()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_ERROR_NO_SAVE_DATA", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_DIALOG_OK", ProfileManager::DialogResponseNoData);
    HighlightActiveOption(v2, 0);
    SetDialogNullTriangle(v2);
    ReformActiveDialog(v2);
}

// ea: 0x0058F570
void ProfileManager::DialogDisplayOverwrite()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_CARD_CHANGED_WARNING", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_DIALOG_YES", ProfileManager::DialogResponseOverwrite);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v4)->AddOption(
        "MEM_DIALOG_NO", ProfileManager::DialogResponseContinueNoSave);
    HighlightActiveOption(v4, 1);
    SetDialogNullTriangle(v4);
    ReformActiveDialog(v4);
}

// ea: 0x0058F700
void ProfileManager::DialogDisplayNoFreeSpace()
{
    char msg[1024];
    GameSettings::sInst->get_insufficient_space_error(msg, true);
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp(msg, false, false, defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_CONTINUE_WO_SAVING",
        ProfileManager::DialogResponseContinueNoSave);
    DialogMenuSystem* v5 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v5)->AddOption(
        "MEM_XBOX_MANAGE_MEM_UNIT",
        ProfileManager::DialogResponseFreeMoreBlocks);
    HighlightActiveOption(v5, 0);
    SetDialogNullTriangle(v5);
    ReformActiveDialog(v5);
}

// ea: 0x0058F8A0
void ProfileManager::DialogDisplayLoadFailed()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_LOAD_FAILED", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_CONTINUE_WO_SAVING",
        ProfileManager::DialogResponseContinueNoSave);
    HighlightActiveOption(v2, 0);
    SetDialogNullTriangle(v2);
    ReformActiveDialog(v2);
}

// ea: 0x0058F9F0
void ProfileManager::DialogDisplaySaveFailed()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp(mOverwriting ? "MEM_XBOX_OVERWRITE_FAILED"
                              : "MEM_XBOX_SAVE_FAILED",
                 false, false, defaultFileName, true);
    DialogMenuSystem* v3 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v3)->AddOption(
        "MEM_CONTINUE_WO_SAVING",
        ProfileManager::DialogResponseContinueNoSave);
    HighlightActiveOption(v3, 0);
    SetDialogNullTriangle(v3);
    ReformActiveDialog(v3);
}

// ea: 0x0058FB50
void ProfileManager::DialogDisplayDeleteFailed()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_DELETE_FAILED", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_DIALOG_OK", ProfileManager::DialogResponseContinue);
    HighlightActiveOption(v2, 0);
    SetDialogNullTriangle(v2);
    ReformActiveDialog(v2);
}

// ea: 0x0058FCA0
void ProfileManager::DialogDisplayFormatFailed()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_FORMAT_FAILED", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_CONTINUE_WO_SAVING",
        ProfileManager::DialogResponseContinueNoSave);
    HighlightActiveOption(v2, 0);
    SetDialogNullTriangle(v2);
    ReformActiveDialog(v2);
}

// ea: 0x00594470
void ProfileManager::DialogDisplayCorruptData()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_ERROR_CORRUPTED_FILE", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_DIALOG_YES",
        ProfileManager::DialogResponseDeleteCorruptSave);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v4)->AddOption(
        "MEM_DIALOG_NO", ProfileManager::DialogResponseContinue);
    HighlightActiveOption(v4, 1);
    SetDialogNullTriangle(v4);
    ReformActiveDialog(v4);
}

// ea: 0x00594600
void ProfileManager::DialogDisplayFormat()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_ERROR_UNFORMATTED", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_CONTINUE_WO_SAVING",
        ProfileManager::DialogResponseContinueNoSave);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v4)->AddOption(
        "MEM_FORMAT", ProfileManager::DialogResponseConfirmFormat);
    HighlightActiveOption(v4, 0);
    SetDialogNullTriangle(v4);
    ReformActiveDialog(v4);
}

// ea: 0x0059C720
void ProfileManager::DialogDisplayFormatSuccess()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_FORMAT_SUCCESS", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_DIALOG_OK",
        ProfileManager::DialogResponseConfirmCreateSave);
    HighlightActiveOption(v2, 0);
    SetDialogNullTriangle(v2);
    ReformActiveDialog(v2);
}

// ea: 0x0059C870
void ProfileManager::DialogDisplayCreateSave()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_CONFIRM_CREATE_SAVE", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_DIALOG_YES",
        ProfileManager::DialogResponseConfirmCreateSave);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v4)->AddOption(
        "MEM_DIALOG_NO",
        ProfileManager::DialogResponseContinueNoSave);
    HighlightActiveOption(v4, 1);
    SetDialogNullTriangle(v4);
    ReformActiveDialog(v4);
}

// ============================================================================
// ProfileMainMenu
// ============================================================================

// ea: 0x00574EB0
ProfileMainMenu* ProfileMainMenu::Me()
{
    return (ProfileMainMenu*)g_femanager.fems->menus[27];
}

// ea: 0x005B7BF0
void ProfileMainMenu::Init()
{
}

// ea: 0x00574EC0
void ProfileMainMenu::ClearEntries()
{
    for (int v2 = 0; v2 < 6; ++v2)
    {
        entries[v2]->SetText("FEMENU_PROFILE_EMPTY");
        mMenuStatus[v2] = 2;
    }
}

// ea: 0x00574F00
void ProfileMainMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00574F10
void ProfileMainMenu::CreateProfile()
{
    ((VKMenu*)g_femanager.fems->menus[18])->mSlotNum = highlighted;
    system->MakeActive(18);
}

// ea: 0x00574F40
void ProfileMainMenu::ButtonHeldAction()
{
    if (button_held_down == 4)
        OnUp(0);
    else if (button_held_down == 8)
        OnDown(0);
}

// ea: 0x00574F70
bool ProfileMainMenu::DialogResponseProfileLoadOk(int client)
{
    (void)client;
    g_femanager.fems->MakeActive(8);
    return true;
}

// ea: 0x00574F80
bool ProfileMainMenu::DialogResponseProfileEdit(int client)
{
    (void)client;
    g_femanager.fems->MakeActive(29);
    return true;
}

// ea: 0x00574F90
bool ProfileMainMenu::DialogResponseDeleteCancel(int client)
{
    (void)client;
    return true;
}

// ea: 0x00580730
void ProfileMainMenu::Draw()
{
    mPanel->Draw();
    mHelpBar->Draw(false);
    FEMenu::Draw();
}

// ea: 0x00580760
void ProfileMainMenu::UpdateWidescreen(bool widescreen)
{
    if (mPanel != nullptr)
    {
        mPanel->UpdateWidescreen(widescreen, 320.0f);
        mHelpBar->UpdateForWidescreen(widescreen);
        if (highlighted != -1)
        {
            if (mMenuStatus[highlighted] == 1)
                mHelpBar->SetText("FEMENU_HELPBAR_MANAGER");
            else
                mHelpBar->SetText("FEMENU_OP_HELPBAR");
        }
    }
}

// ea: 0x005807D0
void ProfileMainMenu::OnTriangle(int c)
{
    (void)c;
    if (gSaveGameData[0].mStubData.mProfileName[0] != 0)
        system->MakeActive(8);
}

// ea: 0x005B7C00
void ProfileMainMenu::OnUp(int c)
{
    (void)c;
    Up();
    OnSelectionChange();
}

// ea: 0x005B7C20
void ProfileMainMenu::OnDown(int c)
{
    (void)c;
    Down();
    OnSelectionChange();
}

// ea: 0x005807F0
void ProfileMainMenu::OnSelectionChange()
{
    for (int v2 = 0; v2 < 6; ++v2)
    {
        if (mMenuStatus[v2] == 0)
        {
            mMenuStatus[v2] = 2;
            entries[v2]->SetText("FEMENU_PROFILE_EMPTY");
        }
    }
    int v5 = mMenuStatus[highlighted];
    if (v5 == 2)
    {
        mMenuStatus[highlighted] = 0;
        entries[highlighted]->SetText("FEMENU_PROFILE_CREATE");
        v5 = 0;
    }
    FEText* TextPointer = mPanel->GetTextPointer("text_current_selected");
    Broc::string v31 = entries[highlighted]->GetText();
    const char* v12 = v31.mBlock != nullptr
                          ? (const char*)&v31.mBlock[1]
                          : defaultFileName;
    TextPointer->SetText(v12);
    if (v5 == 1)
    {
        SaveGameData* slot = mSaveSlots[highlighted];
        FEText* v15 =
            mPanel->GetTextPointer("text_option_set_01");
        v15->SetShown(true);
        switch (slot->mStubData.mControllerStickConfiguration)
        {
        case 0:
            v15->SetText("FEMENU_COP_STICK_DEFAULT");
            break;
        case 1:
            v15->SetText("FEMENU_COP_STICK_SOUTHPAW");
            break;
        case 2:
            v15->SetText("FEMENU_COP_STICK_LEGACY");
            break;
        case 3:
            v15->SetText("FEMENU_COP_STICK_LEGACYSOUTHPAW");
            break;
        default:
            break;
        }
        FEText* v17 =
            mPanel->GetTextPointer("text_option_set_02");
        v17->SetText(
            OptionsControlsMenu::kButtonLayoutStrings
                [slot->mStubData.mControllerButtonConfiguration]);
        v17->SetShown(true);
        mPanel->GetTextPointer("text_option_01")->SetShown(true);
        mPanel->GetTextPointer("text_option_02")->SetShown(true);
        mPanel->GetTextPointer("text_option_03")->SetShown(true);
        Broc::string v30 = entries[highlighted]->GetText();
        mSelectedProfile = v30.mBlock != nullptr
                               ? (const char*)&v30.mBlock[1]
                               : defaultFileName;
        mHelpBar->SetText("FEMENU_HELPBAR_MANAGER");
    }
    else
    {
        mPanel->GetTextPointer("text_option_01")->SetShown(false);
        mPanel->GetTextPointer("text_option_02")->SetShown(false);
        mPanel->GetTextPointer("text_option_03")->SetShown(false);
        mPanel->GetTextPointer("text_option_set_01")->SetShown(false);
        mPanel->GetTextPointer("text_option_set_02")->SetShown(false);
        mPanel->GetTextPointer("text_option_set_03")->SetShown(false);
        mHelpBar->SetText("FEMENU_OP_HELPBAR");
    }
}

// ea: 0x00580B00
void ProfileMainMenu::DeleteDone(int client)
{
    (void)client;
    if (g_femanager.mProfileManager->mSaveEnabled)
    {
        ((ProfileMainMenu*)g_femanager.fems->menus[27])
            ->mMenuStatus[((ProfileMainMenu*)g_femanager.fems->menus[27])
                              ->highlighted] = 2;
    }
    ((ProfileMainMenu*)g_femanager.fems->menus[27])->OnSelectionChange();
}

// ea: 0x00586E20
void ProfileMainMenu::OnActivate(int previous)
{
    (void)previous;
    FEMenu::OnActivate();
    SetHigh(0, false);
    OnSelectionChange();
}

// ea: 0x00586E50
void ProfileMainMenu::LoadProfilesDone()
{
    ProfileManager::Profile* profileSlots[6];
    int numProfiles = g_femanager.mProfileManager->GetProfiles(profileSlots);
    for (int v2 = 0; v2 < 6; ++v2)
    {
        entries[v2]->SetText("FEMENU_PROFILE_EMPTY");
        mMenuStatus[v2] = 2;
    }
    for (int v4 = 0; v4 < numProfiles; ++v4)
    {
        ProfileManager::Profile* v5 = profileSlots[v4];
        int profileSlot = v5->profileSlot;
        entries[profileSlot]->SetText(v5->profileName);
        mMenuStatus[profileSlot] = 1;
    }
    DialogMenuSystem* v10 = g_femanager.mDMS[currCl];
    if (v10->GetActiveMenu() == 1 && (v10->flags & 2) != 0)
    {
        v10->MakeActive(0);
        v10->flags &= ~2;
    }
    else
    {
        if (!v10->mWasIGMSUpWhenLaunched)
            g_femanager.mIGMS[currCl]->MakeActive(-1);
        v10->mDisplay->mIsClosing = true;
    }
}

// ea: 0x005933A0
void ProfileMainMenu::DialogDisplayProfileLoading(int delaySecs)
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_XBOX_LOAD_WARNING", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v3 = g_femanager.GetDMS(currCl);
    SetDialogNullTriangle(v3);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v4)->CloseOnDelay(
        delaySecs, ProfileMainMenu::DialogDisplayProfileLoadSuccess);
    ReformActiveDialog(v4);
}

// ea: 0x0058ECC0
void ProfileMainMenu::DialogDisplayProfileLoadSuccess(int client)
{
    (void)client;
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_LOAD_SUCCESS", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v1 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v1)->AddOption(
        "MEM_DIALOG_OK", ProfileMainMenu::DialogResponseProfileLoadOk);
    SetDialogNullTriangle(v1);
    HighlightActiveOption(v1, 0);
    ReformActiveDialog(v1);
}

// ea: 0x00597960
void ProfileMainMenu::SetPanelFile(PanelFile* pf)
{
    mPanel = pf;
    pf->GetPointer("bkg_main_image")->SetShown(false);
    FEText* TextPointer = mPanel->GetTextPointer("text_helpbar");
    FEMultiLineText* v6 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v6 != nullptr)
    {
        color32 col;
        int v19 = TextPointer->GetColor().i;
        float layer = TextPointer->GetScaleX();
        float x1 = TextPointer->GetY();
        float v16 = TextPointer->GetX();
        font_index v7 = TextPointer->GetFont();
        v6 = new (v6) FEMultiLineText(v7, x1, 0.0f, 0,
                                      (panel_layer)layer, 0.0f, 0, v19,
                                      col);
    }
    mHelpBar = v6;
    v6->SetNumLines(1);
    mPanel->GetTextPointer("text_title_main")->SetText(
        "FEMENU_PROFILE_OPTIONS");
    mPanel->GetTextPointer("text_title_instructions")->SetText(
        "FEMENU_PROFILE_INSTRUCTIONS");
    mPanel->GetTextPointer("text_option_01")->SetText(
        "FEMENU_PROFILE_STICK_LAYOUT");
    mPanel->GetTextPointer("text_option_02")->SetText(
        "FEMENU_PROFILE_BUTTON_LAYOUT");
    for (int i = 0; i < 6; ++i)
        AddEntry(i, mPanel->GetTextPointer(kProfileTextGeoms[3 + i]),
                 false);
}

// ea: 0x00597B10
void ProfileMainMenu::Select(int entry_num)
{
    int v2 = mMenuStatus[entry_num];
    if (v2 != 0)
    {
        if (v2 == 1)
        {
            LoadSelectedProfile(true);
        }
        else if (v2 != 2)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\ProfileMainMenu.cpp";
            AeAssert::gCurrentLine = 226;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Unknown profile menu item: %d", v2))
                __debugbreak();
        }
    }
    else
    {
        ((VKMenu*)g_femanager.fems->menus[18])->mSlotNum = highlighted;
        system->MakeActive(18);
    }
}

// ea: 0x00597BB0
bool ProfileMainMenu::DialogResponseDeleteConfirm(int client)
{
    (void)client;
    int v1 = ((ProfileMainMenu*)g_femanager.fems->menus[27])->highlighted;
    const char* v2 =
        ((ProfileMainMenu*)g_femanager.fems->menus[27])
            ->mSaveSlots[v1]->mStubData.mProfileName;
    if (gSaveGameData[0].mStubData.mProfileName[0] != 0
        && strcmp(gSaveGameData[0].mStubData.mProfileName, v2) == 0)
    {
        gSaveGameData[0].Init();
        gSaveGameData[0].mStubData.Init();
        gSaveGameData[0].mStubData.mProfileName[0] = 0;
    }
    g_femanager.mProfileManager->DeleteProfile(
        v1, ProfileMainMenu::DeleteDone);
    return false;
}

// ea: 0x00594300
void ProfileMainMenu::LoadSelectedProfile(bool displayDialog)
{
    if (mMenuStatus[highlighted] != 1)
    {
        gSaveGameData[0].Init();
        gSaveGameData[0].mStubData.Init();
    }
    else
    {
        if (displayDialog)
            DialogDisplayProfileLoading(1);
        SaveGameData* v3 = mSaveSlots[highlighted];
        if (v3 != nullptr)
        {
            gSaveGameData[0].LoadData(v3);
            gSaveGameData[0].mStubData.mSaved = true;
            gSaveGameData[0].mStubData.ApplyStubOptions();
        }
        else
        {
            gSaveGameData[0].Init();
            gSaveGameData[0].mStubData.Init();
        }
    }
}

// ea: 0x0059AE70
bool ProfileMainMenu::DialogResponseDelete(int client)
{
    (void)client;
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("FEMENU_PROFILE_DELETE_TITLE", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v1 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v1)->AddOption(
        "FEMENU_PROFILE_DELETE_CANCEL",
        ProfileMainMenu::DialogResponseDeleteCancel);
    DialogMenuSystem* v3 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v3)->AddOption(
        "FEMENU_PROFILE_DELETE_CONFIRM",
        ProfileMainMenu::DialogResponseDeleteConfirm);
    HighlightActiveOption(v3, 0);
    ReformActiveDialog(v3);
    return false;
}

// ea: 0x0059C560
void ProfileMainMenu::DialogDisplayProfileSelected()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("FEMENU_PROFILE_SELECTED", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "FEMENU_PROFILE_EDIT",
        ProfileMainMenu::DialogResponseProfileEdit);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v4)->AddOption(
        "FEMENU_PROFILE_DELETE", ProfileMainMenu::DialogResponseDelete);
    HighlightActiveOption(v4, 0);
    ReformActiveDialog(v4);
}

// ea: 0x0059D1A0
void ProfileMainMenu::OnSquare(int c)
{
    (void)c;
    if (mMenuStatus[highlighted] == 1)
    {
        SaveGameData* v4 = mSaveSlots[highlighted];
        if (v4 != nullptr)
        {
            gSaveGameData[0].LoadData(v4);
            gSaveGameData[0].mStubData.mSaved = true;
            gSaveGameData[0].mStubData.ApplyStubOptions();
        }
        else
        {
            gSaveGameData[0].Init();
            gSaveGameData[0].mStubData.Init();
        }
        DialogDisplayProfileSelected();
    }
}

// ea: 0x00593220
ProfileMainMenu::ProfileMainMenu(FEMenuSystem* s)
    : FEMenu(s, 6, 320, 240, 8, 0)
{
    flags = (int16_t)(flags | 0x82);
    mSelectedProfile = nullptr;
    mPanel = nullptr;
    mHelpBar = nullptr;
    default_color_scheme = 19;
    for (int i = 0; i < 6; ++i)
    {
        unsigned int GameSaveSize =
            MemoryUnitManager::GetGameSaveSize(0x1BF4u);
        mSaveSlots[i] =
            (SaveGameData*)mem_heap_malloc(32, GameSaveSize);
    }
}

// ea: 0x005932C0
ProfileMainMenu::~ProfileMainMenu()
{
    mPanel = nullptr;
    if (mHelpBar != nullptr)
        delete mHelpBar;
    mHelpBar = nullptr;
    for (int i = 0; i < 6; ++i)
    {
        mem_heap_free(mSaveSlots[i]);
        mSaveSlots[i] = nullptr;
    }
    FEMenu::Cleanup();
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    mem_heap_free(entries);
    if (helpbar1 != nullptr)
        delete helpbar1;
    if (helpbar2 != nullptr)
        delete helpbar2;
    if (helpbar3 != nullptr)
        delete helpbar3;
}

// ============================================================================
// ProfileEditMenu
// ============================================================================

// ea: 0x00574FA0
ProfileEditMenu* ProfileEditMenu::Me()
{
    return (ProfileEditMenu*)g_femanager.fems->menus[29];
}

// ea: 0x005B7C40
void ProfileEditMenu::Init()
{
}

// ea: 0x005B7C50
void ProfileEditMenu::OnLeft(int c)
{
    (void)c;
}

// ea: 0x005B7C60
void ProfileEditMenu::OnRight(int c)
{
    (void)c;
}

SaveGameData** ProfileMainMenu::GetSaveSlots()
{
    return mSaveSlots;
}

void ProfileEditMenu::NeedWrite()
{
    mNeedWrite = true;
}

// ea: 0x00574FB0
void ProfileEditMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00574FC0
void ProfileEditMenu::OnUp(int c)
{
    (void)c;
    Up();
    mProfileEditText[2]->SetText(
        kProfileTextOptionStrings[highlighted]);
    int v4 = mInstructions->GetBoxWidth();
    mInstructions->SetTextBox(
        kProfileTextInstructionStrings[highlighted], v4, -1.0f);
}

// ea: 0x00575020
void ProfileEditMenu::OnDown(int c)
{
    (void)c;
    Down();
    mProfileEditText[2]->SetText(
        kProfileTextOptionStrings[highlighted]);
    int v4 = mInstructions->GetBoxWidth();
    mInstructions->SetTextBox(
        kProfileTextInstructionStrings[highlighted], v4, -1.0f);
}

// ea: 0x00575080
void ProfileEditMenu::Select(int entry_num)
{
    switch (entry_num)
    {
    case 0:
        system->MakeActive(19);
        break;
    case 1:
        system->MakeActive(20);
        break;
    case 3:
        system->MakeActive(22);
        break;
    case 4:
        system->MakeActive(26);
        break;
    default:
        return;
    }
}

// ea: 0x005750E0
void ProfileEditMenu::ButtonHeldAction()
{
    if (button_held_down == 4)
        OnUp(0);
    else if (button_held_down == 8)
        OnDown(0);
}

// ea: 0x00575110
void ProfileEditMenu::HighlightDefault(int previousMenu)
{
    switch (previousMenu)
    {
    case 20:
        SetHigh(1, false);
        break;
    case 22:
        SetHigh(3, false);
        break;
    case 26:
        SetHigh(4, false);
        break;
    default:
        SetHigh(0, false);
        break;
    }
}

// ea: 0x005751A0
void ProfileEditMenu::ExitMenu(int client)
{
    (void)client;
    if (((ProfileEditMenu*)g_femanager.fems->menus[29])->mExitToMain)
        g_femanager.fems->MakeActive(8);
    else
        g_femanager.fems->MakeActive(27);
}

// ea: 0x00580B40
void ProfileEditMenu::Draw()
{
    mPanel->Draw();
    mHelpBar->Draw(false);
    FEMenu::Draw();
}

// ea: 0x00580B60
void ProfileEditMenu::UpdateWidescreen(bool widescreen)
{
    if (mPanel != nullptr)
    {
        mPanel->UpdateWidescreen(widescreen, 320.0f);
        mHelpBar->UpdateForWidescreen(widescreen);
        mHelpBar->SetText("FEMENU_OP_HELPBAR");
    }
}

// ea: 0x00580BA0
void ProfileEditMenu::OnActivate(int previous)
{
    FEMenu::OnActivate();
    mSaveDialogDisplayed = false;
    mPreviousSettings = gSaveGameData[0].mStubData;
    if (previous == 18 || previous == 8)
    {
        mExitToMain = true;
    }
    else if (previous == 27)
    {
        mExitToMain = false;
    }
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
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ProfileEditMenu.cpp";
        AeAssert::gCurrentLine = 201;
        AeAssert::gCurrentExpr = "profile";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("NULL profile being edited"))
            __debugbreak();
    }
    const char* STBString = STBManager::sInst->GetSTBString(
        "MEM_PROFILE_CURRENTLYEDITING");
    char text[256];
    snprintf(text, 0x100, "%s %s", STBString, v3->mProfileName);
    mProfileEditText[1]->SetTextNoLocalize(text);
    mProfileEditText[2]->SetText(
        kProfileTextOptionStrings[highlighted]);
    int v7 = mInstructions->GetBoxWidth();
    mInstructions->SetTextBox(
        kProfileTextInstructionStrings[highlighted], v7, -1.0f);
}

// ea: 0x00580CD0
bool ProfileEditMenu::DialogResponseCancel(int client)
{
    memcpy(&gSaveGameData[0].mStubData,
           &((ProfileEditMenu*)g_femanager.fems->menus[29])
                ->mPreviousSettings,
           sizeof(StubData));
    LocalClient::ClientToPort(client);
    if (((ProfileEditMenu*)g_femanager.fems->menus[29])->mExitToMain)
        g_femanager.fems->MakeActive(8);
    else
        g_femanager.fems->MakeActive(27);
    return true;
}

// ea: 0x00586FD0
void ProfileEditMenu::SetPanelFile(PanelFile* pf)
{
    mPanel = pf;
    FEText* TextPointer = pf->GetTextPointer("text_helpbar");
    FEMultiLineText* v5 =
        (FEMultiLineText*)mem_heap_malloc(0xA8u);
    if (v5 != nullptr)
    {
        color32 col;
        int v17 = TextPointer->GetColor().i;
        col.i = (unsigned int)v17;
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
    mHelpBar->SetText("FEMENU_OP_HELPBAR");

    for (int i = 0; i < 3; ++i)
        mProfileEditText[i] =
            mPanel->GetTextPointer(kProfileTextGeoms[i]);
    mProfileEditText[0]->SetText("FEMENU_OP_TITLE");
    mInstructions =
        (FEMultiLineText*)mPanel->GetTextPointer(
            "text_option_instructions");
    for (int i = 0; i < 5; ++i)
    {
        FEText* v12 =
            mPanel->GetTextPointer(kProfileTextOptionGeoms[i]);
        AddEntry(i, v12, false);
        entries[i]->SetText(kProfileTextOptionStrings[i]);
    }
}

// ea: 0x0059C6B0
bool ProfileEditMenu::DialogResponseSave(int client)
{
    ProfileEditMenu* v1 = (ProfileEditMenu*)g_femanager.fems->menus[29];
    if (v1->mNeedWrite)
    {
        v1->mNeedWrite = false;
        g_femanager.mProfileManager->SaveProfile(
            ProfileEditMenu::ExitMenu, false);
        return false;
    }
    else
    {
        LocalClient::ClientToPort(client);
        if (((ProfileEditMenu*)g_femanager.fems->menus[29])->mExitToMain)
            g_femanager.fems->MakeActive(8);
        else
            g_femanager.fems->MakeActive(27);
        return true;
    }
}

// ea: 0x0059D200
void ProfileEditMenu::DialogDisplayConfirmSave()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_CONFIRM_SAVE", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v2 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v2)->AddOption(
        "MEM_DIALOG_YES", ProfileEditMenu::DialogResponseSave);
    DialogMenuSystem* v4 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v4)->AddOption(
        "MEM_DIALOG_NO", ProfileEditMenu::DialogResponseCancel);
    HighlightActiveOption(v4, 1);
    ReformActiveDialog(v4);
}

// ea: 0x0059D720
void ProfileEditMenu::OnTriangle(int c)
{
    (void)c;
    if (mNeedWrite)
    {
        memcpy(((ProfileMainMenu*)g_femanager.fems->menus[27])
                   ->mSaveSlots[gSaveGameData[0].mStubData.mSaveGameSlot],
               &gSaveGameData[0], 7156);
        DialogDisplayConfirmSave();
    }
    else if (mExitToMain)
    {
        g_femanager.fems->MakeActive(8);
    }
    else
    {
        g_femanager.fems->MakeActive(27);
    }
}

// ea: 0x005934A0
ProfileEditMenu::ProfileEditMenu(FEMenuSystem* s)
    : FEMenu(s, 5, 320, 240, 8, 0)
{
    mNeedWrite = false;
    mPanel = nullptr;
    mHelpBar = nullptr;
    mInstructions = nullptr;
    mPreviousSettings.mProfileName[0] = 0;
    mPreviousSettings.mControllerPort = 0;
    mPreviousSettings.Init();
    flags = (int16_t)(flags | 0x82);
    mExitToMain = false;
    mSaveDialogDisplayed = false;
    default_color_scheme = 19;
}

// ea: 0x00593530
ProfileEditMenu::~ProfileEditMenu()
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
    FEMenu::~FEMenu();
}

// ============================================================================
// MemCardCheckMenu
// ============================================================================

// ea: 0x00573480
MemCardCheckMenu* MemCardCheckMenu::Me()
{
    return (MemCardCheckMenu*)g_femanager.fems->menus[28];
}

// ea: 0x005B7910
void MemCardCheckMenu::Init()
{
}

void MemCardCheckMenu::SetDialogDisplayed(int dialog)
{
    mDialogDisplayed = dialog;
}

// ea: 0x00573490
void MemCardCheckMenu::UpdateWidescreen(bool ws)
{
    (void)ws;
}

// ea: 0x0057F6A0
bool MemCardCheckMenu::DialogResponseContinue(int client)
{
    (void)client;
    g_femanager.mProfileManager->Reset();
    ((MemCardCheckMenu*)g_femanager.fems->menus[28])->mDialogDisplayed = 0;
    return true;
}

// ea: 0x005866F0
void MemCardCheckMenu::Activate()
{
    mActivated = true;
    mDelayTime = Sys_Milliseconds() + 2000;
    g_femanager.mProfileManager->Reset();
    ((MemCardCheckMenu*)g_femanager.fems->menus[28])->mDialogDisplayed = 0;
}

// ea: 0x00586730
void MemCardCheckMenu::Update(float time_inc)
{
    if (!mActivated)
    {
        mActivated = true;
        mDelayTime = Sys_Milliseconds() + 2000;
        g_femanager.mProfileManager->Reset();
        ((MemCardCheckMenu*)g_femanager.fems->menus[28])->mDialogDisplayed =
            0;
    }
    FEMenu::Update(time_inc);
    if (gSaveGameData[0].mStubData.mProfileName[0] != 0)
        g_femanager.fems->MakeActive(8);
}

// ea: 0x0058E4C0
void MemCardCheckMenu::DialogDisplayNotifyAutoSave()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_WARN_AUTOSAVE", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v3 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v3)->AddOption(
        "MEM_DIALOG_OK", MemCardCheckMenu::DialogResponseContinue);
    HighlightActiveOption(v3, 0);
    SetDialogNullTriangle(v3);
    ReformActiveDialog(v3);
    mDialogDisplayed = 1;
}

// ea: 0x0058E610
bool MemCardCheckMenu::DialogResponseNotifyAutoSave(int client)
{
    (void)client;
    MemCardCheckMenu::Me()->DialogDisplayNotifyAutoSave();
    return false;
}

// ea: 0x00592440
void MemCardCheckMenu::DialogDisplayNoMemCard()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_ERROR_INSERT_CARD_LONG", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v3 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v3)->AddOption(
        "MEM_CONTINUE_WO_SAVING",
        MemCardCheckMenu::DialogResponseNotifyAutoSave);
    DialogMenuSystem* v5 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v5)->AddOption(
        "MEM_RETRY", MemCardCheckMenu::DialogResponseRetry);
    HighlightActiveOption(v5, 0);
    SetDialogNullTriangle(v5);
    ReformActiveDialog(v5);
    mDialogDisplayed = 1;
}

// ea: 0x005925D0
void MemCardCheckMenu::DialogDisplayNoFreeSpace()
{
    DialogMenuSystem* DMS = g_femanager.GetDMS(currCl);
    DMS->BringUp("MEM_ERROR_INSUFFICIENT_SPACE", false, false,
                 defaultFileName, true);
    DialogMenuSystem* v3 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v3)->AddOption(
        "MEM_CONTINUE_WO_SAVING",
        MemCardCheckMenu::DialogResponseNotifyAutoSave);
    DialogMenuSystem* v5 = g_femanager.GetDMS(currCl);
    ActiveDialogLayer(v5)->AddOption(
        "MEM_RETRY", MemCardCheckMenu::DialogResponseRetry);
    HighlightActiveOption(v5, 0);
    SetDialogNullTriangle(v5);
    ReformActiveDialog(v5);
    mDialogDisplayed = 1;
}

// ea: 0x00592760
bool MemCardCheckMenu::DialogResponseRetry(int client)
{
    (void)client;
    ProfileManager* mProfileManager = g_femanager.mProfileManager;
    MemoryUnitManager::SetActiveMemoryUnit(8);
    mProfileManager->mSaveEnabled = true;
    MemoryUnitManager::MemoryUnitInfo storageInfo;
    MemoryUnitManager::eStatus MemoryUnitInfo =
        MemoryUnitManager::GetMemoryUnitInfo(&storageInfo);
    if (MemoryUnitInfo == MemoryUnitManager::eUnformatted
        || (MemoryUnitInfo == MemoryUnitManager::eSuccess
            && storageInfo.formated == 0)
        || GameSettings::sInst->does_file_exist()
        || GameSettings::sInst->enough_space())
    {
        MemCardCheckMenu::Me()->DialogDisplayNotifyAutoSave();
        return false;
    }
    else
    {
        MemCardCheckMenu::Me()->DialogDisplayNoFreeSpace();
        return false;
    }
}

// ea: 0x005923F0
MemCardCheckMenu::MemCardCheckMenu(FEMenuSystem* s)
    : FEMenu(s, 0, 0, 0, 8, 0)
{
    mDialogDisplayed = 0;
    mActivated = false;
    mDelayTime = 0;
}

// ea: 0x00592430
MemCardCheckMenu::~MemCardCheckMenu()
{
    FEMenu::~FEMenu();
}

MemCardCheckMenu* MemCardCheckMenu_ctor(void* mem, FEMenuSystem* s)
{
    return new (mem) MemCardCheckMenu(s);
}
