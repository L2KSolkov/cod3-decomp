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

// Cross-type ctors / assignments (g.o 0x4A5CD0-0x4A5F80)
const math::Dir3& math::Dir3::operator=(const math::Position3& _v)
{
    v = _v.v;
    return *this;
}
math::Dir3::Dir3(const math::Position3& _v)
{
    v = _v.v;
}
math::Dir3::Dir3(const math::Vector4& _v)
{
    v = _v.v;
}
math::Dir3::Dir3(const math::Dir3::Packed& _p)
{
    v = _mm_set_ps(0.0f, _p.z, _p.y, _p.x);
}
const math::Position3& math::Position3::operator=(const math::Position3::Packed& _p)
{
    v = _mm_set_ps(0.0f, _p.z, _p.y, _p.x);
    return *this;
}
math::Position3::Position3(const math::Dir3& _v)
{
    v = _v.v;
}
math::Position3::Position3(const math::Position3::Packed& _p)
{
    v = _mm_set_ps(0.0f, _p.z, _p.y, _p.x);
}
math::Vector4::Vector4(const math::Dir3& _v)
{
    v = _mm_shuffle_ps(_v.v, _mm_shuffle_ps(_mm_setzero_ps(), _v.v, 0xA0), 0x34);
}

// Free-function vector math (g.o 0x4A5FB0-0x4A65E0)
math::Dir3 math::operator-(const math::Dir3& _v)
{
    math::Dir3 r;
    r.v = _mm_xor_ps(_mm_set1_ps(-0.0f), _v.v);
    return r;
}
math::Position3 math::operator-(const math::Position3& _v)
{
    math::Position3 r;
    r.v = _mm_xor_ps(_mm_set1_ps(-0.0f), _v.v);
    return r;
}
math::Vector4 math::operator-(const math::Vector4& _v)
{
    math::Vector4 r;
    r.v = _mm_xor_ps(_mm_set1_ps(-0.0f), _v.v);
    return r;
}
float math::Length(const math::Dir3& _v)
{
    __m128 v1 = _mm_mul_ps(_v.v, _v.v);
    return (float)sqrt((double)(v1.m128_f32[0]
                                + (_mm_shuffle_ps(v1, v1, 0x55).m128_f32[0]
                                   + _mm_shuffle_ps(v1, v1, 0xAA).m128_f32[0])));
}
float math::AbsSquared(const math::Dir3& _v)
{
    __m128 v1 = _mm_mul_ps(_v.v, _v.v);
    return v1.m128_f32[0]
           + (_mm_shuffle_ps(v1, v1, 0x55).m128_f32[0]
              + _mm_shuffle_ps(v1, v1, 0xAA).m128_f32[0]);
}
float math::AbsSquared(const math::Position3& _v)
{
    __m128 v1 = _mm_mul_ps(_v.v, _v.v);
    return v1.m128_f32[0]
           + (_mm_shuffle_ps(v1, v1, 0x55).m128_f32[0]
              + _mm_shuffle_ps(v1, v1, 0xAA).m128_f32[0]);
}
float math::Abs(const math::Dir3& _v)
{
    __m128 v1 = _mm_mul_ps(_v.v, _v.v);
    return (float)sqrt((double)(v1.m128_f32[0]
                                + (_mm_shuffle_ps(v1, v1, 0x55).m128_f32[0]
                                   + _mm_shuffle_ps(v1, v1, 0xAA).m128_f32[0])));
}
float math::Abs(const math::Position3& _v)
{
    __m128 v1 = _mm_mul_ps(_v.v, _v.v);
    return (float)sqrt((double)(v1.m128_f32[0]
                                + (_mm_shuffle_ps(v1, v1, 0x55).m128_f32[0]
                                   + _mm_shuffle_ps(v1, v1, 0xAA).m128_f32[0])));
}
math::Vector4 math::AbsValue(const math::Vector4& _v)
{
    math::Vector4 r;
    r.v = _mm_andnot_ps(_mm_set1_ps(-0.0f), _v.v);
    return r;
}
math::Vector4 math::Ceil(const math::Vector4& _v)
{
    static const __m128 FloorMagic = _mm_set1_ps(8388608.0f);
    math::Vector4 r;
    r.v = _mm_add_ps(_mm_sub_ps(_v.v, FloorMagic), FloorMagic);
    return r;
}
bool math::operator==(const math::Position3& _a, const math::Position3& _b)
{
    return _a.v.m128_f32[0] == _b.v.m128_f32[0]
        && _a.v.m128_f32[1] == _b.v.m128_f32[1]
        && _a.v.m128_f32[2] == _b.v.m128_f32[2];
}
bool math::operator!=(const math::Position3& _a, const math::Position3& _b)
{
    return _a.v.m128_f32[0] != _b.v.m128_f32[0]
        || _a.v.m128_f32[1] != _b.v.m128_f32[1]
        || _a.v.m128_f32[2] != _b.v.m128_f32[2];
}
math::Dir3 math::operator+(const math::Dir3& _a, const math::Position3& _b)
{
    math::Dir3 r;
    r.v = _mm_add_ps(_a.v, _b.v);
    return r;
}
math::Position3 math::operator+(const math::Position3& _a, const math::Dir3& _b)
{
    math::Position3 r;
    r.v = _mm_add_ps(_a.v, _b.v);
    return r;
}

// Broc::vector::Set (g.o 0x4A5DE0)
void Broc::vector::Set(float X, float Y, float Z)
{
    x = X;
    y = Y;
    z = Z;
}
// Broc::entity ctors (g.o 0x4A6250/0x4A6270)
Broc::entity::entity(unsigned int v)
{
    ___u0 = v;
}
Broc::entity::entity(const Broc::entity& rhs)
{
    ___u0 = rhs.___u0;
}

// ScriptEventParams ctor (g.o 0x4A6430)
struct ScriptEventParams {
    int ent1;
    int ent2;
    float f1, f2, f3;
    struct { float x, y, z; } v1;
    ScriptEventParams();
};
ScriptEventParams::ScriptEventParams()
{
    ent1 = 0;
    ent2 = 0;
    f1 = (float)NAN;  // sNaN
    f2 = (float)NAN;
    f3 = (float)NAN;
    v1.x = (float)NAN;
    v1.y = (float)NAN;
    v1.z = (float)NAN;
}

// AeThread / manager accessors (g.o 0x4A6330-0x4A6590)
namespace AeThread {
struct BackupStack {
    struct Block {
        static PoolAllocator* sAllocator;  // ?sAllocator@Block@BackupStack@AeThread@@2PAVPoolAllocator@@A
        static PoolAllocator* GetAllocator();  // ?GetAllocator@Block@BackupStack@AeThread@@SAPAVPoolAllocator@@XZ
    };
};
}
PoolAllocator* AeThread::BackupStack::Block::sAllocator;
PoolAllocator* AeThread::BackupStack::Block::GetAllocator()
{
    return AeThread::BackupStack::Block::sAllocator;
}
AeThreadManager* AeThreadManager::Inst()
{
    return &AeThreadManager::sInst;
}
DestructibleBankManager* DestructibleBankManager::Inst()
{
    return DestructibleBankManager::sInst;
}

// Entity::GetNotifySet (g.o 0x4A6620)
EntityNotifySet* Entity::GetNotifySet()
{
    return mNotifySet;
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
