// Exact Xbox DirectSound declarations used by the reconstructed NVL code.

#pragma once

#include <windows.h>

struct IDirectSound;
struct IDirectSoundBuffer;
struct _DSMIXBINS;

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

extern "C" {
HRESULT __stdcall j_DirectSoundCreate(LPCGUID pcGuidDevice,
                                      LPDIRECTSOUND* ppDS,
                                      LPUNKNOWN pUnkOuter);
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
