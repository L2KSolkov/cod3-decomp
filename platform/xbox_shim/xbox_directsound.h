// Exact Xbox DirectSound declarations used by the reconstructed NVL code.

#pragma once

#include <windows.h>

struct IDirectSound;
struct IDirectSoundBuffer;
struct _DSMIXBINS;

struct _DSEFFECTMAP {
    void* lpvCodeSegment;
    unsigned int dwCodeSize;
    void* lpvStateSegment;
    unsigned int dwStateSize;
    void* lpvYMemorySegment;
    unsigned int dwYMemorySize;
    void* lpvScratchSegment;
    unsigned int dwScratchSize;
};

struct _DSEFFECTIMAGELOC {
    unsigned int dwI3DL2ReverbIndex;
    unsigned int dwCrosstalkIndex;
};

struct _DSEFFECTIMAGEDESC {
    unsigned int dwEffectCount;
    unsigned int dwTotalScratchSize;
    _DSEFFECTMAP aEffectMaps[1];
};

typedef IDirectSound* LPDIRECTSOUND;

struct xbox_adpcmwaveformat_tag {
    tWAVEFORMATEX wfx;
    unsigned short wSamplesPerBlock;
};

struct _DSBUFFERDESC {
    unsigned int dwSize;
    unsigned int dwFlags;
    unsigned int dwBufferBytes;
    tWAVEFORMATEX* lpwfxFormat;
    const _DSMIXBINS* lpMixBins;
    unsigned int dwInputMixBin;
};

static_assert(sizeof(xbox_adpcmwaveformat_tag) == 20,
              "Xbox ADPCM format layout mismatch");
static_assert(sizeof(_DSBUFFERDESC) == 24, "Xbox DSBUFFERDESC layout mismatch");
static_assert(sizeof(_DSEFFECTMAP) == 32, "Xbox DSEFFECTMAP layout mismatch");
static_assert(sizeof(_DSEFFECTIMAGELOC) == 8, "Xbox DSEFFECTIMAGELOC layout mismatch");
static_assert(sizeof(_DSEFFECTIMAGEDESC) == 40, "Xbox DSEFFECTIMAGEDESC layout mismatch");

extern "C" {
HRESULT __stdcall j_DirectSoundCreate(LPCGUID pcGuidDevice,
                                      LPDIRECTSOUND* ppDS,
                                      LPUNKNOWN pUnkOuter);
HRESULT __stdcall j_IDirectSound_DownloadEffectsImage(
    IDirectSound* pDirectSound,
    const void* pvImageBuffer,
    unsigned int dwImageSize,
    const _DSEFFECTIMAGELOC* pImageLoc,
    _DSEFFECTIMAGEDESC** ppImageDesc);
int __stdcall j_IDirectSound_SetDistanceFactor(IDirectSound* pDirectSound,
                                                float flDistanceFactor,
                                                int dwApply);
HRESULT __stdcall j_IDirectSound_EnableHeadphones(IDirectSound* pDirectSound,
                                                   int fEnabled);
int __stdcall j_DirectSoundUseLightHRTF(void);
HRESULT __stdcall j_IDirectSound_CreateSoundBuffer(IDirectSound* pDirectSound,
                                                    const _DSBUFFERDESC* pdsbd,
                                                    IDirectSoundBuffer** ppBuffer,
                                                    IUnknown* pUnkOuter);
HRESULT __stdcall j_IDirectSoundBuffer_GetCurrentPosition(
    IDirectSoundBuffer* pBuffer, unsigned int* pdwPlayCursor, unsigned int* pdwWriteCursor);
HRESULT __stdcall j_IDirectSoundBuffer_GetStatus(IDirectSoundBuffer* pBuffer,
                                                 unsigned int* pdwStatus);
HRESULT __stdcall j_IDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer* pBuffer,
                                                           unsigned int dwPlayCursor);
HRESULT __stdcall j_IDirectSoundBuffer_PlayEx(IDirectSoundBuffer* pBuffer,
                                              __int64 rtTimeStamp,
                                              unsigned int dwFlags);
HRESULT __stdcall j_IDirectSoundBuffer_Stop(IDirectSoundBuffer* pBuffer);
HRESULT __stdcall j_IDirectSoundBuffer_SetBufferData(IDirectSoundBuffer* pBuffer,
                                                      void* pvBufferData,
                                                      unsigned int dwBufferBytes);
HRESULT __stdcall j_IDirectSoundBuffer_SetPlayRegion(IDirectSoundBuffer* pBuffer,
                                                     unsigned int dwPlayStart,
                                                     unsigned int dwPlayLength);
HRESULT __stdcall j_IDirectSoundBuffer_SetLoopRegion(IDirectSoundBuffer* pBuffer,
                                                     unsigned int dwLoopStart,
                                                     unsigned int dwLoopLength);
}
