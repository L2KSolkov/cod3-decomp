// ============================================================================
// CallFunctor.cpp — invoke a thread functor (1 non-inline func).
// Source: CallFunctor.o (game object, no lib prefix)
// Verified against IDA:
//   CallFunctor @0x7BADB0 (?CallFunctor@@YAXPAVAeThreadFunctor@@@Z)
// ============================================================================
#include "AeThreadFunctor.h"

// ============================================================================
// CallFunctor — call the functor's virtual CallFunction.
// ea: 0x7BADB0
// ============================================================================
void CallFunctor(AeThreadFunctor* f) {
    f->CallFunction();
}
