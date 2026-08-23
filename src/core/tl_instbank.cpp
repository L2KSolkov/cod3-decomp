// ============================================================================
// TL Instance Bank — skip-list-based named instance registry
// Source: tl_instbank.cpp (9 funcs)
// ea: 0x833B90-0x833FC0

#include "core/tlInstanceBank.h"

#include <cstdlib>
#include <cstring>

// Forward
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);

// tlInstanceBank::tlInstanceBank
// ea: 0x833B90
// ============================================================================
tlInstanceBank::tlInstanceBank() : NIL(nullptr), Head(nullptr) {
    Init();
}

// ============================================================================
// tlInstanceBank::Init
// ea: 0x833FC0
// ============================================================================
void tlInstanceBank::Init() {
    if (NIL != nullptr)
        return;

    NIL = (Instance*)tlMemAlloc(0x2C, 8, 0);
    for (int i = 0; i < 8; ++i)
        ((uint32_t*)&NIL->Key)[i] = 0xFFFFFFFFu;

    RandomBits = rand();
    RandomsLeft = 7;
    Level = 0;
    Head = (Instance*)tlMemAlloc(0x6C, 8, 0);
    for (int offset = 0x28; offset < 0x6C; offset += 4)
        *(Instance**)((uint8_t*)Head + offset) = NIL;
}

// ============================================================================
// tlInstanceBank::~tlInstanceBank
// ea: 0x833FB0
// ============================================================================
tlInstanceBank::~tlInstanceBank() {
    Destroy();
}

// ============================================================================
// tlInstanceBank::Destroy
// ea: 0x833F50
// ============================================================================
void tlInstanceBank::Destroy() {
    Instance* cur = Head;
    while (cur != NIL) {
        Instance* next = *(Instance**)((char*)cur + 4 * Level + 40);
        tlMemFree(cur);
        cur = next;
    }
    tlMemFree(NIL);
    NIL = nullptr;
    Head = nullptr;
}

// ============================================================================
// tlInstanceBank::NewNodeOfLevel
// ea: 0x833BA0
// ============================================================================
tlInstanceBank::Instance* tlInstanceBank::NewNodeOfLevel(int l) {
    return (Instance*)tlMemAlloc(4 * l + 44, 0, 0);
}

// ============================================================================
// tlInstanceBank::RandomLevel
// ea: 0x833BC0
// ============================================================================
int tlInstanceBank::RandomLevel() {
    int level = 0;
    int r;
    do {
        r = RandomBits & 3;
        if (!r) ++level;
        RandomBits >>= 2;
        if (--RandomsLeft == 0) {
            RandomBits = rand();
            RandomsLeft = 7;
        }
    } while (!r);

    return (level <= MAX_LEVEL) ? level : MAX_LEVEL;
}

// ============================================================================
// tlInstanceBank::Insert
// ea: 0x833C00
// ============================================================================
tlInstanceBank::Instance* tlInstanceBank::Insert(const tlFixedString& key, void* value) {
    Instance* update[MAX_LEVEL + 1];
    Instance* p = Head;
    int k = (int)Level;

    // Search for insertion point, tracking update[] nodes per level
    for (int i = (int)Level; i >= 0; --i) {
        while (true) {
            Instance* q = *(Instance**)((char*)p + 4 * i + 40);

            // The release skip list compares the eight tlFixedString dwords,
            // not their byte representation.
            if (key.Order(q->Key) <= 0)
                break;
            p = q;
        }
        update[i] = p;
    }

    // Check if key already exists
    Instance* q = *(Instance**)((char*)p + 40); // Forward[0]
    if (q->Key.Order(key) == 0) {
        ++q->RefCount;
        return q;
    }

    // Insert new node
    int newLevel = RandomLevel();
    if (newLevel > (int)Level) {
        newLevel = (int)Level + 1;
        Level = newLevel;
        update[newLevel] = Head;
    }

    Instance* node = NewNodeOfLevel(newLevel);
    memcpy(&node->Key, &key, sizeof(tlFixedString));
    node->Value = value;
    node->RefCount = 1;

    // Splice into each level
    for (int i = newLevel; i >= 0; --i) {
        Instance** fwd = (Instance**)((char*)node + 4 * i + 40);
        Instance** fwdUpd = (Instance**)((char*)update[i] + 4 * i + 40);
        *fwd = *fwdUpd;
        *fwdUpd = node;
    }

    return node;
}

// ============================================================================
// tlInstanceBank::Delete
// ea: 0x833D50
// ============================================================================
int tlInstanceBank::Delete(const tlFixedString& key) {
    Instance* update[MAX_LEVEL + 1];
    Instance* p = Head;

    // Search
    for (int i = (int)Level; i >= 0; --i) {
        while (true) {
            Instance* q = *(Instance**)((char*)p + 4 * i + 40);
            if (key.Order(q->Key) <= 0)
                break;
            p = q;
        }
        update[i] = p;
    }

    Instance* q = *(Instance**)((char*)p + 40); // Forward[0]
    if (q->Key.Order(key) != 0) return -1;

    if (--q->RefCount > 0) return q->RefCount;

    // Unlink from each level
    for (int i = (int)Level; i >= 0; --i) {
        if (*(Instance**)((char*)update[i] + 4 * i + 40) == q) {
            Instance* fwd = *(Instance**)((char*)q + 4 * i + 40);
            *(Instance**)((char*)update[i] + 4 * i + 40) = fwd;
        }
    }

    tlMemFree(q);

    // Shrink level if top levels are empty
    while (Level > 0 && *(Instance**)((char*)Head + 4 * Level + 40) == NIL)
        --Level;

    return 0;
}

// ============================================================================
// tlInstanceBank::Search
// ea: 0x833EA0
// ============================================================================
tlInstanceBank::Instance* tlInstanceBank::Search(const tlFixedString& key) {
    Instance* p = Head;

    for (int i = (int)Level; i >= 0; --i) {
        while (true) {
            Instance* q = *(Instance**)((char*)p + 4 * i + 40);

            const int compare = key.Order(q->Key);
            if (compare == 0)
                return q;
            if (compare < 0)
                break;
            p = q;
        }
    }

    // Final check at base level
    Instance* q = *(Instance**)((char*)p + 40);
    if (q->Key.Order(key) == 0) return q;
    return nullptr;
}
