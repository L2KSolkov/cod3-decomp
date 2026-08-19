// ============================================================================
// configstring.cpp - ConfigStringManager (core.o)
// ============================================================================

#include "game/core/core_systems.h"

#include <stdio.h>
#include <string.h>

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

struct ConfigStringInplaceTree {
    unsigned int mSize;
    void* mArray;
};

struct ConfigStringBankLayout {
    unsigned int mFileId;
    float mVersion;
    ConfigStringInplaceTree mTree;
    InplaceVector<const ConfigString*> mPtrs;
    void* mPtrFixupTable;
};
static_assert(sizeof(ConfigStringBankLayout) == 0x1C,
              "ConfigStringBank layout mismatch");

struct ConfigStringManagerLayout {
    void* mVtable;
    ConfigStringBank* mBankArray[99];
};
static_assert(sizeof(ConfigStringManagerLayout) == 0x190,
              "ConfigStringManager layout mismatch");

struct ConfigStringTreeElement {
    InplaceString mKey;
    unsigned int mValue;
};

extern void PtrFixupTable_Fixup(void* self, void* basePtr);
extern void GetPakPrerequisites(TPakId pakId,
                                ae_sized_array<TPakId, 32>* ret);

static void ConfigStringFixupImpl(void* data)
{
    ConfigStringBankLayout* bank =
        reinterpret_cast<ConfigStringBankLayout*>(data);
    if (reinterpret_cast<uintptr_t>(bank->mPtrFixupTable) >= 0x10000000u) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
        AeAssert::gCurrentLine = 122;
        AeAssert::gCurrentExpr = "((unsigned)mPtrFixupTable<0x10000000)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Fixup offset is unusually large"))
            __debugbreak();
    }
    void* fixup = reinterpret_cast<unsigned char*>(bank)
                  + reinterpret_cast<uintptr_t>(bank->mPtrFixupTable);
    bank->mPtrFixupTable = fixup;
    PtrFixupTable_Fixup(fixup, bank);
}

// Both pointer spellings are present in the reconstructed callers; preserve
// each existing decoration while sharing the IDA-backed body.
void InplaceAssetBank_Fixup_ConfigString(ConfigStringBank* data)
{
    ConfigStringFixupImpl(data);
}
void InplaceAssetBank_Fixup_ConfigString(void* data)
{
    ConfigStringFixupImpl(data);
}

// ?InplaceAssetBankSet_AddBank_ConfigString@@YAXPAXW4TPakId@@PAUConfigStringBank@@@Z
static void ConfigStringAddBankImpl(void* self, TPakId pakId,
                                    ConfigStringBank* bank)
{
    ConfigStringManagerLayout* manager =
        reinterpret_cast<ConfigStringManagerLayout*>(self);
    ConfigStringBank*& slot = manager->mBankArray[(int)pakId];
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
void InplaceAssetBankSet_AddBank_ConfigString(void* self, TPakId pakId,
                                              ConfigStringBank* bank)
{
    ConfigStringAddBankImpl(self, pakId, bank);
}
void InplaceAssetBankSet_AddBank_ConfigString(void* self, TPakId pakId,
                                              void* bank)
{
    ConfigStringAddBankImpl(self, pakId,
                            reinterpret_cast<ConfigStringBank*>(bank));
}

static bool ConfigStringTreeIsUsed(ConfigStringBankLayout* bank,
                                   ConfigStringTreeElement* tree,
                                   unsigned int index)
{
    if (index >= bank->mTree.mSize) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
        AeAssert::gCurrentLine = 211;
        AeAssert::gCurrentExpr = "index >= 0 && index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    static const unsigned char nullElement[sizeof(ConfigStringTreeElement)] = {};
    return memcmp(&tree[index], nullElement,
                  sizeof(ConfigStringTreeElement)) != 0;
}

static unsigned int* ConfigStringTreeFind(ConfigStringBankLayout* bank,
                                           const char* key)
{
    if (bank->mTree.mSize == 0)
        return nullptr;
    ConfigStringTreeElement* tree =
        reinterpret_cast<ConfigStringTreeElement*>(bank->mTree.mArray);
    unsigned int index = 0;
    for (;;) {
        if (!ConfigStringTreeIsUsed(bank, tree, index)) {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
            AeAssert::gCurrentLine = 101;
            AeAssert::gCurrentExpr = "IsUsed(index)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("index must be used"))
                __debugbreak();
        }
        int comparison = _stricmp(tree[index].mKey.mStr, key);
        if (comparison == 0)
            return &tree[index].mValue;
        index = comparison >= 0 ? (2 * index + 1) : (2 * index + 2);
        if (index >= bank->mTree.mSize
            || !ConfigStringTreeIsUsed(bank, tree, index)
            || index >= bank->mTree.mSize)
            return nullptr;
    }
}

// ?InplaceAssetBankSet_Find_ConfigString@@YAXPAX...
void InplaceAssetBankSet_Find_ConfigString(void* self, ConfigStringPtr* result,
                                           TPakId pakId, const char* key,
                                           void*, void* foundPakId)
{
    ConfigStringManagerLayout* manager =
        reinterpret_cast<ConfigStringManagerLayout*>(self);
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
        result->mValue = nullptr;
        result->mPakId = PAK_ID_INVALID;
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
        ConfigStringBankLayout* bank =
            reinterpret_cast<ConfigStringBankLayout*>(
                manager->mBankArray[(int)candidate]);
        if (bank == nullptr)
            continue;
        unsigned int* index = ConfigStringTreeFind(bank, key);
        if (index != nullptr) {
            if (foundPakId != nullptr)
                *reinterpret_cast<TPakId*>(foundPakId) = candidate;
            result->mPakId = candidate;
            result->mValue = const_cast<ConfigString*>(
                bank->mPtrs.mList[*index]);
            return;
        }
    }
    result->mValue = nullptr;
    result->mPakId = PAK_ID_INVALID;
}

struct ConfigStringTypeCallbackSearch {
    const char* mType;
    void (*mCallback)(const char*, const ConfigString*);
    int mTypeLength;
};

// ?InplaceAssetBankSet_PredicateSearch_ConfigString@@YAXPAX...
void InplaceAssetBankSet_PredicateSearch_ConfigString(
    void* self, ConfigStringPtr* result, TPakId pakId, void* op, void*)
{
    ConfigStringManagerLayout* manager =
        reinterpret_cast<ConfigStringManagerLayout*>(self);
    ConfigStringTypeCallbackSearch& search =
        *reinterpret_cast<ConfigStringTypeCallbackSearch*>(op);
    ae_sized_array<TPakId, 32> prereqs;
    prereqs.m_size = 0;
    GetPakPrerequisites(pakId, &prereqs);
    for (unsigned int i = 0; i < (unsigned int)prereqs.m_size; ++i) {
        TPakId candidate = prereqs.m_elements[i];
        if (candidate == PAK_ID_INVALID)
            continue;
        ConfigStringBankLayout* bank =
            reinterpret_cast<ConfigStringBankLayout*>(
                manager->mBankArray[(int)candidate]);
        if (bank == nullptr || bank->mPtrs.mSize == 0)
            continue;
        for (unsigned int node = 0; node < bank->mPtrs.mSize; ++node) {
            const ConfigString* value = bank->mPtrs.mList[node];
            if (value != nullptr && value->mName.mStr != nullptr
                && strncmp(value->mName.mStr, search.mType,
                           search.mTypeLength) == 0)
                search.mCallback(value->mName.mStr + search.mTypeLength + 1,
                                 value);
        }
    }
    result->mValue = nullptr;
    result->mPakId = PAK_ID_INVALID;
}

// ea: 0x004C5C80
void ConfigStringManager::DecodeBank(const char* name, unsigned char* data,
                                     int size, TPakId pakId)
{
    InplaceAssetBank_Fixup_ConfigString((ConfigStringBank*)data);
    InplaceAssetBankSet_AddBank_ConfigString(this, pakId,
                                             (ConfigStringBank*)data);
}

// ea: 0x004CE8D0
IVPointer<ConfigString> ConfigStringManager::GetConfigString(
    TPakId pakId, const char* name, const char* type)
{
    char nm[128];
    nm[0] = 0;
    sprintf(nm, "%s.%s", name, type);
    ConfigStringPtr xm;
    InplaceAssetBankSet_Find_ConfigString(this, &xm, pakId, nm, 0, nullptr);
    IVPointer<ConfigString> result;
    result.mValue = xm.mValue;
    result.mPakId = xm.mPakId;
    return result;
}

// ea: 0x004CE940
void ConfigStringManager::CallbackSearch(TPakId pakId, const char* type,
                                         void (*callback)(const char*,
                                                          const ConfigString*))
{
    struct TypeCallbackSearch {
        const char* mType;
        void (*mCallback)(const char*, const ConfigString*);
        int mTypeLength;
    } v4;
    v4.mType = type;
    v4.mCallback = callback;
    v4.mTypeLength = (int)strlen(type);
    ConfigStringPtr result;
    InplaceAssetBankSet_PredicateSearch_ConfigString(this, &result, pakId,
                                                     &v4, 0);
}
