// ============================================================================
// tlSkipList<T, Key> - probabilistic skip list (used by ngl resource dirs).
// Source: streamer.o / ngl_internal.o / ngl_aux.o COMDATs.
// Instance = { T* DataPtr; Instance** Forward; } ; list = { rand state, level,
// head }. Add/Del/Find/Destroy/Init/RandomLevel are the inline methods.
// ============================================================================

#ifndef COD3_CORE_TL_SKIPLIST_H
#define COD3_CORE_TL_SKIPLIST_H

#include "bd/bd_types.h"
#include "core/tlFixedString.h"
#include <stdlib.h>

struct _tlSkipListBase {
    unsigned int RandomBits;   // +0x00
    int          RandomsLeft;  // +0x04

    int RandomLevel() {
        int level = 0;
        while (level < 15) {
            if (RandomsLeft == 0) {
                RandomBits = (unsigned int)rand();
                RandomsLeft = 7;
            }
            if ((RandomBits & 1) == 0)
                break;
            RandomBits >>= 1;
            RandomsLeft--;
            level++;
        }
        return level;
    }
};

template <typename T, typename Key>
class tlSkipList : public _tlSkipListBase {
public:
    struct Instance {
        T*         DataPtr;   // +0x00
        Instance** Forward;   // +0x04 (array of Level+1)
    };

    int       Level;         // +0x08
    Instance* Head;          // +0x0C

    tlSkipList() {
        RandomBits = (unsigned int)rand();
        RandomsLeft = 7;
        Head = NULL;
        Level = 0;
    }

    void Init() {
        Head = (Instance*)tlMemAlloc(8u, 8u, 0);
        Head->DataPtr = NULL;
        Head->Forward = (Instance**)tlMemAlloc(4 * 16, 8u, 0);
        for (int i = 0; i < 16; i++)
            Head->Forward[i] = NULL;
        Level = 0;
    }

    static const tlFixedString* GetKeyOf(const T* t) { return ::GetKey(t); }

    // Skip-list comparison: 0 equal, -1 key < instance key, +1 key > instance key.
    static int CompareKey(const tlFixedString& key, const tlFixedString& other) {
        unsigned int kh = key.hash;
        unsigned int oh = other.hash;
        for (int i = 0; i < 8; i++) {
            if (kh == oh) {
                if (i == 7)
                    return 0;
                kh = key.str[i];
                oh = other.str[i];
                continue;
            }
            return kh < oh ? -1 : 1;
        }
        return 0;
    }

    T* Find(const tlFixedString& key) const {
        Instance* p = Head;
        if (p == NULL)
            return NULL;
        int k = Level;
        Instance* f = NULL;
        do {
            f = p->Forward[k];
            while (f != NULL) {
                int cmp = CompareKey(key, *GetKeyOf(f->DataPtr));
                if (cmp == 0)
                    return f->DataPtr;
                if (cmp > 0)
                    break;
                p = f;
                f = f->Forward[k];
            }
            --k;
        } while (k >= 0);
        return NULL;
    }

    T* Add(T* dataPtr) {
        if (Head == NULL)
            Init();
        const tlFixedString* key = GetKeyOf(dataPtr);
        Instance* update[16];
        Instance* p = Head;
        int k = Level;
        Instance* f = NULL;
        do {
            f = p->Forward[k];
            while (f != NULL) {
                int cmp = CompareKey(*key, *GetKeyOf(f->DataPtr));
                if (cmp == 0)
                    break;
                if (cmp > 0)
                    break;
                p = f;
                f = f->Forward[k];
            }
            update[k] = p;
            --k;
        } while (k >= 0);
        if (f != NULL)
            return f->DataPtr;

        int newLevel = RandomLevel();
        if (newLevel > Level) {
            newLevel = Level + 1;
            Level = newLevel;
            update[newLevel] = Head;
        }
        Instance* node = (Instance*)tlMemAlloc(4 * newLevel + 8, 8u, 0);
        node->DataPtr = dataPtr;
        node->Forward = (Instance**)tlMemAlloc(4 * (newLevel + 1), 8u, 0);
        for (int i = newLevel; i >= 0; i--) {
            node->Forward[i] = update[i]->Forward[i];
            update[i]->Forward[i] = node;
        }
        return NULL;
    }

    void Del(T* dataPtr) {
        Instance* update[16];
        Instance* p = Head;
        int k = Level;
        Instance* f = NULL;
        if (p == NULL)
            return;
        const tlFixedString* key = GetKeyOf(dataPtr);
        do {
            f = p->Forward[k];
            while (f != NULL) {
                int cmp = CompareKey(*key, *GetKeyOf(f->DataPtr));
                if (cmp == 0)
                    break;
                if (cmp > 0)
                    break;
                p = f;
                f = f->Forward[k];
            }
            update[k] = p;
            --k;
        } while (k >= 0);
        if (f == NULL)
            return;
        for (int i = 0; i <= Level; i++) {
            if (update[i]->Forward[i] == f)
                update[i]->Forward[i] = f->Forward[i];
        }
        tlMemFree(f->Forward);
        tlMemFree(f);
    }

    void Destroy() {
        Instance* head = Head;
        if (head != NULL) {
            for (Instance* i = head->Forward[0]; i != NULL; i = Head->Forward[0])
                Del(i->DataPtr);
            tlMemFree(head->Forward);
            tlMemFree(head);
            Head = NULL;
            Level = 0;
        }
    }
};

#endif // COD3_CORE_TL_SKIPLIST_H