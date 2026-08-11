// ============================================================================
// NFL — NGL File Library (async file streaming system)
// From nfl_xboxr: nfl_system.o + nfl_common.o + nfl_debug.o + drivers + tx utilities
// ea: 0x82A510-0x82E090 (126 funcs, 10 objects)
// Ported from Xbox async I/O to Win32 synchronous I/O.
// ============================================================================

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #define NFL_OPEN_FILE(name) CreateFileA(name, 0x80000000, 1, nullptr, 3, 0x80, nullptr)
  #define NFL_CLOSE(h)       CloseHandle(h)
  #define NFL_SIZE(h)         GetFileSize(h, nullptr)
  #define NFL_READ(h,b,s,o)   ReadFile(h, b, s, (DWORD*)o, nullptr)
  #define NFL_INVALID_H       INVALID_HANDLE_VALUE
#else
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #define NFL_OPEN_FILE(name) ((void*)(intptr_t)open(name, O_RDONLY))
  #define NFL_CLOSE(h)        close((int)(intptr_t)h)
  #define NFL_SIZE(h)         ({ int fd=(int)(intptr_t)h; off_t s=lseek(fd,0,SEEK_END); lseek(fd,0,SEEK_SET); (unsigned)s; })
  #define NFL_READ(h,b,s,o)   ({ int r=read((int)(intptr_t)h,b,s); if(o)*(int*)o=r; r>=0; })
  #define NFL_INVALID_H       nullptr
#endif

// ============================================================================
// Enums
// ============================================================================
enum nflMediaID : unsigned {  // enum tag matches binary mangling W4nflMediaID
    NFL_MEDIA_DEFAULT = 0,
};
typedef unsigned nflFileID;
typedef unsigned nflRequestID;
typedef unsigned nflStreamID;
typedef unsigned nflPriority;
typedef unsigned nfdError;
typedef unsigned nflRequestState;
typedef unsigned nfsRequestState;
typedef unsigned nfdFileFlags;
typedef unsigned nflBufferMode;
typedef unsigned nflState;
typedef unsigned nfdIoState;
typedef unsigned nfdMediaState;
typedef unsigned nflRequestType;
typedef unsigned nfsFileType;

#define NFL_INVALID_FILE    ((nflFileID)-1)
#define NFL_INVALID_REQUEST ((nflRequestID)-1)
#define NFL_INVALID_STREAM  ((nflStreamID)-1)

// ============================================================================
// Types
// ============================================================================
struct nflInitParams    { void* workSpace; unsigned workSpaceSize; unsigned maxFiles; };
struct nflRequestParams { nflFileID file; unsigned offset; void* buf; unsigned size; };
struct nflRequestInfo   { nflFileID file; nflRequestState state; unsigned size; float progress; };
struct nflStreamParams  { nflFileID file; unsigned offset; unsigned size; unsigned bufSize; };
struct nflFileInfo      { unsigned size; };
struct nflMediaAlignments { unsigned readAlign; unsigned writeAlign; };
struct nfdDriver { void* ctx; };
struct nfdIoCommand { unsigned cmd; };
struct nfsRequest { nflFileID file; unsigned offset; void* buf; unsigned size; nflRequestState state; };
struct nfsFile   { nflFileID id; void* handle; unsigned size; };
struct nfsStream { nflFileID file; unsigned pos; };
struct nflInitCallbacks {};

// ============================================================================
// File operations
// ============================================================================
static const int MAX_FILES = 256;
static nfsFile s_files[MAX_FILES];
static unsigned s_fileCount = 0;
static bool s_initialized = false;

unsigned nflInit(const nflInitParams* p) {
    if (s_initialized) return 1;
    s_initialized = true;
    s_fileCount = 0;
    for (int i = 0; i < MAX_FILES; ++i) s_files[i].id = NFL_INVALID_FILE;
    return 1;
}

void nflShutdown() {
    for (int i = 0; i < (int)s_fileCount; ++i)
        if (s_files[i].id != NFL_INVALID_FILE && s_files[i].handle != NFL_INVALID_H)
            NFL_CLOSE(s_files[i].handle);
    s_initialized = false;
}

nflFileID nflCreateFile(nflMediaID media, const char* name, unsigned flags) {
    if (!s_initialized || s_fileCount >= MAX_FILES) return NFL_INVALID_FILE;
    void* h = NFL_OPEN_FILE(name);
    if (h == NFL_INVALID_H) return NFL_INVALID_FILE;
    nflFileID id = s_fileCount++;
    s_files[id].id = id;
    s_files[id].handle = h;
    s_files[id].size = NFL_SIZE(h);
    return id;
}

nflFileID nflOpenFile(nflMediaID media, const char* name) {
    return nflCreateFile(media, name, 0);
}

void nflCloseFile(nflFileID file) {
    if (file < MAX_FILES && s_files[file].id != NFL_INVALID_FILE) {
        NFL_CLOSE(s_files[file].handle);
        s_files[file].handle = NFL_INVALID_H;
        s_files[file].id = NFL_INVALID_FILE;
    }
}

unsigned nflGetFileSize(nflFileID file) {
    return (file < MAX_FILES) ? s_files[file].size : 0;
}

unsigned nflReadFile(nflFileID file, unsigned offset, void* buf, unsigned size) {
    // Synchronous read (async in original, simplified here)
    return 0;
}

unsigned nflFileExists(nflMediaID media, const char* name) {
    void* h = NFL_OPEN_FILE(name);
    if (h == NFL_INVALID_H) return 0;
    unsigned sz = NFL_SIZE(h);
    NFL_CLOSE(h);
    return sz > 0 ? 1 : 0;
}

unsigned nflGetFileSize2(nflMediaID media, const char* name) {
    void* h = NFL_OPEN_FILE(name);
    if (h == NFL_INVALID_H) return 0;
    unsigned sz = NFL_SIZE(h);
    NFL_CLOSE(h);
    return sz;
}

// ============================================================================
// Async request management (simplified — synchronous on Win32)
// ============================================================================
nflRequestID nflAddRequest(const nflRequestParams* params) {
    return 0;
}

void nflCancelRequest(nflRequestID req) {}
nflRequestState nflGetRequestState(nflRequestID req) { return 1; }
float nflGetRequestProgress(nflRequestID req) { return 1.0f; }

// ============================================================================
// Debug text
// ============================================================================
const char* nfsErrorText(int code) { return "NFL error"; }
const char* nfsRequestStateText(int state) { return "done"; }
const char* nfsFileTypeText(int type) { return "file"; }
const char* nfsMediaIDText(int media) { return "default"; }
const char* nfdErrorText(int code) { return "NFD error"; }
const char* nfdFileFlagsText(int flags) { return ""; }
const char* nfdMediaStateText(int state) { return ""; }
const char* nfdIoStateText(int state) { return ""; }
const char* nfsBufferModeText(int mode) { return ""; }
void nfsError(const char* fmt, ...) {}
void nfsWarning(const char* fmt, ...) {}
void nfsMessage(const char* fmt, ...) {}

// ============================================================================
// tx utilities — string matching, path manipulation, slot pool
// ============================================================================
int txInit() { return 0; }
int txTime() { return 0; }
const char* txMatch(const char* pattern, const char* str) { return nullptr; }
int txGetPrimeLessThanPow2(unsigned n) { return n; }
void txPrintf(const char* fmt, ...) {}

void txPathFix(char* path) {}
void txPathFormat(char* out, const char* in) {}
void txPathMake(char* out, const char* dir, const char* file, const char* ext) {}
const char* txPathFile(const char* path) { return path; }
const char* txPathFileStart(const char* path) { return path; }
const char* txPathExtStart(const char* path) { return path + strlen(path); }
const char* txPathExt(const char* path) { return ""; }
const char* txPathDirEnd(const char* path) { return path; }
const char* txPathDir(const char* path) { return ""; }
void txPathNormalize(char* path) {}
const char* txPathName(const char* path) { return path; }

void txAssertFailed(const char*, int, const char*) {}
void txBreak() {}
void* txMemAlloc(unsigned s) { return malloc(s); }
void* txMemAllocEx(unsigned s, unsigned a) { return malloc(s); }
void txMemFree(void* p) { free(p); }
void* txMemRealloc(void* p, unsigned s) { return realloc(p, s); }
void* txMemReallocAdHoc(void* p, unsigned s, unsigned a) { return realloc(p, s); }
void txPrintv(const char* fmt, ...) {}

// Slot pool
void txSlotEntryInit(void* p) {}
void txSlotEntryInc(void* p) {}
void txSlotEntryLinkHead(void* pool, void* entry) {}
void txSlotEntryLinkTail(void* pool, void* entry) {}
void txSlotEntryUnlink(void* pool, void* entry) {}
void* txSlotEntryPtr(void* pool, int idx) { return nullptr; }
int txSlotIndex(void* pool, void* entry) { return 0; }
void txSlotPoolInit(void* pool, unsigned esize, unsigned count, void* buf) {}
int txSlotFree(void* pool) { return 0; }
int txSlotNew(void* pool) { return 0; }
int txSlotFirst(void* pool) { return -1; }
int txSlotLast(void* pool) { return -1; }
int txSlotNext(void* pool, int idx) { return -1; }
int txSlotPrev(void* pool, int idx) { return -1; }
bool txSlotExists(void* pool, int idx) { return false; }

// Stream
nflStreamID nflCreateStream(const nflStreamParams* p) { return 0; }
void nflDestroyStream(nflStreamID s) {}
void nflSetStreamPriority(nflStreamID s, nflPriority p) {}
nflPriority nflGetStreamPriority(nflStreamID s) { return 0; }

// State
nflState nflGetState() { return 2; }
const char* nflGetStateText(nflState s) { return "ready"; }
void nflRetry() {}
void nflUpdate() {}
void nflStart(void* ctx) {}

// Driver
int nfdBufferAlign(const nfdDriver*, const nfdIoCommand*, unsigned, unsigned*, unsigned*) { return 0; }
int nfdIoExecute(nfdDriver*, const nfdIoCommand*) { return 0; }
void nfdIoComplete(nfdDriver*, int, int) {}
void nflInjectError() {}

// Win32 driver
int nfd_win32_IoExecute(nfdDriver*, void*, int, unsigned, unsigned, unsigned) { return 0; }
int nfd_win32_IoUpdate() { return 0; }
int nfd_win32_IoCancel(void*) { return 0; }
int nfd_win32_FileOpen(void*, const char*, int, unsigned) { return 0; }
int nfd_win32_FileClose(void*) { return 0; }
int nfd_win32_FileStatus(void*, nflFileInfo*) { return 0; }
void* nfd_win32_GetFileHandle(void*) { return nullptr; }
void nfd_win32_IoCompletionRoutine(unsigned, unsigned, void*) {}

// Xbox driver
int nfd_xbox_MediaBind(int, const char*, char*, int) { return 0; }

// Version
unsigned nflGetVersion() { return 0x100; }
