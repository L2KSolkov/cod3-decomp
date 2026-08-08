// ============================================================================
// NSL — NGL Sound Library (237 funcs, 17 objects)
// ea: 0x8E0000-0x8XXXXX (sound subsystem)
// Xbox DirectSound wrapper — stubbed for Win32/XAudio2.
// ============================================================================

#include <cstdint>

// ============================================================================
// Handle types
// ============================================================================
typedef unsigned nslSourceID;
typedef unsigned nslEmitterID;
typedef unsigned nslWaveID;
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
enum nslSourceState        { NSL_SOURCE_STATE_INVALID=0, NSL_SOURCE_STATE_PLAYING=1 };

// ============================================================================
// Forward types
// ============================================================================
struct nslInitParams { unsigned maxVoices; unsigned maxSources; nslSpeakerMode speakerMode; };
struct nslSource {};
struct nslEmitter {};
struct nslWave {};
struct nslWaveBank {};
struct nslWaveBankSlot {};
struct nslGroup {};
struct nslVoice {};
struct nslListener {};
struct nslWaveName {};
struct nslDriverParams {};
struct nslWaveBankLoader {};
typedef unsigned nflFileID;

// ============================================================================
// nslInit — init/shutdown
// ============================================================================
void         nslInit(const nslInitParams*) {}
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
nslSourceID   nslNewSource(nslEmitterID, nslWaveID, unsigned flags) { return 0; }
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
nslWaveBankID nslWaveBankLoad(const char*) { return NSL_INVALID_BANK; }
nslWaveBankID nslWaveBankLoadInPlace(void*) { return NSL_INVALID_BANK; }
void          nslWaveBankRelease(nslWaveBankID) {}
nslWaveBank*  nslWaveBankPtr(nslWaveBankID) { return nullptr; }
nslWaveBankSlot* nslWaveBankGetSlot(nslWaveBankID) { return nullptr; }
unsigned      nslWaveBankCount() { return 0; }
nslWaveBankID nslWaveBankGetFirst() { return NSL_INVALID_BANK; }
nslWaveBankID nslWaveBankGetNext(nslWaveBankID) { return NSL_INVALID_BANK; }
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
void          nslSetBusVolume(unsigned, float) {}
float         nslGetBusVolume(unsigned) { return 1.0f; }
bool          nslIsBusVolumeName(unsigned) { return false; }
const char*   nslGetBusName(unsigned) { return ""; }
unsigned      nslGetBusIndex(const char*) { return 0; }
void          nslSetBusPitch(unsigned, float) {}
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
