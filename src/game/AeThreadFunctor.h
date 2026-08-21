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

class PoolAllocator;

// ============================================================================
// AeThreadFunctor — base class for script thread functors.
// Size: 0x04 (4 bytes, vtable only) — verified against IDA
// ============================================================================
class AeThreadFunctor {
public:
    static PoolAllocator* sAllocator;
    static void SetAllocator(PoolAllocator* allocator);

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

// IDA-backed two-argument functor layout: base/vtable, function pointer,
// entity argument, then the second captured value.
template <typename T1, typename T2> class AeThreadFunctor2 : public AeThreadFunctor {
public:
    typedef void (__cdecl *Function)(Broc::entity, T2);

    AeThreadFunctor2(Function fp, const Broc::entity& arg1, const T2& arg2)
        : mFp(fp), mArg1(arg1), mArg2(arg2) {}

    unsigned int GetEnt() override { return mArg1.GetHandle(); }

    void CallFunction() override {
        Broc::entity value(mArg1);
        mFp(value, mArg2);
    }

    Function mFp;       // +0x04
    Broc::entity mArg1; // +0x08
    T2 mArg2;            // +0x0C
};

static_assert(sizeof(AeThreadFunctor2<Broc::entity, Broc::entity>) == 16,
              "AeThreadFunctor2<Broc::entity,Broc::entity> size mismatch");
static_assert(sizeof(AeThreadFunctor2<Broc::entity, Broc::bbool>) == 16,
              "AeThreadFunctor2<Broc::entity,bbool> size mismatch");
static_assert(sizeof(AeThreadFunctor2<Broc::entity, Broc::bint>) == 16,
              "AeThreadFunctor2<Broc::entity,bint> size mismatch");
static_assert(sizeof(AeThreadFunctor2<Broc::entity, Broc::bfloat>) == 16,
              "AeThreadFunctor2<Broc::entity,bfloat> size mismatch");
static_assert(sizeof(AeThreadFunctor2<Broc::entity, Broc::string>) == 16,
              "AeThreadFunctor2<Broc::entity,string> size mismatch");
static_assert(sizeof(AeThreadFunctor2<Broc::entity, HashStr>) == 16,
              "AeThreadFunctor2<Broc::entity,HashStr> size mismatch");
static_assert(sizeof(AeThreadFunctor2<Broc::entity, Broc::vector>) == 24,
              "AeThreadFunctor2<Broc::entity,vector> size mismatch");

// IDA-backed three-argument functor layout: base/vtable, function pointer,
// entity argument, then the two captured values.
template <typename T1, typename T2, typename T3> class AeThreadFunctor3 : public AeThreadFunctor {
public:
    typedef void (__cdecl *Function)(Broc::entity, T2, T3);

    AeThreadFunctor3(Function fp, const Broc::entity& arg1,
                     const T2& arg2, const T3& arg3)
        : mFp(fp), mArg1(arg1), mArg2(arg2), mArg3(arg3) {}

    unsigned int GetEnt() override { return mArg1.GetHandle(); }

    void CallFunction() override {
        Broc::entity value(mArg1);
        mFp(value, mArg2, mArg3);
    }

    Function mFp;       // +0x04
    Broc::entity mArg1; // +0x08
    T2 mArg2;           // +0x0C
    T3 mArg3;           // after mArg2
};

static_assert(sizeof(AeThreadFunctor3<Broc::entity, Broc::vector, Broc::vector>) == 36,
              "AeThreadFunctor3<entity,vector,vector> size mismatch");
static_assert(sizeof(AeThreadFunctor3<Broc::entity, Broc::string, Broc::bfloat>) == 20,
              "AeThreadFunctor3<entity,string,bfloat> size mismatch");
static_assert(sizeof(AeThreadFunctor3<Broc::entity, Broc::string, Broc::vector>) == 28,
              "AeThreadFunctor3<entity,string,vector> size mismatch");
static_assert(sizeof(AeThreadFunctor3<Broc::entity, Broc::bint, HashStr>) == 20,
              "AeThreadFunctor3<entity,bint,HashStr> size mismatch");
static_assert(sizeof(AeThreadFunctor3<Broc::entity, Broc::bint, Broc::bint>) == 20,
              "AeThreadFunctor3<entity,bint,bint> size mismatch");
static_assert(sizeof(AeThreadFunctor3<Broc::entity, Broc::bfloat, const char*>) == 20,
              "AeThreadFunctor3<entity,bfloat,const char*> size mismatch");

// IDA-backed four-argument functor layout: base/vtable, function pointer,
// entity argument, then the three captured values.
template <typename T1, typename T2, typename T3, typename T4>
class AeThreadFunctor4 : public AeThreadFunctor {
public:
    typedef void (__cdecl *Function)(Broc::entity, T2, T3, T4);

    AeThreadFunctor4(Function fp, const Broc::entity& arg1,
                     const T2& arg2, const T3& arg3, const T4& arg4)
        : mFp(fp), mArg1(arg1), mArg2(arg2), mArg3(arg3), mArg4(arg4) {}

    unsigned int GetEnt() override { return mArg1.GetHandle(); }

    void CallFunction() override {
        Broc::entity value(mArg1);
        mFp(value, mArg2, mArg3, mArg4);
    }

    Function mFp;       // +0x04
    Broc::entity mArg1; // +0x08
    T2 mArg2;           // +0x0C
    T3 mArg3;           // after mArg2
    T4 mArg4;           // after mArg3
};

static_assert(sizeof(AeThreadFunctor4<Broc::entity, Broc::string, Broc::string,
                                     Broc::bfloat>) == 24,
              "AeThreadFunctor4<entity,string,string,bfloat> size mismatch");
static_assert(sizeof(AeThreadFunctor4<Broc::entity, Broc::string, Broc::string,
                                     Broc::string>) == 24,
              "AeThreadFunctor4<entity,string,string,string> size mismatch");

// IDA-backed five-argument functor layout: base/vtable, function pointer,
// entity argument, then the four captured values.
template <typename T1, typename T2, typename T3, typename T4, typename T5>
class AeThreadFunctor5 : public AeThreadFunctor {
public:
    typedef void (__cdecl *Function)(Broc::entity, T2, T3, T4, T5);

    AeThreadFunctor5(Function fp, const Broc::entity& arg1,
                     const T2& arg2, const T3& arg3, const T4& arg4,
                     const T5& arg5)
        : mFp(fp), mArg1(arg1), mArg2(arg2), mArg3(arg3), mArg4(arg4),
          mArg5(arg5) {}

    unsigned int GetEnt() override { return mArg1.GetHandle(); }

    void CallFunction() override {
        Broc::entity value(mArg1);
        mFp(value, mArg2, mArg3, mArg4, mArg5);
    }

    Function mFp;       // +0x04
    Broc::entity mArg1; // +0x08
    T2 mArg2;           // +0x0C
    T3 mArg3;           // after mArg2
    T4 mArg4;           // after mArg3
    T5 mArg5;           // after mArg4
};

static_assert(sizeof(AeThreadFunctor5<Broc::entity, Broc::string, Broc::string,
                                     Broc::string, Broc::bfloat>) == 28,
              "AeThreadFunctor5<entity,string,string,string,bfloat> size mismatch");

// IDA-backed six-argument functor layout: base/vtable, function pointer,
// entity argument, then the five captured values.
template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6>
class AeThreadFunctor6 : public AeThreadFunctor {
public:
    typedef void (__cdecl *Function)(Broc::entity, T2, T3, T4, T5, T6);

    AeThreadFunctor6(Function fp, const Broc::entity& arg1,
                     const T2& arg2, const T3& arg3, const T4& arg4,
                     const T5& arg5, const T6& arg6)
        : mFp(fp), mArg1(arg1), mArg2(arg2), mArg3(arg3), mArg4(arg4),
          mArg5(arg5), mArg6(arg6) {}

    unsigned int GetEnt() override { return mArg1.GetHandle(); }

    void CallFunction() override {
        Broc::entity value(mArg1);
        mFp(value, mArg2, mArg3, mArg4, mArg5, mArg6);
    }

    Function mFp;       // +0x04
    Broc::entity mArg1; // +0x08
    T2 mArg2;           // +0x0C
    T3 mArg3;           // after mArg2
    T4 mArg4;           // after mArg3
    T5 mArg5;           // after mArg4
    T6 mArg6;           // after mArg5
};

static_assert(sizeof(AeThreadFunctor6<Broc::entity, Broc::string, Broc::bbool,
                                     Broc::bint, Broc::bint, Broc::bint>) == 32,
              "AeThreadFunctor6<entity,string,bbool,bint,bint,bint> size mismatch");

// ============================================================================
// CallFunctor — invoke a thread functor.
// ea: 0x7BADB0
// ============================================================================
void CallFunctor(AeThreadFunctor* f);

#endif // COD3_GAME_AETHREADFUNCTOR_H
