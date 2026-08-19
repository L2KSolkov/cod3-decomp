// Exact Xbox DirectSound declarations used by the reconstructed NVL code.

#pragma once

#include <windows.h>

struct IDirectSound;
struct IDirectSoundBuffer;
struct IDirectSoundStream {
    void* vtbl;
};
struct _DSMIXBINS;

// IDA's XMediaObject/XFileMediaObject interfaces are four-byte vtable
// objects. Keep the exact Xbox ABI here; the Win32 implementation remains a
// shim boundary until the audio backend is ported.
struct _XMEDIAINFO;
struct _XMEDIAPACKET;
struct XMediaObject;
struct XFileMediaObject;
struct XMediaObject_vtbl {
    unsigned int (__stdcall *AddRef)(XMediaObject*);
    unsigned int (__stdcall *Release)(XMediaObject*);
    HRESULT (__stdcall *GetInfo)(XMediaObject*, _XMEDIAINFO*);
    HRESULT (__stdcall *GetStatus)(XMediaObject*, unsigned int*);
    HRESULT (__stdcall *Process)(XMediaObject*, const _XMEDIAPACKET*, const _XMEDIAPACKET*);
    HRESULT (__stdcall *Discontinuity)(XMediaObject*);
    HRESULT (__stdcall *Flush)(XMediaObject*);
};
struct XMediaObject {
    XMediaObject_vtbl* __vftable;
};
struct XFileMediaObject_vtbl {
    unsigned int (__stdcall *AddRef)(XFileMediaObject*);
    unsigned int (__stdcall *Release)(XFileMediaObject*);
    HRESULT (__stdcall *GetInfo)(XFileMediaObject*, _XMEDIAINFO*);
    HRESULT (__stdcall *GetStatus)(XFileMediaObject*, unsigned int*);
    HRESULT (__stdcall *Process)(XFileMediaObject*, const _XMEDIAPACKET*, const _XMEDIAPACKET*);
    HRESULT (__stdcall *Discontinuity)(XFileMediaObject*);
    HRESULT (__stdcall *Flush)(XFileMediaObject*);
    HRESULT (__stdcall *Seek)(XFileMediaObject*, int, unsigned int, unsigned int*);
    HRESULT (__stdcall *GetLength)(XFileMediaObject*, unsigned int*);
    void (__stdcall *DoWork)(XFileMediaObject*);
};
struct XFileMediaObject {
    XFileMediaObject_vtbl* __vftable;
};

// IDA's Xbox WAVEFORMATEXTENSIBLE uses the Windows tWAVEFORMATEX layout;
// keep a distinct name because the Win32 multimedia headers do not expose
// the Xbox typedef under WAVEFORMATEXTENSIBLE in every translation unit.
struct xbox_WAVEFORMATEXTENSIBLE {
    tWAVEFORMATEX Format;
    unsigned short Samples;
    unsigned int dwChannelMask;
    GUID SubFormat;
};

struct _DSENVELOPEDESC {
    unsigned int dwEG;
    unsigned int dwMode;
    unsigned int dwDelay;
    unsigned int dwAttack;
    unsigned int dwHold;
    unsigned int dwDecay;
    unsigned int dwRelease;
    unsigned int dwSustain;
    int lPitchScale;
    int lFilterCutOff;
};

struct _DSSTREAMDESC {
    unsigned int dwFlags;
    unsigned int dwMaxAttachedPackets;
    tWAVEFORMATEX* lpwfxFormat;
    void (__stdcall *lpfnCallback)(void*, void*, unsigned int);
    void* lpvContext;
    const _DSMIXBINS* lpMixBins;
};

struct _DSI3DL2LISTENER {
    int lRoom;
    int lRoomHF;
    float flRoomRolloffFactor;
    float flDecayTime;
    float flDecayHFRatio;
    int lReflections;
    float flReflectionsDelay;
    int lReverb;
    float flReverbDelay;
    float flDiffusion;
    float flDensity;
    float flHFReference;
};

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
static_assert(sizeof(XMediaObject) == 4, "Xbox XMediaObject layout mismatch");
static_assert(sizeof(XFileMediaObject) == 4,
              "Xbox XFileMediaObject layout mismatch");
static_assert(sizeof(tWAVEFORMATEX) == 18, "Xbox WAVEFORMATEX layout mismatch");
static_assert(sizeof(xbox_WAVEFORMATEXTENSIBLE) == 40,
              "Xbox WAVEFORMATEXTENSIBLE layout mismatch");
static_assert(sizeof(_DSENVELOPEDESC) == 40, "Xbox DSENVELOPEDESC layout mismatch");
static_assert(sizeof(_DSSTREAMDESC) == 24, "Xbox DSSTREAMDESC layout mismatch");
static_assert(sizeof(_DSI3DL2LISTENER) == 48,
              "Xbox DSI3DL2LISTENER layout mismatch");
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
HRESULT __stdcall j_DirectSoundCreateStream(const _DSSTREAMDESC* pdssd,
                                             IDirectSoundStream** ppStream);
HRESULT __stdcall j_DirectSoundCreateBuffer(const _DSBUFFERDESC* pdsbd,
                                             IDirectSoundBuffer** ppBuffer);
HRESULT __stdcall j_IDirectSound_SetI3DL2Listener(
    IDirectSound* pDirectSound, const _DSI3DL2LISTENER* pds3dl, unsigned int dwFlags);
HRESULT __stdcall j_IDirectSoundStream_SetHeadroom(IDirectSoundStream* pStream,
                                                    unsigned int dwHeadroom);
HRESULT __stdcall j_IDirectSoundStream_SetMode(IDirectSoundStream* pStream,
                                                unsigned int dwMode,
                                                unsigned int dwFlags);
HRESULT __stdcall j_IDirectSoundStream_SetEG(
    IDirectSoundStream* pStream, const _DSENVELOPEDESC* pEnvelopeDesc);
void __stdcall j_XAudioCreatePcmFormat(unsigned short nChannels,
                                       unsigned int nSamplesPerSec,
                                       unsigned short wBitsPerSample,
                                       tWAVEFORMATEX* pwfx);
void __stdcall j_XAudioCreateAdpcmFormat(unsigned short nChannels,
                                         unsigned int nSamplesPerSec,
                                         xbox_adpcmwaveformat_tag* pwfx);
HRESULT __stdcall j_IDirectSoundStream_SetFormat(
    IDirectSoundStream* pStream, const tWAVEFORMATEX* pwfxFormat);
HRESULT __stdcall j_IDirectSoundStream_SetMixBins(
    IDirectSoundStream* pStream, const _DSMIXBINS* pMixBins);
HRESULT __stdcall j_XFileCreateMediaObjectAsync(
    void* hFile, unsigned int dwMaxPackets, XFileMediaObject** ppMediaObject);
HRESULT __stdcall j_IDirectSoundBuffer_SetHeadroom(IDirectSoundBuffer* pBuffer,
                                                    unsigned int dwHeadroom);
HRESULT __stdcall j_IDirectSoundBuffer_SetMode(IDirectSoundBuffer* pBuffer,
                                                unsigned int dwMode,
                                                unsigned int dwFlags);
HRESULT __stdcall j_IDirectSoundBuffer_SetEG(
    IDirectSoundBuffer* pBuffer, const _DSENVELOPEDESC* pEnvelopeDesc);
HRESULT __stdcall j_IDirectSoundBuffer_SetFormat(
    IDirectSoundBuffer* pBuffer, const tWAVEFORMATEX* pwfxFormat);
HRESULT __stdcall j_IDirectSoundBuffer_SetMixBins(
    IDirectSoundBuffer* pBuffer, const _DSMIXBINS* pMixBins);
HRESULT __stdcall j_IDirectSoundBuffer_SetVolume(IDirectSoundBuffer* pBuffer,
                                                 int lVolume);
extern unsigned int g_dwDirectSoundDebugBreakLevel;
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
