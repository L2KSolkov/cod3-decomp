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

// Generic singleton. Inline ctor/dtor (verified against apsError.o disasm):
//   ctor: assert apsUtil.h:86 "0 == sInstancePtr" / "singleton already initialised"
//   dtor: assert apsUtil.h:104 "sInstancePtr" / "singleton not initialised"
template <class T>
struct apsSingleton {
    apsSingleton() {
        if (sInstancePtr != 0) {
            if (_tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 86,
                          "0 == sInstancePtr", "singleton already initialised"))
                __debugbreak();
        }
        sInstancePtr = static_cast<T*>(this);
    }

    ~apsSingleton() {
        if (sInstancePtr != 0) {
            sInstancePtr = 0;
        } else {
            if (_tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 104,
                          "sInstancePtr", "singleton not initialised"))
                __debugbreak();
            sInstancePtr = 0;
        }
    }

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

#endif // COD3_AEPS_APSUTIL_H
