// Win32 audio backend for the Xbox DirectSound surface used by NSL.
// The game still speaks the release Xbox ABI; this file translates those
// packets to the Windows multimedia wave output device.

#include "xbox_directsound.h"

#define WIN32_LEAN_AND_MEAN
#include <mmsystem.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <vector>

#pragma comment(lib, "winmm.lib")

unsigned int g_dwDirectSoundDebugBreakLevel = 0;

namespace {

int clampInt(int value, int minimum, int maximum) {
    return value < minimum ? minimum : (value > maximum ? maximum : value);
}
unsigned minUnsigned(unsigned left, unsigned right) {
    return left < right ? left : right;
}

static const int kImaIndexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};
static const int kImaStepTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int clampSample(int value) {
    return value < -32768 ? -32768 : (value > 32767 ? 32767 : value);
}

static int xboxAdpcmDelta(int step, unsigned nibble) {
    int delta = step >> 3;
    if ((nibble & 1u) != 0)
        delta += step >> 2;
    if ((nibble & 2u) != 0)
        delta += step >> 1;
    if ((nibble & 4u) != 0)
        delta += step;
    return (nibble & 8u) != 0 ? -delta : delta;
}

// Xbox ADPCM uses the IMA predictor/index tables with 36 bytes per channel
// block: a four-byte channel header followed by eight four-byte nibble chunks.
// Stereo blocks interleave one chunk for each channel after both headers.
static bool decodeXboxAdpcm(const unsigned char* input, unsigned size,
                            unsigned channels, unsigned blockAlign,
                            std::vector<unsigned char>& output) {
    if (input == nullptr || channels == 0 || channels > 2 ||
        blockAlign != 36u * channels || size == 0 || size % blockAlign != 0)
        return false;

    constexpr unsigned kSamplesPerBlock = 65;
    constexpr unsigned kChunksPerBlock = 8;
    constexpr unsigned kSamplesPerChunk = 8;
    const unsigned blockCount = size / blockAlign;
    output.clear();
    output.reserve(static_cast<size_t>(blockCount) * kSamplesPerBlock *
                   channels * sizeof(short));
    for (unsigned block = 0; block < blockCount; ++block) {
        const unsigned char* bytes = input + block * blockAlign;
        int samples[2][kSamplesPerBlock] = {};
        int indices[2] = {};
        unsigned cursor = 0;
        for (unsigned channel = 0; channel < channels; ++channel) {
            samples[channel][0] = static_cast<short>(
                static_cast<unsigned short>(bytes[cursor]) |
                (static_cast<unsigned short>(bytes[cursor + 1]) << 8));
            const unsigned headerIndex =
                static_cast<unsigned>(bytes[cursor + 2]) |
                (static_cast<unsigned>(bytes[cursor + 3]) << 8);
            indices[channel] = std::min<unsigned>(headerIndex, 88u);
            cursor += 4;
        }
        for (unsigned chunk = 0; chunk < kChunksPerBlock; ++chunk) {
            for (unsigned channel = 0; channel < channels; ++channel) {
                unsigned packed = static_cast<unsigned>(bytes[cursor]) |
                    (static_cast<unsigned>(bytes[cursor + 1]) << 8) |
                    (static_cast<unsigned>(bytes[cursor + 2]) << 16) |
                    (static_cast<unsigned>(bytes[cursor + 3]) << 24);
                cursor += 4;
                for (unsigned sample = 0; sample < kSamplesPerChunk; ++sample) {
                    const unsigned nibble = packed & 0xFu;
                    const int step = kImaStepTable[indices[channel]];
                    samples[channel][1 + chunk * kSamplesPerChunk + sample] =
                        clampSample(samples[channel][chunk * kSamplesPerChunk + sample] +
                                    xboxAdpcmDelta(step, nibble));
                    indices[channel] = clampInt(
                        indices[channel] + kImaIndexTable[nibble], 0, 88);
                    packed >>= 4;
                }
            }
        }
        for (unsigned sample = 0; sample < kSamplesPerBlock; ++sample) {
            for (unsigned channel = 0; channel < channels; ++channel) {
                const short value = static_cast<short>(samples[channel][sample]);
                const unsigned char* raw = reinterpret_cast<const unsigned char*>(&value);
                output.push_back(raw[0]);
                output.push_back(raw[1]);
            }
        }
    }
    return true;
}

struct HostWaveBlock {
    WAVEHDR header{};
    std::vector<unsigned char> bytes;
    std::atomic<bool> done{false};
};

class HostWaveOut {
public:
    ~HostWaveOut() { reset(); }

    void setFormat(const tWAVEFORMATEX& format) {
        if (std::memcmp(&m_format, &format, sizeof(format)) != 0) {
            reset();
            m_format = format;
            m_outputFormat = format;
            if (format.wFormatTag == 105u) {
                m_outputFormat.wFormatTag = WAVE_FORMAT_PCM;
                m_outputFormat.wBitsPerSample = 16;
                m_outputFormat.nBlockAlign = static_cast<unsigned short>(
                    format.nChannels * sizeof(short));
                m_outputFormat.nAvgBytesPerSec =
                    format.nSamplesPerSec * m_outputFormat.nBlockAlign;
                m_outputFormat.cbSize = 0;
            }
        }
    }
    void setVolume(int volumeDb) { m_volumeDb = volumeDb; }
    void setFrequency(unsigned frequency) {
        if (frequency != 0 && frequency != m_format.nSamplesPerSec) {
            reset();
            m_format.nSamplesPerSec = frequency;
            m_format.nAvgBytesPerSec =
                frequency * static_cast<unsigned>(m_format.nBlockAlign);
            m_outputFormat.nSamplesPerSec = frequency;
            m_outputFormat.nAvgBytesPerSec =
                frequency * static_cast<unsigned>(m_outputFormat.nBlockAlign);
        }
    }
    bool submit(const void* data, unsigned size) {
        reap();
        std::vector<unsigned char> decoded;
        const unsigned char* source = static_cast<const unsigned char*>(data);
        if (m_format.wFormatTag == 105u) {
            if (!decodeXboxAdpcm(source, size, m_format.nChannels,
                                 m_format.nBlockAlign, decoded))
                return false;
            source = decoded.data();
            size = static_cast<unsigned>(decoded.size());
        }
        if (source == nullptr || size == 0 || !open())
            return false;
        auto* block = new HostWaveBlock();
        block->bytes.resize(size);
        std::memcpy(block->bytes.data(), source, size);
        applyVolume(block->bytes.data(), size);
        block->header.lpData = reinterpret_cast<LPSTR>(block->bytes.data());
        block->header.dwBufferLength = size;
        block->header.dwUser = reinterpret_cast<DWORD_PTR>(block);
        if (waveOutPrepareHeader(m_device, &block->header, sizeof(WAVEHDR)) !=
            MMSYSERR_NOERROR) {
            delete block;
            return false;
        }
        if (waveOutWrite(m_device, &block->header, sizeof(WAVEHDR)) !=
            MMSYSERR_NOERROR) {
            waveOutUnprepareHeader(m_device, &block->header, sizeof(WAVEHDR));
            delete block;
            return false;
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        m_blocks.push_back(block);
        return true;
    }
    bool hasPending() {
        reap();
        std::lock_guard<std::mutex> lock(m_mutex);
        return !m_blocks.empty();
    }
    void pause() {
        if (m_device != nullptr)
            waveOutPause(m_device);
    }
    void resume() {
        if (m_device != nullptr)
            waveOutRestart(m_device);
    }
    void reset() {
        if (m_device != nullptr) {
            waveOutReset(m_device);
            std::lock_guard<std::mutex> lock(m_mutex);
            for (HostWaveBlock* block : m_blocks) {
                waveOutUnprepareHeader(m_device, &block->header, sizeof(WAVEHDR));
                delete block;
            }
            m_blocks.clear();
            waveOutClose(m_device);
            m_device = nullptr;
        } else {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (HostWaveBlock* block : m_blocks)
                delete block;
            m_blocks.clear();
        }
    }

private:
    static void CALLBACK waveCallback(HWAVEOUT, UINT message, DWORD_PTR instance,
                                      DWORD_PTR parameter, DWORD_PTR) {
        if (message == WOM_DONE && instance != 0 && parameter != 0)
            reinterpret_cast<HostWaveBlock*>(parameter)->done.store(
                true, std::memory_order_release);
    }
    bool open() {
        if (m_device != nullptr)
            return true;
        if (m_outputFormat.wFormatTag != WAVE_FORMAT_PCM ||
            m_outputFormat.nChannels == 0 || m_outputFormat.nSamplesPerSec == 0 ||
            (m_outputFormat.wBitsPerSample != 8 &&
             m_outputFormat.wBitsPerSample != 16))
            return false;
        WAVEFORMATEX format{};
        static_assert(sizeof(format) == sizeof(tWAVEFORMATEX),
                      "Win32 and IDA WAVEFORMATEX layouts differ");
        std::memcpy(&format, &m_outputFormat, sizeof(format));
        const MMRESULT result = waveOutOpen(
            &m_device, WAVE_MAPPER, &format,
            reinterpret_cast<DWORD_PTR>(&waveCallback),
            reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
        return result == MMSYSERR_NOERROR;
    }
    void reap() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_blocks.begin(); it != m_blocks.end();) {
            HostWaveBlock* block = *it;
            if (!block->done.load(std::memory_order_acquire)) {
                ++it;
                continue;
            }
            waveOutUnprepareHeader(m_device, &block->header, sizeof(WAVEHDR));
            delete block;
            it = m_blocks.erase(it);
        }
    }
    void applyVolume(unsigned char* data, unsigned size) const {
        if (m_volumeDb == 0 || data == nullptr)
            return;
        const float gain = std::exp2(
            static_cast<float>(m_volumeDb) * 0.0016609640474436812f);
        if (m_outputFormat.wBitsPerSample == 16) {
            auto* samples = reinterpret_cast<short*>(data);
            const unsigned count = size / sizeof(short);
            for (unsigned i = 0; i < count; ++i) {
                const int value = static_cast<int>(std::lround(samples[i] * gain));
                samples[i] = static_cast<short>(clampInt(value, -32768, 32767));
            }
        } else if (m_outputFormat.wBitsPerSample == 8) {
            for (unsigned i = 0; i < size; ++i) {
                const int value = static_cast<int>(std::lround(
                    (static_cast<int>(data[i]) - 128) * gain)) + 128;
                data[i] = static_cast<unsigned char>(clampInt(value, 0, 255));
            }
        }
    }
    HWAVEOUT m_device = nullptr;
    tWAVEFORMATEX m_format{};
    tWAVEFORMATEX m_outputFormat{};
    int m_volumeDb = 0;
    std::mutex m_mutex;
    std::vector<HostWaveBlock*> m_blocks;
};

struct HostDevice {
    float distanceFactor = 1.0f;
    float position[3]{};
    float velocity[3]{};
    float front[3]{0.0f, 0.0f, 1.0f};
    float top[3]{0.0f, 1.0f, 0.0f};
};

struct HostStream : IDirectSoundStream {
    tWAVEFORMATEX format{};
    HostWaveOut output;
    bool paused = false;
    unsigned references = 1;
    static unsigned __stdcall AddRef(IDirectSoundStream* value) {
        return ++reinterpret_cast<HostStream*>(value)->references;
    }
    static unsigned __stdcall Release(IDirectSoundStream* value) {
        auto* self = reinterpret_cast<HostStream*>(value);
        const unsigned references = --self->references;
        if (references == 0)
            delete self;
        return references;
    }
    static HRESULT __stdcall GetInfo(IDirectSoundStream*, _XMEDIAINFO*) { return S_OK; }
    static HRESULT __stdcall GetStatus(IDirectSoundStream* value, unsigned* status) {
        auto* self = reinterpret_cast<HostStream*>(value);
        if (status != nullptr)
            *status = self->paused ? 0x20000u :
                (self->output.hasPending() ? 0x10000u : 0u);
        return S_OK;
    }
    static HRESULT __stdcall Process(IDirectSoundStream* value,
                                     const _XMEDIAPACKET* source,
                                     const _XMEDIAPACKET*) {
        auto* self = reinterpret_cast<HostStream*>(value);
        if (source == nullptr || source->pvBuffer == nullptr || self->paused)
            return S_OK;
        self->output.submit(source->pvBuffer, source->dwMaxSize);
        if (source->pdwCompletedSize != nullptr)
            *source->pdwCompletedSize = source->dwMaxSize;
        if (source->pdwStatus != nullptr)
            *source->pdwStatus = 0;
        return S_OK;
    }
    static HRESULT __stdcall Discontinuity(IDirectSoundStream*) { return S_OK; }
    static HRESULT __stdcall Flush(IDirectSoundStream* value) {
        reinterpret_cast<HostStream*>(value)->output.reset();
        return S_OK;
    }
};

IDirectSoundStream_vtbl g_streamVtbl = {
    HostStream::AddRef, HostStream::Release, HostStream::GetInfo,
    HostStream::GetStatus, HostStream::Process, HostStream::Discontinuity,
    HostStream::Flush};

struct HostBuffer {
    tWAVEFORMATEX format{};
    HostWaveOut output;
    const unsigned char* data = nullptr;
    unsigned dataSize = 0;
    unsigned playStart = 0;
    unsigned playLength = 0;
    bool looping = false;
    bool paused = false;
    int volumeDb = 0;
};

struct HostMediaObject : XFileMediaObject {
    HANDLE file = INVALID_HANDLE_VALUE;
    unsigned position = 0;
    unsigned length = 0;
    unsigned references = 1;
    static unsigned __stdcall AddRef(XFileMediaObject* value) {
        return ++reinterpret_cast<HostMediaObject*>(value)->references;
    }
    static unsigned __stdcall Release(XFileMediaObject* value) {
        auto* self = reinterpret_cast<HostMediaObject*>(value);
        const unsigned references = --self->references;
        if (references == 0)
            delete self;
        return references;
    }
    static HRESULT __stdcall GetInfo(XFileMediaObject*, _XMEDIAINFO*) { return S_OK; }
    static HRESULT __stdcall GetStatus(XFileMediaObject* value, unsigned* status) {
        auto* self = reinterpret_cast<HostMediaObject*>(value);
        if (status != nullptr)
            *status = self->position < self->length ? 2u : 0u;
        return S_OK;
    }
    static HRESULT __stdcall Process(XFileMediaObject* value, const _XMEDIAPACKET*,
                                     const _XMEDIAPACKET* destination) {
        if (destination == nullptr || destination->pvBuffer == nullptr)
            return E_INVALIDARG;
        auto* self = reinterpret_cast<HostMediaObject*>(value);
        const unsigned available = self->position < self->length
            ? self->length - self->position : 0u;
        const unsigned requested = minUnsigned(destination->dwMaxSize, available);
        DWORD bytesRead = 0;
        OVERLAPPED overlapped{};
        overlapped.Offset = self->position;
        BOOL readOk = TRUE;
        if (requested != 0) {
            readOk = ReadFile(self->file, destination->pvBuffer, requested,
                              &bytesRead, &overlapped);
            if (!readOk && GetLastError() == ERROR_IO_PENDING)
                readOk = GetOverlappedResult(self->file, &overlapped,
                                             &bytesRead, TRUE);
        }
        if (!readOk)
            return HRESULT_FROM_WIN32(GetLastError());
        self->position += bytesRead;
        if (destination->pdwCompletedSize != nullptr)
            *destination->pdwCompletedSize = bytesRead;
        if (destination->pdwStatus != nullptr)
            *destination->pdwStatus = 0;
        return S_OK;
    }
    static HRESULT __stdcall Discontinuity(XFileMediaObject*) { return S_OK; }
    static HRESULT __stdcall Flush(XFileMediaObject*) { return S_OK; }
    static HRESULT __stdcall Seek(XFileMediaObject* value, int offset,
                                   unsigned origin, unsigned* result) {
        auto* self = reinterpret_cast<HostMediaObject*>(value);
        if (origin == 1)
            self->position = static_cast<unsigned>(clampInt(
                static_cast<int>(self->position) + offset, 0, INT_MAX));
        else
            self->position = offset < 0 ? 0u : static_cast<unsigned>(offset);
        if (result != nullptr)
            *result = self->position;
        return S_OK;
    }
    static HRESULT __stdcall GetLength(XFileMediaObject* value, unsigned* length) {
        if (length != nullptr)
            *length = reinterpret_cast<HostMediaObject*>(value)->length;
        return S_OK;
    }
    static void __stdcall DoWork(XFileMediaObject*) {}
};

XFileMediaObject_vtbl g_mediaVtbl = {
    HostMediaObject::AddRef, HostMediaObject::Release, HostMediaObject::GetInfo,
    HostMediaObject::GetStatus, HostMediaObject::Process,
    HostMediaObject::Discontinuity, HostMediaObject::Flush,
    HostMediaObject::Seek, HostMediaObject::GetLength, HostMediaObject::DoWork};

HostStream* asStream(IDirectSoundStream* value) {
    return reinterpret_cast<HostStream*>(value);
}
HostBuffer* asBuffer(IDirectSoundBuffer* value) {
    return reinterpret_cast<HostBuffer*>(value);
}
HostDevice* asDevice(IDirectSound* value) {
    return reinterpret_cast<HostDevice*>(value);
}
void copyFormat(tWAVEFORMATEX& destination, const tWAVEFORMATEX* source) {
    if (source != nullptr)
        std::memcpy(&destination, source, sizeof(destination));
}

} // namespace

extern "C" HRESULT __stdcall j_DirectSoundCreate(LPCGUID, LPDIRECTSOUND* result,
                                                   LPUNKNOWN) {
    if (result == nullptr)
        return E_POINTER;
    *result = reinterpret_cast<IDirectSound*>(new HostDevice());
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSound_DownloadEffectsImage(
    IDirectSound*, const void*, unsigned int, const _DSEFFECTIMAGELOC*,
    _DSEFFECTIMAGEDESC** descriptor) {
    if (descriptor != nullptr)
        *descriptor = nullptr;
    return S_OK;
}
extern "C" int __stdcall j_IDirectSound_SetDistanceFactor(IDirectSound* value,
                                                             float factor, int) {
    if (value != nullptr) asDevice(value)->distanceFactor = factor;
    return 0;
}
extern "C" HRESULT __stdcall j_IDirectSound_EnableHeadphones(IDirectSound*, int) {
    return S_OK;
}
extern "C" int __stdcall j_DirectSoundUseLightHRTF(void) { return 0; }

extern "C" HRESULT __stdcall j_DirectSoundCreateStream(
    const _DSSTREAMDESC* description, IDirectSoundStream** result) {
    if (result == nullptr)
        return E_POINTER;
    auto* host = new HostStream();
    host->__vftable = &g_streamVtbl;
    if (description != nullptr) copyFormat(host->format, description->lpwfxFormat);
    host->output.setFormat(host->format);
    *result = reinterpret_cast<IDirectSoundStream*>(host);
    return S_OK;
}
extern "C" HRESULT __stdcall j_DirectSoundCreateBuffer(
    const _DSBUFFERDESC* description, IDirectSoundBuffer** result) {
    if (result == nullptr)
        return E_POINTER;
    auto* host = new HostBuffer();
    if (description != nullptr) copyFormat(host->format, description->lpwfxFormat);
    host->output.setFormat(host->format);
    *result = reinterpret_cast<IDirectSoundBuffer*>(host);
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSound_SetI3DL2Listener(
    IDirectSound*, const _DSI3DL2LISTENER*, unsigned int) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundStream_SetHeadroom(
    IDirectSoundStream*, unsigned int) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundStream_SetMode(
    IDirectSoundStream*, unsigned int, unsigned int) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundStream_SetEG(
    IDirectSoundStream*, const _DSENVELOPEDESC*) { return S_OK; }

extern "C" void __stdcall j_XAudioCreatePcmFormat(
    unsigned short channels, unsigned int samplesPerSec,
    unsigned short bitsPerSample, tWAVEFORMATEX* format) {
    if (format == nullptr) return;
    *format = {};
    format->wFormatTag = WAVE_FORMAT_PCM;
    format->nChannels = channels;
    format->nSamplesPerSec = samplesPerSec;
    format->wBitsPerSample = bitsPerSample;
    format->nBlockAlign = static_cast<unsigned short>(channels * bitsPerSample / 8u);
    format->nAvgBytesPerSec = samplesPerSec * format->nBlockAlign;
}
extern "C" void __stdcall j_XAudioCreateAdpcmFormat(
    unsigned short channels, unsigned int samplesPerSec,
    xbox_adpcmwaveformat_tag* format) {
    if (format == nullptr) return;
    *format = {};
    format->wfx.wFormatTag = 105;
    format->wfx.nChannels = channels;
    format->wfx.nSamplesPerSec = samplesPerSec;
    format->wfx.wBitsPerSample = 4;
    format->wfx.nBlockAlign = static_cast<unsigned short>(36u * channels);
    format->wfx.nAvgBytesPerSec = format->wfx.nBlockAlign * (samplesPerSec >> 6);
    format->wfx.cbSize = 2;
    format->wSamplesPerBlock = 64;
}
extern "C" HRESULT __stdcall j_IDirectSoundStream_SetFormat(
    IDirectSoundStream* value, const tWAVEFORMATEX* format) {
    if (value == nullptr || format == nullptr) return E_INVALIDARG;
    auto* host = asStream(value);
    copyFormat(host->format, format);
    host->output.setFormat(host->format);
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundStream_SetMixBins(
    IDirectSoundStream*, const _DSMIXBINS*) { return S_OK; }

extern "C" HRESULT __stdcall j_XFileCreateMediaObjectAsync(
    void* fileHandle, unsigned int, XFileMediaObject** result) {
    if (fileHandle == nullptr || result == nullptr) return E_INVALIDARG;
    auto* host = new HostMediaObject();
    host->__vftable = &g_mediaVtbl;
    host->file = static_cast<HANDLE>(fileHandle);
    LARGE_INTEGER length{};
    if (!GetFileSizeEx(host->file, &length)) {
        delete host;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    host->length = length.QuadPart > UINT32_MAX ? UINT32_MAX
                                                : static_cast<unsigned>(length.QuadPart);
    *result = reinterpret_cast<XFileMediaObject*>(host);
    return S_OK;
}

extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetHeadroom(
    IDirectSoundBuffer*, unsigned int) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetMode(
    IDirectSoundBuffer*, unsigned int, unsigned int) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetEG(
    IDirectSoundBuffer*, const _DSENVELOPEDESC*) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetFormat(
    IDirectSoundBuffer* value, const tWAVEFORMATEX* format) {
    if (value == nullptr || format == nullptr) return E_INVALIDARG;
    auto* host = asBuffer(value);
    copyFormat(host->format, format);
    host->output.setFormat(host->format);
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetMixBins(
    IDirectSoundBuffer*, const _DSMIXBINS*) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetVolume(
    IDirectSoundBuffer* value, int volume) {
    if (value == nullptr) return E_INVALIDARG;
    auto* host = asBuffer(value);
    host->volumeDb = volume;
    host->output.setVolume(volume);
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundStream_FlushEx(
    IDirectSoundStream* value, __int64, unsigned int) {
    return value == nullptr ? E_INVALIDARG : HostStream::Flush(value);
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_StopEx(
    IDirectSoundBuffer* value, __int64, unsigned int) {
    if (value == nullptr) return E_INVALIDARG;
    asBuffer(value)->output.reset();
    asBuffer(value)->paused = false;
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_Play(
    IDirectSoundBuffer* value, unsigned int, unsigned int, unsigned int flags) {
    if (value == nullptr) return E_INVALIDARG;
    auto* host = asBuffer(value);
    if (host->data == nullptr || host->playLength == 0 ||
        host->playStart >= host->dataSize)
        return S_OK;
    const unsigned length = minUnsigned(host->playLength,
                                         host->dataSize - host->playStart);
    host->looping = flags != 0;
    host->paused = false;
    host->output.setVolume(host->volumeDb);
    host->output.submit(host->data + host->playStart, length);
    return S_OK;
}
extern "C" int __stdcall j_IDirectSoundStream_SetMinDistance(
    IDirectSoundStream*, float, int) { return 0; }
extern "C" int __stdcall j_IDirectSoundStream_SetMaxDistance(
    IDirectSoundStream*, float, int) { return 0; }
extern "C" HRESULT __stdcall j_IDirectSoundStream_SetRolloffCurve(
    IDirectSoundStream*, const float*, unsigned int, unsigned int) { return S_OK; }
extern "C" int __stdcall j_IDirectSoundBuffer_SetMinDistance(
    IDirectSoundBuffer*, float, int) { return 0; }
extern "C" int __stdcall j_IDirectSoundBuffer_SetMaxDistance(
    IDirectSoundBuffer*, float, int) { return 0; }
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetRolloffCurve(
    IDirectSoundBuffer*, const float*, unsigned int, unsigned int) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundStream_Pause(
    IDirectSoundStream* value, unsigned int pause) {
    if (value == nullptr) return E_INVALIDARG;
    auto* stream = asStream(value);
    if (pause == 1u) {
        stream->paused = true;
        stream->output.pause();
    } else if (pause == 0u || pause == 2u) {
        stream->paused = false;
        stream->output.resume();
    }
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_Pause(
    IDirectSoundBuffer* value, unsigned int pause) {
    if (value == nullptr) return E_INVALIDARG;
    auto* buffer = asBuffer(value);
    if (pause == 1u) {
        buffer->paused = true;
        buffer->output.pause();
    } else if (pause == 0u || pause == 2u) {
        buffer->paused = false;
        buffer->output.resume();
    }
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundStream_SetAllParameters(
    IDirectSoundStream*, const _DS3DBUFFER*, unsigned int) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetAllParameters(
    IDirectSoundBuffer*, const _DS3DBUFFER*, unsigned int) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundStream_SetVolume(
    IDirectSoundStream* value, int volume) {
    if (value == nullptr) return E_INVALIDARG;
    asStream(value)->output.setVolume(volume);
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundStream_SetFrequency(
    IDirectSoundStream* value, unsigned frequency) {
    if (value == nullptr) return E_INVALIDARG;
    asStream(value)->output.setFrequency(frequency);
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetFrequency(
    IDirectSoundBuffer* value, unsigned frequency) {
    if (value == nullptr) return E_INVALIDARG;
    asBuffer(value)->output.setFrequency(frequency);
    return S_OK;
}
extern "C" int __stdcall j_IDirectSound_SetPosition(
    IDirectSound* value, float x, float y, float z, int) {
    if (value != nullptr) {
        asDevice(value)->position[0] = x; asDevice(value)->position[1] = y;
        asDevice(value)->position[2] = z;
    }
    return 0;
}
extern "C" int __stdcall j_IDirectSound_SetVelocity(
    IDirectSound* value, float x, float y, float z, int) {
    if (value != nullptr) {
        asDevice(value)->velocity[0] = x; asDevice(value)->velocity[1] = y;
        asDevice(value)->velocity[2] = z;
    }
    return 0;
}
extern "C" int __stdcall j_IDirectSound_SetOrientation(
    IDirectSound* value, float fx, float fy, float fz, float tx, float ty,
    float tz, int) {
    if (value != nullptr) {
        asDevice(value)->front[0] = fx; asDevice(value)->front[1] = fy;
        asDevice(value)->front[2] = fz; asDevice(value)->top[0] = tx;
        asDevice(value)->top[1] = ty; asDevice(value)->top[2] = tz;
    }
    return 0;
}
extern "C" HRESULT __stdcall j_IDirectSound_CommitDeferredSettings(
    IDirectSound*) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSound_SynchPlayback(IDirectSound*) {
    return S_OK;
}
extern "C" int __cdecl j_DirectSoundDoWork(
    unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
    unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
    unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
    unsigned int, unsigned int, unsigned int, unsigned int) { return 0; }
extern "C" HRESULT __stdcall j_IDirectSound_CreateSoundBuffer(
    IDirectSound*, const _DSBUFFERDESC* description, IDirectSoundBuffer** result,
    IUnknown*) { return j_DirectSoundCreateBuffer(description, result); }
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_GetCurrentPosition(
    IDirectSoundBuffer*, unsigned* playCursor, unsigned* writeCursor) {
    if (playCursor != nullptr) *playCursor = 0;
    if (writeCursor != nullptr) *writeCursor = 0;
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_GetStatus(
    IDirectSoundBuffer* value, unsigned* status) {
    if (value == nullptr) return E_INVALIDARG;
    auto* host = asBuffer(value);
    if (host->looping && !host->paused && !host->output.hasPending() &&
        host->data != nullptr && host->playLength != 0 &&
        host->playStart < host->dataSize) {
        const unsigned length = minUnsigned(host->playLength,
                                            host->dataSize - host->playStart);
        host->output.submit(host->data + host->playStart, length);
    }
    if (status != nullptr)
        *status = host->paused ? 0u : (host->output.hasPending() ? 1u : 0u);
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetCurrentPosition(
    IDirectSoundBuffer*, unsigned) { return S_OK; }
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_PlayEx(
    IDirectSoundBuffer* value, __int64, unsigned flags) {
    return j_IDirectSoundBuffer_Play(value, 0, 0, flags);
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_Stop(IDirectSoundBuffer* value) {
    return j_IDirectSoundBuffer_StopEx(value, 0, 0);
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetBufferData(
    IDirectSoundBuffer* value, void* data, unsigned bytes) {
    if (value == nullptr) return E_INVALIDARG;
    auto* host = asBuffer(value);
    host->data = static_cast<const unsigned char*>(data);
    host->dataSize = bytes;
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetPlayRegion(
    IDirectSoundBuffer* value, unsigned start, unsigned length) {
    if (value == nullptr) return E_INVALIDARG;
    auto* host = asBuffer(value);
    host->playStart = start;
    host->playLength = length;
    return S_OK;
}
extern "C" HRESULT __stdcall j_IDirectSoundBuffer_SetLoopRegion(
    IDirectSoundBuffer* value, unsigned, unsigned length) {
    if (value == nullptr) return E_INVALIDARG;
    asBuffer(value)->looping = length != 0;
    return S_OK;
}
