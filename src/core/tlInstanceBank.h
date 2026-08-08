// ============================================================================
// tlInstanceBank - randomized skip-list named instance registry.
// Source: tl_instbank.cpp (tl_xboxr:tl_instbank.o, 9 funcs, PORTED).
// Layout verified against IDA: tlInstanceBank 0x14, Instance 0x2C.
// ============================================================================
#ifndef COD3_CORE_TL_INSTANCE_BANK_H
#define COD3_CORE_TL_INSTANCE_BANK_H

#include "core/tlFixedString.h"

// ============================================================================
// tlInstanceBank - randomized skip-list container
// ============================================================================
class tlInstanceBank {
public:
    struct Instance {
        tlFixedString Key;       // +0x00 (32 bytes: hash + str[28])
        void*         Value;     // +0x20
        unsigned      RefCount;  // +0x24
        // Variable: Forward[] array at +0x28 (4 bytes per level)
        Instance*     Forward[1];  // +0x28
    };
    static_assert(sizeof(Instance) == 0x2C, "tlInstanceBank::Instance size mismatch");

    static const int MAX_LEVEL = 15;

    Instance* NIL;       // +0x00 sentinel node
    Instance* Head;      // +0x04 head of skip list
    unsigned  Level;     // +0x10 current max level
    unsigned  RandomBits;// +0x0C
    int       RandomsLeft;  // +0x08

    tlInstanceBank();
    ~tlInstanceBank();

    void     Init();
    void     Destroy();
    Instance* Insert(const tlFixedString& key, void* value);
    int      Delete(const tlFixedString& key);
    Instance* Search(const tlFixedString& key);

private:
    int      RandomLevel();
    Instance* NewNodeOfLevel(int level);
};
static_assert(sizeof(tlInstanceBank) == 0x14, "tlInstanceBank size mismatch");

#endif // COD3_CORE_TL_INSTANCE_BANK_H
