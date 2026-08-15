// ============================================================================
// bdReferencable — reference-counted base class (COD3 release)
// ea: 0x89EB80 (virtual destructor sets vtable)
// Ref counting (addRef/releaseRef) is INLINE — emitted as COMDAT in each caller.
// ============================================================================

#pragma once

#ifdef _WIN32
  #include <windows.h>
  #define BD_INTERLOCKED_INC(p) InterlockedIncrement((volatile LONG*)(p))
  #define BD_INTERLOCKED_DEC(p) InterlockedDecrement((volatile LONG*)(p))
#else
  #define BD_INTERLOCKED_INC(p) __sync_add_and_fetch((int*)(p), 1)
  #define BD_INTERLOCKED_DEC(p) __sync_sub_and_fetch((int*)(p), 1)
#endif

typedef int bdInt;

class bdReferencable {
public:
    bdReferencable() : m_refCount(0) {}
    virtual ~bdReferencable() {}

    bdInt addRef() {
        BD_INTERLOCKED_INC(&m_refCount);
        return m_refCount;
    }
    bdInt releaseRef() {
        BD_INTERLOCKED_DEC(&m_refCount);
        return m_refCount;
    }
    bdInt m_refCount;
};
