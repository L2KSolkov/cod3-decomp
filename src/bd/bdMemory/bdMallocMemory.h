#pragma once

#include "bd/bd_types.h"

class bdMallocMemory {
public:
    struct bdMemoryChainElement {
        unsigned short m_magic;
        unsigned short m_reserved;
        unsigned int m_size;
        unsigned char m_aligned;
        unsigned char m_padding[3];
        bdMemoryChainElement* m_prev;
        bdMemoryChainElement* m_next;
    };

    static void* allocate(unsigned int size);
    static void deallocate(void* p);
    static void* reallocate(void* p, unsigned int size);
    static void* alignedAllocate(unsigned int size, unsigned int align);
    static void alignedDeallocate(void* p);
    static void* alignedReallocate(void* p, unsigned int size,
                                   unsigned int align);

    static void leakCheck();
    static void releaseAllMemory();

private:
    static char* recordMemory(bdMemoryChainElement* element,
                              unsigned int size, bool aligned);
    static void eraseMemory(bdMemoryChainElement* element);

    static bdMutex m_mutex;
    static bdMemoryChainElement* m_memoryChain;
    static unsigned int m_allocatedBytes;
    static unsigned int m_numAllocations;
};

static_assert(sizeof(bdMallocMemory::bdMemoryChainElement) == 0x14,
              "bdMemoryChainElement size mismatch");
