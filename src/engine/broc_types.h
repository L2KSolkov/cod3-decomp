// ============================================================================
// COD3 Engine Types — Broc/BrocSys string, entity, and hashing primitives
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

#include <stdint.h>

namespace Broc {

// ============================================================================
// Broc::string — reference-counted string (4 bytes)
// Size: 0x04 (4 bytes) — verified against IDA
// Points to a Block (refcount + data).
// ============================================================================
struct string {
    struct Block {
        int32_t  refCount;  // reference count
        char     data[4];   // inline string data (variable length)
    };
    Block* mBlock;  // +0x00

    string() : mBlock(nullptr) {}
    ~string() {}  // stub — real implementation releases ref

    const char* c_str() const { return mBlock ? mBlock->data : ""; }
};
static_assert(sizeof(string) == 4, "Broc::string size mismatch");
static_assert(offsetof(string, mBlock) == 0, "Broc::string::mBlock offset mismatch");
static_assert(sizeof(string::Block) >= 8, "Broc::string::Block too small");

// ============================================================================
// Broc::entity — script entity handle (4 bytes)
// Size: 0x04 (4 bytes) — verified against IDA
// ============================================================================
struct entity {
    uint32_t ___u0;  // +0x00 — opaque handle data
};
static_assert(sizeof(entity) == 4, "Broc::entity size mismatch");

} // namespace Broc

// ============================================================================
// HashString — hashed string identifier (4 bytes)
// Size: 0x04 (4 bytes) — verified against IDA
// Stores a DJB2-like hash of the original string.
// ============================================================================
struct HashString {
    unsigned int mHash;  // +0x00
};
static_assert(sizeof(HashString) == 4, "HashString size mismatch");

// ============================================================================
// InplaceString — in-place char* (4 bytes)
// Size: 0x04 (4 bytes) — verified against IDA
// Used for constant/config strings stored in data segment.
// ============================================================================
struct InplaceString {
    char* mStr;  // +0x00
};
static_assert(sizeof(InplaceString) == 4, "InplaceString size mismatch");

// ============================================================================
// Handle — generic resource handle (4 bytes)
// ============================================================================
typedef unsigned int Handle;
