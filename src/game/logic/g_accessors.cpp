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

// ============================================================================
// XModel::GetXModelParts (g.o 0x4A5170 / 0x4A51D0)
// ============================================================================
XModelParts* XModel::GetXModelParts(int lodIndex)
{
    if (lodIndex >= 0)
        return lod[lodIndex]->xmodelParts;
    int v3 = 0;
    if (lod[0] != nullptr)
        return lod[0]->xmodelParts;
    do
    {
        ++v3;
    } while (lod[v3] == nullptr);
    return lod[v3]->xmodelParts;
}
const XModelParts* XModel::GetXModelParts(int lodIndex) const
{
    if (lodIndex >= 0)
        return lod[lodIndex]->xmodelParts;
    int v3 = 0;
    if (lod[0] != nullptr)
        return lod[0]->xmodelParts;
    do
    {
        ++v3;
    } while (lod[v3] == nullptr);
    return lod[v3]->xmodelParts;
}

// ============================================================================
// scr_vehicle_t CollisionDamage / GetAverageWheelSpeed (g.o)
// ============================================================================
void scr_vehicle_t::CollisionDamage(Entity* ent, const math::Position3& pos,
                                    const math::Position3& dir,
                                    float intensity)
{
    G_Damage(ent, nullptr, nullptr, dir.v.m128_f32, pos.v.m128_f32,
             (int)(s_vehicleInfos[this->infoIdx]->collisionDamage * intensity),
             32, 27, HITLOC_NONE, -1);
}

float scr_vehicle_t::GetAverageWheelSpeed()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    float m_wheel_vel = 0.0f;
    int totalWheels = 0;
    if (mRBVeh != nullptr)
    {
        for (int i = 0; i < 6; ++i)
        {
            rigid_body_constraint_wheel* w = mRBVeh->m_wheels[i];
            if (w != nullptr && (w->m_wheel_flags & 0x10) != 0)
            {
                m_wheel_vel += w->m_wheel_vel;
                ++totalWheels;
            }
        }
        if (totalWheels > 0)
            return m_wheel_vel / (float)totalWheels;
    }
    return 0.0f;
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
// math vector accessors (g.o 0x4A55C0-0x4A5BA0)
// ============================================================================
math::Dir3::Dir3()
{
}
math::Dir3::Dir3(float _x, float _y, float _z)
{
    v.m128_f32[0] = _x;
    v.m128_f32[1] = _y;
    v.m128_f32[2] = _z;
    v.m128_f32[3] = 0.0f;
}
math::Dir3::Dir3(float _x)
{
    v.m128_f32[0] = _x;
    v.m128_f32[1] = _x;
    v.m128_f32[2] = _x;
    v.m128_f32[3] = 0.0f;
}
float math::Dir3::GetX() const { return v.m128_f32[0]; }
float math::Dir3::GetY() const
{
    return _mm_shuffle_ps(v, v, 0x55).m128_f32[0];
}
float math::Dir3::GetZ() const
{
    return _mm_shuffle_ps(v, v, 0xAA).m128_f32[0];
}
void math::Dir3::SetX(float _x)
{
    __m128 t = v;
    t.m128_f32[0] = _x;
    v = t;
}
void math::Dir3::SetY(float _y)
{
    v = _mm_shuffle_ps(_mm_shuffle_ps(_mm_set1_ps(_y), v, 0), v, 0xE2);
}
void math::Dir3::SetZ(float _z)
{
    v = _mm_shuffle_ps(v, _mm_shuffle_ps(_mm_set1_ps(_z), v, 0xF0), 0xC4);
}
float& math::Dir3::operator[](unsigned int i) { return v.m128_f32[i]; }
const float& math::Dir3::operator[](unsigned int i) const { return v.m128_f32[i]; }

math::Position3::Position3(float _x, float _y, float _z)
{
    v.m128_f32[0] = _x;
    v.m128_f32[1] = _y;
    v.m128_f32[2] = _z;
    v.m128_f32[3] = 0.0f;
}
math::Position3::Position3(float _x)
{
    v.m128_f32[0] = _x;
    v.m128_f32[1] = _x;
    v.m128_f32[2] = _x;
    v.m128_f32[3] = 0.0f;
}
float math::Position3::GetX() const { return v.m128_f32[0]; }
float math::Position3::GetY() const
{
    return _mm_shuffle_ps(v, v, 0x55).m128_f32[0];
}
float math::Position3::GetZ() const
{
    return _mm_shuffle_ps(v, v, 0xAA).m128_f32[0];
}
void math::Position3::SetX(float _x)
{
    __m128 t = v;
    t.m128_f32[0] = _x;
    v = t;
}
void math::Position3::SetY(float _y)
{
    v = _mm_shuffle_ps(_mm_shuffle_ps(_mm_set1_ps(_y), v, 0), v, 0xE2);
}
void math::Position3::SetZ(float _z)
{
    v = _mm_shuffle_ps(v, _mm_shuffle_ps(_mm_set1_ps(_z), v, 0xF0), 0xC4);
}
float& math::Position3::operator[](unsigned int i) { return v.m128_f32[i]; }
const float& math::Position3::operator[](unsigned int i) const
{
    return v.m128_f32[i];
}

math::Vector4::Vector4(float _x, float _y, float _z, float _w)
{
    v.m128_f32[0] = _x;
    v.m128_f32[1] = _y;
    v.m128_f32[2] = _z;
    v.m128_f32[3] = _w;
}
math::Vector4::Vector4(float _x)
{
    v = _mm_set1_ps(_x);
}
float math::Vector4::GetX() const { return v.m128_f32[0]; }
float math::Vector4::GetY() const
{
    return _mm_shuffle_ps(v, v, 0x55).m128_f32[0];
}
float math::Vector4::GetZ() const
{
    return _mm_shuffle_ps(v, v, 0xAA).m128_f32[0];
}
float math::Vector4::GetW() const
{
    return _mm_shuffle_ps(v, v, 0xFF).m128_f32[0];
}
void math::Vector4::SetX(float _x)
{
    __m128 t = v;
    t.m128_f32[0] = _x;
    v = t;
}
void math::Vector4::SetY(float _y)
{
    v = _mm_shuffle_ps(_mm_shuffle_ps(_mm_set1_ps(_y), v, 0), v, 0xE2);
}
void math::Vector4::SetZ(float _z)
{
    v = _mm_shuffle_ps(v, _mm_shuffle_ps(_mm_set1_ps(_z), v, 0xF0), 0xC4);
}
void math::Vector4::SetW(float _w)
{
    v = _mm_shuffle_ps(v, _mm_shuffle_ps(_mm_set1_ps(_w), v, 0xE0), 0x39);
}

// EntityState::GetLerpAngles (g.o 0x4A5750)
math::Position3 EntityState::GetLerpAngles() const
{
    return lerpAngles;
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
