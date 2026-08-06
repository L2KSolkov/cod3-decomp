// ============================================================================
// apsAction — base particle-action class + apsArray<T> template.
// Source: c:\cod\code\tl\aeps\source\apsAction.cpp
// Verified against IDA (aeps_xboxr:apsAction.o):
//   ctor(int,int,IterationStyle,uint) @0x808500 (??0apsAction@@IAE@HHW4IterationStyle@0@I@Z)
//   dtor                            @0x808430 (??1apsAction@@UAE@XZ)
//   SetParam                        @0x8081F0 (?SetParam@apsAction@@QAEXHM@Z)
//   SetDomain                       @0x808260 (?SetDomain@apsAction@@QAEXHPAVapsDomain@@@Z)
//   Fixup                           @0x8082F0 (?Fixup@apsAction@@QAEXABUapsFixupParams@@@Z)
// apsAction vtable: [0]=~dtor, [1]=Act, [2]=GetId, [3]=GetVersion (3 pure).
// apsVirtualBase vtable: [0]=~dtor.
// ============================================================================
#ifndef COD3_AEPS_APSACTION_H
#define COD3_AEPS_APSACTION_H

#include "apsCommon.h"    // apsAllocator, apsCommon (allocator/pak accessors)
#include "apsUtil.h"      // _tlAssert
#include "apsRenderer.h"  // apsFixupParams

#include <cstring>

// tl_system.o (tl_xboxr, ported)
extern void tlFatal(const char* fmt, ...);

class apsGroup;
class apsEffect;

// ============================================================================
// apsVirtualBase — vtable-only root of the action/domain hierarchy (4 bytes).
// vtable: [0]=~dtor (apsRegister.o COMDATs). Accessors:
//   GetVtable ea:0x7F0940, SetVtable ea:0x808020, dtor/ctor ea:0x7F0930/0x7F0CE0.
// The vtable pointer is implicit (virtual dtor); it sits at +0x00.
// ============================================================================
class apsVirtualBase {
public:
    virtual ~apsVirtualBase() {}    // ??1apsVirtualBase@@UAE@XZ

    unsigned int GetVtable() const { return *(const unsigned int*)this; }  // ?GetVtable@apsVirtualBase@@QBEIXZ
    void         SetVtable(unsigned int tbl) { *(unsigned int*)this = tbl; }  // ?SetVtable@apsVirtualBase@@QAEXI@Z
};
static_assert(sizeof(apsVirtualBase) == 4, "apsVirtualBase size mismatch");

// ============================================================================
// apsDomain — abstract domain base (4 bytes, apsVirtualBase-derived).
// Owned by apsRegister.o (unported): ctor/dtor/Fixup are inline COMDATs there.
// ============================================================================
class apsDomain : public apsVirtualBase {
public:
    virtual ~apsDomain() {}                     // ??1apsDomain@@UAE@XZ (apsRegister.o)
    void Fixup(const apsFixupParams& iFixupParams);   // ?Fixup@apsDomain@@QAEXABUapsFixupParams@@@Z
};
static_assert(sizeof(apsDomain) == 4, "apsDomain size mismatch");

// ============================================================================
// apsArray<T> — fixed-capacity dynamic array (8 bytes).
//   mElements@0 (T*), mCapacity@4 (short), mSize@6 (short). Verified vs IDA.
// All methods are inline COMDATs (emitted in apsAction.o / callers).
// ============================================================================
template <typename T>
struct apsArray {
    T*      mElements;   // +0x00
    short   mCapacity;   // +0x04
    short   mSize;       // +0x06

    apsArray() : mElements(0), mCapacity(0), mSize(0) {}  // ??0?$apsArray@T@@QAE@XZ

    // ctor(iSize, fill): allocate + fill. ea: 0x808610/0x8086A0.
    apsArray(int iSize, const T& iFillValue) {
        int old = apsCommon::SetPakAllocs(0);
        T* buf = (T*)apsCommon::GetAllocator()->MemAlign(4 * iSize, 4);
        apsCommon::SetPakAllocs(old);

        mElements = buf;
        if (buf == 0) {
            mSize = 0;
            mCapacity = 0;
            return;
        }
        mSize = (short)iSize;
        mCapacity = (short)iSize;
        for (int i = 0; i < mSize; ++i)
            mElements[i] = iFillValue;
    }

    ~apsArray() {   // ??1?$apsArray@T@@QAE@XZ (ea: 0x7FCD20)
        if (mElements != 0) {
            int old = apsCommon::SetPakAllocs(0);
            apsCommon::GetAllocator()->MemFree(mElements);
            apsCommon::SetPakAllocs(old);
            mElements = 0;
            mCapacity = 0;
            mSize = 0;
        }
    }

    int  size() const { return mSize; }   // ?size@?$apsArray@T@@QBEHXZ
    T*   begin() { return mElements; }    // ?begin@?$apsArray@T@@QAEPATXZ
    T*   end() { return &mElements[mSize]; }  // ?end@?$apsArray@T@@QAEPATXZ

    T& operator[](int iIndex) {           // ??A?$apsArray@T@@QAEAATH@Z (ea: 0x808040)
        if ((iIndex < 0 || iIndex >= mSize) &&
            _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                      "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            __debugbreak();
        return mElements[iIndex];
    }

    const T& operator[](int iIndex) const {  // ??A?$apsArray@T@@QBEABTH@Z
        if ((iIndex < 0 || iIndex >= mSize) &&
            _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                      "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            __debugbreak();
        return mElements[iIndex];
    }

    // push_back — append at end, growing by +1 (≤3) or +4. ?push_back@...QAEIABQAT@@@Z
    int push_back(const T& iElement) {
        if (mSize < mCapacity) {
            mElements[mSize] = iElement;
            ++mSize;
            return 1;
        }
        int oldSize = mSize;
        int newCapacity = (mSize <= 3) ? (mSize + 1) : (mSize + 4);
        int old = apsCommon::SetPakAllocs(0);
        T* buf = (T*)apsCommon::GetAllocator()->MemAlign(4 * newCapacity, 4);
        apsCommon::SetPakAllocs(old);
        int result = 0;
        if (buf != 0) {
            for (int i = 0; i < oldSize; ++i)
                buf[i] = mElements[i];
            if (mElements != 0) {
                int old2 = apsCommon::SetPakAllocs(0);
                apsCommon::GetAllocator()->MemFree(mElements);
                apsCommon::SetPakAllocs(old2);
                mElements = 0;
                mCapacity = 0;
                mSize = 0;
            }
            mCapacity = (short)newCapacity;
            mSize = (short)oldSize;
            mElements = buf;
            buf[oldSize] = iElement;
            ++mSize;
            return 1;
        }
        return result;
    }

    // push_front — prepend, shifting existing elements. ?push_front@...QAEIABQAT@@@Z
    int push_front(const T& iElement) {
        int oldSize = mSize;
        if (mSize >= mCapacity) {
            int v6 = oldSize + 1;
            int old = apsCommon::SetPakAllocs(0);
            T* buf = (T*)apsCommon::GetAllocator()->MemAlign(4 * v6, 4);
            apsCommon::SetPakAllocs(old);
            int result = 0;
            if (buf != 0) {
                for (int i = 0; i < oldSize; ++i)
                    buf[i + 1] = mElements[i];
                if (mElements != 0) {
                    int old2 = apsCommon::SetPakAllocs(0);
                    apsCommon::GetAllocator()->MemFree(mElements);
                    apsCommon::SetPakAllocs(old2);
                    mElements = 0;
                    mCapacity = 0;
                    mSize = 0;
                }
                mCapacity = (short)(oldSize + 1);
                mSize = (short)oldSize;
                mElements = buf;
                buf[0] = iElement;
                ++mSize;
                return 1;
            }
            return result;
        }
        for (int i = mSize; i > 0; --i)
            mElements[i] = mElements[i - 1];
        mElements[0] = iElement;
        ++mSize;
        return 1;
    }

protected:
    T* construct_array(int iNumber) {     // ?construct_array@?$apsArray@T@@AAEPATH@Z
        int old = apsCommon::SetPakAllocs(0);
        T* buf = (T*)apsCommon::GetAllocator()->MemAlign(4 * iNumber, 4);
        apsCommon::SetPakAllocs(old);
        return buf;
    }
};

// ============================================================================
// apsDestroy<T> — virtual-destructor helper. ea: 0x808180.
// ============================================================================
template <typename T>
void apsDestroy(T* iPtr) {
    iPtr->~T();
}

// ============================================================================
// apsFixUp<T> — pointer-base fixup: ptr += basePtr (byte offset).
// ea: 0x808190/0x8081B0/0x8081D0.
// ============================================================================
template <typename T>
void apsFixUp(T*& ptr, void* basePtr) {
    ptr = (T*)((char*)ptr + (char*)basePtr);
}

// ============================================================================
// apsAction — base particle action (28 bytes).
//   apsVirtualBase @0x00, mParams @0x04 (8), mDomains @0x0C (8),
//   mIterationStyle @0x14, mRequiredParticleFields @0x18.
// vtable: [0]=~dtor, [1]=Act, [2]=GetId, [3]=GetVersion (all 3 pure).
// ============================================================================
class apsAction : public apsVirtualBase {
public:
    enum IterationStyle {
        eSource = 0,
        eAsync = 1,
        eSync = 2,
    };

    // ---- data ----
    apsArray<float>      mParams;                     // +0x04
    apsArray<apsDomain*> mDomains;                    // +0x0C
    IterationStyle       mIterationStyle;             // +0x14
    unsigned int         mRequiredParticleFields;     // +0x18

    // ---- ctors / dtor (apsAction.o) ----
    virtual ~apsAction();                             // ??1apsAction@@UAE@XZ

    // ---- pure virtual interface ----
    virtual void Act(unsigned char* iBegin, unsigned char* iEnd, apsGroup* ioGroup,
                     apsEffect* iEffect, float iElapsedTime, float iTimeDelta) = 0;  // slot 1
    virtual unsigned int GetId() const = 0;           // slot 2
    virtual float GetVersion() const = 0;             // slot 3

protected:
    // protected ctor (map: ??0apsAction@@IAE@HHW4IterationStyle@0@I@Z)
    apsAction(int iNumParams, int iNumDomains, IterationStyle iIterationStyle,
              unsigned int iRequiredParticleFields);  // ??0apsAction@@IAE@HHW4IterationStyle@0@I@Z

public:
    // ---- params / domains (apsAction.o) ----
    void SetParam(int iParamNum, float iParam);       // ?SetParam@apsAction@@QAEXHM@Z
    void SetDomain(int iDomainNum, apsDomain* iDomain);  // ?SetDomain@apsAction@@QAEXHPAVapsDomain@@@Z
    void Fixup(const apsFixupParams& iFixupParams);   // ?Fixup@apsAction@@QAEXABUapsFixupParams@@@Z

    // ---- inline accessors (COMDATs in various objects) ----
    float GetParam(int iParamNum) const {             // ?GetParam@apsAction@@QBEMH@Z (ea: 0x7EFFD0)
        if (iParamNum >= 0 && iParamNum < mParams.mSize)
            return mParams.mElements[iParamNum];
        if (!_tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                       "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            return mParams.mElements[iParamNum];
        float result = mParams.mElements[iParamNum];
        __debugbreak();
        return result;
    }
    void* GetParamAddr(int iParamNum) const {         // ?GetParamAddr@apsAction@@QBEPAXH@Z (ea: 0x810680)
        if ((iParamNum < 0 || iParamNum >= mParams.mSize) &&
            _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 145,
                      "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            __debugbreak();
        return &mParams.mElements[iParamNum];
    }    apsDomain* GetDomain(int iDomainNum) {            // ?GetDomain@apsAction@@QAEPAVapsDomain@@H@Z (ea: 0x8106D0)
        if (iDomainNum >= 0 && iDomainNum < mDomains.mSize)
            return mDomains.mElements[iDomainNum];
        if (!_tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsArray.h", 151,
                       "iIndex >= 0 && iIndex < mSize", "out of bounds"))
            return mDomains.mElements[iDomainNum];
        apsDomain* result = mDomains.mElements[iDomainNum];
        __debugbreak();
        return result;
    }
    IterationStyle GetIterationStyle() const { return mIterationStyle; }  // ?GetIterationStyle@apsAction@@QBE?AW4IterationStyle@1@XZ
    unsigned int GetRequiredParticleFields() const { return mRequiredParticleFields; }  // ?GetRequiredParticleFields@apsAction@@QBEIXZ
};
static_assert(sizeof(apsAction) == 0x1C, "apsAction size mismatch");

#endif // COD3_AEPS_APSACTION_H
