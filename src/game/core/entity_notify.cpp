// ============================================================================
// entity_notify.cpp - EntityNotifySet (core.o)
// ============================================================================

#include "game/core/core_systems.h"

extern void* PoolAllocator_Allocate(void* allocator, unsigned int s,
                                    bool forceHeapAlloc);
extern void PoolAllocator_Release(void* allocator, void* ptr);
extern void* EntityNotify_sAllocator;
extern void* EntityNotifySet_sAllocator;
extern void WaitTilOutput_AssignData(void* self, void* data);
extern void WaitTilOutput_Dtor(void* self, int flags);

// ============================================================================
// EntityNotifySet internals
// ============================================================================

// ea: 0x004C1E50
void EntityNotifySet::AddNotify(const HashString& h,
                                DbLinkedHandle<EntityHandleDb, Entity> owner)
{
    void* v4 = PoolAllocator_Allocate(EntityNotify_sAllocator, 0x14u, false);
    EntityNotify* n = (EntityNotify*)v4;
    if (v4 == nullptr)
        return;
    n->mStr = h.mHash;
    n->mOwner.mHandle.mVal = owner.mHandle.mVal;
    n->mParam = nullptr;
    // link into mStrings list
    n->m_dlist_node.mNext = mStrings.mRoot.mNext;
    n->m_dlist_node.mPrev = mStrings.mRoot.mPrev;
    if (mStrings.mRoot.mPrev)
        mStrings.mRoot.mPrev->mNext = &n->m_dlist_node;
    mStrings.mRoot.mPrev = &n->m_dlist_node;
    if (!mStrings.mRoot.mNext)
        mStrings.mRoot.mNext = &n->m_dlist_node;
}

// ea: 0x004C6450
EntityNotify* EntityNotifySet::GetNotify(const HashString& chk) const
{
    for (EntityNotify* n = (EntityNotify*)mStrings.mRoot.mNext;
         n != nullptr; n = (EntityNotify*)n->m_dlist_node.mNext)
    {
        if (n->mStr == chk.mHash)
            return n;
    }
    return nullptr;
}

// ea: 0x004C64D0
bool EntityNotifySet::AssignScriptVariable(const HashString& chk,
                                          WaitTilOutput* scriptVariable)
{
    EntityNotify* Notify = GetNotify(chk);
    if (Notify == nullptr)
        return false;
    WaitTilOutput* mParam = Notify->mParam;
    if (mParam == nullptr)
        return false;
    WaitTilOutput_AssignData(mParam, scriptVariable);
    return true;
}

// ea: 0x004C6500
bool EntityNotifySet::IsFinished()
{
    EntityNotify* n = (EntityNotify*)mStrings.mRoot.mNext;
    while (n != nullptr)
    {
        EntityNotify* next = (EntityNotify*)n->m_dlist_node.mNext;
        WaitTilOutput* mParam = n->mParam;
        if (mParam != nullptr)
        {
            WaitTilOutput_Dtor(mParam, 1);
            n->mParam = nullptr;
        }
        PoolAllocator_Release(EntityNotify_sAllocator, n);
        n = next;
    }
    mStrings.mRoot.mNext = nullptr;
    mStrings.mRoot.mPrev = nullptr;
    return mStrings.mRoot.mNext == nullptr && mEndOnList.mRoot.mNext == nullptr;
}

// ea: 0x004C6590
void EntityNotifySet::KillEndOnThreads()
{
    mEndOnList.mRoot.mNext = nullptr;
    mEndOnList.mRoot.mPrev = nullptr;
}

// ea: 0x004CFBA0
void EntityNotifySet::UpdateList()
{
    extern reserved_dlist<EntityNotifySet> sEntityNotifySet;
    EntityNotifySet* n = (EntityNotifySet*)sEntityNotifySet.mRoot.mNext;
    while (n != nullptr)
    {
        EntityNotifySet* next = (EntityNotifySet*)n->m_dlist_node.mNext;
        if (n->IsFinished() && n != nullptr)
        {
            PoolAllocator_Release(EntityNotifySet_sAllocator, n);
        }
        n = next;
    }
    sEntityNotifySet.mRoot.mNext = nullptr;
    sEntityNotifySet.mRoot.mPrev = nullptr;
}
