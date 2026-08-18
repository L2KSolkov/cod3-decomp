# Xbox API → Win32 Shim Needs Tracker

This file tracks Xbox API calls found in game/engine code that need
Win32 implementations in `platform/xbox_shim/`.

Format: `XboxAPI | caller_count | Win32_replacement`

## Kernel (xboxkrnl)

| Xbox API | Callers (est.) | Win32 Replacement |
|---|---|---|
| RtlEnterCriticalSection | many | EnterCriticalSection |
| RtlLeaveCriticalSection | many | LeaveCriticalSection |
| RtlInitializeCriticalSection | many | InitializeCriticalSection |
| KeQueryPerformanceCounter | many | QueryPerformanceCounter |
| KeQueryPerformanceFrequency | many | QueryPerformanceFrequency |
| KeQuerySystemTime | several | GetSystemTimeAsFileTime |
| KeTickCount | several | GetTickCount |
| KeDelayExecutionThread | several | Sleep |
| KeSetEvent | several | SetEvent |
| KeWaitForSingleObject | many | WaitForSingleObject |
| KeWaitForMultipleObjects | several | WaitForMultipleObjects |
| KeInitializeTimerEx | several | CreateWaitableTimer |
| KeSetTimer / KeSetTimerEx | several | SetWaitableTimer |
| KeCancelTimer | several | CancelWaitableTimer |
| PsCreateSystemThreadEx | several | CreateThread |
| PsTerminateSystemThread | several | ExitThread |
| NtCreateEvent | several | CreateEvent |
| NtSetEvent | several | SetEvent |
| NtClearEvent | several | ResetEvent |
| NtCreateSemaphore | several | CreateSemaphore |
| NtReleaseSemaphore | several | ReleaseSemaphore |
| NtCreateMutant | several | CreateMutex |
| NtReleaseMutant | several | ReleaseMutex |
| NtClose | many | CloseHandle |
| NtWaitForSingleObject | several | WaitForSingleObject |
| NtWaitForMultipleObjects | several | WaitForMultipleObjects |
| KeRaiseIrqlToDpcLevel / KfLowerIrql / KfRaiseIrql | several | NOP (no kernel mode) |
| KeStallExecutionProcessor | several | _mm_pause() or spin loop |
| KeSaveFloatingPointState | few | _fxsave / compiler intrinsic |
| KeRestoreFloatingPointState | few | _fxrstor / compiler intrinsic |
| KeBugCheck | few | __debugbreak() + ExitProcess |
| RtlRaiseException | few | RaiseException |
| HalReturnToFirmware | few | ExitProcess |
| DbgPrint | many | OutputDebugStringA |

## Memory (xboxkrnl + xapilibd)

| Xbox API | Win32 Replacement |
|---|---|
| XMemAlloc | malloc |
| XMemFree | free |
| XMemSize | _msize |
| MmAllocateContiguousMemory | VirtualAlloc |
| MmFreeContiguousMemory | VirtualFree |
| MmAllocateContiguousMemoryEx | VirtualAlloc with guard pages |
| MmPersistContiguousMemory | NOP (no XBE-loader persist) |
| MmGetPhysicalAddress | return 0 |
| ExAllocatePoolWithTag | malloc |
| ExFreePool | free |

## File I/O (xboxkrnl)

| Xbox API | Win32 Replacement |
|---|---|
| NtCreateFile | CreateFileA |
| NtOpenFile | CreateFileA (OPEN_EXISTING) |
| NtReadFile | ReadFile |
| NtWriteFile | WriteFile |
| NtClose | CloseHandle |
| NtSetInformationFile | SetFilePointer + SetEndOfFile |
| NtQueryInformationFile | GetFileSize / GetFileInformationByHandle |
| NtQueryVolumeInformationFile | GetVolumeInformation |
| NtQueryDirectoryFile | FindFirstFile / FindNextFile |
| NtFlushBuffersFile | FlushFileBuffers |
| NtDeleteFile | DeleteFileA |

## Graphics (d3d8d, xgraphicsd, d3dx8d)

| Xbox API | Win32 Replacement |
|---|---|
| D3DDevice_* | D3D11 equivalent via NGL backend |
| D3DDevice_SetRenderTarget | D3D11 OMSetRenderTargets (called from ngl_dx_draw.o) |
| D3DTexture_GetSurfaceLevel2 | D3D11 texture-to-RTV/SRV surface (ngl_dx_draw.o) |
| D3DCubeTexture_GetCubeMapSurface2 | D3D11 cube face subresource (ngl_dx_draw.o) |
| D3DResource_Release | Release (ngl_dx_draw.o) |
| XGRPH swizzled textures | D3D11 texture creation |
| D3DX math (Vec3, Matrix, Quat) | DirectXMath library |
| Xbox push buffers | NOP (D3D11 manages GPU ring buffer) |

## Sound (dsoundd, dmusicd)

| Xbox API | Win32 Replacement |
|---|---|
| IDirectSound* | XAudio2 via IXAudio2 |
| NVL `j_DirectSoundCreate` and `j_IDirectSound*` calls | Current Win32 shim preserves the exact ABI and returns `E_NOTIMPL`; map to XAudio2 before enabling movie audio |
| DSound 3D positioning | X3DAudio |
| Dolby encoder | Multichannel PCM (no encoding needed) |

## Networking (xonlinesd, xnet)

| Xbox API | Win32 Replacement |
|---|---|
| XNet* | WinSock2 |
| XOnline* | Stub (return offline); Steamworks later |

## Input (xapilibd controller)

| Xbox API | Win32 Replacement |
|---|---|
| XInputGetState (legacy XDK) | XInputGetState (modern) |
| XInputGetCapabilities | XInputGetCapabilities |
| XID driver | stub (use modern XInput) |

## Voice (xvoiced)

| Xbox API | Win32 Replacement |
|---|---|
| XVoice* | NOP (no voice for Phase 0) |

## Other

| Xbox API | Win32 Replacement |
|---|---|
| XGetTickCount | GetTickCount |
| XMountUtilityDrive | return 0 |
| XeImageFileName | GetModuleFileName |
| XCalculateSignature* | return 0 (load-time signing not needed) |

## Memory unit / save games (peripherals_xboxr) - IMPLEMENTED in xbox_shim.cpp

`peripherals_xboxr` was ported (81/81) onto the Win32 file system. The logical
MU root (`U`/`F`) maps onto `%USERPROFILE%\.cod3\MemoryUnit\`; all of these are
implemented in `platform/xbox_shim/xbox_shim.cpp`:

| Xbox API | Win32 Replacement |
|---|---|
| XGetDevices / XGetDeviceChanges | return 0 (only device 8 / hard drive exists) |
| XMountMUA / XUnmountMU | set drive 'U', no-op |
| XCreateSaveGame | CreateDirectoryA under the save root |
| XDeleteSaveGame | delete save directory + contents |
| XFindFirstSaveGame / XFindNextSaveGame / XFindClose | FindFirstFileA dir enumeration |
| XGetDisplayBlocks | total file bytes / 16384 rounded up |
| XGetDiskSectorSizeA / XGetDiskClusterSizeA | 512 / 16384 |
| XCalculateSignatureBegin/Update/End | deterministic 20-byte CRC digest |

## Xbox Live / matchmaking (game_xbox.o LiveWrapper.cpp + xboxMatch.cpp)

`CSession` / `LiveWrapper` / live-player classes are ported (XboxLive.cpp);
they call the Xbox Live surface verbatim. Declarations in
`platform/xbox_shim/xlive.h`; implementations pending (link via /FORCE).

| Xbox API | Win32 Replacement |
|---|---|
| XOnlineMatchSessionCreate / Update / Delete | stub (offline; create returns failure) |
| XOnlineMatchSessionGetInfo | stub |
| XOnlineTaskContinue / XOnlineTaskClose | stub (task completes immediately) |
| XOnlineMutelistGet | stub (empty mute list) |
| XNetQosListen / XNetRegisterKey / XNetUnregisterKey | stub (return 0) |
| LiveEngine_NotificationSetState / SetProperty / Release | stub (no-op) |
| XHVEngine_IsTalking / XHVEngine_SetPlaybackPriority | stub (return 0) |
| ITitleFontRenderer/ITitleUIPlugin/ITitleAudioPlugin::Release | stub (no-op) |
