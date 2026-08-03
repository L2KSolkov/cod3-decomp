// ============================================================================
// APK — Asset Package file loader implementation
// Source: source/apk.cpp (line refs: 0x7B, 0xAA, 0x109)
// ea: 0x834050-0x834D70 (23 functions)
// ============================================================================

#include "apk.h"
#include <cstring>
#include <cstdio>
#include <cstdint>

namespace apk {

apkFileCallbackListEntry*    apkFileCallbackList = nullptr;
apkSectionCallbackListEntry* apkSectionCallbackList = nullptr;
char apkRootDirectory[256] = "";
void* (*apkResourceLocatorCallback)(const tlFixedString&, uint32_t) = nullptr;

// External stubs
extern void  tlWarning(const char* fmt, ...);
extern void* tlMemAlloc(uint32_t size, uint32_t align, uint32_t flags);
extern void  tlMemFree(void* ptr);
extern bool  tlReadFile(const char* path, void* buf, uint32_t size, uint32_t offset);
extern bool  _tlAssert(const char* file, int line, const char* cond, const char* msg);

// ============================================================================
// apkRegisterFileType
// ea: 0x834050
// ============================================================================
void apkRegisterFileType(uint32_t FourCC, uint32_t Version,
    void (*load)(apkFile*, apkFileEntry*, void*),
    void (*del)(apkFile*, apkFileEntry*, void*), void* userData)
{
    apkFileCallbackListEntry* entry = (apkFileCallbackListEntry*)tlMemAlloc(0x18, 0, 0);
    entry->FourCC = FourCC;
    entry->Version = Version;
    entry->LoadCallback = (void*)load;
    entry->DeleteCallback = (void*)del;
    entry->UserData = userData;
    entry->Next = apkFileCallbackList;
    apkFileCallbackList = entry;
}

// ============================================================================
// apkUnregisterFileType
// ea: 0x834090
// ============================================================================
void apkUnregisterFileType(uint32_t FourCC, uint32_t Version) {
    apkFileCallbackListEntry** prev = &apkFileCallbackList;
    for (apkFileCallbackListEntry* cur = apkFileCallbackList; cur; cur = cur->Next) {
        if (cur->FourCC == FourCC && cur->Version == Version) {
            *prev = cur->Next;
            tlMemFree(cur);
            return;
        }
        prev = &cur->Next;
    }
}

// ============================================================================
// apkRegisterSectionType
// ea: 0x834100
// ============================================================================
void apkRegisterSectionType(const tlFixedString& Name,
    void (*load)(apkFile*, apkFileSection*, void*),
    void (*del)(apkFile*, apkFileSection*, void*), void* userData)
{
    apkSectionCallbackListEntry* entry = (apkSectionCallbackListEntry*)tlMemAlloc(0x30, 0, 0);
    memcpy(&entry->Name, &Name, sizeof(tlFixedString));
    entry->LoadCallback = (void*)load;
    entry->DeleteCallback = (void*)del;
    entry->UserData = userData;
    entry->Next = apkSectionCallbackList;
    apkSectionCallbackList = entry;
}

// ============================================================================
// apkUnregisterSectionType
// ea: 0x834170
// ============================================================================
void apkUnregisterSectionType(const tlFixedString& Name) {
    apkSectionCallbackListEntry** prev = &apkSectionCallbackList;
    for (apkSectionCallbackListEntry* cur = apkSectionCallbackList; cur; cur = cur->Next) {
        if (memcmp(&cur->Name, &Name, sizeof(tlFixedString)) == 0) {
            *prev = cur->Next;
            tlMemFree(cur);
            return;
        }
        prev = &cur->Next;
    }
}

// ============================================================================
// apkSetResourceCallback
// ea: 0x834200
// ============================================================================
void apkSetResourceCallback(void* (*cb)(const tlFixedString&, uint32_t)) {
    apkResourceLocatorCallback = cb;
}

// ============================================================================
// apkSetRootDirectory
// ea: 0x834930
// ============================================================================
void apkSetRootDirectory(const char* path) {
    strncpy(apkRootDirectory, path, 255);
    apkRootDirectory[255] = 0;
}

// ============================================================================
// apkFileEntry::GetData
// ea: 0x834210
// ============================================================================
void* apkFileEntry::GetData(apkFile* file, int section, bool assertIfNoData) {
    apkFileTypeEntry* typeEntry = file->FileTypes;
    while (typeEntry->Type) {
        if (file->NSections == 0) break;
        for (uint32_t i = 0; i < typeEntry->NEntries; ++i) {
            uint8_t* ptr = typeEntry->FirstEntry + i * (4 * file->NSections + 4);
            apkFileEntry* e = (apkFileEntry*)ptr;
            if (e != this) continue;

            uint8_t* secInfo = (uint8_t*)(typeEntry + 1);
            uint8_t secOffset = secInfo[section * 4 + 3];
            if (secOffset == 255) {
                if (assertIfNoData) {
                    _tlAssert("source/apk.cpp", 0x7B, "", "");
                    __builtin_debugtrap();
                }
                return nullptr;
            }
            uint8_t* entryBytes = (uint8_t*)this;
            return *(void**)(entryBytes + secOffset);
        }
        typeEntry = (apkFileTypeEntry*)((uint8_t*)typeEntry + 4 * file->NSections + 20);
    }
    return nullptr;
}

// ============================================================================
// apkFileEntry::GetDataSize
// ea: 0x8342B0
// ============================================================================
uint32_t apkFileEntry::GetDataSize(apkFile* file, int section, bool assertIfNoData, void** dataPtr) {
    void* found = nullptr;
    apkFileTypeEntry* typeEntry = file->FileTypes;

    while (typeEntry->Type) {
        for (uint32_t i = 0; i < typeEntry->NEntries; ++i) {
            uint8_t* ptr = typeEntry->FirstEntry + i * (4 * file->NSections + 4);
            apkFileEntry* e = (apkFileEntry*)ptr;

            if (found) {
                uint8_t* secInfo = (uint8_t*)(typeEntry + 1);
                uint8_t secOffset = secInfo[section * 4 + 3];
                if (secOffset != 255) {
                    if (dataPtr) *dataPtr = found;
                    uint8_t* entryBytes = (uint8_t*)e;
                    uint8_t* endPtr = *(uint8_t**)(entryBytes + secOffset);
                    return (uint32_t)(endPtr - (uint8_t*)found);
                }
            } else if (e == this) {
                uint8_t* secInfo = (uint8_t*)(typeEntry + 1);
                uint8_t secOffset = secInfo[section * 4 + 3];
                if (secOffset == 255) {
                    if (assertIfNoData) {
                        _tlAssert("source/apk.cpp", 0xAA, "", "");
                        __builtin_debugtrap();
                    }
                    if (dataPtr) *dataPtr = nullptr;
                    return 0;
                }
                uint8_t* entryBytes = (uint8_t*)this;
                found = *(void**)(entryBytes + secOffset);
            }
        }
        typeEntry = (apkFileTypeEntry*)((uint8_t*)typeEntry + 4 * file->NSections + 20);
    }

    if (dataPtr) *dataPtr = found;
    if (!found) return 0;
    apkFileSection* sec = &file->Sections[section];
    return (uint32_t)((uint8_t*)sec->Data + sec->Size - (uint8_t*)found);
}

// ============================================================================
// apkFile::GetFileTypeEntry
// ea: 0x8343C0
// ============================================================================
apkFileTypeEntry* apkFile::GetFileTypeEntry(uint32_t type) {
    apkFileTypeEntry* entry = FileTypes;
    while (entry->Type) {
        if (entry->Type == type) return entry;
        entry = (apkFileTypeEntry*)((uint8_t*)entry + 4 * NSections + 20);
    }
    return nullptr;
}

// ============================================================================
// apkFile::GetFirstFile
// ea: 0x8343F0
// ============================================================================
apkFileEntry* apkFile::GetFirstFile(uint32_t type) {
    apkFileTypeEntry* te = GetFileTypeEntry(type);
    if (te && te->NEntries > 0) return (apkFileEntry*)te->FirstEntry;
    return nullptr;
}

// ============================================================================
// apkFile::GetNextFile
// ea: 0x834420
// ============================================================================
apkFileEntry* apkFile::GetNextFile(uint32_t type, apkFileEntry* current) {
    apkFileTypeEntry* te = GetFileTypeEntry(type);
    if (!te) return nullptr;
    uint32_t stride = 4 * NSections + 4;
    for (uint32_t i = 0; i < (uint32_t)(te->NEntries - 1); ++i) {
        uint8_t* ptr = te->FirstEntry + i * stride;
        if ((apkFileEntry*)ptr == current)
            return (apkFileEntry*)(ptr + stride);
    }
    return nullptr;
}

// ============================================================================
// apkFile::GetFile (by index)
// ea: 0x834470
// ============================================================================
apkFileEntry* apkFile::GetFile(uint32_t idx) {
    apkFileTypeEntry* te = FileTypes;
    while (te->Type) {
        if (te->NEntries > idx)
            return (apkFileEntry*)(te->FirstEntry + idx * (4 * NSections + 4));
        idx -= te->NEntries;
        te = (apkFileTypeEntry*)((uint8_t*)te + 4 * NSections + 20);
    }
    _tlAssert("source/apk.cpp", 0x109, "", "");
    __builtin_debugtrap();
    return nullptr;
}

// ============================================================================
// apkFile::GetFile (by name)
// ea: 0x8344D0
// ============================================================================
apkFileEntry* apkFile::GetFile(const tlFixedString& name, uint32_t type) {
    apkFileTypeEntry* te = GetFileTypeEntry(type);
    if (!te) return nullptr;
    uint32_t stride = 4 * NSections + 4;
    for (uint32_t i = 0; i < te->NEntries; ++i) {
        apkFileEntry* e = (apkFileEntry*)(te->FirstEntry + i * stride);
        if (memcmp(e->Name, name.str, 28) == 0) return e;
    }
    return nullptr;
}

// ============================================================================
// apkFile::GetSectionIndex
// ea: 0x834570
// ============================================================================
int apkFile::GetSectionIndex(const tlFixedString& name) {
    for (uint32_t i = 0; i < NSections; ++i) {
        if (memcmp(Sections[i].Name, name.str, 28) == 0) return (int)i;
    }
    return -1;
}

// ============================================================================
// apkFile::ApplyFixups
// ea: 0x8345C0
// ============================================================================
void apkFile::ApplyFixups(uint32_t** fixupData, tlFixedString* stringTable) {
    while (**fixupData != 0xFFFFFFFF) {
        uint32_t val = **fixupData;
        ++(*fixupData);
        uint32_t secIdx = val >> 26;
        uint32_t idx = val & 0x3FFFFFF;
        uint32_t* ptr = (uint32_t*)((uint8_t*)Sections[secIdx].Data + 4 * idx);

        uint32_t target = *ptr;
        uint32_t targetSec = target >> 26;
        uint32_t targetOff = 4 * (target & 0x3FFFFFF);

        if (targetSec == 63)
            *ptr = (uint32_t)(uintptr_t)((uint8_t*)stringTable + targetOff);
        else
            *ptr = (uint32_t)(uintptr_t)((uint8_t*)Sections[targetSec].Data + targetOff);
    }
    ++(*fixupData);
}

// ============================================================================
// apkFile::ApplyReferences
// ea: 0x834630
// ============================================================================
void apkFile::ApplyReferences(uint32_t** refData, tlFixedString* stringTable) {
    while (**refData != 0xFFFFFFFF) {
        uint32_t val = **refData;
        ++(*refData);
        uint32_t secIdx = val >> 26;
        uint32_t idx = val & 0x3FFFFFF;
        uint32_t* ptr = (uint32_t*)((uint8_t*)Sections[secIdx].Data + 4 * idx);

        uint32_t stringIndex = *(*refData)++;
        (*refData)++; // skip second word

        const tlFixedString* name = (const tlFixedString*)((uint8_t*)stringTable + stringIndex);
        if (apkResourceLocatorCallback)
            *ptr = (uint32_t)(uintptr_t)apkResourceLocatorCallback(*name, 0);
        else
            *ptr = 0;
    }
    ++(*refData);
}

// ============================================================================
// apkFile::InvokeLoadCallbacks
// ea: 0x8346B0
// ============================================================================
void apkFile::InvokeLoadCallbacks(bool invokeFileCallbacks) {
    for (uint32_t i = 0; i < NSections; ++i) {
        apkFileSection* sec = &Sections[i];
        for (apkSectionCallbackListEntry* cb = apkSectionCallbackList; cb; cb = cb->Next) {
            if (memcmp(&cb->Name, sec->Name, 32) == 0) {
                if (cb->LoadCallback)
                    ((void(*)(apkFile*, apkFileSection*, void*))cb->LoadCallback)(this, sec, cb->UserData);
                break;
            }
        }
    }

    if (invokeFileCallbacks) {
        apkFileTypeEntry* te = FileTypes;
        while (te->Type) {
            for (apkFileCallbackListEntry* cb = apkFileCallbackList; cb; cb = cb->Next) {
                if (cb->FourCC == te->Type) {
                    if (cb->LoadCallback) {
                        uint32_t stride = 4 * NSections + 4;
                        for (uint32_t j = 0; j < te->NEntries; ++j) {
                            apkFileEntry* e = (apkFileEntry*)(te->FirstEntry + j * stride);
                            ((void(*)(apkFile*, apkFileEntry*, void*))cb->LoadCallback)(this, e, cb->UserData);
                        }
                    }
                    break;
                }
            }
            te = (apkFileTypeEntry*)((uint8_t*)te + 4 * NSections + 20);
        }
    }
}

// ============================================================================
// apkFile::InvokeDeleteCallbacks
// ea: 0x834820
// ============================================================================
void apkFile::InvokeDeleteCallbacks() {
    apkFileTypeEntry* te = FileTypes;
    while (te->Type) {
        for (apkFileCallbackListEntry* cb = apkFileCallbackList; cb; cb = cb->Next) {
            if (cb->FourCC == te->Type) {
                if (cb->DeleteCallback) {
                    uint32_t stride = 4 * NSections + 4;
                    for (uint32_t j = 0; j < te->NEntries; ++j) {
                        apkFileEntry* e = (apkFileEntry*)(te->FirstEntry + j * stride);
                        ((void(*)(apkFile*, apkFileEntry*, void*))cb->DeleteCallback)(this, e, cb->UserData);
                    }
                }
                break;
            }
        }
        te = (apkFileTypeEntry*)((uint8_t*)te + 4 * NSections + 20);
    }

    for (uint32_t i = 0; i < NSections; ++i) {
        apkFileSection* sec = &Sections[i];
        for (apkSectionCallbackListEntry* cb = apkSectionCallbackList; cb; cb = cb->Next) {
            if (memcmp(&cb->Name, sec->Name, 32) == 0) {
                if (cb->DeleteCallback)
                    ((void(*)(apkFile*, apkFileSection*, void*))cb->DeleteCallback)(this, sec, cb->UserData);
                break;
            }
        }
    }
}

// ============================================================================
// apkFile::InvokeFileLoadCallback
// ea: 0x8347D0
// ============================================================================
bool apkFile::InvokeFileLoadCallback(apkFileTypeEntry* typeEntry, apkFileEntry* entry) {
    for (apkFileCallbackListEntry* cb = apkFileCallbackList; cb; cb = cb->Next) {
        if (cb->FourCC == typeEntry->Type) {
            if (cb->LoadCallback) {
                ((void(*)(apkFile*, apkFileEntry*, void*))cb->LoadCallback)(this, entry, cb->UserData);
                return true;
            }
        }
    }
    return false;
}

// ============================================================================
// apkFile::ApplyReferencesForEntry
// ea: 0x834990
// ============================================================================
void apkFile::ApplyReferencesForEntry(apkFileEntry* entry) {
    // Stub — deferred to full reconstruction
}

// ============================================================================
// apkLoadFileInPlace
// ea: 0x834B40
// ============================================================================
apkFile* apkLoadFileInPlace(void* data, bool invokeCallbacks) {
    uint32_t* hdr = (uint32_t*)data;
    uint8_t*  base = (uint8_t*)data;

    if (hdr[0] != *(uint32_t*)"APKF") {
        tlWarning("Invalid APK file.\n");
        return nullptr;
    }
    if (hdr[1] != 260) {
        tlWarning("APK file version 0x%08x detected. Supported: 0x%08x.\n", hdr[1], 260);
        return nullptr;
    }
    if (hdr[2] & 4) {
        tlWarning("APK is big-endian, host is little-endian.\n");
        return nullptr;
    }

    apkFile* file = (apkFile*)hdr;
    file->Flags = hdr[2] | 1;
    uint32_t nSec = file->NSections;

    // Fix up sections pointers
    uint8_t* secData = (uint8_t*)&file->Sections;
    file->Sections = (apkFileSection*)(secData + (uintptr_t)file->Sections);
    file->FileTypes = (apkFileTypeEntry*)(secData + (uintptr_t)file->FileTypes);

    for (uint32_t i = 0; i < nSec; ++i) {
        apkFileSection* s = &file->Sections[i];
        s->Name = (const char*)(base + (uintptr_t)s->Name);
        s->Data = (void*)(base + (uintptr_t)s->Data);
    }

    // Fix up file type entries and file entries
    uint32_t sectionOffsets[64] = {};
    apkFileTypeEntry* te = file->FileTypes;
    while (te->Type) {
        uint32_t stride = 4 * nSec + 4;
        te->FirstEntry = (uint8_t*)te + *(uint32_t*)((uint8_t*)te + 12) + 12; // fixup

        for (uint32_t i = 0; i < te->NEntries; ++i) {
            apkFileEntry* e = (apkFileEntry*)(te->FirstEntry + i * stride);
            e->Name = (const char*)((uint8_t*)e + (uintptr_t)e->Name);

            for (uint32_t j = 0; j < nSec; ++j) {
                uint32_t secWord = ((uint32_t*)((uint8_t*)(te + 1)))[j];
                if ((secWord >> 24) != 255) {
                    uint32_t align = secWord & 0xFFFFFF;
                    if (align)
                        sectionOffsets[j] = ~(align - 1) & (sectionOffsets[j] + align - 1);

                    uint8_t* entryBytes = (uint8_t*)e;
                    uint8_t off = ((uint8_t*)(te + 1))[j * 4 + 3];
                    uint8_t* sectionData = (uint8_t*)file->Sections[j].Data;
                    *(uint32_t*)(entryBytes + off) = (uint32_t)(uintptr_t)(sectionData + sectionOffsets[j]);
                    sectionOffsets[j] += *(uint32_t*)(entryBytes + off) - (uintptr_t)(sectionData + sectionOffsets[j]);
                    // Simplified — real code reads Size from entry sections
                }
            }
        }
        te = (apkFileTypeEntry*)((uint8_t*)te + 4 * nSec + 20);
    }

    // Apply fixups
    uint32_t* fixupPtr = (uint32_t*)(file->Sections + nSec);
    file->ApplyFixups(&fixupPtr, (tlFixedString*)((uint8_t*)te));
    if (invokeCallbacks)
        file->ApplyReferences(&fixupPtr, (tlFixedString*)((uint8_t*)te));

    file->InvokeLoadCallbacks(invokeCallbacks);
    return file;
}

// ============================================================================
// apkLoadFile
// ea: 0x834D70
// ============================================================================
apkFile* apkLoadFile(const char* filename) {
    char path[512];
    int rootLen = (int)strlen(apkRootDirectory);
    memcpy(path, apkRootDirectory, rootLen);
    strcpy(path + rootLen, filename);

    void* buf = nullptr;
    if (!tlReadFile(path, &buf, 0, 0)) {
        tlWarning("Unable to open %s.\n", path);
        return nullptr;
    }

    apkFile* file = apkLoadFileInPlace(buf, true);
    if (file) file->Flags |= 2;
    return file;
}

// ============================================================================
// apkDeleteFile
// ea: 0x834960
// ============================================================================
void apkDeleteFile(apkFile* file) {
    if (!file) return;
    file->InvokeDeleteCallbacks();
    if (file->Flags & 2) tlMemFree(file);
}

} // namespace apk
