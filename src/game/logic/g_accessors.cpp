// ============================================================================
// g_accessors.cpp - g.o singleton/accessor/math-wrapper cluster
// (Inst accessors, Task memory ops, math shims, trace_t helpers).
// ============================================================================

#include "core/math_types.h"
#include "core/mem_heap.h"
#include "core/PoolAllocator.h"
#include "core/tlFixedString.h"
#include "game/AeThreadFunctor.h"
#include "game/logic/g_local.h"

#include <intrin.h>
#include <float.h>
#include <math.h>
#include <new>
#include <stdint.h>
#include <utility>

namespace PlayerStats {
int TotalScoreForStats(short* const stats);
}

extern float sNaN;
extern float nslGetWaveParam(nslWaveID waveID, int paramIndex,
                             float defaultValue);

void* DestructibleBankManager::operator new(size_t, void* p)
{
    return p;
}

PoolAllocator* AeThreadFunctor::sAllocator;

// core.o 0x4B5320
void AeThreadFunctor::SetAllocator(PoolAllocator* allocator)
{
    AeThreadFunctor::sAllocator = allocator;
}

// core.o 0x4B5330
bool Broc::vector::IsDefined() const
{
    return x != sNaN || y != sNaN || z != sNaN;
}

// core.o 0x4B5490
void math::mathInit()
{
    _controlfp(0x300u, 0x300u);
    _controlfp(0x20000u, 0x30000u);
    _mm_setcsr(_mm_getcsr() | 0x6000u);
}

// core.o 0x4B54E0
float math::Abs(float a)
{
    return fabsf(a);
}

// core.o 0x4B54F0
unsigned __int64 AeThreadManager::GetTimeStartExec()
{
    return AeThreadManager::sTimeStart;
}

// core.o 0x4B5610
void Entity::SetNotifySet(EntityNotifySet* n)
{
    mNotifySet = n;
}

// core.o 0x4B5630
const math::Position3& Entity::GetPosition()
{
    return r.currentOrigin;
}

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

void* SoundDevice::operator new(size_t, void* p)
{
    return p;
}

SoundDevice::SoundHandleDb* SoundDevice::SoundHandleDb::Inst()
{
    return &SoundDevice::SoundHandleDb::sInst;
}

float SoundDevice::Sound::GetStartingPitch() const
{
    return nslGetWaveParam((nslWaveID)mWave, 1, 1.0f);
}

float SoundDevice::Sound::GetStartingVolume() const
{
    return nslGetWaveParam((nslWaveID)mWave, 0, 1.0f);
}

float SoundDevice::Sound::GetMaxDist() const
{
    return nslGetWaveParam((nslWaveID)mWave, 26, 1.0f);
}

float SoundDevice::Sound::GetMinDist() const
{
    return nslGetWaveParam((nslWaveID)mWave, 25, 1.0f);
}

bool SoundDevice::Sound::IsSourceValid() const
{
    return mSource != -1;
}

nslSourceID SoundDevice::Sound::GetSourceId() const
{
    return (nslSourceID)mSource;
}

float SoundDevice::GetVolScale() const
{
    return mVolScale;
}

// ea: 0x004B5150
void Task::SetAllocator(PoolAllocator* allocator)
{
    Task::sAllocator = allocator;
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
class TaskHandler {
public:
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
math::Vector4::Vector4(const math::Position3& _v)
{
    v = _v.val34().v;
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
math::Mat43::Mat43(const math::Dir3& _x, const math::Dir3& _y,
                   const math::Dir3& _z, const math::Position3& _w)
{
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}
void math::Mat43::SetX(const math::Dir3& _x)
{
    x = _x;
}
void math::Mat43::SetY(const math::Dir3& _y)
{
    y = _y;
}
void math::Mat43::SetZ(const math::Dir3& _z)
{
    z = _z;
}
void math::Mat43::SetW(const math::Position3& _w)
{
    w = _w;
}

math::Dir3 math::UnitDirX()
{
    math::Dir3 result;
    result.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    return result;
}
math::Dir3 math::UnitDirY()
{
    math::Dir3 result;
    result.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    return result;
}
math::Dir3 math::UnitDirZ()
{
    math::Dir3 result;
    result.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    return result;
}

const math::Mat44& math::Mat44::operator=(const math::Mat44& _m)
{
    x = _m.x;
    y = _m.y;
    z = _m.z;
    w = _m.w;
    return *this;
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
class ScriptEventParams {
public:
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

// shell.o 0x5AD2D0
const math::Position3* EntityState::GetLerpOrigin(math::Position3* result)
{
    *result = lerpOrigin;
    return result;
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

// MPEntityHandle global class (g.o 0x4A9760) - definition lives in sv_stubs.h
MPEntityHandle::MPEntityHandle()
{
    mValue = 0;
}
MPEntityHandle::MPEntityHandle(unsigned short peerId, unsigned short index)
{
    if (((peerId + 1) & 0x1FFE0) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp\\MPEntityHandleDB.h";
        AeAssert::gCurrentLine = 28;
        AeAssert::gCurrentExpr = "(peerId + 1) >> 5 == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((index & 0xF800u) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\mp\\MPEntityHandleDB.h";
        AeAssert::gCurrentLine = 28;
        AeAssert::gCurrentExpr = "!( ((32 - 1) << (16 - 5)) & index )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    mValue = (unsigned short)(index | ((peerId + 1) << 11));
}
MPEntityHandle::MPEntityHandle(const MPEntityHandle& value)
{
    mValue = value.mValue;
}
unsigned short MPEntityHandle::GetPeerEntityIndex() const
{
    return mValue & 0x7FF;
}
unsigned short MPEntityHandle::GetValue() const
{
    return mValue;
}
bool MPEntityHandle::IsAssigned() const
{
    return mValue != 0;
}
MPEntityHandle& MPEntityHandle::operator=(const MPEntityHandle& other)
{
    mValue = other.mValue;
    return *this;
}

// Manager singletons (g.o 0x4A9780-0x4A9E70)
MultiplayerMgr* MultiplayerMgr::Inst()
{
    return MultiplayerMgr::sInst;
}

// ea: 0x005E9F50
bool MultiplayerMgr::IsRankedGame()
{
    return this->mRankedGame;
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

// ============================================================================
// Batch 24: vehicle/debug/scr ctors + container template instantiations
// ============================================================================

// ??_H vector-ctor-iterator helper (forwarded from MASM thunk)
void __stdcall vector_ctor_iterator_helper(char* t, unsigned int s, int n,
                                           void* (__thiscall* f)(void*))
{
    for (int i = n; i != 0; --i)
    {
        f(t);
        t += s;
    }
}

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
template class InplaceVector<math::Mat43::Packed>;
template class InplaceVector<XBoneHierarchy>;
template class InplaceVector<nglMesh*>;
template class InplaceVector<XAnimEntry>;

// ============================================================================
// Batch 25: nal orientation, template container instantiations (g.o 0x4A9180-0x4AE540)
// ============================================================================

// Handle operator!= (g.o 0x4A9180)
bool operator!=(Handle lhs, Handle rhs)
{
    return lhs.mVal != rhs.mVal;
}

// nal matrix / position-orientation (g.o 0x4A9240-0x4A96D0)
class nalMatrix4x4 : public math::Mat44 {
public:
    nalMatrix4x4() {}
};
class nalPositionOrientation {
public:
    math::Quaternion o;  // +0x00
    math::Dir3 p;        // +0x10

    nalPositionOrientation(const math::Dir3& _p, const math::Quaternion& _o);
    nalPositionOrientation(const nalMatrix4x4& m);
};
math::Quaternion nalQuaternionFromMatrix(const nalMatrix4x4& m)
{
    return math::GetQuaternion(
        math::Mat33(math::Dir3(m.x), math::Dir3(m.y), math::Dir3(m.z)));
}
nalPositionOrientation::nalPositionOrientation(const math::Dir3& _p,
                                               const math::Quaternion& _o)
{
    o = _o;
    p = _p;
}
nalPositionOrientation::nalPositionOrientation(const nalMatrix4x4& m)
{
    o = nalQuaternionFromMatrix(m);
    p = math::Dir3(m.w);
}

// vehicle_physic_t global ctor (g.o 0x4ABF70)
class vehicle_physic_t {
public:
    vehicle_physic_t();
};
vehicle_physic_t::vehicle_physic_t()
{
}

// debug render assignment (g.o 0x4AE140 / 0x4AE190)
debug_sphere& debug_sphere::operator=(const debug_sphere& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    radius = other.radius;
    for (int i = 0; i < 4; ++i)
        color[i] = other.color[i];
    return *this;
}
debug_aabb& debug_aabb::operator=(const debug_aabb& other)
{
    bmin = other.bmin;
    bmax = other.bmax;
    for (int i = 0; i < 4; ++i)
        color[i] = other.color[i];
    return *this;
}

// cdl_array / phys_static_array members (g.o 0x4AC780-0x4AE4B0)
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);
struct vi4 {
    int v;  // +0x00
};
template <typename T>
class cdl_array {
public:
    unsigned int m_count;    // +0x00
    T*           m_elements; // +0x04

    unsigned int size() const { return m_count; }  // ?size@?$cdl_array@...@@QBEIXZ
    const T& operator[](unsigned int index) const; // ?A@?$cdl_array@...@@QBEABU...@@I@Z
};
template <typename T>
const T& cdl_array<T>::operator[](unsigned int index) const
{
    if (index >= m_count
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    return m_elements[index];
}
template class cdl_array<cdl_object_t>;
template class cdl_array<unsigned char>;
template class cdl_array<cdlPlane>;
template class cdl_array<cdl_brush_t>;
template class cdl_array<cdl_vinfo_t>;
template class cdl_array<vi4>;
template class cdl_array<cdl_patch_t>;

// ae_array<T,SIZE> (g.o 0x4AC7E0)
template <typename T, int SIZE>
class ae_array {
public:
    T m_elements[SIZE];  // +0x00

    T& operator[](int idx) { return m_elements[idx]; }
};

template <typename T, int CAPACITY>
const T& phys_static_array<T, CAPACITY>::operator[](int i) const
{
    if ((i < 0 || i >= m_alloc_count)
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                     114, "i >= 0 && i < m_alloc_count", "invalid index"))
        __debugbreak();
    return m_slot_array[i];
}
template <typename T, int CAPACITY>
void phys_static_array<T, CAPACITY>::call_destructors()
{
}
template <typename T, int CAPACITY>
void phys_static_array<T, CAPACITY>::reset_buffer()
{
    m_alloc_count = 0;
}
template <typename T, int CAPACITY>
phys_static_array<T, CAPACITY>::~phys_static_array()
{
}
template class phys_static_array<proxy_obj_t, 256>;
template class phys_static_array<bounded_proxy_obj_t, 128>;

// cFreeList real bodies (g.o 0x4AD540-0x4ADA70)
template <typename T>
void cFreeList<T>::Init(int num)
{
    if (mpFree != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FreeList.h";
        AeAssert::gCurrentLine = 21;
        AeAssert::gCurrentExpr = "mpFree == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    for (int i = num; i > 0; --i)
    {
        T* node = (T*)PakManager::sInst->MemAlloc(
            PakManager::sInst->FindPakId(kPakTypeGlobal), sizeof(T), false);
        *(void**)node = mpFree;
        ++mFree;
        mpFree = node;
    }
}
template <typename T>
void cFreeList<T>::Shutdown()
{
    if (mUsed != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FreeList.h";
        AeAssert::gCurrentLine = 33;
        AeAssert::gCurrentExpr = "mUsed == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    while (mpFree != nullptr)
    {
        T* next = (T*)*(void**)mpFree;
        PakManager::sInst->MemFree(
            PakManager::sInst->FindPakId(kPakTypeGlobal), mpFree, false);
        --mFree;
        mpFree = next;
    }
}
template <typename T>
void cFreeList<T>::Free(T* ptr)
{
    if (ptr != nullptr)
    {
        --mUsed;
        ++mFree;
        *(void**)ptr = mpFree;
        mpFree = ptr;
    }
}
template class cFreeList<Entity>;
template class cFreeList<DSkel>;
template class cFreeList<DSkelMax>;
template class cFreeList<DSkel4>;

// ae_vector / ae_sized_array remaining instantiations (g.o 0x4ACF00-0x4AE100)
template class ae_vector<DbLinkedHandle<EntityHandleDb, Entity>>;
template class ae_vector<debug_sphere>;
template class ae_vector<debug_aabb>;
template class ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 1000>;
template class ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 64>;
template class ae_sized_array<Entity*, 128>;
template class ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 1000>;
template class ae_sized_array<int, 45>;
template class ae_sized_array<ae_fixed_string<512, unsigned short>, 64>;
template class ae_array<CGBank*, 99>;
template class InplaceVector<AnimTree>;
template class InplaceVector<InplaceTreeElement<unsigned int, InplaceString>>;
template class ae_fixed_string<1024, unsigned short>;
template class ae_fixed_string<128, unsigned char>;
template class DbLinkedHandle<TaskSys, Task>;

// ae_pair (g.o 0x4AD2C0-0x4ADFB0)
template <typename T1, typename T2>
class ae_pair {
public:
    T1 m_first;   // +0x00
    T2 m_second;  // +0x04

    ae_pair(const T1& f, const T2& s) : m_first(f), m_second(s) {}
};
template class ae_pair<const char*, void (*)()>;
template class ae_pair<const char*, void (*)(Entity*)>;
template class ae_pair<const char*, unsigned int>;

// std::pair<unsigned int, const char*> default ctor (g.o 0x4ACEE0)
void force_std_pair_ctor_emit()
{
    std::pair<unsigned int, const char*> p;
    (void)p;
}

// IVPointer Deref (g.o 0x4AE4C0-0x4AE540)
extern void ValidatePakId(TPakId pakId);
template <typename T>
T* IVPointer<T>::Deref() const
{
    ValidatePakId((TPakId)mPakId);
    return mValue;
}
template class IVPointer<XModel>;
template class IVPointer<PhysData>;
template class IVPointer<Destructible>;

// ============================================================================
// Batch 26a: misc small accessors (g.o 0x4AD340-0x4B0100)
// ============================================================================

// cFreeList<DObj>/<trRefEntity> (g.o 0x4AD560-0x4ADBE0)
template class cFreeList<DObj>;
template class cFreeList<trRefEntity>;

// tl_clamp / ae_max / ae_min (g.o 0x4AEA60-0x4AEAB0)
template <typename T, typename U, typename V>
T tl_clamp(const T& v, const U& mn, const V& mx)
{
    if (mn > v)
        return (T)mn;
    if (v > mx)
        return (T)mx;
    return v;
}
template float tl_clamp<float, float, float>(const float&, const float&,
                                             const float&);
template <typename T>
T ae_max(const T& lhs, const T& rhs)
{
    return (lhs <= rhs) ? rhs : lhs;
}
template int ae_max<int>(const int&, const int&);
template <typename T>
T ae_min(const T& lhs, const T& rhs)
{
    return (lhs >= rhs) ? rhs : lhs;
}
template int ae_min<int>(const int&, const int&);

// PakDelete (g.o 0x4AEAD0)
template <typename T>
void PakDelete(TPakId id, T* obj, bool bUseActorHeap)
{
    if (obj != nullptr)
        PakManager::sInst->MemFree(id, obj, bUseActorHeap);
}
template void PakDelete<vehicle_follow>(TPakId, vehicle_follow*, bool);

// inside_aabb_aabb (g.o 0x4AEBB0)
bool inside_aabb_aabb(const math::Position3& min0, const math::Position3& max0,
                      const math::Position3& min1, const math::Position3& max1)
{
    __m128 d = _mm_max_ps(_mm_sub_ps(min1.v, min0.v),
                          _mm_sub_ps(max0.v, max1.v));
    return (_mm_movemask_ps(_mm_cmplt_ps(d, _mm_setzero_ps())) & 7) == 7;
}

// trace_t::Clear (g.o 0x4AEC20)
void trace_t::Clear()
{
    normal.v = _mm_setzero_ps();
    endpos.v = _mm_setzero_ps();
    fraction = 0.0f;
    surfaceFlags = 0;
    contents = 0;
    shader = nullptr;
    mEntity.mHandle.mVal = 0;
    partName.mHash = 0;
    partGroup = HITLOC_NONE;
    allsolid = 0;
    startsolid = 0;
    check_decal = false;
}

// cdl_object_t accessors (g.o 0x4AECC0-0x4AEDA0)
const math::Position3 cdl_object_t::get_center_local() const
{
    math::Position3 r;
    r.v = _mm_set_ps(0.0f, center[2], 0.0f, center[0]);
    return r;
}
math::Dir3 cdl_object_t::get_box_radius() const
{
    math::Dir3 r;
    r.v = _mm_set_ps(0.0f, box_radius[2], 0.0f, box_radius[0]);
    return r;
}
math::Position3 cdl_object_t::get_max() const
{
    math::Position3 r;
    r.v = _mm_add_ps(_mm_set_ps(0.0f, center[2], 0.0f, center[0]),
                     _mm_set_ps(0.0f, box_radius[2], 0.0f, box_radius[0]));
    return r;
}

// DCGSet / CGBank (g.o 0x4AEE40-0x4AEFC0)
unsigned int DCGSet::size() const
{
    return (unsigned int)objects_m_count;
}
const cdl_object_t& DCGSet::get_object(unsigned short index) const
{
    return ((const cdl_object_t*)objects_m_elements)[index];
}
unsigned int CGBank::size() const
{
    return (unsigned int)objects.m_count;
}
const cdl_object_t& CGBank::get_object(unsigned short index) const
{
    return ((const cdl_object_t*)objects.m_elements)[index];
}
CGBank* CGBankManager::GetBank(TPakId pakId)
{
    return mBankArray[pakId];
}

// XModel / XModelParts (g.o 0x4AF100 / 0x4AF110)
int XModelParts::GetNumBones() const
{
    return (int)mHierarchy.mSize;
}
int XModel::GetNumBones(int lodIndex) const
{
    const XModelParts* parts = GetXModelParts(lodIndex);
    if (parts != nullptr)
        return (int)parts->mHierarchy.mSize;
    return 0;
}

// Task::IsActive (g.o 0x4AF150)
bool Task::IsActive() const
{
    return (mFlags & 4) == 0;
}

// EntityState::SetLerpOrigin (g.o 0x4AF2A0)
void EntityState::SetLerpOrigin(const math::Position3& origin)
{
    if (lerpOrigin.v.m128_f32[0] != origin.v.m128_f32[0]
        || lerpOrigin.v.m128_f32[1] != origin.v.m128_f32[1]
        || lerpOrigin.v.m128_f32[2] != origin.v.m128_f32[2])
    {
        lerpOrigin.v = origin.v;
        eFlags |= 0x40000000;
    }
}

// Entity::AssignHandle (g.o 0x4AF3A0)
void Entity::AssignHandle(Handle h)
{
    mHandle.mHandle.mVal = h.mVal;
}

// trigger_info_t::Clear (g.o 0x4AFD20)
void trigger_info_t::Clear()
{
    mEntity.mHandle.mVal = 0;
    mOtherEntity.mHandle.mVal = 0;
    useCount = 0;
    otherUseCount = 0;
}

// VehicleNodeAllocator ctor (g.o 0x4B0040)
VehicleNodeAllocator::VehicleNodeAllocator()
{
    m_numNodes = 0;
    m_numBlocks = 0;
    m_currentBlockIndex = 0;
    for (int i = 0; i < 16; ++i)
        m_pNodeBlocks[i] = nullptr;
}

// ============================================================================
// Batch 27: container machinery + remaining accessors (g.o 0x4AE5D0-0x4B1600)
// ============================================================================

// ae_sized_array_base ctors (g.o 0x4AE5F0 / 0x4AE600 / 0x4AE6E0)
template class ae_sized_array_base<DbLinkedHandle<EntityHandleDb, Entity>, 256>;
template class ae_sized_array_base<DbLinkedHandle<EntityHandleDb, Entity>, 64>;
template class ae_sized_array_base<DbLinkedHandle<EntityHandleDb, Entity>, 1000>;
template class ae_sized_array_base<ae_fixed_string<512, unsigned short>, 64>;

// HandleDb GetObject / BindObjectToHandle (g.o 0x4B0DA0 / 0x4B0E20)
Entity* EntityHandleDb::GetObject(int idx) const
{
    if ((unsigned int)idx >= 0x540)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
        AeAssert::gCurrentLine = 78;
        AeAssert::gCurrentExpr = "idx >= 0 && idx < _MaxEltements";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("index out of bounds"))
            __debugbreak();
    }
    return mElements[idx].mObject;
}
void EntityHandleDb::BindObjectToHandle(Handle handle, Entity* obj)
{
    unsigned int v3 = handle.mVal & 0xFFF;
    if (v3 < 0x540)
    {
        if (mElements[v3].mKey != (int)(handle.mVal >> 12))
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
            AeAssert::gCurrentLine = 123;
            AeAssert::gCurrentExpr = "element.GetKey() == h.GetKey()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("handle was not allocated for this object"))
                __debugbreak();
        }
        mElements[v3].mObject = obj;
    }
}

// TaskFunctor1 ctors/Update (g.o 0x4B1510-0x4B15C0)
template <typename T, typename U>
class TaskFunctor1 {
public:
    void (T::*mFp)(Entity*, U);
    U  mA1;

    TaskFunctor1(void (T::*fp)(Entity*, U), const U& a1)
        : mFp(fp), mA1(a1)
    {
    }
    virtual ~TaskFunctor1() {}
    virtual void Update(Task* t, Entity* e)
    {
        (((T*)t)->*mFp)(e, mA1);
    }
};
template class TaskFunctor1<AnimationUpdateTask, float>;
template class TaskFunctor1<XAnimUpdateTask, float>;
void force_taskfunctor1_delete(TaskFunctor1<AnimationUpdateTask, float>* p)
{
    delete p;
}
void force_taskfunctor1_delete_x(TaskFunctor1<XAnimUpdateTask, float>* p)
{
    delete p;
}

// collision_context_t::filter (g.o 0x4AF020)
bool collision_context_t::filter(Entity* ent) const
{
    (void)ent;
    return false;
}

// player_collision_context_t ctor (g.o 0x4B0000)
player_collision_context_t::player_collision_context_t(
    DbLinkedHandle<EntityHandleDb, Entity> handle, int mask)
    : collision_context_t()
{
    pass_entity1.mHandle.mVal = 0;
    pass_entity2.mHandle.mVal = 0;
    pass_owner1.mHandle.mVal = 0;
    pass_owner2.mHandle.mVal = 0;
    pass_entity1 = handle;
    contentmask = mask;
}

// player_collision_context_t default constructor (used by PM contexts)
player_collision_context_t::player_collision_context_t()
    : collision_context_t()
{
}

// ============================================================================
// Batch 28: HandleDb/DbLinkedHandle/IVPointer/BitSet/WaitTilOutput cluster
// ============================================================================

// Local minimal BitSet + SizedHandle + HandleDb (binary template manglings;
// g_accessors.cpp cannot include core_systems.h where the other BitSet lives).
template <int N>
class BitSet {
public:
    unsigned char mBits[(N + 7) / 8];
    BitSet() { memset(mBits, 0, sizeof(mBits)); }
    void Clear()
    {
        for (int i = (N + 31) / 32 - 1; i >= 0; --i)
            ((unsigned int*)mBits)[i] = 0;
    }
    void Add(int v) { ((unsigned int*)mBits)[v >> 5] |= 1u << (v & 0x1F); }
    void Rmv(int v) { ((unsigned int*)mBits)[v >> 5] &= ~(1u << (v & 0x1F)); }
    BitSet<N> operator~() const
    {
        BitSet<N> r;
        for (int i = 0; i < (N + 31) / 32; ++i)
            ((unsigned int*)r.mBits)[i] = ~((const unsigned int*)mBits)[i];
        return r;
    }
    static int GetNumWords() { return (N + 31) / 32; }
    unsigned int GetWord(int idx) const { return ((unsigned int*)mBits)[idx]; }

    class iterator {
    public:
        BitSet<N>* m_src;        // +0x00
        int m_word_idx;          // +0x04
        unsigned int m_cur_val;  // +0x08
        unsigned int m_cur_word; // +0x0C

        iterator() : m_src(nullptr), m_word_idx(0), m_cur_val(0), m_cur_word(0) {}
        iterator(const BitSet<N>& src)
        {
            m_src = (BitSet<N>*)&src;
            m_cur_word = ((const unsigned int*)src.mBits)[0];
            m_word_idx = 0;
            m_cur_val = (unsigned int)-1;
            operator++();
        }
        iterator& operator++()
        {
            while (m_word_idx < GetNumWords())
            {
                if (m_cur_word != 0)
                {
                    unsigned long idx;
                    _BitScanForward(&idx, m_cur_word);
                    m_cur_val = m_word_idx * 32 + (int)idx;
                    m_cur_word &= m_cur_word - 1;
                    return *this;
                }
                ++m_word_idx;
                if (m_word_idx < GetNumWords())
                    m_cur_word = ((const unsigned int*)m_src->mBits)[m_word_idx];
            }
            m_cur_val = (unsigned int)-1;
            m_word_idx = -1;
            return *this;
        }
    };
    iterator begin() const { return iterator(*this); }
};

template <int INDEX_BITS, int KEY_BITS>
class SizedHandle {
public:
    unsigned int mVal;  // +0x00
    SizedHandle() : mVal(0) {}
    SizedHandle(int index, int key)
    {
        mVal = (unsigned int)index | ((unsigned int)key << INDEX_BITS);
    }
    SizedHandle(Handle h) { mVal = h.mVal; }
    int GetIndex() const { return (int)(mVal & ((1 << INDEX_BITS) - 1)); }
    int GetKey() const { return (int)(mVal >> INDEX_BITS); }
};

template <typename T, int CAPACITY, typename H>
class HandleDb {
public:
    struct DbElement {
        T*  mObject;  // +0x00
        int mKey;     // +0x04
        DbElement() : mObject(nullptr), mKey(1) {}
        T* GetObject() const { return mObject; }
        void SetObject(T* obj) { mObject = obj; }
        int GetKey() const { return mKey; }
        void Release() { ++mKey; mObject = nullptr; }
    };
    BitSet<1344> mFreeIndices;       // +0x00 (168 bytes)
    DbElement mElements[0x540];      // +0xA8
    void (*mDebugCallback)(int, T*); // +0x2AA8

    HandleDb();
    BitSet<1344> GetAllocatedIndices() const;
    void ReleaseHandle(Handle h);
    T* DereferenceHandle(Handle h) const;
    T* GetObject(int idx) const;
    void BindObjectToHandle(Handle handle, T* obj);
    void RegisterDebugCallback(void (*cb)(int, T*)) { mDebugCallback = cb; }
    void Dump();  // ?Dump@?$HandleDb@VEntity@@$0FEA@V?$SizedHandle@$0M@$0BE@@@@@QAEXXZ (g.o 0x4B3130)
    SizedHandle<12, 20> AllocateHandle();  // ?AllocateHandle@?$HandleDb@VEntity@@$0FEA@V?$SizedHandle@$0M@$0BE@@@@@QAE?AV?$SizedHandle@$0M@$0BE@@@XZ (g.o 0x4B3D70)
};

template <typename T, int CAPACITY, typename H>
HandleDb<T, CAPACITY, H>::HandleDb()
{
    mFreeIndices.Clear();
    for (int i = 0; i < CAPACITY; ++i)
    {
        mElements[i].mObject = nullptr;
        mElements[i].mKey = 1;
    }
    mDebugCallback = nullptr;
    for (int i = 0; i < CAPACITY; ++i)
        mFreeIndices.Add(i);
}
template <typename T, int CAPACITY, typename H>
BitSet<1344> HandleDb<T, CAPACITY, H>::GetAllocatedIndices() const
{
    return ~mFreeIndices;
}
template <typename T, int CAPACITY, typename H>
void HandleDb<T, CAPACITY, H>::ReleaseHandle(Handle h)
{
    if (h.mVal != 0)
    {
        unsigned int v3 = h.mVal & 0xFFF;
        if (v3 >= (unsigned int)CAPACITY)
        {
            if (!AeAssert::IsIgnored() && AeAssert::Warning("freeing invalid handle"))
                __debugbreak();
        }
        else
        {
            if (mElements[v3].mKey == (int)(h.mVal >> 12))
            {
                mFreeIndices.Add((int)v3);
                mElements[v3].mObject = nullptr;
                ++mElements[v3].mKey;
                return;
            }
            if (!AeAssert::IsIgnored() && AeAssert::Warning("freeing invalid handle"))
                __debugbreak();
        }
    }
}
template <typename T, int CAPACITY, typename H>
T* HandleDb<T, CAPACITY, H>::DereferenceHandle(Handle h) const
{
    unsigned int v2 = h.mVal & 0xFFF;
    if (v2 < (unsigned int)CAPACITY && (h.mVal >> 12) == (unsigned int)mElements[v2].mKey)
        return mElements[v2].mObject;
    return nullptr;
}
template <typename T, int CAPACITY, typename H>
T* HandleDb<T, CAPACITY, H>::GetObject(int idx) const
{
    if ((unsigned int)idx >= (unsigned int)CAPACITY)
    {
        if (!AeAssert::IsIgnored() && AeAssert::Assert("index out of bounds"))
            __debugbreak();
    }
    return mElements[idx].mObject;
}
template <typename T, int CAPACITY, typename H>
void HandleDb<T, CAPACITY, H>::BindObjectToHandle(Handle handle, T* obj)
{
    unsigned int v3 = handle.mVal & 0xFFF;
    if (v3 < (unsigned int)CAPACITY)
    {
        if (mElements[v3].mKey != (int)(handle.mVal >> 12))
        {
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("handle was not allocated for this object"))
                __debugbreak();
        }
        mElements[v3].mObject = obj;
    }
}
template <typename T, int CAPACITY, typename H>
void HandleDb<T, CAPACITY, H>::Dump()
{
    if (mDebugCallback != nullptr)
    {
        tlPrintf("handle db contents:\n");
        BitSet<1344> allocatedIndices = ~mFreeIndices;
        BitSet<1344>::iterator it = allocatedIndices.begin();
        for (;;)
        {
            ++it;
            if (it.m_cur_val == (unsigned int)-1 && it.m_word_idx == -1)
                break;
            mDebugCallback((int)it.m_cur_val, mElements[it.m_cur_val].mObject);
        }
    }
    if (!AeAssert::IsIgnored()
        && AeAssert::Error("out of handles! - Tell MikeA (MAX_GENTITIES)"))
        __debugbreak();
}
template <typename T, int CAPACITY, typename H>
SizedHandle<12, 20> HandleDb<T, CAPACITY, H>::AllocateHandle()
{
    BitSet<1344>::iterator it = mFreeIndices.begin();
    int m_cur_val = (int)it.m_cur_val;
    if ((unsigned int)it.m_cur_val >= (unsigned int)CAPACITY)
    {
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("index out of bounds!!! ILLEGAL array access!"))
            __debugbreak();
    }
    mFreeIndices.Rmv(m_cur_val);
    if (m_cur_val == -1)
        Dump();
    return SizedHandle<12, 20>(m_cur_val, mElements[m_cur_val].mKey);
}
template class SizedHandle<12, 20>;
template class HandleDb<Entity, 1344, SizedHandle<12, 20>>;
template class HandleDb<DObj, 1344, SizedHandle<12, 20>>;

// EntityHandleDb ctor (g.o 0x4B3D50)
EntityHandleDb::EntityHandleDb()
{
    unsigned int* bits = (unsigned int*)_pad;
    for (int i = 0; i < 42; ++i)
        bits[i] = 0;
    for (int i = 0; i < 0x540; ++i)
    {
        mElements[i].mObject = nullptr;
        mElements[i].mKey = 1;
    }
    mDebugCallback = nullptr;
    for (int i = 0; i < 0x540; ++i)
        bits[i >> 5] |= 1u << (i & 0x1F);
    mActiveList.m_size = 0;
}

// InteractionController::FreeInteraction (g.o 0x4B00A0)
void InteractionController::FreeInteraction()
{
    InteractionController_EndInteraction(this, mCurState != nullptr);
    InteractionController_ClearQueue(this);
}

// rb_vehicle flag getters (g.o 0x4B0540-0x4B0560)
bool rb_vehicle::is_physics_paused() const
{
    return (m_flags & 1) != 0;
}
bool rb_vehicle::is_attached_path() const
{
    return ((m_flags >> 8) & 1) != 0;
}
bool rb_vehicle::is_driving_path() const
{
    return (m_flags & 0x200) != 0;
}

local_physic_s::local_physic_s()
{
    groundTrace.mEntity.mHandle.mVal = 0;
    groundTrace.partName.mHash = 0;
    hasGround = 0;
    onGround = 0;
}
// ae_formatted_string ctor (g.o 0x4B0FB0) - declared in core/ae_fixed_string.h
template class ae_formatted_string<256, unsigned short>;

// level_locals_t::__unnamed ctor (g.o 0x4B0570)
level_locals_t::__unnamed::__unnamed()
{
    v = 0;
}

// scr_data_t::__unnamed ctor/dtor (g.o 0x4B05A0 / 0x4B08A0)
// Layout: 28 Broc::strings (+8..+332), 91 scr_animscript_t (+336..+1428),
// 5 Broc::strings (+1436..+1456).
scr_data_t::__unnamed::__unnamed()
{
    for (int i = 0; i < 28; ++i)
        new (data + 8 + i * 4) Broc::string((Broc::string::Block*)nullptr);
    for (int i = 0; i < 91; ++i)
        new (data + 336 + i * 12) scr_animscript_t();
    for (int i = 0; i < 5; ++i)
        new (data + 1436 + i * 4) Broc::string((Broc::string::Block*)nullptr);
}
scr_data_t::__unnamed::~__unnamed()
{
    for (int i = 4; i >= 0; --i)
        ((Broc::string*)(data + 1436 + i * 4))->~string();
    for (int i = 90; i >= 0; --i)
        ((scr_animscript_t*)(data + 336 + i * 12))->~scr_animscript_t();
    for (int i = 27; i >= 0; --i)
        ((Broc::string*)(data + 8 + i * 4))->~string();
}
static scr_data_t::__unnamed s_force_scr_unnamed_emit;

// scr_data_t dtor (g.o 0x4B23C0)
scr_data_t::~scr_data_t()
{
    // binary: destroys debris.debug string + anim __unnamed
}

// level_locals_t::Clear (g.o 0x4AFD40)
void level_locals_t::Clear()
{
    clients = nullptr;
    num_entities = 0;
    sentients = nullptr;
    vehicles = nullptr;
    turrets = nullptr;
    for (int i = 0; i < 32; ++i)
        actors[i] = nullptr;
    maxclients = 0;
    framenum = 0;
    time = 0;
    previousTime = 0;
    snapTime = 0;
    numActorCorpses = 0;
    spawning = 0;
    numSpawnVars = 0;
    numSpawnVarChars = 0;
    memset(spawnVarChars, 0, sizeof(spawnVarChars));
    reloadDelayTime = 0;
    iNextObjectiveTime = 0;
    changelevel = 0;
    endgame = 0;
    bMissionSuccess = 0;
    bMissionFailed = 0;
    strMissionFailedReason = "";
    savepersist = 0;
    exitTime = 0;
    memset(nextMap, 0, sizeof(nextMap));
    fFogOpaqueDist = 0.0f;
    fFogOpaqueDistSqrd = 0.0f;
    remapCount = 0;
    iSearchFrame = 0;
    loading = 0;
    actorPredictDepth = 0;
    bounds_width = 0.0f;
    bounds_height_standing = 0.0f;
    viewheight_standing = 0.0f;
    viewheight_crouched = 0.0f;
    viewheight_prone = 0.0f;
    MissleOnlyActiveForTime = 0.0f;
    MaxVehicles = 0;
    bRegisterItems = 0;
    bDrawCompassFriendlies = 0;
    bPlayerIgnoreRadiusDamage = 0;
    bPlayerIgnoreRadiusDamageLatched = 0;
    pathsInvalid = false;
    pathsInited = false;
    pathsConnected = false;
    initializing = 0;
    newAssetLoaded = 0;
    memset(cachedTagMat, 0, sizeof(cachedTagMat));
    for (int i = 0; i < 256; ++i)
        triggerList[i].Clear();
    triggerListSize = 0;
    delayFreeAnimTreeCount = 0;
    memset(delayFreeAnimTree, 0, sizeof(delayFreeAnimTree));
    delayClearAnimTreeCount = 0;
    memset(delayClearAnimTree, 0, sizeof(delayClearAnimTree));
}

// PlayerState::Clear (g.o 0x4AF3C0)
void PlayerState::Clear(bool clearWeapons)
{
    origin.v = _mm_setzero_ps();
    leanf = 0.0f;
    velocity.v = _mm_setzero_ps();
    commandTime = 0;
    pm_type = 0;
    bobCycle = 0;
    pm_flags = 0;
    pm_time = 0;
    weaponTime = 0;
    weaponDelay = 0;
    grenadeTimeLeft = 0;
    iFoliageSoundTime = 0;
    iFatigueSoundTime = 0;
    gravity = 0;
    speed = 0;
    delta_angles[0] = 0;
    delta_angles[1] = 0;
    delta_angles[2] = 0;
    mGroundEntity.mHandle.mVal = 0;
    vLadderVec[0] = 0.0f;
    vLadderVec[1] = 0.0f;
    jumpTime = 0;
    fJumpOriginZ = 0.0f;
    legsAnim = 0;
    legsYaw = 0.0f;
    torsoAnim = 0;
    spotTime = 0;
    respawnUntilTime = 0;
    mLastSpotter.mHandle.mVal = 0;
    mTarget.mHandle.mVal = 0;
    mTargetTime = 0;
    mKiller.mHandle.mVal = 0;
    movementDir = 0;
    mClient.mHandle.mVal = 0;
    weaponstate = 0;
    fWeaponPosFrac = 0.0f;
    reloadFromEmpty = false;
    queuedReloadSound.mVal = 0;
    queuedReloadSoundPlayStarted = false;
    queuedReloadTimer = 0;
    reloadSoundPrequeueAttempted = false;
    viewmodel.mValue = nullptr;
    viewmodel.mPakId = PAK_ID_INVALID;
    viewangles[0] = 0.0f;
    viewangles[1] = 0.0f;
    viewHeightTarget = 0;
    viewHeightCurrent = 0.0f;
    viewHeightLerpTime = 0;
    viewHeightLerpTarget = 0;
    viewHeightLerpDown = 0;
    viewHeightLerpPosAdj = 0.0f;
    eFlags = 0;
    event.Clear();
    stats[0] = 0;
    stats[1] = 0;
    stats[2] = 0;
    stats[3] = 0;
    if (clearWeapons)
    {
        weapon = 0;
        lastWeapon = 0;
        for (int i = 0; i < 92; ++i)
        {
            ammo[i] = 0;
            ammoclip[i] = 0;
        }
        weapons[0] = 0;
        weapons[1] = 0;
        for (int j = 0; j < 10; ++j)
            weaponslots[j] = 0;
        for (int j = 0; j < 2; ++j)
            weaponrechamber[j] = 0;
    }
    mins[0] = 0.0f;
    mins[1] = 0.0f;
    mins[2] = 0.0f;
    maxs[0] = 0.0f;
    maxs[1] = 0.0f;
    maxs[2] = 0.0f;
    proneViewHeight = 0;
    crouchViewHeight = 0;
    deadViewHeight = 0;
    walkSpeedScale = 0.0f;
    runSpeedScale = 0.0f;
    sprintSpeedScale = 0.0f;
    proneSpeedScale = 0.0f;
    crouchSpeedScale = 0.0f;
    strafeSpeedScale = 0.0f;
    backSpeedScale = 0.0f;
    leanSpeedScale = 0.0f;
    proneDirection = 0.0f;
    proneDirectionPitch = 0.0f;
    proneTorsoPitch = 0.0f;
    fatigueScale = 0.0f;
    lastSprintTime = 0;
    viewlocked = 0;
    mViewLockedEntity.mHandle.mVal = 0;
    friction = 0.0f;
    serverCursorHint = 0;
    serverCursorHintVal = 0;
    serverCursorHintString = 0;
    serverCursorHintTrace.Clear();
    iCompassFriendInfo = 0;
    iCompassTankInfo = 0;
    fTorsoHeight = 0.0f;
    fTorsoPitch = 0.0f;
    fWaistPitch = 0.0f;
    vehPos = 0;
    vehType = 0;
    vehSubType = 0;
    weapAnim = 0;
    aimSpreadScale = 0.0f;
    shellshockIndex = 0;
    shellshockTime = 0;
    shellshockDuration = 0;
    mTimeSinceDamage = 0.0f;
    mHealthDelta = 0.0f;
    ctf_has_flag = 0;
    spectatorClient = 0;
    for (int i = 0; i < 16; ++i)
        mDamageFromPlayers[i] = 0;
    mLastFireWeaponTime = -1;
    mLastFireWeapon = 0;
    mAmmoDropTime = -1;
    mMeleeAssistTarget.mHandle.mVal = 0;
    mMeleeAssistSpeed = 0;
    mFlags = 0;
}

// clientPersistent_t::Clear (g.o 0x4AF940)
void clientPersistent_t::Clear()
{
    connected = CON_DISCONNECTED;
    cmd.Clear();
    oldcmd.Clear();
    pmoveFixed = 0;
    maxHealth = 0;
    healthTaskHandle.mHandle.mVal = 0;
    playerClass = -1;
    playerState = 0;
    rank = 0;
}

// clientPersistent_t::GetStat (g.o 0x5E9C80)
int clientPersistent_t::GetStat(int stat)
{
    if (stat > 0x1C) {
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid stat index."))
            __debugbreak();
    }
    return mStats[0][stat] + mStats[1][stat] + mStats[2][stat]
         + mStats[3][stat] + mStats[4][stat] + mStats[5][stat]
         + mStats[6][stat];
}

// clientPersistent_t::GetTotalScore (scr.o 0x5E9D40)
int clientPersistent_t::GetTotalScore()
{
    int totalScore = mBaseScore;
    for (int i = 7; i != 0; --i)
        totalScore += PlayerStats::TotalScoreForStats(mStats[7 - i]);
    return totalScore;
}


// DbLinkedHandle<EntityHandleDb,Entity> deref (g.o 0x4B2670 / 0x4B26B0)
template <>
Entity* DbLinkedHandle<EntityHandleDb, Entity>::operator*() const
{
    unsigned int mVal = mHandle.mVal;
    unsigned int v2 = mVal & 0xFFF;
    if (v2 < 0x540 && (mVal >> 12) == (unsigned int)EntityHandleDb::sInst.mElements[v2].mKey)
        return EntityHandleDb::sInst.mElements[v2].mObject;
    return nullptr;
}
template <>
Entity* DbLinkedHandle<EntityHandleDb, Entity>::operator->() const
{
    unsigned int mVal = mHandle.mVal;
    unsigned int v2 = mVal & 0xFFF;
    if (v2 < 0x540 && (mVal >> 12) == (unsigned int)EntityHandleDb::sInst.mElements[v2].mKey)
        return EntityHandleDb::sInst.mElements[v2].mObject;
    return nullptr;
}

// proximity_data_t dtor (g.o 0x4B2290)
proximity_data_t::~proximity_data_t()
{
}

// ConfigString::operator[] (g.o 0x4B22A0)
const char* ConfigString::operator[](const char* key) const
{
    InplaceString* v2 = mStringMap.Find<const char*>(key);
    if (v2 != nullptr)
        return v2->mStr;
    return nullptr;
}

// vehicle_backup_s ctor/dtor (g.o 0x4B2430 / 0x4B2460)
class vehicle_backup_s {
public:
    uint8_t data[0xB8 + 0xB0];  // vehicle_pathpos_t + vehicle_physic_t
    vehicle_backup_s();
    ~vehicle_backup_s();
};
vehicle_backup_s::vehicle_backup_s()
{
    vehicle_node_t* nodes = (vehicle_node_t*)&((vehicle_pathpos_t*)data)->switchNode;
    for (int i = 0; i < 2; ++i)
        new (&nodes[i]) vehicle_node_t();
}
vehicle_backup_s::~vehicle_backup_s()
{
    vehicle_node_t* nodes = (vehicle_node_t*)&((vehicle_pathpos_t*)data)->switchNode;
    for (int i = 1; i >= 0; --i)
        nodes[i].~vehicle_node_t();
}

// AeThreadManager::AddNotify (g.o 0x4B2170) - mPendingNotifys at +0x24
namespace {
struct EntityNotifyDListNode {
    EntityNotifyDListNode* m_next;  // +0x00
    EntityNotifyDListNode* m_prev;  // +0x04
};
struct PendingNotifyList {
    int m_size;                    // +0x00
    EntityNotifyDListNode* m_head;
    EntityNotifyDListNode* m_end;
    EntityNotifyDListNode* m_tail;
};
}
void AeThreadManager::AddNotify(EntityNotify* notify)
{
    PendingNotifyList* list = (PendingNotifyList*)((char*)this + 0x24);
    // EntityNotify's first member is its dlist node (next +0, prev +4).
    struct RawNode { void* next; void* prev; };
    RawNode* node = (RawNode*)notify;
    EntityNotifyDListNode* m_tail = list->m_tail;
    node->next = list->m_end;
    node->prev = m_tail;
    m_tail->m_next = (EntityNotifyDListNode*)node;
    list->m_tail = (EntityNotifyDListNode*)node;
    ++list->m_size;
}
