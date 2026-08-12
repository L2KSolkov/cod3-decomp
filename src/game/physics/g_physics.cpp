// ============================================================================
// g_physics.cpp - physics.o game integration (phys_xboxr physics.o family)
// Ported smallest-first from IDA (release map offset + 0x40C000 = VA).
// ============================================================================

#include "physics/physics_system.h"
#include "physics/rb_ragdoll_model.h"
#include <intrin.h>
#include <math.h>
#include <new>
#include <string.h>

extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);
extern const char* const defaultFileName;

namespace AeAssert {
// Binary enum (IDA: ARO=0, CD=1, JRS=2, MJK=3, MJU=4, MM=5, TPB=6, SLB=7,
// AC=8, JSV=9, DK=10, PL=11, DL=12). COD3 is a local alias for ARO used by
// earlier ports.
enum ECoderId {
    ARO = 0,
    CD = 1,
    JRS = 2,
    MJK = 3,
    MJU = 4,
    MM = 5,
    TPB = 6,
    SLB = 7,
    AC = 8,
    JSV = 9,
    DK = 10,
    PL = 11,
    DL = 12,
    COD3 = ARO,
};
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
void AnglesToAxis(const math::Position3* angles, float (*axis)[3]);
void AnglesToAxis(const math::Position3& angles, const math::Position3& origin,
                  math::Mat43& mat);  // ?AnglesToAxis@@YAXABVPosition3@math@@0AAVMat43@2@@Z

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
    static DebugRender sInst;
    static void RenderAxis(const math::Mat43& mat, float length, float width);
    void AddRenderer(void (*fp)());  // ?AddRenderer@DebugRender@@QAEXP6AXXZ@Z
};
// ?sInst@DebugRender@@2V1@A (physics.o data @ 0xF74D20)
DebugRender DebugRender::sInst;
void DebugRender::AddRenderer(void (*fp)())
{
    (void)fp;
}

class Entity;
struct rb_extra_info;
class DObj;
class rigid_body;
class biped_phys_info;
struct phys_gjk_geom_list;
struct trajectory_t;
void phys_collision_allocater_ballistic_reinit();  // 0x6F7060
enum phys_bones {
    rb_torso = 0,
    rb_head = 1,
    rb_left_up_arm = 2,
    rb_left_low_arm = 3,
    rb_right_up_arm = 4,
    rb_right_low_arm = 5,
    rb_left_thigh = 6,
    rb_left_calf = 7,
    rb_right_thigh = 8,
    rb_right_calf = 9,
    num_rb_phys_bones = 10,
};
void HelmetController(Entity* owner);  // ?HelmetController@@YAXPAVEntity@@@Z (cg.o)
enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };
#define PAK_ID_INVALID ((TPakId)-1)
void G_SetModel(Entity* ent, const char* modelName, TPakId pakId, int ngIndex);  // game2.o
void G_DObjUpdate(Entity* ent, bool forceWeaponModel);  // game2.o
void g_LinkEntity(Entity* ent);  // g.o
TPakId CurPakId();  // ?CurPakId@@YA?AW4TPakId@@XZ (streamer.o)
void BG_EvaluateTrajectory(const trajectory_t* tr, int atTime,
                           math::Position3& result);  // g.o
struct level_locals_t {
    uint8_t _pad0[0x9C];
    int     time;  // +0x9C
};
extern level_locals_t level;  // ?level@@3Ulevel_locals_t@@A
class EntityManager {
public:
    static EntityManager* sInst;  // ?sInst@EntityManager@@2PAV1@A (game.o)
    Entity* GetPlayer(int idx);   // ?GetPlayer@EntityManager@@QAEPAVEntity@@H@Z (g.o inline)
};
struct sentient_s {
    uint8_t _pad[0x38];
    int32_t bIgnoreMe;  // +0x38
};
struct actor_s {
    uint8_t _pad[0x310];
    int     bIsAlive;  // +0x310 (Physics.bIsAlive)
};
enum EPropPriority {
    PROP_PRIORITY_LOW = 0,
    PROP_PRIORITY_MEDIUM = 1,
    PROP_PRIORITY_HIGH = 2,
};
void DObjGetBasePose(DObj* obj);  // ?DObjGetBasePose@@YAXPAVDObj@@@Z (render.o)
void path_constraint_destroy(class rigid_body_constraint_custom_path* vpc);

// Handle (game_types.h view; local copy)
class Handle {
public:
    unsigned int mVal;  // +0x00
};

// HashString (broc_types.h view; local copy - 4 bytes)
class HashString {
public:
    unsigned int mHash;  // +0x00
};

// hash_const_t (g_local.h view; local copy - only physics fields used)
struct hash_const_t {
    uint8_t    _pad[0x114];
    HashString physicsdone;   // +0x114
    HashString physicsstart;  // +0x118
};
// ?hash_const@@3Uhash_const_t@@A (g.o data @ 0xED2AB0)
extern hash_const_t hash_const;

// Bitmask<T> (ae/core/bitmask.h). Add/Rmv are inline COMDATs (g.o 0x4ACC30 /
// 0x4ACCC0). class tag (V) required for V?$Bitmask@I@@ manglings.
template <typename T>
class Bitmask {
public:
    T mMask;  // +0x00

    void Add(int b)  // ?Add@?$Bitmask@I@@QAEXH@Z
    {
        if (b >= 0x20)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/bitmask.h";
            AeAssert::gCurrentLine = 77;
            AeAssert::gCurrentExpr =
                "b >= 0 && b < (int32)(sizeof(_T) * 8)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Please add a descriptive string"))
                __debugbreak();
        }
        mMask |= (T)(1 << b);
    }

    void Rmv(int b)  // ?Rmv@?$Bitmask@I@@QAEXH@Z
    {
        if (b >= 0x20)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/bitmask.h";
            AeAssert::gCurrentLine = 83;
            AeAssert::gCurrentExpr =
                "b >= 0 && b < (int32)(sizeof(_T) * 8)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Please add a descriptive string"))
                __debugbreak();
        }
        mMask &= (T)~(1 << b);
    }
};

// vehicle_rb_parameter (physics.o; minimal view for rb_vehicle methods)
class vehicle_rb_parameter {
public:
    uint8_t _pad0[0x18];
    float   m_susp_spring_k;      // +0x18
    uint8_t _pad1C[0x28 - 0x1C];
    float   m_tire_fric_fwd;      // +0x28
    float   m_tire_fric_side;     // +0x2C
    float   m_tire_fric_brake;    // +0x30
    float   m_tire_fric_hand_brake;  // +0x34
    uint8_t _pad38[0x58 - 0x38];
    float   m_peel_out_max_speed;  // +0x58

    static vehicle_rb_parameter* GetRBVehParameter(
        const char* name);  // ?GetRBVehParameter@vehicle_rb_parameter@@SAPAV1@PBD@Z
};

// rb_vehicle (physics.o RBVehicle.cpp). Layout verified against the ctor
// (0x6FC080) + is_peeling_out (0x6F4AD0) + path_constraint_update (0x6F5B60)
// disassembly. class tag (V) required for pool/param manglings.
class rb_vehicle {
public:
    int    m_wheel_effect_state[4];  // +0x00 (wheel_effect_state_e)
    Handle m_wheel_effects[4];       // +0x10
    Handle m_exhaust_effect;         // +0x20
    uint8_t _pad24[0x250 - 0x24];
    vehicle_rb_parameter* m_parameter;           // +0x250
    float m_throttle;                            // +0x254
    float m_brake;                               // +0x258
    float m_hand_brake;                          // +0x25C
    float m_script_brake;                        // +0x260
    float m_steer_factor;                        // +0x264
    float m_forward_vel;                         // +0x268
    float m_hand_brake_friction_time;            // +0x26C
    Entity* m_owner;                             // +0x270
    rb_extra_info* m_chassis_rbinf;              // +0x274
    rigid_body_constraint_custom_orientation* m_orientation_constraint;  // +0x278
    rigid_body_constraint_custom_path* m_vpc;    // +0x27C
    Bitmask<unsigned int> m_flags;               // +0x280
    uint8_t _pad284[0x310 - 0x284];
    float m_fake_rpm;                            // +0x310
    int   m_num_colliding_wheels;                // +0x314
    float m_current_side_fric_scale;             // +0x318
    float m_current_fwd_fric_scale;              // +0x31C
    void* m_wheels[8];                           // +0x320 (rigid_body_constraint_wheel*)
    int   m_wheel_count;                         // +0x340
    float m_desired_speed_factor;                // +0x344
    float m_acceleration_factor;                 // +0x348
    float m_power_braking_factor;                // +0x34C
    float m_braking_factor;                      // +0x350
    float m_coasting_factor;                     // +0x354
    float m_reference_wheel_radius;              // +0x358
    float m_steer_current_angle;                 // +0x35C
    float m_steer_max_angle;                     // +0x360
    float m_steer_speed;                         // +0x364
    uint8_t _pad368[0x370 - 0x368];
    math::Dir3 m_steer_front_pt_loc;             // +0x370
    float m_steer_front_back_length;             // +0x380
    int   m_state_flags;                         // +0x384
    void* m_vci;                                 // +0x388

    rb_vehicle();  // ??0rb_vehicle@@QAE@XZ
    void init(Entity* owner, vehicle_rb_parameter* parameter);  // ?init@rb_vehicle@@QAEXPAVEntity@@PAVvehicle_rb_parameter@@@Z
    void set(float power_braking_factor, float braking_factor,
             float desired_speed_factor, float acceleration_factor,
             float coasting_factor, float reference_wheel_radius,
             float steer_max_angle, float steer_speed,
             const math::Dir3& steer_front_pt_loc,
             float steer_front_back_length);  // ?set@rb_vehicle@@QAEXMMMMMMMMABVDir3@math@@M@Z
    void switch_parms(bool firstPerson);  // ?switch_parms@rb_vehicle@@QAEX_N@Z
    void update_parms(vehicle_rb_parameter* params, bool initialization);  // g_scr_vehicle.cpp
    void update_braking_and_acceleration(float delta_t);  // ?update_braking_and_acceleration@rb_vehicle@@QAEXM@Z
    static int get_num_rb_vehicles();  // ?get_num_rb_vehicles@rb_vehicle@@SAHXZ
    static rb_vehicle* get_vehicle(int i);  // ?get_vehicle@rb_vehicle@@SAPAV1@H@Z
    static rb_vehicle* add_vehicle();       // ?add_vehicle@rb_vehicle@@SAPAV1@XZ
    static void remove_vehicle(rb_vehicle* const v);  // ?remove_vehicle@rb_vehicle@@SAXQAV1@@Z
    math::Position3 process_hitp(const math::Position3& hitp);  // ?process_hitp@rb_vehicle@@QAE?AVPosition3@math@@ABV23@@Z
    bool is_peeling_out() const;  // ?is_peeling_out@rb_vehicle@@QBE_NXZ
    void set_default_pose();      // ?set_default_pose@rb_vehicle@@QAEXXZ
    void cleanup_path();          // ?cleanup_path@rb_vehicle@@QAEXXZ
    void remove_wheels();         // ?remove_wheels@rb_vehicle@@QAEXXZ
    void end_path();              // ?end_path@rb_vehicle@@QAEXXZ
    void pause_physics(bool shutdown);  // ?pause_physics@rb_vehicle@@QAEX_N@Z
    void unpause_physics();       // ?unpause_physics@rb_vehicle@@QAEXXZ
    math::Dir3 get_angular_velocity() const;  // ?get_angular_velocity@rb_vehicle@@QBE?AVDir3@math@@XZ
    math::Position3 get_rb_position() const;  // ?get_rb_position@rb_vehicle@@QBE?AVPosition3@math@@XZ
    math::Dir3 get_rb_angles() const;         // ?get_rb_angles@rb_vehicle@@QBE?AVDir3@math@@XZ
    static void frame_prolog_all_systems(float delta_t);  // ?frame_prolog_all_systems@rb_vehicle@@SAXM@Z
    static void frame_epilog_all_systems(float delta_t);  // ?frame_epilog_all_systems@rb_vehicle@@SAXM@Z
private:
    void _update_prolog(float delta_t);  // ?_update_prolog@rb_vehicle@@AAEXM@Z
    void _update_epilog(float delta_t);  // ?_update_epilog@rb_vehicle@@AAEXM@Z
    void _update_unpause();       // ?_update_unpause@rb_vehicle@@AAEXXZ
    void _align_wheels();         // ?_align_wheels@rb_vehicle@@AAEXXZ
    float _calc_initial_susp_spring_k(
        rigid_body_constraint_wheel* wheel_constraint);  // ?_calc_initial_susp_spring_k@rb_vehicle@@AAEMPAVrigid_body_constraint_wheel@@@Z
    void _update_friction(float delta_t);  // ?_update_friction@rb_vehicle@@AAEXM@Z
};
// rb_extra_info (physics.o RBPropSys.cpp). Layout verified against
// set_priority (0x6F6E60), frame_advance (0x6FE8A0) and try_collision_prolog
// (0x705F90) disassembly + IDA local type field order.
class rb_extra_info {
public:
    math::Mat43 m_transform;         // +0x00
    void*       m_gjk_geom_list;     // +0x40
    Entity*     m_ent;               // +0x44
    rigid_body* m_rb;                // +0x48
    math::Mat43* m_cg_mesh_mat;      // +0x4C
    Bitmask<unsigned int> m_flags;   // +0x50
    rb_vehicle* m_rb_vehicle;        // +0x54
    int         m_priority;          // +0x58 (EPropPriority)
    float       m_time_since_last_event;  // +0x5C
    void*       m_next;              // +0x60

    void collision_epilog();  // ?collision_epilog@rb_extra_info@@QAEXXZ
    void set_priority(EPropPriority p);  // ?set_priority@rb_extra_info@@QAEXW4EPropPriority@@@Z
    void frame_advance(float delta_t); // ?frame_advance@rb_extra_info@@QAEXM@Z
    void evaluate_effect_priority();   // ?evaluate_effect_priority@rb_extra_info@@QAEXXZ
    phys_gjk_geom_list* try_collision_prolog();  // ?try_collision_prolog@rb_extra_info@@QAEPAVphys_gjk_geom_list@@XZ
    void collision_prolog();  // ?collision_prolog@rb_extra_info@@QAEXXZ
    void set(Entity* const ent, rigid_body* const rb,
             const math::Mat43& transform, float bs_radius,
             const math::Position3& bs_center_loc);  // ?set@rb_extra_info@@QAEXQAVEntity@@QAVrigid_body@@ABVMat43@math@@MABVPosition3@5@@Z
};

template <typename T, int N> class phys_static_memory_pool;
// ?g_rb_vehicle_list@@3V?$phys_static_memory_pool@Vrb_vehicle@@$09@@A
// (physics.o data @ 0xE2B8F0)
extern phys_static_memory_pool<rb_vehicle, 10> g_rb_vehicle_list;

// ============================================================================
// phys_static_memory_pool<T,N> - fixed inline-slot pool (local copy of the
// pulse_sum.h template, declared class to match the binary's V-tag globals,
// e.g. ?g_rb_vehicle_list@@3V?$phys_static_memory_pool@Vrb_vehicle@@$09@@A).
// remove/is_member ported from COMDATs 0x71B100 / 0x717FC0.
// ============================================================================
template <typename T, int N>
class phys_static_memory_pool {
public:
    T   m_slots[N];        // +0x00
    T*  m_alloc_list[N];   // +N*sizeof(T)
    int m_index_array[N];  // +N*sizeof(T)+N*4
    T*  m_slot_array;      // +N*sizeof(T)+N*8
    int m_alloc_count;     // +N*sizeof(T)+N*8+4

    phys_static_memory_pool()
    {
        m_slot_array = (T*)this;
        m_alloc_count = 0;
        reset_buffer();
    }
    ~phys_static_memory_pool() {}

    void reset_buffer()
    {
        for (int i = 0; i < N; ++i)
        {
            m_index_array[i] = i;
            m_alloc_list[i] = m_slot_array + i;
        }
        m_alloc_count = 0;
    }

    void call_destructors() {}

    T* add(bool no_error, const char* error_msg)
    {
        int count = m_alloc_count;
        if (count < N)
        {
            T* result = m_alloc_list[count];
            m_alloc_count = count + 1;
            if (result != NULL)
                new (result) T;
            return result;
        }
        if (!no_error)
            tlFatal(error_msg);
        return NULL;
    }

    bool is_member(const T* data) const
    {
        int v2 = (int)((const char*)data - (const char*)m_slot_array);
        if (v2 % (int)sizeof(T) == 0)
        {
            int v3 = v2 / (int)sizeof(T);
            if (v3 >= 0 && v3 < N)
            {
                int v4 = m_index_array[v3];
                if (v4 >= 0 && v4 < m_alloc_count)
                    return true;
            }
        }
        return false;
    }

    void remove(T* data)
    {
        if (data != nullptr)
        {
            if (!is_member(data))
                tlFatal("phys_memory_pool: trying to delete an invalid pointer");
            if (!is_member(data)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                       61, "is_member(data)",
                       "phys_memory_pool: trying to delete an invalid pointer"))
                __debugbreak();
            if (m_alloc_count <= 0
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                       62, "m_alloc_count > 0",
                       "phys_memory_pool: trying to delete an invalid pointer"))
                __debugbreak();
            int v3 = (int)(data - m_slot_array);
            int v4 = m_index_array[v3];
            if ((v4 < 0 || v4 >= m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                       67, "alloc_index >= 0 && alloc_index < m_alloc_count",
                       "phys_memory_pool: trying to delete an invalid pointer"))
                __debugbreak();
            if (v4 >= 0)
            {
                int count = m_alloc_count;
                if (v4 < count)
                {
                    if (count > 1)
                    {
                        int  last = count - 1;
                        m_alloc_count = last;
                        T*  last_data = m_alloc_list[last];
                        T*  v8 = m_alloc_list[v4];
                        int slot_of_last = (int)(last_data - m_slot_array);
                        m_alloc_list[v4] = last_data;
                        m_alloc_list[last] = v8;
                        m_index_array[v3] = last;
                        m_index_array[slot_of_last] = v4;
                    }
                    else
                    {
                        reset_buffer();
                    }
                }
            }
        }
    }
};

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
    void* allocate(int size, int alignment, bool no_error,
                   const char* error_msg);  // ?allocate@phys_collision_allocater@@QAEPAXHH_NPBD@Z
    void nullify_buffer();  // ?nullify_buffer@phys_collision_allocater@@QAEXXZ
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

// ea: 0x719180 (inline COMDAT)
void* phys_collision_allocater::allocate(int size, int alignment, bool no_error,
                                         const char* error_msg)
{
    (void)no_error;
    (void)error_msg;
    for (int i = 0; i < m_num_buffers; ++i)
    {
        if (size <= 0
            && _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 59,
                         "size > 0", defaultFileName))
            __debugbreak();
        char* cur = m_list_memory_buffer[i].m_buffer_cur;
        char* result = (char*)((~(alignment - 1))
                               & (intptr_t)(cur + alignment - 1));
        if (result + size <= m_list_memory_buffer[i].m_buffer_end)
        {
            m_list_memory_buffer[i].m_buffer_cur = result + size;
            if (result != nullptr)
                return result;
        }
    }
    if (m_num_buffers >= 5)
        return nullptr;
    allocate_buffer();
    return m_list_memory_buffer[m_num_buffers - 1].allocate_no_error(size,
                                                                     alignment);
}

// ea: 0x7190F0 (inline COMDAT)
void phys_collision_allocater::nullify_buffer()
{
    for (int i = 1; i < m_num_buffers; ++i)
        PakManager::sInst->CrazyTempMemGiveBack(
            m_list_memory_buffer[i].m_buffer_start);
    for (int i = 0; i < 5; ++i)
    {
        m_list_memory_buffer[i].m_buffer_start = nullptr;
        m_list_memory_buffer[i].m_buffer_end = nullptr;
        m_list_memory_buffer[i].m_buffer_cur = nullptr;
        m_list_memory_buffer[i].m_user_start = nullptr;
    }
    m_num_buffers = 0;
}

// ?g_collision_memory_allocater@@3Vphys_collision_allocater@@A
// (physics.o data @ 0xF8BDD0)
phys_collision_allocater g_collision_memory_allocater;

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
    void get_all_collisions();  // ?get_all_collisions@ragdoll_collision_callback@@QAEXXZ
    void process_environment_collision_events();  // ?process_environment_collision_events@ragdoll_collision_callback@@QAEXXZ
    void remove_colgeom(int rb_id);  // ?remove_colgeom@ragdoll_collision_callback@@QAEXH@Z
};

// ea: 0x6F46A0
void ragdoll_collision_callback::set(Entity* const owner)
{
    m_owner = owner;
}

// stub until ragdoll_collision_callback internals are ported (0x70BD00)
void ragdoll_collision_callback::get_all_collisions()
{
}

// stub until ragdoll_collision_callback internals are ported (0x700910)
void ragdoll_collision_callback::process_environment_collision_events()
{
}

// stub until ragdoll_collision_callback internals are ported (0x708DC0)
void ragdoll_collision_callback::remove_colgeom(int rb_id)
{
    (void)rb_id;
}

// DObj (render.o; local stub view). copy_skeleton disasm reads numBones at
// +0xCF; GetMat/GetBoneIndex are render.o symbols (?GetMat@DObj@@QAEABVMat43@
// math@@H@Z / ?GetBoneIndex@DObj@@QBEHPBD@Z) - stub until render.o is ported.
class DObj {
public:
    uint8_t _pad0[0xCF];
    unsigned char numBones;  // +0xCF

    const math::Mat43& GetMat(int boneIndex);
    int GetBoneIndex(const char* name) const;
};
const math::Mat43& DObj::GetMat(int boneIndex)
{
    (void)boneIndex;
    static math::Mat43 zero = {};
    return zero;
}
int DObj::GetBoneIndex(const char* name) const
{
    (void)name;
    return -1;
}

// Entity (game_types.h view; local minimal copy - cannot include game_types.h)
// trajectory_t (game_types.h view; local copy - 40 bytes)
struct trajectory_t {
    int   trType;         // +0x00 (trType_t)
    int32_t trTime;       // +0x04
    int32_t trDuration;   // +0x08
    float trBase[3];      // +0x0C
    float trDelta[3];     // +0x18
    int32_t trGravityOverride;  // +0x24
};
class Entity {
public:
    struct refEntity {  // EntityShared subset
        uint8_t        _pad0[0x04];
        int32_t        svFlags;      // +0x04
        uint8_t        _pad08[0x64 - 0x08];
        int32_t        contents;     // +0x64
        uint8_t        _pad68[0x70 - 0x68];
        math::Position3 currentOrigin;  // +0x70
        math::Position3 currentAngles;  // +0x80
        math::Mat43     currentMat;     // +0x90
    };

    uint8_t _pad0[0x10];
    int32_t eFlags;              // +0x10 (EntityState)
    trajectory_t tr;             // +0x14
    uint8_t _pad3C[0xE0 - 0x3C];
    refEntity r;                 // +0xE0 (r.currentOrigin +0x150, currentAngles +0x160,
                                 //  currentMat +0x170)
    uint8_t _pad140[0x230 - (0xE0 + sizeof(refEntity))];
    int32_t mPakId;              // +0x230
    uint8_t _pad234[0x23C - 0x234];
    DObj* mDObj;                 // +0x23C
    uint8_t _pad240[0x248 - 0x240];
    biped_phys_info* mBPInfo;    // +0x248
    uint8_t _pad24C[0x25C - 0x24C];
    void* actor;                 // +0x258 (actor_s*)
    void* sentient;              // +0x25C (sentient_s*)
    void* scr_vehicle;           // +0x260 (scr_vehicle_t*)
    uint8_t _pad264[0x2B0 - 0x264];
    uint8_t physicsObject;       // +0x2B0
    uint8_t _pad2B1[0x2B8 - 0x2B1];
    int32_t takedamage;          // +0x2B8
    uint8_t _pad2BC[0x2C4 - 0x2BC];
    int32_t  flags;              // +0x2C4
    unsigned int mFlags;         // +0x2C8 (Bitmask<unsigned int>)
    uint8_t _pad2CC[0x348 - 0x2CC];
    int32_t nextthink;           // +0x348
    int32_t think;               // +0x34C (fn_think_e)
    uint8_t _pad350[0x355 - 0x350];
    uint8_t  die;                // +0x355
    uint8_t  _pad356[3];
    int32_t  health;             // +0x358

    math::Mat43 CalcAbsMat(int boneIndex);  // ?CalcAbsMat@Entity@@QAE?AVMat43@math@@H@Z
    math::Mat43 GetRelMat(int boneIndex);   // ?GetRelMat@Entity@@QAE?AVMat43@math@@H@Z
    const math::Mat43 CalcRotTranMat43();  // ?CalcRotTranMat43@Entity@@QAE?BVMat43@math@@XZ
    void Notify(HashString h);  // ?Notify@Entity@@QAEXVHashString@@@Z (game.o)
};

// ?DObjGetBasePose@@YAXPAVDObj@@@Z (render.o; stub until render.o is ported)
void DObjGetBasePose(DObj* obj)
{
    (void)obj;
}

// Camera (cg.o view; minimal local copy for evaluate_effect_priority)
struct Camera {
    uint8_t         _pad0[0x30];
    math::Position3 mPrevViewPos;  // +0x30
    math::Position3 mPrevAngles;   // +0x40
    math::Position3 mPrevViewDir;  // +0x50
};
extern Camera gCamera[8];  // ?gCamera@@3PAVCamera@@A (cg.o @ 0x1358EF0)
extern int currCl;         // ?currCl@@3HA (cg.o)

// ea: 0x6F6E60
void rb_extra_info::set_priority(EPropPriority p)
{
    m_priority = p;
    if (p == PROP_PRIORITY_LOW)
    {
        m_ent->mFlags &= ~1u;
        m_flags.mMask &= ~2u;
        m_flags.mMask &= ~4u;
        m_flags.mMask &= ~8u;
    }
    else if (p == PROP_PRIORITY_HIGH)
    {
        m_flags.mMask |= 8u;
        m_flags.mMask |= 0x10u;
    }
}

// ea: 0x6FE8A0
void rb_extra_info::frame_advance(float delta_t)
{
    if (m_priority != 0)
    {
        m_time_since_last_event = delta_t + m_time_since_last_event;
        if (m_time_since_last_event > 2.0f)
            evaluate_effect_priority();
    }
}

// ea: 0x6F6EC0
void rb_extra_info::evaluate_effect_priority()
{
    if (m_priority != 0)
    {
        __m128 v1 = gCamera[currCl].mPrevViewDir.v;
        math::Position3* p_mPrevViewPos = &gCamera[currCl].mPrevViewPos;
        __m128 v3 = _mm_set1_ps(0.5f);
        __m128 v4 = _mm_add_ps(
            p_mPrevViewPos->v,
            _mm_mul_ps(_mm_mul_ps(v1, _mm_set1_ps(400.0f)), v3));
        __m128 cur_origin = m_ent->r.currentOrigin.v;
        __m128 v5 = _mm_shuffle_ps(
            cur_origin, _mm_shuffle_ps(_mm_setzero_ps(), cur_origin, 240), 196);
        __m128 v6 = _mm_sub_ps(
            _mm_shuffle_ps(v4, _mm_shuffle_ps(_mm_setzero_ps(), v4, 240), 196),
            v5);
        __m128 v7 = _mm_mul_ps(v6, v6);
        if ((v7.m128_f32[0]
             + (_mm_shuffle_ps(v7, v7, 85).m128_f32[0]
                + _mm_shuffle_ps(v7, v7, 170).m128_f32[0]))
            >= 160000.0f)
        {
            __m128 v9 = _mm_add_ps(
                p_mPrevViewPos->v,
                _mm_mul_ps(_mm_mul_ps(v1, _mm_set1_ps(1300.0f)), v3));
            __m128 v10 = _mm_sub_ps(
                _mm_shuffle_ps(v9, _mm_shuffle_ps(_mm_setzero_ps(), v9, 240), 196),
                v5);
            __m128 v11 = _mm_mul_ps(v10, v10);
            if ((v11.m128_f32[0]
                 + (_mm_shuffle_ps(v11, v11, 85).m128_f32[0]
                    + _mm_shuffle_ps(v11, v11, 170).m128_f32[0]))
                < 1690000.0f)
                m_flags.mMask |= 8u;
        }
        else
        {
            unsigned int v8 = m_flags.mMask | 0x18;
            m_flags.mMask |= 8u;
            m_flags.mMask = v8;
        }
    }
}

// ea: 0x6FC080
rb_vehicle::rb_vehicle()
{
    m_wheel_effects[0].mVal = 0;
    m_wheel_effects[1].mVal = 0;
    m_wheel_effects[2].mVal = 0;
    m_wheel_effects[3].mVal = 0;
    m_exhaust_effect.mVal = 0;
    m_parameter = nullptr;
    m_script_brake = 0.0f;
    m_hand_brake_friction_time = 0.0f;
    m_owner = nullptr;
    m_orientation_constraint = nullptr;
    m_vpc = nullptr;
    m_flags.mMask = 0;
    m_fake_rpm = 0.0f;
    m_vci = nullptr;
    m_flags.mMask |= 1;
    m_flags.mMask |= 2;
    m_flags.mMask &= 0xFFFFFFFB;
    m_wheels[0] = nullptr;
    m_wheels[1] = nullptr;
    m_wheels[2] = nullptr;
    m_wheels[3] = nullptr;
    m_wheels[4] = nullptr;
    m_wheels[5] = nullptr;
    m_wheels[6] = nullptr;
    m_wheels[7] = nullptr;
    m_wheel_count = 0;
}

// ea: 0x6FCDE0
int rb_vehicle::get_num_rb_vehicles()
{
    return g_rb_vehicle_list.m_alloc_count;
}

// ea: 0x6F4A00
math::Position3 rb_vehicle::process_hitp(const math::Position3& hitp)
{
    (void)hitp;
    math::Position3 result;
    result.v = _mm_setzero_ps();  // Float4_Zero_12
    return result;
}

// ea: 0x6F4AD0
bool rb_vehicle::is_peeling_out() const
{
    return m_throttle > 0.89999998f
        && m_parameter->m_peel_out_max_speed > m_forward_vel;
}

// ea: 0x6F4F30
void rb_vehicle::set_default_pose()
{
    DObjGetBasePose(m_owner->mDObj);
}

// ea: 0x6FCD60
void rb_vehicle::cleanup_path()
{
    if (m_vpc != nullptr)
    {
        path_constraint_destroy(m_vpc);
        m_vpc = nullptr;
    }
}

// wheel_effect_state_e (physics.o RBVehicle.cpp)
enum wheel_effect_state_e {
    WHEEL_STATE_AIRBORN = 1,
};
// vehicle_info_t (g_vehiclefuncs.h view; type +0x20)
struct vehicle_info_t {
    uint8_t _pad0[0x20];
    short   type;  // +0x20
    uint8_t _pad22[0x21C - 0x22];
    char    vehiclePhysicsParms[32];  // +0x21C
    char    vehiclePhysicsParmsThird[32];  // +0x23C
};
vehicle_info_t* VEH_GetInfo(int idx);  // ?VEH_GetInfo@@YAPAUvehicle_info_t@@H@Z (g.o)
// ?g_vehicle_gravity_multiplier@@3MA (physics.o data @ 0xE01EE0)
float g_vehicle_gravity_multiplier = 1.5f;  // 0x3FC00000
// ?hand_brake_friction_time@@3MA (physics.o data @ 0xE36AC4)
float hand_brake_friction_time = 0.3f;      // 0x3E99999A

// scr_vehicle_t (g_local.h view; pathPos + infoIdx + mRBVeh)
struct scr_vehicle_path_view {
    float origin[3];   // +0x00
    float angles[3];   // +0x10
};
struct scr_vehicle_t {
    scr_vehicle_path_view pathPos;  // +0x00
    uint8_t _pad[0x178 - sizeof(scr_vehicle_path_view)];
    int16_t infoIdx;                // +0x178
    uint8_t _pad17A[0x518 - 0x17A];
    void*   mRBVeh;                 // +0x518 rb_vehicle*
};

// RBVehicleController (physics.o RBVehicleController.cpp; minimal view for the
// script-target methods)
class RBVehicleController {
public:
    math::Position3 m_script_goal_position;  // +0x00
    float m_script_goal_radius;              // +0x10
    float m_script_goal_speed;               // +0x14

    void SetScriptTarget(rb_vehicle& rbveh, const math::Position3& goal_position,
                         float goal_radius, float goal_speed);  // ?SetScriptTarget@RBVehicleController@@QAEXAAVrb_vehicle@@ABVPosition3@math@@MM@Z
};

// ea: 0x6FE100
void RBVehicleController::SetScriptTarget(
    rb_vehicle& rbveh, const math::Position3& goal_position, float goal_radius,
    float goal_speed)
{
    float v5 = 5.0f;
    m_script_goal_position.v = goal_position.v;
    if (goal_radius >= 5.0f)
    {
        v5 = 5000.0f;
        if (goal_radius <= 5000.0f)
            v5 = goal_radius;
    }
    m_script_goal_radius = v5;
    m_script_goal_speed = goal_speed;
    rbveh.m_flags.mMask |= 8u;
}

// ea: 0x6FC140
void rb_vehicle::init(Entity* owner, vehicle_rb_parameter* parameter)
{
    m_parameter = parameter;
    m_owner = owner;
    m_brake = 0.0f;
    m_throttle = 0.0f;
    m_steer_factor = 0.0f;
    m_forward_vel = 0.0f;
    m_script_brake = 0.0f;
    DObjGetBasePose(owner->mDObj);
    _align_wheels();
    if (VEH_GetInfo(((scr_vehicle_t*)owner->scr_vehicle)->infoIdx)->type == 2)
        m_flags.mMask |= 0x20u;
    m_wheel_effect_state[0] = WHEEL_STATE_AIRBORN;
    m_wheel_effects[0].mVal = 0;
    m_wheel_effect_state[1] = WHEEL_STATE_AIRBORN;
    m_wheel_effects[1].mVal = 0;
    m_wheel_effect_state[2] = WHEEL_STATE_AIRBORN;
    m_wheel_effects[2].mVal = 0;
    m_wheel_effect_state[3] = WHEEL_STATE_AIRBORN;
    m_wheel_effects[3].mVal = 0;
    m_exhaust_effect.mVal = 0;
}

// stub until rb_vehicle::_align_wheels (0x6F4830) is ported
void rb_vehicle::_align_wheels()
{
}

// ea: 0x6F4A30
float rb_vehicle::_calc_initial_susp_spring_k(
    rigid_body_constraint_wheel* wheel_constraint)
{
    float v2 = (float)(fabs(wheel_constraint->m_b1_wheel_center_loc
                                .v.m128_f32[0])
                       / m_steer_front_back_length);
    float v5 = v2;
    if (v2 < 0.0f)
        v5 = 0.0f;
    else if (v5 > 1.0f)
        v5 = 1.0f;
    return (1.0f - v5 + 1.0f - v5) * m_parameter->m_susp_spring_k
           * g_vehicle_gravity_multiplier;
}

// ea: 0x6F5780
void rb_vehicle::update_braking_and_acceleration(float delta_t)
{
    (void)delta_t;
    for (int i = 0; i < 8; ++i)
    {
        rigid_body_constraint_wheel* v5 =
            (rigid_body_constraint_wheel*)m_wheels[i];
        if (v5 != nullptr)
        {
            if ((m_state_flags & 1) != 0 && (v5->m_wheel_flags & 0x20) != 0)
            {
                v5->set_wheel_state_braking(m_power_braking_factor);
            }
            else if ((m_state_flags & 2) != 0
                     && (v5->m_wheel_flags & 0x40) != 0)
            {
                v5->set_wheel_state_braking(m_braking_factor);
            }
            else if ((m_state_flags & 4) != 0
                     && (v5->m_wheel_flags & 0x10) != 0)
            {
                v5->set_wheel_state_accelerating(
                    m_desired_speed_factor / m_reference_wheel_radius,
                    m_acceleration_factor);
            }
            else if ((m_state_flags & 8) != 0
                     && (v5->m_wheel_flags & 0x10) != 0)
            {
                v5->set_wheel_state_accelerating(
                    0.0f - (m_desired_speed_factor
                            / m_reference_wheel_radius),
                    m_acceleration_factor);
            }
            else if ((m_state_flags & 0x10) != 0)
            {
                v5->set_wheel_state_braking(m_coasting_factor);
            }
            else
            {
                v5->set_wheel_state_braking(0.0f);
            }
        }
    }
}

// ea: 0x6FCB70
void rb_vehicle::_update_friction(float delta_t)
{
    if ((m_flags.mMask & 0x100) == 0)
    {
        m_hand_brake_friction_time = m_hand_brake_friction_time - delta_t;
        if (m_hand_brake > 0.1f
            && m_parameter->m_tire_fric_hand_brake > 0.0f)
            m_hand_brake_friction_time = hand_brake_friction_time;
        rigid_body* m_rb = m_chassis_rbinf->m_rb;
        if ((~(m_rb->m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h",
                         79, "debug_flag_is_not_in_collision()",
                         defaultFileName))
            __debugbreak();
        float v9 = _mm_shuffle_ps(m_rb->m_mat.z.v, m_rb->m_mat.z.v, 170)
                       .m128_f32[0];
        float v4 = 1.0f;
        if (v9 < 0.94999999f)
        {
            v4 = v9 * 0.7368421f;
            if (v4 < 0.15000001f)
                v4 = 0.15000001f;
            else if (v4 > 1.0f)
                v4 = 1.0f;
        }
        if ((m_flags.mMask & 0x100) != 0)
        {
            m_current_side_fric_scale = 0.0f;
            m_current_fwd_fric_scale = 0.0f;
            return;
        }
        if (m_brake > 0.1f)
        {
            float v6 = m_parameter->m_tire_fric_brake * v4;
            m_current_side_fric_scale = v6;
            m_current_fwd_fric_scale = v6;
            return;
        }
        if (m_hand_brake_friction_time <= 0.0f)
        {
            m_current_side_fric_scale =
                m_parameter->m_tire_fric_side * v4;
        }
        else
        {
            float v7 = m_parameter->m_tire_fric_hand_brake * v4;
            m_current_side_fric_scale = v7;
            if (fabs(m_throttle) <= 0.1f)
            {
                m_current_fwd_fric_scale = v7;
                return;
            }
        }
        m_current_fwd_fric_scale = m_parameter->m_tire_fric_fwd * v4;
    }
}

// ea: 0x6F4F50
void rb_vehicle::remove_wheels()
{
    for (int i = 0; i < 8; ++i)
    {
        if (m_wheels[i] != nullptr)
            m_wheels[i] = nullptr;
    }
    m_wheel_count = 0;
}

// ea: 0x6F4FD0
void rb_vehicle::set(float power_braking_factor, float braking_factor,
                     float desired_speed_factor, float acceleration_factor,
                     float coasting_factor, float reference_wheel_radius,
                     float steer_max_angle, float steer_speed,
                     const math::Dir3& steer_front_pt_loc,
                     float steer_front_back_length)
{
    m_power_braking_factor = power_braking_factor;
    m_braking_factor = braking_factor;
    m_desired_speed_factor = desired_speed_factor;
    m_acceleration_factor = acceleration_factor;
    m_coasting_factor = coasting_factor;
    m_reference_wheel_radius = reference_wheel_radius;
    m_steer_max_angle = steer_max_angle;
    m_steer_speed = steer_speed;
    m_steer_front_pt_loc.v = steer_front_pt_loc.v;
    m_steer_front_back_length = steer_front_back_length;
    m_steer_factor = 0.0f;
    m_steer_current_angle = 0.0f;
    m_state_flags = 0;
    m_forward_vel = 0.0f;
}

// ea: 0x704F00
void rb_vehicle::switch_parms(bool firstPerson)
{
    vehicle_info_t* Info = VEH_GetInfo(((scr_vehicle_t*)m_owner->scr_vehicle)->infoIdx);
    if (Info == nullptr)
        return;
    if (!firstPerson || Info->vehiclePhysicsParms[0] == 0)
    {
        if (Info->vehiclePhysicsParmsThird[0] == 0)
            return;
        vehicle_rb_parameter* RBVehParameter =
            vehicle_rb_parameter::GetRBVehParameter(
                Info->vehiclePhysicsParmsThird);
        if (RBVehParameter == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBVehicle.cpp";
            AeAssert::gCurrentLine = 380;
            AeAssert::gCurrentExpr = "parms";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                       "Failed to find vehicle physics settings for %s",
                       Info->vehiclePhysicsParmsThird))
                __debugbreak();
            return;
        }
        update_parms(RBVehParameter, false);
        return;
    }
    vehicle_rb_parameter* RBVehParameter =
        vehicle_rb_parameter::GetRBVehParameter(Info->vehiclePhysicsParms);
    if (RBVehParameter != nullptr)
    {
        update_parms(RBVehParameter, false);
        return;
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBVehicle.cpp";
    AeAssert::gCurrentLine = 375;
    AeAssert::gCurrentExpr = "parms";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("Failed to find vehicle physics settings for %s",
                            Info->vehiclePhysicsParms))
        __debugbreak();
}

// ea: 0x6FCD90
rb_vehicle* rb_vehicle::get_vehicle(int i)
{
    if (i >= 0 && i < g_rb_vehicle_list.m_alloc_count)
        return g_rb_vehicle_list.m_alloc_list[i];
    if (!_tlAssert(
            "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
            178, "i >= 0 && i < m_alloc_count", defaultFileName))
        __debugbreak();
    return g_rb_vehicle_list.m_alloc_list[i];
}

// ea: 0x701AC0
rb_vehicle* rb_vehicle::add_vehicle()
{
    rb_vehicle* v0 = g_rb_vehicle_list.add(
        true, "phys memory pool add overflow.");
    if (v0 == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBVehicle.cpp";
        AeAssert::gCurrentLine = 1310;
        AeAssert::gCurrentExpr = "vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Failed to add a new rb_vehicle."))
            __debugbreak();
    }
    return v0;
}

// ea: 0x701B20
void rb_vehicle::remove_vehicle(rb_vehicle* const v)
{
    v->pause_physics(true);
    g_rb_vehicle_list.remove(v);
}

// ea: 0x701A70
void rb_vehicle::end_path()
{
    m_flags.mMask &= ~0x100u;
    m_flags.mMask &= ~0x200u;
    if (m_vpc != nullptr)
    {
        path_constraint_destroy(m_vpc);
        m_vpc = nullptr;
    }
}

// stub until rb_vehicle::pause_physics (0x700D50) is ported
void rb_vehicle::pause_physics(bool shutdown)
{
    (void)shutdown;
}

// stub until rb_vehicle::_update_unpause (0x709090) is ported
void rb_vehicle::_update_unpause()
{
}

// ea: 0x70C080
void rb_vehicle::unpause_physics()
{
    if ((m_flags.mMask & 1) != 0)
    {
        m_flags.mMask |= 2u;
        _update_unpause();
    }
}

// ea: 0x6FC290
math::Dir3 rb_vehicle::get_angular_velocity() const
{
    math::Dir3 result;
    if ((m_flags.mMask & 1) != 0)
        result.v = _mm_setzero_ps();
    else
        result.v = m_chassis_rbinf->m_rb->m_a_vel.v;
    return result;
}

// ea: 0x6FC320
math::Position3 rb_vehicle::get_rb_position() const
{
    math::Position3 result;
    if ((m_flags.mMask & 1) != 0)
    {
        result.v = _mm_setzero_ps();
        return result;
    }
    rigid_body* m_rb = m_chassis_rbinf->m_rb;
    if ((~(m_rb->m_flags >> 6) & 1) == 0
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 79,
                     "debug_flag_is_not_in_collision()", defaultFileName))
        __debugbreak();
    result.v = m_rb->m_mat.w.v;
    return result;
}

void AxisToAngles(const float (*axis)[3], float* angles);  // ?AxisToAngles@@YAXQAY02$$CBMQAM@Z (q_math.cpp)

// ea: 0x6FC3D0
math::Dir3 rb_vehicle::get_rb_angles() const
{
    math::Dir3 angles;
    angles.v = _mm_setzero_ps();
    if ((m_flags.mMask & 1) != 0)
        return angles;
    rigid_body* m_rb = m_chassis_rbinf->m_rb;
    if ((~(m_rb->m_flags >> 6) & 1) == 0
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 79,
                     "debug_flag_is_not_in_collision()", defaultFileName))
        __debugbreak();
    AxisToAngles((const float(*)[3])m_rb, &angles.v.m128_f32[0]);
    return angles;
}

// stub until rb_vehicle::_update_prolog (0x70CA40) is ported
void rb_vehicle::_update_prolog(float delta_t)
{
    (void)delta_t;
}

// stub until rb_vehicle::_update_epilog (0x7091E0) is ported
void rb_vehicle::_update_epilog(float delta_t)
{
    (void)delta_t;
}

// ea: 0x70CF60
void rb_vehicle::frame_prolog_all_systems(float delta_t)
{
    int count = g_rb_vehicle_list.m_alloc_count;
    for (int i = 0; i < count; ++i)
        g_rb_vehicle_list.m_alloc_list[i]->_update_prolog(delta_t);
}

// ea: 0x7096A0
void rb_vehicle::frame_epilog_all_systems(float delta_t)
{
    int count = g_rb_vehicle_list.m_alloc_count;
    for (int i = 0; i < count; ++i)
        g_rb_vehicle_list.m_alloc_list[i]->_update_epilog(delta_t);
}

// rb_prop_system free helpers (physics.o RBPropSys.cpp; namespace in binary)
extern phys_static_memory_pool<rb_extra_info, 35> g_list_rb_extra_info;
// ?g_list_rb_extra_info@@3V?$phys_static_memory_pool@Vrb_extra_info@@$0CD@@@A
namespace rb_prop_system {
const rigid_body* get_entity_rb(Entity* e);        // ?get_entity_rb@rb_prop_system@@YAPBVrigid_body@@PAVEntity@@@Z
bool entity_in_system(Entity* e);                  // ?entity_in_system@rb_prop_system@@YA_NPAVEntity@@@Z
bool is_entity_stable(Entity* e);                  // ?is_entity_stable@rb_prop_system@@YA_NPAVEntity@@@Z
rigid_body* get_associated_rigid_body(Entity* e);  // ?get_associated_rigid_body@rb_prop_system@@YAPAVrigid_body@@PAVEntity@@@Z
    void frame_advance(float delta_t);                 // ?frame_advance@rb_prop_system@@YAXM@Z
    void remove_entity(Entity* e);                     // ?remove_entity@rb_prop_system@@YAXPAVEntity@@@Z
}

// ea: 0x6FEAD0
const rigid_body* rb_prop_system::get_entity_rb(Entity* e)
{
    int count = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
    {
        rb_extra_info* inf = g_list_rb_extra_info.m_alloc_list[i];
        if (inf->m_ent == e)
            return inf->m_rb;
    }
    return nullptr;
}

// ea: 0x6FEA40
bool rb_prop_system::entity_in_system(Entity* e)
{
    int count = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
    {
        if (g_list_rb_extra_info.m_alloc_list[i]->m_ent == e)
            return true;
    }
    return false;
}

// ea: 0x6FEA80
bool rb_prop_system::is_entity_stable(Entity* e)
{
    int count = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
    {
        rb_extra_info* inf = g_list_rb_extra_info.m_alloc_list[i];
        if (inf->m_ent == e && (inf->m_rb->m_flags & 4) == 0)
            return false;
    }
    return true;
}

// ea: 0x703DB0
rigid_body* rb_prop_system::get_associated_rigid_body(Entity* e)
{
    return const_cast<rigid_body*>(get_entity_rb(e));
}

// ea: 0x6FE8D0
void rb_prop_system::remove_entity(Entity* e)
{
    int m_alloc_count = g_list_rb_extra_info.m_alloc_count;
    int v2 = 0;
    bool v3 = false;
    int i = 0;
    if (m_alloc_count > 0)
    {
        for (;;)
        {
            if ((v2 < 0 || v2 >= m_alloc_count)
                && _tlAssert(
                       "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                       178, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            rb_extra_info* v4 = g_list_rb_extra_info.m_alloc_list[v2];
            if (v4->m_ent == e)
            {
                if (v3)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JRS;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\RBPropSys.cpp";
                    AeAssert::gCurrentLine = 549;
                    AeAssert::gCurrentExpr = "!found";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(
                               "Found a single entity that had multiple rigid bodies."))
                        __debugbreak();
                }
                e->flags &= ~0x400000u;
                phys_sys::destroy(v4->m_rb);
                g_list_rb_extra_info.remove(v4);
                v3 = true;
            }
            else
            {
                ++i;
            }
            m_alloc_count = g_list_rb_extra_info.m_alloc_count;
            if (i >= m_alloc_count)
                break;
            v2 = i;
        }
    }
}

// ea: 0x7185F0 (inline COMDAT) - dest = left * right (row-vector)
void phys_full_multiply_mat(math::Mat43& dest, const math::Mat43& left,
                            const math::Mat43& right)
{
    __m128 v3 = left.z.v;
    __m128 v4 = left.y.v;
    __m128 v5 = left.x.v;
    dest.x.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(right.x.v, right.x.v, 0),
                              left.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(right.x.v, right.x.v, 85), v4)),
        _mm_mul_ps(_mm_shuffle_ps(right.x.v, right.x.v, 170), v3));
    dest.y.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(right.y.v, right.y.v, 0),
                              left.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(right.y.v, right.y.v, 85), v4)),
        _mm_mul_ps(_mm_shuffle_ps(right.y.v, right.y.v, 170), v3));
    dest.z.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(right.z.v, right.z.v, 0),
                              left.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(right.z.v, right.z.v, 85), v4)),
        _mm_mul_ps(_mm_shuffle_ps(right.z.v, right.z.v, 170), v3));
    dest.w.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(right.w.v, right.w.v, 0), v5),
            _mm_mul_ps(_mm_shuffle_ps(right.w.v, right.w.v, 85), v4)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(right.w.v, right.w.v, 170), v3),
                   left.w.v));
}

// Float4_SignMask_12 (.rdata @ 0xD143B0; 0x80000000 x4)
static const __m128 Float4_SignMask_12 = { -0.0f, -0.0f, -0.0f, -0.0f };

// ea: 0x71A240 (inline COMDAT)
void full_inverse(math::Mat43& dest, const math::Mat43& source)
{
    if (&dest == &source)
    {
        math::Mat43 tmp;
        full_inverse(tmp, source);
        memcpy(&dest, &tmp, sizeof(math::Mat43));
        return;
    }
    __m128 v3 = source.y.v;
    __m128 v4 = source.x.v;
    __m128 v5 = source.z.v;
    __m128 v6 = _mm_shuffle_ps(source.x.v, v3, 68);
    dest.x.v = _mm_shuffle_ps(v6, v5, 136);
    dest.y.v = _mm_shuffle_ps(v6, v5, 221);
    dest.z.v = _mm_shuffle_ps(_mm_shuffle_ps(v4, v3, 238), v5, 168);
    dest.w.v = _mm_xor_ps(
        Float4_SignMask_12,
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(source.w.v, source.w.v, 0),
                           dest.x.v),
                _mm_mul_ps(_mm_shuffle_ps(source.w.v, source.w.v, 85),
                           dest.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(source.w.v, source.w.v, 170),
                       dest.z.v)));
}

// ea: 0x6FA9E0
void calc_rb_mat_from_bone(Entity* ent, int boneIndex, rigid_body* rb,
                           const math::Mat43& bone_mat_loc)
{
    math::Mat43 v7;
    full_inverse(v7, bone_mat_loc);
    math::Mat43 v6 = ent->CalcAbsMat(boneIndex);
    phys_full_multiply_mat(rb->m_mat, v6, v7);
}

// wheel_collision_info (physics.o vehicle_collision.cpp). Layout from setup
// (0x6F6C20) + vehicle_collision_info::setup (0x6FE630) disassembly: m_p0
// +0x00, m_p1 +0x10, m_aabb_mn +0x20, m_aabb_mx +0x30, m_t +0x50,
// m_surface_flags +0x54, m_did_hit +0x58, m_hit_rb +0x5C, m_rbc_wheel +0x60.
struct wheel_collision_info {
    math::Dir3 m_p0;            // +0x00
    math::Dir3 m_p1;            // +0x10
    math::Dir3 m_aabb_mn;       // +0x20
    math::Dir3 m_aabb_mx;       // +0x30
    uint8_t    _pad40[0x50 - 0x40];
    float      m_t;             // +0x50
    int        m_surface_flags; // +0x54
    bool       m_did_hit;       // +0x58
    rigid_body* m_hit_rb;       // +0x5C
    rigid_body_constraint_wheel* m_rbc_wheel;  // +0x60

    void setup(rigid_body* rb, rigid_body_constraint_wheel* rbc_wheel);  // ?setup@wheel_collision_info@@QAEXPAVrigid_body@@PAVrigid_body_constraint_wheel@@@Z
    void process(rb_extra_info* rb_inf, int wheel_i);  // ?process@wheel_collision_info@@QAEXPAVrb_extra_info@@H@Z
};

// phys_gjk_geom_list (physics.o; aabb fields used by vehicle_collision_info)
class phys_gjk_geom_list {
public:
    math::Dir3 m_aabb_mn;  // +0x00
    math::Dir3 m_aabb_mx;  // +0x10
};

// stub until try_collision_prolog (0x705F90) is ported
phys_gjk_geom_list* rb_extra_info::try_collision_prolog()
{
    return nullptr;
}

// ea: 0x7060E0
void rb_extra_info::collision_prolog()
{
    if (try_collision_prolog() == nullptr)
        phys_collision_allocater_ballistic_reinit();
    if (m_gjk_geom_list == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBPropSys.cpp", 173,
                     "m_gjk_geom_list", defaultFileName))
        __debugbreak();
    rb_vehicle* m_rb_vehicle = this->m_rb_vehicle;
    if (m_rb_vehicle != nullptr && m_rb_vehicle->m_vci == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBPropSys.cpp", 175,
                     "m_rb_vehicle->m_vci", defaultFileName))
        __debugbreak();
}

// ea: 0x6F6DA0
void rb_extra_info::set(Entity* const ent, rigid_body* const rb,
                        const math::Mat43& transform, float bs_radius,
                        const math::Position3& bs_center_loc)
{
    (void)bs_radius;
    (void)bs_center_loc;
    m_ent = ent;
    m_rb = rb;
    m_cg_mesh_mat = &ent->r.currentMat;
    m_gjk_geom_list = nullptr;
    memcpy(&m_transform, &transform, sizeof(math::Mat43));
    m_rb_vehicle = nullptr;
    m_flags.mMask |= 1u;
    m_flags.mMask |= 2u;
    m_flags.mMask |= 4u;
    m_time_since_last_event = 0.0f;
}

// ea: 0x6F6C20
void wheel_collision_info::setup(rigid_body* rb,
                                 rigid_body_constraint_wheel* rbc_wheel)
{
    if (rbc_wheel == nullptr
        && _tlAssert("c:\\cod\\code\\game\\vehicle_collision.cpp", 5,
                     "rbc_wheel", defaultFileName))
        __debugbreak();
    m_rbc_wheel = rbc_wheel;
    if ((rb->m_flags & 0x50) == 0
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 109,
                     "debug_flag_is_in_collision()", defaultFileName))
        __debugbreak();
    m_rbc_wheel->get_wheel_collide_segment(rb->m_col_mat, &m_p0, &m_p1);
    m_aabb_mn.v = _mm_min_ps(m_p0.v, m_p1.v);
    m_aabb_mx.v = _mm_max_ps(m_p0.v, m_p1.v);
    m_t = 1.0f;
    m_surface_flags = 0;
    m_did_hit = false;
    m_hit_rb = nullptr;
}

// stub until wheel_collision_info::process (0x6FE3A0) is ported
void wheel_collision_info::process(rb_extra_info* rb_inf, int wheel_i)
{
    (void)rb_inf;
    (void)wheel_i;
}

// stub until rigid_body_constraint_wheel internals are ported (0x884FE0)
void rigid_body_constraint_wheel::get_wheel_collide_segment(
    const math::Mat43& b1_mat, math::Dir3* const p0,
    math::Dir3* const p1) const
{
    (void)b1_mat;
    (void)p0;
    (void)p1;
}

// vehicle_collision_info (physics.o vehicle_collision.cpp)
class vehicle_collision_info {
public:
    wheel_collision_info* m_list_wheel_collision_info;  // +0x00
    int m_list_wheel_collision_info_count;              // +0x04

    void setup(rb_extra_info* rb_inf);  // ?setup@vehicle_collision_info@@QAEXPAVrb_extra_info@@@Z
    void process(rb_extra_info* rb_inf);  // ?process@vehicle_collision_info@@QAEXPAVrb_extra_info@@@Z
};

// ea: 0x6FE630
void vehicle_collision_info::setup(rb_extra_info* rb_inf)
{
    if (rb_inf->m_rb_vehicle == nullptr
        && _tlAssert("c:\\cod\\code\\game\\vehicle_collision.cpp", 66,
                     "rb_inf->m_rb_vehicle", defaultFileName))
        __debugbreak();
    if ((m_list_wheel_collision_info_count <= 0
         || m_list_wheel_collision_info_count > 8)
        && _tlAssert(
               "c:\\cod\\code\\game\\vehicle_collision.cpp", 67,
               "m_list_wheel_collision_info_count > 0 && m_list_wheel_collision_info_count <= MAX_WHEELS",
               defaultFileName))
        __debugbreak();
    if (rb_inf->m_rb_vehicle->m_vci != nullptr
        && _tlAssert("c:\\cod\\code\\game\\vehicle_collision.cpp", 68,
                     "rb_inf->m_rb_vehicle->m_vci == NULL", defaultFileName))
        __debugbreak();
    rb_inf->m_rb_vehicle->m_vci = this;
    rigid_body* rb = rb_inf->m_rb;
    phys_gjk_geom_list* m_gjk_geom_list =
        (phys_gjk_geom_list*)rb_inf->m_gjk_geom_list;
    for (int i = 0; i < m_list_wheel_collision_info_count; ++i)
    {
        wheel_collision_info* v5 = &m_list_wheel_collision_info[i];
        v5->setup(rb,
                  (rigid_body_constraint_wheel*)rb_inf->m_rb_vehicle->m_wheels[i]);
        m_gjk_geom_list->m_aabb_mn.v = _mm_min_ps(
            m_gjk_geom_list->m_aabb_mn.v, v5->m_aabb_mn.v);
        m_gjk_geom_list->m_aabb_mx.v = _mm_max_ps(
            m_gjk_geom_list->m_aabb_mx.v, v5->m_aabb_mx.v);
    }
}

// ea: 0x6FE7B0
vehicle_collision_info* create_vehicle_collision_info(rb_extra_info* rb_inf)
{
    if (rb_inf->m_rb_vehicle == nullptr
        && _tlAssert("c:\\cod\\code\\game\\vehicle_collision.cpp", 90,
                     "rb_inf->m_rb_vehicle", defaultFileName))
        __debugbreak();
    int v1 = 0;
    while (v1 < 8 && rb_inf->m_rb_vehicle->m_wheels[v1] != nullptr)
        ++v1;
    if ((v1 <= 0 || v1 > 8)
        && _tlAssert("c:\\cod\\code\\game\\vehicle_collision.cpp", 92,
                     "wheel_count > 0 && wheel_count <= MAX_WHEELS",
                     defaultFileName))
        __debugbreak();
    vehicle_collision_info* v3 = (vehicle_collision_info*)
        g_collision_memory_allocater.allocate(
            8, 4, false, "phys_collision_allocater overflow.");
    if (v3 == nullptr)
        return nullptr;
    v3->m_list_wheel_collision_info = (wheel_collision_info*)
        g_collision_memory_allocater.allocate(
            112 * v1, 16, false, "phys_collision_allocater overflow.");
    if (v3->m_list_wheel_collision_info == nullptr)
        return nullptr;
    v3->m_list_wheel_collision_info_count = v1;
    v3->setup(rb_inf);
    return v3;
}

// ea: 0x6FE770
void vehicle_collision_info::process(rb_extra_info* rb_inf)
{
    int v3 = 0;
    if (m_list_wheel_collision_info_count > 0)
    {
        int v4 = 0;
        do
            m_list_wheel_collision_info[v4++].process(rb_inf, v3++);
        while (v3 < m_list_wheel_collision_info_count);
    }
}

// gjk_geom_database (physics.o; minimal view for ballistic reinit)
struct gjk_geom_database {
    void* m_tree_root;      // +0x00 (m_ggi_search_tree.m_tree_root)
    int   m_terrain_count;  // +0x04
    int   m_entity_count;   // +0x08
    int   m_actor_count;    // +0x0C
    int   m_patch_count;    // +0x10
    int   m_brush_count;    // +0x14
    int   m_aabb_count;     // +0x18
};
// ?g_gjk_geom_database@@3PAUgjk_geom_database@@A (physics.o data @ 0xF79488)
gjk_geom_database* g_gjk_geom_database = nullptr;

// ?g_in_physics_collision_callback@@3_NA (physics.o data @ 0xF79475)
bool g_in_physics_collision_callback = false;
// ?gPhysicsFrameAdvance@@3_NA (physics.o data @ 0xF79474)
bool gPhysicsFrameAdvance = false;
// ?g_physics_memory_buffer@@3PADA (physics.o data @ 0xF79478)
char* g_physics_memory_buffer = nullptr;

// phys_gjk_cache_system_avl_tree<N> (physics.o; update_cache stub)
template <int N>
class phys_gjk_cache_system_avl_tree {
public:
    void update_cache();  // ?update_cache@?$phys_gjk_cache_system_avl_tree@$0BPE@@@QAEXXZ
    phys_gjk_cache_info* get_gjk_cache_info(
        unsigned int id1, unsigned int id2,
        bool no_error);  // ?get_gjk_cache_info@?$phys_gjk_cache_system_avl_tree@$0BPE@@@QAEPAUphys_gjk_cache_info@@II_N@Z
};
// stub until the gjk cache tree is ported (physics.o inline 0xB0D6C0)
template <int N>
void phys_gjk_cache_system_avl_tree<N>::update_cache()
{
}
// stub until the gjk cache tree is ported (physics.o inline 0xB0D590)
template <int N>
phys_gjk_cache_info* phys_gjk_cache_system_avl_tree<N>::get_gjk_cache_info(
    unsigned int id1, unsigned int id2, bool no_error)
{
    (void)id1;
    (void)id2;
    (void)no_error;
    return nullptr;
}
// ?g_phys_gjk_cache_system@@3V?$phys_gjk_cache_system_avl_tree@$0BPE@@@A
// (physics.o data @ 0xF794D0)
phys_gjk_cache_system_avl_tree<500> g_phys_gjk_cache_system;

// ea: 0x707100
void set_gjk_cache_info(phys_collide_data* d)
{
    d->gjk_ci = g_phys_gjk_cache_system.get_gjk_cache_info(
        d->id1, d->id2, true);
}

// ea: 0x6FF0E0
void prop_system_collision_epilog()
{
    int count = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
    {
        rb_extra_info* v2 = g_list_rb_extra_info.m_alloc_list[i];
        v2->m_gjk_geom_list = nullptr;
        rb_vehicle* m_rb_vehicle = v2->m_rb_vehicle;
        if (m_rb_vehicle != nullptr)
            m_rb_vehicle->m_vci = nullptr;
    }
}

// ea: 0x707130
void prop_system_collision_prolog()
{
    int count = g_list_rb_extra_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
    {
        if (g_list_rb_extra_info.m_alloc_list[i]->try_collision_prolog()
                == nullptr
            && _tlAssert("c:\\cod\\code\\game\\RBCollision.cpp", 361,
                         "gjk_geom_list", defaultFileName))
            __debugbreak();
    }
}

// ea: 0x6F7060
void phys_collision_allocater_ballistic_reinit()
{
    for (int i = 0; i < g_collision_memory_allocater.m_num_buffers; ++i)
        g_collision_memory_allocater.m_list_memory_buffer[i].m_buffer_cur =
            g_collision_memory_allocater.m_list_memory_buffer[i].m_user_start;
    gjk_geom_database* v2 = g_gjk_geom_database;
    v2->m_tree_root = nullptr;
    v2->m_terrain_count = 0;
    v2->m_entity_count = 0;
    v2->m_actor_count = 0;
    v2->m_patch_count = 0;
    v2->m_brush_count = 0;
    v2->m_aabb_count = 0;
    prop_system_collision_epilog();
    prop_system_collision_prolog();
}

// stub until collision_memory_prolog (0x702CB0) is ported
void collision_memory_prolog()
{
}
// prop_phys_collision (physics.o; static collision pass helpers)
struct prop_phys_collision {
    static void get_all_collisions();  // ?get_all_collisions@prop_phys_collision@@SAXXZ
};
// stub until prop_phys_collision::get_all_collisions (0x70AFE0) is ported
void prop_phys_collision::get_all_collisions()
{
}
// stub until process_prop_collide_callbacks (0x702740) is ported
void process_prop_collide_callbacks()
{
}
void do_all_biped_system_collision_callback();  // 0x70C8D0
void do_all_biped_system_process_collision_events();  // 0x704A10

// Collision-memory globals (physics.o data @ 0xF7947C..0xF79498). Typed to
// match the binary V/U-tag manglings.
class phys_gjk_info;
class phys_contact_manifold_process;
class TouchEntityData;
class EntityHandleDb;
template <typename Db, typename T>
class DbLinkedHandle {
public:
    Handle mHandle;  // +0x00
};
int CM_AreaEntities(const math::Position3& mins, const math::Position3& maxs,
                    DbLinkedHandle<EntityHandleDb, Entity>* entityList,
                    int maxcount, int contentmask);  // game.o

// TouchEntityData (g_local.h view; local copy - 0x230 bytes)
class TouchEntityData {
public:
    int   num;  // +0x00
    uint8_t _pad4[0x10 - 0x04];
    math::Position3 mins;  // +0x10
    math::Position3 maxs;  // +0x20
    DbLinkedHandle<EntityHandleDb, Entity> touch[128];  // +0x30
};

// subdivision_visitor (g_local.h view; local copy)
enum visit_result_t { CONTINUE_VISITING = 0 };
struct subdivision_visitor {
    virtual visit_result_t visit(int cluster_offset) = 0;
};

// rtree types (g_local.h / g_rtree.cpp views; local copies)
struct rtree_node_t {
    int16_t minx;  // +0x00
    int16_t maxx;  // +0x02
    int16_t miny;  // +0x04
    int16_t maxy;  // +0x06
    int16_t minz;  // +0x08
    int16_t maxz;  // +0x0A
    int     offs;  // +0x0C
};
struct rtree_root_t {
    math::Position3 region_center;        // +0x00
    math::Position3 region_halfsize_inv32k;  // +0x10
    rtree_node_t*   simd_tree;            // +0x20
    void*           simd_pointer_base;    // +0x24
    int             top_level_aabb_count; // +0x28
    int             nsimd_levels;         // +0x2C
};

// CGBank (g_local.h view; rtree_root only)
class CGBank {
public:
    uint8_t _pad0[0x90];
    rtree_root_t rtree_root;  // +0x90
};

// CGBankManager (g_local.h view; minimal - sInst is port-local void*)
class CGBankManager {
public:
    static void* sInst;  // ?sInst@CGBankManager@@2PAXA (port-local)
    uint8_t _pad4[0x0C - 0x04];
    int    mCount;       // +0x0C
    CGBank* mBankArray[99];  // +0x10
};

// physics_colgeom_visitor (physics.o; subclass of subdivision_visitor)
struct physics_colgeom_visitor : subdivision_visitor {
    uint8_t _pad4[0x804 - 0x04];
    int     m_object_list_count;  // +0x804
    CGBank* m_cur_bank;           // +0x808
    int     m_mask;               // +0x80C

    visit_result_t visit(int cluster_offset);  // physics.o inline 0xB08E20
};

// stub until physics_colgeom_visitor::visit (0xB08E20) is ported
visit_result_t physics_colgeom_visitor::visit(int cluster_offset)
{
    (void)cluster_offset;
    return CONTINUE_VISITING;
}

// traverse_rtree (physics.o; ported in g_rtree.cpp)
void traverse_rtree(const math::Position3& p0, const math::Position3& p1,
                    const rtree_root_t& root, subdivision_visitor& visitor);

struct physics_colgeom_visitor;
class prop_collide_callback;
template <typename K, typename V> class phys_inplace_avl_tree;
template <typename K, typename V>
class phys_inplace_avl_tree {
public:
    void* m_tree_root;  // +0x00
};
phys_gjk_info* g_gjk_info = nullptr;                         // ?g_gjk_info@@3PAVphys_gjk_info@@A
phys_contact_manifold_process* g_cman_process = nullptr;     // ?g_cman_process@@3PAVphys_contact_manifold_process@@A
phys_collide_data* g_list_phys_collide_data = nullptr;       // ?g_list_phys_collide_data@@3PAVphys_collide_data@@A
physics_colgeom_visitor* g_physics_colgeom_visitor = nullptr;  // ?g_physics_colgeom_visitor@@3PAUphysics_colgeom_visitor@@A
TouchEntityData* g_phys_touch_entity_data = nullptr;         // ?g_phys_touch_entity_data@@3PAVTouchEntityData@@A
prop_collide_callback* g_list_prop_collide_callback = nullptr;  // ?g_list_prop_collide_callback@@3PAVprop_collide_callback@@A
int   g_list_prop_collide_callback_count = 0;       // ?g_list_prop_collide_callback_count@@3HA
phys_inplace_avl_tree<rigid_body_pair_key, prop_collide_callback>*
    g_prop_collide_callback_database = nullptr;  // ?g_prop_collide_callback_database@@3V?$phys_inplace_avl_tree@Vrigid_body_pair_key@@Vprop_collide_callback@@@@A
extern bool tlScratchpadLocked;  // ?tlScratchpadLocked@@3_NA (tl_system.o)

// ea: 0x6F30F0
TouchEntityData* generate_local_entities_list(const math::Dir3& aabb_mn,
                                              const math::Dir3& aabb_mx,
                                              int mask)
{
    g_phys_touch_entity_data->mins.v = aabb_mn.v;
    g_phys_touch_entity_data->maxs.v = aabb_mx.v;
    g_phys_touch_entity_data->num = CM_AreaEntities(
        g_phys_touch_entity_data->mins, g_phys_touch_entity_data->maxs,
        g_phys_touch_entity_data->touch, 128, mask);
    return g_phys_touch_entity_data;
}

// ea: 0x6F70B0
physics_colgeom_visitor* generate_local_primitive_list(
    const math::Dir3& aabb_mn, const math::Dir3& aabb_mx, int mask)
{
    physics_colgeom_visitor* v4 = g_physics_colgeom_visitor;
    g_physics_colgeom_visitor->m_object_list_count = 0;
    v4->m_cur_bank = nullptr;
    v4->m_mask = mask;
    CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
    int mCount = mgr->mCount;
    for (int v5 = 0; v5 < mCount; ++v5)
    {
        if (v5 > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        g_physics_colgeom_visitor->m_cur_bank = mgr->mBankArray[v5];
        math::Position3 v9;
        v9.v = aabb_mx.v;
        math::Position3 v8;
        v8.v = aabb_mn.v;
        traverse_rtree(v8, v9,
                       g_physics_colgeom_visitor->m_cur_bank->rtree_root,
                       *g_physics_colgeom_visitor);
    }
    return g_physics_colgeom_visitor;
}

// ea: 0x6F6D20
void collision_memory_epilog()
{
    g_gjk_info = nullptr;
    g_cman_process = nullptr;
    g_list_phys_collide_data = nullptr;
    g_gjk_geom_database = nullptr;
    g_physics_colgeom_visitor = nullptr;
    g_phys_touch_entity_data = nullptr;
    g_list_prop_collide_callback = nullptr;
    g_collision_memory_allocater.nullify_buffer();
    if (!tlScratchpadLocked
        && _tlAssert("c:/cod/code/tl/base/include\\tl_system.h", 300,
                     "tlScratchpadLocked",
                     "Scratchpad is already unlocked!"))
        __debugbreak();
    tlScratchpadLocked = false;
    g_list_prop_collide_callback_count = 0;
    if (g_prop_collide_callback_database != nullptr)
        g_prop_collide_callback_database->m_tree_root = nullptr;
}

// ea: 0x70CFA0
void physics_collision_callback()
{
    g_in_physics_collision_callback = true;
    collision_memory_prolog();
    prop_system_collision_prolog();
    prop_phys_collision::get_all_collisions();
    do_all_biped_system_collision_callback();
    process_prop_collide_callbacks();
    do_all_biped_system_process_collision_events();
    prop_system_collision_epilog();
    collision_memory_epilog();
    g_in_physics_collision_callback = false;
}


// ?g_rb_vehicle_list@@3V?$phys_static_memory_pool@Vrb_vehicle@@$09@@A
// (physics.o data @ 0xE2B8F0)
phys_static_memory_pool<rb_vehicle, 10> g_rb_vehicle_list;
// ?g_list_rb_extra_info@@3V?$phys_static_memory_pool@Vrb_extra_info@@$0CD@@@A
// (physics.o data @ 0xE2A000)
phys_static_memory_pool<rb_extra_info, 35> g_list_rb_extra_info;
// ea: 0x6F6AC0 - fatal trap; x_0 is an unnamed physics.o data global
// (0xF916B8) referenced only from here.
static volatile unsigned int x_0 = 0;
void absolutely_fatal_irrecoverable_error_infinite_loop()
{
    for (;;)
        ++x_0;
}

struct phys_anim_bone_array {
    static void copy_skeleton(Entity* owner, math::Mat43* const skeleton_pose);
    static void write_skeleton(Entity* owner, math::Mat43* const skeleton_pose);
    void copy_back_bones(Entity* owner);  // ?copy_back_bones@phys_anim_bone_array@@QAEXPAVEntity@@@Z
    void remove_rigid_body(int rb_index);  // ?remove_rigid_body@phys_anim_bone_array@@QAEXH@Z
    void copy_back_tween(Entity* owner, float t_);  // ?copy_back_tween@phys_anim_bone_array@@QAEXPAVEntity@@M@Z
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

// biped_system (physics.o RBRagdoll.cpp) - rb_ragdoll_model subclass.
// Offsets from create_system (0x6F44F0, m_bp_info +0x2040) and
// initialize_members (0x6F4500, m_stable_timer +0x2048, m_is_stable +0x204C).
class biped_phys_info;
class biped_system : public rb_ragdoll_model {
public:
    phys_anim_bone_array bp_bone_array;  // +0x130
    uint8_t _pad131[0xC90 - 0x130 - sizeof(phys_anim_bone_array)];
    ragdoll_collision_callback m_collision_callback;  // +0xC90
    uint8_t _padC94[0x2040 - 0xC94];
    biped_phys_info* m_bp_info;   // +0x2040
    int              m_flags;         // +0x2044
    float            m_stable_timer;  // +0x2048
    bool             m_is_stable;     // +0x204C
    float            m_tween_time;    // +0x2050
    float            m_tween_duration;  // +0x2054

    void prolog_frame_advance(Entity* owner, float delta_t);  // ?prolog_frame_advance@biped_system@@QAEXPAVEntity@@M@Z
    void debug_render();  // ?debug_render@biped_system@@QAEXXZ
    void render_joint(int joint_id, Bitmask<unsigned int> render_flags);  // ?render_joint@biped_system@@QAEXHV?$Bitmask@I@@@Z
    void destroy_bps(Entity* owner);  // ?destroy_bps@biped_system@@QAEXPAVEntity@@@Z
    void recreate_bps(Entity* owner, int flags);  // ?recreate_bps@biped_system@@QAEXPAVEntity@@H@Z
    void remove_rigid_body(phys_bones rb_id);  // ?remove_rigid_body@biped_system@@QAEXW4phys_bones@@@Z
    void create_bps(Entity* owner, int flags);  // ?create_bps@biped_system@@QAEXPAVEntity@@H@Z
    void epilog_frame_advance(Entity* owner, float delta_t);  // ?epilog_frame_advance@biped_system@@QAEXPAVEntity@@M@Z
private:
    void initialize_members();  // ?initialize_members@biped_system@@AAEXXZ
    void create_system(biped_phys_info* bp_info);  // ?create_system@biped_system@@AAEXPAVbiped_phys_info@@@Z
    void rdbi_calc_bone_mat_from_rb();  // ?rdbi_calc_bone_mat_from_rb@biped_system@@AAEXXZ
    void update_stability(float delta_t);  // ?update_stability@biped_system@@AAEXM@Z
    void setup_physics(Entity* owner);     // ?setup_physics@biped_system@@AAEXPAVEntity@@@Z
    friend class biped_phys_info;
};

// biped_phys_info (physics.o RBRagdoll.cpp). Layout verified against ctor
// (0x6F71C0), destroy_bp_sys (0x70D080), debug_render (0x6FF7F0) and
// update_bone_vel_info (0x6F3EB0) disassembly: m_owner +0x00, m_cur_mat
// +0x10, m_last_mat +0x290, m_bone +0x550, m_delta_t +0x564, m_bp_sys +0x568,
// m_current_debug_joint +0x56C, m_render_flags +0x570. class tag (V).
class biped_phys_info {
public:
    Entity*        m_owner;         // +0x00
    uint8_t        _pad04[0x10 - 0x04];  // align 16
    math::Mat43    m_cur_mat[10];   // +0x10
    math::Mat43    m_last_mat[10];  // +0x290
    math::Position3 m_cur_angles;    // +0x510
    math::Position3 m_cur_origin;    // +0x520
    math::Position3 m_last_angles;   // +0x530
    math::Position3 m_last_origin;   // +0x540
    int16_t      m_bone[10];         // +0x550
    float        m_delta_t;          // +0x564
    biped_system* m_bp_sys;          // +0x568
    int          m_current_debug_joint;  // +0x56C
    Bitmask<unsigned int> m_render_flags;  // +0x570

    biped_phys_info();  // ??0biped_phys_info@@QAE@XZ
    ~biped_phys_info();  // ??1biped_phys_info@@QAE@XZ
    void get_cur_vel(int rb_id, const math::Position3& com,
                     math::Dir3* cur_tvel, math::Dir3* cur_avel);
    void destroy_bp_sys(bool tween_pos);  // ?destroy_bp_sys@biped_phys_info@@QAEX_N@Z
    void debug_render();  // ?debug_render@biped_phys_info@@QAEXXZ
    static void debug_render_all();  // ?debug_render_all@biped_phys_info@@SAXXZ
    static void prolog_frame_advance_all(float delta_t);  // ?prolog_frame_advance_all@biped_phys_info@@SAXM@Z
    static void epilog_frame_advance_all(float delta_t);  // ?epilog_frame_advance_all@biped_phys_info@@SAXM@Z
    void epilog_frame_advance(float delta_t);  // ?epilog_frame_advance@biped_phys_info@@QAEXM@Z
private:
    void reset_bone_vel_info(float delta_t);   // ?reset_bone_vel_info@biped_phys_info@@AAEXM@Z
    void update_bone_vel_info(float delta_t);  // ?update_bone_vel_info@biped_phys_info@@AAEXM@Z
    bool setup(Entity* owner);                 // ?setup@biped_phys_info@@AAE_NPAVEntity@@@Z
    friend biped_phys_info* create_biped_phys_info(Entity* owner);
public:
    void update_vel_matrices();                // ?update_vel_matrices@biped_phys_info@@QAEXXZ
    void prolog_frame_advance(float delta_t);  // ?prolog_frame_advance@biped_phys_info@@QAEXM@Z
};

biped_phys_info* create_biped_phys_info(Entity* owner);  // ?create_biped_phys_info@@YAPAVbiped_phys_info@@PAVEntity@@@Z
void destroy_biped_phys_info(biped_phys_info* bp_info);  // ?destroy_biped_phys_info@@YAXPAVbiped_phys_info@@@Z

// ?g_list_biped_system@@3V?$phys_static_memory_pool@Vbiped_system@@$0BA@@@A
// (physics.o data @ 0xE09970)
extern phys_static_memory_pool<biped_system, 16> g_list_biped_system;

// cdl_proftimer (cdl_common.o view; local copy)
struct cdl_proftimer {
    void start();  // ?start@cdl_proftimer@@QAEXXZ
    void stop();   // ?stop@cdl_proftimer@@QAEXXZ
};
// ?cdl_proftimer_update_rb@@3Ucdl_proftimer@@A (game.o data @ 0x01334968)
extern cdl_proftimer cdl_proftimer_update_rb;

// stub until RBAdvanceDebug (0x704290) is ported
void RBAdvanceDebug(float delta_t)
{
    (void)delta_t;
}

// stub until rb_prop_system::frame_advance (0x7032B0) is ported
void rb_prop_system::frame_advance(float delta_t)
{
    (void)delta_t;
}

// ea: 0x70D250
void UpdateRigidBody(float delta_t)
{
    char solver_memory_buffer[76480];
    cdl_proftimer_update_rb.start();
    if (delta_t >= 0.1f)
        delta_t = 0.1f;
    phys_sys::solver_memory_buffer_set(solver_memory_buffer,
                                       0);  // dword_12AC0 (.textbss = 0)
    g_physics_memory_buffer = solver_memory_buffer;
    RBAdvanceDebug(delta_t);
    biped_phys_info::prolog_frame_advance_all(delta_t);
    rb_vehicle::frame_prolog_all_systems(delta_t);
    gPhysicsFrameAdvance = true;
    phys_sys::phys_frame_advance(delta_t);
    gPhysicsFrameAdvance = false;
    rb_vehicle::frame_epilog_all_systems(delta_t);
    rb_prop_system::frame_advance(delta_t);
    biped_phys_info::epilog_frame_advance_all(delta_t);
    g_physics_memory_buffer = nullptr;
    phys_sys::solver_memory_buffer_nullify();
    g_phys_gjk_cache_system.update_cache();
    cdl_proftimer_update_rb.stop();
}

// ea: 0x70CFE0
void PhysInit()
{
    phys_mem_info pmi;
    pmi.m_num_user_rigid_body = 10;
    pmi.m_num_rbc_custom_orientation = 10;
    pmi.m_num_rbc_custom_path = 10;
    pmi.m_num_rigid_body = 195;
    pmi.m_contact_point_buffer_size = 0;  // dword_235B0 (.textbss = 0)
    pmi.m_num_rbc_contact = 431;
    pmi.m_num_rbc_point = 12;
    pmi.m_num_rbc_ragdoll = 144;
    pmi.m_num_rbc_angular_actuator = 144;
    pmi.m_num_rbc_hinge = 8;
    pmi.m_num_rbc_wheel = 60;
    pmi.m_num_rbc_dist = 8;
    phys_sys::phys_init(&pmi);
    phys_sys::set_collision_callback(physics_collision_callback);
    DebugRender::sInst.AddRenderer(biped_phys_info::debug_render_all);
    phys_sys::set_max_delta_t(0.051282052f);
    phys_sys::set_v_tol(8, 100);
}

// ea: 0x70C8D0
void do_all_biped_system_collision_callback()
{
    int count = g_list_biped_system.m_alloc_count;
    for (int i = 0; i < count; ++i)
        g_list_biped_system.m_alloc_list[i]->m_collision_callback
            .get_all_collisions();
}

// ea: 0x704A10
void do_all_biped_system_process_collision_events()
{
    int count = g_list_biped_system.m_alloc_count;
    for (int i = 0; i < count; ++i)
        g_list_biped_system.m_alloc_list[i]->m_collision_callback
            .process_environment_collision_events();
}

// ea: 0x6F71C0
biped_phys_info::biped_phys_info()
{
    m_render_flags.mMask = 0;
    m_owner = nullptr;
    m_bp_sys = nullptr;
}

// ?g_list_biped_phys_info@@3V?$phys_static_memory_pool@Vbiped_phys_info@@$0BA@@@A
// (physics.o data @ 0xE040E0)
phys_static_memory_pool<biped_phys_info, 16> g_list_biped_phys_info;
// ?g_list_biped_system@@3V?$phys_static_memory_pool@Vbiped_system@@$0BA@@@A
// (physics.o data @ 0xE09970)
phys_static_memory_pool<biped_system, 16> g_list_biped_system;

// ?gRenderFlags@@3PAHA / ?gJoints@@3PAHA (game2.o data @ 0x012F3EF0/0x012F3EC8)
// Debug render arrays; binary initializes both pointers to -1 (disabled).
int* gRenderFlags = (int*)-1;
int* gJoints = (int*)-1;

// ea: 0x70D140
biped_phys_info::~biped_phys_info()
{
    if (m_bp_sys != nullptr)
        destroy_bp_sys(false);
}

// ea: 0x70D080
void biped_phys_info::destroy_bp_sys(bool tween_pos)
{
    (void)tween_pos;
    biped_system* m_bp_sys_ = m_bp_sys;
    if (m_bp_sys_ != nullptr)
    {
        Entity* m_owner_ = m_owner;
        m_bp_sys_->rdbi_calc_bone_mat_from_rb();
        m_bp_sys_->bp_bone_array.copy_back_bones(m_owner_);
        g_list_biped_system.remove(m_bp_sys_);
        m_bp_sys = nullptr;
        m_owner_->physicsObject = 0;
    }
}

// ea: 0x70D350
void destroy_biped_phys_info(biped_phys_info* bp_info)
{
    if (bp_info->m_bp_sys != nullptr)
        bp_info->destroy_bp_sys(false);
    g_list_biped_phys_info.remove(bp_info);
}

// ea: 0x70D300
biped_phys_info* create_biped_phys_info(Entity* owner)
{
    biped_phys_info* mBPInfo = owner->mBPInfo;
    if (mBPInfo == nullptr)
    {
        mBPInfo = g_list_biped_phys_info.add(
            true, "phys memory pool add overflow.");
        if (mBPInfo == nullptr)
            return nullptr;
    }
    if (!mBPInfo->setup(owner))
    {
        g_list_biped_phys_info.remove(mBPInfo);
        return nullptr;
    }
    return mBPInfo;
}

// ea: 0x7049E0
void biped_phys_info::debug_render_all()
{
    int count = g_list_biped_phys_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
        g_list_biped_phys_info.m_alloc_list[i]->debug_render();
}

// ea: 0x6FF7B0
void biped_phys_info::prolog_frame_advance_all(float delta_t)
{
    int count = g_list_biped_phys_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
        g_list_biped_phys_info.m_alloc_list[i]->prolog_frame_advance(delta_t);
}

// stub until biped_phys_info::epilog_frame_advance (0x70D160) is ported
void biped_phys_info::epilog_frame_advance(float delta_t)
{
    (void)delta_t;
}

// ea: 0x70D210
void biped_phys_info::epilog_frame_advance_all(float delta_t)
{
    int count = g_list_biped_phys_info.m_alloc_count;
    for (int i = 0; i < count; ++i)
        g_list_biped_phys_info.m_alloc_list[i]->epilog_frame_advance(delta_t);
}

// ea: 0x6FF7F0
void biped_phys_info::debug_render()
{
    if (m_bp_sys != nullptr)
    {
        for (int i = 0; i < 10; ++i)
        {
            if (gRenderFlags[i] != 0)
                m_render_flags.Add(i);
            else
                m_render_flags.Rmv(i);
        }
        for (int i = 0; i < 10; ++i)
        {
            if (gJoints[i] != 0)
                m_bp_sys->render_joint(i, m_render_flags);
        }
    }
}

// stub until phys_anim_bone_array internals are ported (physics.o 0x6F76A0)
void phys_anim_bone_array::copy_back_bones(Entity* owner)
{
    (void)owner;
}

// stub until phys_anim_bone_array internals are ported (physics.o 0x6F8EF0)
void phys_anim_bone_array::remove_rigid_body(int rb_index)
{
    (void)rb_index;
}

// stub until biped_system internals are ported (physics.o inline 0x719FA0)
void biped_system::rdbi_calc_bone_mat_from_rb()
{
}

// stub until biped_system internals are ported (physics.o 0x6FB610)
void biped_system::render_joint(int joint_id,
                                Bitmask<unsigned int> render_flags)
{
    (void)joint_id;
    (void)render_flags;
}

// ea: 0x6FAA40
void biped_system::destroy_bps(Entity* owner)
{
    rdbi_calc_bone_mat_from_rb();
    bp_bone_array.copy_back_bones(owner);
}

// ea: 0x6F4530
void biped_system::recreate_bps(Entity* owner, int flags)
{
    (void)owner;
    m_flags = flags;
    rb_ragdoll_model::remove_all_user_rigid_body();
    rb_ragdoll_model::reset_stability();
    rb_ragdoll_model::reset_ballistic_target();
    m_is_stable = false;
    m_stable_timer = 0.0f;
}

// ea: 0x70B2D0
void biped_system::remove_rigid_body(phys_bones rb_id)
{
    bp_bone_array.remove_rigid_body(rb_id);
    m_collision_callback.remove_colgeom(rb_id);
    rb_ragdoll_model::remove_rigid_body(rb_id);
}

// stub until biped_system::setup_physics (0x707180) is ported
void biped_system::setup_physics(Entity* owner)
{
    (void)owner;
}

// stub until biped_system::update_stability (0x6FAA60) is ported
void biped_system::update_stability(float delta_t)
{
    (void)delta_t;
}

// stub until phys_anim_bone_array::copy_back_tween (0x6F79B0) is ported
void phys_anim_bone_array::copy_back_tween(Entity* owner, float t_)
{
    (void)owner;
    (void)t_;
}

// ea: 0x708D50
void biped_system::create_bps(Entity* owner, int flags)
{
    m_flags = flags;
    setup_physics(owner);
    m_tween_time = 0.0f;
    m_tween_duration = 0.5f;
    rb_ragdoll_model::reset_stability();
    rb_ragdoll_model::reset_ballistic_target();
    m_is_stable = false;
    m_stable_timer = 0.0f;
    owner->Notify(hash_const.physicsstart);
}

// ea: 0x6FACB0
void biped_system::epilog_frame_advance(Entity* owner, float delta_t)
{
    update_stability(delta_t);
    rdbi_calc_bone_mat_from_rb();
    float v4 = delta_t + m_tween_time;
    float m_tween_duration_ = m_tween_duration;
    if (m_tween_duration_ <= v4)
    {
        bp_bone_array.copy_back_bones(owner);
    }
    else
    {
        m_tween_time = v4;
        bp_bone_array.copy_back_tween(owner, v4 / m_tween_duration_);
    }
    HelmetController(owner);
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
    DObj* mDObj = owner->mDObj;
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

// ea: 0x6F3CF0
void biped_phys_info::reset_bone_vel_info(float delta_t)
{
    if (m_owner == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 333, "m_owner",
                     defaultFileName))
        __debugbreak();
    m_delta_t = delta_t;
    for (int i = 0; i < 10; ++i)
    {
        if (m_bone[i] < 0
            && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 337,
                         "m_bone[i] >= 0", defaultFileName))
            __debugbreak();
        math::Mat43 mat = m_owner->CalcAbsMat(m_bone[i]);
        memcpy(&m_last_mat[i], &mat, sizeof(math::Mat43));
        memcpy(&m_cur_mat[i], &mat, sizeof(math::Mat43));
    }
}

// ea: 0x6F3EB0
void biped_phys_info::update_bone_vel_info(float delta_t)
{
    if (m_owner == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 346, "m_owner",
                     defaultFileName))
        __debugbreak();
    m_delta_t = delta_t;
    for (int i = 0; i < 10; ++i)
    {
        if (m_bone[i] < 0
            && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 350,
                         "m_bone[i] >= 0", defaultFileName))
            __debugbreak();
        memcpy(&m_last_mat[i], &m_cur_mat[i], sizeof(math::Mat43));
        math::Mat43 rel = m_owner->GetRelMat(m_bone[i]);
        memcpy(&m_cur_mat[i], &rel, sizeof(math::Mat43));
    }
    m_last_origin.v = m_cur_origin.v;
    m_last_angles.v = m_cur_angles.v;
    // refresh cur origin/angles from owner's render state
    const float* curOrigin = (const float*)((const char*)m_owner + 0x150);
    m_cur_origin.v = _mm_loadu_ps(curOrigin);
    const float* curAngles = (const float*)((const char*)m_owner + 0x160);
    m_cur_angles.v = _mm_loadu_ps(curAngles);
}

// ea: 0x6F7620
void biped_phys_info::prolog_frame_advance(float delta_t)
{
    if (m_owner == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 181, "m_owner",
                     defaultFileName))
        __debugbreak();
    m_owner->CalcRotTranMat43();
    update_bone_vel_info(delta_t);
    if (m_bp_sys != nullptr)
        m_bp_sys->update_ballistic_target();
}

// ea: 0x6F45F0
void biped_system::debug_render()
{
}

// ea: 0x6F44F0
void biped_system::create_system(biped_phys_info* bp_info)
{
    m_bp_info = bp_info;
}

// ea: 0x6F4500
void biped_system::initialize_members()
{
    rb_ragdoll_model::reset_stability();
    rb_ragdoll_model::reset_ballistic_target();
    m_is_stable = false;
    m_stable_timer = 0.0f;
}

// ea: 0x6F45E0
void biped_system::prolog_frame_advance(Entity* owner, float delta_t)
{
    (void)owner;
    (void)delta_t;
    rb_ragdoll_model::update_ballistic_target();
}

// USER_BONE_ID name table (physics.o .rdata; 8 entries)
static const char* const s_user_bone_names[8] = {
    "Bip01 Head",        // USER_BONE_ID_HEAD
    "Bip01 L UpperArm",  // USER_BONE_ID_LEFT_UPPERARM
    "Bip01 L Forearm",   // USER_BONE_ID_LEFT_FOREARM
    "Bip01 R UpperArm",  // USER_BONE_ID_RIGHT_UPPERARM
    "Bip01 R Forearm",   // USER_BONE_ID_RIGHT_FOREARM
    "Bip01 L Thigh",     // USER_BONE_ID_LEFT_THIGH
    "Bip01 L Calf",      // USER_BONE_ID_LEFT_CALF
    "Bip01 R Thigh",     // USER_BONE_ID_RIGHT_THIGH
};

// ea: 0x6F71E0
bool biped_phys_info::setup(Entity* owner)
{
    m_owner = owner;
    m_delta_t = 0.033333335f;
    owner->CalcRotTranMat43();
    bool success = true;
    for (int i = 0; i < 8; ++i)
    {
        int BoneIndex = m_owner->mDObj->GetBoneIndex(s_user_bone_names[i]);
        m_bone[i] = (int16_t)BoneIndex;
        if (BoneIndex >= 0)
        {
            math::Mat43 mat = m_owner->CalcAbsMat(BoneIndex);
            memcpy(&m_last_mat[i], &mat, sizeof(math::Mat43));
            memcpy(&m_cur_mat[i], &mat, sizeof(math::Mat43));
        }
        else
        {
            math::Mat43 ident = {};
            ident.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
            ident.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
            ident.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
            ident.w.v = _mm_setzero_ps();
            memcpy(&m_last_mat[i], &ident, sizeof(math::Mat43));
            memcpy(&m_cur_mat[i], &ident, sizeof(math::Mat43));
            success = false;
        }
    }
    const float* curOrigin = (const float*)((const char*)m_owner + 0x150);
    m_cur_origin.v = _mm_loadu_ps(curOrigin);
    const float* curAngles = (const float*)((const char*)m_owner + 0x160);
    m_cur_angles.v = _mm_loadu_ps(curAngles);
    m_last_origin.v = m_cur_origin.v;
    m_last_angles.v = m_cur_angles.v;
    return success;
}

// Mat43 row-vector multiply: result = A * B (rows of A transformed by B).
static math::Mat43 MulMat43Row(const math::Mat43& A, const math::Mat43& B)
{
    math::Mat43 r;
    r.x.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A.x.v, A.x.v, 0), B.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(A.x.v, A.x.v, 85), B.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(A.x.v, A.x.v, 170), B.z.v));
    r.y.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A.y.v, A.y.v, 0), B.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(A.y.v, A.y.v, 85), B.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(A.y.v, A.y.v, 170), B.z.v));
    r.z.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A.z.v, A.z.v, 0), B.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(A.z.v, A.z.v, 85), B.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(A.z.v, A.z.v, 170), B.z.v));
    r.w.v = _mm_add_ps(
        _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A.w.v, A.w.v, 0), B.x.v),
                              _mm_mul_ps(_mm_shuffle_ps(A.w.v, A.w.v, 85), B.y.v)),
                   _mm_mul_ps(_mm_shuffle_ps(A.w.v, A.w.v, 170), B.z.v)),
        B.w.v);
    return r;
}

// ea: 0x6F3150
void biped_phys_info::update_vel_matrices()
{
    // World transforms from cur/last angles + origin (AnglesToAxis layout:
    // axis[0..2] = x/y/z rows, 4th lane 0, translation in w).
    float curAxis[3][3], lastAxis[3][3];
    AnglesToAxis(&m_cur_angles, curAxis);
    AnglesToAxis(&m_last_angles, lastAxis);

    math::Mat43 curWorld, lastWorld;
    curWorld.x.v = _mm_loadu_ps(&curAxis[0][0]);
    curWorld.y.v = _mm_loadu_ps(&curAxis[1][0]);
    curWorld.z.v = _mm_loadu_ps(&curAxis[2][0]);
    curWorld.w.v = m_cur_origin.v;
    lastWorld.x.v = _mm_loadu_ps(&lastAxis[0][0]);
    lastWorld.y.v = _mm_loadu_ps(&lastAxis[1][0]);
    lastWorld.z.v = _mm_loadu_ps(&lastAxis[2][0]);
    lastWorld.w.v = m_last_origin.v;

    for (int i = 0; i < 10; ++i)
    {
        if (m_bone[i] < 0
            && _tlAssert("c:\\cod\\code\\game\\RBRagdoll.cpp", 243,
                         "m_bone[i] >= 0", defaultFileName))
            __debugbreak();
        math::Mat43 lm = MulMat43Row(m_last_mat[i], lastWorld);
        math::Mat43 cm = MulMat43Row(m_cur_mat[i], curWorld);
        memcpy(&g_last_mat[i], &lm, sizeof(math::Mat43));
        memcpy(&g_cur_mat[i], &cm, sizeof(math::Mat43));
    }
}

struct entity_path_view {
    scr_vehicle_t* scr_vehicle;  // +0x00
};

// ea: 0x6F5EC0
rigid_body_constraint_custom_path* path_constraint_create(Entity* veh)
{
    if (veh == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp",
                     209, "veh", defaultFileName))
        __debugbreak();
    entity_path_view* ev = (entity_path_view*)veh;
    if (ev->scr_vehicle == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp",
                     210, "veh->scr_vehicle", defaultFileName))
        __debugbreak();
    // mRBVeh + chassis_rbinf + m_rb chain (offsets from g_local.h view)
    void* mRBVeh = ev->scr_vehicle->mRBVeh;
    if (mRBVeh == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp",
                     211, "veh->scr_vehicle->mRBVeh", defaultFileName))
        __debugbreak();
    void* rbinf = *(void**)((char*)mRBVeh + 0x274);
    if (rbinf == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp",
                     212, "veh->scr_vehicle->mRBVeh->get_chassis_rbinf()",
                     defaultFileName))
        __debugbreak();
    rigid_body* m_rb = *(rigid_body**)((char*)rbinf + 0x0);
    if (m_rb == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp",
                     213, "veh->scr_vehicle->mRBVeh->get_chassis_rbinf()->m_rb",
                     defaultFileName))
        __debugbreak();
    user_rigid_body* user_rigid_body = phys_sys::create_user_rigid_body(false);
    rigid_body_constraint_custom_path* rbc_custom_path =
        phys_sys::create_rbc_custom_path(m_rb, user_rigid_body, false);
    if (rbc_custom_path == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp",
                     219, "rbc_custom_path", defaultFileName))
        __debugbreak();
    rbc_custom_path->b1_r_loc.v = _mm_setzero_ps();
    memcpy(&rbc_custom_path->m_path_mat, (const char*)veh + 0x170,
           sizeof(math::Mat43));  // veh->r.currentMat
    rbc_custom_path->m_urb = user_rigid_body;
    user_rigid_body->set(&rbc_custom_path->m_path_mat);
    return rbc_custom_path;
}

// ea: 0x6F5B60
void path_constraint_update(rigid_body_constraint_custom_path* vpc, Entity* veh)
{
    if (vpc == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp",
                     186, "vpc", defaultFileName))
        __debugbreak();
    if (veh == nullptr
        && _tlAssert("c:\\cod\\code\\game\\RBVehicleCustomConstraint.cpp",
                     187, "veh", defaultFileName))
        __debugbreak();
    entity_path_view* ev = (entity_path_view*)veh;
    scr_vehicle_t* scr_vehicle = ev->scr_vehicle;
    math::Position3 origin;
    origin.v.m128_f32[0] = scr_vehicle->pathPos.origin[0];
    origin.v.m128_f32[1] = scr_vehicle->pathPos.origin[1];
    origin.v.m128_f32[2] = scr_vehicle->pathPos.origin[2];
    origin.v.m128_f32[3] = 0.0f;
    math::Position3 angles;
    angles.v.m128_f32[0] = scr_vehicle->pathPos.angles[0];
    angles.v.m128_f32[1] = scr_vehicle->pathPos.angles[1];
    angles.v.m128_f32[2] = scr_vehicle->pathPos.angles[2];
    angles.v.m128_f32[3] = 0.0f;
    AnglesToAxis(angles, origin, vpc->m_path_mat);
    vpc->b1_r_loc.v = _mm_setzero_ps();
    void* mRBVeh = scr_vehicle->mRBVeh;
    if (((rb_vehicle*)mRBVeh)->m_num_colliding_wheels >= 3)
        vpc->b1_r_loc.v.m128_f32[0] = -6.0f;  // 0xC0C00000
}

// ea: 0x70D0D0
void StopPhysics(Entity* e)
{
    if (e->sentient != nullptr)
    {
        biped_phys_info* mBPInfo = e->mBPInfo;
        if (mBPInfo != nullptr)
            mBPInfo->destroy_bp_sys(false);
    }
    else
    {
        scr_vehicle_t* scr_vehicle = (scr_vehicle_t*)e->scr_vehicle;
        if (scr_vehicle != nullptr)
        {
            rb_vehicle* mRBVeh = (rb_vehicle*)scr_vehicle->mRBVeh;
            if (mRBVeh != nullptr)
            {
                mRBVeh->pause_physics(true);
                g_rb_vehicle_list.remove(mRBVeh);
                ((scr_vehicle_t*)e->scr_vehicle)->mRBVeh = nullptr;
            }
        }
        else
        {
            rb_prop_system::remove_entity(e);
        }
    }
}

// ea: 0x6F5B00
int RecalibrateInputCustom(int val, int threshold)
{
    int eax = val < 0 ? -val : val;
    if (eax < threshold)
        return 0;
    eax -= threshold;
    float scale = (float)eax / (float)(0x80 - threshold) * 128.0f;
    int result = (int)scale;
    int sign = val >= 0 ? 1 : -1;  // ecx = (val>=0) + (val>=0) - 1
    return result * sign;
}

// ea: 0x6F6300
void SetModel(Entity* pEnt, const char* modelName)
{
    TPakId mPakId = (TPakId)pEnt->mPakId;
    if (mPakId == PAK_ID_INVALID)
        mPakId = CurPakId();
    G_SetModel(pEnt, modelName, mPakId, 0);
    G_DObjUpdate(pEnt, false);
    pEnt->r.svFlags |= 0x18;
    pEnt->r.contents |= 0x2080;
    g_LinkEntity(pEnt);
}

// ea: 0x6F6360
void MoveGravity(Entity* pEnt, const float* const vVel, float fTotalTime)
{
    trajectory_t* tr = &pEnt->tr;
    tr->trType = 5;  // TR_GRAVITY
    tr->trTime = level.time;
    tr->trDuration = (int)(fTotalTime * 1000.0f);
    tr->trBase[0] = pEnt->r.currentOrigin.v.m128_f32[0];
    tr->trBase[1] = pEnt->r.currentOrigin.v.m128_f32[1];
    tr->trBase[2] = pEnt->r.currentOrigin.v.m128_f32[2];
    tr->trDelta[0] = vVel[0];
    tr->trDelta[1] = vVel[1];
    tr->trDelta[2] = vVel[2];
    if ((tr->trDelta[0] != tr->trDelta[0])
        || (tr->trDelta[1] != tr->trDelta[1])
        || (tr->trDelta[2] != tr->trDelta[2]))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Destructible.cpp";
        AeAssert::gCurrentLine = 111;
        AeAssert::gCurrentExpr =
            "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    math::Position3 result;
    BG_EvaluateTrajectory(tr, level.time, result);
    g_LinkEntity(pEnt);
}

// Destructible (physics.o; minimal view for the DeletePiece family)
class Destructible {
public:
    void DeletePiece(Entity* ent);  // ?DeletePiece@Destructible@@QAEXPAVEntity@@@Z
};

// ea: 0x6F6470
void Destructible::DeletePiece(Entity* ent)
{
    if (ent != nullptr)
    {
        ent->eFlags |= 0x80;
        ent->flags |= 0x400;
        ent->r.contents &= ~1;
        ent->think = 0x0C;
        ent->nextthink = level.time + 1;
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

// Port-local dietable (defined in g_entity_misc.cpp; uses Broc's EHitLocation
// tag + PBM const-qualification; binary's QBM/W4hitLocation_t form is a
// separate pre-existing mismatch).
enum EHitLocation : int;
extern void (*dietable[8])(Entity* self, Entity* inflictor, Entity* attacker,
                           int damage, int mod, int weapon, const float* point,
                           const float* dir, EHitLocation hitLoc);

// ea: 0x6F61C0
void KillEntity(Entity* e)
{
    uint8_t die = e->die;
    if (die != 0 && e->actor != nullptr
        && ((actor_s*)e->actor)->bIsAlive != 0)
    {
        if (die >= 8u)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RBSimpleAPI.cpp";
            AeAssert::gCurrentLine = 15;
            AeAssert::gCurrentExpr = "e->die > 0 && e->die < DIE_MAX";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        int v4 = e->health + 1;
        e->health = -1;
        sentient_s* sentient = (sentient_s*)e->sentient;
        float dir[3] = {0.0f, 0.0f, 1.0f};
        if (sentient != nullptr)
            sentient->bIgnoreMe = 1;
        int v6 = currCl;
        Entity* ea = EntityManager::sInst->GetPlayer(currCl);
        int v9 = e->die;
        Entity* Player = EntityManager::sInst->GetPlayer(v6);
        dietable[v9](
            e, ea, Player, v4, 0, 0,
            &e->r.currentOrigin.v.m128_f32[0], dir,
            (EHitLocation)HITLOC_NONE);
    }
    else if ((e->flags & 0x2000000) != 0)
    {
        e->takedamage = 1;
        e->health = -1;
    }
}

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
