// ============================================================================
// tlSkipList<T, Key> - probabilistic skip list (used by ngl resource dirs).
// Source: streamer.o / ngl_internal.o / ngl_aux.o COMDATs.
// Instance = { T* DataPtr; Instance* Forward[1]; } - the Forward array is
// INLINE after DataPtr: a level-l node is one tlMemAlloc(4*l+8) block, and the
// head is a single tlMemAlloc(0x44) block (DataPtr + Forward[0..15]).
// Verified against COMDATs: Init 0x7C4AC0, Add 0x7C49A0, Del 0x7C42D0,
// Find 0x685490, Destroy 0x841460, RandomLevel 0x7C4630, NewNodeOfLevel 0x844700.
// ============================================================================

#ifndef COD3_CORE_TL_SKIPLIST_H
#define COD3_CORE_TL_SKIPLIST_H

#include "core/tlFixedString.h"
#include <stdlib.h>

// tl_system.o (tl_xboxr, ported)
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);

struct _tlSkipListBase {
    unsigned int RandomBits;   // +0x00
    int          RandomsLeft;  // +0x04

    // Counts consecutive 2-bit-zero groups; capped at 15. ea: 0x7C4630
    int RandomLevel() {
        int level = 0;
        unsigned int b;
        do {
            b = RandomBits & 3;
            if (b == 0)
                ++level;
            RandomBits >>= 2;
            if (RandomsLeft-- == 1) {
                RandomBits = (unsigned int)rand();
                RandomsLeft = 7;
            }
        } while (b == 0);
        if (level > 15)
            return 15;
        return level;
    }
};

template <typename T, typename Key>
class tlSkipList : public _tlSkipListBase {
public:
    struct Instance {
        T*         DataPtr;   // +0x00
        Instance*  Forward[1];// +0x04 - inline array of (Level+1) pointers

        // ?Key@Instance@?$tlSkipList@...@@QBEABVtlFixedString@@XZ (streamer.o)
        const tlFixedString& Key() const { return *GetKeyOf(DataPtr); }
    };

    Instance* Head;          // +0x08
    int       Level;         // +0x0C

    tlSkipList() {
        RandomBits = (unsigned int)rand();
        RandomsLeft = 7;
        Head = NULL;
        // IDA constructors in nal_init.o initialize only the random state and
        // Head; Level is established by Init().
    }

    // GetKeyOf: free ::GetKey(T*) accessor (per-instantiation).
    static const tlFixedString* GetKeyOf(const T* t) { return ::GetKey(t); }

    // Key compare: 0 equal, +1 key < other, -1 key > other (8 dword compare).
    static int CompareKey(const tlFixedString& key, const tlFixedString& other) {
        const unsigned int* k = (const unsigned int*)&key;
        const unsigned int* o = (const unsigned int*)&other;
        for (int i = 0; i < 8; i++) {
            if (k[i] == o[i])
                continue;
            return k[i] < o[i] ? 1 : -1;
        }
        return 0;
    }

    // ea: 0x7C4AC0 - single 0x44 block: DataPtr + Forward[0..15].
    void Init() {
        Level = 0;
        Head = (Instance*)tlMemAlloc(0x44u, 8u, 0);
        for (int i = 0; i <= 15; ++i)
            Head->Forward[i] = NULL;
    }

    // ea: 0x844700
    Instance* NewNodeOfLevel(int l) {
        return (Instance*)tlMemAlloc(4 * l + 8, 8u, 0x1000000);
    }

    // ea: 0x685490
    T* Find(const tlFixedString& key) const {
        Instance* result = Head;
        if (result == NULL)
            return NULL;
        int k = Level;
        Instance* p = result;
        Instance* i;
        do {
            for (i = p->Forward[k]; i != NULL; i = i->Forward[k]) {
                const unsigned int* kd = (const unsigned int*)&key;
                const unsigned int* fd = (const unsigned int*)GetKeyOf(i->DataPtr);
                int v5 = 0;
                while (kd[v5] == fd[v5]) {
                    ++v5;
                    if (v5 >= 8)
                        return i->DataPtr;
                }
                if (kd[v5] < fd[v5])
                    break;
                p = i;
            }
            --k;
        } while (k >= 0);
        if (i != NULL && *GetKeyOf(i->DataPtr) == key)
            return i->DataPtr;
        return NULL;
    }

    // ea: 0x7C49A0
    T* Add(T* DataPtr) {
        if (Head == NULL)
            Init();
        const tlFixedString* Key = GetKeyOf(DataPtr);
        Instance* update[16];
        Instance* p = Head;
        int k = Level;
        Instance* f;
        do {
            f = p->Forward[k];
            if (f != NULL) {
                while (f != NULL) {
                    const unsigned int* kd = (const unsigned int*)Key;
                    const unsigned int* fd = (const unsigned int*)GetKeyOf(f->DataPtr);
                    int v8 = 0;
                    while (kd[v8] == fd[v8]) {
                        ++v8;
                        if (v8 >= 8)
                            goto found;
                    }
                    if (kd[v8] < fd[v8])
                        break;
                    p = f;
                    f = f->Forward[k];
                }
            }
        found:
            update[k] = p;
            --k;
        } while (k >= 0);
        if (f != NULL) {
            const unsigned int* kd = (const unsigned int*)Key;
            const unsigned int* fd = (const unsigned int*)GetKeyOf(f->DataPtr);
            int v12 = 0;
            while (kd[v12] == fd[v12]) {
                ++v12;
                if (v12 >= 8)
                    return f->DataPtr;
            }
        }
        int newLevel = RandomLevel();
        if (newLevel > Level) {
            newLevel = Level + 1;
            Level = newLevel;
            update[newLevel] = Head;
        }
        Instance* node = (Instance*)tlMemAlloc(4 * newLevel + 8, 8u, 0);
        node->DataPtr = DataPtr;
        Instance** v19 = &node->Forward[newLevel];
        do {
            Instance* v20 = update[newLevel];
            *v19 = v20->Forward[newLevel];
            v20->Forward[newLevel--] = node;
            --v19;
        } while (newLevel >= 0);
        return NULL;
    }

    // ea: 0x7C42D0
    bool Del(const T* DataPtr) {
        if (Head == NULL)
            return false;
        const tlFixedString* Key = GetKeyOf(DataPtr);
        int Level = this->Level;
        int m = Level;
        int k = Level;
        Instance* p = Head;
        Instance* update[16];
        Instance* f;
        do {
            f = p->Forward[k];
            if (f != NULL) {
                while (f != NULL) {
                    const unsigned int* kd = (const unsigned int*)Key;
                    const unsigned int* fd = (const unsigned int*)GetKeyOf(f->DataPtr);
                    int v9 = 0;
                    while (kd[v9] == fd[v9]) {
                        ++v9;
                        if (v9 >= 8)
                            goto next;
                    }
                    if (kd[v9] < fd[v9])
                        break;
                    p = f;
                    f = f->Forward[k];
                }
            }
        next:
            update[k] = p;
            --k;
        } while (k >= 0);
        if (f == NULL || f->DataPtr != DataPtr)
            return false;
        int v13 = 0;
        if (m >= 0) {
            Instance** Forward = f->Forward;
            do {
                Instance* v15 = update[v13];
                if (v15->Forward[v13] != f)
                    break;
                v15->Forward[v13++] = *Forward++;
            } while (v13 <= m);
        }
        tlMemFree(f);
        if (Head->Forward[m] == NULL) {
            unsigned int v16;
            do {
                if (m <= 0)
                    break;
                v16 = (unsigned int)Head->Forward[m - 1];
                --m;
            } while (v16 == 0);
        }
        this->Level = m;
        return true;
    }

    // ea: 0x841460
    void Destroy() {
        Instance* head = Head;
        if (head != NULL) {
            for (Instance* i = head->Forward[0]; i != NULL; i = Head->Forward[0])
                Del(i->DataPtr);
            tlMemFree(Head);
            Head = NULL;
        }
    }
};

#endif // COD3_CORE_TL_SKIPLIST_H
