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
// apsError::apsError — the apsSingleton<apsError> base ctor runs first
// (assert apsUtil.h:86 + sInstancePtr = this); body allocates the error array.
// ea: 0x8041c0
// ============================================================================
apsError::apsError() {
    mNumErrors = 0;
    mErrors = AEPS_VECTOR_NEW<ErrorMessage>(kMaxErrors, 4);
}

// ============================================================================
// apsError::~apsError — free the error array; the apsSingleton<apsError> base
// dtor then clears sInstancePtr (assert apsUtil.h:104).
// ea: 0x804290
// ============================================================================
apsError::~apsError() {
    if (mErrors != 0) {
        AEPS_VECTOR_DELETE<ErrorMessage>(kMaxErrors, mErrors);
        mErrors = 0;
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
