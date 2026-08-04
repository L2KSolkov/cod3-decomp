// ============================================================================
// apsError — error/allocator diagnostics (3 non-inline funcs).
// Source: c:\cod\code\tl\aeps\source\apsError.cpp
// Verified against IDA (aeps_xboxr:apsError.o):
//   AddError @0x804180  (?AddError@apsError@@QAAXW4eErrorType@1@PBDZZ)
//   ctor     @0x8041c0  (??0apsError@@QAE@XZ)
//   dtor     @0x804290  (??1apsError@@QAE@XZ)
// Layout: mNumErrors @0x00, mErrors @0x04 (8 bytes total). No vtable.
// ErrorMessage = 260 bytes: mType @0x00, mMessage[256] @0x04.
// sInstancePtr data lives in apsCommon.o — declared extern via extern template.
// ============================================================================
#ifndef COD3_AEPS_APSERROR_H
#define COD3_AEPS_APSERROR_H

#include "apsUtil.h"
#include "apsCommon.h"

#include <new>

// apsError derives from apsSingleton<apsError>: the original inlines the
// base ctor/dtor into apsError's ctor/dtor (registering/clearing the single
// protected sInstancePtr) and external consumers reach it via Instance().
class apsError : public apsSingleton<apsError> {
public:
    enum eErrorType {
        ERROR_TYPE_INVALID = 0,
        ERROR_TYPE_WARNING = 1,
        ERROR_TYPE_FATAL = 2,
    };

    struct ErrorMessage {
        eErrorType mType;       // @0x00
        char mMessage[256];     // @0x04

        ErrorMessage() : mType(ERROR_TYPE_INVALID) { mMessage[0] = 0; }

        eErrorType GetType() const { return mType; }
        const char* GetMessage() const { return mMessage; }
    };

    apsError();
    ~apsError();

    // Varargs member (QAA = cdecl-style: this pushed on stack, callee cleans up).
    void AddError(eErrorType iType, const char* iFormat, ...);

    int GetNumErrors() const { return mNumErrors; }

    ErrorMessage& GetError(int iError) {
        if (!(iError >= 0 && iError < mNumErrors) &&
            _tlAssert("c:/cod/code/tl/aeps/include\\apsError.h", 53,
                      "(err >= 0) && (err < mNumErrors)", "Requested invalid error"))
            __debugbreak();
        return mErrors[iError];
    }

    void Clear() { mNumErrors = 0; }

    static const int kMaxErrors = 8;
    static const int kMessageLength = 256;

private:
    int mNumErrors;         // @0x00
    ErrorMessage* mErrors;  // @0x04
};

// Allocate an array of T via the aps allocator, default-constructing each
// element. Signature (int, int) matches the original symbol
// ?AEPS_VECTOR_NEW@UErrorMessage@apsError@@YAPAUErrorMessage@apsError@@HH@Z.
template <class T>
T* AEPS_VECTOR_NEW(int iNum, int iAlignment) {
    T* result = static_cast<T*>(apsCommon::GetAllocator()->MemAlign(iNum * sizeof(T), iAlignment));
    for (int i = 0; i < iNum; ++i)
        new (&result[i]) T();
    return result;
}

// Free an array allocated with AEPS_VECTOR_NEW.
// Matches ?AEPS_VECTOR_DELETE@UErrorMessage@apsError@@YAXHPAUErrorMessage@apsError@@@Z.
template <class T>
void AEPS_VECTOR_DELETE(int iNum, T* iPtr) {
    apsCommon::GetAllocator()->MemFree(iPtr);
}

// sInstancePtr definition is emitted in apsCommon.o (explicit instantiation);
// references from apsError.o / apsGroup.o stay external until that object is
// ported (currently covered by /FORCE:UNRESOLVED).
extern template class apsSingleton<apsError>;

#endif // COD3_AEPS_APSERROR_H
