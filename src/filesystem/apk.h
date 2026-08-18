// ============================================================================
// APK - Asset Package (APKF format) file loader
// Source: source/apk.cpp (line refs: 0x7B, 0xAA, 0x109)
// ea: 0x834050-0x834D70 (23 functions)
// ============================================================================

#pragma once

#include <stdint.h>
#include "core/tlFixedString.h"

namespace apk {

// ============================================================================
// apkFileSection - a data section within the APK file (24 bytes)
// Verified against apk.o: Name fixed up at +0x00; Size + Data read at +0x10 /
// +0x14 (GetDataSize ea 0x8342B0, apkLoadFileInPlace ea 0x834B40).
// ============================================================================
struct apkFileSection {
    tlFixedString* Name; // +0x00
    uint32_t        Flags; // +0x04
    uint32_t        Alignment; // +0x08
    uint32_t        MemFlags; // +0x0C
    uint32_t        Size; // +0x10
    void*           Data; // +0x14
};
static_assert(sizeof(apkFileSection) == 0x18, "apkFileSection size mismatch");

// Forward
class apkFile;
class apkFileEntry;

// ============================================================================
// apkFileTypeEntry - registered file type within the APK.
// 20-byte header + NSections dword section table (entry stride 4*NSections+20).
// Verified against apk.o (GetFileTypeEntry ea 0x8343C0, InvokeLoadCallbacks
// ea 0x8346B0).
// ============================================================================
struct apkFileTypeEntry {
    uint32_t Type;              // +0x00
    uint32_t Version;           // +0x04
    uint32_t NSections;         // +0x08
    apkFileEntry* FirstEntry;   // +0x0C - relative until fixed up at load
    uint32_t NEntries;          // +0x10
    // +0x14: uint32_t SectionData[NSections] - (sectionIndex<<24)|offset
};
static_assert(sizeof(apkFileTypeEntry) == 0x14, "apkFileTypeEntry size mismatch");

// ============================================================================
// apkFileEntry - individual file entry (variable size: 4 + 4*NSections)
// ============================================================================
class apkFileEntry {
public:
    tlFixedString* Name;    // +0x00 - fixed-up name
    void*       Sections[1];// +0x04 - per-section data pointers (NSections dwords)

    void* GetData(apkFile* file, int section, bool assertIfNoData);
    uint32_t GetDataSize(apkFile* file, int section, bool assertIfNoData, void** dataPtr);
};

// ============================================================================
// apkFile - loaded APK file object (in-place loaded).
// The in-memory object starts at file offset 8 (the Flags field); Magic and
// Version live at file offsets 0 and 4, before the struct. Verified against
// apk.o: FileTypes at +0x10, NSections at +0x08, Sections at +0x0C
// (GetFileTypeEntry / GetSectionIndex disasm), LastFrameRef at +0x04
// (nglCanReleaseFile ea 0x840FF0 reads [File+4]).
// ============================================================================
class apkFile {
public:
    uint32_t        Flags;      // +0x00 - bit 0=in-place, bit 1=owned
    int             LastFrameRef; // +0x04
    uint32_t        NSections;  // +0x08
    apkFileSection* Sections;   // +0x0C
    apkFileTypeEntry* FileTypes;// +0x10 - first file type entry

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
void apkSetResourceCallback(void* (*cb)(const tlFixedString*, uint32_t));
void apkSetRootDirectory(const char* path);
apkFile* apkLoadFileInPlace(void* data, bool invokeCallbacks);
apkFile* apkLoadFile(const char* filename);
void apkDeleteFile(apkFile* file);

} // namespace apk
