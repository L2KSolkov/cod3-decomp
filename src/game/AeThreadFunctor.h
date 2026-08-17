// ============================================================================
// AeThreadFunctor — abstract thread functor base (4 bytes, vtable only).
// Source: c:\cod\code\game\... (thread functor utilities)
//
// Verified against IDA: vtable @0xD87960 has 3 slots:
//   [0] scalar deleting dtor   (??_GAeThreadFunctor@@UAEPAXI@Z)
//   [1] GetEnt       -> purecall (___purecall)
//   [2] CallFunction -> purecall (___purecall)
// ctor/dtor are inline COMDAT in mp_level.xboxd:mp_level_wad.o.
// ============================================================================
#ifndef COD3_GAME_AETHREADFUNCTOR_H
#define COD3_GAME_AETHREADFUNCTOR_H

#include "engine/broc_types.h"
#include <stddef.h>

// ============================================================================
// AeThreadFunctor — base class for script thread functors.
// Size: 0x04 (4 bytes, vtable only) — verified against IDA
// ============================================================================
class AeThreadFunctor {
public:
    static void* operator new(size_t size) {
        return Broc::gBrocAPI.mPoolAlloc((unsigned int)size);
    }

    static void operator delete(void* ptr) {
        Broc::gBrocAPI.mPoolFree(ptr);
    }

    virtual ~AeThreadFunctor() {}             // vtable[0]
    virtual unsigned int GetEnt() = 0;        // vtable[1] (pure)
    virtual void         CallFunction() = 0;  // vtable[2] (pure)
};
static_assert(sizeof(AeThreadFunctor) == 4, "AeThreadFunctor size mismatch");

// IDA type: AeThreadFunctor1<Broc::entity> (12 bytes).
template <typename T> class AeThreadFunctor1;

template <> class AeThreadFunctor1<Broc::entity> : public AeThreadFunctor {
public:
    typedef void (__cdecl *Function)(Broc::entity);

    AeThreadFunctor1(Function fp, const Broc::entity& arg)
        : mFp(fp), mArg1(arg) {}

    unsigned int GetEnt() override { return mArg1.GetHandle(); }

    void CallFunction() override {
        Broc::entity value(mArg1);
        mFp(value);
    }

    Function mFp;       // +0x04
    Broc::entity mArg1; // +0x08
};

static_assert(sizeof(AeThreadFunctor1<Broc::entity>) == 12,
              "AeThreadFunctor1<Broc::entity> size mismatch");

// ============================================================================
// CallFunctor — invoke a thread functor.
// ea: 0x7BADB0
// ============================================================================
void CallFunctor(AeThreadFunctor* f);

#endif // COD3_GAME_AETHREADFUNCTOR_H
