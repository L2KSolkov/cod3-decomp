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
#include <float.h>
#include <math.h>
#include <new>
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
math::Position3 math::operator+(const math::Position3& _a, const math::Position3& _b)
{
    math::Position3 r;
    r.v = _mm_add_ps(_a.v, _b.v);
    return r;
}
math::Vector4 math::operator+(const math::Vector4& _a, const math::Vector4& _b)
{
    math::Vector4 r;
    r.v = _mm_add_ps(_a.v, _b.v);
    return r;
}
math::Dir3 math::operator-(const math::Dir3& _a, const math::Position3& _b)
{
    math::Dir3 r;
    r.v = _mm_sub_ps(_a.v, _b.v);
    return r;
}
math::Position3 math::operator-(const math::Position3& _a, const math::Dir3& _b)
{
    math::Position3 r;
    r.v = _mm_sub_ps(_a.v, _b.v);
    return r;
}
math::Position3 math::operator-(const math::Position3& _a, const math::Position3& _b)
{
    math::Position3 r;
    r.v = _mm_sub_ps(_a.v, _b.v);
    return r;
}
math::Dir3 math::operator/(const math::Dir3& _a, float _b)
{
    math::Dir3 r;
    r.v = _mm_div_ps(_a.v, _mm_shuffle_ps(_mm_set_ss(_b), _mm_set_ss(_b), 0));
    return r;
}

// Vector4(const Constant&) (g.o 0x4A5F80)
math::Vector4::Vector4(const math::Vector4::Constant& _c)
{
    v = _mm_loadu_ps(&_c.x);
}

// ============================================================================
// Scalar-multiply free functions (g.o 0x4A6830-0x4A6950)
// ============================================================================
math::Dir3 math::operator*(const math::Dir3& _a, float _b)
{
    math::Dir3 r;
    r.v = _mm_mul_ps(_a.v, _mm_set1_ps(_b));
    return r;
}
math::Dir3 math::operator*(float _a, const math::Dir3& _b)
{
    math::Dir3 r;
    r.v = _mm_mul_ps(_b.v, _mm_set1_ps(_a));
    return r;
}
math::Position3 math::operator*(const math::Position3& _a, float _b)
{
    math::Position3 r;
    r.v = _mm_mul_ps(_a.v, _mm_set1_ps(_b));
    return r;
}
math::Position3 math::operator*(float _a, const math::Position3& _b)
{
    math::Position3 r;
    r.v = _mm_mul_ps(_b.v, _mm_set1_ps(_a));
    return r;
}

// Dot products (g.o 0x4A69A0-0x4A6B10)
float math::operator*(const math::Dir3& _a, const math::Dir3& _b)
{
    __m128 v2 = _mm_mul_ps(_a.v, _b.v);
    return v2.m128_f32[0]
           + (_mm_shuffle_ps(v2, v2, 0x55).m128_f32[0]
              + _mm_shuffle_ps(v2, v2, 0xAA).m128_f32[0]);
}
float math::operator*(const math::Dir3& _a, const math::Position3& _b)
{
    __m128 v2 = _mm_mul_ps(_a.v, _b.v);
    return v2.m128_f32[0]
           + (_mm_shuffle_ps(v2, v2, 0x55).m128_f32[0]
              + _mm_shuffle_ps(v2, v2, 0xAA).m128_f32[0]);
}
float math::operator*(const math::Position3& _a, const math::Dir3& _b)
{
    __m128 v2 = _mm_mul_ps(_a.v, _b.v);
    return v2.m128_f32[0]
           + (_mm_shuffle_ps(v2, v2, 0x55).m128_f32[0]
              + _mm_shuffle_ps(v2, v2, 0xAA).m128_f32[0]);
}

// Named vector ops (g.o 0x4A6BC0-0x4A6CB0)
math::Vector4 math::Mul(const math::Vector4& _a, const math::Vector4& _b)
{
    math::Vector4 r;
    r.v = _mm_mul_ps(_a.v, _b.v);
    return r;
}
math::Dir3 math::Cross(const math::Dir3& _a, const math::Dir3& _b)
{
    math::Dir3 r;
    r.v = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(_a.v, _a.v, 9),
                                _mm_shuffle_ps(_b.v, _b.v, 18)),
                     _mm_mul_ps(_mm_shuffle_ps(_a.v, _a.v, 18),
                                _mm_shuffle_ps(_b.v, _b.v, 9)));
    return r;
}
math::Position3 math::Min(const math::Position3& _a, const math::Position3& _b)
{
    math::Position3 r;
    r.v = _mm_min_ps(_a.v, _b.v);
    return r;
}
math::Position3 math::Max(const math::Position3& _a, const math::Position3& _b)
{
    math::Position3 r;
    r.v = _mm_max_ps(_a.v, _b.v);
    return r;
}

// DeclareUnit / Unitize / Zero (g.o 0x4A6E70-0x4A6F60)
math::Dir3 math::DeclareUnit(const math::Dir3& _v)
{
    math::Dir3 r;
    r.v = _v.v;
    return r;
}
math::Dir3 math::Unitize(const math::Dir3& _v)
{
    __m128 v2 = _mm_mul_ps(_v.v, _v.v);
    float len = (float)sqrt((double)(v2.m128_f32[0]
                                     + (_mm_shuffle_ps(v2, v2, 0x55).m128_f32[0]
                                        + _mm_shuffle_ps(v2, v2, 0xAA).m128_f32[0])));
    math::Dir3 r;
    r.v = _mm_div_ps(_v.v, _mm_set1_ps(len));
    return r;
}
math::Dir3 math::Dir3_Zero()
{
    math::Dir3 r;
    r.v = _mm_setzero_ps();
    return r;
}
math::Position3 math::Position3_Zero()
{
    math::Position3 r;
    r.v = _mm_setzero_ps();
    return r;
}

// Compound math ops (g.o 0x4A6CF0-0x4A6E30)
const math::Dir3& math::Dir3::operator/=(float _v)
{
    v = _mm_div_ps(v, _mm_set1_ps(_v));
    return *this;
}
const math::Position3& math::Position3::operator+=(const math::Dir3& _v)
{
    v = _mm_add_ps(v, _v.v);
    return *this;
}
const math::Position3& math::Position3::operator+=(const math::Position3& _v)
{
    v = _mm_add_ps(v, _v.v);
    return *this;
}
const math::Position3& math::Position3::operator*=(float _v)
{
    v = _mm_mul_ps(v, _mm_set1_ps(_v));
    return *this;
}
const math::Position3& math::Position3::operator/=(float _v)
{
    v = _mm_div_ps(v, _mm_set1_ps(_v));
    return *this;
}
const math::Vector4& math::Vector4::operator-=(const math::Vector4& _v)
{
    v = _mm_sub_ps(v, _v.v);
    return *this;
}

// Mat33 / Mat43 (g.o 0x4A6FB0-0x4A74A0)
math::Mat33::Mat33(const math::Dir3& _x, const math::Dir3& _y, const math::Dir3& _z)
{
    x = _x;
    y = _y;
    z = _z;
}
const math::Dir3& math::Mat33::GetX() const
{
    return x;
}
const math::Dir3& math::Mat33::GetY() const
{
    return y;
}
const math::Dir3& math::Mat33::GetZ() const
{
    return z;
}
math::Mat33::Mat33(const math::Mat33& _m)
{
    *this = _m;
}
const math::Mat33& math::Mat33::operator=(const math::Mat33& _m)
{
    x = _m.x;
    y = _m.y;
    z = _m.z;
    return *this;
}
math::Mat43::Mat43(const math::Mat43::Packed& _p)
{
    x = math::Dir3(_p.x);
    y = math::Dir3(_p.y);
    z = math::Dir3(_p.z);
    w = math::Position3(_p.w);
}
math::Mat43::Mat43(const math::Mat33& _m, const math::Position3& _p)
{
    x = _m.x;
    y = _m.y;
    z = _m.z;
    w = _p;
}
const math::Mat43& math::Mat43::operator=(const math::Mat43& _m)
{
    x = _m.x;
    y = _m.y;
    z = _m.z;
    w = _m.w;
    return *this;
}
const math::Dir3& math::Mat43::GetX() const
{
    return x;
}
const math::Dir3& math::Mat43::GetY() const
{
    return y;
}
const math::Dir3& math::Mat43::GetZ() const
{
    return z;
}
const math::Position3& math::Mat43::GetW() const
{
    return w;
}
math::Dir3& math::Mat43::GetX()
{
    return x;
}
math::Dir3& math::Mat43::GetY()
{
    return y;
}
math::Dir3& math::Mat43::GetZ()
{
    return z;
}
math::Position3& math::Mat43::GetW()
{
    return w;
}

// ============================================================================
// Matrix free functions (g.o 0x4A75B0-0x4A82C0)
// ============================================================================
math::Position3 math::Mul(const math::Position3& _v, const math::Mat43& _m)
{
    math::Position3 r;
    r.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_v.v, _v.v, 0), _m.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_v.v, _v.v, 0x55), _m.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(_v.v, _v.v, 0xAA), _m.z.v),
                   _m.w.v));
    return r;
}
math::Position3 math::operator*(const math::Position3& _v, const math::Mat43& _m)
{
    return math::Mul(_v, _m);
}
math::Position3 math::operator/(const math::Position3& _v, const math::Mat43& _m)
{
    math::Position3 r;
    __m128 v3 = _m.y.v;
    __m128 v4 = _m.z.v;
    __m128 v5 = _mm_shuffle_ps(_m.x.v, v3, 68);
    __m128 v6 = _mm_shuffle_ps(_mm_shuffle_ps(_m.x.v, v3, 238), v4, 168);
    __m128 v8 = _mm_shuffle_ps(v5, v4, 221);
    __m128 v9 = _mm_shuffle_ps(v5, v4, 136);
    r.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_v.v, _v.v, 0), v9),
            _mm_mul_ps(_mm_shuffle_ps(_v.v, _v.v, 0x55), v8)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_v.v, _v.v, 0xAA), v6),
            _mm_xor_ps(
                _mm_set1_ps(-0.0f),
                _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(_m.w.v, _m.w.v, 0), v9),
                        _mm_mul_ps(_mm_shuffle_ps(_m.w.v, _m.w.v, 0x55), v8)),
                    _mm_mul_ps(_mm_shuffle_ps(_m.w.v, _m.w.v, 0xAA), v6)))));
    return r;
}

math::Mat33 math::Mul(const math::Mat33& _a, const math::Mat33& _b)
{
    math::Mat33 r;
    __m128 v3 = _b.z.v;
    __m128 v4 = _b.y.v;
    __m128 v5 = _b.x.v;
    __m128 v6 = _a.z.v;
    r.x.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_a.x.v, _a.x.v, 0), _b.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_a.x.v, _a.x.v, 0x55), v4)),
        _mm_mul_ps(_mm_shuffle_ps(_a.x.v, _a.x.v, 0xAA), v3));
    r.y.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_a.y.v, _a.y.v, 0), _b.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_a.y.v, _a.y.v, 0x55), v4)),
        _mm_mul_ps(_mm_shuffle_ps(_a.y.v, _a.y.v, 0xAA), v3));
    r.z.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v6, v6, 0), v5),
            _mm_mul_ps(_mm_shuffle_ps(v6, v6, 0x55), v4)),
        _mm_mul_ps(_mm_shuffle_ps(v6, v6, 0xAA), v3));
    return r;
}
const math::Mat33& math::Mat33::operator*=(const math::Mat33& _m)
{
    math::Mat33 r = math::Mul(*this, _m);
    *this = r;
    return *this;
}

math::Mat43 math::Mul(const math::Mat43& _a, const math::Mat43& _b)
{
    math::Mat43 r;
    __m128 v3 = _b.z.v;
    __m128 v4 = _b.y.v;
    __m128 v5 = _b.x.v;
    r.x.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_a.x.v, _a.x.v, 0), _b.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_a.x.v, _a.x.v, 0x55), v4)),
        _mm_mul_ps(_mm_shuffle_ps(_a.x.v, _a.x.v, 0xAA), v3));
    r.y.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_a.y.v, _a.y.v, 0), _b.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_a.y.v, _a.y.v, 0x55), v4)),
        _mm_mul_ps(_mm_shuffle_ps(_a.y.v, _a.y.v, 0xAA), v3));
    r.z.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_a.z.v, _a.z.v, 0), _b.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_a.z.v, _a.z.v, 0x55), v4)),
        _mm_mul_ps(_mm_shuffle_ps(_a.z.v, _a.z.v, 0xAA), v3));
    r.w.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_a.w.v, _a.w.v, 0), v5),
            _mm_mul_ps(_mm_shuffle_ps(_a.w.v, _a.w.v, 0x55), v4)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(_a.w.v, _a.w.v, 0xAA), v3),
                   _b.w.v));
    return r;
}
math::Mat43 math::operator*(const math::Mat43& _a, const math::Mat43& _b)
{
    return math::Mul(_a, _b);
}

math::Mat43 math::Inv(const math::Mat43& _m)
{
    math::Mat43 r;
    __m128 v2 = _m.y.v;
    __m128 v3 = _m.z.v;
    __m128 v4 = _mm_shuffle_ps(_m.x.v, v2, 68);
    __m128 v5 = _mm_shuffle_ps(v4, v3, 221);
    __m128 tmp_20 = _mm_shuffle_ps(_mm_shuffle_ps(_m.x.v, v2, 238), v3, 168);
    __m128 v6 = _mm_shuffle_ps(v4, v3, 136);
    __m128 tmp_4 = v5;
    __m128 v7 = _mm_mul_ps(_mm_shuffle_ps(_m.w.v, _m.w.v, 0xAA), tmp_20);
    __m128 v8 = _mm_add_ps(
        _mm_mul_ps(_mm_shuffle_ps(_m.w.v, _m.w.v, 0), v6),
        _mm_mul_ps(_mm_shuffle_ps(_m.w.v, _m.w.v, 0x55), v5));
    r.x.v = v6;
    r.y.v = tmp_4;
    r.z.v = tmp_20;
    r.w.v = _mm_xor_ps(_mm_set1_ps(-0.0f), _mm_add_ps(v8, v7));
    return r;
}

// Vector4 cos approximation (g.o 0x4A83C0)
math::Vector4 math::Cos(const math::Vector4& radians, const math::Vector4& frequency)
{
    __m128 sign = _mm_set1_ps(-0.0f);
    __m128 floorMagic = _mm_set1_ps(12582912.0f);
    __m128 half = _mm_set1_ps(0.5f);
    __m128 quarter = _mm_set1_ps(0.25f);
    __m128 v3 = _mm_mul_ps(_mm_xor_ps(sign, _mm_andnot_ps(sign, radians.v)),
                           frequency.v);
    __m128 v5 = _mm_sub_ps(
        _mm_andnot_ps(
            sign,
            _mm_sub_ps(_mm_sub_ps(_mm_add_ps(_mm_sub_ps(v3, floorMagic),
                                             floorMagic),
                                  v3),
                       half)),
        quarter);
    __m128 v6 = _mm_mul_ps(v5, v5);
    __m128 v7 = _mm_mul_ps(v6, v6);
    __m128 v8 = _mm_mul_ps(v5, v6);
    __m128 v9 = _mm_mul_ps(v5, v7);
    math::Vector4 r;
    r.v = _mm_add_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_mul_ps(v9, v7), _mm_set1_ps(39.710659f)),
                    _mm_mul_ps(_mm_mul_ps(v8, v7), _mm_set1_ps(-76.574959f))),
                _mm_mul_ps(v9, _mm_set1_ps(81.602226f))),
            _mm_mul_ps(v8, _mm_set1_ps(-41.341675f))),
        _mm_mul_ps(v5, _mm_set1_ps(6.283185f)));
    return r;
}

bool math::Compare_all_lt(const math::Position3& _a, const math::Position3& _b)
{
    return (_mm_movemask_ps(_mm_cmplt_ps(_a.v, _b.v)) & 7) == 7;
}

// Quaternion ctors / ops (g.o 0x4A8530-0x4A8620)
math::Quaternion::Quaternion()
{
}
math::Quaternion::Quaternion(float _x, float _y, float _z, float _w)
{
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}
math::Quaternion math::operator*(const math::Quaternion& _a, float _b)
{
    math::Quaternion r;
    r.x = _a.x * _b;
    r.y = _a.y * _b;
    r.z = _a.z * _b;
    r.w = _a.w * _b;
    return r;
}
math::Quaternion math::DeclareUnit(const math::Quaternion& _q)
{
    math::Quaternion r;
    r.x = _q.x;
    r.y = _q.y;
    r.z = _q.z;
    r.w = _q.w;
    return r;
}
math::Quaternion math::GetQuaternion(const math::Mat33& rot)
{
    float v2 = rot.z.v.m128_f32[2];
    float v3 = rot.y.v.m128_f32[1];
    float v4 = rot.x.v.m128_f32[0];
    __m128 v5 = _mm_shuffle_ps(rot.x.v, _mm_shuffle_ps(_mm_setzero_ps(), rot.x.v, 160), 52);
    __m128 v6 = _mm_shuffle_ps(rot.y.v, _mm_shuffle_ps(_mm_setzero_ps(), rot.y.v, 160), 52);
    __m128 v7 = _mm_shuffle_ps(rot.z.v, _mm_shuffle_ps(_mm_setzero_ps(), rot.z.v, 160), 52);
    float v8 = (v2 + v3) + v4;
    math::Quaternion r;
    __m128 vv;
    float scale;
    if (v8 < -0.33333299f)
    {
        if (v3 <= v4)
        {
            if (v4 > v2)
            {
                vv.m128_f32[1] = _mm_shuffle_ps(v5, v5, 0x55).m128_f32[0] + v6.m128_f32[0];
                vv.m128_f32[2] = v7.m128_f32[0] + _mm_shuffle_ps(v5, v5, 0xAA).m128_f32[0];
                vv.m128_f32[3] = _mm_shuffle_ps(v7, v7, 0x55).m128_f32[0] - _mm_shuffle_ps(v6, v6, 0xAA).m128_f32[0];
                vv.m128_f32[0] = ((v4 - v3) - v2) + 1.0f;
                scale = 0.5f / sqrt(vv.m128_f32[0]);
                r.x = vv.m128_f32[0] * scale;
                r.y = vv.m128_f32[1] * scale;
                r.z = vv.m128_f32[2] * scale;
                r.w = vv.m128_f32[3] * scale;
                return r;
            }
        }
        else if (v3 > v2)
        {
            vv.m128_f32[0] = _mm_shuffle_ps(v5, v5, 0x55).m128_f32[0] + v6.m128_f32[0];
            vv.m128_f32[2] = _mm_shuffle_ps(v6, v6, 0xAA).m128_f32[0] + _mm_shuffle_ps(v7, v7, 0x55).m128_f32[0];
            vv.m128_f32[3] = _mm_shuffle_ps(v5, v5, 0xAA).m128_f32[0] - v7.m128_f32[0];
            vv.m128_f32[1] = ((v3 - v4) - v2) + 1.0f;
            scale = 0.5f / sqrt(vv.m128_f32[1]);
            r.x = vv.m128_f32[0] * scale;
            r.y = vv.m128_f32[1] * scale;
            r.z = vv.m128_f32[2] * scale;
            r.w = vv.m128_f32[3] * scale;
            return r;
        }
        vv.m128_f32[0] = v7.m128_f32[0] + _mm_shuffle_ps(v5, v5, 0xAA).m128_f32[0];
        vv.m128_f32[1] = _mm_shuffle_ps(v6, v6, 0xAA).m128_f32[0] + _mm_shuffle_ps(v7, v7, 0x55).m128_f32[0];
        vv.m128_f32[3] = v6.m128_f32[0] - _mm_shuffle_ps(v5, v5, 0x55).m128_f32[0];
        vv.m128_f32[2] = ((v2 - v4) - v3) + 1.0f;
        scale = 0.5f / sqrt(vv.m128_f32[2]);
    }
    else
    {
        vv.m128_f32[0] = _mm_shuffle_ps(v7, v7, 0x55).m128_f32[0] - _mm_shuffle_ps(v6, v6, 0xAA).m128_f32[0];
        vv.m128_f32[1] = _mm_shuffle_ps(v5, v5, 0xAA).m128_f32[0] - v7.m128_f32[0];
        vv.m128_f32[2] = v6.m128_f32[0] - _mm_shuffle_ps(v5, v5, 0x55).m128_f32[0];
        vv.m128_f32[3] = v8 + 1.0f;
        scale = 0.5f / sqrt(v8 + 1.0f);
    }
    r.x = vv.m128_f32[0] * scale;
    r.y = vv.m128_f32[1] * scale;
    r.z = vv.m128_f32[2] * scale;
    r.w = vv.m128_f32[3] * scale;
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

// Entity handle / array-index accessors (g.o 0x4A67B0-0x4A67F0)
DbLinkedHandle<EntityHandleDb, Entity> Entity::GetHandle() const
{
    DbLinkedHandle<EntityHandleDb, Entity> result;
    result.mHandle = mHandle.mHandle;
    return result;
}
void Entity::SetEntityArrayIndex(int v)
{
    mEntityArrayIndex = (int16_t)v;
}
int Entity::GetEntityArrayIndex() const
{
    return mEntityArrayIndex;
}

// Entity pak/dobj/destructible accessors (g.o 0x4A6800-0x4A68E0)
TPakId Entity::GetPakId() const
{
    if (mPakId == (int)PAK_ID_INVALID)
        return CurPakId();
    return (TPakId)mPakId;
}
DObj* Entity::GetDObj()
{
    return mDObj;
}
void Entity::SetDestructible(IVPointer<Destructible> d)
{
    mDestructible = d;
    takedamage = 1;
}
IVPointer<Destructible> Entity::GetDestructible()
{
    return mDestructible;
}

// Free EntityManager helpers (g.o 0x4A6B90 / 0x4A6BA0)
Entity* GetWorld()
{
    return EntityManager::sInst->mWorld;
}
int GetPlayerIndex(Entity* player)
{
    return EntityManager::sInst->GetPlayerIndex(player);
}

// EntityState::GetLerpAngles (g.o 0x4A5750)
const math::Position3 EntityState::GetLerpAngles() const
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

// ============================================================================
// Singleton Inst accessors (g.o 0x4A7540-0x4A9070)
// ============================================================================
EffectEventSys* EffectEventSys::Inst()
{
    return EffectEventSys::sInst;
}
TaskSys* TaskSys::Inst()
{
    return &TaskSys::sInst;
}
SoundDevice* SoundDevice::Inst()
{
    return SoundDevice::sInst;
}
SmokeGrenadeMgr* SmokeGrenadeMgr::Inst()
{
    return (SmokeGrenadeMgr*)SmokeGrenadeMgr::sInst;
}
TestFPS* TestFPS::Inst()
{
    return TestFPS::sInst;
}
bool TestFPS::IsTesting()
{
    return mTesting;
}

// PlayerStateEvents::Clear (g.o 0x4A7560)
void PlayerStateEvents::Clear()
{
    eventSequence = 0;
    oldEventSequence = 0;
    damageEvent = 0;
    damageYaw = 0;
    damagePitch = 0;
    damageCount = 0;
    entityEventSequence = 0;
    for (int i = 0; i < 4; ++i)
    {
        events[i] = 0;
        eventParms[i] = 0;
    }
}

// PathNodes::NodeHandle (g.o 0x4A75A0-0x4A76B0)
PathNodes::NodeHandle::NodeHandle()
{
    mValue = 0;
}
PathNodes::NodeHandle::NodeHandle(int value)
{
    mValue = (uint16_t)value;
}
unsigned short PathNodes::NodeHandle::GetZoneIndex() const
{
    return (unsigned short)(mValue - 1);
}
PathNodes::NodeHandle PathNodes::NodeHandle::NullHandle()
{
    PathNodes::NodeHandle h;
    h.mValue = 0;
    return h;
}
bool PathNodes::NodeHandle::IsAssigned() const
{
    return mValue != 0 && mValue != 0xFFFF;
}
bool PathNodes::NodeHandle::operator==(const PathNodes::NodeHandle& rhs) const
{
    return mValue == rhs.mValue;
}
PathNodes::NodeHandle::operator bool() const
{
    return mValue != 0 && mValue != 0xFFFF && operator->() != nullptr;
}

// scr_vehicle_t::LerpedVariables::Clear (g.o 0x4A79E0)
void scr_vehicle_t::LerpedVariables::Clear()
{
    mSteeringAngle = 0.0f;
    mTurretAngles.v.m128_f32[1] = 0.0f;
    mTurretAngles.v.m128_f32[0] = 0.0f;
    mBodyPosition.v.m128_f32[1] = 0.0f;
    mBodyPosition.v.m128_f32[0] = 0.0f;
}

// InteractionController accessors (g.o 0x4A8260-0x4A82A0)
int InteractionController::IsInteracting() const
{
    return mCurState != nullptr;
}
void InteractionController::SetFlag(unsigned int f, int enable)
{
    if (enable != 0)
        mFlags |= f;
    else
        mFlags &= ~f;
}
int InteractionController::IsFlagged(unsigned int f) const
{
    return (f & mFlags) != 0;
}

// Camera accessors (g.o 0x4A9020-0x4A9040)
ECameraModes Camera::GetCameraMode()
{
    return (ECameraModes)mCamMode;
}
EVehicleCameraMode Camera::GetVehicleCameraMode()
{
    return (EVehicleCameraMode)mVehicleCamMode;
}
float Camera::GetLastViewAngles(int axis)
{
    return mPrevAngles.v.m128_f32[axis];
}

// Handle (g.o 0x4A9100-0x4A9160)
Handle::Handle(int v)
{
    mVal = (unsigned int)v;
}
Handle Handle::NullHandle()
{
    Handle h;
    h.mVal = 0;
    return h;
}
bool Handle::IsUnassigned() const
{
    return mVal == 0;
}
unsigned int Handle::GetVal() const
{
    return mVal;
}
bool operator==(Handle lhs, Handle rhs)
{
    return lhs.mVal == rhs.mVal;
}

// ============================================================================
// Batch 23: managers / physics accessors / string & hash helpers
// ============================================================================

// MPEntityHandle global class (g.o 0x4A9760)
class MPEntityHandle {
public:
    unsigned short mVal;  // +0x00
    MPEntityHandle& operator=(const MPEntityHandle& other);
};
MPEntityHandle& MPEntityHandle::operator=(const MPEntityHandle& other)
{
    mVal = other.mVal;
    return *this;
}

// Manager singletons (g.o 0x4A9780-0x4A9E70)
MultiplayerMgr* MultiplayerMgr::Inst()
{
    return MultiplayerMgr::sInst;
}
MPPeer* MultiplayerMgr::GetPeer()
{
    return mPeer;
}
PathNodeMgr* PathNodeMgr::Inst()
{
    return PathNodeMgr::sInst;
}
int PathNodeMgr::GetTotalNodeCount() const
{
    if (mLevelTOC != nullptr)
        return mLevelTOC->mNodeCount;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PathNodeMgr.h";
    AeAssert::gCurrentLine = 208;
    AeAssert::gCurrentExpr = "mLevelTOC";
    if (AeAssert::IsIgnored())
        return mLevelTOC->mNodeCount;
    if (AeAssert::Assert("old cod assert"))
    {
        __debugbreak();
        return mLevelTOC->mNodeCount;
    }
    return mLevelTOC->mNodeCount;
}
CheckpointMgr* CheckpointMgr::Inst()
{
    return CheckpointMgr::sInst;
}
bool CheckpointMgr::CheckpointSaveExists()
{
    return mCheckpointSaveExists;
}
bool CheckpointMgr::IsRestoringCheckpoint() const
{
    return mCheckpointSaveExists && mUsingCheckpoints;
}
const float (&CheckpointMgr::GetPlayerPosition() const)[3]
{
    return *(const float (*)[3])&mOrigin;
}
EntityHandleDb* EntityHandleDb::Inst()
{
    return &EntityHandleDb::sInst;
}
const ae_sized_array<Entity*, 4096>& EntityHandleDb::GetActiveList() const
{
    return *(const ae_sized_array<Entity*, 4096>*)&mActiveList;
}
TimerRenderBars* TimerRenderBars::Inst()
{
    return &TimerRenderBars::sInst;
}
void TimerRenderBars::ToggleActive()
{
    mActive ^= 1;
}
int TimerRenderBars::IsActive() const
{
    return mActive;
}
ConfigStringManager* ConfigStringManager::Inst()
{
    return ConfigStringManager::sInst;
}
SceneManager* SceneManager::Inst()
{
    return SceneManager::sInst;
}
InplaceVector<unsigned char>* SceneManager::GetPersistantStorage()
{
    return mPersistantStorage;
}
PlayerAnimMgr* PlayerAnimMgr::sInst;
PlayerAnimMgr* PlayerAnimMgr::Inst()
{
    return PlayerAnimMgr::sInst;
}
DynamicDecalMgr* DynamicDecalMgr::Inst()
{
    return (DynamicDecalMgr*)DynamicDecalMgr::sInst;
}

// Physics accessors (g.o 0x4A9970-0x4A9F20)
const unsigned int rigid_body::get_flag(unsigned int f) const
{
    return f & m_flags;
}
const unsigned int rigid_body::is_stable() const
{
    return m_flags & 4;
}
const unsigned int rigid_body_constraint_wheel::get_wheel_flag(
    rigid_body_constraint_wheel::wheel_flags_e f) const
{
    return f & m_wheel_flags;
}
const math::Dir3 rigid_body_constraint_wheel::get_hitp_loc() const
{
    return m_b2_hitp_loc;
}
const float rigid_body_constraint_wheel::get_wheel_vel() const
{
    return m_wheel_vel;
}
float rb_vehicle::get_throttle() const
{
    return m_throttle;
}
float rb_vehicle::get_steer_factor() const
{
    return m_steer_factor;
}
float rb_vehicle::get_forward_vel() const
{
    return m_forward_vel;
}
vehicle_rb_parameter* rb_vehicle::get_parameter() const
{
    return m_parameter;
}
const rb_extra_info* rb_vehicle::get_chassis_rbinf() const
{
    return (const rb_extra_info*)m_chassis_rbinf;
}
rigid_body_constraint_wheel* rb_vehicle::get_wheel(int i)
{
    return m_wheels[i];
}
const unsigned int rb_vehicle::get_braking() const
{
    return m_state_flags & 2;
}
const float rb_vehicle::get_max_speed() const
{
    return m_parameter->m_speed_max;
}
const unsigned int rb_vehicle::get_flag(rb_vehicle_model_flags_e f) const
{
    return f & m_state_flags;
}

// Scalar helpers (g.o 0x4A9A00-0x4A9A70)
bool IS_NAN(const float& x)
{
    return (_fpclass((double)x) & 0x297) != 0;
}
int FastRound(float x)
{
    return (int)(x + 0.5f);
}
float Q_fabs(float f)
{
    return (float)fabs((double)f);
}

// usercmd_s::Clear (g.o 0x4A9AC0)
void usercmd_s::Clear()
{
    serverTime = 0;
    buttons = 0;
    weapon = 0;
    angles[0] = 0;
    angles[1] = 0;
    angles[2] = 0;
    forwardmove = 0;
    rightmove = 0;
    upmove = 0;
    gunPitch = 0.0f;
    gunYaw = 0.0f;
    gunXOfs = 0.0f;
    gunYOfs = 0.0f;
    gunZOfs = 0.0f;
}

// trajectory_t ctor (g.o 0x4A9B10)
trajectory_t::trajectory_t()
{
    trType = TR_STATIONARY;
    trTime = 0;
    trDuration = 0;
    trGravityOverride = 0;
    trBase[1] = 0.0f;
    trBase[0] = 0.0f;
    trDelta[1] = 0.0f;
    trDelta[0] = 0.0f;
}

// HashString (g.o 0x4A9B60-0x4A9C50)
HashString::HashString(const char* str)
{
    mHash = HashString::CalcHash(str);
}
HashString::HashString(int hash)
{
    mHash = (unsigned int)hash;
}
unsigned int HashString::GetHash() const
{
    return mHash;
}
bool HashString::Compare(const HashString& lhs, const HashString& rhs)
{
    return lhs.mHash == rhs.mHash;
}
bool operator==(const HashString& lhs, const HashString& rhs)
{
    return lhs.mHash == rhs.mHash;
}
bool operator!=(const HashString& lhs, const HashString& rhs)
{
    return lhs.mHash != rhs.mHash;
}
bool operator==(const HashString& lhs, unsigned int rhs)
{
    return lhs.mHash == rhs;
}
bool operator!=(const HashString& lhs, unsigned int rhs)
{
    return lhs.mHash != rhs;
}

// Broc::string helpers (g.o 0x4A9C70-0x4A9D20)
char* Broc::string::Block::GetBuff()
{
    return (char*)(this + 1);
}
char Broc::string::operator[](unsigned int idx)
{
    if (mBlock != nullptr && idx < mBlock->mLength)
        return *(mBlock->GetBuff() + idx);
    return 0;
}
bool Broc::string::IsDefined() const
{
    return mBlock != nullptr;
}
void Broc::string::SetUndefined()
{
    if (mBlock != nullptr)
    {
        mBlock->DecrementCount();
        mBlock = nullptr;
    }
}
bool Broc::operator!=(const Broc::string& lhs, const Broc::string& rhs)
{
    return !Broc::operator==(lhs, rhs);
}

// TaskFunctor1 dtors (g.o 0x4A9F90 / 0x4A9FA0)
template <typename T, typename U>
struct TaskFunctor1 : TaskFunctor {
    void* fn;
    U     deltaT;
    virtual ~TaskFunctor1();
};
template <typename T, typename U>
TaskFunctor1<T, U>::~TaskFunctor1()
{
}
template struct TaskFunctor1<AnimationUpdateTask, float>;
template struct TaskFunctor1<XAnimUpdateTask, float>;

// ============================================================================
// Batch 24: vehicle/debug/scr ctors + container template instantiations
// ============================================================================

// scr_animscript_t (g.o 0x4ABF00 / 0x4ABF20)
scr_animscript_t::scr_animscript_t()
{
    new (data) Broc::string((Broc::string::Block*)nullptr);
}
scr_animscript_t::~scr_animscript_t()
{
    ((Broc::string*)data)->~string();
}

// vehSqr (g.o 0x4ABF60)
float vehSqr(float x)
{
    return x * x;
}

// vehicle_node_t (g.o 0x4ABF80 / 0x4AC1C0 / 0x4AC310)
vehicle_node_t::vehicle_node_t()
{
    new (&mName) Broc::string((Broc::string::Block*)nullptr);
    new (&mTarget) Broc::string((Broc::string::Block*)nullptr);
    new (&script_noteworthy) Broc::string((Broc::string::Block*)nullptr);
}
vehicle_node_t::vehicle_node_t(const vehicle_node_t& that)
{
    new (&mName) Broc::string(that.mName);
    new (&mTarget) Broc::string(that.mTarget);
    speed = that.speed;
    lookAhead = that.lookAhead;
    new (&script_noteworthy) Broc::string(that.script_noteworthy);
    origin[0] = that.origin[0];
    origin[1] = that.origin[1];
    origin[2] = that.origin[2];
    dir[0] = that.dir[0];
    dir[1] = that.dir[1];
    dir[2] = that.dir[2];
    angles[0] = that.angles[0];
    angles[1] = that.angles[1];
    angles[2] = that.angles[2];
    length = that.length;
    unsigned int dst = (unsigned int)nextIdx;
    dst = ((unsigned int)that.nextIdx & 0x3FFF) | (dst & ~0x3FFF);
    dst = ((unsigned int)that.nextIdx & 0xFFFC000) | (dst & ~0xFFFC000);
    dst = ((unsigned int)that.nextIdx & 0x30000000) | (dst & ~0x30000000);
    dst = ((unsigned int)that.nextIdx & 0x3FFFFFFF) | (dst & ~0x3FFFFFFF);
    nextIdx = (int)dst;
}
vehicle_node_t& vehicle_node_t::operator=(const vehicle_node_t& rhs)
{
    mName = rhs.mName;
    mTarget = rhs.mTarget;
    speed = rhs.speed;
    lookAhead = rhs.lookAhead;
    script_noteworthy = rhs.script_noteworthy;
    origin[0] = rhs.origin[0];
    origin[1] = rhs.origin[1];
    origin[2] = rhs.origin[2];
    dir[0] = rhs.dir[0];
    dir[1] = rhs.dir[1];
    dir[2] = rhs.dir[2];
    angles[0] = rhs.angles[0];
    angles[1] = rhs.angles[1];
    angles[2] = rhs.angles[2];
    length = rhs.length;
    unsigned int dst = (unsigned int)nextIdx;
    dst = ((unsigned int)rhs.nextIdx & 0x3FFF) | (dst & ~0x3FFF);
    dst = ((unsigned int)rhs.nextIdx & 0xFFFC000) | (dst & ~0xFFFC000);
    dst = ((unsigned int)rhs.nextIdx & 0x30000000) | (dst & ~0x30000000);
    dst = ((unsigned int)rhs.nextIdx & 0x3FFFFFFF) | (dst & ~0x3FFFFFFF);
    nextIdx = (int)dst;
    return *this;
}

// debug render primitives (g.o 0x4AC060-0x4AC140)
debug_sphere::debug_sphere()
{
    radius = 0.0f;
}
debug_sphere::debug_sphere(const math::Position3& center, float _radius,
                           const Color& _color)
{
    x = center.v.m128_f32[0];
    y = center.v.m128_f32[1];
    z = center.v.m128_f32[2];
    radius = _radius;
    color[0] = _color.r;
    color[1] = _color.g;
    color[2] = _color.b;
    color[3] = _color.a;
}
debug_aabb::debug_aabb(const math::Position3& _bmin,
                       const math::Position3& _bmax, const Color& _color)
{
    bmin.v = _bmin.v;
    bmax.v = _bmax.v;
    color[0] = _color.r;
    color[1] = _color.g;
    color[2] = _color.b;
    color[3] = _color.a;
}

// tagInfo_t ctor (g.o 0x4AC460)
tagInfo_t::tagInfo_t()
{
    name.mHash = 0;
}

// Explicit template instantiations (g.o 0x4AC4A0-0x4ACE80)
class ae_heap_base;
class PhysData;
template class ae_sized_array<ae_heap_base*, 32>;
template class ae_sized_array<Entity*, 4096>;
template class DbLinkedHandle<EntityHandleDb, Entity>;
template class ae_fixed_string<64, unsigned char>;
template class ae_fixed_string<256, unsigned short>;
template class ae_fixed_string<32, unsigned char>;
template class IVPointer<XModel>;
template class IVPointer<PhysData>;
template class InplaceVector<math::Mat43::Packed>;
template class InplaceVector<XBoneHierarchy>;
template class InplaceVector<nglMesh*>;
template class InplaceVector<XAnimEntry>;
