// ============================================================================
// apsMemory — APS memory pool subsystem (48 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsMemory.cpp
// Verified against IDA (aeps_xboxr:apsMemory.o).
//
// Layouts verified against IDA:
//   Pool (36 bytes): mNumBlocks@0, mNumBlocksUsed@4, mMaxBlocksUsed@8,
//     mMinBlockSize@0xC, mMaxBlockSize@0x10, mBlockSize@0x14,
//     mBlockData@0x18, mBlocks@0x1C, mFreeListHead@0x20 (short).
//   Pool::PoolBlock (8 bytes): mIndex@0 (short), mNext@2 (short),
//     mSize@4 (24-bit), mFlag@7 (8-bit).
//   BlockManager (28 bytes): apsSingleton base, mPools@0 (apsArray 8),
//     mAllocSizeSuccess@8, mAllocSizeFailure@0xC, mAllocSuccesses@0x10,
//     mAllocFailures@0x14, mMaxRequestedSize@0x18.
//   Config (12 bytes): mBufferSize@0, mMaxGroups@4, mMaxEffects@6, mMaxModifiers@8.
// ============================================================================
#ifndef COD3_AEPS_APSMEMORY_H
#define COD3_AEPS_APSMEMORY_H

#include "apsCommon.h"   // apsAllocator, apsCommon
#include "apsUtil.h"     // apsSingleton, _tlAssert
#include "apsError.h"    // AEPS_VECTOR_NEW
#include "apsAction.h"   // apsArray<T>

#include <cstring>

class apsGroup;
class apsGroupManager;

namespace apsMemory {

// ============================================================================
// Config — memory pool configuration (12 bytes). Defaults (via ctor/SetDefaults):
//   buffer 0x40000 (256KB), 250 groups, 200 effects, 50 modifiers.
// ============================================================================
struct Config {
    int   mBufferSize;    // +0x00
    short mMaxGroups;     // +0x04
    short mMaxEffects;    // +0x06
    short mMaxModifiers;  // +0x08

    Config() { SetDefaults(); }                       // ??0Config@apsMemory@@QAE@XZ
    void SetDefaults() {                              // ?SetDefaults@Config@apsMemory@@QAEXXZ
        mBufferSize = 0x40000;
        mMaxGroups = 250;
        mMaxEffects = 200;
        mMaxModifiers = 50;
    }
};
static_assert(sizeof(Config) == 0x0C, "apsMemory::Config size mismatch");

// ============================================================================
// Pool — fixed-size-block memory pool (36 bytes).
// ============================================================================
class Pool {
public:
    struct PoolBlock {     // 8 bytes
        short   mIndex;    // +0x00
        short   mNext;     // +0x02
        // +0x04: packed { mSize:24, mFlag:8 } as one 32-bit word.
        unsigned int mSizeFlag;  // +0x04

        enum { FLAG_FREE = 0, FLAG_USED = 1 };

        // Accessors for the packed word (mSize low 24 bits, mFlag high 8).
        int GetSize() const { return mSizeFlag & 0xFFFFFF; }
        void SetSize(int size) { mSizeFlag = (mSizeFlag & 0xFF000000) | (size & 0xFFFFFF); }
        int GetFlag() const { return (int)(mSizeFlag >> 24); }
        void SetFlag(unsigned int flag) { mSizeFlag = (mSizeFlag & 0xFFFFFF) | ((flag & 0xFF) << 24); }

        PoolBlock() {          // ??0PoolBlock@Pool@apsMemory@@QAE@XZ
            mSizeFlag = 0;
            mIndex = -1;
            mNext = -1;
        }
    };
    static_assert(sizeof(PoolBlock) == 8, "PoolBlock size mismatch");

    int    mNumBlocks;      // +0x00
    int    mNumBlocksUsed;  // +0x04
    int    mMaxBlocksUsed;  // +0x08
    int    mMinBlockSize;   // +0x0C
    int    mMaxBlockSize;   // +0x10
    int    mBlockSize;      // +0x14
    unsigned char* mBlockData;  // +0x18
    PoolBlock*     mBlocks;     // +0x1C
    short  mFreeListHead;   // +0x20

    struct Stats {           // 72 bytes
        int numBlocks;
        int numUsedBlocks;
        int numFreeBlocks;
        int maxUsedBlocks;
        int blockSize;
        int minBlockSize;
        int maxBlockSize;
        int memoryTotal;
        int memoryUsed;
        int memoryFree;
        int memoryWaste;
        int numUsedBlocksPercent;
        int numFreeBlocksPercent;
        int maxUsedBlocksPercent;
        int memoryUsedPercent;
        int memoryFreePercent;
        int wastePercent;
        int wastePercentTotal;
    };
    static_assert(sizeof(Stats) == 0x48, "Pool::Stats size mismatch");

    // ---- non-inline (apsMemory.o) ----
    Pool(PoolBlock* pBlocks);                     // ??0Pool@apsMemory@@QAE@PAUPoolBlock@01@@Z
    ~Pool();                                      // ??1Pool@apsMemory@@QAE@XZ
    void Init(int numBlocks, int blockSize, unsigned char* blockData);  // ?Init@Pool@apsMemory@@QAEXHHPAE@Z
    void Term();                                  // ?Term@Pool@apsMemory@@QAEXXZ
    void* Alloc(int size);                        // ?Alloc@Pool@apsMemory@@QAEPAXH@Z
    void Free(void* pBlockData);                  // ?Free@Pool@apsMemory@@QAEXPAX@Z
    void* Resize(void* pBlockData, int newSize);  // ?Resize@Pool@apsMemory@@QAEPAXPAXH@Z
    unsigned int Contains(void* pBlockData);      // ?Contains@Pool@apsMemory@@QAEIPAX@Z
    void Verify();                                // ?Verify@Pool@apsMemory@@QAEXXZ
    void* GetActiveBlock(int index);              // ?GetActiveBlock@Pool@apsMemory@@QAEPAXH@Z
    void GetStats(Stats& stats);                  // ?GetStats@Pool@apsMemory@@QAEXAAUStats@12@@Z
    void Report();                                // ?Report@Pool@apsMemory@@QAEXXZ

    // ---- static helpers ----
    static int CalcClassOverhead(int numBlocks, int blockSize);  // ?CalcClassOverhead@Pool@apsMemory@@SAHHH@Z
    static int CalcDataOverhead(int numBlocks, int blockSize);  // ?CalcDataOverhead@Pool@apsMemory@@SAHHH@Z

    // ---- inline accessors ----
    int NumBlocks() const;            // ?NumBlocks@Pool@apsMemory@@QBEHXZ
    int BlockSize() const;            // ?BlockSize@Pool@apsMemory@@QBEHXZ
    int NumBlocksUsed() const;        // ?NumBlocksUsed@Pool@apsMemory@@QBEHXZ
    int NumBlocksFree() const;        // ?NumBlocksFree@Pool@apsMemory@@QBEHXZ
    unsigned int IsEmpty() const;     // ?IsEmpty@Pool@apsMemory@@QBEIXZ
    int PtrToIndex(void* pBlockData); // ?PtrToIndex@Pool@apsMemory@@QAEHPAX@Z
    unsigned char* IndexToPtr(int index); // ?IndexToPtr@Pool@apsMemory@@QAEPAEH@Z
};
static_assert(sizeof(Pool) == 0x24, "apsMemory::Pool size mismatch");

// ============================================================================
// BlockAllocator — apsAllocator implementation over the BlockManager pools.
// Layout: apsAllocator vtable @0x00, apsSingleton<BlockAllocator> @0x04 (1 byte).
// Virtuals delegate to apsMemory::Alloc/Free/Resize (the BlockManager).
// ============================================================================
class BlockAllocator : public apsAllocator, public apsSingleton<BlockAllocator> {
public:
    BlockAllocator();                                 // ??0BlockAllocator@apsMemory@@QAE@XZ
    virtual void* MemAlign(unsigned int size, unsigned int alignment);  // ?MemAlign@BlockAllocator@apsMemory@@UBEPAXII@Z
    virtual void MemFree(void* ptr);                  // ?MemFree@BlockAllocator@apsMemory@@UBEXPAX@Z
    virtual void* MemResize(void* ptr, unsigned int size, unsigned int alignment);  // ?MemResize@BlockAllocator@apsMemory@@UBEPAXPAXII@Z
};
static_assert(sizeof(BlockAllocator) == 4, "BlockAllocator size mismatch");

// ============================================================================
// BlockManager — the pool manager singleton (28 bytes).
// ============================================================================
class BlockManager : public apsSingleton<BlockManager> {
public:
    apsArray<Pool*> mPools;          // +0x00 (8 bytes)
    int mAllocSizeSuccess;           // +0x08
    int mAllocSizeFailure;           // +0x0C
    int mAllocSuccesses;             // +0x10
    int mAllocFailures;              // +0x14
    int mMaxRequestedSize;           // +0x18

    BlockManager(int memSize, int maxPools);          // ??0BlockManager@apsMemory@@QAE@HH@Z
    ~BlockManager();                                  // ??1BlockManager@apsMemory@@QAE@XZ

    unsigned int AddPool(int numBlocks, int blockSize);  // ?AddPool@BlockManager@apsMemory@@QAEIHH@Z
    void DestroyPools();                              // ?DestroyPools@BlockManager@apsMemory@@QAEXXZ
    void Reset();                                     // ?Reset@BlockManager@apsMemory@@QAEXXZ
    void* AllocBlock(int blockSize, int alignment);   // ?AllocBlock@BlockManager@apsMemory@@QAEPAXHH@Z
    void FreeBlock(void* data);                       // ?FreeBlock@BlockManager@apsMemory@@QAEXPAX@Z
    unsigned int TestAlloc(int size);                 // ?TestAlloc@BlockManager@apsMemory@@QAEIH@Z
    void* Alloc(int size, int alignment);             // ?Alloc@BlockManager@apsMemory@@QAEPAXHH@Z
    unsigned int Free(void* pBlock);                  // ?Free@BlockManager@apsMemory@@QAEIPAX@Z
    void* Resize(void* pBlock, int newSize);          // ?Resize@BlockManager@apsMemory@@QAEPAXPAXH@Z
    void Report();                                    // ?Report@BlockManager@apsMemory@@QAEXXZ
    void Save();                                      // ?Save@BlockManager@apsMemory@@QAEXXZ
    void Restore();                                   // ?Restore@BlockManager@apsMemory@@QAEXXZ
    void ResetReportCounters();                       // ?ResetReportCounters@BlockManager@apsMemory@@QAEXXZ
};
static_assert(sizeof(BlockManager) == 0x1C, "BlockManager size mismatch");

// ============================================================================
// PoolAllocator<T> — typed wrapper over a single Pool (44 bytes).
//   apsSingleton<PoolAllocator<T>> @0x00, mPoolData @0x00, mPoolSize @0x04,
//   mPool @0x08 (Pool, 36 bytes). Inline COMDATs emitted in apsGroupMgr.o.
// ============================================================================
template <typename T>
class PoolAllocator : public apsSingleton<PoolAllocator<T> > {
public:
    T*    mPoolData;  // +0x00
    int   mPoolSize;  // +0x04
    Pool  mPool;      // +0x08

    PoolAllocator(int poolSize) : mPool(0) {  // ??0?$PoolAllocator@T@@@apsMemory@@QAE@H@Z
        if (apsSingleton<PoolAllocator<T> >::sInstancePtr != 0 &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 86,
                      "0 == sInstancePtr", "singleton already initialised"))
            __debugbreak();
        apsSingleton<PoolAllocator<T> >::sInstancePtr = this;
        mPoolSize = poolSize;
        mPoolData = AEPS_VECTOR_NEW<T>(poolSize, 16);
        mPool.Init(poolSize, sizeof(T), (unsigned char*)mPoolData);
    }
    ~PoolAllocator() {                    // ??1?$PoolAllocator@T@@@apsMemory@@QAE@XZ
        mPool.Term();
        if (mPoolSize > 0) {
            for (int i = 0; i < mPoolSize; ++i) {
                // destroy each pooled element (nullsub in the original)
                (&mPoolData[i])->~T();
            }
        }
        apsCommon::GetAllocator()->MemFree(mPoolData);
        mPool.Term();
        if (apsSingleton<PoolAllocator<T> >::sInstancePtr != 0) {
            apsSingleton<PoolAllocator<T> >::sInstancePtr = 0;
        } else {
            if (_tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 104,
                          "sInstancePtr", "singleton not initialised"))
                __debugbreak();
            apsSingleton<PoolAllocator<T> >::sInstancePtr = 0;
        }
    }

    T* Alloc() { return (T*)mPool.Alloc(sizeof(T)); }  // ?Alloc@?$PoolAllocator@T@@@apsMemory@@QAEPAVT@@XZ
    void Free(const T* p) { mPool.Free((void*)p); }    // ?Free@?$PoolAllocator@T@@@apsMemory@@QAEXPBVT@@@Z
    Pool& GetPool() { return mPool; }                  // ?GetPool@?$PoolAllocator@T@@@apsMemory@@QAEAAVPool@2@XZ
};
static_assert(sizeof(PoolAllocator<int>) == 0x2C, "PoolAllocator size mismatch");

// ============================================================================
// Global API (apsMemory.o)
// ============================================================================
extern Config gConfig;   // ?gConfig@apsMemory@@3UConfig@1@A

void SetBufferSize(int bufferSize);           // ?SetBufferSize@apsMemory@@YAXH@Z
void SetMaxGroups(int maxGroups);             // ?SetMaxGroups@apsMemory@@YAXH@Z
void SetMaxEffects(int maxEffects);           // ?SetMaxEffects@apsMemory@@YAXH@Z
void SetMaxModifiers(int maxModifiers);       // ?SetMaxModifiers@apsMemory@@YAXH@Z
unsigned int AddPool(int numBlocks, int blockSize);   // ?AddPool@apsMemory@@YAIHH@Z
void Init();                                  // ?Init@apsMemory@@YAXXZ
void Reset();                                 // ?Reset@apsMemory@@YAXXZ
void Term();                                  // ?Term@apsMemory@@YAXXZ
void Report();                                // ?Report@apsMemory@@YAXXZ
unsigned int GetPoolInfo(int nPool, int& size, int& capacity, int& used, int& peak);  // ?GetPoolInfo@apsMemory@@YAIHAAH000@Z
int GetMaxRequestedBlockSize();               // ?GetMaxRequestedBlockSize@apsMemory@@YAHXZ
unsigned int TestAlloc(int size);             // ?TestAlloc@apsMemory@@YAIH@Z
void* Alloc(int size, int alignment);         // ?Alloc@apsMemory@@YAPAXHH@Z
void Free(void* data);                        // ?Free@apsMemory@@YAXPAX@Z
unsigned int TestAllocFromPools(int size);    // ?TestAllocFromPools@apsMemory@@YAIH@Z
void* AllocFromPools(int size, int alignment);  // ?AllocFromPools@apsMemory@@YAPAXHH@Z
unsigned int FreeFromPools(void* data);       // ?FreeFromPools@apsMemory@@YAIPAX@Z
void SetBlockAllocator();                     // ?SetBlockAllocator@apsMemory@@YAXXZ
void ClearBlockAllocator();                   // ?ClearBlockAllocator@apsMemory@@YAXXZ
apsAllocator* GetBlockAllocatorDirect();      // ?GetBlockAllocatorDirect@apsMemory@@YAPAVapsAllocator@@XZ

// stubs (map: empty bodies)
unsigned int AddParticlePool(int numBlocks, int blockSize);  // ?AddParticlePool@apsMemory@@YAIHH@Z
void AddPools();                              // ?AddPools@apsMemory@@YAXXZ

} // namespace apsMemory

#endif // COD3_AEPS_APSMEMORY_H
