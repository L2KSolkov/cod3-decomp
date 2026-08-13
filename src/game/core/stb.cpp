// ============================================================================
// stb.cpp - STBManager + DbTablesetMgr (core.o STBManager.cpp)
// ============================================================================

#include "game/core/core_systems.h"

#include <string.h>

// Minimal view of PakManager (full class in game/sv/sv_stubs.h).
class PakManager {
public:
    static PakManager* sInst;
};  // ?sInst@PakManager@@2PAV1@A

extern unsigned int AeHash(const char* str);
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

// ?STBManager_sInst@@3PAUSTBManager@@A (core.o)
STBManager* STBManager_sInst = nullptr;
// streamer.o helpers (stubs, port later)
unsigned int* InplaceTree_Find_U32(void* tree, unsigned int* key)
{
    (void)tree; (void)key;
    return nullptr;
}
void* InplaceAssetBank_Index(void* bank, int i)
{
    (void)bank; (void)i;
    return nullptr;
}
void PtrFixupTable_Fixup(void* self, void* basePtr)
{
    (void)self; (void)basePtr;
}
void InplaceAssetBankSet_AddBank_DbTableset(void* self, TPakId pak, void* bank)
{
    (void)self; (void)pak; (void)bank;
}
void InplaceAssetBankSet_Find_DbTableset(void* self, void* result,
                                        TPakId pakId, const char* key,
                                        void* formal, void* foundPakId)
{
    (void)self; (void)result; (void)pakId; (void)key;
    (void)formal; (void)foundPakId;
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
    // In-place bank decode: fixup the fixup table at offset 6, then register.
    if (*(unsigned int*)(data + 6) >= 0x10000000u)
    {
        // assertion: fixup offset unusually large
    }
    void* v13 = &data[*(unsigned int*)(data + 6)];
    *(unsigned int*)(data + 6) = (unsigned int)v13;
    PtrFixupTable_Fixup(v13, data);
    mBankArray[pak_id] = data;
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
