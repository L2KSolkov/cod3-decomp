// ============================================================================
// Inplace File Format — pointer fixup and tree extraction utilities
// Source: InplaceTree.cpp (14), PtrFixupTable.cpp (36), InplaceFileBuilder.cpp
// ea: 0x7E1800-0x7E1A10 (3 funcs + helper)
// ============================================================================

#include <cstring>
#include <cstdint>

// Forward
namespace AeAssert {
    enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
    extern ECoderId gCurrentAuthor;
    extern const char* gCurrentFile;
    extern int gCurrentLine;
    extern const char* gCurrentExpr;
    bool IsIgnored();
    bool Assert(const char* fmt, ...);
    bool Error(const char* fmt, ...);
}

// ============================================================================
// PtrFixupTable — manages pointer fixups for in-place loaded data
// ============================================================================
class PtrFixupTable {
public:
    uint32_t  mSize;   // +0x00 — number of fixup entries
    uint32_t* mList;   // +0x04 — fixup-list pointer, adjusted by byte offset

    void Fixup(const void* basePtr);
};

template <typename T>
void FixupPointer(T** ptr, const void* base)
{
    *ptr = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(*ptr)
                                + reinterpret_cast<uintptr_t>(base));
}

template void FixupPointer<unsigned int>(unsigned int** ptr,
                                         const void* base);
template void FixupPointer<PtrFixupTable>(PtrFixupTable** ptr,
                                          const void* base);

// ============================================================================
// ExtractNode — unpack a packed node entry into offset + next pointer
// ea: 0x7E1A10
// ============================================================================
void ExtractNode(unsigned int packed, int nextBits, int* outOffset, uint32_t* outNext) {
    int val = (int)packed;
    if (val >= 0) {
        uint32_t mask = (1 << (31 - nextBits)) - 1;
        int extracted = val & mask;
        int signBit = ~((1 << ((31 - nextBits) - 1)) - 1);
        if (signBit & extracted)
            extracted |= signBit;  // sign-extend
        *outOffset = extracted;
        *outNext = (val & 0x7FFFFFFF) >> (31 - nextBits);
    } else {
        if (!(val & 0x40000000))
            val = val & 0x7FFFFFFF;
        *outOffset = val;
        *outNext = 0;
    }
}

// ============================================================================
// FixupPointerChain — apply a single pointer fixup
// ea: 0x7E1890
// ============================================================================
static void FixupPointerChain(uint32_t nextBits, uint32_t* node) {
    uint32_t next;
    int offset;
    do {
        offset = 0;
        next = 0;
        ExtractNode(*node, nextBits, &offset, &next);
        *node = (uint32_t)(uintptr_t)((uint8_t*)node + offset);
        node = (uint32_t*)((uint8_t*)node + next);
    } while (next != 0);
}

// ============================================================================
// PtrFixupTable::Fixup — apply all fixups in the table
// ea: 0x7E18D0
// ============================================================================
void PtrFixupTable::Fixup(const void* basePtr) {
    mList = reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(mList) +
                                        (uintptr_t)basePtr);

    if (mSize >= 10000000) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "PtrFixupTable.cpp";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "mSize < 10000000";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("sanity check"))
            __debugbreak();
    }

    int nextBits = (int)*mList;
    if (nextBits >= 20) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "PtrFixupTable.cpp";
        AeAssert::gCurrentLine = 48;
        AeAssert::gCurrentExpr = "nNextBits < 20";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("possible data corruption"))
            __debugbreak();
    }

    for (uint32_t i = 1; i < mSize; ++i) {
        uint32_t val = mList[i];
        if (val >= 0x5000000) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "PtrFixupTable.cpp";
            AeAssert::gCurrentLine = 53;
            AeAssert::gCurrentExpr = "val < 0x5000000";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("safety check"))
                __debugbreak();
        }
        FixupPointerChain((uint32_t)nextBits, (uint32_t*)((uint8_t*)basePtr + val));
    }
}

// ============================================================================
// GetNullBuffer — return a zero-initialized static buffer
// ea: 0x7E1800
// ============================================================================
static char  sNullBuffer[32] = {};
static int   isInit_0 = 0;

char* GetNullBuffer(int size) {
    if (size > 32) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "InplaceTree.cpp";
        AeAssert::gCurrentLine = 14;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("You need to increase the MIN_NULL_BUFFER_SIZE (currently '%s')", 0x20))
            __debugbreak();
    }
    if (!isInit_0) {
        memset(sNullBuffer, 0, sizeof(sNullBuffer));
        isInit_0 = 1;
    }
    return sNullBuffer;
}
