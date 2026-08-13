// ============================================================================
// configstring.cpp - ConfigStringManager (core.o)
// ============================================================================

#include "game/core/core_systems.h"

#include <stdio.h>
#include <string.h>

extern void InplaceAssetBank_Fixup_ConfigString(ConfigStringBank* data);
extern void InplaceAssetBankSet_AddBank_ConfigString(void* self, TPakId pakId,
                                                     ConfigStringBank* bank);
extern void InplaceAssetBankSet_Find_ConfigString(void* self,
                                                 ConfigStringPtr* result,
                                                 TPakId pakId, const char* key,
                                                 void* formal, void* foundPakId);
// ?InplaceAssetBank_Fixup_ConfigString@@YAXPAUConfigStringBank@@@Z (stub)
void InplaceAssetBank_Fixup_ConfigString(ConfigStringBank* data)
{
    (void)data;
}
// ?InplaceAssetBankSet_AddBank_ConfigString@@YAXPAXW4TPakId@@PAUConfigStringBank@@@Z (stub)
void InplaceAssetBankSet_AddBank_ConfigString(void* self, TPakId pakId,
                                              ConfigStringBank* bank)
{
    (void)self; (void)pakId; (void)bank;
}
extern void InplaceAssetBankSet_PredicateSearch_ConfigString(
    void* self, ConfigStringPtr* result, TPakId pakId, void* op, void* formal);

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
