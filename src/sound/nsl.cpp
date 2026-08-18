// ============================================================================
// NSL — NGL Sound Library (237 funcs, 17 objects)
// ea: 0x8E0000-0x8XXXXX (sound subsystem)
// Xbox DirectSound wrapper — stubbed for Win32/XAudio2.
// ============================================================================

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <intrin.h>

#include "core/tlFixedString.h"

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);
extern "C" void txAssertFailed(unsigned char* ignore, const char* message,
                                const char* function, const char* source, int line);
extern "C" void txPrintf(const char* channel, int level, const char* fmt, ...);
extern "C" unsigned long long txTime();
extern "C" int __cdecl __fpclass(float value);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern "C" char* txPathFix(const char* src, char* dir, int dirSize);
extern const char* const defaultFileName;

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
struct nslGroup;
struct nslParam;
struct nslWave {
    union {
        unsigned nameOffset;
        char* name;
        unsigned id;
    } name;
    unsigned char format;
    unsigned char flags;
    unsigned char speakerMap;
    unsigned char priority;
    unsigned dataSize;
    unsigned sampleCount;
    union {
        unsigned groupOffset;
        char* groupName;
        nslGroup* group;
    } group;
    union {
        unsigned paramOffset;
        nslParam* param;
    } params;
    union {
        unsigned miscOffset;
        unsigned char* misc;
    } misc;
    union {
        unsigned aramAddr;
        unsigned fileOffset;
    } storage;
    unsigned short sampleRate;
    unsigned short miscSize;
    union {
        void* data;
        unsigned file;
    } source;
};
static_assert(sizeof(nslWave) == 40, "IDA nslWave layout");
// IDA type_inspect: nslParam is an 8-byte map followed by a flexible float array.
struct nslParam {
    unsigned __int64 map;
    float values[0];
};
static_assert(sizeof(nslParam) == 8, "IDA nslParam layout");
struct nslParamUnpacked {
    unsigned __int64 map;
    float values[64];
};
struct nslWaveInfo {
    unsigned short sampleRate;
    unsigned short sampleRateOriginal;
    unsigned char waveFormat;
    unsigned char waveFormatFlags;
    unsigned char speakerMap;
    unsigned char soundFlags;
    union {
        const char* groupName;
        const nslGroup* group;
    };
    union {
        const char* levelName;
        void* level;
        char* encodingStamp;
    };
    nslParamUnpacked paramUnpacked;
};
static_assert(sizeof(nslParamUnpacked) == 264, "IDA nslParamUnpacked layout");
static_assert(sizeof(nslWaveInfo) == 280, "IDA nslWaveInfo layout");
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
// IDA type_inspect: nslSpeaker is two four-float vectors (32 bytes).
struct nslSpeaker {
    float local[4];
    float world[4];
};
static_assert(sizeof(nslSpeaker) == 32, "IDA nslSpeaker layout");

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
static unsigned nsl_timeDelta = 0;
static unsigned nsl_timePrev = 0;
static unsigned nsl_frame = 0;
static unsigned nsl_waveBankLoadOrder = 0;
nslSpeakerMode nsl_speakerMode = static_cast<nslSpeakerMode>(-2);
nslWaveBankSlot* nsl_waveBankSlots = nullptr;
static char nsl_waveSourceDirectory[512] = {};
static char nsl_waveObjectDirectory[512] = {};
static unsigned char nsl_waveBankLoaderBuffer[4096] = {};
static nslWaveBankLoader nsl_waveBankLoad = {};
static int dword_E4B690 = 16;
// Listener-frame globals and initial speaker table from IDA's release data.
nslSpeaker nsl_speakers[8] = {
    {{-1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{ 1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{ 0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{ 0.0f, 0.0f,-1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{ 0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{ 0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{ 0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{ 0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}}
};
float nsl_listenerMatrix[3][3] = {};
static float nsl_frontA[3] = {};
static float dword_10E10CC = 0.0f;
static float dword_10E10D0 = 0.0f;
static float nsl_frontB[3] = {};
static float dword_10E10D8 = 0.0f;
static float dword_10E10DC = 0.0f;
static float nsl_topB[3] = {};
static float dword_10E10E4 = 0.0f;
static float dword_10E10E8 = 0.0f;
static float nsl_frontC[3] = {};
static float dword_10E10F0 = 0.0f;
static float dword_10E10F4 = 0.0f;
static float nsl_topC[3] = {};
static float dword_10E10FC = 0.0f;
static float dword_10E1100 = 0.0f;
static float nsl_topA[3] = {};
static float dword_10E1108 = 0.0f;
static float dword_10E110C = 0.0f;
static float dword_10E11CC = 0.0f;
static float dword_10E11D0 = 0.0f;
static float v3a = 0.0f;
static float dword_10E11D8 = 0.0f;
static float dword_10E11DC = 0.0f;
static float v3b = 0.0f;
static float dword_10E11E4 = 0.0f;
static float dword_10E11E8 = 0.0f;
static float dword_E4B6CC = 0.0f;
static float dword_E4B6D0 = 0.0f;
static float dword_E4B6D8 = 0.0f;
static float dword_E4B6DC = 0.0f;
static float dword_E4B6E0 = 0.0f;
static float dword_E4B6E8 = 1.0f;
static float dword_E4B6EC = 0.0f;
static float dword_E4B6F0 = 0.0f;
static float dword_E4B6F8 = 0.0f;
static float dword_E4B6FC = 0.0f;
static float dword_E4B700 = 0.0f;
static unsigned char byte_E4B6A4 = 0;
float nsl_dampenLevel = 1.0f;
static unsigned char ignoreAssert = 0;
static unsigned char ignoreAssert_0 = 0;
static unsigned char ignoreAssert_1 = 0;
static unsigned char ignoreAssert_2 = 0;
static unsigned char ignoreAssert_3 = 0;
int nsl_random_play = 0;
nslWave* wave = nullptr;
const char* waveName = nullptr;
nslWaveID waveID = NSL_INVALID_WAVE;
int i = 0;
unsigned bankIndex = 0;
char prevbuf[300] = {};
nslSourceID sourceID = NSL_INVALID_SOURCE;
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
// IDA globals referenced by nslPriority.o (0x00F4240 and 0x010E1430).
static int dword_F4240 = 0;
static int s_LastVoiceCount = 0;

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

// ea: 0x0082CCB0
static txSlot nslSlotFirst(const txSlotPool* pool) {
    return pool->usedSlots.next->slot;
}

// ea: 0x0082CCD0
static txSlot nslSlotNext(const txSlotPool* pool, txSlot slot) {
    const unsigned result =
        slot & (static_cast<unsigned>(pool->mask) | 0x80000000u);
    if (result < static_cast<unsigned>(pool->count)) {
        txSlotEntry* entry = reinterpret_cast<txSlotEntry*>(
            reinterpret_cast<unsigned char*>(pool->slots) +
            pool->stride * result);
        if (slot == entry->slot)
            return entry->next->slot;
    }
    return TX_SLOT_INVALID;
}
extern "C" void txSlotFree(txSlotPool* pool, txSlot slot);
extern "C" txSlot txSlotNew(txSlotPool* pool);

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
nslGroup* nslGroupGet(const char* groupName);
int nslWaveBankFixup(nslWaveBank* waveBank);
int nslWaveBankSetAram(nslWaveBank* waveBank, void* waveBankAram);
int nslWaveBankSetFile(nslWaveBank* waveBank, nflFileID waveBankFile, unsigned waveBankFileOffset);
void nslWaveBankFree(nslWaveBankID waveBankID);
nslWaveID nslWaveLookup(const char* waveName);
int nslWaveIsStreaming(nslWaveID waveID);
int nslWaveIsLooping(nslWaveID waveID);
unsigned nslWaveGetLength(nslWaveID waveID);
float nslWaveGetParam(nslWaveID waveID, unsigned paramIndex, float defaultValue);
const char* nslWaveGetName(nslWaveID waveID);
const char* nslWaveGetGroupName(nslWaveID waveID);
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
nslVoice*      nslVoicePtr(int);
unsigned      nslVoiceCount();
int           nslVoiceAlloc(nslWaveID, nslSourceID, int);
void          nslVoiceFree(int);
void          nslDriverUpdate();
void          nslPriorityUpdate();
int           nslPriorityCanPlay(int priority);
int           nslSourceGetPriority(nslSource* source);
void          nslSetSourceEffect(nslSourceID sid, int effectOn);

// ea: 0x00820410
static bool tlIsPow2(unsigned x) {
    return x != 0 && ((x - 1u) & x) == 0;
}

// ea: 0x00820640
static void nslParam_UnpackRaw(float* dv, unsigned __int64 sm,
                               const float* sv) {
    unsigned low = static_cast<unsigned>(sm);
    unsigned high = static_cast<unsigned>(sm >> 32);
    unsigned highBit = 0;
    unsigned lowBit = 1;
    if (sm != 0) {
        do {
            if ((high & highBit) != 0 || (low & lowBit) != 0) {
                low &= ~lowBit;
                high &= ~highBit;
                *dv = *sv++;
            }
            highBit = (highBit << 1) | (lowBit >> 31);
            lowBit *= 2;
            ++dv;
        } while ((high | low) != 0);
    }
}
// ea: 0x00820800
nslEmitterID  nslNewEmitter(const float* position) {
    const txSlot slot = txSlotNew(&nsl_emitterPool);
    const int index = nslSlotIndex(&nsl_emitterPool, slot);
    if (index != -1) {
        unsigned char* emitterRaw = reinterpret_cast<unsigned char*>(nsl_emitters) +
            nslEmitterStride * static_cast<unsigned>(index);
        if (emitterRaw != nullptr) {
            std::memset(emitterRaw, 0, 0x110u);
            if (position != nullptr) {
                float* params = reinterpret_cast<float*>(emitterRaw + 0x10u);
                params[19] = position[0];
                params[20] = position[1];
                params[21] = position[2];
                *reinterpret_cast<unsigned*>(emitterRaw + 0x8u) |= 0x380000u;
            }
            return slot;
        }
    }
    txPrintf("NSL", 0,
             "Out of emitters! Please increase your startup nslInitParams.maxEmitters, currently it's set to %d\n",
             nsl_initParams.maxEmitters);
    return NSL_INVALID_EMITTER;
}
// ea: 0x008208A0
void          nslFreeEmitter(nslEmitterID eid) {
    txSlotFree(&nsl_emitterPool, static_cast<txSlot>(eid));
}
nslSourceID   nslNewSource(nslWaveID waveID, int mImportance) {
    nslWave* wave = nslWavePtr(waveID);
    if (wave == nullptr)
        return NSL_SOURCE_ID_INVALID;

    const unsigned char* info =
        *reinterpret_cast<const unsigned char* const*>(wave);
    const unsigned char waveFlags = info[5];
    const char* looping = (waveFlags & 2u) != 0 ? "LOOPING" : defaultFileName;
    const char* streaming = (waveFlags & 1u) != 0 ? "STREAMING" : defaultFileName;

    unsigned char speakerMap = info[6];
    unsigned char channelCount;
    if (speakerMap != 0) {
        const unsigned char folded = static_cast<unsigned char>(
            (((speakerMap & 0x55u) + ((speakerMap >> 1) & 0x55u)) & 0x33u) +
            (((((speakerMap & 0x55u) + ((speakerMap >> 1) & 0x55u)) >> 2) &
              0x33u)));
        channelCount = static_cast<unsigned char>((folded >> 4) + (folded & 0xFu));
    } else {
        channelCount = 1;
    }
    const char* channels = channelCount == 1 ? "MONO" : "STEREO";
    const char* dimensionality = (info[7] & 1u) != 0 ? "3D" : "2D";
    txPrintf("NSL", 6, "NewSource: %p: %s: %s %s %s %s\n\n",
             static_cast<unsigned>(waveID), "nonname", dimensionality,
             channels, streaming, looping);

    const txSlot slot = txSlotNew(&nsl_sourcePool);
    const int sourceIndex = nslSlotIndex(&nsl_sourcePool, slot);
    unsigned char* sourceRaw = sourceIndex == -1
        ? nullptr
        : reinterpret_cast<unsigned char*>(nsl_sources) +
              nslSourceStride * static_cast<unsigned>(sourceIndex);
    if (sourceIndex == -1 || sourceRaw == nullptr) {
        txPrintf(
            "NSL", 0,
            "Out of sources! Please increase your startup nslInitParams.maxSources, currently it's set to %d\n",
            nsl_initParams.maxSources);
        return NSL_SOURCE_ID_INVALID;
    }

    std::memset(sourceRaw, 0, 0x148u);
    *reinterpret_cast<nslWaveID*>(sourceRaw + 0x110u) = waveID;
    *reinterpret_cast<int*>(sourceRaw + 0x118u) = -1;
    sourceRaw[0x120u] = 1;
    *reinterpret_cast<nslEmitterID*>(sourceRaw + 0x114u) = NSL_INVALID_EMITTER;

    const unsigned short sampleRate =
        *reinterpret_cast<const unsigned short*>(info);
    unsigned long long length = 0;
    if (sampleRate != 0)
        length = (1000ull * wave->sampleCount) / sampleRate;
    *reinterpret_cast<unsigned*>(sourceRaw + 0x12Cu) =
        static_cast<unsigned>(length);

    const nslSourceID sourceID = static_cast<nslSourceID>(slot);
    if ((info[7] & 1u) != 0) {
        *reinterpret_cast<unsigned*>(sourceRaw + 0x8u) |= 0x16000000u;
        *reinterpret_cast<float*>(sourceRaw + 0x74u) = 10.0f;
        *reinterpret_cast<float*>(sourceRaw + 0x78u) = 30.0f;
        *reinterpret_cast<float*>(sourceRaw + 0x80u) = 1.0f;
        if (mImportance != 0)
            sourceRaw[0x122u] |= 0x40u;
    } else {
        const int voice = nslVoiceAlloc(waveID, sourceID, mImportance);
        *reinterpret_cast<int*>(sourceRaw + 0x118u) = voice;
        if (voice == -1 && mImportance == 0) {
            const int isStreaming = info[5] & 1u;
            const int isLooping = (info[5] & 2u) != 0;
            txPrintf(
                "NSL", 2,
                "Trying to allocate non-3D source failed, wave=%s, streaming=%d, looping=%d\n",
                nslWaveGetName(waveID), isStreaming, isLooping);
            txSlotFree(&nsl_sourcePool, slot);
            return NSL_SOURCE_ID_INVALID;
        }
    }

    const unsigned __int64 paramMap =
        *reinterpret_cast<const unsigned __int64*>(info + 16u);
    nslParam_UnpackRaw(reinterpret_cast<float*>(sourceRaw + 0x10u), paramMap,
                       reinterpret_cast<const float*>(info + 24u));
    *reinterpret_cast<unsigned*>(sourceRaw + 0x8u) |=
        static_cast<unsigned>(paramMap);
    *reinterpret_cast<unsigned*>(sourceRaw + 0xCu) |=
        static_cast<unsigned>(paramMap >> 32);

    float randomVolume =
        ((static_cast<float>(std::rand()) *
          *reinterpret_cast<float*>(sourceRaw + 0x20u) * 2.0f) *
             0.000030518509f) -
        *reinterpret_cast<float*>(sourceRaw + 0x20u);
    randomVolume = randomVolume < 0.0f
        ? 1.0f / (1.0f - randomVolume)
        : randomVolume + 1.0f;
    float randomPitch =
        ((static_cast<float>(std::rand()) *
          *reinterpret_cast<float*>(sourceRaw + 0x24u) * 2.0f) *
             0.000030518509f) -
        *reinterpret_cast<float*>(sourceRaw + 0x24u);
    randomPitch = randomPitch < 0.0f
        ? 1.0f / (1.0f - randomPitch)
        : randomPitch + 1.0f;
    if (randomVolume > 1.0f)
        randomVolume = 1.0f;

    const unsigned paramsUpdate =
        *reinterpret_cast<const unsigned*>(sourceRaw + 0x8u);
    const unsigned paramsUsedHigh =
        *reinterpret_cast<const unsigned*>(sourceRaw + 0xCu);
    *reinterpret_cast<float*>(sourceRaw + 0x24u) = randomPitch;
    *reinterpret_cast<float*>(sourceRaw + 0x14u) = 1.0f;
    *reinterpret_cast<float*>(sourceRaw + 0x10u) = 1.0f;
    *reinterpret_cast<float*>(sourceRaw + 0x20u) = randomVolume;
    *reinterpret_cast<float*>(sourceRaw + 0x7Cu) = 1.0f;
    *reinterpret_cast<unsigned*>(sourceRaw + 0x8u) = paramsUpdate | 0x08000003u;
    *reinterpret_cast<unsigned*>(sourceRaw + 0xCu) = paramsUsedHigh;
    nslSetSourceEffect(sourceID, 1);
    return sourceID;
}
nslSourceID   nslNewSource(const char* waveName, int mImportance) {
    return nslNewSource(nslWaveLookup(waveName), mImportance);
}
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

// ea: 0x00820F80
void          nslSetEmitterParam(nslEmitterID eid, unsigned index, float value) {
    const int emitterIndex = nslSlotIndex(&nsl_emitterPool, static_cast<txSlot>(eid));
    if (emitterIndex == -1)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(nsl_emitters) +
        nslEmitterStride * static_cast<unsigned>(emitterIndex);
    if (raw == nullptr || index >= 0x40u)
        return;
    const unsigned __int64 mask = UINT64_C(1) << index;
    *reinterpret_cast<unsigned __int64*>(raw + 0x8u) |= mask;
    *reinterpret_cast<float*>(raw + 0x10u + 4u * index) = value;
}

// ea: 0x00821050
float         nslGetEmitterParam(nslEmitterID eid, unsigned index,
                                 float defaultValue) {
    const int emitterIndex = nslSlotIndex(&nsl_emitterPool, static_cast<txSlot>(eid));
    if (emitterIndex == -1)
        return defaultValue;
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(nsl_emitters) +
        nslEmitterStride * static_cast<unsigned>(emitterIndex);
    if (raw == nullptr || index >= 0x40u)
        return defaultValue;
    return *reinterpret_cast<const float*>(raw + 0x10u + 4u * index);
}

// ea: 0x00820D70
const char*   nslGetSourceGroup(nslSourceID sid) {
    const int sourceIndex = nslSlotIndex(&nsl_sourcePool, static_cast<txSlot>(sid));
    nslWaveID waveID = NSL_WAVE_ID_INVALID;
    if (sourceIndex != -1) {
        const unsigned char* raw = reinterpret_cast<const unsigned char*>(nsl_sources) +
            nslSourceStride * static_cast<unsigned>(sourceIndex);
        if (raw != nullptr)
            waveID = *reinterpret_cast<const nslWaveID*>(raw + 0x110u);
    }
    return nslWaveGetGroupName(waveID);
}

// Voice enumeration (used by EffectEventSys::NumberOfVoicesUsed)
// ea: 0x008203F0
unsigned      nslGetNumVoices() { return nslVoiceCount(); }
// ea: 0x008203E0
nslVoice*     nslGetVoice(unsigned voiceIndex) {
    return nslVoicePtr(static_cast<int>(voiceIndex));
}
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
// ea: 0x008211C0
void          nslSetEmitterPosition(nslEmitterID sid, const float* position) {
    nslEmitter* emitter = nslEmitterPtr(sid);
    if (emitter == nullptr)
        return;
    if ((__fpclass(static_cast<double>(position[0])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 406,
               "!(_fpclass(position[0])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    if ((__fpclass(static_cast<double>(position[1])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 407,
               "!(_fpclass(position[1])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    if ((__fpclass(static_cast<double>(position[2])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 408,
               "!(_fpclass(position[2])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    emitter->params[19] = position[0];
    emitter->params[20] = position[1];
    emitter->paramsUpdate |= 0x00380000u;
    emitter->params[21] = position[2];
}
// ea: 0x00821320
void          nslGetEmitterPosition(nslEmitterID sid, float* position) {
    nslEmitter* emitter = nslEmitterPtr(sid);
    if (emitter == nullptr)
        return;
    position[0] = emitter->params[19];
    position[1] = emitter->params[20];
    position[2] = emitter->params[21];
}
// ea: 0x00821480
void          nslSetEmitterVelocity(nslEmitterID sid, const float* velocity) {
    nslEmitter* emitter = nslEmitterPtr(sid);
    if (emitter == nullptr)
        return;
    if ((__fpclass(static_cast<double>(velocity[0])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 438,
               "!(_fpclass(velocity[0])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    if ((__fpclass(static_cast<double>(velocity[1])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 439,
               "!(_fpclass(velocity[1])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    if ((__fpclass(static_cast<double>(velocity[2])) & 0x297) != 0
        && _tlAssert(
               "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 440,
               "!(_fpclass(velocity[2])&(_FPCLASS_SNAN|_FPCLASS_QNAN|_FPCLASS_NINF|_FPCLASS_PINF|_FPCLASS_ND|_FPCLASS_PD))",
               "invalid floating point number"))
        __debugbreak();
    emitter->params[22] = velocity[0];
    emitter->params[23] = velocity[1];
    emitter->paramsUpdate |= 0x01C00000u;
    emitter->params[24] = velocity[2];
}
// ea: 0x008215E0
void          nslGetEmitterVelocity(nslEmitterID sid, float* velocity) {
    nslEmitter* emitter = nslEmitterPtr(sid);
    if (emitter == nullptr)
        return;
    velocity[0] = emitter->params[22];
    velocity[1] = emitter->params[23];
    velocity[2] = emitter->params[24];
}
// ea: 0x00821830
nslEmitterID  nslGetSourceEmitter(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return NSL_INVALID_EMITTER;
    return *reinterpret_cast<const nslEmitterID*>(
        reinterpret_cast<const unsigned char*>(source) + 0x114u);
}
// ea: 0x00821630
void          nslSetSourceEmitter(nslSourceID sid, nslEmitterID eid) {
    nslEmitter* emitter = nslEmitterPtr(eid);
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    unsigned char* sourceRaw = reinterpret_cast<unsigned char*>(source);
    if (emitter == nullptr) {
        *reinterpret_cast<nslEmitterID*>(sourceRaw + 0x114u) = NSL_INVALID_EMITTER;
        return;
    }

    const unsigned __int64 mask = emitter->paramsUsed | emitter->paramsUpdate;
    *reinterpret_cast<nslEmitterID*>(sourceRaw + 0x114u) = eid;
    *reinterpret_cast<unsigned __int64*>(sourceRaw + 0x8u) |= mask;
    const unsigned char* emitterRaw = reinterpret_cast<const unsigned char*>(emitter);
    for (unsigned index = 0; index < 64u; ++index) {
        if ((mask & (1ull << index)) == 0)
            continue;
        *reinterpret_cast<unsigned*>(sourceRaw + 0x10u + 4u * index) =
            *reinterpret_cast<const unsigned*>(emitterRaw + 0x10u + 4u * index);
    }
}
// ea: 0x00821870
void          nslAddEmitterSource(nslEmitterID eid, nslSourceID sid) {
    nslSetSourceEmitter(sid, eid);
}
// ea: 0x00821900
void          nslDebugDraw() {}
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
// ea: 0x00820400
unsigned      nslGetMaxNumVoices() { return nslVoiceCount(); }  // ?nslGetMaxNumVoices@@YAIXZ (nslCompat.o)
// ea: 0x00820D30
const char*   nslGetSourceName(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return nullptr;
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(source);
    const nslWaveID waveID = *reinterpret_cast<const nslWaveID*>(raw + 0x110u);
    return nslWaveGetName(waveID);
}
// ea: 0x00820350
const char*   nslGetWaveName(nslWaveID waveID) { return nslWaveGetName(waveID); }       // ?nslGetWaveName@@YAPBDW4nslWaveID@@@Z (nslCompat.o)
// ea: 0x00827040
const char*   nslWaveGetName(nslWaveID waveID) {
    const unsigned encoded = static_cast<unsigned>(waveID) | 0xFFFFu;
    const unsigned aramBase = nsl_initParams.aramBase;
    if (aramBase == 0)
        return "?";
    const unsigned waveIndex = encoded & 0xFFFFu;
    const unsigned slotIndex = (encoded >> 16) % aramBase;
    nslWaveBankSlot* slot = &nsl_waveBankSlots[slotIndex];
    if (slot->waveBankID != encoded ||
        slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED)
        return "?";
    nslWaveBank* waveBank = slot->waveBank;
    if (waveIndex > waveBank->waveCount)
        return "?";
    if ((waveBank->waveBankFlags & 2u) != 0)
        return nullptr;
    return waveBank->names[waveIndex].name;
}
// ea: 0x00827140
const char*   nslWaveGetGroupName(nslWaveID waveID) {
    nslWave* wave = nslWavePtr(waveID);
    if (wave == nullptr)
        return nullptr;
    const unsigned char* metadata =
        *reinterpret_cast<const unsigned char* const*>(wave);
    if (metadata == nullptr)
        return nullptr;
    const unsigned char* group =
        *reinterpret_cast<const unsigned char* const*>(metadata + 8u);
    if (group == nullptr)
        return nullptr;
    return reinterpret_cast<const char*>(group + 0x108u);
}
// ea: 0x00820340
const char*   nslGetWaveGroup(nslWaveID waveID) { return nslWaveGetGroupName(waveID); }      // ?nslGetWaveGroup@@YAPBDW4nslWaveID@@@Z (nslCompat.o)
enum nslBankID : unsigned { NSL_BANK_ID_INVALID = (unsigned)-1 };
nslBankID      nslLoadBank(unsigned int flags, unsigned int file, unsigned int fileOffset) { return static_cast<nslBankID>(nslWaveBankLoad(static_cast<nflFileID>(file), fileOffset, flags)); }  // ?nslLoadBank@@YA?AW4nslBankID@@III@Z
// ea: 0x00820310
nslWaveID      nslGetWave(const char* name) { return nslWaveLookup(name); }                  // ?nslGetWave@@YA?AW4nslWaveID@@PBD@Z
// ea: 0x00820330
float          nslGetWaveParam(nslWaveID waveID, int paramIndex, float defaultValue) {
    return nslWaveGetParam(waveID, static_cast<unsigned>(paramIndex), defaultValue);
}  // ?nslGetWaveParam@@YAMW4nslWaveID@@HM@Z
// ea: 0x00820FF0
float         nslGetSourceParam(nslSourceID sid, int index, float defaultValue) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr || index < 0 || index >= 0x40)
        return defaultValue;
    return source->params[index];
}  // ?nslGetSourceParam@@YAMW4nslSourceID@@HM@Z (nslSource.o)
// ea: 0x00820320
int           nslGetWaveLength(nslWaveID waveID) { return static_cast<int>(nslWaveGetLength(waveID)); } // ?nslGetWaveLength@@YAHW4nslWaveID@@@Z
// ea: 0x00820360
int           nslIsWaveLooped(nslWaveID waveID) { return nslWaveIsLooping(waveID); }       // ?nslIsWaveLooped@@YAHW4nslWaveID@@@Z
// ea: 0x008203D0
int           nslIsWaveStreamed(nslWaveID waveID) { return nslWaveIsStreaming(waveID); }     // ?nslIsWaveStreamed@@YAHW4nslWaveID@@@Z (nslCompat.o)

// nslCompat.o / nslSource.o family (stubbed; manglings match binary)
int           nslGetBankState(nslBankID bankID) { return nslWaveBankGetState(static_cast<nslWaveBankID>(bankID)); }
void          nslFreeBank(nslBankID bankID) { nslWaveBankFree(static_cast<nslWaveBankID>(bankID)); }
// ea: 0x00820920
void          nslFreeSource(nslSourceID sid) {
    const int index = nslSlotIndex(&nsl_sourcePool, static_cast<txSlot>(sid));
    if (index == -1)
        return;
    nslSource* source = reinterpret_cast<nslSource*>(
        reinterpret_cast<unsigned char*>(nsl_sources) +
        nslSourceStride * static_cast<unsigned>(index));
    if (source == nullptr)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(source);
    const int voice = *reinterpret_cast<const int*>(raw + 0x118u);
    if (voice != -1)
        nslVoiceFree(voice);
    txSlotFree(&nsl_sourcePool, static_cast<txSlot>(sid));
}
// ea: 0x00820B50
void          nslStopSource(nslSourceID sid) { nslFreeSource(sid); }
// ea: 0x008208C0
void          nslFreeSourceVoice(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(source);
    const int voice = *reinterpret_cast<const int*>(raw + 0x118u);
    if (voice == -1)
        return;
    nslVoiceFree(voice);
    *reinterpret_cast<int*>(raw + 0x118u) = -1;
    *reinterpret_cast<int*>(raw + 0x124u) = 0;
}
// ea: 0x00820980
void          nslQueueSource(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    reinterpret_cast<unsigned char*>(source)[0x122] |= 1u;
}
// ea: 0x00820B60
void          nslSetSourceCallback(nslSourceID sid, unsigned flags,
                                   nslSourceCallback callback,
                                   void* userObject, void* userData) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(source);
    raw[0x122] |= static_cast<unsigned char>(flags) | 4u;
    *reinterpret_cast<nslSourceCallback*>(raw + 0x134u) = callback;
    *reinterpret_cast<void**>(raw + 0x138u) = userObject;
    *reinterpret_cast<void**>(raw + 0x13Cu) = userData;
}
// ea: 0x008226A0
void          nslDampenSource(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(source);
    int dampenCount = *reinterpret_cast<int*>(raw + 0x50u);
    if (dampenCount < 1)
        *reinterpret_cast<int*>(raw + 0x50u) = dampenCount + 1;
}
// ea: 0x008226E0
void          nslUndampenSource(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    *reinterpret_cast<int*>(reinterpret_cast<unsigned char*>(source) + 0x50u) = 0;
}
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

// ea: 0x00822520
void          nslUpdateEmitters() {
    txSlot slot = nslSlotFirst(&nsl_emitterPool);
    while (slot != TX_SLOT_INVALID) {
        const txSlot next = nslSlotNext(&nsl_emitterPool, slot);
        const int index = nslSlotIndex(&nsl_emitterPool, slot);
        nslEmitter* emitter = reinterpret_cast<nslEmitter*>(
            reinterpret_cast<unsigned char*>(nsl_emitters) +
            nslEmitterStride * static_cast<unsigned>(index));
        emitter->paramsUsed |= emitter->paramsUpdate;
        emitter->paramsUpdate = 0;
        slot = next;
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
// ea: 0x008270B0
unsigned int  nslWaveGetHash(nslWaveID waveID) {
    const unsigned encodedWaveID = static_cast<unsigned>(waveID) | 0xFFFFu;
    if (nsl_initParams.aramBase == 0)
        return 0;

    const unsigned slotIndex = (encodedWaveID >> 16) % nsl_initParams.aramBase;
    nslWaveBankSlot* slot = &nsl_waveBankSlots[slotIndex];
    if (slot->waveBankID != encodedWaveID ||
        slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED)
        return 0;

    nslWaveBank* waveBank = slot->waveBank;
    const unsigned waveIndex = static_cast<unsigned>(waveID) & 0xFFFFu;
    if (waveIndex > waveBank->waveCount)
        return 0;

    if ((waveBank->waveBankFlags & 2u) != 0)
        return waveBank->names[waveIndex].hash;

    const tlFixedString fixedString(waveBank->names[waveIndex].name);
    return fixedString.hash;
}
// nslGetWaveLength is implemented in the compatibility wrapper above.
// ea: 0x00820DD0
unsigned      nslGetSourceLength(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return 0;
    return *reinterpret_cast<const unsigned*>(
        reinterpret_cast<const unsigned char*>(source) + 0x12Cu);
}  // ?nslGetSourceLength@@YAIW4nslSourceID@@@Z
// nslIsWaveLooped is implemented in the compatibility wrapper above.
// ea: 0x00820BC0
void          nslPauseSource(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(source);
    const int oldPauseCount = *reinterpret_cast<int*>(raw + 0x11Cu);
    *reinterpret_cast<int*>(raw + 0x11Cu) = oldPauseCount + 1;
    if (oldPauseCount != 0)
        return;
    if (raw[0x120] == 5)
        raw[0x123] &= 0xFBu;
    else
        raw[0x123] |= 2u;
}
// ea: 0x00820C30
void          nslUnpauseSource(nslSourceID sid) {
    nslSource* source = nslSourcePtr(sid);
    if (source == nullptr)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(source);
    const nslWaveID waveID =
        *reinterpret_cast<const nslWaveID*>(raw + 0x110u);
    if (nslWavePtr(waveID) == nullptr)
        return;
    int pauseCount = *reinterpret_cast<int*>(raw + 0x11Cu);
    if (pauseCount == 0)
        return;
    --pauseCount;
    *reinterpret_cast<int*>(raw + 0x11Cu) = pauseCount;
    if (pauseCount != 0)
        return;
    if (raw[0x120] == 5)
        raw[0x123] |= 4u;
    else
        raw[0x123] &= 0xFDu;
}
// ea: 0x008209C0
void          nslPlaySource(nslSourceID sid) {
    const int index = nslSlotIndex(&nsl_sourcePool, static_cast<txSlot>(sid));
    if (index == -1)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(nsl_sources) +
        nslSourceStride * static_cast<unsigned>(index);
    if (raw == nullptr)
        return;
    if (nslPriorityCanPlay(nslSourceGetPriority(reinterpret_cast<nslSource*>(raw))) == 0)
        return;
    raw[0x122u] |= 3u;
    *reinterpret_cast<unsigned*>(raw + 0x128u) = 0;
}

using nslSourceForAllCallback = void (__cdecl *)(nslSourceID);

// ea: 0x00820AC0
unsigned      nslSource_ForAll(nslSourceForAllCallback callback) {
    unsigned result = 0;
    txSlot slot = nslSlotFirst(&nsl_sourcePool);
    if (slot == TX_SLOT_INVALID)
        return result;
    do {
        const txSlot current = slot;
        slot = nslSlotNext(&nsl_sourcePool, current);
        const int sourceIndex = nslSlotIndex(&nsl_sourcePool, current);
        unsigned char* raw = sourceIndex == -1
            ? nullptr
            : reinterpret_cast<unsigned char*>(nsl_sources) +
                  nslSourceStride * static_cast<unsigned>(sourceIndex);
        if (callback != nullptr)
            callback(static_cast<nslSourceID>(current));
        if (result > raw[0x120u])
            result = raw[0x120u];
    } while (slot != TX_SLOT_INVALID);
    return result;
}

// ea: 0x00820A20
unsigned      nslEmitter_ForAll(nslEmitterID eid,
                                nslSourceForAllCallback callback) {
    unsigned result = 0;
    txSlot slot = nslSlotFirst(&nsl_sourcePool);
    if (slot == TX_SLOT_INVALID)
        return result;
    do {
        const txSlot current = slot;
        slot = nslSlotNext(&nsl_sourcePool, current);
        const int sourceIndex = nslSlotIndex(&nsl_sourcePool, current);
        unsigned char* raw = sourceIndex == -1
            ? nullptr
            : reinterpret_cast<unsigned char*>(nsl_sources) +
                  nslSourceStride * static_cast<unsigned>(sourceIndex);
        if (*reinterpret_cast<const unsigned*>(raw + 0x114u) == eid) {
            if (callback != nullptr)
                callback(static_cast<nslSourceID>(current));
            if (result > raw[0x120u])
                result = raw[0x120u];
        }
    } while (slot != TX_SLOT_INVALID);
    return result;
}

// ea: 0x00822720
void          nslDampenGuardSource(nslSourceID sid) {
    const int sourceIndex = nslSlotIndex(&nsl_sourcePool, static_cast<txSlot>(sid));
    if (sourceIndex == -1)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(nsl_sources) +
        nslSourceStride * static_cast<unsigned>(sourceIndex);
    if (raw != nullptr && *reinterpret_cast<const int*>(raw + 0x50u) <= 0)
        *reinterpret_cast<int*>(raw + 0x50u) = -1;
}

// ea: 0x008225A0
void          nslPause() { nslSource_ForAll(nslPauseSource); }
// ea: 0x008225B0
void          nslStop() { nslSource_ForAll(nslStopSource); }
// ea: 0x008225C0
void          nslUnpause() { nslSource_ForAll(nslUnpauseSource); }
// ea: 0x008225D0
nslSourceState nslGetState() {
    return static_cast<nslSourceState>(nslSource_ForAll(nullptr));
}
// ea: 0x008225E0
void          nslQueueEmitter(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslQueueSource);
}
// ea: 0x00822600
void          nslPlayEmitter(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslPlaySource);
}
// ea: 0x00822620
void          nslPauseEmitter(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslPauseSource);
}
// ea: 0x00822640
void          nslStopEmitter(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslStopSource);
}
// ea: 0x00822660
void          nslUnpauseEmitter(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslUnpauseSource);
}
// ea: 0x00822680
nslSourceState nslGetEmitterState(nslEmitterID eid) {
    return static_cast<nslSourceState>(nslEmitter_ForAll(eid, nullptr));
}

// ea: 0x00822770
void          nslDampenGuardEmitter(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslDampenGuardSource);
}
// ea: 0x00822790
void          nslDampenEmitter(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslDampenSource);
}
// ea: 0x008227B0
void          nslUndampenEmitter(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslUndampenSource);
}
// ea: 0x008227D0
void          nslDampen(float dampenLevel) {
    nsl_dampenLevel = dampenLevel;
    nslSource_ForAll(nslDampenSource);
}
// ea: 0x008227F0
void          nslUndampen() { nslSource_ForAll(nslUndampenSource); }

// ea: 0x00822990
void          nslSetEmitterEffectOn(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslSetSourceEffectOn);
}
// ea: 0x008229B0
void          nslSetEmitterEffectOff(nslEmitterID eid) {
    nslEmitter_ForAll(eid, nslSetSourceEffectOff);
}
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
// ea: 0x0082A020
const char*   nslWaveGetSourceDirectory() { return nsl_waveSourceDirectory; }
// ea: 0x0082A030
const char*   nslWaveGetObjectDirectory() { return nsl_waveObjectDirectory; }
// ea: 0x0082A040
void          nslWaveSetSourceDirectory(const char* newWaveSourceDirectory) {
    std::strncpy(nsl_waveSourceDirectory, newWaveSourceDirectory, 0x1FFu);
    txPathFix(nsl_waveSourceDirectory, nsl_waveSourceDirectory, 512);
    const char* suffix = nsl_waveSourceDirectory[0] == 0 ? "./" : "/";
    std::strcat(nsl_waveSourceDirectory, suffix);
    txPathFix(nsl_waveSourceDirectory, nsl_waveSourceDirectory, 512);
}
// ea: 0x0082A0D0
void          nslWaveSetObjectDirectory(const char* newWaveObjectDirectory) {
    std::strncpy(nsl_waveObjectDirectory, newWaveObjectDirectory, 0x1FFu);
    txPathFix(nsl_waveObjectDirectory, nsl_waveObjectDirectory, 512);
    const char* suffix = nsl_waveObjectDirectory[0] == 0 ? "./" : "/";
    std::strcat(nsl_waveObjectDirectory, suffix);
    txPathFix(nsl_waveObjectDirectory, nsl_waveObjectDirectory, 512);
}
// ea: 0x0082A160
const char*   nslWaveGetSourceFilename(const char* sourceFilename,
                                       char* fullSourceFilename,
                                       int fullSourceFilenameSize) {
    _snprintf(fullSourceFilename, fullSourceFilenameSize, "%s/%s",
              nsl_waveSourceDirectory, sourceFilename);
    txPathFix(fullSourceFilename, fullSourceFilename, fullSourceFilenameSize);
    return fullSourceFilename;
}
// ea: 0x0082A1A0
const char*   nslWaveGetObjectFilename(const char* objectFilename,
                                       const char* platform,
                                       char* fullObjectFilename,
                                       int fullObjectFilenameSize) {
    static_cast<void>(platform);
    _snprintf(fullObjectFilename, fullObjectFilenameSize, "%s/%s",
              nsl_waveObjectDirectory, objectFilename);
    txPathFix(fullObjectFilename, fullObjectFilename, fullObjectFilenameSize);
    return fullObjectFilename;
}
// ea: 0x0082A1E0
nslWaveInfo*  nslWaveInfoAlloc() {
    return static_cast<nslWaveInfo*>(std::calloc(1u, 0x118u));
}
// ea: 0x0082A1F0
unsigned      nslWaveInfoSize(const nslWaveInfo* waveInfo) {
    if (waveInfo == nullptr)
        return 0;
    unsigned __int64 map = waveInfo->paramUnpacked.map;
    unsigned count = 0;
    while (map != 0) {
        count += static_cast<unsigned>(map & 1u);
        map >>= 1;
    }
    return 4u * count + 24u;
}
// ea: 0x0082A220
nslWave*     nslWaveAlloc() {
    nslWave* wave = static_cast<nslWave*>(std::calloc(1u, 0x10u));
    if (wave != nullptr)
        *reinterpret_cast<nslWaveInfo**>(wave) = nslWaveInfoAlloc();
    return wave;
}
// ea: 0x0082A250
void          nslWaveInfoFree(nslWaveInfo* waveInfo) {
    if (waveInfo == nullptr)
        return;
    std::free(const_cast<char*>(waveInfo->groupName));
    std::free(const_cast<char*>(waveInfo->levelName));
    std::free(waveInfo);
}
// ea: 0x0082A280
void          nslWaveFree(nslWave* wave) {
    if (wave == nullptr)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(wave);
    nslWaveInfo* waveInfo = *reinterpret_cast<nslWaveInfo**>(raw);
    if (waveInfo != nullptr) {
        std::free(const_cast<char*>(waveInfo->groupName));
        std::free(const_cast<char*>(waveInfo->levelName));
        std::free(waveInfo);
    }
    void* data = *reinterpret_cast<void**>(raw + 4u);
    if (data != nullptr) {
        const unsigned dataSize = *reinterpret_cast<const unsigned*>(raw + 8u);
        std::memset(data, 0xBF, dataSize);
        std::free(data);
    }
    std::memset(raw, 0xBF, 0x10u);
    std::free(wave);
}
// ea: 0x0082A300
void          nslWaveSetSoundInfo(nslWave* wave,
                                  const nslWaveInfo* waveInfo) {
    if (wave == nullptr || waveInfo == nullptr)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(wave);
    nslWaveInfo* current = *reinterpret_cast<nslWaveInfo**>(raw);
    std::free(const_cast<char*>(current->groupName));
    std::free(const_cast<char*>(current->levelName));
    current->soundFlags = waveInfo->soundFlags;
    current->groupName = waveInfo->groupName == nullptr
        ? nullptr : _strdup(waveInfo->groupName);
    current->levelName = waveInfo->levelName == nullptr
        ? nullptr : _strdup(waveInfo->levelName);
    std::memcpy(reinterpret_cast<unsigned char*>(current) + 0x10u,
                reinterpret_cast<const unsigned char*>(waveInfo) + 0x10u,
                0x108u);
}
// ea: 0x0082A380
const nslWaveInfo* nslWaveGetInfo(const nslWave* wave) {
    if (wave == nullptr)
        return nullptr;
    return *reinterpret_cast<const nslWaveInfo* const*>(wave);
}
// ea: 0x0082A3A0
nslWaveInfo*  nslWaveInfoCopy(const nslWaveInfo* sourceWaveInfo) {
    if (sourceWaveInfo == nullptr)
        return nullptr;
    nslWaveInfo* copy = static_cast<nslWaveInfo*>(std::calloc(1u, 0x118u));
    if (copy == nullptr)
        return nullptr;
    std::memcpy(copy, sourceWaveInfo, 0x118u);
    copy->groupName = nullptr;
    copy->levelName = nullptr;
    if (sourceWaveInfo->groupName != nullptr) {
        copy->groupName = _strdup(sourceWaveInfo->groupName);
        if (copy->groupName == nullptr)
            goto failure;
    }
    if (sourceWaveInfo->levelName != nullptr) {
        copy->levelName = _strdup(sourceWaveInfo->levelName);
        if (copy->levelName == nullptr)
            goto failure;
    }
    return copy;
failure:
    std::free(const_cast<char*>(copy->groupName));
    std::free(const_cast<char*>(copy->levelName));
    std::free(copy);
    return nullptr;
}
// ea: 0x0082A440
nslWave*     nslWaveCopy(const nslWave* src, unsigned __formal, int flags,
                         int __formal2, const char** const __formal3) {
    static_cast<void>(__formal2);
    static_cast<void>(__formal3);
    nslWave* copy = static_cast<nslWave*>(std::calloc(1u, 0x10u));
    if (copy == nullptr)
        return nullptr;
    unsigned char* copyRaw = reinterpret_cast<unsigned char*>(copy);
    *reinterpret_cast<nslWaveInfo**>(copyRaw) = nslWaveInfoAlloc();
    const unsigned char* sourceRaw = reinterpret_cast<const unsigned char*>(src);
    nslWaveInfo* infoCopy = nslWaveInfoCopy(
        *reinterpret_cast<const nslWaveInfo* const*>(sourceRaw));
    *reinterpret_cast<nslWaveInfo**>(copyRaw) = infoCopy;
    if (infoCopy == nullptr) {
        nslWaveFree(copy);
        return nullptr;
    }
    *reinterpret_cast<unsigned*>(copyRaw + 0x0Cu) =
        *reinterpret_cast<const unsigned*>(sourceRaw + 0x0Cu);
    const unsigned dataSize =
        *reinterpret_cast<const unsigned*>(sourceRaw + 8u);
    *reinterpret_cast<unsigned*>(copyRaw + 8u) = dataSize;
    if ((flags & 0x10000000) == 0) {
        void* data = std::calloc(1u, 0x10000u + dataSize);
        *reinterpret_cast<void**>(copyRaw + 4u) = data;
        if (data == nullptr) {
            nslWaveFree(copy);
            return nullptr;
        }
        std::memcpy(data, *reinterpret_cast<void* const*>(sourceRaw + 4u),
                    dataSize);
    }
    return copy;
}
// ea: 0x00826F70
nslWave*      nslWavePtr(nslWaveID waveID) {
    const unsigned encodedWaveID = static_cast<unsigned>(waveID) | 0xFFFFu;
    if (nsl_initParams.aramBase == 0)
        return nullptr;

    const unsigned slotIndex = (encodedWaveID >> 16) % nsl_initParams.aramBase;
    nslWaveBankSlot* slot = &nsl_waveBankSlots[slotIndex];
    if (slot->waveBankID != encodedWaveID ||
        slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED)
        return nullptr;

    nslWaveBank* waveBank = slot->waveBank;
    const unsigned waveIndex = static_cast<unsigned>(waveID) & 0xFFFFu;
    if (waveIndex > waveBank->waveCount)
        return nullptr;

    return reinterpret_cast<nslWave*>(
        reinterpret_cast<unsigned char*>(waveBank->waves) + 0x10u * waveIndex);
}
unsigned      nslWaveCount() { return 0; }
unsigned      nslWaveGetSize(nslWaveID) { return 0; }
unsigned      nslWaveGetFormat(nslWaveID) { return 0; }
unsigned      nslWaveGetSampleRate(nslWaveID) { return 0; }
unsigned      nslWaveGetChannels(nslWaveID) { return 0; }
nslWaveID     nslWaveGetFirst() { return NSL_INVALID_WAVE; }
nslWaveID     nslWaveGetNext(nslWaveID) { return NSL_INVALID_WAVE; }
// ea: 0x008271F0
unsigned      nslWaveGetLength(nslWaveID waveID) {
    nslWave* wave = nslWavePtr(waveID);
    if (wave == nullptr)
        return 0;
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(wave);
    const unsigned char* metadata =
        *reinterpret_cast<const unsigned char* const*>(raw);
    const unsigned sampleCount = *reinterpret_cast<const unsigned*>(raw + 0xCu);
    const unsigned sampleRate =
        *reinterpret_cast<const unsigned short*>(metadata);
    return (sampleCount * 1000u) / sampleRate;
}
// ea: 0x00827220
int           nslWaveIsStreaming(nslWaveID waveID) {
    nslWave* wave = nslWavePtr(waveID);
    if (wave == nullptr)
        return 0;
    const unsigned char* metadata =
        *reinterpret_cast<const unsigned char* const*>(wave);
    return metadata[5] & 1u;
}
// ea: 0x00827250
int           nslWaveIsLooping(nslWaveID waveID) {
    nslWave* wave = nslWavePtr(waveID);
    if (wave == nullptr)
        return 0;
    const unsigned char* metadata =
        *reinterpret_cast<const unsigned char* const*>(wave);
    return (metadata[5] >> 1) & 1u;
}
// ea: 0x00827280
int           nslWaveGetChannelCount(nslWaveID waveID) {
    nslWave* wave = nslWavePtr(waveID);
    if (wave == nullptr)
        return 0;

    const unsigned char* metadata =
        *reinterpret_cast<const unsigned char* const*>(wave);
    const unsigned char speakerMap = metadata[6];
    if (speakerMap == 0)
        return 1;

    const unsigned char pairBits = static_cast<unsigned char>(
        (speakerMap & 0x55u) + ((speakerMap >> 1) & 0x55u));
    const unsigned char nibbleBits = static_cast<unsigned char>(
        (pairBits & 0x33u) + ((pairBits >> 2) & 0x33u));
    return (nibbleBits >> 4) + (nibbleBits & 0x0Fu);
}
// ea: 0x008272D0
int           nslWaveIsImportant(nslWaveID waveID) {
    nslWave* wave = nslWavePtr(waveID);
    if (wave == nullptr)
        return 0;
    const unsigned char* metadata =
        *reinterpret_cast<const unsigned char* const*>(wave);
    return (metadata[7] >> 2) & 1u;
}
// ea: 0x00826D90
static int nslParam_Index_1(const nslParam* params, unsigned __int64 param) {
    const unsigned low = static_cast<unsigned>(param);
    const unsigned high = static_cast<unsigned>(param >> 32);
    const bool lowIsPow2 = tlIsPow2(low);
    const bool highIsPow2 = tlIsPow2(high);
    if (static_cast<unsigned>(lowIsPow2) + static_cast<unsigned>(highIsPow2) != 1u &&
        _tlAssert("c:/cod/code/tl/nsl2/include\\nsl/param.h", 199,
                  "tlIsPow2((unsigned)param)+tlIsPow2((unsigned)(param>>32))==1",
                  "Param must be power of two (e.g. only one bit set)")) {
        __debugbreak();
    }
    if ((param & params->map) != param)
        return -1;
    unsigned __int64 value = (param * 2u) - 1u;
    value &= params->map;
    int bitCount = 0;
    while (value != 0) {
        bitCount += static_cast<int>(value & 1u);
        value >>= 1;
    }
    return bitCount - 1;
}

// ea: 0x00820610
static float nslParam_Get(const nslParam* params, unsigned __int64 param,
                          float defaultValue) {
    if (params == nullptr)
        return defaultValue;
    const int index = nslParam_Index_1(params, param);
    return index == -1 ? defaultValue : params->values[index];
}
// ea: 0x00827170
float         nslWaveGetParam(nslWaveID waveID, unsigned paramIndex,
                              float defaultValue) {
    nslWave* wave = nslWavePtr(waveID);
    if (wave == nullptr || paramIndex >= 0x40u)
        return defaultValue;
    const unsigned char* metadata =
        *reinterpret_cast<const unsigned char* const*>(wave);
    const uintptr_t paramAddress = reinterpret_cast<uintptr_t>(metadata) + 0x10u;
    if (paramAddress == 0)
        return defaultValue;
    const nslParam* params = reinterpret_cast<const nslParam*>(paramAddress);
    const int index = nslParam_Index_1(params, UINT64_C(1) << paramIndex);
    if (index == -1)
        return defaultValue;
    return params->values[index];
}

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
    if (nsl_initParams.aramBase == 0)
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
    if (nsl_initParams.aramBase == 0)
        return nullptr;
    const unsigned index = (waveBankID >> 16) % nsl_initParams.aramBase;
    nslWaveBankSlot* slot = &nsl_waveBankSlots[index];
    if (slot->waveBankID != waveBankID ||
        slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED ||
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
    if (nsl_initParams.aramBase == 0)
        return NSL_INVALID_BANK;

    nslWaveBankSlot* slot = &nsl_waveBankSlots[
        (waveBankID >> 16) % nsl_initParams.aramBase];
    const unsigned waveIndex = static_cast<unsigned>(waveID) & 0xFFFFu;
    if (slot->waveBankID != waveBankID ||
        slot->state != NSL_WAVE_BANK_SLOT_STATE_LOADED ||
        waveIndex > slot->waveBank->waveCount)
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
// ea: 0x00829DD0
int           nslWaveBankFixup(nslWaveBank* waveBank) {
    if (waveBank == nullptr || (waveBank->waveBankFlags & 0xC0u) != 0)
        return 0;

    const uintptr_t base = reinterpret_cast<uintptr_t>(waveBank);
    waveBank->names = reinterpret_cast<nslWaveName*>(
        reinterpret_cast<uintptr_t>(waveBank->names) + base);
    waveBank->waves = reinterpret_cast<nslWave*>(
        reinterpret_cast<uintptr_t>(waveBank->waves) + base);
    waveBank->infos = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(waveBank->infos) + base);
    waveBank->text = reinterpret_cast<char*>(
        reinterpret_cast<uintptr_t>(waveBank->text) + base);
    waveBank->storage.backing.waveBankAram = nullptr;
    waveBank->storage.backing.waveBankFile = static_cast<nflFileID>(0);
    waveBank->storage.backing.waveBankFileOffset = 0;
    waveBank->storage.backing.reserved = 0;
    std::memset(waveBank->streamMD5, 0, sizeof(waveBank->streamMD5));

    if ((waveBank->waveBankFlags & 2u) == 0) {
        const unsigned textAddress =
            static_cast<unsigned>(reinterpret_cast<uintptr_t>(waveBank->text));
        for (unsigned index = 0; index < waveBank->waveCount; ++index)
            waveBank->names[index].hash += textAddress;
    }

    unsigned char* waves = reinterpret_cast<unsigned char*>(waveBank->waves);
    const uintptr_t infoAddress = reinterpret_cast<uintptr_t>(waveBank->infos);
    for (unsigned index = 0; index < waveBank->waveCount; ++index) {
        unsigned char* wave = waves + 16u * index;
        uintptr_t nameOffset = *reinterpret_cast<uintptr_t*>(wave);
        nameOffset += infoAddress;
        *reinterpret_cast<uintptr_t*>(wave) = nameOffset;
        unsigned char* metadata =
            *reinterpret_cast<unsigned char**>(wave);
        if ((metadata[5] & 1u) != 0)
            *reinterpret_cast<unsigned*>(wave + 4u) += waveBank->streamOffset;
    }

    unsigned char* infoBytes = reinterpret_cast<unsigned char*>(waveBank->infos);
    unsigned char* infoEnd = infoBytes + waveBank->infoSize;
    while (infoBytes < infoEnd) {
        nslWaveInfo* info = reinterpret_cast<nslWaveInfo*>(infoBytes);
        if (info->groupName != nullptr) {
            const uintptr_t groupOffset =
                reinterpret_cast<uintptr_t>(info->groupName);
            const char* groupName = waveBank->text + groupOffset;
            info->groupName = reinterpret_cast<const char*>(nslGroupGet(groupName));
        }
        if ((info->soundFlags & 1u) != 0 && info->speakerMap != 0)
            info->soundFlags &= 0xFEu;
        infoBytes += (nslWaveInfoSize(info) + 15u) & 0xFFFFFFF0u;
    }

    waveBank->waveBankFlags |= 0x40u;
    return 1;
}
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
nslGroup*     nslListenerGetGroup(unsigned listenerIndex);
int           nslGetNumberOfListeners();
// ea: 0x00820290
nslVoice*     nslVoiceGet(int voiceIndex) { return nslVoicePtr(voiceIndex); }
// ea: 0x008202A0
nslGroup*     nslGetGroup(const char* groupName) { return nslGroupGet(groupName); }
// ea: 0x008202B0
nslGroup*     nslGetListenerGroup() { return nslListenerGetGroup(0); }
// ea: 0x008202C0
nslGroup*     nslGetMasterGroup() { return nslMasterGetGroup(); }
void          nslCompatInit() {}
void          nslCompatUpdate() {}
unsigned      nslCompatGetVoiceCount() { return 0; }
bool          nslCompatIsVoicePlaying(int) { return false; }
void          nslCompatVoicePlay(int) {}
void          nslCompatVoiceStop(int) {}
void          nslCompatVoiceSetVolume(int, float) {}
void          nslCompatVoiceSetPitch(int, float) {}
void          nslCompatVoiceSetPan(int, float) {}

// ea: 0x00821910
void          nslUpdateSources() {
    unsigned sourceStateHistogram[6] = {};
    const nslGroup* masterGroup = nslGetMasterGroup();

    for (txSlot slot = nslSlotFirst(&nsl_sourcePool);
         slot != TX_SLOT_INVALID;) {
        const txSlot nextSlot = nslSlotNext(&nsl_sourcePool, slot);
        const int sourceIndex = nslSlotIndex(&nsl_sourcePool, slot);
        unsigned char* sourceRaw = sourceIndex == -1
            ? nullptr
            : reinterpret_cast<unsigned char*>(nsl_sources) +
                  nslSourceStride * static_cast<unsigned>(sourceIndex);
        ++sourceStateHistogram[sourceRaw[0x120u]];

        const nslWaveID waveID =
            *reinterpret_cast<const nslWaveID*>(sourceRaw + 0x110u);
        nslWave* wave = nslWavePtr(waveID);
        if (wave == nullptr) {
            if (sourceIndex != -1 && sourceRaw != nullptr) {
                const int voice = *reinterpret_cast<const int*>(sourceRaw + 0x118u);
                if (voice != -1)
                    nslVoiceFree(voice);
                txSlotFree(&nsl_sourcePool, slot);
            }
            slot = nextSlot;
            continue;
        }

        unsigned char* sourceState = sourceRaw + 0x120u;
        unsigned char* sourceFlags = sourceRaw + 0x122u;
        unsigned char* sourceUpdate = sourceRaw + 0x123u;
        unsigned __int64 sourceParamsUsed =
            *reinterpret_cast<unsigned __int64*>(sourceRaw + 0x0u);
        unsigned __int64 sourceParamsUpdate =
            *reinterpret_cast<unsigned __int64*>(sourceRaw + 0x8u);

        if (*sourceState == 1u) {
            if ((*sourceFlags & 1u) == 0u) {
                slot = nextSlot;
                continue;
            }
            *sourceState = 2u;
        }

        const unsigned emitterID =
            *reinterpret_cast<const unsigned*>(sourceRaw + 0x114u);
        ++*reinterpret_cast<unsigned*>(sourceRaw + 0x124u);
        if (nslSlotIndex(&nsl_emitterPool, static_cast<txSlot>(emitterID)) == -1) {
            if (emitterID != NSL_INVALID_EMITTER) {
                *reinterpret_cast<unsigned*>(sourceRaw + 0x114u) = NSL_INVALID_EMITTER;
                if ((*sourceFlags & 8u) != 0u) {
                    txPrintf("NSL", 3,
                             "Freeing %p (flagged to die with the emitter)\n",
                             slot);
                    nslFreeSource(static_cast<nslSourceID>(slot));
                    slot = nextSlot;
                    continue;
                }
            }
        } else {
            const int emitterIndex =
                nslSlotIndex(&nsl_emitterPool, static_cast<txSlot>(emitterID));
            unsigned char* emitterRaw = reinterpret_cast<unsigned char*>(nsl_emitters) +
                nslEmitterStride * static_cast<unsigned>(emitterIndex);
            const unsigned __int64 emitterUpdate =
                *reinterpret_cast<const unsigned __int64*>(emitterRaw + 0x8u);
            sourceParamsUpdate |= emitterUpdate;
            float* sourceParams = reinterpret_cast<float*>(sourceRaw + 0x10u);
            const float* emitterParams = reinterpret_cast<const float*>(emitterRaw + 0x10u);
            for (unsigned index = 0; index < 64u; ++index) {
                if ((emitterUpdate & (UINT64_C(1) << index)) != 0u)
                    sourceParams[index] = emitterParams[index];
            }
        }

        *reinterpret_cast<unsigned __int64*>(sourceRaw + 0x0u) =
            sourceParamsUsed | sourceParamsUpdate;
        *reinterpret_cast<unsigned __int64*>(sourceRaw + 0x8u) = 0;

        float* sourceParams = reinterpret_cast<float*>(sourceRaw + 0x10u);
        const float maxDistance = sourceParams[26] * sourceParams[26];
        float distance = 100000000.0f;
        for (int listenerIndex = 0;
             listenerIndex < nslGetNumberOfListeners(); ++listenerIndex) {
            nslGroup* listener = nslListenerGetGroup(
                static_cast<unsigned>(listenerIndex));
            const float dx = sourceParams[20] - listener->params[20];
            const float dy = sourceParams[19] - listener->params[19];
            const float dz = sourceParams[21] - listener->params[21];
            const float listenerDistance = dx * dx + dy * dy + dz * dz;
            if (distance > listenerDistance)
                distance = listenerDistance;
        }

        const unsigned char* waveInfo =
            *reinterpret_cast<const unsigned char* const*>(wave);
        const bool isLoopingStream = (waveInfo[7] & 1u) != 0u &&
                                     (waveInfo[5] & 2u) != 0u;
        float distanceForAllocation = distance;
        float maxDistanceForAllocation = maxDistance;
        if (isLoopingStream && distance >= maxDistance) {
            const int voice = *reinterpret_cast<const int*>(sourceRaw + 0x118u);
            if (voice != -1 && distance >= maxDistance * 1.1f) {
                txPrintf("NSL", 5,
                         "Suspending loopsnd %p: %s: dist=%f range=[%f, %f]\n",
                         slot, nslWaveGetName(waveID), distance,
                         sourceParams[29], maxDistance);
                *sourceFlags |= 0x20u;
                nslFreeSourceVoice(static_cast<nslSourceID>(slot));
                slot = nextSlot;
                continue;
            }
        }

        int voice = *reinterpret_cast<const int*>(sourceRaw + 0x118u);
        if (voice == -1) {
            if (isLoopingStream && distanceForAllocation >= maxDistanceForAllocation) {
                slot = nextSlot;
                continue;
            }
            voice = nslVoiceAlloc(waveID, static_cast<nslSourceID>(slot),
                                  *sourceFlags & 0x40u);
            *reinterpret_cast<int*>(sourceRaw + 0x118u) = voice;
            if (voice == -1) {
                unsigned& allocationFailures =
                    *reinterpret_cast<unsigned*>(sourceRaw + 0x124u);
                ++allocationFailures;
                if (allocationFailures <= 10u || (waveInfo[5] & 2u) != 0u) {
                    slot = nextSlot;
                    continue;
                }
                const unsigned char* group =
                    *reinterpret_cast<const unsigned char* const*>(waveInfo + 8u);
                txPrintf("NSL", 5,
                         "%s: Forcing source to be freed: %p %s:%s \n",
                         "nslUpdateSources", slot,
                         group == nullptr ? "" : reinterpret_cast<const char*>(group + 264u),
                         nslWaveGetName(waveID));
                const int freeIndex = nslSlotIndex(&nsl_sourcePool, slot);
                if (freeIndex != -1) {
                    unsigned char* freeRaw = reinterpret_cast<unsigned char*>(nsl_sources) +
                        nslSourceStride * static_cast<unsigned>(freeIndex);
                    const int freeVoice =
                        *reinterpret_cast<const int*>(freeRaw + 0x118u);
                    if (freeVoice != -1)
                        nslVoiceFree(freeVoice);
                    txSlotFree(&nsl_sourcePool, slot);
                }
                slot = nextSlot;
                continue;
            }
            if ((*sourceFlags & 0x20u) != 0u) {
                *sourceFlags &= static_cast<unsigned char>(~0x20u);
                *sourceState = 1u;
                txPrintf("NSL", 5,
                         "Restoring loopsnd %p: %s: dist=%f range=[%f, %f]\n",
                         slot, nslWaveGetName(waveID), distance,
                         sourceParams[29], maxDistance);
            }
            sourceParamsUpdate |= *reinterpret_cast<unsigned __int64*>(sourceRaw + 0x0u);
            *reinterpret_cast<unsigned*>(sourceRaw + 0x124u) = 0;
        }

        if (static_cast<unsigned>(voice) >= nsl_initParams.aramSize) {
            txAssertFailed(&ignoreAssert_3,
                           "(unsigned)s->voice<(unsigned)nsl_initParams.maxVoices",
                           "nslUpdateSources",
                           "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 882);
        }
        unsigned char* voiceRaw = reinterpret_cast<unsigned char*>(nsl_voices) +
            320u * static_cast<unsigned>(voice);
        if (voiceRaw[0x108u] != 6u) {
            if ((*sourceUpdate & 2u) != 0u) {
                if (*sourceState == 5u)
                    txAssertFailed(&ignoreAssert_2, "s->state != NSL_SOURCE_STATE_PAUSED",
                                   "nslUpdateSources",
                                   "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 899);
                if ((*sourceUpdate & 4u) != 0u)
                    txAssertFailed(&ignoreAssert_1,
                                   "!(s->update&NSL_SOURCE_UPDATE_UNPAUSE)",
                                   "nslUpdateSources",
                                   "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 900);
                const unsigned char previousState = *sourceState;
                *sourceUpdate &= static_cast<unsigned char>(~2u);
                if (previousState == 4u)
                    voiceRaw[0x109u] |= 2u;
                *reinterpret_cast<unsigned char*>(sourceRaw + 0x121u) = previousState;
                *sourceState = 5u;
            }
            if ((*sourceUpdate & 4u) != 0u) {
                if (*sourceState != 5u)
                    txAssertFailed(&ignoreAssert_0, "s->state == NSL_SOURCE_STATE_PAUSED",
                                   "nslUpdateSources",
                                   "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 910);
                if ((*sourceUpdate & 2u) != 0u)
                    txAssertFailed(&ignoreAssert, "!(s->update&NSL_SOURCE_UPDATE_PAUSE)",
                                   "nslUpdateSources",
                                   "c:/cod/code/tl/nsl2/src/nsl/nslSource.cpp", 911);
                *sourceUpdate &= static_cast<unsigned char>(~4u);
                const unsigned char previousState =
                    *reinterpret_cast<const unsigned char*>(sourceRaw + 0x121u);
                *sourceState = previousState;
                if (previousState == 4u)
                    voiceRaw[0x109u] |= 4u;
            }
            if (*sourceState == 2u && voiceRaw[0x108u] == 3u)
                *sourceState = 3u;
            if (*sourceState == 3u && (*sourceFlags & 2u) != 0u) {
                *sourceState = 4u;
                voiceRaw[0x109u] |= 1u;
            }
            if ((*sourceUpdate & 8u) != 0u) {
                *sourceUpdate &= static_cast<unsigned char>(~8u);
                voiceRaw[0x109u] |= (*sourceFlags & 0x10u) != 0u ? 0x20u : 0x40u;
            }

            float* voiceParams = reinterpret_cast<float*>(voiceRaw + 8u);
            const float volumeBase = sourceParams[2];
            const float pitchBase = sourceParams[3];
            float volume = sourceParams[8] *
                (nslParam_Get(reinterpret_cast<const nslParam*>(waveInfo + 16u),
                              1u, 1.0f) * volumeBase);
            float pitch = sourceParams[9] *
                (nslParam_Get(reinterpret_cast<const nslParam*>(waveInfo + 16u),
                              2u, 1.0f) * pitchBase);
            float dopplerFactor = sourceParams[31] *
                nslParam_Get(reinterpret_cast<const nslParam*>(waveInfo + 16u),
                             UINT64_C(0x08000000), 1.0f);
            const nslGroup* waveGroup =
                *reinterpret_cast<const nslGroup* const*>(waveInfo + 8u);
            if (waveGroup != nullptr) {
                volume *= waveGroup->params[0];
                pitch *= waveGroup->params[1];
            }

            bool busFound = false;
            const unsigned busId = *reinterpret_cast<const unsigned*>(waveInfo + 12u);
            for (int index = 0; index < nslGetBusIdPitchCount(); ++index) {
                if (nslGetBusIdPitch(index) == busId)
                    busFound = true;
            }
            if (nslGetBusIdPitchCount() != 0 && !busFound)
                pitch *= nslGetBusPitch();
            busFound = false;
            for (int index = 0; index < nslGetBusIdVolumeCount(); ++index) {
                if (nslGetBusIdVolume(index) == busId)
                    busFound = true;
            }
            if (nslGetBusIdVolumeCount() != 0 && !busFound)
                volume *= nslGetBusVolume();

            volume *= masterGroup->params[0];
            pitch *= masterGroup->params[1];
            if (*reinterpret_cast<const int*>(sourceRaw + 0x50u) > 0)
                volume *= nsl_dampenLevel;
            if (volume < 0.0f)
                volume = 0.0f;
            else if (volume > 1.0f)
                volume = 1.0f;
            if (pitch < 0.0f)
                pitch = 0.0f;
            else if (pitch > 2.0f)
                pitch = 2.0f;

            voiceParams[0] = volume;
            voiceParams[1] = pitch;
            voiceParams[29] = dopplerFactor;
            if (voice != -1) {
                unsigned char* allocatedVoice = reinterpret_cast<unsigned char*>(nsl_voices) +
                    320u * static_cast<unsigned>(voice);
                if (allocatedVoice != nullptr) {
                    reinterpret_cast<float*>(allocatedVoice)[78] = volume;
                    reinterpret_cast<float*>(allocatedVoice)[79] = pitch;
                }
            }
            *reinterpret_cast<unsigned __int64*>(voiceRaw + 0x0u) |=
                sourceParamsUpdate | UINT64_C(0x8000003);
        } else if ((waveInfo[5] & 2u) != 0u) {
            const int staleVoice = *reinterpret_cast<const int*>(sourceRaw + 0x118u);
            if (staleVoice != -1)
                nslVoiceFree(staleVoice);
            *reinterpret_cast<int*>(sourceRaw + 0x118u) = -1;
            *reinterpret_cast<unsigned*>(sourceRaw + 0x124u) = 0;
            *sourceState = 1u;
        } else {
            const int freeIndex = nslSlotIndex(&nsl_sourcePool, slot);
            if (freeIndex != -1) {
                unsigned char* freeRaw = reinterpret_cast<unsigned char*>(nsl_sources) +
                    nslSourceStride * static_cast<unsigned>(freeIndex);
                const int freeVoice = *reinterpret_cast<const int*>(freeRaw + 0x118u);
                if (freeVoice != -1)
                    nslVoiceFree(freeVoice);
                txSlotFree(&nsl_sourcePool, slot);
            }
        }
        slot = nextSlot;
    }
}

// ============================================================================
// nslListener — audio listener (3D ears)
// ============================================================================
static int    s_NumberOfListeners = 1;
static float* txVectorNormalize(float* dst, const float* src);

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
void          nslListenerSetMatrix(unsigned listenerIndex, const float (*m)[4]) {
    nslGroup* group = nslListenerGetGroup(listenerIndex);
    if (group == nullptr)
        return;
    group->params[19] = (*m)[3];
    group->params[20] = (*m)[7];
    group->params[21] = (*m)[11];
    group->params[49] = (*m)[4];
    group->params[50] = (*m)[5];
    group->params[52] = (*m)[6];
    group->params[46] = (*m)[8];
    group->params[47] = (*m)[9];
    group->params[49] = (*m)[10];
    txVectorNormalize(group->params + 46, group->params + 46);
    txVectorNormalize(group->params + 49, group->params + 49);
    group->paramsUpdate |= UINT64_C(0x380000) |
                           (UINT64_C(0xFC000) << 32);
}

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
void          nslListenerSetOrientation(unsigned listenerIndex, const float* frt,
                                         const float* top) {
    nslGroup* group = nslListenerGetGroup(listenerIndex);
    if (group == nullptr)
        return;
    group->params[49] = top[0];
    group->params[50] = top[1];
    group->params[51] = top[2];
    group->params[46] = frt[0];
    group->params[47] = frt[1];
    group->params[48] = frt[2];
    txVectorNormalize(group->params + 46, group->params + 46);
    txVectorNormalize(group->params + 49, group->params + 49);
    group->paramsUpdate |= UINT64_C(0xFC000) << 32;

    nsl_frontA[0] = group->params[46];
    dword_10E10CC = group->params[47];
    dword_10E10D0 = group->params[48];
    nsl_frontB[0] = frt[0];
    dword_10E10D8 = frt[1];
    dword_10E10DC = frt[2];
    nsl_frontC[0] = nsl_frontA[0] - nsl_frontB[0];
    dword_10E10F0 = dword_10E10CC - dword_10E10D8;
    dword_10E10F4 = dword_10E10D0 - dword_10E10DC;
    nsl_topA[0] = group->params[49];
    dword_10E1108 = group->params[50];
    dword_10E110C = group->params[51];
    nsl_topB[0] = top[0];
    dword_10E10E4 = top[1];
    dword_10E10E8 = top[2];
    nsl_topC[0] = nsl_topA[0] - nsl_topB[0];
    dword_10E10FC = dword_10E1108 - dword_10E10E4;
    dword_10E1100 = dword_10E110C - dword_10E10E8;
}
void          nslListenerSetDopplerFactor(unsigned, float) {}

// ea: 0x008232D0
static float* txVectorNormalize(float* dst, const float* src) {
    const long double length = std::sqrt(static_cast<long double>(
        src[0] * src[0] + src[1] * src[1] + src[2] * src[2]));
    if (length > 0.00000011920929L) {
        const float inverse = static_cast<float>(1.0L / length);
        dst[0] = src[0] * inverse;
        dst[1] = src[1] * inverse;
        dst[2] = src[2] * inverse;
    } else {
        dst[0] = 0.0f;
        dst[1] = 0.0f;
        dst[2] = 0.0f;
    }
    return dst;
}

// ea: 0x00823400
void          nslUpdateListener() {
    nslGroup* listenerGroup = nslGetListenerGroup();
    float* params = listenerGroup->params;

    nsl_listenerMatrix[0][0] =
        (params[50] * params[48]) - (params[51] * params[47]);
    dword_10E11CC = (params[51] * params[46]) - (params[48] * params[49]);
    dword_10E11D0 = (params[49] * params[47]) - (params[50] * params[46]);
    txVectorNormalize(nsl_listenerMatrix[0], nsl_listenerMatrix[0]);

    v3a = (dword_10E11D0 * params[47]) - (dword_10E11CC * params[48]);
    dword_10E11D8 = (nsl_listenerMatrix[0][0] * params[48])
                  - (params[46] * dword_10E11D0);
    dword_10E11DC = (params[46] * dword_10E11CC)
                  - (nsl_listenerMatrix[0][0] * params[47]);
    txVectorNormalize(&v3a, &v3a);

    v3b = (dword_10E11DC * dword_10E11CC)
        - (dword_10E11D8 * dword_10E11D0);
    dword_10E11E4 = (v3a * dword_10E11D0)
                  - (dword_10E11DC * nsl_listenerMatrix[0][0]);
    dword_10E11E8 = (dword_10E11D8 * nsl_listenerMatrix[0][0])
                  - (v3a * dword_10E11CC);
    txVectorNormalize(&v3b, &v3b);

    dword_E4B6D8 = (v3b * dword_E4B6D0) + (v3a * dword_E4B6CC)
                 + (nsl_listenerMatrix[0][0] * nsl_speakers[0].local[0]);
    dword_E4B6DC = (dword_10E11E4 * dword_E4B6D0)
                 + (dword_10E11D8 * dword_E4B6CC)
                 + (dword_10E11CC * nsl_speakers[0].local[0]);
    dword_E4B6E0 = (dword_10E11E8 * dword_E4B6D0)
                 + (dword_10E11DC * dword_E4B6CC)
                 + (dword_10E11D0 * nsl_speakers[0].local[0]);
    dword_E4B6F8 = (v3b * dword_E4B6F0) + (v3a * dword_E4B6EC)
                 + (nsl_listenerMatrix[0][0] * dword_E4B6E8);
    dword_E4B6FC = (dword_10E11E4 * dword_E4B6F0)
                 + (dword_10E11D8 * dword_E4B6EC)
                 + (dword_10E11CC * dword_E4B6E8);
    dword_E4B700 = (dword_10E11E8 * dword_E4B6F0)
                 + (dword_10E11DC * dword_E4B6EC)
                 + (dword_10E11D0 * dword_E4B6E8);
}
// ea: 0x00823700
void          nslUpdate() {
    if (nsl_frame != 0)
        nsl_timeDelta = nsl_time - nsl_timePrev;
    else
        nsl_timeDelta = 0;
    nsl_timePrev = nsl_time;
    nsl_time = static_cast<unsigned>(txTime());
    ++nsl_frame;

    nslUpdateBanks();
    nslPriorityUpdate();
    nslUpdateSources();
    nslUpdateEmitters();
    nslUpdateListener();

    const unsigned aramSize = nsl_initParams.aramSize;
    for (unsigned voiceIndex = 0; voiceIndex < aramSize; ++voiceIndex) {
        unsigned char* voiceRaw = reinterpret_cast<unsigned char*>(nsl_voices) +
            320u * voiceIndex;
        if (voiceRaw[0x108u] != 0u &&
            nslWavePtr(*reinterpret_cast<const nslWaveID*>(voiceRaw + 0x110u)) == nullptr) {
            nslFreeSource(*reinterpret_cast<const nslSourceID*>(voiceRaw + 0x114u));
            nslVoiceFree(static_cast<int>(voiceIndex));
        }
    }

    if ((byte_E4B6A4 & 1u) == 0u)
        nslDriverUpdate();

    char voiceStateBuffer[300] = {};
    static const char voiceStateChars[] = "_IQqPpFf";
    unsigned stateCount = 0;
    while (stateCount < aramSize) {
        const unsigned char* voiceRaw = reinterpret_cast<const unsigned char*>(nsl_voices) +
            320u * stateCount;
        voiceStateBuffer[stateCount] = voiceStateChars[voiceRaw[0x108u]];
        ++stateCount;
    }
    voiceStateBuffer[aramSize] = 0;
    if (std::strcmp(prevbuf, voiceStateBuffer) != 0)
        txPrintf("NSL", 9, "%s\n", voiceStateBuffer);
    std::memcpy(prevbuf, voiceStateBuffer, stateCount + 1u);

    if (nsl_random_play != 0 && nsl_initParams.aramBase != 0) {
        bankIndex = bankIndex % nsl_initParams.aramBase + 1u;
        const nslWaveBankID waveBankID = nsl_waveBankSlots[bankIndex].waveBankID;
        nslWaveBank* waveBank = nslWaveBankPtr(waveBankID);
        if (waveBank != nullptr && waveBank->waveCount != 0 &&
            nslGetSourceState(sourceID) == NSL_SOURCE_STATE_INVALID) {
            i = i % static_cast<int>(waveBank->waveCount) + 1;
            waveID = nslWaveBankGetWave(waveBankID, static_cast<unsigned>(i));
            waveName = nslWaveGetName(waveID);
            wave = nslWavePtr(waveID);
            if (wave != nullptr) {
                const unsigned char* waveInfo =
                    *reinterpret_cast<const unsigned char* const*>(wave);
                if ((waveInfo[5] & 2u) == 0u) {
                    sourceID = nslNewSource(waveID, 1);
                    nslPlaySource(sourceID);
                }
            }
        }
    }
}

// ============================================================================
// nslPriority — voice priority / attenuation
// ============================================================================
// ea: 0x00828B30
static int nslParam_Index_2(const nslParam* params, unsigned __int64 param) {
    return nslParam_Index_1(params, param);
}

// ea: 0x00828C30
static float nslDistanceAttenuationValue(const nslWave* wave, float dist) {
    const uintptr_t metadata = reinterpret_cast<uintptr_t>(
        *reinterpret_cast<const unsigned char* const*>(wave));
    const uintptr_t paramAddress = metadata + 0x10u;
    const nslParam* params = reinterpret_cast<const nslParam*>(paramAddress);

    float minDistance = 1.0f;
    if (paramAddress != 0) {
        const int index = nslParam_Index_2(params, UINT64_C(0x02000000));
        if (index != -1) {
            minDistance = params->values[index];
            if (minDistance <= 0.0f)
                minDistance = 1.0f;
        }
    }

    float maxDistance = 1.0f;
    if (paramAddress != 0) {
        const int index = nslParam_Index_2(params, UINT64_C(0x04000000));
        if (index != -1)
            maxDistance = params->values[index];
    }

    if (dist >= maxDistance)
        return 0.0f;
    if (minDistance >= dist)
        return 1.0f;
    if (maxDistance <= minDistance)
        return dist;

    const float inverse = 1.0f /
        ((minDistance * minDistance) -
         ((maxDistance * minDistance) * 2.0f) +
         (maxDistance * maxDistance));
    float result = ((((inverse * maxDistance) * -2.0f) +
                     (inverse * dist)) * dist) +
                   ((inverse * maxDistance) * maxDistance);
    if (result > 1.0f)
        return 1.0f;
    if (result < 0.0f)
        return 0.0f;
    return result;
}

// ea: 0x00828D50
float         nslVoiceGetAttenuation(nslVoice* voice) {
    nslGroup* listenerGroup = nslGetListenerGroup();
    const unsigned char* voiceRaw = reinterpret_cast<const unsigned char*>(voice);
    const float* voiceParams = reinterpret_cast<const float*>(voiceRaw + 8u);
    const float* listenerPosition = listenerGroup->params + 19;
    const float dx = voiceParams[19] - listenerPosition[0];
    const float dy = voiceParams[20] - listenerPosition[1];
    const float dz = voiceParams[21] - listenerPosition[2];
    const float distance = sqrtf(dx * dx + dy * dy + dz * dz);
    const nslWaveID waveID =
        *reinterpret_cast<const nslWaveID*>(voiceRaw + 0x110u);
    nslWave* wave = nslWavePtr(waveID);
    if (wave != nullptr)
        return nslDistanceAttenuationValue(wave, distance);
    return 1.0f;
}

// ea: 0x00828DD0
float         nslSourceGetAttenuation(nslSource* source) {
    nslGroup* listenerGroup = nslGetListenerGroup();
    const unsigned char* sourceRaw = reinterpret_cast<const unsigned char*>(source);
    const float* sourceParams = reinterpret_cast<const float*>(sourceRaw + 0x10u);
    const float* listenerPosition = listenerGroup->params + 19;
    const float dx = sourceParams[19] - listenerPosition[0];
    const float dy = sourceParams[20] - listenerPosition[1];
    const float dz = sourceParams[21] - listenerPosition[2];
    const float distance = sqrtf(dx * dx + dy * dy + dz * dz);
    const nslWaveID waveID =
        *reinterpret_cast<const nslWaveID*>(sourceRaw + 0x110u);
    return nslDistanceAttenuationValue(nslWavePtr(waveID), distance);
}

// ea: 0x00828E70
int           nslSourceIs3D(nslSource* source) {
    if (source == nullptr)
        return 0;
    const unsigned char* sourceRaw = reinterpret_cast<const unsigned char*>(source);
    const nslWaveID waveID =
        *reinterpret_cast<const nslWaveID*>(sourceRaw + 0x110u);
    nslWave* wave = nslWavePtr(waveID);
    if (wave != nullptr) {
        const unsigned char* metadata =
            *reinterpret_cast<const unsigned char* const*>(wave);
        if (metadata != nullptr)
            return metadata[7] & 1u;
    }
    return 1;
}

// ea: 0x00828EF0
int           nslPriorityCanPlay(int priority) {
    static_cast<void>(nslVoicePtr(0));
    if (priority <= 0)
        return 0;
    if (s_LastVoiceCount >= 59)
        return priority > 50;
    return 1;
}

// ea: 0x00828F30
int           nslSourceGetPriority(nslSource* source) {
    if (source == nullptr)
        return 100;

    const unsigned char* sourceRaw = reinterpret_cast<const unsigned char*>(source);
    const nslWaveID waveID =
        *reinterpret_cast<const nslWaveID*>(sourceRaw + 0x110u);
    nslWave* wave = nslWavePtr(waveID);
    if (wave != nullptr) {
        const unsigned char* metadata =
            *reinterpret_cast<const unsigned char* const*>(wave);
        if (metadata != nullptr && (metadata[7] & 1u) == 0)
            return 100;
    }

    const float attenuation = nslSourceGetAttenuation(source);
    const float maxAttenuation = source->params[10];
    float clampedAttenuation = attenuation;
    const float minAttenuation = source->params[9];
    if (clampedAttenuation > maxAttenuation)
        clampedAttenuation = maxAttenuation;
    if (minAttenuation > clampedAttenuation)
        clampedAttenuation = minAttenuation;
    return static_cast<int>(
        (((clampedAttenuation - minAttenuation) /
          (maxAttenuation - minAttenuation)) *
         (source->params[8] - source->params[7])) + source->params[7]);
}

// ea: 0x00828FC0
int           nslVoiceGetPriority(nslVoice& voice) {
    const unsigned char* voiceRaw = reinterpret_cast<const unsigned char*>(&voice);
    const nslSourceID sourceID =
        *reinterpret_cast<const nslSourceID*>(voiceRaw + 0x114u);
    return nslSourceGetPriority(nslSourcePtr(sourceID));
}

// ea: 0x00828FE0
void          nslPriorityUpdate() {
    struct PriorityEntry {
        int priority;
        int voiceIndex;
        int sourcePriority;
    };

    nslVoice* firstVoice = nslVoicePtr(0);
    PriorityEntry entries[5] = {
        {101, 0, 0}, {101, 0, 0}, {101, 0, 0},
        {101, 0, 0}, {101, 0, 0}
    };
    int voiceCount = 0;

    if (firstVoice != nullptr) {
        const unsigned voiceTotal = nslVoiceCount();
        for (unsigned voiceIndex = 0; voiceIndex < voiceTotal; ++voiceIndex) {
            unsigned char* voiceRaw = reinterpret_cast<unsigned char*>(firstVoice) +
                320u * voiceIndex;
            const nslSourceID sourceID =
                *reinterpret_cast<const nslSourceID*>(voiceRaw + 0x114u);
            if (nslGetSourceState(sourceID) == NSL_SOURCE_STATE_INVALID)
                continue;

            const nslWaveID voiceWaveID =
                *reinterpret_cast<const nslWaveID*>(voiceRaw + 0x110u);
            nslWave* voiceWave = nslWavePtr(voiceWaveID);
            if (voiceWave != nullptr) {
                const unsigned char* metadata =
                    *reinterpret_cast<const unsigned char* const*>(voiceWave);
                if (metadata != nullptr && (metadata[7] & 1u) != 0)
                    continue;
            }

            nslSource* source = nslSourcePtr(sourceID);
            int priority;
            if (source == nullptr) {
                priority = 100;
            } else {
                const unsigned char* sourceRaw =
                    reinterpret_cast<const unsigned char*>(source);
                const nslWaveID sourceWaveID =
                    *reinterpret_cast<const nslWaveID*>(sourceRaw + 0x110u);
                nslWave* sourceWave = nslWavePtr(sourceWaveID);
                const unsigned char* metadata = sourceWave == nullptr
                    ? nullptr
                    : *reinterpret_cast<const unsigned char* const*>(sourceWave);
                if (sourceWave != nullptr && metadata != nullptr &&
                    (metadata[7] & 1u) == 0) {
                    priority = 100;
                } else {
                    const float attenuation = nslSourceGetAttenuation(source);
                    const float maxAttenuation = source->params[10];
                    float clampedAttenuation = attenuation;
                    const float minAttenuation = source->params[9];
                    if (clampedAttenuation > maxAttenuation)
                        clampedAttenuation = maxAttenuation;
                    if (minAttenuation > clampedAttenuation)
                        clampedAttenuation = minAttenuation;
                    priority = static_cast<int>(
                        (((clampedAttenuation - minAttenuation) /
                          (maxAttenuation - minAttenuation)) *
                         (source->params[8] - source->params[7])) +
                        source->params[7]);
                }
            }

            const int sourcePriority = source == nullptr
                ? dword_F4240
                : *reinterpret_cast<const int*>(
                    reinterpret_cast<const unsigned char*>(source) + 0x128u);
            ++voiceCount;

            int insertIndex = 0;
            while (insertIndex < 5) {
                const PriorityEntry& entry = entries[insertIndex];
                if (priority <= entry.priority &&
                    (priority != entry.priority ||
                     sourcePriority >= entry.sourcePriority))
                    break;
                ++insertIndex;
            }
            if (insertIndex <= 3) {
                for (int index = 4; index > insertIndex; --index)
                    entries[index] = entries[index - 1];
            }
            if (insertIndex < 5) {
                entries[insertIndex].priority = priority;
                entries[insertIndex].voiceIndex = static_cast<int>(voiceIndex);
                if (source != nullptr)
                    entries[insertIndex].sourcePriority = sourcePriority;
            }
        }
    }

    s_LastVoiceCount = voiceCount;
    const int voicesToStop = voiceCount - 59;
    for (int index = 0; index < voicesToStop; ++index) {
        if (index >= 5)
            break;
        unsigned char* voiceRaw = reinterpret_cast<unsigned char*>(firstVoice) +
            320u * static_cast<unsigned>(entries[index].voiceIndex);
        const nslSourceID sourceID =
            *reinterpret_cast<const nslSourceID*>(voiceRaw + 0x114u);
        nslStopSource(sourceID);
    }
}

void          nslVoiceSetPriority(nslVoice*, unsigned) {}
void          nslSourceSetPriorityScale(nslSource*, float) {}

// ============================================================================
// nslVoice — voice allocation
// ============================================================================
// ea: 0x008285E0
nslVoice*     nslVoicePtr(int voiceIndex) {
    const unsigned index = static_cast<unsigned>(voiceIndex);
    if (index > nsl_initParams.aramSize)
        return nullptr;
    return reinterpret_cast<nslVoice*>(
        reinterpret_cast<unsigned char*>(nsl_voices) + 320u * index);
}
// ea: 0x00828610
unsigned      nslVoiceCount() { return nsl_initParams.aramSize; }
int           nslVoiceAlloc(nslWaveID, nslSourceID, int) { return -1; }
// ea: 0x00828700
void          nslVoiceFree(int voiceIndex) {
    const unsigned index = static_cast<unsigned>(voiceIndex);
    if (index >= nsl_initParams.aramSize)
        return;
    unsigned char* raw = reinterpret_cast<unsigned char*>(nsl_voices) +
        320u * index;
    if (raw[0x108] == 0)
        return;
    if (raw[0x108] == 1) {
        raw[0x108] = 0;
    } else if (raw[0x108] != 7) {
        raw[0x109] |= 8u;
    }
}
int           nslVoiceGetState(int) { return NSL_VOICE_FREE; }
void          nslVoiceSetVolume(int, float) {}

// ============================================================================
// nslDriverXBOXDSOUND — Xbox DirectSound driver (XAudio2 replacement)
// ============================================================================
// ea: 0x00824590
unsigned      nslDriverVoiceSize() { return 172u; }
// ea: 0x00824680
float         nslDriverClamp(float value, float min, float max) {
    float clamped = value;
    if (value > max) {
        clamped = max;
        value = max;
    }
    if (min > clamped)
        return min;
    return value;
}
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
const char*   nslGetStateText(int) { return "ready"; }
