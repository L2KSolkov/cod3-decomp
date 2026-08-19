// ============================================================================
// Job Queue — multithreaded batch job processing system
// Source: source/jobqueue.cpp (line refs: 0x60, 0x8D, 0x98, 0xE9)
// ea: 0x834E80-0x835520 (25 funcs)
// ============================================================================

#include <cstring>
#include <cstdlib>
#include <cstdint>

// External
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);
extern bool  _tlAssert(const char* file, int line, const char* expr, const char* msg);

// ============================================================================
// Types — 56-byte batch descriptor (verified against IDA)
// ============================================================================
struct jqBatch {
    void*    Func;          // +0x00 — function to execute
    void*    Data;          // +0x04 — user data
    uint32_t Priority;      // +0x08 — 0=low, 1=med, 2=high
    uint32_t GroupID;       // +0x0C — batch group (-1 = none)
    int32_t  Handle;        // +0x10 — batch handle (index into pool)
    int32_t  Next;          // +0x14 — next in free/queue list
    // +0x18 ... +0x38 — padding (56 bytes total)
    uint8_t  _pad[32];
};
// NOTE: sizeof(jqBatch) is 56 on 32-bit Xbox, varies on 64-bit due to pointer padding
// static_assert(sizeof(jqBatch) == 56, "");  // 32-bit only

struct jqWorker {
    jqBatch* LocalBatchPool;       // +0x00
    uint32_t LocalBatchPoolSize;   // +0x04
    uint32_t LocalBatchPoolCount;  // +0x08
    // +0x0C ... +0x14 — padding (20 bytes total)
    uint8_t  _pad[8];
};

struct jqBatchGroup {
    int32_t Next;            // +0x00 — next in free list
    int32_t BatchCount;      // +0x04 — active batches in group
    int32_t NDependencies;   // +0x08 — unresolved dependencies
};

struct jqBatchGroupPool {
    jqBatchGroup* Value;     // +0x00
    int32_t Size;            // +0x04
    int32_t Head;            // +0x08
};

struct jqBatchPool {
    jqBatch* Value;          // +0x00
    int32_t  Size;           // +0x04
    int32_t  Head;           // +0x08
};

struct jqPoolState {
    jqBatchPool      BatchPool;
    int32_t          BatchPoolSize;
    int32_t          BatchPoolHead;
    int32_t          BatchPoolCount;
    int32_t          BatchQueueHead[3];  // priority queues
    int32_t          BatchQueueTail[3];
    int32_t          BatchQueueCount[3];
    jqBatchGroupPool BatchGroupPool;
    int32_t          BatchGroupPoolHead;
};

// Forward declarations (interdependencies)
void jqStop();
void _jqStop();
void _jqStart(int nWorkers);
void _jqAddBatch(jqBatch* batch);
void jqFlush(int groupID);
static jqPoolState jqPool = {};
static jqWorker*   jqWorkers = nullptr;
static int         jqNWorkers = 0;
static int         jqBatchPoolMutex = 0;

// ============================================================================
// Init / Shutdown
// ============================================================================
void _jqInit() { /* actual init is in jqStart */ }
void _jqShutdown() {}

void jqInit() {
    jqBatchPoolMutex = 0;
    _jqInit();
}

// ea: 0x8353D0
void jqShutdown() {
    jqStop();
    if (jqWorkers) {
        tlMemFree(jqWorkers);
        jqWorkers = nullptr;
    }
    jqNWorkers = 0;
    _jqShutdown();
}

// ============================================================================
// jqStop — stop all worker threads
// ea: 0x834EA0
// ============================================================================
void jqStop() {
    _jqStop();
}

void _jqStop() {
    // Signal workers to stop and join
}

// ============================================================================
// jqStart — start worker threads
// ea: 0x834F10
// ============================================================================
void jqStart(int nWorkers) {
    jqStop();

    jqNWorkers = nWorkers;
    jqWorkers = (jqWorker*)tlMemAlloc(nWorkers * 20, 0, 0);

    for (int i = 0; i < nWorkers; ++i) {
        jqWorkers[i].LocalBatchPool = nullptr;
        jqWorkers[i].LocalBatchPoolSize = 0;
        jqWorkers[i].LocalBatchPoolCount = 0;
    }

    _jqStart(nWorkers);
}

void _jqStart(int nWorkers) {
    // Create worker threads
    // On Xbox: PsCreateSystemThreadEx or jobqueue kernel
    // On Win32: std::thread or CreateThread
}

// ============================================================================
// jqValidate — assert queue integrity
// ea: 0x835470
// ============================================================================
void jqValidate() {
    // No-op in release builds
}

// ============================================================================
// jqGetMaxWorkers
// ea: 0x834E90
// ============================================================================
int jqGetMaxWorkers() {
    return jqNWorkers;
}

// ============================================================================
// jqLockBatchPool / jqUnlockBatchPool
// ea: 0x834EF0, 0x834F00
// ============================================================================
void jqLockBatchPool() {
    // Spinlock / mutex acquire
}

void jqUnlockBatchPool() {
    // Spinlock / mutex release
}

// ============================================================================
// jqSetBatchPoolSize — allocate/resize batch descriptor pool
// ea: 0x834F90
// ============================================================================
void jqSetBatchPoolSize(int size) {
    if (size <= 0) {
        _tlAssert("source/jobqueue.cpp", 0x60, "", "");
        __debugbreak();
    }

    // Flush existing
    jqFlush(0);

    if (jqPool.BatchPool.Value) {
        tlMemFree(jqPool.BatchPool.Value);
        jqPool.BatchPool.Value = nullptr;
    }

    if (size > 0) {
        jqPool.BatchPool.Value = (jqBatch*)tlMemAlloc(size * sizeof(jqBatch), 0, 0);
    }

    // Initialize free list
    jqPool.BatchPool.Size = size;
    for (int i = 0; i < size - 1; ++i)
        jqPool.BatchPool.Value[i].Next = i + 1;
    if (size > 0)
        jqPool.BatchPool.Value[size - 1].Next = -1;

    // Clear priority queues
    for (int p = 0; p < 3; ++p) {
        jqPool.BatchQueueHead[p] = -1;
        jqPool.BatchQueueTail[p] = -1;
        jqPool.BatchQueueCount[p] = 0;
    }

    jqPool.BatchPoolHead = 0;
    jqPool.BatchPoolCount = size;

    // Initialize handles
    for (int i = 0; i < size; ++i)
        jqPool.BatchPool.Value[i].Handle = i;
}

// ============================================================================
// jqFlush — wait for all batches to complete
// ea: 0x8354D0
// ============================================================================
void jqFlush(int groupID) {
    // Wait until BatchQueueCount[0..2] are all 0
    // and groupID's BatchCount is 0
}

// ============================================================================
// jqAddBatch — enqueue a batch job
// ea: 0x8350A0
// ============================================================================
int jqAddBatch(const jqBatch& data) {
    jqValidate();

    int idx = jqPool.BatchPoolHead;
    if (idx == -1) {
        _tlAssert("source/jobqueue.cpp", 0x8D, "", "");
        __debugbreak();
    }

    // Pop from free list
    jqBatch* batch = &jqPool.BatchPool.Value[idx];
    jqPool.BatchPoolHead = batch->Next;
    --jqPool.BatchPoolCount;

    // Copy data
    memcpy(batch, &data, sizeof(jqBatch));

    uint32_t priority = batch->Priority;
    batch->Handle = idx;

    if (priority > 2) {
        _tlAssert("source/jobqueue.cpp", 0x98, "", "");
        __debugbreak();
    }

    // Add to priority queue
    int tail = jqPool.BatchQueueTail[priority];
    if (tail != -1)
        jqPool.BatchPool.Value[tail].Next = batch->Handle;
    jqPool.BatchQueueTail[priority] = batch->Handle;
    batch->Next = -1;

    if (jqPool.BatchQueueHead[priority] == -1)
        jqPool.BatchQueueHead[priority] = batch->Handle;

    ++jqPool.BatchQueueCount[priority];

    // Track batch group
    if (batch->GroupID != -1)
        ++jqPool.BatchGroupPool.Value[batch->GroupID].BatchCount;

    _jqAddBatch(batch);
    jqValidate();

    return idx;
}

void _jqAddBatch(jqBatch* batch) {
    // Signal worker threads to process
}

// ============================================================================
// jqRemoveBatch — cancel a batch
// ea: 0x8354C0
// ============================================================================
bool jqRemoveBatch(int handle) {
    if (handle < 0 || handle >= jqPool.BatchPoolSize) return false;

    jqBatch* batch = &jqPool.BatchPool.Value[handle];
    unsigned prio = batch->Priority;

    // Unlink from priority queue
    int prev = -1;
    int cur = jqPool.BatchQueueHead[prio];
    int found = -1;
    while (cur != -1) {
        if (cur == handle) { found = prev; break; }
        prev = cur;
        cur = jqPool.BatchPool.Value[cur].Next;
    }

    if (found >= 0 || jqPool.BatchQueueHead[prio] == handle) {
        if (found >= 0) {
            jqPool.BatchPool.Value[found].Next = batch->Next;
        } else {
            jqPool.BatchQueueHead[prio] = batch->Next;
        }
        if (jqPool.BatchQueueTail[prio] == handle)
            jqPool.BatchQueueTail[prio] = found;

        --jqPool.BatchQueueCount[prio];

        // Return to free list
        batch->Next = jqPool.BatchPoolHead;
        jqPool.BatchPoolHead = handle;
        ++jqPool.BatchPoolCount;

        if (batch->GroupID != -1)
            --jqPool.BatchGroupPool.Value[batch->GroupID].BatchCount;

        return true;
    }

    return false;
}

// ============================================================================
// Batch Group Management
// ============================================================================
void jqSetBatchGroupPoolSize(int size) {  // ea: 0x8351F0
    if (jqPool.BatchGroupPool.Value)
        tlMemFree(jqPool.BatchGroupPool.Value);

    if (size > 0) {
        jqPool.BatchGroupPool.Value = (jqBatchGroup*)tlMemAlloc(size * sizeof(jqBatchGroup), 0, 0);
        for (int i = 0; i < size - 1; ++i)
            jqPool.BatchGroupPool.Value[i].Next = i + 1;
        if (size > 0)
            jqPool.BatchGroupPool.Value[size - 1].Next = -1;
        jqPool.BatchGroupPool.Head = 0;
    } else {
        jqPool.BatchGroupPool.Value = nullptr;
        jqPool.BatchGroupPool.Head = -1;
    }
    jqPool.BatchGroupPool.Size = size;
}

int jqCreateBatchGroup() {  // ea: 0x835320
    if (!jqPool.BatchPoolSize) return -1;

    int idx = jqPool.BatchGroupPool.Head;
    if (idx == -1) {
        _tlAssert("source/jobqueue.cpp", 0xE9, "", "");
        __debugbreak();
    }

    jqBatchGroup* g = &jqPool.BatchGroupPool.Value[idx];
    jqPool.BatchGroupPool.Head = g->Next;
    g->BatchCount = 0;
    g->NDependencies = 0;

    return idx;
}

void jqDestroyBatchGroup(int id) {  // ea: 0x835390
    if (id < 0 || id >= jqPool.BatchGroupPool.Size) return;

    jqBatchGroup* g = &jqPool.BatchGroupPool.Value[id];
    g->BatchCount = 0;
    g->NDependencies = 0;
    g->Next = jqPool.BatchGroupPool.Head;
    jqPool.BatchGroupPool.Head = id;
}

void jqAddBatchGroupDependency(int groupFrom, int groupTo) {  // ea: 0x835280
    if (groupFrom < 0 || groupTo < 0) return;
    jqPool.BatchGroupPool.Value[groupTo].NDependencies++;
}

void jqRemoveBatchGroupDependency(int groupFrom, int groupTo) {  // ea: 0x8352B0
    if (groupFrom < 0 || groupTo < 0) return;
    if (jqPool.BatchGroupPool.Value[groupTo].NDependencies > 0)
        jqPool.BatchGroupPool.Value[groupTo].NDependencies--;
}
