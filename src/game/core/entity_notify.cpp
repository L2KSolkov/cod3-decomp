// ============================================================================
// entity_notify.cpp - EntityNotifySet (core.o)
// ============================================================================

#include "game/core/core_systems.h"

extern void* PoolAllocator_Allocate(void* allocator, unsigned int s,
                                    bool forceHeapAlloc);
extern void PoolAllocator_Release(void* allocator, void* ptr);
extern PoolAllocator* EntityNotify_sAllocator;
extern void* EntityNotifySet_sAllocator;
extern void WaitTilOutput_AssignData(void* self, void* data);
extern "C" void WaitTilOutput_Dtor(void* self, int flags);

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
    n->m_dlist_node.mNext =
        reinterpret_cast<reserved_dlist<EntityNotify>::dlist_node*>(
            &mStrings.m_end);
    n->m_dlist_node.mPrev = mStrings.m_tail;
    if (mStrings.m_tail)
        mStrings.m_tail->mNext = &n->m_dlist_node;
    mStrings.m_tail = &n->m_dlist_node;
    if (!mStrings.m_head)
        mStrings.m_head = &n->m_dlist_node;
    ++mStrings.m_size;
}

// ea: 0x005EF500 (scr.o)
void EntityNotifySet::AddEndOn(EndOnScriptNode* node)
{
    reserved_dlist<EndOnScriptNode>::dlist_node* tail = mEndOnList.m_tail;
    reserved_dlist<EndOnScriptNode>::dlist_node* nodeData =
        reinterpret_cast<reserved_dlist<EndOnScriptNode>::dlist_node*>(node);
    nodeData->mNext = mEndOnList.m_end;
    nodeData->mPrev = tail;
    tail->mNext = nodeData;
    mEndOnList.m_tail = nodeData;
    ++mEndOnList.m_size;
}

// ea: 0x005EF700 (scr.o)
void EntityNotifySet::RmvEndOn(EndOnScriptNode* node)
{
    mEndOnList.erase(node);
}

// ea: 0x004C6450
EntityNotify* EntityNotifySet::GetNotify(const HashString& chk) const
{
    EntityNotify* n = (EntityNotify*)mStrings.m_head;
    EntityNotify* next = n != nullptr
                             ? (EntityNotify*)n->m_dlist_node.mNext
                             : nullptr;
    if (n == (EntityNotify*)&mStrings.m_end || next == nullptr)
        return nullptr;
    while (n->mStr != chk.mHash)
    {
        n = next;
        next = (EntityNotify*)next->m_dlist_node.mNext;
        if (next == nullptr)
            return nullptr;
    }
    return n;
}

// ea: 0x005E95C0
bool EntityNotifySet::CheckForNotify(const HashString& chk) const
{
    return GetNotify(chk) != nullptr;
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
    EntityNotify* n = (EntityNotify*)mStrings.m_head;
    EntityNotify* next = n != nullptr
                             ? (EntityNotify*)n->m_dlist_node.mNext
                             : nullptr;
    if (n != (EntityNotify*)&mStrings.m_end && next != nullptr)
    {
        do
        {
            EntityNotify* current = n;
            n = next;
            next = (EntityNotify*)next->m_dlist_node.mNext;
            mStrings.erase(current);
            if (current != nullptr)
            {
                WaitTilOutput* mParam = current->mParam;
                if (mParam != nullptr)
                    WaitTilOutput_Dtor(mParam, 1);
                current->mParam = nullptr;
                PoolAllocator_Release(EntityNotify_sAllocator, current);
            }
        }
        while (next != nullptr);
    }
    return mStrings.m_head ==
               reinterpret_cast<reserved_dlist<EntityNotify>::dlist_node*>(
                   &mStrings.m_end)
           && mEndOnList.m_head ==
                  reinterpret_cast<reserved_dlist<EndOnScriptNode>::dlist_node*>(
                      &mEndOnList.m_end);
}

// ea: 0x004C6590
void EntityNotifySet::KillEndOnThreads()
{
    mEndOnList.m_head = nullptr;
    mEndOnList.m_tail = nullptr;
}

extern "C" void* EntityNotifySet_GetNotify_Impl(void* self,
                                                  unsigned int hash)
{
    return static_cast<EntityNotifySet*>(self)->GetNotify(HashString((int)hash));
}
