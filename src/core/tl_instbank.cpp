// ============================================================================
// TL Instance Bank — skip-list-based named instance registry
// Source: tl_instbank.cpp (9 funcs)
// ea: 0x833B90-0x833FC0
// ============================================================================

#include "core/tlFixedString.h"
#include <cstdlib>
#include <cstring>

// Forward
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);

// ============================================================================
// tlInstanceBank — randomized skip-list container
// ============================================================================
class tlInstanceBank {
public:
    struct Instance {
        tlFixedString Key;       // +0x00 (32 bytes: hash + str[28])
        void*         Value;     // +0x20
        unsigned      RefCount;  // +0x24
        // Variable: Forward[] array at +0x28 (4 bytes per level)
        // Forward[0] at +0x28, Forward[N] at +0x28 + 4*N
    };

    static const int MAX_LEVEL = 15;

    Instance* NIL;       // sentinel node
    Instance* Head;      // head of skip list
    unsigned   Level;    // current max level
    unsigned   RandomBits;
    int        RandomsLeft;

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

// ============================================================================
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
    if (NIL) Destroy();

    NIL = NewNodeOfLevel(MAX_LEVEL);
    memset(NIL, 0, 4 * MAX_LEVEL + 44);
    Head = NIL;
    Level = 0;
    RandomBits = rand();
    RandomsLeft = 7;

    NIL->Key.hash = 0xFFFFFFFF;
    NIL->Key.str[0] = 0;
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

            // Compare Key strings
            const unsigned char* k1 = (const unsigned char*)&q->Key.hash;
            const unsigned char* k2 = (const unsigned char*)&key.hash;
            int cmp = 0;
            for (int j = 0; j < 32; ++j) {
                if (k1[j] != k2[j]) {
                    cmp = k1[j] < k2[j] ? -1 : 1;
                    break;
                }
            }

            if (cmp >= 0) break;
            p = q;
        }
        update[i] = p;
    }

    // Check if key already exists
    Instance* q = *(Instance**)((char*)p + 40); // Forward[0]
    if (memcmp(&q->Key, &key, sizeof(tlFixedString)) == 0) {
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
            const unsigned char* k1 = (const unsigned char*)&q->Key.hash;
            const unsigned char* k2 = (const unsigned char*)&key.hash;
            int cmp = 0;
            for (int j = 0; j < 32; ++j) {
                if (k1[j] != k2[j]) { cmp = k1[j] < k2[j] ? -1 : 1; break; }
            }
            if (cmp >= 0) break;
            p = q;
        }
        update[i] = p;
    }

    Instance* q = *(Instance**)((char*)p + 40); // Forward[0]
    if (memcmp(&q->Key, &key, sizeof(tlFixedString)) != 0) return -1;

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

            const unsigned char* k1 = (const unsigned char*)&q->Key.hash;
            const unsigned char* k2 = (const unsigned char*)&key.hash;
            int cmp = 0;
            for (int j = 0; j < 32; ++j) {
                if (k1[j] != k2[j]) { cmp = k1[j] < k2[j] ? -1 : 1; break; }
            }

            if (cmp == 0) return q; // found
            if (cmp > 0) break;     // past insertion point
            p = q;
        }
    }

    // Final check at base level
    Instance* q = *(Instance**)((char*)p + 40);
    if (memcmp(&q->Key, &key, sizeof(tlFixedString)) == 0) return q;
    return nullptr;
}
