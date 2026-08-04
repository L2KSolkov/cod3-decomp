// ============================================================================
// COD3 Cvar Types — cvar_t, vmCvar_t, cvarTable_t, CVarTable
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

// ============================================================================
// cvar_t — console variable (44 bytes)
// Size: 0x2C (44 bytes) — verified against IDA
// ============================================================================
struct cvar_t {
    char*  name;               // +0x00
    char*  string;             // +0x04
    char*  resetString;        // +0x08
    char*  latchedString;      // +0x0C
    int    flags;              // +0x10
    int    modified;           // +0x14
    int    modificationCount;  // +0x18
    float  value;              // +0x1C
    int    integer;            // +0x20
    cvar_t* next;              // +0x24
    cvar_t* hashNext;          // +0x28
};
static_assert(sizeof(cvar_t) == 0x2C, "cvar_t size mismatch");
static_assert(offsetof(cvar_t, name) == 0x00, "cvar_t::name offset mismatch");
static_assert(offsetof(cvar_t, value) == 0x1C, "cvar_t::value offset mismatch");
static_assert(offsetof(cvar_t, hashNext) == 0x28, "cvar_t::hashNext offset mismatch");

// ============================================================================
// vmCvar_t — VM-facing cvar cache (144 bytes)
// Size: 0x90 (144 bytes) — verified against IDA
// ============================================================================
struct vmCvar_t {
    int   handle;               // +0x00
    int   modificationCount;    // +0x04
    float value;                // +0x08
    int   integer;              // +0x0C
    char  string[128];          // +0x10
};
static_assert(sizeof(vmCvar_t) == 0x90, "vmCvar_t size mismatch");
static_assert(offsetof(vmCvar_t, string) == 0x10, "vmCvar_t::string offset mismatch");

// ============================================================================
// cvarTable_t — cvar registration entry (16 bytes)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
struct cvarTable_t {
    vmCvar_t* vmCvar;        // +0x00
    char*     cvarName;      // +0x04
    char*     defaultString; // +0x08
    int       cvarFlags;     // +0x0C
};
static_assert(sizeof(cvarTable_t) == 0x10, "cvarTable_t size mismatch");

// ============================================================================
// CVarTable — extended cvar table entry (24 bytes)
// Size: 0x18 (24 bytes) — verified against IDA
// ============================================================================
struct CVarTable {
    vmCvar_t* vmCvar;             // +0x00
    char*     cvarName;           // +0x04
    char*     defaultString;      // +0x08
    int       cvarFlags;          // +0x0C
    int       modificationCount;  // +0x10
    int       trackChange;        // +0x14
};
static_assert(sizeof(CVarTable) == 0x18, "CVarTable size mismatch");
static_assert(offsetof(CVarTable, trackChange) == 0x14, "CVarTable::trackChange offset mismatch");
