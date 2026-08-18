// ============================================================================
// apsError.cpp — error/allocator diagnostics (3 funcs).
// Reconstructed from codmp_xboxr.xbe (release build /O2)
// Source: c:\cod\code\tl\aeps\source\apsError.cpp
//
// Port strategy (matches apsGroup.o precedent):
//   - All three non-inline functions verified against IDA disasm.
//   - sInstancePtr (data) is owned by apsCommon.o; referenced extern here.
//   - AEPS_VECTOR_NEW/DELETE + ErrorMessage ctor are inline templates/ctors
//     emitted as COMDATs into this object (matching codmp_xboxr.map).
// ============================================================================
#include "apsError.h"

#include <stdarg.h>
#include <stdio.h>

// ============================================================================
// apsError::apsError — singleton registration plus error-array allocation.
// ea: 0x8041c0
// ============================================================================
apsError::apsError() {
    if (apsSingleton<apsError>::sInstancePtr != 0 &&
        _tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 86,
                  "0 == sInstancePtr", "singleton already initialised"))
        __debugbreak();
    apsSingleton<apsError>::sInstancePtr = this;
    mNumErrors = 0;
    mErrors = AEPS_VECTOR_NEW<ErrorMessage>(kMaxErrors, 4);
}

// ============================================================================
// apsError::~apsError — free the error array and clear the singleton pointer.
// ea: 0x804290
// ============================================================================
apsError::~apsError() {
    if (mErrors != 0) {
        AEPS_VECTOR_DELETE<ErrorMessage>(kMaxErrors, mErrors);
        mErrors = 0;
    }
    if (apsSingleton<apsError>::sInstancePtr != 0) {
        apsSingleton<apsError>::sInstancePtr = 0;
    } else {
        if (_tlAssert("c:/cod/code/tl/aeps/include\\apsUtil.h", 104,
                      "sInstancePtr", "singleton not initialised"))
            __debugbreak();
        apsSingleton<apsError>::sInstancePtr = 0;
    }
}

// ============================================================================
// apsError::AddError — record a formatted error (ring stops at kMaxErrors).
// ea: 0x804180
// ============================================================================
void apsError::AddError(eErrorType iType, const char* iFormat, ...) {
    if (mNumErrors < kMaxErrors) {
        va_list ap;
        va_start(ap, iFormat);
        mErrors[mNumErrors].mType = iType;
        vsprintf(mErrors[mNumErrors].mMessage, iFormat, ap);
        va_end(ap);
        ++mNumErrors;
    }
}
