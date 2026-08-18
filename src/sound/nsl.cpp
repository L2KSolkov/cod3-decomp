// ============================================================================
// NSL — NGL Sound Library (237 funcs, 17 objects)
// ea: 0x8E0000-0x8XXXXX (sound subsystem)
// Xbox DirectSound wrapper — stubbed for Win32/XAudio2.
// ============================================================================

#include <cstdint>
#include <cstring>

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
enum nslSpeakerMode        { NSL_SPEAKER_STEREO=0, NSL_SPEAKER_5_1=1, NSL_SPEAKER_MONO=2 };
enum nslWaveBankLoaderState{ NSL_WB_LOADING=0, NSL_WB_READY=1, NSL_WB_FAILED=2 };
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
struct nslWave {};
struct nslWaveBank {};
enum nflFileID : unsigned { NFL_FILE_ID_INVALID = (unsigned)-1 };
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
struct nslWaveName {};
struct nslDriverParams {};
struct nslWaveBankLoader {};

// Release globals verified from IDA addresses 0xE4B680, 0x10E11AC-0x10E11F0,
// and 0x10E1220.  These are the state consumed by the bank-slot functions.
nslInitParams nsl_initParams = { 512u, 0u, 8u, 0u };
static void* nsl_work = nullptr;
static unsigned nsl_workUsed = 0;
static unsigned nsl_workLimit = 0;
static unsigned nsl_time = 0;
static unsigned nsl_frame = 0;
static unsigned nsl_waveBankLoadOrder = 0;
nslWaveBankSlot* nsl_waveBankSlots = nullptr;
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
nslSpeakerMode nslGetSpeakerMode() { return NSL_SPEAKER_STEREO; }
void         nslSetSpeakerMode(nslSpeakerMode) {}
void         nslGetInitParams(nslInitParams*) {}
void         nslInitDefaults() {}
void         nslFinalInit() {}
bool         nslIsInitDone() { return true; }

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
void          nslFreeBank(nslBankID) {}
void          nslStopSource(nslSourceID) {}
void          nslQueueSource(nslSourceID) {}
void          nslFreeSource(nslSourceID) {}
void          nslSetSourceParam(nslSourceID, int, float) {}
void          nslSetSourcePosition(nslSourceID, const float*) {}
void          nslSetSourceVelocity(nslSourceID, const float*) {}
void          nslDampen(float) {}
void          nslUndampen() {}
void          nslUpdateBanks() {}
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
unsigned int nsl_aramSize = 0;  // ?nsl_aramSize@@3IA (nslAram.o)
unsigned int nsl_aramFree = 0;  // ?nsl_aramFree@@3IA (nslAram.o)
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
nslWaveBank*  nslWaveBankPtr(nslWaveBankID) { return nullptr; }
nslWaveBankSlot* nslWaveBankGetSlot(nslWaveBankID) { return nullptr; }
unsigned      nslWaveBankCount() { return 0; }
nslWaveBankID nslWaveBankGetFirst() { return NSL_INVALID_BANK; }
nslWaveBankID nslWaveBankGetNext(nslWaveBankID) { return NSL_INVALID_BANK; }
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
void          nslWaveBankSort(nslWaveBank*) {}
nslWaveID     nslWaveBankGetWave(nslWaveBankID, const char*) { return NSL_INVALID_WAVE; }
nslWaveID     nslWaveBankGetWaveByIndex(nslWaveBankID, unsigned) { return NSL_INVALID_WAVE; }
unsigned      nslWaveBankGetWaveCount(nslWaveBankID) { return 0; }
const char*   nslWaveBankGetWaveName(nslWaveBankID, unsigned) { return ""; }
int           nslWaveNameCompareHash(const nslWaveName*, const nslWaveName*) { return 0; }
int           nslWaveNameCompareText(const nslWaveName*, const nslWaveName*) { return 0; }

// ============================================================================
// nslWaveBankSort
// ============================================================================
void          nslWaveBankSortSwap(nslWaveName*, nslWave*, unsigned, unsigned) {}
void          nslWaveBankSortRecursive(nslWaveBank*, int, int, int (*cmp)(const nslWaveName*, const nslWaveName*)) {}

// ============================================================================
// nslWaveBankLoader — async wave bank loading
// ============================================================================
void          nslWaveBankLoaderInit(nslWaveBankLoader*, unsigned, nflFileID, unsigned) {}
int           nslWaveBankLoaderUpdate(nslWaveBankLoader*) { return 1; }
void          nslWaveBankLoaderCancel(nslWaveBankLoader*) {}

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
void*         nslAramGetBase() { return nullptr; }
unsigned      nslAramGetSize() { return 0; }
unsigned      nslAramGetFree() { return 0; }
void*         nslAramAlloc(unsigned) { return nullptr; }
void          nslAramFree(void*) {}
unsigned      nslAramGetUsed() { return 0; }
bool          nslAramIsInAram(void*) { return false; }
unsigned      nslAramGetAlignment() { return 16; }
void          nslAramCompact() {}

// ============================================================================
// nslMemory — audio heap wrappers
// ============================================================================
void*         nslMemoryAlloc(unsigned sz) { return nullptr; }
void          nslMemoryFree(void* p) {}

// ============================================================================
// Misc
// ============================================================================
bool          nslGetState() { return true; }
const char*   nslGetStateText(int) { return "ready"; }
