// ============================================================================
// XBoxStorage.h - filesystem backing for the memory-unit manager
// Source: peripherals_xboxr:XboxMemoryUnit.o
// Reconstructed from IDA; Win32 port maps the MU drive onto the file system.
// ============================================================================

#pragma once

#include "MemoryUnitManager.h"
#include <windows.h>

class XBoxStorage
{
public:
    static HANDLE mFileHandle;
    static char mGameNamePath[260];
    static OVERLAPPED mOverlapped;
    static char mDrive;    // 63 '?' unmounted, 85 'U' mounted (device 8)
    static char mRoot[3];
    static MemoryUnitManager::eStatus (*mAsyncService)();
    static int mMasks[9];

    static MemoryUnitManager::eStatus PortSlotToDeviceID(int* devID,
        unsigned int port, unsigned int slot);
    static HANDLE GetFileHandle();
    static const char* GetGameNamePath();
    static void ResetOverlapped();
    static MemoryUnitManager::eStatus (*GetAsyncService())();
    static char GetDrive();
    static const char* GetRoot();
    static void CloseHandles();
    static void Reset();
    static MemoryUnitManager::eStatus ConvertError(int returnValue);
    static MemoryUnitManager::eStatus MountActiveMU();
    static MemoryUnitManager::eStatus UnmountActiveMU();
    static MemoryUnitManager::eStatus CreateSaveGame(const char* gameName);
    static MemoryUnitManager::eStatus DeleteSaveGame(const char* gameName);
    static MemoryUnitManager::eStatus OpenFile(const char* fileName,
                                               unsigned int access);
    static MemoryUnitManager::eStatus AsyncWriteService();
    static MemoryUnitManager::eStatus AsyncReadService();
    static int GetSectorSize();
    static int GetClusterSize();
    static int GetAsyncChunk();
    static MemoryUnitManager::eStatus BeginAsyncWrite();
    static MemoryUnitManager::eStatus BeginAsyncRead();
    static MemoryUnitManager::eStatus DeviceIDToPortSlot(int devID,
        unsigned int* port, unsigned int* slot);

private:
    XBoxStorage();
    ~XBoxStorage();
};
