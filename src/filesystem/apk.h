// ============================================================================
// APK — Asset Package (APKF format) file loader
// Source: source/apk.cpp (line refs: 0x7B, 0xAA, 0x109)
// ea: 0x834050-0x834D70 (23 functions)
// ============================================================================

#pragma once

#include <stdint.h>
#include "core/tlFixedString.h"

namespace apk {

// ============================================================================
// apkFileSection — a data section within the APK file (24 bytes)
// ============================================================================
struct apkFileSection {
    const char* Name;   // +0x00
    void*       Data;   // +0x04
    uint32_t    Size;   // +0x08
    uint32_t    Flags;  // +0x0C
    uint32_t    pad[3]; // +0x10
};

// Forward
struct apkFile;
struct apkFileEntry;

// ============================================================================
// apkFileTypeEntry — registered file type within the APK
// ============================================================================
struct apkFileTypeEntry {
    uint32_t Type;       // +0x00
    uint32_t Version;    // +0x04 — actually shares space with NSections
    uint32_t NSections;  // (derived from apkFile context)
    uint32_t EntryStride;// +0x08
    uint32_t NEntries;   // +0x0C
    uint8_t* FirstEntry; // +0x10 — first entry pointer (fixed-up at load)
};

// ============================================================================
// apkFileEntry — individual file entry (variable size)
// ============================================================================
struct apkFileEntry {
    const char* Name;    // +0x00 — fixed-up name
    uint8_t     Sections[1]; // variable — per-section byte offsets (NSections bytes)

    void* GetData(apkFile* file, int section, bool assertIfNoData);
    uint32_t GetDataSize(apkFile* file, int section, bool assertIfNoData, void** dataPtr);
};

// ============================================================================
// apkFile — loaded APK file object (in-place loaded)
// ============================================================================
struct apkFile {
    uint32_t        Magic;      // +0x00 — "APKF"
    uint32_t        Version;    // +0x04 — 260
    uint32_t        Flags;      // +0x08 — bit 0=in-place, bit 1=owned
    uint32_t        NSections;  // +0x0C
    apkFileSection* Sections;   // +0x10 — section array
    apkFileTypeEntry* FileTypes;// +0x14 — first file type entry

    apkFileTypeEntry* GetFileTypeEntry(uint32_t type);
    apkFileEntry* GetFirstFile(uint32_t type);
    apkFileEntry* GetNextFile(uint32_t type, apkFileEntry* current);
    apkFileEntry* GetFile(uint32_t idx);
    apkFileEntry* GetFile(const tlFixedString& name, uint32_t type);
    int GetSectionIndex(const tlFixedString& name);
    void ApplyFixups(uint32_t** fixupData, tlFixedString* stringTable);
    void ApplyReferences(uint32_t** refData, tlFixedString* stringTable);
    void ApplyReferencesForEntry(apkFileEntry* entry);
    void InvokeLoadCallbacks(bool invokeFileCallbacks);
    void InvokeDeleteCallbacks();
    bool InvokeFileLoadCallback(apkFileTypeEntry* typeEntry, apkFileEntry* entry);
};

// ============================================================================
// Callback list entries
// ============================================================================
struct apkFileCallbackListEntry {
    uint32_t    FourCC;
    uint32_t    Version;
    void*       LoadCallback;
    void*       DeleteCallback;
    void*       UserData;
    apkFileCallbackListEntry* Next;
};

struct apkSectionCallbackListEntry {
    tlFixedString Name;
    void*         LoadCallback;
    void*         DeleteCallback;
    void*         UserData;
    apkSectionCallbackListEntry* Next;
};

// ============================================================================
// Globals
// ============================================================================
extern apkFileCallbackListEntry*    apkFileCallbackList;
extern apkSectionCallbackListEntry* apkSectionCallbackList;
extern char apkRootDirectory[256];

// ============================================================================
// API
// ============================================================================
void apkRegisterFileType(uint32_t FourCC, uint32_t Version,
    void (*load)(apkFile*, apkFileEntry*, void*),
    void (*del)(apkFile*, apkFileEntry*, void*), void* userData);
void apkUnregisterFileType(uint32_t FourCC, uint32_t Version);
void apkRegisterSectionType(const tlFixedString& Name,
    void (*load)(apkFile*, apkFileSection*, void*),
    void (*del)(apkFile*, apkFileSection*, void*), void* userData);
void apkUnregisterSectionType(const tlFixedString& Name);
void apkSetResourceCallback(void* (*cb)(const tlFixedString&, uint32_t));
void apkSetRootDirectory(const char* path);
apkFile* apkLoadFileInPlace(void* data, bool invokeCallbacks);
apkFile* apkLoadFile(const char* filename);
void apkDeleteFile(apkFile* file);

} // namespace apk
