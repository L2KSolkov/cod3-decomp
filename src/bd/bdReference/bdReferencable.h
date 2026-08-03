// ============================================================================
// bdReferencable — reference-counted base class (Demonware 2.0, ported from 2.3.4)
// Verified against COD3 ea: 0x89EB80 (virtual destructor)
// ============================================================================

#pragma once

#ifdef _WIN32
  #include <windows.h>
#else
  #define InterlockedIncrement(p) __sync_add_and_fetch(p, 1)
  #define InterlockedDecrement(p) __sync_sub_and_fetch(p, 1)
#endif

typedef int bdInt;

/// Base class for reference-counted objects used with bdReference<T>.
class bdReferencable
{
public:
    bdReferencable() : m_refCount(0) {}

    virtual ~bdReferencable() {}

    bdInt addRef() {
        InterlockedIncrement((volatile long*)&m_refCount);
        return m_refCount;
    }

    bdInt releaseRef() {
        InterlockedDecrement((volatile long*)&m_refCount);
        return m_refCount;
    }

    bdInt getRefCount() const { return m_refCount; }

protected:
    bdInt m_refCount;
};
