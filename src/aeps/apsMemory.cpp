// ============================================================================
// apsMemory.cpp — APS memory pool subsystem (48 non-inline funcs).
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsMemory.cpp
//
// Port strategy (matches apsAction.o / apsCommon.o precedent):
//   - All 48 non-inline functions verified against IDA disasm.
//   - apsArray<T> accessors, apsSingleton, BlockAllocator virtuals are inline
//     COMDATs in the headers, emitted per codmp_xboxr.map.
// ============================================================================
#include "apsMemory.h"
#include "apsError.h"    // apsError, AEPS_VECTOR_NEW
#include "apsDebug.h"    // apsDebug::Print/PrintWarning

#include <new>
#include <stdio.h>

// tl_system.o (tl_xboxr, ported)
extern void tlPrintf(const char* fmt, ...);

// apsEffect.o (unported): pool init/term — static methods.
class apsEffect {
public:
    static void InitPool(int maxEffects);   // ?InitPool@apsEffect@@SAXH@Z
    static void TermPool();                 // ?TermPool@apsEffect@@SAXXZ
};

// apsGroupMgr.o (unported): group manager singleton.
class apsGroupManager {
public:
    apsGroupManager(int maxGroups);         // ??0apsGroupManager@@QAE@H@Z
    ~apsGroupManager();                     // ??1apsGroupManager@@QAE@XZ
};

// ============================================================================
// Data (apsMemory.o)
// ============================================================================
apsMemory::Config apsMemory::gConfig;   // @0xE49684 (default ctor runs)

// apsSingleton<apsMemory::BlockManager>::InstancePtr() (explicit instantiation below)

// ============================================================================
// apsMemory::Pool::Init — set up the block array + free list.
// ea: 0x7EC1E0
// ============================================================================
void apsMemory::Pool::Init(int numBlocks, int blockSize, unsigned char* blockData) {
    mNumBlocks = numBlocks;
    mBlockSize = blockSize;
    mBlockData = blockData;
    if (numBlocks != 0) {
        if (mBlocks == 0) {
            PoolBlock* p = AEPS_VECTOR_NEW<Pool::PoolBlock>(numBlocks, 4);
            mBlocks = p;
            if (p == 0 &&
                _tlAssert("source/apsMemory.cpp", 269, "mBlocks", "alloc failed"))
                __debugbreak();
        }
        for (int i = 0; i < mNumBlocks; ++i) {
            mBlocks[i].mIndex = (short)i;
            mBlocks[i].mNext = (short)(i + 1);
            mBlocks[i].SetSize(0);
            mBlocks[i].SetFlag(PoolBlock::FLAG_FREE);
        }
        mBlocks[mNumBlocks - 1].mNext = -1;
        mFreeListHead = 0;
    }
    apsDebug::Print("pool initialised, %d blocks of %d bytes\n", mNumBlocks, mBlockSize);
}

// ============================================================================
// apsMemory::Pool::Term — free the block array (asserts all blocks released).
// ea: 0x7EC2A0
// ============================================================================
void apsMemory::Pool::Term() {
    if (mNumBlocksUsed != 0 &&
        _tlAssert("source/apsMemory.cpp", 300, "0 == NumBlocksUsed()",
                  "Destroying pool with allocated blocks"))
        __debugbreak();
    PoolBlock* mBlocks = this->mBlocks;
    if (mBlocks != 0) {
        if (this + 1 != (Pool*)mBlocks) {
            apsCommon::GetAllocator()->MemFree(mBlocks);
        }
        this->mBlocks = 0;
    }
    mBlockData = 0;
}

// ============================================================================
// apsMemory::Pool::Alloc — pop a free block, record size stats.
// ea: 0x7EC300
// ============================================================================
void* apsMemory::Pool::Alloc(int size) {
    int mFreeListHead = this->mFreeListHead;
    if (mFreeListHead == -1) {
        if (apsSingleton<apsError>::InstancePtr() == 0 &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        apsSingleton<apsError>::InstancePtr()->AddError(apsError::ERROR_TYPE_WARNING,
            "apsMemory : pool(size=%d, count=%d) is empty", mBlockSize, mNumBlocks);
        return 0;
    }
    PoolBlock* pBlock = &mBlocks[mFreeListHead];
    if (mFreeListHead != pBlock->mIndex &&
        _tlAssert("source/apsMemory.cpp", 334, "nBlock == pBlock->mIndex", "block mismatch"))
        __debugbreak();
    if (pBlock->GetFlag() != PoolBlock::FLAG_FREE &&
        _tlAssert("source/apsMemory.cpp", 335, "PoolBlock::FLAG_FREE == pBlock->mFlag",
                  "block already allocated"))
        __debugbreak();

    mFreeListHead = pBlock->mNext;
    pBlock->SetSize(size);
    pBlock->mNext = -1;
    pBlock->SetFlag(PoolBlock::FLAG_USED);
    ++mNumBlocksUsed;
    if (mNumBlocksUsed > mMaxBlocksUsed)
        mMaxBlocksUsed = mNumBlocksUsed;
    if (size < mMinBlockSize)
        mMinBlockSize = size;
    if (mMaxBlockSize < size)
        mMaxBlockSize = size;
    return &mBlockData[mFreeListHead * mBlockSize];
}

// ============================================================================
// apsMemory::Pool::Free — return a block to the free list.
// ea: 0x7EBDE0
// ============================================================================
void apsMemory::Pool::Free(void* pBlockData) {
    if (pBlockData != 0) {
        int nBlock = ((int*)pBlockData - (int*)mBlockData) / mBlockSize;
        if ((nBlock < 0 || nBlock >= mNumBlocks) &&
            _tlAssert("source/apsMemory.cpp", 411,
                      "((nBlock >= 0) && (nBlock < mNumBlocks))", "bad block"))
            __debugbreak();
        PoolBlock* pBlock = &mBlocks[nBlock];
        if (nBlock != pBlock->mIndex &&
            _tlAssert("source/apsMemory.cpp", 415, "nBlock == pBlock->mIndex", "block mismatch"))
            __debugbreak();
        if (pBlock->GetFlag() != PoolBlock::FLAG_USED &&
            _tlAssert("source/apsMemory.cpp", 416, "PoolBlock::FLAG_USED == pBlock->mFlag",
                      "block not allocated"))
            __debugbreak();
        pBlock->mNext = (short)mFreeListHead;
        mFreeListHead = (short)nBlock;
        pBlock->SetSize(0);
        pBlock->SetFlag(PoolBlock::FLAG_FREE);
        --mNumBlocksUsed;
    }
}

// ============================================================================
// apsMemory::Pool::Resize — change a block's recorded size (in place).
// ea: 0x7EBCE0
// ============================================================================
void* apsMemory::Pool::Resize(void* pBlockData, int newSize) {
    if (newSize <= mBlockSize) {
        int nBlock = ((int*)pBlockData - (int*)mBlockData) / mBlockSize;
        if ((nBlock < 0 || nBlock >= mNumBlocks) &&
            _tlAssert("source/apsMemory.cpp", 377,
                      "((nBlock >= 0) && (nBlock < mNumBlocks))", "bad block"))
            __debugbreak();
        PoolBlock* pBlock = &mBlocks[nBlock];
        if (nBlock != pBlock->mIndex &&
            _tlAssert("source/apsMemory.cpp", 381, "nBlock == pBlock->mIndex", "block mismatch"))
            __debugbreak();
        if (pBlock->GetFlag() != PoolBlock::FLAG_USED &&
            _tlAssert("source/apsMemory.cpp", 382, "PoolBlock::FLAG_USED == pBlock->mFlag",
                      "block not allocated"))
            __debugbreak();
        int oldSize = pBlock->GetSize();
        if (newSize == oldSize) {
            apsDebug::PrintWarning(apsDebug::YELLOW_ALERT,
                "failed resize block(%08x) size(%d)\n", nBlock, newSize);
            return 0;
        }
        pBlock->SetSize(newSize);
        return pBlockData;
    }
    apsDebug::PrintWarning(apsDebug::YELLOW_ALERT,
        "failed resize blocksize(%d) size(%d)\n", mBlockSize, newSize);
    return 0;
}

// ============================================================================
// apsMemory::Pool::Contains — is pBlockData within this pool's data range?
// ea: 0x7EBEA0
// ============================================================================
unsigned int apsMemory::Pool::Contains(void* pBlockData) {
    if (mBlockData == 0)
        return 0;
    return ((unsigned char*)pBlockData - mBlockData >= 0) &&
           (&mBlockData[mBlockSize * (mNumBlocks - 1)] - (unsigned char*)pBlockData >= 0);
}

// ============================================================================
// apsMemory::Pool::Verify — free-list sanity check.
// ea: 0x7EBEE0
// ============================================================================
void apsMemory::Pool::Verify() {
    int iBlock = mFreeListHead;
    int nFree = 0;
    for (; iBlock != -1; iBlock = mBlocks[iBlock].mNext) {
        ++nFree;
        if (mBlocks[iBlock].GetFlag() != 0 &&
            _tlAssert("source/apsMemory.cpp", 464, "0 == mBlocks[iBlock].mFlag",
                      "free block is allocated"))
            __debugbreak();
    }
    if (nFree != mNumBlocks - mNumBlocksUsed &&
        _tlAssert("source/apsMemory.cpp", 468, "nFree == (mNumBlocks - mNumBlocksUsed)",
                  "bad free list"))
        __debugbreak();
}

// ============================================================================
// apsMemory::Pool::GetActiveBlock — return block data if index is used.
// ea: 0x7EBF60
// ============================================================================
void* apsMemory::Pool::GetActiveBlock(int index) {
    if ((index < 0 || index >= mNumBlocks) &&
        _tlAssert("source/apsMemory.cpp", 476,
                  "(index >= 0) && (index < NumBlocks() )", "invalid block index"))
        __debugbreak();
    if (mBlocks[index].GetFlag() == PoolBlock::FLAG_USED)
        return &mBlockData[index * mBlockSize];
    return 0;
}

// ============================================================================
// apsMemory::Pool::GetStats — fill a Stats struct.
// ea: 0x7EBFC0
// ============================================================================
void apsMemory::Pool::GetStats(Stats& stats) {
    memset(&stats, 0, sizeof(Stats));
    if (mNumBlocks != 0) {
        stats.numBlocks = mNumBlocks;
        stats.numUsedBlocks = mNumBlocksUsed;
        stats.numUsedBlocksPercent = 100 * mNumBlocksUsed / stats.numBlocks;
        int numFree = mNumBlocks - mNumBlocksUsed;
        stats.numFreeBlocks = numFree;
        stats.numFreeBlocksPercent = 100 * numFree / stats.numBlocks;
        stats.maxUsedBlocks = mMaxBlocksUsed;
        stats.maxUsedBlocksPercent = 100 * mMaxBlocksUsed / stats.numBlocks;
        stats.blockSize = mBlockSize;
        stats.minBlockSize = (mMinBlockSize == 0x7FFFFFFF) ? 0 : mMinBlockSize;
        stats.maxBlockSize = (mMaxBlockSize == -1) ? 0 : mMaxBlockSize;
        int usedBytes = mNumBlocksUsed * mBlockSize;
        int totalBytes = mNumBlocks * mBlockSize;
        stats.memoryUsed = usedBytes;
        stats.memoryTotal = totalBytes;
        stats.memoryUsedPercent = 100 * usedBytes / totalBytes;
        stats.memoryFree = totalBytes - usedBytes;
        stats.memoryFreePercent = 100 * stats.memoryFree / totalBytes;
        int waste = 0;
        for (int i = 0; i < mNumBlocks; ++i) {
            if (mBlocks[i].GetFlag() == PoolBlock::FLAG_USED)
                waste += mBlockSize - mBlocks[i].GetSize();
        }
        stats.memoryWaste = waste;
        stats.wastePercent = (usedBytes != 0) ? (100 * waste / usedBytes) : 0;
        stats.wastePercentTotal = 100 * waste / stats.memoryTotal;
    }
}

// ============================================================================
// apsMemory::Pool::Report — print pool stats.
// ea: 0x7EC0E0
// ============================================================================
void apsMemory::Pool::Report() {
    Stats stats;
    GetStats(stats);
    tlPrintf("Pool\n");
    tlPrintf("num blocks,             %10d\n"
             "block size (bytes),     %10d\n"
             "min block size (bytes), %10d\n"
             "max block size (bytes), %10d\n",
             stats.numBlocks, stats.blockSize, stats.minBlockSize, stats.maxBlockSize);
    tlPrintf("blocks used,    %10d, %d percent\n"
             "blocks free,    %10d, %d percent\n"
             "blocks peak,    %10d, %d percent\n",
             stats.numUsedBlocks, stats.numUsedBlocksPercent,
             stats.numFreeBlocks, stats.numFreeBlocksPercent,
             stats.maxUsedBlocks, stats.maxUsedBlocksPercent);
    tlPrintf("memory total (bytes),   %10d\n"
             "memory used (bytes),    %10d\n"
             "memory free (bytes),    %10d\n",
             stats.memoryTotal, stats.memoryUsed, stats.memoryFree);
    tlPrintf("memory wasted (bytes),    %10d\n"
             "waste of used (percent),  %10d\n"
             "waste of total (percent), %10d\n",
             stats.memoryWaste, stats.wastePercent, stats.wastePercentTotal);
    tlPrintf("\n\n");
}

// ============================================================================
// apsMemory::Pool::Pool / ~Pool
// ea: 0x7EBCB0 / 0x7EC920
// ============================================================================
apsMemory::Pool::Pool(PoolBlock* pBlocks) {
    mNumBlocks = 0;
    mNumBlocksUsed = 0;
    mMaxBlocksUsed = 0;
    mBlockSize = 0;
    mBlockData = 0;
    mMinBlockSize = 0x7FFFFFFF;
    mMaxBlockSize = -1;
    mBlocks = pBlocks;
    mFreeListHead = -1;
}

apsMemory::Pool::~Pool() {
    Term();
}

// ============================================================================
// apsMemory::Pool::CalcClassOverhead / CalcDataOverhead
// ea: 0x7EBC70 / 0x7EBC90
// ============================================================================
int apsMemory::Pool::CalcClassOverhead(int numBlocks, int blockSize) {
    return (8 * numBlocks + 163) & 0xFFFFFF80;
}

int apsMemory::Pool::CalcDataOverhead(int numBlocks, int blockSize) {
    return (blockSize * numBlocks + 127) & 0xFFFFFF80;
}

// ============================================================================
// apsMemory::Config defaults (inline in header). SetBufferSize etc.
// ============================================================================
void apsMemory::SetBufferSize(int bufferSize) { gConfig.mBufferSize = bufferSize; }
void apsMemory::SetMaxGroups(int maxGroups) { gConfig.mMaxGroups = maxGroups; }
void apsMemory::SetMaxEffects(int maxEffects) { gConfig.mMaxEffects = maxEffects; }
void apsMemory::SetMaxModifiers(int maxModifiers) { gConfig.mMaxModifiers = maxModifiers; }

// ============================================================================
// apsMemory::BlockManager — constructor.
// ea: 0x7EC930
// ============================================================================
apsMemory::BlockManager::BlockManager(int memSize, int maxPools) {
    if (apsSingleton<apsMemory::BlockManager>::sInstancePtr != 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 86,
                  "0 == sInstancePtr", "singleton already initialised"))
        __debugbreak();
    apsSingleton<apsMemory::BlockManager>::sInstancePtr = this;
    mPools.mElements = 0;
    mPools.mCapacity = 0;
    mPools.mSize = 0;
    mPools.reserve(maxPools);
    mPools.resize(0);
    mMaxRequestedSize = 0;
    mAllocFailures = 0;
    mAllocSizeFailure = 0;
    mAllocSuccesses = 0;
    mAllocSizeSuccess = 0;
}

// ============================================================================
// apsMemory::BlockManager::AddPool — allocate a pool in one aligned block.
// ea: 0x7EC9A0
// ============================================================================
unsigned int apsMemory::BlockManager::AddPool(int numBlocks, int blockSize) {
    int n = (int)numBlocks;
    int dataBytes = blockSize * n;
    if (dataBytes <= 0)
        return 0;
    unsigned int classSize = (8 * n + 163) & 0xFFFFFF80;
    unsigned int total = classSize + ((dataBytes + 127) & 0xFFFFFF80);
    Pool* pool = (Pool*)apsCommon::GetAllocator()->MemAlign(total, 16);
    if (pool != 0) {
        pool->mNumBlocks = 0;
        pool->mNumBlocksUsed = 0;
        pool->mMaxBlocksUsed = 0;
        pool->mBlockSize = 0;
        pool->mBlockData = 0;
        pool->mMaxBlockSize = -1;
        pool->mFreeListHead = -1;
        pool->mBlocks = (Pool::PoolBlock*)(pool + 1);
        pool->mMinBlockSize = 0x7FFFFFFF;
        pool->Init(n, blockSize, (unsigned char*)pool + classSize);
        Pool* pp = pool;
        mPools.push_back(pp);
        return 1;
    }
    if (_tlAssert("source/apsMemory.cpp", 685, "0",
                  "AddPool failed, not enough Aeps memory"))
        __debugbreak();
    return 0;
}

// ============================================================================
// apsMemory::BlockManager::DestroyPools — tear down all pools.
// ea: 0x7ECA70
// ============================================================================
void apsMemory::BlockManager::DestroyPools() {
    Pool** first = mPools.mElements;
    Pool** last = &mPools.mElements[mPools.mSize];
    if (mPools.mElements != last) {
        do {
            Pool* pool = *first;
            if (pool->mNumBlocksUsed != 0 &&
                _tlAssert("source/apsMemory.cpp", 300, "0 == NumBlocksUsed()",
                          "Destroying pool with allocated blocks"))
                __debugbreak();
            Pool::PoolBlock* blocks = pool->mBlocks;
            if (blocks != 0) {
                if (pool + 1 != (Pool*)blocks)
                    apsCommon::GetAllocator()->MemFree(blocks);
                pool->mBlocks = 0;
            }
            pool->mBlockData = 0;
            apsCommon::GetAllocator()->MemFree(pool);
            ++first;
        } while (first != last);
    }
    mPools.resize(0);
}

// ============================================================================
// apsMemory::BlockManager::Reset — thunk to DestroyPools.
// ea: 0x7ECCB0
// ============================================================================
void apsMemory::BlockManager::Reset() {
    DestroyPools();
}

// ============================================================================
// apsMemory::BlockManager::~BlockManager — destroy pools + free mPools.
// ea: 0x7ECC30
// ============================================================================
apsMemory::BlockManager::~BlockManager() {
    DestroyPools();
    if (mPools.mElements != 0) {
        int old = apsCommon::SetPakAllocs(0);
        apsCommon::GetAllocator()->MemFree(mPools.mElements);
        apsCommon::SetPakAllocs(old);
        mPools.mElements = 0;
        mPools.mCapacity = 0;
        mPools.mSize = 0;
    }
    if (apsSingleton<apsMemory::BlockManager>::sInstancePtr != 0) {
        apsSingleton<apsMemory::BlockManager>::sInstancePtr = 0;
    } else {
        if (_tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 104,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        apsSingleton<apsMemory::BlockManager>::sInstancePtr = 0;
    }
}

// ============================================================================
// apsMemory::BlockManager::TestAlloc — can `size` fit a non-full pool?
// ea: 0x7EC420
// ============================================================================
unsigned int apsMemory::BlockManager::TestAlloc(int size) {
    if (size == 0)
        return 0;
    Pool** first = mPools.mElements;
    Pool** last = &mPools.mElements[mPools.mSize];
    Pool* best = 0;
    int closest = 0x40000000;
    if (mPools.mElements == last)
        return 0;
    for (;;) {
        Pool* pool = *first;
        int delta = pool->mBlockSize - size;
        if (delta == 0) {
            best = pool;
            break;
        }
        if (delta >= 0 && delta < closest) {
            closest = delta;
            best = pool;
        }
        if (++first == last)
            return best != 0 && best->mNumBlocks != best->mNumBlocksUsed;
    }
    return best != 0 && best->mNumBlocks != best->mNumBlocksUsed;
}

// ============================================================================
// apsMemory::BlockManager::Alloc — best-fit pool allocation.
// ea: 0x7EC490
// ============================================================================
void* apsMemory::BlockManager::Alloc(int size, int alignment) {
    if (size == 0)
        return 0;
    if (size > mMaxRequestedSize)
        mMaxRequestedSize = size;
    Pool** first = mPools.mElements;
    Pool** last = &mPools.mElements[mPools.mSize];
    Pool* best = 0;
    int closest = 0x40000000;
    Pool** it = first;
    if (mPools.mElements == last)
        return 0;
    do {
        Pool* pool = *it;
        int delta = pool->mBlockSize - size;
        if (delta != 0) {
            if (delta >= 0) {
                if (delta < closest) {
                    closest = delta;
                    best = pool;
                }
            }
        } else if (best == 0 || best->mNumBlocks == best->mNumBlocksUsed) {
            best = pool;
        }
        ++it;
    } while (it != last);
    if (best == 0)
        return 0;
    void* result = best->Alloc(size);
    if (result != 0) {
        ++mAllocSuccesses;
        mAllocSizeSuccess += size;
    } else {
        ++mAllocFailures;
        mAllocSizeFailure += size;
    }
    return result;
}

// ============================================================================
// apsMemory::BlockManager::Free — find the pool containing pBlock, free it.
// ea: 0x7EC540
// ============================================================================
unsigned int apsMemory::BlockManager::Free(void* pBlock) {
    if (pBlock == 0)
        return 0;
    Pool** first = mPools.mElements;
    Pool** last = &mPools.mElements[mPools.mSize];
    if (mPools.mElements == last)
        return 0;
    for (;;) {
        Pool* pool = *first;
        if (pool->mBlockData != 0 &&
            ((unsigned char*)pBlock - pool->mBlockData >= 0) &&
            (&pool->mBlockData[pool->mBlockSize * (pool->mNumBlocks - 1)] - (unsigned char*)pBlock >= 0))
            break;
        if (++first == last)
            return 0;
    }
    (*first)->Free(pBlock);
    return 1;
}

// ============================================================================
// apsMemory::BlockManager::Resize — find pool + resize block.
// ea: 0x7EC5B0
// ============================================================================
void* apsMemory::BlockManager::Resize(void* pBlock, int newSize) {
    if (pBlock == 0)
        return 0;
    Pool** first = mPools.mElements;
    Pool** last = &mPools.mElements[mPools.mSize];
    if (mPools.mElements == last)
        return 0;
    for (;;) {
        Pool* pool = *first;
        if (pool->mBlockData != 0 &&
            ((unsigned char*)pBlock - pool->mBlockData >= 0) &&
            (&pool->mBlockData[pool->mBlockSize * (pool->mNumBlocks - 1)] - (unsigned char*)pBlock >= 0))
            break;
        if (++first == last)
            return 0;
    }
    return (*first)->Resize(pBlock, newSize);
}

// ============================================================================
// apsMemory::BlockManager::Report / AllocBlock / FreeBlock
// ============================================================================
void apsMemory::BlockManager::Report() {
    tlPrintf("max requested block size (bytes), %d\n", mMaxRequestedSize);
    tlPrintf("\nPools\n\n");
    Pool** first = mPools.mElements;
    Pool** last = &mPools.mElements[mPools.mSize];
    while (first != last)
        (*first++)->Report();
    mAllocFailures = 0;
    mAllocSizeFailure = 0;
    mAllocSuccesses = 0;
    mAllocSizeSuccess = 0;
}

void* apsMemory::BlockManager::AllocBlock(int blockSize, int alignment) {
    return apsCommon::GetAllocator()->MemAlign(blockSize, alignment);
}

void apsMemory::BlockManager::FreeBlock(void* data) {
    if (data != 0)
        apsCommon::GetAllocator()->MemFree(data);
}

// ============================================================================
// apsMemory::BlockAllocator — delegates to the BlockManager.
// ============================================================================
apsMemory::BlockAllocator::BlockAllocator() {
    if (apsSingleton<apsMemory::BlockAllocator>::sInstancePtr != 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 86,
                  "0 == sInstancePtr", "singleton already initialised"))
        __debugbreak();
    apsSingleton<apsMemory::BlockAllocator>::sInstancePtr = this;
}

void* apsMemory::BlockAllocator::MemAlign(unsigned int size, unsigned int alignment) {
    if (size == 0)
        return 0;
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsSingleton<apsMemory::BlockManager>::InstancePtr()->Alloc(size, alignment);
}

void apsMemory::BlockAllocator::MemFree(void* ptr) {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    apsSingleton<apsMemory::BlockManager>::InstancePtr()->Free((unsigned char*)ptr);
}

void* apsMemory::BlockAllocator::MemResize(void* ptr, unsigned int size, unsigned int alignment) {
    if (size == 0)
        return 0;
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsSingleton<apsMemory::BlockManager>::InstancePtr()->Resize((unsigned char*)ptr, size);
}

// ============================================================================
// apsMemory global API
// ============================================================================
unsigned int apsMemory::AddPool(int numBlocks, int blockSize) {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsSingleton<apsMemory::BlockManager>::InstancePtr()->AddPool(numBlocks, blockSize);
}

unsigned int apsMemory::AddParticlePool(int numBlocks, int blockSize) {
    return 0;
}

void apsMemory::AddPools() {
}

void apsMemory::Init() {
    apsAllocator* alloc = apsCommon::GetAllocator();
    BlockManager* bm = (BlockManager*)alloc->MemAlign(28, 4);
    if (bm != 0)
        new (bm) BlockManager(gConfig.mBufferSize, 10);

    BlockAllocator* ba = (BlockAllocator*)apsCommon::GetAllocator()->MemAlign(4, 4);
    if (ba != 0) {
        new (ba) BlockAllocator();
    }
    apsCommon::SetBlockAllocator(apsSingleton<apsMemory::BlockAllocator>::InstancePtr());

    apsGroupManager* gm = (apsGroupManager*)apsCommon::GetAllocator()->MemAlign(12, 4);
    if (gm != 0)
        new (gm) apsGroupManager(gConfig.mMaxGroups);
    apsEffect::InitPool(gConfig.mMaxEffects);

    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0) {
        if (_tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
    }
}

void apsMemory::Reset() {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    apsSingleton<apsMemory::BlockManager>::InstancePtr()->DestroyPools();
}void apsMemory::Term() {
    apsEffect::TermPool();
    if (apsSingleton<apsGroupManager>::InstancePtr() != 0) {
        apsSingleton<apsGroupManager>::InstancePtr()->~apsGroupManager();
        apsCommon::GetAllocator()->MemFree(apsSingleton<apsGroupManager>::InstancePtr());
    }
    apsCommon::SetBlockAllocator(0);
    if (apsSingleton<apsMemory::BlockAllocator>::InstancePtr() != 0) {
        apsDestroy(apsSingleton<apsMemory::BlockAllocator>::InstancePtr());
        apsCommon::GetAllocator()->MemFree(apsSingleton<apsMemory::BlockAllocator>::InstancePtr());
    }
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() != 0) {
        apsDestroy(apsSingleton<apsMemory::BlockManager>::InstancePtr());
        apsCommon::GetAllocator()->MemFree(apsSingleton<apsMemory::BlockManager>::InstancePtr());
    }
}

void apsMemory::Report() {
    tlPrintf("Memory\n\n");
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    apsSingleton<apsMemory::BlockManager>::InstancePtr()->Report();
}

unsigned int apsMemory::GetPoolInfo(int nPool, int& size, int& capacity, int& used, int& peak) {
    if (nPool < 0)
        return 0;
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    if (nPool >= apsSingleton<apsMemory::BlockManager>::InstancePtr()->mPools.mSize)
        return 0;
    Pool::Stats stats;
    apsSingleton<apsMemory::BlockManager>::InstancePtr()->mPools[nPool]->GetStats(stats);
    size = stats.blockSize;
    capacity = stats.numBlocks;
    used = stats.numUsedBlocks;
    peak = stats.maxUsedBlocks;
    return 1;
}

int apsMemory::GetMaxRequestedBlockSize() {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() != 0)
        return apsSingleton<apsMemory::BlockManager>::InstancePtr()->mMaxRequestedSize;
    if (_tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsSingleton<apsMemory::BlockManager>::InstancePtr()->mMaxRequestedSize;
}

unsigned int apsMemory::TestAlloc(int size) {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsSingleton<apsMemory::BlockManager>::InstancePtr()->TestAlloc(size);
}

void* apsMemory::Alloc(int size, int alignment) {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsCommon::GetAllocator()->MemAlign(size, alignment);
}

void apsMemory::Free(void* data) {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    if (data != 0)
        apsCommon::GetAllocator()->MemFree(data);
}

unsigned int apsMemory::TestAllocFromPools(int size) {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsSingleton<apsMemory::BlockManager>::InstancePtr()->TestAlloc(size);
}

void* apsMemory::AllocFromPools(int size, int alignment) {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsSingleton<apsMemory::BlockManager>::InstancePtr()->Alloc(size, alignment);
}

unsigned int apsMemory::FreeFromPools(void* data) {
    if (apsSingleton<apsMemory::BlockManager>::InstancePtr() == 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                  "sInstancePtr", "singleton not initialised"))
        __debugbreak();
    return apsSingleton<apsMemory::BlockManager>::InstancePtr()->Free((unsigned char*)data);
}

void apsMemory::SetBlockAllocator() {
    apsCommon::SetBlockAllocator(apsSingleton<apsMemory::BlockAllocator>::InstancePtr());
}

void apsMemory::ClearBlockAllocator() {
    apsCommon::SetBlockAllocator(0);
}

apsAllocator* apsMemory::GetBlockAllocatorDirect() {
    return apsSingleton<apsMemory::BlockAllocator>::InstancePtr();
}

// ============================================================================
// apsSingleton explicit instantiations (emit sInstancePtr in apsMemory.o).
// ============================================================================
template class apsSingleton<apsMemory::BlockManager>;
template class apsSingleton<apsMemory::BlockAllocator>;

// apsSingleton<apsGroupManager>::InstancePtr() is owned by apsGroupMgr.o
// (unported); referenced extern here, resolved by /FORCE:UNRESOLVED for now.
extern template class apsSingleton<apsGroupManager>;
