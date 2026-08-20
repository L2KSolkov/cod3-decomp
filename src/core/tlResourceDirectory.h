// ============================================================================
// tlResourceDirectory<T> - resource directory base (vtable-only, 4 bytes).
// Source: render_xboxr:ngl_aux.o COMDATs (and streamer.o).
//
// Layout (verified against IDA local types):
//   tlResourceDirectory<T>  { __vftable }                      (4 bytes)
//   tlResourceDirectory<T>::Iterator { __vftable }             (4 bytes)
//   tlResourceDirectory<T>::Iterator_vtbl (5 slots):
//      0 dtr, 1 reset, 2 operator(), 3 operator++, 4 operator*
//   tlResourceDirectory<T>_vtbl (9 slots):
//      0 dtr, 1 DirectoryName, 2 Find, 3 Add, 4 Del, 5 Enumerate,
//      6 ReleaseAll, 7 Load, 8 Release
//
// The default implementations are pure stubs that trap (the specialized
// delegates override them). Verified against:
//   StandardRelease 0x7C2FD0, StandardLoad 0x7C3010, ReleaseAll 0x7C3260,
//   Load 0x7C3360, Release 0x7C3370, DirectoryName 0x7C3250,
//   dtor 0x7C3240, ctor 0x7C3890.
// ============================================================================

#ifndef COD3_CORE_TL_RESOURCE_DIRECTORY_H
#define COD3_CORE_TL_RESOURCE_DIRECTORY_H

#include "core/tlFixedString.h"
#include "core/tlSkipList.h"

extern void tlWarning(const char* Format, ...);

template <typename T>
class tlResourceDirectory {
public:
    // IDA local type: tlResourceDirectory<T>::Iterator.
    class Iterator {
    public:
        virtual ~Iterator() {}
        virtual void reset() = 0;
        virtual bool operator()() = 0;
        virtual void operator++() = 0;
        virtual T* operator*() = 0;
    };

    // ??2tlResourceDirectory@@SAPAXI@Z (nal_init.o / ngl_aux.o)
    // The Xbox template routes directory allocations through tlMemAlloc with
    // the library's standard 8-byte alignment and no special flags.
    static void* operator new(unsigned int size) {
        return tlMemAlloc(size, 8u, 0u);
    }

    virtual ~tlResourceDirectory() {}

    virtual const char* DirectoryName() { return "<Unnamed>"; }

    virtual T* Find(const tlFixedString& key) {
        (void)key;
        return NULL;
    }

    virtual T* Add(T* DataPtr) {
        (void)DataPtr;
        return NULL;
    }

    virtual bool Del(T* const DataPtr) {
        (void)DataPtr;
        return false;
    }

    virtual Iterator* Enumerate() { return NULL; }

    virtual void ReleaseAll(bool warn, bool system, int maxforce) {
        (void)warn;
        (void)system;
        (void)maxforce;
    }

    virtual T* Load(const tlFixedString& key) { return StandardLoad(key); }

    virtual int Release(T* data, int force, bool system) {
        return StandardRelease(data, force, system);
    }

protected:
    T* StandardLoad(const tlFixedString& key) {
        (void)key;
        __debugbreak();
        return NULL;
    }

    int StandardRelease(T* data, int force, bool system) {
        (void)data;
        (void)force;
        (void)system;
        __debugbreak();
        return -1;
    }
};

static_assert(sizeof(tlResourceDirectory<int>) == 4, "tlResourceDirectory size mismatch");

// ============================================================================
// delegate_directory<T> - a tlResourceDirectory backed by an external
// tlSkipList<T, tlFixedString>. Layout: vftable (0x0) + skiplist (0x4) = 8 bytes.
// Verified against delegate_directory COMDATs:
//   ctor 0x7C38D0, dtor 0x7C3980/0x7C3950, Find 0x7C3B00, Del 0x7C3B10,
//   Add 0x7C4500, Enumerate 0x7C3C50, SkipListIterator 0x7C3C90..0x7C3CD0,
//   Init 0x7C4670.
// ============================================================================
template <typename T>
class delegate_directory : public tlResourceDirectory<T> {
public:
    delegate_directory(tlSkipList<T, tlFixedString>* external)
        : skiplist(external) {}

    virtual ~delegate_directory() {}

    virtual const char* DirectoryName() { return "<Unnamed>"; }

    virtual T* Find(const tlFixedString& key) { return skiplist->Find(key); }

    virtual T* Add(T* DataPtr) { return skiplist->Add(DataPtr); }

    virtual bool Del(T* const DataPtr) { return skiplist->Del(DataPtr); }

    class SkipListIterator : public tlResourceDirectory<T>::Iterator {
    public:
        SkipListIterator(tlSkipList<T, tlFixedString>* list)
            : skiplist(list), cur(list ? list->Head->Forward[0] : NULL) {}

        virtual ~SkipListIterator() {}

        virtual void reset() {
            if (skiplist && skiplist->Head)
                cur = skiplist->Head->Forward[0];
            else
                cur = NULL;
        }

        virtual bool operator()() { return cur != NULL; }

        virtual void operator++() {
            if (cur)
                cur = cur->Forward[0];
        }

        virtual T* operator*() {
            if (cur)
                return cur->DataPtr;
            return NULL;
        }

        tlSkipList<T, tlFixedString>* skiplist;
        typename tlSkipList<T, tlFixedString>::Instance* cur;
    };

    virtual typename tlResourceDirectory<T>::Iterator* Enumerate() {
        if (!skiplist)
            return NULL;
        SkipListIterator* it = (SkipListIterator*)tlMemAlloc(sizeof(SkipListIterator), 8u, 0);
        if (!it)
            return NULL;
        it->skiplist = skiplist;
        it->cur = skiplist->Head ? skiplist->Head->Forward[0] : NULL;
        return it;
    }

    virtual void ReleaseAll(bool warn, bool system, int maxforce) {
        tlResourceDirectory<T>::ReleaseAll(warn, system, maxforce);
    }

    virtual T* Load(const tlFixedString& key) {
        return tlResourceDirectory<T>::StandardLoad(key);
    }

    virtual int Release(T* data, int force, bool system) {
        return tlResourceDirectory<T>::StandardRelease(data, force, system);
    }

    tlSkipList<T, tlFixedString>* skiplist;
};

// tlInstanceBankResourceDirectory<T> stores its own skip list inline.  The
// nalAnimFile constructor at 0x867880 only initializes the base vtable and
// this member; Init() establishes the skip-list level when the first item is
// added.
template <typename T>
class tlInstanceBankResourceDirectory : public tlResourceDirectory<T> {
public:
    class SkipListIterator : public tlResourceDirectory<T>::Iterator {
    public:
        struct State {
            tlSkipList<T, tlFixedString>* skiplist;
            typename tlSkipList<T, tlFixedString>::Instance* cur;
        };

        explicit SkipListIterator(tlSkipList<T, tlFixedString>* list)
        {
            state.skiplist = list;
            state.cur = (list && list->Head) ? list->Head->Forward[0] : NULL;
        }

        virtual void reset()
        {
            typename tlSkipList<T, tlFixedString>::Instance* head =
                state.skiplist->Head;
            state.cur = head ? head->Forward[0] : NULL;
        }

        virtual bool operator()() { return state.cur != NULL; }

        virtual void operator++()
        {
            if (state.cur)
                state.cur = state.cur->Forward[0];
        }

        virtual T* operator*()
        {
            return state.cur ? state.cur->DataPtr : NULL;
        }

        State state;
    };

    tlInstanceBankResourceDirectory() : tlResourceDirectory<T>(), skiplist() {}

    virtual T* Find(const tlFixedString& key)
    {
        return skiplist.Find(key);
    }

    virtual bool Del(T* const data)
    {
        return skiplist.Del(data);
    }

    virtual typename tlResourceDirectory<T>::Iterator* Enumerate()
    {
        void* memory = tlMemAlloc(sizeof(SkipListIterator), 8u, 0u);
        if (!memory)
            return NULL;
        return new (memory) SkipListIterator(&skiplist);
    }

    virtual T* Add(T* data)
    {
        return skiplist.Add(data);
    }

    tlSkipList<T, tlFixedString> skiplist;
};

#endif // COD3_CORE_TL_RESOURCE_DIRECTORY_H
