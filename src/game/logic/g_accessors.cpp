// ============================================================================
// g_accessors.cpp - g.o singleton/accessor/math-wrapper cluster
// (Inst accessors, Task memory ops, math shims, trace_t helpers).
// ============================================================================

#include "core/math_types.h"
#include "core/mem_heap.h"
#include "core/PoolAllocator.h"
#include "core/tlFixedString.h"
#include "game/logic/g_local.h"

#include <intrin.h>
#include <math.h>
#include <stdint.h>

// ============================================================================
// trace_t helpers (g.o 0x4A4FD0 / 0x4A4FF0)
// ============================================================================
void trace_t::check_for_decal(float radius)
{
    check_decal = true;
    decal_radius = radius;
}
bool trace_t::decal_ok()
{
    return check_decal;
}

// ============================================================================
// Singleton Inst accessors (g.o)
// ============================================================================
XModelManager* XModelManager::Inst()
{
    return XModelManager::sInst;
}
CGBankManager* CGBankManager::Inst()
{
    return (CGBankManager*)CGBankManager::sInst;
}
PhysDataBankManager* PhysDataBankManager::Inst()
{
    return PhysDataBankManager::sInst;
}

class AnimBankManager {
public:
    static AnimBankManager* sInst;
    static AnimBankManager* Inst();
};
AnimBankManager* AnimBankManager::Inst()
{
    return AnimBankManager::sInst;
}

const char* XModel::GetName() const
{
    return name.mStr;
}

// XAnimUpdateTask / AnimationUpdateTask GetHandler (g.o 0x4A5330/0x4A5340)
struct TaskHandler {
    uint8_t _pad[4];
};
class XAnimUpdateTask {
public:
    static TaskHandler sHandler;  // ?sHandler@XAnimUpdateTask@@0VTaskHandler@@A
    static TaskHandler* GetHandler();
};
class AnimationUpdateTask {
public:
    static TaskHandler sHandler;  // ?sHandler@AnimationUpdateTask@@0VTaskHandler@@A
    static TaskHandler* GetHandler();
};
TaskHandler XAnimUpdateTask::sHandler;
TaskHandler AnimationUpdateTask::sHandler;
TaskHandler* XAnimUpdateTask::GetHandler()
{
    return &XAnimUpdateTask::sHandler;
}
TaskHandler* AnimationUpdateTask::GetHandler()
{
    return &AnimationUpdateTask::sHandler;
}

// ============================================================================
// Task memory ops / Update (g.o 0x4A5250-0x4A52E0)
// ============================================================================
void* Task::operator new(size_t size, bool forceHeapAlloc, const char* /*file*/,
                         int /*line*/)
{
    return Task::sAllocator->Allocate((unsigned int)size, forceHeapAlloc);
}
void Task::operator delete(void* ptr, bool /*forceHeapAlloc*/,
                           const char* /*file*/, int /*line*/)
{
    Task::sAllocator->Release(ptr);
}
void* Task::operator new(size_t size)
{
    return Task::sAllocator->Allocate((unsigned int)size, false);
}
void Task::operator delete(void* ptr)
{
    Task::sAllocator->Release(ptr);
}
void Task::Update(Entity* /*e*/, float /*deltaT*/)
{
}

// TaskFunctor dtor (g.o 0x4A5320)
TaskFunctor::~TaskFunctor()
{
}

// Force emission of Task vector deleting destructor (??_E; game2.o stub)
void force_emit_task_vec_dtor(Task* p)
{
    delete[] p;
}

float cos(float x) { return (float)cos((double)x); }
float fabs(float x) { return (float)fabs((double)x); }
float pow(float x, float y) { return (float)pow((double)x, (double)y); }
float ceil(float x) { return (float)ceil((double)x); }

namespace math {
float Sqrt(float a) { return (float)sqrt((double)a); }
float RSqrt(float a) { return 1.0f / (float)sqrt((double)a); }
}

// ============================================================================
// Misc accessors (g.o)
// ============================================================================
unsigned __int64 tlGetTick()
{
    return __rdtsc();
}
unsigned int* tlFixedString::value()
{
    return (unsigned int*)this;
}

float* cdl_to_native(const math::Position3& v)
{
    return (float*)&v;
}
float* cdl_to_native(const math::Dir3& v)
{
    return (float*)&v;
}

// Collision descriptor ctors (g.o 0x4A54B0 / 0x4A5510)
class SimpleCollisionDesc {
public:
    math::Position3 coord;   // +0x00
    math::Position3 normal;  // +0x10
    SimpleCollisionDesc(const math::Position3& c, const math::Position3& n);
};
class CollisionDesc {
public:
    SimpleCollisionDesc simple;  // +0x00
    ECollisionMaterial material; // +0x20
    CollisionDesc(const math::Position3& c, const math::Position3& n,
                  ECollisionMaterial m);
};
SimpleCollisionDesc::SimpleCollisionDesc(const math::Position3& c,
                                         const math::Position3& n)
{
    coord.v = c.v;
    normal.v = n.v;
}
CollisionDesc::CollisionDesc(const math::Position3& c,
                             const math::Position3& n,
                             ECollisionMaterial m)
    : simple(c, n)
{
    material = m;
}
