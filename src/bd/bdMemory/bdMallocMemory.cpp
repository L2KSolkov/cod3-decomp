#include "bd/bdMemory/bdMallocMemory.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

extern const char defaultFileName[];

void* bdAlignedOffsetMalloc(unsigned int size, unsigned int align,
                            unsigned int offset);
void bdAlignedOffsetFree(void* p);
void* bdAlignedOffsetRealloc(void* p, unsigned int oldSize,
                             unsigned int newSize, unsigned int align,
                             unsigned int offset);

bdMutex::bdMutex()
    : m_handle(NULL)
{
#ifdef _WIN32
    m_handle = CreateMutexA(NULL, FALSE, NULL);
#endif
}

bdMutex::~bdMutex()
{
#ifdef _WIN32
    ReleaseMutex(m_handle);
    CloseHandle(m_handle);
#endif
    m_handle = NULL;
}

void bdMutex::lock()
{
#ifdef _WIN32
    WaitForSingleObject(m_handle, INFINITE);
#endif
}

void bdMutex::unlock()
{
#ifdef _WIN32
    ReleaseMutex(m_handle);
#endif
}

namespace bdMemory {
void* (*m_allocateFunc)(unsigned int) = NULL;
void (*m_deallocateFunc)(void*) = NULL;
void* (*m_reallocateFunc)(void*, unsigned int) = NULL;
void* (*m_alignedAllocateFunc)(unsigned int, unsigned int) = NULL;
void (*m_alignedDeallocateFunc)(void*) = NULL;
void* (*m_alignedReallocateFunc)(void*, unsigned int, unsigned int) = NULL;

void* (*setAllocateFunc(void* (*func)(unsigned int)))(unsigned int)
{
    m_allocateFunc = func;
    return func;
}

void* (*setAlignedAllocateFunc(void* (*func)(unsigned int, unsigned int)))(unsigned int, unsigned int)
{
    m_alignedAllocateFunc = func;
    return func;
}

void (*setDeallocateFunc(void (*func)(void*)))(void*)
{
    m_deallocateFunc = func;
    return func;
}

void (*setAlignedDeallocateFunc(void (*func)(void*)))(void*)
{
    m_alignedDeallocateFunc = func;
    return func;
}

void* (*setReallocateFunc(void* (*func)(void*, unsigned int)))(void*, unsigned int)
{
    m_reallocateFunc = func;
    return func;
}

void* (*setAlignedReallocateFunc(void* (*func)(void*, unsigned int, unsigned int)))(void*, unsigned int, unsigned int)
{
    m_alignedReallocateFunc = func;
    return func;
}

void* (*getAllocateFunc())(unsigned int) { return m_allocateFunc; }
void (*getDeallocateFunc())(void*) { return m_deallocateFunc; }
void* (*getReallocateFunc())(void*, unsigned int) { return m_reallocateFunc; }
void* (*getAlignedAllocateFunc())(unsigned int, unsigned int) { return m_alignedAllocateFunc; }
void (*getAlignedDeallocateFunc())(void*) { return m_alignedDeallocateFunc; }
void* (*getAlignedReallocateFunc())(void*, unsigned int, unsigned int) { return m_alignedReallocateFunc; }

void* allocate(unsigned int size)
{
    void* result = NULL;
    if (m_allocateFunc != NULL) {
        result = m_allocateFunc(size);
        if (result == NULL)
            __debugbreak();
        return result;
    }
    return result;
}

void deallocate(void* p)
{
    if (m_deallocateFunc != NULL)
        m_deallocateFunc(p);
}

void* reallocate(void* p, unsigned int size)
{
    void* result = NULL;
    if (m_reallocateFunc != NULL) {
        result = m_reallocateFunc(p, size);
        if (result == NULL)
            __debugbreak();
        return result;
    }
    return result;
}

void* alignedAllocate(unsigned int size, unsigned int align)
{
    void* result = NULL;
    if (m_alignedAllocateFunc != NULL) {
        result = m_alignedAllocateFunc(size, align);
        if (result == NULL)
            __debugbreak();
        do {
            if ((reinterpret_cast<unsigned int>(result) & (align - 1)) != 0) {
                bdMessageProxy proxy(
                    ".\\bdMemory\\bdMemory.cpp",
                    "void *__cdecl bdMemory::alignedAllocate(__w64 const unsigned int,__w64 const unsigned int)",
                    0xB5u, "dw/err");
                proxy.log(defaultFileName, "Memory block has incorrect alignment.");
            }
        } while (g_assertFalse);
        return result;
    }
    return result;
}

void alignedDeallocate(void* p)
{
    if (m_alignedDeallocateFunc != NULL)
        m_alignedDeallocateFunc(p);
}

void* alignedReallocate(void* p, unsigned int size, unsigned int align)
{
    void* result = NULL;
    if (m_alignedReallocateFunc != NULL) {
        result = m_alignedReallocateFunc(p, size, align);
        if (result == NULL)
            __debugbreak();
        do {
            if ((reinterpret_cast<unsigned int>(result) & (align - 1)) != 0) {
                bdMessageProxy proxy(
                    ".\\bdMemory\\bdMemory.cpp",
                    "void *__cdecl bdMemory::alignedReallocate(void *,__w64 const unsigned int,__w64 const unsigned int)",
                    0xE2u, "dw/err");
                proxy.log(defaultFileName, "Memory block has incorrect alignment.");
            }
        } while (g_assertFalse);
        return result;
    }
    return result;
}
}

bdMutex bdMallocMemory::m_mutex;
bdMallocMemory::bdMemoryChainElement* bdMallocMemory::m_memoryChain = NULL;
unsigned int bdMallocMemory::m_allocatedBytes = 0;
unsigned int bdMallocMemory::m_numAllocations = 0;

char* bdMallocMemory::recordMemory(bdMemoryChainElement* element,
                                   unsigned int size, bool aligned)
{
    if (element == NULL)
        return NULL;
    m_mutex.lock();
    element->m_magic = 0xBDBD;
    element->m_size = size;
    element->m_aligned = aligned ? 1 : 0;
    element->m_next = m_memoryChain;
    element->m_prev = NULL;
    if (m_memoryChain != NULL)
        m_memoryChain->m_prev = element;
    m_allocatedBytes += size;
    m_memoryChain = element;
    ++m_numAllocations;
    m_mutex.unlock();
    return reinterpret_cast<char*>(element) + 0x14;
}

void bdMallocMemory::eraseMemory(bdMemoryChainElement* element)
{
    m_mutex.lock();
    if (element->m_magic != 0xBDBD) {
        m_mutex.unlock();
        bdMessageProxy proxy(
            ".\\bdMemory\\bdMallocMemory.cpp",
            "void __cdecl bdMallocMemory::eraseMemory(struct bdMallocMemory::bdMemoryChainElement *)",
            0x74u, "dw/err/");
        proxy.log("mallocmemory", " BD_MEMORY_MAGIC is incorrect.");
        m_mutex.lock();
    }
    if (element->m_prev != NULL)
        element->m_prev->m_next = element->m_next;
    else
        m_memoryChain = element->m_next;
    if (element->m_next != NULL)
        element->m_next->m_prev = element->m_prev;
    m_allocatedBytes -= element->m_size;
    --m_numAllocations;
    m_mutex.unlock();
}

void* bdMallocMemory::allocate(unsigned int size)
{
    bdMemoryChainElement* element = static_cast<bdMemoryChainElement*>(
        bdAlignedOffsetMalloc(size + 20, 8, 0x14));
    return recordMemory(element, size, false);
}

void bdMallocMemory::deallocate(void* p)
{
    if (p != NULL) {
        bdMemoryChainElement* element = reinterpret_cast<bdMemoryChainElement*>(
            static_cast<unsigned char*>(p) - 20);
        do {
            if (element->m_aligned != 0) {
                bdMessageProxy proxy(
                    "..\\bdCore/bdMemory/bdMallocMemory.inl",
                    "void __cdecl bdMallocMemory::deallocate(void *)",
                    0x23u, "dw/err");
                proxy.log(defaultFileName,
                    "bdMallocMemory::free, memory block allocated as aligned but is beign deallocated with bdMallocMemory::free. Use bdMallocMemory::alignedFree instead.");
            }
        } while (g_assertFalse);
        eraseMemory(element);
        bdAlignedOffsetFree(element);
    }
}

void* bdMallocMemory::reallocate(void* p, unsigned int size)
{
    if (p != NULL) {
        bdMemoryChainElement* element = reinterpret_cast<bdMemoryChainElement*>(
            static_cast<unsigned char*>(p) - 20);
        unsigned int oldSize = element->m_size + 20;
        eraseMemory(element);
        bdMemoryChainElement* replacement = static_cast<bdMemoryChainElement*>(
            bdAlignedOffsetRealloc(element, oldSize, size + 20, 8, 0x14));
        return recordMemory(replacement, size, false);
    }
    bdMemoryChainElement* element = static_cast<bdMemoryChainElement*>(
        bdAlignedOffsetMalloc(size + 20, 8, 0x14));
    return recordMemory(element, size, false);
}

void* bdMallocMemory::alignedAllocate(unsigned int size, unsigned int align)
{
    bdMemoryChainElement* element = static_cast<bdMemoryChainElement*>(
        bdAlignedOffsetMalloc(size + 20, align, 0x14));
    return recordMemory(element, size, true);
}

void bdMallocMemory::alignedDeallocate(void* p)
{
    if (p != NULL) {
        bdMemoryChainElement* element = reinterpret_cast<bdMemoryChainElement*>(
            static_cast<unsigned char*>(p) - 20);
        do {
            if (element->m_aligned != 1) {
                bdMessageProxy proxy(
                    "..\\bdCore/bdMemory/bdMallocMemory.inl",
                    "void __cdecl bdMallocMemory::alignedDeallocate(void *)",
                    0x58u, "dw/err");
                proxy.log(defaultFileName,
                    "bdMallocMemory::alignedFree, memory block allocated unaligned but is beign deallocated with bdMallocMemory::alignedFree. Use bdMallocMemory::free instead.");
            }
        } while (g_assertFalse);
        eraseMemory(element);
        bdAlignedOffsetFree(element);
    }
}

void* bdMallocMemory::alignedReallocate(void* p, unsigned int size,
                                        unsigned int align)
{
    if (p != NULL) {
        bdMemoryChainElement* element = reinterpret_cast<bdMemoryChainElement*>(
            static_cast<unsigned char*>(p) - 20);
        unsigned int oldSize = element->m_size + 20;
        eraseMemory(element);
        bdMemoryChainElement* replacement = static_cast<bdMemoryChainElement*>(
            bdAlignedOffsetRealloc(element, oldSize, size + 20, align, 0x14));
        return recordMemory(replacement, size, true);
    }
    bdMemoryChainElement* element = static_cast<bdMemoryChainElement*>(
        bdAlignedOffsetMalloc(size + 20, align, 0x14));
    return recordMemory(element, size, true);
}

void bdMallocMemory::leakCheck()
{
    int result = static_cast<int>(m_allocatedBytes);
    if (m_allocatedBytes != 0) {
        char buf[100];
        m_mutex.lock();
        bdSnprintf(buf, 100, "%u Bytes leaked in %u allocation(s)\n",
                   m_allocatedBytes, m_numAllocations);
        m_mutex.unlock();
        bdFprintf(stderr, "********************************************\n");
        bdFprintf(stderr, "*      BITDEMON MEMORY LEAKS DETECTED      *\n");
        bdFprintf(stderr, "********************************************\n");
        bdFprintf(stderr, "%s", buf);
        bdFprintf(stderr, "********************************************\n");
    }
    (void)result;
}

void bdMallocMemory::releaseAllMemory()
{
    m_mutex.lock();
    for (bdMemoryChainElement* element = m_memoryChain;
         m_memoryChain != NULL; element = m_memoryChain) {
        eraseMemory(element);
        if (element->m_aligned != 0)
            bdAlignedOffsetFree(element);
        else
            free(element);
    }
    m_mutex.unlock();
}

int bdFprintf(void* stream, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int result = vfprintf(static_cast<FILE*>(stream), format, ap);
    va_end(ap);
    return result;
}

void bdCore::init(bool netEnabled)
{
    if (m_initialized) {
        bdMessageProxy proxy(".\\bdCore.cpp",
                             "void __cdecl bdCore::init(const bool)",
                             0x3Du, "dw/warn/");
        proxy.log("core", "init() has been called twice without an intermediate quit()");
    } else {
        if (netEnabled) {
            bdMemory::setAllocateFunc(bdMallocMemory::allocate);
            bdMemory::setAlignedAllocateFunc(bdMallocMemory::alignedAllocate);
            bdMemory::setDeallocateFunc(bdMallocMemory::deallocate);
            bdMemory::setAlignedDeallocateFunc(bdMallocMemory::alignedDeallocate);
            bdMemory::setReallocateFunc(bdMallocMemory::reallocate);
            bdMemory::setAlignedReallocateFunc(bdMallocMemory::alignedReallocate);
        }
        m_initialized = true;
    }
}

void bdCore::quit()
{
    if (m_initialized) {
        bdSingletonRegistryCleanUp();
        bdMallocMemory::leakCheck();
        bdMemory::setAllocateFunc(NULL);
        bdMemory::setAlignedAllocateFunc(NULL);
        bdMemory::setDeallocateFunc(NULL);
        bdMemory::setAlignedDeallocateFunc(NULL);
        bdMemory::setReallocateFunc(NULL);
        bdMemory::setAlignedReallocateFunc(NULL);
        m_initialized = false;
    } else {
        bdMessageProxy proxy(".\\bdCore.cpp",
                             "void __cdecl bdCore::quit(void)",
                             0x5Du, "dw/warn/");
        proxy.log("core", "quit() has been called twice without an intermediate init()");
    }
}

bool bdCore::m_initialized = false;

void bdCore_quit()
{
    bdCore::quit();
}
