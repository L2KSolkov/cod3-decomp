// Win32 ownership of the Xbox DirectSound entry points used by NVL.
// A real XAudio2 backend remains a later platform phase; these declarations
// preserve the Xbox call surface while making the unavailable device explicit.

#include "xbox_directsound.h"

extern "C" HRESULT __stdcall j_DirectSoundCreate(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN) {
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
