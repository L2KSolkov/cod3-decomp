// ============================================================================
// MemoryUnit.cpp - MemoryUnitManager core (peripherals_xboxr:MemoryUnit.o)
// 35 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// Win32 port: see platform/xbox_shim/xbox_shim.cpp for the MU file-system map.
// ============================================================================

#include "MemoryUnitManager.h"
#include "XBoxStorage.h"
#include "xbox_shim.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// rdata 0xCD67AE - shared empty default file name (defined in GameXbox.cpp).
extern const char* const defaultFileName;

static void DbgPrintf(const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);
    OutputDebugStringA(buffer);
}

static void __assert(const char* expr, const char* filename, unsigned int lineno)
{
    fprintf(stderr, "Assertion failed: %s, file %s, line %u\n", expr, filename, lineno);
    __debugbreak();
    abort();
}

// ============================================================================
// Class statics (MemoryUnit.o)
// ============================================================================
MemoryUnitManager::eStatus MemoryUnitManager::mLastError = MemoryUnitManager::eSuccess;
char MemoryUnitManager::mTitlePrefix[64];
int MemoryUnitManager::mActiveMemoryUnit;
unsigned char* MemoryUnitManager::mBuffer;
unsigned int MemoryUnitManager::mNumBytes;
MemoryUnitManager::eOperation MemoryUnitManager::mCurrentOperation;
MemoryUnitManager::Container MemoryUnitManager::mGameSave(defaultFileName);
char MemoryUnitManager::mDisplayStrings[9][64];
MemoryUnitManager::Observer* MemoryUnitManager::mObserver;
MemoryUnitManager::InsertRemoveObserver* MemoryUnitManager::mInsertRemoveObserver;
unsigned int MemoryUnitManager::mInitFlags;

// ea: 0x899070
MemoryUnitManager::MemoryUnitManager()
{
}

// ea: 0x899080
MemoryUnitManager::~MemoryUnitManager()
{
}

// ea: 0x899090
MemoryUnitManager::eStatus MemoryUnitManager::GetLastError()
{
    return mLastError;
}

// ea: 0x8990A0
void MemoryUnitManager::SetLastError(eStatus lastError)
{
    mLastError = lastError;
}

// ea: 0x8990B0
void MemoryUnitManager::SetTitlePrefix(const char* titlePrefix)
{
    strcpy(mTitlePrefix, titlePrefix);
}

// ea: 0x8990D0
const char* MemoryUnitManager::GetTitlePrefix()
{
    return mTitlePrefix;
}

// ea: 0x8990E0
void MemoryUnitManager::SetActiveMemoryUnit(int deviceID)
{
    mActiveMemoryUnit = deviceID;
}

// ea: 0x8990F0
int MemoryUnitManager::GetActiveMemoryUnit()
{
    return mActiveMemoryUnit;
}

// ea: 0x899100
void MemoryUnitManager::RegisterObserver(Observer* callBack)
{
    mObserver = callBack;
}

// ea: 0x899110
void MemoryUnitManager::RegisterInsertRemoveObserver(InsertRemoveObserver* callBack)
{
    mInsertRemoveObserver = callBack;
}

// ea: 0x899120
MemoryUnitManager::Observer* MemoryUnitManager::GetObserver()
{
    return mObserver;
}

// ea: 0x899130
MemoryUnitManager::InsertRemoveObserver* MemoryUnitManager::GetInsertRemoveObserver()
{
    return mInsertRemoveObserver;
}

// ea: 0x899140
unsigned char* MemoryUnitManager::GetBuffer()
{
    return mBuffer;
}

// ea: 0x899150
unsigned int MemoryUnitManager::GetNumBytes()
{
    return mNumBytes;
}

// ea: 0x899160
void MemoryUnitManager::Container::Reset(const char* gameName)
{
    strcpy(mGameName, gameName);
    mCurrentFile = 0;
    mIndex = 0;
}

// ea: 0x8991A0
bool MemoryUnitManager::Container::AddFile(const char* fileName,
                                           unsigned char* buffer,
                                           unsigned int numBytes)
{
    unsigned int mIndex = this->mIndex;
    if (mIndex >= 8)
        return false;
    strcpy(this->mFileNames[mIndex], fileName);
    this->mBuffers[this->mIndex] = buffer;
    this->mNumBytes[this->mIndex++] = numBytes;
    return true;
}

// ea: 0x899200
int MemoryUnitManager::Container::GetTotalBytes()
{
    unsigned int mIndex = this->mIndex;
    int result = 0;
    if (mIndex != 0)
    {
        unsigned int* mNumBytes = this->mNumBytes;
        do
        {
            result += *mNumBytes++;
            --mIndex;
        }
        while (mIndex != 0);
    }
    return result;
}

// ea: 0x899220
int MemoryUnitManager::Container::GetNumFiles() const
{
    return (int)mIndex;
}

// ea: 0x899230
unsigned char* MemoryUnitManager::Container::GetFile(int idx)
{
    return mBuffers[idx];
}

// ea: 0x899250
bool MemoryUnitManager::Container::GetNextFile(char* fileName,
                                               unsigned char** buffer,
                                               unsigned int* numBytes)
{
    unsigned int mCurrentFile = this->mCurrentFile;
    if (mCurrentFile >= this->mIndex)
        return false;
    strcpy(fileName, this->mFileNames[mCurrentFile]);
    *buffer = this->mBuffers[this->mCurrentFile];
    *numBytes = this->mNumBytes[this->mCurrentFile++];
    return true;
}

// ea: 0x8992B0
const char* MemoryUnitManager::Container::GetGameName() const
{
    return mGameName;
}

// ea: 0x8992C0
MemoryUnitManager::eOperation MemoryUnitManager::GetCurrentOperation()
{
    return mCurrentOperation;
}

// ea: 0x8992D0
void MemoryUnitManager::SetCurrentOperation(eOperation op)
{
    (void)op;
}

// ea: 0x8992E0
void MemoryUnitManager::SetDisplayStrings(const char (*displayStrings)[64])
{
    int FirstDeviceID = GetFirstDeviceID();
    if (FirstDeviceID <= GetLastDeviceID())
    {
        do
        {
            strcpy(mDisplayStrings[FirstDeviceID], displayStrings[FirstDeviceID]);
            ++FirstDeviceID;
        }
        while (FirstDeviceID <= GetLastDeviceID());
    }
}

// ea: 0x899330
const char* MemoryUnitManager::GetDisplayString()
{
    if (mActiveMemoryUnit < GetFirstDeviceID()
        || mActiveMemoryUnit > GetLastDeviceID())
    {
        return defaultFileName;
    }
    return mDisplayStrings[mActiveMemoryUnit];
}

// ea: 0x899360
int MemoryUnitManager::FindFirstMemoryUnit()
{
    int storageDevices[9];
    if (GetAvailableMemoryUnits(storageDevices) <= 0)
        return -1;
    return storageDevices[0];
}

// ea: 0x899390
int MemoryUnitManager::FindNextMemoryUnit()
{
    int storageDevices[9];
    int AvailableMemoryUnits = GetAvailableMemoryUnits(storageDevices);
    if (AvailableMemoryUnits <= 0)
        return -1;
    if (mActiveMemoryUnit == storageDevices[AvailableMemoryUnits - 1])
        return storageDevices[0];
    int v2 = 0;
    while (mActiveMemoryUnit >= storageDevices[v2])
    {
        if (++v2 >= AvailableMemoryUnits)
            return -1;
    }
    return storageDevices[v2];
}

// ea: 0x8993E0
int MemoryUnitManager::FindPrevMemoryUnit()
{
    int storageDevices[9];
    int AvailableMemoryUnits = GetAvailableMemoryUnits(storageDevices);
    if (AvailableMemoryUnits <= 0)
        return -1;
    if (mActiveMemoryUnit == storageDevices[0])
        return storageDevices[AvailableMemoryUnits - 1];
    int v2 = AvailableMemoryUnits - 1;
    if (v2 < 0)
        return -1;
    while (mActiveMemoryUnit <= storageDevices[v2])
    {
        if (--v2 < 0)
            return -1;
    }
    return storageDevices[v2];
}

// ea: 0x899430
MemoryUnitManager::eStatus MemoryUnitManager::SaveGameSync(const Container& gameSave)
{
    if (SaveGame(gameSave) == eSuccess)
    {
        while (Service())
            ;
    }
    return mLastError;
}

// ea: 0x899460
MemoryUnitManager::eStatus MemoryUnitManager::LoadGameSync(const Container& gameLoad)
{
    if (LoadGame(gameLoad) == eSuccess)
    {
        while (Service())
            ;
    }
    return mLastError;
}

// ea: 0x899490
MemoryUnitManager::eStatus MemoryUnitManager::DeleteGameSync(const char* gameName)
{
    if (DeleteGame(gameName) == eSuccess)
    {
        while (Service())
            ;
    }
    return mLastError;
}

// ea: 0x8994C0
MemoryUnitManager::eStatus MemoryUnitManager::FormatSync()
{
    if (Format() == eSuccess)
    {
        while (Service())
            ;
    }
    return mLastError;
}

// ea: 0x8994E0
const char* MemoryUnitManager::StatusToString(eStatus status)
{
    switch (status)
    {
    case eInitializeNotCalled:
        return "Initialize not called";
    case eNoTitlePrefixSet:
        return "No Title Prefix Set";
    case eInvalidIconIndex:
        return "Invalid Icon Index";
    case eQueueUnderflow:
        return "Queue underflow";
    case eQueueFull:
        return "Queue full";
    case eInvalidDeviceID:
        return "Invalid device ID";
    case eDeviceBusy:
        return "Device busy";
    case eEmptyString:
        return "Empty string";
    case eDirNotEmpty:
        return "Directory is not empty";
    case eFileAlreadyExists:
        return "File already exists";
    case eFileDoesNotExist:
        return "File does not exist";
    case eCRCFailure:
        return "CRC failure";
    case eDamagedMedium:
        return "Damaged medium";
    case eNoMedium:
        return "No medium";
    case eIncorrectPadding:
        return "Incorrect padding";
    case eTooManyOpenFiles:
        return "Too many files open";
    case eIncorrectMedium:
        return "Incorrect medium";
    case eNoWritePermissions:
        return "File cannot be opened for writing";
    case eNoReadPermissions:
        return "File cannot be opened for reading";
    case eNotEnoughFreeSpace:
        return "Not enough free space";
    case eMediumFull:
        return "Medium full";
    case eUnformatted:
        return "Unformatted medium";
    case eGenericFailure:
        return "Generic failure";
    case eSuccess:
        return "Success";
    default:
        return "Unknown error";
    }
}

// ea: 0x899680
unsigned int peripheralsGetVersion()
{
    return 1792;
}

// ea: 0x899690
MemoryUnitManager::Container::Container(const char* gameName)
{
    strcpy(mGameName, gameName);
    mCurrentFile = 0;
    mIndex = 0;
}
