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

// ============================================================================
// AeThreadFunctor — base class for script thread functors.
// Size: 0x04 (4 bytes, vtable only) — verified against IDA
// ============================================================================
class AeThreadFunctor {
public:
    virtual ~AeThreadFunctor();               // vtable[0]
    virtual unsigned int GetEnt() = 0;        // vtable[1] (pure)
    virtual void         CallFunction() = 0;  // vtable[2] (pure)
};
static_assert(sizeof(AeThreadFunctor) == 4, "AeThreadFunctor size mismatch");

// ============================================================================
// CallFunctor — invoke a thread functor.
// ea: 0x7BADB0
// ============================================================================
void CallFunctor(AeThreadFunctor* f);

#endif // COD3_GAME_AETHREADFUNCTOR_H
