// ============================================================================
// PoolAllocator — fixed-size block pool allocator implementation
// Source: c:/cod/code/ae/core/PoolAllocator.cpp (line refs: 33, 137, 157, 183, 272)
// ea: 0x7BD510-0x7BDC90 (all PoolAllocator + BlockPool functions)
// ============================================================================

#include "PoolAllocator.h"
#include "ae_fixed_string.h"
#include <cstdio>     // snprintf
#include <cstring>    // memset
#include <new>        // placement new

// Portable debug break
#ifdef _MSC_VER
#define COD3_BREAK() __debugbreak()
#else
#define COD3_BREAK() __builtin_debugtrap()
#endif

// External dependencies (stubbed until mem_heap ported)
extern void* mem_heap_malloc(unsigned int size);
extern void  mem_heap_free(void* ptr);

// AE assertion system (extern — defined in ae_assert.cpp)
namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
    extern ECoderId gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int   gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* msg, ...);
    bool Error(const char* msg, ...);
}

// Tech library logging
extern void tlPrintf(const char* fmt, ...);

// ============================================================================
// PoolAllocator::BlockPool::BlockPool
// ea: 0x7BD510
// ============================================================================
PoolAllocator::BlockPool::BlockPool(
    unsigned int entrySize,
    unsigned int capacity,
    unsigned int id,
    unsigned int alignment,
    void* preallocatedBlock)
    : mAlignment(alignment)
    , mId(id)
    , mEntrySize(entrySize)
    , mCapacity(capacity)
    , mNumRemaining(capacity)
    , mMaxUsed(0)
    , mDebug(0)
{
    mBlockList.m_head = nullptr;

    // Alignment check
    unsigned int remainder = entrySize % mAlignment;
    if (remainder) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "PoolAllocator.cpp";
        AeAssert::gCurrentLine = 33;
        AeAssert::gCurrentExpr = 0;
        if (AeAssert::Error("cannot align %d to %d bytes", entrySize, mAlignment)) {
            COD3_BREAK();
        }
    }

    if (preallocatedBlock) {
        // Preallocated block: reserve some capacity for internal bookkeeping
        unsigned int overhead = 4 * capacity / entrySize;
        if (overhead <= 1) overhead = 1;
        mCapacity -= overhead;
        mNumRemaining = mCapacity;
        mBlockPtr = (char*)preallocatedBlock;
        mPreallocatedBlock = 1;
    } else {
        mBlockPtr = (char*)mem_heap_malloc(capacity * entrySize);
        mPreallocatedBlock = 0;
    }

    mBlockEnd = mBlockPtr + (capacity * entrySize);

    // Initialize free list with all blocks
    char* cur = mBlockPtr;
    for (unsigned int i = mCapacity; (int)(i - 1) >= 0; --i) {
        Block* block = (Block*)cur;
        block->mPoolId = (unsigned short)mId;
        block->mFlag = 0;
        // block->mLink has m_next at offset 4; clear it
        *(unsigned int*)(cur + 4) = 0;

        reserved_slist<Block>::slist_node* node = (reserved_slist<Block>::slist_node*)(cur + 4);
        node->m_next = mBlockList.m_head;
        mBlockList.m_head = node;

        cur += mEntrySize;
    }
}

// ============================================================================
// PoolAllocator::BlockPool::~BlockPool
// ea: 0x7BD420
// ============================================================================
PoolAllocator::BlockPool::~BlockPool() {
    if (!mPreallocatedBlock) {
        mem_heap_free(mBlockPtr);
    }
}

// ============================================================================
// PoolAllocator::BlockPool::Pop
// ea: 0x7BD670
// ============================================================================
PoolAllocator::BlockPool::Block* PoolAllocator::BlockPool::Pop() {
    reserved_slist<Block>::slist_node* head = mBlockList.m_head;
    if (!head) {
        return nullptr;
    }

    mBlockList.m_head = head->m_next;
    // Block starts sizeof(slist_node) bytes before the slist_node
    Block* block = (Block*)((char*)head - 4);

    unsigned int newRemaining = mNumRemaining - 1;
    mNumRemaining = newRemaining;

    // Assert: mNumRemaining <= mCapacity
    if (newRemaining > mCapacity) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "PoolAllocator.cpp";
        AeAssert::gCurrentLine = 137;
        AeAssert::gCurrentExpr = "mNumRemaining <= mCapacity";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Please add a descriptive string")) {
            COD3_BREAK();
        }
    }

    unsigned int used = mCapacity - mNumRemaining;
    if (mMaxUsed < used) {
        mMaxUsed = used;
    }

    return block;
}

// ============================================================================
// PoolAllocator::BlockPool::Push
// ea: 0x7BD700
// ============================================================================
void PoolAllocator::BlockPool::Push(Block* ptr) {
    // Validate pointer is within this pool's block range
    if ((char*)ptr < mBlockPtr || (char*)ptr > (mBlockPtr + mCapacity * mEntrySize)) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "PoolAllocator.cpp";
        AeAssert::gCurrentLine = 157;
        AeAssert::gCurrentExpr =
            "(ptr >= mBlockPtr) && ptr <= mBlockPtr + mCapacity * ( mEntrySize + tl_align(sizeof(BlockPool::Header),mAlignment) )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Block does not belong to this pool")) {
            COD3_BREAK();
        }
    }

    // Fill with 0xEF to catch use-after-free
    memset(ptr, 0xEF, mEntrySize);

    unsigned int newRemaining = mNumRemaining + 1;
    mNumRemaining = newRemaining;

    if (newRemaining > mCapacity) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "PoolAllocator.cpp";
        AeAssert::gCurrentLine = 183;
        AeAssert::gCurrentExpr = "mNumRemaining <= mCapacity";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("internal count mismatch")) {
            COD3_BREAK();
        }
    }

    // slist_node is at offset 4 within the block
    reserved_slist<Block>::slist_node* node = (reserved_slist<Block>::slist_node*)((char*)ptr + 4);
    node->m_next = mBlockList.m_head;
    mBlockList.m_head = node;
}

// ============================================================================
// PoolAllocator::PoolAllocator
// ea: 0x7BD830
// ============================================================================
PoolAllocator::PoolAllocator(const ae_sized_array<PoolConfig, 16>& cfgList, unsigned int flags)
    : mFlags(flags)
{
    mPoolArray.m_size = 0;
    mPoolSizes.m_size = 0;

    unsigned int sizeLastPool = 0;
    unsigned short idPool = 0;

    for (int i = 0; i < cfgList.m_size; ++i) {
        const PoolConfig& cfg = *(const PoolConfig*)((const char*)&cfgList + i * 16);

        // Assert: pools must be sorted by block size ascending
        if (cfg.blockSize <= sizeLastPool) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "PoolAllocator.cpp";
            AeAssert::gCurrentLine = 272;
            AeAssert::gCurrentExpr = "cfg.blockSize > size_last_pool";
            if (!AeAssert::IsIgnored() &&
                AeAssert::Assert("pool config list must be sorted by entry size, smallest to largest")) {
                COD3_BREAK();
            }
        }
        sizeLastPool = cfg.blockSize;

        BlockPool* pool = (BlockPool*)mem_heap_malloc(sizeof(BlockPool));
        if (pool) {
            new (pool) BlockPool(cfg.blockSize, cfg.numBlocks, idPool, cfg.blockAlign,
                                cfg.block);
        }

        // Add to pool array
        if (mPoolArray.m_size < MAX_POOLS) {
            mPoolArray.m_elements[mPoolArray.m_size] = pool;
            ++mPoolArray.m_size;
        } else {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:/cod/code/ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 174;
            AeAssert::gCurrentExpr = "m_size < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("no room left in array")) {
                COD3_BREAK();
            }
        }

        // Add to sizes array
        if (mPoolSizes.m_size < MAX_POOLS) {
            mPoolSizes.m_elements[mPoolSizes.m_size] = (short)cfg.blockSize;
            ++mPoolSizes.m_size;
        } else {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:/cod/code/ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 174;
            AeAssert::gCurrentExpr = "m_size < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("no room left in array")) {
                COD3_BREAK();
            }
        }

        ++idPool;
    }
}

// ============================================================================
// PoolAllocator::~PoolAllocator
// ea: 0x7BD9F0
// ============================================================================
PoolAllocator::~PoolAllocator() {
    for (int i = 0; i < mPoolArray.m_size; ++i) {
        BlockPool* pool = mPoolArray.m_elements[i];

        // Warn about unreleased allocations
        if (pool->mNumRemaining != pool->mCapacity) {
            tlPrintf("unreleased allocations for %d pool\n", pool->mEntrySize);
            tlPrintf("\tSize %d, Capacity %d, Used %d, Max %d\n",
                     pool->mEntrySize,
                     pool->mCapacity,
                     pool->mCapacity - pool->mNumRemaining,
                     pool->mMaxUsed);
        }

        if (!pool->mPreallocatedBlock) {
            mem_heap_free(pool->mBlockPtr);
        }
        pool->~BlockPool();
        mem_heap_free(pool);
    }
}

// ============================================================================
// PoolAllocator::Allocate
// ea: 0x7BDA70
// ============================================================================
void* PoolAllocator::Allocate(unsigned int size, bool forceHeapAlloc) {
    bool heapAlloc = forceHeapAlloc || (mFlags & 4) != 0;

    if (!heapAlloc && mPoolSizes.m_size > 0) {
        int foundAny = 0;

        for (int i = 0; i < mPoolSizes.m_size; ++i) {
            unsigned short poolEntrySize = mPoolSizes.m_elements[i];

            if (size <= poolEntrySize) {
                ++foundAny;

                BlockPool* pool = mPoolArray.m_elements[i];
                if (pool->GetNumRemaining() > 0) {
                    void* result = (void*)pool->Pop();
                    if (result) {
                        return result;
                    }
                }
            }
        }

        // No pool had available blocks
        if (!foundAny) {
            goto fallback_alloc;
        }

        // Pools had matching sizes but all exhausted
        if ((mFlags & 1) == 0) {
            extern void tlFatal(const char* fmt, ...);
            tlFatal("all pool exhausted");
            return nullptr;
        }
        return mem_heap_malloc(size);
    }

fallback_alloc:
    if (mFlags & 2) {
        return mem_heap_malloc(size);
    }
    extern void tlFatal(const char* fmt, ...);
    tlFatal("%d byte block does not fit into any pool", size);
    if (heapAlloc) {
        return mem_heap_malloc(size);
    }
    return nullptr;
}

// ============================================================================
// PoolAllocator::Release
// ea: 0x7BDB90
// ============================================================================
void PoolAllocator::Release(void* ptr) {
    if (!ptr) return;

    char* cptr = (char*)ptr;

    for (int i = 0; i < mPoolArray.m_size; ++i) {
        BlockPool* pool = mPoolArray.m_elements[i];

        if (cptr >= pool->mBlockPtr && cptr <= pool->mBlockEnd) {
            pool->Push((BlockPool::Block*)cptr);
            return;
        }
    }

    // Not in any pool — free from heap
    mem_heap_free(cptr);
}

// ============================================================================
// PoolAllocator::InPool
// ea: 0x7BDBE0
// ============================================================================
bool PoolAllocator::InPool(void* ptr) const {
    char* cptr = (char*)ptr;

    for (int i = 0; i < mPoolArray.m_size; ++i) {
        BlockPool* pool = mPoolArray.m_elements[i];
        if (cptr >= pool->mBlockPtr && cptr <= pool->mBlockEnd) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// PoolAllocator::GetMemSize
// ea: 0x7BD610
// ============================================================================
int PoolAllocator::GetMemSize() const {
    int total = 0;
    for (int i = 0; i < mPoolArray.m_size; ++i) {
        total += mPoolArray.m_elements[i]->mEntrySize * mPoolArray.m_elements[i]->mCapacity;
    }
    return total;
}

// ============================================================================
// PoolAllocator::GetMemRemaining
// ea: 0x7BD640
// ============================================================================
int PoolAllocator::GetMemRemaining() const {
    int total = 0;
    for (int i = 0; i < mPoolArray.m_size; ++i) {
        total += mPoolArray.m_elements[i]->mEntrySize * mPoolArray.m_elements[i]->mNumRemaining;
    }
    return total;
}

// ============================================================================
// PoolAllocator::ReportTotals
// ea: 0x7BDC20
// ============================================================================
void PoolAllocator::ReportTotals(ae_sized_array<ae_fixed_string<64, unsigned char>, 8>* strList) {
    char buf[64];
    for (int i = 0; i < mPoolArray.m_size; ++i) {
        BlockPool* pool = mPoolArray.m_elements[i];
        snprintf(buf, sizeof(buf), "\tSize %d, Capacity %d, Used %d, Max %d",
                 pool->mEntrySize,
                 pool->mCapacity,
                 pool->mCapacity - pool->mNumRemaining,
                 pool->mMaxUsed);

        // Copy into ae_fixed_string
        ae_fixed_string<64, unsigned char> str;
        str.mLength = (unsigned char)(snprintf((char*)str.mBuff, str.capacity(), "%s", buf) & 0xFF);
        strList->push_back(str);
    }
}

// ============================================================================
// PoolAllocator::ReportAllocations
// ea: 0x7BDC90 (dispatches to per-pool)
// ============================================================================
void PoolAllocator::ReportAllocations() {
    for (int i = 0; i < mPoolArray.m_size; ++i) {
        mPoolArray.m_elements[i]->ReportAllocations();
    }
}

void PoolAllocator::BlockPool::ReportAllocations() const {
    tlPrintf("\tSize %d, Capacity %d, Used %d, Max %d\n",
             mEntrySize,
             mCapacity,
             mCapacity - mNumRemaining,
             mMaxUsed);
}
