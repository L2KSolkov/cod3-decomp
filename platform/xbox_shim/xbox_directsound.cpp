// Win32 ownership of the Xbox DirectSound entry points used by NVL.
// A real XAudio2 backend remains a later platform phase; these declarations
// preserve the Xbox call surface while making the unavailable device explicit.

#include "xbox_directsound.h"

unsigned int g_dwDirectSoundDebugBreakLevel = 0;

extern "C" HRESULT __stdcall j_DirectSoundCreate(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSound_DownloadEffectsImage(
    IDirectSound*, const void*, unsigned int, const _DSEFFECTIMAGELOC*, _DSEFFECTIMAGEDESC**) {
    return E_NOTIMPL;
}

extern "C" int __stdcall j_IDirectSound_SetDistanceFactor(IDirectSound*, float, int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSound_EnableHeadphones(IDirectSound*, int) {
    return E_NOTIMPL;
}

extern "C" int __stdcall j_DirectSoundUseLightHRTF(void) {
    return 0;
}

extern "C" HRESULT __stdcall j_DirectSoundCreateStream(
    const _DSSTREAMDESC*, IDirectSoundStream**) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_DirectSoundCreateBuffer(
    const _DSBUFFERDESC*, IDirectSoundBuffer**) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSound_SetI3DL2Listener(
    IDirectSound*, const _DSI3DL2LISTENER*, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_SetHeadroom(
    IDirectSoundStream*, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_SetMode(
    IDirectSoundStream*, unsigned int, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_SetEG(
    IDirectSoundStream*, const _DSENVELOPEDESC*) {
    return E_NOTIMPL;
}

extern "C" void __stdcall j_XAudioCreatePcmFormat(
    unsigned short, unsigned int, unsigned short, tWAVEFORMATEX*) {}

extern "C" void __stdcall j_XAudioCreateAdpcmFormat(
    unsigned short, unsigned int, xbox_adpcmwaveformat_tag*) {}

extern "C" HRESULT __stdcall j_IDirectSoundStream_SetFormat(
    IDirectSoundStream*, const tWAVEFORMATEX*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_SetMixBins(
    IDirectSoundStream*, const _DSMIXBINS*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_XFileCreateMediaObjectAsync(
    void*, unsigned int, XFileMediaObject**) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetHeadroom(
    IDirectSoundBuffer*, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetMode(
    IDirectSoundBuffer*, unsigned int, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetEG(
    IDirectSoundBuffer*, const _DSENVELOPEDESC*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetFormat(
    IDirectSoundBuffer*, const tWAVEFORMATEX*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetMixBins(
    IDirectSoundBuffer*, const _DSMIXBINS*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetVolume(
    IDirectSoundBuffer*, int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_FlushEx(
    IDirectSoundStream*, __int64, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_StopEx(
    IDirectSoundBuffer*, __int64, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_Play(
    IDirectSoundBuffer*, unsigned int, unsigned int, unsigned int) {
    return E_NOTIMPL;
}

extern "C" int __stdcall j_IDirectSoundStream_SetMinDistance(
    IDirectSoundStream*, float, int) {
    return E_NOTIMPL;
}

extern "C" int __stdcall j_IDirectSoundStream_SetMaxDistance(
    IDirectSoundStream*, float, int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_SetRolloffCurve(
    IDirectSoundStream*, const float*, unsigned int, unsigned int) {
    return E_NOTIMPL;
}

extern "C" int __stdcall j_IDirectSoundBuffer_SetMinDistance(
    IDirectSoundBuffer*, float, int) {
    return E_NOTIMPL;
}

extern "C" int __stdcall j_IDirectSoundBuffer_SetMaxDistance(
    IDirectSoundBuffer*, float, int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetRolloffCurve(
    IDirectSoundBuffer*, const float*, unsigned int, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_Pause(
    IDirectSoundStream*, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_Pause(
    IDirectSoundBuffer*, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_SetAllParameters(
    IDirectSoundStream*, const _DS3DBUFFER*, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetAllParameters(
    IDirectSoundBuffer*, const _DS3DBUFFER*, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_SetVolume(
    IDirectSoundStream*, int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundStream_SetFrequency(
    IDirectSoundStream*, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetFrequency(
    IDirectSoundBuffer*, unsigned int) {
    return E_NOTIMPL;
}

extern "C" int __stdcall j_IDirectSound_SetPosition(
    IDirectSound*, float, float, float, int) {
    return E_NOTIMPL;
}

extern "C" int __stdcall j_IDirectSound_SetVelocity(
    IDirectSound*, float, float, float, int) {
    return E_NOTIMPL;
}

extern "C" int __stdcall j_IDirectSound_SetOrientation(
    IDirectSound*, float, float, float, float, float, float, int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSound_CommitDeferredSettings(
    IDirectSound*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSound_SynchPlayback(IDirectSound*) {
    return E_NOTIMPL;
}

extern "C" int __cdecl j_DirectSoundDoWork(
    unsigned int, unsigned int, unsigned int, unsigned int,
    unsigned int, unsigned int, unsigned int, unsigned int,
    unsigned int, unsigned int, unsigned int, unsigned int,
    unsigned int, unsigned int, unsigned int, unsigned int,
    unsigned int, unsigned int, unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSound_CreateSoundBuffer(
    IDirectSound*, const _DSBUFFERDESC*, IDirectSoundBuffer**, IUnknown*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_GetCurrentPosition(
    IDirectSoundBuffer*, unsigned int*, unsigned int*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_GetStatus(IDirectSoundBuffer*, unsigned int*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer*,
                                                                        unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_PlayEx(IDirectSoundBuffer*,
                                                           __int64,
                                                           unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_Stop(IDirectSoundBuffer*) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetBufferData(IDirectSoundBuffer*,
                                                                   void*,
                                                                   unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetPlayRegion(IDirectSoundBuffer*,
                                                                  unsigned int,
                                                                  unsigned int) {
    return E_NOTIMPL;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetLoopRegion(IDirectSoundBuffer*,
                                                                  unsigned int,
                                                                  unsigned int) {
    return E_NOTIMPL;
}
