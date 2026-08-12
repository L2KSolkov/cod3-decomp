// ============================================================================
// g_physics.cpp - physics.o game integration (phys_xboxr physics.o family)
// Ported smallest-first from IDA (release map offset + 0x40C000 = VA).
// ============================================================================

#include "physics/physics_system.h"
#include <intrin.h>
#include <math.h>

extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);
extern const char* const defaultFileName;

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

class PakManager {
public:
    static PakManager* sInst;
    void* CrazyTempMemBorrow(unsigned int align, unsigned int size);
    void  CrazyTempMemGiveBack(void* ptr);
};

void merge_spheres(const math::Position3& c1, float r1, math::Position3* c2,
                   float* r2);
class Color;
class DebugRender {
public:
    static void RenderAxis(const math::Mat43& mat, float length, float width);
};
struct rb_vehicle {
    uint8_t _pad[0x388];
    void*   m_vci;  // +0x388
};
struct rb_extra_info {
    void* m_rb;              // +0x00
    uint8_t _pad04[0x40 - 0x04];
    void* m_gjk_geom_list;   // +0x40
    uint8_t _pad44[0x54 - 0x44];
    rb_vehicle* m_rb_vehicle;  // +0x54
    void collision_epilog();
};
class rigid_body;
void render_single_rigid_body(rigid_body* const rb);

// ea: 0x6F2FD0
void rb_extra_info::collision_epilog()
{
    m_gjk_geom_list = nullptr;
    if (m_rb_vehicle != nullptr)
        m_rb_vehicle->m_vci = nullptr;
}

// ea: 0x6F4600
void render_single_rigid_body(class rigid_body* const rb)
{
    if (rb != nullptr)
    {
        if ((~(rb->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         79, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        DebugRender::RenderAxis(rb->m_mat, 6.0f, 0.050000001f);
    }
}

// phys_collision_allocater (physics.o RBCollision.cpp; private members)
class phys_collision_allocater {
public:
    phys_memory_heap m_list_memory_buffer[5];  // +0x00 (0x10 stride)
    int              m_num_buffers;            // +0x50
    int              m_high_buffer_count;      // +0x54
    bool             m_out_of_memory;          // +0x58

private:
    void allocate_buffer();  // ?allocate_buffer@phys_collision_allocater@@AAEXXZ
    void free_buffers();     // ?free_buffers@phys_collision_allocater@@AAEXXZ
};

// ea: 0x6F2FF0
void phys_collision_allocater::allocate_buffer()
{
    if (m_num_buffers >= 5
        && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 69,
                     "m_num_buffers < MAX_BUFFERS", defaultFileName))
        __debugbreak();
    void* v2 = PakManager::sInst->CrazyTempMemBorrow(4u, 0x5000u);
    if (v2 != nullptr)
    {
        int m_num_buffers = this->m_num_buffers;
        phys_memory_heap* v4 = &m_list_memory_buffer[m_num_buffers];
        this->m_num_buffers = m_num_buffers + 1;
        v4->set_buffer((char*)v2, 0x5000, 4);
        v4->m_user_start = v4->m_buffer_cur;
        int v5 = this->m_num_buffers;
        if (v5 > m_high_buffer_count)
            m_high_buffer_count = v5;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBCollision.cpp";
        AeAssert::gCurrentLine = 73;
        AeAssert::gCurrentExpr = "ptr";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Ran out of physics memory"))
            __debugbreak();
        m_out_of_memory = true;
    }
}

// ea: 0x6F30B0
void phys_collision_allocater::free_buffers()
{
    int v2 = 1;
    if (m_num_buffers > 1)
    {
        phys_memory_heap* v3 = &m_list_memory_buffer[1];
        do
        {
            PakManager::sInst->CrazyTempMemGiveBack(v3->m_buffer_start);
            ++v2;
            ++v3;
        } while (v2 < m_num_buffers);
    }
}

// Binary parameter type for GetPhysBoneID (mangles as W4hitLocation_t@@; the
// COD2-derived enum tag, distinct from Broc's EHitLocation which lives in
// broc_types.h and shares enumerator names).
enum hitLocation_t {
    HITLOC_NONE = 0,
    HITLOC_HELMET,
    HITLOC_HEAD,
    HITLOC_NECK,
    HITLOC_TORSO_UPR,
    HITLOC_TORSO_LWR,
    HITLOC_R_ARM_UPR,
    HITLOC_L_ARM_UPR,
    HITLOC_R_ARM_LWR,
    HITLOC_L_ARM_LWR,
    HITLOC_R_HAND,
    HITLOC_L_HAND,
    HITLOC_R_LEG_UPR,
    HITLOC_L_LEG_UPR,
    HITLOC_R_LEG_LWR,
    HITLOC_L_LEG_LWR,
    HITLOC_R_FOOT,
    HITLOC_L_FOOT,
    HITLOC_GUN,
    HITLOC_NUM = 0x13,
};

// ea: 0x6F2F20
void PhysShutdown()
{
    phys_sys::phys_shutdown();
}

// ea: 0x6F2F30
int GetPhysBoneID(hitLocation_t id)
{
    switch (id)
    {
    case HITLOC_HELMET:
    case HITLOC_HEAD:
    case HITLOC_NECK:
        return 1;
    case HITLOC_R_ARM_UPR:
        return 4;
    case HITLOC_L_ARM_UPR:
        return 2;
    case HITLOC_R_ARM_LWR:
    case HITLOC_R_HAND:
    case HITLOC_GUN:
        return 5;
    case HITLOC_L_ARM_LWR:
    case HITLOC_L_HAND:
        return 3;
    case HITLOC_R_LEG_UPR:
        return 8;
    case HITLOC_L_LEG_UPR:
        return 6;
    case HITLOC_R_LEG_LWR:
    case HITLOC_R_FOOT:
        return 9;
    case HITLOC_L_LEG_LWR:
    case HITLOC_L_FOOT:
        return 7;
    default:
        return 0;
    }
}

// ea: 0x6F5F00 (PAV mangling: class tag)
void path_constraint_destroy(class rigid_body_constraint_custom_path* vpc)
{
    if (vpc->m_urb == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp", 232,
                     "vpc->m_urb", defaultFileName))
        __debugbreak();
    if (vpc->b2 != vpc->m_urb
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp", 233,
                     "vpc->get_b2() == vpc->m_urb", defaultFileName))
        __debugbreak();
    phys_sys::destroy(vpc->m_urb);
}

// ea: 0x6FBD50 (inline; phys_array_base.inc index asserts retained)
struct rb_collision_sphere {
    math::Position3 m_center_loc;  // +0x00
    float           m_radius;      // +0x10
};
struct phys_sphere_array {
    rb_collision_sphere* m_slot_array;  // +0x00
    int                 m_alloc_count;  // +0x04
};
struct rigid_body_sphere_list {
    uint8_t        _pad0[0x40];
    rb_collision_sphere* m_slot_array;  // +0x40
    int            m_alloc_count;       // +0x44
    uint8_t        _pad48[0x50 - 0x48];
    math::Position3 m_bounding_sphere_center_loc;  // +0x50
    float          m_bounding_sphere_radius;       // +0x60

private:
    void calc_bounding_sphere();  // ?calc_bounding_sphere@rigid_body_sphere_list@@AAEXXZ
};
void rigid_body_sphere_list::calc_bounding_sphere()
{
    if (m_alloc_count <= 0
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                     108, "i >= 0 && i < m_alloc_count", defaultFileName))
        __debugbreak();
    m_bounding_sphere_center_loc.v = m_slot_array->m_center_loc.v;
    if (m_alloc_count <= 0
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                     108, "i >= 0 && i < m_alloc_count", defaultFileName))
        __debugbreak();
    m_bounding_sphere_radius = m_slot_array->m_radius;
    int sphere_count = m_alloc_count;
    for (int i = 1; i < sphere_count; ++i)
    {
        if ((i < 0 || i >= m_alloc_count)
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                         108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        merge_spheres(m_slot_array[i].m_center_loc,
                      m_slot_array[i].m_radius,
                      &m_bounding_sphere_center_loc,
                      &m_bounding_sphere_radius);
    }
}

// ea: 0x716760 (physics.o; RBRagdollCollision.cpp)
void merge_spheres(const math::Position3& c1ref, float r1, math::Position3* c2,
                   float* r2)
{
    const math::Position3* c1 = &c1ref;
    __m128 v4 = _mm_sub_ps(c1->v, c2->v);
    __m128 v5 = _mm_mul_ps(v4, v4);
    float v6 = r1;
    float v12 = v5.m128_f32[0]
                + (_mm_shuffle_ps(v5, v5, 85).m128_f32[0]
                   + _mm_shuffle_ps(v5, v5, 170).m128_f32[0]);
    float* v7 = r2;
    __m128 v8 = _mm_castsi128_ps(_mm_set1_epi32((int)r1));
    v8.m128_f32[0] = r1 - *r2;
    if ((v8.m128_f32[0] * v8.m128_f32[0]) < v12)
    {
        float v13 = sqrtf(v12);
        if (v13 <= 0.000099999997f)
        {
            if (!_tlAssert("c:\\cod\\code\\game\\RBRagdollCollision.cpp", 191,
                           "ndelta_c > 0.0001f", defaultFileName))
                __debugbreak();
        }
        v8.m128_f32[0] = v8.m128_f32[0] / v13;
        __m128 v10 = _mm_mul_ps(v4, _mm_shuffle_ps(v8, v8, 0));
        *v7 = ((*v7 + v13) + v6) * 0.5f;
        __m128 v11 = _mm_add_ps(_mm_add_ps(c1->v, c2->v), v10);
        __m128 half = _mm_set1_ps(0.5f);
        c2->v = _mm_mul_ps(v11, half);
    }
    else if (r1 > *r2)
    {
        c2->v = c1->v;
        *r2 = r1;
    }
}
