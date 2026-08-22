// ============================================================================
// stb.cpp - STBManager + DbTablesetMgr (core.o STBManager.cpp)
// ============================================================================

#include "game/core/core_systems.h"
#include "game/core/core_globals.h"

#include <string.h>
#include <new>
#include "core/mem_heap.h"

extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);

// IDA-backed views used by STBManager::DecodeBank.  The complete definitions
// live in streamer/pakmanager.cpp; these declarations preserve the same
// offsets without coupling core.o to that translation unit's implementation.
struct PakHeader {
    struct File;
    struct Bank;
    struct Section {
        char name[8];
        int sectionId;
        unsigned short numBanks;
        unsigned short numFiles;
        Bank* banks;
        File* files;
    };
    struct File {
        const char* shortName;
        const char* longName;
        unsigned int fileSize;
        unsigned int bankIndex;
        unsigned int bankOffset;
    };
    struct Bank {
        unsigned int fileOffset;
        unsigned int size;
        unsigned int capacity;
        unsigned int flags;
        unsigned int compressedSize;
        int requestId;
        unsigned char* memptr;
        unsigned int memsize;
    };
    unsigned int id;
    float version;
    Section* sections;
    unsigned int numSections;
    unsigned int persistentSize;
    unsigned int headerShortSize;
    unsigned int headerLongSize;
    char packedBy[16];
    unsigned int packedInfo;
};
static_assert(sizeof(PakHeader::Section) == 0x18, "PakHeader::Section layout mismatch");
static_assert(sizeof(PakHeader::File) == 0x14, "PakHeader::File layout mismatch");
static_assert(sizeof(PakHeader::Bank) == 0x20, "PakHeader::Bank layout mismatch");
static_assert(sizeof(PakHeader) == 0x30, "PakHeader layout mismatch");

class NumBanks {
public:
    float ps2;
    float ps3_main;
    float ps3_lram;
    float xbox;
    float xenon;
    float pcx;
    float gc_main;
    float gc_aram;
};
static_assert(sizeof(NumBanks) == 0x20, "NumBanks layout mismatch");

struct mem_info {
    unsigned char* data;
    int size;
};
static_assert(sizeof(mem_info) == 0x8, "mem_info layout mismatch");

class BankManager {
public:
    static BankManager* sInst;
    TBankAlloc mFreeBanks;
    float mNumMramBanks;
    unsigned int mMramBankSize;
    unsigned char* mMramArena;
    float mLowestFreeAmount;
    TBankAlloc mLastAlloc;

    TBankAlloc Allocate(NumBanks numBanks);
    mem_info get_alloc(const TBankAlloc& alloc, int which,
                       bool mram) const;
    void release(TBankAlloc& alloc);
};
static_assert(sizeof(BankManager) == 0x30, "BankManager layout mismatch");

// Minimal view of PakManager (full class in game/sv/sv_stubs.h).
class PakManager {
public:
    unsigned char mPrefix[0x40];
    PakFile* mSlots[99];
    static PakManager* sInst;
};
static_assert(offsetof(PakManager, mSlots) == 0x40,
              "PakManager::mSlots offset mismatch");

extern unsigned int codNflReadFile(nflFileID fileID, unsigned int fileOffset,
                                   void* buffer, unsigned int dataSize);

extern "C" unsigned int AeHash(const char* str);

// ea: 0x004DF4B0
unsigned int STBManager_hash(const unsigned char* str)
{
    return AeHash(reinterpret_cast<const char*>(str));
}

extern unsigned int* InplaceTree_Find_U32(void* tree, unsigned int* key);
extern void* InplaceAssetBank_Index(void* bank, int i);
extern void InplaceAssetBank_Fixup_ConfigString(void* data);
extern void InplaceAssetBankSet_AddBank_ConfigString(void* self, TPakId pakId,
                                                     void* bank);
extern void InplaceAssetBankSet_AddBank_DbTableset(void* self, TPakId pakId,
                                                   void* bank);
extern void InplaceAssetBankSet_Find_DbTableset(void* self, void* result,
                                                TPakId pakId, const char* key,
                                                void* formal, void* foundPakId);
extern void PtrFixupTable_Fixup(void* self, void* basePtr);
extern int PakManager_GetPakFile(void* self, TPakId pakId);
extern void GetPakPrerequisites(TPakId pakId,
                                ae_sized_array<TPakId, 32>* ret);

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

struct U32TreeElement {
    unsigned int mKey;
    unsigned int mValue;
};

struct StringTreeElement {
    char* mKey;
    unsigned int mValue;
};

struct GenericAssetBankLayout {
    unsigned int mFileId;
    float mVersion;
    unsigned int mTreeSize;
    void* mTreeArray;
    unsigned int mPtrsSize;
    void** mPtrsList;
    void* mPtrFixupTable;
};

class PtrFixupTable {
public:
    void Fixup(const void* basePtr);
};

static bool U32TreeIsUsed(GenericAssetBankLayout* tree,
                          U32TreeElement* elements, unsigned int index)
{
    if (index >= tree->mTreeSize) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
        AeAssert::gCurrentLine = 211;
        AeAssert::gCurrentExpr = "index >= 0 && index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    static const unsigned char nullElement[sizeof(U32TreeElement)] = {};
    return memcmp(&elements[index], nullElement, sizeof(U32TreeElement)) != 0;
}

static unsigned int* U32TreeFind(GenericAssetBankLayout* tree,
                                 unsigned int* key)
{
    U32TreeElement* elements =
        reinterpret_cast<U32TreeElement*>(tree->mTreeArray);
    unsigned int index = 0;
    if (tree->mTreeSize == 0)
        return nullptr;
    for (;;) {
        if (!U32TreeIsUsed(tree, elements, index)) {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
            AeAssert::gCurrentLine = 101;
            AeAssert::gCurrentExpr = "IsUsed(index)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("index must be used"))
                __debugbreak();
        }
        unsigned int nodeKey = elements[index].mKey;
        if (nodeKey == *key)
            return &elements[index].mValue;
        index = nodeKey >= *key ? (2 * index + 1) : (2 * index + 2);
        if (index >= tree->mTreeSize
            || !U32TreeIsUsed(tree, elements, index)
            || index >= tree->mTreeSize)
            return nullptr;
    }
}

static bool StringTreeIsUsed(GenericAssetBankLayout* tree,
                              StringTreeElement* elements,
                              unsigned int index)
{
    if (index >= tree->mTreeSize) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
        AeAssert::gCurrentLine = 211;
        AeAssert::gCurrentExpr = "index >= 0 && index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    static const unsigned char nullElement[sizeof(StringTreeElement)] = {};
    return memcmp(&elements[index], nullElement,
                  sizeof(StringTreeElement)) != 0;
}

static unsigned int* StringTreeFind(GenericAssetBankLayout* tree,
                                    const char* key)
{
    StringTreeElement* elements =
        reinterpret_cast<StringTreeElement*>(tree->mTreeArray);
    unsigned int index = 0;
    if (tree->mTreeSize == 0)
        return nullptr;
    for (;;) {
        if (!StringTreeIsUsed(tree, elements, index)) {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
            AeAssert::gCurrentLine = 101;
            AeAssert::gCurrentExpr = "IsUsed(index)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("index must be used"))
                __debugbreak();
        }
        int comparison = _stricmp(elements[index].mKey, key);
        if (comparison == 0)
            return &elements[index].mValue;
        index = comparison >= 0 ? (2 * index + 1) : (2 * index + 2);
        if (index >= tree->mTreeSize
            || !StringTreeIsUsed(tree, elements, index)
            || index >= tree->mTreeSize)
            return nullptr;
    }
}

struct DbTablesetManagerLayout {
    void* mVtable;
    void* mBankArray[99];
};
static_assert(sizeof(DbTablesetManagerLayout) == 0x190,
              "DbTablesetMgr layout mismatch");

// ?STBManager_sInst@@3PAUSTBManager@@A (core.o)
STBManager* STBManager_sInst = nullptr;

// ea: 0x4DC7E0
void* STBManager::operator new(size_t size, void* p)
{
    (void)size;
    return p;
}

// ea: 0x4DC7F0
STBManager* STBManager::Inst()
{
    return STBManager::sInst;
}

// ea: 0x4E8800 (core.o inline)
void STBManager::CreateInst()
{
    if (sInst != nullptr
        && _tlAssert("c:\\cod\\code\\game\\STBManager.h", 46,
                     "sInst==0", "singleton already created!"))
        __debugbreak();

    void* memory = mem_heap_malloc_ctx(
        0x190u, 4, "core", "c:\\cod\\code\\game\\STBManager.h", 46);
    if (memory != nullptr)
    {
        sInst = new (memory) STBManager();
        STBManager_sInst = sInst;
    }
    else
    {
        sInst = nullptr;
        STBManager_sInst = nullptr;
    }
}

// ea: 0x4DC800 (core.o inline)
void STBManager::DeleteInst()
{
    if (sInst == nullptr
        && _tlAssert("c:\\cod\\code\\game\\STBManager.h", 46,
                     "sInst!=0", "singleton not created!"))
        __debugbreak();
    if (sInst != nullptr)
    {
        sInst->~STBManager();
        mem_heap_free(sInst);
    }
    sInst = nullptr;
    STBManager_sInst = nullptr;
}

// streamer.o helpers (stubs, port later)
unsigned int* InplaceTree_Find_U32(void* tree, unsigned int* key)
{
    return U32TreeFind(reinterpret_cast<GenericAssetBankLayout*>(
                           reinterpret_cast<unsigned char*>(tree) - 8),
                       key);
}
void* InplaceAssetBank_Index(void* bank, int i)
{
    GenericAssetBankLayout* assetBank =
        reinterpret_cast<GenericAssetBankLayout*>(bank);
    if ((unsigned int)i >= assetBank->mPtrsSize) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
        AeAssert::gCurrentLine = 199;
        AeAssert::gCurrentExpr = "i<mPtrs.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("bounds check"))
            __debugbreak();
    }
    return assetBank->mPtrsList[i];
}
void PtrFixupTable_Fixup(void* self, void* basePtr)
{
    reinterpret_cast<PtrFixupTable*>(self)->Fixup(basePtr);
}
void InplaceAssetBankSet_AddBank_DbTableset(void* self, TPakId pak, void* bank)
{
    DbTablesetManagerLayout* manager =
        reinterpret_cast<DbTablesetManagerLayout*>(self);
    void*& slot = manager->mBankArray[(int)pak];
    if (slot != nullptr) {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\InplaceAssetBankSet.h";
        AeAssert::gCurrentLine = 109;
        AeAssert::gCurrentExpr = "mBankArray[(int)pakId] == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("We already have a bank for this pak id!"))
            __debugbreak();
    }
    slot = bank;
}
void InplaceAssetBankSet_Find_DbTableset(void* self, void* result,
                                        TPakId pakId, const char* key,
                                        void* formal, void* foundPakId)
{
    (void)formal;
    DbTablesetManagerLayout* manager =
        reinterpret_cast<DbTablesetManagerLayout*>(self);
    IVPointer<DbTableSet>* output =
        reinterpret_cast<IVPointer<DbTableSet>*>(result);
    ae_sized_array<TPakId, 32> prereqs;
    prereqs.m_size = 0;
    if (pakId == PAK_ID_INVALID) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\InplaceAssetBankSet.h";
        AeAssert::gCurrentLine = 121;
        AeAssert::gCurrentExpr = "pakId != PAK_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("bad pak id"))
            __debugbreak();
        output->mValue = nullptr;
        output->mPakId = PAK_ID_INVALID;
        return;
    }
    GetPakPrerequisites(pakId, &prereqs);
    for (unsigned int i = 0; i < (unsigned int)prereqs.m_size; ++i) {
        if (i >= 0x20) {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        TPakId candidate = prereqs.m_elements[i];
        if (candidate == PAK_ID_INVALID)
            continue;
        GenericAssetBankLayout* bank =
            reinterpret_cast<GenericAssetBankLayout*>(
                manager->mBankArray[(int)candidate]);
        if (bank == nullptr)
            continue;
        unsigned int* index = StringTreeFind(bank, key);
        if (index != nullptr) {
            if (foundPakId != nullptr)
                *reinterpret_cast<TPakId*>(foundPakId) = candidate;
            output->mPakId = candidate;
            output->mValue = const_cast<DbTableSet*>(
                reinterpret_cast<const DbTableSet*>(
                    bank->mPtrsList[*index]));
            return;
        }
    }
    output->mValue = nullptr;
    output->mPakId = PAK_ID_INVALID;
}

// ea: 0x004C5D00
const StringTableEntry* STBManager::GetSTBEntry(TPakId pakId, unsigned int hash)
{
    if (pakId != -1)
    {
        void* v3 = mBankArray[pakId];
        if (v3 != nullptr)
        {
            unsigned int* v4 = InplaceTree_Find_U32(&((char*)v3)[8], &hash);
            if (v4 != nullptr)
                return (const StringTableEntry*)InplaceAssetBank_Index(v3, *v4);
        }
    }
    return nullptr;
}

// ea: 0x004C5D50
const StringTableEntry* STBManager::GetSTBEntry(unsigned int hash)
{
    for (int v2 = 0; v2 < 99; ++v2)
    {
        void* v4 = mBankArray[v2];
        if (v4 != nullptr)
        {
            unsigned int key = hash;
            unsigned int* v5 = InplaceTree_Find_U32(&((char*)v4)[8], &key);
            if (v5 != nullptr)
            {
                const StringTableEntry* result =
                    (const StringTableEntry*)InplaceAssetBank_Index(v4, *v5);
                if (result != nullptr)
                    return result;
            }
        }
    }
    return nullptr;
}

// ea: 0x004C5E00
const StringTableEntry* STBManager::GetSTBEntry(const char* pszReference)
{
    if (pszReference != nullptr)
        return GetSTBEntry(AeHash(pszReference));
    return nullptr;
}

// ea: 0x004C5E30
const char* STBManager::GetSTBString(const char* pszReference)
{
    if (pszReference == nullptr)
        return "NO STRING";
    char buf[512];
    strcpy(buf, pszReference);
    char* v2 = buf + strlen(buf);
    strcpy(v2, "_XBOX");
    const StringTableEntry* STBEntry = STBManager_sInst->GetSTBEntry(AeHash(buf));
    if (STBEntry == nullptr)
    {
        STBEntry = STBManager_sInst->GetSTBEntry(AeHash(pszReference));
        if (STBEntry == nullptr)
            return pszReference;
    }
    char* result = STBEntry->mLoc.mStr;
    if (result == nullptr)
        return "NO STRING";
    return result;
}

// ea: 0x004C5EF0
const char* STBManager::GetSTBString(unsigned int hash)
{
    const StringTableEntry* STBEntry = STBManager_sInst->GetSTBEntry(hash);
    if (STBEntry == nullptr)
        return nullptr;
    char* result = STBEntry->mLoc.mStr;
    if (result == nullptr)
        return "STRING MISSING";
    return result;
}

// ea: 0x004C5F30
const char* STBManager::GetSTBString(TPakId pakId, unsigned int hash)
{
    if (pakId == -1)
        return nullptr;
    void* v3 = STBManager_sInst->mBankArray[pakId];
    if (v3 == nullptr)
        return nullptr;
    unsigned int* v4 = InplaceTree_Find_U32(&((char*)v3)[8], &hash);
    if (v4 == nullptr)
        return nullptr;
    StringTableEntry* v5 =
        (StringTableEntry*)InplaceAssetBank_Index(v3, *v4);
    if (v5 == nullptr)
        return nullptr;
    char* result = v5->mLoc.mStr;
    if (result == nullptr)
        return "STRING MISSING";
    return result;
}

// ea: 0x004C5FA0
unsigned int STBManager::GetSTBFlags(const char* pszReference)
{
    if (pszReference != nullptr)
    {
        const StringTableEntry* STBEntry =
            STBManager_sInst->GetSTBEntry(AeHash(pszReference));
        if (STBEntry != nullptr)
            return STBEntry->mFlags;
    }
    return 0;
}

// ea: 0x004C5FE0
unsigned int STBManager::GetSTBFlags(unsigned int hash)
{
    const StringTableEntry* STBEntry = STBManager_sInst->GetSTBEntry(hash);
    if (STBEntry != nullptr)
        return STBEntry->mFlags;
    return 0;
}

// ea: 0x004C6010
unsigned int STBManager::GetSTBFlags(TPakId pakId, unsigned int hash)
{
    if (pakId != -1)
    {
        void* v3 = STBManager_sInst->mBankArray[pakId];
        if (v3 != nullptr)
        {
            unsigned int* v4 = InplaceTree_Find_U32(&((char*)v3)[8], &hash);
            if (v4 != nullptr)
            {
                StringTableEntry* v5 =
                    (StringTableEntry*)InplaceAssetBank_Index(v3, *v4);
                if (v5 != nullptr)
                    return v5->mFlags;
            }
        }
    }
    return 0;
}

// ea: 0x004C6070
void STBManager::DecodeBank(const char* name, unsigned char* data, int size,
                            TPakId pak_id)
{
    (void)name;
    (void)size;
    TPakId v5 = pak_id;
    int sectionIndex = 0;
    PakFile* pakFile = nullptr;
    if (pak_id > 0x62)
        pakFile = nullptr;
    else
        pakFile = PakManager::sInst->mSlots[pak_id];

    PakHeader* header = pakFile->mHeader;
    unsigned int fileIndex = 0;
    unsigned char* bankData = data;
    if (header->numSections != 0)
    {
        PakHeader::Section* section = nullptr;
        while (true)
        {
            section = &header->sections[sectionIndex];
            if (strncmp(section->name, "STRING", 6) == 0)
                break;
            ++sectionIndex;
            if (++fileIndex >= header->numSections)
                goto fixup_bank;
        }

        if (section->numBanks != 1)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\STBManager.cpp";
            AeAssert::gCurrentLine = 299;
            AeAssert::gCurrentExpr = "sect.numBanks == 1";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("check"))
                __debugbreak();
        }

        int selectedFile = 0;
        fileIndex = 0;
        if (section->numFiles != 0)
        {
            PakHeader::File* file = nullptr;
            while (true)
            {
                file = &section->files[selectedFile];
                const char* shortName = file->shortName;
                bool selected =
                    (gLanguage == kLanguageEnglish
                     && strstr(shortName, ".en.") != nullptr)
                    || (gLanguage == kLanguageGerman
                        && strstr(shortName, ".de.") != nullptr)
                    || (gLanguage == kLanguageFrench
                        && strstr(shortName, ".fr.") != nullptr)
                    || (gLanguage == kLanguageSpanish
                        && strstr(shortName, ".es.") != nullptr)
                    || (gLanguage == kLanguageItalian
                        && strstr(shortName, ".it.") != nullptr);
                if (selected)
                    break;
                ++selectedFile;
                if (++fileIndex >= section->numFiles)
                    goto fixup_bank;
            }

            unsigned int offset =
                file->bankOffset + section->banks[file->bankIndex].fileOffset;
            NumBanks banks;
            banks.ps2 = 1.0f;
            banks.ps3_main = 1.0f;
            banks.ps3_lram = 0.0f;
            banks.xbox = 1.0f;
            banks.xenon = 1.0f;
            banks.pcx = 1.0f;
            banks.gc_main = 1.0f;
            banks.gc_aram = 0.0f;
            TBankAlloc allocation = BankManager::sInst->Allocate(banks);
            mem_info memory =
                BankManager::sInst->get_alloc(allocation, 0, true);
            codNflReadFile(
                pakFile->mFileId, offset, memory.data,
                (file->fileSize + 0x7FFFu) & 0xFFFF8000u);
            memcpy(data, memory.data, file->fileSize);
            bankData = data;
            BankManager::sInst->release(allocation);
        }
    }

fixup_bank:
    if (*(unsigned int*)(bankData + 0x18) >= 0x10000000u)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
        AeAssert::gCurrentLine = 122;
        AeAssert::gCurrentExpr = "((unsigned)mPtrFixupTable<0x10000000)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Fixup offset is unusually large"))
            __debugbreak();
    }
    void* v13 = &bankData[*(unsigned int*)(bankData + 0x18)];
    *(unsigned int*)(bankData + 0x18) = (unsigned int)v13;
    PtrFixupTable_Fixup(v13, bankData);

    if (v5 < 0 || v5 > 0x62)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
        AeAssert::gCurrentLine = 31;
        AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    if (v5 >= 0 && v5 <= 0x62 && mBankArray[v5] != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\InplaceAssetBankSet.h";
        AeAssert::gCurrentLine = 109;
        AeAssert::gCurrentExpr = "mBankArray[(int)pakId] == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("We already have a bank for this pak id!"))
            __debugbreak();
    }
    if (v5 >= 0 && v5 <= 0x62)
        mBankArray[v5] = bankData;
}

// ea: 0x004C5CB0
void DecodeConfigStrings(const char* name, unsigned char* data, int size,
                         TPakId pakId, PakFile* pakFile)
{
    (void)name; (void)size; (void)pakFile;
    InplaceAssetBank_Fixup_ConfigString(data);
    InplaceAssetBankSet_AddBank_ConfigString(ConfigStringManager::sInst, pakId,
                                             data);
}

// ea: 0x004C5500
void DbTablesetMgr_DecodeBank(const char* name, void* data, int size,
                              TPakId pakId)
{
    extern void* DbTablesetMgr_sInst;
    InplaceAssetBankSet_AddBank_DbTableset(DbTablesetMgr_sInst, pakId, data);
}

// ea: 0x004CA630
IVPointer<DbTableSet> DbTablesetMgr::GetTableSet(TPakId pakId,
                                                 const char* id) const
{
    extern void* DbTablesetMgr_sInst;
    IVPointer<DbTableSet> result;
    InplaceAssetBankSet_Find_DbTableset(DbTablesetMgr_sInst, &result, pakId, id,
                                        0, nullptr);
    return result;
}
