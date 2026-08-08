// ============================================================================
// dialogue.cpp - DialogueManager (core.o DialogueManager.cpp)
// ============================================================================

#include "game/core/core_systems.h"

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

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

extern void InplaceAssetBank_Fixup_Dialogue(DialogueBank* data);
extern unsigned int* InplaceTree_Find_Dialogue(void* tree, unsigned int* key);

// DialogueManager.mBanks is opaque; expose element access
extern DialogueManager* DialogueManager_sInst;

// ea: 0x004C0BE0
void DialogueManager::UnloadBank(TPakId pakId)
{
    mBanks[pakId] = nullptr;
}

// ea: 0x004C5530
void DialogueManager::DecodeDialogueBank(const char* name, DialogueBank* data,
                                         int size, TPakId pakId)
{
    InplaceAssetBank_Fixup_Dialogue(data);
    if (mBanks[pakId] != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DialogueManager.cpp";
        AeAssert::gCurrentLine = 32;
        AeAssert::gCurrentExpr = "mBanks[pakId] == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("bank already loaded"))
            __debugbreak();
    }
    mBanks[pakId] = data;
}

// ea: 0x004C55B0
char* DialogueManager::GetDialogue(unsigned int hash)
{
    for (int v2 = 0; v2 < 99; ++v2)
    {
        DialogueBank* v3 = mBanks[v2];
        if (v3 != nullptr)
        {
            unsigned int* v4 = InplaceTree_Find_Dialogue(&v3->mData, &hash);
            if (v4 != nullptr)
            {
                // mPtrs[index] -> InplaceVector<InplaceString>
                void* vec = *(void**)((char*)&v3->mData + *v4);
                if (vec != nullptr)
                {
                    unsigned int mSize = *(unsigned int*)vec;
                    if (mSize != 0)
                    {
                        if (mSize == 1)
                            return ((char**)vec)[1];
                        unsigned int v8 = (*(unsigned int*)((char*)vec + 8) + 1) % mSize;
                        *(unsigned int*)((char*)vec + 8) = v8;
                        return ((char**)vec)[1 + v8];
                    }
                    return nullptr;
                }
            }
        }
    }
    return nullptr;
}

// ea: 0x004C5690
void DecodeDialogueBank(const char* name, unsigned char* data, int size,
                        TPakId pakId)
{
    DialogueManager_sInst->DecodeDialogueBank(name, (DialogueBank*)data, size,
                                              pakId);
}
