// ============================================================================
// MemoryUnitManager.h - Xbox memory-unit save/load manager
// Source: peripherals_xboxr:MemoryUnit.o + XboxMemoryUnit.o
// Reconstructed from IDA local types (PDB symbol data). All sizes verified.
// Win32 port: the logical memory unit maps onto %USERPROFILE%\.cod3\MemoryUnit\.
// ============================================================================

#pragma once

#include <stdint.h>

// ============================================================================
// MemoryUnitManager - static-class save/load peripheral manager
// ============================================================================
class MemoryUnitManager
{
public:
    enum eStatus
    {
        eSuccess = 0,
        eGenericFailure = -1,
        eUnformatted = -2,
        eMediumFull = -3,
        eNotEnoughFreeSpace = -4,
        eNoReadPermissions = -5,
        eNoWritePermissions = -6,
        eIncorrectMedium = -7,
        eTooManyOpenFiles = -8,
        eIncorrectPadding = -9,
        eNoMedium = -10,
        eDamagedMedium = -11,
        eCRCFailure = -12,
        eFileDoesNotExist = -13,
        eFileAlreadyExists = -14,
        eDirNotEmpty = -15,
        eEmptyString = -16,
        eDeviceBusy = -17,
        eInvalidDeviceID = -18,
        eQueueFull = -500,
        eQueueUnderflow = -501,
        eInvalidIconIndex = -502,
        eNoTitlePrefixSet = -503,
        eInitializeNotCalled = -504,
    };

    enum eOperation
    {
        eLoad = 1,
        eSave = 2,
        eDelete = 3,
        eFormat = 4,
    };

    struct Observer
    {
        virtual void Callback(eOperation op) = 0;
    };

    struct InsertRemoveObserver
    {
        enum eChange
        {
            eRemoved = 0,
            eInserted = 1,
        };
        virtual void Callback(eChange change, int deviceID) = 0;
    };

    // A named save game: up to 8 files per container. Size 0x288.
    struct Container
    {
        char mFileNames[8][64];      // +0x000
        unsigned char* mBuffers[8];  // +0x200
        unsigned int mNumBytes[8];   // +0x220
        unsigned int mCurrentFile;   // +0x240
        unsigned int mIndex;         // +0x244
        char mGameName[64];          // +0x248

        Container(const char* gameName);
        void Reset(const char* gameName);
        bool AddFile(const char* fileName, unsigned char* buffer,
                     unsigned int numBytes);
        int GetTotalBytes();
        int GetNumFiles() const;
        unsigned char* GetFile(int idx);
        bool GetNextFile(char* fileName, unsigned char** buffer,
                         unsigned int* numBytes);
        const char* GetGameName() const;
    };

    // One listed save game. Size 0x4C.
    struct SavedGame
    {
        struct ModifyTime
        {
            unsigned char second;  // +0x00
            unsigned char minute;  // +0x01
            unsigned char hour;    // +0x02
            unsigned char day;     // +0x03
            unsigned char month;   // +0x04
            unsigned char pad;     // +0x05
            unsigned short year;   // +0x06
        };
        char name[64];            // +0x00
        unsigned int size;        // +0x40
        ModifyTime mt;            // +0x44
    };

    // Storage volume info. Size 0x20.
    struct MemoryUnitInfo
    {
        unsigned __int64 bytesFree;  // +0x00
        unsigned __int64 maxBytes;   // +0x08
        unsigned int blocksFree;     // +0x10
        unsigned int maxBlocks;      // +0x14
        unsigned int formated;       // +0x18
        unsigned int filesFree;      // +0x1C
    };

    // Class statics (defined in MemoryUnit.cpp / XBoxStorage.cpp).
    static eStatus mLastError;
    static char mTitlePrefix[64];
    static int mActiveMemoryUnit;
    static unsigned char* mBuffer;
    static unsigned int mNumBytes;
    static eOperation mCurrentOperation;
    static Container mGameSave;
    static char mDisplayStrings[9][64];
    static Observer* mObserver;
    static InsertRemoveObserver* mInsertRemoveObserver;
    static unsigned int mInitFlags;

    static eStatus GetLastError();
    static void SetLastError(eStatus lastError);
    static void SetTitlePrefix(const char* titlePrefix);
    static const char* GetTitlePrefix();
    static void SetActiveMemoryUnit(int deviceID);
    static int GetActiveMemoryUnit();
    static void RegisterObserver(Observer* callBack);
    static void RegisterInsertRemoveObserver(InsertRemoveObserver* callBack);
    static Observer* GetObserver();
    static InsertRemoveObserver* GetInsertRemoveObserver();
    static unsigned char* GetBuffer();
    static unsigned int GetNumBytes();
    static eOperation GetCurrentOperation();
    static void SetCurrentOperation(eOperation op);
    static void SetDisplayStrings(const char (*displayStrings)[64]);
    static const char* GetDisplayString();
    static int FindFirstMemoryUnit();
    static int FindNextMemoryUnit();
    static int FindPrevMemoryUnit();
    static eStatus SaveGameSync(const Container& gameSave);
    static eStatus LoadGameSync(const Container& gameLoad);
    static eStatus DeleteGameSync(const char* gameName);
    static eStatus FormatSync();
    static const char* StatusToString(eStatus status);
    static int GetFirstDeviceID();
    static int GetLastDeviceID();
    static eStatus Format();
    static eStatus Initialize(unsigned int initFlags);
    static int GetAvailableMemoryUnits(int* storageDevices);
    static void GetCRC(unsigned char* buffer, unsigned int size,
                       unsigned char* crcBuffer, unsigned int* crcSize);
    static float GetProgress();
    static unsigned int BytesToBlocks(unsigned __int64 bytes);
    static unsigned __int64 BlocksToBytes(unsigned int blocks);
    static unsigned int GetGameSaveSize(unsigned int size);
    static unsigned int GetClusterSize();
    static bool Service();
    static eStatus SaveGame(const Container& gameSave);
    static eStatus LoadGame(const Container& gameLoad);
    static eStatus DeleteGame(const char* gameName);
    static int GetSavedGames(SavedGame* savedGames);
    static eStatus GetMemoryUnitInfo(MemoryUnitInfo* storageInfo);

private:
    MemoryUnitManager();
    ~MemoryUnitManager();
    static eStatus Save(const char* fileName, unsigned char* saveBuffer,
                        unsigned int numBytesToSave);
    static eStatus Load(const char* fileName, unsigned char* loadBuffer,
                        int numBytesToLoad);
    static eStatus Delete(const char* gameName);
    static bool StartOperation();
};

static_assert(sizeof(MemoryUnitManager::Container) == 0x288,
              "MemoryUnitManager::Container size mismatch");
static_assert(sizeof(MemoryUnitManager::SavedGame) == 0x4C,
              "MemoryUnitManager::SavedGame size mismatch");
static_assert(sizeof(MemoryUnitManager::SavedGame::ModifyTime) == 0x8,
              "MemoryUnitManager::SavedGame::ModifyTime size mismatch");
static_assert(sizeof(MemoryUnitManager::MemoryUnitInfo) == 0x20,
              "MemoryUnitManager::MemoryUnitInfo size mismatch");
