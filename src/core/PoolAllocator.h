// ============================================================================
// PoolAllocator — fixed-size block pool allocator
// Source: c:/cod/code/ae/core/PoolAllocator.cpp (line refs: 33, 137, 157, 183, 272)
// Size: 60 bytes (0x3C) — verified against IDA
// ea: 0x7BD830 (constructor), 0x7BDA70 (Allocate), 0x7BDB90 (Release)
// ============================================================================

#pragma once

#include "ae_array.h"
#include <stdint.h>

// Forward
struct mem_heap;

// ============================================================================
// PoolAllocator — block pool allocator
// Manages multiple BlockPools of increasing block size.
// Flags:
//   bit 0 (1) — allow heap fallback when pool exhausted
//   bit 1 (2) — fatal error on mis-sized allocation
//   bit 2 (4) — force heap allocation
// ============================================================================
class PoolAllocator {
public:
    enum {
        MAX_POOLS = 8,
    };

    // Configuration for one pool level
    struct PoolConfig {
        unsigned short  blockSize;    // entry size in bytes
        unsigned short  numBlocks;    // number of blocks
        unsigned short  blockAlign;   // alignment requirement
        char*           block;        // preallocated block (or nullptr)
    };

    // Internal per-pool free list manager
    struct BlockPool {
        // Block header (inline in each allocated block)
        struct Block {
            unsigned short  mPoolId;     // +0x00 — pool identifier
            unsigned short  mFlag;       // +0x02 — flags
            reserved_slist<Block>::slist_node mLink;  // +0x04 — free list node (intrusive)
        };

        unsigned int    mAlignment;      // +0x00
        unsigned int    mId;             // +0x04
        unsigned int    mEntrySize;      // +0x08
        unsigned int    mCapacity;       // +0x0C
        unsigned int    mNumRemaining;   // +0x10
        unsigned int    mMaxUsed;        // +0x14
        unsigned int    mDebug;          // +0x18 (unused in release)
        char*           mBlockPtr;       // +0x1C
        char*           mBlockEnd;       // +0x20
        unsigned int    mPreallocatedBlock; // +0x24
        reserved_slist<Block> mBlockList;   // +0x28

        unsigned int    GetCapacity() const { return mCapacity; }
        unsigned int    GetNumRemaining() const { return mNumRemaining; }

        // Pop a free block from the free list. Returns nullptr if empty.
        Block* Pop();

        // Push a block back onto the free list.
        void Push(Block* ptr);

        // Construction / destruction
        BlockPool(
            unsigned int entrySize,
            unsigned int capacity,
            unsigned int id,
            unsigned int alignment,
            char* preallocatedBlock);

        ~BlockPool();

        void ReportAllocations() const;
    };

    // ================================================================
    // Members (60 bytes total, verified against IDA)
    // ================================================================
    ae_sized_array<BlockPool*, MAX_POOLS> mPoolArray;  // +0x00 (36 bytes)
    ae_sized_array<short, MAX_POOLS>      mPoolSizes;  // +0x24 (20 bytes)
    unsigned int                          mFlags;       // +0x38

    // ================================================================
    // Public API
    // ================================================================

    // Construct with pool configurations (must be sorted by block size).
    PoolAllocator(const ae_sized_array<PoolConfig, 16>& cfgList, unsigned int flags);

    ~PoolAllocator();

    // Allocate a block. Falls back to heap if no pool fits (or flags permit).
    void* Allocate(unsigned int size, bool forceHeapAlloc = false);

    // Release a previously allocated block back to its pool.
    void Release(void* ptr);

    // Check if a pointer belongs to any pool managed by this allocator.
    bool InPool(void* ptr) const;

    // Diagnostic reporting
    void ReportAllocations();
    void ReportTotals(ae_sized_array<ae_fixed_string<64, unsigned char>, 8>* strList);

    // Check if pool has remaining capacity
    bool IsEmpty() const { return mPoolArray.m_size == 0; }
    int  GetMemSize() const;
    int  GetMemRemaining() const;
};
