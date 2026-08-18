// ============================================================================
// apsUtil — small APS helpers (singleton template + error-suppression macros).
// Source: c:\cod\code\tl\aeps\include\apsUtil.h
// apsSingleton<apsError>::sInstancePtr lives in apsCommon.o (per codmp_xboxr.map),
// so apsError.o / apsGroup.o reference it as an extern via `extern template`.
// ============================================================================
#ifndef COD3_AEPS_APSUTIL_H
#define COD3_AEPS_APSUTIL_H

// _tlAssert — shared assert hook (implemented in src/core/tl_system.cpp).
// Returns true when the caller should __debugbreak().
extern bool _tlAssert(const char* file, int line, const char* expr, const char* msg);

// Generic singleton storage/access. The ported concrete constructors perform
// the IDA-recovered registration checks themselves; keeping this base
// constructor/destructor inert avoids injecting an extra base call that is not
// present in those emitted constructors.
template <class T>
struct apsSingleton {
    apsSingleton() = default;
    ~apsSingleton() = default;

    static T& Instance() {
        if (sInstancePtr == 0 &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 109,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        return *sInstancePtr;
    }

    static T* InstancePtr() { return sInstancePtr; }

protected:
    // Protected in the original (symbol ?sInstancePtr@...@@1PAV...): accessed
    // only by derived singletons and by the public Instance()/InstancePtr().
    static T* sInstancePtr;
};

// Out-of-class definition (C++14): required so explicit instantiation in
// apsCommon.cpp emits ?sInstancePtr@?$apsSingleton@VapsError@@@@1PAVapsError@@A.
template <class T>
T* apsSingleton<T>::sInstancePtr = 0;

#endif // COD3_AEPS_APSUTIL_H
