// ============================================================================
// NSL — NGL Sound Library (237 funcs, 17 objects)
// ea: 0x8E0000-0x8XXXXX (sound subsystem)
// Xbox DirectSound wrapper — stubbed for Win32/XAudio2.
// ============================================================================

#include <cstdint>
#include <cstring>
#include <cstdlib>

#include "core/tlFixedString.h"

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);
extern "C" void txAssertFailed(unsigned char* ignore, const char* message,
                                const char* function, const char* source, int line);
extern "C" void txPrintf(const char* channel, int level, const char* fmt, ...);

// ============================================================================
// Handle types
// ============================================================================
enum nslSourceID : int { NSL_SOURCE_ID_INVALID = -1 };
typedef unsigned nslEmitterID;
enum nslWaveID : int { NSL_WAVE_ID_INVALID = -1 };
typedef unsigned nslWaveBankID;
typedef unsigned nslGroupID;
typedef unsigned nslVoiceID;

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
struct nslSource {};
struct nslEmitter {};
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
struct nslGroup {};
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
static void* nsl_sourceEntries = nullptr;
static void* nsl_emitterEntries = nullptr;
static void* nsl_sources = nullptr;
static void* nsl_sourcesSorted = nullptr;
static void* nsl_emitters = nullptr;
static void* nsl_groups = nullptr;
static void* nsl_voices = nullptr;
static void* nsl_driverVoices = nullptr;

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

    nsl_sourceEntries = nslInit_Allocate(12u * nsl_initParams.maxSources, 0x100u);
    if (nsl_sourceEntries != nullptr) {
        nsl_emitterEntries = nslInit_Allocate(12u * nsl_initParams.maxEmitters, 0x100u);
        if (nsl_emitterEntries != nullptr) {
            nsl_sources = nslInit_Allocate(328u * nsl_initParams.maxSources, 0x100u);
            if (nsl_sources != nullptr) {
                nsl_sourcesSorted = nslInit_Allocate(4u * nsl_initParams.maxSources, 0x100u);
                if (nsl_sourcesSorted != nullptr) {
                    nsl_emitters = nslInit_Allocate(272u * nsl_initParams.maxEmitters, 0x100u);
                    if (nsl_emitters != nullptr) {
                        nsl_groups = nslInit_Allocate(288u * static_cast<unsigned>(dword_E4B690), 0x100u);
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
nslEmitterID  nslNewEmitter(const float* pos) { return 0; }
nslSourceID   nslNewSource(nslWaveID waveID, int mImportance) { return NSL_SOURCE_ID_INVALID; }
void          nslDeleteSource(nslSourceID) {}
void          nslDeleteEmitter(nslEmitterID) {}
nslSource*    nslSourcePtr(nslSourceID) { return nullptr; }
nslEmitter*   nslEmitterPtr(nslEmitterID) { return nullptr; }
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

// Voice enumeration (used by EffectEventSys::NumberOfVoicesUsed)
unsigned      nslGetNumVoices() { return 0; }
nslVoice*     nslGetVoice(unsigned) { return nullptr; }
nslSourceState nslGetSourceState(nslSourceID) {
    return NSL_SOURCE_STATE_INVALID;
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
float         nslGetSourceParam(nslSourceID, int, float defaultValue) { return defaultValue; }  // ?nslGetSourceParam@@YAMW4nslSourceID@@HM@Z (nslSource.o)
int           nslIsWaveStreamed(nslWaveID) { return 0; }     // ?nslIsWaveStreamed@@YAHW4nslWaveID@@@Z (nslCompat.o)
void          nslGetSourcePosition(nslSourceID, float* position) {}  // ?nslGetSourcePosition@@YAXW4nslSourceID@@QAM@Z (nslSource.o)

// nslCompat.o / nslSource.o family (stubbed; manglings match binary)
int           nslGetBankState(nslBankID bankID) { return nslWaveBankGetState(static_cast<nslWaveBankID>(bankID)); }
void          nslFreeBank(nslBankID bankID) { nslWaveBankFree(static_cast<nslWaveBankID>(bankID)); }
void          nslStopSource(nslSourceID) {}
void          nslQueueSource(nslSourceID) {}
void          nslFreeSource(nslSourceID) {}
void          nslSetSourceParam(nslSourceID, int, float) {}
void          nslSetSourcePosition(nslSourceID, const float*) {}
void          nslSetSourceVelocity(nslSourceID, const float*) {}
void          nslDampen(float) {}
void          nslUndampen() {}
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
}
void          nslExit() {}
void          nslSetEffect(const void*) {}
void          nslSetListenerPosition(const float*) {}
void          nslSetListenerOrientation(const float*, const float*) {}
void          nslSetBusPitchAddBus(unsigned int) {}
void          nslSetBusPitchRemoveBus(unsigned int) {}
void          nslSetBusVolumeAddBus(unsigned int) {}
void          nslSetBusVolumeRemoveBus(unsigned int) {}
unsigned int  nslWaveGetHash(nslWaveID) { return 0; }
int           nslGetWaveLength(nslWaveID) { return 0; }      // ?nslGetWaveLength@@YAHW4nslWaveID@@@Z
unsigned      nslGetSourceLength(nslSourceID) { return 0; }  // ?nslGetSourceLength@@YAIW4nslSourceID@@@Z
int           nslIsWaveLooped(nslWaveID) { return 0; }       // ?nslIsWaveLooped@@YAHW4nslWaveID@@@Z
void          nslSetSourceEffectOn(nslSourceID) {}           // ?nslSetSourceEffectOn@@YAXW4nslSourceID@@@Z
void          nslSetSourceEffectOff(nslSourceID) {}          // ?nslSetSourceEffectOff@@YAXW4nslSourceID@@@Z
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
nslGroup*     nslGroupGet(unsigned) { return nullptr; }
nslGroup*     nslGroupGet(const char*) { return nullptr; }
void          nslGroupSetParam(const char*, unsigned, float) {}
void          nslGroupSetVolume(nslGroupID, float) {}
void          nslGroupSetPitch(nslGroupID, float) {}
float         nslGroupGetVolume(nslGroupID) { return 1.0f; }

// ============================================================================
// nslMaster — master bus
// ============================================================================
nslGroup*     nslMasterGetGroup() { return nullptr; }
float         nslBusVolume = 1.0f;  // ?nslBusVolume@@3MA @ 0xE4B674
float         nslBusPitch = 1.0f;   // ?nslBusPitch@@3MA @ 0xE4B678
void          nslSetBusVolume(unsigned, float) {}
void          nslSetBusVolume(float volume) { nslBusVolume = volume; }
float         nslGetBusVolume() { return nslBusVolume; }
float         nslGetBusVolume(unsigned) { return 1.0f; }
bool          nslIsBusVolumeName(unsigned) { return false; }
const char*   nslGetBusName(unsigned) { return ""; }
unsigned      nslGetBusIndex(const char*) { return 0; }
void          nslSetBusPitch(unsigned, float) {}
void          nslSetBusPitch(float pitch) { nslBusPitch = pitch; }
float         nslGetBusPitch() { return nslBusPitch; }
void          nslSetBusFilter(unsigned, unsigned, float) {}
void          nslSetBusReverb(unsigned, float) {}
void          nslSetMasterVolume(float) {}
float         nslGetMasterVolume() { return 1.0f; }

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
int           nslGetNumberOfListeners() { return 1; }
void          nslSetNumberOfListeners(int) {}
nslGroup*     nslListenerGetGroup(unsigned) { return nullptr; }
void          nslListenerSetPosition(unsigned, const float*) {}
void          nslListenerSetOrientation(unsigned, const float*, const float*) {}
void          nslListenerSetVelocity(unsigned, const float*) {}
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
