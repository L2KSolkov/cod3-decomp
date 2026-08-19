// ============================================================================
// XBoxStorage.cpp - memory-unit filesystem backing + async save/load driver
// Source: peripherals_xboxr:XboxMemoryUnit.o (46 functions)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// Win32 port: the logical MU drive maps onto %USERPROFILE%\.cod3\MemoryUnit\
// via the shims in platform/xbox_shim/xbox_shim.cpp.
// ============================================================================

#include "XBoxStorage.h"
#include "xbox_shim.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

// rdata 0xCD67AE - shared empty default file name (defined in GameXbox.cpp).
extern const char defaultFileName[];

// rdata 0xD50340 - default display strings for every device slot.
static const char DefaultDisplayStrings[9][64] = { "F:\\" };

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
// Class statics (XboxMemoryUnit.o)
// ============================================================================
HANDLE XBoxStorage::mFileHandle;
char XBoxStorage::mGameNamePath[260];
OVERLAPPED XBoxStorage::mOverlapped;
char XBoxStorage::mDrive;
char XBoxStorage::mRoot[3];
MemoryUnitManager::eStatus (*XBoxStorage::mAsyncService)();
int XBoxStorage::mMasks[9] = { 1, 2, 4, 8, 16, 32, 64, 128, 256 };

// ea: 0x899700
MemoryUnitManager::eStatus XBoxStorage::PortSlotToDeviceID(int* devID,
                                                           unsigned int port,
                                                           unsigned int slot)
{
    if (port > 4 || slot > 2)
        return MemoryUnitManager::eInvalidDeviceID;
    *devID = (int)(slot + 2 * port);
    return MemoryUnitManager::eSuccess;
}

// ea: 0x899730
HANDLE XBoxStorage::GetFileHandle()
{
    return mFileHandle;
}

// ea: 0x899740
const char* XBoxStorage::GetGameNamePath()
{
    return mGameNamePath;
}

// ea: 0x899750
void XBoxStorage::ResetOverlapped()
{
    mOverlapped.hEvent = nullptr;
    mOverlapped.Internal = 0;
    mOverlapped.InternalHigh = 0;
    mOverlapped.Offset = 0;
}

// ea: 0x899770
MemoryUnitManager::eStatus (*XBoxStorage::GetAsyncService())()
{
    return mAsyncService;
}

// ea: 0x899780
char XBoxStorage::GetDrive()
{
    return mDrive;
}

// ea: 0x899790
const char* XBoxStorage::GetRoot()
{
    mRoot[0] = mDrive;
    return mRoot;
}

// ea: 0x8997A0
void XBoxStorage::CloseHandles()
{
    mAsyncService = nullptr;
    if (mFileHandle != nullptr)
    {
        if (!CloseHandle(mFileHandle))
            __assert("bSuccess", "source/xbox/XboxMemoryUnit.cpp", 0x2C7);
        mFileHandle = nullptr;
    }
    if (mOverlapped.hEvent != nullptr)
    {
        if (!CloseHandle(mOverlapped.hEvent))
            __assert("bSuccess", "source/xbox/XboxMemoryUnit.cpp", 0x2CF);
        mOverlapped.hEvent = nullptr;
    }
}

// ea: 0x899820
void XBoxStorage::Reset()
{
    CloseHandles();
    mGameNamePath[0] = 0;
    mOverlapped.hEvent = nullptr;
    mOverlapped.Internal = 0;
    mOverlapped.InternalHigh = 0;
    mOverlapped.Offset = 0;
    mDrive = 63;
    mRoot[0] = 0;
}

// ea: 0x899860
MemoryUnitManager::eStatus XBoxStorage::ConvertError(int returnValue)
{
    if (returnValue <= 112)
    {
        switch (returnValue)
        {
        case 112:
            return MemoryUnitManager::eMediumFull;
        case 0:
            return MemoryUnitManager::eSuccess;
        case 85:
            return MemoryUnitManager::eDeviceBusy;
        default:
            break;
        }
    }
    else if (returnValue == 1167)
    {
        return MemoryUnitManager::eNoMedium;
    }
    return MemoryUnitManager::eGenericFailure;
}

// ea: 0x8998A0
XBoxStorage::XBoxStorage()
{
}

// ea: 0x8998B0
XBoxStorage::~XBoxStorage()
{
}

// ea: 0x8998C0
int MemoryUnitManager::GetFirstDeviceID()
{
    return 0;
}

// ea: 0x8998D0
int MemoryUnitManager::GetLastDeviceID()
{
    return 8;
}

// ea: 0x8998E0
MemoryUnitManager::eStatus MemoryUnitManager::Delete(const char* gameName)
{
    (void)gameName;
    return GetLastError();
}

// ea: 0x8998F0
MemoryUnitManager::eStatus MemoryUnitManager::Format()
{
    if (mObserver != nullptr)
        mObserver->Callback(eFormat);
    return GetLastError();
}

// ea: 0x899910
MemoryUnitManager::eStatus MemoryUnitManager::Initialize(unsigned int initFlags)
{
    DbgPrintf("\nMemoryUnitManager: Initialized for XBox...\n");
    mInitFlags = initFlags;
    SetDisplayStrings(DefaultDisplayStrings);
    SetActiveMemoryUnit(8);
    SetLastError(eSuccess);
    return GetLastError();
}

// ea: 0x899950
int MemoryUnitManager::GetAvailableMemoryUnits(int* storageDevices)
{
    int* v1 = storageDevices;
    int v2 = 0;
    DbgPrintf("MemoryUnitManager: Getting Available Storage Devices...\n");
    unsigned int v3 = XGetDevices(XDEVICE_TYPE_MEMORY_UNIT_TABLE);
    if (v3 != 0)
    {
        if ((v3 & (unsigned int)XBoxStorage::mMasks[0]) != 0)
        {
            *storageDevices = 0;
            v1 = storageDevices + 1;
            v2 = 1;
        }
        if ((v3 & (unsigned int)XBoxStorage::mMasks[1]) != 0)
        {
            *v1++ = 1;
            ++v2;
        }
        if ((v3 & (unsigned int)XBoxStorage::mMasks[2]) != 0)
        {
            *v1++ = 2;
            ++v2;
        }
        if ((v3 & (unsigned int)XBoxStorage::mMasks[3]) != 0)
        {
            *v1++ = 3;
            ++v2;
        }
        if ((v3 & (unsigned int)XBoxStorage::mMasks[4]) != 0)
        {
            *v1++ = 4;
            ++v2;
        }
        if ((v3 & (unsigned int)XBoxStorage::mMasks[5]) != 0)
        {
            *v1++ = 5;
            ++v2;
        }
        if ((v3 & (unsigned int)XBoxStorage::mMasks[6]) != 0)
        {
            *v1++ = 6;
            ++v2;
        }
        if ((v3 & (unsigned int)XBoxStorage::mMasks[7]) != 0)
        {
            *v1++ = 7;
            ++v2;
        }
    }
    *v1 = 8;
    return v2 + 1;
}

// ea: 0x899A20
void MemoryUnitManager::GetCRC(unsigned char* buffer, unsigned int size,
                               unsigned char* crcBuffer, unsigned int* crcSize)
{
    if (crcSize != nullptr)
        *crcSize = 20;
    if (crcBuffer != nullptr)
    {
        void* v4 = XCalculateSignatureBegin(0);
        if (v4 == (void*)-1)
            __assert("INVALID_HANDLE_VALUE != hSig", "source/xbox/XboxMemoryUnit.cpp", 0x615);
        if (XCalculateSignatureUpdate(v4, buffer, size) != 0)
            __assert("ERROR_SUCCESS == ret", "source/xbox/XboxMemoryUnit.cpp", 0x619);
        int xsig[5];
        if (XCalculateSignatureEnd(v4, xsig) != 0)
            __assert("ERROR_SUCCESS == ret", "source/xbox/XboxMemoryUnit.cpp", 0x61C);
        *(unsigned int*)(crcBuffer + 0) = (unsigned int)xsig[0];
        *(unsigned int*)(crcBuffer + 4) = (unsigned int)xsig[1];
        *(unsigned int*)(crcBuffer + 8) = (unsigned int)xsig[2];
        *(unsigned int*)(crcBuffer + 12) = (unsigned int)xsig[3];
        *(unsigned int*)(crcBuffer + 16) = (unsigned int)xsig[4];
    }
}

// ea: 0x899AE0
float MemoryUnitManager::GetProgress()
{
    if (GetCurrentOperation() == 0 || mActiveMemoryUnit == -1)
        return 0.0f;
    double mIndex = (double)mGameSave.mIndex;
    float progress_offset = (float)((mGameSave.mCurrentFile - 1) / mIndex);
    float v3 = (float)(1.0 / mIndex * XBoxStorage::mOverlapped.Offset);
    return (float)(v3 / GetNumBytes() + progress_offset);
}

// ea: 0x899B80
unsigned int MemoryUnitManager::BytesToBlocks(unsigned __int64 bytes)
{
    return (unsigned int)((bytes + 0x3FFF) >> 14);
}

// ea: 0x899BA0
MemoryUnitManager::eStatus XBoxStorage::DeviceIDToPortSlot(int devID,
                                                           unsigned int* port,
                                                           unsigned int* slot)
{
    if (devID > 8)
        return MemoryUnitManager::eInvalidDeviceID;
    unsigned int portID[4];
    unsigned int slotID[2];
    portID[0] = 0;
    portID[1] = 1;
    portID[2] = 2;
    portID[3] = 3;
    slotID[0] = 0;
    slotID[1] = 1;
    *port = portID[devID / 2];
    *slot = slotID[devID & 1];
    return MemoryUnitManager::eSuccess;
}

// ea: 0x899C10
MemoryUnitManager::eStatus XBoxStorage::MountActiveMU()
{
    mDrive = 63;
    if (MemoryUnitManager::GetActiveMemoryUnit() == 8)
    {
        mDrive = 85;
        return MemoryUnitManager::eSuccess;
    }
    unsigned int muport = 0;
    unsigned int muslot = 0;
    int ActiveMemoryUnit = MemoryUnitManager::GetActiveMemoryUnit();
    MemoryUnitManager::eStatus result =
        DeviceIDToPortSlot(ActiveMemoryUnit, &muport, &muslot);
    if (result == MemoryUnitManager::eSuccess)
    {
        if (MemoryUnitManager::GetActiveMemoryUnit() >= 0
            && (XGetDevices(XDEVICE_TYPE_MEMORY_UNIT_TABLE)
                & (unsigned int)mMasks[MemoryUnitManager::GetActiveMemoryUnit()]) != 0)
        {
            int v3 = XMountMUA(muport, muslot, &mDrive);
            return ConvertError(v3);
        }
        else
        {
            return MemoryUnitManager::eNoMedium;
        }
    }
    return result;
}

// ea: 0x899CA0
MemoryUnitManager::eStatus XBoxStorage::UnmountActiveMU()
{
    mDrive = 63;
    if (MemoryUnitManager::GetActiveMemoryUnit() == 8)
        return MemoryUnitManager::eSuccess;
    unsigned int muport = 0;
    unsigned int muslot = 0;
    int ActiveMemoryUnit = MemoryUnitManager::GetActiveMemoryUnit();
    MemoryUnitManager::eStatus result =
        DeviceIDToPortSlot(ActiveMemoryUnit, &muport, &muslot);
    if (result == MemoryUnitManager::eSuccess)
    {
        int v2 = XUnmountMU(muport, muslot);
        return ConvertError(v2);
    }
    return result;
}

// ea: 0x899D00
MemoryUnitManager::eStatus XBoxStorage::CreateSaveGame(const char* gameName)
{
    unsigned short wideGameName[64];
    wideGameName[0] = 0;
    if (MultiByteToWideChar(0, 0, gameName, -1,
                            (wchar_t*)wideGameName, 63) == 0)
    {
        __assert("numWideChars != 0", "source/xbox/XboxMemoryUnit.cpp", 0x11C);
    }
    mRoot[0] = mDrive;
    int v1 = XCreateSaveGame(mRoot, (const wchar_t*)wideGameName,
                             4, 0, mGameNamePath, 259);
    if (v1 <= 112)
    {
        switch (v1)
        {
        case 112:
            return MemoryUnitManager::eMediumFull;
        case 0:
            return MemoryUnitManager::eSuccess;
        case 85:
            return MemoryUnitManager::eDeviceBusy;
        default:
            break;
        }
    }
    else if (v1 == 1167)
    {
        return MemoryUnitManager::eNoMedium;
    }
    return MemoryUnitManager::eGenericFailure;
}

// ea: 0x899DB0
MemoryUnitManager::eStatus XBoxStorage::DeleteSaveGame(const char* gameName)
{
    unsigned short wideGameName[64];
    wideGameName[0] = 0;
    if (MultiByteToWideChar(0, 0, gameName, -1,
                            (wchar_t*)wideGameName, 63) == 0)
    {
        __assert("numWideChars != 0", "source/xbox/XboxMemoryUnit.cpp", 0x138);
    }
    mRoot[0] = mDrive;
    int v1 = XDeleteSaveGame(mRoot, (const wchar_t*)wideGameName);
    if (v1 <= 112)
    {
        switch (v1)
        {
        case 112:
            return MemoryUnitManager::eMediumFull;
        case 0:
            return MemoryUnitManager::eSuccess;
        case 85:
            return MemoryUnitManager::eDeviceBusy;
        default:
            break;
        }
    }
    else if (v1 == 1167)
    {
        return MemoryUnitManager::eNoMedium;
    }
    return MemoryUnitManager::eGenericFailure;
}

// ea: 0x899E50
MemoryUnitManager::eStatus XBoxStorage::OpenFile(const char* fileName,
                                                 unsigned int access)
{
    char fullPath[260];
    strcpy(fullPath, mGameNamePath);
    strcat(fullPath, "\\");
    strcat(fullPath, fileName);
    mFileHandle = CreateFileA(fullPath, access, 0, nullptr,
                              OPEN_ALWAYS,
                              FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING
                                  | FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    return (mFileHandle != INVALID_HANDLE_VALUE)
               ? MemoryUnitManager::eSuccess
               : MemoryUnitManager::eGenericFailure;
}

// ea: 0x899F00
MemoryUnitManager::eStatus XBoxStorage::AsyncWriteService()
{
    DWORD numBytesTransferred = 0;
    if (mOverlapped.Internal == 259)
        return MemoryUnitManager::eDeviceBusy;
    if (GetOverlappedResult(mFileHandle, &mOverlapped,
                            &numBytesTransferred, FALSE)
        && numBytesTransferred == MemoryUnitManager::GetNumBytes())
    {
        CloseHandles();
        return MemoryUnitManager::eSuccess;
    }
    else
    {
        CloseHandles();
        return MemoryUnitManager::eGenericFailure;
    }
}

// ea: 0x899F60
MemoryUnitManager::eStatus XBoxStorage::AsyncReadService()
{
    DWORD numBytesTransferred = 0;
    if (mOverlapped.Internal == 259)
        return MemoryUnitManager::eDeviceBusy;
    if (GetOverlappedResult(mFileHandle, &mOverlapped,
                            &numBytesTransferred, FALSE)
        && numBytesTransferred == MemoryUnitManager::GetNumBytes())
    {
        CloseHandles();
        return MemoryUnitManager::eSuccess;
    }
    else
    {
        CloseHandles();
        return MemoryUnitManager::eGenericFailure;
    }
}

// ea: 0x899FC0
int XBoxStorage::GetSectorSize()
{
    mRoot[0] = mDrive;
    return XGetDiskSectorSizeA(mRoot);
}

// ea: 0x899FE0
int XBoxStorage::GetClusterSize()
{
    mRoot[0] = mDrive;
    return XGetDiskClusterSizeA(mRoot);
}

// ea: 0x89A000
int XBoxStorage::GetAsyncChunk()
{
    mRoot[0] = mDrive;
    return XGetDiskSectorSizeA(mRoot);
}

// ea: 0x89A020
MemoryUnitManager::eStatus XBoxStorage::BeginAsyncWrite()
{
    mAsyncService = AsyncWriteService;
    mOverlapped.hEvent = nullptr;
    mOverlapped.Internal = 0;
    mOverlapped.InternalHigh = 0;
    mOverlapped.Offset = 0;
    mRoot[0] = mDrive;
    unsigned int v0 = XGetDiskSectorSizeA(mRoot);
    if (MemoryUnitManager::GetNumBytes() % v0 != 0)
    {
        __assert("MemoryUnitManager::GetNumBytes() % XBoxStorage::GetSectorSize() == 0",
                 "source/xbox/XboxMemoryUnit.cpp", 0x268);
    }
    unsigned int NumBytes = MemoryUnitManager::GetNumBytes();
    DWORD v2 = SetFilePointer(mFileHandle, (LONG)NumBytes,
                              nullptr, FILE_BEGIN);
    if (v2 != MemoryUnitManager::GetNumBytes())
    {
        __assert("dwPos == MemoryUnitManager::GetNumBytes()",
                 "source/xbox/XboxMemoryUnit.cpp", 0x276);
    }
    if (!SetEndOfFile(mFileHandle))
        __assert("bSuccess", "source/xbox/XboxMemoryUnit.cpp", 0x27A);
    DWORD v5 = MemoryUnitManager::GetNumBytes();
    unsigned char* Buffer = MemoryUnitManager::GetBuffer();
    if (WriteFile(mFileHandle, Buffer, v5, nullptr, &mOverlapped)
        || GetLastError() == ERROR_IO_PENDING)
    {
        return MemoryUnitManager::eSuccess;
    }
    CloseHandles();
    mGameNamePath[0] = 0;
    mOverlapped.hEvent = nullptr;
    mOverlapped.Internal = 0;
    mOverlapped.InternalHigh = 0;
    mOverlapped.Offset = 0;
    mRoot[0] = 0;
    mDrive = 63;
    return MemoryUnitManager::eGenericFailure;
}

// ea: 0x89A160
MemoryUnitManager::eStatus XBoxStorage::BeginAsyncRead()
{
    mAsyncService = AsyncReadService;
    mOverlapped.hEvent = nullptr;
    mOverlapped.Internal = 0;
    mOverlapped.InternalHigh = 0;
    mOverlapped.Offset = 0;
    DWORD NumBytes = MemoryUnitManager::GetNumBytes();
    unsigned char* Buffer = MemoryUnitManager::GetBuffer();
    if (!ReadFile(mFileHandle, Buffer, NumBytes, nullptr, &mOverlapped))
    {
        if (GetLastError() == ERROR_HANDLE_EOF)
        {
            CloseHandles();
            return MemoryUnitManager::eSuccess;
        }
        if (GetLastError() != ERROR_IO_PENDING)
        {
            __assert("::GetLastError() == ERROR_IO_PENDING",
                     "source/xbox/XboxMemoryUnit.cpp", 0x2A5);
        }
    }
    return MemoryUnitManager::eSuccess;
}

// ea: 0x89A1E0
int MemoryUnitManager::GetSavedGames(SavedGame* savedGames)
{
    if (mCurrentOperation != 0)
    {
        SetLastError(eDeviceBusy);
        return GetLastError();
    }
    int v2 = 0;
    DbgPrintf("MemoryUnit: Retrieving Saved Games for %d...\n", mActiveMemoryUnit);
    eStatus v3 = XBoxStorage::MountActiveMU();
    SetLastError(v3);
    if (GetLastError() != eSuccess)
    {
        XBoxStorage::UnmountActiveMU();
        return eGenericFailure;
    }
    XBoxStorage::mRoot[0] = XBoxStorage::mDrive;
    XGAME_FIND_DATA gameFindData;
    HANDLE SaveGame = XFindFirstSaveGame(XBoxStorage::mRoot, &gameFindData);
    if (SaveGame == (HANDLE)-1)
    {
        XBoxStorage::UnmountActiveMU();
        return eSuccess;
    }
    do
    {
        size_t v6 = wcslen((const wchar_t*)gameFindData.szSaveGameName);
        if (WideCharToMultiByte(0, 0,
                                (const wchar_t*)gameFindData.szSaveGameName,
                                (int)v6 + 1, savedGames[v2].name, 64,
                                nullptr, nullptr) == 0)
        {
            __assert("numChars != 0", "source/xbox/XboxMemoryUnit.cpp", 0x3CB);
        }
        savedGames[v2].size = XGetDisplayBlocks(gameFindData.szSaveGameDirectory);
        FILETIME localTime;
        SYSTEMTIME systemTime;
        FileTimeToLocalFileTime(&gameFindData.wfd.ftLastWriteTime, &localTime);
        FileTimeToSystemTime(&localTime, &systemTime);
        savedGames[v2].mt.second = (unsigned char)systemTime.wSecond;
        savedGames[v2].mt.minute = (unsigned char)systemTime.wMinute;
        savedGames[v2].mt.hour = (unsigned char)systemTime.wHour;
        savedGames[v2].mt.day = (unsigned char)systemTime.wDay;
        savedGames[v2].mt.month = (unsigned char)systemTime.wMonth;
        savedGames[v2].mt.year = (unsigned short)systemTime.wYear;
        ++v2;
    }
    while (XFindNextSaveGame(SaveGame, &gameFindData) != 0);
    if (XFindClose(SaveGame) == 0)
    {
        SetLastError(eGenericFailure);
        XBoxStorage::UnmountActiveMU();
        return eGenericFailure;
    }
    eStatus v11 = XBoxStorage::UnmountActiveMU();
    SetLastError(v11);
    if (GetLastError() != eSuccess)
        return eGenericFailure;
    return v2;
}

// ea: 0x89A370
MemoryUnitManager::eStatus MemoryUnitManager::GetMemoryUnitInfo(MemoryUnitInfo* storageInfo)
{
    if (mCurrentOperation != 0)
    {
        SetLastError(eDeviceBusy);
        return GetLastError();
    }
    eStatus v2 = XBoxStorage::MountActiveMU();
    SetLastError(v2);
    if (GetLastError() != eSuccess)
        goto LABEL_6;
    XBoxStorage::mRoot[0] = XBoxStorage::mDrive;
    ULARGE_INTEGER totalNumberOfFreeBytes;
    unsigned __int64 totalNumberOfBytes;
    int freeBytesAvailable[2];
    // WIN32: the logical MU root ("U") is not a real volume; query the
    // directory behind it (shim-owned divergence).
    if (GetDiskFreeSpaceExA(XGetMemoryUnitRootPath(),
                            (PULARGE_INTEGER)freeBytesAvailable,
                            (PULARGE_INTEGER)&totalNumberOfBytes,
                            &totalNumberOfFreeBytes) == 0)
    {
        SetLastError(eGenericFailure);
LABEL_6:
        XBoxStorage::UnmountActiveMU();
        return GetLastError();
    }
    storageInfo->bytesFree =
        (unsigned __int64)(unsigned int)freeBytesAvailable[0]
        | ((unsigned __int64)(unsigned int)freeBytesAvailable[1] << 32);
    unsigned __int64 v5 = (storageInfo->bytesFree + 0x3FFF) >> 14;
    storageInfo->blocksFree = (unsigned int)v5;
    if (v5 > 0xC351)
        storageInfo->blocksFree = 50001;
    unsigned __int64 v6 = totalNumberOfBytes;
    storageInfo->maxBytes = v6;
    unsigned __int64 v7 = (v6 + 0x3FFF) >> 14;
    storageInfo->maxBlocks = (unsigned int)v7;
    if (v7 > 0xC351)
        storageInfo->maxBlocks = 50001;
    storageInfo->formated = 1;
    eStatus v8 = XBoxStorage::UnmountActiveMU();
    SetLastError(v8);
    GetLastError();
    return GetLastError();
}

// ea: 0x89A470
MemoryUnitManager::eStatus MemoryUnitManager::Save(const char* fileName,
                                                   unsigned char* saveBuffer,
                                                   unsigned int numBytesToSave)
{
    if (mCurrentOperation != eSave)
        __assert("mCurrentOperation == eSave", "source/xbox/XboxMemoryUnit.cpp", 0x432);
    mBuffer = saveBuffer;
    mNumBytes = numBytesToSave;
    if (numBytesToSave > 0x20)
    {
        GetCRC(saveBuffer, numBytesToSave - 32,
               &saveBuffer[numBytesToSave - 32], nullptr);
    }
    eStatus v3 = XBoxStorage::OpenFile(fileName, 0x40000000);
    SetLastError(v3);
    if (GetLastError() != eSuccess)
        goto LABEL_8;
    XBoxStorage::mRoot[0] = XBoxStorage::mDrive;
    if (mNumBytes % XGetDiskSectorSizeA(XBoxStorage::mRoot) != 0)
    {
        SetLastError(eIncorrectPadding);
LABEL_8:
        XBoxStorage::UnmountActiveMU();
        XBoxStorage::CloseHandles();
        XBoxStorage::mGameNamePath[0] = 0;
        XBoxStorage::mOverlapped.hEvent = nullptr;
        XBoxStorage::mOverlapped.Internal = 0;
        XBoxStorage::mOverlapped.InternalHigh = 0;
        XBoxStorage::mOverlapped.Offset = 0;
        XBoxStorage::mDrive = 63;
        XBoxStorage::mRoot[0] = 0;
        goto LABEL_9;
    }
    {
        eStatus v5 = XBoxStorage::BeginAsyncWrite();
        SetLastError(v5);
        if (GetLastError() == eSuccess)
            return GetLastError();
        XBoxStorage::UnmountActiveMU();
        XBoxStorage::Reset();
    }
LABEL_9:
    mGameSave.Reset(defaultFileName);
    eOperation v4 = mCurrentOperation;
    mCurrentOperation = (eOperation)0;
    if (mObserver != nullptr)
        mObserver->Callback(v4);
    return GetLastError();
}

// ea: 0x89A5B0
MemoryUnitManager::eStatus MemoryUnitManager::Load(const char* fileName,
                                                   unsigned char* loadBuffer,
                                                   int numBytesToLoad)
{
    if (mCurrentOperation != eLoad)
        __assert("mCurrentOperation == eLoad", "source/xbox/XboxMemoryUnit.cpp", 0x47A);
    mBuffer = loadBuffer;
    mNumBytes = (unsigned int)numBytesToLoad;
    eStatus v3 = XBoxStorage::OpenFile(fileName, 0x80000000);
    SetLastError(v3);
    if (GetLastError() != eSuccess)
    {
        XBoxStorage::UnmountActiveMU();
        XBoxStorage::CloseHandles();
    }
    else
    {
        DWORD FileSize = GetFileSize(XBoxStorage::mFileHandle, nullptr);
        if (FileSize == GetNumBytes())
        {
            XBoxStorage::mRoot[0] = XBoxStorage::mDrive;
            if (mNumBytes % XGetDiskSectorSizeA(XBoxStorage::mRoot) != 0)
            {
                SetLastError(eIncorrectPadding);
            }
            else
            {
                eStatus v8 = XBoxStorage::BeginAsyncRead();
                SetLastError(v8);
                if (GetLastError() == eSuccess)
                    return GetLastError();
            }
            XBoxStorage::UnmountActiveMU();
            XBoxStorage::Reset();
            mGameSave.Reset(defaultFileName);
            eOperation v7 = mCurrentOperation;
            mCurrentOperation = (eOperation)0;
            if (mObserver != nullptr)
            {
                mObserver->Callback(v7);
                return GetLastError();
            }
            return GetLastError();
        }
        SetLastError(eCRCFailure);
        XBoxStorage::UnmountActiveMU();
        XBoxStorage::CloseHandles();
    }
    XBoxStorage::mGameNamePath[0] = 0;
    XBoxStorage::mOverlapped.hEvent = nullptr;
    XBoxStorage::mOverlapped.Internal = 0;
    XBoxStorage::mOverlapped.InternalHigh = 0;
    XBoxStorage::mOverlapped.Offset = 0;
    XBoxStorage::mDrive = 63;
    XBoxStorage::mRoot[0] = 0;
    mGameSave.Reset(defaultFileName);
    eOperation v5 = mCurrentOperation;
    mCurrentOperation = (eOperation)0;
    if (mObserver != nullptr)
    {
        mObserver->Callback(v5);
        return GetLastError();
    }
    return GetLastError();
}

// ea: 0x89A730
MemoryUnitManager::eStatus MemoryUnitManager::DeleteGame(const char* gameName)
{
    if (mCurrentOperation != 0)
    {
        SetLastError(eDeviceBusy);
        return GetLastError();
    }
    mCurrentOperation = eDelete;
    mGameSave.Reset(gameName);
    eStatus v2 = XBoxStorage::MountActiveMU();
    SetLastError(v2);
    if (GetLastError() != eSuccess)
    {
        XBoxStorage::UnmountActiveMU();
        XBoxStorage::CloseHandles();
        XBoxStorage::mGameNamePath[0] = 0;
        XBoxStorage::mOverlapped.hEvent = nullptr;
        XBoxStorage::mOverlapped.Internal = 0;
        XBoxStorage::mOverlapped.InternalHigh = 0;
        XBoxStorage::mOverlapped.Offset = 0;
        XBoxStorage::mDrive = 63;
        XBoxStorage::mRoot[0] = 0;
        mGameSave.Reset(defaultFileName);
        eOperation v3 = mCurrentOperation;
        mCurrentOperation = (eOperation)0;
        if (mObserver != nullptr)
        {
            mObserver->Callback(v3);
            return GetLastError();
        }
    }
    else
    {
        const char* v4 = mGameSave.GetGameName();
        eStatus v5 = XBoxStorage::DeleteSaveGame(v4);
        SetLastError(v5);
        if (GetLastError() != eSuccess)
        {
            XBoxStorage::UnmountActiveMU();
        }
        else
        {
            eStatus v7 = XBoxStorage::UnmountActiveMU();
            SetLastError(v7);
            if (GetLastError() == eSuccess)
            {
                XBoxStorage::Reset();
                mGameSave.Reset(defaultFileName);
                eOperation v8 = mCurrentOperation;
                mCurrentOperation = (eOperation)0;
                if (mObserver != nullptr)
                    mObserver->Callback(v8);
                return GetLastError();
            }
        }
        XBoxStorage::Reset();
        mGameSave.Reset(defaultFileName);
        eOperation v6 = mCurrentOperation;
        mCurrentOperation = (eOperation)0;
        if (mObserver != nullptr)
            mObserver->Callback(v6);
    }
    return GetLastError();
}

// ea: 0x89A8A0
bool MemoryUnitManager::StartOperation()
{
    char fileName[64];
    unsigned char* buffer;
    unsigned int numBytes;
    memset(fileName, 0, sizeof(fileName));
    buffer = nullptr;
    numBytes = 0;
    if (mGameSave.GetNextFile(fileName, &buffer, &numBytes))
    {
        if (mCurrentOperation == eLoad)
        {
            Load(fileName, buffer, numBytes);
        }
        else if (mCurrentOperation == eSave)
        {
            Save(fileName, buffer, numBytes);
            return 1;
        }
        return 1;
    }
    eOperation v1 = mCurrentOperation;
    mCurrentOperation = (eOperation)0;
    mGameSave.Reset(defaultFileName);
    XBoxStorage::CloseHandles();
    XBoxStorage::mGameNamePath[0] = 0;
    XBoxStorage::mOverlapped.hEvent = nullptr;
    XBoxStorage::mOverlapped.Internal = 0;
    XBoxStorage::mOverlapped.InternalHigh = 0;
    XBoxStorage::mOverlapped.Offset = 0;
    XBoxStorage::mDrive = 63;
    XBoxStorage::mRoot[0] = 0;
    eStatus v2 = XBoxStorage::UnmountActiveMU();
    SetLastError(v2);
    if (mObserver != nullptr)
        mObserver->Callback(v1);
    return 0;
}

// ea: 0x89A9A0
unsigned int MemoryUnitManager::GetGameSaveSize(unsigned int size)
{
    eStatus v1 = XBoxStorage::MountActiveMU();
    SetLastError(v1);
    if (GetLastError() != eSuccess)
    {
        XBoxStorage::UnmountActiveMU();
        return 0;
    }
    int v3 = (int)(size + 32);
    XBoxStorage::mRoot[0] = XBoxStorage::mDrive;
    unsigned int v4 = (size + 32) % XGetDiskClusterSizeA(XBoxStorage::mRoot);
    if (v4 != 0)
    {
        XBoxStorage::mRoot[0] = XBoxStorage::mDrive;
        v3 += XGetDiskClusterSizeA(XBoxStorage::mRoot) - (int)v4;
    }
    eStatus v5 = XBoxStorage::UnmountActiveMU();
    SetLastError(v5);
    return GetLastError() == eSuccess ? (unsigned int)v3 : 0;
}

// ea: 0x89AA30
unsigned __int64 MemoryUnitManager::BlocksToBytes(unsigned int blocks)
{
    XBoxStorage::mRoot[0] = XBoxStorage::mDrive;
    return (unsigned __int64)blocks * XGetDiskClusterSizeA(XBoxStorage::mRoot);
}

// ea: 0x89AA50
unsigned int MemoryUnitManager::GetClusterSize()
{
    XBoxStorage::MountActiveMU();
    XBoxStorage::mRoot[0] = XBoxStorage::mDrive;
    int v0 = XGetDiskClusterSizeA(XBoxStorage::mRoot);
    XBoxStorage::UnmountActiveMU();
    return (unsigned int)v0;
}

// ea: 0x89AA80
bool MemoryUnitManager::Service()
{
    unsigned char compareBuffer[32];
    unsigned int dwInsertions;
    unsigned int dwRemovals;

    if (XBoxStorage::mAsyncService == nullptr)
    {
        if (mCurrentOperation == 0)
        {
            if (XGetDeviceChanges(XDEVICE_TYPE_MEMORY_UNIT_TABLE,
                                  (DWORD*)&dwInsertions,
                                  (DWORD*)&dwRemovals) != 0
                && mActiveMemoryUnit != 8
                && (dwRemovals
                    & (unsigned int)XBoxStorage::mMasks[GetActiveMemoryUnit()]) != 0
                && mInsertRemoveObserver != nullptr)
            {
                mInsertRemoveObserver->Callback(
                    MemoryUnitManager::eRemoved, mActiveMemoryUnit);
            }
            return false;
        }
        if (XBoxStorage::mAsyncService == nullptr)
            goto LABEL_15;
    }
    {
        eStatus v1 = XBoxStorage::mAsyncService();
        if (v1 == eDeviceBusy)
            return true;
        if (v1 != eSuccess)
        {
            SetLastError(v1);
            eOperation v2 = mCurrentOperation;
            mCurrentOperation = (eOperation)0;
            XBoxStorage::UnmountActiveMU();
            XBoxStorage::Reset();
            mGameSave.Reset(defaultFileName);
            if (mObserver != nullptr)
                mObserver->Callback(v2);
            return false;
        }
    }
LABEL_15:
    SetLastError(eSuccess);
    if (mCurrentOperation == eLoad)
    {
        dwRemovals = 0;
        GetCRC(mBuffer, mNumBytes - 32, compareBuffer, &dwRemovals);
        if (strncmp((const char*)&mBuffer[mNumBytes - 32],
                    (const char*)compareBuffer, dwRemovals) != 0)
        {
            SetLastError(eCRCFailure);
            XBoxStorage::UnmountActiveMU();
            XBoxStorage::Reset();
            mGameSave.Reset(defaultFileName);
            eOperation v4 = mCurrentOperation;
            mCurrentOperation = (eOperation)0;
            if (mObserver != nullptr)
                mObserver->Callback(v4);
            return false;
        }
    }
    return StartOperation();
}

// ea: 0x89AC00
MemoryUnitManager::eStatus MemoryUnitManager::SaveGame(const Container& gameSave)
{
    if (mCurrentOperation != 0)
    {
        SetLastError(eDeviceBusy);
        return GetLastError();
    }
    mGameSave = gameSave;
    mCurrentOperation = eSave;
    eStatus v2 = XBoxStorage::MountActiveMU();
    SetLastError(v2);
    if (GetLastError() != eSuccess)
    {
        XBoxStorage::UnmountActiveMU();
        XBoxStorage::CloseHandles();
        XBoxStorage::mGameNamePath[0] = 0;
        XBoxStorage::mOverlapped.hEvent = nullptr;
        XBoxStorage::mOverlapped.Internal = 0;
        XBoxStorage::mOverlapped.InternalHigh = 0;
        XBoxStorage::mOverlapped.Offset = 0;
        XBoxStorage::mDrive = 63;
        XBoxStorage::mRoot[0] = 0;
        mGameSave.Reset(defaultFileName);
    mCurrentOperation = (eOperation)0;
        return GetLastError();
    }
    const char* GameName = mGameSave.GetGameName();
    eStatus SaveGameStatus = XBoxStorage::CreateSaveGame(GameName);
    SetLastError(SaveGameStatus);
    if (GetLastError() != eSuccess)
    {
        XBoxStorage::UnmountActiveMU();
        XBoxStorage::Reset();
        mGameSave.Reset(defaultFileName);
    mCurrentOperation = (eOperation)0;
        return GetLastError();
    }
    StartOperation();
    return GetLastError();
}

// ea: 0x89ACF0
MemoryUnitManager::eStatus MemoryUnitManager::LoadGame(const Container& gameLoad)
{
    if (mCurrentOperation != 0)
    {
        SetLastError(eDeviceBusy);
        return GetLastError();
    }
    mGameSave = gameLoad;
    mCurrentOperation = eLoad;
    eStatus v2 = XBoxStorage::MountActiveMU();
    SetLastError(v2);
    if (GetLastError() != eSuccess)
    {
        XBoxStorage::UnmountActiveMU();
        XBoxStorage::CloseHandles();
        XBoxStorage::mGameNamePath[0] = 0;
        XBoxStorage::mOverlapped.hEvent = nullptr;
        XBoxStorage::mOverlapped.Internal = 0;
        XBoxStorage::mOverlapped.InternalHigh = 0;
        XBoxStorage::mOverlapped.Offset = 0;
        XBoxStorage::mDrive = 63;
        XBoxStorage::mRoot[0] = 0;
        mGameSave.Reset(defaultFileName);
        mCurrentOperation = (eOperation)0;
        return GetLastError();
    }
    const char* GameName = mGameSave.GetGameName();
    eStatus SaveGameStatus = XBoxStorage::CreateSaveGame(GameName);
    SetLastError(SaveGameStatus);
    if (GetLastError() != eSuccess)
    {
        XBoxStorage::UnmountActiveMU();
        XBoxStorage::Reset();
        mGameSave.Reset(defaultFileName);
        mCurrentOperation = (eOperation)0;
        return GetLastError();
    }
    StartOperation();
    return GetLastError();
}
