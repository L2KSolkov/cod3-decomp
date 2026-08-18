// ============================================================================
// NSL — NGL Sound Library (237 funcs, 17 objects)
// ea: 0x8E0000-0x8XXXXX (sound subsystem)
// Xbox DirectSound wrapper — stubbed for Win32/XAudio2.
// ============================================================================

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <intrin.h>

#include "core/tlFixedString.h"

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);
extern "C" void txAssertFailed(unsigned char* ignore, const char* message,
                                const char* function, const char* source, int line);
extern "C" void txPrintf(const char* channel, int level, const char* fmt, ...);
extern "C" int __cdecl __fpclass(float value);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Handle types
// ============================================================================
enum nslSourceID : int { NSL_SOURCE_ID_INVALID = -1 };
typedef unsigned nslEmitterID;
enum nslWaveID : int { NSL_WAVE_ID_INVALID = -1 };
typedef unsigned nslWaveBankID;
typedef unsigned nslGroupID;
typedef unsigned nslVoiceID;
typedef unsigned txSlot;

#define NSL_INVALID_SOURCE   ((nslSourceID)-1)
#define NSL_INVALID_EMITTER  ((nslEmitterID)-1)
#define NSL_INVALID_WAVE     ((nslWaveID)-1)
#define NSL_INVALID_BANK     ((nslWaveBankID)-1)

// ============================================================================
// Enums
// ============================================================================
enum nslSpeakerMode : int {
    NSL_SPEAKER_MODE_INVALID = 0,
    NSL_SPEAKER_MODE_MONO = 1,
    NSL_SPEAKER_MODE_STEREO = 2,
    NSL_SPEAKER_MODE_SURROUND = 3,
    NSL_SPEAKER_MODE_HEADPHONES = 4
};
enum nslWaveBankLoaderState {
    NSL_WAVE_BANK_LOADER_ERROR_INVALIDBANK = -5,
    NSL_WAVE_BANK_LOADER_ERROR_NOMEMORY = -4,
    NSL_WAVE_BANK_LOADER_ERROR_READING = -3,
    NSL_WAVE_BANK_LOADER_ERROR_NOARAM = -2,
    NSL_WAVE_BANK_LOADER_STATE_CANCELED = -1,
    NSL_WAVE_BANK_LOADER_STATE_INITIAL = 0,
    NSL_WAVE_BANK_LOADER_STATE_COMPLETED = 1,
    NSL_WAVE_BANK_LOADER_STATE_START_READ_HEADER = 2,
    NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_HEADER = 3,
    NSL_WAVE_BANK_LOADER_STATE_START_READ_DIR_SECTION = 4,
    NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_DIR_SECTION = 5,
    NSL_WAVE_BANK_LOADER_STATE_START_READ_ARAM_SECTION = 6,
    NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_ARAM_SECTION = 7,
    NSL_WAVE_BANK_LOADER_STATE_START_COPY_ARAM_SECTION = 8,
    NSL_WAVE_BANK_LOADER_STATE_CHECK_COPY_ARAM_SECTION = 9,
    NSL_WAVE_BANK_LOADER_STATE_START_READ_PRIMED_STREAMS = 10,
    NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_PRIMED_STREAMS = 11,
    NSL_WAVE_BANK_LOADER_STATE_CANCELING = 0x100
};
enum nslVoiceState         { NSL_VOICE_FREE=0, NSL_VOICE_PLAYING=1, NSL_VOICE_PAUSED=2 };
// Values verified vs disasm SoundDevice::Sound::IsPlaying (compares 4/2/3/5)
enum nslSourceState        { NSL_SOURCE_STATE_INVALID=0,
                             NSL_SOURCE_STATE_QUEUING=2,
                             NSL_SOURCE_STATE_QUEUED=3,
                             NSL_SOURCE_STATE_PLAYING=4,
                             NSL_SOURCE_STATE_PAUSED=5 };

// ============================================================================
// Forward types
// ============================================================================
// IDA type_inspect: nslInitParams is 16 bytes: maxSources, maxEmitters,
// aramBase, and aramSize.  The release global is initialized to {512, 0, 8, 0}.
struct nslInitParams {
    unsigned maxSources;
    unsigned maxEmitters;
    unsigned aramBase;
    unsigned aramSize;
};
using nslSourceCallback = void (__cdecl *)(void*, void*, nslSourceID, unsigned, void*, int);
struct nslSource {
    unsigned __int64 paramsUsed;
    unsigned __int64 paramsUpdate;
    float params[46];
    nslWaveID waveId;
    nslEmitterID emitterId;
    short voice;
    short pauseCount;
    unsigned char state;
    unsigned char beforePauseState;
    unsigned char flags;
    unsigned char update;
    unsigned voiceAllocCount;
    unsigned position;
    unsigned length;
    unsigned offset;
    nslSourceCallback callback;
    void* callbackObject;
    void* callbackData;
    int dampenCount;
};
struct nslEmitter {
    unsigned __int64 paramsUsed;
    unsigned __int64 paramsUpdate;
    float params[46];
};
static_assert(sizeof(nslSource) == 248, "IDA nslSource layout");
static_assert(sizeof(nslEmitter) == 200, "IDA nslEmitter layout");
static constexpr unsigned nslSourceStride = 328u;
static constexpr unsigned nslEmitterStride = 272u;

struct txSlotEntry {
    txSlotEntry* next;
    txSlotEntry* prev;
    txSlot slot;
};
struct txSlotPool {
    txSlotEntry* slots;
    txSlotEntry freeSlots;
    txSlotEntry usedSlots;
    int stride;
    int count;
    int mask;
};
static_assert(sizeof(txSlotEntry) == 12, "IDA txSlotEntry layout");
static_assert(sizeof(txSlotPool) == 40, "IDA txSlotPool layout");
static constexpr txSlot TX_SLOT_INVALID = static_cast<txSlot>(-1);
enum nflFileID : unsigned { NFL_FILE_ID_INVALID = (unsigned)-1 };
struct nslWave {
    unsigned char opaque[40]; // IDA type_inspect: nslWave size 0x28; fields not needed by this loader step.
};
struct nslWaveName {
    union {
        const char* name;
        unsigned hash;
    };
};
struct nslWaveBank {
    char header[4];
    unsigned char versionMajor;
    unsigned char versionMinor;
    unsigned char waveBankFlags;
    char waveBankState;
    unsigned waveBankSize;
    unsigned reserved;
    unsigned waveCount;
    unsigned waveHashTableSize;
    nslWaveName* names;
    nslWave* waves;
    unsigned infoSize;
    void* infos;
    unsigned textSize;
    char* text;
    unsigned aramSize;
    unsigned aramOffset;
    unsigned streamSize;
    unsigned streamOffset;
    char name[32];
    union {
        unsigned char aramMD5[16];
        struct {
            void* waveBankAram;
            nflFileID waveBankFile;
            unsigned waveBankFileOffset;
            unsigned reserved;
        } backing;
    } storage;
    unsigned char streamMD5[16];
};
enum nslWaveBankSlotState {
    NSL_WAVE_BANK_SLOT_STATE_NOTUSED = 0,
    NSL_WAVE_BANK_SLOT_STATE_PENDING = 1,
    NSL_WAVE_BANK_SLOT_STATE_LOADING = 2,
    NSL_WAVE_BANK_SLOT_STATE_LOADED = 3
};
struct nslWaveBankSlotProfile {
    unsigned timeCreated;
    unsigned timeStarted;
    unsigned timeLoaded;
    unsigned frameCreated;
    unsigned frameStarted;
    unsigned frameLoaded;
};
struct nslWaveBankSlot {
    nslWaveBankID waveBankID;
    nslWaveBankSlotState state;
    unsigned flags;
    nflFileID file;
    unsigned fileOffset;
    unsigned loadOrder;
    nslWaveBank* waveBank;
    nslWaveBankSlotProfile profile;
};
struct nslGroup {
    unsigned __int64 paramsUpdate;
    float params[64];
    char name[32];
};
static_assert(sizeof(nslGroup) == 296, "IDA nslGroup layout");

// IDA's release code advances the allocated records by 0x120 bytes even
// though the named UDT includes a 32-byte name member (sizeof == 0x128).
static constexpr unsigned nslGroupStride = 0x120u;
struct nslVoice {};
struct nslListener {};
struct nslDriverParams {};
enum nflRequestID : unsigned { NFL_REQUEST_ID_INVALID = (unsigned)-1 };
struct nslWaveBankLoader {
    nslWaveBankLoaderState state;
    unsigned flags;
    nflRequestID rid;
    int stid;
    nflFileID file;
    unsigned fileOffset;
    nslWaveBank* waveBank;
    unsigned waveBankSize;
    unsigned aramSize;
    unsigned aramOffset;
    void* aram;
};
static_assert(sizeof(nslWaveBank) == 128, "IDA nslWaveBank layout");
static_assert(sizeof(nslWaveBankSlot) == 52, "IDA nslWaveBankSlot layout");
static_assert(sizeof(nslWaveBankLoader) == 44, "IDA nslWaveBankLoader layout");

// Release globals verified from IDA addresses 0xE4B680, 0x10E11AC-0x10E11F0,
// and 0x10E1220.  These are the state consumed by the bank-slot functions.
nslInitParams nsl_initParams = { 512u, 0u, 8u, 0u };
static void* nsl_work = nullptr;
static unsigned nsl_workUsed = 0;
static unsigned nsl_workLimit = 0;
static unsigned nsl_time = 0;
static unsigned nsl_frame = 0;
static unsigned nsl_waveBankLoadOrder = 0;
nslSpeakerMode nsl_speakerMode = static_cast<nslSpeakerMode>(-2);
nslWaveBankSlot* nsl_waveBankSlots = nullptr;
static unsigned char nsl_waveBankLoaderBuffer[4096] = {};
static nslWaveBankLoader nsl_waveBankLoad = {};
static int dword_E4B690 = 16;
static txSlotEntry* nsl_sourceEntries = nullptr;
static txSlotEntry* nsl_emitterEntries = nullptr;
static nslSource* nsl_sources = nullptr;
static void* nsl_sourcesSorted = nullptr;
static nslEmitter* nsl_emitters = nullptr;
static nslGroup* nsl_groups = nullptr;
static void* nsl_voices = nullptr;
static void* nsl_driverVoices = nullptr;
static txSlotPool nsl_sourcePool = {};
static txSlotPool nsl_emitterPool = {};

static bool nslSlotPoolInit(txSlotPool* pool, txSlotEntry* slots, int count,
                            unsigned stride) {
    if (count < 0 || count > 0xFFFFFu || stride < sizeof(txSlotEntry))
        return false;

    int bits = 0;
    for (int value = count; value != 0; ++bits)
        value >>= 1;

    pool->slots = slots;
    pool->stride = static_cast<int>(stride);
    pool->count = count;
    pool->mask = (1 << bits) - 1;
    pool->freeSlots.slot = TX_SLOT_INVALID;
    pool->freeSlots.next = &pool->freeSlots;
    pool->freeSlots.prev = &pool->freeSlots;
    pool->usedSlots.slot = TX_SLOT_INVALID;
    pool->usedSlots.next = &pool->usedSlots;
    pool->usedSlots.prev = &pool->usedSlots;

    for (int index = 0; index < count; ++index) {
        txSlotEntry* entry = reinterpret_cast<txSlotEntry*>(
            reinterpret_cast<unsigned char*>(slots) + stride * index);
        entry->slot = static_cast<txSlot>(index) | 0x80000000u;
        entry->next = &pool->freeSlots;
        entry->prev = pool->freeSlots.prev;
        pool->freeSlots.prev->next = entry;
        pool->freeSlots.prev = entry;
    }
    return true;
}

static int nslSlotIndex(const txSlotPool* pool, txSlot slot) {
    const unsigned result = slot & (static_cast<unsigned>(pool->mask) | 0x80000000u);
    if (result >= static_cast<unsigned>(pool->count))
        return -1;
    const txSlotEntry* entry = reinterpret_cast<const txSlotEntry*>(
        reinterpret_cast<const unsigned char*>(pool->slots) + pool->stride * result);
    return entry->slot == slot ? static_cast<int>(result) : -1;
}

// Forward declarations for the IDA-backed bank layer below.
unsigned nslDriverVoiceSize();
nslWaveBankID nslWaveBankLoad(nflFileID file, unsigned fileOffset, unsigned flags);
int nslWaveBankGetState(nslWaveBankID waveBankID);
unsigned nslWaveBankSlotsGetUsedCount();
unsigned nslWaveBankSlotsGetLoadingCount();
void* nslAramAlloc(unsigned size, unsigned flags);
void nslAramFree(void* ptr);
void* nslMemoryAlloc(unsigned size);
void nslMemoryFree(void* ptr);
int nslWaveBankFixup(nslWaveBank* waveBank);
int nslWaveBankSetAram(nslWaveBank* waveBank, void* waveBankAram);
int nslWaveBankSetFile(nslWaveBank* waveBank, nflFileID waveBankFile, unsigned waveBankFileOffset);
void nslWaveBankFree(nslWaveBankID waveBankID);
void nslWaveBankLoaderInit(nslWaveBankLoader* waveBankLoader, unsigned waveBankLoadFlags,
                           nflFileID file, unsigned fileOffset);
nslWaveBankLoaderState nslWaveBankLoaderUpdate(nslWaveBankLoader* waveBankLoader);
void nslWaveBankLoaderCancel(nslWaveBankLoader* waveBankLoader);
void nslWaveBankSort(nslWaveBank* waveBank);
enum nflRequestState : unsigned {
    NFL_REQUEST_STATE_INVALID = (unsigned)-1,
    NFL_REQUEST_STATE_COMPLETED = 0,
    NFL_REQUEST_STATE_CANCELED = 1,
    NFL_REQUEST_STATE_TIMEOUT = 2,
    NFL_REQUEST_STATE_ERROR = 3,
    NFL_REQUEST_STATE_ACTIVE = 4
};
extern void nflCancelRequest(nflRequestID requestID);
extern nflRequestID nflReadFileAsync(nflFileID fileID, unsigned offset, void* buffer,
                                     unsigned dataSize);
extern nflRequestState nflGetRequestState(nflRequestID requestID);

// IDA nslInit_Allocate (0x826800): callers pass size on the stack and 0x100
// in ECX.  The 32-bit release arithmetic is preserved for the Win32 target.
static void* nslInit_Allocate(unsigned size, unsigned align) {
    const unsigned workAddress = static_cast<unsigned>(reinterpret_cast<uintptr_t>(nsl_work));
    const unsigned aligned = align * ((nsl_workUsed + workAddress + align - 1u) / align);
    const unsigned used = nsl_workUsed + align + size - 1u;
    nsl_workUsed = used;
    if (nsl_work != nullptr)
        return nsl_workLimit >= used ? reinterpret_cast<void*>(static_cast<uintptr_t>(aligned)) : nullptr;
    return reinterpret_cast<void*>(static_cast<uintptr_t>(1));
}

// ============================================================================
// nslInit — init/shutdown
// ============================================================================
int          nslInit(const nslInitParams* ip) {
    if (nsl_workUsed != 0)
        return -1;
    if (ip != nullptr)
        nsl_initParams = *ip;

    nsl_sourceEntries = static_cast<txSlotEntry*>(
        nslInit_Allocate(12u * nsl_initParams.maxSources, 0x100u));
    if (nsl_sourceEntries != nullptr) {
        nsl_emitterEntries = static_cast<txSlotEntry*>(
            nslInit_Allocate(12u * nsl_initParams.maxEmitters, 0x100u));
        if (nsl_emitterEntries != nullptr) {
            nsl_sources = static_cast<nslSource*>(
                nslInit_Allocate(nslSourceStride * nsl_initParams.maxSources, 0x100u));
            if (nsl_sources != nullptr) {
                nsl_sourcesSorted = nslInit_Allocate(4u * nsl_initParams.maxSources, 0x100u);
                if (nsl_sourcesSorted != nullptr) {
                    nsl_emitters = static_cast<nslEmitter*>(
                        nslInit_Allocate(nslEmitterStride * nsl_initParams.maxEmitters, 0x100u));
                    if (nsl_emitters != nullptr) {
                        nsl_groups = static_cast<nslGroup*>(
                            nslInit_Allocate(nslGroupStride * static_cast<unsigned>(dword_E4B690),
                                             0x100u));
                        if (nsl_groups != nullptr) {
                            nsl_voices = nslInit_Allocate(320u * nsl_initParams.aramSize, 0x100u);
                            if (nsl_voices != nullptr) {
                                nsl_driverVoices = nslInit_Allocate(
                                    nsl_initParams.aramSize * nslDriverVoiceSize(), 0x100u);
                                if (nsl_driverVoices != nullptr) {
                                    nsl_waveBankSlots = static_cast<nslWaveBankSlot*>(
                                        nslInit_Allocate(52u * nsl_initParams.aramBase, 0x100u));
                                    if (nsl_waveBankSlots != nullptr)
                                        return static_cast<int>(nsl_workUsed);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return nsl_work != nullptr ? -1 : static_cast<int>(nsl_workUsed);
}
void         nslShutdown() {}
// ea: 0x004267B0
nslSpeakerMode nslGetSpeakerMode() {
    const int value = static_cast<int>(nsl_speakerMode);
    return static_cast<nslSpeakerMode>(value < 0 ? -value : value);
}
// ea: 0x004267C0
void         nslSetSpeakerMode(nslSpeakerMode speakerMode) {
    const int value = static_cast<int>(speakerMode);
    nsl_speakerMode = static_cast<nslSpeakerMode>(-(value < 0 ? -value : value));
}
void         nslGetInitParams(nslInitParams*) {}
void         nslInitDefaults() {}
void         nslFinalInit() {}
bool         nslIsInitDone() { return true; }
// ea: 0x00426C50
unsigned     nslGetVersion() { return 4; }

// ============================================================================
// nslSource — sound sources / emitters (3D positioned)
// ============================================================================
nslWave*      nslWavePtr(nslWaveID);
nslEmitterID  nslNewEmitter(const float* pos) { return 0; }
nslSourceID   nslNewSource(nslWaveID waveID, int mImportance) { return NSL_SOURCE_ID_INVALID; }
void          nslDeleteSource(nslSourceID) {}
void          nslDeleteEmitter(nslEmitterID) {}
// ea: 0x008207A0
nslSource*    nslSourcePtr(nslSourceID sourceID) {
    const int index = nslSlotIndex(&nsl_sourcePool, static_cast<txSlot>(sourceID));
    if (index == -1)
        return nullptr;
    return reinterpret_cast<nslSource*>(
        reinterpret_cast<unsigned char*>(nsl_sources) +
        nslSourceStride * static_cast<unsigned>(index));
}
// ea: 0x008207D0
nslEmitter*   nslEmitterPtr(nslEmitterID emitterID) {
    const int index = nslSlotIndex(&nsl_emitterPool, static_cast<txSlot>(emitterID));
    if (index == -1)
        return nullptr;
    return reinterpret_cast<nslEmitter*>(
        reinterpret_cast<unsigned char*>(nsl_emitters) +
        nslEmitterStride * static_cast<unsigned>(index));
}
void          nslSourcePlay(nslSourceID) {}
void          nslSourceStop(nslSourceID) {}
void          nslSourcePause(nslSourceID, bool) {}
bool          nslSourceIsPlaying(nslSourceID) { return false; }
void          nslSourceSetVolume(nslSourceID, float) {}
void          nslSourceSetPitch(nslSourceID, float) {}
void          nslSourceSetPosition(nslSourceID, const float*) {}
void          nslSourceSetVelocity(nslSourceID, const float*) {}
void          nslSourceSetConeAngles(nslSourceID, float, float) {}
void          nslSourceSetConeOutsideVolume(nslSourceID, float) {}
void          nslSourceSetMinDistance(nslSourceID, float) {}
void          nslSourceSetMaxDistance(nslSourceID, float) {}
// ea: 0x00820F10
void          nslSetSourceParam(nslSourceID sid, int index, float value) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr || index >= 0x40u)
        return;
    const unsigned __int64 mask = 1ull << index;
    source->paramsUpdate |= mask;
    source->params[index] = value;
}

// Voice enumeration (used by EffectEventSys::NumberOfVoicesUsed)
unsigned      nslGetNumVoices() { return 0; }
nslVoice*     nslGetVoice(unsigned) { return nullptr; }
// ea: 0x00820E10
nslSourceState nslGetSourceState(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return NSL_SOURCE_STATE_INVALID;

    // These fields are outside IDA's named 0xF8-byte UDT but are read at
    // fixed offsets by the release source pool stride (0x148).
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(source);
    const unsigned char state = raw[0x120];
    if (state == 5 || (raw[0x123] & 2u) != 0)
        return NSL_SOURCE_STATE_PAUSED;
    const unsigned char flags = raw[0x122];
    if ((flags & 2u) != 0)
        return NSL_SOURCE_STATE_PLAYING;
    if ((flags & 1u) != 0)
        return static_cast<nslSourceState>((state == 3) + 2);
    return static_cast<nslSourceState>(state);
}
// ea: 0x00822D60
bool          nslIsSourceQueued(nslSourceID sid) {
    return nslGetSourceState(sid) == NSL_SOURCE_STATE_QUEUED;
}
// ea: 0x00822D80
bool          nslIsSourcePlaying(nslSourceID sid) {
    return nslGetSourceState(sid) == NSL_SOURCE_STATE_PLAYING;
}
// ea: 0x00822DA0
bool          nslIsSourceFinished(nslSourceID sid) {
    return nslGetSourceState(sid) == NSL_SOURCE_STATE_INVALID;
}
// ea: 0x00820CB0
nslWaveID     nslGetSourceWaveID(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return NSL_WAVE_ID_INVALID;
    return *reinterpret_cast<const nslWaveID*>(
        reinterpret_cast<const unsigned char*>(source) + 0x110u);
}
// ea: 0x00820CF0
nslWaveID     nslGetSourceWave(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return NSL_WAVE_ID_INVALID;
    return *reinterpret_cast<const nslWaveID*>(
        reinterpret_cast<const unsigned char*>(source) + 0x110u);
}
// ea: 0x00820E90
void          nslSetSourceOffset(nslSourceID sid, unsigned offset) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(source);
    raw[0x123] |= 1u;
    *reinterpret_cast<unsigned*>(raw + 0x130u) = offset;
}
// ea: 0x00820ED0
unsigned      nslGetSourceOffset(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return 0;
    return *reinterpret_cast<const unsigned*>(
        reinterpret_cast<const unsigned char*>(source) + 0x130u);
}
// ea: 0x008212D0
void          nslGetSourcePosition(nslSourceID sid, float* position) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    const float* value = reinterpret_cast<const float*>(
        reinterpret_cast<const unsigned char*>(source) + 0x5Cu);
    position[0] = value[0];
    position[1] = value[1];
    position[2] = value[2];
}
// ea: 0x00821590
void          nslGetSourceVelocity(nslSourceID sid, float* velocity) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    const float* value = reinterpret_cast<const float*>(
        reinterpret_cast<const unsigned char*>(source) + 0x68u);
    velocity[0] = value[0];
    velocity[1] = value[1];
    velocity[2] = value[2];
}
// ea: 0x008210B0
void          nslSetSourcePosition(nslSourceID sid, const float* position) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    if ((__fpclass(static_cast<double>(position[0])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 396,
               "!(_fpclass(position[0])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    if ((__fpclass(static_cast<double>(position[1])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 397,
               "!(_fpclass(position[1])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    if ((__fpclass(static_cast<double>(position[2])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 398,
               "!(_fpclass(position[2])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    unsigned char* raw = reinterpret_cast<unsigned char*>(source);
    float* value = reinterpret_cast<float*>(raw + 0x5Cu);
    value[0] = position[0];
    value[1] = position[1];
    value[2] = position[2];
    source->paramsUpdate |= 0x00380000u;
}
// ea: 0x00821370
void          nslSetSourceVelocity(nslSourceID sid, const float* velocity) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    if ((__fpclass(static_cast<double>(velocity[0])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 428,
               "!(_fpclass(velocity[0])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    if ((__fpclass(static_cast<double>(velocity[1])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 429,
               "!(_fpclass(velocity[1])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    if ((__fpclass(static_cast<double>(velocity[2])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 430,
               "!(_fpclass(velocity[2])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    unsigned char* raw = reinterpret_cast<unsigned char*>(source);
    float* value = reinterpret_cast<float*>(raw + 0x68u);
    value[0] = velocity[0];
    value[1] = velocity[1];
    value[2] = velocity[2];
    source->paramsUpdate |= 0x01C00000u;
}
// ea: 0x00821830
nslEmitterID  nslGetSourceEmitter(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return NSL_INVALID_EMITTER;
    return *reinterpret_cast<const nslEmitterID*>(
        reinterpret_cast<const unsigned char*>(source) + 0x114u);
}
// ea: 0x00822800
void          nslSetSourceEffect(nslSourceID sid, int effectOn) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(source);
    const nslWaveID waveID = *reinterpret_cast<const nslWaveID*>(raw + 0x110u);
    nslWave* wave = nslWavePtr(waveID);
    if (wave == nullptr)
        return;
    const unsigned char* nameOffset = *reinterpret_cast<const unsigned char* const*>(wave);
    if (nameOffset == nullptr || (nameOffset[7] & 2u) == 0)
        return;
    unsigned char* mutableRaw = reinterpret_cast<unsigned char*>(source);
    if (effectOn != 0)
        mutableRaw[0x122] |= 0x10u;
    else
        mutableRaw[0x122] &= 0xEFu;
    mutableRaw[0x123] |= 8u;
}
// ea: 0x00822870
void          nslSetSourceEffectOn(nslSourceID sid) {
    nslSetSourceEffect(sid, 1);
}
// ea: 0x008228E0
void          nslSetSourceEffectOff(nslSourceID sid) {
    nslSetSourceEffect(sid, 0);
}
// ea: 0x00822950
int           nslGetSourceEffect(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return 0;
    return (reinterpret_cast<const unsigned char*>(source)[0x122] & 0x10u) != 0;
}
unsigned      nslGetMaxNumVoices() { return 0; }  // ?nslGetMaxNumVoices@@YAIXZ (nslCompat.o)
const char*   nslGetSourceName(nslSourceID) { return ""; }   // ?nslGetSourceName@@YAPBDW4nslSourceID@@@Z (nslSource.o)
const char*   nslGetWaveName(nslWaveID) { return ""; }       // ?nslGetWaveName@@YAPBDW4nslWaveID@@@Z (nslCompat.o)
const char*   nslWaveGetName(nslWaveID) { return ""; }       // ?nslWaveGetName@@YAPBDW4nslWaveID@@@Z (nslWaveBank.o)
const char*   nslWaveGetGroupName(nslWaveID) { return ""; }  // ?nslWaveGetGroupName@@YAPBDW4nslWaveID@@@Z (nslWaveBank.o)
const char*   nslGetWaveGroup(nslWaveID) { return ""; }      // ?nslGetWaveGroup@@YAPBDW4nslWaveID@@@Z (nslCompat.o)
enum nslBankID : unsigned { NSL_BANK_ID_INVALID = (unsigned)-1 };
nslBankID      nslLoadBank(unsigned int flags, unsigned int file, unsigned int fileOffset) { return static_cast<nslBankID>(nslWaveBankLoad(static_cast<nflFileID>(file), fileOffset, flags)); }  // ?nslLoadBank@@YA?AW4nslBankID@@III@Z
nslWaveID      nslGetWave(const char*) { return NSL_WAVE_ID_INVALID; }                  // ?nslGetWave@@YA?AW4nslWaveID@@PBD@Z
float          nslGetWaveParam(nslWaveID, int, float defaultValue) { return defaultValue; }  // ?nslGetWaveParam@@YAMW4nslWaveID@@HM@Z
// ea: 0x00820FF0
float         nslGetSourceParam(nslSourceID sid, int index, float defaultValue) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr || index < 0 || index >= 0x40)
        return defaultValue;
    return source->params[index];
}  // ?nslGetSourceParam@@YAMW4nslSourceID@@HM@Z (nslSource.o)
int           nslIsWaveStreamed(nslWaveID) { return 0; }     // ?nslIsWaveStreamed@@YAHW4nslWaveID@@@Z (nslCompat.o)

// nslCompat.o / nslSource.o family (stubbed; manglings match binary)
int           nslGetBankState(nslBankID bankID) { return nslWaveBankGetState(static_cast<nslWaveBankID>(bankID)); }
void          nslFreeBank(nslBankID bankID) { nslWaveBankFree(static_cast<nslWaveBankID>(bankID)); }
void          nslFreeSource(nslSourceID);
// ea: 0x00820B50
void          nslStopSource(nslSourceID sid) { nslFreeSource(sid); }
void          nslQueueSource(nslSourceID) {}
void          nslFreeSource(nslSourceID) {}
void          nslDampen(float) {}
void          nslUndampen() {}
// ea: 0x00821890
void          nslRemoveEmitterSource(nslEmitterID eid, nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    nslEmitter* emitter = nslEmitterPtr(eid);
    if (source == nullptr || emitter == nullptr)
        return;
    *reinterpret_cast<int*>(reinterpret_cast<unsigned char*>(source) + 0x114u) = -1;
}
void          nslUpdateBanks() {
    if (nsl_initParams.aramBase == 0 || nsl_waveBankSlots == nullptr)
        return;

    int loadIndex = -1;
    unsigned loadOrder = static_cast<unsigned>(-1);
    for (unsigned i = 0; i < nsl_initParams.aramBase; ++i) {
        nslWaveBankSlot* slot = &nsl_waveBankSlots[i];
        if (slot->state == NSL_WAVE_BANK_SLOT_STATE_PENDING) {
            if (loadOrder > slot->loadOrder) {
                loadOrder = slot->loadOrder;
                loadIndex = static_cast<int>(i);
            }
        } else if (slot->state == NSL_WAVE_BANK_SLOT_STATE_LOADING) {
            const nslWaveBankLoaderState state = nslWaveBankLoaderUpdate(&nsl_waveBankLoad);
            if (state == NSL_WAVE_BANK_LOADER_STATE_COMPLETED) {
                slot->waveBank = nsl_waveBankLoad.waveBank;
                slot->state = NSL_WAVE_BANK_SLOT_STATE_LOADED;
                slot->profile.timeLoaded = nsl_time;
                slot->profile.frameLoaded = nsl_frame;
                nslWaveBankSort(slot->waveBank);
                loadIndex = -1;
            } else if (state == NSL_WAVE_BANK_LOADER_STATE_CANCELED || state < 0) {
                slot->state = NSL_WAVE_BANK_SLOT_STATE_LOADED;
                nslWaveBankFree(slot->waveBankID);
            }
            nsl_waveBankLoad.state = NSL_WAVE_BANK_LOADER_STATE_INITIAL;
        }
    }

    if (loadIndex >= 0 && nsl_waveBankLoad.state == NSL_WAVE_BANK_LOADER_STATE_INITIAL) {
        nslWaveBankSlot* slot = &nsl_waveBankSlots[loadIndex];
        slot->profile.timeStarted = nsl_time;
        slot->profile.frameStarted = nsl_frame;
        slot->state = NSL_WAVE_BANK_SLOT_STATE_LOADING;
        nslWaveBankLoaderInit(&nsl_waveBankLoad, slot->flags, slot->file, slot->fileOffset);
        nslWaveBankLoaderUpdate(&nsl_waveBankLoad);
    }
}
// ea: 0x00826A00
void          nslStart(void* work) {
    if (work == nullptr || nsl_work != nullptr)
        return;
    nsl_workLimit = nsl_workUsed;
    nsl_workUsed = 0;
    nsl_work = work;
    if (nslInit(nullptr) < 0)
        return;
    std::memset(nsl_work, 0, nsl_workUsed);
    if (nsl_initParams.aramBase != 0 && nsl_waveBankSlots != nullptr)
        nsl_waveBankSlots->waveBankID = nsl_initParams.aramBase;
    nslSlotPoolInit(&nsl_sourcePool, nsl_sourceEntries,
                    static_cast<int>(nsl_initParams.maxSources), 12u);
    nslSlotPoolInit(&nsl_emitterPool, nsl_emitterEntries,
                    static_cast<int>(nsl_initParams.maxEmitters), 12u);
}
void          nslExit() {}
void          nslSetEffect(const void*) {}
void          nslSetListenerPosition(const float*) {}
void          nslSetListenerOrientation(const float*, const float*) {}
unsigned int  nslWaveGetHash(nslWaveID) { return 0; }
int           nslGetWaveLength(nslWaveID) { return 0; }      // ?nslGetWaveLength@@YAHW4nslWaveID@@@Z
// ea: 0x00820DD0
unsigned      nslGetSourceLength(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return 0;
    return *reinterpret_cast<const unsigned*>(
        reinterpret_cast<const unsigned char*>(source) + 0x12Cu);
}  // ?nslGetSourceLength@@YAIW4nslSourceID@@@Z
int           nslIsWaveLooped(nslWaveID) { return 0; }       // ?nslIsWaveLooped@@YAHW4nslWaveID@@@Z
void          nslPauseSource(nslSourceID) {}                 // ?nslPauseSource@@YAXW4nslSourceID@@@Z
void          nslUnpauseSource(nslSourceID) {}               // ?nslUnpauseSource@@YAXW4nslSourceID@@@Z
void          nslPlaySource(nslSourceID) {}                  // ?nslPlaySource@@YAXW4nslSourceID@@@Z
void          nslDampenGuardSource(nslSourceID) {}           // ?nslDampenGuardSource@@YAXW4nslSourceID@@@Z
int           nslAreAllBanksLoaded() { return nslWaveBankSlotsGetLoadingCount() == 0; } // ?nslAreAllBanksLoaded@@YAHXZ
int           nslNumBanksInUse() { return static_cast<int>(nslWaveBankSlotsGetUsedCount()); } // ?nslNumBanksInUse@@YAHXZ

// ============================================================================
// nslAram â€” audio RAM accounting (nslAram.o)
// ============================================================================
void*         nsl_aramBase = nullptr; // ?nsl_aramBase@@3PAXA (nslAram.o)
unsigned int nsl_aramSize = 0;        // ?nsl_aramSize@@3IA (nslAram.o)
unsigned int nsl_aramFree = 0;        // ?nsl_aramFree@@3IA (nslAram.o)
unsigned int nsl_aramAlignment = 0x480; // ?nsl_aramAlignment@@3IA (nslAram.o)
int           nsl_aramStackTop = 64;  // ?nsl_aramStackTop@@3HA (nslAram.o)
int           nsl_aramStackBottom = -1; // ?nsl_aramStackBottom@@3HA (nslAram.o)
unsigned int  nsl_aramStack[64] = {}; // ?nsl_aramStack@@3PAIA (nslAram.o)
unsigned char ignoreAssert_4 = 0;
unsigned char ignoreAssert_5 = 0;
unsigned char ignoreAssert_6 = 0;
unsigned char ignoreAssert_7 = 0;
unsigned char ignoreAssert_8 = 0;
void          nslSourceSetMode(nslSourceID, unsigned) {}
void          nslSourceSetLooping(nslSourceID, bool) {}
void          nslSourceSetPriority(nslSourceID, unsigned) {}
float         nslSourceGetVolume(nslSourceID) { return 1.0f; }
unsigned      nslSourceCount() { return 0; }
nslSourceID   nslSourceGetFirst() { return NSL_INVALID_SOURCE; }
nslSourceID   nslSourceGetNext(nslSourceID) { return NSL_INVALID_SOURCE; }
bool          nslSourceIs3D(nslSource*) { return false; }
float         nslSourceGetAttenuation(nslSource*) { return 1.0f; }
void          nslSourceSetDopplerFactor(nslSourceID, float) {}
void          nslSourceSetReverbSend(nslSourceID, float) {}
void          nslSourceSetBusSend(nslSourceID, unsigned, float) {}
void          nslSourceSetFilter(nslSourceID, unsigned, float) {}
void          nslSourceSetOcclusion(nslSourceID, float) {}

// ============================================================================
// nslWave — wave data
// ============================================================================
nslWaveID     nslWaveLoad(const char*, unsigned) { return NSL_INVALID_WAVE; }
nslWaveID     nslWaveLoadInPlace(void*, unsigned) { return NSL_INVALID_WAVE; }
void          nslWaveRelease(nslWaveID) {}
void          nslWaveSetSourceDirectory(const char*) {}
const char*   nslWaveGetSourceDirectory() { return ""; }
void          nslWaveSetObjectDirectory(const char*) {}
const char*   nslWaveGetObjectDirectory() { return ""; }
nslWave*      nslWavePtr(nslWaveID) { return nullptr; }
unsigned      nslWaveCount() { return 0; }
unsigned      nslWaveGetSize(nslWaveID) { return 0; }
unsigned      nslWaveGetFormat(nslWaveID) { return 0; }
unsigned      nslWaveGetSampleRate(nslWaveID) { return 0; }
unsigned      nslWaveGetChannels(nslWaveID) { return 0; }
bool          nslWaveIsStreaming(nslWaveID) { return false; }
nslWaveID     nslWaveGetFirst() { return NSL_INVALID_WAVE; }
nslWaveID     nslWaveGetNext(nslWaveID) { return NSL_INVALID_WAVE; }

// ============================================================================
// nslWaveBank — wave bank (collection of waves)
// ============================================================================
nslWaveBankID nslWaveBankLoad(nflFileID file, unsigned fileOffset, unsigned flags) {
    if (file == NFL_FILE_ID_INVALID || nsl_initParams.aramBase == 0)
        return NSL_INVALID_BANK;

    unsigned slotIndex = 0;
    nslWaveBankSlot* slot = nsl_waveBankSlots;
    for (; slot->state != NSL_WAVE_BANK_SLOT_STATE_NOTUSED; ++slot) {
        if (++slotIndex >= nsl_initParams.aramBase)
            return NSL_INVALID_BANK;
    }

    const unsigned generation = slotIndex +
        (((nsl_initParams.aramBase & 0xffffu) *
          (((slot->waveBankID >> 16) / nsl_initParams.aramBase) + 1u)) & 0x7fffu);
    slot->profile.timeCreated = static_cast<unsigned>(-1);
    slot->profile.timeStarted = static_cast<unsigned>(-1);
    slot->profile.timeLoaded = static_cast<unsigned>(-1);
    slot->profile.frameCreated = static_cast<unsigned>(-1);
    slot->profile.frameStarted = static_cast<unsigned>(-1);
    slot->profile.frameLoaded = static_cast<unsigned>(-1);
    slot->profile.timeCreated = nsl_time;
    slot->waveBankID = (generation << 16) | 0xffffu;
    slot->profile.frameCreated = nsl_frame;
    slot->flags = flags;
    slot->file = file;
    slot->state = NSL_WAVE_BANK_SLOT_STATE_PENDING;
    slot->fileOffset = fileOffset;
    slot->loadOrder = nsl_waveBankLoadOrder++;
    slot->waveBank = nullptr;
    return slot->waveBankID;
}
nslWaveBankID nslWaveBankLoad(const char*) { return NSL_INVALID_BANK; }
nslWaveBankID nslWaveBankLoadInPlace(void*) { return NSL_INVALID_BANK; }
void          nslWaveBankRelease(nslWaveBankID) {}
// ea: 0x00826EE0
nslWaveBankSlot* nslWaveBankGetSlot(nslWaveBankID waveBankID) {
    if (nsl_initParams.aramBase == 0 || nsl_waveBankSlots == nullptr)
        return nullptr;
    const unsigned index = (waveBankID >> 16) % nsl_initParams.aramBase;
    nslWaveBankSlot* slot = &nsl_waveBankSlots[index];
    return slot->waveBankID == waveBankID ? slot : nullptr;
}
// ea: 0x00827420
nslWaveBank*  nslWaveBankPtr(nslWaveBankID waveBankID) {
    nslWaveBankSlot* slot = nslWaveBankGetSlot(waveBankID);
    return slot != nullptr && slot->state == NSL_WAVE_BANK_SLOT_STATE_LOADED
        ? slot->waveBank : nullptr;
}
unsigned      nslWaveBankCount() { return 0; }
nslWaveBankID nslWaveBankGetFirst() { return NSL_INVALID_BANK; }
nslWaveBankID nslWaveBankGetNext(nslWaveBankID) { return NSL_INVALID_BANK; }
// ea: 0x008274C0
nflFileID      nslWaveBankGetFile(nslWaveBankID waveBankID) {
    if (nsl_initParams.aramBase != 0 && nsl_waveBankSlots != nullptr) {
        const unsigned index = (waveBankID >> 16) % nsl_initParams.aramBase;
        nslWaveBankSlot* slot = &nsl_waveBankSlots[index];
        if (slot->waveBankID == waveBankID && slot->waveBank != nullptr)
            return slot->file;
    }
    return NFL_FILE_ID_INVALID;
}
// ea: 0x00827510
unsigned       nslWaveBankGetFileOffset(nslWaveBankID waveBankID) {
    if (nsl_initParams.aramBase != 0 && nsl_waveBankSlots != nullptr) {
        const unsigned index = (waveBankID >> 16) % nsl_initParams.aramBase;
        nslWaveBankSlot* slot = &nsl_waveBankSlots[index];
        if (slot->waveBankID == waveBankID && slot->waveBank != nullptr)
            return slot->fileOffset;
    }
    return 0;
}

static nslWaveBank* nslWaveBankData(nslWaveBankID waveBankID) {
    nslWaveBankSlot* slot = nslWaveBankGetSlot(waveBankID);
    return slot != nullptr ? slot->waveBank : nullptr;
}

// ea: 0x00826F20
void*          nslWaveBankGetAram(nslWaveBankID waveBankID) {
    nslWaveBankSlot* slot = nslWaveBankGetSlot(waveBankID);
    if (slot == nullptr || slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED ||
        slot->waveBank == nullptr)
        return nullptr;
    return slot->waveBank->storage.backing.waveBankAram;
}

// ea: 0x00827470
const char*    nslWaveBankGetName(nslWaveBankID waveBankID) {
    nslWaveBank* waveBank = nslWaveBankData(waveBankID);
    return waveBank != nullptr ? waveBank->name : nullptr;
}

// ea: 0x00827630
int            nslWaveBankGetAramSize(nslWaveBankID waveBankID) {
    nslWaveBank* waveBank = nslWaveBankData(waveBankID);
    return waveBank != nullptr ? static_cast<int>(waveBank->aramSize) : -1;
}

// ea: 0x00827680
int            nslWaveBankGetTextSize(nslWaveBankID waveBankID) {
    nslWaveBank* waveBank = nslWaveBankData(waveBankID);
    return waveBank != nullptr ? static_cast<int>(waveBank->textSize) : -1;
}

// ea: 0x008276D0
int            nslWaveBankGetInfoSize(nslWaveBankID waveBankID) {
    nslWaveBank* waveBank = nslWaveBankData(waveBankID);
    return waveBank != nullptr ? static_cast<int>(waveBank->infoSize) : -1;
}

// ea: 0x00827720
int            nslWaveBankGetStreamSize(nslWaveBankID waveBankID) {
    nslWaveBank* waveBank = nslWaveBankData(waveBankID);
    return waveBank != nullptr ? static_cast<int>(waveBank->streamSize) : -1;
}

// ea: 0x00827770
nslWaveBankID nslWaveBankGet(unsigned index) {
    if (index < nsl_initParams.aramBase) {
        nslWaveBankSlot* slot = &nsl_waveBankSlots[index];
        if (slot->state != NSL_WAVE_BANK_SLOT_STATE_NOTUSED)
            return slot->waveBankID;
    }
    return NSL_INVALID_BANK;
}

int           nslWaveBankGetState(nslWaveBankID waveBankID) {
    if (nsl_initParams.aramBase == 0)
        return -1;
    const unsigned index = (waveBankID >> 16) % nsl_initParams.aramBase;
    nslWaveBankSlot* slot = &nsl_waveBankSlots[index];
    if (slot->waveBankID != waveBankID)
        return -1;
    if (slot->state == NSL_WAVE_BANK_SLOT_STATE_PENDING)
        return 2;
    if (slot->state == NSL_WAVE_BANK_SLOT_STATE_LOADING)
        return 1;
    if (slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED)
        return -1;
    return 0;
}

// ea: 0x00827B00
void*          nslWaveBankGetAram(const nslWaveBank* waveBank) {
    if (waveBank != nullptr && (waveBank->waveBankFlags & 0x80u) == 0)
        return waveBank->storage.backing.waveBankAram;
    return nullptr;
}

// ea: 0x00826FE0
nslWaveBankID nslWaveGetBank(nslWaveID waveID) {
    const nslWaveBankID waveBankID =
        static_cast<nslWaveBankID>(static_cast<unsigned>(waveID) | 0xffffu);
    if (nsl_initParams.aramBase == 0 || nsl_waveBankSlots == nullptr)
        return NSL_INVALID_BANK;

    nslWaveBankSlot* slot = &nsl_waveBankSlots[
        (waveBankID >> 16) % nsl_initParams.aramBase];
    if (slot->waveBankID != waveBankID ||
        slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED ||
        slot->waveBank == nullptr ||
        static_cast<unsigned>(waveID) > slot->waveBank->waveCount)
        return NSL_INVALID_BANK;
    return waveBankID;
}

// ea: 0x00827C40
nflFileID      nslWaveGetFile(nslWaveID waveID) {
    const nslWaveBankID waveBankID = nslWaveGetBank(waveID);
    nslWaveBankSlot* slot = nslWaveBankGetSlot(waveBankID);
    if (slot != nullptr && slot->waveBank != nullptr)
        return slot->file;
    return NFL_FILE_ID_INVALID;
}
void          nslWaveBankFree(nslWaveBankID waveBankID) {
    if (nsl_initParams.aramBase == 0 || nsl_waveBankSlots == nullptr)
        return;
    nslWaveBankSlot* slot = &nsl_waveBankSlots[(waveBankID >> 16) % nsl_initParams.aramBase];
    if (slot->waveBankID != waveBankID)
        return;
    if (slot->state == NSL_WAVE_BANK_SLOT_STATE_LOADING) {
        nslWaveBankLoaderCancel(&nsl_waveBankLoad);
        return;
    }
    if (slot->state == NSL_WAVE_BANK_SLOT_STATE_LOADED && slot->waveBank != nullptr) {
        nslAramFree(slot->waveBank->storage.backing.waveBankAram);
        nslMemoryFree(slot->waveBank);
        slot->waveBank = nullptr;
    }
    slot->waveBankID += nsl_initParams.aramBase << 16;
    slot->state = NSL_WAVE_BANK_SLOT_STATE_NOTUSED;
}
unsigned      nslWaveBankSlotsGetCount() { return nsl_initParams.aramBase; }
unsigned      nslWaveBankSlotsGetUsedCount() {
    unsigned result = 0;
    nslWaveBankSlot* slot = nsl_waveBankSlots;
    for (unsigned count = nsl_initParams.aramBase; count != 0; --count) {
        if (slot->state != NSL_WAVE_BANK_SLOT_STATE_NOTUSED)
            ++result;
        ++slot;
    }
    return result;
}
unsigned      nslWaveBankSlotsGetLoadingCount() {
    unsigned result = 0;
    nslWaveBankSlot* slot = nsl_waveBankSlots;
    for (unsigned count = nsl_initParams.aramBase; count != 0; --count, ++slot) {
        if (slot->state == NSL_WAVE_BANK_SLOT_STATE_LOADING ||
            slot->state == NSL_WAVE_BANK_SLOT_STATE_PENDING)
            ++result;
    }
    return result;
}
unsigned      nslWaveBankSlotsGetLoadedCount() {
    unsigned result = 0;
    nslWaveBankSlot* slot = nsl_waveBankSlots;
    for (unsigned count = nsl_initParams.aramBase; count != 0; --count, ++slot) {
        if (slot->state == NSL_WAVE_BANK_SLOT_STATE_LOADED)
            ++result;
    }
    return result;
}
int           nslWaveBankFixup(nslWaveBank*) { return 0; }
// ea: 0x00827A00
nflFileID      nslWaveBankGetFile(const nslWaveBank* waveBank, unsigned* waveBankFileOffset) {
    if (waveBank == nullptr || (waveBank->waveBankFlags & 0x80u) != 0)
        return NFL_FILE_ID_INVALID;
    if (waveBankFileOffset != nullptr)
        *waveBankFileOffset = waveBank->storage.backing.waveBankFileOffset;
    return waveBank->storage.backing.waveBankFile;
}
int           nslWaveBankSetAram(nslWaveBank* waveBank, void* waveBankAram) {
    if (waveBank == nullptr || (waveBank->waveBankFlags & 0x40u) == 0)
        return 0;
    waveBank->storage.backing.waveBankAram = waveBankAram;
    return 1;
}
int           nslWaveBankSetFile(nslWaveBank* waveBank, nflFileID waveBankFile,
                                 unsigned waveBankFileOffset) {
    if (waveBank == nullptr || (waveBank->waveBankFlags & 0x40u) == 0)
        return 0;
    waveBank->storage.backing.waveBankFile = waveBankFile;
    waveBank->storage.backing.waveBankFileOffset = waveBankFileOffset;
    return 1;
}
void          nslWaveBankSort(nslWaveBank*) {}
// ea: 0x00827560
nslWaveID     nslWaveBankGetWave(nslWaveBankID waveBankID, unsigned index) {
    nslWaveBankSlot* slot = nslWaveBankGetSlot(waveBankID);
    if (slot == nullptr || slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED ||
        slot->waveBank == nullptr || index >= slot->waveBank->waveCount)
        return NSL_INVALID_WAVE;
    return static_cast<nslWaveID>(index | (slot->waveBankID & 0xffff0000u));
}
nslWaveID     nslWaveBankGetWave(nslWaveBankID, const char*) { return NSL_INVALID_WAVE; }
nslWaveID     nslWaveBankGetWaveByIndex(nslWaveBankID, unsigned) { return NSL_INVALID_WAVE; }
// ea: 0x008275E0
int           nslWaveBankGetWaveCount(nslWaveBankID waveBankID) {
    nslWaveBankSlot* slot = nslWaveBankGetSlot(waveBankID);
    return slot != nullptr && slot->waveBank != nullptr
        ? static_cast<int>(slot->waveBank->waveCount) : -1;
}
// ea: 0x008275C0
const char*   nslWaveBankGetWaveName(nslWaveBankID waveBankID, unsigned index) {
    return nslWaveGetName(nslWaveBankGetWave(waveBankID, index));
}
// ea: 0x00826EA0
int           nslWaveNameCompareText(const nslWaveName* waveNameA,
                                     const nslWaveName* waveNameB) {
    return _stricmp(waveNameA->name, waveNameB->name);
}
// ea: 0x00826EC0
int           nslWaveNameCompareHash(const nslWaveName* waveNameA,
                                     const nslWaveName* waveNameB) {
    if (waveNameB->hash <= waveNameA->hash)
        return waveNameB->hash < waveNameA->hash;
    return -1;
}

// ea: 0x00827840
int           nslWaveBankLookup(const nslWaveBank* waveBank,
                                const nslWaveName* waveName) {
    if (waveBank == nullptr || waveName == nullptr)
        return -1;

    using BsearchCompare = int (__cdecl *)(const void*, const void*);
    const BsearchCompare compare = (waveBank->waveBankFlags & 2u) != 0
        ? reinterpret_cast<BsearchCompare>(&nslWaveNameCompareHash)
        : reinterpret_cast<BsearchCompare>(&nslWaveNameCompareText);
    const nslWaveName* found = static_cast<const nslWaveName*>(
        std::bsearch(waveName, waveBank->names, waveBank->waveCount,
                     sizeof(nslWaveName), compare));
    if (found == nullptr)
        return -1;

    const int foundIndex = static_cast<int>(found - waveBank->names);
    if (foundIndex <= -1)
        return foundIndex;

    int firstElem = foundIndex;
    unsigned lastElem = static_cast<unsigned>(foundIndex);
    if ((waveBank->waveBankFlags & 2u) != 0) {
        const unsigned hash = found->hash;
        int index = foundIndex;
        while (index >= 0 && waveBank->names[index].hash == hash) {
            --index;
            --firstElem;
        }
        while (lastElem < waveBank->waveCount &&
               waveBank->names[lastElem].hash == hash) {
            ++lastElem;
        }
    } else {
        const char* name = found->name;
        int index = foundIndex;
        while (index >= 0 && std::strcmp(waveBank->names[index].name, name) == 0) {
            --index;
            --firstElem;
        }
        while (lastElem < waveBank->waveCount &&
               std::strcmp(waveBank->names[lastElem].name, found->name) == 0) {
            ++lastElem;
        }
    }

    const float randomOffset = static_cast<float>(std::rand()) *
        static_cast<float>(lastElem - static_cast<unsigned>(firstElem) - 2u) *
        -0.000030518509f;
    return firstElem - static_cast<int>(randomOffset) + 1;
}

// ea: 0x00827DC0
int           nslWaveBankLookup(const nslWaveBank* waveBank, const char* waveName) {
    if (waveBank == nullptr || waveName == nullptr)
        return -1;
    nslWaveName key = {};
    if ((waveBank->waveBankFlags & 2u) != 0) {
        const tlFixedString fixedString(waveName);
        key.hash = fixedString.hash;
    } else {
        key.name = waveName;
    }
    return nslWaveBankLookup(waveBank, &key);
}

// ea: 0x00827E10
int           nslWaveBankLookup(const nslWaveBank* waveBank, unsigned waveNameHash) {
    if (waveBank == nullptr || (waveBank->waveBankFlags & 2u) == 0)
        return -1;
    nslWaveName key = {};
    key.hash = waveNameHash;
    return nslWaveBankLookup(waveBank, &key);
}

// ea: 0x00827BD0
nslWaveID     nslWaveLookup(const nslWaveName* waveName) {
    if (waveName == nullptr || nsl_initParams.aramBase == 0 ||
        nsl_waveBankSlots == nullptr)
        return NSL_INVALID_WAVE;
    for (unsigned index = 0; index < nsl_initParams.aramBase; ++index) {
        nslWaveBankSlot* slot = &nsl_waveBankSlots[index];
        if (slot->state == NSL_WAVE_BANK_SLOT_STATE_LOADED) {
            const int waveIndex = nslWaveBankLookup(slot->waveBank, waveName);
            if (waveIndex != -1)
                return static_cast<nslWaveID>(waveIndex |
                    (slot->waveBankID & 0xffff0000u));
        }
    }
    return NSL_INVALID_WAVE;
}

// ea: 0x00827E40
nslWaveID     nslWaveLookup(const char* waveName) {
    if (waveName == nullptr || nsl_initParams.aramBase == 0 ||
        nsl_waveBankSlots == nullptr)
        return NSL_INVALID_WAVE;
    for (unsigned index = 0; index < nsl_initParams.aramBase; ++index) {
        nslWaveBankSlot* slot = &nsl_waveBankSlots[index];
        nslWaveBank* waveBank = slot->waveBank;
        if (slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED || waveBank == nullptr)
            continue;
        nslWaveName key = {};
        if ((waveBank->waveBankFlags & 2u) != 0) {
            const tlFixedString fixedString(waveName);
            key.hash = fixedString.hash;
        } else {
            key.name = waveName;
        }
        const int waveIndex = nslWaveBankLookup(waveBank, &key);
        if (waveIndex != -1)
            return static_cast<nslWaveID>(waveIndex |
                (slot->waveBankID & 0xffff0000u));
    }
    return NSL_INVALID_WAVE;
}

// ea: 0x00827EF0
nslWaveID     nslWaveLookup(unsigned waveNameHash) {
    if (waveNameHash == 0 || nsl_initParams.aramBase == 0 ||
        nsl_waveBankSlots == nullptr)
        return NSL_INVALID_WAVE;
    nslWaveName key = {};
    key.hash = waveNameHash;
    for (unsigned index = 0; index < nsl_initParams.aramBase; ++index) {
        nslWaveBankSlot* slot = &nsl_waveBankSlots[index];
        nslWaveBank* waveBank = slot->waveBank;
        if (slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED || waveBank == nullptr ||
            (waveBank->waveBankFlags & 2u) == 0)
            continue;
        const int waveIndex = nslWaveBankLookup(waveBank, &key);
        if (waveIndex != -1)
            return static_cast<nslWaveID>(waveIndex |
                (slot->waveBankID & 0xffff0000u));
    }
    return NSL_INVALID_WAVE;
}

// ============================================================================
// nslWaveBankSort
// ============================================================================
void          nslWaveBankSortSwap(nslWaveName*, nslWave*, unsigned, unsigned) {}
void          nslWaveBankSortRecursive(nslWaveBank*, int, int, int (*cmp)(const nslWaveName*, const nslWaveName*)) {}

// ============================================================================
// nslWaveBankLoader — async wave bank loading
// ============================================================================
void          nslWaveBankLoaderInit(nslWaveBankLoader* waveBankLoader,
                                    unsigned waveBankLoadFlags, nflFileID file,
                                    unsigned fileOffset) {
    std::memset(waveBankLoader, 0, sizeof(nslWaveBankLoader));
    waveBankLoader->flags = waveBankLoadFlags;
    waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_INITIAL;
    waveBankLoader->file = file;
    waveBankLoader->rid = NFL_REQUEST_ID_INVALID;
    waveBankLoader->fileOffset = fileOffset;
    waveBankLoader->waveBank = reinterpret_cast<nslWaveBank*>(nsl_waveBankLoaderBuffer);
    waveBankLoader->stid = -1;
}
nslWaveBankLoaderState nslWaveBankLoaderUpdate(nslWaveBankLoader* waveBankLoader) {
    if (waveBankLoader->state == NSL_WAVE_BANK_LOADER_STATE_CANCELING) {
        if (waveBankLoader->rid != NFL_REQUEST_ID_INVALID &&
            nflGetRequestState(waveBankLoader->rid) == NFL_REQUEST_STATE_INVALID)
            waveBankLoader->rid = NFL_REQUEST_ID_INVALID;
        if (waveBankLoader->rid == NFL_REQUEST_ID_INVALID && waveBankLoader->stid == -1) {
            waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_CANCELED;
            return NSL_WAVE_BANK_LOADER_STATE_CANCELED;
        }
    }

    if (waveBankLoader->state == NSL_WAVE_BANK_LOADER_STATE_INITIAL)
        waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_START_READ_HEADER;
    if (waveBankLoader->state == NSL_WAVE_BANK_LOADER_STATE_START_READ_HEADER) {
        waveBankLoader->rid = nflReadFileAsync(waveBankLoader->file, waveBankLoader->fileOffset,
                                                waveBankLoader->waveBank, 0x1000u);
        if (waveBankLoader->rid != NFL_REQUEST_ID_INVALID)
            waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_HEADER;
    }
    if (waveBankLoader->state == NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_HEADER &&
        waveBankLoader->rid != NFL_REQUEST_ID_INVALID &&
        nflGetRequestState(waveBankLoader->rid) == NFL_REQUEST_STATE_INVALID) {
        nslWaveBank* waveBank = waveBankLoader->waveBank;
        waveBankLoader->rid = NFL_REQUEST_ID_INVALID;
        if (std::memcmp(waveBank->header, "NSLB", 4) != 0 ||
            waveBank->versionMajor != 1 || waveBank->versionMinor != 0) {
            waveBankLoader->state = NSL_WAVE_BANK_LOADER_ERROR_INVALIDBANK;
            return waveBankLoader->state;
        }
        waveBankLoader->waveBankSize = waveBank->waveBankSize;
        waveBankLoader->aramOffset = waveBank->aramOffset;
        waveBankLoader->aramSize = waveBank->aramSize;
        waveBankLoader->waveBank = static_cast<nslWaveBank*>(
            nslMemoryAlloc(waveBankLoader->waveBankSize));
        if (waveBankLoader->waveBank != nullptr && waveBankLoader->aramSize != 0) {
            waveBankLoader->aramSize = (waveBankLoader->aramSize + 4095u) & 0xfffff000u;
            waveBankLoader->aram = nslAramAlloc(waveBankLoader->aramSize, waveBankLoader->flags);
        }
        if (waveBankLoader->waveBank == nullptr ||
            (waveBankLoader->aramSize != 0 && waveBankLoader->aram == nullptr)) {
            nslAramFree(waveBankLoader->aram);
            nslMemoryFree(waveBankLoader->waveBank);
            waveBankLoader->waveBank = nullptr;
            waveBankLoader->state = NSL_WAVE_BANK_LOADER_ERROR_NOMEMORY;
            return waveBankLoader->state;
        }
        waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_START_READ_DIR_SECTION;
    }
    if (waveBankLoader->state == NSL_WAVE_BANK_LOADER_STATE_START_READ_DIR_SECTION) {
        waveBankLoader->rid = nflReadFileAsync(waveBankLoader->file, waveBankLoader->fileOffset,
                                                waveBankLoader->waveBank,
                                                waveBankLoader->waveBankSize);
        if (waveBankLoader->rid != NFL_REQUEST_ID_INVALID)
            waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_DIR_SECTION;
    }
    if (waveBankLoader->state == NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_DIR_SECTION &&
        waveBankLoader->rid != NFL_REQUEST_ID_INVALID &&
        nflGetRequestState(waveBankLoader->rid) == NFL_REQUEST_STATE_INVALID) {
        waveBankLoader->rid = NFL_REQUEST_ID_INVALID;
        waveBankLoader->state = waveBankLoader->aramSize == 0
            ? NSL_WAVE_BANK_LOADER_STATE_START_READ_PRIMED_STREAMS
            : NSL_WAVE_BANK_LOADER_STATE_START_READ_ARAM_SECTION;
    }
    if (waveBankLoader->state == NSL_WAVE_BANK_LOADER_STATE_START_READ_ARAM_SECTION) {
        waveBankLoader->rid = nflReadFileAsync(
            waveBankLoader->file, waveBankLoader->fileOffset + waveBankLoader->aramOffset,
            waveBankLoader->aram, waveBankLoader->aramSize);
        if (waveBankLoader->rid != NFL_REQUEST_ID_INVALID)
            waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_ARAM_SECTION;
    }
    if (waveBankLoader->state == NSL_WAVE_BANK_LOADER_STATE_CHECK_READ_ARAM_SECTION &&
        waveBankLoader->rid != NFL_REQUEST_ID_INVALID &&
        nflGetRequestState(waveBankLoader->rid) == NFL_REQUEST_STATE_INVALID) {
        waveBankLoader->rid = NFL_REQUEST_ID_INVALID;
        waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_START_READ_PRIMED_STREAMS;
    }
    if (waveBankLoader->state == NSL_WAVE_BANK_LOADER_STATE_START_READ_PRIMED_STREAMS) {
        waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_COMPLETED;
        nslWaveBankFixup(waveBankLoader->waveBank);
        nslWaveBankSetAram(waveBankLoader->waveBank, waveBankLoader->aram);
        nslWaveBankSetFile(waveBankLoader->waveBank, waveBankLoader->file,
                           waveBankLoader->fileOffset);
    }
    return waveBankLoader->state;
}
void          nslWaveBankLoaderCancel(nslWaveBankLoader* waveBankLoader) {
    if (waveBankLoader->state != NSL_WAVE_BANK_LOADER_STATE_CANCELED &&
        waveBankLoader->state != NSL_WAVE_BANK_LOADER_STATE_CANCELING) {
        waveBankLoader->state = NSL_WAVE_BANK_LOADER_STATE_CANCELING;
        if (waveBankLoader->rid != NFL_REQUEST_ID_INVALID)
            nflCancelRequest(waveBankLoader->rid);
    }
}

// ============================================================================
// nslGroup — sound groups
// ============================================================================
nslGroupID    nslGroupCreate(const char*) { return 0; }
void          nslGroupDestroy(nslGroupID) {}

static nslGroup* nslGroupAt(unsigned index) {
    const uintptr_t base = reinterpret_cast<uintptr_t>(nsl_groups);
    return reinterpret_cast<nslGroup*>(base +
                                       static_cast<uintptr_t>(nslGroupStride) * index);
}

// ea: 0x00823950
nslGroup* nslGroupGet(unsigned index) {
    if (index >= static_cast<unsigned>(dword_E4B690))
        return nullptr;

    nslGroup* result = nslGroupAt(index);
    return result->name[0] == 0 ? nullptr : result;
}

// ea: 0x00823980
nslGroup* nslGroupGet(const char* groupName) {
    if (groupName == nullptr || *groupName == 0)
        return nullptr;

    const unsigned groupCount = static_cast<unsigned>(dword_E4B690);
    unsigned index = 0;
    for (; index < groupCount; ++index) {
        nslGroup* group = nslGroupAt(index);
        if (group->name[0] == 0)
            break;
        if (_strnicmp(groupName, group->name, 0x18u) == 0)
            return group;
    }

    if (index == groupCount) {
        txPrintf("NSL", 0, "Out of sound groups. Maximum allowed is %d\n", 0);
        return nullptr;
    }

    nslGroup* group = nslGroupAt(index);
    group->paramsUpdate = UINT64_C(0xFFFFFFFFFFFFFFFF);
    group->params[0] = 1.0f;
    group->params[1] = 1.0f;
    std::strncpy(group->name, groupName, 0x18u);
    return group;
}

// ea: 0x00823A70
void nslGroupSetParam(const char* name, int index, float value) {
    nslGroup* group = nslGroupGet(name);
    if (group != nullptr) {
        group->paramsUpdate |= UINT64_C(1) << index;
        group->params[index] = value;
    }
}

// ea: 0x00823AC0
float nslGroupGetParam(const char* name, int index, float defaultValue) {
    nslGroup* group = nslGroupGet(name);
    return group != nullptr ? group->params[index] : defaultValue;
}

// ea: 0x00823B00
void nslGroupSetVolume(const char* groupName) {
    nslGroupGet(groupName);
}

// ea: 0x00823B20
float nslGroupGetVolume(const char* groupName, float defaultVolume) {
    nslGroup* group = nslGroupGet(groupName);
    return group != nullptr ? group->params[0] : defaultVolume;
}

void          nslGroupSetParam(const char*, unsigned, float) {}
void          nslGroupSetVolume(nslGroupID, float) {}
void          nslGroupSetPitch(nslGroupID, float) {}
float         nslGroupGetVolume(nslGroupID) { return 1.0f; }

// ============================================================================
// nslMaster — master bus
// ============================================================================
// ea: 0x00826580
nslGroup*     nslMasterGetGroup() { return nslGroupGet("NSL_MASTER"); }
float         nslBusVolume = 1.0f;  // ?nslBusVolume@@3MA @ 0xE4B674
float         nslBusPitch = 1.0f;   // ?nslBusPitch@@3MA @ 0xE4B678
int           nslTotalPitchBuses = 0;
int           nslTotalVolumeBuses = 0;
unsigned      nslBusIdPitch[10] = {};
unsigned      nslBusIdVolume[10] = {};

// ea: 0x00826590
bool          nslIsBusVolumeName(unsigned busId) {
    return busId == nslBusIdVolume[0];
}

// ea: 0x008265B0
void          nslSetBusVolume(unsigned busId, float volume) {
    nslBusIdVolume[0] = busId;
    nslBusVolume = volume;
    nslTotalVolumeBuses = 1;
}

// ea: 0x008265E0
void          nslSetBusPitch(unsigned busId, float pitch) {
    nslBusIdPitch[0] = busId;
    nslBusPitch = pitch;
    nslTotalPitchBuses = 1;
}

// ea: 0x00826610
void          nslSetBusVolume(float volume) { nslBusVolume = volume; }
// ea: 0x00826630
void          nslSetBusPitch(float pitch) { nslBusPitch = pitch; }

// ea: 0x00826650
void          nslSetBusVolumeAddBus(unsigned busId) {
    const int count = nslTotalVolumeBuses;
    if (count < 10) {
        nslBusIdVolume[count] = busId;
        nslTotalVolumeBuses = count + 1;
    }
}

// ea: 0x00826670
void          nslSetBusPitchAddBus(unsigned busId) {
    const int count = nslTotalPitchBuses;
    if (count < 10) {
        nslBusIdPitch[count] = busId;
        nslTotalPitchBuses = count + 1;
    }
}

// ea: 0x00826690
int           nslGetBusIdVolumeCount() { return nslTotalVolumeBuses; }
// ea: 0x008266A0
int           nslGetBusIdPitchCount() { return nslTotalPitchBuses; }

// ea: 0x008266B0
void          nslSetBusVolumeRemoveBus(unsigned busId) {
    const int originalCount = nslTotalVolumeBuses;
    int removedCount = 0;
    int index = 0;
    if (originalCount > 0) {
        int remainingCount = originalCount;
        do {
            if (nslBusIdVolume[index] == busId) {
                ++removedCount;
                --remainingCount;
            }
            if (index < remainingCount)
                nslBusIdVolume[index] = nslBusIdVolume[index + removedCount];
            ++index;
        } while (index < originalCount);
    }
    nslTotalVolumeBuses = originalCount - removedCount;
}

// ea: 0x00826710
void          nslSetBusPitchRemoveBus(unsigned busId) {
    const int originalCount = nslTotalPitchBuses;
    int removedCount = 0;
    int index = 0;
    if (originalCount > 0) {
        int remainingCount = originalCount;
        do {
            if (nslBusIdPitch[index] == busId) {
                ++removedCount;
                --remainingCount;
            }
            if (index < remainingCount)
                nslBusIdPitch[index] = nslBusIdPitch[index + removedCount];
            ++index;
        } while (index < originalCount);
    }
    nslTotalPitchBuses = originalCount - removedCount;
}

// ea: 0x00826770
unsigned      nslGetBusIdVolume(int index) { return nslBusIdVolume[index]; }
// ea: 0x00826780
unsigned      nslGetBusIdPitch(int index) { return nslBusIdPitch[index]; }

// ea: 0x00826790
float         nslGetBusVolume() { return nslBusVolume; }
// ea: 0x008267A0
float         nslGetBusPitch() { return nslBusPitch; }

float         nslGetBusVolume(unsigned) { return 1.0f; }
const char*   nslGetBusName(unsigned) { return ""; }
unsigned      nslGetBusIndex(const char*) { return 0; }
void          nslSetBusFilter(unsigned, unsigned, float) {}
void          nslSetBusReverb(unsigned, float) {}
// ea: 0x00823B60
void          nslSetMasterVolume(float newVolume) {
    nslGroup* group = nslGroupGet("NSL_MASTER");
    if (group != nullptr) {
        group->paramsUpdate |= 1u;
        group->params[0] = newVolume;
    }
    const nslSpeakerMode speakerMode = nslGetSpeakerMode();
    nslSetSpeakerMode(speakerMode);
}

// ea: 0x00823BA0
float         nslGetMasterVolume() {
    nslGroup* group = nslGroupGet("NSL_MASTER");
    return group != nullptr ? group->params[0] : 0.0f;
}

// ============================================================================
// nslCompat — backward compatibility
// ============================================================================
nslVoice*     nslVoiceGet(int) { return nullptr; }
nslGroup*     nslGetGroup(const char*) { return nullptr; }
nslGroup*     nslGetListenerGroup() { return nullptr; }
void          nslCompatInit() {}
void          nslCompatUpdate() {}
unsigned      nslCompatGetVoiceCount() { return 0; }
bool          nslCompatIsVoicePlaying(int) { return false; }
void          nslCompatVoicePlay(int) {}
void          nslCompatVoiceStop(int) {}
void          nslCompatVoiceSetVolume(int, float) {}
void          nslCompatVoiceSetPitch(int, float) {}
void          nslCompatVoiceSetPan(int, float) {}

// ============================================================================
// nslListener — audio listener (3D ears)
// ============================================================================
static int    s_NumberOfListeners = 1;

// ea: 0x00823D50
int           nslGetNumberOfListeners() { return s_NumberOfListeners; }

// ea: 0x00823D60
void          nslSetNumberOfListeners(int listeners) {
    s_NumberOfListeners = listeners;
}

// ea: 0x00823D70
nslGroup*     nslListenerGetGroup(unsigned listenerIndex) {
    char listenerGroupName[16] = "NSL_LISTENER_0";
    if (listenerIndex >= 4)
        return nullptr;
    listenerGroupName[13] = static_cast<char>(listenerGroupName[13] + listenerIndex);
    return nslGroupGet(listenerGroupName);
}

// ea: 0x00823DD0
void          nslListenerSetMatrix(unsigned, const float (*)[4]) {}

// ea: 0x00823E60
void          nslListenerSetPosition(unsigned listenerIndex, const float* pos) {
    nslGroup* group = nslListenerGetGroup(listenerIndex);
    if (group != nullptr) {
        group->params[19] = pos[0];
        group->params[20] = pos[1];
        group->params[21] = pos[2];
        group->paramsUpdate |= UINT64_C(0x380000);
    }
}

// ea: 0x00823EA0
void          nslListenerSetVelocity(unsigned listenerIndex, const float* vel) {
    nslGroup* group = nslListenerGetGroup(listenerIndex);
    if (group != nullptr) {
        group->params[22] = vel[0];
        group->params[23] = vel[1];
        group->params[24] = vel[2];
        group->paramsUpdate |= UINT64_C(0x1C00000);
    }
}

// ea: 0x00823EE0
void          nslListenerSetOrientation(unsigned, const float*, const float*) {}
void          nslListenerSetDopplerFactor(unsigned, float) {}
void          nslUpdateListener() {}
void          nslUpdate() {}

// ============================================================================
// nslPriority — voice priority / attenuation
// ============================================================================
float         nslVoiceGetAttenuation(nslVoice*) { return 1.0f; }
unsigned      nslVoiceGetPriority(nslVoice*) { return 0; }
void          nslVoiceSetPriority(nslVoice*, unsigned) {}
void          nslSourceSetPriorityScale(nslSource*, float) {}

// ============================================================================
// nslVoice — voice allocation
// ============================================================================
nslVoice*     nslVoicePtr(int) { return nullptr; }
unsigned      nslVoiceCount() { return 0; }
int           nslVoiceAlloc(nslWaveID, nslSourceID, int) { return -1; }
void          nslVoiceFree(int) {}
int           nslVoiceGetState(int) { return NSL_VOICE_FREE; }
void          nslVoiceSetVolume(int, float) {}

// ============================================================================
// nslDriverXBOXDSOUND — Xbox DirectSound driver (XAudio2 replacement)
// ============================================================================
unsigned      nslDriverVoiceSize() { return 0; }
float         nslDriverClamp(float v, float lo, float hi) { return v<lo?lo:(v>hi?hi:v); }
void          nslDriverCalculateRolloff(float* out, float dist, const nslWave*) {}
void          nslDriverInit() {}
void          nslDriverShutdown() {}
void          nslDriverUpdate() {}
void          nslDriverSet3DEnabled(bool) {}
void          nslDriverSetSurroundEnabled(bool) {}
void          nslDriverSetReverbEnabled(bool) {}
void          nslDriverSetDSPEffect(unsigned, unsigned) {}
void          nslDriverSetOutputMode(unsigned) {}
void          nslDriverSetBufferSize(unsigned) {}

// ============================================================================
// nslAram — Xbox audio RAM management
// ============================================================================
// ea: 0x00422DC0
void*         nslAramGetBase() { return nsl_aramBase; }
// ea: 0x00422DD0
unsigned      nslAramGetSize() { return nsl_aramSize; }
// ea: 0x00422DE0
unsigned      nslAramGetFree() { return nsl_aramFree; }
// ea: 0x00422DF0
void          nslAramSetAlignment(unsigned alignment) { nsl_aramAlignment = alignment; }
// ea: 0x00422E00
unsigned      nslAramGetAlignment() { return nsl_aramAlignment; }
// ea: 0x00422E10
void          nslAramInit(void* base, unsigned size) {
    if (nsl_aramBase != nullptr || nsl_aramSize != 0 ||
        nsl_aramStackBottom != -1 || nsl_aramStackTop != 64) {
        txAssertFailed(&ignoreAssert_4,
                       "nsl_aramBase==0 && nsl_aramSize==0 && nsl_aramStackBottom==-1 && nsl_aramStackTop==TX_ARRAYSIZE(nsl_aramStack)",
                       "nslAramInit", "c:/cod/code/tl/nsl2/src/nsl/nslAram.cpp", 38);
    }
    nsl_aramStackTop = 64;
    nsl_aramStackBottom = -1;
    const uintptr_t baseAddress = reinterpret_cast<uintptr_t>(base);
    nsl_aramBase = reinterpret_cast<void*>(
        nsl_aramAlignment * ((baseAddress + nsl_aramAlignment - 1u) / nsl_aramAlignment));
    nsl_aramSize = nsl_aramAlignment * ((size - nsl_aramAlignment) / nsl_aramAlignment);
}
// ea: 0x00422EA0
unsigned      nslAramGetFreeBlock(unsigned* ptop, unsigned* pbottom) {
    int stackBottom = nsl_aramStackBottom;
    unsigned bottom = nsl_aramSize;
    unsigned top = 0;
    while (stackBottom >= 0) {
        if ((nsl_aramStack[stackBottom] & 0x80000000u) == 0)
            break;
        nsl_aramStack[stackBottom] = 0;
        --stackBottom;
    }
    nsl_aramStackBottom = stackBottom;
    for (int i = stackBottom; i >= 0; --i)
        top += nsl_aramStack[i] & 0x7fffffffu;

    int stackTop = nsl_aramStackTop;
    while (stackTop < 64) {
        if ((nsl_aramStack[stackTop] & 0x80000000u) == 0)
            break;
        nsl_aramStack[stackTop++] = 0;
    }
    nsl_aramStackTop = stackTop;
    for (int i = stackTop; i < 64; ++i)
        bottom -= nsl_aramStack[i] & 0x7fffffffu;

    if (top > bottom) {
        txAssertFailed(&ignoreAssert_5, "top <= bottom", "nslAramGetFreeBlock",
                       "c:/cod/code/tl/nsl2/src/nsl/nslAram.cpp", 73);
    }
    if (ptop != nullptr)
        *ptop = top;
    if (pbottom != nullptr)
        *pbottom = bottom;
    nsl_aramFree = bottom - top;
    return nsl_aramFree;
}
// ea: 0x00422F90
void*         nslAramAlloc(unsigned size, unsigned flags) {
    const unsigned alignedSize = nsl_aramAlignment *
        ((nsl_aramAlignment + size - 1u) / nsl_aramAlignment);
    unsigned top = 0;
    const unsigned freeBlock = nslAramGetFreeBlock(&top, &size);
    if (freeBlock < alignedSize) {
        txPrintf("NSL", 0, "Out of memory, %d requested, only %d free", alignedSize, freeBlock);
        return nullptr;
    }

    int stackTop = nsl_aramStackTop;
    if (nsl_aramStackBottom >= nsl_aramStackTop) {
        txAssertFailed(&ignoreAssert_6, "nsl_aramStackBottom < nsl_aramStackTop",
                       "nslAramAlloc", "c:/cod/code/tl/nsl2/src/nsl/nslAram.cpp", 93);
        stackTop = nsl_aramStackTop;
    }
    const int stackBottom = nsl_aramStackBottom + 1;
    if (stackBottom >= stackTop) {
        txPrintf("NSL", 0, "Out of stack, all %d blocks were used", 64);
        return nullptr;
    }

    void* result = nullptr;
    if ((flags & 1u) != 0) {
        nsl_aramStackTop = stackTop - 1;
        nsl_aramStack[nsl_aramStackTop] = alignedSize;
        auto* base = static_cast<unsigned char*>(nsl_aramBase);
        result = base + size - alignedSize;
    } else {
        ++nsl_aramStackBottom;
        nsl_aramStack[stackBottom] = alignedSize;
        result = static_cast<unsigned char*>(nsl_aramBase) + top;
    }
    nslAramGetFreeBlock(nullptr, nullptr);
    return result;
}
// ea: 0x004230A0
void          nslAramFree(void* buffer) {
    if (buffer == nullptr)
        return;
    auto* base = static_cast<unsigned char*>(nsl_aramBase);
    auto* address = static_cast<unsigned char*>(buffer);
    const uintptr_t offset = address - base;
    unsigned size = nsl_aramSize;
    if (offset < nsl_aramSize) {
        unsigned top = 0;
        int index = 0;
        if (nsl_aramStackBottom >= 0) {
            while (true) {
                const unsigned entry = nsl_aramStack[index];
                if (top == offset) {
                    if ((entry & 0x80000000u) == 0) {
                        nsl_aramStack[index] = entry | 0x80000000u;
                        nslAramGetFreeBlock(nullptr, nullptr);
                        return;
                    }
                    break;
                }
                top += entry & 0x7fffffffu;
                if (top > size) {
                    txAssertFailed(&ignoreAssert_8, "a <= nsl_aramSize", "nslAramFree",
                                   "c:/cod/code/tl/nsl2/src/nsl/nslAram.cpp", 138);
                    size = nsl_aramSize;
                }
                if (++index > nsl_aramStackBottom)
                    break;
            }
        }

        index = 63;
        unsigned bottom = size;
        if (nsl_aramStackTop <= 63) {
            while (true) {
                bottom -= nsl_aramStack[index] & 0x7fffffffu;
                if (bottom > size) {
                    txAssertFailed(&ignoreAssert_7, "a <= nsl_aramSize", "nslAramFree",
                                   "c:/cod/code/tl/nsl2/src/nsl/nslAram.cpp", 144);
                    size = nsl_aramSize;
                }
                if (bottom == offset)
                    break;
                if (--index < nsl_aramStackTop)
                    goto invalid_block;
            }
            const unsigned entry = nsl_aramStack[index];
            if ((entry & 0x80000000u) == 0) {
                nsl_aramStack[index] = entry | 0x80000000u;
                nslAramGetFreeBlock(nullptr, nullptr);
                return;
            }
        }
    }
invalid_block:
    txPrintf("NSL", 0, "Invalid or already freed block %p", buffer);
}
unsigned      nslAramGetUsed() { return 0; }
bool          nslAramIsInAram(void*) { return false; }
void          nslAramCompact() {}

// ============================================================================
// nslMemory — audio heap wrappers
// ============================================================================
void*         nslMemoryAlloc(unsigned size) { return tlMemAlloc(size, 0x40u, 0u); }
void          nslMemoryFree(void* ptr) { tlMemFree(ptr); }

// ============================================================================
// Misc
// ============================================================================
bool          nslGetState() { return true; }
const char*   nslGetStateText(int) { return "ready"; }
