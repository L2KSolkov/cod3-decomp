// NFL — NGL File Library (nfl_xboxr)
// Win32 driver port of the IDA-typed file, stream, and request layer.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cctype>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/stat.h>
#endif

enum nflMediaID : unsigned {
    NFL_MEDIA_ID_DISC = 1,
    NFL_MEDIA_ID_HOST = 2,
    NFL_MEDIA_ID_LINK = 4,
    NFL_MEDIA_DEFAULT = 3,
    NFL_MEDIA_ID_DEFAULT = 3,
    NFL_MEDIA_ID_ALL = 0x7FFFFFFF,
    NFL_MEDIA_ID_INVALID = 0xFFFFFFFFu,
};
enum nflFileID : unsigned { NFL_FILE_ID_INVALID = 0xFFFFFFFFu };
enum nflRequestID : unsigned { NFL_REQUEST_ID_INVALID = 0xFFFFFFFFu };
enum nflStreamID : unsigned {
    NFL_STREAM_ID_INVALID = 0xFFFFFFFFu,
    NFL_STREAM_ID_DEFAULT = 0,
};
enum nflPriority : unsigned {
    NFL_PRIORITY_INVALID = 0xFFFFFFFFu,
    NFL_PRIORITY_LOWEST = 0,
    NFL_PRIORITY_LOW = 1,
    NFL_PRIORITY_NORMAL = 2,
    NFL_PRIORITY_HIGH = 3,
    NFL_PRIORITY_HIGHEST = 4,
};
enum nflRequestType : unsigned {
    NFL_REQUEST_TYPE_INVALID = 0xFFFFFFFFu,
    NFL_REQUEST_TYPE_READ = 0,
    NFL_REQUEST_TYPE_WRITE = 1,
};
enum nflBufferMode : unsigned {
    NFL_BUFFER_MODE_DEFAULT = 0,
    NFL_BUFFER_MODE_NOBUFFER = 1,
    NFL_BUFFER_MODE_UNALIGNED = 2,
    NFL_BUFFER_MODE_ALL = 3,
    NFL_BUFFER_MODE_VM = 4,
};
enum nflThreadMode : unsigned {
    NFL_THREAD_MODE_SINGLE = 0,
    NFL_THREAD_MODE_MULTI = 1,
};
enum nflRequestState : unsigned {
    NFL_REQUEST_STATE_INVALID = 0xFFFFFFFFu,
    NFL_REQUEST_STATE_COMPLETED = 0,
    NFL_REQUEST_STATE_CANCELED = 1,
    NFL_REQUEST_STATE_TIMEOUT = 2,
    NFL_REQUEST_STATE_ERROR = 3,
    NFL_REQUEST_STATE_ACTIVE = 4,
};
enum nfsRequestState : unsigned {
    NFS_REQUEST_STATE_WAITING = 0,
    NFS_REQUEST_STATE_WORKING = 1,
    NFS_REQUEST_STATE_WORKDONE = 2,
    NFS_REQUEST_STATE_CANCELING = 3,
    NFS_REQUEST_STATE_CANCELED = 4,
    NFS_REQUEST_STATE_TIMEOUT = 5,
    NFS_REQUEST_STATE_IO_ERROR = 6,
    NFS_REQUEST_STATE_IO_ERROR_WAITING = 7,
};
enum nfsFileType : unsigned {
    NFS_FILE_TYPE_NATIVE = 0,
    NFS_FILE_TYPE_SUBFILE = 1,
};
enum nfdError : unsigned {
    NFD_ERROR_INVALID = 0xFFFFFFFFu,
    NFD_ERROR_NOERROR = 0,
    NFD_ERROR_FAILURE = 1,
    NFD_ERROR_EOF = 2,
    NFD_ERROR_UNALIGNED_ACCESS = 3,
    NFD_ERROR_INVALID_MEDIA = 4,
    NFD_ERROR_INVALID_STATE = 5,
    NFD_ERROR_INVALID_FLAGS = 6,
    NFD_ERROR_INVALID_FILENAME = 7,
    NFD_ERROR_INVALID_ARGUMENTS = 8,
};
enum nfdMediaState : unsigned {
    NFD_MEDIA_STATE_INVALID = 0xFFFFFFFFu,
    NFD_MEDIA_STATE_LOADING = 0,
    NFD_MEDIA_STATE_LOADED = 1,
    NFD_MEDIA_STATE_LOADFAILED = 2,
    NFD_MEDIA_STATE_UNLOADING = 3,
    NFD_MEDIA_STATE_UNLOADED = 4,
};
enum nfdFileFlags : unsigned {
    NFD_FILE_FLAGS_INVALID = 0xFFFFFFFFu,
    NFD_FILE_FLAGS_READ = 1,
    NFD_FILE_FLAGS_WRITE = 2,
    NFD_FILE_FLAGS_CREATE = 4,
    NFD_FILE_FLAGS_ASYNC_OPEN = 8,
};
enum nfdIoState : unsigned {
    NFD_IO_STATE_INVALID = 0xFFFFFFFFu,
    NFD_IO_STATE_IDLE = 0,
    NFD_IO_STATE_WORKING = 1,
    NFD_IO_STATE_WORKDONE = 2,
    NFD_IO_STATE_CANCELING = 3,
    NFD_IO_STATE_CANCELED = 4,
    NFD_IO_STATE_ERROR = 5,
};
enum nflState : unsigned {
    NFL_STATE_INVALID = 0xFFFFFFFFu,
    NFL_STATE_IDLE = 0,
    NFL_STATE_BUSY = 1,
    NFL_STATE_ERROR = 2,
};

typedef unsigned txSlot;
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
static const txSlot TX_SLOT_INVALID = (txSlot)-1;

struct nflInitParams {
    unsigned maxFiles;
    unsigned maxStreams;
    unsigned maxRequests;
    nflBufferMode bufferMode;
    nflThreadMode threadMode;
};
struct nflRequestParams {
    nflFileID fileID;
    nflStreamID streamID;
    void (*callback)(nflRequestState, nflRequestID, void*);
    nflRequestType type;
    nflPriority priority;
    unsigned fileOffset;
    void* buffer;
    unsigned dataSize;
    unsigned timeout;
    void* userData;
    nflRequestParams();
};
struct nflRequestInfo {
    unsigned bytesCompleted;
    unsigned timeElapsed;
    unsigned activeTimeElapsed;
};
struct nflStreamParams {
    nflPriority streamPriority;
    nflStreamParams();
};
struct nflSimulateError {
    int errorCode;
    unsigned count;
    unsigned delayMin;
    unsigned delayMax;
    unsigned delayCnt;
};
struct nflMediaAlignments {
    unsigned mediaAlignment;
    unsigned memoryAlignment;
    unsigned transferSizeAlignment;
};
struct nfdFileInfo { unsigned location; unsigned size; };

struct nfdDriver;
struct nfdMediaInfo { nfdMediaState mediaState; };
struct nfdMedia {
    unsigned registry;
    nfdError (*fnBind)(nflMediaID, const char*, char*, int);
    nfdError (*fnLoad)(nflMediaID);
    nfdError (*fnUnload)(nflMediaID);
    nfdError (*fnStatus)(nflMediaID, nfdMediaInfo*);
};
struct nfdInit {
    nflBufferMode bufferMode;
    nfdError (*fnInit)(nfdDriver*);
    nfdError (*fnDone)(nfdDriver*);
};
struct nfsFile {
    unsigned size;
    union {
        struct {
            nflMediaID mediaID;
            nfdDriver* driver;
            nfdFileFlags flags;
            int childCount;
            char filename[256];
        } native;
        struct {
            unsigned offset;
            unsigned parent;
            unsigned chunkSize;
            unsigned strideSize;
        } subfile;
    } as;
    txSlotEntry slotEntry;
    nfsFileType fileType;
};
struct nfsStream {
    txSlotEntry slotEntry;
    nflPriority priority;
};
struct nfsRequest {
    unsigned fileOffset;
    nflRequestType type;
    unsigned streamID;
    unsigned fileID;
    void (*callback)(nflRequestState, nflRequestID, void*);
    void* callbackData;
    unsigned queueTime;
    unsigned expirationTime;
    unsigned priority;
    char* buffer;
    unsigned bufferSize;
    nfsRequestState state;
    unsigned lastWorkingTime;
    unsigned bytesCompleted;
    txSlotEntry slotEntry;
};
struct nfsRequestCB {
    nflRequestState state;
    void (*callback)(nflRequestState, nflRequestID, void*);
    nflRequestID requestID;
    void* callbackData;
    int shouldDie;
};
struct nfdIoCommand {
    unsigned fileSize;
    unsigned fileOffset;
    void* fileHandle;
    unsigned requestID;
    void* buffer;
    unsigned bufferSize;
    nflRequestType requestType;
};
struct nfdWork {
    unsigned fileOffsetA;
    unsigned bytesCompleted;
    unsigned bufferSizeA;
    nfdIoCommand ioCommand;
    nfdIoState ioState;
    int usesBuffer;
};
struct nfdFile {
    unsigned handleSize;
    unsigned handleAlign;
    nfdError (*fnOpen)(void*, const char*, nfdFileFlags, unsigned);
    nfdError (*fnClose)(void*);
    nfdError (*fnStatus)(void*, nfdFileInfo*);
    nfdError (*fnHandle)(void*, void**, unsigned*);
};
struct nfdIo {
    nfdError (*fnExecute)(nfdDriver*, void*, nflRequestType, unsigned, void*, unsigned);
    nfdError (*fnCancel)(void*);
    nfdError (*fnUpdate)();
};
struct nfdBuffer {
    unsigned addrAlignment;
    unsigned sizeAlignment;
    unsigned offsAlignment;
    unsigned size;
    void* addr;
};
struct nfdDriver {
    const char* name;
    nfdInit* init;
    nfdMedia* media;
    nfdFile* file;
    nfdIo* io;
    nfdBuffer* buffer;
    nfdWork work;
};

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void tlMemFree(void* memory);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* message);
extern "C" void txAssertFailed(unsigned char* ignore, const char* message,
                                const char* function, const char* source, int line);
void nfsError(const char* fmt, ...);

#ifdef _WIN32
struct nfd_win32_IoWork {
    OVERLAPPED overlapped;
    nfdDriver* driver;
};
#endif

// ea: 0x0041E4E0
nflRequestParams::nflRequestParams()
    : fileID(NFL_FILE_ID_INVALID), streamID(NFL_STREAM_ID_DEFAULT), callback(nullptr),
      type(NFL_REQUEST_TYPE_INVALID), priority(NFL_PRIORITY_NORMAL), fileOffset(0),
      buffer(nullptr), dataSize(0), timeout(0), userData(nullptr)
{
}
// ea: 0x0041E770
nflStreamParams::nflStreamParams() : streamPriority(NFL_PRIORITY_NORMAL)
{
}

static_assert(sizeof(txSlotEntry) == 12, "txSlotEntry layout changed");
static_assert(sizeof(nflInitParams) == 20, "nflInitParams layout changed");
static_assert(sizeof(nflRequestParams) == 40, "nflRequestParams layout changed");
static_assert(sizeof(nflRequestInfo) == 12, "nflRequestInfo layout changed");
static_assert(sizeof(nflMediaAlignments) == 12, "nflMediaAlignments layout changed");
static_assert(sizeof(nflSimulateError) == 20, "nflSimulateError layout changed");
static_assert(sizeof(nfsRequest) == 68, "nfsRequest layout changed");
static_assert(sizeof(nfsFile) == 292, "nfsFile layout changed");
static_assert(sizeof(nfsStream) == 16, "nfsStream layout changed");
static_assert(sizeof(nfdMedia) == 20, "nfdMedia layout changed");
static_assert(sizeof(nfdInit) == 12, "nfdInit layout changed");
static_assert(sizeof(nfdFile) == 24, "nfdFile layout changed");
static_assert(sizeof(nfdIo) == 12, "nfdIo layout changed");
static_assert(sizeof(nfdBuffer) == 20, "nfdBuffer layout changed");
static_assert(sizeof(nfdIoCommand) == 28, "nfdIoCommand layout changed");
static_assert(sizeof(nfdWork) == 48, "nfdWork layout changed");
static_assert(sizeof(nfdDriver) == 72, "nfdDriver layout changed");

static const unsigned kMaxFiles = 256;
static const unsigned kMaxStreams = 64;
static const unsigned kMaxRequests = 256;
static nfsFile s_files[kMaxFiles];
static void* s_fileHandles[kMaxFiles];
static nfsStream s_streams[kMaxStreams];
static nfsRequest s_requests[kMaxRequests];
static txSlotPool s_filePool;
static txSlotPool s_streamPool;
static txSlotPool s_requestPool;
static nfsRequestCB s_callRequests[kMaxRequests];
static unsigned s_callRequestsCount = 0;
static nflInitParams s_initParams = {64, 16, 256, NFL_BUFFER_MODE_ALL,
                                     NFL_THREAD_MODE_SINGLE};
static nflStreamID s_defaultStreamID = NFL_STREAM_ID_INVALID;
static bool s_initialized = false;
static bool s_started = false;
static void* s_nfsMemory = nullptr;
static unsigned s_nfsMemoryUsed = 0;
static unsigned s_nfsMemoryFree = 0;
static unsigned s_nfsFileHandleSize = sizeof(void*);
static nflSimulateError nfl_simulateError = {251706046, 0, 0, 0, 0};
extern "C" int txSlotPoolInit(txSlotPool*, txSlotEntry*, int, unsigned);
extern "C" txSlot txSlotNew(txSlotPool*);
extern "C" void txSlotFree(txSlotPool*, txSlot);
extern "C" int txSlotIndex(const txSlotPool*, txSlot);
extern "C" txSlot txSlotFirst(const txSlotPool*);
extern "C" txSlot txSlotNext(const txSlotPool*, txSlot);
#ifdef _WIN32
HANDLE nfs_mutex = nullptr;
HANDLE nfs_event = nullptr;
HANDLE nfs_thread = nullptr;
#define s_nfsMutex nfs_mutex
#define s_nfsEvent nfs_event
#define s_nfsThread nfs_thread
static nfdDriver* s_win32IoDriver = nullptr;
DWORD WINAPI nfsUpdateThread(void*);
#endif
extern nfdError nfd_win32_FileOpen(void*, const char*, nfdFileFlags, unsigned);
extern nfdError nfd_win32_FileClose(void*);
extern nfdError nfd_win32_FileStatus(void*, nfdFileInfo*);
extern nfdError nfd_win32_FileHandle(void*, void**, unsigned*);
extern nfdError nfd_win32_IoExecute(nfdDriver*, void*, nflRequestType, unsigned, void*, unsigned);
extern nfdError nfd_win32_IoCancel(void*);
extern nfdError nfd_win32_IoUpdate();
extern nfdError nfd_xbox_MediaBind(nflMediaID, const char*, char*, int);
extern nfdInit nfd_win32_init;
extern nfdFile nfd_win32_file;
extern nfdIo nfd_win32_io;
extern nfdBuffer nfd_win32_buffer;
extern nfdDriver nfd_win32_driver;
#ifdef _WIN32
extern nfd_win32_IoWork nfd_win32_ioWork;
#endif
static nfdDriver* s_nfsDrivers[1] = {&nfd_win32_driver};
static int s_nfsDriversCount = 1;
nfdInit nfd_win32_init = {NFL_BUFFER_MODE_UNALIGNED, nullptr, nullptr};
nfdFile nfd_win32_file = {sizeof(void*), sizeof(void*), nfd_win32_FileOpen,
                          nfd_win32_FileClose, nfd_win32_FileStatus, nfd_win32_FileHandle};
nfdIo nfd_win32_io = {nfd_win32_IoExecute, nfd_win32_IoCancel, nfd_win32_IoUpdate};
nfdBuffer nfd_win32_buffer = {4, 0x1000, 0x800, 0x20000, nullptr};
nfdMedia nfd_xbox_media = {NFL_MEDIA_ID_DISC | NFL_MEDIA_ID_HOST,
                            nfd_xbox_MediaBind, nullptr, nullptr, nullptr};
nfdDriver nfd_win32_driver = {"win32", &nfd_win32_init, &nfd_xbox_media,
                              &nfd_win32_file, &nfd_win32_io, &nfd_win32_buffer, {}};
#ifdef _WIN32
nfd_win32_IoWork nfd_win32_ioWork = {};
#endif
#define s_win32Init nfd_win32_init
#define s_win32Buffer nfd_win32_buffer
#define s_win32Driver nfd_win32_driver

#ifdef _WIN32
static void* OpenHandle(const char* name, nfdFileFlags flags)
{
    DWORD access = 0;
    if (flags & NFD_FILE_FLAGS_READ) access |= GENERIC_READ;
    if (flags & NFD_FILE_FLAGS_WRITE) access |= GENERIC_WRITE;
    DWORD share = flags & (NFD_FILE_FLAGS_READ | NFD_FILE_FLAGS_WRITE);
    DWORD creation = (flags & NFD_FILE_FLAGS_CREATE) ? CREATE_ALWAYS : OPEN_EXISTING;
    const DWORD attributes = (~(static_cast<DWORD>(flags) << 28) & 0x20000000u)
                           | 0x40000000u;
    HANDLE handle = CreateFileA(name, access, share, nullptr, creation,
                                 attributes, nullptr);
    return handle == INVALID_HANDLE_VALUE ? nullptr : handle;
}
static void CloseHandleValue(void* handle) { CloseHandle((HANDLE)handle); }
static unsigned HandleSize(void* handle) { return GetFileSize((HANDLE)handle, nullptr); }
static unsigned ReadAt(void* handle, unsigned offset, void* buffer, unsigned size)
{
    LARGE_INTEGER position; position.QuadPart = offset;
    if (!SetFilePointerEx((HANDLE)handle, position, nullptr, FILE_BEGIN)) return 0;
    DWORD bytes = 0;
    return ReadFile((HANDLE)handle, buffer, size, &bytes, nullptr) ? bytes : 0;
}
static unsigned WriteAt(void* handle, unsigned offset, const void* buffer, unsigned size)
{
    LARGE_INTEGER position; position.QuadPart = offset;
    if (!SetFilePointerEx((HANDLE)handle, position, nullptr, FILE_BEGIN)) return 0;
    DWORD bytes = 0;
    return WriteFile((HANDLE)handle, buffer, size, &bytes, nullptr) ? bytes : 0;
}
#else
static void* OpenHandle(const char* name, nfdFileFlags flags)
{
    int mode = (flags & NFD_FILE_FLAGS_WRITE) ? O_RDWR : O_RDONLY;
    if (flags & NFD_FILE_FLAGS_CREATE) mode |= O_CREAT;
    int fd = open(name, mode, 0666);
    return fd < 0 ? nullptr : (void*)(intptr_t)fd;
}
static void CloseHandleValue(void* handle) { close((int)(intptr_t)handle); }
static unsigned HandleSize(void* handle)
{
    struct stat info;
    return fstat((int)(intptr_t)handle, &info) == 0 ? (unsigned)info.st_size : 0;
}
static unsigned ReadAt(void* handle, unsigned offset, void* buffer, unsigned size)
{
    if (lseek((int)(intptr_t)handle, (off_t)offset, SEEK_SET) < 0) return 0;
    ssize_t bytes = read((int)(intptr_t)handle, buffer, size);
    return bytes > 0 ? (unsigned)bytes : 0;
}
static unsigned WriteAt(void* handle, unsigned offset, const void* buffer, unsigned size)
{
    if (lseek((int)(intptr_t)handle, (off_t)offset, SEEK_SET) < 0) return 0;
    ssize_t bytes = write((int)(intptr_t)handle, buffer, size);
    return bytes > 0 ? (unsigned)bytes : 0;
}
#endif

static unsigned Now()
{
#ifdef _WIN32
    return GetTickCount();
#else
    return 0;
#endif
}
static int FileIndex(nflFileID id)
{
    return s_filePool.slots == nullptr ? -1 : txSlotIndex(&s_filePool, (txSlot)id);
}
static int StreamIndex(nflStreamID id)
{
    return s_streamPool.slots == nullptr ? -1 : txSlotIndex(&s_streamPool, (txSlot)id);
}
static int RequestIndex(nflRequestID id)
{
    return s_requestPool.slots == nullptr ? -1 : txSlotIndex(&s_requestPool, (txSlot)id);
}
static bool ValidFile(nflFileID id) { return FileIndex(id) >= 0; }
static bool ValidStream(nflStreamID id) { return StreamIndex(id) >= 0; }
static bool ValidRequest(nflRequestID id) { return RequestIndex(id) >= 0; }
template <typename T> static void ClearSlotObject(T* object)
{
    txSlotEntry links = object->slotEntry;
    std::memset(object, 0, sizeof(*object));
    object->slotEntry = links;
}
static nflFileID AllocateFile()
{
    return s_filePool.slots == nullptr ? NFL_FILE_ID_INVALID
                                       : (nflFileID)txSlotNew(&s_filePool);
}
static nflStreamID AllocateStream()
{
    return s_streamPool.slots == nullptr ? NFL_STREAM_ID_INVALID
                                         : (nflStreamID)txSlotNew(&s_streamPool);
}
static nflRequestID AllocateRequest()
{
    return s_requestPool.slots == nullptr ? NFL_REQUEST_ID_INVALID
                                          : (nflRequestID)txSlotNew(&s_requestPool);
}

// ea: 0x0041E830
nfsFile* nfsGetFile(nflFileID fileID)
{
    const int index = FileIndex(fileID);
    return index >= 0 ? &s_files[index] : nullptr;
}
// ea: 0x0041E860
nflFileID nfsGetNativeFileID(nflFileID fileID)
{
    nfsFile* file = nfsGetFile(fileID);
    if (file == nullptr) return NFL_FILE_ID_INVALID;
    return file->fileType == NFS_FILE_TYPE_SUBFILE
               ? (nflFileID)file->as.subfile.parent
               : (nflFileID)fileID;
}
// ea: 0x0041E950
nfsFile* nfsGetNativeFile(nflFileID fileID)
{
    nflFileID native = nfsGetNativeFileID(fileID);
    return native == NFL_FILE_ID_INVALID ? nullptr : nfsGetFile(native);
}
// ea: 0x0041E990
nfdDriver* nfsGetFileDriver(nflFileID fileID)
{
    nfsFile* file = nfsGetNativeFile(fileID);
    return file != nullptr ? file->as.native.driver : nullptr;
}
// ea: 0x0041EA90
nfsStream* nfsGetStream(nflStreamID streamID)
{
    if (streamID == NFL_STREAM_ID_DEFAULT) streamID = s_defaultStreamID;
    const int index = StreamIndex(streamID);
    return index >= 0 ? &s_streams[index] : nullptr;
}
// ea: 0x0041EAD0
nfsRequest* nfsGetRequest(nflRequestID requestID)
{
    const int index = RequestIndex(requestID);
    return index >= 0 ? &s_requests[index] : nullptr;
}
// ea: 0x0041EA50
void* nfsGetFileHandle(nflFileID fileID)
{
    nflFileID native = nfsGetNativeFileID(fileID);
    if (native == NFL_FILE_ID_INVALID) return nullptr;
    const int index = FileIndex(native);
    return index >= 0 ? &s_fileHandles[index] : nullptr;
}
// ea: 0x0041EB00
nflRequestState nfsExportRequestState(nfsRequestState state)
{
    switch (state) {
    case NFS_REQUEST_STATE_WAITING:
    case NFS_REQUEST_STATE_WORKING: return NFL_REQUEST_STATE_ACTIVE;
    case NFS_REQUEST_STATE_WORKDONE: return NFL_REQUEST_STATE_COMPLETED;
    case NFS_REQUEST_STATE_CANCELING:
    case NFS_REQUEST_STATE_CANCELED: return NFL_REQUEST_STATE_CANCELED;
    case NFS_REQUEST_STATE_TIMEOUT: return NFL_REQUEST_STATE_TIMEOUT;
    case NFS_REQUEST_STATE_IO_ERROR:
    case NFS_REQUEST_STATE_IO_ERROR_WAITING: return NFL_REQUEST_STATE_ERROR;
    default: return NFL_REQUEST_STATE_INVALID;
    }
}
// ea: 0x0041EB60
void* nfsPreAllocate(unsigned size, unsigned align)
{
    if (size == 0)
        txAssertFailed(nullptr, "size>=1", "nfsPreAllocate",
                       "c:/cod/code/tl/nfl/src/nfl_system.cpp", 211);
    if (align == 0)
        txAssertFailed(nullptr, "align>=1", "nfsPreAllocate",
                       "c:/cod/code/tl/nfl/src/nfl_system.cpp", 212);
    const unsigned allocationSize = size + align - 1;
    if (s_nfsMemory != nullptr) {
        if (allocationSize > s_nfsMemoryFree) {
            nfsError("nfsPreAllocate: Out of %d bytes, used %d, free %d, total %d",
                     allocationSize, s_nfsMemoryUsed, s_nfsMemoryFree,
                     s_nfsMemoryUsed + s_nfsMemoryFree);
            return nullptr;
        }
        s_nfsMemoryFree -= allocationSize;
    }
    const uintptr_t base = reinterpret_cast<uintptr_t>(s_nfsMemory) + s_nfsMemoryUsed;
    const uintptr_t aligned = static_cast<uintptr_t>(align)
                            * ((base + align - 1) / align);
    s_nfsMemoryUsed += allocationSize;
    return s_nfsMemory != nullptr ? reinterpret_cast<void*>(aligned) : nullptr;
}
// ea: 0x0041EC30
unsigned nfsPreAllocateWorkSpace(void* work)
{
    s_nfsMemory = work;
    s_nfsMemoryFree = work != nullptr ? s_nfsMemoryUsed : 0;
    s_nfsMemoryUsed = 0;
    void* driverBuffers[32] = {};
    unsigned driverBufferFailed = 0;
    unsigned handleSize = 1;
    unsigned handleAlign = 1;
    const int driverCount = s_nfsDriversCount < 32 ? s_nfsDriversCount : 32;
    for (int index = 0; index < driverCount; ++index) {
        nfdDriver* driver = s_nfsDrivers[index];
        if (driver == nullptr || driver->file == nullptr || driver->init == nullptr)
            continue;
        if (handleSize < driver->file->handleSize) handleSize = driver->file->handleSize;
        if (handleAlign < driver->file->handleAlign) handleAlign = driver->file->handleAlign;
        const nflBufferMode mode = driver->init->bufferMode;
        if ((mode == NFL_BUFFER_MODE_UNALIGNED || mode == NFL_BUFFER_MODE_ALL
             || mode == NFL_BUFFER_MODE_VM)
            && driver->buffer != nullptr && driver->buffer->addr == nullptr) {
            unsigned alignment = driver->buffer->addrAlignment;
            if (alignment < driver->buffer->sizeAlignment) alignment = driver->buffer->sizeAlignment;
            driverBuffers[index] = nfsPreAllocate(driver->buffer->size, alignment);
            driverBufferFailed |= driverBuffers[index] == nullptr ? (1u << index) : 0u;
        }
    }
    const unsigned alignedHandleSize = handleAlign
        * ((handleAlign + handleSize - 1) / handleAlign);
    nfsPreAllocate(sizeof(nfsRequest) * s_initParams.maxRequests, 0x40);
    nfsPreAllocate(sizeof(nfsRequestCB) * s_initParams.maxRequests, 4);
    nfsPreAllocate(sizeof(nfsStream) * s_initParams.maxStreams, 0x40);
    nfsPreAllocate(sizeof(nfsFile) * s_initParams.maxFiles, 0x40);
    nfsPreAllocate(alignedHandleSize * s_initParams.maxFiles, handleAlign);
    s_nfsFileHandleSize = alignedHandleSize;
    if (s_nfsMemory != nullptr && driverBufferFailed == 0) {
        for (int index = 0; index < driverCount; ++index) {
            nfdDriver* driver = s_nfsDrivers[index];
            if (driver != nullptr && driver->buffer != nullptr && driver->init != nullptr
                && driver->buffer->addr == nullptr
                && (driver->init->bufferMode == NFL_BUFFER_MODE_UNALIGNED
                    || driver->init->bufferMode == NFL_BUFFER_MODE_ALL
                    || driver->init->bufferMode == NFL_BUFFER_MODE_VM))
                driver->buffer->addr = driverBuffers[index];
        }
    }
    return s_nfsMemoryUsed;
}
// ea: 0x0041E7C0
int nfsGetMediaDriverIndex(nflMediaID media)
{
    for (int index = 0; index < s_nfsDriversCount; ++index) {
        nfdDriver* driver = s_nfsDrivers[index];
        if (driver != nullptr && driver->media != nullptr
            && (static_cast<unsigned>(media) & driver->media->registry) != 0)
            return index;
    }
    return -1;
}
// ea: 0x0041E800
nfdDriver* nfsGetMediaDriver(nflMediaID media)
{
    const int index = nfsGetMediaDriverIndex(media);
    return index < 0 ? nullptr : s_nfsDrivers[index];
}

// ea: 0x0041E780
#ifdef _WIN32
DWORD nfsLock()
#else
unsigned long nfsLock()
#endif
{
#ifdef _WIN32
    DWORD result;
    if (s_initParams.threadMode == NFL_THREAD_MODE_MULTI)
        return WaitForSingleObject(s_nfsMutex, INFINITE);
    return result;
#else
    unsigned long result;
    return result;
#endif
}
// ea: 0x0041E7A0
#ifdef _WIN32
BOOL nfsUnlock()
#else
int nfsUnlock()
#endif
{
#ifdef _WIN32
    BOOL result;
    if (s_initParams.threadMode == NFL_THREAD_MODE_MULTI)
        return ReleaseMutex(s_nfsMutex);
    return result;
#else
    int result;
    return result;
#endif
}

enum nflFileID nflOpenFile(nflMediaID media, const char* name);
void nflCloseFile(nflFileID fileID);
extern nfdError nfdIoExecute(nfdDriver*, const nfdIoCommand*);
extern void nfdIoComplete(nfdDriver*, unsigned, int);
extern "C" void txPrintv(const char* channel, int level, const char* fmt, char* list);
extern "C" unsigned long long txTime();

void nfsError(const char* fmt, ...)
{
    va_list args; va_start(args, fmt); txPrintv("NFL", 0, fmt, reinterpret_cast<char*>(args)); va_end(args);
}
void nfsWarning(const char* fmt, ...)
{
    va_list args; va_start(args, fmt); txPrintv("NFL", 1, fmt, reinterpret_cast<char*>(args)); va_end(args);
}
void nfsMessage(const char* fmt, ...)
{
    va_list args; va_start(args, fmt); txPrintv("NFL", 2, fmt, reinterpret_cast<char*>(args)); va_end(args);
}

const char* nfsRequestStateText(nfsRequestState state)
{
    switch (state) {
    case NFS_REQUEST_STATE_WAITING: return "NFS_REQUEST_STATE_WAITING";
    case NFS_REQUEST_STATE_WORKING: return "NFS_REQUEST_STATE_WORKING";
    case NFS_REQUEST_STATE_WORKDONE: return "NFS_REQUEST_STATE_WORKDONE";
    case NFS_REQUEST_STATE_CANCELING: return "NFS_REQUEST_STATE_CANCELING";
    case NFS_REQUEST_STATE_CANCELED: return "NFS_REQUEST_STATE_CANCELED";
    case NFS_REQUEST_STATE_TIMEOUT: return "NFS_REQUEST_STATE_TIMEOUT";
    case NFS_REQUEST_STATE_IO_ERROR: return "NFS_REQUEST_STATE_IO_ERROR";
    case NFS_REQUEST_STATE_IO_ERROR_WAITING: return "NFS_REQUEST_STATE_IO_ERROR_WAITING";
    default: return "NFS_REQUEST_STATE_???";
    }
}
const char* nfsFileTypeText(nfsFileType type)
{
    if (type == NFS_FILE_TYPE_NATIVE) return "NFS_FILE_TYPE_NATIVE";
    if (type == NFS_FILE_TYPE_SUBFILE) return "NFS_FILE_TYPE_SUBFILE";
    return "NFS_FILE_TYPE_???";
}
const char* nfsRequestStateText(nflRequestState state)
{
    switch (state) {
    case NFL_REQUEST_STATE_INVALID: return "NFL_REQUEST_STATE_INVALID";
    case NFL_REQUEST_STATE_COMPLETED: return "NFL_REQUEST_STATE_COMPLETED";
    case NFL_REQUEST_STATE_CANCELED: return "NFL_REQUEST_STATE_CANCELED";
    case NFL_REQUEST_STATE_TIMEOUT: return "NFL_REQUEST_STATE_TIMEOUT";
    case NFL_REQUEST_STATE_ERROR: return "NFL_REQUEST_STATE_ERROR";
    case NFL_REQUEST_STATE_ACTIVE: return "NFL_REQUEST_STATE_ACTIVE";
    default: return "NFL_REQUEST_STATE_???";
    }
}
const char* nfsMediaIDText(nflMediaID media)
{
    static char text[32]; text[0] = 0;
    if (media & NFL_MEDIA_ID_DISC) strcat_s(text, sizeof(text), "DISC:");
    if (media & NFL_MEDIA_ID_HOST) strcat_s(text, sizeof(text), "HOST:");
    if (media & NFL_MEDIA_ID_LINK) strcat_s(text, sizeof(text), "LINK:");
    if (static_cast<unsigned>(media) & ~((unsigned)NFL_MEDIA_ID_DISC
                                        | (unsigned)NFL_MEDIA_ID_HOST
                                        | (unsigned)NFL_MEDIA_ID_LINK))
        strcat_s(text, sizeof(text), "UNKN:");
    return text;
}
const char* nfsBufferModeText(nflBufferMode mode)
{
    switch (mode) {
    case NFL_BUFFER_MODE_NOBUFFER: return "NFL_BUFFER_MODE_NOBUFFER";
    case NFL_BUFFER_MODE_UNALIGNED: return "NFL_BUFFER_MODE_UNALIGNED";
    case NFL_BUFFER_MODE_ALL: return "NFL_BUFFER_MODE_ALL";
    case NFL_BUFFER_MODE_VM: return "NFL_BUFFER_MODE_VM";
    default: return "NFL_BUFFER_MODE_???";
    }
}
const char* nfdErrorText(nfdError error)
{
    switch (error) {
    case NFD_ERROR_NOERROR: return "NFD_ERROR_NOERROR";
    case NFD_ERROR_FAILURE: return "NFD_ERROR_FAILURE";
    case NFD_ERROR_EOF:
    case NFD_ERROR_UNALIGNED_ACCESS: return "NFD_ERROR_UNALIGNED_ACCESS";
    case NFD_ERROR_INVALID_MEDIA: return "NFD_ERROR_INVALID_MEDIA";
    case NFD_ERROR_INVALID_STATE: return "NFD_ERROR_INVALID_STATE";
    case NFD_ERROR_INVALID_FLAGS: return "NFD_ERROR_INVALID_FLAGS";
    case NFD_ERROR_INVALID_FILENAME: return "NFD_ERROR_INVALID_FILENAME";
    case NFD_ERROR_INVALID_ARGUMENTS: return "NFD_ERROR_INVALID_ARGUMENTS";
    default: return "NFD_ERROR_???";
    }
}
const char* nfdFileFlagsText(nfdFileFlags flags)
{
    static char text[64]; strcpy_s(text, sizeof(text), "NFD_FILE_FLAGS");
    if (flags & NFD_FILE_FLAGS_READ) strcat_s(text, sizeof(text), "_READ");
    if (flags & NFD_FILE_FLAGS_WRITE) strcat_s(text, sizeof(text), "_WRITE");
    if (flags & NFD_FILE_FLAGS_CREATE) strcat_s(text, sizeof(text), "_CREATE");
    if (flags & ~(NFD_FILE_FLAGS_READ | NFD_FILE_FLAGS_WRITE | NFD_FILE_FLAGS_CREATE))
        strcat_s(text, sizeof(text), "_???");
    return text;
}
const char* nfdMediaStateText(nfdMediaState state)
{
    switch (state) {
    case NFD_MEDIA_STATE_LOADING: return "NFD_MEDIA_STATE_LOADING";
    case NFD_MEDIA_STATE_LOADED: return "NFD_MEDIA_STATE_LOADED";
    case NFD_MEDIA_STATE_LOADFAILED: return "NFD_MEDIA_STATE_LOADFAILED";
    case NFD_MEDIA_STATE_UNLOADING: return "NFD_MEDIA_STATE_EJECTING";
    case NFD_MEDIA_STATE_UNLOADED: return "NFD_MEDIA_STATE_EJECTED";
    default: return "NFD_MEDIA_STATE_???";
    }
}
const char* nfdIoStateText(nfdIoState state) { return state == NFD_IO_STATE_IDLE ? "NFD_IO_STATE_IDLE" : "NFD_IO_STATE_???"; }

// ea: 0x0041EE20
unsigned nflInit(const nflInitParams* params)
{
    if (params != nullptr) s_initParams = *params;
    for (int index = 0; index < s_nfsDriversCount; ++index) {
        nfdDriver* driver = s_nfsDrivers[index];
        if (driver == nullptr || driver->init == nullptr) continue;
        if (s_initParams.bufferMode != (nflBufferMode)-1)
            driver->init->bufferMode = s_initParams.bufferMode;
        if (driver->init->fnInit != nullptr) {
            nfsMessage("nflInit: Initializing driver #%d: %s", index,
                       driver->name != nullptr ? driver->name : "");
            driver->init->fnInit(driver);
        }
    }
    if (s_initParams.maxFiles == 0) s_initParams.maxFiles = 64;
    if (s_initParams.maxStreams == 0) s_initParams.maxStreams = 16;
    if (s_initParams.maxRequests == 0) s_initParams.maxRequests = 256;
    if (s_initParams.maxFiles > kMaxFiles) s_initParams.maxFiles = kMaxFiles;
    if (s_initParams.maxStreams > kMaxStreams) s_initParams.maxStreams = kMaxStreams;
    if (s_initParams.maxRequests > kMaxRequests) s_initParams.maxRequests = kMaxRequests;
    memset(s_fileHandles, 0, sizeof(s_fileHandles));
    s_filePool = {};
    s_streamPool = {};
    s_requestPool = {};
    s_defaultStreamID = NFL_STREAM_ID_INVALID;
#ifdef _WIN32
    if (s_nfsMutex != nullptr) { CloseHandle(s_nfsMutex); s_nfsMutex = nullptr; }
#endif
    s_initialized = true; s_started = false;
    return nfsPreAllocateWorkSpace(nullptr);
}
// ea: 0x00420860
void nflStart(void* work)
{
    if (work == nullptr) {
        nfsError("nflStart: work is required");
        return;
    }
    if (s_started) return;
    nfsPreAllocateWorkSpace(work);
    txSlotPoolInit(&s_requestPool, &s_requests[0].slotEntry,
                   (int)s_initParams.maxRequests, sizeof(nfsRequest));
    txSlotPoolInit(&s_streamPool, &s_streams[0].slotEntry,
                   (int)s_initParams.maxStreams, sizeof(nfsStream));
    txSlotPoolInit(&s_filePool, &s_files[0].slotEntry,
                   (int)s_initParams.maxFiles, sizeof(nfsFile));
    s_defaultStreamID = AllocateStream();
    nfsStream* defaultStream = nfsGetStream(s_defaultStreamID);
    if (defaultStream != nullptr) defaultStream->priority = NFL_PRIORITY_NORMAL;
#ifdef _WIN32
    if (s_initParams.threadMode == NFL_THREAD_MODE_MULTI) {
        if (s_nfsMutex == nullptr) s_nfsMutex = CreateMutexA(nullptr, FALSE, nullptr);
        if (s_nfsEvent == nullptr) s_nfsEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    }
#endif
    s_started = true;
#ifdef _WIN32
    if (s_initParams.threadMode == NFL_THREAD_MODE_MULTI)
        s_nfsThread = CreateThread(nullptr, 0x2000, nfsUpdateThread, nullptr, 0, nullptr);
#endif
}
// ea: 0x0041EF00
void nflShutdown()
{
#ifdef _WIN32
    s_started = false;
    if (s_nfsEvent != nullptr) SetEvent(s_nfsEvent);
    if (s_nfsThread != nullptr) {
        WaitForSingleObject(s_nfsThread, INFINITE);
        CloseHandle(s_nfsThread);
        s_nfsThread = nullptr;
    }
    if (s_nfsEvent != nullptr) { CloseHandle(s_nfsEvent); s_nfsEvent = nullptr; }
#endif
    for (int index = 0; index < s_nfsDriversCount; ++index) {
        nfdDriver* driver = s_nfsDrivers[index];
        if (driver != nullptr && driver->init != nullptr && driver->init->fnDone != nullptr)
            driver->init->fnDone(driver);
    }
    if (s_filePool.slots != nullptr) {
        for (txSlot fileID = txSlotFirst(&s_filePool); fileID != TX_SLOT_INVALID;) {
            const txSlot next = txSlotNext(&s_filePool, fileID);
            const int index = FileIndex((nflFileID)fileID);
            if (index >= 0) {
                if (s_files[index].fileType == NFS_FILE_TYPE_NATIVE
                    && s_fileHandles[index] != nullptr)
                    CloseHandleValue(s_fileHandles[index]);
                s_fileHandles[index] = nullptr;
            }
            txSlotFree(&s_filePool, fileID);
            fileID = next;
        }
    }
    s_streamPool = {};
    s_requestPool = {};
    s_filePool = {};
#ifdef _WIN32
    if (s_nfsMutex != nullptr) { CloseHandle(s_nfsMutex); s_nfsMutex = nullptr; }
#endif
    s_defaultStreamID = NFL_STREAM_ID_INVALID; s_started = false;
    s_initialized = false;
}

// ea: 0x0041F4E0
unsigned nflGetFileSize(nflFileID fileID)
{
    nfsFile* file = nfsGetFile(fileID);
    return file != nullptr ? file->size : (unsigned)-1;
}
// ea: 0x0041E540
unsigned nflGetFileSize(nflMediaID media, const char* name)
{
    nflFileID file = nflOpenFile(media, name);
    unsigned size = nflGetFileSize(file);
    nflCloseFile(file);
    return size;
}
unsigned nflGetFileSize2(nflMediaID media, const char* name)
{
    nflFileID file = nflOpenFile(media, name); unsigned size = nflGetFileSize(file);
    nflCloseFile(file); return size;
}
// ea: 0x0041E9D0
nflMediaID nflGetFileMedia(nflFileID fileID)
{
    nfsFile* file = nfsGetNativeFile(fileID);
    return file != nullptr ? file->as.native.mediaID : (nflMediaID)-1;
}
// ea: 0x0041EA10
const char* nflGetFileName(nflFileID fileID)
{
    nfsFile* file = nfsGetNativeFile(fileID);
    return file != nullptr ? file->as.native.filename : nullptr;
}

// ea: 0x0041F0D0
nflFileID nfsOpenFile(nflMediaID media, const char* name,
                      nfdFileFlags flags, unsigned* fileSize)
{
    if (!s_initialized || name == nullptr) return NFL_FILE_ID_INVALID;
    for (unsigned bit = 1; bit != 0 && bit <= static_cast<unsigned>(NFL_MEDIA_ID_ALL); bit <<= 1) {
        const nflMediaID selectedMedia = static_cast<nflMediaID>(static_cast<unsigned>(media) & bit);
        if (selectedMedia == 0) continue;
        nfdDriver* driver = nfsGetMediaDriver(selectedMedia);
        if (driver == nullptr || driver->media == nullptr || driver->media->fnBind == nullptr)
            continue;
        char fullName[256] = {};
        if (driver->media->fnBind(selectedMedia, name, fullName, sizeof(fullName))
            != NFD_ERROR_NOERROR)
            continue;
        nfsLock();
        nflFileID id = AllocateFile();
        if (id == NFL_FILE_ID_INVALID) {
            nfsUnlock();
            break;
        }
        const int index = FileIndex(id);
        if (index < 0) {
            nfsUnlock();
            return NFL_FILE_ID_INVALID;
        }
        nfsFile& file = s_files[index];
        ClearSlotObject(&file);
        file.size = 0;
        file.fileType = NFS_FILE_TYPE_NATIVE;
        file.as.native.mediaID = selectedMedia;
        file.as.native.driver = driver;
        file.as.native.flags = flags;
        s_fileHandles[index] = nullptr;
        strncpy_s(file.as.native.filename, sizeof(file.as.native.filename), fullName, _TRUNCATE);
        if (driver->file == nullptr || driver->file->fnOpen == nullptr
            || driver->file->fnStatus == nullptr) {
            txSlotFree(&s_filePool, (txSlot)id);
            nfsUnlock();
            return NFL_FILE_ID_INVALID;
        }
        const unsigned createdFileSize = fileSize != nullptr ? *fileSize : 0;
        void* handleStorage = nfsGetFileHandle(id);
        const nfdError openResult = driver->file->fnOpen(handleStorage, fullName,
                                                         flags, createdFileSize);
        nfdFileInfo info = {};
        const nfdError statusResult = openResult == NFD_ERROR_NOERROR
            ? driver->file->fnStatus(handleStorage, &info) : NFD_ERROR_FAILURE;
        if (openResult == NFD_ERROR_NOERROR && statusResult == NFD_ERROR_NOERROR) {
            file.size = info.size;
            if (fileSize != nullptr) *fileSize = info.size;
            nfsUnlock();
            return id;
        }
        if (openResult == NFD_ERROR_NOERROR && driver->file->fnClose != nullptr)
            driver->file->fnClose(handleStorage);
        s_fileHandles[index] = nullptr;
        txSlotFree(&s_filePool, (txSlot)id);
        nfsUnlock();
    }
    nfsWarning("nfsOpenFile: unable to bind/open %s", name);
    return NFL_FILE_ID_INVALID;
}
// ea: 0x0041F520
nflFileID nflOpenFile(nflMediaID media, const char* name)
{
    return nfsOpenFile(media, name, NFD_FILE_FLAGS_READ, nullptr);
}
// ea: 0x0041F540
nflFileID nflOpenFileEx(nflMediaID media, const char* name, unsigned* fileSize)
{
    return nfsOpenFile(media, name, NFD_FILE_FLAGS_READ, fileSize);
}
// ea: 0x0041F560
nflFileID nflCreateFile(nflMediaID media, const char* name, unsigned fileSize)
{
    return nfsOpenFile(media, name,
                       (nfdFileFlags)(NFD_FILE_FLAGS_READ | NFD_FILE_FLAGS_WRITE
                                      | NFD_FILE_FLAGS_CREATE), &fileSize);
}
void nflCancelFileRequests(nflFileID fileID);
// ea: 0x00420740
void nflCloseFile(nflFileID fileID)
{
    for (;;) {
        const int index = FileIndex(fileID);
        if (index < 0) return;
        nfsFile& file = s_files[index];
        nflCancelFileRequests(fileID);
        if (file.fileType == NFS_FILE_TYPE_NATIVE) {
            if (file.as.native.childCount == 0) {
                if (file.as.native.driver != nullptr && file.as.native.driver->file != nullptr
                    && file.as.native.driver->file->fnClose != nullptr)
                    file.as.native.driver->file->fnClose(nfsGetFileHandle(fileID));
                s_fileHandles[index] = nullptr;
                txSlotFree(&s_filePool, (txSlot)fileID);
            }
            return;
        }
        if (file.fileType != NFS_FILE_TYPE_SUBFILE) {
            nfsError("nflCloseFile: Invalid filetype");
            return;
        }
        const nflFileID parentID = (nflFileID)file.as.subfile.parent;
        nfsFile* parent = nfsGetFile(parentID);
        if (parent == nullptr) return;
        --parent->as.native.childCount;
        fileID = parentID;
    }
}
// ea: 0x0041E510
unsigned nflFileExists(nflMediaID media, const char* name)
{
    nflFileID id = nflOpenFile(media, name); bool exists = id != NFL_FILE_ID_INVALID;
    nflCloseFile(id); return exists ? 1u : 0u;
}

// ea: 0x0041F310
nflFileID nflOpenSubFile(nflFileID parentID, unsigned parentOffset,
                         unsigned fileSize, unsigned chunkSize, unsigned strideSize)
{
    if (fileSize == 0
        || (strideSize != 0 && (chunkSize == 0 || strideSize < chunkSize))
        || (strideSize == 0 && chunkSize != 0)) return NFL_FILE_ID_INVALID;
    if (!ValidFile(parentID)) return NFL_FILE_ID_INVALID;
    nfsFile* parent = nfsGetFile(parentID);
    if (parent == nullptr) return NFL_FILE_ID_INVALID;
    nfsFile* nativeParent = parent;
    unsigned offset = parentOffset;
    if (parent->fileType == NFS_FILE_TYPE_SUBFILE) {
        if (strideSize != 0 || chunkSize != 0) return NFL_FILE_ID_INVALID;
        offset += parent->as.subfile.offset;
        nativeParent = nfsGetNativeFile(parentID);
        if (nativeParent == nullptr) return NFL_FILE_ID_INVALID;
    }
    if (offset > nativeParent->size) return NFL_FILE_ID_INVALID;
    if (fileSize > nativeParent->size - offset) fileSize = nativeParent->size - offset;
    if (fileSize == 0) return NFL_FILE_ID_INVALID;
    nflFileID id = AllocateFile(); if (id == NFL_FILE_ID_INVALID) return id;
    const int index = FileIndex(id);
    if (index < 0) return NFL_FILE_ID_INVALID;
    nfsFile& file = s_files[index]; ClearSlotObject(&file);
    file.size = fileSize; file.fileType = NFS_FILE_TYPE_SUBFILE;
    file.as.subfile.offset = offset; file.as.subfile.parent = nfsGetNativeFileID(parentID);
    file.as.subfile.chunkSize = chunkSize; file.as.subfile.strideSize = strideSize;
    ++nativeParent->as.native.childCount; return id;
}

// ea: 0x0041F760
nflRequestID nflAddRequest(const nflRequestParams* params)
{
    if (params == nullptr || !ValidFile(params->fileID)) return NFL_REQUEST_ID_INVALID;
    nfsLock();
    nflStreamID stream = params->streamID == NFL_STREAM_ID_DEFAULT ? s_defaultStreamID : (nflStreamID)params->streamID;
    if (!ValidStream(stream)) { nfsUnlock(); return NFL_REQUEST_ID_INVALID; }
    nflRequestID id = AllocateRequest();
    if (id == NFL_REQUEST_ID_INVALID) { nfsUnlock(); return id; }
    const int index = RequestIndex(id);
    if (index < 0) { nfsUnlock(); return NFL_REQUEST_ID_INVALID; }
    nfsRequest& request = s_requests[index]; ClearSlotObject(&request);
    request.fileOffset = params->fileOffset; request.type = params->type;
    request.streamID = params->streamID; request.fileID = params->fileID;
    request.callback = params->callback; request.callbackData = params->userData;
    request.queueTime = Now(); request.expirationTime = params->timeout
        ? request.queueTime + params->timeout : 0x7FFFFFFFu;
    nfsStream* requestStream = nfsGetStream(stream);
    request.priority = params->priority | ((requestStream != nullptr ? requestStream->priority : NFL_PRIORITY_NORMAL) << 8);
    request.buffer = (char*)params->buffer; request.bufferSize = params->dataSize;
    request.state = NFS_REQUEST_STATE_WAITING; request.lastWorkingTime = request.queueTime;
    nfsUnlock();
#ifdef _WIN32
    if (s_initParams.threadMode == NFL_THREAD_MODE_MULTI && s_nfsEvent != nullptr)
        SetEvent(s_nfsEvent);
#endif
    return id;
}
int nfsUpdate();
void nfsAddRequestToCallList(nfsRequest* request, int shouldDie);
// ea: 0x00420190
void nflUpdate()
{
    if (!s_started) return;
    s_callRequestsCount = 0;
    for (txSlot requestSlot = txSlotFirst(&s_requestPool); requestSlot != TX_SLOT_INVALID;) {
        const txSlot nextSlot = txSlotNext(&s_requestPool, requestSlot);
        nfsRequest* requestPtr = nfsGetRequest((nflRequestID)requestSlot);
        if (requestPtr == nullptr) { requestSlot = nextSlot; continue; }
        nfsRequest& request = *requestPtr;
        switch (request.state) {
        case NFS_REQUEST_STATE_WAITING:
            (void)Now();
            break;
        case NFS_REQUEST_STATE_WORKDONE:
        case NFS_REQUEST_STATE_CANCELED:
            nfsAddRequestToCallList(&request, 1);
            break;
        case NFS_REQUEST_STATE_IO_ERROR:
            request.state = NFS_REQUEST_STATE_IO_ERROR_WAITING;
            nfsAddRequestToCallList(&request, 0);
            break;
        default:
            break;
        }
        requestSlot = nextSlot;
    }
    if (s_callRequestsCount != 0) {
        nfsUnlock();
        for (unsigned i = 0; i < s_callRequestsCount; ++i) {
            if (s_callRequests[i].callback != nullptr)
                s_callRequests[i].callback(s_callRequests[i].state,
                                           s_callRequests[i].requestID,
                                           s_callRequests[i].callbackData);
        }
        nfsLock();
    }
    for (unsigned i = 0; i < s_callRequestsCount; ++i) {
        if (s_callRequests[i].shouldDie)
            txSlotFree(&s_requestPool, (txSlot)s_callRequests[i].requestID);
    }
    s_callRequestsCount = 0;
    nfsUnlock();
    if (s_initParams.threadMode == NFL_THREAD_MODE_SINGLE)
        nfsUpdate();
}

nflRequestID nfsScheduleRequest(nfdDriver* driver)
{
    nflRequestID best = NFL_REQUEST_ID_INVALID;
    unsigned bestPriority = 0;
    unsigned bestTime = 0xFFFFFFFFu;
    for (txSlot requestSlot = txSlotFirst(&s_requestPool); requestSlot != TX_SLOT_INVALID;
         requestSlot = txSlotNext(&s_requestPool, requestSlot)) {
        nfsRequest* request = nfsGetRequest((nflRequestID)requestSlot);
        if (request == nullptr || request->state != NFS_REQUEST_STATE_WAITING) continue;
        if (driver != nullptr && nfsGetFileDriver((nflFileID)request->fileID) != driver) continue;
        const unsigned priority = request->priority;
        if (best == NFL_REQUEST_ID_INVALID || priority > bestPriority
            || (priority == bestPriority && request->lastWorkingTime < bestTime)) {
            best = (nflRequestID)requestSlot;
            bestPriority = priority;
            bestTime = request->lastWorkingTime;
        }
    }
    return best;
}
nfdError nfsExecuteRequest(nfdDriver* driver, nflRequestID requestID)
{
    if (driver == nullptr) return NFD_ERROR_INVALID_ARGUMENTS;
    if (driver->work.ioState != NFD_IO_STATE_IDLE) {
        nfsWarning("nfdIoExecute: called a busy state %s", nfdIoStateText(driver->work.ioState));
        return NFD_ERROR_INVALID_STATE;
    }
    if (!ValidRequest(requestID)) return NFD_ERROR_INVALID_ARGUMENTS;
    nfsRequest* requestPtr = nfsGetRequest(requestID);
    if (requestPtr == nullptr) return NFD_ERROR_INVALID_ARGUMENTS;
    nfsRequest& request = *requestPtr;
    nfsFile* file = nfsGetFile((nflFileID)request.fileID);
    if (file == nullptr || request.bytesCompleted > request.bufferSize)
        return NFD_ERROR_INVALID_ARGUMENTS;

    const unsigned bytesCompleted = request.bytesCompleted;
    nfdIoCommand command = {};
    command.requestType = request.type;
    command.buffer = request.buffer + bytesCompleted;
    command.bufferSize = request.bufferSize - bytesCompleted;
    command.fileOffset = request.fileOffset + bytesCompleted;
    command.requestID = requestID;
    const nflFileID nativeID = nfsGetNativeFileID((nflFileID)request.fileID);
    const int nativeIndex = FileIndex(nativeID);
    command.fileHandle = nativeIndex >= 0 ? &s_fileHandles[nativeIndex] : nullptr;
    if (command.fileHandle == nullptr) return NFD_ERROR_INVALID_ARGUMENTS;
    command.fileSize = file->size;

    if (file->fileType == NFS_FILE_TYPE_SUBFILE) {
        const nflFileID parentID = (nflFileID)file->as.subfile.parent;
        nfsFile* parent = nfsGetFile(parentID);
        if (parent == nullptr || parent->as.native.driver != driver)
            return NFD_ERROR_INVALID_ARGUMENTS;
        unsigned chunkSize = file->as.subfile.chunkSize;
        unsigned strideSize = file->as.subfile.strideSize;
        if (chunkSize == 0 && strideSize == 0) {
            chunkSize = 0x40000000u;
            strideSize = 0x40000000u;
        }
        const unsigned chunk = bytesCompleted / chunkSize;
        const unsigned chunkOffset = bytesCompleted % chunkSize;
        const unsigned chunkRemaining = chunkSize - chunkOffset;
        if (command.bufferSize > chunkRemaining) command.bufferSize = chunkRemaining;
        command.fileOffset = chunkOffset + file->as.subfile.offset + strideSize * chunk;
        const int parentIndex = FileIndex((nflFileID)nfsGetNativeFileID(parentID));
        command.fileHandle = parentIndex >= 0 ? &s_fileHandles[parentIndex] : nullptr;
        if (command.fileHandle == nullptr) return NFD_ERROR_INVALID_ARGUMENTS;
    }

    request.state = NFS_REQUEST_STATE_WORKING;
    nfsUnlock();
    const nfdError result = nfdIoExecute(driver, &command);
    nfsLock();
    nfsRequest* current = nfsGetRequest(requestID);
    if (current != nullptr && current->state == NFS_REQUEST_STATE_WORKING
        && result != NFD_ERROR_NOERROR)
        current->state = NFS_REQUEST_STATE_IO_ERROR;
    return result;
}
int nfsUpdateDriver(nfdDriver* driver)
{
    if (driver == nullptr || driver->io == nullptr) return 1;
    if (driver->work.ioState == NFD_IO_STATE_IDLE) return 1;
    if (driver->io->fnUpdate != nullptr && driver->io->fnUpdate() != 0)
        return 0;
    nfsRequest* request = nfsGetRequest((nflRequestID)driver->work.ioCommand.requestID);
    if (request == nullptr) {
        driver->work.ioState = NFD_IO_STATE_IDLE;
        driver->work.ioCommand.buffer = nullptr;
        driver->work.ioCommand.bufferSize = 0;
        driver->work.ioCommand.fileHandle = nullptr;
        driver->work.ioCommand.fileOffset = 0;
        driver->work.ioCommand.requestID = NFL_REQUEST_ID_INVALID;
        driver->work.ioCommand.requestType = NFL_REQUEST_TYPE_INVALID;
        return 1;
    }
    switch (driver->work.ioState) {
    case NFD_IO_STATE_WORKDONE:
        if (request->state == NFS_REQUEST_STATE_CANCELING) {
            request->state = NFS_REQUEST_STATE_CANCELED;
        } else {
            if (request->state != NFS_REQUEST_STATE_WORKING)
                txAssertFailed(nullptr, "request->state==NFS_REQUEST_STATE_WORKING",
                               "nfsUpdateDriver", "c:/cod/code/tl/nfl/src/nfl_system.cpp", 896);
            const unsigned bytesCompleted = driver->work.bytesCompleted;
            if (bytesCompleted != 0) {
                const unsigned total = bytesCompleted + request->bytesCompleted;
                request->bytesCompleted = total;
                request->state = total < request->bufferSize
                    ? NFS_REQUEST_STATE_WAITING : NFS_REQUEST_STATE_WORKDONE;
            } else {
                request->state = NFS_REQUEST_STATE_WORKDONE;
            }
        }
        driver->work.ioState = NFD_IO_STATE_IDLE;
        driver->work.ioCommand.buffer = nullptr;
        driver->work.ioCommand.bufferSize = 0;
        driver->work.ioCommand.fileHandle = nullptr;
        driver->work.ioCommand.fileOffset = 0;
        driver->work.ioCommand.requestID = NFL_REQUEST_ID_INVALID;
        driver->work.ioCommand.requestType = NFL_REQUEST_TYPE_INVALID;
        return 1;
    case NFD_IO_STATE_WORKING:
        if (request->state == NFS_REQUEST_STATE_CANCELING) {
            if (driver->io->fnCancel != nullptr
                && driver->io->fnCancel(driver->work.ioCommand.fileHandle) == NFD_ERROR_NOERROR)
                driver->work.ioState = NFD_IO_STATE_CANCELING;
            return 0;
        }
        if (request->state != NFS_REQUEST_STATE_WORKING)
            txAssertFailed(nullptr, "request->state==NFS_REQUEST_STATE_WORKING",
                           "nfsUpdateDriver", "c:/cod/code/tl/nfl/src/nfl_system.cpp", 926);
        return 0;
    case NFD_IO_STATE_CANCELING:
        if (request->state != NFS_REQUEST_STATE_CANCELING)
            txAssertFailed(nullptr, "request->state==NFS_REQUEST_STATE_CANCELING",
                           "nfsUpdateDriver", "c:/cod/code/tl/nfl/src/nfl_system.cpp", 932);
        return 0;
    case NFD_IO_STATE_CANCELED:
        if (request->state != NFS_REQUEST_STATE_CANCELING)
            txAssertFailed(nullptr, "request->state==NFS_REQUEST_STATE_CANCELING",
                           "nfsUpdateDriver", "c:/cod/code/tl/nfl/src/nfl_system.cpp", 912);
        request->state = NFS_REQUEST_STATE_CANCELED;
        driver->work.ioState = NFD_IO_STATE_IDLE;
        driver->work.ioCommand.buffer = nullptr;
        driver->work.ioCommand.bufferSize = 0;
        driver->work.ioCommand.fileHandle = nullptr;
        driver->work.ioCommand.fileOffset = 0;
        driver->work.ioCommand.requestID = NFL_REQUEST_ID_INVALID;
        driver->work.ioCommand.requestType = NFL_REQUEST_TYPE_INVALID;
        return 1;
    case NFD_IO_STATE_ERROR:
        if (request->state != NFS_REQUEST_STATE_IO_ERROR_WAITING)
            request->state = NFS_REQUEST_STATE_IO_ERROR;
        return 0;
    default:
        nfsError("nfsUpdateDriver: invalid state %s", nfdIoStateText(driver->work.ioState));
        return 0;
    }
}
void nfsAddRequestToCallList(nfsRequest* request, int shouldDie)
{
    if (request == nullptr || (shouldDie == 0 && request->callback == nullptr)) return;
    if (s_callRequestsCount >= kMaxRequests) return;
    nfsRequestCB& callback = s_callRequests[s_callRequestsCount++];
    callback.state = nfsExportRequestState(request->state);
    callback.callback = request->callback;
    callback.requestID = (nflRequestID)request->slotEntry.slot;
    callback.callbackData = request->callbackData;
    callback.shouldDie = shouldDie;
}
int nfsUpdate()
{
    nfdDriver* drivers[32] = {};
    nflRequestID scheduled[32] = {};
    int shouldSchedule[32] = {};
    const int count = s_nfsDriversCount < 32 ? s_nfsDriversCount : 32;
    for (int index = 0; index < count; ++index) {
        drivers[index] = s_nfsDrivers[index];
        shouldSchedule[index] = nfsUpdateDriver(drivers[index]);
    }
    for (int index = 0; index < count; ++index) {
        if (!shouldSchedule[index]) {
            scheduled[index] = NFL_REQUEST_ID_INVALID;
            continue;
        }
        scheduled[index] = count == 1
            ? nfsScheduleRequest(nullptr) : nfsScheduleRequest(drivers[index]);
    }
    for (int index = 0; index < count; ++index) {
        if (scheduled[index] != NFL_REQUEST_ID_INVALID)
            nfsExecuteRequest(drivers[index], scheduled[index]);
    }
    for (int index = 0; index < count; ++index)
        if (drivers[index] != nullptr && drivers[index]->work.ioState != NFD_IO_STATE_IDLE)
            return 1;
    return 0;
}
void nfsResumeUpdateThread()
{
#ifdef _WIN32
    if (s_nfsEvent != nullptr) SetEvent(s_nfsEvent);
#endif
}
DWORD WINAPI nfsUpdateThread(void*)
{
    while (s_started) {
#ifdef _WIN32
        if (s_nfsEvent != nullptr) WaitForSingleObject(s_nfsEvent, INFINITE);
        if (!s_started) break;
        nfsLock();
#endif
        const int busy = nfsUpdate();
#ifdef _WIN32
        nfsUnlock();
        if (busy != 0) {
            Sleep(8);
            SwitchToThread();
            if (s_nfsEvent != nullptr) SetEvent(s_nfsEvent);
        }
#endif
    }
    return 0;
}

nflRequestID nflWriteFileAsync(nflFileID fileID, unsigned offset, void* buffer, unsigned dataSize);
nflRequestState nflGetRequestState(nflRequestID requestID);
void nflCancelRequest(nflRequestID requestID);
// ea: 0x0041E660
unsigned nfsReadWriteFile(nflFileID fileID, unsigned offset, void* buffer,
                           unsigned dataSize, int doWriteFile)
{
    nflRequestID requestID;
    if (doWriteFile) {
        requestID = nflWriteFileAsync(fileID, offset, buffer, dataSize);
    } else {
        nflRequestParams params;
        params.fileOffset = offset;
        params.fileID = fileID;
        params.streamID = NFL_STREAM_ID_DEFAULT;
        params.callback = nullptr;
        params.type = NFL_REQUEST_TYPE_READ;
        params.priority = NFL_PRIORITY_NORMAL;
        params.buffer = buffer;
        params.dataSize = dataSize;
        params.timeout = 0;
        params.userData = nullptr;
        requestID = nflAddRequest(&params);
    }
    if (requestID == NFL_REQUEST_ID_INVALID) return (unsigned)-1;
    nflUpdate();
    nflRequestState state = nflGetRequestState(requestID);
    if (state == NFL_REQUEST_STATE_INVALID) return dataSize;
    while (state != NFL_REQUEST_STATE_ERROR) {
        nflUpdate();
        state = nflGetRequestState(requestID);
        if (state == NFL_REQUEST_STATE_INVALID) return dataSize;
    }
    nflCancelRequest(requestID);
    return 0;
}
// ea: 0x0041E720
unsigned nflReadFile(nflFileID fileID, unsigned offset, void* buffer, unsigned dataSize)
{
    return nfsReadWriteFile(fileID, offset, buffer, dataSize, 0);
}
// ea: 0x0041E740
unsigned nflWriteFile(nflFileID fileID, unsigned offset, void* buffer, unsigned dataSize)
{
    return nfsReadWriteFile(fileID, offset, buffer, dataSize, 1);
}
// ea: 0x0041E5C0
nflRequestID nflReadFileAsync(nflFileID fileID, unsigned offset, void* buffer, unsigned dataSize)
{
    nflRequestParams params = {}; params.fileID = fileID; params.streamID = NFL_STREAM_ID_DEFAULT;
    params.type = NFL_REQUEST_TYPE_READ; params.priority = NFL_PRIORITY_NORMAL;
    params.fileOffset = offset; params.buffer = buffer; params.dataSize = dataSize;
    return nflAddRequest(&params);
}
// ea: 0x0041E570
nflRequestID nflReadFileAsyncWithCallBack(nflFileID fileID, unsigned offset, void* buffer,
                                               unsigned dataSize,
                                               void (*callback)(nflRequestState, nflRequestID, void*))
{
    nflRequestParams params = {}; params.fileID = fileID; params.streamID = NFL_STREAM_ID_DEFAULT;
    params.callback = callback; params.type = NFL_REQUEST_TYPE_READ; params.priority = NFL_PRIORITY_NORMAL;
    params.fileOffset = offset; params.buffer = buffer; params.dataSize = dataSize;
    return (nflRequestID)nflAddRequest(&params);
}
// ea: 0x0041E610
nflRequestID nflWriteFileAsync(nflFileID fileID, unsigned offset, void* buffer, unsigned dataSize)
{
    nflRequestParams params = {}; params.fileID = fileID; params.streamID = NFL_STREAM_ID_DEFAULT;
    params.type = NFL_REQUEST_TYPE_WRITE; params.priority = NFL_PRIORITY_NORMAL;
    params.fileOffset = offset; params.buffer = buffer; params.dataSize = dataSize;
    return nflAddRequest(&params);
}

// ea: 0x0041F590
nflRequestState nflGetRequestState(nflRequestID requestID)
{
    nfsRequest* request = nfsGetRequest(requestID);
    if (request == nullptr) return NFL_REQUEST_STATE_INVALID;
    switch (request->state) {
    case NFS_REQUEST_STATE_WAITING:
    case NFS_REQUEST_STATE_WORKING: return NFL_REQUEST_STATE_ACTIVE;
    case NFS_REQUEST_STATE_WORKDONE: return NFL_REQUEST_STATE_COMPLETED;
    case NFS_REQUEST_STATE_CANCELED:
    case NFS_REQUEST_STATE_CANCELING: return NFL_REQUEST_STATE_CANCELED;
    case NFS_REQUEST_STATE_TIMEOUT: return NFL_REQUEST_STATE_TIMEOUT;
    case NFS_REQUEST_STATE_IO_ERROR:
    case NFS_REQUEST_STATE_IO_ERROR_WAITING: return NFL_REQUEST_STATE_ERROR;
    default: return NFL_REQUEST_STATE_INVALID;
    }
}
// ea: 0x0041F910
void nflCancelRequest(nflRequestID requestID)
{
    nfsLock();
    nfsRequest* request = nfsGetRequest(requestID);
    if (request != nullptr) {
        request->state = request->state == NFS_REQUEST_STATE_WORKING
            ? NFS_REQUEST_STATE_CANCELING : NFS_REQUEST_STATE_CANCELED;
    }
    nfsUnlock();
}
void nflCancelFileRequests(nflFileID fileID)
{
    for (txSlot requestSlot = txSlotFirst(&s_requestPool); requestSlot != TX_SLOT_INVALID;
         requestSlot = txSlotNext(&s_requestPool, requestSlot)) {
        nfsRequest* request = nfsGetRequest((nflRequestID)requestSlot);
        if (request != nullptr && request->fileID == fileID)
            nflCancelRequest((nflRequestID)requestSlot);
    }
}
// ea: 0x0041F5D0
void nflSetRequestPriority(nflRequestID requestID, nflPriority priority)
{
    nfsLock();
    nfsRequest* request = nfsGetRequest(requestID);
    if (request != nullptr) request->priority = priority | (request->priority & 0xFF00);
    nfsUnlock();
}
// ea: 0x0041F640
nflPriority nflGetRequestPriority(nflRequestID requestID)
{
    nfsRequest* request = nfsGetRequest(requestID);
    return request != nullptr ? (nflPriority)request->priority : (nflPriority)-1;
}
// ea: 0x0041F680
nflRequestInfo* nflGetRequestInfo(nflRequestID requestID, nflRequestInfo* info)
{
    nfsRequest* request = nfsGetRequest(requestID);
    if (info == nullptr || request == nullptr) return info;
    info->bytesCompleted = request->bytesCompleted;
    info->timeElapsed = (unsigned)txTime() - request->queueTime;
    return info;
}
// ea: 0x0041F6D0
float nflGetRequestProgress(nflRequestID requestID)
{
    nfsRequest* requestPtr = nfsGetRequest(requestID);
    if (requestPtr == nullptr) return -1.0f;
    const nfsRequest& request = *requestPtr;
    return (float)request.bytesCompleted / request.bufferSize;
}

// ea: 0x004205D0
nflStreamID nflCreateStream(const nflStreamParams* params)
{
    nflStreamID id = AllocateStream(); if (id == NFL_STREAM_ID_INVALID) return id;
    nfsStream* stream = nfsGetStream(id);
    if (stream == nullptr) return NFL_STREAM_ID_INVALID;
    ClearSlotObject(stream);
    stream->priority = params ? params->streamPriority : NFL_PRIORITY_NORMAL; return id;
}
// ea: 0x0041EF80
void nflSetStreamPriority(nflStreamID streamID, nflPriority priority)
{
    if (streamID == NFL_STREAM_ID_DEFAULT) streamID = s_defaultStreamID;
    nfsStream* stream = nfsGetStream(streamID);
    if (stream != nullptr) stream->priority = priority;
}
// ea: 0x0041F090
nflPriority nflGetStreamPriority(nflStreamID streamID)
{
    if (streamID == NFL_STREAM_ID_DEFAULT) streamID = s_defaultStreamID;
    nfsStream* stream = nfsGetStream(streamID);
    return stream != nullptr ? stream->priority : (nflPriority)-1;
}
// ea: 0x00420600
void nflCancelStreamRequests(nflStreamID streamID)
{
    for (txSlot requestSlot = txSlotFirst(&s_requestPool); requestSlot != TX_SLOT_INVALID;
         requestSlot = txSlotNext(&s_requestPool, requestSlot)) {
        nfsRequest* request = nfsGetRequest((nflRequestID)requestSlot);
        if (request != nullptr && request->streamID == streamID)
            nflCancelRequest((nflRequestID)requestSlot);
    }
}
// ea: 0x00420680
void nflDestroyStream(nflStreamID streamID)
{
    nflCancelStreamRequests(streamID);
    txSlotFree(&s_streamPool, (txSlot)streamID);
}

// ea: 0x00420330
void* nflGetFileHandle(nflFileID fileID, unsigned* handleSize, unsigned* fileStart,
                       unsigned* fileLength, unsigned* fileStride)
{
    nfsFile* filePtr = nfsGetFile(fileID);
    if (filePtr == nullptr) return nullptr;
    nfsFile& file = *filePtr;
    unsigned start = 0, stride = 0;
    nflFileID native = fileID;
    if (file.fileType == NFS_FILE_TYPE_SUBFILE) {
        start = file.as.subfile.offset;
        stride = file.as.subfile.strideSize;
        native = (nflFileID)file.as.subfile.parent;
    }
    nfsFile* nativeFile = nfsGetNativeFile(native);
    const int nativeIndex = FileIndex(native);
    if (nativeFile == nullptr || nativeIndex < 0 || nativeFile->as.native.driver == nullptr)
        return nullptr;
    nfdDriver* driver = nativeFile->as.native.driver;
    if (driver->file == nullptr || driver->file->fnHandle == nullptr)
        return nullptr;
    void* handle = nullptr;
    unsigned nativeHandleSize = 0;
    if (driver->file->fnHandle(&s_fileHandles[nativeIndex], &handle, &nativeHandleSize)
        != NFD_ERROR_NOERROR)
        return nullptr;
    if (handleSize) *handleSize = nativeHandleSize;
    if (fileStart) *fileStart = start;
    if (fileLength) *fileLength = file.size;
    if (fileStride) *fileStride = stride;
    return handle;
}
// ea: 0x00420460
nflState nflGetState()
{
    nflState result = NFL_STATE_INVALID;
    for (int index = 0; index < s_nfsDriversCount; ++index) {
        nfdDriver* driver = s_nfsDrivers[index];
        if (driver == nullptr) continue;
        switch (driver->work.ioState) {
        case NFD_IO_STATE_IDLE:
        case NFD_IO_STATE_WORKDONE:
        case NFD_IO_STATE_CANCELED:
            if (result == NFL_STATE_INVALID) result = NFL_STATE_IDLE;
            break;
        case NFD_IO_STATE_WORKING:
        case NFD_IO_STATE_CANCELING:
            if (result == NFL_STATE_INVALID || result == NFL_STATE_IDLE)
                result = NFL_STATE_BUSY;
            break;
        case NFD_IO_STATE_ERROR:
            result = NFL_STATE_ERROR;
            break;
        default:
            break;
        }
    }
    return result;
}
const char* nflGetStateText(nflState state)
{
    switch (state) { case NFL_STATE_INVALID: return "INVALID"; case NFL_STATE_IDLE: return "IDLE"; case NFL_STATE_BUSY: return "BUSY"; case NFL_STATE_ERROR: return "ERROR"; default: return "???"; }
}
// ea: 0x00420520
void nflRetry()
{
    nfsLock();
    for (int index = 0; index < s_nfsDriversCount; ++index) {
        nfdDriver* driver = s_nfsDrivers[index];
        if (driver == nullptr || driver->work.ioState != NFD_IO_STATE_ERROR) continue;
        nfsRequest* request = nfsGetRequest((nflRequestID)driver->work.ioCommand.requestID);
        if (request != nullptr
            && (request->state == NFS_REQUEST_STATE_IO_ERROR
                || request->state == NFS_REQUEST_STATE_IO_ERROR_WAITING))
            request->state = NFS_REQUEST_STATE_WAITING;
        driver->work.ioState = NFD_IO_STATE_IDLE;
#ifdef _WIN32
        if (s_nfsEvent != nullptr) SetEvent(s_nfsEvent);
#endif
    }
    nfsUnlock();
}
// ea: 0x0041EF30
nflMediaAlignments* nflGetMediaAlignments(nflMediaID media, nflMediaAlignments* alignments)
{
    nfdDriver* driver = nfsGetMediaDriver(media);
    if (alignments == nullptr || driver == nullptr || driver->buffer == nullptr) return nullptr;
    alignments->mediaAlignment = driver->buffer->offsAlignment;
    alignments->memoryAlignment = driver->buffer->addrAlignment;
    alignments->transferSizeAlignment = driver->buffer->sizeAlignment;
    return alignments;
}
const char* nflGetVersion() { return "D3DX"; }
// ea: 0x00421D20
void nflInjectError()
{
    nfl_simulateError.count = 10;
    nfl_simulateError.delayMin = 20;
    nfl_simulateError.delayMax = 40;
}

// ea: 0x00421E70
void* nfd_win32_GetFileHandle(void* fileHandle) { return fileHandle ? *(void**)fileHandle : nullptr; }
// ea: 0x00421E80
nfdError nfd_win32_FileOpen(void* fileHandle, const char* fileName,
                            nfdFileFlags flags, unsigned createdFileSize)
{
    void* handle = OpenHandle(fileName, flags); if (!handle) return NFD_ERROR_FAILURE;
#ifdef _WIN32
    if ((flags & NFD_FILE_FLAGS_CREATE) != 0) {
        LARGE_INTEGER position; position.QuadPart = createdFileSize;
        if (!SetFilePointerEx((HANDLE)handle, position, nullptr, FILE_BEGIN)
            || !SetEndOfFile((HANDLE)handle)) {
            CloseHandleValue(handle);
            return NFD_ERROR_FAILURE;
        }
        SetFilePointer((HANDLE)handle, 0, nullptr, FILE_BEGIN);
    }
#else
    (void)createdFileSize;
#endif
    if (fileHandle) *(void**)fileHandle = handle;
    return NFD_ERROR_NOERROR;
}
// ea: 0x00421F10
nfdError nfd_win32_FileClose(void* fileHandle)
{
    if (!fileHandle) return NFD_ERROR_INVALID_ARGUMENTS;
#ifdef _WIN32
    return CloseHandle(*(HANDLE*)fileHandle) ? NFD_ERROR_NOERROR : NFD_ERROR_FAILURE;
#else
    CloseHandleValue(*(void**)fileHandle);
    return NFD_ERROR_NOERROR;
#endif
}
// ea: 0x00421F30
nfdError nfd_win32_FileStatus(void* fileHandle, nfdFileInfo* info)
{
    if (!fileHandle || !info) return NFD_ERROR_INVALID_ARGUMENTS;
    info->location = 0; info->size = HandleSize(*(void**)fileHandle); return NFD_ERROR_NOERROR;
}
// ea: 0x00421F60
nfdError nfd_win32_FileHandle(void* fileHandle, void** handle, unsigned* handleSize)
{
    if (!fileHandle || !handle || !handleSize) return NFD_ERROR_INVALID_ARGUMENTS;
    *handle = fileHandle;
    *handleSize = sizeof(void*);
    return NFD_ERROR_NOERROR;
}
void __stdcall nfd_win32_IoCompletionRoutine(DWORD, DWORD, OVERLAPPED*);
// ea: 0x00421FD0
nfdError nfd_win32_IoExecute(nfdDriver* driver, void* fileHandle, nflRequestType type, unsigned offset, void* buffer, unsigned size)
{
    if (type != NFL_REQUEST_TYPE_READ && type != NFL_REQUEST_TYPE_WRITE)
        txAssertFailed(nullptr, "requestType==NFL_REQUEST_TYPE_READ || requestType==NFL_REQUEST_TYPE_WRITE",
                       "nfd_win32_IoExecute", "c:/cod/code/tl/nfl/src/win32/nfl_win32_driver.cpp", 100);
    if (driver == nullptr || fileHandle == nullptr || buffer == nullptr)
        return NFD_ERROR_INVALID_ARGUMENTS;
#ifdef _WIN32
    nfd_win32_ioWork.driver = driver;
    nfd_win32_ioWork.overlapped.Offset = offset;
    const BOOL queued = type == NFL_REQUEST_TYPE_WRITE
        ? WriteFileEx(*(HANDLE*)fileHandle, buffer, size, &nfd_win32_ioWork.overlapped,
                      nfd_win32_IoCompletionRoutine)
        : ReadFileEx(*(HANDLE*)fileHandle, buffer, size, &nfd_win32_ioWork.overlapped,
                     nfd_win32_IoCompletionRoutine);
    return queued ? NFD_ERROR_NOERROR : NFD_ERROR_FAILURE;
#else
    const bool failed = type == NFL_REQUEST_TYPE_WRITE
        ? WriteAt(*(void**)fileHandle, offset, buffer, size) != size
        : ReadAt(*(void**)fileHandle, offset, buffer, size) != size;
    return failed ? NFD_ERROR_FAILURE : NFD_ERROR_NOERROR;
#endif
}
// ea: 0x00422070
nfdError nfd_win32_IoCancel(void* fileHandle)
{
#ifdef _WIN32
    return fileHandle != nullptr && CancelIo(*(HANDLE*)fileHandle) ? NFD_ERROR_NOERROR : NFD_ERROR_FAILURE;
#else
    (void)fileHandle;
    return NFD_ERROR_NOERROR;
#endif
}
// ea: 0x00422090
nfdError nfd_win32_IoUpdate()
{
#ifdef _WIN32
    SleepEx(0, TRUE);
#endif
    return NFD_ERROR_NOERROR;
}
// ea: 0x00421F80
void __stdcall nfd_win32_IoCompletionRoutine(DWORD errorCode, DWORD bytesTransferred,
                                              OVERLAPPED* overlapped)
{
    if (overlapped != &nfd_win32_ioWork.overlapped)
        txAssertFailed(nullptr, "lpOverlapped == &nfd_win32_ioWork.overlapped",
                       "nfd_win32_IoCompletionRoutine",
                       "c:/cod/code/tl/nfl/src/win32/nfl_win32_driver.cpp", 92);
    nfdIoComplete(nfd_win32_ioWork.driver, bytesTransferred, (int)errorCode);
}
// ea: 0x00421A70
int nfdBufferAlign(const nfdDriver* driver, const nfdIoCommand* command,
                   unsigned fileSize, unsigned* alignedFileOffset,
                   unsigned* alignedBufferSize)
{
    if (alignedFileOffset) *alignedFileOffset = 0;
    if (alignedBufferSize) *alignedBufferSize = 0;
    if (driver == nullptr || command == nullptr || driver->init == nullptr
        || driver->buffer == nullptr || alignedFileOffset == nullptr
        || alignedBufferSize == nullptr)
        return NFD_ERROR_INVALID_ARGUMENTS;
    const nfdBuffer& buffer = *driver->buffer;
    const unsigned bufferSize = command->bufferSize;
    const unsigned fileOffset = command->fileOffset;
    if (bufferSize == 0) return 0;
    const uintptr_t address = reinterpret_cast<uintptr_t>(command->buffer);
    const uintptr_t bufferA = (address + buffer.addrAlignment - 1)
                            & ~(uintptr_t)(buffer.addrAlignment - 1);
    const unsigned alignedOffset = fileOffset & ~(buffer.offsAlignment - 1);
    unsigned alignedSize = (fileOffset + buffer.sizeAlignment - alignedOffset + bufferSize - 1)
                         & ~(buffer.sizeAlignment - 1);
    const unsigned maxSize = (buffer.sizeAlignment + fileSize - 1)
                           & ~(buffer.sizeAlignment - 1);
    if (alignedSize > buffer.size) alignedSize = buffer.size;
    if (command->requestType == NFL_REQUEST_TYPE_READ) {
        if (fileOffset >= fileSize) return 0;
        if (alignedSize + alignedOffset > maxSize) alignedSize = maxSize - alignedOffset;
        alignedSize = (buffer.sizeAlignment + alignedSize - 1)
                    & ~(buffer.sizeAlignment - 1);
    }
    *alignedFileOffset = alignedOffset;
    *alignedBufferSize = alignedSize;
    switch (driver->init->bufferMode) {
    case NFL_BUFFER_MODE_NOBUFFER:
    case NFL_BUFFER_MODE_UNALIGNED:
    case NFL_BUFFER_MODE_VM: {
        unsigned unaligned = 0;
        if (address != bufferA) {
            unaligned |= 1;
            nfsWarning("nfdBufferAlign: Unaligned access to 0x%08X, should be aligned to 0x%08X\n",
                       static_cast<unsigned>(address), buffer.addrAlignment);
        }
        if (((buffer.sizeAlignment - 1) & bufferSize) != 0) {
            unaligned |= 2;
            nfsWarning("nfdBufferAlign: Unaligned transfer of 0x%08X bytes, should be aligned to 0x%08X\n",
                       bufferSize, buffer.sizeAlignment);
        }
        if (fileOffset != alignedOffset) {
            unaligned |= 4;
            nfsWarning("nfdBufferAlign: Unaligned seek to offset 0x%08X, should be aligned to 0x%08X\n",
                       fileOffset, buffer.offsAlignment);
        }
        return driver->init->bufferMode == NFL_BUFFER_MODE_NOBUFFER
            ? -(unaligned != 0) : (unaligned != 0);
    }
    case NFL_BUFFER_MODE_ALL:
        return 1;
    default:
        nfsError("Invalid nflInitParams.bufferMode specified\n");
        return -1;
    }
}
// ea: 0x00421C30
nfdError nfdIoExecute(nfdDriver* driver, const nfdIoCommand* command)
{
    if (driver == nullptr || command == nullptr) return NFD_ERROR_INVALID_ARGUMENTS;
    if (command->requestType != NFL_REQUEST_TYPE_READ && command->requestType != NFL_REQUEST_TYPE_WRITE)
        return NFD_ERROR_INVALID_ARGUMENTS;
    unsigned fileOffset = command->fileOffset;
    unsigned bufferSize = command->bufferSize;
    unsigned alignedOffset = 0;
    unsigned alignedSize = 0;
    int usesBuffer = 0;
    if (command->requestType == NFL_REQUEST_TYPE_READ) {
        usesBuffer = nfdBufferAlign(driver, command, command->fileSize, &alignedOffset, &alignedSize);
        if (usesBuffer < 0) return NFD_ERROR_UNALIGNED_ACCESS;
        if (alignedSize == 0) {
            driver->work.bytesCompleted = 0;
            driver->work.ioCommand = *command;
            driver->work.ioState = NFD_IO_STATE_WORKDONE;
            return NFD_ERROR_NOERROR;
        }
        fileOffset = alignedOffset;
        bufferSize = alignedSize;
    }
    driver->work.usesBuffer = usesBuffer;
    driver->work.fileOffsetA = fileOffset;
    driver->work.bufferSizeA = bufferSize;
    void* address = usesBuffer != 0 && driver->buffer != nullptr ? driver->buffer->addr : command->buffer;
    nfdError result = NFD_ERROR_FAILURE;
    if (driver->io != nullptr && driver->io->fnExecute != nullptr)
        result = driver->io->fnExecute(driver, command->fileHandle, command->requestType,
                                       fileOffset, address, bufferSize);
    else
        result = nfd_win32_IoExecute(driver, command->fileHandle, command->requestType,
                                     fileOffset, address, bufferSize)
            ? NFD_ERROR_FAILURE : NFD_ERROR_NOERROR;
    if (result == NFD_ERROR_NOERROR) {
        driver->work.ioCommand = *command;
        driver->work.ioState = NFD_IO_STATE_WORKING;
    }
    return result;
}
// ea: 0x00421D40
void nfdIoComplete(nfdDriver* driver, unsigned bytesCompleted, int errorCode)
{
    if (driver == nullptr) return;
    int effectiveError = errorCode;
    if (effectiveError == 0 && nfl_simulateError.count != 0) {
        if (nfl_simulateError.delayMax < nfl_simulateError.delayMin)
            nfl_simulateError.delayMax = nfl_simulateError.delayMin;
        const unsigned span = nfl_simulateError.delayMax - nfl_simulateError.delayMin + 1;
        if (nfl_simulateError.delayCnt++ >= nfl_simulateError.delayMin
            + (span != 0 ? (unsigned)std::rand() % span : 0)) {
            --nfl_simulateError.count;
            nfsMessage("Simulating error: count=%u delay=[%u,%u]\n",
                       nfl_simulateError.count,
                       nfl_simulateError.delayMin,
                       nfl_simulateError.delayMax);
            effectiveError = nfl_simulateError.errorCode;
            nfl_simulateError.delayCnt = 0;
        }
    }
    if (driver->work.ioState == NFD_IO_STATE_CANCELING) {
        driver->work.ioState = NFD_IO_STATE_CANCELED;
        return;
    }
    if (driver->work.ioState != NFD_IO_STATE_WORKING) {
        nfsError("nfdIoCompleted: called in inappropriate state %s",
                 nfdIoStateText(driver->work.ioState));
        return;
    }
    if (effectiveError != 0) {
        nfsMessage("nfdIoComplete: failed, errorCode=%d", effectiveError);
        driver->work.ioState = NFD_IO_STATE_ERROR;
        return;
    }
    if (driver->work.usesBuffer != 0 && driver->buffer != nullptr && driver->work.ioCommand.buffer != nullptr) {
        unsigned delta = driver->work.ioCommand.fileOffset - driver->work.fileOffsetA;
        unsigned count = bytesCompleted > delta ? bytesCompleted - delta : 0;
        if (count > driver->work.ioCommand.bufferSize) count = driver->work.ioCommand.bufferSize;
        if (count != 0) std::memcpy(driver->work.ioCommand.buffer,
                                    static_cast<const char*>(driver->buffer->addr) + delta, count);
        driver->work.bytesCompleted = count;
    } else {
        driver->work.bytesCompleted = bytesCompleted;
    }
    driver->work.ioState = NFD_IO_STATE_WORKDONE;
}
// ea: 0x00421560
nfdError nfd_xbox_MediaBind(nflMediaID media, const char* src, char* dst, int dstSize)
{
    if (!src || !dst || dstSize <= 0) return NFD_ERROR_INVALID_ARGUMENTS;
    const char* prefix = nullptr;
    if (std::isalpha(static_cast<unsigned char>(src[0])) && src[1] == ':') {
        prefix = "";
    } else if (media == NFL_MEDIA_ID_DISC) {
        prefix = "D:\\";
    } else if (media == NFL_MEDIA_ID_HOST) {
        prefix = "E:\\";
    } else {
        return NFD_ERROR_INVALID_MEDIA;
    }
#ifdef _WIN32
    strcpy_s(dst, static_cast<size_t>(dstSize), prefix);
    strncat_s(dst, static_cast<size_t>(dstSize), src, _TRUNCATE);
#else
    std::strncpy(dst, prefix, static_cast<size_t>(dstSize - 1));
    dst[dstSize - 1] = 0;
    std::strncat(dst, src, static_cast<size_t>(dstSize - 1 - std::strlen(dst)));
#endif
    for (char* cursor = dst; *cursor != 0; ++cursor)
        if (*cursor == '/') *cursor = '\\';
    return NFD_ERROR_NOERROR;
}

// Transaction and path helpers from tx.o / txPath.o.
extern "C" {
// ea: 0x00420E40
void txInit() {}
// ea: 0x00420D70
unsigned long long txTime() { return (unsigned long long)Now(); }
// ea: 0x00420DA0
int txMatch(const char* value, const char* pattern)
{
    if (pattern == nullptr || value == nullptr) return 0;
    if (*pattern == 0) return *value == 0;
    if (*pattern != '*') {
        if (*value == 0 || (*pattern != '?' && *pattern != *value)) return 0;
        return txMatch(value + 1, pattern + 1);
    }
    ++pattern;
    if (*pattern == 0) return 1;
    if (*pattern == '?' || *pattern == '*') {
        while (*value != 0) {
            if (txMatch(value, pattern)) return 1;
            ++value;
        }
        return 0;
    }
    while (*value != 0) {
        if (*value == *pattern && txMatch(value + 1, pattern + 1)) return 1;
        ++value;
    }
    return 0;
}
// ea: 0x00420E50
unsigned txGetPrimeLessThanPow2(unsigned num)
{
    static const unsigned primes[] = {
        0, 1, 2, 3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039,
        4093, 8191, 16381, 32749, 65521, 131071, 262139, 524287,
        1048573, 2097143, 4194301, 8388593, 16777213, 33554393,
        67108859, 134217689, 268435399, 536870909, 1073741789,
        2147483647u, 4294967292u,
    };
    int low = 0;
    int high = 34;
    int index = 17;
    while (index != low) {
        const unsigned value = primes[index];
        if (num <= value) high = index;
        if (num >= value) low = index;
        index = (high + low) / 2;
    }
    return primes[low];
}
// ea: 0x00420990
void txSlotEntryInit(txSlotEntry* entry)
{
    entry->next = entry; entry->prev = entry; entry->slot = TX_SLOT_INVALID;
}
// ea: 0x004209B0
void txSlotEntryLinkHead(txSlotEntry* root, txSlotEntry* item)
{
    item->next = root->next; item->prev = root;
    root->next->prev = item; root->next = item;
}
// ea: 0x004209D0
void txSlotEntryLinkTail(txSlotEntry* root, txSlotEntry* item)
{
    item->next = root; item->prev = root->prev;
    root->prev->next = item; root->prev = item;
}
// ea: 0x004209F0
void txSlotEntryUnlink(txSlotEntry* item)
{
    item->prev->next = item->next; item->next->prev = item->prev;
}
// ea: 0x00420A10
txSlotEntry* txSlotEntryPtr(const txSlotPool* pool, txSlot slot)
{
    unsigned index = slot & (pool->mask | 0x80000000u);
    if (index >= (unsigned)pool->count) return nullptr;
    txSlotEntry* entry = (txSlotEntry*)((char*)pool->slots + index * pool->stride);
    return entry->slot == slot ? entry : nullptr;
}
// ea: 0x00420A50
int txSlotIndex(const txSlotPool* pool, txSlot slot)
{
    txSlotEntry* entry = txSlotEntryPtr(pool, slot);
    return entry == nullptr ? -1 : (int)(slot & (pool->mask | 0x80000000u));
}
// ea: 0x00420A90
int txSlotPoolInit(txSlotPool* pool, txSlotEntry* slots, int count, unsigned stride)
{
    if (pool == nullptr || slots == nullptr || count < 0 || stride < 12) return 0;
    unsigned bits = 0; for (int value = count; value != 0; value >>= 1) ++bits;
    pool->slots = slots; pool->stride = (int)stride; pool->count = count;
    pool->mask = (1 << bits) - 1;
    txSlotEntryInit(&pool->freeSlots); txSlotEntryInit(&pool->usedSlots);
    for (int i = 0; i < count; ++i) {
        txSlotEntry* entry = (txSlotEntry*)((char*)slots + i * stride);
        entry->slot = (txSlot)(i | 0x80000000u);
        txSlotEntryLinkTail(&pool->freeSlots, entry);
    }
    return 1;
}
// ea: 0x00420B40
bool txSlotExists(const txSlotPool* pool, txSlot slot)
{
    return txSlotEntryPtr(pool, slot) != nullptr;
}
// ea: 0x00420B90
txSlot txSlotEntryInc(const txSlotPool* pool, txSlotEntry* entry)
{
    txSlot value = (entry->slot + pool->mask + 1) & 0x7FFFFFFFu;
    entry->slot = value;
    if ((~(pool->mask | 0x80000000u) & value) == 0)
        entry->slot = value + pool->mask + 1;
    return entry->slot;
}
// ea: 0x00420BD0
txSlot txSlotNew(txSlotPool* pool)
{
    txSlotEntry* entry = pool->freeSlots.prev;
    if (entry == &pool->freeSlots) return TX_SLOT_INVALID;
    txSlotEntryUnlink(entry); txSlotEntryLinkTail(&pool->usedSlots, entry);
    return txSlotEntryInc(pool, entry);
}
// ea: 0x00420C40
void txSlotFree(txSlotPool* pool, txSlot slot)
{
    txSlotEntry* entry = txSlotEntryPtr(pool, slot);
    if (entry == nullptr) return;
    txSlotEntryUnlink(entry); entry->slot |= 0x80000000u;
    txSlotEntryLinkTail(&pool->freeSlots, entry);
}
// ea: 0x00420CB0
txSlot txSlotFirst(const txSlotPool* pool) { return pool->usedSlots.next->slot; }
// ea: 0x00420CC0
txSlot txSlotLast(const txSlotPool* pool) { return pool->usedSlots.prev->slot; }
// ea: 0x00420CD0
txSlot txSlotNext(const txSlotPool* pool, txSlot slot)
{
    txSlotEntry* entry = txSlotEntryPtr(pool, slot);
    return entry == nullptr || entry->next == &pool->usedSlots ? TX_SLOT_INVALID : entry->next->slot;
}
// ea: 0x00420D20
txSlot txSlotPrev(const txSlotPool* pool, txSlot slot)
{
    txSlotEntry* entry = txSlotEntryPtr(pool, slot);
    return entry == nullptr || entry->prev == &pool->usedSlots ? TX_SLOT_INVALID : entry->prev->slot;
}
// ea: 0x00420EA0
void txPrintf(const char* channel, int level, const char* fmt, ...)
{
    va_list args; va_start(args, fmt);
    if (channel != nullptr) std::fprintf(stdout, "%s:%d: ", channel, level);
    vfprintf(stdout, fmt, args);
    va_end(args);
}
// ea: 0x00421450
void txPathNormalize(char* path)
{
    if (path == nullptr || *path == 0) return;
    int index = 0;
    do {
        const char value = path[index];
        if (value == '/' && path[index + 1] == '/') {
            int next = index + 2;
            while (path[next] == '/') ++next;
            const int delta = index + 1 - next;
            char copied;
            do {
                copied = path[next];
                path[next + delta] = copied;
                ++next;
            } while (copied != 0);
        } else if (index == 0 || path[index - 1] != '.') {
            if (value == '.' && path[index + 1] == '/') {
                int next = index + 2;
                if (path[index + 2] == '/') {
                    do { ++next; } while (path[next] == '/');
                }
                const int delta = index - next;
                char copied;
                do {
                    copied = path[next];
                    path[next + delta] = copied;
                    ++next;
                } while (copied != 0);
            } else if (value == '/' && path[index + 1] == '.'
                       && path[index + 2] == '.' && path[index + 3] == '/') {
                int previous = index - 1;
                int next = index + 3;
                while (path[next] == '/') ++next;
                if (previous > 0) {
                    while (path[previous] == '/') {
                        if (--previous <= 0) break;
                    }
                    while (previous > 0 && path[previous] != '/') --previous;
                }
                index = previous;
                if (path[previous] == '/') ++previous;
                const int delta = previous - next;
                char copied;
                do {
                    copied = path[next];
                    path[next + delta] = copied;
                    ++next;
                } while (copied != 0);
            }
        }
        --index;
        ++index;
    } while (path[index] != 0);
}
// ea: 0x00421060
void txBreak();
void txAssertFailed(unsigned char* ignore, const char* message, const char* function,
                    const char* source, int line)
{
    if (ignore != nullptr && *ignore != 0 && *ignore != 0xFF) {
        ++*ignore;
        return;
    }
    if (_tlAssert(source != nullptr ? source : "", line,
                  function != nullptr ? function : "",
                  message != nullptr ? message : "assertion failed"))
        txBreak();
}
// ea: 0x00420F90
void txBreak()
{
#ifdef _WIN32
    DebugBreak();
#endif
}
// ea: 0x00420FD0
void* txMemAlloc(unsigned size) { return tlMemAlloc(size, 8u, 0); }
// ea: 0x00420FA0
void* txMemAllocEx(unsigned size, unsigned alignment, unsigned flags)
{
    if (alignment == 0) alignment = 4;
    return tlMemAlloc(size, alignment, flags);
}
// ea: 0x00420FF0
void txMemFree(void* ptr) { tlMemFree(ptr); }
// ea: 0x00421000
void* txMemRealloc() { txBreak(); return nullptr; }
// ea: 0x00421010
void* txMemReallocAdHoc(void* ptr, int size, int oldSize)
{
    if (size < 0) return nullptr;
    void* result = tlMemAlloc((unsigned)size, 8u, 0);
    if (result == nullptr) return nullptr;
    if (ptr != nullptr && oldSize > 0)
        std::memcpy(result, ptr, (size_t)(size < oldSize ? size : oldSize));
    tlMemFree(ptr);
    return result;
}
// ea: 0x00420EC0
void txPrintv(const char* channel, int level, const char* fmt, char* list)
{
    if (channel != nullptr) std::fprintf(stdout, "%s:%d: ", channel, level);
    if (list != nullptr) vfprintf(stdout, fmt, reinterpret_cast<va_list>(list));
}

// ea: 0x004210A0
char* txPathFix(const char* src, char* dir, int dirSize)
{
    if (src == nullptr || dir == nullptr || dirSize <= 0) return nullptr;
    int remaining = dirSize - 1;
    const char* input = src;
    char* output = dir;
    if (input[0] == '\\' && input[1] == '\\' && remaining >= 2) {
        *output++ = '\\'; *output++ = '\\'; input += 2; remaining -= 2;
    }
    bool pendingSlash = true;
    while (*input != 0 && remaining > 0) {
        char ch = *input++;
        if (ch == '\\' || ch == '/') {
            if (pendingSlash) continue;
            *output++ = '/'; --remaining; pendingSlash = true;
        } else {
            *output++ = ch; --remaining; pendingSlash = false;
        }
    }
    *output = 0;
    char* read = dir;
    char* write = dir;
    char previous = 0;
    while (*read != 0 && write < dir + dirSize - 1) {
        if ((previous == 0 || previous == ':' || previous == '/')
            && read[0] == '.' && read[1] == '/') {
            read += 2; continue;
        }
        previous = *read; *write++ = *read++;
    }
    *write = 0;
    return dir;
}
// ea: 0x00421170
char* txPathFormat(const char* src, char* dest, int destSize)
{
    char buffer[256]; const char* input = src != nullptr && *src != 0 ? src : ".";
    strncpy_s(buffer, sizeof(buffer), input, _TRUNCATE);
    strncat_s(buffer, sizeof(buffer), "/", _TRUNCATE);
    return txPathFix(buffer, dest, destSize);
}
// ea: 0x004211E0
char* txPathMake(const char* src, char* dest, int destSize)
{
    return txPathFormat(src, dest, destSize);
}
// ea: 0x00421250
const char* txPathDirEnd(char* src)
{
    char* slash = src != nullptr ? strrchr(src, '/') : nullptr;
    return slash != nullptr ? slash + 1 : nullptr;
}
// ea: 0x00421270
char* txPathFileStart(char* src)
{
    char* slash = src != nullptr ? strrchr(src, '/') : nullptr;
    return slash != nullptr ? slash + 1 : src;
}
// ea: 0x00421290
void txPathExtStart(char* src)
{
    char* file = txPathFileStart(src);
    if (file != nullptr) strrchr(file, '.');
}
// ea: 0x004212C0
char* txPathFile(char* src, char* dest, int destSize)
{
    if (dest == nullptr || destSize <= 0) return dest;
    const char* file = txPathFileStart(src); if (file == nullptr) file = "";
    strncpy_s(dest, destSize, file, _TRUNCATE); return dest;
}
// ea: 0x00421320
char* txPathExt(char* src, char* dest, int destSize)
{
    if (dest == nullptr || destSize <= 0) return dest;
    char* file = txPathFileStart(src); char* ext = file != nullptr ? strrchr(file, '.') : nullptr;
    strncpy_s(dest, destSize, ext != nullptr ? ext : "", _TRUNCATE); return dest;
}
// ea: 0x00421390
char* txPathDir(char* src, char* dest, int destSize)
{
    if (dest == nullptr || destSize <= 0) return dest;
    char* slash = src != nullptr ? strrchr(src, '/') : nullptr;
    int length = slash != nullptr ? (int)(slash - src + 1) : 0;
    if (length >= destSize) length = destSize - 1;
    if (length > 0) memcpy(dest, src, length); dest[length] = 0; return dest;
}
// ea: 0x004213E0
char* txPathName(char* src, char* dest, int destSize)
{
    if (dest == nullptr || destSize <= 0) return dest;
    char* file = txPathFileStart(src); char* ext = file != nullptr ? strrchr(file, '.') : nullptr;
    int length = ext != nullptr ? (int)(ext - file) : 0;
    if (length >= destSize) length = destSize - 1;
    if (length > 0) memcpy(dest, file, length); dest[length] = 0; return dest;
}
}
