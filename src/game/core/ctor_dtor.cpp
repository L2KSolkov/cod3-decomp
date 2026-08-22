// ============================================================================
// ctor_dtor.cpp - class ctors/dtors + inline object init (core.o ctor_dtor)
// ============================================================================

#include "game/core/core_systems.h"
#include "game/core/core_globals.h"
#include "core/PoolAllocator.h"
#include "aeps/apsEffect.h"

#include <string.h>
#include <new>

// Minimal view of SoundDevice (full class in game/sv/sv_stubs.h).
class SoundDevice { public: static SoundDevice* sInst; };  // ?sInst@SoundDevice@@2PAV1@A


// Minimal view of controller (full class in game/platform_xbox/XboxLiveMenus.h).
class controller { public:
    int locked_port;
    static controller* inst();  // ?inst@controller@@SAPAV1@XZ (controller_xbox.o)
};


namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

extern const char* const defaultFileName;

#define ASSERT_IDX(idx, cap, line)                                         \
    do {                                                                   \
        if ((idx) < 0 || (idx) >= (cap)) {                                 \
            AeAssert::gCurrentAuthor = AeAssert::COD3;                     \
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";             \
            AeAssert::gCurrentLine = (line);                               \
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";        \
            if (!AeAssert::IsIgnored()                                     \
                && AeAssert::Assert("out of bounds"))                      \
                __debugbreak();                                            \
        }                                                                  \
    } while (0)

extern "C" unsigned int AeHash(const char* str);
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file,
                                 int line);
struct mem_heap;
extern void mem_heap_create(mem_heap* heap, void* start, void* end,
                            mem_heap* reserve);
extern PoolAllocator* ActiveEffectSet_sAllocator;  // 0x00F00E84
extern int dword_F6A290[4 * 0x322];
extern void CameraShake_StopCameraShake(void* self, void* pShake);
struct CameraShake;
extern CameraShake* g_cameraShake;
extern void SoundDevice_ReleaseSound(void* sInst, void* s);
struct SoundHandleDbLocal {
    struct El {
        void* mObject;  // +0x00
        unsigned int mKey;  // +0x04
    };
    El mElements[512];
};
SoundHandleDbLocal SoundHandleDb_sInst;  // ?SoundHandleDb_sInst (core.o)
extern void controller_stop_all_rumble(void* self);
struct tlSystemCallbacks;
extern void tlSetSystemCallbacks(const tlSystemCallbacks* callbacks);
extern void* AssetBankSet_ctor(void* self);
extern void AssetBankSet_dtor(void* self);
extern void* InplaceAssetBankSet_ConfigStringBank_ctor(void* self);
extern void* InplaceAssetBankSet_StringTableBank_ctor(void* self);
extern void RemoveLight(void* light);

ServerTime::ServerTime()
    : mNumTicksElapsed(0), mTickMSec(1), mTickDelta(0.001f),
      mTickDeltaInv(1000.0f), mElapsedTime(0.0f)
{
}

void ServerTime::Update(int tickMSec)
{
    ++mNumTicksElapsed;
    int clampedTickMSec = tickMSec;
    if (clampedTickMSec <= 1)
        clampedTickMSec = 1;
    float tickDelta = tickMSec / 1000.0f;
    mTickDeltaInv = 1000.0f / tickMSec;
    mTickMSec = clampedTickMSec;
    mTickDelta = tickDelta;
    mElapsedTime += tickDelta;
}

namespace EffectEventSysStatics {
extern EffectEventSys* sInst;
}

// Sentinel-list layout used by the notify/rumble dlist containers
struct RealDList {
    void* m_head;  // points at first node's m_next (or &m_end)
    void* m_end;   // null sentinel
    void* m_tail;  // points at last node's m_next (or &m_head)
    int   m_size;
};
struct DNode {
    DNode* m_prev;
    DNode* m_next;
};

static RealDList sEntityNotifySet;

class EntityHandleDb {
public:
    struct DbElement {
        Entity* mObject;  // +0x00
        int     mKey;     // +0x04
    };
    unsigned char _pad[0xA8];
    DbElement     mElements[0x540];
    static EntityHandleDb sInst;
};

// ============================================================================
// EntityNotify / EntityNotifySet
// ============================================================================

// core.o data (0xF00E28 / 0xF00E2C)
PoolAllocator* EntityNotify::sAllocator = nullptr;
PoolAllocator* EntityNotifySet::sAllocator = nullptr;

// ea: 0x004BDAA0
EntityNotify::EntityNotify(unsigned int hashStr,
                           DbLinkedHandle<EntityHandleDb, Entity> ent,
                           WaitTilOutput* param)
{
    mStr = hashStr;
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mOwner = ent;
    mParam = param;
}

// ea: 0x004B5650
EntityNotify::~EntityNotify()
{
    if (mParam != nullptr)
        delete mParam;
    mParam = nullptr;
}

// EntityNotify memory ops / dlist accessor (g.o 0x4A5B70-0x4A5C60)
void* EntityNotify::get_dlist_node()
{
    return this;
}
// ea: 0x004B3F80
int EntityNotify::get_dlist_node_offset()
{
    return 0;
}
void EntityNotify::SetAllocator(PoolAllocator* allocator)
{
    EntityNotify::sAllocator = allocator;
}
unsigned int EntityNotify::GetStr() const
{
    return mStr;
}
WaitTilOutput* EntityNotify::GetParam() const
{
    return mParam;
}
void* EntityNotify::operator new(size_t size, bool forceHeapAlloc,
                                 const char* /*file*/, int /*line*/)
{
    return EntityNotify::sAllocator->Allocate((unsigned int)size,
                                               forceHeapAlloc);
}
void EntityNotify::operator delete(void* ptr)
{
    EntityNotify::sAllocator->Release(ptr);
}
void EntityNotify::operator delete(void* ptr, bool /*forceHeapAlloc*/,
                                   const char* /*file*/, int /*line*/)
{
    EntityNotify::sAllocator->Release(ptr);
}

// ea: 0x005E9560
DbLinkedHandle<EntityHandleDb, Entity> EntityNotify::GetOwner() const
{
    DbLinkedHandle<EntityHandleDb, Entity> result;
    result.mHandle.mVal = mOwner.mHandle.mVal;
    return result;
}

// ea: 0x005E9580
void* EntityNotifySet::operator new(size_t size, bool forceHeapAlloc)
{
    return EntityNotifySet::sAllocator->Allocate((unsigned int)size,
                                                  forceHeapAlloc);
}

// ea: 0x005E95A0
void EntityNotifySet::operator delete(void* ptr)
{
    EntityNotifySet::sAllocator->Release(ptr);
}
void* EntityNotifySet::get_dlist_node()
{
    return this;
}
int EntityNotifySet::get_dlist_node_offset()
{
    return 0;
}
void EntityNotifySet::SetAllocator(PoolAllocator* allocator)
{
    EntityNotifySet::sAllocator = allocator;
}

// WaitTilOutput memory ops / dtor (g.o 0x4A5810-0x4A5980)
PoolAllocator* WaitTilOutput::sAllocator;
// ea: 0x004B3EB0
void WaitTilOutput::SetAllocator(PoolAllocator* allocator)
{
    WaitTilOutput::sAllocator = allocator;
}
// WaitTilOutput ctor (g.o 0x4B1680)
WaitTilOutput::WaitTilOutput()
{
    dListNodeFiller1 = nullptr;
    dListNodeFiller2 = nullptr;
}
int WaitTilOutput::GetSize()
{
    return 0;
}
void* WaitTilOutput::operator new(size_t size, bool forceHeapAlloc,
                                  const char* /*file*/, int /*line*/)
{
    return WaitTilOutput::sAllocator->Allocate((unsigned int)size,
                                               forceHeapAlloc);
}
void* WaitTilOutput::operator new(size_t size)
{
    return WaitTilOutput::sAllocator->Allocate((unsigned int)size, false);
}
void WaitTilOutput::operator delete(void* ptr, bool /*forceHeapAlloc*/,
                                    const char* /*file*/, int /*line*/)
{
    WaitTilOutput::sAllocator->Release(ptr);
}
void WaitTilOutput::operator delete(void* ptr)
{
    WaitTilOutput::sAllocator->Release(ptr);
}
WaitTilOutput::~WaitTilOutput()
{
}

// Force emission of WaitTilOutput scalar deleting destructor (??_G)
void force_emit_wait_delete(WaitTilOutput* p)
{
    delete p;
}

// g.o explicit instantiations (0x4AE510 / 0x4AE530)
template reserved_dlist<WaitTilOutput>::reserved_dlist();
template void reserved_dlist<WaitTilOutput>::validate() const;
template class reserved_dlist<EntityNotify>;

// ============================================================================
// WaitTilOutputInst1/2 - WaitTilOutput parameter carriers (g.o 0x4B1CD0+)
// ============================================================================
template <typename T>
class WaitTilOutputInst1 : public WaitTilOutput {
public:
    T data;  // +0x0C

    WaitTilOutputInst1(const T& d) : WaitTilOutput(), data(d)
    {
        dListNodeFiller1 = nullptr;
        dListNodeFiller2 = nullptr;
    }
    virtual int GetSize() { return 1; }           // ?GetSize@...@@UAEHXZ
    virtual void AssignData(WaitTilOutput* sv)    // ?AssignData@...@@UAEXPAVWaitTilOutput@@@Z
    {
        if (sv->GetSize() > GetSize())
        {
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        *(T*)((char*)sv + 0x0C) = data;
    }
    virtual ~WaitTilOutputInst1();                // ??1...@@UAE@XZ
};

template <typename T>
WaitTilOutputInst1<T>::~WaitTilOutputInst1()
{
}

template <typename T1, typename T2>
class WaitTilOutputInst2 : public WaitTilOutput {
public:
    T1 data1;  // +0x0C
    T2 data2;  // +0x10

    WaitTilOutputInst2(const T1& d1, const T2& d2)
        : WaitTilOutput(), data1(d1), data2(d2)
    {
        dListNodeFiller1 = nullptr;
        dListNodeFiller2 = nullptr;
    }
    virtual int GetSize() { return 2; }           // ?GetSize@...@@UAEHXZ
    virtual void AssignData(WaitTilOutput* sv)    // ?AssignData@...@@UAEXPAVWaitTilOutput@@@Z
    {
        if (sv->GetSize() > GetSize())
        {
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (sv->GetSize() == 1)
        {
            *(T1*)((char*)sv + 0x0C) = data1;
        }
        else if (sv->GetSize() == 2)
        {
            *(T1*)((char*)sv + 0x0C) = data1;
            *(T2*)((char*)sv + 0x10) = data2;
        }
    }
    virtual ~WaitTilOutputInst2();                // ??1...@@UAE@XZ
};

template <typename T1, typename T2>
WaitTilOutputInst2<T1, T2>::~WaitTilOutputInst2()
{
}

template <>
void WaitTilOutputInst1<int>::AssignData(WaitTilOutput* scriptVariable)
{
    int size = scriptVariable->GetSize();
    if (size > GetSize())
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\script\\include\\WaitTilParms.h";
        AeAssert::gCurrentLine = 91;
        AeAssert::gCurrentExpr = "scriptVariable->GetSize() <= GetSize()";
        if (AeAssert::IsIgnored())
        {
            *(int*)((unsigned char*)scriptVariable + 0x0C) = data;
        }
        else
        {
            if (AeAssert::Assert(defaultFileName))
            {
                __debugbreak();
                *(int*)((unsigned char*)scriptVariable + 0x0C) = data;
                return;
            }
            *(int*)((unsigned char*)scriptVariable + 0x0C) = data;
        }
        return;
    }
    *(int*)((unsigned char*)scriptVariable + 0x0C) = data;
}

template <>
void WaitTilOutputInst2<float, unsigned int>::AssignData(
    WaitTilOutput* scriptVariable)
{
    int size = scriptVariable->GetSize();
    if (size > GetSize())
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\script\\include\\WaitTilParms.h";
        AeAssert::gCurrentLine = 118;
        AeAssert::gCurrentExpr = "scriptVariable->GetSize() <= GetSize()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    unsigned int data1Bits;
    memcpy(&data1Bits, &data1, sizeof(data1Bits));
    if (size == 1)
    {
        *(unsigned int*)((unsigned char*)scriptVariable + 0x0C) = data1Bits;
    }
    else if (size == 2)
    {
        *(unsigned int*)((unsigned char*)scriptVariable + 0x0C) = data1Bits;
        *(unsigned int*)((unsigned char*)scriptVariable + 0x10) = data2;
    }
}

template class WaitTilOutputInst1<Broc::string>;
template class WaitTilOutputInst1<unsigned int>;
template class WaitTilOutputInst1<Broc::entity>;
template class WaitTilOutputInst2<Broc::string, Broc::string>;
template class WaitTilOutputInst2<int, Broc::entity>;
template class WaitTilOutputInst1<int>;
template class WaitTilOutputInst2<float, unsigned int>;

// Shared bridge for the mp_util_wad waittill<entity> wrapper.  The carrier
// type and its virtual layout are the IDA-defined WaitTilOutputInst1 entity.
static_assert(sizeof(WaitTilOutputInst1<Broc::entity>) == 0x10,
              "WaitTilOutputInst1<entity> size mismatch");
WaitTilOutput* WaitTilOutputInst1Entity_Construct(void* storage,
                                                  const Broc::entity& value)
{
    return ::new (storage) WaitTilOutputInst1<Broc::entity>(value);
}

void WaitTilOutputInst1Entity_CopyData(const WaitTilOutput* output,
                                       Broc::entity* value)
{
    const WaitTilOutputInst1<Broc::entity>* carrier =
        static_cast<const WaitTilOutputInst1<Broc::entity>*>(output);
    *value = carrier->data;
}

void WaitTilOutputInst1Entity_Destroy(WaitTilOutput* output)
{
    static_cast<WaitTilOutputInst1<Broc::entity>*>(output)->
        ~WaitTilOutputInst1<Broc::entity>();
}

void force_waitinst1_delete(WaitTilOutputInst1<Broc::string>* p) { delete p; }
void force_waitinst1u_delete(WaitTilOutputInst1<unsigned int>* p) { delete p; }
void force_waitinst1e_delete(WaitTilOutputInst1<Broc::entity>* p) { delete p; }
void force_waitinst2_delete(WaitTilOutputInst2<Broc::string, Broc::string>* p) { delete p; }
void force_waitinst2e_delete(WaitTilOutputInst2<int, Broc::entity>* p) { delete p; }

// ============================================================================
// Entity::Notify<...> (g.o 0x4B3560-0x4B3B40)
// ============================================================================
namespace {
// AeThreadManager opaque view (mPendingNotifys at +0x20)
struct NotifyDListNode {
    NotifyDListNode* m_next;  // +0x00
    NotifyDListNode* m_prev;  // +0x04
};
struct PendingList {
    int m_size;                    // +0x00
    NotifyDListNode* m_head;
    NotifyDListNode* m_end;
    NotifyDListNode* m_tail;
};
class AeThreadManagerLocal {
public:
    uint8_t _pad[0x20];
    PendingList mPendingNotifys;   // +0x20
    static AeThreadManagerLocal sInst;  // ?sInst@AeThreadManager@@0V1@A
};
AeThreadManagerLocal AeThreadManagerLocal::sInst;
}

// ScriptEventParams (completed from game_types.h forward decl)
class ScriptEventParams {
public:
    int ent1;
    int ent2;
    float f1, f2, f3;
    struct { float x, y, z; } v1;
};

static void EntityNotify_Push(EntityNotify* notify)
{
    PendingList* list = &AeThreadManagerLocal::sInst.mPendingNotifys;
    notify->m_dlist_node.mNext = (reserved_dlist<EntityNotify>::dlist_node*)list->m_end;
    NotifyDListNode* tail = list->m_tail;
    notify->m_dlist_node.mPrev = (reserved_dlist<EntityNotify>::dlist_node*)tail;
    tail->m_next = (NotifyDListNode*)&notify->m_dlist_node;
    list->m_tail = (NotifyDListNode*)&notify->m_dlist_node;
    ++list->m_size;
}

// ea: 0x004B3760
void Entity::Notify(HashString h, const unsigned int& e)
{
    EntityNotify* notify = (EntityNotify*)EntityNotify::sAllocator->Allocate(
        0x14, false);
    WaitTilOutput* param = nullptr;
    if (notify != nullptr)
    {
        WaitTilOutputInst1<unsigned int>* output =
            (WaitTilOutputInst1<unsigned int>*)WaitTilOutput::sAllocator->Allocate(
                0x10, false);
        if (output != nullptr)
            param = ::new (output) WaitTilOutputInst1<unsigned int>(e);
        notify = ::new (notify) EntityNotify(h.mHash, mHandle, param);
    }
    if (notify != nullptr)
        EntityNotify_Push(notify);

    ScriptEventParams p;
    memset(&p, 0, sizeof(p));
    p.ent1 = e;
    p.ent2 = 0;
    p.f1 = (float)NAN;
    p.f2 = (float)NAN;
    p.f3 = (float)NAN;
    p.v1.x = (float)NAN;
    p.v1.y = (float)NAN;
    p.v1.z = (float)NAN;
    ExecScriptHandler(h, &p);
}

// ea: 0x004B39E0
void Entity::Notify(HashString h, const int& d, const Broc::entity& e,
                    const int& mod, const int& hitloc)
{
    EntityNotify* notify = (EntityNotify*)EntityNotify::sAllocator->Allocate(
        0x14, false);
    WaitTilOutput* param = nullptr;
    if (notify != nullptr)
    {
        WaitTilOutputInst2<int, Broc::entity>* output =
            (WaitTilOutputInst2<int, Broc::entity>*)WaitTilOutput::sAllocator->Allocate(
                0x14, false);
        if (output != nullptr)
            param = ::new (output) WaitTilOutputInst2<int, Broc::entity>(d, e);
        notify = ::new (notify) EntityNotify(h.mHash, mHandle, param);
    }
    if (notify != nullptr)
        EntityNotify_Push(notify);

    ScriptEventParams p;
    memset(&p, 0, sizeof(p));
    p.ent1 = e.GetHandle();
    p.ent2 = 0;
    p.f1 = (float)d;
    p.f2 = (float)mod;
    p.f3 = (float)hitloc;
    p.v1.x = (float)NAN;
    p.v1.y = (float)NAN;
    p.v1.z = (float)NAN;
    ExecScriptHandler(h, &p);
}

template <typename T>
void Entity::Notify(HashString hashStr, const T& d)
{
    EntityNotify* v4 = (EntityNotify*)EntityNotify::sAllocator->Allocate(0x14, false);
    WaitTilOutputInst1<T>* v5 = nullptr;
    if (v4 != nullptr)
    {
        v5 = (WaitTilOutputInst1<T>*)WaitTilOutput::sAllocator->Allocate(sizeof(WaitTilOutputInst1<T>), false);
        if (v5 != nullptr)
            v5 = ::new (v5) WaitTilOutputInst1<T>(d);
        v4 = ::new (v4) EntityNotify(hashStr.mHash, mHandle, v5);
    }
    if (v4 != nullptr)
        EntityNotify_Push(v4);
    ExecScriptHandler(hashStr, nullptr);
}
template void Entity::Notify<Broc::string>(HashString, const Broc::string&);

template <typename T1, typename T2>
void Entity::Notify(HashString hashStr, const T1& d1, const T2& d2)
{
    EntityNotify* v4 = (EntityNotify*)EntityNotify::sAllocator->Allocate(0x14, false);
    WaitTilOutputInst2<T1, T2>* v5 = nullptr;
    if (v4 != nullptr)
    {
        v5 = (WaitTilOutputInst2<T1, T2>*)WaitTilOutput::sAllocator->Allocate(sizeof(WaitTilOutputInst2<T1, T2>), false);
        if (v5 != nullptr)
            v5 = ::new (v5) WaitTilOutputInst2<T1, T2>(d1, d2);
        v4 = ::new (v4) EntityNotify(hashStr.mHash, mHandle, v5);
    }
    if (v4 != nullptr)
        EntityNotify_Push(v4);
    ExecScriptHandler(hashStr, nullptr);
}
template void Entity::Notify<Broc::string, Broc::string>(HashString, const Broc::string&, const Broc::string&);

// Entity::Notify(HashString, const Broc::entity&) (g.o 0x4B38A0)
void Entity::Notify(HashString h, const Broc::entity& e)
{
    EntityNotify* v4 = (EntityNotify*)EntityNotify::sAllocator->Allocate(0x14, false);
    WaitTilOutputInst1<Broc::entity>* v5 = nullptr;
    if (v4 != nullptr)
    {
        v5 = (WaitTilOutputInst1<Broc::entity>*)WaitTilOutput::sAllocator->Allocate(0x10, false);
        if (v5 != nullptr)
            v5 = ::new (v5) WaitTilOutputInst1<Broc::entity>(e);
        v4 = ::new (v4) EntityNotify(h.mHash, mHandle, v5);
    }
    if (v4 != nullptr)
        EntityNotify_Push(v4);
    ScriptEventParams p;
    memset(&p, 0, sizeof(p));
    p.ent1 = e.GetHandle();
    p.f1 = (float)NAN;
    p.f2 = (float)NAN;
    p.f3 = (float)NAN;
    p.v1.x = (float)NAN;
    p.v1.y = (float)NAN;
    p.v1.z = (float)NAN;
    ExecScriptHandler(h, &p);
}

// Entity::Notify(HashString, const int&, const Broc::entity&, const int&, const int&, const float*) (g.o 0x4B3B40)
void Entity::Notify(HashString h, const int& d, const Broc::entity& e,
                    const int& mod, const int& hitloc, const float* hit_normal)
{
    EntityNotify* v7 = (EntityNotify*)EntityNotify::sAllocator->Allocate(0x14, false);
    WaitTilOutputInst2<int, Broc::entity>* v8 = nullptr;
    if (v7 != nullptr)
    {
        v8 = (WaitTilOutputInst2<int, Broc::entity>*)WaitTilOutput::sAllocator->Allocate(0x14, false);
        if (v8 != nullptr)
            v8 = ::new (v8) WaitTilOutputInst2<int, Broc::entity>(d, e);
        v7 = ::new (v7) EntityNotify(h.mHash, mHandle, v8);
    }
    if (v7 != nullptr)
        EntityNotify_Push(v7);
    ScriptEventParams p;
    memset(&p, 0, sizeof(p));
    p.ent1 = e.GetHandle();
    p.f1 = (float)d;
    p.f2 = (float)mod;
    p.f3 = (float)hitloc;
    p.v1.x = (float)NAN;
    p.v1.y = (float)NAN;
    p.v1.z = (float)NAN;
    if (hit_normal != nullptr)
    {
        p.v1.x = hit_normal[0];
        p.v1.y = hit_normal[1];
        p.v1.z = hit_normal[2];
    }
    ExecScriptHandler(h, &p);
}

// ea: 0x004C1D80
EntityNotifySet::EntityNotifySet(Entity* e)
{
    m_dlist_node.mNext = nullptr;
    m_dlist_node.mPrev = nullptr;
    mEnt.mHandle.mVal = e->mHandle.mHandle.mVal;
    RealDList* strings = (RealDList*)&mStrings;
    strings->m_end = nullptr;
    strings->m_head = &strings->m_end;
    strings->m_tail = &strings->m_head;
    strings->m_size = 0;
    RealDList* endOn = (RealDList*)&mEndOnList;
    endOn->m_end = nullptr;
    endOn->m_head = &endOn->m_end;
    endOn->m_tail = &endOn->m_head;
    endOn->m_size = 0;
    DNode* m_head = (DNode*)sEntityNotifySet.m_head;
    DNode* m_next = m_head != nullptr ? m_head->m_next : nullptr;
    if (m_next != nullptr)
    {
        while (m_head != (DNode*)this)
        {
            m_head = m_next;
            m_next = m_next->m_next;
            if (m_next == nullptr)
                goto LABEL_9;
        }
        if (m_dlist_node.mNext != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityNotifySet.cpp";
            AeAssert::gCurrentLine = 38;
            AeAssert::gCurrentExpr =
                "sEntityNotifySet.find( this ) == sEntityNotifySet.end()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("hmm?"))
                __debugbreak();
        }
    }
LABEL_9:
    m_dlist_node.mNext = (reserved_dlist<EntityNotifySet>::dlist_node*)&sEntityNotifySet.m_end;
    m_dlist_node.mPrev = (reserved_dlist<EntityNotifySet>::dlist_node*)sEntityNotifySet.m_tail;
    *(DNode**)sEntityNotifySet.m_tail = (DNode*)&m_dlist_node;
    ++sEntityNotifySet.m_size;
    sEntityNotifySet.m_tail = &m_dlist_node;
}

// ea: 0x004CEAE0
EntityNotifySet::~EntityNotifySet()
{
    // unlink from sEntityNotifySet
    DNode* node = (DNode*)&m_dlist_node;
    if (node->m_prev != nullptr)
        node->m_prev->m_next = node->m_next;
    else
        sEntityNotifySet.m_head = node->m_next;
    if (node->m_next != nullptr)
        node->m_next->m_prev = node->m_prev;
    else
        sEntityNotifySet.m_tail = node->m_prev;
    if (sEntityNotifySet.m_size > 0)
        --sEntityNotifySet.m_size;
    // destroy string nodes
    RealDList* strings = (RealDList*)&mStrings;
    DNode* cur = (DNode*)strings->m_head;
    while (cur != nullptr && cur != (DNode*)&strings->m_end)
    {
        DNode* next = cur->m_next;
        delete (EntityNotify*)cur;
        cur = next;
    }
    strings->m_head = &strings->m_end;
    strings->m_tail = &strings->m_head;
    strings->m_size = 0;
    unsigned int mVal = mEnt.mHandle.mVal;
    unsigned int v7 = mVal & 0xFFF;
    if (v7 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v7].mKey
        && EntityHandleDb::sInst.mElements[v7].mObject != nullptr)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v7].mObject;
        mObject->mNotifySet = nullptr;
    }
}

// ============================================================================
// DialogueManager / ConfigStringManager / STBManager
// ============================================================================

// ea: 0x004C0B40
DialogueManager::DialogueManager()
{
    AssetBankSet_ctor(this);
    for (int i = 0; i < 99; ++i)
        mBanks[i] = nullptr;
}

// ea: 0x004BCD80
DialogueManager::~DialogueManager()
{
    AssetBankSet_dtor(this);
}

// ea: 0x004C5C60
ConfigStringManager::ConfigStringManager()
{
    InplaceAssetBankSet_ConfigStringBank_ctor(this);
}

// ea: 0x004E8A20
void ConfigStringManager::CreateInst()
{
    if (ConfigStringManager::sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ConfigStringManager.h";
        AeAssert::gCurrentLine = 19;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    void* memory = mem_heap_malloc_ctx(
        0x190u, 4, "core", "c:\\cod\\code\\game\\ConfigStringManager.h", 19);
    if (memory != nullptr)
    {
        ConfigStringManager::sInst = new (memory) ConfigStringManager();
    }
    else
    {
        ConfigStringManager::sInst = nullptr;
    }
}

// ea: 0x004C1440
ConfigStringManager::~ConfigStringManager()
{
    AssetBankSet_dtor(this);
}

// ea: 0x004DD290
void ConfigStringManager::DeleteInst()
{
    if (sInst == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ConfigStringManager.h";
        AeAssert::gCurrentLine = 19;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }

    if (sInst != nullptr)
        delete sInst;
    sInst = nullptr;
}

// ea: 0x004C5CE0
STBManager::STBManager()
{
    InplaceAssetBankSet_StringTableBank_ctor(this);
}

// ea: 0x004C14D0
STBManager::~STBManager()
{
    AssetBankSet_dtor(this);
}

// ============================================================================
// CtrlIcon / HashString
// ============================================================================

// ea: 0x004BD6E0
CtrlIcon::CtrlIcon()
{
}

// ea: 0x004BD6F0
CtrlIcon::~CtrlIcon()
{
}

// ea: 0x004C1450
HashString::HashString(Broc::string& str)
{
    if (str.mBlock != nullptr)
        mHash = AeHash((const char*)(str.mBlock + 1));
    else
        mHash = AeHash("");
}

// ea: 0x004BD6B0
unsigned int HashString::CalcHash(const char* str)
{
    return AeHash(str);
}

// ============================================================================
// AnimHeap
// ============================================================================

// ea: 0x004C1380
AnimHeap::AnimHeap()
{
    mBlock = mem_heap_malloc(0x100000);
    mem_heap_create((mem_heap*)mHeap, mBlock, (char*)mBlock + 0x100000,
                    nullptr);
    AnimHeapStatics::sInst = this;
}

// ea: 0x004BD610
AnimHeap::~AnimHeap()
{
    mem_heap_free(mBlock);
}

// ============================================================================
// RumbleManager
// ============================================================================

// ea: 0x004BCF60
RumbleManager::InstanceHolder::InstanceHolder()
{
    sInst[0] = nullptr;
}

// ea: 0x004C56F0
RumbleManager::RumbleManager(int client)
{
    mNextHandle.mVal = 1;
    for (int i = 0; i < 2; ++i)
    {
        RealDList* list = (RealDList*)&mRumbleLists[i];
        list->m_end = nullptr;
        list->m_head = &list->m_end;
        list->m_tail = &list->m_head;
        list->m_size = 0;
    }
    mClient = client;
    mLastTimeNotRumbling = 0;
    mDontRumbleAgainUntil = 0;
}

// ea: 0x004CF310
RumbleManager::~RumbleManager()
{
    controller_stop_all_rumble(controller::inst());
    for (int i = 0; i < 2; ++i)
    {
        RealDList* list = (RealDList*)&mRumbleLists[i];
        DNode* cur = (DNode*)list->m_head;
        while (cur != nullptr && cur != (DNode*)&list->m_end)
        {
            DNode* next = cur->m_next;
            delete (RumbleEffectInstance*)cur;
            cur = next;
        }
        list->m_head = &list->m_end;
        list->m_tail = &list->m_head;
        list->m_size = 0;
    }
}

// ============================================================================
// AbstractEffect family
// ============================================================================

// ea: 0x004C1240
AbstractEffect::AbstractEffect(TPakId pak_id,
                               DbLinkedHandle<EntityHandleDb, Entity> ent,
                               int flags, float delay_trigger)
{
    mEffectName = Broc::string((Broc::string::Block*)nullptr);
    mEntity.mHandle.mVal = 0;
    mCodeFlags.mVal = 0;
    mPoPtr = nullptr;
    mDelayTrigger = delay_trigger;
    mCountSinceStarted = 0;
    mEffectNameHashStr = 0;
    mCodeFlags.mVal = 0;
    mPakId = pak_id;
    mEntity = ent;
    mFlags = flags;
    mDelayCount = 0.0f;
}

// ea: 0x004BD160
AbstractEffect::~AbstractEffect()
{
    mEffectNameHashStr = 0;
    mEffectName.clear();
}

// ea: 0x004CC3F0
AbstractEffectSound::~AbstractEffectSound()
{
    unsigned int mVal = mSound.mHandle.mVal;
    unsigned int v3 = mVal & 0xFFF;
    if (v3 < 0x200 && mVal >> 12 == SoundHandleDb_sInst.mElements[v3].mKey
        && SoundHandleDb_sInst.mElements[v3].mObject != nullptr)
    {
        SoundDevice_ReleaseSound(SoundDevice::sInst,
                                 SoundHandleDb_sInst.mElements[v3].mObject);
    }
    mEffectNameHashStr = 0;
    AbstractEffect::~AbstractEffect();
}

// ea: 0x004C12C0
AbstractEffectParticle::~AbstractEffectParticle()
{
    ParticleEffect* mParticle = this->mParticle;
    if (mParticle != nullptr)
    {
        apsEffect* mEffect = mParticle->mEffect;
        if (mEffect != nullptr)
        {
            if (EffectEventSysStatics::sInst->mStoppingAll
                || (mFlags & 8) == 0)
                mParticle->mFlags.mVal |= 0x20u;
            else
                mEffect->StopEmitting();
        }
        else
        {
            this->mParticle = nullptr;
        }
    }
    mEffectNameHashStr = 0;
    AbstractEffect::~AbstractEffect();
}

// ea: 0x004BD2A0
AbstractEffectLight::~AbstractEffectLight()
{
    mEffectNameHashStr = 0;
    RemoveLight(mProjLight);
    RemoveLight(mVertLight);
    AbstractEffect::~AbstractEffect();
}

// ea: 0x004CF890
AbstractEffectShakeAndRumble::AbstractEffectShakeAndRumble(
    TPakId pak_id, DbLinkedHandle<EntityHandleDb, Entity> ent, float delay_trigger,
    int flags, float time, float freq, float movement, float nextDelay,
    float rumble, float blur, float minDist, float maxDist,
    float steadyDuration, float rampUpTime, float rampDownTime,
    bool useHighFreqVib, bool rumbleEnabled, EUserBoneId bone)
{
    mEffectName = Broc::string((Broc::string::Block*)nullptr);
    mEntity.mHandle.mVal = 0;
    mCodeFlags.mVal = 0;
    mDelayTrigger = delay_trigger;
    mPakId = pak_id;
    mEntity = ent;
    mFlags = flags;
    mPoPtr = nullptr;
    mDelayCount = 0.0f;
    mCountSinceStarted = 0;
    mEffectNameHashStr = 0;
    mCodeFlags.mVal = 0;
    mTime = time;
    mFreq = freq;
    mMovement = movement;
    mNextDelay = nextDelay;
    mRumble = rumble;
    mBlur = blur;
    mMinDist2 = minDist * minDist;
    mSteadyDuration = steadyDuration;
    mRampUpTime = rampUpTime;
    mMaxDist2 = maxDist * maxDist;
    mRampDownTime = rampDownTime;
    mUseHighFreqVibrator = useHighFreqVib;
    mRumbleEnabled = rumbleEnabled;
    mBone = bone;
    mRumbleHandle[0].mVal = 0;
    mType = AeHash("AbstractEffectShakeAndRumble");
    mShake[0] = nullptr;
    FrameAdvance(0.0f);
}

// ea: 0x004CF9D0
AbstractEffectShakeAndRumble::~AbstractEffectShakeAndRumble()
{
    if (dword_F6A290[0] == 2)
    {
        if (mRumbleHandle[0].mVal != 0)
        {
            RumbleManager* v2 = RumbleManager::Inst(0);
            v2->Remove(mRumbleHandle[0]);
            mRumbleHandle[0].mVal = 0;
        }
        if (mShake[0] != nullptr)
        {
            CameraShake_StopCameraShake(g_cameraShake, mShake[0]);
            mShake[0] = nullptr;
        }
    }
    mEffectNameHashStr = 0;
    AbstractEffect::~AbstractEffect();
}

// ============================================================================
// ActiveEffectSet / EffectEventSys
// ============================================================================

// ea: 0x004CEC60
ActiveEffectSet::~ActiveEffectSet()
{
    for (unsigned int i = 0; i < (unsigned int)mEffects.m_size; ++i)
    {
        ASSERT_IDX(i, 6, 154);
        AbstractEffect* effect = mEffects[i];
        if (EffectEventSysStatics::sInst->mStoppingAll
            || (effect->mFlags & 8) == 0)
        {
            delete effect;
        }
        else
        {
            effect->StartFadeOut(2.0f);
            EffectEventSysStatics::sInst->mFadingEffects.push_back(effect);
        }
    }
    if ((mFlags.mVal & 4) != 0)
    {
        ((PoolAllocator*)gCommonPoolAllocator)->Release(mPoPtr);
        mPoPtr = nullptr;
    }
    if (mId.mVal != 0)
        EffectEventSysStatics::sInst->mHandleDb.ReleaseHandle(mId);
}

// ea: 0x004E8CF0
HandleDb::HandleDb()
{
    mFreeIndices.Clear();
    for (int i = 0; i < 512; ++i)
    {
        mElements[i].mObject = nullptr;
        mElements[i].mKey = 1;
    }
    mDebugCallback = nullptr;
    for (int i = 0; i < 512; ++i)
        mFreeIndices.Add(i);
}

// ea: 0x004D0130
EffectEventSys::EffectEventSys()
{
    mEffectSets.m_size = 0;
    mFadingEffects.m_size = 0;
    mCurrentQuery = nullptr;
    mStoppingAll = false;
    for (int i = 0; i < 32; ++i)
    {
        mEffectRefs.m_elements[i].first = 0;
        mEffectRefs.m_elements[i].second = 0;
    }
    mEffectRefs.m_size = 0;
    mDebuggingLevel = 0;
}

// ea: 0x004D01E0
EffectEventSys::~EffectEventSys()
{
    for (unsigned int i = 0; i < (unsigned int)mEffectSets.m_size; ++i)
    {
        ASSERT_IDX(i, 512, 154);
        ActiveEffectSet* v3 = mEffectSets[i];
        if (v3 != nullptr)
        {
            v3->~ActiveEffectSet();
            ActiveEffectSet_sAllocator->Release(v3);
        }
    }
    for (unsigned int j = 0; j < (unsigned int)mFadingEffects.m_size; ++j)
    {
        ASSERT_IDX(j, 128, 154);
        AbstractEffect* v5 = mFadingEffects[j];
        if (v5 != nullptr)
            delete v5;
    }
}

// ============================================================================
// TlSystemCallbacks
// ============================================================================

// ea: 0x004D0A20
TlSystemCallbacks::TlSystemCallbacks()
{
    mTlCallbacks.ReadFile =
        (bool (*)(const char*, tlFileBuf*, unsigned int, unsigned int))ReadFile;
    mTlCallbacks.ReleaseFile =
        (void (*)(tlFileBuf*))ReleaseFile;
    mTlCallbacks.CriticalError =
        (void (*)(const char*))CriticalError;
    mTlCallbacks.Warning = (void (*)(const char*))Warning;
    mTlCallbacks.DebugPrint = (void (*)(const char*))DebugPrint;
    mTlCallbacks.MemAlloc = (void* (*)(unsigned int, unsigned int,
                                       unsigned int))MemAlloc;
    mTlCallbacks.MemFree = (void (*)(void*))MemFree;
    mTlCallbacks.MemRealloc =
        (void* (*)(void*, unsigned int, unsigned int, unsigned int))MemRealloc;
    mTlCallbacks.FinalPrint = nullptr;
    mTlCallbacks.LinkConnected = (bool (*)())LinkConnected;
    tlSetSystemCallbacks((const tlSystemCallbacks*)this);
}
