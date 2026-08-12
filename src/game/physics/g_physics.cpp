// ============================================================================
// g_physics.cpp - physics.o game integration (phys_xboxr physics.o family)
// Ported smallest-first from IDA (release map offset + 0x40C000 = VA).
// ============================================================================

#include "physics/physics_system.h"
#include <intrin.h>
#include <math.h>
#include <string.h>

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

namespace nuge {
void calc_velocities(const math::Mat43& mat0, const math::Mat43& mat1,
                     const math::Dir3& center_offset_loc, float delta_t,
                     math::Dir3* t_vel, math::Dir3* a_vel);
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
public:
    void set_buffer(void* const start, int size, int alignment);  // ?set_buffer@phys_collision_allocater@@QAEXQAXHH@Z
    void capture_user_start();  // ?capture_user_start@phys_collision_allocater@@QAEXXZ
    void reset_to_user_start(); // ?reset_to_user_start@phys_collision_allocater@@QAEXXZ
    int  get_warning_level();   // ?get_warning_level@phys_collision_allocater@@QAEHXZ
    void reset_warning_level(); // ?reset_warning_level@phys_collision_allocater@@QAEXXZ
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

// ea: 0x6F1E20
void phys_collision_allocater::set_buffer(void* const start, int size,
                                          int alignment)
{
    m_list_memory_buffer[0].set_buffer((char*)start, size, alignment);
    m_num_buffers = 1;
    m_high_buffer_count = 1;
}

// ea: 0x6F1E60
void phys_collision_allocater::capture_user_start()
{
    for (int i = 0; i < m_num_buffers; ++i)
        m_list_memory_buffer[i].m_user_start =
            m_list_memory_buffer[i].m_buffer_cur;
}

// ea: 0x6F1E90
void phys_collision_allocater::reset_to_user_start()
{
    for (int i = 0; i < m_num_buffers; ++i)
        m_list_memory_buffer[i].m_buffer_cur =
            m_list_memory_buffer[i].m_user_start;
}

// ea: 0x6F1DF0
int phys_collision_allocater::get_warning_level()
{
    if (m_out_of_memory)
        return 2;
    return m_high_buffer_count > 1;
}

// ea: 0x6F1E10
void phys_collision_allocater::reset_warning_level()
{
    m_out_of_memory = false;
    m_high_buffer_count = 0;
}

// ?g_ragdoll_mass_scale@@3MA (physics.o data @ 0xE01E50)
float g_ragdoll_mass_scale = 1.0f;

// ea: 0x6F4570
double get_joint_damp_k(int rb_bone_id)
{
    float v1 = g_ragdoll_mass_scale * 25.0f;
    switch (rb_bone_id)
    {
    case 3:
    case 5:
        return v1 * 0.5;
    case 7:
    case 9:
        return v1 * 0.80000001f;
    default:
        return v1;
    }
}

// rb_collision_capsule (physics.o; inline getters 0xAE1810..0xAE1830)
struct rb_collision_capsule {
    math::Position3 m_p1_loc;  // +0x00
    math::Position3 m_p2_loc;  // +0x10
    float           m_r;       // +0x20

    void set(const math::Position3& p1_loc, const math::Position3& p2_loc,
             float r);  // ?set@rb_collision_capsule@@QAEXABVPosition3@math@@0M@Z
};

// ea: 0x6F4650
void rb_collision_capsule::set(const math::Position3& p1_loc,
                               const math::Position3& p2_loc, float r)
{
    m_p1_loc.v = p1_loc.v;
    m_p2_loc.v = p2_loc.v;
    m_r = r;
}

// ragdoll_collision_callback (physics.o)
class Entity;
struct ragdoll_collision_callback {
    Entity* m_owner;  // +0x00

    void set(Entity* const owner);  // ?set@ragdoll_collision_callback@@QAEXQAVEntity@@@Z
};

// ea: 0x6F46A0
void ragdoll_collision_callback::set(Entity* const owner)
{
    m_owner = owner;
}

// phys_anim_bone_array (physics.o; static helpers)
class Entity {
public:
    class DObj {
    public:
        int numBones;  // +0x00 (minimal view for copy/write_skeleton)
        const math::Mat43& GetMat(int boneIndex);
        int GetBoneIndex(const char* name);  // ?GetBoneIndex@DObj@@QBEHPBD@Z
    };
    DObj* mDObj;  // +0x00
};
const math::Mat43& Entity::DObj::GetMat(int boneIndex)
{
    (void)boneIndex;
    static math::Mat43 zero = {};
    return zero;
}
int Entity::DObj::GetBoneIndex(const char* name)
{
    (void)name;
    return -1;
}

struct phys_anim_bone_array {
    static void copy_skeleton(Entity* owner, math::Mat43* const skeleton_pose);
    static void write_skeleton(Entity* owner, math::Mat43* const skeleton_pose);
};

// ea: 0x6F4120
void phys_anim_bone_array::copy_skeleton(Entity* owner,
                                         math::Mat43* const skeleton_pose)
{
    int bone_count = owner->mDObj->numBones;
    for (int i = 0; i < bone_count; ++i)
        memcpy(&skeleton_pose[i], &owner->mDObj->GetMat(i),
               sizeof(math::Mat43));
}

// ea: 0x6F41F0
void phys_anim_bone_array::write_skeleton(Entity* owner,
                                          math::Mat43* const skeleton_pose)
{
    int bone_count = owner->mDObj->numBones;
    for (int i = 0; i < bone_count; ++i)
        memcpy(&const_cast<math::Mat43&>(owner->mDObj->GetMat(i)),
               &skeleton_pose[i], sizeof(math::Mat43));
}

// biped_phys_info (physics.o RBRagdoll.cpp); ctor only - the full layout is
// mapped incrementally as the family is ported.
struct biped_phys_info {
    unsigned int m_render_flags;  // +0x00 (Bitmask mMask)
    Entity*      m_owner;         // +0x04
    void*        m_bp_sys;        // +0x08 (biped_system*)
    uint8_t      _pad0C[0x510 - 0x0C];
    math::Position3 m_cur_angles;    // +0x510
    math::Position3 m_cur_origin;    // +0x520
    math::Position3 m_last_angles;   // +0x530
    math::Position3 m_last_origin;   // +0x540
    int16_t      m_bone[10];         // +0x550
    float        m_delta_t;          // +0x564

    biped_phys_info();  // ??0biped_phys_info@@QAE@XZ
    void get_cur_vel(int rb_id, const math::Position3& com,
                     math::Dir3* cur_tvel, math::Dir3* cur_avel);
};

// ea: 0x6F71C0
biped_phys_info::biped_phys_info()
{
    m_render_flags = 0;
    m_owner = nullptr;
    m_bp_sys = nullptr;
}

// USER_BONE_ID_* globals (physics.o data @ 0xE01EA8..0xE01EDC)
int USER_BONE_ID_PELVIS = -1;
int USER_BONE_ID_HEAD = -1;
int USER_BONE_ID_LEFT_UPPERARM = -1;
int USER_BONE_ID_LEFT_FOREARM = -1;
int USER_BONE_ID_LEFT_HAND = -1;
int USER_BONE_ID_RIGHT_UPPERARM = -1;
int USER_BONE_ID_RIGHT_FOREARM = -1;
int USER_BONE_ID_RIGHT_HAND = -1;
int USER_BONE_ID_LEFT_THIGH = -1;
int USER_BONE_ID_LEFT_CALF = -1;
int USER_BONE_ID_LEFT_FOOT = -1;
int USER_BONE_ID_RIGHT_THIGH = -1;
int USER_BONE_ID_RIGHT_CALF = -1;
int USER_BONE_ID_RIGHT_FOOT = -1;

// ea: 0x6F43F0
void setup_user_bone_ids(Entity* owner)
{
    Entity::DObj* mDObj = owner->mDObj;
    USER_BONE_ID_PELVIS = mDObj->GetBoneIndex("Bip01 Pelvis");
    USER_BONE_ID_HEAD = mDObj->GetBoneIndex("Bip01 Head");
    USER_BONE_ID_LEFT_UPPERARM = mDObj->GetBoneIndex("Bip01 L UpperArm");
    USER_BONE_ID_LEFT_FOREARM = mDObj->GetBoneIndex("Bip01 L Forearm");
    USER_BONE_ID_LEFT_HAND = mDObj->GetBoneIndex("Bip01 L Hand");
    USER_BONE_ID_RIGHT_UPPERARM = mDObj->GetBoneIndex("Bip01 R UpperArm");
    USER_BONE_ID_RIGHT_FOREARM = mDObj->GetBoneIndex("Bip01 R Forearm");
    USER_BONE_ID_RIGHT_HAND = mDObj->GetBoneIndex("Bip01 R Hand");
    USER_BONE_ID_LEFT_THIGH = mDObj->GetBoneIndex("Bip01 L Thigh");
    USER_BONE_ID_LEFT_CALF = mDObj->GetBoneIndex("Bip01 L Calf");
    USER_BONE_ID_LEFT_FOOT = mDObj->GetBoneIndex("Bip01 L Foot");
    USER_BONE_ID_RIGHT_THIGH = mDObj->GetBoneIndex("Bip01 R Thigh");
    USER_BONE_ID_RIGHT_CALF = mDObj->GetBoneIndex("Bip01 R Calf");
    USER_BONE_ID_RIGHT_FOOT = mDObj->GetBoneIndex("Bip01 R Foot");
}

// ?g_cur_mat@@3PAVMat43@math@@A / ?g_last_mat@@3PAVMat43@math@@A (physics.o)
math::Mat43 g_cur_mat[10];
math::Mat43 g_last_mat[10];

// ea: 0x6F3AF0
void biped_phys_info::get_cur_vel(int rb_id, const math::Position3& com,
                                  math::Dir3* cur_tvel, math::Dir3* cur_avel)
{
    float m_delta_t = this->m_delta_t;
    math::Dir3 v12;
    v12.v = com.v;
    nuge::calc_velocities(g_last_mat[rb_id], g_cur_mat[rb_id], v12,
                          m_delta_t, cur_tvel, cur_avel);
    // velocity sanity asserts + 10.0 clamp (RBRagdoll.cpp:262-263)
    if ((cur_tvel->v.m128_f32[0] > 1000000.0f
         || cur_tvel->v.m128_f32[1] > 1000000.0f
         || cur_tvel->v.m128_f32[2] > 1000000.0f)
        && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 262,
                     "!IS_BAD_NUMBER((tv)[0]) && !IS_BAD_NUMBER((tv)[1]) && !IS_BAD_NUMBER((tv)[2])",
                     "Invalid vector"))
        __debugbreak();
    if ((cur_avel->v.m128_f32[0] > 1000000.0f
         || cur_avel->v.m128_f32[1] > 1000000.0f
         || cur_avel->v.m128_f32[2] > 1000000.0f)
        && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 263,
                     "!IS_BAD_NUMBER((av)[0]) && !IS_BAD_NUMBER((av)[1]) && !IS_BAD_NUMBER((av)[2])",
                     "Invalid vector"))
        __debugbreak();
    __m128 v = cur_tvel->v;
    if (_mm_shuffle_ps(v, v, 170).m128_f32[0] > 10.0f)
    {
        v.m128_f32[2] = 10.0f;
        cur_tvel->v = v;
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
